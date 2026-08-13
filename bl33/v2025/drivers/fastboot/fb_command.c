// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (C) 2016 The Android Open Source Project
 */

#include <command.h>
#include <console.h>
#include <env.h>
#include <fastboot.h>
#include <fastboot-internal.h>
#include <fb_mmc.h>
#include <fb_nand.h>
#include <part.h>
#include <stdlib.h>
#include <vsprintf.h>
#include <linux/printk.h>
#ifdef CONFIG_AMLOGIC_MODIFY
#include <amlogic/emmc_partitions.h>
#include <amlogic/storage.h>
#include <amlogic/aml_efuse.h>
#include <amlogic/aml_mmc.h>

#include <android_image.h>
#include <image.h>
#include <amlogic/store_wrapper.h>
#include <malloc.h>
#include <amlogic/image_check.h>
#include <fs.h>
#include <version.h>
#if defined(CONFIG_AML_ANTIROLLBACK) || defined(CONFIG_AML_AVB2_ANTIROLLBACK)
#include <amlogic/anti-rollback.h>
#endif
#endif

/**
 * image_size - final fastboot image size
 */
static u32 image_size;

/**
 * fastboot_bytes_received - number of bytes received in the current download
 */
static u32 fastboot_bytes_received;

/**
 * fastboot_bytes_expected - number of bytes expected in the current download
 */
static u32 fastboot_bytes_expected;

static void okay(char *, char *);
static void getvar(char *, char *);

#ifdef CONFIG_FASTBOOT_WRITING_CMD
static void download(char *, char *);
static void flash(char *, char *);
static void erase(char *, char *);
#endif
static void reboot_bootloader(char *, char *);
static void reboot_fastbootd(char *, char *);
static void reboot_recovery(char *, char *);
#ifdef CONFIG_FASTBOOT_WRITING_CMD
#ifdef CONFIG_AMLOGIC_MODIFY
static void fetch(char *, char *);
static void oem_cmd(char *, char *);
#if !CONFIG_IS_ENABLED(NO_FASTBOOT_FLASHING)
static void flashing(char *, char *);
#endif
static void set_active_cmd(char *, char *);
#endif
static void oem_format(char *, char *);
static void oem_partconf(char *, char *);
static void oem_bootbus(char *, char *);
#endif
static void oem_console(char *, char *);
static void oem_board(char *, char *);
#ifdef CONFIG_AMLOGIC_MODIFY
extern int is_partition_logical(char* partition_name);
#endif
static void run_ucmd(char *, char *);
static void run_acmd(char *, char *);

static const struct {
	const char *command;
	void (*dispatch)(char *cmd_parameter, char *response);
} commands[FASTBOOT_COMMAND_COUNT] = {
	[FASTBOOT_COMMAND_GETVAR] = {
		.command = "getvar",
		.dispatch = getvar
	},
#ifdef CONFIG_FASTBOOT_WRITING_CMD
	[FASTBOOT_COMMAND_DOWNLOAD] = {
		.command = "download",
		.dispatch = download
	},
#ifdef CONFIG_AMLOGIC_MODIFY
#if !CONFIG_IS_ENABLED(NO_FASTBOOT_FLASHING)
	[FASTBOOT_COMMAND_FLASHING] =  {
		.command = "flashing",
		.dispatch = flashing
	},
#endif
	[FASTBOOT_COMMAND_FETCH] =  {
		.command = "fetch",
		.dispatch = fetch
	},
	[FASTBOOT_COMMAND_OEM] = {
		.command = "oem",
		.dispatch = oem_cmd,
	},
#endif
	[FASTBOOT_COMMAND_FLASH] =  {
		.command = "flash",
		.dispatch = CONFIG_IS_ENABLED(FASTBOOT_FLASH, (flash), (NULL))
	},
	[FASTBOOT_COMMAND_ERASE] =  {
		.command = "erase",
		.dispatch = CONFIG_IS_ENABLED(FASTBOOT_FLASH, (erase), (NULL))
	},
	[FASTBOOT_COMMAND_BOOT] =  {
		.command = "boot",
		.dispatch = okay
	},
#endif
	[FASTBOOT_COMMAND_CONTINUE] =  {
		.command = "continue",
		.dispatch = okay
	},
	[FASTBOOT_COMMAND_REBOOT_BOOTLOADER] =  {
		.command = "reboot-bootloader",
		.dispatch = reboot_bootloader
	},
	[FASTBOOT_COMMAND_REBOOT_FASTBOOTD] =  {
		.command = "reboot-fastboot",
		.dispatch = reboot_fastbootd
	},
	[FASTBOOT_COMMAND_REBOOT_RECOVERY] =  {
		.command = "reboot-recovery",
		.dispatch = reboot_recovery
	},
	[FASTBOOT_COMMAND_REBOOT] =  {
		.command = "reboot",
		.dispatch = okay
	},
#ifdef CONFIG_FASTBOOT_WRITING_CMD
	[FASTBOOT_COMMAND_SET_ACTIVE] =  {
		.command = "set_active",
#ifdef CONFIG_AMLOGIC_MODIFY
		.dispatch = set_active_cmd
#else
		.dispatch = okay
#endif
	},
	[FASTBOOT_COMMAND_OEM_FORMAT] = {
		.command = "oem format",
		.dispatch = CONFIG_IS_ENABLED(FASTBOOT_CMD_OEM_FORMAT, (oem_format), (NULL))
	},
	[FASTBOOT_COMMAND_OEM_PARTCONF] = {
		.command = "oem partconf",
		.dispatch = CONFIG_IS_ENABLED(FASTBOOT_CMD_OEM_PARTCONF, (oem_partconf), (NULL))
	},
	[FASTBOOT_COMMAND_OEM_BOOTBUS] = {
		.command = "oem bootbus",
		.dispatch = CONFIG_IS_ENABLED(FASTBOOT_CMD_OEM_BOOTBUS, (oem_bootbus), (NULL))
	},
	[FASTBOOT_COMMAND_OEM_RUN] = {
		.command = "oem run",
		.dispatch = CONFIG_IS_ENABLED(FASTBOOT_OEM_RUN, (run_ucmd), (NULL))
	},
#endif
	[FASTBOOT_COMMAND_OEM_CONSOLE] = {
		.command = "oem console",
		.dispatch = CONFIG_IS_ENABLED(FASTBOOT_CMD_OEM_CONSOLE, (oem_console), (NULL))
	},
	[FASTBOOT_COMMAND_OEM_BOARD] = {
		.command = "oem board",
		.dispatch = CONFIG_IS_ENABLED(FASTBOOT_OEM_BOARD, (oem_board), (NULL))
	},
	[FASTBOOT_COMMAND_UCMD] = {
		.command = "UCmd",
		.dispatch = CONFIG_IS_ENABLED(FASTBOOT_UUU_SUPPORT, (run_ucmd), (NULL))
	},
	[FASTBOOT_COMMAND_ACMD] = {
		.command = "ACmd",
		.dispatch = CONFIG_IS_ENABLED(FASTBOOT_UUU_SUPPORT, (run_acmd), (NULL))
	},
};

