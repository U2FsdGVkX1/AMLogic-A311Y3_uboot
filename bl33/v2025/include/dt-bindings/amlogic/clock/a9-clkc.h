/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2025 Amlogic, Inc. All rights reserved.
 */

#ifndef __DT_BINDINGS_CLOCK_A9_H
#define __DT_BINDINGS_CLOCK_A9_H

/*
 * FIXME: In CCF, the clock with clkid equal to 0 is marked as a non-valid clock,
 * so here the clkid starts with 1.
 */
#define CLKID_FIX_PLL				0
#define CLKID_FCLK_DIV2				1
#define CLKID_FCLK_DIV3				2
#define CLKID_FCLK_DIV4				3
#define CLKID_FCLK_DIV5				4
#define CLKID_FCLK_DIV7				5
#define CLKID_FCLK_DIV2P5			6
#define CLKID_SYS_CLK				7
#define CLKID_GP0_PLL				8
#define CLKID_HIFI_PLL				9
#define CLKID_SAR_ADC_SEL			10
#define CLKID_SAR_ADC_DIV			11
#define CLKID_SAR_ADC				12
#define CLKID_SPISG_0_SEL			13
#define CLKID_SPISG_0_DIV			14
#define CLKID_SPISG_0				15
#define CLKID_SPISG_1_SEL			16
#define CLKID_SPISG_1_DIV			17
#define CLKID_SPISG_1				18
#define CLKID_SPISG_2_SEL			19
#define CLKID_SPISG_2_DIV			20
#define CLKID_SPISG_2				21
#define CLKID_SD_EMMC_A_SEL			22
#define CLKID_SD_EMMC_A_DIV			23
#define CLKID_SD_EMMC_A				24
#define CLKID_SD_EMMC_B_SEL			25
#define CLKID_SD_EMMC_B_DIV			26
#define CLKID_SD_EMMC_B				27
#define CLKID_SD_EMMC_C_SEL			28
#define CLKID_SD_EMMC_C_DIV			29
#define CLKID_SD_EMMC_C				30
#define CLKID_ETH_RMII_DIV			31
#define CLKID_ETH_RMII				32
#define CLKID_TS_DIV				33
#define CLKID_TS				34
#define CLKID_AMFC_SEL				35
#define CLKID_AMFC_DIV				36
#define CLKID_AMFC				37
#define CLKID_VPU_0_SEL				38
#define CLKID_VPU_0_DIV				39
#define CLKID_VPU_0				40
#define CLKID_VPU_1_SEL				41
#define CLKID_VPU_1_DIV				42
#define CLKID_VPU_1				43
#define CLKID_VPU				44
#define CLKID_VAPB_0_SEL			45
#define CLKID_VAPB_0_DIV			46
#define CLKID_VAPB_0				47
#define CLKID_VAPB_1_SEL			48
#define CLKID_VAPB_1_DIV			49
#define CLKID_VAPB_1				50
#define CLKID_VAPB				51
#define CLKID_GE2D				52
#define CLKID_SYS_ETH_PHY			53
#define CLKID_SYS_ETH_AXI			54
#define CLKID_SYS_SD_EMMC_A			55
#define CLKID_SYS_SD_EMMC_B			56
#define CLKID_SYS_SD_EMMC_C			57
#define CLKID_SYS_ETH				58
#define CLKID_SYS_TS_A55			59
#define CLKID_SYS_TS_CORE			60
#define CLKID_SYS_TS_PLL			61
#define CLKID_SYS_GE2D				62
#define CLKID_SYS_SPISG_0			63
#define CLKID_SYS_VPU_INTR			64
#define CLKID_SYS_SAR_ADC			65
#define CLKID_I3C_SEL				66
#define CLKID_I3C_DIV				67
#define CLKID_I3C				68

#endif /* __A9_CLKC_H */
