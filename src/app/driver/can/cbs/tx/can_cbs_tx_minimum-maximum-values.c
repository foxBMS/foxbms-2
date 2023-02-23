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
 * @file    can_cbs_tx_minimum-maximum-values.c
 * @author  foxBMS Team
 * @date    2021-04-20 (date of creation)
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
 * @ingroup DRIVER
 * @prefix  CANTX
 *
 * @brief   CAN driver Tx callback implementation
 * @details CAN Tx callback for min/max values
 */

/*========== Includes =======================================================*/
#include "bms.h"
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
extern uint32_t CANTX_MinimumMaximumValues(
    CAN_MESSAGE_PROPERTIES_s message,
    uint8_t *pCanData,
    uint8_t *pMuxId,
    const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(message.id == CANTX_MINIMUM_MAXIMUM_VALUES_ID);
    FAS_ASSERT(message.idType == CANTX_MINIMUM_MAXIMUM_VALUES_ID_TYPE);
    FAS_ASSERT(message.dlc == CAN_FOXBMS_MESSAGES_DEFAULT_DLC);
    FAS_ASSERT(pCanData != NULL_PTR);
    FAS_ASSERT(pMuxId == NULL_PTR); /* pMuxId is not used here, therefore has to be NULL_PTR */
    FAS_ASSERT(kpkCanShim != NULL_PTR);
    uint64_t messageData = 0u;

    DATA_READ_DATA(kpkCanShim->pTableMinMax);

    int16_t packMaximumVoltage_mV        = INT16_MIN;
    int16_t packMinimumVoltage_mV        = INT16_MAX;
    int16_t packMaximumTemperature_ddegC = INT16_MIN;
    int16_t packMinimumTemperature_ddegC = INT16_MAX;

    if (0u == BMS_GetNumberOfConnectedStrings()) {
        /* Calculate min/max values of complete pack if all slice switches are open */
        for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
            if (kpkCanShim->pTableMinMax->maximumCellVoltage_mV[s] >= packMaximumVoltage_mV) {
                packMaximumVoltage_mV = kpkCanShim->pTableMinMax->maximumCellVoltage_mV[s];
            }
            if (kpkCanShim->pTableMinMax->minimumCellVoltage_mV[s] <= packMinimumVoltage_mV) {
                packMinimumVoltage_mV = kpkCanShim->pTableMinMax->minimumCellVoltage_mV[s];
            }
            if (kpkCanShim->pTableMinMax->maximumTemperature_ddegC[s] >= packMaximumTemperature_ddegC) {
                packMaximumTemperature_ddegC = kpkCanShim->pTableMinMax->maximumTemperature_ddegC[s];
            }
            if (kpkCanShim->pTableMinMax->minimumTemperature_ddegC[s] <= packMinimumTemperature_ddegC) {
                packMinimumTemperature_ddegC = kpkCanShim->pTableMinMax->minimumTemperature_ddegC[s];
            }
        }
    } else {
        /* Calculate min/max values of connected slices */
        for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
            if (BMS_IsStringClosed(s) == true) {
                if (kpkCanShim->pTableMinMax->maximumCellVoltage_mV[s] >= packMaximumVoltage_mV) {
                    packMaximumVoltage_mV = kpkCanShim->pTableMinMax->maximumCellVoltage_mV[s];
                }
                if (kpkCanShim->pTableMinMax->minimumCellVoltage_mV[s] <= packMinimumVoltage_mV) {
                    packMinimumVoltage_mV = kpkCanShim->pTableMinMax->minimumCellVoltage_mV[s];
                }
                if (kpkCanShim->pTableMinMax->maximumTemperature_ddegC[s] >= packMaximumTemperature_ddegC) {
                    packMaximumTemperature_ddegC = kpkCanShim->pTableMinMax->maximumTemperature_ddegC[s];
                }
                if (kpkCanShim->pTableMinMax->minimumTemperature_ddegC[s] <= packMinimumTemperature_ddegC) {
                    packMinimumTemperature_ddegC = kpkCanShim->pTableMinMax->minimumTemperature_ddegC[s];
                }
            }
        }
    }

    /* AXIVION Disable Style Generic-NoMagicNumbers: Signal data defined in .dbc file. */
    /* Minimum cell voltage */
    float_t signalData = (float_t)packMinimumVoltage_mV;
    float_t offset     = 0.0f;
    float_t factor     = 1.0f;
    signalData         = (signalData + offset) * factor;
    uint64_t data      = (int64_t)signalData;
    /* set data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(&messageData, 10u, 13u, data, message.endianness);

    /* Maximum cell voltage */
    signalData = (float_t)packMaximumVoltage_mV;
    offset     = 0.0f;
    factor     = 1.0f;
    signalData = (signalData + offset) * factor;
    data       = (int64_t)signalData;
    /* set data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(&messageData, 7u, 13u, data, message.endianness);

    /* Minimum cell temperature */
    signalData = (float_t)packMinimumTemperature_ddegC;
    offset     = 0.0f;
    factor     = 0.1f; /* convert ddegC to degC */
    signalData = (signalData + offset) * factor;
    data       = (int64_t)signalData;
    /* set data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(&messageData, 63u, 8u, data, message.endianness);

    /* Maximum cell temperature */
    signalData = (float_t)packMaximumTemperature_ddegC;
    offset     = 0.0f;
    factor     = 0.1f; /* convert ddegC to degC */
    signalData = (signalData + offset) * factor;
    data       = (int64_t)signalData;
    /* set data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(&messageData, 55u, 8u, data, message.endianness);
    /* AXIVION Enable Style Generic-NoMagicNumbers: */

    /* now copy data in the buffer that will be used to send data */
    CAN_TxSetCanDataWithMessageData(messageData, pCanData, message.endianness);

    return 0;
}

