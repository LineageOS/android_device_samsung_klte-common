#!/bin/bash
#
# Copyright (C) 2016 The CyanogenMod Project
# Copyright (C) 2017-2021 The LineageOS Project
#
# SPDX-License-Identifier: Apache-2.0
#

set -e

if [ -z "${DEVICE_COMMON}" ]; then
    echo ""
    echo "error: This is a script in a common tree. Please execute" $(basename $0) "from a device tree."
    echo ""
    exit 1
fi

# Load extract_utils and do some sanity checks
MY_DIR="${BASH_SOURCE%/*}"
if [[ ! -d "${MY_DIR}" ]]; then MY_DIR="${PWD}"; fi

ANDROID_ROOT="${MY_DIR}/../../.."

HELPER="${ANDROID_ROOT}/tools/extract-utils/extract_utils.sh"
if [ ! -f "${HELPER}" ]; then
    echo "Unable to find helper script at ${HELPER}"
    exit 1
fi
source "${HELPER}"

# Initialize the helper for common
setup_vendor "${DEVICE_COMMON}" "${VENDOR}" "${ANDROID_ROOT}" true

# Warning headers and guards
write_headers "klte klteactivexx kltechn kltechnduo klteduos kltedv kltekdi kltekor"

# The standard common blobs
write_makefiles "${MY_DIR}/common-proprietary-files.txt" true

# Finish
write_footers

# This is the "common" M radio/QC framework stack. Most variants will inherit
# this file. If they cannot then they will ship their own stack
PRODUCTMK="$(echo $PRODUCTMK | sed -e 's|-vendor.mk|-vendor-ril-m.mk|g')"
write_makefile_header "$PRODUCTMK"
parse_file_list "${MY_DIR}/common-proprietary-files-ril-m.txt"
write_product_copy_files true

export BOARD_COMMON=msm8974-common

"./../../${VENDOR}/${BOARD_COMMON}/setup-makefiles.sh" "$@"
