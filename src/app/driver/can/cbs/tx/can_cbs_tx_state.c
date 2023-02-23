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
 * @file    can_cbs_tx_state.c
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
#include "bms.h"
#include "can_cbs_tx.h"
#include "can_cfg_tx-message-definitions.h"
#include "can_helper.h"
#include "diag.h"
#include "sys_mon.h"

#include <math.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/
/** get a boolean for if any timing error (current or recorded) occurred */
static bool CANTX_AnySysMonTimingIssueDetected(const CAN_SHIM_s *const kpkCanShim);

/*========== Static Function Implementations ================================*/
static bool CANTX_AnySysMonTimingIssueDetected(const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(kpkCanShim != NULL_PTR);
    SYSM_TIMING_VIOLATION_RESPONSE_s recordedTimingViolations = {0};
    SYSM_GetRecordedTimingViolations(&recordedTimingViolations);

    const bool anyTimingViolation =
        (recordedTimingViolations.recordedViolationAny ||
         kpkCanShim->pTableErrorState->taskEngineTimingViolationError ||
         kpkCanShim->pTableErrorState->task1msTimingViolationError ||
         kpkCanShim->pTableErrorState->task10msTimingViolationError ||
         kpkCanShim->pTableErrorState->task100msTimingViolationError ||
         kpkCanShim->pTableErrorState->task100msAlgoTimingViolationError);

    return anyTimingViolation;
}