#ifdef CONFIG_AMLOGIC_MODIFY
struct fastboot_read fastboot_readInfo;

static int strcmp_l1(const char *s1, const char *s2)
{
	if (!s1 || !s2)
		return -1;
	if (strlen(s2) < strlen(s1))
		return -2;
	return strncmp(s1, s2, strlen(s1));
}

void dump_lock_info(LockData_t info)
{
#if 0
	printf("info.version_major = %d\n", info.version_major);
	printf("info.version_minor = %d\n", info.version_minor);
	printf("info.unlock_ability = %d\n", info.unlock_ability);
	printf("info.lock_state = %d\n", info.lock_state);
	printf("info.lock_critical_state = %d\n", info.lock_critical_state);
	printf("info.lock_bootloader = %d\n", info.lock_bootloader);
#endif
}
#endif

/**
 * fastboot_handle_command - Handle fastboot command
 *
 * @cmd_string: Pointer to command string
 * @response: Pointer to fastboot response buffer
 *
 * Return: Executed command, or -1 if not recognized
 */
int fastboot_handle_command(char *cmd_string, char *response)
{
	int i;
	char *cmd_parameter;

	cmd_parameter = cmd_string;
	strsep(&cmd_parameter, ":");

	if (CONFIG_IS_ENABLED(NO_FASTBOOT_FLASHING) && !strcmp_l1("flashing", cmd_string)) {
		pr_err("For product mode, command: %s is not supported.\n", cmd_string);
		fastboot_fail("For product mode, flashing lock/unlock is not supported.", response);
		return -1;
	}

	for (i = 0; i < FASTBOOT_COMMAND_COUNT; i++) {
#ifdef CONFIG_AMLOGIC_MODIFY
		if (!strcmp_l1(commands[i].command, cmd_string)) {
			if (commands[i].dispatch) {
				if (cmd_parameter) {
					commands[i].dispatch(cmd_parameter,
							response);
				} else {
					commands[i].dispatch(cmd_string,
							response);
				}
				return i;
			} else {
				break;
			}
		}
#else
		if (!strcmp(commands[i].command, cmd_string)) {
			if (commands[i].dispatch) {
				commands[i].dispatch(cmd_parameter,
							response);
				return i;
			} else {
				pr_err("command %s not supported.\n", cmd_string);
				fastboot_fail("Unsupported command", response);
				return -1;
			}
		}
#endif
	}

	pr_err("command %s not recognized.\n", cmd_string);
	fastboot_fail("unrecognized command", response);
	return -1;
}

void fastboot_multiresponse(int cmd, char *response)
{
	switch (cmd) {
	case FASTBOOT_COMMAND_GETVAR:
		fastboot_getvar_all(response);
		break;
	case FASTBOOT_COMMAND_OEM_CONSOLE:
		if (CONFIG_IS_ENABLED(FASTBOOT_CMD_OEM_CONSOLE)) {
			char buf[FASTBOOT_RESPONSE_LEN] = { 0 };

			if (console_record_isempty()) {
				console_record_reset();
				fastboot_okay(NULL, response);
			} else {
				int ret = console_record_readline(buf, sizeof(buf) - 5);

				if (ret < 0)
					fastboot_fail("Error reading console", response);
				else
					fastboot_response("INFO", response, "%s", buf);
			}
			break;
		}
	default:
		fastboot_fail("Unknown multiresponse command", response);
		break;
	}
}

/**
 * okay() - Send bare OKAY response
 *
 * @cmd_parameter: Pointer to command parameter
 * @response: Pointer to fastboot response buffer
 *
 * Send a bare OKAY fastboot response. This is used where the command is
 * valid, but all the work is done after the response has been sent (e.g.
 * boot, reboot etc.)
 */
static void okay(char *cmd_parameter, char *response)
{
	fastboot_okay(NULL, response);
}

/**
 * getvar() - Read a config/version variable
 *
 * @cmd_parameter: Pointer to command parameter
 * @response: Pointer to fastboot response buffer
 */
static void getvar(char *cmd_parameter, char *response)
{
	fastboot_getvar(cmd_parameter, response);
}

/**
 * fastboot_download() - Start a download transfer from the client
 *
 * @cmd_parameter: Pointer to command parameter
 * @response: Pointer to fastboot response buffer
 */
#ifdef CONFIG_FASTBOOT_WRITING_CMD
static void download(char *cmd_parameter, char *response)
{
	char *tmp;

#ifdef CONFIG_AMLOGIC_MODIFY
	if (check_lock() == 1) {
		printf("device is locked, can not run this cmd.Please flashing unlock & flashing unlock_critical\n");
		fastboot_fail("locked device", response);
		return;
	}
#endif

	if (!cmd_parameter) {
		fastboot_fail("Expected command parameter", response);
		return;
	}
	fastboot_bytes_received = 0;
	fastboot_bytes_expected = hextoul(cmd_parameter, &tmp);
	if (fastboot_bytes_expected == 0) {
		fastboot_fail("Expected nonzero image size", response);
		return;
	}
	/*
	 * Nothing to download yet. Response is of the form:
	 * [DATA|FAIL]$cmd_parameter
	 *
	 * where cmd_parameter is an 8 digit hexadecimal number
	 */
	if (fastboot_bytes_expected > fastboot_buf_size) {
#ifdef CONFIG_AMLOGIC_MODIFY
		char str[128] = {0};

		printf("fastboot_bytes_expected %d > fastboot_buf_size %d\n",
		       fastboot_bytes_expected, fastboot_buf_size);
		sprintf(str, "data too large, please add -S %dM",
			(fastboot_buf_size / 1024 / 1024));
		fastboot_fail(str, response);
#else
		fastboot_fail(cmd_parameter, response);
#endif
	} else {
		printf("Starting download of %d bytes\n",
		       fastboot_bytes_expected);
		fastboot_response("DATA", response, "%s", cmd_parameter);
	}
}
#endif
/**
 * fastboot_data_remaining() - return bytes remaining in current transfer
 *
 * Return: Number of bytes left in the current download
 */
u32 fastboot_data_remaining(void)
{
	return fastboot_bytes_expected - fastboot_bytes_received;
}

