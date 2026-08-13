// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <malloc.h>
#include <dm.h>
#include <dm/pinctrl.h>
#include <asm/gpio.h>
#include <fdtdec.h>
#include <amlogic/media/vout/lcd/aml_lcd.h>
#ifdef CONFIG_AML_LCD_BL_LDIM
#include <amlogic/media/vout/lcd/bl_ldim.h>
#endif
#ifdef CONFIG_AML_LCD_BL_EXTERN
#include <amlogic/media/vout/lcd/bl_extern.h>
#endif
#include "lcd_bl.h"
#include "../lcd_reg.h"
#include "../lcd_common.h"
#include "env.h"

static int bl_index_lut[LCD_MAX_DRV];
static struct aml_bl_drv_s *bl_driver[LCD_MAX_DRV];

struct aml_bl_drv_s *aml_bl_get_driver(int index)
{
	if (index >= LCD_MAX_DRV)
		return NULL;
	if (!bl_driver[index])
		return NULL;
	if (bl_driver[index]->config.method >= BL_CTRL_MAX)
		return NULL;

	return bl_driver[index];
}

int aml_bl_get_state(int index)
{
	struct aml_bl_drv_s *bdrv = aml_bl_get_driver(index);

	if (!bdrv)
		return 0;

	return bdrv->state;
}

static struct bl_config_s *bl_check_valid(struct aml_bl_drv_s *bdrv)
{
	struct bl_config_s *bconf;
	unsigned int bconf_flag = 1;
#ifdef CONFIG_AML_LCD_BL_EXTERN
	struct aml_bl_extern_driver_s *bl_ext;
#endif
#ifdef CONFIG_AML_LCD_BL_LDIM
	struct aml_ldim_driver_s *ldim_drv;
#endif
	int i;

	if (!bdrv)
		return NULL;

	bconf = &bdrv->config;
	switch (bconf->method) {
	case BL_CTRL_PWM:
		if (!bconf->bl_pwm) {
			BLERR("no bl_pwm struct\n");
			bconf_flag = 0;
		}
		break;
	case BL_CTRL_PWM_COMBO:
		if (!bconf->bl_pwm_combo0) {
			BLERR("no bl_pwm_combo_0 struct\n");
			bconf_flag = 0;
		}
		if (!bconf->bl_pwm_combo1) {
			BLERR("no bl_pwm_combo_1 struct\n");
			bconf_flag = 0;
		}
		break;
	case BL_CTRL_PWM_ARRAY:
		for (i = 0; i < 4; i++) {
			if (!bconf->bl_pwm_array[i]) {
				BLERR("no bl_pwm_array_%d struct\n", i);
				bconf_flag = 0;
			}
		}
		break;
	case BL_CTRL_GPIO:
		break;
#ifdef CONFIG_AML_LCD_BL_LDIM
	case BL_CTRL_LOCAL_DIMMING:
		if (bdrv->index > 0) {
			BLERR("no ldim driver\n");
			bconf_flag = 0;
			break;
		}
		ldim_drv = aml_ldim_get_driver();
		if (!ldim_drv) {
			BLERR("no ldim driver\n");
			bconf_flag = 0;
		}
		break;
#endif
#ifdef CONFIG_AML_LCD_BL_EXTERN
	case BL_CTRL_EXTERN:
		bl_ext = aml_bl_extern_get_driver();
		if (!bl_ext) {
			BLERR("no bl_extern driver\n");
			bconf_flag = 0;
		}
		break;
#endif
	default:
		if (lcd_debug_print_flag & LCD_DBG_PR_BL_NORMAL)
			BLPR("invalid control_method: %d\n", bconf->method);
		bconf_flag = 0;
		break;
	}

	if (!bconf_flag)
		bconf = NULL;

	return bconf;
}

static char *bl_pinmux_str[BL_PINMUX_MAX] = {
	"pwm_on",               /* 0 */
	"pwm_vs_on",            /* 1 */
	"pwm_combo_0_1_on",     /* 2 */
	"pwm_combo_0_vs_1_on",  /* 3 */
	"pwm_combo_0_1_vs_on",  /* 4 */
	"pwm_off",              /* 5 */
	"pwm_combo_off",        /* 6 */
	"none",
};

