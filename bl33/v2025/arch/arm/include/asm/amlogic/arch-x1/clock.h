/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __X1_H
#define __X1_H

// REG_BASE:  REGISTER_BASE_ADDR = 0xfe000000
// -----------------------------------------------
#define X1_CLKCTRL_OSCIN_CTRL                         (0x0001  << 2)
#define X1_CLKCTRL_RTC_BY_OSCIN_CTRL0                 (0x0002  << 2)
#define X1_CLKCTRL_RTC_BY_OSCIN_CTRL1                 (0x0003  << 2)
#define X1_CLKCTRL_RTC_CTRL                           (0x0004  << 2)
#define X1_CLKCTRL_CHECK_CLK_RESULT                   (0x0005  << 2)
#define X1_CLKCTRL_MBIST_ATSPEED_CTRL                 (0x0006  << 2)
#define X1_CLKCTRL_LOCK_BIT_REG0                      (0x0008  << 2)
#define X1_CLKCTRL_LOCK_BIT_REG1                      (0x0009  << 2)
#define X1_CLKCTRL_LOCK_BIT_REG2                      (0x000a  << 2)
#define X1_CLKCTRL_LOCK_BIT_REG3                      (0x000b  << 2)
#define X1_CLKCTRL_PROT_BIT_REG0                      (0x000c  << 2)
#define X1_CLKCTRL_PROT_BIT_REG1                      (0x000d  << 2)
#define X1_CLKCTRL_PROT_BIT_REG2                      (0x000e  << 2)
#define X1_CLKCTRL_PROT_BIT_REG3                      (0x000f  << 2)
#define X1_CLKCTRL_SYS_CLK_CTRL0                      (0x0010  << 2)
#define X1_CLKCTRL_SYS_CLK_EN0_REG0                   (0x0011  << 2)
#define X1_CLKCTRL_SYS_CLK_EN0_REG1                   (0x0012  << 2)
#define X1_CLKCTRL_SYS_CLK_EN0_REG2                   (0x0013  << 2)
#define X1_CLKCTRL_SYS_CLK_EN0_REG3                   (0x0014  << 2)
#define X1_CLKCTRL_SYS_CLK_EN1_REG0                   (0x0015  << 2)
#define X1_CLKCTRL_SYS_CLK_EN1_REG1                   (0x0016  << 2)
#define X1_CLKCTRL_SYS_CLK_EN1_REG2                   (0x0017  << 2)
#define X1_CLKCTRL_SYS_CLK_EN1_REG3                   (0x0018  << 2)
#define X1_CLKCTRL_AXI_CLK_CTRL0                      (0x001b  << 2)
#define X1_CLKCTRL_TST_CTRL0                          (0x0020  << 2)
#define X1_CLKCTRL_TST_CTRL1                          (0x0021  << 2)
#define X1_CLKCTRL_CLK12_24_CTRL                      (0x002a  << 2)
#define X1_CLKCTRL_AXI_CLK_EN0                        (0x002b  << 2)
#define X1_CLKCTRL_AXI_CLK_EN1                        (0x002c  << 2)
#define X1_CLKCTRL_TS_CLK_CTRL                        (0x0056  << 2)
#define X1_CLKCTRL_EPP_CLK_CTRL                       (0x0058  << 2)
#define X1_CLKCTRL_NAND_CLK_CTRL                      (0x005a  << 2)
#define X1_CLKCTRL_SPISG_CLK_CTRL                     (0x005d  << 2)
#define X1_CLKCTRL_GEN_CLK_CTRL                       (0x005e  << 2)
#define X1_CLKCTRL_SAR_CLK_CTRL0                      (0x005f  << 2)
#define X1_CLKCTRL_PWM_CLK_AB_CTRL                    (0x0060  << 2)
#define X1_CLKCTRL_PWM_CLK_CD_CTRL                    (0x0061  << 2)
#define X1_CLKCTRL_PWM_CLK_EF_CTRL                    (0x0062  << 2)
#define X1_CLKCTRL_PCIE_CLK_CTRL0                     (0x0065  << 2)
#define X1_CLKCTRL_PCIE_CLK_CTRL1                     (0x0066  << 2)
#define X1_CLKCTRL_USB_CLK_CTRL                       (0x0067  << 2)
#define X1_CLKCTRL_25M_CLK_CTRL                       (0x0068  << 2)
#define X1_CLKCTRL_TIMESTAMP_CTRL                     (0x0100  << 2)
#define X1_CLKCTRL_TIMESTAMP_CTRL1                    (0x0101  << 2)
#define X1_CLKCTRL_TIMESTAMP_CTRL2                    (0x0103  << 2)
#define X1_CLKCTRL_TIMESTAMP_RD0                      (0x0104  << 2)
#define X1_CLKCTRL_TIMESTAMP_RD1                      (0x0105  << 2)
#define X1_CLKCTRL_TIMEBASE_CTRL0                     (0x0106  << 2)
#define X1_CLKCTRL_TIMEBASE_CTRL1                     (0x0107  << 2)
#define X1_CLKCTRL_EFUSE_CPU_CFG01                    (0x0120  << 2)
#define X1_CLKCTRL_EFUSE_CPU_CFG2                     (0x0121  << 2)
#define X1_CLKCTRL_EFUSE_N600_CFG01                   (0x0122  << 2)
#define X1_CLKCTRL_EFUSE_N600_CFG2                    (0x0123  << 2)
#define X1_CLKCTRL_EFUSE_LOCK                         (0x0126  << 2)
#define X1_CLKCTRL_SYS_OSC_CTRL                       (0x0131  << 2)
//========================================================================

