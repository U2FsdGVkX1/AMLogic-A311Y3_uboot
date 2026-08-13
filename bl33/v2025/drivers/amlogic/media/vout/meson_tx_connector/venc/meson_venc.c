// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2025 Amlogic, Inc. All rights reserved.
 */

#include <asm/global_data.h>
#include <env.h>
#include <amlogic/media/vout/meson_tx_connector/meson_tx_dev.h>
#include <asm/io.h>
#include <fdtdec.h>
#include <errno.h>
#include <stdio.h>
#include <malloc.h>
#include <amlogic/media/vout/meson_tx_connector/venc/meson_venc.h>
#include <amlogic/cpu_id.h>
#include "meson_venc_reg.h"

#define MAX_VENC	4

struct meson_tx_venc_data {
	u32 venc_count;
	/* vbus base address */
	u32 reg_base;
	/* there may be 2 VENC, thus there's address offset */
	u32 venc_offset[MAX_VENC];
};

/* private unique data for specific chip */
static struct meson_tx_venc_data *venc_data;

static void venc_reg_write(u32 enc_index, u32 reg, u32 val)
{
	u32 phys_addr = 0;

	if (enc_index >= venc_data->venc_count) {
		pr_err("venc_idx(%d) exceed support count(%d)\n", enc_index, venc_data->venc_count);
		return;
	}
	phys_addr = venc_data->reg_base + (venc_data->venc_offset[enc_index] << 2) + (reg << 2);
	writel(val, phys_addr);
}

static void venc_reg_update_bits(u32 enc_index, u32 reg, u32 mask, u32 val)
{
	u32 phys_addr = 0;
	u32 reg_val = 0;

	if (enc_index >= venc_data->venc_count) {
		pr_err("venc_idx(%d) exceed support count(%d)\n", enc_index, venc_data->venc_count);
		return;
	}
	phys_addr = venc_data->reg_base + (venc_data->venc_offset[enc_index] << 2) + (reg << 2);

	reg_val = readl(phys_addr);
	reg_val = (reg_val & ~mask) | (val & mask);
	writel(reg_val, phys_addr);
}

static u32 venc_reg_read(u32 enc_index, u32 reg)
{
	u32 val = 10;
	u32 phys_addr = 0;

	if (enc_index >= venc_data->venc_count) {
		pr_err("venc_idx(%d) exceed support count(%d)\n", enc_index, venc_data->venc_count);
		return 0;
	}
	phys_addr = venc_data->reg_base + (venc_data->venc_offset[enc_index] << 2) + (reg << 2);
	val = readl(phys_addr);

	return val;
}

int meson_venc_bist_mode_set(u32 enc_index, enum venc_type enc_type,
	enum venc_bist_type bist_type)
{
	/* Both encp and encl have VIDEO_TST_EN, VIDEO_TST_MDSEL, ..., VIDEO_TST_VDCNT_STSET,
	 * only with the offset 0x128
	 */
	u32 encl_offset;
	u32 reg_mode_adv;
	u32 reg_rgbin_ctrl;
	u32 h_active;
	u32 v_active;
	u32 h_start;
	u32 temp;

	if (enc_index >= MAX_VENC)
		return -1;

	pr_info("%s enc_index[%d] enc_type[%s] bist_type[%d]\n",
		__func__, enc_index, enc_type == VENC_ENCL ? "ENCL" : "ENCP", bist_type);

	encl_offset = (enc_type == VENC_ENCL) ? 0x0128 : 0;
	reg_mode_adv = (enc_type == VENC_ENCL) ? ENCL_VIDEO_MODE_ADV : ENCP_VIDEO_MODE_ADV;
	reg_rgbin_ctrl = (enc_type == VENC_ENCL) ? ENCL_VIDEO_RGBIN_CTRL : ENCP_VIDEO_RGBIN_CTRL;

	if (bist_type == VENC_BIST_PTTN_OFF) {
		venc_reg_update_bits(enc_index, reg_mode_adv, BIT(3), BIT(3));
		venc_reg_update_bits(enc_index, VENC_VIDEO_TST_EN + encl_offset, BIT(0), 0);
		return 0;
	}
	venc_reg_update_bits(enc_index, VENC_VIDEO_TST_EN + encl_offset, GENMASK(2, 0), GENMASK(2, 0));
	venc_reg_update_bits(enc_index, reg_mode_adv, BIT(3), 0);

