// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2021 Amlogic, Inc. All rights reserved.
 */

#include <malloc.h>
#include <amlogic/media/vout/lcd/aml_lcd.h>
#include "../lcd_reg.h"
#include "../lcd_common.h"
#include "lcd_clk_config.h"
#include "lcd_clk_ctrl.h"
#include "lcd_clk_utils.h"
#include <amlogic/clk_measure.h>
#include "../connectors/lcd_connector.h"

#ifdef CONFIG_MESON_A9
/*
 *  DSI_VCO(2.8G) --- 3od --- dsi_phy_clk(2div)
 *        |                        '--- host clk
 *        '--- /enc_xd --- encl_clk
 * -----------------------------------------------------
 * SW arch:
 * PLL: 1.4~2.8G VCO, 0 od
 * 3 od + dsi_phy_clk(2div) as special for phy div
 */

static void lcd_pll_frac_set_a9(struct aml_lcd_drv_s *pdrv, unsigned int frac)
{
	struct lcd_clk_config_s *cconf;
	unsigned int reg, val;
	int offset;

	cconf = get_lcd_clk_config(pdrv);
	if (!cconf)
		return;
	offset = cconf->data->pll_data[0]->pll_offset;
	reg = ANACTRL_TCON_PLL0_CNTL2;
	val = lcd_ana_read(reg);
	lcd_ana_setb(reg, frac, 0, 17);
	udelay(10);
	lcd_ana_setb(ANACTRL_TCON_PLL_VLOCK + offset, 1, 4, 1);
	udelay(10);
	lcd_ana_setb(ANACTRL_TCON_PLL_VLOCK + offset, 0, 4, 1);
	if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
		LCDPR("[%d]: %s: reg 0x%x: 0x%08x->0x%08x\n",
		      pdrv->index, __func__, reg, val, lcd_ana_read(reg));
	}
	LCDPR("[%d]: %s: pll_frac=0x%x\n", pdrv->index, __func__, frac);
	if (cconf->pll_mode & LCD_PLL_MODE_DUAL_PLL) {
		offset = cconf->data->pll_data[1]->pll_offset;
		reg = ANACTRL_GP2PLL_CTRL1 + offset;
		val = lcd_ana_read(reg);
		lcd_ana_setb(reg, cconf->pll_config[1].pll_frac, 0, 17);
		if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
			LCDPR("[%d]: %s: reg 0x%x: 0x%08x->0x%08x\n",
				pdrv->index, __func__, reg, val, lcd_ana_read(reg));
		}
		LCDPR("[%d]: %s: pixel pll_frac=0x%x\n", pdrv->index,
		      __func__, cconf->pll_config[1].pll_frac);
	}
}

static void lcd_set_pll_ss_a9(struct aml_lcd_drv_s *pdrv, unsigned int ss_flag)
{
	struct lcd_clk_config_s *cconf;
	unsigned int pll_ctrl0, pll_ctrl1;
	char prt_str[64] = {0};
	int len = 0;
	int offset;

	cconf = get_lcd_clk_config(pdrv);
	if (!cconf)
		return;

	offset = cconf->data->pll_data[0]->pll_offset;
	pll_ctrl0 = lcd_ana_read(ANACTRL_TCON_PLL0_CNTL0 + offset);
	pll_ctrl1 = lcd_ana_read(ANACTRL_TCON_PLL0_CNTL1 + offset);

	if (ss_flag & LCD_SSC_LEVEL) {
		pll_ctrl0 &= ~((1 << 19) | (0xff << 20));
		if (cconf->ss_level > 0) {
			cconf->ss_en = 1;
			pll_ctrl0 |= ((1 << 19) | (cconf->ss_dep_sel << 24) |
					(cconf->ss_str_m << 20));
			len += sprintf(prt_str + len, "level: %d, %dppm",
					cconf->ss_level, cconf->ss_ppm);
		} else {
			cconf->ss_en = 0;
			len += sprintf(prt_str + len, "disable");
		}
	}

	if (ss_flag & LCD_SSC_FREQ) {
		pll_ctrl1 &= ~(0x7 << 15);
		pll_ctrl1 |= (cconf->ss_freq << 15);
		len += sprintf(prt_str + len, "%sfreq=%d", len ? ", " : "", cconf->ss_freq);
	}

	if (ss_flag & LCD_SSC_MODE) {
		pll_ctrl1 &= ~(0x3 << 23); /* ss_mode */
		pll_ctrl1 |= (cconf->ss_mode << 23);
		len += sprintf(prt_str + len, "%smode=%d", len ? ", " : "", cconf->ss_mode);
	}

	lcd_ana_write(ANACTRL_TCON_PLL0_CNTL1 + offset, pll_ctrl1);
	lcd_ana_write(ANACTRL_TCON_PLL0_CNTL0 + offset, pll_ctrl0);

	LCDPR("[%d]: set ssc: %s\n", pdrv->index, len ? prt_str : "none");
}

