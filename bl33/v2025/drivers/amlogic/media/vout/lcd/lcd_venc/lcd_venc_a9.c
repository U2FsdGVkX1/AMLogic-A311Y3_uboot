// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <malloc.h>
// #include <asm/arch/io.h>
#ifdef CONFIG_AML_VPP
#include <amlogic/media/vpp/vpp.h>
#endif
#include <amlogic/media/vout/lcd/aml_lcd.h>
#include "../lcd_reg.h"
#include "../lcd_common.h"
#include "lcd_venc.h"

#if defined (CONFIG_MESON_A9)
static void lcd_venc_wait_vsync(struct aml_lcd_drv_s *pdrv)
{
	unsigned int offset, reg;
	int line_cnt, line_cnt_previous;
	int i = 0;

	offset = pdrv->data->offset_venc[pdrv->index];
	reg = VPU_VENCL_STAT + offset;

	line_cnt = 0x1fff;
	line_cnt_previous = lcd_vcbus_getb(reg, 16, 13);
	while (i++ < LCD_WAIT_VSYNC_TIMEOUT) {
		line_cnt = lcd_vcbus_getb(reg, 16, 13);
		if (line_cnt < line_cnt_previous)
			break;
		line_cnt_previous = line_cnt;
		udelay(2);
	}
}

static unsigned int lcd_venc_get_max_lint_cnt(struct aml_lcd_drv_s *pdrv)
{
	unsigned int offset, reg, line_cnt;

	offset = pdrv->data->offset_venc[pdrv->index];
	reg = ENCL_VIDEO_MAX_LNCNT + offset;

	line_cnt = lcd_vcbus_read(reg) + 1;
	/*LCDPR("[%d]: %s: line_cnt=%d", pdrv->index, __func__, line_cnt); */

	return line_cnt;
}

struct lcd_enc_test_t {
	char *name;
	unsigned int mode;
	unsigned int y;
	unsigned int cb;
	unsigned int cr;
	unsigned int en;
	unsigned int vfifo_en;
	unsigned int rgb_in;
};

static struct lcd_enc_test_t lcd_enc_tst_a9[] = {
	{"0-None",         0,    0x200,   0x200, 0x200, 0, 1, 3},  /* 0 */
	{"1-Color Bar",    1,    0x200,   0x200, 0x200, 1, 0, 1},  /* 1 */
	{"2-Thin Line",    2,    0x200,   0x200, 0x200, 1, 0, 1},  /* 2 */
	{"3-Dot Grid",     3,    0x200,   0x200, 0x200, 1, 0, 1},  /* 3 */
	{"4-Gray",         0,    0x1ff,   0x1ff, 0x1ff, 1, 0, 3},  /* 4 */
	{"5-Red",          0,    0x3ff,     0x0,   0x0, 1, 0, 3},  /* 5 */
	{"6-Green",        0,      0x0,   0x3ff,   0x0, 1, 0, 3},  /* 6 */
	{"7-Blue",         0,      0x0,     0x0, 0x3ff, 1, 0, 3},  /* 7 */
	{"8-Black",        0,      0x0,     0x0,   0x0, 1, 0, 3},  /* 8 */
	{"9-X icon",       4,    0x3ff,    0x10,   0x9, 1, 0, 2},  /* 9 */
	{"10-Gray Scale",  5, 0xffffffff,   0x7,   0x7, 1, 0, 3},  /* 10 */
	{"11-Red Scale",   5,      0x1,     0x0,   0x1, 1, 0, 3},  /* 11 */
	{"12-Green Scale", 5,      0x1,     0x0,   0x2, 1, 0, 3},  /* 12 */
	{"13-Blue Scale",  5,      0x1,     0x0,   0x4, 1, 0, 3},  /* 13 */
	{"14-Window",      6,    0x3ff,    0x10,   0x9, 1, 0, 3},  /* 14 */
};

