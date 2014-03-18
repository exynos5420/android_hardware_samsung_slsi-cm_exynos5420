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

//#define LOG_NDEBUG 0
#define LOG_TAG "ExynosCameraVDis"
#include <cutils/log.h>

#include "ExynosCameraVDis.h"

ExynosCameraVDis::ExynosCameraVDis()
{
    ALOGV("DEBUG(%s)", __func__);

    m_dstBufIndex = 0;
    m_preRcount = 0;
    m_preFcount = 0;
    m_firstDataIn = false;
    m_frameCount = 0;
    m_vdisThread = NULL;
    m_exitValVdisThread = false;
    m_vdisThreadRunning = false;
    m_numOfDisShotedFrame = 0;
    m_numOfSrcShotedFrame = 4;
    m_isHWVDis = true;
}

ExynosCameraVDis::~ExynosCameraVDis()
{
    this->release();
}

void ExynosCameraVDis::release()
{
    ALOGV("DEBUG(%s)", __func__);

    if (m_vdisThread != NULL) {
        m_vdisThread->requestExit();
        m_exitValVdisThread = true;
        m_vdisThreadRunning = true; // let it run so it can exit
        m_vdisThreadCondition.signal();
        m_vdisThread->requestExitAndWait();
        m_vdisThread.clear();
    }
}

bool ExynosCameraVDis::init(ExynosCamera *Camera)
{
    ALOGV("DEBUG(%s)", __func__);

#ifdef USE_VDIS
    m_secCamera = Camera;
    m_secCamera->setVDisSrcW(VDIS_SRC_WDITH);
    m_secCamera->setVDisSrcH(VDIS_SRC_HEIGHT);
    m_secCamera->setVDisDstW(VDIS_DST_WDITH);
    m_secCamera->setVDisDstH(VDIS_DST_HEIGHT);

    m_secCamera->setVDisSrcBufNum(VDIS_SRC_BUF_NUM);
    m_secCamera->setVDisDstBufNum(VDIS_DST_BUF_NUM);

    return true;
#else
    return false;
#endif
}

void ExynosCameraVDis::stopVDisThread()
{
    if (m_vdisThreadRunning == true) {
        m_vdisThreadRunning = false;

        m_secCamera->setRecordingHint(false);

        m_vdisThreadLock.lock();
        ALOGD("DEBUG(%s:%d): SIGNAL(m_vdisThreadLock) - send", __FUNCTION__, __LINE__);
        m_vdisThreadCondition.signal();
        /* wait until preview thread is stopped */
        ALOGD("DEBUG(%s:%d): SIGNAL(m_vdisThreadStoppedCondition) - waiting", __FUNCTION__, __LINE__);
        m_vdisThreadStoppedCondition.wait(m_vdisThreadLock);
        ALOGD("DEBUG(%s:%d): SIGNAL(m_vdisThreadStoppedCondition) - recevied", __FUNCTION__, __LINE__);
        m_vdisThreadLock.unlock();
    }
}

bool ExynosCameraVDis::createThread()
{
    ALOGD("DEBUG(%s)", __func__);

    m_vdisThread = new VdisThread(this);

    return true;
}

void ExynosCameraVDis::startVDisInternal()
{
    int i;
#ifdef USE_VDIS
    m_vdisThreadLock.lock();
    if (m_vdisThreadRunning == false) {

        for(i = 0; i < VDIS_DST_BUF_NUM; i++){
            if (m_secCamera->getVDisDstBufAddr(&m_dstBuffer[i], i) == false) {
                ALOGE("ERR(%s):getVdisInBuf() fail", __func__);
                return;
            }
        }
        m_exitValVdisThread = false;
        m_vdisThreadRunning = true;
        m_vdisThreadCondition.signal();
        ALOGV("DEBUG(%s): signal to start", __func__);
    } else {
        m_vdisThreadCondition.signal();
    }
    m_vdisThreadLock.unlock();
#endif
}

void ExynosCameraVDis::setVdisSignal()
{
    m_vdisThreadCondition.signal();
}

bool ExynosCameraVDis::m_vdisThreadFuncWrap(void)
{
    bool ret = true;

    while (1) {
        m_vdisThreadLock.lock();
        while (m_vdisThreadRunning == false) {
            ALOGD("DEBUG(%s:%d): SIGNAL(m_vdisThreadStoppedCondition) - send", __FUNCTION__, __LINE__);
            m_vdisThreadStoppedCondition.signal();
            ALOGD("DEBUG(%s:%d): SIGNAL(m_vdisThreadCondition) - waiting", __FUNCTION__, __LINE__);
            m_vdisThreadCondition.wait(m_vdisThreadLock);
            ALOGD("DEBUG(%s:%d): SIGNAL(m_vdisThreadCondition) - recevied", __FUNCTION__, __LINE__);
        }
        m_vdisThreadLock.unlock();

        if (m_exitValVdisThread == true) {
            ALOGD("DEBUG(%s):Thread finished", __func__);
            return true;
        }

        m_vdisThreadFunc();
        usleep(10);
    }
}

