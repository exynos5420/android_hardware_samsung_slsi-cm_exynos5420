/** Mobicore Driver Registry.
 *
 * Implements the MobiCore driver registry which maintains trustlets.
 *
 * @file
 * @ingroup MCD_MCDIMPL_DAEMON_REG
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

#include <stdlib.h>
#include <dirent.h>
#include <stdio.h>
#include <sys/stat.h>
#include <assert.h>
#include <string.h>
#include <string>
#include <cstring>
#include <cstddef>
#include <sys/mman.h>
#include <errno.h>
#include <fcntl.h>

#include "mcLoadFormat.h"
#include "mcSpid.h"
#include "mcVersionHelper.h"

#include "PrivateRegistry.h"
#include "MobiCoreRegistry.h"

#include "log.h"

/** Maximum size of a trustlet in bytes. */
#define MAX_TL_SIZE       (1 * 1024 * 1024)
/** Maximum size of a shared object container in bytes. */
#define MAX_SO_CONT_SIZE  (512)

// Asserts expression at compile-time (to be used within a function body).
#define ASSERT_STATIC(e) do { enum { assert_static__ = 1 / (e) }; } while (0)

using namespace std;

static const string MC_REGISTRY_CONTAINER_PATH = "/data/app/mcRegistry";
static const string MC_REGISTRY_DEFAULT_PATH = "/system/app/mcRegistry";
static const string MC_REGISTRY_FALLBACK_PATH = "/data/app/mcRegistry";
static const string AUTH_TOKEN_FILE_NAME = "00000000.authtokcont";
static const string ROOT_FILE_NAME = "00000000.rootcont";
static const string SP_CONT_FILE_EXT = ".spcont";
static const string TL_CONT_FILE_EXT = ".tlcont";
static const string TL_BIN_FILE_EXT = ".tlbin";
static const string DATA_CONT_FILE_EXT = ".datacont";

static const string ENV_MC_AUTH_TOKEN_PATH = "MC_AUTH_TOKEN_PATH";

//------------------------------------------------------------------------------
static string byteArrayToString(const void *bytes, size_t elems)
{
    char hx[elems * 2 + 1];

    for (size_t i = 0; i < elems; i++) {
        sprintf(&hx[i * 2], "%02x", ((uint8_t *)bytes)[i]);
    }
    return string(hx);
}
//------------------------------------------------------------------------------
static string uint32ToString(uint32_t value)
{
    char hx[4 * 2 + 1];
    sprintf(hx, "%08X", value);
    string str(hx);
    return string(str.rbegin(), str.rend());
}

