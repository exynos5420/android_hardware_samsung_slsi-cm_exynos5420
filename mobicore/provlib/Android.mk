# =============================================================================
#
# Makefile pointing to all makefiles within the project.
#
# =============================================================================

PROJECT_PATH := $(call my-dir)

$(call import-module,$(COMP_PATH_MobiCoreDriverLib))

# Include the Scripts
include $(PROJECT_PATH)/jni/Android.mk