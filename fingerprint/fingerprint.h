/*
 * Copyright (C) 2017 The LineageOS Project
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

#ifdef __cplusplus
extern "C" {
#endif

#include <sqlite3.h>

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

uint64_t fingerprint_get_auth_id();
int fingerprint_set_active_group(uint32_t gid, const char __unused *path);
int fingerprint_authenticate(uint64_t operation_id, __unused uint32_t gid);
int fingerprint_enroll(const hw_auth_token_t *hat, uint32_t __unused gid,
        uint32_t timeout_sec);
uint64_t fingerprint_pre_enroll();
int fingerprint_post_enroll();
int fingerprint_cancel();
int fingerprint_enumerate();
int fingerprint_remove(uint32_t __unused gid, uint32_t fid);
int set_notify_callback(fingerprint_notify_t notify);
void fingerprint_deinit(void);
void fingerprint_init(void);

#ifdef __cplusplus
}
#endif
