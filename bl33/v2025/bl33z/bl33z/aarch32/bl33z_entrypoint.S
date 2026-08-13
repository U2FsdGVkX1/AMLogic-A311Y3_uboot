// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <arch.h>
#include "platform_def.h"
#include "regs.h"

	.global bl33z_entrypoint
	.global bl33z_entry
	.globl	__zero_mem
	.global uart_put

/* Entry trampoline */
bl33z_entrypoint:
	b   bl33z_entry

/* Magic label (64-bit split into two 32-bit words) */
bl33z_label:
	.word 0x4C4D4140      /* low 32bit */
	.word 0x504D5544      /* high 32bit */

/* Main entry */
bl33z_entry:
	/* =========================================================
	 * BL33Z entry context save (AArch32 version)
	 *	High Address
	 *	+------------------+  __STACK_TOP__
	 *	| Context Frame     |  (saved U-Boot state, 64B)
	 *	+------------------+
	 *	| BL33Z Runtime     |
	 *	| Stack             |  (descending stack, 4KB)
	 *	|           sp ->   |
	 *	+------------------+
	 *	Low Address
	 * ========================================================= */
	/* 1. save uboot old stack frame */
	ldr     r9, =PLATFORM_STACK_TOP
	sub     r9, r9, #64             @ reserved 64 bytes frame
	/* 2. save uboot callee-saved registers */
	str     r4,  [r9, #0]
	str     r5,  [r9, #4]
	str     r6,  [r9, #8]
	str     r7,  [r9, #12]
	str     r8,  [r9, #16]
	str     r10, [r9, #20]
	str     r11, [r9, #24]
	/* 3. save uboot lr + sp */
	mov     r12, sp                  @ SP is r13
	str     r12, [r9, #28]           @ old SP
	str     lr,  [r9, #32]           @ lr is r14 return address
	/* 4. save uboot args */
	str     r0,  [r9, #36]
	str     r1,  [r9, #40]
	str     r2,  [r9, #44]
	str     r3,  [r9, #48]
	/* 5. switch to BL33Z private stack */
	sub     sp, r9, #16              @ set new sp

	/*---------------------------------------------*/
	ldr	r0, =__BSS_START__
	ldr	r1, =__BSS_SIZE__
	bl	__zero_mem

#ifdef COHERENT_RAM
	ldr r0, =__COHERENT_RAM_START__
	ldr r1, =__COHERENT_RAM_UNALIGNED_SIZE__
	bl  __zero_mem
#endif

#ifdef BL33Z_RELOCATE_SELF
	ldr	r0, =BL33Z_C4_RUN_ADDR
	ldr	r1, =BL33Z_LOAD_ADDR
	ldr	r2, =BL33Z_LOAD_SIZE
	bl  relocate_self
#endif

	//mov r0, #0x61
	//bl uart_put
	bl  bl33z_main

	/* 6. restore U-Boot context */
	ldr     r9, =PLATFORM_STACK_TOP
	sub     r9, r9, #64
	ldr     r12, [r9,#28]
	ldr     lr,  [r9,#32]
	ldr     r4,  [r9,#0]
	ldr     r5,  [r9,#4]
	ldr     r6,  [r9,#8]
	ldr     r7,  [r9,#12]
	ldr     r8,  [r9,#16]
	ldr     r10, [r9,#20]
	ldr     r11, [r9,#24]
	mov     sp, r12
	ldr		r0,  [r9,#36]
	ldr		r1,  [r9,#40]
	ldr		r2,  [r9,#44]
	ldr		r3,  [r9,#48]

	bx      lr
/* -------------------------------------------------------- */
/* Zero memory routine                                      */
/* r0 = start address                                       */
/* r1 = size in bytes                                       */
/* -------------------------------------------------------- */
	.func	__zero_mem
__zero_mem:
	mov	r4, #0
	mov	r5, #0
	mov	r6, #0
	mov	r7, #0
	mov	r8, #0
	mov	r9, #0
	mov	r10, #0
	mov	r11, #0

	add	r2, r1, r0
1:
	stmia	r0!, {r4-r11}
	cmp	r0, r2
	blt	1b
	bx	lr
	.endfunc

/* -------------------------------------------------------- */
/* uart_put for print 'A' (AArch32 version)                 */
/*     mov r0, #0x41                                        */
/*     bl uart_put                                          */
/* -------------------------------------------------------- */
	.func	uart_put
uart_put:
    cmp     r0, #0x0A
    bne     loop
loop_r:
    ldr     r1, =0xFE07A00C      @ UART status register
1:
    ldr     r2, [r1]
    tst     r2, #(1 << 21)
    bne     1b
    ldr     r1, =0xFE07A000      @ UART data register
    mov     r2, #0x0D
    str     r2, [r1]
loop:
    ldr     r1, =0xFE07A00C
2:
    ldr     r2, [r1]
    tst     r2, #(1 << 21)
    bne     2b
    ldr     r1, =0xFE07A000
    str     r0, [r1]
    bx      lr
	.endfunc
	.section .note.GNU-stack,"",%progbits
