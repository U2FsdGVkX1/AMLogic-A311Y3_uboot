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

#define PHY_20_BASE 			0xfe48e000
#define PHY_21_BASE				0xfe4ce000
#define PHY_22_BASE				0xfe4ae000
#define PHY_23_BASE				0xfe4a6000

#define PIPE_CLK_REGS			0xfe000360
#define PIPE_CLK_GATE_REGS		0xfe488090

//RESET 0 U3DRD
#define USB_GENERAL_A_BIT	6
#define USB_DRD_A_BIT		5
#define USB_PHY_A_BIT		4
#define USB_PHY3_BIT		3
#define USB_PHY_A_APB_BIT	7

//RESET 3 U3H
#define USB_GENERAL_B_BIT	23//?
#define USB_DRD_B_BIT		27
#define USB_PHY_B_BIT		30
#define USB_PHY3_B_BIT		24
#define USB_PHY_B_APB_BIT	29

//RESET 2 U2H
#define USB_GENERAL_C_BIT	13
#define USB_DRD_C_BIT		14
#define USB_PHY_C_BIT		20
#define USB_PHY_C_APB_BIT	15

//RESET 0 U2DRD
#define USB_GENERAL_D_BIT	14
#define USB_DRD_D_BIT		15
#define USB_PHY_D_BIT		11 //RESET 2
#define USB_PHY_D_APB_BIT	13

#define USB_PLL_CFG_BIT		27
#define USB_PLL_BIAS_EN_BIT	26
#define USB_PLL_RSTN_BIT	25
#define USB_PLL_LOCK_EN_BIT	24

#define PHY20_RESET_LEVEL_BIT	USB_PHY_A_BIT //RESET0
#define PHY21_RESET_LEVEL_BIT	USB_PHY_B_BIT //RESET3
#define PHY22_RESET_LEVEL_BIT	USB_PHY_C_BIT //RESET2
#define PHY23_RESET_LEVEL_BIT	USB_PHY_D_BIT //RESET2

#define RESET_BASE              0xFE002000
#define RESET_LEVEL_BASE        0xFE002040

#define USB_AML_U2A_REGS		0xfe488000
#define USB_AML_U2D_REGS		0xfe4a4000

#define USB_PHY_A_BASE			PHY_20_BASE
#define USB_PHY_D_BASE			PHY_23_BASE

#define TUNING_DISCONNECT_THRESHOLD 0x7f

#define AMLOGIC_CTR_COUNT		(0x4)

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
		cali = (cali & 0xf) + 2;

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
		value |= 0x1ff;
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
	writel(readl(RESET_LEVEL_BASE) | (1 << PHY20_RESET_LEVEL_BIT),
		RESET_LEVEL_BASE);
	writel(readl(RESET_LEVEL_BASE + 0xc) | (1 << PHY21_RESET_LEVEL_BIT),
		RESET_LEVEL_BASE + 0xc);
	writel(readl(RESET_LEVEL_BASE + 0x8) | (1 << PHY22_RESET_LEVEL_BIT),
		RESET_LEVEL_BASE + 0x8);
	writel(readl(RESET_LEVEL_BASE + 0x8) | (1 << PHY23_RESET_LEVEL_BIT),
		RESET_LEVEL_BASE + 0x8);
}

