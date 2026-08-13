// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <malloc.h>
#include <spi.h>
#include <asm/amlogic/arch/gpio.h>
#include <fdtdec.h>
#include <amlogic/media/vout/lcd/aml_lcd.h>
#include <amlogic/media/vout/lcd/bl_ldim.h>
#include "../../lcd_common.h"
#include "ldim_drv.h"
#include "ldim_dev_drv.h"

#define NORMAL_MSG      (0 << 7)
#define BROADCAST_MSG   BIT(7)
#define BLOCK_DATA      (0 << 6)
#define SINGLE_DATA     BIT(6)
#define BLMCU_CLASS_NAME "blmcu"

#define VSYNC_INFO_FREQUENT        300

struct blmcu_s {
	unsigned int dev_on_flag;
	unsigned short vsync_cnt;
	unsigned int spi_dev_num;
	struct spi_slave *spi;
	unsigned int rbuf_size;
	unsigned int tbuf_size;
	unsigned char header_cnt;
	unsigned char *header_data;
	unsigned char extend_cnt;
	unsigned char *extend_data;
	unsigned char adim_width;
	unsigned char pdim_width;
	unsigned short adim;	/*1byte 0x4d 30%duty */
	unsigned short pdim;	/*1byte 0xff */
	unsigned char datawidth;	/*0:8bit, 1:12bit, 2:16bit */
	unsigned char ext_len;
	unsigned char bl_pwm_en;	/*0:default uboot have pwm, 1: uboot without pwm*/
	unsigned int apl;
	unsigned int duty_size;
	unsigned int zone_num;

	/* local dimming driver smr api usage */
	unsigned char *rbuf;
	unsigned char *tbuf;

	/*for 2nd spi*/
	struct spi_slave *spi1;
	unsigned int rbuf_size1;
	unsigned int tbuf_size1;
	unsigned char *rbuf1;
	unsigned char *tbuf1;
};

struct blmcu_s *bl_mcu;

static int blmcu_hw_init_on(struct ldim_dev_driver_s *dev_drv)
{
	if (!dev_drv) {
		LDIMERR("%s: dev_drv is null\n", __func__);
		return -1;
	}

	LDIMPR("%s\n", __func__);

	/* step 1: system power_on */
	ldim_gpio_set(dev_drv, dev_drv->en_gpio, dev_drv->en_gpio_on);

	/* step 2: delay for internal logic stable */
	mdelay(dev_drv->hw_on_delay); // default 500ms

	/* step 3: Generate external VSYNC to VSYNC/PWM pin */
	ldim_set_duty_pwm(&dev_drv->ldim_pwm_config);
	ldim_set_duty_pwm(&dev_drv->analog_pwm_config);
	dev_drv->pinmux_ctrl(dev_drv, 1);

	return 0;
}

static int blmcu_hw_init_off(struct ldim_dev_driver_s *dev_drv)
{
	ldim_gpio_set(dev_drv, dev_drv->en_gpio, dev_drv->en_gpio_off);
	dev_drv->pinmux_ctrl(dev_drv, 0);
	ldim_pwm_off(&dev_drv->ldim_pwm_config);
	ldim_pwm_off(&dev_drv->analog_pwm_config);

	return 0;
}

static inline void ldim_data_mapping(unsigned short *duty_buf,
				     unsigned int max, unsigned int min,
				     unsigned int zone_num)
{
	unsigned int i, j, val, apl, k;

	j = 0;
	apl = 0;
	for (i = 0; i < zone_num; i++) {
		val = duty_buf[i];
		apl += val;
		if (bl_mcu->datawidth == 16) { //16bits (6+10)
			//bl_mcu->rbuf[j + 1] = (val >> 8) & 0xff;
			//bl_mcu->rbuf[j] = val & 0xff;
			bl_mcu->rbuf[j] = ((bl_mcu->adim << 2) | (val >> 10)) & 0xff;
			bl_mcu->rbuf[j + 1] = (val >> 2) & 0xff;
			j += 2;
		} else if (bl_mcu->datawidth == 12) { //12bits
			if (i % 2 == 0) {
				bl_mcu->rbuf[j] = (val >> 4) & 0xff;
				bl_mcu->rbuf[j + 1] = ((val & 0xf) << 4) & 0xff;
			} else {
				bl_mcu->rbuf[j + 1] |= (val >> 8) & 0xf;
				bl_mcu->rbuf[j + 2] = val & 0xff;
				j += 3;
			}
		} else {
			bl_mcu->rbuf[j] = (val >> 4) & 0xff;
			j++;
		}
	}

	bl_mcu->apl = apl / zone_num;

	if (bl_mcu->header_data) {
		for (i = 0; i < bl_mcu->header_cnt; i++)
			bl_mcu->tbuf[i] = bl_mcu->header_data[i];
	}

	for (i = 0; i < bl_mcu->duty_size; i++)
		bl_mcu->tbuf[i + bl_mcu->header_cnt] = bl_mcu->rbuf[i];

	k = bl_mcu->duty_size + bl_mcu->header_cnt;
	switch (bl_mcu->extend_cnt) {
	case 3:// 3byte tail
		bl_mcu->tbuf[k + 0] = bl_mcu->pdim;
		bl_mcu->tbuf[k + 1] = bl_mcu->adim;
		bl_mcu->tbuf[k + 2] = (bl_mcu->apl >> 4) & 0xff;  //apl
		break;
	case 6:// 6byte tail
		bl_mcu->tbuf[k + 0] = bl_mcu->pdim;
		bl_mcu->tbuf[k + 1] = bl_mcu->adim;
		bl_mcu->tbuf[k + 2] = (bl_mcu->apl >> 8) & 0xff;  //apl
		bl_mcu->tbuf[k + 3] = bl_mcu->apl & 0xff;  //apl
		bl_mcu->tbuf[k + 4] = 0xff;  //reseve
		bl_mcu->tbuf[k + 5] = 0xff;  //reserve
		break;
	default:
		break;
	}

	k = bl_mcu->duty_size + bl_mcu->ext_len;
	for (i = k; i < bl_mcu->tbuf_size; i++)
		bl_mcu->tbuf[i] = 0;

	/*for 2nd spi*/
	/* here For reference only, need to be set according to the actual device spec!!!*/
	if (bl_mcu->spi_dev_num == 2)
		memcpy(bl_mcu->tbuf1, bl_mcu->tbuf, bl_mcu->tbuf_size);

}