static void lcd_pll_ss_enable_a9(struct aml_lcd_drv_s *pdrv, int status)
{
	struct lcd_clk_config_s *cconf;
	unsigned int pll_ctrl0;
	unsigned int flag;
	int offset;

	cconf = get_lcd_clk_config(pdrv);
	if (!cconf)
		return;

	offset = cconf->data->pll_data[0]->pll_offset;
	pll_ctrl0 = lcd_ana_read(ANACTRL_TCON_PLL0_CNTL0 + offset);
	pll_ctrl0 &= ~((1 << 19) | (1 << 19) | (0xff << 20));

	if (status) {
		if (cconf->ss_level > 0)
			flag = 1;
		else
			flag = 0;
	} else {
		flag = 0;
	}

	if (flag) {
		cconf->ss_en = 1;
		pll_ctrl0 |= ((1 << 19) | (cconf->ss_dep_sel << 24) |
					(cconf->ss_str_m << 20));
		LCDPR("[%d]: pll ss enable: level: %d, %dppm\n",
		      pdrv->index, cconf->ss_level, cconf->ss_ppm);
	} else {
		cconf->ss_en = 0;
		LCDPR("[%d]: pll ss disable\n", pdrv->index);
	}
	lcd_ana_write(ANACTRL_TCON_PLL0_CNTL0 + offset, pll_ctrl0);
}

static unsigned int tcon_div[][4] = {
	/* vx1pll_div214h, tcon_bypass_en, vx1pll_clk1x_selh */
	{0, 0, 1, 0x80},  /* div1 */
	{0, 0, 0, 0x80},  /* div2 */
	{1, 0, 0, 0x80},  /* div4 */
	{0, 0, 0, 0x0f},  /* div8 */
	{1, 0, 0, 0x0f},  /* div16 */
};

