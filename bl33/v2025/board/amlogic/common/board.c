// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <asm/io.h>
#include <malloc.h>
#include <errno.h>
#include <cli.h>
#include <exports.h>
#include <asm/amlogic/arch/secure_apb.h>
#include <abuf.h>
#include <version.h>
#include <dm.h>
#include <amlogic/aml_v3_burning.h>
#include <amlogic/aml_v2_burning.h>
#include <command.h>
#include <amlogic/board.h>
#include <amlogic/cpu_id.h>
#include <asm-generic/u-boot.h>
#include <amlogic/aml_profile.h>
#ifdef CONFIG_AML_CVBS
#include <amlogic/media/vout/aml_cvbs.h>
#endif
#ifdef CONFIG_AML_VPU
#include <amlogic/media/vpu/vpu.h>
#endif
#ifdef CONFIG_AML_VPP
#include <amlogic/media/vpp/vpp.h>
#endif
#ifdef CONFIG_AML_LCD
#include <amlogic/media/vout/lcd/lcd_vout.h>
#endif
#ifdef CONFIG_CMD_AML_MODEL
#include <amlogic/aml_model.h>
#endif
#ifdef CONFIG_RX_RTERM
#include <amlogic/aml_hdmirx.h>
#endif
#ifdef CONFIG_AML_VOUT
#include <amlogic/media/vout/aml_vout.h>
#endif
#ifdef CONFIG_ARMV8_MULTIENTRY
#include <asm/arch-meson/smp.h>
#endif
#ifdef CONFIG_AMLOGIC_AMFC
#include <amlogic/amfc.h>
#endif
#ifdef CONFIG_RX_RTERM
#include <amlogic/aml_hdmirx.h>
#endif
#ifdef CONFIG_CEC_TRIM_VAL
#include <amlogic/aml_cec.h>
#endif
#ifdef CONFIG_CVBS_CALI
#include <amlogic/aml_tvafe.h>
#endif
#ifdef CONFIG_DDR_FASTBOOT
#include <amlogic/aml_mmc.h>
#endif
DECLARE_GLOBAL_DATA_PTR;
#define UNUSED(x) (void)(x)

#ifdef CONFIG_MISC_INIT_R
#define _AML_MISC_INTERRUPT_KEY 0x09
static int _aml_interrupt_key_pressed;
static int ctrli(void)
{
	if (1/*gd->have_console*/) {
		if (tstc()) {
			switch (getchar()) {
			case _AML_MISC_INTERRUPT_KEY:/* ^I - Control I */
				_aml_interrupt_key_pressed = 1;
				printf("Detect Ctrl I...\t\nInput yes to force stopped anyway:\t");
				return 1;
			default:
				break;
			}
		}
	}

	return 0;
}

static int aml_misc_confirm_yesno(const int tm/*timeout in ms*/)
{
	int i;
	unsigned long ts;
	char str_input[5];

	_aml_interrupt_key_pressed = 0;
	if (!ctrli()) {//not detect ctrl-I when bootup
		return 0;
	}
	ts = get_timer(0);

	for (i = 0; i < sizeof(str_input);) {
		int c = 0;

		while (!tstc()) {
			if (get_timer(ts) >= tm) {
				printf("Input timeout\n");
				return 0;
			}
		}
		c = getchar();
		if (i == 0 && c == _AML_MISC_INTERRUPT_KEY) {//drop first duplicated ctrlI
			printf("Wait YES/y input\n");
			continue;
		}
		putc(c);
		str_input[i++] = c;
		if (c == '\r')
			break;
	}
	putc('\n');
	if (strncmp(str_input, "y\r", 2) == 0 ||
	    strncmp(str_input, "Y\r", 2) == 0 ||
	    strncmp(str_input, "yes\r", 4) == 0 ||
	    strncmp(str_input, "YES\r", 4) == 0)
		return 1;
	return 0;
}

int misc_init_r(void)
{
	printf("board common misc_init\n");
#if defined(CONFIG_SUPPORT_BL33Z) && \
	CONFIG_IS_ENABLED(AMLOGIC_RAMDUMP)
	extern void ramdump_init(void);
	ramdump_init();
#endif
	if (!aml_misc_confirm_yesno(5000))
		return 0;

	cli_init();
	cli_loop();
	panic("No CLI available");
	return 0;
}
#endif // #ifdef CONFIG_MISC_INIT_R

#ifdef CONFIG_DDR_FASTBOOT
typedef struct __ddr_savedparams {
	uint8_t d1[16];
	uint32_t data[2048 / 4 - 16 / 4 - 32 / 4];
	uint8_t d2[32];
} ddr_savedparams_t;

