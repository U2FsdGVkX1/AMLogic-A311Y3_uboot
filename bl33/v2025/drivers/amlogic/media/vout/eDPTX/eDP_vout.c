// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

// #include <common.h>
#include <malloc.h>
#include <amlogic/cpu_id.h>
#include <fdtdec.h>
#ifdef CONFIG_SECURE_POWER_CONTROL
#include <asm/amlogic/arch/pwr_ctrl.h>
#endif
#include <string.h>
#include <amlogic/pm.h>
#include <amlogic/media/vout/eDPTX/eDPTX.h>
#include <amlogic/media/vout/eDPTX/eDPTX_export.h>
#include "eDP_common.h"
#include "eDP_dummy_reg.h"
#include <env.h>
#include <dm.h>

uint8_t dptx_print_level;
static unsigned int dptx_debug_test_flag;

struct dptx_drv_s *dptx_drivers[eDPTX_MAX_DRV];

// static char *g_dt_addr = (char *)gd->fdt_blob;

#if defined(CONFIG_MESON_T7) || defined(CONFIG_MESON_T7C)
char *dptx_pm_name[eDPTX_MAX_DRV] = {"eDPTX-T7-0-pm", "eDPTX-T7-1-pm"};

static struct edptx_chip_data_s edptx_data_t7 = {
	.chip_type = eDPTX_CHIP_T7,
	.chip_name = "T7",
	.drv_max = 2,
	.offset_venc      = {0x0, (0x600 << 2)},
	.offset_venc_if   = {0x0, (0x500 << 2)},
	.offset_venc_data = {0x0, (0x100 << 2)},
	.venc_clk_msr_id  = {222, 221},

	.link_rate    = {{DP_LINK_RATE_HBR, DP_LINK_RATE_HBR}, {DP_LINK_RATE_HBR, 0}},
	.lane_count   = {{4, 4}, {4, 0}},
	.TPS_support  = BIT(0),
	.pixel_clk_limit = 667,
};

#elif defined(CONFIG_MESON_A9)
char *dptx_pm_name[eDPTX_MAX_DRV] = {"eDPTX-A9-0-pm"};

static const struct edptx_chip_data_s edptx_data_a9 = {
	.chip_type = eDPTX_CHIP_A9,
	.chip_name = "A9",
	.offset_venc      = {0x0},
	.offset_venc_if   = {0x0},
	.offset_venc_data = {0x0},
	.venc_clk_msr_id  = {53},

	.link_rate    = {{DP_LINK_RATE_HBR2}},
	.lane_count   = {{4}},
	.TPS_support  = BIT(0) | BIT(1),
	.pixel_clk_limit = 667,
};
#endif

static int edptx_chip_data_add(struct dptx_drv_s *dptx)
{
#if defined(CONFIG_MESON_T7) || defined(CONFIG_MESON_T7C)
	dptx->data = (struct edptx_chip_data_s *)&edptx_data_t7;
#elif defined(CONFIG_MESON_A9)
	dptx->data = (struct edptx_chip_data_s *)&edptx_data_a9;
#else
	dptx->data = NULL;
	DPTX_ERR(dptx, "chip or index invalid");
	return -1;
#endif

	DPTX_PR(dptx, "Chip: %u-%s, %u.%uGHz",
		dptx->data->chip_type, dptx->data->chip_name,
		(dptx->data->link_rate[0][0] * 27) / 100,
		(dptx->data->link_rate[0][0] * 27) % 100);
	return 0;
}

static int dptx_board_data_add(char *dt_addr, struct dptx_drv_s *dptx)
{
	int parent_offset;
	char *propdata, snode[20];
	uint8_t i;

	sprintf(snode, "/eDPTX%u", dptx->idx);
	parent_offset = fdt_path_offset(dt_addr, snode);
	if (parent_offset < 0) {
		DPTX_ERR(dptx, "load %s node: %s", snode, fdt_strerror(parent_offset));
		return -1;
	}

	DPTX_DBG(dptx, "load %s node", snode);

	/* check lcd status enable or not */
	propdata = (char *)fdt_getprop(dt_addr, parent_offset, "status", NULL);
	if (!propdata) {
		DPTX_ERR(dptx, "failed to get status, default disable");
		return -1;
	}
	if (strcmp(propdata, "okay")) {
		DPTX_PR(dptx, "status disabled, exit");
		return -1;
	}

	propdata = (char *)fdt_getprop(dt_addr, parent_offset, "device_name", NULL);
	if (propdata && (strlen(propdata) != 0)) {
		strncpy(dptx->setting.dev_name, propdata, 31);
		DPTX_PR(dptx, "device name: %s", dptx->setting.dev_name);
	} else {
		strcpy(dptx->setting.dev_name, "Amlogic eDPTX");
	}

	propdata = (char *)fdt_getprop(dt_addr, parent_offset, "edptx_vcc-gpio-name", NULL);
	if (propdata && (strlen(propdata) != 0)) {
		strncpy(dptx->board_data.edptx_vcc_gpio_name, propdata, (eDPTX_GPIO_NAME_MAX - 1));
		DPTX_PR(dptx, "vcc GPIO: %s", dptx->board_data.edptx_vcc_gpio_name);
	} else {
		strcpy(dptx->board_data.edptx_vcc_gpio_name, "invalid");
	}

	propdata = (char *)fdt_getprop(dt_addr, parent_offset, "edptx_bl_en-gpio-name", NULL);
	if (propdata && (strlen(propdata) != 0)) {
		strncpy(dptx->board_data.edptx_bl_en_gpio_name, propdata, (eDPTX_GPIO_NAME_MAX - 1));
		DPTX_PR(dptx, "bl en GPIO: %s", dptx->board_data.edptx_bl_en_gpio_name);
	} else {
		strcpy(dptx->board_data.edptx_bl_en_gpio_name, "invalid");
	}
	propdata = (char *)fdt_getprop(dt_addr, parent_offset, "edptx_bl_pwm-gpio-name", NULL);
	if (propdata && (strlen(propdata) != 0)) {
		strncpy(dptx->board_data.edptx_bl_pwm_gpio_name, propdata, (eDPTX_GPIO_NAME_MAX - 1));
		DPTX_PR(dptx, "bl pwm GPIO: %s", dptx->board_data.edptx_bl_pwm_gpio_name);
	} else {
		strcpy(dptx->board_data.edptx_bl_pwm_gpio_name, "invalid");
	}
	propdata = (char *)fdt_getprop(dt_addr, parent_offset, "drive-strength-lut", NULL);
	if (propdata) {
		for (i = 0; i < 30; i++) {
			dptx->board_data.driver_strength_lut[i / 10][i % 3] =
				(u16)be32_to_cpup(((u32 *)propdata) + i);
		}
	} else {
		DPTX_ERR(dptx, "DPCD level to PHY LUT not set");
	}
	return 0;
}

