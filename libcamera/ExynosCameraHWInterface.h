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

/*!
 * \file      ExynosCameraHWInterface.h
 * \brief     hearder file for Android Camera HAL
 * \author    thun.hwang(thun.hwang@samsung.com)
 * \date      2010/06/03
 *
 * <b>Revision History: </b>
 * - 2011/12/31 : thun.hwang(thun.hwang@samsung.com) \n
 *   Initial version
 *
 * - 2012/03/14 : sangwoo.park(sw5771.park@samsung.com) \n
 *   Change file, class name to ExynosXXX.
 *
 */

#ifndef EXYNOS_CAMERA_HW_INTERFACE_H
#define EXYNOS_CAMERA_HW_INTERFACE_H

#include <utils/threads.h>
#include <utils/RefBase.h>
#include <binder/MemoryBase.h>
#include <binder/MemoryHeapBase.h>
#include <hardware/camera.h>
#include <hardware/gralloc.h>
#include <camera/Camera.h>
#include <camera/CameraParameters.h>
#include <media/hardware/MetadataBufferType.h>

#include "exynos_format.h"
#include "csc.h"
#include "ExynosCamera.h"

#include <fcntl.h>
#include <sys/mman.h>

namespace android {

class ExynosCameraHWInterface : public virtual RefBase {
public:
    ExynosCameraHWInterface(){};

    virtual status_t    setPreviewWindow(preview_stream_ops *w) = 0;
    virtual void        setCallbacks(camera_notify_callback notify_cb,
                                     camera_data_callback data_cb,
                                     camera_data_timestamp_callback data_cb_timestamp,
                                     camera_request_memory get_memory,
                                     void *user) = 0;

    virtual void        enableMsgType(int32_t msgType) = 0;
    virtual void        disableMsgType(int32_t msgType) = 0;
    virtual bool        msgTypeEnabled(int32_t msgType) = 0;

    virtual status_t    startPreview() = 0;
    virtual void        stopPreview() = 0;
    virtual bool        previewEnabled() = 0;

    virtual status_t    storeMetaDataInBuffers(bool enable) = 0;

    virtual status_t    startRecording() = 0;
    virtual void        stopRecording() = 0;
    virtual bool        recordingEnabled() = 0;
    virtual void        releaseRecordingFrame(const void *opaque) = 0;

    virtual status_t    autoFocus() = 0;
    virtual status_t    cancelAutoFocus() = 0;

    virtual status_t    takePicture() = 0;
    virtual status_t    cancelPicture() = 0;

    virtual status_t    setParameters(const CameraParameters& params) = 0;
    virtual CameraParameters  getParameters() const = 0;
    virtual status_t    sendCommand(int32_t command, int32_t arg1, int32_t arg2) = 0;

    virtual void        release() = 0;

    virtual status_t    dump(int fd) const = 0;

    virtual inline  int         getCameraId() const = 0;

//private:

};

}; // namespace android

#endif
