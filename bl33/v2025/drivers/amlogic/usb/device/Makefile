# SPDX-License-Identifier: GPL-2.0
#

ifndef CONFIG_USB_GADGET_CRG
ifndef CONFIG_USB_GADGET_DWC3
obj-$(CONFIG_FASTBOOT_DWC_PCD) += dwc_pcd.o dwc_pcd_irq.o
endif
else
obj-$(CONFIG_USB_GADGET_CRG) += crg_udc.o
endif

obj-$(CONFIG_USB_GADGET_DWC3) += dwc3_meson_udc.o
obj-$(CONFIG_USB_GADGET) +=  platform.o
