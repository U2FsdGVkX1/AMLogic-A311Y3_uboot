/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef _PLAT_STORAGE_H_
#define _PLAT_STORAGE_H_

#define COMPACT_BOOTLOADER			0
#define DISCRETE_BOOTLOADER			1
#define ADVANCE_BOOTLOADER			2

//#define BL2E_STORAGE_PARAM_SIZE		(0x80)
//#define BOOT_FIRST_BLOB_SIZE        (166*1024)
//#define BOOT_FILLER_SIZE            (4*1024)
//#define BOOT_RESERVED_SIZE          (4*1024)
//#define BOOT_RANDOM_NONCE           (16)
//#define BOOT_BL2E_SIZE              (66672) //74864-8K
//#define BOOT_EBL2E_SIZE             (BOOT_FILLER_SIZE + BOOT_RESERVED_SIZE + BOOT_BL2E_SIZE)
//#define BOOT_BL2X_SIZE              (66672)
#define MAX_BOOT_AREA_ENTRIES		(8)
/* bl2 core address base */
#define BL2_CORE_BASE_OFFSET_EMMC	(0x200)
/* boot area entry index */
#define BOOT_AREA_BB1ST             (0)
/* filler and reserved are considered part of the bl2E in storage view */
#define BOOT_AREA_BL2E              (1)
#define BOOT_AREA_BL2X              (2)
#define BOOT_AREA_DDRFIP            (3)
#define BOOT_AREA_DEVFIP            (4)
#define BOOT_AREA_INVALID           (MAX_BOOT_AREA_ENTRIES)

#define BOOTLOADER_MODE_EMMC		COMPACT_BOOTLOADER
#define BOOTLOADER_MODE_NAND		ADVANCE_BOOTLOADER
#define BOOTLOADER_MODE_SNAND		ADVANCE_BOOTLOADER
#define BOOTLOADER_MODE_SNOR		ADVANCE_BOOTLOADER

#ifdef ADVANCE_BOOTLOADER
#define BOOTLOADER_MODE_ADVANCE_INIT		1
#else
#define BOOTLOADER_MODE_ADVANCE_INIT		0
#endif
#define BOOTINFO_PROGRAMMER_SUPPORT            1

#define SPINOR_HAS_BOOTINFO    1
/*
 * TODO: This is the attribute of nand, it will be moved to the
 * plat file of nand in the future.
 */
#ifdef BOOTLOADER_MODE_NAND
#define NAND_ECC_ONLY_BCH8_1K
#endif

#define CONFIG_DDR_PARAMETER_SUPPORT 1

#ifndef CLKCTRL_NAND_CLK_CTRL
#define CLKCTRL_NAND_CLK_CTRL	CLKCTRL_SD_EMMC_CLK_CTRL1
#endif

#ifdef BL2_CONFIG
/*  store param for fastboot */
#define	STORE_PARAM_SRAM_ADDR	(0xF3C23800)
#define	STORE_PARAM_SRAM_SIZE	(0x800)
#endif

#endif /* _PLAT_STORAGE_H_ */
