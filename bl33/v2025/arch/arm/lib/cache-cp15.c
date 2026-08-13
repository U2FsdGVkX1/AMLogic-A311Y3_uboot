// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <cpu_func.h>
#include <log.h>
#include <asm/global_data.h>
#include <asm/system.h>
#include <asm/cache.h>
#include <linux/compiler.h>
#include <asm/armv7_mpu.h>

#ifdef CONFIG_AMLOGIC_MODIFY
#include <asm/amlogic/arch/register.h>
#include <asm/io.h>
#include <linux/sizes.h>
#endif

#if !(CONFIG_IS_ENABLED(SYS_ICACHE_OFF) && CONFIG_IS_ENABLED(SYS_DCACHE_OFF))

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_SYS_ARM_MMU
__weak void arm_init_before_mmu(void)
{
}

static void set_section_phys(int section, phys_addr_t phys,
			     enum dcache_option option)
{
#ifdef CONFIG_ARMV7_LPAE
	u64 *page_table = (u64 *)gd->arch.tlb_addr;
	/* Need to set the access flag to not fault */
	u64 value = TTB_SECT_AP | TTB_SECT_AF;
#else
	u32 *page_table = (u32 *)gd->arch.tlb_addr;
	u32 value = TTB_SECT_AP;
#endif

	/* Add the page offset */
	value |= phys;

	/* Add caching bits */
	value |= option;

	/* Set PTE */
	page_table[section] = value;
}

#ifdef CONFIG_ARMV7_LPAE
static void set_page_dcache(phys_addr_t phys, enum dcache_option option)
{
}
#else
#define L1_SECTION_TYPE		0x02
#define L1_PAGETABLE_TYPE	0x01
#define L2_MMU_PAGE_SHIFT	12
u32 used_page = 0;
static u32 find_l2_page_addr(phys_addr_t phys, u32* page_addr)
{
	u32 l1idx, l2table_sum;
	u32 *section_table = (u32 *)gd->arch.tlb_addr;
	u32 attr, ret;
	u32 l2_start = gd->arch.tlb_addr + PGTABLE_SIZE;

	l2table_sum = (gd->arch.tlb_size - PGTABLE_SIZE) >> 10;  //L2 table is 1K
	l1idx = phys >> MMU_SECTION_SHIFT;
	attr = section_table[l1idx];

	if ((attr & 0x3) == L1_PAGETABLE_TYPE) {
		*page_addr = attr >> 3 << 3;
		ret = 1;  //existed page
		return ret;
	}

	if (used_page < l2table_sum) {
		l2_start +=  used_page * 0x400;
		*page_addr = l2_start;
		ret = 2;	//new page
	} else {
		ret = 0;  //not found
		printf("no free memory for l2 page table\n");
	}
	used_page++;

	return ret;
}

static void set_page_phys(u32 *base_table, phys_addr_t phys, u32 size, enum dcache_option option)
{
	u32 i, end_idx, value;
	phys_addr_t addr = phys;

	end_idx = (size >> L2_MMU_PAGE_SHIFT);

	if (option == 0) {
		memset((void *)base_table, 0, size);
	} else {
		for (i = 0; i < end_idx; i++) {
			value =  TTB_PAGE_ATTR | addr | option;
			base_table[i] = value;
			addr += (1 << L2_MMU_PAGE_SHIFT);
		}
	}
}

int set_page_dcache(phys_addr_t phys, u32 size, enum dcache_option option, enum dcache_option def_option)
{
	u32 l1idx, l2idx, ret;
	u32 *section_table = (u32 *)gd->arch.tlb_addr;
	u32 page_table = 0;

	ret = find_l2_page_addr(phys, &page_table);
	if (ret == 0)
		return -1; //no free page

	l1idx = phys >> MMU_SECTION_SHIFT;
	if (ret == 2) {
		section_table[l1idx] = page_table | L1_PAGETABLE_TYPE; //set L1
		set_page_phys((u32*)page_table, l1idx << MMU_SECTION_SHIFT, 1 << L2_MMU_PAGE_SHIFT, def_option);
	}

	l2idx =  (phys - (l1idx << MMU_SECTION_SHIFT)) >> L2_MMU_PAGE_SHIFT;
	page_table += (l2idx << 2);
	set_page_phys((u32*)page_table, phys, size, option);

	return 0;
}

