/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef _AMLOGIC_DisplayPort_TX_H
#define _AMLOGIC_DisplayPort_TX_H

#include <amlogic/media/vout/eDPTX/eDPTX.h>

// struct aml_lcd_data_s *aml_lcd_get_data(void);
// struct aml_lcd_drv_s *aml_lcd_get_driver(int index);

// char *lcd_get_dt_addr(void);
int dptx_probe(void);

/* global api for cmd */
unsigned int edptx_driver_outputmode_check(uint8_t index, char *mode);
void edptx_driver_vmode_list(void);

void edptx_driver_probe(void);
uint8_t edptx_driver_prepare(uint8_t index, char *mode);
uint8_t edptx_driver_enable(uint8_t index, char *mode);
void edptx_driver_disable(uint8_t index);
void edptx_driver_info(uint8_t index);
void edptx_driver_reg_print(uint8_t index);
void edptx_driver_test(uint8_t index, uint8_t num);
void edptx_driver_list_vmode(uint8_t index);
void edptx_driver_set_vmode(uint8_t index, uint8_t num);
void edptx_driver_reset(uint8_t index, uint8_t num);
void edptx_driver_PSR1_en(uint8_t index, uint8_t num);
void edptx_driver_PSR2_en(uint8_t index, uint8_t num);
void edptx_driver_set_pattern(uint8_t index, uint8_t pattern, uint32_t d0, uint32_t d1, uint32_t d2);
void edptx_driver_set_link(uint8_t index, uint8_t lane, uint8_t rate);

// void edptx_driver_clk_info(uint8_t index);
// void edptx_driver_debug_print(uint8_t index, unsigned int val);
// void edptx_driver_reg_info(uint8_t index);

// void edptx_driver_list_support_mode(void);
// int aml_lcd_edp_debug(int index, char *str, int num);

// int aml_lcd_driver_prbs(int index, unsigned int s, unsigned int mode_flag);

// int aml_lcd_driver_suspend(void *pm_ops);
// int aml_lcd_driver_resume(void *pm_ops);
// int aml_lcd_driver_poweroff(void *pm_ops);
// void aml_lcd_set_poweron_suspend_sta(int state);

#endif /* INC_AML_LCD_VOUT_H */