static int blmcu_smr(struct aml_ldim_driver_s *ldim_drv, unsigned short *buf,
		      unsigned int len)
{
	struct ldim_dev_driver_s *dev_drv = ldim_drv->dev_drv;
	int ret = 0;

	if (!bl_mcu)
		return -1;

	if (bl_mcu->vsync_cnt++ >= VSYNC_INFO_FREQUENT)
		bl_mcu->vsync_cnt = 0;

	if (bl_mcu->dev_on_flag == 0) {
		if (bl_mcu->vsync_cnt == 0)
			LDIMPR("%s: on_flag=%d\n", __func__, bl_mcu->dev_on_flag);
		return 0;
	}
	if (len != dev_drv->zone_num) {
		if (bl_mcu->vsync_cnt == 0)
			LDIMERR("%s: data len %d invalid\n", __func__, len);
		return -1;
	}
	if (!bl_mcu->rbuf) {
		if (bl_mcu->vsync_cnt == 0)
			LDIMERR("%s: rbuf is null\n", __func__);
		return -1;
	}
	if (!bl_mcu->tbuf) {
		if (bl_mcu->vsync_cnt == 0)
			LDIMERR("%s: tbuf is null\n", __func__);
		return -1;
	}

	ldim_data_mapping(buf, dev_drv->dim_max, dev_drv->dim_min,
				  dev_drv->zone_num);

	ret = ldim_spi_write(bl_mcu->spi, bl_mcu->tbuf, bl_mcu->tbuf_size);

	/*for 2nd spi*/
	if (bl_mcu->spi_dev_num == 2)
		ret |= ldim_spi_write(bl_mcu->spi1, bl_mcu->tbuf1, bl_mcu->tbuf_size1);

	return ret;
}

static int blmcu_power_on(struct aml_ldim_driver_s *ldim_drv)
{
	if (!bl_mcu)
		return -1;

	if (bl_mcu->dev_on_flag) {
		LDIMPR("%s: blmcu is already on, exit\n", __func__);
		return 0;
	}

	blmcu_hw_init_on(ldim_drv->dev_drv);
	bl_mcu->dev_on_flag = 1;
	bl_mcu->vsync_cnt = 0;

	LDIMPR("%s: ok\n", __func__);
	return 0;
}

static int blmcu_power_off(struct aml_ldim_driver_s *ldim_drv)
{
	if (!bl_mcu)
		return -1;

	bl_mcu->dev_on_flag = 0;
	blmcu_hw_init_off(ldim_drv->dev_drv);

	LDIMPR("%s: ok\n", __func__);
	return 0;
}

static int blmcu_ldim_dev_update(struct ldim_dev_driver_s *dev_drv)
{
	dev_drv->power_on = blmcu_power_on;
	dev_drv->power_off = blmcu_power_off;
	dev_drv->dev_smr = blmcu_smr;

	dev_drv->reg_write = NULL;
	dev_drv->reg_read = NULL;
	return 0;
}

