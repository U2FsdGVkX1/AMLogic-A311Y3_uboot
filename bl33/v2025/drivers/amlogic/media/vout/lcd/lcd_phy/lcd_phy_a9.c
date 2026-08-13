// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <amlogic/media/vout/lcd/aml_lcd.h>
#include <amlogic/aml_efuse.h>
#include "../lcd_reg.h"
#include "lcd_phy_config.h"
#include "../lcd_common.h"

#ifdef CONFIG_MESON_A9
#define PHY_DEF_ODT  0x0
#define PHY_DEF_BIAS 0x10
static unsigned int cali_bias, cali_odt, phy_ctrl_bit_on;
static u32 chctrl_reg[] = {
	ANACTRL_DIF_PHY_CNTL1, ANACTRL_DIF_PHY_CNTL2,
	ANACTRL_DIF_PHY_CNTL3, ANACTRL_DIF_PHY_CNTL4,
	ANACTRL_DIF_PHY_CNTL5,ANACTRL_DIF_PHY_CNTL1,
	ANACTRL_DIF_PHY_CNTL2, ANACTRL_DIF_PHY_CNTL3,
	ANACTRL_DIF_PHY_CNTL4, ANACTRL_DIF_PHY_CNTL5,
};

static unsigned int lcd_phy_get_def_odt(void)
{
	int efuse_odt = 0;

	efuse_odt = efuse_get_cali_item("p2p_vinlp");
	if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL)
		LCDPR("efuse odt=%#x\n", efuse_odt);
	if (efuse_odt < 0) {
		efuse_odt = PHY_DEF_ODT;
		LCDERR("odt uncalibrated, use odt=%#x\n", efuse_odt);
	}
	return (unsigned int)efuse_odt;
}

static unsigned int lcd_phy_get_def_bias(void)
{
	int efuse_bias = 0;

	efuse_bias = efuse_get_cali_item("p2p_common");
	if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL)
		LCDPR("efuse bias=%#x\n", efuse_bias);
	if (efuse_bias < 0) {
		efuse_bias = PHY_DEF_BIAS;
		LCDERR("bias uncalibrated, use bias=%#x\n", efuse_bias);
	}
	return (unsigned int)efuse_bias;
}

static void lcd_phy_reg_dump(struct aml_lcd_drv_s *pdrv)
{
	struct reg_name_set_s reg_table[] = {
		{ANACTRL_DIF_PHY_CNTL1,  "PHY_CNTL1"},
		{ANACTRL_DIF_PHY_CNTL2,  "PHY_CNTL2"},
		{ANACTRL_DIF_PHY_CNTL3,  "PHY_CNTL3"},
		{ANACTRL_DIF_PHY_CNTL4,  "PHY_CNTL4"},
		{ANACTRL_DIF_PHY_CNTL5,  "PHY_CNTL5"},
		{ANACTRL_DIF_PHY_CNTL6,  "PHY_CNTL6"},
		{ANACTRL_DIF_PHY_CNTL7,  "PHY_CNTL7"},
		{ANACTRL_DIF_PHY_CNTL8,  "PHY_CNTL8"},
		{ANACTRL_DIF_PHY_CNTL9,  "PHY_CNTL9"},
	};

	str_add_reg_sets(pdrv, LCD_REG_DBG_ANA_BUS, 0, reg_table, ARRAY_SIZE(reg_table));
}


/*
 * update odt based on efuse trim value
 *   write_odt = cali_odt + (custom_odt - DEF_ODT)
 */
static unsigned int lcd_phy_get_write_odt(struct phy_attr_s *phy)
{
	int odt = cali_odt + phy->odt - PHY_DEF_ODT;

	odt = odt < 0 ? 0 : odt;
	odt = odt > 0xf ? 0xf : odt;

	if (lcd_debug_print_flag & LCD_DBG_PR_ADV)
		LCDPR("cali odt=0x%x, write odt=0x%x\n", cali_odt, odt);
	return odt;
}

static int lcd_phy_param_get_from_reg(struct aml_lcd_drv_s *pdrv,
				      struct phy_config_s *phy_cfg, struct phy_attr_s *phy)
{
	return 0;
}

#define CH_NUM_EACH_GROUP 5

