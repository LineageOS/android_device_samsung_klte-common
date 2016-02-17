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

public class IdentifyResult {
    public byte templateId[] = new byte[16];
    public int matchedFingerIndexes[] = new int[20];
    public int corrMinuCount[] = new int[4];
    public int vMinuCount[] = new int[4];
    public String appData;
    public int fMinuCount;
    public int fingerIndex;
    public float matchRate;
    public int matchScore;
    public int templateUpdated;
    public String userId;
}
