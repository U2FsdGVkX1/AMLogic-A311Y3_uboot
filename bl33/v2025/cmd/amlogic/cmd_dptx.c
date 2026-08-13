
// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <command.h>
#include <env.h>
#include <malloc.h>
#include <asm/byteorder.h>
#include <amlogic/clk_measure.h>
#include <linux/delay.h>
#include <image.h>
#include <linux/libfdt_env.h>
#include <linux/arm-smccc.h>
#include <linux/compat.h>

#include <amlogic/media/vout/meson_tx_connector/meson_tx_edid.h>
#include <amlogic/media/vout/meson_tx_connector/meson_tx_hw_opcode.h>
#include <amlogic/media/vout/meson_tx_connector/dptx_common/dptx_common.h>
#include <amlogic/media/vout/meson_tx_connector/dptx_common/dptx_hw_common.h>
#include <amlogic/media/vout/meson_tx_connector/venc/meson_venc.h>
#include "../../drivers/amlogic/media/vout/meson_tx_connector/dptx/dptx_internal.h"
#include "../../drivers/amlogic/media/vout/meson_tx_connector/dptx/dptx_aux_helper.h"
#include "../../drivers/amlogic/media/vout/meson_tx_connector/dptx/dptx_log.h"
#include "../../drivers/amlogic/media/vout/meson_tx_connector/dptx/dptx_link.h"

#define DP_SINK_COUNT			    0x200

//#define msleep(i) udelay((i) * 1000)
//static unsigned char edid_raw_buf[512] = {0};

static unsigned char displayid_test[384] = {
	0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x1C, 0x54, 0x00, 0x28, 0x01, 0x01, 0x01, 0x01,
	0x28, 0x1F, 0x01, 0x03, 0x80, 0x3F, 0x24, 0x78, 0xEE, 0x0E, 0x10, 0xAE, 0x51, 0x44, 0xA2, 0x25,
	0x0E, 0x50, 0x54, 0x25, 0x4A, 0x00, 0xD1, 0xC0, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x08, 0xE8, 0x00, 0x30, 0xF2, 0x70, 0x5A, 0x80, 0xB0, 0x58,
	0x8A, 0x00, 0xB9, 0x88, 0x21, 0x00, 0x00, 0x1E, 0x00, 0x00, 0x00, 0xFD, 0x00, 0x30, 0x90, 0x1E,
	0x87, 0x3C, 0x00, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x4D,
	0x32, 0x38, 0x55, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0xFF,
	0x00, 0x32, 0x31, 0x34, 0x30, 0x30, 0x42, 0x30, 0x30, 0x35, 0x30, 0x38, 0x30, 0x0A, 0x02, 0x95,
	0x02, 0x03, 0x50, 0xF2, 0x4A, 0x61, 0x76, 0x01, 0x03, 0x04, 0x2F, 0x10, 0x3F, 0x5D, 0x20, 0x23,
	0x09, 0x07, 0x07, 0x83, 0x01, 0x00, 0x00, 0x67, 0x03, 0x0C, 0x00, 0x10, 0x00, 0x38, 0x44, 0x6D,
	0xD8, 0x5D, 0xC4, 0x01, 0x78, 0x88, 0x33, 0x02, 0x30, 0x90, 0xC3, 0x35, 0x0C, 0x6D, 0x1A, 0x00,
	0x00, 0x02, 0x0B, 0x30, 0x90, 0xEC, 0x0F, 0x66, 0x1F, 0x66, 0x1F, 0xE3, 0x05, 0xC3, 0x01, 0xE2,
	0x0F, 0x03, 0xE3, 0x0E, 0x61, 0x76, 0xE6, 0x06, 0x05, 0x01, 0x66, 0x66, 0x1F, 0xE2, 0x00, 0xD5,
	0x56, 0xC2, 0x00, 0xA0, 0xA0, 0xA0, 0x55, 0x50, 0x30, 0x20, 0x35, 0x00, 0xB9, 0x88, 0x21, 0x00,
	0x00, 0x1E, 0x56, 0x5E, 0x00, 0xA0, 0xA0, 0xA0, 0x29, 0x50, 0x30, 0x20, 0x35, 0x00, 0xB9, 0x88,
	0x21, 0x00, 0x00, 0x1E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x6C,
	0x70, 0x12, 0x79, 0x00, 0x00, 0x03, 0x01, 0x3C, 0xBA, 0xE9, 0x01, 0x04, 0xFF, 0x0E, 0x4F, 0x00,
	0x07, 0x80, 0x1F, 0x00, 0x6F, 0x08, 0x3C, 0x00, 0x09, 0x00, 0x07, 0x00, 0xC1, 0xB0, 0x00, 0x04,
	0x7F, 0x07, 0xEF, 0x02, 0xA7, 0x80, 0xCF, 0x00, 0x37, 0x04, 0x61, 0x00, 0x02, 0x80, 0x04, 0x00,
	0x2F, 0xE7, 0x00, 0x04, 0xFF, 0x09, 0xB3, 0x00, 0x1B, 0x80, 0x1F, 0x00, 0x9F, 0x05, 0x3B, 0x00,
	0x08, 0x80, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC7, 0x90
};

