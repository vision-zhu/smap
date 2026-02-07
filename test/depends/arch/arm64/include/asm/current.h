/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __ASM_DEPENDS_CURRENT_H
#define __ASM_DEPENDS_CURRENT_H

#include <linux/compiler.h>

#ifndef __ASSEMBLY__

struct task_struct;

static struct task_struct *get_current(void)
{
#if defined(__aarch64__)
	unsigned long depends_sp_el0;

	asm volatile("mrs %0, sp_el0" : "=r"(depends_sp_el0));

	return (struct task_struct *)depends_sp_el0;
#else
	/*
	 * DT harness is built/ran in user-space and may run on non-arm64 hosts.
	 * Provide a safe stub to avoid embedding arm64-only inline asm.
	 */
	return (struct task_struct *)0;
#endif
}

#define current get_current()

#endif

#endif

