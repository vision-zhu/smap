// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 * Description: linux namei.h stub
 */
#ifndef _LINUX_NAMEI_H
#define _LINUX_NAMEI_H

#include <linux/errno.h>
#include <linux/path.h>

#define LOOKUP_DIRECTORY 0x0002

static inline int kern_path(const char *name, unsigned int flags, struct path *path)
{
	(void)name;
	(void)flags;
	(void)path;
	return -ENOENT;
}

static inline void path_put(const struct path *path)
{
	(void)path;
}

#endif
