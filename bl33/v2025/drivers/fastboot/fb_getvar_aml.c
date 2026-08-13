// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (C) 2016 The Android Open Source Project
 */

#include <env.h>
#include <fastboot.h>
#include <fastboot-internal.h>
#include <fb_mmc.h>
#include <fb_nand.h>
#include <fs.h>
#include <part.h>
#include <version.h>
#include <timestamp.h>
#include <vsprintf.h>
#include <linux/printk.h>
#include <amlogic/partition_table.h>
#include <amlogic/storage.h>
#include <amlogic/emmc_partitions.h>
#include <amlogic/aml_efuse.h>

u32 kMaxDownloadSizeDefault = 0x7000000;
u32 kMaxFetchSizeDefault = 0x8000000;

static void getvar_version(char *var_parameter, char *response);
static void getvar_bootloader_version(char *var_parameter, char *response);
static void getvar_downloadsize(char *var_parameter, char *response);
static void getvar_serialno(char *var_parameter, char *response);
static void getvar_version_baseband(char *var_parameter, char *response);
static void getvar_hw_revision(char *var_parameter, char *response);
static void getvar_off_mode_charge(char *var_parameter, char *response);
static void getvar_variant(char *var_parameter, char *response);
static void getvar_battery_soc_ok(char *var_parameter, char *response);
static void getvar_battery_voltage(char *var_parameter, char *response);
static void getvar_block_size(char *var_parameter, char *response);
static void getvar_secure(char *var_parameter, char *response);
static void getvar_unlocked(char *var_parameter, char *response);
static void getvar_is_userspace(char *var_parameter, char *response);
static void getvar_is_logical(char *var_parameter, char *response);
static void getvar_slot_count(char *var_parameter, char *response);
static void getvar_super_partition_name(char *var_parameter, char *response);
static void getvar_product(char *var_parameter, char *response);
static void getvar_platform(char *var_parameter, char *response);
static void getvar_current_slot(char *var_parameter, char *response);
static void getvar_slot_suffixes(char *var_parameter, char *response);
static void getvar_has_slot(char *var_parameter, char *response);
#if CONFIG_IS_ENABLED(FASTBOOT_FLASH)
static void getvar_partition_type(char *part_name, char *response);
static void getvar_partition_size(char *part_name, char *response);
static int is_f2fs_by_name(char const *name);
#endif
static void getvar_maxdownloadsize(char *var_parameter, char *response);
static void getvar_maxfetchsize(char *var_parameter, char *response);


#ifdef CONFIG_BOOTLOADER_CONTROL_BLOCK
extern int is_partition_logical(char* partition_name);
#endif


static const struct {
	const char *variable;
	bool list;
	void (*dispatch)(char *var_parameter, char *response);
} getvar_dispatch[] = {
	{
		.variable = "version",
		.dispatch = getvar_version,
		.list = true,
	}, {
		.variable = "bootloader-version",
		.dispatch = getvar_bootloader_version,
		.list = true,
	}, {
		.variable = "version-bootloader",
		.dispatch = getvar_bootloader_version,
		.list = true,
	}, {
		.variable = "off-mode-charge",
		.dispatch = getvar_off_mode_charge,
		.list = true,
	}, {
		.variable = "hw-revision",
		.dispatch = getvar_hw_revision,
		.list = true,
	}, {
		.variable = "variant",
		.dispatch = getvar_variant,
		.list = true,
	}, {
		.variable = "battery-soc-ok",
		.dispatch = getvar_battery_soc_ok,
		.list = true,
	}, {
		.variable = "battery-voltage",
		.dispatch = getvar_battery_voltage,
		.list = true,
	}, {
		.variable = "erase-block-size",
		.dispatch = getvar_block_size,
		.list = true,
	}, {
		.variable = "logical-block-size",
		.dispatch = getvar_block_size,
		.list = true,
	}, {
		.variable = "secure",
		.dispatch = getvar_secure,
		.list = true,
	}, {
		.variable = "unlocked",
		.dispatch = getvar_unlocked,
		.list = true,
	}, {
		.variable = "is-userspace",
		.dispatch = getvar_is_userspace,
		.list = true,
	}, {
		.variable = "is-logical",
		.dispatch = getvar_is_logical,
		.list = false,
	}, {
		.variable = "super-partition-name",
		.dispatch = getvar_super_partition_name,
		.list = true,
	}, {
		.variable = "max-download-size",
		.dispatch = getvar_maxdownloadsize,
		.list = true,
	}, {
		.variable = "max-fetch-size",
		.dispatch = getvar_maxfetchsize,
		.list = true,
	}, {
		.variable = "downloadsize",
		.dispatch = getvar_downloadsize,
		.list = true
	}, {
		.variable = "serialno",
		.dispatch = getvar_serialno,
		.list = true
	}, {
		.variable = "version-baseband",
		.dispatch = getvar_version_baseband,
		.list = true
	}, {
		.variable = "product",
		.dispatch = getvar_product,
		.list = true
	}, {
		.variable = "platform",
		.dispatch = getvar_platform,
		.list = true
	}, {
		.variable = "current-slot",
		.dispatch = getvar_current_slot,
		.list = true,
	}, {
		.variable = "slot-count",
		.dispatch = getvar_slot_count,
		.list = true,
	}, {
		.variable = "slot-suffixes",
		.dispatch = getvar_slot_suffixes,
		.list = true,
	}, {
		.variable = "has-slot",
		.dispatch = getvar_has_slot,
		.list = false,
#if CONFIG_IS_ENABLED(FASTBOOT_FLASH)
	}, {
		.variable = "partition-type",
		.dispatch = getvar_partition_type,
		.list = false
#endif
#if CONFIG_IS_ENABLED(FASTBOOT_FLASH)
	}, {
		.variable = "partition-size",
		.dispatch = getvar_partition_size,
		.list = false
#endif
	}
};

