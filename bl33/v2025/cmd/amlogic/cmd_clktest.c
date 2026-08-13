// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2025 Amlogic, Inc. All rights reserved.
 */

#include <command.h>
#include <clk.h>
#include <dm.h>
#include <dm/device.h>
#include <dm/root.h>
#include <dm/device-internal.h>
#include <linux/arm-smccc.h>
#include <linux/clk-provider.h>
#include <linux/math64.h>
#include <asm/amlogic/arch/io.h>
#include <asm/amlogic/arch/secure_apb.h>
#include <amlogic/clk-core.h>

#define MAX_CLK_MEASURE_TOL			1000000ul

static unsigned long clk_util_clk_msr(unsigned long clk_mux)
{
	unsigned int regval = 0;

	writel(0, MSR_CLK_REG0);
	/* Set the measurement gate to 640us */
	setbits_le32(MSR_CLK_REG0, 640 - 1);
	/* Disable continuous measurement */
	/* Disable interrupts */
	clrbits_le32(MSR_CLK_REG0, (1 << 17) | (1 << 18));
	clrbits_le32(MSR_CLK_REG0, 0x7f << 20);
	setbits_le32(MSR_CLK_REG0, (clk_mux) << 20 | (1 << 19) | (1 << 16));
	/* Wait for the measurement to be done */
	regval = readl(MSR_CLK_REG0);
	do {
		regval = readl(MSR_CLK_REG0);
	} while (regval & (1 << 31));

	/* Disable measuring */
	clrbits_le32(MSR_CLK_REG0, 1 << 16);
	regval = readl(MSR_CLK_REG2) & 0x000fffff;

	return DIV_ROUND_UP_ULL(regval * 1000000ull, 640);
}

static struct clk *aml_clk_get_by_name(const char *name)
{
	struct udevice *dev;
	struct uclass *uc;
	struct clk *clkp;
	int ret;

	ret = uclass_get(UCLASS_CLK, &uc);
	if (ret)
		return NULL;

	uclass_foreach_dev(dev, uc) {
		clkp = dev_get_clk_ptr(dev);
		if (device_get_uclass_id(dev) == UCLASS_CLK && clkp &&
		    (strcmp(name, clkp->dev->name) == 0))
			return clkp;
	}

	return NULL;
}

static int aml_clk_msr_check(const char *msr_name, unsigned long msr_rate,
			     unsigned long req_rate)
{
	if (abs(req_rate - msr_rate) > MAX_CLK_MEASURE_TOL) {
		log_err("%s: measure check failed, req_rate = %ld, msr_rate = %ld\n",
			msr_name, req_rate, msr_rate);

		return -EIO;
	}

	return 0;
}

static int aml_clk_mux_test(const char *clk_name, struct clk_mux *data,
			    const char *msr_name, int msr_id)
{
	struct clk *clk = aml_clk_get_by_name(clk_name);
	struct clk *msr_clk = aml_clk_get_by_name(msr_name);
	struct clk *pclk;
	int i, pnum = data->num_parents;
	unsigned long last_msr_clk_rate = 0, msr_clk_rate, msr_rate;

	if (!clk || !msr_clk || !data)
		return -EINVAL;

	for (i = 0; i < pnum; i++) {
		pclk = aml_clk_get_by_name(data->parent_names[i]);
		if (!pclk) {
			log_notice("%s: can't find parent of %s \n", clk_name,
				   data->parent_names[i]);

			continue;
		}

		clk_set_parent(clk, pclk);
		msr_clk_rate = clk_get_rate(msr_clk);
		if (msr_clk_rate == last_msr_clk_rate)
			log_warning("%s: no change after switching parent\n",
				    clk_name);

		clk_enable(msr_clk);
		msr_rate = clk_util_clk_msr(msr_id);
		clk_disable(msr_clk);
		if (aml_clk_msr_check(msr_name, msr_rate, msr_clk_rate)) {
			log_err("%s: test failed after switching parent to %s\n",
				clk_name, data->parent_names[i]);

			return -EIO;
		}

		last_msr_clk_rate = msr_clk_rate;
	}

	return 0;
}

static unsigned int clk_div_get_max_table_div(const struct clk_div_table *table)
{
	const struct clk_div_table *clkt;
	unsigned int max_div = 0;

	for (clkt = table; clkt->div; clkt++)
		if (clkt->div > max_div)
			max_div = clkt->div;

	return max_div;
}

