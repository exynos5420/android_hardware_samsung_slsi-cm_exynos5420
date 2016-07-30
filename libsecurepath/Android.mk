#
#
# Copyright (C) 2009 The Android Open Source Project
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
#

LOCAL_PATH := $(call my-dir)


###############################################################################
# libcsecurepath.a
include $(CLEAR_VARS)
LOCAL_MODULE	:= libsecurepath
BUILD_DATE	:= \"`date '+%Y.%m.%d'`\"
LOCAL_CPPFLAGS	:= -Wall -D_BUILD_DATE=$(BUILD_DATE)
LOCAL_SRC_FILES	+= 	\
	tlc_communication.cpp \
	content_protect.cpp \
	sec_g2ddrm.cpp

LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/../../exynos5/include \
	$(TOP)/hardware/samsung_slsi-cm/exynos/include \
	$(TOP)/hardware/samsung_slsi-cm/exynos5420/mobicore/common/LogWrapper

LOCAL_SHARED_LIBRARIES += libMcClient liblog

LOCAL_CFLAGS += -DLOG_ANDROID

include $(BUILD_STATIC_LIBRARY)
