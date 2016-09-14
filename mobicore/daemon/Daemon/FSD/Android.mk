# =============================================================================
#
# MC driver server files
#
# =============================================================================

# This is not a separate module.
# Only for inclusion by other modules.

FSD_PATH := Daemon/FSD

# Add new folders with header files here
LOCAL_C_INCLUDES += $(LOCAL_PATH)/$(FSD_PATH)/public \
                    $(MOBICORE_PROJECT_PATH)/include/GPD_TEE_Internal_API

# Add new source files here
LOCAL_SRC_FILES += $(FSD_PATH)/FSD.cpp