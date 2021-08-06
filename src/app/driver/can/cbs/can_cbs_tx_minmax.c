/**
 *
 * @copyright &copy; 2010 - 2021, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * @file    can_cbs_tx_minmax.c
 * @author  foxBMS Team
 * @date    2021-04-20 (date of creation)
 * @updated 2021-07-21 (date of last update)
 * @ingroup DRIVER
 * @prefix  CAN
 *
 * @brief   CAN driver Tx callback implementation
 * @details CAN Tx callback for min/max values
 */

/*========== Includes =======================================================*/
#include "bms.h"
#include "can_cbs.h"
#include "can_helper.h"

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/
extern uint32_t CAN_TxMinimumMaximumValues(
    uint32_t id,
    uint8_t dlc,
    CAN_ENDIANNESS_e endianness,
    uint8_t *pCanData,
    uint8_t *pMuxId,
    const CAN_SHIM_s *const kpkCanShim) {
    /* pMuxId is not used here, therefore has to be NULL_PTR */
    FAS_ASSERT(pMuxId == NULL_PTR);

    FAS_ASSERT(id < CAN_MAX_11BIT_ID); /* Currently standard ID, 11 bit */
    FAS_ASSERT(dlc <= CAN_MAX_DLC);    /* Currently max 8 bytes in a CAN frame */
    FAS_ASSERT(pCanData != NULL_PTR);
    FAS_ASSERT(kpkCanShim != NULL_PTR);
    uint64_t message = 0;

    DATA_READ_DATA(kpkCanShim->pTableMinMax);

    int16_t packMaximumVoltage_mV        = INT16_MIN;
    int16_t packMinimumVoltage_mV        = INT16_MAX;
    int16_t packMaximumTemperature_ddegC = INT16_MIN;
    int16_t packMinimumTemperature_ddegC = INT16_MAX;

    if (0u == BMS_GetNumberOfConnectedStrings()) {
        /* Calculate min/max values of complete pack if all slice switches are open */
        for (uint8_t stringNumber = 0u; stringNumber < BS_NR_OF_STRINGS; stringNumber++) {
            if (kpkCanShim->pTableMinMax->maximumCellVoltage_mV[stringNumber] >= packMaximumVoltage_mV) {
                packMaximumVoltage_mV = kpkCanShim->pTableMinMax->maximumCellVoltage_mV[stringNumber];
            }
            if (kpkCanShim->pTableMinMax->minimumCellVoltage_mV[stringNumber] <= packMinimumVoltage_mV) {
                packMinimumVoltage_mV = kpkCanShim->pTableMinMax->minimumCellVoltage_mV[stringNumber];
            }
            if (kpkCanShim->pTableMinMax->maximumTemperature_ddegC[stringNumber] >= packMaximumTemperature_ddegC) {
                packMaximumTemperature_ddegC = kpkCanShim->pTableMinMax->maximumTemperature_ddegC[stringNumber];
            }
            if (kpkCanShim->pTableMinMax->minimumTemperature_ddegC[stringNumber] <= packMinimumTemperature_ddegC) {
                packMinimumTemperature_ddegC = kpkCanShim->pTableMinMax->minimumTemperature_ddegC[stringNumber];
            }
        }
    } else {
        /* Calculate min/max values of connected slices */
        for (uint8_t stringNumber = 0u; stringNumber < BS_NR_OF_STRINGS; stringNumber++) {
            if (true == BMS_IsStringClosed(stringNumber)) {
                if (kpkCanShim->pTableMinMax->maximumCellVoltage_mV[stringNumber] >= packMaximumVoltage_mV) {
                    packMaximumVoltage_mV = kpkCanShim->pTableMinMax->maximumCellVoltage_mV[stringNumber];
                }
                if (kpkCanShim->pTableMinMax->minimumCellVoltage_mV[stringNumber] <= packMinimumVoltage_mV) {
                    packMinimumVoltage_mV = kpkCanShim->pTableMinMax->minimumCellVoltage_mV[stringNumber];
                }
                if (kpkCanShim->pTableMinMax->maximumTemperature_ddegC[stringNumber] >= packMaximumTemperature_ddegC) {
                    packMaximumTemperature_ddegC = kpkCanShim->pTableMinMax->maximumTemperature_ddegC[stringNumber];
                }
                if (kpkCanShim->pTableMinMax->minimumTemperature_ddegC[stringNumber] <= packMinimumTemperature_ddegC) {
                    packMinimumTemperature_ddegC = kpkCanShim->pTableMinMax->minimumTemperature_ddegC[stringNumber];
                }
            }
        }
    }

    /* AXIVION Disable Style Generic-NoMagicNumbers: Signal data defined in .dbc file. */
    /* Minimum cell voltage */
    float signalData = (float)packMinimumVoltage_mV;
    float offset     = 0.0f;
    float factor     = 1.0f;
    signalData       = (signalData + offset) * factor;
    uint64_t data    = (int64_t)signalData;
    /* set data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(&message, 10u, 13u, data, endianness);

    /* Maximum cell voltage */
    signalData = (float)packMaximumVoltage_mV;
    offset     = 0.0f;
    factor     = 1.0f;
    signalData = (signalData + offset) * factor;
    data       = (int64_t)signalData;
    /* set data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(&message, 7u, 13u, data, endianness);

    /* Minimum cell temperature */
    signalData = (float)packMinimumTemperature_ddegC;
    offset     = 0.0f;
    factor     = 0.1f; /* convert ddegC to degC */
    signalData = (signalData + offset) * factor;
    data       = (int64_t)signalData;
    /* set data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(&message, 63u, 8u, data, endianness);

    /* Maximum cell temperature */
    signalData = (float)packMaximumTemperature_ddegC;
    offset     = 0.0f;
    factor     = 0.1f; /* convert ddegC to degC */
    signalData = (signalData + offset) * factor;
    data       = (int64_t)signalData;
    /* set data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(&message, 55u, 8u, data, endianness);
    /* AXIVION Enable Style Generic-NoMagicNumbers: */

    /* now copy data in the buffer that will be used to send data */
    CAN_TxSetCanDataWithMessageData(message, pCanData, endianness);

    return 0;
}

