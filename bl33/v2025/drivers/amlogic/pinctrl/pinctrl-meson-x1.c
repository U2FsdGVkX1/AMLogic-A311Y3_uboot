// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2025 Amlogic, Inc. All rights reserved.
 */

#include <dm.h>
#include <dm/pinctrl.h>
#include <dt-bindings/amlogic/gpio/meson-x1-gpio.h>

#include <../../pinctrl/meson/pinctrl-meson-axg.h>

/* Bank[D]:Function[1] */
static const unsigned int uart_a_rx_pins[]			= { GPIOD_0 };
static const unsigned int uart_a_tx_pins[]			= { GPIOD_1 };
static const unsigned int jtag_clk_pins[]			= { GPIOD_2 };
static const unsigned int jtag_tdo_pins[]			= { GPIOD_3 };
static const unsigned int jtag_tms_pins[]			= { GPIOD_4 };
static const unsigned int jtag_tdi_pins[]			= { GPIOD_5 };
static const unsigned int gphy_led0_link_active_pins[]		= { GPIOD_6 };
static const unsigned int gphy_led1_link_active_pins[]		= { GPIOD_7 };
static const unsigned int gphy_led2_link_active_pins[]		= { GPIOD_8 };
static const unsigned int gphy_led3_link_active_pins[]		= { GPIOD_9 };
static const unsigned int gphy_led0_speed_pins[]		= { GPIOD_10 };
static const unsigned int gphy_led1_speed_pins[]		= { GPIOD_11 };
static const unsigned int gphy_led2_speed_pins[]		= { GPIOD_12 };
static const unsigned int gphy_led3_speed_pins[]		= { GPIOD_13 };
static const unsigned int i2c1_scl_pins[]			= { GPIOD_18 };
static const unsigned int i2c1_sda_pins[]			= { GPIOD_19 };

/* Bank[D]:Function[2] */
static const unsigned int spi_a_mosi_pins[]			= { GPIOD_2 };
static const unsigned int spi_a_miso_pins[]			= { GPIOD_3 };
static const unsigned int spi_a_ss0_pins[]			= { GPIOD_4 };
static const unsigned int spi_a_ss1_pins[]			= { GPIOD_5 };
static const unsigned int spi_b_clk_pins[]			= { GPIOD_10 };
static const unsigned int spi_b_mosi_pins[]			= { GPIOD_11 };
static const unsigned int spi_b_miso_pins[]			= { GPIOD_12 };
static const unsigned int spi_b_ss0_pins[]			= { GPIOD_13 };
static const unsigned int spi_b_ss1_pins[]			= { GPIOD_14 };
static const unsigned int pwm_c_d_pins[]			= { GPIOD_17 };
static const unsigned int pwm_d_d_pins[]			= { GPIOD_18 };
static const unsigned int spi_a_clk_pins[]			= { GPIOD_19 };

/* Bank[D]:Function[3] */
static const unsigned int uart_d_tx_d_pins[]			= { GPIOD_10 };
static const unsigned int uart_d_rx_d_pins[]			= { GPIOD_11 };
static const unsigned int uart_d_cts_d_pins[]			= { GPIOD_12 };
static const unsigned int uart_d_rts_d_pins[]			= { GPIOD_13 };
static const unsigned int pwm_e_pins[]				= { GPIOD_14 };
static const unsigned int pwm_f_pins[]				= { GPIOD_15 };

/* Bank[D]:Function[4] */
static const unsigned int tdm_sclk1_pins[]			= { GPIOD_10 };
static const unsigned int tdm_fs1_pins[]			= { GPIOD_11 };
static const unsigned int tdm_d0_pins[]				= { GPIOD_12 };
static const unsigned int tdm_d1_pins[]				= { GPIOD_13 };

/* Bank[D]:Function[5] */
static const unsigned int gen_clk_pins[]			= { GPIOD_2 };
static const unsigned int gen_clk_d_pins[]			= { GPIOD_13 };
static const unsigned int tst_loop2_in_pins[]			= { GPIOD_14 };
static const unsigned int tst_loop2_out_pins[]			= { GPIOD_15 };

/* Bank[D]:Function[6] */
static const unsigned int pio0_pins[]				= { GPIOD_0 };
static const unsigned int pio1_pins[]				= { GPIOD_1 };
static const unsigned int pio2_pins[]				= { GPIOD_2 };
static const unsigned int pio3_pins[]				= { GPIOD_3 };
static const unsigned int pio4_pins[]				= { GPIOD_4 };
static const unsigned int pio5_pins[]				= { GPIOD_5 };
static const unsigned int pio6_pins[]				= { GPIOD_6 };
static const unsigned int pio7_pins[]				= { GPIOD_7 };
static const unsigned int pio8_pins[]				= { GPIOD_8 };
static const unsigned int pio9_pins[]				= { GPIOD_9 };
static const unsigned int pio10_pins[]				= { GPIOD_10 };
static const unsigned int pio11_pins[]				= { GPIOD_11 };
static const unsigned int pio12_pins[]				= { GPIOD_12 };
static const unsigned int pio13_pins[]				= { GPIOD_13 };
static const unsigned int pio14_pins[]				= { GPIOD_14 };
static const unsigned int pio15_pins[]				= { GPIOD_15 };
static const unsigned int pio16_pins[]				= { GPIOD_16 };
static const unsigned int pio17_pins[]				= { GPIOD_17 };
static const unsigned int pio18_pins[]				= { GPIOD_18 };
static const unsigned int pio19_pins[]				= { GPIOD_19 };

/* Bank[D]:Function[7] */
static const unsigned int debug_i0_pins[]			= { GPIOD_0 };
static const unsigned int debug_i1_pins[]			= { GPIOD_1 };
static const unsigned int debug_i2_pins[]			= { GPIOD_2 };
static const unsigned int debug_i3_pins[]			= { GPIOD_3 };
static const unsigned int debug_i4_pins[]			= { GPIOD_4 };
static const unsigned int debug_i5_pins[]			= { GPIOD_5 };
static const unsigned int debug_i6_pins[]			= { GPIOD_6 };
static const unsigned int debug_i7_pins[]			= { GPIOD_7 };
static const unsigned int debug_i8_pins[]			= { GPIOD_8 };
static const unsigned int debug_i9_pins[]			= { GPIOD_9 };
static const unsigned int debug_i10_pins[]			= { GPIOD_10 };
static const unsigned int debug_i11_pins[]			= { GPIOD_11 };
static const unsigned int debug_i12_pins[]			= { GPIOD_12 };

