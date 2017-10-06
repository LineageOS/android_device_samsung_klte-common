/*
 * Copyright (C) 2015 The CyanogenMod Project
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

#ifndef ANDROID_HARDWARE_CAMERA_PARAMETERS_EXT_H
#define ANDROID_HARDWARE_CAMERA_PARAMETERS_EXT_H

#include <utils/KeyedVector.h>

namespace android {

class CameraParameters;
struct Size;

class CameraParameters_EXT
{
public:
    CameraParameters_EXT();
    CameraParameters_EXT(CameraParameters *p);
    ~CameraParameters_EXT();

    int get_from_attr(const char *path, char *buf, size_t len);
    bool check_flashlight_restriction();
    int lookupAttr(/* CameraParameters_EXT::CameraMap const* */
            void *cameraMap, int a3, const char *a4);

    const char *getPreviewFrameRateMode() const;
    void setPreviewFrameRateMode(const char *mode);

    void setBrightnessLumaTargetSet(int brightness, int luma);
    int getBrightnessLumaTargetSet(int *brightness, int *luma) const;

    void setTouchIndexAec(int x, int y);
    void getTouchIndexAec(int *x, int *y);

    void setTouchIndexAf(int x, int y);
    void getTouchIndexAf(int *x, int *y);

    void setZsl(const char *mode);
    const char *getZsl() const;

    void setRawSize(int x, int y);
    void getRawSize(int *x, int *y);

    void getMeteringAreaCenter(int *x, int *y) const;
    void getSupportedHfrSizes(Vector<Size> &sizes) const;
    void setPreviewFpsRange(int min, int max);
    int getOrientation() const;
    void setOrientation(int orientation);

    static const char SCENE_MODE_BURST[];
    static const char SCENE_MODE_MANUAL[];
    static const char SCENE_MODE_PANORAMA[];
    static const char SCENE_MODE_PANORAMA_360[];
    static const char SCENE_MODE_TEXT[];
    static const char SCENE_MODE_ZOE[];
    static const char ISO_AUTO[];
    static const char ISO_HJR[];
    static const char ISO_100[];
    static const char ISO_200[];
    static const char ISO_400[];
    static const char ISO_800[];
    static const char ISO_1600[];
    static const char VIDEO_HFR_OFF[];
    static const char VIDEO_HFR_2X[];
    static const char VIDEO_HFR_3X[];
    static const char VIDEO_HFR_4X[];
    static const char VIDEO_HFR_5X[];
    static const char KEY_FASTVIDEO_FPS60_1080P_SUPPORTED[];
    static const char KEY_SLOW_MOTION_SUPPORTED[];
    static const char KEY_SLOW_MOTION_MULTIPLE[];
    static const char KEY_SLOW_MOTION_RES[];
    static const char KEY_FASTVIDEO_FPS60_SUPPORTED[];
    static const char KEY_CONTIBURST_TAKE[];
    static const char KEY_CONTIBURST_SUPPORTED_MODE[];
    static const char KEY_NON_ZSL_MANUAL_MODE[];
    static const char KEY_VIDEO_MODE[];
    static const char KEY_FORCE_USE_AUDIO_ENABLED[];
    static const char KEY_SLOW_MOTION_VERSION[];
    static const char DENOISE_ON[];
    static const char DENOISE_OFF[];

private:
    CameraParameters *mParams;
};

}

#endif