static void lcd_set_pll_a9(struct aml_lcd_drv_s *pdrv)
{
	struct lcd_clk_config_s *cconf;
	unsigned int pll_ctrl0, pll_ctrl1, pll_ctrl2, pll_ctrl3;
	unsigned int tcon_div_sel;
	int ret, cnt = 0;
	int offset;

	if (lcd_debug_print_flag & LCD_DBG_PR_ADV2)
		LCDPR("[%d]: %s\n", pdrv->index, __func__);

	cconf = get_lcd_clk_config(pdrv);
	if (!cconf)
		return;

	tcon_div_sel = cconf->pll_tcon_div_sel;
	pll_ctrl0 = 0x13e00 | (cconf->pll_config[0].pll_m << 0);
	pll_ctrl2 = cconf->pll_config[0].pll_frac |
		(1 << 20) |/*pll sdm_en*/
		(tcon_div[tcon_div_sel][0] << 24) | /*div2_4_sel*/
		(tcon_div[tcon_div_sel][2] << 25) | /*clk1x_sel*/
		(tcon_div[tcon_div_sel][1] << 31);
	pll_ctrl3 =
		(cconf->pll_config[0].pll_od1_sel << 10) |
		(cconf->pll_config[0].pll_od2_sel << 12) |
		(cconf->pll_config[0].pll_od3_sel << 14) |
		(1 << 8) | /*tcon_od_en*/
		(tcon_div[tcon_div_sel][3]); /*tcon_bypass_en*/
	do {
		offset = cconf->data->pll_data[0]->pll_offset;
		lcd_ana_write(ANACTRL_TCON_PLL0_CNTL0 + offset, pll_ctrl0);
		lcd_ana_write(ANACTRL_TCON_PLL0_CNTL1 + offset, 0x94401545);
		lcd_ana_write(ANACTRL_TCON_PLL0_CNTL2 + offset, pll_ctrl2);
		lcd_ana_write(ANACTRL_TCON_PLL0_CNTL3 + offset, pll_ctrl3);
		lcd_ana_setb(ANACTRL_TCON_PLL0_CNTL0 + offset, 1, 28, 1);
		udelay(10);
		lcd_ana_setb(ANACTRL_TCON_PLL0_CNTL0 + offset, 1, 30, 1);
		udelay(80);
		lcd_ana_setb(ANACTRL_TCON_PLL0_CNTL0 + offset, 1, 29, 1);
		offset = pdrv->index << 2;
		ret = lcd_pll_wait_lock(cconf->pll_config[0].pll_id,
					ANACTRL_TCON_PLL0_STS + offset, 31);
	} while (ret && ++cnt < PLL_RETRY_MAX);
	if (ret)
		LCDERR("[%d]: vx1 pll lock failed\n", pdrv->index);

	/* set load to 0 */
	lcd_ana_setb(ANACTRL_TCON_PLL_VLOCK + offset, 0, 4, 1);
	/* select ANACTRL_TCON_PLL_VLOCK[4] as load */
	lcd_ana_setb(ANACTRL_TCON_PLL_VLOCK + offset, 0, 3, 1);
	/* enable load en*/
	lcd_ana_setb(ANACTRL_TCON_PLL0_CNTL0 + offset, 1, 14, 1);

	if (cconf->pll_mode & LCD_PLL_MODE_DUAL_PLL) {
		cnt = 0;
		ret = 0;
		do {
			offset = cconf->data->pll_data[1]->pll_offset;
			pll_ctrl0 = 0xe00a000 |
				((cconf->pll_config[1].pll_od1_sel & 0x3) << 19) |
				(cconf->pll_config[1].pll_m & 0x1ff) |
				((cconf->pll_config[1].pll_od2_sel & 0x3) << 22);
			pll_ctrl1 = 0x11480000 | (cconf->pll_config[1].pll_frac & 0x1ffff);
			lcd_ana_write(ANACTRL_TCON_PLL2_CNTL0 + offset, pll_ctrl0);
			lcd_ana_write(ANACTRL_TCON_PLL2_CNTL1 + offset, pll_ctrl1);
			lcd_ana_write(ANACTRL_TCON_PLL2_CNTL2 + offset, 0x12001230);
			lcd_ana_setb(ANACTRL_TCON_PLL2_CNTL0 + offset, 1, 28, 1);
			udelay(20);
			lcd_ana_setb(ANACTRL_TCON_PLL2_CNTL0 + offset, 1, 29, 1);
			udelay(20);
			lcd_ana_setb(ANACTRL_TCON_PLL2_CNTL0 + offset, 1, 30, 1);
			offset = pdrv->index << 2;
			ret = lcd_pll_wait_lock(cconf->pll_config[1].pll_id,
						ANACTRL_TCON_PLL2_STS + offset, 31);
		} while (ret && ++cnt < PLL_RETRY_MAX);
		if (ret)
			LCDERR("[%d]: gp2 pll lock failed\n", pdrv->index);
	}

	if (cconf->ss_level > 0)
		lcd_set_pll_ss_a9(pdrv, (LCD_SSC_LEVEL | LCD_SSC_FREQ | LCD_SSC_MODE));
}