static void getvar_version(char *var_parameter, char *response)
{
	fastboot_okay(FASTBOOT_VERSION, response);
}

static void getvar_bootloader_version(char *var_parameter, char *response)
{
	char s_version[32];
	strcpy(s_version, "01.01.\0");
	strcat(s_version, U_BOOT_DATE_TIME);
	printf("s_version: %s\n", s_version);
	fastboot_okay(s_version, response);
}

static void getvar_hw_revision(char *var_parameter, char *response)
{
	fastboot_okay("0", response);
}

static void getvar_off_mode_charge(char *var_parameter, char *response)
{
	fastboot_okay("0", response);
}

static void getvar_variant(char *var_parameter, char *response)
{
	const char *tmp = env_get("board");

	if (tmp)
		fastboot_okay(tmp, response);
	else
		fastboot_fail("Value not set", response);
}

static void getvar_battery_soc_ok(char *var_parameter, char *response)
{
	fastboot_okay("yes", response);
}

static void getvar_battery_voltage(char *var_parameter, char *response)
{
	fastboot_okay("4", response);
}

static void getvar_block_size(char *var_parameter, char *response)
{
	fastboot_okay("2000", response);
}

static void getvar_secure(char *var_parameter, char *response)
{
	if (IS_FEAT_BOOT_VERIFY())
		fastboot_okay("yes", response);
	else
		fastboot_okay("no", response);
}

static void getvar_unlocked(char *var_parameter, char *response)
{
	if (check_lock())
		fastboot_okay("no", response);
	else
		fastboot_okay("yes", response);
}

static void getvar_is_userspace(char *var_parameter, char *response)
{
	if (dynamic_partition)
		fastboot_okay("no", response);
	else
		fastboot_fail("do not use dynamic", response);
}

static void getvar_super_partition_name(char *var_parameter, char *response)
{
	fastboot_okay("super", response);
}

