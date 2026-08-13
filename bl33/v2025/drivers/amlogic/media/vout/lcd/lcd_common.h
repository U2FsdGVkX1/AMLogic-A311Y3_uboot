/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef _AML_LCD_COMMON_H
#define _AML_LCD_COMMON_H

#include <div64.h>
#include <amlogic/aml_model.h>
#include <amlogic/media/vout/lcd/aml_lcd.h>
#include "./lcd_clk/lcd_clk_config.h"
#include "lcd_reg.h"
#ifdef CONFIG_AML_LCD_JSON
#include "lcd_parser/json_parse.h"
#endif
#ifdef CONFIG_CMD_AML_MODEL
#include "lcd_parser/lcd_ini.h"
#endif

/* 20241211: initial version */
/* 20250123: update lcd bootargs transfer by lrm */
/* 20250828: update preboot support */
/* 20250917: tcon support discontinued bin */
/* 20260203: optimize t6 phy & dphy init flow */
#define LCD_DRV_VERSION    "20260203"

#define CTYPE_MASK           0xf0
#define CTYPE_RGB            0x00
#define CTYPE_YUV422         0x10
#define CTYPE_YUV444         0x20
#define CTYPE_YUV420         0x30

#define CFMT_RGB565          0x05
#define CFMT_RGB_6bit        0x06
#define CFMT_RGB_8bit        0x08
#define CFMT_RGB_10bit       0x0a
#define CFMT_RGB_12bit       0x0c
#define CFMT_YCbCr422_8bit   0x18
#define CFMT_YCbCr422_10bit  0x1a
#define CFMT_YCbCr422_12bit  0x1c
#define CFMT_YCbCr444_8bit   0x28
#define CFMT_YCbCr444_10bit  0x2a
#define CFMT_YCbCr444_12bit  0x2c
#define CFMT_YCbCr420_8bit   0x38
#define CFMT_YCbCr420_10bit  0x3a
#define CFMT_YCbCr420_12bit  0x3c
#define CFMT_INVALID         0xff

struct color_fmt_info_s {
	unsigned int cfmt;
	unsigned char bits;
	char name[32];
};

void mdelay(unsigned long n);

static inline unsigned long long lcd_do_div(unsigned long long num, unsigned int den)
{
	unsigned long long ret = num;

	if (den == 0)
		return 0;

	do_div(ret, den);

	return ret;
}

static inline unsigned long long div_around(unsigned long long num, unsigned int den)
{
	unsigned long long ret = num + den / 2;

	if (den == 0)
		return 0;
	if (den == 1)
		return num;

	do_div(ret, den);

	return ret;
}

static inline unsigned long long lcd_diff(unsigned long long a, unsigned long long b)
{
	return (a >= b) ? (a - b) : (b - a);
}

static inline int lcd_s32_constraint(int v, int min, int max)
{
	return v > max ? max : v < min ? min : v;
}

static inline unsigned long long gcd(unsigned long long a, unsigned long long b)
{
	unsigned long long temp;

	while (b != 0) {
		temp = b;
		b = a % b;
		a = temp;
	}
	return a;
}

extern unsigned int lcd_prbs_freq, lcd_prbs_performed, lcd_prbs_err;

struct num_str_s {
	int  num;
	char str[32];
};

void lcd_display_init_test(struct aml_lcd_drv_s *pdrv);
void lcd_display_init_reg_dump(struct aml_lcd_drv_s *pdrv);

#define LCD_CMA_PAGE_SIZE_1K (1 * 1024)
#define LCD_CMA_PAGE_SIZE_2K (2 * 1024)
#define LCD_CMA_PAGE_SIZE_4K (4 * 1024)
#define LCD_CMA_PAGE_SIZE_8K (8 * 1024)

/* lcd common */
int strnum_get_num(const char *str, struct num_str_s *arr, int size_arr, int dft);
char *strnum_get_str(int num, struct num_str_s *arr, int size_arr, char *dft);
unsigned char *lcd_init_table_load_array(char *name, unsigned char cmd_size,
					 unsigned int *buf, int buf_len,
					 int tbl_max, int *tbl_cnt);