// static int dptx_panel_data_add(struct dptx_drv_s *dptx, struct display_config_s *config_p)
// {
// 	if (!config_p)
// 		return -1;

// 	if (config_p->sig.eDP.port_count) {
// 		DPTX_PR(dptx, "assign to %u port", config_p->sig.eDP.port_count);
// 		if (config_p->sig.eDP.port_count == 2)
// 			dptx->sink.port_mask = 0x3;
// 		else if (config_p->sig.eDP.port_count == 4)
// 			dptx->sink.port_mask = 0xf;
// 		else
// 			dptx->sink.port_mask = 0x1;
// 	} else {
// 		dptx->sink.port_mask = 0x1;
// 	}

// 	if (!config_p->sig.eDP.link_from_DPCD) {
// 		DPTX_PR(dptx, "assign to %u lane %u.%uG", config_p->sig.eDP.lane_count,
// 			config_p->sig.eDP.link_rate * 27 / 100,
// 			config_p->sig.eDP.link_rate * 27 % 100);
// 	} else {
// 		config_p->sig.eDP.link_rate = 0;
// 		config_p->sig.eDP.lane_count = 0;
// 	}

// 	if (config_p->sig.eDP.hpd_ignore)
// 		DPTX_PR(dptx, "assign to wait hpd %ums", config_p->sig.eDP.hpd_ignore);

// 	if (config_p->sig.eDP.training_mode)
// 		DPTX_PR(dptx, "assign training mode %u", config_p->sig.eDP.training_mode);

// 	dptx->sink.config = config_p;

// 	return 0;
// }


static struct dptx_drv_s *edptx_driver_check_valid(uint8_t index)
{
	if (index >= eDPTX_MAX_DRV)
		return NULL;

	if (!dptx_drivers[index] || !(dptx_drivers[index]->status & DPTX_STA_PROBE_DONE)) {
		DPTXPR(0, LOG_E, "invalid DPTX%d config", index);
		return NULL;
	}
	return dptx_drivers[index];
}

static char *get_current_env_connector(unsigned char cnt_idx)
{
	char cnt_name[20];

	sprintf(cnt_name, "connector%hu_type", cnt_idx);

	return env_get(cnt_name);
}

int dptx_connector_check(struct dptx_drv_s *dptx)
{
	char dptx_cnt_name[8];
	uint8_t i;
	char *cnt_type_name;

	sprintf(dptx_cnt_name, "EDP-%c", 'A' + dptx->idx);
	for (i = 0; i < 3; i++) {
		cnt_type_name = get_current_env_connector(i);
		if (!cnt_type_name)
			continue;

		if (strcmp(dptx_cnt_name, cnt_type_name))
			continue;

		dptx->viu_sel = i + 1;
		DPTX_PR(dptx, "%s: connector[%hu] - (%s)\n", __func__, i, cnt_type_name);
		return 0;
	}

	DPTX_ERR(dptx, "%s: no connector to eDPTX: %s", __func__, dptx_cnt_name);
	return -1;
}

int dptx_outputmode_check(struct dptx_drv_s *dptx, char *mode)
{
	char vmd_name[24];
	uint8_t i;

	for (i = 0; i < DPTX_DRV_VMODE_MAX; i++) {
		if (!(dptx->vmode_mgr.vmodes[i].flag & VMODE_FLAG_VALID))
			continue;
		memset(vmd_name, 0, 24 * sizeof(char));
		__str_add_vmode(dptx, vmd_name, i, 1);
		// printf("%s\n", vmd_name);
		if (strcmp(mode, vmd_name)) {
			dptx->vmode_mgr.vmode_sel_idx = i;
			DPTX_PR(dptx, "%s: timing: %u, %s", __func__, i, mode);
			return 0;
		}
	}

	dptx->vmode_mgr.vmode_sel_idx = U8_MAX;
	DPTX_PR(dptx, "%s: timing: %s (not match)", __func__, mode);
	return 0;
}