void set_usb_pll(uint32_t phy2_pll_base)
{
	uint32_t pll_val0;
	u64 phy_reg_base;
	phy_reg_base = phy2_pll_base;

	/* set default value
	 USB_PLL_CFG_BIT	 27
	 USB_PLL_BIAS_EN_BIT 26
	 USB_PLL_RSTN_BIT	 25
	 USB_PLL_LOCK_EN_BIT 24
	 */
	pll_val0 = 0x549540;

	writel(pll_val0 | (1 << USB_PLL_CFG_BIT), (phy_reg_base + 0x40));

	usb_udelay(100);

	writel(pll_val0 | (1 << USB_PLL_CFG_BIT) | (1 << USB_PLL_BIAS_EN_BIT),
		(phy_reg_base + 0x40));

	usb_udelay(100);

	writel(pll_val0 | (1 << USB_PLL_CFG_BIT) | (1 << USB_PLL_BIAS_EN_BIT)
		| (1 << USB_PLL_RSTN_BIT), (phy_reg_base + 0x40));

	usb_udelay(100);

	writel(pll_val0 | (1 << USB_PLL_CFG_BIT) | (1 << USB_PLL_BIAS_EN_BIT)
		| (1 << USB_PLL_RSTN_BIT) | (1 <<USB_PLL_LOCK_EN_BIT ),
		(phy_reg_base + 0x40));

	// wait for 200us
	usb_udelay(200);

	writel(TUNING_DISCONNECT_THRESHOLD, phy_reg_base + 0xc);
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

	writel(0x901, CLKCTRL_PP_CLK_CTRL);
	udelay(500);

	writel((readl(RESET_LEVEL_BASE) & (~(1 << USB_PHY3_BIT))), RESET_LEVEL_BASE);
	writel((readl(RESET_LEVEL_BASE + 0xc) & (~(1 << USB_PHY3_B_BIT))), RESET_LEVEL_BASE + 0xc);

	udelay(500);

	usb_save_phy_dev(0, phy);
	usb_enable_phy_pll(priv->base_addr);

	if (priv->usb_phy2_pll_base_addr[0] == PHY_20_BASE) {
		debug("priv->reset_addr is 0x%x\n", priv->reset_addr);
		writel((1 << USB_DRD_A_BIT) | (1 << USB_GENERAL_A_BIT), priv->reset_addr);

		udelay(500);
		priv->usbphy_reset_bit[0] = PHY20_RESET_LEVEL_BIT;
	} else if (priv->usb_phy2_pll_base_addr[0] == PHY_21_BASE) {
		debug("priv->reset_addr is 0x%x\n", priv->reset_addr);
		writel((1 << USB_DRD_B_BIT) | (1 << USB_GENERAL_B_BIT), priv->reset_addr + 0xc);

		udelay(500);
		priv->usbphy_reset_bit[0] = PHY21_RESET_LEVEL_BIT;
	} else if (priv->usb_phy2_pll_base_addr[0] == PHY_22_BASE) {
		debug("priv->reset_addr is 0x%x\n", priv->reset_addr);
		writel((1 << USB_DRD_C_BIT) | (1 << USB_GENERAL_C_BIT), priv->reset_addr + 0x8);

		udelay(500);
		priv->usbphy_reset_bit[0] = PHY22_RESET_LEVEL_BIT;
	} else {
		debug("priv->reset_addr is 0x%x\n", priv->reset_addr);
		writel((1 << USB_DRD_D_BIT) | (1 << USB_GENERAL_D_BIT), priv->reset_addr);

		udelay(500);
		priv->usbphy_reset_bit[0] = PHY23_RESET_LEVEL_BIT;
	}

	for (i = 0; i < priv->u2_port_num; i++) {
		u2p_aml_reg = (struct u2p_aml_regs *)((ulong)(priv->base_addr + i * PHY_REGISTER_SIZE));
		debug("u2p_aml_reg is 0x%x\n", (u32)(u64)u2p_aml_reg);
		dev_u2p_r0.d32 = u2p_aml_reg->u2p_r0;
		dev_u2p_r0.b.host_device = 1;
		dev_u2p_r0.b.POR = 0;
		u2p_aml_reg->u2p_r0  = dev_u2p_r0.d32;
		udelay(10);
		if (priv->usb_phy2_pll_base_addr[0] == PHY_20_BASE)
			writel(1 << priv->usbphy_reset_bit[i], priv->reset_addr);
		else if (priv->usb_phy2_pll_base_addr[0] == PHY_21_BASE)
			writel(1 << priv->usbphy_reset_bit[i], priv->reset_addr + 0xc);
		else
			writel(1 << priv->usbphy_reset_bit[i], priv->reset_addr + 0x8);

		udelay(50);

		writel(readl(PIPE_CLK_REGS) | 0x101, PIPE_CLK_REGS);
		udelay(12);
		writel((readl(priv->base_addr + 0x90) | (1 << 2) | (1 << 3)), priv->base_addr + 0x90);
		udelay(50);

		usb_set_calibration_trim(priv->usb_phy2_pll_base_addr[i]);
		udelay(50);

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
				udelay(5);
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

static unsigned int usb_powerctrl_reg_1 = 0xffffffff;
static unsigned int usb_powerctrl_reg_2 = 0xffffffff;
static unsigned int usb_powerctrl_reg_3 = 0xffffffff;

void set_usb_power_off(void)
{
	unsigned int val;

	printf("set a9 usb phy off.\n");
	usb_powerctrl_reg_1 = readl(RESETCTRL_RESET0_LEVEL);
	val = usb_powerctrl_reg_1;
	val &= ~(0x3 << USB_PHY3_BIT);
	writel(val, RESETCTRL_RESET0_LEVEL);

	usb_powerctrl_reg_2 = readl(RESETCTRL_RESET2_LEVEL);
	val = usb_powerctrl_reg_2;
	val &= ~((0x1 << 11) | (0x1 << 20));
	writel(val, RESETCTRL_RESET2_LEVEL);

	usb_powerctrl_reg_3 = readl(RESETCTRL_RESET3_LEVEL);
	val = usb_powerctrl_reg_3;
	val &= ~((0x1 << 24) | (0x1 << 30));
	writel(val, RESETCTRL_RESET3_LEVEL);
}

void set_usb_power_on(void)
{
	printf("set a9 usb phy on.\n");
	writel(usb_powerctrl_reg_1, RESETCTRL_RESET0_LEVEL);
	writel(usb_powerctrl_reg_2, RESETCTRL_RESET2_LEVEL);
	writel(usb_powerctrl_reg_3, RESETCTRL_RESET3_LEVEL);
}

static int set_usbphy_vbus_off(void)
{
	struct udevice *bus;
	struct uclass *uc;
	int ret, i;
	int count;
	struct phy usb_phy;

	ret = uclass_get(UCLASS_USB, &uc);
	if (ret)
		return ret;

	uclass_foreach_dev(bus, uc) {
		debug("bus->name=%s, bus->driver->name =%s\n", bus->name, bus->driver->name);
		count = dev_count_phandle_with_args(bus, "phys", "#phy-cells", 1);
		debug("usb phy count=%u\n", count);
		if (count <= 0) {
			pr_err("Error %s no USB PHY.\n", bus->name);
			return count;
		}

		for (i = 0; i < count; i++) {
			ret = generic_phy_get_by_index(bus, i, &usb_phy);
			if (ret && ret != -ENOENT) {
				pr_err("Failed to get USB PHY%d for %s\n",
				       i, bus->name);
				return ret;
			}

			if (IS_ENABLED(CONFIG_DM_GPIO)) {
				int ret;
				struct gpio_desc gd;

				debug("turing off %s usb gpios\n",
				      ofnode_get_name(dev_ofnode(usb_phy.dev)));

				ret = gpio_request_by_name(usb_phy.dev,
							   "gpios", 0, &gd,
							   GPIOD_IS_OUT);
				if (!ret) {
					if (dm_gpio_set_value(&gd, 0))
						pr_err("failed to set VBUS logical low\n");
					dm_gpio_free(usb_phy.dev, &gd);
				}
			}
		}
	}
	return 0;
}

void usb_hw_cleanup(void)
{
	set_usb_power_off();
	set_usbphy_vbus_off();
	printf("usb hw cleanup done.\n");
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
		u2p_aml_regs = (u2p_aml_regs_t *)USB_AML_U2A_REGS;
		phy_base_addr = USB_PHY_A_BASE;
	} else {
		u2p_aml_regs = (u2p_aml_regs_t *)USB_AML_U2D_REGS;
		phy_base_addr = USB_PHY_D_BASE;
	}

	reset_addr = RESET_BASE;

	printf("PHY2=%p,phy-base=0x%08x\n", u2p_aml_regs, phy_base_addr);
	//if ((*(volatile uint32_t *)(unsigned long)(phy_base_addr + 0x38)) != 0) {
		//usb_phy_tuning_reset(phy_num);
		//mdelay(150);
	//}

	writel((readl(RESET_LEVEL_BASE) & (~(1 << USB_PHY3_BIT))), RESET_LEVEL_BASE);
	if (force_usb3_enable) {
		writel((readl(RESET_LEVEL_BASE) & (~(1 << USB_PHY_A_BIT))), RESET_LEVEL_BASE);
		usb_udelay(500);
		writel((readl(RESET_LEVEL_BASE) | (1 << USB_PHY_A_BIT)), RESET_LEVEL_BASE);
	} else {
		writel((readl(RESET_LEVEL_BASE + 0x8) & (~(1 << USB_PHY_D_BIT))), RESET_LEVEL_BASE + 0x8);
		usb_udelay(500);
		writel((readl(RESET_LEVEL_BASE + 0x8) | (1 << USB_PHY_D_BIT)), RESET_LEVEL_BASE + 0x8);
	}
	usb_udelay(10);

	//step 1: usb controller reset
	if (force_usb3_enable)
		usb_reset(reset_addr, USB_GENERAL_A_BIT);
	else
		usb_reset(reset_addr, USB_GENERAL_D_BIT);
	usb_udelay(10);

	/*step 2: config phy device mode*/
	/*power on dwc3 controller*/
	if (force_usb3_enable)
		writel(readl(USB_AML_U2A_REGS + 0x88) | (1 << 25), (USB_AML_U2A_REGS + 0x88));
	else
		writel(readl(USB_AML_U2D_REGS + 0x88) | (1 << 25), (USB_AML_U2D_REGS + 0x88));
	usb_udelay(10);

	// step 3: config phy21 device mode
	dev_u2p_r0.d32	 = u2p_aml_regs->u2p_r0;
	dev_u2p_r0.b.host_device = 0;
	dev_u2p_r0.b.POR = 0;
	u2p_aml_regs->u2p_r0  = dev_u2p_r0.d32;

	usb_udelay(10);

	/*step 4: phy apb reset*/
	if (force_usb3_enable)
		writel((1 << USB_PHY_A_APB_BIT), RESET_BASE);
	else
		writel((1 << USB_PHY_D_APB_BIT), RESET_BASE);
	usb_udelay(10);

	//step 5: phy reset
	if (force_usb3_enable)
		usb_reset(reset_addr, USB_PHY_A_BIT);
	else
		usb_reset(reset_addr + 0x8, USB_PHY_D_BIT);
	usb_udelay(10);

	/* step 6: pipe clk setting*/
	if (force_usb3_enable) {
		writel(readl(PIPE_CLK_REGS) | 0x101, PIPE_CLK_REGS);
		usb_udelay(12);
		writel((readl(PIPE_CLK_GATE_REGS) | (1 << 2) | (1 << 3)), PIPE_CLK_GATE_REGS);
	}

	usb_udelay(50);
	usb_set_calibration_trim(phy_base_addr);
	usb_udelay(50);

	// step 7: wait for phy ready
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