/* Bank[E]:Function[1] */
static const unsigned int pwm_a_pins[]				= { GPIOE_0 };
static const unsigned int pwm_b_pins[]				= { GPIOE_1 };

/* Bank[E]:Function[2] */
static const unsigned int i2c0_sda_pins[]			= { GPIOE_0 };
static const unsigned int i2c0_scl_pins[]			= { GPIOE_1 };

/* Bank[E]:Function[3] */
static const unsigned int uart_b_tx_e_pins[]			= { GPIOE_0 };
static const unsigned int uart_b_rx_e_pins[]			= { GPIOE_1 };

/* Bank[E]:Function[5] */
static const unsigned int gen_clk_e_pins[]			= { GPIOE_1 };

/* Bank[E]:Function[6] */
static const unsigned int pio30_pins[]				= { GPIOE_0 };
static const unsigned int pio31_pins[]				= { GPIOE_1 };

/* Bank[B]:Function[1] */
static const unsigned int emmc_d0_pins[]			= { GPIOB_0 };
static const unsigned int emmc_d1_pins[]			= { GPIOB_1 };
static const unsigned int emmc_d2_pins[]			= { GPIOB_2 };
static const unsigned int emmc_d3_pins[]			= { GPIOB_3 };
static const unsigned int emmc_d4_pins[]			= { GPIOB_4 };
static const unsigned int emmc_d5_pins[]			= { GPIOB_5 };
static const unsigned int emmc_d6_pins[]			= { GPIOB_6 };
static const unsigned int emmc_d7_pins[]			= { GPIOB_7 };
static const unsigned int emmc_clk_pins[]			= { GPIOB_8 };
static const unsigned int emmc_rst_gpio_pins[]			= { GPIOB_9 };
static const unsigned int emmc_cmd_pins[]			= { GPIOB_10 };
static const unsigned int emmc_ds_pins[]			= { GPIOB_11 };

/* Bank[B]:Function[2] */
static const unsigned int spinf_mo_d0_pins[]			= { GPIOB_0 };
static const unsigned int spinf_mi_d1_pins[]			= { GPIOB_1 };
static const unsigned int spinf_wp_d2_pins[]			= { GPIOB_2 };
static const unsigned int spinf_hold_d3_pins[]			= { GPIOB_3 };
static const unsigned int spinf_clk_pins[]			= { GPIOB_10 };
static const unsigned int spinf_rst_gpio_pins[]			= { GPIOB_11 };
static const unsigned int spinf_cs1_pins[]			= { GPIOB_12 };
static const unsigned int spinf_cs0_pins[]			= { GPIOB_13 };

/* Bank[B]:Function[3] */
static const unsigned int spi_a_mosi_b_pins[]			= { GPIOB_4 };
static const unsigned int spi_a_miso_b_pins[]			= { GPIOB_5 };
static const unsigned int spi_a_ss0_b_pins[]			= { GPIOB_6 };
static const unsigned int spi_a_clk_b_pins[]			= { GPIOB_7 };
static const unsigned int uart_b_tx_b_pins[]			= { GPIOB_8 };
static const unsigned int uart_b_rx_b_pins[]			= { GPIOB_9 };
static const unsigned int uart_b_cts_b_pins[]			= { GPIOB_10 };
static const unsigned int uart_b_rts_b_pins[]			= { GPIOB_11 };
static const unsigned int i2c0_scl_od_pins[]			= { GPIOB_12 };
static const unsigned int i2c0_sda_od_pins[]			= { GPIOB_13 };

/* Bank[B]:Function[5] */
static const unsigned int tst_loop3_in_pins[]			= { GPIOB_4 };
static const unsigned int tst_loop3_out_pins[]			= { GPIOB_5 };
static const unsigned int gen_clk_b_pins[]			= { GPIOB_8 };

/* Bank[B]:Function[6] */
static const unsigned int tst_out0_pins[]			= { GPIOB_8 };
static const unsigned int tst_out1_pins[]			= { GPIOB_9 };
static const unsigned int tst_out2_pins[]			= { GPIOB_10 };
static const unsigned int tst_out3_pins[]			= { GPIOB_11 };
static const unsigned int tst_out4_pins[]			= { GPIOB_12 };
static const unsigned int tst_out5_pins[]			= { GPIOB_13 };

/* Bank[B]:Function[7] */
static const unsigned int debug_o0_pins[]			= { GPIOB_8 };
static const unsigned int debug_o1_pins[]			= { GPIOB_9 };
static const unsigned int debug_o2_pins[]			= { GPIOB_10 };
static const unsigned int debug_o3_pins[]			= { GPIOB_11 };
static const unsigned int debug_o4_pins[]			= { GPIOB_12 };
static const unsigned int debug_o5_pins[]			= { GPIOB_13 };

/* Bank[X]:Function[1] */
static const unsigned int pcie_a_ck_reqn_pins[]			= { GPIOX_1 };
static const unsigned int pcie_b_ck_reqn_pins[]			= { GPIOX_3 };
static const unsigned int pcie_c_rstb_in_pins[]			= { GPIOX_4 };
static const unsigned int pcie_c_ck_reqn_pins[]			= { GPIOX_5 };
static const unsigned int pwm_e_x_pins[]			= { GPIOX_6 };
static const unsigned int pwm_f_x_pins[]			= { GPIOX_7 };

/* Bank[X]:Function[2] */
static const unsigned int uart_b_tx_x_pins[]			= { GPIOX_4 };
static const unsigned int uart_b_rx_x_pins[]			= { GPIOX_5 };
static const unsigned int i2c1_scl_x_pins[]			= { GPIOX_8 };
static const unsigned int i2c1_sda_x_pins[]			= { GPIOX_9 };

/* Bank[X]:Function[5] */
static const unsigned int test_clk_1m_pins[]			= { GPIOX_0 };
static const unsigned int tst_loop1_in_pins[]			= { GPIOX_1 };
static const unsigned int tst_loop1_out_pins[]			= { GPIOX_3 };
static const unsigned int test_clk_1m_x_pins[]			= { GPIOX_4 };
static const unsigned int gen_clk_x_pins[]			= { GPIOX_8 };

/* Bank[X]:Function[6] */
static const unsigned int test_clk_1m_x3_pins[]			= { GPIOX_3 };
static const unsigned int tst_out6_pins[]			= { GPIOX_6 };
static const unsigned int tst_out7_pins[]			= { GPIOX_7 };
static const unsigned int tst_out8_pins[]			= { GPIOX_8 };
static const unsigned int tst_out9_pins[]			= { GPIOX_9 };