/**
 * fastboot_data_download() - Copy image data to fastboot_buf_addr.
 *
 * @fastboot_data: Pointer to received fastboot data
 * @fastboot_data_len: Length of received fastboot data
 * @response: Pointer to fastboot response buffer
 *
 * Copies image data from fastboot_data to fastboot_buf_addr. Writes to
 * response. fastboot_bytes_received is updated to indicate the number
 * of bytes that have been transferred.
 *
 * On completion sets image_size and ${filesize} to the total size of the
 * downloaded image.
 */
void fastboot_data_download(const void *fastboot_data,
			    unsigned int fastboot_data_len,
			    char *response)
{
#define BYTES_PER_DOT	0x20000
	u32 pre_dot_num, now_dot_num;

	if (fastboot_data_len == 0 ||
	    (fastboot_bytes_received + fastboot_data_len) >
	    fastboot_bytes_expected) {
		fastboot_fail("Received invalid data length",
			      response);
		return;
	}
	/* Download data to fastboot_buf_addr */
	memcpy(fastboot_buf_addr + fastboot_bytes_received,
	       fastboot_data, fastboot_data_len);

	pre_dot_num = fastboot_bytes_received / BYTES_PER_DOT;
	fastboot_bytes_received += fastboot_data_len;
	now_dot_num = fastboot_bytes_received / BYTES_PER_DOT;

	if (pre_dot_num != now_dot_num) {
		putc('.');
		if (!(now_dot_num % 74))
			putc('\n');
	}
	*response = '\0';
}

/**
 * fastboot_data_complete() - Mark current transfer complete
 *
 * @response: Pointer to fastboot response buffer
 *
 * Set image_size and ${filesize} to the total size of the downloaded image.
 */
void fastboot_data_complete(char *response)
{
	/* Download complete. Respond with "OKAY" */
	fastboot_okay(NULL, response);
	printf("\ndownloading of %d bytes finished\n", fastboot_bytes_received);
	image_size = fastboot_bytes_received;
	env_set_hex("filesize", image_size);
	fastboot_bytes_expected = 0;
	fastboot_bytes_received = 0;
}

#ifdef CONFIG_FASTBOOT_WRITING_CMD
#ifdef CONFIG_AMLOGIC_MODIFY
#if CONFIG_IS_ENABLED(FASTBOOT_FLASH_MMC)
static void write_dts_reserve(void)
{
	int ret;
	void *addr = NULL;
	char *mem_addr;

	if (run_command("imgread dtb ${boot_part} ${dtb_mem_addr}", 0)) {
		printf("Fail in load dtb\n");
	} else {
		mem_addr = env_get("dtb_mem_addr");

		if (mem_addr) {
			addr = (void *)simple_strtoul(mem_addr, NULL, 16);
			ret = dtb_write(addr);
			if (ret)
				printf("write dtb error\n");
			else
				printf("write dtb ok\n");
		}
	}
}

#if 0
static int clear_misc_partition(void)
{
	char *partition = "misc";
	char *buffer = NULL;
	u64 rc = 0;

	rc = store_part_size(partition);
	buffer = (char *)malloc(rc - AVB_CUSTOM_KEY_LEN_MAX);
	if (!buffer) {
		printf("malloc error\n");
		return -1;
	}
	memset(buffer, 0, rc - AVB_CUSTOM_KEY_LEN_MAX);

	store_write((const char *)partition, 0, rc - AVB_CUSTOM_KEY_LEN_MAX,
		(unsigned char *)buffer);

	free(buffer);

	return 0;
}
#endif

static void set_fastboot_flag(int flag)
{
	env_set("default_env", "1");
	if (flag == 1)
		env_set("fastboot_step", "1");
#if CONFIG_IS_ENABLED(AML_UPDATE_ENV)
	run_command("update_env_part -p default_env;", 0);
	if (flag == 1)
		run_command("update_env_part -p fastboot_step;", 0);
#else
	run_command("defenv_reserve;", 0);
	run_command("setenv default_env 1;", 0);
	if (flag == 1)
		run_command("setenv fastboot_step 1;", 0);
	run_command("saveenv;", 0);
#endif//#if CONFIG_IS_ENABLED(AML_UPDATE_ENV)
}
#endif
#endif

/**
 * flash() - write the downloaded image to the indicated partition.
 *
 * @cmd_parameter: Pointer to partition name
 * @response: Pointer to fastboot response buffer
 *
 * Writes the previously downloaded image to the partition indicated by
 * cmd_parameter. Writes to response.
 */
