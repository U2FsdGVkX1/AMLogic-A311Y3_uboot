// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <malloc.h>
#include <string.h>
#include <amlogic/aml_model.h>
#include <amlogic/media/vout/lcd/aml_lcd.h>
#include "../lcd_common.h"

#define LCD_INI_PARSER_MEM_SIZE         204800 //200k

#define AML_START     "amlogic_start"
#define AML_END       "amlogic_end"

void *lcd_ini_get_section(void *inip, const char *section)
{
	return handle_ini_get_section(inip, section);
}

const char *lcd_ini_get_str(void *inip, void *psec, const char *key, const char *def_val)
{
	return handle_ini_get_str(inip, psec, key, def_val);
}

unsigned int lcd_ini_get_val(void *inip, void *psec, const char *key, unsigned int def_val)
{
	return handle_ini_get_val(inip, psec, key, def_val);
}

int lcd_ini_get_array_cnt(void *inip, void *psec, const char *key)
{
	const char *str;

	str = handle_ini_get_str(inip, psec, key, NULL);
	if (!str)
		return -1;

	return lcd_get_str_array_cnt(str);
}

int lcd_ini_get_array(void *inip, void *psec, const char *key, unsigned int *buf, unsigned int cnt)
{
	const char *str;

	if (!buf)
		return 0;

	str = handle_ini_get_str(inip, psec, key, NULL);
	if (!str)
		return -1;

	return lcd_trans_str_array(str, buf, cnt);
}

unsigned int lcd_ini_get_array_index(void *inip, void *psec, const char *key,
				     unsigned int index, unsigned int def_val)
{
	const char *str;

	str = handle_ini_get_str(inip, psec, key, NULL);
	if (!str)
		return def_val;

	return lcd_get_str_array_index(str, index, def_val);
}

int lcd_ini_set_exist_single_key(void *inip, void *psec, const char *key, const char *str)
{
	return handle_ini_set_exist_single_key(inip, psec, key, str);
}

int lcd_ini_set_exist_keys(void *inip, void *psec,
			   const char *new_line, const char *new_value, int line_cnt)
{
	return handle_ini_set_exist_keys(inip, psec, new_line, new_value, line_cnt);
}

static int lcd_ini_integrity_flag(void *inip)
{
	void *psec;
	const char *str = NULL;

	if (!inip)
		return -1;

	psec = lcd_ini_get_section(inip, "start");
	str = lcd_ini_get_str(inip, psec, "start_tag", "null");
	if (strcasecmp(str, AML_START)) {
		LCDERR("%s: start_tag (%s) is error!\n", __func__, str);
		return -1;
	}

	psec = lcd_ini_get_section(inip, "end");
	str = lcd_ini_get_str(inip, psec, "end_tag", "null");
	if (strcasecmp(str, AML_END)) {
		LCDERR("%s: end_tag (%s) is error!\n", __func__, str);
		return -1;
	}

	return 0;
}

static void lcd_ini_load_panel_misc(void *inip)
{
	struct panel_misc_s misc_attr;
	void *psec;
	const char *str = NULL;
	const char *outputmode[3] = {"outputmode", "outputmode2", "outputmode3"};
	const char *connector[3] = {"connector0_type", "connector1_type", "connector2_type"};
	int i;

	psec = lcd_ini_get_section(inip, "panel_misc");
	if (!psec)
		return;

	memset(&misc_attr, 0, sizeof(misc_attr));
	misc_attr.version = 1;
	misc_attr.disp_idx = 0xff;
	strlcpy(misc_attr.outputmode, "null", sizeof(misc_attr.outputmode));
	strlcpy(misc_attr.connector_type, "null", sizeof(misc_attr.connector_type));

	for (i = 0; i < 3; i++) {
		str = lcd_ini_get_str(inip, psec, outputmode[i], NULL);
		if (str) {
			misc_attr.disp_idx = i;
			strlcpy(misc_attr.outputmode, str, sizeof(misc_attr.outputmode));
			break;
		}
	}

	str = lcd_ini_get_str(inip, psec, "connector_type", NULL);
	if (str) {
		strlcpy(misc_attr.connector_type, str, sizeof(misc_attr.connector_type));
		goto handle_panel_misc_next;
	}

	for (i = 0; i < 3; i++) {
		str = lcd_ini_get_str(inip, psec, connector[i], NULL);
		if (str) {
			misc_attr.disp_idx = i;
			strlcpy(misc_attr.connector_type, str, sizeof(misc_attr.connector_type));
			break;
		}
	}

handle_panel_misc_next:
	str = lcd_ini_get_str(inip, psec, "panel_reverse", "null");
	strlcpy(misc_attr.panel_reverse, str, sizeof(misc_attr.panel_reverse));

	str = lcd_ini_get_str(inip, psec, "display_layer", "null");
	strlcpy(misc_attr.display_layer, str, sizeof(misc_attr.display_layer));

	model_set_panel_misc(&misc_attr);
}

