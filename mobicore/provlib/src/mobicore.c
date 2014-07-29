#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <gdmcprovlib.h>
#include <gdmcprovprotocol.h>
#include <gdmcinstance.h>

typedef struct tagMCCM        MCCM;

struct tagMCCM
{
  cmp_t                *cmp;          ///< World Shared Memory (WSM) to the TCI buffer
  mcSessionHandle_t     sess;         ///< session handle
  mcResult_t            lasterror;    ///< last MC driver error
  cmpReturnCode_t       lastcmperr;   ///< last Content Management Protocol error
  uint32_t              lastmccmerr;  ///< error code from MCCM (MobiCore Content Management) library
};

static MCCM g_mccm;

#ifdef ARM

extern void GDMCLog ( int prio, const char *tag, const char *fmt, ... );

#ifdef _DEBUG
extern void GDMCHexDump ( const unsigned char *data, int size );
#else
#define GDMCHexDump(...) do { } while(0)
#endif

#define LOG_TAG		"GDMCProvLib"

#ifdef _DEBUG
#define LOG_d(...)		do { GDMCLog(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__); } while(0)
#else
#define LOG_d(...)		do { } while(0)
#endif
#define LOG_i(...)		do { GDMCLog(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__); } while(0)
#define LOG_w(...)		do { GDMCLog(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__); } while(0)
#define LOG_e(...)		do { GDMCLog(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__); } while(0)

#else

#define LOG_d(...)		do { } while(0)
#define LOG_i(...)		do { } while(0)
#define LOG_w(...)		do { } while(0)
#define LOG_e(...)		do { } while(0)

#endif // ARM

static void dumpErrorInformation ( const char *function, mcResult_t result )
{
  int32_t lastErr = -1;

  LOG_e("%s returned error %u (0x%08X)",function,result,result);

  if (MC_DRV_OK==mcGetSessionErrorCode(&g_mccm.sess,&lastErr))
  {
    LOG_e("mcGetSessionErrorCode for %s returned %i (0x%08X)",function,lastErr,lastErr);
  }
  else
  {
    LOG_i("No additional error code for %s from mcGetSessionErrorCode available.",function);
  }
}

// Copied from MCCM library not to have this additional dependency!

// returns 1 if successful, 0 otherwise
bool mccmOpen ( void )
{
  const mcUuid_t      UUID = TL_CM_UUID;
  mcResult_t          result;

  LOG_d("++++ ENTERED mccmOpen.");

  memset(&g_mccm,0,sizeof(MCCM));

  result = mcOpenDevice(MC_DEVICE_ID_DEFAULT);

  if (MC_DRV_OK != result)
  {
	  LOG_e("mcOpenDevice returned error %u",result);
    LOG_d("++++ LEFT mccmOpen.");
    return false;
  }

  result = mcMallocWsm(MC_DEVICE_ID_DEFAULT, 0, sizeof(cmp_t), (uint8_t **)&g_mccm.cmp, 0);
  if (MC_DRV_OK != result)
  {
    LOG_e("mcMallocWsm returned error %u",result);
    mcCloseDevice(MC_DEVICE_ID_DEFAULT);
    LOG_d("++++ LEFT mccmOpen.");
    return false;
  }

  result = mcOpenSession(&g_mccm.sess,(const mcUuid_t *)&UUID,(uint8_t *)g_mccm.cmp,(uint32_t)sizeof(cmp_t));
  if (MC_DRV_OK != result)
  {
    LOG_e("mcOpenSession returned error %u",result);
    mcFreeWsm(MC_DEVICE_ID_DEFAULT,(uint8_t*)g_mccm.cmp);
    mcCloseDevice(MC_DEVICE_ID_DEFAULT);
    LOG_d("++++ LEFT mccmOpen.");
    return false;
  }

  LOG_d("++++ LEFT mccmOpen.");
  return true;
}

void mccmClose ( void )
{
  LOG_d("++++ ENTERED mccmClose.");

  mcCloseSession(&g_mccm.sess);

  if (NULL!=g_mccm.cmp)
    mcFreeWsm(MC_DEVICE_ID_DEFAULT,(uint8_t*)g_mccm.cmp);

  mcCloseDevice(MC_DEVICE_ID_DEFAULT);

  memset(&g_mccm,0,sizeof(MCCM));

  LOG_d("++++ LEFT mccmClose.");
}

static bool mccmTransmit ( int32_t timeout )
{
  LOG_d("++++ ENTERED mccmTransmit.");

  // Send CMP message to content management trustlet.

  g_mccm.lasterror = mcNotify(&g_mccm.sess);

  if (unlikely( MC_DRV_OK!=g_mccm.lasterror ))
  {
    dumpErrorInformation("mcNotify",g_mccm.lasterror);
    LOG_d("++++ LEFT mccmTransmit.");
    return false;
  }

  // Wait for trustlet response.

  g_mccm.lasterror = mcWaitNotification(&g_mccm.sess, timeout);

  if (unlikely( MC_DRV_OK!=g_mccm.lasterror ))
  {
    dumpErrorInformation("mcWaitNotification",g_mccm.lasterror);
    LOG_d("++++ LEFT mccmTransmit.");
    return false;
  }

  LOG_d("++++ LEFT mccmTransmit.");

  return true;
}

