// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <malloc.h>
#include <command.h>
#include <stdio.h>
#include <linux/ctype.h>
#include <linux/string.h>
#include <amlogic/partition_table.h>
#include <amlogic/aml_model.h>
#include "model_log.h"
#include "model_size.h"
#include <env.h>

#define LOG_TAG     "model"
#define LOG_NDEBUG  0

#ifndef CONFIG_YOCTO
#define DEFAULT_MODEL_PATH1      "/odm/etc/tvconfig/model"
#else
#define DEFAULT_MODEL_PATH1      "/vendor/etc/tvconfig/model"
#endif
#define DEFAULT_MODEL_PATH2      "/odm_ext/etc/tvconfig/model"
#define DEFAULT_MODEL_NAME "DEFAULT"
#define DEFAULT_SECTION "GENERAL_CONFIG_FROM_MODEL_SUM"

/*********************************  common api  *************************************/
static int get_model_multi_max_cnt(void)
{
	int cnt_max = 1;

#ifdef CONFIG_VOUT_MULTI
	cnt_max = CONFIG_VOUT_MULTI;
#endif
	return cnt_max;
}

int model_set_panel_misc(struct panel_misc_s *misc_attr)
{
	int tmp_val = 0;
	char *rev_ctrl = NULL;
	char *ret = NULL;
	char param[32], buf[64] = {0};
	unsigned int panel_reverse = 0, display_layer = 0, n;

	if (!misc_attr)
		return -1;

	if (misc_attr->disp_idx == 0xff)
		return -1;

	tmp_val = env_get_ulong("model_outputmode_bypass", 10, 0);
	if (tmp_val) {
		ALOGI("model_outputmode_bypass\n");
		goto model_set_panel_misc_next;
	}

	if (misc_attr->disp_idx == 0)
		snprintf(param, 31, "outputmode");
	else
		snprintf(param, 31, "outputmode%d", misc_attr->disp_idx + 1);
	ALOGD("%s: %s is (%s)\n", __func__, param, misc_attr->outputmode);
	snprintf(buf, 63, "setenv %s %s", param, misc_attr->outputmode);
	run_command(buf, 0);

model_set_panel_misc_next:
	tmp_val = env_get_ulong("model_connector_bypass", 10, 0);
	if (tmp_val) {
		ALOGI("model_connector_bypass\n");
		goto model_set_panel_misc_next2;
	}

	snprintf(param, 31, "connector%d_type", misc_attr->disp_idx);
	misc_attr->connector_type[sizeof(misc_attr->connector_type) - 1] = '\0';
	ret = strstr(misc_attr->connector_type, "_");
	if (ret)
		misc_attr->connector_type[ret - misc_attr->connector_type] = '-';
	ALOGD("%s: %s is (%s)\n", __func__, param, misc_attr->connector_type);
	snprintf(buf, 63, "setenv %s %s", param, misc_attr->connector_type);
	run_command(buf, 0);

model_set_panel_misc_next2:
	if (misc_attr->version >= 2)
		goto model_set_panel_misc_next3;
	rev_ctrl = env_get("reverse_ctrl");
	if (!rev_ctrl || strcmp(rev_ctrl, "0") == 0) {
		ALOGD("%s: panel_reverse is (%s)\n", __func__, misc_attr->panel_reverse);
		if (strcmp(misc_attr->panel_reverse, "null") == 0 ||
		    strcmp(misc_attr->panel_reverse, "0") == 0 ||
		    strcmp(misc_attr->panel_reverse, "false") == 0 ||
		    strcmp(misc_attr->panel_reverse, "no_rev") == 0) {
			panel_reverse = 0;
		} else if (strcmp(misc_attr->panel_reverse, "true") == 0 ||
			   strcmp(misc_attr->panel_reverse, "1") == 0 ||
			   strcmp(misc_attr->panel_reverse, "have_rev") == 0) {
			panel_reverse = 1;
		} else if (strcmp(misc_attr->panel_reverse, "x_rev") == 0 ||
			   strcmp(misc_attr->panel_reverse, "2") == 0) {
			panel_reverse = 2;
		} else if (strcmp(misc_attr->panel_reverse, "y_rev") == 0 ||
			   strcmp(misc_attr->panel_reverse, "3") == 0) {
			panel_reverse = 3;
		} else {
			panel_reverse = 0;
		}
		if (panel_reverse) {
			if (strcmp(misc_attr->display_layer, "osd0") == 0 ||
			    strcmp(misc_attr->display_layer, "0") == 0)
				display_layer = 0;
			else if (strcmp(misc_attr->display_layer, "osd1") == 0 ||
				 strcmp(misc_attr->display_layer, "1") == 0)
				display_layer = 1;
			else
				display_layer = 4;
		}
		switch (panel_reverse) {
		case 1:
			run_command("setenv panel_reverse 1", 0);
			switch (display_layer) {
			case 0:
				run_command("setenv osd_reverse osd0,true", 0);
				break;
			case 1:
				run_command("setenv osd_reverse osd1,true", 0);
				break;
			default:
				run_command("setenv osd_reverse all,true", 0);
				break;
			}
			run_command("setenv video_reverse 1", 0);
			break;
		case 2:
			run_command("setenv panel_reverse 2", 0);
			switch (display_layer) {
			case 0:
				run_command("setenv osd_reverse osd0,x_rev", 0);
				break;
			case 1:
				run_command("setenv osd_reverse osd1,x_rev", 0);
				break;
			default:
				run_command("setenv osd_reverse all,x_rev", 0);
				break;
			}
			run_command("setenv video_reverse 2", 0);
			break;
		case 3:
			run_command("setenv panel_reverse 3", 0);
			switch (display_layer) {
			case 0:
				run_command("setenv osd_reverse osd0,y_rev", 0);
				break;
			case 1:
				run_command("setenv osd_reverse osd1,y_rev", 0);
				break;
			default:
				run_command("setenv osd_reverse all,y_rev", 0);
				break;
			}
			run_command("setenv video_reverse 3", 0);
			break;
		default:
			run_command("setenv panel_reverse 0", 0);
			run_command("setenv osd_reverse n", 0);
			run_command("setenv video_reverse 0", 0);
			break;
		}
	}
	return 0;

model_set_panel_misc_next3:
	n = misc_attr->hmirror_val << 1 | misc_attr->vmirror_val;
	snprintf(buf, 64, "setenv osd_reverse %s%s",
		misc_attr->disp_layer_val == 0 ?
			"osd0," : misc_attr->disp_layer_val == 1 ? "osd1," : "all,",
		n == 0 ? "n" : n == 1 ? "y_rev" : n == 2 ? "x_rev" : "true");
	ALOGD("%s: %s\n", __func__, buf);
	run_command(buf, 0);

	snprintf(buf, 64, "setenv video_reverse %c",
		n == 0 ? '0' : n == 1 ? '3' : n == 2 ? '2' : '1');
	ALOGD("%s: %s\n", __func__, buf);
	run_command(buf, 0);

	snprintf(buf, 64, "setenv panel_reverse %c",
		n == 0 ? '0' : n == 1 ? '3' : n == 2 ? '2' : '1');
	ALOGD("%s: %s\n", __func__, buf);
	run_command(buf, 0);

	return 0;
}

