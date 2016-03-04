/*
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

package org.cyanogenmod.hardware;

import org.cyanogenmod.hardware.util.FileUtils;

import java.io.File;

public class VibratorHW {

    private static String LEVEL_PATH = "/sys/class/timed_output/vibrator/intensity";

    public static boolean isSupported() {
        File f = new File(LEVEL_PATH);
        return f.exists();
    }

    public static int getMaxIntensity()  {
        return 10000;
    }

    public static int getMinIntensity()  {
        return 0;
    }

    public static int getWarningThreshold()  {
        return 9000;
    }

    public static int getCurIntensity()  {
        File f = new File(LEVEL_PATH);
        String actualIntensity = FileUtils.readOneLine(LEVEL_PATH).replace("intensity: ", "");

        return f.exists() ? Integer.parseInt(actualIntensity) : 0;
    }

    public static int getDefaultIntensity()  {
        return 7500;
    }

    public static boolean setIntensity(int intensity)  {
        File f = new File(LEVEL_PATH);

        return f.exists() && FileUtils.writeLine(LEVEL_PATH, String.valueOf(intensity));
    }
}