static void lcd_clk_set_a9(struct aml_lcd_drv_s *pdrv)
{
	lcd_set_pll_a9(pdrv);

	// reset for clk msr
	lcd_ana_setb(ANACTRL_DIF_PHY_CNTL9, 0x1, 25, 1);
	if (pdrv->index == 0) {
		lcd_ana_setb(ANACTRL_DIF_PHY_CNTL9, 0x1, 16, 1);
		udelay(5);
		lcd_ana_setb(ANACTRL_DIF_PHY_CNTL9, 0x0, 16, 1);

	} else {
		lcd_ana_setb(ANACTRL_DIF_PHY_CNTL9, 0x1, 20, 1);
		udelay(5);
		lcd_ana_setb(ANACTRL_DIF_PHY_CNTL9, 0x0, 20, 1);

	}
	lcd_ana_setb(ANACTRL_DIF_PHY_CNTL9, 0x0, 25, 1);
}

static void lcd_set_vid_pll_div_a9(struct aml_lcd_drv_s *pdrv)
{
	struct lcd_clk_config_s *cconf;
	unsigned int shift_val, shift_sel;
	int offset = pdrv->index ? (ANACTRL_VID_PLL1_CLK_DIV - ANACTRL_VID_PLL0_CLK_DIV) : 0;

	cconf = get_lcd_clk_config(pdrv);
	if (!cconf || cconf->pll_mode & LCD_PLL_MODE_DUAL_PLL)
		return;

	if (lcd_debug_print_flag & LCD_DBG_PR_ADV2)
		LCDPR("[%d]: %s\n", pdrv->index, __func__);

	lcd_clk_setb(CLKCTRL_HDMI_CLK_CTRL, 1, 15, 1);
	udelay(5);

	/* Disable the div output clock */
	lcd_ana_setb(ANACTRL_VID_PLL0_CLK_DIV + offset, 0, 19, 1);
	lcd_ana_setb(ANACTRL_VID_PLL0_CLK_DIV + offset, 0, 15, 1);

	if (cconf->pll_mode & LCD_PLL_MODE_DUAL_PLL) {
		if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL)
			LCDPR("no need to set clk divider for gp2\n\n");
		return;
	}

	if (cconf->data->pll_data[0]->div_sel_max == CLK_DIV_SEL_1 ||
	    cconf->pll_config[0].div_sel > cconf->data->pll_data[0]->div_sel_max ||
	    cconf->pll_config[0].div_sel >= ARRAY_SIZE(lcd_clk_div_table)) {
		LCDERR("[%d]: invalid clk divider\n", pdrv->index);
		return;
	}

	shift_val = lcd_clk_div_table[cconf->pll_config[0].div_sel].shift_val;
	shift_sel = lcd_clk_div_table[cconf->pll_config[0].div_sel].shift_sel;

	if (shift_val == 0xffff) { /* if divide by 1 */
		lcd_ana_setb(ANACTRL_VID_PLL0_CLK_DIV + offset, 1, 18, 1);
	} else {
		lcd_ana_setb(ANACTRL_VID_PLL0_CLK_DIV + offset, 0, 15, 3);
		lcd_ana_setb(ANACTRL_VID_PLL0_CLK_DIV + offset, 0, 0, 14);

		lcd_ana_setb(ANACTRL_VID_PLL0_CLK_DIV + offset, shift_sel, 16, 2);
		lcd_ana_setb(ANACTRL_VID_PLL0_CLK_DIV + offset, 1, 15, 1);
		lcd_ana_setb(ANACTRL_VID_PLL0_CLK_DIV + offset, (shift_val & 0x7fff), 0, 15);
		lcd_ana_setb(ANACTRL_VID_PLL0_CLK_DIV + offset, 0, 15, 1);
	}
	/* Enable the final output clock */
	lcd_ana_setb(ANACTRL_VID_PLL0_CLK_DIV + offset, 1, 19, 1);
}

