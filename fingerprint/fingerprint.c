/*
 * Copyright (C) 2016 The Android Open Source Project
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
#define LOG_NDEBUG 0

#include <errno.h>
#include <endian.h>
#include <fcntl.h>
#include <inttypes.h>
#include <malloc.h>
#include <signal.h>
#include <sqlite3.h>
#include <string.h>
#include <unistd.h>

#include <cutils/log.h>
#include <sys/ioctl.h>

#include <hardware/hardware.h>
#include <hardware/fingerprint.h>

#include "fp_klte.h"
#include "fingerprint_tz.h"
#include "hash.h"

typedef struct vcs_fingerprint_device_t {
    fingerprint_device_t device;  // "inheritance"
    uint64_t op_id;
    uint64_t challenge;
    uint64_t user_id;
    uint64_t group_id;
    uint64_t secure_user_id;
    uint64_t authenticator_id;
    uint32_t active_gid;
    sqlite3 *db;
    pthread_mutex_t lock;
} vcs_fingerprint_device_t;

extern trust_zone_t tz;
vcs_sensor_t sensor;

/***************************************Sensor***************************************/
int sensor_uninit() {
    ALOGV("----------------> %s ----------------->", __FUNCTION__);
    int ret = 0;
    if (!sensor.init)
        return ret;
    ioctl(sensor.fd, VFSSPI_IOCTL_RESET_SPI_CONFIGURATION);
    ioctl(sensor.fd, VFSSPI_IOCTL_DISABLE_SPI_CLOCK);
    ioctl(sensor.fd, VFSSPI_IOCTL_DEVICE_SUSPEND);
    ioctl(sensor.fd, VFSSPI_IOCTL_POWER_OFF);
    sensor.init = false;
    return ret;
}

int sensor_init() {
    ALOGV("----------------> %s ----------------->", __FUNCTION__);
    int ret = 0;
    int clock = 0;
    if (sensor.init)
        return ret;
    ioctl(sensor.fd, VFSSPI_IOCTL_POWER_ON);
    clock = 65535;
    ioctl(sensor.fd, VFSSPI_IOCTL_SET_CLK, &clock);
    ioctl(sensor.fd, VFSSPI_IOCTL_DEVICE_RESET);
    ioctl(sensor.fd, VFSSPI_IOCTL_SET_SPI_CONFIGURATION);
    clock = 4800;
    ioctl(sensor.fd, VFSSPI_IOCTL_SET_CLK, &clock);
    sensor.init = true;
    return ret;
}

void sensor_process_signal(int signum) {
    ALOGV("----------------> %s ----------------->", __FUNCTION__);
    ALOGV("%s: signal %d received", __FUNCTION__, signum);
    int flag = 0;
    if (tz.timeout.timeout_thread) {
        pthread_mutex_lock(&tz.timeout.lock);
        pthread_cond_signal(&tz.timeout.cond);
        pthread_mutex_unlock(&tz.timeout.lock);
        //pthread_join(tz.timeout.timeout_thread, NULL);
        tz.timeout.timeout_thread = 0;
    }
    pthread_mutex_lock(&sensor.lock);
    if (get_tz_state() != STATE_IDLE && get_tz_state() != STATE_CANCEL)
        sensor.signal = true;
    ioctl(sensor.fd, VFSSPI_IOCTL_SET_DRDY_INT, &flag);
    pthread_cond_signal(&sensor.cond);
    pthread_mutex_unlock(&sensor.lock);
}

int sensor_register() {
    ALOGV("----------------> %s ----------------->", __FUNCTION__);
    struct vfsspi_iocRegSignal usrSignal;
    int ret = 0;
    struct sigaction sa_usr;
    sa_usr.sa_flags = 0;
    sa_usr.sa_handler = sensor_process_signal;
    sigaction(SIGUSR2, &sa_usr, NULL);
    usrSignal.userPID = getpid();
    usrSignal.signalID = SIGUSR2;
    ioctl(sensor.fd, VFSSPI_IOCTL_REGISTER_DRDY_SIGNAL, &usrSignal);
    return ret;
}

int sensor_capture_start() {
    ALOGV("----------------> %s ----------------->", __FUNCTION__);
    int ret = 0;
    int flag = 1;
    ret = ioctl(sensor.fd, VFSSPI_IOCTL_SET_DRDY_INT, &flag);
    return ret;
}

