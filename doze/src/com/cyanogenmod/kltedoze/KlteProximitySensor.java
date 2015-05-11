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

package com.cyanogenmod.kltedoze;

import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;

public class KlteProximitySensor implements IKlteSensor, SensorEventListener {
    private SensorManager mSensorManager;
    private Sensor mSensor;
    private boolean mSawNear = false;
    private float mMaxRange;

    public KlteProximitySensor(SensorManager sensorManager) {
        mSensorManager = sensorManager;
        mSensor = mSensorManager.getDefaultSensor(Sensor.TYPE_PROXIMITY);
        mMaxRange = mSensor.getMaximumRange();
    }

    @Override
    public void onSensorChanged(SensorEvent event) {
        boolean isNear = event.values[0] < mMaxRange;
        if (mSawNear && !isNear) {
            KlteDozeService.launchDozePulse();
        } else {
        }
        mSawNear = isNear;
    }

    @Override
    public void onAccuracyChanged(Sensor sensor, int accuracy) {
        /* Empty */
    }

    public void enable() {
        mSensorManager.registerListener(this, mSensor, SensorManager.SENSOR_DELAY_NORMAL);
    }

    public void disable() {
        mSensorManager.unregisterListener(this, mSensor);
    }
}
