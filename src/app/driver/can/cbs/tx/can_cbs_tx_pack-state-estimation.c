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
 * @file    can_cbs_tx_pack-state-estimation.c
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
extern uint32_t CANTX_PackStateEstimation(
    CAN_MESSAGE_PROPERTIES_s message,
    uint8_t *pCanData,
    uint8_t *pMuxId,
    const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(message.id == CANTX_PACK_STATE_ESTIMATION_ID);
    FAS_ASSERT(message.idType == CANTX_PACK_STATE_ESTIMATION_ID_TYPE);
    FAS_ASSERT(message.dlc <= CAN_MAX_DLC);
    FAS_ASSERT(pCanData != NULL_PTR);
    FAS_ASSERT(pMuxId == NULL_PTR); /* pMuxId is not used here, therefore has to be NULL_PTR */
    FAS_ASSERT(kpkCanShim != NULL_PTR);
    uint64_t messageData = 0u;

    float_t minimumStringSoc_perc   = FLT_MAX;
    float_t maximumStringSoc_perc   = FLT_MIN;
    float_t minimumStringSoe_perc   = FLT_MAX;
    float_t maximumStringSoe_perc   = FLT_MIN;
    uint32_t minimumStringEnergy_Wh = UINT32_MAX;

    DATA_READ_DATA(kpkCanShim->pTableSox);

    /* Check current direction  */
    if (BMS_GetBatterySystemState() == BMS_CHARGING) {
        /* If battery system is charging use maximum values */
        for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
            if (BMS_IsStringClosed(s) == true) {
                if (maximumStringSoc_perc < kpkCanShim->pTableSox->maximumSoc_perc[s]) {
                    maximumStringSoc_perc = kpkCanShim->pTableSox->maximumSoc_perc[s];
                }
                if (maximumStringSoe_perc < kpkCanShim->pTableSox->maximumSoe_perc[s]) {
                    maximumStringSoe_perc = kpkCanShim->pTableSox->maximumSoe_perc[s];
                }
                if (minimumStringEnergy_Wh > kpkCanShim->pTableSox->minimumSoe_Wh[s]) {
                    minimumStringEnergy_Wh = kpkCanShim->pTableSox->minimumSoe_Wh[s];
                }
            }
        }
    } else {
        /* If battery system is discharging or at rest use minimum values */
        for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
            if (BMS_IsStringClosed(s) == true) {
                if (minimumStringSoc_perc > kpkCanShim->pTableSox->minimumSoc_perc[s]) {
                    minimumStringSoc_perc = kpkCanShim->pTableSox->minimumSoc_perc[s];
                }
                if (minimumStringSoe_perc > kpkCanShim->pTableSox->minimumSoe_perc[s]) {
                    minimumStringSoe_perc = kpkCanShim->pTableSox->minimumSoe_perc[s];
                }
                if (minimumStringEnergy_Wh > kpkCanShim->pTableSox->minimumSoe_Wh[s]) {
                    minimumStringEnergy_Wh = kpkCanShim->pTableSox->minimumSoe_Wh[s];
                }
            }
        }
    }

    float_t packSoc_perc       = 0.0f;
    float_t packSoe_perc       = 0.0f;
    uint32_t packEnergyLeft_Wh = 0u;

    /* Calculate pack value */
    if (BMS_GetNumberOfConnectedStrings() != 0u) {
        if (BMS_GetBatterySystemState() == BMS_CHARGING) {
            packSoc_perc = (BMS_GetNumberOfConnectedStrings() * maximumStringSoc_perc) / BS_NR_OF_STRINGS;
            packSoe_perc = (BMS_GetNumberOfConnectedStrings() * maximumStringSoe_perc) / BS_NR_OF_STRINGS;
        } else {
            packSoc_perc = (BMS_GetNumberOfConnectedStrings() * minimumStringSoc_perc) / BS_NR_OF_STRINGS;
            packSoe_perc = (BMS_GetNumberOfConnectedStrings() * minimumStringSoe_perc) / BS_NR_OF_STRINGS;
        }
        packEnergyLeft_Wh = BMS_GetNumberOfConnectedStrings() * minimumStringEnergy_Wh;
    } else {
        packSoc_perc      = 0.0f;
        packSoe_perc      = 0.0f;
        packEnergyLeft_Wh = 0u;
    }

    /* SOC */
    float_t signalData = packSoc_perc;
    float_t offset     = 0.0f;
    float_t factor     = 100.0f; /* convert from perc to 0.01perc */
    signalData         = (signalData + offset) * factor;
    uint64_t data      = (int64_t)signalData;
    /* set data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(&messageData, 7u, 14u, data, message.endianness);

    /* SOE */
    signalData = packSoe_perc;
    offset     = 0.0f;
    factor     = 100.0f; /* convert from perc to 0.01perc */
    signalData = (signalData + offset) * factor;
    data       = (int64_t)signalData;
    /* set data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(&messageData, 9u, 14u, data, message.endianness);

    /* Pack energy */
    signalData = packEnergyLeft_Wh;
    offset     = 0.0f;
    factor     = 0.1f; /* convert from Wh to 10Wh */
    signalData = (signalData + offset) * factor;
    data       = (int64_t)signalData;
    /* set data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(&messageData, 47u, 24u, data, message.endianness);

    /* SOH */
    signalData = 100.0f; /* TODO */
    offset     = 0.0f;
    factor     = 1.0f / 0.025f; /* convert from perc to 0.025% */
    signalData = (signalData + offset) * factor;
    data       = (int64_t)signalData;
    /* set data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(&messageData, 27u, 12u, data, message.endianness);

    /* now copy data in the buffer that will be used to send data */
    CAN_TxSetCanDataWithMessageData(messageData, pCanData, message.endianness);

    return 0;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