static void lcd_set_vclk_crt_a9(struct aml_lcd_drv_s *pdrv)
{
	struct lcd_clk_config_s *cconf;
	unsigned int reg_vid_clk_div, reg_vid_clk_ctrl;
	unsigned int encl_clk_sel_bit, encl_clk_sel;
	unsigned int encl_gate_bit;


	if (lcd_debug_print_flag & LCD_DBG_PR_ADV2)
		LCDPR("%s\n", __func__);
	cconf = get_lcd_clk_config(pdrv);
	if (!cconf)
		return;
	switch (cconf->pll_config->pll_id) {
	case 1:
		reg_vid_clk_div = CLKCTRL_VIID_CLK_DIV;
		reg_vid_clk_ctrl = CLKCTRL_VIID_CLK_CTRL;
		encl_clk_sel_bit = 8;
		encl_clk_sel = 8;
		encl_gate_bit = 11;
		break;
	case 0:
	default:
		reg_vid_clk_div = CLKCTRL_VID_CLK_DIV;
		reg_vid_clk_ctrl = CLKCTRL_VID_CLK_CTRL;
		encl_clk_sel_bit = 12;
		encl_clk_sel = 0;
		encl_gate_bit = 10;
		break;
	}

	lcd_clk_setb(reg_vid_clk_ctrl, 0, VCLK2_EN, 1);
	udelay(2);

	lcd_set_vid_pll_div_a9(pdrv);

#ifdef CONFIG_AML_LCD_PXP
	/* setup the XD divider value */
	lcd_clk_setb(reg_vid_clk_div, 0, VCLK2_XD, 8);
	udelay(5);
	/* select vid_pll_clk */
	lcd_clk_setb(reg_vid_clk_ctrl, 6, VCLK2_CLK_IN_SEL, 3);
#else
	/* setup the XD divider value */
	lcd_clk_setb(reg_vid_clk_div, (cconf->xd - 1), VCLK2_XD, 8);
	udelay(5);

	/* select dsi_pll_clk */
	lcd_clk_setb(reg_vid_clk_ctrl, cconf->data->vclk_sel, VCLK2_CLK_IN_SEL, 3);
#endif
	/* [15:12] encl_clk_sel, select vclk2_div1 */
	lcd_clk_setb(CLKCTRL_VIID_CLK_DIV, encl_clk_sel, encl_clk_sel_bit, 4);
	/* release vclk2_div_reset and enable vclk2_div */
	lcd_clk_setb(reg_vid_clk_div, 1, VCLK2_XD_EN, 2);
	udelay(5);
	lcd_clk_setb(reg_vid_clk_ctrl, 1, VCLK2_DIV1_EN, 1);
	lcd_clk_setb(reg_vid_clk_ctrl, 1, VCLK2_SOFT_RST, 1);
	lcd_clk_setb(reg_vid_clk_ctrl, 0, VCLK2_SOFT_RST, 1);
	udelay(5);

	/* enable CTS_ENCL clk gate */
	lcd_clk_setb(CLKCTRL_VID_CLK_CTRL2, 1, encl_gate_bit, 1);

	lcd_clk_setb(reg_vid_clk_ctrl, 1, VCLK2_EN, 1);

	if (pdrv->config.basic.lcd_type == LCD_MIPI &&
	    pdrv->config.control.mipi_cfg.multi_port_cfg & BIT(0)) {
		lcd_clk_setb(CLKCTRL_VID_CLK_CTRL2, 3, 16, 1);
		lcd_clk_setb(CLKCTRL_VIID_CLK_DIV, 1, 8, 4); // seltect enc1 clk source
		lcd_clk_setb(CLKCTRL_VID_CLK_CTRL, 1, 1, 1); // enable div2 gate
	}
	udelay(2);
}

static void lcd_clk_disable_a9(struct aml_lcd_drv_s *pdrv)
{
	int offset = pdrv->index ? (ANACTRL_TCON_PLL1_CNTL0 - ANACTRL_TCON_PLL0_CNTL0) : 0;

	lcd_clk_setb(CLKCTRL_VIID_CLK_CTRL, 0, VCLK2_EN, 1);
	lcd_clk_setb(CLKCTRL_VID_CLK_CTRL2, 0, ENCL_GATE_VCLK, 1);

	lcd_ana_write(ANACTRL_DSIPLL_CTRL0 + offset, 0x0);
	lcd_ana_write(ANACTRL_DSIPLL_CTRL1 + offset, 0x0);
	lcd_ana_write(ANACTRL_DSIPLL_CTRL2 + offset, 0x0);
	lcd_ana_write(ANACTRL_DSIPLL_CTRL3 + offset, 0x0);
}

