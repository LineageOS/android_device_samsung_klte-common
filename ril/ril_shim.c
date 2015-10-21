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

#define LOG_TAG "SEC_RIL_SHIM"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <utils/Log.h>

//various funcs we'll need to call, in their mangled form

//android::Parcel::writeString16(char16_t const*, unsigned int)
extern void _ZN7android6Parcel13writeString16EPKDsj(void **str16P,
        uint16_t const *str, unsigned int len);

//code exports we provide

void _ZN7android6Parcel13writeString16EPKtj(void **str16P,
        unsigned short const *str, unsigned int len);

//library on-load and on-unload handlers (to help us set things up and tear them down)
void libEvtLoading(void) __attribute__((constructor));
void libEvtUnloading(void) __attribute__((destructor));

/*
 * FUNCTION: android::Parcel::writeString16(unsigned short const*, unsigned int)
 * USE:      INTERPOSE: write String16 to binder
 * NOTES:    This function no longer exists in M, instead now one must pass
 *           in a char16_t instead of a short. M So we'll craft the same call here.
 */
void _ZN7android6Parcel13writeString16EPKtj(void **str16P,
        unsigned short const *str, unsigned int len)
{
    _ZN7android6Parcel13writeString16EPKDsj(str16P, str, len);
}

/*
 * FUNCTION: libEvtLoading()
 * USE:      Handle library loading
 * NOTES:    This is a good time to log the fact that we were loaded and plan to
 *           do our thing.
 */
void libEvtLoading(void)
{
    ALOGV("libbinder interposition library loaded");
}

/*
 * FUNCTION: libEvtUnloading()
 * USE:      Handle library unloading
 * NOTES:    This is a good time to free whatever is unfreed and say goodbye
 */
void libEvtUnloading(void)
{
    ALOGV("libbinder interposition library unloading. Goodbye...");
}