extern uint32_t CANTX_StringMinimumMaximumValues(
    CAN_MESSAGE_PROPERTIES_s message,
    uint8_t *pCanData,
    uint8_t *pMuxId,
    const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(message.id == CANTX_STRING_MIN_MAX_VALUES_ID);
    FAS_ASSERT(message.idType == CANTX_STRING_MIN_MAX_VALUES_ID_TYPE);
    FAS_ASSERT(message.dlc == CAN_FOXBMS_MESSAGES_DEFAULT_DLC);
    FAS_ASSERT(pCanData != NULL_PTR);
    FAS_ASSERT(pMuxId != NULL_PTR);
    FAS_ASSERT(*pMuxId < BS_NR_OF_STRINGS);
    FAS_ASSERT(kpkCanShim != NULL_PTR);
    uint64_t messageData = 0u;

    /** Database entry with minimum and maximum values does not need to be read
     *  within this callback as it is already read by function
     *  #CANTX_MinimumMaximumValues */
    const uint8_t stringNumber = *pMuxId;

    /* AXIVION Disable Style Generic-NoMagicNumbers: Signal data defined in .dbc file. */
    /* Minimum cell voltage */
    float_t signalData = (float_t)kpkCanShim->pTableMinMax->minimumCellVoltage_mV[stringNumber];
    float_t offset     = 0.0f;
    float_t factor     = 1.0f;
    signalData         = (signalData + offset) * factor;
    uint64_t data      = (int64_t)signalData;
    /* set data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(&messageData, 18u, 13u, data, message.endianness);

    /* Maximum cell voltage */
    signalData = (float_t)kpkCanShim->pTableMinMax->maximumCellVoltage_mV[stringNumber];
    offset     = 0.0f;
    factor     = 1.0f;
    signalData = (signalData + offset) * factor;
    data       = (int64_t)signalData;
    /* set data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(&messageData, 15u, 13u, data, message.endianness);

    /* Minimum cell temperature */
    signalData = (float_t)kpkCanShim->pTableMinMax->minimumTemperature_ddegC[stringNumber];
    offset     = 0.0f;
    factor     = 0.2f; /* convert ddegC to 0.5degC */
    signalData = (signalData + offset) * factor;
    data       = (int64_t)signalData;
    /* set data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(&messageData, 44u, 9u, data, message.endianness);

    /* Maximum cell temperature */
    signalData = (float_t)kpkCanShim->pTableMinMax->maximumTemperature_ddegC[stringNumber];
    offset     = 0.0f;
    factor     = 0.2f; /* convert ddegC to 0.5degC */
    signalData = (signalData + offset) * factor;
    data       = (int64_t)signalData;
    /* set data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(&messageData, 37, 9u, data, message.endianness);
    /* AXIVION Enable Style Generic-NoMagicNumbers: */

    /* now copy data in the buffer that will be use to send data */
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
