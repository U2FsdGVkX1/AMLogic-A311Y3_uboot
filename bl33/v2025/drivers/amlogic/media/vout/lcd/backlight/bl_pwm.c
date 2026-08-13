// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <dm.h>
#include <asm/gpio.h>
#include <amlogic/media/vout/lcd/aml_lcd.h>
#include "lcd_bl.h"
#include "../lcd_reg.h"
#include "../lcd_common.h"

#define PWM_REG_MAX    0xffffffff

struct bl_pwm_clkctrl_s {
	unsigned int reg;
	unsigned int bit_div;
	unsigned int bit_sel;
	unsigned int bit_en;
};

struct bl_pwm_misc_s {
	unsigned int reg;
	unsigned int bit_pre_div;
	unsigned int bit_clk_sel;
	unsigned int bit_clk_en;
	unsigned int bit_pwm_en;
	unsigned int val_pwm_en;
};

struct bl_pwm_ctrl_config_s {
	unsigned int pwm_div_flag; /*1:div in clktree*/
	unsigned int pwm_vs_flag; /*1:8ch*/
	struct bl_pwm_clkctrl_s *pwm_clk;
	struct bl_pwm_misc_s *pwm_misc;
	unsigned int *pwm_reg;
	unsigned int pwm_cnt;
	struct bl_pwm_clkctrl_s *pwm_ao_clk;
	struct bl_pwm_misc_s *pwm_ao_misc;
	unsigned int *pwm_ao_reg;
	unsigned int pwm_ao_cnt;
};

static struct bl_pwm_ctrl_config_s *bl_pwm_ctrl_conf;

__maybe_unused static unsigned int pwm_reg_dft[] = {
	PWM_PWM_A,
	PWM_PWM_B,
	PWM_PWM_C,
	PWM_PWM_D,
	PWM_PWM_E,
	PWM_PWM_F,
	PWM_PWM_G,
	PWM_PWM_H,
	PWM_PWM_I,
	PWM_PWM_J,
	PWM_REG_MAX
};

__maybe_unused static struct bl_pwm_clkctrl_s pwm_clk_ctrl_dft[] = {
	/* pwm_reg,                bit_div, bit_sel, bit_en*/
	{CLKCTRL_PWM_CLK_AB_CTRL,   0,       9,       8},
	{CLKCTRL_PWM_CLK_AB_CTRL,   16,      25,      24},
	{CLKCTRL_PWM_CLK_CD_CTRL,   0,       9,       8},
	{CLKCTRL_PWM_CLK_CD_CTRL,   16,      25,      24},
	{CLKCTRL_PWM_CLK_EF_CTRL,   0,       9,       8},
	{CLKCTRL_PWM_CLK_EF_CTRL,   16,      25,      24},
	{CLKCTRL_PWM_CLK_GH_CTRL,   0,       9,       8},
	{CLKCTRL_PWM_CLK_GH_CTRL,   16,      25,      24},
	{CLKCTRL_PWM_CLK_IJ_CTRL,   0,       9,       8},
	{CLKCTRL_PWM_CLK_IJ_CTRL,   16,      25,      24},
	{PWM_REG_MAX,               0,       0,       0}
};

static struct bl_pwm_misc_s pwm_misc_t3[] = {
	/* pwm_reg,      bit_pre_div, bit_clk_sel, bit_clk_en, bit_pwm_en, val_en*/
	{PWMAB_MISC_REG_AB,   8,       4,          15,         0,          1,},
	{PWMAB_MISC_REG_AB,   16,      6,          23,         1,          1,},
	{PWMCD_MISC_REG_AB,   8,       4,          15,         0,          1,},
	{PWMCD_MISC_REG_AB,   16,      6,          23,         1,          1,},
	{PWMEF_MISC_REG_AB,   8,       4,          15,         0,          1,},
	{PWMEF_MISC_REG_AB,   16,      6,          23,         1,          1,},
	{PWMGH_MISC_REG_AB,   8,       4,          15,         0,          1,},
	{PWMGH_MISC_REG_AB,   16,      6,          23,         1,          1,},
	{PWMIJ_MISC_REG_AB,   8,       4,          15,         0,          1,},
	{PWMIJ_MISC_REG_AB,   16,      6,          23,         1,          1,},
	{PWM_REG_MAX,         0,       0,          0,          0,          0,}
};

