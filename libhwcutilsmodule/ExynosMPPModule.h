#ifndef EXYNOS_MPP_MODULE_H
#define EXYNOS_MPP_MODULE_H

#include "ExynosMPP.h"

class ExynosDisplay;

class ExynosMPPModule : public ExynosMPP {
    public:
        ExynosMPPModule(ExynosDisplay *display, int index);
        ~ExynosMPPModule();
};

#endif