static void __maybe_unused flash(char *cmd_parameter, char *response)
{
#ifdef CONFIG_AMLOGIC_MODIFY
	char name[32] = {0};
	u64 rc = 0;
	int ret = -1;

	if (check_lock() == 1) {
		printf("device is locked, can not run this cmd.Please flashing unlock & flashing unlock_critical\n");
		fastboot_fail("locked device", response);
		return;
	}

	printf("cmd_parameter: %s\n", cmd_parameter);

#if CONFIG_IS_ENABLED(FASTBOOT_FLASH_MMC)
	struct mmc *mmc = find_mmc_device(CONFIG_FASTBOOT_FLASH_MMC_DEV);
	if (mmc && strcmp(cmd_parameter, "bootloader") == 0) {
		printf("try to read gpt data from bootloader.img\n");
		struct blk_desc *dev_desc;
		int erase_flag = 0;
		char *bootloaderindex;
		char *slot_name = NULL;
		char partname[32] = {0};
		int gpt_flag = -1;
		gpt_header *gpt_h;

		/*ret = update_boot_hdr_4_s7d_reva(fastboot_buf_addr, image_size, 0);
		if (ret) {
			printf("Failed to write s7d reva boot0\n");
			fastboot_fail("Failed to write s7d reva boot0", response);
			return;
		}*/

		slot_name = env_get("active_slot");
		if (slot_name && (strcmp(slot_name, "_a") == 0))
			strcpy((char *)partname, "bootloader_a");
		else if (slot_name && (strcmp(slot_name, "_b") == 0))
			strcpy((char *)partname, "bootloader_b");
		else
			strcpy((char *)partname, "bootloader_up");

		rc = store_part_size(partname);
		if (rc != -1) {
			fastboot_mmc_erase(partname, response);
			fastboot_mmc_flash_write(partname, fastboot_buf_addr, image_size,
				response);
		}

		/* the max size of bootloader.img is 4M, we reserve 128k for gpt.bin
		 * so we put gpt.bin at offset 0x3DFE00
		 * 0 ~ 512 bootloader secure boot, we don't care it here.
		 * 512 ~ 0x3DFDFF  original bootloader.img and 0
		 * 0x3DFE00 ~ end  gpt.bin
		 */

		dev_desc = blk_get_dev("mmc", CONFIG_FASTBOOT_FLASH_MMC_DEV);
		if (!dev_desc || dev_desc->type == DEV_TYPE_UNKNOWN) {
			printf("invalid mmc device\n");
			fastboot_fail("invalid mmc device", response);
			return;
		}

		gpt_h = fastboot_buf_addr + 0x3DFE00 + (GPT_PRIMARY_PARTITION_TABLE_LBA *
		       dev_desc->blksz);
		if (le64_to_cpu(gpt_h->signature) != GPT_HEADER_SIGNATURE_UBOOT) {
			printf("printf normal bootloader.img, no gpt partition table\n");
		} else {
			check_gpt_part(dev_desc, fastboot_buf_addr + 0x3DFE00);
			if (is_valid_gpt_buf(dev_desc, fastboot_buf_addr + 0x3DFE00)) {
				printf("invalid gpt\n");
			} else {
				printf("find gpt partition table, update it\n"
					"and write bootloader to boot0/boot1\n");

				erase_flag = check_gpt_change(dev_desc, fastboot_buf_addr + 0x3DFE00);

				if (erase_flag != 0) {
					printf("partition changes, erase emmc\n");
					//run_command("store erase.chip 0;", 0);
					if (usb_burn_erase_data(0x3)) {
						printf("partition erase failed!\n");
						return;
					}
				}

				if (write_mbr_and_gpt_partitions(dev_desc, fastboot_buf_addr + 0x3DFE00)) {
					printf("%s: writing GPT partitions failed\n", __func__);
					fastboot_fail("writing GPT partitions failed", response);
					return;
				}

				if (mmc_device_init(mmc) != 0) {
					printf(" update gpt partition table fail\n");
					fastboot_fail("fastboot update gpt partition fail", response);
					return;
				}
				printf("%s: writing GPT partitions ok\n", __func__);

				char *mem_addr;
				void *addr = NULL;
				int ret;

				mem_addr = env_get("dtb_mem_addr");

				if (mem_addr && erase_flag == 1) {
					printf("partition changes, erase emmc\n");
					//run_command("store erase.chip 0;", 0);
					if (usb_burn_erase_data(0x3)) {
						printf("partition erase failed!\n");
						return;
					}
					printf("write _aml_dtb\n");
					addr = (void *)simple_strtoul(mem_addr, NULL, 16);
					ret = dtb_write(addr);
					if (ret)
						printf("write _aml_dtb error\n");
					else
						printf("write _aml_dtb ok\n");
				}
			}
		}

		gpt_flag = aml_gpt_valid(mmc);
		if (gpt_flag == 0)
			ret = 0;

#if defined(CONFIG_EFUSE_OBJ_API) && defined(CONFIG_CMD_EFUSE)
		run_command("efuse_obj get FEAT_DISABLE_EMMC_USER", 0);

		if (*efuse_field.data == 1) {
			wrnP("efuse_field.data == 1\n");
			ret = 0;
			env_set("nocs_mode", "true");
		} else {
			wrnP("efuse_field.data != 1\n");
			env_set("nocs_mode", "false");
		}
#endif//#ifdef CONFIG_EFUSE_OBJ_API

		bootloaderindex = env_get("forUpgrade_bootloaderIndex");

		if (ret == 0) {
			printf("gpt/nocs mode\n");
			fastboot_mmc_flash_write("bootloader-boot1", fastboot_buf_addr, image_size,
				 response);
			if (!bootloaderindex ||
				(bootloaderindex && strcmp(bootloaderindex, "1") != 0)) {
				printf("boot from boot1, means boot0 is error, rewrite it\n");
				fastboot_mmc_flash_write("bootloader-boot0",
					fastboot_buf_addr, image_size, response);
				run_command("mmc dev 1 0;", 0);
				set_fastboot_flag(0);
			} else {
				printf("need to set fastboot_step\n");
				run_command("mmc dev 1 0;", 0);
				set_fastboot_flag(1);
			}
			return;
		} else {
			printf("normal mode\n");
			fastboot_mmc_flash_write("bootloader-boot0", fastboot_buf_addr, image_size,
				response);
			fastboot_mmc_flash_write("bootloader-boot1", fastboot_buf_addr, image_size,
				 response);
			if (!bootloaderindex ||
				(bootloaderindex && strcmp(bootloaderindex, "0") != 0)) {
				printf("boot from boot0, rewrite user bootloader is error\n");
				fastboot_mmc_flash_write("bootloader",
					fastboot_buf_addr, image_size, response);
				run_command("mmc dev 1 0;", 0);
				set_fastboot_flag(0);
			} else {
				run_command("mmc dev 1 0;", 0);
				set_fastboot_flag(1);
			}
			return;
		}
	}
#endif

	if (strcmp(cmd_parameter, "avb_custom_key") == 0) {
		char* buffer = NULL;
		char *partition = "misc";
		AvbKey_t key;
		printf("avb_custom_key image_size: %d\n", image_size);

		if (image_size > AVB_CUSTOM_KEY_LEN_MAX - sizeof(AvbKey_t)) {
			printf("key size is too large\n");
			fastboot_fail("size error", response);
			return;
		}

		memcpy(key.magic_name, "AVBK", 4);
		key.size = image_size;
		rc = store_part_size(partition);

		buffer = (char *)malloc(AVB_CUSTOM_KEY_LEN_MAX);
		if (!buffer) {
			printf("malloc error\n");
			fastboot_fail("malloc error", response);
			return;
		}
		memset(buffer, 0, AVB_CUSTOM_KEY_LEN_MAX);
		memcpy(buffer, &key, sizeof(AvbKey_t));
		memcpy(buffer + sizeof(AvbKey_t), fastboot_buf_addr, image_size);

		store_write((const char *)partition, rc - AVB_CUSTOM_KEY_LEN_MAX,
			AVB_CUSTOM_KEY_LEN_MAX, (unsigned char *)buffer);

		fastboot_okay(NULL, response);
		free(buffer);
		return;
	}

	if (strcmp(cmd_parameter, "userdata") == 0 || strcmp(cmd_parameter, "data") == 0) {
		fastboot_okay(NULL, response);
		return;
	} else if (strcmp(cmd_parameter, "dts") == 0) {
		strcpy(name, "dtb");
	} else if (!strcmp(cmd_parameter, "bootloader-boot0") ||
		!strcmp(cmd_parameter, "bootloader-boot1")) {
		/*ret = update_boot_hdr_4_s7d_reva(fastboot_buf_addr, image_size, 0);
		if (ret) {
			printf("Failed to write s7d reva boot0\n");
			fastboot_fail("Failed to write s7d reva boot0-1", response);
			return;
		}*/
		strlcpy(name, cmd_parameter, 31);
	} else {
		strlcpy(name, cmd_parameter, 31);
	}
	strcat(name, "\0");

#ifdef CONFIG_BOOTLOADER_CONTROL_BLOCK
	if (dynamic_partition) {
		if (is_partition_logical(name) == 0) {
			printf("logic partition, can not write here.......\n");
			fastboot_fail("logic partition", response);
			return;
		}
	}
#endif

#if CONFIG_IS_ENABLED(FASTBOOT_FLASH_MMC)
	fastboot_mmc_flash_write(name, fastboot_buf_addr, image_size,
				 response);
#endif
#if CONFIG_IS_ENABLED(FASTBOOT_FLASH_NAND)
	fastboot_nand_flash_write(name, fastboot_buf_addr, image_size,
				  response);
#endif

	if (strcmp(name, "bootloader") == 0) {
		env_set("default_env", "1");
#if CONFIG_IS_ENABLED(AML_UPDATE_ENV)
		run_command("update_env_part -p default_env;", 0);
#else
		run_command("defenv_reserve;saveenv;", 0);
#endif// #if CONFIG_IS_ENABLED(AML_UPDATE_ENV)
	}

	if (strcmp(name, "bootloader-boot0") == 0 ||
		strcmp(name, "bootloader-boot1") == 0) {
		printf("try to switch back to user\n");
		run_command("mmc dev 1 0;", 0);
	}

#if CONFIG_IS_ENABLED(FASTBOOT_FLASH_MMC)
	if (mmc && aml_gpt_valid(mmc) == 0) {
		if (vendor_boot_partition) {
			if (strcmp_l1("vendor_boot", name) == 0) {
				printf("gpt mode, write dts to reserve\n");
				write_dts_reserve();
			}
		} else {
			if (strcmp_l1("boot", name) == 0) {
				printf("gpt mode, write dts to reserve\n");
				write_dts_reserve();
			}
		}
	}
#endif

#endif
}

