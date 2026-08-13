// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <asm/io.h>
#include <bitfield.h>
#include <dm.h>
#include <errno.h>
#include <generic-phy.h>
#include <regmap.h>
#include <power/regulator.h>
#include <clk.h>
#include <asm/amlogic/arch/usb.h>
#include <amlogic/cpu_id.h>

#include <linux/compat.h>
#include <linux/ioport.h>
#include <asm-generic/gpio.h>
#include <asm/amlogic/arch/timer.h>

#define USB_PHY30_BIT			3
#define USB_PHY_20_BIT			4
#define USB_20_DRD_BIT		    5
#define USB_20_GENERAL_BIT		6
#define USB_PHY20_APB_BIT		7

#define USB_PHY_21_BIT			11
#define USB_21_DRD_BIT		    8
#define USB_21_GENERAL_BIT		9
#define USB_PHY21_APB_BIT		10

#define PIPE_CLK_REGS 			0xfe000360
#define PIPE_CLK_GATE_REGS 		0xfe310090

#define USB2_PLL_LOCK_EN_BIT 	24
#define USB2_PLL_RST_BIT 		25
#define USB2_PLL_BIAS_EN_BIT 	26
#define USB2_MPPLL_EN_CTRL_BIT 	27

#define PHY_20_COMP_BASE 		0xfe310000
#define PHY_20_BASE 			0xfe314000

#define PHY_21_COMP_BASE 		0xfe300000
#define PHY_21_BASE 			0xfe302000

#define RESET_BASE 				0xFE002000
#define RESET_LEVEL_BASE 		0xFE002040

#define AMLOGIC_CTR_COUNT		(0x2)

struct ctr_info {
	struct phy usb_phys[4];
	unsigned long phy_count;
};

static struct ctr_info ctr[AMLOGIC_CTR_COUNT];

void usb_udelay(unsigned int us)
{
	uint32_t t0 = get_time();

	while (get_time() - t0 <= us)
		;
}

int get_usbphy_baseinfo(void)
{
	struct udevice *bus;
	struct uclass *uc;
	int ret, i, j = 0;
	int count;

	for (i = 0; i < AMLOGIC_CTR_COUNT; i++) {
		if (ctr[i].usb_phys[0].dev && ctr[i].usb_phys[1].dev)
			return 0;
	}

	ret = uclass_get(UCLASS_USB, &uc);
	if (ret)
		return ret;
	uclass_foreach_dev(bus, uc) {
		debug("bus->name=%s, bus->driver->name =%s\n",
			bus->name, bus->driver->name);
		count = dev_count_phandle_with_args(bus, "phys", "#phy-cells", 1);
		debug("usb phy count=%u\n", count);
		if (count <= 0)
			return count;
		if (j >= AMLOGIC_CTR_COUNT) {
			pr_err("AMLOGIC_CTR_COUNT is small: %d\n", j);
			return -1;
		}
		for (i = 0; i < count; i++) {
			ret = generic_phy_get_by_index(bus, i, &ctr[j].usb_phys[i]);
			if (ret && ret != -ENOENT) {
				pr_err("Failed to get USB PHY%d for %s\n",
				       i, bus->name);
				return ret;
			}
			ret = generic_phy_getinfo(&ctr[j].usb_phys[i]);
			if (ret)
				return ret;
		}
		ctr[j].phy_count = count;
		j++;
	}
	return 0;
}

void usb_aml_detect_operation(int argc, char * const argv[])
{
	struct phy_aml_usb2_priv *usb2_priv;
	struct phy_aml_usb3_priv *usb3_priv;
	int ret, i;

	ret = get_usbphy_baseinfo();
	if (ret) {
		printf("get usb dts failed\n");
		return;
	}
	for (i = 0; i < AMLOGIC_CTR_COUNT; i++) {
		usb2_priv = dev_get_priv(ctr[i].usb_phys[0].dev);
		usb3_priv = dev_get_priv(ctr[i].usb_phys[1].dev);

		if (usb3_priv) {
			printf("priv->usb3 port num = %d, config addr=0x%08x\n",
			       usb3_priv->usb3_port_num, usb3_priv->base_addr);
		}
		if (usb2_priv) {
			printf("usb2 phy: config addr = 0x%08x, reset addr=0x%08x\n",
			       usb2_priv->base_addr, usb2_priv->reset_addr);

			printf("usb2 phy: portnum=%d, phy-addr1= 0x%08x, phy-addr2= 0x%08x\n",
			       usb2_priv->u2_port_num, usb2_priv->usb_phy2_pll_base_addr[0],
			usb2_priv->usb_phy2_pll_base_addr[1]);
			printf("dwc2_a base addr: 0x%08x\n", usb2_priv->dwc2_a_addr);
		}
	}
}

