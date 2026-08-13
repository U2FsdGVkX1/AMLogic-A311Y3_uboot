// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <malloc.h>
#include <spi.h>
#include <asm/amlogic/arch/gpio.h>
#include <fdtdec.h>
#include <amlogic/media/vout/lcd/aml_lcd.h>
#include <amlogic/media/vout/lcd/aml_bl.h>
#include <amlogic/media/vout/lcd/bl_ldim.h>
#include "../../lcd_common.h"
#include "../lcd_bl.h"
#include "ldim_drv.h"
#include "ldim_dev_drv.h"
#include "env.h"

static int ldim_dev_zone_mapping_load(struct ldim_dev_driver_s *dev_drv, const char *path)
{
	int bin_size = 0, map_size;
	unsigned char *buf;
	int i, j, cnt;
	char size_err = 0;

	//ret = handle_ldim_dev_zone_mapping_get(buf, size, path);
	buf = model_read_bin_to_buffer(path, &bin_size);
	if (!buf) {
		LDIMERR("%s: load bin: %s error\n", __func__, path);
		return -1;
	}

	if (dev_drv->type == LDIM_DEV_TYPE_ABCON) {
		cnt = (bin_size + 1) >> 1;

		//not over max mapping size
		map_size = dev_drv->zone_num * 4;
		if (bin_size > map_size)
			size_err = 1;
	} else {
		/* 2byte per zone */
		map_size = dev_drv->zone_num * 2;
		if (bin_size != map_size)
			size_err = 1;

		cnt = dev_drv->zone_num;
	}

	if (size_err) {
		memset(buf, 0, bin_size);
		free(buf);
		LDIMERR("%s: bin_size(%d) not match zone_num(%d), exit!\n",
			__func__, bin_size, dev_drv->zone_num);
		return -1;
	}

	for (i = 0; i < cnt; i++) {
		j = 2 * i;
		dev_drv->bl_mapping[i] = buf[j] | (buf[j + 1] << 8);
	}

	LDIMPR("%s: load bin: %s finish, size=%d\n", __func__, path, bin_size);
	memset(buf, 0, bin_size);
	free(buf);
	return 0;
}

#ifdef CONFIG_OF_LIBFDT
static int ldim_dev_init_table_handle_dts(char *dtaddr, int nodeoffset,
					  struct ldim_dev_driver_s *dev_drv)
{
	int len_on, len_off, init_len, init_buf_size;
	unsigned int *init_buf;
	char *init_on, *init_off;
	unsigned char *table;
	int i = 0;

	init_on = (char *)fdt_getprop(dtaddr, nodeoffset, "init_on", &len_on);
	if (!init_on) {
		LDIMERR("%s: get init_on failed\n", dev_drv->name);
		return -1;
	}
	init_off = (char *)fdt_getprop(dtaddr, nodeoffset, "init_off", &len_off);
	if (!init_off) {
		LDIMERR("%s: get init_off failed\n", dev_drv->name);
		return -1;
	}
	len_on /= 4;
	len_off /= 4;
	init_len = len_on >= len_off ? len_on : len_off;
	if (init_len <= 0)
		return 0;

	init_buf_size = init_len * sizeof(unsigned int);
	init_buf = (unsigned int *)malloc(init_buf_size);
	if (!init_buf) {
		LDIMERR("%s: alloc memory error\n", __func__);
		return -1;
	}

	//init_on
	for (i = 0; i < len_on; i++)
		init_buf[i] = be32_to_cpup((((u32 *)init_on) + i));
	table = lcd_init_table_load_array("ldim_dev_init_on", LCD_EXT_CMD_SIZE_DYNAMIC,
				init_buf, len_on, LDIM_INIT_ON_MAX, &init_len);
	if (!table)
		goto ldim_dev_init_table_handle_dts_err;
	dev_drv->init_on = table;
	dev_drv->init_on_cnt = init_len;

	//init_off
	for (i = 0; i < len_off; i++)
		init_buf[i] = be32_to_cpup((((u32 *)init_off) + i));
	table = lcd_init_table_load_array("ldim_dev_init_off", LCD_EXT_CMD_SIZE_DYNAMIC,
				init_buf, len_off, LDIM_INIT_OFF_MAX, &init_len);
	if (!table)
		goto ldim_dev_init_table_handle_dts_err;
	dev_drv->init_off = table;
	dev_drv->init_off_cnt = init_len;

	dev_drv->init_loaded = 1;

	memset(init_buf, 0, init_buf_size);
	free(init_buf);
	return 0;

ldim_dev_init_table_handle_dts_err:
	memset(init_buf, 0, init_buf_size);
	free(init_buf);
	return -1;
}

