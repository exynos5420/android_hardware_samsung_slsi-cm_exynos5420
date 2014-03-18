/*
 * Copyright 2008, The Android Open Source Project
 * Copyright 2013, Samsung Electronics Co. LTD
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

 /*!
 * \file      ISecCameraHardware.cpp
 * \brief     source file for Android Camera Ext HAL
 * \author    teahyung kim (tkon.kim@samsung.com)
 * \date      2013/04/30
 *
 */

#ifndef ANDROID_HARDWARE_ISECCAMERAHARDWARE_CPP
#define ANDROID_HARDWARE_ISECCAMERAHARDWARE_CPP

#define LOG_NDEBUG 0
#define LOG_NFPS 1
#define LOG_NPERFORMANCE 1

#define LOG_TAG "ISecCameraHardware"

#include <ISecCameraHardware.h>

namespace android {

ISecCameraHardware::ISecCameraHardware(int cameraId, camera_device_t *dev)
    : mCameraId(cameraId),
      mParameters(),
      mFlagANWindowRegister(false),
      mPreviewHeap(NULL),
      mRawHeap(NULL),
      mRecordingHeap(NULL),
      mJpegHeap(NULL),
      mPreviewFormat(CAM_PIXEL_FORMAT_YUV420SP),
      mPictureFormat(CAM_PIXEL_FORMAT_JPEG),
      mFliteFormat(CAM_PIXEL_FORMAT_YUV420SP),
      mMirror(false),
      mNeedSizeChange(false),
      mRecordingTrace(false),
      mMsgEnabled(0),
      mGetMemoryCb(0),
      mPreviewWindow(NULL),
      mNotifyCb(0),
      mDataCb(0),
      mDataCbTimestamp(0),
      mCallbackCookie(0),
      mDisablePostview(false),
      mSamsungApp(false),
      mAntibanding60Hz(-1),
      mHalDevice(dev)
{
    if (mCameraId == CAMERA_FACING_BACK) {
        mZoomSupport = IsZoomSupported();
        mEnableDZoom = mZoomSupport ? IsAPZoomSupported() : false;
        mFastCaptureSupport = IsFastCaptureSupportedOnRear();

        mFLiteSize.width = backFLiteSizes[0].width;
        mFLiteSize.height = backFLiteSizes[0].height;
        mFLiteCaptureSize = mFLiteSize;

        mPreviewSize.width = backPreviewSizes[0].width;
        mPreviewSize.height = backPreviewSizes[0].height;
        mOrgPreviewSize = mPreviewSize;

        mPictureSize.width = backPictureSizes[0].width;
        mPictureSize.height = backPictureSizes[0].height;

        mThumbnailSize.width = backThumbSizes[0].width;
        mThumbnailSize.height = backThumbSizes[0].height;

        mVideoSize.width = backRecordingSizes[0].width;
        mVideoSize.height = backRecordingSizes[0].height;
    } else {
        mZoomSupport = IsZoomSupported();
        mEnableDZoom = mZoomSupport ? IsAPZoomSupported() : false;
        mFastCaptureSupport = IsFastCaptureSupportedOnFront();

        mFLiteSize.width = frontFLiteSizes[0].width;
        mFLiteSize.height = frontFLiteSizes[0].height;
        mFLiteCaptureSize = mFLiteSize;

        mPreviewSize.width = frontPreviewSizes[0].width;
        mPreviewSize.height = frontPreviewSizes[0].height;
        mOrgPreviewSize = mPreviewSize;

        mPictureSize.width = frontPictureSizes[0].width;
        mPictureSize.height = frontPictureSizes[0].height;

        mThumbnailSize.width = frontThumbSizes[0].width;
        mThumbnailSize.height = frontThumbSizes[0].height;

        mVideoSize.width = frontRecordingSizes[0].width;
        mVideoSize.height = frontRecordingSizes[0].height;
    }

    mRawSize = mPreviewSize;

#if FRONT_ZSL
    rawImageMem = NULL;
    mFullPreviewHeap = NULL;
#endif

    mPreviewWindowSize.width = mPreviewWindowSize.height = 0;

    mFrameRate = 0;
    mFps = 0;
    mJpegQuality= 100;
    mSceneMode = (cam_scene_mode)sceneModes[0].val;
    mFlashMode = (cam_flash_mode)flashModes[0].val;
    if (mCameraId == CAMERA_FACING_BACK)
        mFocusMode = (cam_focus_mode)backFocusModes[0].val;
    else
        mFocusMode = (cam_focus_mode)frontFocusModes[0].val;

    mFirmwareMode = CAM_FW_MODE_NONE;
    mVtMode = CAM_VT_MODE_NONE;
    mMovieMode = false;
#ifdef DEBUG_PREVIEW_NO_FRAME /* 130221.DSLIM Delete me in a week*/
    mFactoryMode = false;
#endif
    mDtpMode = false;

    mMaxFrameRate = 30000;
    mDropFrameCount = 3;
    mbFirst_preview_started = false;

#if IS_FW_DEBUG
    if (mCameraId == CAMERA_FACING_FRONT || mCameraId == FD_SERVICE_CAMERA_ID)
        mPrintDebugEnabled = false;
#endif
    mIonCameraClient = -1;
    mPictureFrameSize = 0;
    mFullPreviewFrameSize = 0;
    CLEAR(mAntiBanding);
    mAutoFocusExit = false;
    mPreviewRunning = false;
    mRecordingRunning = false;
    mAutoFocusRunning = false;
    mPictureRunning = false;
    mRecordSrcIndex = -1;
    CLEAR(mRecordSrcSlot);
    mRecordDstIndex = -1;
    CLEAR(mRecordFrameAvailable);
    mRecordFrameAvailableCnt = 0;
    mFlagFirstFrameDump = 0;
    mPostRecordIndex = -1;
    mRecordTimestamp = 0;
    mLastRecordTimestamp = 0;
    mPostRecordExit = false;
    mPreviewInitialized = false;
    mPreviewHeapFd = -1;
    mRecordHeapFd = -1;
    CLEAR(mFrameMetadata);
    CLEAR(mFaces);
    mZSLindex = -1;
    mFullPreviewRunning = false;
    mPreviewFrameSize = 0;
    mRecordingFrameSize = 0;
    mRawFrameSize = 0;
}

ISecCameraHardware::~ISecCameraHardware()
{
    if (mPreviewHeap) {
        mPreviewHeap->release(mPreviewHeap);
        mPreviewHeap = 0;
        mPreviewHeapFd = -1;
    }
    if (mRecordingHeap) {
        mRecordingHeap->release(mRecordingHeap);
        mRecordingHeap = 0;
    }

    if (mRawHeap != NULL) {
        mRawHeap->release(mRawHeap);
        mRawHeap = 0;
    }

    if (mJpegHeap) {
        mJpegHeap->release(mJpegHeap);
        mJpegHeap = 0;
    }

    if (0 < mIonCameraClient)
        ion_client_destroy(mIonCameraClient);
    mIonCameraClient = -1;
}

bool ISecCameraHardware::init()
{
    mPreviewRunning = false;
    mFullPreviewRunning = false; /* for FRONT_ZSL */
    mPreviewInitialized = false;
#ifdef DEBUG_PREVIEW_CALLBACK
    mPreviewCbStarted = false;
#endif
    mRecordingRunning = false;
    mPictureRunning = false;
    mAutoFocusRunning = false;
    mAutoFocusExit = false;

    if (mEnableDZoom) {
        /* Thread for zoom */
        mPreviewZoomThread = new CameraThread(this, &ISecCameraHardware::previewZoomThread, "previewZoomThread");
        mPostRecordThread = new CameraThread(this, &ISecCameraHardware::postRecordThread, "postRecordThread");
        mPreviewThread = mRecordingThread = NULL;
    } else {
        mPreviewThread = new CameraThread(this, &ISecCameraHardware::previewThread, "previewThread");
        mRecordingThread = new CameraThread(this, &ISecCameraHardware::recordingThread, "recordingThread");
        mPreviewZoomThread = mPostRecordThread = NULL;
    }

    mPictureThread = new CameraThread(this, &ISecCameraHardware::pictureThread, "pictureThread");
#if FRONT_ZSL
    mZSLPictureThread = new CameraThread(this, &ISecCameraHardware::zslpictureThread, "zslpictureThread");
#endif

    mAutoFocusThread = new CameraThread(this, &ISecCameraHardware::autoFocusThread, "autoFocusThread");
    mAutoFocusThread->run("autoFocusThread", PRIORITY_DEFAULT);

#if IS_FW_DEBUG
    if (!mPrintDebugEnabled &&
        (mCameraId == CAMERA_FACING_FRONT || mCameraId == FD_SERVICE_CAMERA_ID)) {
        mPrevOffset = 0;
        mCurrOffset = 0;
        mPrevWp = 0;
        mCurrWp = 0;
        mDebugVaddr = 0;

        int ret = nativeGetDebugAddr(&mDebugVaddr);
        if (ret < 0) {
            ALOGE("ERR(%s):Fail on SecCamera->getDebugAddr()", __func__);
            mPrintDebugEnabled = false;
        } else {
            ALOGD("mDebugVaddr = 0x%x", mDebugVaddr);
            mPrintDebugEnabled = true;
#if IS_FW_DEBUG_THREAD
            mStopDebugging = false;
            mDebugThread = new DebugThread(this);
            mDebugThread->run("debugThread", PRIORITY_DEFAULT);
#endif
        }
    }
#endif

    mIonCameraClient = ion_client_create();
    if (mIonCameraClient < 0) {
        ALOGE("ERR(%s):ion_client_create() fail", __func__);
        mIonCameraClient = -1;
    }

    for (int i = 0; i < REC_BUF_CNT; i++) {
        for (int j = 0; j < REC_PLANE_CNT; j++) {
            mRecordDstHeap[i][j] = NULL;
        }
    }

    mInitRecSrcQ();
    mInitRecDstBuf();

    return true;
}

void ISecCameraHardware::initDefaultParameters()
{
    int len;

    /* Preview */
    mParameters.setPreviewSize(mPreviewSize.width, mPreviewSize.height);

    if (mCameraId == CAMERA_FACING_BACK) {
        mParameters.set(CameraParameters::KEY_SUPPORTED_PREVIEW_SIZES,
                SecCameraParameters::createSizesStr(backPreviewSizes, ARRAY_SIZE(backPreviewSizes)).string());

        mParameters.set(CameraParameters::KEY_PREVIEW_FPS_RANGE, B_KEY_PREVIEW_FPS_RANGE_VALUE);
        mParameters.set(CameraParameters::KEY_SUPPORTED_PREVIEW_FPS_RANGE, B_KEY_SUPPORTED_PREVIEW_FPS_RANGE_VALUE);

        mParameters.set(CameraParameters::KEY_SUPPORTED_PREVIEW_FRAME_RATES, B_KEY_SUPPORTED_PREVIEW_FRAME_RATES_VALUE);
        mParameters.setPreviewFrameRate(B_KEY_PREVIEW_FRAME_RATE_VALUE);

        mParameters.set(CameraParameters::KEY_PREFERRED_PREVIEW_SIZE_FOR_VIDEO, B_KEY_PREFERRED_PREVIEW_SIZE_FOR_VIDEO_VALUE);
    } else {
        mParameters.set(CameraParameters::KEY_SUPPORTED_PREVIEW_SIZES,
                SecCameraParameters::createSizesStr(frontPreviewSizes, ARRAY_SIZE(frontPreviewSizes)).string());

        mParameters.set(CameraParameters::KEY_PREVIEW_FPS_RANGE, F_KEY_PREVIEW_FPS_RANGE_VALUE);
        mParameters.set(CameraParameters::KEY_SUPPORTED_PREVIEW_FPS_RANGE, F_KEY_SUPPORTED_PREVIEW_FPS_RANGE_VALUE);

        mParameters.set(CameraParameters::KEY_SUPPORTED_PREVIEW_FRAME_RATES, F_KEY_SUPPORTED_PREVIEW_FRAME_RATES_VALUE);
        mParameters.setPreviewFrameRate(F_KEY_PREVIEW_FRAME_RATE_VALUE);

        mParameters.set(CameraParameters::KEY_PREFERRED_PREVIEW_SIZE_FOR_VIDEO, F_KEY_PREFERRED_PREVIEW_SIZE_FOR_VIDEO_VALUE);
    }

    mParameters.setPreviewFormat(previewPixelFormats[0].desc);
    mParameters.set(CameraParameters::KEY_SUPPORTED_PREVIEW_FORMATS,
        SecCameraParameters::createValuesStr(previewPixelFormats, ARRAY_SIZE(previewPixelFormats)).string());

    /* Picture */
    mParameters.setPictureSize(mPictureSize.width, mPictureSize.height);
    mParameters.set(CameraParameters::KEY_JPEG_THUMBNAIL_WIDTH, mThumbnailSize.width);
    mParameters.set(CameraParameters::KEY_JPEG_THUMBNAIL_HEIGHT, mThumbnailSize.height);

    if (mCameraId == CAMERA_FACING_BACK) {
        mParameters.set(CameraParameters::KEY_SUPPORTED_PICTURE_SIZES,
                SecCameraParameters::createSizesStr(backPictureSizes, ARRAY_SIZE(backPictureSizes)).string());

        mParameters.set(CameraParameters::KEY_SUPPORTED_JPEG_THUMBNAIL_SIZES,
                SecCameraParameters::createSizesStr(backThumbSizes, ARRAY_SIZE(backThumbSizes)).string());
    } else {
        mParameters.set(CameraParameters::KEY_SUPPORTED_PICTURE_SIZES,
                SecCameraParameters::createSizesStr(frontPictureSizes, ARRAY_SIZE(frontPictureSizes)).string());

        mParameters.set(CameraParameters::KEY_SUPPORTED_JPEG_THUMBNAIL_SIZES,
                SecCameraParameters::createSizesStr(frontThumbSizes, ARRAY_SIZE(frontThumbSizes)).string());
    }

    mParameters.setPictureFormat(picturePixelFormats[0].desc);
    mParameters.set(CameraParameters::KEY_SUPPORTED_PICTURE_FORMATS,
        SecCameraParameters::createValuesStr(picturePixelFormats, ARRAY_SIZE(picturePixelFormats)).string());

    mParameters.set(CameraParameters::KEY_JPEG_QUALITY, 100);
    mParameters.set(CameraParameters::KEY_JPEG_THUMBNAIL_QUALITY, 100);

    mParameters.set(CameraParameters::KEY_VIDEO_FRAME_FORMAT, CameraParameters::PIXEL_FORMAT_YUV420SP);

    /* Video */
    mParameters.setVideoSize(mVideoSize.width, mVideoSize.height);
    if (mCameraId == CAMERA_FACING_BACK) {
        mParameters.set(CameraParameters::KEY_SUPPORTED_VIDEO_SIZES,
            SecCameraParameters::createSizesStr(backRecordingSizes, ARRAY_SIZE(backRecordingSizes)).string());
        mParameters.set(CameraParameters::KEY_VIDEO_STABILIZATION_SUPPORTED, B_KEY_VIDEO_STABILIZATION_SUPPORTED_VALUE);
    } else {
        mParameters.set(CameraParameters::KEY_SUPPORTED_VIDEO_SIZES,
                SecCameraParameters::createSizesStr(frontRecordingSizes, ARRAY_SIZE(frontRecordingSizes)).string());
        mParameters.set(CameraParameters::KEY_VIDEO_STABILIZATION_SUPPORTED, F_KEY_VIDEO_STABILIZATION_SUPPORTED_VALUE);
    }

    mParameters.set(CameraParameters::KEY_VIDEO_SNAPSHOT_SUPPORTED, KEY_VIDEO_SNAPSHOT_SUPPORTED_VALUE);

    /* UI settings */
    mParameters.set(CameraParameters::KEY_WHITE_BALANCE, whiteBalances[0].desc);
    mParameters.set(CameraParameters::KEY_SUPPORTED_WHITE_BALANCE,
        SecCameraParameters::createValuesStr(whiteBalances, ARRAY_SIZE(whiteBalances)).string());

    mParameters.set(CameraParameters::KEY_EFFECT, effects[0].desc);
    mParameters.set(CameraParameters::KEY_SUPPORTED_EFFECTS,
       SecCameraParameters::createValuesStr(effects, ARRAY_SIZE(effects)).string());

    if (mCameraId == CAMERA_FACING_BACK) {
        mParameters.set(CameraParameters::KEY_SCENE_MODE, sceneModes[0].desc);
        mParameters.set(CameraParameters::KEY_SUPPORTED_SCENE_MODES,
                SecCameraParameters::createValuesStr(sceneModes, ARRAY_SIZE(sceneModes)).string());

        if (IsFlashSupported()) {
            mParameters.set(CameraParameters::KEY_FLASH_MODE, flashModes[0].desc);
            mParameters.set(CameraParameters::KEY_SUPPORTED_FLASH_MODES,
                    SecCameraParameters::createValuesStr(flashModes, ARRAY_SIZE(flashModes)).string());
        }

        mParameters.set(CameraParameters::KEY_FOCUS_MODE, backFocusModes[0].desc);
        mParameters.set(CameraParameters::KEY_FOCUS_DISTANCES, B_KEY_NORMAL_FOCUS_DISTANCES_VALUE);
        mParameters.set(CameraParameters::KEY_SUPPORTED_FOCUS_MODES,
                SecCameraParameters::createValuesStr(backFocusModes, ARRAY_SIZE(backFocusModes)).string());

        if (IsAutoFocusSupported()) {
            /* FOCUS AREAS supported.
             * MAX_NUM_FOCUS_AREAS > 0 : supported
             * MAX_NUM_FOCUS_AREAS = 0 : not supported
             *
             * KEY_FOCUS_AREAS = "left,top,right,bottom,weight"
             */
            mParameters.set(CameraParameters::KEY_MAX_NUM_FOCUS_AREAS, "1");
            mParameters.set(CameraParameters::KEY_FOCUS_AREAS, "(0,0,0,0,0)");
        }
    } else {
        mParameters.set(CameraParameters::KEY_FOCUS_MODE, frontFocusModes[0].desc);
        mParameters.set(CameraParameters::KEY_FOCUS_DISTANCES, F_KEY_FOCUS_DISTANCES_VALUE);
        mParameters.set(CameraParameters::KEY_SUPPORTED_FOCUS_MODES,
                SecCameraParameters::createValuesStr(frontFocusModes, ARRAY_SIZE(frontFocusModes)).string());
    }

    /* Zoom */
    if (mZoomSupport) {
        mParameters.set(CameraParameters::KEY_ZOOM, 0);
        mParameters.set(CameraParameters::KEY_MAX_ZOOM, 30);
        /* If zoom is supported, set zoom ratios as value of KEY_ZOOM_RATIOS here.
         * EX) mParameters.set(CameraParameters::KEY_ZOOM_RATIOS, "100,102,104");
         */
        mParameters.set(CameraParameters::KEY_ZOOM_RATIOS,
                "100,102,104,109,111,113,119,121,124,131,"
                "134,138,146,150,155,159,165,170,182,189,"
                "200,213,222,232,243,255,283,300,319,364,400");
        mParameters.set(CameraParameters::KEY_ZOOM_SUPPORTED, B_KEY_ZOOM_SUPPORTED_VALUE);
        mParameters.set(CameraParameters::KEY_SMOOTH_ZOOM_SUPPORTED, B_KEY_SMOOTH_ZOOM_SUPPORTED_VALUE);
    }

    mParameters.set(CameraParameters::KEY_MAX_NUM_METERING_AREAS, "0");

    mParameters.set(CameraParameters::KEY_EXPOSURE_COMPENSATION, 0);
    mParameters.set(CameraParameters::KEY_MAX_EXPOSURE_COMPENSATION, 4);
    mParameters.set(CameraParameters::KEY_MIN_EXPOSURE_COMPENSATION, -4);
    mParameters.setFloat(CameraParameters::KEY_EXPOSURE_COMPENSATION_STEP, 0.5);

    /* AE, AWB Lock */
    if (mCameraId == CAMERA_FACING_BACK) {
        mParameters.set(CameraParameters::KEY_AUTO_EXPOSURE_LOCK_SUPPORTED, B_KEY_AUTO_EXPOSURE_LOCK_SUPPORTED_VALUE);
        mParameters.set(CameraParameters::KEY_AUTO_WHITEBALANCE_LOCK_SUPPORTED, B_KEY_AUTO_WHITEBALANCE_LOCK_SUPPORTED_VALUE);
    } else {
        mParameters.set(CameraParameters::KEY_AUTO_EXPOSURE_LOCK_SUPPORTED, F_KEY_AUTO_EXPOSURE_LOCK_SUPPORTED_VALUE);
        mParameters.set(CameraParameters::KEY_AUTO_WHITEBALANCE_LOCK_SUPPORTED, F_KEY_AUTO_WHITEBALANCE_LOCK_SUPPORTED_VALUE);
    }

    /* Face Detect */
    if (mCameraId == CAMERA_FACING_BACK) {
        mParameters.set(CameraParameters::KEY_MAX_NUM_DETECTED_FACES_HW, B_KEY_MAX_NUM_DETECTED_FACES_HW_VALUE);
        mParameters.set(CameraParameters::KEY_MAX_NUM_DETECTED_FACES_SW, B_KEY_MAX_NUM_DETECTED_FACES_SW_VALUE);
    } else {
        mParameters.set(CameraParameters::KEY_MAX_NUM_DETECTED_FACES_HW, F_KEY_MAX_NUM_DETECTED_FACES_HW_VALUE);
        mParameters.set(CameraParameters::KEY_MAX_NUM_DETECTED_FACES_SW, F_KEY_MAX_NUM_DETECTED_FACES_SW_VALUE);
    }

    /* AntiBanding */
    /*
    chooseAntiBandingFrequency();
    if (mCameraId == CAMERA_FACING_BACK) {
        char supportedAntiBanding[20] = {0,};
        sprintf(supportedAntiBanding,"%s,off", (char *)mAntiBanding);

        mParameters.set(CameraParameters::KEY_SUPPORTED_ANTIBANDING, supportedAntiBanding);
        mParameters.set(CameraParameters::KEY_ANTIBANDING, antibandings[3].desc);
    }
    */

	//mhjang temp add for param error
	mParameters.set("brightness-min",100);
	mParameters.set("brightness-max",100);
	mParameters.set("saturation-min",100);
	mParameters.set("saturation-max",100);
	mParameters.set("sharpness-min",100);
	mParameters.set("sharpness-max",100);
	mParameters.set("hue-min",100);
	mParameters.set("hue-max",100);
	

    ALOGV("initDefaultParameters EX: %s", mParameters.flatten().string());
}

status_t ISecCameraHardware::setPreviewWindow(preview_stream_ops *w)
{
    mPreviewWindow = w;

    if (CC_UNLIKELY(!w)) {
        mPreviewWindowSize.width = mPreviewWindowSize.height = 0;
        ALOGE("setPreviewWindow: NULL Surface!");
        return OK;
    }


#if 0//mhjang
	int halPixelFormat = HAL_PIXEL_FORMAT_EXYNOS_YCrCb_420_SP;
#else
    int halPixelFormat = HAL_PIXEL_FORMAT_EXYNOS_YCrCb_420_SP;
//	int halPixelFormat = HAL_PIXEL_FORMAT_YV12;

    if (mMovieMode)
        halPixelFormat = V4L2_PIX_2_HAL_PIXEL_FORMAT(V4L2_PIX_FMT_NV12);
    ALOGD("DEBUG(%s): size(%d/%d)", __FUNCTION__, mPreviewSize.width, mPreviewSize.height);
    /* YV12 */
    ALOGV("setPreviewWindow: halPixelFormat = %s",
        halPixelFormat == HAL_PIXEL_FORMAT_YV12 ? "YV12" :
        halPixelFormat == HAL_PIXEL_FORMAT_YCrCb_420_SP ? "NV21" :
        halPixelFormat == HAL_PIXEL_FORMAT_CUSTOM_YCbCr_420_SP ? "NV21M" :
        halPixelFormat == HAL_PIXEL_FORMAT_EXYNOS_YCrCb_420_SP_FULL ? "NV21 FULL" :
        "Others");
#endif

    mPreviewWindowSize = mPreviewSize;
    ALOGD("DEBUG(%s): setPreviewWindow window Size width=%d height=%d", __FUNCTION__, mPreviewWindowSize.width, mPreviewWindowSize.height);
    if (nativeCreateSurface(mPreviewWindowSize.width, mPreviewWindowSize.height, halPixelFormat) == false) {
        ALOGE("setPreviewWindow: error, nativeCreateSurface");
        return UNKNOWN_ERROR;
    }

    return NO_ERROR;
}

#if IS_FW_DEBUG
void ISecCameraHardware::printDebugFirmware()
{
    unsigned int LP, CP;

    if (!mPrintDebugEnabled) {
        ALOGD("printDebugFirmware mPrintDebugEnabled is false..");
        return;
    }

    mIs_debug_ctrl.write_point = *((unsigned int *)(mDebugVaddr + FIMC_IS_FW_DEBUG_REGION_SIZE));
    mIs_debug_ctrl.assert_flag = *((unsigned int *)(mDebugVaddr + FIMC_IS_FW_DEBUG_REGION_SIZE + 4));
    mIs_debug_ctrl.pabort_flag = *((unsigned int *)(mDebugVaddr + FIMC_IS_FW_DEBUG_REGION_SIZE + 8));
    mIs_debug_ctrl.dabort_flag = *((unsigned int *)(mDebugVaddr + FIMC_IS_FW_DEBUG_REGION_SIZE + 12));

    mCurrWp = mIs_debug_ctrl.write_point;
    mCurrOffset = mCurrWp - FIMC_IS_FW_DEBUG_REGION_ADDR;

    if (mCurrWp != mPrevWp) {
        *(char *)(mDebugVaddr + mCurrOffset - 1) = '\0';
        LP = CP = mDebugVaddr + mPrevOffset;

        if (mCurrWp < mPrevWp) {
            *(char *)(mDebugVaddr + FIMC_IS_FW_DEBUG_REGION_SIZE - 1) = '\0';
            while (CP < (mDebugVaddr + FIMC_IS_FW_DEBUG_REGION_SIZE) && *(char *)CP != 0) {
                while(*(char *)CP != '\n' && *(char *)CP != 0)
                    CP++;
                *(char *)CP = NULL;
                if (*(char *)LP != 0)
                    LOGD_IS("%s", (char *)LP);
                LP = ++CP;
            }
            LP = CP = mDebugVaddr;
        }

        while (CP < (mDebugVaddr + mCurrOffset) && *(char *)CP != 0) {
            while(*(char *)CP != '\n' && *(char *)CP != 0)
                CP++;
            *(char *)CP = NULL;
            if (*(char *)LP != 0)
                LOGD_IS("%s", (char *)LP);
            LP = ++CP;
        }
    }

    mPrevWp = mIs_debug_ctrl.write_point;
    mPrevOffset = mPrevWp - FIMC_IS_FW_DEBUG_REGION_ADDR;

}

#if IS_FW_DEBUG_THREAD
bool ISecCameraHardware::debugThread()
{
    mDebugLock.lock();
    mDebugCondition.waitRelative(mDebugLock, 2000000000);
    if (mStopDebugging) {
        mDebugLock.unlock();
        return false;
    }

    printDebugFirmware();

    mDebugLock.unlock();

    return true;

}
#endif
#endif

status_t ISecCameraHardware::startPreview()
{
    ALOGD("startPreview E");

    LOG_PERFORMANCE_START(1);

    Mutex::Autolock lock(mLock);

    if (mPictureRunning) {
        ALOGW("startPreview: warning, picture is not completed yet");
        if ((mMsgEnabled & CAMERA_MSG_RAW_IMAGE) ||
            (mMsgEnabled & CAMERA_MSG_POSTVIEW_FRAME)) {
            /* upper layer can access the mmaped memory if raw or postview message is enabled
            But the data will be changed after preview is started */
            ALOGE("startPreview: error, picture data is not transferred yet");
            return INVALID_OPERATION;
        }
    }

    status_t ret;
    if (mEnableDZoom)
        ret = nativeStartPreviewZoom();
    else
        ret = nativeStartPreview();

    if (ret != NO_ERROR) {
        ALOGE("startPreview: error, nativeStartPreview");
#if IS_FW_DEBUG
        if (mCameraId == CAMERA_FACING_FRONT || mCameraId == FD_SERVICE_CAMERA_ID)
            printDebugFirmware();
#endif
        return NO_INIT;
    }

    mFlagFirstFrameDump = true;
    if (mEnableDZoom)
        ret = mPreviewZoomThread->run("previewZoomThread", PRIORITY_URGENT_DISPLAY);
    else
        ret = mPreviewThread->run("previewThread", PRIORITY_URGENT_DISPLAY);

    if (ret != NO_ERROR) {
        ALOGE("startPreview: error, Not starting preview");
        return UNKNOWN_ERROR;
    }

#if FRONT_ZSL
    if (mSamsungApp && !mMovieMode && mCameraId == CAMERA_FACING_FRONT) {
        if (nativeStartFullPreview() != NO_ERROR) {
            ALOGE("startPreview: error, nativeStartPreview");
            return NO_INIT;
        }

        if (mZSLPictureThread->run("zslpictureThread", PRIORITY_URGENT_DISPLAY) != NO_ERROR) {
            ALOGE("startPreview: error, Not starting preview");
            return UNKNOWN_ERROR;
        }

        mFullPreviewRunning = true;
    }
#endif

    LOG_PERFORMANCE_END(1, "total");

    ALOGD("startPreview X");
    return NO_ERROR;
}

bool ISecCameraHardware::previewThread()
{
    int index = nativeGetPreview();
    if (CC_UNLIKELY(index < 0)) {
        ALOGE("previewThread: error, nativeGetPreview");
#if IS_FW_DEBUG
        if (mCameraId == CAMERA_FACING_FRONT || mCameraId == FD_SERVICE_CAMERA_ID) {
            dump_is_fw_log("streamOn", (uint8_t *)mDebugVaddr, (uint32_t)FIMC_IS_FW_DEBUG_REGION_SIZE);
            printDebugFirmware();
        }
#endif
        if (!mPreviewThread->exitRequested()) {
#ifdef DEBUG_PREVIEW_NO_FRAME
            if (!mFactoryMode)
                LOG_FATAL("nativeGetPreview: failed to get a frame!");
            else
                mNotifyCb(CAMERA_MSG_ERROR, 2000, 0, mCallbackCookie);
#else
            if (!recordingEnabled())
                mNotifyCb(CAMERA_MSG_ERROR, 2000, 0, mCallbackCookie);
            else {
                ALOGI("previewThread: X");
                return false;
            }
#endif
            ALOGI("previewThread: X, after callback");
        }
        return false;
    }else if (mPreviewThread->exitRequested()) {
        return false;
    }

#ifdef DUMP_LAST_PREVIEW_FRAME
    mLastIndex = index;
#endif

    mLock.lock();

    if (mDropFrameCount > 0) {
        mDropFrameCount--;
        mLock.unlock();
        nativeReleasePreviewFrame(index);
        return true;
    }

    mLock.unlock();

    /* Notify the client of a new frame. */
    if (mMsgEnabled & CAMERA_MSG_PREVIEW_FRAME) {
#ifdef DEBUG_PREVIEW_CALLBACK
        if (!mPreviewCbStarted) {
            mPreviewCbStarted = true;
            ALOGD("preview callback started");
        }
#endif
        mDataCb(CAMERA_MSG_PREVIEW_FRAME, mPreviewHeap, index, NULL, mCallbackCookie);
    }

    /* Display a new frame */
    if (CC_LIKELY(mFlagANWindowRegister)) {
        bool ret = nativeFlushSurface(mPreviewWindowSize.width, mPreviewWindowSize.height, mPreviewFrameSize, index);
        if (CC_UNLIKELY(!ret))
            ALOGE("previewThread: error, nativeFlushSurface");
    }

#if DUMP_FILE
    static int i = 0;
    if (++i % 15 == 0) {
        dumpToFile(mPreviewHeap->data + (mPreviewFrameSize*index), mPreviewFrameSize, "/data/preview.yuv");
        i = 0;
    }
#endif

    /* Release the frame */
    int err = nativeReleasePreviewFrame(index);
    if (CC_UNLIKELY(err < 0)) {
        ALOGE("previewThread: error, nativeReleasePreviewFrame");
        return false;
    }

#if defined(SEC_USES_TVOUT) && defined(SUSPEND_ENABLE)
    nativeTvoutSuspendCall();
#endif

    /* prevent a frame rate from getting higher than the max value */
    mPreviewThread->calcFrameWaitTime(mMaxFrameRate);

    return true;
}

bool ISecCameraHardware::previewZoomThread()
{
    int index = nativeGetPreview();
    int err = -1;

ALOGE("+++ previewZoomThread()");

    if (CC_UNLIKELY(index < 0)) {
        ALOGE("previewZoomThread: error, nativeGetPreview in %s", recordingEnabled() ? "recording" : "preview");
        if (!mPreviewZoomThread->exitRequested()) {
#ifdef DEBUG_PREVIEW_NO_FRAME
            if (!mFactoryMode) {
                LOG_FATAL("nativeGetPreview: failed to get a frame!");
            } else {
                mNotifyCb(CAMERA_MSG_ERROR, 2000, 0, mCallbackCookie);
                ALOGI("previewZoomThread: X, after callback");
            }
#else
            mNotifyCb(CAMERA_MSG_ERROR, 2000, 0, mCallbackCookie);
            ALOGI("previewZoomThread: X, after callback");
#endif
        }
        return false;
    } else if (mPreviewZoomThread->exitRequested()) {
        return false;
    }

#ifdef DUMP_LAST_PREVIEW_FRAME
    mLastIndex = index;
#endif

    mPostRecordIndex = index;

    mLock.lock();
    if (mDropFrameCount > 0) {
        mDropFrameCount--;
        mLock.unlock();
        nativeReleasePreviewFrame(index);
        return true;
    }
    mLock.unlock();

    /* first frame dump to jpeg */
    if (mFlagFirstFrameDump == false) {
        memcpy(mPictureBuf.virt.extP[0], mFliteNode.buffer[index].virt.extP[0], mPictureBuf.size.extS[0]);
        nativeMakeJpegDump();
        mFlagFirstFrameDump = false;
    }

    /* when recording mode, push frame of dq from FLite */
    if (mRecordingRunning) {
        int videoSlotIndex = getRecSrcBufSlotIndex();
        if (videoSlotIndex < 0) {
            ALOGE("ERROR(%s): videoSlotIndex is -1", __func__);
        } else {
            mRecordSrcSlot[videoSlotIndex].buf = &(mFliteNode.buffer[index]);
            mRecordSrcSlot[videoSlotIndex].timestamp = mRecordTimestamp;
            /* ALOGV("DEBUG(%s): recording src(%d) adr %p, timestamp %lld", __func__, videoSlotIndex,
                (mRecordSrcSlot[videoSlotIndex].buf)->virt.p, mRecordSrcSlot[videoSlotIndex].timestamp); */
            mPushRecSrcQ(&mRecordSrcSlot[videoSlotIndex]);
            mPostRecordCondition.signal();
        }
    }

    /* Display a new frame */
    if (CC_LIKELY(mFlagANWindowRegister)) {
        bool ret = nativeFlushSurface(mPreviewWindowSize.width, mPreviewWindowSize.height, mPreviewFrameSize, index);
        if (CC_UNLIKELY(!ret))
            ALOGE("previewThread: error, nativeFlushSurface");
    } else {
        /* if not register ANativeWindow, just prepare callback buffer on CAMERA_MSG_PREVIEW_FRAME */
        if (mMsgEnabled & CAMERA_MSG_PREVIEW_FRAME)
            if (nativePreviewCallback(index, NULL) < 0)
                ALOGE("ERROR(%s): nativePreviewCallback failed", __func__);
    }

    mPreviewRunning = true;

    /* Notify the client of a new frame. */
    if (mMsgEnabled & CAMERA_MSG_PREVIEW_FRAME) {
#ifdef DEBUG_PREVIEW_CALLBACK
        if (!mPreviewCbStarted) {
            mPreviewCbStarted = true;
            ALOGD("preview callback started");
        }
#endif
        mDataCb(CAMERA_MSG_PREVIEW_FRAME, mPreviewHeap, index, NULL, mCallbackCookie);
    }

#if DUMP_FILE
    static int i = 0;
    if (++i % 15 == 0) {
        dumpToFile(mPreviewHeap->data + (mPreviewFrameSize*index), mPreviewFrameSize, "/data/preview.yuv");
        i = 0;
    }
#endif

    /* Release the frame */
    err = nativeReleasePreviewFrame(index);
    if (CC_UNLIKELY(err < 0)) {
        ALOGE("previewZoomThread: error, nativeReleasePreviewFrame");
        return false;
    }

ALOGE("--- previewZoomThread()");

    return true;
}

void ISecCameraHardware::stopPreview()
{
    ALOGD("stopPreview E");

    /*
     * try count to wait for stopping previewThread
     * maximum wait time = 30 * 10ms
     */
    int waitForCnt = 30;

    LOG_PERFORMANCE_START(1);

    {
        Mutex::Autolock lock(mLock);
        if (!mPreviewRunning) {
            ALOGW("stopPreview: warning, preview has been stopped");
            return;
        }
    }

    nativeDestroySurface();
    /* don't hold the lock while waiting for the thread to quit */
#if FRONT_ZSL
    if (mFullPreviewRunning) {
        mZSLPictureThread->requestExitAndWait();
        nativeForceStopFullPreview();
        nativeStopFullPreview();
        mFullPreviewRunning = false;
    }
#endif
    if (mEnableDZoom) {
        mPreviewZoomThread->requestExit();
        /* if previewThread can't finish, wait for 25ms */
        while (waitForCnt > 0 && mPreviewZoomThread->getTid() >= 0) {
            usleep(10000);
            waitForCnt--;
        }
    } else {
        mPreviewThread->requestExitAndWait();
    }

#ifdef DUMP_LAST_PREVIEW_FRAME
    uint32_t offset = mPreviewFrameSize * mLastIndex;
    dumpToFile(mPreviewHeap->base() + offset, mPreviewFrameSize, "/data/preview-last.dump");
#endif

    Mutex::Autolock lock(mLock);

    nativeStopPreview();

    if (mEnableDZoom == true && waitForCnt > 0)
        mPreviewZoomThread->requestExitAndWait();

    mPreviewRunning = false;
    mPreviewInitialized = false;
#ifdef DEBUG_PREVIEW_CALLBACK
    mPreviewCbStarted = false;
#endif
    mDropFrameCount = INITIAL_REAR_SKIP_FRAME;
    LOG_PERFORMANCE_END(1, "total");

    if (mPreviewHeap) {
        mPreviewHeap->release(mPreviewHeap);
        mPreviewHeap = 0;
        mPreviewHeapFd = -1;
    }

    if (mRecordingHeap) {
        mRecordingHeap->release(mRecordingHeap);
        mRecordingHeap = 0;
    }

    if (mRawHeap != NULL) {
        mRawHeap->release(mRawHeap);
        mRawHeap = 0;
    }

    if (mJpegHeap) {
        mJpegHeap->release(mJpegHeap);
        mJpegHeap = 0;
    }

    ALOGD("stopPreview X");
}

status_t ISecCameraHardware::storeMetaDataInBuffers(bool enable)
{
    ALOGV("%s", __FUNCTION__);

    if (!enable) {
        ALOGE("Non-m_frameMetadata buffer mode is not supported!");
        return INVALID_OPERATION;
    }

    return OK;
}

status_t ISecCameraHardware::startRecording()
{
    ALOGD("startRecording E");

    Mutex::Autolock lock(mLock);

    status_t ret;
    mLastRecordTimestamp = 0;
#if FRONT_ZSL
    if (mFullPreviewRunning) {
        nativeForceStopFullPreview();
        mZSLPictureThread->requestExitAndWait();
        nativeStopFullPreview();
        mFullPreviewRunning = false;
    }
#endif

    if (mEnableDZoom) {
        ret = nativeStartRecordingZoom();
    } else {
        ret = nativeStartRecording();
    }

    if (CC_UNLIKELY(ret != NO_ERROR)) {
        ALOGE("startRecording X: error, nativeStartRecording");
        return UNKNOWN_ERROR;
    }

    if (mEnableDZoom) {
        mPostRecordExit = false;
        ret = mPostRecordThread->run("postRecordThread", PRIORITY_URGENT_DISPLAY);
    } else
        ret = mRecordingThread->run("recordingThread", PRIORITY_URGENT_DISPLAY);

    if (CC_UNLIKELY(ret != NO_ERROR)) {
        mRecordingTrace = true;
        ALOGE("startRecording: error %d, Not starting recording", ret);
        return ret;
    }

    mRecordingRunning = true;

    ALOGD("startRecording X");
    return NO_ERROR;
}

bool ISecCameraHardware::recordingThread()
{
    int index = -1;
    bool ret = false;
    return true;
}

bool ISecCameraHardware::postRecordThread()
{
    int index = -1;
    bool ret = false;

    mPostRecordLock.lock();
    mPostRecordCondition.wait(mPostRecordLock);
    mPostRecordLock.unlock();

    if (mSizeOfRecSrcQ() == 0) {
        ALOGW("WARN(%s): mSizeOfRecSrcQ size is zero", __func__);
    } else {
        rec_src_buf_t srcBuf;

        while (mSizeOfRecSrcQ() > 0) {

            /* get dst video buf index */
            index = getRecDstBufIndex();
            if (index < 0) {
                ALOGW("WARN(%s): getRecDstBufIndex(%d) return", __func__, index);
                return false;
            }

            /* get src video buf */
            if (mPopRecSrcQ(&srcBuf) == false) {
                ALOGW("WARN(%s): mPopRecSrcQ(%d) failed", __func__, index);
                return false;
            }

            /* ALOGV("DEBUG(%s): SrcBuf(%d, %d, %lld), Dst idx(%d)", __func__,
                srcBuf.buf->fd.extFd[0], srcBuf.buf->fd.extFd[1], srcBuf.timestamp, index); */

            /* Notify the client of a new frame. */
            if (mMsgEnabled & CAMERA_MSG_VIDEO_FRAME) {
                /* csc from flite to video MHB and callback */
                ret = nativeCSCRecording(&srcBuf, index);
                if (ret == false) {
                    ALOGE("ERROR(%s): nativeCSCRecording failed.. SrcBuf(%d, %d, %lld), Dst idx(%d)", __func__,
                        srcBuf.buf->fd.extFd[0], srcBuf.buf->fd.extFd[1], srcBuf.timestamp, index);
                    setAvailDstBufIndex(index);
                    return false;
                } else {
                    if (0L < srcBuf.timestamp && mLastRecordTimestamp < srcBuf.timestamp) {
                        mDataCbTimestamp(srcBuf.timestamp, CAMERA_MSG_VIDEO_FRAME,
                                         mRecordingHeap, index, mCallbackCookie);
                        mLastRecordTimestamp = srcBuf.timestamp;
                        LOG_RECORD_TRACE("callback returned");
                    } else {
                        ALOGW("WARN(%s): timestamp(%lld) invaild - last timestamp(%lld) systemtime(%lld)",
                            __func__, srcBuf.timestamp, mLastRecordTimestamp, systemTime(SYSTEM_TIME_MONOTONIC));
                        setAvailDstBufIndex(index);
                    }
                }
            }
        }
    }
    LOG_RECORD_TRACE("X");
    return true;
}

void ISecCameraHardware::stopRecording()
{
    ALOGD("stopRecording E");

    Mutex::Autolock lock(mLock);
    if (!mRecordingRunning) {
        ALOGW("stopRecording: warning, recording has been stopped");
        return;
    }

    /* We request thread to exit. Don't wait. */
    if (mEnableDZoom) {
        mPostRecordExit = true;

        /* Change calling order of requestExit...() and signal
         * if you want to change requestExit() to requestExitAndWait().
         */
        mPostRecordThread->requestExit();
        mPostRecordCondition.signal();
        nativeStopRecording();
    } else {
        mRecordingThread->requestExit();
        nativeStopRecording();
    }

    mRecordingRunning = false;

    ALOGD("stopRecording X");
}

void ISecCameraHardware::releaseRecordingFrame(const void *opaque)
{
    status_t ret = NO_ERROR;
    bool found = false;
    int i;

    /* We does not release frames recorded any longer
     * if this function is called after stopRecording().
     */
    if (mEnableDZoom)
        ret = mPostRecordThread->exitRequested();
    else
        ret = mRecordingThread->exitRequested();

    if (CC_UNLIKELY(ret)) {
        ALOGW("releaseRecordingFrame: warning, we do not release any more!!");
        return;
    }

    {
        Mutex::Autolock lock(mLock);
        if (CC_UNLIKELY(!mRecordingRunning)) {
            ALOGW("releaseRecordingFrame: warning, recording is not running");
            return;
        }
    }

    struct addrs *addrs = (struct addrs *)mRecordingHeap->data;

    /* find MHB handler to match */
    if (addrs) {
        for (i = 0; i < REC_BUF_CNT; i++) {
            if ((char *)(&(addrs[i].type)) == (char *)opaque) {
                found = true;
                break;
            }
        }
    }

    mRecordDstLock.lock();
    if (found) {
        mRecordFrameAvailableCnt++;
        /* ALOGV("DEBUG(%s): found index[%d] FDy(%d), FDcbcr(%d) availableCount(%d)", __func__,
            i, addrs[i].fd_y, addrs[i].fd_cbcr, mRecordFrameAvailableCnt); */
        mRecordFrameAvailable[i] = true;
    } else
        ALOGE("ERR(%s):no matched index(%p)", __func__, (char *)opaque);

    if (mRecordFrameAvailableCnt > REC_BUF_CNT) {
        ALOGW("WARN(%s): mRecordFrameAvailableCnt is more than REC_BUF!!", __func__);
        mRecordFrameAvailableCnt = REC_BUF_CNT;
    }
    mRecordDstLock.unlock();
}

status_t ISecCameraHardware::autoFocus()
{
    ALOGV("autoFocus EX");
    /* signal autoFocusThread to run once */
    mAutoFocusCondition.signal();
    return NO_ERROR;
}

bool ISecCameraHardware::autoFocusThread()
{
    /* block until we're told to start.  we don't want to use
     * a restartable thread and requestExitAndWait() in cancelAutoFocus()
     * because it would cause deadlock between our callbacks and the
     * caller of cancelAutoFocus() which both want to grab the same lock
     * in CameraServices layer.
     */
    mAutoFocusLock.lock();
    mAutoFocusCondition.wait(mAutoFocusLock);
    mAutoFocusLock.unlock();

    /* check early exit request */
    if (mAutoFocusExit)
        return false;

    ALOGV("autoFocusThread E");
    LOG_PERFORMANCE_START(1);

    mAutoFocusRunning = true;

    if (!IsAutoFocusSupported()) {
        ALOGV("autofocus not supported");
        mNotifyCb(CAMERA_MSG_FOCUS, true, 0, mCallbackCookie);
        goto out;
    }

    if (!autoFocusCheckAndWaitPreview()) {
        ALOGI("autoFocusThread: preview not started");
        mNotifyCb(CAMERA_MSG_FOCUS, false, 0, mCallbackCookie);
        goto out;
    }

    /* Start AF operations */
    if (!nativeSetAutoFocus()) {
        ALOGE("autoFocusThread X: error, nativeSetAutofocus");
        goto out;
    }

    if (!mAutoFocusRunning) {
        ALOGV("autoFocusThread X: AF is canceled");
        nativeSetParameters(CAM_CID_FOCUS_MODE, mFocusMode | V4L2_FOCUS_MODE_DEFAULT);
        mNotifyCb(CAMERA_MSG_FOCUS, true, 0, mCallbackCookie);
        goto out;
    }

    ALOGV("autoFocusThread X: AF success");
    mNotifyCb(CAMERA_MSG_FOCUS, true, 0, mCallbackCookie);

#if 1//NOTDEFINED
    if (mMsgEnabled & CAMERA_MSG_FOCUS) {
        switch (nativeGetAutoFocus()) {
        case 0x02:
            ALOGV("autoFocusThread X: AF success");
            mNotifyCb(CAMERA_MSG_FOCUS, true, 0, mCallbackCookie);
            break;

        case 0x04:
            ALOGV("autoFocusThread X: AF cancel");
            nativeSetParameters(CAM_CID_FOCUS_MODE, mFocusMode | FOCUS_MODE_DEFAULT);
            mNotifyCb(CAMERA_MSG_FOCUS, true, 0, mCallbackCookie);
            break;

        default:
            ALOGW("autoFocusThread X: AF fail");
            mNotifyCb(CAMERA_MSG_FOCUS, false, 0, mCallbackCookie);
            break;
        }
    }
#endif
out:
    mAutoFocusRunning = false;

    LOG_PERFORMANCE_END(1, "total");
    return true;
}

status_t ISecCameraHardware::cancelAutoFocus()
{
    ALOGV("cancelAutoFocus: autoFocusThread is %s",
        mAutoFocusRunning ? "running" : "not running");

    if (!IsAutoFocusSupported())
        return NO_ERROR;

    status_t err = NO_ERROR;
    if (mAutoFocusRunning) {
        ALOGE("%s [%d]", __func__, mAutoFocusRunning);
        int i, waitUs = 3000, tryCount = (500 * 1000) / waitUs;

        err = nativeCancelAutoFocus();

        for (i = 1; i <= tryCount; i++) {
            if (mAutoFocusRunning)
                usleep(waitUs);
            else
                break;

            if (!(i % 40))
                ALOGD("AF, waiting to be cancelled\n");
        }

        if (CC_UNLIKELY(i > tryCount))
            ALOGI("cancelAutoFocus: cancel timeout");
    } else {
        ALOGV("%s [%d]", __func__, mAutoFocusRunning);
        err = nativeSetParameters(CAM_CID_FOCUS_MODE, mFocusMode | V4L2_FOCUS_MODE_DEFAULT);
    }

    if (CC_UNLIKELY(err != NO_ERROR)) {
        ALOGE("cancelAutoFocus: error, nativeCancelAutofocus");
        return UNKNOWN_ERROR;
    }

    mAutoFocusRunning = false;
    return NO_ERROR;
}

#if FRONT_ZSL
bool ISecCameraHardware::zslpictureThread()
{
    mZSLindex = nativeGetFullPreview();

    if (CC_UNLIKELY(mZSLindex < 0)) {
        ALOGE("zslpictureThread: error, nativeGetFullPreview");
        return true;
    }

    nativeReleaseFullPreviewFrame(mZSLindex);

    return true;
}
#endif

status_t ISecCameraHardware::takePicture()
{
    ALOGD("takePicture E");

    if (mFastCaptureSupport) {
        uint32_t pictureSize = (mPictureSize.width << 16) | (mPictureSize.height & 0xFFFF);
        nativeSetParameters(CAM_CID_CAPTURE_MODE, pictureSize);
    }

    if (mPreviewRunning == false) {
        ALOGW("WARN(%s): preview is not initialized", __func__);
        int retry = 10;
        while (retry > 0 || mPreviewRunning == false) {
            usleep(5000);
            retry--;
        }
    }

    if (mMovieMode) {
    /* We assert mMovieMode is zero or bigger than zero. */
#ifdef RECORDING_CAPTURE
        if (CC_UNLIKELY(!mRecordingRunning))
            return NO_ERROR;
#else
        ALOGW("takePicture: warning, not support taking picture in recording mode");
#if defined(P4NOTE_CAMERA) || defined(KONA_CAMERA)
        LOG_FATAL("Do not take picture on recording!!");
#endif
        return NO_ERROR;
#endif /* RECORDING_CAPTURE */
    }

    Mutex::Autolock lock(mLock);
    if (mPictureRunning) {
        ALOGE("takePicture: error, picture already running");
        return INVALID_OPERATION;
    }

    if (mPictureThread->run("pictureThread", PRIORITY_DEFAULT) != NO_ERROR) {
        ALOGE("takePicture: error, Not starting take picture");
        return UNKNOWN_ERROR;
    }

    mPictureRunning = true;

    ALOGD("takePicture X");
    return NO_ERROR;
}

bool ISecCameraHardware::pictureThread()
{
#ifdef RECORDING_CAPTURE
    if (mMovieMode)
        doRecordingCapture();
    else
        doCameaCapture();

    return false;
#else /* !RECORDING_CAPTURE */
    ALOGD("pictureThread E");

    if (mPreviewRunning && !mFullPreviewRunning) {
        ALOGW("takePicture: warning, preview is running");
        stopPreview();
    }

    mPictureLock.lock();

    LOG_PERFORMANCE_START(1);

    LOG_PERFORMANCE_START(2);
    if (!nativeStartSnapshot()) {
        ALOGE("pictureThread: error, nativeStartSnapshot");
        mPictureLock.unlock();
        goto out;
    }
    LOG_PERFORMANCE_END(2, "nativeStartSnapshot");

    if ((mMsgEnabled & CAMERA_MSG_SHUTTER) && (mSceneMode != SCENE_MODE_NIGHTSHOT)) {
        mPictureLock.unlock();
        mNotifyCb(CAMERA_MSG_SHUTTER, 0, 0, mCallbackCookie);
        mPictureLock.lock();
    }

    LOG_PERFORMANCE_START(3);
    int postviewOffset;
    if (!nativeGetSnapshot(&postviewOffset)) {
        ALOGE("pictureThread: error, nativeGetSnapshot");
        mPictureLock.unlock();
        mNotifyCb(CAMERA_MSG_ERROR, -1, 0, mCallbackCookie);
        goto out;
    }
    LOG_PERFORMANCE_END(3, "nativeGetSnapshot");

    mPictureLock.unlock();

    if ((mMsgEnabled & CAMERA_MSG_SHUTTER) && (mSceneMode == SCENE_MODE_NIGHTSHOT))
        mNotifyCb(CAMERA_MSG_SHUTTER, 0, 0, mCallbackCookie);

    /* Display postview */
    LOG_PERFORMANCE_START(4);
    /* callbacks for capture */
    if ((mMsgEnabled & CAMERA_MSG_RAW_IMAGE) && (mRawHeap != NULL))
        mDataCb(CAMERA_MSG_RAW_IMAGE, mRawHeap, 0, NULL, mCallbackCookie);

    if (mMsgEnabled & CAMERA_MSG_RAW_IMAGE_NOTIFY)
        mNotifyCb(CAMERA_MSG_RAW_IMAGE_NOTIFY, 0, 0, mCallbackCookie);

    if ((mMsgEnabled & CAMERA_MSG_POSTVIEW_FRAME) && (mRawHeap != NULL))
        mDataCb(CAMERA_MSG_POSTVIEW_FRAME, mRawHeap, 0, NULL, mCallbackCookie);

    if ((mMsgEnabled & CAMERA_MSG_COMPRESSED_IMAGE) && (mJpegHeap != NULL))
        mDataCb(CAMERA_MSG_COMPRESSED_IMAGE, mJpegHeap, 0, NULL, mCallbackCookie);

    LOG_PERFORMANCE_END(4, "callback functions");

#if DUMP_FILE
    dumpToFile(mJpegHeap->base(), mPictureFrameSize, "/data/capture01.jpg");
#endif

out:
    nativeStopSnapshot();
    mLock.lock();
    mPictureRunning = false;
    mLock.unlock();

    LOG_PERFORMANCE_END(1, "total");

    ALOGD("pictureThread X");
    return false;
#endif /* RECORDING_CAPTURE */
}

#ifdef RECORDING_CAPTURE
status_t ISecCameraHardware::doCameaCapture()
{
    ALOGD("doCameaCapture E");

    mPictureLock.lock();

    LOG_PERFORMANCE_START(1);

    LOG_PERFORMANCE_START(2);
    if (!nativeStartSnapshot()) {
        ALOGE("doCameaCapture: error, nativeStartSnapshot");
        mPictureLock.unlock();
        goto out;
    }
    LOG_PERFORMANCE_END(2, "nativeStartSnapshot");

    if (mMsgEnabled & CAMERA_MSG_SHUTTER) {
        mPictureLock.unlock();
        mNotifyCb(CAMERA_MSG_SHUTTER, 0, 0, mCallbackCookie);
        mPictureLock.lock();
    }

    LOG_PERFORMANCE_START(3);
    int postviewOffset;
    if (!nativeGetSnapshot(&postviewOffset)) {
        ALOGE("doCameaCapture: error, nativeGetSnapshot");
        mPictureLock.unlock();
        mNotifyCb(CAMERA_MSG_ERROR, -1, 0, mCallbackCookie);
        goto out;
    }
    LOG_PERFORMANCE_END(3, "nativeGetSnapshot");

    mPictureLock.unlock();

    /* Display postview */
    LOG_PERFORMANCE_START(4);
#if 0
    if (mMsgEnabled & CAMERA_MSG_RAW_IMAGE) {
        sp<MemoryBase> mem = new MemoryBase(mRawHeap, postviewOffset, mRawFrameSize);
        mDataCb(CAMERA_MSG_RAW_IMAGE, mem, mCallbackCookie);
    }

    if (mMsgEnabled & CAMERA_MSG_POSTVIEW_FRAME) {
        sp<MemoryBase> mem = new MemoryBase(mRawHeap, postviewOffset, mRawFrameSize);
        mDataCb(CAMERA_MSG_POSTVIEW_FRAME, mem, mCallbackCookie);
    }
#endif

    if (mMsgEnabled & CAMERA_MSG_RAW_IMAGE_NOTIFY)
        mNotifyCb(CAMERA_MSG_RAW_IMAGE_NOTIFY, 0, 0, mCallbackCookie);

    if (mMsgEnabled & CAMERA_MSG_COMPRESSED_IMAGE)
        mDataCb(CAMERA_MSG_COMPRESSED_IMAGE, mJpegHeap, 0, NULL, mCallbackCookie);

    LOG_PERFORMANCE_END(4, "callback functions");

#if DUMP_FILE
    dumpToFile(mJpegHeap->base(), mPictureFrameSize, "/data/capture01.jpg");
#endif

out:
    nativeStopSnapshot();
    mLock.lock();
    mPictureRunning = false;
    mLock.unlock();

    LOG_PERFORMANCE_END(1, "total");

    ALOGD("doCameaCapture X");
    return NO_ERROR;
}

status_t ISecCameraHardware::doRecordingCapture()
{
    ALOGD("doRecordingCapture: E");

    if (!mRecordingRunning) {
        ALOGI("doRecordingCapture: nothing to do");
        return NO_ERROR;
    }

    mPictureLock.lock();

    sp<MemoryHeapBase> rawPreview420Heap = new MemoryHeapBase(mPreviewFrameSize);
    memcpy(rawPreview420Heap->base(), (void *)((uint32_t)mPreviewHeap->data + mPreviewFrameOffset), mPreviewFrameSize);
    ALOGD("%s: mPreviewFrameSize=%d, (%dx%d)", __FUNCTION__, mPreviewFrameSize, mPreviewSize.width, mPreviewSize.height);

    uint32_t rawBufSize = mPreviewSize.width * mPreviewSize.height * 2;
    sp<MemoryHeapBase> rawFrame422Heap = new MemoryHeapBase(rawBufSize);
    conversion420to422((uint8_t *)rawPreview420Heap->base(), (uint8_t *)rawFrame422Heap->base(),
                    mPreviewSize.width, mPreviewSize.height);
    ALOGD("%s: conversion420to422, src size=%d, dest size=%d", __FUNCTION__, rawPreview420Heap->getSize(), rawFrame422Heap->getSize());

    uint32_t jpegSize;
    bool ret = nativeGetRecordingJpeg((uint8_t *)rawFrame422Heap->base(), rawFrame422Heap->getSize(),
                    mPreviewSize.width, mPreviewSize.width);
    if (CC_UNLIKELY(!ret)) {
        ALOGE("doRecordingCapture: error, nativeGetRecordingJpeg");
        mPictureLock.unlock();
        mNotifyCb(CAMERA_MSG_ERROR, -1, 0, mCallbackCookie);
        goto out;
    }

    mPictureLock.unlock();

#if DUMP_FILE
    dumpToFile(mJpegHeap->base(), mPictureFrameSize, "/data/capture01.jpg");
#endif

    if (mMsgEnabled & CAMERA_MSG_COMPRESSED_IMAGE)
        mDataCb(CAMERA_MSG_COMPRESSED_IMAGE, mJpegHeap, 0, NULL, mCallbackCookie);

out:
    mLock.lock();
    mPictureRunning = false;
    mLock.unlock();

    ALOGD("doRecordingCapture: X");
    return NO_ERROR;
}
#endif

status_t ISecCameraHardware::cancelPicture()
{
    mPictureThread->requestExitAndWait();

    ALOGD("cancelPicture EX");
    return NO_ERROR;
}

status_t ISecCameraHardware::sendCommand(int32_t command, int32_t arg1, int32_t arg2)
{
    ALOGV("sendCommand E: command %d, arg1 %d, arg2 %d", command, arg1, arg2);

    switch(command) {
    case CAMERA_CMD_DISABLE_POSTVIEW:
        mDisablePostview = arg1;
        break;

    case CAMERA_CMD_SET_SAMSUNG_APP:
        mSamsungApp = true;
        break;

    case CAMERA_CMD_START_FACE_DETECTION:
        if (arg1 == CAMERA_FACE_DETECTION_HW) {
            ALOGI("Not supported Face Detection HW");
            mParameters.set(CameraParameters::KEY_MAX_NUM_DETECTED_FACES_HW, "0");
            return BAD_VALUE; /* return BAD_VALUE if not supported. */
        } else if (arg1 == CAMERA_FACE_DETECTION_SW) {
            if (arg2 > 0) {
                ALOGI("Support Face Detection SW");
                char Result[12];
                CLEAR(Result);
                snprintf(Result, sizeof(Result), "%d", arg2);
                mParameters.set(CameraParameters::KEY_MAX_NUM_DETECTED_FACES_SW, Result);
            } else {
                ALOGI("Not supported Face Detection SW");
                mParameters.set(CameraParameters::KEY_MAX_NUM_DETECTED_FACES_SW, "0");
                return BAD_VALUE;
            }
        } else {
            ALOGI("Not supported Face Detection");
        }
        break;

    case CAMERA_CMD_STOP_FACE_DETECTION:
        if (arg1 == CAMERA_FACE_DETECTION_SW) {
            mParameters.set(CameraParameters::KEY_MAX_NUM_DETECTED_FACES_SW, "0");
        } else if (arg1 == CAMERA_FACE_DETECTION_HW) {
            /* TODO: If implement the CAMERA_FACE_DETECTION_HW, remove the "return UNKNOWN_ERROR" */
            ALOGI("Not supported Face Detection HW");
            mParameters.set(CameraParameters::KEY_MAX_NUM_DETECTED_FACES_HW, "0");
            return UNKNOWN_ERROR;
        }
        break;

    case CAMERA_CMD_SET_FLIP:
        nativeSetParameters(CAM_CID_HFLIP, arg1, 0);
        nativeSetParameters(CAM_CID_HFLIP, arg1, 1);
        mMirror = arg1;
        break;

    default:
        break;
    }

    return NO_ERROR;
}

void ISecCameraHardware::release()
{
    if (mPreviewThread != NULL) {
        mPreviewThread->requestExitAndWait();
        mPreviewThread.clear();
    }

    if (mPreviewZoomThread != NULL) {
        mPreviewZoomThread->requestExitAndWait();
        mPreviewZoomThread.clear();
    }

#if FRONT_ZSL
    if (mZSLPictureThread != NULL) {
        mZSLPictureThread->requestExitAndWait();
        mZSLPictureThread.clear();
    }
#endif

#if IS_FW_DEBUG_THREAD
    if (mCameraId == CAMERA_FACING_FRONT || mCameraId == FD_SERVICE_CAMERA_ID) {
        mStopDebugging = true;
        mDebugCondition.signal();
        if (mDebugThread != NULL) {
            mDebugThread->requestExitAndWait();
            mDebugThread.clear();
        }
    }
#endif

    if (mRecordingThread != NULL) {
        mRecordingThread->requestExitAndWait();
        mRecordingThread.clear();
    }

    if (mPostRecordThread != NULL) {
        mPostRecordThread->requestExit();
        mPostRecordExit = true;
        mPostRecordCondition.signal();
        mPostRecordThread->requestExitAndWait();
        mPostRecordThread.clear();
    }

    if (mAutoFocusThread != NULL) {
        /* this thread is normally already in it's threadLoop but blocked
         * on the condition variable.  signal it so it wakes up and can exit.
         */
        mAutoFocusThread->requestExit();
        mAutoFocusExit = true;
        mAutoFocusCondition.signal();
        mAutoFocusThread->requestExitAndWait();
        mAutoFocusThread.clear();
    }

    if (mPictureThread != NULL) {
        mPictureThread->requestExitAndWait();
        mPictureThread.clear();
    }

    if (mPreviewHeap) {
        mPreviewHeap->release(mPreviewHeap);
        mPreviewHeap = 0;
        mPreviewHeapFd = -1;
    }

    if (mRecordingHeap) {
        mRecordingHeap->release(mRecordingHeap);
        mRecordingHeap = 0;
    }

    if (mRawHeap != NULL) {
        mRawHeap->release(mRawHeap);
        mRawHeap = 0;
    }

    if (mJpegHeap) {
        mJpegHeap->release(mJpegHeap);
        mJpegHeap = 0;
    }
    nativeDestroySurface();
}

status_t ISecCameraHardware::dump(int fd) const
{
    return NO_ERROR;
}

int ISecCameraHardware::getCameraId() const
{
    return mCameraId;
}

status_t ISecCameraHardware::setParameters(const CameraParameters &params)
{
    LOG_PERFORMANCE_START(1);

    if (mPictureRunning) {
        ALOGW("setParameters: warning, capture is not complete. please wait...");
        Mutex::Autolock l(&mPictureLock);
    }

    Mutex::Autolock l(&mLock);

    ALOGV("DEBUG(%s): [Before Param] %s", __FUNCTION__, params.flatten().string());

    status_t rc, final_rc = NO_ERROR;

    if ((rc = setRecordingMode(params)))
        final_rc = rc;
    if ((rc = setMovieMode(params)))
        final_rc = rc;
    /* if ((rc = setFirmwareMode(params)))
        final_rc = rc;
     */
    if ((rc = setDtpMode(params)))
        final_rc = rc;
    if ((rc = setVtMode(params)))
        final_rc = rc;

    if ((rc = setPreviewSize(params)))
        final_rc = rc;
    if ((rc = setPreviewFormat(params)))
        final_rc = rc;
    if ((rc = setPictureSize(params)))
        final_rc = rc;
    if ((rc = setPictureFormat(params)))
        final_rc = rc;
    if ((rc = setThumbnailSize(params)))
        final_rc = rc;
    if ((rc = setJpegQuality(params)))
        final_rc = rc;
    if ((rc = setVideoSize(params)))
        final_rc = rc;
    if ((rc = setFrameRate(params)))
        final_rc = rc;
    if ((rc = setRotation(params)))
        final_rc = rc;
    if ((rc = setFocusMode(params)))
        final_rc = rc;

    /* UI settings */
    if (mCameraId == CAMERA_FACING_BACK) {
        /* Set anti-banding both rear and front camera if needed. */
#ifdef USE_CSC_FEATURE
        if ((rc = setAntiBanding()))
            final_rc = rc;
#else
        if ((rc = setAntiBanding(params)))
            final_rc = rc;
#endif
        if ((rc = setSceneMode(params)))
            final_rc = rc;
        if ((rc = setFocusAreas(params)))
            final_rc = rc;
        if ((rc = setIso(params)))
            final_rc = rc;
        if ((rc = setFlash(params)))
            final_rc = rc;
        if ((rc = setMetering(params)))
            final_rc = rc;
        if ((rc = setAutoContrast(params)))
            final_rc = rc;
        if ((rc = setAntiShake(params)))
            final_rc = rc;
        if ((rc = setFaceBeauty(params)))
            final_rc = rc;
    } else {
        if ((rc = setBlur(params)))
            final_rc = rc;
    }
    if ((rc = setZoom(params)))
        final_rc = rc;
    if ((rc = setWhiteBalance(params)))
        final_rc = rc;
    if ((rc = setEffect(params)))
        final_rc = rc;
    if ((rc = setBrightness(params)))
        final_rc = rc;
    if ((rc = setAELock(params)))
        final_rc = rc;
    if ((rc = setAWBLock(params)))
        final_rc = rc;

    if ((rc = setGps(params)))
        final_rc = rc;

#if defined(KOR_CAMERA)
    if ((rc = setSelfTestMode(params)))
        final_rc = rc;
#endif

    LOG_PERFORMANCE_END(1, "total");

    ALOGV("DEBUG(%s): [After Param] %s", __FUNCTION__, params.flatten().string());

    ALOGD("setParameters X: %s", final_rc == NO_ERROR ? "success" : "failed");
    return final_rc;
}

void ISecCameraHardware::setDropFrame(int count)
{
    /* should be locked */
    if (mDropFrameCount < count)
        mDropFrameCount = count;
}

status_t ISecCameraHardware::setAELock(const CameraParameters &params)
{
    const char *str_support = params.get(CameraParameters::KEY_AUTO_EXPOSURE_LOCK_SUPPORTED);
    if (str_support == NULL || strcmp(str_support, "true"))
        return NO_ERROR;

    const char *str = params.get(CameraParameters::KEY_AUTO_EXPOSURE_LOCK);
    const char *prevStr = mParameters.get(CameraParameters::KEY_AUTO_EXPOSURE_LOCK);
    if (str == NULL || (prevStr && !strcmp(str, prevStr)))
        return NO_ERROR;

    ALOGV("setAELock: %s", str);
    if (!(!strcmp(str, "true") || !strcmp(str, "false")))
        return BAD_VALUE;

    int val;
    mParameters.set(CameraParameters::KEY_AUTO_EXPOSURE_LOCK, str);
    if (!strcmp(str, "true"))
        val = AE_LOCK;
    else
        val = AE_UNLOCK;

    return NO_ERROR;
    /*  TODO : sctrl not defined */
    /* return nativeSetParameters(CAM_CID_AE_LOCK_UNLOCK, val); */
}

status_t ISecCameraHardware::setAWBLock(const CameraParameters &params)
{
    const char *str_support = params.get(CameraParameters::KEY_AUTO_WHITEBALANCE_LOCK_SUPPORTED);
    if (str_support == NULL || strcmp(str_support, "true"))
        return NO_ERROR;

    const char *str = params.get(CameraParameters::KEY_AUTO_WHITEBALANCE_LOCK);
    const char *prevStr = mParameters.get(CameraParameters::KEY_AUTO_WHITEBALANCE_LOCK);
    if (str == NULL || (prevStr && !strcmp(str, prevStr)))
        return NO_ERROR;

    ALOGV("setAWBLock: %s", str);
    if (!(!strcmp(str, "true") || !strcmp(str, "false")))
        return BAD_VALUE;

    int val;
    mParameters.set(CameraParameters::KEY_AUTO_WHITEBALANCE_LOCK, str);
    if (!strcmp(str, "true"))
        val = AWB_LOCK;
    else
        val = AWB_UNLOCK;

    return NO_ERROR;
    /*  TODO : sctrl not defined */
    /*  return nativeSetParameters(CAM_CID_AWB_LOCK_UNLOCK, val); */
}

#ifdef PX_COMMON_CAMERA
/*
 * called when starting preview
 */
 inline void ISecCameraHardware::setDropUnstableInitFrames()
{
    int32_t frameCount = 3;

    if (mCameraId == CAMERA_FACING_BACK) {
        if (mbFirst_preview_started == false) {
            /* When camera_start_preview is called for the first time after camera application starts. */
            if (mSceneMode == SCENE_MODE_NIGHTSHOT || mSceneMode == SCENE_MODE_FIREWORKS)
                frameCount = 3;
            else
                frameCount = mSamsungApp ? INITIAL_REAR_SKIP_FRAME : 6;

            mbFirst_preview_started = true;
        } else {
            /* When startPreview is called after camera application got started. */
           frameCount = (mSamsungApp && mMovieMode) ? (INITIAL_REAR_SKIP_FRAME + 4) : 2;
        }
    } else {
        if (mbFirst_preview_started == false) {
            /* When camera_start_preview is called for the first time after camera application starts. */
            frameCount = INITIAL_FRONT_SKIP_FRAME;
            mbFirst_preview_started = true;
        } else {
            /* When startPreview is called after camera application got started. */
            frameCount = 2;
        }
    } /* (mCameraId == CAMERA_FACING_BACK) */

    setDropFrame(frameCount);
}
#endif

status_t ISecCameraHardware::setFirmwareMode(const CameraParameters &params)
{
    const char *str = params.get(SecCameraParameters::KEY_FIRMWARE_MODE);
    if (str == NULL)
        return NO_ERROR;

    int val = SecCameraParameters::lookupAttr(firmwareModes, ARRAY_SIZE(firmwareModes), str);
    if (CC_UNLIKELY(val == NOT_FOUND)) {
        ALOGE("setFirmwareMode: error, invalid value %s", str);
        return BAD_VALUE;
    }

    ALOGV("setFirmwareMode: %s", str);
    mFirmwareMode = (cam_fw_mode)val;
    mParameters.set(SecCameraParameters::KEY_FIRMWARE_MODE, str);

    return nativeSetParameters(CAM_CID_FW_MODE, val);
}

status_t ISecCameraHardware::setDtpMode(const CameraParameters &params)
{
    int val = params.getInt(SecCameraParameters::KEY_DTP_MODE);

#ifdef DEBUG_PREVIEW_NO_FRAME /* 130221.DSLIM Delete me in a week*/
    if ((val >= 0) && !mFactoryMode) {
        mFactoryMode = true;
        ALOGD("setDtpMode: Factory mode (DTP %d)", val);
    } else if ((val < 0) && mFactoryMode) {
        mFactoryMode = false;
        ALOGD("setDtpMode: not Factory mode");
    }
#endif

    if (val == -1 || mDtpMode == (bool)val)
        return NO_ERROR;

    ALOGD("setDtpMode: %d", val);
    mDtpMode = val ? true : false;
    mParameters.set(SecCameraParameters::KEY_DTP_MODE, val);
    if (mDtpMode > 0)
        setDropFrame(15);

    return nativeSetParameters(CAM_CID_DTP_MODE, val);
}

status_t ISecCameraHardware::setVtMode(const CameraParameters &params)
{
    int val = params.getInt(SecCameraParameters::KEY_VT_MODE);
    if (val == -1 || mVtMode == (cam_vt_mode)val)
        return NO_ERROR;

    ALOGV("setVtmode: %d", val);
    mVtMode = (cam_vt_mode)val;
    mParameters.set(SecCameraParameters::KEY_VT_MODE, val);

    return nativeSetParameters(CAM_CID_VT_MODE, val);
}

status_t ISecCameraHardware::setMovieMode(const CameraParameters &params)
{
    int val = params.getInt(SecCameraParameters::KEY_MOVIE_MODE);
    if (val == -1 || mMovieMode == (bool)val)
        return NO_ERROR;

    ALOGV("setMovieMode: %d", val);
    mMovieMode = val ? true : false;
    mParameters.set(SecCameraParameters::KEY_MOVIE_MODE, val);

    /* Activate the below codes if effect setting has no problem in recording mode */
    if (((mCameraId == CAMERA_FACING_FRONT) || (mCameraId == FD_SERVICE_CAMERA_ID))
      && nativeIsInternalISP()) {
        if (mMovieMode)
            return nativeSetParameters(CAM_CID_IS_S_FORMAT_SCENARIO, IS_MODE_PREVIEW_VIDEO);
        else
            return nativeSetParameters(CAM_CID_IS_S_FORMAT_SCENARIO, IS_MODE_PREVIEW_STILL);
    }

    return nativeSetParameters(CAM_CID_MOVIE_MODE, val);
}

status_t ISecCameraHardware::setRecordingMode(const CameraParameters &params)
{
    const char *str = params.get(CameraParameters::KEY_RECORDING_HINT);
    const char *prevStr = mParameters.get(CameraParameters::KEY_RECORDING_HINT);
    if (str == NULL || (prevStr && !strcmp(str, prevStr)))
        return NO_ERROR;

    mParameters.set(CameraParameters::KEY_RECORDING_HINT, str);

    String8 recordHint(str);
    ALOGV("setRecordingMode: %s", recordHint.string());

    mMovieMode = (recordHint == "true") ? true : false;

    if (mMovieMode) {
        mFps = mMaxFrameRate / 1000;
        ALOGD("DEBUG(%s): fps(%d) %s ", __FUNCTION__, mFps, recordHint.string());
    }

    return NO_ERROR;
#ifdef NOTDEFINED
    if (((mCameraId == CAMERA_FACING_FRONT) || (mCameraId == FD_SERVICE_CAMERA_ID))
      && nativeIsInternalISP()) {
        if (mMovieMode)
            return nativeSetParameters(CAM_CID_IS_S_FORMAT_SCENARIO, IS_MODE_PREVIEW_VIDEO);
        else
            return nativeSetParameters(CAM_CID_IS_S_FORMAT_SCENARIO, IS_MODE_PREVIEW_STILL);
    }

    return nativeSetParameters(CAM_CID_MOVIE_MODE, mMovieMode);
#endif
}

status_t ISecCameraHardware::setPreviewSize(const CameraParameters &params)
{
    int width, height;
    params.getPreviewSize(&width, &height);

    if ((mPreviewSize.width == (uint32_t)width) && (mPreviewSize.height == (uint32_t)height))
        return NO_ERROR;

    if (width <= 0 || height <= 0)
        return BAD_VALUE;

    int count;
    const image_rect_type *sizes, *defaultSizes = NULL, *size = NULL;

    if (mCameraId == CAMERA_FACING_BACK) {
        count = ARRAY_SIZE(backPreviewSizes);
        sizes = backPreviewSizes;
    } else {
        count = ARRAY_SIZE(frontPreviewSizes);
        sizes = frontPreviewSizes;
    }

retry:
    for (int i = 0; i < count; i++) {
        if (((uint32_t)width == sizes[i].width) && ((uint32_t)height == sizes[i].height)) {
            size = &sizes[i];
            break;
        }
    }

    if (CC_UNLIKELY(!size)) {
        if (!defaultSizes) {
            defaultSizes = sizes;
            if (mCameraId == CAMERA_FACING_BACK) {
                count = ARRAY_SIZE(hiddenBackPreviewSizes);
                sizes = hiddenBackPreviewSizes;
            } else {
                count = ARRAY_SIZE(hiddenFrontPreviewSizes);
                sizes = hiddenFrontPreviewSizes;
            }
            goto retry;
        } else
            sizes = defaultSizes;

        ALOGW("setPreviewSize: warning, not supported size(%dx%d)", width, height);
        size = &sizes[0];
    }

    mPreviewSize = *size;
    mParameters.setPreviewSize((int)size->width, (int)size->height);

    /* FLite size depends on preview size */
    mFLiteSize = *size;

    /* backup orginal preview size due to ALIGN */
    mOrgPreviewSize = mPreviewSize;
    mPreviewSize.width = ALIGN(mPreviewSize.width, 128);
    mPreviewSize.height = ALIGN(mPreviewSize.height, 2);

    if (mPreviewSize.width < 640 || mPreviewSize.width < 480) {
        mFLiteSize.width = 640;
        mFLiteSize.height = 480;
    }

    ALOGD("DEBUG(%s)(%d): preview size %dx%d/%dx%d/%dx%d", __FUNCTION__, __LINE__,
                width, height, mOrgPreviewSize.width, mOrgPreviewSize.height,
                mPreviewSize.width, mPreviewSize.height);

    return NO_ERROR;
}

status_t ISecCameraHardware::setPreviewFormat(const CameraParameters &params)
{
    const char *str = params.getPreviewFormat();
    const char *prevStr = mParameters.getPreviewFormat();
    if (str == NULL || (prevStr && !strcmp(str, prevStr)))
        return NO_ERROR;

    int val;
retry:
    val = SecCameraParameters::lookupAttr(previewPixelFormats, ARRAY_SIZE(previewPixelFormats), str);
    if (CC_UNLIKELY(val == NOT_FOUND)) {
        ALOGW("setPreviewFormat: warning, not supported value(%s)", str);
        str = reinterpret_cast<const char*>(previewPixelFormats[0].desc);
        goto retry;
    }

    ALOGD("setPreviewFormat: %s", str);
    mPreviewFormat = (cam_pixel_format)val;
    ALOGV("setPreviewFormat: mPreviewFormat = %s",
        mPreviewFormat == CAM_PIXEL_FORMAT_YVU420P ? "YV12" :
        mPreviewFormat == CAM_PIXEL_FORMAT_YUV420SP ? "NV21" :
        "Others");
    mParameters.setPreviewFormat(str);
    return NO_ERROR;
}

status_t ISecCameraHardware::setVideoSize(const CameraParameters &params)
{
    int width = 0, height = 0;
    params.getVideoSize(&width, &height);

    if ((mVideoSize.width == (uint32_t)width) && (mVideoSize.height == (uint32_t)height))
        return NO_ERROR;

    int count;
    const image_rect_type *sizes, *defaultSizes = NULL, *size = NULL;

    if (mCameraId == CAMERA_FACING_BACK) {
        count = ARRAY_SIZE(backRecordingSizes);
        sizes = backRecordingSizes;
    } else {
        count = ARRAY_SIZE(frontRecordingSizes);
        sizes = frontRecordingSizes;
    }

retry:
    for (int i = 0; i < count; i++) {
        if (((uint32_t)width == sizes[i].width) && ((uint32_t)height == sizes[i].height)) {
            size = &sizes[i];
            break;
        }
    }

    if (CC_UNLIKELY(!size)) {
        if (!defaultSizes) {
            defaultSizes = sizes;
            if (mCameraId == CAMERA_FACING_BACK) {
                count = ARRAY_SIZE(hiddenBackRecordingSizes);
                sizes = hiddenBackRecordingSizes;
            } else {
                count = ARRAY_SIZE(hiddenFrontRecordingSizes);
                sizes = hiddenFrontRecordingSizes;
            }
            goto retry;
        } else {
            sizes = defaultSizes;
        }

        ALOGW("setVideoSize: warning, not supported size(%dx%d)", width, height);
        size = &sizes[0];
    }

    ALOGD("setVideoSize: recording %dx%d", size->width, size->height);
    mVideoSize = *size;
    mParameters.setVideoSize((int)size->width, (int)size->height);

    /* const char *str = mParameters.get(CameraParameters::KEY_VIDEO_SIZE); */
    return NO_ERROR;
}

status_t ISecCameraHardware::setPictureSize(const CameraParameters &params)
{
    int width, height;
    params.getPictureSize(&width, &height);
    ALOGD("DEBUG(%s): previous %dx%d after %dx%d", __FUNCTION__,
            mPictureSize.width, mPictureSize.height, width, height);

    if ((mPictureSize.width == (uint32_t)width) && (mPictureSize.height == (uint32_t)height)) {
        mFLiteCaptureSize = mPictureSize;
        if (mPictureSize.width < 640 || mPictureSize.width < 480) {
            mFLiteCaptureSize.width = 640;
            mFLiteCaptureSize.height = 480;
        }
        return NO_ERROR;
    }

    int count;
    const image_rect_type *sizes, *defaultSizes = NULL, *size = NULL;

    if (mCameraId == CAMERA_FACING_BACK) {
        count = ARRAY_SIZE(backPictureSizes);
        sizes = backPictureSizes;
    } else {
        count = ARRAY_SIZE(frontPictureSizes);
        sizes = frontPictureSizes;
    }

retry:
    for (int i = 0; i < count; i++) {
        if (((uint32_t)width == sizes[i].width) && ((uint32_t)height == sizes[i].height)) {
            size = &sizes[i];
            break;
        }
	}

    if (CC_UNLIKELY(!size)) {
        if (!defaultSizes) {
            defaultSizes = sizes;
            if (mCameraId == CAMERA_FACING_BACK) {
                count = ARRAY_SIZE(hiddenBackPictureSizes);
                sizes = hiddenBackPictureSizes;
            } else {
                count = ARRAY_SIZE(hiddenFrontPictureSizes);
                sizes = hiddenFrontPictureSizes;
            }
            goto retry;
        } else
            sizes = defaultSizes;

        ALOGW("setPictureSize: warning, not supported size(%dx%d)", width, height);
        size = &sizes[0];
    }

    ALOGD("setPictureSize: %dx%d", size->width, size->height);
    mPictureSize = *size;
    mParameters.setPictureSize((int)size->width, (int)size->height);

    mFLiteCaptureSize = mPictureSize;
    if (mPictureSize.width < 640 || mPictureSize.width < 480) {
        mFLiteCaptureSize.width = 640;
        mFLiteCaptureSize.height = 480;
    }
    mRawSize = mFLiteCaptureSize;

    return NO_ERROR;
}

status_t ISecCameraHardware::setPictureFormat(const CameraParameters &params)
{
    const char *str = params.getPictureFormat();
    const char *prevStr = mParameters.getPictureFormat();
    if (str == NULL || (prevStr && !strcmp(str, prevStr)))
        return NO_ERROR;

    int val;
retry:
    val = SecCameraParameters::lookupAttr(picturePixelFormats, ARRAY_SIZE(picturePixelFormats), str);
    if (CC_UNLIKELY(val == NOT_FOUND)) {
        ALOGW("setPictureFormat: warning, not supported value(%s)", str);
        str = reinterpret_cast<const char*>(picturePixelFormats[0].desc);
        goto retry;
    }

    ALOGV("setPictureFormat: %s", str);
    mPictureFormat = (cam_pixel_format)val;
    mParameters.setPictureFormat(str);
    return NO_ERROR;
}

status_t ISecCameraHardware::setThumbnailSize(const CameraParameters &params)
{
    int width, height;
    width = params.getInt(CameraParameters::KEY_JPEG_THUMBNAIL_WIDTH);
    height = params.getInt(CameraParameters::KEY_JPEG_THUMBNAIL_HEIGHT);

    if (mThumbnailSize.width == (uint32_t)width && mThumbnailSize.height == (uint32_t)height)
        return NO_ERROR;

    int count;
    const image_rect_type *sizes, *size = NULL;

    if (mCameraId == CAMERA_FACING_BACK) {
        count = ARRAY_SIZE(backThumbSizes);
        sizes = backThumbSizes;
    } else {
        count = ARRAY_SIZE(frontThumbSizes);
        sizes = frontThumbSizes;
    }

    for (int i = 0; i < count; i++) {
        if ((uint32_t)width == sizes[i].width && (uint32_t)height == sizes[i].height) {
            size = &sizes[i];
            break;
        }
    }

    if (!size) {
        ALOGW("setThumbnailSize: warning, not supported size(%dx%d)", width, height);
        size = &sizes[0];
    }

    ALOGV("setThumbnailSize: %dx%d", size->width, size->height);
    mThumbnailSize = *size;
    mParameters.set(CameraParameters::KEY_JPEG_THUMBNAIL_WIDTH, (int)size->width);
    mParameters.set(CameraParameters::KEY_JPEG_THUMBNAIL_HEIGHT, (int)size->height);

    return NO_ERROR;
}

status_t ISecCameraHardware::setJpegQuality(const CameraParameters &params)
{
    int val = params.getInt(CameraParameters::KEY_JPEG_QUALITY);
    int prevVal = mParameters.getInt(CameraParameters::KEY_JPEG_QUALITY);
    if (val == -1 || prevVal == val)
        return NO_ERROR;

    if (CC_UNLIKELY(val < 1 || val > 100)) {
        ALOGE("setJpegQuality: error, invalid value(%d)", val);
        return BAD_VALUE;
    }

    ALOGV("setJpegQuality: %d", val);
    mJpegQuality = val;
    mParameters.set(CameraParameters::KEY_JPEG_QUALITY, val);

#if 1 //NOTDEFINED
    return nativeSetParameters(CAM_CID_JPEG_QUALITY, val);
#else
    return 0;
#endif
}

status_t ISecCameraHardware::setFrameRate(const CameraParameters &params)
{
    int min, max;
    params.getPreviewFpsRange(&min, &max);
    int frameRate = params.getPreviewFrameRate();
    int prevFrameRate = mParameters.getPreviewFrameRate();
    if ((frameRate != -1) && (frameRate != prevFrameRate))
        mParameters.setPreviewFrameRate(frameRate);

    if (CC_UNLIKELY(min < 0 || max < 0 || max < min)) {
        ALOGE("setFrameRate: error, invalid range(%d, %d)", min, max);
        return BAD_VALUE;
    }

    /* 0 means auto frame rate */
    int val = (min == max) ? min : 0;
    mMaxFrameRate = max;

    if (mMovieMode)
        mFps = mMaxFrameRate / 1000;
    else
        mFps = val / 1000;

    ALOGV("setFrameRate: %d,%d,%d", min, max, mFps);

    if (mFrameRate == val)
        return NO_ERROR;

    mFrameRate = val;

    const char *str = params.get(CameraParameters::KEY_PREVIEW_FPS_RANGE);
    if (CC_LIKELY(str)) {
        mParameters.set(CameraParameters::KEY_PREVIEW_FPS_RANGE, str);
    } else {
        ALOGE("setFrameRate: corrupted data (params)");
        char buffer[32];
        CLEAR(buffer);
        snprintf(buffer, sizeof(buffer), "%d,%d", min, max);
        mParameters.set(CameraParameters::KEY_PREVIEW_FPS_RANGE, buffer);
    }

    mParameters.setPreviewFrameRate(val/1000);
    return NO_ERROR;
}

status_t ISecCameraHardware::setRotation(const CameraParameters &params)
{
    int val = params.getInt(CameraParameters::KEY_ROTATION);
    int prevVal = mParameters.getInt(CameraParameters::KEY_ROTATION);
    if (val == -1 || prevVal == val)
        return NO_ERROR;

    if (CC_UNLIKELY(val != 0 && val != 90 && val != 180 && val != 270)) {
        ALOGE("setRotation: error, invalid value(%d)", val);
        return BAD_VALUE;
    }

    ALOGV("setRotation: %d", val);
    mParameters.set(CameraParameters::KEY_ROTATION, val);

    if (mVtMode)
        return nativeSetParameters(CAM_CID_ROTATION, val);

    return NO_ERROR;
}

status_t ISecCameraHardware::setPreviewFrameRate(const CameraParameters &params)
{
    int val = params.getPreviewFrameRate();
    int prevVal = mParameters.getPreviewFrameRate();
    if (val == -1 || prevVal == val)
        return NO_ERROR;

    if (CC_UNLIKELY(val < 0 || val > (mMaxFrameRate / 1000) )) {
        ALOGE("setPreviewFrameRate: error, invalid value(%d)", val);
        return BAD_VALUE;
    }

    ALOGV("setPreviewFrameRate: %d", val);
    mFrameRate = val * 1000;
    mParameters.setPreviewFrameRate(val);

    return NO_ERROR;
}

status_t ISecCameraHardware::setSceneMode(const CameraParameters &params)
{
    const char *str = params.get(CameraParameters::KEY_SCENE_MODE);
    const char *prevStr = mParameters.get(CameraParameters::KEY_SCENE_MODE);
    if (str == NULL || (prevStr && !strcmp(str, prevStr)))
        return NO_ERROR;

    int val;

retry:
    val = SecCameraParameters::lookupAttr(sceneModes, ARRAY_SIZE(sceneModes), str);
    if (CC_UNLIKELY(val == NOT_FOUND)) {
        ALOGW("setSceneMode: warning, not supported value(%s)", str);
        str = reinterpret_cast<const char*>(sceneModes[0].desc);
        goto retry;
    }

    ALOGV("setSceneMode: %s", str);
    mSceneMode = (cam_scene_mode)val;
    mParameters.set(CameraParameters::KEY_SCENE_MODE, str);

    return nativeSetParameters(CAM_CID_SCENE_MODE, val);
}

/* -------------------Focus Area STARTS here---------------------------- */
status_t ISecCameraHardware::findCenter(struct FocusArea *focusArea,
        struct FocusPoint *center)
{
    /* range check */
    if ((focusArea->top > focusArea->bottom) || (focusArea->right < focusArea->left)) {
        ALOGE("findCenter: Invalid value range");
        return -EINVAL;
    }