#endif

void set_section_dcache(int section, enum dcache_option option)
{
	set_section_phys(section, (u32)section << MMU_SECTION_SHIFT, option);
}

__weak void mmu_page_table_flush(unsigned long start, unsigned long stop)
{
	debug("%s: Warning: not implemented\n", __func__);
}

void mmu_set_region_dcache_behaviour_phys(phys_addr_t start, phys_addr_t phys,
					size_t size, enum dcache_option option)
{
#ifdef CONFIG_ARMV7_LPAE
	u64 *page_table = (u64 *)gd->arch.tlb_addr;
#else
	u32 *page_table = (u32 *)gd->arch.tlb_addr;
#endif
	unsigned long startpt, stoppt;
	unsigned long upto, end;

	/* div by 2 before start + size to avoid phys_addr_t overflow */
	end = ALIGN((start / 2) + (size / 2), MMU_SECTION_SIZE / 2)
	      >> (MMU_SECTION_SHIFT - 1);
	start = start >> MMU_SECTION_SHIFT;

#ifdef CONFIG_ARMV7_LPAE
	debug("%s: start=%pa, size=%zu, option=%llx\n", __func__, &start, size,
	      option);
#else
	debug("%s: start=%pa, size=%zu, option=0x%x\n", __func__, &start, size,
	      option);
#endif
	for (upto = start; upto < end; upto++, phys += MMU_SECTION_SIZE)
		set_section_phys(upto, phys, option);

	/*
	 * Make sure range is cache line aligned
	 * Only CPU maintains page tables, hence it is safe to always
	 * flush complete cache lines...
	 */

	startpt = (unsigned long)&page_table[start];
	startpt &= ~(CONFIG_SYS_CACHELINE_SIZE - 1);
	stoppt = (unsigned long)&page_table[end];
	stoppt = ALIGN(stoppt, CONFIG_SYS_CACHELINE_SIZE);
	mmu_page_table_flush(startpt, stoppt);
}

__weak void dram_bank_mmu_setup(int bank)
{
	struct bd_info *bd = gd->bd;
	int	i;

	/* bd->bi_dram is available only after relocation */
	if ((gd->flags & GD_FLG_RELOC) == 0)
		return;

	debug("%s: bank: %d\n", __func__, bank);
	for (i = bd->bi_dram[bank].start >> MMU_SECTION_SHIFT;
	     i < (bd->bi_dram[bank].start >> MMU_SECTION_SHIFT) +
		 (bd->bi_dram[bank].size >> MMU_SECTION_SHIFT);
	     i++)
		set_section_dcache(i, DCACHE_DEFAULT_OPTION);
}

#ifdef CONFIG_AMLOGIC_MODIFY
/* update mmu map from bl2 ddr auto detect size */

__weak int get_page_region(long* page, u32 max_region)
{
	return -1;
}

void mmu_set_page_table(void)
{
#ifdef MMU_USE_PAGE_TABLE
	long page_table[16]; //max region is 8
	u32 region_num;
	int i, ret;

	region_num = get_page_region(page_table, 8);
	if ((region_num == 0) || (region_num > 8))
		return;

	for (i = 0; i< (region_num * 2); i += 2) {
		ret = set_page_dcache(page_table[i], page_table[i+1], DCACHE_DEFAULT_OPTION, 0);
		if (ret)
			break;
	}
#endif
}

