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

package com.validity.fingerprint;

public class VcsEvents {

    public static final int FINGER_INDEX_LEFT_THUMB = 1;
    public static final int FINGER_INDEX_LEFT_INDEX = 2;
    public static final int FINGER_INDEX_LEFT_MIDDLE = 3;
    public static final int FINGER_INDEX_LEFT_RING = 4;
    public static final int FINGER_INDEX_LEFT_LITTLE = 5;
    public static final int FINGER_INDEX_RIGHT_THUMB = 6;
    public static final int FINGER_INDEX_RIGHT_INDEX = 7;
    public static final int FINGER_INDEX_RIGHT_MIDDLE = 8;
    public static final int FINGER_INDEX_RIGHT_RING = 9;
    public static final int FINGER_INDEX_RIGHT_LITTLE = 10;
    public static final int FINGER_INDEX_LEFT_THUMB_SECOND = 11;
    public static final int FINGER_INDEX_LEFT_INDEX_SECOND = 12;
    public static final int FINGER_INDEX_LEFT_MIDDLE_SECOND = 13;
    public static final int FINGER_INDEX_LEFT_RING_SECOND = 14;
    public static final int FINGER_INDEX_LEFT_LITTLE_SECOND = 15;
    public static final int FINGER_INDEX_RIGHT_THUMB_SECOND = 16;
    public static final int FINGER_INDEX_RIGHT_INDEX_SECOND = 17;
    public static final int FINGER_INDEX_RIGHT_MIDDLE_SECOND = 18;
    public static final int FINGER_INDEX_RIGHT_RING_SECOND = 19;
    public static final int FINGER_INDEX_RIGHT_LITTLE_SECOND = 20;
    public static final int FINGER_INDEX_ALL = 21;

    public static final int VCS_EVT_SENSOR_REMOVED = 1;
    public static final int VCS_EVT_SENSOR_DETECTED = 2;
    public static final int VCS_EVT_SENSOR_READY_FOR_USE = 3;
    public static final int VCS_EVT_SENSOR_FAILED_INITIALIZATION = 4;
    public static final int VCS_EVT_SENSOR_FINGERPRINT_CAPTURE_COMPLETE = 5;
    public static final int VCS_EVT_SENSOR_RAW_FINGERPRINT_CAPTURE_COMPLETE = 6;
    public static final int VCS_EVT_SENSOR_FINGERPRINT_CAPTURE_FAILED = 7;
    public static final int VCS_EVT_SENSOR_FINGERPRINT_CAPTURE_START = 8;
    public static final int VCS_EVT_ALL_SENSORS_INITIALIZED = 9;
    public static final int VCS_EVT_SENSOR_FINGERPRINT_FAILED_SWIPE_RETRY = 10;
    public static final int VCS_EVT_FINGER_DETECTED = 11;
    public static final int VCS_EVT_ENROLL_COMPLETED = 13;
    public static final int VCS_EVT_VERIFY_COMPLETED = 14;
    public static final int VCS_EVT_IDENTIFY_COMPLETED = 15;
    public static final int VCS_EVT_ENROLL_NEXT_CAPTURE_START = 16;
    public static final int VCS_EVT_EIV_FINGERPRINT_CAPTURED = 17;
    public static final int VCS_EVT_FINGER_REMOVED = 20;
    public static final int VCS_EVT_ENROLL_CAPTURE_STATUS = 32;
    public static final int VCS_EVT_FINGER_SETTLED = 33;
    public static final int VCS_EVT_SET_IR_FLAGS = 37;
    public static final int VCS_EVT_SWIPE_DIRECTION = 41;
    public static final int VCS_EVT_SWIPE_SPEED_UPDATE = 42;
    public static final int VCS_EVT_EIV_FINGERPRINT_CAPTURED_BAD = 45;
    public static final int VCS_EVT_SENSOR_NON_RECOVERABLE = 46;
    public static final int VCS_EVT_ENVIRONMENT_NOISE_DETECTED = 47;
    public static final int VCS_EVT_EIV_FINGERPRINT_CAPTURE_REDUNDANT = 49;
    public static final int VCS_EVT_EIV_FINGERPRINT_CAPTURE_EARLY = 50;