/* Bank[X]:Function[7] */
static const unsigned int debug_o6_pins[]			= { GPIOX_6 };
static const unsigned int debug_o7_pins[]			= { GPIOX_7 };
static const unsigned int debug_o8_pins[]			= { GPIOX_8 };
static const unsigned int debug_o9_pins[]			= { GPIOX_9 };

/* Bank[A]:Function[1] */
static const unsigned int i2c2_scl_pins[]			= { GPIOA_0 };
static const unsigned int i2c2_sda_pins[]			= { GPIOA_1 };
static const unsigned int mclk1_pins[]				= { GPIOA_2 };
static const unsigned int tdm_sclk1_a_pins[]			= { GPIOA_3 };
static const unsigned int tdm_fs1_a_pins[]			= { GPIOA_4 };
static const unsigned int tdm_d0_a_pins[]			= { GPIOA_5 };
static const unsigned int tdm_d1_a_pins[]			= { GPIOA_6 };
static const unsigned int tdm_d2_pins[]				= { GPIOA_7 };
static const unsigned int tdm_d3_pins[]				= { GPIOA_8 };
static const unsigned int tdm_fs2_pins[]			= { GPIOA_9 };
static const unsigned int tdm_sclk2_pins[]			= { GPIOA_10 };

/* Bank[A]:Function[2] */
static const unsigned int uart_d_tx_a_pins[]			= { GPIOA_0 };
static const unsigned int uart_d_rx_a_pins[]			= { GPIOA_1 };
static const unsigned int uart_d_cts_a_pins[]			= { GPIOA_2 };
static const unsigned int uart_c_tx_a_pins[]			= { GPIOA_4 };
static const unsigned int uart_c_rx_a_pins[]			= { GPIOA_5 };
static const unsigned int uart_c_cts_a_pins[]			= { GPIOA_6 };
static const unsigned int uart_c_rts_a_pins[]			= { GPIOA_7 };
static const unsigned int uart_d_rts_a_pins[]			= { GPIOA_8 };
static const unsigned int pwm_e_a_pins[]			= { GPIOA_9 };
static const unsigned int pwm_f_a_pins[]			= { GPIOA_10 };

/* Bank[A]:Function[3] */
static const unsigned int spi_a_clk_a_pins[]			= { GPIOA_0 };
static const unsigned int spi_a_mosi_a_pins[]			= { GPIOA_1 };
static const unsigned int spi_a_miso_a_pins[]			= { GPIOA_2 };
static const unsigned int zsi_clk_pins[]			= { GPIOA_3 };
static const unsigned int zsi_fs_pins[]				= { GPIOA_4 };
static const unsigned int zsi_mosi_pins[]			= { GPIOA_5 };
static const unsigned int zsi_miso_pins[]			= { GPIOA_6 };
static const unsigned int spi_a_ss0_a_pins[]			= { GPIOA_7 };
static const unsigned int spi_a_ss1_a_pins[]			= { GPIOA_8 };
static const unsigned int clk12m_24m_pins[]			= { GPIOA_9 };

/* Bank[A]:Function[4] */
static const unsigned int isi_clk_pins[]			= { GPIOA_3 };
static const unsigned int isi_mosi_pins[]			= { GPIOA_5 };
static const unsigned int isi_miso_pins[]			= { GPIOA_6 };

/* Bank[A]:Function[5] */
static const unsigned int gen_clk_a_pins[]			= { GPIOA_10 };

/* Bank[A]:Function[6] */
static const unsigned int tst_out10_pins[]			= { GPIOA_0 };
static const unsigned int tst_out11_pins[]			= { GPIOA_1 };
static const unsigned int tst_out12_pins[]			= { GPIOA_2 };
static const unsigned int tst_out13_pins[]			= { GPIOA_3 };
static const unsigned int tst_out14_pins[]			= { GPIOA_4 };
static const unsigned int tst_out15_pins[]			= { GPIOA_5 };
static const unsigned int tst_out16_pins[]			= { GPIOA_6 };
static const unsigned int tst_out17_pins[]			= { GPIOA_7 };
static const unsigned int tst_out18_pins[]			= { GPIOA_8 };
static const unsigned int tst_out19_pins[]			= { GPIOA_9 };

/* Bank[A]:Function[7] */
static const unsigned int debug_o10_pins[]			= { GPIOA_0 };
static const unsigned int debug_o11_pins[]			= { GPIOA_1 };
static const unsigned int debug_o12_pins[]			= { GPIOA_2 };
static const unsigned int debug_o13_pins[]			= { GPIOA_3 };
static const unsigned int debug_o14_pins[]			= { GPIOA_4 };
static const unsigned int debug_o15_pins[]			= { GPIOA_5 };
static const unsigned int debug_o16_pins[]			= { GPIOA_6 };
static const unsigned int debug_o17_pins[]			= { GPIOA_7 };
static const unsigned int debug_o18_pins[]			= { GPIOA_8 };
static const unsigned int debug_o19_pins[]			= { GPIOA_9 };

/* Bank[Z]:Function[1] */
static const unsigned int eth_a_mdc_pins[]			= { GPIOZ_0 };
static const unsigned int eth_a_mdio_pins[]			= { GPIOZ_1 };
static const unsigned int clk25m_a_pins[]			= { GPIOZ_2 };
static const unsigned int pwm_c_z_pins[]			= { GPIOZ_3 };
static const unsigned int pwm_d_z_pins[]			= { GPIOZ_4 };
static const unsigned int i2c0_scl_z_pins[]			= { GPIOZ_5 };
static const unsigned int i2c0_sda_z_pins[]			= { GPIOZ_6 };
static const unsigned int eth_b_mdc_pins[]			= { GPIOZ_7 };
static const unsigned int eth_b_mdio_pins[]			= { GPIOZ_8 };
static const unsigned int clk25m_b_pins[]			= { GPIOZ_9 };

/* Bank[Z]:Function[2] */
static const unsigned int i2cs_a_scl_pins[]			= { GPIOZ_5 };
static const unsigned int i2cs_a_sda_pins[]			= { GPIOZ_6 };

/* Bank[Z]:Function[3] */
static const unsigned int clk12m_24m_z_pins[]			= { GPIOZ_2 };
static const unsigned int uart_c_tx_z_pins[]			= { GPIOZ_5 };
static const unsigned int uart_c_rx_z_pins[]			= { GPIOZ_6 };
static const unsigned int uart_c_cts_z_pins[]			= { GPIOZ_7 };
static const unsigned int uart_c_rts_z_pins[]			= { GPIOZ_8 };