/*========== Extern Function Implementations ================================*/
extern uint32_t CANTX_BmsState(
    CAN_MESSAGE_PROPERTIES_s message,
    uint8_t *pCanData,
    uint8_t *pMuxId,
    const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(message.id == CANTX_BMS_STATE_ID);
    FAS_ASSERT(message.dlc == CAN_FOXBMS_MESSAGES_DEFAULT_DLC);
    FAS_ASSERT(pCanData != NULL_PTR);
    FAS_ASSERT(pMuxId == NULL_PTR); /* pMuxId is not used here, therefore has to be NULL_PTR */
    FAS_ASSERT(kpkCanShim != NULL_PTR);
    uint64_t messageData = 0u;

    DATA_READ_DATA(kpkCanShim->pTableErrorState, kpkCanShim->pTableInsulation, kpkCanShim->pTableMsl);

    /* AXIVION Disable Style Generic-NoMagicNumbers: Signal data defined in .dbc file. */
    /* State */
    uint64_t data = (uint64_t)BMS_GetState();
    /* set data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(&messageData, 3u, 4u, data, message.endianness);

    /* Connected strings */
    data = (uint64_t)BMS_GetNumberOfConnectedStrings();
    /* set data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(&messageData, 7u, 4u, data, message.endianness);

    /* General warning: TODO */

    /* General error - implement now */
    data = CAN_ConvertBooleanToInteger(DIAG_IsAnyFatalErrorSet());
    CAN_TxSetMessageDataWithSignalData(&messageData, 10u, 1u, data, message.endianness);

    /* Emergency shutoff */
    data = CAN_ConvertBooleanToInteger(BMS_IsTransitionToErrorStateActive());
    CAN_TxSetMessageDataWithSignalData(&messageData, 11u, 1u, data, message.endianness);

    /* Number of deactivated strings: TODO */

    /* sys mon error */
    data = CAN_ConvertBooleanToInteger(CANTX_AnySysMonTimingIssueDetected(kpkCanShim));
    CAN_TxSetMessageDataWithSignalData(&messageData, 12u, 1u, data, message.endianness);

    /* Insulation monitoring active */
    data = CAN_ConvertBooleanToInteger(kpkCanShim->pTableInsulation->isImdRunning);
    CAN_TxSetMessageDataWithSignalData(&messageData, 13u, 1u, data, message.endianness);

    /* Error: insulation */
    data = CAN_ConvertBooleanToInteger(kpkCanShim->pTableErrorState->criticalLowInsulationResistanceError);
    CAN_TxSetMessageDataWithSignalData(&messageData, 23u, 1u, data, message.endianness);

    /* Insulation resistance */
    float_t signalData = (float_t)kpkCanShim->pTableInsulation->insulationResistance_kOhm;
    signalData         = signalData * 0.1f; /* convert kOhm to 10kOhm */
    data               = (uint64_t)signalData;
    CAN_TxSetMessageDataWithSignalData(&messageData, 63u, 8u, data, message.endianness);

    /* Charging complete: TODO */

    /* Heater state: TODO */
    /* Cooling state: TODO */

    /* Error: Precharge voltage */
    data = 0u; /* No precharge error detected */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        if (kpkCanShim->pTableErrorState->prechargeAbortedDueToVoltage[s] == true) {
            data = 1u;
        }
    }
    CAN_TxSetMessageDataWithSignalData(&messageData, 16u, 1u, data, message.endianness);

    /* Error: Precharge current */
    data = 0u; /* No precharge error detected */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        if (kpkCanShim->pTableErrorState->prechargeAbortedDueToCurrent[s] == true) {
            data = 1u;
        }
    }
    CAN_TxSetMessageDataWithSignalData(&messageData, 17u, 1u, data, message.endianness);

    /* Error: MCU die temperature */
    data = CAN_ConvertBooleanToInteger(kpkCanShim->pTableErrorState->mcuDieTemperatureViolationError);
    CAN_TxSetMessageDataWithSignalData(&messageData, 18u, 1u, data, message.endianness);

    /* Error: master overtemperature: TODO */
    /* Error: master undertemperature: TODO */

    /* Main fuse state */
    data = CAN_ConvertBooleanToInteger(kpkCanShim->pTableErrorState->mainFuseError);
    CAN_TxSetMessageDataWithSignalData(&messageData, 21u, 1u, data, message.endianness);

    /* Error: interlock */
    data = CAN_ConvertBooleanToInteger(kpkCanShim->pTableErrorState->interlockOpenedError);
    CAN_TxSetMessageDataWithSignalData(&messageData, 22u, 1u, data, message.endianness);

    /* Error: Can timing */
    data = kpkCanShim->pTableErrorState->stateRequestTimingViolationError;
    CAN_TxSetMessageDataWithSignalData(&messageData, 24u, 1u, data, message.endianness);

    /* Error: Overcurrent pack charge */
    data = kpkCanShim->pTableMsl->packChargeOvercurrent;
    CAN_TxSetMessageDataWithSignalData(&messageData, 25u, 1u, data, message.endianness);

    /* Error: Overcurrent pack discharge */
    data = kpkCanShim->pTableMsl->packDischargeOvercurrent;
    CAN_TxSetMessageDataWithSignalData(&messageData, 26u, 1u, data, message.endianness);

    /* Error: Alert flag */
    data = CAN_ConvertBooleanToInteger(kpkCanShim->pTableErrorState->alertFlagSetError);
    CAN_TxSetMessageDataWithSignalData(&messageData, 27u, 1u, data, message.endianness);

    /* Error: NVRAM CRC */
    data = CAN_ConvertBooleanToInteger(kpkCanShim->pTableErrorState->framReadCrcError);
    CAN_TxSetMessageDataWithSignalData(&messageData, 28u, 1u, data, message.endianness);
    /* AXIVION Enable Style Generic-NoMagicNumbers: */

    /* now copy data in the buffer that will be use to send data */
    CAN_TxSetCanDataWithMessageData(messageData, pCanData, message.endianness);

    return 0;
}

