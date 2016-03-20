/*
 * Copyright (C) 2014 The Android Open Source Project
 * Copyright (C) 2016 The Mokee Project
 * Copyright (C) 2016 The CyanogenMod Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "FingerprintHal_msm8974"
#define LOG_NDEBUG 1

#include <errno.h>
#include <endian.h>
#include <inttypes.h>
#include <malloc.h>
#include <string.h>
#include <cutils/log.h>
#include <cutils/sockets.h>
#include <hardware/hardware.h>
#include <hardware/fingerprint.h>
#include <unistd.h>

#include "fp_klte.h"

#define MAX_COMM_CHARS 128
#define MAX_NUM_FINGERS 5
#define SOCKET_NAME_SEND "validityservice"
#define SOCKET_NAME_RECEIVE "validityservice_callback"

/******************************************************************************/
static void checkinit(vcs_fingerprint_device_t* vdev) { //wait for hal connect validity service
    while(!vdev->init)
        sleep(1);
}

static int sendcommand(vcs_fingerprint_device_t* vdev, uint8_t* command, int num) {
    int ret = -1;
    char answer[255];
    if (fd_write(vdev->send_fd, command, num) != num) {
        ALOGE("cannot send command to service");
        //close(vdev->send_fd);
        return ret;
    }
    if (fd_read(vdev->send_fd, answer, 255))
        ret = atoi(answer);
    return ret;
}

static int getfingermask(vcs_fingerprint_device_t* vdev) {
    uint8_t command_getlist[2] = {CALL_GET_ENROLLED_FINGER_LIST, (uint8_t)vdev->active_gid};
    return sendcommand(vdev, command_getlist, 2);
}

static int initService(vcs_fingerprint_device_t* vdev) {
    ALOGV("----------------> %s ----------------->", __FUNCTION__);
    int ret = -EINVAL;
    while (vdev->send_fd <= 0) {
        vdev->send_fd = socket_local_client(SOCKET_NAME_SEND, ANDROID_SOCKET_NAMESPACE_ABSTRACT,SOCK_STREAM);
        if (vdev->send_fd < 0) {
            ALOGW("cannot open validity service!");
            sleep(1);
        }
    }
    uint8_t command[1] = {CALL_INITSERVICE};

    ret = sendcommand(vdev, command, 1);
    vdev->authenticator_id = getfingermask(vdev);
    vdev->init = true;
    return ret;
}

static void send_error_notice(vcs_fingerprint_device_t* vdev, fingerprint_error_t error_info) {
    ALOGV("----------------> %s ----------------->", __FUNCTION__);

    fingerprint_msg_t msg = {0};
    msg.type = FINGERPRINT_ERROR;
    msg.data.error = error_info;
    ALOGI("error_info=%d", (int)error_info);

    pthread_mutex_lock(&vdev->lock);
    vdev->device.notify(&msg);
    pthread_mutex_unlock(&vdev->lock);

    return;
}

static void send_acquired_notice(vcs_fingerprint_device_t* vdev, fingerprint_acquired_info_t acquired_info) {
    ALOGV("----------------> %s ----------------->", __FUNCTION__);

    fingerprint_msg_t acqu_msg = {0};
    acqu_msg.type = FINGERPRINT_ACQUIRED;
    acqu_msg.data.acquired.acquired_info = acquired_info;
    ALOGI("acqu_info=%d", (int)acquired_info);

    pthread_mutex_lock(&vdev->lock);
    vdev->device.notify(&acqu_msg);
    pthread_mutex_unlock(&vdev->lock);

    return;
}

static void send_enroll_notice(vcs_fingerprint_device_t* vdev, int fid, int remaining) {
    ALOGV("----------------> %s -----------------> fid %d", __FUNCTION__, fid);

    if (fid == 0) {
        ALOGD("Fingerprint ID is zero (invalid)");
        return;
    }
    if (vdev->secure_user_id == 0) {
        ALOGD("Secure user ID is zero (invalid)");
        return;
    }

    pthread_mutex_lock(&vdev->lock);

    vdev->listener.state = STATE_SCAN;

    fingerprint_msg_t msg = {0};
    msg.type = FINGERPRINT_TEMPLATE_ENROLLING;
    msg.data.enroll.finger.fid = fid;
    msg.data.enroll.samples_remaining = remaining;
    vdev->device.notify(&msg);

    pthread_mutex_unlock(&vdev->lock);

    return;
}

