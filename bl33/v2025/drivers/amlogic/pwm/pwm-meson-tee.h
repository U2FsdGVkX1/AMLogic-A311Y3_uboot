/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef _PWM_MESON_TEE_H
#define _PWM_MESON_TEE_H
#include <linux/bitfield.h>
#include <linux/bitops.h>
#include <amlogic/secure_pwm_i2c.h>

#define PWM_MESON_CLOCK_RATE			24000000
enum sec_pwm {
	SECID_PWM_ENABLE_MAIN = 0,
	SECID_PWM_ENABLE_SUB,
	SECID_PWM_DISABLE_MAIN,
	SECID_PWM_DISABLE_SUB,
	SECID_PWM_CONSTANT_EN,
	SECID_PWM_CONSTANT_DIS,
	SECID_PWM_TIMES_MAIN,
	SECID_PWM_TIMES_SUB,
};

#define REG_PWM			0x0
#define PWM_LOW_MASK		GENMASK(15, 0)
#define PWM_HIGH_MASK		GENMASK(31, 16)
#define PWM_MISC_REG		0x8
#define MISC_EN			BIT(0)
#define MISC_CONSTANT		BIT(28)
#define CHANNEL_MAIN		0

#define DOUBLE_CHAN_COMPAT
#ifdef DOUBLE_CHAN_COMPAT
#define MESON_NUM_PWMS		2
#define CHANNEL_SUB			1
#else
#define MESON_NUM_PWMS		1
#endif

enum pwm_polarity {
	PWM_POLARITY_NORMAL,
	PWM_POLARITY_INVERSED,
};

struct meson_pwm_tee_state {
	unsigned int period;
	unsigned int duty_cycle;
	unsigned int hi;
	unsigned int lo;
	unsigned int pre_div;
	enum pwm_polarity polarity;
	bool enabled;
};

struct meson_pwm_tee_reg {
	u32 dar;/* Duty Register */
	u32 reserved1;/* reserved */
	u32 miscr;/* misc Register */
	u32 dsr;/*DS Register*/
	u32 tr;/*times Register*/
	u32 da2r;/* Sub Duty Register */
	u32 reserved2;/* reserved */
	u32 br;/*Blink Register*/
};

struct meson_pwm_tee_priv {
	struct meson_pwm_tee_reg *regs;
	struct meson_pwm_tee_state state[MESON_NUM_PWMS];
	fdt_addr_t extern_clk_addr;
	u32 tee_id;
};

#endif   /* _PWM_MESON_TEE_H_ */