static void dptx_update_outputmode(struct dptx_drv_s *dptx)
{
	char cnt_type_str[16] = "connectorX_type";
	char mode_env_name[16] = "outputmode\0\0";
	char temp_str[20] = "EDP-X";
	uint8_t i;

	if (!(dptx->status & DPTX_STA_LINK_ON)) {
		DPTX_ERR(dptx, "curr link not on");
		dptx->vmode_mgr.vmode_sel_idx = U8_MAX;
	}

	temp_str[4] = 'A' + dptx->idx;

	for (i = 0; i < eDPTX_MAX_VOUT; i++) {
		cnt_type_str[9] = '0' + i;
		if (env_get(cnt_type_str) && strcmp(temp_str, env_get(cnt_type_str)) == 0) {
			dptx->viu_sel = 1 + i;
			if (i)
				mode_env_name[11] = '1' + i;
			break;
		}
	}
	if (!dptx->viu_sel)
		return;

	memset(temp_str, 0, 20 * sizeof(char));
	__str_add_vmode(dptx, temp_str, dptx->vmode_mgr.vmode_sel_idx, 1);

	env_set(mode_env_name, temp_str);

	DPTX_PR(dptx, "%s=%s: set %s=%s",
		cnt_type_str, (char *)env_get(cnt_type_str), mode_env_name, temp_str);
}

/***** DISPLAYPORT DRIVER PREPARE / ENABLE / REMOVE ********/
static uint8_t dptx_module_prepare(struct dptx_drv_s *dptx, char *mode)
{
	if ((dptx->status & DPTX_STA_PROBE_DONE) == 0)
		return 0;

	if ((dptx->status & DPTX_STA_DRV_READY) == 0)
		edptx_driver_ready(dptx);

	return 0; // RGB
	/*
	dptx_drv_check_HPD(dptx);

	if (dptx->status & DPTX_STA_HPD_HIGH)
		dptx_drv_start(dptx);

	dptx_update_outputmode(dptx);

	if (dptx->status & DPTX_STA_LINK_ON) {
		switch (dptx->vmode_mgr.vmode_cfmt_sel) {
		case DPTX_CFMT_YCbCr422_8bit:
		case DPTX_CFMT_YCbCr422_10bit:
		case DPTX_CFMT_YCbCr422_12bit:
		case DPTX_CFMT_YCbCr444_8bit:
		case DPTX_CFMT_YCbCr444_10bit:
		case DPTX_CFMT_YCbCr444_12bit:
		case DPTX_CFMT_YCbCr420_8bit:
		case DPTX_CFMT_YCbCr420_10bit:
		case DPTX_CFMT_YCbCr420_12bit:
		case DPTX_CFMT_Y_only_8bit:
		case DPTX_CFMT_Y_only_10bit:
		case DPTX_CFMT_Y_only_12bit:
			return 2; // YUV
		case DPTX_CFMT_RGB_6bit:
		case DPTX_CFMT_RGB_8bit:
		case DPTX_CFMT_RGB_10bit:
		case DPTX_CFMT_RGB_12bit:
		default:
			return 0; // RGB
		}
	}
	return 0;
	*/
}

static uint8_t dptx_module_enable(struct dptx_drv_s *dptx, char *mode)
{
	if ((dptx->status & DPTX_STA_PROBE_DONE) == 0)
		return 0;

	if ((dptx->status & DPTX_STA_DRV_READY) == 0)
		edptx_driver_ready(dptx);

	dptx_drv_check_HPD(dptx);

	if (dptx->status & DPTX_STA_HPD_HIGH)
		dptx_drv_start(dptx);

	dptx_update_outputmode(dptx);

	if (dptx->status & DPTX_STA_LINK_ON) {
		dptx_drv_disp_on(dptx);

		switch (dptx->vmode_mgr.vmode_cfmt_sel) {
		case DPTX_CFMT_YCbCr422_8bit:
		case DPTX_CFMT_YCbCr422_10bit:
		case DPTX_CFMT_YCbCr422_12bit:
		case DPTX_CFMT_YCbCr444_8bit:
		case DPTX_CFMT_YCbCr444_10bit:
		case DPTX_CFMT_YCbCr444_12bit:
		case DPTX_CFMT_YCbCr420_8bit:
		case DPTX_CFMT_YCbCr420_10bit:
		case DPTX_CFMT_YCbCr420_12bit:
		case DPTX_CFMT_Y_only_8bit:
		case DPTX_CFMT_Y_only_10bit:
		case DPTX_CFMT_Y_only_12bit:
			return 2; // YUV
		case DPTX_CFMT_RGB_6bit:
		case DPTX_CFMT_RGB_8bit:
		case DPTX_CFMT_RGB_10bit:
		case DPTX_CFMT_RGB_12bit:
		default:
			return 0; // RGB
		}
	}
	return 0;
}

static void dptx_module_disable(struct dptx_drv_s *dptx)
{
	if (dptx->status & DPTX_STA_DISP_ON)
		dptx_drv_disp_off(dptx);

	if (dptx->status & DPTX_STA_LINK_ON)
		edptx_driver_close(dptx);
	//lcd_update_outputmode(pdrv);
}