/*********************************  cmd api *************************************/
static int parse_model_sum(int index, const char *file_name, char *model_name)
{
	void *local_ini_mem, *psec;
	const char *str = NULL;
#ifdef CONFIG_AML_LCD
	char model_panel[24], model_det[32], *p;
	unsigned char file_type = PANEL_FILE_INVILD;
#endif

	local_ini_mem = handle_ini_file_parse(file_name, 0);
	if (!local_ini_mem) {
		ALOGE("%s: error: %s\n", __func__, file_name);
		return -1;
	}

	psec = handle_ini_get_section(local_ini_mem, model_name);
	if (!psec)
		goto parse_model_sum_err;
	ALOGD("%s: find model_name: %s, in file: %s\n", __func__, model_name, file_name);

#ifdef CONFIG_AML_LCD
	rm_panel_file_path(index);
	str = handle_ini_get_str(local_ini_mem, psec, "PANELINI_PATH", "null");
	if (strcmp(str, "null") == 0) {
		ALOGE("%s: %s: invalid PANELINI_PATH!\n", __func__, model_name);
		goto parse_model_sum_err;
	}
	p = strrchr(str, '.');
	if (p && (!strncmp(p + 1, "ini", 3) || !strncmp(p + 1, "INI", 3)))
		file_type = PANEL_FILE_INI;
	else //regard as json file, will be parse later
		file_type = PANEL_FILE_JSON;

	if (index == 0)
		snprintf(model_panel, 15, "model_panel");
	else
		snprintf(model_panel, 15, "model%d_panel", index);
	env_set(model_panel, str);
	set_panel_file_path(index, str, file_type);

	rm_bl_file_path(index);
	str = handle_ini_get_str(local_ini_mem, psec, "BACKLIGHT_PATH", NULL);
	if (str) {
		p = strrchr(str, '.');
		if (p && (!strncmp(p + 1, "ini", 3) || !strncmp(p + 1, "INI", 3)))
			file_type = PANEL_FILE_INI;
		else //regard as json file, will be parse later
			file_type = PANEL_FILE_JSON;

		if (index == 0)
			snprintf(model_panel, 19, "model_backlight");
		else
			snprintf(model_panel, 19, "model%d_backlight", index);
		env_set(model_panel, str);
		set_bl_file_path(index, str, file_type);
	}

	rm_panel_alt_file_path(index);
	snprintf(model_det, 31, "model%d_name_alternate", index);
	p = env_get(model_det);
	if (p && strcmp(p, "alternate") == 0) {
		str = handle_ini_get_str(local_ini_mem, psec, "PANEL_ALT_PATH", NULL);
		if (str) {
			p = strrchr(str, '.');
			if (p && (!strncmp(p + 1, "ini", 3) || !strncmp(p + 1, "INI", 3)))
				file_type = PANEL_FILE_INI;
			else //regard as json file, will be parse later
				file_type = PANEL_FILE_JSON;

			snprintf(model_panel, 23, "model%d_panel_alt", index);
			env_set(model_panel, str);
			set_panel_alt_file_path(index, str, file_type);
		}
	}
#endif

	str = handle_ini_get_str(local_ini_mem, psec, "EDID_14_FILE_PATH", "null");
	if (strcmp(str, "null") == 0) {
		ALOGD("%s: %s: invalid EDID_14_FILE_PATH!\n", __func__, model_name);
		goto parse_model_sum_end;
	}
	env_set("model_edid", str);

	/*
	 *str = handle_ini_get_str(local_ini_mem, psec, "PQINI_PATH", "null");
	 *env_set("model_pq", str);
	 *
	 *str = handle_ini_get_str(local_ini_mem, psec, "AMLOGIC_AUDIO_EFFECT_INI_PATH", "null");
	 *env_set("model_audio", str);
	 */

parse_model_sum_end:
	handle_ini_parser_uninit(local_ini_mem);
	return 0;

parse_model_sum_err:
	handle_ini_parser_uninit(local_ini_mem);
	return -1;
}

