#!/bin/bash
#
# Copyright (C) 2016 The CyanogenMod Project
# Copyright (C) 2017 The LineageOS Project
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

if [ $# -eq 0 ]; then
    SRC=adb
else
    if [ $# -eq 1 ]; then
        SRC=$1
    else
        echo "$0: bad number of arguments"
        echo ""
        echo "usage: $0 [PATH_TO_EXPANDED_ROM]"
        echo ""
        echo "If PATH_TO_EXPANDED_ROM is not specified, blobs will be extracted from"
        echo "the device using adb pull."
        exit 1
    fi
fi

# Initialize the helper for common device
setup_vendor "$DEVICE_COMMON" "$VENDOR" "$CM_ROOT" true

extract "$MY_DIR"/common-proprietary-files.txt "$SRC"
extract "$MY_DIR"/common-proprietary-files-pn547.txt "$SRC"
extract "$MY_DIR"/common-proprietary-files-ril-m.txt "$SRC"

COMMON_BLOB_ROOT="$CM_ROOT"/vendor/"$VENDOR"/"$DEVICE_COMMON"/proprietary

MMCAMERA2_SENSOR_MODULES="$COMMON_BLOB_ROOT"/vendor/lib/libmmcamera2_sensor_modules.so
sed -i 's|system/etc|vendor/etc|g;
        s|/system/lib|/vendor/lib|g;
        s|/system/cameradata|/vendor/cameradata|g' "$MMCAMERA2_SENSOR_MODULES"

THERMAL_ENGINE="$COMMON_BLOB_ROOT"/vendor/bin/thermal-engine
sed -i 's|/system/etc|/vendor/etc|g' "$THERMAL_ENGINE"

# Reinitialize the helper for device
setup_vendor "$DEVICE" "$VENDOR" "$CM_ROOT"

for BLOB_LIST in "$MY_DIR"/../$DEVICE/device-proprietary-files*.txt; do
    extract $BLOB_LIST "$SRC"
done

./../msm8974-common/extract-files.sh $@

"$MY_DIR"/setup-makefiles.sh
