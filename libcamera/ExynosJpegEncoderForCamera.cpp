/*
 * Copyright Samsung Electronics Co.,LTD.
 * Copyright (C) 2012 The Android Open Source Project
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

#include <utils/Log.h>

#include "ExynosCameraParameters.h"
#include "ExynosCameraHWImpl.h"
#include "exynos_format.h"

#ifdef USE_CSC_FEATURE
#include <cutils/properties.h>
#include <SecNativeFeature.h>
#endif

#include "ExynosJpegEncoderForCamera.h"

static const char ExifAsciiPrefix[] = { 0x41, 0x53, 0x43, 0x49, 0x49, 0x0, 0x0, 0x0 };

#define THUMBNAIL_IMAGE_PIXEL_SIZE (4)
#define MAX_JPG_WIDTH (8192)
#define MAX_JPG_HEIGHT (8192)

#define MAX_INPUT_BUFFER_PLANE_NUM (3)
#define MAX_OUTPUT_BUFFER_PLANE_NUM (1)

ExynosJpegEncoderForCamera::ExynosJpegEncoderForCamera()
{
    m_flagCreate = false;
    m_jpegMain = NULL;
    m_jpegThumb = NULL;
    m_thumbnailW = 0;
    m_thumbnailH = 0;
    m_thumbnailQuality = JPEG_THUMBNAIL_QUALITY;
    m_exynosThumbCSC = NULL;
    m_ionJpegClient = 0;
    initJpegMemory(&m_stThumbInBuf, MAX_IMAGE_PLANE_NUM);
    initJpegMemory(&m_stThumbOutBuf, MAX_IMAGE_PLANE_NUM);
}

ExynosJpegEncoderForCamera::~ExynosJpegEncoderForCamera()
{
    if (m_flagCreate == true)
        this->destroy();
}

bool ExynosJpegEncoderForCamera::flagCreate(void)
{
    return m_flagCreate;
}

int ExynosJpegEncoderForCamera::create(void)
{
    int ret = ERROR_NONE;
    if (m_flagCreate == true)
        return ERROR_ALREADY_CREATE;

    if (m_jpegMain == NULL) {
        m_jpegMain = new ExynosJpegEncoder;

        if (m_jpegMain == NULL) {
            ALOGE("ERR(%s):Cannot create ExynosJpegEncoder class", __func__);
            return ERROR_CANNOT_CREATE_EXYNOS_JPEG_ENC_HAL;
        }

        ret = m_jpegMain->create();
        if (ret)
            return ret;

        ret = m_jpegMain->setCache(JPEG_CACHE_ON);

        if (ret) {
            m_jpegMain->destroy();
            return ret;
        }
    }

    m_ionJpegClient = createIonClient(m_ionJpegClient);
    if(m_ionJpegClient == 0)
        return ERROR_CANNOT_CREATE_EXYNOS_JPEG_ENC_HAL;

    m_stThumbInBuf.ionClient = m_stThumbOutBuf.ionClient = m_ionJpegClient;

    m_flagCreate = true;

    return ERROR_NONE;
}

int ExynosJpegEncoderForCamera::destroy(void)
{
    if (m_flagCreate == false)
        return ERROR_ALREADY_DESTROY;

    if (m_jpegMain != NULL) {
        m_jpegMain->destroy();
        delete m_jpegMain;
        m_jpegMain = NULL;
    }

    if (m_jpegThumb != NULL) {
        int iSize = sizeof(char)*m_thumbnailW*m_thumbnailH*4;

        freeJpegMemory(&m_stThumbInBuf, MAX_IMAGE_PLANE_NUM);
        freeJpegMemory(&m_stThumbOutBuf, MAX_IMAGE_PLANE_NUM);
        m_ionJpegClient = deleteIonClient(m_ionJpegClient);
        m_stThumbInBuf.ionClient = m_stThumbOutBuf.ionClient = m_ionJpegClient;
        m_jpegThumb->destroy();
        delete m_jpegThumb;
        m_jpegThumb = NULL;
    }

    m_flagCreate = false;
    m_thumbnailW = 0;
    m_thumbnailH = 0;
    m_thumbnailQuality = JPEG_THUMBNAIL_QUALITY;
    return ERROR_NONE;
}

int ExynosJpegEncoderForCamera::setSize(int w, int h)
{
    if (m_flagCreate == false)
        return ERROR_NOT_YET_CREATED;

    return m_jpegMain->setSize(w, h);
}


int ExynosJpegEncoderForCamera::setQuality(int quality)
{
    if (m_flagCreate == false)
        return ERROR_NOT_YET_CREATED;

    return m_jpegMain->setQuality(quality);
}

int ExynosJpegEncoderForCamera::setColorFormat(int colorFormat)
{
    if (m_flagCreate == false)
        return ERROR_NOT_YET_CREATED;

    return m_jpegMain->setColorFormat(colorFormat);
}

int ExynosJpegEncoderForCamera::setJpegFormat(int jpegFormat)
{
    if (m_flagCreate == false)
        return ERROR_NOT_YET_CREATED;

    return m_jpegMain->setJpegFormat(jpegFormat);
}

int ExynosJpegEncoderForCamera::updateConfig(void)
{
    if (m_flagCreate == false)
        return ERROR_NOT_YET_CREATED;

    return m_jpegMain->updateConfig();
}

int ExynosJpegEncoderForCamera::setInBuf(int *buf, int *size)
{
    if (m_flagCreate == false)
        return ERROR_NOT_YET_CREATED;

    if (buf == NULL)
        return ERROR_BUFFR_IS_NULL;

    if (size == NULL)
        return ERROR_BUFFR_IS_NULL;

    int ret = ERROR_NONE;

    ret = m_jpegMain->setInBuf(buf, size);
    if (ret) {
        ALOGE("ERR(%s):Fail to JPEG input buffer!!", __func__);
        return ret;
    }

    return ERROR_NONE;
}

int ExynosJpegEncoderForCamera::setOutBuf(int buf, int size)
{
    if (m_flagCreate == false)
        return ERROR_NOT_YET_CREATED;

    if (buf == NULL)
        return ERROR_BUFFR_IS_NULL;

    if (size<=0)
        return ERROR_BUFFER_TOO_SMALL;

    int ret = ERROR_NONE;

    ret = m_jpegMain->setOutBuf(buf, size);
    if (ret) {
        ALOGE("ERR(%s):Fail to JPEG output buffer!!", __func__);
        return ret;
    }

    return ERROR_NONE;
}

int  ExynosJpegEncoderForCamera::setInBuf(char **buf, int *size)
{
    if (m_flagCreate == false)
        return ERROR_NOT_YET_CREATED;

    if (buf == NULL)
        return ERROR_BUFFR_IS_NULL;

    if (size == NULL)
        return ERROR_BUFFR_IS_NULL;

    int ret = ERROR_NONE;

    ret = m_jpegMain->setInBuf(buf, size);
    if (ret) {
        ALOGE("ERR(%s):Fail to JPEG input buffer!!", __func__);
        return ret;
    }

    return ERROR_NONE;
}

int  ExynosJpegEncoderForCamera::setOutBuf(char *buf, int size)
{
    if (m_flagCreate == false)
        return ERROR_NOT_YET_CREATED;

    if (buf == NULL)
        return ERROR_BUFFR_IS_NULL;

    if (size<=0)
        return ERROR_BUFFER_TOO_SMALL;

    int ret = ERROR_NONE;
    ret = m_jpegMain->setOutBuf(buf, size);
    if (ret) {
        ALOGE("ERR(%s):Fail to JPEG output buffer!!", __func__);
        return ret;
    }

    return ERROR_NONE;
}

int ExynosJpegEncoderForCamera::encode(int *size, exif_attribute_t *exifInfo)
{
    int ret = ERROR_NONE;
    unsigned char *exifOut = NULL;

    if (m_flagCreate == false)
        return ERROR_NOT_YET_CREATED;

    ret = m_jpegMain->encode();
    if (ret) {
        ALOGE("encode failed");
        return ret;
    }

    int iJpegSize = m_jpegMain->getJpegSize();

    if (iJpegSize<=0) {
        ALOGE("ERR(%s): output_size is too small(%d)!!", __func__, iJpegSize);
        return ERROR_OUT_BUFFER_SIZE_TOO_SMALL;
    }

    int iOutputSize = 0;
    char *pcJpegBuffer = NULL;

    int iJpegBuffer = 0;

    if (m_jpegMain->checkInBufType() & JPEG_BUF_TYPE_USER_PTR) {
        ret = m_jpegMain->getOutBuf((char **)&pcJpegBuffer, &iOutputSize);
        if (ret != ERROR_NONE)
            return ret;
    } else if (m_jpegMain->checkInBufType() & JPEG_BUF_TYPE_DMA_BUF) {
        ret = m_jpegMain->getOutBuf((int *)&iJpegBuffer, &iOutputSize);
        if (ret != ERROR_NONE) {
            ALOGE("ERR(%s): m_jpegMain->getOutBuf() fail", __func__);
            return ret;
        }

        if (iJpegBuffer != 0) {
            if (mmapJpegMemory(&iJpegBuffer, &pcJpegBuffer, &iOutputSize, MAX_OUTPUT_BUFFER_PLANE_NUM) == false) {
                ALOGE("ERR(%s): mmapJpegMemory() fail", __func__);

                unmapJpegMemory(&iJpegBuffer, &pcJpegBuffer, &iOutputSize, MAX_OUTPUT_BUFFER_PLANE_NUM);
                return ERROR_MEM_ALLOC_FAIL;
            }
        }
    } else {
        ALOGE("ERR(%s): invalid buffer type (%d) fail", __func__, m_jpegMain->checkInBufType());
        return ERROR_BUFFR_IS_NULL;
    }

    if (pcJpegBuffer == NULL) {
        ALOGE("ERR(%s):pcJpegBuffer is null!!", __func__);
        return ERROR_OUT_BUFFER_CREATE_FAIL;
    }

    if (exifInfo != NULL) {
        unsigned int thumbLen, exifLen;

        unsigned int bufSize = 0;
        if (exifInfo->enableThumb) {
            if (encodeThumbnail(&thumbLen)) {
                ALOGE("ERR(%s):encodeThumbnail() fail", __func__);
                bufSize = EXIF_FILE_SIZE;
                exifInfo->enableThumb = false;
            } else {
                if (thumbLen > EXIF_LIMIT_SIZE) {
                    ALOGE("ERR(%s):thumbLen(%d) is too bigger than EXIF_LIMIT_SIZE(%d)",
                        __func__, thumbLen, EXIF_LIMIT_SIZE);

                    bufSize = EXIF_FILE_SIZE;
                    exifInfo->enableThumb = false;
                } else {
                    bufSize = EXIF_FILE_SIZE + thumbLen;
                }
            }
        } else {
            bufSize = EXIF_FILE_SIZE;
            exifInfo->enableThumb = false;
        }

        exifOut = new unsigned char[bufSize];
        if (exifOut == NULL) {
            ALOGE("ERR(%s):Failed to allocate for exifOut", __func__);
            delete[] exifOut;
            return ERROR_EXIFOUT_ALLOC_FAIL;
        }
        memset(exifOut, 0, bufSize);

        if (makeExif (exifOut, exifInfo, &exifLen)) {
            ALOGE("ERR(%s):Failed to make EXIF", __func__);
            delete[] exifOut;
            return ERROR_MAKE_EXIF_FAIL;
        }

        /* Exif info size is overflow */
        if (exifLen - thumbLen > EXIF_INFO_LIMIT_SIZE) {
            ALOGE("ERR(%s):Exif info size(%d) is bigger than EXIF_INFO_LIMIT_SIZE(%d)",
                __func__, exifLen - thumbLen, EXIF_INFO_LIMIT_SIZE);
        }

        if (exifLen <= EXIF_LIMIT_SIZE) {
            memmove(pcJpegBuffer+exifLen + 2, pcJpegBuffer + 2, iJpegSize - 2);
            if (exifLen <= bufSize) {
                memcpy(pcJpegBuffer + 2, exifOut, exifLen);
                iJpegSize += exifLen;
            } else {
                ALOGE("ERR(%s):exifLen(%d) is too bigger than EXIF_FILE_SIZE(%d)",
                      __func__, exifLen, EXIF_FILE_SIZE);
            }
        } else {
            ALOGE("ERR(%s):exifLen(%d) is too bingger than EXIF_LIMIT_SIZE(%d)",
	          __func__, exifLen, EXIF_LIMIT_SIZE);
        }

        delete[] exifOut;
    }

    if (iJpegBuffer != 0)
        unmapJpegMemory(&iJpegBuffer, &pcJpegBuffer, &iOutputSize, MAX_OUTPUT_BUFFER_PLANE_NUM);

    *size = iJpegSize;

    return ERROR_NONE;
}

