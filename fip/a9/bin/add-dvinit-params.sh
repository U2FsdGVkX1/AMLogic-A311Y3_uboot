#!/bin/bash
#
# Copyright (C) 2025 Amlogic, Inc. All rights reserved.
#

set -e
# set -x

#
# Variables
#

EXEC_BASEDIR=$(dirname $(readlink -f $0))
ACPU_IMAGETOOL=${EXEC_BASEDIR}/../binary-tool/acpu-imagetool

BASEDIR_TOP=$(readlink -f ${EXEC_BASEDIR}/..)

#
# Settings
#

BASEDIR_TEMPLATE=$1
BASEDIR_INPUT_BB1ST_REF=$2
BASEDIR_INPUT_BL2E=$3
BASEDIR_PAYLOAD=$4
BASEDIR_OUTPUT_BLOB=$5
SOC_FAMILY=$6

#
# Arguments
#

BB1ST_ARGS="${BB1ST_ARGS}"


if [ "$CS_SIGNING_SCHEME" == "rsa" ]; then
  BB1ST_ARGS="${BB1ST_ARGS} --chipset-authen-algorithm=rsa,none"
elif [ "$CS_SIGNING_SCHEME" == "rsa-mldsa" ]; then
  BB1ST_ARGS="${BB1ST_ARGS} --chipset-authen-algorithm=rsa,mldsa"
elif [ "$CS_SIGNING_SCHEME" == "mldsa" ]; then
  BB1ST_ARGS="${BB1ST_ARGS} --chipset-authen-algorithm=none,mldsa"
fi
if [ "$DV_SIGNING_SCHEME" == "rsa" ]; then
  BB1ST_ARGS="${BB1ST_ARGS} --device-authen-algorithm=rsa,none"
elif [ "$DV_SIGNING_SCHEME" == "rsa-mldsa" ]; then
  BB1ST_ARGS="${BB1ST_ARGS} --device-authen-algorithm=rsa,mldsa"
elif [ "$DV_SIGNING_SCHEME" == "mldsa" ]; then
  BB1ST_ARGS="${BB1ST_ARGS} --device-authen-algorithm=none,mldsa"
fi

### Input: template ###
BB1ST_ARGS="${BB1ST_ARGS} --infile-template-bb1st=${BASEDIR_TEMPLATE}"
BB1ST_ARGS="${BB1ST_ARGS} --infile-blob-bb1st-ref=${BASEDIR_INPUT_BB1ST_REF}"

### Input: payload ###
BB1ST_ARGS="${BB1ST_ARGS} --infile-blob-bl2e=${BASEDIR_INPUT_BL2E}"

### Input: payloads ###
BB1ST_ARGS="${BB1ST_ARGS} --infile-dvinit-params=${BASEDIR_PAYLOAD}"

BB1ST_ARGS="${BB1ST_ARGS} --scs-family=a9"

### Output: blobs ###
BB1ST_ARGS="${BB1ST_ARGS} --outfile-bb1st=${BASEDIR_OUTPUT_BLOB}"

#
# Main
#

set -x

${ACPU_IMAGETOOL} \
        create-boot-blobs \
        ${BB1ST_ARGS}

# vim: set tabstop=2 expandtab shiftwidth=2:
