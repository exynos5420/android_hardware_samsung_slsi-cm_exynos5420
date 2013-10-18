///
/// @file       gdmcprovlib.h
/// @author     Giesecke & Devrient GmbH, Munich, Germany
///
/// This header file declares simple data types and functions
/// comprising the G&D Provisioning API.
///

#ifndef _INC_GDPROVLIB_H_
#define _INC_GDPROVLIB_H_

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

// Check defines (macros)...

#if !defined(WIN32) && !defined(LINUX) && !defined(ARM)
#error "You MUST define one of WIN32, LINUX, and ARM (platform)."
#endif

#if !defined(_32BIT) && !defined(_64BIT)
#error "You MUST define either _32BIT or _64BIT."
#endif

#if !defined(_LENDIAN) && !defined(_BENDIAN)
#error "You MUST define either _LENDIAN or _BENDIAN."
#endif

// Declare simple signed and unsigned integer types

/// a byte (octet), unsigned, 0..255
typedef unsigned char             _u8;

/// a signed byte, -128..+127
typedef signed char               _i8;

/// an unsigned 16bit integer, 0..65.535
typedef unsigned short            _u16;

/// a signed 16bit integer, -32.768..+32.767
typedef signed short              _i16;

/// an unsigned 32bit integer, 0..4.294.967.295
typedef unsigned int              _u32;

/// a signed 32bit integer, -2.147.483.648..+2.147.483.647
typedef signed int                _i32;

#ifdef WIN32

#define GDPUBLIC
#define GDPROVAPI         __fastcall

/// an unsigned 64bit integer, 0..18.446.744.073.709.551.615
typedef unsigned __int64          _u64;

/// a signed 64bit integer, -9.223.372.036.854.775.808..+9.223.372.036.854.775.807
typedef signed __int64            _i64;

#else

#define GDPUBLIC          __attribute__((visibility("default")))
#define GDPROVAPI

#ifdef _32BIT

/// an unsigned 64bit integer, 0..18.446.744.073.709.551.615
typedef unsigned long long        _u64;

/// a signed 64bit integer, -9.223.372.036.854.775.808..+9.223.372.036.854.775.807
typedef signed long long          _i64;

#else // 64bit

/// an unsigned 64bit integer, 0..18.446.744.073.709.551.615
typedef unsigned long             _u64;

/// a signed 64bit integer, -9.223.372.036.854.775.808..+9.223.372.036.854.775.807
typedef signed long               _i64;

#endif // _32BIT

#endif // WIN32

//////////////////////////////////////////////////////////////////////////////

/// G&D error codes, which are unsigned 32bit integers
typedef _u32                          gderror;

/// everything okay, operation successful
#define GDERROR_OK                    ((gderror)0x00000000)

/// one or more of the input parameters to a function is/are invalid
#define GDERROR_PARAMETER             ((gderror)0x00000001)

/// connection problem occured, unable to establish a connection to the
/// Key Provisioning Host (KPH)
#define GDERROR_CONNECTION            ((gderror)0x00000002)

/// communication problem occured, unable to communicate with the
/// Key Provisioning Host (KPH)
#define GDERROR_COMMUNICATION         ((gderror)0x00000003)

/// GDMCProvShutdownLibrary was called without calling GDMCProvInitializeLibrary
#define GDERROR_NOT_INITIALIZED       ((gderror)0x00000004)

/// GDMCProvBeginProvisioning called but no more handles available
#define GDERROR_NO_MORE_HANDLES       ((gderror)0x00000005)

/// An unknown or invalid gdhandle was passed to a function
#define GDERROR_INVALID_HANDLE        ((gderror)0x00000006)

/// A so called structured exception occured, which is a severe error
/// (MS Windows only)
#define GDERROR_CPU_EXCEPTION         ((gderror)0x00000007)

/// Unable to retrieve the SUID of the SoC
#define GDERROR_CANT_GET_SUID         ((gderror)0x00000008)

/// Unable to generate the authentication token SO.AuthToken
#define GDERROR_CANT_BUILD_AUTHTOKEN  ((gderror)0x00000009)

