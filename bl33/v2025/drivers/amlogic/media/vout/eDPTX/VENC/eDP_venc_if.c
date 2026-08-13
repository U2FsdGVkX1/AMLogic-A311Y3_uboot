// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

// #include <common.h>
#include <malloc.h>
#include <asm/amlogic/arch/io.h>
#include <amlogic/media/vout/eDPTX/eDPTX.h>
#ifdef CONFIG_AML_VPP
#include <amlogic/media/vpp/vpp.h>
#endif
#include "eDP_venc.h"
#include "../eDP_regs.h"
#include <linux/delay.h>

struct edptx_venc_op_s *edptx_venc_op;

void dptx_wait_vsync(struct dptx_drv_s *dptx)
{
	if (!dptx || !edptx_venc_op->wait_vsync)
		return;

	edptx_venc_op->wait_vsync(dptx);
}

unsigned int dptx_get_encl_line_cnt(struct dptx_drv_s *dptx)
{
	unsigned int lcnt;

	if (!dptx || !edptx_venc_op->get_encl_line_cnt)
		return 0;

	lcnt = edptx_venc_op->get_encl_line_cnt(dptx);
	return lcnt;
}

unsigned int dptx_get_max_line_cnt(struct dptx_drv_s *dptx)
{
	unsigned int lcnt;

	if (!dptx)
		return 0;
	if (!edptx_venc_op->get_max_lcnt) {
		DPTX_ERR(dptx, "%s: invalid\n", __func__);
		return 0;
	}

	lcnt = edptx_venc_op->get_max_lcnt(dptx);
	return lcnt;
}

void dptx_debug_test(struct dptx_drv_s *dptx, u8 num)
{
	int ret;

	if (!dptx)
		return;
	if (!edptx_venc_op->venc_debug_test) {
		DPTX_ERR(dptx, "%s: invalid\n", __func__);
		return;
	}

	ret = edptx_venc_op->venc_debug_test(dptx, num);
	if (ret) {
		DPTX_ERR(dptx, "%s: %d not support\n", __func__, num);
		return;
	}

	if (num == 0)
		DPTX_PR(dptx, "disable test pattern");
}

/*
 *static void dptx_gamma_init(struct dptx_drv_s *dptx)
 *{
 *	if (!dptx)
 *		return;
 *#ifdef CONFIG_AML_VPP
 *	dptx_wait_vsync(dptx);
 *	vpp_disable_lcd_gamma_table(dptx->idx);
 *
 *	vpp_init_lcd_gamma_table(dptx->idx);
 *
 *	dptx_wait_vsync(dptx);
 *	vpp_enable_lcd_gamma_table(dptx->idx);
 *#endif
 *}
 */

void dptx_set_venc_timing(struct dptx_drv_s *dptx)
{
	if (!dptx)
		return;
	if (!edptx_venc_op->venc_set_timing) {
		DPTX_ERR(dptx, "%s: invalid", __func__);
		return;
	}

	DPTX_PR(dptx, "%s", __func__);
	edptx_venc_op->venc_set_timing(dptx);
}

void dptx_set_venc(struct dptx_drv_s *dptx)
{
	if (!dptx)
		return;
	if (!edptx_venc_op->venc_set) {
		DPTX_ERR(dptx, "%s: invalid", __func__);
		return;
	}

	DPTX_PR(dptx, "%s", __func__);
	edptx_venc_op->venc_set(dptx);
	// lcd_gamma_init(dptx);
}

void dptx_venc_enable(struct dptx_drv_s *dptx, uint8_t flag)
{
	if (!dptx)
		return;
	if (!edptx_venc_op->venc_switch) {
		DPTX_ERR(dptx, "%s: invalid", __func__);
		return;
	}

	DPTX_PR(dptx, "%s: %d", __func__, flag);
	edptx_venc_op->venc_switch(dptx, flag);
}

void dptx_mute_set(struct dptx_drv_s *dptx, uint8_t flag)
{
	if (!dptx)
		return;
	if (!edptx_venc_op->mute_set) {
		DPTX_ERR(dptx, "%s: invalid", __func__);
		return;
	}

	DPTX_PR(dptx, "%s: %d", __func__, flag);
	edptx_venc_op->mute_set(dptx, flag);
}

int edptx_venc_probe(struct dptx_drv_s *dptx)
{
	switch (dptx->data->chip_type) {
#if defined(CONFIG_MESON_T7) || defined(CONFIG_MESON_T7C)
	case eDPTX_CHIP_T7:
		edptx_venc_op = dptx_venc_op_init_t7(dptx);
		break;
#endif
#if defined(CONFIG_MESON_A9)
	case eDPTX_CHIP_A9:
		edptx_venc_op = dptx_venc_op_init_a9(dptx);
		break;
#endif
	default:
		break;
	}

	return 0;
}
