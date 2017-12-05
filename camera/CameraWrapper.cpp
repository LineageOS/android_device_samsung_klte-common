/*
 * Copyright (C) 2012-2016, The CyanogenMod Project
 * Copyright (C) 2017, The LineageOS Project
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

/**
* @file CameraWrapper.cpp
*
* This file wraps a vendor camera module.
*
*/

//#define LOG_NDEBUG 0

#define LOG_TAG "CameraWrapper"
#include <cutils/log.h>

#include <utils/threads.h>
#include <utils/String8.h>
#include <hardware/hardware.h>
#include <hardware/camera.h>
#include <camera/Camera.h>
#include <camera/CameraParameters.h>

#define REAR_CAMERA_ID 0
#define FRONT_CAMERA_ID 1

using namespace android;

static Mutex gCameraWrapperLock;
static camera_module_t *gVendorModule = 0;

static char **fixed_set_params = NULL;

static int camera_device_open(const hw_module_t *module, const char *name,
        hw_device_t **device);
static int camera_get_number_of_cameras(void);
static int camera_get_camera_info(int camera_id, struct camera_info *info);

static struct hw_module_methods_t camera_module_methods = {
    .open = camera_device_open,
};

camera_module_t HAL_MODULE_INFO_SYM = {
    .common = {
        .tag = HARDWARE_MODULE_TAG,
        .module_api_version = CAMERA_MODULE_API_VERSION_1_0,
        .hal_api_version = HARDWARE_HAL_API_VERSION,
        .id = CAMERA_HARDWARE_MODULE_ID,
        .name = "KLTE Camera Wrapper",
        .author = "The LineageOS Project",
        .methods = &camera_module_methods,
        .dso = NULL, /* remove compilation warnings */
        .reserved = {0}, /* remove compilation warnings */
    },
    .get_number_of_cameras = camera_get_number_of_cameras,
    .get_camera_info = camera_get_camera_info,
    .set_callbacks = NULL, /* remove compilation warnings */
    .get_vendor_tag_ops = NULL, /* remove compilation warnings */
    .open_legacy = NULL, /* remove compilation warnings */
    .set_torch_mode = NULL, /* remove compilation warnings */
    .init = NULL, /* remove compilation warnings */
    .reserved = {0}, /* remove compilation warnings */
};

typedef struct wrapper_camera_device {
    camera_device_t base;
    int id;
    camera_device_t *vendor;
} wrapper_camera_device_t;

#define VENDOR_CALL(device, func, ...) ({ \
    wrapper_camera_device_t *__wrapper_dev = (wrapper_camera_device_t*) device; \
    __wrapper_dev->vendor->ops->func(__wrapper_dev->vendor, ##__VA_ARGS__); \
})

#define CAMERA_ID(device) (((wrapper_camera_device_t *)(device))->id)

/* Some QC camera parameters we use */
#define QC_PIXEL_FORMAT_NV12_VENUS "nv12-venus"
#define KEY_QC_ISO_MODE "iso"
#define KEY_QC_SUPPORTED_ISO_MODES "iso-values"
#define KEY_QC_SUPPORTED_VIDEO_HIGH_FRAME_RATE_MODES "video-hfr-values"

/* Some Samsung camera parameters we use */
#define KEY_DYNAMIC_RANGE_CONTROL "dynamic-range-control"
#define KEY_PHASE_AF "phase-af"
#define KEY_RT_HDR "rt-hdr"

static int check_vendor_module()
{
    int rv;
    ALOGV("%s", __func__);

    if (gVendorModule)
        return 0;

    rv = hw_get_module_by_class("camera", "vendor",
            (const hw_module_t**)&gVendorModule);
    if (rv)
        ALOGE("%s: failed to open vendor camera module", __func__);
    return rv;
}

static inline bool is_4k_video(CameraParameters &params) {
    int video_width, video_height;
    params.getVideoSize(&video_width, &video_height);
    return video_width * video_height == 3840 * 2160;
}