/**
 * erase() - erase the indicated partition.
 *
 * @cmd_parameter: Pointer to partition name
 * @response: Pointer to fastboot response buffer
 *
 * Erases the partition indicated by cmd_parameter (clear to 0x00s). Writes
 * to response.
 */
static void __maybe_unused erase(char *cmd_parameter, char *response)
{
#ifdef CONFIG_AMLOGIC_MODIFY
	char name[32] = {0};
	u64 rc = 0;

	if (check_lock() == 1) {
		printf("device is locked, can not run this cmd.Please flashing unlock & flashing unlock_critical\n");
		fastboot_fail("locked device", response);
		return;
	}

	printf("cmd_parameter in erase: %s\n", cmd_parameter);

#ifdef CONFIG_FASTBOOT_FLASH_MMC_DEV
	struct mmc *mmc = find_mmc_device(CONFIG_FASTBOOT_FLASH_MMC_DEV);
	if ((mmc != NULL) && strcmp(cmd_parameter, "bootloader") == 0 && (aml_gpt_valid(mmc) == 0)) {
		printf("we write gpt partition table to bootloader now\n");
		printf("plese write bootloader to bootloader-boot0/bootloader-boot1\n");
		fastboot_okay("gpt mode, skip", response);
		return;
	}
#endif

	if (strcmp(cmd_parameter, "avb_custom_key") == 0) {
		char* buffer = NULL;
		char *partition = "misc";

		rc = store_part_size(partition);
		buffer = (char *)malloc(AVB_CUSTOM_KEY_LEN_MAX);
		if (!buffer) {
			printf("malloc error\n");
			fastboot_fail("malloc error", response);
			return;
		}
		memset(buffer, 0, AVB_CUSTOM_KEY_LEN_MAX);

		store_write((const char *)partition, rc - AVB_CUSTOM_KEY_LEN_MAX,
			AVB_CUSTOM_KEY_LEN_MAX, (unsigned char *)buffer);

		fastboot_okay(NULL, response);
		free(buffer);
		return;
	}

	if (strcmp(cmd_parameter, "env") == 0) {
		char *fastboot_step = env_get("fastboot_step");

		if (fastboot_step && strcmp(fastboot_step, "0")) {
			printf("fastboot_step: %s, run defenv_reserv\n", fastboot_step);
			run_command("defenv_reserv; setenv upgrade_step 2; saveenv;", 0);
			run_command("run bcb_cmd", 0);
			fastboot_okay(NULL, response);
			return;
		}
	}

	if (strcmp(cmd_parameter, "misc") == 0) {
		char* buffer = NULL;
		char *partition = "misc";

		rc = store_part_size(partition);

		buffer = (char *)malloc(rc - AVB_CUSTOM_KEY_LEN_MAX);
		if (!buffer) {
			printf("malloc error\n");
			fastboot_fail("malloc error", response);
			return;
		}
		memset(buffer, 0, rc - AVB_CUSTOM_KEY_LEN_MAX);

		store_write((const char *)partition, 0, rc - AVB_CUSTOM_KEY_LEN_MAX,
			(unsigned char *)buffer);

		fastboot_okay(NULL, response);
		free(buffer);
		return;
	}

	if (strcmp(cmd_parameter, "userdata") == 0 || strcmp(cmd_parameter, "data") == 0) {
		rc = store_part_size("userdata");
		if (-1 == rc)
			strcpy(name, "data");
		else
			strcpy(name, "userdata");
	} else if (strcmp(cmd_parameter, "dts") == 0) {
		strcpy(name, "dtb");
	} else {
		strncpy(name, cmd_parameter, 31);
	}
	strcat(name, "\0");

#ifdef CONFIG_BOOTLOADER_CONTROL_BLOCK
		if (dynamic_partition) {
			if (is_partition_logical(name) == 0) {
				printf("logic partition, can not erase here.......\n");
				fastboot_fail("logic partition", response);
				return;
			}
		}
#endif

#if CONFIG_IS_ENABLED(FASTBOOT_FLASH_MMC)
	fastboot_mmc_erase(name, response);
#endif
#if CONFIG_IS_ENABLED(FASTBOOT_FLASH_NAND)
	fastboot_nand_erase(name, response);
#endif

#endif
}

#ifdef CONFIG_AMLOGIC_MODIFY
static u64 my_ato11(const char *str)
{
	u64 result = 0;
	int len, i;

	len = strlen(str);
	for (i = 0; i < len; i++) {
		if (*str >= '0' && *str <= '9')
			result = result * 16 + (*str - '0');
		else if (*str >= 'A' && *str <= 'F')
			result = result * 16 + (*str - 'A') + 10;
		else if (*str >= 'a' && *str <= 'f')
			result = result * 16 + (*str - 'a') + 10;
		str++;
	}

	return result;
}

