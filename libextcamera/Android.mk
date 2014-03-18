LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

# HAL module implemenation stored in
# hw/<COPYPIX_HARDWARE_MODULE_ID>.<ro.product.board>.so

# mhjang LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR_SHARED_LIBRARIES)/hw
LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/

LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/../include \
	$(TOP)/hardware/samsung_slsi/exynos/include \
	$(TOP)/hardware/samsung_slsi/exynos3/include \
	$(TOP)/hardware/samsung_slsi/exynos5/include \
	$(TOP)/hardware/samsung_slsi/exynos5410/include \
	frameworks/native/include \
	system/media/camera/include

LOCAL_SRC_FILES:= \
	Exif.cpp \
	SecCameraParameters.cpp \
	ISecCameraHardware.cpp \
	SecCameraHardware.cpp

LOCAL_SHARED_LIBRARIES:= libutils libcutils libbinder liblog libcamera_client libhardware

LOCAL_CFLAGS += -DGAIA_FW_BETA

LOCAL_SHARED_LIBRARIES += libexynosutils libhwjpeg libexynosv4l2 libcsc libion_exynos libcamera_metadata

# LOCAL_MODULE := camera.tf4
# LOCAL_MODULE := camera.ext.$(TARGET_BOOTLOADER_BOARD_NAME)
LOCAL_MODULE := libextcamera

LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)
