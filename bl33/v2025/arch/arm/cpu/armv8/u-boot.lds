/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2013
 * David Feng <fenghua@phytium.com.cn>
 *
 * (C) Copyright 2002
 * Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
 */

#include <config.h>
#include <asm/psci.h>

OUTPUT_FORMAT("elf64-littleaarch64", "elf64-littleaarch64", "elf64-littleaarch64")
OUTPUT_ARCH(aarch64)
ENTRY(_start)
SECTIONS
{
#ifdef CONFIG_ARMV8_SECURE_BASE
	/DISCARD/ : { *(.rela._secure*) }
#endif
	. = 0x00000000;

	. = ALIGN(8);
	 __TEXT_START__ = .;
	__image_copy_start = ADDR(.text);
	.text :
	{
		CPUDIR/start.o (.text*)
	}

	/* This needs to come before *(.text*) */
	.efi_runtime : {
                __efi_runtime_start = .;
		. = ALIGN(4096);
		__EFI_TEXT_START__ = .;
		*(.text.efi_runtime*)
		. = ALIGN(4096);
		__EFI_RO_START__ = .;
		*(.rodata.efi_runtime*)
		. = ALIGN(4096);
		__EFI_DATA_START__ = .;
		*(.data.efi_runtime*)
		. = ALIGN(4096);
		__EFI_END__ = .;
                __efi_runtime_stop = .;
	}

	. = ALIGN(4096);
	 __TEXT_REAL_START__ = .;
	.text_rest :
	{
		*(.text*)
	}

	. = ALIGN(4096);
	__TEXT_END__ = .;

#ifdef CONFIG_ARMV8_PSCI
	.__secure_start :
#ifndef CONFIG_ARMV8_SECURE_BASE
		ALIGN(CONSTANT(COMMONPAGESIZE))
#endif
	{
		KEEP(*(.__secure_start))
	}

#ifndef CONFIG_ARMV8_SECURE_BASE
#define __ARMV8_SECURE_BASE
#define __ARMV8_PSCI_STACK_IN_RAM
#else
#define __ARMV8_SECURE_BASE	CONFIG_ARMV8_SECURE_BASE
#endif
	.secure_text __ARMV8_SECURE_BASE :
		AT(ADDR(.__secure_start) + SIZEOF(.__secure_start))
	{
		*(._secure.text)
		. = ALIGN(8);
		__secure_svc_tbl_start = .;
		KEEP(*(._secure_svc_tbl_entries))
		__secure_svc_tbl_end = .;
	}

	.secure_data : AT(LOADADDR(.secure_text) + SIZEOF(.secure_text))
	{
		*(._secure.data)
	}

	.secure_stack ALIGN(ADDR(.secure_data) + SIZEOF(.secure_data),
			    CONSTANT(COMMONPAGESIZE)) (NOLOAD) :
#ifdef __ARMV8_PSCI_STACK_IN_RAM
		AT(ADDR(.secure_stack))
#else
		AT(LOADADDR(.secure_data) + SIZEOF(.secure_data))
#endif
	{
		KEEP(*(.__secure_stack_start))

		. = . + CONFIG_ARMV8_PSCI_NR_CPUS * ARM_PSCI_STACK_SIZE;

		. = ALIGN(CONSTANT(COMMONPAGESIZE));

		KEEP(*(.__secure_stack_end))
	}

#ifndef __ARMV8_PSCI_STACK_IN_RAM
	. = LOADADDR(.secure_stack);
#endif

	.__secure_end : AT(ADDR(.__secure_end)) {
		KEEP(*(.__secure_end))
		LONG(0x1d1071c);	/* Must output something to reset LMA */
	}
#endif

	. = ALIGN(4096);
	__RO_START__ = .;
#ifdef CONFIG_KALLSYMS
	.__rodata_start :
	{
		*(.__rodata_start)
	}
#endif
	.rodata : { *(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*))) }

	. = ALIGN(4096);
	__RO_END__ = .;

	. = ALIGN(4096);
	__DATA_START__ = .;
	. = ALIGN(8);
	.data : {
		*(.data*)
	}

	. = ALIGN(4096);
	__DATA_END__ = .;
	. = .;

	. = ALIGN(8);
	__u_boot_list : {
		KEEP(*(SORT(__u_boot_list*)));
	}

	.efi_runtime_rel : {
                __efi_runtime_rel_start = .;
		*(.rel*.efi_runtime)
		*(.rel*.efi_runtime.*)
                __efi_runtime_rel_stop = .;
	}

	. = ALIGN(8);
	__image_copy_end = .;

	.rela.dyn : {
		__rel_dyn_start = .;
		*(.rela*)
		__rel_dyn_end = .;
	}

#if CONFIG_IS_ENABLED(AMLOGIC_RAMDUMP)
	. = ALIGN(4096);
#endif
	_end = .;

	/*
	 * arch/arm/lib/crt0_64.S assumes __bss_start - __bss_end % 8 == 0
	 */
	. = ALIGN(4096);
	__BSS_START__ = .;
	.bss ALIGN(8) : {
		__bss_start = .;
		*(.bss*)
		. = ALIGN(8);
		__bss_end = .;
	}

	. = ALIGN(4096);
	__BSS_END__ = .;

	/DISCARD/ : { *(.dynsym) }
	/DISCARD/ : { *(.dynstr*) }
	/DISCARD/ : { *(.dynamic*) }
	/DISCARD/ : { *(.plt*) }
	/DISCARD/ : { *(.interp*) }
	/DISCARD/ : { *(.gnu*) }

#ifdef CONFIG_LINUX_KERNEL_IMAGE_HEADER
#include "linux-kernel-image-header-vars.h"
#endif
}
