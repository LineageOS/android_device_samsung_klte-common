#!/bin/bash
#
# Copyright (C) 2016 The CyanogenMod Project
# Copyright (C) 2017-2020 The LineageOS Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

set -e

export INITIAL_COPYRIGHT_YEAR=2014

# Load extract_utils and do some sanity checks
MY_DIR="${BASH_SOURCE%/*}"
if [[ ! -d "$MY_DIR" ]]; then MY_DIR="$PWD"; fi

CM_ROOT="$MY_DIR"/../../..

HELPER="$CM_ROOT"/vendor/lineage/build/tools/extract_utils.sh
if [ ! -f "$HELPER" ]; then
    echo "Unable to find helper script at $HELPER"
    exit 1
fi
. "$HELPER"

# Initialize the helper for common device
setup_vendor "$DEVICE_COMMON" "$VENDOR" "$CM_ROOT" true

# Copyright headers and common guards
write_headers "klte klteactivexx kltechn kltechnduo klteduos kltedv kltekdi kltekor kltespr kltesprsports klteusc kltevzw"

write_makefiles "$MY_DIR"/common-proprietary-files.txt

write_footers

# This is the "common" M radio/QC framework stack. Most variants will inherit
# this file. If they cannot then they will ship their own stack
PRODUCTMK="$(echo $PRODUCTMK | sed -e 's|-vendor.mk|-vendor-ril-m.mk|g')"
write_makefile_header "$PRODUCTMK"
parse_file_list "$MY_DIR"/common-proprietary-files-ril-m.txt
write_product_copy_files

if [ ! -z $VARIANT_COPYRIGHT_YEAR ]; then
    export INITIAL_COPYRIGHT_YEAR=$VARIANT_COPYRIGHT_YEAR
fi

# Reinitialize the helper for device
setup_vendor "$DEVICE" "$VENDOR" "$CM_ROOT"

# Copyright headers and guards
write_headers

for BLOB_LIST in "$MY_DIR"/../$DEVICE/device-proprietary-files*.txt; do
    write_makefiles $BLOB_LIST
done

write_footers

./../msm8974-common/setup-makefiles.sh $@
