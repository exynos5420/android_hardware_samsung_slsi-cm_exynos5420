///
/// @file       gdmcprovlib.cpp
/// @author     Giesecke & Devrient GmbH, Munich, Germany
///
/// Implementation of the API functions (Provisioning
/// Library)
///

#include <gdmcprovlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

//////////////////////////////////////////////////////////////////////////////
// MS Windows-specific includes
//////////////////////////////////////////////////////////////////////////////

#if defined(WIN32)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
static HMODULE g_hInstance = NULL;

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

extern "C" {
extern bool mccmOpen ( void );
extern void mccmClose ( void );
}

authtok_writecb         g_authtok_writecb = NULL;
authtok_readcb          g_authtok_readcb  = NULL;

//////////////////////////////////////////////////////////////////////////////
// API functions (implementation)
//////////////////////////////////////////////////////////////////////////////

// this API function is not available on ARM
static gderror GDPROVAPI _GDMCProvFormatErrorMessage (
                              gdhandle provhandle,
                              gderror  errorcode,
                              char    *msgbuf,
                              _u32    *size )
{
  LOG_d("++++ ENTERED GDMCProvFormatErrorMessage: NOT IMPLEMENTED.");
  return GDERROR_NOT_IMPLEMENTED;
}

static gderror GDPROVAPI _GDMCProvInitializeLibrary ( void )
{
  LOG_d("++++ ENTERED GDMCProvInitializeLibrary.");

  if (unlikely( !mccmOpen() ))
  {
	LOG_e("CMTL open FAILED.");
	LOG_d("++++ LEFT GDMCProvInitializeLibrary.");
    return GDERROR_MOBICORE_LIBRARY;
  }

  LOG_i("CMTL open successful.");

  LOG_d("++++ LEFT GDMCProvInitializeLibrary.");

  return GDERROR_OK;
}

static gderror GDPROVAPI _GDMCProvShutdownLibrary ( void )
{
  mccmClose();
  return GDERROR_OK;
}

static gderror GDPROVAPI _GDMCProvBeginProvisioning ( gdhandle *provhandle )
{
  gdmcinst       *inst;

  if (IsBadWritePtr(provhandle,sizeof(gdhandle)))
    return GDERROR_PARAMETER;

  inst = (gdmcinst*)malloc(sizeof(gdmcinst));

  if (NULL==inst)
  {
    *provhandle = 0;
    return GDERROR_INSUFFICIENT_MEMORY;
  }

  memset(inst,0,sizeof(gdmcinst));

  *provhandle = (gdhandle)inst;

  return GDERROR_OK;
}

static gderror GDPROVAPI _GDMCProvEndProvisioning ( gdhandle provhandle )
{
  gdmcinst       *inst = (gdmcinst*)provhandle;

  if (IsBadWritePtr(inst,sizeof(gdmcinst)))
    return GDERROR_PARAMETER;

  free(inst);

  return GDERROR_OK;
}