static int lcd_prbs_test_a9(struct aml_lcd_drv_s *pdrv, unsigned int ms,
			    unsigned int mode_flag)
{
	LCDPR("TODO\n");
	return 0;
}

static void lcd_clk_reg_dump_a9(struct aml_lcd_drv_s *pdrv)
{
	int i;
	unsigned int *table = NULL, size = 0;
	unsigned int pll_reg_table[] = {
		ANACTRL_TCON_PLL0_CNTL0,
		ANACTRL_TCON_PLL0_CNTL1,
		ANACTRL_TCON_PLL0_CNTL2,
		ANACTRL_TCON_PLL0_CNTL3,
		ANACTRL_TCON_PLL0_STS,
		ANACTRL_VID_PLL0_CLK_DIV
	};
	unsigned int clk_reg_table[][2] = {
		{
			CLKCTRL_VID_CLK_CTRL,
			CLKCTRL_VID_CLK_DIV,
		},
		{
			CLKCTRL_VIID_CLK_CTRL,
			CLKCTRL_VIID_CLK_DIV,
		}
	};
	unsigned int key_clk_msr_id[] = {52, 53};
	struct lcd_clk_config_s *cconf;
	int offset;

	cconf = get_lcd_clk_config(pdrv);
	if (!cconf)
		return;
	offset = cconf->data->pll_data[0]->pll_offset;

	table = pll_reg_table;
	size = ARRAY_SIZE(pll_reg_table);
	for (i = 0; i < size; i++)
		printf("pll [0x%08x] = 0x%08x\n", table[i], lcd_ana_read(table[i] + offset));

	table = clk_reg_table[pdrv->index];
	size = ARRAY_SIZE(clk_reg_table[pdrv->index]);
	for (i = 0; i < size; i++)
		printf("clk [0x%08x] = 0x%08x\n", table[i], lcd_clk_read(table[i]));
	printf("clk [0x%08x] = 0x%08x\n",
			CLKCTRL_VID_CLK_CTRL2, lcd_clk_read(CLKCTRL_VID_CLK_CTRL2));


	table = key_clk_msr_id;
	size = ARRAY_SIZE(key_clk_msr_id);
	for (i = 0; i < size; i++)
		clk_msr(table[i]);

}

static void lcd_clk_generate_a9(struct aml_lcd_drv_s *pdrv)
{
	struct lcd_clk_config_s *cconf;

	cconf = get_lcd_clk_config(pdrv);
	if (!cconf)
		return;
	if (pdrv->config.basic.lcd_type == LCD_MIPI) {
		LCDPR("[%d]: %s: dual pll config\n", pdrv->index, __func__);
		if (cconf->pll_conf_num < 2) {
			LCDERR("[%d]: %s: clk_conf_num %d error\n",
				pdrv->index, __func__, cconf->pll_conf_num);
			return;
		}
		cconf->pll_mode |= LCD_PLL_MODE_DUAL_PLL;
		cconf->data->vclk_sel = pdrv->index ? 3 : 1;
		lcd_clk_generate_dft(pdrv);
	} else {
		cconf->pll_mode &= ~LCD_PLL_MODE_DUAL_PLL;
		cconf->data->vclk_sel = pdrv->index ? 7 : 2;
		lcd_clk_generate_od_div_5_7(pdrv);
	}
}

static struct lcd_pll_data_s lcd_tcon_pll_data = {
	.pll_od_fb = 0,
	.pll_div_0p5_en = 1,
	.pll_m_max = 511,
	.pll_m_min = 2,
	.pll_n_max = 1,
	.pll_n_min = 1,
	.pll_frac_range = (1 << 17),
	.pll_frac_sign_bit = 18,
	.pll_od_sel_max = 4,
	.pll_ref_fmax = 25000000,
	.pll_ref_fmin = 5000000,
	.pll_vco_fmax = 6000000000ULL,
	.pll_vco_fmin = 3000000000ULL,
	.pll_out_fmax = 4250000000ULL,
	.pll_out_fmin = 5859375,
	.od_model = LCD_OD_MODEL_3DIV_2P5_1517_2P5,
	.have_tcon_div = 1,
	.div_in_fmax = 4250000000ULL,
	.div_out_fmax = 840000000,
	.div_sel_max = CLK_DIV_SEL_15,
};

