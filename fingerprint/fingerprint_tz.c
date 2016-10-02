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

#define LOG_TAG "Fingerprint_tz"
#define LOG_NDEBUG 1

#include <errno.h>
#include <string.h>
#include <stdint.h>

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <dirent.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>

#include <cutils/log.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <dlfcn.h>

#include "fingerprint_tz.h"
#include "fp_klte.h"

extern vcs_sensor_t sensor;
trust_zone_t tz;

worker_state_t get_tz_state() {
    worker_state_t state;

    pthread_mutex_lock(&tz.lock);
    state = tz.state;
    pthread_mutex_unlock(&tz.lock);

    return state;
}

void set_tz_state(worker_state_t state) {
    pthread_mutex_lock(&tz.lock);
    tz.state = state;
    pthread_mutex_unlock(&tz.lock);
}

/*
 * cmd: vendorUpdateCalData
 */
int vcs_update_cal_data() {
    struct QSEECom_handle *handle = (struct QSEECom_handle *)(tz.qhandle);
    trust_zone_vendor_cmd_t *send_vendor_cmd = NULL;
    trust_zone_normal_result_t *resp = NULL;
    int ret = 0;
    send_vendor_cmd = (trust_zone_vendor_cmd_t *)handle->ion_sbuffer;
    resp = (trust_zone_normal_result_t *)(handle->ion_sbuffer + QSEECOM_ALIGN(sizeof(trust_zone_vendor_cmd_t)));

    memset(send_vendor_cmd, 0, QSEECOM_ALIGN(sizeof(*send_vendor_cmd)) + QSEECOM_ALIGN(sizeof(*resp)));
    send_vendor_cmd->cmd = vfmVendorDefinedOperation;
    send_vendor_cmd->vendor_cmd = vendorUpdateCalData;
    resp->data[0] = CALIBRATE_DATA_MAX_LENGTH;
    ret = QSEECom_send_cmd(handle, send_vendor_cmd, QSEECOM_ALIGN(sizeof(*send_vendor_cmd)), resp, QSEECOM_ALIGN(sizeof(*resp)));
    if (ret || resp->result) {
        ALOGE("Update Cal Data error");
        return ret;
    }
    tz.calibrate_len = resp->data[3] + 0xf;
    memcpy(&tz.calibrate_data, &resp->data[2], tz.calibrate_len);
    ALOGV("Sended vendorUpdateCalData");
    return ret;
}

int vcs_check_state() {
    struct QSEECom_handle *handle = (struct QSEECom_handle *)(tz.qhandle);
    trust_zone_base_cmd_t *send_cmd = NULL;
    trust_zone_normal_result_t *resp = NULL;
    int ret = 0;
    send_cmd = (trust_zone_base_cmd_t *)handle->ion_sbuffer;
    resp = (trust_zone_normal_result_t *)(handle->ion_sbuffer + QSEECOM_ALIGN(sizeof(int)));

    if (get_tz_state() == STATE_IDLE)
        return 1;
    if (get_tz_state() == STATE_CANCEL) {
        memset(send_cmd, 0, QSEECOM_ALIGN(sizeof(int)) + QSEECOM_ALIGN(sizeof(int)));
        send_cmd->cmd = vfmCaptureAbort;
        ret = QSEECom_send_cmd(handle, send_cmd, QSEECOM_ALIGN(sizeof(int)), resp, QSEECOM_ALIGN(sizeof(int)));
        set_tz_state(STATE_IDLE);
        return 1;
    }
    return 0;
}

/*
 * cmd: 1.vfmCaptureStart
 *      2.vfmCaptureReadData * N
 *      3.vfmCaptureProcessData
 */
