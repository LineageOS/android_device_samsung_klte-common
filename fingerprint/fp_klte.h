/*
 * Copyright (C) 2016 The Android Open Source Project
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

#include "fingerprint_common.h"
#include "vfs61xx_ioctl.h"

#define FINGER_DATABASE_FILENAME "/data/validity/finger.db"
#define SAMSUNG_FP_DB_PATH "/data/validity/template.db"
#define SENSOR_FILE_NAME "/dev/vfsspi"
#define MAX_DATABASE_CMD 255

int sensor_uninit();
int sensor_init();
void sensor_process_signal(int signum);
int sensor_register();
int sensor_capture_start();

int db_check_and_create_table(void* device);
int db_read_to_tz(void *device);
int db_write_to_db(void *device, bool remove, int fid);
int db_init(void *device);
int db_uninit(void *device);

void send_error_notice(void* device, int error_info_int);
void send_acquired_notice(void* device, int acquired_ret);
void send_enroll_notice(void *device, int fid, int remaining);
void send_authenticated_notice(void *device, int fid);
void send_remove_notice(void *device, int fid);