/***************************************Database***************************************/
int db_check_and_create_table(void* device) {
    ALOGV("----------------> %s ----------------->", __FUNCTION__);
    vcs_fingerprint_device_t* vdev = (vcs_fingerprint_device_t*)device;
    char* errmsg;
    int ret = 0;
    char cmd[MAX_DATABASE_CMD];
    sprintf(cmd, "create table gid_%d(id integer, data blob, payload blob)", vdev->active_gid);
    ret = sqlite3_exec(vdev->db, cmd, NULL, NULL, &errmsg);
    return ret;
}

int db_convert_old_db(vcs_fingerprint_device_t* vdev) {
    ALOGV("----------------> %s ----------------->", __FUNCTION__);
    int ret = 0;
    sqlite3 *db;
    sqlite3_stmt *stat;
    char cmd[MAX_DATABASE_CMD];
    int idx = 0;

    memset(tz.finger, 0, MAX_NUM_FINGERS * sizeof(finger_t));
    ret = sqlite3_open(SAMSUNG_FP_DB_PATH, &db);
    if (ret != SQLITE_OK) {
        ALOGE("Open samsung finger database failed!");
        return -1;
    }

    sprintf(cmd, "select * from enrollments");
    sqlite3_prepare(db, cmd, -1, &stat, 0);
    while(1) {
        int ret = sqlite3_step(stat);
        if (ret != SQLITE_ROW) {
            break;
        }
        int id = sqlite3_column_int(stat, 1);
        const void *data = sqlite3_column_blob(stat, 2);
        int len = sqlite3_column_bytes(stat, 2);
        memcpy(tz.finger[id].data, data, len);
        ALOGV("read fingerprint data from samsung fp database: id=%d", id);
        tz.finger[id].exist = true;
    }
    sqlite3_finalize(stat);

    memset(cmd, 0, MAX_DATABASE_CMD);
    sprintf(cmd, "select * from properties");
    sqlite3_prepare(db, cmd, -1, &stat, 0);
    sqlite3_step(stat);
    const void *payload = sqlite3_column_blob(stat, 2);
    int len = sqlite3_column_bytes(stat, 2);
    for (idx = 1; idx <= MAX_NUM_FINGERS; idx++) {
        if (tz.finger[idx].exist) {
            memcpy(tz.finger[idx].payload, payload, len);
			db_write_to_db(vdev, false, idx);
        }
    }
    sqlite3_finalize(stat);

    ret = sqlite3_close(db);
    if (ret != SQLITE_OK) {
        ALOGE("Close samsung finger database failed!");
        return -1;
    }

    ret = remove(SAMSUNG_FP_DB_PATH);
    if (ret) {
        ALOGE("Cannot remove Samsung fingerprint database");
    }
    return ret;
}

int db_read_to_tz(void* device) {
    ALOGV("----------------> %s ----------------->", __FUNCTION__);
    vcs_fingerprint_device_t* vdev = (vcs_fingerprint_device_t*)device;
    char* errmsg;
    int ret = 0;
    char cmd[MAX_DATABASE_CMD];
    sqlite3_stmt *stat;

    db_check_and_create_table(vdev);

    if (!access(SAMSUNG_FP_DB_PATH, 0)) {
        ALOGI("Samsung fingerprint database exist! Convert it");
        return db_convert_old_db(vdev);
    }

    memset(tz.finger, 0, MAX_NUM_FINGERS * sizeof(finger_t));
    sprintf(cmd, "select * from gid_%d", vdev->active_gid);
    sqlite3_prepare(vdev->db, cmd, -1, &stat, 0);
    while(1) {
        int ret = sqlite3_step(stat);
        if (ret != SQLITE_ROW) {
            break;
        }
        int id = sqlite3_column_int(stat, 0);
        const void *data = sqlite3_column_blob(stat, 1);
        memcpy(tz.finger[id].data, data, FINGER_DATA_MAX_LENGTH);
        const void *payload = sqlite3_column_blob(stat, 2);
        memcpy(tz.finger[id].payload, payload, PAYLOAD_MAX_LENGTH);
        ALOGV("read fingerprint data from database: id=%d", id);
        tz.finger[id].exist = true;
    }
    sqlite3_finalize(stat);

    return 0;
}

