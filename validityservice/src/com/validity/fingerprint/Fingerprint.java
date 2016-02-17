/*
 * Copyright (C) 2008 The Android Open Source Project
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

import android.content.Context;
import android.util.Log;

public class Fingerprint extends FingerprintCore {

    public static final int VCS_ENROLL_MODE_DEFAULT = 1;
    public static final int VCS_ENROLL_MODE_REENROLL = 2;
    public static final int VCS_ENROLL_MODE_SECONDARY = 3;

    public static final int VCS_FINGER_POSITION_FEEDBACK_SWIPE_MIDDLE = 11;
    public static final int VCS_FINGER_POSITION_FEEDBACK_SWIPE_MIDDLE_FULL = 12;
    public static final int VCS_FINGER_POSITION_FEEDBACK_SWIPE_LEFT = 21;
    public static final int VCS_FINGER_POSITION_FEEDBACK_SWIPE_RIGHT = 31;
    public static final int VCS_FINGER_POSITION_FEEDBACK_SWIPE_ANY = 41;

    public static final int VCS_NOTIFY_SNSR_TEST_CONTINUE = 1;
    public static final int VCS_NOTIFY_SNSR_TEST_STOP = 2;
    public static final int VCS_NOTIFY_ENROLL_BEGIN = 3;
    public static final int VCS_NOTIFY_ENROLL_END = 4;

    public static final int VCS_REQUEST_PROCESS_FIDO = 1;
    public static final int VCS_REQUEST_PROCESS_ALIPAY = 2;
    public static final int VCS_REQUEST_COMMAND_SENSOR_TEST = 11;
    public static final int VCS_REQUEST_GET_SENSOR_INFO = 12;
    public static final int VCS_REQUEST_GET_ENROLL_REPEAT_COUNT = 13;
    private static final int VCS_REQUEST_ENABLE_WOF = 21;
    private static final int VCS_REQUEST_GET_WOF_STATE = 22;

    public static final int VCS_SENSOR_STATUS_OK = 0;
    public static final int VCS_SENSOR_STATUS_WORKING = 46;
    public static final int VCS_SENSOR_STATUS_INITIALISING = 358;
    public static final int VCS_SENSOR_STATUS_INITIALIZING = 358;
    public static final int VCS_SENSOR_STATUS_OUT_OF_ORDER = 360;
    public static final int VCS_SENSOR_STATUS_MALFUNCTIONED = 534;
    public static final int VCS_SENSOR_STATUS_FAILURE = 535;

    public static final int VCS_WOF_STATE_INACTIVE = 0;
    public static final int VCS_WOF_STATE_ACTIVE = 1;

    public static final int VCS_SNSR_TEST_65AB_TN_NOFLASH_SCRIPT_ID = 2070;
    public static final int VCS_SNSR_TEST_65AB_TN_STON_CAL_SIG3_CHCK_SCRIPT_ID = 2078;
    public static final int VCS_SNSR_TEST_65AB_TN_STON_CHCK_SCRIPT_ID = 2076;
    public static final int VCS_SNSR_TEST_65AB_TN_STON_SSS_CHCK_SCRIPT_ID = 2077;
    public static final int VCS_SNSR_TEST_65AB_TN_STON_SSS_CHCK_TZ_SCRIPT_ID = 4077;
    public static final int VCS_SNSR_TEST_65AB_TS_NOFLASH_SCRIPT_ID = 2080;
    public static final int VCS_SNSR_TEST_65AB_TS_STON_CAL_SIG3_CHCK_SCRIPT_ID = 2088;
    public static final int VCS_SNSR_TEST_65AB_TS_STON_CHCK_SCRIPT_ID = 2086;
    public static final int VCS_SNSR_TEST_65AB_TS_STON_SSS_CHCK_SCRIPT_ID = 2087;
    public static final int VCS_SNSR_TEST_CHCK_RAPT_CAL_FAIL_BIN = 20;
    public static final int VCS_SNSR_TEST_CHECK_SCRIPT_CRC16_FLAG = 64;
    public static final int VCS_SNSR_TEST_CODE_FAIL_BIN = 2;
    public static final int VCS_SNSR_TEST_DATALOG_CALL_INFO_FLAG = 0x80000;
    public static final int VCS_SNSR_TEST_DATALOG_DATA_ONLY_TESTS_FLAG = 2;
    public static final int VCS_SNSR_TEST_DATALOG_FILE_APPEND_FLAG = 0x20000000;
    public static final int VCS_SNSR_TEST_DATALOG_FILE_CREATE_FLAG = 0x10000000;
    public static final int VCS_SNSR_TEST_DATALOG_GET_PRINT_FLAG = 8192;
    public static final int VCS_SNSR_TEST_DATALOG_INC_BIN_DESC_FLAG = 256;
    public static final int VCS_SNSR_TEST_DATALOG_INC_NOTES_FLAG = 1024;
    public static final int VCS_SNSR_TEST_DATALOG_INC_SCRIPT_DB_LIST_FLAG = 2048;
    public static final int VCS_SNSR_TEST_DATALOG_INC_TEST_LIST_FLAG = 512;
    public static final int VCS_SNSR_TEST_DATALOG_INTERMEIDATE_FLAG = 0x100000;
    public static final int VCS_SNSR_TEST_DATALOG_NS_REGION_DATA_FLAG = 128;
    public static final int VCS_SNSR_TEST_DATALOG_NS_WAVEFORM_FLAG = 64;
    public static final int VCS_SNSR_TEST_DATALOG_RAW_DEC_DATA_FLAG = 0x40000;
    public static final int VCS_SNSR_TEST_DATALOG_RAW_SENSOR_FLAG = 32;
    public static final int VCS_SNSR_TEST_DATALOG_SENSOR_INFO_FLAG = 4;
    public static final int VCS_SNSR_TEST_DATALOG_STON_CAL_SIG_DATA_FLAG = 0x10000;
    public static final int VCS_SNSR_TEST_DATALOG_STON_CAL_SIG_REG_DATA_FLAG = 0x20000;
    public static final int VCS_SNSR_TEST_DATALOG_STON_PIXEL_DATA_FLAG = 16384;
    public static final int VCS_SNSR_TEST_DATALOG_STON_SSS_DATA_FLAG = 32768;
    public static final int VCS_SNSR_TEST_DATALOG_TESTS_FLAG = 1;
    public static final int VCS_SNSR_TEST_DATALOG_THK_WAVEFORM_FLAG = 4096;
    public static final int VCS_SNSR_TEST_DATA_CLCT_FAIL_BIN = 8;
    public static final int VCS_SNSR_TEST_DATA_MISSING_FAIL_BIN = 9;
    public static final int VCS_SNSR_TEST_INFO_CHCK_FAIL_BIN = 6;
    public static final int VCS_SNSR_TEST_NO_CALLBACK_FAIL_BIN = 5;
    public static final int VCS_SNSR_TEST_NO_SENSOR_BIN = 250;
    public static final int VCS_SNSR_TEST_NO_TEST_SCRIPT_FAIL_BIN = 4;
    public static final int VCS_SNSR_TEST_NS_ENV_FAIL_BIN = 32;
    public static final int VCS_SNSR_TEST_NS_FAIL_BIN = 30;
    public static final int VCS_SNSR_TEST_NS_NOISE_FAIL_BIN = 34;
    public static final int VCS_SNSR_TEST_NS_REGION_FAIL_BIN = 31;
    public static final int VCS_SNSR_TEST_NS_UNIFORMITY_FAIL_BIN = 33;
    public static final int VCS_SNSR_TEST_OPTIONS_ATTEMPTS_MASK = 15;
    public static final int VCS_SNSR_TEST_OTP_FAIL_BIN = 40;
    public static final int VCS_SNSR_TEST_PASS_BIN = 1;
    public static final int VCS_SNSR_TEST_RESET_SENSOR_AFTER_TEST_FLAG = 32;
    public static final int VCS_SNSR_TEST_ROM_CHECKSUM_BIN = 45;
    public static final int VCS_SNSR_TEST_SCRIPT_DB_EXT_INT_FLAG = 0;
    public static final int VCS_SNSR_TEST_SCRIPT_DB_EXT_ONLY_FLAG = 768;
    public static final int VCS_SNSR_TEST_SCRIPT_DB_INT_EXT_FLAG = 256;
    public static final int VCS_SNSR_TEST_SCRIPT_DB_INT_ONLY_FLAG = 512;
    public static final int VCS_SNSR_TEST_SEND_CB_DATALOG_FLAG = 0x40000;
    public static final int VCS_SNSR_TEST_SEND_CB_SCRIPT_START_STOP_FLAG = 0x10000;
    public static final int VCS_SNSR_TEST_SEND_CB_SECT_START_STOP_FLAG = 0x20000;
    public static final int VCS_SNSR_TEST_SEND_CB_SNR_INFO_FLAG = 0x100000;
    public static final int VCS_SNSR_TEST_SEND_CB_TEST_INFO_FLAG = 0x80000;
    public static final int VCS_SNSR_TEST_SER_NUM_FAIL_BIN = 11;
    public static final int VCS_SNSR_TEST_SNSR_RUN_BIN = 70;
    public static final int VCS_SNSR_TEST_SPI42A46B_STON_CHCK_SCRIPT_ID = 2006;
    public static final int VCS_SNSR_TEST_SPI42A46B_STON_SSS_CHCK_SCRIPT_ID = 2007;
    public static final int VCS_SNSR_TEST_SPI46B_ID47_NOFLASH_SCRIPT_ID = 2010;
    public static final int VCS_SNSR_TEST_SPI48B_ID17_NOFLASH_SCRIPT_ID = 2020;
    public static final int VCS_SNSR_TEST_SPI48B_STON_CHCK_SCRIPT_ID = 2026;
    public static final int VCS_SNSR_TEST_SPI48B_STON_SSS_CHCK_SCRIPT_ID = 2027;
    public static final int VCS_SNSR_TEST_SPI48B_THK_CHCK2_SCRIPT_ID = 2024;
    public static final int VCS_SNSR_TEST_SPI60A_ID41_NOFLASH_SCRIPT_ID = 2030;
    public static final int VCS_SNSR_TEST_SPI60A_ID41_NOFLASH_TZ_SCRIPT_ID = 4030;
    public static final int VCS_SNSR_TEST_SPI60A_STON_CHCK_SCRIPT_ID = 2036;
    public static final int VCS_SNSR_TEST_SPI60A_STON_SSS_CHCK_SCRIPT_ID = 2037;
    public static final int VCS_SNSR_TEST_SPI60A_STON_SSS_CHCK_TZ_SCRIPT_ID = 4037;
    public static final int VCS_SNSR_TEST_SPI60A_THK_CHCK2_SCRIPT_ID = 2034;
    public static final int VCS_SNSR_TEST_SPI60M_ID2041_NOFLASH_SCRIPT_ID = 2060;
    public static final int VCS_SNSR_TEST_SPI60M_ID2041_NOFLASH_TZ_SCRIPT_ID = 4060;
    public static final int VCS_SNSR_TEST_SPI60M_STON_CAL_SIG2_CHCK_SCRIPT_ID = 2068;
    public static final int VCS_SNSR_TEST_SPI60M_STON_CHCK_SCRIPT_ID = 2066;
    public static final int VCS_SNSR_TEST_SPI60M_STON_SSS_CHCK_SCRIPT_ID = 2067;
    public static final int VCS_SNSR_TEST_SPI60M_STON_SSS_CHCK_TZ_SCRIPT_ID = 4067;
    public static final int VCS_SNSR_TEST_SPI65AB_TN_ID82_NOFLASH_TZ_SCRIPT_ID = 4070;
    public static final int VCS_SNSR_TEST_SPI65AB_TS_ID2082_NOFLASH_TZ_SCRIPT_ID = 4080;
    public static final int VCS_SNSR_TEST_STON_FAIL_BASE_BIN = 60;
    public static final int VCS_SNSR_TEST_STON_FAIL_DATA_CHCK_BIN = 64;
    public static final int VCS_SNSR_TEST_STON_FAIL_NOISE_BIN = 61;
    public static final int VCS_SNSR_TEST_STON_FAIL_SIGNAL_BIN = 62;
    public static final int VCS_SNSR_TEST_STON_FAIL_STON_BIN = 63;
    public static final int VCS_SNSR_TEST_STOP_ON_FAIL_FLAG = 16;
    public static final int VCS_SNSR_TEST_THK_FAIL_BIN = 51;
    public static final int VCS_SNSR_TEST_THK_SETUP_FAIL_BIN = 50;
    public static final int VCS_SNSR_TEST_USER_STOP_FAIL_BIN = 3;
    public static final int VCS_SNSR_TEST_VAL_SNSR_FAIL_BIN = 10;
    public static final int VCS_SNSR_TEST_WRONG_SECT_TYPE_VER_BIN = 7;

    public static final int VCS_EVT_SNSR_TEST_SCRIPT_START = 2001;
    public static final int VCS_EVT_SNSR_TEST_SECTION_START = 2002;
    public static final int VCS_EVT_SNSR_TEST_SECTION_END = 2003;
    public static final int VCS_EVT_SNSR_TEST_SCRIPT_END = 2004;
    public static final int VCS_EVT_SNSR_TEST_DATALOG_DATA = 2010;
    public static final int VCS_EVT_SNSR_TEST_RESET_AFTER_TEST_RES = 2022;
    public static final int VCS_EVT_SNSR_TEST_PUT_TERM_BLOCK_ON_SENSOR = 2023;
    public static final int VCS_EVT_SNSR_TEST_REMOVE_TERM_BLOCK_FROM_SENSOR = 2024;
    public static final int VCS_EVT_SNSR_TEST_PUT_STIMULUS_ON_SENSOR = 2025;
    public static final int VCS_EVT_SNSR_TEST_REMOVE_STIMULUS_FROM_SENSOR = 2026;
    public static final int VCS_EVT_SNSR_TEST_SNR_DATA = 2027;
    public static final int VCS_EVT_SNSR_TEST_PUT_STON_SSS_ON_SENSOR = 2028;
    public static final int VCS_EVT_SNSR_TEST_REMOVE_STON_SSS_FROM_SENSOR = 2029;

    private native int jniEnableSensorDevice(int enable);
    private native int jniEnrollUser(Object enrollInfo);
    private native int jniEnrollUser(String userId, int fingerIndex, int mode);
    private native int jniEnrollUser(String userId, String appData, int fingerIndex);
    private native int jniGetEnrolledTemplateIdByFinger(String userId, int fingerIndex, Object vcsTemplateIds);
    private native int jniGetEnrolledTemplateIds(String userId, Object vcsTemplateIds);
    private native int jniGetSensorStatus();
    private native int jniNotify(int code, Object data);
    private native int jniProcessFIDO(byte requestData[], VcsByteArray responseData);
    private native int jniReEnrollUser(String userId, String appData, int fingerIndex, int mode);
    private native int jniRemoveEnrolledFinger(Object enrollInfo);
    private native int jniRemoveEnrolledFinger(String userId, int fingerIndex);
    private native int jniRequest(int command, Object data);
    private native int jniSetPassword(String userId, byte abyte0[], byte newPwdHash[]); // What is abyte0 means?
    private native int jniVerifyPassword(String userId, byte pwdHash[]);

    public Fingerprint(Context ctx) {
        super(ctx);
    }

    public Fingerprint(Context ctx, FingerprintCore.EventListener listener) {
        super(ctx, listener);
    }

    public int enableSensorDevice(boolean enable) {
        if (mOperation != 150)
            return VCS_RESULT_ALREADY_INPROGRESS;
        return jniEnableSensorDevice((enable? 1: 0));
    }

    public int enableWakeOnFinger() {
        return jniRequest(VCS_REQUEST_ENABLE_WOF, null);
    }

    public int enroll(EnrollUser enrollInfo) {
        int ret = VCS_RESULT_FAILED;
        if (mOperation != 150)
            return ret;
        ret = jniEnrollUser(enrollInfo);
        if (ret == VCS_RESULT_OK)
            mOperation = 151;
        return ret;
    }

    public int enroll(String userId, int fingerIndex) {
        int ret = VCS_RESULT_FAILED;
        if (mOperation != 150)
            return ret;
        if (null == userId)
            userId = "";
        ret = jniEnrollUser(userId, fingerIndex, VCS_ENROLL_MODE_DEFAULT);
        if (ret == VCS_RESULT_OK)
            mOperation = 151;
        return ret;
    }

    public int enroll(String userId, int fingerIndex, int mode) {
        int ret = VCS_RESULT_FAILED;
        if (mOperation != 150)
            return ret;
        if (null == userId)
            userId = "";
        ret = jniEnrollUser(userId, fingerIndex, mode);
        if (ret == VCS_RESULT_OK)
            mOperation = 151;
        return ret;
    }

    public int enroll(String userId, String appData, int fingerIndex) {
        int ret = VCS_RESULT_FAILED;
        if (mOperation != 150)
            return ret;
        if (null == userId)
            userId = "";
        if (null == appData)
            appData = "";
        ret = jniEnrollUser(userId, appData, fingerIndex);
        if (ret == VCS_RESULT_OK)
            mOperation = 151;
        return ret;
    }

    public int enroll(String userId, String appData, int fingerIndex, int mode) {
        int ret = VCS_RESULT_FAILED;
        if (mOperation != 150)
            return ret;
        if (null == userId)
            userId = "";
        if (null == appData)
            appData = "";
        ret = jniReEnrollUser(userId, appData, fingerIndex, mode);
        if (ret == VCS_RESULT_OK)
            mOperation = 151;
        return ret;
    }

    public int getEnrolledTemplateIdByFinger(String userId, int fingerIndex, VcsTemplateIds vcsTemplateIds) {
        if (mOperation != 150)
            return VCS_RESULT_ALREADY_INPROGRESS;
        return jniGetEnrolledTemplateIdByFinger(userId, fingerIndex, vcsTemplateIds);
    }

    public int getEnrolledTemplateIds(String userId, VcsTemplateIds vcsTemplateIds) {
        if (mOperation != 150)
            return VCS_RESULT_ALREADY_INPROGRESS;
        return jniGetEnrolledTemplateIds(userId, vcsTemplateIds);
    }

    public int getSensorStatus() {
        if (mOperation != 150)
            return VCS_RESULT_ALREADY_INPROGRESS;
        return jniGetSensorStatus();
    }

    public int getWakeOnFingerState(VcsInt wofState) {
        if (wofState == null)
            return VCS_RESULT_INVALID_ARGUMENT;
        return jniRequest(VCS_REQUEST_GET_WOF_STATE, wofState);
    }

    public int notify(int code, Object data) {
        return jniNotify(code, data);
    }

    public int processFIDO(byte requestData[], VcsByteArray responseData) {
        if ((requestData == null) || (responseData == null))
            return VCS_RESULT_INVALID_ARGUMENT;
        return jniProcessFIDO(requestData, responseData);
    }

    public int removeEnrolledFinger(RemoveEnroll enrollInfo) {
        return jniRemoveEnrolledFinger(enrollInfo);
    }

    public int removeEnrolledFinger(String userId, int fingerIndex) {
        if (mOperation != 150)
            return VCS_RESULT_ALREADY_INPROGRESS;
        if (null == userId)
            userId = "";
        return jniRemoveEnrolledFinger(userId, fingerIndex);
    }

    public int removeEnrolledUser(String userId) {
        if (mOperation != 150)
            return VCS_RESULT_ALREADY_INPROGRESS;
        if (null == userId)
            userId = "";
        return jniRemoveEnrolledFinger(userId, FINGER_INDEX_LEFT_THUMB_SECOND);
    }

    public int request(int command, Object data) {
        return jniRequest(command, data);
    }

    public int setPassword(String userId, byte newPwdHash[]) {
        if (userId == null || newPwdHash == null)
            return VCS_RESULT_INVALID_ARGUMENT;
        return jniSetPassword(userId, "".getBytes(), newPwdHash);
    }

    public int verifyPassword(String userId, byte pwdHash[]) {
        if (userId == null || pwdHash == null)
            return VCS_RESULT_INVALID_ARGUMENT;
        return jniVerifyPassword(userId, pwdHash);
    }
}
