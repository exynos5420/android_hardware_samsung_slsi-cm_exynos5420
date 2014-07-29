///
/// @file       gdmcdevicebinding.cpp
/// @author     Giesecke & Devrient GmbH, Munich, Germany
///
/// Implementation of the (internal) device binding
///

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <gdmcprovlib.h>
#include <gdmcprovprotocol.h>
#include <gdmcinstance.h>

extern "C"
{
  gderror MCGetSUID ( _u8 *suid );

  gderror MCGenerateAuthToken ( gdmcinst *inst, const gdmc_actmsg_req *req, gdmc_so_authtok *authtok );
}

//////////////////////////////////////////////////////////////////////////////
// MS Windows-specific includes
//////////////////////////////////////////////////////////////////////////////

#if defined(WIN32)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#ifdef _DEBUG
#include <crtdbg.h>
#endif

//////////////////////////////////////////////////////////////////////////////
// Linux-specific includes
//////////////////////////////////////////////////////////////////////////////

#elif defined(LINUX)

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <pthread.h>
#include <syslog.h>
#include <errno.h>
#include <time.h>
#include <sys/socket.h>
#include <arpa/inet.h>

//////////////////////////////////////////////////////////////////////////////
// ARM-specific includes
//////////////////////////////////////////////////////////////////////////////

#else // ARM

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#endif

#include <gdmcprovprotocol.h>
#include <gdmcinstance.h>
#include <gdmcdevicebinding.h>

#define MAX_MSGSIZE       4096

extern authtok_writecb g_authtok_writecb;
extern authtok_readcb  g_authtok_readcb;

#ifdef WIN32
#define vsnprintf _vsnprintf

#pragma warning ( disable : 4996 )

#endif

#ifdef ARM

extern "C" void GDMCLog ( int prio, const char *tag, const char *fmt, ... )
{
  va_list       ap;

  va_start(ap,fmt);
#if defined(WIN32) || defined(LINUX)
  {
    char buffer[1024];
    FILE *f = fopen("libMcClient.log","at");
    if (likely(NULL!=f))
    {
      vsprintf(buffer,fmt,ap);
      fprintf(f,"[%i][%s] %s\n",prio,tag,buffer);
      fclose(f);
    }
  }
#else
  __android_log_vprint(prio,tag,fmt,ap);
#endif
  va_end(ap);
}

#ifdef _DEBUG
extern "C" void GDMCHexDump ( const unsigned char *data, int size )
{
  static char     szHexLine[80], szHex[12];
  unsigned char   x, h, l;
  int             i,j;

  if (!size)
    return;

  while (size>0)
  {
    memset(szHexLine,0x20,sizeof(szHexLine));
    szHexLine[77] = 0x00;
    szHexLine[78] = 0x00;
    if (size>8)
      szHexLine[34] = '-';

    sprintf(szHex,"%08X",(unsigned int)data);
    memcpy(szHexLine,szHex,8);

    i=0;j=0;
    while (size>0)
    {
      x = *(data++);
      size--;
      h = (x>>4)+0x30;
      l = (x&15)+0x30;
      if (h>0x39) h+=7;
      if (l>0x39) l+=7;
      szHexLine[i*3+10+j] = (char)h;
      szHexLine[i*3+11+j] = (char)l;

      if ((x<32) || (x>=127)) x = '.';

      szHexLine[i+61] = (char)x;

      i++;
      if (8==i)
        j = 2;
      if (16==i)
        break;
    }

    LOG_d("%s",szHexLine);
  }
}

#endif // _DEBUG

#endif // ARM