static void usb_set_calibration_trim(uint32_t phy2_pll_base)
{
	uint32_t cali, value, i;
	uint8_t cali_en;

	cali = readl(SYSCTRL_SEC_STATUS_REG12);
	//printf("SYSCTRL_SEC_STATUS_REG12=0x%08x\n", cali);
	/*****if cali_en ==0, set 0x10 to the default value: 0x1700****/
	cali_en = (cali >> 12) & 0x1;
	cali = cali >> 8;

	if (cali_en) {
		cali = (cali & 0xf);

		if (cali > 12)
			cali = 12;
		value = readl(phy2_pll_base + 0x10);
		value &= (~0xfff);

		for (i = 0; i < cali; i++)
			value |= (1 << i);

		writel(value, phy2_pll_base + 0x10);
	} else {
		value = readl(phy2_pll_base + 0x10);
		value &= (~0xfff);
		value |= 0x7f;
		writel(value, phy2_pll_base + 0x10);
	}

	printf("0x10 trim value=0x%08x\n", value);
}

void usb_reset(unsigned int reset_addr, int bit)
{
	*(volatile unsigned int *)(unsigned long)reset_addr = (1 << bit);
	writel((1 << bit), reset_addr);
}

static void usb_enable_phy_pll(u32 base_addr)
{
	writel(readl(RESET_LEVEL_BASE) | (1 << USB_PHY_20_BIT), RESET_LEVEL_BASE);
	writel(readl(RESET_LEVEL_BASE) | (1 << USB_PHY_21_BIT), RESET_LEVEL_BASE);
}

void set_usb_pll(uint32_t phy2_pll_base)
{
	int retry = 5;
	uint32_t pll_val0;
	uint32_t pll_val1;
	u64 phy_reg_base;

	phy_reg_base = phy2_pll_base;
	pll_val0 = 0x549540;
	pll_val1 = 0x1d00;

__retry:
	writel(pll_val1, (phy_reg_base + 0x44));

	usb_udelay(100);

	writel(pll_val0 | (1 << USB2_MPPLL_EN_CTRL_BIT), (phy_reg_base + 0x40));

	usb_udelay(100);

	writel(pll_val0 | (1 << USB2_MPPLL_EN_CTRL_BIT) | (1 << USB2_PLL_BIAS_EN_BIT),
		(phy_reg_base + 0x40));

	usb_udelay(100);

	writel(pll_val0 | (1 << USB2_MPPLL_EN_CTRL_BIT) | (1 << USB2_PLL_BIAS_EN_BIT) |
		(1 << USB2_PLL_RST_BIT), (phy_reg_base + 0x40));

	usb_udelay(100);

	writel(pll_val0 | (1 << USB2_MPPLL_EN_CTRL_BIT) | (1 << USB2_PLL_BIAS_EN_BIT) |
		(1 << USB2_PLL_RST_BIT) | (1 << USB2_PLL_LOCK_EN_BIT), (phy_reg_base + 0x40));

	// wait for 200us
	usb_udelay(200);
	//check lock bit
	writel(0x39, (phy_reg_base + 0x0c));
	writel(0x7e18, (phy_reg_base + 0x50));
	writel(0x523, (phy_reg_base + 0x60));

	if (readl((phy_reg_base + 0x40)) >> 31) {
		return;
	} else {
		retry --;
		if (!retry) {
			return;
		}
		goto __retry;
	}
}

int usb_save_phy_dev(unsigned int number, struct phy *phy)
{
	int i;

	for (i = 0; i < AMLOGIC_CTR_COUNT; i++) {
		if (!ctr[i].usb_phys[number].dev) {
			ctr[i].usb_phys[number].dev = phy->dev;
			ctr[i].usb_phys[number].id = phy->id;
		} else {
			if (ctr[i].usb_phys[number].dev == phy->dev)
				break;
		}
	}
	return 0;
}

