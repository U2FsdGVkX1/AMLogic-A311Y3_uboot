// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */
#include <asm/amlogic/arch-a9/register.h>
#include <dm.h>
#include <dt-bindings/amlogic/clock/a9-clkc.h>
#include <amlogic/clk-core.h>

static const struct pll_params_table a9_gp0_pll_table[] = {
	PLL_RATE_PARAMS(1536000000, 128, 0, 0),
	PLL_RATE_PARAMS(1152000000, 192, 0, 1)
};

static const struct reg_sequence a9_gp0_init_regs[] = {
	{ .reg = ANACTRL_GP0PLL_CTRL0, .def = 0x08010000 },
	{ .reg = ANACTRL_GP0PLL_CTRL1, .def = 0x11480000 },
	{ .reg = ANACTRL_GP0PLL_CTRL2, .def = 0x1219b010 },
	{ .reg = ANACTRL_GP0PLL_CTRL3, .def = 0x00008010 }
};

static const struct pll_params_table a9_hifi_pll_table[] = {
	PLL_RATE_PARAMS_FRAC(491520000, 163, 0, 2, 84000)
};

static const struct reg_sequence a9_hifi_init_regs[] = {
	{ .reg = ANACTRL_HIFI0PLL_CTRL0, .def = 0x08010000 },
	{ .reg = ANACTRL_HIFI0PLL_CTRL1, .def = 0x11480000 },
	{ .reg = ANACTRL_HIFI0PLL_CTRL2, .def = 0x1219b010 },
	{ .reg = ANACTRL_HIFI0PLL_CTRL3, .def = 0x00008010 }
};

static const char * const a9_sar_adc_parents[] = {
	"xtal",
	"sys_clk"
};

static const char * const a9_spisg_parents[] = {
	"xtal",
	"sys_clk",
	"fclk_div4",
	"fclk_div3",
	"fclk_div2",
	"fclk_div5",
	"fclk_div7",
	"gp0_pll"
};

static u32 sd_emmc_parent_table[] = { 0, 1, 2, 3, 4, 6, 7};

static const char * const a9_sd_emmc_parents[] = {
	"xtal",
	"fclk_div2",
	"fclk_div3",
	"hifi_pll",
	"fclk_div2p5",
	"gp1_pll",  /* NOTE: Currently not supported. */
	"gp0_pll"
};

static const char * const a9_amfc_parents[] = {
	"xtal",
	"sys_clk",
	"fclk_div2",
	"fclk_div2p5",
	"fclk_div3",
	"fclk_div4",
	"fclk_div5",
	"fclk_div7"
};

static const char * const a9_vpu_pre_parents[] = {
	"fclk_div3",
	"fclk_div4",
	"fclk_div5",
	"vid1_pll",  /* NOTE: Currently not supported. */
	"fclk_div2",
	"vid_pll",  /* NOTE: Currently not supported. */
	"vid2_pll",  /* NOTE: Currently not supported. */
	"gp1_pll"  /* NOTE: Currently not supported. */
};

static const char * const a9_vpu_parents[] = {
	"vpu_0",
	"vpu_1"
};

static const char * const a9_vapb_pre_parents[] = {
	"fclk_div4",
	"fclk_div3",
	"fclk_div5",
	"fclk_div7",
	"fclk_div2",
	"vid_pll",  /* NOTE: Currently not supported. */
	"hifi_pll",
	"fclk_div2p5"  /* NOTE: Currently not supported. */
};

static const char * const a9_vapb_parents[] = {
	"vapb_0",
	"vapb_1"
};

static const char * const a9_i3c_parents[] = {
	"sys_clk",
	"xtal",
	"fclk_div5"
};

