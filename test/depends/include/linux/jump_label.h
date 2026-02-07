/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_JUMP_LABEL_H
#define _LINUX_JUMP_LABEL_H

struct static_key {
	int enabled;
};

#define DEFINE_STATIC_KEY_FALSE(name) struct static_key name = { 0 }

#endif /* _LINUX_JUMP_LABEL_H */