int usb2_phy_init(struct phy *phy)
{
	struct phy_aml_usb2_priv *priv = dev_get_priv(phy->dev);
	struct u2p_aml_regs *u2p_aml_reg;
	u2p_r0_t dev_u2p_r0;
	u2p_r1_t dev_u2p_r1;
	int i, cnt;

	usb_save_phy_dev(0, phy);
	usb_enable_phy_pll(priv->base_addr);

	writel((readl(priv->reset_addr + 0x40) & (~(1 << USB_PHY30_BIT))),
		priv->reset_addr + 0x40);
	usb_udelay(10);

	if (priv->usb_phy2_pll_base_addr[0] == PHY_20_BASE) {
		debug("priv->reset_addr is 0x%x\n", priv->reset_addr);
		writel((1 << USB_20_DRD_BIT) | (1 << USB_20_GENERAL_BIT), priv->reset_addr);

		usb_udelay(500);
		priv->usbphy_reset_bit[0] = USB_PHY_20_BIT;
	} else {
		debug("priv->reset_addr is 0x%x\n", priv->reset_addr);
		writel((1 << USB_21_DRD_BIT) | (1 << USB_21_GENERAL_BIT), priv->reset_addr);

		usb_udelay(500);
		priv->usbphy_reset_bit[0] = USB_PHY_21_BIT;
	}

	for (i = 0; i < priv->u2_port_num; i++) {
		u2p_aml_reg = (struct u2p_aml_regs *)((ulong)(priv->base_addr + i * PHY_REGISTER_SIZE));
		debug("u2p_aml_reg is 0x%x\n", (u32)(u64)u2p_aml_reg);
		dev_u2p_r0.d32 = u2p_aml_reg->u2p_r0;
		dev_u2p_r0.b.host_device = 1;
		dev_u2p_r0.b.POR = 0;
		u2p_aml_reg->u2p_r0  = dev_u2p_r0.d32;
		usb_udelay(10);
		writel(1 << priv->usbphy_reset_bit[i], priv->reset_addr);
		usb_udelay(50);

		writel(readl(PIPE_CLK_REGS) | 0x101, PIPE_CLK_REGS);
		usb_udelay(12);
		writel((readl(priv->base_addr + 0x90) | (1 << 2) | (1 << 3)), priv->base_addr + 0x90);
		usb_udelay(12);

		/* wait for phy ready */
		dev_u2p_r1.d32  = u2p_aml_reg->u2p_r1;
		cnt = 0;
		while (dev_u2p_r1.b.phy_rdy != 1) {
			dev_u2p_r1.d32 = u2p_aml_reg->u2p_r1;
			/*we wait phy ready max 1ms, common is 100us*/
			if (cnt > 200) {
				break;
			} else {
				cnt++;
				usb_udelay(5);
			}
		}
	}

	for (i = 0; i < priv->u2_port_num; i++) {
		debug("------set usb pll\n");
		set_usb_pll(priv->usb_phy2_pll_base_addr[i]);
	}
	return 0;
}

int usb2_phy_tuning(uint32_t phy2_pll_base, int port)
{
	return 0;
}

static unsigned int usb_powerctrl_reg = 0xffffffff;

void set_usb_power_off(void)
{
	unsigned int val;
	printf("set c5 usb phy off.\n");
	usb_powerctrl_reg = readl(RESETCTRL_RESET0_LEVEL);
	val = usb_powerctrl_reg;
	val &= ~(0x3ff << 2);
	writel(val, RESETCTRL_RESET0_LEVEL);
}

void set_usb_power_on(void)
{
	printf("set c5 usb phy on.\n");
	writel(usb_powerctrl_reg, RESETCTRL_RESET0_LEVEL);
}