static const char *get_model_path(int index)
{
	char *model_path, str[16];

	if (index == 0)
		snprintf(str, 15, "model_path");
	else
		snprintf(str, 15, "model%d_path", index);

	model_path = env_get(str);
	if (!model_path) {
		if (dynamic_partition)
			return DEFAULT_MODEL_PATH2;
		else
			return DEFAULT_MODEL_PATH1;
	}

	printf("%s: %s: %s\n", __func__, str, model_path);
	return model_path;
}

#ifdef CONFIG_AML_LCD
static char *model_list_panel_path(int index)
{
	char *path_str, str[16];

	if (index == 0)
		snprintf(str, 15, "model_panel");
	else
		snprintf(str, 15, "model%d_panel", index);

	path_str = env_get(str);
	if (path_str)
		printf("current %s: %s\n", str, path_str);

	return path_str;
}
#endif

static int read_model_name_ukey(const char *item_name, unsigned char *data_buf, int data_size)
{
	int rd_size = 0;
	unsigned char *read_buf;

	if (!item_name || !data_buf)
		return -1;

	read_buf = model_detect_ukey_data(item_name, &rd_size);
	if (!read_buf || rd_size <= 0)
		return -1;
	rd_size += 1; //add \0 for string
	if (data_size < rd_size) {
		ALOGE("%s: size error: %s\n", __func__, item_name);
		memset(read_buf, 0, rd_size);
		free(read_buf);
		return -1;
	}
	strlcpy(data_buf, read_buf, rd_size);
	memset(read_buf, 0, rd_size);
	free(read_buf);

	return rd_size;
}

