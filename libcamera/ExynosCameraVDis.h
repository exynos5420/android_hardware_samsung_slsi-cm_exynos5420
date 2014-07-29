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

#ifndef EXYNOS_CAMERA_VDIS_H
#define EXYNOS_CAMERA_VDIS_H

#include <utils/threads.h>
#include <utils/RefBase.h>
#include "exynos_format.h"
#include "csc.h"
#include "ExynosCamera.h"

using namespace android;

#define VDIS_SRC_WDITH     2400
#define VDIS_SRC_HEIGHT    1350
#define VDIS_DST_WDITH     1920
#define VDIS_DST_HEIGHT    1080

#define VDIS_SRC_BUF_NUM   4
#define VDIS_DST_BUF_NUM   4

struct VDis_info{
    unsigned int srcW;
    unsigned int srcH;
    unsigned int dstW;
    unsigned int dstH;
    unsigned int srcBufNum;
    unsigned int dstBufNum;
};

class ExynosCameraVDis : public virtual RefBase {
public:
    ExynosCameraVDis();
    virtual ~ExynosCameraVDis();

    bool init(ExynosCamera *Camera);
    void deinit();
    bool createThread();
    void startVDisInternal();
    void stopVDisThread();
    void setVdisSignal();

private:
    class VdisThread : public Thread {
        ExynosCameraVDis *mHardware;
    public:
        VdisThread(ExynosCameraVDis *hw):
        Thread(false),
        mHardware(hw) { }
        virtual void onFirstRef() {
            run("CameraVdisThread", PRIORITY_DEFAULT);
        }
        virtual bool threadLoop () {
            mHardware->m_vdisThreadFuncWrap();
            return false;
        }
    };

    bool    m_vdisThreadFuncWrap(void);
    bool    m_vdisThreadFunc(void);
    void    release();

    mutable Mutex        m_vdisThreadLock;
    mutable Condition    m_vdisThreadCondition;
    mutable Condition    m_vdisThreadStoppedCondition;
    bool                 m_exitValVdisThread;

    bool                 m_vdisThreadRunning;

    sp<VdisThread>    m_vdisThread;
    ExynosBuffer         *m_dstBuffer[VDIS_DST_BUF_NUM];
    int                  m_dstBufIndex ;
    int                  m_dstBufIndexLastShot;
    unsigned int         m_preRcount;
    unsigned int         m_preFcount;
    ExynosBuffer         m_tempSrcBuffer;
    ExynosBuffer         m_tempDstBuffer;
    int                  m_firstDataIn;
    ExynosCamera         *m_secCamera;
    unsigned int         m_frameCount;
    int                  m_numOfDisShotedFrame;
    int                  m_numOfSrcShotedFrame;
    bool m_isHWVDis;
};
#endif
