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
 * @file    can_cbs_tx_state.c
 * @author  foxBMS Team
 * @date    2021-07-21 (date of creation)
 * @updated 2021-07-21 (date of last update)
 * @ingroup DRIVER
 * @prefix  CAN
 *
 * @brief   CAN driver Tx callback implementation
 * @details CAN Tx callback for state messages
 */

/*========== Includes =======================================================*/
#include "bms.h"
#include "can_cbs.h"
#include "can_helper.h"
#include "diag.h"

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/
extern uint32_t CAN_TxState(
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

    DATA_READ_DATA(kpkCanShim->pTableErrorState, kpkCanShim->pTableInsulation, kpkCanShim->pTableMsl);

    /* AXIVION Disable Style Generic-NoMagicNumbers: Signal data defined in .dbc file. */
    /* State */
    uint64_t data = (uint64_t)BMS_GetState();
    /* set data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(&message, 3u, 4u, data, endianness);

    /* Connected strings */
    data = (uint64_t)BMS_GetNumberOfConnectedStrings();
    /* set data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(&message, 7u, 4u, data, endianness);

    /* General error - implement now */
    if (true == DIAG_IsAnyFatalErrorSet()) {
        data = 1u;
    } else {
        data = 0u;
    }
    CAN_TxSetMessageDataWithSignalData(&message, 11u, 1u, data, endianness);

    /* General warning: TODO */

    /* Emergency shutoff */
    if (true == BMS_IsTransitionToErrorStateActive()) {
        data = 1u;
    } else {
        data = 0u;
    }
    CAN_TxSetMessageDataWithSignalData(&message, 10u, 1u, data, endianness);

    /* Number of deactivated strings: TODO */

    /* Error: insulation */
    data = kpkCanShim->pTableErrorState->insulationError;
    CAN_TxSetMessageDataWithSignalData(&message, 23u, 1u, data, endianness);

    /* Insulation resistance */
    float signalData = (float)kpkCanShim->pTableInsulation->insulationResistance_kOhm;
    signalData       = signalData * 0.1f; /* convert kOhm to 10kOhm */
    data             = (uint64_t)signalData;
    CAN_TxSetMessageDataWithSignalData(&message, 63u, 8u, data, endianness);

    /* Charging complete: TODO */

    /* Insulation monitoring active */
    data = kpkCanShim->pTableInsulation->insulationMeasurements;
    CAN_TxSetMessageDataWithSignalData(&message, 13u, 1u, data, endianness);

    /* Heater state: TODO */
    /* Cooling state: TODO */

    /* Error: Precharge voltage: TODO */
    /* Error: Precharge current: TODO */

    /* Error: MCU die temperature */
    data = kpkCanShim->pTableErrorState->mcuDieTemperature;
    CAN_TxSetMessageDataWithSignalData(&message, 18u, 1u, data, endianness);

    /* Error: master overtemperature: TODO */
    /* Error: master undertemperature: TODO */

    /* Error: interlock */
    data = kpkCanShim->pTableErrorState->interlock;
    CAN_TxSetMessageDataWithSignalData(&message, 22u, 1u, data, endianness);

    /* Main fuse state: TODO */

    /* Error: Can timing */
    data = kpkCanShim->pTableErrorState->canTiming;
    CAN_TxSetMessageDataWithSignalData(&message, 24u, 1u, data, endianness);

    /* Error: Overcurrent pack charge */
    data = kpkCanShim->pTableMsl->packChargeOvercurrent;
    CAN_TxSetMessageDataWithSignalData(&message, 25u, 1u, data, endianness);

    /* Error: Overcurrent pack discharge */
    data = kpkCanShim->pTableMsl->packDischargeOvercurrent;
    CAN_TxSetMessageDataWithSignalData(&message, 26u, 1u, data, endianness);
    /* AXIVION Enable Style Generic-NoMagicNumbers: */

    /* now copy data in the buffer that will be use to send data */
    CAN_TxSetCanDataWithMessageData(message, pCanData, endianness);

    return 0;
}
extern uint32_t CAN_TxStringState(
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
    uint64_t data    = 0;

    const uint8_t stringNumber = *pMuxId;

    /* First signal to transmit cell voltages: get database values */
    if (stringNumber == 0u) {
        /* Do not read pTableMsl and pTableErrorState as they already are read
         * with a higher frequency from CAN_TxState callback */
        DATA_READ_DATA(kpkCanShim->pTableRsl, kpkCanShim->pTableMol);
    }

    /* AXIVION Disable Style Generic-NoMagicNumbers: Signal data defined in .dbc file. */
    /* mux value */
    data = (uint64_t)stringNumber;
    /* set data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(&message, 3u, 4u, data, endianness);

    /* String connected */
    if (true == BMS_IsStringClosed(stringNumber)) {
        data = 1u;
    } else {
        data = 0u;
    }
    CAN_TxSetMessageDataWithSignalData(&message, 4u, 1u, data, endianness);

    /* Balancing active: TODO */

    /* String fuse blown */
    if ((1u == kpkCanShim->pTableErrorState->fuseStateCharge[stringNumber]) ||
        (1u == kpkCanShim->pTableErrorState->fuseStateNormal[stringNumber])) {
        data = 1u;
    } else {
        data = 0u;
    }
    CAN_TxSetMessageDataWithSignalData(&message, 6u, 1u, data, endianness);

    /* Error: Deep-discharge */
    data = kpkCanShim->pTableErrorState->deepDischargeDetected[stringNumber];
    CAN_TxSetMessageDataWithSignalData(&message, 7u, 1u, data, endianness);

    /* Error: Overtemperature charge */
    data = kpkCanShim->pTableMsl->overtemperatureCharge[stringNumber];
    CAN_TxSetMessageDataWithSignalData(&message, 8u, 1u, data, endianness);

    /* Error: Undertemperature charge */
    data = kpkCanShim->pTableMsl->undertemperatureCharge[stringNumber];
    CAN_TxSetMessageDataWithSignalData(&message, 9u, 1u, data, endianness);

    /* Error: Overtemperature discharge */
    data = kpkCanShim->pTableMsl->overtemperatureDischarge[stringNumber];
    CAN_TxSetMessageDataWithSignalData(&message, 10u, 1u, data, endianness);

    /* Error: Undertemperature discharge */
    data = kpkCanShim->pTableMsl->undertemperatureDischarge[stringNumber];
    CAN_TxSetMessageDataWithSignalData(&message, 11u, 1u, data, endianness);

    /* Error: Overcurrent charge */
    if ((1u == kpkCanShim->pTableMsl->cellChargeOvercurrent[stringNumber]) ||
        (1u == kpkCanShim->pTableMsl->stringChargeOvercurrent[stringNumber])) {
        data = 1u;
    } else {
        data = 0u;
    }
    CAN_TxSetMessageDataWithSignalData(&message, 12u, 1u, data, endianness);

    /* Error: Overcurrent discharge */
    if ((1u == kpkCanShim->pTableMsl->cellDischargeOvercurrent[stringNumber]) ||
        (1u == kpkCanShim->pTableMsl->stringDischargeOvercurrent[stringNumber])) {
        data = 1u;
    } else {
        data = 0u;
    }
    CAN_TxSetMessageDataWithSignalData(&message, 13u, 1u, data, endianness);

    /* Error: Overvoltage */
    data = kpkCanShim->pTableMsl->overVoltage[stringNumber];
    CAN_TxSetMessageDataWithSignalData(&message, 14u, 1u, data, endianness);

    /* Error: Undervoltage */
    data = kpkCanShim->pTableMsl->underVoltage[stringNumber];
    CAN_TxSetMessageDataWithSignalData(&message, 15u, 1u, data, endianness);

    /* Info: Overtemperature charge - MOL */
    data = kpkCanShim->pTableMol->overtemperatureCharge[stringNumber];
    CAN_TxSetMessageDataWithSignalData(&message, 16u, 1u, data, endianness);

    /* Info: Undertemperature charge - MOL */
    data = kpkCanShim->pTableMol->undertemperatureCharge[stringNumber];
    CAN_TxSetMessageDataWithSignalData(&message, 17u, 1u, data, endianness);

    /* Info: Overtemperature discharge - MOL */
    data = kpkCanShim->pTableMol->overtemperatureDischarge[stringNumber];
    CAN_TxSetMessageDataWithSignalData(&message, 18u, 1u, data, endianness);

    /* Info: Undertemperature discharge - MOL */
    data = kpkCanShim->pTableMol->undertemperatureCharge[stringNumber];
    CAN_TxSetMessageDataWithSignalData(&message, 19u, 1u, data, endianness);

    /* Info: Overcurrent charge - MOL */
    if ((1u == kpkCanShim->pTableMol->cellChargeOvercurrent[stringNumber]) ||
        (1u == kpkCanShim->pTableMol->stringChargeOvercurrent[stringNumber])) {
        data = 1u;
    } else {
        data = 0u;
    }
    CAN_TxSetMessageDataWithSignalData(&message, 20u, 1u, data, endianness);

    /* Info: Overcurrent discharge - MOL */
    if ((1u == kpkCanShim->pTableMol->cellDischargeOvercurrent[stringNumber]) ||
        (1u == kpkCanShim->pTableMol->stringDischargeOvercurrent[stringNumber])) {
        data = 1u;
    } else {
        data = 0u;
    }
    CAN_TxSetMessageDataWithSignalData(&message, 21u, 1u, data, endianness);

    /* Info: Overvoltage - MOL */
    data = kpkCanShim->pTableMol->overVoltage[stringNumber];
    CAN_TxSetMessageDataWithSignalData(&message, 22u, 1u, data, endianness);

    /* Info: Undervoltage - MOL */
    data = kpkCanShim->pTableMol->underVoltage[stringNumber];
    CAN_TxSetMessageDataWithSignalData(&message, 23u, 1u, data, endianness);

    /* Warning: Overtemperature charge - RSL */
    data = kpkCanShim->pTableRsl->overtemperatureCharge[stringNumber];
    CAN_TxSetMessageDataWithSignalData(&message, 24u, 1u, data, endianness);

    /* Warning: Undertemperature charge - RSL */
    data = kpkCanShim->pTableRsl->undertemperatureCharge[stringNumber];
    CAN_TxSetMessageDataWithSignalData(&message, 25u, 1u, data, endianness);

    /* Warning: Overtemperature discharge - RSL */
    data = kpkCanShim->pTableRsl->overtemperatureDischarge[stringNumber];
    CAN_TxSetMessageDataWithSignalData(&message, 26u, 1u, data, endianness);

    /* Warning: Undertemperature discharge - RSL */
    data = kpkCanShim->pTableRsl->undertemperatureDischarge[stringNumber];
    CAN_TxSetMessageDataWithSignalData(&message, 27u, 1u, data, endianness);

    /* Warning: Overcurrent charge - RSL */
    if ((1u == kpkCanShim->pTableRsl->cellChargeOvercurrent[stringNumber]) ||
        (1u == kpkCanShim->pTableRsl->stringChargeOvercurrent[stringNumber])) {
        data = 1u;
    } else {
        data = 0u;
    }
    CAN_TxSetMessageDataWithSignalData(&message, 28u, 1u, data, endianness);

    /* Warning: Overcurrent discharge - RSL */
    if ((1u == kpkCanShim->pTableMol->cellDischargeOvercurrent[stringNumber]) ||
        (1u == kpkCanShim->pTableMol->stringDischargeOvercurrent[stringNumber])) {
        data = 1u;
    } else {
        data = 0u;
    }
    CAN_TxSetMessageDataWithSignalData(&message, 29u, 1u, data, endianness);

    /* Warning: Overvoltage - RSL */
    data = kpkCanShim->pTableRsl->overVoltage[stringNumber];
    CAN_TxSetMessageDataWithSignalData(&message, 30u, 1u, data, endianness);

    /* Warning: Undervoltage - RSL */
    data = kpkCanShim->pTableRsl->underVoltage[stringNumber];
    CAN_TxSetMessageDataWithSignalData(&message, 31u, 1u, data, endianness);

    /* Error: Positive string contactor: TODO */
    /* Error: Negative string contactor: TODO */
    /* Error: Slave hardware: TODO */

    /* Error: Daisy-chain base: communication */
    data = kpkCanShim->pTableErrorState->spiError[stringNumber];
    CAN_TxSetMessageDataWithSignalData(&message, 35u, 1u, data, endianness);

    /* Error: Daisy-chain redundancy: communication: TODO */
    /* Error: Daisy-chain base: CRC */
    data = kpkCanShim->pTableErrorState->crcError[stringNumber];
    CAN_TxSetMessageDataWithSignalData(&message, 37u, 1u, data, endianness);

    /* Error: Daisy-chain redundancy: CRC: TODO */
    /* Error: Daisy-chain base: Voltage out of operating range */
    data = kpkCanShim->pTableErrorState->afeCellvoltageError[stringNumber];
    CAN_TxSetMessageDataWithSignalData(&message, 39u, 1u, data, endianness);

    /* Error: Daisy-chain redundancy: Voltage out of operating range: TODO */
    /* Error: Daisy-chain base: Temperature out of operating range */
    data = kpkCanShim->pTableErrorState->afeCellTemperatureError[stringNumber];
    CAN_TxSetMessageDataWithSignalData(&message, 41u, 1u, data, endianness);

    /* Error: Daisy-chain redundancy: Voltage out of operating range: TODO */

    /* Error: current measurement */
    if ((1u == kpkCanShim->pTableErrorState->currentMeasurementError[stringNumber]) ||
        (1u == kpkCanShim->pTableErrorState->currentMeasurementTimeout[stringNumber])) {
        data = 1u;
    } else {
        data = 0u;
    }
    CAN_TxSetMessageDataWithSignalData(&message, 43u, 1u, data, endianness);

    /* Error: Coulomb counting measurement */
    data = kpkCanShim->pTableErrorState->canTimingCc[stringNumber];
    CAN_TxSetMessageDataWithSignalData(&message, 44u, 1u, data, endianness);

    /* Error: Energy counting measurement */
    data = kpkCanShim->pTableErrorState->canTimingEc[stringNumber];
    CAN_TxSetMessageDataWithSignalData(&message, 45u, 1u, data, endianness);

    /* Error: Current sensor V1 measurement */
    data = kpkCanShim->pTableErrorState->currentSensorTimeoutV1[stringNumber];
    CAN_TxSetMessageDataWithSignalData(&message, 46u, 1u, data, endianness);

    /* Error: Current sensor V2 measurement: TODO */
    /* Error: Current sensor V3 measurement */
    data = kpkCanShim->pTableErrorState->currentSensorTimeoutV3[stringNumber];
    CAN_TxSetMessageDataWithSignalData(&message, 48u, 1u, data, endianness);

    /* Error: Open wire */
    data = kpkCanShim->pTableErrorState->open_wire[stringNumber];
    CAN_TxSetMessageDataWithSignalData(&message, 49u, 1u, data, endianness);

    /* Error: Plausibility: Cell temperature */
    data = kpkCanShim->pTableErrorState->plausibilityCheckCelltemperature[stringNumber];
    CAN_TxSetMessageDataWithSignalData(&message, 51u, 1u, data, endianness);

    /* Error: Plausibility: Cell voltage */
    data = kpkCanShim->pTableErrorState->plausibilityCheckCellvoltage[stringNumber];
    CAN_TxSetMessageDataWithSignalData(&message, 52u, 1u, data, endianness);

    /* Error: Plausibility: String voltage */
    data = kpkCanShim->pTableErrorState->plausibilityCheckPackvoltage[stringNumber];
    CAN_TxSetMessageDataWithSignalData(&message, 53u, 1u, data, endianness);

    /* Error: Plausibility: Cell temperature spread */
    data = kpkCanShim->pTableErrorState->plausibilityCheckCelltemperatureSpread[stringNumber];
    CAN_TxSetMessageDataWithSignalData(&message, 54u, 1u, data, endianness);

    /* Error: Plausibility: Cell voltage spread */
    data = kpkCanShim->pTableErrorState->plausibilityCheckCellvoltageSpread[stringNumber];
    CAN_TxSetMessageDataWithSignalData(&message, 55u, 1u, data, endianness);
    /* AXIVION Enable Style Generic-NoMagicNumbers: */

    /* Open wire number: TODO */

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