int lcd_base_config_load_from_dts(char *dt_addr, struct aml_lcd_drv_s *pdrv);
int lcd_base_config_load_from_bsp(struct aml_lcd_drv_s *pdrv);
void lcd_init_config_to_drv(struct aml_lcd_drv_s *pdrv);
int lcd_check_config_load(struct aml_lcd_drv_s *pdrv);
int lcd_get_panel_config(char *dt_addr, int load_id, struct aml_lcd_drv_s *pdrv);

unsigned int str_add_vmode(char *buf, struct lcd_vmode_info_s *vm_info, unsigned short framerate);
void lcd_cma_pool_init(struct aml_lcd_cma_mem *cma,
		phys_addr_t pa, unsigned long size, unsigned int page_size);
void *lcd_cma_pool_simple_alloc(struct aml_lcd_cma_mem *cma, unsigned long size);
void *lcd_alloc_dma_buffer(struct aml_lcd_drv_s *pdrv, unsigned long size);

int lcd_type_str_to_type(const char *str);
char *lcd_type_type_to_str(int type);
int lcd_mode_str_to_mode(const char *str);
char *lcd_mode_mode_to_str(int mode);
int lcd_get_dts_panel_node_ofst(unsigned char drv_idx);
unsigned char dtimg_info_add(char *c_buf, struct lcd_detail_timing_s *dtm, unsigned char c_bits);

void lcd_encl_on(struct aml_lcd_drv_s *pdrv);
unsigned int lcd_config_timing_check(struct aml_lcd_drv_s *pdrv,
				     struct lcd_detail_timing_s *ptiming);

void update_panel_param_to_kernel(void);
unsigned char lcd_get_dbg_source(void);
unsigned char lcd_panel_config_load_detect(int index, int dt_valid, const char *func_name);

void lcd_clk_frame_rate_init(struct lcd_detail_timing_s *ptiming);
void lcd_default_to_basic_timing_init_config(struct aml_lcd_drv_s *pdrv);
void lcd_enc_timing_init_config(struct aml_lcd_drv_s *pdrv);

int lcd_fr_is_frac(struct aml_lcd_drv_s *pdrv, unsigned int frame_rate);
int lcd_vmode_frac_is_support(struct aml_lcd_drv_s *pdrv, unsigned int frame_rate);
int lcd_frame_rate_change(struct aml_lcd_drv_s *pdrv);
void lcd_lvds_bit_rate_config(struct aml_lcd_drv_s *pdrv);

void lcd_rgb_pinmux_set(struct aml_lcd_drv_s *pdrv, int status);
void lcd_bt_pinmux_set(struct aml_lcd_drv_s *pdrv, int status);
void lcd_vbyone_pinmux_set(struct aml_lcd_drv_s *pdrv, int status);
void lcd_mlvds_pinmux_set(struct aml_lcd_drv_s *pdrv, int status);
void lcd_p2p_pinmux_set(struct aml_lcd_drv_s *pdrv, int status);
void lcd_mipi_pinmux_set(struct aml_lcd_drv_s *pdrv, int status);
void lcd_vbyone_bit_rate_config(struct aml_lcd_drv_s *pdrv);
void lcd_mlvds_bit_rate_config(struct aml_lcd_drv_s *pdrv);
void lcd_p2p_bit_rate_config(struct aml_lcd_drv_s *pdrv);
void lcd_mipi_dsi_bit_rate_config(struct aml_lcd_drv_s *pdrv);
struct lcd_detail_timing_s *lcd_timing_alloc(struct aml_lcd_drv_s *pdrv);
void lcd_timing_free_last(struct aml_lcd_drv_s *pdrv);
struct phy_attr_s *lcd_phy_alloc(struct aml_lcd_drv_s *pdrv);
void lcd_phy_free_last(struct aml_lcd_drv_s *pdrv);

void lcd_detail_timing_print(struct aml_lcd_drv_s *pdrv, struct lcd_detail_timing_s *dt);
void lcd_phy_cfg_print(struct phy_config_s *cfg);
void lcd_phy_attr_print(struct phy_attr_s *phy, u32 lane_num);
#ifdef CONFIG_AML_LCD_TCON
void lcd_tcon_global_reset(struct aml_lcd_drv_s *pdrv);
#endif

