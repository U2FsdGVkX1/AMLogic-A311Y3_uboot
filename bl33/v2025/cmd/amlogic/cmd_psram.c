// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * PSRAM test command for C5 U-Boot.
 * psram_reg.h + psram.h + bl2 driver inlined.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <command.h>
#include <vsprintf.h>

static void wr_reg(uint32_t base_add, uint32_t dat)
{
	*(volatile uint32_t *)(((uintptr_t)(0) << 2) + base_add) = (dat);
}

static uint32_t rd_reg(uint32_t base_add)
{
	return *(volatile uint32_t *)(((uintptr_t)(0) << 2) + base_add);
}

static void psram_uboot_udelay(unsigned int us)
{
	udelay(us);
}

static void psram_uboot_watchdog_disable(void)
{
}

#define _udelay(us)			psram_uboot_udelay(us)
#define watchdog_disable()		psram_uboot_watchdog_disable()

static void psram_serial_puts(const char *s) { printf("%s", s); }
static void psram_serial_put_dec(unsigned int v) { printf("%u", v); }
static void psram_serial_put_hex(unsigned int v, int bits) { (void)bits; printf("0x%x", v); }
static void psram_bl2_print(const char *prefix, unsigned int val, int unused, const char *suffix)
{
	(void)unused;
	printf("%s%u%s", prefix, val, suffix);
}
#define serial_puts(s)		psram_serial_puts(s)
#define serial_put_dec(v)	psram_serial_put_dec(v)
#define serial_put_hex(v,b)	psram_serial_put_hex(v,b)
#define bl2_print(p,v,u,s)	psram_bl2_print(p,v,u,s)

/* ---------- psram_reg.h ---------- */
/*
* Copyright (c) 2025 Amlogic, Inc. All rights reserved.
*
* This source code is subject to the terms and conditions defined in the
* file 'LICENSE' which is part of this source code package.
*/
// Reading file:  ../psram/rtl/psram_reg.vh

//
// -----------------------------------------------
// APB_BASE:  APB0_BASE_ADDR = 0xfe007400
// -----------------------------------------------
//64MB addr: 0xf400_0000 ~ 0xf7ff_ffff

#define PADCTRL_GPIOP_I                            ((0x0020  << 2) + 0xffa04000)
#define PADCTRL_GPIOP_O                            ((0x0021  << 2) + 0xffa04000)
#define PADCTRL_GPIOP_OEN                          ((0x0022  << 2) + 0xffa04000)

#define PADCTRL_PIN_MUX_REGAO_5                    ((0x0005  << 2) + 0xffa04000)
#define PADCTRL_PIN_MUX_REGAO_6                    ((0x0006  << 2) + 0xffa04000)

#define PADCTRL_GPIOP_DS                           ((0x0027  << 2) + 0xffa04000)
#define AO_CLKCTRL_PSRAM_CLK_CTRL                  ((0x0016  << 2) + 0xffa00000)

#define AO_RESETCTRL_RESET0                        ((0x0000  << 2) + 0xffa02000)
#define AO_PWRCTRL_FOCRST0                         ((0x0003  << 2) + 0xffa0c000)
#define PWRCTRL_FOCRST0                            ((0x000c  << 2) + 0xfe00c000)
#define PWRCTRL_FOCRST1                            ((0x000d  << 2) + 0xfe00c000)

#define P_PWRCTRL_FOCRST0                          (volatile uint32_t *)0xfe00c030
#define P_PWRCTRL_FOCRST1                          (volatile uint32_t *)0xfe00c034
#define P_PWRCTRL_MEM_PD0                          (volatile uint32_t *)0xfe00c040
#define P_PWRCTRL_MEM_PD1                          (volatile uint32_t *)0xfe00c044
#define P_PWRCTRL_MEM_PD2                          (volatile uint32_t *)0xfe00c048
#define P_PWRCTRL_MEM_PD3                          (volatile uint32_t *)0xfe00c04c
#define P_PWRCTRL_MEM_PD4                          (volatile uint32_t *)0xfe00c050
#define P_PWRCTRL_MEM_PD5                          (volatile uint32_t *)0xfe00c054
#define P_PWRCTRL_MEM_PD6                          (volatile uint32_t *)0xfe00c058
#define P_PWRCTRL_MEM_PD7                          (volatile uint32_t *)0xfe00c05c
#define P_PWRCTRL_MEM_PD8                          (volatile uint32_t *)0xfe00c060
#define P_PWRCTRL_MEM_PD9                          (volatile uint32_t *)0xfe00c064
#define P_PWRCTRL_MEM_PD10                         (volatile uint32_t *)0xfe00c068
#define P_PWRCTRL_MEM_PD11                         (volatile uint32_t *)0xfe00c06c
#define P_PWRCTRL_MEM_PD12                         (volatile uint32_t *)0xfe00c070
#define P_PWRCTRL_MEM_PD13                         (volatile uint32_t *)0xfe00c074
#define P_PWRCTRL_MEM_PD14                         (volatile uint32_t *)0xfe00c078
#define P_PWRCTRL_MEM_PD15                         (volatile uint32_t *)0xfe00c07c
#define P_PWRCTRL_MEM_PD16                         (volatile uint32_t *)0xfe00c080
#define P_PWRCTRL_MEM_PD17                         (volatile uint32_t *)0xfe00c084
#define P_PWRCTRL_MEM_PD18                         (volatile uint32_t *)0xfe00c088
#define P_PWRCTRL_MEM_PD19                         (volatile uint32_t *)0xfe00c08c
#define P_PWRCTRL_MEM_PD20                         (volatile uint32_t *)0xfe00c090
#define P_PWRCTRL_MEM_PD21                         (volatile uint32_t *)0xfe00c094
#define P_PWRCTRL_MEM_PD22                         (volatile uint32_t *)0xfe00c098
#define P_PWRCTRL_MEM_PD23                         (volatile uint32_t *)0xfe00c09c
#define P_PWRCTRL_MEM_PD24                         (volatile uint32_t *)0xfe00c0a0
#define P_PWRCTRL_MEM_PD25                         (volatile uint32_t *)0xfe00c0a4
#define P_PWRCTRL_MEM_PD26                         (volatile uint32_t *)0xfe00c0a8

#define P_AO_PWRCTRL_FOCRST0                       (volatile uint32_t *)0xffa0c00c
#define P_AO_PWRCTRL_MEM_PD0                       (volatile uint32_t *)0xffa0c034


#define PSRAM_AXI_INTF_CTRL                        ((0x0000  << 2) + 0xffa18000)
 //bit 4   axi interface psram clock domain soft reset.      1 = reset AXI interface.  0 = normal.
 //bit 3   axi interface axi clock domain soft reset.      1 = reset AXI interface.  0 = normal.
 //bit 2   disable axi interface clock.    1 = disable; 0 = enable.
 //bit 1   axi interface auto clock gating enable. 1 = enable; 0 = disable.
 //bit 0   axi interface AXI request enable.  1 = enable. 0 = disable.
#define PSRAM_DBUF_CTRL                            ((0x0001  << 2) + 0xffa18000)
 //bit 19.    DBUF clock disable.   1: disable dbuf clock.  0: normal working mode.
 //bit 18.    DBUF auto clock gating disable.  1: disable auto clock gating feature.  0: use hardware auto clock gating to save power.
 //bit 17.    DBUF  soft reset.   1: reset DBUF. 0: normal working mode.
 //bit 16.    MWRITE_EN.  1 : enable mask write(with DM pin). 0 not enable.
 //bit 15:0   DBUF AGE to write back to PSRAM if DBUF is dirty. but whole DBUF data is not READ to write.(with MWRITE enabled).
#define PSRAM_DBUF_CTRL1                           ((0x0002  << 2) + 0xffa18000)
  //bit 15:0  DBUF AGE to write back to psram if DBUF is dirty and whole DBUF data is ready to write.
#define PSRAM_APB_CTRL                             ((0x0003  << 2) + 0xffa18000)
  //7:4      PSRAM PHY register APB secure contrl.
			 //bit 7,  1: no secure control.  0 : use bit 4 to match PPROT[0] bit.
			 //bit 4.  when bit 7 == 0, bit 4 must match PPROT[0] to access.
  //3:0      PCTL_CTRL APB bus control registers secure control.
			 //bit 3,  1: no secure control.  0 : use bit 0 to match PPROT[0] bit.
			 //bit 0.  when bit 3 == 0, bit 0 must match PPROT[0] to access.
#define PSRAM_SEC_CTRL                             ((0x0004  << 2) + 0xffa18000)
   //bit 31.  ADDRESS security range enable.  1 = enable; 0: disable.
			//if scruity range disabled, but AXI data describe enabled key0 is selected.
   //bit 23   range 7  des key selection 0 : key0;  1: key1;
   //bit 22   range 6  des key selection 0 : key0;  1: key1;
   //bit 21   range 5  des key selection 0 : key0;  1: key1;
   //bit 20   range 4  des key selection 0 : key0;  1: key1;
   //bit 19   range 3  des key selection 0 : key0;  1: key1;
   //bit 18   range 2  des key selection 0 : key0;  1: key1;
   //bit 17   range 1  des key selection 0 : key0;  1: key1;
   //bit 16   range0  des key selection 0 : key0;  1: key1;
   //bit 6.   range6 enable:   1: enable; 0 : disable.
   //bit 5.   range5 enable:   1: enable; 0 : disable.
   //bit 4.   range4 enable:   1: enable; 0 : disable.
   //bit 3.   range3 enable:   1: enable; 0 : disable.
   //bit 2.   range2 enable:   1: enable; 0 : disable.
   //bit 1.   range1 enable:   1: enable; 0 : disable.
   //bit 0.   range0 enable:   1: enable; 0 : disable.
#define PSRAM_DES_PADDING                          ((0x0005  << 2) + 0xffa18000)
   //bit 31 :0 Padding, with address together to generate describe 64 bits input.
#define PSRAM_RANGE0_STA                           ((0x0010  << 2) + 0xffa18000)
  //bit 22 :0.
  //AXI security range 0  start address in unit of 64Byte. related to HADDR bit 28:6.
#define PSRAM_RANGE0_EDA                           ((0x0011  << 2) + 0xffa18000)
  //bit 22 :0.
  //AXI security range 0  end address in unit of 64Byte. related to HADDR bit 28:6.
#define PSRAM_RANGE0_CTRL0                         ((0x0012  << 2) + 0xffa18000)
  // AXI security range 0 control.
#define PSRAM_RANGE0_CTRL1                         ((0x0013  << 2) + 0xffa18000)
  // AXI security range 0 control.
#define PSRAM_RANGE1_STA                           ((0x0014  << 2) + 0xffa18000)
  //bit 22 :0.
  //AXI security range 1  start address in unit of 64Byte. related to HADDR bit 28:6.
#define PSRAM_RANGE1_EDA                           ((0x0015  << 2) + 0xffa18000)
  //bit 22 :0.
  //AXI security range 1  end address in unit of 64Byte. related to HADDR bit 28:6.
#define PSRAM_RANGE1_CTRL0                         ((0x0016  << 2) + 0xffa18000)
  // AXI security range 1 control.
#define PSRAM_RANGE1_CTRL1                         ((0x0017  << 2) + 0xffa18000)
  // AXI security range 1 control.
#define PSRAM_RANGE2_STA                           ((0x0018  << 2) + 0xffa18000)
  //bit 31 : 6.
  //AXI security range 2  start address in unit of 64Byte. related to HADDR bit 28:6.
#define PSRAM_RANGE2_EDA                           ((0x0019  << 2) + 0xffa18000)
  //bit 31 :6.
  //AXI security range 2  end address in unit of 64Byte. related to HADDR bit 28:6.
#define PSRAM_RANGE2_CTRL0                         ((0x001a  << 2) + 0xffa18000)
#define PSRAM_RANGE2_CTRL1                         ((0x001b  << 2) + 0xffa18000)
#define PSRAM_RANGE3_STA                           ((0x001c  << 2) + 0xffa18000)
  //bit 31 : 6.
  //AXI security range 3  start address in unit of 64Byte. related to HADDR bit 28:6.
