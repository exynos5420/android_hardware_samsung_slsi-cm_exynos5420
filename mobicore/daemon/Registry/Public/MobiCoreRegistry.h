/**
 * @addtogroup MCD_MCDIMPL_DAEMON_REG
 * @{
 * G&D MobiCore Registry
 *
 * @file
 * Mobicore Driver Registry.
 */

/* <!-- Copyright Giesecke & Devrient GmbH 2009 - 2012 -->
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
#ifndef MOBICORE_REGISTRY_H_
#define MOBICORE_REGISTRY_H_

#include "MobiCoreDriverApi.h"
#include "mcContainer.h"

#ifdef __cplusplus
extern "C" {
#endif

    /** Stores an authentication token in registry.
     * @param  so Authentication token secure object.
     * @param  size Authentication token object size
     * @return MC_DRV_OK if successful, otherwise error code.
     */
    mcResult_t mcRegistryStoreAuthToken(void *so, uint32_t size);

    /** Reads an authentication token from registry.
     * @param[out] so Authentication token secure object.
     * @param[out] size Authentication token secure object size
     * @return MC_DRV_OK if successful, otherwise error code.
     */
    mcResult_t mcRegistryReadAuthToken(void *so, uint32_t *size);

    /** Deletes the authentication token secure object from the registry.
     * @return MC_DRV_OK if successful, otherwise error code.
     */
    mcResult_t mcRegistryDeleteAuthToken(void);

    /** Stores a root container secure object in the registry.
     * @param so Root container secure object.
     * @param size Root container secure object size
     * @return MC_DRV_OK if successful, otherwise error code.
     */
    mcResult_t mcRegistryStoreRoot(void *so, uint32_t size);

    /** Reads a root container secure object from the registry.
     * @param[out] so Root container secure object.
     * @param[out] size Root container secure object size
     * @return MC_DRV_OK if successful, otherwise error code.
     */
     mcResult_t mcRegistryReadRoot(void *so, uint32_t *size);

    /** Stores a service provider container secure object in the registry.
     * @param spid Service provider ID.
     * @param so Service provider container secure object.
     * @return MC_DRV_OK if successful, otherwise error code.
     */
    mcResult_t mcRegistryStoreSp(mcSpid_t spid, void *so, uint32_t size);

    /** Reads a service provider container secure object from the registry.
     * @param spid Service provider ID.
     * @param[out] so Service provider container secure object.
     * @param[out] size Service provider container secure object size
     * @return MC_DRV_OK if successful, otherwise error code.
     */
     mcResult_t mcRegistryReadSp(mcSpid_t spid, void *so, uint32_t *size);

    /** Deletes a service provider recursively, including all trustlets and
     * data.
     * @param spid Service provider ID.
     * @return MC_DRV_OK if successful, otherwise error code.
     */
    mcResult_t mcRegistryCleanupSp(mcSpid_t spid);

    /** Stores a trustlet container secure object in the registry.
     * @param uuid Trustlet UUID.
     * @param so Trustlet container secure object.
     * @param size Trustlet container secure object size
     * @return MC_DRV_OK if successful, otherwise error code.
     */
    mcResult_t mcRegistryStoreTrustletCon(const mcUuid_t *uuid, const mcSpid_t spid, void *so, uint32_t size);

    /** Reads a trustlet container secure object from the registry.
     * @param uuid Trustlet UUID.
     * @param spid SPID of the trustlet container
     * @param[out] so Trustlet container secure object.
     * @param[out] size Trustlet container secure object size
     * @return MC_DRV_OK if successful, otherwise error code.
     */
    mcResult_t mcRegistryReadTrustletCon(const mcUuid_t *uuid, const mcSpid_t spid, void *so, uint32_t *size);

    /** Deletes a trustlet container secure object and all of its associated data.
     * @param uuid Trustlet UUID.
     * @param spid Service provider ID
     * @return MC_DRV_OK if successful, otherwise error code.
     */
    mcResult_t mcRegistryCleanupTrustlet(const mcUuid_t *uuid, const mcSpid_t spid);

    /** Deletes the root container and all of its associated service provider
     * containers.
     * @return MC_DRV_OK if successful, otherwise error code.
     */
    mcResult_t mcRegistryCleanupRoot(void);

#ifdef __cplusplus
}
#endif

#endif // MOBICORE_REGISTRY_H_

/** @} */
