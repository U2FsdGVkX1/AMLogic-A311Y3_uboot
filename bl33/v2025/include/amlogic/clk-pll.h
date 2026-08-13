/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef _CLK_PLL_H
#define _CLK_PLL_H

#include <clk.h>

/* PLL Parameters */
struct pll_parm {
	u32 reg;
	u8 shift;
	u8 width;
};

struct pll_params_table {
	unsigned long	rate;
	unsigned int	m;
	unsigned int	n;
	unsigned int	od;
	unsigned int	frac;
};

#define PLL_RATE_PARAMS(_rate, _m, _n, _od)				\
	{								\
		.rate	= (_rate),					\
		.m	= (_m),						\
		.n	= (_n),						\
		.od	= (_od),					\
	}

#define PLL_RATE_PARAMS_FRAC(_rate, _m, _n, _od, _frac)			\
	{								\
		.rate	= (_rate),					\
		.m	= (_m),						\
		.n	= (_n),						\
		.od	= (_od),					\
		.frac	= (_frac),					\
	}

struct reg_sequence {
	unsigned int reg;
	unsigned int def;
	unsigned int delay_us;
};

#define CLK_AML_PLL_ROUND_CLOSEST			BIT(0)
#define CLK_AML_PLL_FORCE_INIT				BIT(1)
#define CLK_AML_PLL_FIXED_FRAC_WEIGHT_PRECISION		BIT(2)
#define CLK_AML_PLL_POWER_OF_TWO			BIT(3)
#define CLK_AML_PLL_M_EN0P5				BIT(5)
#define CLK_AML_PLL_RSTN				BIT(6)
#define CLK_AML_PLL_L_DETECTN				BIT(7)
#define CLK_AML_PLL_READ_ONLY				BIT(9)

struct aml_clk_pll {
	struct clk clk;
	struct pll_parm en;
	struct pll_parm l;
	struct pll_parm rst;
	struct pll_parm l_detect_en;
	struct pll_parm m;
	struct pll_parm n;
	struct pll_parm frac;
	struct pll_parm od;
	const struct reg_sequence *init_regs;
	unsigned int init_count;
	const struct pll_params_table *table;
	unsigned int table_count;
	unsigned int flags;
};

extern const struct clk_ops clk_pll_ops;

#define REG_PARM(_name, _reg, _shift, _width)				\
	_name = {							\
		.reg    = _reg,						\
		.shift  = _shift,					\
		.width  = _width,					\
}

#define CLK_PLL(_clkid, _name, _initregs, _init_count, _table, _table_count,\
		_en_reg, _en_shift, _en_width,				\
		_rst_reg, _rst_shift, _rst_width,			\
		_l_detect_en_reg, _l_detect_en_shift, _l_detect_en_width,\
		_l_reg, _l_shift, _l_width,				\
		_m_reg, _m_shift, _m_width,				\
		_n_reg, _n_shift, _n_width,				\
		_frac_reg, _frac_shift, _frac_width,			\
		_od_reg, _od_shift, _od_width,				\
		_pll_flags, _pname, _flags)				\
	[_clkid] = {							\
		.type = PLL,						\
		.name = _name,						\
		.parent_name = _pname,					\
		.data.pll = {						\
			.init_regs = _initregs,				\
			.init_count = _init_count,			\
			.table = _table,				\
			.table_count = _table_count,			\
			.REG_PARM(en, _en_reg, _en_shift, _en_width),	\
			.REG_PARM(rst, _rst_reg, _rst_shift, _rst_width),\
			.REG_PARM(l_detect_en, _l_detect_en_reg,	\
				  _l_detect_en_shift, _l_detect_en_width),\
			.REG_PARM(l, _l_reg, _l_shift, _l_width),	\
			.REG_PARM(m, _m_reg, _m_shift, _m_width),	\
			.REG_PARM(n, _n_reg, _n_shift, _n_width),	\
			.REG_PARM(frac, _frac_reg, _frac_shift, _frac_width),\
			.REG_PARM(od, _od_reg, _od_shift, _od_width),	\
			.flags = _pll_flags,				\
	},								\
	.flags = _flags,						\
}

struct clk *aml_clk_register_pll(const char *name, const char *parent_name,
				 unsigned long flags,
				 struct aml_clk_pll *data);

#endif
