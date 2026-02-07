/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __LINUX_KMOD_H__
#define __LINUX_KMOD_H__


#include <linux/umh.h>
#include <linux/gfp.h>
#include <linux/stddef.h>
#include <linux/errno.h>
#include <linux/compiler.h>
#include <linux/workqueue.h>
#include <linux/sysctl.h>

#define KMOD_PATH_LEN 256

#ifdef __cplusplus
extern "C" {
#endif
extern int call_usermodehelper(const char *path, char **argv, char **envp, int wait);
#ifdef __cplusplus
}
#endif

#endif /* __LINUX_KMOD_H__ */