#define PSRAM_RANGE3_EDA                           ((0x001d  << 2) + 0xffa18000)
  //bit 31 :6.
  //AXI security range 3  end address in unit of 64Byte. related to HADDR bit 28:6.
#define PSRAM_RANGE3_CTRL0                         ((0x001e  << 2) + 0xffa18000)
  // AXI security range 3 control.
#define PSRAM_RANGE3_CTRL1                         ((0x001f  << 2) + 0xffa18000)
  // AXI security range 3 control.
#define PSRAM_RANGE4_STA                           ((0x0020  << 2) + 0xffa18000)
  //bit 31 :6.
  //AXI security range 4  start address in unit of 64Byte. related to HADDR bit 28:6.
#define PSRAM_RANGE4_EDA                           ((0x0021  << 2) + 0xffa18000)
  //bit 31 :6.
  //AXI security range 4  end address in unit of 64Byte. related to HADDR bit 28:6.
#define PSRAM_RANGE4_CTRL0                         ((0x0022  << 2) + 0xffa18000)
  // AXI security range 4 control.
#define PSRAM_RANGE4_CTRL1                         ((0x0023  << 2) + 0xffa18000)
#define PSRAM_RANGE5_STA                           ((0x0024  << 2) + 0xffa18000)
  //bit 22 :0.
  //AXI security range 5 start address in unit of 64Byte. related to HADDR bit 28:6.
#define PSRAM_RANGE5_EDA                           ((0x0025  << 2) + 0xffa18000)
  //bit 22 :0.
  //AXI security range 5  end address in unit of 64Byte. related to HADDR bit 28:6.
#define PSRAM_RANGE5_CTRL0                         ((0x0026  << 2) + 0xffa18000)
  // AXI security range 5 control.
#define PSRAM_RANGE5_CTRL1                         ((0x0027  << 2) + 0xffa18000)
  // AXI security range 5 control.
#define PSRAM_RANGE6_STA                           ((0x0028  << 2) + 0xffa18000)
  //bit 22 :0.
  //AXI security range 6 start address in unit of 64Byte. related to HADDR bit 28:6.
#define PSRAM_RANGE6_EDA                           ((0x0029  << 2) + 0xffa18000)
  //bit 22 :0.
  //AXI security range 6  end address in unit of 64Byte. related to HADDR bit 28:6.
#define PSRAM_RANGE6_CTRL0                         ((0x002a  << 2) + 0xffa18000)
  // AXI security range 6 control.
#define PSRAM_RANGE6_CTRL1                         ((0x002b  << 2) + 0xffa18000)
  // AXI security range 6 control.
#define PSRAM_RANGE7_CTRL0                         ((0x002c  << 2) + 0xffa18000)
#define PSRAM_RANGE7_CTRL1                         ((0x002d  << 2) + 0xffa18000)
#define PSRAM_VIO_STATUS                           ((0x002e  << 2) + 0xffa18000)
  //bit 31:  security violation.  write 1 to clean security violation status.
  //bit 30.   1 : AXI write violation.  0 : axi read violation.
  //bit 18:16.  AXI ARPROT/AWPROT>
  //bit 11:0.   AXI violation ID.
#define PSRAM_VIO_ADDR                             ((0x002f  << 2) + 0xffa18000)
  //read only
  //31:0  AXI violation address.
#define PSRAM_USER_CTRL0                           ((0x0080  << 2) + 0xffa18000)
  //bit 31.    user request enable.  write 1 to enable.  read 0, means user command accepted by the SPI_CTRL.
  //bit 30.    usr request done.     after user command done. this bit set to 1.
  //bit 29.    USR COMMAND enable.
  //bit 28.    command type0. 1 READ command.  0 write command.
  //bit 27.    command type1. 1 for register.  0 : for memory.
  //bit 21~16. USR  command clock cycles
  //bit 15~0.  USR COMMAND CODE.
#define PSRAM_USER_CTRL1                           ((0x0081  << 2) + 0xffa18000)
  //bit 31.    USR_ADDR_EN
  //bit 30:29. USR_ADDR_DW
  //bit 28~20. USR address clock cycle number.
  //bit 19:16  USR data output/input switch position
  //bit 15.    USR DUMMY ENABLE
  //bit 13~8.  USR DUMMY clock cycles.
  //bit 5:0    USR data  DQS read enable position.
#define PSRAM_USER_CTRL2                           ((0x0082  << 2) + 0xffa18000)
  //bit 31  usr des key selection. 1 : key 1; 0 : key 0.
  //bit 30. usr check latency. for Winbond PSRAM data read/write, DQS input identify the latency is doubled or not.
  //bit 29. usr data des enable.
  //bit 28   usr datain enable.
  //bit 27.  usr datain destination. 1 : write to CFG_STS register. 0 : write to DATA REGISTERs.
  //bit 26~16  usr data in clock cycle number.
  //bit 15.   usr datout enable.
  //bit 14.  usr data out source  1 : from CFG_STS register. 0 from DATA register.
  //bit 13~11. not used.
  //bit 10~0.  usr dataout clock cycles.
#define PSRAM_USER_CTRL3                           ((0x0083  << 2) + 0xffa18000)
  //bit 17:16. which cycle to ask phy to check read/write latency at PSRAM SEND COMMAND stage.
			 //since PSRAM send command stage only 3 clock cycle.  So this number only can chose 0, 1, 2.
  //bit 15:0.  user dummy data when output in dummy cycle.
#define PSRAM_USER_ADDR                            ((0x0084  << 2) + 0xffa18000)
  //bit 31:0.  32bits user address.
#define PSRAM_AXI_REQ_CTRL0                        ((0x0085  << 2) + 0xffa18000)
 //bit 31.    AXI request enable.
 //bit 30     AXI cmd_en.  command cycle enable.
 //bit 29:24. AXI request command cycle clock numbers.
 //bit 22.    AXI request address cycle enable.
 //bit 21:16. AXI request address cycle clock number.
 //bit 15:14. AXI address cycle data width.
 //bit 13:12. at which clock cycle to send signal to phy to check latency flag for WINBOND PSRAM flexible latency.
			 //since PSRAM send command stage only 3 clock cycle.  So this number only can chose 0, 1, 2.
  //bit 6     // AXI Write dummy enable.
  //bit 5:0  // axi write dummy clock cycle number.
#define PSRAM_AXI_REQ_CTRL1                        ((0x0086  << 2) + 0xffa18000)
  //bit 31:16. AXI request psram READ command code.
  //bit 15:0.  AXI request psram write command code.
#define PSRAM_AXI_REQ_CTRL2                        ((0x0087  << 2) + 0xffa18000)
  //bit 31.  AXI  read request Dummy enable.
  //bit 30.  Check latency enable for WINBOND PSRAM flexible latency.
  //bit 27:22.  clock cycles in dummy stage to enable PSRAM PHY reading logic.
  //bit 21:16.  axi_read dummy clock cycles number.
  //bit 15:0.   dummy data if output in dummy cycles.
#define PSRAM_AXI_REQ_CTRL3                        ((0x0088  << 2) + 0xffa18000)
  //bit 31      des enable.
  //bit 26:16.  DATA input clock cycles.
  //bit 10:0.   DATA output clock cycles.
#define PSRAM_ACTIMING0                            ((0x0089  << 2) + 0xffa18000)
  //bit 31:30.  tSLCH
  //bit 29:28   tCLSH
  //bit 20:16   tSHWL
  //bit 15:12   tSHSL2
  //bit 11:8    tSHSL1
  //bit 7:0     tWHSL
#define PSRAM_ACTIMING1                            ((0x008a  << 2) + 0xffa18000)
  //bit 7:0 tRWR  for winbond PSRAM,  = tRWR -3. Winbond PSRAM include one CS cycle and 2 command cycle.
				  //for APMEMORY  should be same value of tCPH
#define PSRAM_ACTIMING2                            ((0x008b  << 2) + 0xffa18000)
#define PSRAM_WDG_CTRL                             ((0x008c  << 2) + 0xffa18000)
 //bit [31]   1: force to reset PSRAM PCTL and PHY logic. 0: normal.
 //bit [30]   1: automatic reset PSRAM PCTL and PHY logic if watch dog triggered.
 //bit 12:0.  watch dog timer.   for Winbond PSRAM should be 1us for tRC.  for APMEMORY should be 1us for tCEM.
//SPI DATA BUFFER.  There's a total 512KByte + 64byte(organized as 36x128bits) SRAM .
// since each APB access is 32bits. So we use word address as APB read/write address.
// But for SPI side we have to use 128bit boundary. That's the first Program/read SPI with data buffer must start at data buffer 128bit boundary.  that means the PSRAM_USE_DBUF_ADDR last 2 bits must be 0.
#define PSRAM_DBUF_0                               ((0x0090  << 2) + 0xffa18000)
#define PSRAM_DBUF_1                               ((0x0091  << 2) + 0xffa18000)
#define PSRAM_DBUF_2                               ((0x0092  << 2) + 0xffa18000)
#define PSRAM_DBUF_3                               ((0x0093  << 2) + 0xffa18000)
#define PSRAM_DBUF_4                               ((0x0094  << 2) + 0xffa18000)
#define PSRAM_DBUF_5                               ((0x0095  << 2) + 0xffa18000)
#define PSRAM_DBUF_6                               ((0x0096  << 2) + 0xffa18000)
#define PSRAM_DBUF_7                               ((0x0097  << 2) + 0xffa18000)
#define PSRAM_DBUF_8                               ((0x0098  << 2) + 0xffa18000)
#define PSRAM_DBUF_9                               ((0x0099  << 2) + 0xffa18000)
#define PSRAM_DBUF_A                               ((0x009a  << 2) + 0xffa18000)
#define PSRAM_DBUF_B                               ((0x009b  << 2) + 0xffa18000)
#define PSRAM_DBUF_C                               ((0x009c  << 2) + 0xffa18000)
#define PSRAM_DBUF_D                               ((0x009d  << 2) + 0xffa18000)
#define PSRAM_DBUF_E                               ((0x009e  << 2) + 0xffa18000)
#define PSRAM_DBUF_F                               ((0x009f  << 2) + 0xffa18000)
#define PSRAM_CFG_STS                              ((0x00a0  << 2) + 0xffa18000)
 //32bits register to save the USR read command read back values.
#define PSRAM_STATUS                               ((0x00a1  << 2) + 0xffa18000)
  //bit 31.   usr_req_done flag.  1 : user request done . write 1 to clean.
  //bit 30~11.
  //bit 10:8
  //bit 7:3.
  //bit 2.  axi_data buffer idle bit.  1 : idle. 0 : working.
  //bit 1.  axi interface idle bit.    1 : idle. 0 : working.
  //bit 0.  psram ctrl idle bit.       1 : idle. 0 : working.
#define PSRAM_CTRL                                 ((0x00a2  << 2) + 0xffa18000)
  //bit 31.   PSRAM in DTR mode.
  //bit 30.   psram input data latch  clock select. 1: from clock input pin. 0: from DQS
			  //if use clock input pin as data input latch clock, bit 13:12 should select 01.
  //bit 29:27.  PSRAM mode.
		//'b000 : STR SPI mode.
		//'b001 : STR QPI mode.
		//'b010 : DTR QPI mode.
		//'b011 : DTR OPI FLASH.
		//'b100 : APmemory PSRAM mode.
		//'b101 : Winbond PSRAM mode.
  //bit 26. enable to generation interruption after usr request done.  1: enable.  0: disable.
  //bit 25.  to enable APMEMORY 2 command cycle mode.  1 : enable : 0: 3 command cycles.
  //bit 24.  to enable the psram clock output always enable. 1 : enable. 0: disable.
  //bit 23.  to enable the psram clock output extend mode to cover APMEMORY       tCPI.
  //bit 22.  to disable the CKN output.  1: disable PSRAM CK_N pin output. 0: CK_N working normal.
  //bit 21.  PSRAM DATA IN/OUT high 8bit and low 8 bit endian. 1 : rising edge is low byte. falling edge data is high byte.  0 : rising edge is high byte.  falling edge data is low byte.
  //bit 20.   PSRAM ADDRESS unit.  1: Byte.  0 : word.
			// APMEMORY used byte address.   Winbond use word address.
  //bit 19.  DQSEN generation.   if use dqs to latch input data,  this bit  should be set to 1 to ask DMC generate ENABLE signal in dummy stage. 0: not generate DQS_EN.
  //bit 18.  PSRAM RESET I/O oe_n value.   if need RESET pin.
  //bit 17.  PSRAM RESET I/O output vale.  if need reset pin.
  //bit 16.  force CS output low. for APMEMORY exit DPPD mode.  1:  to force cs output low  : 0 output normal working mode.
  //bit 13:12.  psram data in clock enable selection. 00: use DQSEN.  01: use clkin cnt.
  //bit 11.  DM Disable.  1: disable DM output.   0: with dmoutput.
  //bit 9. psram pctl auto clock gating enable.   1: enable. 0 : disable.
  //bit 8. psram pctl clock disable.              1: disable. 0 : enable.
  //bit 2  PSRAM SPI mode, DQ2 works as  WP(write protection mode).
  //bit 1.  DQ2 WP mode value.
  //bit 0.  PSRAM SPI mode, DQ3 works as HOLD function.