extern uint32_t CANTX_BmsStateDetails(
    CAN_MESSAGE_PROPERTIES_s message,
    uint8_t *pCanData,
    uint8_t *pMuxId,
    const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(message.id == CANTX_BMS_STATE_DETAILS_ID);
    FAS_ASSERT(message.idType == CANTX_BMS_STATE_DETAILS_ID_TYPE);
    FAS_ASSERT(message.dlc == CAN_FOXBMS_MESSAGES_DEFAULT_DLC);
    FAS_ASSERT(pCanData != NULL_PTR);
    FAS_ASSERT(pMuxId == NULL_PTR); /* pMuxId is not used here, therefore has to be NULL_PTR */
    FAS_ASSERT(kpkCanShim != NULL_PTR);
    uint64_t messageData = 0u;

    DATA_READ_DATA(kpkCanShim->pTableErrorState);
    SYSM_TIMING_VIOLATION_RESPONSE_s recordedTimingViolations = {0};
    SYSM_GetRecordedTimingViolations(&recordedTimingViolations);

    /* AXIVION Disable Style Generic-NoMagicNumbers: Signal data defined in .dbc file. */
    /* current violation engine */
    uint64_t data = CAN_ConvertBooleanToInteger(kpkCanShim->pTableErrorState->taskEngineTimingViolationError);
    CAN_TxSetMessageDataWithSignalData(&messageData, 0u, 1u, data, message.endianness);
    /* current violation 1ms */
    data = CAN_ConvertBooleanToInteger(kpkCanShim->pTableErrorState->task1msTimingViolationError);
    CAN_TxSetMessageDataWithSignalData(&messageData, 1u, 1u, data, message.endianness);
    /* current violation 10ms */
    data = CAN_ConvertBooleanToInteger(kpkCanShim->pTableErrorState->task10msTimingViolationError);
    CAN_TxSetMessageDataWithSignalData(&messageData, 2u, 1u, data, message.endianness);
    /* current violation 100ms */
    data = CAN_ConvertBooleanToInteger(kpkCanShim->pTableErrorState->task100msTimingViolationError);
    CAN_TxSetMessageDataWithSignalData(&messageData, 3u, 1u, data, message.endianness);
    /* current violation 100ms algorithm */
    data = CAN_ConvertBooleanToInteger(kpkCanShim->pTableErrorState->task100msAlgoTimingViolationError);
    CAN_TxSetMessageDataWithSignalData(&messageData, 4u, 1u, data, message.endianness);

    /* recorded violation engine */
    data = CAN_ConvertBooleanToInteger(recordedTimingViolations.recordedViolationEngine);
    CAN_TxSetMessageDataWithSignalData(&messageData, 8u, 1u, data, message.endianness);
    /* recorded violation 1ms */
    data = CAN_ConvertBooleanToInteger(recordedTimingViolations.recordedViolation1ms);
    CAN_TxSetMessageDataWithSignalData(&messageData, 9u, 1u, data, message.endianness);
    /* recorded violation 10ms */
    data = CAN_ConvertBooleanToInteger(recordedTimingViolations.recordedViolation10ms);
    CAN_TxSetMessageDataWithSignalData(&messageData, 10u, 1u, data, message.endianness);
    /* recorded violation 100ms */
    data = CAN_ConvertBooleanToInteger(recordedTimingViolations.recordedViolation100ms);
    CAN_TxSetMessageDataWithSignalData(&messageData, 11u, 1u, data, message.endianness);
    /* recorded violation 100ms algorithm */
    data = CAN_ConvertBooleanToInteger(recordedTimingViolations.recordedViolation100msAlgo);
    CAN_TxSetMessageDataWithSignalData(&messageData, 12u, 1u, data, message.endianness);
    /* AXIVION Enable Style Generic-NoMagicNumbers: */

    /* now copy data in the buffer that will be use to send data */
    CAN_TxSetCanDataWithMessageData(messageData, pCanData, message.endianness);

    return 0;
}