static unsigned int _get_max_div(const struct clk_div_table *table,
				 unsigned long flags, u8 width)
{
	int div_mask = clk_div_mask(width);

	if (flags & CLK_DIVIDER_ONE_BASED)
		return div_mask;
	if (flags & CLK_DIVIDER_POWER_OF_TWO)
		return 1 << div_mask;
	if (flags & CLK_DIVIDER_MAX_AT_ZERO)
		return div_mask + 1;
	if (table)
		return clk_div_get_max_table_div(table);

	return div_mask + 1;
}

static int aml_clk_div_test(const char *clk_name, struct clk_divider *data,
			    const char *msr_name, int msr_id)
{
	struct clk *clk = aml_clk_get_by_name(clk_name);
	struct clk *msr_clk = aml_clk_get_by_name(msr_name);
	struct clk *pclk;
	unsigned long p_rate, last_msr_clk_rate = 0, msr_clk_rate, msr_rate;
	int div;

	if (!clk || !msr_clk || !data)
		return -EINVAL;

	pclk = clk_get_parent(clk);
	p_rate = clk_get_rate(pclk);

	// set min div
	clk_set_rate(clk, p_rate);
	msr_clk_rate = clk_get_rate(msr_clk);
	clk_enable(msr_clk);
	msr_rate = clk_util_clk_msr(msr_id);
	clk_disable(msr_clk);
	if (aml_clk_msr_check(msr_name, msr_rate, msr_clk_rate)) {
		log_err("%s: test min division parameter failed\n",clk_name);

		return -EIO;
	}

	last_msr_clk_rate = msr_clk_rate;
	// set max div
	div = _get_max_div(data->table, data->flags, data->width);
	clk_set_rate(clk, p_rate / div);
	msr_clk_rate = clk_get_rate(msr_clk);
	if (msr_clk_rate == last_msr_clk_rate)
		log_warning("%s: no change after setting rate?\n", clk_name);

	clk_enable(msr_clk);
	msr_rate = clk_util_clk_msr(msr_id);
	clk_disable(msr_clk);
	if (aml_clk_msr_check(msr_name, msr_rate, msr_clk_rate)) {
		log_err("%s: test max division parameter failed\n",clk_name);

		return -EIO;
	}

	return 0;
}

static int aml_clk_gate_test(const char *clk_name, const char *msr_name, int msr_id)
{
	struct clk *clk = aml_clk_get_by_name(clk_name);
	struct clk *msr_clk = aml_clk_get_by_name(msr_name);
	unsigned long msr_clk_rate, msr_rate;

	if (!clk || !msr_clk)
		return -EINVAL;

	msr_clk_rate = clk_get_rate(msr_clk);
	clk_enable(msr_clk);
	msr_rate = clk_util_clk_msr(msr_id);
	if (aml_clk_msr_check(msr_name, msr_rate, msr_clk_rate)) {
		log_err("%s: test gate enable failed\n",clk_name);

		return -EIO;
	}

	clk_disable(msr_clk);
	msr_rate = clk_util_clk_msr(msr_id);
	if (aml_clk_msr_check(msr_name, msr_rate, 0)) {
		log_err("%s: test gate disable failed\n",clk_name);

		return -EIO;
	}

	return 0;
}

static int aml_clk_pll_test(const char *clk_name, struct aml_clk_pll *data,
			    const char *msr_name, int msr_id)
{
	struct clk *clk = aml_clk_get_by_name(clk_name);
	struct clk *msr_clk = aml_clk_get_by_name(msr_name);
	unsigned long last_msr_clk_rate = 0, msr_clk_rate, msr_rate;
	int i;

	if (!clk || !msr_clk || !data)
		return -EINVAL;

	for (i = 0; i < data->table_count; i++) {
		clk_set_rate(clk, data->table[i].rate);
		msr_clk_rate = clk_get_rate(msr_clk);
		if (msr_clk_rate == last_msr_clk_rate)
			log_warning("%s: no change after setting rate?\n",
				    clk_name);

		clk_enable(msr_clk);
		msr_rate = clk_util_clk_msr(msr_id);
		clk_disable(msr_clk);
		if (aml_clk_msr_check(msr_name, msr_rate, msr_clk_rate)) {
			log_err("%s: test failed after setting rate to %ld\n",
				clk_name, data->table[i].rate);

			return -EIO;
		}

		last_msr_clk_rate = msr_clk_rate;
	}

	return 0;
}