/// Unable to dump the authentication token SO.AuthToken
#define GDERROR_CANT_DUMP_AUTHTOKEN   ((gderror)0x0000000A)

/// Unable to generate the receipt SD.Receipt
#define GDERROR_CANT_BUILD_RECEIPT    ((gderror)0x0000000B)

/// (only product version): Authentication KPH Connector <-> Key Provisioning Host (KPH) failed
#define GDERROR_AUTH_FAILED           ((gderror)0x0000000C)

/// validation of the device binding failed
#define GDERROR_VALIDATION_FAILURE    ((gderror)0x0000000D)

/// insufficient memory available
#define GDERROR_INSUFFICIENT_MEMORY   ((gderror)0x0000000E)

/// synchronization error occurred (thread concurrency)
#define GDERROR_SYNCHRONIZATION       ((gderror)0x0000000F)

/// the Key Provisioning Host (KPH) was not able to generate a random key (TRNG)
#define GDERROR_CANT_GENERATE_KEY     ((gderror)0x00000010)

/// the received cryptographic message format is erroneous
#define GDERROR_MESSAGE_FORMAT        ((gderror)0x00000011)

/// CRC32 checksum error
#define GDERROR_CRC32                 ((gderror)0x00000012)

/// Hash value (message digest) validation error
#define GDERROR_MESSAGE_DIGEST        ((gderror)0x00000013)

/// SUID comparison failed
#define GDERROR_SUID_MISMATCH         ((gderror)0x00000014)

/// the Device could not generate the authentication token SO.AuthToken for any reason
#define GDERROR_GENAUTHTOK_FAILED     ((gderror)0x00000015)

/// the Device could not wrap the authentication token in a secure object (SO)
#define GDERROR_WRAPOBJECT_FAILED     ((gderror)0x00000016)

/// the Device could not store SO.AuthToken for any reason
#define GDERROR_STORE_SO_FAILED       ((gderror)0x00000017)

/// the Key Provisioning Host (KPH) could not generate the receipt SD.Receipt for any reason
#define GDERROR_GENRECEIPT_FAILED     ((gderror)0x00000018)

/// the Key Provisioning Host (KPH) triggered a SO.AuthToken validation in the Device but no SO.AuthToken is available
#define GDERROR_NO_AUTHTOK_AVAILABLE  ((gderror)0x00000019)

/// the Device could not perform a read-back of the recently stored SO.AuthToken
#define GDERROR_AUTHTOK_RB_FAILED     ((gderror)0x0000001A)

/// the called API function is not implemented
#define GDERROR_NOT_IMPLEMENTED       ((gderror)0x0000001B)

/// generic (unspecified) error
#define GDERROR_UNKNOWN               ((gderror)0x0000001C)

/// MobiCore library initialization or cleanup failed
#define GDERROR_MOBICORE_LIBRARY      ((gderror)0x0000001D)

/// supplied (output) buffer too small
#define GDERROR_BUFFER_TOO_SMALL      ((gderror)0x0000001E)

/// cryptographic-related error occured, e.g. loading of RSA keys, etc.
#define GDERROR_CRYPTO_FAILURE        ((gderror)0x0000001F)

/// no error code: device binding completed successfully
#define GDERROR_PROVISIONING_DONE     ((gderror)0x10000001)

//////////////////////////////////////////////////////////////////////////////

/// G&D handle (to one instance of the Provisioning API)
typedef _u32                      gdhandle;

/// Returns the current version of the Provisioning API.
///
/// @return an unsigned 32bit integer consisting of four bytes aa|bb|cc|dd
///         with major version (aa), minor version (bb), patch level (cc), and
///         OEM (dd), which denotes the numeric ID of an OEM.
GDPUBLIC _u32 GDPROVAPI GDMCProvGetVersion ( void );

