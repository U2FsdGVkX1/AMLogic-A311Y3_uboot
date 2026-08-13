// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <malloc.h>
#include <string.h>
#include <amlogic/aml_model.h>
#include <amlogic/media/vout/lcd/aml_lcd.h>
#include "json_parse.h"
#include "../lcd_common.h"

int set_panel_jsp(int index, struct json_parse_s *jsp)
{
	return set_panel_file_parse_mem(index, (void *)jsp, sizeof(struct json_parse_s),
					PANEL_FILE_JSON);
}

struct json_parse_s *get_panel_jsp(int index)
{
	if (get_lcd_panel_file_type(index) != PANEL_FILE_JSON)
		return NULL;

	return (struct json_parse_s *)get_panel_file_parse_mem(index);
}

static int panel_json_parse_misc(struct json_parse_s *jsp)
{
	struct panel_misc_s misc_attr;
	struct json_s *parent;
	const char *str;
	const char *outputmode[3] = {"outputmode", "outputmode2", "outputmode3"};
	const char *connector[3] = {"connector0_type", "connector1_type", "connector2_type"};
	int i;

	parent = json_path_to_node(jsp, jsp->root, "/misc");
	if (!parent) {
		LCDERR("find /misc\n");
		return -1;
	}

	memset(&misc_attr, 0, sizeof(misc_attr));
	misc_attr.version = 2;
	misc_attr.disp_idx = 0xff;
	strlcpy(misc_attr.outputmode, "null", sizeof(misc_attr.outputmode));
	strlcpy(misc_attr.connector_type, "null", sizeof(misc_attr.connector_type));

	for (i = 0; i < 3; i++) {
		str = json_get_obj_str(jsp, parent, outputmode[i], NULL);
		if (str) {
			misc_attr.disp_idx = i;
			strlcpy(misc_attr.outputmode, str, sizeof(misc_attr.outputmode));
			break;
		}
	}

	str = json_get_obj_str(jsp, parent, "connector_type", NULL);
	if (str) {
		strlcpy(misc_attr.connector_type, str, sizeof(misc_attr.connector_type));
		goto handle_panel_misc_next;
	}

	for (i = 0; i < 3; i++) {
		str = json_get_obj_str(jsp, parent, connector[i], NULL);
		if (str) {
			misc_attr.disp_idx = i;
			strlcpy(misc_attr.connector_type, str, sizeof(misc_attr.connector_type));
		}
	}

handle_panel_misc_next:
	misc_attr.hmirror_val = json_get_obj_u32(jsp, parent, "hmirror", 0);
	misc_attr.vmirror_val = json_get_obj_u32(jsp, parent, "vmirror", 0);
	misc_attr.disp_layer_val = json_get_obj_u32(jsp, parent, "layer", 4);

	model_set_panel_misc(&misc_attr);

	return 0;
}

struct json_parse_s *handle_json_file_parse(const char *file_name)
{
	struct json_parse_s *jsp = NULL;
	unsigned char *tmp_buf;
	int file_size = 0, ret;

	tmp_buf = model_read_file_to_buffer(file_name, &file_size);
	if (!tmp_buf)
		return NULL;

	jsp = (struct json_parse_s *)malloc(sizeof(struct json_parse_s));
	if (!jsp)
		return NULL;
	if (json_init(jsp, JSON_STR_MAX, JSON_NODE_MAX) < 0) {
		ret = -1;
		goto handle_json_file_parse_err;
	}
	if (!json_parse(jsp, (char *)tmp_buf, file_size)) {
		ret = -2;
		goto handle_json_file_parse_err;
	}

	memset((void *)tmp_buf, 0, file_size);
	free(tmp_buf);

	return jsp;

handle_json_file_parse_err:
	memset((void *)tmp_buf, 0, file_size);
	free(tmp_buf);
	LCDERR("%s: jsp parse failed: %d: %s\n", __func__, ret, file_name);
	jsp->status = JSON_STATUS_ERROR;
	json_deinit(jsp);
	memset(jsp, 0, sizeof(struct json_parse_s));
	free(jsp);
	return NULL;
}

struct json_parse_s *panel_json_parse(int index)
{
	struct json_parse_s *local_jsp, *jsp, *jsp_bl, *jsp_alt;
	struct json_s *dst_node, *src_node, *node_alt;
	const char *file_name, *bl_name, *alt_name, *alt_node_name;
	char alt_obj_key[32];
	int i, ret;

	file_name = get_panel_file_path(index);
	if (!file_name)
		return NULL;

	rm_panel_file_parse_mem(index);

	jsp = handle_json_file_parse(file_name);
	if (!jsp)
		return NULL;
	local_jsp = jsp; //default pre-set_model

	if (get_bl_file_type(index) != PANEL_FILE_JSON)
		goto panel_json_panel_done;

