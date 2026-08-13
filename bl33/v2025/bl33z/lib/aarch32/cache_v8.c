// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

/*
 * Copyright (C) 2014-2018 Amlogic, Inc. All rights reserved.
 *
 * All information contained herein is Amlogic confidential.
 *
 * This software is provided to you pursuant to Software License Agreement
 * (SLA) with Amlogic Inc ("Amlogic"). This software may be used
 * only in accordance with the terms of this agreement.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification is strictly prohibited without prior written permission from
 * Amlogic.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <mmu.h>
#include <cache_v8.h>
#include <serial.h>
#include <io.h>
#include "ram_compress.h"
#include "platform_def.h"
#include "regs.h"

/* Asm functions from cache_v7_asm.S */
void v7_flush_dcache_all(void);
void v7_invalidate_dcache_all(void);

u32 bl2e_ttbr0 = 0, bl2e_dacr = 0, bl2e_sctlr = 0;

//porting from arch/arm/lib/cache-cp15.c
static inline unsigned int get_cr(void)
{
	unsigned int val;

	asm volatile("mrc p15, 0, %0, c1, c0, 0	@ get CR" : "=r" (val)
								  :
								  : "cc");
	return val;
}

static inline void set_cr(unsigned int val)
{
	asm volatile("mcr p15, 0, %0, c1, c0, 0	@ set CR" :
								  : "r" (val)
								  : "cc");
	asm volatile("isb");
}

u32 __asm_get_sctlr(void)
{
	return get_cr();
}

void __asm_set_sctlr(u32 val)
{
	set_cr(val);
}

//porting from arch/arm/cpu/armv7/cache_v7.c
/* Invalidate entire I-cache and branch predictor array */
void invalidate_icache_all(void)
{
	/*
	 * Invalidate all instruction caches to PoU.
	 * Also flushes branch target cache.
	 */
	asm volatile ("mcr p15, 0, %0, c7, c5, 0" : : "r" (0));

	/* Invalidate entire branch predictor array */
	asm volatile ("mcr p15, 0, %0, c7, c5, 6" : : "r" (0));

	/* Full system DSB - make sure that the invalidation is complete */
	asm volatile("dsb");

	/* ISB - make sure the instruction stream sees it */
	asm volatile("isb");
}

/* Invalidate TLB */
static void invalidate_tlb_all(void)
{
	/* Invalidate entire unified TLB */
	asm volatile ("mcr p15, 0, %0, c8, c7, 0" : : "r" (0));
	/* Invalidate entire data TLB */
	asm volatile ("mcr p15, 0, %0, c8, c6, 0" : : "r" (0));
	/* Invalidate entire instruction TLB */
	asm volatile ("mcr p15, 0, %0, c8, c5, 0" : : "r" (0));
	/* Full system DSB - make sure that the invalidation is complete */
	asm volatile("dsb");
	/* Full system ISB - make sure the instruction stream sees it */
	asm volatile("isb");
}

void get_bl2e_mmu_state(void)
{
	bl2e_ttbr0 = __asm_get_ttbr0();
	bl2e_dacr  = __asm_get_dacr();
	bl2e_sctlr = get_cr();

	serial_puts("Get bl2e mmu table. ttbr0_EL2:0x");
	serial_put_hex(bl2e_ttbr0, 32);
	serial_puts(", dacr_EL2:0x");
	serial_put_hex(bl2e_dacr, 32);
	serial_puts(", sctlr_EL2:0x");
	serial_put_hex(bl2e_sctlr, 32);
	serial_puts("\n");
}

void set_bl2e_mmu_state(void)
{
	__asm_set_ttbr0(bl2e_ttbr0);
	__asm_set_dacr(bl2e_dacr);
	set_cr(bl2e_sctlr);
	serial_puts("Overwrite bl2e mmu table for bl33 board_f.");
}

void set_pgtable_section(u32 *pgtable, u32 index, u32 phys, u32 flags)
{
	u32 desc;

	desc = (phys & 0xFFF00000)   // section base
			| (2 << 0)              // bit[1:0] = 10 → section
			| (1 << 10)             // AP[2] (full access)
			| (3 << 10);            // AP[1:0] = 11 (RW)

	desc |= flags;               // TEX/C/B 等

	pgtable[index] = desc;
}

/*
 * Section Mapping (1MB) + Identity Mapping
 *      VA == PA, using ARMv7 first-level section descriptors.
 *
 * Each entry maps one 1MB region:
 *      section[i] -> physical base (i << 20)
 *
 * 4GB is initially mapped as Device memory,
 * then DDR is overwritten as Normal cacheable memory.
 */
