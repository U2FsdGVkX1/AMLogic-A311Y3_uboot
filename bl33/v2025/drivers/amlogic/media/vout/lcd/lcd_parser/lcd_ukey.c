// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <malloc.h>
#include <amlogic/aml_model.h>
#include <amlogic/media/vout/lcd/lcd_vout.h>

#define LCDUKEY(fmt, args...)     printf("lcd: ukey: " fmt "", ## args)
#define LCDUKEYERR(fmt, args...)  printf("lcd: ukey err: " fmt "", ## args)

#define LCD_UKEY_RETRY_CNT_MAX   5

int lcd_ukey_get_size(const char *key_name)
{
	return model_check_ukey_size(key_name);
}

unsigned char *lcd_ukey_get_tcon(const char *key_name, int *len)
{
#ifdef CONFIG_AML_LCD_TCON
	unsigned char *buf;
	struct lcd_tcon_init_block_header_s *init_header;
	int retry_cnt = 0;
	unsigned int key_crc32, size = 0;

lcd_ukey_get_tcon_retry:
	buf = model_read_ukey_data(key_name, &size);
	if (!buf) {
		if (retry_cnt++ < LCD_UKEY_RETRY_CNT_MAX)
			goto lcd_ukey_get_tcon_retry;
		LCDUKEYERR("%s: %s failed\n", __func__, key_name);
		return NULL;
	}
	init_header = (struct lcd_tcon_init_block_header_s *)buf;
	if (size != init_header->block_size) {  //length check
		if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
			LCDUKEYERR("%s: %s data_len %d is not match key_len %d\n",
				   __func__, key_name, init_header->block_size, size);
		}
		if (retry_cnt++ < LCD_UKEY_RETRY_CNT_MAX) {
			memset(buf, 0, size);
			free(buf);
			goto lcd_ukey_get_tcon_retry;
		}
		goto lcd_ukey_get_tcon_err;
	}
	key_crc32 = cal_CRC32(0, &buf[4], (size - 4)); //except crc32
	if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
		LCDUKEY("%s: %s crc32: 0x%08x, header_crc32: 0x%08x\n",
			__func__, key_name, key_crc32, init_header->crc32);
	}
	if (key_crc32 != init_header->crc32) {  //crc32 check
		if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
			LCDUKEYERR("%s: %s header_crc32 0x%08x is not match 0x%08x\n",
				   __func__, key_name, init_header->crc32, key_crc32);
		}
		if (retry_cnt++ < LCD_UKEY_RETRY_CNT_MAX) {
			memset(buf, 0, size);
			free(buf);
			goto lcd_ukey_get_tcon_retry;
		}
		goto lcd_ukey_get_tcon_err;
	}

	*len = size;
	return buf;

lcd_ukey_get_tcon_err:
	LCDUKEYERR("%s: %s failed\n", __func__, key_name);
	memset(buf, 0, size);
	free(buf);
	return NULL;
#else
	LCDUKEYERR("Don't support tcon\n");
	return NULL;
#endif
}

int lcd_ukey_write(const char *key_name, unsigned char *buf, int len)
{
	model_write_ukey_data(key_name, buf, len);
	return 0;
}
