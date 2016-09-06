/*
 * Copyright (C) 2016 The Android Open Source Project
 * Copyright (C) 2016 The CyanogenMod Project
 * Copyright (C) 2016 The Mokee Project
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you
 *  may not use this file except in compliance with the License.  You may
 *  obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 *  implied.  See the License for the specific language governing
 *  permissions and limitations under the License.
 */
#ifndef __FINGERPRINT_TZ_H_
#define __FINGERPRINT_TZ_H_

#include "fingerprint_common.h"
#include "QSEEComAPI.h"

/* QSEE cmd */
typedef enum trust_zone_cmd_id_t {
    vfmProvision = 0x1,
    vfmInitialize = 0x2,
    vfmUninitialize = 0x3,
    vfmDeviceInitialize = 0x4,
    vfmDeviceCalibrate = 0x5,
    vfmAuthSessionBegin = 0x6,
    vfmAuthSessionEnd = 0x7,
    vfmCaptureStart = 0x8,
    vfmCaptureReadData = 0x9,
    vfmCaptureProcessData = 0xa,
    vfmCaptureAbort = 0xb,
    vfmCaptureGetStatus = 0xc,
    /* QSEE cmd id 0xd is unused. */
    vfmCaptureFlushImage = 0xe,
    vfmEnrollBegin = 0xf,
    vfmEnrollAddImage = 0x10,
    vfmEnrollFinish = 0x11,
    vfmEnrollmentPasswordSet = 0x12,
    vfmEnrollmentPasswordVerify = 0x13,
    vfmMatchImageToTemplates = 0x14,
    vfmPayloadBind = 0x15,
    vfmPayloadRelease = 0x16,
    vfmVendorDefinedOperation = 0x17
}trust_zone_cmd_id_t;

typedef enum trust_zone_vendor_cmd_id_t {
    vendorUnknown0 = 0x0,
    vendorGetVersion = 0x1,//0x10
    vendorUnknownA = 0xa,
    vendorGetAuthToken = 0x14,
    vendorEnterAuthSession = 0x15,
    vendorUpdateCalData = 0x17
}trust_zone_vendor_cmd_id_t;

typedef struct trust_zone_normal_cmd_t {
    trust_zone_cmd_id_t cmd;
    int len;
    int zero;
    char data[BUFFER_LENGTH - 12];
}trust_zone_base_cmd_t;

typedef struct trust_zone_vendor_cmd_t {
    trust_zone_cmd_id_t cmd;
    trust_zone_vendor_cmd_id_t vendor_cmd;
    int len;
    int zero;
    char data[BUFFER_LENGTH - 16];
}trust_zone_vendor_cmd_t;

typedef struct trust_zone_2x_cmd_t {
    trust_zone_cmd_id_t cmd;
    int len;
    int zero;
    char data[BUFFER_LENGTH * 2 - 12];
}trust_zone_2x_cmd_t;

typedef struct trust_zone_3x_cmd_t {
    trust_zone_cmd_id_t cmd;
    int len;
    int zero;
    char data[BUFFER_LENGTH * 3 - 12];
}trust_zone_3x_cmd_t;

typedef struct trust_zone_5x_cmd_t { //only use on vfmMatchImageToTemplates
    trust_zone_cmd_id_t cmd;
    int len;
    int zero;
    int unknown[2];
    time_t time_now;
    char data[563304];
}trust_zone_5x_cmd_t;

typedef struct trust_zone_normal_result_t {
    int zero;
    int result;
    int data[BUFFER_LENGTH/4 - 2];
}trust_zone_normal_result_t;

typedef struct trust_zone_2x_result_t {
    int zero;
    int result;
    int data[(BUFFER_LENGTH * 2)/4 - 2];
}trust_zone_2x_result_t;

#define FINGERPRINT_ERROR_HW_UNAVAILABLE (1)
#define FINGERPRINT_ERROR_UNABLE_TO_PROCESS (2)
#define FINGERPRINT_ERROR_TIMEOUT (3)
#define FINGERPRINT_ERROR_NO_SPACE (4)
#define FINGERPRINT_ERROR_CANCELED (5)
#define FINGERPRINT_ERROR_UNABLE_TO_REMOVE (6)
#define FINGERPRINT_ERROR_VENDOR_BASE (1000)

worker_state_t get_tz_state();
void set_tz_state(worker_state_t state);

int vcs_update_cal_data();
int vcs_check_state();
int vcs_start_capture();
void* vcs_authenticate(void* vdev);
void* vcs_enroll(void* vdev);
int vcs_start_authenticate(void *vdev);
int vcs_start_enroll(void *vdev, uint32_t timeout);
int vcs_get_enrolled_finger_num();
int vcs_update_auth_token();
int vcs_start_auth_session();
int vcs_stop_auth_session();
int vcs_resume();
int vcs_uninit();
int vcs_init();

#endif /* __FINGERPRINT_TZ_H_ */