int ExynosJpegEncoderForCamera::makeExif (unsigned char *exifOut,
                              exif_attribute_t *exifInfo,
                              unsigned int *size,
                              bool useMainbufForThumb)
{
    unsigned char *pCur, *pApp1Start, *pIfdStart, *pGpsIfdPtr, *pNextIfdOffset, *pInteroperabilityIfdPtr;
    unsigned int tmp, LongerTagOffest = 0, exifSizeExceptThumb;
    pApp1Start = pCur = exifOut;

    if (!m_jpegMain)
        return ERROR_FAIL;
    if (!m_jpegThumb && exifInfo->enableThumb)
        return ERROR_FAIL;

    //2 Exif Identifier Code & TIFF Header
    pCur += 4;  // Skip 4 Byte for APP1 marker and length
    unsigned char ExifIdentifierCode[6] = { 0x45, 0x78, 0x69, 0x66, 0x00, 0x00 };
    memcpy(pCur, ExifIdentifierCode, 6);
    pCur += 6;

    /* Byte Order - little endian, Offset of IFD - 0x00000008.H */
    unsigned char TiffHeader[8] = { 0x49, 0x49, 0x2A, 0x00, 0x08, 0x00, 0x00, 0x00 };
    memcpy(pCur, TiffHeader, 8);
    pIfdStart = pCur;
    pCur += 8;

    //2 0th IFD TIFF Tags
    if (exifInfo->enableGps)
        tmp = NUM_0TH_IFD_TIFF;
    else
        tmp = NUM_0TH_IFD_TIFF - 1;

    memcpy(pCur, &tmp, NUM_SIZE);
    pCur += NUM_SIZE;

    LongerTagOffest += 8 + NUM_SIZE + tmp*IFD_SIZE + OFFSET_SIZE;

    writeExifIfd(&pCur, EXIF_TAG_IMAGE_WIDTH, EXIF_TYPE_LONG,
                 1, exifInfo->width);
    writeExifIfd(&pCur, EXIF_TAG_IMAGE_HEIGHT, EXIF_TYPE_LONG,
                 1, exifInfo->height);
    writeExifIfd(&pCur, EXIF_TAG_MAKE, EXIF_TYPE_ASCII,
                 strlen((char *)exifInfo->maker) + 1, exifInfo->maker, &LongerTagOffest, pIfdStart);
    writeExifIfd(&pCur, EXIF_TAG_MODEL, EXIF_TYPE_ASCII,
                 strlen((char *)exifInfo->model) + 1, exifInfo->model, &LongerTagOffest, pIfdStart);
    writeExifIfd(&pCur, EXIF_TAG_ORIENTATION, EXIF_TYPE_SHORT,
                 1, exifInfo->orientation);
    writeExifIfd(&pCur, EXIF_TAG_SOFTWARE, EXIF_TYPE_ASCII,
                 strlen((char *)exifInfo->software) + 1, exifInfo->software, &LongerTagOffest, pIfdStart);
    writeExifIfd(&pCur, EXIF_TAG_DATE_TIME, EXIF_TYPE_ASCII,
                 20, exifInfo->date_time, &LongerTagOffest, pIfdStart);
    writeExifIfd(&pCur, EXIF_TAG_YCBCR_POSITIONING, EXIF_TYPE_SHORT,
                 1, exifInfo->ycbcr_positioning);
    writeExifIfd(&pCur, EXIF_TAG_EXIF_IFD_POINTER, EXIF_TYPE_LONG,
                 1, LongerTagOffest);
    if (exifInfo->enableGps) {
        pGpsIfdPtr = pCur;
        pCur += IFD_SIZE;   // Skip a ifd size for gps IFD pointer
    }

    pNextIfdOffset = pCur;  // Skip a offset size for next IFD offset
    pCur += OFFSET_SIZE;

    //2 0th IFD Exif Private Tags
    pCur = pIfdStart + LongerTagOffest;

    tmp = NUM_0TH_IFD_EXIF;
    memcpy(pCur, &tmp , NUM_SIZE);
    pCur += NUM_SIZE;

    LongerTagOffest += NUM_SIZE + NUM_0TH_IFD_EXIF*IFD_SIZE + OFFSET_SIZE;

    writeExifIfd(&pCur, EXIF_TAG_EXPOSURE_TIME, EXIF_TYPE_RATIONAL,
                 1, &exifInfo->exposure_time, &LongerTagOffest, pIfdStart);
    writeExifIfd(&pCur, EXIF_TAG_FNUMBER, EXIF_TYPE_RATIONAL,
                 1, &exifInfo->fnumber, &LongerTagOffest, pIfdStart);
    writeExifIfd(&pCur, EXIF_TAG_EXPOSURE_PROGRAM, EXIF_TYPE_SHORT,
                 1, exifInfo->exposure_program);
    writeExifIfd(&pCur, EXIF_TAG_ISO_SPEED_RATING, EXIF_TYPE_SHORT,
                 1, exifInfo->iso_speed_rating);
    writeExifIfd(&pCur, EXIF_TAG_EXIF_VERSION, EXIF_TYPE_UNDEFINED,
                 4, exifInfo->exif_version);
    writeExifIfd(&pCur, EXIF_TAG_DATE_TIME_ORG, EXIF_TYPE_ASCII,
                 20, exifInfo->date_time, &LongerTagOffest, pIfdStart);
    writeExifIfd(&pCur, EXIF_TAG_DATE_TIME_DIGITIZE, EXIF_TYPE_ASCII,
                 20, exifInfo->date_time, &LongerTagOffest, pIfdStart);
    writeExifIfd(&pCur, EXIF_TAG_SHUTTER_SPEED, EXIF_TYPE_SRATIONAL,
                 1, (rational_t *)&exifInfo->shutter_speed, &LongerTagOffest, pIfdStart);
    writeExifIfd(&pCur, EXIF_TAG_APERTURE, EXIF_TYPE_RATIONAL,
                 1, &exifInfo->aperture, &LongerTagOffest, pIfdStart);
    writeExifIfd(&pCur, EXIF_TAG_BRIGHTNESS, EXIF_TYPE_SRATIONAL,
                 1, (rational_t *)&exifInfo->brightness, &LongerTagOffest, pIfdStart);
    writeExifIfd(&pCur, EXIF_TAG_EXPOSURE_BIAS, EXIF_TYPE_SRATIONAL,
                 1, (rational_t *)&exifInfo->exposure_bias, &LongerTagOffest, pIfdStart);
    writeExifIfd(&pCur, EXIF_TAG_MAX_APERTURE, EXIF_TYPE_RATIONAL,
                 1, &exifInfo->max_aperture, &LongerTagOffest, pIfdStart);
    writeExifIfd(&pCur, EXIF_TAG_METERING_MODE, EXIF_TYPE_SHORT,
                 1, exifInfo->metering_mode);
    writeExifIfd(&pCur, EXIF_TAG_FLASH, EXIF_TYPE_SHORT,
                 1, exifInfo->flash);
    writeExifIfd(&pCur, EXIF_TAG_FOCAL_LENGTH, EXIF_TYPE_RATIONAL,
                 1, &exifInfo->focal_length, &LongerTagOffest, pIfdStart);
    writeExifIfd(&pCur, EXIF_TAG_MAKER_NOTE, EXIF_TYPE_UNDEFINED,
                 exifInfo->maker_note_size, exifInfo->maker_note, &LongerTagOffest, pIfdStart);
    char code[8] = { 0x00, 0x00, 0x00, 0x49, 0x49, 0x43, 0x53, 0x41 };
    int commentsLen = strlen((char *)exifInfo->user_comment) + 1;
    memmove(exifInfo->user_comment + sizeof(code), exifInfo->user_comment, commentsLen);
    memcpy(exifInfo->user_comment, code, sizeof(code));
    writeExifIfd(&pCur, EXIF_TAG_USER_COMMENT, EXIF_TYPE_UNDEFINED,
                 commentsLen + sizeof(code), exifInfo->user_comment, &LongerTagOffest, pIfdStart);
    writeExifIfd(&pCur, EXIF_TAG_COLOR_SPACE, EXIF_TYPE_SHORT,
                 1, exifInfo->color_space);
    writeExifIfd(&pCur, EXIF_TAG_PIXEL_X_DIMENSION, EXIF_TYPE_LONG,
                 1, exifInfo->width);
    writeExifIfd(&pCur, EXIF_TAG_PIXEL_Y_DIMENSION, EXIF_TYPE_LONG,
                 1, exifInfo->height);

    pInteroperabilityIfdPtr = pCur;
    pCur += IFD_SIZE;   // Skip a ifd size for interoperability IFD pointer

    writeExifIfd(&pCur, EXIF_TAG_EXPOSURE_MODE, EXIF_TYPE_LONG,
                 1, exifInfo->exposure_mode);
    writeExifIfd(&pCur, EXIF_TAG_WHITE_BALANCE, EXIF_TYPE_LONG,
                 1, exifInfo->white_balance);
    writeExifIfd(&pCur, EXIF_TAG_FOCA_LENGTH_IN_35MM_FILM, EXIF_TYPE_LONG,
                 1, exifInfo->focal_length_in_35mm_length);
    writeExifIfd(&pCur, EXIF_TAG_SCENCE_CAPTURE_TYPE, EXIF_TYPE_LONG,
                 1, exifInfo->scene_capture_type);
    writeExifIfd(&pCur, EXIF_TAG_IMAGE_UNIQUE_ID, EXIF_TYPE_ASCII,
                 11, exifInfo->unique_id, &LongerTagOffest, pIfdStart);
    tmp = 0;
    memcpy(pCur, &tmp, OFFSET_SIZE); // next IFD offset
    pCur += OFFSET_SIZE;

    // 2 0th IFD Interoperability
    writeExifIfd(&pInteroperabilityIfdPtr, EXIF_TAG_INTEROPERABILITY, EXIF_TYPE_LONG,
                     1, LongerTagOffest); // Interoperability IFD pointer skipped on 0th IFD

    pCur = pIfdStart + LongerTagOffest;

    tmp = NUM_0TH_IFD_INTEROPERABILITY;
    memcpy(pCur, &tmp, NUM_SIZE);
    pCur += NUM_SIZE;

    LongerTagOffest += NUM_SIZE + NUM_0TH_IFD_INTEROPERABILITY * IFD_SIZE + OFFSET_SIZE;

    char interoperability_index_str[8];

    if (exifInfo->interoperability_index == 0)
        strncpy(interoperability_index_str, "R98", 3);
    else
        strncpy(interoperability_index_str, "THM", 3);

    writeExifIfd(&pCur, EXIF_TAG_INTEROPERABILITY_INDEX, EXIF_TYPE_ASCII,
                 4, (unsigned char *)interoperability_index_str);

    char interoperability_index_version[8];
    strncpy(interoperability_index_version, "0100", 4);

    writeExifIfd(&pCur, EXIF_TAG_INTEROPERABILITY_VERSION, EXIF_TYPE_UNDEFINED,
                 4, (unsigned char *)interoperability_index_version);

    tmp = 0;
    memcpy(pCur, &tmp, OFFSET_SIZE); // next IFD offset
    pCur += OFFSET_SIZE;

    //2 0th IFD GPS Info Tags
    if (exifInfo->enableGps) {
        writeExifIfd(&pGpsIfdPtr, EXIF_TAG_GPS_IFD_POINTER, EXIF_TYPE_LONG,
                     1, LongerTagOffest); // GPS IFD pointer skipped on 0th IFD

        pCur = pIfdStart + LongerTagOffest;

        if (exifInfo->gps_processing_method[0] == 0) {
            // don't create GPS_PROCESSING_METHOD tag if there isn't any
            tmp = NUM_0TH_IFD_GPS - 1;
        } else {
            tmp = NUM_0TH_IFD_GPS;
        }
        memcpy(pCur, &tmp, NUM_SIZE);
        pCur += NUM_SIZE;

        LongerTagOffest += NUM_SIZE + tmp*IFD_SIZE + OFFSET_SIZE;

        writeExifIfd(&pCur, EXIF_TAG_GPS_VERSION_ID, EXIF_TYPE_BYTE,
                     4, exifInfo->gps_version_id);
        writeExifIfd(&pCur, EXIF_TAG_GPS_LATITUDE_REF, EXIF_TYPE_ASCII,
                     2, exifInfo->gps_latitude_ref);
        writeExifIfd(&pCur, EXIF_TAG_GPS_LATITUDE, EXIF_TYPE_RATIONAL,
                     3, exifInfo->gps_latitude, &LongerTagOffest, pIfdStart);
        writeExifIfd(&pCur, EXIF_TAG_GPS_LONGITUDE_REF, EXIF_TYPE_ASCII,
                     2, exifInfo->gps_longitude_ref);
        writeExifIfd(&pCur, EXIF_TAG_GPS_LONGITUDE, EXIF_TYPE_RATIONAL,
                     3, exifInfo->gps_longitude, &LongerTagOffest, pIfdStart);
        writeExifIfd(&pCur, EXIF_TAG_GPS_ALTITUDE_REF, EXIF_TYPE_BYTE,
                     1, exifInfo->gps_altitude_ref);
        writeExifIfd(&pCur, EXIF_TAG_GPS_ALTITUDE, EXIF_TYPE_RATIONAL,
                     1, &exifInfo->gps_altitude, &LongerTagOffest, pIfdStart);
        writeExifIfd(&pCur, EXIF_TAG_GPS_TIMESTAMP, EXIF_TYPE_RATIONAL,
                     3, exifInfo->gps_timestamp, &LongerTagOffest, pIfdStart);
        tmp = strlen((char*)exifInfo->gps_processing_method);
        if (tmp > 0) {
            if (tmp > 100)
                tmp = 100;

            unsigned char tmp_buf[100+sizeof(ExifAsciiPrefix)];
            memcpy(tmp_buf, ExifAsciiPrefix, sizeof(ExifAsciiPrefix));
            memcpy(&tmp_buf[sizeof(ExifAsciiPrefix)], exifInfo->gps_processing_method, tmp);
            writeExifIfd(&pCur, EXIF_TAG_GPS_PROCESSING_METHOD, EXIF_TYPE_UNDEFINED,
                         tmp+sizeof(ExifAsciiPrefix), tmp_buf, &LongerTagOffest, pIfdStart);
        }
        writeExifIfd(&pCur, EXIF_TAG_GPS_DATESTAMP, EXIF_TYPE_ASCII,
                     11, exifInfo->gps_datestamp, &LongerTagOffest, pIfdStart);
        tmp = 0;
        memcpy(pCur, &tmp, OFFSET_SIZE); // next IFD offset
        pCur += OFFSET_SIZE;
    }

    //2 1th IFD TIFF Tags
    char *thumbBuf = NULL;
    unsigned int thumbSize = 0;
    int ret = ERROR_NONE;

    int iThumbFd = 0;
    int thumbBufSize = 0;

    if (exifInfo->enableThumb) {
        if (m_jpegMain->checkInBufType() & JPEG_BUF_TYPE_DMA_BUF) {
            if (useMainbufForThumb) {
                ret = m_jpegMain->getOutBuf((int *)&iThumbFd, (int *)&thumbBufSize);
                if (ret != ERROR_NONE)
                    iThumbFd = -1;

                thumbSize = (unsigned int)m_jpegMain->getJpegSize();
            } else {
                ret = m_jpegThumb->getOutBuf((int *)&iThumbFd, (int *)&thumbBufSize);
                if (ret != ERROR_NONE)
                    iThumbFd = -1;

                thumbSize = (unsigned int)m_jpegThumb->getJpegSize();
            }

            if (mmapJpegMemory(&iThumbFd, &thumbBuf, &thumbBufSize, MAX_OUTPUT_BUFFER_PLANE_NUM) == false) {
                ALOGE("ERR(%s): mmapJpegMemory() fail", __func__);

                unmapJpegMemory(&iThumbFd, &thumbBuf, &thumbBufSize, MAX_OUTPUT_BUFFER_PLANE_NUM);
                return ERROR_MEM_ALLOC_FAIL;
            }
        }

        if (m_jpegMain->checkInBufType() & JPEG_BUF_TYPE_USER_PTR) {
            if (useMainbufForThumb) {
                ret = m_jpegMain->getOutBuf((char **)&thumbBuf, (int *)&thumbSize);
                if (ret != ERROR_NONE)
                    thumbBuf = NULL;

                thumbSize = (unsigned int)m_jpegMain->getJpegSize();
            } else {
                ret = m_jpegThumb->getOutBuf((char **)&thumbBuf, (int *)&thumbSize);
                if (ret != ERROR_NONE)
                    thumbBuf = NULL;

                thumbSize = (unsigned int)m_jpegThumb->getJpegSize();
            }
        }
    }

    if (exifInfo->enableThumb && (thumbBuf != NULL) && (thumbSize != 0)) {
        exifSizeExceptThumb = tmp = LongerTagOffest;
        memcpy(pNextIfdOffset, &tmp, OFFSET_SIZE);  // NEXT IFD offset skipped on 0th IFD

        pCur = pIfdStart + LongerTagOffest;

        tmp = NUM_1TH_IFD_TIFF;
        memcpy(pCur, &tmp, NUM_SIZE);
        pCur += NUM_SIZE;

        LongerTagOffest += NUM_SIZE + NUM_1TH_IFD_TIFF*IFD_SIZE + OFFSET_SIZE;

        writeExifIfd(&pCur, EXIF_TAG_IMAGE_WIDTH, EXIF_TYPE_LONG,
                     1, exifInfo->widthThumb);
        writeExifIfd(&pCur, EXIF_TAG_IMAGE_HEIGHT, EXIF_TYPE_LONG,
                     1, exifInfo->heightThumb);
        writeExifIfd(&pCur, EXIF_TAG_COMPRESSION_SCHEME, EXIF_TYPE_SHORT,
                     1, exifInfo->compression_scheme);
        writeExifIfd(&pCur, EXIF_TAG_ORIENTATION, EXIF_TYPE_SHORT,
                     1, exifInfo->orientation);
        writeExifIfd(&pCur, EXIF_TAG_X_RESOLUTION, EXIF_TYPE_RATIONAL,
                     1, &exifInfo->x_resolution, &LongerTagOffest, pIfdStart);
        writeExifIfd(&pCur, EXIF_TAG_Y_RESOLUTION, EXIF_TYPE_RATIONAL,
                     1, &exifInfo->y_resolution, &LongerTagOffest, pIfdStart);
        writeExifIfd(&pCur, EXIF_TAG_RESOLUTION_UNIT, EXIF_TYPE_SHORT,
                     1, exifInfo->resolution_unit);
        writeExifIfd(&pCur, EXIF_TAG_JPEG_INTERCHANGE_FORMAT, EXIF_TYPE_LONG,
                     1, LongerTagOffest);
        writeExifIfd(&pCur, EXIF_TAG_JPEG_INTERCHANGE_FORMAT_LEN, EXIF_TYPE_LONG,
                     1, thumbSize);

        tmp = 0;
        memcpy(pCur, &tmp, OFFSET_SIZE); // next IFD offset
        pCur += OFFSET_SIZE;

        memcpy(pIfdStart + LongerTagOffest,
               thumbBuf, thumbSize);
        LongerTagOffest += thumbSize;
        if (LongerTagOffest > EXIF_LIMIT_SIZE) {
            ALOGE("ERR(%s):ExifTagOffset(%d) is too bigger than EXIF_LIMIT_SIZE(%d)",
                  __func__, LongerTagOffest, EXIF_LIMIT_SIZE);

            LongerTagOffest = exifSizeExceptThumb;
            tmp = 0;
            memcpy(pNextIfdOffset, &tmp, OFFSET_SIZE);  // NEXT IFD offset skipped on 0th IFD
        }
    } else {
        tmp = 0;
        memcpy(pNextIfdOffset, &tmp, OFFSET_SIZE);  // NEXT IFD offset skipped on 0th IFD
    }

    unsigned char App1Marker[2] = { 0xff, 0xe1 };
    memcpy(pApp1Start, App1Marker, 2);
    pApp1Start += 2;

    *size = 10 + LongerTagOffest;
    tmp = *size - 2;    // APP1 Maker isn't counted
    unsigned char size_mm[2] = {(tmp >> 8) & 0xFF, tmp & 0xFF};
    memcpy(pApp1Start, size_mm, 2);

    if (m_jpegMain->checkInBufType() & JPEG_BUF_TYPE_DMA_BUF)
        unmapJpegMemory(&iThumbFd, &thumbBuf, &thumbBufSize, MAX_OUTPUT_BUFFER_PLANE_NUM);

    return ERROR_NONE;
}