gderror GDMCComposeErrorMessage ( gdmcinst *inst, gderror error, _u8 *msgout, _u32 *msgout_size, _u32 initial_msgout_size, const char *pszmsg, ... )
{
  _u32              msgsize = 0;
  gdmc_msgheader   *header;
  gdmc_error_msg   *body;
  gdmc_msgtrailer  *trailer;
  va_list           ap;
  char             *buffer = NULL;
  _u32              errmsgsize = 0;
  _u32              errmsgsize_aligned;

  if (NULL!=pszmsg)
  {
    buffer = (char*)malloc(MAX_MSGSIZE);

    if (NULL!=buffer)
    {
      memset(buffer,0,MAX_MSGSIZE);
      va_start(ap,pszmsg);
      vsnprintf(buffer,MAX_MSGSIZE,pszmsg,ap);
      va_end(ap);

      errmsgsize = ((_u32)strlen(buffer))+1;
      if (1==errmsgsize)
        errmsgsize--;     // if empty message, then do not send anything
    }
  }

  errmsgsize_aligned = (errmsgsize+3)&(~3);

  // compose MC_ERROR message

  msgsize = sizeof(gdmc_msgheader)+sizeof(gdmc_error_msg)+errmsgsize_aligned+sizeof(gdmc_msgtrailer);

  if (msgsize>initial_msgout_size)
  {
    if (NULL!=buffer)
      free(buffer);
    return GDERROR_BUFFER_TOO_SMALL;
  }

  header  = (gdmc_msgheader*)msgout;
  body    = (gdmc_error_msg*)(msgout+sizeof(gdmc_msgheader));
  trailer = (gdmc_msgtrailer*)(msgout+sizeof(gdmc_msgheader)+sizeof(gdmc_error_msg)+errmsgsize_aligned);

  header->msg_type    = MC_ERROR;
  header->body_size   = sizeof(gdmc_error_msg)+errmsgsize;

  body->errorcode     = error;
  body->errmsg_length = errmsgsize_aligned;

  if ((NULL!=buffer) && (0!=errmsgsize))
  {
    memset(body->errmsg,0,errmsgsize_aligned);
    memcpy(body->errmsg,buffer,errmsgsize);
  }

  if (NULL!=buffer)
    free(buffer);

  trailer->magic = ~MC_ERROR;
  trailer->crc32 = CalcCRC32(msgout,msgsize-sizeof(_u32));

  *msgout_size = msgsize;

  return GDERROR_OK;
}