int vcs_start_capture(void *vdev, time_t t) {
    struct QSEECom_handle *handle = (struct QSEECom_handle *)(tz.qhandle);
    trust_zone_base_cmd_t *send_cmd = NULL;
    trust_zone_normal_result_t *resp = NULL;
    int ret = 0;
    send_cmd = (trust_zone_base_cmd_t *)handle->ion_sbuffer;
    resp = (trust_zone_normal_result_t *)(handle->ion_sbuffer + QSEECOM_ALIGN(sizeof(trust_zone_base_cmd_t)));

    memset(send_cmd, 0, QSEECOM_ALIGN(sizeof(*send_cmd)) + QSEECOM_ALIGN(sizeof(int)));
    send_cmd->cmd = vfmCaptureStart;
    send_cmd->len = 0x1c;
    send_cmd->data[16] = 0x1;
    if (t) {
        *(time_t*)(&send_cmd->data[20]) = t;
    }
    send_cmd->data[24] = 0xc0;
    send_cmd->data[25] = 0x12;
    ret = QSEECom_send_cmd(handle, send_cmd, QSEECOM_ALIGN(sizeof(*send_cmd)), resp, QSEECOM_ALIGN(sizeof(int)));
    if (ret || resp->result) {
        ALOGE("Send vfmCaptureStart error");
        return ret;
    }
    vcs_update_cal_data();
    pthread_mutex_lock(&sensor.lock);
    sensor_capture_start();
    pthread_mutex_unlock(&sensor.lock);

    pthread_mutex_lock(&sensor.lock);
    while ((!vcs_check_state()) && !sensor.signal) {
        pthread_cond_wait(&sensor.cond, &sensor.lock);
    }
    pthread_mutex_unlock(&sensor.lock);
    if (vcs_check_state()) {
        return -1;
    }
    pthread_mutex_lock(&sensor.lock);
    if (sensor.signal == true) {
        sensor.signal = false;
    }
    pthread_mutex_unlock(&sensor.lock);

    while(1) {
        if (vcs_check_state()) {
            return -1;
        }
        resp = (trust_zone_normal_result_t *)(handle->ion_sbuffer + QSEECOM_ALIGN(sizeof(int)));
        memset(send_cmd, 0, QSEECOM_ALIGN(sizeof(int)) + QSEECOM_ALIGN(sizeof(*resp)));
        send_cmd->cmd = vfmCaptureReadData;
        send_cmd->len = 0x8000;
        resp->data[0] = 0xc;
        ret = QSEECom_send_cmd(handle, send_cmd, QSEECOM_ALIGN(sizeof(int)), resp, QSEECOM_ALIGN(sizeof(*resp)));
        if (ret || resp->result) {
            ALOGE("Send vfmCaptureReadData error");
            continue;
        }
        if (resp->data[2] == 2) {
            ALOGV("User's finger removed from sensor");
            break;
        }
        //usleep(200000);
    }
    resp = (trust_zone_normal_result_t *)(handle->ion_sbuffer + QSEECOM_ALIGN(sizeof(trust_zone_base_cmd_t)));
    memset(send_cmd, 0, QSEECOM_ALIGN(sizeof(*send_cmd)) + QSEECOM_ALIGN(sizeof(*resp)));
    send_cmd->cmd = vfmCaptureProcessData;
    send_cmd->len = 0x1c;
    send_cmd->data[16] = 0x1;
    *(time_t*)(&send_cmd->data[20]) = time(NULL);
    send_cmd->data[24] = 0xc0;
    send_cmd->data[25] = 0x12;
    resp->data[0] = 0xc;
    ret = QSEECom_send_cmd(handle, send_cmd, QSEECOM_ALIGN(sizeof(int)), resp, QSEECOM_ALIGN(sizeof(*resp)));
    if (ret) {
        ALOGE("Send vfmCaptureProcessData error");
        return -1;
    }
    if (resp->result != 0) {
        ALOGE("resp->result=%d",resp->result);
        send_acquired_notice(vdev, resp->result);
        return vcs_start_capture(vdev, time(NULL));
    }
    return 0;
}

/*
 * cmd: 1.vendorUnknown0
 *      2.vcs_start_capture
 *      3.vfmMatchImageToTemplates
 *      4.vfmPayloadRelease
 */