static char *camera_fixup_getparams(int id, const char *settings)
{
    CameraParameters params;
    params.unflatten(String8(settings));

#if !LOG_NDEBUG
    ALOGV("%s: original parameters:", __func__);
    params.dump();
#endif

    /* Hide nv12-venus from Android */
    const char *previewFormat = params.get(CameraParameters::KEY_PREVIEW_FORMAT);
    if (previewFormat) {
        if (!strcmp(previewFormat, QC_PIXEL_FORMAT_NV12_VENUS)) {
            params.set(CameraParameters::KEY_PREVIEW_FORMAT, CameraParameters::PIXEL_FORMAT_YUV420SP);
        }
    }

    /* Hacks for rear camera */
    if (id == REAR_CAMERA_ID) {
        /* Advertise 4K video support */
        const char *videoSizeValues = params.get(CameraParameters::KEY_SUPPORTED_VIDEO_SIZES);
        if (videoSizeValues) {
            char videoSizes[strlen(videoSizeValues) + 10 + 1];
            sprintf(videoSizes, "3840x2160,%s", videoSizeValues);
            params.set(CameraParameters::KEY_SUPPORTED_VIDEO_SIZES, videoSizes);
        }

        /* Advertise that hfr can be turned off */
        const char *hfrModeValues = params.get(KEY_QC_SUPPORTED_VIDEO_HIGH_FRAME_RATE_MODES);
        if (hfrModeValues) {
            char hfrModes[strlen(hfrModeValues) + 4 + 1];
            sprintf(hfrModes, "%s,off", hfrModeValues);
            params.set(KEY_QC_SUPPORTED_VIDEO_HIGH_FRAME_RATE_MODES, hfrModes);
        }

        /* Advertise scene mode as hdr when rt-hdr is on */
        const char *rtHdr = params.get(KEY_RT_HDR);
        if (rtHdr) {
            if (!strcmp(rtHdr, "on")) {
                params.set(CameraParameters::KEY_SCENE_MODE, CameraParameters::SCENE_MODE_HDR);
            }
        }
    }

    /* Advertise video snapshot support if we're not in 4K mode */
    if (!is_4k_video(params)) {
        params.set(CameraParameters::KEY_VIDEO_SNAPSHOT_SUPPORTED, "true");
    }

    /* Properly advertise supported ISO values */
    params.set(KEY_QC_SUPPORTED_ISO_MODES,
            "auto,ISO_HJR,ISO100,ISO200,ISO400,ISO800,ISO1600");

#if !LOG_NDEBUG
    ALOGV("%s: fixed parameters:", __func__);
    params.dump();
#endif

    String8 strParams = params.flatten();
    char *ret = strdup(strParams.string());

    return ret;
}

