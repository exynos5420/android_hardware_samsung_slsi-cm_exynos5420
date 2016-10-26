# Copyright (C) 2013 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


LOCAL_PATH := $(call my-dir)

# HAL module implemenation stored in
# hw/<OVERLAY_HARDWARE_MODULE_ID>.<ro.product.board>.so
include $(CLEAR_VARS)

LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR_SHARED_LIBRARIES)/hw
LOCAL_SHARED_LIBRARIES := liblog libcutils libion libutils libGLESv1_CM

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/../include \
	$(TOP)/hardware/samsung_slsi-cm/exynos/include \
	$(TOP)/hardware/samsung_slsi-cm/exynos5/include

LOCAL_SRC_FILES := 	\
	gralloc.cpp 	\
	gralloc_vsync.cpp \
	framebuffer.cpp \
	mapper.cpp

LOCAL_CFLAGS := -DLOG_TAG=\"gralloc\"

ifeq ($(BOARD_USE_BGRA_8888),true)
LOCAL_CFLAGS += -DUSE_BGRA_8888
endif

LOCAL_MODULE := gralloc.exynos5
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_OWNER := samsung_arm

include $(BUILD_SHARED_LIBRARY)
