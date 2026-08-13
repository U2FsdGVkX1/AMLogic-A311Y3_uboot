// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

//#include <linux/module.h>
#include <init.h>
//#include <linux/platform_device.h>
#include <dm/of.h>
//#include <dm/of_device.h>
//#include <linux/of_platform.h>
//#include <linux/component.h>
#include <linux/compat.h> //#include <linux/vmalloc.h>
#include <dm/of_addr.h>

#include <amlogic/media/vout/meson_tx_connector/dptx_common/dptx_common.h>
#include <amlogic/media/vout/meson_tx_connector/dptx_common/dptx_hw_common.h>
#include <amlogic/media/vout/meson_tx_connector/meson_tx_log.h>
#include <amlogic/media/vout/meson_tx_connector/clk/meson_tx_clk.h>
#include <amlogic/media/vout/meson_tx_connector/venc/meson_venc.h>
#include <amlogic/cpu_id.h>
//#include <drm/amlogic/meson_drm_bind.h>

#include "../meson_tx_internal.h"
#include "dptx_log.h"
#include "dptx_internal.h"
#include "dptx_hw_opcode.h"

#define DEVICE_NAME "dptx"
#define DPTX_DEV_COUNT 2

struct dptx_plat_data_s {
	enum dptx_chip_type_e chip_type;
	const char *chip_name;
	u8 dptx_core_max;
};

struct dptx_common *global_tx_comm[DPTX_DEV_COUNT] = {NULL};
struct dptx_plat_data_s *dptx_chip_data;

/* refer to dts node("dptx0", "dptx1"...), 0/1 for dptx device idx */
struct dptx_common *get_dptx_device(u8 idx)
{
	if (!dptx_chip_data)
		return NULL;

	if (idx >= dptx_chip_data->dptx_core_max)
		return NULL;

	return global_tx_comm[idx];
}

static int dptx_get_dt_info(struct dptx_common *tx_comm,
			struct dptx_hw_common *hw_comm, u8 idx)
{
	int ret;
	u32 dev_index = 0;
	//u32 irq_index;
	//u32 val = 0;
	u32 i;
	//struct resource res;
	struct dptx_cap *tx_cap = NULL;
	const void *dt_blob;
	int node;
	char *propdata;
	//todo for build pass
	//struct platform_device *pdev;
	//struct device_node *np;
	char dptx_node_name[10] = {0};

	if (!tx_comm || !hw_comm)
		return -1;

	tx_cap = kzalloc(sizeof(*tx_cap), GFP_KERNEL);
	if (!tx_cap)
		return -ENOMEM;

	dt_blob = gd->fdt_blob;
	if (dt_blob == NULL) {
		kfree(tx_cap);
		DPTX_INFO("ERR: dptx: dt_blob is null\n");
		return -1;
	}

	ret = fdt_check_header(dt_blob);
	if (ret < 0) {
		kfree(tx_cap);
		DPTX_ERROR("dptx: check dts: %s\n", fdt_strerror(ret));
		return -1;
	}

	snprintf(dptx_node_name, 10, "%s%d", "/dptx", idx);
	node = fdt_path_offset(dt_blob, dptx_node_name);
	if (node < 0) {
		kfree(tx_cap);
		DPTX_ERROR("dptx: not find /dptx node: %s\n", fdt_strerror(node));
		return -1;
	}

	propdata = (char *)fdt_getprop(dt_blob, node, "dev_index", NULL);
	if (propdata) {
		dev_index = be32_to_cpup((u32 *)propdata);
		if (dev_index >= dptx_chip_data->dptx_core_max) {
			tx_comm->dev_idx = 0;
			DPTX_INFO("%s: index exceed maximum count and set 0\n", __func__);
		} else {
			tx_comm->dev_idx = dev_index;
		}
	} else {
		tx_comm->dev_idx = 0;
		DPTX_INFO("%s: no index exist, default to 0\n", __func__);
	}
	hw_comm->dev_idx = tx_comm->dev_idx;

	propdata = (char *)fdt_getprop(dt_blob, node, "is_edp", NULL);
	if (propdata)
		tx_comm->is_edp = be32_to_cpup((u32 *)propdata);
	else
		tx_comm->is_edp = 0;

