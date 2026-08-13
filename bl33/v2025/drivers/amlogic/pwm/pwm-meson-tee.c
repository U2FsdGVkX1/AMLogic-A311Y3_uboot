// SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
/*
 * PWM controller driver for Amlogic Meson SoCs.
 *
 * This PWM is only a set of Gates, Dividers and Counters:
 * PWM output is achieved by calculating a clock that permits calculating
 * two periods (low and high). The counter then has to be set to switch after
 * N cycles for the first half period.
 * The hardware has no "polarity" setting. This driver reverses the period
 * cycles (the low length is inverted with the high length) for
 * PWM_POLARITY_INVERSED. This means that .get_state cannot read the polarity
 * from the hardware.
 * Setting the duty cycle will disable and re-enable the PWM output.
 * Disabling the PWM stops the output immediately (without waiting for the
 * current period to complete first).
 *
 * The public S912 (GXM) datasheet contains some documentation for this PWM
 * controller starting on page 543:
 * https://dl.khadas.com/Hardware/VIM2/Datasheet/S912_Datasheet_V0.220170314publicversion-Wesion.pdf
 * An updated version of this IP block is found in S922X (G12B) SoCs. The
 * datasheet contains the description for this IP block revision starting at
 * page 1084:
 * https://dn.odroid.com/S922X/ODROID-N2/Datasheet/S922X_Public_Datasheet_V0.2.pdf
 *
 * Copyright (c) 2016 BayLibre, SAS.
 * Author: Neil Armstrong <narmstrong@baylibre.com>
 * Copyright (C) 2014 Amlogic, Inc.
 */
#include <dm.h>
#include <pwm.h>
#include <regmap.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/sizes.h>
#include <linux/printk.h>
#include <linux/time.h>
#include <div64.h>
#include <dm/device.h>
#include <dm/device_compat.h>
#include <linux/bitfield.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/math64.h>
#include <linux/arm-smccc.h>
#include "pwm-meson-tee.h"

#define DOUBLE_CHAN_COMPAT
//#define MESON_PWM_DEBUG

static int meson_pwm_tee_set_invert(struct udevice *dev, uint channel,
			      bool polarity)
{
	struct meson_pwm_tee_priv *priv = dev_get_priv(dev);

	debug("%s: polarity=%u\n", __func__, polarity);
	if (channel > MESON_NUM_PWMS)
		return -EACCES;
	priv->state[channel].polarity = polarity;

	return 0;
}

static int pwm_meson_t6x_get_polarity(struct udevice *dev, uint channel)
{
	struct meson_pwm_tee_priv *priv = dev_get_priv(dev);
	struct meson_pwm_tee_state *pwm_state = priv->state;

	return pwm_state[channel].polarity;
}

static int meson_pwm_tee_calc(struct udevice *dev, uint channel, uint period,
			uint duty)
{
	struct meson_pwm_tee_priv *priv = dev_get_priv(dev);
	struct meson_pwm_tee_state *pwm_state = priv->state;
	unsigned int cnt, duty_cnt, inv;
	u64 fin_freq, pre_div , fin_ps;


	inv = pwm_meson_t6x_get_polarity(dev, channel);
	if (inv)
		duty = period - duty;
#ifdef MESON_PWM_DEBUG
	dev_err(dev, "set perido:%u, duty:%u\n", period, duty);
#endif
	pwm_state->duty_cycle = duty;
	pwm_state->period = period;
	fin_freq = PWM_MESON_CLOCK_RATE;
	fin_ps = (u64)NSEC_PER_SEC * 1000;
	do_div(fin_ps, fin_freq);
	for (pre_div = 0; pre_div < 0x7f; pre_div++) {
		cnt = DIV_ROUND_CLOSEST_ULL((u64)period * 1000,
				fin_ps * (pre_div + 1));
		if (cnt <= 0xffff)
			break;
	}
#ifdef MESON_PWM_DEBUG
	dev_err(dev, "cal cnt:%u pre div:%llx\n", cnt, pre_div);
#endif
	if (cnt > 0xffff) {
		dev_err(dev, "unable to get period cnt\n");
		return -EINVAL;
	}

	if (duty == period) {
		pwm_state[channel].pre_div = pre_div;
		pwm_state[channel].hi = cnt;
		pwm_state[channel].lo = 0;
	} else if (duty == 0) {
		pwm_state[channel].pre_div = pre_div;
		pwm_state[channel].hi = 0;
		pwm_state[channel].lo = cnt;
	} else {
		duty_cnt = DIV_ROUND_CLOSEST_ULL(fin_freq * (u64)duty,
			(u64)NSEC_PER_SEC * (pre_div + 1));
#ifdef MESON_PWM_DEBUG
		dev_err(dev, "cal duty_cnt:%u\n", cnt);
#endif
		if (duty_cnt == 0)
			duty_cnt++;
		pwm_state[channel].pre_div = pre_div;
		pwm_state[channel].hi = duty_cnt - 1;
		pwm_state[channel].lo = cnt - duty_cnt - 1;
	}
#ifdef MESON_PWM_DEBUG
	dev_err(dev, "set hi:%u, lo:%u\n", pwm_state[channel].hi , pwm_state[channel].lo);
#endif

	return 0;
}

