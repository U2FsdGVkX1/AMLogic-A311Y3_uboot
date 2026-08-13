/* SPDX-License-Identifier: GPL-2.0+ */

#include <config.h>
#include <linux/linkage.h>
#include <linux/sizes.h>
#include <asm/system.h>

#if CONFIG_IS_ENABLED(SYS_THUMB_BUILD)
#define ARM(x...)
#define THUMB(x...)	x
#else
#define ARM(x...)	x
#define THUMB(x...)
#endif

/*
 *	v7_flush_dcache_all()
 *
 *	Flush the whole D-cache.
 *
 *	Corrupted registers: r0-r6, r9-r10
 *
 *	- mm    - mm_struct describing address space
 */
ENTRY(__v7_flush_dcache_all)
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
ENDPROC(__v7_flush_dcache_all)

ENTRY(v7_flush_dcache_all)
	stmfd	sp!, {r4-r6, r9-r10, lr}
	bl	__v7_flush_dcache_all
	ldmfd	sp!, {r4-r6, r9-r10, lr}
	bx	lr
ENDPROC(v7_flush_dcache_all)

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
ENTRY(__v7_invalidate_dcache_all)
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
	mov	r10, #0				@ swith back to cache level 0
	mcr	p15, 2, r10, c0, c0, 0		@ select current cache level in cssr
	dsb	st
	isb
	bx	lr
ENDPROC(__v7_invalidate_dcache_all)

ENTRY(v7_invalidate_dcache_all)
	stmfd	sp!, {r4-r6, r9-r10, lr}
	bl	__v7_invalidate_dcache_all
	ldmfd	sp!, {r4-r6, r9-r10, lr}
	bx	lr
ENDPROC(v7_invalidate_dcache_all)
