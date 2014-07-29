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
#define LOG_TAG "ExynosCameraActivityFlash"
#include <cutils/log.h>

#include "ExynosCamera.h"

#include "ExynosCameraActivityFlash.h"

namespace android {

class ExynosCamera;

ExynosCameraActivityFlash::ExynosCameraActivityFlash()
{
    t_isExclusiveReq = false;
    t_isActivated = false;
    t_reqNum = 0x1F;
    t_reqStatus = 0;

    m_isNeedFlash = false;
    m_isNeedCaptureFlash = true;
    m_flashTriggerStep = 0;

    m_flashStepErrorCount = -1;

    m_checkMainCaptureRcount = false;
    m_checkMainCaptureFcount = false;

    m_waitingCount = -1;
    m_isCapture = false;
    m_timeoutCount = 0;
    m_aeWaitMaxCount = 0;

    m_flashStatus = FLASH_STATUS_OFF;
    m_flashReq = FLASH_REQ_OFF;
    m_flashStep = FLASH_STEP_OFF;
    m_ShotFcount = 0;

    m_flashPreStatus = FLASH_STATUS_OFF;
    m_aePreState = AE_STATE_INACTIVE;
    m_flashTrigger = FLASH_TRIGGER_OFF;
    m_mainWaitCount = 0;

    m_aeflashMode = AA_FLASHMODE_OFF;
    m_checkFlashStepCancel = false;
    m_mainCaptureRcount = 0;
    m_mainCaptureFcount = 0;
    m_isRecording = false;
    m_flashMode = CAM2_FLASH_MODE_OFF;
    m_currentIspInputFcount = 0;
    m_awbMode = AA_AWBMODE_OFF;
    m_aeState = AE_STATE_INACTIVE;
    m_aeMode = AA_AEMODE_OFF;
}

ExynosCameraActivityFlash::~ExynosCameraActivityFlash()
{
}

int ExynosCameraActivityFlash::t_funcNull(void *args)
{
    ExynosBuffer *buf = (ExynosBuffer *)args;
    return 1;
}

int ExynosCameraActivityFlash::t_funcSensorBefore(void *args)
{
    ExynosBuffer *buf = (ExynosBuffer *)args;
    m_reqBuf = *buf;

    return 1;
}

int ExynosCameraActivityFlash::t_funcSensorAfter(void *args)
{
    ExynosBuffer *buf = (ExynosBuffer *)args;
    camera2_shot_ext *shot_ext = (struct camera2_shot_ext *)(buf->virt.extP[1]);

    if (m_checkMainCaptureFcount == true) {
        /* Update m_waitingCount */
        m_waitingCount = checkMainCaptureFcount(shot_ext->shot.dm.request.frameCount);
        ALOGV("[%s] (%d) (0x%x)", __func__, __LINE__, m_waitingCount);
    }

    return 1;
}

int ExynosCameraActivityFlash::t_funcISPBefore(void *args)
{
    ExynosBuffer *buf = (ExynosBuffer *)args;

    return 1;
}

int ExynosCameraActivityFlash::t_funcISPAfter(void *args)
{
    ExynosBuffer *buf = (ExynosBuffer *)args;

    return 1;
}

int ExynosCameraActivityFlash::t_func3ABefore(void *args)
{
    ExynosBuffer *buf = (ExynosBuffer *)args;

    camera2_shot_ext *shot_ext = (struct camera2_shot_ext *)(buf->virt.extP[1]);

    m_currentIspInputFcount = shot_ext->shot.dm.request.frameCount;

    ALOGV("[%s] (%d) m_flashReq %d m_flashStatus %d m_flashStep %d", __func__, __LINE__, (int)m_flashReq, (int)m_flashStatus, (int)m_flashStep);

    if ((m_flashPreStatus != m_flashStatus) ||(m_aePreState != m_aeState)) {
        ALOGD("[%s] (%d) m_flashReq %d m_flashStatus %d m_flashStep %d m_aeState %d",
                    __func__, __LINE__,
                    (int)m_flashReq, (int)m_flashStatus, (int)m_flashStep, (int)m_aeState);

        m_flashPreStatus = m_flashStatus;
        m_aePreState = m_aeState;
    }

    if (m_flashStep == FLASH_STEP_CANCEL) {
        m_isNeedFlash = false;

        shot_ext->shot.ctl.aa.aeflashMode = AA_FLASHMODE_CANCLE;
        shot_ext->shot.ctl.flash.flashMode = CAM2_FLASH_MODE_OFF;
        shot_ext->shot.ctl.flash.firingTime = 0;
        shot_ext->shot.ctl.flash.firingPower = 0;
        shot_ext->shot.ctl.aa.aeMode = m_aeMode;
        shot_ext->shot.ctl.aa.awbMode = m_awbMode;

        m_waitingCount = -1;
        m_flashStepErrorCount = -1;

        m_checkMainCaptureRcount = false;
        m_checkMainCaptureFcount = false;
        m_checkFlashStepCancel = false;
        m_isCapture = false;

        goto done;
    }

    if (m_flashReq == FLASH_REQ_OFF) {
        m_isNeedFlash = false;

        shot_ext->shot.ctl.aa.aeflashMode = AA_FLASHMODE_OFF;
        shot_ext->shot.ctl.flash.flashMode = CAM2_FLASH_MODE_OFF;
        shot_ext->shot.ctl.flash.firingTime = 0;
        shot_ext->shot.ctl.flash.firingPower = 0;

        m_waitingCount = -1;
        m_flashStepErrorCount = -1;
        m_flashStep = FLASH_STEP_OFF;

        m_checkMainCaptureRcount = false;
        m_checkMainCaptureFcount = false;

        goto done;
    } else if (m_flashReq == FLASH_REQ_TORCH) {
        m_isNeedFlash = true;

        shot_ext->shot.ctl.aa.aeflashMode = AA_FLASHMODE_ON_ALWAYS;
        shot_ext->shot.ctl.flash.flashMode = CAM2_FLASH_MODE_TORCH;
        shot_ext->shot.ctl.flash.firingTime = 50L * 1000L; /* 1sec */
        shot_ext->shot.ctl.flash.firingPower = 63; /* 63 is max */

        m_waitingCount = -1;

        goto done;
    } else if (m_flashReq == FLASH_REQ_ON) {
        m_isNeedFlash = true;

        if (m_flashStatus == FLASH_STATUS_OFF || m_flashStatus == FLASH_STATUS_PRE_CHECK) {
            shot_ext->shot.ctl.aa.aeflashMode = AA_FLASHMODE_OFF;
            shot_ext->shot.ctl.flash.flashMode = CAM2_FLASH_MODE_OFF;
            shot_ext->shot.ctl.flash.firingTime = 0;
            shot_ext->shot.ctl.flash.firingPower = 0;

            m_flashStatus = FLASH_STATUS_PRE_READY;
        } else if (m_flashStatus == FLASH_STATUS_PRE_READY) {
            if (m_flashStep == FLASH_STEP_PRE_START) {
                shot_ext->shot.ctl.aa.aeflashMode = AA_FLASHMODE_START;
                shot_ext->shot.ctl.flash.flashMode = CAM2_FLASH_MODE_OFF;
                shot_ext->shot.ctl.flash.firingTime = 0;
                shot_ext->shot.ctl.flash.firingPower = 0;

                shot_ext->shot.ctl.aa.aeMode = m_aeMode;
                shot_ext->shot.ctl.aa.awbMode = AA_AWBMODE_LOCKED;

                m_flashStatus = FLASH_STATUS_PRE_ON;
                m_aeWaitMaxCount--;
            }
        } else if (m_flashStatus == FLASH_STATUS_PRE_ON) {
            shot_ext->shot.ctl.aa.aeflashMode = AA_FLASHMODE_ON;
            shot_ext->shot.ctl.flash.flashMode = CAM2_FLASH_MODE_TORCH;
            shot_ext->shot.ctl.flash.firingTime = 10 * 1000L * 1000L; /* 10sec */
            shot_ext->shot.ctl.flash.firingPower = 63; /* 63 is max */
            shot_ext->shot.ctl.aa.aeMode = m_aeMode;

            m_flashStatus = FLASH_STATUS_PRE_ON;
            m_aeWaitMaxCount--;
        } else if (m_flashStatus == FLASH_STATUS_PRE_AE_DONE) {
            shot_ext->shot.ctl.aa.aeflashMode = AA_FLASHMODE_ON;
            shot_ext->shot.ctl.flash.flashMode = CAM2_FLASH_MODE_TORCH;
            shot_ext->shot.ctl.flash.firingTime = 10 * 1000L * 1000L; /* 10sec */
            shot_ext->shot.ctl.flash.firingPower = 63; /* 63 is max */
            shot_ext->shot.ctl.aa.aeMode = AA_AEMODE_LOCKED;
            shot_ext->shot.ctl.aa.awbMode = AA_AWBMODE_LOCKED;

            m_flashStatus = FLASH_STATUS_PRE_AE_DONE;
            m_aeWaitMaxCount = 0;
            /* AE AWB LOCK */
        } else if (m_flashStatus == FLASH_STATUS_PRE_AF) {
            shot_ext->shot.ctl.aa.aeflashMode = AA_FLASHMODE_ON;
            shot_ext->shot.ctl.flash.flashMode = CAM2_FLASH_MODE_TORCH;
            shot_ext->shot.ctl.flash.firingTime = 10 * 1000L * 1000L; /* 10sec */
            shot_ext->shot.ctl.flash.firingPower = 63; /* 63 is max */
            shot_ext->shot.ctl.aa.aeMode = AA_AEMODE_LOCKED;
            shot_ext->shot.ctl.aa.awbMode = AA_AWBMODE_LOCKED;

            m_flashStatus = FLASH_STATUS_PRE_AF;
            m_aeWaitMaxCount = 0;
            /*
        } else if (m_flashStatus == FLASH_STATUS_PRE_AF_DONE) {
            shot_ext->shot.ctl.aa.aeflashMode = AA_FLASHMODE_AUTO;
            shot_ext->shot.ctl.aa.awbMode = AA_AWBMODE_LOCKED;

            m_waitingCount = -1;
            m_aeWaitMaxCount = 0;
            */
        } else if (m_flashStatus == FLASH_STATUS_PRE_DONE) {
            if (m_flashTrigger == FLASH_TRIGGER_TOUCH_DISPLAY) {
                shot_ext->shot.ctl.aa.aeMode = m_aeMode;
                shot_ext->shot.ctl.aa.awbMode = m_awbMode;
            }
            shot_ext->shot.ctl.aa.aeflashMode = AA_FLASHMODE_AUTO;
            shot_ext->shot.ctl.flash.flashMode = CAM2_FLASH_MODE_OFF;
            shot_ext->shot.ctl.flash.firingTime = 0;
            shot_ext->shot.ctl.flash.firingPower = 0;

            m_waitingCount = -1;
            m_aeWaitMaxCount = 0;
        } else if (m_flashStatus == FLASH_STATUS_MAIN_READY) {
            if (m_flashStep == FLASH_STEP_MAIN_START) {
                ALOGD("[%s] (%d) Main Flash On %d", __func__, __LINE__, shot_ext->shot.dm.request.frameCount);
                shot_ext->shot.ctl.aa.aeflashMode = AA_FLASHMODE_CAPTURE;
                shot_ext->shot.ctl.flash.flashMode = CAM2_FLASH_MODE_SINGLE;
                shot_ext->shot.ctl.flash.firingTime = 500L * 1000L; /* 1sec */
                shot_ext->shot.ctl.flash.firingPower = 63; /* 63 is max */

                m_flashStatus = FLASH_STATUS_MAIN_ON;

                m_waitingCount--;
                m_aeWaitMaxCount = 0;
            }
        } else if (m_flashStatus == FLASH_STATUS_MAIN_ON) {
            ALOGD("[%s] (%d) FLASH_STATUS_MAIN_ON %d", __func__, __LINE__, shot_ext->shot.dm.request.frameCount);
            shot_ext->shot.ctl.aa.aeflashMode = AA_FLASHMODE_CAPTURE;
            shot_ext->shot.ctl.flash.flashMode = CAM2_FLASH_MODE_SINGLE;
            shot_ext->shot.ctl.flash.firingTime = 500L * 1000L; /* 1sec */
            shot_ext->shot.ctl.flash.firingPower = 63; /* 63 is max */

            m_flashStatus = FLASH_STATUS_MAIN_ON;
            m_waitingCount--;
            m_aeWaitMaxCount = 0;
        } else if (m_flashStatus == FLASH_STATUS_MAIN_WAIT) {
            ALOGD("[%s] (%d) FLASH_STATUS_MAIN_WAIT %d", __func__, __LINE__, shot_ext->shot.dm.request.frameCount);
            shot_ext->shot.ctl.aa.aeflashMode = AA_FLASHMODE_CAPTURE;
            shot_ext->shot.ctl.flash.flashMode = CAM2_FLASH_MODE_SINGLE;
            shot_ext->shot.ctl.flash.firingTime = 500L * 1000L; /* 1sec */
            shot_ext->shot.ctl.flash.firingPower = 63; /* 63 is max */

            m_flashStatus = FLASH_STATUS_MAIN_WAIT;
            m_waitingCount--;
            m_aeWaitMaxCount = 0;
        } else if (m_flashStatus == FLASH_STATUS_MAIN_DONE) {
            ALOGD("[%s] (%d) Main Flash Off %d", __func__, __LINE__, shot_ext->shot.dm.request.frameCount);
            shot_ext->shot.ctl.aa.aeflashMode = AA_FLASHMODE_OFF;
            shot_ext->shot.ctl.flash.flashMode = CAM2_FLASH_MODE_OFF;
            shot_ext->shot.ctl.flash.firingTime = 0;
            shot_ext->shot.ctl.flash.firingPower = 0;

            m_flashStatus = FLASH_STATUS_OFF;
            m_waitingCount = -1;

            m_aeWaitMaxCount = 0;
        }
    } else if (m_flashReq == FLASH_REQ_AUTO) {
        if (m_aeState == AE_STATE_INACTIVE) {
            m_isNeedFlash = false;

            shot_ext->shot.ctl.aa.aeflashMode = AA_FLASHMODE_OFF;
            shot_ext->shot.ctl.flash.flashMode = CAM2_FLASH_MODE_OFF;
            shot_ext->shot.ctl.flash.firingTime = 0;
            shot_ext->shot.ctl.flash.firingPower = 0;

            m_flashStatus = FLASH_STATUS_OFF;
            m_flashStep = FLASH_STEP_OFF;

            m_checkMainCaptureRcount = false;
            m_checkMainCaptureFcount = false;
            m_waitingCount = -1;

            goto done;
        } else if (m_aeState == AE_STATE_CONVERGED) {
            m_isNeedFlash = false;

            shot_ext->shot.ctl.aa.aeflashMode = AA_FLASHMODE_OFF;
            shot_ext->shot.ctl.flash.flashMode = CAM2_FLASH_MODE_OFF;
            shot_ext->shot.ctl.flash.firingTime = 0;
            shot_ext->shot.ctl.flash.firingPower = 0;

            m_flashStatus = FLASH_STATUS_OFF;
            m_flashStep = FLASH_STEP_OFF;

            m_checkMainCaptureRcount = false;
            m_checkMainCaptureFcount = false;
            m_waitingCount = -1;

            goto done;
        } else if (m_aeState == AE_STATE_FLASH_REQUIRED) {
            m_isNeedFlash = true;

            if (m_flashStatus == FLASH_STATUS_OFF || m_flashStatus == FLASH_STATUS_PRE_CHECK) {
                shot_ext->shot.ctl.aa.aeflashMode = AA_FLASHMODE_OFF;
                shot_ext->shot.ctl.flash.flashMode = CAM2_FLASH_MODE_OFF;
                shot_ext->shot.ctl.flash.firingTime = 0;
                shot_ext->shot.ctl.flash.firingPower = 0;

                shot_ext->shot.ctl.aa.aeMode = m_aeMode;

                m_flashStatus = FLASH_STATUS_PRE_READY;
            } else if (m_flashStatus == FLASH_STATUS_PRE_READY) {
                if (m_flashStep == FLASH_STEP_PRE_START) {
                    shot_ext->shot.ctl.aa.aeflashMode = AA_FLASHMODE_START;
                    shot_ext->shot.ctl.flash.flashMode = CAM2_FLASH_MODE_OFF;
                    shot_ext->shot.ctl.flash.firingTime = 0;
                    shot_ext->shot.ctl.flash.firingPower = 0;
                    shot_ext->shot.ctl.aa.aeMode = m_aeMode;
                    shot_ext->shot.ctl.aa.awbMode = AA_AWBMODE_LOCKED;

                    m_flashStatus = FLASH_STATUS_PRE_ON;
                    m_aeWaitMaxCount--;
                }
            } else if (m_flashStatus == FLASH_STATUS_PRE_ON) {
                shot_ext->shot.ctl.aa.aeflashMode = AA_FLASHMODE_ON;
                shot_ext->shot.ctl.flash.flashMode = CAM2_FLASH_MODE_TORCH;
                shot_ext->shot.ctl.flash.firingTime = 10 * 1000L * 1000L; /* 10sec */
                shot_ext->shot.ctl.flash.firingPower = 63; /* 63 is max */
                shot_ext->shot.ctl.aa.aeMode = m_aeMode;

                m_flashStatus = FLASH_STATUS_PRE_ON;
                m_aeWaitMaxCount--;
            } else if (m_flashStatus == FLASH_STATUS_PRE_AE_DONE) {
                shot_ext->shot.ctl.aa.aeflashMode = AA_FLASHMODE_ON;
                shot_ext->shot.ctl.flash.flashMode = CAM2_FLASH_MODE_TORCH;
                shot_ext->shot.ctl.flash.firingTime = 10 * 1000L * 1000L; /* 10sec */
                shot_ext->shot.ctl.flash.firingPower = 63; /* 63 is max */
                shot_ext->shot.ctl.aa.aeMode = AA_AEMODE_LOCKED;
                shot_ext->shot.ctl.aa.awbMode = AA_AWBMODE_LOCKED;

                m_flashStatus = FLASH_STATUS_PRE_AE_DONE;
                m_aeWaitMaxCount = 0;
                /* AE AWB LOCK */
            } else if (m_flashStatus == FLASH_STATUS_PRE_AF) {
                shot_ext->shot.ctl.aa.aeflashMode = AA_FLASHMODE_ON;
                shot_ext->shot.ctl.flash.flashMode = CAM2_FLASH_MODE_TORCH;
                shot_ext->shot.ctl.flash.firingTime = 10 * 1000L * 1000L; /* 10sec */
                shot_ext->shot.ctl.flash.firingPower = 63; /* 63 is max */
                shot_ext->shot.ctl.aa.aeMode = AA_AEMODE_LOCKED;
                shot_ext->shot.ctl.aa.awbMode = AA_AWBMODE_LOCKED;

                m_flashStatus = FLASH_STATUS_PRE_AF;
                m_aeWaitMaxCount = 0;
                /*
            } else if (m_flashStatus == FLASH_STATUS_PRE_AF_DONE) {
                shot_ext->shot.ctl.aa.aeflashMode = AA_FLASHMODE_AUTO;
                shot_ext->shot.ctl.aa.awbMode = AA_AWBMODE_LOCKED;

                m_waitingCount = -1;
                m_aeWaitMaxCount = 0;
                */
            } else if (m_flashStatus == FLASH_STATUS_PRE_DONE) {
                if (m_flashTrigger == FLASH_TRIGGER_TOUCH_DISPLAY) {
                    shot_ext->shot.ctl.aa.aeMode = m_aeMode;
                    shot_ext->shot.ctl.aa.awbMode = m_awbMode;
                }
                shot_ext->shot.ctl.aa.aeflashMode = AA_FLASHMODE_AUTO;
                shot_ext->shot.ctl.flash.flashMode = CAM2_FLASH_MODE_OFF;
                shot_ext->shot.ctl.flash.firingTime = 0;
                shot_ext->shot.ctl.flash.firingPower = 0;

                m_waitingCount = -1;
                m_aeWaitMaxCount = 0;
            } else if (m_flashStatus == FLASH_STATUS_MAIN_READY) {
                if (m_flashStep == FLASH_STEP_MAIN_START) {
                    ALOGD("[%s] (%d) Main Flash On %d", __func__, __LINE__, shot_ext->shot.dm.request.frameCount);
                    shot_ext->shot.ctl.aa.aeflashMode = AA_FLASHMODE_CAPTURE;
                    shot_ext->shot.ctl.flash.flashMode = CAM2_FLASH_MODE_SINGLE;
                    shot_ext->shot.ctl.flash.firingTime = 500L * 1000L; /* 1sec */
                    shot_ext->shot.ctl.flash.firingPower = 63; /* 63 is max */

                    m_flashStatus = FLASH_STATUS_MAIN_ON;

                    m_waitingCount--;
                    m_aeWaitMaxCount = 0;
                }
            } else if (m_flashStatus == FLASH_STATUS_MAIN_ON) {
                shot_ext->shot.ctl.aa.aeflashMode = AA_FLASHMODE_CAPTURE;
                shot_ext->shot.ctl.flash.flashMode = CAM2_FLASH_MODE_SINGLE;
                shot_ext->shot.ctl.flash.firingTime = 500L * 1000L; /* 1sec */
                shot_ext->shot.ctl.flash.firingPower = 63; /* 63 is max */

                m_flashStatus = FLASH_STATUS_MAIN_ON;
                m_waitingCount--;
                m_aeWaitMaxCount = 0;
            } else if (m_flashStatus == FLASH_STATUS_MAIN_WAIT) {
                shot_ext->shot.ctl.aa.aeflashMode = AA_FLASHMODE_CAPTURE;
                shot_ext->shot.ctl.flash.flashMode = CAM2_FLASH_MODE_SINGLE;
                shot_ext->shot.ctl.flash.firingTime = 500L * 1000L; /* 1sec */
                shot_ext->shot.ctl.flash.firingPower = 63; /* 63 is max */

                m_flashStatus = FLASH_STATUS_MAIN_WAIT;
                m_waitingCount--;
                m_aeWaitMaxCount = 0;
            } else if (m_flashStatus == FLASH_STATUS_MAIN_DONE) {
                ALOGD("[%s] (%d) Main Flash Off %d", __func__, __LINE__, shot_ext->shot.dm.request.frameCount);
                shot_ext->shot.ctl.aa.aeflashMode = AA_FLASHMODE_OFF;
                shot_ext->shot.ctl.flash.flashMode = CAM2_FLASH_MODE_OFF;
                shot_ext->shot.ctl.flash.firingTime = 0;
                shot_ext->shot.ctl.flash.firingPower = 0;

                m_flashStatus = FLASH_STATUS_OFF;
                m_waitingCount = -1;

                m_aeWaitMaxCount = 0;
            }
        }
    }

    if (0 < m_flashStepErrorCount)
        m_flashStepErrorCount++;

    ALOGV("[%s] (%d)(%d)", __func__, __LINE__, (int)shot_ext->shot.ctl.aa.aeflashMode);

done:
    return 1;
}

int ExynosCameraActivityFlash::t_func3AAfter(void *args)
{
    ExynosBuffer *buf = (ExynosBuffer *)args;
    camera2_shot_ext *shot_ext = (struct camera2_shot_ext *)(buf->virt.extP[1]);

    if (m_isCapture == false)
        m_aeState = shot_ext->shot.dm.aa.aeState;

    if (m_flashStep == FLASH_STEP_CANCEL &&
        m_checkFlashStepCancel == false) {
        m_flashStep = FLASH_STEP_OFF;
        m_flashStatus = FLASH_STATUS_OFF;

        goto done;
    }

    if (m_flashStatus == FLASH_STATUS_PRE_CHECK) {
        if (shot_ext->shot.dm.flash.decision == 2 ||
            FLASH_TIMEOUT_COUNT < m_timeoutCount) {
            m_flashStatus = FLASH_STATUS_PRE_READY;
            m_timeoutCount = 0;
        } else {
            m_timeoutCount++;
        }
    } else if (m_flashStatus == FLASH_STATUS_PRE_ON) {
        if (shot_ext->shot.dm.flash.flashReady == 1 ||
            FLASH_AE_TIMEOUT_COUNT < m_timeoutCount) {
            if (FLASH_AE_TIMEOUT_COUNT < m_timeoutCount)
                ALOGD("[%s] (%d) auto exposure timeoutCount %d", __func__, __LINE__, m_timeoutCount);
            m_flashStatus = FLASH_STATUS_PRE_AE_DONE;
            m_timeoutCount = 0;
        } else {
            m_timeoutCount++;
        }
    } else if (m_flashStatus == FLASH_STATUS_PRE_AE_DONE) {
        m_flashStatus = FLASH_STATUS_PRE_AF;
    } else if (m_flashStatus == FLASH_STATUS_PRE_AF) {
        if (m_flashStep == FLASH_STEP_PRE_DONE ||
            FLASH_AF_TIMEOUT_COUNT < m_timeoutCount) {
            if (FLASH_AF_TIMEOUT_COUNT < m_timeoutCount)
                ALOGD("[%s] (%d) auto focus timeoutCount %d", __func__, __LINE__, m_timeoutCount);
            m_flashStatus = FLASH_STATUS_PRE_DONE;
            m_timeoutCount = 0;
        } else {
            m_timeoutCount++;
        }
    /*
    } else if (m_flashStatus == FLASH_STATUS_PRE_AF_DONE) {
        if (shot_ext->shot.dm.flash.flashOffReady == 1 ||
            FLASH_TIMEOUT_COUNT < m_timeoutCount) {
            if (shot_ext->shot.dm.flash.flashOffReady == 1)
                ALOGD("[%s] (%d) flashOffReady == 1 frameCount %d", __func__, __LINE__, shot_ext->shot.dm.request.frameCount);

            m_flashStatus = FLASH_STATUS_PRE_DONE;
            m_timeoutCount = 0;
        } else {
            m_timeoutCount++;
        }
        */
    } else if (m_flashStatus == FLASH_STATUS_PRE_DONE) {
        if (shot_ext->shot.dm.flash.flashReady == 2 ||
            FLASH_TIMEOUT_COUNT < m_timeoutCount) {
            if (shot_ext->shot.dm.flash.flashReady == 2) {
                ALOGD("[%s] (%d) flashReady == 2 frameCount %d", __func__, __LINE__, shot_ext->shot.dm.request.frameCount);
            } else if (FLASH_MAIN_TIMEOUT_COUNT < m_timeoutCount) {
                ALOGD("[%s] (%d) m_timeoutCount %d", __func__, __LINE__, m_timeoutCount);
            }

            m_flashStatus = FLASH_STATUS_MAIN_READY;
            m_timeoutCount = 0;
        } else {
            m_timeoutCount++;
        }
    } else if (m_flashStatus == FLASH_STATUS_MAIN_READY) {
        if (m_flashTrigger == FLASH_TRIGGER_TOUCH_DISPLAY) {
            if (FLASH_TIMEOUT_COUNT < m_timeoutCount) {
                if (FLASH_TIMEOUT_COUNT < m_timeoutCount) {
                    ALOGD("[%s] (%d) m_timeoutCount %d", __func__, __LINE__, m_timeoutCount);
                }

                m_flashStep = FLASH_STEP_OFF;
                m_flashStatus = FLASH_STATUS_OFF;
                //m_flashTrigger = FLASH_TRIGGER_OFF;
                m_isCapture = false;

                m_waitingCount = -1;
                m_checkMainCaptureFcount = false;
                m_timeoutCount = 0;
            } else {
                m_timeoutCount++;
            }
        }
    } else if (m_flashStatus == FLASH_STATUS_MAIN_ON) {
        if ((shot_ext->shot.dm.flash.flashOffReady == 2) ||
            (shot_ext->shot.dm.flash.firingStable == 1) ||
            FLASH_MAIN_TIMEOUT_COUNT < m_timeoutCount) {

            if (shot_ext->shot.dm.flash.flashOffReady == 2) {
                ALOGD("[%s] (%d) flashOffReady %d", __func__, __LINE__, shot_ext->shot.dm.flash.flashOffReady);
            } else if (shot_ext->shot.dm.flash.firingStable == 1) {
                m_ShotFcount = shot_ext->shot.dm.request.frameCount;
                ALOGD("[%s] (%d) m_ShotFcount %u", __func__, __LINE__, m_ShotFcount);
            } else if (FLASH_MAIN_TIMEOUT_COUNT < m_timeoutCount) {
                ALOGD("[%s] (%d) m_timeoutCount %d", __func__, __LINE__, m_timeoutCount);
            }
            ALOGD("[%s] (%d) frameCount %d" ,__func__, __LINE__, shot_ext->shot.dm.request.frameCount);

            m_flashStatus = FLASH_STATUS_MAIN_DONE;
            m_timeoutCount = 0;
            m_mainWaitCount = 0;

            m_waitingCount--;
        } else {
            m_timeoutCount++;
        }
    } else if (m_flashStatus == FLASH_STATUS_MAIN_WAIT) {
        /* 1 frame is used translate status MAIN_ON to MAIN_WAIT */
        if (m_mainWaitCount < FLASH_MAIN_WAIT_COUNT -1) {
            ALOGD("[%s] (%d) %d", __func__, __LINE__, shot_ext->shot.dm.request.frameCount);
            m_mainWaitCount ++;
        } else {
            ALOGD("[%s] (%d) %d", __func__, __LINE__, shot_ext->shot.dm.request.frameCount);
            m_mainWaitCount = 0;
            m_waitingCount = -1;
            m_flashStatus = FLASH_STATUS_MAIN_DONE;
        }
    }

    ALOGV("[%s] (%d) (m_aeState %d)(m_flashStatus %d)", __func__, __LINE__, (int)m_aeState, (int)m_flashStatus);
    ALOGV("[%s] (%d) (decision %d flashReady %d flashOffReady %d firingStable %d)", __func__, __LINE__,
            (int)shot_ext->shot.dm.flash.decision,
            (int)shot_ext->shot.dm.flash.flashReady,
            (int)shot_ext->shot.dm.flash.flashOffReady,
            (int)shot_ext->shot.dm.flash.firingStable);

    m_aeflashMode = shot_ext->shot.dm.aa.aeflashMode;

done:
    return 1;
}

int ExynosCameraActivityFlash::t_funcSCPBefore(void *args)
{
    ExynosBuffer *buf = (ExynosBuffer *)args;
    camera2_shot_ext *shot_ext = (struct camera2_shot_ext *)(buf->virt.extP[1]);

    return 1;
}

int ExynosCameraActivityFlash::t_funcSCPAfter(void *args)
{
    ExynosBuffer *buf = (ExynosBuffer *)args;

    return 1;
}

int ExynosCameraActivityFlash::t_funcSCCBefore(void *args)
{
    ExynosBuffer *buf = (ExynosBuffer *)args;

    return 1;
}

int ExynosCameraActivityFlash::t_funcSCCAfter(void *args)
{
    ExynosBuffer *buf = (ExynosBuffer *)args;

    return 1;
}

bool ExynosCameraActivityFlash::checkSensorBuf(ExynosBuffer *buf)
{
    return true;
}

bool ExynosCameraActivityFlash::setFlashMode(enum flash_mode flashModeVal,
        enum aa_ae_flashmode aeflashModeVal,
        enum aa_aemode aeModeVal)
{
    m_flashMode = flashModeVal;
    m_aeflashMode = aeflashModeVal;
    m_aeMode = aeModeVal;

