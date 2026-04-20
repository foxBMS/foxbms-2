/**
 *
 * @copyright &copy; 2010 - 2026, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * @file    can_cbs_tx_f_sys-state.c
 * @author  foxBMS Team
 * @date    2021-07-21 (date of creation)
 * @updated 2026-04-20 (date of last update)
 * @version v1.11.0
 * @ingroup DRIVERS
 * @prefix  CANTX
 *
 * @brief   CAN driver Tx callback implementation
 * @details CAN Tx callback for state messages
 */

/*========== Includes =======================================================*/
#include "can.h"
#include "sys.h"
/* AXIVION Next Codeline Generic-LocalInclude: 'can_cbs_tx_cyclic.h' declares
 * the prototype for the callback 'CANTX_SysState' */
#include "can_cbs_tx_cyclic.h"
#include "can_cfg_tx-cyclic-message-definitions.h"
#include "can_helper.h"
#include "diag.h"

/*========== Macros and Definitions =========================================*/

#define CANTX_SIGNAL_SYS_STATE_START_BIT    (0u)
#define CANTX_SIGNAL_SYS_STATE_LENGTH       (5u)
#define CANTX_SIGNAL_SYS_SUBSTATE_START_BIT (7u)
#define CANTX_SIGNAL_SYS_SUBSTATE_LENGTH    (7u)

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/
extern uint32_t CANTX_SysState(
    CAN_MESSAGE_PROPERTIES_s message,
    uint8_t *pCanData,
    uint8_t *pMuxId,
    const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(message.id == CANTX_SYSTEM_STATE_ID);
    FAS_ASSERT(message.dlc == CANTX_SYSTEM_STATE_DLC);
    FAS_ASSERT(message.idType == CANTX_SYSTEM_STATE_ID_TYPE);
    FAS_ASSERT(message.endianness == CANTX_SYSTEM_STATE_ENDIANNESS);
    FAS_ASSERT(pCanData != NULL_PTR);
    FAS_ASSERT(pMuxId == NULL_PTR); /* pMuxId is not used here, therefore has to be NULL_PTR */
    FAS_ASSERT(kpkCanShim != NULL_PTR);
    uint64_t messageData = 0u;

    /* SYS State */
    uint64_t data = (uint64_t)SYS_GetSystemState();
    /* set data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(
        &messageData,
        CANTX_SIGNAL_SYS_STATE_START_BIT,
        CANTX_SIGNAL_SYS_STATE_LENGTH,
        data,
        CANTX_BMS_STATE_ENDIANNESS);

    /* SYS Substate */
    data = (uint64_t)SYS_GetSystemSubstate();
    /* set data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(
        &messageData,
        CANTX_SIGNAL_SYS_SUBSTATE_START_BIT,
        CANTX_SIGNAL_SYS_SUBSTATE_LENGTH,
        data,
        CANTX_BMS_STATE_ENDIANNESS);

    CAN_TxSetCanDataWithMessageData(messageData, pCanData, CANTX_BMS_STATE_ENDIANNESS);

    return 0u;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