	/* get tx capability from dts */
	tx_comm->base.tx_cap = tx_cap;
	propdata = (char *)fdt_getprop(dt_blob, node, "max_link_rate", NULL);
	if (propdata) {
		tx_cap->max_link_rate = be32_to_cpup((u32 *)propdata);
	} else {
		DPTX_INFO("%s: no max_link_rate, default to 5400000\n", __func__);
		tx_cap->max_link_rate = 54000000;
	}
	propdata = (char *)fdt_getprop(dt_blob, node, "max_lane_count", NULL);
	if (propdata) {
		tx_cap->max_lane_count = be32_to_cpup((u32 *)propdata);
	} else {
		DPTX_INFO("%s: no max_lane_count, default to 4\n", __func__);
		tx_cap->max_lane_count = 4;
	}
	propdata = (char *)fdt_getprop(dt_blob, node, "max_fresh_rate", NULL);
	if (propdata) {
		tx_cap->max_fresh_rate = be32_to_cpup((u32 *)propdata);
	} else {
		DPTX_INFO("%s: no max_fresh_rate, default to 60\n", __func__);
		tx_cap->max_fresh_rate = 60;
	}
	propdata = (char *)fdt_getprop(dt_blob, node, "max_h_active", NULL);
	if (propdata) {
		tx_cap->max_h_active = be32_to_cpup((u32 *)propdata);
	} else {
		DPTX_INFO("%s: no max_h_active, default to 3840\n", __func__);
		tx_cap->max_h_active = 3840;
	}
	propdata = (char *)fdt_getprop(dt_blob, node, "max_v_active", NULL);
	if (propdata) {
		tx_cap->max_v_active = be32_to_cpup((u32 *)propdata);
	} else {
		DPTX_INFO("%s: no max_v_active, default to 3840\n", __func__);
		tx_cap->max_v_active = 3840;
	}
	propdata = (char *)fdt_getprop(dt_blob, node, "pxp_mode", NULL);
	if (propdata)
		tx_comm->base.pxp_mode = be32_to_cpup((u32 *)propdata);
	else
		tx_comm->base.pxp_mode = 0;
	hw_comm->hw_base.pxp_mode = tx_comm->base.pxp_mode;
	DPTX_INFO("%s: pxp_mode:%d\n", __func__, tx_comm->base.pxp_mode);

	propdata = (char *)fdt_getprop(dt_blob, node, "ext_edptx_en", NULL);
	if (propdata)
		tx_comm->ext_edptx_drv_en = be32_to_cpup((u32 *)propdata);
	else
		tx_comm->ext_edptx_drv_en = 1;

	hw_comm->hw_base.regs_region = kzalloc(sizeof(*hw_comm->hw_base.regs_region) * REG_IDX_MAX,
		GFP_KERNEL);
	if (!hw_comm->hw_base.regs_region) {
		DPTX_ERROR("cannot alloc memory for regs_region\n");
		return -ENOMEM;
	}

	propdata = (char *)fdt_getprop(dt_blob, node, "reg", NULL);
	if (!propdata) {
		DPTX_ERROR("cannot find reg resource\n");
		kfree(hw_comm->hw_base.regs_region);
		return -ENOMEM;
	}

	/* get the core, vpu, ana, ... etc address */
	for (i = 0; i < REG_IDX_MAX; i++) {
		hw_comm->hw_base.regs_region[i] = be32_to_cpup((((u32 *)propdata) + i));
		DPTX_INFO("Mapped Addr: 0x%x\n", hw_comm->hw_base.regs_region[i]);
	}

#if 0
	if (tx_comm->is_edp == 0) {
		irq_index = platform_get_irq_byname(pdev, "apb_dp_int");
		if (irq_index == -ENXIO) {
			DPTX_ERROR("%s: apb_dp_int not found\n", __func__);
			return -ENXIO;
		}
		hw_comm->dptx_irq_id = irq_index;
		irq_index = platform_get_irq_byname(pdev, "hdcp2tx_intr");
		if (irq_index == -ENXIO) {
			DPTX_ERROR("%s: hdcp2tx_intr not found\n", __func__);
			return -ENXIO;
		}
		hw_comm->hdcp2tx_irq_id = irq_index;
	} else {
		irq_index = platform_get_irq_byname(pdev, "apb_int_edptx");
		if (irq_index == -ENXIO) {
			DPTX_ERROR("%s: apb_int_edptx not found\n", __func__);
			return -ENXIO;
		}
		hw_comm->dptx_irq_id = irq_index;
	}
#endif