static void bl_pwm_pinmux_ctrl(struct aml_bl_drv_s *bdrv, int status)
{
	struct bl_config_s *bconf = &bdrv->config;
	int index = 0xff;

	if (lcd_debug_print_flag & LCD_DBG_PR_BL_NORMAL)
		BLPR("[%d]: %s\n", bdrv->index, __func__);

	switch (bconf->method) {
	case BL_CTRL_PWM:
		if (status) {
			if (bconf->bl_pwm->pwm_port == BL_PWM_VS)
				index = 1;
			else
				index = 0;
		} else {
			index = 5;
		}
		break;
	case BL_CTRL_PWM_COMBO:
		if (status) {
			if (bconf->bl_pwm_combo0->pwm_port == BL_PWM_VS) {
				index = 3;
			} else {
				if (bconf->bl_pwm_combo1->pwm_port == BL_PWM_VS)
					index = 4;
				else
					index = 2;
			}
		} else {
			index = 6;
		}
		break;
	default:
		BLERR("[%d]: %s: wrong ctrl_mothod=%d\n",
		      bdrv->index, __func__, bconf->method);
		break;
	}

	if (index >= BL_PINMUX_MAX) {
		BLERR("[%d]: %s: pinmux index %d is invalid\n",
		      bdrv->index, __func__, index);
		return;
	}

	if (bdrv->pinmux_flag == index) {
		BLPR("[%d]: pinmux %s is already selected\n",
		     bdrv->index, bl_pinmux_str[index]);
		return;
	}

	/* request pwm pinmux */
	if (pinctrl_select_state(bdrv->udev, bl_pinmux_str[index])) {
		BLERR("[%d]: set pinmux %s error\n",
		      bdrv->index, bl_pinmux_str[index]);
	} else {
		if (lcd_debug_print_flag & LCD_DBG_PR_BL_NORMAL) {
			BLPR("[%d]: set pinmux %s\n",
			     bdrv->index, bl_pinmux_str[index]);
		}
	}
	bdrv->pinmux_flag = index;
}

static void bl_pwm_config_update(struct aml_bl_drv_s *bdrv)
{
#ifdef CONFIG_AML_LCD_BL_LDIM
	struct aml_ldim_driver_s *ldim_drv;
#endif
	int i;

	switch (bdrv->config.method) {
	case BL_CTRL_PWM:
		bl_pwm_config_init(bdrv->config.bl_pwm);
		break;
	case BL_CTRL_PWM_COMBO:
		bl_pwm_config_init(bdrv->config.bl_pwm_combo0);
		bl_pwm_config_init(bdrv->config.bl_pwm_combo1);
		break;
	case BL_CTRL_PWM_ARRAY:
		for (i = 0; i < 4; i++) {
			if (bdrv->config.bl_pwm_array[i])
				bl_pwm_config_init(bdrv->config.bl_pwm_array[i]);
		}
	break;
#ifdef CONFIG_AML_LCD_BL_LDIM
	case BL_CTRL_LOCAL_DIMMING:
		if (bdrv->index > 0) {
			BLERR("no ldim driver\n");
			break;
		}
		ldim_drv = aml_ldim_get_driver();
		if (!ldim_drv || !ldim_drv->dev_drv) {
			BLERR("ldim_drv is null\n");
			break;
		}
		if (ldim_drv->dev_drv->ldim_pwm_config.pwm_port >= BL_PWM_MAX)
			break;
		bl_pwm_config_init(&ldim_drv->dev_drv->ldim_pwm_config);
		if (ldim_drv->dev_drv->analog_pwm_config.pwm_port < BL_PWM_VS)
			bl_pwm_config_init(&ldim_drv->dev_drv->analog_pwm_config);
		break;
#endif
	default:
		break;
	}
}

int bl_bri_level_input_scale(struct aml_bl_drv_s *bdrv, int level)
{
	int out_level, half;
	unsigned long long temp = BL_LEVEL_FULL_SCALE;

	if (!bdrv || bdrv->config.level_in_scale == 0)
		return 0;
	if (level < 0)
		return level;

	half = (bdrv->config.level_in_scale + 1) >> 1;
	out_level = lcd_do_div(level * temp + half, bdrv->config.level_in_scale);
	return out_level;
}