#define PSRAM_PIN_CTRL                             ((0x00a3  << 2) + 0xffa18000)
//bit 30:28  DATA 6 DQ pin selection.
//bit 27:24  DATA 6 DQ pin selection.
//bit 22:20  DATA 5 DQ pin selection.
//bit 18:16  DATA 4 DQ pin selection.
//bit 14:12  DATA 3 DQ pin selection.
//bit 10:8   DATA 2 DQ pin selection.
//bit 6:4    DATA 1 DQ pin selection.
//bit 2:0    DATA 0 DQ pin selection.
   //0 :  from DQ[0]
   //1 :  from DQ[1]
   //2 :  from DQ[2]
   //3 :  from DQ[3]
   //4 :  from DQ[4]
   //5 :  from DQ[5]
   //6 :  from DQ[6]
   //7 :  from DQ[7]
//psram_dly_16 is used for all DQ[0~] in/out/oen delay adjustment.
//psram_dly_16 consists of 15 delay cells, one delay cell is 50ps delay( typical corner).
//psram_dly_16 is control by delay_sel[3:0].  0 = no delay. 1~15 controls how many delay cells used.
//psram_dly_16 can be controlled by each control register either with real delay mode or VT updated with 4xclock period.
//psram_lcdl is used for DQS/DQSN input and CK/CKN output delay 90degree delay generation and/or delay fine tune.
//psram_lcdl consits of 96 delay cells,  same delay cell used in psram_dly_16.
//psram_lcdl should be calibrated with the psram_4xclock. and tracked with psram_4xclock with VT updated.
#define PSRAM_DQ0_DIN_DLY                          ((0x00c0  << 2) + 0xffa18000)
  //bit 31:28. DQ7 DIN delay control.  in DLY16 real mode, only [3:0] is valid.
  //bit 27:24. DQ6 DIN delay control.  in DLY16 real mode, only [3:0] is valid.
  //bit 23:20. DQ5 DIN delay control.  in DLY16 real mode, only [3:0] is valid.
  //bit 19:16. DQ4 DIN delay control.  in DLY16 real mode, only [3:0] is valid.
  //bit 15:12. DQ3 DIN delay control.  in DLY16 real mode, only [3:0] is valid.
  //bit 11:8.  DQ2 DIN delay control.  in DLY16 real mode, only [3:0] is valid.
  //bit 7:4.   DQ1 DIN delay control.  in DLY16 real mode, only [3:0] is valid.
  //bit 3:0.   DQ0 DIN delay control.  in DLY16 real mode, only [3:0] is valid.
#define PSRAM_DQ1_DIN_DLY                          ((0x00c1  << 2) + 0xffa18000)
  //bit 6:0. DQ1 input delay control. in DLY16 read mode,  only [3:0] is valid.
#define PSRAM_DQ2_DIN_DLY                          ((0x00c2  << 2) + 0xffa18000)
  //bit 6:0. DQ2 input delay control. in DLY16 real mode, only [3:0] is valid.
#define PSRAM_DQ3_DIN_DLY                          ((0x00c3  << 2) + 0xffa18000)
  //bit 6:0. DQ3 input delay control. in DLY16 real mode, only [3:0] is valid.
#define PSRAM_DQ4_DIN_DLY                          ((0x00c4  << 2) + 0xffa18000)
  //bit 6:0. DQ4 input delay control. in DLY16 real mode, only [3:0] is valid.
#define PSRAM_DQ5_DIN_DLY                          ((0x00c5  << 2) + 0xffa18000)
  //bit 6:0. DQ5 input delay control. in DLY16 real mode, only [3:0] is valid.
#define PSRAM_DQ6_DIN_DLY                          ((0x00c6  << 2) + 0xffa18000)
  //bit 6:0. DQ6 input delay control. in DLY16 real mode, only [3:0] is valid.
#define PSRAM_DQ7_DIN_DLY                          ((0x00c7  << 2) + 0xffa18000)
  //bit 6:0. DQ7 input delay control.  in DLY16 real mode, only [3:0] is valid.
#define PSRAM_DQ0_DOUT_DLY                         ((0x00c8  << 2) + 0xffa18000)
  //bit 6:0. DQ0 DOUT delay control.  in DLY16 real mode, only [3:0] is valid.
#define PSRAM_DQ1_DOUT_DLY                         ((0x00c9  << 2) + 0xffa18000)
  //bit 6:0. DQ1 output delay control. in DLY16 read mode,  only [3:0] is valid.
#define PSRAM_DQ2_DOUT_DLY                         ((0x00ca  << 2) + 0xffa18000)
  //bit 6:0. DQ2 output delay control. in DLY16 real mode, only [3:0] is valid.
#define PSRAM_DQ3_DOUT_DLY                         ((0x00cb  << 2) + 0xffa18000)
  //bit 6:0. DQ3 output delay control. in DLY16 real mode, only [3:0] is valid.
#define PSRAM_DQ4_DOUT_DLY                         ((0x00cc  << 2) + 0xffa18000)
  //bit 6:0. DQ4 output delay control. in DLY16 real mode, only [3:0] is valid.
#define PSRAM_DQ5_DOUT_DLY                         ((0x00cd  << 2) + 0xffa18000)
  //bit 6:0. DQ5 output delay control. in DLY16 real mode, only [3:0] is valid.
#define PSRAM_DQ6_DOUT_DLY                         ((0x00ce  << 2) + 0xffa18000)
  //bit 6:0. DQ6 output delay control. in DLY16 real mode, only [3:0] is valid.
#define PSRAM_DQ7_DOUT_DLY                         ((0x00cf  << 2) + 0xffa18000)
  //bit 6:0. DQ7 output delay control.  in DLY16 real mode, only [3:0] is valid.
#define PSRAM_DQ0_OEN_DLY                          ((0x00d0  << 2) + 0xffa18000)
  //bit 6:0. DQ0 OEN delay control.  in DLY16 real mode, only [3:0] is valid.
#define PSRAM_DQ1_OEN_DLY                          ((0x00d1  << 2) + 0xffa18000)
  //bit 6:0. DQ1 output enable delay control. in DLY16 read mode,  only [3:0] is valid.
#define PSRAM_DQ2_OEN_DLY                          ((0x00d2  << 2) + 0xffa18000)
  //bit 6:0. DQ2 output enable delay control. in DLY16 real mode, only [3:0] is valid.
#define PSRAM_DQ3_OEN_DLY                          ((0x00d3  << 2) + 0xffa18000)
  //bit 6:0. DQ3 output enable delay control. in DLY16 real mode, only [3:0] is valid.
#define PSRAM_DQ4_OEN_DLY                          ((0x00d4  << 2) + 0xffa18000)
  //bit 6:0. DQ4 output enable delay control. in DLY16 real mode, only [3:0] is valid.
#define PSRAM_DQ5_OEN_DLY                          ((0x00d5  << 2) + 0xffa18000)
  //bit 6:0. DQ5 output enable delay control. in DLY16 real mode, only [3:0] is valid.
#define PSRAM_DQ6_OEN_DLY                          ((0x00d6  << 2) + 0xffa18000)
  //bit 6:0. DQ6 output enable delay control. in DLY16 real mode, only [3:0] is valid.
#define PSRAM_DQ7_OEN_DLY                          ((0x00d7  << 2) + 0xffa18000)
  //bit 6:0. DQ7 output enable delay control.  in DLY16 real mode, only [3:0] is valid.
#define PSRAM_DM_DOUT_DLY                          ((0x00d8  << 2) + 0xffa18000)
  //bit 6:0. DM output delay control.  in DLY16 real mode, only [3:0] is valid.
#define PSRAM_DM_OEN_DLY                           ((0x00d9  << 2) + 0xffa18000)
  //bit 6:0. DM output enable delay control.  in DLY16 real mode, only [3:0] is valid.
#define PSRAM_CS_DOUT_DLY                          ((0x00da  << 2) + 0xffa18000)
  //bit 6:0. CS output  delay control.  in DLY16 real mode, only [3:0] is valid.
#define PSRAM_DQS_DIN_DLY                          ((0x00db  << 2) + 0xffa18000)
  //bit 6:0. DQS input delay control.   unit = 1/64 UI with PSRAM_LCDL_CTRL bit 28 = 0;
  //  unit = 1 delay cell,    with PSRAM_LCDL_CTRL bit 28 = 1;
#define PSRAM_DQSN_DIN_DLY                         ((0x00dc  << 2) + 0xffa18000)
  //bit 6:0. DQSN input delay control.   unit = 1/64 UI with PSRAM_LCDL_CTRL bit 28 = 0;
  //  unit = 1 delay cell,    with PSRAM_LCDL_CTRL bit 28 = 1;
#define PSRAM_CKN_DOUT_DLY                         ((0x00dd  << 2) + 0xffa18000)
  //bit 6:0. CKN output delay control.   unit = 1/64 UI with PSRAM_LCDL_CTRL bit 28 = 0;
  //  unit = 1 delay cell,    with PSRAM_LCDL_CTRL bit 28 = 1;
#define PSRAM_CK_DOUT_DLY                          ((0x00de  << 2) + 0xffa18000)
  //bit 6:0. CK output delay control.   unit = 1/64 UI with PSRAM_LCDL_CTRL bit 28 = 0;
  //  unit = 1 delay cell,    with PSRAM_LCDL_CTRL bit 28 = 1;
#define PSRAM_RDEN_DLY                             ((0x00df  << 2) + 0xffa18000)
  //bit 10:7.  READ enable phase delay. unit = 1UI.  for AP QPI PSRAM.  PSRAM only send one cycle DQS preamble.
  //           We have to used this UI delay to compensate the whole clock output delay and the DQS input delay.
  //           if run high frequency, this delay need to be trained.  the valid value from 0 ~ 12.  Since one UI is 1/4 of PSRAM clock,
  //           the total delay of the clock output delay + input delay should be less than 3 PSRAM clock.
  //bit 6:0. READ enable fine tune delay control.   unit = 1/64 UI with PSRAM_LCDL_CTRL bit 28 = 0;
  //  unit = 1 delay cell,    with PSRAM_LCDL_CTRL bit 28 = 1;
#define PSRAM_LCDL_CTRL                            ((0x00f0  << 2) + 0xffa18000)
  //bit 31.   write 1 to update all delay cell delays control.
  //bit 30.   write 1 to  calibration LCDL.
  //bit 29.   LCDL track enable.  1: enable LCDL auto track VT changes.
  //bit 28.   LCDL delay control mode.
	 //1 : real delay mode. For DQS/DQSN/CKN/CK/RDEN delay control register, one number means one delay cells.
	 //0 : UI MODE.    For DQS/DQSN/CKN/CK/RDEN delay control register, one number  means 1/64 psram_4xclk period.
  //bit 27.   DLY16 ( psram_dly_16 cell) delay control mode.
	 //1 : real delay mode. For all data delay control register, one number means one delay cells.
	 //0 : UI MODE.     For all data delay control register, one number means 1/64 psram_4xclk period.
  //bit 27:6.  not used.
  //bit 26.  to generate clock phase same as data phase.
  //bit 25.  enable to extend I/O pin OE one cycle later.
  //bit 24.  enable to extend I/O pin OE one cycle earlier.
  //bit 23:0.   timer do trigger  LCDL track the VT once.  each track will compensate the UI with  1/16 delay cell delay.
