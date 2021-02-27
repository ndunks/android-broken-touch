
URL := https://android.googlesource.com/platform
BASE_TAG := android-4.4.2_r2.0.1

all: frameworks/native/include \
     hardware/libhardware/include \
     system/core/include \
     external/skia/include \
     external/skia/src

%:
	@echo "downloading \033[32m$@\033[0m"
	mkdir -p $*
	wget -O - $(URL)/$(dir $*)+archive/refs/tags/$(BASE_TAG)/$(notdir $*).tar.gz | \
	tar -xzC $*

clean:
	rm -rf external frameworks hardware system

.PHONY: clean all