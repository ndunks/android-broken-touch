ifndef ANDROID_SDK_ROOT
$(error "ANDROID_SDK_ROOT not set")
endif

ifndef ANDROID_NDK_HOME
ANDROID_NDK_HOME=$(ANDROID_SDK_ROOT)/ndk
endif

export MODULE_NAME := brokentouch
export PROJECT_PATH := $(PWD)
export ROOT_PATH := $(dir $(PWD))

BUILD_PATH := $(PROJECT_PATH)/build
NDK_OUT := $(BUILD_PATH)
NDK_LIBS_OUT := $(BUILD_PATH)
APP_BUILD_SCRIPT  := $(PROJECT_PATH)/Android.mk
APP_ABI := armeabi-v7a


ANDROID_ARGS := \
	PROJECT_PATH=$(PROJECT_PATH) \
	APP_BUILD_SCRIPT=$(APP_BUILD_SCRIPT) \
	APP_ABI=$(APP_ABI) \
	NDK_OUT=$(NDK_OUT) \
	NDK_LIBS_OUT=$(NDK_LIBS_OUT) \
	PROJECT_PATH=$(PROJECT_PATH) \
	APP_PLATFORM=android-19 \
	NDK_PROJECT_PATH=null

ANDROID_BUILD := @make  --no-print-dir -f $(ANDROID_NDK_HOME)/build/core/build-local.mk $(ANDROID_ARGS)
UNLOCK_SCREEN = input keyevent KEYCODE_POWER

build:
ifeq (,$(wildcard include/frameworks))
	make -C include
endif
	$(ANDROID_BUILD)

clean:
	$(ANDROID_BUILD) clean
	@rm -rf $(BUILD_PATH)/* 2>/dev/null

upload:
	@adb $(ADB_FLAGS) push $(NDK_OUT)/$(APP_ABI)/$(MODULE_NAME) /data/local/tmp/
	@adb $(ADB_FLAGS) shell "su -c 'chmod 0755 /data/local/tmp/$(MODULE_NAME)'"

install: build upload
	@adb $(ADB_FLAGS) shell "su -c 'killall $(MODULE_NAME) & mount -o remount,rw /system'"
	@adb $(ADB_FLAGS) shell "su -c 'cp -f /data/local/tmp/$(MODULE_NAME) /system/xbin/$(MODULE_NAME)'"
	@adb $(ADB_FLAGS) shell "su -c 'chmod 0755 /system/xbin/$(MODULE_NAME)'"
	@adb $(ADB_FLAGS) shell "su -c 'mount -o remount,ro /system'"

exec: build upload
	
	@adb $(ADB_FLAGS) shell -n " \
		$(UNLOCK_SCREEN) && \
		cd /data/local/tmp/ && \
		busybox chmod +x $(MODULE_NAME) && \
		(pidof $(MODULE_NAME) && su -c 'killall $(MODULE_NAME)' || true ) && \
		./$(MODULE_NAME)"

watch:
	nodemon -V -i build/ -i include/ \
		-e c,cpp,h \
		-x "make 'ADB_FLAGS=$(ADB_FLAGS)' \
				'APPEND_CFLAGS=-DDEBUG' \
			'IP=$(IP)' exec || false"

.PHONY: build clean exec upload install