void* vcs_authenticate(void* vdev) {
    struct QSEECom_handle *handle = (struct QSEECom_handle *)(tz.qhandle);
    trust_zone_vendor_cmd_t *send_vendor_cmd = NULL;
    trust_zone_normal_result_t *resp = NULL;
    int ret = 0;
    send_vendor_cmd = (trust_zone_vendor_cmd_t *)handle->ion_sbuffer;
    resp = (trust_zone_normal_result_t *)(handle->ion_sbuffer + QSEECOM_ALIGN(sizeof(trust_zone_vendor_cmd_t)));
    int fingerindex = 0;
    int len = 0;
    int fake_fid = 0;

    memset(send_vendor_cmd, 0, QSEECOM_ALIGN(sizeof(*send_vendor_cmd)) + QSEECOM_ALIGN(sizeof(*resp)));
    send_vendor_cmd->cmd = vfmVendorDefinedOperation;
    send_vendor_cmd->vendor_cmd = vendorUnknown0;
    resp->data[0] = 0x4;
    ret = QSEECom_send_cmd(handle, send_vendor_cmd, QSEECOM_ALIGN(sizeof(*send_vendor_cmd)), resp, QSEECOM_ALIGN(sizeof(*resp)));
    if (resp->result) {
        ALOGE("%s:Send vendor unknown 0 cmd error", __FUNCTION__);
    }
    while (get_tz_state() == STATE_SCAN) {
        ret = vcs_start_capture(vdev, 0);
        if (ret == -1)
            goto out;
        trust_zone_5x_cmd_t *send_5x_cmd = NULL;
        trust_zone_2x_result_t *resp_2x = NULL;
        int idx = 1;
        send_5x_cmd = (trust_zone_5x_cmd_t *)handle->ion_sbuffer;
        resp_2x = (trust_zone_2x_result_t *)(handle->ion_sbuffer + QSEECOM_ALIGN(sizeof(trust_zone_5x_cmd_t)));

        memset(send_5x_cmd, 0, QSEECOM_ALIGN(sizeof(*send_5x_cmd)) + QSEECOM_ALIGN(sizeof(*resp_2x)));
        send_5x_cmd->cmd = vfmMatchImageToTemplates;
        send_5x_cmd->len = 0x14;
        send_5x_cmd->unknown[0] = 1;
        send_5x_cmd->unknown[1] = 100000;
        send_5x_cmd->time_now = time(NULL);
        send_5x_cmd->data[0] = 0x1;
        send_5x_cmd->data[102388] = vcs_get_enrolled_finger_num();
        len = 0;
        for (idx = 1; idx <= MAX_NUM_FINGERS; idx++)
            if (tz.finger[idx].exist) {
                len++;
                int address = 102392 + (len - 1) * 15364;
                send_5x_cmd->data[address] = 0x45;
                send_5x_cmd->data[address + 1] = 0x28;
                memcpy(&send_5x_cmd->data[address + 4], &tz.finger[idx].data, FINGER_DATA_MAX_LENGTH);
            }
        resp_2x->data[0] = 0x5c;
        resp_2x->data[25602] = 0x3000;
        ret = QSEECom_send_cmd(handle, send_5x_cmd, QSEECOM_ALIGN(sizeof(*send_5x_cmd)), resp_2x, QSEECOM_ALIGN(sizeof(*resp_2x)));
        if (ret) {
            ALOGE("%s:send vfmMatchImageToTemplates error", __FUNCTION__);
            send_error_notice(vdev, FINGERPRINT_ERROR_UNABLE_TO_PROCESS);
            goto out;
        }
        if (resp_2x->result != 0) {
            send_acquired_notice(vdev, resp_2x->result);
            continue;
        }
        fake_fid = (int)resp_2x->data[22] + 1;
        len = 0;
        for (idx = 1; idx <= MAX_NUM_FINGERS; idx++) {
            if (tz.finger[idx].exist) {
                len++;
                if (len == fake_fid) {
                    fingerindex = idx;
                    break;
                }
            }
        }
        ALOGV("Auth fingerindex=%d", fingerindex);
        //memcpy(&tz.finger[fingerindex].data, &resp_2x->data[102419], FINGER_DATA_MAX_LENGTH);
        //db_write_to_db(vdev, false, fingerindex);

        trust_zone_2x_cmd_t *send_2x_cmd = NULL;
        send_2x_cmd = (trust_zone_2x_cmd_t *)handle->ion_sbuffer;
        resp = (trust_zone_normal_result_t *)(handle->ion_sbuffer + QSEECOM_ALIGN(sizeof(trust_zone_2x_cmd_t)));
        memset(send_2x_cmd, 0, QSEECOM_ALIGN(sizeof(*send_2x_cmd)) + QSEECOM_ALIGN(sizeof(*resp)));
        send_2x_cmd->cmd = vfmPayloadRelease;
        send_2x_cmd->len = PAYLOAD_MAX_LENGTH;
        memcpy(&send_2x_cmd->data, &tz.finger[fingerindex].payload, PAYLOAD_MAX_LENGTH);
        resp->data[0] = 0x24;
        ret = QSEECom_send_cmd(handle, send_2x_cmd, QSEECOM_ALIGN(sizeof(*send_2x_cmd)), resp, QSEECOM_ALIGN(sizeof(*resp)));
        break;
    }
out:
    sensor_uninit();
    vcs_uninit();
    set_tz_state(STATE_IDLE);
    if (fingerindex) {
        send_authenticated_notice(vdev, fingerindex);
    }
    return NULL;
}

