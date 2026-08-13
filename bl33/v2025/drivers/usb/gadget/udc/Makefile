# SPDX-License-Identifier: GPL-2.0+
#
# USB peripheral controller drivers

ifndef CONFIG_$(XPL_)DM_USB_GADGET
obj-$(CONFIG_USB_DWC3_GADGET)	+= udc-core.o
endif

ifndef CONFIG_USB_GADGET_DWC3
obj-$(CONFIG_$(XPL_)DM_USB_GADGET)	+= udc-core.o
else
obj-$(CONFIG_USB_GADGET_DWC3)      += udc-core.o
endif

obj-y += udc-uclass.o
