/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_PID_H
#define _LINUX_PID_H

#include <linux/types.h>

enum pid_type {
	PIDTYPE_PID,
	PIDTYPE_TGID,
	PIDTYPE_PGID,
	PIDTYPE_SID,
	PIDTYPE_MAX,
};

struct pid {};

#ifdef __cplusplus
extern "C" {
#endif

extern struct task_struct *get_pid_task(struct pid *pid, enum pid_type);

extern struct pid *find_vpid(int nr);

extern struct pid *find_get_pid(pid_t nr);
extern void put_pid(struct pid *pid);

#ifdef __cplusplus
}
#endif

#endif /* _LINUX_PID_H */