static int lcd_venc_debug_test(struct aml_lcd_drv_s *pdrv, unsigned int num)
{
	unsigned int start, width, height, offset;
	unsigned gcd_num;

	offset = pdrv->data->offset_venc[pdrv->index];
	start = pdrv->config.timing.hstart;
	width = pdrv->config.timing.act_timing.h_active / 8;

	lcd_venc_wait_vsync(pdrv);
	lcd_vcbus_write(ENCL_VIDEO_RGBIN_CTRL + offset, lcd_enc_tst_a9[num].rgb_in);
	if (num == 9) {
		lcd_vcbus_write(ENCL_TST_Y + offset, lcd_enc_tst_a9[num].y);
		width = pdrv->config.timing.act_timing.h_active;
		height = pdrv->config.timing.act_timing.v_active;
		gcd_num = gcd(width, height);
		lcd_vcbus_write(ENCL_TST_CB + offset, width / gcd_num);
		lcd_vcbus_write(ENCL_TST_CR + offset, height / gcd_num);
		lcd_vcbus_write(ENCL_TST_CLRBAR_STRT + offset, height);
		lcd_vcbus_write(ENCL_TST_CLRBAR_WIDTH + offset, width);
	} else if (num == 14) {
		lcd_vcbus_write(ENCL_TST_VDCNT_STSET + offset, 1);
		lcd_vcbus_write(ENCL_TST_VDCNT_STSET + offset, 2);
		width = pdrv->config.timing.act_timing.h_active / 3;
		height = pdrv->config.timing.act_timing.v_active / 3;
		lcd_vcbus_write(ENCL_TST_CLRBAR_STRT + offset, width);
		lcd_vcbus_write(ENCL_TST_CLRBAR_WIDTH + offset, height);
		lcd_vcbus_write(ENCL_TST_Y + offset, 0);
		lcd_vcbus_write(ENCL_TST_CB + offset, 0);
		lcd_vcbus_write(ENCL_TST_CR + offset, 0);
		LCDPR("start point x=%d y=%d\n", width, height);
		width = pdrv->config.timing.act_timing.h_active * 2 / 3;
		height = pdrv->config.timing.act_timing.v_active * 2 / 3;
		lcd_vcbus_write(ENCL_TST_VDCNT_STSET + offset, 3);
		lcd_vcbus_write(ENCL_TST_CLRBAR_STRT + offset, width);
		lcd_vcbus_write(ENCL_TST_CLRBAR_WIDTH + offset, height);
		lcd_vcbus_write(ENCL_TST_Y + offset, 0x3ff);
		lcd_vcbus_write(ENCL_TST_CB + offset, 0x3ff);
		lcd_vcbus_write(ENCL_TST_CR + offset, 0x3ff);
		LCDPR("end point x=%d y=%d\n", width, height);

		width = pdrv->config.timing.act_timing.h_active;
		height = pdrv->config.timing.act_timing.v_active;
		lcd_vcbus_write(ENCL_TST_VDCNT_STSET + offset, 0);
		lcd_vcbus_write(ENCL_TST_CLRBAR_STRT + offset, height);
		lcd_vcbus_write(ENCL_TST_CLRBAR_WIDTH + offset, width);
	} else {
		lcd_vcbus_write(ENCL_TST_Y + offset, lcd_enc_tst_a9[num].y);
		lcd_vcbus_write(ENCL_TST_CB + offset, lcd_enc_tst_a9[num].cb);
		lcd_vcbus_write(ENCL_TST_CR + offset, lcd_enc_tst_a9[num].cr);
		lcd_vcbus_write(ENCL_TST_CLRBAR_STRT + offset, start - 2);
		lcd_vcbus_write(ENCL_TST_CLRBAR_WIDTH + offset, width - 1);
	}
	lcd_vcbus_write(ENCL_TST_MDSEL + offset, lcd_enc_tst_a9[num].mode);
	lcd_vcbus_setb(ENCL_TST_EN + offset, lcd_enc_tst_a9[num].en, 0, 1);
	lcd_vcbus_setb(ENCL_VIDEO_MODE_ADV + offset, lcd_enc_tst_a9[num].vfifo_en, 3, 1);
	if (num > 0) {
		LCDPR("[%d]: show test pattern: %s\n",
		      pdrv->index, lcd_enc_tst_a9[num].name);
	}
	return 0;
}

