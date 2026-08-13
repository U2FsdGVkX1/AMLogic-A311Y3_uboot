/*
 * =====================================================================================
 *
 *       Filename:  gen_misc_data.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  02/21/2019 08:01:16 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  manliang.tang (mn), 
 *        Company:  
 *
 * =====================================================================================
 */

#include <stdio.h>
#include <string.h>

typedef unsigned char uint8_t;
typedef unsigned int uint32_t;
/* Magic for the A/B struct when serialized. */
#define AVB_AB_MAGIC "\0AB0"
#define AVB_AB_MAGIC_LEN 4

/* Versioning for the on-disk A/B metadata - keep in sync with avbtool. */
#define AVB_AB_MAJOR_VERSION 1
#define AVB_AB_MINOR_VERSION 0

/* Size of avb_ab_data struct. */
#define AVB_AB_DATA_SIZE 32

/* Maximum values for slot data */
#define AVB_AB_MAX_PRIORITY 15
#define AVB_AB_MAX_TRIES_REMAINING 3

struct AvbABSlotData {
	/* Slot priority. Valid values range from 0 to AVB_AB_MAX_PRIORITY,
	 * both inclusive with 1 being the lowest and AVB_AB_MAX_PRIORITY
	 * being the highest. The special value 0 is used to indicate the
	 * slot is unbootable.
	 */

	uint8_t priority;

	/* Number of times left attempting to boot this slot ranging from 0
	 * to AVB_AB_MAX_TRIES_REMAINING.
	 */
	uint8_t tries_remaining;

	/* Non-zero if this slot has booted successfully, 0 otherwise. */
	uint8_t successful_boot;

	/* Reserved for future use. */
	uint8_t reserved[1];
} __attribute__((packed));

/* Struct used for recording A/B metadata.
 *
 * When serialized, data is stored in network byte-order.
 */
struct AvbABData {
	/* Magic number used for identification - see AVB_AB_MAGIC. */
	uint8_t magic[AVB_AB_MAGIC_LEN];

	/* Version of on-disk struct - see AVB_AB_{MAJOR,MINOR}_VERSION. */
	uint8_t version_major;
	uint8_t version_minor;

	/* Padding to ensure |slots| field start eight bytes in. */
	uint8_t reserved1[2];

	/* Per-slot metadata. */
	struct AvbABSlotData slots[2];

	/* Reserved for future use. */
	uint8_t reserved2[12];

	/* CRC32 of all 28 bytes preceding this field. */
	uint32_t crc32;
} __attribute__((packed));

/* The slot_suffix field is used for A/B implementations.
 * Offset 0 from the beginning of misc partition 
 */
struct ab_settings {
    struct AvbABData ab_data; // 32 bytes

    //Round up the entire struct to 4096-byte.
    char reserved[4064];
};

/* Code taken from FreeBSD 8 */

static uint32_t iavb_crc32_tab[] = {
    0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
    0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
    0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
    0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
    0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
    0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
    0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
    0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
    0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
    0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
    0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
    0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
    0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
    0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
    0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
    0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
    0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
    0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
    0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
    0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
    0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
    0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
    0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
    0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
    0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
    0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
    0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
    0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
    0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
    0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
    0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
    0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
    0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
    0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
    0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
    0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
    0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
    0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
    0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
    0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
    0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
    0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
    0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d};


/*
 * A function that calculates the CRC-32 based on the table above is
 * given below for documentation purposes. An equivalent implementation
 * of this function that's actually used in the kernel can be found
 * in sys/libkern.h, where it can be inlined.
 */

static uint32_t iavb_crc32(uint32_t crc_in, const uint8_t* buf, int size) {
  const uint8_t* p = buf;
  uint32_t crc;

  crc = crc_in ^ ~0U;
  while (size--)
    crc = iavb_crc32_tab[(crc ^ *p++) & 0xFF] ^ (crc >> 8);
  return crc ^ ~0U;
}

uint32_t avb_crc32(const uint8_t* buf, size_t size) {
  return iavb_crc32(0, buf, size);
}

uint32_t avb_htobe32(uint32_t in) {
	union {
		uint32_t word;
		uint8_t bytes[4];
	} ret;
	ret.bytes[0] = (in >> 24) & 0xff;
	ret.bytes[1] = (in >> 16) & 0xff;
	ret.bytes[2] = (in >> 8) & 0xff;
	ret.bytes[3] = in & 0xff;
	return ret.word;
}

int reflect(int data, int len) {
	int ref = 0;
	int i;
	for (i = 0; i < len; i++) {
		if (data & 0x1) {
			ref |= (1 << ((len - 1) - i));
		}
		data = (data >> 1);
	}
	return ref;
}