bool ExynosCameraVDis::m_vdisThreadFunc(void)
{
    ExynosBuffer srcBuf;
    ExynosBuffer *dstBuf;
    int timeout = 0;
    int i;
    int rcount, fcount;
#ifdef USE_VDIS
    if (m_secCamera->getVdisMode()) {
        if (m_secCamera->getVDisSrcBuf(&srcBuf, &rcount, &fcount) == false) {
            ALOGE("ERR(%s):getVdisCaptureBuf() fail", __func__);
        } else {
            m_numOfSrcShotedFrame--;
            ALOGV("DEBUG(%s) fcount(%d) m_frameCount(%d)", __func__, m_preFcount, m_frameCount);
        }

        dstBuf = &srcBuf;

        if (m_secCamera->getNumOfShotedFrame() < 5) {
            ALOGV("DEBUG(%s): m_numOfShotedFrame: %d", __func__, m_secCamera->getNumOfShotedFrame());
            ALOGV("DEBUG(%s): fcount(%d) m_frameCount(%d)", __func__, m_preFcount, m_frameCount);
        }

        if (m_secCamera->putVDisDstBuf(dstBuf, rcount, fcount) == false) {
            ALOGE("ERR(%s):putVdisOutputBuf() fail", __func__);
        } else {
            m_numOfDisShotedFrame++;
        }

        m_preRcount = rcount;
        m_preFcount = fcount;

        m_dstBufIndexLastShot = m_dstBufIndex;
        m_dstBufIndex = (m_dstBufIndex + 1) % VDIS_DST_BUF_NUM;
        ALOGV("DEBUG(%s) DisShotedFrame(%d)", __func__, m_numOfDisShotedFrame);

        if (m_secCamera->getVDisDstBuf(dstBuf) == false) {
            ALOGE("ERR(%s):getVdisOutputBuf() fail", __func__);
        } else {
            m_numOfDisShotedFrame--;
        }
        ALOGV("DEBUG(%s) DisShotedFrame(%d)", __func__, m_numOfDisShotedFrame);

        srcBuf = *dstBuf;

        if (m_secCamera->putVDisSrcBuf(&srcBuf) == false) {
            ALOGE("ERR(%s):putVdisCaptureBuf() fail", __func__);
        } else {
            m_numOfSrcShotedFrame++;
            ALOGV("DEBUG(%s): m_numOfSrcShotedFrame: %d, flagStartSensor(): %d",
                    __func__, m_numOfSrcShotedFrame,
                    m_secCamera->flagStartSensor());
        }
    } else {

        if (m_secCamera->getVDisSrcBuf(&srcBuf, &rcount, &fcount) == false) {
            ALOGE("ERR(%s):getVdisCaptureBuf() fail", __func__);
        } else {
            m_numOfSrcShotedFrame--;
            ALOGV("DEBUG(%s) fcount(%d) m_frameCount(%d)", __func__, m_preFcount, m_frameCount);
        }

        dstBuf = m_dstBuffer[m_dstBufIndex];
        dstBuf->reserved.p = m_dstBufIndex;

        if (m_secCamera->putVDisDstBuf(dstBuf, rcount, fcount) == false) {
            ALOGE("ERR(%s):putVdisOutputBuf() fail", __func__);
        } else {
            m_numOfDisShotedFrame++;
        }

        m_preRcount = rcount;
        m_preFcount = fcount;
        m_dstBufIndexLastShot = m_dstBufIndex;
        m_dstBufIndex = (m_dstBufIndex + 1) % VDIS_DST_BUF_NUM;

        if (m_secCamera->putVDisSrcBuf(&srcBuf) == false) {
            ALOGE("ERR(%s):putVdisCaptureBuf() fail", __func__);
        } else {
            m_numOfSrcShotedFrame++;
            ALOGV("DEBUG(%s): m_numOfSrcShotedFrame: %d, flagStartSensor(): %d",
                    __func__, m_numOfSrcShotedFrame,
                    m_secCamera->flagStartSensor());
        }

        if (m_secCamera->getVDisDstBuf(dstBuf) == false) {
            ALOGE("ERR(%s):getVdisOutputBuf() fail", __func__);
        } else {
            m_numOfDisShotedFrame--;
        }

        /* Debug -- Memcpy for image frame on LCD */
        memcpy(dstBuf->virt.extP[0], srcBuf.virt.extP[0], dstBuf->size.extS[0]);
        memcpy(dstBuf->virt.extP[1], srcBuf.virt.extP[1], dstBuf->size.extS[1]);

    }
#endif /* USE_VIDS */
    return true;
}
