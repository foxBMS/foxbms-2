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
 * @file    can_cbs_tx_debug-response.h
 * @author  foxBMS Team
 * @date    2022-07-01 (date of creation)
 * @updated 2024-12-20 (date of last update)
 * @version v1.8.0
 * @ingroup DRIVERS
 * @prefix  CANTX
 *
 * @brief   Declarations for handling the transmit of debug response messages
 * @details This header declares the interface for transmitting debug response
 *          messages and everything else that is externally required to be able
 *          to use that interface.
 */

#ifndef FOXBMS__CAN_CBS_TX_DEBUG_RESPONSE_H_
#define FOXBMS__CAN_CBS_TX_DEBUG_RESPONSE_H_

/*========== Includes =======================================================*/

#include "fstd_types.h"

/*========== Macros and Definitions =========================================*/
/** defines which action is performed when #CANTX_DebugResponse is called */
typedef enum {
    CANTX_DEBUG_RESPONSE_TRANSMIT_BMS_VERSION_INFO,
    CANTX_DEBUG_RESPONSE_TRANSMIT_MCU_UNIQUE_DIE_ID,
    CANTX_DEBUG_RESPONSE_TRANSMIT_MCU_LOT_NUMBER,
    CANTX_DEBUG_RESPONSE_TRANSMIT_MCU_WAFER_INFORMATION,
    CANTX_DEBUG_RESPONSE_TRANSMIT_BOOT_MAGIC_START,
    CANTX_DEBUG_RESPONSE_TRANSMIT_BOOT_MAGIC_END,
    CANTX_DEBUG_RESPONSE_TRANSMIT_RTC_TIME,
    CANTX_DEBUG_RESPONSE_TRANSMIT_COMMIT_HASH,
} CANTX_DEBUG_RESPONSE_ACTIONS_e;

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/

/**
 * @brief   Handles Debug response message
 * @param   action  type of information that should be sent
 * @return  #STD_OK if transmission successful, otherwise #STD_NOT_OK
 */
extern STD_RETURN_TYPE_e CANTX_DebugResponse(CANTX_DEBUG_RESPONSE_ACTIONS_e action);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
extern uint64_t TEST_CANTX_TransmitBmsVersionInfo(void);
extern uint64_t TEST_CANTX_TransmitMcuUniqueDieId(void);
extern uint64_t TEST_CANTX_TransmitMcuLotNumber(void);
extern uint64_t TEST_CANTX_TransmitMcuWaferInformation(void);
extern uint64_t TEST_CANTX_TransmitBootMagic(uint64_t messageData);
extern uint64_t TEST_CANTX_TransmitBootMagicStart(void);
extern uint64_t TEST_CANTX_TransmitBootMagicEnd(void);
extern uint64_t TEST_CANTX_TransmitRtcTime(void);
extern STD_RETURN_TYPE_e TEST_CANTX_TransmitCommitHash(void);
extern uint64_t TEST_CANTX_TransmitCommitHashLow(void);
extern uint64_t TEST_CANTX_TransmitCommitHashHigh(void);
extern uint64_t TEST_CANTX_GetSevenChars(char *string, uint8_t stringLength, uint8_t startChar);
extern STD_RETURN_TYPE_e TEST_CANTX_DebugResponseSendMessage(uint64_t messageData);
#endif

#endif /* FOXBMS__CAN_CBS_TX_DEBUG_RESPONSE_H_ */