static void ddr_save_fastboot_para(void)
{
	unsigned long addr = 0x2000000UL;

	if (!memcmp((void *)addr, "DDRPARAMS", 8)) { // match tag
		store_ddr_save_fastboot_para(sizeof(ddr_savedparams_t), (void *)(addr + 8));
		printf("Saved ddr fastboot\n");
#ifdef CONFIG_BOOT_FASTBOOT
		run_command("reset", 0);
#endif
	}
}
#endif //CONFIG_DDR_FASTBOOT

#ifdef CONFIG_BOARD_LATE_INIT
int aml_board_late_init_front(void *arg)
{
	UNUSED(arg);
	printf("init front\n");
	run_command("aml_update_env", 0);
	/* ****************************************************
	 * 1.setup bootup resource
	 * ****************************************************
	 */
#ifdef CONFIG_DDR_FASTBOOT
	ddr_save_fastboot_para();
#endif
	board_init_mem();
#ifdef CONFIG_CMD_BOOTCTOL_VAB
	run_command("run bcb_cmd", 0);
#endif

#ifndef CONFIG_SYSTEM_RTOS //prue rtos not need dtb
	PUSH_TIME_TE("load dtb", BL33_LOAD_DTB_s);
	if (run_command("run common_dtb_load", 0)) {
		printf("Fail in load dtb with cmd[%s], try _aml_dtb\n", env_get("common_dtb_load"));
		//load dtb here then users can directly use 'fdt' command
		run_command("echo reboot_mode ${reboot_mode}; "
				"imgread dtb _aml_dtb ${dtb_mem_addr}; fi;", 0);
	}
	PUSH_TIME_TE("load dtb", BL33_LOAD_DTB_e);
	run_command("if fdt addr ${dtb_mem_addr}; then else echo no dtb at ${dtb_mem_addr};fi;", 0);
	/* load unifykey */
	run_command("keyman init 0x1234", 0);
#endif//#ifndef CONFIG_SYSTEM_RTOS //prue rtos not need dtb

	return 0;
}

#if !defined(SYSCTRL_SEC_STICKY_REG2) && defined(P_PREG_STICKY_REG2)
#define SYSCTRL_SEC_STICKY_REG2 P_PREG_STICKY_REG2
#endif//#if !defined(SYSCTRL_SEC_STICKY_REG2) && defined(P_PREG_STICKY_REG2)
int aml_board_late_init_tail(void *arg)
{
	unsigned char chipid[16];
	char con_str[128], env_str[128];
	int con_len = 0;

	PUSH_TIME_TE("tail init", BL33_TAIL_INIT_s);
	UNUSED(arg);
	printf("init tail\n");
	run_command("amlsecurecheck", 0);
	run_command("update_tries", 0);

	//Need save outputmode/connector_type to flash if changed after display drv init
	if (env_get("outputmode"))
		con_len = sprintf(con_str, "outputmode");
	if (env_get("connector0_type"))
		con_len += sprintf(con_str + con_len, " connector0_type");
	if (env_get("connector1_type"))
		con_len += sprintf(con_str + con_len, " connector1_type");
	if (env_get("connector2_type"))
		con_len += sprintf(con_str + con_len, " connector2_type");
	if (con_len) {
		sprintf(env_str, "printenv %s", con_str);
		run_command(env_str, 0);
		sprintf(env_str, "update_env_part -p -f %s", con_str);
		run_command(env_str, 0);
	}

	memset(chipid, 0, 16);
	env_set("cpu_id", "1234567890");
	if (get_chip_id(chipid, 16) != -1) {
		char chipid_str[32];
		char chipid_env[32];
		int i, j;
		char buf_tmp[4];

		memset(chipid_str, 0, 32);
		memset(chipid_env, 0, 32);

		char *buff = &chipid_str[0];
		char *buff2 = &chipid_env[0];

		for (i = 0, j = 0; i < 12; ++i) {
			sprintf(&buf_tmp[0], "%02x", chipid[15 - i]);
			if (strcmp(buf_tmp, "00") != 0) {
				sprintf(buff + j, "%02x", chipid[15 - i]);
				j = j + 2;
			}
		}

		for (i = 0, j = 0; i < 12; ++i) {
			sprintf(buff2 + j, "%02x", chipid[15 - i]);
			j = j + 2;
		}

		env_set("cpuid_env", chipid_env);
		env_set("cpu_id", chipid_str);
		printf("buff: %s\n", buff);
	}

//auto enter usb mode after board_late_init if 'adnl.exe setvar burnsteps 0x1b8ec003'
#if defined(CONFIG_AML_V3_FACTORY_BURN) && defined(CONFIG_AML_V3_USB_TOOl)
	if (readl(SYSCTRL_SEC_STICKY_REG2) == 0x1b8ec003)
		aml_v3_factory_usb_burning(0, gd->bd);
#endif//#if defined(CONFIG_AML_V3_FACTORY_BURN) && defined(CONFIG_AML_V3_USB_TOOl)
#ifdef CONFIG_AML_FACTORY_BURN_LOCAL_UPGRADE //try auto upgrade from ext-sdcard
	aml_try_factory_sdcard_burning(0, gd->bd);
#endif//#ifdef CONFIG_AML_FACTORY_BURN_LOCAL_UPGRADE
	PUSH_TIME_TE("tail init", BL33_TAIL_INIT_e);

	return 0;
}
#endif//#ifdef CONFIG_BOARD_LATE_INIT

