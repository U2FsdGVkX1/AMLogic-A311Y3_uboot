/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef _PWRC_H_
#define _PWRC_H_

#include <stdbool.h>

#define PWR_ON    1
#define PWR_OFF   0

#define PDID_USB2		0
#define PDID_USB3  		1
#define PDID_DOS_HCODEC    	2
#define PDID_DOS_HEVC  		3
#define PDID_TAHOE 		4
#define PDID_VPU_VOUT  		5
#define PDID_SD_EMMC_A 		6
#define PDID_SD_EMMC_B 		7
#define PDID_SD_EMMC_C 		8
#define PDID_ETH   		9
#define PDID_NNA3  		10
#define PDID_NNA2  		11
#define PDID_AMFC  		12
#define PDID_ISP   		13
#define PDID_GE2D  		14
#define PDID_CVE   		15
#define PDID_DEWARP    		16
#define PDID_AI_ISP    		17
#define PDID_DSI   		18
#define PDID_PCIE  		19
#define PDID_U3P2_COMP 		20
#define PDID_AO_SED    		21
#define PDID_AO_ISP    		22
#define PDID_AO_SPI    		23
#define PDID_AO_UART_A 		24
#define PDID_AO_UART_B 		25
#define PDID_AO_SPISG  		26
#define PDID_AO_I2C_A  		27
#define PDID_AO_I2C_B  		28
#define PDID_AO_I3C    		29
#define PDID_AO_IR 		30
#define PDID_AO_PWM_A  		31
#define PDID_AO_PWM_B  		32
#define PDID_AO_PSRAM  		33
#define PDID_AUDIO_TOP 		34
#define PDID_CAN0 		35
#define PDID_CAN1 		36

#define PM_ETH			PDID_ETH

#define PM_MAX			37

unsigned long viu_init_psci_smc(void);
unsigned long pwr_ctrl_psci_smc(unsigned int power_domain, bool power_control);
unsigned long pwr_ctrl_status_psci_smc(unsigned int power_domain);
#endif
