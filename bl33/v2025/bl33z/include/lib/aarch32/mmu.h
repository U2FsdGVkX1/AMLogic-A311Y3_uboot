/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef _ASM_ARMV8_MMU_H_
#define _ASM_ARMV8_MMU_H_
 /***************************************************************
  * AArch32 (ARMv7-A) MMU / Page Table Definitions
  *
  * Description:
  * - Uses 1MB section mapping
  * - Typical usage: U-Boot / BL33 identity mapping
  * - Applicable to Cortex-A7 / Cortex-A9 / Cortex-A53 (AArch32 mode)
  ***************************************************************/

#define BIT(nr)			(1 << nr)
typedef unsigned int  u32;

#ifdef __ASSEMBLY__
#define _AC(X, Y)	X
#else
#define _AC(X, Y)	(X##Y)
#endif
#define UL(x)		_AC(x, UL)

/* ============================================================
 * Page Table
 * ============================================================ */
/* 4GB address space / 1MB section = 4096 entries */
#define PAGE_ENTRIES   (0x100000000ULL >> SECTION_SHIFT)

/* page table size (16KB or 64KB aligned depending platform) */
#define PGTABLE_SIZE   (0x10000)

/* ============================================================
 * Section mapping (1MB granularity)
 * ============================================================ */
#define SECTION_SHIFT   20
#define SECTION_SIZE    (1UL << SECTION_SHIFT)
#define SECTION_SIZE_MB (SECTION_SIZE >> 20)

/* ============================================================
 * Section descriptor format (ARMv7-A)
 * ============================================================
 * [1:0]   = descriptor type (10 = section)
 * [10]    = APX / AF (access flag)
 * [31:20] = physical base address
 * ============================================================ */
#define PMD_TYPE_SECT   (0x2)
#define PMD_SECT_AF     BIT(10)

/* ============================================================
 * Memory Attribute (TEX / C / B)
 * ============================================================
 * ARMv7-A section attributes:
 *
 * TEX C B meaning
 * 000 1 1  Write-back, write-allocate (Normal memory)
 * 000 0 0  Strongly ordered / Device
 * ============================================================ */
/* Device memory (non-cacheable) */
#define MT_DEVICE   (0 << 12 | 0 << 3 | 0 << 2)

/* Normal cacheable memory (WBWA) */
#define MT_NORMAL   (0 << 12 | 1 << 3 | 1 << 2)

/* ============================================================
 * TTBR0 configuration (ARMv7-A)
 * ============================================================
 * TTBR0 bits:
 *  - Base address [31:14]
 *  - IRGN / RGN cache policy
 * ============================================================ */
#define TTBR0_BASE_ADDR_MASK   0xFFFFC000

/* Outer cache policy */
#define TTBR0_RGN_NC    (0 << 3)
#define TTBR0_RGN_WBWA  (1 << 3)
#define TTBR0_RGN_WT    (2 << 3)
#define TTBR0_RGN_WB    (3 << 3)

/* Inner cache policy (IRGN[1:0] + TTBR0[6]) */
#define TTBR0_IRGN_NC   ((0 << 0) | (0 << 6))
#define TTBR0_IRGN_WBWA ((0 << 0) | (1 << 6))
#define TTBR0_IRGN_WT   ((1 << 0) | (0 << 6))
#define TTBR0_IRGN_WB   ((1 << 0) | (1 << 6))

/* ============================================================
 * Cache policy (legacy U-Boot style)
 * ============================================================ */
enum dcache_option {
	DCACHE_OFF          = (0 << 2),
	DCACHE_WRITETHROUGH = (3 << 2),
	DCACHE_WRITEBACK    = (4 << 2),
	DCACHE_WRITEALLOC   = (4 << 2),
};

#define DCACHE_DEFAULT_OPTION   DCACHE_WRITEBACK

/* ============================================================
 * SCTLR (CP15 System Control Register)
 * ============================================================ */
#define CR_M     BIT(0)    /* MMU enable */
#define CR_A     BIT(1)    /* Alignment check */
#define CR_C     BIT(2)    /* Data cache enable */
#define CR_W     BIT(3)    /* Write buffer enable */
#define CR_Z     BIT(11)   /* Branch prediction enable */
#define CR_I     BIT(12)   /* Instruction cache enable */
#define CR_V     BIT(13)   /* Vector base high/low */
#define CR_EE    BIT(25)   /* Endianness (0=little,1=big) */

#endif /* _ASM_ARMV8_MMU_H_ */