static int save_model_name_ukey(const char *item_name, const char *data_buf, int wr_size)
{
	if (!item_name || !data_buf)
		return -1;

	return model_write_ukey_data(item_name, data_buf, wr_size);
}

static int get_panel_model_name_param(int index, char *model_buf, int buf_size,
				      char *key_buf, int key_size)
{
	char *model, env_str[64];
	int dbg_env_flag = 1;

	/****** factory mode *******/
	snprintf(key_buf, key_size, "model%d_name_factory", index);
	model = env_get(key_buf);
	if (model) {
		if (strcmp(model, "none")) {
			strlcpy(model_buf, model, buf_size);
			return 0;
		}
	}
	/***************************/

	if (index == 0)
		snprintf(key_buf, key_size, "model_name");
	else
		snprintf(key_buf, key_size, "model%d_name", index);
	if (read_model_name_ukey(key_buf, model_buf, buf_size) > 0) {
		//find unifykey model_name
		dbg_env_flag = env_get_ulong("model_env_flag", 16, 0x1);
		if (dbg_env_flag) {
			env_set(key_buf, model_buf);
			snprintf(env_str, 64, "update_env_part -p -f %s", key_buf);
			run_command(env_str, 0);
			ALOGD("%s: update env[%s]=%s\n", __func__, key_buf, model_buf);
		}
		return 0;
	}

	model = env_get(key_buf);
	if (model) {
		strlcpy(model_buf, model, buf_size);
		return 0;
	}

	ALOGD("%s: no %s\n", __func__, key_buf);
	return -1;
}

static int handle_model_list(void)
{
	void *local_ini_mem;
	char model_path[128], model_ini_path[128];
	char str[32], model_val[64];
	int i, cnt_max, ret;

	cnt_max = get_model_multi_max_cnt();
	for (i = 0; i < cnt_max; i++) {
		ret = get_panel_model_name_param(i, model_val, 64, str, 32);
		if (ret)
			continue;
		printf("current %s: %s\n", str, model_val);
#ifdef CONFIG_AML_LCD
		model_list_panel_path(i);
#endif

		snprintf(model_path, 127, "%s", get_model_path(i));
		if (i)
			snprintf(model_ini_path, 127, "%s/model%d_sum.ini", model_path, i);
		else
			snprintf(model_ini_path, 127, "%s/model_sum.ini", model_path);

		local_ini_mem = handle_ini_file_parse(model_ini_path, 0);
		if (!local_ini_mem) {
			ALOGE("%s: error: %s\n", __func__, model_path);
			return -1;
		}

		printf("%s list:\n", str);
		handle_ini_list_section(local_ini_mem);
		handle_ini_parser_uninit(local_ini_mem);
		printf("\n");

		model_list_match_files(model_ini_path, "Project_index_", ".ini");
		printf("\n");
	}

	return 0;
}