int db_write_to_db(void* device, bool remove, int fid) {
    ALOGV("----------------> %s ----------------->", __FUNCTION__);
    vcs_fingerprint_device_t* vdev = (vcs_fingerprint_device_t*)device;
    char* errmsg;
    int ret = 0;
    char cmd[MAX_DATABASE_CMD];
    sqlite3_stmt *stat;
    if (remove) {
        sprintf(cmd, "delete from gid_%d where id=%d", vdev->active_gid, fid);
        ret = sqlite3_exec(vdev->db, cmd, NULL, NULL, &errmsg);
        if (ret != SQLITE_OK) {
            ALOGE("Remove finger from database failed!");
        }
        memset(&tz.finger[fid], 0, sizeof(finger_t));
    } else {
        sprintf(cmd, "insert into gid_%d(id, data, payload) values( %d, ?, ?)", vdev->active_gid, fid);
        sqlite3_prepare(vdev->db, cmd, -1, &stat, 0);
        sqlite3_bind_blob(stat, 1, tz.finger[fid].data, (int)FINGER_DATA_MAX_LENGTH, NULL);
        sqlite3_bind_blob(stat, 2, tz.finger[fid].payload, (int)PAYLOAD_MAX_LENGTH, NULL);
        ret = sqlite3_step(stat);
        sqlite3_finalize(stat);
    }
    return ret;
}

int db_init(void* device) {
    ALOGV("----------------> %s ----------------->", __FUNCTION__);
    vcs_fingerprint_device_t* vdev = (vcs_fingerprint_device_t*)device;
    int ret = 0;
    ret = sqlite3_open(FINGER_DATABASE_FILENAME, &vdev->db);
    if (ret != SQLITE_OK) {
        ALOGE("Open finger database failed!");
        return -1;
    }
    return db_read_to_tz(vdev);
}

int db_uninit(void* device) {
    ALOGV("----------------> %s ----------------->", __FUNCTION__);
    vcs_fingerprint_device_t* vdev = (vcs_fingerprint_device_t*)device;
    int ret = 0;
    ret = sqlite3_close(vdev->db);
    if (ret != SQLITE_OK) {
        ALOGE("Close finger database failed!");
    }
    return ret;
}

/***************************************notice***************************************/
fingerprint_acquired_info_t convert_ret_to_acquired_info(int acquired_ret) { //TODO
    ALOGV("----------------> %s ----------------->acquired_ret=%d", __FUNCTION__, acquired_ret);
    fingerprint_acquired_info_t ret = FINGERPRINT_ACQUIRED_GOOD;
    switch (acquired_ret) {
        case 0:
            break;
        default:
            ret = FINGERPRINT_ACQUIRED_INSUFFICIENT;
    }
    return ret;
}

void send_error_notice(void* device, int error_info_int) {
    ALOGV("----------------> %s ----------------->", __FUNCTION__);
    vcs_fingerprint_device_t* vdev = (vcs_fingerprint_device_t*)device;
    fingerprint_error_t error_info = (fingerprint_error_t)error_info_int;

    fingerprint_msg_t msg;
    msg.type = FINGERPRINT_ERROR;
    msg.data.error = error_info;
    ALOGV("recevied error notice! error_info=%d", (int)error_info);

    pthread_mutex_lock(&vdev->lock);
    vdev->device.notify(&msg);
    pthread_mutex_unlock(&vdev->lock);

    return;
}

void send_acquired_notice(void* device, int acquired_ret) {
    ALOGV("----------------> %s ----------------->", __FUNCTION__);
    vcs_fingerprint_device_t* vdev = (vcs_fingerprint_device_t*)device;
    fingerprint_acquired_info_t acquired_info = convert_ret_to_acquired_info(acquired_ret);

    fingerprint_msg_t acqu_msg;
    acqu_msg.type = FINGERPRINT_ACQUIRED;
    acqu_msg.data.acquired.acquired_info = acquired_info;
    ALOGI("acqu_info=%d", (int)acquired_info);

    pthread_mutex_lock(&vdev->lock);
    vdev->device.notify(&acqu_msg);
    pthread_mutex_unlock(&vdev->lock);

    return;
}

void send_enroll_notice(void* device, int fid, int remaining) {
    ALOGV("----------------> %s -----------------> fid %d", __FUNCTION__, fid);
    vcs_fingerprint_device_t* vdev = (vcs_fingerprint_device_t*)device;

    if (fid == 0) {
        ALOGE("Fingerprint ID is zero (invalid)");
        return;
    }
    if (vdev->secure_user_id == 0) {
        ALOGE("Secure user ID is zero (invalid)");
        return;
    }

    pthread_mutex_lock(&vdev->lock);

    fingerprint_msg_t msg;
    msg.type = FINGERPRINT_TEMPLATE_ENROLLING;
    msg.data.enroll.finger.fid = fid;
    msg.data.enroll.samples_remaining = remaining;
    vdev->device.notify(&msg);

    pthread_mutex_unlock(&vdev->lock);

    return;
}