int bl_bri_level_output_scale(struct aml_bl_drv_s *bdrv, int level)
{
	int in_level, half = (BL_LEVEL_FULL_SCALE + 1) >> 1;
	unsigned long long temp;

	if (!bdrv || bdrv->config.level_in_scale == 0)
		return 0;
	if (level < 0)
		return level;

	temp = bdrv->config.level_in_scale;
	in_level = lcd_do_div(level * temp + half, BL_LEVEL_FULL_SCALE);
	return in_level;
}

static void bl_set_level(struct aml_bl_drv_s *bdrv, int level)
{
	struct bl_config_s *bconf;
#ifdef CONFIG_AML_LCD_BL_EXTERN
	struct aml_bl_extern_driver_s *bl_ext;
#endif
#ifdef CONFIG_AML_LCD_BL_LDIM
	struct aml_ldim_driver_s *ldim_drv;
#endif
	int new_level = level, i;

	bconf = bl_check_valid(bdrv);
	if (!bconf)
		return;
	if (level < 0)
		return;

	BLPR("set level: %u, last level: %u\n", level, bdrv->level);
	/* level range check */
	if (level > bconf->level_max)
		new_level = bconf->level_max;
	if (level < bconf->level_min)
		new_level = bconf->level_min;
	BLPR("[%d]: set_level: %d(<-%d), last level: %d\n",
	     bdrv->index, new_level, level, bdrv->level);
	bdrv->level = new_level;

	switch (bconf->method) {
	case BL_CTRL_GPIO:
		break;
	case BL_CTRL_PWM:
		bl_pwm_set_level(bdrv, bconf->bl_pwm, new_level);
		break;
	case BL_CTRL_PWM_COMBO:
		bl_pwm_set_level(bdrv, bconf->bl_pwm_combo0, new_level);
		bl_pwm_set_level(bdrv, bconf->bl_pwm_combo1, new_level);
		break;
	case BL_CTRL_PWM_ARRAY:
		for (i = 0; i < 4; i++)
			bl_pwm_set_level(bdrv, bconf->bl_pwm_array[i], new_level);
		break;
#ifdef CONFIG_AML_LCD_BL_LDIM
	case BL_CTRL_LOCAL_DIMMING:
		if (bdrv->index > 0) {
			BLERR("no ldim driver\n");
			break;
		}
		ldim_drv = aml_ldim_get_driver();
		if (!ldim_drv || !ldim_drv->set_level) {
			BLERR("ldim_drv is null\n");
			break;
		}
		ldim_drv->set_level(ldim_drv, bdrv->level);
		break;
#endif
#ifdef CONFIG_AML_LCD_BL_EXTERN
	case BL_CTRL_EXTERN:
		bl_ext = aml_bl_extern_get_driver();
		if (bl_ext->set_level)
			bl_ext->set_level(new_level);
		else
			BLERR("bl_extern set_level is null\n");
		break;
#endif
	default:
		if (lcd_debug_print_flag & LCD_DBG_PR_BL_NORMAL)
			BLERR("wrong backlight control method\n");
		break;
	}
}

static void bl_power_en_ctrl(struct bl_config_s *bconf, int status)
{
	int gpio;
	char *str;

	if (bconf->en_gpio >= BL_GPIO_NUM_MAX) {
		gpio = LCD_GPIO_MAX;
	} else {
		str = bconf->gpio_name[bconf->en_gpio];
		gpio = lcd_gpio_name_map_num(str);
	}

	if (gpio >= LCD_GPIO_MAX)
		return;
	if (status)
		lcd_gpio_set(gpio, bconf->en_gpio_on);
	else
		lcd_gpio_set(gpio, bconf->en_gpio_off);
}

static void bl_pwm_en_ctrl(struct bl_config_s *bconf, int status)
{
	int i;
	switch (bconf->method) {
	case BL_CTRL_PWM:
		bl_pwm_en(bconf->bl_pwm, status);
		break;
	case BL_CTRL_PWM_COMBO:
		bl_pwm_en(bconf->bl_pwm_combo0, status);
		bl_pwm_en(bconf->bl_pwm_combo1, status);
		break;
	case BL_CTRL_PWM_ARRAY:
		for (i = 0; i < 4; i++)
			bl_pwm_en(bconf->bl_pwm_array[i], status);
		break;
	default:
		break;
	}
}

