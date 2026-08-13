// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2026 Amlogic, Inc. All rights reserved.
 */

#include <dm/uclass.h>
#include <linux/err.h>
#include <asm/amlogic/arch/i3c_init.h>

int i3c_devices_active(int i3c_host_num)
{
	int ret;
	int idx;
	struct udevice *i3c_host;

	for (idx = 0; idx < i3c_host_num; idx++) {
		ret = uclass_get_device(UCLASS_I3C, idx, &i3c_host);
		if (ret) {
			debug("%s: failed to active i3c device [%d]",
					__func__, idx);
			return -EINVAL;
		}
	}

	return 0;
}
