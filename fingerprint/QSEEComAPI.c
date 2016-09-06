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

#define LOG_TAG "Custom_QSEE"
#define LOG_NDEBUG 0

#include <cutils/log.h>

#include <stdlib.h>
#include <dlfcn.h>

#include "QSEEComAPI.h"

struct qcom_keymaster_handle {
    void *libhandle;
    int (*QSEECom_start_app)(struct QSEECom_handle ** handle, const char* path,
                          const char* appname, uint32_t size);
    int (*QSEECom_shutdown_app)(struct QSEECom_handle **handle);
    int (*QSEECom_send_cmd)(struct QSEECom_handle* handle, void *cbuf,
                          uint32_t clen, void *rbuf, uint32_t rlen);
    int (*QSEECom_send_modified_cmd)(struct QSEECom_handle* handle, void *cbuf,
                          uint32_t clen, void *rbuf, uint32_t rlen,
                          struct QSEECom_ion_fd_info *ihandle);
    int (*QSEECom_set_bandwidth)(struct QSEECom_handle* handle, bool high);
};
typedef struct qcom_keymaster_handle qcom_keymaster_handle_t;

bool init = false;
qcom_keymaster_handle_t* km_handle;
int file_num = 0;

static int qcom_km_get_lib_sym(qcom_keymaster_handle_t* km_handle)
{
    km_handle->libhandle = dlopen("libQSEEComAPI.so", RTLD_NOW);
    if (  km_handle->libhandle  ) {
        *(void **)(&km_handle->QSEECom_start_app) =
                               dlsym(km_handle->libhandle,"QSEECom_start_app");
        if (km_handle->QSEECom_start_app == NULL) {
               ALOGE("dlsym: Error Loading QSEECom_start_app");
                   dlclose(km_handle->libhandle );
                   km_handle->libhandle  = NULL;
                   return -1;
            }
            *(void **)(&km_handle->QSEECom_shutdown_app) =
                               dlsym(km_handle->libhandle,"QSEECom_shutdown_app");
            if (km_handle->QSEECom_shutdown_app == NULL) {
                   ALOGE("dlsym: Error Loading QSEECom_shutdown_app");
                   dlclose(km_handle->libhandle );
                   km_handle->libhandle  = NULL;
                   return -1;
             }
            *(void **)(&km_handle->QSEECom_send_cmd) =
                               dlsym(km_handle->libhandle,"QSEECom_send_cmd");
            if (km_handle->QSEECom_send_cmd == NULL) {
                   ALOGE("dlsym: Error Loading QSEECom_send_cmd");
                   dlclose(km_handle->libhandle );
                   km_handle->libhandle  = NULL;
                   return -1;
             }
            *(void **)(&km_handle->QSEECom_send_modified_cmd) =
                               dlsym(km_handle->libhandle,"QSEECom_send_modified_cmd");
            if (km_handle->QSEECom_send_modified_cmd == NULL) {
                   ALOGE("dlsym: Error Loading QSEECom_send_modified_cmd");
                   dlclose(km_handle->libhandle );
                   km_handle->libhandle  = NULL;
                   return -1;
             }
            *(void **)(&km_handle->QSEECom_set_bandwidth) =
                               dlsym(km_handle->libhandle,"QSEECom_set_bandwidth");
            if (km_handle->QSEECom_set_bandwidth == NULL) {
                   ALOGE("dlsym: Error Loading QSEECom_set_bandwidth");
                   dlclose(km_handle->libhandle );
                   km_handle->libhandle  = NULL;
                   return -1;
             }

    } else {
        ALOGE("failed to load qseecom library");
        return -1;
    }
    return 0;
}

static int init_qsee()
{
    if (init) {
        return 0;
    }
    int ret = 0;
    km_handle = (qcom_keymaster_handle_t *)malloc(sizeof(qcom_keymaster_handle_t));
    if (km_handle == NULL) {
        ALOGE("Memalloc for keymaster handle failed");
        return -1;
    }
    km_handle->libhandle = NULL;
    ret = qcom_km_get_lib_sym(km_handle);
    if (ret) {
        free(km_handle);
        return -1;
    }
    init = true;
    return 0;
}


int QSEECom_start_app(struct QSEECom_handle **clnt_handle, const char *path,
			const char *fname, uint32_t sb_size)
{
    init_qsee();
    int ret = 0;
    ret = (*km_handle->QSEECom_start_app)(clnt_handle,path,fname,sb_size);
    return ret;
}

int QSEECom_shutdown_app(struct QSEECom_handle **handle)
{
    init_qsee();
    int ret = 0;
    ret = (*km_handle->QSEECom_shutdown_app)(handle);
    return ret;
}

int QSEECom_send_cmd(struct QSEECom_handle *handle, void *send_buf,
			uint32_t sbuf_len, void *rcv_buf, uint32_t rbuf_len)
{
    init_qsee();
    int ret = 0;
    ret = (*km_handle->QSEECom_send_cmd)(handle,send_buf,sbuf_len,rcv_buf,rbuf_len);
    return ret;
}

int QSEECom_send_modified_cmd(struct QSEECom_handle *handle, void *send_buf,
			uint32_t sbuf_len, void *resp_buf, uint32_t rbuf_len,
			struct QSEECom_ion_fd_info  *ifd_data)
{
    init_qsee();
    int ret = 0;
    ret = (*km_handle->QSEECom_send_modified_cmd)(handle,send_buf,sbuf_len,resp_buf,rbuf_len,ifd_data);
    return ret;
}

int QSEECom_set_bandwidth(struct QSEECom_handle *handle, bool high)
{
    init_qsee();
    int ret = 0;
    ret = (*km_handle->QSEECom_set_bandwidth)(handle,high);
    return ret;
}