// REG_BASE:  REGISTER_BASE_ADDR = 0xfe008000
// -----------------------------------------------
#define X1_ANACTRL_SYS0PLL_CTRL0                      ((0x0000  << 2) + 0x8000)
#define X1_ANACTRL_SYS0PLL_CTRL1                      ((0x0001  << 2) + 0x8000)
#define X1_ANACTRL_SYS0PLL_CTRL2                      ((0x0002  << 2) + 0x8000)
#define X1_ANACTRL_SYS0PLL_CTRL3                      ((0x0003  << 2) + 0x8000)
#define X1_ANACTRL_SYS1PLL_CTRL0                      ((0x0004  << 2) + 0x8000)
#define X1_ANACTRL_SYS1PLL_CTRL1                      ((0x0005  << 2) + 0x8000)
#define X1_ANACTRL_SYS1PLL_CTRL2                      ((0x0006  << 2) + 0x8000)
#define X1_ANACTRL_SYS1PLL_CTRL3                      ((0x0007  << 2) + 0x8000)
#define X1_ANACTRL_SYS0PLL_STS                        ((0x000a  << 2) + 0x8000)
#define X1_ANACTRL_SYS1PLL_STS                        ((0x000b  << 2) + 0x8000)
#define X1_ANACTRL_FIXPLL_CTRL0                       ((0x0010  << 2) + 0x8000)
#define X1_ANACTRL_FIXPLL_CTRL1                       ((0x0011  << 2) + 0x8000)
#define X1_ANACTRL_FIXPLL_CTRL2                       ((0x0012  << 2) + 0x8000)
#define X1_ANACTRL_FIXPLL_CTRL3                       ((0x0013  << 2) + 0x8000)
#define X1_ANACTRL_FIXPLL_STS                         ((0x0017  << 2) + 0x8000)
#define X1_ANACTRL_GP0PLL_CTRL0                       ((0x0020  << 2) + 0x8000)
#define X1_ANACTRL_GP0PLL_CTRL1                       ((0x0021  << 2) + 0x8000)
#define X1_ANACTRL_GP0PLL_CTRL2                       ((0x0022  << 2) + 0x8000)
#define X1_ANACTRL_GP0PLL_CTRL3                       ((0x0023  << 2) + 0x8000)
#define X1_ANACTRL_GP0PLL_STS                         ((0x0027  << 2) + 0x8000)
#define X1_ANACTRL_HIFI0PLL_CTRL0                     ((0x0040  << 2) + 0x8000)
#define X1_ANACTRL_HIFI0PLL_CTRL1                     ((0x0041  << 2) + 0x8000)
#define X1_ANACTRL_HIFI0PLL_CTRL2                     ((0x0042  << 2) + 0x8000)
#define X1_ANACTRL_HIFI0PLL_CTRL3                     ((0x0043  << 2) + 0x8000)
#define X1_ANACTRL_HIFI0PLL_STS                       ((0x0048  << 2) + 0x8000)
#define X1_ANACTRL_POR_CTRL                           ((0x00b6  << 2) + 0x8000)
#define X1_ANACTRL_LOCK_BIT                           ((0x00b8  << 2) + 0x8000)
#define X1_ANACTRL_PROT_BIT                           ((0x00b9  << 2) + 0x8000)
#define X1_ANACTRL_CHIP_TEST_STS                      ((0x00e0  << 2) + 0x8000)
//========================================================================

/*
 * CPU clock register offset
 * APB_BASE:  APB1_BASE_ADDR = 0xfe007400
 */

#include <dt-bindings/amlogic/clock/x1-clkc.h>
#define NR_CLKS				(CLKID_END_BASE)

#endif /* __X1_H */