static char *camera_fixup_setparams(int id, const char *settings)
{
    CameraParameters params;
    params.unflatten(String8(settings));

#if !LOG_NDEBUG
    ALOGV("%s: original parameters:", __func__);
    params.dump();
#endif

    /* Use nv12-venus for preview format for 4K videos */
    if (is_4k_video(params)) {
        params.setPreviewFormat(QC_PIXEL_FORMAT_NV12_VENUS);
    }

    /* Enable some Samsung features on rear camera */
    if (id == REAR_CAMERA_ID) {
        /* Use rt-hdr on rear camera in hdr mode */
        const char *sceneMode = params.get(CameraParameters::KEY_SCENE_MODE);
        if (sceneMode) {
            if (!strcmp(sceneMode, CameraParameters::SCENE_MODE_HDR)) {
                params.set(CameraParameters::KEY_SCENE_MODE, CameraParameters::SCENE_MODE_AUTO);
                params.set(KEY_RT_HDR, "on");
            } else {
                params.set(KEY_RT_HDR, "off");
            }
        }

        /* Enable phase af and dynamic range control on rear camera */
        params.set(KEY_PHASE_AF, "on");
        params.set(KEY_DYNAMIC_RANGE_CONTROL, "on");
    } else {
        params.set(KEY_PHASE_AF, "off");
        params.set(KEY_DYNAMIC_RANGE_CONTROL, "off");
    }

    /* Remap ISO values to what our HAL expects */
    const char *isoMode = params.get(KEY_QC_ISO_MODE);
    if (isoMode) {
        if (!strcmp(isoMode, "ISO100"))
            params.set(KEY_QC_ISO_MODE, "100");
        else if (!strcmp(isoMode, "ISO200"))
            params.set(KEY_QC_ISO_MODE, "200");
        else if (!strcmp(isoMode, "ISO400"))
            params.set(KEY_QC_ISO_MODE, "400");
        else if (!strcmp(isoMode, "ISO800"))
            params.set(KEY_QC_ISO_MODE, "800");
        else if (!strcmp(isoMode, "ISO1600"))
            params.set(KEY_QC_ISO_MODE, "1600");
    }

#if !LOG_NDEBUG
    ALOGV("%s: fixed parameters:", __func__);
    params.dump();
#endif

    String8 strParams = params.flatten();
    if (fixed_set_params[id])
        free(fixed_set_params[id]);
    fixed_set_params[id] = strdup(strParams.string());
    char *ret = fixed_set_params[id];

    return ret;
}

/*******************************************************************
 * implementation of camera_device_ops functions
 *******************************************************************/
static char *camera_get_parameters(struct camera_device *device);
static int camera_set_parameters(struct camera_device *device, const char *params);

static int camera_set_preview_window(struct camera_device *device,
        struct preview_stream_ops *window)
{
    if (!device)
        return -EINVAL;

    ALOGV("%s->%08X->%08X", __func__, (uintptr_t)device,
            (uintptr_t)(((wrapper_camera_device_t*)device)->vendor));

    return VENDOR_CALL(device, set_preview_window, window);
}

static void camera_set_callbacks(struct camera_device *device,
        camera_notify_callback notify_cb,
        camera_data_callback data_cb,
        camera_data_timestamp_callback data_cb_timestamp,
        camera_request_memory get_memory,
        void *user)
{
    if (!device)
        return;

    ALOGV("%s->%08X->%08X", __func__, (uintptr_t)device,
            (uintptr_t)(((wrapper_camera_device_t*)device)->vendor));

    VENDOR_CALL(device, set_callbacks, notify_cb, data_cb, data_cb_timestamp,
            get_memory, user);
}

static void camera_enable_msg_type(struct camera_device *device,
        int32_t msg_type)
{
    if (!device)
        return;

    ALOGV("%s->%08X->%08X", __func__, (uintptr_t)device,
            (uintptr_t)(((wrapper_camera_device_t*)device)->vendor));

    VENDOR_CALL(device, enable_msg_type, msg_type);
}

static void camera_disable_msg_type(struct camera_device *device,
        int32_t msg_type)
{
    if (!device)
        return;

    ALOGV("%s->%08X->%08X", __func__, (uintptr_t)device,
            (uintptr_t)(((wrapper_camera_device_t*)device)->vendor));

    VENDOR_CALL(device, disable_msg_type, msg_type);
}

static int camera_msg_type_enabled(struct camera_device *device,
        int32_t msg_type)
{
    if (!device)
        return 0;

    ALOGV("%s->%08X->%08X", __func__, (uintptr_t)device,
            (uintptr_t)(((wrapper_camera_device_t*)device)->vendor));

    return VENDOR_CALL(device, msg_type_enabled, msg_type);
}

static int camera_start_preview(struct camera_device *device)
{
    if (!device)
        return -EINVAL;

    ALOGV("%s->%08X->%08X", __func__, (uintptr_t)device,
            (uintptr_t)(((wrapper_camera_device_t*)device)->vendor));

    return VENDOR_CALL(device, start_preview);
}

