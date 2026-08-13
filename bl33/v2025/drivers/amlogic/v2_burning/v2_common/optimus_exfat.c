// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2025 Amlogic, Inc. All rights reserved.
 */
#include "../v2_burning_i.h"
#include "../v2_sdc_burn/optimus_sdc_burn_i.h"
#include <amlogic/partition_table.h>
#include <vsprintf.h>
#include <blk.h>
#include <config.h>
#include <exports.h>
#include <fat.h>
#include <fs.h>
#include <asm/byteorder.h>
#include <exfat.h>
#include <part.h>
#include <malloc.h>
#include <memalign.h>
#include <linux/compiler.h>
#include <linux/ctype.h>
#undef debug
#define debug(fmt...) //printf("%s[%3d]", __func__, __LINE__),printf(fmt)


struct uboot_file {
	int64_t fileoffset;
	int64_t filesize;
	char path[];
};

#define MAX_UBOOT_FILE		4

struct uboot_file *u_file[MAX_UBOOT_FILE];

static char cur_ifname[32];
static char cur_dev_part_str[32];
int _exfatok = -1;

int optimus_exfat_register_device(const char *ifname, const char *dev_part_str)
{
	int ret = -1;

	ret = fs_set_blk_dev(ifname, dev_part_str, FS_TYPE_EXFAT);
	printf("%s, if:%s, dev:%s, ret:%d\n", __func__, ifname, dev_part_str, ret);
	if (ret < 0) {
		_exfatok = 0;
		ret =  optimus_fat_register_device(ifname, dev_part_str);
	}
	else {
		_exfatok = 1;
		if (!ret) {
			strncpy(cur_ifname, ifname, sizeof(cur_ifname));
			strncpy(cur_dev_part_str, dev_part_str, sizeof(cur_dev_part_str));
		} else {
			memset(cur_ifname, 0, sizeof(cur_ifname));
			memset(cur_dev_part_str, 0, sizeof(cur_dev_part_str));
		}
	}
	return ret;
}

unsigned do_exfat_get_bytesperclust(int fd)
{
	if (cur_ifname[0]) {
		//fs_set_blk_dev(cur_ifname, cur_dev_part_str, FS_TYPE_EXFAT);
		return exfat_fs_bytespercluster();
	}
	return -ENODEV;
}

static int find_opened_file(const char * filename)
{
	int i;

	for (i = 0; i < MAX_UBOOT_FILE; i++) {
		if (u_file[i]) { // opened file
			if (strcmp(u_file[i]->path, filename)) {
				return i;
			}
		}
	}
	return -1;
}

static int find_empty_file(void)
{
	int i;

	for (i = 0; i < MAX_UBOOT_FILE; i++) {
		if (!u_file[i]) {
			return i;
		}
	}
	return -1;
}

s64 do_exfat_get_fileSz(const char* imgItemPath)
{
	loff_t file_size = -1;
	int ret;

	if (cur_ifname[0]) {
		//fs_set_blk_dev(cur_ifname, cur_dev_part_str, FS_TYPE_EXFAT);
		ret = fs_size(imgItemPath, &file_size);
		if (ret)
			return -1;
		return file_size;
	} else {
		return -ENODEV;
	}
}

long do_exfat_fopen(const char *filename)
{
	int len = strlen(filename);
	struct uboot_file *ufile;
	int i;

	i = find_opened_file(filename);
	if (i >= 0) {
		printf("%s alread opened, fd:%d\n", filename, i);
		return i;
	}
	i = find_empty_file();
	if (i >= MAX_UBOOT_FILE) {
		printf("Too many opened files\n");
		return -ENOMEM;
	}

	ufile = malloc(len + sizeof(ufile) + 4);
	if (!ufile) {
		return -ENOMEM;
	}
	memset(ufile, 0, (len + sizeof(ufile) + 4));
	strcpy(ufile->path, filename);
	u_file[i] = ufile;
	u_file[i]->filesize = do_exfat_get_fileSz(filename);

	return i;
}

void do_exfat_fclose(int fd)
{
	if (fd < 0 || fd >= MAX_UBOOT_FILE)
		return;

	if (u_file[fd])
		free(u_file[fd]);
	u_file[fd] = NULL;
}

/* wherehence: 0 to seek from start of file; 1 to seek from current position from file */
int do_exfat_fseek(int fd, const int64_t offset, int wherehence)
{
	int64_t pos;

	if (fd < 0 || fd >= MAX_UBOOT_FILE)
		return -EINVAL;

	if (!u_file[fd]) {
		printf("fd %d not opened\n", fd);
		return -EINVAL;
	}
	switch (wherehence) {
	case 0:		//seek from begin
		pos = offset;
		break;

	case 1:		//seek from current
		pos = u_file[fd]->fileoffset + offset;
		break;
	default:
		DWN_ERR("wherehence %d err\n", wherehence);
		return -__LINE__;
	}
	if (pos > u_file[fd]->filesize || pos < 0) {
		printf("pos %llx out of range %llx\n", pos, u_file[fd]->filesize);
		return -EINVAL;
	}
	u_file[fd]->fileoffset = pos;

	return 0;
}

long do_exfat_fread(int fd, __u8 *buffer, unsigned long maxsize)
{
	loff_t actRead    = 0;
	int ret = -__LINE__;
	struct uboot_file *ufile;

	if (fd < 0 || fd >= MAX_UBOOT_FILE || !buffer)
		return -EINVAL;

	if (!maxsize)
		return 0;
	ufile = u_file[fd];
	if (!ufile) {
		printf("fd %d not opened\n", fd);
		return -EINVAL;
	}

	if (!cur_ifname[0])
		return -ENODEV;

	if (_exfatok <= 0)
		fs_set_blk_dev(cur_ifname, cur_dev_part_str, FS_TYPE_EXFAT);

	ret = fs_read(ufile->path, (unsigned long)buffer, ufile->fileoffset, maxsize, &actRead);
	if (ret) {
		printf("read %s failed, ret:%d\n", ufile->path, ret);
		return -1;
	}
	ufile->fileoffset += actRead;

	return actRead;
}

long do_exfat_ftell(int fd)
{
	struct uboot_file *ufile;

	if (fd < 0 || fd >= MAX_UBOOT_FILE)
		return -EINVAL;

	ufile = u_file[fd];
	if (!ufile) {
		printf("fd %d not opened\n", fd);
		return -EINVAL;
	}

	return ufile->fileoffset;
}

//<0 if failed, 0 is normal, 1 is sparse, others reserved
int do_exfat_get_file_format(const char* imgFilePath, unsigned char* pbuf, const unsigned bufSz)
{
	int readSz = 0;
	int hFile = do_exfat_fopen(imgFilePath);

	if (hFile < 0) {
		printf("Fail to open file (%s)\n", imgFilePath);
		return -1;
	}

	readSz = do_exfat_fread(hFile, pbuf, bufSz);
	if (readSz <= 0) {
		printf("Fail to read file(%s), readSz=%d\n", imgFilePath, readSz);
		do_exfat_fclose(hFile);
		return -1;
	}

	readSz = optimus_simg_probe(pbuf, readSz);

	do_exfat_fclose(hFile);

	return readSz;
}