	if (enc_type == VENC_ENCL) {
		/* ENCL pure color pattern with RGB mode */
		if (bist_type == VENC_BIST_PTTN_BLACK) {
			venc_reg_write(enc_index, VENC_VIDEO_TST_Y + encl_offset, 0x0);
			venc_reg_write(enc_index, VENC_VIDEO_TST_CB + encl_offset, 0x0);
			venc_reg_write(enc_index, VENC_VIDEO_TST_CR + encl_offset, 0x0);
			venc_reg_write(enc_index, VENC_VIDEO_TST_MDSEL + encl_offset, 0);
			return 0;
		}
		if (bist_type == VENC_BIST_PTTN_WHITE) {
			venc_reg_write(enc_index, VENC_VIDEO_TST_Y + encl_offset, 0x3ff);
			venc_reg_write(enc_index, VENC_VIDEO_TST_CB + encl_offset, 0x3ff);
			venc_reg_write(enc_index, VENC_VIDEO_TST_CR + encl_offset, 0x3ff);
			venc_reg_write(enc_index, VENC_VIDEO_TST_MDSEL + encl_offset, 0);
			return 0;
		}
		if (bist_type == VENC_BIST_PTTN_RED) {
			venc_reg_write(enc_index, VENC_VIDEO_TST_Y + encl_offset, 0x3ff);
			venc_reg_write(enc_index, VENC_VIDEO_TST_CB + encl_offset, 0x0);
			venc_reg_write(enc_index, VENC_VIDEO_TST_CR + encl_offset, 0x0);
			venc_reg_write(enc_index, VENC_VIDEO_TST_MDSEL + encl_offset, 0);
			return 0;
		}
		if (bist_type == VENC_BIST_PTTN_GREEN) {
			venc_reg_write(enc_index, VENC_VIDEO_TST_Y + encl_offset, 0x0);
			venc_reg_write(enc_index, VENC_VIDEO_TST_CB + encl_offset, 0x3ff);
			venc_reg_write(enc_index, VENC_VIDEO_TST_CR + encl_offset, 0x0);
			venc_reg_write(enc_index, VENC_VIDEO_TST_MDSEL + encl_offset, 0);
			return 0;
		}
		if (bist_type == VENC_BIST_PTTN_BLUE) {
			venc_reg_write(enc_index, VENC_VIDEO_TST_Y + encl_offset, 0x0);
			venc_reg_write(enc_index, VENC_VIDEO_TST_CB + encl_offset, 0x0);
			venc_reg_write(enc_index, VENC_VIDEO_TST_CR + encl_offset, 0x3ff);
			venc_reg_write(enc_index, VENC_VIDEO_TST_MDSEL + encl_offset, 0);
			return 0;
		}
		/* for other patterns */
		temp = venc_reg_read(enc_index, ENCL_VIDEO_HAVON_END);
		h_active = venc_reg_read(enc_index, ENCL_VIDEO_HAVON_BEGIN);
		/* note that need to use accurate h/vactive(1920/1080) instead
		 * of 1919/1079 in gray/cross pattern
		 */
		h_active = temp - h_active + 1;
		temp = venc_reg_read(enc_index, ENCL_VIDEO_VAVON_ELINE);
		v_active = venc_reg_read(enc_index, ENCL_VIDEO_VAVON_BLINE);
		v_active = temp - v_active + 1;
		h_start = venc_reg_read(enc_index, ENCL_VIDEO_HAVON_BEGIN);
		pr_info("h_active[%d] v_active[%d]\n", h_active, v_active);
	} else if (enc_type == VENC_ENCP) {
		/* ENCP pure color pattern with YCC mode */
		if (bist_type == VENC_BIST_PTTN_BLACK) {
			venc_reg_write(enc_index, VENC_VIDEO_TST_Y + encl_offset, 0x0);
			venc_reg_write(enc_index, VENC_VIDEO_TST_CB + encl_offset, 0x200);
			venc_reg_write(enc_index, VENC_VIDEO_TST_CR + encl_offset, 0x200);
			venc_reg_write(enc_index, VENC_VIDEO_TST_MDSEL + encl_offset, 0);
			return 0;
		}
		if (bist_type == VENC_BIST_PTTN_WHITE) {
			venc_reg_write(enc_index, VENC_VIDEO_TST_Y + encl_offset, 0x3ff);
			venc_reg_write(enc_index, VENC_VIDEO_TST_CB + encl_offset, 0x200);
			venc_reg_write(enc_index, VENC_VIDEO_TST_CR + encl_offset, 0x200);
			venc_reg_write(enc_index, VENC_VIDEO_TST_MDSEL + encl_offset, 0);
			return 0;
		}
		if (bist_type == VENC_BIST_PTTN_RED) {
			venc_reg_write(enc_index, VENC_VIDEO_TST_Y + encl_offset, 0x200);
			venc_reg_write(enc_index, VENC_VIDEO_TST_CB + encl_offset, 0x0);
			venc_reg_write(enc_index, VENC_VIDEO_TST_CR + encl_offset, 0x3ff);
			venc_reg_write(enc_index, VENC_VIDEO_TST_MDSEL + encl_offset, 0);
			return 0;
		}
		if (bist_type == VENC_BIST_PTTN_GREEN) {
			venc_reg_write(enc_index, VENC_VIDEO_TST_Y + encl_offset, 0x200);
			venc_reg_write(enc_index, VENC_VIDEO_TST_CB + encl_offset, 0x0);
			venc_reg_write(enc_index, VENC_VIDEO_TST_CR + encl_offset, 0x0);
			venc_reg_write(enc_index, VENC_VIDEO_TST_MDSEL + encl_offset, 0);
			return 0;
		}
		if (bist_type == VENC_BIST_PTTN_BLUE) {
			venc_reg_write(enc_index, VENC_VIDEO_TST_Y + encl_offset, 0x200);
			venc_reg_write(enc_index, VENC_VIDEO_TST_CB + encl_offset, 0x3ff);
			venc_reg_write(enc_index, VENC_VIDEO_TST_CR + encl_offset, 0x0);
			venc_reg_write(enc_index, VENC_VIDEO_TST_MDSEL + encl_offset, 0);
			return 0;
		}
		/* for other patterns */
		temp = venc_reg_read(enc_index, ENCP_DE_H_END);
		h_active = venc_reg_read(enc_index, ENCP_DE_H_BEGIN);
		h_active = temp - h_active;
		temp = venc_reg_read(enc_index, ENCP_DE_V_END_EVEN);
		v_active = venc_reg_read(enc_index, ENCP_DE_V_BEGIN_EVEN);
		v_active = temp - v_active;
		h_start = venc_reg_read(enc_index, ENCP_VIDEO_HAVON_BEGIN);
	}

