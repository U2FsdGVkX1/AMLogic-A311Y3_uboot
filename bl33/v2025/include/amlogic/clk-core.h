/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2025 Amlogic, Inc. All rights reserved.
 */

#ifndef _AML_CLK_CORE_H
#define _AML_CLK_CORE_H

#include <linux/types.h>
#include <linux/clk-provider.h>
#include "clk-pll.h"

enum clk_type {
	MUX = 1,
	GATE,
	DIVIDER,
	FIXED_FACTOR,
	FIXED_RATE,
	PLL,
};

struct aml_clk_data {
	enum clk_type		type;
	const char		*name;
	const char		*parent_name;
	union {
		struct clk_mux		mux;
		struct clk_gate		gate;
		struct clk_divider	divider;
		struct clk_fixed_factor	fixed_factor;
		struct clk_fixed_rate	fixed_rate;
		struct aml_clk_pll	pll;
	} data;
	u32			flags;
};

#define CLK_FIXED_RATE(_clkid, _name, _rate)				\
	[_clkid] = {							\
		.type = FIXED_RATE,					\
		.name = _name,						\
		.data.fixed_rate = {					\
			.fixed_rate = _rate,				\
		},							\
	}

#define CLK_FIXED_FACTOR(_clkid, _name, _mult, _div, _pname)		\
	[_clkid] = {							\
		.type = FIXED_FACTOR,					\
		.name = _name,						\
		.parent_name = _pname,					\
		.data.fixed_factor = {					\
			.mult = _mult,					\
			.div = _div,					\
		},							\
	}

#define CLK_GATE(_clkid, _name, _reg, _bit, _pname, _flags)		\
	[_clkid] = {							\
		.type = GATE,						\
		.name = _name,						\
		.parent_name = _pname,					\
		.data.gate = {						\
			.reg = (void __iomem *)_reg,			\
			.bit_idx = _bit,				\
		},							\
		.flags = _flags,					\
	}

#define CLK_MUX(_clkid, _name, _reg, _mask, _shift, _pnames,		\
		_num_pnames, _table, _flags)				\
	[_clkid] = {							\
		.type = MUX,						\
		.name = _name,						\
		.data.mux = {						\
			.reg = (void __iomem *)_reg,			\
			.mask = _mask,					\
			.shift = _shift,				\
			.parent_names = _pnames,			\
			.num_parents = _num_pnames,			\
			.table = _table,				\
		},							\
		.flags = _flags,					\
	}

#define CLK_DIVIDER(_clkid, _name, _reg, _shift, _width, _dflags,	\
		    _table, _pname, _flags)				\
	[_clkid] = {							\
		.type = DIVIDER,					\
		.name = _name,						\
		.parent_name = _pname,					\
		.data.divider = {					\
			.reg = (void __iomem *)_reg,			\
			.shift = _shift,				\
			.width = _width,				\
			.flags = _dflags,				\
			.table = _table,				\
		},							\
		.flags = _flags,					\
	}

extern const struct clk_ops aml_clk_ops;

int aml_clk_register_all(struct aml_clk_data *clks, int num_clks);

#endif /* _AML_CLK_CORE_H */
