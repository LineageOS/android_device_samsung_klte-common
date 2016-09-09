LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := \
	SecNativeFeatureCIf.cpp \
	SecNativeFeatureCppIf.cpp

LOCAL_C_INCLUDES += \
	external/expat/lib

LOCAL_SHARED_LIBRARIES := \
	libexpat

LOCAL_CFLAGS := -Wall -Werror

LOCAL_MODULE := libsecnativefeature

include $(BUILD_SHARED_LIBRARY)
