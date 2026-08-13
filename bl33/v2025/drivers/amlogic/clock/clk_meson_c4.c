// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2026 Amlogic, Inc. All rights reserved.
 */
#include <asm/amlogic/arch-c4/register.h>
#include <dm.h>
#include <dt-bindings/amlogic/clock/c4-clkc.h>
#include <amlogic/clk-core.h>

static const struct pll_params_table c4_gp0_pll_table[] = {
	PLL_RATE_PARAMS(1536000000, 256, 1, 1),
	PLL_RATE_PARAMS(1152000000, 192, 1, 1)
};

static const struct reg_sequence c4_gp0_init_regs[] = {
	{ .reg = ANACTRL_GP0PLL_CTRL1, .def = 0x03a00000 },
	{ .reg = ANACTRL_GP0PLL_CTRL2, .def = 0x00040000 },
	{ .reg = ANACTRL_GP0PLL_CTRL3, .def = 0x0f0da200 }
};

static const struct pll_params_table c4_hifi_pll_table[] = {
	PLL_RATE_PARAMS_FRAC(491520000, 163, 1, 2, 84000)
};

static const struct reg_sequence c4_hifi_init_regs[] = {
	{ .reg = ANACTRL_HIFIPLL_CTRL1, .def = 0x03a00000 },
	{ .reg = ANACTRL_HIFIPLL_CTRL2, .def = 0x00040000 },
	{ .reg = ANACTRL_HIFIPLL_CTRL3, .def = 0x0f0da200 }
};

static const char * const c4_sar_adc_parents[] = {
	"xtal",
	"sys_clk"
};

static const char * const c4_spicc_parents[] = {
	"xtal",
	"sys_clk",
	"fclk_div4",
	"fclk_div3",
	"fclk_div2",
	"fclk_div5",
	"fclk_div7",
	"fclk_div2p5"
};

static const char * const c4_sd_emmc_parents[] = {
	"xtal",
	"fclk_div2",
	"fclk_div3",
	"hifi_pll",
	"fclk_div2p5",
	"fclk_div4",
	"fclk_div5",
	"gp0_pll"
};