/*
 * cmd: 1.vfmEnrollBegin
 *      2.vendorUnknownA
 *      3.vcs_start_capture * 8
 *      4.vfmEnrollAddImage
 *      5.vfmEnrollFinish
 *      6.vfmPayloadBind
 *      7.vfmEnrollmentPasswordSet
 */

void* vcs_enroll(void* vdev) {
    int count = 8;
    struct QSEECom_handle *handle = (struct QSEECom_handle *)(tz.qhandle);
    trust_zone_base_cmd_t *send_cmd = NULL;
    trust_zone_vendor_cmd_t *send_vendor_cmd = NULL;
    trust_zone_normal_result_t *resp = NULL;
    int ret = 0;
    int i = 0;
    send_cmd = (trust_zone_base_cmd_t *)handle->ion_sbuffer;
    send_vendor_cmd = (trust_zone_vendor_cmd_t *)handle->ion_sbuffer;
    resp = (trust_zone_normal_result_t *)(handle->ion_sbuffer + QSEECOM_ALIGN(sizeof(trust_zone_base_cmd_t)));

    int idx = 1;
    for (idx = 1; idx <= MAX_NUM_FINGERS; idx++) {
        if (!tz.finger[idx].exist) {
            break;
        }
    }

    memset(send_cmd, 0, QSEECOM_ALIGN(sizeof(*send_cmd)) + QSEECOM_ALIGN(sizeof(int)));
    send_cmd->cmd = vfmEnrollBegin;
    send_cmd->data[102400] = idx;
    ret = QSEECom_send_cmd(handle, send_cmd, QSEECOM_ALIGN(sizeof(*send_cmd)), resp, QSEECOM_ALIGN(sizeof(int)));
    if (ret || resp->result) {
        ALOGE("send EnrollBegin error");
        set_tz_state(STATE_IDLE);
        send_error_notice(vdev, FINGERPRINT_ERROR_UNABLE_TO_PROCESS);
        return NULL;
    }
    memset(send_vendor_cmd, 0, QSEECOM_ALIGN(sizeof(*send_vendor_cmd)) + QSEECOM_ALIGN(sizeof(*resp)));
    send_vendor_cmd->cmd = vfmVendorDefinedOperation;
    send_vendor_cmd->vendor_cmd = vendorUnknownA;
    resp->data[0] = 0x4;
    ret = QSEECom_send_cmd(handle, send_vendor_cmd, QSEECOM_ALIGN(sizeof(*send_vendor_cmd)), resp, QSEECOM_ALIGN(sizeof(*resp)));
    if (ret || resp->result) {
        ALOGE("send vendorUnknownA error");
        set_tz_state(STATE_IDLE);
        send_error_notice(vdev, FINGERPRINT_ERROR_UNABLE_TO_PROCESS);
        return NULL;
    }
    while (get_tz_state() == STATE_ENROLL) {
        ret = vcs_start_capture(vdev, 0);
        if (ret == -1)
            goto out;
        resp = (trust_zone_normal_result_t *)(handle->ion_sbuffer + QSEECOM_ALIGN(sizeof(int)));
        memset(send_cmd, 0, QSEECOM_ALIGN(sizeof(int)) + QSEECOM_ALIGN(sizeof(*resp)));
        send_cmd->cmd = vfmEnrollAddImage;
        resp->data[0] = 0x8;
        ret = QSEECom_send_cmd(handle, send_cmd, QSEECOM_ALIGN(sizeof(int)), resp, QSEECOM_ALIGN(sizeof(*resp)));
        if (ret) {
            ALOGE("%s:send vfmEnrollAddImage error", __FUNCTION__);
            set_tz_state(STATE_IDLE);
            send_error_notice(vdev, FINGERPRINT_ERROR_UNABLE_TO_PROCESS);
            return NULL;
        }
        if (resp->result != 0) {
            send_acquired_notice(vdev, resp->result);
            continue;
        }
        count--;
        if (resp->data[2] == 0x1)
            count = 0;
        if (resp->data[2] != 0x1 && count == 0)
            count = 1;
        send_enroll_notice(vdev, idx, count);
        if (count == 0)
            break;
    }
    resp = (trust_zone_normal_result_t *)(handle->ion_sbuffer + QSEECOM_ALIGN(sizeof(trust_zone_base_cmd_t)));
    memset(send_cmd, 0, QSEECOM_ALIGN(sizeof(*send_cmd)) + QSEECOM_ALIGN(sizeof(*resp)));
    send_cmd->cmd = vfmEnrollFinish;
    send_cmd->len = AUTH_SESSION_TOKEN_LENGTH;
    resp->data[0] = 0x2845;
    memcpy(&send_cmd->data, &tz.auth_session_token, AUTH_SESSION_TOKEN_LENGTH);
    ret = QSEECom_send_cmd(handle, send_cmd, QSEECOM_ALIGN(sizeof(*send_cmd)), resp, QSEECOM_ALIGN(sizeof(*resp)));
    if (ret || resp->result) {
        ALOGE("Send vfmEnrollFinish error");
    }
    memcpy(&tz.finger[idx].data, &resp->data[2], FINGER_DATA_MAX_LENGTH);
    for (i = 0; i < 2; i++) {
        memset(send_cmd, 0, QSEECOM_ALIGN(sizeof(*send_cmd)) + QSEECOM_ALIGN(sizeof(*resp)));
        send_cmd->cmd = vfmPayloadBind;
        send_cmd->len = 0x1;
        send_cmd->zero = 0x7;
        sprintf(&send_cmd->data[4], "User_0");
        if (i == 1) {
            resp->data[0] = 0x50;
        }
        ret = QSEECom_send_cmd(handle, send_cmd, QSEECOM_ALIGN(sizeof(*send_cmd)), resp, QSEECOM_ALIGN(sizeof(*resp)));
    }
    if (ret || resp->result) {
        ALOGE("Send vfmPayloadBind error");
    }
    memcpy(&tz.finger[idx].payload, &resp->data[2], PAYLOAD_MAX_LENGTH);
    tz.finger[idx].exist = true;
    db_write_to_db(vdev, false, idx);

    // We may not need to send vfmEnrollmentPasswordSet.
    for (i = 0; i < 2; i++) {
        trust_zone_3x_cmd_t *send_3x_cmd = NULL;
        send_3x_cmd = (trust_zone_3x_cmd_t *)handle->ion_sbuffer;
        resp = (trust_zone_normal_result_t *)(handle->ion_sbuffer + QSEECOM_ALIGN(sizeof(trust_zone_3x_cmd_t)));
        memset(send_3x_cmd, 0, QSEECOM_ALIGN(sizeof(*send_3x_cmd)) + QSEECOM_ALIGN(sizeof(*resp)));
        send_3x_cmd->cmd = vfmEnrollmentPasswordSet;
        send_3x_cmd->len = AUTH_SESSION_TOKEN_LENGTH;
        memcpy(&send_3x_cmd->data, &tz.auth_session_token, AUTH_SESSION_TOKEN_LENGTH);
        if (i == 0) {
            resp->data[0] = 0x90;
        } else {
            resp->data[0] = 0x80;
        }
        ret = QSEECom_send_cmd(handle, send_3x_cmd, QSEECOM_ALIGN(sizeof(*send_3x_cmd)), resp, QSEECOM_ALIGN(sizeof(*resp)));
    }
    if (ret || resp->result) {
        ALOGE("Send vfmEnrollmentPasswordSet error");
    }
out:
    set_tz_state(STATE_IDLE);
    sensor_uninit();
    vcs_uninit();
    return NULL;
}

