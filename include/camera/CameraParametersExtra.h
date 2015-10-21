/*
 * Copyright (C) 2014 The CyanogenMod Project
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

#define CAMERA_PARAMETERS_EXTRA_C \
const char CameraParameters::KEY_DIS[] = "dis"; \
const char CameraParameters::DIS_DISABLE[] = "disable"; \
\
const char CameraParameters::KEY_DYNAMIC_RANGE_CONTROL[] = "dynamic-range-control"; \
const char CameraParameters::KEY_SUPPORTED_DYNAMIC_RANGE_CONTROL[] = "dynamic-range-control-values"; \
const char CameraParameters::DRC_ON[] = "on"; \
const char CameraParameters::DRC_OFF[] = "off"; \
\
const char CameraParameters::KEY_PHASE_AF[] = "phase-af"; \
const char CameraParameters::KEY_SUPPORTED_PHASE_AF[] = "phase-af-values"; \
const char CameraParameters::PAF_ON[] = "on"; \
const char CameraParameters::PAF_OFF[] = "off"; \
\
const char CameraParameters::KEY_RT_HDR[] = "rt-hdr"; \
const char CameraParameters::KEY_SUPPORTED_RT_HDR[] = "rt-hdr-values"; \
const char CameraParameters::RTHDR_ON[] = "on"; \
const char CameraParameters::RTHDR_OFF[] = "off"; \
\
const char CameraParameters::KEY_ZSL[] = "zsl"; \
const char CameraParameters::ZSL_ON[] = "on"; \
const char CameraParameters::ZSL_OFF[] = "off"; \
\
int CameraParameters::getInt64(__attribute__((__unused__)) const char *key) const { return -1; } \
\
/* LAST_LINE OF CAMERA_PARAMETERS_EXTRA_C, every line before this one *MUST* have
 * a backslash \ at the end of the line or else everything will break.
 */

#define CAMERA_PARAMETERS_EXTRA_H \
    static const char KEY_DIS[]; \
    static const char DIS_DISABLE[]; \
    \
    static const char KEY_DYNAMIC_RANGE_CONTROL[]; \
    static const char KEY_SUPPORTED_DYNAMIC_RANGE_CONTROL[]; \
    static const char DRC_ON[]; \
    static const char DRC_OFF[]; \
    \
    static const char KEY_PHASE_AF[]; \
    static const char KEY_SUPPORTED_PHASE_AF[]; \
    static const char PAF_ON[]; \
    static const char PAF_OFF[]; \
    \
    static const char KEY_RT_HDR[]; \
    static const char KEY_SUPPORTED_RT_HDR[]; \
    static const char RTHDR_ON[]; \
    static const char RTHDR_OFF[]; \
    \
    static const char KEY_ZSL[]; \
    static const char ZSL_ON[]; \
    static const char ZSL_OFF[]; \
    \
    int getInt64(const char *key) const; \
    \
    /* LAST_LINE OF CAMERA_PARAMETERS_EXTRA_H, every line before this one *MUST* have
     * a backslash \ at the end of the line or else everything will break.
     */