/***** DISPLAYPORT DRIVER PREPARE / ENABLE / REMOVE END  ********/

/***** DISPLAYPORT DRIVER PROBE & REMOVE ********/
static struct dptx_drv_s *edptx_driver_add(uint8_t index)
{
	struct dptx_drv_s *dptx;
	int init_once = 0;

	if (index >= eDPTX_MAX_DRV) {
		DPTXPR(0, LOG_E, "%s: invalid index: %d\n", __func__, index);
		return NULL;
	}

	if (!dptx_drivers[index]) {
		dptx_drivers[index] = (struct dptx_drv_s *)malloc(sizeof(struct dptx_drv_s));
		if (!dptx_drivers[index]) {
			DPTXPR(index, LOG_E, "%s: Not enough memory", __func__);
			return NULL;
		}
		init_once = 1;
	}

	dptx = dptx_drivers[index];
	memset(dptx, 0, sizeof(struct dptx_drv_s));
	dptx->idx = index;

	if (init_once) {
		//dptx->power_on_suspend = 1;
		//dptx->dev_pm_ops = dev_register_pm(lcd_pm_name[index],
		//	 &dptx_drivers_suspend, &dptx_drivers_resume, &edptx_driver_poweroff);
	}

	/* default config */
	return dptx;
}

static uint8_t dptx_panel_config_load_timing(char *dt_addr,
				int parent_offset, struct dptx_drv_s *dptx)
{
	char snode[24];
	unsigned char i, j;
	unsigned int tmp;
	int lenp;
	char *propdata;

	for (i = 0; i < DPTX_DRV_TIMING_MAX; i++) {
		memset(snode, 0, 24 * sizeof(char));
		sprintf(snode, "timing-%u", i);
		propdata = (char *)fdt_getprop(dt_addr, parent_offset, snode, NULL);
		if (!propdata)
			break;

		dptx->panel_data.timing[i].h_period = (u16)be32_to_cpup(((u32 *)propdata) + 0);
		dptx->panel_data.timing[i].h_act    = (u16)be32_to_cpup(((u32 *)propdata) + 1);
		dptx->panel_data.timing[i].h_pw     = (u16)be32_to_cpup(((u32 *)propdata) + 2);
		dptx->panel_data.timing[i].h_bp     = (u16)be32_to_cpup(((u32 *)propdata) + 3);
		dptx->panel_data.timing[i].ctrl    |= (u16)be32_to_cpup(((u32 *)propdata) + 4) ?
							CTRL_HSYNC_POS : 0;
		dptx->panel_data.timing[i].v_period = (u16)be32_to_cpup(((u32 *)propdata) + 5);
		dptx->panel_data.timing[i].v_act    = (u16)be32_to_cpup(((u32 *)propdata) + 6);
		dptx->panel_data.timing[i].v_pw     = (u16)be32_to_cpup(((u32 *)propdata) + 7);
		dptx->panel_data.timing[i].v_bp     = (u16)be32_to_cpup(((u32 *)propdata) + 8);
		dptx->panel_data.timing[i].ctrl    |= (u16)be32_to_cpup(((u32 *)propdata) + 9) ?
							CTRL_VSYNC_POS : 0;
		dptx->panel_data.timing[i].h_blank  =
			dptx->panel_data.timing[i].h_period - dptx->panel_data.timing[i].h_act;
		dptx->panel_data.timing[i].v_blank  =
			dptx->panel_data.timing[i].v_period - dptx->panel_data.timing[i].v_act;
		dptx->panel_data.timing[i].h_fp = dptx->panel_data.timing[i].h_blank -
			(dptx->panel_data.timing[i].h_pw + dptx->panel_data.timing[i].h_bp);
		dptx->panel_data.timing[i].v_fp = dptx->panel_data.timing[i].v_blank -
			(dptx->panel_data.timing[i].v_pw + dptx->panel_data.timing[i].v_bp);

		tmp = (u16)be32_to_cpup(((u32 *)propdata) + 10);
		if (tmp == 30)
			dptx->panel_data.timing[i].cfmt = DPTX_CFMT_RGB_10bit;
		else if (tmp == 18)
			dptx->panel_data.timing[i].cfmt = DPTX_CFMT_RGB_6bit;
		else if (tmp == 12)
			dptx->panel_data.timing[i].cfmt = DPTX_CFMT_Y_only_12bit;
		else if (tmp == 10)
			dptx->panel_data.timing[i].cfmt = DPTX_CFMT_Y_only_10bit;
		else if (tmp == 8)
			dptx->panel_data.timing[i].cfmt = DPTX_CFMT_Y_only_8bit;
		else
			dptx->panel_data.timing[i].cfmt = DPTX_CFMT_RGB_8bit;

		tmp = (u16)be32_to_cpup(((u32 *)propdata) + 11);
		dptx->panel_data.timing[i].fr1000 =  tmp * 1000L;

		tmp = dptx->panel_data.timing[i].v_period * dptx->panel_data.timing[i].h_period * tmp;
		dptx->panel_data.timing[i].pclk = tmp;

		DPTX_PR(dptx, "Panel Config add Timing[%u]: %ux%u@%uhz", i,
			dptx->panel_data.timing[i].h_act, dptx->panel_data.timing[i].v_act,
			dptx->panel_data.timing[i].fr1000 / 1000);
		dptx->panel_data.timing_cnt++;

		memset(snode, 0, 24 * sizeof(char));
		sprintf(snode, "timing-%u-fr", i);
		propdata = (char *)fdt_getprop(dt_addr, parent_offset, snode, &lenp);
		if (propdata) {
			// DPTX_PR(dptx, "lenp=%d", lenp);
			for (j = 0; j < (lenp / sizeof(u32)); j++) {
				if (dptx->panel_data.timing[i].vmode_add_fr[j] == 0) {
					dptx->panel_data.timing[i].vmode_add_fr[j] =
						(u16)be32_to_cpup(((u32 *)propdata) + j);
					DPTX_DBG(dptx, "Panel Config Timing[%u] add fr[%u]=%u",
						i, j, dptx->panel_data.timing[i].vmode_add_fr[j]);
					break;
				}
			}
		}
		memset(snode, 0, 24 * sizeof(char));
		sprintf(snode, "timing-%u-range", i);
		propdata = (char *)fdt_getprop(dt_addr, parent_offset, snode, NULL);
		if (propdata) {
			dptx->panel_data.timing[i].frame_rate_range[0] =
				(u16)be32_to_cpup(((u32 *)propdata) + 0);
			dptx->panel_data.timing[i].frame_rate_range[1] =
				(u16)be32_to_cpup(((u32 *)propdata) + 1);
			DPTX_DBG(dptx, "Panel Config Timing[%u] range=[%u~%u]", i,
				dptx->panel_data.timing[i].frame_rate_range[0],
				dptx->panel_data.timing[i].frame_rate_range[1]);
		}

		memset(snode, 0, 24 * sizeof(char));
		sprintf(snode, "timing-%u-range-limit", i);
		propdata = (char *)fdt_getprop(dt_addr, parent_offset, snode, &lenp);
		if (propdata) {
			// DPTX_PR(dptx, "lenp=%d", lenp);
			dptx->panel_data.timing[i].v_period_range[0] =
				(u16)be32_to_cpup(((u32 *)propdata) + 2);
			dptx->panel_data.timing[i].v_period_range[1] =
				(u16)be32_to_cpup(((u32 *)propdata) + 3);
			dptx->panel_data.timing[i].pclk_range[0] =
				(u16)be32_to_cpup(((u32 *)propdata) + 4);
			dptx->panel_data.timing[i].pclk_range[1] =
				(u16)be32_to_cpup(((u32 *)propdata) + 5);
			DPTX_DBG(dptx, "Panel Config Timing[%u] limit vl=[%u~%u] pclk=[%u~%u]", i,
				dptx->panel_data.timing[i].v_period_range[0],
				dptx->panel_data.timing[i].v_period_range[1],
				dptx->panel_data.timing[i].pclk_range[0],
				dptx->panel_data.timing[i].pclk_range[1]);
		}
	}
	return i;
}