//LCDL calibration ctroller.
#define PSRAM_LCDL_CAL_CTRL1                       ((0x00f1  << 2) + 0xffa18000)
  //27:24. which LCDL delay line used to run calibration.
  //19:16. LCDL calibration loop end position.
  //15:12. LCDL calibration loop initial position.
  //10:0.  LCDL calibration initial phase counter.
#define PSRAM_LCDL_CAL_CTRL2                       ((0x00f2  << 2) + 0xffa18000)
 //31     cfg_cal_hp
 //26:16  LCDL phase lock limit. if the lcdl phase counter  difference between 2 calibration loops is less than this number, The LCDL would be locked and stop the calibration.
 //10:0.  deta_init. the first loop phase changing number.
#define PSRAM_LCDL_CAL_CTRL3                       ((0x00f3  << 2) + 0xffa18000)
  //29:25 to configure LCDL calibration step 5 position.
  //24:20 to configure LCDL calibration step 4 position.
  //19:15 to configure LCDL calibration step 3 position.
  //14:10 to configure LCDL calibration step 2 position.
  //9:5   to configure LCDL calibration step 1 position.
  //4:0   to configure LCDL calibration step 0 position.
#define PSRAM_LCDL_CAL_CTRL4                       ((0x00f4  << 2) + 0xffa18000)
  //19:15 to configure LCDL calibration step 9 position.
  //14:10 to configure LCDL calibration step 8 position.
  //9:5   to configure LCDL calibration step 7 position.
  //4:0   to configure LCDL calibration step 6 position.
#define PSRAM_CLK_UI                               ((0x00f5  << 2) + 0xffa18000)
  //bit 6:0. 4xCLOCK period delay measured from LCDL calibration.
#define PSRAM_LCDL_PH                              ((0x00f6  << 2) + 0xffa18000)
  //10:0 LCDL phase counter after calibration.
#define PSRAM_LCDL_STATUS                          ((0x00f7  << 2) + 0xffa18000)
  //bit 31. LCDL lock status.   1: LCDL locked to 4x clock.
//`endif
//
// Closing file:  ../psram/rtl/psram_reg.vh
//
//


/* ---------- psram.h ---------- */

/*
 * Copyright (C) 2019 Amlogic, Inc. All rights reserved.
 */

//#include "../include/timing.h"
//#include "../include/ddr_define.h"
//#include <acs.h>
//#include <fip-v3.h>
//#include <usb/usb_pcd.h>


#define PSRAM_D2PLL_FUNCTION_MASK_BIT		0

#define PSRAM_WINDOW_TEST

#define AXI_INTERFACE_SELECT_DDR			1
#define AXI_INTERFACE_SELECT_PSRAM			2

//#define DRIVER_STRENGTH_INEX_SOC_DRIVER   1
//#define DRIVER_STRENGTH_INEX_PSRAM_WINBOND_DRIVER   2
//#define DRIVER_STRENGTH_INEX_PSRAM_AP_MEMORY_DRIVER  3

#define PSRAM_CHIP_LOGIC_INDEX_SOC			1

#define PSRAM_CHIP_LOGIC_INDEX_WINBOND_3_CMD_W955D8MKY	0x20
#define PSRAM_CHIP_LOGIC_INDEX_WINBOND_3_CMD_W956D8MKY	0x21
#define PSRAM_CHIP_LOGIC_INDEX_WINBOND_3_CMD_W966D8MKY	0x22

#define PSRAM_CHIP_LOGIC_INDEX_AP_MEMORY_3_CMD		0x30

#define PSRAM_WRITE_MODE_REGISTER		1 //do not modify sequence
#define PSRAM_READ_MODE_REGISTER		3 //do not modify sequence
#define PSRAM_WRITE_MEMORY_DATA			0 //do not modify sequence
#define PSRAM_READ_MEMORY_DATA			2 //do not modify sequence

#define PSRAM_CHIP_LOGIC_INDEX_AP_MEMORY_3_CMD_MR1		0x8d
#define PSRAM_CHIP_LOGIC_INDEX_AP_MEMORY_3_CMD_MR2		0x93

#define PSRAM_CHIP_LOGIC_INDEX_WINBOND_3_CMD_W955D8MKY_MR0	0x5f
#define PSRAM_CHIP_LOGIC_INDEX_WINBOND_3_CMD_W956D8MKY_MR0	0x86
#define PSRAM_CHIP_LOGIC_INDEX_WINBOND_3_CMD_W966D8MKY_MR0	0x96

typedef struct psram_set {
	unsigned	int		magic;
	unsigned	int		board_id;
	/* align8 */
	unsigned	int		version;
	unsigned	char	psram_mr[12];
	/* align8 */
	unsigned	char	psram_board_mask;
	unsigned	char	psram_amlogic_protocol_id;
	unsigned	char	psram_test_function[2];
	unsigned	char	psram_vendor_id;
	unsigned	char	psram_device_id;
	unsigned	char	psram_soc_drv;
	unsigned	char	psram_dram_drv;
	/* align8 */
	unsigned	int		psram_ac_timing0;
	unsigned	int		psram_ac_timing1;
	unsigned	int		psram_ac_timing2;
	unsigned	int		psram_mode_crtl;
	/* align8 */
	unsigned	short	psram_frequency;
	unsigned	short	psram_size;
	unsigned	int		psram_mode_crtl_bl33;
	unsigned	int		psram_pin_crtl;
	unsigned	int		psram_lcd_ctrl;
	/* align8 */
	unsigned	char	psram_pin_dq_in_delay[8];
	unsigned	char	psram_pin_dq_out_delay[8];
	unsigned	char	psram_pin_dq_out_oe_delay[8];
	/* align8 */
	unsigned	char	psram_pin_dm_out_delay;
	unsigned	char	psram_pin_dm_out_oe_delay;
	unsigned	char	psram_pin_cs_out_delay;
	unsigned	char	psram_pin_dqsp_in_delay;
	unsigned	char	psram_pin_dqsn_in_delay;
	unsigned	char	psram_pin_ckp_out_delay;
	unsigned	char	psram_pin_ckn_out_delay;
	/* align8 */
	unsigned	char	psram_pin_rden_delay;
	unsigned	char	psram_bdlr_delay;
	unsigned	char	psram_reserve[6];

	/* pls check alignment of each variable */
}__attribute__ ((packed)) psram_set_t;

typedef struct psram_bl2_set{
	uint16_t 	psram_mr[12];
	uint32_t	psram_ac_timing0;
	uint32_t	psram_ac_timing1;
	uint32_t	psram_ac_timing2;
	uint32_t	psram_mode_crtl;
	uint32_t	psram_cur_clk_frequency;
	char 		psram_cur_amlogic_protocol_id;
	char psram_use_hifi_pll;
	char psram_fixed_latency_enable;
	char psram_initial_latency;
	char psram_cmd_cycle;
	char psram_enable_data_mask;
	char psram_user_cmd_code_read_register;
	char psram_user_cmd_code_write_register;
	char psram_user_cmd_code_read_memory;
	char psram_user_cmd_code_write_memory;
	uint32_t	axi_req_ctrl0;
	uint32_t	axi_req_ctrl1;
	uint32_t	axi_req_ctrl2;
	uint32_t	user_ctrl0;
	uint32_t	user_ctrl1;
	uint32_t	user_ctrl2;
	uint32_t	user_ctrl3;
	uint32_t	psram_ctrl;
}psram_bl2_set_t;

typedef struct psram_bl33_set{
	char		psram_board_mask;
	char		psram_amlogic_protocol_id;
	char		psram_test_function[2];
	char		psram_vendor_id;
	char		psram_device_id;
	uint16_t	psram_frequency;
	uint16_t	psram_dram_size;
	char		psram_soc_drv;
	char		psram_dram_drv;
	uint32_t	psram_mode_crtl_bl33;
	uint32_t	psram_pin_crtl;
	uint32_t	psram_lcd_ctrl;
	char		psram_pin_dq_in_delay[8];
	char		psram_pin_dq_out_delay[8];
	char		psram_pin_dq_out_oe_delay[8];
	char		psram_pin_dm_out_delay;
	char		psram_pin_dm_out_oe_delay;
	char		psram_pin_cs_out_delay;
	char		psram_pin_dqsp_in_delay;
	char		psram_pin_dqsn_in_delay;
	char		psram_pin_ckp_out_delay;
	char		psram_pin_ckn_out_delay;
	char		psram_pin_rden_delay;
	char		psram_bdlr_delay;
	char		psram_reserver[7];
}psram_psram_dev_t;



/* ---------- psram driver ---------- */
psram_bl2_set_t *psram_bl2_set_p = {0};
//psram_psram_dev_t  p_dev;
psram_set_t *psram_p_dev_p = {0};
psram_bl2_set_t psram_bl2_set_p_t = {.psram_ac_timing0 = 0,};
psram_set_t psram_p_dev_p_t = {.psram_board_mask = 0,};
uint32_t sram_test_base = 0xf4000000;//64MB

#define P_AO_RESETCTRL_RESET0_LEVEL                (volatile uint32_t *)0xffa02004
#define PSRAM_FREQUENCY_TEST_INDEX  (1 << 0)
#define PSRAM_WINDOW_TEST_INDEX  (1 << 1)

void psram_init_pin(psram_set_t *psram_p_dev);
char psram_init_winbond(psram_set_t *psram_p_dev);
char psram_init_ap(psram_set_t *psram_p_dev);
void psram_re_init(psram_set_t *psram_p_dev);
void psram_frequency_test(psram_set_t *psram_p_dev);
void psram_window_test(psram_set_t *psram_p_dev);
char psram_uboot_drv_init(void);
psram_set_t *psram_uboot_get_dev(void);

static void psram_config_axi_interface(uint32_t ctrl3, uint32_t ctrl2, uint32_t ctrl1, uint32_t ctrl0) {
	wr_reg(PSRAM_AXI_REQ_CTRL3, ctrl3);
	wr_reg(PSRAM_AXI_REQ_CTRL2, ctrl2);
	wr_reg(PSRAM_AXI_REQ_CTRL1, ctrl1);
	wr_reg(PSRAM_AXI_REQ_CTRL0, ctrl0);
	wr_reg(PSRAM_DBUF_CTRL, (1 << 16) | 0x100);
	wr_reg(PSRAM_DBUF_CTRL1, 0x100);
	wr_reg(PSRAM_AXI_INTF_CTRL, 0x3);
}

static int psram_find_ap_freq_index(uint32_t frequency) {
	static const uint16_t ap_freqs[] = {250, 225, 200, 166, 133, 109, 66};
	int i = 0;
	for (; i < 7; i++) {
		if (frequency >= ap_freqs[i]) break;
	}
	return i;
}

static void psram_send_user_cmd(uint32_t addr, uint32_t data, uint32_t ctrl3, uint32_t ctrl2, uint32_t ctrl1, uint32_t ctrl0) {
	wr_reg(PSRAM_CFG_STS, data);
	wr_reg(PSRAM_USER_ADDR, addr);
	wr_reg(PSRAM_USER_CTRL3, ctrl3);
	wr_reg(PSRAM_USER_CTRL2, ctrl2);
	wr_reg(PSRAM_USER_CTRL1, ctrl1);
	wr_reg(PSRAM_USER_CTRL0, ctrl0);
}

static void psram_ap_write_mr(uint32_t addr, uint32_t val) {
	psram_send_user_cmd(addr, val, 0, 0xc001, 0, 0xa003c0c0);
}

static void psram_ap_read_mr(uint32_t addr, uint32_t latency) {
	psram_send_user_cmd(addr, 0, 0, 0x18010000, 0x8000 | (latency << 8) | (latency - 2), 0xf0034040);
}

static void psram_wb_write_reg(uint32_t addr, uint32_t data) {
	psram_send_user_cmd(addr, data, 0x20000, 0xc001, 0, 0xa8030000);
}

static void psram_wait_user_cmd(void) {
	while (((rd_reg(PSRAM_USER_CTRL0) >> 30) & 1) == 0);
}

