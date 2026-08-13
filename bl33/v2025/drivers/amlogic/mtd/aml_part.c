// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#define pr_fmt(fmt)	"aml_part: " fmt

#include <errno.h>
#include <linux/err.h>
#include <linux/compat.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/mtd/spinand.h>
#include<asm/amlogic/arch/cpu_config.h>
#include <amlogic/aml_mtd.h>
#include <amlogic/storage.h>
#include <amlogic/aml_rsv.h>
#include <linux/sizes.h>

/* Hard code, all partitions are aligned in block size, fast erasing */
#define SPINOR_ALIGNED_SIZE		(64 * 1024)
#ifdef STORE_PARAM_SRAM_ADDR
#define FILE_ALIGN_SIZE		(4096)
#define STORAGE_ROUND_UP_IF_UNALIGN(x, y) \
    (((x) + (y) - 1) & (~((y) - 1)))

struct storage_part {
	char name[16];
	unsigned int offset;
	unsigned int size;
	unsigned char flags;
};

struct storage_parts {
#define STORAGE_PART_MAGIC "STO_PART"
#define STORAGE_MAGIC_LEN (8)
	char magic[STORAGE_MAGIC_LEN];
	unsigned char version;
	unsigned char reserved[3];
	unsigned int count;
#define STORAGE_PART_OFFSET (16)
	struct storage_part *parts;
};

static int check_param_hrd(struct storage_parts *parts)
{
	return memcmp(parts->magic, STORAGE_PART_MAGIC, STORAGE_MAGIC_LEN);
}
#endif
/* Bootloader: Bootloader + Pageinfo align with 4K */
uint64_t spinor_bootloader_size(void)
{
#if defined(CONFIG_BOOTLOADER_SIZE)
#ifdef SPINOR_HAS_BOOTINFO
	uint16_t pageinfo_size = 0x200;
#else
	uint16_t pageinfo_size = 0;
#endif
	return ((DIV_ROUND_UP((CONFIG_BOOTLOADER_SIZE + pageinfo_size), 0x1000)) << 12);
#else
	return SZ_2M;
#endif
}

uint32_t __weak spinor_rsv_block_num(void)
{
	return 0;
}

uint64_t spinor_bootpart_size(void)
{
#if defined(CONFIG_BOOTLOADER_SIZE)
	if (store_get_device_bootloader_mode() == COMPACT_BOOTLOADER)
		return ((DIV_ROUND_UP(spinor_bootloader_size(), SPINOR_ALIGNED_SIZE) << 16)
			* CONFIG_NOR_TPL_COPY_NUM);
	else
		return (((DIV_ROUND_UP((spinor_bootloader_size() +spinor_rsv_block_num()),
			SPINOR_ALIGNED_SIZE)) << 16) * CONFIG_NOR_TPL_COPY_NUM);
#else
	return SZ_2M;
#endif
}

static void mtd_get_logic_part_info(struct mtd_info *mtd,
				    struct mtd_partition *part)
{
	loff_t offset = part->offset, end = part->offset + part->size;
	loff_t append_size = 0;

#ifdef CONFIG_NOT_SKIP_BAD_BLOCK
	return;
#endif

	do {
		if (mtd->_block_isbad(mtd, offset) == NAND_FACTORY_BAD) {
			pr_err("%s %d found bad block in 0x%llx\n",
					__func__, __LINE__, offset);
			end += mtd->erasesize;
			append_size += mtd->erasesize;
		}
		offset += mtd->erasesize;
	} while (offset < end && offset < mtd->size);
	part->size += append_size;
}

static inline void set_part_info(struct mtd_partition *part,
			const char *name, uint64_t offset, uint64_t size)
{
	part->name = name;
	part->offset = offset;
	part->size = size;
}

uint64_t mtd_get_normal_part_offset(struct mtd_info *mtd)
{
	if (mtd->type == MTD_NORFLASH)
		return spinor_bootpart_size();
	else
		return meson_rsv_part_get_tpl_start(mtd) + meson_rsv_part_get_tpl_size(mtd);
}