static void fetch(char *cmd_parameter, char *response)
{
	int len;
	int i = 0;
	char *cmd;
	char name[32] = {0};
	u64 offset = 0;
	u64 read_size = 0;
	size_t size = 0;

	if (check_lock() == 1) {
		printf("device is locked, can not run this cmd\n");
		fastboot_fail("locked device", response);
		return;
	}

	cmd = cmd_parameter;
	len = strlen(cmd_parameter);
	while (strsep(&cmd, ":"))
		i++;

	for (cmd = cmd_parameter, i = 0; cmd < (cmd_parameter + len); i++) {
		/* Skip to next assignment */
		if (i == 0) {
			strncpy(name, cmd, 31);
			strcat(name, "\0");
		} else if (i == 1) {
			offset = my_ato11(cmd);
		} else if (i == 2) {
			read_size = my_ato11(cmd);
		}

		for (cmd += strlen(cmd); cmd < (cmd_parameter + len) && !*cmd;)
			cmd++;
	}

	printf("name: %s\n", name);
	if (strncmp("vendor_boot", name, strlen("vendor_boot")) != 0) {
		printf("We can only %s vendor_boot\n", __func__);
		fastboot_fail("Fetch is only allowed on vendor_boot", response);
		return;
	}

#if CONFIG_IS_ENABLED(FASTBOOT_FLASH_MMC)
	struct blk_desc *dev_desc;
	struct disk_partition part_info;
	int r;

	r = fastboot_mmc_get_part_info(name, &dev_desc, &part_info,
				       response);
	if (r >= 0)
		size = part_info.size * 512;
#endif
#if CONFIG_IS_ENABLED(FASTBOOT_FLASH_NAND)
	struct part_info *part_info;
	int r;

	r = fastboot_nand_get_part_info(name, &part_info, response);
	if (r >= 0)
		size = part_info->size * 512;
#endif

	if (offset > size) {
		printf("Invalid offset: 0x%llx, partition size is 0x%lx\n",
			offset, size);
		fastboot_fail("Invalid offset", response);
		return;
	}

	if (read_size == 0 || read_size > size - offset ||
			read_size > kMaxFetchSizeDefault) {
		printf("Invalid read size: 0x%llx, partition size is 0x%lx\n",
			offset, size);
		fastboot_fail("Invalid read size", response);
		return;
	}

	printf("Start read %s partition datas!\n", name);
	void *buffer;
	char str[128] = {0};

	buffer = (void *)CONFIG_FASTBOOT_BUF_ADDR;
	sprintf(str, "DATA%12llx", read_size);
	printf("str: %s, len: %ld\n", str, strlen(str));
	memcpy(buffer, str, strlen(str));

	if (store_read((const char *)name, offset, read_size,
		(unsigned char *)buffer + strlen(str)) < 0) {
		printf("failed to store read %s.\n", name);
		fastboot_fail("read partition data error", response);
		return;
	}

	fastboot_readInfo.transferredBytes = 0;
	fastboot_readInfo.totalBytes = read_size + strlen(str);
	fastboot_response("DATA", response, "%12llx", read_size);
}

static void oem_cmd(char *cmd_parameter, char *response)
{
	char *cmd;
	int i = 0, len = 0, j = 0;
	char cmd_str[FASTBOOT_RESPONSE_LEN];

	printf("oem cmd_parameter: %s\n", cmd_parameter);

	if (IS_FEAT_BOOT_VERIFY()) {
		printf("device is secure mode, can not run this cmd.\n");
		fastboot_fail("secure boot device", response);
		return;
	}

	if (check_lock() == 1) {
		printf("device is locked, can not run this cmd.Please flashing unlock & flashing unlock_critical\n");
		fastboot_fail("locked device", response);
		return;
	}

	cmd = cmd_parameter;
	strsep(&cmd, " ");
	printf("To run cmd[%s]\n", cmd);

	len = strlen(cmd);
	for (i = 0; i < len; i++) {
		if (cmd[i] != '\'')
			cmd_str[j++] = cmd[i];
	}
	cmd_str[j] = '\0';
	printf("cmd_str2: %s\n", cmd_str);

	run_command(cmd_str, 0);

	fastboot_okay(NULL, response);
}

static void set_active_cmd(char *cmd_parameter, char *response)
{
	char *cmd;
	int ret = 0;
	char str[128];

	printf("cmd cb_set_active is %s\n", cmd_parameter);
	cmd = cmd_parameter;

	if (check_lock() == 1) {
		printf("device is locked, can not run this cmd.Please flashing unlock & flashing unlock_critical\n");
		fastboot_fail("locked device", response);
		return;
	}

	sprintf(str, "set_active_slot %s", cmd);
	printf("command:    %s\n", str);
	ret = run_command(str, 0);
	printf("ret = %d\n", ret);
	if (ret == 0)
		fastboot_okay(NULL, response);
	else
		fastboot_fail("set slot error", response);
}

#if !CONFIG_IS_ENABLED(NO_FASTBOOT_FLASHING)
#ifdef CONFIG_AVB2
static void try_unlock_dev(u64 rc)
{
#if defined(CONFIG_AML_ANTIROLLBACK) || defined(CONFIG_AML_AVB2_ANTIROLLBACK)
	if (is_avb_arb_available()) {
		if (avb_unlock()) {
			if (-1 == rc) {
				printf("unlocking device.  Erasing data partition!\n");
				run_command("store erase data 0 0", 0);
			} else {
				printf("unlocking device.  Erasing userdata partition!\n");
				run_command("store erase userdata 0 0", 0);
			}
			printf("unlocking device.  Erasing metadata partition!\n");
			run_command("store erase metadata 0 0", 0);
		} else {
			printf("unlock failed!\n");
		}

		return;
	}
#endif
	if (-1 == rc) {
		printf("unlocking device.  Erasing data partition!\n");
		run_command("store erase data 0 0", 0);
	} else {
		printf("unlocking device.  Erasing userdata partition!\n");
		run_command("store erase userdata 0 0", 0);
	}
	printf("unlocking device.  Erasing metadata partition!\n");
	run_command("store erase metadata 0 0", 0);
}

static void try_lock_dev(u64 rc)
{
#if defined(CONFIG_AML_ANTIROLLBACK) || defined(CONFIG_AML_AVB2_ANTIROLLBACK)
	if (is_avb_arb_available()) {
		if (avb_lock()) {
			if (-1 == rc) {
				printf("locking device.  Erasing data partition!\n");
				run_command("store erase data 0 0", 0);
			} else {
				printf("locking device.  Erasing userdata partition!\n");
				run_command("store erase userdata 0 0", 0);
			}
			printf("locking device.  Erasing metadata partition!\n");
			run_command("store erase metadata 0 0", 0);
		} else {
			printf("lock failed!\n");
		}

		return;
	}
#endif
	if (-1 == rc) {
		printf("locking device.  Erasing data partition!\n");
		run_command("store erase data 0 0", 0);
	} else {
		printf("locking device.  Erasing userdata partition!\n");
		run_command("store erase userdata 0 0", 0);
	}
	printf("locking device.  Erasing metadata partition!\n");
	run_command("store erase metadata 0 0", 0);
}
#endif