static void getvar_is_logical(char *var_parameter, char *response)
{
	char name[64] = {0};
	strncpy(name, var_parameter, strnlen(var_parameter, 64));
	name[63] = 0;
	if (has_boot_slot == 1) {
		char *slot_name;
		slot_name = env_get("slot-suffixes");
		if ((strcmp(var_parameter, "system") == 0) ||
			(strcmp(var_parameter, "vendor") == 0) ||
			(strcmp(var_parameter, "odm") == 0) ||
			(strcmp(var_parameter, "product") == 0) ||
			(strcmp(var_parameter, "system_ext") == 0) ||
			(strcmp(var_parameter, "dtbo") == 0) ||
			(strcmp(var_parameter, "boot") == 0) ||
			(strcmp(var_parameter, "recovery") == 0) ||
			(strcmp(var_parameter, "oem") == 0) ||
			(strcmp(var_parameter, "vbmeta_system") == 0) ||
			(strcmp(var_parameter, "vbmeta_vendor") == 0) ||
			(strcmp(var_parameter, "freertos") == 0) ||
			(strcmp(var_parameter, "init_boot") == 0) ||
			(strcmp(var_parameter, "vendor_boot") == 0) ||
			(strcmp(var_parameter, "bist") == 0) ||
			(strcmp(var_parameter, "vbmeta") == 0)) {
			if (slot_name && (strcmp(slot_name, "0") == 0)) {
				strcat(name, "_a");
			} else if (slot_name && (strcmp(slot_name, "1") == 0)) {
				strcat(name, "_b");
			}
		}
	}
	printf("partition name is %s\n", name);

	if (!dynamic_partition) {
		fastboot_okay("no", response);
	} else {
#ifdef CONFIG_BOOTLOADER_CONTROL_BLOCK
		if (is_partition_logical(name) == 0) {
			printf("%s is logic partition\n", name);
			fastboot_okay("yes", response);
		} else {
			fastboot_okay("no", response);
		}
#else
		fastboot_okay("no", response);
#endif
	}
}

static void getvar_slot_count(char *var_parameter, char *response)
{
	if (has_boot_slot == 1)
		fastboot_okay("2", response);
	else
		fastboot_okay("0", response);
}

static void getvar_downloadsize(char *var_parameter, char *response)
{
	fastboot_response("OKAY", response, "0x%08x", fastboot_buf_size);
}

static void getvar_maxdownloadsize(char *var_parameter, char *response)
{
	printf("kMaxDownloadSizeDefault: 0x%08x\n", kMaxDownloadSizeDefault);
	fastboot_response("OKAY", response, "0x%08x", kMaxDownloadSizeDefault);
}

static void getvar_maxfetchsize(char *var_parameter, char *response)
{
	fastboot_response("OKAY", response, "0x%08x", kMaxFetchSizeDefault);
}

static void getvar_serialno(char *var_parameter, char *response)
{
	const char *tmp = env_get("serial#");

	if (tmp)
		fastboot_okay(tmp, response);
	else
		fastboot_fail("Value not set", response);
}

static void getvar_version_baseband(char *var_parameter, char *response)
{
	fastboot_okay("N/A", response);
}

static void getvar_product(char *var_parameter, char *response)
{
	const char *board = env_get("board");

	if (board)
		fastboot_okay(board, response);
	else
		fastboot_fail("Board not set", response);
}

static void getvar_platform(char *var_parameter, char *response)
{
	const char *p = env_get("platform");

	if (p)
		fastboot_okay(p, response);
	else
		fastboot_fail("platform not set", response);
}

static void getvar_current_slot(char *var_parameter, char *response)
{
	char *slot;
	slot = env_get("slot-suffixes");
	/* A/B not implemented, for now always return _a */
	if (strcmp(slot, "0") == 0)
		fastboot_okay("a", response);
	else if (strcmp(slot, "1") == 0)
		fastboot_okay("b", response);
}
static void getvar_slot_suffixes(char *var_parameter, char *response)
{
	char *s;
	s = env_get("slot-suffixes");
	printf("slot-suffixes: %s\n", s);
	if (!strcmp(s, "-1") == 0)
		fastboot_okay(s, response);
	else
		fastboot_okay("0", response);
}

static void getvar_has_slot(char *part_name, char *response)
{
	if (has_boot_slot == 0) {
		fastboot_okay("no", response);
	} else {
		if ((strcmp(part_name, "system") == 0) ||
			(strcmp(part_name, "vendor") == 0) ||
			(strcmp(part_name, "odm") == 0) ||
			(strcmp(part_name, "product") == 0) ||
			(strcmp(part_name, "system_ext") == 0) ||
			(strcmp(part_name, "dtbo") == 0) ||
			(strcmp(part_name, "boot") == 0) ||
			(strcmp(part_name, "recovery") == 0) ||
			(strcmp(part_name, "vendor_boot") == 0) ||
			(strcmp(part_name, "bist") == 0) ||
			(strcmp(part_name, "vbmeta") == 0) ||
			(strcmp(part_name, "vbmeta_system") == 0) ||
			(strcmp(part_name, "vbmeta_vendor") == 0) ||
			(strcmp(part_name, "freertos") == 0) ||
			(strcmp(part_name, "init_boot") == 0) ||
			(strcmp(part_name, "odm_ext") == 0) || (strcmp(part_name, "oem") == 0)) {
			fastboot_okay("yes", response);
		} else {
			fastboot_okay("no", response);
		}
	}
}