#ifdef STORE_PARAM_SRAM_ADDR
int mtd_add_fastboot_partitions(struct mtd_info *mtd)
{
	struct storage_parts *parts = (struct storage_parts *)STORE_PARAM_SRAM_ADDR;
	struct storage_part *part = (struct storage_part *)(STORE_PARAM_SRAM_ADDR + STORAGE_PART_OFFSET);
	struct mtd_partition *new_part;
	int i, j, ret, nbparts, bootpart_num = 0;

	if (check_param_hrd(parts)) {
		pr_err("store param check failed!\n");
		return -1;
	}

	if (mtd->type == MTD_NORFLASH)
		bootpart_num = 2;	// add bl2 and firmware hdr

	nbparts = parts->count + bootpart_num;
	new_part = kcalloc(nbparts, sizeof(*new_part), GFP_KERNEL);
	if (!new_part)
		return -ENOMEM;

	if (mtd->type == MTD_NORFLASH) {
		set_part_info(&new_part[0], BOOT_BL2, 0, STORAGE_ROUND_UP_IF_UNALIGN(BL2_SIZE, FILE_ALIGN_SIZE));
		set_part_info(&new_part[1], BOOT_HDR, new_part[0].size, FILE_ALIGN_SIZE);
	}

	for (i = bootpart_num, j = 0; i < nbparts; i++, j++) {
		if (i == nbparts - 1)
			set_part_info(&new_part[i], part[j].name, part[j].offset, mtd->size - part[j].offset);
		else
			set_part_info(&new_part[i], part[j].name, part[j].offset, part[j].size);
	}
	ret = add_mtd_partitions(mtd, new_part, nbparts);
	kfree(new_part);

	return ret;
}
#endif

int mtd_add_partitions(struct mtd_info *mtd, const struct mtd_partition *parts,
		       int nbparts, bool bl2_part_only, bool slc_nand)
{
	struct mtd_partition *new_part, *p, bl2_part[1] = { 0 };
	u64 normal_offset;
	int i, n = nbparts, bootpart_num, ret;

	if (bl2_part_only) {
		if (!slc_nand)
			return -1;
		set_part_info(&bl2_part[0], BOOT_BL2, 0,
			      meson_rsv_part_get_bl2_part_size(mtd));
		ret = add_mtd_partitions(mtd, &bl2_part[0], 1);
		return ret;
	}

	if (mtd->type == MTD_NORFLASH)
		bootpart_num = 1;
	else
		bootpart_num = 2;

	n += bootpart_num;

	new_part = kcalloc(n, sizeof(*new_part), GFP_KERNEL);
	if (!new_part)
		return -ENOMEM;

	if (mtd->type == MTD_NORFLASH) {
		set_part_info(&new_part[0], BOOT_LOADER, 0,
				spinor_bootpart_size());
	} else {
		/* simplify that bootloader only has BL2 + TPL */
		set_part_info(&new_part[0], BOOT_BL2, 0,
				meson_rsv_part_get_bl2_part_size(mtd));
		set_part_info(&new_part[1], BOOT_TPL,
				meson_rsv_part_get_tpl_start(mtd),
				meson_rsv_part_get_tpl_size(mtd));
	}

	/* filter out these partitions with wrong size ZERO  */
	for (i = 0, p = new_part + bootpart_num; i < nbparts; i++) {
		if (!parts[i].size && !parts[i].offset)
			continue;
		*p++ = parts[i];
	}

	normal_offset = mtd_get_normal_part_offset(mtd);
	n = p - new_part - bootpart_num;
	for (i = 0, p = new_part + bootpart_num; i < n; i++, p++) {
		if ((normal_offset + p->size) > mtd->size) {
			pr_err("over flash size!\n");
			kfree(new_part);
			return -1;
		}
		p->offset = normal_offset;
		if (mtd->type != MTD_NORFLASH)
			mtd_get_logic_part_info(mtd, p);
		if (i == (n - 1))
			p->size = mtd->size - normal_offset;
		normal_offset += p->size;
	}

	p = (slc_nand) ? new_part + 1 : new_part;
	nbparts = (slc_nand) ? n + 1 : n + bootpart_num;
	ret = add_mtd_partitions(mtd, p, nbparts);
	kfree(new_part);

	return ret;
}

int mtd_raw_nand_add_boot_partitions(struct mtd_info *mtd)
{
	return mtd_add_partitions(mtd, NULL, 0, true, true);
}

int mtd_raw_nand_add_normal_partitions(struct mtd_info *mtd,
				       const struct mtd_partition *parts,
				       int nbparts)
{
	return mtd_add_partitions(mtd, parts, nbparts, false, true);
}

int mtd_spi_nand_add_partitions(struct mtd_info *mtd,
				const struct mtd_partition *parts,
				int nbparts)
{
	return mtd_add_partitions(mtd, parts, nbparts, false, false);
}

int mtd_spi_nor_add_partitions(struct mtd_info *mtd,
				const struct mtd_partition *parts,
				int nbparts)
{
	return mtd_add_partitions(mtd, parts, nbparts, false, false);
}
