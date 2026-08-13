// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <asm/amlogic/arch/secure_apb.h>
#include <asm/amlogic/arch/timing.h>
#include <asm/amlogic/arch/ddr_define.h>
#include <ddr_timing.c>

/* board clk defines */
#define CPU_CLK                                 2004

/* board vmin_value defines */
#define VMIN_FF_VALUE                           670
#define VMIN_TT_VALUE                           720
#define VMIN_SS_VALUE                           770
/* board vddee_value defines */
/* SS/TT/FF = 0.77V/0.74V/0.71V */
#define VDDEE_FF_VALUE                          0x1000c
#define VDDEE_TT_VALUE                          0x1000c
#define VDDEE_SS_VALUE                          0x1000c

board_clk_set_t __board_clk_setting
__attribute__ ((section(".clk_param"))) = {
	/* clock settings for bl2 */
	.cpu_clk	= CPU_CLK / 12 * 12,
#ifdef CONFIG_PXP_DDR
	.pxp = 1,
#else
	.pxp = 0,
#endif
	.low_console_baud = CONFIG_LOW_CONSOLE_BAUD,
};

#define VCCK_A_VAL                                AML_VDD_CPUA_INIT_VOLTAGE
#define VCCK_B_VAL                                AML_VDD_CPUB_INIT_VOLTAGE
#define VCCK_A_SRAM_VAL                          AML_VDD_CPUA_SRAM__INIT_VOLTAGE
#define VCCK_B_SRAM_VAL                          AML_VDD_CPUB_SRAM__INIT_VOLTAGE
#define VDDEE_VAL                               AML_VDDEE_INIT_VOLTAGE

/* VCCK_A PWM table */
#if (VCCK_A_VAL == 969)
#define VCCK_A_VAL_REG  0x0000001c
#elif (VCCK_A_VAL == 959)
#define VCCK_A_VAL_REG  0x0001001b
#elif (VCCK_A_VAL == 949)
#define VCCK_A_VAL_REG  0x0002001a
#elif (VCCK_A_VAL == 939)
#define VCCK_A_VAL_REG  0x00030019
#elif (VCCK_A_VAL == 929)
#define VCCK_A_VAL_REG  0x00040018
#elif (VCCK_A_VAL == 919)
#define VCCK_A_VAL_REG  0x00050017
#elif (VCCK_A_VAL == 909)
#define VCCK_A_VAL_REG  0x00060016
#elif (VCCK_A_VAL == 899)
#define VCCK_A_VAL_REG  0x00070015
#elif (VCCK_A_VAL == 889)
#define VCCK_A_VAL_REG  0x00080014
#elif (VCCK_A_VAL == 879)
#define VCCK_A_VAL_REG  0x00090013
#elif (VCCK_A_VAL == 869)
#define VCCK_A_VAL_REG  0x000a0012
#elif (VCCK_A_VAL == 859)
#define VCCK_A_VAL_REG  0x000b0011
#elif (VCCK_A_VAL == 849)
#define VCCK_A_VAL_REG  0x000c0010
#elif (VCCK_A_VAL == 839)
#define VCCK_A_VAL_REG  0x000d000f
#elif (VCCK_A_VAL == 829)
#define VCCK_A_VAL_REG  0x000e000e
#elif (VCCK_A_VAL == 819)
#define VCCK_A_VAL_REG  0x000f000d
#elif (VCCK_A_VAL == 809)
#define VCCK_A_VAL_REG  0x0010000c
#elif (VCCK_A_VAL == 799)
#define VCCK_A_VAL_REG  0x0011000b
#elif (VCCK_A_VAL == 789)
#define VCCK_A_VAL_REG  0x0012000a
#elif (VCCK_A_VAL == 779)
#define VCCK_A_VAL_REG  0x00130009
#elif (VCCK_A_VAL == 769)
#define VCCK_A_VAL_REG  0x00140008
#elif (VCCK_A_VAL == 759)
#define VCCK_A_VAL_REG  0x00150007
#elif (VCCK_A_VAL == 749)
#define VCCK_A_VAL_REG  0x00160006
#elif (VCCK_A_VAL == 739)
#define VCCK_A_VAL_REG  0x00170005
#elif (VCCK_A_VAL == 729)
#define VCCK_A_VAL_REG  0x00180004
#elif (VCCK_A_VAL == 719)
#define VCCK_A_VAL_REG  0x00190003
#elif (VCCK_A_VAL == 709)
#define VCCK_A_VAL_REG  0x001a0002
#elif (VCCK_A_VAL == 699)
#define VCCK_A_VAL_REG  0x001b0001
#elif (VCCK_A_VAL == 689)
#define VCCK_A_VAL_REG  0x001c0000
#else
#error "VCCK val out of range\n"
#endif

