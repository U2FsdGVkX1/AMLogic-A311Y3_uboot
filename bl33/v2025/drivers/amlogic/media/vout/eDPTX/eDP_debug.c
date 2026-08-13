// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <malloc.h>
#include <amlogic/media/vout/eDPTX/eDPTX.h>
#include <amlogic/media/vout/eDPTX/DPCD_REG.h>
#include "eDP_regs.h"
#include "eDP_common.h"
#include "eDP_debug.h"
#include <amlogic/clk_measure.h>

static void dptx_act_timing_info_print(struct dptx_drv_s *dptx)
{
	printf("Act Timing:\n"
		"  H: period:%4u act:%4u blank:%3u bp:%3u sync:%3u fp:%3u\n"
		"  V: period:%4u act:%4u blank:%3u bp:%3u sync:%3u fp:%3u\n"
		"  PCLK:%uHZ, fr:%u.%3uhz\n",
		dptx->act_timing.h_period,
		dptx->act_timing.h_act,
		dptx->act_timing.h_blank,
		dptx->act_timing.h_bp,
		dptx->act_timing.h_pw,
		dptx->act_timing.h_fp,
		dptx->act_timing.v_period,
		dptx->act_timing.v_act,
		dptx->act_timing.v_blank,
		dptx->act_timing.v_bp,
		dptx->act_timing.v_pw,
		dptx->act_timing.v_fp,
		dptx->act_timing.pclk,
		dptx->act_timing.fr1000 / 1000, dptx->act_timing.fr1000 % 1000);
	printf("\n");
}

static void dptx_link_cfg_info_print(struct dptx_drv_s *dptx)
{
	uint8_t port;

	for (port = 0; port < eDPTX_MAX_PORT; port++) {
		if (!(dptx->sink.port_mask & BIT(port)))
			continue;
		printf("[%u]: max_link: %u-lane %uMHz, act_link: %u-lane %uMHz\n", port,
			dptx->sink.link[port].cap.max_lane_count,
			dptx->sink.link[port].cap.max_link_rate * 270,
			dptx->sink.link[port].lane_count, dptx->sink.link[port].link_rate * 270);

		printf("DACP_support    %u\n"
			"sync_clk_mode   %u\n",
			dptx->sink.link[port].cap.DACP_support,
			dptx->sink.link[port].cap.sync_clk_mode);
	}
}

static void dptx_phy_cfg_info_print(struct dptx_drv_s *dptx)
{
	struct dptx_phy_cfg_s *phy_ctl = &dptx->phy_cfg;
	struct edptx_panel_data_s *p_cfg = &dptx->panel_data;
	const char *lane_name[5] = {" AUX  ", "lane-0", "lane-1", "lane-2", "lane-3"};
	uint8_t i;

	printf("PHY status:\n"
		" global vswing : 0x%x\n", phy_ctl->vswing);

	for (i = 0; i < dptx->panel_data.eDP.port_count * 5; i++) {
		printf(" %c:%s: en:%u sel:%u pn-swap:%u amp:0x%x preem:0x%x, post:0x%x\n",
			'A' + i / 5, lane_name[i % 5],
			p_cfg->ch_ctrl[0].en, p_cfg->ch_ctrl[0].sel, p_cfg->ch_ctrl[0].pn_swap,
			phy_ctl->lane[0].amp, phy_ctl->lane[0].preem, 0);
	}

	printf("\n");
}

static void edptx_clk_info_print(struct dptx_drv_s *dptx)
{
#if defined(CONFIG_MESON_A9)
	uint8_t i;
	uint16_t clk_idx_a9[] = {
		51, 52, 53, 50, 56, 86, 87, 90, 91, 130, 138, 139, 140, 141, 145,
	};

	for (i = 0; i < ARRAY_SIZE(clk_idx_a9); i++)
		clk_msr(clk_idx_a9[i]);
#endif
	printf("\n");
}

void dptx_info_print(struct dptx_drv_s *dptx)
{
	DPTX_PR(dptx, "eDPTX driver version: %s", DPTX_DRV_VERSION);

	dptx_phy_cfg_info_print(dptx);

	edptx_clk_info_print(dptx);

	//dptx_venc_info_print(dptx);

	//dptx_vmode_info_print(dptx);

	dptx_act_timing_info_print(dptx);

	dptx_link_cfg_info_print(dptx);
}

