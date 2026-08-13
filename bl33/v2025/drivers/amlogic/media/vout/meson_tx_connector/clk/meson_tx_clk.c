// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <init.h>
#include <dm/of.h>
#include <dm/device.h>
#include <dm/device_compat.h>
#include <linux/errno.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <dm/of_addr.h>
#include <amlogic/media/vout/meson_tx_connector/clk/meson_tx_clk.h>
#include <amlogic/cpu_id.h>
#include "meson_tx_clk_internal.h"

#define DEVICE_NAME "meson_tx_clk"

u32 tx_clk_read_reg(u32 base, u32 addr_offset)
{
	u32 val;

	val = readl(base + addr_offset);
	pr_debug("clk_reg Rd32[0x%08x] 0x%08x\n", addr_offset, val);
	return val;
}

void tx_clk_write_reg(u32 base, u32 addr, u32 val)
{
	u32 rval;

	writel(val, base + addr);
	rval = readl(base + addr);
	if (val != rval)
		pr_err("clk_reg Wr32[0x%08x] 0x%08x != Rd32 0x%08x\n",
			addr, val, rval);
	else
		pr_debug("clk_reg Wr32[0x%08x] 0x%08x\n", addr, val);
}

void tx_clk_set_reg_bits(u32 base, u32 addr, u32 value, u32 offset, u32 len)
{
	u32 data32 = 0;

	data32 = tx_clk_read_reg(base, addr);
	data32 &= ~(((1 << len) - 1) << offset);
	data32 |= (value & ((1 << len) - 1)) << offset;
	tx_clk_write_reg(base, addr, data32);
}

int meson_tx_clk_set(struct meson_tx_clk *tx_clk, u32 clk_mask)
{
	if (!tx_clk || !tx_clk->tx_clk_ops) {
		pr_err("%s invalid clk param\n", __func__);
		return -EINVAL;
	}

	pr_info("%s", __func__);
	mutex_lock(&tx_clk->clk_set_mutex);
	if (tx_clk->tx_clk_ops->tx_clk_set)
		tx_clk->tx_clk_ops->tx_clk_set(tx_clk, clk_mask);
	mutex_unlock(&tx_clk->clk_set_mutex);
	return 0;
}

int meson_tx_clk_disable(struct meson_tx_clk *tx_clk, u32 clk_mask)
{
	if (!tx_clk || !tx_clk->tx_clk_ops) {
		pr_err("%s invalid meson_tx_clk param\n", __func__);
		return -EINVAL;
	}

	pr_info("%s", __func__);
	mutex_lock(&tx_clk->clk_set_mutex);
	if (tx_clk->tx_clk_ops->tx_clk_disable)
		tx_clk->tx_clk_ops->tx_clk_disable(tx_clk, clk_mask);
	mutex_unlock(&tx_clk->clk_set_mutex);
	return 0;
}

static struct meson_tx_clk_ops tx_clk_ops_a9 = {
	.tx_clk_set = tx_bulk_clk_set_a9,
	.tx_clk_disable = tx_bulk_clk_disable_a9,
};

static u32 reg_base_a9[] = {
	/* ana_ctrl */
	0xFE016000,
	/* clk_ctrl*/
	0xFE000000
};

struct meson_tx_clk *meson_tx_clk_probe(void)
{
	struct meson_tx_clk *tx_clk;
	u32 chip_type = get_cpu_id().family_id;
	const struct meson_tx_clk_ops *tx_clk_ops = NULL;

	switch (chip_type) {
	case MESON_CPU_MAJOR_ID_A9:
		tx_clk_ops = &tx_clk_ops_a9;
		break;
	default:
		pr_err("meson_tx_clk not support chip type: %d\n", chip_type);
		return NULL;
	}

	tx_clk = kzalloc(sizeof(*tx_clk), GFP_KERNEL);
	if (!tx_clk)
		return NULL;

	tx_clk->tx_clk_ops = tx_clk_ops;
	memcpy(tx_clk->reg_io_base, reg_base_a9, sizeof(tx_clk->reg_io_base));
	mutex_init(&tx_clk->clk_set_mutex);

	pr_info("%s\n", __func__);
	return tx_clk;
}