/* Bank[Z]:Function[5] */
static const unsigned int gen_clk_z_pins[]			= { GPIOZ_2 };

/* Bank[Z]:Function[6] */
static const unsigned int pio20_pins[]				= { GPIOZ_0 };
static const unsigned int pio21_pins[]				= { GPIOZ_1 };
static const unsigned int pio22_pins[]				= { GPIOZ_2 };
static const unsigned int pio23_pins[]				= { GPIOZ_3 };
static const unsigned int pio24_pins[]				= { GPIOZ_4 };
static const unsigned int pio25_pins[]				= { GPIOZ_5 };
static const unsigned int pio26_pins[]				= { GPIOZ_6 };
static const unsigned int pio27_pins[]				= { GPIOZ_7 };
static const unsigned int pio28_pins[]				= { GPIOZ_8 };
static const unsigned int pio29_pins[]				= { GPIOZ_9 };

/* Bank[Z]:Function[7] */
static const unsigned int debug_i13_pins[]			= { GPIOZ_0 };
static const unsigned int debug_i14_pins[]			= { GPIOZ_1 };
static const unsigned int debug_i15_pins[]			= { GPIOZ_2 };
static const unsigned int debug_i16_pins[]			= { GPIOZ_3 };
static const unsigned int debug_i17_pins[]			= { GPIOZ_4 };
static const unsigned int debug_i18_pins[]			= { GPIOZ_5 };
static const unsigned int debug_i19_pins[]			= { GPIOZ_6 };

