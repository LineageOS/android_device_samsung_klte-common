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

import java.util.*;

public class VcsTemplateIds {
    public Map<Integer, byte[]> fingerIndexTemplateIdMap = new HashMap();

    public byte[] getTemplateId(int fingerIndex) {
        Iterator<Map.Entry< Integer, byte[]>> iter = fingerIndexTemplateIdMap.entrySet().iterator();

        while(iter.hasNext()) {
            Map.Entry< Integer, byte[]> mEntry = (Map.Entry< Integer, byte[]>)iter.next();
            if (((Integer)mEntry.getKey()).intValue() == fingerIndex)
                return mEntry.getValue();
        }
        return null;
    }

    public Map<Integer, byte[]> getTemplateIds() {
        return fingerIndexTemplateIdMap;
    }

    public boolean setTemplateId(int fingerIndex, byte templateId[]) {
        fingerIndexTemplateIdMap.put(fingerIndex, templateId);
        return true;
    }

}