static void meson_pwm_tee_disable(struct udevice *dev, uint channel)
{
	struct arm_smccc_res res;
	struct meson_pwm_tee_priv *priv = dev_get_priv(dev);

#ifdef DOUBLE_CHAN_COMPAT
	switch (channel) {
	case CHANNEL_MAIN:
		arm_smccc_smc(SECURE_PWM_I2C, SECID_PWM, priv->tee_id,
				 SECID_PWM_DISABLE_MAIN,  0, 0, 0, 0, &res);
		break;
	case CHANNEL_SUB:
		arm_smccc_smc(SECURE_PWM_I2C, SECID_PWM, priv->tee_id,
				 SECID_PWM_DISABLE_SUB,  0, 0, 0, 0, &res);
		break;
	default:
		break;
	}
#else
	arm_smccc_smc(SECURE_PWM_I2C, SECID_PWM, priv->tee_id,
				 SECID_PWM_DISABLE_MIAN, 0, 0, 0, 0, &res);
#endif
}

static void meson_pwm_tee_enable(struct udevice *dev, uint channel)
{
	struct meson_pwm_tee_priv *priv = dev_get_priv(dev);
	struct meson_pwm_tee_state *pwm_state = priv->state;
	fdt_addr_t clk_addr = priv->extern_clk_addr;
	u32 value;
	struct arm_smccc_res res;

	value = FIELD_PREP(PWM_HIGH_MASK, pwm_state[channel].hi) |
		FIELD_PREP(PWM_LOW_MASK, pwm_state[channel].lo);
#ifdef DOUBLE_CHAN_COMPAT
	switch (channel) {
	case CHANNEL_MAIN:
#ifdef MESON_PWM_DEBUG
		dev_err(dev, "main enable value:0x%x\n", value);
#endif
		arm_smccc_smc(SECURE_PWM_I2C, SECID_PWM, priv->tee_id,
				 SECID_PWM_ENABLE_MAIN, value, 0, 0, 0, &res);
		break;
	case CHANNEL_SUB:
#ifdef MESON_PWM_DEBUG
		dev_err(dev, "sub enable value:0x%x\n", value);
#endif
		arm_smccc_smc(SECURE_PWM_I2C, SECID_PWM, priv->tee_id,
				 SECID_PWM_ENABLE_SUB, value, 0, 0, 0, &res);
		break;
	default:
		break;
	}
#else
	arm_smccc_smc(SECURE_PWM_I2C, SECID_PWM, priv->tee_id,
				 SECID_PWM_ENABLE_MAIN, value, 0, 0, 0, &res);
#endif
	clrsetbits_le32(clk_addr, (0xff << 0) | (3 << 9),
				(pwm_state[channel].pre_div << 0 | 1 << 8));
}

static int pwm_constant_enable_tee(struct udevice *dev, uint channel)
{
	struct meson_pwm_tee_priv *priv = dev_get_priv(dev);
	struct meson_pwm_tee_state *pwm_state = priv->state;
	struct arm_smccc_res res;

	arm_smccc_smc(SECURE_PWM_I2C, SECID_PWM, priv->tee_id,
				 SECID_PWM_CONSTANT_EN, FIELD_PREP(PWM_HIGH_MASK, pwm_state[channel].hi) |
		FIELD_PREP(PWM_LOW_MASK, pwm_state[channel].lo), 0, 0, 0, &res);

	return 0;
}