#if (IS_ENABLED(CONFIG_MESON_S6))
static struct bl_pwm_misc_s pwm_misc_s6[] = {
	/* pwm_reg,      bit_pre_div, bit_clk_sel, bit_clk_en, bit_pwm_en, val_en*/
	{PWM_MISC_REG_A, 8,           4,           15,         0,          1,},
	{PWM_MISC_REG_B, 8,           4,           15,         0,          1,},
	{PWM_MISC_REG_C, 8,           4,           15,         0,          1,},
	{PWM_MISC_REG_D, 8,           4,           15,         0,          1,},
	{PWM_MISC_REG_E, 8,           4,           15,         0,          1,},
	{PWM_MISC_REG_F, 8,           4,           15,         0,          1,},
	{PWM_MISC_REG_G, 8,           4,           15,         0,          1,},
	{PWM_MISC_REG_H, 8,           4,           15,         0,          1,},
	{PWM_MISC_REG_I, 8,           4,           15,         0,          1,},
	{PWM_MISC_REG_J, 8,           4,           15,         0,          1,},
	{PWM_REG_MAX,    0,           0,           0,          0,          0,}
};
#endif

#if (IS_ENABLED(CONFIG_MESON_T6D))
static struct bl_pwm_misc_s pwm_misc_t6d[] = {
	/* pwm_reg,      bit_pre_div, bit_clk_sel, bit_clk_en, bit_pwm_en, val_en*/
	{PWM_MISC_REG_A, 8,           4,           15,         0,          1,},
	{PWM_MISC_REG_B, 8,           4,           15,         0,          1,},
	{PWM_MISC_REG_C, 8,           4,           15,         0,          1,},
	{PWM_MISC_REG_D, 8,           4,           15,         0,          1,},
	{PWM_MISC_REG_E, 8,           4,           15,         0,          1,},
	{PWM_MISC_REG_F, 8,           4,           15,         0,          1,},
	{PWM_MISC_REG_G, 8,           4,           15,         0,          1,},
	{PWM_MISC_REG_H, 8,           4,           15,         0,          1,},
	{PWM_REG_MAX,    0,           0,           0,          0,          0,}
};
#endif

static unsigned int pwm_reg_t3[] = {
	PWMAB_PWM_A,
	PWMAB_PWM_B,
	PWMCD_PWM_A,
	PWMCD_PWM_B,
	PWMEF_PWM_A,
	PWMEF_PWM_B,
	PWMGH_PWM_A,
	PWMGH_PWM_B,
	PWMIJ_PWM_A,
	PWMIJ_PWM_B,
	PWM_REG_MAX
};

static struct bl_pwm_ctrl_config_s bl_pwm_ctrl_conf_t3x = {
	.pwm_div_flag = 1,
	.pwm_vs_flag = 1,
	.pwm_clk = pwm_clk_ctrl_dft,
	.pwm_misc = pwm_misc_t3,
	.pwm_reg = pwm_reg_t3,
	.pwm_cnt = 10,
	.pwm_ao_clk = NULL,
	.pwm_ao_misc = NULL,
	.pwm_ao_reg = NULL,
	.pwm_ao_cnt = 0,
};

#if (IS_ENABLED(CONFIG_MESON_S6))
static struct bl_pwm_ctrl_config_s bl_pwm_ctrl_conf_s6 = {
	.pwm_div_flag = 1,
	.pwm_vs_flag = 0,
	.pwm_clk = pwm_clk_ctrl_dft,
	.pwm_misc = pwm_misc_s6,
	.pwm_reg = pwm_reg_dft,
	.pwm_cnt = 10,
	.pwm_ao_clk = NULL,
	.pwm_ao_misc = NULL,
	.pwm_ao_reg = NULL,
	.pwm_ao_cnt = 0,
};
#endif

#if (IS_ENABLED(CONFIG_MESON_T6D))
static struct bl_pwm_ctrl_config_s bl_pwm_ctrl_conf_t6d = {
	.pwm_div_flag = 1,
	.pwm_vs_flag = 1,
	.pwm_clk = pwm_clk_ctrl_dft,
	.pwm_misc = pwm_misc_t6d,
	.pwm_reg = pwm_reg_dft,
	.pwm_cnt = 10,
	.pwm_ao_clk = NULL,
	.pwm_ao_misc = NULL,
	.pwm_ao_reg = NULL,
	.pwm_ao_cnt = 0,
};
#endif

static char *bl_pwm_name[] = {
	"PWM_A",
	"PWM_B",
	"PWM_C",
	"PWM_D",
	"PWM_E",
	"PWM_F",
	"PWM_G",
	"PWM_H",
	"PWM_I",
	"PWM_J"
};