/* VCCK_B PWM table */
#if (VCCK_B_VAL == 969)
#define VCCK_B_VAL_REG  0x0000001c
#elif (VCCK_B_VAL == 959)
#define VCCK_B_VAL_REG  0x0001001b
#elif (VCCK_B_VAL == 949)
#define VCCK_B_VAL_REG  0x0002001a
#elif (VCCK_B_VAL == 939)
#define VCCK_B_VAL_REG  0x00030019
#elif (VCCK_B_VAL == 929)
#define VCCK_B_VAL_REG  0x00040018
#elif (VCCK_B_VAL == 919)
#define VCCK_B_VAL_REG  0x00050017
#elif (VCCK_B_VAL == 909)
#define VCCK_B_VAL_REG  0x00060016
#elif (VCCK_B_VAL == 899)
#define VCCK_B_VAL_REG  0x00070015
#elif (VCCK_B_VAL == 889)
#define VCCK_B_VAL_REG  0x00080014
#elif (VCCK_B_VAL == 879)
#define VCCK_B_VAL_REG  0x00090013
#elif (VCCK_B_VAL == 869)
#define VCCK_B_VAL_REG  0x000a0012
#elif (VCCK_B_VAL == 859)
#define VCCK_B_VAL_REG  0x000b0011
#elif (VCCK_B_VAL == 849)
#define VCCK_B_VAL_REG  0x000c0010
#elif (VCCK_B_VAL == 839)
#define VCCK_B_VAL_REG  0x000d000f
#elif (VCCK_B_VAL == 829)
#define VCCK_B_VAL_REG  0x000e000e
#elif (VCCK_B_VAL == 819)
#define VCCK_B_VAL_REG  0x000f000d
#elif (VCCK_B_VAL == 809)
#define VCCK_B_VAL_REG  0x0010000c
#elif (VCCK_B_VAL == 799)
#define VCCK_B_VAL_REG  0x0011000b
#elif (VCCK_B_VAL == 789)
#define VCCK_B_VAL_REG  0x0012000a
#elif (VCCK_B_VAL == 779)
#define VCCK_B_VAL_REG  0x00130009
#elif (VCCK_B_VAL == 769)
#define VCCK_B_VAL_REG  0x00140008
#elif (VCCK_B_VAL == 759)
#define VCCK_B_VAL_REG  0x00150007
#elif (VCCK_B_VAL == 749)
#define VCCK_B_VAL_REG  0x00160006
#elif (VCCK_B_VAL == 739)
#define VCCK_B_VAL_REG  0x00170005
#elif (VCCK_B_VAL == 729)
#define VCCK_B_VAL_REG  0x00180004
#elif (VCCK_B_VAL == 719)
#define VCCK_B_VAL_REG  0x00190003
#elif (VCCK_B_VAL == 709)
#define VCCK_B_VAL_REG  0x001a0002
#elif (VCCK_B_VAL == 699)
#define VCCK_B_VAL_REG  0x001b0001
#elif (VCCK_B_VAL == 689)
#define VCCK_B_VAL_REG  0x001c0000
#else
#error "VCCK val out of range\n"
#endif

/* VCCK_A_SRAM PWM table  */
#if (VCCK_A_SRAM_VAL == 969)
#define VCCK_A_SRAM_VAL_REG  0x0000001c
#elif (VCCK_A_SRAM_VAL == 959)
#define VCCK_A_SRAM_VAL_REG  0x0001001b
#elif (VCCK_A_SRAM_VAL == 949)
#define VCCK_A_SRAM_VAL_REG  0x0002001a
#elif (VCCK_A_SRAM_VAL == 939)
#define VCCK_A_SRAM_VAL_REG  0x00030019
#elif (VCCK_A_SRAM_VAL == 929)
#define VCCK_A_SRAM_VAL_REG  0x00040018
#elif (VCCK_A_SRAM_VAL == 919)
#define VCCK_A_SRAM_VAL_REG  0x00050017
#elif (VCCK_A_SRAM_VAL == 909)
#define VCCK_A_SRAM_VAL_REG  0x00060016
#elif (VCCK_A_SRAM_VAL == 899)
#define VCCK_A_SRAM_VAL_REG  0x00070015
#elif (VCCK_A_SRAM_VAL == 889)
#define VCCK_A_SRAM_VAL_REG  0x00080014
#elif (VCCK_A_SRAM_VAL == 879)
#define VCCK_A_SRAM_VAL_REG  0x00090013
#elif (VCCK_A_SRAM_VAL == 869)
#define VCCK_A_SRAM_VAL_REG  0x000a0012
#elif (VCCK_A_SRAM_VAL == 859)
#define VCCK_A_SRAM_VAL_REG  0x000b0011
#elif (VCCK_A_SRAM_VAL == 849)
#define VCCK_A_SRAM_VAL_REG  0x000c0010
#elif (VCCK_A_SRAM_VAL == 839)
#define VCCK_A_SRAM_VAL_REG  0x000d000f
#elif (VCCK_A_SRAM_VAL == 829)
#define VCCK_A_SRAM_VAL_REG  0x000e000e
#elif (VCCK_A_SRAM_VAL == 819)
#define VCCK_A_SRAM_VAL_REG  0x000f000d
#elif (VCCK_A_SRAM_VAL == 809)
#define VCCK_A_SRAM_VAL_REG  0x0010000c
#elif (VCCK_A_SRAM_VAL == 799)
#define VCCK_A_SRAM_VAL_REG  0x0011000b
#elif (VCCK_A_SRAM_VAL == 789)
#define VCCK_A_SRAM_VAL_REG  0x0012000a
#elif (VCCK_A_SRAM_VAL == 779)
#define VCCK_A_SRAM_VAL_REG  0x00130009
#elif (VCCK_A_SRAM_VAL == 769)
#define VCCK_A_SRAM_VAL_REG  0x00140008
#elif (VCCK_A_SRAM_VAL == 759)
#define VCCK_A_SRAM_VAL_REG  0x00150007
#elif (VCCK_A_SRAM_VAL == 749)
#define VCCK_A_SRAM_VAL_REG  0x00160006
#elif (VCCK_A_SRAM_VAL == 739)
#define VCCK_A_SRAM_VAL_REG  0x00170005
#elif (VCCK_A_SRAM_VAL == 729)
#define VCCK_A_SRAM_VAL_REG  0x00180004
#elif (VCCK_A_SRAM_VAL == 719)
#define VCCK_A_SRAM_VAL_REG  0x00190003
#elif (VCCK_A_SRAM_VAL == 709)
#define VCCK_A_SRAM_VAL_REG  0x001a0002
#elif (VCCK_A_SRAM_VAL == 699)
#define VCCK_A_SRAM_VAL_REG  0x001b0001
#elif (VCCK_A_SRAM_VAL == 689)
#define VCCK_A_SRAM_VAL_REG  0x001c0000
#else
#error "VCCK val out of range\n"
#endif

