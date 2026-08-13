/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __MESON_TX_PHY_CORE_H
#define __MESON_TX_PHY_CORE_H

//#include <linux/platform_device.h>
//#include <phy.h>

/**
 * struct phy_configure_opts_dp - DisplayPort PHY configuration set
 *
 * This structure is used to represent the configuration state of a
 * DisplayPort phy.
 */
struct phy_configure_opts_dp {
	/**
	 * @link_rate:
	 *
	 * Link Rate, in Mb/s, of the main link.
	 *
	 * Allowed values: 1620, 2160, 2430, 2700, 3240, 4320, 5400, 8100 Mb/s
	 */
	unsigned int link_rate;

	/**
	 * @lanes:
	 *
	 * Number of active, consecutive, data lanes, starting from
	 * lane 0, used for the transmissions on main link.
	 *
	 * Allowed values: 1, 2, 4
	 */
	unsigned int lanes;

	/**
	 * @voltage:
	 *
	 * Voltage swing levels, as specified by DisplayPort specification,
	 * to be used by particular lanes. One value per lane.
	 * voltage[0] is for lane 0, voltage[1] is for lane 1, etc.
	 *
	 * Maximum value: 3
	 */
	unsigned int voltage[4];

	/**
	 * @pre:
	 *
	 * Pre-emphasis levels, as specified by DisplayPort specification, to be
	 * used by particular lanes. One value per lane.
	 *
	 * Maximum value: 3
	 */
	unsigned int pre[4];

	/**
	 * @ssc:
	 *
	 * Flag indicating, whether or not to enable spread-spectrum clocking.
	 *
	 */
	u8 ssc : 1;

	/**
	 * @set_rate:
	 *
	 * Flag indicating, whether or not reconfigure link rate and SSC to
	 * requested values.
	 *
	 */
	u8 set_rate : 1;

	/**
	 * @set_lanes:
	 *
	 * Flag indicating, whether or not reconfigure lane count to
	 * requested value.
	 *
	 */
	u8 set_lanes : 1;

	/**
	 * @set_voltages:
	 *
	 * Flag indicating, whether or not reconfigure voltage swing
	 * and pre-emphasis to requested values. Only lanes specified
	 * by "lanes" parameter will be affected.
	 *
	 */
	u8 set_voltages : 1;
};

enum meson_tx_phy_mode {
	PHY_DP,
	PHY_HDMI,
};

struct meson_tx_phy_cfg_opts {
	u32 phy_clock;
	struct phy_configure_opts_dp dp_ops;
};

struct meson_tx_phy_ops {
	int (*init)(void *phy);
	int (*exit)(void *phy);
	int (*power_on)(void *phy);
	int (*power_off)(void *phy);
	int (*set_mode)(void *phy, enum meson_tx_phy_mode mode);
	int (*configure)(void *phy, struct meson_tx_phy_cfg_opts *opts);
	int (*validate)(void *phy, enum meson_tx_phy_mode mode,
				struct meson_tx_phy_cfg_opts *opts);
	int (*calibrate)(void *phy);
	int (*connect)(void *phy, int port);
	int (*disconnect)(void *phy, int port);
	int (*dump_reg)(void *phy);
};

struct meson_tx_phy {
	struct platform_device *pdev;
	struct meson_tx_phy_ops *ops;
	struct phy *phy;
};

struct meson_tx_phy_data {
	struct meson_tx_phy_ops *ops;
	void *para;
};

int meson_tx_phy_init(struct meson_tx_phy  *phy);
int meson_tx_phy_exit(struct meson_tx_phy  *phy);
int meson_tx_phy_power_on(struct meson_tx_phy  *phy);
int meson_tx_phy_power_off(struct meson_tx_phy  *phy);
int meson_tx_phy_set_mode(struct meson_tx_phy  *phy, enum meson_tx_phy_mode mode);
int meson_tx_phy_configure(struct meson_tx_phy  *phy, struct meson_tx_phy_cfg_opts *opts);
int meson_tx_phy_validate(struct meson_tx_phy  *phy, enum meson_tx_phy_mode mode,
			struct meson_tx_phy_cfg_opts *opts);
int meson_tx_phy_calibrate(struct meson_tx_phy  *phy);

#endif