struct aml_clk_data a9_clks[] = {
	CLK_PLL(CLKID_FIX_PLL, "fix_pll", /* clkid, name */
		NULL, 0, /* initregs, num_initregs */
		NULL, 0, /* table, num_table */
		ANACTRL_FIXPLL_CTRL0, 28, 1, /* en */
		ANACTRL_FIXPLL_CTRL0, 29, 1, /* rst */
		ANACTRL_FIXPLL_CTRL0, 30, 1, /* l_detect_en */
		ANACTRL_FIXPLL_CTRL0, 31, 1, /* l */
		ANACTRL_FIXPLL_CTRL0, 0, 9, /* m */
		ANACTRL_FIXPLL_CTRL0, 12, 3, /* n */
		ANACTRL_FIXPLL_CTRL1, 0, 17, /* frac */
		ANACTRL_FIXPLL_CTRL0, 20, 3, /* od */
		CLK_AML_PLL_POWER_OF_TWO | CLK_AML_PLL_M_EN0P5 |
		CLK_AML_PLL_RSTN | CLK_AML_PLL_READ_ONLY, /* pll_flags */
		"xtal", /* pname */
		0 /* flag */),
	CLK_FIXED_FACTOR(CLKID_FCLK_DIV2, "fclk_div2", 1, 2, "fix_pll"),
	CLK_FIXED_FACTOR(CLKID_FCLK_DIV3, "fclk_div3", 1, 3, "fix_pll"),
	CLK_FIXED_FACTOR(CLKID_FCLK_DIV4, "fclk_div4", 1, 4, "fix_pll"),
	CLK_FIXED_FACTOR(CLKID_FCLK_DIV5, "fclk_div5", 1, 5, "fix_pll"),
	CLK_FIXED_FACTOR(CLKID_FCLK_DIV7, "fclk_div7", 1, 7, "fix_pll"),
	CLK_FIXED_FACTOR(CLKID_FCLK_DIV2P5, "fclk_div2p5", 2, 5, "fix_pll"),
	CLK_FIXED_RATE(CLKID_SYS_CLK, "sys_clk", 166666000),
	CLK_PLL(CLKID_GP0_PLL, "gp0_pll", /* clkid, name */
		a9_gp0_init_regs, ARRAY_SIZE(a9_gp0_init_regs), /* initregs, num_initregs */
		a9_gp0_pll_table, ARRAY_SIZE(a9_gp0_pll_table), /* table, num_table */
		ANACTRL_GP0PLL_CTRL0, 28, 1, /* en */
		ANACTRL_GP0PLL_CTRL0, 29, 1, /* rst */
		ANACTRL_GP0PLL_CTRL0, 30, 1, /* l_detect_en */
		ANACTRL_GP0PLL_CTRL0, 31, 1, /* l */
		ANACTRL_GP0PLL_CTRL0, 0, 9, /* m */
		ANACTRL_GP0PLL_CTRL0, 12, 3, /* n */
		ANACTRL_GP0PLL_CTRL1, 0, 17, /* frac */
		ANACTRL_GP0PLL_CTRL0, 20, 3, /* od */
		CLK_AML_PLL_POWER_OF_TWO | CLK_AML_PLL_M_EN0P5 |
		CLK_AML_PLL_RSTN, /* pll_flags */
		"xtal", /* pname */
		0 /* flag */),
	CLK_PLL(CLKID_HIFI_PLL, "hifi_pll", /* clkid, name */
		a9_hifi_init_regs, ARRAY_SIZE(a9_hifi_init_regs), /* initregs, num_initregs */
		a9_hifi_pll_table, ARRAY_SIZE(a9_hifi_pll_table), /* table, num_table */
		ANACTRL_HIFI0PLL_CTRL0, 28, 1, /* en */
		ANACTRL_HIFI0PLL_CTRL0, 29, 1, /* rst */
		ANACTRL_HIFI0PLL_CTRL0, 30, 1, /* l_detect_en */
		ANACTRL_HIFI0PLL_CTRL0, 31, 1, /* l */
		ANACTRL_HIFI0PLL_CTRL0, 0, 9, /* m */
		ANACTRL_HIFI0PLL_CTRL0, 12, 3, /* n */
		ANACTRL_HIFI0PLL_CTRL1, 0, 17, /* frac */
		ANACTRL_HIFI0PLL_CTRL0, 20, 3, /* od */
		CLK_AML_PLL_POWER_OF_TWO | CLK_AML_PLL_M_EN0P5 |
		CLK_AML_PLL_RSTN | CLK_AML_PLL_FIXED_FRAC_WEIGHT_PRECISION, /* pll_flags */
		"xtal", /* pname */
		0 /* flag */),
	CLK_MUX(CLKID_SAR_ADC_SEL, "sar_adc_sel", /* clkid, name */
		CLKCTRL_SAR_CLK_CTRL0, 0x3, 9, /* reg, mask, shift */
		a9_sar_adc_parents, ARRAY_SIZE(a9_sar_adc_parents), /* pnames, num_pnames */
		NULL, 0 /* table, flag */),
	CLK_DIVIDER(CLKID_SAR_ADC_DIV, "sar_adc_div", /* clkid, name */
		    CLKCTRL_SAR_CLK_CTRL0, 0, 8, /* reg, mask, shift */
		    0, /* dflags */
		    NULL, /* table */
		    "sar_adc_sel", /* pname */
		    0 /* _flags */),
	CLK_GATE(CLKID_SAR_ADC, "sar_adc", /* clkid, name */
		 CLKCTRL_SAR_CLK_CTRL0, 8, /* reg, mask, shift */
		 "sar_adc_div", /* pname */
		 0 /* flags */),
	CLK_MUX(CLKID_SPISG_0_SEL, "spisg_0_sel", /* clkid, name */
		CLKCTRL_SPISG_CLK_CTRL, 0x7, 9, /* reg, mask, shift */
		a9_spisg_parents, ARRAY_SIZE(a9_spisg_parents), /* pnames, num_pnames */
		NULL, 0 /* table, flag */),
	CLK_DIVIDER(CLKID_SPISG_0_DIV, "spisg_0_div", /* clkid, name */
		    CLKCTRL_SPISG_CLK_CTRL, 0, 6, /* reg, mask, shift */
		    0, /* dflags */
		    NULL, /* table */
		    "spisg_0_sel", /* pname */
		    0 /* _flags */),
	CLK_GATE(CLKID_SPISG_0, "spisg_0", /* clkid, name */
		 CLKCTRL_SPISG_CLK_CTRL, 8, /* reg, mask, shift */
		 "spisg_0_div", /* pname */
		 0 /* flags */),
	CLK_MUX(CLKID_SPISG_1_SEL, "spisg_1_sel", /* clkid, name */
		CLKCTRL_SPISG_CLK_CTRL, 0x7, 25, /* reg, mask, shift */
		a9_spisg_parents, ARRAY_SIZE(a9_spisg_parents), /* pnames, num_pnames */
		NULL, 0 /* table, flag */),
	CLK_DIVIDER(CLKID_SPISG_1_DIV, "spisg_1_div", /* clkid, name */
		    CLKCTRL_SPISG_CLK_CTRL, 16, 6, /* reg, mask, shift */
		    0, /* dflags */
		    NULL, /* table */
		    "spisg_1_sel", /* pname */
		    0 /* _flags */),
	CLK_GATE(CLKID_SPISG_1, "spisg_1", /* clkid, name */
		 CLKCTRL_SPISG_CLK_CTRL, 24, /* reg, mask, shift */
		 "spisg_1_div", /* pname */
		 0 /* flags */),
	CLK_MUX(CLKID_SPISG_2_SEL, "spisg_2_sel", /* clkid, name */
		CLKCTRL_SPISG_CLK_CTRL1, 0x7, 9, /* reg, mask, shift */
		a9_spisg_parents, ARRAY_SIZE(a9_spisg_parents), /* pnames, num_pnames */
		NULL, 0 /* table, flag */),
	CLK_DIVIDER(CLKID_SPISG_2_DIV, "spisg_2_div", /* clkid, name */
		    CLKCTRL_SPISG_CLK_CTRL1, 0, 6, /* reg, mask, shift */
		    0, /* dflags */
		    NULL, /* table */
		    "spisg_2_sel", /* pname */
		    0 /* _flags */),
	CLK_GATE(CLKID_SPISG_2, "spisg_2", /* clkid, name */
		 CLKCTRL_SPISG_CLK_CTRL1, 8, /* reg, mask, shift */
		 "spisg_2_div", /* pname */
		 0 /* flags */),
	CLK_MUX(CLKID_SD_EMMC_A_SEL, "sd_emmc_a_sel", /* clkid, name */
		CLKCTRL_SD_EMMC_CLK_CTRL0, 0x7, 9, /* reg, mask, shift */
		a9_sd_emmc_parents, ARRAY_SIZE(a9_sd_emmc_parents), /* pnames, num_pnames */
		sd_emmc_parent_table, 0 /* table, flag */),
	CLK_DIVIDER(CLKID_SD_EMMC_A_DIV, "sd_emmc_a_div", /* clkid, name */
		    CLKCTRL_SD_EMMC_CLK_CTRL0, 0, 7, /* reg, mask, shift */
		    0, /* dflags */
		    NULL, /* table */
		    "sd_emmc_a_sel", /* pname */
		    0 /* _flags */),
	CLK_GATE(CLKID_SD_EMMC_A, "sd_emmc_a", /* clkid, name */
		 CLKCTRL_SD_EMMC_CLK_CTRL0, 8, /* reg, mask, shift */
		 "sd_emmc_a_div", /* pname */
		 0 /* flags */),
	CLK_MUX(CLKID_SD_EMMC_B_SEL, "sd_emmc_b_sel", /* clkid, name */
		CLKCTRL_SD_EMMC_CLK_CTRL0, 0x7, 25, /* reg, mask, shift */
		a9_sd_emmc_parents, ARRAY_SIZE(a9_sd_emmc_parents), /* pnames, num_pnames */
		sd_emmc_parent_table, 0 /* table, flag */),
	CLK_DIVIDER(CLKID_SD_EMMC_B_DIV, "sd_emmc_b_div", /* clkid, name */
		    CLKCTRL_SD_EMMC_CLK_CTRL0, 16, 7, /* reg, mask, shift */
		    0, /* dflags */
		    NULL, /* table */
		    "sd_emmc_b_sel", /* pname */
		    0 /* _flags */),
	CLK_GATE(CLKID_SD_EMMC_B, "sd_emmc_b", /* clkid, name */
		 CLKCTRL_SD_EMMC_CLK_CTRL0, 24, /* reg, mask, shift */
		 "sd_emmc_b_div", /* pname */
		 0 /* flags */),
	CLK_MUX(CLKID_SD_EMMC_C_SEL, "sd_emmc_c_sel", /* clkid, name */
		CLKCTRL_SD_EMMC_CLK_CTRL1, 0x7, 9, /* reg, mask, shift */
		a9_sd_emmc_parents, ARRAY_SIZE(a9_sd_emmc_parents), /* pnames, num_pnames */
		sd_emmc_parent_table, 0 /* table, flag */),
	CLK_DIVIDER(CLKID_SD_EMMC_C_DIV, "sd_emmc_c_div", /* clkid, name */
		    CLKCTRL_SD_EMMC_CLK_CTRL1, 0, 7, /* reg, mask, shift */
		    0, /* dflags */
		    NULL, /* table */
		    "sd_emmc_c_sel", /* pname */
		    0 /* _flags */),
	CLK_GATE(CLKID_SD_EMMC_C, "sd_emmc_c", /* clkid, name */
		 CLKCTRL_SD_EMMC_CLK_CTRL1, 8, /* reg, mask, shift */
		 "sd_emmc_c_div", /* pname */
		 0 /* flags */),
	CLK_DIVIDER(CLKID_ETH_RMII_DIV, "eth_rmii_div", /* clkid, name */
		    CLKCTRL_ETH_CLK_CTRL, 0, 7, /* reg, mask, shift */
		    0, /* dflags */
		    NULL, /* table */
		    "fclk_div2", /* pname */
		    0 /* _flags */),
	CLK_GATE(CLKID_ETH_RMII, "eth_rmii", /* clkid, name */
		 CLKCTRL_ETH_CLK_CTRL, 8, /* reg, mask, shift */
		 "eth_rmii_div", /* pname */
		 0 /* flags */),
	CLK_DIVIDER(CLKID_TS_DIV, "ts_div", /* clkid, name */
		    CLKCTRL_TS_CLK_CTRL, 0, 7, /* reg, mask, shift */
		    0, /* dflags */
		    NULL, /* table */
		    "xtal", /* pname */
		    0 /* _flags */),
	CLK_GATE(CLKID_TS, "ts", /* clkid, name */
		 CLKCTRL_TS_CLK_CTRL, 8, /* reg, mask, shift */
		 "ts_div", /* pname */
		 0 /* flags */),
	CLK_MUX(CLKID_AMFC_SEL, "amfc_sel", /* clkid, name */
		CLKCTRL_AMFC_CLK_CTRL, 0x7, 9, /* reg, mask, shift */
		a9_amfc_parents, ARRAY_SIZE(a9_amfc_parents), /* pnames, num_pnames */
		NULL, 0 /* table, flag */),
	CLK_DIVIDER(CLKID_AMFC_DIV, "amfc_div", /* clkid, name */
		    CLKCTRL_AMFC_CLK_CTRL, 0, 6, /* reg, mask, shift */
		    0, /* dflags */
		    NULL, /* table */
		    "amfc_sel", /* pname */
		    0 /* _flags */),
	CLK_GATE(CLKID_AMFC, "amfc", /* clkid, name */
		 CLKCTRL_AMFC_CLK_CTRL, 8, /* reg, mask, shift */
		 "amfc_div", /* pname */
		 0 /* flags */),
	CLK_MUX(CLKID_VPU_0_SEL, "vpu_0_sel", /* clkid, name */
		CLKCTRL_VPU_CLK_CTRL, 0x7, 9, /* reg, mask, shift */
		a9_vpu_pre_parents, ARRAY_SIZE(a9_vpu_pre_parents), /* pnames, num_pnames */
		NULL, 0 /* table, flag */),
	CLK_DIVIDER(CLKID_VPU_0_DIV, "vpu_0_div", /* clkid, name */
		    CLKCTRL_VPU_CLK_CTRL, 0, 7, /* reg, mask, shift */
		    0, /* dflags */
		    NULL, /* table */
		    "vpu_0_sel", /* pname */
		    0 /* _flags */),
	CLK_GATE(CLKID_VPU_0, "vpu_0", /* clkid, name */
		 CLKCTRL_VPU_CLK_CTRL, 8, /* reg, mask, shift */
		 "vpu_0_div", /* pname */
		 0 /* flags */),
	CLK_MUX(CLKID_VPU_1_SEL, "vpu_1_sel", /* clkid, name */
		CLKCTRL_VPU_CLK_CTRL, 0x7, 25, /* reg, mask, shift */
		a9_vpu_pre_parents, ARRAY_SIZE(a9_vpu_pre_parents), /* pnames, num_pnames */
		NULL, 0 /* table, flag */),
	CLK_DIVIDER(CLKID_VPU_1_DIV, "vpu_1_div", /* clkid, name */
		    CLKCTRL_VPU_CLK_CTRL, 16, 7, /* reg, mask, shift */
		    0, /* dflags */
		    NULL, /* table */
		    "vpu_1_sel", /* pname */
		    0 /* _flags */),
	CLK_GATE(CLKID_VPU_1, "vpu_1", /* clkid, name */
		 CLKCTRL_VPU_CLK_CTRL, 24, /* reg, mask, shift */
		 "vpu_1_div", /* pname */
		 0 /* flags */),
	CLK_MUX(CLKID_VPU, "vpu", /* clkid, name */
		CLKCTRL_VPU_CLK_CTRL, 0x1, 31, /* reg, mask, shift */
		a9_vpu_parents, ARRAY_SIZE(a9_vpu_parents), /* pnames, num_pnames */
		NULL, CLK_OPS_PARENT_ENABLE /* table, flag */),
	CLK_MUX(CLKID_VAPB_0_SEL, "vapb_0_sel", /* clkid, name */
		CLKCTRL_VAPBCLK_CTRL, 0x7, 9, /* reg, mask, shift */
		a9_vapb_pre_parents, ARRAY_SIZE(a9_vapb_pre_parents), /* pnames, num_pnames */
		NULL, 0 /* table, flag */),
	CLK_DIVIDER(CLKID_VAPB_0_DIV, "vapb_0_div", /* clkid, name */
		    CLKCTRL_VAPBCLK_CTRL, 0, 7, /* reg, mask, shift */
		    0, /* dflags */
		    NULL, /* table */
		    "vapb_0_sel", /* pname */
		    0 /* _flags */),
	CLK_GATE(CLKID_VAPB_0, "vapb_0", /* clkid, name */
		 CLKCTRL_VAPBCLK_CTRL, 8, /* reg, mask, shift */
		 "vapb_0_div", /* pname */
		 0 /* flags */),
	CLK_MUX(CLKID_VAPB_1_SEL, "vapb_1_sel", /* clkid, name */
		CLKCTRL_VAPBCLK_CTRL, 0x7, 25, /* reg, mask, shift */
		a9_vapb_pre_parents, ARRAY_SIZE(a9_vapb_pre_parents), /* pnames, num_pnames */
		NULL, 0 /* table, flag */),
	CLK_DIVIDER(CLKID_VAPB_1_DIV, "vapb_1_div", /* clkid, name */
		    CLKCTRL_VAPBCLK_CTRL, 16, 7, /* reg, mask, shift */
		    0, /* dflags */
		    NULL, /* table */
		    "vapb_1_sel", /* pname */
		    0 /* _flags */),
	CLK_GATE(CLKID_VAPB_1, "vapb_1", /* clkid, name */
		 CLKCTRL_VAPBCLK_CTRL, 24, /* reg, mask, shift */
		 "vapb_1_div", /* pname */
		 0 /* flags */),
	CLK_MUX(CLKID_VAPB, "vapb", /* clkid, name */
		CLKCTRL_VAPBCLK_CTRL, 0x1, 31, /* reg, mask, shift */
		a9_vapb_parents, ARRAY_SIZE(a9_vapb_parents), /* pnames, num_pnames */
		NULL, CLK_OPS_PARENT_ENABLE /* table, flag */),
	CLK_GATE(CLKID_GE2D, "ge2d", /* clkid, name */
		 CLKCTRL_VAPBCLK_CTRL, 30, /* reg, mask, shift */
		 "vapb", /* pname */
		 0 /* flags */),
	CLK_GATE(CLKID_SYS_ETH_PHY, "sys_eth_phy", /* clkid, name */
		 CLKCTRL_SYS_CLK_EN0_REG0, 4, /* reg, mask, shift */
		 "sys_clk", /* pname */
		 0 /* flags */),
	CLK_GATE(CLKID_SYS_ETH_AXI, "sys_eth_axi", /* clkid, name */
		 CLKCTRL_SYS_CLK_EN0_REG0, 8, /* reg, mask, shift */
		 "sys_clk", /* pname */
		 0 /* flags */),
	CLK_GATE(CLKID_SYS_SD_EMMC_A, "sys_sd_emmc_a", /* clkid, name */
		 CLKCTRL_SYS_CLK_EN0_REG0, 24, /* reg, mask, shift */
		 "sys_clk", /* pname */
		 0 /* flags */),
	CLK_GATE(CLKID_SYS_SD_EMMC_B, "sys_sd_emmc_b", /* clkid, name */
		 CLKCTRL_SYS_CLK_EN0_REG0, 25, /* reg, mask, shift */
		 "sys_clk", /* pname */
		 0 /* flags */),
	CLK_GATE(CLKID_SYS_SD_EMMC_C, "sys_sd_emmc_c", /* clkid, name */
		 CLKCTRL_SYS_CLK_EN0_REG0, 26, /* reg, mask, shift */
		 "sys_clk", /* pname */
		 0 /* flags */),
	CLK_GATE(CLKID_SYS_ETH, "sys_eth", /* clkid, name */
		 CLKCTRL_SYS_CLK_EN0_REG1, 3, /* reg, mask, shift */
		 "sys_clk", /* pname */
		 0 /* flags */),
	CLK_GATE(CLKID_SYS_TS_A55, "sys_ts_a55", /* clkid, name */
		 CLKCTRL_SYS_CLK_EN0_REG1, 11, /* reg, mask, shift */
		 "sys_clk", /* pname */
		 0 /* flags */),
	CLK_GATE(CLKID_SYS_TS_CORE, "sys_ts_core", /* clkid, name */
		 CLKCTRL_SYS_CLK_EN0_REG1, 15, /* reg, mask, shift */
		 "sys_clk", /* pname */
		 0 /* flags */),
	CLK_GATE(CLKID_SYS_TS_PLL, "sys_ts_pll", /* clkid, name */
		 CLKCTRL_SYS_CLK_EN0_REG1, 16, /* reg, mask, shift */
		 "sys_clk", /* pname */
		 0 /* flags */),
	CLK_GATE(CLKID_SYS_GE2D, "sys_ge2d", /* clkid, name */
		 CLKCTRL_SYS_CLK_EN0_REG1, 20, /* reg, mask, shift */
		 "sys_clk", /* pname */
		 0 /* flags */),
	CLK_GATE(CLKID_SYS_SPISG_0, "sys_spisg_0", /* clkid, name */
		 CLKCTRL_SYS_CLK_EN0_REG1, 21, /* reg, mask, shift */
		 "sys_clk", /* pname */
		 0 /* flags */),
	CLK_GATE(CLKID_SYS_VPU_INTR, "sys_vpu_intr", /* clkid, name */
		 CLKCTRL_SYS_CLK_EN0_REG2, 25, /* reg, mask, shift */
		 "sys_clk", /* pname */
		 0 /* flags */),
	CLK_GATE(CLKID_SYS_SAR_ADC, "sys_sar_adc", /* clkid, name */
		 CLKCTRL_SYS_CLK_EN0_REG2, 28, /* reg, mask, shift */
		 "sys_clk", /* pname */
		 0 /* flags */),
	CLK_MUX(CLKID_I3C_SEL, "i3c_sel", /* clkid, name */
		CLKCTRL_I3C_CLK_CTRL, 0x7, 9, /* reg, mask, shift */
		a9_i3c_parents, ARRAY_SIZE(a9_i3c_parents), /* pnames, num_pnames */
		NULL, 0 /* table, flag */),
	CLK_DIVIDER(CLKID_I3C_DIV, "i3c_div", /* clkid, name */
		    CLKCTRL_I3C_CLK_CTRL, 0, 8, /* reg, mask, shift */
		    0, /* dflags */
		    NULL, /* table */
		    "i3c_sel", /* pname */
		    0 /* _flags */),
	CLK_GATE(CLKID_I3C, "i3c", /* clkid, name */
		 CLKCTRL_I3C_CLK_CTRL, 8, /* reg, mask, shift */
		 "i3c_div", /* pname */
		 0 /* flags */),
};

static int aml_clk_probe(struct udevice *dev)
{
	return aml_clk_register_all(a9_clks, ARRAY_SIZE(a9_clks));
}

static const struct udevice_id aml_clk_ids[] = {
	{ .compatible = "amlogic,a9-clkc" },
	{ }
};

U_BOOT_DRIVER(aml_clk) = {
	.name		= "amlogic-clk-a9",
	.id		= UCLASS_CLK,
	.of_match	= aml_clk_ids,
	.ops		= &aml_clk_ops,
	.probe		= aml_clk_probe,
};