static int ldim_dev_get_config_from_dts(struct ldim_dev_driver_s *dev_drv,
					char *dt_addr, int index)
{
	int child_offset;
	char *propname, *propdata;
	const char *str;
	int temp;
	struct bl_pwm_config_s *bl_pwm;
	char dbg_str[160];
	int i, dbg_str_len = 0, ret = 0;

	if (lcd_debug_print_flag & LCD_DBG_PR_BL_NORMAL)
		LDIMPR("load ldim_dev config from dts\n");

	/* get device config */
	propname = (char *)malloc(50);
	if (!propname) {
		LDIMERR("%s: propname malloc failed\n", __func__);
		return -1;
	}
	memset(propname, 0, 50);
	sprintf(propname, "/local_dimming_device/ldim_dev_%d", dev_drv->index);
	child_offset = fdt_path_offset(dt_addr, propname);
	if (child_offset < 0) {
		LDIMERR("not find %s node: %s\n", propname, fdt_strerror(child_offset));
		free(propname);
		return -1;
	}
	free(propname);

	propdata = (char *)fdt_getprop(dt_addr, child_offset, "ldim_dev_name", NULL);
	if (!propdata)
		LDIMERR("failed to get ldim_dev_name\n");
	else
		strlcpy(dev_drv->name, propdata, LDIM_DEV_NAME_MAX);

	propdata = (char *)fdt_getprop(dt_addr, child_offset, "type", NULL);
	if (!propdata) {
		LDIMERR("failed to get type\n");
		return -1;
	}
	dev_drv->type = be32_to_cpup((u32 *)propdata);
	if (dev_drv->type >= LDIM_DEV_TYPE_MAX) {
		LDIMERR("invalid type %d\n", dev_drv->type);
		return -1;
	}

	switch (dev_drv->type) {
	case LDIM_DEV_TYPE_SPI:
		/* get spi config */
		propdata = (char *)fdt_getprop(dt_addr, child_offset, "spi_bus_num", NULL);
		if (!propdata) {
			LDIMERR("failed to get spi_bus_num\n");
		} else {
			temp = be32_to_cpup((u32 *)propdata);
			dev_drv->spi_info.bus_num[0] = temp & 0xf;
			dev_drv->spi_info.bus_num[1] = (temp >> 4) & 0xf;
			if (dev_drv->spi_info.bus_num[1])
				dev_drv->spi_info.spi_dev_num = 2;
		}

		propdata = (char *)fdt_getprop(dt_addr, child_offset, "spi_chip_select", NULL);
		if (!propdata) {
			LDIMERR("failed to get spi_chip_select\n");
		} else {
			temp = be32_to_cpup((u32 *)propdata);
			dev_drv->spi_info.chip_select[0] = temp & 0xf;
			dev_drv->spi_info.chip_select[1] = (temp >> 4) & 0xf;
		}

		propdata = (char *)fdt_getprop(dt_addr, child_offset, "spi_max_frequency", NULL);
		if (!propdata)
			LDIMERR("failed to get spi_max_frequency\n");
		else
			dev_drv->spi_info.max_speed_hz = be32_to_cpup((u32 *)propdata);

		propdata = (char *)fdt_getprop(dt_addr, child_offset, "spi_mode", NULL);
		if (!propdata)
			LDIMERR("failed to get spi_mode\n");
		else
			dev_drv->spi_info.mode = be32_to_cpup((u32 *)propdata);

		if (lcd_debug_print_flag & LCD_DBG_PR_BL_NORMAL) {
			LDIMPR("spi bus_num=%d:%d, chip_select=%d:%d",
				dev_drv->spi_info.bus_num[0], dev_drv->spi_info.bus_num[1],
				dev_drv->spi_info.chip_select[0], dev_drv->spi_info.chip_select[1]);
			LDIMPR("max_frequency=%d, mode=%d, spi_dev_num:%d\n",
			       dev_drv->spi_info.max_speed_hz,
			       dev_drv->spi_info.mode, dev_drv->spi_info.spi_dev_num);
		}

		propdata = (char *)fdt_getprop(dt_addr, child_offset, "spi_cs_delay", NULL);
		if (!propdata) {
			LDIMERR("failed to get spi_cs_delay\n");
		} else {
			dev_drv->cs2clk_delay = be32_to_cpup((u32 *)propdata);
			dev_drv->clk2cs_delay = be32_to_cpup((((u32 *)propdata) + 1));
		}
		propdata = (char *)fdt_getprop(dt_addr, child_offset, "spi_line_n", NULL);
		if (!propdata) {
			LDIMERR("failed to get spi_line_n\n");
			dev_drv->use_ctrl_cs = 0;
		} else {
			temp = (unsigned int)(be32_to_cpup((u32 *)propdata));
			if (temp)
				dev_drv->use_ctrl_cs = 1;
			else
				dev_drv->use_ctrl_cs = 0;
		}
		if (lcd_debug_print_flag & LCD_DBG_PR_BL_NORMAL) {
			LDIMPR("cs2clk_delay=%dus, clk2cs_delay=%dus, use_ctrl_cs=%d\n",
			       dev_drv->cs2clk_delay,
			       dev_drv->clk2cs_delay,
				   dev_drv->use_ctrl_cs);
		}
		break;
	default:
		break;
	}

	/* ldim pwm */
	bl_pwm = &dev_drv->ldim_pwm_config;
	bl_pwm->drv_index = 0; /* only venc0 support ldim */
	propdata = (char *)fdt_getprop(dt_addr, child_offset, "ldim_pwm_port", NULL);
	if (!propdata) {
		LDIMERR("failed to get ldim_pwm_port\n");
		bl_pwm->pwm_port = BL_PWM_MAX;
	} else {
		bl_pwm->pwm_port = bl_pwm_str_to_num(propdata);
		LDIMPR("ldim_pwm_port: %s(0x%x)\n", propdata, bl_pwm->pwm_port);
	}
	if (bl_pwm->pwm_port < BL_PWM_MAX) {
		propdata = (char *)fdt_getprop(dt_addr, child_offset, "ldim_pwm_attr", NULL);
		if (!propdata) {
			LDIMERR("failed to get ldim_pwm_attr\n");
			bl_pwm->pwm_method = BL_PWM_POSITIVE;
			if (bl_pwm->pwm_port == BL_PWM_VS)
				bl_pwm->pwm_freq = 1;
			else
				bl_pwm->pwm_freq = 60;
			bl_pwm->pwm_duty_dft = 50;
			bl_pwm->pwm_phase = 0;
		} else {
			bl_pwm->pwm_method = be32_to_cpup((u32 *)propdata);
			temp = be32_to_cpup((((u32 *)propdata) + 1));
			if (bl_pwm->pwm_port == BL_PWM_VS) {
				bl_pwm->pwm_freq = temp & 0xff;
				bl_pwm->pwm_phase = (temp >> 8) & 0xffffff;
			} else {
				bl_pwm->pwm_freq = temp;
				bl_pwm->pwm_phase = 0;
			}
			bl_pwm->pwm_duty_dft = be32_to_cpup((((u32 *)propdata) + 2));
		}
		bl_pwm->pwm_duty = bl_pwm->pwm_duty_dft;
		bl_pwm->pwm_duty_save = bl_pwm->pwm_duty_dft;
		if (bl_pwm->pwm_port == BL_PWM_VS) {
			if (bl_pwm->pwm_freq > 8) {
				LDIMERR("pwm_vs wrong freq %d\n", bl_pwm->pwm_freq);
				bl_pwm->pwm_freq = BL_FREQ_VS_DEFAULT;
			}
		} else {
			if (bl_pwm->pwm_freq > XTAL_HALF_FREQ_HZ)
				bl_pwm->pwm_freq = XTAL_HALF_FREQ_HZ;
		}
		bl_pwm_config_init(bl_pwm);
		LDIMPR("get ldim_pwm pol=%d, freq=%d, duty=%d, phase=%d\n",
		       bl_pwm->pwm_method, bl_pwm->pwm_freq, bl_pwm->pwm_duty, bl_pwm->pwm_phase);
	}

	/* analog pwm */
	bl_pwm = &dev_drv->analog_pwm_config;
	bl_pwm->drv_index = 0; /* only venc0 support ldim */
	propdata = (char *)fdt_getprop(dt_addr, child_offset, "analog_pwm_port", NULL);
	if (!propdata)
		bl_pwm->pwm_port = BL_PWM_MAX;
	else
		bl_pwm->pwm_port = bl_pwm_str_to_num(propdata);
	if (bl_pwm->pwm_port < BL_PWM_MAX) {
		LDIMPR("find analog_pwm_port: %s(0x%x)\n", propdata, bl_pwm->pwm_port);
		propdata = (char *)fdt_getprop(dt_addr, child_offset, "analog_pwm_attr", NULL);
		if (!propdata) {
			LDIMERR("failed to get analog_pwm_attr\n");
			bl_pwm->pwm_method = BL_PWM_POSITIVE;
			if (bl_pwm->pwm_port == BL_PWM_VS)
				bl_pwm->pwm_freq = 1;
			else
				bl_pwm->pwm_freq = 60;
			bl_pwm->pwm_duty_dft = 50;
			bl_pwm->pwm_phase = 0;
		} else {
			bl_pwm->pwm_method = be32_to_cpup((u32 *)propdata);
			temp = be32_to_cpup((((u32 *)propdata) + 1));
			if (bl_pwm->pwm_port == BL_PWM_VS) {
				bl_pwm->pwm_freq = temp & 0xff;
				bl_pwm->pwm_phase = (temp >> 8) & 0xffffff;
			} else {
				bl_pwm->pwm_freq = temp;
				bl_pwm->pwm_phase = 0;
			}
			bl_pwm->pwm_duty_max = be32_to_cpup((((u32 *)propdata) + 2));
			bl_pwm->pwm_duty_min = be32_to_cpup((((u32 *)propdata) + 3));
			bl_pwm->pwm_duty_dft = be32_to_cpup((((u32 *)propdata) + 4));
		}
		bl_pwm->pwm_duty = bl_pwm->pwm_duty_dft;
		bl_pwm->pwm_duty_save = bl_pwm->pwm_duty_dft;
		if (bl_pwm->pwm_freq > XTAL_HALF_FREQ_HZ)
			bl_pwm->pwm_freq = XTAL_HALF_FREQ_HZ;
		bl_pwm_config_init(bl_pwm);
		LDIMPR("get analog_pwm pol=%d, freq=%d, duty=%d [%d~%d], phase=%d\n",
		       bl_pwm->pwm_method, bl_pwm->pwm_freq, bl_pwm->pwm_duty,
		       bl_pwm->pwm_duty_min, bl_pwm->pwm_duty_max, bl_pwm->pwm_phase);
	}

	propdata = (char *)fdt_getprop(dt_addr, child_offset, "ldim_pwm_pinmux_sel", NULL);
	if (propdata) {
		LDIMPR("find custom ldim_pwm_pinmux_sel: %s\n", propdata);
		strlcpy(dev_drv->pinmux_name, propdata, LDIM_DEV_NAME_MAX);
	}

	propdata = (char *)fdt_getprop(dt_addr, child_offset, "en_gpio_on_off", NULL);
	if (!propdata) {
		LDIMERR("failed to get en_gpio_on_off\n");
	} else {
		dev_drv->en_gpio = be32_to_cpup((u32 *)propdata);
		dev_drv->en_gpio_on = be32_to_cpup((((u32 *)propdata) + 1));
		dev_drv->en_gpio_off = be32_to_cpup((((u32 *)propdata) + 2));
	}
	if (lcd_debug_print_flag & LCD_DBG_PR_BL_NORMAL) {
		LDIMPR("en_gpio=%s(%d), en_gpio_on=%d, en_gpio_off=%d\n",
		       dev_drv->gpio_name[dev_drv->en_gpio],
		       dev_drv->en_gpio, dev_drv->en_gpio_on,
		       dev_drv->en_gpio_off);
	}

	propdata = (char *)fdt_getprop(dt_addr, child_offset, "write_check", NULL);
	if (!propdata)
		dev_drv->write_check = 0;
	else
		dev_drv->write_check = (unsigned char)(be32_to_cpup((u32 *)propdata));
	if (lcd_debug_print_flag & LCD_DBG_PR_BL_NORMAL)
		LDIMPR("write_check=%d\n", dev_drv->write_check);

	propdata = (char *)fdt_getprop(dt_addr, child_offset, "dim_max_min", NULL);
	if (!propdata) {
		LDIMERR("failed to get dim_max_min\n");
		return -1;
	}
	dev_drv->dim_max = be32_to_cpup((u32 *)propdata);
	dev_drv->dim_min = be32_to_cpup((((u32 *)propdata) + 1));
	if (lcd_debug_print_flag & LCD_DBG_PR_BL_NORMAL) {
		LDIMPR("dim_max=0x%03x, dim_min=0x%03x\n",
		       dev_drv->dim_max, dev_drv->dim_min);
	}

	propdata = (char *)fdt_getprop(dt_addr, child_offset, "mcu_header", NULL);
	if (!propdata) {
		LDIMERR("failed to get mcu_header\n");
		dev_drv->mcu_header = 0;
	} else {
		dev_drv->mcu_header = (unsigned int)(be32_to_cpup((u32 *)propdata));
	}

	propdata = (char *)fdt_getprop(dt_addr, child_offset, "mcu_dim", NULL);
	if (!propdata) {
		LDIMERR("failed to get mcu_dim\n");
		dev_drv->mcu_dim = 0;
	} else {
		dev_drv->mcu_dim = (unsigned int)(be32_to_cpup((u32 *)propdata));
	}

	propdata = (char *)fdt_getprop(dt_addr, child_offset, "chip_count", NULL);
	if (!propdata)
		dev_drv->chip_cnt = 1;
	else
		dev_drv->chip_cnt = be32_to_cpup((u32 *)propdata);

	propdata = (char *)fdt_getprop(dt_addr, child_offset, "ldim_zone_mapping_path", NULL);
	if (propdata) {
		LDIMPR("%s:find custom ldim_zone_mapping_path\n", __func__);
		str = propdata;
		ret = ldim_dev_zone_mapping_load(dev_drv, str);
		if (ret) {
			for (i = 0; i < dev_drv->zone_num; i++)
				dev_drv->bl_mapping[i] = (unsigned short)i;
		}
		goto ldim_dev_get_config_from_dts_next;
	}
	propdata = (char *)fdt_getprop(dt_addr, child_offset, "ldim_zone_mapping", NULL);
	if (!propdata) {
		propdata = (char *)fdt_getprop(dt_addr, child_offset, "ldim_region_mapping", NULL);
		if (!propdata) {
			for (i = 0; i < dev_drv->zone_num; i++)
				dev_drv->bl_mapping[i] = (unsigned short)i;
			goto ldim_dev_get_config_from_dts_next;
		}
	}
	LDIMPR("%s:find custom ldim_zone_mapping\n", __func__);
	for (i = 0; i < dev_drv->zone_num; i++)
		dev_drv->bl_mapping[i] = (unsigned short)be32_to_cpup((((u32 *)propdata) + i));

ldim_dev_get_config_from_dts_next:
	dbg_str_len += sprintf(dbg_str + dbg_str_len, "mcu_header=0x%08x, mcu_dim=0x%08x, ",
		dev_drv->mcu_header, dev_drv->mcu_dim);
	sprintf(dbg_str + dbg_str_len, "chip_cnt:%d, cus pwm_pinmux_sel:%s",
		dev_drv->chip_cnt, dev_drv->pinmux_name);
	LDIMPR("load dts config: %s[%d]: type:%d, %s\n",
	       dev_drv->name, dev_drv->index, dev_drv->type, dbg_str);

	/* get init_cmd */
	propdata = (char *)fdt_getprop(dt_addr, child_offset, "cmd_size", NULL);
	if (!propdata) {
		LDIMPR("no cmd_size\n");
	} else {
		temp = be32_to_cpup((u32 *)propdata);
		dev_drv->cmd_size = (unsigned char)temp;
	}
	if (lcd_debug_print_flag & LCD_DBG_PR_BL_NORMAL)
		LDIMPR("cmd_size=%d\n", dev_drv->cmd_size);
	if (dev_drv->cmd_size != LCD_EXT_CMD_SIZE_DYNAMIC)
		goto ldim_dev_get_config_from_dts_end;

	ret = ldim_dev_init_table_handle_dts(dt_addr, child_offset, dev_drv);

ldim_dev_get_config_from_dts_end:
	return ret;
}

