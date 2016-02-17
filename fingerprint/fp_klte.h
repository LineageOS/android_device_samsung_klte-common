/*
 * Copyright (C) 2014 The Android Open Source Project
 * Copyright (C) 2016 The CyanogenMod Project
 * Copyright (C) 2016 The Mokee Project
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

#include <hardware/hardware.h>
#include <hardware/fingerprint.h>

#define CALL_BASE 0

#define CALL_INITSERVICE 1
#define CALL_ENROLL 2
//userId ,fingerIndex
#define CALL_CANCEL 3
#define CALL_REMOVE 4
//userId ,fingerIndex
#define CALL_IDENTIFY 5
//userId
#define CALL_GET_ENROLLED_FINGER_LIST  6
//userId
#define CALL_CLEANUP 7

typedef enum worker_state_t {
    STATE_IDLE = 0,
    STATE_ENROLL,
    STATE_SCAN,
    STATE_EXIT
} worker_state_t;

typedef struct worker_thread_t {
    pthread_t thread;
    worker_state_t state;
} worker_thread_t;

typedef struct vcs_fingerprint_device_t {
    fingerprint_device_t device;  // "inheritance"
    worker_thread_t listener;
    uint64_t op_id;
    uint64_t challenge;
    uint64_t user_id;
    uint64_t group_id;
    uint64_t secure_user_id;
    uint64_t authenticator_id;
    uint32_t active_gid;
    int send_fd;  //send to validity service
    int receive_fd;  //recevie from validity service
    bool init;
    pthread_mutex_t lock;
} vcs_fingerprint_device_t;

static __inline__ int fd_write(int fd, const void* buff, int len){
	int len2;
	do {
		len2 = write(fd, buff, len);
	} while (len2 < 0 && errno == EINTR);
	return len2;
}
static __inline__ int fd_read(int fd, void* buff, int len){
	int len2;
	do {
		len2 = read(fd, buff, len);
	} while (len2 < 0 && errno == EINTR);
	return len2;
}

static void checkinit(vcs_fingerprint_device_t* vdev);
static int sendcommand(vcs_fingerprint_device_t* vdev, uint8_t* command, int num);
static int getfingermask(vcs_fingerprint_device_t* vdev);
static int initService(vcs_fingerprint_device_t* vdev);

static void send_error_notice(vcs_fingerprint_device_t* vdev, fingerprint_error_t error_info);
static void send_acquired_notice(vcs_fingerprint_device_t* vdev, fingerprint_acquired_info_t acquired_info);
static void send_enroll_notice(vcs_fingerprint_device_t* vdev, int fid, int remaining);
static void send_authenticated_notice(vcs_fingerprint_device_t* vdev, int fid);
static void send_remove_notice(vcs_fingerprint_device_t* vdev, int fid);

static uint64_t get_64bit_rand();
static uint64_t fingerprint_get_auth_id(struct fingerprint_device* device);
static int fingerprint_set_active_group(struct fingerprint_device *device, uint32_t gid,
        const char __unused *path);
static int fingerprint_authenticate(struct fingerprint_device *device,
    uint64_t operation_id, __unused uint32_t gid);
static int fingerprint_enroll(struct fingerprint_device *device,
        const hw_auth_token_t *hat,
        uint32_t __unused gid,
        uint32_t __unused timeout_sec);
static uint64_t fingerprint_pre_enroll(struct fingerprint_device *device);
static int fingerprint_post_enroll(struct fingerprint_device* device);
static int fingerprint_cancel(struct fingerprint_device *device);
static int fingerprint_enumerate(struct fingerprint_device *device,
        fingerprint_finger_id_t *results, uint32_t *max_size);
static int fingerprint_remove(struct fingerprint_device *device,
        uint32_t __unused gid, uint32_t fid);
static int set_notify_callback(struct fingerprint_device *device,
                               fingerprint_notify_t notify);

static worker_state_t getListenerState(vcs_fingerprint_device_t* dev);
static void* listenerSocket(void* data);

static int fingerprint_close(hw_device_t* device);
static int fingerprint_open(const hw_module_t* module, const char __unused *id,
                            hw_device_t** device);

