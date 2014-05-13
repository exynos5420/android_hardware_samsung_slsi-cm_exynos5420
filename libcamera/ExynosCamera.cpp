/*
 * Copyright 2012, Samsung Electronics Co. LTD
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed toggle an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*!
 * \file      ExynosCamera.cpp
 * \brief     source file for CAMERA HAL MODULE
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

//#define LOG_NDEBUG 0
#define LOG_TAG "ExynosCamera"

#include <cutils/log.h>

#include "ExynosCamera.h"

ExynosCameraInfo::ExynosCameraInfo()
{
    previewW = 2560;
    previewH = 1920;
    previewColorFormat = V4L2_PIX_FMT_NV21;
    previewBufPlane = 3;
    videoW = 1920;
    videoH = 1080;
    videoColorFormat = V4L2_PIX_FMT_NV12M;
    videoBufPlane = 3;
    pictureW = 2560;
    pictureH = 1920;
    pictureColorFormat = V4L2_PIX_FMT_YUYV;
    pictureBufPlane = 2;
    thumbnailW = 512;
    thumbnailH = 384;

    antiBandingList =
          ExynosCamera::ANTIBANDING_AUTO
        | ExynosCamera::ANTIBANDING_50HZ
        | ExynosCamera::ANTIBANDING_60HZ
        | ExynosCamera::ANTIBANDING_OFF
        ;
    antiBanding = ExynosCamera::ANTIBANDING_AUTO;

    effectList =
          ExynosCamera::EFFECT_NONE
        | ExynosCamera::EFFECT_MONO
        | ExynosCamera::EFFECT_NEGATIVE
        | ExynosCamera::EFFECT_SOLARIZE
        | ExynosCamera::EFFECT_SEPIA
        | ExynosCamera::EFFECT_POSTERIZE
        | ExynosCamera::EFFECT_WHITEBOARD
        | ExynosCamera::EFFECT_BLACKBOARD
        | ExynosCamera::EFFECT_AQUA;
    effect = ExynosCamera::EFFECT_NONE;

    flashModeList =
          ExynosCamera::FLASH_MODE_OFF
        | ExynosCamera::FLASH_MODE_AUTO
        | ExynosCamera::FLASH_MODE_ON
        | ExynosCamera::FLASH_MODE_RED_EYE
        | ExynosCamera::FLASH_MODE_TORCH;
    flashMode = ExynosCamera::FLASH_MODE_OFF;
    flashPreMode = ExynosCamera::FLASH_MODE_OFF;

    focusModeList =
          ExynosCamera::FOCUS_MODE_AUTO
        | ExynosCamera::FOCUS_MODE_INFINITY
        | ExynosCamera::FOCUS_MODE_MACRO
        | ExynosCamera::FOCUS_MODE_FIXED
        | ExynosCamera::FOCUS_MODE_EDOF
        | ExynosCamera::FOCUS_MODE_CONTINUOUS_VIDEO
        | ExynosCamera::FOCUS_MODE_CONTINUOUS_PICTURE
        | ExynosCamera::FOCUS_MODE_TOUCH;
    focusMode = ExynosCamera::FOCUS_MODE_AUTO;

    sceneModeList =
          ExynosCamera::SCENE_MODE_AUTO
        | ExynosCamera::SCENE_MODE_ACTION
        | ExynosCamera::SCENE_MODE_PORTRAIT
        | ExynosCamera::SCENE_MODE_LANDSCAPE
        | ExynosCamera::SCENE_MODE_NIGHT
        | ExynosCamera::SCENE_MODE_NIGHT_PORTRAIT
        | ExynosCamera::SCENE_MODE_THEATRE
        | ExynosCamera::SCENE_MODE_BEACH
        | ExynosCamera::SCENE_MODE_SNOW
        | ExynosCamera::SCENE_MODE_SUNSET
        | ExynosCamera::SCENE_MODE_STEADYPHOTO
        | ExynosCamera::SCENE_MODE_FIREWORKS
        | ExynosCamera::SCENE_MODE_SPORTS
        | ExynosCamera::SCENE_MODE_PARTY
        | ExynosCamera::SCENE_MODE_CANDLELIGHT;
    sceneMode = ExynosCamera::SCENE_MODE_AUTO;

    whiteBalanceList =
          ExynosCamera::WHITE_BALANCE_AUTO
        | ExynosCamera::WHITE_BALANCE_INCANDESCENT
        | ExynosCamera::WHITE_BALANCE_FLUORESCENT
        | ExynosCamera::WHITE_BALANCE_WARM_FLUORESCENT
        | ExynosCamera::WHITE_BALANCE_DAYLIGHT
        | ExynosCamera::WHITE_BALANCE_CLOUDY_DAYLIGHT
        | ExynosCamera::WHITE_BALANCE_TWILIGHT
        | ExynosCamera::WHITE_BALANCE_SHADE;
    whiteBalance = ExynosCamera::WHITE_BALANCE_AUTO;

    autoWhiteBalanceLockSupported = false;
    autoWhiteBalanceLock = false;

    rotation = 0;
    minExposure = -4;
    maxExposure = 4;
    exposure = 0;

    autoExposureLockSupported = false;
    autoExposureLock = false;

    fpsRange[0] =  1000;
    fpsRange[1] = 30000;

    fNumberNum = 12;
    fNumberDen = 10;
    focalLengthNum = 343;
    focalLengthDen = 100;
    apertureNum = 27;
    apertureDen = 10;
    supportVideoStabilization = false;
    applyVideoStabilization = false;
    videoStabilization = false;
    maxNumMeteringAreas = 0;
    maxNumDetectedFaces = 0;
    maxNumFocusAreas = 0;
    maxZoom = 0;
    hwZoomSupported = false;
    zoom = 0;
    gpsAltitude = 0;
    gpsLatitude = 0;
    gpsLongitude = 0;
    gpsTimestamp = 0;

    // Additional API default Value.
    angle = 0;
    antiShake = false;
    brightness = 0;
    contrast = ExynosCamera::CONTRAST_DEFAULT;
    gamma = false;
    hue = 2; // 2 is default;
    iso = 0;
    metering = ExynosCamera::METERING_MODE_CENTER;
    objectTracking = false;
    objectTrackingStart = false;
    saturation = 0;
    sharpness = 0;
    slowAE = false;
    touchAfStart = false;
    tdnr = false;
    odc = false;
    vtMode = 0;
    intelligentMode = 0;
#ifdef SCALABLE_SENSOR
    scalableSensorStart = false;
#endif
    focalLengthIn35mmLength = 31;
    autoFocusMacroPosition = 0;
    ispW = 0;
    ispH = 0;
}

ExynosCameraInfoM5M0::ExynosCameraInfoM5M0()
{
    previewW = 1280;
    previewH = 720;
    previewColorFormat = V4L2_PIX_FMT_YVU420M;
    previewBufPlane = 4;
    videoW = 1280;
    videoH = 720;
    videoColorFormat = V4L2_PIX_FMT_NV12M;
    videoBufPlane = 3;
    pictureW = 1280;
    pictureH = 720;
    pictureColorFormat = V4L2_PIX_FMT_YUYV;
    pictureBufPlane = 2;
    thumbnailW = 512;
    thumbnailH = 384;

    antiBandingList = ExynosCamera::ANTIBANDING_OFF;
    antiBanding = ExynosCamera::ANTIBANDING_OFF;

    effectList =
          ExynosCamera::EFFECT_NONE
        | ExynosCamera::EFFECT_MONO
        | ExynosCamera::EFFECT_NEGATIVE
        //| ExynosCamera::EFFECT_SOLARIZE
        | ExynosCamera::EFFECT_SEPIA
        //| ExynosCamera::EFFECT_POSTERIZE
        //| ExynosCamera::EFFECT_WHITEBOARD
        //| ExynosCamera::EFFECT_BLACKBOARD
        | ExynosCamera::EFFECT_AQUA;
    effect = ExynosCamera::EFFECT_NONE;

    flashModeList =
          ExynosCamera::FLASH_MODE_OFF
        | ExynosCamera::FLASH_MODE_AUTO
        | ExynosCamera::FLASH_MODE_ON
        | ExynosCamera::FLASH_MODE_RED_EYE
        | ExynosCamera::FLASH_MODE_TORCH;
    flashMode = ExynosCamera::FLASH_MODE_OFF;

    focusModeList =
          ExynosCamera::FOCUS_MODE_AUTO
        | ExynosCamera::FOCUS_MODE_INFINITY
        | ExynosCamera::FOCUS_MODE_MACRO
        //| ExynosCamera::FOCUS_MODE_FIXED
        //| ExynosCamera::FOCUS_MODE_EDOF
        //| ExynosCamera::FOCUS_MODE_CONTINUOUS_VIDEO
        //| ExynosCamera::FOCUS_MODE_CONTINUOUS_PICTURE
        //| ExynosCamera::FOCUS_MODE_TOUCH
        ;
    focusMode = ExynosCamera::FOCUS_MODE_AUTO;

    sceneModeList =
          ExynosCamera::SCENE_MODE_AUTO
        //| ExynosCamera::SCENE_MODE_ACTION
        | ExynosCamera::SCENE_MODE_PORTRAIT
        | ExynosCamera::SCENE_MODE_LANDSCAPE
        | ExynosCamera::SCENE_MODE_NIGHT
        //| ExynosCamera::SCENE_MODE_NIGHT_PORTRAIT
        //| ExynosCamera::SCENE_MODE_THEATRE
        | ExynosCamera::SCENE_MODE_BEACH
        | ExynosCamera::SCENE_MODE_SNOW
        | ExynosCamera::SCENE_MODE_SUNSET
        //| ExynosCamera::SCENE_MODE_STEADYPHOTO
        | ExynosCamera::SCENE_MODE_FIREWORKS
        | ExynosCamera::SCENE_MODE_SPORTS
        | ExynosCamera::SCENE_MODE_PARTY
        | ExynosCamera::SCENE_MODE_CANDLELIGHT;
    sceneMode = ExynosCamera::SCENE_MODE_AUTO;

    whiteBalanceList =
          ExynosCamera::WHITE_BALANCE_AUTO
        | ExynosCamera::WHITE_BALANCE_INCANDESCENT
        | ExynosCamera::WHITE_BALANCE_FLUORESCENT
        //| ExynosCamera::WHITE_BALANCE_WARM_FLUORESCENT
        | ExynosCamera::WHITE_BALANCE_DAYLIGHT
        | ExynosCamera::WHITE_BALANCE_CLOUDY_DAYLIGHT
        //| ExynosCamera::WHITE_BALANCE_TWILIGHT
        //| ExynosCamera::WHITE_BALANCE_SHADE
        ;
    whiteBalance = ExynosCamera::WHITE_BALANCE_AUTO;

    autoWhiteBalanceLockSupported = false;
    autoWhiteBalanceLock = false;

    rotation = 0;
    minExposure = -4;
    maxExposure = 4;
    exposure = 0;

    autoExposureLockSupported = false;
    autoExposureLock = false;

    fpsRange[0] =  1000;
    fpsRange[1] = 30000;

    fNumberNum = 12;
    fNumberDen = 10;
    focalLengthNum = 343;
    focalLengthDen = 100;
    apertureNum = 27;
    apertureDen = 10;
    supportVideoStabilization = false;
    applyVideoStabilization = false;
    videoStabilization = false;
    maxNumMeteringAreas = 64;
    maxNumDetectedFaces = 16;
    maxNumFocusAreas = 2;
    maxZoom = ZOOM_LEVEL_MAX;
    hwZoomSupported = false;
    zoom = 0;
    gpsAltitude = 0;
    gpsLatitude = 0;
    gpsLongitude = 0;
    gpsTimestamp = 0;
}

ExynosCameraInfoS5K3H5::ExynosCameraInfoS5K3H5()
{
//    previewW = 1920;
//    previewH = 1080;
//    previewW = 1920;
//    previewH = 1088;
    previewW = 1024;
    previewH = 768;

//    previewColorFormat = V4L2_PIX_FMT_YVU420M;
    previewColorFormat = V4L2_PIX_FMT_YUV420M;
    previewBufPlane = 4;
//    videoW = 1920;
//    videoH = 1080;
//    videoW = 1920;
//    videoH = 1088;
    videoW = 1024;
    videoH = 768;
    videoColorFormat = V4L2_PIX_FMT_NV12M;
    videoBufPlane = 3;
    pictureW = 3200;
    pictureH = 2400;
    pictureColorFormat = V4L2_PIX_FMT_YUYV;
    pictureBufPlane = 2;
    thumbnailW = 512;
    thumbnailH = 384;

    antiBandingList =
          ExynosCamera::ANTIBANDING_AUTO
        | ExynosCamera::ANTIBANDING_50HZ
        | ExynosCamera::ANTIBANDING_60HZ
        | ExynosCamera::ANTIBANDING_OFF
        ;
    antiBanding = ExynosCamera::ANTIBANDING_AUTO;

    effectList =
          ExynosCamera::EFFECT_NONE
        | ExynosCamera::EFFECT_MONO
        | ExynosCamera::EFFECT_NEGATIVE
        //| ExynosCamera::EFFECT_SOLARIZE
        | ExynosCamera::EFFECT_SEPIA
        //| ExynosCamera::EFFECT_POSTERIZE
        //| ExynosCamera::EFFECT_WHITEBOARD
        //| ExynosCamera::EFFECT_BLACKBOARD
        //| ExynosCamera::EFFECT_AQUA
        ;
    effect = ExynosCamera::EFFECT_NONE;

    flashModeList =
          ExynosCamera::FLASH_MODE_OFF
        | ExynosCamera::FLASH_MODE_AUTO
        | ExynosCamera::FLASH_MODE_ON
        //| ExynosCamera::FLASH_MODE_RED_EYE
        | ExynosCamera::FLASH_MODE_TORCH;
    flashMode = ExynosCamera::FLASH_MODE_OFF;

    focusModeList =
          ExynosCamera::FOCUS_MODE_AUTO
        | ExynosCamera::FOCUS_MODE_INFINITY
        | ExynosCamera::FOCUS_MODE_MACRO
        //| ExynosCamera::FOCUS_MODE_FIXED
        //| ExynosCamera::FOCUS_MODE_EDOF
        | ExynosCamera::FOCUS_MODE_CONTINUOUS_VIDEO
        | ExynosCamera::FOCUS_MODE_CONTINUOUS_PICTURE
        | ExynosCamera::FOCUS_MODE_TOUCH
        ;
    focusMode = ExynosCamera::FOCUS_MODE_AUTO;

    sceneModeList =
          ExynosCamera::SCENE_MODE_AUTO
        //| ExynosCamera::SCENE_MODE_ACTION
        | ExynosCamera::SCENE_MODE_PORTRAIT
        | ExynosCamera::SCENE_MODE_LANDSCAPE
        | ExynosCamera::SCENE_MODE_NIGHT
        //| ExynosCamera::SCENE_MODE_NIGHT_PORTRAIT
        //| ExynosCamera::SCENE_MODE_THEATRE
        | ExynosCamera::SCENE_MODE_BEACH
        | ExynosCamera::SCENE_MODE_SNOW
        | ExynosCamera::SCENE_MODE_SUNSET
        | ExynosCamera::SCENE_MODE_STEADYPHOTO
        | ExynosCamera::SCENE_MODE_FIREWORKS
        | ExynosCamera::SCENE_MODE_SPORTS
        | ExynosCamera::SCENE_MODE_PARTY
        | ExynosCamera::SCENE_MODE_CANDLELIGHT;
    sceneMode = ExynosCamera::SCENE_MODE_AUTO;

    whiteBalanceList =
          ExynosCamera::WHITE_BALANCE_AUTO
        | ExynosCamera::WHITE_BALANCE_INCANDESCENT
        | ExynosCamera::WHITE_BALANCE_FLUORESCENT
        //| ExynosCamera::WHITE_BALANCE_WARM_FLUORESCENT
        | ExynosCamera::WHITE_BALANCE_DAYLIGHT
        | ExynosCamera::WHITE_BALANCE_CLOUDY_DAYLIGHT
        //| ExynosCamera::WHITE_BALANCE_TWILIGHT
        //| ExynosCamera::WHITE_BALANCE_SHADE
        ;
    whiteBalance = ExynosCamera::WHITE_BALANCE_AUTO;

    autoWhiteBalanceLockSupported = true;
    autoWhiteBalanceLock = false;

    rotation = 0;
    minExposure = -4;
    maxExposure = 4;
    exposure = 0;

    autoExposureLockSupported = true;
    autoExposureLock = false;

    fpsRange[0] =  1000;
    fpsRange[1] = 30000;

    fNumberNum = 26;
    fNumberDen = 10;
    focalLengthNum = 9;
    focalLengthDen = 10;
    apertureNum = 27;
    apertureDen = 10;
    supportVideoStabilization = true;
    applyVideoStabilization = false;
    videoStabilization = false;
    maxNumMeteringAreas = 64;
    maxNumDetectedFaces = 16;
    maxNumFocusAreas = 2;
    maxZoom = ZOOM_LEVEL_MAX;
    hwZoomSupported = false;
    zoom = 0;
    gpsAltitude = 0;
    gpsLatitude = 0;
    gpsLongitude = 0;
    gpsTimestamp = 0;
}

ExynosCameraInfoS5K3H7::ExynosCameraInfoS5K3H7()
{
    previewW = 1920;
    previewH = 1080;
    previewColorFormat = V4L2_PIX_FMT_YVU420M;
    previewBufPlane = 4;
    videoW = 1920;
    videoH = 1080;
    videoColorFormat = V4L2_PIX_FMT_NV12M;
    videoBufPlane = 3;
    pictureW = 3200;
    pictureH = 2400;
    pictureColorFormat = V4L2_PIX_FMT_YUYV;
    pictureBufPlane = 2;
    thumbnailW = 512;
    thumbnailH = 384;

    antiBandingList =
          ExynosCamera::ANTIBANDING_AUTO
        | ExynosCamera::ANTIBANDING_50HZ
        | ExynosCamera::ANTIBANDING_60HZ
        | ExynosCamera::ANTIBANDING_OFF
        ;
    antiBanding = ExynosCamera::ANTIBANDING_AUTO;

    effectList =
          ExynosCamera::EFFECT_NONE
        | ExynosCamera::EFFECT_MONO
        | ExynosCamera::EFFECT_NEGATIVE
        //| ExynosCamera::EFFECT_SOLARIZE
        | ExynosCamera::EFFECT_SEPIA
        //| ExynosCamera::EFFECT_POSTERIZE
        //| ExynosCamera::EFFECT_WHITEBOARD
        //| ExynosCamera::EFFECT_BLACKBOARD
        //| ExynosCamera::EFFECT_AQUA
        ;
    effect = ExynosCamera::EFFECT_NONE;

    flashModeList =
          ExynosCamera::FLASH_MODE_OFF
        | ExynosCamera::FLASH_MODE_AUTO
        | ExynosCamera::FLASH_MODE_ON
        //| ExynosCamera::FLASH_MODE_RED_EYE
        | ExynosCamera::FLASH_MODE_TORCH;
    flashMode = ExynosCamera::FLASH_MODE_OFF;

    focusModeList =
          ExynosCamera::FOCUS_MODE_AUTO
        | ExynosCamera::FOCUS_MODE_INFINITY
        | ExynosCamera::FOCUS_MODE_MACRO
        //| ExynosCamera::FOCUS_MODE_FIXED
        //| ExynosCamera::FOCUS_MODE_EDOF
        | ExynosCamera::FOCUS_MODE_CONTINUOUS_VIDEO
        | ExynosCamera::FOCUS_MODE_CONTINUOUS_PICTURE
        | ExynosCamera::FOCUS_MODE_TOUCH
        ;
    focusMode = ExynosCamera::FOCUS_MODE_AUTO;

    sceneModeList =
          ExynosCamera::SCENE_MODE_AUTO
        //| ExynosCamera::SCENE_MODE_ACTION
        | ExynosCamera::SCENE_MODE_PORTRAIT
        | ExynosCamera::SCENE_MODE_LANDSCAPE
        | ExynosCamera::SCENE_MODE_NIGHT
        //| ExynosCamera::SCENE_MODE_NIGHT_PORTRAIT
        //| ExynosCamera::SCENE_MODE_THEATRE
        | ExynosCamera::SCENE_MODE_BEACH
        | ExynosCamera::SCENE_MODE_SNOW
        | ExynosCamera::SCENE_MODE_SUNSET
        | ExynosCamera::SCENE_MODE_STEADYPHOTO
        | ExynosCamera::SCENE_MODE_FIREWORKS
        | ExynosCamera::SCENE_MODE_SPORTS
        | ExynosCamera::SCENE_MODE_PARTY
        | ExynosCamera::SCENE_MODE_CANDLELIGHT;
    sceneMode = ExynosCamera::SCENE_MODE_AUTO;

    whiteBalanceList =
          ExynosCamera::WHITE_BALANCE_AUTO
        | ExynosCamera::WHITE_BALANCE_INCANDESCENT
        | ExynosCamera::WHITE_BALANCE_FLUORESCENT
        //| ExynosCamera::WHITE_BALANCE_WARM_FLUORESCENT
        | ExynosCamera::WHITE_BALANCE_DAYLIGHT
        | ExynosCamera::WHITE_BALANCE_CLOUDY_DAYLIGHT
        //| ExynosCamera::WHITE_BALANCE_TWILIGHT
        //| ExynosCamera::WHITE_BALANCE_SHADE
        ;
    whiteBalance = ExynosCamera::WHITE_BALANCE_AUTO;

    autoWhiteBalanceLockSupported = true;
    autoWhiteBalanceLock = false;

    rotation = 0;
    minExposure = -4;
    maxExposure = 4;
    exposure = 0;

    autoExposureLockSupported = true;
    autoExposureLock = false;

    fpsRange[0] =  1000;
    fpsRange[1] = 30000;

    fNumberNum = 26;
    fNumberDen = 10;
    focalLengthNum = 9;
    focalLengthDen = 10;
    apertureNum = 27;
    apertureDen = 10;
    supportVideoStabilization = true;
    applyVideoStabilization = false;
    videoStabilization = false;
    maxNumMeteringAreas = 64;
    maxNumDetectedFaces = 16;
    maxNumFocusAreas = 2;
    maxZoom = ZOOM_LEVEL_MAX;
    hwZoomSupported = false;
    zoom = 0;
    gpsAltitude = 0;
    gpsLatitude = 0;
    gpsLongitude = 0;
    gpsTimestamp = 0;
}

ExynosCameraInfoS5K6A3::ExynosCameraInfoS5K6A3()
{
    previewW = 1280;
    previewH = 960;
    previewColorFormat = V4L2_PIX_FMT_YVU420M;
    previewBufPlane = 4;
    videoW = 1280;
    videoH =  720;
    videoColorFormat = V4L2_PIX_FMT_NV12M;
    videoBufPlane = 3;
    pictureW = 1392;
    pictureH = 1392;
    pictureColorFormat = V4L2_PIX_FMT_YUYV;
    pictureBufPlane = 2;
    thumbnailW = 512;
    thumbnailH = 384;

    antiBandingList =
          ExynosCamera::ANTIBANDING_AUTO
        | ExynosCamera::ANTIBANDING_50HZ
        | ExynosCamera::ANTIBANDING_60HZ
        | ExynosCamera::ANTIBANDING_OFF
        ;
    antiBanding = ExynosCamera::ANTIBANDING_AUTO;

    effectList =
          ExynosCamera::EFFECT_NONE
        | ExynosCamera::EFFECT_MONO
        | ExynosCamera::EFFECT_NEGATIVE
        | ExynosCamera::EFFECT_SOLARIZE
        | ExynosCamera::EFFECT_SEPIA
        | ExynosCamera::EFFECT_POSTERIZE
        | ExynosCamera::EFFECT_WHITEBOARD
        | ExynosCamera::EFFECT_BLACKBOARD
        | ExynosCamera::EFFECT_AQUA
        ;
    effect = ExynosCamera::EFFECT_NONE;

    flashModeList =
          ExynosCamera::FLASH_MODE_OFF
        //| ExynosCamera::FLASH_MODE_AUTO
        //| ExynosCamera::FLASH_MODE_ON
        //| ExynosCamera::FLASH_MODE_RED_EYE
        //| ExynosCamera::FLASH_MODE_TORCH
        ;
    flashMode = ExynosCamera::FLASH_MODE_OFF;

    focusModeList =
        //  ExynosCamera::FOCUS_MODE_AUTO
          ExynosCamera::FOCUS_MODE_INFINITY
        //| ExynosCamera::FOCUS_MODE_MACRO
        //| ExynosCamera::FOCUS_MODE_FIXED
        //| ExynosCamera::FOCUS_MODE_EDOF
        //| ExynosCamera::FOCUS_MODE_CONTINUOUS_VIDEO
        //| ExynosCamera::FOCUS_MODE_CONTINUOUS_PICTURE
        //| ExynosCamera::FOCUS_MODE_TOUCH
        ;
    focusMode = ExynosCamera::FOCUS_MODE_INFINITY;

    sceneModeList =
          ExynosCamera::SCENE_MODE_AUTO
        | ExynosCamera::SCENE_MODE_ACTION
        | ExynosCamera::SCENE_MODE_PORTRAIT
        | ExynosCamera::SCENE_MODE_LANDSCAPE
        | ExynosCamera::SCENE_MODE_NIGHT
        | ExynosCamera::SCENE_MODE_NIGHT_PORTRAIT
        | ExynosCamera::SCENE_MODE_THEATRE
        | ExynosCamera::SCENE_MODE_BEACH
        | ExynosCamera::SCENE_MODE_SNOW
        | ExynosCamera::SCENE_MODE_SUNSET
        | ExynosCamera::SCENE_MODE_STEADYPHOTO
        | ExynosCamera::SCENE_MODE_FIREWORKS
        | ExynosCamera::SCENE_MODE_SPORTS
        | ExynosCamera::SCENE_MODE_PARTY
        | ExynosCamera::SCENE_MODE_CANDLELIGHT;
    sceneMode = ExynosCamera::SCENE_MODE_AUTO;

    whiteBalanceList =
          ExynosCamera::WHITE_BALANCE_AUTO
        | ExynosCamera::WHITE_BALANCE_INCANDESCENT
        | ExynosCamera::WHITE_BALANCE_FLUORESCENT
        //| ExynosCamera::WHITE_BALANCE_WARM_FLUORESCENT
        | ExynosCamera::WHITE_BALANCE_DAYLIGHT
        | ExynosCamera::WHITE_BALANCE_CLOUDY_DAYLIGHT
        //| ExynosCamera::WHITE_BALANCE_TWILIGHT
        //| ExynosCamera::WHITE_BALANCE_SHADE
        ;
    whiteBalance = ExynosCamera::WHITE_BALANCE_AUTO;

    autoWhiteBalanceLockSupported = true;
    autoWhiteBalanceLock = false;

    rotation = 0;
    minExposure = -4;
    maxExposure = 4;
    exposure = 0;

    autoExposureLockSupported = true;
    autoExposureLock = false;

    fpsRange[0] =  1000;
    fpsRange[1] = 30000;

    fNumberNum = 245;
    fNumberDen = 100;
    focalLengthNum = 185;
    focalLengthDen = 100;
    apertureNum = 75;
    apertureDen = 100;
    supportVideoStabilization = false;
    applyVideoStabilization = false;
    videoStabilization = false;
    maxNumMeteringAreas = 64;
    maxNumDetectedFaces = 16;
    maxNumFocusAreas = 0;
    maxZoom = ZOOM_LEVEL_MAX;
    hwZoomSupported = false;
    zoom = 0;
    gpsAltitude = 0;
    gpsLatitude = 0;
    gpsLongitude = 0;
    gpsTimestamp = 0;
}

ExynosCameraInfoS5K6B2::ExynosCameraInfoS5K6B2()
{
    previewW = 1920;
    previewH = 1080;
    previewColorFormat = V4L2_PIX_FMT_NV21M;
    previewBufPlane = 3;
    videoW = 1920;
    videoH = 1080;
    videoColorFormat = V4L2_PIX_FMT_NV12M;
    videoBufPlane = 3;
    pictureW = 1920;
    pictureH = 1080;
    pictureColorFormat = V4L2_PIX_FMT_YUYV;
    pictureBufPlane = 2;
    ispW = 1920;
    ispH = 1080;
    thumbnailW = 512;
    thumbnailH = 384;

    antiBandingList =
          ExynosCamera::ANTIBANDING_AUTO
        | ExynosCamera::ANTIBANDING_50HZ
        | ExynosCamera::ANTIBANDING_60HZ
        | ExynosCamera::ANTIBANDING_OFF
        ;
    antiBanding = ExynosCamera::ANTIBANDING_AUTO;

    effectList =
          ExynosCamera::EFFECT_NONE
        | ExynosCamera::EFFECT_MONO
        | ExynosCamera::EFFECT_NEGATIVE
        /* | ExynosCamera::EFFECT_SOLARIZE */
        | ExynosCamera::EFFECT_SEPIA
        | ExynosCamera::EFFECT_POSTERIZE
        /* | ExynosCamera::EFFECT_WHITEBOARD */
        /* | ExynosCamera::EFFECT_BLACKBOARD */
        | ExynosCamera::EFFECT_AQUA
        ;
    effect = ExynosCamera::EFFECT_NONE;

    flashModeList =
          ExynosCamera::FLASH_MODE_OFF
        //| ExynosCamera::FLASH_MODE_AUTO
        //| ExynosCamera::FLASH_MODE_ON
        //| ExynosCamera::FLASH_MODE_RED_EYE
        //| ExynosCamera::FLASH_MODE_TORCH
        ;
    flashMode = ExynosCamera::FLASH_MODE_OFF;

    focusModeList =
        //  ExynosCamera::FOCUS_MODE_AUTO
          ExynosCamera::FOCUS_MODE_INFINITY
        //| ExynosCamera::FOCUS_MODE_MACRO
        //| ExynosCamera::FOCUS_MODE_FIXED
        //| ExynosCamera::FOCUS_MODE_EDOF
        //| ExynosCamera::FOCUS_MODE_CONTINUOUS_VIDEO
        //| ExynosCamera::FOCUS_MODE_CONTINUOUS_PICTURE
        //| ExynosCamera::FOCUS_MODE_TOUCH
        ;
    focusMode = ExynosCamera::FOCUS_MODE_INFINITY;

    sceneModeList =
          ExynosCamera::SCENE_MODE_AUTO
        | ExynosCamera::SCENE_MODE_ACTION
        | ExynosCamera::SCENE_MODE_PORTRAIT
        | ExynosCamera::SCENE_MODE_LANDSCAPE
        | ExynosCamera::SCENE_MODE_NIGHT
        | ExynosCamera::SCENE_MODE_NIGHT_PORTRAIT
        | ExynosCamera::SCENE_MODE_THEATRE
        | ExynosCamera::SCENE_MODE_BEACH
        | ExynosCamera::SCENE_MODE_SNOW
        | ExynosCamera::SCENE_MODE_SUNSET
        | ExynosCamera::SCENE_MODE_STEADYPHOTO
        | ExynosCamera::SCENE_MODE_FIREWORKS
        | ExynosCamera::SCENE_MODE_SPORTS
        | ExynosCamera::SCENE_MODE_PARTY
        | ExynosCamera::SCENE_MODE_CANDLELIGHT
        ;
    sceneMode = ExynosCamera::SCENE_MODE_AUTO;

    whiteBalanceList =
          ExynosCamera::WHITE_BALANCE_AUTO
        | ExynosCamera::WHITE_BALANCE_INCANDESCENT
        | ExynosCamera::WHITE_BALANCE_FLUORESCENT
        //| ExynosCamera::WHITE_BALANCE_WARM_FLUORESCENT
        | ExynosCamera::WHITE_BALANCE_DAYLIGHT
        | ExynosCamera::WHITE_BALANCE_CLOUDY_DAYLIGHT
        //| ExynosCamera::WHITE_BALANCE_TWILIGHT
        //| ExynosCamera::WHITE_BALANCE_SHADE
        ;
    whiteBalance = ExynosCamera::WHITE_BALANCE_AUTO;

    autoWhiteBalanceLockSupported = true;
    autoWhiteBalanceLock = false;

    rotation = 0;
    minExposure = -4;
    maxExposure = 4;
    exposure = 0;

    autoExposureLockSupported = true;
    autoExposureLock = false;

    fpsRange[0] =  4000;
    fpsRange[1] = 30000;

    fNumberNum = 245;
    fNumberDen = 100;
    focalLengthNum = 185;
    focalLengthDen = 100;
    apertureNum = 245;
    apertureDen = 100;
    supportVideoStabilization = false;
    applyVideoStabilization = false;
    videoStabilization = false;
    maxNumMeteringAreas = 32;
    maxNumDetectedFaces = 16;
    maxNumFocusAreas = 0;
    maxZoom = ZOOM_LEVEL_MAX;
    hwZoomSupported = false;
    zoom = 0;
    gpsAltitude = 0;
    gpsLatitude = 0;
    gpsLongitude = 0;
    gpsTimestamp = 0;

    /* Additional API default Value. */
    metering = ExynosCamera::METERING_MODE_CENTER;
    focalLengthIn35mmLength = 27;
}

ExynosCameraInfoS5K4E5::ExynosCameraInfoS5K4E5()
{
    previewW = 1920;
    previewH = 1080;
    previewColorFormat = V4L2_PIX_FMT_NV21M;
    previewBufPlane = 3;
    videoW = 1920;
    videoH = 1080;
    videoColorFormat = V4L2_PIX_FMT_NV12M;
    videoBufPlane = 3;
    pictureW = 2560;
    pictureH = 1920;
    pictureColorFormat = V4L2_PIX_FMT_YUYV;
    pictureBufPlane = 2;
    thumbnailW = 512;
    thumbnailH = 384;
    ispW = 1920;
    ispH = 1440;

    antiBandingList =
          ExynosCamera::ANTIBANDING_AUTO
        | ExynosCamera::ANTIBANDING_50HZ
        | ExynosCamera::ANTIBANDING_60HZ
        | ExynosCamera::ANTIBANDING_OFF
        ;
    antiBanding = ExynosCamera::ANTIBANDING_AUTO;

    effectList =
          ExynosCamera::EFFECT_NONE
        | ExynosCamera::EFFECT_MONO
        | ExynosCamera::EFFECT_NEGATIVE
        //| ExynosCamera::EFFECT_SOLARIZE
        | ExynosCamera::EFFECT_SEPIA
        //| ExynosCamera::EFFECT_POSTERIZE
        //| ExynosCamera::EFFECT_WHITEBOARD
        //| ExynosCamera::EFFECT_BLACKBOARD
        //| ExynosCamera::EFFECT_AQUA
        ;
    effect = ExynosCamera::EFFECT_NONE;

    flashModeList =
          ExynosCamera::FLASH_MODE_OFF
        | ExynosCamera::FLASH_MODE_AUTO
        | ExynosCamera::FLASH_MODE_ON
        //| ExynosCamera::FLASH_MODE_RED_EYE
        | ExynosCamera::FLASH_MODE_TORCH;
    flashMode = ExynosCamera::FLASH_MODE_OFF;

    focusModeList =
          ExynosCamera::FOCUS_MODE_AUTO
        | ExynosCamera::FOCUS_MODE_INFINITY
        | ExynosCamera::FOCUS_MODE_MACRO
        //| ExynosCamera::FOCUS_MODE_FIXED
        //| ExynosCamera::FOCUS_MODE_EDOF
        | ExynosCamera::FOCUS_MODE_CONTINUOUS_VIDEO
        | ExynosCamera::FOCUS_MODE_CONTINUOUS_PICTURE
        | ExynosCamera::FOCUS_MODE_TOUCH
        | ExynosCamera::FOCUS_MODE_CONTINUOUS_PICTURE_MACRO
        ;
    focusMode = ExynosCamera::FOCUS_MODE_AUTO;

    sceneModeList =
          ExynosCamera::SCENE_MODE_AUTO
        //| ExynosCamera::SCENE_MODE_ACTION
        | ExynosCamera::SCENE_MODE_PORTRAIT
        | ExynosCamera::SCENE_MODE_LANDSCAPE
        | ExynosCamera::SCENE_MODE_NIGHT
        //| ExynosCamera::SCENE_MODE_NIGHT_PORTRAIT
        //| ExynosCamera::SCENE_MODE_THEATRE
        | ExynosCamera::SCENE_MODE_BEACH
        | ExynosCamera::SCENE_MODE_SNOW
        | ExynosCamera::SCENE_MODE_SUNSET
        | ExynosCamera::SCENE_MODE_STEADYPHOTO
        | ExynosCamera::SCENE_MODE_FIREWORKS
        | ExynosCamera::SCENE_MODE_SPORTS
        | ExynosCamera::SCENE_MODE_PARTY
        | ExynosCamera::SCENE_MODE_CANDLELIGHT;
    sceneMode = ExynosCamera::SCENE_MODE_AUTO;

    whiteBalanceList =
          ExynosCamera::WHITE_BALANCE_AUTO
        | ExynosCamera::WHITE_BALANCE_INCANDESCENT
        | ExynosCamera::WHITE_BALANCE_FLUORESCENT
        //| ExynosCamera::WHITE_BALANCE_WARM_FLUORESCENT
        | ExynosCamera::WHITE_BALANCE_DAYLIGHT
        | ExynosCamera::WHITE_BALANCE_CLOUDY_DAYLIGHT
        //| ExynosCamera::WHITE_BALANCE_TWILIGHT
        //| ExynosCamera::WHITE_BALANCE_SHADE
        ;
    whiteBalance = ExynosCamera::WHITE_BALANCE_AUTO;

    autoWhiteBalanceLockSupported = true;
    autoWhiteBalanceLock = false;

    rotation = 0;
    minExposure = -4;
    maxExposure = 4;
    exposure = 0;

    autoExposureLockSupported = true;
    autoExposureLock = false;

    fpsRange[0] =  1000;
    fpsRange[1] = 30000;

    fNumberNum = 12;
    fNumberDen = 10;
    focalLengthNum = 343;
    focalLengthDen = 100;
    apertureNum = 27;
    apertureDen = 10;
    supportVideoStabilization = true;
    applyVideoStabilization = false;
    videoStabilization = false;
    maxNumMeteringAreas = 64;
    maxNumDetectedFaces = 16;
    maxNumFocusAreas = 2;
    maxZoom = ZOOM_LEVEL_MAX;
    hwZoomSupported = false;
    zoom = 0;
    gpsAltitude = 0;
    gpsLatitude = 0;
    gpsLongitude = 0;
    gpsTimestamp = 0;
}

ExynosCameraInfoIMX135::ExynosCameraInfoIMX135()
{
    previewW = 1920;
    previewH = 1080;
    previewColorFormat = V4L2_PIX_FMT_NV21M;
    previewBufPlane = 3;
    videoW = 1920;
    videoH = 1080;
    ispW = 1920;
    ispH = 1080;
    videoColorFormat = V4L2_PIX_FMT_NV12M;
    videoBufPlane = 3;
    pictureW = 4128;
    pictureH = 3096;
    pictureColorFormat = V4L2_PIX_FMT_YUYV;
    pictureBufPlane = 2;
    thumbnailW = 512;
    thumbnailH = 384;

    antiBandingList =
          ExynosCamera::ANTIBANDING_AUTO
        | ExynosCamera::ANTIBANDING_50HZ
        | ExynosCamera::ANTIBANDING_60HZ
        | ExynosCamera::ANTIBANDING_OFF
        ;
    antiBanding = ExynosCamera::ANTIBANDING_AUTO;

    effectList =
          ExynosCamera::EFFECT_NONE
        | ExynosCamera::EFFECT_MONO
        | ExynosCamera::EFFECT_NEGATIVE
        /* | ExynosCamera::EFFECT_SOLARIZE */
        | ExynosCamera::EFFECT_SEPIA
        | ExynosCamera::EFFECT_POSTERIZE
        /* | ExynosCamera::EFFECT_WHITEBOARD */
        /* | ExynosCamera::EFFECT_BLACKBOARD */
        | ExynosCamera::EFFECT_AQUA
        ;
    effect = ExynosCamera::EFFECT_NONE;

    flashModeList =
          ExynosCamera::FLASH_MODE_OFF
        | ExynosCamera::FLASH_MODE_AUTO
        | ExynosCamera::FLASH_MODE_ON
        //| ExynosCamera::FLASH_MODE_RED_EYE
        | ExynosCamera::FLASH_MODE_TORCH;
    flashMode = ExynosCamera::FLASH_MODE_OFF;

    focusModeList =
          ExynosCamera::FOCUS_MODE_AUTO
        //| ExynosCamera::FOCUS_MODE_INFINITY
        | ExynosCamera::FOCUS_MODE_MACRO
        //| ExynosCamera::FOCUS_MODE_FIXED
        //| ExynosCamera::FOCUS_MODE_EDOF
        | ExynosCamera::FOCUS_MODE_CONTINUOUS_VIDEO
        | ExynosCamera::FOCUS_MODE_CONTINUOUS_PICTURE
        | ExynosCamera::FOCUS_MODE_TOUCH;

    focusMode = ExynosCamera::FOCUS_MODE_AUTO;

    sceneModeList =
          ExynosCamera::SCENE_MODE_AUTO
        | ExynosCamera::SCENE_MODE_ACTION
        | ExynosCamera::SCENE_MODE_PORTRAIT
        | ExynosCamera::SCENE_MODE_LANDSCAPE
        | ExynosCamera::SCENE_MODE_NIGHT
        | ExynosCamera::SCENE_MODE_NIGHT_PORTRAIT
        | ExynosCamera::SCENE_MODE_THEATRE
        | ExynosCamera::SCENE_MODE_BEACH
        | ExynosCamera::SCENE_MODE_SNOW
        | ExynosCamera::SCENE_MODE_SUNSET
        | ExynosCamera::SCENE_MODE_STEADYPHOTO
        | ExynosCamera::SCENE_MODE_FIREWORKS
        | ExynosCamera::SCENE_MODE_SPORTS
        | ExynosCamera::SCENE_MODE_PARTY
        | ExynosCamera::SCENE_MODE_CANDLELIGHT;
    sceneMode = ExynosCamera::SCENE_MODE_AUTO;

    whiteBalanceList =
          ExynosCamera::WHITE_BALANCE_AUTO
        | ExynosCamera::WHITE_BALANCE_INCANDESCENT
        | ExynosCamera::WHITE_BALANCE_FLUORESCENT
        //| ExynosCamera::WHITE_BALANCE_WARM_FLUORESCENT
        | ExynosCamera::WHITE_BALANCE_DAYLIGHT
        | ExynosCamera::WHITE_BALANCE_CLOUDY_DAYLIGHT
        //| ExynosCamera::WHITE_BALANCE_TWILIGHT
        //| ExynosCamera::WHITE_BALANCE_SHADE
        ;
    whiteBalance = ExynosCamera::WHITE_BALANCE_AUTO;

    autoWhiteBalanceLockSupported = true;
    autoWhiteBalanceLock = false;

    rotation = 0;
    minExposure = -4;
    maxExposure = 4;
    exposure = 0;

    autoExposureLockSupported = true;
    autoExposureLock = false;

    fpsRange[0] =  7000;
    fpsRange[1] = 30000;

    fNumberNum = 22;
    fNumberDen = 10;
    focalLengthNum = 420;
    focalLengthDen = 100;
    apertureNum = 227;
    apertureDen = 100;
    supportVideoStabilization = true;
    applyVideoStabilization = false;
    videoStabilization = false;
    maxNumMeteringAreas = 32;
    maxNumDetectedFaces = 16;
    maxNumFocusAreas = 2;
    maxZoom = ZOOM_LEVEL_MAX;
    hwZoomSupported = false;
    zoom = 0;
    gpsAltitude = 0;
    gpsLatitude = 0;
    gpsLongitude = 0;
    gpsTimestamp = 0;

    /* Additional API default Value. */
    metering = ExynosCamera::METERING_MODE_CENTER;
    focalLengthIn35mmLength = 31;
}

ExynosCameraInfoIMX135Reprocessing::ExynosCameraInfoIMX135Reprocessing()
{
    previewW = 1920;
    previewH = 1080;
    previewColorFormat = V4L2_PIX_FMT_NV21M;
    videoW = 1920;
    videoH = 1080;
    pictureW = 4128;
    pictureH = 3096;
    videoColorFormat = V4L2_PIX_FMT_NV12M;
    ispW = 4128;
    ispH = 3096;
    pictureColorFormat = V4L2_PIX_FMT_YUYV;
    thumbnailW = 512;
    thumbnailH = 384;
}

ExynosCameraInfoS5K6B2Reprocessing::ExynosCameraInfoS5K6B2Reprocessing()
{
    previewW = 1280;
    previewH = 960;
    previewColorFormat = V4L2_PIX_FMT_NV21M;
    videoW = 1920;
    videoH = 1080;
    videoColorFormat = V4L2_PIX_FMT_NV12M;
    pictureW = 1920;
    pictureH = 1080;
    pictureColorFormat = V4L2_PIX_FMT_YUYV;
    thumbnailW = 512;
    thumbnailH = 384;
}

ExynosCameraInfoS5K4E5Reprocessing::ExynosCameraInfoS5K4E5Reprocessing()
{
    previewW = 1920;
    previewH = 1080;
//    previewColorFormat = V4L2_PIX_FMT_YVU420M;
    previewColorFormat = V4L2_PIX_FMT_NV21M;
    videoW = 1920;
    videoH = 1080;
    videoColorFormat = V4L2_PIX_FMT_NV12M;
    ispW = 2560;
    ispH = 1920;
    pictureW = 2560;
    pictureH = 1920;
    pictureColorFormat = V4L2_PIX_FMT_YUYV;
    thumbnailW = 512;
    thumbnailH = 384;
}

#define PFX_NODE                            "/dev/video"

#define M5MOLS_ENTITY_NAME                  "M5MOLS 5-001f"
#define PFX_SUBDEV_ENTITY_MIPI_CSIS         "s5p-mipi-csis"
#define PFX_SUBDEV_ENTITY_FLITE             "flite-subdev"
#define PFX_SUBDEV_ENTITY_GSC_CAP           "gsc-cap-subdev"
#define PFX_VIDEODEV_ENTITY_FLITE           "exynos-fimc-lite"
#define PFX_VIDEODEV_ENTITY_GSC_CAP         "exynos-gsc"

#define MEDIA_DEV_INTERNAL_ISP              "/dev/media2"
#define MEDIA_DEV_EXTERNAL_ISP              "/dev/media1"
#define ISP_VD_NODE_OFFSET                  (40)              //INTERNAL_ISP
#define FLITE_VD_NODE_OFFSET                (36)              //External ISP

#define VIDEO_NODE_PREVIEW_ID               (3)
#define VIDEO_NODE_RECODING_ID              (2)
#define VIDEO_NODE_SNAPSHOT_ID              (1)

#define ISP_SENSOR_MAX_ENTITIES             (1)
#define ISP_SENSOR_PAD_SOURCE_FRONT         (0)
#define ISP_SENSOR_PADS_NUM                 (1)

#define ISP_FRONT_MAX_ENTITIES              (1)
#define ISP_FRONT_PAD_SINK                  (0)
#define ISP_FRONT_PAD_SOURCE_BACK           (1)
#define ISP_FRONT_PAD_SOURCE_BAYER          (2)
#define ISP_FRONT_PAD_SOURCE_SCALERC        (3)
#define ISP_FRONT_PADS_NUM                  (4)

#define ISP_BACK_MAX_ENTITIES               (1)
#define ISP_BACK_PAD_SINK                   (0)
#define ISP_BACK_PAD_SOURCE_3DNR            (1)
#define ISP_BACK_PAD_SOURCE_SCALERP         (2)
#define ISP_BACK_PADS_NUM                   (3)

#define ISP_MODULE_NAME                     "exynos5-fimc-is"
#define ISP_SENSOR_ENTITY_NAME              "exynos5-fimc-is-sensor"
#define ISP_FRONT_ENTITY_NAME               "exynos5-fimc-is-front"
#define ISP_BACK_ENTITY_NAME                "exynos5-fimc-is-back"
#define ISP_VIDEO_BAYER_NAME                "exynos5-fimc-is-bayer"
#define ISP_VIDEO_SCALERC_NAME              "exynos5-fimc-is-scalerc"
#define ISP_VIDEO_3DNR_NAME                 "exynos5-fimc-is-3dnr"
#define ISP_VIDEO_SCALERP_NAME              "exynos5-fimc-is-scalerp"

#define MIPI_NUM                            (1)
#define FLITE_NUM                           (1)
#define GSC_NUM                             (0)

#define PFX_SUBDEV_NODE                     "/dev/v4l-subdev"

#define V4L2_CAMERA_MEMORY_TYPE             (V4L2_MEMORY_DMABUF) /* (V4L2_MEMORY_USERPTR) */
/*
 * V 4 L 2   F I M C   E X T E N S I O N S
 *
 */
#define V4L2_CID_ROTATION                   (V4L2_CID_PRIVATE_BASE + 0)
#define V4L2_CID_PADDR_Y                    (V4L2_CID_PRIVATE_BASE + 1)
#define V4L2_CID_PADDR_CB                   (V4L2_CID_PRIVATE_BASE + 2)
#define V4L2_CID_PADDR_CR                   (V4L2_CID_PRIVATE_BASE + 3)
#define V4L2_CID_PADDR_CBCR                 (V4L2_CID_PRIVATE_BASE + 4)
#define V4L2_CID_STREAM_PAUSE               (V4L2_CID_PRIVATE_BASE + 53)

#define V4L2_CID_CAM_JPEG_MAIN_SIZE         (V4L2_CID_PRIVATE_BASE + 32)
#define V4L2_CID_CAM_JPEG_MAIN_OFFSET       (V4L2_CID_PRIVATE_BASE + 33)
#define V4L2_CID_CAM_JPEG_THUMB_SIZE        (V4L2_CID_PRIVATE_BASE + 34)
#define V4L2_CID_CAM_JPEG_THUMB_OFFSET      (V4L2_CID_PRIVATE_BASE + 35)
#define V4L2_CID_CAM_JPEG_POSTVIEW_OFFSET   (V4L2_CID_PRIVATE_BASE + 36)
#define V4L2_CID_CAM_JPEG_QUALITY           (V4L2_CID_PRIVATE_BASE + 37)

#define V4L2_PIX_FMT_YVYU           v4l2_fourcc('Y', 'V', 'Y', 'U')

/* FOURCC for FIMC specific */
#define V4L2_PIX_FMT_VYUY           v4l2_fourcc('V', 'Y', 'U', 'Y')
#define V4L2_PIX_FMT_NV16           v4l2_fourcc('N', 'V', '1', '6')
#define V4L2_PIX_FMT_NV61           v4l2_fourcc('N', 'V', '6', '1')
#define V4L2_PIX_FMT_NV12T          v4l2_fourcc('T', 'V', '1', '2')

///////////////////////////////////////////////////
// Google Official API : Camera.Parameters
// http://developer.android.com/reference/android/hardware/Camera.Parameters.html
///////////////////////////////////////////////////
int ExynosCamera::cam_int_get_pixel_depth(node_info_t *node)
{
    int depth = 0;

    switch (node->format) {
    case V4L2_PIX_FMT_JPEG:
        depth = 8;
        break;

    case V4L2_PIX_FMT_NV12:
    case V4L2_PIX_FMT_NV21:
    case V4L2_PIX_FMT_YUV420:
    case V4L2_PIX_FMT_YVU420M:
    case V4L2_PIX_FMT_YUV420M:
    case V4L2_PIX_FMT_NV12M:
    case V4L2_PIX_FMT_NV12MT:
        depth = 12;
        break;

    case V4L2_PIX_FMT_RGB565:
    case V4L2_PIX_FMT_YUYV:
    case V4L2_PIX_FMT_YVYU:
    case V4L2_PIX_FMT_UYVY:
    case V4L2_PIX_FMT_VYUY:
    case V4L2_PIX_FMT_NV16:
    case V4L2_PIX_FMT_NV61:
    case V4L2_PIX_FMT_YUV422P:
    case V4L2_PIX_FMT_SBGGR10:
    case V4L2_PIX_FMT_SBGGR12:
    case V4L2_PIX_FMT_SBGGR16:
        depth = 16;
        break;

    case V4L2_PIX_FMT_RGB32:
        depth = 32;
        break;
    default:
        CLOGE("ERR(%s):Get depth fail(format : %d)", __func__, node->format);
        break;
    }

    return depth;
}

int ExynosCamera::cam_int_s_fmt(node_info_t *node)
{
    int ret = 0;
    struct v4l2_format v4l2_fmt;

    if (node->planes <= 0) {
        CLOGE("ERR(%s):S_FMT, Out of bound : Number of element plane(%d)", __func__, node->planes);
        return -1;
    }

    CLOGD("DEBUG(%s):type %d, %d x %d, format %d", __func__, node->type, node->width, node->height, node->format);

    memset(&v4l2_fmt, 0, sizeof(struct v4l2_format));

    v4l2_fmt.type                   = node->type;
    v4l2_fmt.fmt.pix_mp.width       = node->width;
    v4l2_fmt.fmt.pix_mp.height      = node->height;
    v4l2_fmt.fmt.pix_mp.pixelformat = node->format;
    v4l2_fmt.fmt.pix_mp.field       = V4L2_FIELD_ANY;

    ret = exynos_v4l2_s_fmt(node->fd, &v4l2_fmt);
    if (ret < 0) {
        CLOGE("ERR(%s):exynos_v4l2_s_fmt(fd:%d) fail (%d)", __func__, node->fd, ret);
        return ret;
    }

    return ret;
}

int ExynosCamera::cam_int_reqbufs(node_info_t *node)
{
    int ret = 0;
    struct v4l2_requestbuffers req;

    req.count  = node->buffers;
    req.type   = node->type;
    req.memory = node->memory;

    CLOGD("DEBUG(%s):fd %d, count %d, type %d, memory %d", __func__, node->fd, req.count, req.type, req.memory);

    ret = exynos_v4l2_reqbufs(node->fd, &req);
    if (ret < 0) {
        CLOGE("ERR(%s):exynos_v4l2_reqbufs(fd:%d, count:%d) fail (%d)", __func__, node->fd, req.count, ret);
        return ret;
    }

    for (int i = 0; i < VIDEO_MAX_FRAME; i++)
        cam_int_m_setQ(node, i, false);

    return req.count;
}

int ExynosCamera::cam_int_clrbufs(node_info_t * node)
{
    int ret = 0;
    struct v4l2_requestbuffers req;

    req.count  = 0;
    req.type   = node->type;
    req.memory = node->memory;

    CLOGD("DEBUG(%s):fd %d, count %d, type %d, memory %d", __func__, node->fd, req.count, req.type, req.memory);

    ret = exynos_v4l2_reqbufs(node->fd, &req);
    if (ret < 0) {
        CLOGE("ERR(%s):exynos_v4l2_reqbufs(fd:%d, count:%d) fail (%d)", __func__, node->fd, req.count, ret);
        return ret;
    }

    for (int i = 0; i < VIDEO_MAX_FRAME; i++)
        cam_int_m_setQ(node, i, false);

    return req.count;
}

int ExynosCamera::cam_int_qbuf(node_info_t *node, int index)
{
    int ret = 0;
    unsigned int i = 0;
    struct v4l2_buffer v4l2_buf;
    struct v4l2_plane planes[VIDEO_MAX_PLANES];

    /* waiting that index is free*/
    bool flagAlreadyQ = false;

    for (i = 0; i < CAMERA_Q_TATOL_WATING_TIME; i += CAMERA_Q_WATING_TIME) {
        if (cam_int_m_flagQ(node, index) == false)
            break;

        flagAlreadyQ = true;
        usleep(CAMERA_Q_WATING_TIME);
    }

    if (flagAlreadyQ == true) {
        CLOGW("WARN(%s):q waiting on node(%d), index(%d), waiting time(%d) msec", __func__, node->fd, index, i / 1000);

        if (CAMERA_Q_TATOL_WATING_TIME <= i) {
            CLOGE("ERR(%s):q waiting on node(%d), index(%d) time out(%d) msec", __func__, node->fd,  index, i / 1000);
            return -1;
        }
    }

    v4l2_buf.m.planes = planes;
    v4l2_buf.type     = node->type;
    v4l2_buf.memory   = node->memory;
    v4l2_buf.index    = index;
    v4l2_buf.length   = node->planes;

    for (i = 0; i < node->planes; i++) {
        if (node->memory == V4L2_MEMORY_DMABUF)
            v4l2_buf.m.planes[i].m.fd = (int)(node->buffer[index].fd.extFd[i]);
        else if (node->memory == V4L2_MEMORY_USERPTR)
            v4l2_buf.m.planes[i].m.userptr = (unsigned long)(node->buffer[index].virt.extP[i]);
        else {
            CLOGE("ERR(%s):invalid node->memory(%d)", __func__, node->memory);
            return -1;
        }

        v4l2_buf.m.planes[i].length  = (unsigned long)(node->buffer[index].size.extS[i]);
    }

    ret = exynos_v4l2_qbuf(node->fd, &v4l2_buf);
    if (ret < 0) {
        CLOGE("ERR(%s):exynos_v4l2_qbuf(fd:%d, index:%d) fail (%d)", __func__, node->fd, index, ret);
        return ret;
    }

    cam_int_m_setQ(node, index, true);

    return ret;
}

int ExynosCamera::cam_int_qbuf(node_info_t *node, int index, node_info_t *srcNode)
{
    int ret = 0;
    unsigned int i = 0;
    struct v4l2_buffer v4l2_buf;
    struct v4l2_plane planes[VIDEO_MAX_PLANES];

    /* waiting that index is free*/
    bool flagAlreadyQ = false;

    for (i = 0; i < CAMERA_Q_TATOL_WATING_TIME; i += CAMERA_Q_WATING_TIME) {
        if (cam_int_m_flagQ(node, index) == false)
            break;

        flagAlreadyQ = true;
        usleep(CAMERA_Q_WATING_TIME);
    }

    if (flagAlreadyQ == true) {
        CLOGW("WARN(%s):q waiting on node(%d), index(%d), waiting time(%d) msec", __func__, node->fd, index, i / 1000);

        if (CAMERA_Q_TATOL_WATING_TIME <= i) {
            CLOGE("ERR(%s):q waiting on node(%d), index(%d) time out(%d) msec", __func__, node->fd,  index, i / 1000);
            return -1;
        }
    }

    v4l2_buf.m.planes = planes;
    v4l2_buf.type     = node->type;
    v4l2_buf.memory   = srcNode->memory;
    v4l2_buf.index    = index;
    v4l2_buf.length   = srcNode->planes;

    for (i = 0; i < srcNode->planes; i++) {
        if (srcNode->memory == V4L2_MEMORY_DMABUF) {
            v4l2_buf.m.planes[i].m.fd = (int)(srcNode->buffer[index].fd.extFd[i]);
        } else if (srcNode->memory == V4L2_MEMORY_USERPTR) {
            v4l2_buf.m.planes[i].m.userptr = (unsigned long)(srcNode->buffer[index].virt.extP[i]);
        } else {
            CLOGE("ERR(%s):invalid srcNode->memory(%d)", __func__, srcNode->memory);
            return -1;
        }

        v4l2_buf.m.planes[i].length = (unsigned long)(srcNode->buffer[index].size.extS[i]);
    }

    ret = exynos_v4l2_qbuf(node->fd, &v4l2_buf);
    if (ret < 0) {
        CLOGE("ERR(%s):exynos_v4l2_qbuf(fd:%d, index:%d) fail (%d)", __func__, node->fd, index, ret);
        return ret;
    }

    cam_int_m_setQ(node, index, true);

    return ret;
}

int ExynosCamera::cam_int_qbuf(node_info_t *node, int index, node_info_t *srcNode, ExynosBuffer mataBuf)
{
    int ret = 0;
    unsigned int i = 0;
    struct v4l2_buffer v4l2_buf;
    struct v4l2_plane planes[VIDEO_MAX_PLANES];

    /* waiting that index is free*/
    bool flagAlreadyQ = false;

    for (i = 0; i < CAMERA_Q_TATOL_WATING_TIME; i += CAMERA_Q_WATING_TIME) {
        if (cam_int_m_flagQ(node, index) == false)
            break;

        flagAlreadyQ = true;
        usleep(CAMERA_Q_WATING_TIME);
    }

    if (flagAlreadyQ == true) {
        CLOGW("WARN(%s):q waiting on node(%d), index(%d), waiting time(%d) msec", __func__, node->fd, index, i / 1000);

        if (CAMERA_Q_TATOL_WATING_TIME <= i) {
            CLOGE("ERR(%s):q waiting on node(%d), index(%d) time out(%d) msec", __func__, node->fd,  index, i / 1000);
            return -1;
        }
    }

    v4l2_buf.m.planes = planes;
    v4l2_buf.type     = node->type;
    v4l2_buf.memory   = srcNode->memory;
    v4l2_buf.index    = index;
    v4l2_buf.length   = srcNode->planes;

    for (i = 0; i < srcNode->planes; i++) {
        if (srcNode->memory == V4L2_MEMORY_DMABUF) {
            v4l2_buf.m.planes[i].m.fd = (int)(srcNode->buffer[index].fd.extFd[i]);
        } else if (srcNode->memory == V4L2_MEMORY_USERPTR) {
            v4l2_buf.m.planes[i].m.userptr = (unsigned long)(srcNode->buffer[index].virt.extP[i]);
        } else {
            CLOGE("ERR(%s):invalid srcNode->memory(%d)", __func__, srcNode->memory);
            return -1;
        }

        v4l2_buf.m.planes[i].length = (unsigned long)(srcNode->buffer[index].size.extS[i]);
    }

    if (srcNode->memory == V4L2_MEMORY_DMABUF) {
        v4l2_buf.m.planes[v4l2_buf.length - 1].m.fd = (int)(mataBuf.fd.extFd[v4l2_buf.length - 1]);
    } else if (srcNode->memory == V4L2_MEMORY_USERPTR) {
        v4l2_buf.m.planes[v4l2_buf.length - 1].m.userptr = (unsigned long)(mataBuf.virt.extP[v4l2_buf.length - 1]);
    } else {
        CLOGE("ERR(%s):invalid meta(%d)", __func__, srcNode->memory);
        return -1;
    }

    ret = exynos_v4l2_qbuf(node->fd, &v4l2_buf);
    if (ret < 0) {
        CLOGE("ERR(%s):exynos_v4l2_qbuf(fd:%d, index:%d) fail (%d)", __func__, node->fd, index, ret);
        return ret;
    }

    cam_int_m_setQ(node, index, true);

    return ret;
}

int ExynosCamera::cam_int_streamon(node_info_t *node)
{
    int ret = 0;

    CLOGD("DEBUG(%s):fd %d, type %d", __func__, node->fd, (int)node->type);

    ret = exynos_v4l2_streamon(node->fd, node->type);
    if (ret < 0) {
        CLOGE("ERR(%s):exynos_v4l2_streamon(fd:%d, type:%d) fail (%d)", __func__, node->fd, (int)node->type, ret);
        return ret;
    }

    return ret;
}

int ExynosCamera::cam_int_streamoff(node_info_t *node)
{
    int ret = 0;

    CLOGD("DEBUG(%s):fd %d, type %d", __func__, node->fd, (int)node->type);

    ret = exynos_v4l2_streamoff(node->fd, node->type);
    if (ret < 0) {
        CLOGE("ERR(%s):exynos_v4l2_streamoff(fd:%d, type:%d) fail (%d)", __func__, node->fd, (int)node->type, ret);
        return ret;
    }

    for (int i = 0; i < VIDEO_MAX_FRAME; i++)
        cam_int_m_setQ(node, i, false);

    return ret;
}

#ifdef USE_CAMERA_ESD_RESET
int ExynosCamera::cam_int_polling(node_info_t *node)
{
    struct pollfd events;

    /* 50 msec * 100 = 5sec */
    int cnt = 100;
    long sec = 50; /* 50 msec */

    int ret = 0;
    int pollRet = 0;

    events.fd = node->fd;
    events.events = POLLIN | POLLRDNORM | POLLOUT | POLLWRNORM | POLLERR;
    events.revents = 0;

    while (cnt--) {
        pollRet = poll(&events, 1, sec);
        if (pollRet < 0) {
            ret = -1;
            break;
        } else if (0 < pollRet) {
            if (events.revents & POLLIN) {
                break;
            } else if (events.revents & POLLERR) {
                ret = -1;
                break;
            }
        }
    }

    if (ret < 0 || cnt <= 0) {
        CLOGE("ERR(%s):[esdreset] poll[%d], pollRet(%d) event(0x%x), cnt(%d)",
            __func__, node->fd, pollRet, events.revents, cnt);

        if (cnt <= 0)
            ret = -1;
    }

    return ret;
}
#endif

int ExynosCamera::cam_int_dqbuf(node_info_t *node)
{
    int ret = 0;
    struct v4l2_buffer v4l2_buf;
    struct v4l2_plane planes[VIDEO_MAX_PLANES];

    v4l2_buf.type       = node->type;
    v4l2_buf.memory     = node->memory;
    v4l2_buf.m.planes   = planes;
    v4l2_buf.length     = node->planes;

    ret = exynos_v4l2_dqbuf(node->fd, &v4l2_buf);
    if (ret < 0) {
        if (ret != -EAGAIN)
            CLOGE("ERR(%s):exynos_v4l2_dqbuf(fd:%d) fail (%d)", __func__, node->fd, ret);

        return ret;
    }

#ifdef USE_FOR_DTP
    if (v4l2_buf.flags & V4L2_BUF_FLAG_ERROR) {
        CLOGE("ERR(%s):exynos_v4l2_dqbuf(fd:%d) returned with error (%d)", __func__, node->fd, V4L2_BUF_FLAG_ERROR);
        return -1;
    }
#endif

    cam_int_m_setQ(node, v4l2_buf.index, false);

    return v4l2_buf.index;
}

int ExynosCamera::cam_int_s_input(node_info_t *node, int index)
{
    int ret = 0;

    CLOGD("DEBUG(%s):fd %d, index %d", __func__, node->fd, index);

    ret = exynos_v4l2_s_input(node->fd, index);
    if (ret < 0)
        CLOGE("ERR(%s):exynos_v4l2_s_input(fd:%d, index:%d) fail (%d)", __func__, node->fd, index, ret);

    return ret;
}

int ExynosCamera::cam_int_querybuf(node_info_t *node)
{
    int ret = 0;
    struct v4l2_buffer v4l2_buf;
    struct v4l2_plane planes[VIDEO_MAX_PLANES];
    int plane_index;

    for (int i = 0; i < node->buffers; i++) {
        v4l2_buf.type       = node->type;
        v4l2_buf.memory     = node->memory;
        v4l2_buf.index      = i;
        v4l2_buf.m.planes   = planes;
        v4l2_buf.length     = node->planes; //  this is for multi-planar

        ret = ioctl(node->fd, VIDIOC_QUERYBUF, &v4l2_buf);
        if (ret < 0) {
            CLOGE("ERR(%s):VIDIOC_QUERYBUF(fd:%d, index:%d) fail (%d)", __func__, node->fd, v4l2_buf.index, ret);
            return ret;
        }
    }

    return 0;
}

bool ExynosCamera::cam_int_m_flagQ(node_info_t *node, int index)
{
    Mutex::Autolock lock(node->QLock);

    if (node->flagDup == false)
        return node->flagQ[index];

    return false;
}

void ExynosCamera::cam_int_m_setQ(node_info_t *node, int index, bool toggle)
{
    Mutex::Autolock lock(node->QLock);

    if (node->flagDup == false)
        node->flagQ[index] = toggle;
}

ExynosCamera::ExynosCamera()
{
    m_flagCreate = false;
    m_cameraId = CAMERA_ID_BACK;
    m_cameraMode = CAMERA_MODE_BACK;

    for (int i = 0; i < CAMERA_MODE_MAX; i++) {
        m_defaultCameraInfo[i] = NULL;
        m_curCameraInfo[i] = NULL;
        m_flagOpen[i] = false;
    }

    m_jpegQuality= 100;
    m_jpegThumbnailQuality = 100;
    m_currentZoom = -1;
    m_ionCameraClient = -1;

    m_needCallbackCSC = true;

    m_previewInternalMemAlloc = false;

    m_isFirtstIspStartReprocessing = true;
    m_isFirtstSensorStartReprocessing = true;
    m_isFirtstIs3a1SrcStart = true;
    m_isFirtstIs3a1DstStart = true;
    m_isFirtstIs3a0SrcStart = true;
    m_isFirtstIs3a0DstStart = true;

    m_sensorEntity = NULL;
    m_mipiEntity = NULL;
    m_fliteSdEntity = NULL;
    m_fliteVdEntity = NULL;
    m_gscSdEntity = NULL;
    m_gscVdEntity = NULL;
    m_ispSensorEntity = NULL;
    m_ispFrontEntity = NULL;
    m_ispBackEntity = NULL;
    m_ispBayerEntity = NULL;
    m_ispScalercEntity = NULL;
    m_ispScalerpEntity = NULL;
    m_isp3dnrEntity = NULL;

    m_media = NULL;

    memset((void *)m_cameraName, 0, 32);

    memset(&mExifInfo, 0, sizeof(mExifInfo));
    memset(m_imageUniqueIdBuf, '\0', UNIQUE_ID_BUF_SIZE);

    m_internalISP = true;
    m_flagStartFaceDetection = false;
    m_flagAutoFocusRunning = false;
    m_touchAFMode = false;
    m_touchAFModeForFlash = false;
    m_isFirtstIspStart = true;
    m_isFirtstSensorStart = true;
    m_oldMeteringMode = 0;
    m_recordingHint = false;

#ifdef USE_VDIS
    m_vdisInIndex = -1;
    m_vdisInRcount = -1;
    m_vdisInFcount = -1;
    m_vdisInIndexUpdate = false;
    m_isHWVDis = true;
#endif
    m_sensorFrameCount = 0;

#if CAPTURE_BUF_GET
    m_recentCaptureBayerBufIndex = 0;
    m_minCaptureBayerBuf = MIN_CAPTURE_BAYER_COUNT;
    m_notInsertBayer = false;
    m_waitCaptureBayer = false;
    m_isDVFSLocked = false;

    for (int i =0; i < NUM_BAYER_BUFFERS; i++) {
        m_captureBayerIndex[i] = -1;
        m_captureBayerLock[i] = false;
    }
#endif

#ifdef USE_CAMERA_ESD_RESET
    m_stateESDReset = 0;
#endif

    m_numOfShotedFrame = 0;
    m_numOfShotedIspFrame = 0;
    m_notifyStopMsg = false;
    m_traceCount = 0;

    for (int i = 0; i < VIDEO_MAX_FRAME; i++)
        memset(&m_previewMetaBuffer[i], 0, sizeof(struct ExynosBuffer));

    m_flashMgr = NULL;
    m_autofocusMgr = NULL;
    m_sCaptureMgr = NULL;

    m_is3a1SrcLastBufIndex = -1;
    m_is3a1DstLastBufIndex = -1;
    m_is3a1FrameCount = 0;
    m_aeFrameCount = 0;

#ifdef FRONT_NO_ZSL
    m_frontCaptureStatus = 0;
#endif
#ifdef FORCE_LEADER_OFF
    m_forceIspOff = 0;
#endif
}

ExynosCamera::~ExynosCamera()
{
    if (m_flagCreate == true)
        this->destroy();
}

bool ExynosCamera::create(int cameraId)
{
    if (m_flagCreate == true) {
        CLOGW("WARN(%s):Already created", __func__);
        return true;
    }

    CLOGD("DEBUG(%s):(%d) cameraId: %d", __func__, __LINE__, cameraId);
    m_cameraId = cameraId;

/*
    if (exynos_v4l2_enuminput(m_camera_info.preview.fd, cameraId, m_cameraName) == false) {
        CLOGE("ERR(%s):exynos_v4l2_enuminput(%d, %s) fail", __func__, cameraId, m_cameraName);
        goto err;
    }

    if (exynos_v4l2_s_input(m_camera_info.preview.fd, cameraId) < 0) {
        CLOGE("ERR(%s):exynos_v4l2_s_input() fail",  __func__);
        goto err;
    }

    if (strcmp((const char*)m_cameraName, "S5K4E5") == 0) {
        m_defaultCameraInfo  = new ExynosCameraInfoS5K4E5;
        m_curCameraInfo      = new ExynosCameraInfoS5K4E5;
    } else if (strcmp((const char*)m_cameraName, "S5K3H5") == 0) {
        m_defaultCameraInfo  = new ExynosCameraInfoS5K3H5;
        m_curCameraInfo      = new ExynosCameraInfoS5K3H5;
    } else if (strcmp((const char*)m_cameraName, "S5K3H7") == 0) {
        m_defaultCameraInfo  = new ExynosCameraInfoS5K3H7;
        m_curCameraInfo      = new ExynosCameraInfoS5K3H7;
    } else if (strcmp((const char*)m_cameraName, "S5K6A3") == 0) {
        m_defaultCameraInfo  = new ExynosCameraInfoS5K6A3;
        m_curCameraInfo      = new ExynosCameraInfoS5K6A3;
    } else if (strcmp((const char*)m_cameraName, "M5M0") == 0) {
        m_defaultCameraInfo  = new ExynosCameraInfoM5M0;
        m_curCameraInfo      = new ExynosCameraInfoM5M0;
    } else if (strcmp((const char*)m_cameraName, "IMX135") == 0) {
        m_defaultCameraInfo  = new ExynosCameraInfoIMX135;
        m_curCameraInfo      = new ExynosCameraInfoIMX135;
    } else {
        CLOGE("ERR(%s):invalid camera Name (%s) fail", __func__, m_cameraName);
        goto err;
    }
*/
    m_defaultCameraInfo[CAMERA_MODE_BACK]  = new ExynosCameraInfoIMX135;
    m_curCameraInfo    [CAMERA_MODE_BACK]  = new ExynosCameraInfoIMX135;

    m_defaultCameraInfo[CAMERA_MODE_FRONT] = new ExynosCameraInfoS5K6B2;
    m_curCameraInfo    [CAMERA_MODE_FRONT] = new ExynosCameraInfoS5K6B2;

    if (cameraId == CAMERA_ID_BACK) {
        m_cameraMode = CAMERA_MODE_BACK;

        m_defaultCameraInfo[CAMERA_MODE_REPROCESSING]  = new ExynosCameraInfoIMX135Reprocessing;
        m_curCameraInfo    [CAMERA_MODE_REPROCESSING]  = new ExynosCameraInfoIMX135Reprocessing;
    } else if (cameraId == CAMERA_ID_FRONT) {
        m_cameraMode = CAMERA_MODE_FRONT;

        m_defaultCameraInfo[CAMERA_MODE_REPROCESSING]  = new ExynosCameraInfoS5K6B2Reprocessing;
        m_curCameraInfo    [CAMERA_MODE_REPROCESSING]  = new ExynosCameraInfoS5K6B2Reprocessing;
    }

    for (int i = 0; i < CAMERA_MODE_MAX; i++) {
        memset(&m_camera_info[i].sensor,   0, sizeof(node_info_t));
        memset(&m_camera_info[i].is3a0Src, 0, sizeof(node_info_t));
        memset(&m_camera_info[i].is3a0Dst, 0, sizeof(node_info_t));
        memset(&m_camera_info[i].is3a1Src, 0, sizeof(node_info_t));
        memset(&m_camera_info[i].is3a1Dst, 0, sizeof(node_info_t));
        memset(&m_camera_info[i].isp,      0, sizeof(node_info_t));
        memset(&m_camera_info[i].picture,  0, sizeof(node_info_t));
        memset(&m_camera_info[i].preview,  0, sizeof(node_info_t));
        memset(&m_camera_info[i].video,    0, sizeof(node_info_t));
#ifdef USE_VDIS
        memset(&m_camera_info[i].vdisc,    0, sizeof(node_info_t));
        memset(&m_camera_info[i].vdiso,    0, sizeof(node_info_t));
#endif

        m_camera_info[i].sensor.fd   = -1;
        m_camera_info[i].is3a0Src.fd = -1;
        m_camera_info[i].is3a0Dst.fd = -1;
        m_camera_info[i].is3a1Src.fd = -1;
        m_camera_info[i].is3a1Dst.fd = -1;
        m_camera_info[i].isp.fd      = -1;
        m_camera_info[i].picture.fd  = -1;
        m_camera_info[i].preview.fd  = -1;
        m_camera_info[i].video.fd    = -1;
#ifdef USE_VDIS
        m_camera_info[i].vdisc.fd    = -1;
        m_camera_info[i].vdiso.fd    = -1;
#endif
    }

    m_isFirtstSensorStart = true;
    m_isFirtstIs3a1SrcStart= true;
    m_isFirtstIs3a1DstStart = true;
    m_isFirtstIs3a0SrcStart= true;
    m_isFirtstIs3a0DstStart = true;
    m_isFirtstIspStart = true;

    m_isFirtstIspStartReprocessing= true;
    m_isFirtstSensorStartReprocessing = true;

    m_isFirtstIs3a1StreamOn = true;
    m_isFirtstIs3a0StreamOn = true;
    m_isIs3a1ParamChanged = false;
    m_isIs3a0ParamChanged = false;

    m_cameraModeIs3a0 = m_cameraMode;

    m_flagStartFaceDetection = false;
    m_flagAutoFocusRunning = false;

    m_touchAFMode = false;
    m_touchAFModeForFlash= false;
    m_oldMeteringMode = m_curCameraInfo[m_cameraMode]->metering;

    if (m_initSensor(m_cameraMode) == false) {
        CLOGE("ERR(%s):m_initSensor(%d) fail", __func__, m_cameraMode);
        goto err;
    }

    if (this->getCameraMode() == CAMERA_MODE_BACK) {
        if (m_initSensor(CAMERA_MODE_REPROCESSING) == false) {
            CLOGE("ERR(%s):m_initSensor(%d) fail", __func__, m_cameraMode);
            goto err;
        }

    }

    m_ionCameraClient = ion_client_create();
    if (m_ionCameraClient < 0) {
        CLOGE("ERR(%s):ion_client_create() fail", __func__);
        m_ionCameraClient = -1;
    }

    m_setExifFixedAttribute();

    m_flashMgr = new ExynosCameraActivityFlash();
    m_autofocusMgr = new ExynosCameraActivityAutofocus();
    m_sCaptureMgr = new ExynosCameraActivitySpecialCapture();

    m_flagCreate = true;

    return true;

err:
    if (0 < m_ionCameraClient)
        ion_client_destroy(m_ionCameraClient);
    m_ionCameraClient = -1;

    for (int i = 0; i < CAMERA_MODE_MAX; i++) {
        if (m_defaultCameraInfo[i])
            delete m_defaultCameraInfo[i];
        m_defaultCameraInfo[i] = NULL;

        if (m_curCameraInfo[i])
            delete m_curCameraInfo[i];
        m_curCameraInfo[i] = NULL;
    }

    return false;
}

bool ExynosCamera::destroy(void)
{
    CLOGD("DEBUG(%s):in", __func__);

    if (m_flagCreate == false) {
        CLOGW("WARN(%s):Not yet created", __func__);
        return true;
    }

    if (m_isDVFSLocked == true) {
        if (DvfsUnLock() == false)
            CLOGW("WRN(%s): Dvfs unlock fail!!!", __func__);
    }

    if (mExifInfo.maker_note)
        delete mExifInfo.maker_note;
    mExifInfo.maker_note = NULL;
    mExifInfo.maker_note_size = 0;

#ifdef USE_VDIS
    if (m_camera_info[CAMERA_MODE_BACK].vdisc.flagStart == true) {
        if (stopVdisCapture() == false)
            CLOGE("ERR(%s):stopVdisCapture() fail", __func__);
    }

    if (m_camera_info[CAMERA_MODE_BACK].vdiso.flagStart == true) {
        if (stopVdisOutput() == false)
            CLOGE("ERR(%s):stopVdisOutput() fail", __func__);
    }
#endif

    if (m_camera_info[CAMERA_MODE_BACK].video.flagStart == true) {
        if (stopVideo() == false)
            CLOGE("ERR(%s):stopVideo() fail", __func__);
    }

    if (m_camera_info[CAMERA_MODE_BACK].preview.flagStart == true) {
        if (stopPreview() == false)
            CLOGE("ERR(%s):stopPreview() fail", __func__);
    }

    if (m_camera_info[CAMERA_MODE_BACK].picture.flagStart == true) {
        if (stopPicture() == false)
            CLOGE("ERR(%s):stopPicture() fail", __func__);
    }

    if (m_camera_info[CAMERA_MODE_REPROCESSING].sensor.flagStart == true) {
        if (stopSensorOff(CAMERA_MODE_REPROCESSING) == false)
            CLOGE("ERR(%s):stopSensorOff() fail", __func__);

        if (stopSensorReprocessing() == false)
            CLOGE("ERR(%s):stopSensorReprocessing() fail", __func__);
    }

    if (m_camera_info[CAMERA_MODE_BACK].sensor.flagStart == true) {
        if (stopSensor() == false)
            CLOGE("ERR(%s):stopSensor() fail", __func__);
    }

    if (m_camera_info[CAMERA_MODE_BACK].isp.flagStart == true) {
        if (stopIsp() == false)
            CLOGE("ERR(%s):stopIsp() fail", __func__);
    }

    if (m_camera_info[CAMERA_MODE_BACK].is3a0Src.flagStart == true ||
        m_camera_info[CAMERA_MODE_BACK].is3a0Dst.flagStart == true) {
        if (stopIs3a0(CAMERA_MODE_BACK) == false)
            CLOGE("ERR(%s):stopIs3a0() fail", __func__);
    }

    if (m_camera_info[CAMERA_MODE_BACK].is3a1Src.flagStart == true ||
        m_camera_info[CAMERA_MODE_BACK].is3a1Dst.flagStart == true) {
        if (stopIs3a1(CAMERA_MODE_BACK) == false)
            CLOGE("ERR(%s):stopIs3a1() fail", __func__);
    }

#ifdef USE_VDIS
    if (m_camera_info[CAMERA_MODE_FRONT].vdisc.flagStart == true) {
        if (stopVdisCapture() == false)
            CLOGE("ERR(%s):stopVdisCapture() fail", __func__);
    }

    if (m_camera_info[CAMERA_MODE_FRONT].vdiso.flagStart == true) {
        if (stopVdisOutput() == false)
            CLOGE("ERR(%s):stopVdisOutput() fail", __func__);
    }
#endif

    if (m_camera_info[CAMERA_MODE_FRONT].is3a0Src.flagStart == true ||
        m_camera_info[CAMERA_MODE_FRONT].is3a0Dst.flagStart == true) {
        if (stopIs3a0(CAMERA_MODE_FRONT) == false)
            CLOGE("ERR(%s):stopIs3a0() fail", __func__);
    }

    if (m_camera_info[CAMERA_MODE_FRONT].is3a1Src.flagStart == true ||
        m_camera_info[CAMERA_MODE_FRONT].is3a1Dst.flagStart == true) {
        if (stopIs3a1(CAMERA_MODE_FRONT) == false)
            CLOGE("ERR(%s):stopIs3a1() fail", __func__);
    }

#ifdef USE_VDIS
    if (m_camera_info[CAMERA_MODE_REPROCESSING].vdisc.flagStart == true) {
        if (stopVdisCapture() == false)
            CLOGE("ERR(%s):stopVdisCapture() fail", __func__);
    }

    if (m_camera_info[CAMERA_MODE_REPROCESSING].vdiso.flagStart == true) {
        if (stopVdisOutput() == false)
            CLOGE("ERR(%s):stopVdisOutput() fail", __func__);
    }
#endif

    if (m_camera_info[CAMERA_MODE_REPROCESSING].preview.flagStart == true) {
        if (stopPreviewReprocessing() == false)
            CLOGE("ERR(%s):stopPreviewReprocessing() fail", __func__);
    }

    if (m_camera_info[CAMERA_MODE_REPROCESSING].picture.flagStart == true) {
        if (stopPictureReprocessing() == false)
            CLOGE("ERR(%s):stopPictureReprocessing() fail", __func__);
    }

    if (m_camera_info[CAMERA_MODE_REPROCESSING].isp.flagStart == true) {
        if (stopIspReprocessing() == false)
            CLOGE("ERR(%s):stopIspReprocessing() fail", __func__);
    }

    if (m_camera_info[CAMERA_MODE_REPROCESSING].is3a0Src.flagStart == true ||
        m_camera_info[CAMERA_MODE_REPROCESSING].is3a0Dst.flagStart == true) {
        if (stopIs3a0(CAMERA_MODE_REPROCESSING) == false)
            CLOGE("ERR(%s):stopIs3a0() fail", __func__);
    }

    if (m_camera_info[CAMERA_MODE_REPROCESSING].is3a1Src.flagStart == true ||
        m_camera_info[CAMERA_MODE_REPROCESSING].is3a1Dst.flagStart == true) {
        if (stopIs3a1(CAMERA_MODE_REPROCESSING) == false)
            CLOGE("ERR(%s):stopIs3a1() fail", __func__);
    }

    if (getFlagInternalPreviewBuf() == true)
        releaseAllInternalPreviewBuf();

#ifdef USE_VDIS
    m_closeVdisOut();
#endif
    m_closePreview(m_cameraMode);
    m_closePicture(m_cameraMode);

    m_closeIs3a0(CAMERA_MODE_FRONT);
    m_closeIs3a0(CAMERA_MODE_BACK);
    m_closeIs3a0(CAMERA_MODE_REPROCESSING);

    m_closeIs3a1(CAMERA_MODE_FRONT);
    m_closeIs3a1(CAMERA_MODE_BACK);
    m_closeIs3a1(CAMERA_MODE_REPROCESSING);

    m_closeSensor(m_cameraMode);
    m_closeIsp(m_cameraMode);

    m_closePreview(CAMERA_MODE_REPROCESSING);
    m_closePicture(CAMERA_MODE_REPROCESSING);
    m_closeSensor(CAMERA_MODE_REPROCESSING);
    m_closeIsp(CAMERA_MODE_REPROCESSING);

    if (0 < m_ionCameraClient)
        ion_client_destroy(m_ionCameraClient);
    m_ionCameraClient = -1;

    for (int i = 0; i < CAMERA_MODE_MAX; i++) {
        if (m_defaultCameraInfo[i])
            delete m_defaultCameraInfo[i];
        m_defaultCameraInfo[i] = NULL;

        if (m_curCameraInfo[i])
            delete m_curCameraInfo[i];
        m_curCameraInfo[i] = NULL;

        m_flagOpen[i] = false;
    }

    if (m_flashMgr) {
        delete m_flashMgr;
        m_flashMgr = NULL;
    }

    if (m_autofocusMgr) {
        delete m_autofocusMgr;
        m_autofocusMgr = NULL;
    }

    if (m_sCaptureMgr) {
        delete m_sCaptureMgr;
        m_sCaptureMgr = NULL;
    }

    m_flagCreate = false;

    return true;
}

bool ExynosCamera::flagCreate(void)
{
    return m_flagCreate;
}

bool ExynosCamera::openCamera(int cameraId)
{
    CLOGD("DEBUG(%s):(%d) cameraId: %d", __func__, __LINE__, cameraId);

    if (m_flagCreate == false) {
        CLOGE("ERR(%s):Not yet created", __func__);
        return false;
    }

    /* TODO: m_internalIsp have to set by sensor name */
    if (cameraId == CAMERA_ID_BACK)
        m_internalISP = true;
    else if (cameraId == CAMERA_ID_FRONT)
        m_internalISP = true;

    if (m_internalISP == true) {
        if (m_openInternalISP(m_cameraMode) == false) {
            CLOGE("ERR(%s):m_openInternalISP(%d), fail", __func__, m_cameraMode);
            return false;
        }
    } else {
        if (m_openExternalISP(m_cameraMode) == false) {
            CLOGE("ERR(%s):m_openExternalISP(%d), fail", __func__, m_cameraMode);
            return false;
        }
    }

    if (m_getImageUniqueId() == false)
        CLOGE("ERR(%s):m_getImageUniqueId() fail", __func__);

    m_flagOpen[cameraId] = true;

    return true;
}

bool ExynosCamera::m_openInternalISP(int cameraMode)
{
    if (cameraMode == CAMERA_MODE_BACK) {
        if (m_openSensor(cameraMode) == false) {
            CLOGE("ERR(%s):m_openSensor(%d) fail", __func__, cameraMode);
            return false;
        }

        if (m_openIsp(cameraMode) == false) {
            CLOGE("ERR(%s):m_openIsp(%d) fail", __func__, cameraMode);
            goto err;
        }

        if (m_openIs3a1(cameraMode) == false) {
            CLOGE("ERR(%s):m_openIs3a1(%d) fail", __func__, cameraMode);
            goto err;
        }

        if (m_openPicture(cameraMode) == false) {
            CLOGE("ERR(%s):m_openPicture(%d) fail", __func__, cameraMode);
            goto err;
        }

        if (m_openPreview(cameraMode) == false) {
            CLOGE("ERR(%s):m_openPreview(%d) fail", __func__, cameraMode);
            goto err;
        }

#ifdef USE_VDIS
        if (m_openVdisOut() == false) {
            CLOGE("ERR(%s):m_openVdisOut(%d) fail", __func__, cameraMode);
            goto err;
        }
#endif
        if (m_openSensor(CAMERA_MODE_REPROCESSING) == false) {
            CLOGE("ERR(%s):m_openSensor(%d) fail", __func__, CAMERA_MODE_REPROCESSING);
            return false;
        }

        if (m_openIsp(CAMERA_MODE_REPROCESSING) == false) {
            CLOGE("ERR(%s):m_openIsp(%d) fail", __func__, CAMERA_MODE_REPROCESSING);
            goto err;
        }
        if (m_openIs3a0(CAMERA_MODE_REPROCESSING) == false) {
            CLOGE("ERR(%s):m_openIs3a0(%d) fail", __func__, CAMERA_MODE_REPROCESSING);
            goto err;
        }
        if (m_openPicture(CAMERA_MODE_REPROCESSING) == false) {
            CLOGE("ERR(%s):m_openPicture(%d) fail", __func__, CAMERA_MODE_REPROCESSING);
            goto err;
        }
        if (m_openPreview(CAMERA_MODE_REPROCESSING) == false) {
            CLOGE("ERR(%s):m_openPreview(%d) fail", __func__, CAMERA_MODE_REPROCESSING);
            goto err;
        }
    } else if (cameraMode == CAMERA_MODE_FRONT) {
        if (m_openSensor(cameraMode) == false) {
            CLOGE("ERR(%s):m_openSensor(%d) fail", __func__, cameraMode);
            return false;
        }

        if (m_openIsp(cameraMode) == false) {
            CLOGE("ERR(%s):m_openIsp(%d) fail", __func__, cameraMode);
            goto err;
        }

        if (m_openIs3a0(cameraMode) == false) {
            CLOGE("ERR(%s):m_openIs3a0(%d) fail", __func__, cameraMode);
            goto err;
        }

        if (m_openPicture(cameraMode) == false) {
            CLOGE("ERR(%s):m_openPicture(%d) fail", __func__, cameraMode);
            goto err;
        }

        if (m_openPreview(cameraMode) == false) {
            CLOGE("ERR(%s):m_openPreview(%d) fail", __func__, cameraMode);
            goto err;
        }
    }

    return true;

err:
#ifdef USE_VDIS
    m_closeVdisOut();
#endif
    m_closePreview(cameraMode);
    m_closePicture(cameraMode);

    m_closeIs3a0(CAMERA_MODE_FRONT);
    m_closeIs3a0(CAMERA_MODE_BACK);
    m_closeIs3a0(CAMERA_MODE_REPROCESSING);

    m_closeIs3a1(CAMERA_MODE_FRONT);
    m_closeIs3a1(CAMERA_MODE_BACK);
    m_closeIs3a1(CAMERA_MODE_REPROCESSING);
    m_closeIsp(cameraMode);

    m_closePreview(m_cameraMode);
    m_closePicture(m_cameraMode);
    m_closeIsp(CAMERA_MODE_REPROCESSING);

    return false;
}

bool ExynosCamera::m_openExternalISP(int cameraMode)
{
    char node[30];
    struct media_link   *links = NULL;

    //////////////////////////////
    //  external ISP
    //////////////////////////////
    // media device open
    m_media = exynos_media_open(MEDIA_DEV_EXTERNAL_ISP);
    if (m_media == NULL) {
        CLOGE("ERR(%s):Cannot open media device (error : %s)", __func__, strerror(errno));
        goto err;
    }

    //////////////////
    // GET ENTITIES
    //////////////////
    // camera subdev
    memset(&node, 0x00, sizeof(node));
    snprintf(node, sizeof(node), "%s", M5MOLS_ENTITY_NAME);
    CLOGV("DEBUG(%s):node : %s", __func__, node);
    m_sensorEntity = exynos_media_get_entity_by_name(m_media, node, strlen(node));
    CLOGV("DEBUG(%s):m_sensorEntity : 0x%p", __func__, m_sensorEntity);

    // mipi subdev
    memset(&node, 0x00, sizeof(node));
    snprintf(node, sizeof(node), "%s.%d", PFX_SUBDEV_ENTITY_MIPI_CSIS, MIPI_NUM);
    CLOGV("DEBUG(%s):node : %s", __func__, node);
    m_mipiEntity = exynos_media_get_entity_by_name(m_media, node, strlen(node));
    CLOGV("DEBUG(%s):m_mipiEntity : 0x%p", __func__, m_mipiEntity);

    // fimc-lite subdev
    memset(&node, 0x00, sizeof(node));
    snprintf(node, sizeof(node), "%s.%d", PFX_SUBDEV_ENTITY_FLITE, FLITE_NUM);
    CLOGV("DEBUG(%s):node : %s", __func__, node);
    m_fliteSdEntity = exynos_media_get_entity_by_name(m_media, node, strlen(node));
    CLOGV("DEBUG(%s):m_fliteSdEntity : 0x%p", __func__, m_fliteSdEntity);

    // fimc-lite videodev
    memset(&node, 0x00, sizeof(node));
    snprintf(node, sizeof(node), "%s.%d", PFX_VIDEODEV_ENTITY_FLITE, FLITE_NUM);
    CLOGV("DEBUG(%s):node : %s", __func__, node);
    m_fliteVdEntity = exynos_media_get_entity_by_name(m_media, node, strlen(node));
    CLOGV("DEBUG(%s):m_fliteVdEntity : 0x%p", __func__, m_fliteVdEntity);

    // gscaler subdev
    memset(&node, 0x00, sizeof(node));
    snprintf(node, sizeof(node), "%s.%d", PFX_SUBDEV_ENTITY_GSC_CAP, GSC_NUM);
    CLOGV("DEBUG(%s):node : %s", __func__, node);
    m_gscSdEntity = exynos_media_get_entity_by_name(m_media, node, strlen(node));
    CLOGV("DEBUG(%s):m_gscSdEntity : 0x%p", __func__, m_gscSdEntity);

    // gscaler videodev
    memset(&node, 0x00, sizeof(node));
    snprintf(node, sizeof(node), "%s.%d", PFX_VIDEODEV_ENTITY_GSC_CAP, GSC_NUM);
    CLOGV("DEBUG(%s):node : %s", __func__, node);
    m_gscVdEntity = exynos_media_get_entity_by_name(m_media, node, strlen(node));
    CLOGV("DEBUG(%s):m_gscVdEntity : 0x%p", __func__, m_gscVdEntity);

    CLOGV("DEBUG(%s):sensor_sd : numlink : %d", __func__, m_sensorEntity->num_links);
    CLOGV("DEBUG(%s):mipi_sd   : numlink : %d", __func__, m_mipiEntity->num_links);
    CLOGV("DEBUG(%s):flite_sd  : numlink : %d", __func__, m_fliteSdEntity->num_links);
    CLOGV("DEBUG(%s):flite_vd  : numlink : %d", __func__, m_fliteVdEntity->num_links);
    CLOGV("DEBUG(%s):gsc_sd    : numlink : %d", __func__, m_gscSdEntity->num_links);
    CLOGV("DEBUG(%s):gsc_vd    : numlink : %d", __func__, m_gscVdEntity->num_links);

    //////////////////
    // SETUP LINKS
    //////////////////
    // sensor subdev to mipi subdev
    links = m_sensorEntity->links;
    if (links == NULL ||
        links->source->entity != m_sensorEntity ||
        links->sink->entity != m_mipiEntity) {
        CLOGE("ERR(%s):Cannot make link camera sensor to mipi", __func__);
        goto err;
    }

    if (exynos_media_setup_link(m_media,  links->source,  links->sink, MEDIA_LNK_FL_ENABLED) < 0) {
        CLOGE("ERR(%s):Cannot make setup camera sensor to mipi", __func__);
        goto err;
    }
    CLOGV("DEBUG(%s):[LINK SUCCESS] sensor subdev to mipi subdev", __func__);

    // mipi subdev to fimc-lite subdev
    for (unsigned int i = 0; i < m_mipiEntity->num_links; i++) {
        links = &m_mipiEntity->links[i];
        CLOGV("DEBUG(%s):i=%d: links->source->entity : %p, m_mipiEntity : %p", __func__, i,
                links->source->entity, m_mipiEntity);
        CLOGV("DEBUG(%s):i=%d: links->sink->entity : %p, m_fliteSdEntity : %p", __func__, i,
                links->sink->entity, m_fliteSdEntity);
        if (links == NULL ||
            links->source->entity != m_mipiEntity ||
            links->sink->entity != m_fliteSdEntity) {
            continue;
        } else if (exynos_media_setup_link(m_media,  links->source,  links->sink, MEDIA_LNK_FL_ENABLED) < 0) {
            CLOGE("ERR(%s):Cannot make setup mipi subdev to fimc-lite subdev", __func__);
            goto err;
        }
    }
    CLOGV("DEBUG(%s):[LINK SUCCESS] mipi subdev to fimc-lite subdev", __func__);

    // fimc-lite subdev TO fimc-lite video dev
    for (unsigned int i = 0; i < m_fliteSdEntity->num_links; i++) {
        links = &m_fliteSdEntity->links[i];
        CLOGV("DEBUG(%s):i=%d: links->source->entity : %p, m_fliteSdEntity : %p", __func__, i,
            links->source->entity, m_fliteSdEntity);
        CLOGV("DEBUG(%s):i=%d: links->sink->entity : %p, m_fliteVdEntity : %p", __func__, i,
            links->sink->entity, m_fliteVdEntity);
        if (links == NULL ||
            links->source->entity != m_fliteSdEntity ||
            links->sink->entity != m_fliteVdEntity) {
            continue;
        } else if (exynos_media_setup_link(m_media,  links->source,  links->sink, MEDIA_LNK_FL_ENABLED) < 0) {
            CLOGE("ERR(%s):Cannot make setup fimc-lite subdev to fimc-lite video dev", __func__);
            goto err;
        }
    }
    CLOGV("DEBUG(%s):[LINK SUCCESS] fimc-lite subdev to fimc-lite video dev", __func__);

    // fimc-lite subdev to gscaler subdev
    for (unsigned int i = 0; i < m_gscSdEntity->num_links; i++) {
        links = &m_gscSdEntity->links[i];
        CLOGV("DEBUG(%s):i=%d: links->source->entity : %p, m_fliteSdEntity : %p", __func__, i,
                links->source->entity, m_fliteSdEntity);
        CLOGV("DEBUG(%s):i=%d: links->sink->entity : %p, m_gscSdEntity : %p", __func__, i,
                links->sink->entity, m_gscSdEntity);
        if (links == NULL ||
            links->source->entity != m_fliteSdEntity ||
            links->sink->entity != m_gscSdEntity) {
            continue;
        } else if (exynos_media_setup_link(m_media, links->source, links->sink, MEDIA_LNK_FL_ENABLED) < 0) {
            CLOGE("ERR(%s):Cannot make setup fimc-lite subdev to gscaler subdev", __func__);
            goto err;
        }
    }
    CLOGV("DEBUG(%s):[LINK SUCCESS] fimc-lite subdev to gscaler subdev", __func__);

    // gscaler subdev to gscaler video dev
    for (unsigned int i = 0; i < m_gscVdEntity->num_links; i++) {
        links = &m_gscVdEntity->links[i];
        CLOGV("DEBUG(%s):i=%d: links->source->entity : %p, m_gscSdEntity : %p", __func__, i,
                links->source->entity, m_gscSdEntity);
        CLOGV("DEBUG(%s):i=%d: links->sink->entity : %p, m_gscVdEntity : %p", __func__, i,
                links->sink->entity, m_gscVdEntity);
        if (links == NULL ||
            links->source->entity != m_gscSdEntity ||
            links->sink->entity != m_gscVdEntity) {
            continue;
        } else if (exynos_media_setup_link(m_media, links->source, links->sink, MEDIA_LNK_FL_ENABLED) < 0) {
            CLOGE("ERR(%s):Cannot make setup gscaler subdev to gscaler video dev", __func__);
            goto err;
        }
    }
    CLOGV("DEBUG(%s):[LINK SUCCESS] gscaler subdev to gscaler video dev", __func__);

    memset(&node, 0x00, sizeof(node));
    snprintf(node, sizeof(node), "%s%d", PFX_NODE, (FLITE_VD_NODE_OFFSET + VIDEO_NODE_PREVIEW_ID));
    m_camera_info[CAMERA_MODE_BACK].preview.fd = exynos_v4l2_open(node, O_RDWR, 0);
    if (m_camera_info[CAMERA_MODE_BACK].preview.fd <= 0) {
        CLOGE("ERR(%s):exynos_v4l2_open(%s) fail (error : %s)", __func__, node, strerror(errno));
        goto err;
    }
    return true;

err:
    if (m_media)
        exynos_media_close(m_media);
    m_media = NULL;

    return false;
}

bool ExynosCamera::flagOpen(int cameraId)
{
    return m_flagOpen[cameraId];
}


int ExynosCamera::getCameraId(void)
{
    return m_cameraId;
}

int ExynosCamera::getCameraMode()
{
    return m_cameraMode;
}

char *ExynosCamera::getCameraName(void)
{
    return m_cameraName;
}

bool ExynosCamera::startPreview(void)
{
    CLOGD("DEBUG(%s):in", __func__);

    int index;

    if (m_flagCreate == false) {
        CLOGE("ERR(%s):Not yet Created", __func__);
        return false;
    }

    if (m_recordingHint == true) {
        if (set3DNR(true) == false)
            CLOGE("ERR(%s):set3DNR() fail", __func__);
    }

    CLOGD("DEBUG(%s):(flagStart : %d), (size : %d x %d), (format : 0x%x)",
        __func__,
        m_camera_info[m_cameraMode].preview.flagStart,
        m_camera_info[m_cameraMode].preview.width,
        m_camera_info[m_cameraMode].preview.height,
        m_camera_info[m_cameraMode].preview.format);

    if (m_camera_info[m_cameraMode].preview.flagStart == false) {
        m_camera_info[m_cameraMode].preview.width  = m_curCameraInfo[m_cameraMode]->previewW;
        m_camera_info[m_cameraMode].preview.height = m_curCameraInfo[m_cameraMode]->previewH;
        m_camera_info[m_cameraMode].preview.format = m_curCameraInfo[m_cameraMode]->previewColorFormat;
        m_camera_info[m_cameraMode].preview.planes = m_curCameraInfo[m_cameraMode]->previewBufPlane;
        m_camera_info[m_cameraMode].preview.memory = V4L2_CAMERA_MEMORY_TYPE;
        m_camera_info[m_cameraMode].preview.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
        m_camera_info[m_cameraMode].preview.ionClient = m_ionCameraClient;

        m_camera_info[m_cameraMode].preview.buffers = NUM_PREVIEW_BUFFERS;

        // for frame sync
        for (int i = 0; i < NUM_PREVIEW_BUFFERS; i++) {
            m_camera_info[m_cameraMode].preview.buffer[i].size.extS[m_camera_info[m_cameraMode].preview.planes - 1]
                = ALIGN(META_DATA_SIZE, PAGE_SIZE);

            if (allocMemSinglePlane(m_ionCameraClient,
                                    &m_camera_info[m_cameraMode].preview.buffer[i],
                                    m_camera_info[m_cameraMode].preview.planes - 1,
                                    true) == false) {
                CLOGE("ERR(%s):m_allocCameraMemorySingle() fail", __func__);
                goto err;
            } else {
                memset(m_camera_info[m_cameraMode].preview.buffer[i].virt.extP[m_curCameraInfo[m_cameraMode]->previewBufPlane - 1],
                        0, m_camera_info[m_cameraMode].preview.buffer[i].size.extS[m_curCameraInfo[m_cameraMode]->previewBufPlane - 1]);
            }

            CLOGD("DEBUG(%s): index(%d), meta addr(%x)", __func__, i,
                m_camera_info[m_cameraMode].preview.buffer[i].virt.extP[m_curCameraInfo[m_cameraMode]->previewBufPlane - 1]);

            m_previewMetaBuffer[i].virt.p = m_camera_info[m_cameraMode].preview.buffer[i].virt.extP[m_curCameraInfo[m_cameraMode]->previewBufPlane - 1];
            m_previewMetaBuffer[i].size.s = m_camera_info[m_cameraMode].preview.buffer[i].size.extS[m_curCameraInfo[m_cameraMode]->previewBufPlane - 1];
            m_previewMetaBuffer[i].fd.fd  = m_camera_info[m_cameraMode].preview.buffer[i].fd.extFd[m_curCameraInfo[m_cameraMode]->previewBufPlane - 1];
        }

        if (m_startPreview() == false) {
            CLOGE("ERR(%s):m_startPreview() fail", __func__);
            goto err;
        }

        m_camera_info[m_cameraMode].preview.flagStart = true;
    }

    return true;

err:
    if (this->stopPreview() == false)
        CLOGE("ERR(%s):stopPreview() fail", __func__);

    return false;
}

bool ExynosCamera::m_startPreview(void)
{
    int index;
    int BDS_w = 0;
    int BDS_h = 0;

    int sensorId = m_getSensorId(m_cameraMode);
    sensorId = (0 << REPROCESSING_SHFIT) | ((FIMC_IS_VIDEO_SCP_NUM - FIMC_IS_VIDEO_SEN0_NUM) << VIDEO_INDEX_SHFIT) | (sensorId << 0);

    if (cam_int_s_input(&(m_camera_info[m_cameraMode].preview), sensorId) < 0) {
        CLOGE("ERR(%s):cam_int_s_input(%d) fail", __func__, sensorId);
        return false;
    }

    if (cam_int_s_fmt(&m_camera_info[m_cameraMode].preview) < 0) {
        CLOGE("ERR(%s):cam_int_s_fmt() fail", __func__);
        return false;
    }

    if (cam_int_reqbufs(&m_camera_info[m_cameraMode].preview) < 0) {
        CLOGE("ERR(%s):cam_int_reqbufs() fail", __func__);
        return false;
    }

    m_camera_info[m_cameraMode].dummy_shot.request_scp = 1;

    for (int i = 0; i < VIDEO_MAX_FRAME; i++) {
        if (m_camera_info[m_cameraMode].preview.buffer[i].virt.p != NULL ||
            m_camera_info[m_cameraMode].preview.buffer[i].phys.p != 0) {
            CLOGV("DEBUG(%s): preview buffer[%d], virt.P = %x", __func__, i, m_camera_info[m_cameraMode].preview.buffer[i].virt.p);
            if (cam_int_qbuf(&m_camera_info[m_cameraMode].preview, i) < 0) {
                CLOGE("ERR(%s):cam_int_qbuf(%d) fail", __func__, i);
                return false;
            }
        }
    }

    if (cam_int_streamon(&(m_camera_info[m_cameraMode].preview)) < 0) {
        CLOGE("ERR(%s):cam_int_streamon(preview) fail", __func__);
        return false;
    }
#ifdef USE_VDIS
    if ((m_recordingHint == true) &&
        (getCameraId() == CAMERA_ID_BACK) &&
        (getVideoStabilization() == true)) {

        if (startVdisCapture() == false) {
            CLOGE("ERR(%s):Fail on m_secCamera->startVdisCapture()", __func__);
            return false;
        }

        if (startVdisOutput() == false) {
            CLOGE("ERR(%s):Fail on m_secCamera->startVdisOutput()", __func__);
            return false;
        }

        if (cam_int_streamon(&(m_camera_info[m_cameraMode].vdisc)) < 0) {
            CLOGE("ERR(%s):cam_int_streamon(preview) fail", __func__);
            return false;
        }

        if (cam_int_streamon(&(m_camera_info[m_cameraMode].vdiso)) < 0) {
            CLOGE("ERR(%s):cam_int_streamon(preview) fail", __func__);
            return false;
        }
    }
#endif

    // HACK : This from startIsp
    if (cam_int_streamon(&(m_camera_info[m_cameraMode].isp)) < 0) {
        CLOGE("ERR(%s):cam_int_streamon(isp) fail", __func__);
        return false;
    }

#ifdef FORCE_LEADER_OFF
    m_forceIspOff = 0;
#endif

    /* zoom setting */
    int zoom, srcW, srcH, dstW, dstH;
    zoom = m_curCameraInfo[m_cameraMode]->zoom;
    srcW = m_curCameraInfo[m_cameraMode]->pictureW;
    srcH = m_curCameraInfo[m_cameraMode]->pictureH;
    dstW = m_curCameraInfo[m_cameraMode]->ispW;
    dstH = m_curCameraInfo[m_cameraMode]->ispH;

#ifdef SCALABLE_SENSOR
    if (getScalableSensorStart() == true) {
        CLOGD("DEBUG(%s):zoom setting(ScalableSensor)", __func__);
        getScalableSensorSizeOnPreview(&srcW, &srcH);
    }
#endif

    if (m_setZoom(zoom, srcW, srcH, dstW, dstH,
                  (void *)&m_camera_info[m_cameraMode].dummy_shot) == false) {
        CLOGE("ERR(%s):m_setZoom() fail", __func__);
    }

    bool toggle = getVideoStabilization();
    if (setVideoStabilization(toggle) == false)
        CLOGE("ERR(%s):setVideoStabilization() fail", __func__);

    return true;
}

bool ExynosCamera::qAll3a1Buf(void)
{
    int cameraMode = m_cameraMode;
    if (m_flagCreate == false) {
        CLOGD("ERR(%s):Not yet created", __func__);
        return true;
    }

    if (m_camera_info[cameraMode].is3a1Src.flagStart == false) {
        CLOGD("ERR(%s):Not yet is3a1Src started", __func__);
        return true;
    }

    if (m_camera_info[cameraMode].is3a1Dst.flagStart == false) {
        CLOGD("ERR(%s):Not yet is3a1Dst started", __func__);
        return true;
    }

    if (m_setSetfile(cameraMode) == false) {
        CLOGE("ERR(%s):m_setSetfile(%d) fail", __func__, cameraMode);
        return false;
    }

    /* qbuf at least 3 buffers */
    for (int i = 0; i < VIDEO_MAX_FRAME; i++) {
        if (m_camera_info[cameraMode].isp.buffer[i].virt.p != NULL ||
            m_camera_info[cameraMode].isp.buffer[i].phys.p != 0) {

            //metadata buffer
            memset(m_metaBuf[i].virt.extP[1], 0, m_metaBuf[i].size.extS[1]);
            if (cam_int_qbuf(&(m_camera_info[cameraMode].is3a1Dst), i,
                             &(m_camera_info[cameraMode].isp),
                             m_metaBuf[i]) < 0) {
                CLOGE("ERR(%s):is3a1Dst cam_int_qbuf(%d) fail", __func__, i);
                return false;
            }
        }
    }

    /* qbuf at least 3 buffers */
    for (int i = 0; i < VIDEO_MAX_FRAME; i++) {
        if (m_camera_info[cameraMode].sensor.buffer[i].virt.p != NULL ||
            m_camera_info[cameraMode].sensor.buffer[i].phys.p != 0) {
            struct camera2_shot_ext *shot_ext;
            shot_ext = (struct camera2_shot_ext *)m_camera_info[cameraMode].sensor.buffer[i].virt.extP[1];

            memcpy(&shot_ext->shot.ctl, &m_camera_info[cameraMode].dummy_shot.shot.ctl, sizeof(struct camera2_ctl));

            shot_ext->setfile = m_camera_info[cameraMode].dummy_shot.setfile;
            shot_ext->request_3ax = m_camera_info[cameraMode].dummy_shot.request_3ax;
            shot_ext->request_isp = m_camera_info[cameraMode].dummy_shot.request_isp;
            shot_ext->request_scc = m_camera_info[cameraMode].dummy_shot.request_scc;
            shot_ext->request_scp = m_camera_info[cameraMode].dummy_shot.request_scp;
            shot_ext->request_dis = m_camera_info[cameraMode].dummy_shot.request_dis;

            shot_ext->dis_bypass = m_camera_info[cameraMode].dummy_shot.dis_bypass;
            shot_ext->dnr_bypass = m_camera_info[cameraMode].dummy_shot.dnr_bypass;
            shot_ext->fd_bypass = m_camera_info[cameraMode].dummy_shot.fd_bypass;
            shot_ext->shot.magicNumber= m_camera_info[cameraMode].dummy_shot.shot.magicNumber;

            if (cam_int_qbuf(&(m_camera_info[cameraMode].is3a1Src), i,
                             &(m_camera_info[cameraMode].sensor)) < 0) {
                CLOGE("ERR(%s):is3a1Src cam_int_qbuf(%d) fail", __func__, i);
                return false;
            }
        }
    }
    return true;
}

bool ExynosCamera::dqAll3a1Buf(void)
{
    int srcIndex;
    int dstIndex;
    struct camera2_shot_ext *shot_ext_src;
    struct camera2_shot_ext *shot_ext_dst;
    int cameraMode = m_cameraMode;

    if (m_flagCreate == false) {
        CLOGD("ERR(%s):Not yet created", __func__);
        return true;
    }

    if (m_camera_info[cameraMode].is3a1Src.flagStart == false) {
        CLOGD("ERR(%s):Not yet is3a1Src started", __func__);
        return true;
    }

    if (m_camera_info[cameraMode].is3a1Dst.flagStart == false) {
        CLOGD("ERR(%s):Not yet is3a1Dst started", __func__);
        return true;
    }

    for (int i = 0; i < NUM_BAYER_BUFFERS; i++) {
        dstIndex = cam_int_dqbuf(&(m_camera_info[cameraMode].is3a1Dst));
        if (dstIndex < 0) {
            CLOGE("ERR(%s):is3a1Dst cam_int_dqbuf() fail", __func__);
            return false;
        }

        m_is3a1DstLastBufIndex = dstIndex;
        shot_ext_dst = (struct camera2_shot_ext *)m_camera_info[cameraMode].isp.buffer[dstIndex].virt.extP[1];

        srcIndex = cam_int_dqbuf(&(m_camera_info[cameraMode].is3a1Src));
        if (srcIndex < 0) {
            CLOGE("ERR(%s):is3a1Src cam_int_dqbuf() fail", __func__);
            return false;
        }

        m_is3a1SrcLastBufIndex = srcIndex;
        shot_ext_src = (struct camera2_shot_ext *)m_camera_info[cameraMode].sensor.buffer[srcIndex].virt.extP[1];

        CLOGV("DEBUG(%s): index(output:%d, capture:%d), (free:%d) (request:%d) (process:%d) (complete:%d)",
            __func__, srcIndex, dstIndex,
            shot_ext_src->free_cnt,
            shot_ext_src->request_cnt,
            shot_ext_src->process_cnt,
            shot_ext_src->complete_cnt);

        if (shot_ext_src->request_cnt || shot_ext_src->process_cnt || shot_ext_src->complete_cnt)
            continue;
        else
            break;

    }

    return true;
}

bool ExynosCamera::m_stopPreview(void)
{
    bool ret = true;

    if (m_flagCreate == false) {
        CLOGW("WARN(%s):Not yet Created", __func__);
        return false;
    }

    if (setFlashMode(FLASH_MODE_OFF) == false)
        CLOGE("ERR(%s):setFlashMode() fail", __func__);

    if (m_autofocusMgr->flagLockAutofocus() == true)
        m_autofocusMgr->unlockAutofocus();

    if (m_autofocusMgr->flagAutofocusStart() == true)
        m_autofocusMgr->stopAutofocus();

    if (getVideoStabilization() == true) {
        if (setVideoStabilization(false) == false)
            CLOGE("ERR(%s):setVideoStabilization() fail", __func__);
    }

    if (set3DNR(false) == false)
        CLOGE("ERR(%s):set3DNR() fail", __func__);

#ifdef THREAD_PROFILE
    timeUs = 0;
    gettimeofday(&mTimeStart, NULL);
#endif
    if (cam_int_streamoff(&m_camera_info[m_cameraMode].preview) < 0) {
        CLOGE("ERR(%s):exynos_v4l2_streamoff() fail", __func__);
        ret = false;
    }
#ifdef THREAD_PROFILE
    gettimeofday(&mTimeStop, NULL);
    timeUs = (mTimeStop.tv_sec*1000000 + mTimeStop.tv_usec) - (mTimeStart.tv_sec*1000000 + mTimeStart.tv_usec);
    CLOGD("DEBUG(%s):time_check elapsed time=(%ld)us", __func__, timeUs);
#endif

    m_camera_info[m_cameraMode].dummy_shot.request_scp = 0;
    if (m_recordingHint == true && getCameraId() == CAMERA_ID_BACK) {
        m_camera_info[m_cameraMode].dummy_shot.request_dis = 0;
    }

    if (0 < m_camera_info[m_cameraMode].preview.buffers) {
        if (cam_int_clrbufs(&m_camera_info[m_cameraMode].preview) < 0) {
            CLOGE("ERR(%s):cam_int_clrbufs() fail", __func__);
            ret = false;
        }
    }
    m_flagStartFaceDetection = false;

done:
    return ret;
}

bool ExynosCamera::stopPreview(void)
{
    CLOGD("DEBUG(%s):in", __func__);

    bool ret = true;
    int previewPlane;

    if (m_flagCreate == false) {
        CLOGW("WARN(%s):Not yet Created", __func__);
        return false;
    }

    if (m_camera_info[m_cameraMode].preview.flagStart == true) {
        if (m_stopPreview() == false) {
            CLOGE("ERR(%s):m_stopPreview() fail", __func__);
            ret = false;
        }

        previewPlane = m_camera_info[m_cameraMode].preview.planes - 1;

        // for frame sync
        for (int i = 0; i < VIDEO_MAX_FRAME; i++)
            freeMemSinglePlane(&m_camera_info[m_cameraMode].preview.buffer[i], previewPlane);

        m_camera_info[m_cameraMode].preview.flagStart = false;
    }

    return ret;
}

bool ExynosCamera::flagStartPreview(void)
{
    return m_camera_info[m_cameraMode].preview.flagStart;
}

int ExynosCamera::getPreviewMaxBuf(void)
{
    return VIDEO_MAX_FRAME;
}

bool ExynosCamera::setPreviewBuf(ExynosBuffer *buf)
{
    int index = buf->reserved.p;
    if (m_flagCreate == false) {
        CLOGE("ERR(%s):Not yet created fail", __func__);
        return false;
    }

    if (VIDEO_MAX_FRAME <= buf->reserved.p) {
        CLOGE("ERR(%s):index(%d) must smaller than %d", __func__, buf->reserved.p, VIDEO_MAX_FRAME);
        return false;
    }

    m_camera_info[m_cameraMode].preview.buffer[index] = *buf;

    // for frame sync
    int planeNum = m_camera_info[m_cameraMode].preview.planes - 1;

    m_camera_info[m_cameraMode].preview.buffer[index].virt.extP[planeNum] = m_previewMetaBuffer[index].virt.p;
    m_camera_info[m_cameraMode].preview.buffer[index].size.extS[planeNum] = m_previewMetaBuffer[index].size.s;
    m_camera_info[m_cameraMode].preview.buffer[index].fd.extFd[planeNum] = m_previewMetaBuffer[index].fd.fd;

    return true;
}

bool ExynosCamera::cancelPreviewBuf(int index)
{
    CLOGD("DEBUG(%s):in", __func__);

    if (m_flagCreate == false) {
        CLOGE("ERR(%s):Not yet created fail", __func__);
        return false;
    }

    if (m_camera_info[m_cameraMode].preview.flagStart == true) {
        CLOGE("ERR(%s):preview is running, cannot cancel preview buffer!!", __func__);
        return false;
    }

    for (int i = 0; i < m_curCameraInfo[m_cameraMode]->previewBufPlane; i++) {
        m_camera_info[m_cameraMode].preview.buffer[index].fd.extFd[i] = -1;
        m_camera_info[m_cameraMode].preview.buffer[index].virt.extP[i] = NULL;
        m_camera_info[m_cameraMode].preview.buffer[index].size.extS[i] = 0;
    }

    return true;
}

bool ExynosCamera::clearPreviewBuf(void)
{
    CLOGD("DEBUG(%s):in", __func__);

    if (m_flagCreate == false) {
        CLOGE("ERR(%s):Not yet created fail", __func__);
        return false;
    }

    if (m_camera_info[m_cameraMode].preview.flagStart == true) {
        CLOGE("ERR(%s):preview is running, cannot clear preview buffer!!", __func__);
        return false;
    }

    int planeNum = m_camera_info[m_cameraMode].preview.planes;

    for (int i = 0; i < VIDEO_MAX_FRAME; i++) {
        for (int j = 0; j < planeNum; j++) {
            m_camera_info[m_cameraMode].preview.buffer[i].fd.extFd[j] = -1;
            m_camera_info[m_cameraMode].preview.buffer[i].virt.extP[j] = NULL;
            m_camera_info[m_cameraMode].preview.buffer[i].size.extS[j] = 0;
        }
    }

    return true;
}

bool ExynosCamera::allocInternalPreviewBuf(ExynosBuffer *buf)
{
    CLOGD("DEBUG(%s):in", __func__);

    if (getFlagInternalPreviewBuf() == false) {
        CLOGE("ERR(%s):getFlagInternalPreviewBuf() == false", __func__);
        return false;
    }

    if (allocMem(m_ionCameraClient, buf, (1 << 1) | (1 << 2) | (1 << 3)) == false) {
        CLOGE("ERR(%s):allocMem() fail", __func__);
        return false;
    }

    return true;
}

void ExynosCamera::releaseInternalPreviewBuf(int index)
{
    CLOGD("DEBUG(%s):in", __func__);

    if (getFlagInternalPreviewBuf() == false) {
        CLOGE("ERR(%s):getFlagInternalPreviewBuf() == false", __func__);
        return;
    }

    freeMem(&m_camera_info[m_cameraMode].preview.buffer[index]);

    m_camera_info[m_cameraMode].preview.buffers = 0;
}

void ExynosCamera::releaseAllInternalPreviewBuf(void)
{
    CLOGD("DEBUG(%s):in", __func__);

    for (int i = 0; i < NUM_PREVIEW_BUFFERS; i++)
        releaseInternalPreviewBuf(i);
}

void ExynosCamera::setFlagInternalPreviewBuf(bool flag)
{
    CLOGD("DEBUG(%s):in flag(%d)", __func__, flag);

    m_previewInternalMemAlloc = flag;
}

bool ExynosCamera::getFlagInternalPreviewBuf(void)
{
    return m_previewInternalMemAlloc;
}

bool ExynosCamera::getPreviewBuf(ExynosBuffer *buf, bool *isValid, nsecs_t *timestamp)
{
    int index = 0;
    struct camera2_stream *metadata;
    bool found = false;

    if (m_flagCreate == false) {
        CLOGE("ERR(%s):Not yet created fail", __func__);
        return false;
    }

    if (m_camera_info[m_cameraMode].preview.flagStart == false) {
        CLOGE("ERR(%s):Not yet preview started fail", __func__);
        return false;
    }

    CLOGT(m_traceCount, "(%s): dq in", __func__);

#ifdef USE_CAMERA_ESD_RESET
    if (cam_int_polling(&(m_camera_info[m_cameraMode].preview)) < 0) {
        CLOGE("[esdtest](%s): polling() fail", __func__);
        return false;
    }
#endif

    index = cam_int_dqbuf(&(m_camera_info[m_cameraMode].preview));
    if (index < 0) {
        CLOGE("ERR(%s):cam_int_dqbuf() on preview fail", __func__);
        return false;
    }

    CLOGT(m_traceCount, "(%s): dq out(index %d)", __func__, index);

    m_sCaptureMgr->execFunction(ExynosCameraActivityBase::CALLBACK_TYPE_SCP_BEFORE,
        (void *)&(m_camera_info[m_cameraMode].preview.buffer[index]));

    *buf = m_camera_info[m_cameraMode].preview.buffer[index];
    buf->reserved.p = index;

    metadata = (struct camera2_stream *)(m_camera_info[m_cameraMode].preview.buffer[index].virt.extP[m_curCameraInfo[m_cameraMode]->previewBufPlane - 1]);
    if (metadata) {
        *isValid = metadata->fvalid ? true : false;

        CLOGT(m_traceCount, "(%s:%d) index(%d, %p), fcount(%d), rcount(%d)", __func__, __LINE__,
                index, m_camera_info[m_cameraMode].preview.buffer[index].virt.extP[m_curCameraInfo[m_cameraMode]->previewBufPlane - 1],
                metadata->fcount, metadata->rcount);

        if (timestamp) {
            *timestamp = 0;
            struct camera2_shot_ext *shot_ext_temp;

            for (int i = 0; i <  m_camera_info[m_cameraMode].isp.buffers; i++) {
                shot_ext_temp = (struct camera2_shot_ext *)m_camera_info[m_cameraMode].isp.buffer[i].virt.extP[1];

                if (shot_ext_temp->shot.dm.request.frameCount == metadata->fcount &&
                    shot_ext_temp->shot.dm.sensor.timeStamp != 0) {
                    *timestamp = (nsecs_t)shot_ext_temp->shot.dm.sensor.timeStamp;
                    found = true;
                }
            }
        }

        if (!found && timestamp)
            *timestamp = systemTime(SYSTEM_TIME_MONOTONIC);
    } else {
        *isValid = false;
        CLOGD("(%s) metadata is null\n", __func__);
        return false;
    }

    return true;
}

bool ExynosCamera::putPreviewBuf(ExynosBuffer *buf)
{
    if (m_flagCreate == false) {
        CLOGE("ERR(%s):Not yet created fail", __func__);
        return false;
    }

    if (m_camera_info[m_cameraMode].preview.flagStart == false) {
        CLOGE("ERR(%s):Not yet preview started fail", __func__);
        return false;
    }

    if (cam_int_qbuf(&(m_camera_info[m_cameraMode].preview), buf->reserved.p) < 0) {
        CLOGE("ERR(%s):cam_int_qbuf(%d) fail", __func__, buf->reserved.p);
        return false;
    }

    return true;
}

bool ExynosCamera::getIs3a0Buf(enum CAMERA_MODE cameraMode, ExynosBuffer *inBuf, ExynosBuffer *outBuf)
{
    int srcIndex = 0;
    int dstIndex = 0;
    unsigned int fcount_buf = 0;
    int position_buf = inBuf->reserved.p;
    struct camera2_shot_ext * shot_ext_src = NULL;
    struct camera2_shot_ext * shot_ext_dst = NULL;

    if (m_flagCreate == false) {
        CLOGE("ERR(%s):Not yet created fail", __func__);
        return false;
    }

    if (m_camera_info[cameraMode].is3a0Src.flagStart == false) {
        CLOGE("ERR(%s):Not yet is3a0Src started fail", __func__);
        return false;
    }

    if (m_camera_info[cameraMode].is3a0Dst.flagStart == false) {
        CLOGE("ERR(%s):Not yet is3a0Dst started fail", __func__);
        return false;
    }

    /* use same buffer number */
    *outBuf = m_camera_info[cameraMode].isp.buffer[position_buf];
    outBuf->reserved.p = position_buf;

    m_cameraModeIs3a0 = cameraMode;

    shot_ext_src = (camera2_shot_ext *)inBuf->virt.extP[1];
    fcount_buf = shot_ext_src->shot.dm.request.frameCount;

    memcpy(&shot_ext_src->shot.ctl, &m_camera_info[cameraMode].dummy_shot.shot.ctl, sizeof(struct camera2_ctl));

    m_turnOffEffectByFps(shot_ext_src, m_curCameraInfo[cameraMode]->fpsRange[1]);

    memcpy(&m_camera_info[cameraMode].dummy_shot.shot.ctl, &shot_ext_src->shot.ctl, sizeof(struct camera2_ctl));

    CLOGT(m_traceCount, "(%s): q in", __func__);

    //metadata buffer
    memset(m_metaBuf[position_buf].virt.extP[1], 0, m_metaBuf[position_buf].size.extS[1]);

    if (cam_int_qbuf(&(m_camera_info[cameraMode].is3a0Dst), position_buf,
                     &(m_camera_info[cameraMode].isp),
                     m_metaBuf[position_buf]) < 0) {
        CLOGE("ERR(%s):is3a0Dst cam_int_qbuf(%d) fail", __func__, position_buf);
        m_cameraModeIs3a0 = CAMERA_MODE_MAX;

        return false;
    }

    if (cam_int_qbuf(&(m_camera_info[cameraMode].is3a0Src), position_buf,
                     &(m_camera_info[cameraMode].sensor)) < 0) {
        CLOGE("ERR(%s):is3a0Src cam_int_qbuf(%d) fail", __func__, position_buf);
        m_cameraModeIs3a0 = CAMERA_MODE_MAX;

        return false;
    }

    CLOGT(m_traceCount, "(%s): q out(index %d)", __func__, srcIndex);

    CLOGT(m_traceCount, "(%s): dq in", __func__);

    dstIndex = cam_int_dqbuf(&(m_camera_info[cameraMode].is3a0Dst));
    if (dstIndex < 0) {
        CLOGE("ERR(%s):cam_int_dqbuf() on is3a0 fail", __func__);
        m_cameraModeIs3a0 = CAMERA_MODE_MAX;

        return false;
    }

    srcIndex = cam_int_dqbuf(&(m_camera_info[cameraMode].is3a0Src));
    if (srcIndex < 0) {
        CLOGE("ERR(%s):cam_int_dqbuf() on is3a0 fail", __func__);
        m_cameraModeIs3a0 = CAMERA_MODE_MAX;

        return false;
    }

    CLOGT(m_traceCount, "(%s): dq out(index %d)", __func__, srcIndex);

    if (inBuf->virt.extP[1] == NULL || outBuf->virt.extP[1] == NULL) {
        CLOGE("ERR(%s):inBuf->virt.extP[1] == %p || outBuf->virt.extP[1] == %p",
            __func__, inBuf->virt.extP[1], outBuf->virt.extP[1]);
        m_cameraModeIs3a0 = CAMERA_MODE_MAX;

        return false;
    }

    /* back-up is3aa_dm : result of isp metadata */
    memcpy(&m_camera_info[cameraMode].is3aa_dm, shot_ext_src, sizeof(struct camera2_shot_ext));

    /* copy 3a0 to isp */
    memcpy(outBuf->virt.extP[1], inBuf->virt.extP[1], META_DATA_SIZE);

    shot_ext_src = (camera2_shot_ext *)inBuf->virt.extP[1];
    if (shot_ext_src->shot.dm.request.frameCount != fcount_buf) {
        shot_ext_src = (camera2_shot_ext *)inBuf->virt.extP[1];
        shot_ext_dst = (camera2_shot_ext *)outBuf->virt.extP[1];

        CLOGD("DEBUG(%s):(%d) (%d %d)", __func__, __LINE__, shot_ext_src->shot.dm.request.frameCount, fcount_buf);
        shot_ext_dst->shot.dm.request.frameCount = fcount_buf;
    }

    if (shot_ext_src->request_3ax != m_camera_info[cameraMode].dummy_shot.request_3ax) {
        outBuf->reserved.p = -1;
        m_traceCount = TRACE_COUNT;
        CLOGE("ERR(%s):3a0 Shot done is invalid(expected request_3ax is %d, but we got %d), skip frame(%d)",
                __func__, m_camera_info[cameraMode].dummy_shot.request_3ax, shot_ext_src->request_3ax, shot_ext_src->shot.dm.request.frameCount);
    }

    return true;
}

bool ExynosCamera::getIs3a0BufReprocessing(enum CAMERA_MODE cameraMode, ExynosBuffer *inBuf, ExynosBuffer *outBuf)
{
    int srcIndex = 0;
    int dstIndex = 0;
    unsigned int fcount_buf = 0;
    int position_buf = inBuf->reserved.p;
    struct camera2_shot_ext * shot_ext = NULL;
    struct camera2_shot_ext * shot_ext_src = NULL;
    struct camera2_shot_ext * shot_ext_dst = NULL;
    unsigned int request_3ax = 0;

    if (m_flagCreate == false) {
        CLOGE("ERR(%s):Not yet created fail", __func__);
        return false;
    }

    if (m_camera_info[cameraMode].is3a0Src.flagStart == false) {
        CLOGE("ERR(%s):Not yet is3a0Src started fail", __func__);
        return false;
    }

    if (m_camera_info[cameraMode].is3a0Dst.flagStart == false) {
        CLOGE("ERR(%s):Not yet is3a0Dst started fail", __func__);
        return false;
    }

    shot_ext_src = (camera2_shot_ext *)inBuf->virt.extP[1];
    fcount_buf = shot_ext_src->shot.dm.request.frameCount;

    if (cameraMode == CAMERA_MODE_REPROCESSING) {
        int pictureW = 0, pictureH = 0;
        int cropX = 0, cropY = 0, cropW = 0, cropH = 0;
        struct v4l2_crop crop;
        struct v4l2_rect rect;
/* TODO: remove if unused
        camera2_shot_ext *shot_ext_reprocessing;
        shot_ext_reprocessing = (struct camera2_shot_ext *)(m_camera_info[CAMERA_MODE_REPROCESSING].sensor.buffer[position_buf].virt.extP[1]);

        memcpy(shot_ext_reprocessing, shot_ext_src, sizeof(struct camera2_shot_ext));
*/

        ExynosBuffer *inBufTemp = inBuf;
        inBuf = &(m_camera_info[CAMERA_MODE_REPROCESSING].sensor.buffer[position_buf]);
        memcpy(inBuf->virt.extP[1], inBufTemp->virt.extP[1], sizeof(struct camera2_shot_ext));
        shot_ext = (struct camera2_shot_ext *)m_camera_info[cameraMode].sensor.buffer[position_buf].virt.extP[1];
        if (shot_ext != NULL) {
            shot_ext->setfile = m_camera_info[cameraMode].dummy_shot.setfile;
            shot_ext->shot.dm.request.frameCount = fcount_buf;

            if (m_setZoom(m_curCameraInfo[m_cameraMode]->zoom,
                          m_curCameraInfo[CAMERA_MODE_REPROCESSING]->pictureW,
                          m_curCameraInfo[CAMERA_MODE_REPROCESSING]->pictureH,
                          m_curCameraInfo[CAMERA_MODE_REPROCESSING]->ispW,
                          m_curCameraInfo[CAMERA_MODE_REPROCESSING]->ispH,
                          (void *)shot_ext) == false) {
                CLOGE("ERR(%s):m_setZoom() fail", __func__);
                return false;
            }
        } else {
            CLOGE("ERR(%s):sesnor buffer[%d] is NULL", __func__, position_buf);
            return false;
        }

    }

    if (shot_ext != NULL)
        request_3ax = shot_ext->request_3ax;

    CLOGT(m_traceCount, "(%s): q in", __func__);

    if (cam_int_qbuf(&(m_camera_info[cameraMode].is3a0Dst), 0,
                     &(m_camera_info[cameraMode].isp)) < 0) {
        CLOGE("ERR(%s):is3a0Dst cam_int_qbuf(%d) fail", __func__, position_buf);
        m_cameraModeIs3a0 = CAMERA_MODE_MAX;

        return false;
    }

    if (cam_int_qbuf(&(m_camera_info[cameraMode].is3a0Src), position_buf,
                     &(m_camera_info[cameraMode].sensor)) < 0) {
        CLOGE("ERR(%s):is3a0Src cam_int_qbuf(%d) fail", __func__, position_buf);
        m_cameraModeIs3a0 = CAMERA_MODE_MAX;

        return false;
    }

    CLOGT(m_traceCount, "(%s): q out(index %d)", __func__, srcIndex);

    CLOGT(m_traceCount, "(%s): dq in", __func__);

    dstIndex = cam_int_dqbuf(&(m_camera_info[cameraMode].is3a0Dst));
    if (dstIndex < 0) {
        CLOGE("ERR(%s):cam_int_dqbuf() on is3a0 fail", __func__);
        m_cameraModeIs3a0 = CAMERA_MODE_MAX;

        return false;
    }

    srcIndex = cam_int_dqbuf(&(m_camera_info[cameraMode].is3a0Src));
    if (srcIndex < 0) {
        CLOGE("ERR(%s):cam_int_dqbuf() on is3a0 fail", __func__);
        m_cameraModeIs3a0 = CAMERA_MODE_MAX;

        return false;
    }

    CLOGT(m_traceCount, "(%s): dq out(index %d)", __func__, srcIndex);

    /* back-up is3aa_dm : result of isp metadata */
    memcpy(&m_camera_info[cameraMode].is3aa_dm, shot_ext_src, sizeof(struct camera2_shot_ext));

    /* copy 3a0 to isp */
    memcpy(m_camera_info[cameraMode].isp.buffer[0].virt.extP[1],
                m_camera_info[cameraMode].sensor.buffer[position_buf].virt.extP[1], META_DATA_SIZE);

    /* use same buffer number */
    *outBuf = m_camera_info[cameraMode].isp.buffer[0];

    if (shot_ext != NULL) {
        if (request_3ax == shot_ext->request_3ax) {
            outBuf->reserved.p = 0;
        } else {
            outBuf->reserved.p = -1;
            m_traceCount = TRACE_COUNT;
            CLOGE("ERR(%s):3a0 Shot done is invalid(expected request_3ax is %d, but we got %d), skip frame(%d)",
                    __func__, request_3ax, shot_ext->request_3ax, shot_ext->shot.dm.request.frameCount);
        }
    } else {
        CLOGE("ERR(%s):shot_ext is NULL", __func__);
    }

    m_cameraModeIs3a0 = cameraMode;

    shot_ext_src = (camera2_shot_ext *)inBuf->virt.extP[1];
    if (shot_ext_src->shot.dm.request.frameCount != fcount_buf) {
        shot_ext_src = (camera2_shot_ext *)inBuf->virt.extP[1];
        shot_ext_dst = (camera2_shot_ext *)outBuf->virt.extP[1];

        CLOGD("DEBUG(%s):(%d) [src %d] [dst %d] [fcount %d] [pos %d] [in index %d] [out index %d]",
            __func__, __LINE__,
            shot_ext_src->shot.dm.request.frameCount,
            shot_ext_dst->shot.dm.request.frameCount,
            fcount_buf,
            position_buf,
            inBuf->reserved.p,
            outBuf->reserved.p);

        printBayerLockStatus();
    }

    return true;
}

bool ExynosCamera::putIs3a0Buf(ExynosBuffer *buf)
{
    if (m_flagCreate == false) {
        CLOGE("ERR(%s):Not yet created fail", __func__);
        return false;
    }

    return true;
}

bool ExynosCamera::getIs3a1Buf(enum CAMERA_MODE cameraMode, ExynosBuffer *inBuf, ExynosBuffer *outBuf)
{
    int srcIndex = 0;
    int dstIndex = 0;
    int position_buf = inBuf->reserved.p;
    struct camera2_shot_ext *shot_ext;
    struct camera2_shot_ext *shot_ext_src;
    bool ret = true;

    if (m_flagCreate == false) {
        CLOGE("ERR(%s):Not yet created fail", __func__);
        return false;
    }

    if (m_camera_info[cameraMode].is3a1Src.flagStart == false) {
        CLOGE("ERR(%s):Not yet is3a1Src started fail", __func__);
        return false;
    }

    if (m_camera_info[cameraMode].is3a1Dst.flagStart == false) {
        CLOGE("ERR(%s):Not yet is3a1Dst started fail", __func__);
        return false;
    }

    if (position_buf < 0 &&
        !(m_cameraMode == CAMERA_MODE_BACK)) {
        CLOGE("ERR(%s):Invalid inBuf index(%d)", __func__, position_buf);
        return false;
    }

    if (m_notifyStopMsg == true && m_numOfShotedFrame == 0) {
        CLOGV("DEBUG(%s): m_numOfShotedFrame is 0", __func__);
        outBuf->reserved.p = -1;
        return true;
    }

    if (m_cameraMode == CAMERA_MODE_BACK) {
        if (0 <= m_is3a1DstLastBufIndex) {
            CLOGT(m_traceCount, "(%s): qbuf in(out %d, cap %d", __func__, m_is3a1SrcLastBufIndex, m_is3a1DstLastBufIndex);

            //metadata buffer
            memset(m_metaBuf[m_is3a1DstLastBufIndex].virt.extP[1], 0, m_metaBuf[m_is3a1DstLastBufIndex].size.extS[1]);
            if (cam_int_qbuf(&(m_camera_info[cameraMode].is3a1Dst), m_is3a1DstLastBufIndex,
                             &(m_camera_info[cameraMode].isp),
                             m_metaBuf[m_is3a1DstLastBufIndex]) < 0) {
                CLOGE("ERR(%s):is3a1Dst cam_int_qbuf(%d) fail", __func__, m_is3a1DstLastBufIndex);
                m_is3a1DstLastBufIndex = -1;
                ret = false;
            }
        }

        if (0 <= m_is3a1SrcLastBufIndex) {
            shot_ext = (struct camera2_shot_ext *)m_camera_info[cameraMode].sensor.buffer[m_is3a1SrcLastBufIndex].virt.extP[1];
            memcpy(&shot_ext->shot.ctl, &m_camera_info[cameraMode].dummy_shot.shot.ctl, sizeof(struct camera2_ctl));
#ifdef FD_ROTATION
//            shot_ext->shot.uctl.scalerUd.orientation = m_camera_info[cameraMode].dummy_shot.shot.uctl.scalerUd.orientation;
#endif

            shot_ext->setfile = m_camera_info[cameraMode].dummy_shot.setfile;
            shot_ext->request_3ax = m_camera_info[cameraMode].dummy_shot.request_3ax;
            shot_ext->request_isp = m_camera_info[cameraMode].dummy_shot.request_isp;
            shot_ext->request_scc = m_camera_info[cameraMode].dummy_shot.request_scc;
            shot_ext->request_scp = m_camera_info[cameraMode].dummy_shot.request_scp;
            shot_ext->request_dis = m_camera_info[cameraMode].dummy_shot.request_dis;

            shot_ext->dis_bypass = m_camera_info[cameraMode].dummy_shot.dis_bypass;
            shot_ext->dnr_bypass = m_camera_info[cameraMode].dummy_shot.dnr_bypass;
            shot_ext->fd_bypass = m_camera_info[cameraMode].dummy_shot.fd_bypass;
            shot_ext->shot.magicNumber = m_camera_info[cameraMode].dummy_shot.shot.magicNumber;

            m_turnOffEffectByFps(shot_ext, m_curCameraInfo[cameraMode]->fpsRange[1]);

            m_autofocusMgr->execFunction(ExynosCameraActivityBase::CALLBACK_TYPE_3A_BEFORE,
                                         (void *)&(m_camera_info[cameraMode].sensor.buffer[m_is3a1SrcLastBufIndex]));

            m_flashMgr->execFunction(ExynosCameraActivityBase::CALLBACK_TYPE_3A_BEFORE,
                                         (void *)&(m_camera_info[cameraMode].sensor.buffer[m_is3a1SrcLastBufIndex]));

            m_sCaptureMgr->execFunction(ExynosCameraActivityBase::CALLBACK_TYPE_3A_BEFORE,
                (void *)&(m_camera_info[cameraMode].sensor.buffer[m_is3a1SrcLastBufIndex]));

            memcpy(&m_camera_info[cameraMode].dummy_shot.shot.ctl, &shot_ext->shot.ctl, sizeof(struct camera2_ctl));

            if (cam_int_qbuf(&(m_camera_info[cameraMode].is3a1Src), m_is3a1SrcLastBufIndex,
                             &(m_camera_info[cameraMode].sensor)) < 0) {
                CLOGE("ERR(%s):is3a1Src cam_int_qbuf(%d) fail", __func__, m_is3a1SrcLastBufIndex);
                m_is3a1SrcLastBufIndex = -1;
                ret = false;
            }
            CLOGT(m_traceCount, "(%s): qbuf out", __func__);
        }

        if (ret == false && m_numOfShotedFrame == 0) {
            CLOGE("ERR(%s):qbuf fail and m_numOfShotedFrame = %d, Cannot try dqbuf", __func__, m_numOfShotedFrame);
            return ret;
        }
    } else {
        /* use same buffer number */
        *outBuf = m_camera_info[cameraMode].isp.buffer[position_buf];
        outBuf->reserved.p = position_buf;

        shot_ext = (struct camera2_shot_ext *)(inBuf->virt.extP[1]);

        memcpy(&shot_ext->shot.ctl, &m_camera_info[cameraMode].dummy_shot.shot.ctl, sizeof(struct camera2_ctl));
#ifdef FD_ROTATION
//        shot_ext->shot.uctl.scalerUd.orientation = m_camera_info[cameraMode].dummy_shot.shot.uctl.scalerUd.orientation;
#endif
        m_turnOffEffectByFps(shot_ext, m_curCameraInfo[cameraMode]->fpsRange[1]);

        m_autofocusMgr->execFunction(ExynosCameraActivityBase::CALLBACK_TYPE_3A_BEFORE,
            (void *)&(m_camera_info[cameraMode].sensor.buffer[position_buf]));

        m_flashMgr->execFunction(ExynosCameraActivityBase::CALLBACK_TYPE_3A_BEFORE,
            (void *)&(m_camera_info[cameraMode].sensor.buffer[position_buf]));

        m_sCaptureMgr->execFunction(ExynosCameraActivityBase::CALLBACK_TYPE_3A_BEFORE,
            (void *)&(m_camera_info[cameraMode].sensor.buffer[position_buf]));

        memcpy(&m_camera_info[cameraMode].dummy_shot.shot.ctl, &shot_ext->shot.ctl, sizeof(struct camera2_ctl));

        /* metadata buffer */
        memset(m_metaBuf[position_buf].virt.extP[1], 0, m_metaBuf[position_buf].size.extS[1]);
        if (cam_int_qbuf(&(m_camera_info[cameraMode].is3a1Dst), position_buf,
                         &(m_camera_info[cameraMode].isp),
                         m_metaBuf[position_buf]) < 0) {
            CLOGE("ERR(%s):is3a1Dst cam_int_qbuf(%d) fail", __func__, position_buf);
            return false;
        }

        if (cam_int_qbuf(&(m_camera_info[cameraMode].is3a1Src), position_buf,
                         &(m_camera_info[cameraMode].sensor)) < 0) {
            CLOGE("ERR(%s):is3a1Src cam_int_qbuf(%d) fail", __func__, position_buf);
            return false;
        }
    }

    CLOGT(m_traceCount, "(%s): dqbuf in", __func__);

    dstIndex = cam_int_dqbuf(&(m_camera_info[cameraMode].is3a1Dst));
    srcIndex = cam_int_dqbuf(&(m_camera_info[cameraMode].is3a1Src));

    CLOGT(m_traceCount, "(%s): dqbuf out(out %d, cap %d)", __func__, srcIndex, dstIndex);

    if (dstIndex < 0 || srcIndex < 0) {
        CLOGE("ERR(%s):cam_int_dqbuf() on is3a1 fail. index(capture:%d, output:%d)", __func__, dstIndex, srcIndex);
        ret = false;
    }

    if (m_notifyStopMsg == true) {
        m_is3a1SrcLastBufIndex = -1;
        m_is3a1DstLastBufIndex = -1;
    } else {
        m_is3a1SrcLastBufIndex = srcIndex;
        m_is3a1DstLastBufIndex = dstIndex;
    }

    if (ret == false)
        return ret;

    if (m_cameraMode == CAMERA_MODE_BACK) {
        /* TODO: Flash and AF */
        m_autofocusMgr->execFunction(ExynosCameraActivityBase::CALLBACK_TYPE_3A_AFTER,
            (void *)&(m_camera_info[cameraMode].sensor.buffer[srcIndex]));

        m_flashMgr->execFunction(ExynosCameraActivityBase::CALLBACK_TYPE_3A_AFTER,
            (void *)&(m_camera_info[cameraMode].sensor.buffer[srcIndex]));

        m_sCaptureMgr->execFunction(ExynosCameraActivityBase::CALLBACK_TYPE_3A_AFTER,
            (void *)&(m_camera_info[cameraMode].sensor.buffer[srcIndex]));

        shot_ext = (struct camera2_shot_ext *)m_camera_info[cameraMode].sensor.buffer[dstIndex].virt.extP[1];

        /* back-up is3aa_dm : result of isp metadata */
        memcpy(&m_camera_info[cameraMode].is3aa_dm, shot_ext, sizeof(struct camera2_shot_ext));

        /* copy 3a1 to isp */
        memcpy(m_camera_info[cameraMode].isp.buffer[dstIndex].virt.extP[1],
                    m_camera_info[cameraMode].sensor.buffer[srcIndex].virt.extP[1], META_DATA_SIZE);

        m_is3a1FrameCount = shot_ext->shot.dm.request.frameCount;

        CLOGT(m_traceCount, "(%s:%d): dm.request count %d", __func__, __LINE__, m_is3a1FrameCount);

        shot_ext_src = (struct camera2_shot_ext *)m_camera_info[cameraMode].sensor.buffer[srcIndex].virt.extP[1];

        /* use same buffer number */
        *outBuf = m_camera_info[cameraMode].isp.buffer[dstIndex];

        if (shot_ext_src->request_3ax == m_camera_info[cameraMode].dummy_shot.request_3ax) {
            outBuf->reserved.p = dstIndex;
        } else {
            outBuf->reserved.p = -1;
            m_traceCount = TRACE_COUNT;
            CLOGE("ERR(%s):3a1 Shot done is invalid(expected request_3ax is %d, but we got %d), skip frame(%d)",
                    __func__, m_camera_info[cameraMode].dummy_shot.request_3ax, shot_ext_src->request_3ax, shot_ext->shot.dm.request.frameCount);
        }

        CLOGT(m_traceCount, "(%s): index(output:%d, capture:%d), (free:%d) (request:%d) (process:%d) (complete:%d)",
                __func__, srcIndex, dstIndex,
                shot_ext_src->free_cnt,
                shot_ext_src->request_cnt,
                shot_ext_src->process_cnt,
                shot_ext_src->complete_cnt);

        m_numOfShotedFrame = shot_ext_src->request_cnt + shot_ext_src->process_cnt + shot_ext_src->complete_cnt;
    } else {
        m_autofocusMgr->execFunction(ExynosCameraActivityBase::CALLBACK_TYPE_3A_AFTER,
            (void *)&(m_camera_info[cameraMode].sensor.buffer[position_buf]));

        m_flashMgr->execFunction(ExynosCameraActivityBase::CALLBACK_TYPE_3A_AFTER,
            (void *)&(m_camera_info[cameraMode].sensor.buffer[position_buf]));

        m_sCaptureMgr->execFunction(ExynosCameraActivityBase::CALLBACK_TYPE_3A_AFTER,
            (void *)&(m_camera_info[cameraMode].sensor.buffer[position_buf]));

        /* back-up is3aa_dm : result of isp metadata */
        memcpy(&m_camera_info[cameraMode].is3aa_dm, shot_ext, sizeof(struct camera2_shot_ext));

        /* copy 3a1 to isp */
        memcpy(outBuf->virt.extP[1], inBuf->virt.extP[1], META_DATA_SIZE);
    }

    return true;
}

int ExynosCamera::getNumOfShotedFrame(void)
{
    return m_numOfShotedFrame;
}

int ExynosCamera::getNumOfShotedIspFrame(void)
{
    return m_numOfShotedIspFrame;
}

void ExynosCamera::notifyStop(bool msg)
{
    if (m_cameraMode == CAMERA_MODE_BACK)
        m_notifyStopMsg = msg;
    else
        CLOGV("DEBUG(%s): Front camera dose not need notify stop msg", __func__);

    m_traceCount = msg ? TRACE_COUNT : 0;
}

bool ExynosCamera::getNotifyStopMsg(void)
{
    return m_notifyStopMsg;
}

bool ExynosCamera::putIs3a1Buf(ExynosBuffer *buf)
{
    if (m_flagCreate == false) {
        CLOGE("ERR(%s):Not yet created fail", __func__);
        return false;
    }

    return true;
}

#ifdef USE_VDIS
void ExynosCamera::setVDisSrcW(unsigned int value)
{
    m_VDisSrcW = value;
}

void ExynosCamera::setVDisSrcH(unsigned int value)
{
    m_VDisSrcH= value;
}

void ExynosCamera::setVDisDstW(unsigned int value)
{
    m_VDisDstW = value;
}

void ExynosCamera::setVDisDstH(unsigned int value)
{
    m_VDisDstH = value;
}

void ExynosCamera::setVDisSrcBufNum(unsigned int value)
{
    m_VDisSrcBufNum = value;
}

void ExynosCamera::setVDisDstBufNum(unsigned int value)
{
    m_VDisDstBufNum = value;
}

bool ExynosCamera::getVDisSrcBuf(ExynosBuffer *buf, int *rcount, int *fcount)
{
    int index = 0;
    struct camera2_stream *metadata;

    CLOGV("[%s] (%d)", __func__, __LINE__);
    index = cam_int_dqbuf(&(m_camera_info[m_cameraMode].vdisc));
    if (index < 0) {
        CLOGE("ERR(%s):cam_int_dqbuf(1) on preview fail", __func__);
        return false;
    }

    *buf = m_camera_info[m_cameraMode].vdisc.buffer[index];
    buf->reserved.p = index;

    metadata = (struct camera2_stream *)m_camera_info[m_cameraMode].vdisc.buffer[index].virt.extP[1];
    *rcount = metadata->rcount;
    *fcount = metadata->fcount;

    return true;
}

bool ExynosCamera::putVDisSrcBuf(ExynosBuffer *buf)
{
    if (m_isHWVDis) {
        m_camera_info[m_cameraMode].vdisc.buffer[buf->reserved.p].fd.extFd[0] = buf->fd.extFd[0];
        m_camera_info[m_cameraMode].vdisc.buffer[buf->reserved.p].virt.extP[0] = buf->virt.extP[0];
    }

    CLOGV("[%s] (%d)", __func__, __LINE__);
    if (cam_int_qbuf(&(m_camera_info[m_cameraMode].vdisc), buf->reserved.p) < 0) {
        CLOGE("%s: cam_int_qbuf(%d) fail", __func__, buf->reserved.p);
        return false;
    }

    return true;
}

bool ExynosCamera::getVDisDstBuf(ExynosBuffer *buf)
{
    int index = 0;

    CLOGV("[%s] (%d)", __func__, __LINE__);
    index = cam_int_dqbuf(&(m_camera_info[m_cameraMode].vdiso));
    if (index < 0) {
        CLOGE("ERR(%s):cam_int_dqbuf(1) on preview fail", __func__);
        return false;
    }

    *buf = m_camera_info[m_cameraMode].vdiso.buffer[index];
    buf->reserved.p = index;

    return true;
}
bool ExynosCamera::getVDisDstBufAddr(ExynosBuffer **buf, int i)
{
    *buf = &m_camera_info[m_cameraMode].vdiso.buffer[i];

    if (&m_camera_info[m_cameraMode].vdiso.buffer[i] != NULL);
        CLOGD("%s, index(%d)", __func__, index);

    return true;
}

bool ExynosCamera::putVDisDstBuf(ExynosBuffer *buf, int rcount, int fcount)
{
    struct camera2_shot_ext *shot_ext;
    int index = 0;

    index = buf->reserved.p;
    shot_ext = (struct camera2_shot_ext *)m_camera_info[m_cameraMode].vdiso.buffer[index].virt.extP[1];

    shot_ext->request_3ax = m_camera_info[m_cameraMode].dummy_shot.request_3ax;
    shot_ext->request_isp = m_camera_info[m_cameraMode].dummy_shot.request_isp;
    shot_ext->request_scc = m_camera_info[m_cameraMode].dummy_shot.request_scc;
    shot_ext->request_scp = m_camera_info[m_cameraMode].dummy_shot.request_scp;

    if (m_isHWVDis) {
        m_camera_info[m_cameraMode].vdiso.buffer[index].fd.extFd[0] = buf->fd.extFd[0];
        m_camera_info[m_cameraMode].vdiso.buffer[index].virt.extP[0] = buf->virt.extP[0];
    }

    shot_ext->dis_bypass = m_camera_info[m_cameraMode].dummy_shot.dis_bypass;
    shot_ext->dnr_bypass = m_camera_info[m_cameraMode].dummy_shot.dnr_bypass;
    shot_ext->fd_bypass = m_camera_info[m_cameraMode].dummy_shot.fd_bypass;

    shot_ext->shot.ctl.request.frameCount = rcount;
    shot_ext->shot.dm.request.frameCount = fcount;
    shot_ext->shot.magicNumber = 0x23456789;

    CLOGV("[%s] (%d)", __func__, __LINE__);
    if (cam_int_qbuf(&(m_camera_info[m_cameraMode].vdiso), index) < 0) {
        CLOGE("%s: cam_int_qbuf(%d) fail", __func__, index);
        return false;
    }

    return true;
}
#endif

bool ExynosCamera::startSensor(void)
{
    CLOGD("DEBUG(%s):in", __func__);

    if (m_flagCreate == false) {
        CLOGE("ERR(%s):Not yet Created", __func__);
        return false;
    }

    CLOGD("(flagStart %d), (%d x %d), format 0x%x", m_camera_info[m_cameraMode].sensor.flagStart,
        m_camera_info[m_cameraMode].sensor.width,
        m_camera_info[m_cameraMode].sensor.height,
        m_camera_info[m_cameraMode].sensor.format);

    m_is3a1SrcLastBufIndex = -1;
    m_is3a1DstLastBufIndex = -1;
    m_recentCaptureBayerBufIndex = 0;

    if (m_camera_info[m_cameraMode].sensor.flagStart == false) {

        Mutex::Autolock lock(m_sensorLock);

        if (m_cameraMode == CAMERA_MODE_BACK) {
            m_camera_info[m_cameraMode].sensor.width   = SIZE_OTF_WIDTH;
            m_camera_info[m_cameraMode].sensor.height  = SIZE_OTF_HEIGHT;
        } else {
            m_camera_info[m_cameraMode].sensor.width   = m_curCameraInfo[m_cameraMode]->pictureW + 16;
            m_camera_info[m_cameraMode].sensor.height  = m_curCameraInfo[m_cameraMode]->pictureH + 10;
        }

        m_camera_info[m_cameraMode].sensor.format  = V4L2_PIX_FMT_SBGGR12;
        m_camera_info[m_cameraMode].sensor.planes  = 2;
        m_camera_info[m_cameraMode].sensor.buffers = NUM_BAYER_BUFFERS;
        m_camera_info[m_cameraMode].sensor.memory  = V4L2_CAMERA_MEMORY_TYPE;
        m_camera_info[m_cameraMode].sensor.type    = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
        m_camera_info[m_cameraMode].sensor.ionClient = m_ionCameraClient;

        ExynosBuffer nullBuf;
        int planeSize;
        planeSize = getPlaneSizePackedFLiteOutput(m_camera_info[m_cameraMode].sensor.width , m_camera_info[m_cameraMode].sensor.height);

        for (int i = 0; i < m_camera_info[m_cameraMode].sensor.buffers; i++) {
            m_camera_info[m_cameraMode].sensor.buffer[i] = nullBuf;
            m_camera_info[m_cameraMode].sensor.buffer[i].size.extS[0] = planeSize;
            m_camera_info[m_cameraMode].sensor.buffer[i].size.extS[1] = META_DATA_SIZE;

            if (allocMem(m_camera_info[m_cameraMode].sensor.ionClient, &m_camera_info[m_cameraMode].sensor.buffer[i], 1 << 1) == false) {
                CLOGE("ERR(%s):allocMem() fail", __func__);
                goto err;
            } else {
                memset(m_camera_info[m_cameraMode].sensor.buffer[i].virt.extP[1],
                        0, m_camera_info[m_cameraMode].sensor.buffer[i].size.extS[1]);
            }
        }

        m_camera_info[m_cameraMode].sensor.flagStart = true;
    }

    return true;

err:
    m_camera_info[m_cameraMode].sensor.flagStart = true;
    if (stopSensor() == false)
        CLOGE("ERR(%s):stopSensor() fail", __func__);

    return false;
}

bool ExynosCamera::startSensorOn(enum CAMERA_MODE cameraMode)
{
    CLOGD("DEBUG(%s):in", __func__);

    if (m_flagCreate == false) {
        CLOGE("ERR(%s):Not yet Created", __func__);
        return false;
    }

    Mutex::Autolock lock(m_sensorLock);

    // HACK : ISP need to skip s_input on second Preview (fw?)
    if (m_isFirtstSensorStart == true) {
        int sensorId = m_getSensorId(m_cameraMode);
        sensorId = (0 << REPROCESSING_SHFIT) | ((FIMC_IS_VIDEO_SEN0_NUM - FIMC_IS_VIDEO_SEN0_NUM) << VIDEO_INDEX_SHFIT) | (sensorId << 0);

        if (cam_int_s_input(&(m_camera_info[cameraMode].sensor), sensorId) < 0) {
            CLOGE("ERR(%s):cam_int_s_input(%d) fail", __func__, sensorId);
            return false;
        }
        m_isFirtstSensorStart = false;
    }

    if (cam_int_s_fmt(&(m_camera_info[cameraMode].sensor)) < 0) {
        CLOGE("ERR(%s):sensor s_fmt fail",  __func__);
        return false;
    }

    int fps = m_camera_info[m_cameraMode].dummy_shot.shot.ctl.aa.aeTargetFpsRange[1];
    CLOGV("DEBUG(%s):fps(%d)", __func__, fps);
    if (setFPSParam(fps) < 0) {
        CLOGE("ERR(%s):setFPSParam(%d) fail", __func__, fps);
        return false;
    }

    if (cam_int_reqbufs(&(m_camera_info[cameraMode].sensor)) < 0) {
        CLOGE("ERR(%s):cam_int_reqbufs() fail", __func__);
        return false;
    }

    m_sensorFrameCount = 0;

    m_camera_info[m_cameraMode].dummy_shot.request_3ax = 1;
    m_camera_info[m_cameraMode].dummy_shot.request_isp = 1;
    m_camera_info[m_cameraMode].dummy_shot.request_scp = 1;
    m_camera_info[m_cameraMode].dummy_shot.request_scc = 0;
    m_camera_info[m_cameraMode].dummy_shot.request_dis = 0;

    int numOfInitialSensorBuf = m_camera_info[cameraMode].sensor.buffers;

#ifdef SCALABLE_SENSOR
    if (getScalableSensorStart()) {
        int width, height;

        getScalableSensorSizeOnPreview(&width, &height);
        width  += 16;
        height += 10;

        CLOGD("DEBUG(%s):getScalableSensorStart(%d/%d) sensor ", __func__, width, height);
        m_camera_info[cameraMode].sensor.width   = width;
        m_camera_info[cameraMode].sensor.height  = height;

        struct v4l2_crop crop;
        memset(&crop, 0x00, sizeof(struct v4l2_crop));
        crop.c.top    = 0;
        crop.c.left   = 0;
        crop.c.width  = width;
        crop.c.height = height;
        crop.type    = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;

        if (exynos_v4l2_s_crop(m_camera_info[cameraMode].sensor.fd, &crop) < 0) {
            CLOGE("ERR(%s):sensor s_crop fail",  __func__);
            return false;
        }
    }
#endif

    if (NUM_MIN_SENSOR_QBUF < numOfInitialSensorBuf)
        numOfInitialSensorBuf = NUM_MIN_SENSOR_QBUF;

#ifdef DYNAMIC_BAYER_BACK_REC
    if ((m_recordingHint == true) && (m_cameraMode == CAMERA_MODE_BACK)) {
        numOfInitialSensorBuf = NO_BAYER_SENSOR_Q_NUM;
    }
#endif

#ifdef SCALABLE_SENSOR
    if (getScalableSensorStart() == true) {
        numOfInitialSensorBuf = 0;
    }
#endif

    for (int i = 0; i < numOfInitialSensorBuf; i++) {
        memcpy(m_camera_info[cameraMode].sensor.buffer[i].virt.extP[1], &(m_camera_info[m_cameraMode].dummy_shot), sizeof(camera2_shot_ext));
        m_camera_info[cameraMode].sensor.buffer[i].reserved.extP[FRAME_COUNT_INDEX] = m_sensorFrameCount++;

        camera2_shot_ext *shot_ext = (struct camera2_shot_ext *)(m_camera_info[cameraMode].sensor.buffer[i].virt.extP[1]);

        m_turnOffEffectByFps(shot_ext, m_curCameraInfo[m_cameraMode]->fpsRange[1]);

        if (m_setSetfile(cameraMode) == false) {
            CLOGE("ERR(%s):m_setSetfile(%d) fail", __func__, cameraMode);
            return false;
        }

        shot_ext->setfile = m_camera_info[cameraMode].dummy_shot.setfile;

        if (cam_int_qbuf(&(m_camera_info[cameraMode].sensor), i) < 0) {
            CLOGE("ERR(%s):cam_int_qbuf(%d) fail", __func__, i);
            return false;
        }
    }

    for (int i = NUM_MIN_SENSOR_QBUF; i < m_camera_info[cameraMode].sensor.buffers; i++)
        m_pushSensorQ(i);

    if (cam_int_streamon(&(m_camera_info[cameraMode].sensor)) < 0) {
        CLOGE("ERR(%s):cam_int_streamon(sensor) fail", __func__);
        return false;
    }

    return true;
}

bool ExynosCamera::stopSensorOff(enum CAMERA_MODE cameraMode)
{
    if (m_flagCreate == false) {
        CLOGW("WARN(%s):Not yet Created", __func__);
        return false;
    }

    if (m_camera_info[cameraMode].sensor.flagStart == true) {
#ifdef THREAD_PROFILE
        timeUs = 0;
        gettimeofday(&mTimeStart, NULL);
#endif
        if (cam_int_streamoff(&m_camera_info[cameraMode].sensor) < 0) {
            CLOGE("ERR(%s):exynos_v4l2_streamoff() fail", __func__);
            return false;
        }
#ifdef THREAD_PROFILE
        gettimeofday(&mTimeStop, NULL);
        timeUs = (mTimeStop.tv_sec*1000000 + mTimeStop.tv_usec) - (mTimeStart.tv_sec*1000000 + mTimeStart.tv_usec);
        CLOGD("DEBUG(%s):time_check elapsed time=(%ld)us", __func__, timeUs);
#endif

        m_camera_info[cameraMode].dummy_shot.request_3ax = 0;
        m_camera_info[cameraMode].dummy_shot.request_isp = 0;

        m_camera_info[cameraMode].sensor.buffers = 0;
        if (cam_int_reqbufs(&m_camera_info[cameraMode].sensor) < 0) {
            CLOGE("ERR(%s):cam_int_reqbufs() fail", __func__);
            return false;
        }

        m_releaseSensorQ();
        m_releaseBayerQ();
        m_minCaptureBayerBuf = MIN_CAPTURE_BAYER_COUNT;
    }

    return true;
}

bool ExynosCamera::stopSensor(void)
{
    if (m_flagCreate == false) {
        CLOGW("WARN(%s):Not yet Created", __func__);
        return false;
    }

    if (m_camera_info[m_cameraMode].sensor.flagStart == true) {
        Mutex::Autolock lock(m_sensorLock);

        m_isFirtstSensorStart = true;

        for (int i = 0; i < NUM_BAYER_BUFFERS; i++) {
            freeMem(&m_camera_info[m_cameraMode].sensor.buffer[i]);
            CLOGV("ERR(%s):freeMem sensor (%d)", __func__, m_camera_info[m_cameraMode].sensor.buffers);
         }

        for (int i = 0; i < VIDEO_MAX_FRAME; i++)
            freeMemSinglePlane(&m_camera_info[m_cameraMode].sensor.buffer[i], m_camera_info[m_cameraMode].sensor.planes - 1);

        m_camera_info[m_cameraMode].sensor.flagStart = false;
    }

    return true;
}

bool ExynosCamera::flagStartSensor(void)
{
    return m_camera_info[m_cameraMode].sensor.flagStart;
}

bool ExynosCamera::flagEmptySensorBuf(ExynosBuffer *buf)
{
    camera2_shot_ext *shot_ext = (struct camera2_shot_ext *)(buf->virt.extP[1]);
    bool ret = true;

    if (0 < shot_ext->complete_cnt) {
        CLOGD("DEBUG(%s):(%d), %d, %d, %d", __func__, __LINE__, shot_ext->request_cnt, shot_ext->process_cnt, shot_ext->complete_cnt);
        return false;
    }

    return ret;
}

#if CAPTURE_BUF_GET
int ExynosCamera::getCapturerBufIndex()
{
    return m_recentCaptureBayerBufIndex;
}

int ExynosCamera::getCapturerBuf(ExynosBuffer *buf)
{
    *buf = m_camera_info[m_cameraMode].sensor.buffer[m_recentCaptureBayerBufIndex];

    return 1;
}
#endif

bool ExynosCamera::getSensorBuf(ExynosBuffer *buf)
{
    if (m_flagCreate == false) {
        CLOGE("ERR(%s):Not yet Created", __func__);
        return false;
    }

    Mutex::Autolock lock(m_sensorLock);

    if (m_camera_info[m_cameraMode].sensor.flagStart == false) {
        CLOGE("ERR(%s):Not yet sensor started fail", __func__);
        return false;
    }

    int index_sensor = 0;
    int waitBayerFcount;
    int capture_ret = 0;
    int dqTryCount = 0;
    ExynosBuffer firstBuf;

#ifdef BAYER_TRACKING
    camera2_shot_ext *shot_ext = NULL;
    int *bayerImage = NULL;
#endif

    int maxDqTryCount = 1;

    int unQueuedBufCount = 0;
    ExynosBuffer unQueuedBuf[NUM_BAYER_BUFFERS];

    if (m_cameraMode == CAMERA_MODE_FRONT)
        maxDqTryCount = NUM_BAYER_BUFFERS;

    for (int dqTryCount = 0; dqTryCount < maxDqTryCount; dqTryCount++) {
        /* sensor dq */
        index_sensor = cam_int_dqbuf(&(m_camera_info[m_cameraMode].sensor));
        if (index_sensor < 0) {
            CLOGE("ERR(%s):cam_int_dqbuf() on sensor fail", __func__);
            return false;
        }

        if (m_cameraMode == CAMERA_MODE_BACK) {
            *buf = m_camera_info[CAMERA_MODE_REPROCESSING].sensor.buffer[index_sensor];

            m_camera_info[m_cameraMode].sensor.buffer[index_sensor].reserved.p = index_sensor;
            m_camera_info[CAMERA_MODE_REPROCESSING].sensor.buffer[index_sensor].reserved.p = index_sensor;
        } else {
            *buf = m_camera_info[m_cameraMode].sensor.buffer[index_sensor];
        }

        buf->reserved.p = index_sensor;

        m_captureBayerIndex[index_sensor] = 2;

        m_autofocusMgr->execFunction(ExynosCameraActivityBase::CALLBACK_TYPE_SENSOR_AFTER,
            (void *)&(m_camera_info[m_cameraMode].sensor.buffer[index_sensor]));

        m_flashMgr->execFunction(ExynosCameraActivityBase::CALLBACK_TYPE_SENSOR_AFTER,
            (void *)&(m_camera_info[m_cameraMode].sensor.buffer[index_sensor]));

        capture_ret = m_sCaptureMgr->execFunction(ExynosCameraActivityBase::CALLBACK_TYPE_SENSOR_AFTER,
            (void *)buf);

#ifdef BAYER_TRACKING
        shot_ext = (camera2_shot_ext *)(buf->virt.extP[1]);
        bayerImage = (int *)(buf->virt.extP[0]);

        if ( shot_ext != NULL && bayerImage != NULL) {
            bayerImage[0] = shot_ext->shot.dm.request.frameCount;
            CLOGD("%d %d", bayerImage[0], shot_ext->shot.dm.request.frameCount);
        }
#endif

#if CAPTURE_BUF_GET
        if (m_cameraMode == CAMERA_MODE_BACK) {

            if (0 < m_minCaptureBayerBuf) {
                m_pushSensorQ(index_sensor);
                m_minCaptureBayerBuf--;
                m_notInsertBayer = true;
            } else {
                if ((capture_ret == 2) || (capture_ret == 3) || (capture_ret == 4)) {
                    /* setBayerLockIndex(index_sensor, true); */
                    setBayerLockIndex(index_sensor, true);
                }
                /* HAL Bayer Buffer */
                m_pushSensorQ(index_sensor);
                m_notInsertBayer = false;
            }

            m_recentCaptureBayerBufIndex = index_sensor;
        } else {
            m_pushSensorQ(index_sensor);
        }
#else
        m_pushSensorQ(index_sensor);
#endif

        /* if sensorBuf not empty, just use the first one */
        if (this->flagEmptySensorBuf(buf) == true) {
            if (0 < dqTryCount) {
                unQueuedBuf[unQueuedBufCount] = *buf;
                unQueuedBufCount++;

                *buf = firstBuf;
            }
                break;
        } else {
            CLOGD("DEBUG(%s):(%d), flagEmptySensorBuf() is false, so dq again on sensor(count : %d)",
                __func__, __LINE__, dqTryCount);

            if (dqTryCount == 0)
                firstBuf = *buf;
            else {
                unQueuedBuf[unQueuedBufCount] = *buf;
                unQueuedBufCount++;
            }
        }
    }

    /* q to sensor internally */
    m_sensorLock.unlock(); // unlock for Autolock

    for (int i = 0; i < unQueuedBufCount; i++) {
        if (this->putSensorBuf(&unQueuedBuf[i]) == false)
            CLOGE("ERR(%s):putSensorBuf() fail", __func__);
    }

    m_sensorLock.lock(); // lock for Autolock

    return true;
}

bool ExynosCamera::putSensorBuf(ExynosBuffer *buf)
{
    int cameraMode;

    if (m_cameraMode == CAMERA_MODE_BACK)
        cameraMode = CAMERA_MODE_REPROCESSING;
    else
        cameraMode = m_cameraMode;

    if (m_flagCreate == false) {
        CLOGE("ERR(%s):Not yet Created", __func__);
        return false;
    }

    Mutex::Autolock lock(m_sensorLock);

    if (m_camera_info[m_cameraMode].sensor.flagStart == false) {
        CLOGE("ERR(%s):Not yet sensor started fail", __func__);
        return false;
    }

    if (m_notInsertBayer == false) {
        camera2_shot_ext *shot_ext;

        /* sensor q */
        int index_sensor = 0;
        index_sensor = m_popSensorQ();
            CLOGV("[%s] (%d) m_popSensorQ %d", __func__, __LINE__, index_sensor);

        if (index_sensor < 0) {
            CLOGW("WARN(%s):m_popSensorQ() fail", __func__);
            return true;
        }

        /* if bayer is locked in HAL bayer queue, get bayer from HAL capture bayer queue*/
        for (int i = 0; i < NUM_BAYER_BUFFERS; i++) {
            if (index_sensor < 0) {
                CLOGW("WARN(%s):m_popSensorQ() fail", __func__);
                return true;
            }

            if (m_captureBayerLock[index_sensor] == true) {
                m_pushSensorQ(index_sensor);
                index_sensor = m_popSensorQ();
                if (index_sensor < 0) {
                    CLOGW("WARN(%s):m_popSensorQ() fail", __func__);
                    return true;
                }
            } else {
                CLOGV("[%s] (%d) m_popSensorQ %d", __func__, __LINE__, index_sensor);
                break;
            }
        }

        shot_ext = (struct camera2_shot_ext *)(m_camera_info[cameraMode].sensor.buffer[index_sensor].virt.extP[1]);

        shot_ext->setfile = m_camera_info[m_cameraMode].dummy_shot.setfile;
        shot_ext->request_3ax = m_camera_info[m_cameraMode].dummy_shot.request_3ax;
        shot_ext->request_isp = m_camera_info[m_cameraMode].dummy_shot.request_isp;
        shot_ext->request_scc = m_camera_info[m_cameraMode].dummy_shot.request_scc;
        shot_ext->request_scp = m_camera_info[m_cameraMode].dummy_shot.request_scp;
        shot_ext->request_dis = m_camera_info[m_cameraMode].dummy_shot.request_dis;

        shot_ext->dis_bypass = m_camera_info[m_cameraMode].dummy_shot.dis_bypass;
        shot_ext->dnr_bypass = m_camera_info[m_cameraMode].dummy_shot.dnr_bypass;
        shot_ext->fd_bypass = m_camera_info[m_cameraMode].dummy_shot.fd_bypass;
        shot_ext->shot.magicNumber= m_camera_info[m_cameraMode].dummy_shot.shot.magicNumber;

        m_autofocusMgr->execFunction(ExynosCameraActivityBase::CALLBACK_TYPE_SENSOR_BEFORE,
            (void *)&(m_camera_info[m_cameraMode].sensor.buffer[index_sensor]));

        m_flashMgr->execFunction(ExynosCameraActivityBase::CALLBACK_TYPE_SENSOR_BEFORE,
            (void *)&(m_camera_info[m_cameraMode].sensor.buffer[index_sensor]));

        m_sCaptureMgr->execFunction(ExynosCameraActivityBase::CALLBACK_TYPE_SENSOR_BEFORE,
            (void *)&(m_camera_info[m_cameraMode].sensor.buffer[index_sensor]));

        m_captureBayerIndex[index_sensor] = 1;

#ifdef BAYER_TRACKING
        CLOGD("sensor qbuf indx[%d]", index_sensor);
        printBayerLockStatus();
#endif

        if (cam_int_qbuf(&(m_camera_info[cameraMode].sensor), index_sensor) < 0)  {
            CLOGE("ERR(%s):cam_int_qbuf(%d) on sensor fail", __func__, index_sensor);
            return false;
        }

        m_camera_info[m_cameraMode].sensor.buffer[index_sensor].reserved.extP[FRAME_COUNT_INDEX] = m_sensorFrameCount++;
    }

    return true;
}

bool ExynosCamera::getFixedSensorBuf(ExynosBuffer *buf)
{
    if (m_flagCreate == false) {
        CLOGE("ERR(%s):Not yet Created", __func__);
        return false;
    }

    Mutex::Autolock lock(m_sensorLock);

    if (m_camera_info[m_cameraMode].sensor.flagStart == false) {
        CLOGE("ERR(%s):Not yet sensor started fail", __func__);
        return false;
    }

    camera2_shot_ext *shot_ext;
    int index_sensor = 0;
    int waitBayerFcount;
    int capture_ret = 0;

    /* sensor dq */
    index_sensor = cam_int_dqbuf(&(m_camera_info[m_cameraMode].sensor));
    if (index_sensor < 0) {
        CLOGE("ERR(%s):cam_int_dqbuf() on sensor fail", __func__);
        return false;
    }

#ifdef BAYER_TRACKING
    CLOGD("getReservedSensorBuf dqbuf indx[%d]", index_sensor);
    printBayerLockStatus();
#endif

    if (m_cameraMode == CAMERA_MODE_BACK) {
        *buf = m_camera_info[CAMERA_MODE_REPROCESSING].sensor.buffer[index_sensor];
        m_camera_info[m_cameraMode].sensor.buffer[index_sensor].reserved.p = index_sensor;
        m_camera_info[CAMERA_MODE_REPROCESSING].sensor.buffer[index_sensor].reserved.p = index_sensor;
    } else
        *buf = m_camera_info[m_cameraMode].sensor.buffer[index_sensor];

    buf->reserved.p = index_sensor;

    m_captureBayerIndex[index_sensor] = 2;

    m_autofocusMgr->execFunction(ExynosCameraActivityBase::CALLBACK_TYPE_SENSOR_AFTER,
        (void *)&(m_camera_info[m_cameraMode].sensor.buffer[index_sensor]));

    m_flashMgr->execFunction(ExynosCameraActivityBase::CALLBACK_TYPE_SENSOR_AFTER,
        (void *)&(m_camera_info[m_cameraMode].sensor.buffer[index_sensor]));

    capture_ret = m_sCaptureMgr->execFunction(ExynosCameraActivityBase::CALLBACK_TYPE_SENSOR_AFTER,
        (void *)buf);

    return true;
}

bool ExynosCamera::putFixedSensorBuf(ExynosBuffer *buf)
{
    if (m_flagCreate == false) {
        CLOGE("ERR(%s):Not yet Created", __func__);
        return false;
    }

    int cameraMode;
    /* use fixed buffer */
    int index_sensor = 0;

    if (m_cameraMode == CAMERA_MODE_BACK)
        cameraMode = CAMERA_MODE_REPROCESSING;
    else
        cameraMode = m_cameraMode;

    Mutex::Autolock lock(m_sensorLock);

    if (m_camera_info[m_cameraMode].sensor.flagStart == false) {
        CLOGE("ERR(%s):Not yet sensor started fail", __func__);
        return false;
    }

    camera2_shot_ext *shot_ext;

    shot_ext = (struct camera2_shot_ext *)(m_camera_info[cameraMode].sensor.buffer[index_sensor].virt.extP[1]);

    shot_ext->setfile = m_camera_info[m_cameraMode].dummy_shot.setfile;
    shot_ext->request_3ax = m_camera_info[m_cameraMode].dummy_shot.request_3ax;
    shot_ext->request_isp = m_camera_info[m_cameraMode].dummy_shot.request_isp;
    shot_ext->request_scc = m_camera_info[m_cameraMode].dummy_shot.request_scc;
    shot_ext->request_scp = m_camera_info[m_cameraMode].dummy_shot.request_scp;
    shot_ext->request_dis = m_camera_info[m_cameraMode].dummy_shot.request_dis;

    shot_ext->dis_bypass = m_camera_info[m_cameraMode].dummy_shot.dis_bypass;
    shot_ext->dnr_bypass = m_camera_info[m_cameraMode].dummy_shot.dnr_bypass;
    shot_ext->fd_bypass = m_camera_info[m_cameraMode].dummy_shot.fd_bypass;
    shot_ext->shot.magicNumber= m_camera_info[m_cameraMode].dummy_shot.shot.magicNumber;

    m_autofocusMgr->execFunction(ExynosCameraActivityBase::CALLBACK_TYPE_SENSOR_BEFORE,
        (void *)&(m_camera_info[m_cameraMode].sensor.buffer[index_sensor]));

    m_flashMgr->execFunction(ExynosCameraActivityBase::CALLBACK_TYPE_SENSOR_BEFORE,
        (void *)&(m_camera_info[m_cameraMode].sensor.buffer[index_sensor]));

    m_sCaptureMgr->execFunction(ExynosCameraActivityBase::CALLBACK_TYPE_SENSOR_BEFORE,
        (void *)&(m_camera_info[m_cameraMode].sensor.buffer[index_sensor]));

    m_captureBayerIndex[index_sensor] = 1;

#ifdef BAYER_TRACKING
    CLOGD("putReservedSensorBuf qbuf indx[%d]", index_sensor);
    printBayerLockStatus();
#endif

    if (cam_int_qbuf(&(m_camera_info[cameraMode].sensor), index_sensor) < 0)  {
        CLOGE("ERR(%s):cam_int_qbuf(%d) on sensor fail", __func__, index_sensor);
        return false;
    }

    m_camera_info[m_cameraMode].sensor.buffer[index_sensor].reserved.extP[FRAME_COUNT_INDEX] = m_sensorFrameCount++;

    return true;
}

bool ExynosCamera::StartStream(void)
{
    CLOGD("DEBUG(%s):in", __func__);

    if (exynos_v4l2_s_ctrl(m_camera_info[m_cameraMode].sensor.fd,
                V4L2_CID_IS_S_STREAM, IS_ENABLE_STREAM) < 0) {
        CLOGE("ERR(%s):exynos_v4l2_s_ctrl fail(IS_ENABLE_STREAM)", __func__);
        return false;
    }
    return true;
}

bool ExynosCamera::getISPBuf(ExynosBuffer *buf)
{
    if (m_flagCreate == false) {
        CLOGE("ERR(%s):Not yet Created", __func__);
        return false;
    }

    if (m_camera_info[m_cameraMode].isp.flagStart == false) {
        CLOGE("ERR(%s):Not yet isp started fail", __func__);
        return false;
    }

    /* isp dq */
    int index_isp = 0;
    camera2_shot_ext *shot_ext;

    CLOGT(m_traceCount, "(%s): dq in", __func__);

    index_isp = cam_int_dqbuf(&(m_camera_info[m_cameraMode].isp));
    if (index_isp < 0) {
        CLOGE("ERR(%s):cam_int_dqbuf() on isp fail", __func__);
        return false;
    }

    CLOGT(m_traceCount, "(%s): dq out(index %d)", __func__, index_isp);

    shot_ext = (struct camera2_shot_ext *)m_camera_info[m_cameraMode].isp.buffer[index_isp].virt.extP[1];

    if (shot_ext->request_isp == 0) {
        /* TODO: Error handling ISP shot done invalid */
        m_traceCount = TRACE_COUNT;
        CLOGE("ERR(%s):isp Shot done is invalid(expected request_isp is %d, but we got %d), skip frame(%d)",
                __func__, m_camera_info[m_cameraMode].dummy_shot.request_isp, shot_ext->request_isp, shot_ext->shot.dm.request.frameCount);
    }

    CLOGT(m_traceCount, "(%s:%d): cam(%d) dq buf(%d), fre(%d) req(%d), pro(%d), cmp(%d)", __func__, __LINE__, m_cameraMode,  shot_ext->shot.dm.request.frameCount,
        shot_ext->free_cnt,
        shot_ext->request_cnt,
        shot_ext->process_cnt,
        shot_ext->complete_cnt);

    m_numOfShotedIspFrame = shot_ext->request_cnt + shot_ext->process_cnt + shot_ext->complete_cnt;

    m_autofocusMgr->execFunction(ExynosCameraActivityBase::CALLBACK_TYPE_ISP_AFTER,
        (void *)&(m_camera_info[m_cameraMode].isp.buffer[index_isp]));

    m_flashMgr->execFunction(ExynosCameraActivityBase::CALLBACK_TYPE_ISP_AFTER,
        (void *)&(m_camera_info[m_cameraMode].isp.buffer[index_isp]));

    m_sCaptureMgr->execFunction(ExynosCameraActivityBase::CALLBACK_TYPE_ISP_AFTER,
        (void *)&(m_camera_info[m_cameraMode].isp.buffer[index_isp]));

    /* back-up isp_dm : result of isp metadata */
    memcpy(&m_camera_info[m_cameraMode].isp_dm.shot.dm.stats, &shot_ext->shot.dm.stats, sizeof(struct camera2_stats_dm));

    buf->reserved.p = index_isp;

    if (m_traceCount > 0)
        m_traceCount--;

    return true;
}

bool ExynosCamera::putISPBuf(ExynosBuffer *buf)
{
    if (m_flagCreate == false) {
        CLOGE("ERR(%s):Not yet Created", __func__);
        return false;
    }

    if (m_camera_info[m_cameraMode].isp.flagStart == false) {
        CLOGE("ERR(%s):Not yet sensor started fail", __func__);
        return false;
    }

    int index_isp = buf->reserved.p;
    camera2_shot_ext * shot_ext;

    /* post setting */
    shot_ext = (struct camera2_shot_ext *)(m_camera_info[m_cameraMode].isp.buffer[index_isp].virt.extP[1]);

    memcpy(&shot_ext->shot.ctl, &m_camera_info[m_cameraMode].dummy_shot.shot.ctl, sizeof(struct camera2_ctl));

    shot_ext->setfile = m_camera_info[m_cameraMode].dummy_shot.setfile;
    shot_ext->request_3ax = m_camera_info[m_cameraMode].dummy_shot.request_3ax;
    shot_ext->request_isp = m_camera_info[m_cameraMode].dummy_shot.request_isp;
    shot_ext->request_scc = m_camera_info[m_cameraMode].dummy_shot.request_scc;
    shot_ext->request_scp = m_camera_info[m_cameraMode].dummy_shot.request_scp;

#ifdef USE_VDIS
    if (m_recordingHint == true) {
        if (m_curCameraInfo[m_cameraMode]->videoStabilization == true) {
            m_camera_info[m_cameraMode].dummy_shot.request_dis = 1;

            if (m_isHWVDis) {
                 m_camera_info[m_cameraMode].dummy_shot.dis_bypass = 0;
            } else {
                m_camera_info[m_cameraMode].dummy_shot.dis_bypass = 1;
            }
        } else {
            m_camera_info[m_cameraMode].dummy_shot.request_dis = 0;
            m_camera_info[m_cameraMode].dummy_shot.dis_bypass = 1;
        }
    } else {
        m_camera_info[m_cameraMode].dummy_shot.request_dis = 0;
        m_camera_info[m_cameraMode].dummy_shot.dis_bypass = 1;
    }
#else
    m_camera_info[m_cameraMode].dummy_shot.request_dis = 0;
    m_camera_info[m_cameraMode].dummy_shot.dis_bypass = 1;
#endif

    shot_ext->request_dis = m_camera_info[m_cameraMode].dummy_shot.request_dis;

    shot_ext->dis_bypass = m_camera_info[m_cameraMode].dummy_shot.dis_bypass;
    shot_ext->dnr_bypass = m_camera_info[m_cameraMode].dummy_shot.dnr_bypass;
    shot_ext->fd_bypass = m_camera_info[m_cameraMode].dummy_shot.fd_bypass;
    shot_ext->shot.magicNumber= m_camera_info[m_cameraMode].dummy_shot.shot.magicNumber;
#ifdef FD_ROTATION
    shot_ext->shot.uctl.scalerUd.orientation = m_camera_info[m_cameraMode].dummy_shot.shot.uctl.scalerUd.orientation;
#endif

    shot_ext->shot.ctl.request.frameCount = m_camera_info[m_cameraMode].isp.buffer[index_isp].reserved.extP[FRAME_COUNT_INDEX];

    CLOGV("[%s] (%d) %d %d %d %d", __func__, __LINE__,
        shot_ext->shot.udm.internal.vendorSpecific1[0],
        shot_ext->shot.udm.internal.vendorSpecific1[1],
        shot_ext->shot.udm.internal.vendorSpecific2[0],
        shot_ext->shot.udm.internal.vendorSpecific2[1]);
    CLOGV("[%s] (%d)(%d)", __func__, __LINE__, shot_ext->shot.dm.request.frameCount);

    m_autofocusMgr->execFunction(ExynosCameraActivityBase::CALLBACK_TYPE_ISP_BEFORE,
        (void *)&(m_camera_info[m_cameraMode].isp.buffer[buf->reserved.p]));

    m_flashMgr->execFunction(ExynosCameraActivityBase::CALLBACK_TYPE_ISP_BEFORE,
        (void *)&(m_camera_info[m_cameraMode].isp.buffer[buf->reserved.p]));

    m_sCaptureMgr->execFunction(ExynosCameraActivityBase::CALLBACK_TYPE_ISP_BEFORE,
        (void *)&(m_camera_info[m_cameraMode].isp.buffer[buf->reserved.p]));

    memcpy(&m_camera_info[m_cameraMode].dummy_shot.shot.ctl, &shot_ext->shot.ctl, sizeof(struct camera2_ctl));

    CLOGT(m_traceCount, "(%s): q in", __func__);

#ifdef FRONT_NO_ZSL
    if (m_frontCaptureStatus == 1) {
        shot_ext->request_scc = 1;
        m_frontCaptureStatus = 2;

        CLOGD("DEBUG(%s):(%d) m_frontCaptureStatus %d m_frontCaptureStatus %d", __func__, __LINE__, m_frontCaptureStatus, m_frontCaptureStatus);
    } else if (m_frontCaptureStatus == 2) {
        shot_ext->request_scc = 0;
        m_frontCaptureStatus = 0;

        CLOGD("DEBUG(%s):(%d) m_frontCaptureStatus %d m_frontCaptureStatus %d", __func__, __LINE__, m_frontCaptureStatus, m_frontCaptureStatus);
    }
#endif

#ifdef FD_ROTATION
    CLOGV("[%s] (%d) shot_ext->shot.uctl.scalerUd.orientation %d", __func__, __LINE__, shot_ext->shot.uctl.scalerUd.orientation);
#endif

    /* isp q */
    if (cam_int_qbuf(&(m_camera_info[m_cameraMode].isp), index_isp) < 0) {
        CLOGE("ERR(%s):cam_int_qbuf(%d) on isp fail", __func__, index_isp);
        return false;
    }

    CLOGT(m_traceCount, "(%s): q out", __func__);

    return true;
}

bool ExynosCamera::getISPBufReprocessing(ExynosBuffer *buf)
{
    camera2_shot_ext *shot_ext;

    if (m_flagCreate == false) {
        CLOGE("ERR(%s):Not yet Created", __func__);
        return false;
    }

    if (m_camera_info[CAMERA_MODE_REPROCESSING].isp.flagStart == false) {
        CLOGE("ERR(%s):Not yet isp started fail", __func__);
        return false;
    }

    CLOGT(m_traceCount, "(%s): dq in", __func__);

    /* isp dq */
    int index_isp = 0;
    index_isp = cam_int_dqbuf(&(m_camera_info[CAMERA_MODE_REPROCESSING].isp));

    CLOGT(m_traceCount, "(%s): dq out", __func__);

    if (index_isp < 0) {
        CLOGE("ERR(%s):cam_int_dqbuf() on isp fail", __func__);
        return false;
    }

    shot_ext = (struct camera2_shot_ext *)m_camera_info[CAMERA_MODE_REPROCESSING].isp.buffer[index_isp].virt.extP[1];
    if (shot_ext->request_isp == 0) {
        /* TODO: Error handling ISP shot done invalid */
        m_traceCount = TRACE_COUNT;
        CLOGE("ERR(%s):isp Shot done is invalid(expected request_isp is %d, but we got %d), skip frame(%d)",
                __func__, m_camera_info[CAMERA_MODE_REPROCESSING].dummy_shot.request_isp, shot_ext->request_isp, shot_ext->shot.dm.request.frameCount);
    }

    *buf = m_camera_info[m_cameraMode].isp.buffer[index_isp];
    buf->reserved.p = index_isp;

    return true;
}

bool ExynosCamera::putISPBufReprocessing(ExynosBuffer *buf)
{
    if (m_flagCreate == false) {
        CLOGE("ERR(%s):Not yet Created", __func__);
        return false;
    }

    if (m_camera_info[CAMERA_MODE_REPROCESSING].isp.flagStart == false) {
        CLOGE("ERR(%s):Not yet isp started fail", __func__);
        return false;
    }

    camera2_shot_ext * shot_ext;

    /* post setting */
    shot_ext = (struct camera2_shot_ext *)(m_camera_info[CAMERA_MODE_REPROCESSING].isp.buffer[buf->reserved.p].virt.extP[1]);
    shot_ext->request_scc = m_camera_info[CAMERA_MODE_REPROCESSING].dummy_shot.request_scc;
    shot_ext->request_scp = 0;
    shot_ext->shot.ctl.scaler.cropRegion[0] = m_camera_info[m_cameraMode].dummy_shot.shot.ctl.scaler.cropRegion[0];
    shot_ext->shot.ctl.scaler.cropRegion[1] = m_camera_info[m_cameraMode].dummy_shot.shot.ctl.scaler.cropRegion[1];
    shot_ext->shot.ctl.scaler.cropRegion[2] = m_camera_info[m_cameraMode].dummy_shot.shot.ctl.scaler.cropRegion[2];
    shot_ext->shot.ctl.scaler.cropRegion[3] = m_camera_info[m_cameraMode].dummy_shot.shot.ctl.scaler.cropRegion[3];

/* TODO: remove if unused */
    shot_ext->dis_bypass = 1;
    shot_ext->dnr_bypass = 1;
    shot_ext->drc_bypass = 1;
    shot_ext->fd_bypass = 1;

    shot_ext->request_scc = 1;

    shot_ext->shot.ctl.request.frameCount = buf->reserved.extP[FRAME_COUNT_INDEX];

    /* isp q */
    if (cam_int_qbuf(&(m_camera_info[CAMERA_MODE_REPROCESSING].isp), buf->reserved.p) < 0) {
        CLOGE("ERR(%s):cam_int_qbuf(%d) on isp fail", __func__, buf->reserved.p);
        return false;
    }

    return true;
}

bool ExynosCamera::DvfsLock()
{
    CLOGV("[%s] (%d)", __func__, __LINE__);

    Mutex::Autolock lock(m_dvfsLock);

    if (m_isDVFSLocked == true) {
        CLOGW("WRN(%s): DVFS level is already locked", __func__);
        return true;
    }

    if (exynos_v4l2_s_ctrl(m_camera_info[m_cameraMode].isp.fd,
                V4L2_CID_IS_DVFS_LOCK, 800000) < 0) {
        CLOGE("ERR(%s):exynos_v4l2_s_ctrl fail(IS_DVFS_LOCK)", __func__);
        m_isDVFSLocked = false;
        return false;
    } else {
        m_isDVFSLocked = true;
        CLOGD("DEBUG(%s) (%d) IS_DVFS locked", __func__, __LINE__);
    }

    return true;
}

bool ExynosCamera::DvfsUnLock()
{
    CLOGV("[%s] (%d)", __func__, __LINE__);

    Mutex::Autolock lock(m_dvfsLock);

    if (m_isDVFSLocked == false) {
        CLOGW("WRN(%s): DVFS level is not locked", __func__);
        return false;
    }

    if (exynos_v4l2_s_ctrl(m_camera_info[m_cameraMode].isp.fd,
                V4L2_CID_IS_DVFS_UNLOCK, 800000) < 0) {
        CLOGE("ERR(%s):exynos_v4l2_s_ctrl fail(IS_DVFS_UNLOCK)", __func__);
        return false;
    } else {
        m_isDVFSLocked = false;
        CLOGD("DEBUG(%s) (%d) IS_DVFS unlocked", __func__, __LINE__);
    }

    return true;
}

bool ExynosCamera::startIsp(void)
{
    CLOGD("DEBUG(%s):in", __func__);

    CLOGD("(flagStart %d), (%d x %d), format 0x%x", m_camera_info[m_cameraMode].isp.flagStart,
        m_camera_info[m_cameraMode].isp.width,
        m_camera_info[m_cameraMode].isp.height,
        m_camera_info[m_cameraMode].isp.format);

    if (m_camera_info[m_cameraMode].isp.flagStart == false) {
        m_camera_info[m_cameraMode].isp.width   = m_curCameraInfo[m_cameraMode]->ispW;
        m_camera_info[m_cameraMode].isp.height  = m_curCameraInfo[m_cameraMode]->ispH;
        m_camera_info[m_cameraMode].isp.format  = V4L2_PIX_FMT_SBGGR12;
        m_camera_info[m_cameraMode].isp.planes  = 2;
        m_camera_info[m_cameraMode].isp.buffers = NUM_BAYER_BUFFERS;
        m_camera_info[m_cameraMode].isp.memory  = V4L2_CAMERA_MEMORY_TYPE;
        m_camera_info[m_cameraMode].isp.type    = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
        m_camera_info[m_cameraMode].isp.ionClient = m_ionCameraClient;

        ExynosBuffer nullBuf;
        int planeSize;
        planeSize = getPlaneSizePackedFLiteOutput(m_curCameraInfo[m_cameraMode]->ispW, m_curCameraInfo[m_cameraMode]->ispH);

        if (m_cameraMode == CAMERA_MODE_FRONT) {
            for (int i = 0; i < m_camera_info[m_cameraMode].isp.buffers; i++) {
                m_camera_info[m_cameraMode].isp.buffer[i] = nullBuf;
                m_camera_info[m_cameraMode].isp.buffer[i].size.extS[0] = planeSize;
                m_camera_info[m_cameraMode].isp.buffer[i].size.extS[1] = META_DATA_SIZE; /* driver use 12*1024, should be use predefined value */

                if (allocMem(m_camera_info[m_cameraMode].isp.ionClient, &m_camera_info[m_cameraMode].isp.buffer[i], 1 << 1) == false) {
                    CLOGE("ERR(%s):allocMem() fail", __func__);
                    goto err;
                } else {
                    memset(m_camera_info[m_cameraMode].isp.buffer[i].virt.extP[1],
                            0, m_camera_info[m_cameraMode].isp.buffer[i].size.extS[1]);
                }
            }
        } else if (m_cameraMode == CAMERA_MODE_BACK) {
            for (int i = 0; i < m_camera_info[m_cameraMode].isp.buffers; i++) {
                m_camera_info[m_cameraMode].isp.buffer[i] = nullBuf;
                m_camera_info[m_cameraMode].isp.buffer[i].size.extS[0] = planeSize;
                m_camera_info[m_cameraMode].isp.buffer[i].size.extS[1] = META_DATA_SIZE;

                if (allocMem(m_camera_info[m_cameraMode].isp.ionClient, &m_camera_info[m_cameraMode].isp.buffer[i], 1 << 1) == false) {
                    CLOGE("ERR(%s):allocMem() fail", __func__);
                    goto err;
                } else {
                    memset(m_camera_info[m_cameraMode].isp.buffer[i].virt.extP[1],
                            0, m_camera_info[m_cameraMode].isp.buffer[i].size.extS[1]);
                }
            }
        }

        /* meatadata buffer */
        for (int i = 0; i < NUM_BAYER_BUFFERS; i++) {
            m_metaBuf[i] = nullBuf;
            m_metaBuf[i].size.extS[0] = 0;
            m_metaBuf[i].size.extS[1] = META_DATA_SIZE;

            if (allocMem(m_camera_info[m_cameraMode].isp.ionClient, &m_metaBuf[i], 1 << 1) == false) {
                CLOGE("ERR(%s):allocMem() fail", __func__);
                goto err;
            } else {
                memset(m_metaBuf[i].virt.extP[1], 0, m_metaBuf[i].size.extS[1]);
            }
        }

        if (m_startIsp() == false) {
            CLOGE("ERR(%s):m_startIsp() fail", __func__);
            goto err;
        }

        m_camera_info[m_cameraMode].isp.flagStart = true;
    }

    return true;

err:
    if (stopIsp() == false)
        CLOGE("ERR(%s):stopIsp() fail", __func__);

    return false;
}

bool ExynosCamera::m_startIsp(void)
{
   CLOGD("DEBUG(%s):in", __func__);

    /* Reset Dummy Shot's request */
    m_camera_info[m_cameraMode].dummy_shot.request_3ax = 1;
    m_camera_info[m_cameraMode].dummy_shot.request_isp = 1;
    m_camera_info[m_cameraMode].dummy_shot.request_scc = 0;
    m_camera_info[m_cameraMode].dummy_shot.request_scp = 1;

#ifdef USE_VDIS
    if (m_recordingHint == true) {
        if (m_curCameraInfo[m_cameraMode]->videoStabilization == true) {
            m_camera_info[m_cameraMode].dummy_shot.request_dis = 1;

            if (m_isHWVDis) {
                m_camera_info[m_cameraMode].dummy_shot.dis_bypass = 0;
            } else {
                m_camera_info[m_cameraMode].dummy_shot.dis_bypass = 1;
            }
        } else {
            m_camera_info[m_cameraMode].dummy_shot.request_dis = 0;
            m_camera_info[m_cameraMode].dummy_shot.dis_bypass = 1;
        }
    } else {
        m_camera_info[m_cameraMode].dummy_shot.request_dis = 0;
        m_camera_info[m_cameraMode].dummy_shot.dis_bypass = 1;
    }
#else
    m_camera_info[m_cameraMode].dummy_shot.request_dis = 0;
    m_camera_info[m_cameraMode].dummy_shot.dis_bypass = 1;
#endif

    if (getVideoStabilization() == true) {
        if (setVideoStabilization(true) == false)
            CLOGE("ERR(%s):setVideoStabilization() fail", __func__);
    }

    /* FD-AE */
    if (this->getRecordingHint() == true) {
        if (m_startFaceDetection((enum CAMERA_MODE)m_cameraMode, false) == false)
            CLOGE("ERR(%s):m_startFaceDetection(%d, %d) fail", __func__, m_cameraMode, false);
    } else { /* turn on at most case. */
        if (m_startFaceDetection((enum CAMERA_MODE)m_cameraMode, true) == false)
            CLOGE("ERR(%s):m_startFaceDetection(%d, %d) fail", __func__, m_cameraMode, true);
    }

    // HACK : ISP need to skip s_input on second Preview (fw?)
    if (m_isFirtstIspStart == true) {
        int sensorId = m_getSensorId(m_cameraMode);

        if (m_cameraMode == CAMERA_MODE_FRONT)
            sensorId = (0 << REPROCESSING_SHFIT) |
                ((FIMC_IS_VIDEO_SEN1_NUM - FIMC_IS_VIDEO_SEN0_NUM) << SENSOR_INDEX_SHFIT) |
                ((FIMC_IS_VIDEO_3A0_NUM - FIMC_IS_VIDEO_SEN0_NUM) << VIDEO_INDEX_SHFIT) |
                (sensorId << 0);
        else if (m_cameraMode == CAMERA_MODE_BACK)
            sensorId = (0 << REPROCESSING_SHFIT) |
                ((FIMC_IS_VIDEO_SEN0_NUM - FIMC_IS_VIDEO_SEN0_NUM) << SENSOR_INDEX_SHFIT) |
                ((FIMC_IS_VIDEO_3A1_NUM - FIMC_IS_VIDEO_SEN0_NUM) << VIDEO_INDEX_SHFIT) |
                (sensorId << 0);

        if (cam_int_s_input(&(m_camera_info[m_cameraMode].isp), sensorId) < 0) {
            CLOGE("ERR(%s):cam_int_s_input(%d) fail", __func__, sensorId);
            return false;
        }
        m_isFirtstIspStart = false;
    }

    if (cam_int_s_fmt(&(m_camera_info[m_cameraMode].isp)) < 0) {
        CLOGE("ERR(%s):cam_int_s_fmt() fail", __func__);
        return false;
    }

    if (cam_int_reqbufs(&(m_camera_info[m_cameraMode].isp)) < 0) {
        CLOGE("ERR(%s):cam_int_reqbufs() fail", __func__);
        return false;
    }

    return true;
}

bool ExynosCamera::stopIspForceOff(void)
{
    CLOGD("[esdreset] [%s] START ", __func__);

    // isp stream off
    if (cam_int_streamoff(&m_camera_info[m_cameraMode].isp) < 0) {
        CLOGE("[esdreset] ERR(%s):exynos_v4l2_streamoff() fail", __func__);
        return false;
    }

#ifdef FORCE_LEADER_OFF
    m_forceIspOff = 1;
#endif

    CLOGD("[esdreset] [%s] isp stream_off/force done ", __func__);

    return true;
}

bool ExynosCamera::stopSensorForceOff(void)
{
    //sensor
    if (exynos_v4l2_s_ctrl(m_camera_info[m_cameraMode].isp.fd, V4L2_CID_IS_FORCE_DONE, 0x1000) < 0) {
        CLOGE("[esdreset]ERR(%s): sensor exynos_v4l2_s_ctrl force_done fail(sensor)", __func__);
    }
    CLOGD("[esdreset] [%s] DONE", __func__);

    return true;
}

bool ExynosCamera::m_stopIsp(void)
{
    CLOGD("DEBUG(%s):in", __func__);

    bool ret = true;

#ifdef FORCE_DONE
#ifdef FORCE_LEADER_OFF
    if (m_forceIspOff != true) {
#endif
        if (exynos_v4l2_s_ctrl(m_camera_info[m_cameraMode].isp.fd,
            V4L2_CID_IS_FORCE_DONE, 1) < 0) {
            CLOGE("ERR(%s):exynos_v4l2_s_ctrl fail(V4L2_CID_IS_FORCE_DONE)", __func__);
            ret = false;
        }
#ifdef FORCE_LEADER_OFF
    }
#endif
#endif

#ifdef THREAD_PROFILE
    timeUs = 0;
    gettimeofday(&mTimeStart, NULL);
#endif

#ifdef FORCE_LEADER_OFF
    if (m_forceIspOff != true) {
#endif
        if (cam_int_streamoff(&m_camera_info[m_cameraMode].isp) < 0) {
            CLOGW("WRR(%s):exynos_v4l2_streamoff() fail", __func__);
            ret = false;
        }
#ifdef FORCE_LEADER_OFF
    }
#endif

#ifdef THREAD_PROFILE
    gettimeofday(&mTimeStop, NULL);
    timeUs = (mTimeStop.tv_sec*1000000 + mTimeStop.tv_usec) - (mTimeStart.tv_sec*1000000 + mTimeStart.tv_usec);
    CLOGD("DEBUG(%s):time_check elapsed time=(%ld)us", __func__, timeUs);
#endif

    if (0 < m_camera_info[m_cameraMode].isp.buffers) {
        if (cam_int_clrbufs(&m_camera_info[m_cameraMode].isp) < 0) {
            CLOGE("ERR(%s):cam_int_clrbufs() fail", __func__);
            ret = false;
        }
    }

    return ret;
}
bool ExynosCamera::stopIsp(void)
{
    CLOGD("DEBUG(%s):in", __func__);

    bool ret = true;

    if (m_camera_info[m_cameraMode].isp.flagStart == true) {

        if (m_stopIsp() == false) {
            CLOGE("ERR(%s):m_stopIsp() fail", __func__);
            ret = false;
        }

        if (0 < m_camera_info[m_cameraMode].isp.buffers) {
            for (int i = 0; i < m_camera_info[m_cameraMode].isp.buffers; i++)
                freeMem(&m_camera_info[m_cameraMode].isp.buffer[i]);

            m_camera_info[m_cameraMode].isp.buffers = 0;
        }

        for (int i = 0; i < VIDEO_MAX_FRAME; i++)
            freeMemSinglePlane(&m_camera_info[m_cameraMode].isp.buffer[i], m_camera_info[m_cameraMode].isp.planes - 1);

        /* metadata buffer */
        for (int i = 0; i < NUM_BAYER_BUFFERS; i++)
            freeMem(&m_metaBuf[i]);

        m_camera_info[m_cameraMode].isp.flagStart = false;
    }

    return ret;
}

bool ExynosCamera::flagStartIsp(void)
{
    return m_camera_info[m_cameraMode].isp.flagStart;
}

bool ExynosCamera::startIs3a0(enum CAMERA_MODE cameraMode)
{
    CLOGD("DEBUG(%s):in", __func__);

    if (m_setSetfile(cameraMode) == false) {
        CLOGE("ERR(%s):m_setSetfile(%d) fail", __func__, cameraMode);
        return false;
    }

    CLOGD("(flagStart %d), (%d x %d), format 0x%x", m_camera_info[cameraMode].is3a0Src.flagStart,
        m_camera_info[cameraMode].is3a0Src.width,
        m_camera_info[cameraMode].is3a0Src.height,
        m_camera_info[cameraMode].is3a0Src.format);

    if (m_camera_info[cameraMode].is3a0Src.flagStart == false) {
        m_camera_info[cameraMode].is3a0Src.width   = m_curCameraInfo[cameraMode]->pictureW + 16;
        m_camera_info[cameraMode].is3a0Src.height  = m_curCameraInfo[cameraMode]->pictureH + 10;
        m_camera_info[cameraMode].is3a0Src.format  = V4L2_PIX_FMT_SBGGR12;
        m_camera_info[cameraMode].is3a0Src.planes  = 2;
        m_camera_info[cameraMode].is3a0Src.buffers = NUM_BAYER_BUFFERS;
        m_camera_info[cameraMode].is3a0Src.memory  = V4L2_CAMERA_MEMORY_TYPE;
        m_camera_info[cameraMode].is3a0Src.type    = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
        m_camera_info[cameraMode].is3a0Src.ionClient = m_ionCameraClient;

        if (m_isFirtstIs3a0SrcStart == true) {
            int sensorId = m_getSensorId(m_cameraMode);

            if (cam_int_s_input(&(m_camera_info[cameraMode].is3a0Src), sensorId) < 0) {
                CLOGE("ERR(%s):cam_int_s_input(%d) fail", __func__, sensorId);
                return false;
            }
            m_isFirtstIs3a0SrcStart = false;
        }

        if (cam_int_s_fmt(&(m_camera_info[cameraMode].is3a0Src)) < 0) {
            CLOGE("ERR(%s):cam_int_s_fmt() fail", __func__);
            return false;
        }

        m_camera_info[cameraMode].is3a0Src.buffers = NUM_BAYER_BUFFERS;
        if (cam_int_reqbufs(&(m_camera_info[cameraMode].is3a0Src)) < 0) {
            CLOGE("ERR(%s):is3a0Src cam_int_reqbufs() fail", __func__);
            return false;
        }
        if (cam_int_streamon(&(m_camera_info[cameraMode].is3a0Src)) < 0 ) {
            CLOGE("ERR(%s):cam_int_streamon(3a0 output) fail", __func__);
            return false;
        }

        m_camera_info[cameraMode].is3a0Src.flagStart = true;
    }

    CLOGD("(flagStart %d), (%d x %d), format 0x%x", m_camera_info[cameraMode].is3a0Dst.flagStart,
        m_camera_info[cameraMode].is3a0Dst.width,
        m_camera_info[cameraMode].is3a0Dst.height,
        m_camera_info[cameraMode].is3a0Dst.format);

    if (m_camera_info[cameraMode].is3a0Dst.flagStart == false) {
        m_camera_info[cameraMode].is3a0Dst.width   = m_curCameraInfo[cameraMode]->ispW;
        m_camera_info[cameraMode].is3a0Dst.height  = m_curCameraInfo[cameraMode]->ispH;
        m_camera_info[cameraMode].is3a0Dst.format  = V4L2_PIX_FMT_SBGGR12;
        m_camera_info[cameraMode].is3a0Dst.planes  = 2;
        m_camera_info[cameraMode].is3a0Dst.buffers = NUM_BAYER_BUFFERS;
        m_camera_info[cameraMode].is3a0Dst.memory  = V4L2_CAMERA_MEMORY_TYPE;
        m_camera_info[cameraMode].is3a0Dst.type    = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
        m_camera_info[cameraMode].is3a0Dst.ionClient = m_ionCameraClient;

        if (m_isFirtstIs3a0DstStart == true) {
            int sensorId = m_getSensorId(m_cameraMode);

            if (cam_int_s_input(&(m_camera_info[cameraMode].is3a0Dst), sensorId) < 0) {
                CLOGE("ERR(%s):cam_int_s_input(%d) fail", __func__, sensorId);
                return false;
            }
            m_isFirtstIs3a0DstStart = false;
        }

        if (cam_int_s_fmt(&(m_camera_info[cameraMode].is3a0Dst)) < 0) {
            CLOGE("ERR(%s):cam_int_s_fmt() fail", __func__);
            return false;
        }

        m_camera_info[cameraMode].is3a0Dst.buffers = NUM_BAYER_BUFFERS;
        if (cam_int_reqbufs(&(m_camera_info[cameraMode].is3a0Dst)) < 0) {
            CLOGE("ERR(%s):is3a0Dst cam_int_reqbufs() fail", __func__);
            return false;
        }
        if (cam_int_streamon(&(m_camera_info[cameraMode].is3a0Dst)) < 0) {
            CLOGE("ERR(%s):cam_int_streamon(3a0 capture) fail", __func__);
            return false;
        }

        m_camera_info[cameraMode].is3a0Dst.flagStart = true;
    }

    return true;
}

bool ExynosCamera::stopIs3a0(enum CAMERA_MODE cameraMode)
{
    CLOGD("DEBUG(%s):in", __func__);

    bool ret = true;

    if (m_camera_info[cameraMode].is3a0Src.flagStart == true) {
#ifdef FORCE_DONE
        if (exynos_v4l2_s_ctrl(m_camera_info[cameraMode].is3a0Src.fd,
            V4L2_CID_IS_FORCE_DONE, 1) < 0) {
            CLOGE("ERR(%s):exynos_v4l2_s_ctrl fail(V4L2_CID_IS_FORCE_DONE)", __func__);
            ret = false;
        }
#endif

#ifdef THREAD_PROFILE
        timeUs = 0;
        gettimeofday(&mTimeStart, NULL);
#endif
        if (cam_int_streamoff(&(m_camera_info[cameraMode].is3a0Src)) < 0) {
            CLOGE("ERR(%s):exynos_v4l2_streamoff() fail", __func__);
            ret = false;
        }
#ifdef THREAD_PROFILE
        gettimeofday(&mTimeStop, NULL);
        timeUs = (mTimeStop.tv_sec*1000000 + mTimeStop.tv_usec) - (mTimeStart.tv_sec*1000000 + mTimeStart.tv_usec);
        CLOGD("DEBUG(%s):time_check elapsed time=(%ld)us", __func__, timeUs);
#endif

        if (0 < m_camera_info[cameraMode].is3a0Src.buffers) {
            int buffers = m_camera_info[cameraMode].is3a0Src.buffers;

            m_camera_info[cameraMode].is3a0Src.buffers = 0;
            if (cam_int_reqbufs(&(m_camera_info[cameraMode].is3a0Src)) < 0) {
                CLOGE("ERR(%s):is3a0Src cam_int_reqbufs() fail", __func__);
                ret = false;
            }

            for (int i = 0; i < buffers; i++)
                freeMem(&m_camera_info[cameraMode].is3a0Src.buffer[i]);
        }

        for (int i = 0; i < VIDEO_MAX_FRAME; i++)
            freeMemSinglePlane(&m_camera_info[cameraMode].is3a0Src.buffer[i], m_camera_info[cameraMode].is3a0Src.planes - 1);

        m_camera_info[cameraMode].is3a0Src.flagStart = false;
    }

    if (m_camera_info[cameraMode].is3a0Dst.flagStart == true) {
#ifdef FORCE_DONE
        if (exynos_v4l2_s_ctrl(m_camera_info[cameraMode].is3a0Dst.fd,
            V4L2_CID_IS_FORCE_DONE, 1) < 0) {
            CLOGE("ERR(%s):exynos_v4l2_s_ctrl fail(V4L2_CID_IS_FORCE_DONE)", __func__);
            ret = false;
        }
#endif
#ifdef THREAD_PROFILE
        timeUs = 0;
        gettimeofday(&mTimeStart, NULL);
#endif
        if (cam_int_streamoff(&(m_camera_info[cameraMode].is3a0Dst)) < 0) {
            CLOGE("ERR(%s):exynos_v4l2_streamoff() fail", __func__);
            ret = false;
        }
#ifdef THREAD_PROFILE
        gettimeofday(&mTimeStop, NULL);
        timeUs = (mTimeStop.tv_sec*1000000 + mTimeStop.tv_usec) - (mTimeStart.tv_sec*1000000 + mTimeStart.tv_usec);
        CLOGD("DEBUG(%s):time_check elapsed time=(%ld)us", __func__, timeUs);
#endif

        if (0 < m_camera_info[cameraMode].is3a0Dst.buffers) {
            int buffers = m_camera_info[cameraMode].is3a0Dst.buffers;

            m_camera_info[cameraMode].is3a0Dst.buffers = 0;
            if (cam_int_reqbufs(&(m_camera_info[cameraMode].is3a0Dst)) < 0) {
                CLOGE("ERR(%s):is3a0Dst cam_int_reqbufs() fail", __func__);
                ret = false;
            }

            for (int i = 0; i < buffers; i++)
                freeMem(&m_camera_info[cameraMode].is3a0Dst.buffer[i]);
        }

        for (int i = 0; i < VIDEO_MAX_FRAME; i++)
            freeMemSinglePlane(&m_camera_info[cameraMode].is3a0Dst.buffer[i], m_camera_info[cameraMode].is3a0Dst.planes - 1);

        m_camera_info[cameraMode].is3a0Dst.flagStart = false;
    }

    return ret;
}

bool ExynosCamera::flagStartIs3a0Src(enum CAMERA_MODE cameraMode)
{
    return m_camera_info[cameraMode].is3a0Src.flagStart;
}

bool ExynosCamera::flagStartIs3a0Dst(enum CAMERA_MODE cameraMode)
{
    return m_camera_info[cameraMode].is3a0Dst.flagStart;
}

bool ExynosCamera::startIs3a1(enum CAMERA_MODE cameraMode)
{
    CLOGD("DEBUG(%s):in", __func__);

    if (m_setSetfile(cameraMode) == false) {
        CLOGE("ERR(%s):m_setSetfile(%d) fail", __func__, cameraMode);
        return false;
    }

    CLOGD("(flagStart %d), (%d x %d), format 0x%x", m_camera_info[cameraMode].is3a1Src.flagStart,
        m_camera_info[cameraMode].is3a1Src.width,
        m_camera_info[cameraMode].is3a1Src.height,
        m_camera_info[cameraMode].is3a1Src.format);

    if (m_camera_info[cameraMode].is3a1Src.flagStart == false) {
        m_camera_info[cameraMode].is3a1Src.width   = SIZE_OTF_WIDTH;
        m_camera_info[cameraMode].is3a1Src.height  = SIZE_OTF_HEIGHT;
        m_camera_info[cameraMode].is3a1Src.format  = V4L2_PIX_FMT_SBGGR12;
        m_camera_info[cameraMode].is3a1Src.planes  = 2;
        m_camera_info[cameraMode].is3a1Src.buffers = NUM_BAYER_BUFFERS;
        m_camera_info[cameraMode].is3a1Src.memory  = V4L2_CAMERA_MEMORY_TYPE;
        m_camera_info[cameraMode].is3a1Src.type    = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
        m_camera_info[cameraMode].is3a1Src.ionClient = m_ionCameraClient;

        if (m_isFirtstIs3a1SrcStart == true) {
            int sensorId = m_getSensorId(m_cameraMode);

            if (cam_int_s_input(&(m_camera_info[cameraMode].is3a1Src), sensorId) < 0) {
                CLOGE("ERR(%s):cam_int_s_input(%d) fail", __func__, sensorId);
                return false;
            }
            m_isFirtstIs3a1SrcStart = false;
        }

        if (cam_int_s_fmt(&(m_camera_info[cameraMode].is3a1Src)) < 0) {
            CLOGE("ERR(%s):cam_int_s_fmt() fail", __func__);
            return false;
        }

        m_camera_info[cameraMode].is3a1Src.buffers = NUM_BAYER_BUFFERS;
        if (cam_int_reqbufs(&(m_camera_info[cameraMode].is3a1Src)) < 0) {
            CLOGE("ERR(%s):is3a1Src cam_int_reqbufs() fail", __func__);
            return false;
        }

        if (cam_int_streamon(&(m_camera_info[cameraMode].is3a1Src)) < 0) {
            CLOGE("ERR(%s):cam_int_streamon(3a1 output) fail", __func__);
            return false;
        }

        m_camera_info[cameraMode].is3a1Src.flagStart = true;
    }

    CLOGD("(flagStart %d), (%d x %d), format 0x%x", m_camera_info[cameraMode].is3a1Dst.flagStart,
        m_camera_info[cameraMode].is3a1Dst.width,
        m_camera_info[cameraMode].is3a1Dst.height,
        m_camera_info[cameraMode].is3a1Dst.format);

    if (m_camera_info[cameraMode].is3a1Dst.flagStart == false) {
        m_camera_info[cameraMode].is3a1Dst.width   = m_curCameraInfo[cameraMode]->ispW;
        m_camera_info[cameraMode].is3a1Dst.height  = m_curCameraInfo[cameraMode]->ispH;
        m_camera_info[cameraMode].is3a1Dst.format  = V4L2_PIX_FMT_SBGGR12;
        m_camera_info[cameraMode].is3a1Dst.planes  = 2;
        m_camera_info[cameraMode].is3a1Dst.buffers = NUM_BAYER_BUFFERS;
        m_camera_info[cameraMode].is3a1Dst.memory  = V4L2_CAMERA_MEMORY_TYPE;
        m_camera_info[cameraMode].is3a1Dst.type    = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
        m_camera_info[cameraMode].is3a1Dst.ionClient = m_ionCameraClient;

        if (m_isFirtstIs3a1DstStart == true) {
            int sensorId = m_getSensorId(m_cameraMode);

            if (cam_int_s_input(&(m_camera_info[cameraMode].is3a1Dst), sensorId) < 0) {
                CLOGE("ERR(%s):cam_int_s_input(%d) fail", __func__, sensorId);
                return false;
            }
            m_isFirtstIs3a1DstStart = false;
        }

        if (cam_int_s_fmt(&(m_camera_info[cameraMode].is3a1Dst)) < 0) {
            CLOGE("ERR(%s):cam_int_s_fmt() fail", __func__);
            return false;
        }

        m_camera_info[cameraMode].is3a1Dst.buffers = NUM_BAYER_BUFFERS;
        if (cam_int_reqbufs(&(m_camera_info[cameraMode].is3a1Dst)) < 0) {
            CLOGE("ERR(%s):is3a1Dst cam_int_reqbufs() fail", __func__);
            return false;
        }
        if (cam_int_streamon(&(m_camera_info[cameraMode].is3a1Dst)) < 0) {
            CLOGE("ERR(%s):cam_int_streamon(3a1 capture) fail", __func__);
            return false;
        }

        m_camera_info[cameraMode].is3a1Dst.flagStart = true;
    }

    m_cameraModeIs3a0 = cameraMode;

    return true;
}

bool ExynosCamera::stopIs3a1(enum CAMERA_MODE cameraMode)
{
    CLOGD("DEBUG(%s):in", __func__);

    bool ret = true;

    if (m_camera_info[cameraMode].is3a1Src.flagStart == true) {
#ifdef FORCE_DONE
        if (exynos_v4l2_s_ctrl(m_camera_info[cameraMode].is3a1Src.fd,
            V4L2_CID_IS_FORCE_DONE, 1) < 0) {
            CLOGE("ERR(%s):exynos_v4l2_s_ctrl fail(V4L2_CID_IS_FORCE_DONE)", __func__);
            ret = false;
        }
#endif

#ifdef THREAD_PROFILE
        timeUs = 0;
        gettimeofday(&mTimeStart, NULL);
#endif
        if (cam_int_streamoff(&(m_camera_info[cameraMode].is3a1Src)) < 0) {
            CLOGE("ERR(%s):exynos_v4l2_streamoff() fail", __func__);
            ret = false;
        }
#ifdef THREAD_PROFILE
        gettimeofday(&mTimeStop, NULL);
        timeUs = (mTimeStop.tv_sec*1000000 + mTimeStop.tv_usec) - (mTimeStart.tv_sec*1000000 + mTimeStart.tv_usec);
        CLOGD("DEBUG(%s):time_check elapsed time=(%ld)us", __func__, timeUs);
#endif

        if (0 < m_camera_info[cameraMode].is3a1Src.buffers) {
            int buffers = m_camera_info[cameraMode].is3a1Src.buffers;

            m_camera_info[cameraMode].is3a1Src.buffers = 0;
            if (cam_int_reqbufs(&(m_camera_info[cameraMode].is3a1Src)) < 0) {
                CLOGE("ERR(%s):is3a1Src cam_int_reqbufs() fail", __func__);
                ret = false;
            }

            for (int i = 0; i < buffers; i++)
                freeMem(&m_camera_info[cameraMode].is3a1Src.buffer[i]);
        }

        for (int i = 0; i < VIDEO_MAX_FRAME; i++)
            freeMemSinglePlane(&m_camera_info[cameraMode].is3a1Src.buffer[i], m_camera_info[cameraMode].is3a1Src.planes - 1);

        m_camera_info[cameraMode].is3a1Src.flagStart = false;
    }

    if (m_camera_info[cameraMode].is3a1Dst.flagStart == true) {
#ifdef FORCE_DONE
        if (exynos_v4l2_s_ctrl(m_camera_info[cameraMode].is3a1Dst.fd,
            V4L2_CID_IS_FORCE_DONE, 1) < 0) {
            CLOGE("ERR(%s):exynos_v4l2_s_ctrl fail(V4L2_CID_IS_FORCE_DONE)", __func__);
            ret = false;
        }
#endif

#ifdef THREAD_PROFILE
        timeUs = 0;
        gettimeofday(&mTimeStart, NULL);
#endif
        if (cam_int_streamoff(&(m_camera_info[cameraMode].is3a1Dst)) < 0) {
            CLOGE("ERR(%s):exynos_v4l2_streamoff() fail", __func__);
            ret = false;
        }
#ifdef THREAD_PROFILE
        gettimeofday(&mTimeStop, NULL);
        timeUs = (mTimeStop.tv_sec*1000000 + mTimeStop.tv_usec) - (mTimeStart.tv_sec*1000000 + mTimeStart.tv_usec);
        CLOGD("DEBUG(%s):time_check elapsed time=(%ld)us", __func__, timeUs);
#endif

        if (0 < m_camera_info[cameraMode].is3a1Dst.buffers) {
            int buffers = m_camera_info[cameraMode].is3a1Dst.buffers;

            m_camera_info[cameraMode].is3a1Dst.buffers = 0;
            if (cam_int_reqbufs(&(m_camera_info[cameraMode].is3a1Dst)) < 0) {
                CLOGE("ERR(%s):is3a1Dst cam_int_reqbufs() fail", __func__);
                ret = false;
            }

            for (int i = 0; i < buffers; i++)
                freeMem(&m_camera_info[cameraMode].is3a1Dst.buffer[i]);
        }

        for (int i = 0; i < VIDEO_MAX_FRAME; i++)
            freeMemSinglePlane(&m_camera_info[cameraMode].is3a1Dst.buffer[i], m_camera_info[cameraMode].is3a1Dst.planes - 1);

        m_camera_info[cameraMode].is3a1Dst.flagStart = false;
    }

    return ret;
}

bool ExynosCamera::flagStart3a1Src(enum CAMERA_MODE cameraMode)
{
    return m_camera_info[cameraMode].is3a1Src.flagStart;
}

bool ExynosCamera::flagStart3a1Dst(enum CAMERA_MODE cameraMode)
{
    return m_camera_info[cameraMode].is3a1Dst.flagStart;
}

bool ExynosCamera::setVideoSize(int w, int h)
{
    m_curCameraInfo[m_cameraMode]->videoW = w;
    m_curCameraInfo[m_cameraMode]->videoH = h;

#ifdef USE_3DNR_DMAOUT
    // HACK : Video 3dnr port support resize. So, we must make max size video w, h
    m_curCameraInfo[m_cameraMode]->videoW = m_defaultCameraInfo[m_cameraMode]->videoW;
    m_curCameraInfo[m_cameraMode]->videoH = m_defaultCameraInfo[m_cameraMode]->videoH;
#endif
    return true;
}

bool ExynosCamera::getVideoSize(int *w, int *h)
{
    *w = m_curCameraInfo[m_cameraMode]->videoW;
    *h = m_curCameraInfo[m_cameraMode]->videoH;
    return true;
}

bool ExynosCamera::setVideoFormat(int colorFormat)
{
    m_curCameraInfo[m_cameraMode]->videoColorFormat = colorFormat;
    return true;
}

int ExynosCamera::getVideoFormat(void)
{
    return m_curCameraInfo[m_cameraMode]->videoColorFormat;
}

bool ExynosCamera::startVideo(void)
{
    if (m_flagCreate == false) {
        CLOGE("ERR(%s):Not yet Created", __func__);
        return false;
    }

    CLOGD("DEBUG(%s):in", __func__);

    if (m_startFaceDetection((enum CAMERA_MODE)m_cameraMode, false) == false)
        CLOGE("ERR(%s):m_startFaceDetection(%d, %d) fail", __func__, m_cameraMode, false);

#ifdef USE_3DNR_DMAOUT
    if (m_camera_info[m_cameraMode].video.flagStart == false) {
        for (int i = 0; i < VIDEO_MAX_FRAME; i++) {
            if (m_camera_info[m_cameraMode].video.buffer[i].virt.p != NULL ||
                m_camera_info[m_cameraMode].video.buffer[i].phys.p != 0) {
                if (cam_int_qbuf(&m_camera_info[m_cameraMode].video, i) < 0) {
                    CLOGE("ERR(%s):cam_int_qbuf(%d) fail", __func__, i);
                    return false;
                }
            }
        }

        if (exynos_v4l2_streamon(m_camera_info[m_cameraMode].video.fd, V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) < 0) {
            CLOGE("ERR(%s):exynos_v4l2_streamon() fail", __func__);
            return false;
        }

        m_camera_info[m_cameraMode].video.flagStart = true;
    }
#endif

    return true;
}

bool ExynosCamera::stopVideo(void)
{
    if (m_flagCreate == false) {
        CLOGW("WARN(%s):Not yet Created", __func__);
        return false;
    }

    CLOGD("DEBUG(%s):in", __func__);

    if (m_camera_info[m_cameraMode].video.flagStart == true) {
#ifdef THREAD_PROFILE
        timeUs = 0;
        gettimeofday(&mTimeStart, NULL);
#endif
        if (exynos_v4l2_streamoff(m_camera_info[m_cameraMode].video.fd, V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) < 0) {
            CLOGE("ERR(%s):exynos_v4l2_streamoff() fail", __func__);
            return false;
        }
#ifdef THREAD_PROFILE
        gettimeofday(&mTimeStop, NULL);
        timeUs = (mTimeStop.tv_sec*1000000 + mTimeStop.tv_usec) - (mTimeStart.tv_sec*1000000 + mTimeStart.tv_usec);
        CLOGD("DEBUG(%s):time_check elapsed time=(%ld)us", __func__, timeUs);
#endif

        struct v4l2_requestbuffers req;
        req.count  = 0;
        req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
        req.memory = V4L2_MEMORY_USERPTR;

        if (exynos_v4l2_reqbufs(m_camera_info[m_cameraMode].video.fd, &req) < 0) {
            CLOGE("ERR(%s):exynos_v4l2_reqbufs() fail", __func__);
            return false;
        }

        m_camera_info[m_cameraMode].video.flagStart = false;
    }

    bool toggle = true;

    if (m_startFaceDetection((enum CAMERA_MODE)m_cameraMode, toggle) == false)
        CLOGE("ERR(%s):m_startFaceDetection(%d, %d) fail", __func__, m_cameraMode, toggle);

    return true;
}

bool ExynosCamera::flagStartVideo(void)
{
    return m_camera_info[m_cameraMode].video.flagStart;
}

int ExynosCamera::getVideoMaxBuf(void)
{
    return VIDEO_MAX_FRAME;
}

bool ExynosCamera::setVideoBuf(ExynosBuffer *buf)
{
    if (m_flagCreate == false) {
        CLOGE("ERR(%s):Not yet created fail", __func__);
        return false;
    }

    if (VIDEO_MAX_FRAME <= buf->reserved.p) {
        CLOGE("ERR(%s):index(%d) must smaller than %d", __func__, buf->reserved.p, VIDEO_MAX_FRAME);
        return false;
    }

    m_videoBuf[buf->reserved.p] = *buf;
    return true;
}

bool ExynosCamera::getVideoBuf(ExynosBuffer *buf)
{
    if (m_flagCreate == false) {
        CLOGE("ERR(%s):Not yet created fail", __func__);
        return false;
    }

    if (m_camera_info[m_cameraMode].video.flagStart == false) {
        CLOGE("ERR(%s):Not yet video started fail", __func__);
        return false;
    }

    struct v4l2_buffer v4l2_buf;
    struct v4l2_plane  planes[VIDEO_MAX_PLANES];

    v4l2_buf.m.planes = planes;
    v4l2_buf.type     = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    v4l2_buf.memory   = V4L2_CAMERA_MEMORY_TYPE;
    v4l2_buf.length   = 0;

    for (int i = 0; i < ExynosBuffer::BUFFER_PLANE_NUM_DEFAULT; i++) {
        if (m_videoBuf[0].size.extS[i] != 0)
            v4l2_buf.length++;
    }

    if (exynos_v4l2_dqbuf(m_camera_info[m_cameraMode].video.fd, &v4l2_buf) < 0) {
        CLOGE("ERR(%s):exynos_v4l2_dqbuf() fail", __func__);
        return false;
    }

    if (VIDEO_MAX_FRAME <= v4l2_buf.index) {
        CLOGE("ERR(%s):wrong index = %d", __func__, v4l2_buf.index);
        return false;
    }

    *buf = m_videoBuf[v4l2_buf.index];

    return true;
}

bool ExynosCamera::putVideoBuf(ExynosBuffer *buf)
{
    if (m_flagCreate == false) {
        CLOGE("ERR(%s):Not yet created fail", __func__);
        return false;
    }

    if (m_camera_info[m_cameraMode].video.flagStart == false) {
        /* this can happen when recording frames are returned after
         * the recording is stopped at the driver level.  we don't
         * need to return the buffers in this case and we've seen
         * cases where fimc could crash if we called qbuf and it
         * wasn't expecting it.
         */
        CLOGV("DEBUG(%s):recording not in progress, ignoring", __func__);
        return true;
    }

    struct v4l2_buffer v4l2_buf;
    struct v4l2_plane  planes[VIDEO_MAX_PLANES];

    v4l2_buf.m.planes = planes;
    v4l2_buf.type     = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    v4l2_buf.memory   = V4L2_CAMERA_MEMORY_TYPE;
    v4l2_buf.index    = buf->reserved.p;
    v4l2_buf.length   = 0;

    for (int i = 0; i < ExynosBuffer::BUFFER_PLANE_NUM_DEFAULT; i++) {
        v4l2_buf.m.planes[i].m.userptr = (unsigned long)m_videoBuf[buf->reserved.p].virt.extP[i];
        v4l2_buf.m.planes[i].length   = m_videoBuf[buf->reserved.p].size.extS[i];

        if (m_videoBuf[buf->reserved.p].size.extS[i] != 0)
            v4l2_buf.length++;
    }

    if (exynos_v4l2_qbuf(m_camera_info[m_cameraMode].video.fd, &v4l2_buf) < 0) {
        CLOGE("ERR(%s):exynos_v4l2_qbuf() fail", __func__);
        return false;
    }

    return true;
}

bool ExynosCamera::startPicture(void)
{
    CLOGD("DEBUG(%s):in", __func__);

    if (m_flagCreate == false) {
        CLOGE("ERR(%s):Not yet Created", __func__);
        return false;
    }

    if (m_camera_info[m_cameraMode].picture.flagStart == false) {
        m_camera_info[m_cameraMode].picture.width   = m_curCameraInfo[m_cameraMode]->pictureW;
        m_camera_info[m_cameraMode].picture.height  = m_curCameraInfo[m_cameraMode]->pictureH;
        m_camera_info[m_cameraMode].picture.format  = m_curCameraInfo[CAMERA_MODE_BACK]->pictureColorFormat;
        m_camera_info[m_cameraMode].picture.planes  = NUM_CAPTURE_PLANE;
        m_camera_info[m_cameraMode].picture.buffers = NUM_PICTURE_BUFFERS;
        m_camera_info[m_cameraMode].picture.memory  = V4L2_CAMERA_MEMORY_TYPE;
        m_camera_info[m_cameraMode].picture.type    = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
        m_camera_info[m_cameraMode].picture.ionClient = m_ionCameraClient;

        // for frame sync
        for (int i = 0; i < VIDEO_MAX_FRAME; i++) {
            if (m_camera_info[m_cameraMode].picture.buffer[i].virt.p != NULL ||
                m_camera_info[m_cameraMode].picture.buffer[i].phys.p != 0) {
                m_camera_info[m_cameraMode].picture.buffer[i].size.extS[m_camera_info[m_cameraMode].picture.planes - 1]
                    = ALIGN(META_DATA_SIZE, PAGE_SIZE);

                if (allocMemSinglePlane(m_ionCameraClient,
                                        &m_camera_info[m_cameraMode].picture.buffer[i],
                                        m_camera_info[m_cameraMode].picture.planes - 1,
                                        true) == false) {
                    CLOGE("ERR(%s):m_allocCameraMemorySingle() fail", __func__);
                    goto err;
                } else {
                    memset(m_camera_info[m_cameraMode].picture.buffer[i].virt.extP[m_camera_info[m_cameraMode].picture.planes - 1],
                            0, m_camera_info[m_cameraMode].picture.buffer[i].size.extS[m_camera_info[m_cameraMode].picture.planes - 1]);
                }
            }
        }
        if (m_startPicture() == false) {
            CLOGE("ERR(%s):m_startPicture() fail", __func__);
            goto err;
        }

        m_camera_info[m_cameraMode].picture.flagStart = true;
    }

    return true;

err:
    if (this->stopPicture() == false)
        CLOGE("ERR(%s):stopPicture() fail", __func__);

    return false;
}

bool ExynosCamera::m_startPicture(void)
{
    CLOGD("DEBUG(%s):in", __func__);

    int sensorId = m_getSensorId(m_cameraMode);
    sensorId = (0 << REPROCESSING_SHFIT) | ((FIMC_IS_VIDEO_SCC_NUM - FIMC_IS_VIDEO_SEN0_NUM) << VIDEO_INDEX_SHFIT) | (sensorId << 0);

    if (cam_int_s_input(&(m_camera_info[m_cameraMode].picture), sensorId) < 0) {
        CLOGE("ERR(%s):cam_int_s_input(%d) fail", __func__, sensorId);
        return false;
    }

    if (cam_int_s_fmt(&(m_camera_info[m_cameraMode].picture)) < 0) {
       CLOGE("ERR(%s):cam_int_s_fmt() fail", __func__);
        return false;
    }

    if (cam_int_reqbufs(&(m_camera_info[m_cameraMode].picture)) < 0) {
        CLOGE("ERR(%s):cam_int_reqbufs() fail", __func__);
        return false;
    }

    for (int i = 0; i < VIDEO_MAX_FRAME; i++) {
        if (m_camera_info[m_cameraMode].picture.buffer[i].virt.p != NULL ||
            m_camera_info[m_cameraMode].picture.buffer[i].phys.p != 0) {
            if (cam_int_qbuf(&m_camera_info[m_cameraMode].picture, i) < 0) {
                CLOGE("ERR(%s):cam_int_qbuf(%d) fail", __func__, i);
                return false;
            }
        }
    }

#ifdef FRONT_NO_ZSL
#else
    pictureOn();
#endif

    if (cam_int_streamon(&(m_camera_info[m_cameraMode].picture)) < 0) {
        CLOGE("ERR(%s):cam_int_streamon() fail", __func__);
        return false;
    }

    return true;
}

bool ExynosCamera::m_stopPicture(void)
{
    CLOGD("DEBUG(%s):in", __func__);

    bool ret = true;

    int index;
    camera2_shot_ext * shot_ext;

#ifdef THREAD_PROFILE
    timeUs = 0;
    gettimeofday(&mTimeStart, NULL);
#endif
    if (cam_int_streamoff(&m_camera_info[m_cameraMode].picture) < 0) {
        CLOGE("ERR(%s):exynos_v4l2_streamoff() fail", __func__);
        ret = false;
    }
#ifdef THREAD_PROFILE
    gettimeofday(&mTimeStop, NULL);
    timeUs = (mTimeStop.tv_sec*1000000 + mTimeStop.tv_usec) - (mTimeStart.tv_sec*1000000 + mTimeStart.tv_usec);
    CLOGD("DEBUG(%s):time_check elapsed time=(%ld)us", __func__, timeUs);
#endif
    m_camera_info[m_cameraMode].dummy_shot.request_scc = 0;

    if (0 < m_camera_info[m_cameraMode].picture.buffers) {
        if (cam_int_clrbufs(&m_camera_info[m_cameraMode].picture) < 0) {
            CLOGE("ERR(%s):cam_int_clrbufs() fail", __func__);
            ret = false;
        }
        m_camera_info[m_cameraMode].picture.buffers = 0;
    }

done:
    return ret;
}

bool ExynosCamera::stopPicture(void)
{
    bool ret = true;

    if (m_flagCreate == false) {
        CLOGW("WARN(%s):Not yet Created", __func__);
        return false;
    }

    CLOGD("DEBUG(%s):in", __func__);

    if (m_camera_info[m_cameraMode].picture.flagStart == true) {

        if (m_stopPicture() == false) {
            CLOGE("ERR(%s):m_stopPicture() fail", __func__);
            ret = false;
        }

        // for frame sync
        for (int i = 0; i < VIDEO_MAX_FRAME; i++) {
            freeMemSinglePlane(&m_camera_info[m_cameraMode].picture.buffer[i],
                    m_camera_info[m_cameraMode].picture.planes - 1);
        }

        m_camera_info[m_cameraMode].picture.flagStart = false;
    }

    return ret;
}

void ExynosCamera::pictureOn(void)
{
    m_camera_info[m_cameraMode].dummy_shot.request_scc = 1;

    return;
}

bool ExynosCamera::flagStartPicture(void)
{
    return m_camera_info[m_cameraMode].picture.flagStart;
}

int ExynosCamera::getPictureMaxBuf(void)
{
    return VIDEO_MAX_FRAME;
}

bool ExynosCamera::setPictureBuf(ExynosBuffer *buf)
{
    if (m_flagCreate == false) {
        CLOGE("ERR(%s):Not yet created fail", __func__);
        return false;
    }

    if (VIDEO_MAX_FRAME <= buf->reserved.p) {
        CLOGE("ERR(%s):index(%d) must smaller than %d", __func__, buf->reserved.p, VIDEO_MAX_FRAME);
        return false;
    }

    m_camera_info[m_cameraMode].picture.buffer[buf->reserved.p] = *buf;

    return true;
}

bool ExynosCamera::getPictureBuf(ExynosBuffer *buf)
{
    int index = 0;

    if (m_flagCreate == false) {
        CLOGE("ERR(%s):Not yet created fail", __func__);
        return false;
    }

    if (m_camera_info[m_cameraMode].picture.flagStart == false) {
        CLOGE("ERR(%s):Not yet picture started fail", __func__);
        return false;
    }

    index = cam_int_dqbuf(&(m_camera_info[m_cameraMode].picture));
    if (index < 0) {
        CLOGE("ERR(%s):cam_int_dqbuf() on picture fail", __func__);
        return false;
    }

    *buf = m_camera_info[m_cameraMode].picture.buffer[index];
    buf->reserved.p = index;

    struct camera2_stream *metadata = (struct camera2_stream *)buf->virt.extP[NUM_CAPTURE_PLANE - 1];
    unsigned int currentCount = metadata->rcount;

    return true;
}

bool ExynosCamera::putPictureBuf(ExynosBuffer *buf)
{
    if (m_flagCreate == false) {
        CLOGE("ERR(%s):Not yet created fail", __func__);
        return false;
    }

    if (m_camera_info[m_cameraMode].picture.flagStart == false) {
        CLOGE("ERR(%s):Not yet picture started fail", __func__);
        return false;
    }

    if (cam_int_qbuf(&(m_camera_info[m_cameraMode].picture), buf->reserved.p) < 0) {
        CLOGE("ERR(%s):cam_int_qbuf(%d) fail", __func__, buf->reserved.p);
        return false;
    }

    return true;
}

bool ExynosCamera::yuv2Jpeg(ExynosBuffer *yuvBuf,
                            ExynosBuffer *jpegBuf,
                            ExynosRect *rect)
{
    CLOGD("DEBUG(%s):in", __func__);

    unsigned char *addr;

    ExynosJpegEncoderForCamera jpegEnc;
    bool ret = false;

    unsigned int *yuvSize = yuvBuf->size.extS;

    if (jpegEnc.create()) {
        CLOGE("ERR(%s):jpegEnc.create() fail", __func__);
        goto jpeg_encode_done;
    }

    if (jpegEnc.setQuality(m_jpegQuality)) {
        CLOGE("ERR(%s):jpegEnc.setQuality() fail", __func__);
        goto jpeg_encode_done;
    }

    if (jpegEnc.setSize(rect->w, rect->h)) {
        CLOGE("ERR(%s):jpegEnc.setSize() fail", __func__);
        goto jpeg_encode_done;
    }

    if (jpegEnc.setColorFormat(rect->colorFormat)) {
        CLOGE("ERR(%s):jpegEnc.setColorFormat() fail", __func__);
        goto jpeg_encode_done;
    }

    if (jpegEnc.setJpegFormat(V4L2_PIX_FMT_JPEG_422)) {
        CLOGE("ERR(%s):jpegEnc.setJpegFormat() fail", __func__);
        goto jpeg_encode_done;
    }

    if (m_curCameraInfo[m_cameraMode]->thumbnailW != 0 && m_curCameraInfo[m_cameraMode]->thumbnailH != 0) {
        int thumbW = 0, thumbH = 0;
        mExifInfo.enableThumb = true;
        if (rect->w < 320 || rect->h < 240) {
            thumbW = 160;
            thumbH = 120;
        } else {
            thumbW = m_curCameraInfo[m_cameraMode]->thumbnailW;
            thumbH = m_curCameraInfo[m_cameraMode]->thumbnailH;
        }
        if (jpegEnc.setThumbnailSize(thumbW, thumbH)) {
            CLOGE("ERR(%s):jpegEnc.setThumbnailSize(%d, %d) fail", __func__, thumbW, thumbH);
            goto jpeg_encode_done;
        }

        if (0 < m_jpegThumbnailQuality && m_jpegThumbnailQuality <= 100) {
            if (jpegEnc.setThumbnailQuality(m_jpegThumbnailQuality)) {
                CLOGE("ERR(%s):jpegEnc.setThumbnailQuality(%d) fail", __func__, m_jpegThumbnailQuality);
                goto jpeg_encode_done;
            }
        }
    } else {
        mExifInfo.enableThumb = false;
    }

    m_setExifChangedAttribute(&mExifInfo, rect);

    if (jpegEnc.setInBuf((int *)&(yuvBuf->fd.extFd), (int *)yuvSize)) {
        CLOGE("ERR(%s):jpegEnc.setInBuf() fail", __func__);
        goto jpeg_encode_done;
    }

    if (jpegEnc.setOutBuf(jpegBuf->fd.extFd[0], jpegBuf->size.extS[0] + jpegBuf->size.extS[1] + jpegBuf->size.extS[2])) {
        CLOGE("ERR(%s):jpegEnc.setOutBuf() fail", __func__);
        goto jpeg_encode_done;
    }

    if (jpegEnc.updateConfig()) {
        CLOGE("ERR(%s):jpegEnc.updateConfig() fail", __func__);
        goto jpeg_encode_done;
    }

    if (jpegEnc.encode((int *)&jpegBuf->size.s, &mExifInfo)) {
        CLOGE("ERR(%s):jpegEnc.encode() fail", __func__);
        goto jpeg_encode_done;
    }

    ret = true;

jpeg_encode_done:

    if (ret == false) {
        CLOGD("DEBUG(%s):(%d) [yuvBuf->fd.extFd %d][yuvSize %u]", __func__, __LINE__,
            yuvBuf->fd.extFd, yuvSize);
        CLOGD("[jpegBuf->fd.extFd[0] %d][jpegBuf->size.extS[0] + jpegBuf->size.extS[1] + jpegBuf->size.extS[2] %d]",
            jpegBuf->fd.extFd[0], jpegBuf->size.extS[0] + jpegBuf->size.extS[1] + jpegBuf->size.extS[2]);
        CLOGD("[rect->w %d][rect->h %d][rect->colorFormat %d]",
            rect->w, rect->h, rect->colorFormat);
    }

    if (jpegEnc.flagCreate() == true)
        jpegEnc.destroy();

    return ret;
}

bool ExynosCamera::autoFocus(void)
{
    CLOGD("DEBUG(%s):(%d) focusMode : %d", __func__, __LINE__, m_curCameraInfo[m_cameraMode]->focusMode);

    bool ret = true;
    int currentAutofocusState = ExynosCameraActivityAutofocus::AUTOFOCUS_STATE_NONE;
    int newMgrAutofocusMode = ExynosCameraActivityAutofocus::AUTOFOCUS_MODE_BASE;
    int oldMgrAutofocusMode = ExynosCameraActivityAutofocus::AUTOFOCUS_MODE_BASE;
    bool flagAutoFocusTringger = false;

    if (m_camera_info[m_cameraMode].preview.fd < 0) {
        CLOGE("ERR(%s):Camera was closed", __func__);
        ret = false;
        goto done;
    }

    if (m_flashMgr->getNeedCaptureFlash() == true) {
        m_flashMgr->setCaptureStatus(true);

        enum ExynosCameraActivityFlash::FLASH_TRIGGER tiggerPath;
        m_flashMgr->getFlashTrigerPath(&tiggerPath);

        m_flashMgr->setFlashTrigerPath(ExynosCameraActivityFlash::FLASH_TRIGGER_TOUCH_DISPLAY);
        m_flashMgr->setFlashStep(ExynosCameraActivityFlash::FLASH_STEP_PRE_START);

        if (m_flashMgr->waitAeDone() == false)
            CLOGE("ERR(%s):waitAeDone() fail", __func__);
    }

    oldMgrAutofocusMode = m_autofocusMgr->getAutofocusMode();

    switch (m_curCameraInfo[m_cameraMode]->focusMode) {
    case FOCUS_MODE_AUTO:
        if (m_touchAFMode == true)
            newMgrAutofocusMode = ExynosCameraActivityAutofocus::AUTOFOCUS_MODE_TOUCH;
        else
            newMgrAutofocusMode = ExynosCameraActivityAutofocus::AUTOFOCUS_MODE_AUTO;
        break;
    case FOCUS_MODE_INFINITY:
        newMgrAutofocusMode = ExynosCameraActivityAutofocus::AUTOFOCUS_MODE_INFINITY;
        break;
    case FOCUS_MODE_MACRO:
        newMgrAutofocusMode = ExynosCameraActivityAutofocus::AUTOFOCUS_MODE_MACRO;
        break;
    case FOCUS_MODE_CONTINUOUS_VIDEO:
        newMgrAutofocusMode = ExynosCameraActivityAutofocus::AUTOFOCUS_MODE_CONTINUOUS_VIDEO;
        break;
    case FOCUS_MODE_CONTINUOUS_PICTURE:
        newMgrAutofocusMode = ExynosCameraActivityAutofocus::AUTOFOCUS_MODE_CONTINUOUS_PICTURE;
        break;
    case FOCUS_MODE_CONTINUOUS_PICTURE_MACRO:
        newMgrAutofocusMode = ExynosCameraActivityAutofocus::AUTOFOCUS_MODE_CONTINUOUS_PICTURE_MACRO;
        break;
    case FOCUS_MODE_TOUCH:
        newMgrAutofocusMode = ExynosCameraActivityAutofocus::AUTOFOCUS_MODE_TOUCH;
        break;
    case FOCUS_MODE_FIXED:
        newMgrAutofocusMode = ExynosCameraActivityAutofocus::AUTOFOCUS_MODE_FIXED;
        break;
    case FOCUS_MODE_EDOF:
        newMgrAutofocusMode = ExynosCameraActivityAutofocus::AUTOFOCUS_MODE_EDOF;
        break;
    default:
        CLOGE("ERR(%s):Unsupported focusMode(%d)", __func__, m_curCameraInfo[m_cameraMode]->focusMode);
        return false;
        break;
    }

    /*
    * Applications can call autoFocus(AutoFocusCallback) in this mode.
    * If the autofocus is in the middle of scanning,
    * the focus callback will return when it completes
    * If the autofocus is not scanning,
    * the focus callback will immediately return with a boolean
    * that indicates whether the focus is sharp or not.
    */

    /*
     * But, When Continuous af is running,
     * auto focus api can be triggered,
     * and then, af will be lock. (af lock)
     */
    switch (newMgrAutofocusMode) {
    case ExynosCameraActivityAutofocus::AUTOFOCUS_MODE_FIXED:
    case ExynosCameraActivityAutofocus::AUTOFOCUS_MODE_EDOF:
    case ExynosCameraActivityAutofocus::AUTOFOCUS_MODE_CONTINUOUS_VIDEO:
    case ExynosCameraActivityAutofocus::AUTOFOCUS_MODE_CONTINUOUS_PICTURE:
    case ExynosCameraActivityAutofocus::AUTOFOCUS_MODE_CONTINUOUS_PICTURE_MACRO:
        flagAutoFocusTringger = false;
        break;
    default:
        switch(oldMgrAutofocusMode) {
        case ExynosCameraActivityAutofocus::AUTOFOCUS_MODE_FIXED:
        case ExynosCameraActivityAutofocus::AUTOFOCUS_MODE_EDOF:
        case ExynosCameraActivityAutofocus::AUTOFOCUS_MODE_CONTINUOUS_VIDEO:
        case ExynosCameraActivityAutofocus::AUTOFOCUS_MODE_CONTINUOUS_PICTURE:
        case ExynosCameraActivityAutofocus::AUTOFOCUS_MODE_CONTINUOUS_PICTURE_MACRO:
            flagAutoFocusTringger = true;
            break;
        default:
            if (oldMgrAutofocusMode == newMgrAutofocusMode) {
                currentAutofocusState = m_autofocusMgr->getCurrentState();

                if (currentAutofocusState != ExynosCameraActivityAutofocus::AUTOFOCUS_STATE_SCANNING)
                    flagAutoFocusTringger = true;
                else
                    flagAutoFocusTringger = false;
            } else {
                flagAutoFocusTringger = true;
            }
            break;
        }
        break;
    }

    if (flagAutoFocusTringger == true) {
        if (m_autofocusMgr->flagAutofocusStart() == true)
            m_autofocusMgr->stopAutofocus();

        m_autofocusMgr->setAutofocusMode(newMgrAutofocusMode);

        m_autofocusMgr->startAutofocus();
    } else {
        m_autofocusMgr->setAutofocusMode(newMgrAutofocusMode);

        switch (newMgrAutofocusMode) {
        case ExynosCameraActivityAutofocus::AUTOFOCUS_MODE_FIXED:
        case ExynosCameraActivityAutofocus::AUTOFOCUS_MODE_EDOF:
            ret = false;
            goto done;
            break;
        case ExynosCameraActivityAutofocus::AUTOFOCUS_MODE_CONTINUOUS_VIDEO:
        case ExynosCameraActivityAutofocus::AUTOFOCUS_MODE_CONTINUOUS_PICTURE:
        case ExynosCameraActivityAutofocus::AUTOFOCUS_MODE_CONTINUOUS_PICTURE_MACRO:
            currentAutofocusState = m_autofocusMgr->getCurrentState();

            if (m_autofocusMgr->flagLockAutofocus() == true &&
                currentAutofocusState != ExynosCameraActivityAutofocus::AUTOFOCUS_STATE_SUCCEESS) {
                /* for make it fail */
                currentAutofocusState = ExynosCameraActivityAutofocus::AUTOFOCUS_STATE_FAIL;
            }

            if (currentAutofocusState == ExynosCameraActivityAutofocus::AUTOFOCUS_STATE_SUCCEESS) {
                ret = true;
                goto done;
            } else if (currentAutofocusState == ExynosCameraActivityAutofocus::AUTOFOCUS_STATE_FAIL) {
                ret = false;
                goto done;
            }

            break;
        default:
            break;
        }
    }

    ret = m_autofocusMgr->getAutofocusResult();

done :
    /* The focus position is locked after autoFocus call */
    switch (newMgrAutofocusMode) {
    case ExynosCameraActivityAutofocus::AUTOFOCUS_MODE_FIXED:
    case ExynosCameraActivityAutofocus::AUTOFOCUS_MODE_EDOF:
    case ExynosCameraActivityAutofocus::AUTOFOCUS_MODE_CONTINUOUS_VIDEO:
    case ExynosCameraActivityAutofocus::AUTOFOCUS_MODE_CONTINUOUS_PICTURE:
    case ExynosCameraActivityAutofocus::AUTOFOCUS_MODE_CONTINUOUS_PICTURE_MACRO:
        m_autofocusMgr->lockAutofocus();
        break;
    default:
        break;
    }

    enum ExynosCameraActivityFlash::FLASH_STEP flashStep;
    m_flashMgr->getFlashStep(&flashStep);

    if (flashStep == ExynosCameraActivityFlash::FLASH_STEP_PRE_START)
        m_flashMgr->setFlashStep(ExynosCameraActivityFlash::FLASH_STEP_PRE_DONE);

    return ret;
}

int ExynosCamera::getCAFResult(void)
{
     int ret = 0;

     /*
      * 0: fail
      * 1: success
      * 2: canceled
      * 3: focusing
      * 4: restart
      */

     static int  oldRet = 2;
     static bool flagCAFScannigStarted = false;

     switch (m_camera_info[m_cameraMode].is3aa_dm.shot.dm.aa.afMode) {
     case AA_AFMODE_CONTINUOUS_VIDEO:
     case AA_AFMODE_CONTINUOUS_PICTURE:
     /* case AA_AFMODE_CONTINUOUS_VIDEO_FACE: */
     case AA_AFMODE_CONTINUOUS_PICTURE_FACE:
         switch(m_camera_info[m_cameraMode].is3aa_dm.shot.dm.aa.afState) {
         case AA_AFSTATE_INACTIVE:
            ret = 2;
            break;
         case AA_AFSTATE_PASSIVE_SCAN:
            ret = 2;
            break;
         case AA_AFSTATE_ACTIVE_SCAN:
            ret = 3;
            break;
         case AA_AFSTATE_AF_ACQUIRED_FOCUS:
            ret = 1;
            break;
         case AA_AFSTATE_AF_FAILED_FOCUS:
            if (flagCAFScannigStarted == true)
                ret = 0;
            else
                ret = oldRet;
            break;
         default:
            ret = 2;
            break;
         }

         if (m_camera_info[m_cameraMode].is3aa_dm.shot.dm.aa.afState == 3)
             flagCAFScannigStarted = true;
         else
             flagCAFScannigStarted = false;

         oldRet = ret;
         break;
     default:
         flagCAFScannigStarted = false;

         ret = oldRet;
         break;
     }

     return ret;
}

bool ExynosCamera::cancelAutoFocus(void)
{
    CLOGD("DEBUG(%s):(%d) focusMode : %d", __func__, __LINE__, m_curCameraInfo[m_cameraMode]->focusMode);

    if (m_camera_info[m_cameraMode].preview.fd < 0) {
        CLOGE("ERR(%s):Camera was closed", __func__);
        //return false;
        // revise for factory mode
        // this return can be problem when application run cancelAutoFocus before startPreview
        return true;
    }

    //Cancels any auto-focus function in progress.
    //Whether or not auto-focus is currently in progress,
    //this function will return the focus position to the default.
    //If the camera does not support auto-focus, this is a no-op.
    m_touchAFMode = false;
    m_touchAFModeForFlash = false;

    switch (m_autofocusMgr->getAutofocusMode()) {
    /*  If applications want to resume the continuous focus, cancelAutoFocus must be called */
    case ExynosCameraActivityAutofocus::AUTOFOCUS_MODE_CONTINUOUS_VIDEO:
    case ExynosCameraActivityAutofocus::AUTOFOCUS_MODE_CONTINUOUS_PICTURE:
    case ExynosCameraActivityAutofocus::AUTOFOCUS_MODE_CONTINUOUS_PICTURE_MACRO:
        if (m_autofocusMgr->flagLockAutofocus() == true) {
            m_autofocusMgr->unlockAutofocus();

            if (m_autofocusMgr->flagAutofocusStart() == false)
                m_autofocusMgr->startAutofocus();
        }
        break;
    default:
        if (m_autofocusMgr->flagLockAutofocus() == true)
            m_autofocusMgr->unlockAutofocus();

        if (m_autofocusMgr->flagAutofocusStart() == true)
            m_autofocusMgr->stopAutofocus();

        break;
    }

    enum ExynosCameraActivityFlash::FLASH_TRIGGER tiggerPath;
    m_flashMgr->getFlashTrigerPath(&tiggerPath);

    if ((m_flashMgr->getNeedCaptureFlash() == true) && (m_flashMgr->getFlashStatus()!= AA_FLASHMODE_OFF))
        m_flashMgr->setFlashStep(ExynosCameraActivityFlash::FLASH_STEP_CANCEL);
    else
        m_flashMgr->setFlashStep(ExynosCameraActivityFlash::FLASH_STEP_OFF);
    m_flashMgr->setFlashTrigerPath(ExynosCameraActivityFlash::FLASH_TRIGGER_OFF);

    return true;
}

bool ExynosCamera::startFaceDetection(void)
{
    if (m_flagStartFaceDetection == true) {
        CLOGD("DEBUG(%s):Face detection already started..", __func__);
        return true;
    }

    /* FD-AE is always on */
    /* m_startFaceDetection((enum CAMERA_MODE)m_cameraMode, true); */

    if (m_autofocusMgr->setFaceDetection(true) == false) {
        CLOGE("ERR(%s):setFaceDetection(%d)", __func__, true);
    } else {
        /* restart CAF when FD mode changed */
        switch (m_autofocusMgr->getAutofocusMode()) {
        case ExynosCameraActivityAutofocus::AUTOFOCUS_MODE_CONTINUOUS_PICTURE:
        case ExynosCameraActivityAutofocus::AUTOFOCUS_MODE_CONTINUOUS_PICTURE_MACRO:
            if (m_autofocusMgr->flagAutofocusStart() == true &&
                m_autofocusMgr->flagLockAutofocus() == false) {
                m_autofocusMgr->stopAutofocus();
                m_autofocusMgr->startAutofocus();
            }
            break;
        default:
            break;
        }
    }

    m_flagStartFaceDetection = true;

    return true;
}

bool ExynosCamera::stopFaceDetection(void)
{
    if (m_flagStartFaceDetection == false) {
        CLOGD("DEBUG(%s):Face detection already stopped..", __func__);
        return true;
    }

    if (m_autofocusMgr->setFaceDetection(false) == false) {
        CLOGE("ERR(%s):setFaceDetection(%d)", __func__, false);
    } else {
        /* restart CAF when FD mode changed */
        switch (m_autofocusMgr->getAutofocusMode()) {
        case ExynosCameraActivityAutofocus::AUTOFOCUS_MODE_CONTINUOUS_PICTURE:
        case ExynosCameraActivityAutofocus::AUTOFOCUS_MODE_CONTINUOUS_PICTURE_MACRO:
            if (m_autofocusMgr->flagAutofocusStart() == true &&
                m_autofocusMgr->flagLockAutofocus() == false) {
                m_autofocusMgr->stopAutofocus();
                m_autofocusMgr->startAutofocus();
            }
            break;
        default:
            break;
        }
    }

    /* FD-AE is always on */
    /* m_startFaceDetection((enum CAMERA_MODE)m_cameraMode, false); */

    m_flagStartFaceDetection = false;

    return true;
}

bool ExynosCamera::flagStartFaceDetection(void)
{
    return m_flagStartFaceDetection;
}

bool ExynosCamera::startSmoothZoom(int value)
{
    if (m_defaultCameraInfo[m_cameraMode]->hwZoomSupported == false) {
        CLOGE("ERR(%s):m_defaultCameraInfo[m_cameraMode]->hwZoomSupported == false", __func__);
        return false;
    }

    return this->setZoom(value);
}

bool ExynosCamera::stopSmoothZoom(void)
{
    // TODO
    return true;
}

int ExynosCamera::getAntibanding(void)
{
    return m_curCameraInfo[m_cameraMode]->antiBanding;
}

bool ExynosCamera::getAutoExposureLock(void)
{
    return m_curCameraInfo[m_cameraMode]->autoExposureLock;
}

bool ExynosCamera::getAutoWhiteBalanceLock(void)
{
    return m_curCameraInfo[m_cameraMode]->autoWhiteBalanceLock;
}

int ExynosCamera::getColorEffect(void)
{
    return m_curCameraInfo[m_cameraMode]->effect;
}

int ExynosCamera::getDetectedFacesAreas(int num,
                                        int *id,
                                        int *score,
                                        ExynosRect *face,
                                        ExynosRect *leftEye,
                                        ExynosRect *rightEye,
                                        ExynosRect *mouth)
{
    if (m_defaultCameraInfo[m_cameraMode]->maxNumDetectedFaces == 0) {
        CLOGE("ERR(%s):maxNumDetectedFaces == 0 fail", __func__);
        return -1;
    }

    if (m_flagStartFaceDetection == false) {
        CLOGD("DEBUG(%s):m_flagStartFaceDetection == false", __func__);
        return 0;
    }

    if (m_defaultCameraInfo[m_cameraMode]->maxNumDetectedFaces < num)
        num = m_defaultCameraInfo[m_cameraMode]->maxNumDetectedFaces;

    // width   : 0 ~ previewW
    // height  : 0 ~ previewH
    // if eye, mouth is not detectable : -1, -1
    ExynosRect2 *face2     = new ExynosRect2[num];
    ExynosRect2 *leftEye2  = new ExynosRect2[num];
    ExynosRect2 *rightEye2 = new ExynosRect2[num];
    ExynosRect2 *mouth2    = new ExynosRect2[num];

    num = getDetectedFacesAreas(num, id, score, face2, leftEye2, rightEye2, mouth2);

    for (int i = 0; i < num; i++) {

        m_secRect22SecRect(&face2[i], &face[i]);
        face[i].fullW = m_curCameraInfo[m_cameraMode]->previewW;
        face[i].fullH = m_curCameraInfo[m_cameraMode]->previewH;

        m_secRect22SecRect(&leftEye2[i], &leftEye[i]);
        leftEye[i].fullW = m_curCameraInfo[m_cameraMode]->previewW;
        leftEye[i].fullH = m_curCameraInfo[m_cameraMode]->previewH;

        m_secRect22SecRect(&rightEye2[i], &rightEye[i]);
        rightEye[i].fullW = m_curCameraInfo[m_cameraMode]->previewW;
        rightEye[i].fullH = m_curCameraInfo[m_cameraMode]->previewH;

        m_secRect22SecRect(&mouth2[i], &mouth[i]);
        mouth[i].fullW = m_curCameraInfo[m_cameraMode]->previewW;
        mouth[i].fullH = m_curCameraInfo[m_cameraMode]->previewH;
    }

    delete [] face2;
    delete [] leftEye2;
    delete [] rightEye2;
    delete [] mouth2;

    return num;
}

int ExynosCamera::getDetectedFacesAreas(int num,
                                     int *id,
                                     int *score,
                                     ExynosRect2 *face,
                                     ExynosRect2 *leftEye,
                                     ExynosRect2 *rightEye,
                                     ExynosRect2 *mouth)
{
    if (m_defaultCameraInfo[m_cameraMode]->maxNumDetectedFaces == 0) {
        CLOGE("ERR(%s):maxNumDetectedFaces == 0 fail", __func__);
        return -1;
    }

    if (m_flagStartFaceDetection == false) {
        CLOGD("DEBUG(%s):m_flagStartFaceDetection == false", __func__);
        return 0;
    }

    if (m_defaultCameraInfo[m_cameraMode]->maxNumDetectedFaces < num)
        num = m_defaultCameraInfo[m_cameraMode]->maxNumDetectedFaces;

    int faces = 0;
    struct camera2_dm *dm = &m_camera_info[m_cameraMode].isp_dm.shot.dm;

    switch (dm->stats.faceDetectMode) {
    case FACEDETECT_MODE_SIMPLE:
    case FACEDETECT_MODE_FULL:
        break;
    case FACEDETECT_MODE_OFF:
    default:
        num = 0;
        break;
    }

    for (int i = 0; i < num; i++) {
        if (dm->stats.faceIds[i]) {
            switch (dm->stats.faceDetectMode) {
            case FACEDETECT_MODE_OFF:
                break;
            case FACEDETECT_MODE_SIMPLE:
                face[i].x1 = dm->stats.faceRectangles[i][0];
                face[i].y1 = dm->stats.faceRectangles[i][1];
                face[i].x2 = dm->stats.faceRectangles[i][2];
                face[i].y2 = dm->stats.faceRectangles[i][3];
                faces++;
                break;
            case FACEDETECT_MODE_FULL:
                id[i] = dm->stats.faceIds[i];

                score[i] = dm->stats.faceScores[i];

                face[i].x1 = dm->stats.faceRectangles[i][0];
                face[i].y1 = dm->stats.faceRectangles[i][1];
                face[i].x2 = dm->stats.faceRectangles[i][2];
                face[i].y2 = dm->stats.faceRectangles[i][3];

                leftEye[i].x1 = dm->stats.faceLandmarks[i][0];
                leftEye[i].y1 = dm->stats.faceLandmarks[i][1];
                leftEye[i].x2 = dm->stats.faceLandmarks[i][0] + 20;
                leftEye[i].y2 = dm->stats.faceLandmarks[i][1] + 20;

                rightEye[i].x1 = dm->stats.faceLandmarks[i][2];
                rightEye[i].y1 = dm->stats.faceLandmarks[i][3];
                rightEye[i].x2 = dm->stats.faceLandmarks[i][2] + 20;
                rightEye[i].y2 = dm->stats.faceLandmarks[i][3] + 20;

                mouth[i].x1 = -1;
                mouth[i].y1 = -1;
                mouth[i].x2 = -1;
                mouth[i].y2 = -1;

                faces++;
                break;
            default:
                break;
            }
        }
    }

    return faces;
}

int ExynosCamera::getExposureCompensation(void)
{
    return m_curCameraInfo[m_cameraMode]->exposure;
}

float ExynosCamera::getExposureCompensationStep(void)
{
    // CameraParameters.h
    // The exposure compensation step. Exposure compensation index multiply by
    // step eqals to EV. Ex: if exposure compensation index is 6 and step is
    // 0.3333, EV is -2.
    // Example value: "0.333333333" or "0.5". Read only.
    // -> But, this formula doesn't works in apps.
    return 0.5f;
}

int ExynosCamera::getFlashMode(void)
{
    return m_curCameraInfo[m_cameraMode]->flashMode;
}

bool ExynosCamera::getFocalLength(int *num, int *den)
{
    *num = m_defaultCameraInfo[m_cameraMode]->focalLengthNum;
    *den = m_defaultCameraInfo[m_cameraMode]->focalLengthDen;
    return true;
}

int ExynosCamera::getFocusAreas(ExynosRect *rects)
{
    // TODO
    return 0;
}

bool ExynosCamera::getFocusDistances(int *num, int *den)
{
    *num = 0;
    *den = 0;
    return true;
}

int ExynosCamera::getFocusMode(void)
{
    return m_curCameraInfo[m_cameraMode]->focusMode;
}

float ExynosCamera::getHorizontalViewAngle(void)
{
    //TODO
    return 51.2f;
}

int ExynosCamera::getJpegQuality(void)
{
    return m_jpegQuality;
}

int ExynosCamera::getJpegThumbnailQuality(void)
{
    return m_jpegThumbnailQuality;
}

bool ExynosCamera::getJpegThumbnailSize(int *w, int  *h)
{
    *w  = m_curCameraInfo[m_cameraMode]->thumbnailW;
    *h  = m_curCameraInfo[m_cameraMode]->thumbnailH;
    return true;
}

int ExynosCamera::getMaxExposureCompensation(void)
{
    return m_defaultCameraInfo[m_cameraMode]->maxExposure;
}

int ExynosCamera::getMaxNumDetectedFaces(void)
{
    return m_defaultCameraInfo[m_cameraMode]->maxNumDetectedFaces;
}

int ExynosCamera::getMaxNumFocusAreas(void)
{
    return m_defaultCameraInfo[m_cameraMode]->maxNumFocusAreas;
}

int ExynosCamera::getMaxNumMeteringAreas(void)
{
    return m_defaultCameraInfo[m_cameraMode]->maxNumMeteringAreas;
}

int ExynosCamera::getMaxZoom(void)
{
    return m_defaultCameraInfo[m_cameraMode]->maxZoom;
}

int ExynosCamera::getMeteringAreas(ExynosRect *rects)
{
    // TODO
    return 0;
}

int ExynosCamera::getMinExposureCompensation(void)
{
    return m_defaultCameraInfo[m_cameraMode]->minExposure;
}

bool ExynosCamera::getIspSize(int *w, int *h)
{
    if (m_cameraMode == CAMERA_MODE_BACK) {
        *w = m_curCameraInfo[CAMERA_MODE_REPROCESSING]->ispW;
        *h = m_curCameraInfo[CAMERA_MODE_REPROCESSING]->ispH;
    } else if (m_cameraMode == CAMERA_MODE_FRONT) {
        *w = m_curCameraInfo[m_cameraMode]->ispW;
        *h = m_curCameraInfo[m_cameraMode]->ispH;
    }
    return true;
}

int ExynosCamera::getPictureFormat(void)
{
    return m_curCameraInfo[m_cameraMode]->pictureColorFormat;
}

bool ExynosCamera::getPictureSize(int *w, int *h)
{
    *w = m_curCameraInfo[m_cameraMode]->pictureW;
    *h = m_curCameraInfo[m_cameraMode]->pictureH;
    return true;
}

int ExynosCamera::getPreviewFormat(void)
{
    return m_curCameraInfo[m_cameraMode]->previewColorFormat;
}

bool ExynosCamera::getPreviewFpsRange(int *min, int *max)
{
    *min = m_curCameraInfo[m_cameraMode]->fpsRange[0];
    *max = m_curCameraInfo[m_cameraMode]->fpsRange[1];
    return true;
}

bool ExynosCamera::getPreviewSize(int *w, int *h)
{
    *w = m_curCameraInfo[m_cameraMode]->previewW;
    *h = m_curCameraInfo[m_cameraMode]->previewH;
    return true;
}

bool ExynosCamera::getRecordingHint(void)
{
    return m_recordingHint;
}

#ifdef USE_VDIS
bool ExynosCamera::getVdisMode()
{
    return m_isHWVDis;
}

void ExynosCamera::setVdisMode(bool vdisMode)
{
    m_isHWVDis = vdisMode;

    return;
}
#endif
int ExynosCamera::getSceneMode(void)
{
    return m_curCameraInfo[m_cameraMode]->sceneMode;
}

int ExynosCamera::getSupportedAntibanding(void)
{
    return m_defaultCameraInfo[m_cameraMode]->antiBandingList;
}

int ExynosCamera::getSupportedColorEffects(void)
{
    return m_defaultCameraInfo[m_cameraMode]->effectList;
}

int ExynosCamera::getSupportedFlashModes(void)
{
    return m_defaultCameraInfo[m_cameraMode]->flashModeList;
}

int ExynosCamera::getSupportedFocusModes(void)
{
    return m_defaultCameraInfo[m_cameraMode]->focusModeList;
}

bool ExynosCamera::getSupportedJpegThumbnailSizes(int *w, int *h)
{
    *w = m_defaultCameraInfo[m_cameraMode]->thumbnailW;
    *h = m_defaultCameraInfo[m_cameraMode]->thumbnailH;
    return true;
}

bool ExynosCamera::getSupportedPictureSizes(int *w, int *h)
{
    *w = m_defaultCameraInfo[m_cameraMode]->pictureW;
    *h = m_defaultCameraInfo[m_cameraMode]->pictureH;
    return true;
}

bool ExynosCamera::getSupportedPreviewFpsRange(int *min, int *max)
{
    *min = m_defaultCameraInfo[m_cameraMode]->fpsRange[0];
    *max = m_defaultCameraInfo[m_cameraMode]->fpsRange[1];
    return true;
}

bool ExynosCamera::getSupportedPreviewSizes(int *w, int *h)
{
    *w = m_defaultCameraInfo[m_cameraMode]->previewW;
    *h = m_defaultCameraInfo[m_cameraMode]->previewH;
    return true;
}

int ExynosCamera::getSupportedSceneModes(void)
{
    return m_defaultCameraInfo[m_cameraMode]->sceneModeList;
}

bool ExynosCamera::getSupportedVideoSizes(int *w, int *h)
{
    *w = m_defaultCameraInfo[m_cameraMode]->videoW;
    *h = m_defaultCameraInfo[m_cameraMode]->videoH;
    return true;
}

int ExynosCamera::getSupportedWhiteBalance(void)
{
    return m_defaultCameraInfo[m_cameraMode]->whiteBalanceList;
}

float ExynosCamera::getVerticalViewAngle(void)
{
    // TODO
    return 39.4f;
}

bool ExynosCamera::getVideoStabilization(void)
{
    return m_curCameraInfo[m_cameraMode]->videoStabilization;
}

int ExynosCamera::getWhiteBalance(void)
{
    return m_curCameraInfo[m_cameraMode]->whiteBalance;
}

int ExynosCamera::getZoom(void)
{
    return m_curCameraInfo[m_cameraMode]->zoom;
}

int ExynosCamera::getMaxZoomRatio(void)
{
    return 400;
}

bool ExynosCamera::isAutoExposureLockSupported(void)
{
    return m_defaultCameraInfo[m_cameraMode]->autoExposureLockSupported;
}

bool ExynosCamera::isAutoWhiteBalanceLockSupported(void)
{
    return m_defaultCameraInfo[m_cameraMode]->autoWhiteBalanceLockSupported;
}

bool ExynosCamera::isSmoothZoomSupported(void)
{
    if (m_defaultCameraInfo[m_cameraMode]->hwZoomSupported == true)
        return true;
    else
        return false;
}

bool ExynosCamera::isVideoSnapshotSupported(void)
{
    return true;
}

bool ExynosCamera::isVideoStabilizationSupported(void)
{
    return m_defaultCameraInfo[m_cameraMode]->supportVideoStabilization;
}

bool ExynosCamera::isZoomSupported(void)
{
    return true;
}

bool ExynosCamera::setAntibanding(int value)
{
    aa_ae_antibanding_mode mode = AA_AE_ANTIBANDING_OFF;

    switch (value) {
    case ANTIBANDING_AUTO:
        mode = AA_AE_ANTIBANDING_AUTO;
        break;
    case ANTIBANDING_50HZ:
        mode = AA_AE_ANTIBANDING_AUTO_50HZ;
        break;
    case ANTIBANDING_60HZ:
        mode = AA_AE_ANTIBANDING_AUTO_60HZ;
        break;
    case ANTIBANDING_OFF:
        mode = AA_AE_ANTIBANDING_OFF;
        break;
    default:
        CLOGE("ERR(%s):Unsupported value(%d)", __func__, value);
        return false;
        break;
    }

    m_curCameraInfo[CAMERA_MODE_BACK]->antiBanding = value;
    m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.aa.aeAntibandingMode = mode;
    m_curCameraInfo[CAMERA_MODE_FRONT]->antiBanding = value;
    m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.aa.aeAntibandingMode = mode;

    return true;
}

bool ExynosCamera::setAutoExposureLock(bool toggle)
{
    enum aa_aemode aeMode;
    int preMetering;

    aeMode = m_camera_info[m_cameraMode].dummy_shot.shot.ctl.aa.aeMode;
    preMetering = m_curCameraInfo[m_cameraMode]->metering;

    if (m_curCameraInfo[m_cameraMode]->autoExposureLock == toggle)
        return true;

    if (toggle == true)
        aeMode = AA_AEMODE_LOCKED;
    else if (aeMode == AA_AEMODE_OFF || aeMode == AA_AEMODE_LOCKED) {
        switch(preMetering){
        case METERING_MODE_AVERAGE:
            aeMode = AA_AEMODE_AVERAGE;
            break;
        case METERING_MODE_CENTER:
            aeMode = AA_AEMODE_CENTER;
            break;
        case METERING_MODE_MATRIX:
            aeMode = AA_AEMODE_MATRIX;
            break;
        case METERING_MODE_SPOT:
            aeMode = AA_AEMODE_SPOT;
            break;
        default:
            CLOGE("ERR(%s):Unsupported value(%d)", __func__, preMetering);
            return false;
            break;
        }
    }

    m_curCameraInfo[CAMERA_MODE_BACK]->autoExposureLock = toggle;
    m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.aa.aeMode = aeMode;
    m_curCameraInfo[CAMERA_MODE_FRONT]->autoExposureLock = toggle;
    m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.aa.aeMode = aeMode;

    return true;
}

bool ExynosCamera::setAutoWhiteBalanceLock(bool toggle)
{
    enum aa_awbmode awbMode;

    if (m_curCameraInfo[m_cameraMode]->autoWhiteBalanceLock == toggle)
        return true;

    if (toggle == true) {
        awbMode = AA_AWBMODE_LOCKED;
        m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.aa.awbMode = awbMode;
        m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.aa.awbMode = awbMode;
    } else {
        if (this->setWhiteBalance(m_curCameraInfo[m_cameraMode]->whiteBalance) == false) {
            CLOGE("ERR(%s):setWhiteBalance(%d) fail", __func__, m_curCameraInfo[CAMERA_MODE_BACK]->whiteBalance);
            return false;
        }
    }

    m_curCameraInfo[CAMERA_MODE_BACK]->autoWhiteBalanceLock = toggle;
    m_curCameraInfo[CAMERA_MODE_FRONT]->autoWhiteBalanceLock = toggle;

    return true;
}

bool ExynosCamera::setColorEffect(int value)
{
    enum aa_effect_mode effectMode = AA_EFFECT_OFF;
    enum colorcorrection_mode mode;

    switch (value) {
    case EFFECT_NONE:
        effectMode = ::AA_EFFECT_OFF;
        mode       = ::COLORCORRECTION_MODE_FAST;
        break;
    case EFFECT_MONO:
        effectMode = ::AA_EFFECT_MONO;
        mode       = ::COLORCORRECTION_MODE_EFFECT_MONO;
        break;
    case EFFECT_NEGATIVE:
        effectMode = ::AA_EFFECT_NEGATIVE;
        mode       = ::COLORCORRECTION_MODE_EFFECT_NEGATIVE;
        break;
    case EFFECT_SEPIA:
        effectMode = ::AA_EFFECT_SEPIA;
        mode       = ::COLORCORRECTION_MODE_EFFECT_SEPIA;
        break;
    case EFFECT_AQUA:
        effectMode = ::AA_EFFECT_AQUA;
        mode       = ::COLORCORRECTION_MODE_EFFECT_AQUA;
        break;
    case EFFECT_SOLARIZE:
        effectMode = ::AA_EFFECT_SOLARIZE;
        mode       = ::COLORCORRECTION_MODE_EFFECT_SOLARIZE;
        break;
    case EFFECT_POSTERIZE:
        effectMode = ::AA_EFFECT_POSTERIZE;
        mode       = ::COLORCORRECTION_MODE_EFFECT_POSTERIZE;
        break;
    case EFFECT_WHITEBOARD:
        effectMode = ::AA_EFFECT_WHITEBOARD;
        mode       = ::COLORCORRECTION_MODE_EFFECT_WHITEBOARD;
        break;
    case EFFECT_BLACKBOARD:
        effectMode = ::AA_EFFECT_BLACKBOARD;
        mode       = ::COLORCORRECTION_MODE_EFFECT_BLACKBOARD;
        break;
    default:
        CLOGE("ERR(%s):Unsupported value(%d)", __func__, value);
        return false;
        break;
    }

    m_curCameraInfo[CAMERA_MODE_BACK]->effect = value;
    m_curCameraInfo[CAMERA_MODE_FRONT]->effect = value;

    m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.color.mode = mode;
    m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.color.mode = mode;

    return true;
}

bool ExynosCamera::setExposureCompensation(int value)
{
    m_curCameraInfo[m_cameraMode]->exposure = value;
    m_curCameraInfo[CAMERA_MODE_FRONT]->exposure = value;

    /* F/W's middle value is 5, and step is -4, -3, -2, -1, 0, 1, 2, 3, 4 */
    m_camera_info[m_cameraMode].dummy_shot.shot.ctl.aa.aeExpCompensation = 5 + value;
    m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.aa.aeExpCompensation = 5 + value;

    return true;
}

bool ExynosCamera::setFlashMode(int value)
{
    enum flash_mode flashMode;
    enum aa_ae_flashmode aeflashMode;
    enum aa_aemode aeMode;
    int flashPreMode;
    aeMode = m_camera_info[m_cameraMode].dummy_shot.shot.ctl.aa.aeMode;

    switch (value) {
    case FLASH_MODE_OFF:
        flashMode   = ::CAM2_FLASH_MODE_OFF;
        aeflashMode = ::AA_FLASHMODE_OFF;
        if (aeMode == ::AA_AEMODE_OFF || aeMode == ::AA_AEMODE_LOCKED)
            aeMode  = ::AA_AEMODE_CENTER;

        m_flashMgr->setFlashReq(ExynosCameraActivityFlash::FLASH_REQ_OFF);
        break;
    case FLASH_MODE_AUTO:
        flashMode   = ::CAM2_FLASH_MODE_SINGLE;
        //aeflashMode = ::AA_FLASHMODE_AUTO;
        aeflashMode = ::AA_FLASHMODE_CAPTURE;

        m_flashMgr->setFlashReq(ExynosCameraActivityFlash::FLASH_REQ_AUTO);
        break;
    case FLASH_MODE_ON:
        flashMode   = ::CAM2_FLASH_MODE_SINGLE;
        //aeflashMode = ::AA_FLASHMODE_ON;
        aeflashMode = ::AA_FLASHMODE_ON_ALWAYS;

        m_flashMgr->setFlashReq(ExynosCameraActivityFlash::FLASH_REQ_ON);
        break;
    case FLASH_MODE_TORCH:
        flashMode   = ::CAM2_FLASH_MODE_TORCH;
        aeflashMode = ::AA_FLASHMODE_ON_ALWAYS;

        m_flashMgr->setFlashReq(ExynosCameraActivityFlash::FLASH_REQ_TORCH);
        break;
    case FLASH_MODE_RED_EYE:
    default:
        CLOGE("ERR(%s):Unsupported value(%d)", __func__, value);
        return false;
        break;
    }

    if (m_curCameraInfo[m_cameraMode]->autoExposureLock == true)
        aeMode = ::AA_AEMODE_LOCKED;

    flashPreMode = m_curCameraInfo[m_cameraMode]->flashPreMode;
    if (flashPreMode != value) {
        if (flashPreMode != FLASH_MODE_TORCH && m_flashMgr->getFlashStatus() != AA_FLASHMODE_OFF) {
            m_flashMgr->setFlashStep(ExynosCameraActivityFlash::FLASH_STEP_CANCEL);
            m_flashMgr->setFlashTrigerPath(ExynosCameraActivityFlash::FLASH_TRIGGER_OFF);
        }
        m_curCameraInfo[m_cameraMode]->flashMode = value;
    }
    m_curCameraInfo[m_cameraMode]->flashPreMode = value;

    return true;
}

bool ExynosCamera::setFocusAreas(int num, ExynosRect* rects, int *weights)
{
    if (m_defaultCameraInfo[m_cameraMode]->maxNumFocusAreas == 0) {
        CLOGV("DEBUG(%s):maxNumFocusAreas is 0. so, ignored", __func__);
        return true;
    }

    bool ret = true;

    ExynosRect2 *rect2s = new ExynosRect2[num];
    for (int i = 0; i < num; i++)
        m_secRect2SecRect2(&rects[i], &rect2s[i]);

    ret = setFocusAreas(num, rect2s, weights);

    delete [] rect2s;

    return ret;
}

bool ExynosCamera::setFocusAreas(int num, ExynosRect2* rect2s, int *weights)
{
    if (m_defaultCameraInfo[m_cameraMode]->maxNumFocusAreas == 0) {
        CLOGV("DEBUG(%s):maxNumFocusAreas is 0. so, ignored", __func__);
        return true;
    }

    int number = num;
    struct camera2_shot *shot = &m_camera_info[m_cameraMode].dummy_shot.shot;

    if (m_defaultCameraInfo[m_cameraMode]->maxNumFocusAreas < num)
        num = m_defaultCameraInfo[m_cameraMode]->maxNumFocusAreas;

    if (m_flagCreate == true) {
        if ((num == 0) ||
            (num == 1 &&
             rect2s[0].x1 == 0 &&
             rect2s[0].y1 == 0 &&
             rect2s[0].x2 == 0 &&
             rect2s[0].y2 == 0))  {
            /* TODO : driver decide focus areas : center */
            ExynosRect2 newRect2(0, 0, 0, 0);

            m_autofocusMgr->setFocusAreas(newRect2, 1000);

            m_touchAFMode = false;
            m_touchAFModeForFlash = false;
        } else {
            ExynosRect2 newRect2;

            for (int i = 0; i < 1; i++) {
            /* for (int i = 0; i < num; i++) { */
                /*
                newRect2 = m_AndroidArea2HWArea(&rect2s[i],
                                                m_curCameraInfo[m_cameraMode]->pictureW,
                                                m_curCameraInfo[m_cameraMode]->pictureH);
                */
                newRect2 = m_AndroidArea2HWArea(&rect2s[i]);

                m_autofocusMgr->setFocusAreas(newRect2, weights[i]);
            }

            m_touchAFMode = true;
            m_touchAFModeForFlash = true;
        }
    }

    if (m_flagCreate == true) {
        if ((number == 0) ||
            (number == 1 &&
             (rect2s == NULL ||
             (rect2s[0].x1 == 0 &&
              rect2s[0].y1 == 0 &&
              rect2s[0].x2 == 0 &&
              rect2s[0].y2 == 0))) ||
             (number == 2 &&
             (rect2s == NULL ||
             (rect2s[0].x1 == 0 &&
              rect2s[0].y1 == 0 &&
              rect2s[0].x2 == 0 &&
              rect2s[0].y2 == 0))))  {
            m_touchAFModeForFlash = false;
        }
    }

    return true;
}

bool ExynosCamera::setFocusMode(int value)
{
    int newMgrAutofocusMode = 0;
    int oldMgrAutofocusMode = m_autofocusMgr->getAutofocusMode();

    CLOGD("DEBUG(%s):(%d) value : %d / oldMgrAutofocusMode : %d", __func__, __LINE__, value, oldMgrAutofocusMode);

    switch (value) {
    case FOCUS_MODE_INFINITY:
        newMgrAutofocusMode = ExynosCameraActivityAutofocus::AUTOFOCUS_MODE_INFINITY;
        break;
    case FOCUS_MODE_FIXED:
        newMgrAutofocusMode = ExynosCameraActivityAutofocus::AUTOFOCUS_MODE_FIXED;
        break;
    case FOCUS_MODE_EDOF:
        newMgrAutofocusMode = ExynosCameraActivityAutofocus::AUTOFOCUS_MODE_EDOF;
        break;
    case FOCUS_MODE_CONTINUOUS_VIDEO:
        newMgrAutofocusMode = ExynosCameraActivityAutofocus::AUTOFOCUS_MODE_CONTINUOUS_VIDEO;
        break;
    case FOCUS_MODE_CONTINUOUS_PICTURE:
        newMgrAutofocusMode = ExynosCameraActivityAutofocus::AUTOFOCUS_MODE_CONTINUOUS_PICTURE;
        break;
    case FOCUS_MODE_CONTINUOUS_PICTURE_MACRO:
        newMgrAutofocusMode = ExynosCameraActivityAutofocus::AUTOFOCUS_MODE_CONTINUOUS_PICTURE_MACRO;
        break;
    default:
        break;
    }

    if (oldMgrAutofocusMode != newMgrAutofocusMode) {
        if (m_autofocusMgr->flagLockAutofocus() == true)
            m_autofocusMgr->unlockAutofocus();
    }

    if (newMgrAutofocusMode) { /* Continuous autofocus, infinity, fixed ... */
        /*
         * If applications want to resume the continuous focus,
         * cancelAutoFocus must be called.
         * Restarting the preview will not resume the continuous autofocus.
         * To stop continuous focus, applications should change the focus mode to other modes.
         */
        bool flagRestartAutofocus = false;

        if (oldMgrAutofocusMode == newMgrAutofocusMode) {
            if (m_autofocusMgr->flagAutofocusStart() == false &&
                m_autofocusMgr->flagLockAutofocus() == false)
                flagRestartAutofocus = true;
            else
                flagRestartAutofocus = false;
        } else {
            flagRestartAutofocus = true;
        }

        if (flagRestartAutofocus == true &&
            m_autofocusMgr->flagAutofocusStart() == true)
            m_autofocusMgr->stopAutofocus();

        if (oldMgrAutofocusMode != newMgrAutofocusMode)
            m_autofocusMgr->setAutofocusMode(newMgrAutofocusMode);

        if (flagRestartAutofocus == true)
            m_autofocusMgr->startAutofocus();

    } else { /* single autofocus */
        switch (oldMgrAutofocusMode) {
        case ExynosCameraActivityAutofocus::AUTOFOCUS_MODE_INFINITY:
        case ExynosCameraActivityAutofocus::AUTOFOCUS_MODE_FIXED:
        case ExynosCameraActivityAutofocus::AUTOFOCUS_MODE_EDOF:
        case ExynosCameraActivityAutofocus::AUTOFOCUS_MODE_CONTINUOUS_VIDEO:
        case ExynosCameraActivityAutofocus::AUTOFOCUS_MODE_CONTINUOUS_PICTURE:
        case ExynosCameraActivityAutofocus::AUTOFOCUS_MODE_CONTINUOUS_PICTURE_MACRO:
            if (m_autofocusMgr->flagAutofocusStart() == true)
                m_autofocusMgr->stopAutofocus();

            break;
        default:
            break;
        }
    }

    m_curCameraInfo[m_cameraMode]->focusMode = value;

    return true;
}

bool ExynosCamera::setGpsAltitude(const char *gpsAltitude)
{
    double conveted_altitude = 0;

    if (gpsAltitude == NULL)
        m_curCameraInfo[m_cameraMode]->gpsAltitude = 0;
    else {
        conveted_altitude = atof(gpsAltitude);
        m_curCameraInfo[m_cameraMode]->gpsAltitude = conveted_altitude;
    }

    return true;
}

bool ExynosCamera::setGpsLatitude(const char *gpsLatitude)
{
    double conveted_latitude = 0;

    if (gpsLatitude == NULL)
        m_curCameraInfo[m_cameraMode]->gpsLatitude = 0;
    else {
        conveted_latitude = atof(gpsLatitude);
        m_curCameraInfo[m_cameraMode]->gpsLatitude = conveted_latitude;
    }

    return true;
}

bool ExynosCamera::setGpsLongitude(const char *gpsLongitude)
{
    double conveted_longitude = 0;

    if (gpsLongitude == NULL)
        m_curCameraInfo[m_cameraMode]->gpsLongitude = 0;
    else {
        conveted_longitude = atof(gpsLongitude);
        m_curCameraInfo[m_cameraMode]->gpsLongitude = conveted_longitude;
    }

    return true;
}

bool ExynosCamera::setGpsProcessingMethod(const char *gpsProcessingMethod)
{
    memset(mExifInfo.gps_processing_method, 0, sizeof(mExifInfo.gps_processing_method));

    if (gpsProcessingMethod != NULL) {
        size_t len = strlen(gpsProcessingMethod);
        if (len > sizeof(mExifInfo.gps_processing_method)) {
            len = sizeof(mExifInfo.gps_processing_method);
        }
        memcpy(mExifInfo.gps_processing_method, gpsProcessingMethod, len);
    }

    return true;
}

bool ExynosCamera::setGpsTimeStamp(const char *gpsTimestamp)
{
    if (gpsTimestamp == NULL)
        m_curCameraInfo[m_cameraMode]->gpsTimestamp = 0;
    else
        m_curCameraInfo[m_cameraMode]->gpsTimestamp = atol(gpsTimestamp);

    return true;
}

bool ExynosCamera::setJpegQuality(int quality)
{
    if (quality < JPEG_QUALITY_MIN || JPEG_QUALITY_MAX < quality) {
        CLOGE("ERR(%s):Invalid quality (%d)", __func__, quality);
        return false;
    }

    m_jpegQuality = quality;

    return true;
}

bool ExynosCamera::setJpegThumbnailQuality(int quality)
{
    if (quality < JPEG_QUALITY_MIN || JPEG_QUALITY_MAX < quality) {
        CLOGE("ERR(%s):Invalid quality (%d)", __func__, quality);
        return false;
    }

    m_jpegThumbnailQuality = quality;

    return true;
}

bool ExynosCamera::setJpegThumbnailSize(int w, int h)
{
    m_curCameraInfo[m_cameraMode]->thumbnailW = w;
    m_curCameraInfo[m_cameraMode]->thumbnailH = h;
    return true;
}

bool ExynosCamera::cancelMeteringAreas()
{
    if (m_curCameraInfo[m_cameraMode]->metering == METERING_MODE_SPOT) {
        if (setMeteringMode(m_oldMeteringMode) == false) {
            CLOGE("ERR(%s):setMeteringMode(%d) fail", __func__, m_oldMeteringMode);
            return false;
        }
    }

    return true;
}

bool ExynosCamera::setMeteringAreas(int num, ExynosRect *rects, int *weights)
{
    if (m_defaultCameraInfo[m_cameraMode]->maxNumMeteringAreas == 0) {
        CLOGV("DEBUG(%s):maxNumMeteringAreas is 0. so, ignored", __func__);
        return true;
    }

    bool ret = true;

    ExynosRect2 *rect2s = new ExynosRect2[num];

    for (int i = 0; i < num; i++)
        m_secRect2SecRect2(&rects[i], &rect2s[i]);

    ret = setMeteringAreas(num, rect2s, weights);

    delete [] rect2s;

    return true;
}

bool ExynosCamera::setMeteringAreas(int num, ExynosRect2 *rect2s, int *weights)
{
    if (m_defaultCameraInfo[m_cameraMode]->maxNumMeteringAreas == 0) {
        CLOGV("DEBUG(%s):maxNumMeteringAreas is 0. so, ignored", __func__);
        return true;
    }

    if (m_curCameraInfo[m_cameraMode]->autoExposureLock == true) {
        CLOGD("DEBUG(%s):autoExposureLock == true", __func__);
        return true;
    }

    if (m_defaultCameraInfo[m_cameraMode]->maxNumMeteringAreas < num)
        num = m_defaultCameraInfo[m_cameraMode]->maxNumMeteringAreas;

    if (m_flagCreate == true) {
        struct camera2_shot *shot = &m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot;

        if (    num == 1
            && rect2s[0].x1 == 0
            && rect2s[0].y1 == 0
            && rect2s[0].x2 == 0
            && rect2s[0].y2 == 0)  {
            if (this->setMeteringMode(METERING_MODE_CENTER) == false) {
                CLOGE("%s(%d):setMeteringMode(METERING_MODE_CENTER) fail", __func__, __LINE__);
                return false;
            }
        } else {
#if 1
            if (setMeteringMode(METERING_MODE_SPOT) == false) {
                CLOGE("%s(%d):setMeteringMode(METERING_MODE_SPOT) fail", __func__, __LINE__);
                return false;
            }
#else
            ExynosRect2 newRect2;

            for (int i = 0; i < 1; i++) {
            /* for (int i = 0; i < num; i++) { */
                /*
                newRect2 = m_AndroidArea2HWArea(&rect2s[i],
                                                m_curCameraInfo[m_cameraMode]->pictureW,
                                                m_curCameraInfo[m_cameraMode]->pictureH);
                */
                newRect2 = m_AndroidArea2HWArea(&rect2s[i]);

                shot->ctl.aa.aeRegions[0] = newRect2.x1;
                shot->ctl.aa.aeRegions[1] = newRect2.y1;
                shot->ctl.aa.aeRegions[2] = newRect2.x2;
                shot->ctl.aa.aeRegions[3] = newRect2.y2;
                shot->ctl.aa.aeRegions[4] = weights[i];
            }
#endif
        }
    }

    return true;
}

bool ExynosCamera::setPictureFormat(int colorFormat)
{
    m_curCameraInfo[m_cameraMode]->pictureColorFormat = colorFormat;

#if defined(CLOG_NDEBUG) && CLOG_NDEBUG == 0
    m_printFormat(m_curCameraInfo[m_cameraMode]->pictureColorFormat, "PictureFormat");
#endif
    return true;
}

bool ExynosCamera::setPictureSize(int w, int h)
{
    int pictureW, pictureH;
    int newX, newY, newW, newH;

    if (m_curCameraInfo[m_cameraMode]->fpsRange[0] <= 30000 &&
        m_curCameraInfo[m_cameraMode]->fpsRange[1] <= 30000) {
        // HACK : Camera cannot support zoom. So, we must make max size picture w, h
        pictureW = m_defaultCameraInfo[m_cameraMode]->pictureW;
        pictureH = m_defaultCameraInfo[m_cameraMode]->pictureH;

        /* sensor full size */
        m_curCameraInfo[m_cameraMode]->pictureW = pictureW;
        m_curCameraInfo[m_cameraMode]->pictureH = pictureH;
        m_curCameraInfo[CAMERA_MODE_REPROCESSING]->pictureW = pictureW;
        m_curCameraInfo[CAMERA_MODE_REPROCESSING]->pictureH = pictureH;
        m_curCameraInfo[CAMERA_MODE_REPROCESSING]->ispW = pictureW;
        m_curCameraInfo[CAMERA_MODE_REPROCESSING]->ispH = pictureH;
    }

    return true;
}

bool ExynosCamera::setPreviewFormat(int colorFormat)
{
    m_curCameraInfo[m_cameraMode]->previewColorFormat = colorFormat;

    /* TODO: Impl all format */
    switch (colorFormat) {
    case V4L2_PIX_FMT_NV12 :
    case V4L2_PIX_FMT_NV12T :
    case V4L2_PIX_FMT_NV21 :
    case V4L2_PIX_FMT_NV12M :
    case V4L2_PIX_FMT_NV21M :
    case V4L2_PIX_FMT_NV12MT_16X16 :
        m_curCameraInfo[m_cameraMode]->previewBufPlane = 3;
        break;
    case V4L2_PIX_FMT_YUV420 :
    case V4L2_PIX_FMT_YVU420 :
    case V4L2_PIX_FMT_YUV420M :
    case V4L2_PIX_FMT_YVU420M :
        m_curCameraInfo[m_cameraMode]->previewBufPlane = 4;
        break;
    default:
        CLOGE("ERR(%s):Unknown color format(%x)", __func__, colorFormat);
        break;
    }

#if defined(CLOG_NDEBUG) && CLOG_NDEBUG == 0
    m_printFormat(m_curCameraInfo[m_cameraMode]->previewColorFormat, "PreviewtFormat");
#endif

    return true;
}

bool ExynosCamera::setPreviewFpsRange(int min, int max)
{
    int minFps = 0;
    int maxFps = 0;

    if (min == 0 || max == 0) {
        CLOGE("ERR(%s):Invalid fps value(%d, %d)", __func__, min, max);
        return false;
    }
    minFps = min / 1000;
    maxFps = max / 1000;

    if (minFps < FRAME_RATE_AUTO || FRAME_RATE_MAX < maxFps)
        CLOGE("ERR(%s):Invalid fps range(%d, %d)", __func__, min, max);

    if (max < min)
        CLOGE("ERR(%s):Invalid fps range(%d, %d)", __func__, min, max);

    if (m_flagCreate == true) {
        m_curCameraInfo[m_cameraMode]->fpsRange[0] = min;
        m_curCameraInfo[m_cameraMode]->fpsRange[1] = max;

        m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.aa.aeTargetFpsRange[0] = minFps;
        m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.aa.aeTargetFpsRange[1] = maxFps;
        m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.sensor.frameDuration = (1000 * 1000 * 1000) / maxFps;

        m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.aa.aeTargetFpsRange[0] = minFps;
        m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.aa.aeTargetFpsRange[1] = maxFps;
        m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.sensor.frameDuration = (1000 * 1000 * 1000) / maxFps;
    }

    return true;
}

bool ExynosCamera::setPreviewSize(int w, int h)
{
    int ispW, ispH;
    int newX = 0, newY = 0, newW = 0, newH = 0;

    m_curCameraInfo[m_cameraMode]->previewW = w;
    m_curCameraInfo[m_cameraMode]->previewH = h;

    ispW = m_defaultCameraInfo[m_cameraMode]->ispW;
    ispH = m_defaultCameraInfo[m_cameraMode]->ispH;

    if (getCropRectAlign(ispW,  ispH,
                         w,     h,
                         &newX, &newY,
                         &newW, &newH,
                         CAMERA_MAGIC_ALIGN, (CAMERA_MAGIC_ALIGN >> 1),
                         0) == false) {
        CLOGE("ERR(%s):getCropRectAlign(%d, %d, %d, %d) fail",
            __func__, ispW, ispW, w, h);
        return false;
    }

    /* BDS out size for preview instance */
    m_curCameraInfo[m_cameraMode]->ispW = newW;
    m_curCameraInfo[m_cameraMode]->ispH = newH;

    return true;
}

bool ExynosCamera::setRecordingHint(bool hint)
{
    // TODO : fixed fps?
    /* DIS is only possible recording hint is true. */
    m_recordingHint = hint;

    m_autofocusMgr->setRecordingHint(hint);
    m_flashMgr->setRecordingHint(hint);

    return true;
}

#ifdef FD_ROTATION
bool ExynosCamera::setDeviceOrientation(int orientation)
{
    if (orientation < 0 || orientation % 90 != 0) {
        CLOGE("ERR(%s):Invalid orientation (%d)", __func__, orientation);
        return false;
    }

    m_deviceOrientation = orientation;

    /* fd orientation need to be calibrated, according to f/w spec */
    int fdOrientation = orientation;

    switch (orientation) {
    case 180:
        fdOrientation = 270;
        break;
    case 90:
        fdOrientation = 180;
        break;
    case 0:
        fdOrientation = 90;
        break;
    case 270:
        fdOrientation = 0;
        break;
    default:
        CLOGE("ERR(%s):Unsupported value(%d)", __func__, orientation);
        fdOrientation = 0;
        break;
    }

    CLOGD("(%s): orientation(%d), fdOrientation(%d)", __func__, orientation, fdOrientation);

    m_camera_info[m_cameraMode].dummy_shot.shot.uctl.scalerUd.orientation = fdOrientation;

    return true;
}

int ExynosCamera::getDeviceOrientation(void)
{
    return m_deviceOrientation;
}
#endif

bool ExynosCamera::setRotation(int rotation)
{
     if (rotation < 0) {
         CLOGE("ERR(%s):Invalid rotation (%d)", __func__, rotation);
         return false;
     }
     m_curCameraInfo[m_cameraMode]->rotation = rotation;

     return true;
}

int ExynosCamera::getRotation(void)
{
    return m_curCameraInfo[m_cameraMode]->rotation;
}

bool ExynosCamera::setSceneMode(int value)
{
    enum aa_mode mode = AA_CONTROL_AUTO;
    enum aa_scene_mode sceneMode = AA_SCENE_MODE_FACE_PRIORITY;

    switch (value) {
    case SCENE_MODE_AUTO:
        mode = AA_CONTROL_AUTO;
        sceneMode = AA_SCENE_MODE_FACE_PRIORITY;

        if (m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.aa.aeMode == AA_AEMODE_OFF)
            m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.aa.aeMode = AA_AEMODE_CENTER;

        m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.aa.sceneMode = AA_SCENE_MODE_UNSUPPORTED;
        m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.aa.isoMode = AA_ISOMODE_AUTO;
        m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.aa.isoValue = 0;
        m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.noise.mode = PROCESSING_MODE_OFF;
        m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.noise.strength = 0;
        m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.edge.mode = PROCESSING_MODE_OFF;
        m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.edge.strength = 0;

        /* m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.color.saturation = 3; // "3" is default. */

        if (m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.aa.aeMode == AA_AEMODE_OFF)
            m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.aa.aeMode = AA_AEMODE_CENTER;

        m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.aa.sceneMode = AA_SCENE_MODE_UNSUPPORTED;
        m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.aa.isoMode = AA_ISOMODE_AUTO;
        m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.aa.isoValue = 0;
        m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.noise.mode = PROCESSING_MODE_OFF;
        m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.noise.strength = 0;
        m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.edge.mode = PROCESSING_MODE_OFF;
        m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.edge.strength = 0;

        /* m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.color.saturation = 3; // "3" is default. */

        break;
    case SCENE_MODE_PORTRAIT:
        mode = AA_CONTROL_USE_SCENE_MODE;
        sceneMode = AA_SCENE_MODE_PORTRAIT;
        break;
    case SCENE_MODE_LANDSCAPE:
        mode = AA_CONTROL_USE_SCENE_MODE;
        sceneMode = AA_SCENE_MODE_LANDSCAPE;
        break;
    case SCENE_MODE_NIGHT:
        mode = AA_CONTROL_USE_SCENE_MODE;
        sceneMode = AA_SCENE_MODE_NIGHT;
        /* AE_LOCK is prohibited */
        if (m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.aa.aeMode == AA_AEMODE_OFF
         || m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.aa.aeMode == AA_AEMODE_LOCKED)
            m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.aa.aeMode = AA_AEMODE_CENTER;

        m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.aa.awbMode = AA_AWBMODE_WB_AUTO;
        m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.aa.isoMode = AA_ISOMODE_AUTO;
        m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.aa.isoValue = 0;

        if (30000 < m_curCameraInfo[m_cameraMode]->fpsRange[1]) {
            int fps = m_curCameraInfo[m_cameraMode]->fpsRange[1] / 1000;
            m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.aa.aeTargetFpsRange[0] = fps / 4;
            m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.aa.aeTargetFpsRange[1] = fps;
            m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.sensor.frameDuration = (1000 * 1000 * 1000) / fps;
        } else {
            m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.aa.aeTargetFpsRange[0] = 8;
            m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.aa.aeTargetFpsRange[1] = 30;
            m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.sensor.frameDuration = (1000 * 1000 * 1000) / 30;
        }

        m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.noise.mode = PROCESSING_MODE_FAST;
        m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.noise.strength = 0;
        m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.edge.mode = PROCESSING_MODE_FAST;
        m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.edge.strength = 0;

        /* m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.color.saturation = 3; // "3" is default. */

        /* AE_LOCK is prohibited */
        if (m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.aa.aeMode == AA_AEMODE_OFF
         || m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.aa.aeMode == AA_AEMODE_LOCKED)
            m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.aa.aeMode = AA_AEMODE_CENTER;
        m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.aa.awbMode = AA_AWBMODE_WB_AUTO;
        m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.aa.isoMode = AA_ISOMODE_AUTO;
        m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.aa.isoValue = 0;
        m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.aa.aeTargetFpsRange[0] = 8;
        m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.aa.aeTargetFpsRange[1] = 30;
        m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.sensor.frameDuration = (1000 * 1000 * 1000) / 30;

        m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.noise.mode = PROCESSING_MODE_FAST;
        m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.noise.strength = 0;
        m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.edge.mode = PROCESSING_MODE_FAST;
        m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.edge.strength = 0;

        /* m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.color.saturation = 3; // "3" is default. */

        /* TODO: FLASH */
        /* TODO: METERING */
        break;
    case SCENE_MODE_BEACH:
        mode = AA_CONTROL_USE_SCENE_MODE;
        sceneMode = AA_SCENE_MODE_BEACH;
        break;
    case SCENE_MODE_SNOW:
        mode = AA_CONTROL_USE_SCENE_MODE;
        sceneMode = AA_SCENE_MODE_SNOW;
        break;
    case SCENE_MODE_SUNSET:
        mode = AA_CONTROL_USE_SCENE_MODE;
        sceneMode = AA_SCENE_MODE_SUNSET;

        if (m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.aa.aeMode == AA_AEMODE_OFF)
            m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.aa.aeMode = AA_AEMODE_CENTER;

        m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.aa.awbMode = AA_AWBMODE_WB_DAYLIGHT;
        m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.aa.isoMode = AA_ISOMODE_AUTO;
        m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.aa.isoValue = 0;

        if (30000 < m_curCameraInfo[m_cameraMode]->fpsRange[1]) {
            int fps = m_curCameraInfo[m_cameraMode]->fpsRange[1] / 1000;
            m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.aa.aeTargetFpsRange[0] = fps / 2;
            m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.aa.aeTargetFpsRange[1] = fps;
            m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.sensor.frameDuration = (1000 * 1000 * 1000) / fps;
        } else {
            m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.aa.aeTargetFpsRange[0] = 15;
            m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.aa.aeTargetFpsRange[1] = 30;
            m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.sensor.frameDuration = (1000 * 1000 * 1000) / 30;
        }

        m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.noise.mode = PROCESSING_MODE_FAST;
        m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.noise.strength = 0;
        m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.edge.mode = PROCESSING_MODE_FAST;
        m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.edge.strength = 0;

        /* m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.color.saturation = 3; // "3" is default. */

        if (m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.aa.aeMode == AA_AEMODE_OFF)
            m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.aa.aeMode = AA_AEMODE_CENTER;

        m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.aa.awbMode = AA_AWBMODE_WB_DAYLIGHT;
        m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.aa.isoMode = AA_ISOMODE_AUTO;
        m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.aa.isoValue = 0;
        m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.aa.aeTargetFpsRange[0] = 15;
        m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.aa.aeTargetFpsRange[1] = 30;
        m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.sensor.frameDuration = (1000 * 1000 * 1000) / 30;

        m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.noise.mode = PROCESSING_MODE_FAST;
        m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.noise.strength = 0;
        m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.edge.mode = PROCESSING_MODE_FAST;
        m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.edge.strength = 0;

        /* m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.color.saturation = 3; // "3" is default. */

        /* TODO: FLASH */
        /* TODO: METERING */
        break;
    case SCENE_MODE_FIREWORKS:
        mode = AA_CONTROL_USE_SCENE_MODE;
        sceneMode = AA_SCENE_MODE_FIREWORKS;
        break;
    case SCENE_MODE_SPORTS:
        mode = AA_CONTROL_USE_SCENE_MODE;
        sceneMode = AA_SCENE_MODE_SPORTS;
        break;
    case SCENE_MODE_PARTY:
        mode = AA_CONTROL_USE_SCENE_MODE;
        sceneMode = AA_SCENE_MODE_PARTY;

        if (m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.aa.aeMode == AA_AEMODE_OFF)
            m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.aa.aeMode = AA_AEMODE_CENTER;

        m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.aa.awbMode = AA_AWBMODE_WB_AUTO;
        m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.aa.isoMode = AA_ISOMODE_MANUAL;
        m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.aa.isoValue = 200;

        if (30000 < m_curCameraInfo[m_cameraMode]->fpsRange[1]) {
            int fps = m_curCameraInfo[m_cameraMode]->fpsRange[1] / 1000;
            m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.aa.aeTargetFpsRange[0] = fps / 2;
            m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.aa.aeTargetFpsRange[1] = fps;
            m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.sensor.frameDuration = (1000 * 1000 * 1000) / fps;
        } else {
            m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.aa.aeTargetFpsRange[0] = 15;
            m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.aa.aeTargetFpsRange[1] = 30;
            m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.sensor.frameDuration = (1000 * 1000 * 1000) / 30;
        }

        m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.noise.mode = PROCESSING_MODE_FAST;
        m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.noise.strength = 0;
        m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.edge.mode = PROCESSING_MODE_FAST;
        m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.edge.strength = 0;

        /* m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.color.saturation = 4; // "4" is default + 1. */

        if (m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.aa.aeMode == AA_AEMODE_OFF)
            m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.aa.aeMode = AA_AEMODE_CENTER;

        m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.aa.awbMode = AA_AWBMODE_WB_AUTO;
        m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.aa.isoMode = AA_ISOMODE_MANUAL;
        m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.aa.isoValue = 200;
        m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.aa.aeTargetFpsRange[0] = 15;
        m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.aa.aeTargetFpsRange[1] = 30;
        m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.sensor.frameDuration = (1000 * 1000 * 1000) / 30;

        m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.noise.mode = PROCESSING_MODE_FAST;
        m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.noise.strength = 0;
        m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.edge.mode = PROCESSING_MODE_FAST;
        m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.edge.strength = 0;

        /* m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.color.saturation = 4; // "4" is default + 1. */

        /* TODO: FLASH */
        /* TODO: METERING */
        break;
    case SCENE_MODE_CANDLELIGHT:
        mode = AA_CONTROL_USE_SCENE_MODE;
        sceneMode = AA_SCENE_MODE_CANDLELIGHT;
        break;
    case SCENE_MODE_STEADYPHOTO:
        mode = AA_CONTROL_USE_SCENE_MODE;
        sceneMode = AA_SCENE_MODE_STEADYPHOTO;
        break;
    case SCENE_MODE_ACTION:
        mode = AA_CONTROL_USE_SCENE_MODE;
        sceneMode = AA_SCENE_MODE_ACTION;

        if (m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.aa.aeMode == AA_AEMODE_OFF)
            m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.aa.aeMode = AA_AEMODE_CENTER;

        m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.aa.awbMode = AA_AWBMODE_WB_AUTO;
        m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.aa.isoMode = AA_ISOMODE_AUTO;
        m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.aa.isoValue = 0;

        if (30000 < m_curCameraInfo[m_cameraMode]->fpsRange[1]) {
            int fps = m_curCameraInfo[m_cameraMode]->fpsRange[1] / 1000;
            m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.aa.aeTargetFpsRange[0] = fps;
            m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.aa.aeTargetFpsRange[1] = fps;
            m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.sensor.frameDuration = (1000 * 1000 * 1000) / fps;
        } else {
            m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.aa.aeTargetFpsRange[0] = 30;
            m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.aa.aeTargetFpsRange[1] = 30;
            m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.sensor.frameDuration = (1000 * 1000 * 1000) / 30;
        }

        m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.noise.mode = PROCESSING_MODE_FAST;
        m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.noise.strength = 0;
        m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.edge.mode = PROCESSING_MODE_FAST;
        m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.edge.strength = 0;

        /* m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.color.saturation = 3; // "3" is default. */

        if (m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.aa.aeMode == AA_AEMODE_OFF)
            m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.aa.aeMode = AA_AEMODE_CENTER;

        m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.aa.awbMode = AA_AWBMODE_WB_AUTO;
        m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.aa.isoMode = AA_ISOMODE_AUTO;
        m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.aa.isoValue = 0;
        m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.aa.aeTargetFpsRange[0] = 30;
        m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.aa.aeTargetFpsRange[1] = 30;
        m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.sensor.frameDuration = (1000 * 1000 * 1000) / 30;

        m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.noise.mode = PROCESSING_MODE_FAST;
        m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.noise.strength = 0;
        m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.edge.mode = PROCESSING_MODE_FAST;
        m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.edge.strength = 0;

        /* m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.color.saturation = 3; // "3" is default. */

        /* TODO: FLASH */
        /* TODO: METERING */
        break;
    case SCENE_MODE_NIGHT_PORTRAIT:
        mode = AA_CONTROL_USE_SCENE_MODE;
        sceneMode = AA_SCENE_MODE_NIGHT_PORTRAIT;
        break;
    case SCENE_MODE_THEATRE:
        mode = AA_CONTROL_USE_SCENE_MODE;
        sceneMode = AA_SCENE_MODE_THEATRE;
        break;
    default:
        CLOGE("ERR(%s):Unsupported value(%d)", __func__, value);
        return false;
        break;
    }

    m_curCameraInfo[CAMERA_MODE_BACK]->sceneMode = value;
    m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.aa.mode = mode;
    m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.aa.sceneMode = sceneMode;

    /* adjust fpsRange from the aa target range */
    m_curCameraInfo[CAMERA_MODE_BACK]->fpsRange[0]
        = m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.aa.aeTargetFpsRange[0] * 1000;
    m_curCameraInfo[CAMERA_MODE_BACK]->fpsRange[1]
        = m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.aa.aeTargetFpsRange[1] * 1000;

    m_curCameraInfo[CAMERA_MODE_FRONT]->sceneMode = value;
    m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.aa.mode = mode;
    m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.aa.sceneMode = sceneMode;

    m_curCameraInfo[CAMERA_MODE_FRONT]->fpsRange[0]
        = m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.aa.aeTargetFpsRange[0] * 1000;
    m_curCameraInfo[CAMERA_MODE_FRONT]->fpsRange[1]
        = m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.aa.aeTargetFpsRange[1] * 1000;

    return true;
}
#ifdef USE_VDIS
bool ExynosCamera::startVdisCapture()
{
    int sensorId;

    if (m_camera_info[m_cameraMode].vdisc.flagStart == false) {
        CLOGE("startVdisCapture:: w(%d) h(%d)", m_VDisSrcW, m_VDisSrcH);

        m_camera_info[m_cameraMode].vdisc.width  = m_VDisSrcW;
        m_camera_info[m_cameraMode].vdisc.height = m_VDisSrcH;
        m_camera_info[m_cameraMode].vdisc.format = V4L2_PIX_FMT_YUYV;
        m_camera_info[m_cameraMode].vdisc.planes = 2;
        m_camera_info[m_cameraMode].vdisc.memory = V4L2_CAMERA_MEMORY_TYPE;
        m_camera_info[m_cameraMode].vdisc.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
        m_camera_info[m_cameraMode].vdisc.ionClient = m_ionCameraClient;
        m_camera_info[m_cameraMode].vdisc.buffers = m_VDisSrcBufNum;

        for (int i = 0; i < m_VDisSrcBufNum; i++) {
            if (m_isHWVDis == true) {
                m_camera_info[m_cameraMode].vdisc.buffer[i].size.extS[0] = ALIGN(m_camera_info[m_cameraMode].vdisc.width *
                    m_camera_info[m_cameraMode].vdisc.height * 2, PAGE_SIZE);

                if (allocMemSinglePlane(m_ionCameraClient,
                    &m_camera_info[m_cameraMode].vdisc.buffer[i], 0, false) == false) {
                    CLOGE("ERR(%s):m_allocCameraMemorySingleCache() fail", __func__);
                    return false;
                }

                m_camera_info[m_cameraMode].vdisc.buffer[i].size.extS[1]
                    = ALIGN(META_DATA_SIZE, PAGE_SIZE);

                if (allocMemSinglePlane(m_ionCameraClient,
                    &m_camera_info[m_cameraMode].vdisc.buffer[i], 1, true) == false) {
                    CLOGE("ERR(%s):m_allocCameraMemorySingleCache() fail", __func__);
                    return false;
                }
            } else {
                m_camera_info[m_cameraMode].vdisc.buffer[i].size.extS[0] = ALIGN(m_camera_info[m_cameraMode].vdisc.width *
                    m_camera_info[m_cameraMode].vdisc.height * 2, PAGE_SIZE);

                if (allocMemSinglePlaneCache(m_ionCameraClient,
                    &m_camera_info[m_cameraMode].vdisc.buffer[i], 0, true) == false) {
                    CLOGE("ERR(%s):m_allocCameraMemorySingleCache() fail", __func__);
                    return false;
                }

                m_camera_info[m_cameraMode].vdisc.buffer[i].size.extS[1]
                    = ALIGN(META_DATA_SIZE, PAGE_SIZE);

                if (allocMemSinglePlane(m_ionCameraClient,
                    &m_camera_info[m_cameraMode].vdisc.buffer[i], 1, true) == false) {
                    CLOGE("ERR(%s):m_allocCameraMemorySingleCache() fail", __func__);
                    return false;
                }
            }
        }

        sensorId = m_getSensorId(m_cameraMode);

        if (cam_int_s_input(&(m_camera_info[m_cameraMode].vdisc), sensorId) < 0) {
            CLOGE("ERR(%s):cam_int_s_input(%d) fail", __func__, sensorId);
            return false;
        }

        if (cam_int_s_fmt(&m_camera_info[m_cameraMode].vdisc) < 0) {
            CLOGE("ERR(%s):cam_int_s_fmt() fail", __func__);
            return false;
        }

        if (cam_int_reqbufs(&m_camera_info[m_cameraMode].vdisc) < 0) {
            CLOGE("ERR(%s):cam_int_reqbufs() fail", __func__);
            return false;
        }

        for (int i = 0; i < m_VDisSrcBufNum; i++) {
            if (cam_int_qbuf(&m_camera_info[m_cameraMode].vdisc, i) < 0) {
                CLOGE("ERR(%s):cam_int_qbuf(%d) fail", __func__, i);
                return false;
            }
        }
        m_camera_info[m_cameraMode].vdisc.flagStart = true;
    }
    return true;
}

bool ExynosCamera::startVdisOutput()
{
    int sensorId;

    if (m_camera_info[m_cameraMode].vdiso.flagStart == false) {
        m_camera_info[m_cameraMode].dummy_shot.request_scp = 1;
        m_camera_info[m_cameraMode].dummy_shot.request_dis = 1;

        if (m_isHWVDis) {
            CLOGE("startVdisOutput:: w(%d) h(%d)", m_VDisSrcW, m_VDisSrcH);
            m_camera_info[m_cameraMode].vdiso.width  = m_VDisSrcW;
            m_camera_info[m_cameraMode].vdiso.height = m_VDisSrcH;
        } else {
            CLOGE("startVdisOutput:: w(%d) h(%d)", m_VDisDstW, m_VDisDstH);
            m_camera_info[m_cameraMode].vdiso.width  = m_VDisDstW;
            m_camera_info[m_cameraMode].vdiso.height = m_VDisDstH;
        }
        m_camera_info[m_cameraMode].vdiso.format = V4L2_PIX_FMT_YUYV;
        m_camera_info[m_cameraMode].vdiso.planes = 2;
        m_camera_info[m_cameraMode].vdiso.memory = V4L2_CAMERA_MEMORY_TYPE;
        m_camera_info[m_cameraMode].vdiso.type   = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
        m_camera_info[m_cameraMode].vdiso.ionClient = m_ionCameraClient;
        m_camera_info[m_cameraMode].vdiso.buffers = m_VDisDstBufNum;

        sensorId = m_getSensorId(m_cameraMode);

        if (cam_int_s_input(&(m_camera_info[m_cameraMode].vdiso), sensorId) < 0) {
            CLOGE("ERR(%s):cam_int_s_input(%d) fail", __func__, sensorId);
            return false;
        }

        if (cam_int_s_fmt(&m_camera_info[m_cameraMode].vdiso) < 0) {
            CLOGE("ERR(%s):cam_int_s_fmt() fail", __func__);
            return false;
        }

        if (cam_int_reqbufs(&m_camera_info[m_cameraMode].vdiso) < 0) {
            CLOGE("ERR(%s):cam_int_reqbufs() fail", __func__);
            return false;
        }

        for (int i = 0; i < m_VDisSrcBufNum; i++) {
            if (m_isHWVDis == true) {
                m_camera_info[m_cameraMode].vdiso.buffer[i].size.extS[0] = ALIGN(m_camera_info[m_cameraMode].vdiso.width *
                    m_camera_info[m_cameraMode].vdiso.height * 2, PAGE_SIZE);

                if (allocMemSinglePlane(m_ionCameraClient,
                    &m_camera_info[m_cameraMode].vdiso.buffer[i], 0, false) == false) {
                    CLOGE("ERR(%s):m_allocCameraMemorySingleCache() fail", __func__);
                    return false;
                }

                m_camera_info[m_cameraMode].vdiso.buffer[i].size.extS[1]
                    = ALIGN(META_DATA_SIZE, PAGE_SIZE);

                if (allocMemSinglePlane(m_ionCameraClient,
                    &m_camera_info[m_cameraMode].vdiso.buffer[i], 1, true) == false) {
                    CLOGE("ERR(%s):m_allocCameraMemorySingleCache() fail", __func__);
                    return false;
                }
            } else {
                m_camera_info[m_cameraMode].vdiso.buffer[i].size.extS[0] = ALIGN(m_camera_info[m_cameraMode].vdiso.width *
                    m_camera_info[m_cameraMode].vdiso.height * 2, PAGE_SIZE);

                if (allocMemSinglePlaneCache(m_ionCameraClient,
                    &m_camera_info[m_cameraMode].vdiso.buffer[i], 0, true) == false) {
                    CLOGE("ERR(%s):m_allocCameraMemorySingleCache() fail", __func__);
                    return false;
                }

                m_camera_info[m_cameraMode].vdiso.buffer[i].size.extS[1]
                    = ALIGN(META_DATA_SIZE, PAGE_SIZE);

                if (allocMemSinglePlane(m_ionCameraClient,
                    &m_camera_info[m_cameraMode].vdiso.buffer[i], 1, true) == false) {
                    CLOGE("ERR(%s):m_allocCameraMemorySingleCache() fail", __func__);
                    return false;
                }
            }
        }

        m_camera_info[m_cameraMode].vdiso.flagStart = true;

    }

    return true;
}

bool ExynosCamera::stopVdisCapture()
{
    ExynosBuffer srcBuf;
    int rcount, fcount;

    if (m_camera_info[m_cameraMode].vdisc.flagStart == true) {
        CLOGE("DEBUG(%s)", __func__);
        m_camera_info[m_cameraMode].vdisc.flagStart = false;

        m_camera_info[m_cameraMode].dummy_shot.request_dis = 0;
        cam_int_streamoff(&m_camera_info[m_cameraMode].vdisc);

        m_camera_info[m_cameraMode].vdisc.buffers = 0;
        if (cam_int_reqbufs(&m_camera_info[m_cameraMode].vdisc) < 0) {
            CLOGE("ERR(%s):cam_int_reqbufs() fail", __func__);
            return false;
        }
        for (int i = 0; i < m_VDisSrcBufNum; i++) {
            freeMemSinglePlane(&m_camera_info[m_cameraMode].vdisc.buffer[i],
                                          m_camera_info[m_cameraMode].vdisc.planes - 1);
            freeMemSinglePlane(&m_camera_info[m_cameraMode].vdisc.buffer[i], 0);
        }
    }

    return true;
}

bool ExynosCamera::stopVdisOutput()
{
    ExynosBuffer dstBuf;
    int rcount, fcount;

    if (m_camera_info[m_cameraMode].vdiso.flagStart == true) {
        CLOGE("DEBUG(%s)", __func__);
        m_camera_info[m_cameraMode].vdiso.flagStart = false;
        cam_int_streamoff(&m_camera_info[m_cameraMode].vdiso);

        m_camera_info[m_cameraMode].vdiso.buffers = 0;
        if (cam_int_reqbufs(&m_camera_info[m_cameraMode].vdiso) < 0) {
            CLOGE("ERR(%s):cam_int_reqbufs() fail", __func__);
            return false;
        }

        for (int i = 0; i < m_VDisDstBufNum; i++) {
            freeMemSinglePlane(&m_camera_info[m_cameraMode].vdiso.buffer[i],
                                          m_camera_info[m_cameraMode].vdiso.planes - 1);
            freeMemSinglePlane(&m_camera_info[m_cameraMode].vdiso.buffer[i], 0);
        }
    }

    return true;
}

bool ExynosCamera::flagStartVdisCapture(void)
{
CLOGE("(%s) (%d) (%d)", __func__, __LINE__, m_camera_info[m_cameraMode].vdisc.flagStart);
    return m_camera_info[m_cameraMode].vdisc.flagStart;
}

bool ExynosCamera::flagStartVdisOutput(void)
{
CLOGE("(%s) (%d) (%d)", __func__, __LINE__, m_camera_info[m_cameraMode].vdiso.flagStart);
    return m_camera_info[m_cameraMode].vdiso.flagStart;
}
#endif

bool ExynosCamera::setVideoStabilization(bool toggle)
{
    m_curCameraInfo[m_cameraMode]->videoStabilization = toggle;

    if (m_flagCreate == true) {
        if (m_curCameraInfo[m_cameraMode]->applyVideoStabilization != toggle) {
#ifdef USE_DIS
            int dis = (toggle == true) ? 1 : 0;
            int dnr = (toggle == true) ? 1 : 0;

            m_camera_info[m_cameraMode].dummy_shot.shot.ctl.aa.videoStabilizationMode = dis;
            m_camera_info[m_cameraMode].dummy_shot.dis_bypass = !dis;
            m_camera_info[m_cameraMode].dummy_shot.dnr_bypass = !dnr;
#endif
            m_curCameraInfo[m_cameraMode]->applyVideoStabilization = toggle;
        }

        /*HACK: ODC is enabled only when DIS is enabled because
         *      ODC needs croping outer image and DIS does it.*/
        if (setODC(toggle) == false)
            CLOGE("ERR(%s):setODC() fail", __func__);
    }

    return true;
}

bool ExynosCamera::setWhiteBalance(int value)
{
    enum aa_awbmode awbMode = AA_AWBMODE_WB_AUTO;

    switch (value) {
    case WHITE_BALANCE_AUTO:
        awbMode = AA_AWBMODE_WB_AUTO;
        break;
    case WHITE_BALANCE_INCANDESCENT:
        awbMode = AA_AWBMODE_WB_INCANDESCENT;
        break;
    case WHITE_BALANCE_FLUORESCENT:
        awbMode = AA_AWBMODE_WB_FLUORESCENT;
        break;
    case WHITE_BALANCE_DAYLIGHT:
        awbMode = AA_AWBMODE_WB_DAYLIGHT;
        break;
    case WHITE_BALANCE_CLOUDY_DAYLIGHT:
        awbMode = AA_AWBMODE_WB_CLOUDY_DAYLIGHT;
        break;
    case WHITE_BALANCE_WARM_FLUORESCENT:
        awbMode = AA_AWBMODE_WB_WARM_FLUORESCENT;
        break;
    case WHITE_BALANCE_TWILIGHT:
        awbMode = AA_AWBMODE_WB_TWILIGHT;
        break;
    case WHITE_BALANCE_SHADE:
        awbMode = AA_AWBMODE_WB_SHADE;
        break;
    default:
        CLOGE("ERR(%s):Unsupported value(%d)", __func__, value);
        return false;
        break;
    }

    m_curCameraInfo[CAMERA_MODE_BACK]->whiteBalance = value;
    m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.aa.awbMode = awbMode;
    if (m_flashMgr->getNeedFlash() == true) {
        m_flashMgr->setFlashWhiteBalance(awbMode);
    }

    m_curCameraInfo[CAMERA_MODE_FRONT]->whiteBalance = value;
    m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.aa.awbMode = awbMode;

    return true;
}

bool ExynosCamera::setZoom(int value)
{
    if (value < ZOOM_LEVEL_0 || ZOOM_LEVEL_MAX <= value) {
        CLOGE("ERR(%s):Invalid value (%d)", __func__, value);
        return false;
    }

    if (m_curCameraInfo[m_cameraMode]->zoom != value) {
        m_curCameraInfo[m_cameraMode]->zoom = value;
        int srcW, srcH, dstW, dstH;
        srcW = m_curCameraInfo[m_cameraMode]->pictureW;
        srcH = m_curCameraInfo[m_cameraMode]->pictureH;
        dstW = m_curCameraInfo[m_cameraMode]->ispW;
        dstH = m_curCameraInfo[m_cameraMode]->ispH;
#ifdef SCALABLE_SENSOR
        if (getScalableSensorStart() == true) {
            getScalableSensorSizeOnPreview(&srcW, &srcH);
            CLOGD("DEBUG(%s):runtime zoom setting(%d/%d/%d/%d/%d)", __func__, value, srcW, srcH, dstW, dstH);
        }
#endif
        if (m_setZoom(m_curCameraInfo[m_cameraMode]->zoom,
                      srcW, srcH, dstW, dstH,
                      (void *)&m_camera_info[m_cameraMode].dummy_shot) == false) {
            CLOGE("ERR(%s):m_setZoom() fail", __func__);
            return false;
        }
    }

    return true;
}

bool ExynosCamera::m_setSetfile(int cameraMode)
{
    const int wideYUVRange = 0;
    const int narrowYUVRange = 1;

    int flagYUVRange = wideYUVRange;

    if (m_recordingHint == true) {
        m_camera_info[cameraMode].dummy_shot.setfile = ISS_SUB_SCENARIO_VIDEO;
        if (cameraMode != CAMERA_MODE_REPROCESSING)
            flagYUVRange = narrowYUVRange;
    } else {
        if (cameraMode == CAMERA_MODE_FRONT && 0 < m_curCameraInfo[cameraMode]->vtMode) {
            switch (m_curCameraInfo[cameraMode]->vtMode) {
            case 1:
                m_camera_info[cameraMode].dummy_shot.setfile = ISS_SUB_SCENARIO_FRONT_VT1;
                break;
            case 2:
            default:
                m_camera_info[cameraMode].dummy_shot.setfile = ISS_SUB_SCENARIO_FRONT_VT2;
                break;
            }
        } else {
            if (cameraMode == CAMERA_MODE_REPROCESSING)
                m_camera_info[cameraMode].dummy_shot.setfile = ISS_SUB_SCENARIO_STILL_CAPTURE;
            else
                m_camera_info[cameraMode].dummy_shot.setfile = ISS_SUB_SCENARIO_STILL_PREVIEW;
        }
    }

    m_camera_info[cameraMode].dummy_shot.setfile &= (0x0000ffff);
    m_camera_info[cameraMode].dummy_shot.setfile |= (flagYUVRange << 16);

    CLOGD("DEBUG(%s):(%d) cameraMode(%d) setfile index(%d) YUV range(%d)",
        __func__,
        __LINE__,
        cameraMode,
        m_camera_info[cameraMode].dummy_shot.setfile & 0x0000ffff,
        flagYUVRange);

    return true;
}

bool ExynosCamera::m_setZoom(int zoom, int srcW, int srcH, int dstW, int dstH, void *ptr)
{
    m_currentZoom = zoom;

    int real_zoom = 0;

    if (m_defaultCameraInfo[m_cameraMode]->hwZoomSupported == true)
        real_zoom = 0; // just adjust ratio, not digital zoom.
    else
        real_zoom = zoom; // adjust ratio, digital zoom

    int newX = 0;
    int newY = 0;
    int newW = 0;
    int newH = 0;

    if (getCropRectAlign(srcW,  srcH,
                         dstW,  dstH,
                         &newX, &newY,
                         &newW, &newH,
                         CAMERA_MAGIC_ALIGN, 2,
                         real_zoom) == false) {
        CLOGE("ERR(%s):getCropRect(%d, %d, %d, %d) fail",
            __func__, srcW,  srcH, dstW,  dstH);
        return false;
    }

    newX = ALIGN(newX, 2);
    newY = ALIGN(newY, 2);

    CLOGW("(%s): size(%d, %d, %d, %d), level(%d)",
        __func__, newX, newY, newW, newH, real_zoom);

    struct camera2_shot_ext *shot_ext = (struct camera2_shot_ext *)ptr;

    shot_ext->shot.ctl.scaler.cropRegion[0] = newX;
    shot_ext->shot.ctl.scaler.cropRegion[1] = newY;
    shot_ext->shot.ctl.scaler.cropRegion[2] = newW;
    shot_ext->shot.ctl.scaler.cropRegion[3] = newH;

    return true;
}

void ExynosCamera::m_setExifFixedAttribute(void)
{
    char property[PROPERTY_VALUE_MAX];

    memset(&mExifInfo, 0, sizeof(mExifInfo));

    //2 0th IFD TIFF Tags
    //3 Maker
    strncpy((char *)mExifInfo.maker, EXIF_DEF_MAKER,
                sizeof(mExifInfo.maker) - 1);
    mExifInfo.maker[sizeof(EXIF_DEF_MAKER) - 1] = '\0';

    //3 Model
    property_get("ro.product.model", property, EXIF_DEF_MODEL);
    strncpy((char *)mExifInfo.model, property,
                sizeof(mExifInfo.model) - 1);
    mExifInfo.model[sizeof(mExifInfo.model) - 1] = '\0';
    //3 Software
    property_get("ro.build.PDA", property, EXIF_DEF_SOFTWARE);
    strncpy((char *)mExifInfo.software, property,
                sizeof(mExifInfo.software) - 1);
    mExifInfo.software[sizeof(mExifInfo.software) - 1] = '\0';

    //3 YCbCr Positioning
    mExifInfo.ycbcr_positioning = EXIF_DEF_YCBCR_POSITIONING;

    //2 0th IFD Exif Private Tags
    //3 Exposure Program
    mExifInfo.exposure_program = EXIF_DEF_EXPOSURE_PROGRAM;
    //3 Exif Version
    memcpy(mExifInfo.exif_version, EXIF_DEF_EXIF_VERSION, sizeof(mExifInfo.exif_version));
    //3 Aperture
    mExifInfo.aperture.num = m_curCameraInfo[m_cameraMode]->apertureNum;
    mExifInfo.aperture.den = m_curCameraInfo[m_cameraMode]->apertureDen;
    //3 F Number
    mExifInfo.fnumber.num = m_curCameraInfo[m_cameraMode]->fNumberNum;
    mExifInfo.fnumber.den = m_curCameraInfo[m_cameraMode]->fNumberDen;
    //3 Maximum lens aperture
    mExifInfo.max_aperture.num = m_defaultCameraInfo[m_cameraMode]->apertureNum;
    mExifInfo.max_aperture.den = m_defaultCameraInfo[m_cameraMode]->apertureDen;
    //3 Lens Focal Length
    mExifInfo.focal_length.num = m_curCameraInfo[m_cameraMode]->focalLengthNum;
    mExifInfo.focal_length.den = m_curCameraInfo[m_cameraMode]->focalLengthDen;
    //3 Maker note
    if (mExifInfo.maker_note)
        delete mExifInfo.maker_note;
    mExifInfo.maker_note_size = sizeof(struct camera2_udm);
    mExifInfo.maker_note = new unsigned char[mExifInfo.maker_note_size];
    memset((void *)mExifInfo.maker_note, 0, mExifInfo.maker_note_size);
    //3 User Comments
    strncpy((char *)mExifInfo.user_comment, EXIF_DEF_USERCOMMENTS,
                sizeof(mExifInfo.user_comment) - 1);
    //3 Color Space information
    mExifInfo.color_space = EXIF_DEF_COLOR_SPACE;
    // 3 interoperability
    mExifInfo.interoperability_index = EXIF_DEF_INTEROPERABILITY;
    //3 Exposure Mode
    mExifInfo.exposure_mode = EXIF_DEF_EXPOSURE_MODE;

    //2 0th IFD GPS Info Tags
    unsigned char gps_version[4] = { 0x02, 0x02, 0x00, 0x00 };
    memcpy(mExifInfo.gps_version_id, gps_version, sizeof(gps_version));

    //2 1th IFD TIFF Tags
    mExifInfo.compression_scheme = EXIF_DEF_COMPRESSION;
    mExifInfo.x_resolution.num = EXIF_DEF_RESOLUTION_NUM;
    mExifInfo.x_resolution.den = EXIF_DEF_RESOLUTION_DEN;
    mExifInfo.y_resolution.num = EXIF_DEF_RESOLUTION_NUM;
    mExifInfo.y_resolution.den = EXIF_DEF_RESOLUTION_DEN;
    mExifInfo.resolution_unit = EXIF_DEF_RESOLUTION_UNIT;
}

void ExynosCamera::m_setExifChangedAttribute(exif_attribute_t *exifInfo, ExynosRect *rect)
{
    camera2_dm *dm = &(m_camera_info[m_cameraMode].is3aa_dm.shot.dm);
    camera2_udm *udm = &(m_camera_info[m_cameraMode].is3aa_dm.shot.udm);

    if (m_cameraMode == CAMERA_MODE_BACK)
        udm = &(m_camera_info[CAMERA_MODE_REPROCESSING].is3aa_dm.shot.udm);

    // 2 0th IFD TIFF Tags
    // 3 Width
    exifInfo->width = rect->w;
    // 3 Height
    exifInfo->height = rect->h;

    // 3 Orientation
    switch (m_curCameraInfo[m_cameraMode]->rotation) {
    case 90:
        exifInfo->orientation = EXIF_ORIENTATION_90;
        break;
    case 180:
        exifInfo->orientation = EXIF_ORIENTATION_180;
        break;
    case 270:
        exifInfo->orientation = EXIF_ORIENTATION_270;
        break;
    case 0:
    default:
        exifInfo->orientation = EXIF_ORIENTATION_UP;
        break;
    }

    //3 Maker note
    /* back-up udm info for exif's maker note */
    memcpy((void *)exifInfo->maker_note, (void *)udm, exifInfo->maker_note_size);

    // 3 Date time
    time_t rawtime;
    struct tm *timeinfo;
    time(&rawtime);

    timeinfo = localtime(&rawtime);

    strftime((char *)exifInfo->date_time, 20, "%Y:%m:%d %H:%M:%S", timeinfo);

    // 2 0th IFD Exif Private Tags
    bool flagSLSIAlgorithm = true;
    /*
     * CML's Algorithm internal.vendorSpecific2
     * vendorSpecific2[100]       : 0:slsi
     * vendorSpecific2[101]       : cml exposure
     * vendorSpecific2[102]       : cml iso(gain)
     * vendorSpecific2[103] / 256 : cml Bv
     */

    if (udm->internal.vendorSpecific2[100] == 1)
        flagSLSIAlgorithm = false;

    //3 ISO Speed Rating
    exifInfo->iso_speed_rating = dm->aa.isoValue;
    /* if (flagSLSIAlgorithm == false) */
        exifInfo->iso_speed_rating = udm->internal.vendorSpecific2[102];

    if (udm->ae.vendorSpecific[0] == 0xAEAEAEAE) {
        /* value by meta data */
        // 3 Exposure Time
        exifInfo->exposure_time.num = 1;
        exifInfo->exposure_time.den = (uint32_t)udm->ae.vendorSpecific[64];

        // 3 Shutter Speed
        exifInfo->shutter_speed.num = (uint32_t)(ROUND_OFF_HALF(((double)(udm->ae.vendorSpecific[69] / 256.f) * EXIF_DEF_APEX_DEN), 0));
        exifInfo->shutter_speed.den = EXIF_DEF_APEX_DEN;

        // 3 Brightness
        int temp = udm->ae.vendorSpecific[68];
        if ((int)udm->ae.vendorSpecific[68] < 0)
            temp = -temp;

        exifInfo->brightness.num = (int32_t)(ROUND_OFF_HALF(((double)(temp / 256.f) * EXIF_DEF_APEX_DEN), 0));
        if ((int)udm->ae.vendorSpecific[68] < 0)
            exifInfo->brightness.num = -exifInfo->brightness.num;

        exifInfo->brightness.den = EXIF_DEF_APEX_DEN;

        CLOGD("[%s] (%d) udm->ae.vendorSpecific[69](%d) udm->ae.vendorSpecific[68](%d)",
            __func__,
            __LINE__,
            udm->ae.vendorSpecific[69],
            udm->ae.vendorSpecific[68]);
    } else {
        /* calculated by exposure time */
        // 3 Exposure Time
        int shutterSpeed = (dm->sensor.exposureTime / 1000);
        /* if (flagSLSIAlgorithm == false) */
            shutterSpeed = udm->internal.vendorSpecific2[101];

        if (shutterSpeed <= 0) {
            CLOGW("[WARN] %s : exposureTime is invalid value (%lld) - need to check",
                __FUNCTION__, dm->sensor.exposureTime);
            shutterSpeed = 30000;
        }

        exifInfo->exposure_time.num = 1;
        exifInfo->exposure_time.den = (uint32_t)((double)1000000 / shutterSpeed);

        uint32_t av, tv, bv, sv, ev;
        av = APEX_FNUM_TO_APERTURE((double)exifInfo->fnumber.num / exifInfo->fnumber.den);
        tv = APEX_EXPOSURE_TO_SHUTTER((double)exifInfo->exposure_time.num / exifInfo->exposure_time.den);
        sv = APEX_ISO_TO_FILMSENSITIVITY(exifInfo->iso_speed_rating);
        bv = av + tv - sv;
        if (flagSLSIAlgorithm == false)
            bv = udm->internal.vendorSpecific2[103] / 256;
        ev = av + tv;

        // 3 Shutter Speed
        exifInfo->shutter_speed.num = tv * EXIF_DEF_APEX_DEN;
        exifInfo->shutter_speed.den = EXIF_DEF_APEX_DEN;
        // 3 Brightness
        exifInfo->brightness.num = bv*EXIF_DEF_APEX_DEN;
        exifInfo->brightness.den = EXIF_DEF_APEX_DEN;

        CLOGD("[%s] (%d) shutterSpeed(%d) av(%d) tv(%d) sv(%d)",
            __func__,
            __LINE__,
            shutterSpeed, av, tv, sv);
    }

    CLOGD("[%s] (%d) exposure_time(%d/%d) shutter_speed(%d/%d) brightness(%d/%d)",
        __func__,
        __LINE__,
        exifInfo->exposure_time.num, exifInfo->exposure_time.den,
        exifInfo->shutter_speed.num, exifInfo->shutter_speed.den,
        exifInfo->brightness.   num, exifInfo->brightness.   den);

    // 3 Exposure Bias
    exifInfo->exposure_bias.num = (int32_t)getExposureCompensation() * 5;
    exifInfo->exposure_bias.den = 10;
    // 3 Metering Mode
    switch (m_curCameraInfo[m_cameraMode]->metering) {
    case METERING_MODE_CENTER:
        exifInfo->metering_mode = EXIF_METERING_CENTER;
        break;
    case METERING_MODE_MATRIX:
        exifInfo->metering_mode = EXIF_METERING_AVERAGE;
        break;
    case METERING_MODE_SPOT:
        exifInfo->metering_mode = EXIF_METERING_SPOT;
        break;
    case METERING_MODE_AVERAGE:
    default:
        exifInfo->metering_mode = EXIF_METERING_AVERAGE;
        break;
    }

    // 3 Flash
    int flash = 0;
    if (m_flashMgr->getNeedFlash() == true)
        flash = 1;

    if (m_curCameraInfo[m_cameraMode]->flashMode == FLASH_MODE_OFF || flash == 0)
        exifInfo->flash = 0;
    else
        exifInfo->flash = flash;

    // 3 White Balance
    if (m_curCameraInfo[m_cameraMode]->whiteBalance == WHITE_BALANCE_AUTO)
        exifInfo->white_balance = EXIF_WB_AUTO;
    else
        exifInfo->white_balance = EXIF_WB_MANUAL;

    // 3 Focal Length in 35mm length
    exifInfo->focal_length_in_35mm_length = m_curCameraInfo[m_cameraMode]->focalLengthIn35mmLength;

    // 3 Scene Capture Type
    switch (m_curCameraInfo[m_cameraMode]->sceneMode) {
    case SCENE_MODE_PORTRAIT:
        exifInfo->scene_capture_type = EXIF_SCENE_PORTRAIT;
        break;
    case SCENE_MODE_LANDSCAPE:
        exifInfo->scene_capture_type = EXIF_SCENE_LANDSCAPE;
        break;
    case SCENE_MODE_NIGHT:
        exifInfo->scene_capture_type = EXIF_SCENE_NIGHT;
        break;
    default:
        exifInfo->scene_capture_type = EXIF_SCENE_STANDARD;
        break;
    }

    // 3 Image Unique ID
    struct v4l2_ext_controls ctrls;
    struct v4l2_ext_control ctrl;
    int uniqueId = 0;
    char uniqueIdBuf[32] = {0,};

    memset(&ctrls, 0, sizeof(struct v4l2_ext_controls));
    memset(&ctrl, 0, sizeof(struct v4l2_ext_control));

    ctrls.ctrl_class = V4L2_CTRL_CLASS_CAMERA;
    ctrls.count = 1;
    ctrls.controls = &ctrl;
    ctrl.id = V4L2_CID_CAM_SENSOR_FW_VER;
    ctrl.string = uniqueIdBuf;

    if (m_cameraMode == CAMERA_MODE_BACK){
        if (exynos_v4l2_g_ext_ctrl(m_camera_info[m_cameraMode].isp.fd, &ctrls) < 0) {
            CLOGE("ERR(%s):exynos_v4l2_g_ctrl(V4L2_CID_CAM_SENSOR_FW_VER) fail", __func__);
            memset((void *)mExifInfo.unique_id, 0, sizeof(mExifInfo.unique_id));
        } else {
            memcpy(mExifInfo.unique_id, uniqueIdBuf, sizeof(mExifInfo.unique_id));
        }
    } else if (m_cameraMode == CAMERA_MODE_FRONT) {
        memcpy(mExifInfo.unique_id, "SLSI_S5K6B2", sizeof(mExifInfo.unique_id));
    }

    // 2 0th IFD GPS Info Tags
    if (m_curCameraInfo[m_cameraMode]->gpsLatitude != 0 && m_curCameraInfo[m_cameraMode]->gpsLongitude != 0) {
        if (m_curCameraInfo[m_cameraMode]->gpsLatitude > 0)
            strncpy((char *)exifInfo->gps_latitude_ref, "N", 2);
        else
            strncpy((char *)exifInfo->gps_latitude_ref, "S", 2);

        if (m_curCameraInfo[m_cameraMode]->gpsLongitude > 0)
            strncpy((char *)exifInfo->gps_longitude_ref, "E", 2);
        else
            strncpy((char *)exifInfo->gps_longitude_ref, "W", 2);

        if (m_curCameraInfo[m_cameraMode]->gpsAltitude > 0)
            exifInfo->gps_altitude_ref = 0;
        else
            exifInfo->gps_altitude_ref = 1;

        double latitude = fabs(m_curCameraInfo[m_cameraMode]->gpsLatitude);
        double longitude = fabs(m_curCameraInfo[m_cameraMode]->gpsLongitude);
        double altitude = fabs(m_curCameraInfo[m_cameraMode]->gpsAltitude);

        exifInfo->gps_latitude[0].num = (uint32_t)latitude;
        exifInfo->gps_latitude[0].den = 1;
        exifInfo->gps_latitude[1].num = (uint32_t)((latitude - exifInfo->gps_latitude[0].num) * 60);
        exifInfo->gps_latitude[1].den = 1;
        exifInfo->gps_latitude[2].num = (uint32_t)(round((((latitude - exifInfo->gps_latitude[0].num) * 60)
                                        - exifInfo->gps_latitude[1].num) * 60));
        exifInfo->gps_latitude[2].den = 1;

        exifInfo->gps_longitude[0].num = (uint32_t)longitude;
        exifInfo->gps_longitude[0].den = 1;
        exifInfo->gps_longitude[1].num = (uint32_t)((longitude - exifInfo->gps_longitude[0].num) * 60);
        exifInfo->gps_longitude[1].den = 1;
        exifInfo->gps_longitude[2].num = (uint32_t)(round((((longitude - exifInfo->gps_longitude[0].num) * 60)
                                        - exifInfo->gps_longitude[1].num) * 60));
        exifInfo->gps_longitude[2].den = 1;

        exifInfo->gps_altitude.num = (uint32_t)altitude;
        exifInfo->gps_altitude.den = 1;

        struct tm tm_data;
        gmtime_r(&m_curCameraInfo[m_cameraMode]->gpsTimestamp, &tm_data);
        exifInfo->gps_timestamp[0].num = tm_data.tm_hour;
        exifInfo->gps_timestamp[0].den = 1;
        exifInfo->gps_timestamp[1].num = tm_data.tm_min;
        exifInfo->gps_timestamp[1].den = 1;
        exifInfo->gps_timestamp[2].num = tm_data.tm_sec;
        exifInfo->gps_timestamp[2].den = 1;
        snprintf((char*)exifInfo->gps_datestamp, sizeof(exifInfo->gps_datestamp),
                "%04d:%02d:%02d", tm_data.tm_year + 1900, tm_data.tm_mon + 1, tm_data.tm_mday);

        exifInfo->enableGps = true;
    } else {
        exifInfo->enableGps = false;
    }

    // 2 1th IFD TIFF Tags
    if (mExifInfo.enableThumb) {
        if (rect->w < 320 || rect->h < 240) {
            exifInfo->widthThumb = 160;
            exifInfo->heightThumb = 120;
        } else {
            exifInfo->widthThumb = m_curCameraInfo[m_cameraMode]->thumbnailW;
            exifInfo->heightThumb = m_curCameraInfo[m_cameraMode]->thumbnailH;
        }
    } else {
        exifInfo->widthThumb = m_curCameraInfo[m_cameraMode]->thumbnailW;
        exifInfo->heightThumb = m_curCameraInfo[m_cameraMode]->thumbnailH;
    }
}

void ExynosCamera::m_secRect2SecRect2(ExynosRect *rect, ExynosRect2 *rect2)
{
    rect2->x1 = rect->x;
    rect2->y1 = rect->y;
    rect2->x2 = rect->x + rect->w;
    rect2->y2 = rect->y + rect->h;
}

void ExynosCamera::m_secRect22SecRect(ExynosRect2 *rect2, ExynosRect *rect)
{
    rect->x = rect2->x1;
    rect->y = rect2->y1;
    rect->w = rect2->x2 - rect2->x1;
    rect->h = rect2->y2 - rect2->y1;
}

void ExynosCamera::m_printFormat(int colorFormat, const char *arg)
{
    switch (colorFormat) {
    case V4L2_PIX_FMT_YUV420:
        CLOGV("DEBUG(%s):V4L2_PIX_FMT_YUV420", arg);
        break;
    case V4L2_PIX_FMT_YVU420:
        CLOGV("DEBUG(%s):V4L2_PIX_FMT_YVU420", arg);
        break;
    case V4L2_PIX_FMT_YVU420M:
        CLOGV("DEBUG(%s):V4L2_PIX_FMT_YVU420M", arg);
        break;
    case V4L2_PIX_FMT_NV12M:
        CLOGV("DEBUG(%s):V4L2_PIX_FMT_NV12M", arg);
        break;
    case V4L2_PIX_FMT_NV12:
        CLOGV("DEBUG(%s):V4L2_PIX_FMT_NV12", arg);
        break;
    case V4L2_PIX_FMT_NV12T:
        CLOGV("DEBUG(%s):V4L2_PIX_FMT_NV12T", arg);
        break;
    case V4L2_PIX_FMT_NV21:
        CLOGV("DEBUG(%s):V4L2_PIX_FMT_NV21", arg);
        break;
    case V4L2_PIX_FMT_NV21M:
        CLOGV("DEBUG(%s):V4L2_PIX_FMT_NV21M", arg);
        break;
    case V4L2_PIX_FMT_YUV422P:
        CLOGV("DEBUG(%s):V4L2_PIX_FMT_YUV422PP", arg);
        break;
    case V4L2_PIX_FMT_YUYV:
        CLOGV("DEBUG(%s):V4L2_PIX_FMT_YUYV", arg);
        break;
    case V4L2_PIX_FMT_UYVY:
        CLOGV("DEBUG(%s):V4L2_PIX_FMT_UYVYI", arg);
        break;
    case V4L2_PIX_FMT_RGB565:
        CLOGV("DEBUG(%s):V4L2_PIX_FMT_RGB565", arg);
        break;
    default:
        CLOGV("DEBUG(%s):Unknown Format", arg);
        break;
    }
}

///////////////////////////////////////////////////
// Additional API.
///////////////////////////////////////////////////

bool ExynosCamera::setAngle(int angle)
{
    if (m_curCameraInfo[m_cameraMode]->angle != angle) {
        switch (angle) {
        case -360:
        case    0:
        case  360:
            m_curCameraInfo[m_cameraMode]->angle = 0;
            break;

        case -270:
        case   90:
            m_curCameraInfo[m_cameraMode]->angle = 90;
            break;

        case -180:
        case  180:
            m_curCameraInfo[m_cameraMode]->angle = 180;
            break;

        case  -90:
        case  270:
            m_curCameraInfo[m_cameraMode]->angle = 270;
            break;

        default:
            CLOGE("ERR(%s):Invalid angle(%d)", __func__, angle);
            return false;
        }

        if (m_flagCreate == true) {
            if (exynos_v4l2_s_ctrl(m_camera_info[m_cameraMode].preview.fd, V4L2_CID_ROTATION, angle) < 0) {
                CLOGE("ERR(%s):exynos_v4l2_s_ctrl() fail", __func__);
                return false;
            }
        }
    }

    return true;
}

int ExynosCamera::getAngle(void)
{
    return m_curCameraInfo[m_cameraMode]->angle;
}

bool ExynosCamera::setISO(int iso)
{
    struct camera2_shot *shot = &m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot;

    struct camera2_shot *shot2 = &m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot;

    if (m_flagCreate == true) {
        if (iso <= 0) {
            shot->ctl.aa.isoMode  = AA_ISOMODE_AUTO;
            shot->ctl.aa.isoValue = 0;
            shot->ctl.sensor.sensitivity = 0;

            shot2->ctl.aa.isoMode  = AA_ISOMODE_AUTO;
            shot2->ctl.aa.isoValue = 0;
            shot2->ctl.sensor.sensitivity = 0;

         } else {
            shot->ctl.aa.isoMode  = AA_ISOMODE_MANUAL;
            shot->ctl.aa.isoValue = iso;
            shot->ctl.sensor.sensitivity = 0;

            shot2->ctl.aa.isoMode  = AA_ISOMODE_MANUAL;
            shot2->ctl.aa.isoValue = iso;
            shot2->ctl.sensor.sensitivity = 0;
         }
        m_curCameraInfo[m_cameraMode]->iso = iso;
    }

    return true;
}

int ExynosCamera::getISO(void)
{
    return m_curCameraInfo[m_cameraMode]->iso;
}

bool ExynosCamera::setContrast(int value)
{
    int internalValue = -1;

    switch (value) {
    case CONTRAST_AUTO:
        if (m_internalISP == true) {
            internalValue = ::IS_CONTRAST_DEFAULT;
        } else {
            CLOGW("WARN(%s):Invalid contrast value (%d)", __func__, value);
            return true;
        }
        break;
    case CONTRAST_MINUS_2:
        if (m_internalISP == true)
            internalValue = ::IS_CONTRAST_MINUS_2;
        else
            internalValue = ::CONTRAST_MINUS_2;
        break;
    case CONTRAST_MINUS_1:
        if (m_internalISP == true)
            internalValue = ::IS_CONTRAST_MINUS_1;
        else
            internalValue = ::CONTRAST_MINUS_1;
        break;
    case CONTRAST_DEFAULT:
        if (m_internalISP == true)
            internalValue = ::IS_CONTRAST_DEFAULT;
        else
            internalValue = ::CONTRAST_DEFAULT;
        break;
    case CONTRAST_PLUS_1:
        if (m_internalISP == true)
            internalValue = ::IS_CONTRAST_PLUS_1;
        else
            internalValue = ::CONTRAST_PLUS_1;
        break;
    case CONTRAST_PLUS_2:
        if (m_internalISP == true)
            internalValue = ::IS_CONTRAST_PLUS_2;
        else
            internalValue = ::CONTRAST_PLUS_2;
        break;
    default:
        CLOGE("ERR(%s):Unsupported value(%d)", __func__, value);
        return false;
        break;
    }

    if (m_internalISP == true) {
        if (internalValue < ::IS_CONTRAST_AUTO || ::IS_CONTRAST_MAX <= internalValue) {
            CLOGE("ERR(%s):Invalid internalValue (%d)", __func__, internalValue);
            return false;
        }
    } else {
        if (internalValue < ::CONTRAST_MINUS_2 || ::CONTRAST_MAX <= internalValue) {
            CLOGE("ERR(%s):Invalid internalValue (%d)", __func__, internalValue);
            return false;
        }
    }

    if (m_curCameraInfo[m_cameraMode]->contrast != value) {
        m_curCameraInfo[m_cameraMode]->contrast = value;
        if (m_flagCreate == true) {
            if (m_internalISP == true) {
                m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.color.contrast = internalValue;
                m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.color.contrast = internalValue;
            } else {
                if (exynos_v4l2_s_ctrl(m_camera_info[m_cameraMode].preview.fd, V4L2_CID_CAMERA_CONTRAST, internalValue) < 0) {
                    CLOGE("ERR(%s):exynos_v4l2_s_ctrl() fail", __func__);
                    return false;
                }
            }
        }
    }

    return true;
}

int ExynosCamera::getContrast(void)
{
    return m_curCameraInfo[m_cameraMode]->contrast;
}

bool ExynosCamera::setSaturation(int saturation)
{
    int internalValue = saturation + SATURATION_DEFAULT;
    if (internalValue < SATURATION_MINUS_2 || SATURATION_MAX <= internalValue) {
        CLOGE("ERR(%s):Invalid internalValue (%d)", __func__, internalValue);
        return false;
    }

    if (m_curCameraInfo[m_cameraMode]->saturation != saturation) {
        m_curCameraInfo[m_cameraMode]->saturation = saturation;
        if (m_internalISP == true) {
            m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.color.saturation = internalValue + 1;
            m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.color.saturation = internalValue + 1;
        } else {
            if (exynos_v4l2_s_ctrl(m_camera_info[m_cameraMode].preview.fd, V4L2_CID_CAMERA_SATURATION, internalValue) < 0) {
                CLOGE("ERR(%s):exynos_v4l2_s_ctrl() fail", __func__);
                return false;
            }
        }
    }

    return true;
}

int ExynosCamera::getSaturation(void)
{
    return m_curCameraInfo[m_cameraMode]->saturation;
}

bool ExynosCamera::setSharpness(int sharpness)
{
    int internalValue = sharpness + SHARPNESS_DEFAULT;
    if (internalValue < SHARPNESS_MINUS_2 || SHARPNESS_MAX <= internalValue) {
        CLOGE("ERR(%s):Invalid internalValue (%d)", __func__, internalValue);
        return false;
    }

    if (m_curCameraInfo[m_cameraMode]->sharpness != sharpness) {
        m_curCameraInfo[m_cameraMode]->sharpness = sharpness;

        if (m_internalISP == true) {
            if (sharpness == 0) {
                m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.edge.mode = PROCESSING_MODE_OFF;
                m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.edge.mode = PROCESSING_MODE_OFF;
            } else {
                m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.edge.strength = internalValue + 1;
                m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.edge.mode = PROCESSING_MODE_HIGH_QUALITY;
                m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.edge.strength = internalValue + 1;
                m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.edge.mode = PROCESSING_MODE_HIGH_QUALITY;
            }
        } else {
            if (exynos_v4l2_s_ctrl(m_camera_info[m_cameraMode].preview.fd, V4L2_CID_CAMERA_SHARPNESS, internalValue) < 0) {
                CLOGE("ERR(%s):exynos_v4l2_s_ctrl() fail", __func__);
                return false;
            }
        }
    }

    return true;
}

int ExynosCamera::getSharpness(void)
{
    return m_curCameraInfo[m_cameraMode]->sharpness;
}

bool ExynosCamera::setHue(int hue)
{
    int internalValue = hue;

    if (m_internalISP == true) {
        internalValue += IS_HUE_DEFAULT;
        if (internalValue < IS_HUE_MINUS_2 || IS_HUE_MAX <= internalValue) {
            CLOGE("ERR(%s):Invalid hue (%d)", __func__, hue);
            return false;
        }
    } else {
        CLOGV("WARN(%s):Not supported hue setting", __func__);
        return true;
    }

    if (m_curCameraInfo[m_cameraMode]->hue != hue) {
        m_curCameraInfo[m_cameraMode]->hue = hue;
        m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.color.hue = internalValue + 1;
        m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.color.hue = internalValue + 1;
    }

    return true;
}

int ExynosCamera::getHue(void)
{
    return m_curCameraInfo[m_cameraMode]->hue;
}

bool ExynosCamera::setAntiShake(bool toggle)
{
    int internalValue = ANTI_SHAKE_OFF;

    if (toggle == true)
        internalValue = ANTI_SHAKE_STILL_ON;

    if (m_internalISP == true) {
        if (toggle == true) {
            m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.aa.mode = AA_CONTROL_USE_SCENE_MODE;
            m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.aa.sceneMode = AA_SCENE_MODE_ANTISHAKE;

            m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.aa.mode = AA_CONTROL_USE_SCENE_MODE;
            m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.aa.sceneMode = AA_SCENE_MODE_ANTISHAKE;
        }
        m_curCameraInfo[CAMERA_MODE_BACK]->antiShake = toggle;

        m_curCameraInfo[CAMERA_MODE_FRONT]->antiShake = toggle;
    } else {
        if (exynos_v4l2_s_ctrl(m_camera_info[m_cameraMode].preview.fd, V4L2_CID_CAMERA_ANTI_SHAKE, internalValue) < 0) {
            CLOGE("ERR(%s):exynos_v4l2_s_ctrl() fail", __func__);
            return false;
        }
    }

    return true;
}

bool ExynosCamera::getAntiShake(void)
{
    return m_curCameraInfo[m_cameraMode]->antiShake;
}

bool ExynosCamera::setMeteringMode(int value)
{
    struct camera2_shot *shot = &m_camera_info[m_cameraMode].dummy_shot.shot;
    enum aa_aemode aeMode;

    if (m_oldMeteringMode != value) {
        CLOGD("DEBUG(%s):(%d) old mode(%d) / new mode (%d) aeLock(%d)",
        __func__,
        __LINE__,
        m_curCameraInfo[m_cameraMode]->metering,
        value,
        m_curCameraInfo[m_cameraMode]->autoExposureLock);
    }

    if (m_curCameraInfo[m_cameraMode]->autoExposureLock == true) {
        CLOGD("DEBUG(%s):autoExposureLock == true", __func__);
        return true;
    }
    /* aeRegions[0]:left, [1]:top, [2]:right, [3]:bottom, [4]:weight */

    switch (value) {
    case METERING_MODE_AVERAGE:
        aeMode = AA_AEMODE_AVERAGE;
        shot->ctl.aa.aeRegions[0] = 0;
        shot->ctl.aa.aeRegions[1] = 0;
        shot->ctl.aa.aeRegions[2] = m_curCameraInfo[m_cameraMode]->pictureW;
        shot->ctl.aa.aeRegions[3] = m_curCameraInfo[m_cameraMode]->pictureH;
        shot->ctl.aa.aeRegions[4] = 1000;
        break;
    case METERING_MODE_CENTER:
        aeMode = AA_AEMODE_CENTER;
        shot->ctl.aa.aeRegions[0] = 0;
        shot->ctl.aa.aeRegions[1] = 0;
        shot->ctl.aa.aeRegions[2] = 0;
        shot->ctl.aa.aeRegions[3] = 0;
        shot->ctl.aa.aeRegions[4] = 1000;
        break;
    case METERING_MODE_MATRIX:
        aeMode = AA_AEMODE_MATRIX;
        shot->ctl.aa.aeRegions[0] = 0;
        shot->ctl.aa.aeRegions[1] = 0;
        shot->ctl.aa.aeRegions[2] = m_curCameraInfo[m_cameraMode]->pictureW;
        shot->ctl.aa.aeRegions[3] = m_curCameraInfo[m_cameraMode]->pictureH;
        shot->ctl.aa.aeRegions[4] = 1000;
        break;
    case METERING_MODE_SPOT:
        /* In spot mode, default region setting is 100x100 rectangle on center */
        aeMode = AA_AEMODE_SPOT;
        shot->ctl.aa.aeRegions[0] = m_curCameraInfo[m_cameraMode]->pictureW / 2 - 50;
        shot->ctl.aa.aeRegions[1] = m_curCameraInfo[m_cameraMode]->pictureH / 2 - 50;
        shot->ctl.aa.aeRegions[2] = m_curCameraInfo[m_cameraMode]->pictureW / 2 + 50;
        shot->ctl.aa.aeRegions[3] = m_curCameraInfo[m_cameraMode]->pictureH / 2 + 50;
        shot->ctl.aa.aeRegions[4] = 50;
        break;
    default:
        CLOGE("ERR(%s):Unsupported value(%d)", __func__, value);
        return false;
        break;
    }

    m_curCameraInfo[m_cameraMode]->metering = value;
    m_oldMeteringMode = m_curCameraInfo[m_cameraMode]->metering;
    shot->ctl.aa.aeMode = aeMode;

    m_flashMgr->setFlashExposure(aeMode);

    CLOGD("DEBUG(%s):Metering(%d) -> aeMode(%d)(%d,%d,%d,%d,%d)",
        __func__,
        value,
        shot->ctl.aa.aeMode,
        shot->ctl.aa.aeRegions[0],
        shot->ctl.aa.aeRegions[1],
        shot->ctl.aa.aeRegions[2],
        shot->ctl.aa.aeRegions[3],
        shot->ctl.aa.aeRegions[4]);

    return true;
}

int ExynosCamera::getMeteringMode(void)
{
    return m_curCameraInfo[m_cameraMode]->metering;
}

bool ExynosCamera::setBrightness(int brightness)
{
    int internalValue = brightness;

    if (m_internalISP == true) {
        internalValue += IS_BRIGHTNESS_DEFAULT;
        if (internalValue < IS_BRIGHTNESS_MINUS_2 || IS_BRIGHTNESS_PLUS_2 < internalValue) {
            CLOGE("ERR(%s):Invalid internalValue(%d)", __func__, internalValue);
            return false;
        }
    } else {
        internalValue += EV_DEFAULT;
        if (internalValue < EV_MINUS_4 || EV_PLUS_4 < internalValue) {
            CLOGE("ERR(%s):Invalid internalValue(%d)", __func__, internalValue);
            return false;
        }
    }

    if (m_curCameraInfo[m_cameraMode]->brightness != brightness) {
        m_curCameraInfo[m_cameraMode]->brightness = brightness;
        if (m_internalISP == true) {
            m_camera_info[CAMERA_MODE_BACK].dummy_shot.shot.ctl.color.brightness = internalValue + 1;
            m_camera_info[CAMERA_MODE_FRONT].dummy_shot.shot.ctl.color.brightness = internalValue + 1;
        } else {
            if (exynos_v4l2_s_ctrl(m_camera_info[m_cameraMode].preview.fd, V4L2_CID_CAMERA_BRIGHTNESS, internalValue) < 0) {
                CLOGE("ERR(%s):exynos_v4l2_s_ctrl() fail", __func__);
                return false;
            }
        }
    }

    return true;
}

int ExynosCamera::getBrightness(void)
{
    return m_curCameraInfo[m_cameraMode]->brightness;
}

bool ExynosCamera::setObjectTracking(bool toggle)
{
    m_curCameraInfo[m_cameraMode]->objectTracking = toggle;
    return true;
}

bool ExynosCamera::getObjectTracking(void)
{
    return m_curCameraInfo[m_cameraMode]->objectTracking;
}

bool ExynosCamera::setObjectTrackingStart(bool toggle)
{
    if (m_curCameraInfo[m_cameraMode]->objectTrackingStart != toggle) {
        m_curCameraInfo[m_cameraMode]->objectTrackingStart = toggle;

        int startStop = (toggle == true) ? 1 : 0;
        if (exynos_v4l2_s_ctrl(m_camera_info[m_cameraMode].preview.fd, V4L2_CID_CAMERA_OBJ_TRACKING_START_STOP, startStop) < 0) {
            CLOGE("ERR(%s):exynos_v4l2_s_ctrl() fail", __func__);
            return false;
        }
    }

    return true;
}

int ExynosCamera::getObjectTrackingStatus(void)
{
    int ret = 0;

    if (exynos_v4l2_g_ctrl(m_camera_info[m_cameraMode].preview.fd, V4L2_CID_CAMERA_OBJ_TRACKING_STATUS, &ret) < 0) {
        CLOGE("ERR(%s):exynos_v4l2_g_ctrl() fail", __func__);
        return -1;
    }
    return ret;
}

bool ExynosCamera::setObjectPosition(int x, int y)
{
    if (m_curCameraInfo[m_cameraMode]->previewW == 640)
        x = x - 80;

    if (exynos_v4l2_s_ctrl(m_camera_info[m_cameraMode].preview.fd, V4L2_CID_IS_CAMERA_OBJECT_POSITION_X, x) < 0) {
        CLOGE("ERR(%s):exynos_v4l2_s_ctrl() fail", __func__);
        return false;
    }

    if (exynos_v4l2_s_ctrl(m_camera_info[m_cameraMode].preview.fd, V4L2_CID_IS_CAMERA_OBJECT_POSITION_Y, y) < 0) {
        CLOGE("ERR(%s):exynos_v4l2_s_ctrl() fail", __func__);
        return false;
    }

    return true;
}

bool ExynosCamera::setTouchAFStart(bool toggle)
{
    if (m_curCameraInfo[m_cameraMode]->touchAfStart != toggle) {
        m_curCameraInfo[m_cameraMode]->touchAfStart = toggle;
        int startStop = (toggle == true) ? 1 : 0;

        if (exynos_v4l2_s_ctrl(m_camera_info[m_cameraMode].preview.fd, V4L2_CID_CAMERA_TOUCH_AF_START_STOP, startStop) < 0) {
            CLOGE("ERR(%s):exynos_v4l2_s_ctrl() fail", __func__);
            return false;
        }
    }

    return true;
}

bool ExynosCamera::setTopDownMirror(void)
{
    if (m_camera_info[m_cameraMode].preview.fd < 0) {
        CLOGE("ERR(%s):Camera was closed", __func__);
        return false;
    }

    if (exynos_v4l2_s_ctrl(m_camera_info[m_cameraMode].preview.fd, V4L2_CID_VFLIP, 1) < 0) {
        CLOGE("ERR(%s):exynos_v4l2_s_ctrl() fail", __func__);
        return false;
    }

    return true;
}

bool ExynosCamera::setLRMirror(void)
{
    if (m_camera_info[m_cameraMode].preview.fd < 0) {
        CLOGE("ERR(%s):Camera was closed", __func__);
        return false;
    }

    if (exynos_v4l2_s_ctrl(m_camera_info[m_cameraMode].preview.fd, V4L2_CID_HFLIP, 1) < 0) {
        CLOGE("ERR(%s):exynos_v4l2_s_ctrl() fail", __func__);
        return false;
    }

    return true;
}

bool ExynosCamera::setGamma(bool toggle)
{
    if (m_curCameraInfo[m_cameraMode]->gamma != toggle) {
        m_curCameraInfo[m_cameraMode]->gamma = toggle;

        int gamma = (toggle == true) ? GAMMA_ON : GAMMA_OFF;

        if (m_flagCreate == true) {
            if (m_internalISP == true) {
                /* TODO */
                ;
            } else {
                 if (exynos_v4l2_s_ctrl(m_camera_info[m_cameraMode].preview.fd, V4L2_CID_CAMERA_SET_GAMMA, gamma) < 0) {
                     CLOGE("ERR(%s):exynos_v4l2_s_ctrl() fail", __func__);
                     return false;
                 }
            }
        }
    }

    return true;
}

bool ExynosCamera::getGamma(void)
{
    return m_curCameraInfo[m_cameraMode]->gamma;
}

bool ExynosCamera::setODC(bool toggle)
{
    if (m_flagCreate == true) {
        if (m_curCameraInfo[m_cameraMode]->odc != toggle) {
            m_curCameraInfo[m_cameraMode]->odc = toggle;

#ifdef USE_ODC
            int odc = (toggle == true) ? CAMERA_ODC_ON : CAMERA_ODC_OFF;

            if (exynos_v4l2_s_ctrl(m_camera_info[m_cameraMode].preview.fd, V4L2_CID_CAMERA_SET_ODC, odc) < 0) {
                CLOGE("ERR(%s):exynos_v4l2_s_ctrl() fail", __func__);
                return false;
            }
#endif
        }
    }

     return true;
}

bool ExynosCamera::getODC(void)
{
    return m_curCameraInfo[m_cameraMode]->odc;
}

bool ExynosCamera::setSlowAE(bool toggle)
{
    if (m_curCameraInfo[m_cameraMode]->slowAE != toggle) {
        m_curCameraInfo[m_cameraMode]->slowAE = toggle;

        int slow_ae = (toggle == true) ? SLOW_AE_ON : SLOW_AE_OFF;

        if (m_flagCreate == true) {
            if (m_internalISP == true) {
                /* TODO */
                ;
            } else {
                if (exynos_v4l2_s_ctrl(m_camera_info[m_cameraMode].preview.fd, V4L2_CID_CAMERA_SET_SLOW_AE, slow_ae) < 0) {
                    CLOGE("ERR(%s):exynos_v4l2_s_ctrl() fail", __func__);
                    return false;
                }
            }
        }
    }

    return true;
}

bool ExynosCamera::getSlowAE(void)
{
    return m_curCameraInfo[m_cameraMode]->slowAE;
}

void ExynosCamera::setCallbackCSC(bool csc)
{
    m_needCallbackCSC = csc;
}

bool ExynosCamera::getCallbackCSC(void)
{
    return m_needCallbackCSC;
}

bool ExynosCamera::set3DNR(bool toggle)
{
    if (m_flagCreate == true) {
        if (m_curCameraInfo[m_cameraMode]->tdnr != toggle) {
            m_curCameraInfo[m_cameraMode]->tdnr = toggle;
#ifdef USE_3DNR
            int tdnr = (toggle == true) ? CAMERA_3DNR_ON : CAMERA_3DNR_OFF;

            if (exynos_v4l2_s_ctrl(m_camera_info[m_cameraMode].preview.fd, V4L2_CID_CAMERA_SET_3DNR, tdnr) < 0) {
                CLOGE("ERR(%s):exynos_v4l2_s_ctrl() fail", __func__);
                return false;
            }
#endif
        }
    }

    return true;
}

bool ExynosCamera::get3DNR(void)
{
    return m_curCameraInfo[m_cameraMode]->tdnr;
}

int ExynosCamera::getIllumination(void)
{
   return (int)(m_camera_info[m_cameraMode].is3aa_dm.shot.udm.ae.vendorSpecific[5] / 256);
}

bool ExynosCamera::setVtMode(int vtMode)
{
    switch (vtMode) {
    // 3G vtmode (176x144, Fixed 7fps)
    case 1:
        for (int i = 0; i < CAMERA_MODE_MAX; i++)
            m_curCameraInfo[i]->vtMode = 1;
        break;
    // LTE or WIFI vtmode (640x480, Fixed 15fps)
    case 2:
        for (int i = 0; i < CAMERA_MODE_MAX; i++)
            m_curCameraInfo[i]->vtMode = 2;
        break;
    default:
        for (int i = 0; i < CAMERA_MODE_MAX; i++)
            m_curCameraInfo[i]->vtMode = 0;
        break;
    }

    return true;
}

bool ExynosCamera::setIntelligentMode(int intelligentMode)
{
    for (int i = 0; i < CAMERA_MODE_MAX; i++)
        m_curCameraInfo[i]->intelligentMode = intelligentMode;

    return true;
}

int ExynosCamera::getIntelligentMode(void)
{
    return m_curCameraInfo[m_cameraMode]->intelligentMode;
}

bool ExynosCamera::getFnumber(int *num, int *den)
{
    *num = m_curCameraInfo[m_cameraMode]->fNumberNum;
    *den = m_curCameraInfo[m_cameraMode]->fNumberDen;

    return true;
}

bool ExynosCamera::getApertureValue(int *num, int *den)
{
    *num = m_curCameraInfo[m_cameraMode]->apertureNum;
    *den = m_curCameraInfo[m_cameraMode]->apertureDen;

    return true;
}

int ExynosCamera::getFocalLengthIn35mmFilm(void)
{
    return m_curCameraInfo[m_cameraMode]->focalLengthIn35mmLength;
}

const char *ExynosCamera::getImageUniqueId(void)
{
    return m_imageUniqueIdBuf;
}

bool ExynosCamera::setAutoFocusMacroPosition(int autoFocusMacroPosition)
{
    int oldAutoFocusMacroPosition = m_curCameraInfo[m_cameraMode]->autoFocusMacroPosition;

    m_curCameraInfo[m_cameraMode]->autoFocusMacroPosition = autoFocusMacroPosition;

    int macroPosition = ExynosCameraActivityAutofocus::AUTOFOCUS_MACRO_POSITION_BASE;

    switch (autoFocusMacroPosition) {
    case 0:
        macroPosition = ExynosCameraActivityAutofocus::AUTOFOCUS_MACRO_POSITION_CENTER;
        break;
    case 1:
        macroPosition = ExynosCameraActivityAutofocus::AUTOFOCUS_MACRO_POSITION_CENTER_UP;
        break;
    default:
        break;
    }

    m_autofocusMgr->setMacroPosition(macroPosition);

    /* if macro option change, we need to restart CAF */
    if (oldAutoFocusMacroPosition != m_curCameraInfo[m_cameraMode]->autoFocusMacroPosition) {
        if ((m_autofocusMgr->getAutofocusMode() == ExynosCameraActivityAutofocus::AUTOFOCUS_MODE_CONTINUOUS_PICTURE ||
             m_autofocusMgr->getAutofocusMode() == ExynosCameraActivityAutofocus::AUTOFOCUS_MODE_CONTINUOUS_VIDEO ||
             m_autofocusMgr->getAutofocusMode() == ExynosCameraActivityAutofocus::AUTOFOCUS_MODE_CONTINUOUS_PICTURE_MACRO) &&
            m_autofocusMgr->flagAutofocusStart() == true &&
            m_autofocusMgr->flagLockAutofocus() == false) {
            m_autofocusMgr->stopAutofocus();
            m_autofocusMgr->startAutofocus();
        }
    }

    return true;
}

bool ExynosCamera::m_initSensor(int cameraMode)
{
    CLOGD("DEBUG(%s):(%d) fd %d", __func__, __LINE__, m_camera_info[cameraMode].sensor.fd);

    //Init Dummy and Initialized camera_shot
    memset(&m_camera_info[cameraMode].default_shot, 0x00, sizeof(struct camera2_shot_ext));
    memset(&m_camera_info[cameraMode].dummy_shot, 0x00, sizeof(struct camera2_shot_ext));
    memset(&m_camera_info[cameraMode].is3aa_dm, 0x00, sizeof(struct camera2_shot_ext));
    memset(&m_camera_info[cameraMode].isp_dm, 0x00, sizeof(struct camera2_shot_ext));

    struct camera2_shot *shot = &m_camera_info[cameraMode].default_shot.shot;

    // 1. ctl
    // request
    shot->ctl.request.id = 0;
    shot->ctl.request.metadataMode = METADATA_MODE_FULL;
    shot->ctl.request.frameCount = 0;

    // lens
    shot->ctl.lens.focusDistance = 0.0f;
    shot->ctl.lens.aperture = (float)m_curCameraInfo[cameraMode]->apertureNum / (float)m_curCameraInfo[cameraMode]->apertureDen;
    shot->ctl.lens.focalLength = (float)m_curCameraInfo[cameraMode]->focalLengthNum / (float)m_curCameraInfo[cameraMode]->focalLengthDen;
    shot->ctl.lens.filterDensity = 0.0f;
    shot->ctl.lens.opticalStabilizationMode = ::OPTICAL_STABILIZATION_MODE_OFF;

    int minFps = (m_curCameraInfo[cameraMode]->fpsRange[0] == 0) ? 0 : (m_curCameraInfo[cameraMode]->fpsRange[1] / 2) / 1000;
    int maxFps = (m_curCameraInfo[cameraMode]->fpsRange[1] == 0) ? 0 : m_curCameraInfo[cameraMode]->fpsRange[1] / 1000;

    /* The min fps can not be '0'. Therefore it is set up default value '15'. */
    if (minFps == 0) {
        CLOGW("WRN(%s): Invalid min fps value(%d)", __func__, minFps);
        minFps = 15;
    }

    /*  The max fps can not be '0'. Therefore it is set up default value '30'. */
    if (maxFps == 0) {
        CLOGW("WRN(%s): Invalid max fps value(%d)", __func__, maxFps);
        maxFps = 30;
    }

    // sensor
    shot->ctl.sensor.exposureTime = 0;
    shot->ctl.sensor.frameDuration = (1000 * 1000 * 1000) / maxFps;
    shot->ctl.sensor.sensitivity = 0;

    // flash
    shot->ctl.flash.flashMode = ::CAM2_FLASH_MODE_OFF;
    shot->ctl.flash.firingPower = 0;
    shot->ctl.flash.firingTime = 0;

    // hotpixel
    shot->ctl.hotpixel.mode = (enum processing_mode)0;

    // demosaic
    shot->ctl.demosaic.mode = (enum processing_mode)0;

    // noise
    shot->ctl.noise.mode = ::PROCESSING_MODE_OFF;
    shot->ctl.noise.strength = 5;

    // shading
    shot->ctl.shading.mode = (enum processing_mode)0;

    // geometric
    shot->ctl.geometric.mode = (enum processing_mode)0;

    // color
    shot->ctl.color.mode = ::COLORCORRECTION_MODE_FAST;
    static const float colorTransform[9] = {
        1.0f, 0.f, 0.f,
        0.f, 1.f, 0.f,
        0.f, 0.f, 1.f
    };
    memcpy(shot->ctl.color.transform, colorTransform, sizeof(shot->ctl.color.transform));

    // tonemap
    shot->ctl.tonemap.mode = ::TONEMAP_MODE_FAST;
    static const float tonemapCurve[4] = {
        0.f, 0.f,
        1.f, 1.f
    };

    int tonemapCurveSize = sizeof(tonemapCurve);
    int sizeOfCurve = sizeof(shot->ctl.tonemap.curveRed) / sizeof(shot->ctl.tonemap.curveRed[0]);

    for (int i = 0; i < sizeOfCurve; i ++) {
        memcpy(&(shot->ctl.tonemap.curveRed[i]),   tonemapCurve, tonemapCurveSize);
        memcpy(&(shot->ctl.tonemap.curveGreen[i]), tonemapCurve, tonemapCurveSize);
        memcpy(&(shot->ctl.tonemap.curveBlue[i]),  tonemapCurve, tonemapCurveSize);
    }

    // edge
    shot->ctl.edge.mode = ::PROCESSING_MODE_OFF;
    shot->ctl.edge.strength = 5;

    // scaler
    if (m_setZoom(0,
                  m_defaultCameraInfo[cameraMode]->pictureW, m_defaultCameraInfo[cameraMode]->pictureH,
                  m_curCameraInfo[cameraMode]->previewW,     m_curCameraInfo[cameraMode]->previewH,
                  (void *)&m_camera_info[cameraMode].dummy_shot) == false) {
        CLOGE("ERR(%s):m_setZoom() fail", __func__);
    }

    // jpeg
    shot->ctl.jpeg.quality = m_jpegQuality;
    shot->ctl.jpeg.thumbnailSize[0] = m_curCameraInfo[cameraMode]->thumbnailW;
    shot->ctl.jpeg.thumbnailSize[1] = m_curCameraInfo[cameraMode]->thumbnailH;
    shot->ctl.jpeg.thumbnailQuality = m_jpegThumbnailQuality;
    shot->ctl.jpeg.gpsCoordinates[0] = 0;
    shot->ctl.jpeg.gpsCoordinates[1] = 0;
    shot->ctl.jpeg.gpsCoordinates[2] = 0;
    shot->ctl.jpeg.gpsProcessingMethod = 0;
    shot->ctl.jpeg.gpsTimestamp = 0L;
    shot->ctl.jpeg.orientation = 0L;

    // stats
    shot->ctl.stats.faceDetectMode = ::FACEDETECT_MODE_OFF;
    shot->ctl.stats.histogramMode = ::STATS_MODE_OFF;
    shot->ctl.stats.sharpnessMapMode = ::STATS_MODE_OFF;

    // aa
    shot->ctl.aa.captureIntent = ::AA_CAPTURE_INTENT_CUSTOM;
    shot->ctl.aa.mode = ::AA_CONTROL_AUTO;
    //shot->ctl.aa.effectMode = ::AA_EFFECT_OFF;
    shot->ctl.aa.sceneMode = ::AA_SCENE_MODE_FACE_PRIORITY;
    shot->ctl.aa.videoStabilizationMode = 0;

    /* default metering is center */
    shot->ctl.aa.aeMode = ::AA_AEMODE_CENTER;
    shot->ctl.aa.aeRegions[0] = 0;
    shot->ctl.aa.aeRegions[1] = 0;
    shot->ctl.aa.aeRegions[2] = 0;
    shot->ctl.aa.aeRegions[3] = 0;
    shot->ctl.aa.aeRegions[4] = 1000;
    shot->ctl.aa.aeExpCompensation = 5; // 5 is middle

    shot->ctl.aa.aeTargetFpsRange[0] = minFps;
    shot->ctl.aa.aeTargetFpsRange[1] = maxFps;

    shot->ctl.aa.aeAntibandingMode = ::AA_AE_ANTIBANDING_AUTO;
    shot->ctl.aa.aeflashMode = ::AA_FLASHMODE_OFF;

    shot->ctl.aa.awbMode = ::AA_AWBMODE_WB_AUTO;
    shot->ctl.aa.afMode = ::AA_AFMODE_OFF;
    shot->ctl.aa.afRegions[0] = 0;
    shot->ctl.aa.afRegions[1] = 0;
    shot->ctl.aa.afRegions[2] = 0;
    shot->ctl.aa.afRegions[3] = 0;
    shot->ctl.aa.afRegions[4] = 1000;
    shot->ctl.aa.afTrigger = 0;

    shot->ctl.aa.isoMode = AA_ISOMODE_AUTO;
    shot->ctl.aa.isoValue = 0;
    shot->ctl.color.saturation = 3; // "3" is default.


    // 2. dm

    // 3. utrl

    // 4. udm

    // 5. magicNumber
    shot->magicNumber = SHOT_MAGIC_NUMBER;

    // user request
    m_camera_info[cameraMode].default_shot.dis_bypass = 1;
    m_camera_info[cameraMode].default_shot.dnr_bypass = 1;
    m_camera_info[cameraMode].default_shot.fd_bypass  = 1;

    m_camera_info[cameraMode].default_shot.request_3ax = 1;
    m_camera_info[cameraMode].default_shot.request_isp = 1;
    m_camera_info[cameraMode].default_shot.request_scc = 0;
    m_camera_info[cameraMode].default_shot.request_scp = 1;
    m_camera_info[cameraMode].default_shot.request_dis = 0;

    // default shot
    memcpy(&m_camera_info[cameraMode].dummy_shot, &m_camera_info[cameraMode].default_shot, sizeof(struct camera2_shot_ext));
    memcpy(&m_camera_info[cameraMode].is3aa_dm, &m_camera_info[cameraMode].default_shot, sizeof(struct camera2_shot_ext));
    memcpy(&m_camera_info[cameraMode].isp_dm, &m_camera_info[cameraMode].default_shot, sizeof(struct camera2_shot_ext));

    return true;
}

bool ExynosCamera::m_openSensor(int cameraMode)
{
    CLOGD("DEBUG(%s):in", __func__);

    bool ret = true;

    if (0 <= m_camera_info[cameraMode].sensor.fd) {
        CLOGE("ERR(%s):Already open fd (%d)", __func__, m_camera_info[cameraMode].sensor.fd);
        return false;
    }

    char node_name[30];

    switch (cameraMode) {
    case CAMERA_MODE_BACK:
        memset(&node_name, 0x00, sizeof(node_name));
        snprintf(node_name, sizeof(node_name), "%s%d", NODE_PREFIX, FIMC_IS_VIDEO_SEN0_NUM);

        m_camera_info[cameraMode].sensor.fd = exynos_v4l2_open(node_name, O_RDWR, 0);
        if (m_camera_info[cameraMode].sensor.fd < 0) {
            CLOGE("ERR(%s):exynos_v4l2_open(%s) fail (%d)", __func__, node_name, m_camera_info[cameraMode].sensor.fd);
            m_camera_info[cameraMode].sensor.fd = -1;
            ret = false;
        }

        break;
    case CAMERA_MODE_FRONT:
        memset(&node_name, 0x00, sizeof(node_name));
        snprintf(node_name, sizeof(node_name), "%s%d", NODE_PREFIX, FIMC_IS_VIDEO_SEN1_NUM);

        m_camera_info[cameraMode].sensor.fd = exynos_v4l2_open(node_name, O_RDWR, 0);
        if (m_camera_info[cameraMode].sensor.fd < 0) {
            CLOGE("ERR(%s):exynos_v4l2_open(%s) fail (%d)", __func__, node_name, m_camera_info[cameraMode].sensor.fd);
            m_camera_info[cameraMode].sensor.fd = -1;
            ret = false;
        }
        break;
    case CAMERA_MODE_REPROCESSING:
        /* TODO: currently the reprocessing sensor fd is same with non-reprocessing */
        if (0 <= m_camera_info[m_cameraMode].sensor.fd) {
            m_camera_info[cameraMode].sensor.fd = m_camera_info[m_cameraMode].sensor.fd;
            m_camera_info[cameraMode].sensor.flagDup = true;
        }
        break;

    default:
        CLOGE("ERR(%s):invalid cameraMode(%d)", __func__, cameraMode);
        break;
    }

    CLOGD("DEBUG(%s):cameraMode(%d) : fd(%d)", __func__, cameraMode, m_camera_info[cameraMode].sensor.fd);

    return ret;
}

bool ExynosCamera::m_closeSensor(int cameraMode)
{
    CLOGD("DEBUG(%s):in", __func__);

    switch (cameraMode) {
    case CAMERA_MODE_BACK:
    case CAMERA_MODE_FRONT:
        if (0 <= m_camera_info[cameraMode].sensor.fd) {
            if (exynos_v4l2_close(m_camera_info[cameraMode].sensor.fd) != 0) {
                CLOGE("ERR(%s):exynos_v4l2_close(%d) fail", __func__, m_camera_info[cameraMode].sensor.fd);
                return false;
            }

            m_camera_info[cameraMode].sensor.fd = -1;
        }
        break;

    case CAMERA_MODE_REPROCESSING:
        m_camera_info[cameraMode].sensor.fd = -1;
        break;
    default:
        CLOGE("ERR(%s):invalid cameraMode(%d)", __func__, cameraMode);
        break;
    }

    CLOGD("DEBUG(%s):cameraMode(%d) : fd(%d)", __func__, cameraMode, m_camera_info[cameraMode].sensor.fd);

    return true;
}

bool ExynosCamera::m_openIsp(int cameraMode)
{
    CLOGD("DEBUG(%s):in", __func__);

    if (0 <= m_camera_info[cameraMode].isp.fd) {
        CLOGE("ERR(%s):Already open fd (%d)", __func__, m_camera_info[cameraMode].isp.fd);
        return false;
    }

    char node_name[30];
    memset(&node_name, 0x00, sizeof(node_name));
    snprintf(node_name, sizeof(node_name), "%s%d", NODE_PREFIX, FIMC_IS_VIDEO_ISP_NUM);

    m_camera_info[cameraMode].isp.fd = exynos_v4l2_open(node_name, O_RDWR, 0);
    if (m_camera_info[cameraMode].isp.fd < 0) {
        CLOGE("ERR(%s):exynos_v4l2_open(%s) fail (%d)", __func__, node_name, m_camera_info[cameraMode].isp.fd);
        return false;
    }

    CLOGD("DEBUG(%s):cameraMode(%d) : fd(%d)", __func__, cameraMode, m_camera_info[cameraMode].isp.fd);

    return true;
}

bool ExynosCamera::m_closeIsp(int cameraMode)
{
    CLOGD("DEBUG(%s):in", __func__);

    if (0 <= m_camera_info[cameraMode].isp.fd) {
        if (exynos_v4l2_close(m_camera_info[cameraMode].isp.fd) != 0) {
            CLOGE("ERR(%s):exynos_v4l2_close(%d) fail", __func__, m_camera_info[cameraMode].isp.fd);
            return false;
        }

        m_camera_info[cameraMode].isp.fd = -1;
    }

    CLOGD("DEBUG(%s):cameraMode(%d) : fd(%d)", __func__, cameraMode, m_camera_info[cameraMode].isp.fd);

    return true;
}

#ifdef USE_VDIS
bool ExynosCamera::m_openVdisOut(void)
{
    CLOGD("DEBUG(%s):in", __func__);

    if (0 <=  m_camera_info[m_cameraMode].vdisc.fd) {
        CLOGE("ERR(%s):Already init fd (%d)", __func__, m_camera_info[m_cameraMode].vdisc.fd);
        return false;
    }

    if (0 <=  m_camera_info[m_cameraMode].vdiso.fd) {
        CLOGE("ERR(%s):Already init fd (%d)", __func__, m_camera_info[m_cameraMode].vdiso.fd);
        return false;
    }

    char node_name[30];

    //from allocateStream()
    memset(&node_name, 0x00, sizeof(node_name));
    snprintf(node_name, sizeof(node_name), "%s%d", NODE_PREFIX, FIMC_IS_VIDEO_VDISC_NUM);

    m_camera_info[m_cameraMode].vdisc.fd = exynos_v4l2_open(node_name, O_RDWR, 0);
    if (m_camera_info[m_cameraMode].vdisc.fd < 0) {
        CLOGE("DEBUG(%s): fail to open vdisc video node (%s) fd (%d)",
            __func__, node_name, m_camera_info[m_cameraMode].vdisc.fd);
        m_camera_info[m_cameraMode].vdisc.fd = -1;
        return false;
    } else {
        CLOGE("VDISC node is opened");
    }

    memset(&node_name, 0x00, sizeof(node_name));
    snprintf(node_name, sizeof(node_name), "%s%d", NODE_PREFIX, FIMC_IS_VIDEO_VDISO_NUM);

    m_camera_info[m_cameraMode].vdiso.fd = exynos_v4l2_open(node_name, O_RDWR, 0);
    if (m_camera_info[m_cameraMode].vdiso.fd < 0) {
        CLOGE("DEBUG(%s): fail to open vdiso video node (%s) fd (%d)",
            __func__, node_name, m_camera_info[m_cameraMode].vdiso.fd);
        m_camera_info[m_cameraMode].vdiso.fd = -1;
        return false;
    } else
        CLOGE("VDISO node is opened");

    return true;
}

bool ExynosCamera::m_closeVdisOut(void)
{
    if (0 <= m_camera_info[m_cameraMode].vdisc.fd) {
        if (exynos_v4l2_close(m_camera_info[m_cameraMode].vdisc.fd) != 0) {
            CLOGE("ERR(%s):exynos_v4l2_close fail", __func__ );
            return false;
        }

        m_camera_info[m_cameraMode].vdisc.fd= -1;
    }

    if (0 <= m_camera_info[m_cameraMode].vdiso.fd) {
        if (exynos_v4l2_close(m_camera_info[m_cameraMode].vdiso.fd) != 0) {
            CLOGE("ERR(%s):exynos_v4l2_close fail", __func__ );
            return false;
        }

        m_camera_info[m_cameraMode].vdiso.fd= -1;
    }
    return true;
}
#endif

bool ExynosCamera::m_openPreview(int cameraMode)
{
    CLOGD("DEBUG(%s):in", __func__);

    if (0 <= m_camera_info[cameraMode].preview.fd) {
        CLOGE("ERR(%s):Already open fd (%d)", __func__, m_camera_info[cameraMode].preview.fd);
        return false;
    }

    char node_name[30];
    memset(&node_name, 0x00, sizeof(node_name));
    snprintf(node_name, sizeof(node_name), "%s%d", NODE_PREFIX, FIMC_IS_VIDEO_SCP_NUM);

    m_camera_info[cameraMode].preview.fd = exynos_v4l2_open(node_name, O_RDWR, 0);
    if (m_camera_info[cameraMode].preview.fd < 0) {
        CLOGE("ERR(%s):exynos_v4l2_open(%s) fail (%d)", __func__, node_name, m_camera_info[cameraMode].preview.fd);
        m_camera_info[cameraMode].preview.fd = -1;
        return false;
    }

    CLOGD("DEBUG(%s):cameraMode(%d) : fd(%d)", __func__, cameraMode, m_camera_info[cameraMode].preview.fd);

    return true;
}

bool ExynosCamera::m_closePreview(int cameraMode)
{
    CLOGD("DEBUG(%s):in", __func__);

    if (0 <= m_camera_info[cameraMode].preview.fd) {
        if (exynos_v4l2_close(m_camera_info[cameraMode].preview.fd) != 0) {
            CLOGE("ERR(%s):exynos_v4l2_close(%d) fail", __func__, m_camera_info[cameraMode].preview.fd);
            return false;
        }

        m_camera_info[cameraMode].preview.fd = -1;
    }

    return true;
}

bool ExynosCamera::m_openPicture(int cameraMode)
{
    CLOGD("DEBUG(%s):in", __func__);

    if (0 <= m_camera_info[cameraMode].picture.fd) {
        CLOGE("ERR(%s):Already open fd (%d)", __func__, m_camera_info[cameraMode].picture.fd);
        return false;
    }

    char node_name[30];
    memset(&node_name, 0x00, sizeof(node_name));
    snprintf(node_name, sizeof(node_name), "%s%d", NODE_PREFIX, FIMC_IS_VIDEO_SCC_NUM);

    m_camera_info[cameraMode].picture.fd = exynos_v4l2_open(node_name, O_RDWR, 0);
    if (m_camera_info[cameraMode].picture.fd < 0) {
        CLOGE("ERR(%s):exynos_v4l2_open(%s) fail (%d)", __func__, node_name, m_camera_info[cameraMode].picture.fd);
        m_camera_info[cameraMode].picture.fd = -1;
        return false;
    }

    CLOGD("DEBUG(%s):cameraMode(%d) : fd(%d)", __func__, cameraMode, m_camera_info[cameraMode].picture.fd);

    return true;
}

bool ExynosCamera::m_closePicture(int cameraMode)
{
    CLOGD("DEBUG(%s):in", __func__);

    if (0 <= m_camera_info[cameraMode].picture.fd) {
        if (exynos_v4l2_close(m_camera_info[cameraMode].picture.fd) != 0) {
            CLOGE("ERR(%s):exynos_v4l2_close(%d) fail", __func__, m_camera_info[cameraMode].picture.fd);
            return false;
        }

        m_camera_info[cameraMode].picture.fd = -1;
    }

    return true;
}

bool ExynosCamera::m_openIs3a0(int cameraMode)
{
    CLOGD("DEBUG(%s):in", __func__);

    if (0 <= m_camera_info[cameraMode].is3a0Src.fd) {
        CLOGE("ERR(%s):Already open fd (%d)", __func__, m_camera_info[cameraMode].is3a0Src.fd);
        return false;
    }

    if (0 <= m_camera_info[cameraMode].is3a0Dst.fd) {
        CLOGE("ERR(%s):Already open fd (%d)", __func__, m_camera_info[cameraMode].is3a0Dst.fd);
        return false;
    }

    char node_name[30];
    memset(&node_name, 0x00, sizeof(node_name));
    snprintf(node_name, sizeof(node_name), "%s%d", NODE_PREFIX, FIMC_IS_VIDEO_3A0_NUM);

    m_camera_info[cameraMode].is3a0Src.fd = exynos_v4l2_open(node_name, O_RDWR, 0);
    if (m_camera_info[cameraMode].is3a0Src.fd < 0) {
        CLOGE("ERR(%s):exynos_v4l2_open(%s) fail (%d)", __func__, node_name, m_camera_info[cameraMode].is3a0Src.fd);
        m_camera_info[cameraMode].is3a0Src.fd = -1;
        return false;
    }

    m_camera_info[cameraMode].is3a0Dst.fd = m_camera_info[cameraMode].is3a0Src.fd;

    CLOGD("DEBUG(%s):cameraMode(%d) : fd(%d)", __func__, cameraMode, m_camera_info[cameraMode].is3a0Src.fd);

    return true;
}

bool ExynosCamera::m_closeIs3a0(int cameraMode)
{
    CLOGD("DEBUG(%s):in", __func__);

    if (0 <= m_camera_info[cameraMode].is3a0Src.fd) {
        if (exynos_v4l2_close(m_camera_info[cameraMode].is3a0Src.fd) != 0) {
            CLOGE("ERR(%s):exynos_v4l2_close(%d) fail", __func__, m_camera_info[cameraMode].is3a0Src.fd);
            return false;
        }

        m_camera_info[cameraMode].is3a0Src.fd = -1;
        m_camera_info[cameraMode].is3a0Dst.fd = -1;
    }

    return true;
}

bool ExynosCamera::m_openIs3a1(int cameraMode)
{
    CLOGD("DEBUG(%s):in", __func__);

    if (0 <= m_camera_info[cameraMode].is3a1Src.fd) {
        CLOGE("ERR(%s):Already open fd (%d)", __func__, m_camera_info[cameraMode].is3a1Src.fd);
        return false;
    }

    if (0 <= m_camera_info[cameraMode].is3a1Dst.fd) {
        CLOGE("ERR(%s):Already open fd (%d)", __func__, m_camera_info[cameraMode].is3a1Dst.fd);
        return false;
    }

    char node_name[30];
    memset(&node_name, 0x00, sizeof(node_name));
    snprintf(node_name, sizeof(node_name), "%s%d", NODE_PREFIX, FIMC_IS_VIDEO_3A1_NUM);

    m_camera_info[cameraMode].is3a1Src.fd = exynos_v4l2_open(node_name, O_RDWR, 0);
    if (m_camera_info[cameraMode].is3a1Src.fd < 0) {
        CLOGE("ERR(%s):exynos_v4l2_open(%s) fail (%d)", __func__, node_name, m_camera_info[cameraMode].is3a1Src.fd);
        m_camera_info[cameraMode].is3a1Src.fd = -1;
        return false;
    }

    m_camera_info[cameraMode].is3a1Dst.fd = m_camera_info[cameraMode].is3a1Src.fd;

    CLOGD("DEBUG(%s):cameraMode(%d) : fd(%d)", __func__, cameraMode, m_camera_info[cameraMode].is3a1Src.fd);

    return true;
}

bool ExynosCamera::m_closeIs3a1(int cameraMode)
{
    CLOGD("DEBUG(%s):in", __func__);

    if (0 <= m_camera_info[cameraMode].is3a1Src.fd) {
        if (exynos_v4l2_close(m_camera_info[cameraMode].is3a1Src.fd) != 0) {
            CLOGE("ERR(%s):exynos_v4l2_close(%d) fail", __func__, m_camera_info[cameraMode].is3a1Src.fd);
            return false;
        }

        m_camera_info[cameraMode].is3a1Src.fd = -1;
        m_camera_info[cameraMode].is3a1Dst.fd = -1;
    }

    return true;
}

bool ExynosCamera::getCropRect(int  src_w,  int   src_h,
                               int  dst_w,  int   dst_h,
                               int *crop_x, int *crop_y,
                               int *crop_w, int *crop_h,
                               int           zoom)
{
    *crop_w = src_w;
    *crop_h = src_h;

    if (src_w == 0 || src_h == 0 || dst_w == 0 || dst_h == 0) {
        CLOGE("ERR(%s):width or height valuse is 0, src(%dx%d), dst(%dx%d)",
                 __func__, src_w, src_h, dst_w, dst_h);
        return false;
    }

    if (   src_w != dst_w
        || src_h != dst_h) {
        float src_ratio = 1.0f;
        float dst_ratio = 1.0f;

        // ex : 1024 / 768
        src_ratio = (float)src_w / (float)src_h;

        // ex : 352  / 288
        dst_ratio = (float)dst_w / (float)dst_h;

        if (dst_w * dst_h < src_w * src_h) {
            if (dst_ratio <= src_ratio) {
                // shrink w
                *crop_w = src_h * dst_ratio;
                *crop_h = src_h;
            } else {
                // shrink h
                *crop_w = src_w;
                *crop_h = src_w / dst_ratio;
            }
        } else {
            if (dst_ratio <= src_ratio) {
                // shrink w
                *crop_w = src_h * dst_ratio;
                *crop_h = src_h;
            } else {
                // shrink h
                *crop_w = src_w;
                *crop_h = src_w / dst_ratio;
            }
        }
    }

    if (zoom != 0) {
        float zoomLevel = ((float)zoom + 10.0) / 10.0;
        *crop_w = (int)((float)*crop_w / zoomLevel);
        *crop_h = (int)((float)*crop_h / zoomLevel);
    }

    /*
     * HACK: align size issue
     * <CAMERA_CROP_WIDTH_RESTRAIN_NUM and CAMERA_CROP_HIGHT_RESTRAIN_NUM>
     * FIMC-IS Driver cannot separate original size for OTF input/output
     * aligned size for DMA output buffer.
     * So, cameraHAL should set aligned size for compatibility with other IPs
     * CameraHAL will set original size and aligned size
     * to FIMC-IS driver.
     */
    #define CAMERA_CROP_WIDTH_RESTRAIN_NUM  (CAMERA_MAGIC_ALIGN * 2)
    unsigned int w_align = (*crop_w & (CAMERA_CROP_WIDTH_RESTRAIN_NUM - 1));
    if (w_align != 0) {
        if (  (CAMERA_CROP_WIDTH_RESTRAIN_NUM >> 1) <= w_align
            && (int)(*crop_w + (CAMERA_CROP_WIDTH_RESTRAIN_NUM - w_align)) <= src_w) {
            *crop_w += (CAMERA_CROP_WIDTH_RESTRAIN_NUM - w_align);
        }
        else
            *crop_w -= w_align;
    }

    #define CAMERA_CROP_HEIGHT_RESTRAIN_NUM  (CAMERA_MAGIC_ALIGN)
    unsigned int h_align = (*crop_h & (CAMERA_CROP_HEIGHT_RESTRAIN_NUM - 1));
    if (h_align != 0) {
        if (  (CAMERA_CROP_HEIGHT_RESTRAIN_NUM >> 1) <= h_align
            && (int)(*crop_h + (CAMERA_CROP_HEIGHT_RESTRAIN_NUM - h_align)) <= src_h) {
            *crop_h += (CAMERA_CROP_HEIGHT_RESTRAIN_NUM - h_align);
        }
        else
            *crop_h -= h_align;
    }

    *crop_x = (src_w - *crop_w) >> 1;
    *crop_y = (src_h - *crop_h) >> 1;

    /*
    if (*crop_x & (CAMERA_CROP_WIDTH_RESTRAIN_NUM >> 1))
        *crop_x -= 1;

    if (*crop_y & (CAMERA_CROP_HEIGHT_RESTRAIN_NUM >> 1))
        *crop_y -= 1;
    */

    if (*crop_x < 0 || *crop_y < 0) {
        CLOGE("ERR(%s):crop size too big (%d, %d, %d, %d)",
                 __func__, *crop_x, *crop_y, *crop_w, *crop_h);
        return false;
    }

    return true;
}

bool ExynosCamera::getCropRect2(int  src_w,     int   src_h,
                                int  dst_w,     int   dst_h,
                                int *new_src_x, int *new_src_y,
                                int *new_src_w, int *new_src_h,
                                int           zoom)
{
    unsigned int crop_x, crop_y, crop_width, crop_height;
    unsigned int sensor_width, sensor_height, sensor_ratio;
    unsigned int chain3_width, chain3_height, chain3_ratio;

    sensor_width = src_w;
    sensor_height = src_h;
    chain3_width = dst_w;
    chain3_height = dst_h;
    crop_width = sensor_width;
    crop_height = sensor_height;
    crop_x = crop_y = 0;

    sensor_ratio = sensor_width * 1000 / sensor_height;
    chain3_ratio = chain3_width * 1000 / chain3_height;

    if (sensor_ratio == chain3_ratio) {
        crop_width = sensor_width;
        crop_height = sensor_height;
    } else if (sensor_ratio < chain3_ratio) {
        /* isp dma input limitation
        height : 2 times */
        crop_height =
            (sensor_width * chain3_height) / chain3_width;
        crop_height = ALIGN(crop_height, 2);
        crop_y = ((sensor_height - crop_height) >> 1) &
                0xFFFFFFFE;
    } else {
        /* isp dma input limitation
        width : 4 times */
        crop_width =
            (sensor_height * chain3_width) / chain3_height;
        crop_width = ALIGN(crop_width, 4);
        crop_x =  ((sensor_width - crop_width) >> 1) &
            0xFFFFFFFE;
    }

    *new_src_w = (int)crop_width;
    *new_src_h = (int)crop_height;
    *new_src_x = (int)crop_x;
    *new_src_y = (int)crop_y;

    return true;
}

bool ExynosCamera::getCropRectAlign(int  src_w,  int   src_h,
                                    int  dst_w,  int   dst_h,
                                    int *crop_x, int *crop_y,
                                    int *crop_w, int *crop_h,
                                    int align_w, int align_h,
                                    int           zoom)
{
    *crop_w = src_w;
    *crop_h = src_h;

    if (src_w == 0 || src_h == 0 || dst_w == 0 || dst_h == 0) {
        CLOGE("ERR(%s):width or height valuse is 0, src(%dx%d), dst(%dx%d)",
                 __func__, src_w, src_h, dst_w, dst_h);
        return false;
    }

    /* Calculation aspect ratio */
    if (   src_w != dst_w
        || src_h != dst_h) {
        float src_ratio = 1.0f;
        float dst_ratio = 1.0f;

        // ex : 1024 / 768
        src_ratio = (float)src_w / (float)src_h;

        // ex : 352  / 288
        dst_ratio = (float)dst_w / (float)dst_h;

        if (dst_ratio <= src_ratio) {
            // shrink w
            *crop_w = src_h * dst_ratio;
            *crop_h = src_h;
        } else {
            // shrink h
            *crop_w = src_w;
            *crop_h = src_w / dst_ratio;
        }
    }

    /* Calculation zoom */
    if (zoom != 0) {
        float zoomLevel = ((float)zoom + 10.0) / 10.0;
        *crop_w = (int)((float)*crop_w / zoomLevel);
        *crop_h = (int)((float)*crop_h / zoomLevel);
    }

    /* Alignment by desired size */
    unsigned int w_remain = (*crop_w & (align_w - 1));
    if (w_remain != 0) {
        if (  (align_w >> 1) <= w_remain
            && (int)(*crop_w + (align_w - w_remain)) <= src_w) {
            *crop_w += (align_w - w_remain);
        }
        else
            *crop_w -= w_remain;
    }

    unsigned int h_remain = (*crop_h & (align_h - 1));
    if (h_remain != 0) {
        if (  (align_h >> 1) <= h_remain
            && (int)(*crop_h + (align_h - h_remain)) <= src_h) {
            *crop_h += (align_h - h_remain);
        }
        else
            *crop_h -= h_remain;
    }

    *crop_x = (src_w - *crop_w) >> 1;
    *crop_y = (src_h - *crop_h) >> 1;

    if (*crop_x < 0 || *crop_y < 0) {
        CLOGE("ERR(%s):crop size too big (%d, %d, %d, %d)",
                 __func__, *crop_x, *crop_y, *crop_w, *crop_h);
        return false;
    }

    return true;
}

bool ExynosCamera::fileDump(char *filename,
                            char *srcBuf,
                            int w, int h, unsigned int size)
{
    FILE *yuvFd = NULL;
    char *buffer = NULL;

    yuvFd = fopen(filename, "w+");

    if (yuvFd == NULL) {
        CLOGE("ERR(%s):open(%s) fail",
            __func__, filename);
        return false;
    }

    buffer = (char *)malloc(size);

    if (buffer == NULL) {
        CLOGE("ERR malloc file");
        fclose(yuvFd);
        return false;
    }

    memcpy(buffer, srcBuf, size);

    fflush(stdout);

    fwrite(buffer, 1, size, yuvFd);

    fflush(yuvFd);

    if (yuvFd)
        fclose(yuvFd);
    if (buffer)
        free(buffer);

    CLOGD("DEBUG(%s):filedump(%s, w(%d)h(%d)size(%d) is successed!!",
            __func__, filename, w, h, size);

    return true;
}

void ExynosCamera::m_pushSensorQ(int index)
{
    Mutex::Autolock lock(m_requestMutex);
    m_sensorQ.push_back(index);
}

int ExynosCamera::m_popSensorQ(void)
{
   List<int>::iterator sensor_token;
   int index;

    Mutex::Autolock lock(m_requestMutex);

    if (m_sensorQ.size() == 0)
        return -1;

    sensor_token = m_sensorQ.begin()++;
    index = *sensor_token;
    m_sensorQ.erase(sensor_token);

    return (index);
}

void ExynosCamera::m_printSensorQ(void)
{
   List<int>::iterator token;
   int index;

    if (m_sensorQ.size() == 0) {
        CLOGW("WARN(%s):(%d) no entry", __func__, __LINE__);
        return;
    }

    CLOGE("[%s] (%d) m_sensorQ.size() = %d", __func__, __LINE__, m_sensorQ.size());
    for (token = m_sensorQ.begin(); token != m_sensorQ.end(); token++) {
        index = *token;
        CLOGE("[%s] (%d) index = %d", __func__, __LINE__, index);
    }

    return;
}

void ExynosCamera::m_releaseSensorQ(void)
{
    List<int>::iterator r;

    Mutex::Autolock lock(m_requestMutex);
    CLOGV("(%s)m_sensorQ.size : %d", __func__, m_sensorQ.size());

    while (m_sensorQ.size() > 0) {
        r = m_sensorQ.begin()++;
        m_sensorQ.erase(r);
    }
    return;
}

#if CAPTURE_BUF_GET
void ExynosCamera::m_setBayerQ(int index)
{
    Mutex::Autolock lock(m_bayerMutex);
    m_captureBayerQ.push_back(index);
}

int ExynosCamera::m_checkLastBayerQ(void)
{
   List<int>::iterator token;
   int index;

    if (m_captureBayerQ.size() == 0)
        return -1;

    token = m_captureBayerQ.begin()++;
    index = *token;

    return (index);
}

int ExynosCamera::m_getBayerQ(void)
{
   List<int>::iterator token;
   int index;

    Mutex::Autolock lock(m_bayerMutex);

    if (m_captureBayerQ.size() == 0)
        return -1;

    token = m_captureBayerQ.begin()++;
    index = *token;
    m_captureBayerQ.erase(token);

    return (index);
}

void ExynosCamera::m_printBayerQ(void)
{
   List<int>::iterator token;
   int index;

    if (m_captureBayerQ.size() == 0) {
        CLOGW("WARN(%s):(%d) no entry", __func__, __LINE__);
        return;
    }

    CLOGD("DEBUG(%s):(%d) m_captureBayerQ.size() = %d", __func__, __LINE__, m_captureBayerQ.size());
    for (token = m_captureBayerQ.begin(); token != m_captureBayerQ.end(); token++) {
        index = *token;
        CLOGD("DEBUG(%s):(%d) index = %d", __func__, __LINE__, index);
    }

    return;
}

void ExynosCamera::m_releaseBayerQ(void)
{
    List<int>::iterator r;

    Mutex::Autolock lock(m_bayerMutex);

    CLOGV("(%s)m_captureBayerQ.size : %d", __func__, m_captureBayerQ.size());

    while (m_captureBayerQ.size() > 0) {
        r = m_captureBayerQ.begin()++;
        m_captureBayerQ.erase(r);
    }
    return;
}
#endif

int ExynosCamera::m_getSensorId(int cameraId)
{
    int sensorId = -1;

    switch (cameraId) {
    case CAMERA_ID_BACK:
        sensorId = BACK_CAMERA_SENSOR_NAME;

        if (BACK_CAMERA_SENSOR_NAME != SENSOR_NAME_IMX135)
            CLOGE("ERR(%s):invalid camera sensor name(%d) fail", __func__, BACK_CAMERA_SENSOR_NAME);
        break;
    case CAMERA_ID_FRONT:
        sensorId = FRONT_CAMERA_SENSOR_NAME;
        break;
    default:
        CLOGE("ERR(%s):invalid cameraId(%d) fail", __func__, cameraId);
        break;
    }

    return sensorId;
}

void ExynosCamera::m_turnOffEffectByFps(camera2_shot_ext *shot_ext, int fps)
{
    if (30000 < fps) {
        // vdis off
        shot_ext->shot.ctl.aa.videoStabilizationMode = 0;
        shot_ext->dis_bypass = 1;

        // drc off
        shot_ext->drc_bypass = 1;

        // 3dnr off.
        shot_ext->dnr_bypass = 1;

        // odc off
    }

    if (60000 < fps) {
        // ae off
        //shot_ext->shot.ctl.aa.aeMode = ::AA_AEMODE_OFF;

        // af off
        //shot_ext->shot.ctl.aa.afMode = ::AA_AFMODE_OFF;

        // face detection off.
        shot_ext->shot.ctl.stats.faceDetectMode = ::FACEDETECT_MODE_OFF;
        shot_ext->fd_bypass = 1;
    }
}

ExynosRect2 ExynosCamera::m_AndroidArea2HWArea(ExynosRect2 *rect)
{
    int x = m_camera_info[m_cameraMode].dummy_shot.shot.ctl.scaler.cropRegion[0];
    int y = m_camera_info[m_cameraMode].dummy_shot.shot.ctl.scaler.cropRegion[1];
    int w = m_camera_info[m_cameraMode].dummy_shot.shot.ctl.scaler.cropRegion[2];
    int h = m_camera_info[m_cameraMode].dummy_shot.shot.ctl.scaler.cropRegion[3];

    ExynosRect2 newRect2;

    newRect2.x1 = (rect->x1 + 1000) * w / 2000;
    newRect2.y1 = (rect->y1 + 1000) * h / 2000;
    newRect2.x2 = (rect->x2 + 1000) * w / 2000;
    newRect2.y2 = (rect->y2 + 1000) * h / 2000;

    if (newRect2.x1 < 0)
        newRect2.x1 = 0;
    else if (w <= newRect2.x1)
        newRect2.x1 = w - 1;

    if (newRect2.y1 < 0)
        newRect2.y1 = 0;
    else if (h <= newRect2.y1)
        newRect2.y1 = h - 1;

    if (newRect2.x2 < 0)
        newRect2.x2 = 0;
    else if (w <= newRect2.x2)
        newRect2.x2 = w - 1;

    if (newRect2.y2 < 0)
        newRect2.y2 = 0;
    else if (h <= newRect2.y2)
        newRect2.y2 = h - 1;

    if (newRect2.x2 < newRect2.x1)
        newRect2.x2 = newRect2.x1;

    if (newRect2.y2 < newRect2.y1)
        newRect2.y2 = newRect2.y1;

    newRect2.x1 += x;
    newRect2.y1 += y;
    newRect2.x2 += x;
    newRect2.y2 += y;

    return newRect2;
}

ExynosRect2 ExynosCamera::m_AndroidArea2HWArea(ExynosRect2 *rect, int w, int h)
{
    ExynosRect2 newRect2;

    newRect2.x1 = (rect->x1 + 1000) * w / 2000;
    newRect2.y1 = (rect->y1 + 1000) * h / 2000;
    newRect2.x2 = (rect->x2 + 1000) * w / 2000;
    newRect2.y2 = (rect->y2 + 1000) * h / 2000;

    if (newRect2.x1 < 0)
        newRect2.x1 = 0;
    else if (w <= newRect2.x1)
        newRect2.x1 = w - 1;

    if (newRect2.y1 < 0)
        newRect2.y1 = 0;
    else if (h <= newRect2.y1)
        newRect2.y1 = h - 1;

    if (newRect2.x2 < 0)
        newRect2.x2 = 0;
    else if (w <= newRect2.x2)
        newRect2.x2 = w - 1;

    if (newRect2.y2 < 0)
        newRect2.y2 = 0;
    else if (h <= newRect2.y2)
        newRect2.y2 = h - 1;

    if (newRect2.x2 < newRect2.x1)
        newRect2.x2 = newRect2.x1;

    if (newRect2.y2 < newRect2.y1)
        newRect2.y2 = newRect2.y1;

    return newRect2;
}

bool ExynosCamera::m_startFaceDetection(enum CAMERA_MODE cameraMode, bool toggle)
{
    CLOGD("DEBUG(%s):(%d) cameraMode : %d, toggle : %d", __func__, __LINE__, (int)cameraMode, toggle);

    if (toggle == true) {
        m_camera_info[cameraMode].dummy_shot.shot.ctl.stats.faceDetectMode = FACEDETECT_MODE_FULL;
        m_camera_info[cameraMode].dummy_shot.fd_bypass = 0;
    } else {
        m_camera_info[cameraMode].dummy_shot.shot.ctl.stats.faceDetectMode = FACEDETECT_MODE_OFF;
        m_camera_info[cameraMode].dummy_shot.fd_bypass = 1;
    }

    return true;
}

bool ExynosCamera::m_getImageUniqueId(void)
{
    int fd = 0;
    struct v4l2_ext_controls ctrls;
    struct v4l2_ext_control ctrl;
    char uniqueIdBuf[UNIQUE_ID_BUF_SIZE] = {'\0',};

    memset(&ctrls, 0, sizeof(struct v4l2_ext_controls));
    memset(&ctrl, 0, sizeof(struct v4l2_ext_control));

    ctrls.ctrl_class = V4L2_CTRL_CLASS_CAMERA;
    ctrls.count = 1;
    ctrls.controls = &ctrl;

    ctrl.id = V4L2_CID_CAM_SENSOR_FW_VER;
    ctrl.string = uniqueIdBuf;

    int copySize = sizeof(mExifInfo.unique_id);
    if (UNIQUE_ID_BUF_SIZE < copySize)
        copySize = UNIQUE_ID_BUF_SIZE;

    if (m_cameraMode == CAMERA_MODE_BACK) {
        if (m_internalISP == true) {
            if (m_camera_info[m_cameraMode].isp.fd < 0) {
                CLOGW("WARN(%s):m_camera_info[m_cameraMode].isp.fd < 0", __func__);
                return false;
            }

            fd = m_camera_info[m_cameraMode].isp.fd;
        } else {
            CLOGE("ERR(%s):external isp not yet support", __func__);
            return false;
        }

        if (exynos_v4l2_g_ext_ctrl(fd, &ctrls) < 0) {
            CLOGE("ERR(%s):exynos_v4l2_g_ext_ctrl(V4L2_CID_CAM_SENSOR_FW_VER) fail", __func__);
            return false;
        } else {
            memcpy(m_imageUniqueIdBuf, uniqueIdBuf, copySize);
        }
    } else if (m_cameraMode == CAMERA_MODE_FRONT) {
        memcpy(m_imageUniqueIdBuf, "SLSI_S5K6B2", copySize);
    }

    return true;
}

bool ExynosCamera::getFlagFlashOn(void)
{
    bool ret = false;

    switch(m_camera_info[CAMERA_MODE_BACK].is3aa_dm.shot.dm.flash.flashMode) {
    case ::CAM2_FLASH_MODE_SINGLE:
    case ::CAM2_FLASH_MODE_TORCH:
        if (m_camera_info[CAMERA_MODE_BACK].is3aa_dm.shot.dm.flash.decision == 1) {
            ret = true;
            CLOGD("DEBUG(%s):frameCount   : %d",   __func__, m_camera_info[CAMERA_MODE_BACK].is3aa_dm.shot.dm.request.frameCount);
            CLOGD("DEBUG(%s):flashMode    : %d",   __func__, m_camera_info[CAMERA_MODE_BACK].is3aa_dm.shot.dm.flash.flashMode);
            CLOGD("DEBUG(%s):firingPower  : %d",   __func__, m_camera_info[CAMERA_MODE_BACK].is3aa_dm.shot.dm.flash.firingPower);
            CLOGD("DEBUG(%s):firingTime   : %lld", __func__, m_camera_info[CAMERA_MODE_BACK].is3aa_dm.shot.dm.flash.firingTime);
            CLOGD("DEBUG(%s):firingStable : %d",   __func__, m_camera_info[CAMERA_MODE_BACK].is3aa_dm.shot.dm.flash.firingStable);
            CLOGD("DEBUG(%s):decision     : %d",   __func__, m_camera_info[CAMERA_MODE_BACK].is3aa_dm.shot.dm.flash.decision);
        }
        else
            ret = false;
        break;
    default:
        break;
    }

    return ret;
}

# define NUM_BAYER_USE_RESERVED 4

bool ExynosCamera::startSensorReprocessing(void)
{
    CLOGD("DEBUG(%s):in", __func__);

    if (m_flagCreate == false) {
        CLOGE("ERR(%s):Not yet Created", __func__);
        return false;
    }

    Mutex::Autolock lock(m_sensorLockReprocessing);

    CLOGD("(flagStart %d), (%d x %d), format 0x%x", m_camera_info[CAMERA_MODE_REPROCESSING].sensor.flagStart,
        m_camera_info[CAMERA_MODE_REPROCESSING].sensor.width,
        m_camera_info[CAMERA_MODE_REPROCESSING].sensor.height,
        m_camera_info[CAMERA_MODE_REPROCESSING].sensor.format);

    if (m_camera_info[CAMERA_MODE_REPROCESSING].sensor.flagStart == false) {

        m_camera_info[CAMERA_MODE_REPROCESSING].sensor.width   = m_curCameraInfo[CAMERA_MODE_REPROCESSING]->pictureW + 16;
        m_camera_info[CAMERA_MODE_REPROCESSING].sensor.height  = m_curCameraInfo[CAMERA_MODE_REPROCESSING]->pictureH + 10;
        m_camera_info[CAMERA_MODE_REPROCESSING].sensor.format  = V4L2_PIX_FMT_SBGGR12;
        m_camera_info[CAMERA_MODE_REPROCESSING].sensor.planes  = 2;
        m_camera_info[CAMERA_MODE_REPROCESSING].sensor.buffers = NUM_BAYER_BUFFERS;
        m_camera_info[CAMERA_MODE_REPROCESSING].sensor.memory  = V4L2_CAMERA_MEMORY_TYPE;
        m_camera_info[CAMERA_MODE_REPROCESSING].sensor.type    = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
        m_camera_info[CAMERA_MODE_REPROCESSING].sensor.ionClient = m_ionCameraClient;

        ExynosBuffer nullBuf;
        int planeSize;

        planeSize = getPlaneSizePackedFLiteOutput((m_curCameraInfo[m_cameraMode]->pictureW + 16), (m_curCameraInfo[m_cameraMode]->pictureH + 10));

        for (int i = 0; i < m_camera_info[CAMERA_MODE_REPROCESSING].sensor.buffers; i++) {
            m_camera_info[CAMERA_MODE_REPROCESSING].sensor.buffer[i] = nullBuf;
            m_camera_info[CAMERA_MODE_REPROCESSING].sensor.buffer[i].size.extS[0] = planeSize;
            m_camera_info[CAMERA_MODE_REPROCESSING].sensor.buffer[i].size.extS[1] = META_DATA_SIZE;

            bool allocmem_ret;
            if (i < NUM_BAYER_USE_RESERVED) {
                CLOGE("Alloc reserved %d ", i);
                if (i == 0 || i == 1)
                    allocmem_ret = allocMemReserved(m_camera_info[CAMERA_MODE_REPROCESSING].sensor.ionClient, &m_camera_info[CAMERA_MODE_REPROCESSING].sensor.buffer[i], (1 << 1), ION_HEAP_EXYNOS_CONTIG_MASK, ION_EXYNOS_MFC_OUTPUT_MASK);
                else if (i == 2 || i==3)
                    allocmem_ret = allocMemReserved(m_camera_info[CAMERA_MODE_REPROCESSING].sensor.ionClient, &m_camera_info[CAMERA_MODE_REPROCESSING].sensor.buffer[i], (1 << 1), ION_HEAP_EXYNOS_CONTIG_MASK, ION_EXYNOS_FIMD_VIDEO_MASK);
                else
                    CLOGE("ERR Wrong index [%s] (%d)", __func__, __LINE__);
            } else {
                allocmem_ret = allocMem(m_camera_info[CAMERA_MODE_REPROCESSING].sensor.ionClient, &m_camera_info[CAMERA_MODE_REPROCESSING].sensor.buffer[i], 1 << 1);
            }

            if (allocmem_ret == false) {
                CLOGE("ERR(%s):allocMem() fail", __func__);
                return false;
            } else {
                memset(m_camera_info[CAMERA_MODE_REPROCESSING].sensor.buffer[i].virt.extP[1],
                        0, m_camera_info[CAMERA_MODE_REPROCESSING].sensor.buffer[i].size.extS[1]);
            }
        }

        m_camera_info[CAMERA_MODE_REPROCESSING].sensor.flagStart = true;
    }

    return true;
}

bool ExynosCamera::stopSensorReprocessing(void)
{
    CLOGD("DEBUG(%s):in", __func__);

    if (m_flagCreate == false) {
        CLOGW("WARN(%s):Not yet Created", __func__);
        return false;
    }

    Mutex::Autolock lock(m_sensorLockReprocessing);

    if (m_camera_info[CAMERA_MODE_REPROCESSING].sensor.flagStart == true) {
        m_isFirtstSensorStartReprocessing = true;

        for (int i = 0; i < NUM_BAYER_BUFFERS; i++) {
            freeMem(&m_camera_info[CAMERA_MODE_REPROCESSING].sensor.buffer[i]);
            CLOGV("ERR(%s):freeMem sensor (%d)", __func__, m_camera_info[CAMERA_MODE_REPROCESSING].sensor.buffers);
         }

        for (int i = 0; i < VIDEO_MAX_FRAME; i++)
            freeMemSinglePlane(&m_camera_info[CAMERA_MODE_REPROCESSING].sensor.buffer[i], m_camera_info[CAMERA_MODE_REPROCESSING].sensor.planes - 1);

        m_camera_info[CAMERA_MODE_REPROCESSING].sensor.flagStart = false;
    }

    return true;
}

bool ExynosCamera::getSensorBufReprocessing(ExynosBuffer *buf)
{
    return true;
}

bool ExynosCamera::putSensorBufReprocessing(ExynosBuffer *buf)
{
    if (m_flagCreate == false) {
        CLOGE("ERR(%s):Not yet Created", __func__);
        return false;
    }

    camera2_shot_ext *shot_ext;
    camera2_shot_ext *shot_ext2;

    Mutex::Autolock lock(m_sensorLockReprocessing);

    shot_ext = (struct camera2_shot_ext *)(m_camera_info[CAMERA_MODE_BACK].isp.buffer[buf->reserved.p].virt.extP[1]);
    shot_ext2 = (struct camera2_shot_ext *)(m_camera_info[CAMERA_MODE_REPROCESSING].isp.buffer[buf->reserved.p].virt.extP[1]);

    memcpy(shot_ext2, shot_ext, sizeof(struct camera2_shot_ext));

    shot_ext2->request_scc = m_camera_info[CAMERA_MODE_REPROCESSING].dummy_shot.request_scc;
    shot_ext2->request_scp = 0;
    shot_ext2->request_dis = 0;
    shot_ext2->shot.ctl.request.frameCount = buf->reserved.extP[FRAME_COUNT_INDEX];

    shot_ext2->dis_bypass = 1;
    shot_ext2->dnr_bypass = 1;
    shot_ext2->fd_bypass = 1;

    /* isp q */
    if (cam_int_qbuf(&(m_camera_info[CAMERA_MODE_REPROCESSING].isp), buf->reserved.p) < 0) {
        CLOGE("ERR(%s):cam_int_qbuf(%d) on isp fail", __func__, buf->reserved.p);
        return false;
    }

    /* isp dq */
    int index_isp = 0;
    index_isp = cam_int_dqbuf(&(m_camera_info[CAMERA_MODE_REPROCESSING].isp));
    if (index_isp < 0) {
        CLOGE("ERR(%s):cam_int_dqbuf() on isp fail", __func__);
        return false;
    }

    return true;
}

bool ExynosCamera::flagStartSensorReprocessing(void)
{
    return m_camera_info[CAMERA_MODE_REPROCESSING].sensor.flagStart;
}

bool ExynosCamera::startPreviewReprocessing(void)
{
    CLOGD("DEBUG(%s):in", __func__);

    if (m_flagCreate == false) {
        CLOGE("ERR(%s):Not yet Created", __func__);
        return false;
    }

    if (m_recordingHint == true && getCameraId() == CAMERA_ID_BACK) {
        if (set3DNR(true) == false)
            CLOGE("ERR(%s):set3DNR() fail", __func__);
    }

    CLOGD("(flagStart %d), (%d x %d), format 0x%x", m_camera_info[CAMERA_MODE_REPROCESSING].preview.flagStart,
        m_camera_info[CAMERA_MODE_REPROCESSING].preview.width,
        m_camera_info[CAMERA_MODE_REPROCESSING].preview.height,
        m_camera_info[CAMERA_MODE_REPROCESSING].preview.format);

    if (m_camera_info[CAMERA_MODE_REPROCESSING].preview.flagStart == false) {
        m_camera_info[CAMERA_MODE_REPROCESSING].preview.width  = m_curCameraInfo[CAMERA_MODE_REPROCESSING]->previewW;
        m_camera_info[CAMERA_MODE_REPROCESSING].preview.height = m_curCameraInfo[CAMERA_MODE_REPROCESSING]->previewH;
        m_camera_info[CAMERA_MODE_REPROCESSING].preview.format = m_curCameraInfo[CAMERA_MODE_REPROCESSING]->previewColorFormat;
        m_camera_info[CAMERA_MODE_REPROCESSING].preview.planes = m_curCameraInfo[m_cameraMode]->previewBufPlane;
        m_camera_info[CAMERA_MODE_REPROCESSING].preview.memory = V4L2_CAMERA_MEMORY_TYPE;
        m_camera_info[CAMERA_MODE_REPROCESSING].preview.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
        m_camera_info[CAMERA_MODE_REPROCESSING].preview.ionClient = m_ionCameraClient;
        m_camera_info[CAMERA_MODE_REPROCESSING].preview.buffers = NUM_PREVIEW_BUFFERS;

        int sensorId = m_getSensorId(m_cameraMode);
        sensorId = (1 << REPROCESSING_SHFIT) | ((FIMC_IS_VIDEO_SCP_NUM - FIMC_IS_VIDEO_SEN0_NUM) << VIDEO_INDEX_SHFIT) | (sensorId << 0);

        if (cam_int_s_input(&(m_camera_info[CAMERA_MODE_REPROCESSING].preview), sensorId) < 0) {
            CLOGE("ERR(%s):cam_int_s_input(%d) fail", __func__, sensorId);
            return false;
        }

        if (cam_int_s_fmt(&m_camera_info[CAMERA_MODE_REPROCESSING].preview) < 0) {
            CLOGE("ERR(%s):cam_int_s_fmt() fail", __func__);
            return false;
        }

        /* This from startIsp */
        if (cam_int_streamon(&(m_camera_info[CAMERA_MODE_REPROCESSING].isp)) < 0) {
            CLOGE("ERR(%s):cam_int_streamon(isp) fail", __func__);
            return false;
        }

        bool toggle = getVideoStabilization();
        if (setVideoStabilization(toggle) == false)
            CLOGE("ERR(%s):setVideoStabilization() fail", __func__);

        m_camera_info[CAMERA_MODE_REPROCESSING].preview.flagStart = true;
    }

    return true;
}

bool ExynosCamera::stopPreviewReprocessing(void)
{
    CLOGD("DEBUG(%s):in", __func__);

    if (m_flagCreate == false) {
        CLOGW("WARN(%s):Not yet Created", __func__);
        return false;
    }

    if (m_camera_info[CAMERA_MODE_REPROCESSING].preview.flagStart == true) {
        if (setFlashMode(FLASH_MODE_OFF) == false)
            CLOGE("ERR(%s):setFlashMode() fail", __func__);

        if (getVideoStabilization() == true) {
            if (setVideoStabilization(false) == false)
                CLOGE("ERR(%s):setVideoStabilization() fail", __func__);
        }

        if (set3DNR(false) == false)
            CLOGE("ERR(%s):set3DNR() fail", __func__);

        m_camera_info[CAMERA_MODE_REPROCESSING].preview.flagStart = false;
    }

    return true;
}

bool ExynosCamera::startIspReprocessing(void)
{
    CLOGD("DEBUG(%s):in", __func__);

    CLOGD("(flagStart %d), (%d x %d), format 0x%x", m_camera_info[CAMERA_MODE_REPROCESSING].isp.flagStart,
        m_camera_info[CAMERA_MODE_REPROCESSING].isp.width,
        m_camera_info[CAMERA_MODE_REPROCESSING].isp.height,
        m_camera_info[CAMERA_MODE_REPROCESSING].isp.format);

    if (m_camera_info[CAMERA_MODE_REPROCESSING].isp.flagStart == false) {
        m_camera_info[CAMERA_MODE_REPROCESSING].isp.width   = m_curCameraInfo[CAMERA_MODE_REPROCESSING]->ispW;
        m_camera_info[CAMERA_MODE_REPROCESSING].isp.height  = m_curCameraInfo[CAMERA_MODE_REPROCESSING]->ispH;
        m_camera_info[CAMERA_MODE_REPROCESSING].isp.format  = V4L2_PIX_FMT_SBGGR12;
        m_camera_info[CAMERA_MODE_REPROCESSING].isp.planes  = 2;
        m_camera_info[CAMERA_MODE_REPROCESSING].isp.buffers = 1;
        m_camera_info[CAMERA_MODE_REPROCESSING].isp.memory  = V4L2_CAMERA_MEMORY_TYPE;
        m_camera_info[CAMERA_MODE_REPROCESSING].isp.type    = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
        m_camera_info[CAMERA_MODE_REPROCESSING].isp.ionClient = m_ionCameraClient;

        ExynosBuffer nullBuf;

        for (int i = 0; i < m_camera_info[CAMERA_MODE_REPROCESSING].isp.buffers; i++) {
            m_camera_info[CAMERA_MODE_REPROCESSING].isp.buffer[i] = nullBuf;
            m_camera_info[CAMERA_MODE_REPROCESSING].isp.buffer[i].size.extS[0] = m_curCameraInfo[CAMERA_MODE_REPROCESSING]->ispW * m_curCameraInfo[CAMERA_MODE_REPROCESSING]->ispH * 2;
            m_camera_info[CAMERA_MODE_REPROCESSING].isp.buffer[i].size.extS[1] = META_DATA_SIZE;

            if (allocMem(m_camera_info[CAMERA_MODE_REPROCESSING].isp.ionClient, &m_camera_info[CAMERA_MODE_REPROCESSING].isp.buffer[i], 1 << 1) == false) {
                CLOGE("ERR(%s):allocMem() fail", __func__);
                goto err;
            } else {
                memset(m_camera_info[CAMERA_MODE_REPROCESSING].isp.buffer[i].virt.extP[1],
                        0, m_camera_info[CAMERA_MODE_REPROCESSING].isp.buffer[i].size.extS[1]);
            }
        }

        if (m_startIspReprocessingOn() == false) {
            CLOGE("ERR(%s):m_startIspReprocessingOn() fail", __func__);
            goto err;
        }

        m_camera_info[CAMERA_MODE_REPROCESSING].isp.flagStart = true;
    }

    return true;

err:
    if (stopIspReprocessing() == false)
        CLOGE("ERR(%s):stopIspReprocessing() fail", __func__);

    return false;
}

bool ExynosCamera::m_startIspReprocessingOn(void)
{
    CLOGD("DEBUG(%s):in", __func__);

    // Reset Dummy Shot's request
    m_camera_info[CAMERA_MODE_REPROCESSING].default_shot.request_3ax = 1;
    m_camera_info[CAMERA_MODE_REPROCESSING].default_shot.request_isp = 1;
    m_camera_info[CAMERA_MODE_REPROCESSING].default_shot.request_scp = 0;
    m_camera_info[CAMERA_MODE_REPROCESSING].default_shot.request_scc = 0;
    m_camera_info[CAMERA_MODE_REPROCESSING].default_shot.request_dis = 0;

    if (m_isFirtstIspStartReprocessing == true) {
        int sensorId = m_getSensorId(m_cameraMode);
        sensorId = (1 << REPROCESSING_SHFIT) |
            ((FIMC_IS_VIDEO_SEN0_NUM - FIMC_IS_VIDEO_SEN0_NUM) << SENSOR_INDEX_SHFIT) |
            ((FIMC_IS_VIDEO_3A0_NUM - FIMC_IS_VIDEO_SEN0_NUM) << VIDEO_INDEX_SHFIT) |
            (sensorId << 0);

        if (cam_int_s_input(&(m_camera_info[CAMERA_MODE_REPROCESSING].isp), sensorId) < 0) {
            CLOGE("ERR(%s):cam_int_s_input(%d) fail", __func__, sensorId);
            return false;
        }
        m_isFirtstIspStartReprocessing= false;
    }

    if (cam_int_s_fmt(&(m_camera_info[CAMERA_MODE_REPROCESSING].isp)) < 0) {
        CLOGE("ERR(%s):cam_int_s_fmt() fail", __func__);
        return false;
    }

    if (cam_int_reqbufs(&(m_camera_info[CAMERA_MODE_REPROCESSING].isp)) < 0) {
        CLOGE("ERR(%s):cam_int_reqbufs() fail", __func__);
        return false;
    }

    return true;
}

bool ExynosCamera::m_stopIspReprocessingOff(void)
{
    CLOGD("DEBUG(%s):in", __func__);

    bool ret = true;

#ifdef THREAD_PROFILE
    timeUs = 0;
    gettimeofday(&mTimeStart, NULL);
#endif
    if (cam_int_streamoff(&m_camera_info[CAMERA_MODE_REPROCESSING].isp) < 0) {
        CLOGE("ERR(%s):exynos_v4l2_streamoff() fail", __func__);
        ret = false;
    }
#ifdef THREAD_PROFILE
    gettimeofday(&mTimeStop, NULL);
    timeUs = (mTimeStop.tv_sec*1000000 + mTimeStop.tv_usec) - (mTimeStart.tv_sec*1000000 + mTimeStart.tv_usec);
    CLOGD("DEBUG(%s):time_check elapsed time=(%ld)us", __func__, timeUs);
#endif

    if (0 < m_camera_info[CAMERA_MODE_REPROCESSING].isp.buffers) {
        if (cam_int_clrbufs(&m_camera_info[CAMERA_MODE_REPROCESSING].isp) < 0) {
            CLOGE("ERR(%s):cam_int_clrbufs() fail", __func__);
            ret = false;
        }
    }

    return ret;
}

bool ExynosCamera::stopIspReprocessing(void)
{
    CLOGD("DEBUG(%s):in", __func__);

    bool ret = true;

    if (m_camera_info[CAMERA_MODE_REPROCESSING].isp.flagStart == true) {
        if (m_stopIspReprocessingOff() == false) {
            CLOGE("ERR(%s):m_stopIspReprocessingOff() fail", __func__);
            ret = false;
        }

        if (0 < m_camera_info[CAMERA_MODE_REPROCESSING].isp.buffers) {
            for (int i = 0; i < m_camera_info[CAMERA_MODE_REPROCESSING].isp.buffers; i++)
                freeMem(&m_camera_info[CAMERA_MODE_REPROCESSING].isp.buffer[i]);

            m_camera_info[CAMERA_MODE_REPROCESSING].isp.buffers = 0;
        }

        m_camera_info[CAMERA_MODE_REPROCESSING].isp.flagStart = false;
    }

    return ret;
}

bool ExynosCamera::flagStartIspReprocessing(void)
{
    return m_camera_info[CAMERA_MODE_REPROCESSING].isp.flagStart;
}

bool ExynosCamera::startPictureReprocessing(void)
{
    CLOGD("DEBUG(%s):in", __func__);

    if (m_flagCreate == false) {
        CLOGE("ERR(%s):Not yet Created", __func__);
        return false;
    }

    CLOGD("(flagStart %d), (%d x %d), format 0x%x", m_camera_info[CAMERA_MODE_REPROCESSING].picture.flagStart,
        m_camera_info[CAMERA_MODE_REPROCESSING].picture.width,
        m_camera_info[CAMERA_MODE_REPROCESSING].picture.height,
        m_camera_info[CAMERA_MODE_REPROCESSING].picture.format);

    if (m_camera_info[CAMERA_MODE_REPROCESSING].picture.flagStart == false) {
        m_camera_info[CAMERA_MODE_REPROCESSING].picture.width   = m_curCameraInfo[CAMERA_MODE_REPROCESSING]->pictureW;
        m_camera_info[CAMERA_MODE_REPROCESSING].picture.height  = m_curCameraInfo[CAMERA_MODE_REPROCESSING]->pictureH;
        m_camera_info[CAMERA_MODE_REPROCESSING].picture.format  = m_curCameraInfo[CAMERA_MODE_REPROCESSING]->pictureColorFormat;
        m_camera_info[CAMERA_MODE_REPROCESSING].picture.planes  = NUM_CAPTURE_PLANE;
        m_camera_info[CAMERA_MODE_REPROCESSING].picture.buffers = NUM_PICTURE_BUFFERS;
        m_camera_info[CAMERA_MODE_REPROCESSING].picture.memory  = V4L2_CAMERA_MEMORY_TYPE;
        m_camera_info[CAMERA_MODE_REPROCESSING].picture.type    = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
        m_camera_info[CAMERA_MODE_REPROCESSING].picture.ionClient = m_ionCameraClient;

        /* for frame sync */
        for (int i = 0; i < VIDEO_MAX_FRAME; i++) {
            if (m_camera_info[CAMERA_MODE_REPROCESSING].picture.buffer[i].virt.p != NULL ||
                m_camera_info[CAMERA_MODE_REPROCESSING].picture.buffer[i].phys.p != 0) {
                m_camera_info[CAMERA_MODE_REPROCESSING].picture.buffer[i].size.extS[m_camera_info[CAMERA_MODE_REPROCESSING].picture.planes - 1]
                    = ALIGN(META_DATA_SIZE, PAGE_SIZE);

                if (allocMemSinglePlane(m_ionCameraClient,
                                        &m_camera_info[CAMERA_MODE_REPROCESSING].picture.buffer[i],
                                        m_camera_info[CAMERA_MODE_REPROCESSING].picture.planes - 1,
                                        true) == false) {
                    CLOGE("ERR(%s):m_allocCameraMemorySingle() fail", __func__);
                    goto err;
                } else {
                    memset(m_camera_info[CAMERA_MODE_REPROCESSING].picture.buffer[i].virt.extP[m_camera_info[CAMERA_MODE_REPROCESSING].picture.planes - 1],
                            0, m_camera_info[CAMERA_MODE_REPROCESSING].picture.buffer[i].size.extS[m_camera_info[CAMERA_MODE_REPROCESSING].picture.planes - 1]);
                }
            }
        }
        if (m_startPictureReprocessing() == false) {
            CLOGE("ERR(%s):m_startPictureReprocessing() fail", __func__);
            goto err;
        }

        m_camera_info[CAMERA_MODE_REPROCESSING].picture.flagStart = true;
    }

    return true;

err:
    if (stopPictureReprocessing() == false)
        CLOGE("ERR(%s):stopPictureReprocessing() fail", __func__);

    return false;
}

bool ExynosCamera::m_startPictureReprocessing(void)
{
    CLOGD("DEBUG(%s):in", __func__);

    if (m_flagCreate == false) {
        CLOGE("ERR(%s):Not yet Created", __func__);
        return false;
    }

    int sensorId = m_getSensorId(m_cameraMode);
    sensorId = (1 << REPROCESSING_SHFIT) | ((FIMC_IS_VIDEO_SCC_NUM - FIMC_IS_VIDEO_SEN0_NUM) << 16) | (sensorId << 0);

    if (cam_int_s_input(&(m_camera_info[CAMERA_MODE_REPROCESSING].picture), sensorId) < 0) {
        CLOGE("ERR(%s):cam_int_s_input(%d) fail", __func__, sensorId);
        return false;
    }

    if (cam_int_s_fmt(&(m_camera_info[CAMERA_MODE_REPROCESSING].picture)) < 0) {
        CLOGE("ERR(%s):cam_int_s_fmt() fail", __func__);
        return false;
    }

    if (cam_int_reqbufs(&(m_camera_info[CAMERA_MODE_REPROCESSING].picture)) < 0) {
        CLOGE("ERR(%s):cam_int_reqbufs() fail", __func__);
        return false;
    }

    for (int i = 0; i < VIDEO_MAX_FRAME; i++) {
        if (m_camera_info[CAMERA_MODE_REPROCESSING].picture.buffer[i].virt.p != NULL ||
            m_camera_info[CAMERA_MODE_REPROCESSING].picture.buffer[i].phys.p != 0) {
            if (cam_int_qbuf(&m_camera_info[CAMERA_MODE_REPROCESSING].picture, i) < 0) {
                CLOGE("ERR(%s):cam_int_qbuf(%d) fail", __func__, i);
                return false;
            }
        }
    }

    if (cam_int_streamon(&(m_camera_info[CAMERA_MODE_REPROCESSING].picture)) < 0) {
        CLOGE("ERR(%s):cam_int_streamon() fail", __func__);
        return false;
    }

    return true;
}

bool ExynosCamera::m_stopPictureReprocessing(void)
{
    CLOGD("DEBUG(%s):in", __func__);

    bool ret = true;

    camera2_shot_ext *shot_ext;

#ifdef THREAD_PROFILE
    timeUs = 0;
    gettimeofday(&mTimeStart, NULL);
#endif
    if (cam_int_streamoff(&m_camera_info[CAMERA_MODE_REPROCESSING].picture) < 0) {
        CLOGE("ERR(%s):exynos_v4l2_streamoff() fail", __func__);
        ret = false;
    }
#ifdef THREAD_PROFILE
    gettimeofday(&mTimeStop, NULL);
    timeUs = (mTimeStop.tv_sec*1000000 + mTimeStop.tv_usec) - (mTimeStart.tv_sec*1000000 + mTimeStart.tv_usec);
    CLOGD("DEBUG(%s):time_check elapsed time=(%ld)us", __func__, timeUs);
#endif

    m_camera_info[CAMERA_MODE_REPROCESSING].dummy_shot.request_scc = 0;

    if (0 < m_camera_info[CAMERA_MODE_REPROCESSING].picture.buffers) {
        if (cam_int_clrbufs(&m_camera_info[CAMERA_MODE_REPROCESSING].picture) < 0) {
            CLOGE("ERR(%s):cam_int_clrbufs() fail", __func__);
            ret = false;
        }
        m_camera_info[CAMERA_MODE_REPROCESSING].picture.buffers = 0;
    }

    return ret;
}

bool ExynosCamera::stopPictureReprocessing(void)
{
    CLOGD("DEBUG(%s):in", __func__);

    bool ret = true;

    if (m_flagCreate == false) {
        CLOGW("WARN(%s):Not yet Created", __func__);
        return false;
    }

    if (m_camera_info[CAMERA_MODE_REPROCESSING].picture.flagStart == true) {
        if (m_stopPictureReprocessing() == false) {
            CLOGE("ERR(%s):m_stopPictureReprocessing() fail", __func__);
            ret = false;
        }

        /* for frame sync */
        for (int i = 0; i < VIDEO_MAX_FRAME; i++) {
            freeMemSinglePlane(&m_camera_info[CAMERA_MODE_REPROCESSING].picture.buffer[i],
                    m_camera_info[CAMERA_MODE_REPROCESSING].picture.planes - 1);
        }

        m_camera_info[CAMERA_MODE_REPROCESSING].picture.flagStart = false;
    }

    return ret;
}

void ExynosCamera::pictureOnReprocessing(void)
{
    CLOGV("[%s] (%d)", __func__, __LINE__);

    m_camera_info[CAMERA_MODE_REPROCESSING].dummy_shot.request_scc = 1;

    return;
}

bool ExynosCamera::setPictureBufReprocessing(ExynosBuffer *buf)
{
    if (m_flagCreate == false) {
        CLOGE("ERR(%s):Not yet created fail", __func__);
        return false;
    }

    if (VIDEO_MAX_FRAME <= buf->reserved.p) {
        CLOGE("ERR(%s):index(%d) must smaller than %d", __func__, buf->reserved.p, VIDEO_MAX_FRAME);
        return false;
    }

    m_camera_info[CAMERA_MODE_REPROCESSING].picture.buffer[buf->reserved.p] = *buf;

    return true;
}

bool ExynosCamera::getPictureBufReprocessing(ExynosBuffer *buf)
{
    int index = 0;

    if (m_flagCreate == false) {
        CLOGE("ERR(%s):Not yet created fail", __func__);
        return false;
    }

    if (m_camera_info[CAMERA_MODE_REPROCESSING].picture.flagStart == false) {
        CLOGE("ERR(%s):Not yet picture started fail", __func__);
        return false;
    }

    index = cam_int_dqbuf(&(m_camera_info[CAMERA_MODE_REPROCESSING].picture));
    if (index < 0) {
        CLOGE("ERR(%s):cam_int_dqbuf() on picture fail", __func__);
        return false;
    }

    *buf = m_camera_info[CAMERA_MODE_REPROCESSING].picture.buffer[index];
    buf->reserved.p = index;

    return true;
}

bool ExynosCamera::putPictureBufReprocessing(ExynosBuffer *buf)
{
    if (m_flagCreate == false) {
        CLOGE("ERR(%s):Not yet created fail", __func__);
        return false;
    }

    if (m_camera_info[CAMERA_MODE_REPROCESSING].picture.flagStart == false) {
        CLOGE("ERR(%s):Not yet picture started fail", __func__);
        return false;
    }

    if (cam_int_qbuf(&(m_camera_info[CAMERA_MODE_REPROCESSING].picture), buf->reserved.p) < 0) {
        CLOGE("ERR(%s):cam_int_qbuf(%d) fail", __func__, buf->reserved.p);
        return false;
    }

    return true;
}

bool ExynosCamera::flagStartPictureReprocessing(void)
{
    return m_camera_info[CAMERA_MODE_REPROCESSING].picture.flagStart;
}

bool ExynosCamera::getPictureSizeReprocessing(int *w, int *h)
{
    *w = m_curCameraInfo[CAMERA_MODE_REPROCESSING]->pictureW;
    *h = m_curCameraInfo[CAMERA_MODE_REPROCESSING]->pictureH;
    return true;
}

bool ExynosCamera::getPreviewSizeReprocessing(int *w, int *h)
{
    *w = m_curCameraInfo[CAMERA_MODE_REPROCESSING]->pictureW;
    *h = m_curCameraInfo[CAMERA_MODE_REPROCESSING]->pictureH;
    return true;
}

bool ExynosCamera::flagStartPreviewReprocessing(void)
{
    return m_camera_info[CAMERA_MODE_REPROCESSING].preview.flagStart;
}

void ExynosCamera::StartStreamReprocessing()
{
    if (exynos_v4l2_s_ctrl(m_camera_info[CAMERA_MODE_REPROCESSING].sensor.fd, V4L2_CID_IS_S_STREAM, IS_ENABLE_STREAM) < 0)
        CLOGE("ERR(%s):exynos_v4l2_s_ctrl fail(IS_ENABLE_STREAM)", __func__);
}

bool ExynosCamera::allocMemSinglePlane(ion_client ionClient, ExynosBuffer *buf, int index, bool flagCache)
{
    if (ionClient == 0) {
        CLOGE("ERR(%s):ionClient is zero (%d)", __func__, ionClient);
        return false;
    }

    if (buf->size.extS[index] != 0) {
        int flagIon = (flagCache == true) ? ION_FLAG_CACHED : 0;

        /* HACK: For non-cacheable */
        buf->fd.extFd[index] = ion_alloc(ionClient, buf->size.extS[index], 0, ION_HEAP_SYSTEM_MASK, 0);
        if (buf->fd.extFd[index] <= 0) {
            CLOGE("ERR(%s):ion_alloc(%d, %d) fail", __func__, index, buf->size.extS[index]);
            buf->fd.extFd[index] = -1;
            freeMemSinglePlane(buf, index);
            return false;
        }

        buf->virt.extP[index] = (char *)ion_map(buf->fd.extFd[index], buf->size.extS[index], 0);
        if ((buf->virt.extP[index] == (char *)MAP_FAILED) || (buf->virt.extP[index] == NULL)) {
            CLOGE("ERR(%s):ion_map(%d) fail", __func__, buf->size.extS[index]);
            buf->virt.extP[index] = NULL;
            freeMemSinglePlane(buf, index);
            return false;
        }
    }

    return true;
}

bool ExynosCamera::allocMemSinglePlaneReserved(ion_client ionClient, ExynosBuffer *buf, int index, bool flagCache, unsigned int heap_mask, unsigned int flags)
{
    CLOGD("[%s] (%d) index=%d, size=%d", __func__, __LINE__,index, buf->size.extS[index]);
    if (ionClient == 0) {
        CLOGE("ERR(%s): ionClient is zero (%d)", __func__, ionClient);
        return false;
    }

    if (buf->size.extS[index] != 0) {
        int flagIon = (flagCache == true) ? ION_FLAG_CACHED : 0;

        /* HACK: For non-cacheable */
        buf->fd.extFd[index] = ion_alloc(ionClient, buf->size.extS[index], 0, heap_mask, flags);
        if (buf->fd.extFd[index] <= 0) {
            CLOGE("ERR(%s): ion_alloc(%d, %d) failed", __func__, index, buf->size.extS[index]);
            buf->fd.extFd[index] = -1;
            freeMemSinglePlane(buf, index);
            return false;
        }

        buf->virt.extP[index] = (char *)ion_map(buf->fd.extFd[index], buf->size.extS[index], 0);
        if ((buf->virt.extP[index] == (char *)MAP_FAILED) || (buf->virt.extP[index] == NULL)) {
            CLOGE("ERR(%s): ion_map(%d) failed", __func__, buf->size.extS[index]);
            buf->virt.extP[index] = NULL;
            freeMemSinglePlane(buf, index);
            return false;
        }
    } else {
        CLOGD(" Do NOT allocate size is %d of index %d ", buf->size.extS[index], index);
    }

    return true;
}

void ExynosCamera::freeMemSinglePlane(ExynosBuffer *buf, int index)
{
    int ret = 0;

    if (0 < buf->fd.extFd[index]) {
        if (buf->virt.extP[index] != NULL) {
            ret = ion_unmap(buf->virt.extP[index], buf->size.extS[index]);
            if (ret < 0)
                CLOGE("ERR(%s):ion_unmap(%p, %d) fail", __func__, buf->virt.extP[index], buf->size.extS[index]);
        }
        ion_free(buf->fd.extFd[index]);
    }

    buf->fd.extFd[index] = -1;
    buf->virt.extP[index] = NULL;
    buf->size.extS[index] = 0;
}

bool ExynosCamera::allocMem(ion_client ionClient, ExynosBuffer *buf, int cacheIndex)
{
    for (int i = 0; i < ExynosBuffer::BUFFER_PLANE_NUM_DEFAULT; i++) {
        bool flagCache = ((1 << i) & cacheIndex) ? true : false;
        if (allocMemSinglePlane(ionClient, buf, i, flagCache) == false) {
            freeMem(buf);
            CLOGE("ERR(%s):allocMemSinglePlane(%d) fail", __func__, i);
            return false;
        }
    }

    return true;
}

bool ExynosCamera::allocMemReserved(ion_client ionClient, ExynosBuffer *buf, int cacheIndex, unsigned int heap_mask, unsigned int flags)
{
    CLOGD("[%s] (%d)", __func__, __LINE__);
    for (int i = 0; i < ExynosBuffer::BUFFER_PLANE_NUM_DEFAULT; i++) {
        bool flagCache = ((1 << i) & cacheIndex) ? true : false;
        if (allocMemSinglePlaneReserved(ionClient, buf, i, flagCache, heap_mask, flags) == false) {
            freeMem(buf);
            CLOGE("ERR(%s):allocMemSinglePlaneReserved(%d) fail", __func__, i);
            return false;
        }
    }

    return true;
}

bool ExynosCamera::allocMemSinglePlaneCache(ion_client ionClient, ExynosBuffer *buf, int index, bool flagCache)
{
    if (ionClient == 0) {
        CLOGE("ERR(%s):ionClient is zero (%d)", __func__, ionClient);
        return false;
    }

    if (buf->size.extS[index] != 0) {
        int flagIon = (flagCache == true) ? ION_FLAG_CACHED : 0;

        buf->fd.extFd[index] = ion_alloc(ionClient, buf->size.extS[index], 0, ION_HEAP_SYSTEM_MASK,
            ION_FLAG_CACHED | ION_FLAG_CACHED_NEEDS_SYNC | ION_FLAG_PRESERVE_KMAP);

        if (buf->fd.extFd[index] <= 0) {
            CLOGE("ERR(%s):ion_alloc(%d, %d) fail", __func__, index, buf->size.extS[index]);
            buf->fd.extFd[index] = -1;
            freeMemSinglePlane(buf, index);
            return false;
        }

        buf->virt.extP[index] = (char *)ion_map(buf->fd.extFd[index], buf->size.extS[index], 0);
        if ((buf->virt.extP[index] == (char *)MAP_FAILED) || (buf->virt.extP[index] == NULL)) {
            CLOGE("ERR(%s):ion_map(%d) fail", __func__, buf->size.extS[index]);
            buf->virt.extP[index] = NULL;
            freeMemSinglePlane(buf, index);
            return false;
        }
    }

    return true;
}

bool ExynosCamera::allocMemCache(ion_client ionClient, ExynosBuffer *buf, int cacheIndex)
{
    for (int i = 0; i < ExynosBuffer::BUFFER_PLANE_NUM_DEFAULT; i++) {
        bool flagCache = ((1 << i) & cacheIndex) ? true : false;
        if (allocMemSinglePlaneCache(ionClient, buf, i, flagCache) == false) {
            freeMem(buf);
            CLOGE("ERR(%s):allocMemSinglePlane(%d) fail", __func__, i);
            return false;
        }
    }

    return true;
}

void ExynosCamera::freeMem(ExynosBuffer *buf)
{
    for (int i = 0; i < ExynosBuffer::BUFFER_PLANE_NUM_DEFAULT; i++)
        freeMemSinglePlane(buf, i);
}

ion_client ExynosCamera::getIonClient(void)
{
    return m_ionCameraClient;
}

ExynosCameraActivityFlash *ExynosCamera::getFlashMgr(void)
{
    return m_flashMgr;
}

ExynosCameraActivitySpecialCapture *ExynosCamera::getSpecialCaptureMgr(void)
{
    return m_sCaptureMgr;
}

ExynosBuffer *ExynosCamera::searchSensorBuffer(unsigned int fcount)
{
    ExynosBuffer targetBuffer;
    camera2_shot_ext *shot_ext = NULL;

    CLOGV("[%s] (%d) fcount  %d", __func__, __LINE__, fcount);

    for (int i = 0; i < NUM_BAYER_BUFFERS; i++) {
        targetBuffer = m_camera_info[CAMERA_MODE_REPROCESSING].sensor.buffer[i];
        targetBuffer.reserved.p = i;

        shot_ext = (struct camera2_shot_ext *)(targetBuffer.virt.extP[1]);
        if (!shot_ext) {
            CLOGE("[%s] (%d) shot_ext is null and fcount  %d", __func__, __LINE__, fcount);

            continue;
        }

        if (shot_ext->shot.dm.request.frameCount == fcount) {
            CLOGD("DEBUG(%s):(%d) HIT fcount  %d", __func__, __LINE__, fcount);
            return &(m_camera_info[CAMERA_MODE_REPROCESSING].sensor.buffer[i]);
        }
    }

    return NULL;
}

ExynosBuffer *ExynosCamera::searchSensorBufferOnHal(unsigned int fcount)
{
    ExynosBuffer targetBuffer;
    camera2_shot_ext *shot_ext = NULL;

    CLOGV("[%s] (%d) fcount  %d", __func__, __LINE__, fcount);

    for (int i = 0; i < NUM_BAYER_BUFFERS; i++) {
        targetBuffer = m_camera_info[CAMERA_MODE_REPROCESSING].sensor.buffer[i];
        targetBuffer.reserved.p = i;

        shot_ext = (struct camera2_shot_ext *)(targetBuffer.virt.extP[1]);
        if (!shot_ext) {
            CLOGE("[%s] (%d) shot_ext is null and fcount  %d", __func__, __LINE__, fcount);

            continue;
        }

        if (shot_ext->shot.dm.request.frameCount == fcount) {
            if (checkCaptureBayerOnHAL(i)) {
                CLOGD("DEBUG(%s):(%d) HIT fcount  %d", __func__, __LINE__, fcount);
                return &(m_camera_info[CAMERA_MODE_REPROCESSING].sensor.buffer[i]);
            } else {
                int findIndex = -1;
                int findFrameCnt = -1;
                ExynosBuffer tmpBuffer;
                camera2_shot_ext *tmp_shot_ext = NULL;

                for (int j = 0; j < NUM_BAYER_BUFFERS; j++) {
                    if (m_captureBayerIndex[j] == 2) {
                        if (findIndex == -1) {
                            tmpBuffer = m_camera_info[CAMERA_MODE_REPROCESSING].sensor.buffer[j];
                            tmp_shot_ext = (struct camera2_shot_ext *)(tmpBuffer.virt.extP[1]);
                            if (!tmp_shot_ext) {
                                CLOGE("[%s] (%d) shot_ext is null and findIndex %d", __func__, __LINE__, findIndex);
                                continue;
                            }

                            findIndex = j;
                            findFrameCnt = tmp_shot_ext->shot.dm.request.frameCount;
                        } else {
                            tmpBuffer = m_camera_info[CAMERA_MODE_REPROCESSING].sensor.buffer[j];
                            tmp_shot_ext = (struct camera2_shot_ext *)(tmpBuffer.virt.extP[1]);
                            if (!tmp_shot_ext) {
                                CLOGE("[%s] (%d) shot_ext is null and findIndex %d", __func__, __LINE__, findIndex);
                                continue;
                            }

                            if (findFrameCnt < tmp_shot_ext->shot.dm.request.frameCount) {
                                findIndex = j;
                                findFrameCnt = tmp_shot_ext->shot.dm.request.frameCount;
                            }
                        }
                    }
                }
                if (findIndex == -1)
                    findIndex = i;

                CLOGW("WARN(%s):buffer(fcount %d) is not DQed. select buffer(fcount %d)", __func__, fcount, findFrameCnt);
                printBayerLockStatus();
                return &(m_camera_info[CAMERA_MODE_REPROCESSING].sensor.buffer[findIndex]);
            }
        }
    }

    return NULL;
}

#if CAPTURE_BUF_GET
bool ExynosCamera::setBayerLockIndex(int index, bool setLock)
{
    CLOGD("[%s], (%d) index %d setLock %d", __func__, __LINE__, index, setLock);

    m_captureBayerLock[index] = setLock;

    return true;
}

bool ExynosCamera::setBayerLock(unsigned int fcount, bool setLock)
{
    ExynosBuffer targetBuffer;
    camera2_shot_ext *shot_ext = NULL;

    CLOGV("[%s] (%d) fcount  %d", __func__, __LINE__, fcount);

    for (int i = 0; i < NUM_BAYER_BUFFERS; i++) {
        targetBuffer = m_camera_info[CAMERA_MODE_REPROCESSING].sensor.buffer[i];

        shot_ext = (struct camera2_shot_ext *)(targetBuffer.virt.extP[1]);
        if (shot_ext->shot.dm.request.frameCount == fcount) {
            setBayerLockIndex(i, setLock);

            return true;
        }
    }

    return false;
}

void ExynosCamera::printBayerLockStatus()
{
    ExynosBuffer targetBuffer;
    camera2_shot_ext *shot_ext = NULL;

    for (int i = 0; i < NUM_BAYER_BUFFERS; i++) {
        targetBuffer = m_camera_info[CAMERA_MODE_REPROCESSING].sensor.buffer[i];
        shot_ext = (struct camera2_shot_ext *)(targetBuffer.virt.extP[1]);

        if (!shot_ext) {
            CLOGE("[%s] (%d) shot_ext is null", __func__, __LINE__);

            continue;
        }

        CLOGD("DEBUG(%s):(%d) [%d] [lock %d] [bayer %d] [fcount %d] [fd %d %d]", __func__, __LINE__,
            i, m_captureBayerLock[i], m_captureBayerIndex[i], shot_ext->shot.dm.request.frameCount, targetBuffer.fd.extFd[0], targetBuffer.fd.extFd[1]);
    }

    return;
}

bool ExynosCamera::checkCaptureBayerOnHAL(int index)
{
    if (m_captureBayerIndex[index] == 2)
        return true;
    else if (m_captureBayerIndex[index] == 1)
        CLOGW("WRN(%s): capture buffer is not DQed", __func__);
    else
        CLOGE("ERR(%s):captuer buffer is not yet Qbuf", __func__);

    return false;
}

#endif

bool ExynosCamera::fileDump(char *filename, char *srcBuf, unsigned int size)
{
    FILE *yuvFd = NULL;
    char *buffer = NULL;

    yuvFd = fopen(filename, "w+");

    if (yuvFd == NULL) {
        CLOGE("ERR(%s):open(%s) fail",
            __func__, filename);
        return false;
    }

    buffer = (char *)malloc(size);

    if (buffer == NULL) {
        CLOGE("ERR malloc file");
        fclose(yuvFd);
        return false;
    }

    memcpy(buffer, srcBuf, size);

    fflush(stdout);

    fwrite(buffer, 1, size, yuvFd);

    fflush(yuvFd);

    if (yuvFd)
        fclose(yuvFd);
    if (buffer)
        free(buffer);

    CLOGD("DEBUG(%s):filedump(%s, size(%d) is successed!!",
            __func__, filename, size);

    return true;
}

bool ExynosCamera::setSensorStreamOff(enum CAMERA_MODE cameraMode)
{
    /* sensor stream off */
    CLOGV("DEBUG(%s): sensor stream off", __func__);

    if (exynos_v4l2_s_ctrl(m_camera_info[cameraMode].sensor.fd, V4L2_CID_IS_S_STREAM, IS_DISABLE_STREAM) < 0) {
        CLOGE("ERR(%s):exynos_v4l2_s_ctrl fail(IS_ENABLE_STREAM)", __func__);
        return false;
    }
    return true;
}

bool ExynosCamera::setSensorStreamOn(enum CAMERA_MODE cameraMode,  int width, int height, bool isSetFps)
{
    /* sensor */
    m_camera_info[cameraMode].sensor.width   = width + 16;
    m_camera_info[cameraMode].sensor.height  = height + 10;

    if (isSetFps) {
        int fps = m_camera_info[m_cameraMode].dummy_shot.shot.ctl.aa.aeTargetFpsRange[1];
        CLOGD("DEBUG(%s):fps(%d)", __func__, fps);
        if (setFPSParam(fps) < 0) {
            CLOGE("ERR(%s):setFPSParam(%d) fail", __func__, fps);
            return false;
        }
    }

    struct v4l2_crop crop;
    crop.c.top    = 0;
    crop.c.left   = 0;
    crop.c.width  = width + 16;
    crop.c.height = height + 10;
    crop.type    = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    if (exynos_v4l2_s_crop(m_camera_info[cameraMode].sensor.fd, &crop) < 0) {
        CLOGE("ERR(%s):sensor s_crop fail",  __func__);
        return false;
    }

    m_camera_info[m_cameraMode].dummy_shot.request_isp = 1;
    m_camera_info[m_cameraMode].dummy_shot.request_scp = 1;
    m_camera_info[m_cameraMode].dummy_shot.request_scc = 0;
    return true;
}

int ExynosCamera::setFPSParam(int fps)
{
    struct v4l2_streamparm stream_parm;

    memset(&stream_parm, 0x0, sizeof(v4l2_streamparm));
    stream_parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    stream_parm.parm.capture.timeperframe.numerator = 1;
    stream_parm.parm.capture.timeperframe.denominator = fps;

    CLOGV("DEBUG(%s):d/n(%d/%d)", __func__,
        stream_parm.parm.capture.timeperframe.numerator, stream_parm.parm.capture.timeperframe.denominator);
    if (exynos_v4l2_s_parm(m_camera_info[m_cameraMode].sensor.fd, &stream_parm) < 0) {
        CLOGE("ERR(%s):exynos_v4l2_s_parm() fail", __func__);
        return -1;
    }
    return 1;
}

#ifdef FRONT_NO_ZSL
void ExynosCamera::setFrontCaptureCmd(int captureCmd)
{
    m_frontCaptureStatus = captureCmd;

    return;
}
#endif

#ifdef SCALABLE_SENSOR
/* for scalable sensor in capture */
bool ExynosCamera::setScalableSensorSize(enum SCALABLE_SENSOR_SIZE sizeMode)
{
    CLOGD("DEBUG(%s):start", __func__);

    if (m_flagCreate == false) {
        CLOGE("ERR(%s):Not yet Created", __func__);
        return false;
    }

#ifdef SCALABLE_SENSOR_FORCE_DONE
    /* 1. Force Done */
    CLOGD("DEBUG(%s):SCALABLE_SENSOR_FORCE_DONE_3A1", __func__);
    if (exynos_v4l2_s_ctrl(m_camera_info[CAMERA_MODE_BACK].is3a1Src.fd, V4L2_CID_IS_FORCE_DONE, 1) < 0) {
        CLOGE("ERR(%s):exynos_v4l2_s_ctrl force_done fail(is3a1Src)", __func__);
        return false;
    }

    if (exynos_v4l2_s_ctrl(m_camera_info[CAMERA_MODE_BACK].is3a1Dst.fd, V4L2_CID_IS_FORCE_DONE, 1) < 0) {
        CLOGE("ERR(%s):exynos_v4l2_s_ctrl force_done fail(is3a1Dst)", __func__);
        return false;
    }
#ifdef SCALABLE_SENSOR_CTRL_ISP
    CLOGD("DEBUG(%s):SCALABLE_SENSOR_FORCE_DONE_ISP", __func__);
    if (exynos_v4l2_s_ctrl(m_camera_info[CAMERA_MODE_BACK].isp.fd, V4L2_CID_IS_FORCE_DONE, 1) < 0) {
        CLOGE("ERR(%s):exynos_v4l2_s_ctrl force_done fail(isp)", __func__);
        return false;
    }
#endif
#endif

    /* 2. 3A1, ISP stream off */
    CLOGD("DEBUG(%s):3A1 STREAM OFF", __func__);
    if (cam_int_streamoff(&(m_camera_info[CAMERA_MODE_BACK].is3a1Src)) < 0) {
        CLOGE("ERR(%s):cam_int_streamoff(3a1 output) fail", __func__);
        return false;
    }

    if (cam_int_streamoff(&(m_camera_info[CAMERA_MODE_BACK].is3a1Dst)) < 0) {
        CLOGE("ERR(%s):cam_int_streamoff(3a1 capture) fail", __func__);
        return false;
    }

#ifdef SCALABLE_SENSOR_CTRL_ISP
    CLOGD("DEBUG(%s):ISP STREAM OFF", __func__);
    if (cam_int_streamoff(&(m_camera_info[CAMERA_MODE_BACK].isp)) < 0) {
        CLOGE("ERR(%s):cam_int_streamoff(isp) fail", __func__);
        return false;
    }
#endif

    CLOGD("DEBUG(%s):SENSOR STREAM OFF", __func__);
    if (cam_int_streamoff(&(m_camera_info[CAMERA_MODE_BACK].sensor)) < 0) {
        CLOGE("ERR(%s):cam_int_streamoff(sensor) fail", __func__);
        return false;
    }

    /* 3. s_ctrl stream off */
    CLOGD("DEBUG(%s):S_CTRL STREAM OFF", __func__);
    if (setSensorStreamOff(CAMERA_MODE_REPROCESSING) == false) {
        CLOGE("ERR(%s):setSensorStreamOff() fail", __func__);
        return false;
    }

    /* 4. init var and setting zoom */
    m_numOfShotedFrame = 0;
    m_numOfShotedIspFrame = 0;
    m_is3a1DstLastBufIndex = -1;
    m_is3a1SrcLastBufIndex = -1;

    int zoom, srcW, srcH, dstW, dstH;
    zoom = m_curCameraInfo[m_cameraMode]->zoom;
    srcW = m_curCameraInfo[m_cameraMode]->pictureW;
    srcH = m_curCameraInfo[m_cameraMode]->pictureH;
    dstW = m_curCameraInfo[m_cameraMode]->ispW;
    dstH = m_curCameraInfo[m_cameraMode]->ispH;

    int width, height;
    switch (sizeMode) {
    case SCALABLE_SENSOR_SIZE_13M:
        width  = m_curCameraInfo[m_cameraMode]->pictureW;
        height = m_curCameraInfo[m_cameraMode]->pictureH;
        break;
    case SCALABLE_SENSOR_SIZE_FHD:
        getScalableSensorSizeOnPreview(&width, &height);
        srcW = width;
        srcH = height;
        break;
    default:
        CLOGE("ERR(%s):not defiened sizeMode(%d)", __func__, sizeMode);
        return -1;
        break;
    }

    CLOGD("DEBUG(%s):realSW(%d), realSH(%d)", __func__, width, height);

    if (m_setZoom(zoom, srcW, srcH, dstW, dstH,
                  (void *)&m_camera_info[m_cameraMode].dummy_shot) == false) {
        CLOGE("ERR(%s):m_setZoom() fail", __func__);
    }

    /* 5. sensor size change(s_crop) */
    CLOGD("DEBUG(%s):SENSOR S_CROP", __func__);
    if (!setSensorStreamOn(CAMERA_MODE_REPROCESSING, width, height, false)) {
        CLOGE("ERR(%s):setSensorStreamOn() fail", __func__);
        return false;
    }

    /* 6. sensor req_buf */
    CLOGD("DEBUG(%s):SENSOR REQ_BUF", __func__);
    m_camera_info[CAMERA_MODE_REPROCESSING].sensor.buffers = 0;
    if (cam_int_reqbufs(&m_camera_info[CAMERA_MODE_REPROCESSING].sensor) < 0) {
        CLOGE("ERR(%s):cam_int_reqbufs() fail", __func__);
        return false;
    }

    m_camera_info[CAMERA_MODE_REPROCESSING].sensor.buffers = NUM_BAYER_BUFFERS;
    if (cam_int_reqbufs(&(m_camera_info[CAMERA_MODE_REPROCESSING].sensor)) < 0) {
        CLOGE("ERR(%s):cam_int_reqbufs() fail", __func__);
        return false;
    }

    /* 8. sensor stream on */
    CLOGD("DEBUG(%s):SENSOR STREAM ON", __func__);
    if (cam_int_streamon(&(m_camera_info[CAMERA_MODE_REPROCESSING].sensor)) < 0) {
        CLOGE("ERR(%s):cam_int_streamon(sensor) fail", __func__);
        return false;
    }

    switch (sizeMode) {
    case SCALABLE_SENSOR_SIZE_13M:
        m_camera_info[m_cameraMode].dummy_shot.request_scp = 0;
        break;
    case SCALABLE_SENSOR_SIZE_FHD:
        m_camera_info[m_cameraMode].dummy_shot.request_scp = 1;
        break;
    }

    /* 9. is3a1Src change size */
    CLOGD("DEBUG(%s):SENSOR SET_CROP", __func__);
    struct v4l2_crop crop;
    width  = SIZE_OTF_WIDTH;
    height = SIZE_OTF_HEIGHT;
    m_camera_info[CAMERA_MODE_BACK].is3a1Src.width   = width;
    m_camera_info[CAMERA_MODE_BACK].is3a1Src.height  = height;
    crop.c.top    = 0;
    crop.c.left   = 0;
    crop.c.width  = width;
    crop.c.height = height;
    crop.type    = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
    if (exynos_v4l2_s_crop(m_camera_info[CAMERA_MODE_BACK].is3a1Src.fd, &crop) < 0) {
        ALOGE("ERR(%s): is3a1Src s_crop fail",  __func__);
        return false;
    }

#ifdef SCALABLE_SENSOR_CTRL_ISP
    /* 10. ISP req_buf */
    CLOGD("DEBUG(%s):ISP REQ_BUF", __func__);
    m_camera_info[CAMERA_MODE_BACK].isp.buffers = 0;
    if (cam_int_reqbufs(&m_camera_info[CAMERA_MODE_BACK].isp) < 0) {
        CLOGE("ERR(%s):cam_int_reqbufs(isp) fail", __func__);
        return false;
    }

    m_camera_info[CAMERA_MODE_BACK].isp.buffers = NUM_BAYER_BUFFERS;
    if (cam_int_reqbufs(&(m_camera_info[CAMERA_MODE_BACK].isp)) < 0) {
        CLOGE("ERR(%s):cam_int_reqbufs(isp) fail", __func__);
        return false;
    }

    /* 11. ISP stream on */
    CLOGD("DEBUG(%s):ISP STREAM ON", __func__);
    if (cam_int_streamon(&(m_camera_info[CAMERA_MODE_BACK].isp)) < 0) {
        CLOGE("ERR(%s):cam_int_streamon(isp) fail", __func__);
        return false;
    }
#endif

    /* 12. 3a1 output req_buf */
    CLOGD("DEBUG(%s):3A1 output REQ_BUF", __func__);
    m_camera_info[CAMERA_MODE_BACK].is3a1Src.buffers = 0;
    if (cam_int_reqbufs(&m_camera_info[CAMERA_MODE_BACK].is3a1Src) < 0) {
        CLOGE("ERR(%s):cam_int_reqbufs(3a1 output) fail", __func__);
        return false;
    }

    m_camera_info[CAMERA_MODE_BACK].is3a1Src.buffers = NUM_BAYER_BUFFERS;
    if (cam_int_reqbufs(&(m_camera_info[CAMERA_MODE_BACK].is3a1Src)) < 0) {
        CLOGE("ERR(%s):cam_int_reqbufs(3a1 output) fail", __func__);
        return false;
    }

    /* 13. 3a1 output stream on */
    CLOGD("DEBUG(%s):3A1 output STREAM ON", __func__);
    if (cam_int_streamon(&(m_camera_info[CAMERA_MODE_BACK].is3a1Src)) < 0) {
        CLOGE("ERR(%s):cam_int_streamon(3a1 output) fail", __func__);
        return false;
    }

    /* 14. 3a1 capture req_buf */
    CLOGD("DEBUG(%s):3A1 output REQ_BUF", __func__);
    m_camera_info[CAMERA_MODE_BACK].is3a1Dst.buffers = 0;
    if (cam_int_reqbufs(&(m_camera_info[CAMERA_MODE_BACK].is3a1Dst)) < 0) {
        CLOGE("ERR(%s):cam_int_reqbufs(3a1 capture) fail", __func__);
        return false;
    }

    m_camera_info[CAMERA_MODE_BACK].is3a1Dst.buffers = NUM_BAYER_BUFFERS;
    if (cam_int_reqbufs(&(m_camera_info[CAMERA_MODE_BACK].is3a1Dst)) < 0) {
        CLOGE("ERR(%s):cam_int_reqbufs(3a1 capture) fail", __func__);
        return false;
    }

    /* 15. 3a1 capture stream on */
    CLOGD("DEBUG(%s):3A1 capture STREAM ON", __func__);
    if (cam_int_streamon(&(m_camera_info[CAMERA_MODE_BACK].is3a1Dst)) < 0) {
        CLOGE("ERR(%s):cam_int_streamon(3a1 capture) fail", __func__);
        return false;
    }

    /* 16. 3a1 capture Q(6) */
    CLOGD("DEBUG(%s):3A1 Q(%d)", __func__, NUM_BAYER_BUFFERS);
    if (qAll3a1Buf() == false) {
        CLOGE("ERR(%s):qAll3a1Buf fail", __func__);
        return false;
    }

    /* 17. s_ctrl stream on */
    CLOGD("DEBUG(%s):S_CTRL STREAM ON", __func__);
    if (exynos_v4l2_s_ctrl(m_camera_info[CAMERA_MODE_BACK].sensor.fd, V4L2_CID_IS_S_STREAM, IS_ENABLE_STREAM) < 0) {
        CLOGE("ERR(%s):exynos_v4l2_s_ctrl fail(IS_ENABLE_STREAM)", __func__);
        return false;
    }

    CLOGD("DEBUG(%s):All done", __func__);
    return true;
}

bool ExynosCamera::getScalableSensorStart(void)
{
    return m_curCameraInfo[m_cameraMode]->scalableSensorStart;
}

bool ExynosCamera::setScalableSensorStart(bool toggle)
{
    m_curCameraInfo[m_cameraMode]->scalableSensorStart = toggle;
    return true;
}

void ExynosCamera::getScalableSensorSizeOnPreview(int *w, int *h)
{
    int width  = m_curCameraInfo[m_cameraMode]->previewW;
    int height = m_curCameraInfo[m_cameraMode]->previewH;

    /* when preview size is 1440x1080(4:3), return sensor size(1920x1440) */
    if (width == 1440 && height == 1080) {
        width  = 1920;
        height = 1440;
    } else if (!(width == 1920 && height == 1080)) {
        /* default sensor size is 1920x1080(16:9) */
        CLOGW("WARN(%s): preview size is (%d/%d)", __func__, width, height);
        width  = 1920;
        height = 1080;
    }

    *w = width;
    *h = height;
    CLOGD("DEBUG(%s): preview size (%d/%d)", __func__, *w, *h);
}

int ExynosCamera::getPlaneSizePackedFLiteOutput(int width, int height)
{
    /*
    1. The sensor output is 10bit per pixel but Flite output is 12bit added zero padding
        because there is not 10bit output mode in Flite.
    2. Flite write 5pixel in 64bit.
    3. The pixel should be aligned by 10's multiple.
    */
    int PlaneSize;
    int Alligned_Width;
    int Bytes;

    Alligned_Width = (width + 9) / 10 * 10;
    Bytes = Alligned_Width * 8 / 5 ; /* 5:64 = x:width */

    PlaneSize = Bytes * height;

    return PlaneSize;
}

#endif