#if defined(CONFIG_AML_LCD_JSON) || defined(CONFIG_CMD_AML_MODEL)
static struct num_str_s ldim_dev_type_match[] = {
	{LDIM_DEV_TYPE_NORMAL, "NORMAL"},
	{LDIM_DEV_TYPE_SPI,    "SPI"},
	{LDIM_DEV_TYPE_I2C,    "I2C"},
	{LDIM_DEV_TYPE_MAX,    "MAX"}
};
#endif

/* config from json =============================================================================*/

#ifdef CONFIG_AML_LCD_JSON
static int ldim_gpio_name_to_index(struct ldim_dev_driver_s *drv, char *name)
{
	int i = 0;

	if (!drv || !name)
		return LCD_GPIO_MAX;

	for (i = 0; i < BL_GPIO_NUM_MAX; i++)
		if (!strcmp(drv->gpio_name[i], name))
			return i;
	return LCD_GPIO_MAX;
}

int ldim_dev_get_config_from_json(struct ldim_dev_driver_s *dev_drv)
{
	struct json_parse_s *jsp = get_panel_jsp(0);
	struct json_s *parent, *child, *child2, *child3;
	int i = 0, cnt, cnt_max, nums_size, data_cnt;
	const char *str = NULL;
	struct ldim_spi_dev_info_s *spi_info;
	struct bl_pwm_config_s *bl_pwm, *pwms[3] = {NULL, NULL, NULL};
	unsigned int *nums = NULL, temp;
	unsigned char *table;
	int ret = 0;

	if (!json_parse_ok(jsp)) {
		LDIMERR("panel0 jsp not ok\n");
		return -1;
	}

	parent = json_path_to_node(jsp, jsp->root, "backlight/ldim_dev");
	if (!parent) {
		LDIMERR("failed find /backlight/ldim_dev\n");
		return -1;
	}

//basic_info
	child = json_get_object_child(jsp, parent, "basic_info");
	if (!child) {
		LDIMERR("fail to get basic_info\n");
		return -1;
	}

	str = json_get_obj_str(jsp, child, "name", NULL);
	strncpy(dev_drv->name, str, str ? LDIM_DEV_NAME_MAX - 1 : 0);
	dev_drv->index    = 0;
	dev_drv->chip_cnt = json_get_obj_u32(jsp, child, "chip_count", 1);
	dev_drv->dim_min  = json_get_obj_u32(jsp, child, "dim_min", 0);
	dev_drv->dim_max  = json_get_obj_u32(jsp, child, "dim_max", 4095);
	temp  = json_get_obj_u32(jsp, child, "dim_default", 0xffffff);
	if (temp != 0xffffff)
		dev_drv->dim_dft = temp;
	dev_drv->uboot_dim_en  = json_get_obj_u32(jsp, child, "uboot_dim_en", 1);

//interface
	child = json_get_object_child(jsp, parent, "interface");
	if (!child) {
		LDIMERR("fail to get interface\n");
		return -1;
	}

	str = json_get_obj_str(jsp, child, "type", NULL);
	dev_drv->type = strnum_get_num(str, ldim_dev_type_match,
				       ARRAY_SIZE(ldim_dev_type_match), LDIM_DEV_TYPE_MAX);
	if (dev_drv->type == LDIM_DEV_TYPE_MAX) {
		LDIMERR("invalid type:%d\n", dev_drv->type);
		return -1;
	}

	switch (dev_drv->type) {
	case LDIM_DEV_TYPE_SPI:
		spi_info = &dev_drv->spi_info;
		spi_info->spi_dev_num = json_get_obj_u32(jsp, child, "spi_dev_num", 1);
		child2 = json_get_object_child(jsp, child, "bus_number");
		if (child2) {
			for (i = 0; i < spi_info->spi_dev_num; i++)
				spi_info->bus_num[i] = json_get_arr_u32(jsp, child2, i, 0);
		}
		child2 = json_get_object_child(jsp, child, "chip_select");
		if (child2) {
			for (i = 0; i < spi_info->spi_dev_num; i++)
				spi_info->chip_select[i] = json_get_arr_u32(jsp, child2, i, 0);
		}

		spi_info->max_speed_hz = json_get_obj_u32(jsp, child, "max_freq", 3000000);
		spi_info->mode = json_get_obj_u32(jsp, child, "spi_mode", 0);
		dev_drv->cs2clk_delay = json_get_obj_u32(jsp, child, "cs2clk_delay_ms", 0);
		dev_drv->clk2cs_delay = json_get_obj_u32(jsp, child, "clk2cs_delay_ms", 0);

		if (lcd_debug_print_flag & LCD_DBG_PR_BL_NORMAL) {
			LDIMPR("spi_dev_num:%d, spi bus: %d:%d:%d, cs:%d:%d:%d\n"
			       "max_freq:%d, mode:%d, cs2clk_delay:%dms, clk2cs_delay:%dms\n",
			       spi_info->spi_dev_num, spi_info->bus_num[0], spi_info->bus_num[1],
				   spi_info->chip_select[0],  spi_info->chip_select[1],
				   spi_info->max_speed_hz,
			       spi_info->mode, dev_drv->cs2clk_delay, dev_drv->clk2cs_delay);
		}
		break;
	default:
		break;
	}

//pwms
	child = json_get_object_child(jsp, parent, "pwms");
	if (child) {
		cnt = json_get_array_size(jsp, child);
		cnt = lcd_s32_constraint(cnt, 0, 2);
		pwms[0] = &dev_drv->ldim_pwm_config;
		pwms[1] = &dev_drv->analog_pwm_config;
		for (i = 0; i < cnt; i++) {
			child2 = json_get_array_child(jsp, child, i);
			if (!child2) {
				BLPR("fail find pwm[%d]\n", i);
				break;
			}

			bl_pwm = pwms[i];
			bl_pwm->drv_index = 0;
			str = json_get_obj_str(jsp, child2, "port", NULL);
			bl_pwm->pwm_port = bl_pwm_str_to_num(str ? str : "Invalid");
			if (bl_pwm->pwm_port >= BL_PWM_MAX ||
			    (i == 1 && bl_pwm->pwm_port >= BL_PWM_VS))
				continue;

			bl_pwm->pwm_method = json_get_obj_u32(jsp, child2, "polarity", 1);
			bl_pwm->pwm_phase  = json_get_obj_u32(jsp, child2, "phase", 0);
			bl_pwm->pwm_freq   = json_get_obj_u32(jsp, child2, "freq", 300);
			if (bl_pwm->pwm_freq > XTAL_HALF_FREQ_HZ)
				bl_pwm->pwm_freq = XTAL_HALF_FREQ_HZ;

			child3 = json_get_object_child(jsp, child2, "duty_range");
			if (child3) {
				bl_pwm->pwm_duty_min = json_get_arr_u32(jsp, child3, 0, 0);
				bl_pwm->pwm_duty_max = json_get_arr_u32(jsp, child3, 1, 4095);
			}
			bl_pwm->pwm_duty_dft = json_get_obj_u32(jsp, child2, "duty",
								bl_pwm->pwm_duty_min);
			bl_pwm->pwm_duty = bl_pwm->pwm_duty_dft;
			bl_pwm->pwm_duty_save = bl_pwm->pwm_duty_dft;
			bl_pwm_config_init(bl_pwm);

			LDIMPR("get pwm[%d] pol=%d, freq=%d, phase=%d, duty=%d [%d~%d]\n",
			       i, bl_pwm->pwm_method, bl_pwm->pwm_freq, bl_pwm->pwm_phase,
			       bl_pwm->pwm_duty, bl_pwm->pwm_duty_min, bl_pwm->pwm_duty_max);
		}
	}

//ctrl
	child = json_get_object_child(jsp, parent, "ctrl");
	if (child) {
		str = json_get_obj_str(jsp, child, "pinmux_name", NULL);
		strncpy(dev_drv->pinmux_name, str ? str : "invalid", (LDIM_DEV_NAME_MAX - 1));

		str = json_get_obj_str(jsp, child, "err_gpio", NULL);
		dev_drv->lamp_err_gpio = ldim_gpio_name_to_index(dev_drv, (char *)str);
		str = json_get_obj_str(jsp, child, "en_gpio", NULL);
		dev_drv->en_gpio = ldim_gpio_name_to_index(dev_drv, (char *)str);
		dev_drv->en_gpio_on = json_get_obj_u32(jsp, child, "en_gpio_on", 1);
		dev_drv->en_gpio_off = json_get_obj_u32(jsp, child, "en_gpio_off", 0);

		if (dev_drv->lamp_err_gpio < BL_GPIO_NUM_MAX)
			dev_drv->fault_check = 1;

		dev_drv->hw_on_delay = json_get_obj_u32(jsp, child, "hw_on_delay_ms", 500);
		dev_drv->hw_off_delay = json_get_obj_u32(jsp, child, "hw_off_delay_ms", 0);
		dev_drv->write_check = json_get_obj_u32(jsp, child, "write_check", 0);
	}

//packet_info
	child = json_get_object_child(jsp, parent, "packet_info");
	if (child) {
		dev_drv->header_cnt = json_get_obj_u32(jsp, child, "headercnt", 0x0);
		child2 = json_get_object_child(jsp, child, "headerdata");
		if (child2) {
			dev_drv->header_data = malloc(dev_drv->header_cnt);
			if (dev_drv->header_data) {
				for (i = 0; i < dev_drv->header_cnt; i++)
					dev_drv->header_data[i] =
						(unsigned char)json_get_arr_u32(jsp, child2, i, 0);
			} else {
				LDIMERR("malloc header_data failed!\n");
			}
		} else {
			LDIMERR("get headerdata failed!\n");
		}

		dev_drv->extend_cnt = json_get_obj_u32(jsp, child, "extendcnt", 0x0);
		child2 = json_get_object_child(jsp, child, "extenddata");
		if (child2) {
			dev_drv->extend_data = malloc(dev_drv->extend_cnt);
			if (dev_drv->extend_data) {
				for (i = 0; i < dev_drv->extend_cnt; i++)
					dev_drv->extend_data[i] =
						(unsigned char)json_get_arr_u32(jsp, child2, i, 0);
			} else {
				LDIMERR("malloc extenddata failed!\n");
			}
		} else {
			LDIMERR("get extenddata failed!\n");
		}

		dev_drv->adim_width = json_get_obj_u32(jsp, child, "adim_width", 0x0);
		dev_drv->adim = json_get_obj_u32(jsp, child, "adim_data", 0x0);
		dev_drv->pdim_width = json_get_obj_u32(jsp, child, "pdim_width", 0x0);
		dev_drv->pdim = json_get_obj_u32(jsp, child, "pdim_data", 0x0);
		dev_drv->datamapping_en = json_get_obj_u32(jsp, child, "datamapping_en", 0x1);
	}

//boost

//profile & zone map
	str = json_get_obj_str(jsp, parent, "zone_mapping_path", NULL);
	if (str) {
		LDIMPR("find custom ldim_zone_mapping_path:%s\n", str);
		ret = ldim_dev_zone_mapping_load(dev_drv, str);
		if (ret) {
			for (i = 0; i < dev_drv->zone_num; i++)
				dev_drv->bl_mapping[i] = (unsigned short)i;
		}
	} else {
		for (i = 0; i < dev_drv->zone_num; i++)
			dev_drv->bl_mapping[i] = (unsigned short)i;
	}

//custom_params
	child = json_get_object_child(jsp, parent, "custom_params");
	if (child) {
		cnt = json_get_array_size(jsp, child);
		cnt = lcd_s32_constraint(cnt, 0, 256);
		LDIMPR("get custom_params cnt = %d\n", cnt);
		if (cnt) {
			dev_drv->custom_params = malloc(cnt * sizeof(unsigned int));
			if (!dev_drv->custom_params) {
				LDIMERR("no memory for custom_params\n");
				goto ldim_dev_get_config_from_json_end;
			}
			memset(dev_drv->custom_params, 0, cnt * sizeof(unsigned int));
			for (i = 0; i < cnt; i++)
				dev_drv->custom_params[i] = json_get_arr_u32(jsp, child, i, 0);
			dev_drv->custom_params_cnt = cnt;
		}
	}

//commands
	child = json_get_object_child(jsp, parent, "commands");
	if (child) {
		dev_drv->cmd_size = LCD_EXT_CMD_SIZE_DYNAMIC;

		str = json_get_obj_str(jsp, child, "init_on", NULL);
		if (str) {
			cnt_max = lcd_get_str_array_cnt(str);
			nums_size = cnt_max * sizeof(unsigned int);
			nums = malloc(nums_size);
			if (!nums) {
				LDIMERR("init_on: no memory to save nums\n");
				goto parse_ldim_init_off;
			}

			memset(nums, 0, nums_size);
			cnt = lcd_trans_str_array(str, nums, cnt_max);
			table = lcd_init_table_load_array("ldim_dev_init_on",
							  LCD_EXT_CMD_SIZE_DYNAMIC,
							  nums, cnt_max, LDIM_INIT_ON_MAX,
							  &data_cnt);
			if (!table) {
				free(nums);
				goto ldim_dev_get_config_from_json_end;
			}
			dev_drv->init_on = table;
			dev_drv->init_on_cnt = data_cnt;
			memset(nums, 0, nums_size);
			free(nums);
		}
parse_ldim_init_off:
		str = json_get_obj_str(jsp, child, "init_off", NULL);
		if (str) {
			cnt_max = lcd_get_str_array_cnt(str);
			nums_size = cnt_max * sizeof(unsigned int);
			nums = malloc(nums_size);
			if (!nums) {
				LDIMERR("init_off: no memory to save nums\n");
				goto ldim_dev_get_config_from_json_end;
			}

			memset(nums, 0, nums_size);
			cnt = lcd_trans_str_array(str, nums, cnt_max);
			table = lcd_init_table_load_array("ldim_dev_init_off",
							  LCD_EXT_CMD_SIZE_DYNAMIC,
							  nums, cnt_max, LDIM_INIT_OFF_MAX,
							  &data_cnt);
			if (!table) {
				free(nums);
				goto ldim_dev_get_config_from_json_end;
			}
			dev_drv->init_off = table;
			dev_drv->init_off_cnt = data_cnt;
			free(nums);
		}
		dev_drv->init_loaded = 1;
	}

ldim_dev_get_config_from_json_end:
	return 0;
}