static void psram_wait_status_ready(void) {
	while ((rd_reg(PSRAM_STATUS) & 0x80000000) != 0x80000000);
}

static void psram_setup_pads(psram_set_t *psram_p_dev) {
	psram_init_pin(psram_p_dev);
	if (psram_p_dev->psram_amlogic_protocol_id == PSRAM_CHIP_LOGIC_INDEX_AP_MEMORY_3_CMD) {
		wr_reg(PADCTRL_PIN_MUX_REGAO_5, 0x11111110);
		wr_reg(PADCTRL_GPIOP_OEN, rd_reg(PADCTRL_GPIOP_OEN) & 0xfffffffe);
		wr_reg(PADCTRL_GPIOP_O, rd_reg(PADCTRL_GPIOP_O) & 0xfffffffe); //set clk n to level 0
	}
}

static char psram_detect_ap_size(psram_set_t *psram_p_dev) {
	if ((psram_p_dev->psram_mr[1] & 0x1f) == (PSRAM_CHIP_LOGIC_INDEX_AP_MEMORY_3_CMD_MR1 & 0x1f)) {
		switch (psram_p_dev->psram_mr[2]) {
		case 0x91: psram_p_dev->psram_size = 4; break;
		case 0x93: psram_p_dev->psram_size = 8; break;
		case 0x95: psram_p_dev->psram_size = 16; break;
		case 0x97: psram_p_dev->psram_size = 32; break;
		case 0xc7: psram_p_dev->psram_size = 64; break;
		default: break;
		}
		return 1;
	}
	return 0;
}

#define ENABLE_WINBOND_PSRAM_PXP_DEBUG   1

//#define ENABLE_PSRAM_GET_AP_EID
#define ENABLE_PSRAM_EXTRA_TEST
char psram_calculate_drv(char chip_logic_index, char driver_strength_value) {
	if (chip_logic_index == PSRAM_CHIP_LOGIC_INDEX_WINBOND_3_CMD_W956D8MKY) {
		return (driver_strength_value > 114) ? 1 :
		       (driver_strength_value > 0) ? (134 / driver_strength_value) : 0;
	}
	if (chip_logic_index == PSRAM_CHIP_LOGIC_INDEX_WINBOND_3_CMD_W966D8MKY) {
		return (driver_strength_value > 114) ? 1 :
		       (driver_strength_value > 0) ? (134 / driver_strength_value) : 0;
	}
	if (chip_logic_index == PSRAM_CHIP_LOGIC_INDEX_SOC) {
		if (driver_strength_value > 240) return 0;
		if (driver_strength_value > 60) return 1;
		return (driver_strength_value > 50) ? 2 : 3;
	}
	if (driver_strength_value > 150) return 3;
	if (driver_strength_value > 75) return 2;
	char limit = (chip_logic_index == PSRAM_CHIP_LOGIC_INDEX_WINBOND_3_CMD_W955D8MKY) ? 42 : 38;
	if (chip_logic_index == PSRAM_CHIP_LOGIC_INDEX_WINBOND_3_CMD_W955D8MKY) {
		return (driver_strength_value > limit) ? 0 : 1;
	}
	return (driver_strength_value > limit) ? 1 : 0;
}

char psram_calculate_latency(char chip_logic_index, uint32_t psram_frequency) {
	if (chip_logic_index == PSRAM_CHIP_LOGIC_INDEX_AP_MEMORY_3_CMD) {
		return 10 - psram_find_ap_freq_index(psram_frequency);
	}
	if (chip_logic_index == PSRAM_CHIP_LOGIC_INDEX_WINBOND_3_CMD_W966D8MKY)
		chip_logic_index = PSRAM_CHIP_LOGIC_INDEX_WINBOND_3_CMD_W956D8MKY;
	char limit = (chip_logic_index == PSRAM_CHIP_LOGIC_INDEX_WINBOND_3_CMD_W956D8MKY) ? 85 : 66;
	if (psram_frequency > 166)
		return (chip_logic_index == PSRAM_CHIP_LOGIC_INDEX_WINBOND_3_CMD_W956D8MKY) ? 7 : 6;
	if (psram_frequency > 133) return 6;
	if (psram_frequency > 104) return 5;
	if (psram_frequency > limit) return 4;
	return 3;
}

char psram_calculate_wl_latency(char chip_logic_index, uint32_t psram_frequency) {
	if (chip_logic_index == PSRAM_CHIP_LOGIC_INDEX_AP_MEMORY_3_CMD) {
		return psram_calculate_latency(chip_logic_index, psram_frequency) + 4;
	}
	return 0;
}

void psram_init_pin(psram_set_t *psram_p_dev) {
	uint32_t gpio_drv = 0;

	// Set GPIO PAD0/PAD3 low before enabling mux to avoid IO model warning
	wr_reg(PADCTRL_GPIOP_OEN, 0);
	wr_reg(PADCTRL_GPIOP_O, 0);

	_udelay(1);
	wr_reg(PADCTRL_PIN_MUX_REGAO_5, 0x11111111);
	wr_reg(PADCTRL_PIN_MUX_REGAO_6, 0x11111);
	if (psram_p_dev->psram_pin_crtl == 0)
		psram_p_dev->psram_pin_crtl = 0x76543210;
	wr_reg(PSRAM_PIN_CTRL, psram_p_dev->psram_pin_crtl);
	gpio_drv = psram_calculate_drv(PSRAM_CHIP_LOGIC_INDEX_SOC,
		psram_p_dev->psram_soc_drv);
	gpio_drv = (gpio_drv * 0x01555555);
	wr_reg(PADCTRL_GPIOP_DS, gpio_drv);
}

void psram_reset_psram(char chip_logic_index) {
	//if((PSRAM_CHIP_LOGIC_INDEX_WINBOND_3_CMD==chip_logic_index)
	//||(PSRAM_CHIP_LOGIC_INDEX_AP_MEMORY_3_CMD==chip_logic_index))
	// enable PSRAM RESET_N pin
	uint32_t data32 = 0;
	wr_reg(PSRAM_CTRL,(data32) | ((1 << 17)));
	_udelay(1);
	wr_reg(PSRAM_CTRL, (data32) & (~ (1 << 17)));
	//wr_reg(PSRAM_CTRL,(data32)|(( 1 << 17)) );
	//wait 150us for PSRAM stable.
	_udelay(150);
	wr_reg(PSRAM_CTRL,(data32) | ((1 << 17)));
	//wr_reg(PSRAM_CTRL, (data32)&(~ ( 1 << 17)) );
}

char psram_lcdl_calibration(void) {
	//LCDL calibration
	uint32_t data32;
	//uint32_t loop_count = 100;

	//serial_puts("psram debug-4-1-1");
	wr_reg(PSRAM_LCDL_CTRL, 0);
	wr_reg(PSRAM_LCDL_CAL_CTRL2, (2 << 16) | 40);
	wr_reg(PSRAM_LCDL_CAL_CTRL1, 0 | (0 << 12) | (4 << 16) | (0 << 24));
	wr_reg(PSRAM_LCDL_CTRL, 1 << 30);
	//serial_puts("\npxp turn off dll calibration");

	while (rd_reg(PSRAM_LCDL_STATUS) != 0x80000000);
	data32 = rd_reg(PSRAM_CLK_UI);
	serial_puts("CAL RESULT: 1 4xclock period = delay cells ");
	serial_put_hex(data32, 32);
	data32 = rd_reg(PSRAM_CTRL);
	serial_puts("\nPSRAM_CTRL reg = ");
	serial_put_hex(data32, 32);


	//config DQS din phase.
	//  wr_reg(PSRAM_DQS_DIN_DLY, 64);  //1 4xclock period.
	//  wr_reg(PSRAM_DQSN_DIN_DLY, 64);  //1 4xclock period.
	//wr_reg(PSRAM_DQS_DIN_DLY, 32);  //1 2xclock period.
	//wr_reg(PSRAM_DQSN_DIN_DLY, 32);  //1 2xclock period.
	wr_reg(PSRAM_LCDL_CTRL, 1 << 31); //update LCDL setting.
	wr_reg(PSRAM_LCDL_CTRL, 0x20001000 | (3 << 24)); //enable LCDL tracking.

//#if  ENABLE_WINBOND_PSRAM_PXP_DEBUG
//	loop_count = 10;
//#else
//	loop_count = 100;
//#endif
//	do {
//		if (Rd(PSRAM_LCDL_STATUS) & 0x80000000)
//			break;
//	} while(loop_count--);
//	data32 = Rd(PSRAM_CLK_UI);
//	serial_puts("CAL RESULT: 1 4xclock period = delay cells\n");
//	serial_put_hex(data32, 32);
	return data32;
}

static uint32_t pack_delay_value(const uint8_t *delay_arr) {
	uint32_t val = 0;
	for (int i = 0; i < 8; i++) {
		val |= ((uint32_t)delay_arr[i] & 0xf) << (i * 4);
	}
	return val;
}

void psram_config_phy_delay(psram_set_t *psram_p_dev) {
	if (psram_p_dev->psram_pin_dqsp_in_delay == 0)
		psram_p_dev->psram_pin_dqsp_in_delay = 32;

	wr_reg(PSRAM_DQ0_DIN_DLY, pack_delay_value(psram_p_dev->psram_pin_dq_in_delay));
	wr_reg(PSRAM_DQ0_DOUT_DLY, pack_delay_value(psram_p_dev->psram_pin_dq_out_delay));
	wr_reg(PSRAM_DQ0_OEN_DLY, pack_delay_value(psram_p_dev->psram_pin_dq_out_oe_delay));

	wr_reg((PSRAM_DM_DOUT_DLY), (psram_p_dev->psram_pin_dm_out_delay));
	wr_reg((PSRAM_DM_OEN_DLY), (psram_p_dev->psram_pin_dm_out_oe_delay));
	wr_reg((PSRAM_CS_DOUT_DLY), (psram_p_dev->psram_pin_cs_out_delay));
	wr_reg((PSRAM_DQS_DIN_DLY), (psram_p_dev->psram_pin_dqsp_in_delay));
	wr_reg((PSRAM_DQSN_DIN_DLY), (psram_p_dev->psram_pin_dqsp_in_delay));
	wr_reg((PSRAM_CK_DOUT_DLY), (psram_p_dev->psram_pin_ckp_out_delay));
	wr_reg((PSRAM_CKN_DOUT_DLY), (psram_p_dev->psram_pin_ckp_out_delay));
	wr_reg((PSRAM_RDEN_DLY), (psram_p_dev->psram_pin_rden_delay));
}

#if 0
unsigned int ddr_test_single_addr(unsigned int addr, unsigned int value)
{
	addr = ((addr >> 2) << 2);
	serial_puts("ddr test:\n");
	serial_puts("write test: write[0x");
	serial_put_hex(addr, 32);
	serial_puts("]=0x");
	serial_put_hex(value, 32);
	serial_puts("...");
	serial_puts("\n");
	wr_reg(addr, value);


	unsigned int count = 0;

	//for(unsigned int count=0;count<0x10;){

	serial_puts("read[0x");
	serial_put_hex(addr + count, 32);
	serial_puts("]=0x");
	serial_put_hex(rd_reg(addr + count), 32);
	serial_puts("\n");
	//count=count+4;}
	if (value != rd_reg(addr))
		return 1;
	else
		return 0;
}
#endif

