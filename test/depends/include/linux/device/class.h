// SPDX-License-Identifier: GPL-2.0

#ifndef _DEVICE_CLASS_DEPENDS_H_
#define _DEVICE_CLASS_DEPENDS_H_

#include <linux/kobject.h>

struct class {
};

#ifdef __cplusplus
extern "C" {
#endif
struct class *class_create_stub(void *owner, const char *name);
void class_destroy(struct class *cls);
#ifdef __cplusplus
}
#endif

/* Support both historical and newer kernel call sites. */
#define __class_create_1(name) class_create_stub(NULL, (name))
#define __class_create_2(owner, name) class_create_stub((void *)(owner), (name))
#define __CLASS_CREATE_GET(_1, _2, NAME, ...) NAME
#define class_create(...) \
	__CLASS_CREATE_GET(__VA_ARGS__, __class_create_2, __class_create_1)(__VA_ARGS__)

#endif