/*
 * private member functions
*/
inline void ExynosJpegEncoderForCamera::writeExifIfd(unsigned char **pCur,
                                             unsigned short tag,
                                             unsigned short type,
                                             unsigned int count,
                                             unsigned int value)
{
    memcpy(*pCur, &tag, 2);
    *pCur += 2;
    memcpy(*pCur, &type, 2);
    *pCur += 2;
    memcpy(*pCur, &count, 4);
    *pCur += 4;
    memcpy(*pCur, &value, 4);
    *pCur += 4;
}

inline void ExynosJpegEncoderForCamera::writeExifIfd(unsigned char **pCur,
                                             unsigned short tag,
                                             unsigned short type,
                                             unsigned int count,
                                             unsigned char *pValue)
{
    char buf[4] = { 0,};

    memcpy(buf, pValue, count);
    memcpy(*pCur, &tag, 2);
    *pCur += 2;
    memcpy(*pCur, &type, 2);
    *pCur += 2;
    memcpy(*pCur, &count, 4);
    *pCur += 4;
    memcpy(*pCur, buf, 4);
    *pCur += 4;
}

inline void ExynosJpegEncoderForCamera::writeExifIfd(unsigned char **pCur,
                                             unsigned short tag,
                                             unsigned short type,
                                             unsigned int count,
                                             unsigned char *pValue,
                                             unsigned int *offset,
                                             unsigned char *start)
{
    memcpy(*pCur, &tag, 2);
    *pCur += 2;
    memcpy(*pCur, &type, 2);
    *pCur += 2;
    memcpy(*pCur, &count, 4);
    *pCur += 4;
    memcpy(*pCur, offset, 4);
    *pCur += 4;
    memcpy(start + *offset, pValue, count);
    *offset += count;
}

