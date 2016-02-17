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

import android.app.Application;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.util.Log;
import android.net.LocalServerSocket;
import android.net.LocalSocket;

import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.OutputStreamWriter;

public class ValidityService extends Application implements FingerprintCore.EventListener {
    public static final String SOCKET_NAME = "validityservice";
    public static final String SOCKET_NAME_CB = "validityservice_callback";
    private LocalServerSocket mServerSocket = null;
    private LocalServerSocket mServerSocket_cb = null;
    private LocalSocket socket_cb = null;

    private String status = "";
    private int last_enroll_fingerindex = 0;
    private IdentifyResult idresult = null;
    private boolean enroll_bad = false;
    private int enroll_repeat_count = 0;
    private int active_gid = 0;
    private boolean isneedIdentify = false;

    Fingerprint fp = new Fingerprint(this, this);

    public static final int CALL_base = 0;

    public static final int CALL_initService = CALL_base + 1;
    public static final int CALL_enroll = CALL_base + 2; // userId ,fingerIndex
    public static final int CALL_cancel = CALL_base + 3;
    public static final int CALL_remove = CALL_base + 4; // userId ,fingerIndex
    public static final int CALL_identify = CALL_base + 5; // userId
    public static final int CALL_getEnrolledFingerList = CALL_base + 6; // userId
    public static final int CALL_cleanup = CALL_base + 7;

    public static final int CB_error = 1;
    public static final int CB_enroll = 2;
    public static final int CB_removed = 3;
    public static final int CB_acquired = 4;
    public static final int CB_authenticated = 5;

    // We should set a password after we enroll one finger. This password is "123456q"
    // The password hash method is SHA-1.
    public static final byte[] pwdhash = "bfff2dd4f1b310eb0dbf593bd83f94dd8d34077e".getBytes();

