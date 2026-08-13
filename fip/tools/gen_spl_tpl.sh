# SPDX-License-Identifier: GPL-2.0
#
# Copyright (C) 2025 Amlogic, Inc. All rights reserved.
#

#!/bin/bash

source fip/variables.sh

set -e

echo "*****generate spl and tpl*****"

if [ $# -ne 6 ]; then
	echo "input parameter error"
fi

if [ ! -d ${BUILD_FOLDER}spl_tpl ]; then
	mkdir ${BUILD_FOLDER}spl_tpl
fi

postfix=$1
bl30_size=$2
bl31_size=$3
bl32_size=$4
bl40_size=$5
bl33_size=$6

bootloader="${FIP_BUILD_FOLDER}u-boot.bin${postfix}"
device_fip="${FIP_BUILD_FOLDER}device-fip.bin${postfix}"
device_fip_hdr="${FIP_BUILD_FOLDER}device-fip-header.bin"
spl_sto="${BUILD_FOLDER}spl_tpl/spl.sto.bin${postfix}"
spl_usb="${BUILD_FOLDER}spl_tpl/spl.usb.bin${postfix}"
tpl="${BUILD_FOLDER}spl_tpl/tpl.bin${postfix}"

bl3x_size=$((${bl31_size}+${bl32_size}+${bl40_size}))
devfip_hdr_size=`stat -c %s ${device_fip_hdr}`
bl40_off=$((devfip_hdr_size+$2+4096))
uboot_size=`stat -c %s ${bootloader}`
devfip_size=`stat -c %s ${device_fip}`
spl_size=$((uboot_size-devfip_size))

dd if=${bootloader} of=${spl_sto} bs=${spl_size} count=1 conv=notrunc status=none
dd if=${device_fip} of=${spl_sto}.tmp skip=${bl40_off} bs=1 count=${bl3x_size} conv=notrunc status=none

cat ${spl_sto}.tmp >> ${spl_sto}

dd if=${device_fip} of=${tpl} bs=1 count=${bl40_off} conv=notrunc status=none
dd if=${device_fip} of=${tpl}.tmp skip=$((bl40_off+bl3x_size)) bs=1 count=$((bl33_size+4096)) conv=notrunc status=none
cat ${tpl}.tmp >> ${tpl}

rm ${spl_sto}.tmp ${tpl}.tmp

#generate spl.usb.bin
cp ${bootloader} ${spl_usb} -f

