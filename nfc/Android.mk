# Copyright 2016 The CyanogenMod Project
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

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := libpn547_fw.c
LOCAL_MODULE := libpn547_fw
LOCAL_MODULE_OWNER := nxp
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_MODULE_TAGS := optional
LOCAL_PACK_MODULE_RELOCATIONS := false
BOARD_NFC_HAL_SUFFIX := $(TARGET_BOARD_PLATFORM)
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)
