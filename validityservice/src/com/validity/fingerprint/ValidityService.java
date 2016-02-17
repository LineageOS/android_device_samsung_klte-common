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

import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.LocalServerSocket;
import android.net.LocalSocket;
import android.os.Bundle;
import android.os.IBinder;
import android.util.Log;

import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.OutputStreamWriter;

public class ValidityService extends Service implements FingerprintCore.EventListener {
    public static final String SOCKET_NAME = "validityservice";
    public static final String SOCKET_NAME_CB = "validityservice_callback";
    private LocalServerSocket mServerSocket = null;
    private LocalServerSocket mServerSocketCB = null;
    private LocalSocket mSocketCB = null;

    public static final int CALL_BASE = 0;

    public static final int CALL_INITSERVICE = CALL_BASE + 1;
    public static final int CALL_ENROLL = CALL_BASE + 2; // userId ,fingerIndex
    public static final int CALL_CANCEL = CALL_BASE + 3;
    public static final int CALL_REMOVE = CALL_BASE + 4; // userId ,fingerIndex
    public static final int CALL_IDENTIFY = CALL_BASE + 5; // userId
    public static final int CALL_GET_ENROLLED_FINGER_LIST = CALL_BASE + 6; // userId
    public static final int CALL_CLEANUP = CALL_BASE + 7;

    public static final int CB_ERROR = 1;
    public static final int CB_ENROLL = 2;
    public static final int CB_REMOVED = 3;
    public static final int CB_ACQUIRED = 4;
    public static final int CB_AUTHENTICATED = 5;

    private boolean mIsIdentify = false;
    private int mLastEnrollFingerindex = 0;
    private IdentifyResult mIdresult = null;
    private boolean mEnrollBad = false;
    private int mEnrollRepeatCount = 0;
    private int mActiveGid = 0;
    private boolean mIsNeedIdentify = false;
    private int mIdentifyImage = 0;

    Fingerprint fp = new Fingerprint(this, this);

    /** We should set a password after we enroll one finger. This password is "123456q"
     *  The password hash method is SHA-1.
     */
    public static final byte[] pwdhash = "bfff2dd4f1b310eb0dbf593bd83f94dd8d34077e".getBytes();

    public int initService() {
        int ret = fp.getSensorStatus();
        VLog.i("init: ret=" + ret);
        VLog.i("init: version=" + fp.getVersion());
        notify_start();
        if (ret != 0 && ret != 1004) return -1;
        return 0;
    }

    public void notify_start() {
        fp.cancel();
        fp.notify(Fingerprint.VCS_NOTIFY_ENROLL_BEGIN, null);
    }

