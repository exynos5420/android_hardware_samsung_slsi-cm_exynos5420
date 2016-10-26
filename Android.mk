#
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
#

ifeq ($(TARGET_BOARD_PLATFORM), exynos5)
ifeq ($(TARGET_SLSI_VARIANT), cm)
ifeq ($(TARGET_SOC), exynos5420)

exynos5420_dirs := \
    mobicore \
	gralloc \
	libdisplaymodule \
	libhwcutilsmodule \
	libhdmimodule \
	libhwjpeg \
	libexynoscamera \
	libcamera \
	libsecurepath

ifeq ($(BOARD_USES_VIRTUAL_DISPLAY), true)
exynos5420_dirs += \
	libvirtualdisplaymodule
endif

ifeq ($(BOARD_USES_TRUST_KEYMASTER), true)
exynos5420_dirs += \
	libkeymaster
endif

include $(call all-named-subdir-makefiles,$(exynos5420_dirs))

endif
endif
endif