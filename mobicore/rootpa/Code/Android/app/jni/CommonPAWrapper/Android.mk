#
# Copyright © Trustonic Limited 2013
#
# All rights reserved.
#
#  Redistribution and use in source and binary forms, with or without modification, 
#  are permitted provided that the following conditions are met:
#
#  1. Redistributions of source code must retain the above copyright notice, this 
#     list of conditions and the following disclaimer.
#
#  2. Redistributions in binary form must reproduce the above copyright notice, 
#     this list of conditions and the following disclaimer in the documentation 
#     and/or other materials provided with the distribution.
#
#  3. Neither the name of the Trustonic Limited nor the names of its contributors 
#     may be used to endorse or promote products derived from this software 
#     without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
# IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
# INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
# LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
# OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED 
# OF THE POSSIBILITY OF SUCH DAMAGE.
#

#
# makefile for building the provisioning agent JNI wrapper for using the Common part
# from Android Java code. build the code by executing
# $NDK_ROOT/ndk-build in the folder where this file resides
# (or build all from one level up)
#
# Naturally the right way to build this is using build script in Build folder.
# It then uses this file in turn.

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_CFLAGS += -DANDROID_ARM=1
LOCAL_CFLAGS +=-fstack-protector
ifeq ($(DEBUG), 1)
    LOCAL_CFLAGS += -D__DEBUG=1
endif   

LOCAL_SRC_FILES += commonwrapper.cpp
LOCAL_SRC_FILES += JniHelpers.cpp
LOCAL_SRC_FILES += CmpResponses.cpp
LOCAL_SRC_FILES += CmpCommands.cpp
LOCAL_SRC_FILES += CmpBase.cpp

LOCAL_C_INCLUDES +=  $(MOBICORE_DIR_INC)
LOCAL_C_INCLUDES +=  $(MOBICORE_DIR_INC)/TlCm
LOCAL_C_INCLUDES +=  $(LOCAL_PATH)/../../../../Common/include

LOCAL_MODULE    := libcommonpawrapper
LOCAL_MODULE_TAGS := eng optional

ifeq ($(ROOTPA_MODULE_TEST), 1)
    LOCAL_STATIC_LIBRARIES += provisioningagent_test
    LOCAL_STATIC_LIBRARIES += McStub    
else
    LOCAL_STATIC_LIBRARIES += provisioningagent

    LOCAL_SHARED_LIBRARIES  += libMcClient
    LOCAL_SHARED_LIBRARIES  += libMcRegistry
endif

#LOCAL_LDLIBS += -L$(SYSROOT)/usr/lib

LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libz
LOCAL_SHARED_LIBRARIES += libssl
LOCAL_SHARED_LIBRARIES += libcrypto
LOCAL_SHARED_LIBRARIES += libcurl
LOCAL_STATIC_LIBRARIES += libxml2
LOCAL_SHARED_LIBRARIES += libicuuc

include $(BUILD_SHARED_LIBRARY)

