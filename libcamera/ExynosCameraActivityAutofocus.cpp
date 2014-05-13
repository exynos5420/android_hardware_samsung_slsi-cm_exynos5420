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
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* #define LOG_NDEBUG 0 */
#define LOG_TAG "ExynosCameraActivityAutofocus"
#include <cutils/log.h>

#include "ExynosCameraActivityAutofocus.h"

namespace android {

#define WAIT_COUNT_FAIL_STATE                (7)
#define AUTOFOCUS_WAIT_COUNT_STEP_REQUEST    (3)

#define AUTOFOCUS_WAIT_COUNT_FRAME_COUNT_NUM (3)       /* n + x frame count */
#define AUTOFOCUS_WATING_TIME_LOCK_AF        (10000)   /* 10msec */
#define AUTOFOCUS_TOTAL_WATING_TIME_LOCK_AF  (300000)  /* 300msec */
#define AUTOFOCUS_SKIP_FRAME_LOCK_AF         (6)       /* == NUM_BAYER_BUFFERS */

ExynosCameraActivityAutofocus::ExynosCameraActivityAutofocus()
{
    m_flagAutofocusStart = false;
    m_flagAutofocusLock = false;

    /* first Lens position is infinity */
    /* m_autoFocusMode = AUTOFOCUS_MODE_BASE; */
    m_autoFocusMode = AUTOFOCUS_MODE_INFINITY;
    m_interenalAutoFocusMode = AUTOFOCUS_MODE_BASE;

    m_focusWeight = 0;
    /* first AF operation is trigger infinity mode */
    /* m_autofocusStep = AUTOFOCUS_STEP_STOP; */
    m_autofocusStep = AUTOFOCUS_STEP_REQUEST;
    m_aaAfState = ::AA_AFSTATE_INACTIVE;
    m_afState = AUTOFOCUS_STATE_NONE;
    m_aaAFMode = ::AA_AFMODE_OFF;
    m_waitCountFailState = 0;
    m_stepRequestCount = 0;
    m_frameCount = 0;

    m_recordingHint = false;
    m_flagFaceDetection = false;
    m_macroPosition = AUTOFOCUS_MACRO_POSITION_BASE;
}

ExynosCameraActivityAutofocus::~ExynosCameraActivityAutofocus()
{
}

int ExynosCameraActivityAutofocus::t_funcNull(void *args)
{
    return 1;
}

int ExynosCameraActivityAutofocus::t_funcSensorBefore(void *args)
{
    return 1;
}

int ExynosCameraActivityAutofocus::t_funcSensorAfter(void *args)
{
    return 1;
}

int ExynosCameraActivityAutofocus::t_funcISPBefore(void *args)
{
    ExynosBuffer *buf = (ExynosBuffer *)args;

    return 1;
}

int ExynosCameraActivityAutofocus::t_funcISPAfter(void *args)
{
    ExynosBuffer *buf = (ExynosBuffer *)args;

    return 1;
}

int ExynosCameraActivityAutofocus::t_func3ABefore(void *args)
{
    ExynosBuffer *buf = (ExynosBuffer *)args;

    camera2_shot_ext *shot_ext;
    shot_ext = (struct camera2_shot_ext *)(buf->virt.extP[1]);

    int aaAFMode = -1;
    int currentState = this->getCurrentState();

    switch (m_autofocusStep) {
    case AUTOFOCUS_STEP_STOP:
        /* m_interenalAutoFocusMode = m_autoFocusMode; */

        shot_ext->shot.ctl.aa.afMode = ::AA_AFMODE_OFF;
        shot_ext->shot.ctl.aa.afTrigger = 0;
        break;
    case AUTOFOCUS_STEP_REQUEST:
        shot_ext->shot.ctl.aa.afMode = ::AA_AFMODE_OFF;
        shot_ext->shot.ctl.aa.afTrigger = 0;

        /*
         * assure triggering is valid
         * case 0 : adjusted m_aaAFMode is AA_AFMODE_OFF
         * case 1 : AUTOFOCUS_STEP_REQUESTs more than 3 times.
         */
        if (m_aaAFMode == ::AA_AFMODE_OFF ||
            AUTOFOCUS_WAIT_COUNT_STEP_REQUEST < m_stepRequestCount) {

            if (AUTOFOCUS_WAIT_COUNT_STEP_REQUEST < m_stepRequestCount)
                ALOGD("[%s] (%d) m_stepRequestCount(%d), force AUTOFOCUS_STEP_START", __func__, __LINE__, m_stepRequestCount);

            m_stepRequestCount = 0;

            m_autofocusStep = AUTOFOCUS_STEP_START;
        } else {
            m_stepRequestCount++;
        }

        break;
    case AUTOFOCUS_STEP_START:
        aaAFMode = m_AUTOFOCUS_MODE2AA_AFMODE(m_autoFocusMode);
        if (aaAFMode < 0) {
            ALOGE("ERR(%s):m_AUTOFOCUS_MODE2AA_AFMODE(%d) fail", __func__, m_autoFocusMode);
            m_autofocusStep = AUTOFOCUS_STEP_STOP;
            break;
        }

        m_interenalAutoFocusMode = m_autoFocusMode;

        shot_ext->shot.ctl.aa.afMode = (enum aa_afmode)aaAFMode;
        shot_ext->shot.ctl.aa.afTrigger = 1;

        if (m_interenalAutoFocusMode == AUTOFOCUS_MODE_TOUCH) {
            shot_ext->shot.ctl.aa.afRegions[0] = m_focusArea.x1;
            shot_ext->shot.ctl.aa.afRegions[1] = m_focusArea.y1;
            shot_ext->shot.ctl.aa.afRegions[2] = m_focusArea.x2;
            shot_ext->shot.ctl.aa.afRegions[3] = m_focusArea.y2;
            shot_ext->shot.ctl.aa.afRegions[4] = m_focusWeight;
        } else {
            shot_ext->shot.ctl.aa.afRegions[0] = 0;
            shot_ext->shot.ctl.aa.afRegions[1] = 0;
            shot_ext->shot.ctl.aa.afRegions[2] = 0;
            shot_ext->shot.ctl.aa.afRegions[3] = 0;
            shot_ext->shot.ctl.aa.afRegions[4] = 1000;

            /* macro position */
            if (m_interenalAutoFocusMode == AUTOFOCUS_MODE_CONTINUOUS_PICTURE ||
                m_interenalAutoFocusMode == AUTOFOCUS_MODE_MACRO) {
                if (m_macroPosition == AUTOFOCUS_MACRO_POSITION_CENTER)
                    shot_ext->shot.ctl.aa.afRegions[4] = 102;
                else if(m_macroPosition == AUTOFOCUS_MACRO_POSITION_CENTER_UP)
                    shot_ext->shot.ctl.aa.afRegions[4] = 101;
            }
        }

        ALOGD("[%s] (%d) AF-Mode(HAL/FW)=(%d/%d) AF-Region(x1,y1,x2,y2,weight)=(%d, %d, %d, %d, %d)",
                                    __func__, __LINE__, m_interenalAutoFocusMode, aaAFMode,
                                    shot_ext->shot.ctl.aa.afRegions[0],
                                    shot_ext->shot.ctl.aa.afRegions[1],
                                    shot_ext->shot.ctl.aa.afRegions[2],
                                    shot_ext->shot.ctl.aa.afRegions[3],
                                    shot_ext->shot.ctl.aa.afRegions[4]);

        switch (m_interenalAutoFocusMode) {
        /* these affect directly */
        case AUTOFOCUS_MODE_INFINITY:
        case AUTOFOCUS_MODE_FIXED:
            /* These above mode may be considered like CAF. */
            /*
            m_autofocusStep = AUTOFOCUS_STEP_DONE;
            break;
            */
        /* these start scanning directrly */
        case AUTOFOCUS_MODE_CONTINUOUS_VIDEO:
        case AUTOFOCUS_MODE_CONTINUOUS_PICTURE:
        case AUTOFOCUS_MODE_CONTINUOUS_PICTURE_MACRO:
            m_autofocusStep = AUTOFOCUS_STEP_SCANNING;
            break;
        /* these need to wait starting af */
        default:
            m_autofocusStep = AUTOFOCUS_STEP_START_SCANNING;
            break;
        }

        break;
    case AUTOFOCUS_STEP_START_SCANNING:
        if (currentState == AUTOFOCUS_STATE_SCANNING) {
            m_autofocusStep = AUTOFOCUS_STEP_SCANNING;
            m_waitCountFailState = 0;
        }

        break;
    case AUTOFOCUS_STEP_SCANNING:
        switch (m_interenalAutoFocusMode) {
        case AUTOFOCUS_MODE_CONTINUOUS_VIDEO:
        case AUTOFOCUS_MODE_CONTINUOUS_PICTURE:
        case AUTOFOCUS_MODE_CONTINUOUS_PICTURE_MACRO:
            break;
        default:
            if (currentState == AUTOFOCUS_STATE_SUCCEESS ||
                currentState == AUTOFOCUS_STATE_FAIL) {

                /* some times fail is happen on 3, 4, 5 count while scanning */
                if (currentState == AUTOFOCUS_STATE_FAIL &&
                    m_waitCountFailState < WAIT_COUNT_FAIL_STATE) {
                    m_waitCountFailState++;
                    break;
                }

                m_waitCountFailState = 0;
                m_autofocusStep = AUTOFOCUS_STEP_DONE;
            } else {
                 m_waitCountFailState++;
            }
            break;
        }
        break;
    case AUTOFOCUS_STEP_DONE:
        /* to assure next AUTOFOCUS_MODE_AUTO and AUTOFOCUS_MODE_TOUCH */
        shot_ext->shot.ctl.aa.afMode = ::AA_AFMODE_OFF;
        shot_ext->shot.ctl.aa.afTrigger = 0;
        break;
    default:
        break;
    }

    return 1;
}

int ExynosCameraActivityAutofocus::t_func3AAfter(void *args)
{
    ExynosBuffer *buf = (ExynosBuffer *)args;

    camera2_shot_ext *shot_ext;
    shot_ext = (struct camera2_shot_ext *)(buf->virt.extP[1]);

    m_aaAfState = shot_ext->shot.dm.aa.afState;

    m_aaAFMode  = shot_ext->shot.ctl.aa.afMode;

    m_frameCount = shot_ext->shot.dm.request.frameCount;

    return true;
}

int ExynosCameraActivityAutofocus::t_funcSCPBefore(void *args)
{
    ExynosBuffer *buf = (ExynosBuffer *)args;

    return 1;
}

int ExynosCameraActivityAutofocus::t_funcSCPAfter(void *args)
{
    ExynosBuffer *buf = (ExynosBuffer *)args;

    return 1;
}

int ExynosCameraActivityAutofocus::t_funcSCCBefore(void *args)
{
    ExynosBuffer *buf = (ExynosBuffer *)args;

    return 1;
}

int ExynosCameraActivityAutofocus::t_funcSCCAfter(void *args)
{
    ExynosBuffer *buf = (ExynosBuffer *)args;

    return 1;
}

bool ExynosCameraActivityAutofocus::setAutofocusMode(int autoFocusMode)
{
    ALOGD("[%s] (%d) autoFocusMode(%d)", __func__, __LINE__, autoFocusMode);

    bool ret = true;

    switch(autoFocusMode) {
    case AUTOFOCUS_MODE_AUTO:
    case AUTOFOCUS_MODE_INFINITY:
    case AUTOFOCUS_MODE_MACRO:
    case AUTOFOCUS_MODE_FIXED:
    case AUTOFOCUS_MODE_EDOF:
    case AUTOFOCUS_MODE_CONTINUOUS_VIDEO:
    case AUTOFOCUS_MODE_CONTINUOUS_PICTURE:
    case AUTOFOCUS_MODE_CONTINUOUS_PICTURE_MACRO:
    case AUTOFOCUS_MODE_TOUCH:
        m_autoFocusMode = autoFocusMode;
        break;
    default:
        ALOGE("ERR(%s):invalid focus mode(%d) fail", __func__, autoFocusMode);
        ret = false;
        break;
    }

    return ret;
}

int ExynosCameraActivityAutofocus::getAutofocusMode(void)
{
    return m_autoFocusMode;
}

bool ExynosCameraActivityAutofocus::setFocusAreas(ExynosRect2 rect, int weight)
{
    m_focusArea = rect;
    m_focusWeight = weight;

    return true;
}

bool ExynosCameraActivityAutofocus::getFocusAreas(ExynosRect2 *rect, int *weight)
{
    *rect = m_focusArea;
    *weight = m_focusWeight;

    return true;
}

bool ExynosCameraActivityAutofocus::startAutofocus(void)
{
    ALOGD("[%s] (%d) m_autoFocusMode(%d)", __func__, __LINE__, m_autoFocusMode);

    m_autofocusStep = AUTOFOCUS_STEP_REQUEST;
    m_flagAutofocusStart = true;

    return true;
}

bool ExynosCameraActivityAutofocus::stopAutofocus(void)
{
    ALOGD("[%s] (%d) m_autoFocusMode(%d)", __func__, __LINE__, m_autoFocusMode);

    m_autofocusStep = AUTOFOCUS_STEP_STOP;
    m_flagAutofocusStart = false;

    return true;
}

bool ExynosCameraActivityAutofocus::flagAutofocusStart(void)
{
    return m_flagAutofocusStart;
}

bool ExynosCameraActivityAutofocus::lockAutofocus()
{
    ALOGD("[%s] (%d) m_autoFocusMode(%d)", __func__, __LINE__, m_autoFocusMode);

    /* HACK : it may some f/w api rather than stop */
    this->stopAutofocus();

    if (m_aaAfState == AA_AFSTATE_INACTIVE ||
        m_aaAfState == AA_AFSTATE_PASSIVE_SCAN ||
        m_aaAfState == AA_AFSTATE_ACTIVE_SCAN) {
        /*
         * hold, until + 3 Frame
         * n (lockFrameCount) : n - 1's state
         * n + 1              : adjust on f/w
         * n + 2              : adjust on sensor
         * n + 3              : result
         */
        int lockFrameCount = m_frameCount;
        unsigned int i = 0;
        bool flagScanningDetected = false;
        int  scanningDetectedFrameCount = 0;

        for (i = 0; i < AUTOFOCUS_TOTAL_WATING_TIME_LOCK_AF; i++) {
            if (lockFrameCount + AUTOFOCUS_WAIT_COUNT_FRAME_COUNT_NUM <= m_frameCount) {
                ALOGD("DEBUG(%s):find lockFrameCount(%d) + %d, m_frameCount(%d), m_aaAfState(%d)",
                    __func__, lockFrameCount, AUTOFOCUS_WAIT_COUNT_FRAME_COUNT_NUM, m_frameCount, m_aaAfState);
                break;
            }

            if (flagScanningDetected == false) {
                if (m_aaAfState == AA_AFSTATE_PASSIVE_SCAN ||
                    m_aaAfState == AA_AFSTATE_ACTIVE_SCAN) {
                    flagScanningDetected = true;
                    scanningDetectedFrameCount = m_frameCount;
                }
            }

            usleep(AUTOFOCUS_WATING_TIME_LOCK_AF);
        }

        if (AUTOFOCUS_TOTAL_WATING_TIME_LOCK_AF <= i) {
            ALOGW("WARN(%s):AF lock time out (%d)msec", __func__, i / 1000);
        } else {
            /* skip bayer frame when scanning detected */
            if (flagScanningDetected == true) {
                for (i = 0; AUTOFOCUS_TOTAL_WATING_TIME_LOCK_AF; i++) {
                    if (scanningDetectedFrameCount + AUTOFOCUS_SKIP_FRAME_LOCK_AF <= m_frameCount) {
                        ALOGD("DEBUG(%s):kcoolsw find scanningDetectedFrameCount(%d) + %d, m_frameCount(%d), m_aaAfState(%d)",
                            __func__, scanningDetectedFrameCount, AUTOFOCUS_SKIP_FRAME_LOCK_AF, m_frameCount, m_aaAfState);
                        break;
                    }

                    usleep(AUTOFOCUS_WATING_TIME_LOCK_AF);
                }

                if (AUTOFOCUS_TOTAL_WATING_TIME_LOCK_AF <= i)
                    ALOGW("WARN(%s):kcoolsw scanningDectected skip time out (%d)msec", __func__, i / 1000);
            }
        }
    }

    m_flagAutofocusLock = true;

    return true;
}

bool ExynosCameraActivityAutofocus::unlockAutofocus()
{
    ALOGD("[%s] (%d) m_autoFocusMode(%d)", __func__, __LINE__, m_autoFocusMode);

    /* TODO */

    m_flagAutofocusLock = false;

    return true;
}

bool ExynosCameraActivityAutofocus::flagLockAutofocus(void)
{
    return m_flagAutofocusLock;
}

bool ExynosCameraActivityAutofocus::getAutofocusResult(void)
{
    ALOGD("[%s] (%d) getAutofocusResult in m_autoFocusMode(%d)", __func__, __LINE__, m_autoFocusMode);

    bool ret = false;
    bool af_over = false;
    bool flagCheckStep = false;
    int  currentState = AUTOFOCUS_STATE_NONE;
    bool flagScanningStarted = false;

    unsigned int i = 0;

    for (i = 0; i < AUTOFOCUS_TOTAL_WATING_TIME; i += AUTOFOCUS_WATING_TIME) {
        currentState = this->getCurrentState();

        /* If stopAutofocus() called */
        if (m_autofocusStep == AUTOFOCUS_STEP_STOP) {
            af_over = true;

            if (currentState == AUTOFOCUS_STATE_SUCCEESS)
                ret = true;
            else
                ret = false;

            break; /* break for for() loop */
        }

        switch (m_interenalAutoFocusMode) {
        case AUTOFOCUS_MODE_INFINITY:
        case AUTOFOCUS_MODE_FIXED:
            /* These above mode may be considered like CAF. */
            /*
            af_over = true;
            ret = true;
            break;
            */
        case AUTOFOCUS_MODE_CONTINUOUS_VIDEO:
        case AUTOFOCUS_MODE_CONTINUOUS_PICTURE:
        case AUTOFOCUS_MODE_CONTINUOUS_PICTURE_MACRO:
            if (m_autofocusStep == AUTOFOCUS_STEP_SCANNING ||
                m_autofocusStep == AUTOFOCUS_STEP_DONE) {
                flagCheckStep = true;
            }
            break;
        default:
            if (m_autofocusStep == AUTOFOCUS_STEP_DONE)
                flagCheckStep = true;
            break;
        }

        if (flagCheckStep == true) {
            switch (currentState) {
            case AUTOFOCUS_STATE_NONE:
                if (flagScanningStarted == true)
                    ALOGW("WARN(%s):AF restart is detected(%d)", __func__, i / 1000);

                if (m_interenalAutoFocusMode == AUTOFOCUS_MODE_CONTINUOUS_PICTURE) {
                    ALOGD("DEBUG(%s):AF force-success on AUTOFOCUS_MODE_CONTINUOUS_PICTURE (%d)", __func__, i / 1000);
                    af_over = true;
                    ret = true;
                }
                break;
            case AUTOFOCUS_STATE_SCANNING:
                flagScanningStarted = true;
                break;
            case AUTOFOCUS_STATE_SUCCEESS:
                af_over = true;
                ret = true;
                break;
            case AUTOFOCUS_STATE_FAIL:
                af_over = true;
                ret = false;
                break;
             default:
                ALOGV("ERR(%s):Invalid afState(%d)", __func__, currentState);
                ret = false;
                break;
            }
        }

        if (af_over == true)
            break;

        usleep(AUTOFOCUS_WATING_TIME);
    }

    if (AUTOFOCUS_TOTAL_WATING_TIME <= i)
        ALOGW("WARN(%s):AF result time out(%d) msec", __func__, i / 1000);

    ALOGD("[%s] (%d) getAutofocusResult out m_autoFocusMode(%d) m_interenalAutoFocusMode(%d) result(%d) af_over(%d)",
        __func__, __LINE__, m_autoFocusMode, m_interenalAutoFocusMode, ret, af_over);

    return ret;
}

int ExynosCameraActivityAutofocus::getCurrentState(void)
{
    int state = AUTOFOCUS_STATE_NONE;

    if (m_flagAutofocusStart == false) {
        state = m_afState;
        goto done;
    }

    switch (m_aaAfState) {
    case ::AA_AFSTATE_INACTIVE:
        state = AUTOFOCUS_STATE_NONE;
        break;
    case ::AA_AFSTATE_PASSIVE_SCAN:
    case ::AA_AFSTATE_ACTIVE_SCAN:
        state = AUTOFOCUS_STATE_SCANNING;
        break;
    case ::AA_AFSTATE_AF_ACQUIRED_FOCUS:
        state = AUTOFOCUS_STATE_SUCCEESS;
        break;
    case ::AA_AFSTATE_AF_FAILED_FOCUS:
        state = AUTOFOCUS_STATE_FAIL;
        break;
    default:
        state = AUTOFOCUS_STATE_NONE;
        break;
    }

done:
    m_afState = state;

    return state;
}

bool ExynosCameraActivityAutofocus::setRecordingHint(bool hint)
{
    ALOGD("[%s] (%d) hint(%d)", __func__, __LINE__, hint);

    m_recordingHint = hint;
    return true;
}

bool ExynosCameraActivityAutofocus::setFaceDetection(bool toggle)
{
    ALOGD("[%s] (%d) toggle(%d)", __func__, __LINE__, toggle);

    m_flagFaceDetection = toggle;
    return true;
}


bool ExynosCameraActivityAutofocus::setMacroPosition(int macroPosition)
{
    ALOGD("[%s] (%d) macroPosition(%d)", __func__, __LINE__, macroPosition);

    m_macroPosition = macroPosition;
    return true;
}

int ExynosCameraActivityAutofocus::m_AUTOFOCUS_MODE2AA_AFMODE(int autoFocusMode)
{
    int aaAFMode = -1;

    switch (autoFocusMode) {
    case AUTOFOCUS_MODE_AUTO:
        if (m_recordingHint == true)
            aaAFMode = ::AA_AFMODE_AUTO_VIDEO;
        else if (m_flagFaceDetection == true)
            aaAFMode = ::AA_AFMODE_AUTO_FACE;
        else
            aaAFMode = ::AA_AFMODE_AUTO;

        break;
    case AUTOFOCUS_MODE_INFINITY:
        aaAFMode = ::AA_AFMODE_INFINITY;
        break;
    case AUTOFOCUS_MODE_MACRO:
        aaAFMode = ::AA_AFMODE_MACRO;
        break;
    case AUTOFOCUS_MODE_EDOF:
        aaAFMode = ::AA_AFMODE_EDOF;
        break;
    case AUTOFOCUS_MODE_CONTINUOUS_VIDEO:
        aaAFMode = ::AA_AFMODE_CONTINUOUS_VIDEO;
        break;
    case AUTOFOCUS_MODE_CONTINUOUS_PICTURE:
    case AUTOFOCUS_MODE_CONTINUOUS_PICTURE_MACRO:
        if (m_flagFaceDetection == true)
            aaAFMode = ::AA_AFMODE_CONTINUOUS_PICTURE_FACE;
        else
            aaAFMode = ::AA_AFMODE_CONTINUOUS_PICTURE;

        break;
    case AUTOFOCUS_MODE_TOUCH:
        if (m_recordingHint == true)
            aaAFMode = ::AA_AFMODE_AUTO_VIDEO;
        else
            aaAFMode = ::AA_AFMODE_AUTO;

        break;
    default:
        ALOGE("ERR(%s): invalid focus mode (%d)", __func__, autoFocusMode);
        break;
    }

    return aaAFMode;
}

} /* namespace android */