inline void ExynosJpegEncoderForCamera::writeExifIfd(unsigned char **pCur,
                                             unsigned short tag,
                                             unsigned short type,
                                             unsigned int count,
                                             rational_t *pValue,
                                             unsigned int *offset,
                                             unsigned char *start)
{
    memcpy(*pCur, &tag, 2);
    *pCur += 2;
    memcpy(*pCur, &type, 2);
    *pCur += 2;
    memcpy(*pCur, &count, 4);
    *pCur += 4;
    memcpy(*pCur, offset, 4);
    *pCur += 4;
    memcpy(start + *offset, pValue, 8 * count);
    *offset += 8 * count;
}


int ExynosJpegEncoderForCamera::roundToInt(double x)
{
    if (x >= 0)
	    return (int) (x + 0.5);
    return (int) (x - 0.5);
}

int ExynosJpegEncoderForCamera::get_approximate_value(int value, int limit)
{
    int x;
    if( value % 4 != 0 ) {
        for(x = 0; x < 100 && value < limit; x++) {
            if( value % 4 == 0)
                break;
            else
                value++;
        }
    }
    return value;
}

int ExynosJpegEncoderForCamera::scaleDownYuv422(char **srcBuf, unsigned int srcW, unsigned int srcH,  char **dstBuf, unsigned int dstW, unsigned int dstH)
{
    double step_x, step_y;
    int src_y_start_pos, dst_pos, src_pos;
    char *src_buf = srcBuf[0];
    char *dst_buf = dstBuf[0];

    if (dstW & 0x01 || dstH & 0x01)
        return ERROR_INVALID_SCALING_WIDTH_HEIGHT;

    step_x = (double)srcW / (double)dstW;
    step_y = (double)srcH / (double)dstH;

    double srcWStride = srcW * 2;
    double stepXStride = step_x * 2;

    dst_pos = 0;
    for (unsigned int y = 0; y < dstH; y++) {
        src_y_start_pos = srcWStride * get_approximate_value( roundToInt(step_y * (double)y), dstH*srcWStride);

        for (unsigned int x = 0; x < dstW; x += 2) {
            src_pos = src_y_start_pos + get_approximate_value( roundToInt(stepXStride * (double)x), dstW*stepXStride);
            if (src_pos >= (srcW * srcH * 2)) break;
            dst_buf[dst_pos++] = src_buf[src_pos    ];
            dst_buf[dst_pos++] = src_buf[src_pos + 1];
            dst_buf[dst_pos++] = src_buf[src_pos + 2];
            dst_buf[dst_pos++] = src_buf[src_pos + 3];
        }
    }

    return ERROR_NONE;
}

