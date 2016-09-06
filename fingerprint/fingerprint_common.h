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

#ifndef __FINGERPRINT_COMMON_H_
#define __FINGERPRINT_COMMON_H_

#include <stdbool.h>

#define SB_LENGTH (0x100000)
#define BUFFER_LENGTH (0x19000)
#define AUTH_TOKEN_LENGTH (0x70)
#define AUTH_SESSION_TOKEN_LENGTH (0x20)
#define CALIBRATE_DATA_MAX_LENGTH (0x500c)
#define FINGER_DATA_MAX_LENGTH (0x2900)
#define PAYLOAD_MAX_LENGTH (0x24)
#define MAX_NUM_FINGERS (5)

typedef enum worker_state_t {
    STATE_IDLE = 0,
    STATE_ENROLL,
    STATE_SCAN,
    STATE_CANCEL
} worker_state_t;

typedef struct vcs_sensor_t {
    int fd;
    bool init;
    bool signal;
    pthread_cond_t cond;
    pthread_mutex_t lock;
}vcs_sensor_t;

typedef struct finger_t {
    bool exist;
    char data[FINGER_DATA_MAX_LENGTH];
    char payload[PAYLOAD_MAX_LENGTH];
}finger_t;

typedef struct timeout_t {
    uint32_t timeout;
    pthread_t timeout_thread;
    pthread_cond_t cond;
    pthread_mutex_t lock;
}timeout_t;

typedef struct trust_zone_t {
    bool init;
    worker_state_t state;
    struct QSEECom_handle *qhandle;
    bool auth_session_opend;
    char auth_token[AUTH_TOKEN_LENGTH];
    char auth_session_token[AUTH_SESSION_TOKEN_LENGTH];
    int calibrate_len;
    char calibrate_data[CALIBRATE_DATA_MAX_LENGTH];
    finger_t finger[MAX_NUM_FINGERS + 1]; // Start from finger[1]
    timeout_t timeout;
    pthread_t auth_thread;
    pthread_t enroll_thread;
    pthread_mutex_t lock;
}trust_zone_t;

#endif //__FINGERPRINT_COMMON_H_