static int handle_model_sum(void)
{
	char str[32], model_val[64], config_sec[64];
	char model_path[128], model_ini_path[128];
	unsigned int print_flag;
	int i, cnt_max, ret;

	print_flag = env_get_ulong("model_debug_print", 10, 0xffff);
	if (print_flag != 0xffff) {
		model_set_log_level(print_flag);
		printf("model_debug_flag: %d\n", print_flag);
	}

	cnt_max = get_model_multi_max_cnt();
	for (i = 0; i < cnt_max; i++) {
		ret = get_panel_model_name_param(i, model_val, 64, str, 32);
		if (ret)
			continue;
		snprintf(model_path, 127, "%s", get_model_path(i));

		if (i)
			snprintf(model_ini_path, 127, "%s/model%d_sum.ini", model_path, i);
		else
			snprintf(model_ini_path, 127, "%s/model_sum.ini", model_path);
		snprintf(config_sec, 63, "%s", model_val);
		ret = parse_model_sum(i, model_ini_path, config_sec);
		if (ret < 0) {
			if (i) {
				snprintf(model_ini_path, 127, "%s%d/Project_index_%s.ini",
					 model_path, i, model_val);
			} else {
				snprintf(model_ini_path, 127, "%s/Project_index_%s.ini",
					 model_path, model_val);
			}
			snprintf(config_sec, 63, "%s", DEFAULT_SECTION);
			ret = parse_model_sum(i, model_ini_path, config_sec);
		}
		ALOGI("%s: %s: %s %s\n",
		      __func__, model_ini_path, config_sec, ret ? "fail" : "ok");
	}

	return 0;
}

int handle_model_get(const char *model, char *buf, int buf_size)
{
	const char *name;
	char *str = NULL;
	int ret;

	if (!model || !buf)
		return -1;

	if (!strcmp(model, "model_name") || !strcmp(model, "0"))
		name = "model_name";
	else if (!strcmp(model, "model1_name") || !strcmp(model, "1"))
		name = "model1_name";
	else if (!strcmp(model, "model2_name") || !strcmp(model, "2"))
		name = "model2_name";
	else
		return -1;

	ret = read_model_name_ukey(name, buf, buf_size);
	if (ret <= 0) {
		str = env_get(name);
		if (!str) {
			ALOGD("%s, no %s\n", __func__, model);
			return -1;
		}
		strcpy(buf, str);
	}

	return 0;
}

int handle_model_set(const char *model, const char *val)
{
	const char *name;
	int ukey_ok = 0;
	char env_str[64];

	if (!model || !val)
		return -1;

	if (!strcmp(model, "model_name") || !strcmp(model, "0"))
		name = "model_name";
	else if (!strcmp(model, "model1_name") || !strcmp(model, "1"))
		name = "model1_name";
	else if (!strcmp(model, "model2_name") || !strcmp(model, "2"))
		name = "model2_name";
	else
		return -1;

	if (save_model_name_ukey(name, val, strlen(val) + 1) == 0)
		ukey_ok = 1;

	env_set(name, val);
	snprintf(env_str, 63, "update_env_part -p -f %s", name);
	run_command(env_str, 0);

	if (!ukey_ok)
		ALOGE("%s: %s=%s fail\n", __func__, name, val);
	else
		ALOGD("%s: %s=%s ok\n", __func__, name, val);
	return 0;
}

/*********************************  cmd  *************************************/
static int do_ini_model_list(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	int ret;

	if (argc == 1) {
		ret = handle_model_list();
		return ret;
	}
	if (strcmp(argv[1], "lcd") == 0 || strcmp(argv[1], "panel") == 0) {
		aml_lcd_panel_dump(0, NULL);
		return 0;
	} else if (strcmp(argv[1], "lcd1") == 0 || strcmp(argv[1], "panel1") == 0) {
		aml_lcd_panel_dump(1, NULL);
		return 0;
	} else if (strcmp(argv[1], "lcd2") == 0 || strcmp(argv[1], "panel2") == 0) {
		aml_lcd_panel_dump(2, NULL);
		return 0;
	}

	return CMD_RET_USAGE;
}