gderror GDPROVAPI GDMCValidateProvMessage ( const _u8        *msg,
                                            _u32              msgsize,
                                            gdmc_msgheader  **ppheader,
                                            _u8             **ppbody,
                                            gdmc_msgtrailer **pptrailer )
{
  _u32              expected_msgsize, aligned_body_size;
  _u32              crc32;
  gdmc_error_msg   *errmsg;
  gdmc_actmsg_resp *actmsg;

  *ppheader   = NULL;
  *ppbody     = NULL;
  *pptrailer  = NULL;

  if (msgsize<(sizeof(gdmc_msgheader)+sizeof(gdmc_msgtrailer)))
    return GDERROR_MESSAGE_FORMAT;

  if (msgsize&3)
    return GDERROR_MESSAGE_FORMAT;

  if (IsBadReadPtr(msg,sizeof(gdmc_msgheader)+sizeof(gdmc_msgtrailer)))
    return GDERROR_PARAMETER;

  *ppheader = (gdmc_msgheader*)msg;

  aligned_body_size = ((*ppheader)->body_size+3)&(~3);

  expected_msgsize = sizeof(gdmc_msgheader)+sizeof(gdmc_msgtrailer)+aligned_body_size;

  if (msgsize!=expected_msgsize)
    return GDERROR_MESSAGE_FORMAT;

  if (IsBadReadPtr(msg,expected_msgsize))
    return GDERROR_PARAMETER;

  *ppbody    = (_u8*)(msg+sizeof(gdmc_msgheader));
  *pptrailer = (gdmc_msgtrailer*)((*ppbody)+aligned_body_size);

  if ( (*ppheader)->msg_type != (~((*pptrailer)->magic)) )
    return GDERROR_MESSAGE_FORMAT;

  crc32 = CalcCRC32(msg,msgsize-sizeof(_u32));

  if ( crc32 != (*pptrailer)->crc32 )
    return GDERROR_CRC32;

  switch((*ppheader)->msg_type)
  {
    case MC_GETSUID_REQ:
      if ( 0!=(*ppheader)->body_size)
        return GDERROR_MESSAGE_FORMAT;
      break;

    case MC_GETSUID_RESP:
      if ( SUID_LENGTH!=(*ppheader)->body_size)
        return GDERROR_MESSAGE_FORMAT;
      break;

    case MC_GENAUTHTOKEN_REQ:
      if ( sizeof(gdmc_actmsg_req)!=(*ppheader)->body_size)
        return GDERROR_MESSAGE_FORMAT;
      if ( MC_CMP_CMD_GENERATE_AUTH_TOKEN != ((gdmc_actmsg_req*)(*ppbody))->msg_type )
        return GDERROR_MESSAGE_FORMAT;
      break;

    case MC_GENAUTHTOKEN_RESP:
      if ( sizeof(gdmc_actmsg_resp)!=(*ppheader)->body_size)
        return GDERROR_MESSAGE_FORMAT;
      actmsg = (gdmc_actmsg_resp*)*ppbody;
      if ( (MC_CMP_CMD_GENERATE_AUTH_TOKEN|0x80000000) != actmsg->msg_type )
        return GDERROR_MESSAGE_FORMAT;
      if ( (SUID_LENGTH+(sizeof(_u32)*3)) != actmsg->authtok.plain_length)
        return GDERROR_MESSAGE_FORMAT;
      if ( (K_SOC_AUTH_LENGTH/*+SHA256_HASH_LENGTH+AES_BLOCK_SIZE*/) != actmsg->authtok.encrypted_length )
        return GDERROR_MESSAGE_FORMAT;
      if ( AUTHENTICATION_TOKEN != actmsg->authtok.type )
        return GDERROR_MESSAGE_FORMAT;
      if ( CONTEXT_SYSTEM != actmsg->authtok.context )
        return GDERROR_MESSAGE_FORMAT;
      if ( CONT_TYPE_SOC != actmsg->authtok.contType )
        return GDERROR_MESSAGE_FORMAT;
      if ( MC_CONT_STATE_ACTIVATED != actmsg->authtok.contState )
        return GDERROR_MESSAGE_FORMAT;
      break;

    case MC_VALIDATEAUTHTOKEN_REQ:
      if ( sizeof(gdmc_so_authtok)!=(*ppheader)->body_size)
        return GDERROR_MESSAGE_FORMAT;
      break;

    case MC_ERROR:
      if ( (*ppheader)->body_size<sizeof(gdmc_error_msg))
        return GDERROR_MESSAGE_FORMAT;
      errmsg = (gdmc_error_msg*)*ppbody;
      if ( (*ppheader)->body_size!=(errmsg->errmsg_length+sizeof(gdmc_error_msg)) )
        return GDERROR_MESSAGE_FORMAT;
      break;

    default:
      return GDERROR_MESSAGE_FORMAT;
  }

  return GDERROR_OK;
}

gderror GDPROVAPI GDMCHandleGetSUID ( gdmcinst *inst,
                                      _u8      *msgout,
                                      _u32     *msgout_size,
                                      _u32      initial_msgout_size )
{
  _u32              msgsize = sizeof(gdmc_msgheader)+SUID_LENGTH+sizeof(gdmc_msgtrailer);
  gdmc_msgheader   *header  = (gdmc_msgheader*)msgout;
  _u8              *body    = msgout+sizeof(gdmc_msgheader);
  gdmc_msgtrailer  *trailer = (gdmc_msgtrailer*)(msgout+sizeof(gdmc_msgheader)+SUID_LENGTH);
  gderror           error;

  if (msgsize>initial_msgout_size)
    return GDMCComposeErrorMessage(inst,GDERROR_BUFFER_TOO_SMALL,msgout,msgout_size,initial_msgout_size,
                                   ERRMSG_0005,initial_msgout_size,msgsize);

  if (inst->state<GDMC_STATE_HAVE_SUID) // request SUID from MobiCore
  {
    error = MCGetSUID(inst->suid);

    if (GDERROR_OK!=error)
      return GDMCComposeErrorMessage(inst,error,msgout,msgout_size,initial_msgout_size,
                                     ERRMSG_000D);

    inst->state = GDMC_STATE_HAVE_SUID;
  }

  // We have the SUID, so return the message to the caller

  header->msg_type  = MC_GETSUID_RESP;
  header->body_size = SUID_LENGTH;

  memcpy(body,inst->suid,SUID_LENGTH);

  trailer->magic   = ~MC_GETSUID_RESP;
  trailer->crc32   = CalcCRC32(msgout,msgsize-sizeof(_u32));

  *msgout_size = msgsize;

  return GDERROR_OK;
}

