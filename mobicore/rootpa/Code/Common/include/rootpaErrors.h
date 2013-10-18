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

#ifndef ROOTPAERRORS_H
#define ROOTPAERRORS_H

#include<stdint.h>

typedef uint32_t rootpaerror_t;

/*
NOTE to the maintainer. These values and documentation needs to be in line with the ones in CommandResult.java
*/


/**
 no errors detected, successful execution
*/
#define ROOTPA_OK                               0x00000000

/**
 client has requested unsupported command or command that it can not execute via the used interface
*/
#define ROOTPA_COMMAND_NOT_SUPPORTED            0x00000001
#define STRING_ROOTPA_COMMAND_NOT_SUPPORTED    "COMMAND_NOT_SUPPORTED_ERROR"

/**
either rootpa is locked by another client, or the client requests lock or unlock when it is not allowed to do that
*/
#define ROOTPA_ERROR_LOCK                       0x00000002
#define STRING_ROOTPA_ERROR_LOCK                "BUSY_ERROR"

/**
 error in one of the cmp commands, see command specific response for more details
*/
#define ROOTPA_ERROR_COMMAND_EXECUTION          0x00000003
#define STRING_ROOTPA_ERROR_COMMAND_EXECUTION   "COMMAND_EXECUTION_ERROR"

/**
mobicore registry returned an error
*/
#define ROOTPA_ERROR_REGISTRY                   0x00000004
#define STRING_ROOTPA_ERROR_REGISTRY            "REGISTRY_ERROR"

/**
error in communicating with mobicore
*/
#define ROOTPA_ERROR_MOBICORE_CONNECTION        0x00000005
#define STRING_ROOTPA_ERROR_MOBICORE_CONNECTION "MOBICORE_CONNECTION_ERROR"

/**
out of memory
*/
#define ROOTPA_ERROR_OUT_OF_MEMORY              0x00000006
#define STRING_ROOTPA_ERROR_OUT_OF_MEMORY       "OUT_OF_MEMORY_ERROR"

/**
rootpa internal error
*/
#define ROOTPA_ERROR_INTERNAL                   0x00000007
#define STRING_ROOTPA_ERROR_INTERNAL            "INTERNAL_ERROR"

/**
given argument is not allowed (in many cases it is NULL) or e.g. the format of xml is unsupported
*/
#define ROOTPA_ERROR_ILLEGAL_ARGUMENT           0x00000008


/**
error in network connection or use of networking library
*/
#define ROOTPA_ERROR_NETWORK                    0x00000009


/**
error is parsing received XML command or creating new XML response
*/
#define ROOTPA_ERROR_XML                        0x0000000A
#define STRING_ROOTPA_ERROR_XML                 "XML_ERROR"

/**
mobicore registry returned an error
*/
#define ROOTPA_ERROR_REGISTRY_OBJECT_NOT_AVAILABLE          0x0000000B
#define STRING_ROOTPA_ERROR_REGISTRY_OBJECT_NOT_AVAILABLE   "REGISTRY_OBJECT_NOT_AVAILABLE"

/**
CMP version of the device is not supported by SE
*/
#define ROOTPA_ERROR_SE_CMP_VERSION                0x0000000C

/**
Precoditions for SP container installation are not met in SE
*/
#define ROOTPA_ERROR_SE_PRECONDITION_NOT_MET        0x0000000D

/**
requested container does not exist. This is not always considered an error
but is used as an informative return code
*/
#define ROOTPA_ERROR_INTERNAL_NO_CONTAINER      0x00000030

#endif // ROOTPAERRORS_H
