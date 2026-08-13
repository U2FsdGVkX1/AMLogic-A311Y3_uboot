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

//#define LPDDR4_SKT 1
#define DDR4_SKT 1
#define DDR3_SKT 1
#define ENABLE_NTC_256M_DDR3L 1
//#define ENABLE_NTC_512M_DDR3 1

//#define ENABLE_DDR_WINDOW_FAST_BOOT 1

//#define LPDDR4_USE_2LAYER_BOARD 1
//default 1RANK 16BIT X2 DDR4
//#define USE_2RANK_16BIT_X2_DDR4 1 //use ap222 board
//#define ENABLE_8BIT_DDR4_CS0_CS1_SAME_PHASE 1 //t233
//#define ENABLE_8BIT_DDR3 1

ddr_set_ps0_only_t __ddr_setting[] __attribute__ ((section(".ddr_param"))) = {
#if 0//LPDDR4_SKT
#define  CACLU_CLK_LP4   1584 //1792//600 //1200 //(1900)// (1440)//(1008)
//ddr_set_ps0_only_t __attribute__ ((aligned(8))) ddr_set_t_default_lp4 = {
	{
		.cfg_board_common_setting.timming_magic = 0,
		.cfg_board_common_setting.timming_max_valid_configs = 1,
		.cfg_board_common_setting.timming_struct_version = 0x2384,
		.cfg_board_common_setting.timming_struct_org_size =
			sizeof(ddr_set_ps0_only_t),
		.cfg_board_common_setting.timming_struct_real_size = 0,
#if ENABLE_DDR_WINDOW_FAST_BOOT
		.cfg_board_common_setting.fast_boot = {
			0x1, 0, 0, 0xc6
		},
#else
		.cfg_board_common_setting.fast_boot = {
			0x0, 0, 0, 0
		},
#endif
		.cfg_board_common_setting.ddr_func = DDR_FUNC_CONFIG_DFE_FUNCTION |
		DDR_FUNC_CONFIG_DISABLE_DDR_DVFS_FUNCTION,
		//DDR_FUNC_CONFIG_ENABLE_PZQ_DET_DRAM_TYPE_RETURN,
		.cfg_board_common_setting.board_id = CONFIG_BOARD_ID_MASK,
		.cfg_board_common_setting.DramType = CONFIG_DDR_TYPE_LPDDR4,
		.cfg_board_common_setting.enable_lpddr4x_mode = 0,
		//0  force lp4x   1 force lp4
		//2 auto 4x use nn 4 use pn drivere
		//3 auto + force 4 4x both use nn driver
		.cfg_board_common_setting.dram_rank_config = CONFIG_DDR0_32BIT_RANK0_CH0,
		//.cfg_board_common_setting.dram_ch0_size_MB =
		//	(DRAM_SIZE_ID_256MBX2 << CONFIG_CS0_BYTE_01_SIZE_256_ID_OFFSET) +
		//	(DRAM_SIZE_ID_256MBX2 << CONFIG_CS0_BYTE_23_SIZE_256_ID_OFFSET) +
		//	(DRAM_SIZE_ID_256MBX0 << CONFIG_CS1_BYTE_01_SIZE_256_ID_OFFSET) +
		//	(DRAM_SIZE_ID_256MBX0 << CONFIG_CS1_BYTE_23_SIZE_256_ID_OFFSET),
		//.cfg_board_common_setting.dram_ch1_size_MB =
		//	(DRAM_SIZE_ID_256MBX2 << CONFIG_CS0_BYTE_01_SIZE_256_ID_OFFSET) +
		//	(DRAM_SIZE_ID_256MBX2 << CONFIG_CS0_BYTE_23_SIZE_256_ID_OFFSET) +
		//	(DRAM_SIZE_ID_256MBX0 << CONFIG_CS1_BYTE_01_SIZE_256_ID_OFFSET) +
		//	(DRAM_SIZE_ID_256MBX0 << CONFIG_CS1_BYTE_23_SIZE_256_ID_OFFSET),
		.cfg_board_common_setting.dram_ch0_size_MB = 0xffff,
		.cfg_board_common_setting.DisabledDbyte[0] = 0x00,
		//bit 0 -3 ch0 cs0 ,bit 4-7 ch0 cs1,
		.cfg_board_common_setting.DisabledDbyte[1] = 0xf0,
		//bit 0 -3 ch1 cs0 ,bit 4-7 ch1
		.cfg_board_common_setting.dram_x4x8x16_mode = CONFIG_DRAM_MODE_X16,
		.cfg_board_common_setting.Is2Ttiming = CONFIG_USE_DDR_2T_MODE,
		.cfg_board_common_setting.log_level = 0xff,
		//.cfg_board_common_setting.log_level = 4,
		//4,//LOG_LEVEL_BASIC,
		.cfg_board_common_setting.dbi_enable = 0,
		.cfg_board_common_setting.org_tdqs2dq = 0,
		.cfg_board_common_setting.reserve1_test = {
			0
		},
		.cfg_board_common_setting.ddr_vddee_setting = {
			0
		},
		.cfg_board_SI_setting_ps.DRAMFreq = CACLU_CLK_LP4,
		.cfg_board_SI_setting_ps.training_SequenceCtrl = 0,
		.cfg_board_SI_setting_ps.dfi_odt_config = DDR_DRAM_ODT_W_CS0_ODT0,
		.cfg_board_SI_setting_ps.clk_drv_ohm = 40,
		.cfg_board_SI_setting_ps.cs_drv_ohm = 40,
		.cfg_board_SI_setting_ps.ac_drv_ohm = 40,
		.cfg_board_SI_setting_ps.soc_data_drv_ohm_p = 34,
		.cfg_board_SI_setting_ps.soc_data_drv_ohm_n = 34,
		.cfg_board_SI_setting_ps.soc_data_odt_ohm_p = 0,
		.cfg_board_SI_setting_ps.soc_data_odt_ohm_n = 80, //60,
		.cfg_board_SI_setting_ps.dram_data_drv_ohm = 40,
		.cfg_board_SI_setting_ps.dram_data_odt_ohm = 48,
		.cfg_board_SI_setting_ps.dram_data_wr_odt_ohm = 0,
		.cfg_board_SI_setting_ps.dram_ac_odt_ohm = 120,//240,//120,
#ifdef LPDDR4_USE_2LAYER_BOARD
		.cfg_board_SI_setting_ps.soc_data_odt_ohm_n = 120,
#endif
		.cfg_board_SI_setting_ps.dram_drv_pull_up_cal_ohm =
			DDR_DRAM_LPDDR4_ODT_40_OHM,
		.cfg_board_SI_setting_ps.lpddr4_dram_vout_range =
			1,//DDR_DRAM_LPDDR4_OUTPUT_1_3_VDDQ,
		.cfg_board_SI_setting_ps.vref_ac_permil = 375,//420,
		.cfg_board_SI_setting_ps.vref_soc_data_permil = 0,
		.cfg_board_SI_setting_ps.vref_dram_data_permil = 0,
		.cfg_board_SI_setting_ps.max_core_timmming_frequency = 0,

		.cfg_board_common_setting.ddr_rfc_type = 0, // 13,
		.cfg_board_common_setting.pll_ssc_mode = 0x00000000,    // 0,0x00000044
		.cfg_board_common_setting.ac_pinmux = {
			6,	2,	0,	3,	4,	5,	0,	1,	0,
			0,	7,	8,	9,	28,	29,
			15,	0,	13,	0,	17,	16,	11,	0,	10,
			12,	14,	18,	19,	26,	27,
		},

		//.cfg_ddr_training_delay_ps.rx_offset[0] = (1 << 7) | 0x10,
		//.cfg_ddr_training_delay_ps.tx_offset[0] = (1 << 7) | 0x8,
		.cfg_ddr_training_delay_ps.dac_offset[0] = (1 << 7) | 0x4,
		.cfg_ddr_training_delay_ps.dac_offset[1] = (0 << 7) | 0x4,
		.cfg_ddr_training_delay_ps.reserve_para[0] = (1 << 7) | 0xb,     //write dqs
		.cfg_ddr_training_delay_ps.reserve_para[1] = (1 << 7) | 0x4,     //write dqs
		.cfg_ddr_training_delay_ps.reserve_para[2] = (1 << 7) | 0x6,     //write dqs
		.cfg_ddr_training_delay_ps.reserve_para[3] = (1 << 7) | 0xb,     //write dqs
		.cfg_ddr_training_delay_ps.reserve_para[4] = (1 << 7) | 0xd,     //write dqs
		.cfg_ddr_training_delay_ps.reserve_para[5] = (1 << 7) | 0x8,     //write dqs
		.cfg_ddr_training_delay_ps.reserve_para[6] = (1 << 7) | 0x7,     //write dqs
		.cfg_ddr_training_delay_ps.reserve_para[7] = (1 << 7) | 0xd,     //write dqs
		.cfg_ddr_training_delay_ps.reserve_para[8 + 0] = (1 << 7) | 0x10,//read dqs
		.cfg_ddr_training_delay_ps.reserve_para[8 + 1] = (1 << 7) | 0x12,//read dqs
		.cfg_ddr_training_delay_ps.reserve_para[8 + 2] = (1 << 7) | 0x15,//read dqs
		.cfg_ddr_training_delay_ps.reserve_para[8 + 3] = (1 << 7) | 0x14,//read dqs
		.cfg_ddr_training_delay_ps.reserve_para[8 + 4] = (1 << 7) | 0x10,//read dqs
		.cfg_ddr_training_delay_ps.reserve_para[8 + 5] = (1 << 7) | 0x12,//read dqs
		.cfg_ddr_training_delay_ps.reserve_para[8 + 6] = (1 << 7) | 0x14,//read dqs
		.cfg_ddr_training_delay_ps.reserve_para[8 + 7] = (1 << 7) | 0x15,//read dqs
		.cfg_ddr_training_delay_ps.ac_trace_delay[0] = 256,
		.cfg_ddr_training_delay_ps.ac_trace_delay[1] = 256,
		.cfg_ddr_training_delay_ps.ac_trace_delay[2] = 256,
		.cfg_ddr_training_delay_ps.ac_trace_delay[3] = 256,
		.cfg_ddr_training_delay_ps.ac_trace_delay[4] = 256,
		.cfg_ddr_training_delay_ps.ac_trace_delay[5] = 256,
		.cfg_ddr_training_delay_ps.ac_trace_delay[6] = 256,
		.cfg_ddr_training_delay_ps.ac_trace_delay[7] = 256,
		.cfg_ddr_training_delay_ps.ac_trace_delay[8] = 256,
		.cfg_ddr_training_delay_ps.ac_trace_delay[9] = 256,
		.cfg_ddr_training_delay_ps.ac_trace_delay[10] = 256,
		.cfg_ddr_training_delay_ps.ac_trace_delay[11] = 256,
		.cfg_ddr_training_delay_ps.ac_trace_delay[12] = 256,
		.cfg_ddr_training_delay_ps.ac_trace_delay[13] = 256,
		.cfg_ddr_training_delay_ps.ac_trace_delay[14] = 256,
		.cfg_ddr_training_delay_ps.ac_trace_delay[15] = 256,
		.cfg_ddr_training_delay_ps.ac_trace_delay[16] = 256,
		.cfg_ddr_training_delay_ps.ac_trace_delay[17] = 256,
		.cfg_ddr_training_delay_ps.ac_trace_delay[18] = 256,
		.cfg_ddr_training_delay_ps.ac_trace_delay[19] = 256,
		.cfg_ddr_training_delay_ps.ac_trace_delay[20] = 256,
		.cfg_ddr_training_delay_ps.ac_trace_delay[21] = 256,
		.cfg_ddr_training_delay_ps.ac_trace_delay[22] = 256,
		.cfg_ddr_training_delay_ps.ac_trace_delay[23] = 256,
		.cfg_ddr_training_delay_ps.ac_trace_delay[24] = 256,
		.cfg_ddr_training_delay_ps.ac_trace_delay[25] = 256,
		.cfg_ddr_training_delay_ps.ac_trace_delay[26] = 256,
		.cfg_ddr_training_delay_ps.ac_trace_delay[27] = 256,
		.cfg_ddr_training_delay_ps.ac_trace_delay[28] = 256,
		.cfg_ddr_training_delay_ps.ac_trace_delay[29] = 256,

		.cfg_ddr_training_delay_ps.ac_trace_delay[24] = 190,
		.cfg_ddr_training_delay_ps.ac_trace_delay[25] = 206,
		.cfg_ddr_training_delay_ps.ac_trace_delay[0] = 256 + 40,
		.cfg_ddr_training_delay_ps.ac_trace_delay[1] = 256 + 20,
		.cfg_ddr_training_delay_ps.ac_trace_delay[7] = 256 + 30,
		.cfg_ddr_training_delay_ps.ac_trace_delay[9] = 256 + 50,

		.cfg_ddr_training_delay_ps.write_dqs_delay[0] = 128,
		.cfg_ddr_training_delay_ps.write_dqs_delay[1] = 128,
		.cfg_ddr_training_delay_ps.write_dqs_delay[2] = 128,
		.cfg_ddr_training_delay_ps.write_dqs_delay[3] = 128,
		.cfg_ddr_training_delay_ps.write_dqs_delay[4] = 128,
		.cfg_ddr_training_delay_ps.write_dqs_delay[5] = 128,
		.cfg_ddr_training_delay_ps.write_dqs_delay[6] = 128,
		.cfg_ddr_training_delay_ps.write_dqs_delay[7] = 128,

		.cfg_ddr_training_delay_ps.read_dqs_delay[0] = 128 - 32,
		.cfg_ddr_training_delay_ps.read_dqs_delay[1] = 128 - 32,
		.cfg_ddr_training_delay_ps.read_dqs_delay[2] = 128 - 32,
		.cfg_ddr_training_delay_ps.read_dqs_delay[3] = 128 - 32,
		.cfg_ddr_training_delay_ps.read_dqs_delay[4] = 128 - 32,
		.cfg_ddr_training_delay_ps.read_dqs_delay[5] = 128 - 32,
		.cfg_ddr_training_delay_ps.read_dqs_delay[6] = 128 - 32,
		.cfg_ddr_training_delay_ps.read_dqs_delay[7] = 128 - 32,

		.cfg_ddr_training_delay_ps.soc_bit_vref0[0] = 0x0000000,
		.cfg_ddr_training_delay_ps.dram_vref[0] = 0x00000000,

	},
//};
#endif

#if DDR4_SKT
//s7 signoff with 3200MBPS
#if defined USE_2RANK_16BIT_X2_DDR4 || defined ENABLE_8BIT_DDR4_CS0_CS1_SAME_PHASE
//USE_2RANK_16BIT_DDR4 use ap222 board
//#define ENABLE_8BIT_DDR4_CS0_CS1_SAME_PHASE 1 //t233
#define  CACLU_CLK_D4   1176// 600
#else
#define  CACLU_CLK_D4   1464
#endif
//ddr_set_ps0_only_t __attribute__ ((aligned(8))) ddr_set_t_default_ddr4 = {
	{
		.cfg_board_common_setting.timming_magic = 0x0,
		.cfg_board_common_setting.timming_max_valid_configs = 0x1,
		.cfg_board_common_setting.timming_struct_version = 9798,
		.cfg_board_common_setting.timming_struct_org_size =
				sizeof(ddr_set_ps0_only_t),
		.cfg_board_common_setting.timming_struct_real_size = 0,
		.cfg_board_common_setting.timming_magic = 0x0,

#if ENABLE_DDR_WINDOW_FAST_BOOT
				.cfg_board_common_setting.fast_boot = {
					0x1, 0, 0, 0xc6
				},
#else
				.cfg_board_common_setting.fast_boot = {
					0x0, 0, 0, 0
				},
#endif

		.cfg_board_common_setting.ddr_func = DDR_FUNC_CONFIG_AUTO_DET_DQ_PINMUX_FUNCTION,
		.cfg_board_common_setting.board_id = CONFIG_BOARD_ID_MASK,
		.cfg_board_common_setting.DramType = CONFIG_DDR_TYPE_DDR4,
		.cfg_board_common_setting.dram_rank_config = CONFIG_DDR0_32BIT_RANK0_CH0,
		.cfg_board_common_setting.rsv_char0 = 0x0,
		.cfg_board_common_setting.DisabledDbyte[0] = 0xfc,
		.cfg_board_common_setting.DisabledDbyte[1] = 0xfc,
		.cfg_board_common_setting.dram_ch0_size_MB = 0xffff,
		.cfg_board_common_setting.dram_ch1_size_MB = 0x0,
		.cfg_board_common_setting.dram_x4x8x16_mode = CONFIG_DRAM_MODE_X16,
		.cfg_board_common_setting.Is2Ttiming = CONFIG_USE_DDR_2T_MODE,
		.cfg_board_common_setting.log_level = 0xff,
		.cfg_board_common_setting.dbi_enable = DDR_WRITE_READ_DBI_DISABLE,
		.cfg_board_common_setting.ddr_rfc_type = DDR_RFC_TYPE_DDR4_2Gbx4,
		.cfg_board_common_setting.enable_lpddr4x_mode = 0,
		.cfg_board_common_setting.pll_ssc_mode = 0x0,

		.cfg_board_common_setting.ac_pinmux = {
			4, 5, 7, 9, 6, 1, 0, 13, 8, 11,
			2, 10, 15, 14, 3, 26, 12, 20, 14, 22,
			21, 16, 19, 19, 12, 17, 24, 23, 28, 29,
		},

		.cfg_board_SI_setting_ps.DRAMFreq = CACLU_CLK_D4,
		.cfg_board_SI_setting_ps.PllBypassEn = 0x0,
		.cfg_board_SI_setting_ps.training_SequenceCtrl = 0x0,
		.cfg_board_SI_setting_ps.dfi_odt_config = DDR_DRAM_ODT_DDR4_PARK_ENABLE,
		.cfg_board_SI_setting_ps.phy_odt_config_rank[0] = 0x0,
		.cfg_board_SI_setting_ps.phy_odt_config_rank[1] = 0x0,
		.cfg_board_SI_setting_ps.clk_drv_ohm = 40,
		.cfg_board_SI_setting_ps.cs_drv_ohm = 40,
		.cfg_board_SI_setting_ps.ac_drv_ohm = 40,
		.cfg_board_SI_setting_ps.soc_data_drv_ohm_p = 40,
		.cfg_board_SI_setting_ps.soc_data_drv_ohm_n = 40,
		.cfg_board_SI_setting_ps.soc_data_odt_ohm_p = 60,
		.cfg_board_SI_setting_ps.soc_data_odt_ohm_n = 0x0,
		.cfg_board_SI_setting_ps.dram_data_drv_ohm = 34,
		.cfg_board_SI_setting_ps.dram_data_odt_ohm = 60,
		.cfg_board_SI_setting_ps.dram_data_wr_odt_ohm = 0x0,
		.cfg_board_SI_setting_ps.dram_ac_odt_ohm = 0,
		.cfg_board_SI_setting_ps.dram_drv_pull_up_cal_ohm = 40,
		.cfg_board_SI_setting_ps.lpddr4_dram_vout_range = 0x1,

		.cfg_board_SI_setting_ps.vref_ac_permil = 500,
		.cfg_board_SI_setting_ps.vref_soc_data_permil = 680,
		.cfg_board_SI_setting_ps.vref_dram_data_permil = 782,
		.cfg_board_SI_setting_ps.max_core_timmming_frequency = 0,
		.cfg_ddr_training_delay_ps.ac_trace_delay[0] = 384,
		.cfg_ddr_training_delay_ps.ac_trace_delay[1] = 384,
		.cfg_ddr_training_delay_ps.ac_trace_delay[2] = 384,
		.cfg_ddr_training_delay_ps.ac_trace_delay[3] = 384,
		.cfg_ddr_training_delay_ps.ac_trace_delay[4] = 384,
		.cfg_ddr_training_delay_ps.ac_trace_delay[5] = 384,
		.cfg_ddr_training_delay_ps.ac_trace_delay[6] = 384,
		.cfg_ddr_training_delay_ps.ac_trace_delay[7] = 384,
		.cfg_ddr_training_delay_ps.ac_trace_delay[8] = 384,
		.cfg_ddr_training_delay_ps.ac_trace_delay[9] = 384,
		.cfg_ddr_training_delay_ps.ac_trace_delay[10] = 384,
		.cfg_ddr_training_delay_ps.ac_trace_delay[11] = 384,
		.cfg_ddr_training_delay_ps.ac_trace_delay[12] = 384,
		.cfg_ddr_training_delay_ps.ac_trace_delay[13] = 384,
		.cfg_ddr_training_delay_ps.ac_trace_delay[14] = 384,
		.cfg_ddr_training_delay_ps.ac_trace_delay[15] = 256,
		.cfg_ddr_training_delay_ps.ac_trace_delay[16] = 384,
		.cfg_ddr_training_delay_ps.ac_trace_delay[17] = 384,
		.cfg_ddr_training_delay_ps.ac_trace_delay[18] = 384,
		.cfg_ddr_training_delay_ps.ac_trace_delay[19] = 256,
		.cfg_ddr_training_delay_ps.ac_trace_delay[20] = 384,
		.cfg_ddr_training_delay_ps.ac_trace_delay[21] = 384,
		.cfg_ddr_training_delay_ps.ac_trace_delay[22] = 384,
		.cfg_ddr_training_delay_ps.ac_trace_delay[23] = 384,
		.cfg_ddr_training_delay_ps.ac_trace_delay[24] = 384,
		.cfg_ddr_training_delay_ps.ac_trace_delay[25] = 384,
		.cfg_ddr_training_delay_ps.ac_trace_delay[26] = 256,
		.cfg_ddr_training_delay_ps.ac_trace_delay[27] = 256,
		.cfg_ddr_training_delay_ps.ac_trace_delay[28] = 256,
		.cfg_ddr_training_delay_ps.ac_trace_delay[29] = 256,

		.cfg_ddr_training_delay_ps.write_dqs_delay[0] = 0x8d,
		.cfg_ddr_training_delay_ps.write_dqs_delay[1] = 0x8f,

		.cfg_ddr_training_delay_ps.soc_bit_vref0[0] = 0x0,
		.cfg_ddr_training_delay_ps.dram_vref[0] = 0x0,

		.cfg_ddr_training_delay_ps.dac_offset[0] = 0x0,
		.cfg_ddr_training_delay_ps.dac_offset[1] = 0x0,
		.cfg_ddr_training_delay_ps.reserve_para[0] = (1 << 7) | 0x0,//write dqs
		.cfg_ddr_training_delay_ps.reserve_para[1] = (1 << 7) | 0x0,//write dqs
		.cfg_ddr_training_delay_ps.reserve_para[2] = (1 << 7) | 0x0,//write dqs
		.cfg_ddr_training_delay_ps.reserve_para[3] = (1 << 7) | 0x0,//write dqs
		.cfg_ddr_training_delay_ps.reserve_para[4] = (1 << 7) | 0x0,//write dqs
		.cfg_ddr_training_delay_ps.reserve_para[5] = (1 << 7) | 0x0,//write dqs
		.cfg_ddr_training_delay_ps.reserve_para[6] = (1 << 7) | 0x0,//write dqs
		.cfg_ddr_training_delay_ps.reserve_para[7] = (1 << 7) | 0x0,//write dqs
		.cfg_ddr_training_delay_ps.reserve_para[8 + 0] = (1 << 7) | 0x9,//read dqs
		.cfg_ddr_training_delay_ps.reserve_para[8 + 1] = (1 << 7) | 0x9,//read dqs
		.cfg_ddr_training_delay_ps.reserve_para[8 + 2] = (1 << 7) | 0x0,//read dqs
		.cfg_ddr_training_delay_ps.reserve_para[8 + 3] = (1 << 7) | 0x0,//read dqs
		.cfg_ddr_training_delay_ps.reserve_para[8 + 4] = (1 << 7) | 0x0,//read dqs
		.cfg_ddr_training_delay_ps.reserve_para[8 + 5] = (1 << 7) | 0x0,//read dqs
		.cfg_ddr_training_delay_ps.reserve_para[8 + 6] = (1 << 7) | 0x0,//read dqs
		.cfg_ddr_training_delay_ps.reserve_para[8 + 7] = (1 << 7) | 0x0,//read dqs
	},
//};
#endif

#if DDR3_SKT
#define  CACLU_CLK_D3	1056 //636 //1792//600 //1200 //(1900)// (1440)//(1008)
//ddr_set_ps0_only_t __attribute__ ((aligned(8))) ddr_set_t_default_ddr3 = {
	{
		.cfg_board_common_setting.timming_magic = 0,
		.cfg_board_common_setting.timming_max_valid_configs = 1,
		.cfg_board_common_setting.timming_struct_version = 9816,
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
		//.cfg_board_common_setting.dram_ch0_size_MB =
		//	(DRAM_SIZE_ID_256MBX1 << CONFIG_CS0_BYTE_01_SIZE_256_ID_OFFSET) +
		//	(DRAM_SIZE_ID_256MBX0 << CONFIG_CS0_BYTE_23_SIZE_256_ID_OFFSET) +
		//	(DRAM_SIZE_ID_256MBX0 << CONFIG_CS1_BYTE_01_SIZE_256_ID_OFFSET) +
		//	(DRAM_SIZE_ID_256MBX0 << CONFIG_CS1_BYTE_23_SIZE_256_ID_OFFSET),
		//.cfg_board_common_setting.dram_ch1_size_MB =
		//	(DRAM_SIZE_ID_256MBX0 << CONFIG_CS0_BYTE_01_SIZE_256_ID_OFFSET) +
		//	(DRAM_SIZE_ID_256MBX0 << CONFIG_CS0_BYTE_23_SIZE_256_ID_OFFSET) +
		//	(DRAM_SIZE_ID_256MBX0 << CONFIG_CS1_BYTE_01_SIZE_256_ID_OFFSET) +
		//	(DRAM_SIZE_ID_256MBX0 << CONFIG_CS1_BYTE_23_SIZE_256_ID_OFFSET),
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
		.cfg_board_SI_setting_ps.soc_data_drv_ohm_p = 40,
		.cfg_board_SI_setting_ps.soc_data_drv_ohm_n = 40,
		.cfg_board_SI_setting_ps.soc_data_odt_ohm_p = 120,
		.cfg_board_SI_setting_ps.soc_data_odt_ohm_n = 120,
		.cfg_board_SI_setting_ps.dram_data_drv_ohm = 40,
		.cfg_board_SI_setting_ps.dram_data_odt_ohm = 120,
		.cfg_board_SI_setting_ps.dram_data_wr_odt_ohm = 0,
		.cfg_board_SI_setting_ps.dram_ac_odt_ohm = 120,
		.cfg_board_SI_setting_ps.dram_drv_pull_up_cal_ohm =
			DDR_DRAM_LPDDR4_ODT_40_OHM,
		.cfg_board_SI_setting_ps.lpddr4_dram_vout_range =
			DDR_DRAM_LPDDR4_OUTPUT_1_3_VDDQ,
		//.cfg_ddr_training_delay_ps.dfe_offset = 0,
#ifdef ENABLE_NTC_512M_DDR3
		.cfg_board_common_setting.ac_pinmux = {
			0,	5,	1,	6,	2,	11,	7,	8,	9,	13,
			14,	18,	12,	3,	4,	26,	12,	19,	14,	22,
			10,	20,	21,	19,	16,	17,	24,	25,	28,	29,
		},
		.cfg_board_common_setting.ddr_dq_remap = {
			15, 9, 11, 8, 10, 14, 12, 33, 13,
			0, 4, 5, 1, 6, 2, 7, 3, 32,
		},
#endif
#ifdef ENABLE_NTC_256M_DDR3L
		.cfg_board_common_setting.ac_pinmux = {
			0,	4,	5,	2,	1,	7,	6,	9,	11,	13,
			8,	7,	17,	3,	12,	26,	12,	19,	14,	22,
			10,	21,	20,	19,	16,	18,	24,	25,	28,	29,
		},
		.cfg_board_common_setting.ddr_dq_remap = {
			6, 7, 5, 1, 3, 2, 0, 32, 4,
			8, 10, 11, 9, 13, 15, 14, 12, 33,
		},
#endif

		//.cfg_ddr_training_delay_ps.tx_offset[0] = (0 << 7) | 0x0,
		//.cfg_ddr_training_delay_ps.rx_offset[0] = (0 << 7) | 0x0,
		.cfg_ddr_training_delay_ps.reserve_para[0] = (1 << 7) | 0x0,     //write dqs
		.cfg_ddr_training_delay_ps.reserve_para[1] = (1 << 7) | 0x0,     //write dqs
		.cfg_ddr_training_delay_ps.reserve_para[2] = (1 << 7) | 0x0,     //write dqs
		.cfg_ddr_training_delay_ps.reserve_para[3] = (1 << 7) | 0x0,     //write dqs
		.cfg_ddr_training_delay_ps.reserve_para[8 + 0] = (1 << 7) | 0x0,//read dqs
		.cfg_ddr_training_delay_ps.reserve_para[8 + 1] = (1 << 7) | 0x0,//read dqs
		.cfg_ddr_training_delay_ps.reserve_para[8 + 2] = (1 << 7) | 0x0,//read dqs
		.cfg_ddr_training_delay_ps.reserve_para[8 + 3] = (1 << 7) | 0x0,//read dqs


		#define AC_OFF_D3  (128) //for sip should use AC_OFFSET 128,
		#define TDQS2DQ_D3  (0)
		#define WL0_D3  (0)

		//if use ac_offset 0, some chip use coarse 0 bad ,some use coars 1 bad
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
		.cfg_ddr_training_delay_ps.ac_trace_delay[10] = 256 + AC_OFF_D3,
		.cfg_ddr_training_delay_ps.ac_trace_delay[11] = 256 + AC_OFF_D3,
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
		//cke 128 only 1UI margin
		.cfg_ddr_training_delay_ps.ac_trace_delay[22] = 256 + AC_OFF_D3,
		.cfg_ddr_training_delay_ps.ac_trace_delay[23] = 256 + AC_OFF_D3,
		.cfg_ddr_training_delay_ps.ac_trace_delay[24] = 256 + AC_OFF_D3,
		.cfg_ddr_training_delay_ps.ac_trace_delay[25] = 256 + AC_OFF_D3,
		.cfg_ddr_training_delay_ps.ac_trace_delay[26] = 256 + AC_OFF_D3,
		.cfg_ddr_training_delay_ps.ac_trace_delay[27] = 256 + AC_OFF_D3,
		.cfg_ddr_training_delay_ps.ac_trace_delay[28] = 256 + AC_OFF_D3,
		.cfg_ddr_training_delay_ps.ac_trace_delay[29] = 256 + AC_OFF_D3,

		.cfg_ddr_training_delay_ps.ac_trace_delay[15] = 128 + AC_OFF_D3,
		.cfg_ddr_training_delay_ps.ac_trace_delay[19] = 128 + AC_OFF_D3,
		.cfg_ddr_training_delay_ps.ac_trace_delay[26] = 128 + AC_OFF_D3,
		.cfg_ddr_training_delay_ps.ac_trace_delay[27] = 128 + AC_OFF_D3,
		.cfg_ddr_training_delay_ps.ac_trace_delay[28] = 128 + AC_OFF_D3,
		.cfg_ddr_training_delay_ps.ac_trace_delay[29] = 128 + AC_OFF_D3,

		.cfg_ddr_training_delay_ps.read_dq_delay_t[0] = 64,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[1] = 64,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[2] = 64,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[3] = 64,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[4] = 64,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[5] = 64,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[6] = 64,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[7] = 64,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[8] = 64,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[9] = 64,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[10] = 64,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[11] = 64,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[12] = 64,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[13] = 64,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[14] = 64,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[15] = 64,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[16] = 64,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[17] = 64,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[18] = 64,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[19] = 64,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[20] = 64,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[21] = 64,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[22] = 64,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[23] = 64,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[24] = 64,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[25] = 64,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[26] = 64,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[27] = 64,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[28] = 64,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[29] = 64,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[30] = 64,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[31] = 64,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[32] = 64,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[33] = 64,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[34] = 64,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[35] = 64,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[36] = 64,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[37] = 64,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[38] = 64,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[39] = 64,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[40] = 64,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[41] = 64,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[42] = 64,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[43] = 64,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[44] = 64,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[45] = 64,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[46] = 64,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[47] = 64,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[48] = 64,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[49] = 64,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[50] = 64,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[51] = 64,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[52] = 64,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[53] = 64,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[54] = 64,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[55] = 64,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[56] = 64,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[57] = 64,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[58] = 64,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[59] = 64,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[60] = 64,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[61] = 64,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[62] = 64,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[63] = 64,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[64] = 64,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[65] = 64,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[66] = 64,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[67] = 64,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[68] = 64,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[69] = 64,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[70] = 64,
		.cfg_ddr_training_delay_ps.read_dq_delay_t[71] = 64,

		.cfg_ddr_training_delay_ps.write_dqs_delay[0] = 128 + AC_OFF_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.write_dqs_delay[1] = 128 + AC_OFF_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.write_dqs_delay[2] = 128 + AC_OFF_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.write_dqs_delay[3] = 128 + AC_OFF_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.write_dqs_delay[4] = 128 + AC_OFF_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.write_dqs_delay[5] = 128 + AC_OFF_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.write_dqs_delay[6] = 128 + AC_OFF_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.write_dqs_delay[7] = 128 + AC_OFF_D3 + WL0_D3,

		.cfg_ddr_training_delay_ps.wdq_delay[0] = 192 + AC_OFF_D3 + TDQS2DQ_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.wdq_delay[1] = 192 + AC_OFF_D3 + TDQS2DQ_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.wdq_delay[2] = 192 + AC_OFF_D3 + TDQS2DQ_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.wdq_delay[3] = 192 + AC_OFF_D3 + TDQS2DQ_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.wdq_delay[4] = 192 + AC_OFF_D3 + TDQS2DQ_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.wdq_delay[5] = 192 + AC_OFF_D3 + TDQS2DQ_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.wdq_delay[6] = 192 + AC_OFF_D3 + TDQS2DQ_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.wdq_delay[7] = 192 + AC_OFF_D3 + TDQS2DQ_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.wdq_delay[8] = 192 + AC_OFF_D3 + TDQS2DQ_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.wdq_delay[9] = 192 + AC_OFF_D3 + TDQS2DQ_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.wdq_delay[10] = 192 + AC_OFF_D3 + TDQS2DQ_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.wdq_delay[11] = 192 + AC_OFF_D3 + TDQS2DQ_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.wdq_delay[12] = 192 + AC_OFF_D3 + TDQS2DQ_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.wdq_delay[13] = 192 + AC_OFF_D3 + TDQS2DQ_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.wdq_delay[14] = 192 + AC_OFF_D3 + TDQS2DQ_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.wdq_delay[15] = 192 + AC_OFF_D3 + TDQS2DQ_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.wdq_delay[16] = 192 + AC_OFF_D3 + TDQS2DQ_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.wdq_delay[17] = 192 + AC_OFF_D3 + TDQS2DQ_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.wdq_delay[18] = 192 + AC_OFF_D3 + TDQS2DQ_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.wdq_delay[19] = 192 + AC_OFF_D3 + TDQS2DQ_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.wdq_delay[20] = 192 + AC_OFF_D3 + TDQS2DQ_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.wdq_delay[21] = 192 + AC_OFF_D3 + TDQS2DQ_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.wdq_delay[22] = 192 + AC_OFF_D3 + TDQS2DQ_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.wdq_delay[23] = 192 + AC_OFF_D3 + TDQS2DQ_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.wdq_delay[24] = 192 + AC_OFF_D3 + TDQS2DQ_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.wdq_delay[25] = 192 + AC_OFF_D3 + TDQS2DQ_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.wdq_delay[26] = 192 + AC_OFF_D3 + TDQS2DQ_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.wdq_delay[27] = 192 + AC_OFF_D3 + TDQS2DQ_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.wdq_delay[28] = 192 + AC_OFF_D3 + TDQS2DQ_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.wdq_delay[29] = 192 + AC_OFF_D3 + TDQS2DQ_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.wdq_delay[30] = 192 + AC_OFF_D3 + TDQS2DQ_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.wdq_delay[31] = 192 + AC_OFF_D3 + TDQS2DQ_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.wdq_delay[32] = 192 + AC_OFF_D3 + TDQS2DQ_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.wdq_delay[33] = 192 + AC_OFF_D3 + TDQS2DQ_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.wdq_delay[34] = 192 + AC_OFF_D3 + TDQS2DQ_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.wdq_delay[35] = 192 + AC_OFF_D3 + TDQS2DQ_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.wdq_delay[36] = 192 + AC_OFF_D3 + TDQS2DQ_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.wdq_delay[37] = 192 + AC_OFF_D3 + TDQS2DQ_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.wdq_delay[38] = 192 + AC_OFF_D3 + TDQS2DQ_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.wdq_delay[39] = 192 + AC_OFF_D3 + TDQS2DQ_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.wdq_delay[40] = 192 + AC_OFF_D3 + TDQS2DQ_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.wdq_delay[41] = 192 + AC_OFF_D3 + TDQS2DQ_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.wdq_delay[42] = 192 + AC_OFF_D3 + TDQS2DQ_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.wdq_delay[43] = 192 + AC_OFF_D3 + TDQS2DQ_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.wdq_delay[44] = 192 + AC_OFF_D3 + TDQS2DQ_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.wdq_delay[45] = 192 + AC_OFF_D3 + TDQS2DQ_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.wdq_delay[46] = 192 + AC_OFF_D3 + TDQS2DQ_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.wdq_delay[47] = 192 + AC_OFF_D3 + TDQS2DQ_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.wdq_delay[48] = 192 + AC_OFF_D3 + TDQS2DQ_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.wdq_delay[49] = 192 + AC_OFF_D3 + TDQS2DQ_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.wdq_delay[50] = 192 + AC_OFF_D3 + TDQS2DQ_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.wdq_delay[51] = 192 + AC_OFF_D3 + TDQS2DQ_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.wdq_delay[52] = 192 + AC_OFF_D3 + TDQS2DQ_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.wdq_delay[53] = 192 + AC_OFF_D3 + TDQS2DQ_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.wdq_delay[54] = 192 + AC_OFF_D3 + TDQS2DQ_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.wdq_delay[55] = 192 + AC_OFF_D3 + TDQS2DQ_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.wdq_delay[56] = 192 + AC_OFF_D3 + TDQS2DQ_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.wdq_delay[57] = 192 + AC_OFF_D3 + TDQS2DQ_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.wdq_delay[58] = 192 + AC_OFF_D3 + TDQS2DQ_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.wdq_delay[59] = 192 + AC_OFF_D3 + TDQS2DQ_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.wdq_delay[60] = 192 + AC_OFF_D3 + TDQS2DQ_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.wdq_delay[61] = 192 + AC_OFF_D3 + TDQS2DQ_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.wdq_delay[62] = 192 + AC_OFF_D3 + TDQS2DQ_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.wdq_delay[63] = 192 + AC_OFF_D3 + TDQS2DQ_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.wdq_delay[64] = 192 + AC_OFF_D3 + TDQS2DQ_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.wdq_delay[65] = 192 + AC_OFF_D3 + TDQS2DQ_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.wdq_delay[66] = 192 + AC_OFF_D3 + TDQS2DQ_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.wdq_delay[67] = 192 + AC_OFF_D3 + TDQS2DQ_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.wdq_delay[68] = 192 + AC_OFF_D3 + TDQS2DQ_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.wdq_delay[69] = 192 + AC_OFF_D3 + TDQS2DQ_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.wdq_delay[70] = 192 + AC_OFF_D3 + TDQS2DQ_D3 + WL0_D3,
		.cfg_ddr_training_delay_ps.wdq_delay[71] = 192 + AC_OFF_D3 + TDQS2DQ_D3 + WL0_D3,

		.cfg_ddr_training_delay_ps.read_dqs_delay[0] = 128,
		.cfg_ddr_training_delay_ps.read_dqs_delay[1] = 128,
		.cfg_ddr_training_delay_ps.read_dqs_delay[2] = 128,
		.cfg_ddr_training_delay_ps.read_dqs_delay[3] = 128,
		.cfg_ddr_training_delay_ps.read_dqs_delay[4] = 128,
		.cfg_ddr_training_delay_ps.read_dqs_delay[5] = 128,
		.cfg_ddr_training_delay_ps.read_dqs_delay[6] = 128,
		.cfg_ddr_training_delay_ps.read_dqs_delay[7] = 128,

		.cfg_ddr_training_delay_ps.soc_bit_vref0[0] = 0x000000,
		//0 for auto training
		.cfg_ddr_training_delay_ps.dram_vref[0] = 0x00000000,

	},
//};
#endif
};