static void bl_power_ctrl(struct aml_bl_drv_s *bdrv, int status)
{
	int gpio, value;
	struct bl_config_s *bconf;
#ifdef CONFIG_AML_LCD_BL_EXTERN
	struct aml_bl_extern_driver_s *bl_ext;
#endif
#ifdef CONFIG_AML_LCD_BL_LDIM
	struct aml_ldim_driver_s *ldim_drv;
#endif

	bconf = bl_check_valid(bdrv);
	if (!bconf)
		return;

	gpio = bconf->en_gpio;
	value = status ? bconf->en_gpio_on : bconf->en_gpio_off;
	if (lcd_debug_print_flag & LCD_DBG_PR_BL_NORMAL)
		BLPR("status=%d gpio=%d value=%d\n", status, gpio, value);

	if (status) {
		/* bl_off_policy */
		if (bdrv->bl_off_policy != BL_OFF_POLICY_NONE) {
			BLPR("bl_off_policy=%d for bl_off\n", bdrv->bl_off_policy);
			return;
		}

		bdrv->state = 1;
		bl_pwm_en_ctrl(bconf, 1);
		/* check if factory test */
		if (bdrv->factory_bl_on_delay >= 0) {
			BLPR("%s: factory power_on_delay!\n", __func__);
			if (bdrv->factory_bl_on_delay > 0)
				mdelay(bdrv->factory_bl_on_delay);
		} else {
			if (bconf->power_on_delay > 0)
				mdelay(bconf->power_on_delay);
		}

		switch (bconf->method) {
		case BL_CTRL_GPIO:
			bl_power_en_ctrl(bconf, 1);
			break;
		case BL_CTRL_PWM:
		case BL_CTRL_PWM_COMBO:
		case BL_CTRL_PWM_ARRAY:
			if (bconf->en_sequence_reverse) {
				/* step 1: power on enable */
				bl_power_en_ctrl(bconf, 1);
				if (bconf->pwm_on_delay > 0)
					mdelay(bconf->pwm_on_delay);
				/* step 2: power on pwm */
				bl_pwm_pinmux_ctrl(bdrv, 1);
			} else {
				/* step 1: power on pwm */
				bl_pwm_pinmux_ctrl(bdrv, 1);
				if (bconf->pwm_on_delay > 0)
					mdelay(bconf->pwm_on_delay);
				/* step 2: power on enable */
				bl_power_en_ctrl(bconf, 1);
			}
			break;
#ifdef CONFIG_AML_LCD_BL_LDIM
		case BL_CTRL_LOCAL_DIMMING:
			if (bdrv->index > 0) {
				BLERR("no ldim driver\n");
				break;
			}
			ldim_drv = aml_ldim_get_driver();
			if (!ldim_drv || !ldim_drv->power_on) {
				BLERR("no ldim driver\n");
				break;
			}
			if (bconf->en_sequence_reverse) {
				/* step 1: power on enable */
				bl_power_en_ctrl(bconf, 1);
				/* step 2: power on ldim */
				ldim_drv->power_on(ldim_drv);
			} else {
				/* step 1: power on ldim */
				ldim_drv->power_on(ldim_drv);
				/* step 2: power on enable */
				bl_power_en_ctrl(bconf, 1);
			}
			break;
#endif
#ifdef CONFIG_AML_LCD_BL_EXTERN
		case BL_CTRL_EXTERN:
			bl_ext = aml_bl_extern_get_driver();
			if (bconf->en_sequence_reverse) {
				/* step 1: power on enable */
				bl_power_en_ctrl(bconf, 1);
				/* step 2: power on bl_extern */
				if (bl_ext->power_on)
					bl_ext->power_on();
				else
					BLERR("bl_extern power on is null\n");
			} else {
				/* step 1: power on bl_extern */
				if (bl_ext->power_on)
					bl_ext->power_on();
				else
					BLERR("bl_extern power on is null\n");
				/* step 2: power on enable */
				bl_power_en_ctrl(bconf, 1);
			}
			break;
#endif
		default:
			if (lcd_debug_print_flag & LCD_DBG_PR_BL_NORMAL)
				BLERR("wrong backlight control method\n");
			break;
		}
	} else {
		bdrv->state = 0;
		switch (bconf->method) {
		case BL_CTRL_GPIO:
			bl_power_en_ctrl(bconf, 0);
			break;
		case BL_CTRL_PWM:
		case BL_CTRL_PWM_COMBO:
		case BL_CTRL_PWM_ARRAY:
			if (bconf->en_sequence_reverse == 1) {
				/* step 1: power off pwm */
				bl_pwm_pinmux_ctrl(bdrv, 0);
				if (bconf->pwm_off_delay > 0)
					mdelay(bconf->pwm_off_delay);
				/* step 2: power off enable */
				bl_power_en_ctrl(bconf, 0);
			} else {
				/* step 1: power off enable */
				bl_power_en_ctrl(bconf, 0);
				/* step 2: power off pwm */
				if (bconf->pwm_off_delay > 0)
					mdelay(bconf->pwm_off_delay);
				bl_pwm_pinmux_ctrl(bdrv, 0);
			}
			bl_pwm_en_ctrl(bconf, 0);
			break;
#ifdef CONFIG_AML_LCD_BL_LDIM
		case BL_CTRL_LOCAL_DIMMING:
			if (bdrv->index > 0) {
				BLERR("no ldim driver\n");
				break;
			}
			ldim_drv = aml_ldim_get_driver();
			if (!ldim_drv || !ldim_drv->power_off) {
				BLERR("no ldim driver\n");
				break;
			}
			if (bconf->en_sequence_reverse == 1) {
				/* step 1: power off ldim */
				ldim_drv->power_off(ldim_drv);
				/* step 2: power off enable */
				bl_power_en_ctrl(bconf, 0);
			} else {
				/* step 1: power off enable */
				bl_power_en_ctrl(bconf, 0);
				/* step 2: power off ldim */
				ldim_drv->power_off(ldim_drv);
			}
			break;
#endif
#ifdef CONFIG_AML_LCD_BL_EXTERN
		case BL_CTRL_EXTERN:
			bl_ext = aml_bl_extern_get_driver();
			if (bconf->en_sequence_reverse == 1) {
				/* step 1: power off bl_extern */
				if (bl_ext->power_off)
					bl_ext->power_off();
				else
					BLERR("bl_extern: power off is null\n");
				/* step 2: power off enable */
				bl_power_en_ctrl(bconf, 0);
			} else {
				/* step 1: power off enable */
				bl_power_en_ctrl(bconf, 0);
				/* step 2: power off bl_extern */
				if (bl_ext->power_off)
					bl_ext->power_off();
				else
					BLERR("bl_extern: power off is null\n");
			}
			break;
#endif
		default:
			if (lcd_debug_print_flag & LCD_DBG_PR_BL_NORMAL)
				BLERR("wrong backlight control method\n");
			break;
		}
		if (bconf->power_off_delay > 0)
			mdelay(bconf->power_off_delay);
	}
	if (bconf->bl_hold_on > 0)
		mdelay(bconf->bl_hold_on);

	BLPR("%s: %d\n", __func__, status);
}