static void camera_stop_preview(struct camera_device *device)
{
    if (!device)
        return;

    ALOGV("%s->%08X->%08X", __func__, (uintptr_t)device,
            (uintptr_t)(((wrapper_camera_device_t*)device)->vendor));

    VENDOR_CALL(device, stop_preview);
}

static int camera_preview_enabled(struct camera_device *device)
{
    if (!device)
        return -EINVAL;

    ALOGV("%s->%08X->%08X", __func__, (uintptr_t)device,
            (uintptr_t)(((wrapper_camera_device_t*)device)->vendor));

    return VENDOR_CALL(device, preview_enabled);
}

static int camera_store_meta_data_in_buffers(struct camera_device *device,
        int enable)
{
    if (!device)
        return -EINVAL;

    ALOGV("%s->%08X->%08X", __func__, (uintptr_t)device,
            (uintptr_t)(((wrapper_camera_device_t*)device)->vendor));

    return VENDOR_CALL(device, store_meta_data_in_buffers, enable);
}

static int camera_start_recording(struct camera_device *device)
{
    if (!device)
        return EINVAL;

    ALOGV("%s->%08X->%08X", __func__, (uintptr_t)device,
            (uintptr_t)(((wrapper_camera_device_t*)device)->vendor));

    return VENDOR_CALL(device, start_recording);
}

static void camera_stop_recording(struct camera_device *device)
{
    if (!device)
        return;

    ALOGV("%s->%08X->%08X", __func__, (uintptr_t)device,
            (uintptr_t)(((wrapper_camera_device_t*)device)->vendor));

    VENDOR_CALL(device, stop_recording);
}

static int camera_recording_enabled(struct camera_device *device)
{
    if (!device)
        return -EINVAL;

    ALOGV("%s->%08X->%08X", __func__, (uintptr_t)device,
            (uintptr_t)(((wrapper_camera_device_t*)device)->vendor));

    return VENDOR_CALL(device, recording_enabled);
}

static void camera_release_recording_frame(struct camera_device *device,
        const void *opaque)
{
    if (!device)
        return;

    ALOGV("%s->%08X->%08X", __func__, (uintptr_t)device,
            (uintptr_t)(((wrapper_camera_device_t*)device)->vendor));

    VENDOR_CALL(device, release_recording_frame, opaque);
}

static int camera_auto_focus(struct camera_device *device)
{
    if (!device)
        return -EINVAL;

    ALOGV("%s->%08X->%08X", __func__, (uintptr_t)device,
            (uintptr_t)(((wrapper_camera_device_t*)device)->vendor));

    return VENDOR_CALL(device, auto_focus);
}

static int camera_cancel_auto_focus(struct camera_device *device)
{
    if (!device)
        return -EINVAL;

    ALOGV("%s->%08X->%08X", __func__, (uintptr_t)device,
            (uintptr_t)(((wrapper_camera_device_t*)device)->vendor));

    return VENDOR_CALL(device, cancel_auto_focus);
}

static int camera_take_picture(struct camera_device *device)
{
    if (!device)
        return -EINVAL;

    ALOGV("%s->%08X->%08X", __func__, (uintptr_t)device,
            (uintptr_t)(((wrapper_camera_device_t*)device)->vendor));

    return VENDOR_CALL(device, take_picture);
}

static int camera_cancel_picture(struct camera_device *device)
{
    if (!device)
        return -EINVAL;

    ALOGV("%s->%08X->%08X", __func__, (uintptr_t)device,
            (uintptr_t)(((wrapper_camera_device_t*)device)->vendor));

    return VENDOR_CALL(device, cancel_picture);
}