static struct lcd_pll_data_s lcd_pix_pll_data = {
	.pll_od_fb = 0,
	.pll_div_0p5_en = 1,
	.pll_m_max = 511,
	.pll_m_min = 2,
	.pll_n_max = 1,
	.pll_n_min = 1,
	.pll_frac_range = (1 << 17),
	.pll_frac_sign_bit = 18,
	.pll_od_sel_max = 3,
	.pll_ref_fmax = 25000000,
	.pll_ref_fmin = 5000000,
	.pll_vco_fmax = 3200000000ULL,
	.pll_vco_fmin = 1600000000ULL,
	.pll_out_fmax = 3200000000ULL,
	.pll_out_fmin = 200000000,
	.od_model = LCD_OD_MODEL_1DIV_2P5,
	.have_tcon_div = 0,
	.div_in_fmax = 3200000000ULL,
	.div_out_fmax = 840000000,
	.div_sel_max = CLK_DIV_SEL_1,
};

static struct lcd_pll_data_s lcd_tcon_pll_data_1 = {
	.pll_od_fb = 0,
	.pll_div_0p5_en = 1,
	.pll_m_max = 511,
	.pll_m_min = 2,
	.pll_n_max = 1,
	.pll_n_min = 1,
	.pll_frac_range = (1 << 17),
	.pll_frac_sign_bit = 18,
	.pll_od_sel_max = 4,
	.pll_ref_fmax = 25000000,
	.pll_ref_fmin = 5000000,
	.pll_vco_fmax = 6000000000ULL,
	.pll_vco_fmin = 3000000000ULL,
	.pll_out_fmax = 4250000000ULL,
	.pll_out_fmin = 5859375,
	.od_model = LCD_OD_MODEL_3DIV_2P5_1517_2P5,
	.have_tcon_div = 1,
	.div_in_fmax = 4250000000ULL,
	.div_out_fmax = 840000000,
	.div_sel_max = CLK_DIV_SEL_15,
};

static struct lcd_pll_data_s lcd_pix_pll_data_1 = {
	.pll_od_fb = 0,
	.pll_div_0p5_en = 1,
	.pll_m_max = 511,
	.pll_m_min = 2,
	.pll_n_max = 1,
	.pll_n_min = 1,
	.pll_frac_range = (1 << 17),
	.pll_frac_sign_bit = 18,
	.pll_od_sel_max = 3,
	.pll_ref_fmax = 25000000,
	.pll_ref_fmin = 5000000,
	.pll_vco_fmax = 3200000000ULL,
	.pll_vco_fmin = 1600000000ULL,
	.pll_out_fmax = 3200000000ULL,
	.pll_out_fmin = 200000000,
	.od_model = LCD_OD_MODEL_1DIV_2P5,
	.have_tcon_div = 0,
	.div_in_fmax = 3200000000ULL,
	.div_out_fmax = 840000000,
	.div_sel_max = CLK_DIV_SEL_1,
};

static struct lcd_clk_data_s lcd_clk_data_a9_0 = {
	.pll_data[0] = &lcd_tcon_pll_data,
	.pll_data[1] = &lcd_pix_pll_data,
	.xd_out_fmax = 2800000000,
	.phy_clk_location = 1,

	.vclk_sel = 2, // dsi_pll_clk
	.enc_clk_msr_id = 53,
	.fifo_clk_msr_id = 71,

	.xd_max = 128,
	.phy_div_max = 128,

	.ss_support = 1,
	.ss_level_max = 60,
	.ss_freq_max = 6,
	.ss_mode_max = 2,
	.ss_dep_base = 500, //ppm
	.ss_dep_sel_max = 12,
	.ss_str_m_max = 10,

