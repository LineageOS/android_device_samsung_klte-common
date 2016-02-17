/*
 * Copyright (C) 2016 The Android Open Source Project
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

import android.util.Log;

public class VLog {
    private static final String TAG = "ValidityService";
    private static final boolean DEBUG = false;

    public static void i(final String message) {
        Log.i(TAG, message);
    }

    public static void d(final String message) {
        if (DEBUG) {
            Log.d(TAG, message);
        }
    }

    public static void v(final String message) {
        if (DEBUG) {
            Log.v(TAG, message);
        }
    }

    public static void e(final String message, final Throwable t) {
        Log.e(TAG, message, t);
    }

    public static void e(final String message) {
        Log.e(TAG, message);
    }

    public static void w(final String message, final Throwable t) {
        Log.w(TAG, message, t);
    }

    public static void w(final String message) {
        Log.w(TAG, message);
    }
}