struct aml_clk_data c4_clks[] = {
	CLK_PLL(CLKID_FIX_PLL, "fix_pll", /* clkid, name */
		NULL, 0, /* initregs, num_initregs */
		NULL, 0, /* table, num_table */
		ANACTRL_FIXPLL_CTRL0, 28, 1, /* en */
		ANACTRL_FIXPLL_CTRL0, 29, 1, /* rst */
		ANACTRL_FIXPLL_CTRL2, 6, 1, /* l_detect_en */
		ANACTRL_FIXPLL_CTRL0, 31, 1, /* l */
		ANACTRL_FIXPLL_CTRL0, 0, 8, /* m */
		ANACTRL_FIXPLL_CTRL0, 16, 5, /* n */
		0, 0, 0, /* frac */
		ANACTRL_FIXPLL_CTRL0, 12, 3, /* od */
		CLK_AML_PLL_READ_ONLY, /* pll_flags */
		"xtal", /* pname */
		0 /* flag */),
	CLK_FIXED_FACTOR(CLKID_FCLK_DIV2, "fclk_div2", 1, 2, "fix_pll"),
	CLK_FIXED_FACTOR(CLKID_FCLK_DIV3, "fclk_div3", 1, 3, "fix_pll"),
	CLK_FIXED_FACTOR(CLKID_FCLK_DIV4, "fclk_div4", 1, 4, "fix_pll"),
	CLK_FIXED_FACTOR(CLKID_FCLK_DIV5, "fclk_div5", 1, 5, "fix_pll"),
	CLK_FIXED_FACTOR(CLKID_FCLK_DIV7, "fclk_div7", 1, 7, "fix_pll"),
	CLK_FIXED_FACTOR(CLKID_FCLK_DIV2P5, "fclk_div2p5", 2, 5, "fix_pll"),
	CLK_FIXED_RATE(CLKID_SYS_CLK, "sys_clk", 100000000),
	CLK_PLL(CLKID_GP0_PLL, "gp0_pll", /* clkid, name */
		c4_gp0_init_regs, ARRAY_SIZE(c4_gp0_init_regs), /* initregs, num_initregs */
		c4_gp0_pll_table, ARRAY_SIZE(c4_gp0_pll_table), /* table, num_table */
		ANACTRL_GP0PLL_CTRL0, 28, 1, /* en */
		ANACTRL_GP0PLL_CTRL0, 29, 1, /* rst */
		ANACTRL_GP0PLL_CTRL3, 29, 1, /* l_detect_en */
		ANACTRL_GP0PLL_CTRL0, 31, 1, /* l */
		ANACTRL_GP0PLL_CTRL0, 0, 9, /* m */
		ANACTRL_GP0PLL_CTRL0, 16, 5, /* n */
		ANACTRL_GP0PLL_CTRL1, 0, 17, /* frac */
		ANACTRL_GP0PLL_CTRL0, 10, 3, /* od */
		CLK_AML_PLL_M_EN0P5 |
		CLK_AML_PLL_L_DETECTN, /* pll_flags */
		"xtal", /* pname */
		0 /* flag */),
	CLK_PLL(CLKID_HIFI_PLL, "hifi_pll", /* clkid, name */
		c4_hifi_init_regs, ARRAY_SIZE(c4_hifi_init_regs), /* initregs, num_initregs */
		c4_hifi_pll_table, ARRAY_SIZE(c4_hifi_pll_table), /* table, num_table */
		ANACTRL_HIFIPLL_CTRL0, 28, 1, /* en */
		ANACTRL_HIFIPLL_CTRL0, 29, 1, /* rst */
		ANACTRL_HIFIPLL_CTRL3, 29, 1, /* l_detect_en */
		ANACTRL_HIFIPLL_CTRL0, 31, 1, /* l */
		ANACTRL_HIFIPLL_CTRL0, 0, 9, /* m */
		ANACTRL_HIFIPLL_CTRL0, 16, 5, /* n */
		ANACTRL_HIFIPLL_CTRL1, 0, 17, /* frac */
		ANACTRL_HIFIPLL_CTRL0, 10, 3, /* od */
		CLK_AML_PLL_M_EN0P5 | CLK_AML_PLL_L_DETECTN |
		CLK_AML_PLL_FIXED_FRAC_WEIGHT_PRECISION, /* pll_flags */
		"xtal", /* pname */
		0 /* flag */),
	CLK_MUX(CLKID_SAR_ADC_SEL, "sar_adc_sel", /* clkid, name */
		CLKCTRL_SAR_CLK_CTRL0, 0x3, 9, /* reg, mask, shift */
		c4_sar_adc_parents, ARRAY_SIZE(c4_sar_adc_parents), /* pnames, num_pnames */
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
	CLK_MUX(CLKID_SPICC_0_SEL, "spicc_0_sel", /* clkid, name */
		CLKCTRL_SPISG_CLK_CTRL, 0x7, 7, /* reg, mask, shift */
		c4_spicc_parents, ARRAY_SIZE(c4_spicc_parents), /* pnames, num_pnames */
		NULL, 0 /* table, flag */),
	CLK_DIVIDER(CLKID_SPICC_0_DIV, "spicc_0_div", /* clkid, name */
		    CLKCTRL_SPISG_CLK_CTRL, 0, 6, /* reg, mask, shift */
		    0, /* dflags */
		    NULL, /* table */
		    "spicc_0_sel", /* pname */
		    0 /* _flags */),
	CLK_GATE(CLKID_SPICC_0, "spicc_0", /* clkid, name */
		 CLKCTRL_SPISG_CLK_CTRL, 6, /* reg, mask, shift */
		 "spicc_0_div", /* pname */
		 0 /* flags */),
	CLK_MUX(CLKID_SPICC_1_SEL, "spicc_1_sel", /* clkid, name */
		CLKCTRL_SPISG_CLK_CTRL, 0x7, 23, /* reg, mask, shift */
		c4_spicc_parents, ARRAY_SIZE(c4_spicc_parents), /* pnames, num_pnames */
		NULL, 0 /* table, flag */),
	CLK_DIVIDER(CLKID_SPICC_1_DIV, "spicc_1_div", /* clkid, name */
		    CLKCTRL_SPISG_CLK_CTRL, 16, 6, /* reg, mask, shift */
		    0, /* dflags */
		    NULL, /* table */
		    "spicc_1_sel", /* pname */
		    0 /* _flags */),
	CLK_GATE(CLKID_SPICC_1, "spicc_1", /* clkid, name */
		 CLKCTRL_SPISG_CLK_CTRL, 22, /* reg, mask, shift */
		 "spicc_1_div", /* pname */
		 0 /* flags */),
	CLK_MUX(CLKID_SD_EMMC_A_SEL, "sd_emmc_a_sel", /* clkid, name */
		CLKCTRL_SD_EMMC_CLK_CTRL0, 0x7, 9, /* reg, mask, shift */
		c4_sd_emmc_parents, ARRAY_SIZE(c4_sd_emmc_parents), /* pnames, num_pnames */
		NULL, 0 /* table, flag */),
	CLK_DIVIDER(CLKID_SD_EMMC_A_DIV, "sd_emmc_a_div", /* clkid, name */
		    CLKCTRL_SD_EMMC_CLK_CTRL0, 0, 7, /* reg, mask, shift */
		    0, /* dflags */
		    NULL, /* table */
		    "sd_emmc_a_sel", /* pname */
		    0 /* _flags */),
	CLK_GATE(CLKID_SD_EMMC_A, "sd_emmc_a", /* clkid, name */
		 CLKCTRL_SD_EMMC_CLK_CTRL0, 7, /* reg, mask, shift */
		 "sd_emmc_a_div", /* pname */
		 0 /* flags */),
	CLK_MUX(CLKID_SD_EMMC_B_SEL, "sd_emmc_b_sel", /* clkid, name */
		CLKCTRL_SD_EMMC_CLK_CTRL0, 0x7, 25, /* reg, mask, shift */
		c4_sd_emmc_parents, ARRAY_SIZE(c4_sd_emmc_parents), /* pnames, num_pnames */
		NULL, 0 /* table, flag */),
	CLK_DIVIDER(CLKID_SD_EMMC_B_DIV, "sd_emmc_b_div", /* clkid, name */
		    CLKCTRL_SD_EMMC_CLK_CTRL0, 16, 7, /* reg, mask, shift */
		    0, /* dflags */
		    NULL, /* table */
		    "sd_emmc_b_sel", /* pname */
		    0 /* _flags */),
	CLK_GATE(CLKID_SD_EMMC_B, "sd_emmc_b", /* clkid, name */
		 CLKCTRL_SD_EMMC_CLK_CTRL0, 23, /* reg, mask, shift */
		 "sd_emmc_b_div", /* pname */
		 0 /* flags */),
	CLK_MUX(CLKID_SD_EMMC_C_SEL, "sd_emmc_c_sel", /* clkid, name */
		CLKCTRL_SD_EMMC_CLK_CTRL1, 0x7, 9, /* reg, mask, shift */
		c4_sd_emmc_parents, ARRAY_SIZE(c4_sd_emmc_parents), /* pnames, num_pnames */
		NULL, 0 /* table, flag */),
	CLK_DIVIDER(CLKID_SD_EMMC_C_DIV, "sd_emmc_c_div", /* clkid, name */
		    CLKCTRL_SD_EMMC_CLK_CTRL1, 0, 7, /* reg, mask, shift */
		    0, /* dflags */
		    NULL, /* table */
		    "sd_emmc_c_sel", /* pname */
		    0 /* _flags */),
	CLK_GATE(CLKID_SD_EMMC_C, "sd_emmc_c", /* clkid, name */
		 CLKCTRL_SD_EMMC_CLK_CTRL1, 7, /* reg, mask, shift */
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
		    CLKCTRL_TS_CLK_CTRL, 0, 8, /* reg, mask, shift */
		    0, /* dflags */
		    NULL, /* table */
		    "xtal", /* pname */
		    0 /* _flags */),
	CLK_GATE(CLKID_TS, "ts", /* clkid, name */
		 CLKCTRL_TS_CLK_CTRL, 8, /* reg, mask, shift */
		 "ts_div", /* pname */
		 0 /* flags */)
};

static int aml_clk_probe(struct udevice *dev)
{
	return aml_clk_register_all(c4_clks, ARRAY_SIZE(c4_clks));
}

static const struct udevice_id aml_clk_ids[] = {
	{ .compatible = "amlogic,c4-clkc" },
	{ }
};

U_BOOT_DRIVER(aml_clk) = {
	.name		= "amlogic-clk-c4",
	.id		= UCLASS_CLK,
	.of_match	= aml_clk_ids,
	.ops		= &aml_clk_ops,
	.probe		= aml_clk_probe,
};
