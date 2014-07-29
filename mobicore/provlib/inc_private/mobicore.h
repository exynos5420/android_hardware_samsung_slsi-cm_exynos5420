/// @file mobicore.h
/// @author secunet AG (IKU)
///
/// This file is a convenience header file (top-level) including
/// all MobiCore-related and platform-specific stuff.

#ifndef _INC_MOBICORE_H_
#define _INC_MOBICORE_H_

#if !defined(LINUX) && !defined(ANDROID_ARM) && !defined(WIN32)
#error "You MUST define either LINUX or ANDROID_ARM or WIN32"
#endif

// standard C stuff...

#if defined(__cplusplus) && !defined(ANDROID_ARM)
#include <string>
#include <vector>
#include <map>
#endif

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef LINUX
#include <safemem.h>
#endif

#if defined(WIN32) && defined(_DEBUG) // enable memory leak detection
#define _CRTDBG_MAP_ALLOC
#define _CRTDBG_MAP_ALLOC_NEW
#include <windows.h>
#include <crtdbg.h>
#define MYDEBUG_NEW   new( _NORMAL_BLOCK, __FILE__, __LINE__)
#define new MYDEBUG_NEW
#endif

#ifndef _NO_OPENSSL_INCLUDES

// OpenSSL stuff...

#include <openssl/rand.h>
#include <openssl/sha.h>
#include <openssl/aes.h>
#include <openssl/rsa.h>
#include <openssl/bn.h>
#include <openssl/objects.h>
#include <openssl/err.h>

#endif

#ifdef WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#pragma pack(push,4)

#pragma warning ( disable : 4200 4996 )

#define GDPUBLIC
#define GDAPI                 __fastcall
#define PACK_ATTR
#define likely(cond)          cond
#define unlikely(cond)        cond

#define bad_read_ptr(_p,_c)   IsBadReadPtr((const void *)(_p),(UINT_PTR)(_c))
#define bad_write_ptr(_p,_c)  IsBadWritePtr((void *)(_p),(UINT_PTR)(_c))

#define PATH_SEPARATOR        "\\"
#define PATH_SEP_CHAR         '\\'
#define DYNLIB_PREFIX         ""
#define DYNLIB_EXTENSION      ".dll"

#else

#define GDPUBLIC              __attribute__((visibility("default")))
#define GDAPI
#define PACK_ATTR             __attribute__((packed))
#define likely(x)             __builtin_expect((x),1)
#define unlikely(x)           __builtin_expect((x),0)

#define bad_read_ptr(_p,_c)   (NULL==(_p))
#define bad_write_ptr(_p,_c)  (NULL==(_p))

#define PATH_SEPARATOR        "/"
#define PATH_SEP_CHAR         '/'
#define DYNLIB_PREFIX         "lib"
#define DYNLIB_EXTENSION      ".so"

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
#include <sched.h>
#include <dlfcn.h>
#include <signal.h>
#include <ctype.h>
#ifndef LINUX
#include <android/log.h>
#else
#include <syslog.h>
#endif

#endif

#include <stdbool.h>
#include <stdint.h>

// MobiCore stuff...

#ifdef WIN32
#undef UUID
#undef uuid_t
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <MobiCoreDriverApi.h>
#include <mcContainer.h>
#include <tlCmApi.h>
#include <tlCmUuid.h>
#include <mcVersionHelper.h>
#include <mcVersionInfo.h>

enum _mcAuthState
{
  AUTH_NONE     = 0,
  AUTH_SOC,
  AUTH_ROOT,
  AUTH_SP
};

typedef enum _mcAuthState mcAuthState;

#ifdef __cplusplus
}
#endif

#ifdef WIN32
#pragma pack(pop)
#endif

#include <MobiCoreRegistry.h>

#define IS_VALID_SPID(_x)     ((0xFFFFFFFF!=(_x)) && (0xFFFFFFFE!=(_x)))
#define IS_VALID_ROOTID(_x)   IS_VALID_SPID(_x)
#define IS_VALID_UUID(_x)     ( ((_x).value[ 0]!=0xFF) && ((_x).value[ 1]!=0xFF) &&\
                                ((_x).value[ 2]!=0xFF) && ((_x).value[ 3]!=0xFF) &&\
                                ((_x).value[ 4]!=0xFF) && ((_x).value[ 5]!=0xFF) &&\
                                ((_x).value[ 6]!=0xFF) && ((_x).value[ 7]!=0xFF) &&\
                                ((_x).value[ 8]!=0xFF) && ((_x).value[ 9]!=0xFF) &&\
                                ((_x).value[10]!=0xFF) && ((_x).value[11]!=0xFF) &&\
                                ((_x).value[12]!=0xFF) && ((_x).value[13]!=0xFF) &&\
                                ((_x).value[14]!=0xFF) && ((_x).value[15]!=0xFF) && ((_x).value[15]!=0xFE) )

#define MC_SO_PLAIN_SIZE(_struct)   offsetof(_struct,co)
#define MC_SO_ENC_SIZE(_struct)     sizeof(_struct.co)

#endif // _INC_MOBICORE_H_

