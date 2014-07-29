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
 * \file      ExynosCameraList.h
 * \brief     hearder file for CAMERA HAL MODULE
 * \author    Hyeonmyeong Choi(hyeon.choi@samsung.com)
 * \date      2013/03/05
 *
 */

#ifndef EXYNOS_CAMERA_LIST_H__
#define EXYNOS_CAMERA_LIST_H__

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

#include <utils/RefBase.h>
#include <utils/String8.h>
#include <utils/List.h>
#include "cutils/properties.h"

#define WAIT_TIME (60 * 1000000)

using namespace android;

enum LIST_CMD {
    WAKE_UP = 1,
};

template<typename T>
class ExynosCameraList {
public:
    ExynosCameraList()
    {
        m_statusException = NO_ERROR;
        m_waitProcessQ = false;
        m_waitEmptyQ = false;
    }

    ~ExynosCameraList()
    {
        release();
    }

    void        wakeupAll(void)
    {
        setStatusException(TIMED_OUT);
        if (m_waitProcessQ)
            m_processQCondition.signal();

        if (m_waitEmptyQ)
            m_emptyQCondition.signal();
        setStatusException(NO_ERROR);
    }

    void        sendCmd(uint32_t cmd)
    {
        switch (cmd) {
        case WAKE_UP:
            wakeupAll();
            break;
        default:
            ALOGE("ERR(%s): unknown cmd(%d)", __FUNCTION__, cmd);
            break;
        }
    }

    void        setStatusException(status_t exception)
    {
        Mutex::Autolock lock(m_flagMutex);
        m_statusException = exception;
    }

    status_t    getStatusException(void)
    {
        Mutex::Autolock lock(m_flagMutex);
        return m_statusException;
    }

    /* Process Queue */
    void        pushProcessQ(T *buf)
    {
        Mutex::Autolock lock(m_processQMutex);
        m_processQ.push_back(*buf);

        if (m_waitProcessQ)
            m_processQCondition.signal();
    };

    status_t    popProcessQ(T *buf)
    {
        /* TODO: Remove type dependency of iterator r */
        List<ExynosBuffer>::iterator r;

        Mutex::Autolock lock(m_processQMutex);
        if (m_processQ.empty())
            return false;

        r = m_processQ.begin()++;
        *buf = *r;
        m_processQ.erase(r);

        return OK;
    };

    status_t    waitAndPopProcessQ(T *buf)
    {
        /* TODO: Remove type dependency of iterator r */
        List<ExynosBuffer>::iterator r;

        status_t ret;
        m_processQMutex.lock();
        if (m_processQ.empty()) {
            m_waitProcessQ = true;
            ret = m_processQCondition.waitRelative(m_processQMutex, WAIT_TIME);
            m_waitProcessQ = false;

            if (ret < 0) {
                if (ret == TIMED_OUT)
                    ALOGV("DEBUG(%s): Time out, Skip to pop process Q", __FUNCTION__);
                else
                    ALOGE("ERR(%s): Fail to pop processQ", __FUNCTION__);

                m_processQMutex.unlock();
                return ret;
            }

            ret = getStatusException();
            if (ret != NO_ERROR) {
                m_processQMutex.unlock();
                return ret;
            }
        }

        r = m_processQ.begin()++;
        *buf = *r;
        m_processQ.erase(r);

        m_processQMutex.unlock();
        return OK;
    };

    int         getSizeOfProcessQ(void)
    {
        Mutex::Autolock lock(m_processQMutex);
        return m_processQ.size();
    };

    /* Empty Queue */
    void        pushEmptyQ(T *buf)
    {
        Mutex::Autolock lock(m_emptyQMutex);
        m_emptyQ.push_back(*buf);

        if (m_waitEmptyQ)
            m_emptyQCondition.signal();
    };

    status_t popEmptyQ(T *buf)
    {
        /* TODO: Remove type dependency of iterator r */
        List<ExynosBuffer>::iterator r;

        Mutex::Autolock lock(m_emptyQMutex);
        if (m_emptyQ.empty())
            return UNKNOWN_ERROR;

        r = m_emptyQ.begin()++;
        *buf = *r;
        m_emptyQ.erase(r);

        return OK;
    };

    status_t    waitAndPopEmptyQ(T *buf)
    {
        /* TODO: Remove type dependency of iterator r */
        List<ExynosBuffer>::iterator r;

        status_t ret;
        m_emptyQMutex.lock();
        if (m_emptyQ.empty()) {
            m_waitEmptyQ = true;
            ret = m_emptyQCondition.waitRelative(m_emptyQMutex, WAIT_TIME);
            m_waitEmptyQ = false;

            if (ret < 0) {
                if (ret ==  TIMED_OUT)
                    ALOGV("DEBUG(%s): Time out, Skip to pop empty Q", __FUNCTION__);
                else
                    ALOGE("ERR(%s): Fail to pop emptyQ", __FUNCTION__);

                m_emptyQMutex.unlock();
                return ret;
            }

            ret = getStatusException();
            if (ret != NO_ERROR) {
                m_emptyQMutex.unlock();
                return ret;
            }
        }

        r = m_emptyQ.begin()++;
        *buf = *r;
        m_emptyQ.erase(r);

        m_emptyQMutex.unlock();
        return OK;
    };

    int         getSizeOfEmptyQ(void)
    {
        Mutex::Autolock lock(m_emptyQMutex);
        return m_emptyQ.size();
    };

    /* release both Queue */
    void        release(void)
    {
        setStatusException(TIMED_OUT);

        m_processQMutex.lock();
        if (m_waitProcessQ)
            m_processQCondition.signal();

        m_processQ.clear();
        m_processQMutex.unlock();

        m_emptyQMutex.lock();
        if (m_waitEmptyQ)
            m_emptyQCondition.signal();

        m_emptyQ.clear();
        m_emptyQMutex.unlock();

        setStatusException(NO_ERROR);
    };

private:
    List<T>             m_processQ;
    List<T>             m_emptyQ;
    Mutex               m_processQMutex;
    Mutex               m_emptyQMutex;
    Mutex               m_flagMutex;
    mutable Condition   m_processQCondition;
    mutable Condition   m_emptyQCondition;
    bool                m_waitProcessQ;
    bool                m_waitEmptyQ;
    status_t            m_statusException;
};
#endif