gderror GDPROVAPI GDMCHandleGenAuthToken ( gdmcinst          *inst,
                                           gdmc_actmsg_req   *req,
                                          _u8                *msgout,
                                          _u32               *msgout_size,
                                          _u32                initial_msgout_size )
{
  _u32              msgsize = sizeof(gdmc_msgheader)+sizeof(gdmc_actmsg_resp)+sizeof(gdmc_msgtrailer);
  gdmc_msgheader   *header  = (gdmc_msgheader*)msgout;
  gdmc_actmsg_resp *body    = (gdmc_actmsg_resp*)(msgout+sizeof(gdmc_msgheader));
  gdmc_msgtrailer  *trailer = (gdmc_msgtrailer*)(msgout+sizeof(gdmc_msgheader)+sizeof(gdmc_actmsg_resp));
  gderror           error;

  if (msgsize>initial_msgout_size)
    return GDMCComposeErrorMessage(inst,GDERROR_BUFFER_TOO_SMALL,msgout,msgout_size,initial_msgout_size,
                                   ERRMSG_0005,initial_msgout_size,msgsize);

  switch(inst->state)
  {
    case GDMC_STATE_INITIAL: // We do not have the SUID, so get it...
      error = GDMCHandleGetSUID(inst,msgout,msgout_size,initial_msgout_size);
      if (GDERROR_OK!=error)
        return error;

      // discard this message...

      memset(msgout,0,initial_msgout_size);
      *msgout_size = 0;

      // fall through...

    case GDMC_STATE_HAVE_SUID: // We have the SUID but no SO.AuthToken (yet)

      GenerateAuthToken:

      memcpy(inst->kSoCAuth,req->kSoCAuth,sizeof(inst->kSoCAuth)); // save K.SoC.Auth

      error = MCGenerateAuthToken(inst,req,&inst->authTok);

      if (GDERROR_OK!=error)
        return GDMCComposeErrorMessage(inst,error,msgout,msgout_size,initial_msgout_size,ERRMSG_000E);

      if (NULL!=g_authtok_writecb)
      {
        error = g_authtok_writecb((const _u8 *)&inst->authTok,sizeof(gdmc_so_authtok));
        if (GDERROR_OK!=error)
          return GDMCComposeErrorMessage(inst,error,msgout,msgout_size,initial_msgout_size,ERRMSG_000C);
      }

      header->msg_type  = MC_GENAUTHTOKEN_RESP;
      header->body_size = sizeof(gdmc_actmsg_resp);

      body->msg_type = MC_CMP_CMD_GENERATE_AUTH_TOKEN|0x80000000;
      memcpy(&body->authtok,&inst->authTok,sizeof(gdmc_so_authtok));

      trailer->magic   = ~MC_GENAUTHTOKEN_RESP;
      trailer->crc32   = CalcCRC32(msgout,msgsize-sizeof(_u32));

      *msgout_size = msgsize;

      if (inst->state<GDMC_STATE_HAVE_AUTHTOK)
        inst->state = GDMC_STATE_HAVE_AUTHTOK;

      return GDERROR_OK;

    default: //case GDMC_STATE_HAVE_AUTHTOK: -> We have already the SO.AuthTok, check if K.SoC.Auth still matches!!!

      if (memcmp(inst->kSoCAuth,req->kSoCAuth,sizeof(inst->kSoCAuth)))
      {
        // Oh oh... the KPH generated a new K.SoC.Auth and our SO.AuthToken is invalid now... (generate new one)

        memset(&inst->authTok,0,sizeof(inst->authTok));
        inst->state = GDMC_STATE_HAVE_SUID;
        goto GenerateAuthToken;
      }

      // Okay, K.SoC.Auth still matches and we still have the SO.AuthToken

      header->msg_type  = MC_GENAUTHTOKEN_RESP;
      header->body_size = sizeof(gdmc_actmsg_resp);

      body->msg_type = MC_CMP_CMD_GENERATE_AUTH_TOKEN|0x80000000;
      memcpy(&body->authtok,&inst->authTok,sizeof(gdmc_so_authtok));

      trailer->magic   = ~MC_GENAUTHTOKEN_RESP;
      trailer->crc32   = CalcCRC32(msgout,msgsize-sizeof(_u32));

      *msgout_size = msgsize;

      return GDERROR_OK;
  }
}

