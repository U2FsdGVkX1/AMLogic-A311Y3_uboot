// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2024 Amlogic, Inc. All rights reserved.
 */

#include <stdio.h>
#include <i2c.h>

#define I2C_CHARGER_BUS_NUM		0
#define I2C_CHARGER_ADDR		0x6b
#define I2C_CHARGER_ADDR_LEN		1

#define SGM41542S_INPUT_CURR_LIMIT	0x00
#define SGM41542S_IINDPM_I_MASK		0x3f
#define SGM41542S_IINDPM_I_MAX_uA_BIT	0x20 //3.3A
#define SGM_LOG "sgm41542s "

int charger_power_init(void)
{
	struct udevice *dev;
	int ret;
	uint8_t data;

	printf(SGM_LOG "charger ic sgm41542s set ibus to max:\n");

	ret = i2c_get_chip_for_busnum(I2C_CHARGER_BUS_NUM, I2C_CHARGER_ADDR,
				      I2C_CHARGER_ADDR_LEN, &dev);
	if (ret) {
		printf(SGM_LOG "Cannot find CHARGER IC!\n");
		return ret;
	}

	ret = dm_i2c_read(dev, SGM41542S_INPUT_CURR_LIMIT, &data, sizeof(data));
	if (ret) {
		printf(SGM_LOG "dm_i2c_read fail!\n");
		return ret;
	}
	printf(SGM_LOG "charger get default 0x0:0x%x.\n", data);

	data &= ~SGM41542S_IINDPM_I_MASK;
	data |= SGM41542S_IINDPM_I_MAX_uA_BIT & SGM41542S_IINDPM_I_MASK;

	ret = dm_i2c_write(dev, SGM41542S_INPUT_CURR_LIMIT, &data, sizeof(data));
	if (ret) {
		printf(SGM_LOG "dm_i2c_write fail!\n");
		return ret;
	}

	ret = dm_i2c_read(dev, SGM41542S_INPUT_CURR_LIMIT, &data, sizeof(data));
	if (ret) {
		printf(SGM_LOG "dm_i2c_read fail!\n");
		return ret;
	}
	printf(SGM_LOG "charger get after 0x0:0x%x.\n", data);

	return 0;
}