static struct meson_pmx_group meson_x1_periphs_groups[] = {
	/* TEST_N */
	GPIO_GROUP(TEST_N, 0),
	/* GPIOD */
	GPIO_GROUP(GPIOD_0, 0),
	GPIO_GROUP(GPIOD_1, 0),
	GPIO_GROUP(GPIOD_2, 0),
	GPIO_GROUP(GPIOD_3, 0),
	GPIO_GROUP(GPIOD_4, 0),
	GPIO_GROUP(GPIOD_5, 0),
	GPIO_GROUP(GPIOD_6, 0),
	GPIO_GROUP(GPIOD_7, 0),
	GPIO_GROUP(GPIOD_8, 0),
	GPIO_GROUP(GPIOD_9, 0),
	GPIO_GROUP(GPIOD_10, 0),
	GPIO_GROUP(GPIOD_11, 0),
	GPIO_GROUP(GPIOD_12, 0),
	GPIO_GROUP(GPIOD_13, 0),
	GPIO_GROUP(GPIOD_14, 0),
	GPIO_GROUP(GPIOD_15, 0),
	GPIO_GROUP(GPIOD_16, 0),
	GPIO_GROUP(GPIOD_17, 0),
	GPIO_GROUP(GPIOD_18, 0),
	GPIO_GROUP(GPIOD_19, 0),
	/* GPIOE */
	GPIO_GROUP(GPIOE_0, 0),
	GPIO_GROUP(GPIOE_1, 0),
	/* GPIOB */
	GPIO_GROUP(GPIOB_0, 0),
	GPIO_GROUP(GPIOB_1, 0),
	GPIO_GROUP(GPIOB_2, 0),
	GPIO_GROUP(GPIOB_3, 0),
	GPIO_GROUP(GPIOB_4, 0),
	GPIO_GROUP(GPIOB_5, 0),
	GPIO_GROUP(GPIOB_6, 0),
	GPIO_GROUP(GPIOB_7, 0),
	GPIO_GROUP(GPIOB_8, 0),
	GPIO_GROUP(GPIOB_9, 0),
	GPIO_GROUP(GPIOB_10, 0),
	GPIO_GROUP(GPIOB_11, 0),
	GPIO_GROUP(GPIOB_12, 0),
	GPIO_GROUP(GPIOB_13, 0),
	/* GPIOX */
	GPIO_GROUP(GPIOX_0, 0),
	GPIO_GROUP(GPIOX_1, 0),
	GPIO_GROUP(GPIOX_2, 0),
	GPIO_GROUP(GPIOX_3, 0),
	GPIO_GROUP(GPIOX_4, 0),
	GPIO_GROUP(GPIOX_5, 0),
	GPIO_GROUP(GPIOX_6, 0),
	GPIO_GROUP(GPIOX_7, 0),
	GPIO_GROUP(GPIOX_8, 0),
	GPIO_GROUP(GPIOX_9, 0),
	/* GPIOA */
	GPIO_GROUP(GPIOA_0, 0),
	GPIO_GROUP(GPIOA_1, 0),
	GPIO_GROUP(GPIOA_2, 0),
	GPIO_GROUP(GPIOA_3, 0),
	GPIO_GROUP(GPIOA_4, 0),
	GPIO_GROUP(GPIOA_5, 0),
	GPIO_GROUP(GPIOA_6, 0),
	GPIO_GROUP(GPIOA_7, 0),
	GPIO_GROUP(GPIOA_8, 0),
	GPIO_GROUP(GPIOA_9, 0),
	GPIO_GROUP(GPIOA_10, 0),
	/* GPIOZ */
	GPIO_GROUP(GPIOZ_0, 0),
	GPIO_GROUP(GPIOZ_1, 0),
	GPIO_GROUP(GPIOZ_2, 0),
	GPIO_GROUP(GPIOZ_3, 0),
	GPIO_GROUP(GPIOZ_4, 0),
	GPIO_GROUP(GPIOZ_5, 0),
	GPIO_GROUP(GPIOZ_6, 0),
	GPIO_GROUP(GPIOZ_7, 0),
	GPIO_GROUP(GPIOZ_8, 0),
	GPIO_GROUP(GPIOZ_9, 0),
	/* Bank[D]:Function[1] */
	GROUP(uart_a_rx,			1),
	GROUP(uart_a_tx,			1),
	GROUP(jtag_clk,				1),
	GROUP(jtag_tdo,				1),
	GROUP(jtag_tms,				1),
	GROUP(jtag_tdi,				1),
	GROUP(gphy_led0_link_active,		1),
	GROUP(gphy_led1_link_active,		1),
	GROUP(gphy_led2_link_active,		1),
	GROUP(gphy_led3_link_active,		1),
	GROUP(gphy_led0_speed,			1),
	GROUP(gphy_led1_speed,			1),
	GROUP(gphy_led2_speed,			1),
	GROUP(gphy_led3_speed,			1),
	GROUP(i2c1_scl,				1),
	GROUP(i2c1_sda,				1),
	/* Bank[D]:Function[2] */
	GROUP(spi_a_mosi,			2),
	GROUP(spi_a_miso,			2),
	GROUP(spi_a_ss0,			2),
	GROUP(spi_a_ss1,			2),
	GROUP(spi_b_clk,			2),
	GROUP(spi_b_mosi,			2),
	GROUP(spi_b_miso,			2),
	GROUP(spi_b_ss0,			2),
	GROUP(spi_b_ss1,			2),
	GROUP(pwm_c_d,				2),
	GROUP(pwm_d_d,				2),
	GROUP(spi_a_clk,			2),
	/* Bank[D]:Function[3] */
	GROUP(uart_d_tx_d,			3),
	GROUP(uart_d_rx_d,			3),
	GROUP(uart_d_cts_d,			3),
	GROUP(uart_d_rts_d,			3),
	GROUP(pwm_e,				3),
	GROUP(pwm_f,				3),
	/* Bank[D]:Function[4] */
	GROUP(tdm_sclk1,			4),
	GROUP(tdm_fs1,				4),
	GROUP(tdm_d0,				4),
	GROUP(tdm_d1,				4),
	/* Bank[D]:Function[5] */
	GROUP(gen_clk,				5),
	GROUP(gen_clk_d,			5),
	GROUP(tst_loop2_in,			5),
	GROUP(tst_loop2_out,			5),
	/* Bank[D]:Function[6] */
	GROUP(pio0,				6),
	GROUP(pio1,				6),
	GROUP(pio2,				6),
	GROUP(pio3,				6),
	GROUP(pio4,				6),
	GROUP(pio5,				6),
	GROUP(pio6,				6),
	GROUP(pio7,				6),
	GROUP(pio8,				6),
	GROUP(pio9,				6),
	GROUP(pio10,				6),
	GROUP(pio11,				6),
	GROUP(pio12,				6),
	GROUP(pio13,				6),
	GROUP(pio14,				6),
	GROUP(pio15,				6),
	GROUP(pio16,				6),
	GROUP(pio17,				6),
	GROUP(pio18,				6),
	GROUP(pio19,				6),
	/* Bank[D]:Function[7] */
	GROUP(debug_i0,				7),
	GROUP(debug_i1,				7),
	GROUP(debug_i2,				7),
	GROUP(debug_i3,				7),
	GROUP(debug_i4,				7),
	GROUP(debug_i5,				7),
	GROUP(debug_i6,				7),
	GROUP(debug_i7,				7),
	GROUP(debug_i8,				7),
	GROUP(debug_i9,				7),
	GROUP(debug_i10,			7),
	GROUP(debug_i11,			7),
	GROUP(debug_i12,			7),
	/* Bank[E]:Function[1] */
	GROUP(pwm_a,				1),
	GROUP(pwm_b,				1),
	/* Bank[E]:Function[2] */
	GROUP(i2c0_sda,				2),
	GROUP(i2c0_scl,				2),
	/* Bank[E]:Function[3] */
	GROUP(uart_b_tx_e,			3),
	GROUP(uart_b_rx_e,			3),
	/* Bank[E]:Function[5] */
	GROUP(gen_clk_e,			5),
	/* Bank[E]:Function[6] */
	GROUP(pio30,				6),
	GROUP(pio31,				6),
	/* Bank[B]:Function[1] */
	GROUP(emmc_d0,				1),
	GROUP(emmc_d1,				1),
	GROUP(emmc_d2,				1),
	GROUP(emmc_d3,				1),
	GROUP(emmc_d4,				1),
	GROUP(emmc_d5,				1),
	GROUP(emmc_d6,				1),
	GROUP(emmc_d7,				1),
	GROUP(emmc_clk,				1),
	GROUP(emmc_rst_gpio,			1),
	GROUP(emmc_cmd,				1),
	GROUP(emmc_ds,				1),
	/* Bank[B]:Function[2] */
	GROUP(spinf_mo_d0,			2),
	GROUP(spinf_mi_d1,			2),
	GROUP(spinf_wp_d2,			2),
	GROUP(spinf_hold_d3,			2),
	GROUP(spinf_clk,			2),
	GROUP(spinf_rst_gpio,			2),
	GROUP(spinf_cs1,			2),
	GROUP(spinf_cs0,			2),
	/* Bank[B]:Function[3] */
	GROUP(spi_a_mosi_b,			3),
	GROUP(spi_a_miso_b,			3),
	GROUP(spi_a_ss0_b,			3),
	GROUP(spi_a_clk_b,			3),
	GROUP(uart_b_tx_b,			3),
	GROUP(uart_b_rx_b,			3),
	GROUP(uart_b_cts_b,			3),
	GROUP(uart_b_rts_b,			3),
	GROUP(i2c0_scl_od,			3),
	GROUP(i2c0_sda_od,			3),
	/* Bank[B]:Function[5] */
	GROUP(tst_loop3_in,			5),
	GROUP(tst_loop3_out,			5),
	GROUP(gen_clk_b,			5),
	/* Bank[B]:Function[6] */
	GROUP(tst_out0,				6),
	GROUP(tst_out1,				6),
	GROUP(tst_out2,				6),
	GROUP(tst_out3,				6),
	GROUP(tst_out4,				6),
	GROUP(tst_out5,				6),
	/* Bank[B]:Function[7] */
	GROUP(debug_o0,				7),
	GROUP(debug_o1,				7),
	GROUP(debug_o2,				7),
	GROUP(debug_o3,				7),
	GROUP(debug_o4,				7),
	GROUP(debug_o5,				7),
	/* Bank[X]:Function[1] */
	GROUP(pcie_a_ck_reqn,			1),
	GROUP(pcie_b_ck_reqn,			1),
	GROUP(pcie_c_rstb_in,			1),
	GROUP(pcie_c_ck_reqn,			1),
	GROUP(pwm_e_x,				1),
	GROUP(pwm_f_x,				1),
	/* Bank[X]:Function[2] */
	GROUP(uart_b_tx_x,			2),
	GROUP(uart_b_rx_x,			2),
	GROUP(i2c1_scl_x,			2),
	GROUP(i2c1_sda_x,			2),
	/* Bank[X]:Function[5] */
	GROUP(test_clk_1m,			5),
	GROUP(tst_loop1_in,			5),
	GROUP(tst_loop1_out,			5),
	GROUP(test_clk_1m_x,			5),
	GROUP(gen_clk_x,			5),
	/* Bank[X]:Function[6] */
	GROUP(test_clk_1m_x3,			6),
	GROUP(tst_out6,				6),
	GROUP(tst_out7,				6),
	GROUP(tst_out8,				6),
	GROUP(tst_out9,				6),
	/* Bank[X]:Function[7] */
	GROUP(debug_o6,				7),
	GROUP(debug_o7,				7),
	GROUP(debug_o8,				7),
	GROUP(debug_o9,				7),
	/* Bank[A]:Function[1] */
	GROUP(i2c2_scl,			1),
	GROUP(i2c2_sda,			1),
	GROUP(mclk1,				1),
	GROUP(tdm_sclk1_a,			1),
	GROUP(tdm_fs1_a,			1),
	GROUP(tdm_d0_a,				1),
	GROUP(tdm_d1_a,				1),
	GROUP(tdm_d2,				1),
	GROUP(tdm_d3,				1),
	GROUP(tdm_fs2,				1),
	GROUP(tdm_sclk2,			1),
	/* Bank[A]:Function[2] */
	GROUP(uart_d_tx_a,			2),
	GROUP(uart_d_rx_a,			2),
	GROUP(uart_d_cts_a,			2),
	GROUP(uart_c_tx_a,			2),
	GROUP(uart_c_rx_a,			2),
	GROUP(uart_c_cts_a,			2),
	GROUP(uart_c_rts_a,			2),
	GROUP(uart_d_rts_a,			2),
	GROUP(pwm_e_a,				2),
	GROUP(pwm_f_a,				2),
	/* Bank[A]:Function[3] */
	GROUP(spi_a_clk_a,			3),
	GROUP(spi_a_mosi_a,			3),
	GROUP(spi_a_miso_a,			3),
	GROUP(zsi_clk,				3),
	GROUP(zsi_fs,				3),
	GROUP(zsi_mosi,				3),
	GROUP(zsi_miso,				3),
	GROUP(spi_a_ss0_a,			3),
	GROUP(spi_a_ss1_a,			3),
	GROUP(clk12m_24m,			3),
	/* Bank[A]:Function[4] */
	GROUP(isi_clk,				4),
	GROUP(isi_mosi,				4),
	GROUP(isi_miso,				4),
	/* Bank[A]:Function[5] */
	GROUP(gen_clk_a,			5),
	/* Bank[A]:Function[6] */
	GROUP(tst_out10,			6),
	GROUP(tst_out11,			6),
	GROUP(tst_out12,			6),
	GROUP(tst_out13,			6),
	GROUP(tst_out14,			6),
	GROUP(tst_out15,			6),
	GROUP(tst_out16,			6),
	GROUP(tst_out17,			6),
	GROUP(tst_out18,			6),
	GROUP(tst_out19,			6),
	/* Bank[A]:Function[7] */
	GROUP(debug_o10,			7),
	GROUP(debug_o11,			7),
	GROUP(debug_o12,			7),
	GROUP(debug_o13,			7),
	GROUP(debug_o14,			7),
	GROUP(debug_o15,			7),
	GROUP(debug_o16,			7),
	GROUP(debug_o17,			7),
	GROUP(debug_o18,			7),
	GROUP(debug_o19,			7),
	/* Bank[Z]:Function[1] */
	GROUP(eth_a_mdc,			1),
	GROUP(eth_a_mdio,			1),
	GROUP(clk25m_a,				1),
	GROUP(pwm_c_z,				1),
	GROUP(pwm_d_z,				1),
	GROUP(i2c0_scl_z,			1),
	GROUP(i2c0_sda_z,			1),
	GROUP(eth_b_mdc,			1),
	GROUP(eth_b_mdio,			1),
	GROUP(clk25m_b,				1),
	/* Bank[Z]:Function[2] */
	GROUP(i2cs_a_scl,			2),
	GROUP(i2cs_a_sda,			2),
	/* Bank[Z]:Function[3] */
	GROUP(clk12m_24m_z,			3),
	GROUP(uart_c_tx_z,			3),
	GROUP(uart_c_rx_z,			3),
	GROUP(uart_c_cts_z,			3),
	GROUP(uart_c_rts_z,			3),
	/* Bank[Z]:Function[5] */
	GROUP(gen_clk_z,			5),
	/* Bank[Z]:Function[6] */
	GROUP(pio20,				6),
	GROUP(pio21,				6),
	GROUP(pio22,				6),
	GROUP(pio23,				6),
	GROUP(pio24,				6),
	GROUP(pio25,				6),
	GROUP(pio26,				6),
	GROUP(pio27,				6),
	GROUP(pio28,				6),
	GROUP(pio29,				6),
	/* Bank[Z]:Function[7] */
	GROUP(debug_i13,			7),
	GROUP(debug_i14,			7),
	GROUP(debug_i15,			7),
	GROUP(debug_i16,			7),
	GROUP(debug_i17,			7),
	GROUP(debug_i18,			7),
	GROUP(debug_i19,			7)
};

