/*
 * Copyright Samsung Electronics Co.,LTD.
 * Copyright (C) 2013 The Android Open Source Project
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

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <math.h>
#include <sys/poll.h>
#include <cutils/log.h>
#include <utils/Log.h>

#include "ExynosJpegApi.h"

#define JPEG_ERROR_LOG(fmt,...) ALOGE(fmt,##__VA_ARGS__)

#define NUM_JPEG_DEC_IN_PLANES (1)
#define NUM_JPEG_DEC_OUT_PLANES (1)
#define NUM_JPEG_DEC_IN_BUFS (1)
#define NUM_JPEG_DEC_OUT_BUFS (1)

#define MAXIMUM_JPEG_SIZE(n) ((65535 - (n)) * 32768)

ExynosJpegDecoder::ExynosJpegDecoder()
{
    t_iJpegFd = -1;
    t_bFlagCreate = false;
}

ExynosJpegDecoder::~ExynosJpegDecoder()
{
    if (t_bFlagCreate == true)
        this->destroy();
}

int ExynosJpegDecoder::create(void)
{
    return ExynosJpegBase::create(MODE_DECODE);
}

int ExynosJpegDecoder::destroy(void)
{
    return ExynosJpegBase::destroy(NUM_JPEG_DEC_IN_BUFS, NUM_JPEG_DEC_OUT_BUFS);
}

int ExynosJpegDecoder::setJpegConfig(void *pConfig)
{
    return ExynosJpegBase::setJpegConfig(MODE_DECODE, pConfig);
}

int ExynosJpegDecoder::checkInBufType(void)
{
    return checkBufType(&t_stJpegInbuf);
}

int ExynosJpegDecoder::checkOutBufType(void)
{
    return checkBufType(&t_stJpegOutbuf);
}

int ExynosJpegDecoder::getInBuf(char **pcBuf, int *piInputSize)
{
    return getBuf(t_bFlagCreateInBuf, &t_stJpegInbuf, pcBuf, piInputSize,
                    NUM_JPEG_DEC_IN_PLANES, NUM_JPEG_DEC_IN_PLANES);
}

int ExynosJpegDecoder::getOutBuf(char **pcBuf, int *piOutputSize, int iSize)
{
    return getBuf(t_bFlagCreateOutBuf, &t_stJpegOutbuf, pcBuf, piOutputSize, iSize, t_iPlaneNum);
}

int  ExynosJpegDecoder::setInBuf(char *pcBuf, int iSize)
{
    int iRet = ERROR_NONE;
    iRet = setBuf(&t_stJpegInbuf, &pcBuf, &iSize, NUM_JPEG_DEC_IN_PLANES);

    if (iRet == ERROR_NONE)
        t_bFlagCreateInBuf = true;

    return iRet;
}

int  ExynosJpegDecoder::setOutBuf(char **pcBuf, int *iSize)
{
    int iRet = ERROR_NONE;
    iRet = setBuf(&t_stJpegOutbuf, pcBuf, iSize, t_iPlaneNum);

    if (iRet == ERROR_NONE)
        t_bFlagCreateOutBuf = true;

    return iRet;
}

int ExynosJpegDecoder::getInBuf(int *piBuf, int *piInputSize)
{
    return getBuf(t_bFlagCreateInBuf, &t_stJpegInbuf, piBuf, piInputSize,
                    NUM_JPEG_DEC_IN_PLANES, NUM_JPEG_DEC_IN_PLANES);
}

int ExynosJpegDecoder::getOutBuf(int *piBuf, int *piOutputSize, int iSize)
{
    return getBuf(t_bFlagCreateOutBuf, &t_stJpegOutbuf, piBuf, piOutputSize, iSize, t_iPlaneNum);
}

int ExynosJpegDecoder::setInBuf(int piBuf, int iSize)
{
    int iRet = ERROR_NONE;
    iRet = setBuf(&t_stJpegInbuf, &piBuf, &iSize, NUM_JPEG_DEC_IN_PLANES);

    if (iRet == ERROR_NONE)
        t_bFlagCreateInBuf = true;

    return iRet;
}

int ExynosJpegDecoder::setOutBuf(int *piBuf, int *iSize)
{
    int iRet = ERROR_NONE;
    iRet = setBuf(&t_stJpegOutbuf, piBuf, iSize, t_iPlaneNum);

    if (iRet == ERROR_NONE)
        t_bFlagCreateOutBuf = true;

    return iRet;
}

int ExynosJpegDecoder::getSize(int *piW, int *piH)
{
    if (t_bFlagCreate == false)
        return ERROR_JPEG_DEVICE_NOT_CREATE_YET;

    int iRet = t_v4l2GetFmt(t_iJpegFd, V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE, &t_stJpegConfig);
    if (iRet < 0) {
        JPEG_ERROR_LOG("[%s,%d]: get image size failed\n", __func__, iRet);
        return ERROR_GET_SIZE_FAIL;
    }

    *piW = t_stJpegConfig.width;
    *piH = t_stJpegConfig.height;

    return ERROR_NONE;
}

int ExynosJpegDecoder::setColorFormat(int iV4l2ColorFormat)
{
    return ExynosJpegBase::setColorFormat(MODE_DECODE, iV4l2ColorFormat);
}

int ExynosJpegDecoder::setJpegFormat(int iV4l2JpegFormat)
{
    return ExynosJpegBase::setJpegFormat(MODE_DECODE, iV4l2JpegFormat);
}

int ExynosJpegDecoder::updateConfig(void)
{
    return ExynosJpegBase::updateConfig(MODE_DECODE,
                    NUM_JPEG_DEC_IN_BUFS, NUM_JPEG_DEC_OUT_BUFS,
                    NUM_JPEG_DEC_IN_PLANES, NUM_JPEG_DEC_OUT_PLANES);
}

int ExynosJpegDecoder::setScaledSize(int iW, int iH)
{
    int mcu_x_size = 0;

    if (t_bFlagCreate == false)
        return ERROR_JPEG_DEVICE_NOT_CREATE_YET;

    switch (t_stJpegConfig.pix.enc_fmt.out_fmt) {
    case V4L2_PIX_FMT_JPEG_444:
    case V4L2_PIX_FMT_JPEG_GRAY:
        mcu_x_size = 8;
        break;
    case V4L2_PIX_FMT_JPEG_422:
    case V4L2_PIX_FMT_JPEG_420:
        mcu_x_size = 16;
        break;
    default:
        mcu_x_size = 8;
        break;
    }

    if (iH < 0 || iW < 0 || (iW * iH) > MAXIMUM_JPEG_SIZE(mcu_x_size))
        return ERROR_INVALID_IMAGE_SIZE;

    t_stJpegConfig.scaled_width = iW;
    t_stJpegConfig.scaled_height = iH;

    return ERROR_NONE;
}

int ExynosJpegDecoder::setJpegSize(int iJpegSize)
{
    if (t_bFlagCreate == false)
        return ERROR_JPEG_DEVICE_NOT_CREATE_YET;

    if (iJpegSize <= 0)
        return ERROR_JPEG_SIZE_TOO_SMALL;

    t_stJpegConfig.sizeJpeg = iJpegSize;

    return ERROR_NONE;
}

int ExynosJpegDecoder::decode(void)
{
    return ExynosJpegBase::execute(NUM_JPEG_DEC_OUT_PLANES, t_iPlaneNum);
}