#ifdef CONFIG_MODEL_DETECT
#ifdef CONFIG_CMD_AML_MODEL
static int board_model_det_gpio_get_val(char *str_gpio)
{
	unsigned int val;
	int gpio, ret;

#if defined(CONFIG_DM_GPIO)
	ret = gpio_lookup_name(str_gpio, NULL, NULL, (unsigned int *)&gpio);
	if (ret) {
		printf("GPIO: '%s' not found\n", str_gpio);
		return -1;
	}
#else
	/* turn the gpio name into a gpio number */
	gpio = name_to_gpio(str_gpio);
	if (gpio < 0) {
		printf("GPIO: '%s' not found\n", str_gpio);
		return -1;
	}
#endif
	/* grab the pin before we tweak it */
	ret = gpio_request(gpio, "board_model_detect");
	if (ret && ret != -EBUSY) {
		printf("gpio: requesting pin %u failed\n", gpio);
		return -1;
	}
	gpio_direction_input(gpio);
	val = gpio_get_value(gpio);
	printf("gpio: pin %s (gpio %u) value is ", str_gpio, gpio);
	if (IS_ERR_VALUE(val)) {
		printf("unknown (ret=%d)\n", val);
		return -1;
	}
	printf("%d\n", val);
	gpio_free(gpio);

	return val;
}

static int board_model_det_saradc_get_val(int saradc_ch, int level_max, unsigned int *level_table)
{
	char cmd_str[32];
	unsigned int val;
	int i, level = -1;

	snprintf(cmd_str, 32, "saradc open %d", saradc_ch);
	run_command(cmd_str, 0);
	run_command("saradc getval", 0);
	val = env_get_ulong("saradc_val", 16, 0);

	for (i = 0; i < level_max; i++) {
		if (val >= level_table[i * 2] && val <= level_table[i * 2 + 1]) {
			level = i;
			break;
		}
	}
	printf("board_model_det: saradc_val: %d, level: %d\n", val, level);
	return level;
}

