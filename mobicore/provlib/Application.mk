# =============================================================================
#
# Main build file defining the project modules and their global variables.
#
# =============================================================================

# Don't remove this - mandatory
APP_PROJECT_PATH := $(call my-dir)

# Don't optimize for better debugging
APP_OPTIM := debug

# Show all warnings
#APP_CFLAGS := -Wall

MC_INCLUDE_DIR := \
    $(COMP_PATH_TlCm)/Public \
    $(COMP_PATH_TlCm)/Public/TlCm \
    $(COMP_PATH_TlCm)/Public/TlCm/2.0 \
    $(COMP_PATH_MobiCoreDriverLib)/Public
MC_DEBUG := _DEBUG
SYSTEM_LIB_DIR=/system/lib
GDM_PROVLIB_SHARED_LIBS=MobiCoreDriver