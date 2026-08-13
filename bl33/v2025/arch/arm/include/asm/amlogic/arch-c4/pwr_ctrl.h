/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef _PWRC_H_
#define _PWRC_H_

#include <stdbool.h>

#define PWR_ON    1
#define PWR_OFF   0

#define	PDID_NNA	0
#define	PDID_U2DRD	1
#define	PDID_AMFC	2
#define	PDID_SDIOA	3
#define	PDID_SDCARD	4
#define	PDID_EMMC	5
#define	PDID_ETH	6
#define	PDID_MIPI_ISP	7
#define	PDID_GE2D	8
#define	PDID_CVE	9
#define	PDID_OPP	10
#define	PDID_TAHOE	11
#define	PDID_HCODEC	12
#define	PDID_AO_PWM_J	13
#define	PDID_AO_PWM_K	14
#define	PDID_AO_SED	15
#define	PDID_AUDIO_TOP	16
#define PDID_DMC    	17
#define PDID_AXI_SRAM   18
#define PDID_DDRPHY 	19

#define PM_ETH			PDID_ETH

#define PM_MAX		20

unsigned long pwr_ctrl_psci_smc(unsigned int power_domain, bool power_control);
unsigned long pwr_ctrl_status_psci_smc(unsigned int power_domain);
#endif