static int camera_set_parameters(struct camera_device *device,
        const char *params)
{
    if (!device)
        return -EINVAL;

    ALOGV("%s->%08X->%08X", __func__, (uintptr_t)device,
            (uintptr_t)(((wrapper_camera_device_t*)device)->vendor));

    char *tmp = camera_fixup_setparams(CAMERA_ID(device), params);

    return VENDOR_CALL(device, set_parameters, tmp);
}

static char *camera_get_parameters(struct camera_device *device)
{
    if (!device)
        return NULL;

    ALOGV("%s->%08X->%08X", __func__, (uintptr_t)device,
            (uintptr_t)(((wrapper_camera_device_t*)device)->vendor));

    char *params = VENDOR_CALL(device, get_parameters);

    char *tmp = camera_fixup_getparams(CAMERA_ID(device), params);
    VENDOR_CALL(device, put_parameters, params);
    params = tmp;

    return params;
}

static void camera_put_parameters(struct camera_device *device, char *params)
{
    if (params)
        free(params);

    if (!device)
        return;

    ALOGV("%s->%08X->%08X", __func__, (uintptr_t)device,
            (uintptr_t)(((wrapper_camera_device_t*)device)->vendor));
}

static int camera_send_command(struct camera_device *device,
        int32_t cmd, int32_t arg1, int32_t arg2)
{
    if (!device)
        return -EINVAL;

    ALOGV("%s->%08X->%08X", __func__, (uintptr_t)device,
            (uintptr_t)(((wrapper_camera_device_t*)device)->vendor));

    return VENDOR_CALL(device, send_command, cmd, arg1, arg2);
}

static void camera_release(struct camera_device *device)
{
    if (!device)
        return;

    ALOGV("%s->%08X->%08X", __func__, (uintptr_t)device,
            (uintptr_t)(((wrapper_camera_device_t*)device)->vendor));

    VENDOR_CALL(device, release);
}

static int camera_dump(struct camera_device *device, int fd)
{
    if (!device)
        return -EINVAL;

    ALOGV("%s->%08X->%08X", __func__, (uintptr_t)device,
            (uintptr_t)(((wrapper_camera_device_t*)device)->vendor));

    return VENDOR_CALL(device, dump, fd);
}

extern "C" void heaptracker_free_leaked_memory(void);

static int camera_device_close(hw_device_t *device)
{
    int ret = 0;
    wrapper_camera_device_t *wrapper_dev = NULL;

    ALOGV("%s", __func__);

    Mutex::Autolock lock(gCameraWrapperLock);

    if (!device) {
        ret = -EINVAL;
        goto done;
    }

    for (int i = 0; i < camera_get_number_of_cameras(); i++) {
        if (fixed_set_params[i])
            free(fixed_set_params[i]);
    }

    wrapper_dev = (wrapper_camera_device_t*) device;

    wrapper_dev->vendor->common.close((hw_device_t*)wrapper_dev->vendor);
    if (wrapper_dev->base.ops)
        free(wrapper_dev->base.ops);
    free(wrapper_dev);
done:
#ifdef HEAPTRACKER
    heaptracker_free_leaked_memory();
#endif
    return ret;
}

/*******************************************************************
 * implementation of camera_module functions
 *******************************************************************/

/* open device handle to one of the cameras
 *
 * assume camera service will keep singleton of each camera
 * so this function will always only be called once per camera instance
 */