/* VCCK_A_SRAM PWM table */
#if (VCCK_B_SRAM_VAL == 969)
#define VCCK_B_SRAM_VAL_REG  0x0000001c
#elif (VCCK_B_SRAM_VAL == 959)
#define VCCK_B_SRAM_VAL_REG  0x0001001b
#elif (VCCK_B_SRAM_VAL == 949)
#define VCCK_B_SRAM_VAL_REG  0x0002001a
#elif (VCCK_B_SRAM_VAL == 939)
#define VCCK_B_SRAM_VAL_REG  0x00030019
#elif (VCCK_B_SRAM_VAL == 929)
#define VCCK_B_SRAM_VAL_REG  0x00040018
#elif (VCCK_B_SRAM_VAL == 919)
#define VCCK_B_SRAM_VAL_REG  0x00050017
#elif (VCCK_B_SRAM_VAL == 909)
#define VCCK_B_SRAM_VAL_REG  0x00060016
#elif (VCCK_B_SRAM_VAL == 899)
#define VCCK_B_SRAM_VAL_REG  0x00070015
#elif (VCCK_B_SRAM_VAL == 889)
#define VCCK_B_SRAM_VAL_REG  0x00080014
#elif (VCCK_B_SRAM_VAL == 879)
#define VCCK_B_SRAM_VAL_REG  0x00090013
#elif (VCCK_B_SRAM_VAL == 869)
#define VCCK_B_SRAM_VAL_REG  0x000a0012
#elif (VCCK_B_SRAM_VAL == 859)
#define VCCK_B_SRAM_VAL_REG  0x000b0011
#elif (VCCK_B_SRAM_VAL == 849)
#define VCCK_B_SRAM_VAL_REG  0x000c0010
#elif (VCCK_B_SRAM_VAL == 839)
#define VCCK_B_SRAM_VAL_REG  0x000d000f
#elif (VCCK_B_SRAM_VAL == 829)
#define VCCK_B_SRAM_VAL_REG  0x000e000e
#elif (VCCK_B_SRAM_VAL == 819)
#define VCCK_B_SRAM_VAL_REG  0x000f000d
#elif (VCCK_B_SRAM_VAL == 809)
#define VCCK_B_SRAM_VAL_REG  0x0010000c
#elif (VCCK_B_SRAM_VAL == 799)
#define VCCK_B_SRAM_VAL_REG  0x0011000b
#elif (VCCK_B_SRAM_VAL == 789)
#define VCCK_B_SRAM_VAL_REG  0x0012000a
#elif (VCCK_B_SRAM_VAL == 779)
#define VCCK_B_SRAM_VAL_REG  0x00130009
#elif (VCCK_B_SRAM_VAL == 769)
#define VCCK_B_SRAM_VAL_REG  0x00140008
#elif (VCCK_B_SRAM_VAL == 759)
#define VCCK_B_SRAM_VAL_REG  0x00150007
#elif (VCCK_B_SRAM_VAL == 749)
#define VCCK_B_SRAM_VAL_REG  0x00160006
#elif (VCCK_B_SRAM_VAL == 739)
#define VCCK_B_SRAM_VAL_REG  0x00170005
#elif (VCCK_B_SRAM_VAL == 729)
#define VCCK_B_SRAM_VAL_REG  0x00180004
#elif (VCCK_B_SRAM_VAL == 719)
#define VCCK_B_SRAM_VAL_REG  0x00190003
#elif (VCCK_B_SRAM_VAL == 709)
#define VCCK_B_SRAM_VAL_REG  0x001a0002
#elif (VCCK_B_SRAM_VAL == 699)
#define VCCK_B_SRAM_VAL_REG  0x001b0001
#elif (VCCK_B_SRAM_VAL == 689)
#define VCCK_B_SRAM_VAL_REG  0x001c0000
#else
#error "VCCK val out of range\n"
#endif