static int lcd_venc_window_attr_set(struct aml_lcd_drv_s *pdrv,
					struct lcd_window_attr_s *window_attr)
{
	unsigned int offset;

	lcd_venc_wait_vsync(pdrv);
	offset = pdrv->data->offset_venc[pdrv->index];
	if (lcd_vcbus_getb(ENCL_TST_MDSEL + offset, 0, 3) != 6) {
		LCDPR("not in window pattern mode\n");
		return -1;
	}
	lcd_vcbus_write(ENCL_TST_VDCNT_STSET + offset, 1); /* clear line and row cnt */
	lcd_vcbus_write(ENCL_TST_VDCNT_STSET + offset, 2); /* window in */
	lcd_vcbus_write(ENCL_TST_CLRBAR_STRT + offset, window_attr->pos.start_x);
	lcd_vcbus_write(ENCL_TST_CLRBAR_WIDTH + offset, window_attr->pos.start_y);
	lcd_vcbus_write(ENCL_TST_Y + offset, window_attr->color_in.red);
	lcd_vcbus_write(ENCL_TST_CB + offset, window_attr->color_in.green);
	lcd_vcbus_write(ENCL_TST_CR + offset, window_attr->color_in.blue);
	LCDPR("start point x=%d y=%d\n", window_attr->pos.start_x, window_attr->pos.start_y);

	lcd_vcbus_write(ENCL_TST_VDCNT_STSET + offset, 3); /* window out */
	lcd_vcbus_write(ENCL_TST_CLRBAR_STRT + offset, window_attr->pos.end_x);
	lcd_vcbus_write(ENCL_TST_CLRBAR_WIDTH + offset, window_attr->pos.end_y);
	lcd_vcbus_write(ENCL_TST_Y + offset, window_attr->color_out.red);
	lcd_vcbus_write(ENCL_TST_CB + offset, window_attr->color_out.green);
	lcd_vcbus_write(ENCL_TST_CR + offset, window_attr->color_out.blue);
	LCDPR("end point x=%d y=%d\n", window_attr->pos.end_x, window_attr->pos.end_y);
	return 0;
}

static void lcd_venc_probe_cursor(struct aml_lcd_drv_s *pdrv,
					struct lcd_cursor_attr_s *cursor_attr)
{
	unsigned int offset = pdrv->data->offset_venc[pdrv->index];
	if (cursor_attr->status) {
		if (cursor_attr->x >= pdrv->config.timing.act_timing.h_active ||
			cursor_attr->y >= pdrv->config.timing.act_timing.v_active)
			return;
		lcd_vcbus_setb(VPU_VENC_PROBE_CTRL + offset, 3 | (cursor_attr->mode << 2), 0, 3);
		if (cursor_attr->x && cursor_attr->y)
			lcd_vcbus_write(VPU_VENC_PROBE_POS + offset,
					(cursor_attr->y << 16) | cursor_attr->x);
		lcd_vcbus_write(VPU_VENC_PROBE_SIZE + offset,
				pdrv->config.timing.act_timing.h_active);
		lcd_vcbus_write(VPU_VENC_PROBE_HL_COL0 + offset,
				cursor_attr->green << 17 | cursor_attr->blue);
		lcd_vcbus_write(VPU_VENC_PROBE_HL_COL1 + offset, cursor_attr->red);
	} else {
		lcd_vcbus_setb(VPU_VENC_PROBE_CTRL + offset, 0, 0, 3);
	}
}