static uint8_t dptx_panel_data_add(char *dt_addr, struct dptx_drv_s *dptx)
{
#ifdef CONFIG_OF_LIBFDT
	int parent_offset;
	char *propdata;
	uint8_t i;
	char *panel_type, str[15],  propname[30];

	if (dptx->idx == 0)
		sprintf(str, "panel_type");
	else
		sprintf(str, "panel%d_type", dptx->idx);
	panel_type = env_get(str);
	if (!panel_type) {
		DPTX_PR(dptx, "no %s exist", str);
		return 1;
	}

	snprintf(propname, 30, "/Panel_Groups/%s", panel_type);
	parent_offset = fdt_path_offset(dt_addr, propname);
	if (parent_offset < 0) {
		DPTX_PR(dptx, "%s: not find %s node: %s",
			__func__, propname, fdt_strerror(parent_offset));
		return 1;
	}

	propdata = (char *)fdt_getprop(dt_addr, parent_offset, "HPD-ignore", NULL);
	if (propdata) {
		dptx->setting.user_hpd_ignore = (u16)be32_to_cpup((u32 *)propdata);
		if (dptx->setting.user_hpd_ignore)
			DPTX_PR(dptx, "HPD ignore: %ums", dptx->setting.user_hpd_ignore);
	}

	propdata = (char *)fdt_getprop(dt_addr, parent_offset, "assigned-link-rate", NULL);
	if (propdata) {
		dptx->setting.user_link_rate = (u16)be32_to_cpup((u32 *)propdata);
		if (dptx->setting.user_link_rate)
			DPTX_PR(dptx, "assigned link rate: %u", dptx->setting.user_link_rate);
	} else {
		dptx->setting.user_link_rate = 0;
	}

	propdata = (char *)fdt_getprop(dt_addr, parent_offset, "assigned-lane-count", NULL);
	if (propdata) {
		dptx->setting.user_lane_count = (u16)be32_to_cpup((u32 *)propdata);
		if (dptx->setting.user_lane_count)
			DPTX_PR(dptx, "assigned lane count: %u", dptx->setting.user_lane_count);
	} else {
		dptx->setting.user_lane_count = 0;
	}

	propdata = (char *)fdt_getprop(dt_addr, parent_offset, "assigned-port-count", NULL);
	if (propdata) {
		i = (u16)be32_to_cpup((u32 *)propdata);
		if (i == 4)
			dptx->sink.port_mask = 0xf;
		else if (i == 2)
			dptx->sink.port_mask = 0x3;
		else
			dptx->sink.port_mask = 0x1;
		if (i)
			DPTX_PR(dptx, "assigned port mask: %u", dptx->setting.user_port_mask);
	} else {
		dptx->sink.port_mask = 0x1;
	}

	propdata = (char *)fdt_getprop(dt_addr, parent_offset, "assigned-vmode", NULL);
	if (propdata) {
		dptx->setting.user_vmode_sel = (u16)be32_to_cpup((u32 *)propdata);
		if (dptx->setting.user_vmode_sel)
			DPTX_PR(dptx, "assigned vmode idx: %u", dptx->setting.user_vmode_sel);
	} else {
		dptx->setting.user_vmode_sel = 0;
	}

	propdata = (char *)fdt_getprop(dt_addr, parent_offset, "assigned-disable-PSR", NULL);
	if (propdata) {
		dptx->setting.user_disable_PSR = (u16)be32_to_cpup((u32 *)propdata);
		if (dptx->setting.user_disable_PSR)
			DPTX_PR(dptx, "assigned disable PSR: %u", dptx->setting.user_disable_PSR);
	} else {
		dptx->setting.user_disable_PSR = 0;
	}

	propdata = (char *)fdt_getprop(dt_addr, parent_offset, "assigned-preset-timing", NULL);
	if (propdata) {
		dptx->setting.user_disable_PSR = (u16)be32_to_cpup((u32 *)propdata);
		if (dptx->setting.user_disable_PSR)
			DPTX_PR(dptx, "assigned preset timing: %u", dptx->setting.user_disable_PSR);
	} else {
		dptx->setting.user_disable_PSR = 0;
	}

	propdata = (char *)fdt_getprop(dt_addr, parent_offset, "assigned-preset-timing", NULL);
	if (propdata) {
		dptx->setting.user_disable_PSR = (u16)be32_to_cpup((u32 *)propdata);
		if (dptx->setting.user_disable_PSR)
			DPTX_PR(dptx, "assigned preset timing: %u", dptx->setting.user_disable_PSR);
	} else {
		dptx->setting.user_disable_PSR = 0;
	}
	dptx_panel_config_load_timing(dt_addr, parent_offset, dptx);

	return 0;
#endif
	return 1;
}

