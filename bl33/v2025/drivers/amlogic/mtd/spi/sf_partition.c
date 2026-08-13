// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <dm.h>
#include <errno.h>
#include <malloc.h>
#include <spi.h>
#include <spi_flash.h>
#include <linux/mtd/partitions.h>
#include <linux/types.h>
#include <linux/sizes.h>
#include <malloc.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <mtd.h>
#include <amlogic/aml_mtd.h>
#include <amlogic/storage.h>
#include <amlogic/cpu_id.h>

extern boot_area_entry_t general_boot_part_entry[MAX_BOOT_AREA_ENTRIES];

extern struct mtd_partition *get_spiflash_partition_table(int *partitions);
int spinor_add_partitions(struct mtd_info *mtd)
{
	struct mtd_partition *spiflash_partitions;
	int partition_count;

#ifdef STORE_PARAM_SRAM_ADDR
	extern int mtd_add_fastboot_partitions(struct mtd_info *mtd);
	if (store_is_fastboot())
		return mtd_add_fastboot_partitions(mtd);
#endif

	spiflash_partitions = get_spiflash_partition_table(&partition_count);

	return mtd_spi_nor_add_partitions(mtd, spiflash_partitions,
					partition_count);
}

int spinor_del_partitions(struct mtd_info *mtd)
{
	return del_mtd_partitions(mtd);
}