	if (bist_type == VENC_BIST_PTTN_LINE) {
		venc_reg_write(enc_index, VENC_VIDEO_TST_MDSEL + encl_offset, 2);
		return 0;
	}
	if (bist_type == VENC_BIST_PTTN_DOT) {
		venc_reg_write(enc_index, VENC_VIDEO_TST_MDSEL + encl_offset, 3);
		return 0;
	}
	if (bist_type == VENC_BIST_PTTN_COLORBAR) {
		venc_reg_write(enc_index, VENC_VIDEO_TST_MDSEL + encl_offset, 1);
		venc_reg_write(enc_index, VENC_VIDEO_TST_CLRBAR_STRT + encl_offset, h_start - 2);
		venc_reg_write(enc_index, VENC_VIDEO_TST_CLRBAR_WIDTH + encl_offset, h_active / 8);
		return 0;
	}
	if (bist_type == VENC_BIST_PTTN_CROSSING) {
		venc_reg_write(enc_index, VENC_VIDEO_TST_MDSEL + encl_offset, 4);
		venc_reg_write(enc_index, VENC_VIDEO_TST_Y + encl_offset, 0x3ff);
		/* aspect ratio: 16:9 */
		venc_reg_update_bits(enc_index, VENC_VIDEO_TST_CB + encl_offset, GENMASK(5, 0), 16);
		venc_reg_update_bits(enc_index, VENC_VIDEO_TST_CR + encl_offset, GENMASK(5, 0), 9);
		venc_reg_write(enc_index, VENC_VIDEO_TST_CLRBAR_WIDTH + encl_offset, h_active);
		venc_reg_write(enc_index, VENC_VIDEO_TST_CLRBAR_STRT + encl_offset, v_active);
		/* cross box width: value 0 means default width = 1 */
		venc_reg_write(enc_index, VENC_VIDEO_TST_VDCNT_STSET + encl_offset, 0);
		venc_reg_update_bits(enc_index, reg_rgbin_ctrl, BIT(1), BIT(1));
		/* when TST_MDSEL is 4, need to reset BIT(9) */
		venc_reg_update_bits(enc_index, VENC_VIDEO_TST_CB + encl_offset, BIT(9), BIT(9));
		venc_reg_update_bits(enc_index, VENC_VIDEO_TST_CB + encl_offset, BIT(9), 0);
		return 0;
	}
	if (bist_type == VENC_BIST_PTTN_GRAY) {
		u32 steps = 32;

		venc_reg_write(enc_index, VENC_VIDEO_TST_MDSEL + encl_offset, 5);
		venc_reg_write(enc_index, VENC_VIDEO_TST_Y + encl_offset, 1024 / steps - 1);
		venc_reg_update_bits(enc_index, VENC_VIDEO_TST_CB + encl_offset, GENMASK(2, 0), 0);
		venc_reg_update_bits(enc_index, VENC_VIDEO_TST_CR + encl_offset, GENMASK(2, 0),
			GENMASK(2, 0));
		venc_reg_update_bits(enc_index, VENC_VIDEO_TST_CR + encl_offset, GENMASK(9, 3),
			(h_active / steps - 1) << 3);
		venc_reg_write(enc_index, VENC_VIDEO_TST_CLRBAR_WIDTH + encl_offset, 0);
		venc_reg_write(enc_index, VENC_VIDEO_TST_CLRBAR_STRT + encl_offset, h_start - 2);
		venc_reg_write(enc_index, VENC_VIDEO_TST_VDCNT_STSET + encl_offset, 0);
		venc_reg_update_bits(enc_index, reg_rgbin_ctrl, BIT(1), BIT(1));
		venc_reg_update_bits(enc_index, VENC_VIDEO_TST_CB + encl_offset, BIT(9), 0);
		return 0;
	}