/**************************************************************/
/*           device mode config                               */
/**************************************************************/
void set_usb_phy2x_config(uint32_t force_usb3_enable)
{
	u2p_r0_t dev_u2p_r0;
	u2p_r1_t dev_u2p_r1;
	int cnt;
	u2p_aml_regs_t *u2p_aml_regs;
	unsigned int phy_base_addr, reset_addr;

	if (force_usb3_enable) {
		u2p_aml_regs = (u2p_aml_regs_t *)((unsigned long)(PHY_20_COMP_BASE));
		phy_base_addr = PHY_20_BASE;
	} else {
		u2p_aml_regs = (u2p_aml_regs_t *)((unsigned long)(PHY_21_COMP_BASE));
		phy_base_addr = PHY_21_BASE;
	}
	reset_addr = RESET_BASE;

	printf("PHY2=%p,phy-base=0x%08x\n", u2p_aml_regs, phy_base_addr);

	//step 1: power off usb3 phy
	writel((readl(RESET_LEVEL_BASE) & (~(1 << USB_PHY30_BIT))), RESET_LEVEL_BASE);

	//step 2: power on usb2 phy
	if (force_usb3_enable) {
		writel((readl(RESET_LEVEL_BASE) & (~(1 << USB_PHY_20_BIT))), RESET_LEVEL_BASE);
		usb_udelay(500);
		writel((readl(RESET_LEVEL_BASE) | (1 << USB_PHY_20_BIT)), RESET_LEVEL_BASE);
	} else {
		writel((readl(RESET_LEVEL_BASE) & (~(1 << USB_PHY_21_BIT))), RESET_LEVEL_BASE);
		usb_udelay(500);
		writel((readl(RESET_LEVEL_BASE) | (1 << USB_PHY_21_BIT)), RESET_LEVEL_BASE);
	}

	usb_udelay(100);

	//step 3: usb controller reset
	if (force_usb3_enable)
		usb_reset(reset_addr, USB_20_GENERAL_BIT);
	else
		usb_reset(reset_addr, USB_21_GENERAL_BIT);

	usb_udelay(100);

	// step 4: config phy device mode
	if (force_usb3_enable)
		writel(readl(PHY_20_COMP_BASE + 0x88) | (1 << 25), (PHY_20_COMP_BASE + 0x88));
	else
		writel(readl(PHY_21_COMP_BASE + 0x88) | (1 << 25), (PHY_21_COMP_BASE + 0x88));

	usb_udelay(100);

	dev_u2p_r0.d32	 = u2p_aml_regs->u2p_r0;
	dev_u2p_r0.b.host_device = 0;
	dev_u2p_r0.b.POR = 0;
	u2p_aml_regs->u2p_r0  = dev_u2p_r0.d32;

	usb_udelay(10);

	/*step 5: phy apb reset*/
	if (force_usb3_enable)
		writel((1 << USB_PHY20_APB_BIT), RESET_BASE);
	else
		writel((1 << USB_PHY21_APB_BIT), RESET_BASE);
	usb_udelay(10);

	//step 6: phy reset
	if (force_usb3_enable)
		usb_reset(reset_addr, USB_PHY_20_BIT);
	else
		usb_reset(reset_addr, USB_PHY_21_BIT);

	usb_udelay(12);

	/* step 7: pipe clk setting*/
	if (force_usb3_enable) {
		writel(readl(PIPE_CLK_REGS) | 0x101, PIPE_CLK_REGS);
		usb_udelay(12);
		writel((readl(PIPE_CLK_GATE_REGS) | (1 << 2) | (1 << 3)), PIPE_CLK_GATE_REGS);
		usb_udelay(12);
	}

	// step 8: trim
	usb_udelay(50);
	usb_set_calibration_trim(phy_base_addr);
	usb_udelay(50);

	// step 9: wait for phy ready
	dev_u2p_r1.d32	= u2p_aml_regs->u2p_r1;
	cnt = 0;
	while ((dev_u2p_r1.d32 & 0x00000001) != 1) {
		dev_u2p_r1.d32 = u2p_aml_regs->u2p_r1;
		if (cnt > 200) {
			break;
		} else {
			cnt++;
			usb_udelay(5);
		}
	}

	/* step 10: pll setting*/
	set_usb_pll(phy_base_addr);
	//--------------------------------------------------

	// ------------- usb phy initial end ----------

	//--------------------------------------------------
}

void usb_device_mode_init(int phy_num)
{
	uint32_t poc, force_usb3_enable;

	poc = readl(SYSCTRL_POC) & 0xff;
	force_usb3_enable = POC_USB_CHANNEL_A(poc);

	set_usb_phy2x_config(force_usb3_enable);
}