int ldim_dev_blmcu_probe(struct aml_ldim_driver_s *ldim_drv)
{
	struct ldim_dev_driver_s *dev_drv = ldim_drv->dev_drv;

	if (!dev_drv) {
		LDIMERR("%s: dev_drv is null\n", __func__);
		return -1;
	}

	bl_mcu = (struct blmcu_s *)malloc(sizeof(struct blmcu_s));
	if (!bl_mcu) {
		LDIMERR("blmcu malloc error\n");
		return -1;
	}
	memset(bl_mcu, 0, sizeof(struct blmcu_s));

	bl_mcu->spi_dev_num = dev_drv->spi_info.spi_dev_num;
	bl_mcu->spi = dev_drv->spi_info.spi[0];
	bl_mcu->dev_on_flag = 0;
	bl_mcu->vsync_cnt = 0;
	bl_mcu->zone_num = dev_drv->zone_num;

	bl_mcu->header_cnt = dev_drv->header_cnt;
	bl_mcu->header_data = dev_drv->header_data;
	bl_mcu->extend_cnt = dev_drv->extend_cnt;
	bl_mcu->extend_data = dev_drv->extend_data;
	bl_mcu->adim_width = dev_drv->adim_width;
	bl_mcu->pdim_width = dev_drv->pdim_width;
	bl_mcu->adim = dev_drv->adim;
	bl_mcu->pdim = dev_drv->pdim;
	bl_mcu->ext_len = bl_mcu->header_cnt + bl_mcu->extend_cnt;
	bl_mcu->datawidth = bl_mcu->adim_width + bl_mcu->pdim_width;

	if (bl_mcu->datawidth == 16)
		bl_mcu->duty_size = 2 * bl_mcu->zone_num;
	else if (bl_mcu->datawidth == 12)
		bl_mcu->duty_size = (bl_mcu->zone_num * 3 + 1) / 2;
	else //8bit
		bl_mcu->tbuf_size = bl_mcu->zone_num;

	/*packet length*/
	bl_mcu->tbuf_size = bl_mcu->duty_size + bl_mcu->ext_len;

	LDIMPR("%s: bl_mcu zone_num=%d,header_cnt=%d, extend_cnt=%d,\n"
		"adim_width=%d, pdim_width=%d, duty_size=%d, tbuf_size=%d\n",
		__func__, bl_mcu->zone_num, bl_mcu->header_cnt, bl_mcu->extend_cnt,
		bl_mcu->adim_width, bl_mcu->pdim_width, bl_mcu->duty_size, bl_mcu->tbuf_size);

	/* each zone 2 bytes */
	bl_mcu->rbuf_size = bl_mcu->tbuf_size;
	bl_mcu->rbuf = (unsigned char *)malloc(bl_mcu->rbuf_size);
	if (!bl_mcu->rbuf)
		goto ldim_dev_blmcu_probe_err0;
	memset(bl_mcu->rbuf, 0, bl_mcu->rbuf_size);

	bl_mcu->tbuf = (unsigned char *)malloc(bl_mcu->tbuf_size);
	if (!bl_mcu->tbuf)
		goto ldim_dev_blmcu_probe_err1;
	memset(bl_mcu->tbuf, 0, bl_mcu->tbuf_size);

	/* for 2nd spi */
	/* here For reference only, need to be set according to the actual device spec!!!*/
	if (bl_mcu->spi_dev_num == 2) {
		bl_mcu->spi1 = dev_drv->spi_info.spi[1];
		bl_mcu->tbuf_size1 = bl_mcu->tbuf_size;
		bl_mcu->rbuf_size1 = bl_mcu->rbuf_size;

		bl_mcu->rbuf1 = (unsigned char *)malloc(bl_mcu->rbuf_size1);
		if (!bl_mcu->rbuf1)
			goto ldim_dev_blmcu_probe_err2;
		memset(bl_mcu->rbuf1, 0, bl_mcu->rbuf_size1);

		bl_mcu->tbuf1 = (unsigned char *)malloc(bl_mcu->tbuf_size1);
		if (!bl_mcu->tbuf1)
			goto ldim_dev_blmcu_probe_err3;
		memset(bl_mcu->tbuf1, 0, bl_mcu->tbuf_size1);
	}

	blmcu_ldim_dev_update(dev_drv);

	LDIMPR("%s ok\n", __func__);
	return 0;

ldim_dev_blmcu_probe_err3:
	free(bl_mcu->rbuf1);
	bl_mcu->rbuf_size1 = 0;
ldim_dev_blmcu_probe_err2:
	free(bl_mcu->tbuf);
	bl_mcu->tbuf_size = 0;
ldim_dev_blmcu_probe_err1:
	free(bl_mcu->rbuf);
	bl_mcu->rbuf_size = 0;
ldim_dev_blmcu_probe_err0:
	free(bl_mcu);
	bl_mcu = NULL;
	return -1;
}

int ldim_dev_blmcu_remove(struct aml_ldim_driver_s *ldim_drv)
{
	if (!bl_mcu)
		return 0;

	free(bl_mcu->rbuf);
	bl_mcu->rbuf_size = 0;
	free(bl_mcu->tbuf);
	bl_mcu->tbuf_size = 0;
	free(bl_mcu);
	bl_mcu = NULL;

	return 0;
}
