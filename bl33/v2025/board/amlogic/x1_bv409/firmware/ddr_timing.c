// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <asm/amlogic/arch/secure_apb.h>
#include <asm/amlogic/arch/timing.h>
#include <asm/amlogic/arch/ddr_define.h>

#define DDR_FUNC_CONFIG_DISABLE_DDR_DVFS_FUNCTION                   (0 + (1 << 19))
#define DDR_FUNC_CONFIG_ENABLE_PZQ_DET_DRAM_TYPE_RETURN                   (0 + (1 << 20))
#define DDR_FUNC_CONFIG_AUTO_DET_DQ_PINMUX_FUNCTION                   (0 + (1 << 21))
#define CONFIG_DRAM_MODE_FORCE_DISABLE_X8  0x81
#define CONFIG_DRAM_MODE_FORCE_ENABLE_X8  0x1
//bit 6 adc_channel bit 0-5 adc value,chan 3 value 8 is layer 2
#define DDR_ID_ACS_ADC   ((3 << 6) | (8))

#define DDR_RESV_CHECK_ID_ENABLE  0Xfe
#define SAR_ADC_DDR_ID_BASE   0
#define SAR_ADC_DDR_ID_STEP   80

#define DDR_TIMMING_OFFSET(X) (unsigned int)(unsigned long)(&(((ddr_set_ps0_only_t *)(0))->X))
#define DDR_TIMMING_OFFSET_SIZE(X) sizeof(((ddr_set_ps0_only_t *)(0))->X)
#define DDR_TIMMING_TUNE_TIMMING0(DDR_ID, PARA, VALUE) (DDR_ID, \
DDR_TIMMING_OFFSET(PARA), VALUE, DDR_TIMMING_OFFSET_SIZE(PARA), 0, \
DDR_RESV_CHECK_ID_ENABLE)
#define DDR_TIMMING_TUNE_TIMMING1(DDR_ID, PARA, VALUE) (DDR_ID, \
(sizeof(ddr_set_t) + (DDR_TIMMING_OFFSET(PARA))), VALUE, DDR_TIMMING_OFFSET_SIZE(PARA), \
0, DDR_RESV_CHECK_ID_ENABLE)

//bit24-31 define ID and size
#define DDR_ID_FROM_EFUSE  (0Xff000000)
#define DDR_ID_FROM_ADC  (0Xfe000000)
#define DDR_ID_FROM_GPIO_CONFIG1  (0Xfd000000)
#define DDR_ID_FROM_EFUSE_F  (0Xff << 0)
#define DDR_ID_FROM_ADC_F  (0Xfe << 0)
#define DDR_ID_FROM_GPIO_CONFIG1_F  (0Xfd << 0)
#define DDR_ID_FROM_ADC_MULT (0Xfc000000)
#define DDR_ID_FROM_ADC_MULT_F   (0Xfc << 0)
#define DDR_ID_START_MASK  (0XFFDDCCBB)

#define DDR_ADC_CH0 (0X0 << 5)
#define DDR_ADC_CH1 (0X1 << 5)
#define DDR_ADC_CH2 (0X2 << 5)
#define DDR_ADC_CH3 (0X3 << 5)
#define DDR_ADC_CH4 (0X4 << 5)

#define DDR_ADC_VALUE0 (0X0 << 0)
#define DDR_ADC_VALUE1 (0X1 << 0)
#define DDR_ADC_VALUE2 (0X2 << 0)
#define DDR_ADC_VALUE3 (0X3 << 0)
#define DDR_ADC_VALUE4 (0X4 << 0)
#define DDR_ADC_VALUE5 (0X5 << 0)
#define DDR_ADC_VALUE6 (0X6 << 0)
#define DDR_ADC_VALUE7 (0X7 << 0)
#define DDR_ADC_VALUE8 (0X8 << 0)
#define DDR_ADC_VALUE9 (0X9 << 0)
#define DDR_ADC_VALUE10 (0Xa << 0)
#define DDR_ADC_VALUE11 (0Xb << 0)
#define DDR_ADC_VALUE12 (0Xc << 0)
#define DDR_ADC_VALUE13 (0Xd << 0)
#define DDR_ADC_VALUE14 (0Xe << 0)
#define DDR_ADC_VALUE15 (0Xf << 0)
#define V0  (0X0 << 0)
#define V1  (0X1 << 0)
#define V2  (0X2 << 0)
#define V3  (0X3 << 0)
#define V4  (0X4 << 0)
#define V5  (0X5 << 0)
#define V6  (0X6 << 0)
#define V7  (0X7 << 0)
#define V8  (0X8 << 0)
#define V9  (0X9 << 0)
#define V10  (0Xa << 0)
#define V11  (0Xb << 0)
#define V12  (0Xc << 0)

#define VX  (0Xf << 0)