/* VDDEE_VAL_REG GPU DDR One-Way Power*/
#if    (VDDEE_VAL == 690)
#define VDDEE_VAL_REG   0xd0000
#elif (VDDEE_VAL == 700)
#define VDDEE_VAL_REG   0xc0001
#elif (VDDEE_VAL == 710)
#define VDDEE_VAL_REG   0xb0002
#elif (VDDEE_VAL == 720)
#define VDDEE_VAL_REG   0xa0003
#elif (VDDEE_VAL == 730)
#define VDDEE_VAL_REG   0x90004
#elif (VDDEE_VAL == 740)
#define VDDEE_VAL_REG   0x80005
#elif (VDDEE_VAL == 750)
#define VDDEE_VAL_REG   0x70006
#elif (VDDEE_VAL == 760)
#define VDDEE_VAL_REG   0x60007
#elif (VDDEE_VAL == 770)
#define VDDEE_VAL_REG   0x50008
#elif (VDDEE_VAL == 780)
#define VDDEE_VAL_REG   0x40009
#elif (VDDEE_VAL == 790)
#define VDDEE_VAL_REG   0x3000a
#elif (VDDEE_VAL == 800)
#define VDDEE_VAL_REG   0x2000b
#elif (VDDEE_VAL == 810)
#define VDDEE_VAL_REG   0x1000c
#elif (VDDEE_VAL == 820)
#define VDDEE_VAL_REG   0x0000d
#else
#error "VDDEE val out of range\n"
#endif

bl2_reg_t __bl2_reg[] __attribute__ ((section(".generic_param"))) = {
	//need fine tune
	{ 0, 0, 0xffffffff, 0, 0, 0 },
};

/* gpio/pinmux/pwm init */
register_ops_t __bl2_ops_reg[MAX_REG_OPS_ENTRIES]
__attribute__ ((section(".misc_param"))) = {
	/* config vmin_ft value */
	{ 0, VMIN_SS_VALUE, 0xffffffff, 0, BL2_INIT_STAGE_VMIN_FLAG_1, 0 },
	{ 0, VMIN_TT_VALUE, 0xffffffff, 0, BL2_INIT_STAGE_VMIN_FLAG_2, 0 },
	{ 0, VMIN_FF_VALUE, 0xffffffff, 0, BL2_INIT_STAGE_VMIN_FLAG_3, 0 },
	/* config vcck pwm_a - pwm_e*/
#ifdef CONFIG_PDVFS_ENABLE
	{ AO_PWM_PWM_E, VDDEE_SS_VALUE, 0xffffffff, 0, BL2_INIT_STAGE_VDDCORE_CONFIG_1, 0 },
	{ AO_PWM_PWM_E, VDDEE_TT_VALUE, 0xffffffff, 0, BL2_INIT_STAGE_VDDCORE_CONFIG_2, 0 },
	{ AO_PWM_PWM_E, VDDEE_FF_VALUE, 0xffffffff, 0, BL2_INIT_STAGE_VDDCORE_CONFIG_3, 0 },
#else
	{ AO_PWM_PWM_E,		   VDDEE_VAL_REG, 0xffffffff, 0, 0, 0},
#endif
	{ AO_PWM_PWM_D,		   VCCK_A_VAL_REG,  0xffffffff, 0, 0, 0 },
	{ AO_PWM_PWM_A,		   VCCK_B_VAL_REG,  0xffffffff, 0, 0, 0 },
	{ AO_PWM_PWM_C,		   VCCK_A_SRAM_VAL_REG,  0xffffffff, 0, 0, 0 },
	{ AO_PWM_PWM_B,		   VCCK_B_SRAM_VAL_REG,  0xffffffff, 0, 0, 0 },
	/* config vcck pwm_a - pwm_d*/
	{ AO_PWM_MISC_REG_A,	   (0x1 << 0),	  (0x1 << 0), 0, 0, 0 },
	{ AO_PWM_MISC_REG_B,	   (0x1 << 0),	  (0x1 << 0), 0, 0, 0 },
	{ AO_PWM_MISC_REG_C,	   (0x1 << 0),	  (0x1 << 0), 0, 0, 0 },
	{ AO_PWM_MISC_REG_D,	   (0x1 << 0),	  (0x1 << 0), 0, 0, 0 },
	{ AO_PWM_MISC_REG_E,	   (0x1 << 0),	  (0x1 << 0), 0, 0, 0 },
	/* set pwm a -pwm e clock rate to 24M, enable them */
	{ AO_CLKCTRL_PWM_CLK_A_CTRL, (0x1 << 8), 0xffffffff, 0, 0, 0 },
	{ AO_CLKCTRL_PWM_CLK_B_CTRL, (0x1 << 8), 0xffffffff, 0, 0, 0 },
	{ AO_CLKCTRL_PWM_CLK_C_CTRL, (0x1 << 8), 0xffffffff, 0, 0, 0 },
	{ AO_CLKCTRL_PWM_CLK_D_CTRL, (0x1 << 8), 0xffffffff, 0, 0, 0 },
	{ AO_CLKCTRL_PWM_CLK_E_CTRL, (0x1 << 8), 0xffffffff, 0, 0, 0 },
	/* set GPIOAO_0 GPIOAO_3 mux to pwm_a - pwm_e */
	{ PADCTRL_PIN_MUX_REGAO_0,	   (0x11111 << 0),	  (0xfffff << 0), 0, 0, 0 },
#ifdef CONFIG_NOVERBOSE_BUILD
	 { UART_B_WFIFO, 0, 0xffffffff, 0, 1, 0 },
#endif
};

