/*
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ANDROID_EXYNOS_HWC_MODULE_H_
#define ANDROID_EXYNOS_HWC_MODULE_H_
#include <hardware/hwcomposer.h>
#include <linux/s3c-fb.h>
const size_t GSC_DST_W_ALIGNMENT_RGB888 = 16;
const size_t GSC_DST_CROP_W_ALIGNMENT_RGB888 = 1;
#define VSYNC_DEV_NAME  "/sys/devices/platform/exynos-sysmmu.11/exynos5-fb.1/vsync"
#define HWC_DYNAMIC_RECOMPOSITION
#define SKIP_STATIC_LAYER_COMP
#define USES_ONLY_GSC0_GSC1
#define MIXER_UPDATE
#define FIMD_WORD_SIZE_BYTES   16
#define FIMD_BURSTLEN   16
#define TV_BLANK
#define FIMD_BW_OVERLAP_CHECK
#define TRY_SECOND_VSYNC_DEV
#ifdef TRY_SECOND_VSYNC_DEV
#define VSYNC_DEV_NAME2  "/sys/devices/platform/exynos-sysmmu.30/exynos-sysmmu.11/exynos5-fb.1/vsync"
#endif

#define FIMD_DMA_BW_BALANCE

#ifdef FIMD_BW_OVERLAP_CHECK
const size_t MAX_NUM_FIMD_DMA_CH = 2;
const int FIMD_DMA_CH_IDX[] = {0, 1, 1, 1, 0};
const int FIMD_DMA_CH_BW_SET1[MAX_NUM_FIMD_DMA_CH] = {1920 * 1080 *2, 1920 * 1080 *2};
const int FIMD_DMA_CH_BW_SET2[MAX_NUM_FIMD_DMA_CH] = {2560 * 1600, 2560 * 1600 *2};
const int FIMD_DMA_CH_OVERLAP_CNT_SET1[MAX_NUM_FIMD_DMA_CH] = {2, 2};
const int FIMD_DMA_CH_OVERLAP_CNT_SET2[MAX_NUM_FIMD_DMA_CH] = {1, 2};

inline void fimd_bw_overlap_limits_init(int xres, int yres,
            int *fimd_dma_chan_max_bw, int *fimd_dma_chan_max_overlap_cnt)
{
    if (xres * yres > 1920 * 1080) {
        for (size_t i = 0; i < MAX_NUM_FIMD_DMA_CH; i++) {
            fimd_dma_chan_max_bw[i] = FIMD_DMA_CH_BW_SET2[i];
            fimd_dma_chan_max_overlap_cnt[i] = FIMD_DMA_CH_OVERLAP_CNT_SET2[i];
        }
    } else {
        for (size_t i = 0; i < MAX_NUM_FIMD_DMA_CH; i++) {
            fimd_dma_chan_max_bw[i] = FIMD_DMA_CH_BW_SET1[i];
            fimd_dma_chan_max_overlap_cnt[i] = FIMD_DMA_CH_OVERLAP_CNT_SET1[i];
        }
    }
}
#endif
#endif
