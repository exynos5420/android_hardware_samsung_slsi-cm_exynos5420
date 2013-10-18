/*
Copyright  © Trustonic Limited 2013

All rights reserved.

Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright notice, this 
     list of conditions and the following disclaimer.

  2. Redistributions in binary form must reproduce the above copyright notice, 
     this list of conditions and the following disclaimer in the documentation 
     and/or other materials provided with the distribution.

  3. Neither the name of the Trustonic Limited nor the names of its contributors 
     may be used to endorse or promote products derived from this software 
     without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED 
OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <string.h>
#include <stdlib.h>
#include <TlCm/tlCmUuid.h>
#include "tools.h"
#include "logging.h"
#include "trustletchannel.h"


/* keep this in global variable with file scope so that we can easily start
start using other than default device id if need arises */

static uint32_t tltChannelDeviceId=MC_DEVICE_ID_DEFAULT;

/*
Open session to content management trustlet and allocate enough memory for communication
*/
CMTHANDLE tltChannelOpen(int sizeOfWsmBuffer,  mcResult_t* result){
    CMTHANDLE           handle = (CMTHANDLE)malloc(sizeof(CMTSTRUCT));
    const mcUuid_t      UUID = TL_CM_UUID;


    if (unlikely( NULL==handle ))
    {
        *result=MC_DRV_ERR_NO_FREE_MEMORY;
        return NULL;
    }

    memset(handle,0,sizeof(CMTSTRUCT));

#if ! ( defined(LINUX) || (defined(WIN32) && defined(_TEST_SUITE)) )

    *result = mcOpenDevice(tltChannelDeviceId);

    if (MC_DRV_OK != *result) 
    {
      LOGE("tltChannelOpen: Unable to open device, error: %d", *result);
      free(handle);

      return NULL;
    }

#endif

    *result = mcMallocWsm(tltChannelDeviceId, 0, sizeOfWsmBuffer, &handle->wsmP, 0);
    if (MC_DRV_OK != *result) 
    {
        LOGE("tltChannelOpen: Allocation of CMP WSM failed, error: %d", *result);
        mcCloseDevice(tltChannelDeviceId);
        free(handle);
        return NULL;
    }

    *result = mcOpenSession(&handle->session,(const mcUuid_t *)&UUID,handle->wsmP,(uint32_t)sizeOfWsmBuffer);
    if (MC_DRV_OK != *result)
    {
        LOGE("tltChannelOpen: Open session failed, error: %d", *result);
        mcFreeWsm(tltChannelDeviceId,handle->wsmP);
        mcCloseDevice(tltChannelDeviceId);
        free(handle);
        return NULL;
    }
    return handle;
}

/*
Close the communication channel and free resources
*/
void tltChannelClose(CMTHANDLE handle){
    mcResult_t          result;

    if (!bad_read_ptr(handle,sizeof(CMTSTRUCT)))
    {
        result = mcCloseSession(&handle->session);
        if (MC_DRV_OK != result) 
        {
        LOGE("tltChannelClose: Closing session failed:, error: %d", result);
        }

        if (NULL!=handle->wsmP) mcFreeWsm(tltChannelDeviceId, handle->wsmP);

#if ! ( defined(LINUX) || (defined(WIN32) && defined(_TEST_SUITE)) )
        result = mcCloseDevice(tltChannelDeviceId);
        if (MC_DRV_OK != result) 
        {
            LOGE("tltChannelClose: Closing MobiCore device failed, error: %d", result);
        }
#endif
        free(handle);
    }
}

/*
Initiate transfer of the data between NWD and SWD. The actual data needs to be copied to wsmP beforehand 
(and from it afterwards in case of response)
*/
bool tltChannelTransmit(CMTHANDLE handle, int timeout){
    if (unlikely(bad_write_ptr(handle,sizeof(CMTSTRUCT)))) return false;

    // Send CMP message to content management trustlet.

    handle->lasterror = mcNotify(&handle->session);

    if (unlikely( MC_DRV_OK!=handle->lasterror ))
    {
        LOGE("tltChannelTransmit: mcNotify failed, error: %d",handle->lasterror);
        return false;
    }

  // Wait for trustlet response.

    handle->lasterror = mcWaitNotification(&handle->session, timeout);

    if (unlikely( MC_DRV_OK!=handle->lasterror )) 
    {
        LOGE("tltChannelTransmit: Wait for response notification failed, error: %d", handle->lasterror);
        return false;
    }
    return true;
}
