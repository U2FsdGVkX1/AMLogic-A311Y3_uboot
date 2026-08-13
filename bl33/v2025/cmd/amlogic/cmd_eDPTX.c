// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

// #include <common.h>
#include <dm.h>
#include <env.h>
#include <command.h>
#include <amlogic/media/vout/eDPTX/eDPTX_export.h>
#include <linux/types.h>

static int do_edptx_probe(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	edptx_driver_probe();
	return 0;
}

static int do_edptx0_enable(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	edptx_driver_enable(0, "NULL");
	return 0;
}

static int do_edptx0_disable(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	edptx_driver_disable(0);
	return 0;
}

static int do_edptx0_info(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	edptx_driver_info(0);
	return 0;
}

static int do_edptx0_reg(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	edptx_driver_reg_print(0);
	return 0;
}

static int do_edptx0_test(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned char num;

	if (argc == 1)
		return -1;

	num = (unsigned char)simple_strtoul(argv[1], NULL, 10);
	edptx_driver_test(0, num);
	return 0;
}

static int do_edptx0_list_vmode(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	edptx_driver_list_vmode(0);
	return 0;
}

static int do_edptx0_set_vmode(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned char num;

	if (argc == 1)
		return -1;

	num = (unsigned char)simple_strtoul(argv[1], NULL, 10);
	edptx_driver_set_vmode(0, num);
	return 0;
}

static int do_edptx0_reset(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned char num;

	if (argc == 1)
		return -1;

	num = (unsigned char)simple_strtoul(argv[1], NULL, 10);
	edptx_driver_reset(0, num);
	return 0;
}

static int do_edptx0_PSR1(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned char num;

	if (argc == 1)
		return -1;

	num = (unsigned char)simple_strtoul(argv[1], NULL, 10);
	edptx_driver_PSR1_en(0, num);
	return 0;
}

static int do_edptx0_PSR2(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned char num;

	if (argc == 1)
		return -1;

	num = (unsigned char)simple_strtoul(argv[1], NULL, 10);
	edptx_driver_PSR2_en(0, num);
	return 0;
}

static int do_edptx0_pattern(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	uint8_t pattern;
	uint32_t d0 = 0, d1 = 0, d2 = 0;

	if (argc == 1)
		return -1;

	pattern = (unsigned char)simple_strtoul(argv[1], NULL, 10);
	if (argc >= 5) {
		d0 = (unsigned int)simple_strtoul(argv[2], NULL, 16);
		d1 = (unsigned int)simple_strtoul(argv[3], NULL, 16);
		d2 = (unsigned int)simple_strtoul(argv[4], NULL, 16);
	}
	edptx_driver_set_pattern(0, pattern, d0, d1, d2);
	return 0;
}

static int do_edptx0_link(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned char lane;
	unsigned char rate;

	if (argc < 3)
		return -1;

	lane = (unsigned char)simple_strtoul(argv[1], NULL, 10);
	rate = (unsigned char)simple_strtoul(argv[2], NULL, 16);
	edptx_driver_set_link(0, lane, rate);
	return 0;
}

static int do_edptx1_enable(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	edptx_driver_enable(1, "NULL");
	return 0;
}

static int do_edptx1_disable(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	edptx_driver_disable(1);
	return 0;
}

static int do_edptx1_info(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	edptx_driver_info(1);
	return 0;
}

static int do_edptx1_reg(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	edptx_driver_reg_print(1);
	return 0;
}

static int do_edptx1_test(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned char num;

	if (argc == 1)
		return -1;

	num = (unsigned char)simple_strtoul(argv[1], NULL, 10);
	edptx_driver_test(1, num);
	return 0;
}

static int do_edptx1_list_vmode(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	edptx_driver_list_vmode(1);
	return 0;
}

static int do_edptx1_set_vmode(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned char num;

	if (argc == 1)
		return -1;

	num = (unsigned char)simple_strtoul(argv[1], NULL, 10);
	edptx_driver_set_vmode(1, num);
	return 0;
}

static int do_edptx1_reset(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned char num;

	if (argc == 1)
		return -1;

	num = (unsigned char)simple_strtoul(argv[1], NULL, 10);
	edptx_driver_reset(1, num);
	return 0;
}

static int do_edptx1_PSR1(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned char num;

	if (argc == 1)
		return -1;

	num = (unsigned char)simple_strtoul(argv[1], NULL, 10);
	edptx_driver_PSR1_en(1, num);
	return 0;
}

static int do_edptx1_PSR2(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned char num;

	if (argc == 1)
		return -1;

	num = (unsigned char)simple_strtoul(argv[1], NULL, 10);
	edptx_driver_PSR2_en(1, num);
	return 0;
}

static int do_edptx1_pattern(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	uint8_t pattern;
	uint32_t d0 = 0, d1 = 0, d2 = 0;

	if (argc == 1)
		return -1;

	pattern = (unsigned char)simple_strtoul(argv[1], NULL, 10);
	if (argc >= 5) {
		d0 = (unsigned int)simple_strtoul(argv[2], NULL, 16);
		d1 = (unsigned int)simple_strtoul(argv[3], NULL, 16);
		d2 = (unsigned int)simple_strtoul(argv[4], NULL, 16);
	}
	edptx_driver_set_pattern(1, pattern, d0, d1, d2);
	return 0;
}