    public void notify_end() {
        fp.cancel();
        fp.notify(Fingerprint.VCS_NOTIFY_ENROLL_END, null);
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public void onCreate() {
        super.onCreate();
        VLog.v("onCreate");
        try {
            mServerSocket = new LocalServerSocket(SOCKET_NAME);
            mServerSocketCB = new LocalServerSocket(SOCKET_NAME_CB);
        } catch (IOException e) {
            VLog.v("in onCreate, making server socket: " + e);
            return;
        }
        Thread t_server = new Thread() {
            @Override
            public void run() {
                LocalSocket socket = null;
                while (true) {
                    try {
                        VLog.v("Waiting for connection...");
                        socket = mServerSocket.accept();
                        VLog.v(".....Got socket: " + socket);
                        if (socket != null) {
                            startService(socket);
                        } else {
                            return;
                        }
                    } catch (IOException e) {
                        VLog.v("in accept: " + e);
                    }
                }
            }
        };
        Thread t_server_cb = new Thread() {
            @Override
            public void run() {
                while (true) {
                    try {
                        VLog.v("Waiting for connection...");
                        mSocketCB = mServerSocketCB.accept();
                        VLog.v(".....Got socket: " + mSocketCB);
                        if (mSocketCB == null) {
                            return;
                        }
                    } catch (IOException e) {
                        VLog.v("in accept: " + e);
                    }
                }
            }
        };
        t_server.start();
        t_server_cb.start();

        ScreenReceiver receiver = new ScreenReceiver();
        IntentFilter filter = new IntentFilter();
        filter.addAction(Intent.ACTION_SCREEN_ON);
        filter.addAction(Intent.ACTION_SCREEN_OFF);
        registerReceiver(receiver, filter);
    }

    private void startService(final LocalSocket socket) {
        Thread t = new Thread() {
            @Override
            public void run() {
                try {
                    InputStream is = socket.getInputStream();
                    OutputStream os = socket.getOutputStream();
                    InputStreamReader isr = new InputStreamReader(is);
                    OutputStreamWriter osr = new OutputStreamWriter(os);
                    while (true) {
                        byte[] data = new byte[128];
                        int count = is.read(data);
                        int ret = -1;
                        String userId;
                        int fingerIndex;
                        for (int i = 0;i < count;i++){
                            VLog.d("data["+i+"]="+data[i]);
                        }
                        switch (data[0]) {
                            case CALL_INITSERVICE:
                                ret = initService();
                                break;
                            case CALL_ENROLL:
                                userId = "User_" + data[1];
                                fingerIndex = data[2];
                                mLastEnrollFingerindex = fingerIndex;
                                mEnrollBad = false;
                                mEnrollRepeatCount = 8;
                                fp.verifyPassword(userId, pwdhash);
                                ret = fp.enroll(userId, "", fingerIndex, Fingerprint.VCS_ENROLL_MODE_DEFAULT);
                                break;
                            case CALL_CANCEL:
                                ret = fp.cancel();
                                break;
                            case CALL_REMOVE:
                                userId = "User_" + data[1];
                                fingerIndex = data[2];
                                ret = fp.removeEnrolledFinger(userId, fingerIndex);
                                if (fingerIndex == VcsEvents.FINGER_INDEX_ALL) {
                                    notify_end();
                                    notify_start();
                                }
                                break;
                            case CALL_IDENTIFY:
                                userId = "User_" + data[1];
                                mIsIdentify = true;
                                fp.setSecurityLevel(VcsEvents.VCS_SECURITY_LEVEL_HIGH);
                                mIdresult = null;
                                mActiveGid = data[1];
                                ret = fp.identify(userId);
                                break;
                            case CALL_GET_ENROLLED_FINGER_LIST:
                                userId = "User_" + data[1];
                                VcsInt fingermask = new VcsInt();
                                fp.getEnrolledFingerList(userId, fingermask);
                                ret = fingermask.num;
                                break;
                            case CALL_CLEANUP:
                                notify_end();
                                ret = fp.cleanUp();
                                break;
                            default:
                                VLog.e("unknown function:" + data[0]);
                        }
                        String str = ret + "";
                        osr.write(str);
                        osr.flush();
                    }
                } catch (IOException e) {
                    VLog.e("in startService loop: " + e.getMessage());
                    try {
                        socket.close();
                    } catch (IOException es) {
                        VLog.e("Cannot close socket: " + es.getMessage());
                    }
                    return;
                }
            }
        };
        t.start();
    }

    public int convertImageQuality(int imageQuality) {
        switch (imageQuality) {
            case VcsEvents.VCS_IMAGE_QUALITY_GOOD:
                return 0;  //  FINGERPRINT_ACQUIRED_GOOD
            case VcsEvents.VCS_IMAGE_QUALITY_REVERSE_MOTION:
            case VcsEvents.VCS_IMAGE_QUALITY_TOO_SHORT:
            case VcsEvents.VCS_IMAGE_QUALITY_WET_FINGER:
                return 1;  //  FINGERPRINT_ACQUIRED_PARTIAL
            case VcsEvents.VCS_IMAGE_QUALITY_STICTION:
            case VcsEvents.VCS_IMAGE_QUALITY_SOMETHING_ON_THE_SENSOR:
                return 3;  //  FINGERPRINT_ACQUIRED_IMAGER_DIRTY
            case VcsEvents.VCS_IMAGE_QUALITY_TOO_SLOW:
                return 4;  //  FINGERPRINT_ACQUIRED_TOO_SLOW
            case VcsEvents.VCS_IMAGE_QUALITY_TOO_FAST:
                return 5;  //  FINGERPRINT_ACQUIRED_TOO_FAST
            default:
                VLog.d("imageQuality="+imageQuality);
                return 2;  //  FINGERPRINT_ACQUIRED_INSUFFICIENT
        }
    }

    /** Get which finger we detected. */
    public int getIdentifyFid() {
        if (mIdresult == null) return 1;
        return mIdresult.fingerIndex;
    }

    public void onEvent(final FingerprintEvent event) {
        VLog.v("identify onEvent: receive event :" + event.eventId);
        OutputStreamWriter osr = null;
        try {
            OutputStream os = mSocketCB.getOutputStream();
            osr = new OutputStreamWriter(os);
            String str = null;
            switch (event.eventId) {
                case VcsEvents.VCS_EVT_EIV_FINGERPRINT_CAPTURE_REDUNDANT:
                     str = CB_ACQUIRED + ":" + 1;
                     mEnrollBad = true;
                     break;
                case VcsEvents.VCS_EVT_ENROLL_CAPTURE_STATUS:
                     EnrollCaptureStatus data_status = (EnrollCaptureStatus)event.eventData;
                     if (mEnrollBad) mEnrollBad = false;
                     else mEnrollRepeatCount = mEnrollRepeatCount - 1;
                     if (mEnrollRepeatCount != 0) {
                         str = CB_ENROLL + ":" + mLastEnrollFingerindex + ":" + mEnrollRepeatCount;
                     }
                     break;
                case VcsEvents.VCS_EVT_EIV_FINGERPRINT_CAPTURED_BAD:
                     str = CB_ACQUIRED + ":" + convertImageQuality((int)event.eventData);
                     mEnrollBad = true;
                     break;
                case VcsEvents.VCS_EVT_ENROLL_SUCCESS:
                     str = CB_ENROLL + ":" + mLastEnrollFingerindex + ":" + 0;
                     fp.setPassword("User_0", pwdhash);
                     break;
                case VcsEvents.VCS_EVT_ENROLL_FAILED:
                     str = CB_ERROR + ":" + 2; //FINGERPRINT_ERROR_UNABLE_TO_PROCESS
                     VLog.e("enroll onEvent: enroll error, result=" + (int)event.eventData);
                     break;
                case VcsEvents.VCS_EVT_VERIFY_COMPLETED:
                case VcsEvents.VCS_EVT_IDENTIFY_COMPLETED:
                     mIdresult = (IdentifyResult)event.eventData;
                     break;
                case VcsEvents.VCS_EVT_EIV_FINGERPRINT_CAPTURED:
                     FingerprintBitmap data_map = (FingerprintBitmap)event.eventData;
                     mIdentifyImage = convertImageQuality(data_map.quality);
                     break;
                case VcsEvents.VCS_EVT_VERIFY_SUCCESS:
                case VcsEvents.VCS_EVT_IDENTIFY_SUCCESS:
                     mIsIdentify = false;
                     str = CB_AUTHENTICATED + ":" + getIdentifyFid();
                     break;
                case VcsEvents.VCS_EVT_SENSOR_REMOVED:
                     mIsIdentify = false;
                     str = CB_ERROR + ":" + 1; //FINGERPRINT_ERROR_HW_UNAVAILABLE
                     VLog.e("identify onEvent: identify error, result=" + (int)event.eventData);
                     break;
                case VcsEvents.VCS_EVT_VERIFY_FAILED:
                case VcsEvents.VCS_EVT_IDENTIFY_FAILED:
                     mIsIdentify = false;
                     VLog.e("identify onEvent: identify error, result=" + (int)event.eventData);
                     switch ((int)event.eventData) {
                         case VcsEvents.VCS_RESULT_BAD_QUALITY_IMAGE:
                              str = CB_ACQUIRED + ":" + mIdentifyImage;
                         case VcsEvents.VCS_RESULT_USER_DOESNT_EXIST:
                              fp.setSecurityLevel(VcsEvents.VCS_SECURITY_LEVEL_HIGH);
                              mIdresult = null;
                              mIsIdentify = true;
                              fp.identify("User_" + mActiveGid);
                              if (str == null) {
                                  str = CB_ACQUIRED + ":" + 1; //  FINGERPRINT_ACQUIRED_PARTIAL
                              }
                              break;
                         case VcsEvents.VCS_RESULT_OPERATION_CANCELED:
                              break;
                         default:
                              str = CB_ERROR + ":" + 2; //FINGERPRINT_ERROR_UNABLE_TO_PROCESS
                     }
                     break;
                default:
                     VLog.v("identify onEvent: No need to process event :" + event.eventId);
            }
            if (str != null) {
                osr.write(str);
                osr.flush();
            }
        } catch (IOException e) {
            VLog.e("in onEvent: " + e.getMessage());
            try {
                osr.close();
                mSocketCB.close();
            } catch (IOException es) {
                VLog.e("Cannot close socket: " + es.getMessage());
            }
            return;
        }
    }

    /** Our Keyguard will not call identify when turn on screen, so we need call it. */
    public void onScreenOn() {
        if (mIsNeedIdentify) {
            mIsNeedIdentify = false;
            fp.setSecurityLevel(VcsEvents.VCS_SECURITY_LEVEL_HIGH);
            mIdresult = null;
            mIsIdentify = true;
            fp.identify("User_" + mActiveGid);
        }
    }

    public void onScreenOff() {
        if (mIsIdentify) {
            mIsNeedIdentify = true;
        }
    }

    public class ScreenReceiver extends BroadcastReceiver {

        @Override
        public void onReceive(Context context, Intent intent) {
            if (intent.getAction().equals(Intent.ACTION_SCREEN_ON)) {
                onScreenOn();
            }
            else if (intent.getAction().equals(Intent.ACTION_SCREEN_OFF)) {
                onScreenOff();
            }
            else VLog.e("Unknown intent:" + intent.getAction());
        }
    }

}