static void lcd_venc_set_tcon(struct aml_lcd_drv_s *pdrv)
{
	struct lcd_config_s *pconf = &pdrv->config;

	lcd_vcbus_write(LCD_RGB_BASE_ADDR, 0x0);
	lcd_vcbus_write(LCD_RGB_COEFF_ADDR, 0x400);

	switch (pconf->basic.lcd_type) {
	case LCD_LVDS:
		lcd_vcbus_setb(LCD_POL_CNTL_ADDR, 1, 0, 3);
		// refs to lcd_lvds.c@lcd_lvds_enable
		if (pconf->timing.act_timing.vsync_pol == pconf->timing.act_timing.hsync_pol)
			lcd_vcbus_setb(LCD_POL_CNTL_ADDR, 1, 1, 1);
		break;
	case LCD_VBYONE:
		if (pconf->timing.act_timing.hsync_pol)
			lcd_vcbus_setb(LCD_POL_CNTL_ADDR, 1, 0, 1);
		if (pconf->timing.act_timing.vsync_pol)
			lcd_vcbus_setb(LCD_POL_CNTL_ADDR, 1, 1, 1);
		break;
	case LCD_MIPI:
		//lcd_vcbus_setb(LCD_POL_CNTL_ADDR, 0x3, 0, 2);
		/*lcd_vcbus_write(LCD_POL_CNTL_ADDR,
		 *	(lcd_vcbus_read(LCD_POL_CNTL_ADDR) |
		 *	 ((0 << 2) | (vs_pol_adj << 1) | (hs_pol_adj << 0))));
		 */
		/*lcd_vcbus_write(LCD_POL_CNTL_ADDR, (lcd_vcbus_read(LCD_POL_CNTL_ADDR) |
		 *	 ((1 << LCD_TCON_DE_SEL) | (1 << LCD_TCON_VS_SEL) |
		 *	  (1 << LCD_TCON_HS_SEL))));
		 */
		break;
	default:
		break;
	}

	/* DE signal */
	lcd_vcbus_write(DE_HS_ADDR,    pconf->timing.de_hs_addr);
	lcd_vcbus_write(DE_HE_ADDR,    pconf->timing.de_he_addr);
	lcd_vcbus_write(DE_VS_ADDR,    pconf->timing.de_vs_addr);
	lcd_vcbus_write(DE_VE_ADDR,    pconf->timing.de_ve_addr);

	/* Hsync signal */
	lcd_vcbus_write(HSYNC_HS_ADDR, pconf->timing.hs_hs_addr);
	lcd_vcbus_write(HSYNC_HE_ADDR, pconf->timing.hs_he_addr);
	lcd_vcbus_write(HSYNC_VS_ADDR, pconf->timing.hs_vs_addr);
	lcd_vcbus_write(HSYNC_VE_ADDR, pconf->timing.hs_ve_addr);

	/* Vsync signal */
	lcd_vcbus_write(VSYNC_HS_ADDR, pconf->timing.vs_hs_addr);
	lcd_vcbus_write(VSYNC_HE_ADDR, pconf->timing.vs_he_addr);
	lcd_vcbus_write(VSYNC_VS_ADDR, pconf->timing.vs_vs_addr);
	lcd_vcbus_write(VSYNC_VE_ADDR, pconf->timing.vs_ve_addr);
}