#define DEFAULT_MODEL_DET_PATH      "/vendor/etc/tvconfig/model/model_det.ini"
static void board_model_detect(void)
{
	char *model_path, model_det_env[32], key_str[32], det_key_str[32];
	void *local_ini_mem, *psec;
	const char *str = NULL;
	unsigned int val, det_en, adc_ch, level_max, *level_table, table_size;
	int i, ret;

	env_set("model0_name_factory", "none");
	env_set("model0_name_alternate", "none");
	model_path = env_get("model_det_path");
	if (!model_path)
		model_path = DEFAULT_MODEL_DET_PATH;

	//printf("%s: %s\n", __func__, model_path);

	//load model_detection file
	local_ini_mem = handle_ini_file_parse(model_path, 0);
	if (!local_ini_mem) {
		printf("error: %s: %s\n", __func__, model_path);
		return;
	}

	psec = handle_ini_get_section(local_ini_mem, "MODEL_DETECT_CONFIG");
	if (!psec) {
		printf("error: %s: section error\n", __func__);
		goto board_model_detect_exit;
	}

	det_en = handle_ini_get_val(local_ini_mem, psec, "detect_en", 0);
	if (det_en == 0)
		goto board_model_detect_exit;

	switch (det_en) {
	case 2:
		snprintf(model_det_env, 32, "model0_name_alternate");
		break;
	default:
		snprintf(model_det_env, 32, "model0_name_factory");
		break;
	}

	//model detect
	str = handle_ini_get_str(local_ini_mem, psec, "detect_mothod", "null");
	if (strcmp(str, "null") == 0)
		goto board_model_detect_exit;
	if (strcmp(str, "GPIO") == 0) {
		str = handle_ini_get_str(local_ini_mem, psec, "detect_gpio_pin", "null");
		if (strcmp(str, "null") == 0)
			goto board_model_detect_exit;

		ret = board_model_det_gpio_get_val(str);
		if (ret < 0)
			goto board_model_detect_exit;

		snprintf(det_key_str, 32, "detect_model_name_%d", ret);
		str = handle_ini_get_str(local_ini_mem, psec, det_key_str, "none");
	} else if (strcmp(str, "SARADC") == 0) {
		adc_ch = handle_ini_get_val(local_ini_mem, psec, "detect_saradc_ch", 0xff);
		if (adc_ch == 0xff)
			goto board_model_detect_exit;

		level_max = handle_ini_get_val(local_ini_mem, psec,
					       "detect_saradc_level_max", 0xffff);
		if (level_max == 0xffff)
			goto board_model_detect_exit;

		table_size = sizeof(unsigned int) * level_max * 2; //min~max
		level_table = (unsigned int *)malloc(table_size);
		if (!level_table)
			goto board_model_detect_exit;

		for (i = 0; i < level_max; i++) {
			snprintf(key_str, 32, "detect_saradc_level_%d_min", i);
			val = handle_ini_get_val(local_ini_mem, psec, key_str, 0xffffffff);
			if (val == 0xffffffff) {
				memset(level_table, 0, table_size);
				free(level_table);
				goto board_model_detect_exit;
			}
			level_table[i * 2] = val;

			snprintf(key_str, 32, "detect_saradc_level_%d_max", i);
			val = handle_ini_get_val(local_ini_mem, psec, key_str, 0xffffffff);
			if (val == 0xffffffff) {
				memset(level_table, 0, table_size);
				free(level_table);
				goto board_model_detect_exit;
			}
			level_table[i * 2 + 1] = val;
		}

		ret = board_model_det_saradc_get_val(adc_ch, level_max, level_table);
		snprintf(det_key_str, 32, "detect_model_name_%d", ret);
		str = handle_ini_get_str(local_ini_mem, psec, det_key_str, "none");
	}
	//update env
	env_set(model_det_env, str);
	printf("model_detect: en:%d, %s=%s\n", det_en, model_det_env, str);

board_model_detect_exit:
	handle_ini_parser_uninit(local_ini_mem);
}
#endif
#endif

void aml_board_display_env_handler(void)
{
	char *bootup_display;
#ifdef CONFIG_AML_LCD
	char lcd_init_str[8];
#endif

	run_command("get_rebootmode", 0);
	printf("reboot_mode: %s\n", env_get("reboot_mode"));
	run_command("run check_display", 0);
	bootup_display = env_get("bootup_display");
	if (!bootup_display)
		return;

	printf("bootup_display: %s\n", bootup_display);
#ifdef CONFIG_AML_LCD
	if (strcmp(bootup_display, "on") == 0)
		snprintf(lcd_init_str, 8, "%d", LCD_INIT_LEVEL_NORMAL);
	else if (strcmp(bootup_display, "preboot") == 0)
		snprintf(lcd_init_str, 8, "%d", LCD_INIT_LEVEL_PREBOOT);
	else
		snprintf(lcd_init_str, 8, "%d", LCD_INIT_LEVEL_PWR_OFF);

	env_set("lcd_init_level", lcd_init_str);
	//printf("lcd_init_level: %s\n", env_get("lcd_init_level"));
#endif
}

#ifdef CONFIG_AML_VOUT
#ifdef CONFIG_ARMV8_MULTIENTRY
static void aml_display_on_pre(unsigned char vout_bit)
{
	run_command("run init_display", 0);

	printf("display[0x%x] on pre done\n", vout_bit);
}

//param bit[0:7]:curr vout idx;
static void aml_display_on_post_job(unsigned long param)
{
	if ((param & 0xf) == 0)
		aml_vout_output(0, env_get("outputmode"));
	else if ((param & 0xf) == 1)
		aml_vout_output(1, env_get("outputmode2"));
	else if ((param & 0xf) == 2)
		aml_vout_output(2, env_get("outputmode3"));

	printf("display[%lu] on post done\n", param);
	secondary_off();
}
#endif
#endif