static int dptx_load_fixed_edid(struct dptx_common *tx_comm)
{
	int i;

	if (!tx_comm)
		return -1;

	for (i = 0; i < 384; i++)
		tx_comm->base.edid_buf[i] = displayid_test[i];

	meson_tx_edid_parse(&tx_comm->base.rxcap, tx_comm->base.edid_buf, tx_comm->base.edid_parse_mask);

	return 0;
}

static int do_dptx_init(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	dptx_init();
	return 0;
}

/*
 * cmd usage: dp_tx output idx cmd
 * for example: "dp_tx output 0 test_attr" for dptx0
 */
static int dptx_do_output(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	struct dptx_common *tx_comm = NULL;
	struct tx_timing *timing_list = NULL;
	int cnt = 0;
	int ret = 0;
	u32 idx = 0;
	static char test_color_attr[16];

	if (argc < 2)
		return cmd_usage(cmdtp);

	idx = simple_strtoul(argv[1], NULL, 0);
	tx_comm = get_dptx_device(idx & 0xFF);
	if (!tx_comm)
		return CMD_RET_FAILURE;
	if (strcmp(argv[2], "bist") == 0) {
		//bist
	} else if (strncmp(argv[2], "get_modes", 8) == 0) {
		cnt = dptx_get_mode_list(tx_comm, &timing_list);
		if (cnt > 0) {
			/* meson_dptx_mode_probed_add(count, timing_list, connector); */
			kfree(timing_list);
			timing_list = NULL;
		}
		if (tx_comm->base.rxcap.disp_id_cap.version)
			display_id_cap_print(&tx_comm->base.rxcap.disp_id_cap);
		DPTX_INFO("%s get mode list count: %d\n", __func__, cnt);
	} else if (strncmp(argv[2], "load_edid", 9) == 0) {
		dptx_load_fixed_edid(tx_comm);
		DPTX_INFO("%s load and use fixed edid\n", __func__);
	} else if (strncmp(argv[2], "test_attr", 9) == 0) {
		/* step1: for pxp test color attribute, for example: "y444,10bit,f" */
		if (argc >= 4)
			strncpy(test_color_attr, argv[3], sizeof(test_color_attr) - 1);
		else
			strcpy(test_color_attr, "rgb,8bit");

		test_color_attr[15] = '\0';
		DPTX_INFO("set test_color_attr: %s\n", test_color_attr);
	} else if (strncmp(argv[2], "colorimetry", 11) == 0) {
		u32 colorimetry = 0;

		if (argc < 4) {
			DPTX_ERROR("%s invalid colorimetry\n", __func__);
			return CMD_RET_USAGE;
		}

		colorimetry = simple_strtoul(argv[3], NULL, 0);
		tx_comm->base.sw_fmt_para.colorimetry = colorimetry & 0xFF;
		DPTX_INFO("set colorimetry: %d\n", colorimetry);
	} else if (strncmp(argv[2], "test_timing_idx", 15) == 0) {
		/* step2 build format para with test_attr and test_mode */
		const struct tx_timing *match_timing = NULL;
		struct tx_timing *tmp_timing = NULL;
		struct meson_tx_format_para *sw_para = NULL;
		struct dptx_hw_fmt_para *hw_para = NULL;
		enum hdmi_color_depth cd = COLORDEPTH_24B;
		enum hdmi_colorspace cs = HDMI_COLORSPACE_RGB;
		enum hdmi_quantization_range cr = HDMI_QUANTIZATION_RANGE_LIMITED;
		u32 timing_idx = 0;

		if (argc < 4) {
			DPTX_ERROR("%s invalid timing mode\n", __func__);
			return CMD_RET_USAGE;
		}

		timing_idx = simple_strtoul(argv[3], NULL, 0);
		DPTX_INFO("Using timing_idx: %d\n", timing_idx);
		/* return timing in mode list, not check tx/rx cap */
		match_timing = meson_tx_mode_index_to_timing(timing_idx);
		if (!match_timing) {
			DPTX_ERROR("no timing with timing_idx(%d)\n", timing_idx);
			return CMD_RET_FAILURE;
		}
		tmp_timing = kzalloc(sizeof(*tmp_timing), GFP_KERNEL);
		if (!tmp_timing)
			return CMD_RET_FAILURE;
		memcpy(tmp_timing, match_timing, sizeof(struct tx_timing));

		if (strstr(test_color_attr, "6bit"))
			cd = COLORDEPTH_18B;
		else if (strstr(test_color_attr, "8bit"))
			cd = COLORDEPTH_24B;
		else if (strstr(test_color_attr, "10bit"))
			cd = COLORDEPTH_30B;
		else if (strstr(test_color_attr, "12bit"))
			cd = COLORDEPTH_36B;
		else
			cd = COLORDEPTH_24B;

		if (strstr(test_color_attr, "y444"))
			cs = HDMI_COLORSPACE_YUV444;
		else if (strstr(test_color_attr, "y422"))
			cs = HDMI_COLORSPACE_YUV422;
		else if (strstr(test_color_attr, "y420"))
			cs = HDMI_COLORSPACE_YUV420;
		else
			cs = HDMI_COLORSPACE_RGB;

		/* "f" means full */
		if (strstr(test_color_attr, "f"))
			cr = HDMI_QUANTIZATION_RANGE_FULL;
		else
			cr = HDMI_QUANTIZATION_RANGE_LIMITED;

		sw_para = &tx_comm->base.sw_fmt_para;
		hw_para = &sw_para->tx_hw_para.dptx_hw_para;

		/* build SW/HW format param */
		if (meson_tx_format_para_init(&tx_comm->base, tmp_timing, 0, cs, cd, cr, sw_para)) {
			DPTX_ERROR("%s build SW para failed\n", __func__);
			kfree(tmp_timing);
			return CMD_RET_FAILURE;
		}
		if (dptx_calc_hw_fmt_para(&tx_comm->base, sw_para, hw_para)) {
			DPTX_ERROR("%s build HW para failed\n", __func__);
			kfree(tmp_timing);
			return CMD_RET_FAILURE;
		}
		DPTX_INFO("%s build test format: %s cs[%d] cd[%d] cr[%d]\n",
			__func__, tmp_timing->name, cs, cd, cr);
		kfree(tmp_timing);
	} else if (strncmp(argv[2], "clk_path", 8) == 0) {
		int clk_path;
		if (argc < 4) {
			DPTX_ERROR("%s invalid clk_path\n", __func__);
			return CMD_RET_USAGE;
		}
		clk_path = simple_strtoul(argv[3], NULL, 0);
		DPTX_INFO("Using val: %d\n", clk_path);
		/*
		 * step2.2 build clk path
		 * bit[3:0]: venc_idx, bit[7:4]: hdmi_if_idx
		 */
		meson_tx_build_clk_param(&tx_comm->base, &tx_comm->base.sw_fmt_para,
			clk_path & 0xF, (clk_path >> 4) & 0xF);
		DPTX_INFO("%s build clk_path: enc_idx: %x, hdmi_if_idx: %x\n",
			__func__, clk_path & 0xF, (clk_path >> 4) & 0xF);
	} else if (strncmp(argv[2], "pre_enable", 10) == 0) {
		/* step3: dptx_pre_mode_enable */
		dptx_hw_cntl(tx_comm->base.tx_hw_base, MODE_FLOW_PRE_ENABLE_MODE,
			&tx_comm->base.sw_fmt_para, NULL);
		DPTX_INFO("%s dptx_pre_mode_enable\n", __func__);
	} else if (strncmp(argv[2], "force_pattern", 13) == 0) {
		int force_pattern;
		if (argc < 4) {
			DPTX_ERROR("%s force tps pattern\n", __func__);
			return CMD_RET_USAGE;
		}
		force_pattern = simple_strtoul(argv[3], NULL, 0);

		tx_comm->hw_comm->force_tps_pattern = force_pattern;
		DPTX_INFO("%s force training pattern: 0x%x\n", __func__, force_pattern);
	} else if (strncmp(argv[2], "link_train", 10) == 0) {
		/* step4: link training */
		ret = dptx_link_training(tx_comm->link_train);
		DPTX_INFO("%s link training: %s\n", __func__, ret == 0 ? "pass" : "failed");
	} else if (strncmp(argv[2], "lt_timeout_ms", 13) == 0) {
		int timeout_ms;
		if (argc < 4) {
			DPTX_ERROR("%s invalid lt_timeout_ms\n", __func__);
			return CMD_RET_USAGE;
		}
		timeout_ms = simple_strtoul(argv[3], NULL, 0);

		tx_comm->link_train->timeout_ms = timeout_ms;
		DPTX_INFO("%s dptx link train timeout: %dms\n", __func__, timeout_ms);
	} else if (strncmp(argv[2], "lt_force_lr", 11) == 0) {
		int force_lr;
		if (argc < 4) {
			DPTX_ERROR("%s invalid lt_force_lr\n", __func__);
			return CMD_RET_USAGE;
		}
		force_lr = simple_strtoul(argv[3], NULL, 0);

		tx_comm->link_train->force_lr = force_lr;
		DPTX_INFO("%s force link rate: 0x%x\n", __func__, force_lr);
	} else if (strncmp(argv[2], "lt_force_lc", 11) == 0) {
		int force_lc;
		if (argc < 4) {
			DPTX_ERROR("%s invalid lt_force_lc\n", __func__);
			return 0;
		}
		force_lc = simple_strtoul(argv[3], NULL, 0);

		tx_comm->link_train->force_lc = force_lc;
		DPTX_INFO("%s force lane count: %d\n", __func__, force_lc);
	} else if (strncmp(argv[2], "vid_clk_sync", 12) == 0) {
		int vid_clk_sync_mode;
		if (argc < 3) {
			DPTX_ERROR("%s invalid vid_clk_sync\n", __func__);
			return CMD_RET_USAGE;
		}
		vid_clk_sync_mode = simple_strtoul(argv[3], NULL, 0);

		tx_comm->hw_comm->vid_clk_sync_mode = !!vid_clk_sync_mode;
		DPTX_INFO("%s set vid_clk_sync_mode: %d\n", __func__, !!vid_clk_sync_mode);
	} else if (strncmp(argv[2], "video_mode_set", 14) == 0) {
		/* step5: set VPU format & CORE */
		dptx_hw_cntl(tx_comm->base.tx_hw_base, MODE_FLOW_ENABLE_MODE, &tx_comm->base.sw_fmt_para, NULL);
		DPTX_INFO("%s dptx_mode_enable\n", __func__);
	} else if (strncmp(argv[2], "venc_index_set", 14) == 0) {
		u32 venc_index;

		if (argc < 4) {
			DPTX_ERROR("%s invalid venc_index\n", __func__);
			return CMD_RET_USAGE;
		}
		venc_index = simple_strtoul(argv[3], NULL, 0);

		tx_comm->hw_comm->enc_index = venc_index;
		DPTX_INFO("%s set venc index = %d\n", __func__, tx_comm->hw_comm->enc_index);
	} else if (strncmp(argv[2], "venc_type_set", 13) == 0) {
		u32 venc_type;

		if (argc < 4) {
			DPTX_ERROR("%s invalid venc_type\n", __func__);
			return CMD_RET_USAGE;
		}
		venc_type = simple_strtoul(argv[3], NULL, 0);

		if (venc_type == 0)
			tx_comm->hw_comm->enc_type = VENC_ENCP;
		else if (venc_type == 1)
			tx_comm->hw_comm->enc_type = VENC_ENCL;
		DPTX_INFO("%s set venc type = %d\n", __func__, tx_comm->hw_comm->enc_type);
	} else if (strncmp(argv[2], "venc_set", 8) == 0) {
		meson_venc_mode_set(tx_comm->hw_comm->enc_index, tx_comm->hw_comm->enc_type,
			VENC_BIST_PTTN_OFF, &tx_comm->base.sw_fmt_para);
		DPTX_INFO("%s venc mode set done\n", __func__);
	} else if (strncmp(argv[2], "venc_bist_set", 13) == 0) {
		enum venc_bist_type venc_bist = VENC_BIST_PTTN_OFF;

		if (argc < 4) {
			DPTX_ERROR("%s invalid venc_bist pattern\n", __func__);
			return CMD_RET_USAGE;
		}

		venc_bist = simple_strtoul(argv[3], NULL, 0);
		meson_venc_bist_mode_set(tx_comm->hw_comm->enc_index,
			tx_comm->hw_comm->enc_type, venc_bist);
		DPTX_INFO("%s venc bist set done\n", __func__);
	} else if (strncmp(argv[2], "venc_disable", 12) == 0) {
		meson_venc_mode_disable(tx_comm->hw_comm->enc_index, tx_comm->hw_comm->enc_type);
		DPTX_INFO("%s venc disable done\n", __func__);
	}

	return CMD_RET_SUCCESS;
}

