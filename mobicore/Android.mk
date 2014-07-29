# =============================================================================
#
# Makefile pointing to all makefiles within the project.
#
# =============================================================================
MOBICORE_PROJECT_PATH := $(call my-dir)
# Setup common variables
LOG_WRAPPER := $(MOBICORE_PROJECT_PATH)/common/LogWrapper
COMP_PATH_MobiCore := $(MOBICORE_PROJECT_PATH)/common/MobiCore
COMP_PATH_MobiCoreDriverMod := $(MOBICORE_PROJECT_PATH)/include


# Application wide Cflags
GLOBAL_INCLUDES := bionic \
	external/stlport/stlport \
	$(COMP_PATH_MobiCore)/inc \
	$(COMP_PATH_MobiCoreDriverMod)/Public \
	$(COMP_PATH_MobiCore)/inc/TlCm

GLOBAL_LIBRARIES := libstlport

# Include the Daemon
include $(MOBICORE_PROJECT_PATH)/daemon/Android.mk

MC_INCLUDE_DIR := $(COMP_PATH_MobiCore)/inc \
    $(COMP_PATH_MobiCore)/inc/TlCm \
    $(COMP_PATH_MobiCore)/inc/TlCm/2.0 \
    $(MOBICORE_PROJECT_PATH)/daemon/ClientLib/public \
    $(MOBICORE_PROJECT_PATH)/daemon/Registry/Public
MC_DEBUG := _DEBUG
SYSTEM_LIB_DIR=/system/lib
GDM_PROVLIB_SHARED_LIBS=libMcClient
# Include the provisioning lib
include $(MOBICORE_PROJECT_PATH)/provlib/Android.mk


MOBICORE_DIR_INC := $(MC_INCLUDE_DIR) $(MOBICORE_PROJECT_PATH)/common/curl/include
include $(MOBICORE_PROJECT_PATH)/common/curl/Android.mk
include $(MOBICORE_PROJECT_PATH)/rootpa/Code/Android/app/jni/Android.mk
include $(MOBICORE_PROJECT_PATH)/rootpa/Code/Android/lib/Android.mk
include $(MOBICORE_PROJECT_PATH)/rootpa/Code/Android/app/Android.mk