typedef struct ddr_para_data {
	// start from	DDR_ID_START_MASK,ddr_id;//bit0-23
	// ddr_id value,bit 24-31 ddr_id source  ,0xfe source
	// from adc ,0xfd source from gpio_default_config
	// reg_offset
	// //bit 0-15 parameter offset value,bit16-23 overrid
	// size,bit24-31 mux ddr_id source unsigned int
	// reg_offset; unsigned int	value;
	uint32_t	value : 16;             // bit0-15 only support data size =1byte
	// or 2bytes,no support int value
	uint32_t	reg_offset : 12;        // bit16-27
	uint32_t	data_size : 4;          // bit28-31 if data size =15,then
	// will mean DDR_ID start
} ddr_para_data_t;

typedef struct ddr_para_data_start {
	uint32_t	id_value : 24;          // bit0-23  efuse id or ddr id
	// uint32_t	id_adc_ch : 2;//bit6-7
	uint32_t	id_src_from : 8;        // bit24-31 ddr id from adc or gpio
} ddr_para_data_start_t;

#define DDR_TIMMING_TUNE_STRUCT_SIZE(a)  sizeof(a)

#define DDR_TIMMING_TUNE_TIMMING0_F(PARA, VALUE) ((DDR_TIMMING_OFFSET(PARA)) << 16) |\
((DDR_TIMMING_OFFSET_SIZE(PARA)) << 28) | VALUE
#define DDR_TIMMING_TUNE_TIMMING1_F(PARA, VALUE) ((sizeof(ddr_set_ps0_only_t) +\
DDR_TIMMING_OFFSET(PARA)) << 16) | ((DDR_TIMMING_OFFSET_SIZE(PARA)) << 28) | (VALUE)

#define DDR_TIMMING_TUNE_START(id_src_from, id_adc_ch, id_value) (id_src_from) |\
(id_adc_ch) | (id_value)
#define DDR_TIMMING_TUNE_ADC_MULT_START(id_value, ch0, ch1, ch2, ch3, ch4, ch5) (id_value) |\
(ch0) | ((ch1) << 4) | ((ch2) << 8) | ((ch3) << 12) | ((ch4) << 16) | ((ch5) << 20)
#define DDR_TIMMING_TUNE_STRUCT_SIZE(a)  sizeof(a)

#if 1
uint32_t __bl2_ddr_reg_data[] __attribute__ ((section(".ddr_2acs_data"))) = {
	DDR_ID_START_MASK,
	//DDR_TIMMING_TUNE_ADC_MULT_START(DDR_ID_FROM_ADC_MULT, V4, VX, VX, VX, VX, VX),
	//data start
	//DDR_TIMMING_TUNE_TIMMING0_F(cfg_board_common_setting.Is2Ttiming, CONFIG_USE_DDR_2T_MODE),
	//DDR_TIMMING_TUNE_TIMMING0_F(cfg_board_SI_setting_ps.DRAMFreq, 1320),
};

////_ddr_para_2nd_setting

uint32_t __ddr_parameter_reg_index[] __attribute__ ((section(".ddr_2acs_index"))) = {
	0,
};
#endif

//#define LPDDR4_SKT
#define DDR4_SKT 1
#define DDR3_SKT 1
//DDR3 bringup parameter basing on DDR4 TO DDR3 TRANS BOARD on BV409 socket board;

//#define ENABLE_DDR_WINDOW_FAST_BOOT 1

//#define LPDDR4_USE_2LAYER_BOARD 1
//default 1RANK 16BIT X2 DDR4
//#define USE_2RANK_16BIT_X2_DDR4 1 //use ap222 board
//#define ENABLE_8BIT_DDR4_CS0_CS1_SAME_PHASE 1 //t233
//#define ENABLE_8BIT_DDR3 1