	.clk_parameter_init = NULL,
	.clk_generate_parameter = lcd_clk_generate_a9,
	.pll_frac_generate = NULL,
	.set_ss = NULL,
	.clk_ss_enable = lcd_pll_ss_enable_a9,
	.pll_frac_set = lcd_pll_frac_set_a9,
	.clk_set = lcd_clk_set_a9,
	.vclk_crt_set = lcd_set_vclk_crt_a9,
	.clk_disable = lcd_clk_disable_a9,
	.clktree_set = NULL,
	.clk_config_init_print = lcd_clk_config_init_print_dft,
	.clk_config_print = lcd_clk_config_print_dft,
	.clk_reg_print = lcd_clk_reg_dump_a9,
	.prbs_test = lcd_prbs_test_a9,
};

static struct lcd_clk_data_s lcd_clk_data_a9_1 = {
	.pll_data[0] = &lcd_tcon_pll_data_1,
	.pll_data[1] = &lcd_pix_pll_data_1,
	.xd_out_fmax = 2800000000,
	.phy_clk_location = 1,

	.vclk_sel = 7, // dsi_pll_clk
	.enc_clk_msr_id = 54,
	.fifo_clk_msr_id = 72,

	.xd_max = 128,
	.phy_div_max = 128,

	.ss_support = 1,
	.ss_level_max = 60,
	.ss_freq_max = 6,
	.ss_mode_max = 2,
	.ss_dep_base = 500, //ppm
	.ss_dep_sel_max = 12,
	.ss_str_m_max = 10,

	.clk_parameter_init = NULL,
	.clk_generate_parameter = lcd_clk_generate_a9,
	.pll_frac_generate = NULL,
	.set_ss = NULL,
	.clk_ss_enable = lcd_pll_ss_enable_a9,
	.pll_frac_set = lcd_pll_frac_set_a9,
	.clk_set = lcd_clk_set_a9,
	.vclk_crt_set = lcd_set_vclk_crt_a9,
	.clk_disable = lcd_clk_disable_a9,
	.clktree_set = NULL,
	.clk_config_init_print = lcd_clk_config_init_print_dft,
	.clk_config_print = lcd_clk_config_print_dft,
	.clk_reg_print = lcd_clk_reg_dump_a9,
	.prbs_test = lcd_prbs_test_a9,
};

struct lcd_clk_config_s *lcd_clk_config_chip_init_a9(struct aml_lcd_drv_s *pdrv)
{
	struct lcd_clk_config_s *cconf;
	// unsigned int size;

	if (!pdrv)
		return NULL;

	if (!pdrv->clk_conf) {
		cconf = (struct lcd_clk_config_s *)malloc(sizeof(struct lcd_clk_config_s));
		if (!cconf) {
			LCDERR("[%d]: %s: Not enough memory\n", pdrv->index, __func__);
			return NULL;
		}
	} else {
		cconf = (struct lcd_clk_config_s *)pdrv->clk_conf;
	}

	memset(cconf, 0, sizeof(struct lcd_clk_config_s));

	cconf->pll_conf_num = 2;
	//if (pdrv->config.basic.lcd_type == LCD_MIPI)
	//	cconf->pll_conf_num = 2;
	//else
	//	cconf->pll_conf_num = 1;

	//size = cconf->pll_conf_num * sizeof(struct lcd_pll_config_s);
	//cconf->pll_config = (struct lcd_pll_config_s *)malloc(size);
	//if (!cconf->pll_config) {
	//	LCDERR("[%d]: %s: Not enough memory for pll config\n", pdrv->index, __func__);
	//	free(cconf);
	//	return NULL;
	//}
	//memset(cconf->pll_config, 0, size);
	if (pdrv->index == 0) {
		cconf->data = &lcd_clk_data_a9_0;
		cconf->pll_config->pll_id = 0;
		cconf->data->pll_data[0]->pll_offset = 0;
		cconf->data->pll_data[1]->pll_offset = 0;
	} else if (pdrv->index == 1) {
		cconf->data = &lcd_clk_data_a9_1;
		cconf->pll_config->pll_id = 1;
		cconf->data->pll_data[0]->pll_offset =
			ANACTRL_TCON_PLL1_CNTL0 - ANACTRL_TCON_PLL0_CNTL0;
		cconf->data->pll_data[1]->pll_offset =
			ANACTRL_TCON_PLL3_CNTL0 - ANACTRL_TCON_PLL2_CNTL0;
	}
	return cconf;
}
#endif