#define DEV_FIP_SIZE 0x300000
#define DDR_FIP_SIZE 0x40000
/* for all the storage parameter */
#ifdef CONFIG_MTD_SPI_NAND
/* for spinand storage parameter */
storage_parameter_t __store_para __attribute__ ((section(".store_param"))) = {
	.common				= {
		.version = 0x01,
		.device_fip_container_size = CONFIG_TPL_SIZE_PER_COPY,
		.device_fip_container_copies = ((CONFIG_NAND_TPL_COPY_NUM) |
						   (CONFIG_BL2_COPY_NUM << 16) |
						   (BOARD_CONFIG_BL2_LAYOUT_TYPE << 24)),
		.ddr_fip_container_size = BOOTLOADER_DDR_FIP_SIZE,
	},
	.nand				= {
		.version = 0x01,
		.bbt_pages = 1, // TODO: BL2E BBT
		.bbt_start_block = 20,
		.discrete_mode = 1,
		.setup_data.spi_nand_page_size = 2048,
		.reserved.spi_nand_planes_per_lun = 1,
		.reserved_area_blk_cnt = MTD_RSV_BLOCK_CNT,
		.page_per_block = 64,
		.use_param_page_list = 0,
	},
};
#else
storage_parameter_t __store_para __attribute__ ((section(".store_param"))) = {
	.common					= {
		.version			= 0x01,
		.device_fip_container_size	= CONFIG_TPL_SIZE_PER_COPY,
		.device_fip_container_copies    = ((CONFIG_NAND_TPL_COPY_NUM) |
						   (CONFIG_BL2_COPY_NUM << 16) |
						   (BOARD_CONFIG_BL2_LAYOUT_TYPE << 24)),
		.ddr_fip_container_size		= BOOTLOADER_DDR_FIP_SIZE,
	},
	.nand					= {
		.version			= 0x01,
		.bbt_pages			= 0x1,
		.bbt_start_block		= 20,
		.discrete_mode			= 1,
		.setup_data.nand_setup_data = (2 << 20) |		    \
						  (0 << 19) |			  \
						  (1 << 17) |			  \
						  (1 << 14) |			  \
						  (0 << 13) |			  \
						  (64 << 6) |			  \
						  (8 << 0),
		.reserved_area_blk_cnt		= MTD_RSV_BLOCK_CNT,
		.page_per_block			= 64,
		.use_param_page_list		= 0,
	},
};
#endif
