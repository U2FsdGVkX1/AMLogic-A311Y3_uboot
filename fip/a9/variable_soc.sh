#!/bin/bash
#
# Copyright (C) 2025 Amlogic, Inc. All rights reserved.
#

# static
declare BLX_BIN_SUB_CHIP="${CONFIG_CHIPSET_NAME}"

if [ -n "${SCRIPT_ARG_CHIPSET_VARIANT}" ]; then
	declare CHIPSET_VARIANT_SUFFIX=".${SCRIPT_ARG_CHIPSET_VARIANT}"
elif [ -n "${CONFIG_CHIPSET_VARIANT}" ]; then
	declare CHIPSET_VARIANT_SUFFIX=".${CONFIG_CHIPSET_VARIANT}"
else
	declare CHIPSET_VARIANT_SUFFIX=""
fi
declare BL2_MAX_SIZE=272384

declare -a BLX_NAME=("bl2"	\
		     "bl2"	\
		     "bl2e"	\
		     "bl2e"	\
		     "bl2x"	\
		     "bl31"	\
		     "bl32"	\
		     "bl40"	\
		     "bl30")

declare -a BLX_SRC_FOLDER=("bl2/core"		\
			   "bl2/core"		\
			   "bl2/ree"		\
			   "bl2/ree"		\
			   "bl2/tee"		\
			   "bl31/bl31_2.7/src"	\
			   "bl32/bl32_4.4/src"	\
			   "NULL"		\
			   "bl30/src_ao"	\
			   "bl33")

declare -a BLX_BIN_FOLDER=("bl2/bin"		\
			   "bl2/bin"		\
			   "bl2/bin"		\
			   "bl2/bin"		\
			   "bl2/bin"		\
			   "bl31/bl31_2.7/bin"	\
			   "bl32/bl32_4.4/bin"\
			   "bl40/bin"		\
			   "bl30/bin_ao")

if [ "y" == "${CONFIG_BUILD_UNSIGN}" ]; then
declare -a BLX_BIN_NAME=("bl2.bin.sto"	\
			    "bl2.bin.usb"	\
			    "bl2e.bin.sto"	\
			    "bl2e.bin.usb"	\
			    "bl2x.bin"		\
			    "bl31.bin"		\
			    "bl32.bin"		\
			    "bl40.bin"		\
			    "NULL")

elif [ -n "$CHIPSET_VARIANT_SUFFIX" ]; then
declare -a BLX_BIN_NAME=("bb1st.sto${CHIPSET_VARIANT_SUFFIX}.bin.signed.${DV_SIGNING_SCHEME}.${CS_SIGNING_SCHEME}"     \
			 "bb1st.usb${CHIPSET_VARIANT_SUFFIX}.bin.signed.${DV_SIGNING_SCHEME}.${CS_SIGNING_SCHEME}"     \
			 "blob-bl2e.sto${CHIPSET_VARIANT_SUFFIX}.bin.signed.${DV_SIGNING_SCHEME}.${CS_SIGNING_SCHEME}" \
			 "blob-bl2e.usb${CHIPSET_VARIANT_SUFFIX}.bin.signed.${DV_SIGNING_SCHEME}.${CS_SIGNING_SCHEME}" \
			 "blob-bl2x.bin.signed.rsa-mldsa.${CS_SIGNING_SCHEME}"                              \
			 "blob-bl31.bin.signed.rsa-mldsa.${CS_SIGNING_SCHEME}"                              \
			 "blob-bl32.bin.signed.rsa-mldsa.${CS_SIGNING_SCHEME}"                              \
			 "blob-bl40.bin.signed.rsa-mldsa.${CS_SIGNING_SCHEME}"                              \
			 "bl30.bin")
else
declare -a BLX_BIN_NAME=("bb1st.sto.bin.signed.rsa-mldsa.${CS_SIGNING_SCHEME}"     \
			 "bb1st.usb.bin.signed.rsa-mldsa.${CS_SIGNING_SCHEME}"     \
			 "blob-bl2e.sto.bin.signed.rsa-mldsa.${CS_SIGNING_SCHEME}" \
			 "blob-bl2e.usb.bin.signed.rsa-mldsa.${CS_SIGNING_SCHEME}" \
			 "blob-bl2x.bin.signed.rsa-mldsa.${CS_SIGNING_SCHEME}"                              \
			 "blob-bl31.bin.signed.rsa-mldsa.${CS_SIGNING_SCHEME}"                              \
			 "blob-bl32.bin.signed.rsa-mldsa.${CS_SIGNING_SCHEME}"                              \
			 "blob-bl40.bin.signed.rsa-mldsa.${CS_SIGNING_SCHEME}"                              \
			 "bl30.bin")