	return 0;
}

static void config_tv_encp_calc(u32 enc_index, struct meson_tx_format_para *para,
	enum venc_bist_type bist_type)
{
	const struct tx_timing *tp = NULL;
	struct tx_timing timing = {0};
	/* adjust to align upsample and video enable */
	u32 hsync_st = 5; // hsync start pixel count
	u32 vsync_st = 1; // vsync start line count
	// Latency in pixel clock from ENCP_VFIFO2VD request to data ready to HDMI
	const u32 vfifo2vd_to_hdmi_latency = 2;
	u32 de_h_begin = 0;
	u32 de_h_end = 0;
	u32 de_v_begin = 0;
	u32 de_v_end = 0;
	bool y420_mode = 0;
	int hpara_div = 1;

	if (para->cs == HDMI_COLORSPACE_YUV420)
		y420_mode = 1;

	tp = &para->timing;
	timing = *tp;

	timing.h_total /= hpara_div;
	timing.h_blank /= hpara_div;
	timing.h_front /= hpara_div;
	timing.h_sync /= hpara_div;
	timing.h_back /= hpara_div;
	timing.h_active /= hpara_div;

	de_h_end = tp->h_total - (tp->h_front - hsync_st);
	de_h_begin = de_h_end - tp->h_active;
	de_v_end = tp->v_total - (tp->v_front - vsync_st);
	de_v_begin = de_v_end - tp->v_active;

	// VENC timing gen is disabled
	venc_reg_write(enc_index, ENCP_VIDEO_EN, 0);

	// Enable viu vsync interrupt
	venc_reg_write(enc_index, VPU_VENC_CTRL, 1);

	// set DVI/HDMI transfer timing
	// generate hsync
	venc_reg_write(enc_index, ENCP_DVI_HSO_BEGIN, hsync_st);
	venc_reg_write(enc_index, ENCP_DVI_HSO_END, hsync_st + tp->h_sync);

	// generate vsync
	venc_reg_write(enc_index, ENCP_DVI_VSO_BLINE_EVN, vsync_st + y420_mode);
	venc_reg_write(enc_index, ENCP_DVI_VSO_ELINE_EVN,
		vsync_st + tp->v_sync + y420_mode);
	venc_reg_write(enc_index, ENCP_DVI_VSO_BEGIN_EVN, hsync_st);
	venc_reg_write(enc_index, ENCP_DVI_VSO_END_EVN, hsync_st);

	// generate data valid
	venc_reg_write(enc_index, ENCP_DE_H_BEGIN, de_h_begin);
	venc_reg_write(enc_index, ENCP_DE_H_END, de_h_end);
	venc_reg_write(enc_index, ENCP_DE_V_BEGIN_EVEN, de_v_begin);
	venc_reg_write(enc_index, ENCP_DE_V_END_EVEN, de_v_end);