#else
int ldim_dev_get_config_from_json(struct ldim_dev_driver_s *dev_drv)
{
	return -1;
}
#endif

#if defined(CONFIG_CMD_AML_MODEL)
static inline int ldim_dev_type_str2num(const char *str)
{
	const char *start;

	start = strchr(str, 'V');
	if (start)
		start += 2;
	else
		start = str;

	return strnum_get_num(start, ldim_dev_type_match, ARRAY_SIZE(ldim_dev_type_match),
			      LDIM_DEV_TYPE_MAX);
}

static int ldim_dev_pwm_port_str2num(const char *str)
{
	char *start;

	start = strchr(str, 'P');
	if (!start)
		return BL_PWM_MAX;

	return bl_pwm_str_to_num(start);
}

int ldim_dev_get_config_from_ini(struct ldim_dev_driver_s *dev_drv)
{
	void *inip, *psec;
	const char *str = NULL;
	struct bl_pwm_config_s *bl_pwm;
	unsigned int val, *init_buf;
	unsigned char *table;
	int init_len, data_cnt, buf_size;
	char dbg_str[160];
	int i, dbg_str_len = 0, ret = 0;
	struct ldim_spi_dev_info_s *spi_info;
	unsigned int temp;

	inip = get_lcd_ini_parse_mem(0);
	if (!inip)
		return -1;

	psec = lcd_ini_get_section(inip, "Ldim_dev_Attr");
	if (!psec) {
		LDIMERR("%s: not find Ldim_dev_Attr\n", __func__);
		return -1;
	}

	dev_drv->index = 0;
	str = lcd_ini_get_str(inip, psec, "dev_name", "null");
	strlcpy(dev_drv->name, str, LDIM_DEV_NAME_MAX);

	str = lcd_ini_get_str(inip, psec, "if_type", "null");
	dev_drv->type = ldim_dev_type_str2num(str);
	if (dev_drv->type == LDIM_DEV_TYPE_MAX)
		return -1;

	switch (dev_drv->type) {
	case LDIM_DEV_TYPE_SPI:
		spi_info = &dev_drv->spi_info;
		spi_info->max_speed_hz = lcd_ini_get_val(inip, psec, "if_freq", 0);
		val = lcd_ini_get_val(inip, psec, "if_attr_0", 0);
		spi_info->bus_num[0] = val & 0xf;
		spi_info->bus_num[1] = (val >> 4) & 0xf;
		spi_info->bus_num[2] = (val >> 8) & 0xf;
		i = (val >> 12) & 0xf;
		if (i)
			spi_info->spi_dev_num = i;

		val = lcd_ini_get_val(inip, psec, "if_attr_1", 0);
		spi_info->chip_select[0] = val & 0xf;
		spi_info->chip_select[1] = (val >> 4) & 0xf;
		spi_info->chip_select[2] = (val >> 8) & 0xf;

		spi_info->mode = lcd_ini_get_val(inip, psec, "if_attr_2", 0);
		dev_drv->cs2clk_delay = lcd_ini_get_val(inip, psec, "if_attr_4", 0);
		dev_drv->clk2cs_delay = lcd_ini_get_val(inip, psec, "if_attr_5", 0);

		if (lcd_debug_print_flag & LCD_DBG_PR_BL_NORMAL) {
			LDIMPR("spi bus_num: %d:%d:%d, chip_select:%d:%d:%d, max_freq:%d\n"
			       "spi_dev_num:%d, spi mode:%d, cs2clk_delay=%d, clk2cs_delay=%d\n",
			       spi_info->bus_num[0], spi_info->bus_num[1],
			       spi_info->bus_num[2], spi_info->chip_select[0],
			       spi_info->chip_select[1], spi_info->chip_select[2],
			       spi_info->max_speed_hz, spi_info->spi_dev_num,
			       spi_info->mode, dev_drv->cs2clk_delay, dev_drv->clk2cs_delay);
		}
		break;
	default:
		break;
	}

	/* pwm (48Byte) */
	bl_pwm = &dev_drv->ldim_pwm_config;
	bl_pwm->drv_index = 0; /* only venc0 support ldim */
	str = lcd_ini_get_str(inip, psec, "pwm_vs_port", "null");
	bl_pwm->pwm_port = ldim_dev_pwm_port_str2num(str);
	if (bl_pwm->pwm_port < BL_PWM_MAX) {
		str = lcd_ini_get_str(inip, psec, "pwm_vs_pol", "null");
		bl_pwm->pwm_method = bl_str_to_pwm_method(str, BL_PWM_POSITIVE);
		if (bl_pwm->pwm_port == BL_PWM_VS) {
			val = lcd_ini_get_val(inip, psec, "pwm_vs_freq", 0);
			bl_pwm->pwm_freq = (val & 0xff);
			bl_pwm->pwm_phase = (val >> 8) & 0xffffff;

		} else {
			bl_pwm->pwm_freq = lcd_ini_get_val(inip, psec, "pwm_vs_freq", 0);
			bl_pwm->pwm_phase = 0;
		}
		bl_pwm->pwm_duty_dft = lcd_ini_get_val(inip, psec, "pwm_vs_duty", 0);
		bl_pwm->pwm_duty = bl_pwm->pwm_duty_dft;
		bl_pwm->pwm_duty_save = bl_pwm->pwm_duty_dft;

		if (bl_pwm->pwm_port == BL_PWM_VS) {
			if (bl_pwm->pwm_freq > 4) {
				LDIMERR("pwm_vs wrong freq %d\n", bl_pwm->pwm_freq);
				bl_pwm->pwm_freq = BL_FREQ_VS_DEFAULT;
			}
		} else {
			if (bl_pwm->pwm_freq > XTAL_HALF_FREQ_HZ)
				bl_pwm->pwm_freq = XTAL_HALF_FREQ_HZ;
		}
		bl_pwm_config_init(bl_pwm);
		LDIMPR("get ldim_pwm pol=%d, freq=%d, duty=%d, phase=%d\n",
		       bl_pwm->pwm_method, bl_pwm->pwm_freq, bl_pwm->pwm_duty, bl_pwm->pwm_phase);
	}

	bl_pwm = &dev_drv->analog_pwm_config;
	bl_pwm->drv_index = 0; /* only venc0 support ldim */
	str = lcd_ini_get_str(inip, psec, "pwm_adj_port", "null");
	bl_pwm->pwm_port = ldim_dev_pwm_port_str2num(str);
	if (bl_pwm->pwm_port < BL_PWM_VS) {
		str = lcd_ini_get_str(inip, psec, "pwm_adj_pol", "BL_PWM_POSITIVE");
		bl_pwm->pwm_method = bl_str_to_pwm_method(str, BL_PWM_POSITIVE);
		if (bl_pwm->pwm_port == BL_PWM_VS) {
			val = lcd_ini_get_val(inip, psec, "pwm_adj_freq", 0);
			bl_pwm->pwm_freq = (val & 0xff);
			bl_pwm->pwm_phase = (val >> 8) & 0xffffff;
		} else {
			bl_pwm->pwm_freq = lcd_ini_get_val(inip, psec, "pwm_adj_freq", 0);
			bl_pwm->pwm_phase = 0;
		}
		bl_pwm->pwm_duty_max = lcd_ini_get_val(inip, psec, "pwm_adj_attr_0", 4095);
		bl_pwm->pwm_duty_min = lcd_ini_get_val(inip, psec, "pwm_adj_attr_1", 0);
		bl_pwm->pwm_duty_dft = lcd_ini_get_val(inip, psec, "pwm_adj_duty", 0);
		bl_pwm->pwm_duty = bl_pwm->pwm_duty_dft;
		bl_pwm->pwm_duty_save = bl_pwm->pwm_duty_dft;

		if (bl_pwm->pwm_freq > XTAL_HALF_FREQ_HZ)
			bl_pwm->pwm_freq = XTAL_HALF_FREQ_HZ;
		bl_pwm_config_init(bl_pwm);
		LDIMPR("get analog_pwm pol=%d, freq=%d, duty=%d [%d~%d], phase=%d\n",
		       bl_pwm->pwm_method, bl_pwm->pwm_freq, bl_pwm->pwm_duty,
		       bl_pwm->pwm_duty_min, bl_pwm->pwm_duty_max, bl_pwm->pwm_phase);
	}

	str = lcd_ini_get_str(inip, psec, "pinmux_sel", "invalid");
	strlcpy(dev_drv->pinmux_name, str, LDIM_DEV_NAME_MAX);

	/* ctrl (271Byte) */
	dev_drv->en_gpio = lcd_ini_get_val(inip, psec, "en_gpio", LCD_GPIO_MAX);
	dev_drv->en_gpio_on = lcd_ini_get_val(inip, psec, "en_gpio_on", 0);
	dev_drv->en_gpio_off = lcd_ini_get_val(inip, psec, "en_gpio_off", 0);

	dev_drv->lamp_err_gpio = lcd_ini_get_val(inip, psec, "err_gpio", LCD_GPIO_MAX);
	if (dev_drv->lamp_err_gpio >= BL_GPIO_NUM_MAX) {
		dev_drv->fault_check = 0;
	} else {
		dev_drv->fault_check = 1;
		ldim_gpio_set(dev_drv, dev_drv->lamp_err_gpio, LCD_GPIO_INPUT);
	}

	dev_drv->hw_on_delay = lcd_ini_get_val(inip, psec, "hw_on_delay", 500);
	dev_drv->hw_off_delay = lcd_ini_get_val(inip, psec, "hw_off_delay", 0);
	dev_drv->write_check = lcd_ini_get_val(inip, psec, "write_check", 0);

	dev_drv->dim_max = lcd_ini_get_val(inip, psec, "dim_max", 0);
	dev_drv->dim_min = lcd_ini_get_val(inip, psec, "dim_min", 0);
	temp  = lcd_ini_get_val(inip, psec, "dim_default", 0xffffff);
	if (temp != 0xffffff)
		dev_drv->dim_dft = temp;
	dev_drv->uboot_dim_en  = lcd_ini_get_val(inip, psec, "uboot_dim_en", 1);

	//dev_drv->mcu_header = lcd_ini_get_val(inip, psec, "custome_attr_0", 0);
	//dev_drv->mcu_dim = lcd_ini_get_val(inip, psec, "custome_attr_1", 0);

	temp = lcd_ini_get_val(inip, psec, "custome_attr_0", 0);
	dev_drv->header_cnt = temp & 0xff;
	dev_drv->extend_cnt = (temp >> 8) & 0xff;
	dev_drv->adim_width = (temp >> 16) & 0x1f;
	dev_drv->pdim_width = (temp >> 21) & 0x1f;
	dev_drv->datamapping_en = (temp >> 26) & 0x1;
	dev_drv->pkt_type = (temp >> 27) & 0xf;

	temp =	lcd_ini_get_val(inip, psec, "custome_attr_1", 0);
	dev_drv->adim = temp & 0xffff;
	dev_drv->pdim = (temp >> 16) & 0xffff;
	//ini fix max header_cnt is 8
	dev_drv->header_data = malloc(8);
	if (dev_drv->header_data) {
		temp =	lcd_ini_get_val(inip, psec, "custome_attr_6", 0);
		dev_drv->header_data[0] =  temp & 0xff;
		dev_drv->header_data[1] = (temp >> 8) & 0xff;
		dev_drv->header_data[2] = (temp >> 16) & 0xff;
		dev_drv->header_data[3] = (temp >> 24) & 0xff;
		temp =	lcd_ini_get_val(inip, psec, "custome_attr_7", 0);
		dev_drv->header_data[4] =  temp & 0xff;
		dev_drv->header_data[5] = (temp >> 8) & 0xff;
		dev_drv->header_data[6] = (temp >> 16) & 0xff;
		dev_drv->header_data[7] = (temp >> 24) & 0xff;
	} else {
		LDIMERR("kcalloc header_data failed!\n");
	}

	dev_drv->extend_data = malloc(8);
	if (dev_drv->extend_data) {
		temp =	lcd_ini_get_val(inip, psec, "custome_attr_8", 0);
		dev_drv->extend_data[0] =  temp & 0xff;
		dev_drv->extend_data[1] = (temp >> 8) & 0xff;
		dev_drv->extend_data[2] = (temp >> 16) & 0xff;
		dev_drv->extend_data[3] = (temp >> 24) & 0xff;
		temp =	lcd_ini_get_val(inip, psec, "custome_attr_9", 0);
		dev_drv->extend_data[4] =  temp & 0xff;
		dev_drv->extend_data[5] = (temp >> 8) & 0xff;
		dev_drv->extend_data[6] = (temp >> 16) & 0xff;
		dev_drv->extend_data[7] = (temp >> 24) & 0xff;
	} else {
		LDIMERR("kcalloc extend_data failed!\n");
	}

	LDIMPR("header_cnt:%d, extend_cnt:%d, adim_width:%d,pdim_width:%d\n"
	       "datamapping_en:%d, pkt_type:%d, uboot_dim_en:%d,adim:%d, pdim:%d\n",
		   dev_drv->header_cnt, dev_drv->extend_cnt,
		   dev_drv->adim_width, dev_drv->pdim_width,
		   dev_drv->datamapping_en, dev_drv->pkt_type,
		   dev_drv->uboot_dim_en, dev_drv->adim, dev_drv->pdim);

	LDIMPR("header_data:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x,\n"
	       "extend_data:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x,\n",
		   dev_drv->header_data[0], dev_drv->header_data[1],
		   dev_drv->header_data[2], dev_drv->header_data[3],
		   dev_drv->header_data[4], dev_drv->header_data[5],
		   dev_drv->header_data[6], dev_drv->header_data[7],
		   dev_drv->extend_data[0], dev_drv->extend_data[1],
		   dev_drv->extend_data[2], dev_drv->extend_data[3],
		   dev_drv->extend_data[4], dev_drv->extend_data[5],
		   dev_drv->extend_data[6], dev_drv->extend_data[7]);

	dev_drv->chip_cnt = lcd_ini_get_val(inip, psec, "chip_count", 0);

	str = lcd_ini_get_str(inip, psec, "zone_mapping_path", NULL);
	if (!str) {
		for (i = 0; i < dev_drv->zone_num; i++)
			dev_drv->bl_mapping[i] = (unsigned short)i;
	} else {
		LDIMPR("find custom zone_mapping: %s\n", str);
		ret = ldim_dev_zone_mapping_load(dev_drv, str);
		if (ret) {
			for (i = 0; i < dev_drv->zone_num; i++)
				dev_drv->bl_mapping[i] = (unsigned short)i;
		}
	}

	dbg_str_len += sprintf(dbg_str + dbg_str_len, "mcu_header=0x%08x, mcu_dim=0x%08x, ",
		dev_drv->mcu_header, dev_drv->mcu_dim);
	sprintf(dbg_str + dbg_str_len, "chip_cnt:%d, cus pwm_pinmux_sel:%s",
		dev_drv->chip_cnt, dev_drv->pinmux_name);
	LDIMPR("load ukey config: %s: type:%d, %s\n", dev_drv->name, dev_drv->type, dbg_str);

	dev_drv->cmd_size = lcd_ini_get_val(inip, psec, "cmd_size", 0);
	if (lcd_debug_print_flag & LCD_DBG_PR_BL_NORMAL)
		LDIMPR("%s: cmd_size = %d\n", dev_drv->name, dev_drv->cmd_size);
	if (dev_drv->cmd_size != LCD_EXT_CMD_SIZE_DYNAMIC)
		return -1;

	init_len = lcd_ini_get_array_cnt(inip, psec, "init_on");
	if (init_len < 0) {
		LDIMPR("%s: not find init_on\n", dev_drv->name);
		return 0;
	}
	buf_size = init_len * sizeof(unsigned int);
	init_buf = malloc(buf_size);
	if (!init_buf) {
		LDIMERR("malloc init_on buf failed\n");
		return -1;
	}
	memset(init_buf, 0, buf_size);
	data_cnt = lcd_ini_get_array(inip, psec, "init_on", init_buf, init_len);
	table = lcd_init_table_load_array("ldim_dev_init_on", LCD_EXT_CMD_SIZE_DYNAMIC,
					  init_buf, data_cnt, LDIM_INIT_ON_MAX, &init_len);
	if (!table)
		goto ldim_dev_get_config_from_ini_err;
	dev_drv->init_on = table;
	dev_drv->init_on_cnt = init_len;
	memset(init_buf, 0, buf_size);
	free(init_buf);

	init_len = lcd_ini_get_array_cnt(inip, psec, "init_off");
	if (init_len < 0) {
		LDIMERR("%s: not find init_on\n", dev_drv->name);
		return -1;
	}
	buf_size = init_len * sizeof(unsigned int);
	init_buf = malloc(buf_size);
	if (!init_buf) {
		LDIMERR("malloc init_off buf failed\n");
		return -1;
	}
	memset(init_buf, 0, buf_size);
	data_cnt = lcd_ini_get_array(inip, psec, "init_off", init_buf, init_len);
	table = lcd_init_table_load_array("ldim_dev_init_off", LCD_EXT_CMD_SIZE_DYNAMIC,
					  init_buf, data_cnt, LDIM_INIT_OFF_MAX, &init_len);
	if (!table)
		goto ldim_dev_get_config_from_ini_err;
	dev_drv->init_off = table;
	dev_drv->init_off_cnt = init_len;
	memset(init_buf, 0, buf_size);
	free(init_buf);

	dev_drv->init_loaded = 1;

	return 0;

ldim_dev_get_config_from_ini_err:
	memset(init_buf, 0, buf_size);
	free(init_buf);
	return -1;
}