static char *bl_pwm_ao_name[] = {
	"PWM_AO_A",
	"PWM_AO_B",
	"PWM_AO_C",
	"PWM_AO_D",
	"PWM_AO_E",
	"PWM_AO_F",
	"PWM_AO_G",
	"PWM_AO_H"
};

static char bl_pwm_vs_name[] = {"PWM_VS"};
static char bl_pwm_invalid_name[] = {"invalid"};

enum bl_pwm_port_e bl_pwm_str_to_num(const char *str)
{
	enum bl_pwm_port_e pwm_port = BL_PWM_MAX;
	int i, cnt;

	if (!bl_pwm_ctrl_conf)
		return BL_PWM_MAX;

	cnt = bl_pwm_ctrl_conf->pwm_cnt;
	if (cnt > ARRAY_SIZE(bl_pwm_name))
		cnt = ARRAY_SIZE(bl_pwm_name);
	for (i = 0; i < cnt; i++) {
		if (strcmp(str, bl_pwm_name[i]) == 0) {
			pwm_port = i + BL_PWM_A;
			return pwm_port;
		}
	}

	cnt = bl_pwm_ctrl_conf->pwm_ao_cnt;
	if (cnt > ARRAY_SIZE(bl_pwm_ao_name))
		cnt = ARRAY_SIZE(bl_pwm_ao_name);
	for (i = 0; i < cnt; i++) {
		if (strcmp(str, bl_pwm_ao_name[i]) == 0) {
			pwm_port = i + BL_PWM_AO_A;
			return pwm_port;
		}
	}

	if (strcmp(str, bl_pwm_vs_name) == 0) {
		pwm_port = BL_PWM_VS;
		return pwm_port;
	}

	return BL_PWM_MAX;
}

char *bl_pwm_num_to_str(unsigned int num)
{
	unsigned int temp, cnt;

	if (num < BL_PWM_AO_A) {
		temp = num - BL_PWM_A;
		cnt = ARRAY_SIZE(bl_pwm_name);
		if (temp >= cnt)
			return bl_pwm_invalid_name;
		return bl_pwm_name[temp];
	} else if (num < BL_PWM_VS) {
		temp = num - BL_PWM_AO_A;
		cnt = ARRAY_SIZE(bl_pwm_ao_name);
		if (temp >= cnt)
			return bl_pwm_invalid_name;
		return bl_pwm_ao_name[temp];
	} else if (num == BL_PWM_VS) {
		return bl_pwm_vs_name;
	}

	return bl_pwm_invalid_name;
}

int bl_str_to_pwm_method(const char *str, int def_val)
{
	if (strcmp(str, "BL_PWM_NEGATIVE") == 0)
		return BL_PWM_NEGATIVE;
	else if (strcmp(str, "BL_PWM_POSITIVE") == 0)
		return BL_PWM_POSITIVE;
	else
		return def_val;
}

unsigned int bl_pwm_duty_input_scale(struct aml_bl_drv_s *bdrv, unsigned int duty)
{
	unsigned int out_duty, half;
	unsigned long long temp = BL_PWM_DUTY_FULL_SCALE;

	if (!bdrv || bdrv->config.pwm_duty_in_scale == 0)
		return 0;

	half = (bdrv->config.pwm_duty_in_scale + 1) >> 1;
	out_duty = lcd_do_div(duty * temp + half, bdrv->config.pwm_duty_in_scale);
	return out_duty;
}

unsigned int bl_pwm_duty_output_scale(struct aml_bl_drv_s *bdrv, unsigned int duty)
{
	unsigned int in_duty, half = (BL_PWM_DUTY_FULL_SCALE + 1) >> 1;
	unsigned long long temp;

	if (!bdrv || bdrv->config.pwm_duty_in_scale == 0)
		return 0;

	temp = bdrv->config.pwm_duty_in_scale;
	in_duty = lcd_do_div(duty * temp + half, BL_PWM_DUTY_FULL_SCALE);
	return in_duty;
}

static int bl_level_to_pwm_duty(struct aml_bl_drv_s *bdrv,
				struct bl_pwm_config_s *bl_pwm, int level)
{
	int lmin, lmax, dmin, dmax;
	int half, duty;
	unsigned long long temp;

	lmin = bl_pwm->bl_level_min;
	lmax = bl_pwm->bl_level_max;
	if (level < lmin || level > lmax || lmin >= lmax)
		return -1; //invalid

