/* SPDX-License-Identifier: GPL-2.0-only */

#include "platform_def.h"
#include "regs.h"

OUTPUT_FORMAT("elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(bl33z_entrypoint)

MEMORY {
	RAM (rwx) : ORIGIN = BL33Z_LOAD_ADDR, LENGTH = BL33Z_LOAD_SIZE
	STACK (rwx) : ORIGIN = PLATFORM_STACK_BOTTOM, LENGTH = PLATFORM_STACK_SIZE
}

PHDRS {
	text PT_LOAD FLAGS(5);  /* R + X = 101b */
	data PT_LOAD FLAGS(6);  /* R + W = 110b */
}

SECTIONS
{
	. = BL33Z_LOAD_ADDR;
	. = ALIGN(16);

	.vectors : {
		*(.vectors*)
	} >RAM

	.text : {
		__TEXT_START__ = .;
		*bl33z_entrypoint.o(.text*)
		*(.text*)
		*(.rodata*)
		__TEXT_END__ = .;
	} > RAM :text

	.data : {
		__DATA_START__ = .;
		*(.data*)
		__DATA_END__ = .;
	} >RAM :data

	MARK_ADDR = .;
	.stack (NOLOAD) : ALIGN(16)
	{
		__STACK_START__ = .;
		. = . + PLATFORM_STACK_SIZE;
		__STACK_TOP__ = .;
	} >STACK
	. = MARK_ADDR;

	.bss : ALIGN(16) {
		__BSS_START__ = .;
		*(.bss*)
		. = ALIGN(4);
		__BSS_END__ = .;
	} >RAM :data

	__BL33Z_END__ = .;
	__BSS_SIZE__ = SIZEOF(.bss);

	ASSERT(. <= BL33Z_LIMIT, "BL33Z image has exceeded its limit.")
}
