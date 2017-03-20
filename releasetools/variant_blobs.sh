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

better_copy()
{
  cp -dp "$1" "$2"
  # symlinks don't have a context
  if [ ! -L "$1" ]; then
    # it is assumed that every label starts with 'u:object_r' and has no white-spaces
    local context=`ls -Z "$1" | grep -o 'u:object_r:[^ ]*' | head -1`
    chcon -v "$context" "$2"
  fi
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
    better_copy $FILE /system/$FILE
  done

  for FILE in bin/* ; do
    chmod 755 /system/$FILE
  done
else
  echo "Expected source directory does not exist!"
  exit 1
fi

exit 0