fi

declare -a BLX_BIN_SIZE=("272384"	\
			 "272384"	\
			 "116912"	\
			 "116912"	\
			 "108720"	\
			 "266240"	\
			 "528384"	\
			 "102400"	\
			 "NULL")

declare BL30_BIN_SIZE="102400"	# 100k
declare BL33_BIN_SIZE="1572864"
declare DEV_ACS_BIN_SIZE="7168"
declare -a BLX_RAWBIN_NAME=("bl2.bin.sto"	\
			    "bl2.bin.usb"	\
			    "bl2e.bin.sto"	\
			    "bl2e.bin.usb"	\
			    "bl2x.bin"		\
			    "bl31.bin"		\
			    "bl32.bin"		\
			    "bl40.bin"		\
			    "NULL")

declare -a BLX_IMG_NAME=("NULL"	\
			 "NULL"	\
			 "NULL"	\
			 "NULL"	\
			 "NULL"	\
			 "NULL"	\
			 "NULL"	\
			 "NULL")

declare -a BLX_NEEDFUL=("true"	\
			"true"	\
			"true"	\
			"true"	\
			"true"	\
			"ture"	\
			"true"	\
			"true")

declare -a BLX_SRC_GIT=("bootloader/amlogic-advanced-bootloader/core" \
			"bootloader/amlogic-advanced-bootloader/core" \
			"bootloader/amlogic-advanced-bootloader/ree" \
			"bootloader/amlogic-advanced-bootloader/ree" \
			"bootloader/amlogic-advanced-bootloader/tee" \
			"ARM-software/arm-trusted-firmware" \
			"OP-TEE/optee_os" \
			"firmware/aocpu" \
			"uboot")

declare -a BLX_BIN_GIT=("firmware/bin/bl2" \
			"firmware/bin/bl2" \
			"firmware/bin/bl2" \
			"firmware/bin/bl2" \
			"firmware/bin/bl2" \
			"firmware/bin/bl31"\
			"firmware/bin/bl32"\
			"firmware/bin/b40")

# blx priority. null: default, source: src code, others: bin path
declare -a BIN_PATH=("null"	\
		     "null"	\
		     "null"	\
		     "null"	\
		     "null"	\
		     "null"	\
		     "null"	\
		     "null"	\
		     "source")

# variables
declare -a CUR_REV # current version of each blx
declare -a BLX_READY=("false",	\
		      "false",	\
		      "false",	\
		      "false",	\
		      "false",	\
		      "false",	\
		      "false",	\
		      "false",	\
		      "false") # blx build/get flag

# package variables
declare BL33_COMPRESS_FLAG=""
declare BL3X_SUFFIX="bin"
declare V3_PROCESS_FLAG=""
declare FIP_ARGS=""
declare AML_BL2_NAME=""
declare AML_KEY_BLOB_NAME=""
declare FIP_BL32_PROCESS=""
declare BOOT_SIG_FLAG=""
declare EFUSE_GEN_FLAG=""
declare DDRFW_TYPE=""

BUILD_PATH=${FIP_BUILD_FOLDER}
BUILD_PAYLOAD=${FIP_BUILD_FOLDER}/payload
CHIPSET_TEMPLATES_PATH="soc/templates"
CONFIG_DDR_FW=0
DDR_FW_NAME="aml_ddr.fw"

CONFIG_NEED_BL32=y
ADVANCED_BOOTLOADER=1

declare CONFIG_RTOS_SDK_ENABLE=1
declare CONFIG_SOC_NAME="a9"

if [ "${BL30_SELECT}" == "a9_by401" ]; then
	declare CONFIG_BOARD_PACKAGE_NAME="by401_a311y3"
elif [ "${BL30_SELECT}" == "a9_by408" ]; then
	declare CONFIG_BOARD_PACKAGE_NAME="by408_a311y3"
elif [ "${BL30_SELECT}" == "a9_by409" ]; then
	declare CONFIG_BOARD_PACKAGE_NAME="by409_a311y3"
elif [ "${BL30_SELECT}" == "a9_pxp" ]; then
	declare CONFIG_BOARD_PACKAGE_NAME="a9_pxp"
else
	echo "Error: bl30 board not found... abort. Please check 'BL30_SELECT' and fip!"
	exit 1
fi

declare BLOB_HDR_FOLDER=""
CONFIG_DYNAMIC_SZ=y