	bl_name = get_bl_file_path(index);
	if (!bl_name)
		goto panel_json_panel_done;
	jsp_bl = handle_json_file_parse(bl_name);
	if (!jsp_bl)
		goto panel_json_panel_done;
	ret = -1;
	src_node = json_get_object_child(jsp_bl, jsp_bl->root, "backlight");
	if (src_node) {
		dst_node = json_create_object_child(jsp, jsp->root, "backlight", src_node->type);
		ret = json_copy(jsp, dst_node, jsp_bl, src_node);
	}
	LCDPR("[%d]: copy backlight to panel: %s\n", index, ret ? "fail" : "ok");
	json_deinit(jsp_bl);
	memset(jsp_bl, 0, sizeof(struct json_parse_s));
	free(jsp_bl);

panel_json_panel_done:
	/* panel_alt detect */
	if (get_panel_alt_file_type(index) != PANEL_FILE_JSON)
		goto panel_json_panel_next;
	alt_name = get_panel_alt_file_path(index);
	if (!alt_name)
		goto panel_json_panel_next;
	jsp_alt = handle_json_file_parse(alt_name);
	if (!jsp_alt)
		goto panel_json_panel_next;
	node_alt = json_path_to_node(jsp_alt, jsp_alt->root, "/alternate");
	if (!node_alt) {
		LCDERR("[%d]: not find /alternate\n", index);
		json_deinit(jsp_alt); //release pre-set model
		memset(jsp_alt, 0, sizeof(struct json_parse_s));
		free(jsp_alt);
		goto panel_json_panel_next;
	}

	i = 0;
	snprintf(alt_obj_key, 32, "alt_obj_%d", i);
	alt_node_name = json_get_obj_str(jsp_alt, node_alt, alt_obj_key, NULL);
	while (alt_node_name) {
		src_node = json_get_object_child(jsp, jsp->root, alt_node_name);
		dst_node = json_create_object_child(jsp_alt, jsp_alt->root,
						    alt_node_name, src_node->type);
		ret = json_copy(jsp_alt, dst_node, jsp, src_node);
		LCDPR("[%d]: copy %s to panel: %s\n", index, alt_node_name, ret ? "fail" : "ok");

		i++;
		snprintf(alt_obj_key, 32, "alt_obj_%d", i);
		alt_node_name = json_get_obj_str(jsp_alt, node_alt, alt_obj_key, NULL);
	};
	local_jsp = jsp_alt; //use panel-alt_model
	json_deinit(jsp); //release pre-set model
	memset(jsp, 0, sizeof(struct json_parse_s));
	free(jsp);

panel_json_panel_next:
	local_jsp->status = JSON_STATUS_OK;
	set_panel_jsp(index, local_jsp);

	/*misc*/
	panel_json_parse_misc(local_jsp);

	return local_jsp;
}

int panel_json_mem_save(void *parse_mem, int index)
{
#define JSON_PANEL_HANDLE_HEAD_SIZE (32)
	struct json_parse_s *jsp;
	unsigned char *p;//, *save;
	char name[16];
	struct json_panel_handle_head_s {
		unsigned int size;
		unsigned int json_cnt;
		unsigned int js_len;
		unsigned int json_start;
		unsigned int js_start;
		unsigned char rsvd[JSON_PANEL_HANDLE_HEAD_SIZE - 20];
	} head; //for make memory handle to kernel

	if (!parse_mem)
		return -1;
	jsp = (struct json_parse_s *)parse_mem;
	/* save jsp to reserved memory for kernel use */
	sprintf(name, "panel%d_jsp", index);

	//|size(4)|json_cnt(4)|js_len(4)|json_start(4)|(js_start)
	head.json_cnt = jsp->json_cnt;
	head.js_len = jsp->js_len;
	head.json_start = JSON_PANEL_HANDLE_HEAD_SIZE;
	head.json_start = ALIGN(head.json_start, 16);
	head.js_start = head.json_start + head.json_cnt * sizeof(*jsp->root);
	head.js_start = ALIGN(head.js_start, 16);
	head.size = head.js_start + head.js_len;
	head.size = ALIGN(head.size, 16);
	p = (unsigned char *)malloc(head.size);
	if (p) {
		memcpy(p, &head, JSON_PANEL_HANDLE_HEAD_SIZE);
		memcpy(p + head.json_start, jsp->root, jsp->json_cnt * sizeof(*jsp->root));
		memcpy(p + head.js_start, jsp->js, jsp->js_len);
		panel_param_mem_put(p, name, head.size);
		free(p);
		p = NULL;
	}

	return 0;
}

void panel_json_mem_free(void *parse_mem)
{
	json_deinit((struct json_parse_s *)parse_mem);
	free(parse_mem);
}
