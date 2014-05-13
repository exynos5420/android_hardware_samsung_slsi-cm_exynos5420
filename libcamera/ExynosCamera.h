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
** distributed under the License is distributed toggle an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

/*!
 * \file      ExynosCamera.h
 * \brief     hearder file for CAMERA HAL MODULE
 * \author    thun.hwang(thun.hwang@samsung.com)
 * \date      2010/06/03
 *
 * <b>Revision History: </b>
 * - 2011/12/31 : thun.hwang(thun.hwang@samsung.com) \n
 *   Initial version
 *
 * - 2012/01/18 : Sangwoo, Park(sw5771.park@samsung.com) \n
 *   Adjust Doxygen Document
 *
 * - 2012/02/01 : Sangwoo, Park(sw5771.park@samsung.com) \n
 *   Adjust libv4l2
 *   Adjust struct ExynosCameraInfo
 *   External ISP feature
 *
 * - 2012/03/14 : sangwoo.park(sw5771.park@samsung.com) \n
 *   Change file, class name to ExynosXXX.
 *
 * - 2012/08/01 : Pilsun, Jang(pilsun.jang@samsung.com) \n
 *   Adjust Camera2.0 Driver
 */

#ifndef EXYNOS_CAMERA_H__
#define EXYNOS_CAMERA_H__

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

#include "ExynosCameraActivityFlash.h"
#include "ExynosCameraActivityAutofocus.h"
#include "ExynosCameraActivitySpecialCapture.h"

using namespace android;

#define FU_3INSTANCE
//#define DUMP_BAYER_IMAGE

#define CAPTURE_BUF_GET 1

#define TRACE_COUNT 10

#define SCALABLE_SENSOR
#define SCALABLE_SENSOR_FORCE_DONE
#define SCALABLE_SENSOR_CHKTIME
#define SCALABLE_SENSOR_CTRL_ISP
#define OTF_SENSOR_REPROCESSING

/* #define BAYER_TRACKING */
#define DYNAMIC_BAYER_BACK_REC
#define NO_BAYER_SENSOR_Q_NUM 3

#define FRONT_NO_ZSL
#define FORCE_LEADER_OFF
/* #define USE_CAMERA_ESD_RESET */
#define FD_ROTATION

/* FIXME: This define will be removed when functions are stable */
//#define USE_3DNR
//#define USE_ODC
//#define USE_VDIS
#define USE_FOR_DTP

#define FORCE_DONE
//#define THREAD_PROFILE

#define XPaste(s) s
#define Paste2(a, b) XPaste(a)b
#define CAM0 "[CAM-BACK]-"
#define CAM1 "[CAM-FRONT]-"

