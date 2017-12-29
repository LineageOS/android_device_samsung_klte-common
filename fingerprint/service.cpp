/*
 * Copyright (C) 2017 The Android Open Source Project
 * Copyright (C) 2017 The LineageOS Project
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

 #define LOG_TAG "android.hardware.biometrics.fingerprint@2.1-service"

 #include <android/log.h>
 #include <hidl/HidlSupport.h>
 #include <hidl/HidlTransportSupport.h>
 #include <android/hardware/biometrics/fingerprint/2.1/IBiometricsFingerprint.h>
 #include <android/hardware/biometrics/fingerprint/2.1/types.h>
 #include "BiometricsFingerprint.h"

 using android::hardware::biometrics::fingerprint::V2_1::IBiometricsFingerprint;
 using android::hardware::biometrics::fingerprint::V2_1::implementation::BiometricsFingerprint;
 using android::hardware::configureRpcThreadpool;
 using android::hardware::joinRpcThreadpool;
 using android::OK;
 using android::sp;
 using android::status_t;

int main() {

    status_t status;
    android::sp<IBiometricsFingerprint> service = nullptr;

    ALOGI("Fingerprint HAL Service 1.1 for KLTE is starting.");

    service = new BiometricsFingerprint();
    if (service == nullptr) {
        ALOGE("Can not create an instance of Fingerprint HAL Iface, exiting.");

        goto shutdown;
    }

    configureRpcThreadpool(1, true /*callerWillJoin*/);

    status = service->registerAsService();
    if (status != OK) {
        ALOGE("Could not register service for Fingerprint HAL Iface (%d).", status);
        goto shutdown;
    }

    ALOGI("Fingerprint Service is ready");
    joinRpcThreadpool();
    //Should not pass this line

shutdown:
    // In normal operation, we don't expect the thread pool to exit

    ALOGE("Fingerprint Service is shutting down");
    return 1;
}