void* vcs_timeout(void* vdev) {
    struct timeval now;
    struct timespec outtime;
    int ret = 0;

    pthread_mutex_lock(&tz.timeout.lock);
    gettimeofday(&now, NULL);
    outtime.tv_sec = now.tv_sec + tz.timeout.timeout;
    outtime.tv_nsec = now.tv_usec * 1000;
    ret = pthread_cond_timedwait(&tz.timeout.cond, &tz.timeout.lock, &outtime);
    pthread_mutex_unlock(&tz.timeout.lock);

    if (ret == ETIMEDOUT) {
        ALOGI("Enroll timeout! Exit!");
        int flag = 0;
        if (get_tz_state() != STATE_IDLE && get_tz_state() != STATE_CANCEL) {
            set_tz_state(STATE_CANCEL);
            ioctl(sensor.fd, VFSSPI_IOCTL_SET_DRDY_INT, &flag);
            while (1) {
                usleep(100000);
                if (get_tz_state() == STATE_IDLE)
                    break;
            }
        }
        send_error_notice(vdev, FINGERPRINT_ERROR_TIMEOUT);
    }
    return NULL;
}

int vcs_start_authenticate(void *vdev) {
    int times = 0;
    for (times = 0; times < 5; times++)
        if (get_tz_state() != STATE_IDLE) {
            ALOGE("%s:Sensor is busy!", __FUNCTION__);
            if (times < 4) {
                usleep(100000);
                continue;
            }
            return -1;
        }
    set_tz_state(STATE_SCAN);
    int ret = 0;

    sensor_init();

    ret = vcs_init();
    if (ret) return ret;
    ret = pthread_create(&tz.auth_thread, NULL, vcs_authenticate, vdev);
    if (ret) {
        ALOGE("Can't create authenticate thread!!");
    }
    return ret;
}

