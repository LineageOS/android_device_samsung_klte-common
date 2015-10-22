LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SHARED_LIBRARIES := liblog libcutils libbinder libutils

LOCAL_SRC_FILES := \
    ril_shim.c

LOCAL_MODULE := librilsym
LOCAL_MODULE_CLASS := SHARED_LIBRARIES

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_SHARED_LIBRARIES := liblog libcutils libutils

LOCAL_SRC_FILES := \
    rmt_storage_shim.c

LOCAL_MODULE := librmtsym
LOCAL_MODULE_CLASS := SHARED_LIBRARIES

include $(BUILD_SHARED_LIBRARY)