	if (bdrv->config.pwm_mapping_method == PWM_MAP_NORMALIZATION) {
		dmin = 0;
		dmax = BL_PWM_DUTY_FULL_SCALE;
	} else { //PWM_MAP_RESCALING
		dmin = bl_pwm->pwm_duty_min;
		dmax = bl_pwm->pwm_duty_max;
	}
	if (dmin > dmax)
		return -1;
	half = (lmax - lmin + 1) >> 1;
	temp = dmax - dmin;
	duty = lcd_do_div(temp * (level - lmin) + half, lmax - lmin) + dmin;
	return duty;
}

static void bl_pwm_duty_to_pwm_value(struct bl_pwm_config_s *bl_pwm)
{
	unsigned int half = (BL_PWM_DUTY_FULL_SCALE + 1) >> 1;
	unsigned long long temp;

	temp = bl_pwm->pwm_cnt;
	bl_pwm->pwm_value = lcd_do_div(temp * bl_pwm->pwm_duty + half, BL_PWM_DUTY_FULL_SCALE);
}

static void bl_pwm_set_value(struct bl_pwm_config_s *bl_pwm)
{
	unsigned int *pwm_reg;
	int port;

	switch (bl_pwm->pwm_method) {
	case BL_PWM_NEGATIVE:
		bl_pwm->pwm_lo = bl_pwm->pwm_value;
		bl_pwm->pwm_hi = bl_pwm->pwm_cnt - bl_pwm->pwm_value;
		break;
	default: //BL_PWM_POSITIVE
		bl_pwm->pwm_hi = bl_pwm->pwm_value;
		bl_pwm->pwm_lo = bl_pwm->pwm_cnt - bl_pwm->pwm_value;
		break;
	}

	switch (bl_pwm->pwm_port) {
	case BL_PWM_A:
	case BL_PWM_B:
	case BL_PWM_C:
	case BL_PWM_D:
	case BL_PWM_E:
	case BL_PWM_F:
	case BL_PWM_G:
	case BL_PWM_H:
	case BL_PWM_I:
	case BL_PWM_J:
		port = bl_pwm->pwm_port - BL_PWM_A;
		if (port >= bl_pwm_ctrl_conf->pwm_cnt) {
			BLERR("invalid pwm_port 0x%x\n", bl_pwm->pwm_port);
			return;
		}
		pwm_reg = bl_pwm_ctrl_conf->pwm_reg;
		break;
	case BL_PWM_AO_A:
	case BL_PWM_AO_B:
	case BL_PWM_AO_C:
	case BL_PWM_AO_D:
	case BL_PWM_AO_E:
	case BL_PWM_AO_F:
	case BL_PWM_AO_G:
	case BL_PWM_AO_H:
		port = bl_pwm->pwm_port - BL_PWM_AO_A;
		if (port >= bl_pwm_ctrl_conf->pwm_ao_cnt) {
			BLERR("invalid pwm_port 0x%x\n", bl_pwm->pwm_port);
			return;
		}
		pwm_reg = bl_pwm_ctrl_conf->pwm_ao_reg;
		break;
	default:
		return;
	}

	lcd_cbus_write(pwm_reg[port], (bl_pwm->pwm_hi << 16) | bl_pwm->pwm_lo);
	if (lcd_debug_print_flag & LCD_DBG_PR_BL_NORMAL)
		BLPR("pwm_reg=0x%08x\n", lcd_cbus_read(pwm_reg[port]));
}

static int bl_pwm_out_level_check(struct bl_pwm_config_s *bl_pwm)
{
	int out_level = 0xff;

	switch (bl_pwm->pwm_method) {
	case BL_PWM_POSITIVE:
		if (bl_pwm->pwm_duty == 0)
			out_level = 0;
		else if (bl_pwm->pwm_duty == BL_PWM_DUTY_FULL_SCALE)
			out_level = 1;
		else
			out_level = 0xff;
		break;
	case BL_PWM_NEGATIVE:
		if (bl_pwm->pwm_duty == 0)
			out_level = 1;
		else if (bl_pwm->pwm_duty == BL_PWM_DUTY_FULL_SCALE)
			out_level = 0;
		else
			out_level = 0xff;
		break;
	default:
		BLERR("%s: port %d: invalid pwm_method %d\n",
		      __func__, bl_pwm->pwm_port, bl_pwm->pwm_method);
		break;
	}

	return out_level;
}

