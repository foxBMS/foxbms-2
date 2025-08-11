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
 * @file    can_cbs_tx_debug-unsupported-multiplexer-values.c
 * @author  foxBMS Team
 * @date    2022-11-16 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup DRIVERS
 * @prefix  CANTX
 *
 * @brief   CAN
 * @details CAN Tx callback for unsupported requests
 */

/*========== Includes =======================================================*/
#include "general.h"

#include "can_cbs_tx_debug-unsupported-multiplexer-values.h"

#include "can.h"
#include "can_cfg_tx-async-message-definitions.h"
#include "can_helper.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/
#define CANTX_UNSUPPORTED_MULTIPLEXER_VAL_MESSAGE_SIGNAL_MESSAGE_ID_START_BIT        (0u)
#define CANTX_UNSUPPORTED_MULTIPLEXER_VAL_MESSAGE_SIGNAL_MESSAGE_ID_LENGTH           (16u)
#define CANTX_UNSUPPORTED_MULTIPLEXER_VAL_MESSAGE_SIGNAL_MULTIPLEXER_VALUE_START_BIT (16u)
#define CANTX_UNSUPPORTED_MULTIPLEXER_VAL_MESSAGE_SIGNAL_MULTIPLEXER_VALUE_LENGTH    (16u)

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/

extern void CANTX_DebugUnsupportedMultiplexerVal(uint32_t messageId, uint32_t multiplexerValue) {
    /* AXIVION Routine Generic-MissingParameterAssert: messageId: parameter accepts whole range */
    /* AXIVION Routine Generic-MissingParameterAssert: multiplexerValue: parameter accepts whole range */
    uint8_t data[]   = {GEN_REPEAT_U(0u, GEN_STRIP(CAN_MAX_DLC))};
    uint64_t message = 0u;
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_UNSUPPORTED_MULTIPLEXER_VAL_MESSAGE_SIGNAL_MESSAGE_ID_START_BIT,
        CANTX_UNSUPPORTED_MULTIPLEXER_VAL_MESSAGE_SIGNAL_MESSAGE_ID_LENGTH,
        messageId,
        CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_UNSUPPORTED_MULTIPLEXER_VAL_MESSAGE_SIGNAL_MULTIPLEXER_VALUE_START_BIT,
        CANTX_UNSUPPORTED_MULTIPLEXER_VAL_MESSAGE_SIGNAL_MULTIPLEXER_VALUE_LENGTH,
        multiplexerValue,
        CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData(message, &data[0], CAN_BIG_ENDIAN);
    (void)CAN_DataSend(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_UNSUPPORTED_MULTIPLEXER_VAL_ID,
        CANTX_DEBUG_UNSUPPORTED_MULTIPLEXER_VAL_ID_TYPE,
        &data[0]);
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
