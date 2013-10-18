LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE      := libgdmcprov
LOCAL_MODULE_TAGS := eng

LOCAL_C_INCLUDES  := $(LOCAL_PATH)/../inc_private \
                     $(LOCAL_PATH)/../inc_public \
                     $(MC_INCLUDE_DIR)

LOCAL_SRC_FILES   := ../src/gdmcprovlib.cpp \
                     ../src/crc32.c \
                     ../src/mobicore.c \
                     ../src/gdmcdevicebinding.cpp

LOCAL_CFLAGS      := -O2 -Wall -fomit-frame-pointer -DANDROID_ARM -DARM -D_LENDIAN -D_32BIT \
                     -fvisibility=hidden -I$(OPENSSL_INC_DIR) \
                     -DGDMCPROVLIB_VERSION=0x01000001 -D$(MC_DEBUG) \
                     -D_NO_OPENSSL_INCLUDES

LOCAL_CXXFLAGS    := -O2 -Wall -fomit-frame-pointer -DANDROID_ARM -DARM -D_LENDIAN -D_32BIT \
                     -fvisibility-inlines-hidden -fvisibility=hidden \
                     -DGDMCPROVLIB_VERSION=0x01000001 -D$(MC_DEBUG)

LOCAL_CPPFLAGS    := -O2 -Wall -fomit-frame-pointer -DANDROID_ARM -DARM -D_LENDIAN -D_32BIT \
                     -fvisibility-inlines-hidden -fvisibility=hidden \
                     -DGDMCPROVLIB_VERSION=0x01000001 -D$(MC_DEBUG)

LOCAL_LDFLAGS     := -Wl,-rpath-link,$(SYSTEM_LIB_DIR) \
                     -L$(SYSTEM_LIB_DIR) -llog

LOCAL_SHARED_LIBRARIES  := $(GDM_PROVLIB_SHARED_LIBS)

include $(BUILD_SHARED_LIBRARY)