/// [PRODUCTION STATION ONLY] Formats an error message for an error code, 
/// possibly containing more detailed information about the error. This function
/// is NOT implemented in the ARM version of the library because no diagnostic
/// messages can be displayed during the production.
///
/// @param[in]      provhandle  the handle returned by GDMCProvBeginProvisioning;
///                             can be null (0) to format a message for a global
///                             error code (not context-specific)
/// @param[in]      errorcode   the G&D error code
/// @param[in/out]  msgbuf      pointer to buffer receiving the UTF-8 encoded
///                             error message (in), buffer filled with error
///                             message (out)
/// @param[in/out]  size        size of buffer pointed to by msgbuf specified
///                             as wide characters (in), number of wide
///                             characters copied into msgbuf (out)
///
/// @return                     result code (e.g. buffer too small)
GDPUBLIC gderror GDPROVAPI GDMCProvFormatErrorMessage ( gdhandle provhandle,
                                                        gderror  errorcode,
                                                        char    *msgbuf, 
                                                        _u32    *size );

/// Initializes the G&D Provisioning API (library) globally. If called
/// by the Production Software Station, then a TLS-secured channel to
/// the Key Provisioning Host (KPH) is established.
/// In a multithreaded environment, this function has to be called from
/// the primary thread (LWP 0).
///
/// @return G&D error code
GDPUBLIC gderror GDPROVAPI GDMCProvInitializeLibrary ( void );

/// Performs a global shutdown of the G&D Provisioning API (library).
/// After this call, all resources are cleaned up and all handles are
/// closed. No functions except for GDMCProvInitializeLibrary may be
/// called anymore.
/// In a multithread environment, this function has to be called from
/// the primary thread (LWP 0).
///
/// @return G&D error code
GDPUBLIC gderror GDPROVAPI GDMCProvShutdownLibrary ( void );

/// Creates one instance of the key provisioning (aka "device binding")
///
/// @param[in/out]  provhandle  pointer to memory location receiving the
///                             handle (in), the handle or 0 (out)
///
/// @return                     G&D error code
GDPUBLIC gderror GDPROVAPI GDMCProvBeginProvisioning ( gdhandle *provhandle );

/// Destroys one instance of the key provisioning (aka "device binding")
///
/// @param[in]  provhandle      the handle returned by GDMCProvBeginProvisioning
///
/// @return                     G&D error code
GDPUBLIC gderror GDPROVAPI GDMCProvEndProvisioning ( gdhandle provhandle );

/// Executes one provisioning step of the full sequence. The caller has to
/// call this function in a loop until either an error is reported or the
/// error code GDERROR_PROVISIONING_DONE is returned (meaning successful
/// provisioning). Please refer to the MobiCore Provisioning API documentation
/// for details.
///
/// @param[in]      provhandle      the handle returned by 
///                                 GDMCProvBeginProvisioning
/// @param[in]      msgin           pointer to buffer containing the 
///                                 input message; may be NULL if no message 
///                                 available
/// @param[in]      msgin_size      size of buffer pointed to by msgin in bytes
/// @param[in/out]  msgout          pointer to buffer receiving the output
///                                 message (in); output message (out)
/// @param[in/out]  msgout_size     size of buffer pointed to by msgout in
///                                 bytes (in); number of bytes copied to msgout
///                                 (out)
///
/// @return                         G&D error code; GDERROR_PROVISIONING_DONE
///                                 if provisioning successfully completed.
GDPUBLIC gderror GDPROVAPI GDMCProvExecuteProvisioningStep ( 
                  gdhandle    provhandle,
                  const _u8  *msgin,
                  _u32        msgin_size,
                  _u8        *msgout,
                  _u32       *msgout_size );

/// [PRODUCTION STATION ONLY] Convenience function to format an SD.Receipt
///
/// @param[in]      receipt           pointer to buffer containing the 
///                                   binary SD.Receipt
/// @param[in]      receipt_size      size of binary data pointed to by 
///                                   receipt in bytes
/// @param[in/out]  fmt_receipt       pointer to buffer receiving the receipt as
///                                   a BASE64-encoded string (in); the string (out)
/// @param[in/out]  fmt_receipt_size  size of buffer pointed to by fmt_receipt in
///                                   bytes (in); number of bytes copied to 
///                                   fmt_receipt (out)
///
/// @return                           G&D error code
GDPUBLIC gderror GDPROVAPI GDMCProvFormatReceipt (
                  const _u8  *receipt,
                  _u32        receipt_size,
                  _u8        *fmt_receipt,
                  _u32       *fmt_receipt_size );

