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
 * @file    can_cbs_tx_limit-values.c
 * @author  foxBMS Team
 * @date    2021-07-21 (date of creation)
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
 * @ingroup DRIVER
 * @prefix  CANTX
 *
 * @brief   CAN driver Tx callback implementation
 * @details CAN Tx callback for state messages
 */

/*========== Includes =======================================================*/
#include "battery_cell_cfg.h"

#include "can_cbs_tx.h"
#include "can_cfg_tx-message-definitions.h"
#include "can_helper.h"

#include <math.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/
extern uint32_t CANTX_LimitValues(
    CAN_MESSAGE_PROPERTIES_s message,
    uint8_t *pCanData,
    uint8_t *pMuxId,
    const CAN_SHIM_s *const kpkCanShim) {
    /* pMuxId is not used here, therefore has to be NULL_PTR */
    FAS_ASSERT(pMuxId == NULL_PTR);
    FAS_ASSERT(message.id == CANTX_LIMIT_VALUES_ID);
    FAS_ASSERT(message.idType == CANTX_LIMIT_VALUES_ID_TYPE);
    FAS_ASSERT(message.dlc == CAN_FOXBMS_MESSAGES_DEFAULT_DLC);
    FAS_ASSERT(pCanData != NULL_PTR);
    FAS_ASSERT(kpkCanShim != NULL_PTR);
    uint64_t messageData = 0u;

    DATA_READ_DATA(kpkCanShim->pTableSof);

    /* AXIVION Disable Style Generic-NoMagicNumbers: Signal data defined in .dbc file. */
    /* maximum charge current */
    float_t signalData = (float_t)kpkCanShim->pTableSof->recommendedContinuousPackChargeCurrent_mA;
    float_t offset     = 0.0f;
    float_t factor     = 0.004f; /* convert mA to 250mA */
    signalData         = (signalData + offset) * factor;
    uint64_t data      = (int64_t)signalData;
    /* set data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(&messageData, 11u, 12u, data, message.endianness);

    /* maximum discharge current */
    signalData = (float_t)kpkCanShim->pTableSof->recommendedContinuousPackDischargeCurrent_mA;
    offset     = 0.0f;
    factor     = 0.004f; /* convert mA to 250mA */
    signalData = (signalData + offset) * factor;
    data       = (int64_t)signalData;
    /* set data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(&messageData, 7u, 12u, data, message.endianness);

    /* TODO: maximum charge power */
    /* TODO: maximum discharge power */

    /* minimum pack voltage */
    signalData = (float_t)(BS_NR_OF_CELL_BLOCKS_PER_STRING * BC_VOLTAGE_MIN_MSL_mV);
    offset     = 0.0f;
    factor     = 0.00025f; /* convert mV to 4V */
    signalData = (signalData + offset) * factor;
    data       = (int64_t)signalData;
    /* set data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(&messageData, 63u, 8u, data, message.endianness);

    /* maximum pack voltage */
    signalData = (float_t)(BS_NR_OF_CELL_BLOCKS_PER_STRING * BC_VOLTAGE_MAX_MSL_mV);
    offset     = 0.0f;
    factor     = 0.00025f; /* convert mV to 4V */
    signalData = (signalData + offset) * factor;
    data       = (int64_t)signalData;
    /* set data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(&messageData, 55u, 8u, data, message.endianness);
    /* AXIVION Enable Style Generic-NoMagicNumbers: */

    /* now copy data in the buffer that will be used to send data */
    CAN_TxSetCanDataWithMessageData(messageData, pCanData, message.endianness);

    return 0;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