char psram_test(uint32_t test_size) {
	////test psram add and data size
	sram_test_base = 0xf4000000;//64MB
	//ddr_test_single_addr(sram_test_base + 0x00000000, 0x12345678);
	//ddr_test_single_addr(sram_test_base + 0x00000004, 0x11223344);
	//ddr_test_single_addr(sram_test_base + 0x00000008, 0x55667788);
	//ddr_test_single_addr(sram_test_base + 0x0000000c, 0x33ccddee);
	//ddr_test_single_addr(sram_test_base + 0x00000010, 0x7788aabb);
#if 1
	uint32_t test_fail = 0;
	unsigned long i = 0;
	unsigned long repeat = 0;
	unsigned long repeat_max = 1;

	watchdog_disable();
	//serial_puts("\nwrite test");
	for (repeat = 0; repeat < repeat_max; repeat++) {
		for (i = sram_test_base; i < (sram_test_base + test_size);) {
			wr_reg(i, (i >> 2));
			if (i % 0x10000 == 0) {
				serial_puts("\r");
				serial_put_hex(i, 32);
			}
			i = i + 4;
		}
	}
	//serial_puts("\nread test");
	for (repeat = 0; repeat < repeat_max; repeat++) {
		for (i = sram_test_base; i < (sram_test_base + test_size);) {
			//wr_reg(i, (i>>2));
			if (i % 0x10000 == 0) {
				serial_puts("\r");
				serial_put_hex(i, 32);
				serial_puts(" ");
				serial_put_hex(rd_reg(i), 32);
			}
			if ((rd_reg(i)) != (i >> 2)) {
				#if 0
				serial_puts("\nadd 0x");
				serial_put_hex(i, 32);
				serial_puts("  data ");
				serial_put_hex(rd_reg(i), 32);
				serial_puts(" ");
				#endif
				test_fail++;
				//return test_fail;
			}
			i = i + 4;
		}
	}
	serial_puts("\npsram test error ==");
	serial_put_dec(test_fail);
	return test_fail;
#endif
}

uint32_t psram_pll_init(psram_set_t *psram_p_dev) {
	uint32_t freq = (psram_p_dev->psram_frequency);
	serial_puts("\npsram freq==");
	serial_put_dec(freq);
	serial_puts("\n");
	//if ( (freq != 192) && (freq != 128) && (freq != 32)) {
		/* todo, hifi pll init here */
	//	hifi_pll_init(freq << 3);
		//CONFIG PSRAM CLOCK.
	if (freq <= 12) //12MHz psram clk
		wr_reg(AO_CLKCTRL_PSRAM_CLK_CTRL, (1 << 8) | (0 << 9) | 0); //rtc_oscin_clk 24Mhz
	else if (freq <= 100)
		wr_reg(AO_CLKCTRL_PSRAM_CLK_CTRL, (1 << 8) | (1 << 9) | 1); //fix_div5/2 = 200MHz
	else if (freq <= 166)
		wr_reg(AO_CLKCTRL_PSRAM_CLK_CTRL, (1 << 8) | (3 << 9) | 1); //fix_div3/2 = 333MHz
	else if (freq <= 200)
		wr_reg(AO_CLKCTRL_PSRAM_CLK_CTRL, (1 << 8) | (1 << 9) | 0); //fix_div5 = 400MHz
	else if (freq <= 250)
		wr_reg(AO_CLKCTRL_PSRAM_CLK_CTRL, (1 << 8) | (2 << 9) | 0); //fix_div4 = 500MHz
	else //166Mhz
		wr_reg(AO_CLKCTRL_PSRAM_CLK_CTRL, (1 << 8) | (3 << 9) | 1); //fix_div3/2 = 333MHz
		//wr_reg(AO_CLKCTRL_PSRAM_CLK_CTRL, 1 | (3 << 9) | (1 << 8));
		//fix_div3/2 = 333MHz  //for 2x_clk
	//}
	//else {
	//	/* config clk ctrl */
	//	wr_reg(AO_CLKCTRL_PSRAM_CLK_CTRL, get_ram_clk_ctrl(freq << 2));
	//	/* enable clk */
	//	wr_reg(AO_CLKCTRL_PSRAM_CLK_CTRL, rd_reg(AO_CLKCTRL_PSRAM_CLK_CTRL) | (1 << 8));
	//}
	return 0;
}

/* AO reset pulse before PHY bring-up (AP path) */
static void psram_ao_reset_pulse(void)
{
	*P_AO_RESETCTRL_RESET0_LEVEL &= ~(1 << 30);
	_udelay(1);
	*P_AO_RESETCTRL_RESET0_LEVEL |= (1 << 30);
	_udelay(1);
	_udelay(1);
	_udelay(1);
}

#define PSRAM_BRINGUP_PRE_UDELAY	(1u << 0)
#define PSRAM_BRINGUP_AO_RESET		(1u << 1)
#define PSRAM_BRINGUP_PHY_DELAY		(1u << 2)
#define PSRAM_BRINGUP_POST_UDELAY10	(1u << 3)

/* LCDL cal + pad mux + device reset */
static void psram_init_cal_pads_reset(psram_set_t *psram_p_dev)
{
	psram_lcdl_calibration();
	psram_setup_pads(psram_p_dev);
	psram_reset_psram(psram_p_dev->psram_amlogic_protocol_id);
}

/* PLL/APB/LCDL bring-up; flags select AP vs Winbond differences */
static void psram_init_hw_bringup(psram_set_t *psram_p_dev, unsigned char flags)
{
	//if (flags & PSRAM_BRINGUP_PRE_UDELAY) {
	//	_udelay(1);
	//	_udelay(1);
	//}
	//if (flags & PSRAM_BRINGUP_AO_RESET)
	psram_ao_reset_pulse();
	_udelay(1);
	psram_pll_init(psram_p_dev);
	_udelay(1);
	//if (flags & PSRAM_BRINGUP_PHY_DELAY)
	psram_config_phy_delay(psram_p_dev);
	_udelay(1);
	wr_reg(PSRAM_APB_CTRL, 0x99);
	//if (flags & PSRAM_BRINGUP_POST_UDELAY10)
	_udelay(10);
	psram_init_cal_pads_reset(psram_p_dev);
	_udelay(1);
}

/* PSRAM_CTRL / ACTIMING / LCDL after device reset */
static void psram_init_phy_regs(unsigned char chip_id)
{
	wr_reg(PSRAM_ACTIMING0, (1 << 28));
	if (chip_id == PSRAM_CHIP_LOGIC_INDEX_AP_MEMORY_3_CMD) {
		wr_reg(PSRAM_CTRL, (0 << 24) | (1 << 17) | (1 << 31) | (0 << 30) |
			(1 << 20) | (1 << 21) | (1 << 22) | (4 << 27) | (0 << 9));
		wr_reg(PSRAM_ACTIMING1, 4);
		//wr_reg(PSRAM_DQS_DIN_DLY, 32);
		//wr_reg(PSRAM_DQSN_DIN_DLY, 32);
		//wr_reg(PSRAM_LCDL_CTRL, (1 << 31) | (3 << 24));
		//wr_reg(PSRAM_LCDL_CTRL, (1 << 29) | 0x1000 | (3 << 24));
	} else {
		wr_reg(PSRAM_CTRL, (1 << 31) | (5 << 27) | (1 << 9) | (1 << 17) | (1 << 22));
		wr_reg(PSRAM_CTRL, (1 << 31) | (5 << 27) | (1 << 9) | (1 << 17) | (0 << 22));
		wr_reg(PSRAM_ACTIMING1, 5);
	}
	wr_reg(PSRAM_LCDL_CTRL, 1 << 31);
	wr_reg(PSRAM_LCDL_CTRL, 0x20001000 | (3 << 24));
}

void psram_re_init(psram_set_t *psram_p_dev) {
	switch (psram_p_dev->psram_amlogic_protocol_id) {
	case PSRAM_CHIP_LOGIC_INDEX_WINBOND_3_CMD_W955D8MKY:
	case PSRAM_CHIP_LOGIC_INDEX_WINBOND_3_CMD_W956D8MKY:
	case PSRAM_CHIP_LOGIC_INDEX_WINBOND_3_CMD_W966D8MKY:
		psram_init_winbond(psram_p_dev);
		break;
	case PSRAM_CHIP_LOGIC_INDEX_AP_MEMORY_3_CMD:
		psram_init_ap(psram_p_dev);
		break;
	default:
		break;
	}
}
#ifdef PSRAM_WINDOW_TEST
#if 1
void psram_window_test(psram_set_t *psram_p_dev) {
	uint32_t write_read_dqs[2][3] = {{0,0,0,}, {0,0,0,}};
	unsigned count_i = 0;
	unsigned count = 0;
	char count_far_fail = 0;
	char count_near_ok = 0;
	char test_fail = 0;
	unsigned clk_org = 0;
	unsigned test_size_min = (1 << 15);
	unsigned test_size = (4 << 19);
	unsigned long reg_add = 0;
	clk_org = rd_reg(PSRAM_CK_DOUT_DLY);
	unsigned count_j = 0;
	for (count_i = 0; count_i < 2; count_i++) {
		uint32_t *dqs = write_read_dqs[count_i];
		if (count_i == 0)
			reg_add = PSRAM_DQS_DIN_DLY;
		else
			reg_add = PSRAM_CKN_DOUT_DLY;

		count = rd_reg(reg_add);
		dqs[0] = count;
		dqs[1] = 0;
		dqs[2] = 96;
		for (count_j = 0; count_j < 2; count_j++) {
			count_near_ok = dqs[0];
			if (count_j == 0) {
				count_far_fail = dqs[1];
				if (count_i == 1) {
					wr_reg(PSRAM_CKN_DOUT_DLY, 64 + clk_org);
					wr_reg(PSRAM_CK_DOUT_DLY, 64 + clk_org);
					wr_reg(PSRAM_LCDL_CTRL, 0x20001000 | (3 << 24) | (1 << 26));
					wr_reg(PSRAM_LCDL_CTRL, 0x20001000 | (3 << 24) | (1 << 26) | (1 << 31));
					count = rd_reg(reg_add);
					dqs[0] = count;
					count_near_ok = dqs[0];
				}
			}
			else {
				count_far_fail = dqs[2];
				if (count_i == 1) {
					count = rd_reg(reg_add);
					dqs[0] = count;
					count_near_ok = dqs[0];
				}
			}
			while (1) {
				count = ((count_far_fail + count_near_ok) >> 1);
				wr_reg(reg_add, count);
				wr_reg(reg_add + 4, count);
				if ((count_i == 1) && (count_j == 0)) {
					wr_reg(PSRAM_LCDL_CTRL, 0x20001000 | (3 << 24) | (1 << 26));
				}
				wr_reg(PSRAM_LCDL_CTRL, (rd_reg(PSRAM_LCDL_CTRL)) | (1 << 31));
				test_fail = psram_test(test_size_min);
				if (test_fail == 0) {
					test_fail = test_fail + psram_test(test_size);
				}
				if (test_fail) {
					count_far_fail = count;
					test_fail = 0;
					psram_re_init(psram_p_dev);
				}
				else {
					count_near_ok = count;
				}
				if ((((count_far_fail + 1) >= count_near_ok) && (count_j == 0)) || \
					(((count_near_ok + 1) >= count_far_fail) && (count_j == 1))) {
					dqs[count_j == 0 ? 1 : 2] = count_near_ok;
					test_fail = 0;
					psram_re_init(psram_p_dev);
					break;
				}
			}
		}
	}
	uint32_t *dqs_w = write_read_dqs[1];
	uint32_t *dqs_r = write_read_dqs[0];
	dqs_w[2] = 64 + dqs_w[2];
	dqs_w[0] = 64 + clk_org;
	for (count_i = 0; count_i < 2; count_i++) {
		uint32_t *dqs = write_read_dqs[count_i];
		serial_puts(count_i == 0 ? "\npsram window  read    " : "\npsram window  write   ");
		serial_put_dec(dqs[0]);
		serial_puts("   ");
		serial_put_dec(dqs[1]);
		serial_puts("   ");
		serial_put_dec(dqs[2]);
		serial_puts("    1/64UI==");
		serial_put_dec((1000000 / (psram_p_dev->psram_frequency << 2)) / 64);
		serial_puts("ps\n");
	}
	wr_reg(PSRAM_DQS_DIN_DLY, (dqs_r[1] + dqs_r[2]) >> 1);
	wr_reg(PSRAM_DQSN_DIN_DLY, (dqs_r[1] + dqs_r[2]) >> 1);
	count_i = ((dqs_w[1] + dqs_w[2]) >> 1);
	if (count_i > 64) {
		wr_reg(PSRAM_CKN_DOUT_DLY, count_i - 64);
		wr_reg(PSRAM_CK_DOUT_DLY, count_i - 64);
	}
	else {
		for (count_j = PSRAM_DQ0_DOUT_DLY; count_j <= PSRAM_DQ7_DOUT_DLY - PSRAM_DQ0_DOUT_DLY;) {
			wr_reg(count_j, 64 - count_i);
			count_j = count_j + 4;
		}
	}
	wr_reg(PSRAM_LCDL_CTRL, 0x20001000 | (3 << 24) | (0 << 26));
	wr_reg(PSRAM_LCDL_CTRL, 0x20001000 | (3 << 24) | (0 << 26) | (1 << 31));
}
#endif