static void lcd_phy_common_update(struct aml_lcd_drv_s *pdrv, unsigned int common0,
					unsigned int common1, unsigned int common2)
{
	struct phy_attr_s *phy = pdrv->config.phy_cfg.act_phy;
	unsigned int dual_port = 0;

	switch (pdrv->config.basic.lcd_type) {
	case LCD_LVDS:
		if (pdrv->config.control.lvds_cfg.dual_port)
			dual_port = 1;
		break;
	case LCD_MIPI:
		if (pdrv->config.control.mipi_cfg.multi_port_cfg & BIT(0))
			dual_port = 1;
		break;
	case LCD_VBYONE:
		dual_port = 1;
		break;
	default:
		break;
	}

	if (dual_port) {
		lcd_ana_setb(ANACTRL_DIF_PHY_CNTL6, common0, 0, 16);
		lcd_ana_setb(ANACTRL_DIF_PHY_CNTL6, common0, 16, 16);
		lcd_ana_setb(ANACTRL_DIF_PHY_CNTL7, common1 | lcd_phy_get_write_odt(phy), 0, 16);
		lcd_ana_setb(ANACTRL_DIF_PHY_CNTL7, common1 | lcd_phy_get_write_odt(phy), 16, 16);
		lcd_ana_setb(ANACTRL_DIF_PHY_CNTL8, common2, 0, 16);
		lcd_ana_setb(ANACTRL_DIF_PHY_CNTL8, common2, 16, 16);
	} else {
		if (pdrv->index) {
			lcd_ana_setb(ANACTRL_DIF_PHY_CNTL6, common0, 16, 16);
			lcd_ana_setb(ANACTRL_DIF_PHY_CNTL7, common1 | phy->odt, 16, 16);
			lcd_ana_setb(ANACTRL_DIF_PHY_CNTL8, common2, 16, 16);
		} else {
			lcd_ana_setb(ANACTRL_DIF_PHY_CNTL6, common0, 0, 16);
			lcd_ana_setb(ANACTRL_DIF_PHY_CNTL7, common1 | phy->odt, 0, 16);
			lcd_ana_setb(ANACTRL_DIF_PHY_CNTL8, common2, 0, 16);
		}
	}
}

static void lcd_phy_bias_common_update(struct aml_lcd_drv_s *pdrv, unsigned int bias_common)
{
	lcd_ana_setb(ANACTRL_DIF_PHY_CNTL9, bias_common, 0, 16);
}

static void lcd_phy_serializer_common_update(struct aml_lcd_drv_s *pdrv)
{
	uint8_t model_set = 0, dual_port = 0;

	switch (pdrv->config.basic.lcd_type) {
	case LCD_LVDS:
		model_set = 0;
		dual_port = pdrv->config.control.lvds_cfg.dual_port ? 1 : 0;
		break;
	case LCD_MIPI:
		dual_port = pdrv->config.control.mipi_cfg.multi_port_cfg & BIT(0) ? 1 : 0;
		break;
	case LCD_VBYONE:
		dual_port = 1;
		model_set = 2;
		break;
	default:
		return;
	}

	if (pdrv->index == 0) {
		lcd_ana_setb(ANACTRL_DIF_PHY_CNTL9, 0x1, 16, 1);
		lcd_ana_setb(ANACTRL_DIF_PHY_CNTL9, model_set, 18, 2);
		if (dual_port) {
			lcd_ana_setb(ANACTRL_DIF_PHY_CNTL9, 0x1, 20, 1);
			lcd_ana_setb(ANACTRL_DIF_PHY_CNTL9, model_set, 22, 2);
			lcd_ana_setb(ANACTRL_DIF_PHY_CNTL9, 0, 24, 2);
		}
	} else if (pdrv->index == 1) {
		lcd_ana_setb(ANACTRL_DIF_PHY_CNTL9, 0x1, 20, 1);
		lcd_ana_setb(ANACTRL_DIF_PHY_CNTL9, model_set, 22, 2);
		lcd_ana_setb(ANACTRL_DIF_PHY_CNTL9, 1, 24, 2);
	}

	lcd_ana_setb(ANACTRL_DIF_PHY_CNTL9, 1, 25, 2);
}


static void lcd_phy_ch_set(struct aml_lcd_drv_s *pdrv)
{
	unsigned int chctrl = 0;
	unsigned char i, bit;
	struct phy_config_s *phy_cfg = &pdrv->config.phy_cfg;
	struct phy_attr_s *phy = pdrv->config.phy_cfg.act_phy;

	if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL)
		LCDPR("%s: ckdi:0x%x\n", __func__, phy_cfg->ckdi);

	for (i = 0; i < phy_cfg->lane_num; i++) {
		bit = pdrv->index ? 16 : 0;
		if (i >= CH_NUM_EACH_GROUP)
			bit = 16;
		chctrl = 1 << 13;
		if (pdrv->config.basic.lcd_type == LCD_LVDS)
			chctrl |= 1 << 15;
		//if (phy->cv_mode == PHY_VMODE) {
			//chctrl |= (phy->lane[i].rterm & 0x7) << 10;
		chctrl |= 0x4 << 10;
		chctrl |= (phy->vcm & 0x7) << 7;

		chctrl |= (phy->lane[i].preem & 0x7) << 4;
		chctrl |= (phy->lane[i].amp & 0xf);
		lcd_ana_setb(chctrl_reg[i], chctrl, bit, 16);
	}
}