static void bl_power_init_on(struct aml_bl_drv_s *bdrv)
{
	int i;

	switch (bdrv->config.method) {
	case BL_CTRL_PWM:
		bl_set_pwm(bdrv->config.bl_pwm);
		break;
	case BL_CTRL_PWM_COMBO:
		bl_set_pwm(bdrv->config.bl_pwm_combo0);
		bl_set_pwm(bdrv->config.bl_pwm_combo1);
		break;
	case BL_CTRL_PWM_ARRAY:
		for (i = 0; i < 4; i++) {
			if (!bdrv->config.bl_pwm_array[i])
				continue;
			bl_set_pwm(bdrv->config.bl_pwm_array[i]);
		}
		break;
	default:
		break;
	}
}

static void bl_power_init_off(struct aml_bl_drv_s *bdrv)
{
	struct bl_config_s *bconf;

	bconf = bl_check_valid(bdrv);
	if (!bconf)
		return;

	if (lcd_debug_print_flag & LCD_DBG_PR_BL_NORMAL) {
		BLPR("[%d]: init_off: gpio=%d value=%d\n",
		      bdrv->index,
		      bconf->en_gpio, bconf->en_gpio_off);
	}

	bdrv->state = 0;
	switch (bconf->method) {
	case BL_CTRL_PWM:
	case BL_CTRL_PWM_COMBO:
	case BL_CTRL_PWM_ARRAY:
		bl_power_en_ctrl(bconf, 0);
		bl_pwm_pinmux_ctrl(bdrv, 0);
		break;
	default:
		bl_power_en_ctrl(bconf, 0);
		break;
	}

	if (lcd_debug_print_flag & LCD_DBG_PR_BL_NORMAL)
		BLPR("%s finish\n", __func__);
}