U_BOOT_CMD(model_list, 4, 0, do_ini_model_list,
	"list ini model name",
	" "
);

static int do_ini_model(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	int ret;

	if (argc > 1)
		return CMD_RET_USAGE;

	ret = handle_model_sum();
	return ret;
}

U_BOOT_CMD(ini_model, 4, 0, do_ini_model,
	"parse ini file by env model_name",
	" "
);

int parse_model_powermode(int index, const char *file_name, char *model_name)
{
	void *local_ini_mem, *psec;
	const char *powermode_value = NULL;
	int ret = -1;

	local_ini_mem = handle_ini_file_parse(file_name, 0);
	if (!local_ini_mem) {
		ALOGE("%s: error: %s\n", __func__, file_name);
		return ret;
	}

	psec = handle_ini_get_section(local_ini_mem, model_name);
	if (!psec) {
		ALOGE("%s: section %s: error: %s\n", __func__, model_name, file_name);
		return ret;
	}

	powermode_value = handle_ini_get_str(local_ini_mem, psec, "POWER_ON_MODE", "null");
	if (strcmp(powermode_value, "null") != 0) {
		ALOGI("powermode_value is %s\n", powermode_value);
		if (!strcmp(powermode_value, "on") || !strcmp(powermode_value, "standby") ||
		    !strcmp(powermode_value, "last")) {
			env_set("powermode", powermode_value);
			env_set("powermode_ini", "1");
			env_save();
		}
		ret = 0;
	} else {
		ALOGD("not find POWER_ON_MODE in ini files: %s,%s !!!\n", file_name, model_name);
//		env_set("powermode_ini", "2");
//		env_save();
	}

	handle_ini_parser_uninit(local_ini_mem);

	return ret;
}

int handle_model_powermode(void)
{
	char str[32], model_val[64], config_sec[64];
	char model_path[128], model_ini_path[128];
	int i, cnt_max;
	int ret = 1;

	cnt_max = get_model_multi_max_cnt();
	for (i = 0; i < cnt_max; i++) {
		ret = get_panel_model_name_param(i, model_val, 64, str, 32);
		if (ret)
			continue;
		snprintf(model_path, 127, "%s", get_model_path(i));

		if (i)
			snprintf(model_ini_path, 127, "%s/model%d_sum.ini", model_path, i);
		else
			snprintf(model_ini_path, 127, "%s/model_sum.ini", model_path);
		snprintf(config_sec, 63, "%s", DEFAULT_MODEL_NAME);

		ret = parse_model_powermode(i, model_ini_path, config_sec);
		if (ret != 0) {
			if (i) {
				snprintf(model_ini_path, 127, "%s%d/Project_index_%s.ini",
					 model_path, i, model_val);
			} else {
				snprintf(model_ini_path, 127, "%s/Project_index_%s.ini",
					 model_path, model_val);
			}
			snprintf(config_sec, 63, "%s", DEFAULT_SECTION);
			ret = parse_model_powermode(i, model_ini_path, config_sec);
		}
		ALOGI("%s: %s: %s %s\n",
		      __func__, model_ini_path, config_sec, ret ? "fail" : "ok");
		if (ret != 0)
			continue;
	}

	return ret;
}

static int do_ini_powermode(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	int ret = -1;

	if (argc > 1)
		return CMD_RET_USAGE;

	char str[] = "powermode_ini";
	char *val = NULL;

	val = env_get(str);
	if (!val) {
		ALOGI("powermode_ini is not set\n");
	} else if (strcmp(val, "0") == 0) {
		ALOGI("powermode_ini is %s,not read from ini file\n", val);
		return 0;
	}

	ret = handle_model_powermode();
	return ret;
}

U_BOOT_CMD(ini_powermode, 4, 0, do_ini_powermode,
		"parse ini file to get powermode",
		" "
);