static int dptx_do_debug(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	struct dptx_common *tx_comm = NULL;
	int cnt = 0;
	int ret = 0;
	u32 idx = 0;
	u32 addr = 0;
	u32 len = 0;

	if (argc < 2)
		return cmd_usage(cmdtp);

	idx = simple_strtoul(argv[1], NULL, 0);
	tx_comm = get_dptx_device(idx & 0xFF);
	if (!tx_comm)
		return CMD_RET_FAILURE;

	if (strncmp(argv[2], "aux", 3) == 0) {
		if (argc < 4) {
			DPTX_ERROR("%s invalid aux cmd param\n", __func__);
			return CMD_RET_USAGE;
		}
		if (strncmp(argv[3], "dpcd", 4) == 0) {
			ret = dptx_aux_read_dpcd_caps(tx_comm->tx_aux,
				tx_comm->base.dpcd_buf, sizeof(tx_comm->base.dpcd_buf));
			if (ret < 0) {
				DPTX_INFO("read dpcd cap failed, ret%d\n", ret);
			} else {
				for (cnt = 0; cnt < ARRAY_SIZE(tx_comm->base.dpcd_buf); cnt++)
					DPTX_INFO("aux[%d] = 0x%x\n", cnt,
						tx_comm->base.dpcd_buf[cnt]);
			}
		} else if (strncmp(argv[3], "rd", 2) == 0) {
			if (argc < 5) {
				DPTX_ERROR("%s invalid aux addr\n", __func__);
				return CMD_RET_USAGE;
			}
			addr = simple_strtoul(argv[4], NULL, 16);
			if (argc < 6) {
				DPTX_ERROR("%s invalid aux read len\n", __func__);
				return CMD_RET_USAGE;
			}
			len = simple_strtoul(argv[5], NULL, 16);

			DPTX_INFO("aux read offset: 0x%x, size: 0x%x\n", addr, len);
			ret = dptx_aux_read_dpcd(tx_comm->tx_aux, addr,
				tx_comm->base.dpcd_buf, len);
			if (ret < 0) {
				DPTX_INFO("read dpcd cap failed, ret: %d\n", ret);
			} else {
				DPTX_INFO("read dpcd ret length: %d\n", ret);
				for (cnt = 0; cnt < ret; cnt++)
					DPTX_INFO("aux[%d] = 0x%x\n", cnt,
						tx_comm->base.dpcd_buf[cnt]);
			}
		}
	}

	return CMD_RET_SUCCESS;
}

