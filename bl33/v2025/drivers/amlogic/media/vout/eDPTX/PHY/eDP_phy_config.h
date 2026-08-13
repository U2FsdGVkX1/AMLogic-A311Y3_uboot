/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef _AML_DPTX_PHY_CONFIG_H
#define _AML_DPTX_PHY_CONFIG_H
#include <amlogic/media/vout/eDPTX/eDPTX.h>
#include <linux/types.h>

struct dptx_phy_ctrl_s {
	unsigned int lane_lock;
	void (*phy_enable)(struct dptx_drv_s *dptx, uint8_t port);
	void (*phy_disable)(struct dptx_drv_s *dptx, uint8_t port);
	void (*phy_set_lane)(struct dptx_drv_s *dptx, uint8_t port, uint8_t lane_mask);
};

#if defined(CONFIG_MESON_T7) || defined(CONFIG_MESON_T7C)
struct dptx_phy_ctrl_s *dptx_phy_config_init_t7(void);
#endif

#if defined(CONFIG_MESON_A9)
struct dptx_phy_ctrl_s *dptx_phy_config_init_a9(void);
#endif

#endif
