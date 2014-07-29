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
 * \file      ExynosCameraActivitySpecialCapture.h
 * \brief     hearder file for CAMERA HAL MODULE
 * \author    Pilsun Jang(pilsun.jang@samsung.com)
 * \date      2012/12/19
 *
 */

#ifndef EXYNOS_CAMERA_ACTIVITY_SPECIAL_CAPTURE_H__
#define EXYNOS_CAMERA_ACTIVITY_SPECIAL_CAPTURE_H__

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
#include "ExynosJpegEncoderForCamera.h"
#include "ExynosExif.h"
#include "exynos_v4l2.h"

#include "fimc-is-metadata.h"
#include "ExynosCameraActivityBase.h"


#define CAPTURE_SKIP_COUNT (1)

#define BAYER_LOCK (2)

namespace android {

class ExynosCameraActivitySpecialCapture : public ExynosCameraActivityBase {
public:
    enum SCAPTURE_DUMMY {
        SCAPTURE_DUMMY,
        SCAPTURE_DUMMY1
    };

    enum SCAPTURE_MODE {
        SCAPTURE_MODE_NONE,
        SCAPTURE_MODE_LLL,
        SCAPTURE_MODE_END
    };

    enum SCAPTURE_STEP {
        SCAPTURE_STEP_OFF,
        SCAPTURE_STEP_START,
        SCAPTURE_STEP_MINUS_SET,
        SCAPTURE_STEP_ZERO_DELAY_SET,
        SCAPTURE_STEP_ZERO_SET,
        SCAPTURE_STEP_PLUS_SET,
        SCAPTURE_STEP_RESTORE,
        SCAPTURE_STEP_WAIT_CAPTURE_DELAY,
        SCAPTURE_STEP_WAIT_CAPTURE,
        SCAPTURE_STEP_END
    };

public:
    ExynosCameraActivitySpecialCapture();
    virtual ~ExynosCameraActivitySpecialCapture();

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
    int setCaptureMode(enum SCAPTURE_MODE sCaptureModeVal);
    int setCaptureStep(enum SCAPTURE_STEP sCaptureStepVal);
private:
    enum SCAPTURE_MODE m_specialCaptureMode;
    enum SCAPTURE_STEP m_specialCaptureStep;

    unsigned int m_currentInputFcount;

    int m_backupAeExpCompensation;
    enum aa_scene_mode m_backupSceneMode;
    enum aa_mode m_backupAaMode;
    int m_delay;
    bool m_check;

    ExynosBuffer m_reqBuf;
};
}

#endif /* EXYNOS_CAMERA_ACTIVITY_SPECIAL_CAPTURE_H__ */
