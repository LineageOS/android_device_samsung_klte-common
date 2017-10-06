LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_C_INCLUDES := \
    system/media/camera/include

LOCAL_SRC_FILES := \
    CameraWrapper.cpp

LOCAL_STATIC_LIBRARIES := libbase libarect
LOCAL_SHARED_LIBRARIES := \
    libhardware \
    liblog \
    libcamera_client \
    libgui \
    libhidltransport \
    libsensor \
    libutils \
    android.hidl.token@1.0-utils \
    android.hardware.graphics.bufferqueue@1.0

LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_MODULE := camera.$(TARGET_BOARD_PLATFORM)
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := CameraParameters.cpp
LOCAL_MODULE := libcamera_parameters_shim
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_TAGS := optional
LOCAL_32_BIT_ONLY := true
include $(BUILD_SHARED_LIBRARY)