    center->x = (focusArea->left + focusArea->right) / 2;
    center->y = (focusArea->top + focusArea->bottom) / 2;

    /* ALOGV("%s: center point (%d, %d)", __func__, center->x, center->y); */
    return NO_ERROR;
}

status_t ISecCameraHardware::normalizeArea(struct FocusPoint *center)
{
    struct FocusPoint tmpPoint;
    size_t hRange, vRange;
    double hScale, vScale;

    tmpPoint.x = center->x;
    tmpPoint.y = center->y;

    /* ALOGD("%s: before x = %d, y = %d", __func__, tmpPoint.x, tmpPoint.y); */

    hRange = FOCUS_AREA_RIGHT - FOCUS_AREA_LEFT;
    vRange = FOCUS_AREA_BOTTOM - FOCUS_AREA_TOP;
    hScale = (double)mPreviewSize.height / (double) hRange;
    vScale = (double)mPreviewSize.width / (double) vRange;

    /* Nomalization */
    /* ALOGV("normalizeArea: mPreviewSize.width = %d, mPreviewSize.height = %d",
            mPreviewSize.width, mPreviewSize.height);
     */

    tmpPoint.x = (center->x + vRange / 2) * vScale;
    tmpPoint.y = (center->y + hRange / 2) * hScale;

    center->x = tmpPoint.x;
    center->y = tmpPoint.y;

    if (center->x == 0 && center->y == 0) {
        ALOGE("normalizeArea: Invalid focus center point");
        return -EINVAL;
    }

    return NO_ERROR;
}

status_t ISecCameraHardware::checkArea(ssize_t top,
        ssize_t left,
        ssize_t bottom,
        ssize_t right,
        ssize_t weight)
{
    /* Handles the invalid regin corner case. */
    if ((0 == top) && (0 == left) && (0 == bottom) && (0 == right) && (0 == weight)) {
        ALOGE("checkArea: error, All values are zero");
        return NO_ERROR;
    }

    if ((FOCUS_AREA_WEIGHT_MIN > weight) || (FOCUS_AREA_WEIGHT_MAX < weight)) {
        ALOGE("checkArea: error, Camera area weight is invalid %ld", weight);
        return -EINVAL;
    }

    if ((FOCUS_AREA_TOP > top) || (FOCUS_AREA_BOTTOM < top)) {
        ALOGE("checkArea: error, Camera area top coordinate is invalid %ld", top );
        return -EINVAL;
    }

    if ((FOCUS_AREA_TOP > bottom) || (FOCUS_AREA_BOTTOM < bottom)) {
        ALOGE("checkArea: error, Camera area bottom coordinate is invalid %ld", bottom );
        return -EINVAL;
    }

    if ((FOCUS_AREA_LEFT > left) || (FOCUS_AREA_RIGHT < left)) {
        ALOGE("checkArea: error, Camera area left coordinate is invalid %ld", left );
        return -EINVAL;
    }

    if ((FOCUS_AREA_LEFT > right) || (FOCUS_AREA_RIGHT < right)) {
        ALOGE("checkArea: error, Camera area right coordinate is invalid %ld", right );
        return -EINVAL;
    }

    if (left >= right) {
        ALOGE("checkArea: error, Camera area left larger than right");
        return -EINVAL;
    }

    if (top >= bottom) {
        ALOGE("checkArea: error, Camera area top larger than bottom");
        return -EINVAL;
    }

    return NO_ERROR;
}

/* TODO : muliple focus area is not supported yet */
status_t ISecCameraHardware::parseAreas(const char *area,
        size_t areaLength,
        struct FocusArea *focusArea,
        int *num_areas)
{
    status_t ret = NO_ERROR;
    char *ctx;
    char *pArea = NULL;
    char *pStart = NULL;
    char *pEnd = NULL;
    const char *startToken = "(";
    const char endToken = ')';
    const char sep = ',';
    ssize_t left, top, bottom, right, weight;
    char *tmpBuffer = NULL;

    if (( NULL == area ) || ( 0 >= areaLength)) {
        ALOGE("parseAreas: error, area is NULL or areaLength is less than 0");
        return -EINVAL;
    }

    tmpBuffer = (char *)malloc(areaLength);
    if (NULL == tmpBuffer) {
        ALOGE("parseAreas: error, tmpBuffer is NULL");
        return -ENOMEM;
    }

    memcpy(tmpBuffer, area, areaLength);

    pArea = strtok_r(tmpBuffer, startToken, &ctx);

    do {
        pStart = pArea;
        if (NULL == pStart) {
            ALOGE("parseAreas: error, Parsing of the left area coordinate failed!");
            ret = -EINVAL;
            break;
        } else {
            left = static_cast<ssize_t>(strtol(pStart, &pEnd, 10));
        }

        if (sep != *pEnd) {
            ALOGE("parseAreas: error, Parsing of the top area coordinate failed!");
            ret = -EINVAL;
            break;
        } else {
            top = static_cast<ssize_t>(strtol(pEnd + 1, &pEnd, 10));
        }

        if (sep != *pEnd) {
            ALOGE("parseAreas: error, Parsing of the right area coordinate failed!");
            ret = -EINVAL;
            break;
        } else {
            right = static_cast<ssize_t>(strtol(pEnd + 1, &pEnd, 10));
        }

        if (sep != *pEnd) {
            ALOGE("parseAreas: error, Parsing of the bottom area coordinate failed!");
            ret = -EINVAL;
            break;
        } else {
            bottom = static_cast<ssize_t>(strtol(pEnd + 1, &pEnd, 10));
        }

        if (sep != *pEnd) {
            ALOGE("parseAreas: error, Parsing of the weight area coordinate failed!");
            ret = -EINVAL;
            break;
        } else {
            weight = static_cast<ssize_t>(strtol(pEnd + 1, &pEnd, 10));
        }

        if (endToken != *pEnd) {
            ALOGE("parseAreas: error, malformed area!");
            ret = -EINVAL;
            break;
        }

        ret = checkArea(top, left, bottom, right, weight);
        if (NO_ERROR != ret)
            break;

        /*
        ALOGV("parseAreas: Area parsed [%dx%d, %dx%d] %d",
                ( int ) left,
                ( int ) top,
                ( int ) right,
                ( int ) bottom,
                ( int ) weight);
         */

        pArea = strtok_r(NULL, startToken, &ctx);

        focusArea->left = (int)left;
        focusArea->top = (int)top;
        focusArea->right = (int)right;
        focusArea->bottom = (int)bottom;
        focusArea->weight = (int)weight;
        (*num_areas)++;
    } while ( NULL != pArea );

    if (NULL != tmpBuffer)
        free(tmpBuffer);

    return ret;
}

/* TODO : muliple focus area is not supported yet */
status_t ISecCameraHardware::setFocusAreas(const CameraParameters &params)
{
    if (!IsAutoFocusSupported())
        return NO_ERROR;

    const char *str = params.get(CameraParameters::KEY_FOCUS_AREAS);
    const char *prevStr = mParameters.get(CameraParameters::KEY_FOCUS_AREAS);
    if ((str == NULL) || (prevStr && !strcmp(str, prevStr)))
        return NO_ERROR;

    struct FocusArea focusArea;
    struct FocusPoint center;
    int err, num_areas = 0;
    const char *maxFocusAreasStr = params.get(CameraParameters::KEY_MAX_NUM_FOCUS_AREAS);
    if (!maxFocusAreasStr) {
        ALOGE("setFocusAreas: error, KEY_MAX_NUM_FOCUS_AREAS is NULL");
        return NO_ERROR;
    }

    int maxFocusAreas = atoi(maxFocusAreasStr);
    if (!maxFocusAreas) {
        ALOGD("setFocusAreas: FocusAreas is not supported");
        return NO_ERROR;
    }

    /* Focus area parse here */
    err = parseAreas(str, (strlen(str) + 1), &focusArea, &num_areas);
    if (CC_UNLIKELY(err < 0)) {
        ALOGE("setFocusAreas: error, parseAreas %s", str);
        return BAD_VALUE;
    }
    if (CC_UNLIKELY(num_areas > maxFocusAreas)) {
        ALOGE("setFocusAreas: error, the number of areas is more than max");
        return BAD_VALUE;
    }

    /* find center point */
    err = findCenter(&focusArea, &center);
    if (CC_UNLIKELY(err < 0)) {
        ALOGE("setFocusAreas: error, findCenter");
        return BAD_VALUE;
    }

    /* Normalization */
    err = normalizeArea(&center);
    if (err < 0) {
        ALOGE("setFocusAreas: error, normalizeArea");
        return BAD_VALUE;
    }

    ALOGV("setFocusAreas: FocusAreas(%s) to (%d, %d)", str, center.x, center.y);

    mParameters.set(CameraParameters::KEY_FOCUS_AREAS, str);

#ifdef ENABLE_TOUCH_AF
    err = nativeSetParameters(CAM_CID_SET_TOUCH_AF_POSX, center.x);
    if (CC_UNLIKELY(err < 0)) {
        ALOGE("setFocusAreas: error, SET_TOUCH_AF_POSX");
        return UNKNOWN_ERROR;
    }

    err = nativeSetParameters(CAM_CID_SET_TOUCH_AF_POSY, center.y);
    if (CC_UNLIKELY(err < 0)) {
        ALOGE("setFocusAreas: error, SET_TOUCH_AF_POSX");
        return UNKNOWN_ERROR;
    }

    return nativeSetParameters(CAM_CID_SET_TOUCH_AF, 1);
#endif

    return NO_ERROR;
}

/* -------------------Focus Area ENDS here---------------------------- */

status_t ISecCameraHardware::setIso(const CameraParameters &params)
{
    const char *str = params.get(SecCameraParameters::KEY_ISO);
    const char *prevStr = mParameters.get(SecCameraParameters::KEY_ISO);
    if (str == NULL || (prevStr && !strcmp(str, prevStr)))
        return NO_ERROR;
    if (prevStr == NULL && !strcmp(str, isos[0].desc))  /* default */
        return NO_ERROR;

    int val;
retry:
    val = SecCameraParameters::lookupAttr(isos, ARRAY_SIZE(isos), str);
    if (CC_UNLIKELY(val == NOT_FOUND)) {
        ALOGW("setIso: warning, not supported value(%s)", str);
        /* str = reinterpret_cast<const char*>(isos[0].desc);
        goto retry;
         */
        return BAD_VALUE;
    }

    ALOGV("setIso: %s", str);
    mParameters.set(SecCameraParameters::KEY_ISO, str);

    return nativeSetParameters(CAM_CID_ISO, val);
}

status_t ISecCameraHardware::setBrightness(const CameraParameters &params)
{
    int val;
    if (CC_LIKELY(mSceneMode == SCENE_MODE_NONE)) {
        val = params.getInt(CameraParameters::KEY_EXPOSURE_COMPENSATION);
    } else {
        switch (mSceneMode) {
        case SCENE_MODE_BEACH_SNOW:
            val = 2;
            break;

        default:
            val = 0;
            break;
        }
    }

    int prevVal = mParameters.getInt(CameraParameters::KEY_EXPOSURE_COMPENSATION);
    int max = mParameters.getInt(CameraParameters::KEY_MAX_EXPOSURE_COMPENSATION);
    int min = mParameters.getInt(CameraParameters::KEY_MIN_EXPOSURE_COMPENSATION);
    if (prevVal == val)
        return NO_ERROR;

    if (CC_UNLIKELY(val < min || val > max)) {
        ALOGE("setBrightness: error, invalid value(%d)", val);
        return BAD_VALUE;
    }

    ALOGV("setBrightness: %d", val);
    mParameters.set(CameraParameters::KEY_EXPOSURE_COMPENSATION, val);

    return nativeSetParameters(CAM_CID_BRIGHTNESS, val);
}

status_t ISecCameraHardware::setWhiteBalance(const CameraParameters &params)
{
    const char *str;
    if (mSamsungApp) {
        if (CC_LIKELY(mSceneMode == SCENE_MODE_NONE)) {
            str = params.get(CameraParameters::KEY_WHITE_BALANCE);
        } else {
            switch (mSceneMode) {
            case SCENE_MODE_SUNSET:
            case SCENE_MODE_CANDLE_LIGHT:
                str = CameraParameters::WHITE_BALANCE_DAYLIGHT;
                break;

            case SCENE_MODE_DUSK_DAWN:
                str = CameraParameters::WHITE_BALANCE_FLUORESCENT;
                break;

            default:
                str = CameraParameters::WHITE_BALANCE_AUTO;
                break;
            }
        }
    } else
        str = params.get(CameraParameters::KEY_WHITE_BALANCE);

    const char *prevStr = mParameters.get(CameraParameters::KEY_WHITE_BALANCE);
    if (str == NULL || (prevStr && !strcmp(str, prevStr)))
        return NO_ERROR;

    int val;
retry:
    val = SecCameraParameters::lookupAttr(whiteBalances, ARRAY_SIZE(whiteBalances), str);
    if (CC_UNLIKELY(val == NOT_FOUND)) {
        ALOGW("setWhiteBalance: warning, not supported value(%s)", str);
        str = reinterpret_cast<const char*>(whiteBalances[0].desc);
        goto retry;
    }

    ALOGV("setWhiteBalance: %s", str);
    mParameters.set(CameraParameters::KEY_WHITE_BALANCE, str);

    return nativeSetParameters(CAM_CID_WHITE_BALANCE, val);
}

status_t ISecCameraHardware::setFlash(const CameraParameters &params)
{
    if (!IsFlashSupported())
        return NO_ERROR;

    const char *str;
    if (mSamsungApp) {
        if (CC_LIKELY(mSceneMode == SCENE_MODE_NONE)) {
            str = params.get(CameraParameters::KEY_FLASH_MODE);
        } else {
            switch (mSceneMode) {
            case SCENE_MODE_PORTRAIT:
            case SCENE_MODE_PARTY_INDOOR:
            case SCENE_MODE_BACK_LIGHT:
            case SCENE_MODE_TEXT:
                str = params.get(CameraParameters::KEY_FLASH_MODE);
                break;

            default:
                str = CameraParameters::FLASH_MODE_OFF;
                break;
            }
        }
    } else {
        str = params.get(CameraParameters::KEY_FLASH_MODE);
    }

    const char *prevStr = mParameters.get(CameraParameters::KEY_FLASH_MODE);
    if (str == NULL || (prevStr && !strcmp(str, prevStr)))
        return NO_ERROR;

    int val;
retry:
    val = SecCameraParameters::lookupAttr(flashModes, ARRAY_SIZE(flashModes), str);
    if (CC_UNLIKELY(val == NOT_FOUND)) {
        ALOGW("setFlash: warning, not supported value(%s)", str);
        return BAD_VALUE; /* return BAD_VALUE if invalid parameter */
    }

    ALOGV("setFlash: %s", str);
    mFlashMode = (cam_flash_mode)val;
    mParameters.set(CameraParameters::KEY_FLASH_MODE, str);

    return nativeSetParameters(CAM_CID_FLASH, val);
}

status_t ISecCameraHardware::setMetering(const CameraParameters &params)
{
    const char *str;
    if (CC_LIKELY(mSceneMode == SCENE_MODE_NONE)) {
        str = params.get(SecCameraParameters::KEY_METERING);
    } else {
        switch (mSceneMode) {
        case SCENE_MODE_LANDSCAPE:
            str = SecCameraParameters::METERING_MATRIX;
            break;

        case SCENE_MODE_BACK_LIGHT:
            if (mFlashMode == FLASH_MODE_OFF)
                str = SecCameraParameters::METERING_SPOT;
            else
                str = SecCameraParameters::METERING_CENTER;
            break;

        default:
            str = SecCameraParameters::METERING_CENTER;
            break;
        }
    }

    const char *prevStr = mParameters.get(SecCameraParameters::KEY_METERING);
    if (str == NULL || (prevStr && !strcmp(str, prevStr)))
        return NO_ERROR;
    if (prevStr == NULL && !strcmp(str, meterings[0].desc)) /* default */
        return NO_ERROR;

    int val;
retry:
    val = SecCameraParameters::lookupAttr(meterings, ARRAY_SIZE(meterings), str);
    if (CC_UNLIKELY(val == NOT_FOUND)) {
        ALOGW("setMetering: warning, not supported value(%s)", str);
        str = reinterpret_cast<const char*>(meterings[0].desc);
        goto retry;
    }

    ALOGV("setMetering: %s", str);
    mParameters.set(SecCameraParameters::KEY_METERING, str);

    return nativeSetParameters(CAM_CID_METERING, val);
}

status_t ISecCameraHardware::setFocusMode(const CameraParameters &params)
{
    const char *str = params.get(CameraParameters::KEY_FOCUS_MODE);
    const char *prevStr = mParameters.get(CameraParameters::KEY_FOCUS_MODE);
    if (str == NULL || (prevStr && !strcmp(str, prevStr)))
        return NO_ERROR;

    int count, val;
    const cam_strmap_t *focusModes;

    if (mCameraId == CAMERA_FACING_BACK) {
        count = ARRAY_SIZE(backFocusModes);
        focusModes = backFocusModes;
    } else {
        count = ARRAY_SIZE(frontFocusModes);
        focusModes = frontFocusModes;
    }

retry:
    val = SecCameraParameters::lookupAttr(focusModes, count, str);
    if (CC_UNLIKELY(val == NOT_FOUND)) {
        ALOGW("setFocusMode: warning, not supported value(%s)", str);
        return BAD_VALUE; /* return BAD_VALUE if invalid parameter */
    }

    ALOGV("setFocusMode: %s", str);
    mFocusMode = (cam_focus_mode)val;
    mParameters.set(CameraParameters::KEY_FOCUS_MODE, str);
    if (val == FOCUS_MODE_MACRO) {
        mParameters.set(CameraParameters::KEY_FOCUS_DISTANCES,
                B_KEY_MACRO_FOCUS_DISTANCES_VALUE);
    } else {
        mParameters.set(CameraParameters::KEY_FOCUS_DISTANCES,
                B_KEY_NORMAL_FOCUS_DISTANCES_VALUE);
    }

    return nativeSetParameters(CAM_CID_FOCUS_MODE, val);
}

status_t ISecCameraHardware::setEffect(const CameraParameters &params)
{
    const char *str = params.get(CameraParameters::KEY_EFFECT);
    const char *prevStr = mParameters.get(CameraParameters::KEY_EFFECT);
    if (str == NULL || (prevStr && !strcmp(str, prevStr)))
        return NO_ERROR;

    int val;
retry:
    val = SecCameraParameters::lookupAttr(effects, ARRAY_SIZE(effects), str);
    if (CC_UNLIKELY(val == NOT_FOUND)) {
        ALOGW("setEffect: warning, not supported value(%s)", str);
        str = reinterpret_cast<const char*>(effects[0].desc);
        goto retry;
    }

    ALOGV("setEffect: %s", str);
    mParameters.set(CameraParameters::KEY_EFFECT, str);

    return nativeSetParameters(CAM_CID_EFFECT, val);
}

status_t ISecCameraHardware::setZoom(const CameraParameters &params)
{
    if (!mZoomSupport)
        return NO_ERROR;

    int val = params.getInt(CameraParameters::KEY_ZOOM);
    int prevVal = mParameters.getInt(CameraParameters::KEY_ZOOM);
    if (val == -1 || prevVal == val)
        return NO_ERROR;

    int max = params.getInt(CameraParameters::KEY_MAX_ZOOM);
    if (CC_UNLIKELY(val < 0 || val > max)) {
        ALOGE("setZoom: error, invalid value(%d)", val);
        return BAD_VALUE;
    }

    ALOGV("setZoom: %d", val);
    mParameters.set(CameraParameters::KEY_ZOOM, val);

    if (mEnableDZoom)
        /* Set AP zoom ratio */
        return nativeSetZoomRatio(val);
    else
        /* Set ISP/sensor zoom ratio */
        return nativeSetParameters(CAM_CID_ZOOM, val);

}

status_t ISecCameraHardware::setAutoContrast(const CameraParameters &params)
{
    int val = params.getInt(SecCameraParameters::KEY_AUTO_CONTRAST);
    int prevVal = mParameters.getInt(SecCameraParameters::KEY_AUTO_CONTRAST);
    if (val == -1 || prevVal == val)
        return NO_ERROR;
    if (prevVal == -1 && val == 0)  /* default */
        return NO_ERROR;

    ALOGV("setAutoContrast: %d", val);
    mParameters.set(SecCameraParameters::KEY_AUTO_CONTRAST, val);

    return nativeSetParameters(CAM_CID_AUTO_CONTRAST, val);
}

status_t ISecCameraHardware::setAntiShake(const CameraParameters &params)
{
    int val = params.getInt(SecCameraParameters::KEY_ANTI_SHAKE);
    int prevVal = mParameters.getInt(SecCameraParameters::KEY_ANTI_SHAKE);
    if (val == -1 || prevVal == val)
        return NO_ERROR;
    if (prevVal == -1 && val == 0)  /* default */
        return NO_ERROR;

    ALOGV("setAntiShake: %d", val);
    mParameters.set(SecCameraParameters::KEY_ANTI_SHAKE, val);

    return nativeSetParameters(CAM_CID_ANTISHAKE, val);
}

status_t ISecCameraHardware::setFaceBeauty(const CameraParameters &params)
{
    int val = params.getInt(SecCameraParameters::KEY_FACE_BEAUTY);
    int prevVal = mParameters.getInt(SecCameraParameters::KEY_FACE_BEAUTY);
    if (val == -1 || prevVal == val)
        return NO_ERROR;
    if (prevVal == -1 && val == 0)  /* default */
        return NO_ERROR;

    ALOGV("setFaceBeauty: %d", val);
    mParameters.set(SecCameraParameters::KEY_FACE_BEAUTY, val);

    return nativeSetParameters(CAM_CID_FACE_BEAUTY, val);
}

status_t ISecCameraHardware::setBlur(const CameraParameters &params)
{
    int val = params.getInt(SecCameraParameters::KEY_BLUR);
    int prevVal = mParameters.getInt(SecCameraParameters::KEY_BLUR);
    if (val == -1 || prevVal == val)
        return NO_ERROR;
    if (prevVal == -1 && val == 0)  /* default */
        return NO_ERROR;

    ALOGV("setBlur: %d", val);
    mParameters.set(SecCameraParameters::KEY_BLUR, val);
    if (val > 0)
        setDropFrame(2);

    return nativeSetParameters(CAM_CID_BLUR, val);
}

static void getAntiBandingFromLatinMCC(char *temp_str, int tempStrLength)
{
    char value[10];
    char country_value[10];

    if (temp_str == NULL) {
        ALOGE("ERR(%s) (%d): temp_str is null", __FUNCTION__, __LINE__);
        return;
    }

    memset(value, 0x00, sizeof(value));
    memset(country_value, 0x00, sizeof(country_value));
    if (!property_get("gsm.operator.numeric", value,"")) {
        strncpy(temp_str, CameraParameters::ANTIBANDING_60HZ, tempStrLength);
        return;
    }
    memcpy(country_value, value, 3);

    /** MCC Info. Jamaica : 338 / Argentina : 722 / Chile : 730 / Paraguay : 744 / Uruguay : 748  **/
    if (strstr(country_value,"338") || strstr(country_value,"722") ||
        strstr(country_value,"730") || strstr(country_value,"744") ||
        strstr(country_value,"748"))
        strncpy(temp_str, CameraParameters::ANTIBANDING_50HZ, tempStrLength);
    else
        strncpy(temp_str, CameraParameters::ANTIBANDING_60HZ, tempStrLength);
}

static int IsLatinOpenCSC()
{
    char sales_code[5] = {0};
    property_get("ro.csc.sales_code", sales_code, "");
    if (strstr(sales_code,"TFG") || strstr(sales_code,"TPA") || strstr(sales_code,"TTT") || strstr(sales_code,"JDI") || strstr(sales_code,"PCI") )
        return 1;
    else
        return 0;
}

void ISecCameraHardware::chooseAntiBandingFrequency()
{
#if NOTDEFINED
    status_t ret = NO_ERROR;
    int LatinOpenCSClength = 5;
    char *LatinOpenCSCstr = NULL;
    char *CSCstr = NULL;
    const char *defaultStr = "50hz";

    if (IsLatinOpenCSC()) {
        LatinOpenCSCstr = (char *)malloc(LatinOpenCSClength);
        if (LatinOpenCSCstr == NULL) {
            ALOGE("LatinOpenCSCstr is NULL");
            CSCstr = (char *)defaultStr;
            memset(mAntiBanding, 0, sizeof(mAntiBanding));
            strcpy(mAntiBanding, CSCstr);
            return;
        }
        memset(LatinOpenCSCstr, 0, LatinOpenCSClength);

        getAntiBandingFromLatinMCC(LatinOpenCSCstr, LatinOpenCSClength);
        CSCstr = LatinOpenCSCstr;
    } else {
        CSCstr = (char *)SecNativeFeature::getInstance()->getString("CscFeature_Camera_CameraFlicker");
    }

    if (CSCstr == NULL || strlen(CSCstr) == 0) {
        CSCstr = (char *)defaultStr;
    }
    memset(mAntiBanding, 0, sizeof(mAntiBanding));
    strcpy(mAntiBanding, CSCstr);
    ALOGV("mAntiBanding = %s",mAntiBanding);

    if (LatinOpenCSCstr != NULL)
       free(LatinOpenCSCstr);
#endif
}

status_t ISecCameraHardware::setAntiBanding()
{
    status_t ret = NO_ERROR;
    const char *prevStr = mParameters.get(CameraParameters::KEY_ANTIBANDING);

    if (prevStr && !strcmp(mAntiBanding, prevStr))
        return NO_ERROR;

retry:
    int val = SecCameraParameters::lookupAttr(antibandings, ARRAY_SIZE(antibandings), mAntiBanding);
    if (CC_UNLIKELY(val == NOT_FOUND)) {
        ALOGE("setAntiBanding: error, not supported value(%s)", mAntiBanding);
        return BAD_VALUE;
    }
    ALOGD("setAntiBanding: %s", mAntiBanding);
    mParameters.set(CameraParameters::KEY_ANTIBANDING, mAntiBanding);
    return nativeSetParameters(CAM_CID_ANTIBANDING, val);
}

status_t ISecCameraHardware::setAntiBanding(const CameraParameters &params)
{
    const char *str = params.get(CameraParameters::KEY_ANTIBANDING);
    const char *prevStr = mParameters.get(CameraParameters::KEY_ANTIBANDING);

    if (str == NULL || (prevStr && !strcmp(str, prevStr)))
        return NO_ERROR;

    int val;

retry:
    val = SecCameraParameters::lookupAttr(antibandings, ARRAY_SIZE(antibandings), str);
    if (CC_UNLIKELY(val == NOT_FOUND)) {
        ALOGW("setAntiBanding: warning, not supported value(%s)", str);
        str = reinterpret_cast<const char*>(antibandings[0].desc);
        goto retry;
    }

    ALOGV("setAntiBanding: %s, val: %d", str, val);
    mParameters.set(CameraParameters::KEY_ANTIBANDING, str);

    return nativeSetParameters(CAM_CID_ANTIBANDING, val);
}

status_t ISecCameraHardware::setGps(const CameraParameters &params)
{
    const char *latitude = params.get(CameraParameters::KEY_GPS_LATITUDE);
    const char *logitude = params.get(CameraParameters::KEY_GPS_LONGITUDE);
    const char *altitude = params.get(CameraParameters::KEY_GPS_ALTITUDE);
    if (latitude && logitude && altitude) {
        ALOGV("setParameters: GPS latitude %f, logitude %f, altitude %f",
             atof(latitude), atof(logitude), atof(altitude));
        mParameters.set(CameraParameters::KEY_GPS_LATITUDE, latitude);
        mParameters.set(CameraParameters::KEY_GPS_LONGITUDE, logitude);
        mParameters.set(CameraParameters::KEY_GPS_ALTITUDE, altitude);
    } else {
        mParameters.remove(CameraParameters::KEY_GPS_LATITUDE);
        mParameters.remove(CameraParameters::KEY_GPS_LONGITUDE);
        mParameters.remove(CameraParameters::KEY_GPS_ALTITUDE);
    }

    const char *timestamp = params.get(CameraParameters::KEY_GPS_TIMESTAMP);
    if (timestamp) {
        ALOGV("setParameters: GPS timestamp %s", timestamp);
        mParameters.set(CameraParameters::KEY_GPS_TIMESTAMP, timestamp);
    } else {
        mParameters.remove(CameraParameters::KEY_GPS_TIMESTAMP);
    }

    const char *progressingMethod = params.get(CameraParameters::KEY_GPS_PROCESSING_METHOD);
    if (progressingMethod) {
        ALOGV("setParameters: GPS timestamp %s", timestamp);
        mParameters.set(CameraParameters::KEY_GPS_PROCESSING_METHOD, progressingMethod);
    } else {
        mParameters.remove(CameraParameters::KEY_GPS_PROCESSING_METHOD);
    }

    return NO_ERROR;
}

#if defined(KOR_CAMERA)
status_t ISecCameraHardware::setSelfTestMode(const CameraParameters &params)
{
    int val = params.getInt("selftestmode");
    if (val == -1)
        return NO_ERROR;

    ALOGV("selftestmode: %d", val);
    mSamsungApp = val ? true : false;

    return NO_ERROR;
}
#endif

bool ISecCameraHardware::allocMemSinglePlane(ion_client ionClient, ExynosBuffer *buf, int index, bool flagCache)
{
    if (ionClient == 0) {
        ALOGE("ERR(%s): ionClient is zero (%d)", __func__, ionClient);
        return false;
    }

    if (buf->size.extS[index] != 0) {
        int flagIon = (flagCache == true) ? ION_FLAG_CACHED : 0;

        /* HACK: For non-cacheable */
        buf->fd.extFd[index] = ion_alloc(ionClient, buf->size.extS[index], 0, ION_HEAP_SYSTEM_MASK, 0);
        if (buf->fd.extFd[index] <= 0) {
            ALOGE("ERR(%s): ion_alloc(%d, %d) failed", __func__, index, buf->size.extS[index]);
            buf->fd.extFd[index] = -1;
            freeMemSinglePlane(buf, index);
            return false;
        }

        buf->virt.extP[index] = (char *)ion_map(buf->fd.extFd[index], buf->size.extS[index], 0);
        if ((buf->virt.extP[index] == (char *)MAP_FAILED) || (buf->virt.extP[index] == NULL)) {
            ALOGE("ERR(%s): ion_map(%d) failed", __func__, buf->size.extS[index]);
            buf->virt.extP[index] = NULL;
            freeMemSinglePlane(buf, index);
            return false;
        }
    }

    return true;
}

void ISecCameraHardware::freeMemSinglePlane(ExynosBuffer *buf, int index)
{
    int ret = 0;

    if (0 < buf->fd.extFd[index]) {
        if (buf->virt.extP[index] != NULL) {
            ret = ion_unmap(buf->virt.extP[index], buf->size.extS[index]);
            if (ret < 0)
                ALOGE("ERR(%s):ion_unmap(%p, %d) fail", __FUNCTION__, buf->virt.extP[index], buf->size.extS[index]);
        }
        ion_free(buf->fd.extFd[index]);
    }

    buf->fd.extFd[index] = -1;
    buf->virt.extP[index] = NULL;
    buf->size.extS[index] = 0;
}

bool ISecCameraHardware::allocMem(ion_client ionClient, ExynosBuffer *buf, int cacheIndex)
{
    for (int i = 0; i < ExynosBuffer::BUFFER_PLANE_NUM_DEFAULT; i++) {
        bool flagCache = ((1 << i) & cacheIndex) ? true : false;
        if (allocMemSinglePlane(ionClient, buf, i, flagCache) == false) {
            freeMem(buf);
            ALOGE("ERR(%s): allocMemSinglePlane(%d) fail", __func__, i);
            return false;
        }
    }

    return true;
}

void ISecCameraHardware::freeMem(ExynosBuffer *buf)
{
    for (int i = 0; i < ExynosBuffer::BUFFER_PLANE_NUM_DEFAULT; i++)
        freeMemSinglePlane(buf, i);
}

void ISecCameraHardware::mInitRecSrcQ(void)
{
    Mutex::Autolock lock(mRecordSrcLock);
    mRecordSrcIndex = -1;

    mRecordSrcQ.clear();
}

int ISecCameraHardware::getRecSrcBufSlotIndex(void)
{
    Mutex::Autolock lock(mRecordSrcLock);
    mRecordSrcIndex++;
    mRecordSrcIndex = mRecordSrcIndex % FLITE_BUF_CNT;
    return mRecordSrcIndex;
}

void ISecCameraHardware::mPushRecSrcQ(rec_src_buf_t *buf)
{
    Mutex::Autolock lock(mRecordSrcLock);
    mRecordSrcQ.push_back(buf);
}

bool ISecCameraHardware::mPopRecSrcQ(rec_src_buf_t *buf)
{
    List<rec_src_buf_t *>::iterator r;

    Mutex::Autolock lock(mRecordSrcLock);

    if (mRecordSrcQ.size() == 0)
        return false;

    r = mRecordSrcQ.begin()++;

    buf->buf = (*r)->buf;
    buf->timestamp = (*r)->timestamp;
    mRecordSrcQ.erase(r);

    return true;
}

int ISecCameraHardware::mSizeOfRecSrcQ(void)
{
    Mutex::Autolock lock(mRecordSrcLock);

    return mRecordSrcQ.size();
}

#if 0
bool ISecCameraHardware::setRecDstBufStatus(int index, enum REC_BUF_STATUS status)
{
    Mutex::Autolock lock(mRecordDstLock);

    if (index < 0 || index >= REC_BUF_CNT) {
        ALOGE("ERR(%s): index(%d) out of range, status(%d)", __func__, index, status);
        return false;
    }

    mRecordDstStatus[index] = status;
    return true;
}
#endif

int ISecCameraHardware::getRecDstBufIndex(void)
{
    Mutex::Autolock lock(mRecordDstLock);

    for (int i = 0; i < REC_BUF_CNT; i++) {
        mRecordDstIndex++;
        mRecordDstIndex = mRecordDstIndex % REC_BUF_CNT;

        if (mRecordFrameAvailable[mRecordDstIndex] == true) {
            mRecordFrameAvailableCnt--;
            mRecordFrameAvailable[mRecordDstIndex] = false;
            return mRecordDstIndex;
        }
    }

    return -1;
}

void ISecCameraHardware::setAvailDstBufIndex(int index)
{
    Mutex::Autolock lock(mRecordDstLock);
    mRecordFrameAvailableCnt++;
    mRecordFrameAvailable[index] = true;
    return;
}

void ISecCameraHardware::mInitRecDstBuf(void)
{
    Mutex::Autolock lock(mRecordDstLock);

    mRecordDstIndex = -1;
    mRecordFrameAvailableCnt = REC_BUF_CNT;

    for (int i = 0; i < REC_BUF_CNT; i++) {
        for (int j = 0; j < REC_PLANE_CNT; j++) {
            if (mRecordDstHeap[i][j] != NULL)
                mRecordDstHeap[i][j]->release(mRecordDstHeap[i][j]);
            mRecordDstHeap[i][j] = NULL;
            mRecordDstHeapFd[i][j] = -1;
        }
        mRecordFrameAvailable[i] = true;
    }
}

int ISecCameraHardware::getAlignedYUVSize(int colorFormat, int w, int h, ExynosBuffer *buf, bool flagAndroidColorFormat)
{
    int FrameSize = 0;
    ExynosBuffer alignedBuf;

    /* ALOGV("[%s] (%d) colorFormat %d", __func__, __LINE__, colorFormat); */
    switch (colorFormat) {
    /* 1p */
    case V4L2_PIX_FMT_RGB565 :
    case V4L2_PIX_FMT_YUYV :
    case V4L2_PIX_FMT_UYVY :
    case V4L2_PIX_FMT_VYUY :
    case V4L2_PIX_FMT_YVYU :
        alignedBuf.size.extS[0] = FRAME_SIZE(V4L2_PIX_2_HAL_PIXEL_FORMAT(colorFormat), w, h);
        /* ALOGV("V4L2_PIX_FMT_YUYV buf->size.extS[0] %d", alignedBuf->size.extS[0]); */
        alignedBuf.size.extS[1] = 0;
        alignedBuf.size.extS[2] = 0;
        break;
    /* 2p */
    case V4L2_PIX_FMT_NV12 :
    case V4L2_PIX_FMT_NV12T :
    case V4L2_PIX_FMT_NV21 :
    case V4L2_PIX_FMT_NV12M :
    case V4L2_PIX_FMT_NV21M :
        if (flagAndroidColorFormat == true) {
            alignedBuf.size.extS[0] = w * h;
            alignedBuf.size.extS[1] = w * h / 2;
            alignedBuf.size.extS[2] = 0;
        } else {
            alignedBuf.size.extS[0] = ALIGN_UP(w, 16) * ALIGN_UP(h, 16);
            alignedBuf.size.extS[1] = ALIGN_UP(w, 16) * ALIGN_UP(h, 16) / 2;
            alignedBuf.size.extS[2] = 0;
        }
        /* ALOGV("V4L2_PIX_FMT_NV21 buf->size.extS[0] %d buf->size.extS[1] %d",
            alignedBuf->size.extS[0], alignedBuf->size.extS[1]); */
        break;
    case V4L2_PIX_FMT_NV12MT_16X16 :
        if (flagAndroidColorFormat == true) {
            alignedBuf.size.extS[0] = w * h;
            alignedBuf.size.extS[1] = w * h / 2;
            alignedBuf.size.extS[2] = 0;
        } else {
            alignedBuf.size.extS[0] = ALIGN_UP(w, 16) * ALIGN_UP(h, 16);
            alignedBuf.size.extS[1] = ALIGN(alignedBuf.size.extS[0] / 2, 256);
            alignedBuf.size.extS[2] = 0;
        }
        /* ALOGV("V4L2_PIX_FMT_NV12M buf->size.extS[0] %d buf->size.extS[1] %d",
            alignedBuf->size.extS[0], alignedBuf->size.extS[1]); */
        break;
    case V4L2_PIX_FMT_NV16 :
    case V4L2_PIX_FMT_NV61 :
        alignedBuf.size.extS[0] = ALIGN_UP(w, 16) * ALIGN_UP(h, 16);
        alignedBuf.size.extS[1] = ALIGN_UP(w, 16) * ALIGN_UP(h, 16);
        alignedBuf.size.extS[2] = 0;
        /* ALOGV("V4L2_PIX_FMT_NV16 buf->size.extS[0] %d buf->size.extS[1] %d",
            alignedBuf->size.extS[0], alignedBuf->size.extS[1]); */
        break;
    /* 3p */
    case V4L2_PIX_FMT_YUV420 :
    case V4L2_PIX_FMT_YVU420 :
        /* http://developer.android.com/reference/android/graphics/ImageFormat.html#YV12 */
        if (flagAndroidColorFormat == true) {
            alignedBuf.size.extS[0] = ALIGN_UP(w, 16) * h;
            alignedBuf.size.extS[1] = ALIGN_UP(w / 2, 16) * h / 2;
            alignedBuf.size.extS[2] = ALIGN_UP(w / 2, 16) * h / 2;
        } else {
            alignedBuf.size.extS[0] = (w * h);
            alignedBuf.size.extS[1] = (w * h) >> 2;
            alignedBuf.size.extS[2] = (w * h) >> 2;
        }
        /* ALOGV("V4L2_PIX_FMT_YUV420 Buf.size.extS[0] %d Buf.size.extS[1] %d Buf.size.extS[2] %d",
            alignedBuf.size.extS[0], alignedBuf.size.extS[1], alignedBuf.size.extS[2]); */
        break;
    case V4L2_PIX_FMT_YUV420M :
    case V4L2_PIX_FMT_YVU420M :
        if (flagAndroidColorFormat == true) {
            alignedBuf.size.extS[0] = ALIGN_UP(w, 16) * h;
            alignedBuf.size.extS[1] = ALIGN_UP(w / 2, 16) * h / 2;
            alignedBuf.size.extS[2] = ALIGN_UP(w / 2, 16) * h / 2;
        } else {
            alignedBuf.size.extS[0] = ALIGN_UP(w,   32) * ALIGN_UP(h,  16);
            alignedBuf.size.extS[1] = ALIGN_UP(w/2, 16) * ALIGN_UP(h/2, 8);
            alignedBuf.size.extS[2] = ALIGN_UP(w/2, 16) * ALIGN_UP(h/2, 8);
        }
        /* ALOGV("V4L2_PIX_FMT_YUV420M buf->size.extS[0] %d buf->size.extS[1] %d buf->size.extS[2] %d",
            alignedBuf->size.extS[0], alignedBuf->size.extS[1], alignedBuf->size.extS[2]); */
        break;
    case V4L2_PIX_FMT_YUV422P :
        alignedBuf.size.extS[0] = ALIGN_UP(w,   16) * ALIGN_UP(h,  16);
        alignedBuf.size.extS[1] = ALIGN_UP(w/2, 16) * ALIGN_UP(h/2, 8);
        alignedBuf.size.extS[2] = ALIGN_UP(w/2, 16) * ALIGN_UP(h/2, 8);
        /* ALOGV("V4L2_PIX_FMT_YUV422P Buf.size.extS[0] %d Buf.size.extS[1] %d Buf.size.extS[2] %d",
            alignedBuf.size.extS[0], alignedBuf.size.extS[1], alignedBuf.size.extS[2]); */
        break;
    default:
        ALOGE("ERR(%s):unmatched colorFormat(%d)", __func__, colorFormat);
        return 0;
        break;
    }

    for (int i = 0; i < ExynosBuffer::BUFFER_PLANE_NUM_DEFAULT; i++)
        FrameSize += alignedBuf.size.extS[i];

    if (buf != NULL) {
        for (int i = 0; i < ExynosBuffer::BUFFER_PLANE_NUM_DEFAULT; i++) {
            buf->size.extS[i] = alignedBuf.size.extS[i];

            /* if buf has vadr, calculate another vadr per plane */
            if (buf->virt.extP[0] != NULL && i > 0) {
                if (buf->size.extS[i] != 0)
                    buf->virt.extP[i] = buf->virt.extP[i - 1] + buf->size.extS[i - 1];
                else
                    buf->virt.extP[i] = NULL;
            }
        }
    }

    return FrameSize;
}

}; /* namespace android */
#endif /* ANDROID_HARDWARE_ISECCAMERAHARDWARE_CPP */
