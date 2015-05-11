/*
 * Copyright (c) 2015 The CyanogenMod Project
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

package com.cyanogenmod.settings.device;

import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.IBinder;
import android.os.PowerManager;
import android.preference.PreferenceManager;
import android.provider.Settings;
import android.util.Log;

import java.util.ArrayList;
import java.util.List;

public class KlteDozeService extends Service {
    private static final String TAG = "KlteDozeService";
    private static final boolean DEBUG = false;

    private static final String DOZE_INTENT = "com.android.systemui.doze.pulse";

    protected static final String GESTURE_HAND_WAVE_KEY = "gesture_hand_wave";

    private Context mContext;
    SensorManager mSensorManager;
    private List<IKlteSensor> mSensors;

    private boolean mHandwaveGestureEnabled = false;

    @Override
    public void onCreate() {
        Log.d(TAG, "KlteDozeService Started");
        mContext = this;
        mSensors = new ArrayList<IKlteSensor>();
        mSensorManager = (SensorManager)mContext.getSystemService(Context.SENSOR_SERVICE);
        IKlteSensor ProxSensor = new KlteProximitySensor();
        mSensors.add(ProxSensor);
        SharedPreferences sharedPrefs = PreferenceManager.getDefaultSharedPreferences(mContext);
        loadPreferences(sharedPrefs);
        sharedPrefs.registerOnSharedPreferenceChangeListener(mPrefListener);
        if (!isInteractive() && isDozeEnabled()) {
            for (IKlteSensor sensor : mSensors) {
                sensor.enable();
            }
        }
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        if (DEBUG) Log.d(TAG, "Starting service");
        IntentFilter screenStateFilter = new IntentFilter(Intent.ACTION_SCREEN_ON);
        screenStateFilter.addAction(Intent.ACTION_SCREEN_OFF);
        mContext.registerReceiver(mScreenStateReceiver, screenStateFilter);
        return START_STICKY;
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    protected static void launchDozePulse(Context context) {
        context.sendBroadcast(new Intent(DOZE_INTENT));
    }

    private boolean isInteractive() {
        PowerManager pm = (PowerManager) getSystemService(Context.POWER_SERVICE);
        return pm.isInteractive();
    }

    private boolean isDozeEnabled() {
        return Settings.Secure.getInt(mContext.getContentResolver(),
                Settings.Secure.DOZE_ENABLED, 1) != 0;
    }

    private void loadPreferences(SharedPreferences sharedPreferences) {
        mHandwaveGestureEnabled = sharedPreferences.getBoolean(GESTURE_HAND_WAVE_KEY, true);
    }

    private SharedPreferences.OnSharedPreferenceChangeListener mPrefListener =
            new SharedPreferences.OnSharedPreferenceChangeListener() {
        @Override
        public void onSharedPreferenceChanged(SharedPreferences sharedPreferences, String key) {
            if (GESTURE_HAND_WAVE_KEY.equals(key)) {
                mHandwaveGestureEnabled = sharedPreferences.getBoolean(GESTURE_HAND_WAVE_KEY, true);
            }
        }
    };

    private void onDisplayOn() {
        if (DEBUG) Log.d(TAG, "Display on");
        for (IKlteSensor sensor : mSensors) {
            sensor.disable();
        }
    }

    private void onDisplayOff() {
        if (DEBUG) Log.d(TAG, "Display off");
        if (isDozeEnabled()) {
            for (IKlteSensor sensor : mSensors) {
                sensor.enable();
            }
        }
    }

    private BroadcastReceiver mScreenStateReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (intent.getAction().equals(Intent.ACTION_SCREEN_OFF)) {
                onDisplayOff();
            } else if (intent.getAction().equals(Intent.ACTION_SCREEN_ON)) {
                onDisplayOn();
            }
        }
    };

    public class KlteProximitySensor implements IKlteSensor, SensorEventListener {
        private Sensor mSensor;
        private boolean mSawNear = false;
        private float mMaxRange;

        public KlteProximitySensor() {
            mSensor = mSensorManager.getDefaultSensor(Sensor.TYPE_PROXIMITY);
            mMaxRange = mSensor.getMaximumRange();
        }

        @Override
        public void onSensorChanged(SensorEvent event) {
            boolean isNear = event.values[0] < mMaxRange;
            if (mSawNear && !isNear) {
                KlteDozeService.launchDozePulse(mContext);
            } else {
            }
            mSawNear = isNear;
        }

        @Override
        public void onAccuracyChanged(Sensor sensor, int accuracy) {
            /* Empty */
        }

        public void enable() {
            if (mHandwaveGestureEnabled) {
                mSensorManager.registerListener(this, mSensor, SensorManager.SENSOR_DELAY_NORMAL);
            }
        }

        public void disable() {
            mSensorManager.unregisterListener(this, mSensor);
        }
    }
}
