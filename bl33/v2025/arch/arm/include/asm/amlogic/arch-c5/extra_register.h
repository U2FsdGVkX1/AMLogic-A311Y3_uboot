// SPDX-License-Identifier: (GPL-2.0-only OR MIT)
/*
 * Copyright (C) 2025 Amlogic, Inc. All rights reserved
 */

#ifndef __EXTRA_REGISTER_H__
#define __EXTRA_REGISTER_H__

//OTP
#define OTP_LIC										(OTP_LIC_A)
#define OTP_LIC00                                  (OTP_LIC + 0x00)
#define OTP_LIC01                                  (OTP_LIC + 0x04)
#define OTP_LIC02                                  (OTP_LIC + 0x08)
#define OTP_LIC03                                  (OTP_LIC + 0x0C)

#define OTP_LIC10                                  (OTP_LIC + 0x10)
#define OTP_LIC11                                  (OTP_LIC + 0x14)
#define OTP_LIC12                                  (OTP_LIC + 0x18)
#define OTP_LIC13                                  (OTP_LIC + 0x1C)

#define OTP_LIC20                                  (OTP_LIC + 0x20)
#define OTP_LIC21                                  (OTP_LIC + 0x24)
#define OTP_LIC22                                  (OTP_LIC + 0x28)
#define OTP_LIC23                                  (OTP_LIC + 0x2C)

#define OTP_LIC30                                  (OTP_LIC + 0x30)
#define OTP_LIC31                                  (OTP_LIC + 0x34)
#define OTP_LIC32                                  (OTP_LIC + 0x38)
#define OTP_LIC33                                  (OTP_LIC + 0x3C)

#define OTP_LIC0                                   (OTP_LIC00)

//sec_ao
#define SEC_AO_SEC_GP_CFG2  SYSCTRL_SEC_STATUS_REG6

//clk
#define CLKCTRL_PWM_CLK_AB_CTRL                    ((0x0060  << 2) + 0xfe000000)
#define PADCTRL_GPIOH_PULL_EN                      ((0x0053  << 2) + 0xfe004000)

//pwm
#define PWM_PWM_A                                  ((0x0000  << 2) + 0xfe058000)
#define PWM_PWM_B                                  ((0x0000  << 2) + 0xfe058200)

#define PWM_MISC_REG_A                             ((0x0002  << 2) + 0xfe058000)
#define PWM_MISC_REG_B                             ((0x0002  << 2) + 0xfe058200)

#endif