extern uint32_t CAN_TxStringMinimumMaximumValues(
    uint32_t id,
    uint8_t dlc,
    CAN_ENDIANNESS_e endianness,
    uint8_t *pCanData,
    uint8_t *pMuxId,
    const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(id < CAN_MAX_11BIT_ID); /* Currently standard ID, 11 bit */
    FAS_ASSERT(dlc <= CAN_MAX_DLC);    /* Currently max 8 bytes in a CAN frame */
    FAS_ASSERT(pCanData != NULL_PTR);
    FAS_ASSERT(pMuxId != NULL_PTR);
    FAS_ASSERT(*pMuxId < BS_NR_OF_STRINGS);
    FAS_ASSERT(kpkCanShim != NULL_PTR);
    uint64_t message = 0u;

    /** Database entry with minimum and maximum values does not need to be read
     *  within this callback as it is already read by function
     *  #CAN_TxMinimumMaximumValues */
    const uint8_t stringNumber = *pMuxId;

    /* AXIVION Disable Style Generic-NoMagicNumbers: Signal data defined in .dbc file. */
    /* Minimum cell voltage */
    float signalData = (float)kpkCanShim->pTableMinMax->minimumCellVoltage_mV[stringNumber];
    float offset     = 0.0f;
    float factor     = 1.0f;
    signalData       = (signalData + offset) * factor;
    uint64_t data    = (int64_t)signalData;
    /* set data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(&message, 18u, 13u, data, endianness);

    /* Maximum cell voltage */
    signalData = (float)kpkCanShim->pTableMinMax->maximumCellVoltage_mV[stringNumber];
    offset     = 0.0f;
    factor     = 1.0f;
    signalData = (signalData + offset) * factor;
    data       = (int64_t)signalData;
    /* set data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(&message, 15u, 13u, data, endianness);

    /* Minimum cell temperature */
    signalData = (float)kpkCanShim->pTableMinMax->minimumTemperature_ddegC[stringNumber];
    offset     = 0.0f;
    factor     = 0.2f; /* convert ddegC to 0.5degC */
    signalData = (signalData + offset) * factor;
    data       = (int64_t)signalData;
    /* set data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(&message, 44u, 9u, data, endianness);

    /* Maximum cell temperature */
    signalData = (float)kpkCanShim->pTableMinMax->maximumTemperature_ddegC[stringNumber];
    offset     = 0.0f;
    factor     = 0.2f; /* convert ddegC to 0.5degC */
    signalData = (signalData + offset) * factor;
    data       = (int64_t)signalData;
    /* set data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(&message, 37, 9u, data, endianness);
    /* AXIVION Enable Style Generic-NoMagicNumbers: */

    /* now copy data in the buffer that will be use to send data */
    CAN_TxSetCanDataWithMessageData(message, pCanData, endianness);

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
