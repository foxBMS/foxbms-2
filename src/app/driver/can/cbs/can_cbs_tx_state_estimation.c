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
 * @file    can_cbs_tx_state_estimation.c
 * @author  foxBMS Team
 * @date    2021-07-21 (date of creation)
 * @updated 2021-07-21 (date of last update)
 * @ingroup DRIVER
 * @prefix  CAN
 *
 * @brief   CAN driver Tx callback implementation
 * @details CAN Tx callback for state estimation messages
 */

/*========== Includes =======================================================*/
#include "bms.h"
#include "can_cbs.h"
#include "can_helper.h"
#include "foxmath.h"

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/
extern uint32_t CAN_TxStateEstimation(
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

    float minimumStringSoc_perc     = FLT_MAX;
    float maximumStringSoc_perc     = FLT_MIN;
    float minimumStringSoe_perc     = FLT_MAX;
    float maximumStringSoe_perc     = FLT_MIN;
    uint32_t minimumStringEnergy_Wh = UINT32_MAX;

    DATA_READ_DATA(kpkCanShim->pTableSox);

    /* Check current direction  */
    if (BMS_GetBatterySystemState() == BMS_CHARGING) {
        /* If battery system is charging use maximum values */
        for (uint8_t stringNumber = 0u; stringNumber < BS_NR_OF_STRINGS; stringNumber++) {
            if (true == BMS_IsStringClosed(stringNumber)) {
                if (maximumStringSoc_perc < kpkCanShim->pTableSox->maximumSoc_perc[stringNumber]) {
                    maximumStringSoc_perc = kpkCanShim->pTableSox->maximumSoc_perc[stringNumber];
                }
                if (maximumStringSoe_perc < kpkCanShim->pTableSox->maximumSoe_perc[stringNumber]) {
                    maximumStringSoe_perc = kpkCanShim->pTableSox->maximumSoe_perc[stringNumber];
                }
                if (minimumStringEnergy_Wh > kpkCanShim->pTableSox->minimumSoe_Wh[stringNumber]) {
                    minimumStringEnergy_Wh = kpkCanShim->pTableSox->minimumSoe_Wh[stringNumber];
                }
            }
        }
    } else {
        /* If battery system is discharging or at rest use minimum values */
        for (uint8_t stringNumber = 0u; stringNumber < BS_NR_OF_STRINGS; stringNumber++) {
            if (true == BMS_IsStringClosed(stringNumber)) {
                if (minimumStringSoc_perc > kpkCanShim->pTableSox->minimumSoc_perc[stringNumber]) {
                    minimumStringSoc_perc = kpkCanShim->pTableSox->minimumSoc_perc[stringNumber];
                }
                if (minimumStringSoe_perc > kpkCanShim->pTableSox->minimumSoe_perc[stringNumber]) {
                    minimumStringSoe_perc = kpkCanShim->pTableSox->minimumSoe_perc[stringNumber];
                }
                if (minimumStringEnergy_Wh > kpkCanShim->pTableSox->minimumSoe_Wh[stringNumber]) {
                    minimumStringEnergy_Wh = kpkCanShim->pTableSox->minimumSoe_Wh[stringNumber];
                }
            }
        }
    }

    float packSoc_perc         = 0.0f;
    float packSoe_perc         = 0.0f;
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
    float signalData = packSoc_perc;
    float offset     = 0.0f;
    float factor     = 100.0f; /* convert from perc to 0.01perc */
    signalData       = (signalData + offset) * factor;
    uint64_t data    = (int64_t)signalData;
    /* set data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(&message, 7u, 14u, data, endianness);

    /* SOE */
    signalData = packSoe_perc;
    offset     = 0.0f;
    factor     = 100.0f; /* convert from perc to 0.01perc */
    signalData = (signalData + offset) * factor;
    data       = (int64_t)signalData;
    /* set data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(&message, 9u, 14u, data, endianness);

    /* Pack energy */
    signalData = packEnergyLeft_Wh;
    offset     = 0.0f;
    factor     = 0.1f; /* convert from Wh to 10Wh */
    signalData = (signalData + offset) * factor;
    data       = (int64_t)signalData;
    /* set data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(&message, 47u, 24u, data, endianness);

    /* SOH */
    signalData = 100.0f; /* TODO */
    offset     = 0.0f;
    factor     = 1.0f / 0.025f; /* convert from perc to 0.025% */
    signalData = (signalData + offset) * factor;
    data       = (int64_t)signalData;
    /* set data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(&message, 27u, 12u, data, endianness);

    /* now copy data in the buffer that will be used to send data */
    CAN_TxSetCanDataWithMessageData(message, pCanData, endianness);

    return 0;
}

extern uint32_t CAN_TxStringStateEstimation(
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
    uint64_t message = 0;

    /** Database entry with state estimation values does not need to be read
     *  within this callback as it is already read by function
     *  #CAN_TxStateEstimation */
    const uint8_t stringNumber = *pMuxId;

    /* set multiplexer in CAN frame */
    /* AXIVION Disable Style Generic-NoMagicNumbers: Signal data defined in .dbc file. */
    uint64_t data = (uint64_t)stringNumber;
    CAN_TxSetMessageDataWithSignalData(&message, 7u, 4u, data, endianness);

    /* Minimum SOC */
    float signalData = kpkCanShim->pTableSox->minimumSoc_perc[stringNumber];
    float offset     = 0.0f;
    float factor     = 4.0f; /* convert from perc to 0.25perc */
    signalData       = (signalData + offset) * factor;
    data             = (int64_t)signalData;
    /* set data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(&message, 3u, 9u, data, endianness);

    /* Average SOC */
    signalData = kpkCanShim->pTableSox->averageSoc_perc[stringNumber];
    offset     = 0.0f;
    factor     = 4.0f; /* convert from perc to 0.25perc */
    signalData = (signalData + offset) * factor;
    data       = (int64_t)signalData;
    /* set data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(&message, 10u, 9u, data, endianness);

    /* Maximum SOC */
    signalData = kpkCanShim->pTableSox->maximumSoc_perc[stringNumber];
    offset     = 0.0f;
    factor     = 4.0f; /* convert from perc to 0.25perc */
    signalData = (signalData + offset) * factor;
    data       = (int64_t)signalData;
    /* set data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(&message, 17u, 9u, data, endianness);

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
    CAN_TxSetMessageDataWithSignalData(&message, 24u, 9u, data, endianness);

    /* SOH */
    signalData = 100.0f;
    offset     = 0.0f;
    factor     = 4.0f; /* convert from perc to 0.25perc */
    signalData = (signalData + offset) * factor;
    data       = (int64_t)signalData;
    /* set data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(&message, 47u, 9u, data, endianness);

    /* String energy */
    signalData = kpkCanShim->pTableSox->minimumSoe_Wh[stringNumber];
    offset     = 0.0f;
    factor     = 0.1f; /* convert from Wh to 10Wh */
    signalData = (signalData + offset) * factor;
    data       = (int64_t)signalData;
    /* set data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(&message, 54u, 15u, data, endianness);
    /* AXIVION Enable Style Generic-NoMagicNumbers: */

    /* now copy data in the buffer that will be used to send data */
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