#if defined(CONFIG_MESON_T7) || defined(CONFIG_MESON_T7C)
static struct dptx_debug_info_reg_s dptx_debug_info_reg_t7_0 = {
	.reg_pll        = dptx_regs_pll_t7_0,
	.reg_clk        = dptx_regs_clk_t7_0,
	.reg_combo_dphy = dptx_regs_combo_dphy_t7_0,
	.reg_encl       = dptx_reg_venc_t7,
	.reg_encl_if    = dptx_reg_venc_if_t7,
	.reg_encl_data  = dptx_reg_venc_data_t7,
	.reg_vpu        = dptx_reg_vpu_t7,
	.reg_analog_phy = dptx_regs_analog_phy_t7,
	.reg_dptx_IP    = dptx_reg_dptx_IP,
};

static struct dptx_debug_info_reg_s dptx_debug_info_reg_t7_1 = {
	.reg_pll        = dptx_regs_pll_t7_1,
	.reg_clk        = dptx_regs_clk_t7_1,
	.reg_combo_dphy = dptx_regs_combo_dphy_t7_1,
	.reg_encl       = dptx_reg_venc_t7,
	.reg_encl_if    = dptx_reg_venc_if_t7,
	.reg_encl_data  = dptx_reg_venc_data_t7,
	.reg_vpu        = dptx_reg_vpu_t7,
	.reg_analog_phy = dptx_regs_analog_phy_t7,
	.reg_dptx_IP    = dptx_reg_dptx_IP,
};
#endif

#if defined(CONFIG_MESON_A9)
static struct dptx_debug_info_reg_s dptx_debug_info_reg_a9_0 = {
	.reg_pll        = dptx_regs_pll_a9_0,
	.reg_clk        = dptx_regs_clk_a9_0,
	.reg_combo_dphy = dptx_regs_combo_dphy_a9_0,
	.reg_encl       = dptx_reg_venc_a9,
	.reg_encl_if    = dptx_reg_venc_if_a9,
	.reg_encl_data  = dptx_reg_venc_data_a9,
	.reg_vpu        = dptx_reg_vpu_a9,
	.reg_analog_phy = dptx_regs_analog_phy_a9,
	.reg_dptx_IP    = dptx_reg_dptx_IP_a9,
};
#endif

static void dptx_regs_pr(struct dptx_drv_s *dptx, uint8_t port,
			uint8_t reg_t, struct reg_sets_s *reg_sets)
{
	uint8_t idx, str_pos = 0, reg_temp = 0;
	uint32_t reg_addr = 0, reg_val = 0;

	for (idx = 0; reg_sets[idx].addr != DPTX_REG_END; idx++) {
		if (strlen(reg_sets[idx].name) > str_pos)
			str_pos = strlen(reg_sets[idx].name);
	}
	str_pos++;

	printf("%s regs:\n", dptx_reg_type_name[reg_t]);

	for (idx = 0; reg_sets[idx].addr != DPTX_REG_END; idx++) {
		switch (reg_t) {
		case DPTC_REG_TYPE_VENC:
			reg_addr = reg_sets[idx].addr + dptx->data->offset_venc[dptx->idx];
			reg_val  = dptx_vcbus_read(reg_addr);
			break;
		case DPTC_REG_TYPE_VENC_IF:
			reg_addr = reg_sets[idx].addr + dptx->data->offset_venc_if[dptx->idx];
			reg_val  = dptx_vcbus_read(reg_addr);
			break;
		case DPTC_REG_TYPE_VENC_DATA:
			reg_addr = reg_sets[idx].addr + dptx->data->offset_venc_data[dptx->idx];
			reg_val  = dptx_vcbus_read(reg_addr);
			break;
		case DPTC_REG_TYPE_VPU:
			reg_addr = reg_sets[idx].addr;
			reg_val  = dptx_vcbus_read(reg_addr);
			break;
		case DPTC_REG_TYPE_PLL:
		case DPTC_REG_TYPE_PHY:
			reg_addr = reg_sets[idx].addr;
			reg_val  = dptx_ana_read(reg_addr);
			break;
		case DPTC_REG_TYPE_CLK:
			reg_addr = reg_sets[idx].addr;
			reg_val  = dptx_clk_read(reg_addr);
			break;
		case DPTC_REG_TYPE_DP_IP:
			reg_addr = reg_sets[idx].addr;
			reg_val  = dptx_reg_read(dptx, port, reg_addr);
			break;
		case DPTC_REG_TYPE_COMBO_DPHY:
			reg_addr = reg_sets[idx].addr;
			reg_val  = dptx_combo_dphy_read(reg_addr);
			break;
		case DPTC_REG_TYPE_DPCD_RECEIVER_CAP:
		case DPTC_REG_TYPE_DPCD_LINK_CONFIG:
		case DPTC_REG_TYPE_DPCD_LINK_STATUS:
			if (dptx_if_aux_read(dptx, port, reg_sets[idx].addr, 1, &reg_temp))
				break;
			reg_addr = reg_sets[idx].addr;
			reg_val = reg_temp;
			break;
		default:
			reg_addr = 0;
			reg_val  = 0;
			break;
		}
		printf("%-*s [0x%08x]= 0x%08x\n", str_pos, reg_sets[idx].name, reg_addr, reg_val);
	}
}