    public static final int VCS_EVT_ENROLL_SUCCESS = 421;
    public static final int VCS_EVT_VERIFY_SUCCESS = 422;
    public static final int VCS_EVT_IDENTIFY_SUCCESS = 423;
    public static final int VCS_EVT_ENROLL_FAILED = 424;
    public static final int VCS_EVT_VERIFY_FAILED = 425;
    public static final int VCS_EVT_IDENTIFY_FAILED = 426;
    public static final int VCS_EVT_GESTURE = 427;
    public static final int VCS_EVT_SENSOR_INFO = 428;

    public static final int VCS_IMAGE_QUALITY_GOOD = 0;
    public static final int VCS_IMAGE_QUALITY_STICTION = 1;
    public static final int VCS_IMAGE_QUALITY_TOO_FAST = 2;
    public static final int VCS_IMAGE_QUALITY_TOO_SHORT = 4;
    public static final int VCS_IMAGE_QUALITY_REVERSE_MOTION = 8;
    public static final int VCS_IMAGE_QUALITY_TOO_SLOW = 16;
    public static final int VCS_IMAGE_QUALITY_SOMETHING_ON_THE_SENSOR = 512;
    public static final int VCS_IMAGE_QUALITY_ONE_HAND_SWIPE = 1024;
    public static final int VCS_IMAGE_QUALITY_FINGER_OFFSET = 4096;
    public static final int VCS_IMAGE_QUALITY_BAD_SWIPE = 8192;
    public static final int VCS_IMAGE_QUALITY_SKEW_TOO_LARGE = 32768;
    public static final int VCS_IMAGE_QUALITY_PRESSURE_TOO_LIGHT = 0x10000;
    public static final int VCS_IMAGE_QUALITY_FINGER_OFFSET_TOO_FAR_LEFT = 0x20000;
    public static final int VCS_IMAGE_QUALITY_FINGER_OFFSET_TOO_FAR_RIGHT = 0x40000;
    public static final int VCS_IMAGE_QUALITY_PRESSURE_TOO_HARD = 0x80000;
    public static final int VCS_IMAGE_QUALITY_ASP_DATA_INVALID = 0x100000;
    public static final int VCS_IMAGE_QUALITY_NOT_A_FINGER_SWIPE = 0x200000;
    public static final int VCS_IMAGE_QUALITY_BASELINE_DATA_INVALID = 0x400000;
    public static final int VCS_IMAGE_QUALITY_WET_FINGER = 0x1000000;
    public static final int VCS_IMAGE_QUALITY_FINGER_TOO_THIN = 0x2000000;
    public static final int VCS_IMAGE_QUALITY_PARTIAL_TOUCH = 0x8000000;
    public static final int VCS_IMAGE_QUALITY_EMPTY_TOUCH = 0x10000000;
    public static final int VCS_IMAGE_QUALITY_GESTURE = 0x20000000;
    public static final int VCS_IMAGE_QUALITY_PROCESS_FAILED = 0x40000000;
    public static final int VCS_IMAGE_QUALITY_PROCESS_FAILED_FATAL = 0x80000000;

    public static final int VCS_NOTIFY_AUTH_SESSION_BEGIN = 5;
    public static final int VCS_NOTIFY_AUTH_SESSION_END = 6;

    public static final int VCS_POLICY_AUTHENTICATE_ALWAYS = 1;
    public static final int VCS_POLICY_AUTHENTICATE_ON_SCREEN_UNLOCK_TIMEOUT = 2;
    public static final int VCS_POLICY_AUTHENTICATE_ON_PREVIOUS_IDENTIFY_TIMEOUT = 3;

    public static final int VCS_PROTECT_ALG_SIMPLE_USER_DATA = 1;
    public static final int VCS_PROTECT_ALG_SIMPLE_APP_DATA = 2;