#else
int ldim_dev_get_config_from_ini(struct ldim_dev_driver_s *dev_drv)
{
	return -1;
}
#endif

static int ldim_dt_valid(char *dt_addr)
{
#ifdef CONFIG_OF_LIBFDT

	int parent_offset;
	char *propdata;

	parent_offset = fdt_path_offset(dt_addr, "/local_dimming_device");
	if (parent_offset < 0) {
		parent_offset = fdt_path_offset(dt_addr, "/local_diming_device");
		if (parent_offset < 0) {
			LDIMERR("not find /local_dimming_device node: %s\n",
				fdt_strerror(parent_offset));
			return -1;
		}
	}

	propdata = (char *)fdt_getprop(dt_addr, parent_offset, "status", NULL);
	if (propdata && strncmp(propdata, "okay", 2) == 0)
		return 1;

	LDIMPR("local_dimming_device status disabled\n");
	return 0;
#else
	return 0;
#endif
}

static int ldim_check_config_load(struct ldim_dev_driver_s *dev_drv)
{
	int ret = 0, dt_sta;

	dt_sta = ldim_dt_valid(lcd_get_dt_addr());
	dev_drv->config_load = lcd_panel_config_load_detect(0, dt_sta, __func__);
	if (dev_drv->config_load == LCD_CONFIG_NONE || dev_drv->config_load == LCD_CONFIG_ERR)
		return -1;

	return ret;
}

