/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __XLAT_MMU_HELPERS_H__
#define __XLAT_MMU_HELPERS_H__

#ifdef AARCH32
/* AArch32 specific translation table API */
void enable_mmu_secure(uint32_t flags);
#else
/* AArch64 specific translation table APIs */
void enable_mmu_el1(unsigned int flags);
void enable_mmu_el3(unsigned int flags);
#endif /* AARCH32 */

#endif /* __XLAT_MMU_HELPERS_H__ */