int vcs_start_enroll(void *vdev, uint32_t timeout) {
    if (get_tz_state() != STATE_IDLE) {
        ALOGE("%s:Sensor is busy!", __FUNCTION__);
        return -1;
    }
    set_tz_state(STATE_ENROLL);
    int ret = 0;

    sensor_init();

    ret = vcs_init();
    if (ret) {
        return ret;
    }
    ret = pthread_create(&tz.enroll_thread, NULL, vcs_enroll, vdev);
    if (ret) {
        ALOGE("Can't create enroll thread!!");
        return ret;
    }
    if (timeout) {
        tz.timeout.timeout = timeout;
        ret = pthread_create(&tz.timeout.timeout_thread, NULL, vcs_timeout, vdev);
        if (ret) {
            ALOGE("Can't create timeout thread!!");
        }
    }
    return ret;
}

int vcs_get_enrolled_finger_num() {
    int num = 0;
    int idx = 1;
    for (idx = 1; idx <= MAX_NUM_FINGERS; idx++)
        if (tz.finger[idx].exist)
            num++;
    ALOGV("%s: num=%d", __FUNCTION__, num);
    return num;
}

/*
 * cmd: vendorGetAuthToken
 */
int vcs_update_auth_token() {
    struct QSEECom_handle *handle = (struct QSEECom_handle *)(tz.qhandle);
    trust_zone_vendor_cmd_t *send_vendor_cmd = NULL;
    trust_zone_normal_result_t *resp = NULL;
    int ret = 0;
    send_vendor_cmd = (trust_zone_vendor_cmd_t *)handle->ion_sbuffer;
    resp = (trust_zone_normal_result_t *)(handle->ion_sbuffer + QSEECOM_ALIGN(sizeof(trust_zone_vendor_cmd_t)));
    int i = 0;

    for (i = 0;i < 2; i++) {
        memset(send_vendor_cmd, 0, QSEECOM_ALIGN(sizeof(*send_vendor_cmd)) + QSEECOM_ALIGN(sizeof(*resp)));
        send_vendor_cmd->cmd = vfmVendorDefinedOperation;
        send_vendor_cmd->vendor_cmd = vendorGetAuthToken;
        if (i == 1) {
            resp->data[0] = 0x80;
        }
        ret = QSEECom_send_cmd(handle, send_vendor_cmd, QSEECOM_ALIGN(sizeof(*send_vendor_cmd)), resp, QSEECOM_ALIGN(sizeof(*resp)));
    }
    if (resp->result) {
        ALOGE("send vendorGetAuthToken failed, qsapp result=%d", resp->result);
        return resp->result;
    }
    memcpy(&tz.auth_token, &resp->data[2], AUTH_TOKEN_LENGTH);
    ALOGV("Sended vendorGetAuthToken");
    return ret;
}

/*
 * cmd: vfmAuthSessionBegin
 */