int ldim_dev_get_config(char *dt_addr, struct aml_ldim_driver_s *ldim_drv)
{
	struct ldim_dev_driver_s *dev_drv = ldim_drv->dev_drv;
	int parent_offset;
	char *propdata;
	char *p;
	const char *str;
	int i, j, ret = 0;
	unsigned char file_type = PANEL_FILE_INVILD;

	if (!dt_addr) {
		LDIMERR("%s: dt_addr is NULL\n", __func__);
		return -1;
	}
	if (!dev_drv) {
		LDIMERR("%s: dev_drv is NULL\n", __func__);
		return -1;
	}

	parent_offset = fdt_path_offset(dt_addr, "/local_dimming_device");
	if (parent_offset < 0) {
		parent_offset = fdt_path_offset(dt_addr, "/local_diming_device");
		if (parent_offset < 0) {
			LDIMERR("not find /local_dimming_device node: %s\n",
				fdt_strerror(parent_offset));
			return -1;
		}
	}
	propdata = (char *)fdt_getprop(dt_addr, parent_offset, "status", NULL);
	if (!propdata) {
		LDIMERR("not find local_dimming_device status, default to disabled\n");
		return -1;
	}
	if (strncmp(propdata, "okay", 2)) {
		LDIMPR("local_dimming_device status disabled\n");
		return -1;
	}

	/* init gpio */
	i = 0;
	propdata = (char *)fdt_getprop(dt_addr, parent_offset, "ldim_dev_gpio_names", NULL);
	if (!propdata) {
		LDIMERR("failed to get ldim_dev_gpio_names\n");
	} else {
		p = propdata;
		while (i < BL_GPIO_NUM_MAX) {
			if (i > 0)
				p += strlen(p) + 1;
			str = p;
			if (strlen(str) == 0)
				break;
			strlcpy(dev_drv->gpio_name[i], str, LCD_CPU_GPIO_NAME_MAX);
			if (lcd_debug_print_flag & LCD_DBG_PR_BL_NORMAL)
				LDIMPR("i=%d, gpio=%s\n", i, dev_drv->gpio_name[i]);
			i++;
		}
	}
	for (j = i; j < BL_GPIO_NUM_MAX; j++)
		strcpy(dev_drv->gpio_name[j], "invalid");

	ret = ldim_check_config_load(dev_drv);
	if (ret)
		return -1;

	switch (dev_drv->config_load) {
	case LCD_CONFIG_FILE:
		file_type = get_lcd_panel_file_type(0);
		if (file_type == PANEL_FILE_JSON)
			ret = ldim_dev_get_config_from_json(dev_drv);
		else if (file_type == PANEL_FILE_INI)
			ret = ldim_dev_get_config_from_ini(dev_drv);
		break;
	case LCD_CONFIG_DTS:
	case LCD_CONFIG_BSP:
		ret = ldim_dev_get_config_from_dts(dev_drv, dt_addr, dev_drv->index);
		break;

	default:
		ret = -1;
		break;
	}


	return ret;
}
#endif