static void lcd_lvds_phy_set(struct aml_lcd_drv_s *pdrv, int status)
{
	unsigned int common0 = 0;
	unsigned int common1 = 0x4020;
	unsigned int common2 = 0x4026;
	unsigned int bias_common = 0xe450;

	lcd_phy_common_update(pdrv, common0, common1, common2);
	lcd_phy_bias_common_update(pdrv, bias_common);
	lcd_phy_serializer_common_update(pdrv);

	lcd_phy_ch_set(pdrv);
}

static void lcd_vx1_phy_set(struct aml_lcd_drv_s *pdrv, int status)
{
	unsigned int common0 = 0x27;
	unsigned int common1 = 0x20;
	unsigned int common2 = 0x4000;
	unsigned int bias_common = 0xe050;

	lcd_phy_common_update(pdrv, common0, common1, common2);
	lcd_phy_bias_common_update(pdrv, bias_common);
	lcd_phy_serializer_common_update(pdrv);

	lcd_phy_ch_set(pdrv);
}

#define DSI_VCM_SETTING 0x4
#define DSI_PREEM_SETTING 0x0
#define DSI_CURRENT_SETTING 0xf

static void lcd_mipi_phy_set(struct aml_lcd_drv_s *pdrv, int status)
{
	struct phy_attr_s *phy = pdrv->config.phy_cfg.act_phy;

	uint32_t common0 =
		((0x0 & 0x1f) << 8) |
		// serilizer output data inv,bite<12:8> control ch5,ch4,ch3,ch2,ch1,ch0 respectively
		((0x0 &  0x1) << 7) | // lvds rterm  trim enable signal, enable for rterm trim
		((0x0 &  0x1) << 6) | // prbs check enable signal
		((0x1 &  0x1) << 5) | // ccp enable signal;
		((0x0 &  0x1) << 4) | // input data select 0:digital 1:analog prbs
		((0x7 &  0xf) << 0);  // vml resistance trim
	uint32_t common1 =
		((0x0 &  0x1) << 15) | // voltage lp_vref，0:bg current 1:resistance division of avdd18
		((0x6 &  0x7) << 12) | // lptx slew setting
		((0x8 &  0x1f) << 7) | // vml iboost control signal;
		((0x1 & 0x1) << 6) | // ch1~ch4 vml boost enable signal;
		((0x0 & 0x1) << 5) | // ch1~ch4 vml ldo power 0:ldo on；1:ldo off1.8V drive 0.8V nmos
		((0x0 & 0x1) << 4) | // channel ldo for clk, 1:on; 0:0ff;
		((phy->odt & 0xf) << 4);  // lvds rterm  resistance control bite;
	uint32_t common2 =
		((0x1 & 0x1) << 15) | //ch0 vml iboost enable signal;
		((0x0 & 0x1) << 14) | //ch0 vml ldo power 0: ldo on; 1: ldo off，1.8V drive 0.8V nmos
		((0x0 & 0x3) << 12) | //dsi hs slew control bite;
		((0x0 & 0x1) << 11) | //auxrx/mipi_lp output data inverse,1b'1 is inversed;
		((0x0 & 0x1) << 10) | //lprx refh voltagesetting;    1: 0.74V ;  0: 0.88V;
		((0x0 & 0x1) << 9) | // lpcd reference low voltage select; 0: REFL 0.6v 1: ULV 0.3v;
		((0x0 & 0x1) << 8) | // lpcd/lprx reference voltage select;   0: vbg   1: avdd18
		((0x1 & 0x3) << 6) | //lprx high 00: 0.82v 01:0.86v 10: 0.89v 11: 0.93v
		((0x1 & 0x3) << 4) | //lprx low 00: 0.52v 01:0.56v 10: 0.60v 11: 0.64v
		((0x2 & 0x3) << 2) | //lpcd high 00: 0.37v 01:0.41v 10: 0.45v 11: 0.49v
		((0x1 & 0x3) << 0);  //lpcd low 00: 0.17v 01:0.20v 10: 0.24v 11: 0.27v
	uint32_t bias_common = 0xe050;
	uint32_t chctrl;
	uint8_t i;
	uint8_t dsi_2p = pdrv->config.control.mipi_cfg.multi_port_cfg & BIT(0) ? 1 : 0;

	chctrl = (1 << 13) | // channel enable signal
		((0x4 & 0x7) << 10) | // vml adjust the number of Slicers manually;
		((DSI_VCM_SETTING & 0x7) << 7) | //vcm
		((DSI_PREEM_SETTING & 0x7) << 4) | //lvds pre_emphasis control bite;
		(DSI_CURRENT_SETTING & 0xf << 0); //lvds main &post current control bite;

	if (pdrv->index == 0) {
		lcd_ana_setb(ANACTRL_DIF_PHY_CNTL6, common0, 0, 16);
		lcd_ana_setb(ANACTRL_DIF_PHY_CNTL7, common1, 0, 16);
		lcd_ana_setb(ANACTRL_DIF_PHY_CNTL8, common2, 0, 16);

		lcd_ana_setb(ANACTRL_DIF_PHY_CNTL9, 0x1, 16, 1);
		lcd_ana_setb(ANACTRL_DIF_PHY_CNTL9, 1, 18, 2);

		for (i = 0; i < 5; i++)
			lcd_ana_setb(chctrl_reg[i], chctrl | (((i == 2) ? 1 : 0) << 14), 0, 16);
	}
	if (pdrv->index == 1 || dsi_2p) {
		lcd_ana_setb(ANACTRL_DIF_PHY_CNTL6, common0, 16, 16);
		lcd_ana_setb(ANACTRL_DIF_PHY_CNTL7, common1, 16, 16);
		lcd_ana_setb(ANACTRL_DIF_PHY_CNTL8, common2, 16, 16);

		lcd_ana_setb(ANACTRL_DIF_PHY_CNTL9, 0x1, 20, 1);
		lcd_ana_setb(ANACTRL_DIF_PHY_CNTL9, 1, 22, 2); // mode set

		if (pdrv->index == 1)
			lcd_ana_setb(ANACTRL_DIF_PHY_CNTL9, 1, 24, 2);
		else // dsi_2p
			lcd_ana_setb(ANACTRL_DIF_PHY_CNTL9, 0, 24, 2);

		for (i = 0; i < 5; i++)
			lcd_ana_setb(chctrl_reg[i], chctrl | (((i == 2) ? 1 : 0) << 14), 16, 16);
	}

	lcd_ana_setb(ANACTRL_DIF_PHY_CNTL9, bias_common, 0, 16);

	lcd_ana_setb(ANACTRL_DIF_PHY_CNTL9, 1, 25, 2);
}