static struct aml_bl_drv_s *bl_driver_add(unsigned char index)
{
	struct aml_bl_drv_s *bdrv = bl_driver[index];

	if (bl_index_lut[index] >= BL_INDEX_INVALID)
		return NULL;

	if (!bdrv) {
		bdrv = (struct aml_bl_drv_s *)malloc(sizeof(struct aml_bl_drv_s));
		if (!bdrv) {
			BLERR("%s: Not enough memory\n", __func__);
			return NULL;
		}
	}
	bl_driver[index] = bdrv;
	memset(bdrv, 0, sizeof(struct aml_bl_drv_s));
	bdrv->index = index;
	bdrv->data = aml_lcd_get_data();

	/* default config */
	bdrv->level = -1;
	bdrv->config.index = bl_index_lut[index];
	bdrv->config.method = BL_CTRL_MAX;
	bdrv->config.level_max = BL_LEVEL_FULL_SCALE;
	bdrv->config.level_min = 0;
	bdrv->config.level_default = -1; //invalid
	bdrv->config.en_gpio = 0xff;
	bdrv->config.extern_index = 0xff;
	bdrv->factory_bl_on_delay = -1;

	return bdrv;
}

void bl_driver_remove(unsigned char index)
{
	if (!bl_driver[index])
		return;
	memset(bl_driver[index], 0, sizeof(struct aml_bl_drv_s));
	free(bl_driver[index]);
	bl_driver[index] = NULL;
}

static int aml_bl_probe(struct udevice *dev)
{
	struct aml_bl_drv_s *bdrv;
	unsigned int index;
	int ret;

	ret = dev_read_u32(dev, "index", &index);
	if (ret) {
		if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL)
			BLPR("%s: no index exist, default to 0\n", __func__);
		index = 0;
	}
	bl_driver_remove(index);

	bdrv = bl_driver_add(index);
	if (!bdrv)
		return -1;
	bdrv->udev = dev;

	ret = aml_bl_load_config(bdrv, lcd_get_dt_addr());
	if (ret) {
		bl_driver_remove(index);
		return -1;
	}
	bl_pwm_config_update(bdrv);
	bl_power_init_off(bdrv);
	return 0;
}

int aml_bl_reprobe(unsigned int index, char *dt_addr, unsigned char load_id)
{
	int ret;
	struct aml_bl_drv_s *bdrv = bl_driver[index];
	struct udevice *dev;

	if (!bdrv)
		return -1;
	dev = bdrv->udev;
	bl_driver_remove(index);
	bdrv = bl_driver_add(index);
	if (!bdrv)
		return -1;
	bdrv->udev = dev;

	ret = aml_bl_reload_config(bdrv, dt_addr, load_id);
	if (ret) {
		bl_driver_remove(index);
		return -1;
	}
	bl_pwm_config_update(bdrv);
	bl_power_init_off(bdrv);
	return 0;
}

int aml_bl_index_add(int drv_index, int conf_index)
{
	if (drv_index >= LCD_MAX_DRV) {
		BLERR("%s: invalid drv_index: %d\n", __func__, drv_index);
		return -1;
	}

	bl_index_lut[drv_index] = conf_index;
	if (lcd_debug_print_flag & LCD_DBG_PR_BL_NORMAL) {
		BLPR("%s: drv_index %d, config index: %d\n",
			__func__, drv_index, conf_index);
	}
	return 0;
}

int aml_bl_init(int index)
{
	if (index >= LCD_MAX_DRV)
		return -1;

	bl_driver_remove(index);
	bl_index_lut[index] = BL_INDEX_INVALID;

	return 0;
}

