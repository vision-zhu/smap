/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * class.c - basic device class management
 *
 * Copyright (c) 2002-3 Patrick Mochel
 * Copyright (c) 2002-3 Open Source Development Labs
 * Copyright (c) 2003-2004 Greg Kroah-Hartman
 * Copyright (c) 2003-2004 IBM Corp.
 */

#include <linux/device/class.h>
#include <linux/err.h>
#include <linux/errno.h>

struct class *class_create_stub(void *owner, const char *name)
{
	(void)owner;
	static struct class cls;

	if (!name) {
		return (struct class *)ERR_PTR(-EINVAL);
	}
	return &cls;
}

void class_destroy(struct class *cls)
{
	(void)cls;

}