/*
 * cmd usage: dp_tx get_parse_edid idx
 * for example: dp_tx get_parse_edid 0 for dptx0
 */
static int do_dptx_get_parse_edid(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	struct dptx_common *tx_comm = NULL;
	int ret = 0;
	int i;
	u32 idx = 0;

	if (argc < 2)
		return -1;

	DPTX_INFO("%s\n", __func__);

	idx = simple_strtoul(argv[1], NULL, 0);
	tx_comm = get_dptx_device(idx & 0xFF);
	if (!tx_comm)
		return -1;

	ret = dptx_aux_read_dpcd(tx_comm->tx_aux, DP_SINK_COUNT,
			tx_comm->link_sink_status,
			sizeof(tx_comm->link_sink_status));
	if (ret < 0)
		DPTX_ERROR("read dpcd link/sink status failed:%d\n", ret);

	ret = dptx_aux_read_dpcd_caps(tx_comm->tx_aux,
		tx_comm->base.dpcd_buf, sizeof(tx_comm->base.dpcd_buf));
	if (ret < 0)
		DPTX_INFO("read dpcd cap failed\n");

	for (i = 0; i < EDID_READ_MAX_RETRY; i++) {
		ret = dptx_aux_read_edid_data(tx_comm->tx_aux, tx_comm->base.edid_buf,
			sizeof(tx_comm->base.edid_buf));
		if (ret < 0)
			DPTX_INFO("read edid failed\n");
		else
			break;
	}

	meson_tx_edid_parse(&tx_comm->base.rxcap, tx_comm->base.edid_buf, tx_comm->base.edid_parse_mask);

	return 0;
}