/**
 * flashing() - lock/unlock.
 *
 * @cmd_parameter: Pointer to partition name
 * @response: Pointer to fastboot response buffer
 *
 * Writes the previously downloaded image to the partition indicated by
 * cmd_parameter. Writes to response.
 */
static void flashing(char *cmd_parameter, char *response)
{
	char *cmd;
	char* lock_s;
	LockData_t info = {0};
	char lock_d[LOCK_DATA_SIZE];
	u64 rc;

#ifndef CONFIG_REFERENCE
	if (IS_FEAT_BOOT_VERIFY()) {
		printf("device is secure mode, can not run this cmd.\n");
		fastboot_fail("secure boot device", response);
		return;
	}
#endif

	lock_s = env_get("lock");
	if (!lock_s) {
		printf("lock state is NULL \n");
		memcpy(lock_d, "10101000", 8);
		lock_s = "10101000";
		env_set("lock", "10101000");
#if CONFIG_IS_ENABLED(AML_UPDATE_ENV)
		run_command("update_env_part -p lock;", 0);
#else
		run_command("defenv_reserv; saveenv;", 0);
#endif//#if CONFIG_IS_ENABLED(AML_UPDATE_ENV)
	} else {
		printf("lock state: %s\n", lock_s);
		if (strlen(lock_s) > 15)
			strncpy(lock_d, lock_s, 15);
		else
			strncpy(lock_d, lock_s, strlen(lock_s));
	}

	info.version_major = (int)(lock_d[0] - '0');
	info.version_minor = (int)(lock_d[1] - '0');
	info.unlock_ability = (int)(lock_d[2] - '0');
	info.lock_state = (int)(lock_d[4] - '0');
	info.lock_critical_state = (int)(lock_d[5] - '0');
	info.lock_bootloader = (int)(lock_d[6] - '0');
	dump_lock_info(info);

	printf("cb_flashing cmd_parameter: %s\n", cmd_parameter);
	cmd = cmd_parameter;
	strsep(&cmd, " ");
	printf("cb_flashing: %s\n", cmd);
	if (!cmd) {
		printf("missing variable\n");
		fastboot_fail("missing var", response);
		return;
	}

	if (info.unlock_ability == 0)
		info.unlock_ability = 1;

	rc = store_part_size("userdata");

	if (!strcmp_l1("unlock_critical", cmd)) {
		info.lock_critical_state = 0;
		fastboot_okay(NULL, response);
	} else if (!strcmp_l1("lock_critical", cmd)) {
		info.lock_critical_state = 1;
		fastboot_okay(NULL, response);
	} else if (!strcmp_l1("get_unlock_ability", cmd)) {
		fastboot_okay(NULL, response);
	} else if (!strcmp_l1("get_unlock_bootloader_nonce", cmd)) {
		char str_num[8];
		sprintf(str_num, "%d", info.lock_critical_state);
		fastboot_response("OKAY", response, "%s", str_num);
	} else if (!strcmp_l1("lock_bootloader", cmd)) {
		info.lock_bootloader = 1;
	} else if (!strcmp_l1("unlock", cmd)) {
#ifdef CONFIG_AVB2
#if defined(CONFIG_AML_ANTIROLLBACK) || defined(CONFIG_AML_AVB2_ANTIROLLBACK)
		u32 rpmb_lock_state = 0;
		bool ret = get_avb_lock_state(&rpmb_lock_state);

		if (info.lock_state == 1 || (ret && rpmb_lock_state == 1))
			try_unlock_dev(rc);
#else
		if (info.lock_state == 1)
			try_unlock_dev(rc);
#endif
#endif
		info.lock_state = 0;
		info.lock_critical_state = 0;
		env_set("lock_state", "green");
		fastboot_okay(NULL, response);
	} else if (!strcmp_l1("lock", cmd)) {
#ifdef CONFIG_AVB2
#if defined(CONFIG_AML_ANTIROLLBACK) || defined(CONFIG_AML_AVB2_ANTIROLLBACK)
		u32 rpmb_lock_state = 0;
		bool ret = get_avb_lock_state(&rpmb_lock_state);

		if (info.lock_state == 0 || (ret && rpmb_lock_state == 0))
			try_lock_dev(rc);
#else
		if (info.lock_state == 0)
			try_lock_dev(rc);
#endif
#endif
		info.lock_state = 1;
		env_set("lock_state", "orange");
		fastboot_okay(NULL, response);
	} else {
		printf("unknown variable: %s\n", cmd);
		fastboot_response("FAIL", response, "%s", "Variable not implemented");
	}

	sprintf(lock_d, "%d%d%d0%d%d%d0", info.version_major, info.version_minor,
		info.unlock_ability, info.lock_state, info.lock_critical_state,
		info.lock_bootloader);
	printf("lock_d state: %s\n", lock_d);
	env_set("lock", lock_d);
#if CONFIG_IS_ENABLED(AML_UPDATE_ENV)
	run_command("update_env_part -p lock;", 0);
#else
	run_command("defenv_reserv; saveenv;", 0);
#endif//#if CONFIG_IS_ENABLED(AML_UPDATE_ENV)
	return;
}
#endif// #if !CONFIG_IS_ENABLED(NO_FASTBOOT_FLASHING)
#endif
#endif

/**
 * run_ucmd() - Execute the UCmd command
 *
 * @cmd_parameter: Pointer to command parameter
 * @response: Pointer to fastboot response buffer
 */
static void __maybe_unused run_ucmd(char *cmd_parameter, char *response)
{
	if (!cmd_parameter) {
		pr_err("missing slot suffix\n");
		fastboot_fail("missing command", response);
		return;
	}

	if (run_command(cmd_parameter, 0))
		fastboot_fail("", response);
	else
		fastboot_okay(NULL, response);
}

static char g_a_cmd_buff[64];

void fastboot_acmd_complete(void)
{
	run_command(g_a_cmd_buff, 0);
}

/**
 * run_acmd() - Execute the ACmd command
 *
 * @cmd_parameter: Pointer to command parameter
 * @response: Pointer to fastboot response buffer
 */
