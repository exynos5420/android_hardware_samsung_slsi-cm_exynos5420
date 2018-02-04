# Copyright (C) 2008 The Android Open Source Project
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

ifeq ($(filter-out exynos5,$(TARGET_BOARD_PLATFORM)),)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SHARED_LIBRARIES := liblog libutils libcutils libexynosutils libexynosv4l2 libsync libexynosdisplay libvirtualdisplay

LOCAL_CFLAGS += -DUSES_VIRTUAL_DISPLAY

LOCAL_C_INCLUDES := \
	$(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include \
	$(LOCAL_PATH)/../../exynos5/include \
	$(TOP)/hardware/samsung_slsi-cm/exynos/include \
	$(TOP)/hardware/samsung_slsi-cm/exynos/libexynosutils \
	$(TOP)/hardware/samsung_slsi-cm/$(TARGET_SOC)/include \
	$(TOP)/hardware/samsung_slsi-cm/$(TARGET_SOC)/libhwcmodule \
	$(TOP)/hardware/samsung_slsi-cm/exynos/libhwc \
	$(TOP)/hardware/samsung_slsi-cm/exynos/libhwcutils \
	$(TOP)/hardware/samsung_slsi-cm/exynos/libexynosdisplay \
	$(TOP)/hardware/samsung_slsi-cm/exynos/libvirtualdisplay

LOCAL_ADDITIONAL_DEPENDENCIES += \
	$(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_SRC_FILES := \
	ExynosVirtualDisplayModule.cpp

LOCAL_MODULE_TAGS := eng
LOCAL_MODULE := libvirtualdisplaymodule
include $(BUILD_SHARED_LIBRARY)

endif
