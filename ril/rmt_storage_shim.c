/*
 * Copyright (C) 2015 The CyanogenMod Project
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

#define LOG_TAG "RMT_STORAGE_SHIM"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/syscall.h>

#include <linux/ioprio.h>

#include <utils/Log.h>

//code exports we provide

int ioprio_set(int which, int who, int ioprio);

//library on-load and on-unload handlers (to help us set things up and tear them down)
void libEvtLoading(void) __attribute__((constructor));
void libEvtUnloading(void) __attribute__((destructor));

/*
 * FUNCTION: ioprio_set
 * USE:      INTERPOSE: Remaps to syscall(SYS_ioprio_set, ...)
 * NOTES:    This function no longer exists in M, instead remap this function
 *           make the appropriate syscall instead.
 */
int ioprio_set(int which, int who, int ioprio)
{
    return syscall(SYS_ioprio_set, which, who, ioprio);
}

/*
 * FUNCTION: libEvtLoading()
 * USE:      Handle library loading
 * NOTES:    This is a good time to log the fact that we were loaded and plan to
 *           do our thing.
 */
void libEvtLoading(void)
{
    ALOGV("rmt_storage interposition library loaded");
}

/*
 * FUNCTION: libEvtUnloading()
 * USE:      Handle library unloading
 * NOTES:    This is a good time to free whatever is unfreed and say goodbye
 */
void libEvtUnloading(void)
{
    ALOGV("rmt_storage interposition library unloading. Goodbye...");
}
