/**
 *
 * @copyright &copy; 2010 - 2025, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * We kindly request you to use one or more of the following phrases to refer to
 * foxBMS in your hardware, software, documentation or advertising materials:
 *
 * - "This product uses parts of foxBMS&reg;"
 * - "This product includes parts of foxBMS&reg;"
 * - "This product is derived from foxBMS&reg;"
 *
 */

/**
 * @file    can_cfg_tx-async-message-definitions.h
 * @author  foxBMS Team
 * @date    2022-07-01 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup DRIVERS
 * @prefix  CANTX
 *
 * @brief   Header for the driver for the CAN module
 * @details TODO
 */

#ifndef FOXBMS__CAN_CFG_TX_ASYNC_MESSAGE_DEFINITIONS_H_
#define FOXBMS__CAN_CFG_TX_ASYNC_MESSAGE_DEFINITIONS_H_

/*========== Includes =======================================================*/
#include "can_cfg.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/** CAN message properties for debug response message. Required properties are:
 *  - ID
 *  - Identifier type (standard or extended)
 *  - data length
 *  - Endianness @{*/
#define CANTX_DEBUG_RESPONSE_ID         (0x301u)
#define CANTX_DEBUG_RESPONSE_ID_TYPE    (CAN_STANDARD_IDENTIFIER_11_BIT)
#define CANTX_DEBUG_RESPONSE_ENDIANNESS (CAN_BIG_ENDIAN)
#define CANTX_DEBUG_RESPONSE_DLC        (CAN_DEFAULT_DLC)
/**@}*/

/** CAN message properties for unsupported multiplexer values in messages.
 *  Required properties are:
 *  - ID
 *  - Identifier type (standard or extended)
 *  - data length
 *  - Endianness @{*/
#define CANTX_DEBUG_UNSUPPORTED_MULTIPLEXER_VAL_ID         (0x302u)
#define CANTX_DEBUG_UNSUPPORTED_MULTIPLEXER_VAL_ID_TYPE    (CAN_STANDARD_IDENTIFIER_11_BIT)
#define CANTX_DEBUG_UNSUPPORTED_MULTIPLEXER_VAL_ENDIANNESS (CAN_BIG_ENDIAN)
#define CANTX_DEBUG_UNSUPPORTED_MULTIPLEXER_VAL_DLC        (CAN_DEFAULT_DLC)
/**@}*/

/** CAN message properties for debug build configuration message.
 *  Required properties are:
 *  - ID
 *  - Identifier type (standard or extended)
 *  - data length
 *  - Endianness @{*/
#define CANTX_DEBUG_BUILD_CONFIGURATION_ID         (0x303u)
#define CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE    (CAN_STANDARD_IDENTIFIER_11_BIT)
#define CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS (CAN_BIG_ENDIAN)
#define CANTX_DEBUG_BUILD_CONFIGURATION_DLC        (CAN_DEFAULT_DLC)
/**@}*/

/** CAN message properties for crash dump. Required properties are:
 *  - ID
 *  - Identifier type (standard or extended)
 *  - data length
 *  - Endianness @{*/
#define CANTX_CRASH_DUMP_ID         (0xFFu)
#define CANTX_CRASH_DUMP_ID_TYPE    (CAN_STANDARD_IDENTIFIER_11_BIT)
#define CANTX_CRASH_DUMP_ENDIANNESS (CAN_BIG_ENDIAN)
#define CANTX_CRASH_DUMP_DLC        (CAN_DEFAULT_DLC)
/**@}*/

/** CAN message properties for fatal error transmission. Required properties are:
 *  - ID
 *  - Identifier type (standard or extended)
 *  - data length
 *  - Endianness @{*/
#define CANTX_BMS_FATAL_ERROR_ID         (0x222u)
#define CANTX_BMS_FATAL_ERROR_ID_TYPE    (CAN_STANDARD_IDENTIFIER_11_BIT)
#define CANTX_BMS_FATAL_ERROR_ENDIANNESS (CAN_BIG_ENDIAN)
#define CANTX_BMS_FATAL_ERROR_DLC        (CAN_DEFAULT_DLC)
/**@}*/

/**
 * -------------------------CAUTION-------------------------
 * The following defines are used by the insulation monitoring device (IMD).
 * If they are changed, the IMD will not work anymore
 * -------------------------CAUTION-------------------------
 */
/** CAN message properties for imd request. Required properties are:
 *  - ID
 *  - Identifier type (standard or extended)
 *  - data length
 *  - Endianness @{*/
#define CANTX_IMD_REQUEST_ID         (0x022u)
#define CANTX_IMD_REQUEST_ID_TYPE    (CAN_STANDARD_IDENTIFIER_11_BIT)
#define CANTX_IMD_REQUEST_ENDIANNESS (CAN_LITTLE_ENDIAN)
#define CANTX_IMD_REQUEST_DLC        (5u)
/**@}*/

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
#endif

#endif /* FOXBMS__CAN_CFG_TX_ASYNC_MESSAGE_DEFINITIONS_H_ */