extern uint32_t CANTX_StringState(
    CAN_MESSAGE_PROPERTIES_s message,
    uint8_t *pCanData,
    uint8_t *pMuxId,
    const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(message.id == CANTX_STRING_STATE_ID);
    FAS_ASSERT(message.idType == CANTX_STRING_STATE_ID_TYPE);
    FAS_ASSERT(message.dlc == CAN_FOXBMS_MESSAGES_DEFAULT_DLC);
    FAS_ASSERT(pCanData != NULL_PTR);
    FAS_ASSERT(pMuxId != NULL_PTR);
    FAS_ASSERT(*pMuxId < BS_NR_OF_STRINGS);
    FAS_ASSERT(kpkCanShim != NULL_PTR);
    uint64_t messageData = 0u;
    uint64_t data        = 0;

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
    CAN_TxSetMessageDataWithSignalData(&messageData, 3u, 4u, data, message.endianness);

    /* String connected */
    data = CAN_ConvertBooleanToInteger(BMS_IsStringClosed(stringNumber));
    CAN_TxSetMessageDataWithSignalData(&messageData, 4u, 1u, data, message.endianness);

    /* Balancing active: TODO */

    /* String fuse blown */
    data = CAN_ConvertBooleanToInteger(kpkCanShim->pTableErrorState->stringFuseError[stringNumber]);
    CAN_TxSetMessageDataWithSignalData(&messageData, 6u, 1u, data, message.endianness);

    /* Error: Deep-discharge */
    data = CAN_ConvertBooleanToInteger(kpkCanShim->pTableErrorState->deepDischargeDetectedError[stringNumber]);
    CAN_TxSetMessageDataWithSignalData(&messageData, 7u, 1u, data, message.endianness);

    /* Error: Overtemperature charge */
    data = kpkCanShim->pTableMsl->overtemperatureCharge[stringNumber];
    CAN_TxSetMessageDataWithSignalData(&messageData, 8u, 1u, data, message.endianness);

    /* Error: Undertemperature charge */
    data = kpkCanShim->pTableMsl->undertemperatureCharge[stringNumber];
    CAN_TxSetMessageDataWithSignalData(&messageData, 9u, 1u, data, message.endianness);

    /* Error: Overtemperature discharge */
    data = kpkCanShim->pTableMsl->overtemperatureDischarge[stringNumber];
    CAN_TxSetMessageDataWithSignalData(&messageData, 10u, 1u, data, message.endianness);

    /* Error: Undertemperature discharge */
    data = kpkCanShim->pTableMsl->undertemperatureDischarge[stringNumber];
    CAN_TxSetMessageDataWithSignalData(&messageData, 11u, 1u, data, message.endianness);

    /* Error: Overcurrent charge */
    if ((kpkCanShim->pTableMsl->cellChargeOvercurrent[stringNumber] == 1u) ||
        (kpkCanShim->pTableMsl->stringChargeOvercurrent[stringNumber] == 1u)) {
        data = 1u;
    } else {
        data = 0u;
    }
    CAN_TxSetMessageDataWithSignalData(&messageData, 12u, 1u, data, message.endianness);

    /* Error: Overcurrent discharge */
    if ((kpkCanShim->pTableMsl->cellDischargeOvercurrent[stringNumber] == 1u) ||
        (kpkCanShim->pTableMsl->stringDischargeOvercurrent[stringNumber] == 1u)) {
        data = 1u;
    } else {
        data = 0u;
    }
    CAN_TxSetMessageDataWithSignalData(&messageData, 13u, 1u, data, message.endianness);

    /* Error: Overvoltage */
    data = kpkCanShim->pTableMsl->overVoltage[stringNumber];
    CAN_TxSetMessageDataWithSignalData(&messageData, 14u, 1u, data, message.endianness);

    /* Error: Undervoltage */
    data = kpkCanShim->pTableMsl->underVoltage[stringNumber];
    CAN_TxSetMessageDataWithSignalData(&messageData, 15u, 1u, data, message.endianness);

    /* Info: Overtemperature charge - MOL */
    data = kpkCanShim->pTableMol->overtemperatureCharge[stringNumber];
    CAN_TxSetMessageDataWithSignalData(&messageData, 16u, 1u, data, message.endianness);

    /* Info: Undertemperature charge - MOL */
    data = kpkCanShim->pTableMol->undertemperatureCharge[stringNumber];
    CAN_TxSetMessageDataWithSignalData(&messageData, 17u, 1u, data, message.endianness);

    /* Info: Overtemperature discharge - MOL */
    data = kpkCanShim->pTableMol->overtemperatureDischarge[stringNumber];
    CAN_TxSetMessageDataWithSignalData(&messageData, 18u, 1u, data, message.endianness);

    /* Info: Undertemperature discharge - MOL */
    data = kpkCanShim->pTableMol->undertemperatureCharge[stringNumber];
    CAN_TxSetMessageDataWithSignalData(&messageData, 19u, 1u, data, message.endianness);

    /* Info: Overcurrent charge - MOL */
    if ((kpkCanShim->pTableMol->cellChargeOvercurrent[stringNumber] == 1u) ||
        (kpkCanShim->pTableMol->stringChargeOvercurrent[stringNumber] == 1u)) {
        data = 1u;
    } else {
        data = 0u;
    }
    CAN_TxSetMessageDataWithSignalData(&messageData, 20u, 1u, data, message.endianness);

    /* Info: Overcurrent discharge - MOL */
    if ((kpkCanShim->pTableMol->cellDischargeOvercurrent[stringNumber] == 1u) ||
        (kpkCanShim->pTableMol->stringDischargeOvercurrent[stringNumber] == 1u)) {
        data = 1u;
    } else {
        data = 0u;
    }
    CAN_TxSetMessageDataWithSignalData(&messageData, 21u, 1u, data, message.endianness);

    /* Info: Overvoltage - MOL */
    data = kpkCanShim->pTableMol->overVoltage[stringNumber];
    CAN_TxSetMessageDataWithSignalData(&messageData, 22u, 1u, data, message.endianness);

    /* Info: Undervoltage - MOL */
    data = kpkCanShim->pTableMol->underVoltage[stringNumber];
    CAN_TxSetMessageDataWithSignalData(&messageData, 23u, 1u, data, message.endianness);

    /* Warning: Overtemperature charge - RSL */
    data = kpkCanShim->pTableRsl->overtemperatureCharge[stringNumber];
    CAN_TxSetMessageDataWithSignalData(&messageData, 24u, 1u, data, message.endianness);

    /* Warning: Undertemperature charge - RSL */
    data = kpkCanShim->pTableRsl->undertemperatureCharge[stringNumber];
    CAN_TxSetMessageDataWithSignalData(&messageData, 25u, 1u, data, message.endianness);

    /* Warning: Overtemperature discharge - RSL */
    data = kpkCanShim->pTableRsl->overtemperatureDischarge[stringNumber];
    CAN_TxSetMessageDataWithSignalData(&messageData, 26u, 1u, data, message.endianness);

    /* Warning: Undertemperature discharge - RSL */
    data = kpkCanShim->pTableRsl->undertemperatureDischarge[stringNumber];
    CAN_TxSetMessageDataWithSignalData(&messageData, 27u, 1u, data, message.endianness);

    /* Warning: Overcurrent charge - RSL */
    if ((kpkCanShim->pTableRsl->cellChargeOvercurrent[stringNumber] == 1u) ||
        (kpkCanShim->pTableRsl->stringChargeOvercurrent[stringNumber] == 1u)) {
        data = 1u;
    } else {
        data = 0u;
    }
    CAN_TxSetMessageDataWithSignalData(&messageData, 28u, 1u, data, message.endianness);

    /* Warning: Overcurrent discharge - RSL */
    if ((kpkCanShim->pTableMol->cellDischargeOvercurrent[stringNumber] == 1u) ||
        (kpkCanShim->pTableMol->stringDischargeOvercurrent[stringNumber] == 1u)) {
        data = 1u;
    } else {
        data = 0u;
    }
    CAN_TxSetMessageDataWithSignalData(&messageData, 29u, 1u, data, message.endianness);

    /* Warning: Overvoltage - RSL */
    data = kpkCanShim->pTableRsl->overVoltage[stringNumber];
    CAN_TxSetMessageDataWithSignalData(&messageData, 30u, 1u, data, message.endianness);

    /* Warning: Undervoltage - RSL */
    data = kpkCanShim->pTableRsl->underVoltage[stringNumber];
    CAN_TxSetMessageDataWithSignalData(&messageData, 31u, 1u, data, message.endianness);

    /* Error: Positive string contactor */
    data = CAN_ConvertBooleanToInteger(
        kpkCanShim->pTableErrorState->contactorInPositivePathOfStringFeedbackError[stringNumber]);
    CAN_TxSetMessageDataWithSignalData(&messageData, 32u, 1u, data, message.endianness);

    /* Error: Negative string contactor */
    data = CAN_ConvertBooleanToInteger(
        kpkCanShim->pTableErrorState->contactorInNegativePathOfStringFeedbackError[stringNumber]);
    CAN_TxSetMessageDataWithSignalData(&messageData, 33u, 1u, data, message.endianness);

    /* Error: Slave hardware: TODO */

    /* Error: Daisy-chain base: communication */
    data = kpkCanShim->pTableErrorState->afeCommunicationSpiError[stringNumber];
    CAN_TxSetMessageDataWithSignalData(&messageData, 35u, 1u, data, message.endianness);

    /* Error: Daisy-chain redundancy: communication: TODO */
    /* Error: Daisy-chain base: CRC */
    data = kpkCanShim->pTableErrorState->afeCommunicationCrcError[stringNumber];
    CAN_TxSetMessageDataWithSignalData(&messageData, 37u, 1u, data, message.endianness);

    /* Error: Daisy-chain redundancy: CRC: TODO */
    /* Error: Daisy-chain base: Voltage out of operating range */
    data = kpkCanShim->pTableErrorState->afeCellVoltageInvalidError[stringNumber];
    CAN_TxSetMessageDataWithSignalData(&messageData, 39u, 1u, data, message.endianness);

    /* Error: Daisy-chain redundancy: Voltage out of operating range: TODO */
    /* Error: Daisy-chain base: Temperature out of operating range */
    data = kpkCanShim->pTableErrorState->afeCellTemperatureInvalidError[stringNumber];
    CAN_TxSetMessageDataWithSignalData(&messageData, 41u, 1u, data, message.endianness);

    /* Error: Daisy-chain redundancy: Voltage out of operating range: TODO */

    /* Error: current measurement */
    if ((kpkCanShim->pTableErrorState->currentMeasurementInvalidError[stringNumber] == true) ||
        (kpkCanShim->pTableErrorState->currentMeasurementTimeoutError[stringNumber] == true)) {
        data = 1u;
    } else {
        data = 0u;
    }
    CAN_TxSetMessageDataWithSignalData(&messageData, 43u, 1u, data, message.endianness);

    /* Error: Coulomb counting measurement */
    data = kpkCanShim->pTableErrorState->currentSensorCoulombCounterTimeoutError[stringNumber];
    CAN_TxSetMessageDataWithSignalData(&messageData, 44u, 1u, data, message.endianness);

    /* Error: Energy counting measurement */
    data = kpkCanShim->pTableErrorState->currentSensorEnergyCounterTimeoutError[stringNumber];
    CAN_TxSetMessageDataWithSignalData(&messageData, 45u, 1u, data, message.endianness);

    /* Error: Current sensor V1 measurement */
    data = kpkCanShim->pTableErrorState->currentSensorVoltage1TimeoutError[stringNumber];
    CAN_TxSetMessageDataWithSignalData(&messageData, 46u, 1u, data, message.endianness);

    /* Error: Current sensor V2 measurement */
    data = kpkCanShim->pTableErrorState->currentSensorVoltage2TimeoutError[stringNumber];
    CAN_TxSetMessageDataWithSignalData(&messageData, 47u, 1u, data, message.endianness);

    /* Error: Current sensor V3 measurement */
    data = kpkCanShim->pTableErrorState->currentSensorVoltage3TimeoutError[stringNumber];
    CAN_TxSetMessageDataWithSignalData(&messageData, 48u, 1u, data, message.endianness);

    /* Error: Open wire */
    data = kpkCanShim->pTableErrorState->openWireDetectedError[stringNumber];
    CAN_TxSetMessageDataWithSignalData(&messageData, 49u, 1u, data, message.endianness);

    /* Error: Plausibility: Cell temperature */
    data = kpkCanShim->pTableErrorState->plausibilityCheckCellTemperatureError[stringNumber];
    CAN_TxSetMessageDataWithSignalData(&messageData, 51u, 1u, data, message.endianness);

    /* Error: Plausibility: Cell voltage */
    data = kpkCanShim->pTableErrorState->plausibilityCheckCellVoltageError[stringNumber];
    CAN_TxSetMessageDataWithSignalData(&messageData, 52u, 1u, data, message.endianness);

    /* Error: Plausibility: String voltage */
    data = kpkCanShim->pTableErrorState->plausibilityCheckPackVoltageError[stringNumber];
    CAN_TxSetMessageDataWithSignalData(&messageData, 53u, 1u, data, message.endianness);

    /* Error: Plausibility: Cell temperature spread */
    data = kpkCanShim->pTableErrorState->plausibilityCheckCellTemperatureSpreadError[stringNumber];
    CAN_TxSetMessageDataWithSignalData(&messageData, 54u, 1u, data, message.endianness);

    /* Error: Plausibility: Cell voltage spread */
    data = kpkCanShim->pTableErrorState->plausibilityCheckCellVoltageSpreadError[stringNumber];
    CAN_TxSetMessageDataWithSignalData(&messageData, 55u, 1u, data, message.endianness);
    /* AXIVION Enable Style Generic-NoMagicNumbers: */

    /* Open wire number: TODO */

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
