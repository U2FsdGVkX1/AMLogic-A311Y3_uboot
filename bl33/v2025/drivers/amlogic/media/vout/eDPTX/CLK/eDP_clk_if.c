// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

// #include <common.h>
#include <malloc.h>
#include <amlogic/media/vout/eDPTX/eDPTX.h>
#include "../eDP_common.h"
#include "./eDP_clk_ctrl.h"
#include <linux/delay.h>

struct dptx_clk_op_s *dptx_clk_op;

void dptx_clk_set_vid_clk(struct dptx_drv_s *dptx, uint32_t pixel_clk)
{
	uint8_t cnt = 0;

	if (dptx_clk_op->vid_clk_config)
		dptx_clk_op->vid_clk_config(dptx, pixel_clk);
	if (dptx_clk_op->vid_clk_set)
		dptx_clk_op->vid_clk_set(dptx);

	mdelay(1);

	while (dptx_clk_msr_check(dptx->data->venc_clk_msr_id[dptx->idx], pixel_clk)) {
		if (cnt++ >= 10) {
			DPTX_ERR(dptx, "%s timeout", __func__);
			break;
		}
	}
}

void dptx_clk_disable_vid_clk(struct dptx_drv_s *dptx)
{
	if (dptx_clk_op->vid_clk_disable)
		dptx_clk_op->vid_clk_disable(dptx);
}

void dptx_clk_set_link_clk(struct dptx_drv_s *dptx, uint8_t port, uint8_t dptx_link_rate)
{
	if (dptx_clk_op->link_clk_config)
		dptx_clk_op->link_clk_config(dptx, port, dptx_link_rate);

	if (dptx_clk_op->clktree_set)
		dptx_clk_op->clktree_set(dptx);

	if (dptx_clk_op->link_clk_set)
		dptx_clk_op->link_clk_set(dptx, port);

	if (dptx_clk_op->clk_ssc_switch)
		dptx_clk_op->clk_ssc_switch(dptx, port, 1);
}

void dptx_clk_disable_link_clk(struct dptx_drv_s *dptx, uint8_t port)
{
	if (dptx_clk_op->link_clk_disable)
		dptx_clk_op->link_clk_disable(dptx, port);
}

void dptx_clk_config_print(struct dptx_drv_s *dptx)
{
	if (dptx_clk_op->clk_config_print)
		dptx_clk_op->clk_config_print(dptx);
}

void edptx_clk_config_probe(struct dptx_drv_s *dptx)
{
	switch (dptx->data->chip_type) {
#if defined(CONFIG_MESON_T7) || defined(CONFIG_MESON_T7C)
	case eDPTX_CHIP_T7:
		dptx_clk_op = dptx_clk_op_init_t7(dptx);
		break;
#endif
#if defined(CONFIG_MESON_A9)
	case eDPTX_CHIP_A9:
		dptx_clk_op = dptx_clk_op_init_a9(dptx);
		break;
#endif
	default:
		DPTX_ERR(dptx, "%s: invalid chip type", __func__);
		return;
	}
}