static void __maybe_unused run_acmd(char *cmd_parameter, char *response)
{
	if (!cmd_parameter) {
		pr_err("missing slot suffix\n");
		fastboot_fail("missing command", response);
		return;
	}

	if (strlen(cmd_parameter) > sizeof(g_a_cmd_buff)) {
		pr_err("too long command\n");
		fastboot_fail("too long command", response);
		return;
	}

	strcpy(g_a_cmd_buff, cmd_parameter);
	fastboot_okay(NULL, response);
}

/**
 * reboot_bootloader() - Sets reboot bootloader flag.
 *
 * @cmd_parameter: Pointer to command parameter
 * @response: Pointer to fastboot response buffer
 */
static void reboot_bootloader(char *cmd_parameter, char *response)
{
#ifndef CONFIG_AMLOGIC_MODIFY
	if (fastboot_set_reboot_flag(FASTBOOT_REBOOT_REASON_BOOTLOADER))
		fastboot_fail("Cannot set reboot flag", response);
	else
#endif
		fastboot_okay(NULL, response);
}

/**
 * reboot_fastbootd() - Sets reboot fastboot flag.
 *
 * @cmd_parameter: Pointer to command parameter
 * @response: Pointer to fastboot response buffer
 */
static void reboot_fastbootd(char *cmd_parameter, char *response)
{
#ifndef CONFIG_AMLOGIC_MODIFY
	if (fastboot_set_reboot_flag(FASTBOOT_REBOOT_REASON_FASTBOOTD))
		fastboot_fail("Cannot set fastboot flag", response);
	else
#endif
		fastboot_okay(NULL, response);
}

/**
 * reboot_recovery() - Sets reboot recovery flag.
 *
 * @cmd_parameter: Pointer to command parameter
 * @response: Pointer to fastboot response buffer
 */
static void reboot_recovery(char *cmd_parameter, char *response)
{
#ifndef CONFIG_AMLOGIC_MODIFY
	if (fastboot_set_reboot_flag(FASTBOOT_REBOOT_REASON_RECOVERY))
		fastboot_fail("Cannot set recovery flag", response);
	else
#endif
		fastboot_okay(NULL, response);
}

#ifdef CONFIG_FASTBOOT_WRITING_CMD
/**
 * oem_format() - Execute the OEM format command
 *
 * @cmd_parameter: Pointer to command parameter
 * @response: Pointer to fastboot response buffer
 */
static void __maybe_unused oem_format(char *cmd_parameter, char *response)
{
	char cmdbuf[32];
	const int mmc_dev = config_opt_enabled(CONFIG_FASTBOOT_FLASH_MMC,
					       CONFIG_FASTBOOT_FLASH_MMC_DEV, -1);

#ifdef CONFIG_AMLOGIC_MODIFY
#ifndef CONFIG_REFERENCE
	if (IS_FEAT_BOOT_VERIFY()) {
		printf("device is secure mode, can not run this cmd.\n");
		fastboot_fail("secure boot device", response);
		return;
	}
#endif

	if (check_lock() == 1) {
		printf("device is locked, can not run this cmd.Please flashing unlock & flashing unlock_critical\n");
		fastboot_fail("locked device", response);
		return;
	}
#endif

	if (!env_get("partitions")) {
		fastboot_fail("partitions not set", response);
	} else {
		sprintf(cmdbuf, "gpt write mmc %x $partitions", mmc_dev);
		if (run_command(cmdbuf, 0))
			fastboot_fail("", response);
		else
			fastboot_okay(NULL, response);
	}
}

/**
 * oem_partconf() - Execute the OEM partconf command
 *
 * @cmd_parameter: Pointer to command parameter
 * @response: Pointer to fastboot response buffer
 */
static void __maybe_unused oem_partconf(char *cmd_parameter, char *response)
{
	char cmdbuf[32];
	const int mmc_dev = config_opt_enabled(CONFIG_FASTBOOT_FLASH_MMC,
					       CONFIG_FASTBOOT_FLASH_MMC_DEV, -1);

	if (!cmd_parameter) {
		fastboot_fail("Expected command parameter", response);
		return;
	}

	/* execute 'mmc partconfg' command with cmd_parameter arguments*/
	snprintf(cmdbuf, sizeof(cmdbuf), "mmc partconf %x %s 0", mmc_dev, cmd_parameter);
	printf("Execute: %s\n", cmdbuf);
	if (run_command(cmdbuf, 0))
		fastboot_fail("Cannot set oem partconf", response);
	else
		fastboot_okay(NULL, response);
}

/**
 * oem_bootbus() - Execute the OEM bootbus command
 *
 * @cmd_parameter: Pointer to command parameter
 * @response: Pointer to fastboot response buffer
 */
static void __maybe_unused oem_bootbus(char *cmd_parameter, char *response)
{
	char cmdbuf[32];
	const int mmc_dev = config_opt_enabled(CONFIG_FASTBOOT_FLASH_MMC,
					       CONFIG_FASTBOOT_FLASH_MMC_DEV, -1);

	if (!cmd_parameter) {
		fastboot_fail("Expected command parameter", response);
		return;
	}

	/* execute 'mmc bootbus' command with cmd_parameter arguments*/
	snprintf(cmdbuf, sizeof(cmdbuf), "mmc bootbus %x %s", mmc_dev, cmd_parameter);
	printf("Execute: %s\n", cmdbuf);
	if (run_command(cmdbuf, 0))
		fastboot_fail("Cannot set oem bootbus", response);
	else
		fastboot_okay(NULL, response);
}
#endif

/**
 * oem_console() - Execute the OEM console command
 *
 * @cmd_parameter: Pointer to command parameter
 * @response: Pointer to fastboot response buffer
 */
static void __maybe_unused oem_console(char *cmd_parameter, char *response)
{
	if (cmd_parameter)
		console_in_puts(cmd_parameter);

	if (console_record_isempty())
		fastboot_fail("Empty console", response);
	else
		fastboot_response(FASTBOOT_MULTIRESPONSE_START, response, NULL);
}

/**
 * fastboot_oem_board() - Execute the OEM board command. This is default
 * weak implementation, which may be overwritten in board/ files.
 *
 * @cmd_parameter: Pointer to command parameter
 * @data: Pointer to fastboot input buffer
 * @size: Size of the fastboot input buffer
 * @response: Pointer to fastboot response buffer
 */
void __weak fastboot_oem_board(char *cmd_parameter, void *data, u32 size, char *response)
{
	fastboot_fail("oem board function not defined", response);
}

/**
 * oem_board() - Execute the OEM board command
 *
 * @cmd_parameter: Pointer to command parameter
 * @response: Pointer to fastboot response buffer
 */
static void __maybe_unused oem_board(char *cmd_parameter, char *response)
{
	fastboot_oem_board(cmd_parameter, (void *)fastboot_buf_addr, image_size, response);
}
