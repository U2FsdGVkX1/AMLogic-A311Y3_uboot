// SPDX-License-Identifier: (GPL-2.0+ OR MIT)

/*
 * This file provides low-level assembly routines for cache and MMU control
 * on ARM architectures, covering both AArch64 (ARMv8-A) and AArch32 (ARMv7-A).
 *
 * ================================================================
 * AArch64 (ARMv8-A, EL2)
 * ================================================================
 * The original implementation targets AArch64 running at EL2 and
 * relies on system registers and instructions such as:
 *
 *   - System registers:
 *       SCTLR_EL2, TTBR0_EL2, TCR_EL2, MAIR_EL2
 *
 *   - Instructions:
 *       MRS / MSR     (system register access)
 *       TLBI          (TLB invalidate)
 *       DC / IC       (data/instruction cache operations)
 *
 *   - Features:
 *       * Full MMU configuration via TTBR + TCR + MAIR
 *       * Fine-grained memory attribute control
 *       * Cache maintenance by level / set-way / address range
 *       * Exception level awareness (EL1 / EL2 / EL3)
 *
 * This version is typically used in ARM Trusted Firmware or bootloader
 * stages (e.g., BL2/BL33) running in AArch64 mode.
 *
 *
 * ================================================================
 * AArch32 (ARMv7-A compatible version)
 * ================================================================
 * The adapted version replaces AArch64 system register access with
 * ARMv7 CP15 interface:
 *
 *   - System registers (CP15):
 *       SCTLR   -> c1
 *       TTBR0   -> c2
 *       DACR    -> c3
 *       TLB ops -> c8
 *       Cache ops -> c7
 *
 *   - Instructions:
 *       MRC / MCR     (CP15 register access)
 *
 *   - Features:
 *       * MMU configuration via TTBR0 + DACR
 *       * No TCR/MAIR (memory attributes encoded in descriptors)
 *       * Cache maintenance via set/way or VA operations
 *       * Single privilege model (no EL abstraction)
 *
 *   - Limitations vs AArch64:
 *       * No MAIR (uses TEX/C/B bits in page tables)
 *       * No TCR (address size & granularity fixed by design)
 *       * Less flexible cache and MMU configuration
 *
 *
 * ================================================================
 * Key Differences (AArch64 vs AArch32)
 * ================================================================
 *
 *   MMU Configuration:
 *     AArch64 : TTBR + TCR + MAIR
 *     AArch32 : TTBR0 + DACR (attributes in descriptors)
 *
 *   Register Access:
 *     AArch64 : MRS / MSR
 *     AArch32 : MRC / MCR (CP15)
 *
 *   Cache/TLB Ops:
 *     AArch64 : TLBI / DC / IC instructions
 *     AArch32 : CP15 c7/c8 operations
 *
 *   Exception Model:
 *     AArch64 : EL0–EL3 hierarchy
 *     AArch32 : Privileged modes (SVC, HYP, etc.)
 *
 *   Descriptor Format:
 *     AArch64 : 64-bit descriptors
 *     AArch32 : 32-bit section/page descriptors (or LPAE 64-bit)
 *
 *
 * ================================================================
 * Notes
 * ================================================================
 *
 * - The AArch64 and AArch32 implementations are NOT compatible and
 *   must be compiled separately depending on target architecture.
 *
 * - When porting between them, special care must be taken for:
 *       * Page table format
 *       * Cache maintenance semantics
 *       * MMU enable sequence
 *
 * - This file is intended for early boot stages where cache/MMU
 *   must be initialized before C runtime is fully available.
 *
 */
#include <arch.h>

    .global __asm_get_ttbr0
    .global __asm_set_ttbr0
	.global __asm_get_dacr
    .global __asm_set_dacr
    .global __asm_invalidate_tlb_all
    .global __asm_invalidate_icache_all
    .global __asm_flush_dcache_all
    .global __asm_invalidate_dcache_all
    .global __asm_flush_dcache_range

/* ===================================== */
/* TTBR0 */
/* ===================================== */
__asm_get_ttbr0:
    mrc p15, 0, r0, c2, c0, 0
    bx lr

__asm_set_ttbr0:
    mcr p15, 0, r0, c2, c0, 0
    dsb
    isb
    bx lr

