#ifndef _INC_GDMCPROVPROTOCOL_H_
#define _INC_GDMCPROVPROTOCOL_H_

#include <gdmcprovlib.h>
#include <mobicore.h>

#ifdef WIN32

#pragma warning ( disable : 4200 )

#pragma pack(push,1)

#ifndef PACK_ATTR
#define PACK_ATTR
#endif // PACK_ATTR

#else

#ifndef PACK_ATTR
#define PACK_ATTR   __attribute__((packed))
#endif // PACK_ATTR

#define IsBadReadPtr(p,c)       (NULL==p)
#define IsBadWritePtr(p,c)      (NULL==p)

#endif

#define AUTHENTICATION_TOKEN            MC_SO_TYPE_REGULAR    
#define CONTEXT_SYSTEM                  MC_SO_CONTEXT_TLT     
#define SUID_LENGTH                     MC_SUID_LEN                 // 16
#define K_SOC_AUTH_LENGTH               MC_CONT_SYMMETRIC_KEY_SIZE  // 32
#define SHA256_HASH_LENGTH              MC_SO_HASH_SIZE             // 32

#undef AES_BLOCK_SIZE

#define AES_BLOCK_SIZE                  MC_SO_ENCRYPT_BLOCK_SIZE    // 16

typedef struct _gdmc_actmsg_req         gdmc_actmsg_req;
typedef struct _gdmc_actmsg_resp        gdmc_actmsg_resp;
typedef struct _gdmc_so_authtok         gdmc_so_authtok;
typedef struct _gdmc_error_msg          gdmc_error_msg;

#define MC_GETSUID_REQ            ((_u32)0x0100434D)
#define MC_GETSUID_RESP           ((_u32)0x0200434D)
#define MC_GENAUTHTOKEN_REQ       ((_u32)0x0300434D)
#define MC_GENAUTHTOKEN_RESP      ((_u32)0x0400434D)
#define MC_VALIDATEAUTHTOKEN_REQ  ((_u32)0x0500434D)
#define MC_ERROR                  ((_u32)0x0600434D)

#ifndef CMP_GEN_AUTH_TOKEN_PSS_SIZE
#define CMP_GEN_AUTH_TOKEN_PSS_SIZE 256
#endif

/// G&D MobiCore error message
struct _gdmc_error_msg
{
  _u32              errorcode;              ///< error code; you can safely cast this to gderror.
  _u32              errmsg_length;          ///< length of error message, may be 0
  _u8               errmsg[];               ///< error message (variable)
} PACK_ATTR;

/// G&D MobiCore SO.AuthToken (authentication token)
struct _gdmc_so_authtok
{
  // Header

  _u32              type;
  _u32              version;
  _u32              context;
  _u32              lifetime;             // NEW2 -> ignore
  _u32              producer_spid;        // NEW2 -> ignore
  _u8               producer_uuid[16];    // NEW2 -> ignore
  _u32              plain_length;         // OLD: 16 (SUID_LENGTH), NEW: 24 (two additional ints)
                                          // NEW2: 28 (version is new)
  _u32              encrypted_length;     // here: K_SOC_AUTH_LENGTH

  // Plaintext Data

  _u32              contType;             // NEW: contType_t         = CONT_TYPE_SOC
  _u32              contVersion;          // NEW2: version
  _u32              contState;            // NEW: mcContainerState_t = MC_CONT_STATE_UNREGISTERED
  _u8               suid[SUID_LENGTH];
  
  // Encrypted Data (encrypted with K.Device.Ctxt)

  _u8               kSoCAuth[K_SOC_AUTH_LENGTH];
  _u8               md[SHA256_HASH_LENGTH];
  _u8               padding[AES_BLOCK_SIZE];

} PACK_ATTR;

/// MobiCore activation message (request)
struct _gdmc_actmsg_req
{
  _u32        msg_type;                     ///< type of message = MC_CMP_CMD_GENERATE_AUTH_TOKEN
  _u8         suid[SUID_LENGTH];            ///< SoC SUID
  _u8         kSoCAuth[K_SOC_AUTH_LENGTH];  ///< K.SoC.Auth (AES-256bit key)
  _u32        kid;                          ///< NEW: key id (currently: 1)
  //_u8         md[SHA256_HASH_LENGTH];       ///< SHA-256 hash
  _u8         dsig[CMP_GEN_AUTH_TOKEN_PSS_SIZE];  ///< new: hash substituted by PSS-SIG
} PACK_ATTR;

/// MobiCore activation response
struct _gdmc_actmsg_resp
{
  _u32              msg_type;               ///< type of message = MC_CMP_CMD_GENERATE_AUTH_TOKEN | 0x80000000
  _u32              retcode;                ///< NEW: return code (status of operation)
  gdmc_so_authtok   authtok;                ///< SO.AuthToken (124 bytes)
} PACK_ATTR;

#ifdef WIN32
#pragma pack(pop)
#endif

#ifdef __cplusplus
extern "C" {
#endif

_u32 GDPROVAPI CalcCRC32 ( const _u8 *data, _u32 length );

void GDPROVAPI InitCRCTable ( void );

#ifdef __cplusplus
}
#endif

#endif // _INC_GDMCPROVPROTOCOL_H_