	return 0;
}

static struct meson_tx_phy *meson_tx_probe_phy(struct meson_tx_hw *tx_hw)
{
	struct meson_tx_phy *tx_phy = NULL;

#if 0
	struct platform_device *phy_pdev;
	struct device_node *phy_node;

	phy_node = of_parse_phandle(dev->of_node, "tx_phy", 0);
	if (!phy_node) {
		dev_err(dev, "cannot find phy device\n");
		return NULL;
	}

	phy_pdev = of_find_device_by_node(phy_node);
	if (phy_pdev)
		tx_phy = platform_get_drvdata(phy_pdev);

	of_node_put(phy_node);

	if (!phy_pdev) {
		dev_err(dev, "%s: phy driver is not ready\n", __func__);
		return NULL;
	}
	if (!tx_phy) {
		put_device(&phy_pdev->dev);
		dev_err(dev, "%s: phy driver is not ready\n", __func__);
		return NULL;
	}
#endif
	return tx_phy;
}

static int dptx_probe(u8 idx)
{
#if 0
	int ret = 0;
	struct dptx_common *tx_comm;
	struct meson_tx_hw *tx_hw;
	const struct meson_tx_plat_data *plat_data;
	struct device *device = &pdev->dev;

	struct meson_tx_phy *tx_phy;
	struct meson_tx_clk *tx_clk;

	/* get config from device tree match */
	plat_data = of_device_get_match_data(&pdev->dev);
	if (!plat_data)
		return -ENOMEM;

	tx_hw = plat_data->alloc_tx_hw();
	if (!tx_hw)
		return -ENOMEM;

	tx_comm = kzalloc(sizeof(*tx_comm), GFP_KERNEL);
	if (!tx_comm) {
		ret = -ENOMEM;
		goto dptx_common_alloc_fail;
	}
	tx_comm->is_edp	= plat_data->is_edp;
	tx_comm->base.pdev = device;
	tx_comm->hw_comm = to_dptx_hw_common(tx_hw);

	/* get config from dts */
	ret = dptx_get_dt_info(pdev, tx_comm, tx_comm->hw_comm);
	if (ret < 0)
		goto dptx_get_dt_info_fail;

	/* dptx common init */
	ret = dptx_common_init(tx_comm, tx_comm->hw_comm);
	if (ret < 0)
		goto dptx_common_init_fail;

	ret = tx_hw->init_tx_hw(tx_comm, tx_hw);
	if (ret < 0)
		goto dptx_init_tx_hw_fail;

	/* dptx probe phy */
	tx_phy = meson_tx_probe_phy(device, tx_hw);
	if (!tx_phy) {
		ret = -ENODEV;
		goto meson_tx_probe_phy_fail;
	}
	/* dptx tx hw setup tx_phy */
	meson_tx_hw_setup_phy(tx_hw, tx_phy);

	tx_clk = meson_tx_probe_clk(device);
	if (!tx_clk) {
		ret = -ENODEV;
		goto meson_tx_probe_clk_fail;
	}
	meson_tx_hw_setup_clk(tx_hw, tx_clk);

	dev_set_drvdata(device, tx_comm);

	component_add(device, &dptx_bind_ops);

	DPTX_INFO("%s for dev_index[%d]\n", __func__, tx_comm->dev_idx);

	return 0;
meson_tx_probe_clk_fail:
meson_tx_probe_phy_fail:
	tx_comm->base.tx_hw_base->uninit_tx_hw(tx_comm, tx_comm->base.tx_hw_base);
dptx_init_tx_hw_fail:
	dptx_common_uninit(tx_comm);
dptx_common_init_fail:
dptx_get_dt_info_fail:
	kfree(tx_comm);
	tx_comm = NULL;
dptx_common_alloc_fail:
	plat_data->free_tx_hw(tx_hw);

	return ret;
#endif
	int ret = 0;
	struct dptx_common *tx_comm;
	struct meson_tx_hw *tx_hw;

	struct meson_tx_phy *tx_phy;
	struct meson_tx_clk *tx_clk;

	tx_hw = dptx20_alloc_tx_hw();
	if (!tx_hw)
		return -ENOMEM;

	tx_comm = kzalloc(sizeof(*tx_comm), GFP_KERNEL);
	if (!tx_comm) {
		ret = -ENOMEM;
		goto dptx_common_alloc_fail;
	}

	tx_comm->hw_comm = to_dptx_hw_common(tx_hw);

	/* get config from dts */
	ret = dptx_get_dt_info(tx_comm, tx_comm->hw_comm, idx);
	if (ret < 0)
		goto dptx_get_dt_info_fail;

	/* dptx common init */
	ret = dptx_common_init(tx_comm, tx_comm->hw_comm);
	if (ret < 0)
		goto dptx_common_init_fail;

	/* dptx probe phy */
	tx_phy = meson_tx_probe_phy(tx_hw);

	/* dptx tx hw setup tx_phy */
	meson_tx_hw_setup_phy(tx_hw, tx_phy);
	tx_hw->init_tx_hw(tx_comm, tx_hw);
	if (ret < 0)
		goto dptx_init_tx_hw_fail;

	tx_clk = meson_tx_clk_probe();
	if (!tx_clk) {
		ret = -ENODEV;
		goto meson_tx_probe_clk_fail;
	}
	meson_tx_hw_setup_clk(tx_hw, tx_clk);

	/* dev_set_drvdata(device, tx_comm); */
	/* component_add(device, &dptx_bind_ops); */
	global_tx_comm[tx_comm->dev_idx] = tx_comm;
	DPTX_INFO("%s for dev_index[%d]\n", __func__, tx_comm->dev_idx);

	return 0;

meson_tx_probe_clk_fail:
/* meson_tx_probe_phy_fail: */
	tx_comm->base.tx_hw_base->uninit_tx_hw(tx_comm, tx_hw);
dptx_init_tx_hw_fail:
	dptx_common_uninit(tx_comm);

dptx_common_init_fail:
	kfree(tx_comm->base.tx_cap);
	tx_comm->base.tx_cap = NULL;
dptx_get_dt_info_fail:
	kfree(tx_comm);
	tx_comm = NULL;
dptx_common_alloc_fail:
	dptx20_free_tx_hw(tx_hw);

	return ret;
}

