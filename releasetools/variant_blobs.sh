#!/sbin/sh
#
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

copy()
{
  LD_LIBRARY_PATH=/system/lib /system/bin/toybox cp --preserve=a "$1" "$2"
}

VAR_SELECT_HOOK=/tmp/install/bin/variant_blobs_hook.sh

# Detect variant and copy its specific-blobs
BOOTLOADER=`getprop ro.bootloader`

if [ -f $VAR_SELECT_HOOK ] ; then
  . $VAR_SELECT_HOOK
else
  echo "Could not find variant selector hook: $VAR_SELECT_HOOK"
  exit 1
fi

echo "Device with bootloader $BOOTLOADER requires $VARIANT blobs"

BLOBBASE=/system/blobs/$VARIANT

if [ -d $BLOBBASE ]; then
  cd $BLOBBASE

  for FILE in `find . -type f` ; do
    mkdir -p `dirname /system/$FILE`
    copy $FILE /system/$FILE
  done

  for FILE in bin/* ; do
    chmod 755 /system/$FILE
  done
else
  echo "Expected source directory does not exist!"
  exit 1
fi

exit 0