static int aml_clk_msr_test(const char *clk_name, const char *msr_name, int msr_id)
{
	struct clk *clk = aml_clk_get_by_name(clk_name);
	struct clk *msr_clk = aml_clk_get_by_name(msr_name);
	unsigned long msr_clk_rate, msr_rate;

	if (!clk || !msr_clk)
		return -EINVAL;

	msr_clk_rate = clk_get_rate(msr_clk);
	msr_rate = clk_util_clk_msr(msr_id);
	if (aml_clk_msr_check(msr_name, msr_rate, msr_clk_rate)) {
		log_err("%s: msr test failed\n", clk_name);

		return -EIO;
	}

	return 0;
}

struct aml_clk_test_data {
	struct aml_clk_data *clk_data;
	const char * msr_name;
	int msr_id;
};

void aml_clk_test(const struct aml_clk_test_data *test_data, int num)
{
	int i, ret;
	struct aml_clk_data *clk_data;
	struct clk *clk;

	if (!test_data || !num)
		return;

	for (i = 0; i < num; i++) {
		clk_data = test_data[i].clk_data;
		clk = aml_clk_get_by_name(clk_data->name);
		if (!clk) {
			log_notice("can't find %s, skip testing it\n",
				   clk_data->name);

			continue;
		}

		if (clk && clk->enable_count) {
			log_notice("%s has been enabled, skip testing it\n",
				   clk_data->name);

			continue;
		}

		switch (clk_data->type) {
			case MUX:
				ret = aml_clk_mux_test(clk_data->name,
						       &clk_data->data.mux,
						       test_data[i].msr_name,
						       test_data[i].msr_id);
				break;
			case DIVIDER:
				ret = aml_clk_div_test(clk_data->name,
						       &clk_data->data.divider,
						       test_data[i].msr_name,
						       test_data[i].msr_id);
				break;
			case GATE:
				ret = aml_clk_gate_test(clk_data->name,
							test_data[i].msr_name,
							test_data[i].msr_id);
				break;
			case PLL:
				ret = aml_clk_pll_test(clk_data->name,
						       &clk_data->data.pll,
						       test_data[i].msr_name,
						       test_data[i].msr_id);
				break;
			default:
				ret = aml_clk_msr_test(clk_data->name,
						       test_data[i].msr_name,
						       test_data[i].msr_id);
				break;

		}
		if (ret)
			log_err("%s: clock function test failed, ret = %d\n",
				clk_data->name, ret);
	}
}

#ifdef CONFIG_CLK_MESON_C4
#include <dt-bindings/amlogic/clock/c4-clkc.h>

extern struct aml_clk_data c4_clks[];