static uint8_t dptx_config_load_from_dts(struct dptx_drv_s *dptx)
{
#ifdef CONFIG_OF_LIBFDT
	int ret;

	char *dt_addr = (char *)gd->fdt_blob;


	ret = dptx_board_data_add(dt_addr, dptx);
	if (ret) {
		DPTX_PR(dptx, "not loading board config");
		return 0;
	}

	ret = dptx_panel_data_add(dt_addr, dptx);
	if (ret) {
		DPTX_PR(dptx, "not loading panel config");
		dptx->setting.user_link_rate = 0;
		dptx->setting.user_lane_count = 0;
		dptx->sink.port_mask = 0x1;
		dptx->setting.user_vmode_sel = 0;
		dptx->setting.user_disable_PSR = 0;
		dptx->setting.user_color_format = 0xff;
	}
#endif
	return 0;
}

static void dptx_user_env_load(struct dptx_drv_s *dptx)
{
	char env_str[20];
	uint32_t temp;
	char *_str;

	sprintf(env_str, "edp%u_link_rate", dptx->idx);
	temp = env_get_ulong(env_str, 10, 0);
	if (temp) {
		dptx->setting.user_link_rate = temp;
		DPTX_PR(dptx, "user env: %s: %u", env_str, temp);
	}

	sprintf(env_str, "edp%u_lane_count", dptx->idx);
	temp = env_get_ulong(env_str, 10, 0);
	if (temp) {
		dptx->setting.user_lane_count = temp;
		DPTX_PR(dptx, "user env %s: %u", env_str, temp);
	}

	sprintf(env_str, "edp%u_hpd_ignore", dptx->idx);
	temp = env_get_ulong(env_str, 10, 0);
	if (temp) {
		dptx->setting.user_hpd_ignore = temp;
		DPTX_PR(dptx, "user env: %s: %u", env_str, temp);
	}

	sprintf(env_str, "edp%u_vmode", dptx->idx);
	temp = env_get_ulong(env_str, 10, 0);
	if (temp) {
		dptx->setting.user_vmode_sel = temp;
		DPTX_PR(dptx, "user env: %s: %u", env_str, temp);
	}

	sprintf(env_str, "edp%u_port_count", dptx->idx);
	temp = env_get_ulong(env_str, 10, 0);
	if (temp) {
		if (temp == 4)
			dptx->setting.user_port_mask = 0xf;
		else if (temp == 2)
			dptx->setting.user_port_mask = 0x3;
		else
			dptx->setting.user_port_mask = 0x1;
		DPTX_PR(dptx, "user env: %s: %u", env_str, temp);
		dptx->sink.port_mask = dptx->setting.user_port_mask;
	} else {
		dptx->setting.user_port_mask = 0x1;
	}

	sprintf(env_str, "edp%u_psr_disable", dptx->idx);
	temp = env_get_ulong(env_str, 10, 0);
	if (temp) {
		dptx->setting.user_disable_PSR = 1;
		DPTX_PR(dptx, "user env: %s: %u", env_str, temp);
	}

	sprintf(env_str, "edp%u_color_format", dptx->idx);
	_str = env_get(env_str);
	if (_str) {
		for (temp = 0;; temp++) {
			if (dptx_cfmt_table[temp].cfmt_id == DPTX_CFMT_invalid)
				break;
			if (strcmp(_str, dptx_cfmt_table[temp].name) == 0) {
				dptx->setting.user_color_format = temp;
				DPTX_PR(dptx, "user env: %s: %s", env_str, _str);
				break;
			}
		}
	}
}