/* lcd cus_ctrl */
int lcd_cus_ctrl_load_from_dts(struct aml_lcd_drv_s *pdrv);
int lcd_cus_ctrl_load_from_ini(struct aml_lcd_drv_s *pdrv, void *inip, void *psec,
			       unsigned char version);

/* lcd model */
const char *get_lcd_config_load(unsigned char type);
int lcd_get_str_array_cnt(const char *data_str);
int lcd_trans_str_array(const char *data_str, unsigned int *data_buf, int cnt_max);
unsigned int lcd_get_str_array_index(const char *data_str, unsigned int index,
				     unsigned int def_val);

void mem_dump(unsigned char *addr, int size);
void panel_param_mem_dump(const char *key_name);
int is_panel_param_mem_ok(void);
void update_panel_param_to_kernel(void);
void lcd_panel_param_test(char *name);

int path_name_compose(const char *path, const char *name, char *path_name);
int set_panel_file_parse_mem(int index, void *parse_mem, int size, unsigned char type);
void *get_panel_file_parse_mem(int index);
void rm_panel_file_parse_mem(int index);
int panel_file_parse_mem_save(void);

int panel_param_mem_put(unsigned char *mem, const char *name, u32 len);
unsigned char *panel_param_mem_get(const char *name, u32 *len);
int panel_param_mem_modify(unsigned char *mem, const char *name, u32 len);

#ifdef CONFIG_AML_LCD_JSON
struct json_parse_s *get_panel_jsp(int index);
struct json_parse_s *panel_json_parse(int index);
int panel_json_mem_save(void *parse_mem, int index);
void panel_json_mem_free(void *parse_mem);
#endif

int lcd_ukey_get_size(const char *key_name);
unsigned char *lcd_ukey_get_tcon(const char *key_name, int *len);
int lcd_ukey_write(const char *key_name, unsigned char *buf, int len);

/* lcd venc */
void lcd_wait_vsync(struct aml_lcd_drv_s *pdrv);
unsigned int lcd_get_encl_line_cnt(struct aml_lcd_drv_s *pdrv);
unsigned int lcd_get_max_line_cnt(struct aml_lcd_drv_s *pdrv);
void lcd_debug_test(struct aml_lcd_drv_s *pdrv, unsigned int num);
void lcd_debug_window(struct aml_lcd_drv_s *pdrv, struct lcd_window_attr_s *window_attr);
void lcd_probe_cursor(struct aml_lcd_drv_s *pdrv, struct lcd_cursor_attr_s *cursor_attr);
void lcd_set_venc_timing(struct aml_lcd_drv_s *pdrv);
void lcd_set_venc(struct aml_lcd_drv_s *pdrv);
void lcd_venc_enable(struct aml_lcd_drv_s *pdrv, int flag);
void lcd_mute_set(struct aml_lcd_drv_s *pdrv,  unsigned char flag);
void lcd_on_off_mute_ctrl(struct aml_lcd_drv_s *pdrv,  unsigned char flag);
void lcd_venc_reg_print(struct aml_lcd_drv_s *pdrv);
void lcd_venc_save_bootctrl_to_regs(struct aml_lcd_drv_s *pdrv);
int lcd_venc_probe(struct aml_lcd_data_s *pdata);

/* lcd clk*/
struct lcd_clk_config_s *get_lcd_clk_config(struct aml_lcd_drv_s *pdrv);
void lcd_clk_config_print(struct aml_lcd_drv_s *pdrv);
void lcd_clk_reg_print(struct aml_lcd_drv_s *pdrv);
void lcd_get_ss(struct aml_lcd_drv_s *pdrv);
int lcd_set_ss(struct aml_lcd_drv_s *pdrv, unsigned int level,
	       unsigned int freq, unsigned int mode);
void lcd_update_clk_frac(struct aml_lcd_drv_s *pdrv);
void lcd_set_clk(struct aml_lcd_drv_s *pdrv);
void lcd_disable_clk(struct aml_lcd_drv_s *pdrv);
void lcd_clk_generate_parameter(struct aml_lcd_drv_s *pdrv);
void lcd_clk_config_probe(struct aml_lcd_drv_s *pdrv);
int aml_lcd_prbs_test(struct aml_lcd_drv_s *pdrv, unsigned int ms, unsigned int mode_flag);