int ExynosJpegEncoderForCamera::scaleDownYuv422_2p(char **srcBuf, unsigned int srcW, unsigned int srcH, char **dstBuf, unsigned int dstW, unsigned int dstH)
{
    int32_t step_x, step_y;
    int32_t src_y_start_pos, dst_pos, src_pos;
    int32_t src_Y_offset;
    char *src_buf;
    char *dst_buf;

    if (dstW % 2 != 0 || dstH % 2 != 0)
        return ERROR_INVALID_SCALING_WIDTH_HEIGHT;

    step_x = srcW / dstW;
    step_y = srcH / dstH;

    // Y scale down
    src_buf = srcBuf[0];
    dst_buf = dstBuf[0];
    dst_pos = 0;
    for (uint32_t y = 0; y < dstH; y++) {
        src_y_start_pos = y * step_y * srcW;

        for (uint32_t x = 0; x < dstW; x++) {
            src_pos = src_y_start_pos + (x * step_x);

            dst_buf[dst_pos++] = src_buf[src_pos];
        }
    }

    // UV scale down
    src_buf = srcBuf[1];
    dst_buf = dstBuf[1];
    dst_pos = 0;
    for (uint32_t i = 0; i < dstH; i++) {
        src_y_start_pos = i * step_y * srcW;

        for (uint32_t j = 0; j < dstW; j += 2) {
            src_pos = src_y_start_pos + (j * step_x);

            dst_buf[dst_pos++] = src_buf[src_pos    ];
            dst_buf[dst_pos++] = src_buf[src_pos + 1];
        }
    }

    return ERROR_NONE;
}

// thumbnail
int ExynosJpegEncoderForCamera::setThumbnailSize(int w, int h)
{
    if (m_flagCreate == false)
        return ERROR_CANNOT_CREATE_EXYNOS_JPEG_ENC_HAL;

    if (w < 0 || MAX_JPG_WIDTH < w)
        return false;

    if (h < 0 || MAX_JPG_HEIGHT < h)
        return false;

    m_thumbnailW = w;
    m_thumbnailH = h;
    return ERROR_NONE;
}

int ExynosJpegEncoderForCamera::setThumbnailQuality(int quality)
{
    if (m_flagCreate == false)
        return ERROR_CANNOT_CREATE_EXYNOS_JPEG_ENC_HAL;

    if (quality < 1 || 100 < quality)
        return false;

    m_thumbnailQuality = quality;
    return ERROR_NONE;
}

int ExynosJpegEncoderForCamera::encodeThumbnail(unsigned int *size, bool useMain)
{
    int ret = ERROR_NONE;

    if (m_flagCreate == false)
        return ERROR_CANNOT_CREATE_EXYNOS_JPEG_ENC_HAL;

    // create jpeg thumbnail class
    if (m_jpegThumb == NULL) {
        m_jpegThumb = new ExynosJpegEncoder;

        if (m_jpegThumb == NULL) {
            ALOGE("ERR(%s):Cannot open a jpeg device file", __func__);
            return ERROR_CANNOT_CREATE_SEC_THUMB;
        }
    }

    ret = m_jpegThumb->create();
    if (ret) {
        ALOGE("ERR(%s):Fail create", __func__);
        return ret;
    }

        ret = m_jpegThumb->setCache(JPEG_CACHE_ON);
    if (ret) {
        ALOGE("ERR(%s):Fail cache set", __func__);
        return ret;
    }

    void *pConfig = m_jpegMain->getJpegConfig();
    if (pConfig == NULL) {
        ALOGE("ERR(%s):Fail getJpegConfig", __func__);
        return ERROR_BUFFR_IS_NULL;
    }

    ret = m_jpegThumb->setJpegConfig(pConfig);
    if (ret) {
        ALOGE("ERR(%s):Fail setJpegConfig", __func__);
        return ret;
    }

    ret = m_jpegThumb->setSize(m_thumbnailW, m_thumbnailH);
    if (ret) {
        ALOGE("ERR(%s):Fail setSize", __func__);
        return ret;
    }

    freeJpegMemory(&m_stThumbInBuf, MAX_IMAGE_PLANE_NUM);
    freeJpegMemory(&m_stThumbOutBuf, MAX_IMAGE_PLANE_NUM);

    if (m_jpegThumb->setColorBufSize(m_stThumbInBuf.iSize, MAX_IMAGE_PLANE_NUM) != ERROR_NONE)
        return ERROR_INVALID_COLOR_FORMAT;

    m_stThumbOutBuf.iSize[0] = sizeof(char)*m_thumbnailW*m_thumbnailH*THUMBNAIL_IMAGE_PIXEL_SIZE;

    if (allocJpegMemory(&m_stThumbInBuf, MAX_IMAGE_PLANE_NUM) != ERROR_NONE)
        return ERROR_MEM_ALLOC_FAIL;

    if (allocJpegMemory(&m_stThumbOutBuf, MAX_IMAGE_PLANE_NUM) != ERROR_NONE)
        return ERROR_MEM_ALLOC_FAIL;

    /* Thumbnail InBuf is DMA_BUF */
    ret = m_jpegThumb->setInBuf(m_stThumbInBuf.ionBuffer, m_stThumbInBuf.iSize);

    if (ret) {
        ALOGE("ERR(%s):Fail setInBuf", __func__);
        return ret;
    }

    /* Thumbnail OutBuf is DMA_BUF */
    ret = m_jpegThumb->setOutBuf(m_stThumbOutBuf.ionBuffer[0], m_stThumbOutBuf.iSize[0]);

    if (ret) {
        ALOGE("ERR(%s):Fail setOutBuf", __func__);
        return ret;
    }

    if (useMain) {
        int iTempWidth=0;
        int iTempHeight=0;

        int iMainInputBuf[MAX_INPUT_BUFFER_PLANE_NUM] = {NULL,};
        char *pcMainInputBuf[MAX_INPUT_BUFFER_PLANE_NUM] = {NULL,};
        char *pcThumbInputBuf[MAX_INPUT_BUFFER_PLANE_NUM] = {NULL,};
        int iMainInputSize[MAX_INPUT_BUFFER_PLANE_NUM] = {NULL,};
        int iThumbInputSize[MAX_INPUT_BUFFER_PLANE_NUM] = {NULL,};
        int iTempColorformat = 0;

        iTempColorformat = m_jpegMain->getColorFormat();

        ret = m_jpegMain->getSize(&iTempWidth, &iTempHeight);
        if (ret) {
            ALOGE("ERR(%s):Fail getSize", __func__);
            return ret;
        }

        if (m_jpegMain->checkInBufType() & JPEG_BUF_TYPE_USER_PTR)
            ret = m_jpegMain->getInBuf(pcMainInputBuf, iMainInputSize, MAX_INPUT_BUFFER_PLANE_NUM);
        else if (m_jpegMain->checkInBufType() & JPEG_BUF_TYPE_DMA_BUF)
            ret = m_jpegMain->getInBuf(iMainInputBuf, iMainInputSize, MAX_INPUT_BUFFER_PLANE_NUM);
        else
            return ERROR_BUFFR_IS_NULL;

        if (ret) {
            ALOGE("ERR(%s):Fail getInBuf", __func__);
            return ret;
        }

#if 0
        if (m_jpegMain->checkInBufType() & JPEG_BUF_TYPE_DMA_BUF) {
            if (mmapJpegMemory(iMainInputBuf, pcMainInputBuf, iMainInputSize, MAX_INPUT_BUFFER_PLANE_NUM) == false) {
                ALOGE("ERR(%s): mmapJpegMemory() fail", __func__);

                unmapJpegMemory(iMainInputBuf, pcMainInputBuf, iMainInputSize, MAX_INPUT_BUFFER_PLANE_NUM);
                return ERROR_MEM_ALLOC_FAIL;
            }
        }
#endif

        pcThumbInputBuf[0] = m_stThumbInBuf.pcBuf[0];
        pcThumbInputBuf[1] = (char *)(MAP_FAILED);

        /* using H/W scaler */
        CSC_METHOD cscMethod = CSC_METHOD_HW;

        m_exynosThumbCSC = csc_init(cscMethod);
        if (m_exynosThumbCSC == NULL)
            ALOGE("ERR(%s):csc_init() fail", __func__);
        csc_set_hw_property(m_exynosThumbCSC, CSC_HW_PROPERTY_FIXED_NODE, 1);
        /********************/

        switch (iTempColorformat) {
        case V4L2_PIX_FMT_YUYV:
            if (m_jpegMain->checkInBufType() & JPEG_BUF_TYPE_DMA_BUF) {
#if 1
                if (m_exynosThumbCSC) {
                    csc_set_src_format(m_exynosThumbCSC,
                        ALIGN_DOWN(iTempWidth,16), ALIGN_DOWN(iTempHeight,16),
                        0, 0, ALIGN(iTempWidth, 16), ALIGN_DOWN( iTempHeight,16),
                        V4L2_PIX_2_HAL_PIXEL_FORMAT(V4L2_PIX_FMT_YUYV), 1);
                    csc_set_dst_format(m_exynosThumbCSC,
                        m_thumbnailW, m_thumbnailH,
                        0, 0, m_thumbnailW, m_thumbnailH,
                        V4L2_PIX_2_HAL_PIXEL_FORMAT(V4L2_PIX_FMT_YUYV), 1);
                    csc_set_src_buffer(m_exynosThumbCSC,
                        (void**)iMainInputBuf, CSC_MEMORY_DMABUF);
                    csc_set_dst_buffer(m_exynosThumbCSC,
                        (void**)m_stThumbInBuf.ionBuffer, CSC_MEMORY_DMABUF);
                    if (csc_convert(m_exynosThumbCSC) != 0)
                        ALOGE("ERR(%s) dmabuf : csc_convert() from gralloc to callback fail", __func__);
                }
#else
                ALOGE("@ f_width = %d, f_height = %d, crop_width = %d, crop_height = %d",
                       iTempWidth, iTempHeight, m_thumbnailW, m_thumbnailH);

                ret = scaleDownYuv422(pcMainInputBuf,
                                  iTempWidth,
                                  iTempHeight,
                                  m_stThumbInBuf.pcBuf,
                                  m_thumbnailW,
                                  m_thumbnailH);
#endif
            } else if (m_jpegMain->checkInBufType() & JPEG_BUF_TYPE_USER_PTR) {
#if 1
                if (m_exynosThumbCSC) {
                    csc_set_src_format(m_exynosThumbCSC,
                        ALIGN_DOWN(iTempWidth,16), ALIGN_DOWN(iTempHeight,16),
                        0, 0, ALIGN_DOWN(iTempWidth,16), ALIGN_DOWN(iTempHeight,16),
                        V4L2_PIX_2_HAL_PIXEL_FORMAT(V4L2_PIX_FMT_YUYV), 1);
                    csc_set_dst_format(m_exynosThumbCSC,
                        m_thumbnailW, m_thumbnailH,
                        0, 0, m_thumbnailW, m_thumbnailH,
                        V4L2_PIX_2_HAL_PIXEL_FORMAT(V4L2_PIX_FMT_YUYV), 1);
                    csc_set_src_buffer(m_exynosThumbCSC,
                        (void**)pcMainInputBuf, CSC_MEMORY_USERPTR);
                    csc_set_dst_buffer(m_exynosThumbCSC,
                        (void**)m_stThumbInBuf.pcBuf, CSC_MEMORY_USERPTR);
                    if (csc_convert(m_exynosThumbCSC) != 0)
                        ALOGE("ERR(%s) userptr : csc_convert() from gralloc to callback fail", __func__);
                }
#else

                ret = scaleDownYuv422(pcMainInputBuf,
                              iTempWidth,
                              iTempHeight,
                              pcThumbInputBuf,
                              m_thumbnailW,
                              m_thumbnailH);
#endif
            } else {
#if 1
                if (m_exynosThumbCSC)
                    csc_deinit(m_exynosThumbCSC);
                m_exynosThumbCSC = NULL;
#endif
                return ERROR_BUFFR_IS_NULL;
            }
            break;
        case V4L2_PIX_FMT_NV16:
            pcMainInputBuf[1] = pcMainInputBuf[0] + (iTempWidth*iTempHeight);
            pcThumbInputBuf[1] = pcThumbInputBuf[0] + (m_thumbnailW*m_thumbnailH);
            if (m_jpegMain->checkInBufType() & JPEG_BUF_TYPE_DMA_BUF) {
                ret = scaleDownYuv422_2p(pcMainInputBuf,
                                  iTempWidth,
                                  iTempHeight,
                                  m_stThumbInBuf.pcBuf,
                                  m_thumbnailW,
                                  m_thumbnailH);
            } else if (m_jpegMain->checkInBufType() & JPEG_BUF_TYPE_USER_PTR) {
                ret = scaleDownYuv422_2p(pcMainInputBuf,
                              iTempWidth,
                              iTempHeight,
                              pcThumbInputBuf,
                              m_thumbnailW,
                              m_thumbnailH);
            } else {
#if 1
                if (m_exynosThumbCSC)
                    csc_deinit(m_exynosThumbCSC);
                m_exynosThumbCSC = NULL;
#endif

                return ERROR_BUFFR_IS_NULL;
            }
            break;
        default:
#if 1
            if (m_exynosThumbCSC)
                csc_deinit(m_exynosThumbCSC);
            m_exynosThumbCSC = NULL;
#endif

            return ERROR_INVALID_COLOR_FORMAT;
            break;
        }

#if 1
    if (m_exynosThumbCSC)
        csc_deinit(m_exynosThumbCSC);
    m_exynosThumbCSC = NULL;
#endif

        pcMainInputBuf[1] = (char *)(MAP_FAILED);
        if (m_jpegMain->checkInBufType() & JPEG_BUF_TYPE_DMA_BUF)
            unmapJpegMemory(iMainInputBuf, pcMainInputBuf, iMainInputSize, MAX_INPUT_BUFFER_PLANE_NUM);

        if (ret) {
            ALOGE("ERR(%s):scaleDown(%d, %d, %d, %d) fail", __func__, iTempWidth, iTempHeight, m_thumbnailW, m_thumbnailH);
            return ret;
        }
    }
    else {
        return ERROR_IMPLEMENT_NOT_YET;
    }

    int iOutSizeThumb;
    unsigned int quality_array[3] = {38, 30, 10};
    unsigned int quality_index = 0;

    /* Retry encode thumbnail */
    do {
        ret = m_jpegThumb->setQuality(quality_array[quality_index++]);
        if (ret) {
            ALOGE("ERR(%s):Fail setQuality", __func__);
            return ret;
        }

        ret = m_jpegThumb->updateConfig();
        if (ret) {
            ALOGE("update config failed");
            return ret;
        }

        ret = m_jpegThumb->encode();
        if (ret) {
            ALOGE("encode failed");
            return ret;
        }

        iOutSizeThumb = m_jpegThumb->getJpegSize();
        if (iOutSizeThumb <= 0) {
            ALOGE("jpeg size is wrong!");
            return ERROR_THUMB_JPEG_SIZE_TOO_SMALL;
        }
    } while(iOutSizeThumb >= EXIF_LIMIT_SIZE - EXIF_INFO_LIMIT_SIZE
                    && quality_index < sizeof(quality_array) / sizeof(unsigned int));

    *size = (unsigned int)iOutSizeThumb;

    return ERROR_NONE;
}