ddr_set_ps0_only_t __ddr_setting[] __attribute__ ((section(".ddr_param"))) = {
#if DDR4_SKT
#define  CACLU_CLK_D4   1320
	{
		.cfg_board_common_setting.timming_magic = 0,
		.cfg_board_common_setting.timming_max_valid_configs = 1,
			//sizeof(ddr_set_t_default) / sizeof(ddr_set_ps0_only_t),
		.cfg_board_common_setting.timming_struct_version = 9636,
		.cfg_board_common_setting.timming_struct_org_size =
			sizeof(ddr_set_ps0_only_t),
		.cfg_board_common_setting.timming_struct_real_size = 0,
		.cfg_board_common_setting.fast_boot = {
			0, 0, 0, 0
		},
		.cfg_board_common_setting.ddr_func = DDR_FUNC_CONFIG_AUTO_DET_DQ_PINMUX_FUNCTION,
		.cfg_board_common_setting.board_id = CONFIG_BOARD_ID_MASK,
		.cfg_board_common_setting.DramType = CONFIG_DDR_TYPE_DDR4,
		.cfg_board_common_setting.enable_lpddr4x_mode = 0,
		.cfg_board_common_setting.dram_rank_config =
		//CONFIG_DDR0_16BIT_CH0,
		CONFIG_DDR0_32BIT_RANK0_CH0,
		//.cfg_board_common_setting.dram_cs0_base_add = 0,
		//.cfg_board_common_setting.dram_cs1_base_add = 0,
		.cfg_board_common_setting.dram_ch0_size_MB =
			(DRAM_SIZE_ID_256MBX4 << CONFIG_CS0_BYTE_01_SIZE_256_ID_OFFSET) +
			(DRAM_SIZE_ID_256MBX0 << CONFIG_CS0_BYTE_23_SIZE_256_ID_OFFSET) +
			(DRAM_SIZE_ID_256MBX0 << CONFIG_CS1_BYTE_01_SIZE_256_ID_OFFSET) +
			(DRAM_SIZE_ID_256MBX0 << CONFIG_CS1_BYTE_23_SIZE_256_ID_OFFSET),
		.cfg_board_common_setting.dram_ch1_size_MB =
			(DRAM_SIZE_ID_256MBX0 << CONFIG_CS0_BYTE_01_SIZE_256_ID_OFFSET) +
			(DRAM_SIZE_ID_256MBX0 << CONFIG_CS0_BYTE_23_SIZE_256_ID_OFFSET) +
			(DRAM_SIZE_ID_256MBX0 << CONFIG_CS1_BYTE_01_SIZE_256_ID_OFFSET) +
			(DRAM_SIZE_ID_256MBX0 << CONFIG_CS1_BYTE_23_SIZE_256_ID_OFFSET),
		.cfg_board_common_setting.DisabledDbyte[0] = 0xfc,
		//bit 0 -3 ch0 cs0 ,bit 4-7 ch0 cs1,
		.cfg_board_common_setting.DisabledDbyte[1] = 0xfc,
		//bit 0 -3 ch1 cs0 ,bit 4-7 ch1
		.cfg_board_common_setting.dram_x4x8x16_mode = CONFIG_DRAM_MODE_X16,
		.cfg_board_common_setting.Is2Ttiming = CONFIG_USE_DDR_2T_MODE,
		.cfg_board_common_setting.log_level = 0xff,
		//.cfg_board_common_setting.log_level = 4,
		//4,//LOG_LEVEL_BASIC,
		.cfg_board_common_setting.dbi_enable = DDR_WRITE_READ_DBI_DISABLE,
		.cfg_board_common_setting.pll_ssc_mode =
			(1 << 20) | (1 << 8) | (2 << 4) | 0,
		//center_ssc_1000ppm,//SSC_DISABLE,(1 << 20) | (0 << 8) | (2 << 4) | 0,
		.cfg_board_common_setting.org_tdqs2dq = 0,
		.cfg_board_common_setting.reserve1_test = {
			0
		},
		.cfg_board_common_setting.ddr_dmc_remap = {
			[0] = (4 | 5 << 5 | 7 << 10 | 8 << 15 | 9 << 20 | 10 << 25),
			[1] = (11 | 0 << 5 | 0 << 10 | 14 << 15 | 15 << 20 | 16 << 25),
			[2] = (17 | (18 << 5) | (19 << 10) | (20 << 15) | (21 << 20) | (22 << 25)),
			[3] = (23 | 24 << 5 | 25 << 10 | 26 << 15 | 27 << 20 | 28 << 25),
			[4] = (29 | 12 << 5 | 13 << 10 | 6 << 15 | 31 << 20 | 30 << 25),
		},

		.cfg_board_common_setting.ddr_dqs_swap = 0,

		.cfg_board_common_setting.ddr_vddee_setting = {
			0
		},
		.cfg_board_SI_setting_ps.DRAMFreq = CACLU_CLK_D4,
		.cfg_board_SI_setting_ps.training_SequenceCtrl = 0,
		.cfg_board_SI_setting_ps.dfi_odt_config = DDR_DRAM_ODT_W_CS0_ODT0,
		.cfg_board_SI_setting_ps.dfi_odt_config = DDR_DRAM_ODT_DDR4_PARK_ENABLE,
		.cfg_board_SI_setting_ps.vref_ac_permil = 0,
		.cfg_board_SI_setting_ps.vref_soc_data_permil = 0,
		.cfg_board_SI_setting_ps.vref_dram_data_permil = 0,
		.cfg_board_SI_setting_ps.max_core_timmming_frequency = 0,
		.cfg_board_common_setting.dbi_enable = 0x00000000,
		.cfg_board_common_setting.ddr_rfc_type = DDR_RFC_TYPE_DDR4_2Gbx4,
		.cfg_board_common_setting.pll_ssc_mode = 0x00000000,
//.cfg_board_common_setting.max_core_timmming_frequency=0x00000e10,// 3600,0x0000006a

//.cfg_board_common_setting.lpddr4_x8_mode=0x00000000,// 0,0x00000087
//.cfg_board_common_setting.tdqs2dq=0x00000000,// 0,0x0000008a
//.cfg_board_common_setting.dfe_offset_value=0x00000000,// 0,0x0000008e
//.cfg_board_common_setting.training_offset=0x00000000,// 0,0x0000008f

		.cfg_board_SI_setting_ps.clk_drv_ohm = 40,
		.cfg_board_SI_setting_ps.cs_drv_ohm = 40,
		.cfg_board_SI_setting_ps.ac_drv_ohm = 40,
		.cfg_board_SI_setting_ps.soc_data_drv_ohm_p = 40,
		.cfg_board_SI_setting_ps.soc_data_drv_ohm_n = 40,
		.cfg_board_SI_setting_ps.soc_data_odt_ohm_p = 60,
		.cfg_board_SI_setting_ps.soc_data_odt_ohm_n = 0,
		.cfg_board_SI_setting_ps.dram_data_drv_ohm = 34,
		.cfg_board_SI_setting_ps.dram_data_odt_ohm = 60,
		.cfg_board_SI_setting_ps.dram_data_wr_odt_ohm = 0,
		.cfg_board_SI_setting_ps.dram_ac_odt_ohm = 120,
		.cfg_board_SI_setting_ps.dram_drv_pull_up_cal_ohm =
			DDR_DRAM_LPDDR4_ODT_40_OHM,
		.cfg_board_SI_setting_ps.lpddr4_dram_vout_range =
			DDR_DRAM_LPDDR4_OUTPUT_1_3_VDDQ,

		.cfg_board_common_setting.ac_pinmux = {
			9, 5, 27, 11, 17, 8, 14, 4, 26, 6,
			21, 0, 1, 2, 13, 20, 22, 7, 18, 19, 23,
			12, 3, 10, 15, 16, 24, 25, 28, 29,
		},//for X1 ddr4 pinmux;

		.cfg_ddr_training_delay_ps.tx_offset[0] = (0 << 7) | 0x0,
		.cfg_ddr_training_delay_ps.rx_offset[0] = (0 << 7) | 0x0,
		.cfg_ddr_training_delay_ps.reserve_para[0] = (1 << 7) | 0x8,     //write dqs
		.cfg_ddr_training_delay_ps.reserve_para[1] = (1 << 7) | 0x8,     //write dqs
		.cfg_ddr_training_delay_ps.reserve_para[2] = (1 << 7) | 0x8,     //write dqs
		.cfg_ddr_training_delay_ps.reserve_para[3] = (1 << 7) | 0x8,     //write dqs
		.cfg_ddr_training_delay_ps.reserve_para[4] = (1 << 7) | 0x8,     //write dqs
		.cfg_ddr_training_delay_ps.reserve_para[5] = (1 << 7) | 0x8,     //write dqs
		.cfg_ddr_training_delay_ps.reserve_para[6] = (1 << 7) | 0x8,     //write dqs
		.cfg_ddr_training_delay_ps.reserve_para[7] = (1 << 7) | 0x8,     //write dqs
		.cfg_ddr_training_delay_ps.reserve_para[8 + 0] = (1 << 7) | 0x9,//read dqs
		.cfg_ddr_training_delay_ps.reserve_para[8 + 1] = (1 << 7) | 0x9,//read dqs
		.cfg_ddr_training_delay_ps.reserve_para[8 + 2] = (1 << 7) | 0x10,//read dqs
		.cfg_ddr_training_delay_ps.reserve_para[8 + 3] = (1 << 7) | 0x10,//read dqs
		.cfg_ddr_training_delay_ps.reserve_para[8 + 4] = (1 << 7) | 0x10,//read dqs
		.cfg_ddr_training_delay_ps.reserve_para[8 + 5] = (1 << 7) | 0x10,//read dqs
		.cfg_ddr_training_delay_ps.reserve_para[8 + 6] = (1 << 7) | 0x10,//read dqs
		.cfg_ddr_training_delay_ps.reserve_para[8 + 7] = (1 << 7) | 0x10,//read dqs

		#define  AC_OFFSET  (128)
		.cfg_ddr_training_delay_ps.ac_trace_delay[0] = 256 + AC_OFFSET,
		.cfg_ddr_training_delay_ps.ac_trace_delay[1] = 256 + AC_OFFSET,
		.cfg_ddr_training_delay_ps.ac_trace_delay[2] = 256 + AC_OFFSET,
		.cfg_ddr_training_delay_ps.ac_trace_delay[3] = 256 + AC_OFFSET,
		.cfg_ddr_training_delay_ps.ac_trace_delay[4] = 256 + AC_OFFSET,
		.cfg_ddr_training_delay_ps.ac_trace_delay[5] = 256 + AC_OFFSET,
		.cfg_ddr_training_delay_ps.ac_trace_delay[6] = 256 + AC_OFFSET,
		.cfg_ddr_training_delay_ps.ac_trace_delay[7] = 256 + AC_OFFSET,
		.cfg_ddr_training_delay_ps.ac_trace_delay[8] = 256 + AC_OFFSET,
		.cfg_ddr_training_delay_ps.ac_trace_delay[9] = 256 + AC_OFFSET,
		.cfg_ddr_training_delay_ps.ac_trace_delay[10] = 256 + AC_OFFSET,
		.cfg_ddr_training_delay_ps.ac_trace_delay[11] = 256 + AC_OFFSET,
		.cfg_ddr_training_delay_ps.ac_trace_delay[12] = 256 + AC_OFFSET,
		.cfg_ddr_training_delay_ps.ac_trace_delay[13] = 256 + AC_OFFSET,
		.cfg_ddr_training_delay_ps.ac_trace_delay[14] = 256 + AC_OFFSET,
		.cfg_ddr_training_delay_ps.ac_trace_delay[15] = 256 + AC_OFFSET,
		.cfg_ddr_training_delay_ps.ac_trace_delay[16] = 256 + AC_OFFSET,
		.cfg_ddr_training_delay_ps.ac_trace_delay[17] = 256 + AC_OFFSET,
		.cfg_ddr_training_delay_ps.ac_trace_delay[18] = 256 + AC_OFFSET,
		.cfg_ddr_training_delay_ps.ac_trace_delay[19] = 256 + AC_OFFSET,
		.cfg_ddr_training_delay_ps.ac_trace_delay[20] = 256 + AC_OFFSET,
		.cfg_ddr_training_delay_ps.ac_trace_delay[21] = 256 + AC_OFFSET,
		//cke 128 only 1UI margin
		.cfg_ddr_training_delay_ps.ac_trace_delay[22] = 256 + AC_OFFSET,
		.cfg_ddr_training_delay_ps.ac_trace_delay[23] = 256 + AC_OFFSET,
		.cfg_ddr_training_delay_ps.ac_trace_delay[24] = 256 + AC_OFFSET,
		.cfg_ddr_training_delay_ps.ac_trace_delay[25] = 256 + AC_OFFSET,
		.cfg_ddr_training_delay_ps.ac_trace_delay[26] = 256 + AC_OFFSET,
		.cfg_ddr_training_delay_ps.ac_trace_delay[27] = 256 + AC_OFFSET,
		.cfg_ddr_training_delay_ps.ac_trace_delay[28] = 256 + AC_OFFSET,
		.cfg_ddr_training_delay_ps.ac_trace_delay[29] = 256 + AC_OFFSET,

		.cfg_ddr_training_delay_ps.ac_trace_delay[16] = 128 + AC_OFFSET + 90,     // cs
		.cfg_ddr_training_delay_ps.ac_trace_delay[28] = 128 + AC_OFFSET,  //ck
		.cfg_ddr_training_delay_ps.ac_trace_delay[29] = 128 + AC_OFFSET,  //ck
		.cfg_ddr_training_delay_ps.ac_trace_delay[27] = 128 + AC_OFFSET,    //cke
		.cfg_ddr_training_delay_ps.ac_trace_delay[26] = 128 + AC_OFFSET,    //cke
		.cfg_ddr_training_delay_ps.ac_trace_delay[8] = 128 + AC_OFFSET,      //odt0

		.cfg_ddr_training_delay_ps.read_dq_delay_t[0] = 0x51,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[1] = 0x5a,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[2] = 0x47,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[3] = 0x63,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[4] = 0x54,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[5] = 0x58,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[6] = 0x69,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[7] = 0x4b,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[8] = 0x67,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[9] = 0x4d,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[10] = 0x55,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[11] = 0x3c,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[12] = 0x56,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[13] = 0x49,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[14] = 0x4f,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[15] = 0x52,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[16] = 0x44,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[17] = 0x56,

		.cfg_ddr_training_delay_ps.write_dqs_delay[0] = 0xac,
		.cfg_ddr_training_delay_ps.write_dqs_delay[1] = 0xb1,

		.cfg_ddr_training_delay_ps.wdq_delay[0] = 0xec,
		.cfg_ddr_training_delay_ps.wdq_delay[1] = 0xf2,
		.cfg_ddr_training_delay_ps.wdq_delay[2] = 0xea,
		.cfg_ddr_training_delay_ps.wdq_delay[3] = 0xf7,
		.cfg_ddr_training_delay_ps.wdq_delay[4] = 0xf3,
		.cfg_ddr_training_delay_ps.wdq_delay[5] = 0xf3,
		.cfg_ddr_training_delay_ps.wdq_delay[6] = 0xfc,
		.cfg_ddr_training_delay_ps.wdq_delay[7] = 0xef,
		.cfg_ddr_training_delay_ps.wdq_delay[8] = 0xfe,
		.cfg_ddr_training_delay_ps.wdq_delay[9] = 0xf3,
		.cfg_ddr_training_delay_ps.wdq_delay[10] = 0xf7,
		.cfg_ddr_training_delay_ps.wdq_delay[11] = 0xed,
		.cfg_ddr_training_delay_ps.wdq_delay[12] = 0xf8,
		.cfg_ddr_training_delay_ps.wdq_delay[13] = 0xf1,
		.cfg_ddr_training_delay_ps.wdq_delay[14] = 0xf5,
		.cfg_ddr_training_delay_ps.wdq_delay[15] = 0xf7,
		.cfg_ddr_training_delay_ps.wdq_delay[16] = 0xef,
		.cfg_ddr_training_delay_ps.wdq_delay[17] = 0xf6,

		.cfg_ddr_training_delay_ps.read_dqs_gate_delay[0] = 570,
		.cfg_ddr_training_delay_ps.read_dqs_gate_delay[1] = 570,

		.cfg_ddr_training_delay_ps.read_dqs_delay[0] = 112,
		.cfg_ddr_training_delay_ps.read_dqs_delay[1] = 112,

		.cfg_ddr_training_delay_ps.soc_bit_vref0[0] = 0x0,
		//0 for auto training
		.cfg_ddr_training_delay_ps.dram_vref[0] = 0x00000000,//1d

		.cfg_ddr_training_delay_ps.dca_dq_tx[0] = 0x00000002,
		.cfg_ddr_training_delay_ps.dca_dq_tx[1] = 0x00000002,
		.cfg_ddr_training_delay_ps.dca_dq_tx[2] = 0x00000002,
		.cfg_ddr_training_delay_ps.dca_dq_tx[3] = 0x00000002,
		.cfg_ddr_training_delay_ps.dca_dq_tx[4] = 0x00000002,
		.cfg_ddr_training_delay_ps.dca_dq_tx[5] = 0x00000002,
		.cfg_ddr_training_delay_ps.dca_dq_tx[6] = 0x00000002,
		.cfg_ddr_training_delay_ps.dca_dq_tx[7] = 0x00000002,
		.cfg_ddr_training_delay_ps.dfi_mrl[0] = 0x00000004,
		.cfg_ddr_training_delay_ps.dfi_mrl[1] = 0x00000004,
		.cfg_ddr_training_delay_ps.dfi_mrl[2] = 0x00000004,
		.cfg_ddr_training_delay_ps.dfi_mrl[3] = 0x00000004,
		.cfg_ddr_training_delay_ps.dfi_hwtmrl = 0x00000004,
		.cfg_ddr_training_delay_ps.csr_hwtctrl = 0x00000004,

		.cfg_ddr_training_delay_ps.pptdqscnttg0[0] = 0x00000026,
		.cfg_ddr_training_delay_ps.pptdqscnttg0[1] = 0x00000026,
		.cfg_ddr_training_delay_ps.pptdqscnttg0[2] = 0x00000026,
		.cfg_ddr_training_delay_ps.pptdqscnttg0[3] = 0x00000026,
},
//};
#endif

#if DDR3_SKT
#define  CACLU_CLK_D3	1056 //636 //1792//600 //1200 //(1900)// (1440)//(1008)
//ddr_set_ps0_only_t __attribute__ ((aligned(8))) ddr_set_t_default_ddr3 = {
	{
		.cfg_board_common_setting.timming_magic = 0,
		.cfg_board_common_setting.timming_max_valid_configs = 1,
		.cfg_board_common_setting.timming_struct_version = 9636,
		.cfg_board_common_setting.timming_struct_org_size =
			sizeof(ddr_set_ps0_only_t),
		.cfg_board_common_setting.timming_struct_real_size = 0,
#if ENABLE_DDR_WINDOW_FAST_BOOT
		.cfg_board_common_setting.fast_boot = {
			0x1, 0, 0x00, 0xc6
			//fast_boot[0]  enable
			//fast_boot[1]  margin bit 0-3 read 4-7 write
			//fast_boot[2]  offset bit 0-3 read 4-7 write
			//fast_boot[3]  test index
		},
#else
		.cfg_board_common_setting.fast_boot = {
			0, 0, 0, 0
		},
#endif
		.cfg_board_common_setting.ddr_func =
		DDR_FUNC_CONFIG_DISABLE_DDR_DVFS_FUNCTION,
		//DDR_FUNC_CONFIG_ENABLE_PZQ_DET_DRAM_TYPE_RETURN,
		.cfg_board_common_setting.board_id = CONFIG_BOARD_ID_MASK,
		.cfg_board_common_setting.DramType = CONFIG_DDR_TYPE_DDR3,
		.cfg_board_common_setting.enable_lpddr4x_mode = 0,
		.cfg_board_common_setting.dram_rank_config = CONFIG_DDR0_32BIT_RANK0_CH0,

		.cfg_board_common_setting.dram_ch0_size_MB = 0xffff,
		.cfg_board_common_setting.DisabledDbyte[0] = 0x00,
		//bit 0 -3 ch0 cs0 ,bit 4-7 ch0 cs1,
		//.cfg_board_common_setting.DisabledDbyte[1] = 0xf0,
		//bit 0 -3 ch1 cs0 ,bit 4-7 ch1
		.cfg_board_common_setting.dram_x4x8x16_mode = CONFIG_DRAM_MODE_X16,
		.cfg_board_common_setting.Is2Ttiming = CONFIG_USE_DDR_2T_MODE,
		.cfg_board_common_setting.log_level = 0xff,
		//.cfg_board_common_setting.log_level = 4,
		//4,//LOG_LEVEL_BASIC,
		.cfg_board_SI_setting_ps.DRAMFreq = CACLU_CLK_D3,
		.cfg_board_SI_setting_ps.training_SequenceCtrl = 0,
		.cfg_board_SI_setting_ps.dfi_odt_config = DDR_DRAM_ODT_W_CS0_ODT0,
		//.cfg_ddr_training_delay_ps.dfe_offset = 0,
		.cfg_board_SI_setting_ps.vref_ac_permil = 0,
		.cfg_board_SI_setting_ps.vref_soc_data_permil = 0,
		.cfg_board_SI_setting_ps.vref_dram_data_permil = 0,
		.cfg_board_SI_setting_ps.max_core_timmming_frequency = 0,//1320,//0,
		.cfg_board_common_setting.dbi_enable = 0x00000000,
		.cfg_board_common_setting.ddr_rfc_type = 0,
		.cfg_board_common_setting.pll_ssc_mode = 0x00000000,

		.cfg_board_SI_setting_ps.clk_drv_ohm = 40,
		.cfg_board_SI_setting_ps.cs_drv_ohm = 40,
		.cfg_board_SI_setting_ps.ac_drv_ohm = 40,
		.cfg_board_SI_setting_ps.soc_data_drv_ohm_p = 34,
		.cfg_board_SI_setting_ps.soc_data_drv_ohm_n = 34,
		.cfg_board_SI_setting_ps.soc_data_odt_ohm_p = 120,
		.cfg_board_SI_setting_ps.soc_data_odt_ohm_n = 120,
		.cfg_board_SI_setting_ps.dram_data_drv_ohm = 40,
		.cfg_board_SI_setting_ps.dram_data_odt_ohm = 60,
		.cfg_board_SI_setting_ps.dram_data_wr_odt_ohm = 0,
		.cfg_board_SI_setting_ps.dram_ac_odt_ohm = 120,
		.cfg_board_SI_setting_ps.dram_drv_pull_up_cal_ohm =
			DDR_DRAM_LPDDR4_ODT_40_OHM,
		.cfg_board_SI_setting_ps.lpddr4_dram_vout_range =
			DDR_DRAM_LPDDR4_OUTPUT_1_3_VDDQ,
		//.cfg_ddr_training_delay_ps.dfe_offset = 0,

		.cfg_board_common_setting.ddr_dq_remap = {
			2, 6, 4, 0, 3, 32, 1, 7, 5,
			33, 11, 8, 9, 14, 15, 13, 10, 12,
			16, 17, 18, 19, 20, 21, 22, 23, 34,
			24, 25, 26, 27, 28, 29, 30, 31, 35,
		},
		.cfg_board_common_setting.ac_pinmux = {
			14, 4 , 27, 13, 18, 5, 3, 0, 26, 2,
			16, 9, 11, 7, 6, 20, 22, 8, 14, 12, 23,
			10, 1, 21, 17, 19, 24, 25, 28, 29,
		},

		//.cfg_ddr_training_delay_ps.tx_offset[0] = (0 << 7) | 0x0,
		//.cfg_ddr_training_delay_ps.rx_offset[0] = (0 << 7) | 0x0,
		.cfg_ddr_training_delay_ps.reserve_para[0] = (1 << 7) | 0x8,     //write dqs
		.cfg_ddr_training_delay_ps.reserve_para[1] = (1 << 7) | 0x8,     //write dqs
		.cfg_ddr_training_delay_ps.reserve_para[2] = (1 << 7) | 0x8,     //write dqs
		.cfg_ddr_training_delay_ps.reserve_para[3] = (1 << 7) | 0x8,     //write dqs
		.cfg_ddr_training_delay_ps.reserve_para[8 + 0] = (1 << 7) | 0x6,//read dqs
		.cfg_ddr_training_delay_ps.reserve_para[8 + 1] = (1 << 7) | 0x6,//read dqs
		.cfg_ddr_training_delay_ps.reserve_para[8 + 2] = (1 << 7) | 0x6,//read dqs
		.cfg_ddr_training_delay_ps.reserve_para[8 + 3] = (1 << 7) | 0x6,//read dqs


		#define AC_OFF_D3  (128) //for sip should use AC_OFFSET 128,
		#define TDQS2DQ_D3  (0)
		#define WL0_D3  (0)

		.cfg_ddr_training_delay_ps.ac_trace_delay[0] = 256 + AC_OFF_D3,
		.cfg_ddr_training_delay_ps.ac_trace_delay[1] = 256 + AC_OFF_D3,
		.cfg_ddr_training_delay_ps.ac_trace_delay[2] = 256 + AC_OFF_D3,
		.cfg_ddr_training_delay_ps.ac_trace_delay[3] = 256 + AC_OFF_D3,
		.cfg_ddr_training_delay_ps.ac_trace_delay[4] = 256 + AC_OFF_D3,
		.cfg_ddr_training_delay_ps.ac_trace_delay[5] = 256 + AC_OFF_D3,
		.cfg_ddr_training_delay_ps.ac_trace_delay[6] = 256 + AC_OFF_D3,
		.cfg_ddr_training_delay_ps.ac_trace_delay[7] = 256 + AC_OFF_D3,
		.cfg_ddr_training_delay_ps.ac_trace_delay[8] = 256 + AC_OFF_D3,
		.cfg_ddr_training_delay_ps.ac_trace_delay[9] = 256 + AC_OFF_D3,
		.cfg_ddr_training_delay_ps.ac_trace_delay[10] = 128 + AC_OFF_D3,
		.cfg_ddr_training_delay_ps.ac_trace_delay[11] = 128 + AC_OFF_D3,
		.cfg_ddr_training_delay_ps.ac_trace_delay[12] = 256 + AC_OFF_D3,
		.cfg_ddr_training_delay_ps.ac_trace_delay[13] = 256 + AC_OFF_D3,
		.cfg_ddr_training_delay_ps.ac_trace_delay[14] = 256 + AC_OFF_D3,
		.cfg_ddr_training_delay_ps.ac_trace_delay[15] = 256 + AC_OFF_D3,
		.cfg_ddr_training_delay_ps.ac_trace_delay[16] = 256 + AC_OFF_D3,
		.cfg_ddr_training_delay_ps.ac_trace_delay[17] = 256 + AC_OFF_D3,
		.cfg_ddr_training_delay_ps.ac_trace_delay[18] = 256 + AC_OFF_D3,
		.cfg_ddr_training_delay_ps.ac_trace_delay[19] = 256 + AC_OFF_D3,
		.cfg_ddr_training_delay_ps.ac_trace_delay[20] = 256 + AC_OFF_D3,
		.cfg_ddr_training_delay_ps.ac_trace_delay[21] = 256 + AC_OFF_D3,
		.cfg_ddr_training_delay_ps.ac_trace_delay[22] = 256 + AC_OFF_D3,
		.cfg_ddr_training_delay_ps.ac_trace_delay[23] = 256 + AC_OFF_D3,
		.cfg_ddr_training_delay_ps.ac_trace_delay[24] = 256 + AC_OFF_D3,
		.cfg_ddr_training_delay_ps.ac_trace_delay[25] = 256 + AC_OFF_D3,
		.cfg_ddr_training_delay_ps.ac_trace_delay[26] = 128 + 32 + AC_OFF_D3,
		.cfg_ddr_training_delay_ps.ac_trace_delay[27] = 128 + AC_OFF_D3,
		.cfg_ddr_training_delay_ps.ac_trace_delay[28] = 128 + AC_OFF_D3,
		.cfg_ddr_training_delay_ps.ac_trace_delay[29] = 128 + AC_OFF_D3,

		.cfg_ddr_training_delay_ps.ac_trace_delay[16] = 320,//cs0
		.cfg_ddr_training_delay_ps.ac_trace_delay[8] = 256,//odt0
		.cfg_ddr_training_delay_ps.ac_trace_delay[26] = 256,//cke0

		.cfg_ddr_training_delay_ps.soc_bit_vref0[0] = 0x000000,
		//0 for auto training
		.cfg_ddr_training_delay_ps.dram_vref[0] = 0x00000000,

		.cfg_ddr_training_delay_ps.write_dqs_delay[0] = 177,
		.cfg_ddr_training_delay_ps.write_dqs_delay[1] = 177,
		.cfg_ddr_training_delay_ps.read_dqs_gate_delay[0] = 830,
		.cfg_ddr_training_delay_ps.read_dqs_gate_delay[1] = 830,
	},
//};
#endif
};
