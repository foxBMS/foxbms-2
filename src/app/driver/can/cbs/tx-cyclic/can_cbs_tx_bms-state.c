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
 * @file    can_cbs_tx_bms-state.c
 * @author  foxBMS Team
 * @date    2021-07-21 (date of creation)
 * @updated 2024-08-08 (date of last update)
 * @version v1.7.0
 * @ingroup DRIVERS
 * @prefix  CANTX
 *
 * @brief   CAN driver Tx callback implementation
 * @details CAN Tx callback for state messages
 */

/*========== Includes =======================================================*/
#include "bms.h"
/* AXIVION Next Codeline Generic-LocalInclude: 'can_cbs_tx_cyclic.h' declares
 * the prototype for the callback 'CANTX_BmsState' */
#include "can_cbs_tx_cyclic.h"
#include "can_cfg_tx-cyclic-message-definitions.h"
#include "can_helper.h"
#include "diag.h"
#include "foxmath.h"
#include "sys_mon.h"

#include <math.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/
/**
 * Configuration of the signals
 */
#define CANTX_SIGNAL_BMS_NUMBER_OF_CONNECTED_STRINGS_START_BIT           (7u)
#define CANTX_SIGNAL_BMS_NUMBER_OF_CONNECTED_STRINGS_LENGTH              (4u)
#define CANTX_SIGNAL_BMS_BMS_STATE_START_BIT                             (3u)
#define CANTX_SIGNAL_BMS_BMS_STATE_LENGTH                                (4u)
#define CANTX_SIGNAL_BMS_COOLING_STATE_START_BIT                         (15u)
#define CANTX_SIGNAL_BMS_COOLING_STATE_LENGTH                            (CAN_BIT)
#define CANTX_SIGNAL_BMS_HEATER_STATE_START_BIT                          (14u)
#define CANTX_SIGNAL_BMS_HEATER_STATE_LENGTH                             (CAN_BIT)
#define CANTX_SIGNAL_BMS_INSULATION_MONITORING_START_BIT                 (13u)
#define CANTX_SIGNAL_BMS_INSULATION_MONITORING_LENGTH                    (CAN_BIT)
#define CANTX_SIGNAL_BMS_SYSTEM_MONITORING_ERROR_START_BIT               (12u)
#define CANTX_SIGNAL_BMS_SYSTEM_MONITORING_ERROR_LENGTH                  (CAN_BIT)
#define CANTX_SIGNAL_BMS_EMERGENCY_SHUTOFF_START_BIT                     (11u)
#define CANTX_SIGNAL_BMS_EMERGENCY_SHUTOFF_LENGTH                        (CAN_BIT)
#define CANTX_SIGNAL_BMS_GENERAL_ERROR_START_BIT                         (10u)
#define CANTX_SIGNAL_BMS_GENERAL_ERROR_LENGTH                            (CAN_BIT)
#define CANTX_SIGNAL_BMS_GENERAL_WARNING_START_BIT                       (9u)
#define CANTX_SIGNAL_BMS_GENERAL_WARNING_LENGTH                          (CAN_BIT)
#define CANTX_SIGNAL_BMS_CHARGING_COMPLETE_START_BIT                     (8u)
#define CANTX_SIGNAL_BMS_CHARGING_COMPLETE_LENGTH                        (CAN_BIT)
#define CANTX_SIGNAL_BMS_INSULATION_ERROR_START_BIT                      (23u)
#define CANTX_SIGNAL_BMS_INSULATION_ERROR_LENGTH                         (CAN_BIT)
#define CANTX_SIGNAL_BMS_INTERLOCK_STATE_START_BIT                       (22u)
#define CANTX_SIGNAL_BMS_INTERLOCK_STATE_LENGTH                          (CAN_BIT)
#define CANTX_SIGNAL_BMS_MAIN_FUSE_BLOWN_START_BIT                       (21u)
#define CANTX_SIGNAL_BMS_MAIN_FUSE_BLOWN_LENGTH                          (CAN_BIT)
#define CANTX_SIGNAL_BMS_BMS_MASTER_PCB_UNDERTEMPERATURE_ERROR_START_BIT (20u)
#define CANTX_SIGNAL_BMS_BMS_MASTER_PCB_UNDERTEMPERATURE_ERROR_LENGTH    (CAN_BIT)
#define CANTX_SIGNAL_BMS_BMS_MASTER_PCB_OVERTEMPERATURE_ERROR_START_BIT  (19u)
#define CANTX_SIGNAL_BMS_BMS_MASTER_PCB_OVERTEMPERATURE_ERROR_LENGTH     (CAN_BIT)
#define CANTX_SIGNAL_BMS_MCU_DIE_TEMPERATURE_ERROR_START_BIT             (18u)
#define CANTX_SIGNAL_BMS_MCU_DIE_TEMPERATURE_ERROR_LENGTH                (CAN_BIT)
#define CANTX_SIGNAL_BMS_PRECHARGE_CURRENT_ERROR_START_BIT               (17u)
#define CANTX_SIGNAL_BMS_PRECHARGE_CURRENT_ERROR_LENGTH                  (CAN_BIT)
#define CANTX_SIGNAL_BMS_PRECHARGE_VOLTAGE_ERROR_START_BIT               (16u)
#define CANTX_SIGNAL_BMS_PRECHARGE_VOLTAGE_ERROR_LENGTH                  (CAN_BIT)
#define CANTX_SIGNAL_BMS_NVRAM_CRC_ERROR_START_BIT                       (28u)
#define CANTX_SIGNAL_BMS_NVRAM_CRC_ERROR_LENGTH                          (CAN_BIT)
#define CANTX_SIGNAL_BMS_ALERT_FLAG_START_BIT                            (27u)
#define CANTX_SIGNAL_BMS_ALERT_FLAG_LENGTH                               (CAN_BIT)
#define CANTX_SIGNAL_BMS_PACK_OVERCURRENT_DISCHARGE_ERROR_START_BIT      (26u)
#define CANTX_SIGNAL_BMS_PACK_OVERCURRENT_DISCHARGE_ERROR_LENGTH         (CAN_BIT)
#define CANTX_SIGNAL_BMS_PACK_OVERCURRENT_CHARGE_ERROR_START_BIT         (25u)
#define CANTX_SIGNAL_BMS_PACK_OVERCURRENT_CHARGE_ERROR_LENGTH            (CAN_BIT)
#define CANTX_SIGNAL_BMS_CAN_TIMING_ERROR_START_BIT                      (24u)
#define CANTX_SIGNAL_BMS_CAN_TIMING_ERROR_LENGTH                         (CAN_BIT)
#define CANTX_SIGNAL_BMS_NUMBER_OF_DEACTIVATED_STRINGS_START_BIT         (51u)
#define CANTX_SIGNAL_BMS_NUMBER_OF_DEACTIVATED_STRINGS_LENGTH            (4u)
#define CANTX_SIGNAL_BMS_INSULATION_RESISTANCE_START_BIT                 (63u)
#define CANTX_SIGNAL_BMS_INSULATION_RESISTANCE_LENGTH                    (8u)

#define CANTX_FACTOR_INSULATION_RESISTANCE        (200.0f)
#define CANTX_MINIMUM_VALUE_INSULATION_RESISTANCE (0.0f)
#define CANTX_MAXIMUM_VALUE_INSULATION_RESISTANCE (51000.0f)

/** @{
 * configuration of the insulation resistance signal
*/
static const CAN_SIGNAL_TYPE_s cantx_signalInsulationResistance = {
    CANTX_SIGNAL_BMS_INSULATION_RESISTANCE_START_BIT,
    CANTX_SIGNAL_BMS_INSULATION_RESISTANCE_LENGTH,
    CANTX_FACTOR_INSULATION_RESISTANCE,
    CAN_SIGNAL_OFFSET_0,
    CANTX_MINIMUM_VALUE_INSULATION_RESISTANCE,
    CANTX_MAXIMUM_VALUE_INSULATION_RESISTANCE};
/** @} */

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/
/**
 * @brief   get a boolean for if any timing error (current or recorded) occurred
 * @param   kpkCanShim  const pointer to CAN shim
 * @return  returns if there has been any timing violations
 */
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
    FAS_ASSERT(message.idType == CANTX_BMS_STATE_ID_TYPE);
    FAS_ASSERT(message.dlc == CANTX_BMS_STATE_DLC);
    FAS_ASSERT(message.endianness == CANTX_BMS_STATE_ENDIANNESS);
    FAS_ASSERT(pCanData != NULL_PTR);
    FAS_ASSERT(pMuxId == NULL_PTR); /* pMuxId is not used here, therefore has to be NULL_PTR */
    FAS_ASSERT(kpkCanShim != NULL_PTR);
    uint64_t messageData = 0u;

    DATA_READ_DATA(kpkCanShim->pTableErrorState, kpkCanShim->pTableInsulation, kpkCanShim->pTableMsl);

    /* State */
    uint64_t data = (uint64_t)BMS_GetState();
    /* set data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(
        &messageData,
        CANTX_SIGNAL_BMS_BMS_STATE_START_BIT,
        CANTX_SIGNAL_BMS_BMS_STATE_LENGTH,
        data,
        message.endianness);

    /* Connected strings */
    data = (uint64_t)BMS_GetNumberOfConnectedStrings();
    /* set data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(
        &messageData,
        CANTX_SIGNAL_BMS_NUMBER_OF_CONNECTED_STRINGS_START_BIT,
        CANTX_SIGNAL_BMS_NUMBER_OF_CONNECTED_STRINGS_LENGTH,
        data,
        message.endianness);

    /* General warning: TODO */

    /* General error - implement now */
    data = CAN_ConvertBooleanToInteger(DIAG_IsAnyFatalErrorSet());
    CAN_TxSetMessageDataWithSignalData(
        &messageData,
        CANTX_SIGNAL_BMS_GENERAL_ERROR_START_BIT,
        CANTX_SIGNAL_BMS_GENERAL_ERROR_LENGTH,
        data,
        message.endianness);

    /* Emergency shutoff */
    data = CAN_ConvertBooleanToInteger(BMS_IsTransitionToErrorStateActive());
    CAN_TxSetMessageDataWithSignalData(
        &messageData,
        CANTX_SIGNAL_BMS_EMERGENCY_SHUTOFF_START_BIT,
        CANTX_SIGNAL_BMS_EMERGENCY_SHUTOFF_LENGTH,
        data,
        message.endianness);

    /* Number of deactivated strings: TODO */

    /* sys mon error */
    data = CAN_ConvertBooleanToInteger(CANTX_AnySysMonTimingIssueDetected(kpkCanShim));
    CAN_TxSetMessageDataWithSignalData(
        &messageData,
        CANTX_SIGNAL_BMS_SYSTEM_MONITORING_ERROR_START_BIT,
        CANTX_SIGNAL_BMS_SYSTEM_MONITORING_ERROR_LENGTH,
        data,
        message.endianness);

    /* Insulation monitoring active */
    data = CAN_ConvertBooleanToInteger(kpkCanShim->pTableInsulation->isImdRunning);
    CAN_TxSetMessageDataWithSignalData(
        &messageData,
        CANTX_SIGNAL_BMS_INSULATION_MONITORING_START_BIT,
        CANTX_SIGNAL_BMS_INSULATION_MONITORING_LENGTH,
        data,
        message.endianness);

    /* Error: insulation */
    data = CAN_ConvertBooleanToInteger(kpkCanShim->pTableErrorState->criticalLowInsulationResistanceError);
    CAN_TxSetMessageDataWithSignalData(
        &messageData,
        CANTX_SIGNAL_BMS_INSULATION_ERROR_START_BIT,
        CANTX_SIGNAL_BMS_INSULATION_ERROR_LENGTH,
        data,
        message.endianness);

    /* Insulation resistance */
    float_t signalData = (float_t)kpkCanShim->pTableInsulation->insulationResistance_kOhm;
    CAN_TxPrepareSignalData(&signalData, cantx_signalInsulationResistance);
    data = (uint64_t)signalData;
    CAN_TxSetMessageDataWithSignalData(
        &messageData,
        cantx_signalInsulationResistance.bitStart,
        cantx_signalInsulationResistance.bitLength,
        data,
        message.endianness);

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
    CAN_TxSetMessageDataWithSignalData(
        &messageData,
        CANTX_SIGNAL_BMS_PRECHARGE_VOLTAGE_ERROR_START_BIT,
        CANTX_SIGNAL_BMS_PRECHARGE_VOLTAGE_ERROR_LENGTH,
        data,
        message.endianness);

    /* Error: Precharge current */
    data = 0u; /* No precharge error detected */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        if (kpkCanShim->pTableErrorState->prechargeAbortedDueToCurrent[s] == true) {
            data = 1u;
        }
    }
    CAN_TxSetMessageDataWithSignalData(
        &messageData,
        CANTX_SIGNAL_BMS_PRECHARGE_CURRENT_ERROR_START_BIT,
        CANTX_SIGNAL_BMS_PRECHARGE_CURRENT_ERROR_LENGTH,
        data,
        message.endianness);

    /* Error: MCU die temperature */
    data = CAN_ConvertBooleanToInteger(kpkCanShim->pTableErrorState->mcuDieTemperatureViolationError);
    CAN_TxSetMessageDataWithSignalData(
        &messageData,
        CANTX_SIGNAL_BMS_MCU_DIE_TEMPERATURE_ERROR_START_BIT,
        CANTX_SIGNAL_BMS_MCU_DIE_TEMPERATURE_ERROR_LENGTH,
        data,
        message.endianness);

    /* Error: master overtemperature: TODO */
    /* Error: master undertemperature: TODO */

    /* Main fuse state */
    data = CAN_ConvertBooleanToInteger(kpkCanShim->pTableErrorState->mainFuseError);
    CAN_TxSetMessageDataWithSignalData(
        &messageData,
        CANTX_SIGNAL_BMS_MAIN_FUSE_BLOWN_START_BIT,
        CANTX_SIGNAL_BMS_MAIN_FUSE_BLOWN_LENGTH,
        data,
        message.endianness);

    /* Error: interlock */
    data = CAN_ConvertBooleanToInteger(kpkCanShim->pTableErrorState->interlockOpenedError);
    CAN_TxSetMessageDataWithSignalData(
        &messageData,
        CANTX_SIGNAL_BMS_INTERLOCK_STATE_START_BIT,
        CANTX_SIGNAL_BMS_INTERLOCK_STATE_LENGTH,
        data,
        message.endianness);

    /* Error: Can timing */
    data = kpkCanShim->pTableErrorState->stateRequestTimingViolationError;
    CAN_TxSetMessageDataWithSignalData(
        &messageData,
        CANTX_SIGNAL_BMS_CAN_TIMING_ERROR_START_BIT,
        CANTX_SIGNAL_BMS_CAN_TIMING_ERROR_LENGTH,
        data,
        message.endianness);

    /* Error: Overcurrent pack charge */
    data = kpkCanShim->pTableMsl->packChargeOvercurrent;
    CAN_TxSetMessageDataWithSignalData(
        &messageData,
        CANTX_SIGNAL_BMS_PACK_OVERCURRENT_CHARGE_ERROR_START_BIT,
        CANTX_SIGNAL_BMS_PACK_OVERCURRENT_CHARGE_ERROR_LENGTH,
        data,
        message.endianness);

    /* Error: Overcurrent pack discharge */
    data = kpkCanShim->pTableMsl->packDischargeOvercurrent;
    CAN_TxSetMessageDataWithSignalData(
        &messageData,
        CANTX_SIGNAL_BMS_PACK_OVERCURRENT_DISCHARGE_ERROR_START_BIT,
        CANTX_SIGNAL_BMS_PACK_OVERCURRENT_DISCHARGE_ERROR_LENGTH,
        data,
        message.endianness);

    /* Error: Alert flag */
    data = CAN_ConvertBooleanToInteger(kpkCanShim->pTableErrorState->alertFlagSetError);
    CAN_TxSetMessageDataWithSignalData(
        &messageData,
        CANTX_SIGNAL_BMS_ALERT_FLAG_START_BIT,
        CANTX_SIGNAL_BMS_ALERT_FLAG_LENGTH,
        data,
        message.endianness);

    /* Error: NVRAM CRC */
    data = CAN_ConvertBooleanToInteger(kpkCanShim->pTableErrorState->framReadCrcError);
    CAN_TxSetMessageDataWithSignalData(
        &messageData,
        CANTX_SIGNAL_BMS_NVRAM_CRC_ERROR_START_BIT,
        CANTX_SIGNAL_BMS_NVRAM_CRC_ERROR_LENGTH,
        data,
        message.endianness);

    /* now copy data in the buffer that will be use to send data */
    CAN_TxSetCanDataWithMessageData(messageData, pCanData, message.endianness);

    return 0u;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
extern bool TEST_CANTX_AnySysMonTimingIssueDetected(const CAN_SHIM_s *const kpkCanShim) {
    return CANTX_AnySysMonTimingIssueDetected(kpkCanShim);
}

#endif
