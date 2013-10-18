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
# makefile for building the provisioning agent Common part for android. build the code by executing 
# $NDK_ROOT/ndk-build in the folder where this file resides
#
# naturally the right way to build is to use build script under Build folder. It then uses this file.
#



LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_CFLAGS += -DANDROID_ARM=1
LOCAL_CFLAGS += -DANDROID 
LOCAL_CFLAGS +=-fstack-protector
ifeq ($(DEBUG), 1)
    LOCAL_CFLAGS += -D__DEBUG=1
endif    

LOCAL_SRC_FILES += ../../../../Common/commandhandler.c
LOCAL_SRC_FILES += ../../../../Common/pacmp3.c
LOCAL_SRC_FILES += ../../../../Common/pacmtl.c
LOCAL_SRC_FILES += ../../../../Common/trustletchannel.c
LOCAL_SRC_FILES += ../../../../Common/registry.c
LOCAL_SRC_FILES += ../../../../Common/seclient.c
LOCAL_SRC_FILES += ../../../../Common/base64.c
LOCAL_SRC_FILES += ../../../../Common/xmlmessagehandler.c
LOCAL_SRC_FILES += ../../../../Common/provisioningengine.c
LOCAL_SRC_FILES += ../../../../Common/contentmanager.c

LOCAL_C_INCLUDES +=  $(MOBICORE_DIR_INC)
LOCAL_C_INCLUDES +=  $(MOBICORE_DIR_INC)/TlCm
LOCAL_C_INCLUDES +=  $(MOBICOREDRIVER_DIR_INC)
LOCAL_C_INCLUDES +=  $(MOBICOREDRIVER_DIR_INC2)
LOCAL_C_INCLUDES +=  external/curl/include
LOCAL_C_INCLUDES +=  external/libxml2/include
LOCAL_C_INCLUDES +=  external/icu4c/common
LOCAL_C_INCLUDES +=  $(LOCAL_PATH)/../../../../Common
LOCAL_C_INCLUDES +=  $(LOCAL_PATH)/../../../../Common/include

ifeq ($(ROOTPA_MODULE_TEST), 1)
    LOCAL_STATIC_LIBRARIES +=  McStub
    LOCAL_MODULE    := provisioningagent_test
else
    LOCAL_MODULE    := provisioningagent
endif

LOCAL_MODULE_TAGS := optional

include $(BUILD_STATIC_LIBRARY)