static void lcd_venc_set_timing(struct aml_lcd_drv_s *pdrv)
{
	struct lcd_config_s *pconf = &pdrv->config;
	unsigned int hstart, hend, vstart, vend;
	unsigned int offset;
	unsigned int pre_vde, pre_de_vs, pre_de_ve, pre_de_hs, pre_de_he;

	hstart = pconf->timing.hstart;
	hend = pconf->timing.hend;
	vstart = pconf->timing.vstart;
	vend = pconf->timing.vend;
	offset = pdrv->data->offset_venc[pdrv->index];

	lcd_vcbus_write(ENCL_VIDEO_MAX_PXCNT + offset, pconf->timing.act_timing.h_period - 1);
	lcd_vcbus_write(ENCL_VIDEO_MAX_LNCNT + offset, pconf->timing.act_timing.v_period - 1);
	lcd_vcbus_write(ENCL_VIDEO_HAVON_BEGIN + offset, hstart);
	lcd_vcbus_write(ENCL_VIDEO_HAVON_END + offset,   hend);
	lcd_vcbus_write(ENCL_VIDEO_VAVON_BLINE + offset, vstart);
	lcd_vcbus_write(ENCL_VIDEO_VAVON_ELINE + offset, vend);
	if (pconf->basic.lcd_type == LCD_P2P ||
	    pconf->basic.lcd_type == LCD_MLVDS) {
		pre_vde = pconf->timing.pre_de_v ? pconf->timing.pre_de_v : 8;
		pre_de_vs = vstart - pre_vde;
		pre_de_ve = pconf->timing.act_timing.v_active + pre_de_vs;
		pre_de_hs = hstart + PRE_DE_DELAY;
		pre_de_he = pconf->timing.act_timing.h_active - 1 + pre_de_hs;
		lcd_vcbus_write(ENCL_VIDEO_V_PRE_DE_BLINE + offset, pre_de_vs);
		lcd_vcbus_write(ENCL_VIDEO_V_PRE_DE_ELINE + offset, pre_de_ve);
		lcd_vcbus_write(ENCL_VIDEO_H_PRE_DE_BEGIN + offset, pre_de_hs);
		lcd_vcbus_write(ENCL_VIDEO_H_PRE_DE_END + offset,   pre_de_he);
	}

	lcd_vcbus_write(ENCL_VIDEO_HSO_BEGIN + offset, pconf->timing.hs_hs_addr);
	lcd_vcbus_write(ENCL_VIDEO_HSO_END + offset,   pconf->timing.hs_he_addr);
	lcd_vcbus_write(ENCL_VIDEO_VSO_BEGIN + offset, pconf->timing.vs_hs_addr);
	lcd_vcbus_write(ENCL_VIDEO_VSO_END + offset,   pconf->timing.vs_he_addr);
	lcd_vcbus_write(ENCL_VIDEO_VSO_BLINE + offset, pconf->timing.vs_vs_addr);
	lcd_vcbus_write(ENCL_VIDEO_VSO_ELINE + offset, pconf->timing.vs_ve_addr);

	lcd_vcbus_write(ENCL_INBUF_CNTL1 + offset,
			(4 << 13) | (pconf->timing.act_timing.h_active - 1));

	lcd_venc_set_tcon(pdrv);
}

