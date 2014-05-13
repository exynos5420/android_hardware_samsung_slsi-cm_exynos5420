/*
 * Copyright 2012, Samsung Electronics Co. LTD
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed toggle an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*!
 * \file      ExynosCameraActivityAutofocus.h
 * \brief     hearder file for ExynosCameraActivityAutofocus
 * \author    Sangowoo Park(sw5771.park@samsung.com)
 * \date      2012/03/07
 *
 */

#ifndef EXYNOS_CAMERA_ACTIVITY_AUTOFOCUS_H__
#define EXYNOS_CAMERA_ACTIVITY_AUTOFOCUS_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <utils/threads.h>

#include <videodev2.h>
#include <videodev2_exynos_camera.h>
#include <linux/vt.h>

#include <utils/RefBase.h>
#include <utils/String8.h>
#include <utils/List.h>
#include "cutils/properties.h"

#include "exynos_format.h"
#include "ExynosBuffer.h"
#include "ExynosRect.h"
#include "exynos_v4l2.h"
#include "fimc-is-metadata.h"
#include "ExynosCameraActivityBase.h"

#define AUTOFOCUS_WATING_TIME        (10000)   /* 10msec */
#define AUTOFOCUS_TOTAL_WATING_TIME  (3000000) /* 3000msec */

namespace android {

class ExynosCameraActivityAutofocus : public ExynosCameraActivityBase {
public:
    enum AUTOFOCUS_MODE {
        AUTOFOCUS_MODE_BASE                     = (0),
        AUTOFOCUS_MODE_AUTO                     = (1 << 0), //!< \n
        AUTOFOCUS_MODE_INFINITY                 = (1 << 1), //!< \n
        AUTOFOCUS_MODE_MACRO                    = (1 << 2), //!< \n
        AUTOFOCUS_MODE_FIXED                    = (1 << 3), //!< \n
        AUTOFOCUS_MODE_EDOF                     = (1 << 4), //!< \n
        AUTOFOCUS_MODE_CONTINUOUS_VIDEO         = (1 << 5), //!< \n
        AUTOFOCUS_MODE_CONTINUOUS_PICTURE       = (1 << 6), //!< \n
        AUTOFOCUS_MODE_TOUCH                    = (1 << 7), //!< \n
        AUTOFOCUS_MODE_CONTINUOUS_PICTURE_MACRO = (1 << 8), //!< \n
    };

    enum AUTOFOCUS_MACRO_POSITION {
        AUTOFOCUS_MACRO_POSITION_BASE           = (0),
        AUTOFOCUS_MACRO_POSITION_CENTER         = (1 << 0), //!< \n
        AUTOFOCUS_MACRO_POSITION_CENTER_UP      = (1 << 1), //!< \n
    };

    enum AUTOFOCUS_STATE {
        AUTOFOCUS_STATE_NONE = 0,
        AUTOFOCUS_STATE_SCANNING,
        AUTOFOCUS_STATE_SUCCEESS,
        AUTOFOCUS_STATE_FAIL,
    };

public:
    ExynosCameraActivityAutofocus();
    virtual ~ExynosCameraActivityAutofocus();

protected:
    int t_funcNull(void *args);
    int t_funcSensorBefore(void *args);
    int t_funcSensorAfter(void *args);
    int t_func3ABefore(void *args);
    int t_func3AAfter(void *args);
    int t_funcISPBefore(void *args);
    int t_funcISPAfter(void *args);
    int t_funcSCPBefore(void *args);
    int t_funcSCPAfter(void *args);
    int t_funcSCCBefore(void *args);
    int t_funcSCCAfter(void *args);

public:
    bool setAutofocusMode(int autoFocusMode);
    int  getAutofocusMode(void);

    bool setFocusAreas(ExynosRect2 rect, int weight);
    bool getFocusAreas(ExynosRect2 *rect, int *weight);

    bool startAutofocus(void);
    bool stopAutofocus(void);
    bool flagAutofocusStart(void);

    bool lockAutofocus(void);
    bool unlockAutofocus(void);
    bool flagLockAutofocus(void);

    bool getAutofocusResult(void);
    int  getCurrentState(void);

    bool setRecordingHint(bool hint);
    bool setFaceDetection(bool toggle);
    bool setMacroPosition(int macroPosition);

private:
    enum AUTOFOCUS_STEP {
        AUTOFOCUS_STEP_BASE = 0,
        AUTOFOCUS_STEP_STOP,
        AUTOFOCUS_STEP_REQUEST,
        AUTOFOCUS_STEP_START,
        AUTOFOCUS_STEP_START_SCANNING,
        AUTOFOCUS_STEP_SCANNING,
        AUTOFOCUS_STEP_DONE,
    };

    bool m_flagAutofocusStart;
    bool m_flagAutofocusLock;

    int  m_autoFocusMode;           /* set by user */
    int  m_interenalAutoFocusMode;  /* set by this */

    ExynosRect2 m_focusArea;
    int  m_focusWeight;

    int  m_autofocusStep;
    int  m_aaAfState;
    int  m_afState;
    int  m_aaAFMode;                /* set on h/w */
    int  m_waitCountFailState;
    int  m_stepRequestCount;
    int  m_frameCount;

    bool m_recordingHint;
    bool m_flagFaceDetection;
    int  m_macroPosition;

    int  m_AUTOFOCUS_MODE2AA_AFMODE(int autoFocusMode);
};
}

#endif /* EXYNOS_CAMERA_ACTIVITY_AUTOFOCUS_H__ */