static void bl_pwm_vs_set_value(struct bl_pwm_config_s *bl_pwm)
{
	unsigned int vs[8], ve[8], sw, n, i, pol = 0;
	unsigned int out_level = 0xff;

	pol = (bl_pwm->pwm_method == BL_PWM_NEGATIVE) ? 1 : 0;
	out_level = bl_pwm_out_level_check(bl_pwm);
	if (lcd_debug_print_flag & LCD_DBG_PR_BL_NORMAL)
		BLPR("%s: pwm_duty=%d, out_level=%d\n", __func__, bl_pwm->pwm_duty, out_level);

	if (out_level == 0) {
		for (i = 0; i < 8; i++) {
			vs[i] = 0x1fff;
			ve[i] = 0;
		}
	} else if (out_level == 1) {
		for (i = 0; i < 8; i++) {
			vs[i] = 0;
			ve[i] = 0x1fff;
		}
	} else {
		bl_pwm->pwm_hi = bl_pwm->pwm_value;
		n = bl_pwm->pwm_freq;
		sw = (bl_pwm->pwm_cnt * 10 / n + 5) / 10;
		bl_pwm->pwm_hi = (bl_pwm->pwm_hi * 10 / n + 5) / 10;
		bl_pwm->pwm_hi = (bl_pwm->pwm_hi > 1) ? bl_pwm->pwm_hi : 1;
		if (lcd_debug_print_flag & LCD_DBG_PR_BL_NORMAL)
			BLPR("n=%d, sw=%d, pwm_high=%d, phase=%d\n",
			     n, sw, bl_pwm->pwm_hi, bl_pwm->pwm_phase);
		for (i = 0; i < n; i++) {
			vs[i] = 1 + (sw * i) + bl_pwm->pwm_phase;
			ve[i] = vs[i] + bl_pwm->pwm_hi - 1;
			if (lcd_debug_print_flag & LCD_DBG_PR_BL_NORMAL)
				BLPR("vs[%d]=%d, ve[%d]=%d\n", i, vs[i], i, ve[i]);
		}
		for (i = n; i < 8; i++) {
			vs[i] = 0x1fff;
			ve[i] = 0x1fff;
		}
	}
	lcd_vcbus_write(VPU_VPU_PWM_V0, (pol << 31) | (ve[0] << 16) | (vs[0]));
	lcd_vcbus_write(VPU_VPU_PWM_V1, (ve[1] << 16) | (vs[1]));
	lcd_vcbus_write(VPU_VPU_PWM_V2, (ve[2] << 16) | (vs[2]));
	lcd_vcbus_write(VPU_VPU_PWM_V3, (ve[3] << 16) | (vs[3]));
	if (bl_pwm_ctrl_conf->pwm_vs_flag) {
		if (lcd_debug_print_flag & LCD_DBG_PR_BL_NORMAL)
			BLPR("pwm_vs_flag support\n");
		lcd_vcbus_setb(VPU_VPU_PWM_H0, 1, 31, 1);
		lcd_vcbus_setb(VPU_VPU_PWM_V0, vs[4], 0, 13);
		lcd_vcbus_setb(VPU_VPU_PWM_V0, ve[4], 16, 13);
		lcd_vcbus_write(VPU_VPU_PWM_V1, (ve[5] << 16) | (vs[5]));
		lcd_vcbus_write(VPU_VPU_PWM_V2, (ve[6] << 16) | (vs[6]));
		lcd_vcbus_write(VPU_VPU_PWM_V3, (ve[7] << 16) | (vs[7]));
		lcd_vcbus_setb(VPU_VPU_PWM_H0, 0, 31, 1);
	}
}

static void bl_pwm_vs_config_update(struct bl_pwm_config_s *bl_pwm)
{
	struct aml_lcd_drv_s *pdrv;

	if (bl_pwm->pwm_port != BL_PWM_VS)
		return;

	pdrv = aml_lcd_get_driver(bl_pwm->drv_index);
	bl_pwm->pwm_cnt = lcd_get_max_line_cnt(pdrv);
}

void bl_set_pwm(struct bl_pwm_config_s *bl_pwm)
{
	if (!bl_pwm || !bl_pwm_ctrl_conf)
		return;

	bl_pwm_vs_config_update(bl_pwm);
	if (bl_pwm->pwm_cnt == 0) {
		BLERR("%s: pwm port %d: pwm_cnt is 0\n", __func__, bl_pwm->pwm_port);
		return;
	}

	bl_pwm->pwm_duty_save = bl_pwm->pwm_duty;
	bl_pwm_duty_to_pwm_value(bl_pwm);

	switch (bl_pwm->pwm_port) {
	case BL_PWM_VS:
		bl_pwm_vs_set_value(bl_pwm);
		break;
	default:
		bl_pwm_set_value(bl_pwm);
		break;
	}
}

