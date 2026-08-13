// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2025 Amlogic, Inc. All rights reserved.
 */
#include <amlogic/clk-core.h>
#include <log.h>
#include <dm/device.h>

int aml_clk_register_all(struct aml_clk_data *clks, int num_clks)
{
	struct clk *clk;
	int i;

	if (!clks)
		return -EFAULT;

	for (i = 0; i < num_clks; i++, clks++) {
		if (clks->type == 0)
			continue;

		switch (clks->type) {
		case MUX:
			clk = clk_register_mux_table(NULL, clks->name,
						     clks->data.mux.parent_names,
						     clks->data.mux.num_parents,
						     clks->flags,
						     clks->data.mux.reg,
						     clks->data.mux.shift,
						     clks->data.mux.mask,
						     clks->data.mux.flags,
						     clks->data.mux.table);
			break;

		case GATE:
			clk = clk_register_gate(NULL, clks->name,
						clks->parent_name, clks->flags,
						clks->data.gate.reg,
						clks->data.gate.bit_idx,
						clks->data.gate.flags,
						NULL);
			break;

		case DIVIDER:
			clk = clk_register_divider(NULL, clks->name,
						   clks->parent_name,
						   clks->flags,
						   clks->data.divider.reg,
						   clks->data.divider.shift,
						   clks->data.divider.width,
						   clks->data.divider.flags);
			break;

		case FIXED_FACTOR:
			clk = clk_register_fixed_factor(NULL, clks->name,
							clks->parent_name,
							clks->flags,
							clks->data.fixed_factor.mult,
							clks->data.fixed_factor.div);
			break;

		case FIXED_RATE:
			clk = clk_register_fixed_rate(NULL, clks->name,
						      clks->data.fixed_rate.fixed_rate);
			break;

		case PLL:
			clk = aml_clk_register_pll(clks->name, clks->parent_name,
						   clks->flags, &clks->data.pll);
			break;

		default:
			log_err("%s: unknown clock type\n", clks->name);
			return -EINVAL;
		}
		if (IS_ERR(clk)) {
			log_err("%s registration failed\n", clks->name);

			return -EFAULT;
		}

		clk_dm(i, clk);
	}

	return 0;
}

ulong aml_clk_get_rate(struct clk *clk)
{
	struct clk *c;
	const struct clk_ops *ops;
	int err = clk_get_by_id(clk->id, &c);

	if (err)
		return err;

	ops = (const struct clk_ops *)c->dev->driver->ops;
	if (!ops->get_rate)
		return -ENOSYS;

	return ops->get_rate(c);
}

ulong aml_clk_set_rate(struct clk *clk, unsigned long rate)
{
	struct clk *c;
	const struct clk_ops *ops;
	int err = clk_get_by_id(clk->id, &c);

	if (err)
		return err;

	ops = (const struct clk_ops *)c->dev->driver->ops;
	if (!ops->set_rate)
		return -ENOSYS;

	return ops->set_rate(c, rate);
}

int aml_clk_set_parent(struct clk *clk, struct clk *parent)
{
	struct clk *c, *p;
	const struct clk_ops *ops;
	int err = clk_get_by_id(clk->id, &c);

	if (err)
		return err;

	err = clk_get_by_id(parent->id, &p);
	if (err)
		return err;

	ops = (const struct clk_ops *)c->dev->driver->ops;
	if (!ops->set_parent)
		return -ENOSYS;

	return ops->set_parent(c, p);
}

static int aml_clk_enable(struct clk *clk)
{
	struct clk *c;
	const struct clk_ops *ops;
	int err = clk_get_by_id(clk->id, &c);

	if (err)
		return err;

	ops = (const struct clk_ops *)c->dev->driver->ops;
	if (!ops->enable)
		return -ENOSYS;

	return ops->enable(c);
}

static int aml_clk_disable(struct clk *clk)
{
	struct clk *c;
	const struct clk_ops *ops;
	int err = clk_get_by_id(clk->id, &c);

	if (err)
		return err;

	ops = (const struct clk_ops *)c->dev->driver->ops;
	if (!ops->disable)
		return -ENOSYS;

	return ops->disable(c);
}

const struct clk_ops aml_clk_ops = {
	.set_rate = aml_clk_set_rate,
	.get_rate = aml_clk_get_rate,
	.set_parent = aml_clk_set_parent,
	.enable = aml_clk_enable,
	.disable = aml_clk_disable,
};