static const char * const gpio_periphs_groups[] = {
	"TEST_N",   "GPIOA_0",  "GPIOA_1",  "GPIOA_2",  "GPIOA_3",
	"GPIOA_4",  "GPIOA_5",  "GPIOA_6",  "GPIOA_7",  "GPIOA_8",
	"GPIOA_9",  "GPIOB_0",  "GPIOB_1",  "GPIOB_2",  "GPIOB_3",
	"GPIOB_4",  "GPIOB_5",  "GPIOB_6",  "GPIOB_7",  "GPIOB_8",
	"GPIOB_9",  "GPIOD_0",  "GPIOD_1",  "GPIOD_2",  "GPIOD_3",
	"GPIOD_4",  "GPIOD_5",  "GPIOD_6",  "GPIOD_7",  "GPIOD_8",
	"GPIOD_9",  "GPIOE_0",  "GPIOE_1",  "GPIOX_0",  "GPIOX_1",
	"GPIOX_2",  "GPIOX_3",  "GPIOX_4",  "GPIOX_5",  "GPIOX_6",
	"GPIOX_7",  "GPIOX_8",  "GPIOX_9",  "GPIOZ_0",  "GPIOZ_1",
	"GPIOZ_2",  "GPIOZ_3",  "GPIOZ_4",  "GPIOZ_5",  "GPIOZ_6",
	"GPIOZ_7",  "GPIOZ_8",  "GPIOZ_9",  "GPIOA_10", "GPIOB_10",
	"GPIOB_11", "GPIOB_12", "GPIOB_13", "GPIOD_10", "GPIOD_11",
	"GPIOD_12", "GPIOD_13", "GPIOD_14", "GPIOD_15", "GPIOD_16",
	"GPIOD_17", "GPIOD_18", "GPIOD_19"
};

