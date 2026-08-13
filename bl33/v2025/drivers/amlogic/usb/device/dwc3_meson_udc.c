// SPDX-License-Identifier: GPL-2.0
/*
 * Layerscape DWC3 Glue layer
 *
 * Copyright (C) 2021 Michael Walle <michael@walle.cc>
 *
 * Based on dwc3-generic.c.
 */

#include <dm.h>
#include <dm/device_compat.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dwc3-uboot.h>
#include <linux/usb/gadget.h>
#include <usb.h>
#include "../../../usb/dwc3/core.h"
#include "../../../usb/dwc3/gadget.h"
#include <usb/xhci.h>
#include <asm/amlogic/arch/usb.h>

int dwc3_phy_num = 1;
EXPORT_SYMBOL_GPL(dwc3_phy_num);

struct dwc3_gadget_dev {
	void *base;
	struct dwc3 dwc3;
	struct device dev;
};

static struct dwc3_gadget_dev dwc3_udc_dev;

void dwc3_meson_probe(void)
{
	struct dwc3_gadget_dev *dwc3_udc;

	//dcache_disable();

	dwc3_udc = &dwc3_udc_dev;

	usb_device_mode_init(dwc3_phy_num);

	dwc3_udc->base = (void __iomem *)(uintptr_t)DWC3_UDC_BASE;
	dwc3_udc->dwc3.regs = dwc3_udc->base + DWC3_GLOBALS_REGS_START;
	dwc3_udc->dwc3.dr_mode = USB_DR_MODE_PERIPHERAL;
	dwc3_udc->dwc3.maximum_speed = USB_SPEED_HIGH;

	dwc3_init(&(dwc3_udc->dwc3));
}
EXPORT_SYMBOL_GPL(dwc3_meson_probe);

static int dwc3_gadget_handle_interrupts(struct dwc3_gadget_dev *dwc3_udc)
{
	dwc3_gadget_uboot_handle_interrupt(&(dwc3_udc->dwc3));

	return 0;
}

int dm_usb_gadget_handle_interrupts(struct udevice *dev)
{
	return dwc3_gadget_handle_interrupts(&dwc3_udc_dev);
}

void dwc_otg_power_off_phy_fb(void)
{
}
