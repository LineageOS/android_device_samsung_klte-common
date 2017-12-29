#
# Copyright (C) 2016 The Mokee Project
# Copyright (C) 2016 The CyanogenMod Project
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

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := android.hardware.biometrics.fingerprint@2.1-service.klte
LOCAL_INIT_RC := android.hardware.biometrics.fingerprint@2.1-service.klte.rc
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_SRC_FILES := \
    service.cpp \
    BiometricsFingerprint.cpp \
    fingerprint.c \
    fingerprint_tz.c \
    QSEEComAPI.c \
    hash.c

LOCAL_C_INCLUDES += \
    external/sqlite/dist

LOCAL_SHARED_LIBRARIES := \
    liblog \
    libsqlite \
    libcutils \
    libdl \
    libxml2 \
    libhidlbase \
    libhidltransport \
    libhardware \
    libutils \
    android.hardware.biometrics.fingerprint@2.1

LOCAL_MODULE_TAGS := optional
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_EXECUTABLE)
