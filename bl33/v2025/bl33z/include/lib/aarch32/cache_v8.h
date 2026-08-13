/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */
#include <mmu.h>

#ifndef __BL2_CACHE_V8_H_
#define __BL2_CACHE_V8_H_

void __asm_flush_dcache_all(void);
void __asm_invalidate_dcache_all(void);
void __asm_invalidate_icache_all(void);
void __asm_flush_dcache_range(u32, u32);
void __asm_invalidate_tlb_all(void);

u32 __asm_get_ttbr0(void);
u32 __asm_get_dacr(void);
u32 __asm_get_sctlr(void);
void __asm_set_ttbr0(u32);
void __asm_set_dacr(u32);
void __asm_set_sctlr(u32);

/* Asm functions from cache_v7_asm.S */
void v7_flush_dcache_all(void);
void v7_invalidate_dcache_all(void);

void enable_icache(void);
void enable_dcache(unsigned long ddr_size);
void disable_icache(void);
void disable_dcache(void);
void enable_caches(unsigned long ddr_size);
void disable_caches(void);
void get_bl2e_mmu_state(void);
void set_bl2e_mmu_state(void);
#endif /*__BL2_CACHE_V8_H_*/