    return true;
}

bool ExynosCameraActivityFlash::setFlashReq(enum FLASH_REQ flashReqVal)
{
    if (m_flashReq != flashReqVal) {
        m_flashReq = flashReqVal;
        setFlashStatus(ExynosCameraActivityFlash::FLASH_STATUS_OFF);
        if (m_isRecording == false)
            m_isNeedCaptureFlash = true;
    }

    if (m_flashReq == FLASH_REQ_ON)
        m_isNeedFlash = true;
    if (m_flashReq == FLASH_REQ_TORCH)
        m_isNeedCaptureFlash = false;

    ALOGD("[%s] (%d)(%d)", __func__, __LINE__, (int)m_flashReq);
    return true;
}

bool ExynosCameraActivityFlash::setFlashStatus(enum FLASH_STATUS flashStatusVal)
{
    m_flashStatus = flashStatusVal;
    ALOGV("[%s] (%d)(%d)", __func__, __LINE__, (int)m_flashStatus);
    return true;
}

int ExynosCameraActivityFlash::getFlashStatus()
{
    return m_aeflashMode;
}

bool ExynosCameraActivityFlash::setFlashExposure(enum aa_aemode aeModeVal)
{
	m_aeMode = aeModeVal;
	ALOGV("[%s] (%d)(%d)", __func__, __LINE__, (int)m_aeMode);
	return true;
}

bool ExynosCameraActivityFlash::setFlashWhiteBalance(enum aa_awbmode wbModeVal)
{
    m_awbMode = wbModeVal;
    ALOGV("[%s] (%d)(%d)", __func__, __LINE__, (int)m_awbMode);
    return true;
}

bool ExynosCameraActivityFlash::setFlashStep(enum FLASH_STEP flashStepVal)
{
    m_flashStep = flashStepVal;

    /* trigger events */
    switch (flashStepVal) {
    case FLASH_STEP_OFF:
        m_waitingCount = -1;
        m_checkMainCaptureFcount = false;
        m_flashStatus = FLASH_STATUS_OFF;
        break;
    case FLASH_STEP_PRE_START:
        m_aeWaitMaxCount = 25;
        if ((m_flashStatus == FLASH_STATUS_PRE_DONE || m_flashStatus == FLASH_STATUS_MAIN_READY) &&
            m_flashTrigger == FLASH_TRIGGER_TOUCH_DISPLAY)
            m_flashStatus = FLASH_STATUS_OFF;
        break;
    case FLASH_STEP_PRE_DONE:
        break;
    case FLASH_STEP_MAIN_START:
        setShouldCheckedFcount(m_currentIspInputFcount + CAPTURE_SKIP_COUNT);

        m_waitingCount = 15;
        m_timeoutCount = 0;
        m_checkMainCaptureFcount = false;
        break;
    case FLASH_STEP_MAIN_DONE:
        m_waitingCount = -1;
        m_checkMainCaptureFcount = false;
        break;
    case FLASH_STEP_CANCEL:
        m_checkFlashStepCancel = true;
        m_isNeedCaptureFlash = true;
        break;
    case FLASH_STEP_END:
        break;
    default:
        break;
    }

    ALOGD("[%s] (%d) (%d)", __func__, __LINE__, (int)flashStepVal);

    if (flashStepVal != FLASH_STEP_OFF)
        m_flashStepErrorCount = 0;

    return true;
}

bool ExynosCameraActivityFlash::getFlashStep(enum FLASH_STEP *flashStepVal)
{
    *flashStepVal = m_flashStep;

    return true;
}

bool ExynosCameraActivityFlash::setFlashTrigerPath(enum FLASH_TRIGGER flashTriggerVal)
{
    m_flashTrigger = flashTriggerVal;

    ALOGD("[%s] (%d) (%d)", __func__, __LINE__, (int)flashTriggerVal);

    return true;
}

bool ExynosCameraActivityFlash::getFlashTrigerPath(enum FLASH_TRIGGER *flashTriggerVal)
{
    *flashTriggerVal = m_flashTrigger;

    return true;
}

bool ExynosCameraActivityFlash::setShouldCheckedRcount(int rcount)
{
    m_mainCaptureRcount = rcount;
    ALOGV("[%s] (%d) (%d)", __func__, __LINE__, m_mainCaptureRcount);

    return true;
}

bool ExynosCameraActivityFlash::waitAeDone(void)
{
    bool ret = true;

    int status = 0;
    unsigned int totalWaitingTime = 0;

    while (status == 0 && totalWaitingTime <= FLASH_MAX_AEDONE_WAITING_TIME) {
        if (m_flashStatus == FLASH_STATUS_PRE_ON || m_flashStep == FLASH_STEP_PRE_START) {
            if ((m_aeWaitMaxCount <= 0) || (m_flashStatus == FLASH_STATUS_PRE_AE_DONE)) {
                status = 1;
                break;
            } else {
                ALOGV("[%s] (%d) (%d)", __func__, __LINE__, m_aeWaitMaxCount);
                status = 0;
            }
        } else {
            status = 1;
            break;
        }

        usleep(FLASH_WAITING_SLEEP_TIME);
        totalWaitingTime += FLASH_WAITING_SLEEP_TIME;
    }

    if (status == 0 || FLASH_MAX_AEDONE_WAITING_TIME < totalWaitingTime) {
        ALOGW("[%s]:waiting too much (%d msec)", __func__, totalWaitingTime);
        ret = false;
    }

    return ret;
}

bool ExynosCameraActivityFlash::waitPreDone(void)
{
    bool ret = true;
    unsigned int totalWaitingTime = 0;

    ALOGV("[%s] (%d)", __func__, __LINE__);

    while (m_flashStatus < FLASH_STATUS_PRE_DONE && totalWaitingTime <= FLASH_MAX_PRE_DONE_WAITING_TIME) {
        usleep(FLASH_WAITING_SLEEP_TIME);
        totalWaitingTime += FLASH_WAITING_SLEEP_TIME;
    }

    if (m_flashStatus < FLASH_STATUS_PRE_DONE || FLASH_MAX_PRE_DONE_WAITING_TIME < totalWaitingTime) {
        ALOGW("[%s]:waiting too much (%d msec)", __func__, totalWaitingTime);
        ret = false;
    }

    return ret;
}

bool ExynosCameraActivityFlash::waitMainReady(void)
{
    bool ret = true;
    unsigned int totalWaitingTime = 0;

    ALOGV("[%s] (%d)", __func__, __LINE__);

    while (m_flashStatus < FLASH_STATUS_MAIN_READY && totalWaitingTime <= FLASH_MAX_WAITING_TIME) {
        usleep(FLASH_WAITING_SLEEP_TIME);
        totalWaitingTime += FLASH_WAITING_SLEEP_TIME;
    }

    if (m_flashStatus < FLASH_STATUS_MAIN_READY || FLASH_MAX_WAITING_TIME < totalWaitingTime) {
        ALOGW("[%s]:waiting too much (%d msec)", __func__, totalWaitingTime);
        m_flashStatus = FLASH_STATUS_MAIN_READY;
        ret = false;
    }

    return ret;
}

bool ExynosCameraActivityFlash::waitMainReadyShortTouch(void)
{
    bool ret = true;
    unsigned int totalWaitingTime = 0;

    ALOGV("[%s] (%d)", __func__, __LINE__);

    while (m_flashStatus < FLASH_STATUS_MAIN_READY && totalWaitingTime <= FLASH_MAX_WAITING_TIME * 4) {
        usleep(FLASH_WAITING_SLEEP_TIME);
        totalWaitingTime += FLASH_WAITING_SLEEP_TIME;
    }

    if (m_flashStatus < FLASH_STATUS_MAIN_READY || FLASH_MAX_WAITING_TIME * 4 < totalWaitingTime) {
        ALOGW("[%s]:waiting too much (%d msec)", __func__, totalWaitingTime);
        m_flashStatus = FLASH_STATUS_MAIN_READY;
        ret = false;
    }

    return ret;
}

int ExynosCameraActivityFlash::checkMainCaptureRcount(int rcount)
{
    if (rcount == m_mainCaptureRcount)
        return 0;
    else if (rcount < m_mainCaptureRcount)
        return 1;
    else
        return -1;
}

bool ExynosCameraActivityFlash::setShouldCheckedFcount(int fcount)
{
    m_mainCaptureFcount = fcount;
    ALOGV("[%s] (%d) (%d)", __func__, __LINE__, m_mainCaptureFcount);

    return true;
}

int ExynosCameraActivityFlash::checkMainCaptureFcount(int fcount)
{
    ALOGV("[%s] (%d) (%d)(%d)", __func__, __LINE__, m_mainCaptureFcount, fcount);

    if (fcount < m_mainCaptureFcount)
        return (m_mainCaptureFcount - fcount); /* positive count  */
    else
        return 0;
}

int ExynosCameraActivityFlash::getWaitingCount()
{
    return m_waitingCount;
}

bool ExynosCameraActivityFlash::getNeedFlash()
{
    ALOGD("m_isNeedFlash %d", m_isNeedFlash);
    return m_isNeedFlash;
}

bool ExynosCameraActivityFlash::getNeedCaptureFlash()
{
    ALOGD("m_isNeedCaptureFlash %d", m_isNeedCaptureFlash);
    if (m_isNeedFlash == true)
        return m_isNeedCaptureFlash;

    return false;
}

unsigned int ExynosCameraActivityFlash::getShotFcount()
{
    return m_ShotFcount;
}
void ExynosCameraActivityFlash::resetShotFcount(void)
{
    m_ShotFcount = 0;
}
void ExynosCameraActivityFlash::setCaptureStatus(bool isCapture)
{
    m_isCapture = isCapture;
}

bool ExynosCameraActivityFlash::setRecordingHint(bool hint)
{
    m_isRecording = hint;
    if (m_isRecording == true)
        m_isNeedCaptureFlash = false;
    else
        m_isNeedCaptureFlash = true;

    return true;
}

}/* namespace android */