#define CLOGV(fmt, ...) \
    (m_cameraId == 0) \
    ? ALOGV(Paste2(CAM0, fmt), ##__VA_ARGS__) \
    : ALOGV(Paste2(CAM1, fmt), ##__VA_ARGS__)

#define CLOGD(fmt, ...) \
    (m_cameraId == 0) \
    ? ALOGD(Paste2(CAM0, fmt), ##__VA_ARGS__) \
    : ALOGD(Paste2(CAM1, fmt), ##__VA_ARGS__)

#define CLOGW(fmt, ...) \
    (m_cameraId == 0) \
    ? ALOGW(Paste2(CAM0, fmt), ##__VA_ARGS__) \
    : ALOGW(Paste2(CAM1, fmt), ##__VA_ARGS__)

#define CLOGE(fmt, ...) \
    (m_cameraId == 0) \
    ? ALOGE(Paste2(CAM0, fmt), ##__VA_ARGS__) \
    : ALOGE(Paste2(CAM1, fmt), ##__VA_ARGS__)

#define CLOGI(fmt, ...) \
    (m_cameraId == 0) \
    ? ALOGI(Paste2(CAM0, fmt), ##__VA_ARGS__) \
    : ALOGI(Paste2(CAM1, fmt), ##__VA_ARGS__)

#define CLOGT(cnt, fmt, ...) \
    if (cnt != 0) CLOGI(Paste2("#TRACE#", fmt), ##__VA_ARGS__) \

#define CLOG_ASSERT(fmt, ...) \
    android_printAssert(NULL, LOG_TAG, Paste2(CAM0, fmt), ##__VA_ARGS__);

#define BACK_CAMERA_SENSOR_NAME  (SENSOR_NAME_IMX135)
#define FRONT_CAMERA_SENSOR_NAME (SENSOR_NAME_S5K6B2)

#define NUM_BAYER_BUFFERS           (6)
#define META_DATA_SIZE              (16 *1024)
#define NUM_PREVIEW_BUFFERS         (8)
#define NUM_PICTURE_BUFFERS         (4)
#define NUM_MIN_SENSOR_QBUF         (NUM_BAYER_BUFFERS)

#define NUM_BAYER_PLANE             (2)
#define NUM_CAPTURE_PLANE           (2)
#define NUM_MAX_PLANE               (4)

#define FRAME_COUNT_INDEX           (1)

#define SIZE_ISP_WIDTH              (1440)
#define SIZE_ISP_HEIGHT             (1072)

#define SIZE_OTF_WIDTH              (64)
#define SIZE_OTF_HEIGHT             (32)

#define REPROCESSING_FLAG           0x80000000
#define REPROCESSING_MASK           0xFF000000
#define REPROCESSING_SHFIT          24
#define SENSOR_INDEX_MASK           0x00FF0000
#define SENSOR_INDEX_SHFIT          16
#define VIDEO_INDEX_MASK            0x0000FF00
#define VIDEO_INDEX_SHFIT           8
#define MODULE_MASK                 0x000000FF

#define NODE_PREFIX                 "/dev/video"

#define CAMERA_ISP_ALIGN            (4)
#define CAMERA_MAGIC_ALIGN          (16)

#define UNIQUE_ID_BUF_SIZE          (32)

#define CAMERA_Q_WATING_TIME          (1000)   /*   1msec */
#define CAMERA_Q_TATOL_WATING_TIME  (300000)   /* 300msec */

#define ROUND_OFF(x, dig)           (floor((x) * pow(10, dig)) / pow(10, dig))
#define ROUND_OFF_HALF(x, dig)      (floor((x) * pow(10.0f, dig) + 0.5) / pow(10.0f, dig))

enum fimc_is_video_dev_num {
    FIMC_IS_VIDEO_SEN0_NUM = 40,
    FIMC_IS_VIDEO_SEN1_NUM,
    FIMC_IS_VIDEO_3A0_NUM,
    FIMC_IS_VIDEO_3A1_NUM,
    FIMC_IS_VIDEO_ISP_NUM,
    FIMC_IS_VIDEO_SCC_NUM,
    FIMC_IS_VIDEO_SCP_NUM,
    FIMC_IS_VIDEO_VDISC_NUM,
    FIMC_IS_VIDEO_VDISO_NUM,
    FIMC_IS_VIDEO_MAX_NUM,
};

enum sensor_name {
    SENSOR_NAME_NOTHING=0,
    SENSOR_NAME_S5K3H2=1,
    SENSOR_NAME_S5K6A3,
    SENSOR_NAME_S5K4E5,
    SENSOR_NAME_S5K3H5,
    SENSOR_NAME_S5K3H7,
    SENSOR_NAME_S5K3H7_SUNNY,
    SENSOR_NAME_S5K3H7_SUNNY_2M,
    SENSOR_NAME_IMX135,
    SENSOR_NAME_S5K6B2,
    SENSOR_NAME_FAKE_S5K6A3 = 90,
    SENSOR_NAME_FAKE_S5K6B2,
    SENSOR_NAME_FAKE_IMX135 = 99,
    SENSOR_NAME_CUSTOM=100,
    SENSOR_NAME_END
};

static int PREVIEW_LIST[][2] =
{
    { 4128, 3096},
    { 4096, 3072},
    { 3200, 2400},
    { 3072, 1728},
    { 2592, 1944},
    { 2592, 1936},
    { 2560, 1920},
    { 2048, 1536},
    { 1920, 1080},
//  { 1446, 1080}, // preview ratio for 2592x1936
/* remove for CTS */
    { 1440, 1080},
    { 1600, 1200},
    { 1392, 1392},
    { 1280,  960},
    { 1280,  720},
    { 1056,  864},
    { 1024,  768},
    {  800,  600},
    {  800,  480},
    {  720,  480},
    {  640,  480},
    {  528,  432},
    {  480,  320},
    {  480,  270},
    {  352,  288},
    {  320,  240},
};

static int PICTURE_LIST[][2] =
{
    { 4128, 3096},
    { 4128, 2322},
    { 4096, 3072},
    { 4096, 2304},
    { 3264, 2448},
    { 3264, 1836},
    { 3200, 2400},
    { 3072, 1728},
    { 2592, 1944},
    { 2592, 1936},
    { 2560, 1920},
    { 2048, 1536},
    { 2048, 1152},
    { 1920, 1080},
    { 1600, 1200},
    { 1440, 1080},
    /* { 1392, 1392}, */ /* remove for CTS */
    { 1280,  960},
    { 1280,  720},
    { 1024,  768},
    {  800,  600},
    {  800,  480},
    {  720,  480},
    {  640,  480},
    /* {  528,  432}, */ /* remove for CTS */
    {  512,  384},
    {  512,  288},
    {  480,  320},
    /* {  352,  288}, */ /* remove for CTS */
    {  320,  240},
    {  320,  180},
// TODO : will be support after enable REPROCESSING
//        {  176,  144}
};

static int VIDEO_LIST[][2] =
{
    { 1920, 1080},
    { 1440, 1080},
    { 1280,  720},
    {  960,  720},
    {  720,  480},
    {  640,  480},
    {  480,  320},
    {  320,  240},
    {  176,  144}
};

static int FPS_RANGE_LIST[][2] =
{
    {   5000,   5000},
    {   7000,   7000},
    {  10000,  10000},
    {  15000,  15000},
    {  20000,  20000},
    {  24000,  24000},
    {  27000,  27000},
    {   4000,  30000},
    {  15000,  30000},
    {  30000,  30000},
    {  30000,  60000},
};

typedef struct node_info {
    int                fd;
    int                width;
    int                height;
    int                format;
    int                planes;
    int                buffers;
    enum v4l2_memory   memory;
    enum v4l2_buf_type type;
    pollfd             events;

    ion_client         ionClient;
    ExynosBuffer       buffer[VIDEO_MAX_FRAME];
    bool               flagQ[VIDEO_MAX_FRAME];
    Mutex              QLock;

    bool               flagDup;

    bool               flagStart;
} node_info_t;

#ifdef SCALABLE_SENSOR
enum SCALABLE_SENSOR_SIZE {
    SCALABLE_SENSOR_SIZE_13M=1,
    SCALABLE_SENSOR_SIZE_FHD,
};
#endif

typedef struct camera_hw_info {
    node_info_t sensor;
    node_info_t is3a0Src;
    node_info_t is3a0Dst;
    node_info_t is3a1Src;
    node_info_t is3a1Dst;
    node_info_t isp;
    node_info_t picture;
    node_info_t preview;
    node_info_t video;
#ifdef USE_VDIS
    node_info_t vdisc;
    node_info_t vdiso;
#endif

    /*shot*/  // default using reset shot / output from driver / input to driver
    struct camera2_shot_ext dummy_shot;
    struct camera2_shot_ext is3aa_dm;
    struct camera2_shot_ext isp_dm;
    struct camera2_shot_ext default_shot;
} camera_hw_info_t;

//! struct for Camera sensor information
/*!
 * \ingroup Exynos
 */
struct ExynosCameraInfo
{
public:
    // Google Official API : Camera.Parameters
    // http://developer.android.com/reference/android/hardware/Camera.Parameters.html
    int  previewW;
    int  previewH;
    int  previewColorFormat;
    int  previewBufPlane;
    int  videoW;
    int  videoH;
    int  videoColorFormat;
    int  videoBufPlane;
    int  pictureW;
    int  pictureH;
    int  pictureColorFormat;
    int  pictureBufPlane;
    int  thumbnailW;
    int  thumbnailH;

    int  ispW;
    int  ispH;

    int  antiBandingList;
    int  antiBanding;

    int  effectList;
    int  effect;

    int  flashModeList;
    int  flashMode;
    int  flashPreMode;

    int  focusModeList;
    int  focusMode;

    int  sceneModeList;
    int  sceneMode;

    int  whiteBalanceList;
    int  whiteBalance;
    bool autoWhiteBalanceLockSupported;
    bool autoWhiteBalanceLock;

    int  rotation;
    int  minExposure;
    int  maxExposure;
    int  exposure;

    bool autoExposureLockSupported;
    bool autoExposureLock;

    int  fpsRange[2];

    int  fNumberNum;
    int  fNumberDen;
    int  focalLengthNum;
    int  focalLengthDen;
    int  apertureNum;
    int  apertureDen;
    bool supportVideoStabilization;
    bool applyVideoStabilization;
    bool videoStabilization;
    int  maxNumMeteringAreas;
    int  maxNumDetectedFaces;
    int  maxNumFocusAreas;
    int  maxZoom;
    bool hwZoomSupported;
    int  zoom;

    double gpsLatitude;
    double gpsLongitude;
    double gpsAltitude;
    long gpsTimestamp;

    // Additional API.
    int  angle;
    bool antiShake;
    int  brightness;
    int  contrast;
    bool gamma;
    bool odc;
    int  hue;
    int  iso;
    int  metering;
    bool objectTracking;
    bool objectTrackingStart;

    int  saturation;
    int  sharpness;
    bool slowAE;
    bool touchAfStart;
    bool tdnr;
    int  vtMode;
    int  intelligentMode;
#ifdef SCALABLE_SENSOR
    bool scalableSensorStart;
#endif
    int  focalLengthIn35mmLength;
    int  autoFocusMacroPosition;

public:
    ExynosCameraInfo();
};

struct ExynosCameraInfoM5M0 : public ExynosCameraInfo
{
public:
    ExynosCameraInfoM5M0();
};

struct ExynosCameraInfoS5K6A3 : public ExynosCameraInfo
{
public:
    ExynosCameraInfoS5K6A3();
};

struct ExynosCameraInfoS5K6B2 : public ExynosCameraInfo
{
public:
    ExynosCameraInfoS5K6B2();
};

struct ExynosCameraInfoS5K4E5 : public ExynosCameraInfo
{
public:
    ExynosCameraInfoS5K4E5();
};

struct ExynosCameraInfoS5K3H5 : public ExynosCameraInfo
{
public:
    ExynosCameraInfoS5K3H5();
};

struct ExynosCameraInfoS5K3H7 : public ExynosCameraInfo
{
public:
    ExynosCameraInfoS5K3H7();
};

struct ExynosCameraInfoIMX135 : public ExynosCameraInfo
{
public:
    ExynosCameraInfoIMX135();
};

struct ExynosCameraInfoIMX135Reprocessing : public ExynosCameraInfoIMX135
{
public:
    ExynosCameraInfoIMX135Reprocessing();
};

struct ExynosCameraInfoS5K6B2Reprocessing : public ExynosCameraInfoS5K6B2
{
public:
    ExynosCameraInfoS5K6B2Reprocessing();
};

struct ExynosCameraInfoS5K4E5Reprocessing : public ExynosCameraInfoS5K4E5
{
public:
    ExynosCameraInfoS5K4E5Reprocessing();
};

//! ExynosCamera
/*!
 * \ingroup Exynos
 */
class ExynosCamera : public virtual RefBase {

///////////////////////////////////////////////////
// Google Official API : Camera.Parameters
// http://developer.android.com/reference/android/hardware/Camera.Parameters.html
///////////////////////////////////////////////////
public:
    //! Camera ID
    enum CAMERA_ID {
        CAMERA_ID_BACK  = 0,   //!<
        CAMERA_ID_FRONT = 1,   //!<
        CAMERA_ID_MAX,         //!<
    };

    //! Camera Mode
    enum CAMERA_MODE {
        CAMERA_MODE_BACK          = 0,   //!<
        CAMERA_MODE_FRONT         = 1,   //!<
        CAMERA_MODE_REPROCESSING  = 2,   //!<
        CAMERA_MODE_MAX,                 //!<
    };

    //! Anti banding
    enum {
        ANTIBANDING_AUTO = (1 << 0), //!< \n
        ANTIBANDING_50HZ = (1 << 1), //!< \n
        ANTIBANDING_60HZ = (1 << 2), //!< \n
        ANTIBANDING_OFF  = (1 << 3), //!< \n
    };

    //! Effect
    enum {
        EFFECT_NONE       = (1 << 0), //!< \n
        EFFECT_MONO       = (1 << 1), //!< \n
        EFFECT_NEGATIVE   = (1 << 2), //!< \n
        EFFECT_SOLARIZE   = (1 << 3), //!< \n
        EFFECT_SEPIA      = (1 << 4), //!< \n
        EFFECT_POSTERIZE  = (1 << 5), //!< \n
        EFFECT_WHITEBOARD = (1 << 6), //!< \n
        EFFECT_BLACKBOARD = (1 << 7), //!< \n
        EFFECT_AQUA       = (1 << 8), //!< \n
    };

    //! Flash mode
    enum {
        FLASH_MODE_OFF     = (1 << 0), //!< \n
        FLASH_MODE_AUTO    = (1 << 1), //!< \n
        FLASH_MODE_ON      = (1 << 2), //!< \n
        FLASH_MODE_RED_EYE = (1 << 3), //!< \n
        FLASH_MODE_TORCH   = (1 << 4), //!< \n
    };

    //! Focus mode
    enum {
        FOCUS_MODE_AUTO               = (1 << 0), //!< \n
        FOCUS_MODE_INFINITY           = (1 << 1), //!< \n
        FOCUS_MODE_MACRO              = (1 << 2), //!< \n
        FOCUS_MODE_FIXED              = (1 << 3), //!< \n
        FOCUS_MODE_EDOF               = (1 << 4), //!< \n
        FOCUS_MODE_CONTINUOUS_VIDEO   = (1 << 5), //!< \n
        FOCUS_MODE_CONTINUOUS_PICTURE = (1 << 6), //!< \n
        FOCUS_MODE_TOUCH              = (1 << 7), //!< \n
    };

    //! Scene mode
    enum {
        SCENE_MODE_AUTO           = (1 << 0), //!< \n
        SCENE_MODE_ACTION         = (1 << 1), //!< \n
        SCENE_MODE_PORTRAIT       = (1 << 2), //!< \n
        SCENE_MODE_LANDSCAPE      = (1 << 3), //!< \n
        SCENE_MODE_NIGHT          = (1 << 4), //!< \n
        SCENE_MODE_NIGHT_PORTRAIT = (1 << 5), //!< \n
        SCENE_MODE_THEATRE        = (1 << 6), //!< \n
        SCENE_MODE_BEACH          = (1 << 7), //!< \n
        SCENE_MODE_SNOW           = (1 << 8), //!< \n
        SCENE_MODE_SUNSET         = (1 << 9), //!< \n
        SCENE_MODE_STEADYPHOTO    = (1 << 10), //!< \n
        SCENE_MODE_FIREWORKS      = (1 << 11), //!< \n
        SCENE_MODE_SPORTS         = (1 << 12), //!< \n
        SCENE_MODE_PARTY          = (1 << 13), //!< \n
        SCENE_MODE_CANDLELIGHT    = (1 << 14), //!< \n
    };

    //! White balance
    enum {
        WHITE_BALANCE_AUTO             = (1 << 0), //!< \n
        WHITE_BALANCE_INCANDESCENT     = (1 << 1), //!< \n
        WHITE_BALANCE_FLUORESCENT      = (1 << 2), //!< \n
        WHITE_BALANCE_WARM_FLUORESCENT = (1 << 3), //!< \n
        WHITE_BALANCE_DAYLIGHT         = (1 << 4), //!< \n
        WHITE_BALANCE_CLOUDY_DAYLIGHT  = (1 << 5), //!< \n
        WHITE_BALANCE_TWILIGHT         = (1 << 6), //!< \n
        WHITE_BALANCE_SHADE            = (1 << 7), //!< \n
    };

    //! Jpeg Qualtiy
    enum JPEG_QUALITY {
        JPEG_QUALITY_MIN        = 0,    //!<
        JPEG_QUALITY_ECONOMY    = 70,   //!<
        JPEG_QUALITY_NORMAL     = 80,   //!<
        JPEG_QUALITY_SUPERFINE  = 90,   //!<
        JPEG_QUALITY_MAX        = 100,  //!<
    };

public:
    //! Constructor
    ExynosCamera();
    //! Destructor
    virtual ~ExynosCamera();

    //! Create the instance
    bool            create(int cameraId);
    //! Destroy the instance
    bool            destroy(void);
    //! Check if the instance was created
    bool            flagCreate(void);

    //! Open camera
    bool            openCamera(int cameraId);
    //! Check if the instance was opened
    bool            flagOpen(int cameraId);

    //! Gets current camera_id
    int             getCameraId(void);
    //! Gets current camera mode
    int             getCameraMode();
    //! Gets camera sensor name
    char           *getCameraName(void);

    bool            qAll3a1Buf(void);
    bool            dqAll3a1Buf(void);
    bool            dq3a1Buf(enum CAMERA_MODE cameraMode, ExynosBuffer *inBuf, ExynosBuffer *outBuf);
    bool            q3a1Buf(enum CAMERA_MODE cameraMode);

    //! Starts capturing and drawing preview frames to the screen.
    bool            startPreview(void);
    //! Stop preview
    bool            stopPreview(void);
    //! Check preview start
    bool            flagStartPreview(void);
    //! Gets preview's max buffer
    int             getPreviewMaxBuf(void);
    //! Sets preview's buffer
    bool            setPreviewBuf(ExynosBuffer *buf);
    //! Gets preview's buffer
    bool            getPreviewBuf(ExynosBuffer *buf, bool *isValid, nsecs_t *timestamp);
    //! Put(dq) preview's buffer
    bool            putPreviewBuf(ExynosBuffer *buf);
    //! Cancel preview's buffer
    bool            cancelPreviewBuf(int index);
    //! Deinitialize preview's buffer
    bool            clearPreviewBuf(void);
    //! Alloc preview's internal buffer
    bool            allocInternalPreviewBuf(ExynosBuffer *buf);
    //! Free preview's internal buffer
    void            releaseInternalPreviewBuf(int index);
    //! Free All preview's internal buffer
    void            releaseAllInternalPreviewBuf(void);
    //! Set flag preview's internal buffer
    void            setFlagInternalPreviewBuf(bool flag);
    //! get flag preview's internal buffer
    bool            getFlagInternalPreviewBuf(void);

    //! Start sensor
    bool            startSensor(void);
    //! Stop sensor
    bool            stopSensor(void);
    //! Start sensor on
    bool            startSensorOn(enum CAMERA_MODE cameraMode);
    //! Stop sensor off
    bool            stopSensorOff(enum CAMERA_MODE cameraMode);
    //! Check sensor start
    bool            flagStartSensor(void);
    //! Check sensor buffer empty
    bool			flagEmptySensorBuf(ExynosBuffer *buf);
    //! Gets sensor buffer
    bool            getSensorBuf(ExynosBuffer *buf);
    //! Put sensor buffer
    bool            putSensorBuf(ExynosBuffer *buf);

    bool            getFixedSensorBuf(ExynosBuffer *buf);
    bool            putFixedSensorBuf(ExynosBuffer *buf);

    bool            StartStream(void);

    //! Start isp
    bool            startIsp(void);
    //! Stop isp
    bool            stopIsp(void);
    //! Check isp start
    bool            flagStartIsp(void);
    //! Stop isp force off
    bool            stopIspForceOff(void);
    //! Stop isp force off
    bool            stopSensorForceOff(void);
    //! Gets ISP's buffer
    bool            getISPBuf(ExynosBuffer *buf);
    //! Put(dq) ISP's buffer
    bool            putISPBuf(ExynosBuffer *buf);
    //! Gets ISPReprocessing's buffer
    bool            getISPBufReprocessing(ExynosBuffer *buf);
    //! Put(dq) ISPReprocessing's buffer
    bool            putISPBufReprocessing(ExynosBuffer *buf);

    //! Start is3a0
    bool            startIs3a0(enum CAMERA_MODE cameraMode);
    //! Stop is3a0
    bool            stopIs3a0(enum CAMERA_MODE cameraMode);
    //! Check is3a1Src start
    bool            flagStartIs3a0Src(enum CAMERA_MODE cameraMode);
    //! Check is3a1Dst start
    bool            flagStartIs3a0Dst(enum CAMERA_MODE cameraMode);

    //! Start is3a1
    bool            startIs3a1(enum CAMERA_MODE cameraMode);
    //! Stop is3a1
    bool            stopIs3a1(enum CAMERA_MODE cameraMode);
    //! Check is3a1Src start
    bool            flagStart3a1Src(enum CAMERA_MODE cameraMode);
    //! Check is3a1Dst start
    bool            flagStart3a1Dst(enum CAMERA_MODE cameraMode);
    //! Gets pre chain's buffer
    bool            getIs3a0Buf(enum CAMERA_MODE cameraMode, ExynosBuffer *inBuf, ExynosBuffer *outBuf);
    //! Put(dq) pre chain's buffer
    bool            putIs3a0Buf(ExynosBuffer *buf);
    //! Gets pre chain's buffer
    bool            getIs3a0BufReprocessing(enum CAMERA_MODE cameraMode, ExynosBuffer *inBuf, ExynosBuffer *outBuf);
    //! Gets pre chain's buffer
    bool            getIs3a1Buf(enum CAMERA_MODE cameraMode, ExynosBuffer *inBuf, ExynosBuffer *outBuf);
    //! Put(dq) pre chain's buffer
    bool            putIs3a1Buf(ExynosBuffer *buf);

#ifdef USE_VDIS
    void            setVdisInStatus(bool toggle);
    bool            getVdisInStatus(void);

    //! stream on VdisCapture
    bool            startVdisCapture(void);
    //! stream on VdisOutput
    bool            startVdisOutput(void);
    //! stream off VdisCapture
    bool            stopVdisCapture(void);
    //! stream off VdisOutput
    bool            stopVdisOutput(void);
    //! Check VdisCapture start
    bool            flagStartVdisCapture();
    //! Check VdisOutput start
    bool            flagStartVdisOutput();
    //! Gets vdis out buffer
    bool            getVDisSrcBuf(ExynosBuffer *buf, int *rcount, int *fcount);
    //! Put vdis out  buffer
    bool            putVDisSrcBuf(ExynosBuffer *buf);
    //! Gets vdis in  buffer
    bool            getVDisDstBuf(ExynosBuffer *buf);
    //! Put vdis in  buffer
    bool            putVDisDstBuf(ExynosBuffer *buf, int rcount, int fcount);
    //! Gets vdis in  buffer address
    bool            getVDisDstBufAddr(ExynosBuffer **buf, int i);

    void            setVDisSrcW(unsigned int value);
    void            setVDisSrcH(unsigned int value);
    void            setVDisDstW(unsigned int value);
    void            setVDisDstH(unsigned int value);
    void            setVDisSrcBufNum(unsigned int value);
    void            setVDisDstBufNum(unsigned int value);

    bool            getVdisMode();
    void            setVdisMode(bool setVdisMode);
#endif

    //! Sets video's width, height
    bool            setVideoSize(int w, int h);
    //! Gets video's width, height
    bool            getVideoSize(int *w, int *h);

    //! Sets video's color format
    bool            setVideoFormat(int colorFormat);
    //! Gets video's color format
    int             getVideoFormat(void);

    //! Start video
    bool            startVideo(void);
    //! Stop video
    bool            stopVideo(void);
    //! Check video start
    bool            flagStartVideo(void);
    //! Gets video's buffer
    int             getVideoMaxBuf(void);
    //! Sets video's buffer
    bool            setVideoBuf(ExynosBuffer *buf);
    //! Gets video's buffer
    bool            getVideoBuf(ExynosBuffer *buf);
    //! Put(dq) video's buffer
    bool            putVideoBuf(ExynosBuffer *buf);

    //! Start snapshot
    bool            startPicture(void);
    //! Stop snapshot
    bool            stopPicture(void);
    //! Check snapshot start
    bool            flagStartPicture(void);
    //! Gets snapshot's buffer
    int             getPictureMaxBuf(void);
    //! Sets snapshot's buffer
    bool            setPictureBuf(ExynosBuffer *buf);
    //! Gets snapshot's buffer
    bool            getPictureBuf(ExynosBuffer *buf);
    //! Put(dq) snapshot's buffer
    bool            putPictureBuf(ExynosBuffer *buf);
    //! Set Picture Req
    void            pictureOn(void);

    //! Encode JPEG from YUV
    bool            yuv2Jpeg(ExynosBuffer *yuvBuf, ExynosBuffer *jpegBuf, ExynosRect *rect);

    //! Starts camera auto-focus and registers a callback function to run when the camera is focused.
    bool            autoFocus(void);
    //! Cancel auto-focus operation
    bool            cancelAutoFocus(void);
    int             getCAFResult(void);

    //! Starts the face detection.
    bool            startFaceDetection(void);
    //! Stop face detection
    bool            stopFaceDetection(void);
    //! Gets the face detection started
    bool            flagStartFaceDetection(void);

    //! Zooms to the requested value smoothly.
    bool            startSmoothZoom(int value);
    //! Stop the face detection.
    bool            stopSmoothZoom(void);

    //! Gets the current antibanding setting.
    int             getAntibanding(void);

    //! Gets the state of the auto-exposure lock.
    bool            getAutoExposureLock(void);

    //! Gets the state of the auto-white balance lock.
    bool            getAutoWhiteBalanceLock(void);

    //! Gets the current color effect setting.
    int             getColorEffect(void);

    //! Gets the detected faces areas.
    int             getDetectedFacesAreas(int num, int *id, int *score, ExynosRect *face, ExynosRect *leftEye, ExynosRect *rightEye, ExynosRect *mouth);

    //! Gets the detected faces areas. (Using ExynosRect2)
    int             getDetectedFacesAreas(int num, int *id, int *score, ExynosRect2 *face, ExynosRect2 *leftEye, ExynosRect2 *rightEye, ExynosRect2 *mouth);

    //! Gets the current exposure compensation index.
    int             getExposureCompensation(void);

    //! Gets the exposure compensation step.
    float           getExposureCompensationStep(void);

    //! Gets the current flash mode setting.
    int             getFlashMode(void);

    //! Gets the focal length (in millimeter) of the camera.
    bool            getFocalLength(int *num, int *den);

    //! Gets the current focus areas.
    int             getFocusAreas(ExynosRect *rects);

    //! Gets the distances from the camera to where an object appears to be in focus.
    bool            getFocusDistances(int *num, int *den);;

    //! Gets the current focus mode setting.
    int             getFocusMode(void);

    //! Gets the horizontal angle of view in degrees.
    float           getHorizontalViewAngle(void);

    //int             getInt(String key);

    //! Returns the quality setting for the JPEG picture.
    int             getJpegQuality(void);

    //! Returns the quality setting for the EXIF thumbnail in Jpeg picture.
    int             getJpegThumbnailQuality(void);

    //! Returns the dimensions for EXIF thumbnail in Jpeg picture.
    bool            getJpegThumbnailSize(int *w, int *h);

    //! Gets the maximum exposure compensation index.
    int             getMaxExposureCompensation(void);

    //! Gets the maximum number of detected faces supported.
    int             getMaxNumDetectedFaces(void);

    //! Gets the maximum number of focus areas supported.
    int             getMaxNumFocusAreas(void);

    //! Gets the maximum number of metering areas supported.
    int             getMaxNumMeteringAreas(void);

    //! Gets the maximum zoom value allowed for snapshot.
    int             getMaxZoom(void);

    //! Gets the current metering areas.
    int             getMeteringAreas(ExynosRect *rects);

    //! Gets the minimum exposure compensation index.
    int             getMinExposureCompensation(void);

    //! Returns the dimension setting for isp.
    bool            getIspSize(int *w, int *h);

    //! Returns the image format for pictures.
    int             getPictureFormat(void);

    //! Returns the dimension setting for pictures.
    bool            getPictureSize(int *w, int *h);

    //Camera.Size     getPreferredPreviewSizeForVideo();

    //! Returns the image format for preview frames got from Camera.PreviewCallback.
    int             getPreviewFormat(void);

    //! Returns the current minimum and maximum preview fps.
    bool            getPreviewFpsRange(int *min, int *max);

    //! Returns the dimensions setting for preview pictures.
    bool            getPreviewSize(int *w, int *h);

    //! Returns the recording mode hint.
    bool            getRecordingHint(void);

    //! Gets scene mode
    int             getSceneMode(void);

    //! Gets the supported antibanding values.
    int             getSupportedAntibanding(void);

    //! Gets the supported color effects.
    int             getSupportedColorEffects(void);

    //! Check whether the target support Flash
    int             getSupportedFlashModes(void);

    //! Gets the supported focus modes.
    int             getSupportedFocusModes(void);

    //! Gets the supported jpeg thumbnail sizes.
    bool            getSupportedJpegThumbnailSizes(int *w, int *h);

    // List<Integer>  getSupportedPictureFormats()

    //! Gets the supported picture sizes.
    bool            getSupportedPictureSizes(int *w, int *h);

    //List<Integer>   getSupportedPreviewFormats()

    //! Gets the supported preview fps range.
    bool            getSupportedPreviewFpsRange(int *min, int *max);

    //List<Integer>   getSupportedPreviewFrameRates()

    //! Gets the supported preview sizes.
    bool            getSupportedPreviewSizes(int *w, int *h);

    //! Gets the supported scene modes.
    int             getSupportedSceneModes(void);

    //! Gets the supported video frame sizes that can be used by MediaRecorder.
    bool            getSupportedVideoSizes(int *w, int *h);

    //! Gets the supported white balance.
    int             getSupportedWhiteBalance(void);

    //! Gets the vertical angle of view in degrees.
    float           getVerticalViewAngle(void);

    //! Gets the current state of video stabilization.
    bool            getVideoStabilization(void);

    //! Gets the current white balance setting.
    int             getWhiteBalance(void);

    //! Gets current zoom value.
    int             getZoom(void);

    //List<Integer>   getZoomRatios()
    //! Gets max zoom ratio
    int             getMaxZoomRatio(void);

    //! Returns true if auto-exposure locking is supported.
    bool            isAutoExposureLockSupported(void);

    //! Returns true if auto-white balance locking is supported.
    bool            isAutoWhiteBalanceLockSupported(void);

    //! Returns true if smooth zoom is supported.
    bool            isSmoothZoomSupported(void);

    //! Returns true if video snapshot is supported.
    bool            isVideoSnapshotSupported(void);

    //! Returns true if video stabilization is supported.
    bool            isVideoStabilizationSupported(void);

    //! Returns true if zoom is supported.
    bool            isZoomSupported(void);

    //void            remove(String key)

    //void            removeGpsData()

    //void            set(String key, String value)

    //void            set(String key, int value)

    //! Sets the antibanding.
    bool            setAntibanding(int value);

    //! Sets the auto-exposure lock state.
    bool            setAutoExposureLock(bool toggle);

    //! Sets the auto-white balance lock state.
    bool            setAutoWhiteBalanceLock(bool toggle);

    //! Sets the current color effect setting.
    bool            setColorEffect(int value);

    //! Sets the exposure compensation index.
    bool            setExposureCompensation(int value);

    //! Sets the flash mode.
    bool            setFlashMode(int value);

    //! Sets focus z.
    bool            setFocusAreas(int num, ExynosRect* rects, int *weights);

    //! Sets focus areas. (Using ExynosRect2)
    bool            setFocusAreas(int num, ExynosRect2* rect2s, int *weights);

    //! Sets the focus mode.
    bool            setFocusMode(int value);

    //! Sets GPS altitude.
    bool            setGpsAltitude(const char *gpsAltitude);

    //! Sets GPS latitude coordinate.
    bool            setGpsLatitude(const char *gpsLatitude);

    //! Sets GPS longitude coordinate.
    bool            setGpsLongitude(const char *gpsLongitude);

    //! Sets GPS processing method.
    bool            setGpsProcessingMethod(const char *gpsProcessingMethod);

    //! Sets GPS timestamp.
    bool            setGpsTimeStamp(const char *gpsTimestamp);

    //! Sets Jpeg quality of captured picture.
    bool            setJpegQuality(int quality);

    //! Sets the quality of the EXIF thumbnail in Jpeg picture.
    bool            setJpegThumbnailQuality(int quality);

    //! Sets the dimensions for EXIF thumbnail in Jpeg picture.
    bool            setJpegThumbnailSize(int w, int h);

    //! Sets metering areas.
    bool            setMeteringAreas(int num, ExynosRect  *rects, int *weights);

    //! Sets metering areas.(Using ExynosRect2)
    bool            setMeteringAreas(int num, ExynosRect2 *rect2s, int *weights);

    //! Cancel metering areas.
    bool            cancelMeteringAreas();

    //! Sets the image format for pictures.
    bool            setPictureFormat(int colorFormat);

    //! Sets the dimensions for pictures.
    bool            setPictureSize(int w, int h);

    //! Sets the image format for preview pictures.
    bool            setPreviewFormat(int colorFormat);

    //! Sets the frame rate range for preview.
    bool            setPreviewFpsRange(int min, int max);

    //! Sets the dimensions for preview pictures.
    bool            setPreviewSize(int w, int h);

    //! Sets recording mode hint.
    bool            setRecordingHint(bool hint);

    //! Sets the rotation angle in degrees relative to the orientation of the camera.
    bool            setRotation(int rotation);

    //! Gets the rotation angle in degrees relative to the orientation of the camera.
    int             getRotation(void);
#ifdef FD_ROTATION
    //! Sets and notify the device orientation angle in degrees to camera FW for FD scanning property.
    bool            setDeviceOrientation(int Orientation);

    //! Gets the device orientation angle in degrees to camera FW for FD scanning property.
    int             getDeviceOrientation(void);
#endif
    //! Sets the scene mode.
    bool            setSceneMode(int value);

    //! Enables and disables video stabilization.
    bool            setVideoStabilization(bool toggle);

    //! Sets the white balance.
    bool            setWhiteBalance(int value);

    //! Sets current zoom value.
    bool            setZoom(int value);

    //void            unflatten(String flattened)

    bool            DvfsLock();
    bool            DvfsUnLock();

    bool            getCropRect(int  src_w,  int   src_h,
                                int  dst_w,  int   dst_h,
                                int *crop_x, int *crop_y,
                                int *crop_w, int *crop_h,
                                int  zoom);
    bool            getCropRect2(int  src_w,     int   src_h,
                                 int  dst_w,     int   dst_h,
                                 int *new_src_x, int *new_src_y,
                                 int *new_src_w, int *new_src_h,
                                 int  zoom);
    bool            getCropRectAlign(int  src_w,  int   src_h,
                                     int  dst_w,  int   dst_h,
                                     int *crop_x, int *crop_y,
                                     int *crop_w, int *crop_h,
                                     int align_w, int align_h,
                                     int  zoom);

    bool            getFlagFlashOn(void);

    bool            fileDump(char *filename,
                             char *srcBuf,
                             int w, int h, unsigned int size);

    bool            startSensorReprocessing(void);
    bool            stopSensorReprocessing(void);
    bool            getSensorBufReprocessing(ExynosBuffer *buf);
    bool            putSensorBufReprocessing(ExynosBuffer *buf);
    bool            flagStartSensorReprocessing(void);
    bool            startPreviewReprocessing(void);
    bool            stopPreviewReprocessing(void);
    bool            flagStartPreviewReprocessing(void);
    bool            startIspReprocessing(void);
    bool            stopIspReprocessing(void);
    bool            m_startIspReprocessingOn(void);
    bool            m_stopIspReprocessingOff(void);
    bool            flagStartIspReprocessing(void);
    bool            startPictureReprocessing(void);
    bool            m_startPictureReprocessing(void);
    bool            m_stopPictureReprocessing(void);
    bool            stopPictureReprocessing(void);
    void            pictureOnReprocessing(void);
    bool            setPictureBufReprocessing(ExynosBuffer *buf);
    bool            getPictureBufReprocessing(ExynosBuffer *buf);
    bool            putPictureBufReprocessing(ExynosBuffer *buf);
    bool            flagStartPictureReprocessing(void);
    bool            getPictureSizeReprocessing(int *w, int *h);
    bool            getPreviewSizeReprocessing(int *w, int *h);
    void            StartStreamReprocessing();

    int             getNumOfShotedFrame(void);
    int             getNumOfShotedIspFrame(void);
    void            notifyStop(bool msg);
    bool            getNotifyStopMsg(void);
    bool            setSensorStreamOff(enum CAMERA_MODE cameraMode);

public:
#ifdef SCALABLE_SENSOR
    bool            setScalableSensorSize(enum SCALABLE_SENSOR_SIZE sizeMode);
    bool            getScalableSensorStart(void);
    bool            setScalableSensorStart(bool toggle);
    void            getScalableSensorSizeOnPreview(int *w, int *h);
#endif
#if CAPTURE_BUF_GET
    int             getCapturerBufIndex();
    int             getCapturerBuf(ExynosBuffer *buf);
    bool            setBayerLockIndex(int index, bool setLock);
    bool            setBayerLock(unsigned int fcount, bool setLock);
    void            printBayerLockStatus();
    bool            checkCaptureBayerOnHAL(int index);
#endif
    ExynosCameraActivityFlash *getFlashMgr(void);
    ExynosCameraActivitySpecialCapture *getSpecialCaptureMgr(void);
    bool            fileDump(char *filename, char *srcBuf, unsigned int size);

#ifdef USE_CAMERA_ESD_RESET
    bool		  stateESDReset(void);
    int 		getPlaneSizePackedFLiteOutput(int width, int heiht);
#endif

private:
    bool            m_flagCreate;

    int             m_cameraId;    /* set by service */
    int             m_cameraMode;  /* set by hal */

    int             m_needCallbackCSC;

    ExynosCameraInfo  *m_defaultCameraInfo[CAMERA_MODE_MAX];
    ExynosCameraInfo  *m_curCameraInfo[CAMERA_MODE_MAX];
    bool            m_flagOpen[CAMERA_MODE_MAX];

    int             m_jpegQuality;
    int             m_jpegThumbnailQuality;

    int             m_currentZoom;
    bool            m_recordingHint;
    bool            m_isHWVDis;

    bool            m_isFirtstSensorStart;
    bool            m_isFirtstIs3a1SrcStart;
    bool            m_isFirtstIs3a1DstStart;
    bool            m_isFirtstIs3a0SrcStart;
    bool            m_isFirtstIs3a0DstStart;
    bool            m_isFirtstIspStart;

    bool            m_isFirtstIspStartReprocessing;
    bool            m_isFirtstSensorStartReprocessing;

    bool            m_isFirtstIs3a1StreamOn;
    bool            m_isFirtstIs3a0StreamOn;
    bool            m_isIs3a1ParamChanged;
    bool            m_isIs3a0ParamChanged;

    int             m_cameraModeIs3a0;

    char            m_cameraName[32];
#ifdef USE_VDIS
    int             m_vdisInIndex;   // buffer index
    int             m_vdisInRcount;  // unique ID from ISP input
    int             m_vdisInFcount;  // unique ID from sensor input
    bool            m_vdisInIndexUpdate;
    unsigned int    m_VDisSrcW;
    unsigned int    m_VDisSrcH;
    unsigned int    m_VDisDstW;
    unsigned int    m_VDisDstH;
    unsigned int    m_VDisSrcBufNum;
    unsigned int    m_VDisDstBufNum;
#endif

    // media controller variable
    struct media_device *m_media;
    struct media_entity *m_sensorEntity;
    struct media_entity *m_mipiEntity;
    struct media_entity *m_fliteSdEntity;
    struct media_entity *m_fliteVdEntity;
    struct media_entity *m_gscSdEntity;
    struct media_entity *m_gscVdEntity;
    struct media_entity *m_ispSensorEntity;
    struct media_entity *m_ispFrontEntity;
    struct media_entity *m_ispBackEntity;
    struct media_entity *m_ispBayerEntity;
    struct media_entity *m_ispScalercEntity;
    struct media_entity *m_ispScalerpEntity;
    struct media_entity *m_isp3dnrEntity;

    struct ExynosBuffer m_videoBuf[VIDEO_MAX_FRAME];

    int              m_is3a1SrcLastBufIndex;
    int              m_is3a1DstLastBufIndex;
    int              m_is3a1FrameCount;

    exif_attribute_t mExifInfo;
    char             m_imageUniqueIdBuf[UNIQUE_ID_BUF_SIZE];

    ion_client       m_ionCameraClient;
    camera_hw_info_t m_camera_info[CAMERA_MODE_MAX];
    mutable Mutex    m_sensorLock;
    mutable Mutex    m_sensorLockReprocessing;

    bool             m_internalISP;
#ifdef FD_ROTATION
    int              m_deviceOrientation;
#endif
    bool             m_flagStartFaceDetection;
    bool             m_flagAutoFocusRunning;
    bool             m_touchAFMode;
    bool             m_touchAFModeForFlash;
    int              m_oldMeteringMode;

#ifdef USE_CAMERA_ESD_RESET
    bool m_stateESDReset;
#endif
    Mutex            m_requestMutex;
    List<int>        m_sensorQ;
    unsigned int     m_sensorFrameCount;

    int              m_aeFrameCount;
    int              m_numOfShotedFrame;
    int              m_numOfShotedIspFrame;
    bool             m_notifyStopMsg;
    int              m_traceCount;

    bool             m_previewInternalMemAlloc;

    ExynosBuffer     m_previewMetaBuffer[VIDEO_MAX_FRAME];

    mutable Mutex    m_dvfsLock;
    bool             m_isDVFSLocked;

#if CAPTURE_BUF_GET
    #define MAX_CAPTURE_BAYER_COUNT 0
    #define MIN_CAPTURE_BAYER_COUNT 2
    int              m_recentCaptureBayerBufIndex;

    Mutex            m_bayerMutex;
    List<int>        m_captureBayerQ;
    bool             m_notInsertBayer;
    int              m_minCaptureBayerBuf;
    int              m_captureBayerIndex[NUM_BAYER_BUFFERS];
    bool             m_captureBayerLock[NUM_BAYER_BUFFERS];
    bool             m_waitCaptureBayer;
#endif

    ExynosCameraActivityFlash *m_flashMgr;
    ExynosCameraActivityAutofocus *m_autofocusMgr;
    ExynosCameraActivitySpecialCapture *m_sCaptureMgr;
    //metadata buffer
    ExynosBuffer     m_metaBuf[VIDEO_MAX_FRAME];

#ifdef THREAD_PROFILE
    struct timeval   mTimeStart;
    struct timeval   mTimeStop;
    unsigned long    timeUs;
#endif

#ifdef FRONT_NO_ZSL
    int m_frontCaptureStatus;
#endif

#ifdef FORCE_LEADER_OFF
    int m_forceIspOff;
#endif

private:
    bool            m_openInternalISP(int cameraMode);
    bool            m_openExternalISP(int cameraMode);

    bool            m_initSensor(int cameraMode);

    bool            m_openSensor(int cameraMode);
    bool            m_closeSensor(int cameraMode);

    bool            m_openIs3a0(int cameraMode);
    bool            m_closeIs3a0(int cameraMode);

    bool            m_openIs3a1(int cameraMode);
    bool            m_closeIs3a1(int cameraMode);

    bool            m_openIsp(int cameraMode);
    bool            m_closeIsp(int cameraMode);

#ifdef USE_VDIS
    bool            m_openVdisOut(void);
    bool            m_closeVdisOut(void);
#endif

    bool            m_openPreview(int cameraMode);
    bool            m_closePreview(int cameraMode);

    bool            m_openPicture(int cameraMode);
    bool            m_closePicture(int cameraMode);

    bool            m_startIsp(void);
    bool            m_stopIsp(void);
    bool            m_startPreview(void);
    bool            m_stopPreview(void);
    bool            m_startPicture(void);
    bool            m_stopPicture(void);

    bool            m_setSetfile(int cameraMode);
    bool            m_setZoom(int zoom, int srcW, int srcH, int dstW, int dstH, void *ptr);
    void            m_setExifFixedAttribute(void);
    void            m_setExifChangedAttribute(exif_attribute_t *exifInfo, ExynosRect *rect);
    void            m_secRect2SecRect2(ExynosRect *rect, ExynosRect2 *rect2);
    void            m_secRect22SecRect(ExynosRect2 *rect2, ExynosRect *rect);
    void            m_printFormat(int colorFormat, const char *arg);
    bool            m_sensorThreadFuncWrap(void);
    bool            m_sensorThreadFunc(void);
    void            m_pushSensorQ(int index);
    int             m_popSensorQ(void);
    void            m_printSensorQ(void);
    void            m_releaseSensorQ(void);
    int             m_getSensorId(int cameraId);
    void            m_turnOffEffectByFps(camera2_shot_ext *shot_ext, int fps);
    ExynosRect2     m_AndroidArea2HWArea(ExynosRect2 *rect2);
    ExynosRect2     m_AndroidArea2HWArea(ExynosRect2 *rect2, int w, int h);
    bool            m_startFaceDetection(enum CAMERA_MODE cameraMode, bool toggle);
    bool            m_getImageUniqueId(void);
#if CAPTURE_BUF_GET
    void            m_setBayerQ(int index);
    int             m_checkLastBayerQ(void);
    int             m_getBayerQ(void);
    void            m_printBayerQ(void);
    void            m_releaseBayerQ(void);
#endif

    /* For v4l2_ioctls interfaces */
    int             cam_int_s_input(node_info_t *node, int index);
    int             cam_int_s_fmt(node_info_t *node);
    int             cam_int_querybuf(node_info_t *node);
    int             cam_int_reqbufs(node_info_t *node);
    int             cam_int_clrbufs(node_info_t *node);
    int             cam_int_streamon(node_info_t *node);
    int             cam_int_streamoff(node_info_t *node);
    int             cam_int_qbuf(node_info_t *node, int index);
    int             cam_int_qbuf(node_info_t *node, int index, node_info_t *srcNode);
    int             cam_int_qbuf(node_info_t *node, int index, node_info_t *srcNode, ExynosBuffer mataBuf);
    int             cam_int_dqbuf(node_info_t *node);
#ifdef USE_CAMERA_ESD_RESET
    int             cam_int_polling(node_info_t *node);
#endif
    int             cam_int_get_pixel_depth(node_info_t *node);
    bool            cam_int_m_flagQ(node_info_t *node, int index);
    void            cam_int_m_setQ(node_info_t *node, int index, bool toggle);

private:
    bool            allocMemSinglePlane(ion_client ionClient, ExynosBuffer *buf, int index, bool flagCache = true);
    bool            allocMemSinglePlaneReserved(ion_client ionClient, ExynosBuffer *buf, int index, bool flagCache = true,  unsigned int heap_mask = 0, unsigned int flags = 0);
    void            freeMemSinglePlane(ExynosBuffer *buf, int index);
    bool            allocMem(ion_client ionClient, ExynosBuffer *buf, int cacheIndex = 0xff);
    bool            allocMemReserved(ion_client ionClient, ExynosBuffer *buf, int cacheIndex = 0xff,  unsigned int heap_mask = 0, unsigned int flags = 0);
    void            freeMem(ExynosBuffer *buf);
    bool            allocMemSinglePlaneCache(ion_client ionClient, ExynosBuffer *buf, int index, bool flagCache = true);
    bool            allocMemCache(ion_client ionClient, ExynosBuffer *buf, int cacheIndex = 0xff);
    ion_client      getIonClient(void);
    int             setFPSParam(int fps);

    bool            setSensorStreamOn(enum CAMERA_MODE cameraMode, int width, int height, bool isSetFps);

///////////////////////////////////////////////////
// Additional API.
///////////////////////////////////////////////////
public:
    //! Focus mode
    enum {
        FOCUS_MODE_CONTINUOUS_PICTURE_MACRO = (1 << 8), //!< \n
    };

    //! Metering
    enum {
        METERING_MODE_AVERAGE = (1 << 0), //!< \n
        METERING_MODE_CENTER  = (1 << 1), //!< \n
        METERING_MODE_MATRIX  = (1 << 2), //!< \n
        METERING_MODE_SPOT    = (1 << 3), //!< \n
    };

    //! Contrast
    enum {
        CONTRAST_AUTO    = (1 << 0), //!< \n
        CONTRAST_MINUS_2 = (1 << 1), //!< \n
        CONTRAST_MINUS_1 = (1 << 2), //!< \n
        CONTRAST_DEFAULT = (1 << 3), //!< \n
        CONTRAST_PLUS_1  = (1 << 4), //!< \n
        CONTRAST_PLUS_2  = (1 << 5), //!< \n
    };

    //! Sets camera angle
    bool            setAngle(int angle);

    //! Gets camera angle
    int             getAngle(void);

    //! Sets metering areas.
    bool            setMeteringMode(int value);
    //! Gets metering
    int             getMeteringMode(void);

    //! Sets Top-down mirror
    bool            setTopDownMirror(void);
    //! Sets Left-right mirror
    bool            setLRMirror(void);

    //! Sets brightness
    bool            setBrightness(int brightness);
    //! Gets brightness
    int             getBrightness(void);

    //! Sets ISO
    bool            setISO(int iso);
    //! Gets ISO
    int             getISO(void);

    //! Sets Contrast
    bool            setContrast(int value);
    //! Gets Contrast
    int             getContrast(void);

    //! Sets Saturation
    bool            setSaturation(int saturation);
    //! Gets Saturation
    int             getSaturation(void);

    //! Sets Sharpness
    bool            setSharpness(int sharpness);
    //! Gets Sharpness
    int             getSharpness(void);

    // ! Sets Hue
    bool            setHue(int hue);
    // ! Gets Hue
    int             getHue(void);

    //! Sets anti shake
    bool            setAntiShake(bool toggle);
    //! Gets anti shake
    bool            getAntiShake(void);

    //! Sets object tracking
    bool            setObjectTracking(bool toggle);
    //! Gets object tracking
    bool            getObjectTracking(void);
    //! Start or stop object tracking operation
    bool            setObjectTrackingStart(bool toggle);
    //! Gets status of object tracking operation
    int             getObjectTrackingStatus(void);
    //! Sets x, y position for object tracking operation
    bool            setObjectPosition(int x, int y);

    //! Start or stop the touch auto focus operation
    bool            setTouchAFStart(bool toggle);

    //! Sets gamma
    bool            setGamma(bool toggle);
    //! Gets gamma
    bool            getGamma(void);

    //! Sets ODC
    bool            setODC(bool toggle);
    //! Gets ODC
    bool            getODC(void);

    //! Sets Slow AE
    bool            setSlowAE(bool toggle);
    //! Gets Slow AE
    bool            getSlowAE(void);

    //! set callback need CSC
    void            setCallbackCSC(bool csc);
    //! Return callback need CSC
    bool            getCallbackCSC(void);

    //! Sets 3DNR
    bool            set3DNR(bool toggle);
    //! Gets 3DNR
    bool            get3DNR(void);

    //! Gets Illumination
    int             getIllumination(void);

    //! Sets VT mode
    bool            setVtMode(int vtMode);

    //! Sets Intelligent mode
    bool            setIntelligentMode(int intelligentMode);
    //! Gets Intelligent mode
    int             getIntelligentMode(void);

    //! Gets Fnumber
    bool            getFnumber(int *num, int *den);

    //! Gets Aperture value
    bool            getApertureValue(int *num, int *den);

    //! Gets FocalLengthIn35mmFilm
    int             getFocalLengthIn35mmFilm(void);

    //! Gets ImageUniqueId
    const char     *getImageUniqueId(void);

    bool            setAutoFocusMacroPosition(int autoFocusMacroPosition);

    ExynosBuffer   *searchSensorBuffer(unsigned int fcount);
    ExynosBuffer   *searchSensorBufferOnHal(unsigned int fcount);

#ifdef FRONT_NO_ZSL
    void            setFrontCaptureCmd(int captureCmd);
#endif
};

extern unsigned long measure_time_camera(struct timeval *start, struct timeval *stop);

#endif // EXYNOS_CAMERA_H__
