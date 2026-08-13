/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef _PWRC_H_
#define _PWRC_H_

#include <stdbool.h>

#define PWR_ON    1
#define PWR_OFF   0

#define PDID_DSPA       0
#define PDID_U3HSG_U3   1
#define PDID_DP         2
#define PDID_DOS_HCODEC 3
#define PDID_TAHOE      4
#define PDID_DOS_HEVC   5
#define PDID_U2H        6
#define PDID_U3DRD_B    7
#define PDID_VPU_HDMI   8
#define PDID_U2DRD      9
#define PDID_U3DRD_A    10
#define PDID_SD_EMMC_C  11
#define PDID_GE2D       12
#define PDID_AMFC       13
#define PDID_EDPTX      14
#define PDID_OPP        15
#define PDID_VICP       16
#define PDID_SD_EMMC_A  17
#define PDID_SD_EMMC_B  18
#define PDID_ETH        19
#define PDID_PCIE_A     20
#define PDID_PCIE_B     21
#define PDID_NNA_4T     22
#define PDID_HDMIRX     23
#define PDID_CVE        24
#define PDID_ISP        25
#define PDID_ETH_1G     26

#define PDID_U3HSG_HSG  27
#define PDID_U3DPPHY_U3 28
#define PDID_U3DPPHY_DP 29
#define PDID_PCIE3PHY   30
#define PDID_U3HSG_PCIE2 31
#define PDID_MALI_TOP    32

#define PDID_AO_SED     33
#define PDID_AO_IR      34
#define PDID_AO_UART_B  35
#define PDID_AO_UART_C  36
#define PDID_AO_UART_D  37
#define PDID_AO_SPISG   38
#define PDID_AO_UART_E  39
#define PDID_AO_CEC     40

#define PDID_EE_SRAMA   41
#define PDID_AUDIO      42
#define PDID_DMC0       43
#define PDID_GIC        44
#define PDID_DDRPHY     45
#define PDID_AUCPU      46
#define PDID_DSI0       47
#define PDID_DSI1       48
#define PDID_CAN0       49
#define PDID_CAN1       50

#define PM_MAX		51

#define PM_ETH		PM_MAX

unsigned long viu_init_psci_smc(unsigned long flag);
unsigned long pwr_ctrl_psci_smc(unsigned int power_domain, bool power_control);
unsigned long pwr_ctrl_status_psci_smc(unsigned int power_domain);
#endif
