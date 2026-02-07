## 多 NUMA 进程/虚机：当前设计梳理与优化建议

本文梳理当前代码仓中 **SMAP 多 NUMA 进程/虚机** 的核心数据模型、控制链路与迁移策略实现，并给出针对“正确性、可维护性、可观测性与性能”的优化建议。

### 1. NUMA 拓扑与节点编号

#### 1.1 内核侧（`src/tiering/`）NUMA 拓扑

- **本地 NUMA 数量**：由 ACPI/系统内存拓扑初始化得到 `nr_local_numa`（`src/tiering/acpi_mem.h`）。
- **远端 NUMA 识别**：`is_numa_remote(nid)` 认为 `nid >= nr_local_numa && nid < SMAP_MAX_NUMNODES`（`SMAP_MAX_NUMNODES=22`）。
- **远端内存段**：通过 OBMM/`walk_iomem` 等方式刷新 `remote_ram_list`（`src/tiering/iomem.c`），用于：
  - 远端页 “index↔paddr” 转换（如 `calc_acidx_paddr_iomem`）
  - memid 到物理地址段的映射（`find_range_by_memid`）

#### 1.2 用户态（`src/user/`）NUMA bitmap 表示

用户态用 `uint32_t numaNodes` 同时表达：

- **L1（本地）节点集合**：占用低 `LOCAL_NUMA_BITS(=4)` 个 bit
- **L2（远端）节点集合**：占用其后的 `REMOTE_NUMA_BITS(=18)` 个 bit
- **总节点上限**：`MAX_NODES = LOCAL_NUMA_BITS + REMOTE_NUMA_BITS = 22`

为了兼容“实际本地 NUMA 数 < 4”的机器，`manage.h` 对 L2 的存取做了 `offset = LOCAL_NUMA_BITS - nrLocalNuma` 的映射：

- 设置/判断远端 nid 时，会将 **实际 nid** 映射到 bitmap 的 L2 bit 区间（从 bit4 开始）。

相关 helper 定义在：
- `src/user/manage/numa_nodes.h`
- `src/user/manage/manage.h`（`SetAttrL1/SetAttrL2/InAttrL1/InAttrL2/...`）

### 2. 多 NUMA 进程/虚机的控制面与数据面

#### 2.1 纳管对象与配置结构（用户态）

核心结构是 `ProcessAttr`（`src/user/manage/manage.h`），关键字段：

- **NUMA 归属**：`numaAttr.numaNodes`（L1/L2 bitmap）
- **多远端 NUMA 参数**：
  - `remoteNumaCnt`：远端 NUMA 个数
  - `migrateParam[i].nid / memSize`：多远端场景下按远端维度配置（主要用于 VM/2M）
- **策略输入/账本**：
  - `walkPage.nrPages[nid]`：本周期统计的各 NUMA 页数（L1/L2 统一编号）
  - `scanAttr.actcData[nid] / actcLen[nid]`：每个 NUMA 的热点数据（地址+频次+prior）
  - `strategyAttr.remoteNrPagesAfterMigrate[l1][l2]`：迁移后账本（用于资源分摊/复算）
  - `strategyAttr.initRemoteMemRatio[l1][l2]`：用户配置/上游配置的迁出比例矩阵

#### 2.2 周期链路（用户态线程：扫描→策略→迁移）

主循环在 `ScanMigrateWork`（`src/user/strategy/migration.c`）：

1. **禁用 tracking**（避免读写冲突）
2. **清理无效 pid**（`CheckAndRemoveInvalidProcess`）
3. **构建所有 pid 的冷热数据**：`BuildAllPidData`
4. **资源分配/比例调整**：`ConfigRatios`（结合借用远端内存与多 VM 情况）
5. **策略计算迁移矩阵**：`RunStrategy(...)` 产出 `MigList[from][to]`
6. **构造 ioctl 消息并迁移**：`ioctl(SMAP_MIG_MIGRATE, ...)`
7. **更新账本**：`UpdateMigResult` 将成功迁移的数量记回 `remoteNrPagesAfterMigrate`