/* ===================================== */
/* DACR */
/* ===================================== */
__asm_get_dacr:
	mrc p15, 0, r0, c3, c0, 0
	bx lr

__asm_set_dacr:
    mcr p15, 0, r0, c3, c0, 0
    bx lr

/* ===================================== */
/* TLB invalidate */
/* ===================================== */
__asm_invalidate_tlb_all:
    mov r0, #0
    mcr p15, 0, r0, c8, c7, 0   // invalidate unified TLB
    dsb
    isb
    bx lr

/* ===================================== */
/* I-Cache invalidate */
/* ===================================== */
__asm_invalidate_icache_all:
    mov r0, #0
    mcr p15, 0, r0, c7, c5, 0   // invalidate I-cache all
    dsb
    isb
    bx lr

/* ===================================== */
/* D-Cache clean+invalidate all */
/* ===================================== */
__asm_flush_dcache_all:
    push {r0-r1, lr}

    dsb

    /* Clean + invalidate entire data cache */
    mov r0, #0
    mcr p15, 0, r0, c7, c14, 0

    /* Ensure completion */
    dsb
    isb

    pop {r0-r1, pc}

/* ===================================== */
/* D-Cache invalidate all */
/* ===================================== */
__asm_invalidate_dcache_all:
    push {r0-r1, r2, lr}

    dsb

    /* 1. disable D-cache first (critical) */
    mrc p15, 0, r0, c1, c0, 0      @ SCTLR
    bic r0, r0, #(1 << 2)         @ clear C bit (D-cache disable)
    mcr p15, 0, r0, c1, c0, 0
    isb

    dsb

    /* 2. invalidate entire D-cache (safe now) */
    mov r0, #0
    mcr p15, 0, r0, c7, c6, 0      @ D-cache invalidate all (safe only when OFF)

    dsb
    isb

    pop {r0-r1, r2, pc}

/* ===================================== */
/* flush range */
/* ===================================== */
__asm_flush_dcache_range:
    push {r2-r4, lr}

    mrc p15, 0, r2, c0, c0, 1   // CTR
    lsr r2, r2, #16
    and r2, r2, #0xf

    mov r3, #4
    lsl r3, r3, r2              // line size

    sub r2, r3, #1
    bic r0, r0, r2

1:
    mcr p15, 0, r0, c7, c14, 1  // clean+invalidate by VA
    add r0, r0, r3
    cmp r0, r1
    blo 1b

    dsb
    isb
    pop {r2-r4, pc}

/*
 *	v7_flush_dcache_all()
 *
 *	Flush the whole D-cache.
 *
 *	Corrupted registers: r0-r6, r9-r10
 *
 *	- mm    - mm_struct describing address space
 */
.global __v7_flush_dcache_all
__v7_flush_dcache_all:
	dmb					@ ensure ordering with previous memory accesses
	mrc	p15, 1, r0, c0, c0, 1		@ read clidr
	mov	r3, r0, lsr #23			@ move LoC into position
	ands	r3, r3, #7 << 1			@ extract LoC*2 from clidr
	beq	finished			@ if loc is 0, then no need to clean
start_flush_levels:
	mov	r10, #0				@ start clean at cache level 0
flush_levels:
	add	r2, r10, r10, lsr #1		@ work out 3x current cache level
	mov	r1, r0, lsr r2			@ extract cache type bits from clidr
	and	r1, r1, #7			@ mask of the bits for current cache only
	cmp	r1, #2				@ see what cache we have at this level
	blt	skip				@ skip if no cache, or just i-cache
	mcr	p15, 2, r10, c0, c0, 0		@ select current cache level in cssr
	isb					@ isb to sych the new cssr&csidr
	mrc	p15, 1, r1, c0, c0, 0		@ read the new csidr
	and	r2, r1, #7			@ extract the length of the cache lines
	add	r2, r2, #4			@ add 4 (line length offset)
	movw	r4, #0x3ff
	ands	r4, r4, r1, lsr #3		@ find maximum number on the way size
	clz	r5, r4				@ find bit position of way size increment
	movw	r6, #0x7fff
	and	r1, r6, r1, lsr #13		@ extract max number of the index size
	mov	r6, #1
	movne	r4, r4, lsl r5			@ # of ways shifted into bits [31:...]
	movne	r6, r6, lsl r5			@ 1 shifted left by same amount
loop1:
	mov	r9, r1				@ create working copy of max index
