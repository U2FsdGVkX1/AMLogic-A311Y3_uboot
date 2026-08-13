/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */


#ifndef __EXTRA_REGISTER_H__
#define __EXTRA_REGISTER_H__

//sec_ao
#define SEC_AO_SEC_GP_CFG0    SYSCTRL_SEC_STATUS_REG4
#define SEC_AO_SEC_GP_CFG2  SYSCTRL_SEC_STATUS_REG6

//OTP
#define OTP_LIC			(OTP_LIC_A)
#define OTP_LIC00		(OTP_LIC + 0x00)

#define OTP_LIC0		(OTP_LIC00)
#define REG_BASE_VCBUS                             (0xff000000L)

#define CLKCTRL_PWM_CLK_GH_CTRL                    ((0x0063  << 2) + 0xfe000000)
#define CLKCTRL_PWM_CLK_IJ_CTRL                    ((0x0064  << 2) + 0xfe000000)
#define PWM_TEE_ONLY_J                             ((0x0009  << 2) + 0xfe059200)
#define PADCTRL_GPIOH_PULL_EN                      ((0x0053  << 2) + 0xfe004000)

#endif
