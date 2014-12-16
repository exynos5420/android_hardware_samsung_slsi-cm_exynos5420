# Copyright (C) 2012 The Android Open Source Project
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

LOCAL_PATH:= $(call my-dir)

#################
# libexynoscamera

include $(CLEAR_VARS)

LOCAL_SHARED_LIBRARIES:= libutils libcutils libbinder liblog libcamera_client libhardware
LOCAL_SHARED_LIBRARIES += libexynosutils libhwjpeg libexynosv4l2 libexynosgscaler libion_exynos libcsc
LOCAL_SHARED_LIBRARIES += libexpat libstlport
LOCAL_SHARED_LIBRARIES += libpower

LOCAL_CFLAGS += -DGAIA_FW_BETA

LOCAL_CFLAGS += -DUSE_CAMERA_ESD_RESET

LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/../include \
	$(LOCAL_PATH)/../libcamera \
	$(TOP)/hardware/samsung_slsi-cm/exynos/include \
	$(TOP)/hardware/samsung_slsi-cm/$(TARGET_SOC)/include \
	$(TOP)/hardware/samsung_slsi-cm/$(TARGET_BOARD_PLATFORM)/include \
	$(TOP)/hardware/samsung_slsi-cm/$(TARGET_BOARD_PLATFORM)/libcamera \
	$(TOP)/hardware/libhardware_legacy/include/hardware_legacy \
	$(TOP)/vendor/samsung/feature/CscFeature/libsecnativefeature \
	$(TOP)/bionic \
    $(TOP)/external/expat/lib \
    $(TOP)/external/stlport/stlport

LOCAL_SRC_FILES:= \
	ExynosCameraActivityBase.cpp \
	ExynosCameraActivityFlash.cpp \
	ExynosCameraActivityAutofocus.cpp \
	ExynosCameraActivitySpecialCapture.cpp \
	ExynosCameraVDis.cpp \
	ExynosCamera.cpp \
	ExynosJpegEncoderForCamera.cpp \
	ExynosCameraHWImpl.cpp

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libexynoscamera

include $(BUILD_SHARED_LIBRARY)

#################
# libcamera

include $(CLEAR_VARS)

# HAL module implemenation stored in
# hw/<COPYPIX_HARDWARE_MODULE_ID>.<ro.product.board>.so
LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw

LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/../include \
	$(TOP)/hardware/samsung_slsi-cm/exynos/include \
	$(TOP)/hardware/samsung_slsi-cm/$(TARGET_SOC)/include \
	$(TOP)/hardware/samsung_slsi-cm/$(TARGET_BOARD_PLATFORM)/include \
	$(TOP)/hardware/samsung_slsi-cm/$(TARGET_BOARD_PLATFORM)/libcamera \
	frameworks/native/include \
	system/media/camera/include

LOCAL_SRC_FILES:= \
	ExynosCameraHWInterface.cpp

LOCAL_CFLAGS += -DGAIA_FW_BETA
LOCAL_CFLAGS += -DBACK_ROTATION=$(BOARD_BACK_CAMERA_ROTATION)
LOCAL_CFLAGS += -DFRONT_ROTATION=$(BOARD_FRONT_CAMERA_ROTATION)

LOCAL_SHARED_LIBRARIES:= libutils libcutils libbinder liblog libcamera_client libhardware
LOCAL_SHARED_LIBRARIES += libexynosutils libhwjpeg libexynosv4l2 libcsc libion libexynoscamera

LOCAL_MODULE := camera.$(TARGET_BOOTLOADER_BOARD_NAME)

LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)