void dptx_reg_print(struct dptx_drv_s *dptx)
{
	struct dptx_debug_info_reg_s *debug_info_reg = NULL;
	uint8_t port = 0;

	switch (dptx->data->chip_type) {
#if defined(CONFIG_MESON_T7) || defined(CONFIG_MESON_T7C)
	case eDPTX_CHIP_T7:
		if (dptx->idx == 0)
			debug_info_reg = &dptx_debug_info_reg_t7_0;
		else if (dptx->idx == 1)
			debug_info_reg = &dptx_debug_info_reg_t7_1;
		else
			return;
		break;
#endif
#if defined(CONFIG_MESON_A9)
	case eDPTX_CHIP_A9:
		debug_info_reg = &dptx_debug_info_reg_a9_0;
		break;
#endif
	default:
		return;
	}

	DPTX_PR(dptx, "eDPTX driver regs:");

	if (debug_info_reg->reg_pll)
		dptx_regs_pr(dptx, port, DPTC_REG_TYPE_PLL, debug_info_reg->reg_pll);

	if (debug_info_reg->reg_clk)
		dptx_regs_pr(dptx, port, DPTC_REG_TYPE_CLK, debug_info_reg->reg_clk);

	if (debug_info_reg->reg_combo_dphy)
		dptx_regs_pr(dptx, port, DPTC_REG_TYPE_COMBO_DPHY, debug_info_reg->reg_combo_dphy);

	if (debug_info_reg->reg_encl)
		dptx_regs_pr(dptx, port, DPTC_REG_TYPE_VENC, debug_info_reg->reg_encl);

	if (debug_info_reg->reg_encl_if)
		dptx_regs_pr(dptx, port, DPTC_REG_TYPE_VENC_IF, debug_info_reg->reg_encl_if);

	if (debug_info_reg->reg_encl_data)
		dptx_regs_pr(dptx, port, DPTC_REG_TYPE_VENC_DATA, debug_info_reg->reg_encl_data);

	if (debug_info_reg->reg_vpu)
		dptx_regs_pr(dptx, port, DPTC_REG_TYPE_VPU, debug_info_reg->reg_vpu);

	if (debug_info_reg->reg_analog_phy)
		dptx_regs_pr(dptx, port, DPTC_REG_TYPE_PHY, debug_info_reg->reg_analog_phy);

	if (debug_info_reg->reg_dptx_IP) {
		dptx_regs_pr(dptx, 0, DPTC_REG_TYPE_DP_IP, debug_info_reg->reg_dptx_IP);
		if (dptx->sink.port_mask == 0x3)
			dptx_regs_pr(dptx, 1, DPTC_REG_TYPE_DP_IP, debug_info_reg->reg_dptx_IP);
	}

	if (dptx->status & DPTX_STA_LINK_ON) {
		dptx_regs_pr(dptx, port,
			DPTC_REG_TYPE_DPCD_RECEIVER_CAP, dptx_reg_DPCD_receiver_cap);
		dptx_regs_pr(dptx, port,
			DPTC_REG_TYPE_DPCD_LINK_CONFIG, dptx_reg_DPCD_link_config);
		dptx_regs_pr(dptx, port,
			DPTC_REG_TYPE_DPCD_LINK_STATUS, dptx_reg_DPCD_link_status);
	}
}

void dptx_debug_reset(struct dptx_drv_s *dptx, uint8_t port_mask, uint8_t reset_part)
{
	uint8_t port;

	for (port = 0; port < eDPTX_MAX_PORT; port++) {
		if (!(dptx->sink.port_mask & BIT(port)))
			continue;
		dptx_if_path_reset(dptx, port, reset_part);
	}
}
