/**
 *
 * @copyright &copy; 2010 - 2023, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * - &Prime;This product uses parts of foxBMS&reg;&Prime;
 * - &Prime;This product includes parts of foxBMS&reg;&Prime;
 * - &Prime;This product is derived from foxBMS&reg;&Prime;
 *
 */

/**
 * @file    can_cbs_tx_string-state-estimation.c
 * @author  foxBMS Team
 * @date    2021-07-21 (date of creation)
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
 * @ingroup DRIVER
 * @prefix  CANTX
 *
 * @brief   CAN driver Tx callback implementation
 * @details CAN Tx callback for state estimation messages
 */

/*========== Includes =======================================================*/
#include "bms.h"
#include "can_cbs_tx.h"
#include "can_cfg_tx-message-definitions.h"
#include "can_helper.h"
#include "foxmath.h"

#include <math.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/

extern uint32_t CANTX_StringStateEstimation(
    CAN_MESSAGE_PROPERTIES_s message,
    uint8_t *pCanData,
    uint8_t *pMuxId,
    const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(message.id == CANTX_STRING_STATE_ESTIMATION_ID);
    FAS_ASSERT(message.idType == CANTX_STRING_STATE_ESTIMATION_ID_TYPE);
    FAS_ASSERT(message.dlc == CAN_FOXBMS_MESSAGES_DEFAULT_DLC);
    FAS_ASSERT(pCanData != NULL_PTR);
    FAS_ASSERT(pMuxId != NULL_PTR);
    FAS_ASSERT(*pMuxId < BS_NR_OF_STRINGS);
    FAS_ASSERT(kpkCanShim != NULL_PTR);
    uint64_t messageData = 0u;

    /** Database entry with state estimation values does not need to be read
     *  within this callback as it is already read by function
     *  #CANTX_PackStateEstimation */
    const uint8_t stringNumber = *pMuxId;

    /* set multiplexer in CAN frame */
    /* AXIVION Disable Style Generic-NoMagicNumbers: Signal data defined in .dbc file. */
    uint64_t data = (uint64_t)stringNumber;
    CAN_TxSetMessageDataWithSignalData(&messageData, 7u, 4u, data, message.endianness);

    /* Minimum SOC */
    float_t signalData = kpkCanShim->pTableSox->minimumSoc_perc[stringNumber];
    float_t offset     = 0.0f;
    float_t factor     = 4.0f; /* convert from perc to 0.25perc */
    signalData         = (signalData + offset) * factor;
    data               = (int64_t)signalData;
    /* set data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(&messageData, 3u, 9u, data, message.endianness);

    /* Average SOC */
    signalData = kpkCanShim->pTableSox->averageSoc_perc[stringNumber];
    offset     = 0.0f;
    factor     = 4.0f; /* convert from perc to 0.25perc */
    signalData = (signalData + offset) * factor;
    data       = (int64_t)signalData;
    /* set data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(&messageData, 10u, 9u, data, message.endianness);

    /* Maximum SOC */
    signalData = kpkCanShim->pTableSox->maximumSoc_perc[stringNumber];
    offset     = 0.0f;
    factor     = 4.0f; /* convert from perc to 0.25perc */
    signalData = (signalData + offset) * factor;
    data       = (int64_t)signalData;
    /* set data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(&messageData, 17u, 9u, data, message.endianness);

    /* SOE */
    if (BMS_CHARGING == BMS_GetCurrentFlowDirection(kpkCanShim->pTablePackValues->stringCurrent_mA[stringNumber])) {
        signalData = kpkCanShim->pTableSox->maximumSoe_perc[stringNumber];
    } else {
        signalData = kpkCanShim->pTableSox->minimumSoe_perc[stringNumber];
    }
    offset     = 0.0f;
    factor     = 4.0f; /* convert from perc to 0.25perc */
    signalData = (signalData + offset) * factor;
    data       = (int64_t)signalData;
    /* set data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(&messageData, 24u, 9u, data, message.endianness);

    /* SOH */
    signalData = 100.0f;
    offset     = 0.0f;
    factor     = 4.0f; /* convert from perc to 0.25perc */
    signalData = (signalData + offset) * factor;
    data       = (int64_t)signalData;
    /* set data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(&messageData, 47u, 9u, data, message.endianness);

    /* String energy */
    signalData = kpkCanShim->pTableSox->minimumSoe_Wh[stringNumber];
    offset     = 0.0f;
    factor     = 0.1f; /* convert from Wh to 10Wh */
    signalData = (signalData + offset) * factor;
    data       = (int64_t)signalData;
    /* set data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(&messageData, 54u, 15u, data, message.endianness);
    /* AXIVION Enable Style Generic-NoMagicNumbers: */

    /* now copy data in the buffer that will be used to send data */
    CAN_TxSetCanDataWithMessageData(messageData, pCanData, message.endianness);

    /* Increment multiplexer for next cell */
    (*pMuxId)++;

    /* Check mux value */
    if (*pMuxId >= BS_NR_OF_STRINGS) {
        *pMuxId = 0u;
    }

    return 0;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