static void dual_set_a9(struct aml_lcd_drv_s *pdrv, u8 to_port, u8 en, u8 dual_mode)
{
	// uint16_t single_ha = pdrv->curr_dev->dev_cfg.timing.act_timing.h_active / 2;
	int left_hsize, right_hsize;
	if (pdrv->index != 0) {
		LCDPR("dual-port on VENC %u not supported", pdrv->index);
		return;
	}

	if (en == 0) {
		lcd_vcbus_write(VPU_VENC_RGN_CTRL, 0);
		return;
	}

	LCDPR("set dual split %s mode (%c->[%c] | %c->[%c])",
		(dual_mode == LCD_DUAL_PORT_L_R || dual_mode == LCD_DUAL_PORT_R_L) ?
			"Left-Right" : "Odd-Even",
		dual_mode == LCD_DUAL_PORT_L_R ? 'L' :
			(dual_mode == LCD_DUAL_PORT_R_L ? 'R' :
				(dual_mode == LCD_DUAL_PORT_O_E ? 'O' : 'E')), 'A' + pdrv->index,
		dual_mode == LCD_DUAL_PORT_L_R ? 'R' :
			(dual_mode == LCD_DUAL_PORT_R_L ? 'L' :
				(dual_mode == LCD_DUAL_PORT_O_E ? 'E' : 'O')), 'A' + to_port);

	if (pdrv->config.timing.act_timing.h_active % 2 ||
		pdrv->config.timing.act_timing.hsync_width % 2 ||
		pdrv->config.timing.act_timing.hsync_bp % 2 ||
		pdrv->config.timing.act_timing.hsync_fp % 2) {
		LCDPR("dual-port H-active/bp/fp/sync should be even value");
	}

	/* @reg: VPU_VENC_RGN_RSIZE 0x278a
	 * [23:12] reg_region0_size: output region0 size (hsize+1)/2
	 * [11:0]  reg_region1_size: output region0 size hsize - reg_region0_size
	 */
	left_hsize = (pdrv->config.timing.act_timing.h_active + 1) >> 1;
	right_hsize = pdrv->config.timing.act_timing.h_active - left_hsize;
	lcd_vcbus_write(VPU_VENC_RGN_RSIZE, ((right_hsize << 12) | left_hsize));
	/* @reg: VPU_DISP_WRAP_CTRL 0x278b
	 * [7:5] reg_difx_link_prot: 0:disp0 sync venc0 1:disp1 sync venc0 2:disp2 sync venc0
	 * [4] reg_splt2_mode: enable 1ppc->2ppc
	 * [2:0] reg_venc0_difx_link[2:0]:
	 */
	lcd_vcbus_write(VPU_DISP_WRAP_CTRL, (1 << 4)|(1 << 1)|(1 << 0));

	/* @reg: VPU_VENC_RGN_CTRL 0x2789
	 * [11:10] reg_gclk_ctrl: ram clk
	 * [9:8] reg_gclk_ctrl: logic clk
	 * [7] reg_rgn_swap: 0:(0-left, 1right) 1:converse
	 * [6] sw_rst: rgn_buffer soft reset
	 * [5] reg_sync_ctrl: rgn buffer related en signal sync enable
	 * [4] reg_sync_ctrl: vsync polarity 0:up edge 1:down edge
	 * [3] reg_vsync_ctrl: vsync polarity 0:positive 1:negative
	 * [2] reg_hsync_ctrl: hsync polarity 0:positive 1:negative
	 * [1] oe_sp_en: odd even split enable
	 * [0] rgn_en: vbo rgn_buffer enable
	 */
	lcd_vcbus_write(VPU_VENC_RGN_CTRL,
		((dual_mode == LCD_DUAL_PORT_L_R || dual_mode == LCD_DUAL_PORT_R_L) << 0 |
		 (dual_mode == LCD_DUAL_PORT_R_L || dual_mode == LCD_DUAL_PORT_O_E) << 7 |
		(dual_mode == LCD_DUAL_PORT_O_E || dual_mode == LCD_DUAL_PORT_E_O) << 1));
}