/*
 * cmd usage: dp_tx hpd idx
 * for example: dp_tx hpd 0 for dptx0, dp_tx dptx_hpd 1 for dptx1
 */
int do_dptx_hpd_detect(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	struct dptx_common *tx_comm = NULL;
	u32 idx = 0;

	if (argc < 2)
		return 0;

	idx = simple_strtoul(argv[1], NULL, 0);
	tx_comm = get_dptx_device(idx & 0xFF);
	if (!tx_comm)
		return 0;

	/* TODO gpio and need pinmux */
	tx_comm->base.hpd_state = 1;
	DPTX_INFO("dptx hpd state = %d\n", tx_comm->base.hpd_state);
	return 1;
}

static cmd_tbl_t cmd_dptx_sub[] = {
	U_BOOT_CMD_MKENT(init, 2, 1, do_dptx_init, "", ""),
	U_BOOT_CMD_MKENT(hpd, 2, 1, do_dptx_hpd_detect, "", ""),
	U_BOOT_CMD_MKENT(output, 4, 1, dptx_do_output, "", ""),
	U_BOOT_CMD_MKENT(get_parse_edid, 2, 1, do_dptx_get_parse_edid, "", ""),
	U_BOOT_CMD_MKENT(debug, 6, 1, dptx_do_debug, "", ""),
};

static int do_dptx(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	cmd_tbl_t *c;

	if (argc < 2)
		return cmd_usage(cmdtp);

	argc--;
	argv++;

	c = find_cmd_tbl(argv[0], &cmd_dptx_sub[0], ARRAY_SIZE(cmd_dptx_sub));

	if (c)
		return  c->cmd(cmdtp, flag, argc, argv);
	else
		return cmd_usage(cmdtp);
}

U_BOOT_CMD(dp_tx, CONFIG_SYS_MAXARGS, 0, do_dptx,
	   "DPTX sub-system",
	"dptx version:20251210\n"
	"dp_tx hpd dp_core_idx\n"
	"    Detect dprx plug-in for dptx core of idx\n"
	"dp_tx get_parse_edid dp_core_idx\n"
	"    read & parse EDID of dprx connected to  dptx core of idx\n"
	"dptx output dp_core_idx cmd\n"
	"    set output SW/HW param\n"
);