static int edptx_driver_remove(uint8_t index)
{
	if (index >= dptx_drivers[index]->data->drv_max)
		return 0;

	if (!dptx_drivers[index])
		return 0;

	if (dptx_drivers[index]->dev_pm_ops)
		dev_unregister_pm(dptx_drivers[index]->dev_pm_ops);

	free(dptx_drivers[index]);
	dptx_drivers[index] = NULL;

	return 0;
}

static void dptx_power_domain_off(struct dptx_drv_s *dptx)
{
#ifdef CONFIG_SECURE_POWER_CONTROL
	switch (dptx->data->chip_type) {
#if defined(CONFIG_MESON_T7) || defined(CONFIG_MESON_T7C)
	case eDPTX_CHIP_T7:
		pwr_ctrl_psci_smc(PM_EDP0, 0);
		pwr_ctrl_psci_smc(PM_EDP1, 0);
		break;
#endif
#if defined(CONFIG_MESON_A9)
	case eDPTX_CHIP_A9:
		// pwr_ctrl_psci_smc(PM_EDP0, 0);
		break;
#endif
	default:
		return;
	}
	if (dptx_print_level >= LOG_I)
		DPTX_PR(dptx, "power domain off");
#endif
}

static void __dptx_update_ctrl_bootargs(void)
{
	uint32_t val2 = 0, i, temp;
	char env_str[24];
	char ctrl_str[64];

	val2 |= dptx_print_level & 0x3;

	for (i = 0; i < eDPTX_MAX_DRV; i++) {
		sprintf(env_str, "dptx%u_hpd_ignore", i);
		val2 |= env_get_ulong(env_str, 10, 0) ? (1 << (28 + i)) : 0;
	}

	for (i = 0; i < eDPTX_MAX_DRV; i++) {
		sprintf(env_str, "dptx%u_port_count", i);
		if (env_get_ulong(env_str, 10, 0) == 4)
			temp = 3;
		else if (env_get_ulong(env_str, 10, 0) == 2)
			temp = 2;
		else if (env_get_ulong(env_str, 10, 0) == 1)
			temp = 1;
		else
			temp = 0;
		val2 |= temp << (20 + 2 * i);
	}

	sprintf(ctrl_str, "0x%x,%s,%s", val2, (char *)env_get("panel_type"), (char *)env_get("panel1_type"));
	env_set("eDP_ctrl", ctrl_str);
}

int dptx_probe(void)
{
	uint8_t i;
	int ret;
	struct dptx_drv_s *dptx;

	dptx_print_level = env_get_ulong("edptx_debug", 16, 1);
	DPTXPR(0, LOG_I, "DPTX print level: %u", dptx_print_level);

	dptx_debug_test_flag = env_get_ulong("edptx_test", 10, 0);

	for (i = 0; i < eDPTX_MAX_DRV; i++) {
		dptx = edptx_driver_add(i);
		if (!dptx)
			continue;

		edptx_chip_data_add(dptx);
		edptx_phy_probe(dptx);
		edptx_venc_probe(dptx);
		edptx_clk_config_probe(dptx);
		edptx_if_IP_probe(dptx);
		ret = dptx_config_load_from_dts(dptx);
		if (ret)
			edptx_driver_remove(i);

		dptx_user_env_load(dptx);

		dptx->status = DPTX_STA_PROBE_DONE;

		dptx_power_domain_off(dptx);
	}

	__dptx_update_ctrl_bootargs();
	return 0;
}

int dptx_remove(void)
{
	uint8_t i;

	// if (!dptx_dev_data)
	// 	return 0;

	for (i = 0; i < eDPTX_MAX_DRV; i++)
		edptx_driver_remove(i);

	return 0;
}

void edptx_driver_probe(void)
{
	printf("11111\n");
	dptx_probe();
}

uint8_t edptx_driver_prepare(uint8_t index, char *mode)
{
	struct dptx_drv_s *dptx;
	uint8_t port;

	if (!mode) {
		DPTXPR(index, LOG_E, "%s: mode is NULL", __func__);
		return 0;
	}

	dptx = edptx_driver_check_valid(index);
	if (!dptx)
		return 0;

	for (port = 0; port < eDPTX_MAX_PORT; port++) {
		if (dptx->sink.port_mask & BIT(port)) {
			memset(&dptx->sink.link[port], 0, sizeof(struct dptx_link_cfg_s));
		} else {
			if (!(dptx->sink.port_mask & BIT(port)))
				continue;
			// free(dptx->sink.link[port]);
			// dptx->sink.link[port] = NULL;
			DPTX_PR(dptx, "clean unused link[%d]", port);
		}
	}

	dptx->sink.hpd_mask   = 0x1;
	if (dptx->status & DPTX_STA_DRV_READY) {
		DPTX_PR(dptx, "already prepared");
		return 0;
	}

	return dptx_module_prepare(dptx, mode);
}