static void lcd_venc_set(struct aml_lcd_drv_s *pdrv)
{
	unsigned int reg_disp_viu_ctrl, offset;

	offset = pdrv->data->offset_venc[pdrv->index];
	lcd_vcbus_write(ENCL_VIDEO_EN + offset, 0);

	lcd_vcbus_write(ENCL_VIDEO_MODE + offset, 0x40); /* bit[15] shadown en */
	lcd_vcbus_write(ENCL_VIDEO_MODE_ADV + offset, 0x18); /* Sampling rate: 1 */
	lcd_vcbus_write(ENCL_VIDEO_FILT_CTRL + offset, 0x1000); /* bypass filter */
	lcd_vcbus_setb(ENCL_TST_EN + offset, 0x3f, 1, 6); /*vsync latch enable*/

	lcd_venc_set_timing(pdrv);

	lcd_vcbus_write(ENCL_VIDEO_RGBIN_CTRL + offset, 3);

	if (pdrv->status & LCD_STATUS_PRE_MUTE)
		lcd_venc_debug_test(pdrv, 8);//mute
	lcd_vcbus_write(ENCL_VIDEO_EN + offset, 1);

	switch (pdrv->index) {
	case 0:
		reg_disp_viu_ctrl = VPU_DISP_VIU0_CTRL;
		break;
	case 1:
		reg_disp_viu_ctrl = VPU_DISP_VIU1_CTRL;
		break;
	default:
		LCDERR("[%d]: %s: invalid drv_index\n",
			pdrv->index, __func__);
		return;
	}

	/*
	 * bit31: lvds enable
	 * bit30: vx1 enable
	 * bit29: hdmitx enable
	 * bit28: dsi_edp enable
	 */
	switch (pdrv->config.basic.lcd_type) {
	case LCD_LVDS:
		lcd_vcbus_write(reg_disp_viu_ctrl, (1 << 31) |
						(0 << 30) |
						(0 << 29) |
						(1 << 28));
		break;
	case LCD_VBYONE:
		lcd_vcbus_write(reg_disp_viu_ctrl, (0 << 31) |
						(1 << 30) |
						(0 << 29) |
						(0 << 28));
		break;
	case LCD_MIPI:
		lcd_vcbus_write(reg_disp_viu_ctrl, (0 << 31) |
						(0 << 30) |
						(0 << 29) |
						(1 << 28));
		break;
	case LCD_EDP:
		lcd_vcbus_write(reg_disp_viu_ctrl, (0 << 31) |
						(0 << 30) |
						(0 << 29) |
						(1 << 28));
		break;
	default:
		break;
	}
	lcd_vcbus_write(VPU_VENC_CTRL + offset, 2);

	if (pdrv->config.basic.lcd_type == LCD_MIPI &&
	    pdrv->config.control.mipi_cfg.multi_port_cfg & BIT(0)) {
		dual_set_a9(pdrv, 1, 1,
			(pdrv->config.control.mipi_cfg.multi_port_cfg >> 4) & 0Xf);
	}
}

static void lcd_venc_enable_ctrl(struct aml_lcd_drv_s *pdrv, int flag)
{
	unsigned int offset;

	offset = pdrv->data->offset_venc[pdrv->index];
	if (flag)
		lcd_vcbus_write(ENCL_VIDEO_EN + offset, 1);
	else
		lcd_vcbus_write(ENCL_VIDEO_EN + offset, 0);
}

static void lcd_venc_mute_set(struct aml_lcd_drv_s *pdrv, unsigned char flag)
{
	unsigned int offset;

	offset = pdrv->data->offset_venc[pdrv->index];

	lcd_venc_wait_vsync(pdrv);
	if (flag) {
		lcd_vcbus_write(ENCL_VIDEO_RGBIN_CTRL + offset, 3);
		lcd_vcbus_write(ENCL_TST_MDSEL + offset, 0);
		lcd_vcbus_write(ENCL_TST_Y + offset, 0);
		lcd_vcbus_write(ENCL_TST_CB + offset, 0);
		lcd_vcbus_write(ENCL_TST_CR + offset, 0);
		lcd_vcbus_setb(ENCL_TST_EN + offset, 1, 0, 1);
		lcd_vcbus_setb(ENCL_VIDEO_MODE_ADV + offset, 0, 3, 1);
	} else {
		lcd_vcbus_setb(ENCL_VIDEO_MODE_ADV + offset, 1, 3, 1);
		lcd_vcbus_setb(ENCL_TST_EN + offset, 0, 0, 1);
	}
}

static unsigned int lcd_venc_get_encl_line_cnt(struct aml_lcd_drv_s *pdrv)
{
	unsigned int offset, cnt;

	if (!pdrv)
		return 0;

	offset = pdrv->data->offset_venc[pdrv->index];

	cnt = lcd_vcbus_getb(VPU_VENCP_STAT + offset, 16, 13);
	return cnt;
}