#if CONFIG_IS_ENABLED(FASTBOOT_FLASH)
static int strcmp_l1(const char *s1, const char *s2)
{
	if (!s1 || !s2)
		return -1;
	return strncmp(s2, s1, strlen(s2));
}

static void partition_type_reply(char *part_name, char *response, char *part_type)
{
	u64 rc = 0;
	if (strcmp_l1(part_name, "env") != 0)
		rc = store_part_size(part_name);

	if (rc == -1)
		fastboot_fail("partition not found", response);
	else
		fastboot_okay(part_type, response);
}

/*
 * get the partition fs by name
 * return value
 *     = 1 means f2fs
 *     = 0 means ext4
 *     = -1 means no partition found
 */
static int is_f2fs_by_name(char const *name)
{
	int ret = -1;
#ifdef CONFIG_FASTBOOT_FLASH_MMC_DEV
	struct partitions *partition = NULL;

	partition = find_mmc_partition_by_name(name);
	if (!partition)
		return ret;
	ret = (partition->mask_flags >> 12) & 0x1;
#endif

	return ret;
}

static void getvar_partition_type(char *part_name, char *response)
{
	if ((strcmp_l1(part_name, "system") == 0) || (strcmp_l1(part_name, "vendor") == 0) ||
			(strcmp_l1(part_name, "init_boot") == 0) ||
			(strcmp_l1(part_name, "odm") == 0) ||
			(strcmp_l1(part_name, "product") == 0) ||
			(strcmp_l1(part_name, "system_ext") == 0) ||
			(strcmp_l1(part_name, "dtbo") == 0) ||
			(strcmp_l1(part_name, "vbmeta") == 0) ||
			(strcmp_l1(part_name, "vbmeta_system") == 0) ||
			(strcmp_l1(part_name, "vbmeta_vendor") == 0) ||
			(strcmp_l1(part_name, "super") == 0) ||
			(strcmp_l1(part_name, "car_param") == 0) ||
			(strcmp_l1(part_name, "odm_ext") == 0) ||
			(strcmp_l1(part_name, "production_ext") == 0) ||
			(strcmp_l1(part_name, "oem") == 0)) {
		partition_type_reply(part_name, response, "ext4");
	} else if ((strcmp(part_name, "data") == 0) || (strcmp(part_name, "userdata") == 0)) {
		if (is_f2fs_by_name("data") == -1 && is_f2fs_by_name("userdata") == -1) {
			printf("no partition data or userdata found\n");
		} else if (is_f2fs_by_name("data") == 0 || is_f2fs_by_name("userdata") == 0) {
			printf("data is ext4\n");
			fastboot_okay("ext4", response);
		} else {
			printf("data is f2fs\n");
			fastboot_okay("f2fs", response);
		}
	} else if (strcmp(part_name, "metadata") == 0) {
		if (is_f2fs_by_name("metadata") == -1) {
			printf("no partition metadata found\n");
		} else if (is_f2fs_by_name("metadata") == 0) {
			printf("metadata is ext4\n");
			partition_type_reply(part_name, response, "ext4");
		} else {
			printf("metadata is f2fs\n");
			partition_type_reply(part_name, response, "f2fs");
		}
	} else if (strcmp(part_name, "cache") == 0) {
		if (has_boot_slot == 0)
			fastboot_okay("ext4", response);
		else
			fastboot_fail("partition not found", response);
	} else if ((strcmp_l1(part_name, "avb_custom_key") == 0) ||
				(strcmp(part_name, "gpt") == 0)) {
		fastboot_okay("raw", response);
	} else {
		partition_type_reply(part_name, response, "raw");
	}
}

