/**
 *
 * @copyright &copy; 2010 - 2024, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * @file    can_how-to_tx_async.c
 * @author  foxBMS Team
 * @date    2023-09-28 (date of creation)
 * @updated 2024-08-08 (date of last update)
 * @version v1.7.0
 * @ingroup SOME_GROUP
 * @prefix  CAN
 *
 * @brief   Documentation file to show how a new asynchronous transmitted CAN
 *          message is added to the project.
 *
 */

/*========== Includes =======================================================*/

#include "can_how-to_tx_async.h"

#include "can_cfg.h"

#include "can_cbs_tx_debug-response.h"
#include "fstd_types.h"

/*========== Macros and Definitions =========================================*/

/* example-can-tx-asynchronous-message-id-macro-start */
/** CAN message properties for BMS state message. Required properties are:
 * - Message ID
 * - Identifier type (standard or extended)
 * - Endianness of message data
 * - data length of the message @{*/
#define CANTX_DEBUG_RESPONSE_ID         (0x301u)
#define CANTX_DEBUG_RESPONSE_ID_TYPE    (CAN_STANDARD_IDENTIFIER_11_BIT)
#define CANTX_DEBUG_RESPONSE_ENDIANNESS (CAN_BIG_ENDIAN)
#define CANTX_DEBUG_RESPONSE_DLC        (CAN_DEFAULT_DLC)
/**@}*/
/* example-can-tx-asynchronous-message-id-macro-end */

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/

/* example-can-tx-asynchronous-messages-callback-definition-start */
extern STD_RETURN_TYPE_e CANTX_DebugResponse(CANTX_DEBUG_RESPONSE_ACTIONS_e action) {
    STD_RETURN_TYPE_e success = STD_NOT_OK;
    /* Do message handling stuff */
    return success;
}
/* example-can-tx-asynchronous-messages-callback-definition-end */

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