int vcs_start_auth_session() {
    struct QSEECom_handle *handle = (struct QSEECom_handle *)(tz.qhandle);
    trust_zone_base_cmd_t *send_cmd = NULL;
    trust_zone_vendor_cmd_t *send_vendor_cmd = NULL;
    trust_zone_normal_result_t *resp = NULL;
    int ret = 0;
    send_cmd = (trust_zone_base_cmd_t *)handle->ion_sbuffer;
    send_vendor_cmd = (trust_zone_vendor_cmd_t *)handle->ion_sbuffer;
    int i = 0;

    resp = (trust_zone_normal_result_t *)(handle->ion_sbuffer + QSEECOM_ALIGN(sizeof(int)));
    memset(send_cmd, 0, QSEECOM_ALIGN(sizeof(int)) + QSEECOM_ALIGN(sizeof(*resp)));
    send_cmd->cmd = vfmAuthSessionBegin;
    resp->data[0] = AUTH_SESSION_TOKEN_LENGTH;
    ret = QSEECom_send_cmd(handle, send_cmd, QSEECOM_ALIGN(sizeof(int)), resp, QSEECOM_ALIGN(sizeof(*resp)));
    if (ret || resp->result) {
        ALOGE("send vfmAuthSessionBegin failed, qsapp result=%d", resp->result);
        return ret;
    }
    memcpy(&tz.auth_session_token, &resp->data[2], AUTH_SESSION_TOKEN_LENGTH);
    tz.auth_session_opend = true;
    ALOGV("Sended vfmAuthSessionBegin");
    return ret;
}

/*
 * cmd: vfmAuthSessionEnd
 */

int vcs_stop_auth_session() {
    struct QSEECom_handle *handle = (struct QSEECom_handle *)(tz.qhandle);
    trust_zone_base_cmd_t *send_cmd = NULL;
    trust_zone_normal_result_t *resp = NULL;
    int ret = 0;
    send_cmd = (trust_zone_base_cmd_t *)handle->ion_sbuffer;
    resp = (trust_zone_normal_result_t *)(handle->ion_sbuffer + QSEECOM_ALIGN(sizeof(int)));

    memset(send_cmd, 0, QSEECOM_ALIGN(sizeof(int)) + QSEECOM_ALIGN(sizeof(int)));
    send_cmd->cmd = vfmAuthSessionEnd;
    ret = QSEECom_send_cmd(handle, send_cmd, QSEECOM_ALIGN(sizeof(int)), resp, QSEECOM_ALIGN(sizeof(int)));
    if (ret || resp->result) {
        ALOGE("send vfmAuthSessionEnd failed, qsapp result=%d", resp->result);
    }
    memset(tz.auth_session_token, 0, AUTH_SESSION_TOKEN_LENGTH);
    tz.auth_session_opend = false;
    ALOGV("Sended vfmAuthSessionEnd");
    return ret;
}

/*
 * cmd: 1.vfmInitialize
 *      2.vendorEnterAuthSession
 *      3.vfmDeviceInitialize
 *      4.vfmDeviceCalibrate
 */

