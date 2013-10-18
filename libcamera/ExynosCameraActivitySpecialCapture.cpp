/*
**
** Copyright 2012, Samsung Electronics Co. LTD
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

/* #define LOG_NDEBUG 0 */
#define LOG_TAG "ExynosCameraActivitySpecialCapture"
#include <cutils/log.h>

#include "ExynosCameraActivitySpecialCapture.h"
#include "ExynosCamera.h"

#define TIME_CHECK 1

namespace android {

class ExynosCamera;

ExynosCameraActivitySpecialCapture::ExynosCameraActivitySpecialCapture()
{
    t_isExclusiveReq = false;
    t_isActivated = false;
    t_reqNum = 0x1F;
    t_reqStatus = 0;

    m_currentInputFcount = 0;
    m_backupAeExpCompensation = 0;
    m_delay = 0;
    m_specialCaptureMode = SCAPTURE_MODE_NONE;
    m_check = false;
    m_specialCaptureStep = SCAPTURE_STEP_OFF;
    m_backupSceneMode = AA_SCENE_MODE_UNSUPPORTED;
    m_backupAaMode = AA_CONTROL_OFF;
}

ExynosCameraActivitySpecialCapture::~ExynosCameraActivitySpecialCapture()
{
    t_isExclusiveReq = false;
    t_isActivated = false;
    t_reqNum = 0x1F;
    t_reqStatus = 0;

    m_currentInputFcount = 0;
    m_backupAeExpCompensation = 0;
    m_delay = 0;
    m_specialCaptureMode = SCAPTURE_MODE_NONE;
    m_check = false;
}

int ExynosCameraActivitySpecialCapture::t_funcNull(void *args)
{
    ExynosBuffer *buf = (ExynosBuffer *)args;

    return 1;
}

int ExynosCameraActivitySpecialCapture::t_funcSensorBefore(void *args)
{
    ExynosBuffer *buf = (ExynosBuffer *)args;
    camera2_shot_ext *shot_ext = (struct camera2_shot_ext *)(buf->virt.extP[1]);

    return 1;
}

int ExynosCameraActivitySpecialCapture::t_funcSensorAfter(void *args)
{
    ExynosBuffer *buf = (ExynosBuffer *)args;
    camera2_shot_ext *shot_ext = (struct camera2_shot_ext *)(buf->virt.extP[1]);
    int ret = 1;

    return ret;
}

int ExynosCameraActivitySpecialCapture::t_funcISPBefore(void *args)
{
    ExynosBuffer *buf = (ExynosBuffer *)args;
    camera2_shot_ext *shot_ext = (struct camera2_shot_ext *)(buf->virt.extP[1]);

done:
    return 1;
}

int ExynosCameraActivitySpecialCapture::t_funcISPAfter(void *args)
{
    ExynosBuffer *buf = (ExynosBuffer *)args;
    camera2_shot_ext *shot_ext = (struct camera2_shot_ext *)(buf->virt.extP[1]);

    return 1;
}

int ExynosCameraActivitySpecialCapture::t_func3ABefore(void *args)
{
    ExynosBuffer *buf = (ExynosBuffer *)args;
    camera2_shot_ext *shot_ext = (struct camera2_shot_ext *)(buf->virt.extP[1]);
    m_currentInputFcount = shot_ext->shot.dm.request.frameCount;

    return 1;
}

int ExynosCameraActivitySpecialCapture::t_func3AAfter(void *args)
{
    ExynosBuffer *buf = (ExynosBuffer *)args;
    camera2_shot_ext *shot_ext = (struct camera2_shot_ext *)(buf->virt.extP[1]);

    ALOGV("[%s] (%d)", __func__, __LINE__);

    return 1;
}

int ExynosCameraActivitySpecialCapture::t_funcSCPBefore(void *args)
{
    ExynosBuffer *buf = (ExynosBuffer *)args;
    camera2_stream *shot_ext = (struct camera2_stream *)(buf->virt.extP[2]);

    ALOGV("[%s] (%d)", __func__, __LINE__);

    return 1;
}

int ExynosCameraActivitySpecialCapture::t_funcSCPAfter(void *args)
{
    ExynosBuffer *buf = (ExynosBuffer *)args;

    ALOGV("[%s] (%d)", __func__, __LINE__);

    return 1;
}

int ExynosCameraActivitySpecialCapture::t_funcSCCBefore(void *args)
{
    ExynosBuffer *buf = (ExynosBuffer *)args;

    ALOGV("[%s] (%d)", __func__, __LINE__);

    return 1;
}

int ExynosCameraActivitySpecialCapture::t_funcSCCAfter(void *args)
{
    ExynosBuffer *buf = (ExynosBuffer *)args;

    ALOGV("[%s] (%d)", __func__, __LINE__);

    return 1;
}

int ExynosCameraActivitySpecialCapture::setCaptureMode(enum SCAPTURE_MODE sCaptureModeVal)
{
    m_specialCaptureMode = sCaptureModeVal;

ALOGD("[%s] (%d) (%d)", __func__, __LINE__, m_specialCaptureMode);

    return 1;
}
} /* namespace android */