uint8_t edptx_driver_enable(uint8_t index, char *mode)
{
	struct dptx_drv_s *dptx;
	uint8_t port;

	if (!mode) {
		DPTXPR(index, LOG_E, "%s: mode is NULL", __func__);
		return 0;
	}

	dptx = edptx_driver_check_valid(index);
	if (!dptx)
		return 0;

	for (port = 0; port < eDPTX_MAX_PORT; port++) {
		if (dptx->sink.port_mask & BIT(port)) {
			// dptx->sink.link[port] = malloc(sizeof(struct dptx_link_cfg_s));
			// if (!(dptx->sink.port_mask & BIT(port))) {
			// 	DPTX_ERR(dptx, "link[%d] malloc failed", port);
			// 	continue;
			// }
			memset(&dptx->sink.link[port], 0, sizeof(struct dptx_link_cfg_s));
		} else {
			if (!(dptx->sink.port_mask & BIT(port)))
				continue;
			// free(dptx->sink.link[port]);
			// dptx->sink.link[port] = NULL;
			DPTX_PR(dptx, "clean unused link[%d]", port);
		}
	}

	if (dptx->status & DPTX_STA_DISP_ON) {
		DPTX_PR(dptx, "already enabled");
		return 0;
	}

	return dptx_module_enable(dptx, mode);
}

void edptx_driver_disable(uint8_t index)
{
	struct dptx_drv_s *dptx;
	uint8_t port;

	dptx = edptx_driver_check_valid(index);
	if (!dptx)
		return;

	if ((dptx->status & DPTX_STA_LINK_ON) == 0) {
		DPTX_PR(dptx, "already disabled");
		return;
	}

	dptx_module_disable(dptx);

	for (port = 0; port < eDPTX_MAX_PORT; port++) {
		// if (dptx->sink.link[port])
		// 	free(dptx->sink.link[port]);
		// dptx->sink.link[port] = NULL;
	}
}

void edptx_driver_info(uint8_t index)
{
	struct dptx_drv_s *dptx;

	dptx = edptx_driver_check_valid(index);
	if (!dptx)
		return;

	dptx_info_print(dptx);
}

void edptx_driver_reg_print(uint8_t index)
{
	struct dptx_drv_s *dptx;

	dptx = edptx_driver_check_valid(index);
	if (!dptx)
		return;

	dptx_reg_print(dptx);
}

void edptx_driver_test(uint8_t index, uint8_t num)
{
	struct dptx_drv_s *dptx;

	dptx = edptx_driver_check_valid(index);
	if (!dptx)
		return;

	dptx_debug_test(dptx, num);
}

unsigned int edptx_driver_outputmode_check(uint8_t index, char *mode)
{
	struct dptx_drv_s *dptx;

	dptx = edptx_driver_check_valid(index);
	if (!dptx)
		return 0;

	// dptx_connector_mode_check(dptx, mode);

	return 0;
}

void edptx_driver_list_vmode(uint8_t index)
{
	struct dptx_drv_s *dptx;

	dptx = edptx_driver_check_valid(index);
	if (!dptx)
		return;

	dptx_list_support_vmode(dptx);
}

void edptx_driver_set_vmode(uint8_t index, uint8_t num)
{
	struct dptx_drv_s *dptx;

	dptx = edptx_driver_check_valid(index);
	if (!dptx)
		return;

	dptx_user_set_vmode(dptx, num);
}

void edptx_driver_reset(uint8_t index, uint8_t num)
{
	struct dptx_drv_s *dptx;

	dptx = edptx_driver_check_valid(index);
	if (!dptx)
		return;

	dptx_debug_reset(dptx, 0xf, num);
}

void edptx_driver_PSR1_en(uint8_t index, uint8_t num)
{
	struct dptx_drv_s *dptx;

	dptx = edptx_driver_check_valid(index);
	if (!dptx)
		return;

	dptx_drv_eDP_PSR1_en(dptx, 0xf, num);
}

void edptx_driver_PSR2_en(uint8_t index, uint8_t num)
{
	struct dptx_drv_s *dptx;

	dptx = edptx_driver_check_valid(index);
	if (!dptx)
		return;

	dptx_drv_eDP_PSR2_en(dptx, 0xf, num);
}

void edptx_driver_set_pattern(uint8_t index, uint8_t pattern, uint32_t d0, uint32_t d1, uint32_t d2)
{
	struct dptx_drv_s *dptx;

	dptx = edptx_driver_check_valid(index);
	if (!dptx)
		return;

	edptx_set_pattern_to_all_port(dptx, pattern, d0, d1, d2);
}

void edptx_driver_set_link(uint8_t index, uint8_t lane, uint8_t rate)
{
	struct dptx_drv_s *dptx;
	uint8_t port;

	dptx = edptx_driver_check_valid(0);
	if (!dptx)
		return;

	for (port = 0; port < eDPTX_MAX_PORT; port++) {
		if (!(dptx->sink.port_mask & BIT(port)))
			continue;

		dptx->sink.link[port].link_rate  = rate;
		dptx->sink.link[port].lane_count = lane;

		dptx_clk_set_link_clk(dptx, port, rate);
		edptx_set_lane_config(dptx, port);
	}
}