#define DDR_SIZE_256 0X10000000
void mmu_map_update(void)
{
	u32 rsv_ddr;
	u32 rsv_size;
	u32 reg_size;
	u32 i;

#if defined(P_AO_SEC_GP_CFG3)
	rsv_ddr = readl(P_AO_SEC_GP_CFG5);
	rsv_size = readl(P_AO_SEC_GP_CFG3);
#elif defined(SYSCTRL_SEC_STATUS_REG15)
	rsv_ddr = readl(SYSCTRL_SEC_STATUS_REG17);//bl31_start
	reg_size = readl(SYSCTRL_SEC_STATUS_REG15);
#endif
	if ((reg_size >> 16) & 0xff)
		rsv_size = (((reg_size & ~0xffff) >> 16) << 16) + ((reg_size & 0xffff) << 16);
	else
		rsv_size = (((reg_size & ~0xffff) >> 16) << 10) + ((reg_size & 0xffff) << 10);

	for (i = 1; i < (rsv_ddr >> MMU_SECTION_SHIFT); i++)
		set_section_dcache(i, DCACHE_DEFAULT_OPTION);
#ifdef CONFIG_MESON_C4
	if (gd->bd->bi_dram[0].size > DDR_SIZE_256)
#endif
		for (i = (((rsv_ddr + rsv_size) + 0xfffff) >> MMU_SECTION_SHIFT); i < (gd->bd->bi_dram[0].size >> MMU_SECTION_SHIFT); i++)
			set_section_dcache(i, DCACHE_DEFAULT_OPTION);

	for (i = (0xe0000000 >> MMU_SECTION_SHIFT); i < (0x100000000 >> MMU_SECTION_SHIFT); i++)
			set_section_dcache(i, DCACHE_OFF);
	mmu_set_page_table();
}
#endif

/* to activate the MMU we need to set up virtual memory: use 1M areas */
static inline void mmu_setup(void)
{
	u32 reg;

	arm_init_before_mmu();
#ifdef CONFIG_AMLOGIC_MODIFY
	mmu_map_update();
#else
	int i;
	/* Set up an identity-mapping for all 4GB, rw for everyone */
	for (i = 0; i < ((4096ULL * 1024 * 1024) >> MMU_SECTION_SHIFT); i++)
		set_section_dcache(i, DCACHE_OFF);

	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		dram_bank_mmu_setup(i);
	}
#endif

#if defined(CONFIG_ARMV7_LPAE) && __LINUX_ARM_ARCH__ != 4
	/* Set up 4 PTE entries pointing to our 4 1GB page tables */
	for (i = 0; i < 4; i++) {
		u64 *page_table = (u64 *)(gd->arch.tlb_addr + (4096 * 4));
		u64 tpt = gd->arch.tlb_addr + (4096 * i);
		page_table[i] = tpt | TTB_PAGETABLE;
	}

	reg = TTBCR_EAE;
#if defined(CONFIG_SYS_ARM_CACHE_WRITETHROUGH)
	reg |= TTBCR_ORGN0_WT | TTBCR_IRGN0_WT;
#elif defined(CONFIG_SYS_ARM_CACHE_WRITEALLOC)
	reg |= TTBCR_ORGN0_WBWA | TTBCR_IRGN0_WBWA;
#else
	reg |= TTBCR_ORGN0_WBNWA | TTBCR_IRGN0_WBNWA;
#endif
	if (is_hyp()) {
		/* Set HTCR to enable LPAE */
		asm volatile("mcr p15, 4, %0, c2, c0, 2"
			: : "r" (reg) : "memory");
		/* Set HTTBR0 */
		asm volatile("mcrr p15, 4, %0, %1, c2"
			:
			: "r"(gd->arch.tlb_addr + (4096 * 4)), "r"(0)
			: "memory");
		/* Set HMAIR */
		asm volatile("mcr p15, 4, %0, c10, c2, 0"
			: : "r" (MEMORY_ATTRIBUTES) : "memory");
	} else {
		/* Set TTBCR to enable LPAE */
		asm volatile("mcr p15, 0, %0, c2, c0, 2"
			: : "r" (reg) : "memory");
		/* Set 64-bit TTBR0 */
		asm volatile("mcrr p15, 0, %0, %1, c2"
			:
			: "r"(gd->arch.tlb_addr + (4096 * 4)), "r"(0)
			: "memory");
		/* Set MAIR */
		asm volatile("mcr p15, 0, %0, c10, c2, 0"
			: : "r" (MEMORY_ATTRIBUTES) : "memory");
	}
#elif defined(CONFIG_CPU_V7A)
	if (is_hyp()) {
		/* Set HTCR to disable LPAE */
		asm volatile("mcr p15, 4, %0, c2, c0, 2"
			: : "r" (0) : "memory");
	} else {
		/* Set TTBCR to disable LPAE */
		asm volatile("mcr p15, 0, %0, c2, c0, 2"
			: : "r" (0) : "memory");
	}
	/* Set TTBR0 */
	reg = gd->arch.tlb_addr & TTBR0_BASE_ADDR_MASK;