int vcs_resume() {
    struct QSEECom_handle *handle = (struct QSEECom_handle *)(tz.qhandle);
    trust_zone_base_cmd_t *send_cmd = NULL;
    trust_zone_normal_result_t *resp = NULL;
    int ret = 0;
    send_cmd = (trust_zone_base_cmd_t *)handle->ion_sbuffer;
    resp = (trust_zone_normal_result_t *)(handle->ion_sbuffer + QSEECOM_ALIGN(sizeof(trust_zone_base_cmd_t)));

    memset(send_cmd, 0, QSEECOM_ALIGN(sizeof(*send_cmd)) + QSEECOM_ALIGN(sizeof(int)));
    send_cmd->cmd = vfmInitialize;
    send_cmd->len = 4;
    send_cmd->data[0] = 2;
    ret = QSEECom_send_cmd(handle, send_cmd, QSEECOM_ALIGN(sizeof(*send_cmd)), resp, QSEECOM_ALIGN(sizeof(int)));
    if (ret || resp->result) {
        ALOGE("send vfmInitialize failed, qsapp result=%d", resp->result);
        return ret;
    }
    ALOGV("Sended vfmInitialize");

    if (tz.auth_session_opend) {
        trust_zone_vendor_cmd_t *send_vendor_cmd = NULL;
        send_vendor_cmd = (trust_zone_vendor_cmd_t *)handle->ion_sbuffer;
        memset(send_vendor_cmd, 0, QSEECOM_ALIGN(sizeof(*send_vendor_cmd)) + QSEECOM_ALIGN(sizeof(*resp)));
        send_vendor_cmd->cmd = vfmVendorDefinedOperation;
        send_vendor_cmd->vendor_cmd = vendorEnterAuthSession;
        send_vendor_cmd->len = 0x70;
        memcpy(&send_vendor_cmd->data, &tz.auth_token, 0x70);
        ret = QSEECom_send_cmd(handle, send_vendor_cmd, QSEECOM_ALIGN(sizeof(*send_vendor_cmd)), resp, QSEECOM_ALIGN(sizeof(*resp)));
        if (ret || resp->result) {
            ALOGE("send EnterAuthSession failed, qsapp result=%d", resp->result);
            return resp->result;
        }
    }
    ALOGV("Sended EnterAuthSession");

    memset(send_cmd, 0, QSEECOM_ALIGN(sizeof(*send_cmd)) + QSEECOM_ALIGN(sizeof(int)));
    send_cmd->cmd = vfmDeviceInitialize;
    ret = QSEECom_send_cmd(handle, send_cmd, QSEECOM_ALIGN(sizeof(*send_cmd)), resp, QSEECOM_ALIGN(sizeof(int)));
    if (ret || resp->result) {
        ALOGE("send vfmDeviceInitialize failed, qsapp result=%d", resp->result);
        return ret;
    }
    ALOGV("Sended vfmDeviceInitialize");

    memset(send_cmd, 0, QSEECOM_ALIGN(sizeof(*send_cmd)) + QSEECOM_ALIGN(sizeof(*resp)));
    send_cmd->cmd = vfmDeviceCalibrate;
    send_cmd->data[0] = 0xc0;
    send_cmd->data[1] = 0x12;
    resp->data[0] = CALIBRATE_DATA_MAX_LENGTH;
    if (tz.calibrate_len) {
        send_cmd->len = tz.calibrate_len;
        memcpy(&send_cmd->data[4], &tz.calibrate_data, tz.calibrate_len);
    } else {
        send_cmd->len = 0x10;
    }
    ret = QSEECom_send_cmd(handle, send_cmd, QSEECOM_ALIGN(sizeof(*send_cmd)), resp, QSEECOM_ALIGN(sizeof(*resp)));
    if (ret || resp->result) {
        ALOGE("send vfmDeviceCalibrate failed, qsapp result=%d", resp->result);
        return ret;
    }
    if (tz.calibrate_len == 0) {
        tz.calibrate_len = resp->data[3] + 0xf;
        memcpy(&tz.calibrate_data, &resp->data[2], tz.calibrate_len);
    }
    ALOGV("Sended vfmDeviceCalibrate");
    return ret;
}

/*
 * cmd: vfmUninitialize
 * set bandwidth to low and shutdown app
 */

int vcs_uninit() {
    struct QSEECom_handle *handle = (struct QSEECom_handle *)(tz.qhandle);
    trust_zone_base_cmd_t *send_cmd = NULL;
    trust_zone_normal_result_t *resp = NULL;
    send_cmd = (trust_zone_base_cmd_t *)handle->ion_sbuffer;
    resp = (trust_zone_normal_result_t *)(handle->ion_sbuffer + QSEECOM_ALIGN(sizeof(trust_zone_base_cmd_t)));

    if (tz.auth_session_opend) {
        vcs_update_auth_token();
    }

    memset(send_cmd, 0, QSEECOM_ALIGN(sizeof(int)) + QSEECOM_ALIGN(sizeof(int)));
    send_cmd->cmd = vfmUninitialize;
    QSEECom_send_cmd(handle, send_cmd, QSEECOM_ALIGN(sizeof(int)), resp, QSEECOM_ALIGN(sizeof(int)));
    QSEECom_set_bandwidth(handle, false);
    QSEECom_shutdown_app((struct QSEECom_handle **)&tz.qhandle);
    tz.init = false;
    set_tz_state(STATE_IDLE);
    ALOGV("Closed securefp qsapp");
    return 0;
}

/*
 * start app and set bandwidth to high
 * Call vcs_resume
 */

int vcs_init() {
    int ret = 0;
    if (tz.init) {
        ALOGI("securefp qsapp is already running!");
        return ret;
    }
    ret = QSEECom_start_app((struct QSEECom_handle **)&tz.qhandle,
                        "/firmware/image", "securefp", SB_LENGTH);
    if (ret) {
        ALOGE("Loading securefp app failed");
        return -1;
    }
    struct QSEECom_handle *handle = (struct QSEECom_handle *)(tz.qhandle);
    ret = QSEECom_set_bandwidth(handle, true);
    if (ret) {
        ALOGE("Set bandwidth failed");
        return -1;
    }
    tz.init = true;
    ALOGV("securefp qsapp init success!");
    ret = vcs_resume();
    return ret;
}