static unsigned int lcd_phy_preem_level_to_val_a9(struct aml_lcd_drv_s *pdrv, unsigned int level)
{
	return 0;
}

static unsigned int lcd_phy_amp_dft_a9(struct aml_lcd_drv_s *pdrv)
{
	return 0xf;
}

static void phy_glb_param_dft_a9(struct aml_lcd_drv_s *pdrv)
{
	struct phy_attr_s *phy = pdrv->config.phy_cfg.act_phy;

	phy->ref_bias = 0;
	switch (pdrv->config.basic.lcd_type) {
	case LCD_LVDS:
	case LCD_MIPI:
	case LCD_VBYONE:
		phy->vcm = 0x4;
		phy->odt = PHY_DEF_ODT;
		phy->vswing = 0xf;
		phy->cv_mode = 0;
		break;
	default:
		break;
	}
}

static struct lcd_phy_ctrl_s lcd_phy_ctrl_a9 = {
	.lane_num = 10,

	.phy_vswing_level_to_val = lcd_phy_vswing_level_to_value_dft,
	.phy_amp_dft_val = lcd_phy_amp_dft_a9,
	.phy_preem_level_to_val = lcd_phy_preem_level_to_val_a9,
	.phy_lane_phase_sel_def = NULL,
	.phy_glb_param_dft_val = phy_glb_param_dft_a9,
	.phy_param_get = lcd_phy_param_get_from_reg,
	.phy_reg_dump = lcd_phy_reg_dump,

	.phy_reset = NULL,
	.phy_default_off = NULL,

	.phy_set_lvds = lcd_lvds_phy_set,
	.phy_set_vx1 = lcd_vx1_phy_set,
	.phy_set_mlvds = NULL,
	.phy_set_p2p = NULL,
	.phy_set_mipi = lcd_mipi_phy_set,
	.phy_set_edp = NULL,
};

struct lcd_phy_ctrl_s *lcd_phy_config_init_a9(struct aml_lcd_data_s *pdata)
{
	cali_odt = lcd_phy_get_def_odt();
	cali_bias = lcd_phy_get_def_bias();
	phy_ctrl_bit_on = (pdata->rev_type > 0xa) ? 1 : 0;

	return &lcd_phy_ctrl_a9;
}
#endif
