#!/bin/bash

set -e
# set -x


amfc_compress() {
    BASEDIR_TOP=$1
    SOC=$2
    INPUT=$3
    OUTPUT=$4

    TOOLBIN=${BASEDIR_TOP}/tools/zstd
    AMFC_ZSTD_HDR=${BASEDIR_TOP}/amfc_zstd_hdr.bin

    COMPRESS_ORG=${BASEDIR_TOP}/${INPUT}.org
    COMPRESS_ZSTD=${BASEDIR_TOP}/${INPUT}.zstd

    mv -f $INPUT $COMPRESS_ORG

    local magic rate
    case "$SOC" in
        t6d)
            magic="@ZSTD"
            rate="-9"
            ;;
        t6w|t6x|a9)
            magic="ZSTD"
            rate="-9"
            ;;
        c4|c5)
            magic="@ZST"
            rate="-19"
            ;;
        *)
            echo "SOC not found in amfc_compress.sh!"
            exit 1
            ;;
    esac

    ${TOOLBIN} ${COMPRESS_ORG} ${rate} -o ${COMPRESS_ZSTD}

    bin_org_size=`stat -c %s ${COMPRESS_ORG}`
    bin_zstd_size=`stat -c %s ${COMPRESS_ZSTD}`

    printf "%s" "${magic}" > "${AMFC_ZSTD_HDR}"

    printf "%02x%02x%02x%02x" $[(bin_org_size) & 0xff] \
    $[((bin_org_size) >> 8) & 0xff] $[((bin_org_size) >> 16) & 0xff] \
    $[((bin_org_size) >> 24) & 0xff] | xxd -r -ps >>  ${AMFC_ZSTD_HDR}

    printf "%02x%02x%02x%02x" $[(bin_zstd_size) & 0xff] \
    $[((bin_zstd_size) >> 8) & 0xff] $[((bin_zstd_size) >> 16) & 0xff] \
    $[((bin_zstd_size) >> 24) & 0xff] | xxd -r -ps >>  ${AMFC_ZSTD_HDR}

    cat ${AMFC_ZSTD_HDR} ${COMPRESS_ZSTD} > ${OUTPUT}
    rm ${AMFC_ZSTD_HDR} -f
}

if [[ $# -ne 4 ]]; then
    echo "Usage: \$0 <basedir_top> <soc> <input> <output>"
    exit 1
fi

amfc_compress "$@"

# vim: set tabstop=2 expandtab shiftwidth=2:
