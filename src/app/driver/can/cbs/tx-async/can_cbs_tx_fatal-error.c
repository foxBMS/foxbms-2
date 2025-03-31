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
 * @file    can_cbs_tx_fatal-error.c
 * @author  foxBMS Team
 * @date    2024-10-17 (date of creation)
 * @updated 2025-03-31 (date of last update)
 * @version v1.9.0
 * @ingroup DRIVERS
 * @prefix  CANTX
 *
 * @brief   CAN driver Tx callback implementation
 * @details CAN Tx callback for sending fatal error codes
 */

/*========== Includes =======================================================*/
#include "can_cbs_tx_fatal-error.h"

#include "can.h"
#include "can_cfg_tx-async-message-definitions.h"
#include "can_helper.h"

/*========== Macros and Definitions =========================================*/
#define CANTX_FATAL_ERROR_START_BIT (0u)
#define CANTX_FATAL_ERROR_LENGTH    (8u)

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/
/**
 * @brief   Helper function to send the actual error Code
 * @details This function takes a already created Message with signal data and
 *          adds the message data before sending it.
 * @return  #STD_OK if the message was successfully queued, #STD_NOT_OK otherwise
 */
static STD_RETURN_TYPE_e CANTX_SendMessageFatalErrorCode(uint64_t messageData);

/*========== Static Function Implementations ================================*/
static STD_RETURN_TYPE_e CANTX_SendMessageFatalErrorCode(uint64_t messageData) {
    /* AXIVION Routine Generic-MissingParameterAssert: messageData: parameter accept whole range */
    uint8_t data[] = {GEN_REPEAT_U(0u, GEN_STRIP(CAN_MAX_DLC))};

    CAN_TxSetCanDataWithMessageData(messageData, &data[0], CAN_BIG_ENDIAN);
    STD_RETURN_TYPE_e successfullyQueued =
        CAN_DataSend(CAN_NODE_FATAL_ERROR_MESSAGE, CANTX_BMS_FATAL_ERROR_ID, CANTX_BMS_FATAL_ERROR_ID_TYPE, &data[0]);

    return successfullyQueued;
}

/*========== Extern Function Implementations ================================*/
extern STD_RETURN_TYPE_e CANTX_SendFatalErrorId(uint32_t errorId) {
    /* AXIVION Routine Generic-MissingParameterAssert: errorId: parameter accept whole range */
    uint64_t message = 0u;

    CAN_TxSetMessageDataWithSignalData(
        &message, CANTX_FATAL_ERROR_START_BIT, CANTX_FATAL_ERROR_LENGTH, errorId, CAN_BIG_ENDIAN);

    STD_RETURN_TYPE_e success = CANTX_SendMessageFatalErrorCode(message);

    return success;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
extern STD_RETURN_TYPE_e TEST_CANTX_SendMessageFatalErrorCode(uint64_t messageData) {
    return CANTX_SendMessageFatalErrorCode(messageData);
}
#endif