uint32_t calculate_crc32(void* buf, uint32_t len) {
	uint32_t i, j;
	uint32_t byte_length = 8; /*length of unit (i.e. byte) */
	int msb = 0;
	int polynomial = 0x04C11DB7;    /* IEEE 32bit polynomial */
	unsigned int regs = 0xFFFFFFFF; /* init to all ones */
	int regs_mask = 0xFFFFFFFF;     /* ensure only 32 bit answer */
	int regs_msb = 0;
	unsigned int reflected_regs;

	for (i = 0; i < len; i++) {
		int data_byte = *((uint8_t*)buf + i);
		data_byte = reflect(data_byte, 8);
		for (j = 0; j < byte_length; j++) {
			msb = data_byte >> (byte_length - 1); /* get MSB */
			msb &= 1;                             /* ensure just 1 bit */
			regs_msb = (regs >> 31) & 1;          /* MSB of regs */
			regs = regs << 1;                     /* shift regs for CRC-CCITT */
			if (regs_msb ^ msb) {                 /* MSB is a 1 */
				regs = regs ^ polynomial;           /* XOR with generator poly */
			}
			regs = regs & regs_mask; /* Mask off excess upper bits */
			data_byte <<= 1;         /* get to next bit */
		}
	}
	regs = regs & regs_mask;
	reflected_regs = reflect(regs, 32) ^ 0xFFFFFFFF;

	return reflected_regs;
}

void main(int argc, char **argv)
{

	int size = 0;
	int len = 0;
	char test_data[]="1234";
	unsigned int crc32;
	unsigned int crc32_check;
	unsigned int crc32_verify;
	FILE *file = NULL;
	int ret = -1;
	char *output_file = "misc.img";
	struct ab_settings ab_setting_default;
	int i, ab = 0, retry = AVB_AB_MAX_TRIES_REMAINING;

	if (argc > 4) {
show_usage:
		printf("usage:\n"
			"\tgen_misc_data -a/b -r [n]\n"
			"\t-a boot from a\n"
			"\t-b boot from b\n"
			"\t-r retry count\n");
		exit(-1);
	}
	for (i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-a")) {
			ab = 0;
			continue;
		}
		if (!strcmp(argv[i], "-b")) {
			ab = 1;
			continue;
		}
		if (!strcmp(argv[i], "-r")) {
			if (i + 1 > argc) {
				goto show_usage;
			}
			retry = atoi(argv[i + 1]);
			i++;
			continue;
		}
		goto show_usage;
	}

	/* data init */
	memset(&ab_setting_default, '\0', sizeof(struct ab_settings));
	memcpy(ab_setting_default.ab_data.magic, AVB_AB_MAGIC, AVB_AB_MAGIC_LEN);
	ab_setting_default.ab_data.version_major = AVB_AB_MAJOR_VERSION;
	ab_setting_default.ab_data.version_minor = AVB_AB_MINOR_VERSION;
	if (ab) { // from b
		ab_setting_default.ab_data.slots[0].priority = AVB_AB_MAX_PRIORITY - 1;
		ab_setting_default.ab_data.slots[1].priority = AVB_AB_MAX_PRIORITY;
	} else { // from a
		ab_setting_default.ab_data.slots[0].priority = AVB_AB_MAX_PRIORITY;
		ab_setting_default.ab_data.slots[1].priority = AVB_AB_MAX_PRIORITY - 1;
	}
	ab_setting_default.ab_data.slots[0].tries_remaining = retry;
	ab_setting_default.ab_data.slots[0].successful_boot = 0;
	ab_setting_default.ab_data.slots[1].tries_remaining = retry;
	ab_setting_default.ab_data.slots[1].successful_boot = 0;

	printf("magic = %s\n", ab_setting_default.ab_data.magic);
	printf("major version:%d\n", ab_setting_default.ab_data.version_major);

	crc32 = avb_crc32((const uint8_t*)(&ab_setting_default.ab_data), sizeof(struct AvbABData) - sizeof(uint32_t));
	printf("crc32:0x%x\n", crc32);
	ab_setting_default.ab_data.crc32= crc32;

	crc32_check= calculate_crc32("1", 1);
	crc32_check = avb_htobe32(crc32_check);
	printf("crc32_check:0x%x\n", crc32_check);
	//ab_setting_default.ab_data.crc32= crc32_check;
	crc32_verify= calculate_crc32((void *)(&ab_setting_default.ab_data), AVB_AB_DATA_SIZE);
	printf("crc32_verify:0x%x\n", crc32_verify);
	size = sizeof(ab_setting_default);
	printf("ab_data size:%d\n", size);
	file = fopen(output_file, "wb+");
	if(file == NULL)
	{
		printf("open file %s failure\n", output_file);
		return;
	}

	fseek(file, 0, SEEK_SET);
	fwrite((void *)&ab_setting_default, size, 1, file);

	fclose(file);
}