//------------------------------------------------------------------------------
static bool doesDirExist(const char *path)
{
    struct stat ss;
    if (path != NULL && stat(path, &ss) == 0 && S_ISDIR(ss.st_mode)) {
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
static string getRegistryPath()
{
    string registryPath;

    // use the default registry path.
    registryPath = MC_REGISTRY_CONTAINER_PATH;
    LOG_I(" Using default registry path %s", registryPath.c_str());

    assert(registryPath.length() != 0);

    return registryPath;
}


//------------------------------------------------------------------------------
static string getTlRegistryPath()
{
    string registryPath;

    // First, attempt to use regular registry environment variable.
    if (doesDirExist(MC_REGISTRY_DEFAULT_PATH.c_str())) {
        registryPath = MC_REGISTRY_DEFAULT_PATH;
        LOG_I("getTlRegistryPath(): Using MC_REGISTRY_PATH %s", registryPath.c_str());
    } else if (doesDirExist(MC_REGISTRY_FALLBACK_PATH.c_str())) {
        // Second, attempt to use fallback registry environment variable.
        registryPath = MC_REGISTRY_FALLBACK_PATH;
        LOG_I("getTlRegistryPath(): Using MC_REGISTRY_FALLBACK_PATH %s", registryPath.c_str());
    }

    // As a last resort, use the default registry path.
    if (registryPath.length() == 0) {
        registryPath = MC_REGISTRY_CONTAINER_PATH;
        LOG_I(" Using default registry path %s", registryPath.c_str());
    }

    assert(registryPath.length() != 0);

    return registryPath;
}

//------------------------------------------------------------------------------
static string getAuthTokenFilePath()
{
    const char *path;
    string authTokenPath;

    // First, attempt to use regular auth token path environment variable.
    path = getenv(ENV_MC_AUTH_TOKEN_PATH.c_str());
    if (doesDirExist(path)) {
        LOG_I("getAuthTokenFilePath(): Using MC_AUTH_TOKEN_PATH %s", path);
        authTokenPath = path;
    } else {
        authTokenPath = getRegistryPath();
        LOG_I("getAuthTokenFilePath(): Using path %s", authTokenPath.c_str());
    }

    return authTokenPath + "/" + AUTH_TOKEN_FILE_NAME;
}

//------------------------------------------------------------------------------
static string getRootContFilePath()
{
    return getRegistryPath() + "/" + ROOT_FILE_NAME;
}

//------------------------------------------------------------------------------
static string getSpDataPath(mcSpid_t spid)
{
    return getRegistryPath() + "/" + uint32ToString(spid);
}

//------------------------------------------------------------------------------
static string getSpContFilePath(mcSpid_t spid)
{
    return getRegistryPath() + "/" + uint32ToString(spid) + SP_CONT_FILE_EXT;
}

//------------------------------------------------------------------------------
static string getTlContFilePath(const mcUuid_t *uuid, const mcSpid_t spid)
{
    return getRegistryPath() + "/" + byteArrayToString(uuid, sizeof(*uuid))
                + "." + uint32ToString(spid) + TL_CONT_FILE_EXT;
}

//------------------------------------------------------------------------------
static string getTlDataPath(const mcUuid_t *uuid)
{
    return getRegistryPath() + "/" + byteArrayToString(uuid, sizeof(*uuid));
}

//------------------------------------------------------------------------------
static string getTlDataFilePath(const mcUuid_t *uuid, mcPid_t pid)
{
    return getTlDataPath(uuid) + "/" + uint32ToString(pid.data) + DATA_CONT_FILE_EXT;
}

//------------------------------------------------------------------------------
static string getTlBinFilePath(const mcUuid_t *uuid)
{
    return getTlRegistryPath() + "/" + byteArrayToString(uuid, sizeof(*uuid)) + TL_BIN_FILE_EXT;
}

//------------------------------------------------------------------------------
mcResult_t mcRegistryStoreAuthToken(void *so, uint32_t size)
{
    if (so == NULL || size > 3 * MAX_SO_CONT_SIZE) {
        LOG_E("mcRegistry store So.Soc failed: %d", MC_DRV_ERR_INVALID_PARAMETER);
        return MC_DRV_ERR_INVALID_PARAMETER;
    }
    const string &authTokenFilePath = getAuthTokenFilePath();
    LOG_I("store AuthToken: %s", authTokenFilePath.c_str());

    FILE *fs = fopen(authTokenFilePath.c_str(), "wb");
    if (!fs) {
        LOG_E("mcRegistry store So.Soc failed: %d", MC_DRV_ERR_INVALID_DEVICE_FILE);
        return MC_DRV_ERR_INVALID_DEVICE_FILE;
    }
    fseek(fs, 0, SEEK_SET);
    fwrite((char *)so, 1, size, fs);
    fflush(fs);
    fclose(fs);

    return MC_DRV_OK;
}


//------------------------------------------------------------------------------
mcResult_t mcRegistryReadAuthToken(mcSoAuthTokenCont_t *so)
{
    if (NULL == so) {
        LOG_E("mcRegistry read So.Soc failed: %d", MC_DRV_ERR_INVALID_PARAMETER);
        return MC_DRV_ERR_INVALID_PARAMETER;
    }
    const string &authTokenFilePath = getAuthTokenFilePath();
    LOG_I("read AuthToken: %s", authTokenFilePath.c_str());

    FILE *fs = fopen(authTokenFilePath.c_str(), "rb");
    if (!fs) {
        LOG_E("mcRegistry read So.Soc failed: %d", MC_DRV_ERR_INVALID_DEVICE_FILE);
        return MC_DRV_ERR_INVALID_DEVICE_FILE;
    }
    fseek(fs, 0, SEEK_END);
    int32_t filesize = ftell(fs);
    if (sizeof(mcSoAuthTokenCont_t) != filesize) {
        fclose(fs);
        LOG_E("mcRegistry read So.Soc failed: %d", MC_DRV_ERR_OUT_OF_RESOURCES);
        return MC_DRV_ERR_OUT_OF_RESOURCES;
    }
    fseek(fs, 0, SEEK_SET);
    fread((char *)so, 1, sizeof(mcSoAuthTokenCont_t), fs);
    fclose(fs);

    return MC_DRV_OK;
}

//------------------------------------------------------------------------------
mcResult_t mcRegistryDeleteAuthToken(void)
{
    if(remove(getAuthTokenFilePath().c_str())) {
        LOG_ERRNO("Delete Auth token file!");
        return MC_DRV_ERR_UNKNOWN;
    }
    else
        return MC_DRV_OK;
}


//------------------------------------------------------------------------------
mcResult_t mcRegistryStoreRoot(void *so, uint32_t size)
{
    if (so == NULL || size > 3 * MAX_SO_CONT_SIZE) {
        LOG_E("mcRegistry store So.Root failed: %d", MC_DRV_ERR_INVALID_PARAMETER);
        return MC_DRV_ERR_INVALID_PARAMETER;
    }

    const string &rootContFilePath = getRootContFilePath();
    LOG_I("store Root: %s", rootContFilePath.c_str());

    FILE *fs = fopen(rootContFilePath.c_str(), "wb");
    if (!fs) {
        LOG_E("mcRegistry store So.Root failed: %d", MC_DRV_ERR_INVALID_DEVICE_FILE);
        return MC_DRV_ERR_INVALID_DEVICE_FILE;
    }
    fseek(fs, 0, SEEK_SET);
    fwrite((char *)so, 1, size, fs);
    fflush(fs);
    fclose(fs);

    return MC_DRV_OK;
}


//------------------------------------------------------------------------------
mcResult_t mcRegistryReadRoot(void *so, uint32_t *size)
{
    const string &rootContFilePath = getRootContFilePath();
    size_t readBytes;

    if (so == NULL) {
        LOG_E("mcRegistry read So.Root failed: %d", MC_DRV_ERR_INVALID_PARAMETER);
        return MC_DRV_ERR_INVALID_PARAMETER;
    }
    LOG_I("read Root: %s", rootContFilePath.c_str());

    FILE *fs = fopen(rootContFilePath.c_str(), "rb");
    if (!fs) {
        LOG_E("mcRegistry read So.Root failed: %d", MC_DRV_ERR_INVALID_DEVICE_FILE);
        return MC_DRV_ERR_INVALID_DEVICE_FILE;
    }
    readBytes = fread((char *)so, 1, *size, fs);
    fclose(fs);

    if (readBytes > 0) {
        *size = readBytes;
        return MC_DRV_OK;
    } else {
        LOG_E("mcRegistry read So.Root failed: %d", MC_DRV_ERR_INVALID_DEVICE_FILE);
        return MC_DRV_ERR_INVALID_DEVICE_FILE;
    }
}


//------------------------------------------------------------------------------
mcResult_t mcRegistryStoreSp(mcSpid_t spid, void *so, uint32_t size)
{
    if ((spid == 0) || (so == NULL) || size > 3 * MAX_SO_CONT_SIZE) {
        LOG_E("mcRegistry store So.Sp(SpId) failed: %d", MC_DRV_ERR_INVALID_PARAMETER);
        return MC_DRV_ERR_INVALID_PARAMETER;
    }

    const string &spContFilePath = getSpContFilePath(spid);
    LOG_I("store SP: %s", spContFilePath.c_str());

    FILE *fs = fopen(spContFilePath.c_str(), "wb");
    if (!fs) {
        LOG_E("mcRegistry store So.Sp(SpId) failed: %d", MC_DRV_ERR_INVALID_DEVICE_FILE);
        return MC_DRV_ERR_INVALID_DEVICE_FILE;
    }
    fseek(fs, 0, SEEK_SET);
    fwrite((char *)so, 1, size, fs);
    fflush(fs);
    fclose(fs);

    return MC_DRV_OK;
}


//------------------------------------------------------------------------------
mcResult_t mcRegistryReadSp(mcSpid_t spid, void *so, uint32_t *size)
{
    const string &spContFilePath = getSpContFilePath(spid);
    size_t readBytes;
    if ((spid == 0) || (so == NULL)) {
        LOG_E("mcRegistry read So.Sp(SpId) failed: %d", MC_DRV_ERR_INVALID_PARAMETER);
        return MC_DRV_ERR_INVALID_PARAMETER;
    }
    LOG_I("read SP: %s", spContFilePath.c_str());

    FILE *fs = fopen(spContFilePath.c_str(), "rb");
    if (!fs) {
        LOG_E("mcRegistry read So.Sp(SpId) failed: %d", MC_DRV_ERR_INVALID_DEVICE_FILE);
        return MC_DRV_ERR_INVALID_DEVICE_FILE;
    }
    readBytes = fread((char *)so, 1, *size, fs);
    fclose(fs);

    if (readBytes > 0) {
        *size = readBytes;
        return MC_DRV_OK;
    } else {
        LOG_E("mcRegistry read So.Sp(SpId) failed: %d", MC_DRV_ERR_INVALID_DEVICE_FILE);
        return MC_DRV_ERR_INVALID_DEVICE_FILE;
    }
}


//------------------------------------------------------------------------------
mcResult_t mcRegistryStoreTrustletCon(const mcUuid_t *uuid, const mcSpid_t spid, void *so, uint32_t size)
{
    if ((uuid == NULL) || (so == NULL) || size > 3 * MAX_SO_CONT_SIZE) {
        LOG_E("mcRegistry store So.TrustletCont(uuid) failed: %d", MC_DRV_ERR_INVALID_PARAMETER);
        return MC_DRV_ERR_INVALID_PARAMETER;
    }

    const string &tlContFilePath = getTlContFilePath(uuid, spid);
    LOG_I("store TLc: %s", tlContFilePath.c_str());

    FILE *fs = fopen(tlContFilePath.c_str(), "wb");
    if (!fs) {
        LOG_E("mcRegistry store So.TrustletCont(uuid) failed: %d", MC_DRV_ERR_INVALID_DEVICE_FILE);
        return MC_DRV_ERR_INVALID_DEVICE_FILE;
    }
    fseek(fs, 0, SEEK_SET);
    fwrite((char *)so, 1, size, fs);
    fflush(fs);
    fclose(fs);

    return MC_DRV_OK;
}


//------------------------------------------------------------------------------
mcResult_t mcRegistryReadTrustletCon(const mcUuid_t *uuid, const mcSpid_t spid, void *so, uint32_t *size)
{
    if ((uuid == NULL) || (so == NULL)) {
        LOG_E("mcRegistry read So.TrustletCont(uuid) failed: %d", MC_DRV_ERR_INVALID_PARAMETER);
        return MC_DRV_ERR_INVALID_PARAMETER;
    }
    size_t readBytes;
    const string &tlContFilePath = getTlContFilePath(uuid, spid);
    LOG_I("read TLc: %s", tlContFilePath.c_str());

    FILE *fs = fopen(tlContFilePath.c_str(), "rb");
    if (!fs) {
        LOG_E("mcRegistry read So.TrustletCont(uuid) failed: %d", MC_DRV_ERR_INVALID_DEVICE_FILE);
        return MC_DRV_ERR_INVALID_DEVICE_FILE;
    }
    fseek(fs, 0, SEEK_SET);
    readBytes = fread((char *)so, 1, *size, fs);
    fclose(fs);

    if(readBytes > 0) {
        *size = readBytes;
        return MC_DRV_OK;
    } else {
        LOG_E("mcRegistry read So.TrustletCont(uuid) failed: %d", MC_DRV_ERR_INVALID_DEVICE_FILE);
        return MC_DRV_ERR_INVALID_DEVICE_FILE;
    }
}


//------------------------------------------------------------------------------
mcResult_t mcRegistryStoreData(void *so, uint32_t size)
{
    mcSoDataCont_t *dataCont = (mcSoDataCont_t *)so;

    if (dataCont == NULL || size != sizeof(mcSoDataCont_t)) {
        LOG_E("mcRegistry store So.Data failed: %d", MC_DRV_ERR_INVALID_PARAMETER);
        return MC_DRV_ERR_INVALID_PARAMETER;
    }
    string pathname, filename;

    switch (dataCont->cont.type) {
    case CONT_TYPE_SPDATA:
        LOG_E("SPDATA not supported");
        return MC_DRV_ERR_INVALID_PARAMETER;
        break;
    case CONT_TYPE_TLDATA:
        pathname = getTlDataPath(&dataCont->cont.uuid);
        filename = getTlDataFilePath(&dataCont->cont.uuid, dataCont->cont.pid);
        break;
    default:
        LOG_E("mcRegistry store So.Data(cid/pid) failed: %d", MC_DRV_ERR_INVALID_PARAMETER);
        return MC_DRV_ERR_INVALID_PARAMETER;
    }
    mkdir(pathname.c_str(), 0777);

    LOG_I("store DT: %s", filename.c_str());

    FILE *fs = fopen(filename.c_str(), "wb");
    if (!fs) {
        LOG_E("mcRegistry store So.Data(cid/pid) failed: %d", MC_DRV_ERR_INVALID_DEVICE_FILE);
        return MC_DRV_ERR_INVALID_DEVICE_FILE;
    }
    fseek(fs, 0, SEEK_SET);
    fwrite((char *)dataCont, 1, MC_SO_SIZE(dataCont->soHeader.plainLen, dataCont->soHeader.encryptedLen), fs);
    fflush(fs);
    fclose(fs);

    return MC_DRV_OK;
}


//------------------------------------------------------------------------------
mcResult_t mcRegistryReadData(uint32_t context, const mcCid_t *cid, mcPid_t pid,
    mcSoDataCont_t *so, uint32_t maxLen)
{
    if ((NULL == cid) || (NULL == so)) {
        LOG_E("mcRegistry read So.Data failed: %d", MC_DRV_ERR_INVALID_PARAMETER);
        return MC_DRV_ERR_INVALID_PARAMETER;
    }
    string filename;
    switch (context) {
    case 0:
        LOG_E("SPDATA not supported");
        return MC_DRV_ERR_INVALID_PARAMETER;
        break;
    case 1:
        filename = getTlDataFilePath(&so->cont.uuid, so->cont.pid);
        break;
    default:
        LOG_E("mcRegistry read So.Data(cid/pid) failed: %d", MC_DRV_ERR_INVALID_PARAMETER);
        return MC_DRV_ERR_INVALID_PARAMETER;
    }
    LOG_I("read DT: %s", filename.c_str());

    FILE *fs = fopen(filename.c_str(), "rb");
    if (!fs) {
        LOG_E("mcRegistry read So.Data(cid/pid) failed: %d", MC_DRV_ERR_INVALID_DEVICE_FILE);
        return MC_DRV_ERR_INVALID_DEVICE_FILE;
    }
    fseek(fs, 0, SEEK_END);
    uint32_t filesize = ftell(fs);
    if (maxLen < filesize) {
        fclose(fs);
        LOG_E("mcRegistry read So.Data(cid/pid) failed: %d", MC_DRV_ERR_OUT_OF_RESOURCES);
        return MC_DRV_ERR_OUT_OF_RESOURCES;
    }
    fseek(fs, 0, SEEK_SET);
    char *p = (char *) so;
    fread(p, 1, sizeof(mcSoHeader_t), fs);
    p += sizeof(mcSoHeader_t);
    fread(p, 1, MC_SO_SIZE(so->soHeader.plainLen, so->soHeader.encryptedLen) - sizeof(mcSoHeader_t), fs);
    fclose(fs);

    return MC_DRV_OK;
}


//------------------------------------------------------------------------------
mcResult_t mcRegistryCleanupTrustlet(const mcUuid_t *uuid, const mcSpid_t spid)
{
    DIR            *dp;
    struct dirent  *de;
    int             e;

    if (NULL == uuid) {
        LOG_E("mcRegistry cleanupTrustlet(uuid) failed: %d", MC_DRV_ERR_INVALID_PARAMETER);
        return MC_DRV_ERR_INVALID_PARAMETER;
    }
    string pathname = getTlDataPath(uuid);
    if (NULL != (dp = opendir(pathname.c_str()))) {
        while (NULL != (de = readdir(dp))) {
            if (de->d_name[0] != '.') {
                string dname = pathname + "/" + string (de->d_name);
                LOG_I("delete DT: %s", dname.c_str());
                if (0 != (e = remove(dname.c_str()))) {
                    LOG_E("remove UUID-data %s failed! error: %d", dname.c_str(), e);
                }
            }
        }
        closedir(dp);
        LOG_I("delete dir: %s", pathname.c_str());
        if (0 != (e = rmdir(pathname.c_str()))) {
            LOG_E("remove UUID-dir failed! errno: %d", e);
            return MC_DRV_ERR_UNKNOWN;
        }
    }
    string tlBinFilePath = getTlBinFilePath(uuid);
    LOG_I("delete Tlb: %s", tlBinFilePath.c_str());
    if (0 != (e = remove(tlBinFilePath.c_str()))) {
        LOG_E("remove Tlb failed! errno: %d", e);
//        return MC_DRV_ERR_UNKNOWN;     // a trustlet-binary must not be present ! (registered but not usable)
    }
    string tlContFilePath = getTlContFilePath(uuid, spid);
    LOG_I("delete Tlc: %s", tlContFilePath.c_str());
    if (0 != (e = remove(tlContFilePath.c_str()))) {
        LOG_E("remove Tlc failed! errno: %d", e);
        return MC_DRV_ERR_UNKNOWN;
    }
    return MC_DRV_OK;
}


//------------------------------------------------------------------------------
mcResult_t mcRegistryCleanupSp(mcSpid_t spid)
{
    DIR *dp;
    struct dirent  *de;
    mcResult_t ret;
    mcSoSpCont_t data;
    uint32_t i, len;
    int e;

    if (0 == spid) {
        LOG_E("mcRegistry cleanupSP(SpId) failed: %d", MC_DRV_ERR_INVALID_PARAMETER);
        return MC_DRV_ERR_INVALID_PARAMETER;
    }
    len = sizeof(mcSoSpCont_t);
    ret = mcRegistryReadSp(spid, &data, &len);
    if (MC_DRV_OK != ret || len != sizeof(mcSoSpCont_t)) {
        LOG_E("read SP->UUID aborted! Return code: %d", ret);
        return ret;
    }
    for (i = 0; (i < MC_CONT_CHILDREN_COUNT) && (ret == MC_DRV_OK); i++) {
        if (0 != strncmp((const char *) & (data.cont.children[i]), (const char *)&MC_UUID_FREE, sizeof(mcUuid_t))) {
            ret = mcRegistryCleanupTrustlet(&(data.cont.children[i]), spid);
        }
    }
    if (MC_DRV_OK != ret) {
        LOG_E("delete SP->UUID failed! Return code: %d", ret);
        return ret;
    }
    string pathname = getSpDataPath(spid);

    if (NULL != (dp = opendir(pathname.c_str()))) {
        while (NULL != (de = readdir(dp))) {
            if (de->d_name[0] != '.') {
                string dname = pathname + "/" + string (de->d_name);
                LOG_I("delete DT: %s", dname.c_str());
                if (0 != (e = remove(dname.c_str()))) {
                    LOG_E("remove SPID-data %s failed! error: %d", dname.c_str(), e);
                }
            }
        }
        closedir(dp);
        LOG_I("delete dir: %s", pathname.c_str());
        if (0 != (e = rmdir(pathname.c_str()))) {
            LOG_E("remove SPID-dir failed! error: %d", e);
            return MC_DRV_ERR_UNKNOWN;
        }
    }
    string spContFilePath = getSpContFilePath(spid);
    LOG_I("delete Sp: %s", spContFilePath.c_str());
    if (0 != (e = remove(spContFilePath.c_str()))) {
        LOG_E("remove SP failed! error: %d", e);
        return MC_DRV_ERR_UNKNOWN;
    }
    return MC_DRV_OK;
}


//------------------------------------------------------------------------------
mcResult_t mcRegistryCleanupRoot(void)
{
    mcResult_t ret;
    mcSoRootCont_t data;
    uint32_t i, len;
    int e;
    len = sizeof(mcSoRootCont_t);
    ret = mcRegistryReadRoot(&data, &len);
    if (MC_DRV_OK != ret || len != sizeof(mcSoRootCont_t)) {
        LOG_E("read Root aborted! Return code: %d", ret);
        return ret;
    }
    for (i = 0; (i < MC_CONT_CHILDREN_COUNT) && (ret == MC_DRV_OK); i++) {
        mcSpid_t spid = data.cont.children[i];
        if (spid != MC_SPID_FREE) {
            ret = mcRegistryCleanupSp(spid);
            if (MC_DRV_OK != ret) {
                LOG_E("Cleanup SP failed! Return code: %d", ret);
                return ret;
            }
        }
    }

    string rootContFilePath = getRootContFilePath();
    LOG_I("Delete root: %s", rootContFilePath.c_str());
    if (0 != (e = remove(rootContFilePath.c_str()))) {
        LOG_E("Delete root failed! error: %d", e);
        return MC_DRV_ERR_UNKNOWN;
    }
    return MC_DRV_OK;
}

//------------------------------------------------------------------------------
regObject_t *mcRegistryMemGetServiceBlob(mcSpid_t spid, void *trustlet, uint32_t tlSize)
{
    regObject_t *regobj = NULL;

    // Ensure that a UUID is provided.
    if (NULL == trustlet) {
        LOG_E("No trustlet buffer given");
        return NULL;
    }

    // Check service blob size.
    if (tlSize > MAX_TL_SIZE ) {
        LOG_E("mcRegistryGetServiceBlob() failed: service blob too big: %d", tlSize);
        return NULL;
    }

    mclfIntro_t *pIntro = (mclfIntro_t *)trustlet;
    // Check TL magic value.
    if (pIntro->magic != MC_SERVICE_HEADER_MAGIC_BE) {
        LOG_E("mcRegistryGetServiceBlob() failed: wrong header magic value: %d", pIntro->magic);
        return NULL;
    }

    // Get service type.
    mclfHeaderV2_t *pHeader = (mclfHeaderV2_t *)trustlet;
#ifndef NDEBUG
    {
        const char *service_types[] = {
            "illegal", "Driver", "Trustlet", "System Trustlet"
        };
        int serviceType_safe = pHeader->serviceType > SERVICE_TYPE_SYSTEM_TRUSTLET ? SERVICE_TYPE_ILLEGAL : pHeader->serviceType;
        LOG_I(" Service is a %s (service type %d)", service_types[serviceType_safe], pHeader->serviceType);
    }
#endif

    LOG_I("Trustlet text %u data %u ", pHeader->text.len, pHeader->data.len);

    // If loadable driver or system trustlet.
    if (pHeader->serviceType == SERVICE_TYPE_DRIVER  || pHeader->serviceType == SERVICE_TYPE_SYSTEM_TRUSTLET) {
        // Take trustlet blob 'as is'.
        if (NULL == (regobj = (regObject_t *) (malloc(sizeof(regObject_t) + tlSize)))) {
            LOG_E("mcRegistryGetServiceBlob() failed: Out of memory");
            return NULL;
        }
        regobj->len = tlSize;
        regobj->tlStartOffset = 0;
        memcpy((char *)regobj->value, trustlet, tlSize);
        // If user trustlet.
    }
    else if (pHeader->serviceType == SERVICE_TYPE_SP_TRUSTLET) {
        // Take trustlet blob and append root, sp, and tl container.
        size_t regObjValueSize = tlSize + sizeof(mcBlobLenInfo_t) + 3 * MAX_SO_CONT_SIZE;

        // Prepare registry object.
        if (NULL == (regobj = (regObject_t *) malloc(sizeof(regObject_t) + regObjValueSize))) {
            LOG_E("mcRegistryGetServiceBlob() failed: Out of memory");
            return NULL;
        }
        regobj->len = regObjValueSize;
        regobj->tlStartOffset = sizeof(mcBlobLenInfo_t);
        uint8_t *p = regobj->value;

        // Reserve space for the blob length structure
        mcBlobLenInfo_ptr lenInfo = (mcBlobLenInfo_ptr)p;
        lenInfo->magic = MC_TLBLOBLEN_MAGIC;
        p += sizeof(mcBlobLenInfo_t);
        // Fill in trustlet blob after the len info
        memcpy(p, trustlet, tlSize);
        p += tlSize;

        // Final registry object value looks like this:
        //
        //    +---------------+---------------------------+-----------+---------+---------+
        //    | Blob Len Info | TL-Header TL-Code TL-Data | Root Cont | SP Cont | TL Cont |
        //    +---------------+---------------------------+-----------+-------------------+
        //                    /------ Trustlet BLOB ------/
        //
        //    /------------------ regobj->header.len -------------------------------------/

        // start at the end of the trustlet blob
        mcResult_t ret;
        do {
            uint32_t soTltContSize = MAX_SO_CONT_SIZE;
            uint32_t len;

            // Fill in root container.
            len = sizeof(mcSoRootCont_t);
            if (MC_DRV_OK != (ret = mcRegistryReadRoot(p, &len))) {
                break;
            }
            lenInfo->rootContBlobSize = len;
            p += len;

            // Fill in SP container.
            len = sizeof(mcSoSpCont_t);
            if (MC_DRV_OK != (ret = mcRegistryReadSp(spid, p, &len))) {
                break;
            }
            lenInfo->spContBlobSize = len;
            p += len;

            // Fill in TLT Container
            // We know exactly how much space is left in the buffer
            soTltContSize = regObjValueSize - tlSize + sizeof(mcBlobLenInfo_t)
                - lenInfo->spContBlobSize - lenInfo->rootContBlobSize;
            if (MC_DRV_OK != (ret = mcRegistryReadTrustletCon(&pHeader->uuid, spid, p, &soTltContSize))) {
                break;
            }
            lenInfo->tlContBlobSize = soTltContSize;
            LOG_I("Trustlet container %u bytes loaded", soTltContSize);
            // Depending on the trustlet container size we decide which structure to use
            // Unfortunate design but it should have to do for now
            if (soTltContSize == sizeof(mcSoTltCont_2_0_t)) {
                LOG_I("Using 2.0 trustlet container");
            }
            else if (soTltContSize == sizeof(mcSoTltCont_2_1_t)) {
                LOG_I("Using 2.1 trustlet container");
            }
            else {
                LOG_E("Trustlet container has unknown size");
                break;
            }
        } while (false);

        if (MC_DRV_OK != ret) {
            LOG_E("mcRegistryGetServiceBlob() failed: Error code: %d", ret);
            free(regobj);
            return NULL;
        }
        // Now we know the sizes for all containers so set the correct size
        regobj->len = sizeof(mcBlobLenInfo_t) + tlSize +
                        lenInfo->rootContBlobSize +
                        lenInfo->spContBlobSize +
                        lenInfo->tlContBlobSize;
        // Any other service type.
    } else {
        LOG_E("mcRegistryGetServiceBlob() failed: Unsupported service type %u", pHeader->serviceType);
    }
    return regobj;
}


//------------------------------------------------------------------------------
regObject_t *mcRegistryFileGetServiceBlob(const char* trustlet)
{
    struct stat sb;
    regObject_t *regobj = NULL;
    void *buffer;

    // Ensure that a file name is provided.
    if (trustlet == NULL) {
        LOG_E("No file given");
        return NULL;
    }

    int fd = open(trustlet, O_RDONLY);
    if (fd == -1) {
        LOG_E("Cannot open %s", trustlet);
        return NULL;
    }

    if (fstat(fd, &sb) == -1){
        LOG_E("mcRegistryGetServiceBlob() failed: Cound't get file size");
        goto error;
    }

    buffer = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (buffer == MAP_FAILED) {
        LOG_E("mcRegistryGetServiceBlob(): Failed to map file to memory");
        goto error;
    }

    regobj = mcRegistryMemGetServiceBlob(0, buffer, sb.st_size);

    // We don't actually care if either of them fails but should still print warnings
    if (munmap(buffer, sb.st_size)) {
        LOG_E("mcRegistryGetServiceBlob(): Failed to unmap memory");
    }

error:
    if (close(fd)) {
        LOG_E("mcRegistryGetServiceBlob(): Failed to close file %s", trustlet);
    }

    return regobj;
}


//------------------------------------------------------------------------------
regObject_t *mcRegistryGetServiceBlob(const mcUuid_t *uuid)
{
    // Ensure that a UUID is provided.
    if (NULL == uuid) {
        LOG_E("No UUID given");
        return NULL;
    }

    // Open service blob file.
    string tlBinFilePath = getTlBinFilePath(uuid);
    LOG_I(" Loading %s", tlBinFilePath.c_str());

    return mcRegistryFileGetServiceBlob(tlBinFilePath.c_str());
}

//------------------------------------------------------------------------------
regObject_t *mcRegistryGetDriverBlob(const char *filename)
{
    regObject_t *regobj = mcRegistryFileGetServiceBlob(filename);

    if (regobj == NULL) {
        LOG_E("mcRegistryGetDriverBlob() failed");
        return NULL;
    }

    // Get service type.
    mclfHeaderV2_t *pHeader = (mclfHeaderV2_t *)regobj->value;

    // If file is not a driver we are not interested
    if (pHeader->serviceType != SERVICE_TYPE_DRIVER) {
        LOG_E("mcRegistryGetServiceBlob() failed: Unsupported service type %u", pHeader->serviceType);
        pHeader = NULL;
        free(regobj);
        regobj = NULL;
    }

    return regobj;
}

/** @} */