void *lcd_ini_file_parse(int index)
{
	void *local_ini_mem;
	const char *file_name, *file_bl, *file_alt_name;
	/* pre-set_model as new */
	void *inip, *psec_src, *inip_bl;
	/* panel-alt_model as current */
	void *inip_alt, *psec_dst, *psec_alt_attr;
	char alt_sec_key[32];
	const char *str;
	int i, ret = 0;

	file_name = get_panel_file_path(index);
	if (!file_name)
		return NULL;

	rm_panel_file_parse_mem(index);
	inip = handle_ini_file_parse(file_name, LCD_INI_PARSER_MEM_SIZE);
	if (!inip)
		return NULL;
	local_ini_mem = inip; //default pre-set_model

	/* backlight detect */
	if (get_bl_file_type(index) != PANEL_FILE_INI)
		goto lcd_ini_file_parse_done;
	file_bl = get_bl_file_path(index);
	if (!file_bl)
		goto lcd_ini_file_parse_done;
	inip_bl = handle_ini_file_parse(file_bl, LCD_INI_PARSER_MEM_SIZE);
	if (!inip_bl)
		goto lcd_ini_file_parse_done;
	psec_src = lcd_ini_get_section(inip_bl, "Backlight_Attr");
	if (!psec_src) {
		handle_ini_parser_uninit(inip_bl);
		goto lcd_ini_file_parse_done;
	}
	psec_dst = lcd_ini_get_section(inip, "Backlight_Attr");
	ret = handle_ini_write_new_section(inip, psec_dst, inip_bl, psec_src);
	LCDPR("[%d]: copy Backlight_Attr to panel: %s\n", index, ret ? "fail" : "ok");
	handle_ini_parser_uninit(inip_bl);

lcd_ini_file_parse_done:
	/* panel_alt detect */
	if (get_panel_alt_file_type(index) != PANEL_FILE_INI)
		goto lcd_ini_file_parse_next;
	file_alt_name = get_panel_alt_file_path(index);
	if (!file_alt_name)
		goto lcd_ini_file_parse_next;
	inip_alt = handle_ini_file_parse(file_alt_name, LCD_INI_PARSER_MEM_SIZE);
	if (!inip_alt)
		goto lcd_ini_file_parse_next;
	psec_alt_attr = lcd_ini_get_section(inip_alt, "alternate_Attr");
	if (!psec_alt_attr) {
		handle_ini_parser_uninit(inip_alt);
		goto lcd_ini_file_parse_next;
	}
	i = 0;
	snprintf(alt_sec_key, 32, "alt_section_%d", i);
	str = lcd_ini_get_str(inip_alt, psec_alt_attr, alt_sec_key, NULL);
	while (str) {
		psec_dst = lcd_ini_get_section(inip_alt, str);
		psec_src = lcd_ini_get_section(inip, str);
		ret = handle_ini_write_new_section(inip_alt, psec_dst, inip, psec_src);
		LCDPR("[%d]: copy %s to panel: %s\n", index, str, ret ? "fail" : "ok");

		i++;
		snprintf(alt_sec_key, 32, "alt_section_%d", i);
		str = lcd_ini_get_str(inip_alt, psec_alt_attr, alt_sec_key, NULL);
	};
	local_ini_mem = inip_alt; //use panel-alt_model
	handle_ini_parser_uninit(inip); //release pre-set model

lcd_ini_file_parse_next:
	ret = lcd_ini_integrity_flag(local_ini_mem);
	if (ret) {
		handle_ini_parser_uninit(local_ini_mem);
		return NULL;
	}

	set_panel_file_parse_mem(index, local_ini_mem, LCD_INI_PARSER_MEM_SIZE, PANEL_FILE_INI);

	lcd_ini_load_panel_misc(local_ini_mem);

	return local_ini_mem;
}

void *get_lcd_ini_parse_mem(int index)
{
	if (get_lcd_panel_file_type(index) != PANEL_FILE_INI)
		return NULL;

	return get_panel_file_parse_mem(index);
}

void lcd_ini_mem_free(void *parse_mem)
{
	if (parse_mem)
		handle_ini_parser_uninit(parse_mem);
}

int lcd_ini_param_mem_save(void *parse_mem, int index)
{
	int buf_size, ret;
	char name[16];

	if (!parse_mem) {
		LCDERR("[%d]: %s: parse_mem is NULL\n", index, __func__);
		return -1;
	}

	buf_size = handle_ini_parser_mem_get_size(parse_mem);
	if (buf_size <= 0) {
		LCDERR("[%d]: %s: mem size error\n", index, __func__);
		return -1;
	}

	/* save to reserved memory for kernel use */
	sprintf(name, "panel%d_ini", index);
	ret = panel_param_mem_put(parse_mem, name, buf_size);
	return ret;
}

int lcd_ini_param_mem_save_update(void *parse_mem, int index)
{
	int buf_size, ret;
	char name[16];

	if (!parse_mem) {
		LCDERR("[%d]: %s: parse_mem is NULL\n", index, __func__);
		return -1;
	}

	buf_size = handle_ini_parser_mem_get_size(parse_mem);
	if (buf_size <= 0) {
		LCDERR("[%d]: %s: mem size error\n", index, __func__);
		return -1;
	}

	/* save to reserved memory for kernel use */
	sprintf(name, "panel%d_ini", index);
	panel_param_mem_modify(parse_mem, name, buf_size);
	update_panel_param_to_kernel();

	return ret;
}

void lcd_ini_list_key_value(int index)
{
	void *parse_mem = get_lcd_ini_parse_mem(index);

	if (!parse_mem)
		parse_mem = lcd_ini_file_parse(index);
	handle_ini_list_key_value(parse_mem);
}
