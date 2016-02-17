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

    private native int jniEnableSensorDevice(int enable);
    private native int jniEnrollUser(Object enrollInfo);
    private native int jniEnrollUser(String userId, int fingerIndex, int mode);
    private native int jniEnrollUser(String userId, String appData, int fingerIndex);
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
