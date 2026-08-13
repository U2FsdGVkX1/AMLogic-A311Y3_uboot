/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef _AMLOGIC_DisplayPort_TX_H
#define _AMLOGIC_DisplayPort_TX_H

// #include <common.h>
#include <linux/list.h>
// #include <dm.h>
#include <asm/gpio.h>
#include <linux/types.h>
#include <amlogic/media/vout/eDPTX/eDPTX_timing.h>

#define eDPTX_GPIO_NAME_MAX    12

//#define LCD_DEBUG_INFO
extern uint8_t dptx_print_level;
//bit[15:0]
#define LOG_E      0 // log level ERROR & Key
#define LOG_I      1 // log level Informative
#define LOG_V      2 // log level Verbose
#define LOG_A      3 // log level All

#define DPTXPR(idx, level, fmt, args...) \
	do { \
		if (!level) \
			printf("[eDP:%u](err) " fmt "\n", idx, ## args); \
		else if (dptx_print_level >= level) \
			printf("[eDP:%u] " fmt "\n", idx, ## args); \
	} while (0)

#define DPTX_ERR(dptx, fmt, ...)   pr_err("[eDP:%u](err) " fmt "\n", (dptx)->idx, ##__VA_ARGS__)
#define DPTX_PR(dptx, fmt, ...)           \
	do { \
		if (dptx_print_level >= LOG_I) \
			printf("[eDP:%u] " fmt "\n", (dptx)->idx, ##__VA_ARGS__); \
	} while (0)
#define DPTX_DBG(dptx, fmt, ...)          \
	do { \
		if (dptx_print_level >= LOG_V) \
			printf("[eDP:%u] " fmt "\n", (dptx)->idx, ##__VA_ARGS__); \
	} while (0)
#define DPTX_DBG2(dptx, fmt, ...)         \
	do { \
		if (dptx_print_level >= LOG_A) \
			printf("[eDP:%u] " fmt "\n", (dptx)->idx, ##__VA_ARGS__); \
	} while (0)
#define DPTX_P_ERR(dptx, port, fmt, ...)  \
			printf("[eDP:%u]-%u(err) " fmt "\n", (dptx)->idx, port, ##__VA_ARGS__)
#define DPTX_P_PR(dptx, port, fmt, ...)   \
	do { \
		if (dptx_print_level >= LOG_I) \
			printf("[eDP:%u]-%u: " fmt "\n", (dptx)->idx, port, ##__VA_ARGS__); \
	} while (0)
#define DPTX_P_DBG(dptx, port, fmt, ...)   \
	do { \
		if (dptx_print_level >= LOG_V) \
			printf("[eDP:%u]-%u: " fmt "\n", (dptx)->idx, port, ##__VA_ARGS__); \
	} while (0)
#define DPTX_P_DBG2(dptx, port, fmt, ...)   \
	do { \
		if (dptx_print_level >= LOG_A) \
			printf("[eDP:%u]-%u: " fmt "\n", (dptx)->idx, port, ##__VA_ARGS__); \
	} while (0)

#define PR_BUF_MAX              (4 * 1024)

#if defined(CONFIG_MESON_T7) || defined(CONFIG_MESON_T7C)
#define eDPTX_MAX_DRV              2
#define eDPTX_MAX_PORT             2
#define eDPTX_MAX_VOUT             3
#elif defined(CONFIG_MESON_A9)
#define eDPTX_MAX_DRV              1
#define eDPTX_MAX_PORT             1
#define eDPTX_MAX_VOUT             2
#else
#define eDPTX_MAX_DRV              1
#define eDPTX_MAX_PORT             1
#define eDPTX_MAX_VOUT             1
#endif


#define RGB_DELAY                   13
#define PRE_DE_DELAY                8

enum dptx_chip_e {
	eDPTX_CHIP_T7,
	eDPTX_CHIP_A9,
	eDPTX_CHIP_MAX,
};

struct edptx_chip_data_s {
	enum dptx_chip_e chip_type;
	const char *chip_name;
	uint8_t  drv_max;
	uint32_t offset_venc[eDPTX_MAX_DRV];     // 0x600
	uint32_t offset_venc_if[eDPTX_MAX_DRV];  // 0x500
	uint32_t offset_venc_data[eDPTX_MAX_DRV];// 0x100
	uint8_t venc_clk_msr_id[eDPTX_MAX_DRV];

	uint32_t link_rate[eDPTX_MAX_DRV][eDPTX_MAX_PORT];
	uint32_t lane_count[eDPTX_MAX_DRV][eDPTX_MAX_PORT];
	uint8_t  TPS_support;
	uint8_t  DACP_support;
	uint32_t pixel_clk_limit;
};

struct edptx_board_info_s {
	/* user define data */
	uint8_t status;
	char dev_name[16];

	char edptx_vcc_gpio_name[eDPTX_GPIO_NAME_MAX];
	uint16_t edptx_vcc;
	uint8_t edptx_vcc_on_dir;
	uint8_t edptx_vcc_off_dir;

	char edptx_hpd_gpio_name[eDPTX_GPIO_NAME_MAX];
	uint16_t edptx_hpd;
	uint8_t edptx_vcc_on_func;
	uint8_t edptx_vcc_off_func;

	char edptx_bl_en_gpio_name[eDPTX_GPIO_NAME_MAX];
	uint16_t edptx_bl_en;
	uint8_t edptx_bl_en_on_dir;
	uint8_t edptx_bl_en_off_dir;
	char edptx_bl_pwm_gpio_name[eDPTX_GPIO_NAME_MAX];
	uint16_t edptx_bl_pwm;
	uint8_t edptx_bl_pwm_on_dir;
	uint8_t edptx_bl_pwm_off_dir;

	uint8_t driver_strength_lut[10][3];

	uint8_t assigned_link_rate;
	uint8_t assigned_lane_count;
};

#define MOD_LEN_MAX        30

struct dptx_venc_cfg_s {
	uint8_t pll_flag;
	uint8_t clk_mode;
	uint8_t ppc;
	uint8_t clk_change; /* internal used */
	uint8_t ss_level;
	uint8_t ss_freq;
	uint8_t ss_mode;

	unsigned int enc_clk;
	unsigned long long bit_rate; /* Hz */

	unsigned int hstart;
	unsigned int hend;
	unsigned int vstart;
	unsigned int vend;
	uint8_t pre_de_h;
	uint8_t pre_de_v;

	unsigned short de_hs_addr;
	unsigned short de_he_addr;
	unsigned short de_vs_addr;
	unsigned short de_ve_addr;

	unsigned short hs_hs_addr;
	unsigned short hs_he_addr;
	unsigned short hs_vs_addr;
	unsigned short hs_ve_addr;

	unsigned short vs_hs_addr;
	unsigned short vs_he_addr;
	unsigned short vs_vs_addr;
	unsigned short vs_ve_addr;

	unsigned short pre_h_de_start;
	unsigned short pre_h_de_end;
	unsigned short pre_v_de_start;
	unsigned short pre_v_de_end;
	unsigned short pre_hso_start;
	unsigned short pre_hso_end;
	unsigned short pre_vso_hstart;
	unsigned short pre_vso_hend;
	unsigned short pre_vso_start;
	unsigned short pre_vso_end;
};

struct dptx_phy_cfg_s {
	uint32_t flag;
	uint32_t vswing;
	struct dptx_phy_lane_s {
		uint8_t status;
		uint8_t amp;
		uint8_t preem;
		uint8_t post_cur;
	} lane[4];
	// uint16_t level_to_phy_lut[10][3];
};

#define DPTX_DRV_TIMING_MAX  8
struct dptx_edid_info_s {
	uint8_t manufacturer_id[4];  //[8:9]2byte
	uint16_t product_id;         //[10:11]2byte
	uint32_t product_sn;         //[12:15]4byte
	uint8_t week;                //[16]1byte
	uint8_t year;                //[17]1byte + 1990
	uint16_t version;            //[18:19]2byte
	uint32_t established_timing; //[35:37]3byte
	// uint32_t standard_timing1;   //[38:45]4byte
	// uint32_t standard_timing2;   //[46:53]4byte
	uint16_t cfmt_support;       // each bit to CFMT

	char name[14];
	char serial_num[14];
	char asc_string[14];
	// unsigned int VIC_val[8];
	struct dptx_edid_range_limit_s {
		uint16_t vfreq[2];
		uint16_t hfreq[2];
		uint32_t pclk[2];
		uint16_t h_blank_min, v_blank_min;
	} range;

	uint8_t ext_flag;  //[126]1byte
	uint8_t did_version;

	uint8_t edid_crc;
};

struct dptx_link_cfg_s {
	/* internal used */
	// unsigned char coding_support;
	struct dptx_link_cap_s {
		uint8_t DPCD_ver;

		uint8_t max_lane_count;
		uint8_t max_link_rate;
		// uint8_t max_link_rate_UHBR; // [0]=10G [1]=20G [2]=13.5G

		// [0]=TPS3 [1]=TPS4 [4]=enh_frame [5]=ss [6]=no-train [7]=POST_LT_ADJ_REQ
		uint8_t link_cap;
		uint8_t DACP_support;      // [0]=M1(HDCP) [1]=M2(Panel Limit) [2]=M3a(eDP ASSR)
		uint8_t train_rd_intv;
		// uint8_t NORP;              //Number of Receiver Ports
		// uint8_t dev_type;          // 0=DP; 1=eDP
		uint8_t DPCD_reg_func;     // [0]=ext-DPCD [1]=eDP-PCD //[2]=DPCD_BL [3]=DPCD_TOUCH
		uint8_t coding_cap;        // [0]=8b/10b [0]=128b/132b
		uint8_t msa_ignore;        // [0]=msa [1]=adpt-sync_SDP
		// uint8_t training_mode;
		uint8_t sync_clk_mode;

		struct eDP_cap_s {
			uint8_t ver;
			uint8_t bl_ctrl_cap; // [0]=TCON_bl [1]=pin_en [2]=aux_en [3]=lm_cap [4:5]=bl_reg
			uint8_t bl_adj_cap;  // as 00702h
				// [0]=bl_pwm [1]=bl_aux [2]=bl-2byte [3]=aux-pwm_combo
				// [4]=bl_pwm_freq_pass-thru [5]=aux_pwm_freq [6]=dynamic_bl [7]=bl-vsync
			uint8_t bl_rgn_x;
			uint8_t bl_rgn_y;
			uint8_t od_cap;    // [0]=od_cap [1]=od_aux
			uint8_t test_cap;  // [0]=pin [1]=aux
			uint8_t ctrl_cap;  // [0]=frc [1]=color_eng [2]=set_power

			uint8_t PSR_cap;   // as 00702h
				// 0=n 1=PSR1 2=1+PSR2(SU) 3=2+PSR2(Y-coor) 4=3+PSR2(early_transport)
			uint8_t PSR_setup_time; // as 00071h
			uint8_t PSR_req; // [0]=LT_need_when_1_exit
				// [1]Y-coor need on SU [2]=Update Granularity [3]=no_fr_sync on SU
			uint16_t PSR2_SU_X_GRANULARITY;
			uint8_t PSR2_SU_Y_GRANULARITY;
		} eDP;
		struct DSC_cap_s {
			uint8_t cap;            // as 00060h
			uint8_t algm_ver;       // as 00061h
			uint8_t buf_block_size; // as 00062h
			uint8_t slice_cap1;     // as 00064h
			uint8_t buf_bit_depth;  // as 00065h
			uint8_t feature_spt;    // as 00066h
			uint8_t max_bpp_0;      // as 00067h
			uint8_t max_bpp_1;      // as 00068h
			uint8_t color_fmt_cap;  // as 00069h
			uint8_t color_dep_cap;  // as 0006ah
			uint8_t peak_throughput;// as 0006bh
			uint8_t max_slice_width;// as 0006ch
			uint8_t slice_cap2;     // as 0006dh
			uint8_t bpp_delta_increment0;// as 0006eh
			uint8_t bpp_delta_increment1;// as 0006fh
		} dsc;
	} cap;

	uint8_t preset_ds[4];

	uint8_t HPD_level;
	uint8_t irq_sta;

	uint8_t link_rate;
	uint8_t lane_count;
	uint8_t link_rate_update;
	uint8_t ssc_en;
	uint8_t enh_frame_en;

	uint8_t phy_update;
	uint8_t curr_ds[4];
	uint8_t adj_req_ds[4];
};

#define LCD_CLK_SS_BIT_FREQ             0
#define LCD_CLK_SS_BIT_MODE             4

#define DPTX_GPIO_MAX                    0xff
#define DPTX_GPIO_OUTPUT_LOW             0
#define DPTX_GPIO_OUTPUT_HIGH            1
#define DPTX_GPIO_INPUT                  2

#define DPTX_VENC_1PPC          1
#define DPTX_VENC_2PPC          2
#define DPTX_VENC_4PPC          4

#define LCD_PLL_MODE_DEFAULT         BIT(0)
#define LCD_PLL_MODE_SPECIAL_CNTL    BIT(1)
#define LCD_PLL_MODE_FRAC_SHIFT      BIT(2)


struct dptx_pll_data_s {
	/* clk path node parameters */
	u8 pll_od_fb;
	u32 fin_base;
	// u16 pll_m_range[2];
	u32 pll_frac_range;
	u8  pll_frac_sign_bit;
	u64 pll_vco_range[2];
	u64 pll_out_range[2];
	u64 pll_div_in_fmax;
	u32 pll_div_out_fmax;
	u32 od_cnt;
	u32 div_sel_max;

	u32 xd_out_fmax;
	u16 xd_max;

	// u8 vclk_sel;
	// u16 venc_clk_msr_id;

	u16 ss_level_max;
	u16 ss_freq_max;
	u16 ss_mode_max;
	u16 ss_dep_base;
	u16 ss_dep_sel_max;
	u16 ss_str_m_max;

	uint8_t pll_0_5_div_en;
};

struct dptx_clk_cfg_s {
	u64 fin;
	u64 fout;
	uint8_t clk_src; // 0=PLL, 1=FIX_PLL, 2=LINK_CLK
	// void *pll_data;
	struct dptx_pll_data_s  *pll_data;


	/* pll parameters */
	unsigned int pll_id;
	unsigned int pll_offset;
	unsigned int pll_od_fb;
	uint16_t pll_m;
	uint16_t pll_n;
	u64 pll_fvco;
	uint8_t pll_od_sel[3];
	uint8_t pll_tcon_div_sel;

	unsigned int pll_frac;
	unsigned int pll_frac_half_shift;
	unsigned long long pll_fout;
	unsigned int pll_div_fout;

	unsigned int ss_level;
	unsigned int ss_dep_sel;
	unsigned int ss_str_m;
	unsigned int ss_ppm;
	unsigned int ss_freq;
	unsigned int ss_mode;
	unsigned int ss_en;

	uint8_t pll_clk_div_sel;
	uint8_t div0;
	uint8_t div1;
	uint8_t xd;
	uint8_t vclk_sel;

	uint8_t pll_mode;

	// unsigned int err_fmin;
	// unsigned int done;
};

#define DPTX_DRV_VMODE_MAX   64
#define VMODE_FLAG_VALID     BIT(0)
#define VMODE_FLAG_FR_RANGE  BIT(1)
#define VMODE_FLAG_PREFERRED BIT(2)
// #define VMODE_FLAG_BW_ENOUGH BIT(3)
struct dptx_vmode_mgr_s {
	struct dptx_vmode_s {
		uint32_t fr100_int;
		uint8_t fr_adv; // 0=fr_int; 1=fr_frac; 0xff=raw fr
		uint8_t base_dtd_idx;
		uint8_t flag;
		uint16_t cfmt_support;
	} vmodes[DPTX_DRV_VMODE_MAX];
	uint8_t vmode_sel_idx;
	uint8_t vmode_cfmt_sel;
};

#define DPTX_STA_PROBE_DONE   BIT(0)
#define DPTX_STA_DRV_READY    BIT(1)
#define DPTX_STA_HPD_HIGH     BIT(2)
#define DPTX_STA_LINK_ON      BIT(3)
#define DPTX_STA_DISP_ON      BIT(4)

#define eDPTX_PHY_CH_LANE_MAX 5

struct edptx_panel_data_s {
	char sink_name[32];
	uint16_t size[2];
	uint8_t timing_cnt;
	struct dptx_detail_timing_s timing[DPTX_DRV_TIMING_MAX];
	struct signal_eDP_cfg_s {
		uint8_t port_count;  //dft: 1
		bool link_from_DPCD;
		uint8_t link_rate;  //dft: 4
		uint8_t lane_count; //dft: 0xfff

		uint8_t training_mode; // 0=auto, 1=fast, 2=full

		// bool timing_from_EDID;
		bool cap_info_from_EDID;

		uint8_t  hpd_ignore;
		uint16_t power_delay;
		uint16_t bl_delay;
		uint16_t power_reset_delay;
	} eDP;

	// struct backlight_cfg_s backlight;

	uint8_t ch_ctrl_use_default;
	struct phy_ch_ctrl_s {
		uint8_t en;
		uint8_t sel;
		uint8_t pn_swap;
		uint8_t phase_sel;
	} ch_ctrl[eDPTX_PHY_CH_LANE_MAX];
};

struct dptx_drv_s {
	uint8_t idx;
	uint8_t status;
	uint8_t viu_sel; // 1=vout; 2=vou2; 3=vout3

	struct edptx_chip_data_s *data;
	struct edptx_board_info_s board_data;
	struct edptx_panel_data_s panel_data;

	struct dptx_user_set_s {
		char dev_name[32];
		uint8_t user_port_mask;  //dft: 0
		uint8_t dual_port_type;  //dft: 0
		uint8_t user_link_rate;  //dft: 0
		uint8_t user_lane_count; //dft: 0
		uint8_t user_panel_config;
		uint8_t user_color_format;
		uint16_t user_hpd_ignore;
		uint16_t user_vmode_sel;
		uint16_t user_disable_PSR;
	} setting;

	struct dptx_phy_cfg_s phy_cfg;  // !move to link
	struct dptx_clk_cfg_s vid_clk;
	struct dptx_clk_cfg_s link_clk; // !move to link

	struct dptx_venc_cfg_s venc_cfg;

	struct dptx_vmode_mgr_s vmode_mgr;
	struct dptx_detail_timing_s act_timing;

	struct dptx_sink_dev_s {
		uint8_t port_mask;
		uint8_t dual_port_type;
		uint8_t user_port_count;

		uint8_t hpd_mask;
		// uint8_t pwr_gpio_mask;
		// uint8_t dt_cnt;
		// struct dptx_detail_timing_s timing[8];

		struct dptx_edid_info_s edid;
		struct dptx_link_cfg_s link[eDPTX_MAX_PORT];
	} sink;

	struct dev_pm_ops *dev_pm_ops;
};

#endif