	// set mode
	// Enable Hsync and equalization pulse switch in center; bit[14] cfg_de_v = 1
	venc_reg_write(enc_index, ENCP_VIDEO_MODE, 0x0040 | (1 << 14));
	venc_reg_write(enc_index, ENCP_VIDEO_MODE_ADV, 0x18); // Sampling rate: 1

	// set active region
	venc_reg_write(enc_index, ENCP_VIDEO_HAVON_BEGIN, de_h_begin - vfifo2vd_to_hdmi_latency);
	venc_reg_write(enc_index, ENCP_VIDEO_HAVON_END, de_h_end - vfifo2vd_to_hdmi_latency - 1);
	venc_reg_write(enc_index, ENCP_VIDEO_VAVON_BLINE, de_v_begin);
	venc_reg_write(enc_index, ENCP_VIDEO_VAVON_ELINE, de_v_end - 1);

	//set hsync
	venc_reg_write(enc_index, ENCP_VIDEO_HSO_BEGIN, hsync_st - vfifo2vd_to_hdmi_latency);
	venc_reg_write(enc_index, ENCP_VIDEO_HSO_END, hsync_st + tp->h_sync - vfifo2vd_to_hdmi_latency);

	//set vsync
	venc_reg_write(enc_index, ENCP_VIDEO_VSO_BEGIN, 0);
	venc_reg_write(enc_index, ENCP_VIDEO_VSO_END, 0);
	venc_reg_write(enc_index, ENCP_VIDEO_VSO_BLINE, vsync_st);
	venc_reg_write(enc_index, ENCP_VIDEO_VSO_ELINE, vsync_st + tp->v_sync);

	//set vtotal & htotal
	venc_reg_write(enc_index, ENCP_VIDEO_MAX_PXCNT, tp->h_total - 1);
	venc_reg_write(enc_index, ENCP_VIDEO_MAX_LNCNT, tp->v_total - 1);

	meson_venc_bist_mode_set(enc_index, VENC_ENCP, bist_type);
	venc_reg_write(enc_index, ENCP_VIDEO_EN, 1);

	pr_info("%s set done\n", __func__);
}

static void config_tv_encl_calc(u32 enc_index, struct meson_tx_format_para *para,
	enum venc_bist_type bist_type)
{
	const struct tx_timing *tp = NULL;
	u32 de_h_begin = 0;
	u32 de_h_end = 0;
	u32 de_v_begin = 0;
	u32 de_v_end = 0;

	tp = &para->timing;
	de_h_begin = tp->h_total - tp->h_front - tp->h_active;
	de_h_end = tp->h_total - tp->h_front - 1;
	de_v_begin = tp->v_total - tp->v_front - tp->v_active;
	de_v_end = tp->v_total - tp->v_front - 1;

	// VENC timing gen is disabled
	venc_reg_write(enc_index, ENCL_VIDEO_EN, 0);

	// bit[15] shadow en
	venc_reg_write(enc_index, ENCL_VIDEO_MODE, 0x8000);
	// Sampling rate: 1
	venc_reg_write(enc_index, ENCL_VIDEO_MODE_ADV, 0x0418);
	// bypass filter
	venc_reg_write(enc_index, ENCL_VIDEO_FILT_CTRL, 0x1000);

	//set vtotal & htotal
	venc_reg_write(enc_index, ENCL_VIDEO_MAX_PXCNT, tp->h_total - 1);
	venc_reg_write(enc_index, ENCL_VIDEO_MAX_LNCNT, tp->v_total - 1);
	venc_reg_write(enc_index, ENCL_VIDEO_HAVON_BEGIN, de_h_begin);
	venc_reg_write(enc_index, ENCL_VIDEO_HAVON_END, de_h_end);
	venc_reg_write(enc_index, ENCL_VIDEO_VAVON_BLINE, de_v_begin);
	venc_reg_write(enc_index, ENCL_VIDEO_VAVON_ELINE, de_v_end);

	//  set hsync
	venc_reg_write(enc_index, ENCL_VIDEO_HSO_BEGIN, 0x0);
	venc_reg_write(enc_index, ENCL_VIDEO_HSO_END, tp->h_sync);