static gderror GDPROVAPI _GDMCProvExecuteProvisioningStep (
                  gdhandle    provhandle,
                  const _u8  *msgin,
                  _u32        msgin_size,
                  _u8        *msgout,
                  _u32       *msgout_size )
{
  gderror           error       = GDERROR_OK;
  gdmcinst         *inst        = (gdmcinst*)provhandle;
  gdmc_msgheader   *header      = NULL;
  _u8              *body        = NULL;
  gdmc_msgtrailer  *trailer     = NULL;
  _u32              initial_msgout_size;

  // 1.) Prolog: Check parameters...

  if (IsBadWritePtr(inst,sizeof(gdmcinst)))
    return GDERROR_PARAMETER;

  if ((0!=msgin_size) && (IsBadReadPtr(msgin,msgin_size)))
    return GDERROR_PARAMETER;

  if (IsBadWritePtr(msgout_size,sizeof(_u32)))
    return GDERROR_PARAMETER;

  initial_msgout_size = *msgout_size;

  if (0!=*msgout_size)
  {
    if (IsBadWritePtr(msgout,*msgout_size))
      return GDERROR_PARAMETER;
    memset(msgout,0,*msgout_size);
  }

  *msgout_size = 0;

  // 2.) Evaluate the message that has been received

  error = GDMCValidateProvMessage(msgin,msgin_size,&header,&body,&trailer);

  if (GDERROR_OK!=error) // something is wrong with the received message
    return GDMCComposeErrorMessage(inst,error,msgout,msgout_size,initial_msgout_size,ERRMSG_0006);

  // 3.) Check which message has been received

  switch(header->msg_type)
  {
    case MC_GETSUID_REQ:
      return GDMCHandleGetSUID(inst,msgout,msgout_size,initial_msgout_size);

    case MC_GENAUTHTOKEN_REQ:
      return GDMCHandleGenAuthToken(inst,(gdmc_actmsg_req*)body,msgout,msgout_size,initial_msgout_size);

    case MC_VALIDATEAUTHTOKEN_REQ:
      return GDMCHandleValidateAuthToken(inst,(gdmc_so_authtok*)body,msgout,msgout_size,initial_msgout_size);

    default:
      return GDMCComposeErrorMessage(inst,GDERROR_UNKNOWN,msgout,msgout_size,initial_msgout_size,ERRMSG_0007);
  }
}

//////////////////////////////////////////////////////////////////////////////
// Structured Exception Handling (Windows only)
//////////////////////////////////////////////////////////////////////////////

#if defined(WIN32) && !defined(_NO_STRUCTURED_EXCEPTIONS)

static DWORD GDPROVAPI HandleStructuredException ( DWORD dwExcepCode )
{
#ifndef _DEBUG
  return EXCEPTION_EXECUTE_HANDLER;
#else // _DEBUG
  switch(dwExcepCode)
  {
    case EXCEPTION_BREAKPOINT:
    case EXCEPTION_SINGLE_STEP:
      return EXCEPTION_CONTINUE_SEARCH;
    default:
      return EXCEPTION_EXECUTE_HANDLER;
  }
#endif
}

#define SE_TRY          __try {
#define SE_CATCH        } __except(HandleStructuredException(GetExceptionCode())) { return GDERROR_CPU_EXCEPTION; }

#else // !WIN32 || _NO_STRUCTURED_EXCEPTIONS

#define SE_TRY
#define SE_CATCH

#endif // WIN32

//////////////////////////////////////////////////////////////////////////////
// API functions (exported)
//////////////////////////////////////////////////////////////////////////////

extern "C" _u32 GDPROVAPI GDMCProvGetVersion ( void )
{
  return GDMCPROVLIB_VERSION;
}

extern "C" gderror GDPROVAPI GDMCProvFormatErrorMessage (
                              gdhandle provhandle,
                              gderror  errorcode,
                              char    *msgbuf,
                              _u32    *size )
{
  SE_TRY // MUST BE FIRST INSTRUCTION ////////////////////////////////////////

  return _GDMCProvFormatErrorMessage(provhandle,errorcode,msgbuf,size);

  SE_CATCH // MUST BE LAST INSTRUCTION ///////////////////////////////////////
}

extern "C" gderror GDPROVAPI GDMCProvInitializeLibrary ( void )
{
  SE_TRY // MUST BE FIRST INSTRUCTION ////////////////////////////////////////

  return _GDMCProvInitializeLibrary();

  SE_CATCH // MUST BE LAST INSTRUCTION ///////////////////////////////////////
}

extern "C" gderror GDPROVAPI GDMCProvShutdownLibrary ( void )
{
  SE_TRY // MUST BE FIRST INSTRUCTION ////////////////////////////////////////

  return _GDMCProvShutdownLibrary();

  SE_CATCH // MUST BE LAST INSTRUCTION ///////////////////////////////////////
}