static const char * const debug_groups[] = {
	"debug_i0",  "debug_i1",  "debug_i2",  "debug_i3",  "debug_i4",
	"debug_i5",  "debug_i6",  "debug_i7",  "debug_i8",  "debug_i9",
	"debug_o0",  "debug_o1",  "debug_o2",  "debug_o3",  "debug_o4",
	"debug_o5",  "debug_o6",  "debug_o7",  "debug_o8",  "debug_o9",
	"debug_i10", "debug_i11", "debug_i12", "debug_i13", "debug_i14",
	"debug_i15", "debug_i16", "debug_i17", "debug_i18", "debug_i19",
	"debug_o10", "debug_o11", "debug_o12", "debug_o13", "debug_o14",
	"debug_o15", "debug_o16", "debug_o17", "debug_o18", "debug_o19"
};

static const char * const clk12m_24m_groups[] = {
	"clk12m_24m", "clk12m_24m_z"
};

static const char * const emmc_groups[] = {
	"emmc_d0", "emmc_d1",  "emmc_d2",  "emmc_d3",
	"emmc_d4", "emmc_d5",  "emmc_d6",  "emmc_d7",
	"emmc_ds", "emmc_clk", "emmc_cmd", "emmc_rst_gpio"
};

static const char * const clk25m_groups[] = {
	"clk25m_a", "clk25m_b"
};

static const char * const eth_groups[] = {
	"eth_a_mdc", "eth_b_mdc", "eth_a_mdio", "eth_b_mdio"
};

static const char * const gen_clk_groups[] = {
	"gen_clk",   "gen_clk_a", "gen_clk_b", "gen_clk_d", "gen_clk_e",
	"gen_clk_x", "gen_clk_z"
};

static const char * const gphy_groups[] = {
	"gphy_led0_speed",       "gphy_led1_speed",       "gphy_led2_speed",
	"gphy_led3_speed",       "gphy_led0_link_active", "gphy_led1_link_active",
	"gphy_led2_link_active", "gphy_led3_link_active"
};

static const char * const i2c0_groups[] = {
	"i2c0_scl",    "i2c0_sda",    "i2c0_scl_z", "i2c0_sda_z",
	"i2c0_scl_od", "i2c0_sda_od"
};

static const char * const i2c1_groups[] = {
	"i2c1_scl", "i2c1_sda", "i2c1_scl_x", "i2c1_sda_x"
};

static const char * const i2c2_groups[] = {
	"i2c2_scl", "i2c2_sda"
};

static const char * const i2c_slave_groups[] = {
	"i2cs_a_scl", "i2cs_a_sda"
};

static const char * const jtag_groups[] = {
	"jtag_clk", "jtag_tdi", "jtag_tdo", "jtag_tms"
};

static const char * const isi_groups[] = {
	"isi_clk", "isi_miso", "isi_mosi"
};

static const char * const mclk_groups[] = {
	"mclk1"
};

static const char * const pcie_groups[] = {
	"pcie_a_ck_reqn",  "pcie_b_ck_reqn", "pcie_c_ck_reqn",
	"pcie_c_rstb_in）"
};

static const char * const pio_groups[] = {
	"pio0",  "pio1",  "pio2",  "pio3",  "pio4",  "pio5",
	"pio6",  "pio7",  "pio8",  "pio9",  "pio10", "pio11",
	"pio12", "pio13", "pio14", "pio15", "pio16", "pio17",
	"pio18", "pio19", "pio20", "pio21", "pio22", "pio23",
	"pio24", "pio25", "pio26", "pio27", "pio28", "pio29",
	"pio30", "pio31"
};

static const char * const pwm_a_groups[] = {
	"pwm_a"
};

static const char * const pwm_b_groups[] = {
	"pwm_b"
};

static const char * const pwm_c_groups[] = {
	"pwm_c_d", "pwm_c_z"
};

static const char * const pwm_d_groups[] = {
	"pwm_d", "pwm_d_z"
};

static const char * const pwm_e_groups[] = {
	"pwm_e", "pwm_e_a", "pwm_e_x"
};

static const char * const pwm_f_groups[] = {
	"pwm_f", "pwm_f_a", "pwm_f_x"
};

static const char * const spi_a_groups[] = {
	"spi_a_clk",    "spi_a_ss0",    "spi_a_ss1",    "spi_a_miso",
	"spi_a_mosi",   "spi_a_clk_a",  "spi_a_clk_b",  "spi_a_ss0_a",
	"spi_a_ss0_b",  "spi_a_ss1_a",  "spi_a_miso_a", "spi_a_miso_b",
	"spi_a_mosi_a", "spi_a_mosi_b"
};

static const char * const spi_b_groups[] = {
	"spi_b_clk",  "spi_b_ss0", "spi_b_ss1", "spi_b_miso",
	"spi_b_mosi"
};

static const char * const spinf_groups[] = {
	"spinf_clk",   "spinf_cs0",   "spinf_cs1",     "spinf_mi_d1",
	"spinf_mo_d0", "spinf_wp_d2", "spinf_hold_d3", "spinf_rst_gpio",
};

static const char * const tdm_groups[] = {
	"tdm_d0",    "tdm_d1",    "tdm_d2",    "tdm_d3",
	"tdm_fs1",   "tdm_fs2",   "tdm_d0_a",  "tdm_d1_a",
	"tdm_fs1_a", "tdm_sclk1", "tdm_sclk2", "tdm_sclk1_a"
};

static const char * const test_clk_groups[] = {
	"test_clk_1m", "test_clk_1m_x", "test_clk_1m_x3"
};

static const char * const tst_groups[] = {
	"tst_out0",      "tst_out1",      "tst_out2",     "tst_out3",
	"tst_out4",      "tst_out5",      "tst_out6",     "tst_out7",
	"tst_out8",      "tst_out9",      "tst_out10",    "tst_out11",
	"tst_out12",     "tst_out13",     "tst_out14",    "tst_out15",
	"tst_out16",     "tst_out17",     "tst_out18",    "tst_out19",
	"tst_loop1_in",  "tst_loop2_in",  "tst_loop3_in", "tst_loop1_out",
	"tst_loop2_out", "tst_loop3_out"
};

