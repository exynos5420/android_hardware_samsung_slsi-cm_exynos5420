/**
 * @addtogroup MC_SPID mcSpid - service provider ID.
 *
 * Copyright © Trustonic Limited 2013
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 *   1. Redistributions of source code must retain the above copyright notice, this
 *      list of conditions and the following disclaimer.
 *
 *   2. Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *
 *   3. Neither the name of the Trustonic Limited nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * @ingroup  MC_DATA_TYPES
 * @{
 */

#ifndef MC_SPID_H_
#define MC_SPID_H_

/** Service provider Identifier type. */
typedef uint32_t mcSpid_t;

/** SPID value used as free marker in root containers. */
static const mcSpid_t MC_SPID_FREE = 0xFFFFFFFF;

/** Reserved SPID value. */
static const mcSpid_t MC_SPID_RESERVED = 0;

/** SPID for system applications. */
static const mcSpid_t MC_SPID_SYSTEM = 0xFFFFFFFE;

/** SPID reserved for tests only */
static const mcSpid_t MC_SPID_RESERVED_TEST = 0xFFFFFFFD;

#endif // MC_SPID_H_

/** @} */