#if defined(CONFIG_SYS_ARM_CACHE_WRITETHROUGH)
	reg |= TTBR0_RGN_WT | TTBR0_IRGN_WT;
#elif defined(CONFIG_SYS_ARM_CACHE_WRITEALLOC)
	reg |= TTBR0_RGN_WBWA | TTBR0_IRGN_WBWA;
#else
	reg |= TTBR0_RGN_WB | TTBR0_IRGN_WB;
#endif
	asm volatile("mcr p15, 0, %0, c2, c0, 0"
		     : : "r" (reg) : "memory");
#else
	/* Copy the page table address to cp15 */
	asm volatile("mcr p15, 0, %0, c2, c0, 0"
		     : : "r" (gd->arch.tlb_addr) : "memory");
#endif
	/*
	 * initial value of Domain Access Control Register (DACR)
	 * Set the access control to client (1U) for each of the 16 domains
	 */
	asm volatile("mcr p15, 0, %0, c3, c0, 0"
		     : : "r" (0x55555555));

	/* and enable the mmu */
	reg = get_cr();	/* get control reg. */
	set_cr(reg | CR_M);
}

static int mmu_enabled(void)
{
	return get_cr() & CR_M;
}
#endif /* CONFIG_SYS_ARM_MMU */

/* cache_bit must be either CR_I or CR_C */
static void cache_enable(uint32_t cache_bit)
{
	uint32_t reg;
	/* The data cache is not active unless the mmu/mpu is enabled too */
#ifdef CONFIG_SYS_ARM_MMU
	if ((cache_bit == CR_C) && !mmu_enabled())
		mmu_setup();
#elif defined(CONFIG_SYS_ARM_MPU)
	if ((cache_bit == CR_C) && !mpu_enabled()) {
		printf("Consider enabling MPU before enabling caches\n");
		return;
	}
#endif
	reg = get_cr();	/* get control reg. */
	set_cr(reg | cache_bit);
}

/* cache_bit must be either CR_I or CR_C */
static void cache_disable(uint32_t cache_bit)
{
	uint32_t reg;

	reg = get_cr();

	if (cache_bit == CR_C) {
		/* if cache isn;t enabled no need to disable */
		if ((reg & CR_C) != CR_C)
			return;
#ifdef CONFIG_SYS_ARM_MMU
		/* if disabling data cache, disable mmu too */
		cache_bit |= CR_M;
#endif
	}
	reg = get_cr();

#ifdef CONFIG_SYS_ARM_MMU
	if (cache_bit == (CR_C | CR_M))
#elif defined(CONFIG_SYS_ARM_MPU)
	if (cache_bit == CR_C)
#endif
		flush_dcache_all();
	set_cr(reg & ~cache_bit);
}
#endif

#if CONFIG_IS_ENABLED(SYS_ICACHE_OFF)
void icache_enable(void)
{
	return;
}

void icache_disable(void)
{
	return;
}

int icache_status(void)
{
	return 0;					/* always off */
}
#else
void icache_enable(void)
{
	cache_enable(CR_I);
}

void icache_disable(void)
{
	cache_disable(CR_I);
}

int icache_status(void)
{
	return (get_cr() & CR_I) != 0;
}
#endif

#if CONFIG_IS_ENABLED(SYS_DCACHE_OFF)
void dcache_enable(void)
{
	return;
}

void dcache_disable(void)
{
	return;
}

int dcache_status(void)
{
	return 0;					/* always off */
}

void mmu_set_region_dcache_behaviour(phys_addr_t start, size_t size,
				     enum dcache_option option)
{
}

#else
void dcache_enable(void)
{
	cache_enable(CR_C);
}

void dcache_disable(void)
{
	cache_disable(CR_C);
}

int dcache_status(void)
{
	return (get_cr() & CR_C) != 0;
}

void mmu_set_region_dcache_behaviour(phys_addr_t start, size_t size,
				     enum dcache_option option)
{
	mmu_set_region_dcache_behaviour_phys(start, start, size, option);
}
#endif
