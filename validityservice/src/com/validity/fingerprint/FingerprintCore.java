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

public class FingerprintCore extends VcsEvents {
    public static interface EventListener {
        public abstract void onEvent(FingerprintEvent fingerprintevent);
    }
    public static final String API_VERSION = "0.7";
    public static final String VCS_POLICY_VERSION = "0.1";
    protected static final boolean DBG = false;
    protected static final int IDLE = 150;
    protected static final int ENROLL = 151;
    protected static final int IDENTIFY = 152;
    protected static final int STOREDATATOUSER = 153;
    protected static final int GETDATAFROMUSER = 154;
    protected static final int REMOVEDATAFROMUSER = 155;
    protected static final int GETPRINT = 156;
    protected static final String TAG = "Fingerprint";
    protected static final String USER_ID = "android";
    protected int mOperation;
    private EventListener mEventListener;
    private String mAppId;
    private String mUserId;
    private String mAppKey;
    private String mKeyData;

    private native int jniCancelOp();
    private native int jniCleanupVcs();
    private native int jniGetEnrolledFingerList(String userId, VcsInt fingermask);
    private native int jniGetFingerprintImage();
    private native String jniGetVersion();
    private native int jniIdentify(String userId);
    private native int jniInitVcs();
    private native int jniInitVcs(FingerprintCore fingerprintcore);
    private native int jniProtect(byte toProtect[], VcsAddInfo addInfo, VcsByteArray protectedData);
    private native int jniRegisterEventsCB(FingerprintCore fingerprintcore);
    private native int jniSetDetectFinger(int timeout);
    private native int jniSetSecurityLevel(int level);
    private native int jniUnprotect(byte protectedData[], VcsAddInfo addInfo, VcsByteArray unProtectedData);

    static {
        try {
            System.loadLibrary("vcsfp");
        }
        catch (Throwable e) {
            Log.e("Fingerprint", "Error loading library libvcsfp: " + e);
        }
    }

    public FingerprintCore(Context ctx) {
        mOperation = IDLE;
        mEventListener = null;
        mAppId = "";
        mUserId = "";
        mAppKey = "";
        mKeyData = "";
        jniInitVcs(this);
    }

    public FingerprintCore(Context ctx, EventListener listener) {
        mOperation = IDLE;
        mEventListener = null;
        mAppId = "";
        mUserId = "";
        mAppKey = "";
        mKeyData = "";
        VLog.i("init ret="+jniInitVcs(this));
        mEventListener = listener;
    }

    private boolean isOperationComplete(int eventId) {
        if (eventId == VCS_EVT_ENROLL_SUCCESS || eventId == VCS_EVT_IDENTIFY_SUCCESS || 
            eventId == VCS_EVT_VERIFY_SUCCESS || eventId == VCS_EVT_ENROLL_FAILED || 
            eventId == VCS_EVT_IDENTIFY_FAILED || eventId == VCS_EVT_VERIFY_FAILED || 
            eventId == VCS_EVT_GESTURE)
            return true;
        return false;
    }

    public synchronized void FingerprintEventCallback(FingerprintEvent event) {
        if (event == null) {
            Log.e("Fingerprint", "FP - EventsCB()::Invalid event data!");
            return;
        }
        if (isOperationComplete(event.eventId)) {
            Log.i("Fingerprint", "Operation complete, setting to IDLE");
            this.mOperation = IDLE;
        }
        if ((this.mOperation == GETPRINT) && ((event.eventId == VCS_EVT_EIV_FINGERPRINT_CAPTURED) || 
            (event.eventId == VCS_EVT_SENSOR_FINGERPRINT_CAPTURE_FAILED))) {
            this.mOperation = IDLE;
        }
        if (this.mEventListener != null) {
            this.mEventListener.onEvent(event);
        }
    }

    public int cancel() {
        int ret = VCS_RESULT_FAILED;
        if (mOperation == IDLE)
            return ret;
        ret = jniCancelOp();
        mOperation = IDLE;
        return ret;
    }

    public int cleanUp() {
        int ret = VCS_RESULT_FAILED;
        mEventListener = null;
        ret = jniCleanupVcs();
        return ret;
    }

    public int getEnrolledFingerList(String userId, VcsInt fingermask) {
        if (null == userId)
            userId = "";
        int ret = jniGetEnrolledFingerList(userId, fingermask);;
        return ret;
    }

    public int getFingerprintImage(){
        int ret = VCS_RESULT_FAILED;
        if (mOperation != IDLE)
            return ret;
        ret = jniGetFingerprintImage();
        if (ret == VCS_RESULT_OK)
            mOperation = GETPRINT;
        return ret;
    }

    public String getVersion() {
        if (mOperation != IDLE) {
            Log.e("Fingerprint", "Other operation is in progress, cancelling request");
            return null;
        }
        return jniGetVersion();
    }

    public int identify(String userId) {
        int ret = VCS_RESULT_FAILED;
        if (mOperation == IDENTIFY)
            return VCS_RESULT_ALREADY_INPROGRESS;
        if (mOperation != IDLE) {
            return ret;
        } else {
            if (null == userId)
                userId = "";
            ret = jniIdentify(userId);
            if (ret == VCS_RESULT_OK)
                mOperation = IDENTIFY;
        }
        return ret;
    }

    public int protect(byte toProtect[], VcsAddInfo addInfo, VcsByteArray protectedData) {
        if (toProtect == null || addInfo == null || protectedData == null)
            return VCS_RESULT_INVALID_ARGUMENT;
        return jniProtect(toProtect, addInfo, protectedData);
    }

    public int registerListener(EventListener listener) {
        if (mOperation != IDLE)
            return VCS_RESULT_FAILED;
        mEventListener = listener;
        return VCS_RESULT_OK;
    }

    public int setDetectFinger(int timeout) {
        return jniSetDetectFinger(timeout);
    }

    public int setSecurityLevel(int level) {
        if (mOperation != IDLE)
            return VCS_RESULT_FAILED;
        if (level == VCS_SECURITY_LEVEL_LOW || level == VCS_SECURITY_LEVEL_REGULAR || 
            level == VCS_SECURITY_LEVEL_HIGH || level == VCS_SECURITY_LEVEL_VERY_HIGH)
            return jniSetSecurityLevel(level);
        return VCS_RESULT_INVALID_ARGUMENT;
    }

    public int unProtect(byte protectedData[], VcsAddInfo addInfo, VcsByteArray unProtectedData) {
        if (protectedData == null || unProtectedData == null)
            return VCS_RESULT_INVALID_ARGUMENT;
        if (null == addInfo)
            addInfo = new VcsAddInfo();
        return jniUnprotect(protectedData, addInfo, unProtectedData);
    }
}