gderror GDPROVAPI GDMCHandleValidateAuthToken ( gdmcinst         *inst,
                                                gdmc_so_authtok  *validateSoAuthTok,
                                                _u8              *msgout,
                                                _u32             *msgout_size,
                                                _u32              initial_msgout_size )
{
  _u32              msgsize = sizeof(gdmc_msgheader)+sizeof(gdmc_error_msg)+sizeof(gdmc_msgtrailer);
  gdmc_msgheader   *header  = (gdmc_msgheader*)msgout;
  gdmc_error_msg   *body    = (gdmc_error_msg*)(msgout+sizeof(gdmc_msgheader));
  gdmc_msgtrailer  *trailer = (gdmc_msgtrailer*)(msgout+sizeof(gdmc_msgheader)+sizeof(gdmc_error_msg));
  gderror           error;
  gdmc_so_authtok   rb_authtok;
  _u32              rb_authtok_size;

  if (msgsize>initial_msgout_size)
    return GDMCComposeErrorMessage(inst,GDERROR_BUFFER_TOO_SMALL,msgout,msgout_size,initial_msgout_size,
                                   ERRMSG_0005,initial_msgout_size,msgsize);

  if (GDMC_STATE_HAVE_AUTHTOK!=inst->state) // Too early call: We do not have an SO.AuthToken to be validated!
    return GDMCComposeErrorMessage(inst,GDERROR_VALIDATION_FAILURE,msgout,msgout_size,initial_msgout_size,
                                   ERRMSG_000F,initial_msgout_size,msgsize);

  header->msg_type  = MC_ERROR;
  header->body_size = sizeof(gdmc_error_msg);

  body->errorcode = GDERROR_PROVISIONING_DONE;

  // 1.) First of all, compare the delivered SO.AuthToken with the one we have stored in our instance

  if (memcmp(validateSoAuthTok,&inst->authTok,sizeof(gdmc_so_authtok)))
  {
    body->errorcode = GDERROR_VALIDATION_FAILURE;
  }
  else
  {
    // 2.) Perform readback (if available) and re-check auth token

    if (NULL!=g_authtok_readcb)
    {
      rb_authtok_size = sizeof(rb_authtok);

      error = g_authtok_readcb((_u8*)&rb_authtok,&rb_authtok_size);

      if (GDERROR_OK!=error)
        body->errorcode = error;
      else
      {
        if ( (rb_authtok_size!=sizeof(gdmc_so_authtok)) ||
             (memcmp(validateSoAuthTok,&rb_authtok,sizeof(gdmc_so_authtok))) )
          body->errorcode = GDERROR_VALIDATION_FAILURE;
      }
    }
  }

  trailer->magic   = ~MC_ERROR;
  trailer->crc32   = CalcCRC32(msgout,msgsize-sizeof(_u32));

  *msgout_size = msgsize;

  return GDERROR_PROVISIONING_DONE;
}