static void send_authenticated_notice(vcs_fingerprint_device_t* vdev, int fid) {
    ALOGV("----------------> %s ----------------->", __FUNCTION__);

    send_acquired_notice(vdev, FINGERPRINT_ACQUIRED_GOOD);

    fingerprint_msg_t auth_msg = {0};
    auth_msg.type = FINGERPRINT_AUTHENTICATED;
    auth_msg.data.authenticated.finger.fid = fid;
    auth_msg.data.authenticated.finger.gid = 0;  // unused
    auth_msg.data.authenticated.hat.version = HW_AUTH_TOKEN_VERSION;
    auth_msg.data.authenticated.hat.authenticator_type =
            htobe32(HW_AUTH_FINGERPRINT);
    auth_msg.data.authenticated.hat.challenge = vdev->op_id;
    auth_msg.data.authenticated.hat.authenticator_id = vdev->authenticator_id;
    auth_msg.data.authenticated.hat.user_id = vdev->secure_user_id;
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    auth_msg.data.authenticated.hat.timestamp =
            htobe64((uint64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000);

    pthread_mutex_lock(&vdev->lock);
    vdev->device.notify(&auth_msg);
    pthread_mutex_unlock(&vdev->lock);

    return;
}

static void send_remove_notice(vcs_fingerprint_device_t* vdev, int fid) {
    ALOGV("----------------> %s ----------------->fid=%d", __FUNCTION__, fid);

    fingerprint_msg_t msg = {0};
    msg.type = FINGERPRINT_TEMPLATE_REMOVED;
    msg.data.removed.finger.fid = fid;

    pthread_mutex_lock(&vdev->lock);
    vdev->device.notify(&msg);
    pthread_mutex_unlock(&vdev->lock);

    return;
}

/******************************************************************************/

static uint64_t get_64bit_rand() {
    ALOGV("----------------> %s ----------------->", __FUNCTION__);
    uint64_t r = (((uint64_t)rand()) << 32) | ((uint64_t)rand());
    return r != 0 ? r : 1;
}

static uint64_t fingerprint_get_auth_id(struct fingerprint_device* device) {
    vcs_fingerprint_device_t* vdev = (vcs_fingerprint_device_t*)device;
    ALOGV("----------------> %s ----------------->", __FUNCTION__);
    uint64_t authenticator_id = 0;
    pthread_mutex_lock(&vdev->lock);
    vdev->authenticator_id = getfingermask(vdev);
    authenticator_id = vdev->authenticator_id;
    pthread_mutex_unlock(&vdev->lock);

    return authenticator_id;
}

static int fingerprint_set_active_group(struct fingerprint_device *device, uint32_t gid,
        const char __unused *path) {
    vcs_fingerprint_device_t* vdev = (vcs_fingerprint_device_t*)device;
    pthread_mutex_lock(&vdev->lock);
    vdev->active_gid = gid;
    pthread_mutex_unlock(&vdev->lock);

    return 0;
}

static int fingerprint_authenticate(struct fingerprint_device *device,
    uint64_t operation_id, __unused uint32_t gid)
{
    ALOGV("----------------> %s ----------------->", __FUNCTION__);
    vcs_fingerprint_device_t* vdev = (vcs_fingerprint_device_t*)device;
    int ret = 0;
    ALOGI("auth: op_id=%llu ",operation_id);

    checkinit(vdev);

    pthread_mutex_lock(&vdev->lock);

    vdev->op_id = operation_id;
    vdev->listener.state = STATE_SCAN;
    uint8_t command[2] = {CALL_IDENTIFY, (uint8_t)vdev->active_gid};
    ret = sendcommand(vdev, command, 2);

    pthread_mutex_unlock(&vdev->lock);

    // Always return successful
    return 0;
}

static int fingerprint_enroll(struct fingerprint_device *device,
        const hw_auth_token_t *hat,
        uint32_t __unused gid,
        uint32_t __unused timeout_sec) {
    ALOGD("fingerprint_enroll");
    vcs_fingerprint_device_t* vdev = (vcs_fingerprint_device_t*)device;
    int ret = -EINVAL;
    int fingermask = 0;
    int idx = 1;
    uint8_t command[3] = {CALL_ENROLL, (uint8_t)vdev->active_gid, 0};

    checkinit(vdev);

    if (!hat) {
        ALOGW("%s: null auth token", __func__);
        return -EPROTONOSUPPORT;
    }
    if (hat->challenge == vdev->challenge) {
        vdev->secure_user_id = hat->user_id;
    } else {
        ALOGW("%s: invalid auth token", __func__);
    }

    if (hat->version != HW_AUTH_TOKEN_VERSION) {
        return -EPROTONOSUPPORT;
    }
    if (hat->challenge != vdev->challenge && !(hat->authenticator_type & HW_AUTH_FINGERPRINT)) {
        return -EPERM;
    }

    vdev->user_id = hat->user_id;

    pthread_mutex_lock(&vdev->lock);
    vdev->listener.state = STATE_ENROLL;

    fingermask = getfingermask(vdev);
    ALOGI("fingerprint_enroll: fingermask=%d", fingermask);
    for (idx = 1; idx <= MAX_NUM_FINGERS; idx++)
        if (!((fingermask >> idx) & 1))
            break;

    command[2] = (uint8_t)idx;
    ret = sendcommand(vdev, command, 3);

    pthread_mutex_unlock(&vdev->lock);
    ALOGI("enroll ret=%d",ret);

    // workaround
    if (ret == 1) {
        ret = 0;
    }

    vdev->authenticator_id = getfingermask(vdev);

    return ret;
}

static uint64_t fingerprint_pre_enroll(struct fingerprint_device *device) {
    ALOGV("----------------> %s ----------------->", __FUNCTION__);
    uint64_t challenge = 0;
    vcs_fingerprint_device_t* vdev = (vcs_fingerprint_device_t*)device;

    challenge = get_64bit_rand();

    pthread_mutex_lock(&vdev->lock);
    vdev->challenge = challenge;
    pthread_mutex_unlock(&vdev->lock);

    return challenge;
}

static int fingerprint_post_enroll(struct fingerprint_device* device) {
    ALOGV("----------------> %s ----------------->", __FUNCTION__);
    vcs_fingerprint_device_t* vdev = (vcs_fingerprint_device_t*)device;

    pthread_mutex_lock(&vdev->lock);
    vdev->challenge = 0;
    pthread_mutex_unlock(&vdev->lock);

    return 0;
}

static int fingerprint_cancel(struct fingerprint_device *device) {
    ALOGV("----------------> %s ----------------->", __FUNCTION__);
    vcs_fingerprint_device_t* vdev = (vcs_fingerprint_device_t*)device;
    int ret = 0;

    checkinit(vdev);

    pthread_mutex_lock(&vdev->lock);
    vdev->listener.state = STATE_IDLE;

    uint8_t command[1] = {CALL_CANCEL};
    ret = sendcommand(vdev, command, 1);
    pthread_mutex_unlock(&vdev->lock);

    return ret;
}

static int fingerprint_enumerate(struct fingerprint_device *device,
        fingerprint_finger_id_t *results, uint32_t *max_size) {
    ALOGV("----------------> %s ----------------->", __FUNCTION__);
    if (device == NULL || results == NULL || max_size == NULL) {
        ALOGE("Cannot enumerate saved fingerprints with uninitialized params");
        return -1;
    }

    vcs_fingerprint_device_t* vdev = (vcs_fingerprint_device_t*)device;
    int num = 0;
    pthread_mutex_lock(&vdev->lock);

    int fingermask = getfingermask(vdev);

    ALOGI("fingerprint_enumerate: fingermask=%d", fingermask);
    int idx = 0;
    for (idx = 0; idx < MAX_NUM_FINGERS; idx++)
        if ((fingermask >> (idx + 1)) & 1)
            num++;
    pthread_mutex_unlock(&vdev->lock);

    return num;
}

static int fingerprint_remove(struct fingerprint_device *device,
        uint32_t __unused gid, uint32_t fid) {
    int idx = 0, ret = 0;
    ALOGV("----------------> %s -----------------> fid %d", __FUNCTION__, fid);
    if (device == NULL) {
        ALOGE("Can't remove fingerprint (gid=%d, fid=%d); "
              "device not initialized properly",
              gid, fid);
        return -1;
    }

    vcs_fingerprint_device_t* vdev = (vcs_fingerprint_device_t*)device;

    uint8_t command[3] = {CALL_REMOVE, (uint8_t)vdev->active_gid, 0};

    checkinit(vdev);

    if (fid == 0) {
        // Delete all fingerprints
        command[2] = 21;
        int fingermask = getfingermask(vdev);
        pthread_mutex_lock(&vdev->lock);
        ret = sendcommand(vdev, command, 3);
        pthread_mutex_unlock(&vdev->lock);
        if (ret == 0){
            pthread_mutex_lock(&vdev->lock);
            pthread_mutex_unlock(&vdev->lock);
            int idx = 0;
            for (idx = 0; idx < MAX_NUM_FINGERS; idx++)
                if ((fingermask >> (idx + 1)) & 1) {
                    send_remove_notice(vdev, idx + 1);
                }
        }  // end if
        vdev->listener.state = STATE_IDLE;
    } else {
        // Delete one fingerprint
        pthread_mutex_lock(&vdev->lock);

        command[2] = (uint8_t)fid;
        ret = sendcommand(vdev, command, 3);
        vdev->listener.state = STATE_IDLE;
        pthread_mutex_unlock(&vdev->lock);

        // Always send remove notice
        send_remove_notice(vdev, fid);
    }
    pthread_mutex_lock(&vdev->lock);

    int fingermask = getfingermask(vdev);
    if (fingermask == 0) {  // All finger are removed
        command[2] = 21;
        sendcommand(vdev, command, 3);
    }

    pthread_mutex_unlock(&vdev->lock);

    if (ret) {
        send_error_notice(vdev, FINGERPRINT_ERROR_UNABLE_TO_REMOVE);
    }

    return ret;
}

static int set_notify_callback(struct fingerprint_device *device,
                               fingerprint_notify_t notify) {
    ALOGV("----------------> %s ----------------->", __FUNCTION__);
    if (device == NULL || notify == NULL) {
        ALOGE("Failed to set notify callback @ %p for fingerprint device %p",
              device, notify);
        return -1;
    }

    vcs_fingerprint_device_t* vdev = (vcs_fingerprint_device_t*)device;
    pthread_mutex_lock(&vdev->lock);
    vdev->listener.state = STATE_IDLE;
    device->notify = notify;
    pthread_mutex_unlock(&vdev->lock);
    ALOGD("fingerprint callback notification set");

    return 0;
}

static worker_state_t getListenerState(vcs_fingerprint_device_t* dev) {
    ALOGV("----------------> %s ----------------->", __FUNCTION__);
    worker_state_t state = STATE_IDLE;

    pthread_mutex_lock(&dev->lock);
    state = dev->listener.state;
    pthread_mutex_unlock(&dev->lock);

    return state;
}

static void* listenerSocket(void* data) {
    ALOGI("----------------> %s ----------------->", __FUNCTION__);
    vcs_fingerprint_device_t* vdev = (vcs_fingerprint_device_t*)data;

    while (vdev->receive_fd <= 0) {
        vdev->receive_fd = socket_local_client(SOCKET_NAME_RECEIVE, ANDROID_SOCKET_NAMESPACE_ABSTRACT,SOCK_STREAM);
        if (vdev->receive_fd < 0) {
            ALOGW("listener cannot open fingerprint listener service");
            sleep(1);
        }
    }
    initService(vdev);
    pthread_mutex_lock(&vdev->lock);
    vdev->listener.state = STATE_IDLE;
    pthread_mutex_unlock(&vdev->lock);

    while (1) {
        int size = 0;
        char buffer[MAX_COMM_CHARS] = {0};
        if (getListenerState(vdev) == STATE_EXIT) {
            ALOGD("Received request to exit listener thread");
            goto done;
        }

        if ((size = fd_read(vdev->receive_fd, buffer,
                                       sizeof(buffer) - 1)) > 0) {
            buffer[size] = '\0';
            int type, info, info_ex;
            sscanf(buffer, "%d:%d:%d", &type, &info, &info_ex);
            switch (type) {
                case 1: //error
                    send_error_notice(vdev, info);
                    break;
                case 2: //enroll
                    send_enroll_notice(vdev, info, info_ex);
                    break;
                case 3: //removed
                    send_remove_notice(vdev, info);
                    break;
                case 4: //acquired
                    send_acquired_notice(vdev, info);
                    break;
                case 5: //authenticated
                    send_authenticated_notice(vdev, info);
                    break;
                default:
                    ALOGE("unknow type:%d", type);
            }
        } else {
            ALOGE("fingerprint listener receive failure");
            break;
        }
    }

done:
    ALOGD("Listener exit !!");
done_quiet:
    close(vdev->receive_fd);
    return NULL;
}

static int fingerprint_close(hw_device_t* device) {
    ALOGV("----------------> %s ----------------->", __FUNCTION__);
    if (device == NULL) {
        ALOGE("fingerprint hw device is NULL");
        return -1;
    }

    vcs_fingerprint_device_t* vdev = (vcs_fingerprint_device_t*)device;

    checkinit(vdev);

    pthread_mutex_lock(&vdev->lock);
    // Ask listener thread to exit
    vdev->listener.state = STATE_EXIT;
    uint8_t command[1] = {CALL_CLEANUP};
    sendcommand(vdev, command, 1);
    pthread_mutex_unlock(&vdev->lock);

    pthread_join(vdev->listener.thread, NULL);
    pthread_mutex_destroy(&vdev->lock);
    close(vdev->send_fd);
    free(vdev);

    return 0;
}

static int fingerprint_open(const hw_module_t* module, const char __unused *id,
                            hw_device_t** device)
{

    ALOGV("----------------> %s ----------------->", __FUNCTION__);
    if (device == NULL) {
        ALOGE("NULL device on open");
        return -EINVAL;
    }
    vcs_fingerprint_device_t *vdev = (vcs_fingerprint_device_t*)calloc(
            1, sizeof(vcs_fingerprint_device_t));

    if (vdev == NULL) {
        ALOGE("Insufficient memory for fingerprint device");
        return -ENOMEM;
    }

    vdev->device.common.tag = HARDWARE_DEVICE_TAG;
    vdev->device.common.version = HARDWARE_MODULE_API_VERSION(2, 0);
    vdev->device.common.module = (struct hw_module_t*)module;
    vdev->device.common.close = fingerprint_close;

    vdev->device.pre_enroll = fingerprint_pre_enroll;
    vdev->device.enroll = fingerprint_enroll;
    vdev->device.post_enroll = fingerprint_post_enroll;
    vdev->device.get_authenticator_id = fingerprint_get_auth_id;
    vdev->device.set_active_group = fingerprint_set_active_group;
    vdev->device.authenticate = fingerprint_authenticate;
    vdev->device.cancel = fingerprint_cancel;
    vdev->device.enumerate = fingerprint_enumerate;
    vdev->device.remove = fingerprint_remove;
    vdev->device.set_notify = set_notify_callback;
    vdev->device.notify = NULL;

    vdev->active_gid = 0;
    vdev->init = false;

    pthread_mutex_init(&vdev->lock, NULL);
    if (pthread_create(&vdev->listener.thread, NULL, listenerSocket, vdev) !=
        0)
        return -1;

    *device = &vdev->device.common;

    return 0;
}

static struct hw_module_methods_t fingerprint_module_methods = {
    .open = fingerprint_open,
};

fingerprint_module_t HAL_MODULE_INFO_SYM = {
    .common = {
        .tag                = HARDWARE_MODULE_TAG,
        .module_api_version = FINGERPRINT_MODULE_API_VERSION_2_0,
        .hal_api_version    = HARDWARE_HAL_API_VERSION,
        .id                 = FINGERPRINT_HARDWARE_MODULE_ID,
        .name               = "KLTE Fingerprint HAL",
        .author             = "ljzyal(ljzyal@gmail.com)",
        .methods            = &fingerprint_module_methods,
    },
};