// each bit refers to vout index, eg. 0x5=(vout+vout3)
void aml_board_display_init(unsigned char vout_bit)
{
#ifdef CONFIG_AML_LCD
	int ret;
	struct udevice *lcd_dev;
#ifdef CONFIG_AML_LCD_BACKLIGHT
	struct udevice *backlight_dev;
#endif
#endif
#ifdef CONFIG_AML_VPU
	vpu_probe();
#endif
#ifdef CONFIG_AML_VPP
	vpp_init();
#endif
#ifdef CONFIG_RX_RTERM
	rx_set_phy_rterm();
#endif
#ifdef CONFIG_AML_CVBS
	cvbs_init();
#endif
#ifdef CONFIG_CVBS_CALI
	cvbs_dac_cfg();
#endif
#ifdef CONFIG_CEC_TRIM_VAL
	cec_get_trim_val();
#endif

#ifdef CONFIG_MODEL_DETECT
#ifdef CONFIG_CMD_AML_MODEL
	board_model_detect();
#endif
#endif

	run_command("ini_model", 0);
	aml_board_display_env_handler();
#ifdef CONFIG_AML_LCD
	ret = uclass_get_device_by_name(UCLASS_MISC, "lcd", &lcd_dev);
	if (ret)
		printf("device lcd not found\n");
#ifdef CONFIG_AML_LCD_BACKLIGHT
	ret = uclass_get_device_by_name(UCLASS_MISC, "backlight", &backlight_dev);
	if (ret)
		printf("device backlight not found\n");
#endif
#endif

#ifdef CONFIG_AML_VOUT
#ifdef CONFIG_ARMV8_MULTIENTRY
	int smp_ret = 0;
	unsigned char cpu_id = 1;

	if (!(unsigned char)env_get_ulong("display_on_smp", 10, 0))
		return;

	aml_display_on_pre(vout_bit);
	if (env_get("bootup_display") && (strcmp(env_get("bootup_display"), "on") == 0)) {
		if (vout_bit & BIT(0)) {
			smp_ret = run_smp_function(cpu_id, &aml_display_on_post_job, 0);
			if (smp_ret) {
				printf("display smp failed\n");
			} else {
				run_command("setenv vout_init enable", 0);
				cpu_id++;
			}
		}
		if (vout_bit & BIT(1)) {
			smp_ret = run_smp_function(cpu_id, &aml_display_on_post_job, 1);
			if (smp_ret) {
				printf("display2 smp failed\n");
			} else {
				run_command("setenv vout2_init enable", 0);
				cpu_id++;
			}

			cpu_id++;
		}
		if (vout_bit & BIT(2)) {
			smp_ret = run_smp_function(cpu_id, &aml_display_on_post_job, 2);
			if (smp_ret) {
				printf("display3 smp failed\n");
			} else {
				run_command("setenv vout2_init enable", 0);
				cpu_id++;
			}
		}
	}
#endif
#endif
}

#ifdef CONFIG_BOARD_RNG_SEED
unsigned int random(void)
{
	volatile unsigned int val;

#ifdef CONFIG_HW_RNG_OLD
	val = readl(RNG_USR_DATA);
#else
	do {} while (readl(RNG_REE_READY) & 0x1);
	do {} while (readl(RNG_REE_CFG) & 0x1);
	writel(readl(RNG_REE_CFG) | (1 << 31), RNG_REE_CFG);
	do {} while (readl(RNG_REE_CFG) >> 31);

	val = readl(RNG_REE_OUT0);
	writel(0x1, RNG_REE_READY);
#endif
	return val;
}

void get_rng_hw(unsigned char *buf, unsigned int size)
{
	unsigned int *p = (unsigned int *)buf;
	unsigned int cnt, ncnt;
	unsigned int i;
	unsigned int rng;

	cnt = (size >> 2);
	ncnt = size & 0x3;

	for (i = 0; i < cnt; i++)
		p[i] = random();

	if (ncnt) {
		rng = random();
		for (i = 0; i < ncnt; i++)
			buf[cnt * 4 + i] = ((rng >> (i * 8)) & 0xff);
	}
}

int board_rng_seed(struct abuf *buf)
{
	abuf_init(buf);
	abuf_realloc(buf, 128);
	if (buf->data)
		get_rng_hw(buf->data, buf->size);

	return 0;
}
#endif

#ifdef CONFIG_AML_DEFENV
const char * const _aml_env_reserv_array[] = {
	"lock",
	"upgrade_step",
	"bootloader_version",
	"dts_to_gpt",
	"fastboot_step",
	"reboot_status",
	"expect_index",
	"recovery_check_part",
	"rollback_reason",
	"defenv_para",	//set in board_late_init
	NULL//Keep NULL be last to tell END
};
#endif//#ifdef CONFIG_AML_DEFENV

