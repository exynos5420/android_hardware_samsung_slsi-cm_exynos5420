#ifndef EXYNOS_VIRTUAL_DISPLAY_MODULE_H
#define EXYNOS_VIRTUAL_DISPLAY_MODULE_H

#include "ExynosVirtualDisplay.h"

class ExynosVirtualDisplayModule : public ExynosVirtualDisplay {
    public:
        ExynosVirtualDisplayModule(struct exynos5_hwc_composer_device_1_t *pdev);
        ~ExynosVirtualDisplayModule();

        virtual int32_t getDisplayAttributes(const uint32_t attribute);
};

enum {
    HWC_DISPLAY_COMPOSITION_TYPE = 0,
    HWC_DISPLAY_GLES_FORMAT,
    HWC_DISPLAY_SINK_BQ_FORMAT,
    HWC_DISPLAY_SINK_BQ_USAGE,
    HWC_DISPLAY_SINK_BQ_WIDTH,
    HWC_DISPLAY_SINK_BQ_HEIGHT
};

#endif