static void bl_state_sync_with_bootctrl(int index, int state)
{
	struct aml_lcd_drv_s *pdrv = aml_lcd_get_driver(index);

	if (pdrv->boot_ctrl.init_level == LCD_INIT_LEVEL_PREBOOT && state) {
		pdrv->boot_ctrl.init_level = LCD_INIT_LEVEL_NORMAL;
		env_set("lcd_init_level", LCD_INIT_LEVEL_NORMAL);
	}
	lcd_update_ctrl_bootargs(pdrv);

	if (lcd_debug_print_flag & LCD_DBG_PR_BL_NORMAL)
		BLPR("%s: index=%d, init_level=%d, bl_state=%d, state=%d\n",
		     __func__, index, pdrv->boot_ctrl.init_level, pdrv->boot_ctrl.bl_state, state);
}

static void bl_driver_enable_ctrl(int index, int state)
{
	struct aml_bl_drv_s *bdrv;
	int level;

	bdrv = aml_bl_get_driver(index);
	if (!bdrv)
		return;

	if (state) {
		if (bdrv->state) {
			BLPR("already enabled\n");
			return;
		}
		bl_power_init_on(bdrv);
		if (bdrv->config.level_default >= 0) {
			level = bl_bri_level_input_scale(bdrv, bdrv->config.level_default);
			bl_set_level(bdrv, level);
		}
		bl_power_ctrl(bdrv, 1);
	} else {
		if (!bdrv->state) {
			BLPR("already disabled\n");
			return;
		}
		bl_power_ctrl(bdrv, 0);
	}
}

/* manual cmd api, need sync bootctrl */
void aml_bl_driver_enable(int index)
{
	bl_driver_enable_ctrl(index, 1);
	bl_state_sync_with_bootctrl(index, 1);
}

/* manual cmd api, need sync bootctrl */
void aml_bl_driver_disable(int index)
{
	bl_driver_enable_ctrl(index, 0);
	bl_state_sync_with_bootctrl(index, 0);
}

/* driver flow api, no need sync bootctrl */
void aml_bl_lcd_on_ctrl(int index)
{
	struct aml_lcd_drv_s *pdrv = aml_lcd_get_driver(index);

	if (pdrv->boot_ctrl.init_level == LCD_INIT_LEVEL_PREBOOT) {
		BLPR("[%d]: preboot bypass backlight on\n", index);
		return;
	}
	bl_driver_enable_ctrl(index, 1);
}

/* driver flow api, no need sync bootctrl */
void aml_bl_lcd_off_ctrl(int index)
{
	bl_driver_enable_ctrl(index, 0);
}

void aml_bl_set_level(int index, int level)
{
	struct aml_bl_drv_s *bdrv;
	int level_new;

	bdrv = aml_bl_get_driver(index);
	if (!bdrv)
		return;

	level_new = bl_bri_level_input_scale(bdrv, level);
	bl_set_level(bdrv, level_new);
}

int aml_bl_get_level(int index)
{
	struct aml_bl_drv_s *bdrv;

	bdrv = aml_bl_get_driver(index);
	if (!bdrv)
		return 0;

	return bl_bri_level_output_scale(bdrv, bdrv->level);
}

void aml_bl_config_print(int index)
{
	struct aml_bl_drv_s *bdrv;

	bdrv = aml_bl_get_driver(index);
	if (!bdrv)
		return;

	bl_config_print(bdrv);
}

static const struct udevice_id backlight_match_table[] = {
#if defined(CONFIG_MESON_S6)
	{
		.compatible = "amlogic, backlight-s6",
	},
#endif
#if defined(CONFIG_MESON_T6D)
	{
		.compatible = "amlogic, backlight-t6d",
	},
#endif
#if defined(CONFIG_MESON_T6W)
	{
		.compatible = "amlogic, backlight-t6w",
	},
#endif
#if defined(CONFIG_MESON_T6X)
	{
		.compatible = "amlogic, backlight-t6x",
	},
#endif
	{}
};

U_BOOT_DRIVER(backlight) = {
	.name = "backlight",
	.id = UCLASS_MISC,
	.of_match = of_match_ptr(backlight_match_table),
	.probe = aml_bl_probe,
};