int ExynosJpegEncoderForCamera::createIonClient(ion_client ionClient)
{
    if (ionClient == 0) {
        ionClient = ion_client_create();
        if (ionClient < 0) {
            ALOGE("[%s]src ion client create failed, value = %d", __func__, ionClient);
            return 0;
        }
    }

    return ionClient;
}

int ExynosJpegEncoderForCamera::deleteIonClient(ion_client ionClient)
{
    if (ionClient != 0) {
        if (ionClient > 0)
            ion_client_destroy(ionClient);

        ionClient = 0;
    }

    return ionClient;
}

int ExynosJpegEncoderForCamera::allocJpegMemory(struct stJpegMem *pstMem, int iMemoryNum)
{
    int ret = ERROR_NONE;
    int i = 0;

    if (pstMem->ionClient == 0) {
        ALOGE("[%s] i = %d , ionClient is zero (%d)", __func__, i, pstMem->ionClient);
        return ERROR_BUFFR_IS_NULL;
    }

    for (i=0;i<iMemoryNum;i++) {
        if (pstMem->iSize[i] == 0)
            break;

        pstMem->ionBuffer[i] = ion_alloc(pstMem->ionClient, \
                                                        pstMem->iSize[i], 0, ION_HEAP_SYSTEM_MASK, 0);
        if ((pstMem->ionBuffer[i] == -1) ||(pstMem->ionBuffer[i] == 0)) {
            ALOGE("[%s]ion_alloc(%d) failed", __func__, pstMem->iSize[i]);
            pstMem->ionBuffer[i] = -1;
            freeJpegMemory(pstMem, iMemoryNum);
            return ERROR_MEM_ALLOC_FAIL;
        }

        pstMem->pcBuf[i] = (char *)ion_map(pstMem->ionBuffer[i], \
                                                            pstMem->iSize[i], 0);
        if ((pstMem->pcBuf[i] == (char *)MAP_FAILED) || (pstMem->pcBuf[i] == NULL)) {
            ALOGE("[%s]src ion map failed(%d)", __func__, pstMem->iSize[i]);
            pstMem->pcBuf[i] = (char *)MAP_FAILED;
            freeJpegMemory(pstMem, iMemoryNum);
            return ERROR_MEM_ALLOC_FAIL;
        }
    }

    return ERROR_NONE;
}