static int camera_device_open(const hw_module_t *module, const char *name,
        hw_device_t **device)
{
    int rv = 0;
    int num_cameras = 0;
    int cameraid;
    wrapper_camera_device_t *camera_device = NULL;
    camera_device_ops_t *camera_ops = NULL;

    Mutex::Autolock lock(gCameraWrapperLock);

    ALOGV("%s", __func__);

    if (name != NULL) {
        if (check_vendor_module())
            return -EINVAL;

        cameraid = atoi(name);
        num_cameras = gVendorModule->get_number_of_cameras();

        fixed_set_params = (char **) malloc(sizeof(char *) * num_cameras);
        if (!fixed_set_params) {
            ALOGE("parameter memory allocation fail");
            rv = -ENOMEM;
            goto fail;
        }
        memset(fixed_set_params, 0, sizeof(char *) * num_cameras);

        if (cameraid > num_cameras) {
            ALOGE("camera service provided cameraid out of bounds, "
                    "cameraid = %d, num supported = %d",
                    cameraid, num_cameras);
            rv = -EINVAL;
            goto fail;
        }

        camera_device = (wrapper_camera_device_t*)malloc(sizeof(*camera_device));
        if (!camera_device) {
            ALOGE("camera_device allocation fail");
            rv = -ENOMEM;
            goto fail;
        }
        memset(camera_device, 0, sizeof(*camera_device));
        camera_device->id = cameraid;

        rv = gVendorModule->common.methods->open(
                (const hw_module_t*)gVendorModule, name,
                (hw_device_t**)&(camera_device->vendor));
        if (rv) {
            ALOGE("vendor camera open fail");
            goto fail;
        }
        ALOGV("%s: got vendor camera device 0x%08X",
                __func__, (uintptr_t)(camera_device->vendor));

        camera_ops = (camera_device_ops_t*)malloc(sizeof(*camera_ops));
        if (!camera_ops) {
            ALOGE("camera_ops allocation fail");
            rv = -ENOMEM;
            goto fail;
        }

        memset(camera_ops, 0, sizeof(*camera_ops));

        camera_device->base.common.tag = HARDWARE_DEVICE_TAG;
        camera_device->base.common.version = CAMERA_DEVICE_API_VERSION_1_0;
        camera_device->base.common.module = (hw_module_t *)(module);
        camera_device->base.common.close = camera_device_close;
        camera_device->base.ops = camera_ops;

        camera_ops->set_preview_window = camera_set_preview_window;
        camera_ops->set_callbacks = camera_set_callbacks;
        camera_ops->enable_msg_type = camera_enable_msg_type;
        camera_ops->disable_msg_type = camera_disable_msg_type;
        camera_ops->msg_type_enabled = camera_msg_type_enabled;
        camera_ops->start_preview = camera_start_preview;
        camera_ops->stop_preview = camera_stop_preview;
        camera_ops->preview_enabled = camera_preview_enabled;
        camera_ops->store_meta_data_in_buffers = camera_store_meta_data_in_buffers;
        camera_ops->start_recording = camera_start_recording;
        camera_ops->stop_recording = camera_stop_recording;
        camera_ops->recording_enabled = camera_recording_enabled;
        camera_ops->release_recording_frame = camera_release_recording_frame;
        camera_ops->auto_focus = camera_auto_focus;
        camera_ops->cancel_auto_focus = camera_cancel_auto_focus;
        camera_ops->take_picture = camera_take_picture;
        camera_ops->cancel_picture = camera_cancel_picture;
        camera_ops->set_parameters = camera_set_parameters;
        camera_ops->get_parameters = camera_get_parameters;
        camera_ops->put_parameters = camera_put_parameters;
        camera_ops->send_command = camera_send_command;
        camera_ops->release = camera_release;
        camera_ops->dump = camera_dump;

        *device = &camera_device->base.common;
    }

    return rv;

fail:
    if (camera_device) {
        free(camera_device);
        camera_device = NULL;
    }
    if (camera_ops) {
        free(camera_ops);
        camera_ops = NULL;
    }
    *device = NULL;
    return rv;
}

static int camera_get_number_of_cameras(void)
{
    ALOGV("%s", __func__);
    if (check_vendor_module())
        return 0;
    return gVendorModule->get_number_of_cameras();
}

static int camera_get_camera_info(int camera_id, struct camera_info *info)
{
    ALOGV("%s", __func__);
    if (check_vendor_module())
        return 0;
    return gVendorModule->get_camera_info(camera_id, info);
}