    public static final int VCS_RESULT_FAILED = -1;
    public static final int VCS_RESULT_OK = 0;
    public static final int VCS_RESULT_CANCEL = 1;
    public static final int VCS_RESULT_OUT_OF_MEMORY = 2;
    public static final int VCS_RESULT_NO_MORE_ENTRIES = 3;
    public static final int VCS_RESULT_BAD_PARAM = 17;
    public static final int VCS_RESULT_SENSOR_NOT_FOUND = 19;
    public static final int VCS_RESULT_SENSOR_OPEN_FAILED = 22;
    public static final int VCS_RESULT_TIMEOUT = 34;
    public static final int VCS_RESULT_NULL_POINTER = 40;
    public static final int VCS_RESULT_SENSOR_BUSY = 46;
    public static final int VCS_RESULT_ERROR = 54;
    public static final int VCS_RESULT_TEMPLATE_FILE_CREATION_FAILED = 64;
    public static final int VCS_RESULT_USER_DOESNT_EXIST = 65;
    public static final int VCS_RESULT_TEMPLATE_DOESNT_EXIST = 66;
    public static final int VCS_RESULT_INVALID_USER_HANDLE = 70;
    public static final int VCS_RESULT_TEMPLATE_CORRUPTED = 87;
    public static final int VCS_RESULT_USER_FINGER_ALREADY_ENROLLED = 88;
    public static final int VCS_RESULT_USER_DATA_CORRUPTED = 91;
    public static final int VCS_RESULT_MATCHER_OPEN_FAILED = 300;
    public static final int VCS_RESULT_MATCHER_CLOSE_FAILED = 301;
    public static final int VCS_RESULT_MATCHER_MATCH_FAILED = 302;
    public static final int VCS_RESULT_MATCHER_EXTRACT_FAILED = 303;
    public static final int VCS_RESULT_MATCHER_ENROLL_FAILED = 304;
    public static final int VCS_RESULT_MATCHER_VERIFY_FAILED = 305;
    public static final int VCS_RESULT_MATCHER_ADD_IMAGE_FAILED = 306;
    public static final int VCS_RESULT_MATCHER_MATCH_IMAGE_FAILED = 307;
    public static final int VCS_RESULT_MATCHER_CHECK_QUALITY_FAILED = 308;
    public static final int VCS_RESULT_SERVICE_NOT_RUNNING = 321;
    public static final int VCS_RESULT_SENSOR_NOT_READY_FOR_USE = 358;
    public static final int VCS_RESULT_SENSOR_IS_REMOVED = 360;
    public static final int VCS_RESULT_OPERATION_CANCELED = 361;
    public static final int VCS_RESULT_USER_IDENTIFICATION_FAILED = 362;
    public static final int VCS_RESULT_USER_VERIFICATION_FAILED = 363;
    public static final int VCS_RESULT_MATCH_FAILED = 367;
    public static final int VCS_RESULT_SERVICE_STOPPED = 500;
    public static final int VCS_RESULT_GET_PRINT_STOPPED = 501;
    public static final int VCS_RESULT_OPERATION_DENIED = 510;
    public static final int VCS_RESULT_PIPE_ERROR = 511;
    public static final int VCS_RESULT_SENSOR_KEYS_NOT_READY = 512;
    public static final int VCS_RESULT_BAD_DEVICE = 513;
    public static final int VCS_RESULT_TOO_MANY_BAD_SWIPES = 515;
    public static final int VCS_RESULT_BAD_QUALITY_IMAGE = 601;
    public static final int VCS_RESULT_KEYDATA_NOT_FOUND = 777;
    public static final int VCS_RESULT_DATA_STORE_FAILED = 1001;
    public static final int VCS_RESULT_DATA_RETRIEVE_FAILED = 1002;
    public static final int VCS_RESULT_DATA_REMOVE_FAILED = 1003;
    public static final int VCS_RESULT_ALREADY_INPROGRESS = 1004;
    public static final int VCS_RESULT_NOT_IMPLEMENTED = 1005;
    public static final int VCS_RESULT_INVALID_ARGUMENT = 1006;

    public static final int VCS_SECURITY_LEVEL_LOW = 0;
    public static final int VCS_SECURITY_LEVEL_REGULAR = 1;
    public static final int VCS_SECURITY_LEVEL_HIGH = 2;
    public static final int VCS_SECURITY_LEVEL_VERY_HIGH = 3;

    public static final int VCS_SWIPE_DIRECTION_UP = 0;
    public static final int VCS_SWIPE_DIRECTION_DOWN = 1;
    public static final int VCS_SWIPE_DIRECTION_LEFT = 3;
    public static final int VCS_SWIPE_DIRECTION_RIGHT = 4;
    public static final int VCS_SWIPE_DIRECTION_REST = 5;
}
