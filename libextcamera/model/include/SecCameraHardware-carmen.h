/*
**
** Copyright 2008, The Android Open Source Project
** Copyright (c) 2011, Samsung Electronics Co. LTD
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
**
** Author:  Dong-Seong Lim <dongseong.lim@samsung.com>
**
*/
#ifndef ANDROID_HARDWARE_SECCAMERAHARDWARE_CARMEN_H
#define ANDROID_HARDWARE_SECCAMERAHARDWARE_CARMEN_H

#include "Exif.h"
#include "SecCameraParameters.h"

/**
 * Define function feature
**/
#define ZOOM_FUNCTION       true
#define FRONT_ZSL           true
#define IS_FW_DEBUG         false
#if IS_FW_DEBUG
#define IS_FW_DEBUG_THREAD    false
#endif

#define IsZoomSupported()                   (true)
#define IsAPZoomSupported()                 (ZOOM_FUNCTION)
#define IsAutoFocusSupported()              (true)
#define IsFlashSupported()                  (false)
#define IsFastCaptureSupportedOnRear()      (false)
#define IsFastCaptureSupportedOnFront()     (false)


namespace android {

#ifdef ANDROID_HARDWARE_SECCAMERAHARDWARE_CPP
/**
 * Exif Info dependent on Camera Module
**/

/* F Number */
const int Exif::DEFAULT_BACK_FNUMBER_NUM = 27;       /* Optical communication */
const int Exif::DEFAULT_BACK_FNUMBER_DEN = 10;
const int Exif::DEFAULT_FRONT_FNUMBER_NUM = 28;
const int Exif::DEFAULT_FRONT_FNUMBER_DEN = 10;

/* Focal length */
const int Exif::DEFAULT_BACK_FOCAL_LEN_NUM = 343;     /* Optical communication */
const int Exif::DEFAULT_BACK_FOCAL_LEN_DEN = 100;
const int Exif::DEFAULT_FRONT_FOCAL_LEN_NUM = 250;
const int Exif::DEFAULT_FRONT_FOCAL_LEN_DEN = 100;

#endif /* ANDROID_HARDWARE_SECCAMERAHARDWARE_CPP */

/**
 * Define Supported parameters for ISecCameraHardware
**/
#ifdef ANDROID_HARDWARE_ISECCAMERAHARDWARE_CPP

/* Frame Rates */
#define B_KEY_PREVIEW_FPS_RANGE_VALUE "15000,30000"
#define B_KEY_SUPPORTED_PREVIEW_FPS_RANGE_VALUE "(15000,30000),(7000,7000),(15000,15000),(30000,30000)"
#define B_KEY_SUPPORTED_PREVIEW_FRAME_RATES_VALUE "30,15,7"
#define B_KEY_PREVIEW_FRAME_RATE_VALUE 30

#define F_KEY_PREVIEW_FPS_RANGE_VALUE "15000,30000"
#define F_KEY_SUPPORTED_PREVIEW_FPS_RANGE_VALUE "(15000,30000),(15000,15000),(30000,30000)"
#define F_KEY_SUPPORTED_PREVIEW_FRAME_RATES_VALUE "30,15"
#define F_KEY_PREVIEW_FRAME_RATE_VALUE 30

/* Preferred preview size for video.
 * This preferred preview size must be in supported preview size list
 */
#define B_KEY_PREFERRED_PREVIEW_SIZE_FOR_VIDEO_VALUE "1280x720"
#define F_KEY_PREFERRED_PREVIEW_SIZE_FOR_VIDEO_VALUE "1280x720"

/* Video */
#define B_KEY_VIDEO_STABILIZATION_SUPPORTED_VALUE       "false"
#define F_KEY_VIDEO_STABILIZATION_SUPPORTED_VALUE       "false"
#define KEY_VIDEO_SNAPSHOT_SUPPORTED_VALUE              "false"

/* Focus */
#define B_KEY_NORMAL_FOCUS_DISTANCES_VALUE "0.15,1.20,Infinity" /* Normal */
#define B_KEY_MACRO_FOCUS_DISTANCES_VALUE   "0.10,0.15,0.30" /* Macro*/
#define F_KEY_FOCUS_DISTANCES_VALUE  "0.20,0.25,Infinity" /* Fixed */

/* Zoom */
#define B_KEY_ZOOM_SUPPORTED_VALUE                  "true"
#define B_KEY_SMOOTH_ZOOM_SUPPORTED_VALUE           "false"
#define F_KEY_ZOOM_SUPPORTED_VALUE                  "false"
#define F_KEY_SMOOTH_ZOOM_SUPPORTED_VALUE           "false"

/* AE, AWB Lock */
#define B_KEY_AUTO_EXPOSURE_LOCK_SUPPORTED_VALUE            "true"
#define B_KEY_AUTO_WHITEBALANCE_LOCK_SUPPORTED_VALUE        "true"
#define F_KEY_AUTO_EXPOSURE_LOCK_SUPPORTED_VALUE            "false"
#define F_KEY_AUTO_WHITEBALANCE_LOCK_SUPPORTED_VALUE        "false"

/* Face Detect */
#define B_KEY_MAX_NUM_DETECTED_FACES_HW_VALUE "0"
#define B_KEY_MAX_NUM_DETECTED_FACES_SW_VALUE "3"
#define F_KEY_MAX_NUM_DETECTED_FACES_HW_VALUE "0"
#define F_KEY_MAX_NUM_DETECTED_FACES_SW_VALUE "0"


static const image_rect_type backFLiteSizes[] = {
    { 1280,   960 }, /* default */
    { 2560,   1920 },
    { 2048,   1536 },
    { 1920,   1080 },
    { 1600,   1200 },
    { 1280,   720 },
    { 1024,   768 },
//    { 720,    480 },
    { 640,    480 },
    { 176,    144 },
};

static const image_rect_type frontFLiteSizes[] = {
    { 640,    480 }, /* default */
};

static const image_rect_type backPreviewSizes[] = {
    { 1280,   960 }, /* default */
    { 1920,  1080 },
    { 1280,   720 },
//    { 720,    480 },
    { 640,    480 },
};

static const image_rect_type hiddenBackPreviewSizes[0] = {
};

static const image_rect_type backRecordingSizes[] = {
    { 1280,   720 }, /* default */
    { 1920,   1080 },
//    { 720,    480 },
    { 640,    480 },
};

static const image_rect_type hiddenBackRecordingSizes[0] = {
};

static const image_rect_type backPictureSizes[] = {
    { 2560,   1920 }, /* default */
    { 2048,   1536 },
    { 1600,   1200 },
//    { 1280,   960 },
    { 1024,   768 },
    { 640,    480 },
//    { 320,    240 },
//    { 176,    144 },
};

static const image_rect_type hiddenBackPictureSizes[0] = {
};

static const image_rect_type frontPreviewSizes[] = {
    { 640,    480 },    /* default */
    { 320,    240 },
    { 160,    120 },
};

static const image_rect_type hiddenFrontPreviewSizes[0] = {
};

static const image_rect_type frontRecordingSizes[] = {
    { 640,    480 },    /* default */
    { 320,    240 },
    { 160,    120 },
};

static const image_rect_type hiddenFrontRecordingSizes[0] = {
};

static const image_rect_type frontPictureSizes[] = {
    { 640,    480 }, /* default */
    { 320,    240 },
    { 160,    120 },
};

static const image_rect_type hiddenFrontPictureSizes[0] = {
};

static const image_rect_type backThumbSizes[] = {
    { 160,    120 },
    { 0,      0 },
};

static const image_rect_type frontThumbSizes[] = {
    { 160,    120 },    /* default */
    { 0,      0 },
};

static const cam_strmap_t whiteBalances[] = {
    { CameraParameters::WHITE_BALANCE_AUTO,         WHITE_BALANCE_AUTO },
    { CameraParameters::WHITE_BALANCE_INCANDESCENT, WHITE_BALANCE_TUNGSTEN },
    { CameraParameters::WHITE_BALANCE_FLUORESCENT,  WHITE_BALANCE_FLUORESCENT },
    { CameraParameters::WHITE_BALANCE_DAYLIGHT,     WHITE_BALANCE_SUNNY },
    { CameraParameters::WHITE_BALANCE_CLOUDY_DAYLIGHT, WHITE_BALANCE_CLOUDY },
};

static const cam_strmap_t effects[] = {
    { CameraParameters::EFFECT_NONE,        IMAGE_EFFECT_NONE },
    { CameraParameters::EFFECT_MONO,        IMAGE_EFFECT_BNW },
    { CameraParameters::EFFECT_NEGATIVE,    IMAGE_EFFECT_NEGATIVE },
    { CameraParameters::EFFECT_SEPIA,       IMAGE_EFFECT_SEPIA }
};

static const cam_strmap_t sceneModes[] = {
    { CameraParameters::SCENE_MODE_AUTO,        SCENE_MODE_NONE },
    { CameraParameters::SCENE_MODE_PORTRAIT,    SCENE_MODE_PORTRAIT },
    { CameraParameters::SCENE_MODE_LANDSCAPE,   SCENE_MODE_LANDSCAPE },
    { CameraParameters::SCENE_MODE_NIGHT,       SCENE_MODE_NIGHTSHOT },
    { CameraParameters::SCENE_MODE_BEACH,       SCENE_MODE_BEACH_SNOW },
    { CameraParameters::SCENE_MODE_SUNSET,      SCENE_MODE_SUNSET },
    { CameraParameters::SCENE_MODE_FIREWORKS,    SCENE_MODE_FIREWORKS },
    { CameraParameters::SCENE_MODE_SPORTS,      SCENE_MODE_SPORTS },
    { CameraParameters::SCENE_MODE_PARTY,       SCENE_MODE_PARTY_INDOOR },
    { CameraParameters::SCENE_MODE_CANDLELIGHT, SCENE_MODE_CANDLE_LIGHT },
};

static const cam_strmap_t flashModes[] = {
    { CameraParameters::FLASH_MODE_OFF,         FLASH_MODE_OFF },
    { CameraParameters::FLASH_MODE_AUTO,        FLASH_MODE_AUTO },
    { CameraParameters::FLASH_MODE_ON,          FLASH_MODE_ON },
    { CameraParameters::FLASH_MODE_TORCH,       FLASH_MODE_TORCH },
};

static const cam_strmap_t previewPixelFormats[] = {
    { CameraParameters::PIXEL_FORMAT_YUV420SP,  CAM_PIXEL_FORMAT_YUV420SP },
	{ CameraParameters::PIXEL_FORMAT_YUV420P,   CAM_PIXEL_FORMAT_YVU420P },
/*	{ CameraParameters::PIXEL_FORMAT_YUV422I,   CAM_PIXEL_FORMAT_YUV422I },
    { CameraParameters::PIXEL_FORMAT_YUV422SP,  CAM_PIXEL_FORMAT_YUV422SP },
    { CameraParameters::PIXEL_FORMAT_RGB565,    CAM_PIXEL_FORMAT_RGB565, }, */
};

static const cam_strmap_t picturePixelFormats[] = {
    { CameraParameters::PIXEL_FORMAT_JPEG,      CAM_PIXEL_FORMAT_JPEG, },
};

#if IsAutoFocusSupported()
static const cam_strmap_t backFocusModes[] = {
    { CameraParameters::FOCUS_MODE_AUTO,        FOCUS_MODE_AUTO },
    { CameraParameters::FOCUS_MODE_INFINITY,    FOCUS_MODE_INFINITY },
    { CameraParameters::FOCUS_MODE_MACRO,       FOCUS_MODE_MACRO },
    { CameraParameters::FOCUS_MODE_FIXED,       FOCUS_MODE_FIXED },
/*  { CameraParameters::FOCUS_MODE_CONTINUOUS_VIDEO, FOCUS_MODE_CONTINOUS },
    { SecCameraParameters::FOCUS_MODE_FACEDETECT, FOCUS_MODE_FACEDETECT }, */
};
#else
static const cam_strmap_t backFocusModes[] = {
    { CameraParameters::FOCUS_MODE_FIXED,       FOCUS_MODE_FIXED },
};
#endif

static const cam_strmap_t frontFocusModes[] = {
    { CameraParameters::FOCUS_MODE_FIXED,   FOCUS_MODE_INFINITY },
};

static const cam_strmap_t firmwareModes[] = {
    { SecCameraParameters::FIRMWARE_MODE_NONE,  CAM_FW_MODE_NONE },
    { SecCameraParameters::FIRMWARE_MODE_VERSION, CAM_FW_MODE_VERSION },
    { SecCameraParameters::FIRMWARE_MODE_UPDATE, CAM_FW_MODE_UPDATE },
    { SecCameraParameters::FIRMWARE_MODE_DUMP,  CAM_FW_MODE_DUMP },
};

static const cam_strmap_t isos[] = {
    { SecCameraParameters::ISO_AUTO,    ISO_AUTO },
    { SecCameraParameters::ISO_50,      ISO_50 },
    { SecCameraParameters::ISO_100,     ISO_100 },
    { SecCameraParameters::ISO_200,     ISO_200 },
    { SecCameraParameters::ISO_400,     ISO_400 },
    { SecCameraParameters::ISO_800,     ISO_800 },
    { SecCameraParameters::ISO_1600,    ISO_1600 },
    { SecCameraParameters::ISO_SPORTS,  ISO_SPORTS },
    { SecCameraParameters::ISO_NIGHT,   ISO_NIGHT },
};

static const cam_strmap_t meterings[] = {
    { SecCameraParameters::METERING_CENTER, METERING_CENTER },
    { SecCameraParameters::METERING_MATRIX, METERING_MATRIX },
    { SecCameraParameters::METERING_SPOT,   METERING_SPOT },
};

static const cam_strmap_t antibandings[] = {
    { CameraParameters::ANTIBANDING_AUTO, ANTI_BANDING_AUTO },
    { CameraParameters::ANTIBANDING_50HZ, ANTI_BANDING_50HZ },
    { CameraParameters::ANTIBANDING_60HZ, ANTI_BANDING_60HZ },
    { CameraParameters::ANTIBANDING_OFF, ANTI_BANDING_OFF },
};

/* Define initial skip frame count */
static const int INITIAL_REAR_SKIP_FRAME = 3;
static const int INITIAL_FRONT_SKIP_FRAME = 3;

#endif /* ANDROID_HARDWARE_ISECCAMERAHARDWARE_CPP */


/**
 * Define Camera Info
**/
#define SEC_CAMERA_INFO \
    { \
        { \
            facing          : CAMERA_FACING_BACK, \
            orientation     : 0, \
        }, \
        { \
            facing          : CAMERA_FACING_FRONT, \
            orientation     : 270, \
        } \
    }

}; /* namespace android */
#endif /* ANDROID_HARDWARE_SECCAMERAHARDWARE_P4NOTE_H */