void psram_frequency_test(psram_set_t *psram_p_dev) {
	//window test
	//psram_amlogic_sw_training_test();
	uint32_t count = 0;
	serial_puts("\n ddr frequency test end");
	for (count = 96;count < 300;) {
		psram_p_dev->psram_frequency = count;
		//serial_puts("\n test psram clk==");
		//serial_put_dec(count );
		//serial_puts("MHz");
		bl2_print("\nDDR_SWEEP_clk== ",count,1," M");
		psram_re_init(psram_p_dev);
		//serial_puts(" 1232");
		//psram_window_test(psram_p_dev);
		//serial_puts(" result==");
		//serial_put_dec(psram_test(psram_p_dev->psram_size<<20));
		uint32_t pass_flag = (psram_test(psram_p_dev->psram_size << 20));

		serial_puts(" psram test frequency ");
		serial_put_dec(psram_p_dev->psram_frequency);
		if (pass_flag > 1) {
			serial_puts(" 0 skip");
		}
		if (pass_flag == 0) {
			serial_puts(" 1 pass");
		}
		if (pass_flag == 1) {
			serial_puts(" 2 fail");
		}
		serial_puts(" dll ");
		serial_put_dec(rd_reg(PSRAM_CLK_UI));
		count = count + 12;
	}
}

#if 1
#define A1_PWM 0xfe002400

	uint32_t tack[35][2] =
	{{ 1044,0x00000022},
	 { 1034,0x00010021},
	 { 1024,0x00020020},
	 { 1014,0x0003001f},
	 { 1004,0x0004001e},
	 { 994,0x0005001d},
	 { 983,0x0006001c},
	 { 973,0x0007001b},
	 { 963,0x0008001a},
	 { 953,0x00090019},
	 { 943,0x000a0018},
	 { 932,0x000b0017},
	 { 922,0x000c0016},
	 { 912,0x000d0015},
	 { 902,0x000e0014},
	 { 892,0x000f0013},
	 { 881,0x00100012},
	 { 871,0x00110011},
	 { 861,0x00120010},
	 { 851,0x0013000f},
	 { 841,0x0014000e},
	 { 831,0x0015000d},
	 { 820,0x0016000c},
	 { 810,0x0017000b},
	 { 800,0x0018000a},
	 { 790,0x00190009},
	 { 780,0x001a0008},
	 { 769,0x001b0007},
	 { 759,0x001c0006},
	 { 749,0x001d0005},
	 { 739,0x001e0004},
	 { 729,0x001f0003},
	 { 719,0x00200002},
	 { 708,0x00210001},
	 { 698,0x00220000},};

void psram_sweep_vddee_test(psram_set_t *psram_p_dev) {
	//uint32_t org_test_value = 0x00220000;
	//uint32_t c_step=0x0000ffff;
	uint8_t serch = 0;
	while (serch > 0)
	{
		wr_reg(A1_PWM, tack[serch][1]);
		//if (p_dev->ddr_global_message.stick_dmc_ddr_window_test_enable)
		{
		//serial_put_dec(tack[serch][0]);
		//serial_puts("mv\n");
		//bl2_print("\nread org_EE_voltage "tack[serch][0],1," mv \n");
		bl2_print("\nDDR_SWEEP_begin_EE_voltage psram ",tack[serch][0],1," mv bdlr ");
		serial_puts(" \n");
		//psram_frequency_test(psram_p_dev);
		psram_window_test(psram_p_dev);
		bl2_print("\nDDR_SWEEP_end_EE_voltage psram ",tack[serch][0],1," mv \n");
		}
		if (serch == 34)
		{//org_test_value = 0x00220000;
		  serch = 0;
		}
		else
		{
			serch++;
			//org_test_value = org_test_value-c_step;
		}
	}
}
#endif

uint32_t global_stick_g12_d2pll_cmd_enable;
void psram_extra_test(psram_set_t *psram_p_dev) {
	//window test
	//psram_amlogic_sw_training_test();
	if (global_stick_g12_d2pll_cmd_enable)
		psram_p_dev->psram_test_function[0] =
		PSRAM_WINDOW_TEST_INDEX | PSRAM_FREQUENCY_TEST_INDEX;
	#ifdef ENABLE_PSRAM_EXTRA_TEST
	//if (psram_p_dev->psram_test_function[0] == G12_D2PLL_CMD_SWEEP_EE_VOLTAGE_FREQUENCY_TABLE_TEST)
	//psram_sweep_vddee_test(psram_p_dev);
	if (psram_p_dev->psram_test_function[0] & PSRAM_WINDOW_TEST_INDEX)
		psram_window_test(psram_p_dev);
	if (psram_p_dev->psram_test_function[0] & PSRAM_FREQUENCY_TEST_INDEX)
		psram_frequency_test(psram_p_dev);
	#endif
}
#endif

void psram_config_winbond(char chip_logic_index, uint32_t psram_frequency)
{
	//CONFIG WINBOND PSRAM CONFIG0 register.
	uint32_t latency = 0;
	uint32_t data32 = 0;

	latency = psram_calculate_latency(chip_logic_index, psram_frequency);
	data32 = 1 | (1 << 2) | ((0) << 3) | \
			(((latency > 4) ? (latency - 5) : (latency + 11)) << 4) | \
			(0xf << 8) | (0 << 3) | (1 << 15);
	psram_wb_write_reg(0x1000, data32);
	psram_wait_status_ready();
}

void psram_config_winbond_sleep(void)
{
	uint32_t data32;
	data32 = rd_reg(PSRAM_STATUS);

		  //check psram idle
	if (data32 && 0x7 == 0x7)
		serial_puts("Psram : psram idle\n");
	else
		serial_puts("Warning : Psram busy, can't entry hybirf sleep mode !\n");

	//disable axi req before resend user-cmd ,or user cmd can not send out
	data32 = rd_reg(PSRAM_AXI_REQ_CTRL0);
	wr_reg(PSRAM_AXI_REQ_CTRL0, data32 & ~(1 << 31));
	psram_wb_write_reg(0x1002, 0xffa0);
	psram_wait_status_ready();
}

void psram_exit_winbond_sleep(void)
{
	psram_send_user_cmd(0x1002, 0xffa0, 2 << 16, (1 << 15) | (1 << 14) | 1, 0, 0xb8030000);
	psram_wait_status_ready();
	wr_reg(PSRAM_AXI_REQ_CTRL0, rd_reg(PSRAM_AXI_REQ_CTRL0) | (1 << 31));
}


char psram_init_winbond(psram_set_t *psram_p_dev) {
//#if 1	//winbond
	char init_pass = 0;
#if  ENABLE_WINBOND_PSRAM_PXP_DEBUG
#else
	char time_out_count = 0;
#endif
	//uint32_t data32=0;
	uint32_t latency = 0;
	//uint32_t init_freq=0;
	/* enable on real chip */
	//dmc_reset(psram_p_dev);
	serial_puts("\ninit winbond\n");
#if  ENABLE_WINBOND_PSRAM_PXP_DEBUG
	//psram_p_dev->psram_frequency = 166;
	psram_p_dev->psram_amlogic_protocol_id = PSRAM_CHIP_LOGIC_INDEX_WINBOND_3_CMD_W955D8MKY;
#else
	char auto_frequency = 0;

	if (psram_p_dev->psram_frequency == 0xffff) {
		auto_frequency = 1;
		psram_p_dev->psram_frequency = 166;//166;128
	}
	psram_p_dev->psram_frequency = 166;
	psram_p_dev->psram_amlogic_protocol_id = PSRAM_CHIP_LOGIC_INDEX_WINBOND_3_CMD_W955D8MKY;

	//wr_reg (AO_RESETCTRL_RESET0, (1<<4)|rd_reg(AO_RESETCTRL_RESET0));
	WINBOND_REINT:
	_udelay(1);
#endif
	psram_init_hw_bringup(psram_p_dev,
		PSRAM_BRINGUP_PRE_UDELAY | PSRAM_BRINGUP_PHY_DELAY | PSRAM_BRINGUP_POST_UDELAY10);
	latency = psram_calculate_latency(psram_p_dev->psram_amlogic_protocol_id,
		psram_p_dev->psram_frequency);
	psram_init_phy_regs(psram_p_dev->psram_amlogic_protocol_id);

	psram_config_winbond(psram_p_dev->psram_amlogic_protocol_id, psram_p_dev->psram_frequency);
	//read identification reg0
	psram_send_user_cmd(0x0000, 0x0, 2 << 16, (1 << 28) | (1 << 27) | (1 << 16), 0x00008000 | ((latency) << 8) | (latency - 2), (1 << 31) | (1 << 29) | (1 << 28) | (1 << 27) | (3 << 16) | 0);
#if  ENABLE_WINBOND_PSRAM_PXP_DEBUG
#else
	while ((((rd_reg(PSRAM_USER_CTRL0)) >> 30) & 1) == 0)
	{//add time out for  use winbond protocol to probe ap memory  then no dqs may result bus hang
		_udelay(10);
		time_out_count++;
		if (time_out_count > 10)
		{
			if (auto_frequency) {
				psram_p_dev->psram_frequency = 0xffff;
				auto_frequency = 0;
			}
			return init_pass;
		}
	};
#endif
	//serial_puts("\ninit winbond  5\n");
	psram_p_dev->psram_mr[1] = (rd_reg(PSRAM_CFG_STS)) & 0xff;
	//serial_puts("\ninit winbond  6\n");
	psram_p_dev->psram_mr[0] = (rd_reg(PSRAM_CFG_STS)) & 0xff;
	psram_p_dev->psram_mr[1] = ((rd_reg(PSRAM_CFG_STS)) >> 8) & 0xff;
	serial_puts("\npsram_mr[0]== ");
	serial_put_hex(psram_p_dev->psram_mr[0], 32);//0x8d
	serial_puts("\npsram_mr[1]== ");
	serial_put_hex(psram_p_dev->psram_mr[1], 32);
	serial_puts("\n");

	if ((psram_p_dev->psram_mr[0]) == (PSRAM_CHIP_LOGIC_INDEX_WINBOND_3_CMD_W955D8MKY_MR0)) {
		init_pass = 1;
		psram_p_dev->psram_size = 4;
		psram_p_dev->psram_amlogic_protocol_id = PSRAM_CHIP_LOGIC_INDEX_WINBOND_3_CMD_W955D8MKY;
	}
	if ((psram_p_dev->psram_mr[0]) == (PSRAM_CHIP_LOGIC_INDEX_WINBOND_3_CMD_W956D8MKY_MR0)) {
		init_pass = 1;
		psram_p_dev->psram_size = 8;
		psram_p_dev->psram_amlogic_protocol_id = PSRAM_CHIP_LOGIC_INDEX_WINBOND_3_CMD_W956D8MKY;
	}
	if ((psram_p_dev->psram_mr[0]) == (PSRAM_CHIP_LOGIC_INDEX_WINBOND_3_CMD_W966D8MKY_MR0)) {
		init_pass = 1;
		psram_p_dev->psram_size = 32;
		psram_p_dev->psram_amlogic_protocol_id = PSRAM_CHIP_LOGIC_INDEX_WINBOND_3_CMD_W966D8MKY;
	}
#if  ENABLE_WINBOND_PSRAM_PXP_DEBUG
#else
	if (!init_pass) {
		if (auto_frequency) {
			psram_p_dev->psram_frequency = 0xffff;
			auto_frequency = 0;
		}
		return init_pass;
	}
#endif
#if  ENABLE_WINBOND_PSRAM_PXP_DEBUG
#else
	if ((auto_frequency) && (psram_p_dev->psram_amlogic_protocol_id == PSRAM_CHIP_LOGIC_INDEX_WINBOND_3_CMD_W956D8MKY
	|| psram_p_dev->psram_amlogic_protocol_id == PSRAM_CHIP_LOGIC_INDEX_WINBOND_3_CMD_W966D8MKY)) {
		psram_p_dev->psram_frequency = 192;
		auto_frequency = 0;
		goto WINBOND_REINT;
	}
#endif

	psram_config_axi_interface(
		32 | (32 << 16) | (1 << 31),
		(latency << 16) | ((latency - 2) << 22) | (1 << 30) | (1 << 31),
		(0 << 16) | 0,
		latency | (1 << 6) | (2 << 12)| (3 << 24) | (1 << 30) | (1 << 31)
	);
	//psram_test(psram_p_dev); //test psram add and data size

	return init_pass;
}

