// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2025 Altera Corporation <www.altera.com>
 */

// #define LOG_CATEGORY LOGC_DM
// #define LOG_DEBUG 1
#include <dm.h>
#include <i3c.h>
#include <errno.h>
#include <log.h>
#include <dm/device-internal.h>
#include <linux/ctype.h>

int dm_i3c_read(struct udevice *dev, u32 dev_number,
		u8 *buf, u32 num_bytes)
{
	struct dm_i3c_ops *ops = i3c_get_ops(dev);

	if (!ops->read)
		return -ENOSYS;
#ifdef CONFIG_AMLOGIC_MODIFY
	//device 0 is master it not attached itself
	return ops->read(dev, dev_number - 1, buf, num_bytes);
#else
	return ops->read(dev, dev_number, buf, num_bytes);
#endif
}

int dm_i3c_write(struct udevice *dev, u32 dev_number,
		 u8 *buf, u32 num_bytes)
{
	struct dm_i3c_ops *ops = i3c_get_ops(dev);

	if (!ops->write)
		return -ENOSYS;
#ifdef CONFIG_AMLOGIC_MODIFY
	//device 0 is master it not attached itself
	return ops->write(dev, dev_number - 1, buf, num_bytes);
#else
	return ops->write(dev, dev_number, buf, num_bytes);
#endif
}

#ifdef CONFIG_AMLOGIC_MODIFY
static bool i3c_check_device_info_match(u32 *reg, struct i3c_dev_desc *desc)
{
	u64 pid = ((u64)reg[1] << 32) | reg[2];

	if (desc->info.pid == pid)
		return 1;

	return 0;
}

static int i3c_child_pre_probe(struct udevice *dev)
{
	struct udevice *pdev = dev_get_parent(dev);
	struct i3c_master_controller *master = dev_get_plat(pdev);
	struct i3c_dev_desc *desc;
	u32 reg[3];
	int ret;

	log_debug("%s master dev = 0x%p master = 0x%p\n", __func__, pdev, master);
	if (!master->init_done) {
		ret = -EINVAL;
		goto exit_dev_bind;
	}

	ret = dev_read_u32_array(dev, "reg", reg, ARRAY_SIZE(reg));
	if (ret)
		goto exit_dev_bind;
	log_debug("%s, reg0: 0x%x\n", dev->name, reg[0]);
	log_debug("%s, reg1: 0x%x\n", dev->name, reg[1]);
	log_debug("%s, reg2: 0x%x\n", dev->name, reg[2]);
	//bind udev to i3c desc
	i3c_bus_for_each_i3cdev(&master->bus, desc) {
		if (desc->dev)
			continue;
		if (i3c_check_device_info_match(reg, desc)) {
			desc->dev = (struct i3c_device *)dev_get_parent_plat(dev);
			desc->dev->bus = &master->bus;
			desc->dev->desc = desc;
			desc->dev->dev = dev;
			log_debug("%s: bind i3c dev desc pass\n", dev->name);

			return 0;
		}
	}
	ret = -EACCES;
exit_dev_bind:
	log_warning("fail to bind i3c dev to desc. ret:%d\n", ret);

	return ret;
}

static int i3c_post_bind(struct udevice *dev)
{
	log_debug("%s master dev = 0x%p\n", __func__, dev);
	log_debug("%s: %s, seq=%d\n", __func__, dev->name, dev_seq(dev));

#if CONFIG_IS_ENABLED(OF_REAL)
	dm_scan_fdt_dev(dev);
#endif
	return 0;
}
#endif

UCLASS_DRIVER(i3c) = {
	.id		= UCLASS_I3C,
	.name		= "i3c",
#ifdef CONFIG_AMLOGIC_MODIFY
	.per_child_plat_auto	= sizeof(struct i3c_device),
	.post_bind = i3c_post_bind,
	.child_pre_probe = i3c_child_pre_probe,
#endif
};