void mmu_setup(unsigned long ddr_size)
{
	u32 *page_table = (u32 *)CONFIG_AML_MMU_ADDR;
	u32 i, reg;
	unsigned long nTotal;

	/* 1. clean pagetable */
	for (i = 0; i < PAGE_ENTRIES; i++) {
		page_table[i] = 0;
	}

	/* 2. aarch32 full ddr is 4GB: DEVICE ( avoid MMIO crash) */
	for (i = 0; i < PAGE_ENTRIES; i++) {
		page_table[i] =
			(i << SECTION_SHIFT) |
			PMD_TYPE_SECT |
			PMD_SECT_AF |
			MT_DEVICE;
	}

	/* 3. DDR 0~512MB change to NORMAL memory */
	nTotal = readl(REG_MDUMP_CPUBOOT_STATUS) & (AMLOGIC_USERAM_MASK & ~RAMDUMP_STICKY_DMA_MASK);
	nTotal = ((nTotal - 1) / 64 + 1) * 64;	/* ddr size must big than 64MB */
	nTotal = nTotal ? nTotal : (ddr_size >> 20);
	serial_puts("mmu: DDR size is ");
	serial_put_dec(nTotal);
	serial_puts("MB\n");
	for (i = 0; i < (nTotal / SECTION_SIZE_MB); i++) {
		page_table[i] =
			(i << SECTION_SHIFT) |
			PMD_TYPE_SECT |
			PMD_SECT_AF |
			MT_NORMAL;
	}

	/* 4. TTBCR = 0 (disable LPAE / full TTBR0 space) */
	asm volatile("mcr p15, 0, %0, c2, c0, 2"
					: : "r"(0) : "memory");

	/* 5. TTBR0 */
	reg = (u32)page_table;
	/* cache policy（WB/WA） */
	reg &= TTBR0_BASE_ADDR_MASK;
	reg |= TTBR0_RGN_WB;
	reg |= TTBR0_IRGN_WB;

	asm volatile("mcr p15, 0, %0, c2, c0, 0"
					: : "r"(reg) : "memory");

	/* 6. DACR = all domains client */
	asm volatile("mcr p15, 0, %0, c3, c0, 0"
					: : "r"(0x55555555));
	asm volatile("dsb");
	asm volatile("isb");

	/* 7. ENABLE MMU */
	reg = get_cr();
	reg |= (1 << 0);   // M bit
	asm volatile("mcr p15, 0, %0, c1, c0, 0"
					: : "r"(reg) : "memory");
	asm volatile("dsb");
	asm volatile("isb");

	serial_puts("mmu: enabled OK\n");
}

void enable_dcache(unsigned long ddr_size)
{
	uint32_t reg;

	/* already enabled */
	if (get_cr() & CR_C)
		return;

	/*
	 * D-cache requires MMU enabled first.
	 * Build 1MB section identity mapping if MMU is still off.
	 */
	if (!(get_cr() & CR_M)) {
		/*
		 * Optional but recommended:
		 * invalidate TLB before installing new translation base
		 */
		invalidate_tlb_all();

		/*
		 * Setup TTBR0 / DACR / enable CR_M
		 */
		mmu_setup(ddr_size);
	}

	reg = get_cr();
	reg |= CR_C;
	set_cr(reg);
}

void enable_icache(void)
{
	uint32_t reg;

	invalidate_icache_all();

	reg = get_cr();
	set_cr(reg | CR_I);
}

void disable_dcache(void)
{
	uint32_t reg;

	reg = get_cr();
	if (!(reg & CR_C))
		return;
	set_cr(reg & ~(CR_C | CR_M));

	v7_flush_dcache_all();
	invalidate_tlb_all();
}

void disable_icache(void)
{
	uint32_t reg;

	/* 1. Invalidate I-cache first */
	invalidate_icache_all();

	/* 2. Disable I-cache */
	reg = get_cr();
	reg &= ~CR_I;
	set_cr(reg);

	/* 3. Ensure new state takes effect */
	asm volatile("isb");
}

// C4_aarch32 porting from uboot/v2025/arch/arm/lib/cache-cp15.c
void enable_caches(unsigned long ddr_size)
{
	enable_icache();
	enable_dcache(ddr_size);
}

void disable_caches(void)
{
	disable_icache();
	disable_dcache();
}