#ifdef ENABLE_PSRAM_GET_AP_EID
static uint32_t psram_get_eid_half(uint32_t latency, uint32_t len, uint32_t ctrl0) {
	uint32_t eid = 0;
	for (char count = 0; count < 16; count++) {
		wr_reg(PSRAM_DBUF_0 + (count << 2), 0);
	}
	psram_send_user_cmd(0x00000006, 0, 0, (1 << 28) | (0 << 27) | (len << 16),
		(1 << 15) | (latency << 8) | (latency - 2), ctrl0);
	psram_wait_user_cmd();
	for (char count = 0; count < 16; count++) {
		uint32_t r = rd_reg(PSRAM_DBUF_0 + (count << 2));
		eid |= ((r & 1) << (31 - (count << 1))) |
			(((r >> 16) & 1) << (31 - (count << 1) - 1));
	}
	return eid;
}
#endif

char psram_init_ap(psram_set_t *psram_p_dev) {
#if 1
	serial_puts("\ninit ap\n");
	char init_pass = 0;
	psram_p_dev->psram_amlogic_protocol_id = PSRAM_CHIP_LOGIC_INDEX_AP_MEMORY_3_CMD;
	if (psram_p_dev->psram_frequency == 0xffff) {
		psram_p_dev->psram_frequency = 192;
	}
	//uint32_t data32=0;
	uint32_t latency = 0;
	uint32_t wr_latency=0;

	latency = psram_calculate_latency(psram_p_dev->psram_amlogic_protocol_id, psram_p_dev->psram_frequency);
	wr_latency = psram_calculate_wl_latency(psram_p_dev->psram_amlogic_protocol_id, psram_p_dev->psram_frequency);

	psram_init_hw_bringup(psram_p_dev, PSRAM_BRINGUP_AO_RESET);
	psram_init_phy_regs(PSRAM_CHIP_LOGIC_INDEX_AP_MEMORY_3_CMD);
#if 0
	extern void watchdog_disable(void);
	extern void debug_rom(char * file, int line);
	serial_puts("---------- AMLOGIC INTERNAL USED ONLY ----------\n\n");
	serial_puts("---------- BL2 WITH DEBUGROM SUPPORT  ----------\n\n");
	watchdog_disable();
	debug_rom(__FILE__,__LINE__);
#endif

	psram_ap_write_mr(0x00000000, ((latency - 3) << 2) | psram_calculate_drv((psram_p_dev->psram_amlogic_protocol_id), psram_p_dev->psram_dram_drv));

	//configure MR4.
	static const uint8_t ap_mr4s[] = {6, 2, 4, 0, 7, 3, 5, 1};
	char mr4 = ap_mr4s[psram_find_ap_freq_index(psram_p_dev->psram_frequency)];

	psram_ap_write_mr(0x00000004, (mr4 << 5));

	//configure MR8.
	psram_ap_write_mr(0x00000008, 2);

	//read MR8
	psram_ap_read_mr(0x00000008, latency);
	psram_wait_user_cmd();
	//read MR1
	psram_ap_read_mr(0x00000001, latency);
	psram_wait_user_cmd();
	psram_p_dev->psram_mr[1] = (rd_reg(PSRAM_CFG_STS))&0xff;
	psram_p_dev->psram_mr[2] = ((rd_reg(PSRAM_CFG_STS)) >> 8)&0xff;
	serial_puts("\npsram_mr[1]== ");
	serial_put_hex(psram_p_dev->psram_mr[1], 32);//0x8d
	serial_puts("\npsram_mr[2]== ");
	serial_put_hex(psram_p_dev->psram_mr[2], 32);
	serial_puts("\n");
	//ddr_debug_serial_puts("DDR_debug", __FILE__, __LINE__);
	//ddr_debug_rom(__FILE__, __LINE__);
	if (psram_detect_ap_size(psram_p_dev)) {
		init_pass = 1;
	}
	else {
		return init_pass;
	}

#ifdef ENABLE_PSRAM_GET_AP_EID
	uint32_t EID[2];
	EID[0] = psram_get_eid_half(latency, 32, (1 << 31) | (1 << 29) | (1 << 28) | (1 << 27) | (3 << 16) | 0x4040);
	EID[1] = psram_get_eid_half(latency, 64, (1 << 31) | (1 << 29) | (1 << 28) | (1 << 27) | (3 << 16) | 0x4000);
	serial_puts("\nEID  ");
	serial_put_hex(EID[0], 32);
	serial_puts("  ");
	serial_put_hex(EID[1], 32);
#endif

	psram_config_axi_interface(
		32 | (32 << 16) | (0 << 31),
		((latency + 1) << 16) | ((latency + 1 - 2) << 22) | (1 << 28) | (1 << 31),
		(0 << 16) | 0x8080,
		wr_latency | (1 << 6) | (3 << 24) | (1 << 30) | (1 << 31)
	);
	//serial_puts("PSRAM_ACTIMING1==");
	//serial_put_hex(rd_reg(PSRAM_ACTIMING1), 32);//0x8d

#endif
	serial_puts("\nend init ap\n");
	return init_pass;
}

static psram_set_t psram_uboot_dev_storage;
static char psram_uboot_dev_ready;

char psram_uboot_drv_init(void)
{
	psram_set_t *psram_p_dev = &psram_uboot_dev_storage;
	char init_pass = 0;
	uint32_t data32;

	memset(psram_p_dev, 0, sizeof(*psram_p_dev));
	psram_uboot_dev_ready = 0;

	data32 = rd_reg(AO_PWRCTRL_FOCRST0);
	wr_reg(AO_PWRCTRL_FOCRST0, (data32 | (1 << 12)));
	wr_reg(AO_PWRCTRL_FOCRST0, (data32 & ~(1 << 12)));
	*P_AO_PWRCTRL_MEM_PD0 = 0x0;

	psram_p_dev->psram_soc_drv = psram_p_dev->psram_soc_drv ? psram_p_dev->psram_soc_drv : 48;
	psram_p_dev->psram_dram_drv = psram_p_dev->psram_dram_drv ? psram_p_dev->psram_dram_drv : 48;

	{
		static const unsigned char init_delays[32] = {
			1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 32, 32, 16, 16, 1
		};
		unsigned char *dst = (unsigned char *)psram_p_dev->psram_pin_dq_in_delay;
		int i;

		for (i = 0; i < 32; i++)
			dst[i] = init_delays[i];
	}

	*P_AO_RESETCTRL_RESET0_LEVEL &= ~(1 << 30);
	_udelay(1);
	*P_AO_RESETCTRL_RESET0_LEVEL |= (1 << 30);
	psram_p_dev->psram_frequency = 166;
	init_pass = psram_init_winbond(psram_p_dev);
	if (!init_pass)
		init_pass = psram_init_ap(psram_p_dev);

	if (init_pass) {
		psram_uboot_dev_ready = 1;
		printf("psram init ok, freq=%uMHz size=%uMB\n",
		       psram_p_dev->psram_frequency, psram_p_dev->psram_size);
	} else {
		printf("psram init failed\n");
	}
	return init_pass;
}

psram_set_t *psram_uboot_get_dev(void)
{
	return psram_uboot_dev_ready ? &psram_uboot_dev_storage : NULL;
}

static int psram_uboot_set_freq(psram_set_t *pdev, unsigned int freq_mhz)
{
	char init_pass;

	if (freq_mhz == 0 || freq_mhz > 300) {
		printf("freq %u MHz invalid (use 1-300)\n", freq_mhz);
		return 1;
	}

	printf("psram freq %u -> %u MHz, re-init...\n",
	       pdev->psram_frequency, freq_mhz);
	pdev->psram_frequency = freq_mhz;
	init_pass = 0;
	switch (pdev->psram_amlogic_protocol_id) {
	case PSRAM_CHIP_LOGIC_INDEX_WINBOND_3_CMD_W955D8MKY:
	case PSRAM_CHIP_LOGIC_INDEX_WINBOND_3_CMD_W956D8MKY:
	case PSRAM_CHIP_LOGIC_INDEX_WINBOND_3_CMD_W966D8MKY:
		init_pass = psram_init_winbond(pdev);
		break;
	case PSRAM_CHIP_LOGIC_INDEX_AP_MEMORY_3_CMD:
		init_pass = psram_init_ap(pdev);
		break;
	default:
		printf("unknown protocol id 0x%x, run psram_test 0x1 first\n",
		       pdev->psram_amlogic_protocol_id);
		return 1;
	}

	if (!init_pass) {
		printf("psram re-init at %u MHz failed\n", freq_mhz);
		return 1;
	}
	printf("psram freq %u MHz ok, size=%uMB\n", pdev->psram_frequency, pdev->psram_size);
	return 0;
}

/* ---------- psram_test U-Boot command ---------- */
#define PSRAM_TEST_CMD_INIT		0x1
#define PSRAM_TEST_CMD_FREQ_SWEEP	0x2
#define PSRAM_TEST_CMD_WINDOW		0x3
#define PSRAM_TEST_CMD_SET_FREQ		0x4

static int do_psram_test_cmd(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	unsigned long subcmd;
	unsigned long freq_mhz;
	psram_set_t *pdev;
	char *endp;

	if (argc < 2) {
		printf("psram_test subcmd [arg]\n");
		printf("  0x1 - init PSRAM\n");
		printf("  0x2 - frequency sweep\n");
		printf("  0x3 - delay window test\n");
		printf("  0x4 <mhz> - switch PSRAM frequency and re-init\n");
		printf("  psram_test_base - test psram base address %x\n", sram_test_base);
		return 1;
	}

	subcmd = simple_strtoul(argv[1], &endp, 0);
	if (argv[1][0] == 0 || *endp != 0) {
		printf("invalid subcmd\n");
		return 1;
	}

	switch (subcmd) {
	case PSRAM_TEST_CMD_INIT:
		return psram_uboot_drv_init() ? 0 : 1;
	case PSRAM_TEST_CMD_FREQ_SWEEP:
		pdev = psram_uboot_get_dev();
		if (!pdev) {
			printf("run psram_test 0x1 first\n");
			return 1;
		}
		psram_frequency_test(pdev);
		return 0;
	case PSRAM_TEST_CMD_WINDOW:
		pdev = psram_uboot_get_dev();
		if (!pdev) {
			printf("run psram_test 0x1 first\n");
			return 1;
		}
		psram_re_init(pdev);
		psram_window_test(pdev);
		return 0;
	case PSRAM_TEST_CMD_SET_FREQ:
		pdev = psram_uboot_get_dev();
		if (!pdev) {
			printf("run psram_test 0x1 first\n");
			return 1;
		}
		if (argc < 3) {
			printf("usage: psram_test 0x4 <freq_mhz>\n");
			printf("current freq=%u MHz\n", pdev->psram_frequency);
			return 1;
		}
		freq_mhz = simple_strtoul(argv[2], &endp, 0);
		if (argv[2][0] == 0 || *endp != 0) {
			printf("invalid freq\n");
			return 1;
		}
		return psram_uboot_set_freq(pdev, freq_mhz) ? 1 : 0;
	default:
		printf("unknown subcmd 0x%lx\n", subcmd);
		return 1;
	}
}

U_BOOT_CMD(psram_test, 6, 1, do_psram_test_cmd,
	   "psram_test subcmd [arg]",
	   "psram_test 0x1              - init PSRAM\n"
	   "psram_test 0x2              - frequency sweep\n"
	   "psram_test 0x3              - window test\n"
	   "psram_test 0x4 <freq_mhz>   - switch frequency\n");
