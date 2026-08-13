// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <amlogic/media/vout/eDPTX/eDPTX.h>
#include "eDP_IP_ctrls.h"
#include "../eDP_common.h"

struct dptx_if_ctrl_s *dptx_if_ctrl;

uint8_t dptx_if_aux_write(struct dptx_drv_s *dptx, uint8_t port,
				uint32_t addr, int len, uint8_t *buf)
{
	if (dptx_if_ctrl->aux_write)
		return dptx_if_ctrl->aux_write(dptx, port, addr, len, buf);
	return 0;
}

uint8_t dptx_if_aux_write_single(struct dptx_drv_s *dptx, uint8_t port, uint32_t addr, uint8_t val)
{
	if (dptx_if_ctrl->aux_write_single)
		return dptx_if_ctrl->aux_write_single(dptx, port, addr, val);
	return 0;
}

uint8_t dptx_if_aux_read(struct dptx_drv_s *dptx, uint8_t port,
			uint32_t addr, int len, uint8_t *buf)
{
	if (dptx_if_ctrl->aux_read)
		return dptx_if_ctrl->aux_read(dptx, port, addr, len, buf);
	return 0;
}

uint8_t dptx_if_aux_i2c_op(struct dptx_drv_s *dptx, uint8_t port,
				uint8_t cmd_type, uint32_t dev_addr, uint8_t len, uint8_t *data)
{
	if (dptx_if_ctrl->aux_i2c_op)
		return dptx_if_ctrl->aux_i2c_op(dptx, port, cmd_type, dev_addr, len, data);
	return 0;
}

void dptx_if_transmit_pattern(struct dptx_drv_s *dptx, uint8_t port, uint8_t pattern, uint8_t lane,
			uint32_t cos_80b_0, uint32_t cos_80b_1, uint32_t cos_80b_2)
{
	if (dptx_if_ctrl->transmit_pattern)
		dptx_if_ctrl->transmit_pattern
			(dptx, port, pattern, lane, cos_80b_0, cos_80b_1, cos_80b_2);
}

void dptx_if_set_MSA(struct dptx_drv_s *dptx, uint8_t port)
{
	if (dptx_if_ctrl->set_MSA)
		dptx_if_ctrl->set_MSA(dptx, port);
}

void dptx_if_path_reset(struct dptx_drv_s *dptx, uint8_t port, uint8_t mask)
{
	if (dptx_if_ctrl->path_reset)
		dptx_if_ctrl->path_reset(dptx, port, mask);
}

void dptx_if_set_lane_cfg(struct dptx_drv_s *dptx, uint8_t port)
{
	if (dptx_if_ctrl->lane_cfg_to_IP)
		dptx_if_ctrl->lane_cfg_to_IP(dptx, port);
}

void dptx_if_set_phy_cfg(struct dptx_drv_s *dptx, uint8_t port, uint8_t lane_mask)
{
	if (dptx_if_ctrl->phy_cfg_to_IP)
		dptx_if_ctrl->phy_cfg_to_IP(dptx, port, lane_mask);
}

void dptx_if_transmitter_init(struct dptx_drv_s *dptx, uint8_t port)
{
	if (dptx_if_ctrl->transmitter_init)
		dptx_if_ctrl->transmitter_init(dptx, port);
}

void dptx_if_transmitter_output(struct dptx_drv_s *dptx, uint8_t port, uint8_t en)
{
	if (dptx_if_ctrl->transmitter_output)
		dptx_if_ctrl->transmitter_output(dptx, port, en);
}

uint8_t dptx_if_get_hpd_level(struct dptx_drv_s *dptx, uint8_t port)
{
	if (dptx_if_ctrl->get_hpd_level)
		return dptx_if_ctrl->get_hpd_level(dptx, port);
	return 0;
}

uint16_t dptx_if_get_hpd_irq(struct dptx_drv_s *dptx, uint8_t port)
{
	if (dptx_if_ctrl->get_hpd_irq)
		return dptx_if_ctrl->get_hpd_irq(dptx, port);
	return 0;
}

void dptx_if_set_hpd_interrupt_mask(struct dptx_drv_s *dptx, uint8_t port, uint8_t mask)
{
	if (dptx_if_ctrl->set_hpd_interrupt_mask)
		dptx_if_ctrl->set_hpd_interrupt_mask(dptx, port, mask);
}

void dptx_if_scramble_reset_set(struct dptx_drv_s *dptx, uint8_t port, uint8_t sr_type)
{
	if (dptx_if_ctrl->scramble_reset_set)
		dptx_if_ctrl->scramble_reset_set(dptx, port, sr_type);
}

void dptx_if_PSR1_ctrl(struct dptx_drv_s *dptx, uint8_t port, uint8_t flag)
{
	if (dptx_if_ctrl->PSR1_SDP_ctrl)
		dptx_if_ctrl->PSR1_SDP_ctrl(dptx, port, flag);
}

void dptx_if_PSR2_ctrl(struct dptx_drv_s *dptx, uint8_t port, uint8_t flag)
{
	if (dptx_if_ctrl->PSR2_SDP_en)
		dptx_if_ctrl->PSR2_SDP_en(dptx, port, flag);
}

void dptx_if_reg_store(struct dptx_drv_s *dptx, uint8_t port, uint32_t d0, uint32_t d1)
{
	if (dptx_if_ctrl->reg_store)
		dptx_if_ctrl->reg_store(dptx, port, d0, d1);
}

void dptx_if_reg_store_get(struct dptx_drv_s *dptx, uint8_t port, uint32_t *d0, uint32_t *d1)
{
	if (dptx_if_ctrl->reg_store_get)
		dptx_if_ctrl->reg_store_get(dptx, port, d0, d1);
}

void edptx_if_IP_probe(struct dptx_drv_s *dptx)
{
	switch (dptx->data->chip_type) {
#if defined(CONFIG_MESON_T7) || defined(CONFIG_MESON_T7C)
	case eDPTX_CHIP_T7:
		dptx_if_ctrl = dptx_if_bind_t7(dptx);
		break;
#endif
#if defined(CONFIG_MESON_A9)
	case eDPTX_CHIP_A9:
		if (1)
			dptx_if_ctrl = dptx_if_bind_tr14(dptx);
		else
			dptx_if_ctrl = dptx_if_bind_third_party_API(dptx);
		break;
#endif
	default:
		dptx_if_ctrl = NULL;
		DPTX_ERR(dptx, "%s: invalid chip type", __func__);
		return;
	}
}
