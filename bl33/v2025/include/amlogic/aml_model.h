/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef _AML_MODEL_H
#define _AML_MODEL_H

struct panel_misc_s {
	char version;
	char disp_idx;
	char outputmode[64];
	char connector_type[64];
	char panel_reverse[16];
	char display_layer[16];
	unsigned int hmirror_val;
	unsigned int vmirror_val;
	unsigned int disp_layer_val;
};

#ifndef u64
#define u64 unsigned long long
#endif

#ifndef s64
#define s64 signed long long
#endif

#ifndef u32
#define u32 unsigned int
#endif

#ifndef s32
#define s32 signed int
#endif

#ifndef f32
#define f32 float
#endif

#ifndef f64
#define f64 double
#endif

#define PANEL_FILE_INVILD  0
#define PANEL_FILE_INI     1
#define PANEL_FILE_JSON    2

static inline unsigned int cal_CRC32(unsigned int crc, const unsigned char *ptr, int buf_len)
{
	static const unsigned int s_crc32[16] = {
		0, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
		0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
		0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
		0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c
	};

	unsigned int crcu32 = crc;
	unsigned char b;

	if (buf_len <= 0)
		return 0;

	if (!ptr)
		return 0;

	crcu32 = ~crcu32;
	while (buf_len--) {
		b = *ptr++;
		crcu32 = (crcu32 >> 4) ^ s_crc32[(crcu32 & 0xF) ^ (b & 0xF)];
		crcu32 = (crcu32 >> 4) ^ s_crc32[(crcu32 & 0xF) ^ (b >> 4)];
	}

	return ~crcu32;
}

#if defined(CONFIG_CMD_AML_MODEL)
int model_set_panel_misc(struct panel_misc_s *misc_attr);

/*unifykey operation*/
int model_check_ukey_size(const char *key_name);
unsigned char *model_detect_ukey_data(const char *key_name, int *data_size);
unsigned char *model_read_ukey_data(const char *key_name, int *data_size);
int model_write_ukey_data(const char *key_name, const char *data_buf, int buf_size);

/*file operation*/
int model_file_is_exist(const char *file_path);
int model_get_file_size(const char *file_path);
void model_list_match_files(const char *file_path, char *match_prefix, char *match_ext);
int model_read_file(const char *file_path, unsigned char *data_buf, int buf_size);
unsigned char *model_read_file_to_buffer(const char *file_path, int *buf_size);
unsigned char *model_read_bin_to_buffer(const char *file_path, int *buf_size);

int handle_model_get(const char *model, char *buf, int buf_size);
int handle_model_set(const char *model, const char *val);
int handle_model_powermode(void);

/*ini operation*/
int handle_ini_mem_check(void *inip);
void handle_ini_parser_uninit(void *inip);
void *handle_ini_file_parse(const char *filename, int mem_size);
int handle_ini_parser_mem_get_size(void *inip);
void *handle_ini_parser_dupmem(void *inip, int *buf_size);
void *handle_ini_get_section(void *inip, const char *section);
const char *handle_ini_get_str(void *inip, void *psec, const char *key, const char *def_val);
unsigned int handle_ini_get_val(void *inip, void *psec, const char *key, unsigned int def_val);
int handle_ini_key_exist(void *inip, void *psec, const char *key);
int handle_ini_set_exist_single_key(void *inip, void *psec, const char *key, const char *str);
int handle_ini_set_exist_keys(void *inip, void *psec,
			      const char *new_line, const char *new_value, int line_cnt);
int handle_ini_write_new_section(void *cur_inip, void *cur_psec, void *new_inip, void *new_psec);
void handle_ini_list_key_value(void *inip);
void handle_ini_list_section(void *inip);

#else
static inline int model_set_panel_misc(struct panel_misc_s *misc_attr)
{
	return -1;
}

static inline int model_file_is_exist(const char *file_path)
{
	return 0;
}

static inline int model_get_file_size(const char *file_path)
{
	return -1;
}

static inline int model_read_file(const char *file_path, unsigned char *data_buf, int buf_size)
{
	return -1;
}

static inline unsigned char *model_read_file_to_buffer(const char *file_path, int *buf_size)
{
	return NULL;
}

static inline unsigned char *model_read_bin_to_buffer(const char *file_path, int *buf_size)
{
	return NULL;
}

static inline int handle_model_get(const char *model, char *buf, int buf_size)
{
	return -1;
}

static inline int handle_model_set(const char *model, const char *val)
{
	return -1;
}

static inline int handle_model_powermode(void)
{
	return -1;
}

static inline int handle_ini_mem_check(void *inip)
{
	return -1;
}

static inline void handle_ini_parser_uninit(void *inip)
{
}

static inline void *handle_ini_file_parse(const char *filename, int mem_size)
{
	return NULL;
}

static inline int handle_ini_parser_mem_get_size(void *inip)
{
	return -1;
}

static inline void *handle_ini_parser_dupmem(void *inip, int *buf_size)
{
	return NULL;
}

static inline void *handle_ini_get_section(void *inip, const char *section)
{
	return NULL;
}

static inline const char *handle_ini_get_str(void *inip, void *psec, const char *key,
					     const char *def_val)
{
	return def_val;
}

static inline unsigned int handle_ini_get_val(void *inip, void *psec, const char *key,
					      unsigned int def_val)
{
	return def_val;
}

static inline int handle_ini_key_exist(void *inip, void *psec, const char *key)
{
	return 0;
}

static inline int handle_ini_set_exist_single_key(void *inip, void *psec, const char *key,
						  const char *str)
{
	return -1;
}

static inline int handle_ini_set_exist_keys(void *inip, void *psec,
			      const char *new_line, const char *new_value, int line_cnt)
{
	return -1;
}

int handle_ini_write_new_section(void *cur_inip, void *cur_psec, void *new_inip, void *new_psec)
{
	return -1;
}

void handle_ini_list_key_value(void *inip)
{
}

void handle_ini_list_section(void *inip)
{
}

#endif

#ifdef CONFIG_AML_LCD
int set_panel_file_path(int index, const char *str, unsigned char type);
const char *get_panel_file_path(int index);
void rm_panel_file_path(int index);
unsigned char get_lcd_panel_file_type(int index);

unsigned char get_bl_file_type(int index);
int set_bl_file_path(int index, const char *str, unsigned char type);
const char *get_bl_file_path(int index);
void rm_bl_file_path(int index);

unsigned char get_panel_alt_file_type(int index);
int set_panel_alt_file_path(int index, const char *str, unsigned char type);
const char *get_panel_alt_file_path(int index);
void rm_panel_alt_file_path(int index);

void aml_lcd_panel_dump(int index, const char *path);
#endif

#endif /*_AML_MODEL_H*/