/* lcd phy */
unsigned int lcd_phy_check_lane_phase_sel(struct aml_lcd_drv_s *pdrv);
int lcd_phy_param_preset(struct aml_lcd_drv_s *pdrv);
int lcd_phy_param_get(struct aml_lcd_drv_s *pdrv, struct phy_config_s *phy_cfg,
		      struct phy_attr_s *phy);
void lcd_phy_param_print(struct aml_lcd_drv_s *pdrv);
void lcd_phy_analog_reg_print(struct aml_lcd_drv_s *pdrv);
void lcd_phy_reset(struct aml_lcd_drv_s *pdrv);
void lcd_phy_set(struct aml_lcd_drv_s *pdrv, int status);
int lcd_phy_probe(struct aml_lcd_drv_s *pdrv);
int lcd_phy_config_init(struct aml_lcd_data_s *pdata);

/* lcd dphy */
void lcd_lane_map_preset(struct aml_lcd_drv_s *pdrv);
void lcd_lane_map_update(struct aml_lcd_drv_s *pdrv);
int lcd_lane_sel_get(struct aml_lcd_drv_s *pdrv, struct phy_config_s *phy_cfg);
void lcd_mipi_dphy_set(struct aml_lcd_drv_s *pdrv, unsigned char on_off);
void lcd_lvds_dphy_set(struct aml_lcd_drv_s *pdrv, unsigned char on_off);
void lcd_vbyone_dphy_set(struct aml_lcd_drv_s *pdrv, unsigned char on_off);
#ifdef CONFIG_AML_LCD_TCON
void lcd_mlvds_dphy_set(struct aml_lcd_drv_s *pdrv, unsigned char on_off);
void lcd_p2p_dphy_set(struct aml_lcd_drv_s *pdrv, unsigned char on_off);
#endif
void lcd_dphy_set_data(struct aml_lcd_drv_s *pdrv, int data);
void lcd_dphy_reg_print(struct aml_lcd_drv_s *pdrv);

void lcd_connector_driver_init_pre(struct aml_lcd_drv_s *pdrv);
void lcd_connector_driver_init(struct aml_lcd_drv_s *pdrv);
void lcd_connector_driver_disable(struct aml_lcd_drv_s *pdrv);

/* lcd pinctrl */
int lcd_gpio_name_map_num(const char *name);
int lcd_gpio_set(int gpio, int value);
unsigned int lcd_gpio_input_get(int gpio);
void lcd_pinmux_set(struct aml_lcd_drv_s *pdrv, int status);

/* lcd debug */
int lcd_debug_info_len(int num);
void lcd_info_print(struct aml_lcd_drv_s *pdrv);
void lcd_reg_print(struct aml_lcd_drv_s *pdrv);
void lcd_debug_probe(struct aml_lcd_drv_s *pdrv);

char *get_current_env_connector(unsigned char cnt_idx);
void sprintf_lcd_connector(char *buf, unsigned char lcd_idx, unsigned char lcd_type);

/* lcd driver */
int lcd_mode_tv_init(struct aml_lcd_drv_s *pdrv);
int lcd_mode_tablet_init(struct aml_lcd_drv_s *pdrv);

void lcd_wait_vsync(struct aml_lcd_drv_s *pdrv);

/* aml_bl driver */
#ifdef CONFIG_AML_LCD_BACKLIGHT
int aml_bl_reprobe(unsigned int index, char *dt_addr, unsigned char load_id);
void bl_driver_remove(unsigned char index);
int aml_bl_index_add(int drv_index, int conf_index);
int aml_bl_init(int index);
int aml_bl_get_state(int index);
void aml_bl_driver_enable(int index);
void aml_bl_driver_disable(int index);
void aml_bl_lcd_on_ctrl(int index);
void aml_bl_lcd_off_ctrl(int index);
void aml_bl_set_level(int index, int level);
int aml_bl_get_level(int index);
void aml_bl_config_print(int index);
int aml_bl_pwm_chip_init(struct aml_lcd_data_s *pdata);
#endif

#endif