static int do_edptx1_link(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned char lane;
	unsigned char rate;

	if (argc < 3)
		return -1;

	lane = (unsigned char)simple_strtoul(argv[1], NULL, 10);
	rate = (unsigned char)simple_strtoul(argv[2], NULL, 16);
	edptx_driver_set_link(1, lane, rate);
	return 0;
}

static cmd_tbl_t cmd_dptx0_sub[] = {
	U_BOOT_CMD_MKENT(probe,   2, 0, do_edptx_probe,       "", ""),
	U_BOOT_CMD_MKENT(enable,  2, 0, do_edptx0_enable,     "", ""),
	U_BOOT_CMD_MKENT(disable, 2, 0, do_edptx0_disable,    "", ""),
	U_BOOT_CMD_MKENT(test,    3, 0, do_edptx0_test,       "", ""),
	U_BOOT_CMD_MKENT(info,    2, 0, do_edptx0_info,       "", ""),
	U_BOOT_CMD_MKENT(reg,     2, 0, do_edptx0_reg,        "", ""),
	U_BOOT_CMD_MKENT(list,    2, 0, do_edptx0_list_vmode, "", ""),
	U_BOOT_CMD_MKENT(set,     2, 0, do_edptx0_set_vmode,  "", ""),
	U_BOOT_CMD_MKENT(rst,     2, 0, do_edptx0_reset,      "", ""),
	U_BOOT_CMD_MKENT(psr1,    2, 0, do_edptx0_PSR1,       "", ""),
	U_BOOT_CMD_MKENT(psr2,    2, 0, do_edptx0_PSR2,       "", ""),
	U_BOOT_CMD_MKENT(pattern, 2, 0, do_edptx0_pattern,    "", ""),
	U_BOOT_CMD_MKENT(link,    2, 0, do_edptx0_link,       "", ""),
};

static cmd_tbl_t cmd_dptx1_sub[] = {
	U_BOOT_CMD_MKENT(probe,   2, 0, do_edptx_probe,       "", ""),
	U_BOOT_CMD_MKENT(enable,  2, 0, do_edptx1_enable,     "", ""),
	U_BOOT_CMD_MKENT(disable, 2, 0, do_edptx1_disable,    "", ""),
	U_BOOT_CMD_MKENT(test,    3, 0, do_edptx1_test,       "", ""),
	U_BOOT_CMD_MKENT(info,    2, 0, do_edptx1_info,       "", ""),
	U_BOOT_CMD_MKENT(reg,     2, 0, do_edptx1_reg,        "", ""),
	U_BOOT_CMD_MKENT(list,    2, 0, do_edptx1_list_vmode, "", ""),
	U_BOOT_CMD_MKENT(set,     2, 0, do_edptx1_set_vmode,  "", ""),
	U_BOOT_CMD_MKENT(rst,     2, 0, do_edptx1_reset,      "", ""),
	U_BOOT_CMD_MKENT(psr1,    2, 0, do_edptx1_PSR1,       "", ""),
	U_BOOT_CMD_MKENT(psr2,    2, 0, do_edptx1_PSR2,       "", ""),
	U_BOOT_CMD_MKENT(pattern, 2, 0, do_edptx1_pattern,    "", ""),
	U_BOOT_CMD_MKENT(link,    2, 0, do_edptx1_link,       "", ""),
};

static int do_edptx0(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	cmd_tbl_t *c;

	// /* Strip off leading 'bmp' command argument */
	argc--;
	argv++;

	c = find_cmd_tbl(argv[0], &cmd_dptx0_sub[0], ARRAY_SIZE(cmd_dptx0_sub));

	if (c)
		return c->cmd(cmdtp, flag, argc, argv);

	cmd_usage(cmdtp);
	return 1;
}

static int do_edptx1(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	cmd_tbl_t *c;

	// /* Strip off leading 'bmp' command argument */
	argc--;
	argv++;

	c = find_cmd_tbl(argv[0], &cmd_dptx1_sub[0], ARRAY_SIZE(cmd_dptx1_sub));

	if (c)
		return c->cmd(cmdtp, flag, argc, argv);

	cmd_usage(cmdtp);
	return 1;
}

U_BOOT_CMD(edptx, 8, 0, do_edptx0, "eDPTX 0 module",
	" - probe          - probe   DPTX Driver\n"
	" - enable/disable - enable  DPTX module\n"
	" - test           - venc bist pattern module\n"
	" - info           - cat eDP info\n"
	" - list/set       - list/set eDPTX vmode\n"
	" - rst            - do link reset\n"
	" - psr1/psr2      - dump DPTX registers\n"
	" - pattern        - transmit link pattern\n"
	" - link           - set link param\n");

U_BOOT_CMD(edptx0, 8, 0, do_edptx0, "eDPTX 0", " eDP0 debug cmd\n");

U_BOOT_CMD(edptx1, 8, 0, do_edptx1, "eDPTX 1", " eDP0 debug cmd\n");

