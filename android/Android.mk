ifndef ROOT_PATH
$(error ROOT_PATH not defined, please build using GNU Makefile (make) command)
endif

LOCAL_PATH:= $(call my-dir)
PLATFORM_INCLUDE_PATH := $(LOCAL_PATH)/include/$(TARGET_PLATFORM)
PREBUILT=$(PLATFORM_INCLUDE_PATH)/prebuilt-$(TARGET_ARCH_ABI)

include $(CLEAR_VARS)

LOCAL_C_INCLUDES       	:= $(PREBUILT)
include $(CLEAR_VARS)
LOCAL_MODULE 			:= libcutils
LOCAL_SRC_FILES 		:= $(PREBUILT)/libcutils.so
include $(PREBUILT_SHARED_LIBRARY)
include $(CLEAR_VARS)

LOCAL_C_INCLUDES       	:= $(PREBUILT)
include $(CLEAR_VARS)
LOCAL_MODULE 			:= libutils
LOCAL_SRC_FILES 		:= $(PREBUILT)/libutils.so
include $(PREBUILT_SHARED_LIBRARY)
include $(CLEAR_VARS)

LOCAL_C_INCLUDES       	:= $(PREBUILT)
include $(CLEAR_VARS)
LOCAL_MODULE 			:= libbinder
LOCAL_SRC_FILES 		:= $(PREBUILT)/libbinder.so
include $(PREBUILT_SHARED_LIBRARY)
include $(CLEAR_VARS)

LOCAL_C_INCLUDES       	:= $(PREBUILT)
include $(CLEAR_VARS)
LOCAL_MODULE 			:= libui
LOCAL_SRC_FILES 		:= $(PREBUILT)/libui.so
include $(PREBUILT_SHARED_LIBRARY)
include $(CLEAR_VARS)


LOCAL_MODULE = $(MODULE_NAME)
LOCAL_SRC_FILES := \
	main.cpp

LOCAL_CXXFLAGS += -Wno-multichar -DHAVE_SYS_UIO_H $(APPEND_CFLAGS)  -I../shared
#LOCAL_CPPFLAGS += -DHAVE_SYS_UIO_H
#LOCAL_MODULE:= screenstream
LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libutils \
	libbinder \
    libui
LOCAL_C_INCLUDES += \
 	$(PLATFORM_INCLUDE_PATH)/system/core/include \
	$(PLATFORM_INCLUDE_PATH)/frameworks/native/include \
	$(PLATFORM_INCLUDE_PATH)/hardware/libhardware/include
include $(BUILD_EXECUTABLE)