    public int initService() {
        int ret = fp.getSensorStatus();
        VLog.i("init: ret=" + ret);
        VLog.i("init: version=" + fp.getVersion());
        notify_start();
        if (ret != 0 && ret != 1004)
            return -1;
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
    public void onCreate() {
        super.onCreate();
        VLog.v("onCreate");
        try {
            mServerSocket = new LocalServerSocket(SOCKET_NAME);
            mServerSocket_cb = new LocalServerSocket(SOCKET_NAME_CB);
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
                        socket_cb = mServerSocket_cb.accept();
                        VLog.v(".....Got socket: " + socket_cb);
                        if (socket_cb == null) {
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
                            case CALL_initService:
                                ret = initService();
                                break;
                            case CALL_enroll:
                                userId = "User_" + data[1];
                                fingerIndex = data[2];
                                last_enroll_fingerindex = fingerIndex;
                                enroll_bad = false;
                                enroll_repeat_count = 8;
                                fp.verifyPassword(userId, pwdhash);
                                ret = fp.enroll(userId, "", fingerIndex, Fingerprint.VCS_ENROLL_MODE_DEFAULT);
                                break;
                            case CALL_cancel:
                                ret = fp.cancel();
                                break;
                            case CALL_remove:
                                userId = "User_" + data[1];
                                fingerIndex = data[2];
                                ret = fp.removeEnrolledFinger(userId, fingerIndex);
                                if (fingerIndex == VcsEvents.FINGER_INDEX_ALL) {
                                    notify_end();
                                    notify_start();
                                }
                                break;
                            case CALL_identify:
                                userId = "User_" + data[1];
                                status = "identify";
                                fp.setSecurityLevel(VcsEvents.VCS_SECURITY_LEVEL_HIGH);
                                idresult = null;
                                active_gid = data[1];
                                ret = fp.identify(userId);
                                break;
                            case CALL_getEnrolledFingerList:
                                userId = "User_" + data[1];
                                VcsInt fingermask = new VcsInt();
                                fp.getEnrolledFingerList(userId, fingermask);
                                ret = fingermask.num;
                                break;
                            case CALL_cleanup:
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
                    VLog.v("in startService loop: " + e.getMessage());
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

    public int getIdentifyFid() {
        if (idresult == null)
           return 1;
        return idresult.fingerIndex;
    }

    public void onEvent(final FingerprintEvent event) {
        VLog.v("identify onEvent: receive event :" + event.eventId);
        try {
            OutputStream os = socket_cb.getOutputStream();
            OutputStreamWriter osr = new OutputStreamWriter(os);
            String str = "";
            switch (event.eventId) {
                case VcsEvents.VCS_EVT_EIV_FINGERPRINT_CAPTURE_REDUNDANT:
                     str = CB_acquired + ":" + 1;
                     enroll_bad = true;
                     break;
                case VcsEvents.VCS_EVT_ENROLL_CAPTURE_STATUS:
                     EnrollCaptureStatus data_status = (EnrollCaptureStatus)event.eventData;
                     if (enroll_bad)
                         enroll_bad = false;
                     else enroll_repeat_count = enroll_repeat_count - 1;
                     if (enroll_repeat_count != 0) {
                         str = CB_enroll + ":" + last_enroll_fingerindex + ":" + enroll_repeat_count;
                     }
                     break;
                case VcsEvents.VCS_EVT_EIV_FINGERPRINT_CAPTURED_BAD:
                     str = CB_acquired + ":" + convertImageQuality((int)event.eventData);
                     enroll_bad = true;
                     break;
                case VcsEvents.VCS_EVT_ENROLL_SUCCESS:
                     str = CB_enroll + ":" + last_enroll_fingerindex + ":" + 0;
                     fp.setPassword("User_0", pwdhash);
                     break;
                case VcsEvents.VCS_EVT_ENROLL_FAILED:
                     str = CB_error + ":" + 2; //FINGERPRINT_ERROR_UNABLE_TO_PROCESS
                     VLog.e("enroll onEvent: enroll error, result=" + (int)event.eventData);
                     break;
                case VcsEvents.VCS_EVT_VERIFY_COMPLETED:
                case VcsEvents.VCS_EVT_IDENTIFY_COMPLETED:
                     idresult = (IdentifyResult)event.eventData;
                     break;
                case VcsEvents.VCS_EVT_EIV_FINGERPRINT_CAPTURED:
                     FingerprintBitmap data_map = (FingerprintBitmap)event.eventData;
                     str = CB_acquired + ":" + convertImageQuality(data_map.quality);
                     break;
                case VcsEvents.VCS_EVT_VERIFY_SUCCESS:
                case VcsEvents.VCS_EVT_IDENTIFY_SUCCESS:
                     status = "";
                     str = CB_authenticated + ":" + getIdentifyFid();
                     break;
                case VcsEvents.VCS_EVT_SENSOR_REMOVED:
                     status = "";
                     str = CB_error + ":" + 1; //FINGERPRINT_ERROR_HW_UNAVAILABLE
                     VLog.e("identify onEvent: identify error, result=" + (int)event.eventData);
                     break;
                case VcsEvents.VCS_EVT_VERIFY_FAILED:
                case VcsEvents.VCS_EVT_IDENTIFY_FAILED:
                     status = "";
                     VLog.e("identify onEvent: identify error, result=" + (int)event.eventData);
                     switch ((int)event.eventData) {
                         case VcsEvents.VCS_RESULT_BAD_QUALITY_IMAGE:
                         case VcsEvents.VCS_RESULT_USER_DOESNT_EXIST:
                              fp.setSecurityLevel(VcsEvents.VCS_SECURITY_LEVEL_HIGH);
                              idresult = null;
                              status = "identify";
                              fp.identify("User_" + active_gid);
                              break;
                         case VcsEvents.VCS_RESULT_OPERATION_CANCELED:
                              break;
                         default:
                              str = CB_error + ":" + 2; //FINGERPRINT_ERROR_UNABLE_TO_PROCESS
                     }
                     break;
                default:
                     VLog.v("identify onEvent: No need to process event :" + event.eventId);
            }
            if (str.equals("") == false) {
                osr.write(str);
                osr.flush();
            }
        } catch (IOException e) {
            VLog.d("in onEvent: " + e.getMessage());
        }
    }

    public void onScreenOn() {
        if (isneedIdentify) {
            isneedIdentify = false;
            fp.setSecurityLevel(VcsEvents.VCS_SECURITY_LEVEL_HIGH);
            idresult = null;
            status = "identify";
            fp.identify("User_" + active_gid);
        }
    }

    public void onScreenOff() {
        if (status.equals("identify"))
            isneedIdentify = true;
    }

    public class ScreenReceiver extends BroadcastReceiver {

        @Override
        public void onReceive(Context context, Intent intent) {
            if (intent.getAction().equals(Intent.ACTION_SCREEN_ON))
                onScreenOn();
            else if (intent.getAction().equals(Intent.ACTION_SCREEN_OFF))
                onScreenOff();
            else VLog.e("Unknown intent:" + intent.getAction());
        }
    }

}