static int bl_pwm_duty_check(struct aml_bl_drv_s *bdrv, struct bl_pwm_config_s *bl_pwm)
{
	if (bl_pwm->pwm_duty < bl_pwm->pwm_duty_min ||
	    bl_pwm->pwm_duty > bl_pwm->pwm_duty_max) {
		BLERR("[%d]: pwm_duty_check: pwm_port: %d, duty %d out of range [%d~%d]\n",
		      bdrv->index, bl_pwm->pwm_port, bl_pwm->pwm_duty,
		      bl_pwm->pwm_duty_min, bl_pwm->pwm_duty_max);
		return -1;
	}

	return 0;
}

void bl_pwm_set_level(struct aml_bl_drv_s *bdrv, struct bl_pwm_config_s *bl_pwm, int level)
{
	int ret;

	if (!bdrv || !bl_pwm)
		return;
	ret = bl_level_to_pwm_duty(bdrv, bl_pwm, level);
	if (ret < 0)
		return;
	bl_pwm->pwm_duty = ret;
	if (bl_pwm_duty_check(bdrv, bl_pwm)) {
		bl_pwm->pwm_duty = bl_pwm->pwm_duty_save;
		return;
	}

	if (lcd_debug_print_flag & LCD_DBG_PR_BL_NORMAL) {
		BLPR("pwm_port 0x%x: level=%d [%d~%d], duty=%d [%d~%d]\n",
		     bl_pwm->pwm_port, level, bl_pwm->bl_level_min, bl_pwm->bl_level_max,
		     bl_pwm->pwm_duty, bl_pwm->pwm_duty_min, bl_pwm->pwm_duty_max);
	}

	bl_set_pwm(bl_pwm);
}

void bl_pwm_en(struct bl_pwm_config_s *bl_pwm, int flag)
{
	struct bl_pwm_clkctrl_s *pwm_clk;
	struct bl_pwm_misc_s *pwm_misc;
	unsigned int port, pre_div;

	if (!bl_pwm || !bl_pwm_ctrl_conf)
		return;

	if (lcd_debug_print_flag & LCD_DBG_PR_BL_NORMAL)
		BLPR("%s: %d, pwm_port 0x%x\n", __func__, flag, bl_pwm->pwm_port);

	pre_div = bl_pwm->pwm_pre_div;
	switch (bl_pwm->pwm_port) {
	case BL_PWM_A:
	case BL_PWM_B:
	case BL_PWM_C:
	case BL_PWM_D:
	case BL_PWM_E:
	case BL_PWM_F:
	case BL_PWM_G:
	case BL_PWM_H:
	case BL_PWM_I:
	case BL_PWM_J:
		port = bl_pwm->pwm_port - BL_PWM_A;
		if (port >= bl_pwm_ctrl_conf->pwm_cnt) {
			BLERR("invalid pwm_port 0x%x\n", bl_pwm->pwm_port);
			return;
		}
		pwm_clk = bl_pwm_ctrl_conf->pwm_clk;
		pwm_misc = bl_pwm_ctrl_conf->pwm_misc;
		break;
	case BL_PWM_AO_A:
	case BL_PWM_AO_B:
	case BL_PWM_AO_C:
	case BL_PWM_AO_D:
	case BL_PWM_AO_E:
	case BL_PWM_AO_F:
	case BL_PWM_AO_G:
	case BL_PWM_AO_H:
		port = bl_pwm->pwm_port - BL_PWM_AO_A;
		if (port >= bl_pwm_ctrl_conf->pwm_ao_cnt) {
			BLERR("invalid pwm_port 0x%x\n", bl_pwm->pwm_port);
			return;
		}
		pwm_clk = bl_pwm_ctrl_conf->pwm_ao_clk;
		pwm_misc = bl_pwm_ctrl_conf->pwm_ao_misc;
		break;
	default:
		return;
	}

	if (flag) {
		/* pwm clk_en */
		lcd_cbus_setb(pwm_misc[port].reg, 1, pwm_misc[port].bit_clk_en, 1);
		/* pwm enable */
		lcd_cbus_setb(pwm_misc[port].reg, pwm_misc[port].val_pwm_en,
			      pwm_misc[port].bit_pwm_en, 1);
	} else {
		/* pwm clk_disable */
		lcd_cbus_setb(pwm_misc[port].reg, 0, pwm_misc[port].bit_clk_en, 1);
		if (pwm_clk)
			lcd_cbus_setb(pwm_clk[port].reg, 0, pwm_clk[port].bit_en, 1);
	}
}