static void lcd_venc_reg_dump(struct aml_lcd_drv_s *pdrv)
{
	int i;
	unsigned int *reg_table = NULL, size_encl = 0;
	unsigned int encl_0_reg[] = {
		VPU_VIU_VENC_MUX_CTRL,
		ENCL_VIDEO_EN,
		ENCL_VIDEO_MODE,
		ENCL_VIDEO_MODE_ADV,
		ENCL_VIDEO_MAX_PXCNT,
		ENCL_VIDEO_MAX_LNCNT,
		ENCL_VIDEO_HAVON_BEGIN,
		ENCL_VIDEO_HAVON_END,
		ENCL_VIDEO_VAVON_BLINE,
		ENCL_VIDEO_VAVON_ELINE,
		ENCL_VIDEO_HSO_BEGIN,
		ENCL_VIDEO_HSO_END,
		ENCL_VIDEO_VSO_BEGIN,
		ENCL_VIDEO_VSO_END,
		ENCL_VIDEO_VSO_BLINE,
		ENCL_VIDEO_VSO_ELINE,
		ENCL_VIDEO_RGBIN_CTRL,
		LCD_GAMMA_CNTL_PORT0,
		LCD_RGB_BASE_ADDR,
		LCD_RGB_COEFF_ADDR,
		LCD_POL_CNTL_ADDR,
		LCD_DITH_CNTL_ADDR,
		VPU_DISP_VIU0_CTRL,
		VPU_VENC_CTRL,
		ENCL_INBUF_CNTL0,
		ENCL_INBUF_CNTL1
	};
	unsigned int offset;

	offset = pdrv->data->offset_venc[pdrv->index];
	reg_table = encl_0_reg;
	size_encl = ARRAY_SIZE(encl_0_reg);
	for (i = 0; i < size_encl; i++)
		printf("vcbus [0x%04x] = 0x%08x\n",
			reg_table[i] + offset, lcd_vcbus_read(reg_table[i] + offset));
}

static void lcd_venc_save_bootctrl_to_reg(struct aml_lcd_drv_s *pdrv)
{
	unsigned int val = 0;
	unsigned int offset = pdrv->data->offset_venc[pdrv->index];

	val = (pdrv->boot_ctrl.init_level & 0xf) | (!!(pdrv->status & LCD_STATUS_IF_ON) << 4)
		| ((pdrv->boot_ctrl.dccd_flag & 0x1) << 5) | (0xa << 9);
	lcd_vcbus_write(L_STH1_HS_ADDR + offset, val);

	val = pdrv->boot_ctrl.frame_rate & 0x1fff;
		lcd_vcbus_write(L_STH1_HS_ADDR + offset + 4, val);

	val = (pdrv->boot_ctrl.lcd_type & 0xf) | ((pdrv->boot_ctrl.clk_mode & 0xf) << 4)
		| ((pdrv->boot_ctrl.ppc & 0x3) << 8)
		| ((pdrv->boot_ctrl.bl_state & 0x1) << 11);
	lcd_vcbus_write(L_STH1_HS_ADDR + offset + 8, val);

	val = pdrv->boot_ctrl.advanced_flag & 0xff;
	lcd_vcbus_write(L_STH1_HS_ADDR + offset + 12, val);
}

int lcd_venc_op_init_a9(struct lcd_venc_op_s *venc_op)
{
	if (!venc_op)
		return -1;

	venc_op->wait_vsync = lcd_venc_wait_vsync;
	venc_op->get_max_lcnt = lcd_venc_get_max_lint_cnt;
	venc_op->venc_debug_test = lcd_venc_debug_test;
	venc_op->venc_probe_cursor = lcd_venc_probe_cursor;
	venc_op->venc_set_timing = lcd_venc_set_timing;
	venc_op->venc_set = lcd_venc_set;
	venc_op->venc_enable = lcd_venc_enable_ctrl;
	venc_op->mute_set = lcd_venc_mute_set;
	venc_op->get_encl_line_cnt = lcd_venc_get_encl_line_cnt;
	venc_op->venc_reg_dump = lcd_venc_reg_dump;
	venc_op->bootctrl_to_regs = lcd_venc_save_bootctrl_to_reg;
	venc_op->window_attr_set = lcd_venc_window_attr_set;

	return 0;
};
#endif