struct aml_clk_test_data c4_clk_test_data[] = {
	{ &c4_clks[CLKID_FCLK_DIV2], "fclk_div2", 8 },
	{ &c4_clks[CLKID_FCLK_DIV2P5], "fclk_div2p5", 9 },
	{ &c4_clks[CLKID_FCLK_DIV3], "fclk_div3", 10 },
	{ &c4_clks[CLKID_FCLK_DIV4], "fclk_div4", 11 },
	{ &c4_clks[CLKID_FCLK_DIV5], "fclk_div5", 12 },
	{ &c4_clks[CLKID_FCLK_DIV7], "fclk_div7", 13 },
	{ &c4_clks[CLKID_SYS_CLK], "sys_clk", 0 },
	{ &c4_clks[CLKID_GP0_PLL], "gp0_pll", 20 },
	{ &c4_clks[CLKID_HIFI_PLL], "hifi_pll", 19 },
	{ &c4_clks[CLKID_SAR_ADC_SEL], "sar_adc", 111 },
	{ &c4_clks[CLKID_SAR_ADC_DIV], "sar_adc", 111 },
	{ &c4_clks[CLKID_SAR_ADC], "sar_adc", 111 },
	{ &c4_clks[CLKID_SPICC_0_SEL], "spicc_0", 118 },
	{ &c4_clks[CLKID_SPICC_0_DIV], "spicc_0", 118 },
	{ &c4_clks[CLKID_SPICC_0], "spicc_0", 118 },
	{ &c4_clks[CLKID_SPICC_1_SEL], "spicc_1", 117 },
	{ &c4_clks[CLKID_SPICC_1_DIV], "spicc_1", 117 },
	{ &c4_clks[CLKID_SPICC_1], "spicc_1", 117 },
	{ &c4_clks[CLKID_SD_EMMC_A_SEL], "sd_emmc_a", 115 },
	{ &c4_clks[CLKID_SD_EMMC_A_DIV], "sd_emmc_a", 115 },
	{ &c4_clks[CLKID_SD_EMMC_A], "sd_emmc_a", 115 },
	{ &c4_clks[CLKID_SD_EMMC_B_SEL], "sd_emmc_b", 114 },
	{ &c4_clks[CLKID_SD_EMMC_B_DIV], "sd_emmc_b", 114 },
	{ &c4_clks[CLKID_SD_EMMC_B], "sd_emmc_b", 114 },
	{ &c4_clks[CLKID_SD_EMMC_C_SEL], "sd_emmc_c", 113 },
	{ &c4_clks[CLKID_SD_EMMC_C_DIV], "sd_emmc_c", 113 },
	{ &c4_clks[CLKID_SD_EMMC_C], "sd_emmc_c", 113 },
	{ &c4_clks[CLKID_ETH_RMII_DIV], "eth_rmii", 33 },
	{ &c4_clks[CLKID_ETH_RMII], "eth_rmii", 33 },
	{ &c4_clks[CLKID_TS_DIV], "ts", 112 },
	{ &c4_clks[CLKID_TS], "ts", 112 },
};
#endif /* CONFIG_CLK_MESON_C4 */

#ifdef CONFIG_CLK_MESON_A9
#include <dt-bindings/amlogic/clock/a9-clkc.h>

extern struct aml_clk_data a9_clks[];

struct aml_clk_test_data a9_clk_test_data[] = {
	{ &a9_clks[CLKID_FCLK_DIV5], "fclk_div5", 11 },
	{ &a9_clks[CLKID_SYS_CLK], "sys_clk", 0 },
	{ &a9_clks[CLKID_GP0_PLL], "gp0_pll", 20 },
	{ &a9_clks[CLKID_HIFI_PLL], "hifi_pll", 19 },
	{ &a9_clks[CLKID_SAR_ADC_SEL], "sar_adc", 111 },
	{ &a9_clks[CLKID_SAR_ADC_DIV], "sar_adc", 111 },
	{ &a9_clks[CLKID_SAR_ADC], "sar_adc", 111 },
	{ &a9_clks[CLKID_SPISG_0_SEL], "spisg_0", 118 },
	{ &a9_clks[CLKID_SPISG_0_DIV], "spisg_0", 118 },
	{ &a9_clks[CLKID_SPISG_0], "spisg_0", 118 },
	{ &a9_clks[CLKID_SPISG_1_SEL], "spisg_1", 105 },
	{ &a9_clks[CLKID_SPISG_1_DIV], "spisg_1", 105 },
	{ &a9_clks[CLKID_SPISG_1], "spisg_1", 105 },
	{ &a9_clks[CLKID_SPISG_2_SEL], "spisg_2", 104 },
	{ &a9_clks[CLKID_SPISG_2_DIV], "spisg_2", 104 },
	{ &a9_clks[CLKID_SPISG_2], "spisg_2", 104 },
	{ &a9_clks[CLKID_SD_EMMC_A_SEL], "sd_emmc_a", 115 },
	{ &a9_clks[CLKID_SD_EMMC_A_DIV], "sd_emmc_a", 115 },
	{ &a9_clks[CLKID_SD_EMMC_A], "sd_emmc_a", 115 },
	{ &a9_clks[CLKID_SD_EMMC_B_SEL], "sd_emmc_b", 114 },
	{ &a9_clks[CLKID_SD_EMMC_B_DIV], "sd_emmc_b", 114 },
	{ &a9_clks[CLKID_SD_EMMC_B], "sd_emmc_b", 114 },
	{ &a9_clks[CLKID_SD_EMMC_C_SEL], "sd_emmc_c", 113 },
	{ &a9_clks[CLKID_SD_EMMC_C_DIV], "sd_emmc_c", 113 },
	{ &a9_clks[CLKID_SD_EMMC_C], "sd_emmc_c", 113 },
	{ &a9_clks[CLKID_ETH_RMII_DIV], "eth_rmii", 125 },
	{ &a9_clks[CLKID_ETH_RMII], "eth_rmii", 125 },
	{ &a9_clks[CLKID_TS_DIV], "ts", 121 },
	{ &a9_clks[CLKID_TS], "ts", 121 },
	{ &a9_clks[CLKID_AMFC_SEL], "amfc", 89 },
	{ &a9_clks[CLKID_AMFC_DIV], "amfc", 89 },
	{ &a9_clks[CLKID_AMFC], "amfc", 89 },
	{ &a9_clks[CLKID_VPU_0_SEL], "vpu", 61 },
	{ &a9_clks[CLKID_VPU_0_DIV], "vpu", 61 },
	{ &a9_clks[CLKID_VPU_0], "vpu", 61 },
	{ &a9_clks[CLKID_VPU_1_SEL], "vpu", 61 },
	{ &a9_clks[CLKID_VPU_1_DIV], "vpu", 61 },
	{ &a9_clks[CLKID_VPU_1], "vpu", 61 },
	{ &a9_clks[CLKID_VPU], "vpu", 61 },
	{ &a9_clks[CLKID_VAPB_0_SEL], "vapb", 66 },
	{ &a9_clks[CLKID_VAPB_0_DIV], "vapb", 66 },
	{ &a9_clks[CLKID_VAPB_0], "vapb", 66 },
	{ &a9_clks[CLKID_VAPB_1_SEL], "vapb", 66 },
	{ &a9_clks[CLKID_VAPB_1_DIV], "vapb", 66 },
	{ &a9_clks[CLKID_VAPB_1], "vapb", 66 },
	{ &a9_clks[CLKID_VAPB], "vapb", 66 },
	{ &a9_clks[CLKID_GE2D], "ge2d", 67 },
};
#endif /* CONFIG_CLK_MESON_A9 */