static void bl_pwm_channel_init(struct bl_pwm_config_s *bl_pwm)
{
	struct bl_pwm_clkctrl_s *pwm_clk;
	struct bl_pwm_misc_s *pwm_misc;
	unsigned int port, pre_div;

	if (!bl_pwm || !bl_pwm_ctrl_conf)
		return;

	pre_div = bl_pwm->pwm_pre_div;
	switch (bl_pwm->pwm_port) {
	case BL_PWM_A:
	case BL_PWM_B:
	case BL_PWM_C:
	case BL_PWM_D:
	case BL_PWM_E:
	case BL_PWM_F:
	case BL_PWM_G:
	case BL_PWM_H:
	case BL_PWM_I:
	case BL_PWM_J:
		port = bl_pwm->pwm_port - BL_PWM_A;
		if (port >= bl_pwm_ctrl_conf->pwm_cnt) {
			BLERR("invalid pwm_port 0x%x\n", bl_pwm->pwm_port);
			return;
		}
		pwm_clk = bl_pwm_ctrl_conf->pwm_clk;
		pwm_misc = bl_pwm_ctrl_conf->pwm_misc;
		break;
	case BL_PWM_AO_A:
	case BL_PWM_AO_B:
	case BL_PWM_AO_C:
	case BL_PWM_AO_D:
	case BL_PWM_AO_E:
	case BL_PWM_AO_F:
	case BL_PWM_AO_G:
	case BL_PWM_AO_H:
		port = bl_pwm->pwm_port - BL_PWM_AO_A;
		if (port >= bl_pwm_ctrl_conf->pwm_ao_cnt) {
			BLERR("invalid pwm_port 0x%x\n", bl_pwm->pwm_port);
			return;
		}
		pwm_clk = bl_pwm_ctrl_conf->pwm_ao_clk;
		pwm_misc = bl_pwm_ctrl_conf->pwm_ao_misc;
		break;
	default:
		return;
	}

	if (pwm_clk) {
		if (bl_pwm_ctrl_conf->pwm_div_flag)
			lcd_cbus_setb(pwm_clk[port].reg, pre_div, pwm_clk[port].bit_div, 2);
		else
			lcd_cbus_setb(pwm_clk[port].reg, 0, pwm_clk[port].bit_div, 2);
		lcd_cbus_setb(pwm_clk[port].reg, 0, pwm_clk[port].bit_sel, 8);
		lcd_cbus_setb(pwm_clk[port].reg, 1, pwm_clk[port].bit_en, 1);
	}
	if (bl_pwm_ctrl_conf->pwm_div_flag == 0)
		lcd_cbus_setb(pwm_misc[port].reg, pre_div, pwm_misc[port].bit_pre_div, 7);
	lcd_cbus_setb(pwm_misc[port].reg, pre_div, pwm_misc[port].bit_pre_div, 7);
	/* pwm clk_sel */
	lcd_cbus_setb(pwm_misc[port].reg, 0, pwm_misc[port].bit_clk_sel, 2);

	if (lcd_debug_print_flag & LCD_DBG_PR_BL_NORMAL)
		BLPR("%s: pwm_port 0x%x\n", __func__, bl_pwm->pwm_port);
}

void bl_pwm_config_init(struct bl_pwm_config_s *bl_pwm)
{
	struct aml_lcd_drv_s *pdrv;
	unsigned int pre_div, cnt;
	int i;

	if (!bl_pwm) {
		BLERR("%s: bl_pwm is NULL\n", __func__);
		return;
	}
	if (bl_pwm->pwm_port >= BL_PWM_MAX)
		return;

	pdrv = aml_lcd_get_driver(bl_pwm->drv_index);
	switch (bl_pwm->pwm_port) {
	case BL_PWM_VS:
		if (bl_pwm->pwm_freq > 8) {
			BLERR("bl_pwm_vs wrong freq %d\n", bl_pwm->pwm_freq);
			bl_pwm->pwm_freq = BL_FREQ_VS_DEFAULT;
		}
		bl_pwm->pwm_cnt = pdrv->config.timing.act_timing.v_period;
		bl_pwm->pwm_pre_div = 0;
		break;
	default:
		if (bl_pwm->pwm_freq > XTAL_HALF_FREQ_HZ)
			bl_pwm->pwm_freq = XTAL_HALF_FREQ_HZ;
		for (i = 0; i < 0x7f; i++) {
			pre_div = i;
			cnt = XTAL_FREQ_HZ / (bl_pwm->pwm_freq * (pre_div + 1)) - 2;
			if (cnt <= 0xffff) /* 16bit */
				break;
		}
		bl_pwm->pwm_cnt = cnt;
		bl_pwm->pwm_pre_div = pre_div;
		if (lcd_debug_print_flag & LCD_DBG_PR_BL_NORMAL)
			BLPR("pwm_pre_div = %u\n", pre_div);
		break;
	}

	bl_pwm_channel_init(bl_pwm);

	if (lcd_debug_print_flag & LCD_DBG_PR_BL_NORMAL)
		BLPR("pwm_config_init: pwm_port 0x%x: freq=%u, pwm_cnt=%u\n",
		     bl_pwm->pwm_port, bl_pwm->pwm_freq, bl_pwm->pwm_cnt);
}

