/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_VERSION_H
#define _LINUX_VERSION_H

/*
 * DT harness builds kernel-style sources in user space. The host toolchain's
 * `<linux/version.h>` may not match the kernel version the sources expect.
 *
 * Pin a consistent "target kernel" version here so stub headers can select the
 * right compatibility branches.
 */
#ifndef KERNEL_VERSION
#define KERNEL_VERSION(a, b, c) (((a) << 16) + ((b) << 8) + (c))
#endif

#ifndef LINUX_VERSION_CODE
#define LINUX_VERSION_CODE KERNEL_VERSION(6, 6, 0)
#endif

#endif /* _LINUX_VERSION_H */