void ExynosJpegEncoderForCamera::freeJpegMemory(struct stJpegMem *pstMem, int iMemoryNum)
{
    int i =0 ;
    if (pstMem->ionClient == 0)
        return;

    for (i=0;i<iMemoryNum;i++) {
        if (pstMem->ionBuffer[i] != -1) {
            if (pstMem->pcBuf[i] != (char *)MAP_FAILED)
                ion_unmap(pstMem->pcBuf[i], pstMem->iSize[i]);

            ion_free(pstMem->ionBuffer[i]);
        }
        pstMem->ionBuffer[i] = -1;
        pstMem->pcBuf[i] = (char *)MAP_FAILED;
        pstMem->iSize[i] = 0;
    }
}

void ExynosJpegEncoderForCamera::initJpegMemory(struct stJpegMem *pstMem, int iMemoryNum)
{
    int i =0 ;
    for (i=0;i<iMemoryNum;i++) {
        pstMem->pcBuf[i] = (char *)MAP_FAILED;
        pstMem->ionBuffer[i] = -1;
        pstMem->iSize[i] = 0;
    }
    pstMem->ionClient = 0;
}

bool ExynosJpegEncoderForCamera::mmapJpegMemory(int *iFd, char **ppcBuf, int *piSize, int iMemoryNum)
{
    bool ret = true;

    for (int i = 0; i < iMemoryNum; i++) {
        if (piSize[i] != 0) {
            ppcBuf[i] = (char *)ion_map(iFd[i], piSize[i], 0);
            if ((ppcBuf[i] == (char *)MAP_FAILED) || (ppcBuf[i] == NULL)) {
                ALOGE("[%s]ion map failed(size[%u])", __func__, (unsigned int)piSize[i]);

                ppcBuf[i] = (char *)MAP_FAILED;
                ret = false;
            }
        } else {
            ppcBuf[i] = (char *)MAP_FAILED;
        }
    }

    return ret;
}

void ExynosJpegEncoderForCamera::unmapJpegMemory(int *iFd, char **ppcBuf, int *piSize, int iMemoryNum)
{
    for (int i = 0; i < iMemoryNum; i++) {
        if (ppcBuf[i] != (char *)MAP_FAILED)
            ion_unmap(ppcBuf[i], piSize[i]);

        ppcBuf[i] = (char *)MAP_FAILED;
        piSize[i] = 0;
    }
}
