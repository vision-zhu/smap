/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Based on arch/arm/include/asm/io.h
 *
 * Copyright (C) 1996-2000 Russell King
 * Copyright (C) 2012 ARM Ltd.
 */
#ifndef __ASM_IO_H
#define __ASM_IO_H

#include <asm/byteorder.h>

#define readl(c)                 \
	(0U + (unsigned int)0 * (unsigned long)(c))
#define writel(v, c)             \
	do {                     \
		(void)(v);        \
		(void)(c);        \
	} while (0)
#define iounmap(addr)                 \
	do {                          \
		(void)(addr);         \
	} while (0)
#define ioremap(addr, size) ((void *)(unsigned long)(addr))

#endif	/* __ASM_IO_H */