static const char * const uart_a_groups[] = {
	"uart_a_rx", "uart_a_tx"
};

static const char * const uart_b_groups[] = {
	"uart_b_rx_e", "uart_b_tx_e", "uart_b_cts_b", "uart_b_rts_b",
	"uart_b_rx_b", "uart_b_rx_x", "uart_b_tx_b", "uart_b_tx_x"
};

static const char * const uart_c_groups[] = {
	"uart_c_rx_a", "uart_c_tx_a", "uart_c_cts_a", "uart_c_rts_a",
	"uart_c_rx_z", "uart_c_tx_z", "uart_c_cts_z", "uart_c_rts_z"
};

static const char * const uart_d_groups[] = {
	"uart_d_rx_d", "uart_d_tx_d", "uart_d_cts_d", "uart_d_rts_d",
	"uart_d_rx_a", "uart_d_tx_a", "uart_d_cts_a", "uart_d_rts_a"
};

static const char * const zsi_groups[] = {
	"zsi_fs", "zsi_clk", "zsi_miso", "zsi_mosi"
};

static struct meson_pmx_func meson_x1_periphs_functions[] = {
	FUNCTION(gpio_periphs),
	FUNCTION(debug),
	FUNCTION(clk12m_24m),
	FUNCTION(emmc),
	FUNCTION(clk25m),
	FUNCTION(eth),
	FUNCTION(gen_clk),
	FUNCTION(gphy),
	FUNCTION(i2c0),
	FUNCTION(i2c1),
	FUNCTION(i2c2),
	FUNCTION(i2c_slave),
	FUNCTION(jtag),
	FUNCTION(isi),
	FUNCTION(mclk),
	FUNCTION(pcie),
	FUNCTION(pio),
	FUNCTION(pwm_a),
	FUNCTION(pwm_b),
	FUNCTION(pwm_c),
	FUNCTION(pwm_d),
	FUNCTION(pwm_e),
	FUNCTION(pwm_f),
	FUNCTION(spi_a),
	FUNCTION(spi_b),
	FUNCTION(spinf),
	FUNCTION(tdm),
	FUNCTION(test_clk),
	FUNCTION(tst),
	FUNCTION(uart_a),
	FUNCTION(uart_b),
	FUNCTION(uart_c),
	FUNCTION(uart_d),
	FUNCTION(zsi)
};

static struct meson_bank meson_x1_periphs_banks[] = {
	/*    name   first   last   irq   pullen   pull   dir   out   in   ds */
	BANK_DS("TEST",    TEST_N,    TEST_N,
		0x083,  0, 0x084,  0, 0x082,  0, 0x081,  0, 0x080,  0, 0x087,  0),
	BANK_DS("D",    GPIOD_0,  GPIOD_19,
		0x033,  0, 0x034,  0, 0x032,  0, 0x031,  0, 0x030,  0, 0x037,  0),
	BANK_DS("E",    GPIOE_0,   GPIOE_1,
		0x043,  0, 0x044,  0, 0x042,  0, 0x041,  0, 0x040,  0, 0x047,  0),
	BANK_DS("B",    GPIOB_0,  GPIOB_13,
		0x063,  0, 0x064,  0, 0x062,  0, 0x061,  0, 0x060,  0, 0x067,  0),
	BANK_DS("X",    GPIOX_0,   GPIOX_9,
		0x013,  0, 0x014,  0, 0x012,  0, 0x011,  0, 0x010,  0, 0x017,  0),
	BANK_DS("A",    GPIOA_0,  GPIOA_10,
		0x074,  0, 0x074,  0, 0x072,  0, 0x071,  0, 0x070,  0, 0x077,  0),
	BANK_DS("Z",    GPIOZ_0,   GPIOZ_9,
		0x003,  0, 0x004,  0, 0x002,  0, 0x001,  0, 0x000,  0, 0x007,  0)
};

static struct meson_pmx_bank meson_x1_periphs_pmx_banks[] = {
	/*       name      first        last         reg   offset */
	BANK_PMX("TEST",   TEST_N,      TEST_N,     0x007,  8),
	BANK_PMX("D",      GPIOD_0,    GPIOD_19,     0x010,  0),
	BANK_PMX("E",      GPIOE_0,     GPIOE_1,     0x012, 24),
	BANK_PMX("B",      GPIOB_0,    GPIOB_13,     0x000,  0),
	BANK_PMX("X",      GPIOX_0,     GPIOX_9,     0x003,  0),
	BANK_PMX("A",      GPIOA_0,    GPIOA_10,     0x008,  0),
	BANK_PMX("Z",      GPIOZ_0,     GPIOZ_9,     0x006,  0)
};

static struct meson_axg_pmx_data meson_x1_periphs_pmx_banks_data = {
	.pmx_banks	= meson_x1_periphs_pmx_banks,
	.num_pmx_banks	= ARRAY_SIZE(meson_x1_periphs_pmx_banks),
};

static int meson_x1_parse_dt_extra(struct meson_pinctrl *pc)
{
	pc->reg_ds = pc->reg_gpio;

	return 0;
}

static struct meson_pinctrl_data meson_x1_periphs_pinctrl_data = {
	.name		= "periphs-banks",
	.groups		= meson_x1_periphs_groups,
	.funcs		= meson_x1_periphs_functions,
	.banks		= meson_x1_periphs_banks,
	.num_pins	= 68,
	.num_groups	= ARRAY_SIZE(meson_x1_periphs_groups),
	.num_funcs	= ARRAY_SIZE(meson_x1_periphs_functions),
	.num_banks	= ARRAY_SIZE(meson_x1_periphs_banks),
	.gpio_driver	= DM_DRIVER_REF(meson_axg_gpio),
	.pmx_data	= &meson_x1_periphs_pmx_banks_data,
	.parse_dt	= &meson_x1_parse_dt_extra,
};

static const struct udevice_id meson_x1_pinctrl_match[] = {
	{
		.compatible = "amlogic,meson-x1-periphs-pinctrl",
		.data = (ulong)&meson_x1_periphs_pinctrl_data,
	},
	{ }
};

U_BOOT_DRIVER(meson_x1_pinctrl) = {
	.name	= "meson-x1-pinctrl",
	.id	= UCLASS_PINCTRL,
	.of_match = of_match_ptr(meson_x1_pinctrl_match),
	.probe = meson_pinctrl_probe,
	.priv_auto = sizeof(struct meson_pinctrl),
	.ops = &meson_axg_pinctrl_ops,
};