static void getvar_partition_size(char *part_name, char *response)
{
	int r = 0;
	size_t size = 0;
	char name[32] = {0};
	u64 rc = 0;

	if (strcmp(part_name, "userdata") == 0 || strcmp(part_name, "data") == 0) {
		rc = store_part_size("userdata");
		if (-1 == rc)
			strcpy(name, "data");
		else
			strcpy(name, "userdata");
	} else {
		strncpy(name, part_name, 31);
	}
	strcat(name, "\0");

#if CONFIG_IS_ENABLED(FASTBOOT_FLASH_MMC)
	struct blk_desc *dev_desc;
	struct disk_partition part_info;

	if (!strncmp("bootloader", part_name, strlen("bootloader"))) {
		int capacity_boot = 0;

		if (IS_ENABLED(CONFIG_MMC_MESON_GX)) {
			struct mmc *mmc = NULL;

			if (store_get_type() == BOOT_EMMC)
				mmc = find_mmc_device(1);

			if (mmc)
				capacity_boot = mmc->capacity_boot;
		}

		size = capacity_boot;

		printf("capacity_boot: %x\n", capacity_boot);
		printf("size: 0x%016zx\n", size);
	} else {
		r = fastboot_mmc_get_part_info(name, &dev_desc, &part_info, response);
		if (r >= 0)
			size = part_info.size * 512;
	}
#endif
/*#if CONFIG_IS_ENABLED(FASTBOOT_FLASH_NAND)
	struct part_info *part_info;

	r = fastboot_nand_get_part_info(name, &part_info, response);
	if (r >= 0)
		size = part_info->size * 512;
#endif*/

	if (r >= 0)
		fastboot_response("OKAY", response, "0x%016zx", size);
	else
		fastboot_fail("get partition size error", response);
}
#endif

static int current_all_dispatch;
void fastboot_getvar_all(char *response)
{
	/*
	 * Find a dispatch getvar that can be listed and send
	 * it as INFO until we reach the end.
	 */
	while (current_all_dispatch < ARRAY_SIZE(getvar_dispatch)) {
		if (!getvar_dispatch[current_all_dispatch].list) {
			current_all_dispatch++;
			continue;
		}

		char envstr[FASTBOOT_RESPONSE_LEN] = { 0 };

		getvar_dispatch[current_all_dispatch].dispatch(NULL, envstr);

		char *envstr_start = envstr;

		if (!strncmp("OKAY", envstr, 4) || !strncmp("FAIL", envstr, 4))
			envstr_start += 4;

		fastboot_response("INFO", response, "%s: %s",
				  getvar_dispatch[current_all_dispatch].variable,
				  envstr_start);

		current_all_dispatch++;
		return;
	}

	fastboot_response("OKAY", response, NULL);
	current_all_dispatch = 0;
}

/**
 * fastboot_getvar() - Writes variable indicated by cmd_parameter to response.
 *
 * @cmd_parameter: Pointer to command parameter
 * @response: Pointer to fastboot response buffer
 *
 * Look up cmd_parameter first as an environment variable of the form
 * fastboot.<cmd_parameter>, if that exists return use its value to set
 * response.
 *
 * Otherwise lookup the name of variable and execute the appropriate
 * function to return the requested value.
 */
void fastboot_getvar(char *cmd_parameter, char *response)
{
	if (!cmd_parameter) {
		fastboot_fail("missing var", response);
	} else if (!strncmp("all", cmd_parameter, 3) && strlen(cmd_parameter) == 3) {
		current_all_dispatch = 0;
		fastboot_response(FASTBOOT_MULTIRESPONSE_START, response, NULL);
	} else {
#define FASTBOOT_ENV_PREFIX	"fastboot."
		int i;
		char *var_parameter = cmd_parameter;
		char envstr[FASTBOOT_RESPONSE_LEN];
		const char *s;

		snprintf(envstr, sizeof(envstr) - 1,
			 FASTBOOT_ENV_PREFIX "%s", cmd_parameter);
		s = env_get(envstr);
		if (s) {
			fastboot_response("OKAY", response, "%s", s);
			return;
		}

		printf("fastboot_getvar cmd_parameter: %s\n", cmd_parameter);

		strsep(&var_parameter, ":");
		for (i = 0; i < ARRAY_SIZE(getvar_dispatch); ++i) {
			if (!strcmp(getvar_dispatch[i].variable,
				    cmd_parameter)) {
				getvar_dispatch[i].dispatch(var_parameter,
							    response);
				return;
			}
		}
		pr_warn("WARNING: unknown variable: %s\n", cmd_parameter);
		fastboot_fail("Variable not implemented", response);
	}
}
