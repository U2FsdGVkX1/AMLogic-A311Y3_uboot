// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

// #include <common.h>
#include <amlogic/media/vout/eDPTX/eDPTX.h>
#include "eDP_phy_config.h"
#include "../eDP_common.h"

struct dptx_phy_ctrl_s *dptx_phy_ctrl;

void dptx_phy_enable(struct dptx_drv_s *dptx, uint8_t port)
{
	if (!dptx_phy_ctrl->phy_enable) {
		DPTX_P_PR(dptx, port, "%s: phy_enable is null", __func__);
		return;
	}

	DPTX_P_DBG(dptx, port, "%s", __func__);
	dptx_phy_ctrl->phy_enable(dptx, port);
}

void dptx_phy_disable(struct dptx_drv_s *dptx, uint8_t port)
{
	if (!dptx_phy_ctrl->phy_disable) {
		DPTX_P_PR(dptx, port, "%s: phy_disable is null", __func__);
		return;
	}

	DPTX_P_DBG(dptx, port, "%s", __func__);
	dptx_phy_ctrl->phy_disable(dptx, port);
}

void dptx_phy_set_lane(struct dptx_drv_s *dptx, uint8_t port, uint8_t lane_mask)
{
	if (!dptx_phy_ctrl->phy_disable) {
		DPTX_P_PR(dptx, port, "%s: phy_disable is null", __func__);
		return;
	}

	DPTX_P_DBG(dptx, port, "%s", __func__);
	dptx_phy_ctrl->phy_set_lane(dptx, port, lane_mask);
}

void edptx_phy_probe(struct dptx_drv_s *dptx)
{
	switch (dptx->data->chip_type) {
#if defined(CONFIG_MESON_T7) || defined(CONFIG_MESON_T7C)
	case eDPTX_CHIP_T7:
		dptx_phy_ctrl = dptx_phy_config_init_t7();
		break;
#endif
#if defined(CONFIG_MESON_A9)
	case eDPTX_CHIP_A9:
		dptx_phy_ctrl = dptx_phy_config_init_a9();
		break;
#endif
	default:
		break;
	}
}