void bl_pwm_reg_print(struct bl_pwm_config_s *bl_pwm)
{
	unsigned int *pwm_reg;
	int port;

	if (!bl_pwm_ctrl_conf)
		return;

	switch (bl_pwm->pwm_port) {
	case BL_PWM_A:
	case BL_PWM_B:
	case BL_PWM_C:
	case BL_PWM_D:
	case BL_PWM_E:
	case BL_PWM_F:
	case BL_PWM_G:
	case BL_PWM_H:
	case BL_PWM_I:
	case BL_PWM_J:
		port = bl_pwm->pwm_port - BL_PWM_A;
		if (port >= bl_pwm_ctrl_conf->pwm_cnt) {
			BLERR("invalid pwm_port 0x%x\n", bl_pwm->pwm_port);
			return;
		}
		pwm_reg = bl_pwm_ctrl_conf->pwm_reg;
		break;
	case BL_PWM_AO_A:
	case BL_PWM_AO_B:
	case BL_PWM_AO_C:
	case BL_PWM_AO_D:
	case BL_PWM_AO_E:
	case BL_PWM_AO_F:
	case BL_PWM_AO_G:
	case BL_PWM_AO_H:
		port = bl_pwm->pwm_port - BL_PWM_AO_A;
		if (port >= bl_pwm_ctrl_conf->pwm_ao_cnt) {
			BLERR("invalid pwm_port 0x%x\n", bl_pwm->pwm_port);
			return;
		}
		pwm_reg = bl_pwm_ctrl_conf->pwm_ao_reg;
		break;
	default:
		return;
	}

	BLPR("pwm_reg    = 0x%08x\n", lcd_cbus_read(pwm_reg[port]));
}

void bl_pwm_vs_reg_dump(void)
{
	BLPR("pwm_vs_reg0  = 0x%08x\n", lcd_vcbus_read(VPU_VPU_PWM_V0));
	BLPR("pwm_vs_reg1  = 0x%08x\n", lcd_vcbus_read(VPU_VPU_PWM_V1));
	BLPR("pwm_vs_reg2  = 0x%08x\n", lcd_vcbus_read(VPU_VPU_PWM_V2));
	BLPR("pwm_vs_reg3  = 0x%08x\n", lcd_vcbus_read(VPU_VPU_PWM_V3));
	if (bl_pwm_ctrl_conf->pwm_vs_flag) {
		lcd_vcbus_setb(VPU_VPU_PWM_H0, 1, 31, 1);
		BLPR("pwm_vs_reg4  = 0x%08x\n", lcd_vcbus_read(VPU_VPU_PWM_V0));
		BLPR("pwm_vs_reg5  = 0x%08x\n", lcd_vcbus_read(VPU_VPU_PWM_V1));
		BLPR("pwm_vs_reg6  = 0x%08x\n", lcd_vcbus_read(VPU_VPU_PWM_V2));
		BLPR("pwm_vs_reg7  = 0x%08x\n", lcd_vcbus_read(VPU_VPU_PWM_V3));
		lcd_vcbus_setb(VPU_VPU_PWM_H0, 0, 31, 1);
	}
}

int aml_bl_pwm_chip_init(struct aml_lcd_data_s *pdata)
{
	switch (pdata->chip_type) {
	case LCD_CHIP_T3X:
		bl_pwm_ctrl_conf = &bl_pwm_ctrl_conf_t3x;
		break;
#if (IS_ENABLED(CONFIG_MESON_S6))
	case LCD_CHIP_S6:
		bl_pwm_ctrl_conf = &bl_pwm_ctrl_conf_s6;
		break;
#endif
#if (IS_ENABLED(CONFIG_MESON_T6D))
	case LCD_CHIP_T6D:
		bl_pwm_ctrl_conf = &bl_pwm_ctrl_conf_t6d;
		break;
#endif
	default:
		bl_pwm_ctrl_conf = NULL;
		break;
	}

	return 0;
}