/// [PRODUCTION STATION ONLY] Convenience function to query the SUID of
/// the currently provisioned device (e.g. can be used as primary key in
/// a production database)
///
/// @param[in]      provhandle    the handle returned by 
///                               GDMCProvBeginProvisioning
/// @param[in/out]  suid          pointer to buffer (16 octets, in) receiving the
///                               SUID of the current mobile device (out)
///
/// @return                       G&D error code
GDPUBLIC gderror GDPROVAPI GDMCProvGetSUID (
                  gdhandle    provhandle,
                  _u8        *suid );

/// [DEVICE ONLY] Callback function called by the Provisioning API when
/// GDMCProvExecuteProvisioningStep is executed in the Device. This function
/// shall store the authentication token SO.AuthToken in a secure location.
///
/// @param[in]      authtok           pointer to buffer containing SO.AuthToken
/// @param[in]      authtok_size      size of buffer pointed to be authtok;
///                                   shall be 124 octets
///
/// @return                           G&D error code
typedef gderror (*authtok_writecb)( const _u8 *authtok, 
                                    _u32       authtok_size );

/// [DEVICE ONLY] Callback function called by the Provisioning API when
/// GDMCProvExecuteValidationStep is executed in the Device. This function
/// shall perform a read-back of the stored authentication token SO.AuthToken
///
/// @param[in/out]  authtok           pointer to buffer receiving SO.AuthToken
///                                   (in); buffer filled with SO.AuthToken (out)
/// @param[in/out]  authtok_size      size of buffer pointed to be authtok (in);
///                                   number of bytes copied to authtok (out);
///                                   shall be 124 octets
///
/// @return                           G&D error code
typedef gderror (*authtok_readcb)( _u8  *authtok, 
                                   _u32 *authtok_size );

/// [DEVICE ONLY] The OEM must provide two hook functions (callbacks) for the
/// reading and writing of the authentication token SO.AuthToken in the device.
/// 
/// @param[in]  writefunc   callback function called by the Provisioning API
///                         when an authentication token SO.AuthToken has to be
///                         stored
/// @param[in]  readfunc    callback function called by the Provisioning API
///                         when an authentication token SO.AuthToken has to be
///                         read back (for validation purposes)
///
/// @return                 G&D error code
GDPUBLIC gderror GDPROVAPI GDMCProvSetAuthTokenCallbacks ( 
                             authtok_writecb writefunc,
                             authtok_readcb  readfunc );

/// [PRODUCTION STATION ONLY] The configuration of the provisioning library
/// can be patched into the library binary file. If the OEM decided to perform
/// the configuration e.g. by providing the configuration information via the
/// production database, then this function can be called to configure the
/// provisioning library.
///
/// @param[in] config_string  a zero-terminated configuration string containing 
///                           the entire configuration information in a format
///                           that will be defined by G&D; the exact format of 
///                           this configuration information can be OEM-specific
///                           and will be specified in a separate document
///
/// @return                   G&D error code
GDPUBLIC gderror GDPROVAPI GDMCProvSetConfigurationString (
                            const char *config_string );

//////////////////////////////////////////////////////////////////////////////
// Declaration of message header and trailer
//////////////////////////////////////////////////////////////////////////////

#ifdef WIN32

#pragma warning ( disable : 4200 )

#pragma pack(push,1)

#define PACK_ATTR

#else // Linux

#define PACK_ATTR   __attribute__((packed))

#endif

typedef struct _gdmc_msgheader          gdmc_msgheader;
typedef struct _gdmc_msgtrailer         gdmc_msgtrailer;

/// the G&D MobiCore message header
struct _gdmc_msgheader
{
  _u32        msg_type;   ///< message type
  _u32        body_size;  ///< size of body (may be 0)
} PACK_ATTR;

/// the G&D MobiCore message trailer
struct _gdmc_msgtrailer
{
  _u32        magic;      /// message type (one's complement)
  _u32        crc32;      /// CRC32 checksum
} PACK_ATTR;

#ifdef WIN32
#pragma pack(pop)
#endif

#ifdef __cplusplus
}
#endif

#endif // _INC_GDPROVLIB_H_

