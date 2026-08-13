// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include "../v2_sdc_burn/optimus_sdc_burn_i.h"
#include "../v2_sdc_burn/optimus_led.h"
#include <amlogic/storage.h>

extern int optimus_burn_with_cfg_file(const char *cfgFile);
#define _UPGRADE_DISPLAY_PREPARE "echo upgrade display prepare 22;" \
	"setenv display_on_smp 0; setenv fb_for_4k2k 0;" \
	"setenv jpg_logo false; setenv display_layer osd0;" \
	"setenv display_bpp 16; setenv display_color_index 16;"

// added by scy
int optimus_burn_package_in_usb(const char *sdc_cfg_file)
{
	int rcode = 0;

	DWN_MSG("usb start\n");
	rcode = run_command("usb start", 0);
	if (rcode) {
		DWN_ERR("Fail in init usb host, Does usb host not plugged in?\n");
		return __LINE__;
	}

#if 1//this asserted by 'run update' and 'aml_check_is_ready_for_sdc_produce'
#ifdef CONFIG_FS_EXFAT
	optimus_exfat_register_device("usb", "0");
	if (_exfatok > 0) {
		rcode = do_exfat_get_fileSz(sdc_cfg_file);
	} else {
		rcode = do_fat_get_fileSz(sdc_cfg_file);
	}
#else
	rcode = do_fat_get_fileSz(sdc_cfg_file);
#endif

	if (!rcode) {
		DWN_ERR("The [%s] not exist in udisk\n", sdc_cfg_file);
		return __LINE__;
	}
#endif//#if 0

#ifdef CONFIG_FS_EXFAT
	if (_exfatok <= 0)
#endif
	{
		rcode = optimus_device_probe("usb", "0");
		if (rcode) {
			DWN_ERR("Fail to detect device usb 0\n");
			return __LINE__;
		}
	}

	rcode = optimus_burn_with_cfg_file(sdc_cfg_file);

	return rcode;
}


// added by scy
int do_usb_burn(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{

	int rcode = 0;
	const char *sdc_cfg_file = argv[1];

	if (argc < 2) {
		cmd_usage(cmdtp);
		return __LINE__;
	}

	setenv("usb_update", "1");
	optimus_work_mode_set(OPTIMUS_WORK_MODE_UDISK_PRODUCE);
	run_command("osd clear", 0);
	if (!env_get("upgrade_display_prepare"))
		env_set("upgrade_display_prepare", _UPGRADE_DISPLAY_PREPARE);
	run_command("printenv upgrade_display_prepare; run upgrade_display_prepare;", 0);
	show_logo_to_report_burning();//indicate enter flow of burning! when 'run update'
	if (optimus_led_open(LED_TYPE_PWM)) {
		DWN_ERR("Fail to open led for burn\n");
		return __LINE__;
	}
	optimus_led_show_in_process_of_burning();

	rcode = optimus_burn_package_in_usb(sdc_cfg_file);

	return rcode;
}

// added by scy
U_BOOT_CMD(
   usb_burn,      //command name
   5,               //maxargs
   0,               //repeatable
   do_usb_burn,   //command function
   "Burning with amlogic format package in usb ",           //description
   "argv: [sdc_burn_cfg_file]\n"//usage
   "    -aml_sdc_burn.ini is usually used configure file\n"
);
