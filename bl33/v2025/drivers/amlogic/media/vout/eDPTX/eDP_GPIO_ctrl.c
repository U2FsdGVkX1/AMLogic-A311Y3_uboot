// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

// #include <common.h>
#include <errno.h>
#include <dm.h>
#include <asm/gpio.h>
#include <amlogic/media/vout/eDPTX/eDPTX.h>
#include <linux/types.h>
#include "eDP_common.h"
#include "eDP_regs.h"
#include "eDP_dummy_reg.h"

void edptx_gpio_set(const char *name, int value)
{
	int gpio;
	int ret = 0;

#if defined(CONFIG_DM_GPIO)
	if (gpio_lookup_name(name, NULL, NULL, (unsigned int *)&gpio)) {
		DPTXPR(0, LOG_E, "gpio wrong name %s", name);
		return;
	}
#else
	/* turn the gpio name into a gpio number */
	gpio = simple_strtoul(name, NULL, 10);
	if (gpio < 0) {
		DPTXPR(0, LOG_E, "gpio wrong name %s", name);
		return;
	}
#endif

	DPTXPR(0, LOG_I, "%s %d, value=%d", __func__, gpio, value);

	/* grab the pin before we tweak it */
	ret = gpio_request(gpio, "MESON_eDPTX_GPIO");
	if (ret && ret != -EBUSY) {
		DPTXPR(0, LOG_E, "gpio requesting pin %d failed", gpio);
		return;
	}

	gpio_direction_output(gpio, value);
}

unsigned int edptx_gpio_input_get(const char *name)
{
	unsigned int value;
	int gpio;

#if defined(CONFIG_DM_GPIO)
	if (gpio_lookup_name(name, NULL, NULL, (unsigned int *)&gpio)) {
		DPTXPR(0, LOG_E, "gpio wrong name %s", name);
		return -1;
	}
#else
	/* turn the gpio name into a gpio number */
	gpio = simple_strtoul(name, NULL, 10);
	if (gpio < 0) {
		DPTXPR(0, LOG_E, "gpio wrong name %s", name);
		return -1;
	}
#endif

	if (gpio >= DPTX_GPIO_MAX)
		return 0;
	gpio_direction_input(gpio);
	value = gpio_get_value(gpio);
	return value;
}

void edptx_HPD_pinmux_set(struct dptx_drv_s *dptx)
{
	switch (dptx->data->chip_type) {
#if defined(CONFIG_MESON_T7) || defined(CONFIG_MESON_T7C)
	case eDPTX_CHIP_T7:
		if (dptx->idx == 0) {
			if (dptx->sink.hpd_mask & BIT(0))
				dptx_periphs_setb(PADCTRL_PIN_MUX_REGK, 4, 8, 4);
			if (dptx->sink.hpd_mask & BIT(1))
				dptx_periphs_setb(PADCTRL_PIN_MUX_REGK, 4, 12, 4);
		} else if (dptx->idx == 1) {
			dptx_periphs_setb(PADCTRL_PIN_MUX_REGK, 4, 12, 4);
		}
		break;
#endif
#if defined(CONFIG_MESON_A9)
	case eDPTX_CHIP_T7:
		if (dptx->idx == 0) {
			dptx_periphs_setb(PADCTRL_PIN_MUX_REGK, 4, 12, 4);
		} else if (dptx->idx == 1) {
			dptx_periphs_setb(PADCTRL_PIN_MUX_REGK, 4, 12, 4);
		}
		break;
#endif
	case eDPTX_CHIP_MAX:
	default:
		break;
	}
}