	// set vsync
	venc_reg_write(enc_index, ENCL_VIDEO_VSO_BEGIN, 0x0);
	venc_reg_write(enc_index, ENCL_VIDEO_VSO_END, 0x0);
	venc_reg_write(enc_index, ENCL_VIDEO_VSO_BLINE, 0x0);
	venc_reg_write(enc_index, ENCL_VIDEO_VSO_ELINE, tp->v_sync - 1);

	// set inbuf
	venc_reg_write(enc_index, ENCL_INBUF_CNTL1, (5 << 13) | (tp->h_sync - 1));
	venc_reg_write(enc_index, ENCL_INBUF_CNTL0, 0x200);

	venc_reg_write(enc_index, LCD_RGB_BASE_ADDR, 0x0);
	venc_reg_write(enc_index, LCD_RGB_COEFF_ADDR, 0x400);
	venc_reg_write(enc_index, LCD_DITH_CNTL_ADDR, 0x0);
	//venc_reg_write(enc_index, LCD_POL_CNTL_ADDR);

	// DE signal
	venc_reg_write(enc_index, DE_HS_ADDR, de_h_begin);
	venc_reg_write(enc_index, DE_HE_ADDR, de_h_end);
	venc_reg_write(enc_index, DE_VS_ADDR, de_v_begin);
	venc_reg_write(enc_index, DE_VE_ADDR, de_v_end);

	// Hsync signal
	venc_reg_write(enc_index, HSYNC_HS_ADDR, 0x0);
	venc_reg_write(enc_index, HSYNC_HE_ADDR, tp->h_sync);
	venc_reg_write(enc_index, HSYNC_VS_ADDR, 0x0);
	venc_reg_write(enc_index, HSYNC_VE_ADDR, tp->v_total - 1);

	// Vsync signal
	venc_reg_write(enc_index, VSYNC_HS_ADDR, 0x0);
	venc_reg_write(enc_index, VSYNC_HE_ADDR, 0x0);
	venc_reg_write(enc_index, VSYNC_VS_ADDR, 0x0);
	venc_reg_write(enc_index, VSYNC_VE_ADDR, tp->v_sync - 1);

	venc_reg_write(enc_index, ENCL_VIDEO_RGBIN_CTRL, 3);
	meson_venc_bist_mode_set(enc_index, VENC_ENCL, bist_type);
	venc_reg_write(enc_index, ENCL_VIDEO_EN, 1);
	venc_reg_write(enc_index, VPU_VENC_CTRL, 2);
}

int meson_venc_mode_set(u32 enc_index, u32 enc_type,
	enum venc_bist_type bist_type, void *para)
{
	struct meson_tx_format_para *fmt_para = (struct meson_tx_format_para *)para;

	if (enc_index >= MAX_VENC) {
		pr_err("%s: invalid venc offset for index %u\n", __func__, enc_index);
		return -EINVAL;
	}
	pr_info("%s\n", __func__);

	if (enc_type == VENC_ENCP)
		config_tv_encp_calc(enc_index, fmt_para, bist_type);
	else if (enc_type == VENC_ENCL)
		config_tv_encl_calc(enc_index, fmt_para, bist_type);

	return 0;
}

int meson_venc_mode_disable(u32 enc_index, u32 enc_type)
{
	u32 venc_en_addr = (enc_type == VENC_ENCP) ? ENCP_VIDEO_EN : ENCL_VIDEO_EN;

	if (enc_index >= MAX_VENC) {
		pr_err("%s: invalid venc offset for index %u\n", __func__, enc_index);
		return -EINVAL;
	}
	pr_info("%s\n", __func__);

	venc_reg_write(enc_index, venc_en_addr, 0);

	return 0;
}

static struct meson_tx_venc_data a9_venc_data = {
	.venc_count = 2,
	/* v bus base addr */
	.reg_base = 0xFF500000,
	/* offset for venc0/1 */
	.venc_offset = {0x0, 0x600},
};

/* VENC initialization for U-Boot - based on kernel's meson_tx_venc_probe */
int meson_venc_init(void)
{
	u32 chip_type = get_cpu_id().family_id;

	pr_info("%s: Initializing VENC driver\n", __func__);

	switch (chip_type) {
	case MESON_CPU_MAJOR_ID_A9:
		venc_data = &a9_venc_data;
		break;
	default:
		pr_err("meson_venc_init not support chip type: %d\n", chip_type);
		break;
	}

	if (!venc_data)
		return -1;

	return 0;
}
