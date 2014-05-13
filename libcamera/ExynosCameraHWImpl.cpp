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
#define LOG_TAG "ExynosCameraHWImpl"
#include <cutils/log.h>

#include "ExynosCameraParameters.h"
#include "ExynosCameraHWImpl.h"
#include "exynos_format.h"

#define VIDEO_COMMENT_MARKER_H          (0xFFBE)
#define VIDEO_COMMENT_MARKER_L          (0xFFBF)
#define VIDEO_COMMENT_MARKER_LENGTH     (4)
#define JPEG_EOI_MARKER                 (0xFFD9)
#define HIBYTE(x)                       (((x) >> 8) & 0xFF)
#define LOBYTE(x)                       ((x) & 0xFF)

/*TODO: This values will be changed */
#define BACK_CAMERA_AUTO_FOCUS_DISTANCES_STR       "0.10,1.20,Infinity"
#define FRONT_CAMERA_FOCUS_DISTANCES_STR           "0.20,0.25,Infinity"

#define BACK_CAMERA_MACRO_FOCUS_DISTANCES_STR      "0.10,0.20,Infinity"
#define BACK_CAMERA_INFINITY_FOCUS_DISTANCES_STR   "0.10,1.20,Infinity"

#define BACK_CAMERA_FOCUS_DISTANCE_INFINITY        "Infinity"
#define FRONT_CAMERA_FOCUS_DISTANCE_INFINITY       "Infinity"

#define PREVIEW_GSC_NODE_NUM                      (4)
#define PICTURE_GSC_NODE_NUM                      (1)
#define VIDEO_GSC_NODE_NUM                        (4)

#define CSC_MEMORY_TYPE                           CSC_MEMORY_DMABUF /* (CSC_MEMORY_USERPTR) */

#define MFC_7X_BUFFER_OFFSET                      (256)

// This hack does two things:
// -- it sets preview to NV21 (YUV420SP)
// -- it sets gralloc to YV12
//
// The reason being: the samsung encoder understands only yuv420sp, and gralloc
// does yv12 and rgb565.  So what we do is we break up the interleaved UV in
// separate V and U planes, which makes preview look good, and enabled the
// encoder as well.
//
// FIXME: Samsung needs to enable support for proper yv12 coming out of the
//        camera, and to fix their video encoder to work with yv12.
// FIXME: It also seems like either Samsung's YUV420SP (NV21) or img's YV12 has
//        the color planes switched.  We need to figure which side is doing it
//        wrong and have the respective party fix it.

namespace android {

struct addrs {
    uint32_t type;  // make sure that this is 4 byte.
    unsigned int fd_y;
    unsigned int fd_cbcr;
    unsigned int buf_index;
    unsigned int reserved;
};

static const int INITIAL_SKIP_FRAME = 8;
static const int EFFECT_SKIP_FRAME = 1;

gralloc_module_t const* ExynosCameraHWImpl::m_grallocHal;

Mutex ExynosCameraHWImpl::g_is3a0Mutex;
Mutex ExynosCameraHWImpl::g_is3a1Mutex;

ExynosCameraHWImpl::ExynosCameraHWImpl(int cameraId, camera_device_t *dev)
        :
          m_captureInProgress(false),
          m_captureMode(false),
          m_waitForCapture(false),
          m_flip_horizontal(0),
          m_faceDetected(false),
          m_fdThreshold(0),
          m_sensorErrCnt(0),
          m_skipFrame(0),
          m_notifyCb(0),
          m_dataCb(0),
          m_dataCbTimestamp(0),
          m_callbackCookie(0),
          m_msgEnabled(0),
          m_storedMsg(0),
          m_halDevice(dev)
{
    m_cameraId = cameraId;

#ifdef CHECK_TIME_START_PREVIEW
    m_startPreviewTimer.start();
#endif //CHECK_TIME_START_PREVIEW

    CLOGD("DEBUG(%s):in", __func__);

    m_exynosPreviewCSC = NULL;
    m_exynosPictureCSC = NULL;
    m_exynosVideoCSC = NULL;

#ifdef USE_VDIS
    m_VDis = NULL;
#endif
    m_previewWindow = NULL;
    m_secCamera = NULL;

    for (int i = 0; i < NUM_OF_PREVIEW_BUF; i++) {
        m_previewCallbackHeap[i] = NULL;
        m_previewBufHandle[i] = NULL;
        m_previewStride[i] = 0;
        m_avaliblePreviewBufHandle[i] = false;
        m_flagGrallocLocked[i] = false;
        m_matchedGrallocIndex[i] = -1;
        m_grallocVirtAddr[i] = NULL;
    }

    m_minUndequeuedBufs = 0;
#ifdef USE_CAMERA_ESD_RESET
    m_sensorESDReset = false;
#endif

    m_getMemoryCb = NULL;

    memset(m_faces, 0, sizeof(camera_face_t) * NUM_OF_DETECTED_FACES);
    m_frameMetadata.number_of_faces = 0;
    m_frameMetadata.faces = m_faces;

    m_flashMode = ExynosCamera::FLASH_MODE_OFF;

    m_recordHeap = NULL;
    for (int i = 0; i < NUM_OF_VIDEO_BUF; i++) {
        m_videoHeap[i] = NULL;
        m_resizedVideoHeap[i][0] = NULL;
        m_resizedVideoHeap[i][1] = NULL;
    }

    for (int i = 0; i < NUM_OF_PICTURE_BUF; i++) {
        m_pictureHeap[i] = NULL;
    }

    m_rawHeap = NULL;
    m_rawHeapSize = 0;
    m_jpegHeap = NULL;
    m_jpegHeapFd = -1;

    m_exitAutoFocusThread = false;
    m_autoFocusRunning = false;

    m_exitVideoThread = false;
    m_exitIspThread = false;

    /*
     * whether the PreviewThread is active in preview or stopped.  we
     * create the thread but it is initially in stopped state.
     */
    m_previewRunning = false;
    m_videoRunning = false;
    m_pictureRunning = false;
    m_sensorRunning = false;

#ifdef OTF_SENSOR_REPROCESSING
    m_sensorRunningReprocessing = false;
#endif
    m_previewStartDeferred = false;
    m_startComplete = THREAD_ID_ALL_CLEARED;

#ifdef START_HW_THREAD_ENABLE
    m_exitStartThreadMain = false;
    m_exitStartThreadReprocessing = false;
    m_exitStartThreadBufAlloc = false;
    m_errorExistInStartThreadMain = false;
    m_errorExistInStartThreadReprocessing = false;
    m_errorExistInStartThreadBufAlloc = false;
#endif

    if (!m_grallocHal) {
        if (hw_get_module(GRALLOC_HARDWARE_MODULE_ID, (const hw_module_t **)&m_grallocHal))
            CLOGE("ERR(%s):Fail on loading gralloc HAL", __func__);
    }

    if (m_initSecCamera(cameraId) == false) {
        CLOGE("ERR(%s):m_initSecCamera(%d) fail", __func__, cameraId);
    }

#ifdef USE_VDIS
    if (m_secCamera->getCameraMode() == ExynosCamera::CAMERA_MODE_BACK) {
        if (m_VDis == NULL) {
            m_VDis = new ExynosCameraVDis;
        }

        if (m_VDis->init(m_secCamera) == false) {
            CLOGD("WARN(%s): SW DIS is not enabled", __func__);
            delete m_VDis;
        }
    }
#endif

    m_initDefaultParameters(cameraId);

    CSC_METHOD cscMethod = CSC_METHOD_HW;

    m_exynosPreviewCSC = csc_init(cscMethod);
    if (m_exynosPreviewCSC == NULL)
        CLOGE("ERR(%s):csc_init() fail", __func__);
    csc_set_hw_property(m_exynosPreviewCSC, CSC_HW_PROPERTY_FIXED_NODE, PREVIEW_GSC_NODE_NUM);

    m_exynosPictureCSC = csc_init(cscMethod);
    if (m_exynosPictureCSC == NULL)
        CLOGE("ERR(%s):csc_init() fail", __func__);
    csc_set_hw_property(m_exynosPictureCSC, CSC_HW_PROPERTY_FIXED_NODE, PICTURE_GSC_NODE_NUM);

    m_exynosVideoCSC = csc_init(cscMethod);
    if (m_exynosVideoCSC == NULL)
        CLOGE("ERR(%s):csc_init() fail", __func__);
    csc_set_hw_property(m_exynosVideoCSC, CSC_HW_PROPERTY_FIXED_NODE, VIDEO_GSC_NODE_NUM);

    isp_input_count = 0;
    isp_last_frame_cnt = 0;
#ifdef SCALABLE_SENSOR
    m_13MCaptureStart = false;
#endif
    m_forceAELock = false;
#ifdef FORCE_LEADER_OFF
    tryThreadStop = false;
#endif

    CLOGD("DEBUG(%s):out", __func__);
}

ExynosCameraHWImpl::~ExynosCameraHWImpl()
{
    CLOGD("DEBUG(%s):in", __func__);

    this->release();

    CLOGD("DEBUG(%s):out", __func__);
}

status_t ExynosCameraHWImpl::setPreviewWindowLocked(preview_stream_ops *w)
{
    CLOGD("DEBUG(%s):in", __func__);

    int ret_val;

    CLOGD("DEBUG(%s):m_startStopLock.lock()", __func__);
    m_startStopLock.lock();

    ret_val = setPreviewWindow(w);

    m_startStopLock.unlock();
    CLOGD("DEBUG(%s):m_startStopLock.unlock()", __func__);

    CLOGD("DEBUG(%s):out", __func__);

    return ret_val;
}

status_t ExynosCameraHWImpl::setPreviewWindow(preview_stream_ops *w)
{
    CLOGD("DEBUG(%s):in", __func__);

    bool restartPreview = false;
    m_previewWindow = w;
    m_numOfDequeuedBuf = 0;

    if (m_previewWindow == NULL) {
        CLOGD("DEBUG(%s):preview window is NULL!", __func__);
        return OK;
    }

    if (m_previewRunning == true) {
        CLOGD("DEBUG(%s):stop preview (window change)", __func__);
        stopPreview();
        CLOGD("DEBUG(%s):stop preview complete (window change)\n\n\n", __func__);
        /* when window chaged, must be restart preview */
        restartPreview = true;
    }

    m_previewLock.lock();

    int previewW, previewH;
    m_secCamera->getPreviewSize(&previewW, &previewH);

    if (m_previewWindow) {
        int hal_pixel_format = HAL_PIXEL_FORMAT_EXYNOS_YCrCb_420_SP;

        const char *str_preview_format = m_params.getPreviewFormat();
        CLOGV("DEBUG(%s):str_preview_format %s width : %d height : %d ", __func__, str_preview_format, previewW, previewH);

        if (!strcmp(str_preview_format,
                    CameraParameters::PIXEL_FORMAT_RGB565)) {
            hal_pixel_format = HAL_PIXEL_FORMAT_RGB_565;
        } else if (!strcmp(str_preview_format,
                         CameraParameters::PIXEL_FORMAT_RGBA8888)) {
            hal_pixel_format = HAL_PIXEL_FORMAT_RGBA_8888;
        } else if (!strcmp(str_preview_format,
                         CameraParameters::PIXEL_FORMAT_YUV420SP)) {
            hal_pixel_format = HAL_PIXEL_FORMAT_EXYNOS_YCrCb_420_SP;
        } else if (!strcmp(str_preview_format,
                         CameraParameters::PIXEL_FORMAT_YUV420P))
            hal_pixel_format = HAL_PIXEL_FORMAT_EXYNOS_YCrCb_420_SP;

        if (m_previewWindow->get_min_undequeued_buffer_count(m_previewWindow, &m_minUndequeuedBufs) != 0) {
            CLOGE("ERR(%s):could not retrieve min undequeued buffer count", __func__);
            m_previewLock.unlock();
            return INVALID_OPERATION;
        }

        if (NUM_OF_PREVIEW_BUF <= m_minUndequeuedBufs) {
            CLOGE("ERR(%s):min undequeued buffer count %d is too high (expecting at most %d)", __func__,
                 m_minUndequeuedBufs, NUM_OF_PREVIEW_BUF - 1);
        }

        if (m_previewWindow->set_buffer_count(m_previewWindow, NUM_OF_PREVIEW_BUF) != 0) {
            CLOGE("ERR(%s):could not set buffer count", __func__);
            m_previewLock.unlock();
            return INVALID_OPERATION;
        }

        if (m_previewWindow->set_usage(m_previewWindow,
                                       GRALLOC_SET_USAGE_FOR_CAMERA) != 0) {
            CLOGE("ERR(%s):could not set usage on gralloc buffer", __func__);
            m_previewLock.unlock();
            return INVALID_OPERATION;
        }

        if (m_previewWindow->set_buffers_geometry(m_previewWindow,
                                                  previewW, previewH,
                                                  hal_pixel_format) != 0) {
            CLOGE("ERR(%s):could not set buffers geometry to %s",
                 __func__, str_preview_format);
            m_previewLock.unlock();
            return INVALID_OPERATION;
        }
    } else {
        /* gralloc support 16-aligned memory internally */
        /* when we have gralloc, we must make 16-aligned memory */
        previewW = ALIGN_UP(previewW, CAMERA_MAGIC_ALIGN);
        previewH = ALIGN_UP(previewH, CAMERA_MAGIC_ALIGN);
        m_secCamera->setPreviewSize(previewW, previewH);
    }
    m_previewLock.unlock();

    if (restartPreview == true) {
        CLOGD("DEBUG(%s):start/resume preview", __func__);
        //m_previewRunning = true;
        if (this->startPreview() != OK) {
            CLOGE("ERR(%s):startPreview() fail", __func__);
            return UNKNOWN_ERROR;
        }
    }

    CLOGD("DEBUG(%s):out", __func__);

    return OK;
}

void ExynosCameraHWImpl::setCallbacks(camera_notify_callback notify_cb,
                                     camera_data_callback data_cb,
                                     camera_data_timestamp_callback data_cb_timestamp,
                                     camera_request_memory get_memory,
                                     void *user)
{
    CLOGD("DEBUG(%s):(notify_cb(%p), data_cb(%p), data_cb_timestamp(%p), get_memory(%p), user(%p)",
        __func__, notify_cb, data_cb, data_cb_timestamp, get_memory, user);

    m_notifyCb        = notify_cb;
    m_dataCb          = data_cb;
    m_dataCbTimestamp = data_cb_timestamp;
    m_getMemoryCb     = get_memory;
    m_callbackCookie  = user;
}

void ExynosCameraHWImpl::enableMsgType(int32_t msgType)
{
    CLOGD("DEBUG(%s):msgType = 0x%x, m_msgEnable(0x%x -> 0x%x)",
         __func__, msgType, m_msgEnabled, m_msgEnabled | msgType);

    m_msgEnabled |= msgType;
}

void ExynosCameraHWImpl::disableMsgType(int32_t msgType)
{
    CLOGD("DEBUG(%s):msgType = 0x%x, m_msgEnabled(0x%x -> 0x%x)",
         __func__, msgType, m_msgEnabled, m_msgEnabled & ~msgType);

    m_msgEnabled &= ~msgType;
}

bool ExynosCameraHWImpl::msgTypeEnabled(int32_t msgType)
{
    return (m_msgEnabled & msgType);
}

status_t ExynosCameraHWImpl::startPreviewLocked()
{
    CLOGD("DEBUG(%s):in", __func__);

    int ret_val;

    CLOGD("DEBUG(%s):m_startStopLock.lock()", __func__);
    m_startStopLock.lock();

    ret_val = startPreview();

    m_startStopLock.unlock();
    CLOGD("DEBUG(%s):m_startStopLock.unlock()", __func__);

    CLOGD("DEBUG(%s):out", __func__);

    return ret_val;
}

status_t ExynosCameraHWImpl::startPreview()
{
    CLOGD("DEBUG(%s):in", __func__);

    int ret = OK;

    Mutex::Autolock lock(m_stateLock);

    char filePath[50];
    int nw = 0;
    char IDBuf = '0';

    if (m_captureInProgress == true) {
        CLOGE("ERR(%s):capture in progress, not allowed", __func__);
        return INVALID_OPERATION;
    }

#ifdef SCALABLE_SENSOR
    /* check starting scalable sensor. It only starts on dual camera */
    m_secCamera->setScalableSensorStart(false);
    m_13MCaptureStart = false;
    CLOGD("DEBUG(%s):scalable sensor(%d)", __func__, m_secCamera->getScalableSensorStart());
#endif

#ifdef FORCE_LEADER_OFF
    tryThreadStop = false;
    tryThreadStatus = 0;
#endif

    m_previewLock.lock();

    if (m_previewRunning == true) {
        CLOGE("ERR(%s):preview thread already running", __func__);
        m_previewLock.unlock();
        return INVALID_OPERATION;
    }

    m_ispThread->run("CameraISPThread", PRIORITY_DEFAULT);

#ifdef USE_CAMERA_ESD_RESET
    m_sensorESDReset = false;
#endif

    m_sensorErrCnt = 0;
    m_sharedBayerBuffer.reserved.p = -1;
    m_sharedISPBuffer.reserved.p = -1;

    for (int i = 0; i < NUM_PREVIEW_BUFFERS; i++) {
        m_previewBufStatus[i] = ON_SERVICE;
        m_previewBufRegistered[i] = false;
    }
    m_releasePreviewQ();

    if (m_secCamera->flagOpen(m_cameraId) == false) {
        if (m_secCamera->openCamera(m_cameraId) == false) {
            CLOGE("ERR(%s):Fail to camera open", __func__);
            ret = UNKNOWN_ERROR;
            goto err;
        }
    }
#ifdef USE_VDIS
    if ((m_secCamera->getRecordingHint() == true) &&
         (m_secCamera->getCameraMode() == ExynosCamera::CAMERA_MODE_BACK)) {
         m_VDis->createThread();
    }
#endif

    if (m_previewWindow == NULL)
        CLOGD("DEBUG(%s):m_previewWindow is NULL", __func__);

    if (m_startPreviewInternal() == true) {
        m_previewRunning = true;
        m_previewThread->run("CameraPreviewThread", PRIORITY_DEFAULT);

        ret = OK;
        goto done;
    } else  {
        CLOGE("ERR(%s):startPreviewInternal() fail", __func__);
        ret = UNKNOWN_ERROR;
        goto err;
    }

err:
    m_previewRunning = false;

    // send requestexit to all preview related threads
    m_previewThread->requestExit();
    m_ispThread->requestExit();
    m_sensorThread->requestExit();

    if (m_secCamera->getCameraMode() == ExynosCamera::CAMERA_MODE_BACK)
        m_sensorThreadReprocessing->requestExit();

    CLOGD("DEBUG(%s):(%d) m_previewThread try exit", __func__, __LINE__);
    m_previewThread->requestExitAndWait();
    CLOGD("DEBUG(%s):(%d) m_previewThread was exited", __func__, __LINE__);

    CLOGD("DEBUG(%s):(%d) m_ispThread try exit", __func__, __LINE__);
    m_exitIspThread = true;
    m_ispCondition.signal();
    m_ispThread->requestExitAndWait();
    m_exitIspThread = false;
    CLOGD("DEBUG(%s):(%d) m_ispThread was exited", __func__, __LINE__);

done:
    m_previewLock.unlock();

    CLOGD("DEBUG(%s):out", __func__);
    return ret;
}

void ExynosCameraHWImpl::stopPreviewLocked()
{
    CLOGD("DEBUG(%s):in", __func__);

    CLOGD("DEBUG(%s):m_startStopLock.lock()", __func__);
    m_startStopLock.lock();

    stopPreview();

    m_startStopLock.unlock();
    CLOGD("DEBUG(%s):m_startStopLock.unlock()", __func__);

    CLOGD("DEBUG(%s):out", __func__);
}

void ExynosCameraHWImpl::stopPreview()
{
    CLOGD("DEBUG(%s):in", __func__);

#ifdef THREAD_PROFILE
    struct timeval mTimeStart;
    struct timeval mTimeStop;
    unsigned long timeUs = 0;
#endif
#ifdef FORCE_LEADER_OFF
    int tryStopCount = 0;
#endif

    /* This function have to fair with restoreMsgType() */
    m_disableMsgType(CAMERA_MSG_PREVIEW_FRAME | CAMERA_MSG_PREVIEW_METADATA, true);

    // FIXME: Need to reset parameter */
    m_params.set(CameraParameters::KEY_VIDEO_STABILIZATION, "false");

    // HACK: move to destroy() for interval when takePicture
/*
    if (m_captureMode == true) {
        m_captureMode = false;
    } else {
*/

    if (m_captureMode)
        this->cancelPicture();

#ifdef FORCE_LEADER_OFF
        tryThreadStop = true;
#endif
        /* Stop at size change*/
        m_previewLock.lock();

        if (m_previewRunning == true) {
            m_previewRunning = false;
            CLOGD("DEBUG(%s):(%d) m_previewRunning false", __func__, __LINE__);

#ifdef USE_VDIS
            /* Stop Dis thread */
            if (m_secCamera->flagStartVdisCapture() == true)
                m_VDis->stopVDisThread();
#endif

            // this notify message for we are in stop phase.
            // but it is no longer needed
            //m_secCamera->notifyStop(true);
#ifdef THREAD_PROFILE
            gettimeofday(&mTimeStart, NULL);
#endif
#ifdef FORCE_LEADER_OFF
            do {
                CLOGD("DEBUG(%s):tryThreadStatus = 0x%x tryStopCount %d", __func__, tryThreadStatus, tryStopCount);

#ifdef DYNAMIC_BAYER_BACK_REC
                if ((m_secCamera->getRecordingHint() == true) && (m_secCamera->getCameraMode() == ExynosCamera::CAMERA_MODE_BACK)) {
                    if (tryThreadStatus == 0x7)
                        break;
                }
#endif
#ifdef SCALABLE_SENSOR
                if (m_secCamera->getScalableSensorStart() == true) {
                    if (tryThreadStatus == 0x7)
                        break;
                }
#endif
                if (tryThreadStatus == 0xF)
                    break;

                if (20 > tryStopCount) {
                    usleep(10000);
                } else {
                    CLOGE("dq blocked - stream off 0x%x", tryThreadStatus);
                    tryThreadStop = true;
                    break;
                }

                tryStopCount ++;
            } while (30 > tryStopCount);
#endif
            // send requestexit to all preview related threads
            m_previewThread->requestExit();
            m_ispThread->requestExit();
            m_sensorThread->requestExit();

            if (m_secCamera->getCameraMode() == ExynosCamera::CAMERA_MODE_BACK)
                m_sensorThreadReprocessing->requestExit();

#ifdef THREAD_PROFILE
            gettimeofday(&mTimeStop, NULL);
            timeUs = (mTimeStop.tv_sec*1000000 + mTimeStop.tv_usec) - (mTimeStart.tv_sec*1000000 + mTimeStart.tv_usec);
            CLOGD("DEBUG(%s):Thread requestexit time_check elapsed time=(%ld)us", __func__, timeUs);
            gettimeofday(&mTimeStart, NULL);
#endif

            CLOGD("DEBUG(%s):(%d) m_stopSensor try exit", __func__, __LINE__);
            m_stopSensor();
            CLOGD("DEBUG(%s):(%d) m_stopSensor was exited", __func__, __LINE__);

#ifdef THREAD_PROFILE
            gettimeofday(&mTimeStop, NULL);
            timeUs = (mTimeStop.tv_sec*1000000 + mTimeStop.tv_usec) - (mTimeStart.tv_sec*1000000 + mTimeStart.tv_usec);
            CLOGD("DEBUG(%s):Thread exit time_check elapsed time=(%ld)us", __func__, timeUs);
#endif
            // stop 3a1
            if (m_secCamera->flagStart3a1Src(ExynosCamera::CAMERA_MODE_FRONT) == true &&
                m_secCamera->flagStart3a1Dst(ExynosCamera::CAMERA_MODE_FRONT) == true &&
                m_secCamera->stopIs3a1(ExynosCamera::CAMERA_MODE_FRONT) == false)
                CLOGE("ERR(%s):m_secCamera->stopIs3a1(ExynosCamera::CAMERA_MODE_FRONT) fail", __func__);

            if (m_secCamera->flagStart3a1Src(ExynosCamera::CAMERA_MODE_BACK) == true &&
                m_secCamera->flagStart3a1Dst(ExynosCamera::CAMERA_MODE_BACK) == true &&
                m_secCamera->stopIs3a1(ExynosCamera::CAMERA_MODE_BACK) == false)
                CLOGE("ERR(%s):m_secCamera->stopIs3a1(ExynosCamera::CAMERA_MODE_BACK) fail", __func__);

            if (m_secCamera->flagStart3a1Src(ExynosCamera::CAMERA_MODE_REPROCESSING) == true &&
                m_secCamera->flagStart3a1Dst(ExynosCamera::CAMERA_MODE_REPROCESSING) == true &&
                m_secCamera->stopIs3a1(ExynosCamera::CAMERA_MODE_REPROCESSING) == false)
                CLOGE("ERR(%s):m_secCamera->stopIs3a1(ExynosCamera::CAMERA_MODE_REPROCESSING) fail", __func__);


            // stop 3a0
            if (m_secCamera->flagStartIs3a0Src(ExynosCamera::CAMERA_MODE_FRONT) == true &&
                m_secCamera->flagStartIs3a0Dst(ExynosCamera::CAMERA_MODE_FRONT) == true &&
                m_secCamera->stopIs3a0(ExynosCamera::CAMERA_MODE_FRONT) == false)
                CLOGE("ERR(%s):m_secCamera->stopIs3a0(ExynosCamera::CAMERA_MODE_FRONT) fail", __func__);

            if (m_secCamera->flagStartIs3a0Src(ExynosCamera::CAMERA_MODE_BACK) == true &&
                m_secCamera->flagStartIs3a0Dst(ExynosCamera::CAMERA_MODE_BACK) == true &&
                m_secCamera->stopIs3a0(ExynosCamera::CAMERA_MODE_BACK) == false)
                CLOGE("ERR(%s):m_secCamera->stopIs3a0(ExynosCamera::CAMERA_MODE_BACK) fail", __func__);

            if (m_secCamera->flagStartIs3a0Src(ExynosCamera::CAMERA_MODE_REPROCESSING) == true &&
                m_secCamera->flagStartIs3a0Dst(ExynosCamera::CAMERA_MODE_REPROCESSING) == true &&
                m_secCamera->stopIs3a0(ExynosCamera::CAMERA_MODE_REPROCESSING) == false)
                CLOGE("ERR(%s):m_secCamera->stopIs3a0(ExynosCamera::CAMERA_MODE_REPROCESSING) fail", __func__);

            /*Isp off*/
            if (m_secCamera->flagStartIsp() == true &&
                m_secCamera->stopIsp() == false)
                CLOGE("ERR(%s):m_secCamera->stopIsp() fail", __func__);

            if (m_secCamera->flagStartIspReprocessing() == true &&
                m_secCamera->stopIspReprocessing() == false)
                CLOGE("ERR(%s):m_secCamera->stopIspReprocessing() fail", __func__);

            /*Preview off*/
            if (m_secCamera->flagStartPreview() == true &&
                m_secCamera->stopPreview() == false)
                CLOGE("ERR(%s):Fail on m_secCamera->stopPreview()", __func__);

            if (m_secCamera->flagStartPreviewReprocessing() == true &&
                m_secCamera->stopPreviewReprocessing() == false)
                CLOGE("ERR(%s):Fail on m_secCamera->stopPreview()", __func__);
#ifdef USE_VDIS
            if (/*m_secCamera->getRecordingHint() == true &&*/ m_secCamera->getCameraMode() == ExynosCamera::CAMERA_MODE_BACK) {
                /* Dis Capture off */
                if (m_secCamera->flagStartVdisCapture() == true &&
                    m_secCamera->stopVdisCapture() == false)
                    ALOGE("ERR(%s):on m_secCamera->stopVdisCapture() fail", __func__);

                /* Dis Output off */
                if (m_secCamera->flagStartVdisOutput() == true &&
                    m_secCamera->stopVdisOutput() == false)
                    ALOGE("ERR(%s):on m_secCamera->stopVdisOutput() fail", __func__);
            }
#endif

            if (m_secCamera->getCameraMode() == ExynosCamera::CAMERA_MODE_BACK) {
                if (m_pictureRunning == true &&
                    m_stopPictureInternalReprocessing() == false)
                    CLOGE("ERR(%s):m_stopPictureInternalReprocessing() fail", __func__);
            } else if (m_secCamera->getCameraMode() == ExynosCamera::CAMERA_MODE_FRONT) {
                if (m_pictureRunning == true &&
                    m_stopPictureInternal() == false)
                    CLOGE("ERR(%s):m_stopPictureInternal() fail", __func__);
            }

        m_stopPreviewInternal();

        CLOGD("DEBUG(%s):(%d) m_previewThread try exit", __func__, __LINE__);
        m_previewThread->requestExitAndWait();
        CLOGD("DEBUG(%s):(%d) m_previewThread was exited", __func__, __LINE__);

        CLOGD("DEBUG(%s):(%d) m_ispThread try exit", __func__, __LINE__);
        m_exitIspThread = true;
        m_ispCondition.signal();
        m_ispThread->requestExitAndWait();
        m_exitIspThread = false;
        CLOGD("DEBUG(%s):(%d) m_ispThread was exited", __func__, __LINE__);

        CLOGD("DEBUG(%s):(%d) m_sensorThread try exit", __func__, __LINE__);
        m_sensorThread->requestExitAndWait();
        CLOGD("DEBUG(%s):(%d) m_sensorThread was exited", __func__, __LINE__);

        // stop sensorReprocessing thread if this mode is dual or back
        if (m_secCamera->getCameraMode() == ExynosCamera::CAMERA_MODE_BACK) {
            CLOGD("DEBUG(%s):(%d) m_stopSensorReprocessing try exit", __func__, __LINE__);
            m_stopSensorReprocessing();
            CLOGD("DEBUG(%s):(%d) m_stopSensorReprocessing was exited", __func__, __LINE__);
        }

#ifdef THREAD_PROFILE
            gettimeofday(&mTimeStop, NULL);
            timeUs = (mTimeStop.tv_sec*1000000 + mTimeStop.tv_usec) - (mTimeStart.tv_sec*1000000 + mTimeStart.tv_usec);
            CLOGD("DEBUG(%s):time_check elapsed time=(%ld)us", __func__, timeUs);
#endif
        } else {
            CLOGD("DEBUG(%s):preview not running, doing nothing", __func__);
        }

        CLOGD("DEBUG(%s):(%d) out", __func__, __LINE__);
        m_previewLock.unlock();

#ifdef FORCE_LEADER_OFF
    tryThreadStatus = 0;
#endif

    m_restoreMsgType();

    CLOGD("DEBUG(%s):out", __func__);
}

bool ExynosCameraHWImpl::previewEnabled()
{
    Mutex::Autolock lock(m_previewLock);
    CLOGV("DEBUG(%s):%d", __func__, m_previewRunning);
    return m_previewRunning;
}

status_t ExynosCameraHWImpl::storeMetaDataInBuffers(bool enable)
{
    if (!enable) {
        CLOGE("Non-m_frameMetadata buffer mode is not supported!");
        return INVALID_OPERATION;
    }
    return OK;
}

status_t ExynosCameraHWImpl::startRecording()
{
    CLOGD("DEBUG(%s):in", __func__);

    Mutex::Autolock lock(m_videoLock);

    int videoW, videoH, videoFormat, videoFramesize;

    m_recordingTimerIndex = 0;

    m_resetRecordingFrameStatus();
    m_lastRecordingTimestamp = 0;

    if (m_recordHeap != NULL) {
        m_recordHeap->release(m_recordHeap);
        m_recordHeap = 0;
        m_recordHeapFd = -1;
    }
    m_recordHeap = m_getMemoryCb(-1, sizeof(struct addrs), NUM_OF_VIDEO_BUF, &m_recordHeapFd);

    m_secCamera->getVideoSize(&videoW, &videoH);
    videoFormat = m_secCamera->getVideoFormat();
    videoFramesize = FRAME_SIZE(V4L2_PIX_2_HAL_PIXEL_FORMAT(videoFormat), videoW, videoH);

    int orgVideoPlaneSize = ALIGN_UP(m_orgVideoRect.w, CAMERA_MAGIC_ALIGN) * ALIGN_UP(m_orgVideoRect.h, CAMERA_MAGIC_ALIGN);

    for (int i = 0; i < NUM_OF_PREVIEW_BUF; i++)
        m_videoBufTimestamp[i] = 1;
    m_recordingStartTimestamp = systemTime(SYSTEM_TIME_MONOTONIC);

    for (int i = 0; i < NUM_OF_VIDEO_BUF; i++) {

#ifdef USE_3DNR_DMAOUT
        ExynosBuffer videoBuf;

        if (m_videoHeap[i] != NULL) {
            m_videoHeap[i]->release(m_videoHeap[i]);
            m_videoHeap[i] = 0;
            m_videoHeapFd[i] = -1;
        }

        m_videoHeap[i] = m_getMemoryCb(-1, videoFramesize, 1, &(m_videoHeapFd[i]));
        if (!m_videoHeap[i] || m_videoHeapFd[i] <= 0) {
            CLOGE("ERR(%s):m_getMemoryCb(m_videoHeap[%d], size(%d) fail", __func__, i, videoFramesize);
            return UNKNOWN_ERROR;
        }

        videoBuf.virt.extP[0] = (char *)m_videoHeap[i]->data;
        videoBuf.fd.extFd[0] = m_videoHeapFd[i];
        m_getAlignedYUVSize(videoFormat, videoW, videoH, &videoBuf);

        videoBuf.reserved.p = i;

        m_secCamera->setVideoBuf(&videoBuf);
#endif

        // original VideoSized heap
        for (int j = 0; j < 2; j++) {
            if (m_resizedVideoHeap[i][j] != NULL) {
                m_resizedVideoHeap[i][j]->release(m_resizedVideoHeap[i][j]);
                m_resizedVideoHeap[i][j] = 0;
                m_resizedVideoHeapFd[i][j] = -1;
            }

            m_resizedVideoHeap[i][j] = m_getMemoryCb(-1, (orgVideoPlaneSize + MFC_7X_BUFFER_OFFSET), 1, &(m_resizedVideoHeapFd[i][j]));
            if (!m_resizedVideoHeap[i][j] || m_resizedVideoHeapFd[i][j] <= 0) {
                CLOGE("ERR(%s):m_getMemoryCb(m_resizedVideoHeap[%d][%d], size(%d) fail",\
                    __func__, i, j, orgVideoPlaneSize);
                return UNKNOWN_ERROR;
            }
        }
    }

    if (m_videoRunning == false) {
        if (m_secCamera->startVideo() == false) {
            CLOGE("ERR(%s):Fail on m_secCamera->startVideo()", __func__);
            return UNKNOWN_ERROR;
        }

        m_videoRunning = true;

        CLOGD("DEBUG(%s:%d): SIGNAL(m_videoCondition) - send", __func__, __LINE__);
        m_videoCondition.signal();
    }

    CLOGD("DEBUG(%s):out", __func__);

    return NO_ERROR;
}

void ExynosCameraHWImpl::stopRecording()
{
    CLOGD("DEBUG(%s):in", __func__);

    if (m_videoRunning == true) {
        m_videoRunning = false;

        Mutex::Autolock lock(m_videoLock);
        m_resetRecordingFrameStatus();
        CLOGD("DEBUG(%s:%d): SIGNAL(m_videoCondition) - send", __func__, __LINE__);
        m_videoCondition.signal();
        /* wait until video thread is stopped */
        CLOGD("DEBUG(%s:%d): SIGNAL(m_videoStoppedCondition) - waiting", __func__, __LINE__);
        if (m_videoStoppedCondition.waitRelative(m_videoLock, (1000 * 1000000)) == NO_ERROR)
            CLOGD("DEBUG(%s:%d): SIGNAL(m_videoStoppedCondition) - recevied", __func__, __LINE__);
        else
            CLOGD("DEBUG(%s:%d): SIGNAL(m_videoStoppedCondition) - not recevied, but release after 1 sec", __func__, __LINE__);
    } else
        CLOGV("DEBUG(%s):video not running, doing nothing", __func__);

    if (m_recordHeap != NULL) {
        m_recordHeap->release(m_recordHeap);
        m_recordHeap = 0;
        m_recordHeapFd = -1;
    }

    for (int i = 0; i < NUM_OF_VIDEO_BUF; i++) {
        for (int j = 0; j < 2; j++) {
            if (m_resizedVideoHeap[i][j] != NULL) {
                m_resizedVideoHeap[i][j]->release(m_resizedVideoHeap[i][j]);
                m_resizedVideoHeap[i][j] = 0;
                m_resizedVideoHeapFd[i][j] = -1;
            }
        }
    }

    CLOGD("DEBUG(%s):out", __func__);
}

bool ExynosCameraHWImpl::recordingEnabled()
{
    return m_videoRunning;
}

void ExynosCameraHWImpl::releaseRecordingFrame(const void *opaque)
{
    Mutex::Autolock lock(m_recordingFrameMutex);

    int i;
    bool found = false;
    struct addrs *addrs = (struct addrs *)m_recordHeap->data;

    if (addrs) {
        for (i = 0; i < NUM_OF_VIDEO_BUF; i++) {
            if ((char *)(&(addrs[i].type)) == (char *)opaque) {
                found = true;
                break;
            }
        }
    }
    if (found) {
        m_availableRecordingFrameCnt++;
        CLOGV("DEBUG(%s): found index[%d] availableCount(%d)", __func__, i, m_availableRecordingFrameCnt);
        m_recordingFrameAvailable[i] = true;
    } else {
        CLOGE("ERR(%s):no matched index(%p)", __func__, (char *)opaque);
    }
}

status_t ExynosCameraHWImpl::autoFocus()
{
    CLOGD("DEBUG(%s):in", __func__);

    /* waiting previous AF is over */
    m_autoFocusLock.lock();

    m_autoFocusThread->requestExitAndWait();
    m_autoFocusThread->run("CameraAutoFocusThread", PRIORITY_DEFAULT);

    CLOGD("DEBUG(%s):out", __func__);

    return NO_ERROR;
}

status_t ExynosCameraHWImpl::cancelAutoFocus()
{
    CLOGD("DEBUG(%s):in", __func__);

    m_autoFocusRunning = false;

    if (m_secCamera->cancelAutoFocus() == false) {
        CLOGE("ERR(%s):Fail on m_secCamera->cancelAutoFocus()", __func__);
        return UNKNOWN_ERROR;
    }

    /* Adonis can support the different AF area and Metering Areas */
#if 0
    /* TODO: Currently we only able to set same area both touchAF and touchMetering */
    if (m_secCamera->getFocusMode() == ExynosCamera::FOCUS_MODE_TOUCH) {
        if (m_secCamera->cancelMeteringAreas() == false) {
            CLOGE("ERR(%s):Fail on m_secCamera->cancelMeteringArea()", __func__);
            return UNKNOWN_ERROR;
        } else {
            m_params.set(CameraParameters::KEY_METERING_AREAS, "(0,0,0,0,0)");
        }
    }
#endif

    CLOGD("DEBUG(%s):out", __func__);

    return NO_ERROR;
}

status_t ExynosCameraHWImpl::takePicture()
{
    CLOGD("DEBUG(%s):in", __func__);

#ifdef CHECK_TIME_SHOT2SHOT
    m_shot2ShotTimer.start();
#endif //CHECK_TIME_SHOT2SHOT

    Mutex::Autolock lock(m_stateLock);
    if (m_previewRunning == false) {
        CLOGE("ERR(%s):Capture fail: preview is not initialized", __func__);
        return INVALID_OPERATION;
    }

    if (m_captureInProgress == true) {
        CLOGW("WARN(%s):capture already in progress", __func__);
        return NO_ERROR;
    }

    m_captureInProgress = true;
    m_waitForCapture = true;

    if (m_videoRunning == false)
        m_captureMode = true;

#ifdef SCALABLE_SENSOR
#ifdef SCALABLE_SENSOR_CHKTIME
    struct timeval start, end;
    int sec, usec;
    gettimeofday(&start, NULL);
#endif
    if ((m_13MCaptureStart == false) &&
         (m_secCamera->getScalableSensorStart()) &&
         (m_secCamera->getCameraMode() == ExynosCamera::CAMERA_MODE_BACK)) {

        /* AE lock for AE-haunting when zoom capture */
        if ((m_secCamera->getZoom() != m_secCamera->getMaxZoom()) &&
            (m_secCamera->getFlashMode() == FLASH_MODE_OFF || m_secCamera->getFlashMode() == FLASH_MODE_AUTO) &&
            (m_secCamera->getAutoExposureLock() == false)) {
            if (m_secCamera->setAutoExposureLock(true) == false)
                CLOGE("ERR(%s):setAutoExposureLock(true) fail", __func__);
            else
                m_forceAELock = true;
        } else {
            m_forceAELock = false;
        }

        if (m_checkAndWaitScalableSate(SCALABLE_SENSOR_SIZE_13M) == false) {
            CLOGE("ERR(%s):m_checkAndWaitScalableSate() fail", __func__);
            return INVALID_OPERATION;
        }
        if (m_chgScalableSensorSize(SCALABLE_SENSOR_SIZE_13M) == false) {
            CLOGE("ERR(%s):scalable sensor input change(SCALABLE_SENSOR_SIZE_13M)!!", __func__);
            return INVALID_OPERATION;
        }
    }
#ifdef SCALABLE_SENSOR_CHKTIME
    gettimeofday(&end, NULL);
    CLOGD("DEBUG(%s):CHKTIME m_chgScalableSensorSize all done total [to picture size](%d)", __func__, (end.tv_sec - start.tv_sec)*1000000 + (end.tv_usec - start.tv_usec));
#endif
#endif

    if (m_pictureThread->run("CameraPictureThread", PRIORITY_DEFAULT) != NO_ERROR) {
        CLOGE("ERR(%s):couldn't run picture thread", __func__);
        return INVALID_OPERATION;
    }

    CLOGD("DEBUG(%s):out", __func__);

    return NO_ERROR;
}

status_t ExynosCameraHWImpl::cancelPicture()
{
    CLOGD("DEBUG(%s):in", __func__);

    if (m_pictureThread.get()) {
        CLOGV("DEBUG(%s):waiting for picture thread to exit", __func__);
        m_pictureThread->requestExitAndWait();
        CLOGV("DEBUG(%s):picture thread has exited", __func__);
    }

    CLOGD("DEBUG(%s):out", __func__);

    return NO_ERROR;
}

status_t ExynosCameraHWImpl::setParametersLocked(const CameraParameters& params)
{
    CLOGD("DEBUG(%s):in", __func__);

    int ret_val;

    CLOGD("DEBUG(%s):m_startStopLock.lock()", __func__);
    m_startStopLock.lock();

    ret_val = setParameters(params);

    m_startStopLock.unlock();
    CLOGD("DEBUG(%s):m_startStopLock.unlock()", __func__);

    CLOGD("DEBUG(%s):out", __func__);

    return ret_val;
}

status_t ExynosCameraHWImpl::setParameters(const CameraParameters& params)
{
    CLOGD("DEBUG(%s):in", __func__);

    status_t ret = NO_ERROR;
    bool flagRestartPreview = false;

    /* This function have to fair with restoreMsgType() */
    m_disableMsgType(CAMERA_MSG_PREVIEW_FRAME | CAMERA_MSG_PREVIEW_METADATA, true);

    /* if someone calls us while picture thread is running, it could screw
     * up the sensor quite a bit so return error.  we can't wait because
     * that would cause deadlock with the callbacks
     */
    m_stateLock.lock();
    if (m_waitForCapture == true) {
        m_stateLock.unlock();
        m_pictureLock.lock();
        if (m_pictureCondition.waitRelative(m_pictureLock, ((nsecs_t)2000 * (nsecs_t)1000000)) == NO_ERROR)
            CLOGD("DEBUG(%s:%d): SIGNAL(m_pictureCondition) - recevied", __func__, __LINE__);
        m_pictureLock.unlock();
    }
    m_stateLock.unlock();

    ///////////////////////////////////////////////////
    // Google Official API : Camera.Parameters
    // http://developer.android.com/reference/android/hardware/Camera.Parameters.html
    ///////////////////////////////////////////////////

    /* recording hint */
    bool recordingHint = false;
    const char *newRecordingHint = params.get(CameraParameters::KEY_RECORDING_HINT);
    if (newRecordingHint != NULL) {
        CLOGD("DEBUG(%s):newRecordingHint : %s", "setParameters", newRecordingHint);

        recordingHint = (strcmp(newRecordingHint, "true") == 0) ? true : false;

        m_secCamera->setRecordingHint(recordingHint);

        m_params.set(CameraParameters::KEY_RECORDING_HINT, newRecordingHint);
    } else {
        recordingHint = m_secCamera->getRecordingHint();
    }

    /* fps range == frameRate */
    int newMinFps = 0;
    int newMaxFps = 0;
    int curMinfps = 0;
    int curMaxfps = 0;
    m_secCamera->getPreviewFpsRange(&curMinfps, &curMaxfps);

    int newFrameRate = params.getPreviewFrameRate();
    int curFrameRate = m_params.getPreviewFrameRate();
    params.getPreviewFpsRange(&newMinFps, &newMaxFps);
    m_params.getPreviewFpsRange(&curMinfps, &curMaxfps);
    const char *newPreviewFpsRange = params.get(CameraParameters::KEY_PREVIEW_FPS_RANGE);

    if (newPreviewFpsRange && (curMinfps != newMinFps || curMaxfps != newMaxFps)) {
        CLOGD("DEBUG(%s):newPreviewFpsRange : %s", "setParameters", newPreviewFpsRange);

        params.getPreviewFpsRange(&newMinFps, &newMaxFps);

        if (newMinFps < 0 || newMaxFps < 0 || newMaxFps < newMinFps) {
            CLOGE("ERR(%s):Fps range value is not valid.(%d, %d)", __func__, newMinFps, newMaxFps);
            ret = UNKNOWN_ERROR;
        } else {
            if (recordingHint == true) {
                 /* set fixed fps. */
                newMinFps = newMaxFps;
            } else if (newMinFps != newMaxFps) {
                if (m_getSupportedVariableFpsList(newMinFps, newMaxFps, &newMinFps, &newMaxFps) == false)
                    newMinFps = newMaxFps / 2;
            }

            if (m_secCamera->setPreviewFpsRange(newMinFps, newMaxFps) == false) {
                CLOGE("ERR(%s):Fail on m_secCamera->setPreviewFpsRange(%d, %d)", __func__, newMinFps, newMaxFps);
            } else {
                m_params.set(CameraParameters::KEY_PREVIEW_FPS_RANGE, newPreviewFpsRange);
                m_params.setPreviewFrameRate((newMaxFps / 1000));
            }
        }
    } else if (newFrameRate != curFrameRate) {
        CLOGD("DEBUG(%s):newFrameRate : %d", "setParameters", newFrameRate);

        int tempFps = newFrameRate * 1000;

        if (recordingHint == true) {
            /* set fixed fps. */
            newMinFps = tempFps;
            newMaxFps = tempFps;
        } else {
            if (m_getSupportedVariableFpsList(tempFps / 2, tempFps, &newMinFps, &newMaxFps) == false) {
                newMinFps = tempFps / 2;
                newMaxFps = tempFps;
            }
        }

        if (m_secCamera->setPreviewFpsRange(newMinFps, newMaxFps) == false) {
            CLOGE("ERR(%s):Fail on m_secCamera->setPreviewFpsRange(%d, %d)", __func__, newMinFps, newMaxFps);
        } else {
            char newFpsRange[256];
            memset (newFpsRange, 0, 256);
            snprintf(newFpsRange, 256, "%d,%d", newMinFps, newMaxFps);
            m_params.set(CameraParameters::KEY_PREVIEW_FPS_RANGE, newFpsRange);
            m_params.setPreviewFrameRate(newFrameRate);
        }
    }

    // video stablization
    const char *newVideoStabilization = params.get(CameraParameters::KEY_VIDEO_STABILIZATION);
    bool currVideoStabilization = m_secCamera->getVideoStabilization();
    bool VideoStabToggle = false;
    if (newVideoStabilization != NULL) {
        CLOGD("DEBUG(%s):newVideoStabilization %s", "setParameters", newVideoStabilization);
        VideoStabToggle = false;

        if (!strcmp(newVideoStabilization, "true"))
            VideoStabToggle = true;

        if (currVideoStabilization != VideoStabToggle) {
            if (m_secCamera->setVideoStabilization(VideoStabToggle) == false) {
                CLOGE("ERR(%s):setVideoStabilization() fail", __func__);
                ret = UNKNOWN_ERROR;
            }
        }
        m_params.set(CameraParameters::KEY_VIDEO_STABILIZATION, newVideoStabilization);
    }

    // Video size
    int newVideoW = 0;
    int newVideoH = 0;
    params.getVideoSize(&newVideoW, &newVideoH);
    CLOGD("DEBUG(%s):newVideoW (%d) newVideoH (%d)", "setParameters", newVideoW, newVideoH);
    if (0 < newVideoW && 0 < newVideoH && m_videoRunning == false &&
        m_isSupportedVideoSize(newVideoW, newVideoH) == true) {

        m_orgVideoRect.w = newVideoW;
        m_orgVideoRect.h = newVideoH;

        if (m_secCamera->setVideoSize(newVideoW, newVideoH) == false) {
            CLOGE("ERR(%s):Fail on m_secCamera->setVideoSize(width(%d), height(%d))",
            __func__, newVideoW, newVideoH);
            ret = UNKNOWN_ERROR;
        }
        m_params.setVideoSize(newVideoW, newVideoH);
    }

    // preview size
    int newPreviewW = 0;
    int newPreviewH = 0;
    int newCalPreviewW = 0;
    int newCalPreviewH = 0;
    params.getPreviewSize(&newPreviewW, &newPreviewH);

    /* hack : when app give 1446, we calibrate to 1440 */
    if (newPreviewW == 1446 && newPreviewH == 1080) {
        CLOGW("WARN(%s):Invalid previewSize(%d/%d). so, calibrate to (1440/%d)", __func__, newPreviewW, newPreviewH, newPreviewH);
        newPreviewW = 1440;
    }

    m_orgPreviewRect.w = newPreviewW;
    m_orgPreviewRect.h = newPreviewH;

    /* calibrate H/W aligned size*/
    newCalPreviewW = ALIGN_UP(newPreviewW, CAMERA_ISP_ALIGN);
    newCalPreviewH = ALIGN_UP(newPreviewH, CAMERA_ISP_ALIGN);

    const char *strNewPreviewFormat = params.getPreviewFormat();
    CLOGD("DEBUG(%s):newPreviewW x newPreviewH = %dx%d, format = %s",
         "setParameters", newPreviewW, newPreviewH, strNewPreviewFormat);

    if (0 < newPreviewW &&
        0 < newPreviewH &&
        strNewPreviewFormat != NULL &&
        m_isSupportedPreviewSize(newPreviewW, newPreviewH) == true) {
        int newPreviewFormat = 0;

        if (!strcmp(strNewPreviewFormat, CameraParameters::PIXEL_FORMAT_RGB565))
            newPreviewFormat = V4L2_PIX_FMT_RGB565;
        else if (!strcmp(strNewPreviewFormat, CameraParameters::PIXEL_FORMAT_RGBA8888))
            newPreviewFormat = V4L2_PIX_FMT_RGB32;
        else if (!strcmp(strNewPreviewFormat, CameraParameters::PIXEL_FORMAT_YUV420SP))
            newPreviewFormat = V4L2_PIX_FMT_NV21M;
        else if (!strcmp(strNewPreviewFormat, CameraParameters::PIXEL_FORMAT_YUV420P))
            newPreviewFormat = V4L2_PIX_FMT_YVU420M;
        else if (!strcmp(strNewPreviewFormat, "yuv420sp_custom"))
            newPreviewFormat = V4L2_PIX_FMT_NV12T;
        else if (!strcmp(strNewPreviewFormat, "yuv422i"))
            newPreviewFormat = V4L2_PIX_FMT_YUYV;
        else if (!strcmp(strNewPreviewFormat, "yuv422p"))
            newPreviewFormat = V4L2_PIX_FMT_YUV422P;
        else
            newPreviewFormat = V4L2_PIX_FMT_NV21; //for 3rd party

        m_orgPreviewRect.colorFormat = newPreviewFormat;
        if (m_orgPreviewRect.colorFormat == V4L2_PIX_FMT_NV21M)
            m_orgPreviewRect.colorFormat = V4L2_PIX_FMT_NV21;

        if (m_orgPreviewRect.colorFormat == V4L2_PIX_FMT_YVU420M) {
            m_orgPreviewRect.colorFormat = V4L2_PIX_FMT_YVU420;

            /* HACK : V4L2_PIX_FMT_NV21M is set to FIMC-IS  *
             * and Gralloc. V4L2_PIX_FMT_YVU420 is just     *
             * color format for callback frame.             */
            newPreviewFormat = V4L2_PIX_FMT_NV21M;
        }

        int curPreviewW, curPreviewH;
        m_params.getPreviewSize(&curPreviewW, &curPreviewH);
        int curPreviewFormat = m_secCamera->getPreviewFormat();

        if (curPreviewW != newPreviewW ||
            curPreviewH != newPreviewH ||
            curPreviewFormat != newPreviewFormat
        ) {
            if (   m_secCamera->setPreviewSize(newCalPreviewW, newCalPreviewH) == false
                || m_secCamera->setPreviewFormat(newPreviewFormat) == false) {
                CLOGE("ERR(%s):Fail on m_secCamera->setPreviewSize(width(%d), height(%d), format(%d))",
                     __func__, newCalPreviewW, newCalPreviewH, newPreviewFormat);
                ret = UNKNOWN_ERROR;
            } else {
                if (m_previewWindow) {
                    if (m_previewRunning == true && m_previewStartDeferred == false) {
                        CLOGE("%s:preview is running, cannot change size and format!", "setParameters");
                        flagRestartPreview = true;
                    } else {
                        CLOGV("DEBUG(%s):m_previewWindow (%p) set_buffers_geometry", __func__, m_previewWindow);
                        CLOGV("DEBUG(%s):m_previewWindow->set_buffers_geometry (%p)", __func__,
                             m_previewWindow->set_buffers_geometry);

                        m_previewWindow->set_buffers_geometry(m_previewWindow,
                                                             newCalPreviewW, newCalPreviewH,
                                                             V4L2_PIX_2_HAL_PIXEL_FORMAT(newPreviewFormat));

                        CLOGD("DEBUG(%s):DONE m_previewWindow (%p) set_buffers_geometry", "setParameters", m_previewWindow);
                    }
                }
                m_params.setPreviewSize(newPreviewW, newPreviewH);
                m_params.setPreviewFormat(strNewPreviewFormat);
            }
        } else {
            CLOGD("DEBUG(%s):preview size and format has not changed", "setParameters");
        }
    } else {
        CLOGE("ERR(%s):Invalid preview size(%dx%d)", __func__, newPreviewW, newPreviewH);
        ret = INVALID_OPERATION;
    }

    int newPictureW = 0;
    int newPictureH = 0;
    params.getPictureSize(&newPictureW, &newPictureH);
    CLOGD("DEBUG(%s):newPictureW x newPictureH = %dx%d", "setParameters", newPictureW, newPictureH);

    if (0 < newPictureW && 0 < newPictureH) {
        if (m_isSupportedPictureSize(newPictureW, newPictureH) == false) {
            CLOGE("ERR(%s):Invalid picture size(%dx%d)", __func__, newPictureW, newPictureH);
            m_restoreMsgType();
            return INVALID_OPERATION;
        }

        int oldPictureW, oldPictureH = 0;
        m_secCamera->getPictureSize(&oldPictureW, &oldPictureH);

        if (m_secCamera->setPictureSize(newPictureW, newPictureH) == false) {
            CLOGE("ERR(%s):Fail on m_secCamera->setPictureSize(width(%d), height(%d))",
                    __func__, newPictureW, newPictureH);
            ret = UNKNOWN_ERROR;
        } else {
            int tempW, tempH = 0;
            m_secCamera->getPictureSize(&tempW, &tempH);
            if (tempW != oldPictureW || tempH != oldPictureH) {
#ifdef FU_3INSTANCE
                if (m_pictureRunning == true || m_previewRunning == true) {
                        CLOGE("%s: Picture ratio changed without stopPreview, force restart preview", __func__);
                    /* HACK : When preview is running, we need stop/start preview for change sensor ratio */
                    /* We must be wait until picture thread is done, and it's working in stopPreview() */
                        stopPreview();
                        startPreview();
                }
#else
                if (m_pictureRunning == true) {
                    if (m_stopPictureInternal() == false)
                        CLOGE("ERR(%s):m_stopPictureInternal() fail", __func__);

                    if (m_startPictureInternal() == false)
                        CLOGE("ERR(%s):m_startPictureInternal() fail", __func__);
                }
#endif
            }
            m_orgPictureRect.w = newPictureW;
            m_orgPictureRect.h = newPictureH;
            m_params.setPictureSize(newPictureW, newPictureH);

        }
    }

    // picture format
    const char *newPictureFormat = params.getPictureFormat();
    if (newPictureFormat != NULL) {
        CLOGD("DEBUG(%s):newPictureFormat %s", "setParameters", newPictureFormat);
        int value = 0;

        if (!strcmp(newPictureFormat, CameraParameters::PIXEL_FORMAT_RGB565))
            value = V4L2_PIX_FMT_RGB565;
        else if (!strcmp(newPictureFormat, CameraParameters::PIXEL_FORMAT_RGBA8888))
            value = V4L2_PIX_FMT_RGB32;
        else if (!strcmp(newPictureFormat, CameraParameters::PIXEL_FORMAT_YUV420SP))
            value = V4L2_PIX_FMT_NV21;
        else if (!strcmp(newPictureFormat, "yuv420sp_custom"))
            value = V4L2_PIX_FMT_NV12T;
        else if (!strcmp(newPictureFormat, "yuv420p"))
            value = V4L2_PIX_FMT_YUV420;
        else if (!strcmp(newPictureFormat, "yuv422i"))
            value = V4L2_PIX_FMT_YUYV;
        else if (!strcmp(newPictureFormat, "uyv422i_custom")) //Zero copy UYVY format
            value = V4L2_PIX_FMT_UYVY;
        else if (!strcmp(newPictureFormat, "uyv422i")) //Non-zero copy UYVY format
            value = V4L2_PIX_FMT_UYVY;
        else if (!strcmp(newPictureFormat, CameraParameters::PIXEL_FORMAT_JPEG))
            value = V4L2_PIX_FMT_YUYV;
        else if (!strcmp(newPictureFormat, "yuv422p"))
            value = V4L2_PIX_FMT_YUV422P;
        else
            value = V4L2_PIX_FMT_NV21; //for 3rd party

        if (value != m_secCamera->getPictureFormat()) {
            if (m_secCamera->setPictureFormat(value) == false) {
                CLOGE("ERR(%s):Fail on m_secCamera->setPictureFormat(format(%d))", __func__, value);
                ret = UNKNOWN_ERROR;
            } else {
                m_orgPictureRect.colorFormat = value;
                m_params.setPictureFormat(newPictureFormat);
            }
        }
    }

    // JPEG image quality
    int newJpegQuality = params.getInt(CameraParameters::KEY_JPEG_QUALITY);
    CLOGD("DEBUG(%s):newJpegQuality %d", "setParameters", newJpegQuality);
    // we ignore bad values
    if (newJpegQuality >=1 && newJpegQuality <= 100) {
        if (m_secCamera->setJpegQuality(newJpegQuality) == false) {
            CLOGE("ERR(%s):Fail on m_secCamera->setJpegQuality(quality(%d))", __func__, newJpegQuality);
            ret = UNKNOWN_ERROR;
        } else {
            m_params.set(CameraParameters::KEY_JPEG_QUALITY, newJpegQuality);
        }
    }

    // JPEG thumbnail size
    int newJpegThumbnailW = params.getInt(CameraParameters::KEY_JPEG_THUMBNAIL_WIDTH);
    int newJpegThumbnailH = params.getInt(CameraParameters::KEY_JPEG_THUMBNAIL_HEIGHT);
    CLOGD("DEBUG(%s):newJpegThumbnailW X newJpegThumbnailH: %d X %d", "setParameters", newJpegThumbnailW, newJpegThumbnailH);
    if (0 <= newJpegThumbnailW && 0 <= newJpegThumbnailH) {
        if (m_secCamera->setJpegThumbnailSize(newJpegThumbnailW, newJpegThumbnailH) == false) {
            CLOGE("ERR(%s):Fail on m_secCamera->setJpegThumbnailSize(width(%d), height(%d))", __func__, newJpegThumbnailW, newJpegThumbnailH);
            ret = UNKNOWN_ERROR;
        } else {
            m_params.set(CameraParameters::KEY_JPEG_THUMBNAIL_WIDTH,  newJpegThumbnailW);
            m_params.set(CameraParameters::KEY_JPEG_THUMBNAIL_HEIGHT, newJpegThumbnailH);
        }
    }

    // JPEG thumbnail quality
    int newJpegThumbnailQuality = params.getInt(CameraParameters::KEY_JPEG_THUMBNAIL_QUALITY);
    CLOGD("DEBUG(%s):newJpegThumbnailQuality %d", "setParameters", newJpegThumbnailQuality);
    // we ignore bad values
    if (newJpegThumbnailQuality >=1 && newJpegThumbnailQuality <= 100) {
        if (m_secCamera->setJpegThumbnailQuality(newJpegThumbnailQuality) == false) {
            CLOGE("ERR(%s):Fail on m_secCamera->setJpegThumbnailQuality(quality(%d))",
                                               __func__, newJpegThumbnailQuality);
            ret = UNKNOWN_ERROR;
        } else {
            m_params.set(CameraParameters::KEY_JPEG_THUMBNAIL_QUALITY, newJpegThumbnailQuality);
        }
    }

    // 3dnr
    const char *new3dnr = params.get("3dnr");
    if (new3dnr != NULL) {
        CLOGD("DEBUG(%s):new3drn %s", "setParameters", new3dnr);
        bool toggle = false;

        if (!strcmp(new3dnr, "true"))
            toggle = true;

        if (m_secCamera->set3DNR(toggle) == false) {
            CLOGE("ERR(%s):set3DNR() fail", __func__);
            ret = UNKNOWN_ERROR;
        } else {
            m_params.set("3dnr", new3dnr);
        }
    }

    // odc
    const char *newOdc = params.get("odc");
    if (newOdc != NULL) {
        CLOGD("DEBUG(%s):newOdc %s", "setParameters", newOdc);
        bool toggle = false;

        if (!strcmp(newOdc, "true"))
            toggle = true;

        if (m_secCamera->setODC(toggle) == false) {
            CLOGE("ERR(%s):setODC() fail", __func__);
            ret = UNKNOWN_ERROR;
        } else {
            m_params.set("odc", newOdc);
        }
    }

    // zoom
    int newZoom = params.getInt(CameraParameters::KEY_ZOOM);
    if (0 <= newZoom) {
        CLOGD("DEBUG(%s):newZoom %d", "setParameters", newZoom);
        if (m_secCamera->setZoom(newZoom) == false) {
            CLOGE("ERR(%s):Fail on m_secCamera->setZoom(newZoom(%d))", __func__, newZoom);
            ret = UNKNOWN_ERROR;
        } else {
            m_params.set(CameraParameters::KEY_ZOOM, newZoom);
        }
    }

    // rotation
    int newRotation = params.getInt(CameraParameters::KEY_ROTATION);
    if (0 <= newRotation) {
        CLOGD("DEBUG(%s):set orientation:%d", "setParameters", newRotation);
        if (m_secCamera->setRotation(newRotation) == false) {
            CLOGE("ERR(%s):Fail on m_secCamera->setRotation(%d)", __func__, newRotation);
            ret = UNKNOWN_ERROR;
        } else {
            m_params.set(CameraParameters::KEY_ROTATION, newRotation);
        }
    }

    // auto exposure lock
    const char *newAutoExposureLock = params.get(CameraParameters::KEY_AUTO_EXPOSURE_LOCK);
    if (newAutoExposureLock != NULL) {
        CLOGD("DEBUG(%s):newAutoExposureLock %s", "setParameters", newAutoExposureLock);
        bool toggle = false;

        if (!strcmp(newAutoExposureLock, "true"))
            toggle = true;

        if (m_secCamera->setAutoExposureLock(toggle) == false) {
            CLOGE("ERR(%s):Fail on m_secCamera->setAutoExposureLock()", __func__);
            ret = UNKNOWN_ERROR;
        } else {
            m_params.set(CameraParameters::KEY_AUTO_EXPOSURE_LOCK, newAutoExposureLock);
        }
    }

    // exposure
    int minExposureCompensation = params.getInt(CameraParameters::KEY_MIN_EXPOSURE_COMPENSATION);
    int maxExposureCompensation = params.getInt(CameraParameters::KEY_MAX_EXPOSURE_COMPENSATION);
    int newExposureCompensation = params.getInt(CameraParameters::KEY_EXPOSURE_COMPENSATION);
    CLOGD("DEBUG(%s):newExposureCompensation %d", "setParameters", newExposureCompensation);
    if ((minExposureCompensation <= newExposureCompensation) &&
        (newExposureCompensation <= maxExposureCompensation)) {
        if (m_secCamera->setExposureCompensation(newExposureCompensation) == false) {
            CLOGE("ERR(%s):Fail on m_secCamera->setExposureCompensation(exposure(%d))", __func__, newExposureCompensation);
            ret = UNKNOWN_ERROR;
        } else {
            m_params.set(CameraParameters::KEY_EXPOSURE_COMPENSATION, newExposureCompensation);
        }
    } else {
        CLOGE("ERR(%s):ExposureCompensation is bigger than max value or smaller than min value", __func__);
        ret = UNKNOWN_ERROR;
    }

    // metering areas
    const char *newMeteringAreas = params.get(CameraParameters::KEY_METERING_AREAS);
    const char *curMeteringAreas = m_params.get(CameraParameters::KEY_METERING_AREAS);
    int maxNumMeteringAreas = m_secCamera->getMaxNumMeteringAreas();

    int newMeteringAreasSize = 0;
    if (newMeteringAreas)
        newMeteringAreasSize = strlen(newMeteringAreas);

    int curMeteringAreasSize = 0;
    if (curMeteringAreas)
        curMeteringAreasSize = strlen(curMeteringAreas);

    bool isMeteringAreasSame = false;

    if (newMeteringAreas != NULL && maxNumMeteringAreas != 0) {
        CLOGD("DEBUG(%s):newMeteringAreas: %s", "setParameters", newMeteringAreas);

        if (newMeteringAreasSize == curMeteringAreasSize && curMeteringAreas != NULL)
            isMeteringAreasSame = strncmp(newMeteringAreas, curMeteringAreas, newMeteringAreasSize);
        else
            isMeteringAreasSame = false;

        if (curMeteringAreas == NULL || !isMeteringAreasSame) {
            // ex : (-10,-10,0,0,300),(0,0,10,10,700)
            ExynosRect2 *rect2s  = new ExynosRect2[maxNumMeteringAreas];
            int         *weights = new int[maxNumMeteringAreas];

            int validMeteringAreas = m_bracketsStr2Ints((char *)newMeteringAreas, maxNumMeteringAreas, rect2s, weights, 1);
            if (0 < validMeteringAreas && validMeteringAreas <= maxNumMeteringAreas) {
                if (m_secCamera->setMeteringAreas(validMeteringAreas, rect2s, weights) == false) {
                    CLOGE("ERR(%s):setMeteringAreas(%s) fail", __func__, newMeteringAreas);
                    ret = UNKNOWN_ERROR;
                } else {
                    m_params.set(CameraParameters::KEY_METERING_AREAS, newMeteringAreas);
                }
            } else {
                CLOGE("ERR(%s):MeteringAreas value is invalid", __func__);
                ret = UNKNOWN_ERROR;
            }

            delete [] rect2s;
            delete [] weights;
        }
    }

    // Metering
    // This is the additional API(not Google API).
    // But, This is set berfore the below KEY_METERING_AREAS.
    const char *strNewMetering = params.get("metering");
    if (strNewMetering != NULL) {
        CLOGD("DEBUG(%s):strNewMetering %s", "setParameters", strNewMetering);
        int newMetering = -1;

        if (!strcmp(strNewMetering, "average"))
            newMetering = ExynosCamera::METERING_MODE_AVERAGE;
        else if (!strcmp(strNewMetering, "center"))
            newMetering = ExynosCamera::METERING_MODE_CENTER;
        else if (!strcmp(strNewMetering, "matrix"))
            newMetering = ExynosCamera::METERING_MODE_MATRIX;
        else if (!strcmp(strNewMetering, "spot"))
            newMetering = ExynosCamera::METERING_MODE_SPOT;
        else {
            CLOGE("ERR(%s):Invalid metering newMetering(%s)", __func__, strNewMetering);
            ret = UNKNOWN_ERROR;
        }

        if (0 <= newMetering) {
            if (m_secCamera->setMeteringMode(newMetering) == false) {
                CLOGE("ERR(%s):Fail on m_secCamera->setMeteringMode(%d)", __func__, newMetering);
                ret = UNKNOWN_ERROR;
            } else {
                m_params.set("metering", strNewMetering);
            }
        }
    }

    // anti banding
    const char *newAntibanding = params.get(CameraParameters::KEY_ANTIBANDING);
    if (newAntibanding != NULL) {
        CLOGD("DEBUG(%s):newAntibanding %s", "setParameters", newAntibanding);
        int value = -1;

        if (!strcmp(newAntibanding, CameraParameters::ANTIBANDING_AUTO))
            value = ExynosCamera::ANTIBANDING_AUTO;
        else if (!strcmp(newAntibanding, CameraParameters::ANTIBANDING_50HZ))
            value = ExynosCamera::ANTIBANDING_50HZ;
        else if (!strcmp(newAntibanding, CameraParameters::ANTIBANDING_60HZ))
            value = ExynosCamera::ANTIBANDING_60HZ;
        else if (!strcmp(newAntibanding, CameraParameters::ANTIBANDING_OFF))
            value = ExynosCamera::ANTIBANDING_OFF;
        else {
            CLOGE("ERR(%s):Invalid antibanding value(%s)", __func__, newAntibanding);
            ret = UNKNOWN_ERROR;
        }

        if (0 <= value) {
            if (m_secCamera->setAntibanding(value) == false) {
                CLOGE("ERR(%s):Fail on m_secCamera->setAntibanding(%d)", __func__, value);
                ret = UNKNOWN_ERROR;
            } else {
                m_params.set(CameraParameters::KEY_ANTIBANDING, newAntibanding);
            }
        }
    }

    // scene mode
    const char *strNewSceneMode = params.get(CameraParameters::KEY_SCENE_MODE);
    const char *newWhiteBalance = params.get(CameraParameters::KEY_WHITE_BALANCE);
    const char *strNewFlashMode = params.get(CameraParameters::KEY_FLASH_MODE);
    const char *strNewFocusMode = params.get(CameraParameters::KEY_FOCUS_MODE);

    if (strNewSceneMode != NULL) {
        CLOGD("DEBUG(%s):strNewSceneMode %s", "setParameters", strNewSceneMode);
        int  newSceneMode = -1;

        if (!strcmp(strNewSceneMode, CameraParameters::SCENE_MODE_AUTO)) {
            newSceneMode = ExynosCamera::SCENE_MODE_AUTO;
        } else {
            /*
            // defaults for non-auto scene modes
            int focusMode = m_secCamera->getSupportedFocusModes();
            if (focusMode & ExynosCamera::FOCUS_MODE_AUTO) {
                strNewFocusMode = CameraParameters::FOCUS_MODE_AUTO;
            } else if (focusMode & ExynosCamera::FOCUS_MODE_INFINITY) {
                strNewFocusMode = CameraParameters::FOCUS_MODE_INFINITY;
            }
            */

            strNewFlashMode = CameraParameters::FLASH_MODE_OFF;

            /* let it change whitebalalance even if SCENE_MODE_SOMETHING */
            /*
            newWhiteBalance = CameraParameters::WHITE_BALANCE_AUTO;
            m_params.set(CameraParameters::KEY_WHITE_BALANCE, newWhiteBalance);
            */

            if (!strcmp(strNewSceneMode, CameraParameters::SCENE_MODE_ACTION)) {
                newSceneMode = ExynosCamera::SCENE_MODE_ACTION;
            } else if (!strcmp(strNewSceneMode, CameraParameters::SCENE_MODE_PORTRAIT)) {
                newSceneMode = ExynosCamera::SCENE_MODE_PORTRAIT;
#ifdef SCENEMODE_FLASH
                strNewFlashMode = CameraParameters::FLASH_MODE_AUTO;
#endif
            } else if (!strcmp(strNewSceneMode, CameraParameters::SCENE_MODE_LANDSCAPE)) {
                newSceneMode = ExynosCamera::SCENE_MODE_LANDSCAPE;
            } else if (!strcmp(strNewSceneMode, CameraParameters::SCENE_MODE_NIGHT)) {
                newSceneMode = ExynosCamera::SCENE_MODE_NIGHT;
            } else if (!strcmp(strNewSceneMode, CameraParameters::SCENE_MODE_NIGHT_PORTRAIT)) {
                newSceneMode = ExynosCamera::SCENE_MODE_NIGHT_PORTRAIT;
            } else if (!strcmp(strNewSceneMode, CameraParameters::SCENE_MODE_THEATRE)) {
                newSceneMode = ExynosCamera::SCENE_MODE_THEATRE;
            } else if (!strcmp(strNewSceneMode, CameraParameters::SCENE_MODE_BEACH)) {
                newSceneMode = ExynosCamera::SCENE_MODE_BEACH;
            } else if (!strcmp(strNewSceneMode, CameraParameters::SCENE_MODE_SNOW)) {
                newSceneMode = ExynosCamera::SCENE_MODE_SNOW;
            } else if (!strcmp(strNewSceneMode, CameraParameters::SCENE_MODE_SUNSET)) {
                newSceneMode = ExynosCamera::SCENE_MODE_SUNSET;
            } else if (!strcmp(strNewSceneMode, CameraParameters::SCENE_MODE_STEADYPHOTO)) {
                newSceneMode = ExynosCamera::SCENE_MODE_STEADYPHOTO;
            } else if (!strcmp(strNewSceneMode, CameraParameters::SCENE_MODE_FIREWORKS)) {
                newSceneMode = ExynosCamera::SCENE_MODE_FIREWORKS;
            } else if (!strcmp(strNewSceneMode, CameraParameters::SCENE_MODE_SPORTS)) {
                newSceneMode = ExynosCamera::SCENE_MODE_SPORTS;
            } else if (!strcmp(strNewSceneMode, CameraParameters::SCENE_MODE_PARTY)) {
                newSceneMode = ExynosCamera::SCENE_MODE_PARTY;
#ifdef SCENEMODE_FLASH
                strNewFlashMode = CameraParameters::FLASH_MODE_AUTO;
#endif
            } else if (!strcmp(strNewSceneMode, CameraParameters::SCENE_MODE_CANDLELIGHT)) {
                newSceneMode = ExynosCamera::SCENE_MODE_CANDLELIGHT;
            } else {
                CLOGE("ERR(%s):unmatched scene_mode(%s)",
                        __func__, strNewSceneMode); //action, night-portrait, theatre, steadyphoto
                ret = UNKNOWN_ERROR;
            }
        }

        // scene mode
        if (0 <= newSceneMode) {
            if (m_secCamera->setSceneMode(newSceneMode) == false) {
                CLOGE("ERR(%s):m_secCamera->setSceneMode(%d) fail", __func__, newSceneMode);
                ret = UNKNOWN_ERROR;
            } else {
                m_params.set(CameraParameters::KEY_SCENE_MODE, strNewSceneMode);
            }
        }

        // focus mode
        if (strNewFocusMode != NULL) {
            CLOGD("DEBUG(%s):strNewFocusMode %s", "setParameters", strNewFocusMode);
            int  newFocusMode = -1;

            if (!strcmp(strNewFocusMode, CameraParameters::FOCUS_MODE_AUTO)) {
                newFocusMode = ExynosCamera::FOCUS_MODE_AUTO;
                m_params.set(CameraParameters::KEY_FOCUS_DISTANCES,
                                BACK_CAMERA_AUTO_FOCUS_DISTANCES_STR);
            } else if (!strcmp(strNewFocusMode, CameraParameters::FOCUS_MODE_INFINITY)) {
                newFocusMode = ExynosCamera::FOCUS_MODE_INFINITY;
                m_params.set(CameraParameters::KEY_FOCUS_DISTANCES,
                                BACK_CAMERA_INFINITY_FOCUS_DISTANCES_STR);
            } else if (!strcmp(strNewFocusMode, CameraParameters::FOCUS_MODE_MACRO)) {
                newFocusMode = ExynosCamera::FOCUS_MODE_MACRO;
                m_params.set(CameraParameters::KEY_FOCUS_DISTANCES,
                                BACK_CAMERA_MACRO_FOCUS_DISTANCES_STR);
            } else if (!strcmp(strNewFocusMode, CameraParameters::FOCUS_MODE_FIXED)) {
                newFocusMode = ExynosCamera::FOCUS_MODE_FIXED;
            } else if (!strcmp(strNewFocusMode, CameraParameters::FOCUS_MODE_EDOF)) {
                newFocusMode = ExynosCamera::FOCUS_MODE_EDOF;
            } else if (!strcmp(strNewFocusMode, CameraParameters::FOCUS_MODE_CONTINUOUS_VIDEO)) {
                newFocusMode = ExynosCamera::FOCUS_MODE_CONTINUOUS_VIDEO;
            } else if (!strcmp(strNewFocusMode, CameraParameters::FOCUS_MODE_CONTINUOUS_PICTURE)) {
                newFocusMode = ExynosCamera::FOCUS_MODE_CONTINUOUS_PICTURE;
            } else if (!strcmp(strNewFocusMode, "face-priority")) {
                newFocusMode = ExynosCamera::FOCUS_MODE_CONTINUOUS_PICTURE;
            } else if (!strcmp(strNewFocusMode, "continuous-picture-macro")) {
                newFocusMode = ExynosCamera::FOCUS_MODE_CONTINUOUS_PICTURE_MACRO;
            } else {
                CLOGE("ERR(%s):unmatched focus_mode(%s)", __func__, strNewFocusMode);
                ret = UNKNOWN_ERROR;
            }

            if (0 <= newFocusMode) {
                if (m_secCamera->setFocusMode(newFocusMode) == false) {
                    CLOGE("ERR(%s):m_secCamera->setFocusMode(%d) fail", __func__, newFocusMode);
                    ret = UNKNOWN_ERROR;
                } else {
                    m_params.set(CameraParameters::KEY_FOCUS_MODE, strNewFocusMode);
                }
            }
        }

        // flash mode
        if (strNewFlashMode != NULL) {
            CLOGD("DEBUG(%s):strNewFlashMode %s", "setParameters", strNewFlashMode);
            int  newFlashMode = -1;

            if (!strcmp(strNewFlashMode, CameraParameters::FLASH_MODE_OFF))
                newFlashMode = ExynosCamera::FLASH_MODE_OFF;
            else if (!strcmp(strNewFlashMode, CameraParameters::FLASH_MODE_AUTO))
                newFlashMode = ExynosCamera::FLASH_MODE_AUTO;
            else if (!strcmp(strNewFlashMode, CameraParameters::FLASH_MODE_ON))
                newFlashMode = ExynosCamera::FLASH_MODE_ON;
            else if (!strcmp(strNewFlashMode, CameraParameters::FLASH_MODE_RED_EYE))
                newFlashMode = ExynosCamera::FLASH_MODE_RED_EYE;
            else if (!strcmp(strNewFlashMode, CameraParameters::FLASH_MODE_TORCH))
                newFlashMode = ExynosCamera::FLASH_MODE_TORCH;
            else {
                CLOGE("ERR(%s):unmatched flash_mode(%s)", __func__, strNewFlashMode); //red-eye
                ret = UNKNOWN_ERROR;
            }

            m_flashMode = newFlashMode;

            if (m_secCamera->setFlashMode(newFlashMode) == false)
                CLOGE("ERR(%s):m_secCamera->setFlashMode(%d) fail", __func__, newFlashMode);
            if (ret < 0) {
                m_params.set(CameraParameters::KEY_FLASH_MODE, CameraParameters::FLASH_MODE_OFF);
            } else {
                m_params.set(CameraParameters::KEY_FLASH_MODE, strNewFlashMode);
            }
        }
    } else {
        CLOGW("WARN(%s):strNewSceneMode is NULL", __func__);
    }

    // white balance

    if (newWhiteBalance != NULL &&
        strNewSceneMode && !strcmp(strNewSceneMode, CameraParameters::SCENE_MODE_AUTO)) {
        CLOGD("DEBUG(%s):newWhiteBalance %s", "setParameters", newWhiteBalance);
        int value = -1;

        if (!strcmp(newWhiteBalance, CameraParameters::WHITE_BALANCE_AUTO))
            value = ExynosCamera::WHITE_BALANCE_AUTO;
        else if (!strcmp(newWhiteBalance, CameraParameters::WHITE_BALANCE_INCANDESCENT))
            value = ExynosCamera::WHITE_BALANCE_INCANDESCENT;
        else if (!strcmp(newWhiteBalance, CameraParameters::WHITE_BALANCE_FLUORESCENT))
            value = ExynosCamera::WHITE_BALANCE_FLUORESCENT;
        else if (!strcmp(newWhiteBalance, CameraParameters::WHITE_BALANCE_WARM_FLUORESCENT))
            value = ExynosCamera::WHITE_BALANCE_WARM_FLUORESCENT;
        else if (!strcmp(newWhiteBalance, CameraParameters::WHITE_BALANCE_DAYLIGHT))
            value = ExynosCamera::WHITE_BALANCE_DAYLIGHT;
        else if (!strcmp(newWhiteBalance, CameraParameters::WHITE_BALANCE_CLOUDY_DAYLIGHT))
            value = ExynosCamera::WHITE_BALANCE_CLOUDY_DAYLIGHT;
        else if (!strcmp(newWhiteBalance, CameraParameters::WHITE_BALANCE_TWILIGHT))
            value = ExynosCamera::WHITE_BALANCE_TWILIGHT;
        else if (!strcmp(newWhiteBalance, CameraParameters::WHITE_BALANCE_SHADE))
            value = ExynosCamera::WHITE_BALANCE_SHADE;
        else {
            CLOGE("ERR(%s):Invalid white balance(%s)", __func__, newWhiteBalance); //twilight, shade, warm_flourescent
            ret = UNKNOWN_ERROR;
        }

        if (0 <= value) {
            if (m_secCamera->setWhiteBalance(value) == false) {
                CLOGE("ERR(%s):Fail on m_secCamera->setWhiteBalance(white(%d))", __func__, value);
                ret = UNKNOWN_ERROR;
            } else {
                m_params.set(CameraParameters::KEY_WHITE_BALANCE, newWhiteBalance);
            }
        }
    }

    // auto white balance lock
    const char *newAutoWhitebalanceLock = params.get(CameraParameters::KEY_AUTO_WHITEBALANCE_LOCK);
    if (newAutoWhitebalanceLock != NULL) {
        CLOGD("DEBUG(%s):newAutoWhitebalanceLock %s", "setParameters", newAutoWhitebalanceLock);
        bool toggle = false;

        if (!strcmp(newAutoWhitebalanceLock, "true"))
            toggle = true;

        if (m_secCamera->setAutoWhiteBalanceLock(toggle) == false) {
            CLOGE("ERR(%s):Fail on m_secCamera->setAutoWhiteBalanceLock()", __func__);
            ret = UNKNOWN_ERROR;
        } else {
            m_params.set(CameraParameters::KEY_AUTO_WHITEBALANCE_LOCK, newAutoWhitebalanceLock);
        }
    }

    // focus areas
    const char *newFocusAreas = params.get(CameraParameters::KEY_FOCUS_AREAS);
    int maxNumFocusAreas = 1;

    if (newFocusAreas != NULL && maxNumFocusAreas != 0) {
        CLOGD("DEBUG(%s):newFocusAreas %s", "setParameters", newFocusAreas);
        int curFocusMode = m_secCamera->getFocusMode();

        // In CameraParameters.h
        // Focus area only has effect if the cur focus mode is FOCUS_MODE_AUTO,
        // FOCUS_MODE_MACRO, FOCUS_MODE_CONTINUOUS_VIDEO, or
        // FOCUS_MODE_CONTINUOUS_PICTURE.
        if (   curFocusMode & ExynosCamera::FOCUS_MODE_AUTO
            || curFocusMode & ExynosCamera::FOCUS_MODE_MACRO
            || curFocusMode & ExynosCamera::FOCUS_MODE_CONTINUOUS_VIDEO
            || curFocusMode & ExynosCamera::FOCUS_MODE_CONTINUOUS_PICTURE
            || curFocusMode & ExynosCamera::FOCUS_MODE_CONTINUOUS_PICTURE_MACRO) {

            // ex : (-10,-10,0,0,300),(0,0,10,10,700)
            ExynosRect2 *rect2s = new ExynosRect2[maxNumFocusAreas];
            int         *weights = new int[maxNumFocusAreas];

            int validFocusedAreas = m_bracketsStr2Ints((char *)newFocusAreas, maxNumFocusAreas, rect2s, weights, 1);
            if (0 < validFocusedAreas) {
                // CameraParameters.h
                // A special case of single focus area (0,0,0,0,0) means driver to decide
                // the focus area. For example, the driver may use more signals to decide
                // focus areas and change them dynamically. Apps can set (0,0,0,0,0) if they
                // want the driver to decide focus areas.
                if (m_secCamera->setFocusAreas(validFocusedAreas, rect2s, weights) == false) {
                    CLOGE("ERR(%s):setFocusAreas(%s) fail", __func__, newFocusAreas);
                    ret = UNKNOWN_ERROR;
                } else {
                    m_params.set(CameraParameters::KEY_FOCUS_AREAS, newFocusAreas);
                }
            } else {
                CLOGE("ERR(%s):FocusAreas value is invalid", __func__);
                ret = UNKNOWN_ERROR;
            }

            delete [] rect2s;
            delete [] weights;
        }
    } else {
        ExynosRect2 *nullRect2 = NULL;
        if (m_secCamera->setFocusAreas(0, nullRect2, NULL) == false) {
            CLOGE("ERR(%s):setFocusAreas(%d) fail", __func__, 0);
            ret = UNKNOWN_ERROR;
        }
    }

    // image effect
    const char *strNewEffect = params.get(CameraParameters::KEY_EFFECT);
    if (strNewEffect != NULL) {
        CLOGD("DEBUG(%s):strNewEffect %s", "setParameters", strNewEffect);
        int  newEffect = -1;

        if (!strcmp(strNewEffect, CameraParameters::EFFECT_NONE)) {
            newEffect = ExynosCamera::EFFECT_NONE;
        } else if (!strcmp(strNewEffect, CameraParameters::EFFECT_MONO)) {
            newEffect = ExynosCamera::EFFECT_MONO;
        } else if (!strcmp(strNewEffect, CameraParameters::EFFECT_NEGATIVE)) {
            newEffect = ExynosCamera::EFFECT_NEGATIVE;
        } else if (!strcmp(strNewEffect, CameraParameters::EFFECT_SOLARIZE)) {
            newEffect = ExynosCamera::EFFECT_SOLARIZE;
        } else if (!strcmp(strNewEffect, CameraParameters::EFFECT_SEPIA)) {
            newEffect = ExynosCamera::EFFECT_SEPIA;
        } else if (!strcmp(strNewEffect, CameraParameters::EFFECT_POSTERIZE)) {
            newEffect = ExynosCamera::EFFECT_POSTERIZE;
        } else if (!strcmp(strNewEffect, CameraParameters::EFFECT_WHITEBOARD)) {
            newEffect = ExynosCamera::EFFECT_WHITEBOARD;
        } else if (!strcmp(strNewEffect, CameraParameters::EFFECT_BLACKBOARD)) {
            newEffect = ExynosCamera::EFFECT_BLACKBOARD;
        } else if (!strcmp(strNewEffect, CameraParameters::EFFECT_AQUA)) {
            newEffect = ExynosCamera::EFFECT_AQUA;
        } else {
            CLOGE("ERR(%s):Invalid effect(%s)", __func__, strNewEffect);
            /*hack : this comment will remove after revise these error from LSI*/
            /* ret = UNKNOWN_ERROR; */
        }

        if (0 <= newEffect) {
            if (m_secCamera->setColorEffect(newEffect) == false) {
                CLOGE("ERR(%s):Fail on m_secCamera->setColorEffect(effect(%d))", __func__, newEffect);
                ret = UNKNOWN_ERROR;
            } else {
                const char *oldStrEffect = m_params.get(CameraParameters::KEY_EFFECT);

                if (oldStrEffect) {
                    if (strcmp(oldStrEffect, strNewEffect)) {
                        m_setSkipFrame(EFFECT_SKIP_FRAME);
                    }
                }
                m_params.set(CameraParameters::KEY_EFFECT, strNewEffect);
            }
        }
    }

    // gps altitude
    const char *strNewGpsAltitude = params.get(CameraParameters::KEY_GPS_ALTITUDE);
    if (strNewGpsAltitude != NULL)
        CLOGD("DEBUG(%s):strNewGpsAltitude %s", "setParameters", strNewGpsAltitude);

    if (m_secCamera->setGpsAltitude(strNewGpsAltitude) == false) {
        CLOGE("ERR(%s):m_secCamera->setGpsAltitude(%s) fail", __func__, strNewGpsAltitude);
        ret = UNKNOWN_ERROR;
    } else {
        if (strNewGpsAltitude)
            m_params.set(CameraParameters::KEY_GPS_ALTITUDE, strNewGpsAltitude);
        else
            m_params.remove(CameraParameters::KEY_GPS_ALTITUDE);
    }

    // gps latitude
    const char *strNewGpsLatitude = params.get(CameraParameters::KEY_GPS_LATITUDE);
    if (strNewGpsLatitude != NULL)
        CLOGD("DEBUG(%s):strNewGpsLatitude %s", "setParameters", strNewGpsLatitude);
    if (m_secCamera->setGpsLatitude(strNewGpsLatitude) == false) {
        CLOGE("ERR(%s):m_secCamera->setGpsLatitude(%s) fail", __func__, strNewGpsLatitude);
        ret = UNKNOWN_ERROR;
    } else {
        if (strNewGpsLatitude)
            m_params.set(CameraParameters::KEY_GPS_LATITUDE, strNewGpsLatitude);
        else
            m_params.remove(CameraParameters::KEY_GPS_LATITUDE);
    }

    // gps longitude
    const char *strNewGpsLongtitude = params.get(CameraParameters::KEY_GPS_LONGITUDE);
    if (strNewGpsLongtitude != NULL)
        CLOGD("DEBUG(%s):strNewGpsLongtitude %s", "setParameters", strNewGpsLongtitude);
    if (m_secCamera->setGpsLongitude(strNewGpsLongtitude) == false) {
        CLOGE("ERR(%s):m_secCamera->setGpsLongitude(%s) fail", __func__, strNewGpsLongtitude);
        ret = UNKNOWN_ERROR;
    } else {
        if (strNewGpsLongtitude)
            m_params.set(CameraParameters::KEY_GPS_LONGITUDE, strNewGpsLongtitude);
        else
            m_params.remove(CameraParameters::KEY_GPS_LONGITUDE);
    }

    // gps processing method
    const char *strNewGpsProcessingMethod = params.get(CameraParameters::KEY_GPS_PROCESSING_METHOD);
    if (strNewGpsProcessingMethod != NULL)
        CLOGD("DEBUG(%s):strNewGpsProcessingMethod %s", "setParameters", strNewGpsProcessingMethod);

    if (m_secCamera->setGpsProcessingMethod(strNewGpsProcessingMethod) == false) {
        CLOGE("ERR(%s):m_secCamera->setGpsProcessingMethod(%s) fail", __func__, strNewGpsProcessingMethod);
        ret = UNKNOWN_ERROR;
    } else {
        if (strNewGpsProcessingMethod)
            m_params.set(CameraParameters::KEY_GPS_PROCESSING_METHOD, strNewGpsProcessingMethod);
        else
            m_params.remove(CameraParameters::KEY_GPS_PROCESSING_METHOD);
    }

    // gps timestamp
    const char *strNewGpsTimestamp = params.get(CameraParameters::KEY_GPS_TIMESTAMP);
    if (strNewGpsTimestamp != NULL)
        CLOGD("DEBUG(%s):strNewGpsTimestamp %s", "setParameters", strNewGpsTimestamp);
    if (m_secCamera->setGpsTimeStamp(strNewGpsTimestamp) == false) {
        CLOGE("ERR(%s):m_secCamera->setGpsTimeStamp(%s) fail", __func__, strNewGpsTimestamp);
        ret = UNKNOWN_ERROR;
    } else {
        if (strNewGpsTimestamp)
            m_params.set(CameraParameters::KEY_GPS_TIMESTAMP, strNewGpsTimestamp);
        else
            m_params.remove(CameraParameters::KEY_GPS_TIMESTAMP);
    }

    ///////////////////////////////////////////////////
    // Additional API.
    ///////////////////////////////////////////////////
    // brightness
    int newBrightness = params.getInt("brightness");
    int maxBrightness = params.getInt("brightness-max");
    int minBrightness = params.getInt("brightness-min");
    CLOGD("DEBUG(%s):newBrightness %d", "setParameters", newBrightness);
    if ((minBrightness <= newBrightness) && (newBrightness <= maxBrightness)) {
        if (m_secCamera->setBrightness(newBrightness) == false) {
            CLOGE("ERR(%s):Fail on m_secCamera->setBrightness(%d)", __func__, newBrightness);
            ret = UNKNOWN_ERROR;
        } else {
            m_params.set("brightness", newBrightness);
        }
    }

    // saturation
    int newSaturation = params.getInt("saturation");
    int maxSaturation = params.getInt("saturation-max");
    int minSaturation = params.getInt("saturation-min");
    CLOGD("DEBUG(%s):newSaturation %d", "setParameters", newSaturation);
    if ((minSaturation <= newSaturation) && (newSaturation <= maxSaturation)) {
        if (m_secCamera->setSaturation(newSaturation) == false) {
            CLOGE("ERR(%s):Fail on m_secCamera->setSaturation(%d)", __func__, newSaturation);
            ret = UNKNOWN_ERROR;
        } else {
            m_params.set("saturation", newSaturation);
        }
    }

    // sharpness
    int newSharpness = params.getInt("sharpness");
    int maxSharpness = params.getInt("sharpness-max");
    int minSharpness = params.getInt("sharpness-min");
    CLOGD("DEBUG(%s):newSharpness %d", "setParameters", newSharpness);
    if ((minSharpness <= newSharpness) && (newSharpness <= maxSharpness)) {
        if (m_secCamera->setSharpness(newSharpness) == false) {
            CLOGE("ERR(%s):Fail on m_secCamera->setSharpness(%d)", __func__, newSharpness);
            ret = UNKNOWN_ERROR;
        } else {
            m_params.set("sharpness", newSharpness);
        }
    }

    // hue
    int newHue = params.getInt("hue");
    int maxHue = params.getInt("hue-max");
    int minHue = params.getInt("hue-min");
    CLOGD("DEBUG(%s):newHue %d", "setParameters", newHue);
    if ((minHue <= newHue) && (maxHue >= newHue)) {
        if (m_secCamera->setHue(newHue) == false) {
            CLOGE("ERR(%s):Fail on m_secCamera->setHue(hue(%d))", __func__, newHue);
            ret = UNKNOWN_ERROR;
        } else {
            m_params.set("hue", newHue);
        }
    }

    // ISO
    const char *strNewISO = params.get("iso");
    if (strNewISO != NULL) {
        CLOGD("DEBUG(%s):strNewISO %s", "setParameters", strNewISO);
        int newISO = -1;

        if (!strcmp(strNewISO, "auto"))
            newISO = 0;
        else {
            newISO = (int)atoi(strNewISO);
            if (newISO == 0) {
                CLOGE("ERR(%s):Invalid iso value(%s)", __func__, strNewISO);
                ret = UNKNOWN_ERROR;
            }
        }

        if (0 <= newISO) {
            if (m_secCamera->setISO(newISO) == false) {
                CLOGE("ERR(%s):Fail on m_secCamera->setISO(iso(%d))", __func__, newISO);
                ret = UNKNOWN_ERROR;
            } else {
                m_params.set("iso", strNewISO);
            }
        }
    }

    //contrast
    const char *strNewContrast = params.get("contrast");
    if (strNewContrast != NULL) {
        CLOGD("DEBUG(%s):strNewContrast %s", "setParameters", strNewContrast);
        int newContrast = -1;

        if (!strcmp(strNewContrast, "auto"))
            newContrast = ExynosCamera::CONTRAST_AUTO;
        else if (!strcmp(strNewContrast, "-2"))
            newContrast = ExynosCamera::CONTRAST_MINUS_2;
        else if (!strcmp(strNewContrast, "-1"))
            newContrast = ExynosCamera::CONTRAST_MINUS_1;
        else if (!strcmp(strNewContrast, "0"))
            newContrast = ExynosCamera::CONTRAST_DEFAULT;
        else if (!strcmp(strNewContrast, "1"))
            newContrast = ExynosCamera::CONTRAST_PLUS_1;
        else if (!strcmp(strNewContrast, "2"))
            newContrast = ExynosCamera::CONTRAST_PLUS_2;
        else {
            CLOGE("ERR(%s):Invalid contrast value(%s)", __func__, strNewContrast);
            ret = UNKNOWN_ERROR;
        }

        if (0 <= newContrast) {
            if (m_secCamera->setContrast(newContrast) == false) {
                CLOGE("ERR(%s):Fail on m_secCamera->setContrast(contrast(%d))", __func__, newContrast);
                ret = UNKNOWN_ERROR;
            } else {
                m_params.set("contrast", strNewContrast);
            }
        }
    }

    //anti shake
    int newAntiShake = params.getInt("anti-shake");
    if (0 <= newAntiShake) {
        CLOGD("DEBUG(%s):newAntiShake %d", "setParameters", newAntiShake);
        bool toggle = false;
        if (newAntiShake == 1)
            toggle = true;

        if (m_secCamera->setAntiShake(toggle) == false) {
            CLOGE("ERR(%s):Fail on m_secCamera->setAntiShake(%d)", __func__, newAntiShake);
            ret = UNKNOWN_ERROR;
        } else {
            m_params.set("anti-shake", newAntiShake);
        }
    }

    //VT mode
    int newVTMode = params.getInt("vtmode");
    if (0 <= newVTMode) {
        CLOGD("DEBUG(%s):newVTMode %d", "setParameters", newVTMode);
    } else {
        newVTMode = 0;
    }

    if (m_secCamera->setVtMode(newVTMode) == false) {
        CLOGE("ERR(%s):Fail on m_secCamera->setVtMode(%d)", __func__, newVTMode);
        ret = UNKNOWN_ERROR;
    } else {
        m_params.set("vtmode", newVTMode);
    }

    //gamma
    const char *strNewGamma = params.get("video_recording_gamma");
    if (strNewGamma != NULL) {
        CLOGD("DEBUG(%s):strNewGamma %s", "setParameters", strNewGamma);
        int newGamma = -1;
        if (!strcmp(strNewGamma, "off"))
            newGamma = 0;
        else if (!strcmp(strNewGamma, "on"))
            newGamma = 1;
        else {
            CLOGE("ERR(%s):unmatched gamma(%s)", __func__, strNewGamma);
            ret = UNKNOWN_ERROR;
        }

        if (0 <= newGamma) {
            bool toggle = false;
            if (newGamma == 1)
                toggle = true;

            if (m_secCamera->setGamma(toggle) == false) {
                CLOGE("ERR(%s):m_secCamera->setGamma(%s) fail", __func__, strNewGamma);
                ret = UNKNOWN_ERROR;
            }
        }
    }

    //slow ae
    const char *strNewSlowAe = params.get("slow_ae");
    if (strNewSlowAe != NULL) {
        CLOGD("DEBUG(%s):strNewSlowAe %s", "setParameters", strNewSlowAe);
        int newSlowAe = -1;

        if (!strcmp(strNewSlowAe, "off"))
            newSlowAe = 0;
        else if (!strcmp(strNewSlowAe, "on"))
            newSlowAe = 1;
        else {
            CLOGE("ERR(%s):unmatched slow_ae(%s)", __func__, strNewSlowAe);
            ret = UNKNOWN_ERROR;
        }

        if (0 <= newSlowAe) {
            bool toggle = false;
            if (newSlowAe == 1)
                toggle = true;
            if (m_secCamera->setSlowAE(newSlowAe) == false) {
                CLOGE("ERR(%s):m_secCamera->setSlowAE(%d) fail", __func__, newSlowAe);
                ret = UNKNOWN_ERROR;
            }
        }
    }

    if (flagRestartPreview) {
        if ((m_previewRunning == true) &&
            m_previewStartDeferred == false) {
            CLOGE("%s:preview is running, cannot change size and format! but I do", "setParameters");
            this->setPreviewWindow(m_previewWindow);
        }
    }

    // image unique id
    const char *oldImageUniqueId = m_params.get("imageuniqueid-value");
    if (oldImageUniqueId == NULL || strcmp(oldImageUniqueId, "") == 0) {

        const char *newImageUniqueId = m_secCamera->getImageUniqueId();
        if (newImageUniqueId && strcmp(newImageUniqueId, "") != 0) {
            CLOGD("DEBUG(%s):newImageUniqueId %s", "setParameters", newImageUniqueId);
            m_params.set("imageuniqueid-value", newImageUniqueId);
        }
    }

    m_restoreMsgType();

    CLOGD("DEBUG(%s):out ret(%d)", __func__, ret);

    return ret;
}

CameraParameters ExynosCameraHWImpl::getParameters() const
{
    return m_params;
}

status_t ExynosCameraHWImpl::sendCommand(int32_t command, int32_t arg1, int32_t arg2)
{
    switch (command) {
    case CAMERA_CMD_START_FACE_DETECTION:
    case CAMERA_CMD_STOP_FACE_DETECTION:
        if (m_secCamera->getMaxNumDetectedFaces() == 0) {
            CLOGE("ERR(%s):getMaxNumDetectedFaces == 0", __func__);
            return BAD_VALUE;
        }

        if (arg1 == CAMERA_FACE_DETECTION_SW) {
            CLOGE("ERR(%s):only support HW face dectection", __func__);
            return BAD_VALUE;
        }

        if (command == CAMERA_CMD_START_FACE_DETECTION) {
            CLOGD("sendCommand: CAMERA_CMD_START_FACE_DETECTION is called!");
            if (   m_secCamera->flagStartFaceDetection() == false
                && m_secCamera->startFaceDetection() == false) {
                CLOGE("ERR(%s):startFaceDetection() fail", __func__);
                return BAD_VALUE;
            }
        } else { // if (command == CAMERA_CMD_STOP_FACE_DETECTION)
            CLOGD("sendCommand: CAMERA_CMD_STOP_FACE_DETECTION is called!");
            if (   m_secCamera->flagStartFaceDetection() == true
                && m_secCamera->stopFaceDetection() == false) {
                CLOGE("ERR(%s):stopFaceDetection() fail", __func__);
                return BAD_VALUE;
            }
        }
        break;
    case CAMERA_CMD_SET_FLIP:
        CLOGD("sendCommand: CAMERA_CMD_SET_FLIP is called!");
        m_flip_horizontal = arg1;
    break;
#ifdef FD_ROTATION
    case CAMERA_CMD_DEVICE_ORIENTATION:
        CLOGD("sendCommand: CAMERA_CMD_DEVICE_ORIENTATION is called!%d", arg1);
        m_secCamera->setDeviceOrientation(arg1);
        break;
#endif
    case CAMERA_CMD_AUTOFOCUS_MACRO_POSITION:
        CLOGD("sendCommand: CAMERA_CMD_AUTOFOCUS_MACRO_POSITION is called!%d", arg1);
        m_secCamera->setAutoFocusMacroPosition(arg1);
        break;
    default:
        CLOGV("DEBUG(%s):unexpectect command(%d)", __func__, command);
        break;
    }
    return NO_ERROR;
}

void ExynosCameraHWImpl::release()
{
    CLOGD("DEBUG(%s):in", __func__);

    /* shut down any threads we have that might be running.  do it here
     * instead of the destructor.  we're guaranteed to be on another thread
     * than the ones below.  if we used the destructor, since the threads
     * have a reference to this object, we could wind up trying to wait
     * for ourself to exit, which is a deadlock.
     */

#ifdef START_HW_THREAD_ENABLE
    if (m_startThreadMain != NULL) {
        m_startThreadMainLock.lock();
        m_startThreadMain->requestExit();
        m_exitStartThreadMain = true;
        m_startThreadMainCondition.signal();
        m_startThreadMainLock.unlock();
        m_startThreadMain->requestExitAndWait();
        m_startThreadMain.clear();
    }

    if (m_startThreadReprocessing != NULL) {
        m_startThreadReprocessingLock.lock();
        m_startThreadReprocessing->requestExit();
        m_exitStartThreadReprocessing = true;
        m_startThreadReprocessingCondition.signal();
        m_startThreadReprocessingLock.unlock();
        m_startThreadReprocessing->requestExitAndWait();
        m_startThreadReprocessing.clear();
    }

    if (m_startThreadBufAlloc != NULL) {
        m_startThreadBufAllocLock.lock();
        m_startThreadBufAlloc->requestExit();
        m_exitStartThreadBufAlloc = true;
        m_startThreadBufAllocCondition.signal();
        m_startThreadBufAllocLock.unlock();
        m_startThreadBufAlloc->requestExitAndWait();
        m_startThreadBufAlloc.clear();
    }
#endif

    if (m_videoThread != NULL) {
        m_videoThread->requestExit();
        m_exitVideoThread = true;
        m_videoRunning = true; // let it run so it can exit
        m_videoCondition.signal();
        m_videoThread->requestExitAndWait();
        m_videoThread.clear();
    }

    if (m_autoFocusThread != NULL) {
        m_autoFocusThread->requestExit();
        m_exitAutoFocusThread = true;
        m_autoFocusThread->requestExitAndWait();
        m_autoFocusThread.clear();
    }

    if (m_previewThread != NULL) {
        /*
         * this thread is normally already in it's threadLoop but blocked
         * on the condition variable or running.  signal it so it wakes
         * up and can exit.
         */
        m_previewThread->requestExit();
        m_previewRunning = false; // let it run so it can exit
        m_previewThread->requestExitAndWait();
        m_previewThread.clear();
    }

    if (m_ispThread != NULL) {
        m_ispLock.lock();
        m_ispThread->requestExit();
        m_exitIspThread = true;
        m_ispCondition.signal();
        m_ispLock.unlock();

        m_ispThread->requestExitAndWait();
        m_ispThread.clear();
    }

    if (m_pictureThread != NULL) {
        m_pictureThread->requestExitAndWait();
        m_pictureThread.clear();
    }

#ifdef FU_3INSTANCE
    if (m_pictureRunning == true) {
        if (m_stopPictureInternalReprocessing() == false)
            CLOGE("ERR(%s):m_stopPictureInternalReprocessing() fail", __func__);
    }
#else
    if (m_pictureRunning == true) {
        if (m_stopPictureInternal() == false)
            CLOGE("ERR(%s):m_stopPictureInternal() fail", __func__);
    }
#endif

    if (m_sensorThread != NULL) {
        m_sensorThread->requestExit();
        m_sensorRunning = false;
        m_sensorThread->requestExitAndWait();
        m_sensorThread.clear();
    }

#ifdef OTF_SENSOR_REPROCESSING
    if (m_sensorThreadReprocessing != NULL) {
        m_sensorThreadReprocessing->requestExit();
        m_sensorRunningReprocessing = false;
        m_sensorThreadReprocessing->requestExitAndWait();
        m_sensorThreadReprocessing.clear();
    }
#endif

    if (m_recordHeap != NULL) {
        m_recordHeap->release(m_recordHeap);
        m_recordHeap = 0;
    }

#ifdef USE_VDIS
    if (m_VDis != NULL) {
        m_VDis->stopVDisThread();
        delete m_VDis;
        m_VDis = NULL;
    }
#endif

    for (int i = 0; i < NUM_OF_VIDEO_BUF; i++) {
        if (m_videoHeap[i]) {
            m_videoHeap[i]->release(m_videoHeap[i]);
            m_videoHeap[i] = 0;
        }

        for (int j = 0; j < 2; j++) {
            if (m_resizedVideoHeap[i][j]) {
                m_resizedVideoHeap[i][j]->release(m_resizedVideoHeap[i][j]);
                m_resizedVideoHeap[i][j] = 0;
            }
        }
    }

    for (int i = 0; i < NUM_OF_PREVIEW_BUF; i++) {
        if (m_previewCallbackHeap[i]) {
            m_previewCallbackHeap[i]->release(m_previewCallbackHeap[i]);
            m_previewCallbackHeap[i] = 0;
            m_previewCallbackHeapFd[i] = -1;
        }
    }

    for (int i = 0; i < NUM_OF_PICTURE_BUF; i++) {
        if (m_pictureHeap[i]) {
            m_pictureHeap[i]->release(m_pictureHeap[i]);
            m_pictureHeap[i] = 0;
            m_pictureHeapFd[i] = -1;
        }
    }

    if (m_rawHeap) {
        m_rawHeap->release(m_rawHeap);
        m_rawHeap = 0;
        m_rawHeapFd = -1;
        m_rawHeapSize = 0;
    }

    if (m_jpegHeap) {
        m_jpegHeap->release(m_jpegHeap);
        m_jpegHeap = 0;
        m_jpegHeapFd = -1;
    }

    if (m_exynosVideoCSC)
        csc_deinit(m_exynosVideoCSC);
    m_exynosVideoCSC = NULL;

    if (m_exynosPictureCSC)
        csc_deinit(m_exynosPictureCSC);
    m_exynosPictureCSC = NULL;

    if (m_exynosPreviewCSC)
        csc_deinit(m_exynosPreviewCSC);
    m_exynosPreviewCSC = NULL;

     /* close after all the heaps are cleared since those
     * could have dup'd our file descriptor.
     */
    if (m_secCamera) {
        if (m_secCamera->flagCreate() == true)
            m_secCamera->destroy();

        delete m_secCamera;
        m_secCamera = NULL;
    }

#ifdef FORCE_LEADER_OFF
    tryThreadStop = true;
#endif

    CLOGD("DEBUG(%s):out", __func__);
}

status_t ExynosCameraHWImpl::dump(int fd) const
{
    const size_t SIZE = 256;
    char buffer[SIZE];
    String8 result;
    const Vector<String16> args;

    if (m_secCamera != 0) {
        m_params.dump(fd, args);
        snprintf(buffer, 255, " preview running(%s)\n", m_previewRunning?"true": "false");
        result.append(buffer);
    } else {
        result.append("No camera client yet.\n");
    }

    write(fd, result.string(), result.size());
    return NO_ERROR;
}

int ExynosCameraHWImpl::getCameraId() const
{
    return m_cameraId;
}

void ExynosCameraHWImpl::m_disableMsgType(int32_t msgType, bool restore)
{
    CLOGD("DEBUG(%s):msgType = 0x%x, m_storedMsg = 0x%x, m_msgEnabled(0x%x -> 0x%x)",
        __func__, msgType, m_storedMsg, m_msgEnabled, m_msgEnabled & ~msgType);

    if (m_storedMsg == 0 && m_msgEnabled != 0) {
        if (restore == true) {
            m_storedMsg = m_msgEnabled;
            m_msgEnabled &= ~msgType;
        } else {
            m_msgEnabled &= ~msgType;
            m_storedMsg = m_msgEnabled;
        }
    }
}

void ExynosCameraHWImpl::m_restoreMsgType(void)
{
    CLOGD("DEBUG(%s):m_storedMsg = 0x%x, m_msgEnabled(0x%x -> 0x%x)",
         __func__, m_storedMsg, m_msgEnabled, m_storedMsg);

    if (m_storedMsg != 0) {
        m_msgEnabled = m_storedMsg;
        m_storedMsg = 0;
    }
}

bool ExynosCameraHWImpl::m_initSecCamera(int cameraId)
{
    CLOGD("DEBUG(%s):in", __func__);

    if (m_secCamera != NULL) {
        CLOGE("ERR(%s):m_secCamera object is NULL", __func__);
        return false;
    }

    m_secCamera = new ExynosCamera;

    if (m_secCamera->create(cameraId) == false) {
        CLOGE("ERR(%s):Fail on m_secCamera->create(%d)", __func__, cameraId);
        return false;
    }

#ifdef START_HW_THREAD_ENABLE
    m_startThreadMain = new StartThreadMain(this);
    m_startThreadReprocessing = new StartThreadReprocessing(this);
    m_startThreadBufAlloc = new StartThreadBufAlloc(this);
#endif

    m_previewThread = new CameraThread(this, &ExynosCameraHWImpl::m_previewThreadFunc);
    m_videoThread = new VideoThread(this);
    m_autoFocusThread = new AutoFocusThread(this);
    m_pictureThread = new PictureThread(this);

    m_sensorThread = new CameraThread(this, &ExynosCameraHWImpl::m_sensorThreadFuncWrap);
    m_ispThread = new CameraThread(this, &ExynosCameraHWImpl::m_ispThreadFunc);

    if (cameraId == 0)
        m_sensorThreadReprocessing = new CameraThread(this, &ExynosCameraHWImpl::m_sensorThreadFuncReprocessing);

    CLOGD("DEBUG(%s):out", __func__);

    return true;
}

void ExynosCameraHWImpl::m_initDefaultParameters(int cameraId)
{
    if (m_secCamera == NULL) {
        CLOGE("ERR(%s):m_secCamera object is NULL", __func__);
        return;
    }

    CameraParameters p;

    String8 parameterString;

    char * cameraName;
    cameraName = m_secCamera->getCameraName();
    if (cameraName == NULL)
        CLOGE("ERR(%s):getCameraName() fail", __func__);

    char strBuf[256];
    String8 listString;

    // preview
    int previewMaxW  = 0;
    int previewMaxH  = 0;

    m_secCamera->getSupportedPreviewSizes(&previewMaxW, &previewMaxH);

    listString.setTo("");
    if (m_getResolutionList(listString, &previewMaxW, &previewMaxH, MODE_PREVIEW) == false) {
        CLOGE("ERR(%s):m_getResolutionList() fail", __func__);

        previewMaxW = 640;
        previewMaxH = 480;
        listString = String8::format("%dx%d", previewMaxW, previewMaxH);
    }

    p.set(CameraParameters::KEY_SUPPORTED_PREVIEW_SIZES, listString.string());
    CLOGD("DEBUG(%s): Default preview size is %dx%d", __func__, previewMaxW, previewMaxH);
    p.setPreviewSize(previewMaxW, previewMaxH);

    listString.setTo("");
    listString = String8::format("%s,%s", CameraParameters::PIXEL_FORMAT_YUV420SP, CameraParameters::PIXEL_FORMAT_YUV420P);
    p.set(CameraParameters::KEY_SUPPORTED_PREVIEW_FORMATS, listString);
    p.setPreviewFormat(CameraParameters::PIXEL_FORMAT_YUV420SP);

    // video
    int videoMaxW = 0;
    int videoMaxH = 0;

    m_secCamera->getSupportedVideoSizes(&videoMaxW, &videoMaxH);

    listString.setTo("");
    if (m_getResolutionList(listString, &videoMaxW, &videoMaxH, MODE_VIDEO) == false) {
        CLOGE("ERR(%s):m_getResolutionList() fail", __func__);

        videoMaxW = 640;
        videoMaxH = 480;
        listString = String8::format("%dx%d", videoMaxW, videoMaxH);
    }
    p.set(CameraParameters::KEY_SUPPORTED_VIDEO_SIZES, listString.string());
    CLOGD("DEBUG(%s): Default video size is %dx%d", __func__, videoMaxW, videoMaxH);
    p.setVideoSize(videoMaxW, videoMaxH);

    int cropX, cropY, cropW, cropH = 0;
    m_secCamera->getCropRect(previewMaxW,  previewMaxH,
                             videoMaxW,    videoMaxH,
                             &cropX, &cropY, &cropW, &cropH,
                             0);

    listString.setTo("");
    listString = String8::format("%dx%d", previewMaxW, previewMaxH);
    p.set(CameraParameters::KEY_PREFERRED_PREVIEW_SIZE_FOR_VIDEO, listString.string());

    p.set(CameraParameters::KEY_VIDEO_FRAME_FORMAT, CameraParameters::PIXEL_FORMAT_YUV420SP);

    if (m_secCamera->isVideoSnapshotSupported() == true)
        p.set(CameraParameters::KEY_VIDEO_SNAPSHOT_SUPPORTED, "true");
    else
        p.set(CameraParameters::KEY_VIDEO_SNAPSHOT_SUPPORTED, "false");

    if (m_secCamera->isVideoStabilizationSupported() == true)
        p.set(CameraParameters::KEY_VIDEO_STABILIZATION_SUPPORTED, "true");
    else
        p.set(CameraParameters::KEY_VIDEO_STABILIZATION_SUPPORTED, "false");

    // picture
    int pictureMaxW = 0;
    int pictureMaxH = 0;
    int matchedW = 0;
    int matchedH = 0;

    m_secCamera->getSupportedPictureSizes(&pictureMaxW, &pictureMaxH);

    listString.setTo("");
    if (m_getResolutionList(listString, &pictureMaxW, &pictureMaxH, MODE_PICTURE) == false) {
        CLOGE("ERR(%s):m_getResolutionList() fail", __func__);

        pictureMaxW = 640;
        pictureMaxW = 480;
        listString = String8::format("%dx%d", pictureMaxW, pictureMaxH);
    }
    p.set(CameraParameters::KEY_SUPPORTED_PICTURE_SIZES, listString.string());

    if (m_getMatchedPictureSize(previewMaxW, previewMaxH, &matchedW, &matchedH) == false) {
        CLOGW("WRN(%s): Could not found matched picture size, set the max size(%dx%d)", __func__, pictureMaxW, pictureMaxH);
        p.setPictureSize(pictureMaxW, pictureMaxH);
    } else {
        CLOGD("DEBUG(%s): Default picture size is %dx%d", __func__, matchedW, matchedH);
        p.setPictureSize(matchedW, matchedH);
    }

    p.set(CameraParameters::KEY_SUPPORTED_PICTURE_FORMATS,
          CameraParameters::PIXEL_FORMAT_JPEG);

    p.setPictureFormat(CameraParameters::PIXEL_FORMAT_JPEG);

    p.set(CameraParameters::KEY_JPEG_QUALITY, "100"); // maximum quality

    // thumbnail
    int thumbnailMaxW = 0;
    int thumbnailMaxH = 0;

    m_secCamera->getSupportedJpegThumbnailSizes(&thumbnailMaxW, &thumbnailMaxH);
    listString.setTo("");
    if (m_getResolutionList(listString, &thumbnailMaxW, &thumbnailMaxH, MODE_PICTURE) == false) {
        listString = String8::format("%dx%d", thumbnailMaxW, thumbnailMaxH);
    }
    /*
     * listString = String8::format("%dx%d", thumbnailMaxW, thumbnailMaxH);
     * listString.append(",320x180");
     */
    listString.append(",0x0");

    p.set(CameraParameters::KEY_SUPPORTED_JPEG_THUMBNAIL_SIZES, listString.string());
    p.set(CameraParameters::KEY_JPEG_THUMBNAIL_WIDTH,  thumbnailMaxW);
    p.set(CameraParameters::KEY_JPEG_THUMBNAIL_HEIGHT, thumbnailMaxH);
    p.set(CameraParameters::KEY_JPEG_THUMBNAIL_QUALITY, "100");

    // exposure
    p.set(CameraParameters::KEY_MIN_EXPOSURE_COMPENSATION, m_secCamera->getMinExposureCompensation());
    p.set(CameraParameters::KEY_MAX_EXPOSURE_COMPENSATION, m_secCamera->getMaxExposureCompensation());
    p.set(CameraParameters::KEY_EXPOSURE_COMPENSATION, m_secCamera->getExposureCompensation());
    p.setFloat(CameraParameters::KEY_EXPOSURE_COMPENSATION_STEP, m_secCamera->getExposureCompensationStep());

    if (m_secCamera->isAutoExposureLockSupported() == true)
        p.set(CameraParameters::KEY_AUTO_EXPOSURE_LOCK_SUPPORTED, "true");
    else
        p.set(CameraParameters::KEY_AUTO_EXPOSURE_LOCK_SUPPORTED, "false");

    // face detection
    p.set(CameraParameters::KEY_MAX_NUM_DETECTED_FACES_HW, m_secCamera->getMaxNumDetectedFaces());
    p.set(CameraParameters::KEY_MAX_NUM_DETECTED_FACES_SW, 0);

    // focus mode
    int focusMode = m_secCamera->getSupportedFocusModes();
    parameterString.setTo("");
    if (focusMode & ExynosCamera::FOCUS_MODE_AUTO) {
        parameterString.append(CameraParameters::FOCUS_MODE_AUTO);
        parameterString.append(",");
    }
    if (focusMode & ExynosCamera::FOCUS_MODE_INFINITY) {
        parameterString.append(CameraParameters::FOCUS_MODE_INFINITY);
        parameterString.append(",");
    }
    if (focusMode & ExynosCamera::FOCUS_MODE_MACRO) {
        parameterString.append(CameraParameters::FOCUS_MODE_MACRO);
        parameterString.append(",");
    }
    if (focusMode & ExynosCamera::FOCUS_MODE_FIXED) {
        parameterString.append(CameraParameters::FOCUS_MODE_FIXED);
        parameterString.append(",");
    }
    if (focusMode & ExynosCamera::FOCUS_MODE_EDOF) {
        parameterString.append(CameraParameters::FOCUS_MODE_EDOF);
        parameterString.append(",");
    }
    if (focusMode & ExynosCamera::FOCUS_MODE_CONTINUOUS_VIDEO) {
        parameterString.append(CameraParameters::FOCUS_MODE_CONTINUOUS_VIDEO);
        parameterString.append(",");
    }
    if (focusMode & ExynosCamera::FOCUS_MODE_CONTINUOUS_PICTURE) {
        parameterString.append(CameraParameters::FOCUS_MODE_CONTINUOUS_PICTURE);
        parameterString.append(",");
    }
    if (focusMode & ExynosCamera::FOCUS_MODE_CONTINUOUS_PICTURE_MACRO)
        parameterString.append("continuous-picture-macro");

    p.set(CameraParameters::KEY_SUPPORTED_FOCUS_MODES,
          parameterString.string());

    if (focusMode & ExynosCamera::FOCUS_MODE_AUTO)
        p.set(CameraParameters::KEY_FOCUS_MODE,
              CameraParameters::FOCUS_MODE_AUTO);
    else if (focusMode & ExynosCamera::FOCUS_MODE_CONTINUOUS_PICTURE)
        p.set(CameraParameters::KEY_FOCUS_MODE,
              CameraParameters::FOCUS_MODE_CONTINUOUS_PICTURE);
    else if (focusMode & ExynosCamera::FOCUS_MODE_CONTINUOUS_VIDEO)
        p.set(CameraParameters::KEY_FOCUS_MODE,
              CameraParameters::FOCUS_MODE_CONTINUOUS_VIDEO);
    else if (focusMode & ExynosCamera::FOCUS_MODE_INFINITY)
        p.set(CameraParameters::KEY_FOCUS_MODE,
              CameraParameters::FOCUS_MODE_INFINITY);
    else
        p.set(CameraParameters::KEY_FOCUS_MODE,
          CameraParameters::FOCUS_MODE_FIXED);

    // HACK
    if (cameraId == ExynosCamera::CAMERA_ID_BACK) {
        p.set(CameraParameters::KEY_FOCUS_DISTANCES,
              BACK_CAMERA_AUTO_FOCUS_DISTANCES_STR);
        p.set(CameraParameters::FOCUS_DISTANCE_INFINITY,
              BACK_CAMERA_FOCUS_DISTANCE_INFINITY);
    } else {
        p.set(CameraParameters::KEY_FOCUS_DISTANCES,
              FRONT_CAMERA_FOCUS_DISTANCES_STR);
        p.set(CameraParameters::FOCUS_DISTANCE_INFINITY,
              FRONT_CAMERA_FOCUS_DISTANCE_INFINITY);
    }

    p.set(CameraParameters::KEY_MAX_NUM_FOCUS_AREAS, 0);
    if (focusMode & ExynosCamera::FOCUS_MODE_TOUCH) {
        p.set(CameraParameters::KEY_MAX_NUM_FOCUS_AREAS, 1);
        p.set(CameraParameters::KEY_FOCUS_AREAS, "(0,0,0,0,0)");
    }

    // flash
    int flashMode = m_secCamera->getSupportedFlashModes();
    parameterString.setTo("");
    if (flashMode & ExynosCamera::FLASH_MODE_OFF) {
        parameterString.append(CameraParameters::FLASH_MODE_OFF);
        parameterString.append(",");
    }
    if (flashMode & ExynosCamera::FLASH_MODE_AUTO) {
        parameterString.append(CameraParameters::FLASH_MODE_AUTO);
        parameterString.append(",");
    }
    if (flashMode & ExynosCamera::FLASH_MODE_ON) {
        parameterString.append(CameraParameters::FLASH_MODE_ON);
        parameterString.append(",");
    }
    if (flashMode & ExynosCamera::FLASH_MODE_RED_EYE) {
        parameterString.append(CameraParameters::FLASH_MODE_RED_EYE);
        parameterString.append(",");
    }
    if (flashMode & ExynosCamera::FLASH_MODE_TORCH)
        parameterString.append(CameraParameters::FLASH_MODE_TORCH);

    p.set(CameraParameters::KEY_SUPPORTED_FLASH_MODES, parameterString.string());
    p.set(CameraParameters::KEY_FLASH_MODE, CameraParameters::FLASH_MODE_OFF);

    // scene mode
    int sceneMode = m_secCamera->getSupportedSceneModes();
    parameterString.setTo("");
    if (sceneMode & ExynosCamera::SCENE_MODE_AUTO) {
        parameterString.append(CameraParameters::SCENE_MODE_AUTO);
        parameterString.append(",");
    }
    if (sceneMode & ExynosCamera::SCENE_MODE_ACTION) {
        parameterString.append(CameraParameters::SCENE_MODE_ACTION);
        parameterString.append(",");
    }
    if (sceneMode & ExynosCamera::SCENE_MODE_PORTRAIT) {
        parameterString.append(CameraParameters::SCENE_MODE_PORTRAIT);
        parameterString.append(",");
    }
    if (sceneMode & ExynosCamera::SCENE_MODE_LANDSCAPE) {
        parameterString.append(CameraParameters::SCENE_MODE_LANDSCAPE);
        parameterString.append(",");
    }
    if (sceneMode & ExynosCamera::SCENE_MODE_NIGHT) {
        parameterString.append(CameraParameters::SCENE_MODE_NIGHT);
        parameterString.append(",");
    }
    if (sceneMode & ExynosCamera::SCENE_MODE_NIGHT_PORTRAIT) {
        parameterString.append(CameraParameters::SCENE_MODE_NIGHT_PORTRAIT);
        parameterString.append(",");
    }
    if (sceneMode & ExynosCamera::SCENE_MODE_THEATRE) {
        parameterString.append(CameraParameters::SCENE_MODE_THEATRE);
        parameterString.append(",");
    }
    if (sceneMode & ExynosCamera::SCENE_MODE_BEACH) {
        parameterString.append(CameraParameters::SCENE_MODE_BEACH);
        parameterString.append(",");
    }
    if (sceneMode & ExynosCamera::SCENE_MODE_SNOW) {
        parameterString.append(CameraParameters::SCENE_MODE_SNOW);
        parameterString.append(",");
    }
    if (sceneMode & ExynosCamera::SCENE_MODE_SUNSET) {
        parameterString.append(CameraParameters::SCENE_MODE_SUNSET);
        parameterString.append(",");
    }
    if (sceneMode & ExynosCamera::SCENE_MODE_STEADYPHOTO) {
        parameterString.append(CameraParameters::SCENE_MODE_STEADYPHOTO);
        parameterString.append(",");
    }
    if (sceneMode & ExynosCamera::SCENE_MODE_FIREWORKS) {
        parameterString.append(CameraParameters::SCENE_MODE_FIREWORKS);
        parameterString.append(",");
    }
    if (sceneMode & ExynosCamera::SCENE_MODE_SPORTS) {
        parameterString.append(CameraParameters::SCENE_MODE_SPORTS);
        parameterString.append(",");
    }
    if (sceneMode & ExynosCamera::SCENE_MODE_PARTY) {
        parameterString.append(CameraParameters::SCENE_MODE_PARTY);
        parameterString.append(",");
    }
    if (sceneMode & ExynosCamera::SCENE_MODE_CANDLELIGHT)
        parameterString.append(CameraParameters::SCENE_MODE_CANDLELIGHT);

    p.set(CameraParameters::KEY_SUPPORTED_SCENE_MODES,
          parameterString.string());
    p.set(CameraParameters::KEY_SCENE_MODE,
          CameraParameters::SCENE_MODE_AUTO);

    // effect
    int effect = m_secCamera->getSupportedColorEffects();
    parameterString.setTo("");
    if (effect & ExynosCamera::EFFECT_NONE) {
        parameterString.append(CameraParameters::EFFECT_NONE);
        parameterString.append(",");
    }
    if (effect & ExynosCamera::EFFECT_MONO) {
        parameterString.append(CameraParameters::EFFECT_MONO);
        parameterString.append(",");
    }
    if (effect & ExynosCamera::EFFECT_NEGATIVE) {
        parameterString.append(CameraParameters::EFFECT_NEGATIVE);
        parameterString.append(",");
    }
    if (effect & ExynosCamera::EFFECT_SOLARIZE) {
        parameterString.append(CameraParameters::EFFECT_SOLARIZE);
        parameterString.append(",");
    }
    if (effect & ExynosCamera::EFFECT_SEPIA) {
        parameterString.append(CameraParameters::EFFECT_SEPIA);
        parameterString.append(",");
    }
    if (effect & ExynosCamera::EFFECT_POSTERIZE) {
        parameterString.append(CameraParameters::EFFECT_POSTERIZE);
        parameterString.append(",");
    }
    if (effect & ExynosCamera::EFFECT_WHITEBOARD) {
        parameterString.append(CameraParameters::EFFECT_WHITEBOARD);
        parameterString.append(",");
    }
    if (effect & ExynosCamera::EFFECT_BLACKBOARD) {
        parameterString.append(CameraParameters::EFFECT_BLACKBOARD);
        parameterString.append(",");
    }
    if (effect & ExynosCamera::EFFECT_AQUA)
        parameterString.append(CameraParameters::EFFECT_AQUA);

    p.set(CameraParameters::KEY_SUPPORTED_EFFECTS, parameterString.string());
    p.set(CameraParameters::KEY_EFFECT, CameraParameters::EFFECT_NONE);

    // white balance
    int whiteBalance = m_secCamera->getSupportedWhiteBalance();
    parameterString.setTo("");
    if (whiteBalance & ExynosCamera::WHITE_BALANCE_AUTO) {
        parameterString.append(CameraParameters::WHITE_BALANCE_AUTO);
        parameterString.append(",");
    }
    if (whiteBalance & ExynosCamera::WHITE_BALANCE_INCANDESCENT) {
        parameterString.append(CameraParameters::WHITE_BALANCE_INCANDESCENT);
        parameterString.append(",");
    }
    if (whiteBalance & ExynosCamera::WHITE_BALANCE_FLUORESCENT) {
        parameterString.append(CameraParameters::WHITE_BALANCE_FLUORESCENT);
        parameterString.append(",");
    }
    if (whiteBalance & ExynosCamera::WHITE_BALANCE_WARM_FLUORESCENT) {
        parameterString.append(CameraParameters::WHITE_BALANCE_WARM_FLUORESCENT);
        parameterString.append(",");
    }
    if (whiteBalance & ExynosCamera::WHITE_BALANCE_DAYLIGHT) {
        parameterString.append(CameraParameters::WHITE_BALANCE_DAYLIGHT);
        parameterString.append(",");
    }
    if (whiteBalance & ExynosCamera::WHITE_BALANCE_CLOUDY_DAYLIGHT) {
        parameterString.append(CameraParameters::WHITE_BALANCE_CLOUDY_DAYLIGHT);
        parameterString.append(",");
    }
    if (whiteBalance & ExynosCamera::WHITE_BALANCE_TWILIGHT) {
        parameterString.append(CameraParameters::WHITE_BALANCE_TWILIGHT);
        parameterString.append(",");
    }
    if (whiteBalance & ExynosCamera::WHITE_BALANCE_SHADE)
        parameterString.append(CameraParameters::WHITE_BALANCE_SHADE);

    p.set(CameraParameters::KEY_SUPPORTED_WHITE_BALANCE,
          parameterString.string());
    p.set(CameraParameters::KEY_WHITE_BALANCE, CameraParameters::WHITE_BALANCE_AUTO);

    if (m_secCamera->isAutoWhiteBalanceLockSupported() == true)
        p.set(CameraParameters::KEY_AUTO_WHITEBALANCE_LOCK_SUPPORTED, "true");
    else
        p.set(CameraParameters::KEY_AUTO_WHITEBALANCE_LOCK_SUPPORTED, "false");

    // anti banding
    parameterString.setTo("");
    int antiBanding = m_secCamera->getSupportedAntibanding();
    if (antiBanding & ExynosCamera::ANTIBANDING_AUTO) {
        parameterString.append(CameraParameters::ANTIBANDING_AUTO);
        parameterString.append(",");
    }
    if (antiBanding & ExynosCamera::ANTIBANDING_50HZ) {
        parameterString.append(CameraParameters::ANTIBANDING_50HZ);
        parameterString.append(",");
    }
    if (antiBanding & ExynosCamera::ANTIBANDING_60HZ) {
        parameterString.append(CameraParameters::ANTIBANDING_60HZ);
        parameterString.append(",");
    }
    if (antiBanding & ExynosCamera::ANTIBANDING_OFF)
        parameterString.append(CameraParameters::ANTIBANDING_OFF);

    p.set(CameraParameters::KEY_SUPPORTED_ANTIBANDING,
          parameterString.string());

    p.set(CameraParameters::KEY_ANTIBANDING, CameraParameters::ANTIBANDING_AUTO);

    // rotation
    p.set(CameraParameters::KEY_ROTATION, 0);

    // view angle
    p.setFloat(CameraParameters::KEY_HORIZONTAL_VIEW_ANGLE, m_secCamera->getHorizontalViewAngle());
    p.setFloat(CameraParameters::KEY_VERTICAL_VIEW_ANGLE, m_secCamera->getVerticalViewAngle());

    // metering
    if (0 < m_secCamera->getMaxNumMeteringAreas()) {
        p.set(CameraParameters::KEY_MAX_NUM_METERING_AREAS, m_secCamera->getMaxNumMeteringAreas());
        p.set(CameraParameters::KEY_METERING_AREAS, "(0,0,0,0,1000)");
    }

    // zoom
    if (m_secCamera->isZoomSupported() == true) {

        int maxZoom = m_secCamera->getMaxZoom();
        if (0 < maxZoom) {

            p.set(CameraParameters::KEY_ZOOM_SUPPORTED, "true");

            if (m_secCamera->isSmoothZoomSupported() == true)
                p.set(CameraParameters::KEY_SMOOTH_ZOOM_SUPPORTED, "true");
            else
                p.set(CameraParameters::KEY_SMOOTH_ZOOM_SUPPORTED, "false");

            p.set(CameraParameters::KEY_MAX_ZOOM, maxZoom - 1);
            p.set(CameraParameters::KEY_ZOOM, m_secCamera->getZoom());

            int max_zoom_ratio = m_secCamera->getMaxZoomRatio();

            listString.setTo("");

            if (m_getZoomRatioList(listString, maxZoom, 100, max_zoom_ratio) == true)
                p.set(CameraParameters::KEY_ZOOM_RATIOS, listString.string());
            else
                p.set(CameraParameters::KEY_ZOOM_RATIOS, "100");
        } else {
            p.set(CameraParameters::KEY_ZOOM_SUPPORTED, "false");
            p.set(CameraParameters::KEY_SMOOTH_ZOOM_SUPPORTED, "false");
        }

    } else {
        p.set(CameraParameters::KEY_ZOOM_SUPPORTED, "false");
        p.set(CameraParameters::KEY_SMOOTH_ZOOM_SUPPORTED, "false");
    }

    // fps
    int minFpsRange, maxFpsRange;
    m_secCamera->getSupportedPreviewFpsRange(&minFpsRange, &maxFpsRange);

    listString.setTo("");

    int minFps = (minFpsRange == 0) ? 0 : (minFpsRange / 1000);
    int maxFps = (maxFpsRange == 0) ? 0 : (maxFpsRange / 1000);

    listString.setTo("");
    snprintf(strBuf, 256, "%d", minFps);
    listString.append(strBuf);

    for (int i = minFps + 1; i <= maxFps; i++) {
        snprintf(strBuf, 256, ",%d", i);
        listString.append(strBuf);
    }
    p.set(CameraParameters::KEY_SUPPORTED_PREVIEW_FRAME_RATES,  listString.string());

    listString.setTo("");
    m_getSupportedFpsList(listString, minFpsRange, maxFpsRange);
    p.set(CameraParameters::KEY_SUPPORTED_PREVIEW_FPS_RANGE, "(15000,30000),(30000,30000)");

    // limit 30 fps on default setting.
    if (30 < maxFps)
        maxFps = 30;
    p.setPreviewFrameRate(maxFps);

    if (30000 < maxFpsRange)
        maxFpsRange = 30000;
    snprintf(strBuf, 256, "%d,%d", maxFpsRange/2, maxFpsRange);
    p.set(CameraParameters::KEY_PREVIEW_FPS_RANGE, strBuf);

    // focal length
    int num = 0;
    int den = 0;
    int precision = 0;
    m_secCamera->getFocalLength(&num, &den);

    switch (den) {
    default:
    case 1000:
        precision = 3;
        break;
    case 100:
        precision = 2;
        break;
    case 10:
        precision = 1;
        break;
    case 1:
        precision = 0;
        break;
    }

    snprintf(strBuf, 256, "%.*f", precision, ((float)num / (float)den));
    p.set(CameraParameters::KEY_FOCAL_LENGTH, strBuf);
    //p.set(CameraParameters::KEY_FOCAL_LENGTH, "3.43");
    //p.set(CameraParameters::KEY_FOCAL_LENGTH, "0.9");

    // Additional params.

    p.set("contrast", "auto");
    p.set("iso", "auto");
    /* TODO: For 3rd party compatibility */
    //p.set("metering", "matrix");

    p.set("brightness", 0);
    p.set("brightness-max", 2);
    p.set("brightness-min", -2);

    p.set("saturation", 0);
    p.set("saturation-max", 2);
    p.set("saturation-min", -2);

    p.set("sharpness", 0);
    p.set("sharpness-max", 2);
    p.set("sharpness-min", -2);

    p.set("hue", 0);
    p.set("hue-max", 2);
    p.set("hue-min", -2);

    // fnumber
    m_secCamera->getFnumber(&num, &den);
    p.set("fnumber-value-numerator", num);
    p.set("fnumber-value-denominator", den);

    // max aperture value
    m_secCamera->getApertureValue(&num, &den);
    p.set("maxaperture-value-numerator", num);
    p.set("maxaperture-value-denominator", den);

    // focal length
    m_secCamera->getFocalLength(&num, &den);
    p.set("focallength-value-numerator", num);
    p.set("focallength-value-denominator", den);

    // focal length in 35mm film
    int focalLengthIn35mmFilm = 0;
    focalLengthIn35mmFilm = m_secCamera->getFocalLengthIn35mmFilm();
    p.set("focallength-35mm-value", focalLengthIn35mmFilm);

    m_params = p;

    /* make sure m_secCamera has all the settings we do.  applications
     * aren't required to call setParameters themselves (only if they
     * want to change something.
     */
    setParameters(p);
}

status_t ExynosCameraHWImpl::m_startSensor()
{
    CLOGD("DEBUG(%s):in", __func__);

    Mutex::Autolock lock(m_sensorLock);

    // HACK : move to startPreview(), because of seqeunce order.
    /*
    if (m_secCamera->startSensor() == false) {
        CLOGE("ERR(%s):Fail on m_secCamera->startSensor()", __func__);
        return UNKNOWN_ERROR;
    }
    */

    m_sensorRunning = true;
    m_sensorThread->run("CameraSensorThread", PRIORITY_DEFAULT);

    CLOGD("DEBUG(%s):out", __func__);

    return NO_ERROR;
}

void ExynosCameraHWImpl::m_stopSensor()
{
    CLOGD("DEBUG(%s):in", __func__);

    if (m_sensorRunning == true) {
        m_sensorRunning = false;

#ifdef USE_VDIS
        m_secCamera->setRecordingHint(false);
#endif
            if (0 < m_secCamera->getNumOfShotedFrame() &&
                m_secCamera->getCameraMode() == ExynosCamera::CAMERA_MODE_BACK) {
                CLOGD("DEBUG %s(%d), stop phase - %d frames are remained", __func__, __LINE__, m_secCamera->getNumOfShotedFrame());
            // break; // this should be modified... origital code has break;
            }

            if (m_secCamera->getCameraMode() == ExynosCamera::CAMERA_MODE_FRONT) {
                if (m_secCamera->setSensorStreamOff(ExynosCamera::CAMERA_MODE_FRONT) == false)
                    CLOGE("ERR(%s):Fail on m_secCamera->setSensorStreamOff(ExynosCamera::CAMERA_MODE_FRONT)", __func__);

                if (m_secCamera->stopSensorOff(ExynosCamera::CAMERA_MODE_FRONT) == false)
                    CLOGE("ERR(%s):m_secCamera->stopSensorOff(ExynosCamera::CAMERA_MODE_FRONT) fail", __func__);

                if (m_secCamera->flagStartSensor() == true) {
                    if (m_secCamera->stopSensor() == false)
                        CLOGE("ERR(%s):m_secCamera->stopSensor() fail", __func__);
                }
            }

            // stop sensor 0-reprocessing if this mode is dual or back
            if (m_secCamera->getCameraMode() == ExynosCamera::CAMERA_MODE_BACK) {
                if (m_secCamera->setSensorStreamOff(ExynosCamera::CAMERA_MODE_REPROCESSING) == false)
                    CLOGE("ERR(%s):Fail on m_secCamera->setSensorStreamOff(ExynosCamera::CAMERA_MODE_REPROCESSING)", __func__);

                if (m_secCamera->stopSensorOff(ExynosCamera::CAMERA_MODE_REPROCESSING) == false)
                    CLOGE("ERR(%s):Fail on m_secCamera->stopSensorOff(ExynosCamera::CAMERA_MODE_REPROCESSING)", __func__);

                if (m_secCamera->flagStartSensor() == true) {
                    if (m_secCamera->stopSensor() == false)
                        CLOGE("ERR(%s:%d):m_secCamera is not created.. Something is wrong", __func__, __LINE__);
                }

                if (m_secCamera->flagStartSensorReprocessing() == true) {
                    if (m_secCamera->stopSensorReprocessing() == false)
                        CLOGE("ERR(%s:%d):m_secCamera is not created.. Something is wrong", __func__, __LINE__);
                }
            }
    } else {
        CLOGV("DEBUG(%s):sensor not running, doing nothing", __func__);
    }

    CLOGD("DEBUG(%s):out", __func__);
}

bool ExynosCameraHWImpl::m_sensorThreadFuncWrap(void)
{
    bool ret = false;

    int cameraMode = m_secCamera->getCameraMode();

    switch (cameraMode) {
    case ExynosCamera::CAMERA_MODE_BACK:
        ret = m_sensorThreadFuncOTF();
        break;
    case ExynosCamera::CAMERA_MODE_FRONT:
        ret = m_sensorThreadFuncM2M();
        break;
    default:
        CLOGE("ERR(%s):invalid cameraMode(%d)", __func__, cameraMode);
        ret = false;
        break;
    }

#ifdef USE_CAMERA_ESD_RESET
    /* thread stop */
    if (ret == false && m_sensorESDReset == true) {
        CLOGE("ERR(%s):[esdreset]:ret == false && m_sensorESDReset == true", __func__);
        return false;
    }
#endif

    return true;
}

bool ExynosCameraHWImpl::m_sensorThreadFuncM2M(void)
{
    ExynosBuffer sensorBuf;
    ExynosBuffer ispBuf;
    struct camera2_shot_ext *shot_ext;
    bool getSenBufDone = true;

#ifdef USE_CAMERA_ESD_RESET
    if (m_sensorESDReset == true) {
        usleep(33000);
        return false;
    }
#endif

#ifdef FORCE_LEADER_OFF
    if (tryThreadStop == true) {
        tryThreadStatus = tryThreadStatus | (1 << TRY_THREAD_STATUS_SENSOR) | (1 << TRY_THREAD_STATUS_BAYER);
        usleep(10000);

        CLOGD("DEBUG(%s):(%d):tryThreadStop == true", __func__, __LINE__);
        return false;
    }
#endif

    if (m_sensorRunning == true &&
        m_secCamera->getNotifyStopMsg() == false) {

        m_sensorLock.lock();

        if (m_secCamera->getSensorBuf(&sensorBuf) == false) {
            m_sensorLock.unlock();
            if (CHECK_THREADHOLD(m_sensorErrCnt)) {
                CLOG_ASSERT("ERR(%s): getSensorBuf() fail [%d times]!!!", __func__, m_sensorErrCnt);
                return false;
            } else {
                m_sensorErrCnt++;
                CLOGE("ERR(%s):getSensorBuf() fail", __func__);
                usleep(10000);
            }
            return true;
        } else {
            m_sensorErrCnt = 0;
        }

        if (sensorBuf.reserved.p < 0) {
            CLOGV("DEBUG(%s): sensor index(%d)", __func__, sensorBuf.reserved.p);
            getSenBufDone = false;
        } else {
            m_sharedBayerBuffer = sensorBuf;
            shot_ext = (struct camera2_shot_ext *)(sensorBuf.virt.extP[1]);
            m_sharedBayerFcount = shot_ext->shot.dm.request.frameCount;
            getSenBufDone = true;
        }

        m_sensorLock.unlock();
    } else {
        if (m_secCamera->getNumOfShotedFrame() <= 0) {
            CLOGD("DEBUG(%s): getnumOfShoted frame %d", __func__, m_secCamera->getNumOfShotedFrame());
            m_sensorRunning = false;
        }
        CLOGV("DEBUG(%s): stop phase", __func__);
        getSenBufDone = false;
    }

    shot_ext = (struct camera2_shot_ext *)(sensorBuf.virt.extP[1]);

    if (m_secCamera->getCameraMode() == ExynosCamera::CAMERA_MODE_BACK) {
        ExynosCameraHWImpl::g_is3a1Mutex.lock();

        if (m_secCamera->getIs3a1Buf(ExynosCamera::CAMERA_MODE_BACK, &sensorBuf, &ispBuf) == false) {
            CLOGE("ERR(%s):getIs3a1Buf() fail(id:%d)", __func__, getCameraId());

            ExynosCameraHWImpl::g_is3a1Mutex.unlock();
            goto done;
        }
        ExynosCameraHWImpl::g_is3a1Mutex.unlock();
    } else if (m_secCamera->getCameraMode() == ExynosCamera::CAMERA_MODE_FRONT) {
        ExynosCameraHWImpl::g_is3a0Mutex.lock();

        if (m_secCamera->getIs3a0Buf(ExynosCamera::CAMERA_MODE_FRONT, &sensorBuf, &ispBuf) == false) {
            CLOGE("ERR(%s):getIs3a0Buf() fail(id:%d)", __func__, getCameraId());

            ExynosCameraHWImpl::g_is3a0Mutex.unlock();
            goto done;
        }
        ExynosCameraHWImpl::g_is3a0Mutex.unlock();
    }

    if (ispBuf.reserved.p < 0) {
        CLOGW("WRN(%s): ispBuf.reserved.p = %d", __func__, ispBuf.reserved.p);
        goto done;
    }

    shot_ext = (struct camera2_shot_ext *)ispBuf.virt.extP[1];
    isp_last_frame_cnt = shot_ext->shot.dm.request.frameCount;

    CLOGV("(%d) (%d) (%d) (%d)", shot_ext->free_cnt, shot_ext->request_cnt, shot_ext->process_cnt, shot_ext->complete_cnt);

    m_sharedISPBuffer = ispBuf;
    if (m_secCamera->putISPBuf(&ispBuf) == false) {
        CLOGE("ERR(%s):putISPBuf() fail", __func__);
        return false;
    }

    isp_input_count++;
    m_ispCondition.signal();

done:
    if (getSenBufDone == true) {
        m_sensorLock.lock();
        if (m_secCamera->putSensorBuf(&sensorBuf) == false) {
            CLOGE("ERR(%s):putSensorBuf() fail", __func__);
            m_sensorLock.unlock();
            return false;
        }
        m_sensorLock.unlock();
    }

    usleep(10);

    return true;
}

bool ExynosCameraHWImpl::m_sensorThreadFuncOTF(void)
{
    ExynosBuffer sensorBuf;
    ExynosBuffer ispBuf;
    struct camera2_shot_ext *shot_ext;
    bool getSenBufDone = true;

    if (m_sensorRunning == false)
        return false;

#ifdef USE_CAMERA_ESD_RESET
    if (m_sensorESDReset == true) {
        usleep(33000);
        return false;
    }
#endif

#ifdef FORCE_LEADER_OFF
    if (tryThreadStop == true) {
        tryThreadStatus = tryThreadStatus | (1 << TRY_THREAD_STATUS_SENSOR);
        usleep(10000);

        CLOGD("DEBUG(%s):(%d):tryThreadStop == true", __func__, __LINE__);
        return false;
    }
#endif
    if (m_secCamera->getCameraMode() == ExynosCamera::CAMERA_MODE_BACK) {
        ExynosCameraHWImpl::g_is3a1Mutex.lock();

        if (m_secCamera->getIs3a1Buf(ExynosCamera::CAMERA_MODE_BACK, &sensorBuf, &ispBuf) == false) {
            CLOGE("ERR(%s):getIs3a1Buf() fail(id:%d)", __func__, getCameraId());

            ExynosCameraHWImpl::g_is3a1Mutex.unlock();
            goto done;
        }
        ExynosCameraHWImpl::g_is3a1Mutex.unlock();
        if (m_sensorRunning == false)
            return false;
    } else if (m_secCamera->getCameraMode() == ExynosCamera::CAMERA_MODE_FRONT) {
        ExynosCameraHWImpl::g_is3a0Mutex.lock();

        if (m_secCamera->getIs3a0Buf(ExynosCamera::CAMERA_MODE_FRONT, &sensorBuf, &ispBuf) == false) {
            CLOGE("ERR(%s):getIs3a0Buf() fail(id:%d)", __func__, getCameraId());

            ExynosCameraHWImpl::g_is3a0Mutex.unlock();
            goto done;
        }

        ExynosCameraHWImpl::g_is3a0Mutex.unlock();
        if (m_sensorRunning == false)
            return false;
    }

    if (ispBuf.reserved.p < 0) {
        CLOGW("WRN(%s): ispBuf.reserved.p = %d", __func__, ispBuf.reserved.p);
        goto done;
    }

    shot_ext = (struct camera2_shot_ext *)ispBuf.virt.extP[1];
    isp_last_frame_cnt = shot_ext->shot.dm.request.frameCount;

    CLOGV("(%d) (%d) (%d) (%d)", shot_ext->free_cnt, shot_ext->request_cnt, shot_ext->process_cnt, shot_ext->complete_cnt);

    m_sharedISPBuffer = ispBuf;
    if (m_secCamera->putISPBuf(&ispBuf) == false) {
        CLOGE("ERR(%s):putISPBuf() fail", __func__);
        return false;
    }
    isp_input_count++;
    m_ispCondition.signal();

done:
    if (m_secCamera->getCameraMode() != ExynosCamera::CAMERA_MODE_FRONT)
        usleep(10);

    return true;
}

status_t ExynosCameraHWImpl::m_startSensorReprocessing()
{
    Mutex::Autolock lock(m_sensorLockReprocessing);

    m_sensorRunningReprocessing = true;
    m_sensorThreadReprocessing->run("CameraSensorThreadReprocessing", PRIORITY_DEFAULT);

    return NO_ERROR;
}

void ExynosCameraHWImpl::m_stopSensorReprocessing()
{
    if (m_sensorRunningReprocessing == true) {
        m_sensorRunningReprocessing = false;
        m_sensorThreadReprocessing->requestExitAndWait();
    } else
        CLOGV("DEBUG(%s):sensor not running, doing nothing", __func__);
}

bool ExynosCameraHWImpl::m_sensorThreadFuncReprocessing(void)
{
    ExynosBuffer sensorBuf;
    struct camera2_shot_ext *shot_ext;
    bool getSenBufDone = true;

#ifdef USE_CAMERA_ESD_RESET
    if (m_sensorESDReset == true) {
        usleep(33000);
        return false;
    }
#endif
#ifdef FORCE_LEADER_OFF
    if (tryThreadStop == true) {
        tryThreadStatus = tryThreadStatus | (1 << TRY_THREAD_STATUS_BAYER);
        usleep(10000);

        CLOGD("DEBUG(%s):(%d):tryThreadStop == true", __func__, __LINE__);
        return false;
    }
#endif
    if (m_sensorRunningReprocessing == true) {
        m_sensorLockReprocessing.lock();

        if (m_secCamera->getSensorBuf(&sensorBuf) == false) {
            m_sensorLockReprocessing.unlock();
            if (CHECK_THREADHOLD(m_sensorErrCnt)) {
                CLOG_ASSERT("ERR(%s): getSensorBuf() fail [%d times]!!!", __func__, m_sensorErrCnt);
                return false;
            } else {
                m_sensorErrCnt++;
                CLOGE("ERR(%s):getSensorBuf() fail", __func__);
                usleep(10000);
            }
            return true;
        } else {
            m_sensorErrCnt = 0;
        }

        if (sensorBuf.reserved.p < 0) {
            CLOGE("DEBUG(%s): sensor index(%d)", __func__, sensorBuf.reserved.p);
            getSenBufDone = false;
        } else {
            m_sharedBayerBuffer = sensorBuf;

            shot_ext = (struct camera2_shot_ext *)(sensorBuf.virt.extP[1]);
            m_sharedBayerFcount = shot_ext->shot.dm.request.frameCount;
            getSenBufDone = true;
        }

        m_sensorLockReprocessing.unlock();
    } else
        return false;

    if (getSenBufDone == true) {
        m_sensorLockReprocessing.lock();
        if (m_secCamera->putSensorBuf(&sensorBuf) == false) {
            CLOGE("ERR(%s):putSensorBuf() fail", __func__);
            m_sensorLockReprocessing.unlock();
            return true;
        }
        m_sensorLockReprocessing.unlock();
    }

    usleep(10);

    return true;
}

bool ExynosCameraHWImpl::m_ispThreadFunc(void)
{
    ExynosBuffer ispBuf;
#ifdef FORCE_LEADER_OFF
    if (tryThreadStop == true) {
        tryThreadStatus = tryThreadStatus | (1 << TRY_THREAD_STATUS_ISP);
        usleep(10000);

        CLOGD("DEBUG(%s):(%d):tryThreadStop == true", __func__, __LINE__);

        return false;
    }
#endif
    m_ispLock.lock();
    if (m_exitIspThread == true) {
        m_ispLock.unlock();
#ifdef FORCE_LEADER_OFF
        tryThreadStatus = tryThreadStatus | (1 << TRY_THREAD_STATUS_ISP);
#endif

        CLOGD("DEBUG(%s):exit 0", __func__);
        return false;
    }

    m_ispCondition.wait(m_ispLock);

    if (m_exitIspThread == true) {
        m_ispLock.unlock();
#ifdef FORCE_LEADER_OFF
        tryThreadStatus = tryThreadStatus | (1 << TRY_THREAD_STATUS_ISP);
#endif

        CLOGD("DEBUG(%s):exit 1", __func__);
        return false;
    }
    m_ispLock.unlock();

    if (isp_input_count > 1)
        CLOGV("[%s] (%d) (%d)", __func__, __LINE__, isp_input_count);

    do {
/* TODO: Check isp input count */
#if 0
        if (isp_input_count < 1) {
            CLOGW("WRN[%s](%d) isp_input_count(%d)", __func__, __LINE__, isp_input_count);
            break;
        }
#endif

        if (m_secCamera->getISPBuf(&ispBuf) == false) {
            CLOGE("ERR(%s):getISPBuf() fail", __func__);
            m_ispLock.unlock();
            return true;
        }
        isp_input_count--;
    } while (m_secCamera->getNumOfShotedIspFrame());

    if (m_secCamera->getCameraMode() != ExynosCamera::CAMERA_MODE_FRONT)
        usleep(10);

    return true;
}

#ifdef START_HW_THREAD_ENABLE
bool ExynosCameraHWImpl::m_startThreadFuncBufAlloc(void)
{
    CLOGD("DEBUG(%s):in", __func__);

     m_startThreadBufAllocLock.lock();
    /* check early exit request */
    if (m_exitStartThreadBufAlloc == true) {
        m_startThreadBufAllocLock.unlock();
        CLOGV("DEBUG(%s):exiting on request0", __func__);
        return true;
    }

    m_startThreadBufAllocCondition.wait(m_startThreadBufAllocLock);
    /* check early exit request */
    if (m_exitStartThreadBufAlloc == true) {
        m_startThreadBufAllocLock.unlock();
        CLOGV("DEBUG(%s):exiting on request1", __func__);
        return true;
    }
    m_startThreadBufAllocLock.unlock();

    CLOGD("DEBUG(%s):(%d)", __func__, __LINE__);

    m_startThreadBufAllocFinished = false;
    m_errorExistInStartThreadBufAlloc = false;
    if (m_getPreviewCallbackBuffer() == false) {
        CLOGE("ERR(%s):Fail on m_getPreviewCallbackBuffer()", __func__);
        m_errorExistInStartThreadBufAlloc = true;
        goto done;
    }

done:
    m_startThreadBufAllocFinishLock.lock();
    if (m_startThreadBufAllocWaiting == true) {
        CLOGD("DEBUG(%s):before send signal finished ThreadBufAlloc (%d)", __func__, __LINE__);
        m_startThreadBufAllocFinishCondition.signal();
    }
    m_startThreadBufAllocFinished = true;
    m_startThreadBufAllocFinishLock.unlock();

    CLOGD("DEBUG(%s):out", __func__);
    return true;
}

bool ExynosCameraHWImpl::m_startThreadFuncReprocessing(void)
{
    CLOGD("DEBUG(%s):in", __func__);

    m_startThreadReprocessingLock.lock();
    /* check early exit request */
    if (m_exitStartThreadReprocessing == true) {
        m_startThreadReprocessingLock.unlock();
        CLOGV("DEBUG(%s):exiting on request0", __func__);
        return true;
    }

    m_startThreadReprocessingCondition.wait(m_startThreadReprocessingLock);
    /* check early exit request */
    if (m_exitStartThreadReprocessing == true) {
        m_startThreadReprocessingLock.unlock();
        CLOGV("DEBUG(%s):exiting on request1", __func__);
        return true;
    }
    m_startThreadReprocessingLock.unlock();

    CLOGD("DEBUG(%s):(%d)", __func__, __LINE__);

    m_startThreadReprocessingFinished = false;
    m_errorExistInStartThreadReprocessing = false;
    m_startThreadReprocessingRunning = true;

    int i;
    int previewCallbackFramesize = 0;
    ExynosBuffer callbackBuf;

    m_getAlignedYUVSize(m_orgPreviewRect.colorFormat, m_orgPreviewRect.w, m_orgPreviewRect.h, &callbackBuf, true);

    for (i = 0; i < ExynosBuffer::BUFFER_PLANE_NUM_DEFAULT; i++)
        previewCallbackFramesize += callbackBuf.size.extS[i];

    for (i = 0; i < NUM_OF_PREVIEW_BUF; i++) {
        if (m_previewCallbackHeap[i]) {
            m_previewCallbackHeap[i]->release(m_previewCallbackHeap[i]);
            m_previewCallbackHeap[i] = 0;
            m_previewCallbackHeapFd[i] = -1;
        }

        m_previewCallbackHeap[i] = m_getMemoryCb(-1, previewCallbackFramesize, 1, &(m_previewCallbackHeapFd[i]));
        CLOGV("callback size!!!!!!!!!!!!!! %d", previewCallbackFramesize);
        if (!m_previewCallbackHeap[i] || m_previewCallbackHeapFd[i] < 0) {
            CLOGE("ERR(%s):m_getMemoryCb(m_previewCallbackHeap[%d], size(%d) fail", __func__, i, previewCallbackFramesize);
            continue;
        }
    }

    if (m_secCamera->getCameraMode() == ExynosCamera::CAMERA_MODE_BACK) {
        if (m_errorExistInStartThreadMain == false &&
            m_secCamera->startIspReprocessing() == false) {
            CLOGE("ERR(%s):Fail on m_secCamera->startIspReprocessing()", __func__);
            m_errorExistInStartThreadReprocessing = true;
            goto done;
        }

        if (m_errorExistInStartThreadMain == false &&
            m_secCamera->startIs3a0(ExynosCamera::CAMERA_MODE_REPROCESSING) == false) {
            CLOGE("ERR(%s):Fail on m_secCamera->startIs3a0()", __func__);
            m_errorExistInStartThreadReprocessing = true;
            goto done;
        }

        if (m_errorExistInStartThreadMain == false &&
            m_pictureRunning== false
            && m_startPictureInternalReprocessing() == false)
            CLOGE("ERR(%s):m_startPictureInternalReprocessing() fail", __func__);

        if (m_errorExistInStartThreadMain == false &&
            m_secCamera->startPreviewReprocessing() == false) {
            CLOGE("ERR(%s):Fail on m_secCamera->startPreviewReprocessing()", __func__);
            m_errorExistInStartThreadReprocessing = true;
            goto done;
        }
    }

done:
    m_startThreadReprocessingFinishLock.lock();
    if (m_startThreadReprocessingFinishWaiting == true) {
        CLOGD("DEBUG(%s):before send signal finished startThreadReprocessing (%d)", __func__, __LINE__);
        m_startThreadReprocessingFinishCondition.signal();
    }
    m_startThreadReprocessingFinished = true;
    m_startThreadReprocessingFinishLock.unlock();

    CLOGD("DEBUG(%s):out", __func__);
    return true;
}

bool ExynosCameraHWImpl::m_startThreadFuncMain(void)
{
    CLOGD("DEBUG(%s):in", __func__);

    m_startThreadMainLock.lock();
    /* check early exit request */
    if (m_exitStartThreadMain == true) {
        m_startThreadMainLock.unlock();
        CLOGV("DEBUG(%s):exiting on request0", __func__);
        return true;
    }

    m_startThreadMainCondition.wait(m_startThreadMainLock);
    /* check early exit request */
    if (m_exitStartThreadMain == true) {
        m_startThreadMainLock.unlock();
        CLOGV("DEBUG(%s):exiting on request1", __func__);
        return true;
    }
    m_startThreadMainLock.unlock();

    CLOGD("DEBUG(%s):(%d)", __func__, __LINE__);

    m_errorExistInStartThreadMain = false;
    m_startThreadBufAllocWaiting = false;
    m_startThreadReprocessingFinishWaiting = false;

    if (m_startThreadMainRunning == true) {
        if (m_secCamera->getCameraMode() == ExynosCamera::CAMERA_MODE_FRONT) {
           if (m_secCamera->startSensor() == false) {
                CLOGE("ERR(%s):Fail on m_secCamera->startSensor()", __func__);
                m_errorExistInStartThreadMain = true;
                goto done;
            }

            if (m_secCamera->startSensorOn(ExynosCamera::CAMERA_MODE_FRONT) == false) {
                CLOGE("ERR(%s):Fail on m_secCamera->startSensorOn()", __func__);
                m_errorExistInStartThreadMain = true;
                goto done;
            }

            if (m_secCamera->startIsp() == false) {
                CLOGE("ERR(%s):Fail on m_secCamera->startIsp()", __func__);
                m_errorExistInStartThreadMain = true;
                goto done;
            }

            CLOGD("DEBUG(%s):before send signal to startThreadReprocessing (%d)", __func__, __LINE__);
            m_startThreadReprocessingCondition.signal();

            if (m_secCamera->startIs3a0(ExynosCamera::CAMERA_MODE_FRONT) == false) {
                CLOGE("ERR(%s):Fail on m_secCamera->startIs3a0()", __func__);
                m_errorExistInStartThreadMain = true;
                goto done;
            }

            if (m_pictureRunning == false &&
                m_startPictureInternal() == false)
                CLOGE("ERR(%s):m_startPictureInternal() fail", __func__);

        } else if (m_secCamera->getCameraMode() == ExynosCamera::CAMERA_MODE_BACK) {
            if (m_errorExistInStartThreadReprocessing == false &&
                m_secCamera->startSensorReprocessing() == false) {
                CLOGE("ERR(%s):Fail on m_secCamera->startSensorReprocessing()", __func__);
                m_errorExistInStartThreadMain = true;
                goto done;
            }

            /* Sensor stream on at the OTF path */
            if (m_errorExistInStartThreadReprocessing == false &&
                m_secCamera->startSensorOn(ExynosCamera::CAMERA_MODE_REPROCESSING) == false) {
                CLOGE("ERR(%s):Fail on m_secCamera->startSensorOn()", __func__);
                m_errorExistInStartThreadMain = true;
                goto done;
            }

            if (m_errorExistInStartThreadReprocessing == false &&
                m_secCamera->startIsp() == false) {
                CLOGE("ERR(%s):Fail on m_secCamera->startIsp()", __func__);
                m_errorExistInStartThreadMain = true;
                goto done;
            }

            CLOGD("DEBUG(%s):before send signal to startThreadReprocessing (%d)", __func__, __LINE__);
            m_startThreadReprocessingCondition.signal();

            if (m_errorExistInStartThreadReprocessing == false &&
                m_secCamera->startIs3a1(ExynosCamera::CAMERA_MODE_BACK) == false) {
                CLOGE("ERR(%s):Fail on m_secCamera->startIs3a1()", __func__);
                m_errorExistInStartThreadMain = true;
                goto done;
            }
        }

        if (m_secCamera->getCameraMode() == ExynosCamera::CAMERA_MODE_BACK) {
            if (m_errorExistInStartThreadReprocessing == false &&
                m_secCamera->startSensor() == false) {
                CLOGE("ERR(%s):Fail on m_secCamera->startSensor()", __func__);
                m_errorExistInStartThreadMain = true;
                goto done;
            }
        }

        m_startThreadBufAllocFinishLock.lock();
        if (m_startThreadBufAllocFinished == false) {
            m_startThreadBufAllocWaiting = true;
            CLOGD("DEBUG(%s):wait signal finished ThreadBufAlloc (%d)", __func__, __LINE__);
            m_startThreadBufAllocFinishCondition.wait(m_startThreadBufAllocFinishLock);

        }
        m_startThreadBufAllocWaiting = false;
        m_startThreadBufAllocFinishLock.unlock();

        if (m_errorExistInStartThreadReprocessing == false &&
            m_secCamera->startPreview() == false) {
            CLOGE("ERR(%s):Fail on m_secCamera->startPreview()", __func__);
            m_errorExistInStartThreadMain = true;
            goto done;
        }

        if (m_secCamera->getCameraMode() == ExynosCamera::CAMERA_MODE_BACK) {
            if (m_errorExistInStartThreadReprocessing == false &&
                m_secCamera->qAll3a1Buf() == false)
                CLOGE("ERR(%s):Fail qAll3a1Buf", __func__);
        }

        m_startThreadReprocessingFinishLock.lock();
        if (m_startThreadReprocessingFinished == false) {
            m_startThreadReprocessingFinishWaiting = true;
            CLOGD("DEBUG(%s):wait signal finished startThreadReprocessing (%d)", __func__, __LINE__);
            m_startThreadReprocessingFinishCondition.wait(m_startThreadReprocessingFinishLock);
        }
        m_startThreadReprocessingFinishWaiting = false;
        m_startThreadReprocessingFinishLock.unlock();

        if (m_errorExistInStartThreadReprocessing == false &&
            m_secCamera->StartStream() == false) {
            CLOGE("ERR(%s):StartStream fail", __func__);
            m_errorExistInStartThreadMain = true;
            goto done;
        }
#if CAPTURE_BUF_GET
        if (((m_secCamera->getCameraMode() == ExynosCamera::CAMERA_MODE_BACK) ||
            (m_secCamera->getCameraMode() == ExynosCamera::CAMERA_MODE_FRONT)) &&
            m_secCamera->getRecordingHint() != true)
        {
            ExynosBuffer tempBuf;
            if (m_errorExistInStartThreadReprocessing == false &&
                m_secCamera->getSensorBuf(&tempBuf) == false) {
                CLOGE("ERR(%s):getSensorBuf() fail", __func__);
                m_errorExistInStartThreadMain = true;
                goto done;
            }
            m_secCamera->putSensorBuf(&tempBuf);
        }
#endif

        m_setSkipFrame(INITIAL_SKIP_FRAME);

        if (m_errorExistInStartThreadReprocessing == false &&
            m_startSensor() != NO_ERROR) {
            CLOGE("ERR(%s):Fail on m_startSensor()", __func__);
            m_errorExistInStartThreadMain = true;
            goto done;
        }

#ifdef OTF_SENSOR_REPROCESSING
        if (m_secCamera->getCameraMode() == ExynosCamera::CAMERA_MODE_BACK) {
#ifdef DYNAMIC_BAYER_BACK_REC
            if ((m_secCamera->getRecordingHint() == true)
#ifdef SCALABLE_SENSOR
             || (m_secCamera->getScalableSensorStart() == true)
#endif
            ) {
                /* Dummy Start */
                m_sensorRunningReprocessing = true;
                isDqSensor = false;
            } else {
#endif /* DYNAMIC_BAYER_BACK_REC */
            if (m_errorExistInStartThreadReprocessing == false &&
                m_startSensorReprocessing() != NO_ERROR) {
                CLOGE("ERR(%s):Fail on m_startSensorReprocessing()", __func__);
                m_errorExistInStartThreadMain = true;
                goto done;
            }
#ifdef DYNAMIC_BAYER_BACK_REC
            }
#endif /* DYNAMIC_BAYER_BACK_REC */
#endif
        }

#ifdef USE_VDIS
        if ((m_secCamera->getRecordingHint() == true) &&
            (m_secCamera->getCameraMode() == ExynosCamera::CAMERA_MODE_BACK) &&
            (m_secCamera->getVideoStabilization() == true)) {
            m_VDis->startVDisInternal();
        }
#endif
    }

done:
    if (m_startThreadMainRunning == true) {
        m_startThreadMainRunning = false;
        CLOGD("DEBUG(%s):before send signal to startThreadMain (%d)", __func__, __LINE__);
        m_startThreadMainFinishLock.lock();
        m_startThreadMainFinishCondition.signal();
        m_startThreadMainFinishLock.unlock();
    }

    CLOGD("DEBUG(%s):out", __func__);

    return true;
}
#endif

bool ExynosCameraHWImpl::m_startPreviewInternal(void)
{
    CLOGD("DEBUG(%s):in", __func__);

    m_previewTimerIndex = 0;
    /* notify message for we are not in stop phase. */
    m_secCamera->notifyStop(false);

#ifdef START_HW_THREAD_ENABLE
    m_startThreadBufAllocWaiting = false;
    m_startThreadBufAllocFinished = false;
    m_errorExistInStartThreadBufAlloc = false;
    CLOGD("DEBUG(%s):before send signal to startThreadBufAlloc (%d)", __func__, __LINE__);
    m_startThreadBufAllocCondition.signal();

    m_errorExistInStartThreadMain = false;
    m_startThreadReprocessingFinishWaiting = false;
    m_startThreadMainRunning = true;
    m_startThreadReprocessingRunning = false;
    CLOGD("DEBUG(%s):before send signal to startThreadMain (%d)", __func__, __LINE__);
    m_startThreadMainCondition.signal();

    m_startThreadMainFinishLock.lock();
    CLOGD("DEBUG(%s):wait signal finishing startThreadMain (%d)", __func__, __LINE__);
    m_startThreadMainFinishCondition.wait(m_startThreadMainFinishLock);
    m_startThreadMainFinishLock.unlock();

    if (m_errorExistInStartThreadMain == true
        || m_errorExistInStartThreadReprocessing == true
        || m_errorExistInStartThreadBufAlloc == true) {

        m_startThreadBufAllocFinishLock.lock();
        if (m_startThreadBufAllocFinished == false) {
            m_startThreadBufAllocWaiting = true;
            CLOGD("DEBUG(%s):wait signal finished ThreadBufAlloc (%d)", __func__, __LINE__);
            m_startThreadBufAllocFinishCondition.wait(m_startThreadBufAllocFinishLock);
        }
        m_startThreadBufAllocWaiting = false;
        m_startThreadBufAllocFinishLock.unlock();

        m_startThreadReprocessingFinishLock.lock();
        if (m_startThreadReprocessingRunning == true && m_startThreadReprocessingFinished == false) {
            m_startThreadReprocessingFinishWaiting = true;
            CLOGD("DEBUG(%s):wait signal finished startThreadReprocessing (%d)", __func__, __LINE__);
            m_startThreadReprocessingFinishCondition.wait(m_startThreadReprocessingFinishLock);
        }
        m_startThreadReprocessingFinishWaiting = false;
        m_startThreadReprocessingFinishLock.unlock();

        CLOGE("ERR(%s):Fail on startThreadFuncReprocessing()", __func__);
        // 1. stop sensorthread
        // 2. stop sensor if mode is front camera
        CLOGD("DEBUG(%s):(%d) m_stopSensor try exit", __func__, __LINE__);
        if (m_secCamera->getCameraMode() == ExynosCamera::CAMERA_MODE_FRONT) {
            if (m_secCamera->setSensorStreamOff(ExynosCamera::CAMERA_MODE_FRONT) == false)
                CLOGE("ERR(%s):Fail on m_secCamera->setSensorStreamOff(ExynosCamera::CAMERA_MODE_FRONT)", __func__);
            if (m_secCamera->stopSensorOff(ExynosCamera::CAMERA_MODE_FRONT) == false)
                CLOGE("ERR(%s):Fail on m_secCamera->stopSensorOff(ExynosCamera::CAMERA_MODE_FRONT)", __func__);
            if (m_secCamera->stopSensor() == false)
                CLOGE("ERR(%s):Fail on m_secCamera->stopSensor()", __func__);
        }

        // stop sensorReprocessing thread if this mode is dual or back
        if (m_secCamera->getCameraMode() == ExynosCamera::CAMERA_MODE_BACK) {
            CLOGD("DEBUG(%s):(%d) m_stopSensorReprocessing try exit", __func__, __LINE__);
            m_stopSensorReprocessing();
            CLOGD("DEBUG(%s):(%d) m_stopSensorReprocessing was exited", __func__, __LINE__);
        }

        // stop sensor 0-reprocessing if this mode is dual or back
        if (m_secCamera->getCameraMode() == ExynosCamera::CAMERA_MODE_BACK) {
            if (m_secCamera->setSensorStreamOff(ExynosCamera::CAMERA_MODE_REPROCESSING) == false)
                CLOGE("ERR(%s):Fail on m_secCamera->setSensorStreamOff(ExynosCamera::CAMERA_MODE_REPROCESSING)", __func__);

            if (m_secCamera->stopSensorOff(ExynosCamera::CAMERA_MODE_REPROCESSING) == false)
                CLOGE("ERR(%s):Fail on m_secCamera->stopSensorOff(ExynosCamera::CAMERA_MODE_REPROCESSING)", __func__);

            if (m_secCamera->stopSensor() == false)
                CLOGE("ERR(%s:%d):m_secCamera is not created.. Something is wrong", __func__, __LINE__);

            if (m_secCamera->stopSensorReprocessing() == false)
                CLOGE("ERR(%s:%d):m_secCamera is not created.. Something is wrong", __func__, __LINE__);
        }

        // stop 3a1
        if (m_secCamera->stopIs3a1(ExynosCamera::CAMERA_MODE_FRONT) == false)
            CLOGE("ERR(%s):m_secCamera->stopIs3a1(ExynosCamera::CAMERA_MODE_FRONT) fail", __func__);

        if (m_secCamera->stopIs3a1(ExynosCamera::CAMERA_MODE_BACK) == false)
            CLOGE("ERR(%s):m_secCamera->stopIs3a1(ExynosCamera::CAMERA_MODE_BACK) fail", __func__);

        if (m_secCamera->stopIs3a1(ExynosCamera::CAMERA_MODE_REPROCESSING) == false)
            CLOGE("ERR(%s):m_secCamera->stopIs3a1(ExynosCamera::CAMERA_MODE_REPROCESSING) fail", __func__);

        // stop 3a0
        if (m_secCamera->stopIs3a0(ExynosCamera::CAMERA_MODE_FRONT) == false)
            CLOGE("ERR(%s):m_secCamera->stopIs3a0(ExynosCamera::CAMERA_MODE_FRONT) fail", __func__);

        if (m_secCamera->stopIs3a0(ExynosCamera::CAMERA_MODE_BACK) == false)
            CLOGE("ERR(%s):m_secCamera->stopIs3a0(ExynosCamera::CAMERA_MODE_BACK) fail", __func__);

        if (m_secCamera->stopIs3a0(ExynosCamera::CAMERA_MODE_REPROCESSING) == false)
            CLOGE("ERR(%s):m_secCamera->stopIs3a0(ExynosCamera::CAMERA_MODE_REPROCESSING) fail", __func__);

        if (m_secCamera->stopIsp() == false)
            CLOGE("ERR(%s):m_secCamera->stopIsp() fail", __func__);

        if (m_secCamera->stopIspReprocessing() == false)
            CLOGE("ERR(%s):m_secCamera->stopIspReprocessing() fail", __func__);

        if (m_secCamera->stopPreview() == false)
            CLOGE("ERR(%s):m_secCamera->stopPreview() fail", __func__);

        if (m_secCamera->stopPreviewReprocessing() == false)
            CLOGE("ERR(%s):m_secCamera->stopPreviewReprocessing() fail", __func__);

        if (m_secCamera->stopPicture() == false)
            CLOGE("ERR(%s):m_secCamera->stopPicture() fail", __func__);

        if (m_secCamera->stopPictureReprocessing() == false)
            CLOGE("ERR(%s):m_secCamera->stopPictureReprocessing() fail", __func__);

        m_releaseBuffer();
        CLOGD("DEBUG(%s):error exit (%d)", __func__, __LINE__);
        return false;
    }
#else
    if (m_getPreviewCallbackBuffer() == false) {
        CLOGE("ERR(%s):Fail on m_getPreviewCallbackBuffer()", __func__);
        return false;
    }

    if (m_startCameraHw() == false) {
        CLOGE("ERR(%s):Fail on startExynosCameraHw()", __func__);
        return false;
    }
#endif
    if (m_flashMode == ExynosCamera::FLASH_MODE_TORCH) {
        if (m_secCamera->setFlashMode(m_flashMode) == false)
            CLOGE("ERR(%s):setFlashMode() fail", __func__);
    }

    CLOGD("DEBUG(%s):out", __func__);
    return true;
}

void ExynosCameraHWImpl::m_releaseBuffer(void)
{
    CLOGD("DEBUG(%s):in", __func__);

    /* release preview buffer */
    if (m_previewWindow) {
        for (int i = 0; i < NUM_OF_PREVIEW_BUF; i++) {
            if (m_previewBufHandle[i] != NULL) {
                if (m_grallocHal && m_flagGrallocLocked[i] == true) {
                    m_grallocHal->unlock(m_grallocHal, *m_previewBufHandle[i]);
                    m_flagGrallocLocked[i] = false;
                }

                if (m_avaliblePreviewBufHandle[i] == true) {
                    if (m_previewWindow->cancel_buffer(m_previewWindow, m_previewBufHandle[i]) != 0) {
                        CLOGE("ERR(%s):Fail to cancel buffer(%d)", __func__, i);
                    } else {
                        m_previewBufHandle[i] = NULL;
                        m_previewStride[i] = 0;
                    }

                    m_avaliblePreviewBufHandle[i] = false;
                }
            }
            m_grallocVirtAddr[i] = NULL;
            m_matchedGrallocIndex[i] = -1;
        }

        m_numOfDequeuedBuf = 0;

        if (m_previewWindow->set_buffer_count(m_previewWindow, NUM_OF_PREVIEW_BUF) != 0) {
            CLOGE("ERR(%s):could not set buffer count", __func__);
        }
    } else {
        m_secCamera->releaseAllInternalPreviewBuf();
    }

    /* release buffer */
    if (m_recordHeap != NULL) {
        m_recordHeap->release(m_recordHeap);
        m_recordHeap = 0;
    }

    for (int i = 0; i < NUM_OF_VIDEO_BUF; i++) {
        if (m_videoHeap[i]) {
            m_videoHeap[i]->release(m_videoHeap[i]);
            m_videoHeap[i] = 0;
        }

        for (int j = 0; j < 2; j++) {
            if (m_resizedVideoHeap[i][j]) {
                m_resizedVideoHeap[i][j]->release(m_resizedVideoHeap[i][j]);
                m_resizedVideoHeap[i][j] = 0;
            }
        }
    }

    for (int i = 0; i < NUM_OF_PREVIEW_BUF; i++) {
        if (m_previewCallbackHeap[i]) {
            m_previewCallbackHeap[i]->release(m_previewCallbackHeap[i]);
            m_previewCallbackHeap[i] = 0;
            m_previewCallbackHeapFd[i] = -1;
        }
    }

    for (int i = 0; i < NUM_OF_PICTURE_BUF; i++) {
        if (m_pictureHeap[i]) {
            m_pictureHeap[i]->release(m_pictureHeap[i]);
            m_pictureHeap[i] = 0;
            m_pictureHeapFd[i] = -1;
        }
    }

    if (m_rawHeap) {
        m_rawHeap->release(m_rawHeap);
        m_rawHeap = 0;
        m_rawHeapFd = -1;
        m_rawHeapSize = 0;
    }

    if (m_jpegHeap) {
        m_jpegHeap->release(m_jpegHeap);
        m_jpegHeap = 0;
        m_jpegHeapFd = -1;
    }

    return;
}

bool ExynosCameraHWImpl::m_getPreviewCallbackBuffer(void)
{
    CLOGD("DEBUG(%s):in", __func__);

    int i, j;
    int previewW, previewH, previewFormat, previewFramesize, previewBufW, previewBufH;
    int previewCallbackFramesize = 0;
    void *virtAddr[3];

    m_secCamera->getPreviewSize(&previewW, &previewH);
    previewFormat = m_secCamera->getPreviewFormat();

    // previewCallbackFramesize for m_previewCallbackHeap[i]
    ExynosBuffer callbackBuf;
    m_getAlignedYUVSize(m_orgPreviewRect.colorFormat, m_orgPreviewRect.w, m_orgPreviewRect.h, &callbackBuf, true);

    for (i = 0; i < ExynosBuffer::BUFFER_PLANE_NUM_DEFAULT; i++)
        previewCallbackFramesize += callbackBuf.size.extS[i];

    CLOGV("callback size: %d", previewCallbackFramesize);

    if (m_previewWindow)
        m_secCamera->setFlagInternalPreviewBuf(false);
    else
        m_secCamera->setFlagInternalPreviewBuf(true);

    ExynosBuffer previewBuf;
    m_getAlignedYUVSize(previewFormat, previewW, previewH, &previewBuf);

    for (i = 0; i < 3; i++)
        virtAddr[i] = NULL;

    for (i = 0; i < NUM_OF_PREVIEW_BUF; i++) {
        m_avaliblePreviewBufHandle[i] = false;

        if (m_previewWindow) {
            if (m_previewWindow->dequeue_buffer(m_previewWindow, &m_previewBufHandle[i], &m_previewStride[i]) != 0) {
                CLOGE("ERR(%s):Could not dequeue gralloc buffer[%d]!!", __func__, i);
                continue;
            } else {
                if (m_previewBufHandle[i] == NULL)
                    continue;

                m_numOfDequeuedBuf++;

                 if (m_previewWindow->lock_buffer(m_previewWindow, m_previewBufHandle[i]) != 0)
                     CLOGE("ERR(%s):Could not lock gralloc buffer[%d]!!", __func__, i);
            }

            if (m_flagGrallocLocked[i] == false) {
                if (m_grallocHal->lock(m_grallocHal,
                                       *m_previewBufHandle[i],
                                       GRALLOC_LOCK_FOR_CAMERA,
                                       0, 0, previewW, previewH, virtAddr) != 0) {
                    CLOGE("ERR(%s):could not obtain gralloc buffer", __func__);
                    CLOGE("ERR(%s)(%d): gralloc buffer handle information is following;", __func__, __LINE__);
                    if (*m_previewBufHandle[i] != NULL)
                        CLOGE("ERR(%s):version: %d, numFds: %d, numInts: %d, data[0]: %d ",
                                        __func__,
                                        (*m_previewBufHandle[i])->version,
                                        (*m_previewBufHandle[i])->numFds,
                                        (*m_previewBufHandle[i])->numInts,
                                        (*m_previewBufHandle[i])->data[0]);
                    else
                        CLOGE("ERR(%s):bufHandle is null ", __func__);

                    CLOGE("ERR(%s):virtAddr[0]: 0x%08x", __func__, (unsigned int)virtAddr[0]);
                    CLOGE("ERR(%s):virtAddr[1]: 0x%08x", __func__, (unsigned int)virtAddr[1]);
                    CLOGE("ERR(%s):virtAddr[2]: 0x%08x", __func__, (unsigned int)virtAddr[2]);

                    if (m_previewWindow->cancel_buffer(m_previewWindow, m_previewBufHandle[i]) != 0)
                        CLOGE("ERR(%s):Could not cancel_buffer gralloc buffer[%d]!!", __func__, i);

                    continue;
                }

                m_grallocVirtAddr[i] = virtAddr[0];
                m_matchedGrallocIndex[i] = i;
                m_flagGrallocLocked[i] = true;
                m_avaliblePreviewBufHandle[i] = true;
            }

            const private_handle_t *priv_handle = private_handle_t::dynamicCast(*m_previewBufHandle[i]);
                previewBuf.fd.extFd[0] = priv_handle->fd;
                previewBuf.fd.extFd[1] = priv_handle->fd1;
                previewBuf.virt.extP[0] = (char *)virtAddr[0];
                previewBuf.virt.extP[1] = (char *)virtAddr[1];
        } else {
            /* Hal have to allocate internal memory for preview, due to window is NULL */
            if (m_secCamera->allocInternalPreviewBuf(&previewBuf) == false) {
                CLOGE("ERR(%s):Failed to alloc internal preview buffer", __func__);
                continue;
            }
        }

        previewBuf.reserved.p = i;

        m_secCamera->setPreviewBuf(&previewBuf);
        m_setPreviewBufStatus(i, ON_DRIVER);
    }

    if (m_previewWindow) {
        int res = 0;
        for (i = NUM_OF_PREVIEW_BUF - m_minUndequeuedBufs; i < NUM_OF_PREVIEW_BUF; i++) {
            if (m_flagGrallocLocked[i] == true) {
                m_grallocHal->unlock(m_grallocHal, *m_previewBufHandle[i]);
                m_flagGrallocLocked[i] = false;
            }

            res = m_previewWindow->cancel_buffer(m_previewWindow, m_previewBufHandle[i]);
            CLOGD("DEBUG(%s):cancel buffer %d result (%dnd buffer)", __func__, res, i);
            if (res != 0) {
                CLOGE("ERR(%s):could not cancel buffer %d error(%dnd)", __func__, res, i);
                return INVALID_OPERATION;
            }
            m_secCamera->cancelPreviewBuf(i);
            m_setPreviewBufStatus(i, ON_SERVICE);

            m_numOfDequeuedBuf--;
            if (m_numOfDequeuedBuf < 0)
                m_numOfDequeuedBuf = 0;
            m_avaliblePreviewBufHandle[previewBuf.reserved.p] = false;
        }
    }

#ifndef START_HW_THREAD_ENABLE
    for (i = 0; i < NUM_OF_PREVIEW_BUF; i++) {
        if (m_previewCallbackHeap[i]) {
            m_previewCallbackHeap[i]->release(m_previewCallbackHeap[i]);
            m_previewCallbackHeap[i] = 0;
            m_previewCallbackHeapFd[i] = -1;
        }

        m_previewCallbackHeap[i] = m_getMemoryCb(-1, previewCallbackFramesize, 1, &(m_previewCallbackHeapFd[i]));
        CLOGV("callback size: %d", previewCallbackFramesize);
        if (!m_previewCallbackHeap[i] || m_previewCallbackHeapFd[i] < 0) {
            CLOGE("ERR(%s):m_getMemoryCb(m_previewCallbackHeap[%d], size(%d) fail", __func__, i, previewCallbackFramesize);
            continue;
        }
    }
#endif

    CLOGD("DEBUG(%s):out", __func__);

    return true;
}
bool ExynosCameraHWImpl::m_startCameraHw(void)
{
    CLOGD("DEBUG(%s):in", __func__);

    if (m_secCamera->getCameraMode() == ExynosCamera::CAMERA_MODE_FRONT) {
        if (m_secCamera->startSensor() == false) {
            CLOGE("ERR(%s):Fail on m_secCamera->startSensor()", __func__);
            return UNKNOWN_ERROR;
        }

        if (m_secCamera->startSensorOn(ExynosCamera::CAMERA_MODE_FRONT) == false) {
            CLOGE("ERR(%s):Fail on m_secCamera->startSensorOn()", __func__);
            return UNKNOWN_ERROR;
        }

        if (m_secCamera->startIsp() == false) {
            CLOGE("ERR(%s):Fail on m_secCamera->startIsp()", __func__);
            return false;
        }

        if (m_secCamera->startIs3a0(ExynosCamera::CAMERA_MODE_FRONT) == false) {
            CLOGE("ERR(%s):Fail on m_secCamera->startIs3a0()", __func__);
            return false;
        }

        if (m_pictureRunning == false
            && m_startPictureInternal() == false)
            CLOGE("ERR(%s):m_startPictureInternal() fail", __func__);

        if (m_secCamera->startPreview() == false) {
            CLOGE("ERR(%s):Fail on m_secCamera->startPreview()", __func__);
            return false;
        }
    } else if (m_secCamera->getCameraMode() == ExynosCamera::CAMERA_MODE_BACK) {
        if (m_secCamera->startSensorReprocessing() == false) {
            CLOGE("ERR(%s):Fail on m_secCamera->startSensorReprocessing()", __func__);
            return UNKNOWN_ERROR;
        }

        /* Sensor stream on at the OTF path */
        if (m_secCamera->startSensorOn(ExynosCamera::CAMERA_MODE_REPROCESSING) == false) {
            CLOGE("ERR(%s):Fail on m_secCamera->startSensorOn()", __func__);
            return UNKNOWN_ERROR;
        }

        if (m_secCamera->startIsp() == false) {
            CLOGE("ERR(%s):Fail on m_secCamera->startIsp()", __func__);
            return false;
        }

        if (m_secCamera->startIs3a1(ExynosCamera::CAMERA_MODE_BACK) == false) {
            CLOGE("ERR(%s):Fail on m_secCamera->startIs3a1()", __func__);
            return false;
        }

        if (m_secCamera->startPreview() == false) {
            CLOGE("ERR(%s):Fail on m_secCamera->startPreview()", __func__);
            return false;
        }

        if (m_secCamera->startIspReprocessing() == false) {
            CLOGE("ERR(%s):Fail on m_secCamera->startIspReprocessing()", __func__);
            return false;
        }

        if (m_secCamera->startIs3a0(ExynosCamera::CAMERA_MODE_REPROCESSING) == false) {
            CLOGE("ERR(%s):Fail on m_secCamera->startIs3a0()", __func__);
            return false;
        }

        if (m_secCamera->startSensor() == false) {
            CLOGE("ERR(%s):Fail on m_secCamera->startSensor()", __func__);
            return UNKNOWN_ERROR;
        }

        if (m_pictureRunning== false
            && m_startPictureInternalReprocessing() == false)
            CLOGE("ERR(%s):m_startPictureInternalReprocessing() fail", __func__);

        if (m_secCamera->startPreviewReprocessing() == false) {
            CLOGE("ERR(%s):Fail on m_secCamera->startPreviewReprocessing()", __func__);
            return false;
        }
    }

    if (m_secCamera->getCameraMode() == ExynosCamera::CAMERA_MODE_BACK) {
        if (m_secCamera->qAll3a1Buf() == false)
            CLOGE("ERR(%s):Fail qAll3a1Buf", __func__);
    }

    if (m_secCamera->StartStream() == false) {
        CLOGE("ERR(%s):StartStream fail", __func__);
        return false;
    }
    m_setSkipFrame(INITIAL_SKIP_FRAME);

    if (m_startSensor() != NO_ERROR) {
        CLOGE("ERR(%s):Fail on m_startSensor()", __func__);
        return false;
    }

#ifdef OTF_SENSOR_REPROCESSING
    if (m_secCamera->getCameraMode() == ExynosCamera::CAMERA_MODE_BACK) {
        if (m_startSensorReprocessing() != NO_ERROR) {
            CLOGE("ERR(%s):Fail on m_startSensorReprocessing()", __func__);
            return false;
        }
    }
#endif

    CLOGD("DEBUG(%s):out", __func__);

    return true;
}

void ExynosCameraHWImpl::m_stopPreviewInternal(void)
{
    CLOGD("DEBUG(%s):in", __func__);

     /* request that the preview thread stop. */
    if (m_previewWindow) {
        for (int i = 0; i < NUM_OF_PREVIEW_BUF; i++) {
            if (m_previewBufHandle[i] != NULL) {
                 if (m_grallocHal && m_flagGrallocLocked[i] == true) {
                     m_grallocHal->unlock(m_grallocHal, *m_previewBufHandle[i]);
                     m_flagGrallocLocked[i] = false;
                }

                if (m_avaliblePreviewBufHandle[i] == true) {
                    if (m_previewWindow->cancel_buffer(m_previewWindow, m_previewBufHandle[i]) != 0) {
                        CLOGE("ERR(%s):Fail to cancel buffer(%d)", __func__, i);
                    } else {
                        m_previewBufHandle[i] = NULL;
                        m_previewStride[i] = 0;
                    }

                    m_avaliblePreviewBufHandle[i] = false;
                }
            }
            m_grallocVirtAddr[i] = NULL;
            m_matchedGrallocIndex[i] = -1;
        }

        m_numOfDequeuedBuf = 0;
        if (m_secCamera->clearPreviewBuf() == false) {
            CLOGE("ERR(%s):Failed to clearPreviewBuf()", __func__);
        }

        if (m_previewWindow->set_buffer_count(m_previewWindow, NUM_OF_PREVIEW_BUF) != 0) {
            CLOGE("ERR(%s):could not set buffer count", __func__);
        }
    } else {
        m_secCamera->releaseAllInternalPreviewBuf();
    }

    for (int i = 0; i < NUM_OF_PREVIEW_BUF; i++) {
        if (m_previewCallbackHeap[i]) {
            m_previewCallbackHeap[i]->release(m_previewCallbackHeap[i]);
            m_previewCallbackHeap[i] = 0;
            m_previewCallbackHeapFd[i] = -1;
        }
    }

#ifdef DYNAMIC_BAYER_BACK_REC
    isDqSensor = false;
#endif

    CLOGD("DEBUG(%s):out", __func__);
}

bool ExynosCameraHWImpl::m_previewThreadFunc(void)
{
    bool ret = true;
    int stride = 0;

    ExynosBuffer previewBuf;
    int skipFrameCount = 0;
    int previewW = 0, previewH = 0;
    bool doPutPreviewBuf = false;
    bool shouldEraseBack = false;
    bool isValid = true;

    bool needCSC = true;

    ExynosBuffer callbackBuf;
    camera_memory_t *previewCallbackHeap = NULL;
    int previewCallbackHeapFd = -1;

    bool flagPreviewCallback = false;
    ExynosBuffer ispBuf;
    nsecs_t previewBufTimestamp = 0;

    int previewFormat = m_secCamera->getPreviewFormat();

#ifdef USE_CAMERA_ESD_RESET
    if (m_sensorESDReset == true) {
        CLOGE("ERR(%s):[esdreset]:m_sensorESDReset == true", __func__);
        return false;
    }
#endif

#ifdef FORCE_LEADER_OFF
    if (tryThreadStop == true) {
        tryThreadStatus = tryThreadStatus | (1 << TRY_THREAD_STATUS_PREVIEW);
        usleep(10000);

        CLOGD("DEBUG(%s):(%d):tryThreadStop == true", __func__, __LINE__);
        return false;
    }
#endif
    if (m_secCamera->getPreviewBuf(&previewBuf, &isValid, &previewBufTimestamp) == false) {
        CLOGE("ERR(%s):getPreviewBuf() fail(id:%d)", __func__, getCameraId());

#ifdef FORCE_LEADER_OFF
        if (tryThreadStop == true) {
            ret = false;
            CLOGD("DEBUG(%s):(%d):tryThreadStop == true", __func__, __LINE__);
            goto done;
        }
#endif

#ifdef USE_CAMERA_ESD_RESET
        m_sensorESDReset = true;

        m_notifyCb(CAMERA_MSG_ERROR, 1001, 0, m_callbackCookie);
        CLOGE("ERR(%s):[esdreset]: notify error to app", __func__);
#endif

        ret = false;
        goto done;
    }

#ifdef FRONT_NO_ZSL
#else /* FRONT_NOS_ZSL */
    if (m_secCamera->getCameraMode() == ExynosCamera::CAMERA_MODE_FRONT) {
        if (m_secCamera->getPictureBuf(&m_pictureBuf[0]) == false) {
            CLOGE("ERR(%s):getPictureBuf() fail", __func__);
            ret = false;
            goto done;
        }

        if (m_secCamera->putPictureBuf(&m_pictureBuf[0]) == false) {
            CLOGE("ERR(%s):putPictureBuf(%d) fail", __func__, m_pictureBuf[0].reserved.p);
            ret = false;
            goto done;
        }
    }
#endif /* FRONT_NO_ZSL */

    m_setPreviewBufStatus(previewBuf.reserved.p, ON_HAL);
    m_pushPreviewQ(&previewBuf);
    doPutPreviewBuf = true;
    shouldEraseBack = true;

    if (isValid == false) {
        CLOGW("WARN(%s): Preview frame dropped", __func__);
        goto done;
    }

#ifdef CHECK_TIME_PREVIEW
    m_checkPreviewTime();
#endif

    skipFrameCount = m_getSkipFrame();

    if (0 < skipFrameCount) {
        m_decSkipFrame();
        CLOGV("DEBUG(%s):skipping %d frame", __func__, previewBuf.reserved.p);
        goto done;
    }
#ifdef CHECK_TIME_START_PREVIEW
    else if (skipFrameCount == 0){
        m_decSkipFrame();

        m_startPreviewTimer.stop();
        long long startPreviewDurationTime = m_startPreviewTimer.durationUsecs();
        CLOGD("DEBUG(%s):CHECK_TIME_START_PREVIEW : %d msec", __func__, (int)startPreviewDurationTime / 1000);
    }
#endif //CHECK_TIME_START_PREVIEW

    m_secCamera->getPreviewSize(&previewW, &previewH);

    /* callback & face detection */
    previewCallbackHeap = m_previewCallbackHeap[previewBuf.reserved.p];
    previewCallbackHeapFd = m_previewCallbackHeapFd[previewBuf.reserved.p];

    /* Face detection */
    if ((m_previewRunning == true) &&
        (m_msgEnabled & CAMERA_MSG_PREVIEW_METADATA) &&
        (m_secCamera->flagStartFaceDetection() == true ||
         m_faceDetected == true)) {
        if (m_secCamera->flagStartFaceDetection() == true) {
            int id[NUM_OF_DETECTED_FACES];
            int score[NUM_OF_DETECTED_FACES];
            ExynosRect2 detectedFace[NUM_OF_DETECTED_FACES];
            ExynosRect2 detectedLeftEye[NUM_OF_DETECTED_FACES];
            ExynosRect2 detectedRightEye[NUM_OF_DETECTED_FACES];
            ExynosRect2 detectedMouth[NUM_OF_DETECTED_FACES];

            int numOfDetectedFaces = m_secCamera->getDetectedFacesAreas(NUM_OF_DETECTED_FACES,
                                                                        id,
                                                                        score,
                                                                        detectedFace,
                                                                        detectedLeftEye,
                                                                        detectedRightEye,
                                                                        detectedMouth);

            if (0 < numOfDetectedFaces) {
                // camera.h
                // width   : -1000~1000
                // height  : -1000~1000
                // if eye, mouth is not detectable : -2000, -2000.
                memset(m_faces, 0, sizeof(camera_face_t) * NUM_OF_DETECTED_FACES);

                int realNumOfDetectedFaces = 0;

                for (int i = 0; i < numOfDetectedFaces; i++) {
                    // over 50s, we will catch
                    //if (score[i] < 50)
                    //    continue;

                    m_faces[realNumOfDetectedFaces].rect[0] = m_calibratePosition(previewW, 2000, detectedFace[i].x1) - 1000;
                    m_faces[realNumOfDetectedFaces].rect[1] = m_calibratePosition(previewH, 2000, detectedFace[i].y1) - 1000;
                    m_faces[realNumOfDetectedFaces].rect[2] = m_calibratePosition(previewW, 2000, detectedFace[i].x2) - 1000;
                    m_faces[realNumOfDetectedFaces].rect[3] = m_calibratePosition(previewH, 2000, detectedFace[i].y2) - 1000;

                    m_faces[realNumOfDetectedFaces].id = id[i];
                    m_faces[realNumOfDetectedFaces].score = score[i];
                    CLOGV("face posision(cal:%d,%d %dx%d)(org:%d,%d %dx%d), id(%d), score(%d)",
                                                 m_faces[realNumOfDetectedFaces].rect[0], m_faces[realNumOfDetectedFaces].rect[1],
                                                 m_faces[realNumOfDetectedFaces].rect[2], m_faces[realNumOfDetectedFaces].rect[3],
                                                 detectedFace[i].x1, detectedFace[i].y1,
                                                 detectedFace[i].x2, detectedFace[i].y2,
                                                 id[i], score[i]);

                    m_faces[realNumOfDetectedFaces].left_eye[0] = (detectedLeftEye[i].x1 < 0) ? -2000 : m_calibratePosition(previewW, 2000, detectedLeftEye[i].x1) - 1000;
                    m_faces[realNumOfDetectedFaces].left_eye[1] = (detectedLeftEye[i].y1 < 0) ? -2000 : m_calibratePosition(previewH, 2000, detectedLeftEye[i].y1) - 1000;

                    m_faces[realNumOfDetectedFaces].right_eye[0] = (detectedRightEye[i].x1 < 0) ? -2000 : m_calibratePosition(previewW, 2000, detectedRightEye[i].x1) - 1000;
                    m_faces[realNumOfDetectedFaces].right_eye[1] = (detectedRightEye[i].y1 < 0) ? -2000 : m_calibratePosition(previewH, 2000, detectedRightEye[i].y1) - 1000;

                    m_faces[realNumOfDetectedFaces].mouth[0] = (detectedMouth[i].x1 < 0) ? -2000 : m_calibratePosition(previewW, 2000, detectedMouth[i].x1) - 1000;
                    m_faces[realNumOfDetectedFaces].mouth[1] = (detectedMouth[i].y1 < 0) ? -2000 : m_calibratePosition(previewH, 2000, detectedMouth[i].y1) - 1000;

                    realNumOfDetectedFaces++;
                }

                m_frameMetadata.number_of_faces = realNumOfDetectedFaces;
                m_frameMetadata.faces = m_faces;

                m_faceDetected = true;
            } else {
                if (m_faceDetected == true && m_fdThreshold < NUM_OF_DETECTED_FACES_THRESHOLD) {
                    /* waiting the unexpected fail about face detected */
                    m_fdThreshold++;
                } else {
                    if (0 < m_frameMetadata.number_of_faces)
                        memset(m_faces, 0, sizeof(camera_face_t) * NUM_OF_DETECTED_FACES);

                    m_frameMetadata.number_of_faces = 0;
                    m_frameMetadata.faces = m_faces;

                    m_fdThreshold = 0;

                    m_faceDetected = false;
                }
            }
        } else {
            if (0 < m_frameMetadata.number_of_faces)
                memset(m_faces, 0, sizeof(camera_face_t) * NUM_OF_DETECTED_FACES);

            m_frameMetadata.number_of_faces = 0;
            m_frameMetadata.faces = m_faces;

            m_fdThreshold = 0;

            m_faceDetected = false;
        }

        if (m_msgEnabled & CAMERA_MSG_PREVIEW_METADATA)
            m_dataCb(CAMERA_MSG_PREVIEW_METADATA, previewCallbackHeap, 0, &m_frameMetadata, m_callbackCookie);
    } else {
        m_fdThreshold = 0;

        m_faceDetected = false;
    }

    if ((m_previewRunning == true) &&
        (m_msgEnabled & CAMERA_MSG_PREVIEW_FRAME)) {
         needCSC = m_secCamera->getCallbackCSC();

         if (m_doPreviewToCallbackFunc(previewBuf, &callbackBuf, needCSC) == false) {
             CLOGE("ERR(%s):Fail to doPreviewCallbackFunc", __func__);
             flagPreviewCallback = false;
         } else {
             flagPreviewCallback = true;
         }
    }

#ifndef USE_3DNR_DMAOUT
    if (m_videoRunning == true) {
        if (!(m_availableRecordingFrameCnt == 0 && m_sizeOfVideoQ() > 2) && !(m_sizeOfVideoQ() > 3)) {
                if (m_videoBufTimestamp[previewBuf.reserved.p] == 1) {
                    m_videoBufTimestamp[previewBuf.reserved.p] = previewBufTimestamp;
                    m_pushVideoQ(&previewBuf);
                }
                else
                    CLOGW("(%s): Dropping video frame(under processing) [%d]", __func__, previewBuf.reserved.p);
        } else {
                CLOGW("(%s): Dropping video frame m_sizeOfVideoQ(%d), m_availableRecordingFrameCnt(%d)",
                    __func__, m_sizeOfVideoQ(), m_availableRecordingFrameCnt);
        }

        m_videoLock.lock();
        m_videoCondition.signal();
        m_videoLock.unlock();
    }
#endif

    if (m_grallocHal && m_previewRunning == true) {
        if (m_previewWindow) {
            bool findGrallocBuf = false;
            buffer_handle_t *bufHandle = NULL;
            void *virtAddr[3];

            /* recording to LCD */
            /* Unlock grallocHal buffer if locked */
            if (m_flagGrallocLocked[previewBuf.reserved.p] == true) {
                m_grallocHal->unlock(m_grallocHal, *m_previewBufHandle[previewBuf.reserved.p]);
                m_flagGrallocLocked[previewBuf.reserved.p] = false;
            }

            if (m_previewWindow->set_timestamp(m_previewWindow, systemTime(SYSTEM_TIME_MONOTONIC)) != 0)
                CLOGE("ERR(%s):Could not set_timestamp gralloc", __func__);

            /* Enqueue lastest buffer */
            if (m_avaliblePreviewBufHandle[previewBuf.reserved.p] == true) {
                if (m_previewWindow->enqueue_buffer(m_previewWindow,
                                m_previewBufHandle[previewBuf.reserved.p]) != 0) {
                    CLOGE("ERR(%s):Could not enqueue gralloc buffer[%d]!!", __func__, previewBuf.reserved.p);
                    goto done;
                }
                m_setPreviewBufStatus(previewBuf.reserved.p, ON_SERVICE);

                m_numOfDequeuedBuf--;
                if (m_numOfDequeuedBuf < 0)
                    m_numOfDequeuedBuf = 0;
                m_avaliblePreviewBufHandle[previewBuf.reserved.p] = false;
            }

            /* Dequeue buffer from Gralloc */
            if (m_previewWindow->dequeue_buffer(m_previewWindow,
                                                &bufHandle,
                                                &stride) != 0) {
                CLOGE("ERR(%s):Could not dequeue gralloc buffer!!", __func__);
                goto done;
            } else {
                m_numOfDequeuedBuf++;

                if (m_previewWindow->lock_buffer(m_previewWindow, bufHandle) != 0)
                    CLOGE("ERR(%s):Could not lock gralloc buffer!!", __func__);
            }

            /* Get virtual address from dequeued buf */
            if (m_grallocHal->lock(m_grallocHal,
                                   *bufHandle,
                                   GRALLOC_LOCK_FOR_CAMERA,
                                   0, 0, previewW, previewH, virtAddr) != 0) {
                if (m_previewWindow->cancel_buffer(m_previewWindow, bufHandle) != 0)
                    CLOGE("ERR(%s):Fail to cancel buffer", __func__);

                m_numOfDequeuedBuf--;

                CLOGE("ERR(%s):could not obtain gralloc buffer", __func__);
                CLOGE("ERR(%s)(%d): gralloc buffer handle information is following;", __func__, __LINE__);
                if (*bufHandle != NULL)
                    CLOGE("ERR(%s):version: %d, numFds: %d, numInts: %d, data[0]: %d ",
                                __func__,
                                (*bufHandle)->version,
                                (*bufHandle)->numFds,
                                (*bufHandle)->numInts,
                                (*bufHandle)->data[0]);
                else
                    CLOGE("ERR(%s):bufHandle is null ", __func__);

                CLOGE("ERR(%s):virtAddr[0]: 0x%08x", __func__, (unsigned int)virtAddr[0]);
                CLOGE("ERR(%s):virtAddr[1]: 0x%08x", __func__, (unsigned int)virtAddr[1]);
                CLOGE("ERR(%s):virtAddr[2]: 0x%08x", __func__, (unsigned int)virtAddr[2]);
                goto done;
            }

            for (int i = 0; i < NUM_OF_PREVIEW_BUF; i++) {
                if (m_grallocVirtAddr[i] == NULL) {
                    findGrallocBuf = true;

                    m_previewBufHandle[i] = bufHandle;
                    m_previewStride[i] = stride;

                    m_grallocVirtAddr[i] = virtAddr[0];
                    m_matchedGrallocIndex[i] = i;
                    previewBuf.reserved.p = i;
                        const private_handle_t *priv_handle = private_handle_t::dynamicCast(*m_previewBufHandle[i]);
                        previewBuf.fd.extFd[0] = priv_handle->fd;
                        previewBuf.fd.extFd[1] = priv_handle->fd1;
                        previewBuf.virt.extP[0] = (char *)virtAddr[0];
                        previewBuf.virt.extP[1] = (char *)virtAddr[1];

                                                m_secCamera->setPreviewBuf(&previewBuf);
                    m_avaliblePreviewBufHandle[i] = true;
                    m_flagGrallocLocked[i] = true;

                    doPutPreviewBuf = true;

                    break;
                }

                if (m_grallocVirtAddr[i] == virtAddr[0]) {
                    findGrallocBuf = true;

                    const private_handle_t *priv_handle = private_handle_t::dynamicCast(*m_previewBufHandle[i]);

                    m_previewBufHandle[i] = bufHandle;
                    m_previewStride[i] = stride;
                    previewBuf.reserved.p = i;
                        previewBuf.fd.extFd[0] = priv_handle->fd;
                        previewBuf.fd.extFd[1] = priv_handle->fd1;
                        previewBuf.virt.extP[0] = (char *)virtAddr[0];
                        previewBuf.virt.extP[1] = (char *)virtAddr[1];

                                                m_secCamera->setPreviewBuf(&previewBuf);
                    m_matchedGrallocIndex[previewBuf.reserved.p] = i;
                    m_avaliblePreviewBufHandle[i] = true;
                    m_flagGrallocLocked[i] = true;
                    break;
                }
            }
            m_setPreviewBufStatus(previewBuf.reserved.p, ON_HAL);
            if (findGrallocBuf == false) {
                m_grallocHal->unlock(m_grallocHal, *bufHandle);

                if (m_previewWindow->cancel_buffer(m_previewWindow, bufHandle) != 0)
                    CLOGE("ERR(%s):Fail to cancel buffer", __func__);

                m_numOfDequeuedBuf--;

                goto done;
            }
            ExynosBuffer previewBufFromSvc, previewBufTemp, previewBufTemp2;
            previewBufFromSvc = previewBuf;
            if (m_previewBufRegistered[previewBuf.reserved.p] == false) {
                doPutPreviewBuf = true;
                shouldEraseBack = false;
            } else {
                if (m_sizeOfPreviewQ() >= 5) {
                    doPutPreviewBuf = true;
                    shouldEraseBack = false;
                    m_popPreviewQ(&previewBuf);
                    if (m_previewBufStatus[previewBuf.reserved.p] == ON_HAL) {
                        CLOGV("DEBUG(%s): previewBuf[%d] - status(%d) on hal", __func__, previewBuf.reserved.p, m_previewBufStatus[previewBuf.reserved.p]);
                    } else {
                        CLOGV("DEBUG(%s): previewBuf[%d] - status(%d) not on hal", __func__, previewBuf.reserved.p, m_previewBufStatus[previewBuf.reserved.p]);
                        previewBufTemp = previewBuf;
                        m_popPreviewQ(&previewBuf);
                        if (m_previewBufStatus[previewBuf.reserved.p] == ON_HAL) {
                            CLOGV("DEBUG(%s): next previewBuf[%d] - status(%d) on hal", __func__, previewBuf.reserved.p, m_previewBufStatus[previewBuf.reserved.p]);
                        } else {
                            CLOGV("DEBUG(%s): next previewBuf[%d] - status(%d) not on hal", __func__, previewBuf.reserved.p, m_previewBufStatus[previewBuf.reserved.p]);
                            previewBufTemp2 = previewBuf;
                            m_popPreviewQ(&previewBuf);
                            if (m_previewBufStatus[previewBuf.reserved.p] != ON_HAL) {
                                CLOGD("DEBUG(%s): next2 previewBuf[%d] - status(%d) not on hal", __func__, previewBuf.reserved.p, m_previewBufStatus[previewBuf.reserved.p]);
                                previewBuf = previewBufFromSvc;
                                shouldEraseBack = true;
                            }
                            m_pushFrontPreviewQ(&previewBufTemp2);
                        }
                        m_pushFrontPreviewQ(&previewBufTemp);
                    }
                } else {
                    doPutPreviewBuf = false;
                }
            }
        }
    }

done:
    if (doPutPreviewBuf == true) {
        if (shouldEraseBack)
            m_eraseBackPreviewQ();
        if (m_secCamera->putPreviewBuf(&previewBuf) == false) {
            CLOGE("ERR(%s):putPreviewBuf(%d) fail", __func__, previewBuf.reserved.p);
            ret = false;
        } else {
            m_setPreviewBufStatus(previewBuf.reserved.p, ON_DRIVER);
            m_previewBufRegistered[previewBuf.reserved.p] = true;
        }
    }

done2:
    // moved from wrapper func
    if (m_secCamera->getFocusMode() == ExynosCamera::FOCUS_MODE_CONTINUOUS_PICTURE)
    {
        int afstatus = 0;
        static int afResult = 1;

        int prev_afstatus = afResult;
        afstatus = m_secCamera->getCAFResult();
        afResult = afstatus;
        if (afstatus == 3 && (prev_afstatus == 0 || prev_afstatus == 1)) {
            afResult = 4;
        }
#if 1
        if ((m_msgEnabled & CAMERA_MSG_FOCUS) && (prev_afstatus != afstatus)) {
            CLOGD("DEBUG(%s):CAMERA_MSG_FOCUS(%d) mode(%d)", __func__, afResult, m_secCamera->getFocusMode());
            m_notifyCb(CAMERA_MSG_FOCUS, afResult, 0, m_callbackCookie);
        }
#else
        if (m_secCamera->flagStartFaceDetection() == true && m_faceDetected == true) {
            /* draw nothing */
            m_notifyCb(CAMERA_MSG_FOCUS, 4, 0, m_callbackCookie);
            /* skip callback, when FD detected */
            // ; /* nop */
        } else {
            if ((m_msgEnabled & CAMERA_MSG_FOCUS) && (prev_afstatus != afstatus))
                m_notifyCb(CAMERA_MSG_FOCUS, afResult, 0, m_callbackCookie);
        }
#endif
    }

    usleep(10);
    return true;

}

bool ExynosCameraHWImpl::m_doPreviewToCallbackFunc(ExynosBuffer previewBuf, ExynosBuffer *callbackBuf, bool useCSC)
{
    CLOGV("DEBUG(%s): converting preview to callback buffer", __func__);

    camera_memory_t *previewCallbackHeap = NULL;
    int previewCallbackHeapFd = -1;
    int previewW = 0, previewH = 0;
    int previewFormat = m_secCamera->getPreviewFormat();

    if (m_secCamera->getPreviewSize(&previewW, &previewH) == false) {
        CLOGE("ERR(%s):Fail to getPreviewSize", __func__);
        return false;
    }

    previewCallbackHeap = m_previewCallbackHeap[previewBuf.reserved.p];
    previewCallbackHeapFd = m_previewCallbackHeapFd[previewBuf.reserved.p];

    /*
     * If it is not 16-aligend, shrink down it as 16 align. ex) 1080 -> 1072
     * But, memory is set on Android format. so, not aligned area will be black.
     */
    int dst_width = 0, dst_height = 0, dst_crop_width = 0, dst_crop_height = 0;
    dst_width = m_orgPreviewRect.w;
    dst_height = m_orgPreviewRect.h;
    dst_crop_width = dst_width;
    dst_crop_height = dst_height;

    callbackBuf->fd.extFd[0] = previewCallbackHeapFd;
    if (m_orgPreviewRect.colorFormat == V4L2_PIX_FMT_NV21 ||
        m_orgPreviewRect.colorFormat == V4L2_PIX_FMT_NV21M) {

        callbackBuf->size.extS[0] = (m_orgPreviewRect.w * m_orgPreviewRect.h);
        callbackBuf->size.extS[1] = (m_orgPreviewRect.w * m_orgPreviewRect.h) / 2;

        callbackBuf->virt.extP[0] = (char *)previewCallbackHeap->data;
        callbackBuf->virt.extP[1] = callbackBuf->virt.extP[0] + callbackBuf->size.extS[0];

    } else if (m_orgPreviewRect.colorFormat == V4L2_PIX_FMT_YVU420 ||
                m_orgPreviewRect.colorFormat == V4L2_PIX_FMT_YVU420M) {

        callbackBuf->size.extS[0] = ALIGN_UP(m_orgPreviewRect.w, 16) * m_orgPreviewRect.h;
        callbackBuf->size.extS[1] = ALIGN_UP(m_orgPreviewRect.w / 2, 16) * m_orgPreviewRect.h / 2;
        callbackBuf->size.extS[2] = ALIGN_UP(m_orgPreviewRect.w / 2, 16) * m_orgPreviewRect.h / 2;

        callbackBuf->virt.extP[0] = (char *)previewCallbackHeap->data;
        callbackBuf->virt.extP[1] = callbackBuf->virt.extP[0] + callbackBuf->size.extS[0];
        callbackBuf->virt.extP[2] = callbackBuf->virt.extP[1] + callbackBuf->size.extS[1];
    }

    CLOGV("DEBUG(%s): preview size(%dx%d)", __func__, previewW, previewH);
    CLOGV("DEBUG(%s): dst_size(%dx%d), dst_crop_size(%dx%d)", __func__, dst_width, dst_height, dst_crop_width, dst_crop_height);

    if (useCSC) {
        /* resize from previewBuf(max size) to callbackHeap(user's set size) */
        if (m_exynosPreviewCSC) {
            csc_set_src_format(m_exynosPreviewCSC,
                    previewW, previewH,
                    0, 0, previewW, previewH,
                    V4L2_PIX_2_HAL_PIXEL_FORMAT(previewFormat),
                    0);

            if (m_orgPreviewRect.colorFormat == V4L2_PIX_FMT_NV21 ||
                m_orgPreviewRect.colorFormat == V4L2_PIX_FMT_NV21M) {

                csc_set_dst_format(m_exynosPreviewCSC,
                        dst_width, dst_height,
                        0, 0, dst_crop_width, dst_crop_height,
                        V4L2_PIX_2_HAL_PIXEL_FORMAT(m_orgPreviewRect.colorFormat),
                        1);
            } else if (m_orgPreviewRect.colorFormat == V4L2_PIX_FMT_YVU420 ||
                        m_orgPreviewRect.colorFormat == V4L2_PIX_FMT_YVU420M) {

                csc_set_dst_format(m_exynosPreviewCSC,
                        dst_width, dst_height,
                        0, 0, dst_crop_width, dst_crop_height,
                        V4L2_PIX_2_HAL_PIXEL_FORMAT(V4L2_PIX_FMT_YVU420M),
                        1);
            }

            csc_set_src_buffer(m_exynosPreviewCSC,
                    (void **)previewBuf.fd.extFd, CSC_MEMORY_TYPE);

            csc_set_dst_buffer(m_exynosPreviewCSC,
                    (void **)callbackBuf->virt.extP, CSC_MEMORY_USERPTR);

            if (csc_convert(m_exynosPreviewCSC) != 0)
                CLOGE("ERR(%s):csc_convert() from gralloc to callback fail", __func__);

            int remainedH = m_orgPreviewRect.h - dst_height;

            if (remainedH != 0) {
                char *srcAddr = NULL;
                char *dstAddr = NULL;
                int planeDiver = 1;

                for (int plane = 0; plane < 2; plane++) {
                    planeDiver = (plane + 1) * 2 / 2;

                    srcAddr = previewBuf.virt.extP[plane] + (ALIGN_UP(previewW, CAMERA_ISP_ALIGN) * dst_crop_height / planeDiver);
                    dstAddr = callbackBuf->virt.extP[plane] + (m_orgPreviewRect.w * dst_crop_height / planeDiver);

                    for (int i = 0; i < remainedH; i++) {
                        memcpy(dstAddr, srcAddr, (m_orgPreviewRect.w / planeDiver));

                        srcAddr += (ALIGN_UP(previewW, CAMERA_ISP_ALIGN) / planeDiver);
                        dstAddr += (m_orgPreviewRect.w                   / planeDiver);
                    }
                }
            }
        } else {
            CLOGE("ERR(%s):m_exynosPreviewCSC == NULL", __func__);
            return false;
        }
    } else { /* neon memcpy */
        char *srcAddr = NULL;
        char *dstAddr = NULL;

        /* TODO : have to consider all fmt(planes) and stride */
        for (int plane = 0; plane < 2; plane++) {
            srcAddr = previewBuf.virt.extP[plane];
            dstAddr = callbackBuf->virt.extP[plane];
            memcpy(dstAddr, srcAddr, callbackBuf->size.extS[plane]);
        }
    }

    if ((m_msgEnabled & CAMERA_MSG_PREVIEW_FRAME))
        m_dataCb(CAMERA_MSG_PREVIEW_FRAME, previewCallbackHeap, 0, NULL, m_callbackCookie);

    return true;
}

bool ExynosCameraHWImpl::m_doCallbackToPreviewFunc(ExynosBuffer previewBuf, ExynosBuffer *callbackBuf, bool useCSC)
{
    CLOGV("DEBUG(%s): converting callback to preview buffer", __func__);

    camera_memory_t *previewCallbackHeap = NULL;
    int previewCallbackHeapFd = -1;
    int previewW = 0, previewH = 0;
    int previewFormat = m_secCamera->getPreviewFormat();

    if (m_secCamera->getPreviewSize(&previewW, &previewH) == false) {
        CLOGE("ERR(%s):Fail to getPreviewSize", __func__);
        return false;
    }

    if (useCSC) {
        if (m_exynosPreviewCSC) {
            csc_set_src_format(m_exynosPreviewCSC,
                    ALIGN_DOWN(m_orgPreviewRect.w, CAMERA_MAGIC_ALIGN), ALIGN_DOWN(m_orgPreviewRect.h, CAMERA_MAGIC_ALIGN),
                    0, 0, ALIGN_DOWN(m_orgPreviewRect.w, CAMERA_MAGIC_ALIGN), ALIGN_DOWN(m_orgPreviewRect.h, CAMERA_MAGIC_ALIGN),
                    V4L2_PIX_2_HAL_PIXEL_FORMAT(m_orgPreviewRect.colorFormat),
                    1);

            csc_set_dst_format(m_exynosPreviewCSC,
                    previewW, previewH,
                    0, 0, previewW, previewH,
                    V4L2_PIX_2_HAL_PIXEL_FORMAT(previewFormat),
                    0);

            csc_set_src_buffer(m_exynosPreviewCSC,
                    (void **)callbackBuf->virt.extP, CSC_MEMORY_USERPTR);

            csc_set_dst_buffer(m_exynosPreviewCSC,
                    (void **)previewBuf.fd.extFd, CSC_MEMORY_TYPE);

            if (csc_convert(m_exynosPreviewCSC) != 0)
                CLOGE("ERR(%s):csc_convert() from callback to lcd fail", __func__);
        } else {
            CLOGE("ERR(%s):m_exynosPreviewCSC == NULL", __func__);
        }
    } else { /* neon memcpy */
        char *srcAddr = NULL;
        char *dstAddr = NULL;

        /* TODO : have to consider all fmt(planes) and stride */
        for (int plane = 0; plane < 2; plane++) {
            srcAddr = callbackBuf->virt.extP[plane];
            dstAddr = previewBuf.virt.extP[plane];
            memcpy(dstAddr, srcAddr, callbackBuf->size.extS[plane]);
        }
    }

    return true;
}

bool ExynosCameraHWImpl::m_videoThreadFuncWrapper(void)
{
    while (1) {
        while (m_videoRunning == false) {
            m_videoLock.lock();

#ifdef USE_3DNR_DMAOUT
            if (   m_secCamera->flagStartVideo() == true
                && m_secCamera->stopVideo() == false)
                CLOGE("ERR(%s):Fail on m_secCamera->stopVideo()", __func__);
#endif
            m_releaseVideoQ();

            CLOGD("DEBUG(%s:%d): SIGNAL(m_videoStoppedCondition) - send", __func__, __LINE__);
            m_videoStoppedCondition.signal();
            CLOGD("DEBUG(%s:%d): SIGNAL(m_videoCondition) - waiting", __func__, __LINE__);
            m_videoCondition.wait(m_videoLock);
            CLOGD("DEBUG(%s:%d): SIGNAL(m_videoCondition) - recevied", __func__, __LINE__);

            m_videoLock.unlock();
        }

        if (m_exitVideoThread == true) {
            CLOGD("DEBUG(%s:%d):m_exitVideoThread == true", __func__, __LINE__);
            m_videoLock.lock();

#ifdef USE_3DNR_DMAOUT
            if (   m_secCamera->flagStartVideo() == true
                && m_secCamera->stopVideo() == false)
                CLOGE("ERR(%s):Fail on m_secCamera->stopVideo()", __func__);
#endif
            m_releaseVideoQ();

            m_videoLock.unlock();
            return true;
        }

        /* waiting for preview thread */
        if (m_sizeOfVideoQ() == 0) {
            m_videoLock.lock();
            CLOGV("DEBUG(%s:%d): SIGNAL(m_videoCondition) - waiting", __func__, __LINE__);
            m_videoCondition.wait(m_videoLock);
            CLOGV("DEBUG(%s:%d): SIGNAL(m_videoCondition) - recevied", __func__, __LINE__);
            m_videoLock.unlock();
        }

        m_videoThreadFunc();
    }
    return true;
}

bool ExynosCameraHWImpl::m_videoThreadFunc(void)
{
    nsecs_t timestamp;
    struct addrs *addrs;
    ExynosBuffer videoBuf;
    int recordingFrameIndex = 0;

    static int cbcnt = 0;
    int waitCnt = 0;

    if ((m_msgEnabled & CAMERA_MSG_VIDEO_FRAME) &&
        (m_videoRunning == true)) {

        if (m_popVideoQ(&videoBuf) == false) {
            /* CLOGV("DEBUG(%s:%d):m_popVideoQ() false", __func__, __LINE__); */
            return true;
        }

        timestamp = m_videoBufTimestamp[videoBuf.reserved.p];
        m_videoBufTimestamp[videoBuf.reserved.p] = 1;

#ifdef USE_3DNR_DMAOUT
        if (m_secCamera->getVideoBuf(&videoBuf) == false) {
            CLOGE("ERR(%s):Fail on ExynosCamera->getVideoBuf()", __func__);
            return false;
        }
#else
        while (m_availableRecordingFrameCnt == 0) {
            usleep(200);
            waitCnt++;
            if (waitCnt > 1000) {
                CLOGE("ERR(%s):videoThread Timeout", __func__);
                return true;
            }
        }

        recordingFrameIndex = m_getRecordingFrame();
        if (recordingFrameIndex == -1) {
            CLOGD("DEBUG(%s:%d): m_getRecordingFrame() is %d", __func__, __LINE__, recordingFrameIndex);
            return true;
        }

        videoBuf.reserved.p = recordingFrameIndex;
#endif

        /* Notify the client of a new frame. */
        if ((m_msgEnabled & CAMERA_MSG_VIDEO_FRAME) &&
            (m_videoRunning == true)) {

            /* resize from videoBuf(max size) to m_videoHeap(user's set size) */
            if (m_exynosVideoCSC) {
                int videoW, videoH, videoFormat = 0;
                int cropX, cropY, cropW, cropH = 0;

                int previewW, previewH, previewFormat = 0;
                previewFormat = m_secCamera->getPreviewFormat();
                m_secCamera->getPreviewSize(&previewW, &previewH);

                videoFormat = m_secCamera->getVideoFormat();
                m_secCamera->getVideoSize(&videoW, &videoH);

                m_secCamera->getCropRectAlign(previewW, previewH,
                                         m_orgVideoRect.w, m_orgVideoRect.h,
                                         &cropX, &cropY,
                                         &cropW, &cropH,
                                         2, 2,
                                         0);

                CLOGV("DEBUG(%s):cropX = %d, cropY = %d, cropW = %d, cropH = %d",
                         __func__, cropX, cropY, cropW, cropH);

#ifdef USE_3DNR_DMAOUT
                csc_set_src_format(m_exynosVideoCSC,
                                   videoW, videoH,
                                   cropX, cropY, cropW, cropH,
                                   V4L2_PIX_2_HAL_PIXEL_FORMAT(videoFormat),
                                   0);
#else
                csc_set_src_format(m_exynosVideoCSC,
                                   previewW, previewH,
                                   cropX, cropY, cropW, cropH,
                                   V4L2_PIX_2_HAL_PIXEL_FORMAT(previewFormat),
                                   0);
#endif

                csc_set_dst_format(m_exynosVideoCSC,
                                   m_orgVideoRect.w, m_orgVideoRect.h,
                                   0, 0, m_orgVideoRect.w, m_orgVideoRect.h,
                                   V4L2_PIX_2_HAL_PIXEL_FORMAT(videoFormat),
                                   0);

                csc_set_src_buffer(m_exynosVideoCSC,
                                   (void **)videoBuf.fd.extFd, CSC_MEMORY_TYPE);

                ExynosBuffer dstBuf;
                m_getAlignedYUVSize(videoFormat, m_orgVideoRect.w, m_orgVideoRect.h, &dstBuf);

                dstBuf.virt.extP[0] = (char *)m_resizedVideoHeap[videoBuf.reserved.p][0]->data;
                dstBuf.virt.extP[1] = (char *)m_resizedVideoHeap[videoBuf.reserved.p][1]->data;

                dstBuf.fd.extFd[0] = m_resizedVideoHeapFd[videoBuf.reserved.p][0];
                dstBuf.fd.extFd[1] = m_resizedVideoHeapFd[videoBuf.reserved.p][1];

                csc_set_dst_buffer(m_exynosVideoCSC,
                                  (void **)dstBuf.fd.extFd, CSC_MEMORY_TYPE);

                if (csc_convert(m_exynosVideoCSC) != 0)
                    CLOGE("ERR(%s):csc_convert() fail", __func__);

                CLOGV("DEBUG(%s): Camera Meta addrs %d",__func__, recordingFrameIndex);
                addrs = (struct addrs *)m_recordHeap->data;
                addrs[recordingFrameIndex].type      = kMetadataBufferTypeCameraSource;
                addrs[recordingFrameIndex].fd_y      = (unsigned int)dstBuf.fd.extFd[0];
                addrs[recordingFrameIndex].fd_cbcr   = (unsigned int)dstBuf.fd.extFd[1];
                addrs[recordingFrameIndex].buf_index = recordingFrameIndex;
            } else {
                CLOGE("ERR(%s):m_exynosVideoCSC == NULL", __func__);
            }

#ifdef CHECK_TIME_RECORDING
            m_checkRecordingTime();
#endif

            if ((0L < timestamp) && (m_lastRecordingTimestamp < timestamp) && (m_recordingStartTimestamp < timestamp)) {
                if ((m_msgEnabled & CAMERA_MSG_VIDEO_FRAME) && (m_videoRunning == true)) {

                    CLOGV("DEBUG(%s):timestamp(%d msec), curTime(%d msec)",
                        __func__,
                        (int)(timestamp) / (1000 * 1000),
                        (int)(systemTime(SYSTEM_TIME_MONOTONIC)) / (1000 * 1000));

                    m_dataCbTimestamp(timestamp, CAMERA_MSG_VIDEO_FRAME,
                                      m_recordHeap, recordingFrameIndex, m_callbackCookie);

                    m_lastRecordingTimestamp = timestamp;
                }
            } else {
                CLOGW("WRN(%s): timestamp(%lld) invaild - last timestamp(%lld) systemtime(%lld) recordStart(%lld)",
                    __func__, timestamp, m_lastRecordingTimestamp, systemTime(SYSTEM_TIME_MONOTONIC), m_recordingStartTimestamp);
                m_freeRecordingFrame(recordingFrameIndex);
            }
        }

#ifdef USE_3DNR_DMAOUT
        m_secCamera->putVideoBuf(&videoBuf);

        m_pushVideoQ(&videoBuf);
#endif
        // until here
    } else
        usleep(1000); // sleep 1msec for stopRecording

    return true;
}

bool ExynosCameraHWImpl::m_autoFocusThreadFunc(void)
{
    CLOGV("DEBUG(%s):starting", __func__);

    bool afResult = false;

    /* block until we're told to start.  we don't want to use
     * a restartable thread and requestExitAndWait() in cancelAutoFocus()
     * because it would cause deadlock between our callbacks and the
     * caller of cancelAutoFocus() which both want to grab the same lock
     * in CameraServices layer.
     */
    /* check early exit request */
    if (m_exitAutoFocusThread == true) {
        CLOGD("DEBUG(%s):exiting on request", __func__);
        goto done;
    }

    m_autoFocusRunning = true;

    if (m_autoFocusRunning == true) {
        afResult = m_secCamera->autoFocus();
        if (afResult == true)
            CLOGV("DEBUG(%s):autoFocus Success!!", __func__);
        else
            CLOGV("DEBUG(%s):autoFocus Fail !!", __func__);
    } else {
        CLOGV("DEBUG(%s):autoFocus canceled !!", __func__);
    }

    /*
     * CAMERA_MSG_FOCUS only takes a bool.  true for
     * finished and false for failure.
     * If cancelAutofocus() called, no callback.
     */
    if ((m_autoFocusRunning == true) &&
        (m_msgEnabled & CAMERA_MSG_FOCUS)) {

        if (m_notifyCb != NULL) {
            int afFinalResult = (int)afResult;

            /* if inactive detected, tell it */
            if (m_secCamera->getFocusMode() == ExynosCamera::FOCUS_MODE_CONTINUOUS_PICTURE) {
                if (m_secCamera->getCAFResult() == 2) {
                    afFinalResult = 2;
                }
            }

            CLOGD("DEBUG(%s):CAMERA_MSG_FOCUS(%d) mode(%d)", __func__, afFinalResult, m_secCamera->getFocusMode());
            m_notifyCb(CAMERA_MSG_FOCUS, afFinalResult, 0, m_callbackCookie);
        }  else {
            CLOGD("DEBUG(%s):m_notifyCb is NULL mode(%d)", __func__, m_secCamera->getFocusMode());
        }
    } else {
        CLOGV("DEBUG(%s):autoFocus canceled, no callback !!", __func__);
    }

    m_autoFocusRunning = false;

    CLOGV("DEBUG(%s):exiting with no error", __func__);

done:
    m_autoFocusLock.unlock();

    CLOGV("DEBUG(%s):end", __func__);

    return true;
}

bool ExynosCameraHWImpl::m_startPictureInternal(void)
{
    CLOGD("DEBUG(%s):(%d)", __func__, __LINE__);

    if (m_secCamera->getCameraMode() == ExynosCamera::CAMERA_MODE_FRONT) {
        if (m_pictureRunning == true) {
            CLOGE("ERR(%s):Aready m_pictureRunning is running", __func__);
            return false;
        }

        int pictureW, pictureH, pictureFormat, pictureFramesize;
        ExynosBuffer nullBuf;

        pictureFormat = m_secCamera->getPictureFormat();
        m_secCamera->getPictureSize(&pictureW, &pictureH);
        pictureFramesize = FRAME_SIZE(V4L2_PIX_2_HAL_PIXEL_FORMAT(pictureFormat), ALIGN_UP(pictureW, CAMERA_MAGIC_ALIGN), ALIGN_UP(pictureH, CAMERA_MAGIC_ALIGN));

        if (m_rawHeap) {
            m_rawHeap->release(m_rawHeap);
            m_rawHeap = 0;
            m_rawHeapFd = -1;
            m_rawHeapSize = 0;
        }

        int rawHeapSize = pictureFramesize;

        m_rawHeap = m_getMemoryCb(-1, rawHeapSize, 1, &m_rawHeapFd);
        if (!m_rawHeap || m_rawHeapFd <= 0) {
            CLOGE("ERR(%s):m_getMemoryCb(m_rawHeap, size(%d) fail", __func__, rawHeapSize);
            return false;
        }
        m_rawHeapSize = rawHeapSize;

        for (int i = 0; i < NUM_OF_PICTURE_BUF; i++) {
            if (m_pictureHeap[i]) {
                m_pictureHeap[i]->release(m_pictureHeap[i]);
                m_pictureHeap[i] = 0;
                m_pictureHeapFd[i] = -1;
            }

            m_pictureHeap[i] = m_getMemoryCb(-1, pictureFramesize, 1, &(m_pictureHeapFd[i]));
            if (!m_pictureHeap[i] || m_pictureHeapFd[i] <= 0) {
                CLOGE("ERR(%s):m_getMemoryCb(m_pictureHeap[%d], size(%d) fail", __func__, i, pictureFramesize);
                return false;
            }

            m_pictureBuf[0] = nullBuf;
            m_pictureBuf[0].virt.extP[0] = (char *)m_pictureHeap[i]->data;
            m_pictureBuf[0].fd.extFd[0] = m_pictureHeapFd[i];

            m_getAlignedYUVSize(pictureFormat, ALIGN_UP(pictureW, CAMERA_MAGIC_ALIGN), ALIGN_UP(pictureH, CAMERA_MAGIC_ALIGN), &m_pictureBuf[0]);

            m_pictureBuf[0].reserved.p = i;

            m_secCamera->setPictureBuf(&m_pictureBuf[0]);
        }

        if (m_secCamera->startPicture() == false) {
            CLOGE("ERR(%s):Fail on m_secCamera->startPicture()", __func__);
            return false;
        }

        for (int i = 0; i < NUM_OF_FLASH_BUF; i++)
            m_pictureBuf[i] = nullBuf;

        m_pictureRunning = true;
    }
    return true;
}

bool ExynosCameraHWImpl::m_stopPictureInternal(void)
{
    CLOGD("DEBUG(%s):(%d)", __func__, __LINE__);

    if (m_pictureRunning == false) {
        CLOGE("ERR(%s):Aready m_pictureRunning is stop", __func__);
        return false;
    }

    if (m_secCamera->flagStartPicture() == true &&
        m_secCamera->stopPicture() == false)
        CLOGE("ERR(%s):Fail on m_secCamera->stopPicture()", __func__);

    for (int i = 0; i < NUM_OF_PICTURE_BUF; i++) {
        if (m_pictureHeap[i]) {
            m_pictureHeap[i]->release(m_pictureHeap[i]);
            m_pictureHeap[i] = 0;
            m_pictureHeapFd[i] = -1;
        }
    }

    if (m_rawHeap) {
        m_rawHeap->release(m_rawHeap);
        m_rawHeap = 0;
        m_rawHeapFd = -1;
        m_rawHeapSize = 0;
    }

    m_pictureRunning = false;

    return true;
}

bool ExynosCameraHWImpl::m_pictureThreadFunc(void)
{
    CLOGD("DEBUG(%s):(%d)", __func__, __LINE__);

    bool ret = false;

    int oldFocusMode = m_secCamera->getFocusMode();
    int newFocusMode = oldFocusMode;
#ifdef DYNAMIC_BAYER_BACK_REC
    ExynosBuffer dynamicSensorBuf;
#endif

    int pictureW, pictureH, pictureFramesize = 0;
    int ispW, ispH;
    int pictureFormat;
    int cropX = 0, cropY = 0, cropW = 0, cropH = 0;
    bool doPutPictureBuf = false;
    bool flashTurnOnHere = false;
    int numOfPictureBuf = 1;

    ExynosBuffer pictureBuf;
    ExynosBuffer jpegBuf;

    camera_memory_t *JpegHeapOut = NULL;
    int JpegHeapOutFd = -1;
    struct camera2_shot_ext *shot_ext;

    if (m_secCamera->getCameraMode() == ExynosCamera::CAMERA_MODE_FRONT)
        usleep(50000);

    if (m_secCamera->getCameraMode() == ExynosCamera::CAMERA_MODE_FRONT) {
        if (m_pictureRunning == false &&
            m_startPictureInternal() == false) {
            CLOGE("ERR(%s):m_startPictureInternal() fail", __func__);
            goto out;
        }
    }

    if (m_secCamera->getCameraMode() == ExynosCamera::CAMERA_MODE_BACK) {
        if (m_pictureRunning == false &&
            m_startPictureInternalReprocessing() == false) {
            CLOGE("ERR(%s):m_startPictureInternalReprocessing() fail", __func__);
            goto out;
        }
    }

#ifdef DYNAMIC_BAYER_BACK_REC
        if ((m_secCamera->getCameraMode() == ExynosCamera::CAMERA_MODE_BACK &&
             m_secCamera->getRecordingHint() == true)
#ifdef SCALABLE_SENSOR
         || (m_secCamera->getScalableSensorStart() == true)
#endif
        ) {
#ifdef SCALABLE_SENSOR_CHKTIME
            struct timeval start, end;
            int sec, usec;
            gettimeofday(&start, NULL);
#endif
            ExynosBuffer sensorBuf;
            struct camera2_shot_ext *shot_ext;

            if (isDqSensor == false) {
                isDqSensor = true;

                for (int jj = 0; jj < NO_BAYER_SENSOR_Q_NUM; jj++) {
                    if (m_secCamera->getSensorBuf(&sensorBuf) == false) {
                        CLOGE("ERR(%s) (%d) :getSensorBuf() fail %d", __func__, __LINE__, jj);

                        return false;
                    }
                }
            }
#ifdef SCALABLE_SENSOR
            if (m_secCamera->putFixedSensorBuf(&dynamicSensorBuf) == false) {
                CLOGE("ERR(%s) (%d) :putSensorBuf() fail", __func__, __LINE__);
                return false;
            }

            if (m_secCamera->getFixedSensorBuf(&dynamicSensorBuf) == false) {
                CLOGE("ERR(%s) (%d) :getSensorBuf() fail", __func__, __LINE__);
                return false;
            }
#else
            if (m_secCamera->putSensorBuf(&dynamicSensorBuf) == false) {
                CLOGE("ERR(%s) (%d) :putSensorBuf() fail", __func__, __LINE__);

                return false;
            }

            if (m_secCamera->getSensorBuf(&dynamicSensorBuf) == false) {
                CLOGE("ERR(%s) (%d) :getSensorBuf() fail", __func__, __LINE__);

                return false;
            }
#endif

#ifdef SCALABLE_SENSOR_CHKTIME
            gettimeofday(&end, NULL);
            CLOGD("DEBUG(%s):CHKTIME Bayer buffer getting(%d)", __func__, (end.tv_sec - start.tv_sec)*1000000 + (end.tv_usec - start.tv_usec));
#endif
        }
#endif

#ifdef FRONT_NO_ZSL
        if (m_secCamera->getCameraMode() == ExynosCamera::CAMERA_MODE_FRONT) {
            m_secCamera->setFrontCaptureCmd(1);

            if (m_secCamera->getPictureBuf(&m_pictureBuf[0]) == false) {
                CLOGE("ERR(%s):getPictureBuf() fail", __func__);
                return false;
            }

            doPutPictureBuf = true;
        }
#endif

    if ((m_flashMode == ExynosCamera::FLASH_MODE_AUTO ||
         m_flashMode == ExynosCamera::FLASH_MODE_ON ||
         m_flashMode == ExynosCamera::FLASH_MODE_RED_EYE) &&
        (((ExynosCameraActivityFlash *)m_secCamera->getFlashMgr())->getNeedCaptureFlash() == true)) {

        /* AE unlock for AE-haunting when zoom capture */
        if (m_forceAELock == true) {
            if (m_secCamera->setAutoExposureLock(false) == false)
                CLOGE("ERR(%s):setAutoExposureLock(false) fail", __func__);

            m_forceAELock = false;
        }

        if (m_secCamera->setFlashMode(m_flashMode) == false) {
            CLOGE("ERR(%s):m_secCamera->setFlashMode(%d) fail", __func__, m_flashMode);
        } else {
            enum ExynosCameraActivityFlash::FLASH_TRIGGER tiggerPath;
            flashTurnOnHere = true;

            ((ExynosCameraActivityFlash *)m_secCamera->getFlashMgr())->getFlashTrigerPath(&tiggerPath);

            if (tiggerPath == ExynosCameraActivityFlash::FLASH_TRIGGER_TOUCH_DISPLAY) {
                if (((ExynosCameraActivityFlash *)m_secCamera->getFlashMgr())->waitMainReady() == false)
                    CLOGW("ERR(%s):waitMainReady() timeout", __func__);
            }

            ((ExynosCameraActivityFlash *)m_secCamera->getFlashMgr())
                ->setFlashStep(ExynosCameraActivityFlash::FLASH_STEP_MAIN_START);
        }
    }

    if (((ExynosCameraActivityFlash *)m_secCamera->getFlashMgr())->getNeedCaptureFlash() == true) {
        int totalWaitingTime = 0;
        int waitCount = 0;
        unsigned int waitFcount = 0;
        unsigned int shotFcount = 0;

        ((ExynosCameraActivityFlash *)m_secCamera->getFlashMgr())->resetShotFcount();

        do {
            waitCount = ((ExynosCameraActivityFlash *)m_secCamera->getFlashMgr())->getWaitingCount();
            if (0 < waitCount) {
                usleep(FLASH_WAITING_SLEEP_TIME);
                totalWaitingTime += FLASH_WAITING_SLEEP_TIME;
            }
        } while (0 < waitCount && totalWaitingTime <= FLASH_MAX_WAITING_TIME);

        if (0 < waitCount || FLASH_MAX_WAITING_TIME < totalWaitingTime)
            CLOGE("ERR(%s):waiting too much (%d msec)", __func__, totalWaitingTime);

        shotFcount = ((ExynosCameraActivityFlash *)m_secCamera->getFlashMgr())->getShotFcount();
        if (m_sharedBayerFcount != shotFcount)
            CLOGE("ERR(%s):shot frame count not mismatch (getShotFcount %d, m_sharedBayerFcount %d)",
                __func__, shotFcount, m_sharedBayerFcount);
        else
            CLOGD("DEBUG(%s):(%d) getShotFcount %d, m_sharedBayerFcount %d", __func__, __LINE__, shotFcount, m_sharedBayerFcount);

        /* To check Flash Timing */
        waitFcount = ((ExynosCameraActivityFlash *)m_secCamera->getFlashMgr())->getShotFcount() + 1;
        totalWaitingTime= 0;
        while (waitFcount > m_sharedBayerFcount && totalWaitingTime <= FLASH_CAPTURE_WAITING_TIME) {
            usleep(FLASH_WAITING_SLEEP_TIME);
            totalWaitingTime += FLASH_WAITING_SLEEP_TIME;

            CLOGD("DEBUG(%s):(%d) waitFcount %d, m_sharedBayerFcount %d",
                __func__, __LINE__, waitFcount, m_sharedBayerFcount);
        }
    }

    m_secCamera->getIspSize(&ispW, &ispH);
    m_secCamera->getPictureSize(&pictureW, &pictureH);
    pictureFormat = m_secCamera->getPictureFormat();
    pictureFramesize = FRAME_SIZE(V4L2_PIX_2_HAL_PIXEL_FORMAT(pictureFormat), m_orgPictureRect.w, m_orgPictureRect.h);

    if (m_secCamera->getCameraMode() == ExynosCamera::CAMERA_MODE_FRONT) {
        m_secCamera->getCropRectAlign(pictureW,  pictureH,
                                      ispW,  ispH,
                                      &cropX, &cropY,
                                      &cropW, &cropH,
                                      CAMERA_MAGIC_ALIGN, 2,
                                      m_secCamera->getZoom());
    } else if (m_secCamera->getCameraMode() == ExynosCamera::CAMERA_MODE_BACK) {
        for (int i = 0; i < numOfPictureBuf; i++) {
            ExynosBuffer sensorBufReprocessing;
            ExynosBuffer ispBuf;
            unsigned int waitBayerFcount = 0;
            unsigned int normalCaptureFcount = 0;

            int retry = 0;
            do {
#ifdef DYNAMIC_BAYER_BACK_REC
                if (((m_secCamera->getRecordingHint() == true) &&
                    (m_secCamera->getCameraMode() == ExynosCamera::CAMERA_MODE_BACK))
#ifdef SCALABLE_SENSOR
                  || (m_secCamera->getScalableSensorStart() == true)
#endif
                )
                    sensorBufReprocessing = dynamicSensorBuf;
                else
#endif
                    sensorBufReprocessing = m_sharedISPBuffer;
            } while (!m_checkPictureBufferVaild(&sensorBufReprocessing, retry++));

            normalCaptureFcount = ((camera2_shot_ext *)sensorBufReprocessing.virt.extP[1])->shot.dm.request.frameCount;

            ExynosBuffer *tempBuf = NULL;

            CLOGD("DEBUG(%s):(%d) sensorBufReprocessing %d", __func__, __LINE__, sensorBufReprocessing.reserved.p);

            if (sensorBufReprocessing.reserved.p == -1) {
                CLOGE("ERR(%s):Fail to get valid sensor buf(index %d)", __func__, sensorBufReprocessing.reserved.p);
                goto out;
            }

            if (((ExynosCameraActivityFlash *)m_secCamera->getFlashMgr())->getNeedCaptureFlash() == true) {
                waitBayerFcount = ((ExynosCameraActivityFlash *)m_secCamera->getFlashMgr())->getShotFcount() + 1;

                tempBuf = m_secCamera->searchSensorBuffer(waitBayerFcount);
                if (tempBuf == NULL) {
                   CLOGE("[%s] (%d) sensorBufReprocessing is null (%d)", __func__, __LINE__, waitBayerFcount);
                   m_secCamera->printBayerLockStatus();

                   sensorBufReprocessing = m_sharedBayerBuffer;
                } else
                   sensorBufReprocessing = *tempBuf;

                m_secCamera->setBayerLock(waitBayerFcount, true);
                CLOGD("DEBUG(%s):(%d) m_sharedBayerFcount %d", __func__, __LINE__, m_sharedBayerFcount);
            } else {
                tempBuf = m_secCamera->searchSensorBufferOnHal(normalCaptureFcount);
                if (tempBuf == NULL) {
                    CLOGE("[%s] (%d) normalCaptureFcount is null (%d)", __func__, __LINE__, normalCaptureFcount);
                    m_secCamera->printBayerLockStatus();

                    sensorBufReprocessing = m_sharedBayerBuffer;
                } else {
                    sensorBufReprocessing = *tempBuf;
                    normalCaptureFcount = ((camera2_shot_ext *)sensorBufReprocessing.virt.extP[1])->shot.dm.request.frameCount;
                }

                m_secCamera->setBayerLock(normalCaptureFcount, true);
                CLOGD("DEBUG(%s):(%d) normalCaptureFcount %d", __func__, __LINE__, normalCaptureFcount);
            }
            m_secCamera->printBayerLockStatus();

#ifdef SCALABLE_SENSOR
            shot_ext = (struct camera2_shot_ext *)(sensorBufReprocessing.virt.extP[1]);
            int bayerFcount = shot_ext->shot.dm.request.frameCount;
            CLOGE("bayerFcount = (%d)", bayerFcount);
#endif
            ExynosCameraHWImpl::g_is3a0Mutex.lock();

            if (m_secCamera->getIs3a0BufReprocessing(ExynosCamera::CAMERA_MODE_REPROCESSING, &sensorBufReprocessing, &ispBuf) == false) {
                CLOGE("ERR(%s):getIs3a0BufReprocessing() fail(id:%d)", __func__, getCameraId());

                ExynosCameraHWImpl::g_is3a0Mutex.unlock();
                goto out;
            }

            ExynosCameraHWImpl::g_is3a0Mutex.unlock();

            if (ispBuf.reserved.p < 0) {
                CLOGW("WRN(%s): ispBuf.reserved.p = %d", __func__, ispBuf.reserved.p);
                goto out;
            }

            m_secCamera->pictureOnReprocessing();

            if (m_secCamera->putISPBufReprocessing(&ispBuf) == false) {
                CLOGE("ERR(%s):putISPBufReprocessing() fail", __func__);
                goto out;
            }

            if (m_secCamera->getISPBufReprocessing(&ispBuf) == false) {
                CLOGE("ERR(%s):getISPBufReprocessing() fail", __func__);
                goto out;
            }

            if (m_secCamera->getPictureBufReprocessing(&m_pictureBuf[i]) == false) {
                CLOGE("ERR(%s):getPictureBufReprocessing() fail", __func__);
                goto out;
            }

            if (m_secCamera->putPictureBufReprocessing(&m_pictureBuf[i]) == false) {
                CLOGE("ERR(%s):putPictureBufReprocessing(%d) fail", __func__, m_pictureBuf[i].reserved.p);
                goto out;
            }
                    m_secCamera->setBayerLock(normalCaptureFcount, false);

                shot_ext = (struct camera2_shot_ext *)(sensorBufReprocessing.virt.extP[1]);
                cropW = shot_ext->shot.ctl.scaler.cropRegion[2];
                cropH = shot_ext->shot.ctl.scaler.cropRegion[3];
            }
            m_secCamera->printBayerLockStatus();

        for (int k = 0; k < NUM_BAYER_BUFFERS; k++)
            m_secCamera->setBayerLockIndex(k, false);
    }

    if (!m_jpegHeap) {
        m_jpegHeap = m_getMemoryCb(-1, pictureFramesize, 1, &m_jpegHeapFd);
    } else {
        if (m_jpegHeap->size != (size_t)pictureFramesize) {
            m_jpegHeap->release(m_jpegHeap);
            m_jpegHeap = m_getMemoryCb(-1, pictureFramesize, 1, &m_jpegHeapFd);
        }
    }
    if (!m_jpegHeap || m_jpegHeapFd <= 0) {
        CLOGE("ERR(%s):m_getMemoryCb(m_jpegHeap, size(%d) fail", __func__, pictureFramesize);
        goto out;
    }

    for (int i = 0; i < numOfPictureBuf; i++) {
    // resize from pictureBuf(max size) to rawHeap(user's set size)
    if (m_exynosPictureCSC) {
        CLOGD("DEBUG(%s):(%d) CSC start numOfPictureBuf %d", __func__, __LINE__, numOfPictureBuf);

            CLOGV("(%s): src(%d, %d), jpg(%d, %d)", __func__, cropW, cropH, m_orgPictureRect.w, m_orgPictureRect.h);
            if ((cropW == m_orgPictureRect.w) &&
                (cropH == m_orgPictureRect.h) &&
                (m_secCamera->getZoom() == 0) &&
                !m_flip_horizontal) {
                CLOGD("(%s): Bypassing CSC", __func__);
                m_isCSCBypassed = true;
                m_getAlignedYUVSize(JPEG_INPUT_COLOR_FMT, m_orgPictureRect.w, m_orgPictureRect.h, &pictureBuf);
                pictureBuf.virt.extP[0] = m_pictureBuf[i].virt.extP[0];
                pictureBuf.fd.extFd[0] = m_pictureBuf[i].fd.extFd[0];
            } else {
                CLOGD("(%s): Capture crop size(%d, %d) Zoom(%d)",
                    __func__, cropW, cropH, m_secCamera->getZoom());
                m_isCSCBypassed = false;

                int csc_cropX = 0, csc_cropY = 0, csc_cropW = 0, csc_cropH = 0;
                m_secCamera->getCropRectAlign(cropW, cropH,
                                          m_orgPictureRect.w, m_orgPictureRect.h,
                                          &csc_cropX, &csc_cropY,
                                          &csc_cropW, &csc_cropH,
                                          2, 2,
                                          0);

                csc_set_src_format(m_exynosPictureCSC,
                                   ALIGN_UP(cropW, CAMERA_MAGIC_ALIGN), ALIGN_UP(cropH, CAMERA_MAGIC_ALIGN),
                                   csc_cropX, csc_cropY, csc_cropW, csc_cropH,
                                   V4L2_PIX_2_HAL_PIXEL_FORMAT(pictureFormat),
                                   0);

                csc_set_dst_format(m_exynosPictureCSC,
                               m_orgPictureRect.w, m_orgPictureRect.h,
                               0, 0, m_orgPictureRect.w, m_orgPictureRect.h,
                               V4L2_PIX_2_HAL_PIXEL_FORMAT(JPEG_INPUT_COLOR_FMT),
                               0);

                csc_set_src_buffer(m_exynosPictureCSC,
                                   (void **)m_pictureBuf[i].fd.extFd, CSC_MEMORY_TYPE);

                int rawHeapSize = FRAME_SIZE(V4L2_PIX_2_HAL_PIXEL_FORMAT(pictureFormat),
                                             ALIGN_UP(m_orgPictureRect.w, CAMERA_MAGIC_ALIGN),
                                             ALIGN_UP(m_orgPictureRect.h, CAMERA_MAGIC_ALIGN));
                if (m_rawHeapSize != rawHeapSize) {
                    if (m_rawHeap) {
                        m_rawHeap->release(m_rawHeap);
                        m_rawHeap = 0;
                        m_rawHeapFd = -1;
                        m_rawHeapSize = 0;
                    }
                }

                if (m_rawHeap == 0) {
                    m_rawHeap = m_getMemoryCb(-1, rawHeapSize, 1, &m_rawHeapFd);
                    if (!m_rawHeap || m_rawHeapFd <= 0) {
                        CLOGE("ERR(%s):m_getMemoryCb(m_rawHeap, size(%d) fail", __func__, rawHeapSize);
                        goto out;
                    }
                    m_rawHeapSize = rawHeapSize;
                }

                pictureBuf.virt.extP[0] = (char *)m_rawHeap->data;
                pictureBuf.fd.extFd[0] = m_rawHeapFd;

                m_getAlignedYUVSize(JPEG_INPUT_COLOR_FMT, m_orgPictureRect.w, m_orgPictureRect.h, &pictureBuf);

                csc_set_dst_buffer(m_exynosPictureCSC,
                               (void **)pictureBuf.fd.extFd, CSC_MEMORY_TYPE);

                //m_fileDump("/data/gsc_dump.yuv", m_pictureBuf.virt.extP[0], ALIGN_UP(m_orgPictureRect.w, CAMERA_MAGIC_ALIGN) * ALIGN_UP(m_orgPictureRect.h, CAMERA_MAGIC_ALIGN) *2);

                if (csc_convert(m_exynosPictureCSC) != 0)
                    CLOGE("ERR(%s):csc_convert() fail", __func__);

            }
    } else {
        CLOGE("ERR(%s):m_exynosPictureCSC == NULL", __func__);
    }

    //m_secCamera->fileDump("/data/gsc_dump1.yuv", pictureBuf.virt.extP[0], ALIGN_UP(m_orgPictureRect.w, CAMERA_MAGIC_ALIGN) * ALIGN_UP(m_orgPictureRect.h, CAMERA_MAGIC_ALIGN) *2);

    // This is for NV16 color format in exynos5410
    if (JPEG_INPUT_COLOR_FMT == V4L2_PIX_FMT_NV16) {
        pictureBuf.size.extS[0] = m_orgPictureRect.w * m_orgPictureRect.h * 2;
        pictureBuf.size.extS[1] = 0;
        pictureBuf.size.extS[2] = 0;
    } else {
        m_getAlignedYUVSize(JPEG_INPUT_COLOR_FMT, m_orgPictureRect.w, m_orgPictureRect.h, &pictureBuf);
    }

    for (int j = 1; j < 3; j++) {
        if (pictureBuf.size.extS[j] != 0)
            pictureBuf.virt.extP[j] = pictureBuf.virt.extP[j-1] + pictureBuf.size.extS[j-1];
        else
            pictureBuf.virt.extP[j] = NULL;

        CLOGV("(%s): pictureBuf.size.extS[%d] = %d", __func__, j, pictureBuf.size.extS[j]);
    }

    if ((m_msgEnabled & CAMERA_MSG_COMPRESSED_IMAGE)) {
        CLOGD("DEBUG(%s): time test  yuv2Jpeg - start %d\n", __func__, __LINE__);

        jpegBuf.virt.p = (char *)m_jpegHeap->data;
        jpegBuf.size.s = m_jpegHeap->size;
        jpegBuf.fd.extFd[0] = m_jpegHeapFd;

        ExynosRect jpegRect;
        jpegRect.w = m_orgPictureRect.w;
        jpegRect.h = m_orgPictureRect.h;
        jpegRect.colorFormat = JPEG_INPUT_COLOR_FMT;

        if (m_secCamera->yuv2Jpeg(&pictureBuf, &jpegBuf, &jpegRect) == false) {
            CLOGE("ERR(%s):yuv2Jpeg() fail", __func__);

            {
                m_stateLock.lock();
                m_captureInProgress = false;
                m_waitForCapture = false;
                {
                    m_pictureLock.lock();
                    m_pictureCondition.signal();
                    m_pictureLock.unlock();
                }
                m_stateLock.unlock();
            }

            goto out;
        }

        CLOGD("DEBUG(%s): time test  yuv2Jpeg - end %d\n", __func__, __LINE__);

#ifdef CHECK_TIME_SHOT2SHOT
        m_shot2ShotTimer.stop();
        long long shot2ShotDurationTime = m_shot2ShotTimer.durationUsecs();
        CLOGD("DEBUG(%s):CHECK_TIME_SHOT2SHOT : %d msec", __func__, (int)shot2ShotDurationTime / 1000);
#endif //CHECK_TIME_SHOT2SHOT
    }

    if (doPutPictureBuf == true) {
        if (m_secCamera->putPictureBuf(&m_pictureBuf[0]) == false)
            CLOGE("ERR(%s):putPictureBuf(%d) fail", __func__, m_pictureBuf[0].reserved.p);
        else
            doPutPictureBuf = false;
    }

    m_stateLock.lock();
    m_waitForCapture = false;
    m_stateLock.unlock();

    m_pictureLock.lock();
    m_pictureCondition.signal();
    m_pictureLock.unlock();

    if (m_msgEnabled & CAMERA_MSG_SHUTTER)
        m_notifyCb(CAMERA_MSG_SHUTTER, 0, 0, m_callbackCookie);

    if (m_msgEnabled & CAMERA_MSG_RAW_IMAGE) {
        if (m_isCSCBypassed) {
            m_dataCb(CAMERA_MSG_RAW_IMAGE,  m_pictureHeap[m_pictureBuf[i].reserved.p], 0, NULL, m_callbackCookie);
        } else {
            m_dataCb(CAMERA_MSG_RAW_IMAGE, m_rawHeap, 0, NULL, m_callbackCookie);
        }
    }

    /* TODO: Currently framework dose not support CAMERA_MSG_RAW_IMAGE_NOTIFY callback */
    if (m_msgEnabled & CAMERA_MSG_RAW_IMAGE_NOTIFY)
        m_notifyCb(CAMERA_MSG_RAW_IMAGE_NOTIFY, 0, 0, m_callbackCookie);

    if (m_msgEnabled & CAMERA_MSG_POSTVIEW_FRAME) {
        if (m_isCSCBypassed) {
            m_dataCb(CAMERA_MSG_POSTVIEW_FRAME,  m_pictureHeap[m_pictureBuf[i].reserved.p], 0, NULL, m_callbackCookie);
        } else {
            m_dataCb(CAMERA_MSG_POSTVIEW_FRAME, m_rawHeap, 0, NULL, m_callbackCookie);
        }
    }

    if (m_msgEnabled & CAMERA_MSG_COMPRESSED_IMAGE) {
        JpegHeapOut = m_getMemoryCb(-1, jpegBuf.size.s, 1, &JpegHeapOutFd);
        if (!JpegHeapOut || JpegHeapOutFd <= 0) {
            CLOGE("ERR(%s):m_getMemoryCb(JpegHeapOut, size(%d) fail", __func__, jpegBuf.size.s);
            m_captureMode = false;
            return false;
        }

        // TODO : we shall pass JpegHeap mem directly?
        memcpy(JpegHeapOut->data, m_jpegHeap->data, jpegBuf.size.s);

        m_dataCb(CAMERA_MSG_COMPRESSED_IMAGE, JpegHeapOut, 0, NULL, m_callbackCookie);
    }
    }

    flashTurnOnHere = false;

    ((ExynosCameraActivityFlash *)m_secCamera->getFlashMgr())
        ->setFlashTrigerPath(ExynosCameraActivityFlash::FLASH_TRIGGER_OFF);

    ((ExynosCameraActivityFlash *)m_secCamera->getFlashMgr())
        ->setFlashStep(ExynosCameraActivityFlash::FLASH_STEP_OFF);

    /* don't stop preview.. */
    /*
    if (m_videoRunning == false)
        stopPreview();
    */
    CLOGD("DEBUG(%s):m_pictureThread end", __func__);

    ret = true;

out:
    if (JpegHeapOut) {
        JpegHeapOut->release(JpegHeapOut);
        JpegHeapOut = 0;
        JpegHeapOutFd = -1;
    }

    m_stateLock.lock();

    m_captureInProgress = false;

    m_stateLock.unlock();

#ifdef SCALABLE_SENSOR
#ifdef SCALABLE_SENSOR_CHKTIME
    struct timeval start, end;
    int sec, usec;
    gettimeofday(&start, NULL);
#endif
    if ((m_13MCaptureStart == true) &&
        (m_secCamera->getScalableSensorStart()) &&
        (m_secCamera->getCameraMode() == ExynosCamera::CAMERA_MODE_BACK)) {
        if (m_chgScalableSensorSize(SCALABLE_SENSOR_SIZE_FHD) == false) {
            CLOGE("ERR(%s):scalable sensor input change(SCALABLE_SENSOR_SIZE_FHD)!!", __func__);
            ret = false;
        }
    }
#ifdef SCALABLE_SENSOR_CHKTIME
    gettimeofday(&end, NULL);
    CLOGD("DEBUG(%s):CHKTIME m_chgScalableSensorSize all done total [to preview size](%d)", __func__, (end.tv_sec - start.tv_sec)*1000000 + (end.tv_usec - start.tv_usec));
#endif

    /* AE unlock for AE-haunting when zoom capture */
    if (m_forceAELock == true) {
        if (m_secCamera->setAutoExposureLock(false) == false)
            CLOGE("ERR(%s):setAutoExposureLock(false) fail", __func__);

        m_forceAELock = false;
    }
#endif

    ((ExynosCameraActivityFlash *)m_secCamera->getFlashMgr())->setCaptureStatus(false);

    usleep(10);

    m_captureMode = false;
    if (oldFocusMode != newFocusMode)
         m_secCamera->setFocusMode(oldFocusMode);

    return ret;
}

bool ExynosCameraHWImpl::m_startPictureInternalReprocessing(void)
{
    CLOGD("DEBUG(%s):in", __func__);

    if (m_pictureRunning == true) {
        CLOGE("ERR(%s):Aready m_pictureRunning is running", __func__);
        return false;
    }

    int pictureW, pictureH, pictureFormat, pictureFramesize;
    ExynosBuffer nullBuf;

    pictureFormat = m_secCamera->getPictureFormat();
    m_secCamera->getPictureSizeReprocessing(&pictureW, &pictureH);

    pictureFramesize = FRAME_SIZE(V4L2_PIX_2_HAL_PIXEL_FORMAT(pictureFormat), ALIGN_UP(pictureW, CAMERA_MAGIC_ALIGN), ALIGN_UP(pictureH, CAMERA_MAGIC_ALIGN));

    if (m_rawHeap) {
        m_rawHeap->release(m_rawHeap);
        m_rawHeap = 0;
        m_rawHeapFd = -1;
        m_rawHeapSize = 0;
    }

    int rawHeapSize = pictureFramesize;

    m_rawHeap = m_getMemoryCb(-1, rawHeapSize, 1, &m_rawHeapFd);
    if (!m_rawHeap || m_rawHeapFd <= 0) {
        CLOGE("ERR(%s):m_getMemoryCb(m_rawHeap, size(%d) fail", __func__, rawHeapSize);
        return false;
    }
    m_rawHeapSize = rawHeapSize;

    for (int i = 0; i < NUM_OF_PICTURE_BUF; i++) {
        if (m_pictureHeap[i]) {
            m_pictureHeap[i]->release(m_pictureHeap[i]);
            m_pictureHeap[i] = 0;
            m_pictureHeapFd[i] = -1;
        }

        m_pictureHeap[i] = m_getMemoryCb(-1, pictureFramesize, 1, &(m_pictureHeapFd[i]));
        if (!m_pictureHeap[i] || m_pictureHeapFd[i] <= 0) {
            CLOGE("ERR(%s):m_getMemoryCb(m_pictureHeap[%d], size(%d) fail", __func__, i, pictureFramesize);
            return false;
        }

        m_pictureBuf[0] = nullBuf;
        m_pictureBuf[0].virt.extP[0] = (char *)m_pictureHeap[i]->data;
        m_pictureBuf[0].fd.extFd[0] = m_pictureHeapFd[i];
        m_getAlignedYUVSize(pictureFormat, ALIGN_UP(pictureW, CAMERA_MAGIC_ALIGN), ALIGN_UP(pictureH, CAMERA_MAGIC_ALIGN), &m_pictureBuf[0]);

        m_pictureBuf[0].reserved.p = i;

        m_secCamera->setPictureBufReprocessing(&m_pictureBuf[0]);
    }

    if (m_secCamera->startPictureReprocessing() == false) {
        CLOGE("ERR(%s):Fail on m_secCamera->startPictureReprocessing()", __func__);
        return false;
    }

    for (int i = 0; i < NUM_OF_FLASH_BUF; i++)
        m_pictureBuf[i] = nullBuf;

    m_pictureRunning = true;

    CLOGD("DEBUG(%s):out", __func__);

    return true;
}

bool ExynosCameraHWImpl::m_stopPictureInternalReprocessing(void)
{
    CLOGD("DEBUG(%s):in", __func__);

    if (m_pictureRunning == false) {
        CLOGE("ERR(%s):Aready m_pictureRunning is stop", __func__);
        return false;
    }

    if (m_secCamera->flagStartPictureReprocessing() == true
        && m_secCamera->stopPictureReprocessing() == false)
        CLOGE("ERR(%s):Fail on m_secCamera->stopPicture()", __func__);

    for (int i = 0; i < NUM_OF_PICTURE_BUF; i++) {
        if (m_pictureHeap[i]) {
            m_pictureHeap[i]->release(m_pictureHeap[i]);
            m_pictureHeap[i] = 0;
            m_pictureHeapFd[i] = -1;
        }
    }

    if (m_rawHeap) {
        m_rawHeap->release(m_rawHeap);
        m_rawHeap = 0;
        m_rawHeapFd = -1;
        m_rawHeapSize = 0;
    }

    m_pictureRunning = false;

    CLOGD("DEBUG(%s):out", __func__);

    return true;
}

void ExynosCameraHWImpl::m_setSkipFrame(int frame)
{
    Mutex::Autolock lock(m_skipFrameLock);
    if (frame < m_skipFrame)
        return;

    m_skipFrame = frame;
}

int ExynosCameraHWImpl::m_getSkipFrame()
{
    Mutex::Autolock lock(m_skipFrameLock);

    return m_skipFrame;
}

void ExynosCameraHWImpl::m_decSkipFrame()
{
    Mutex::Autolock lock(m_skipFrameLock);

    m_skipFrame--;
}

int ExynosCameraHWImpl::m_saveJpeg( unsigned char *real_jpeg, int jpeg_size)
{
    FILE *yuv_fp = NULL;
    char filename[100], *buffer = NULL;

    /* file create/open, note to "wb" */
    yuv_fp = fopen("mnt/shell/emulated/0/DCIM/Camera.jpeg", "wb");
    if (yuv_fp == NULL) {
        CLOGE("Save jpeg file open error");
        return -1;
    }

    CLOGV("DEBUG(%s): real_jpeg size ========>  %d", __func__, jpeg_size);
    buffer = (char *) malloc(jpeg_size);
    if (buffer == NULL) {
        CLOGE("Save YUV] buffer alloc failed");
        if (yuv_fp)
            fclose(yuv_fp);

        return -1;
    }

    memcpy(buffer, real_jpeg, jpeg_size);

    fflush(stdout);

    fwrite(buffer, 1, jpeg_size, yuv_fp);

    fflush(yuv_fp);

    if (yuv_fp)
            fclose(yuv_fp);
    if (buffer)
            free(buffer);

    return 0;
}

bool ExynosCameraHWImpl::m_scaleDownYuv422(char *srcBuf, uint32_t srcWidth, uint32_t srcHeight,
                                             char *dstBuf, uint32_t dstWidth, uint32_t dstHeight)
{
    int32_t step_x, step_y;
    int32_t iXsrc, iXdst;
    int32_t x, y, src_y_start_pos, dst_pos, src_pos;

    if (dstWidth % 2 != 0 || dstHeight % 2 != 0) {
        CLOGE("scale_down_yuv422: invalid width, height for scaling");
        return false;
    }

    step_x = srcWidth / dstWidth;
    step_y = srcHeight / dstHeight;

    dst_pos = 0;
    for (uint32_t y = 0; y < dstHeight; y++) {
        src_y_start_pos = (y * step_y * (srcWidth * 2));

        for (uint32_t x = 0; x < dstWidth; x += 2) {
            src_pos = src_y_start_pos + (x * (step_x * 2));

            dstBuf[dst_pos++] = srcBuf[src_pos    ];
            dstBuf[dst_pos++] = srcBuf[src_pos + 1];
            dstBuf[dst_pos++] = srcBuf[src_pos + 2];
            dstBuf[dst_pos++] = srcBuf[src_pos + 3];
        }
    }

    return true;
}

bool ExynosCameraHWImpl::m_YUY2toNV21(void *srcBuf, void *dstBuf, uint32_t srcWidth, uint32_t srcHeight)
{
    int32_t        x, y, src_y_start_pos, dst_cbcr_pos, dst_pos, src_pos;
    unsigned char *srcBufPointer = (unsigned char *)srcBuf;
    unsigned char *dstBufPointer = (unsigned char *)dstBuf;

    dst_pos = 0;
    dst_cbcr_pos = srcWidth*srcHeight;
    for (uint32_t y = 0; y < srcHeight; y++) {
        src_y_start_pos = (y * (srcWidth * 2));

        for (uint32_t x = 0; x < (srcWidth * 2); x += 2) {
            src_pos = src_y_start_pos + x;

            dstBufPointer[dst_pos++] = srcBufPointer[src_pos];
        }
    }
    for (uint32_t y = 0; y < srcHeight; y += 2) {
        src_y_start_pos = (y * (srcWidth * 2));

        for (uint32_t x = 0; x < (srcWidth * 2); x += 4) {
            src_pos = src_y_start_pos + x;

            dstBufPointer[dst_cbcr_pos++] = srcBufPointer[src_pos + 3];
            dstBufPointer[dst_cbcr_pos++] = srcBufPointer[src_pos + 1];
        }
    }

    return true;
}

bool ExynosCameraHWImpl::m_checkVideoStartMarker(unsigned char *pBuf)
{
    if (!pBuf) {
        CLOGE("m_checkVideoStartMarker() => pBuf is NULL");
        return false;
    }

    if (HIBYTE(VIDEO_COMMENT_MARKER_H) == * pBuf      && LOBYTE(VIDEO_COMMENT_MARKER_H) == *(pBuf + 1) &&
        HIBYTE(VIDEO_COMMENT_MARKER_L) == *(pBuf + 2) && LOBYTE(VIDEO_COMMENT_MARKER_L) == *(pBuf + 3))
        return true;

    return false;
}

bool ExynosCameraHWImpl::m_checkEOIMarker(unsigned char *pBuf)
{
    if (!pBuf) {
        CLOGE("m_checkEOIMarker() => pBuf is NULL");
        return false;
    }

    // EOI marker [FF D9]
    if (HIBYTE(JPEG_EOI_MARKER) == *pBuf && LOBYTE(JPEG_EOI_MARKER) == *(pBuf + 1))
        return true;

    return false;
}

bool ExynosCameraHWImpl::m_findEOIMarkerInJPEG(unsigned char *pBuf, int dwBufSize, int *pnJPEGsize)
{
    if (NULL == pBuf || 0 >= dwBufSize) {
        CLOGE("m_findEOIMarkerInJPEG() => There is no contents.");
        return false;
    }

    unsigned char *pBufEnd = pBuf + dwBufSize;

    while (pBuf < pBufEnd) {
        if (m_checkEOIMarker(pBuf++))
            return true;

        (*pnJPEGsize)++;
    }

    return false;
}

bool ExynosCameraHWImpl::m_splitFrame(unsigned char *pFrame, int dwSize,
                    int dwJPEGLineLength, int dwVideoLineLength, int dwVideoHeight,
                    void *pJPEG, int *pdwJPEGSize,
                    void *pVideo, int *pdwVideoSize)
{
    CLOGV("DEBUG(%s):===========m_splitFrame Start==============", __func__);

    if (NULL == pFrame || 0 >= dwSize) {
        CLOGE("There is no contents (pFrame=%p, dwSize=%d", pFrame, dwSize);
        return false;
    }

    if (0 == dwJPEGLineLength || 0 == dwVideoLineLength) {
        CLOGE("There in no input information for decoding interleaved jpeg");
        return false;
    }

    unsigned char *pSrc = pFrame;
    unsigned char *pSrcEnd = pFrame + dwSize;

    unsigned char *pJ = (unsigned char *)pJPEG;
    int dwJSize = 0;
    unsigned char *pV = (unsigned char *)pVideo;
    int dwVSize = 0;

    bool bRet = false;
    bool isFinishJpeg = false;

    while (pSrc < pSrcEnd) {
        // Check video start marker
        if (m_checkVideoStartMarker(pSrc)) {
            int copyLength;

            if (pSrc + dwVideoLineLength <= pSrcEnd)
                copyLength = dwVideoLineLength;
            else
                copyLength = pSrcEnd - pSrc - VIDEO_COMMENT_MARKER_LENGTH;

            // Copy video data
            if (pV) {
                memcpy(pV, pSrc + VIDEO_COMMENT_MARKER_LENGTH, copyLength);
                pV += copyLength;
                dwVSize += copyLength;
            }

            pSrc += copyLength + VIDEO_COMMENT_MARKER_LENGTH;
        } else {
            // Copy pure JPEG data
            int size = 0;
            int dwCopyBufLen = dwJPEGLineLength <= pSrcEnd-pSrc ? dwJPEGLineLength : pSrcEnd - pSrc;

            if (m_findEOIMarkerInJPEG((unsigned char *)pSrc, dwCopyBufLen, &size)) {
                isFinishJpeg = true;
                size += 2;  // to count EOF marker size
            } else {
                if ((dwCopyBufLen == 1) && (pJPEG < pJ)) {
                    unsigned char checkBuf[2] = { *(pJ - 1), *pSrc };

                    if (m_checkEOIMarker(checkBuf))
                        isFinishJpeg = true;
                }
                size = dwCopyBufLen;
            }

            memcpy(pJ, pSrc, size);

            dwJSize += size;

            pJ += dwCopyBufLen;
            pSrc += dwCopyBufLen;
        }
        if (isFinishJpeg)
            break;
    }

    if (isFinishJpeg) {
        bRet = true;
        if (pdwJPEGSize)
            *pdwJPEGSize = dwJSize;
        if (pdwVideoSize)
            *pdwVideoSize = dwVSize;
    } else {
        CLOGE("DecodeInterleaveJPEG_WithOutDT() => Can not find EOI");
        bRet = false;
        if (pdwJPEGSize)
            *pdwJPEGSize = 0;
        if (pdwVideoSize)
            *pdwVideoSize = 0;
    }
    CLOGV("DEBUG(%s):===========m_splitFrame end==============", __func__);

    return bRet;
}

int ExynosCameraHWImpl::m_decodeInterleaveData(unsigned char *pInterleaveData,
                                                 int interleaveDataSize,
                                                 int yuvWidth,
                                                 int yuvHeight,
                                                 int *pJpegSize,
                                                 void *pJpegData,
                                                 void *pYuvData)
{
    if (pInterleaveData == NULL)
        return false;

    bool ret = true;
    unsigned int *interleave_ptr = (unsigned int *)pInterleaveData;
    unsigned char *jpeg_ptr = (unsigned char *)pJpegData;
    unsigned char *yuv_ptr = (unsigned char *)pYuvData;
    unsigned char *p;
    int jpeg_size = 0;
    int yuv_size = 0;

    int i = 0;

    CLOGV("DEBUG(%s):m_decodeInterleaveData Start~~~", __func__);
    while (i < interleaveDataSize) {
        if ((*interleave_ptr == 0xFFFFFFFF) || (*interleave_ptr == 0x02FFFFFF) ||
                (*interleave_ptr == 0xFF02FFFF)) {
            // Padding Data
            interleave_ptr++;
            i += 4;
        } else if ((*interleave_ptr & 0xFFFF) == 0x05FF) {
            // Start-code of YUV Data
            p = (unsigned char *)interleave_ptr;
            p += 2;
            i += 2;

            // Extract YUV Data
            if (pYuvData != NULL) {
                memcpy(yuv_ptr, p, yuvWidth * 2);
                yuv_ptr += yuvWidth * 2;
                yuv_size += yuvWidth * 2;
            }
            p += yuvWidth * 2;
            i += yuvWidth * 2;

            // Check End-code of YUV Data
            if ((*p == 0xFF) && (*(p + 1) == 0x06)) {
                interleave_ptr = (unsigned int *)(p + 2);
                i += 2;
            } else {
                ret = false;
                break;
            }
        } else {
            // Extract JPEG Data
            if (pJpegData != NULL) {
                memcpy(jpeg_ptr, interleave_ptr, 4);
                jpeg_ptr += 4;
                jpeg_size += 4;
            }
            interleave_ptr++;
            i += 4;
        }
    }
    if (ret) {
        if (pJpegData != NULL) {
            // Remove Padding after EOI
            for (i = 0; i < 3; i++) {
                if (*(--jpeg_ptr) != 0xFF) {
                    break;
                }
                jpeg_size--;
            }
            *pJpegSize = jpeg_size;

        }
        // Check YUV Data Size
        if (pYuvData != NULL) {
            if (yuv_size != (yuvWidth * yuvHeight * 2)) {
                ret = false;
            }
        }
    }
    CLOGV("DEBUG(%s):m_decodeInterleaveData End~~~", __func__);
    return ret;
}

bool ExynosCameraHWImpl::m_isSupportedPreviewSize(const int width,
                                               const int height)
{
    int maxWidth, maxHeight = 0;
    int sizeOfResSize = 0;

    m_secCamera->getSupportedPreviewSizes(&maxWidth, &maxHeight);
    sizeOfResSize = sizeof(PREVIEW_LIST) / (sizeof(int) * 2);

    if (maxWidth < width || maxHeight < height) {
        CLOGE("ERR(%s):invalid PreviewSize(maxSize(%d/%d) size(%d/%d)",
            __func__, maxWidth, maxHeight, width, height);
        return false;
    }

    for (int i = 0; i < sizeOfResSize; i++) {
        if (PREVIEW_LIST[i][0] > maxWidth || PREVIEW_LIST[i][1] > maxHeight)
            continue;
        if (PREVIEW_LIST[i][0] == width && PREVIEW_LIST[i][1] == height)
            return true;
    }

    CLOGE("ERR(%s):Invalid preview size(%dx%d)", __func__, width, height);

    return false;
}

bool ExynosCameraHWImpl::m_isSupportedPictureSize(const int width,
                                               const int height) const
{
    int maxWidth, maxHeight = 0;
    int sizeOfResSize = 0;

    m_secCamera->getSupportedPictureSizes(&maxWidth, &maxHeight);
    sizeOfResSize = sizeof(PICTURE_LIST) / (sizeof(int) * 2);

    if (maxWidth < width || maxHeight < height) {
        CLOGE("ERR(%s):invalid picture Size(maxSize(%d/%d) size(%d/%d)",
            __func__, maxWidth, maxHeight, width, height);
        return false;
    }

    for (int i = 0; i < sizeOfResSize; i++) {
        if (PICTURE_LIST[i][0] > maxWidth || PICTURE_LIST[i][1] > maxHeight)
            continue;
        if (PICTURE_LIST[i][0] == width && PICTURE_LIST[i][1] == height)
            return true;
    }

    CLOGE("ERR(%s):Invalid picture size(%dx%d)", __func__, width, height);

    return false;
}

bool ExynosCameraHWImpl::m_isSupportedVideoSize(const int width,
                                               const int height) const
{
    int maxWidth, maxHeight = 0;
    int sizeOfResSize = 0;

    m_secCamera->getSupportedVideoSizes(&maxWidth, &maxHeight);
    sizeOfResSize = sizeof(VIDEO_LIST) / (sizeof(int) * 2);

    if (maxWidth < width || maxHeight < height) {
        CLOGE("ERR(%s):invalid video Size(maxSize(%d/%d) size(%d/%d)",
            __func__, maxWidth, maxHeight, width, height);
        return false;
    }

    for (int i = 0; i < sizeOfResSize; i++) {
        if (VIDEO_LIST[i][0] > maxWidth || VIDEO_LIST[i][1] > maxHeight)
            continue;
        if (VIDEO_LIST[i][0] == width && VIDEO_LIST[i][1] == height)
            return true;
    }

    CLOGE("ERR(%s):Invalid video size(%dx%d)", __func__, width, height);

    return false;
}

int ExynosCameraHWImpl::m_getAlignedYUVSize(int colorFormat, int w, int h, ExynosBuffer *buf, bool flagAndroidColorFormat)
{
    int FrameSize = 0;

    CLOGV("[%s] (%d) colorFormat %d", __func__, __LINE__, colorFormat);
    switch (colorFormat) {
    // 1p
    case V4L2_PIX_FMT_RGB565 :
    case V4L2_PIX_FMT_YUYV :
    case V4L2_PIX_FMT_UYVY :
    case V4L2_PIX_FMT_VYUY :
    case V4L2_PIX_FMT_YVYU :
        buf->size.extS[0] = FRAME_SIZE(V4L2_PIX_2_HAL_PIXEL_FORMAT(colorFormat), w, h);
        CLOGV("V4L2_PIX_FMT_YUYV buf->size.extS[0] %d", buf->size.extS[0]);
        buf->size.extS[1] = 0;
        buf->size.extS[2] = 0;
        break;
    // 2p
    case V4L2_PIX_FMT_NV12 :
    case V4L2_PIX_FMT_NV12T :
    case V4L2_PIX_FMT_NV21 :
    case V4L2_PIX_FMT_NV12M :
    case V4L2_PIX_FMT_NV21M :
        if (flagAndroidColorFormat == true) {
            buf->size.extS[0] = w * h;
            buf->size.extS[1] = w * h / 2;
            buf->size.extS[2] = 0;
        } else {
            buf->size.extS[0] = ALIGN_UP(w, 16) * ALIGN_UP(h, 16);
            buf->size.extS[1] = ALIGN_UP(w, 16) * ALIGN_UP(h, 16) / 2;
            buf->size.extS[2] = 0;
        }
        CLOGV("V4L2_PIX_FMT_NV21 buf->size.extS[0] %d buf->size.extS[1] %d", buf->size.extS[0], buf->size.extS[1]);
        break;
    case V4L2_PIX_FMT_NV12MT_16X16 :
        if (flagAndroidColorFormat == true) {
            buf->size.extS[0] = w * h;
            buf->size.extS[1] = w * h / 2;
            buf->size.extS[2] = 0;
        } else {
            buf->size.extS[0] = ALIGN_UP(w, 16) * ALIGN_UP(h, 16);
            buf->size.extS[1] = ALIGN(buf->size.extS[0] / 2, 256);
            buf->size.extS[2] = 0;
        }
        CLOGV("V4L2_PIX_FMT_NV12M buf->size.extS[0] %d buf->size.extS[1] %d", buf->size.extS[0], buf->size.extS[1]);
        break;
    case V4L2_PIX_FMT_NV16 :
    case V4L2_PIX_FMT_NV61 :
        buf->size.extS[0] = ALIGN_UP(w, 16) * ALIGN_UP(h, 16);
        buf->size.extS[1] = ALIGN_UP(w, 16) * ALIGN_UP(h, 16);
        buf->size.extS[2] = 0;
        CLOGV("V4L2_PIX_FMT_NV16 buf->size.extS[0] %d buf->size.extS[1] %d", buf->size.extS[0], buf->size.extS[1]);
        break;
     // 3p
    case V4L2_PIX_FMT_YUV420 :
    case V4L2_PIX_FMT_YVU420 :
        /* http://developer.android.com/reference/android/graphics/ImageFormat.html#YV12 */
        if (flagAndroidColorFormat == true) {
            buf->size.extS[0] = ALIGN_UP(w, 16) * h;
            buf->size.extS[1] = ALIGN_UP(w / 2, 16) * h / 2;
            buf->size.extS[2] = ALIGN_UP(w / 2, 16) * h / 2;
        } else {
            buf->size.extS[0] = (w * h);
            buf->size.extS[1] = (w * h) >> 2;
            buf->size.extS[2] = (w * h) >> 2;
        }
        CLOGV("V4L2_PIX_FMT_YUV420 buf->size.extS[0] %d buf->size.extS[1] %d buf->size.extS[2] %d", buf->size.extS[0], buf->size.extS[1], buf->size.extS[2]);
        break;
    case V4L2_PIX_FMT_YUV420M :
    case V4L2_PIX_FMT_YVU420M :
        if (flagAndroidColorFormat == true) {
            buf->size.extS[0] = ALIGN_UP(w, 16) * h;
            buf->size.extS[1] = ALIGN_UP(w / 2, 16) * h / 2;
            buf->size.extS[2] = ALIGN_UP(w / 2, 16) * h / 2;
        } else {
            buf->size.extS[0] = ALIGN_UP(w,   32) * ALIGN_UP(h,  16);
            buf->size.extS[1] = ALIGN_UP(w/2, 16) * ALIGN_UP(h/2, 8);
            buf->size.extS[2] = ALIGN_UP(w/2, 16) * ALIGN_UP(h/2, 8);
        }
        CLOGV("V4L2_PIX_FMT_YUV420M buf->size.extS[0] %d buf->size.extS[1] %d buf->size.extS[2] %d", buf->size.extS[0], buf->size.extS[1], buf->size.extS[2]);
        break;
    case V4L2_PIX_FMT_YUV422P :
        buf->size.extS[0] = ALIGN_UP(w,   16) * ALIGN_UP(h,  16);
        buf->size.extS[1] = ALIGN_UP(w/2, 16) * ALIGN_UP(h/2, 8);
        buf->size.extS[2] = ALIGN_UP(w/2, 16) * ALIGN_UP(h/2, 8);
        CLOGV("V4L2_PIX_FMT_YUV422P buf->size.extS[0] %d buf->size.extS[1] %d buf->size.extS[2] %d", buf->size.extS[0], buf->size.extS[1], buf->size.extS[2]);
        break;
    default:
        CLOGE("ERR(%s):unmatched colorFormat(%d)", __func__, colorFormat);
        return 0;
        break;
    }

    if (buf->virt.extP[0]) {
        for (int i = 1; i < ExynosBuffer::BUFFER_PLANE_NUM_DEFAULT; i++) {
            if (buf->size.extS[i] != 0)
                buf->virt.extP[i] = buf->virt.extP[i - 1] + buf->size.extS[i - 1];
            else
                buf->virt.extP[i] = NULL;
        }
    }

    for (int i = 0; i < ExynosBuffer::BUFFER_PLANE_NUM_DEFAULT; i++)
        FrameSize += buf->size.extS[i];

    return FrameSize;
}

bool ExynosCameraHWImpl::m_getSupportedFpsList(String8 & string8Buf, int min, int max)
{
    bool ret = false;
    bool flagFirst = true;
    char strBuf[32];
    int numOfList = 0;

    numOfList = sizeof(FPS_RANGE_LIST) / (sizeof(int) * 2);

    for (int i = 0; i < numOfList; i++) {
        if (min <= FPS_RANGE_LIST[i][0] &&
            FPS_RANGE_LIST[i][1] <= max) {
            if (flagFirst == true) {
                flagFirst = false;
                snprintf(strBuf, sizeof(strBuf), "(%d,%d)", FPS_RANGE_LIST[i][0], FPS_RANGE_LIST[i][1]);
            } else {
                snprintf(strBuf, sizeof(strBuf), ",(%d,%d)", FPS_RANGE_LIST[i][0], FPS_RANGE_LIST[i][1]);
            }
            string8Buf.append(strBuf);

            ret = true;
        }
    }

    if (ret == false)
        CLOGE("ERR(%s):cannot find fps list", __func__);

    return ret;
}

bool ExynosCameraHWImpl::m_getSupportedVariableFpsList(int min, int max, int *newMin, int *newMax)
{
    bool ret = false;
    int numOfList = 0;

    numOfList = sizeof(FPS_RANGE_LIST) / (sizeof(int) * 2);

    for (int i = 0; i < numOfList; i++) {
        if (FPS_RANGE_LIST[i][1] == max) {
            if (FPS_RANGE_LIST[i][0] == min) {
                /* found exactly same */
                *newMin = FPS_RANGE_LIST[i][0];
                *newMax = FPS_RANGE_LIST[i][1];

                ret = true;
                break;
            } else if (FPS_RANGE_LIST[i][0] != max) {
                /* found max is similar one */
                *newMin = FPS_RANGE_LIST[i][0];
                *newMax = FPS_RANGE_LIST[i][1];

                ret = true;
            }
        }
    }

    if (ret == false) {
        for (int i = 0; i < numOfList; i++) {
            if (max <= FPS_RANGE_LIST[i][1] && FPS_RANGE_LIST[i][1] != FPS_RANGE_LIST[i][0]) {
                /* found similar fps */
                *newMin = FPS_RANGE_LIST[i][0];
                *newMax = FPS_RANGE_LIST[i][1];

                CLOGW("WARN(%s):calibrate new fps(%d/%d -> %d/%d)", __func__, min, max, *newMin, *newMax);

                ret = true;
                break;
            }
        }
    }

    return ret;
}

bool ExynosCameraHWImpl::m_getResolutionList(String8 & string8Buf, int *w, int *h, int mode)
{
    bool ret = false;
    bool flagFirst = true;
    char strBuf[32];
    int sizeOfResSize = 0;
    int cropX = 0, cropY = 0, cropW = 0, cropH = 0;
    int max_w = 0, max_h = 0;

    // this is up to /packages/apps/Camera/res/values/arrays.xml
    int (*RESOLUTION_LIST)[2] = {NULL,};

    switch (mode) {
    case MODE_PREVIEW:
        RESOLUTION_LIST = PREVIEW_LIST;
        sizeOfResSize = sizeof(PREVIEW_LIST) / (sizeof(int) * 2);
        break;
    case MODE_PICTURE:
        RESOLUTION_LIST = PICTURE_LIST;
        sizeOfResSize = sizeof(PICTURE_LIST) / (sizeof(int) * 2);
        break;
    case MODE_VIDEO:
        RESOLUTION_LIST = VIDEO_LIST;
        sizeOfResSize = sizeof(VIDEO_LIST) / (sizeof(int) * 2);
        break;
    default:
        CLOGE("ERR(%s):invalid mode(%d)", __func__, mode);
        return false;
        break;
    }

    for (int i = 0; i < sizeOfResSize; i++) {
        if (   RESOLUTION_LIST[i][0] <= *w
            && RESOLUTION_LIST[i][1] <= *h) {
            if (flagFirst == true) {
                snprintf(strBuf, sizeof(strBuf), "%dx%d", RESOLUTION_LIST[i][0], RESOLUTION_LIST[i][1]);
                string8Buf.append(strBuf);
                max_w = RESOLUTION_LIST[i][0];
                max_h = RESOLUTION_LIST[i][1];

                flagFirst = false;
            } else {
                snprintf(strBuf, sizeof(strBuf), ",%dx%d", RESOLUTION_LIST[i][0], RESOLUTION_LIST[i][1]);
                string8Buf.append(strBuf);
            }

            ret = true;
        }
    }

    if (ret == false)
        CLOGE("ERR(%s):cannot find resolutions", __func__);

    *w = max_w;
    *h = max_h;

    return ret;
}

bool ExynosCameraHWImpl::m_getZoomRatioList(String8 & string8Buf, int maxZoom, int start, int end)
{
    bool flagFirst = true;
    char strBuf[32];

    int cur = start;
    int step = (end - start) / (maxZoom - 1);

    for (int i = 0; i < (maxZoom - 1); i++) {
        snprintf(strBuf, sizeof(strBuf), "%d", cur);
        string8Buf.append(strBuf);
        string8Buf.append(",");
        cur += step;
    }

    snprintf(strBuf, sizeof(strBuf), "%d", end);
    string8Buf.append(strBuf);

    // ex : "100,130,160,190,220,250,280,310,340,360,400"

    return true;
}

bool ExynosCameraHWImpl::m_getMatchedPictureSize(const int src_w, const int src_h, int *dst_w, int *dst_h)
{
    int sizeOfResSize = 0;
    float src_ratio = (float)src_w / (float)src_h;
    float dst_ratio = 0;
    int maxW, maxH;

    m_secCamera->getSupportedPictureSizes(&maxW, &maxH);
    sizeOfResSize = sizeof(PICTURE_LIST) / (sizeof(int) * 2);

    for (int i = 0; i < sizeOfResSize; i++) {
        if (PICTURE_LIST[i][0] > maxW || PICTURE_LIST[i][1] > maxH)
            continue;

        dst_ratio = (float)PICTURE_LIST[i][0] / (float)PICTURE_LIST[i][1];
        if (src_ratio == dst_ratio) {
            *dst_w = PICTURE_LIST[i][0];
            *dst_h = PICTURE_LIST[i][1];
            return true;
        }
    }

    CLOGE("ERR(%s):Could not find matched ratio size(%dx%d)", __func__, src_w, src_h);

    return false;
}

int ExynosCameraHWImpl::m_bracketsStr2Ints(char *str, int num, ExynosRect2 *rect2s, int *weights, int mode)
{
    char *curStr = str;
    char buf[128];
    char *bracketsOpen;
    char *bracketsClose;

    int tempArray[5];
    int validFocusedAreas = 0;
    bool isValid;
    bool nullArea = false;
    isValid = true;

    for (int i = 0; i < num + 1; i++) {
        if (curStr == NULL) {
            if (i != num) {
                nullArea = false;
            }
            break;
        }

        bracketsOpen = strchr(curStr, '(');
        if (bracketsOpen == NULL) {
            if (i != num) {
                nullArea = false;
            }
            break;
        }

        bracketsClose = strchr(bracketsOpen, ')');
        if (bracketsClose == NULL) {
            CLOGE("ERR(%s):m_subBracketsStr2Ints(%s) fail", __func__, buf);
            if (i != num) {
                nullArea = false;
            }
            break;
        } else if (i == num) {
            return 0;
        }

        strncpy(buf, bracketsOpen, bracketsClose - bracketsOpen + 1);
        buf[bracketsClose - bracketsOpen + 1] = 0;

        if (m_subBracketsStr2Ints(5, buf, tempArray) == false) {
            nullArea = false;
            break;
        }

        rect2s[i].x1 = tempArray[0];
        rect2s[i].y1 = tempArray[1];
        rect2s[i].x2 = tempArray[2];
        rect2s[i].y2 = tempArray[3];
        weights[i] = tempArray[4];

        if (mode) {
            isValid = true;

            for (int j = 0; j < 4; j++) {
                if (tempArray[j] < -1000 || tempArray[j] > 1000)
                    isValid = false;
            }

            if (tempArray[4] < 0 || tempArray[4] > 1000)
                isValid = false;

            if (!rect2s[i].x1 && !rect2s[i].y1 && !rect2s[i].x2 && !rect2s[i].y2 && !weights[i])
                nullArea = true;
            else if (weights[i] == 0)
                isValid = false;
            else if (!(tempArray[0] == 0 && tempArray[2] == 0) && tempArray[0] >= tempArray[2])
                isValid = false;
            else if (!(tempArray[1] == 0 && tempArray[3] == 0) && tempArray[1] >= tempArray[3])
                isValid = false;
            else if (!(tempArray[0] == 0 && tempArray[2] == 0) && (tempArray[1] == 0 && tempArray[3] == 0))
                isValid = false;
            else if ((tempArray[0] == 0 && tempArray[2] == 0) && !(tempArray[1] == 0 && tempArray[3] == 0))
                isValid = false;

            if (isValid)
                validFocusedAreas++;
            else
                return 0;
        } else {
            if (rect2s[i].x1 || rect2s[i].y1 || rect2s[i].x2 || rect2s[i].y2 || weights[i])
                validFocusedAreas++;
        }

        curStr = bracketsClose;
    }
    if (nullArea && mode)
        validFocusedAreas = num;

    if (validFocusedAreas == 0)
        validFocusedAreas = 1;

    return validFocusedAreas;
}

bool ExynosCameraHWImpl::m_subBracketsStr2Ints(int num, char *str, int *arr)
{
    if (str == NULL || arr == NULL) {
        CLOGE("ERR(%s):str or arr is NULL", __func__);
        return false;
    }

    // ex : (-10,-10,0,0,300)
    char buf[128];
    char *bracketsOpen;
    char *bracketsClose;
    char *tok;

    bracketsOpen = strchr(str, '(');
    if (bracketsOpen == NULL) {
        CLOGE("ERR(%s):no '('", __func__);
        return false;
    }

    bracketsClose = strchr(bracketsOpen, ')');
    if (bracketsClose == NULL) {
        CLOGE("ERR(%s):no ')'", __func__);
        return false;
    }

    strncpy(buf, bracketsOpen + 1, bracketsClose - bracketsOpen + 1);
    buf[bracketsClose - bracketsOpen + 1] = 0;

    tok = strtok(buf, ",");
    if (tok == NULL) {
        CLOGE("ERR(%s):strtok(%s) fail", __func__, buf);
        return false;
    }

    arr[0] = atoi(tok);

    for (int i = 1; i < num; i++) {
        tok = strtok(NULL, ",");
        if (tok == NULL) {
            if (i < num - 1) {
                CLOGE("ERR(%s):strtok() (index : %d, num : %d) fail", __func__, i, num);
                return false;
            }
            break;
        }

        arr[i] = atoi(tok);
    }

    return true;
}

int ExynosCameraHWImpl::m_calibratePosition(int w, int new_w, int pos)
{
    return (float)(pos * new_w) / (float)w;
}

void ExynosCameraHWImpl::m_checkPreviewTime(void)
{
    m_previewTimer.stop();
    if (m_previewTimerIndex == CHECK_TIME_FRAME_DURATION) {

        long long totalTime = 0;
        for (int i = 0; i < CHECK_TIME_FRAME_DURATION; i++)
            totalTime += m_previewTimerTime[i];

        unsigned int gapTime = (unsigned int)(totalTime / CHECK_TIME_FRAME_DURATION);
        CLOGD("DEBUG:CHECK_TIME_PREVIEW : %d(%.1f fps)", gapTime, (1000000.0f / (float)gapTime));
        m_previewTimerIndex = 0;
    }

    if (m_previewTimerIndex != 0)
        m_previewTimerTime[m_previewTimerIndex] = m_previewTimer.durationUsecs();

    if (m_previewTimerIndex == 1)
        m_previewTimerTime[0] = m_previewTimerTime[1];

    m_previewTimerIndex++;

    m_previewTimer.start();
}

bool ExynosCameraHWImpl::m_checkPictureBufferVaild(ExynosBuffer *buf, int retry)
{
    int ret = false;

    if (buf->reserved.p >= 0
        && buf->virt.p != NULL
        && buf->size.s > 0) {
        if (((camera2_shot_ext *)buf->virt.extP[1])->shot.dm.request.frameCount != 0) {
            ret = true;
        } else {
            CLOGW("WRN(%s): frame[%d] count is 0", __func__, buf->reserved.p);
            m_secCamera->printBayerLockStatus();
        }

        goto out;
    } else if (retry > 30) {
        CLOGE("ERR(%s):time out", __func__);
        ret = true;
        goto out;
    }

    CLOGW("WRN(%s): buffer is invalid, retry after 1ms", __func__);
    usleep(1000);
out:
    return ret;
}

void ExynosCameraHWImpl::m_checkRecordingTime(void)
{
    m_recordingTimer.stop();
    if (m_recordingTimerIndex == CHECK_TIME_FRAME_DURATION) {

        long long totalTime = 0;
        for (int i = 0; i < CHECK_TIME_FRAME_DURATION; i++)
            totalTime += m_recordingTimerTime[i];

        unsigned int gapTime = (unsigned int)(totalTime / CHECK_TIME_FRAME_DURATION);
        CLOGD("DEBUG:CHECK_TIME_RECORDING : %d(%.1f fps)", gapTime, (1000000.0f / (float)gapTime));
        m_recordingTimerIndex = 0;
    }

    if (m_recordingTimerIndex != 0)
        m_recordingTimerTime[m_recordingTimerIndex] = m_recordingTimer.durationUsecs();

    if (m_recordingTimerIndex == 1)
        m_recordingTimerTime[0] = m_recordingTimerTime[1];

    m_recordingTimerIndex++;

    m_recordingTimer.start();
}

void ExynosCameraHWImpl::m_pushVideoQ(ExynosBuffer *buf)
{
    Mutex::Autolock lock(m_videoQMutex);
    m_videoQ.push_back(*buf);
}

bool ExynosCameraHWImpl::m_popVideoQ(ExynosBuffer *buf)
{
    List<ExynosBuffer>::iterator r;

    Mutex::Autolock lock(m_videoQMutex);

    if (m_videoQ.size() == 0)
        return false;

    r = m_videoQ.begin()++;
    *buf = *r;
    m_videoQ.erase(r);

    return true;
}

int ExynosCameraHWImpl::m_sizeOfVideoQ(void)
{
    Mutex::Autolock lock(m_videoQMutex);

    return m_videoQ.size();
}

void ExynosCameraHWImpl::m_releaseVideoQ(void)
{
    List<ExynosBuffer>::iterator r;

    Mutex::Autolock lock(m_videoQMutex);

    while (m_videoQ.size() > 0) {
        r = m_videoQ.begin()++;
        m_videoQ.erase(r);
    }
}

void ExynosCameraHWImpl::m_pushPreviewQ(ExynosBuffer *buf)
{
    Mutex::Autolock lock(m_previewQMutex);

    m_previewQ.push_back(*buf);
}

void ExynosCameraHWImpl::m_pushFrontPreviewQ(ExynosBuffer *buf)
{
    Mutex::Autolock lock(m_previewQMutex);

    m_previewQ.push_front(*buf);
}

bool ExynosCameraHWImpl::m_popPreviewQ(ExynosBuffer *buf)
{
    List<ExynosBuffer>::iterator r;

    Mutex::Autolock lock(m_previewQMutex);

    if (m_previewQ.size() == 0)
        return false;

    r = m_previewQ.begin()++;
    *buf = *r;
    m_previewQ.erase(r);
    return true;
}

bool ExynosCameraHWImpl::m_eraseBackPreviewQ()
{
    List<ExynosBuffer>::iterator r;
    Mutex::Autolock lock(m_previewQMutex);
    ExynosBuffer buf;

    if (m_previewQ.size() == 0)
        return false;


    r = m_previewQ.begin();
    buf = *r;
    for (unsigned int i = 0; i < m_previewQ.size() - 1; i++) {
        r++;
        buf = *r;
    }
    m_previewQ.erase(r);

    return true;
}

int ExynosCameraHWImpl::m_sizeOfPreviewQ(void)
{
    Mutex::Autolock lock(m_previewQMutex);

    return m_previewQ.size();
}

void ExynosCameraHWImpl::m_releasePreviewQ(void)
{
    List<ExynosBuffer>::iterator r;

    Mutex::Autolock lock(m_previewQMutex);

    while (m_previewQ.size() > 0) {
        r = m_previewQ.begin()++;
        m_previewQ.erase(r);
    }
}

void ExynosCameraHWImpl::m_setPreviewBufStatus(int index, int status)
{
    m_previewBufStatus[index] = status;
}

int ExynosCameraHWImpl::m_getRecordingFrame(void)
{
    Mutex::Autolock lock(m_recordingFrameMutex);

    for (int i = 0; i < NUM_OF_VIDEO_BUF; i++) {
        m_recordingFrameIndex++;

        /* rotate index */
        if (NUM_OF_VIDEO_BUF <= m_recordingFrameIndex)
            m_recordingFrameIndex = 0;

        if (m_recordingFrameAvailable[m_recordingFrameIndex] == true) {
            m_availableRecordingFrameCnt--;
            m_recordingFrameAvailable[m_recordingFrameIndex] = false;
            return m_recordingFrameIndex;
        }
    }

    for (int i = 0; i < NUM_OF_VIDEO_BUF; i++) {
        CLOGD("DEBUG(%s:%d):m_recordingFrameAvailable[%d](%d) / m_availableRecordingFrameCnt(%d)",
            __func__, __LINE__, i, m_recordingFrameAvailable[i], m_availableRecordingFrameCnt);
    }

    return -1;
}

void ExynosCameraHWImpl::m_resetRecordingFrameStatus(void)
{
    Mutex::Autolock lock(m_recordingFrameMutex);

    for (int i = 0; i < NUM_OF_VIDEO_BUF; i++)
        m_recordingFrameAvailable[i] = true;

    m_availableRecordingFrameCnt = NUM_OF_VIDEO_BUF;
    m_recordingFrameIndex = 0;
}

void ExynosCameraHWImpl::m_freeRecordingFrame(int index)
{
    Mutex::Autolock lock(m_recordingFrameMutex);

    m_availableRecordingFrameCnt++;
    m_recordingFrameAvailable[index] = true;
}

void ExynosCameraHWImpl::m_setStartPreviewComplete(int threadId, bool toggle)
{
/*    CLOGE("%s : threadId 0x%x toggle 0x%x", __func__, threadId, toggle); */
    if (toggle == true)
        m_startComplete |= (threadId);
    else
        m_startComplete &= (~threadId);
}

bool ExynosCameraHWImpl::m_checkStartPreviewComplete(uint32_t mask)
{
/*    CLOGE("%s : mask 0x%x state 0x%x", __func__, mask, m_startComplete); */
    if ((m_startComplete & mask) == mask)
        return true;
    else
        return false;
}

void ExynosCameraHWImpl::m_clearAllStartPreviewComplete(void)
{
    m_startComplete = THREAD_ID_ALL_CLEARED;
}

#ifdef SCALABLE_SENSOR
bool ExynosCameraHWImpl::m_chgScalableSensorSize(enum SCALABLE_SENSOR_SIZE sizeMode)
{
    CLOGD("DEBUG(%s):in", __func__);

    Mutex::Autolock lock(m_13MCaptureLock);
#ifdef SCALABLE_SENSOR_CHKTIME
    struct timeval start, end;
    int sec, usec;
#endif
    CLOGD("DEBUG(%s):start", __func__);

    if (m_checkScalableSate(sizeMode) == false) {
        CLOGE("ERR(%s):m_checkScalableSate() fail", __func__);
        return false;
    }

    /* when m_13MCaptureStart is true, skip previewThread */
    switch (sizeMode) {
    case SCALABLE_SENSOR_SIZE_13M:
        m_13MCaptureStart = true;
        break;
    }

    /* 1. stop sensor Thread */
#ifdef SCALABLE_SENSOR_CHKTIME
    gettimeofday(&start, NULL);
#endif

    CLOGD("DEBUG(%s): 1. stop sensor Thread start", __func__);

#if !defined(SCALABLE_SENSOR_FORCE_DONE)
    ExynosBuffer sensorBuf;
    ExynosBuffer ispBuf;
    m_secCamera->notifyStop(true);
    m_sensorThread->requestExitAndWait();
    while (m_secCamera->getNumOfShotedFrame() > 0) {
        CLOGD("DEBUG %s(%d), stop phase - %d frames are remained", __func__, __LINE__, m_secCamera->getNumOfShotedFrame());
        if (m_secCamera->getIs3a1Buf(ExynosCamera::CAMERA_MODE_BACK, &sensorBuf, &ispBuf) == false) {
            CLOGE("ERR(%s):getIs3a1Buf() fail(id:%d)", __func__, getCameraId());
        }
        if (ispBuf.reserved.p < 0) {
            CLOGW("WRN(%s): ispBuf.reserved.p = %d", __func__, ispBuf.reserved.p);
            continue;
        }
        if (m_secCamera->putISPBuf(&ispBuf) == false) {
            CLOGE("ERR(%s):putISPBuf() fail", __func__);
        }
        isp_input_count++;
        m_ispCondition.signal();
    }
    if (0 < isp_input_count)
        usleep(5000);

    isp_input_count = 0;
#else
    m_sensorThread->requestExitAndWait();
#endif

#ifdef SCALABLE_SENSOR_CHKTIME
    gettimeofday(&end, NULL);
    CLOGD("DEBUG(%s):CHKTIME 1. stop sensor Thread (%d)", __func__, (end.tv_sec - start.tv_sec)*1000000 + (end.tv_usec - start.tv_usec));
#endif

    /* 2. Sensor size change */
#ifdef SCALABLE_SENSOR_CHKTIME
    gettimeofday(&start, NULL);
#endif

    CLOGD("DEBUG(%s): 2. Sensor size change start", __func__);
    if (!m_secCamera->setScalableSensorSize(sizeMode)) {
        CLOGE("ERR(%s):setScalableSensorSize fail", __func__);
        return false;
    }

#ifdef SCALABLE_SENSOR_CHKTIME
    gettimeofday(&end, NULL);
    CLOGD("DEBUG(%s):CHKTIME 2. Sensor size change (%d)", __func__, (end.tv_sec - start.tv_sec)*1000000 + (end.tv_usec - start.tv_usec));
#endif

    /* 3. restart sensor thread */
#ifdef SCALABLE_SENSOR_CHKTIME
    gettimeofday(&start, NULL);
#endif

    CLOGD("DEBUG(%s): 3. restart sensor thread start ", __func__);
#if !defined(SCALABLE_SENSOR_FORCE_DONE)
    m_secCamera->notifyStop(false);
#endif

    m_sensorThread->run("CameraSensorThread", PRIORITY_DEFAULT);

    /* when m_13MCaptureStart is false, keep running previewThread */
    switch (sizeMode) {
    case SCALABLE_SENSOR_SIZE_FHD:
        m_13MCaptureStart = false;
        break;
    }

#ifdef SCALABLE_SENSOR_CHKTIME
    gettimeofday(&end, NULL);
    CLOGD("DEBUG(%s):CHKTIME 3. restart sensor thread (%d)", __func__, (end.tv_sec - start.tv_sec)*1000000 + (end.tv_usec - start.tv_usec));
#endif

    CLOGD("DEBUG(%s):out", __func__);

    return true;
}

bool ExynosCameraHWImpl::m_checkScalableSate(enum SCALABLE_SENSOR_SIZE sizeMode)
{
    bool ret = true;
    bool checkMode;

    if (sizeMode == SCALABLE_SENSOR_SIZE_13M) {
        checkMode = true;
    } else if (sizeMode == SCALABLE_SENSOR_SIZE_FHD) {
        checkMode = false;
    } else {
        CLOGE("ERR(%s):Unknown mode", __func__);
        return false;
    }

    if (checkMode == m_13MCaptureStart) {
        CLOGE("ERR(%s):invalid scalable sensor mode(new = %x, cur = %x)", __func__, sizeMode, m_13MCaptureStart);
        ret = false;
    }

    return ret;
}

bool ExynosCameraHWImpl::m_checkAndWaitScalableSate(enum SCALABLE_SENSOR_SIZE sizeMode)
{
    bool ret = true;
    bool checkMode;
    int retryCnt = 30;

    if (sizeMode == SCALABLE_SENSOR_SIZE_13M) {
        checkMode = true;
    } else if (sizeMode == SCALABLE_SENSOR_SIZE_FHD) {
        checkMode = false;
    } else {
        CLOGE("ERR(%s):Unknown mode", __func__);
        return false;
    }

    while (0 < retryCnt) {
        if (checkMode != m_13MCaptureStart) {
            CLOGD("DEBUG(%s): HIT scalable sensor mode", __func__);
            ret = true;
            break;
        }
        CLOGD("DEBUG(%s): waiting change scalable sensor mode", __func__);
        retryCnt--;
        usleep(30000);
    }

    if (retryCnt == 0) {
        CLOGE("ERR(%s):TIMEOUT!", __func__);
        ret = false;
    }

    return ret;
}

#endif

}; // namespace android