loop2:
	mov	r5, r9, lsl r2			@ factor set number into r5
	orr	r5, r5, r4			@ factor way number into r5
	orr	r5, r5, r10			@ factor cache level into r5
	mcr	p15, 0, r5, c7, c14, 2		@ clean & invalidate by set/way
	subs	r9, r9, #1			@ decrement the index
	bge	loop2
	subs	r4, r4, r6			@ decrement the way
	bcs	loop1
skip:
	add	r10, r10, #2			@ increment cache number
	cmp	r3, r10
	dsb
	bgt	flush_levels
finished:
	mov	r10, #0				@ switch back to cache level 0
	mcr	p15, 2, r10, c0, c0, 0		@ select current cache level in cssr
	dsb	st
	isb
	bx	lr

.global v7_flush_dcache_all
v7_flush_dcache_all:
	stmfd	sp!, {r4-r6, r9-r10, lr}
	bl	__v7_flush_dcache_all
	ldmfd	sp!, {r4-r6, r9-r10, lr}
	bx	lr

/*
 *	v7_invalidate_dcache_all()
 *
 *	Invalidate the whole D-cache.
 *
 *	Corrupted registers: r0-r7, r9-r11 (r6 only in Thumb mode)
 *
 *	Note: copied from __v7_flush_dcache_all above with
 *	mcr     p15, 0, r11, c7, c14, 2
 *	Replaced with:
 *	mcr     p15, 0, r11, c7, c6, 2
 */
.global __v7_invalidate_dcache_all
__v7_invalidate_dcache_all:
	dmb					@ ensure ordering with previous memory accesses
	mrc	p15, 1, r0, c0, c0, 1		@ read clidr
	mov	r3, r0, lsr #23			@ move LoC into position
	ands	r3, r3, #7 << 1			@ extract LoC*2 from clidr
	beq	inval_finished			@ if loc is 0, then no need to clean
	mov	r10, #0				@ start clean at cache level 0
inval_levels:
	add	r2, r10, r10, lsr #1		@ work out 3x current cache level
	mov	r1, r0, lsr r2			@ extract cache type bits from clidr
	and	r1, r1, #7			@ mask of the bits for current cache only
	cmp	r1, #2				@ see what cache we have at this level
	blt	inval_skip			@ skip if no cache, or just i-cache
	mcr	p15, 2, r10, c0, c0, 0		@ select current cache level in cssr
	isb					@ isb to sych the new cssr&csidr
	mrc	p15, 1, r1, c0, c0, 0		@ read the new csidr
	and	r2, r1, #7			@ extract the length of the cache lines
	add	r2, r2, #4			@ add 4 (line length offset)
	movw	r4, #0x3ff
	ands	r4, r4, r1, lsr #3		@ find maximum number on the way size
	clz	r5, r4				@ find bit position of way size increment
	movw	r6, #0x7fff
	and	r1, r6, r1, lsr #13		@ extract max number of the index size
	mov	r6, #1
	movne	r4, r4, lsl r5			@ # of ways shifted into bits [31:...]
	movne	r6, r6, lsl r5			@ 1 shifted left by same amount
inval_loop1:
	mov	r9, r1				@ create working copy of max index
inval_loop2:
	mov	r5, r9, lsl r2			@ factor set number into r5
	orr	r5, r5, r4			@ factor way number into r5
	orr	r5, r5, r10			@ factor cache level into r5
	mcr	p15, 0, r5, c7, c6, 2		@ invalidate by set/way
	subs	r9, r9, #1			@ decrement the index
	bge	inval_loop2
	subs	r4, r4, r6			@ decrement the way
	bcs	inval_loop1
inval_skip:
	add	r10, r10, #2			@ increment cache number
	cmp	r3, r10
	dsb
	bgt	inval_levels
inval_finished:
	mov	r10, #0					@ switch back to cache level 0
	mcr	p15, 2, r10, c0, c0, 0		@ select current cache level in cssr
	dsb	st
	isb
	bx	lr

.global v7_invalidate_dcache_all
v7_invalidate_dcache_all:
	stmfd	sp!, {r4-r6, r9-r10, lr}
	bl	__v7_invalidate_dcache_all
	ldmfd	sp!, {r4-r6, r9-r10, lr}
	bx	lr

	.section .note.GNU-stack,"",%progbits