struct aml_clk_test_data_info {
	const struct aml_clk_test_data *data;
	unsigned int num;
};

struct aml_clk_test_data_info aml_clk_test_data_info = {
#ifdef CONFIG_CLK_MESON_C4
	.data = c4_clk_test_data,
	.num = ARRAY_SIZE(c4_clk_test_data)
#ifdef CONFIG_CLK_MESON_A9
	.data = a9_clk_test_data,
	.num = ARRAY_SIZE(a9_clk_test_data)
#endif
};

#define SIP_SMC_SCMI_CMD			0x820000C1
#define SCMI_SUBID_TEST_SCMI_CLK		0x14

static int do_clktest_bl31(struct cmd_tbl *cmdtp, int flag, int argc,
			   char *const argv[])
{
	struct arm_smccc_res res;

	arm_smccc_smc(SIP_SMC_SCMI_CMD, SCMI_SUBID_TEST_SCMI_CLK,
		      0, 0, 0, 0, 0, 0, &res);

	return (int)res.a0;
}

static int do_clktest_bl33(struct cmd_tbl *cmdtp, int flag, int argc,
			   char *const argv[])
{
	aml_clk_test(aml_clk_test_data_info.data, aml_clk_test_data_info.num);

	return 0;
}

static struct cmd_tbl cmd_clktest_sub[] = {
	U_BOOT_CMD_MKENT(bl31, 1, 1, do_clktest_bl31, "", ""),
	U_BOOT_CMD_MKENT(bl33, 1, 1, do_clktest_bl33, "", ""),
};

static int do_clktest(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct cmd_tbl *c;

	if (argc < 2)
		return CMD_RET_USAGE;

	/* Strip off leading 'clktest' command argument */
	argc--;
	argv++;

	c = find_cmd_tbl(argv[0], cmd_clktest_sub, ARRAY_SIZE(cmd_clktest_sub));

	if (c)
		return c->cmd(cmdtp, flag, argc, argv);
	else
		return CMD_RET_USAGE;
}

U_BOOT_CMD(clktest, 2, 1, do_clktest,
	   "Amlogic test clock",
	   "	- Test all supported clocks.\n"
	   "bl31 - Test the clock driving function of BL31 (or BL32)\n"
	   "bl33 - Test the clock driving function of BL33\n"
);