extern "C" gderror GDPROVAPI GDMCProvBeginProvisioning ( gdhandle *provhandle )
{
  SE_TRY // MUST BE FIRST INSTRUCTION ////////////////////////////////////////

  return _GDMCProvBeginProvisioning(provhandle);

  SE_CATCH // MUST BE LAST INSTRUCTION ///////////////////////////////////////
}

extern "C" gderror GDPROVAPI GDMCProvEndProvisioning ( gdhandle provhandle )
{
  SE_TRY // MUST BE FIRST INSTRUCTION ////////////////////////////////////////

  return _GDMCProvEndProvisioning(provhandle);

  SE_CATCH // MUST BE LAST INSTRUCTION ///////////////////////////////////////
}

extern "C" gderror GDPROVAPI GDMCProvExecuteProvisioningStep (
                  gdhandle    provhandle,
                  const _u8  *msgin,
                  _u32        msgin_size,
                  _u8        *msgout,
                  _u32       *msgout_size )
{
  SE_TRY // MUST BE FIRST INSTRUCTION ////////////////////////////////////////

  return _GDMCProvExecuteProvisioningStep(provhandle,msgin,msgin_size,
                                          msgout,msgout_size);

  SE_CATCH // MUST BE LAST INSTRUCTION ///////////////////////////////////////
}

extern "C" gderror GDPROVAPI GDMCProvFormatReceipt (
                  const _u8  *receipt,
                  _u32        receipt_size,
                  _u8        *fmt_receipt,
                  _u32       *fmt_receipt_size )
{
  return GDERROR_NOT_IMPLEMENTED;
}

extern "C"  gderror GDPROVAPI GDMCProvGetSUID (
                  gdhandle    provhandle,
                  _u8        *suid )
{
  return GDERROR_NOT_IMPLEMENTED;
}

extern "C" gderror GDPROVAPI GDMCProvSetAuthTokenCallbacks (
                              authtok_writecb writefunc,
                              authtok_readcb  readfunc )
{
  g_authtok_writecb = writefunc;
  g_authtok_readcb  = readfunc;

  return GDERROR_OK;
}

extern "C" gderror GDPROVAPI GDMCProvSetConfigurationString (
                  const char *config_string )
{
#ifdef ARM

  return GDERROR_NOT_IMPLEMENTED;

#else

  SE_TRY // MUST BE FIRST INSTRUCTION ////////////////////////////////////////

  return GDERROR_OK;

  SE_CATCH // MUST BE LAST INSTRUCTION ///////////////////////////////////////

#endif
}

#ifdef WIN32

/// DLL main function required by MS Windows DLLs
///
/// @param[in]  hinstDLL      instance handle (module)
/// @param[in]  fdwReason     reason for calling (attach, detach, ...)
/// @param[in]  lpvReserved   reserved
///
/// @return     TRUE if DLL loading/unloading successful, FALSE otherwise
BOOL WINAPI DllMain ( HINSTANCE hinstDLL,
                      DWORD     fdwReason,
                      LPVOID    lpvReserved )
{
  switch(fdwReason)
  {
    case DLL_PROCESS_ATTACH:
      // We don't need additional calls with DLL_THREAD_ATTACH.
      g_hInstance = (HMODULE)hinstDLL;
      DisableThreadLibraryCalls(hinstDLL);
      InitCRCTable();
      return TRUE;
    case DLL_PROCESS_DETACH:  // fall through
    case DLL_THREAD_ATTACH:   // fall through
    case DLL_THREAD_DETACH:
      return TRUE;
    default:
      break;
  }
  return FALSE;
}

#else // library initialization and cleanup (Linux/ARM)

void gdmcprovlib_init ( void ) __attribute__((constructor));
void gdmcprovlib_fini ( void ) __attribute__((destructor));

/// shared object global initialization function; gets automatically
/// called when library is loaded
void gdmcprovlib_init ( void )
{
  InitCRCTable();
}

/// shared object global cleanup function; gets automatically
/// called when library is unloaded
void gdmcprovlib_fini ( void )
{

}

#endif // WIN32
