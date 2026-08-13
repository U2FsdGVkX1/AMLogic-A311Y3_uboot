/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2013-2014, ARM Limited and Contributors. All rights reserved.
 */

#include "platform_def.h"
#include "regs.h"

OUTPUT_FORMAT("elf64-littleaarch64", "elf64-littleaarch64", "elf64-littleaarch64")
OUTPUT_ARCH(aarch64)

ENTRY(bl33z_entrypoint)

MEMORY {
    RAM (rw) : ORIGIN = BL33Z_LOAD_ADDR, LENGTH = BL33Z_LOAD_SIZE
}

MEMORY {
    ROM (rx): ORIGIN = BL33Z_LOAD_ADDR, LENGTH = BL33Z_LOAD_SIZE
}

MEMORY {
    STACK (rw) : ORIGIN = PLATFORM_STACK_TOP, LENGTH = PLATFORM_STACK_SIZE
}

SECTIONS
{
    . = BL33Z_LOAD_ADDR;
    . = ALIGN(16);

    .vector . : {
        *(.vectors*)
    } >ROM

    ro . : {
        __RO_START__ = .;
        *bl33z_entrypoint.o(.text*)
        *(.text*)
        *(.rodata*)
        __RO_END_UNALIGNED__ = .;
        . = NEXT(8);
        __RO_END__ = .;
    } >ROM

    .data . : {
        __DATA_START__ = .;
        *(.data*)
        __DATA_END__ = .;
    } >RAM

    MARK_ADDR = .;
    stacks (NOLOAD) : {
        __STACKS_START__ = .;
        *(tzfw_normal_stacks)
        __STACKS_END__ = .;
    } >STACK
    . = MARK_ADDR;

    .bss : ALIGN(16) {
        __BSS_START__ = .;
        *(SORT_BY_ALIGNMENT(.bss*))
        *(COMMON)
        __BSS_END__ = .;
    } >RAM

#ifdef COHERENT_RAM
    coherent_ram (NOLOAD) : ALIGN(16) {
        __COHERENT_RAM_START__ = .;
        *(tzfw_coherent_mem)
        __COHERENT_RAM_END_UNALIGNED__ = .;
        . = NEXT(16);
        __COHERENT_RAM_END__ = .;
    } >RAM
#endif

    __BL33Z_END__ = .;
    __BSS_SIZE__ = SIZEOF(.bss);

#ifdef COHERENT_RAM
    __COHERENT_RAM_UNALIGNED_SIZE__ =
        __COHERENT_RAM_END_UNALIGNED__ - __COHERENT_RAM_START__;
#endif

    ASSERT(. <= BL33Z_LIMIT, "BL33Z image has exceeded its limit.")
}
