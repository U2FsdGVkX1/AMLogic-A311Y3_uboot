// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <config.h>
#include <command.h>
#include <hexdump.h>
#include <amlogic/libavb/libavb.h>
#include <vsprintf.h>

/*
 * *****************************************************************
 * Solution 1:
 * Use TLV to encode the data.
 * T: 1Byte, L: 1Byte, V: Lbyte
 *
 * Product mode: bool: 0x00/0x01 (T[0x00]L[0x01]V[on]: 0x000101)
 * Unsafe CMD: 8bits: 00011111 (TLV: 0x01011F)
 * Network Type: 8bits: 00000011 (TLV: 0x020103)
 * AVB Recovery: bool: 0x00/0x01 (TLV: 0x030101)
 * AVB Algo: 1Byte: 0x01~06 (TLV: 0x040106)
 * SCS Algo: 1Byte: 01~03 (TLV: 0x050102)
 *
 * bootargs:
 * sec_info=0x00010101011F020103030101040106050102
 */

#define TLV_SHIFT	8
#define SEC_INFO_BYTE(type) ((u64)(type) * TLV_SHIFT)

enum uboot_sec_info_type {
	SEC_INFO_PRODUCT_MODE = 0,   // get the production mode type: bool: on(1)/off(0)
	SEC_INFO_UNSAFE_CMD = 1,     // 8 bitmap: UBOOT_UNSAFE_CMD_TYPE
	SEC_INFO_NETWORK_TYPE = 2,   // 8 bitmap: UBOOT_NET_BOOT_TYPE
	SEC_INFO_AVB_RECOVERY = 3,   // bool: on(1)/off(0)
	SEC_INFO_AVB_ALGO = 4,       // 1Byte: UBOOT_AVB_ALGO_TYPE
	SEC_INFO_SCS_ALGO = 5,       // 1Byte: UBOOT_SCS_ALGO_TYPE
	SEC_INFO_MAX
};

enum uboot_scs_algo_type {
	SCS_ALGO_NONE = 0,
	SCS_ALGO_RSA_2048,
	SCS_ALGO_RSA_4096,
	SCS_ALGO_RSA_8192,
};

enum uboot_net_boot_type {
	NET = 0,
	CMD_NFS,
	NET_BOOT_NONE,
};

enum uboot_unsafe_cmd_type {
	CMD_BOOTI = 0,
	CMD_BOOTD,
	CMD_BOOTZ,
	CMD_MEMORY,
	CMD_FAT,
	CMD_MAX
};

static u8 sec_info_buf[SEC_INFO_MAX * 3];
int pubrsa_keysize;

__weak uint32_t get_vbmeta_algorithm_type(void)
{
#ifdef CONFIG_CMD_BOOTCTOL_AVB
	AvbSlotVerifyData * out_data = NULL;
	AvbVBMetaImageHeader toplevel_vbmeta;
	int nret;

	nret = avb_verify_no_partitions(&out_data);
	if (nret != AVB_SLOT_VERIFY_RESULT_OK &&
		nret != AVB_SLOT_VERIFY_RESULT_ERROR_VERIFICATION &&
		nret != AVB_SLOT_VERIFY_RESULT_ERROR_ROLLBACK_INDEX &&
		nret != AVB_SLOT_VERIFY_RESULT_ERROR_PUBLIC_KEY_REJECTED) {
		if (out_data) {
			avb_vbmeta_image_header_to_host_byte_order((const AvbVBMetaImageHeader *)
					out_data->vbmeta_images[0].vbmeta_data, &toplevel_vbmeta);
			avb_slot_verify_data_free(out_data);
			return toplevel_vbmeta.algorithm_type;
		}
	}
	return AVB_ALGORITHM_TYPE_NONE;
#else
	return AVB_ALGORITHM_TYPE_NONE;
#endif
}

static u8 *tlv_put(u8 *p, u8 tag, u8 len, const u8 *val)
{
	*p++ = tag;
	*p++ = len;
	memcpy(p, val, len);
	return p + len;
}

void set_secinfo_to_bootargs(u8 *secinfo_buf, int sec_len)
{
	char buf[SEC_INFO_MAX * 6 + 1] = {0};
	char cmdbuf[128];

	bin2hex(buf, secinfo_buf, sec_len);
	sprintf(cmdbuf, "setenv bootargs ${bootargs} sec_info=0x%s ", buf);
	run_command(cmdbuf, 0);
}

static int do_secinfo(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	u8 sec_v = 0;
#ifdef CONFIG_AVB2
	const char *env_data = NULL;
	long data = 0;
#endif

	// the SEC_INFO_PRODUCT_MODE (bool)
	sec_v = CONFIG_IS_ENABLED(AML_PRODUCT_MODE);
	tlv_put(&sec_info_buf[0], SEC_INFO_PRODUCT_MODE, 1, &sec_v);

	// the SEC_INFO_UNSAFE_CMD (bitmap)
	sec_v = CONFIG_IS_ENABLED(CMD_BOOTI) << CMD_BOOTI;
	sec_v |= CONFIG_IS_ENABLED(CMD_BOOTD) << CMD_BOOTD;
	sec_v |= CONFIG_IS_ENABLED(CMD_BOOTZ) << CMD_BOOTZ;
	sec_v |= CONFIG_IS_ENABLED(CMD_MEMORY) << CMD_MEMORY;
	sec_v |= CONFIG_IS_ENABLED(CMD_FAT) << CMD_FAT;
	tlv_put(&sec_info_buf[3], SEC_INFO_UNSAFE_CMD, 1, &sec_v);

	// the SEC_INFO_NETWORK_TYPE (bitmap)
	sec_v = CONFIG_IS_ENABLED(NET) << NET;
	sec_v |= CONFIG_IS_ENABLED(CMD_NFS) << CMD_NFS;
	tlv_put(&sec_info_buf[6], SEC_INFO_NETWORK_TYPE, 1, &sec_v);

	// the SEC_INFO_AVB_RECOVERY (bool)
	sec_v = 0;
#ifdef CONFIG_AVB2
	env_data = env_get("slot-suffixes");
	if (env_data) {
		data = simple_strtol(env_data, NULL, 0);
		if (data == -1)
			sec_v = 1;		// if is -1 have the recovery
	}
#endif
	tlv_put(&sec_info_buf[9], SEC_INFO_AVB_RECOVERY, 1, &sec_v);

	// the SEC_INFO_AVB_ALGO (index)
	sec_v = get_vbmeta_algorithm_type();
	tlv_put(&sec_info_buf[12], SEC_INFO_AVB_ALGO, 1, &sec_v);

	// the SEC_INFO_SCS_ALGO (index)
	sec_v = SCS_ALGO_NONE;
	if (get_pubrsa_keysize() == 64)
		sec_v = (uint64_t)SCS_ALGO_RSA_4096;
	else if (get_pubrsa_keysize() == 128)
		sec_v = (uint64_t)SCS_ALGO_RSA_8192;
	else if (get_pubrsa_keysize() == 32)
		sec_v = (uint64_t)SCS_ALGO_RSA_2048;
	else
		printf("no right pubkey for SEC_INFO_SCS_ALGO\n");

	tlv_put(&sec_info_buf[15], SEC_INFO_SCS_ALGO, 1, &sec_v);
	set_secinfo_to_bootargs(sec_info_buf, SEC_INFO_MAX * 3);

	return 0;
}

U_BOOT_CMD(secinfo,	1,		0,	do_secinfo,
	"generate bootloader sec_info",
	""
);