static bool mccmGetSuid ( mcSuid_t *suid )
{
  LOG_d("++++ ENTERED mccmGetSuid.");

  g_mccm.lastcmperr = SUCCESSFUL;

  memset(g_mccm.cmp,0,sizeof(cmp_t));
  g_mccm.cmp->msg.cmpCmdGetSuid.cmdHeader.commandId = MC_CMP_CMD_GET_SUID;

  if (unlikely( !mccmTransmit(MC_INFINITE_TIMEOUT) ))
  {
    LOG_d("++++ LEFT mccmGetSuid.");
    return false;
  }

  if (unlikely( (MC_CMP_CMD_GET_SUID|RSP_ID_MASK)!=g_mccm.cmp->msg.cmpRspGetSuid.rspHeader.responseId ))
  {
    LOG_e("Bad response ID of GET_SUID response.");
    g_mccm.lasterror = MC_DRV_ERR_UNKNOWN;
    LOG_d("++++ LEFT mccmGetSuid.");
    return false;
  }

  g_mccm.lastcmperr = g_mccm.cmp->msg.cmpRspGetSuid.rspHeader.returnCode;

  if (unlikely( SUCCESSFUL!=g_mccm.lastcmperr ))
  {
    LOG_e("CMP error occurred, code: %u (0x%08X).",g_mccm.lastcmperr,g_mccm.lastcmperr);
    g_mccm.lasterror = MC_DRV_ERR_UNKNOWN;
    LOG_d("++++ LEFT mccmGetSuid.");
    return false;
  }

  memcpy(suid,&g_mccm.cmp->msg.cmpRspGetSuid.suid,sizeof(mcSuid_t));

#ifdef _DEBUG
  LOG_d("SUID returned is:");
  GDMCHexDump((const unsigned char*)suid,sizeof(*suid));
#endif

  LOG_d("++++ LEFT mccmGetSuid.");
  return true;
}

static bool mccmGenerateAuthToken (
                      const cmpCmdGenAuthToken_t *cmd,
                      cmpRspGenAuthToken_t       *rsp )
{
  LOG_d("++++ ENTERED mccmGenerateAuthToken.");

#ifdef _DEBUG
  LOG_d("CMP request is (hexdump):");
  GDMCHexDump((const unsigned char*)cmd,sizeof(*cmd));
#endif

  g_mccm.lastcmperr = SUCCESSFUL;

  memset(g_mccm.cmp,0,sizeof(cmp_t));

  memcpy(g_mccm.cmp,cmd,sizeof(*cmd));

  if (unlikely( !mccmTransmit(MC_INFINITE_TIMEOUT) ))
  {
    LOG_d("++++ LEFT mccmGenerateAuthToken.");
    return false;
  }

  if (unlikely( (cmd->cmd.sdata.cmdHeader.commandId|RSP_ID_MASK)!=g_mccm.cmp->msg.cmpRspGenAuthToken.rsp.rspHeader.responseId ))
  {
    LOG_e("Bad response ID of GENERATE_AUTH_TOKEN response.");
    g_mccm.lasterror = MC_DRV_ERR_UNKNOWN;
    LOG_d("++++ LEFT mccmGenerateAuthToken.");
    return false;
  }

  g_mccm.lastcmperr = g_mccm.cmp->msg.cmpRspGenAuthToken.rsp.rspHeader.returnCode;

  if (unlikely( SUCCESSFUL!=g_mccm.lastcmperr ))
  {
    LOG_e("CMP error occurred, code: %u (0x%08X).",g_mccm.lastcmperr,g_mccm.lastcmperr);
    g_mccm.lasterror = MC_DRV_ERR_UNKNOWN;
    LOG_d("++++ LEFT mccmGenerateAuthToken.");
    return false;
  }

  memcpy(rsp,g_mccm.cmp,sizeof(*rsp));

#ifdef _DEBUG
  LOG_d("CMP response is (hexdump):");
  GDMCHexDump((const unsigned char*)rsp,sizeof(*rsp));
#endif

  LOG_d("++++ LEFT mccmGenerateAuthToken.");
  return true;
}

///////////////////////////////////////////////////////////////////////////////////////////
// Convenience functions
///////////////////////////////////////////////////////////////////////////////////////////

gderror MCGetSUID ( _u8 *suid )
{
  if (unlikely( NULL==suid ))
    return GDERROR_PARAMETER;

  memset(suid,0,SUID_LENGTH);

  if (!mccmGetSuid((mcSuid_t*)suid))
    return GDERROR_CANT_GET_SUID;

  return GDERROR_OK;
}

gderror MCGenerateAuthToken ( gdmcinst *inst, const gdmc_actmsg_req *req, gdmc_so_authtok *authtok )
{
  cmpRspGenAuthToken_t    rsp;

  if (unlikely( NULL==inst || NULL==req || NULL==authtok ))
    return GDERROR_PARAMETER;

  memset(authtok,0,sizeof(gdmc_so_authtok));

  if (MC_CMP_CMD_GENERATE_AUTH_TOKEN!=req->msg_type)
    return GDERROR_MESSAGE_FORMAT;

  if (!mccmGenerateAuthToken((const cmpCmdGenAuthToken_t *)req,&rsp))
    return GDERROR_CANT_BUILD_AUTHTOKEN;

  memcpy(authtok,&rsp.soAuthCont,sizeof(*authtok));

  return GDERROR_OK;
}