void send_authenticated_notice(void* device, int fid) {
    ALOGV("----------------> %s ----------------->", __FUNCTION__);
    vcs_fingerprint_device_t* vdev = (vcs_fingerprint_device_t*)device;

    send_acquired_notice(vdev, FINGERPRINT_ACQUIRED_GOOD);

    fingerprint_msg_t auth_msg;
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

void send_remove_notice(void* device, int fid) {
    ALOGV("----------------> %s ----------------->fid=%d", __FUNCTION__, fid);
    vcs_fingerprint_device_t* vdev = (vcs_fingerprint_device_t*)device;

    fingerprint_msg_t msg;
    msg.type = FINGERPRINT_TEMPLATE_REMOVED;
    msg.data.removed.finger.fid = fid;

    pthread_mutex_lock(&vdev->lock);
    vdev->device.notify(&msg);
    pthread_mutex_unlock(&vdev->lock);

    return;
}

/***************************************HAL function***************************************/

static uint64_t fingerprint_get_auth_id(struct fingerprint_device* device) {
    vcs_fingerprint_device_t* vdev = (vcs_fingerprint_device_t*)device;
    ALOGV("----------------> %s ----------------->", __FUNCTION__);
    uint64_t authenticator_id = 0;
    pthread_mutex_lock(&vdev->lock);
    vdev->authenticator_id = hash_file(FINGER_DATABASE_FILENAME);
    authenticator_id = vdev->authenticator_id;
    pthread_mutex_unlock(&vdev->lock);

    return authenticator_id;
}

static int fingerprint_set_active_group(struct fingerprint_device *device, uint32_t gid,
        const char __unused *path) {
    ALOGV("----------------> %s ----------------->", __FUNCTION__);
    vcs_fingerprint_device_t* vdev = (vcs_fingerprint_device_t*)device;
    pthread_mutex_lock(&vdev->lock);
    vdev->active_gid = gid;
    db_read_to_tz(vdev);
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

    pthread_mutex_lock(&vdev->lock);

    vdev->op_id = operation_id;
    ret = vcs_start_authenticate(vdev);

    pthread_mutex_unlock(&vdev->lock);

    return ret;
}

static int fingerprint_enroll(struct fingerprint_device *device,
        const hw_auth_token_t *hat,
        uint32_t __unused gid,
        uint32_t timeout_sec) {
    ALOGV("----------------> %s ----------------->", __FUNCTION__);
    vcs_fingerprint_device_t* vdev = (vcs_fingerprint_device_t*)device;
    int ret = -EINVAL;
    int idx = 1;

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

    for (idx = 1; idx <= MAX_NUM_FINGERS; idx++) {
        if (!tz.finger[idx].exist) {
            break;
        }
    }
    if (idx > MAX_NUM_FINGERS) {
        send_error_notice(vdev, FINGERPRINT_ERROR_NO_SPACE);
        pthread_mutex_unlock(&vdev->lock);
        return -1;
    }
    ret = vcs_start_enroll(vdev, timeout_sec);

    pthread_mutex_unlock(&vdev->lock);
    ALOGV("enroll ret=%d",ret);

    return ret;
}

static uint64_t fingerprint_pre_enroll(struct fingerprint_device *device) {
    ALOGV("----------------> %s ----------------->", __FUNCTION__);
    uint64_t challenge = 0;
    vcs_fingerprint_device_t* vdev = (vcs_fingerprint_device_t*)device;

    challenge = hash_string(tz.auth_token);

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
    int flag = 0;

    if (tz.timeout.timeout_thread) {
        pthread_mutex_lock(&tz.timeout.lock);
        pthread_cond_signal(&tz.timeout.cond);
        pthread_mutex_unlock(&tz.timeout.lock);
        pthread_join(tz.timeout.timeout_thread, NULL);
        tz.timeout.timeout_thread = 0;
    }
    if (get_tz_state() != STATE_IDLE && get_tz_state() != STATE_CANCEL) {
        set_tz_state(STATE_CANCEL);
        ioctl(sensor.fd, VFSSPI_IOCTL_SET_DRDY_INT, &flag);
        pthread_mutex_lock(&sensor.lock);
        pthread_cond_signal(&sensor.cond);
        pthread_mutex_unlock(&sensor.lock);
        while (1) {
            usleep(100000);
            if (tz.state == STATE_IDLE)
                break;
        }
        // Not really an error, but framework expects us to send it.
        send_error_notice(vdev, FINGERPRINT_ERROR_CANCELED);
    }

    return ret;
}

static int fingerprint_enumerate(struct fingerprint_device *device) {
    ALOGV("----------------> %s ----------------->", __FUNCTION__);
    if (device == NULL) {
        ALOGE("Cannot enumerate saved fingerprints with uninitialized params");
        return -1;
    }

    int num = 0;

    vcs_fingerprint_device_t* vdev = (vcs_fingerprint_device_t*)device;

    pthread_mutex_lock(&vdev->lock);
    num = vcs_get_enrolled_finger_num();
    pthread_mutex_unlock(&vdev->lock);

    return num;
}

static int fingerprint_remove(struct fingerprint_device *device,
        uint32_t __unused gid, uint32_t fid) {
    int ret = 0;
    ALOGV("----------------> %s -----------------> fid %d", __FUNCTION__, fid);
    if (device == NULL) {
        ALOGE("Can't remove fingerprint (gid=%d, fid=%d); "
              "device not initialized properly",
              gid, fid);
        return -1;
    }

    vcs_fingerprint_device_t* vdev = (vcs_fingerprint_device_t*)device;

    if (fid == 0) {
        // Delete all fingerprints
        int idx = 1;
        for (idx = 1; idx <= MAX_NUM_FINGERS; idx++)
            if (tz.finger[idx].exist) {
                pthread_mutex_lock(&vdev->lock);
                ret = db_write_to_db(vdev, true, idx);
                pthread_mutex_unlock(&vdev->lock);
                if (ret == 0) {
                    send_remove_notice(vdev, idx);
                }
            }
    } else {
        // Delete one fingerprint
        pthread_mutex_lock(&vdev->lock);
        ret = db_write_to_db(vdev, true, fid);
        pthread_mutex_unlock(&vdev->lock);
        if (ret == 0) {
            send_remove_notice(vdev, fid);
        }

    }

    if (ret) {
        ALOGE("Can't remove finger %d", fid);
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
    device->notify = notify;
    pthread_mutex_unlock(&vdev->lock);
    ALOGD("fingerprint callback notification set");

    return 0;
}

static int fingerprint_close(hw_device_t* device) {
    ALOGV("----------------> %s ----------------->", __FUNCTION__);
    if (device == NULL) {
        ALOGE("fingerprint hw device is NULL");
        return -1;
    }

    vcs_fingerprint_device_t* vdev = (vcs_fingerprint_device_t*)device;

    fingerprint_cancel(&vdev->device);

    pthread_mutex_lock(&vdev->lock);
    db_uninit(vdev);
    sensor_init();
    vcs_init();
    vcs_stop_auth_session();
    vcs_uninit();
    sensor_uninit();
    pthread_mutex_unlock(&vdev->lock);

    pthread_mutex_destroy(&vdev->lock);
    pthread_mutex_destroy(&sensor.lock);
    pthread_mutex_destroy(&tz.lock);
    pthread_mutex_destroy(&tz.timeout.lock);
    free(vdev);
    vdev = NULL;

    if (close(sensor.fd) < 0) {
        ALOGE("Close sensor error!");
    }
    sensor.fd = -1;

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
    vdev->device.common.version = FINGERPRINT_MODULE_API_VERSION_2_1;
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
    memset(&tz, 0, sizeof(trust_zone_t));

    pthread_mutex_init(&vdev->lock, NULL);
    pthread_mutex_init(&sensor.lock, NULL);
    pthread_mutex_init(&tz.lock, NULL);
    pthread_mutex_init(&tz.timeout.lock, NULL);
    pthread_cond_init(&tz.timeout.cond, NULL);
    pthread_cond_init(&sensor.cond, NULL);
    sensor.signal = false;
    tz.timeout.timeout_thread = 0;

    *device = &vdev->device.common;

    memset(&sensor, 0, sizeof(vcs_sensor_t));
    sensor.fd = open(SENSOR_FILE_NAME, O_RDWR);
    if (sensor.fd < 0) {
        ALOGE("Open sensor error!");
        return -1;
    }
    sensor_init();
    sensor_register();
    sensor_uninit();

    pthread_mutex_lock(&vdev->lock);
    sensor_init();
    db_init(vdev);
    vcs_init();
    vcs_uninit();
    vcs_init();
    vcs_start_auth_session();
    vcs_uninit();
    sensor_uninit();
    pthread_mutex_unlock(&vdev->lock);

    return 0;
}

static struct hw_module_methods_t fingerprint_module_methods = {
    .open = fingerprint_open,
};

fingerprint_module_t HAL_MODULE_INFO_SYM = {
    .common = {
        .tag                = HARDWARE_MODULE_TAG,
        .module_api_version = FINGERPRINT_MODULE_API_VERSION_2_1,
        .hal_api_version    = HARDWARE_HAL_API_VERSION,
        .id                 = FINGERPRINT_HARDWARE_MODULE_ID,
        .name               = "KLTE Fingerprint HAL",
        .author             = "ljzyal(ljzyal@gmail.com)",
        .methods            = &fingerprint_module_methods,
    },
};