static int pwm_constant_disable_tee(struct udevice *dev, uint channel)
{
	struct meson_pwm_tee_priv *priv = dev_get_priv(dev);
	struct meson_pwm_tee_state *pwm_state = priv->state;
	struct arm_smccc_res res;

	arm_smccc_smc(SECURE_PWM_I2C, SECID_PWM, priv->tee_id,
			SECID_PWM_CONSTANT_DIS, FIELD_PREP(PWM_HIGH_MASK, pwm_state[channel].hi) |
		FIELD_PREP(PWM_LOW_MASK, pwm_state[channel].lo), 0, 0, 0, &res);

	return 0;
}

static int meson_pwm_tee_set_enable(struct udevice *dev, uint channel, bool enable)
{
	struct meson_pwm_tee_priv *priv = dev_get_priv(dev);
	struct meson_pwm_tee_state *pwm_state = priv->state;

	if (pwm_state[channel].enabled != enable) {
		if (enable) {
			meson_pwm_tee_enable(dev, channel);
			pwm_state[channel].enabled = true;
		} else {
			meson_pwm_tee_disable(dev, channel);
			pwm_state[channel].enabled = false;
		}
	}

	return 0;
};

static int meson_pwm_tee_set_config(struct udevice *dev, uint channel, uint period_ns,
				uint duty_ns)
{
	struct meson_pwm_tee_priv *priv = dev_get_priv(dev);
	struct meson_pwm_tee_state *pwm_state = priv->state;

	if (period_ns == 0) {
		dev_err(dev, "period_ns can not be zero\n");
		return -EINVAL;
	}

	if (duty_ns > period_ns) {
		dev_err(dev, "Not available duty_ns period_ns error\n");
		return -EINVAL;
	}

	if (channel > MESON_NUM_PWMS)
		return -EACCES;

	if (pwm_state[channel].period != period_ns ||
	    pwm_state[channel].duty_cycle != duty_ns) {
		if (meson_pwm_tee_calc(dev, channel, period_ns, duty_ns))
			return -EINVAL;
		meson_pwm_tee_enable(dev, channel);
		if (pwm_state->duty_cycle == pwm_state->period || pwm_state->duty_cycle == 0)
			pwm_constant_enable_tee(dev, channel);
		else
			pwm_constant_disable_tee(dev, channel);
	}

	pwm_state[channel].period = period_ns;
	pwm_state[channel].duty_cycle = duty_ns;
	pwm_state[channel].enabled = 1;

	return 0;
};

static int meson_pwm_tee_probe(struct udevice *dev)
{
	return 0;
}

static int meson_pwm_tee_ofdata_to_platdata(struct udevice *dev)
{
	struct meson_pwm_tee_priv *priv = dev_get_priv(dev);
	int err;

	priv->regs = (struct meson_pwm_tee_reg *)dev_read_addr_index(dev, 0);
	if (priv->regs == (void *)FDT_ADDR_T_NONE) {
		dev_err(dev, "Coun't get pwm base regs addr\n");
		return -1;
	}
	priv->extern_clk_addr = dev_read_addr_index(dev, 1);
	if (priv->extern_clk_addr == FDT_ADDR_T_NONE) {
		dev_err(dev, "Coun't get pwm clk regs addr\n");
		return -1;
	}
	/* get pwm tee_id property */
	err = dev_read_u32_index(dev, "tee_id", 0, &priv->tee_id);
	if (err) {
		dev_err(dev, "not config tee_id\n");
		return err;
	}

	return 0;
}

static const struct pwm_ops meson_pwm_tee_ops = {
	.set_invert	= meson_pwm_tee_set_invert,
	.set_config	= meson_pwm_tee_set_config,
	.set_enable	= meson_pwm_tee_set_enable,
};

static const struct udevice_id meson_pwm_tee_ids[] = {
	{.compatible = "amlogic,meson-pwm-a9",},
	{}
};

U_BOOT_DRIVER(meson_pwm_tee) = {
	.name = "meson_pwm_tee",
	.id = UCLASS_PWM,
	.of_match = meson_pwm_tee_ids,
	.of_to_plat = meson_pwm_tee_ofdata_to_platdata,
	.ops = &meson_pwm_tee_ops,
	.probe = meson_pwm_tee_probe,
	.priv_auto = sizeof(struct meson_pwm_tee_priv),
};
