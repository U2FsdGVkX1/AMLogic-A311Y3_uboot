/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef _PWRC_H_
#define _PWRC_H_

#include <stdbool.h>

#define PWR_ON    1
#define PWR_OFF   0

#define PDID_EPP	        0
#define PDID_PCIEA	        1
#define PDID_PCIEB	        2
#define PDID_PCIEC	        3
#define PDID_U3HSG_PHY	   	4
#define PDID_ETH_P2_HSGMII	5
#define PDID_ETH_P2G1	    	6
#define PDID_ETH_P2G2	    	7
#define PDID_ETH_P3G1	    	8
#define PDID_ETH_P3G2	    	9
#define PDID_USB	        10
#define PDID_EMMC	        11

#define PDID_ETH                PM_MAX
#define PM_ETH			PDID_ETH

#define PM_MAX                 12

unsigned long viu_init_psci_smc(void);
unsigned long pwr_ctrl_psci_smc(unsigned int power_domain, bool power_control);
unsigned long pwr_ctrl_status_psci_smc(unsigned int power_domain);
#endif