#### 2.3 多 NUMA VM 的策略实现（用户态）

多 NUMA VM 的核心入口是 `SeparateStrategyMultiNumaVm`（`src/user/strategy/separate_strategy.c`），其差异点在于：

- 不是仅围绕单一 L1↔单一 L2 做冷热交换，而是：
  - 汇总 L1（多个本地 NUMA）与 L2（多个远端 NUMA）的 actc 数据
  - 对每个远端 NUMA 计算 promote/demote 目标量（`RemoteMigInfo`）
  - 按节点剩余 free hugepages 构造 `mlist[from][to]`（可能拆分到多个目的本地 NUMA）

### 3. 迁移执行（内核态）

内核迁移入口主要是 `src/tiering/mig_init.c` 暴露的字符设备 ioctl：

- `SMAP_MIG_MIGRATE`：按 “pid+from+to+addrlist” 粒度迁移（冷热迁移主路径）
- `SMAP_MIG_MIGRATE_NUMA`：按 memid 列表将远端 NUMA 的地址段迁移到另一个远端 NUMA
- `SMAP_MIG_PID_REMOTE_NUMA`：按 pid 扫描页表并将某个远端 NUMA 的页迁移到另一个远端 NUMA

实际迁移执行在 `src/tiering/smap_migrate_pages.c`，并包含：

- 单线程/多线程迁移（`migrate_multi_threaded`）
- 迁回子任务（migrate back）在 `tracking_manage.c` 的 workqueue 定时调度

### 4. 已落地修复（本次 MR）

围绕多 NUMA 的正确性/一致性，本次 MR 修复了几个会影响多 NUMA 行为的关键问题：

- **NUMA bitmap 掩码与置位逻辑修复**（`src/user/manage/numa_nodes.h`）
  - 修正 L1/L2 mask 计算，确保 `SetL1/SetL2` 不会互相误清除对方 bit
- **多 NUMA VM 配比写入更精确**（`src/user/manage/manage.c`）
  - 多 NUMA VM 配比仅写入该 VM 实际使用的本地 NUMA（避免污染未使用 L1）
- **配置恢复时支持多本地 NUMA**（`src/user/manage/smap_config.c`）
  - 从 config 恢复时按 `InAttrL1` 写入各本地 NUMA 的比例/size
- **修复 `QueryManagedProcess` 的类型判断 bug**（`src/user/manage/manage.c`）
  - 避免误改 `ProcessAttr.type` 导致结果错误/潜在越界写
- **修复 `NumaSwapReduce` 指针索引 bug**（`src/user/strategy/migration.c`）
  - 多节点场景下避免错误推进索引
- **完善若干缺失 return**（例如 `InitSeparateParam`/`RemoteNumaInfoInit`）
- **远端页数统计从 `nrLocalNuma` 起算**（`src/user/strategy/strategy.h`）

### 5. 后续优化建议（未在本次 MR 一次性做完）

- **一致性与边界校验**
  - 为多 NUMA VM：在 `ProcessAddManage/SetProcessConfig` 处校验 `remoteNumaCnt`、重复 nid、nid 范围与 ratio 总和。
  - 用户态账本 `remoteNrPagesAfterMigrate` 与 `walkPage.nrPages` 的一致性检查可收敛为单一函数，周期内统一修正。
- **可观测性**
  - 用户态：输出每个 pid 的 “本地节点集合/远端节点集合/目标配比/实际分布/本周期迁移矩阵摘要”。
  - 内核态：为 per-node 迁移耗时、失败原因分类、隔离失败次数等提供 debugfs 统计。
- **性能与并发**
  - 内核 `smap_migrate_pages.c` 的 round-robin 目的节点选择结构是全局变量，未来可加锁/原子化，避免并发回迁时产生不一致。
  - 多线程迁移线程绑定/NUMA aware 的 kthread 分配可进一步减少跨 NUMA 调度开销。

