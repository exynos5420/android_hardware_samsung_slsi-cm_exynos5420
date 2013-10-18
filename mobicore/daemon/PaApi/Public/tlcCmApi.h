/** @addtogroup TLC_CONTENT_MANAGER
 * @{
 * @file
 * Content Manager API.
 *
 * This header file describes the interface to the content management trustlet
 * connector (Content Manager).
 *
 * <!-- Copyright Giesecke & Devrient GmbH 2009 - 2012 -->
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "MobiCoreDriverApi.h"
#include "mcUuid.h"
#include "TlCm/2.0/tlCmApi.h"
#include "TlCm/tlCmError.h"

#ifdef __cplusplus
extern "C" {
#endif

    /** Opens a content management session.
     *
     * After opening a content management session the content manager is ready to
     * accecpt content management requests. Upon successful execution, this function
     * sets the cmp pointer to a content management protocol object that is to be
     * used for subsequent content management operations.
     *
     * @param [out] cmp Content management protocol object.
     *
     * @return MC_DRV_OK or error code.
     */
    mcResult_t cmOpen(cmp_t **cmp);

    /** Closes a content management session.
     *
     * @return MC_DRV_OK or error code.
     */
    mcResult_t cmClose(void);

    /** Performs a content management operation.
     *
     * Executes the command contained in the content management protocol object
     *
     * returned from cmOpen. The result of the content management operation is also
     * conveyed within the CMP object returned from cmOpen, unless the overall
     * operation was aborted due to a low-level driver error, in which case the
     * return value of this routine is different to MC_DRV_OK.
     *
     * @return Indication as to whether the content management operation was executed.
     * MC_DRV_OK denotes that the operation was executed (see response contents of
     * the cmp_t object for details about the result of the operation).
     */
    mcResult_t cmManage(void);

#ifdef __cplusplus
}
#endif

/** @} */