/* only for external eDPTX driver. dptx core and
 * edptx core may be both used for edptx,
 * so use idx to get the corresponding edptx
 * instance in such case, or get the first edptx
 * instance if dptx core is not used for edptx.
 */
struct dptx_common *dptx_get_edptx_inst(u8 idx)
{
	u8 i = 0;
	struct dptx_common *tx_comm = NULL;

	if (idx >= dptx_chip_data->dptx_core_max)
		return NULL;

	/* find the edptx with index in instance list */
	if (global_tx_comm[idx] && global_tx_comm[idx]->is_edp)
		return global_tx_comm[idx];

	/* find the first edptx in instance list */
	for (i = 0; i < dptx_chip_data->dptx_core_max; i++) {
		if (global_tx_comm[i] && global_tx_comm[i]->is_edp) {
			tx_comm	= global_tx_comm[i];
			break;
		}
	}

	return tx_comm;
}

static struct dptx_plat_data_s dptx_data_a9 = {
	.chip_type = DPTX_CHIP_ID_A9,
	.chip_name = "A9",
	.dptx_core_max = 2,
};

int dptx_init(void)
{
	u32 chip_type;
	u8 idx = 0;
	int ret = 0;

	chip_type = get_cpu_id().family_id;
	switch (chip_type) {
	case MESON_CPU_MAJOR_ID_A9:
		dptx_chip_data = &dptx_data_a9;
		break;
	default:
		dptx_chip_data = NULL;
		DPTX_ERROR("not support chip type: %d\n", chip_type);
		return -1;
	}

	DPTX_INFO("chip: %d %s", dptx_chip_data->chip_type, dptx_chip_data->chip_name);

	for (idx = 0; idx < dptx_chip_data->dptx_core_max && idx < DPTX_DEV_COUNT; idx++) {
		ret = dptx_probe(idx);
		if (ret < 0)
			return ret;
	}
	ret = meson_venc_init();
	if (ret < 0)
		DPTX_ERROR("dptx venc init with error\n");
	return ret;
}
