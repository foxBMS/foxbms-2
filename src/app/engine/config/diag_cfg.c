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
 * @file    diag_cfg.c
 * @author  foxBMS Team
 * @date    2019-11-28 (date of creation)
 * @updated 2024-12-20 (date of last update)
 * @version v1.8.0
 * @ingroup ENGINE_CONFIGURATION
 * @prefix  DIAG
 *
 * @brief   Diagnostic module configuration
 * @details The configuration of the different diagnosis events defined in
 *          diag_cfg.h is set in the array diag_diagnosisIdConfiguration[],
 *          e.g., initialization errors or runtime errors.
 *
 *          Every entry of the diag_diagnosisIdConfiguration[] array consists
 *          of
 *           - name of the diagnosis event (defined in diag_cfg.h)
 *           - type of diagnosis event
 *           - diagnosis sensitivity (after how many occurrences event is
 *             counted as error)
 *           - enabling of the recording for diagnosis event
 *           - enabling of the diagnosis event
 *           - callback function for diagnosis event if wished, otherwise
 *             DIAG_dummy_callback
 *
 *          The system monitoring configuration defined in diag_cfg.h is set in
 *          the array sysm_ch_cfg[]. The system monitoring is at the moment
 *          only used for supervising the cyclic/periodic tasks.
 *
 *          Every entry of the sysm_ch_cfg[] consists of
 *           - enum of monitored object
 *           - type of monitored object (at the moment only
 *             DIAG_SYSMON_CYCLICTASK is supported)
 *           - maximum delay in [ms] in which the object needs to call the
 *             SYSM_Notify function defined in diag.c
 *           - enabling of the recording for system monitoring
 *           - enabling of the system monitoring for the monitored object
 *           - callback function if system monitoring notices an error if
 *             wished, otherwise SYSM_DummyCallback
 */

/*========== Includes =======================================================*/
#include "general.h"

#include "diag_cfg.h"

#include "database.h"
#include "diag_cbs.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/
/** value of #DIAG_ID_MAX (as a define for the pre-processor) */
#define DIAG_ID_MAX_FOR_INIT (85u)

FAS_STATIC_ASSERT(DIAG_ID_MAX_FOR_INIT == (uint16_t)DIAG_ID_MAX, "Both values need to be identical.");

/*========== Static Constant and Variable Definitions =======================*/
/** local copy of the #DATA_BLOCK_ERROR_STATE_s table */
static DATA_BLOCK_ERROR_STATE_s diag_tableErrorFlags = {.header.uniqueId = DATA_BLOCK_ID_ERROR_STATE};

/** local copy of the #DATA_BLOCK_MOL_FLAG_s table */
static DATA_BLOCK_MOL_FLAG_s diag_tableMolFlags = {.header.uniqueId = DATA_BLOCK_ID_MOL_FLAG};

/** local copy of the #DATA_BLOCK_RSL_FLAG_s table */
static DATA_BLOCK_RSL_FLAG_s diag_tableRslFlags = {.header.uniqueId = DATA_BLOCK_ID_RSL_FLAG};

/** local copy of the #DATA_BLOCK_MSL_FLAG_s table */
static DATA_BLOCK_MSL_FLAG_s diag_tableMslFlags = {.header.uniqueId = DATA_BLOCK_ID_MSL_FLAG};

const DIAG_DATABASE_SHIM_s diag_kDatabaseShim = {
    .pTableError = &diag_tableErrorFlags,
    .pTableMol   = &diag_tableMolFlags,
    .pTableRsl   = &diag_tableRslFlags,
    .pTableMsl   = &diag_tableMslFlags,
};

/*========== Extern Constant and Variable Definitions =======================*/
/** variable tracking the state of the diag channels */
DIAG_ID_CFG_s diag_diagnosisIdConfiguration[] = {
    /* clang-format off */
    {DIAG_ID_FLASHCHECKSUM,                                     DIAG_SEN_EVENT_1,   DIAG_FATAL_ERROR,   DIAG_NO_DELAY,      DIAG_EVALUATION_ENABLED,    DIAG_DummyCallback},
    {DIAG_ID_SYSTEM_MONITORING,                                 DIAG_SEN_EVENT_1,   DIAG_FATAL_ERROR,   DIAG_NO_DELAY,      DIAG_EVALUATION_ENABLED,    DIAG_ErrorSystemMonitoring},
    {DIAG_ID_INTERLOCK_FEEDBACK,                                DIAG_SEN_EVENT_10,  DIAG_FATAL_ERROR,   DIAG_DELAY_100ms,   DIAG_EVALUATION_ENABLED,    DIAG_ErrorInterlock},
    {DIAG_ID_SUPPLY_VOLTAGE_CLAMP_30C_LOST,                     DIAG_SEN_EVENT_3,   DIAG_FATAL_ERROR,   DIAG_NO_DELAY,      DIAG_EVALUATION_ENABLED,    DIAG_SupplyVoltageClamp30c},
    {DIAG_ID_CELL_VOLTAGE_OVERVOLTAGE_MSL,                      DIAG_SEN_EVENT_50,  DIAG_FATAL_ERROR,   DIAG_DELAY_200ms,   DIAG_EVALUATION_ENABLED,    DIAG_ErrorOvervoltage},
    {DIAG_ID_CELL_VOLTAGE_OVERVOLTAGE_RSL,                      DIAG_SEN_EVENT_50,  DIAG_WARNING,       DIAG_DELAY_DISCARD, DIAG_EVALUATION_ENABLED,    DIAG_ErrorOvervoltage},
    {DIAG_ID_CELL_VOLTAGE_OVERVOLTAGE_MOL,                      DIAG_SEN_EVENT_50,  DIAG_INFO,          DIAG_DELAY_DISCARD, DIAG_EVALUATION_ENABLED,    DIAG_ErrorOvervoltage},
    {DIAG_ID_CELL_VOLTAGE_UNDERVOLTAGE_MSL,                     DIAG_SEN_EVENT_50,  DIAG_FATAL_ERROR,   DIAG_DELAY_200ms,   DIAG_EVALUATION_ENABLED,    DIAG_ErrorUndervoltage},
    {DIAG_ID_CELL_VOLTAGE_UNDERVOLTAGE_RSL,                     DIAG_SEN_EVENT_50,  DIAG_WARNING,       DIAG_DELAY_DISCARD, DIAG_EVALUATION_ENABLED,    DIAG_ErrorUndervoltage},
    {DIAG_ID_CELL_VOLTAGE_UNDERVOLTAGE_MOL,                     DIAG_SEN_EVENT_50,  DIAG_INFO,          DIAG_DELAY_DISCARD, DIAG_EVALUATION_ENABLED,    DIAG_ErrorUndervoltage},
    {DIAG_ID_TEMP_OVERTEMPERATURE_CHARGE_MSL,                   DIAG_SEN_EVENT_500, DIAG_FATAL_ERROR,   DIAG_DELAY_1000ms,  DIAG_EVALUATION_ENABLED,    DIAG_ErrorOvertemperatureCharge},
    {DIAG_ID_TEMP_OVERTEMPERATURE_CHARGE_RSL,                   DIAG_SEN_EVENT_500, DIAG_WARNING,       DIAG_DELAY_DISCARD, DIAG_EVALUATION_ENABLED,    DIAG_ErrorOvertemperatureCharge},
    {DIAG_ID_TEMP_OVERTEMPERATURE_CHARGE_MOL,                   DIAG_SEN_EVENT_500, DIAG_INFO,          DIAG_DELAY_DISCARD, DIAG_EVALUATION_ENABLED,    DIAG_ErrorOvertemperatureCharge},
    {DIAG_ID_TEMP_OVERTEMPERATURE_DISCHARGE_MSL,                DIAG_SEN_EVENT_500, DIAG_FATAL_ERROR,   DIAG_DELAY_1000ms,  DIAG_EVALUATION_ENABLED,    DIAG_ErrorOvertemperatureDischarge},
    {DIAG_ID_TEMP_OVERTEMPERATURE_DISCHARGE_RSL,                DIAG_SEN_EVENT_500, DIAG_WARNING,       DIAG_DELAY_DISCARD, DIAG_EVALUATION_ENABLED,    DIAG_ErrorOvertemperatureDischarge},
    {DIAG_ID_TEMP_OVERTEMPERATURE_DISCHARGE_MOL,                DIAG_SEN_EVENT_500, DIAG_INFO,          DIAG_DELAY_DISCARD, DIAG_EVALUATION_ENABLED,    DIAG_ErrorOvertemperatureDischarge},
    {DIAG_ID_TEMP_UNDERTEMPERATURE_CHARGE_MSL,                  DIAG_SEN_EVENT_500, DIAG_FATAL_ERROR,   DIAG_DELAY_1000ms,  DIAG_EVALUATION_ENABLED,    DIAG_ErrorUndertemperatureCharge},
    {DIAG_ID_TEMP_UNDERTEMPERATURE_CHARGE_RSL,                  DIAG_SEN_EVENT_500, DIAG_WARNING,       DIAG_DELAY_DISCARD, DIAG_EVALUATION_ENABLED,    DIAG_ErrorUndertemperatureCharge},
    {DIAG_ID_TEMP_UNDERTEMPERATURE_CHARGE_MOL,                  DIAG_SEN_EVENT_500, DIAG_INFO,          DIAG_DELAY_DISCARD, DIAG_EVALUATION_ENABLED,    DIAG_ErrorUndertemperatureCharge},
    {DIAG_ID_TEMP_UNDERTEMPERATURE_DISCHARGE_MSL,               DIAG_SEN_EVENT_500, DIAG_FATAL_ERROR,   DIAG_DELAY_1000ms,  DIAG_EVALUATION_ENABLED,    DIAG_ErrorUndertemperatureDischarge},
    {DIAG_ID_TEMP_UNDERTEMPERATURE_DISCHARGE_RSL,               DIAG_SEN_EVENT_500, DIAG_WARNING,       DIAG_DELAY_DISCARD, DIAG_EVALUATION_ENABLED,    DIAG_ErrorUndertemperatureDischarge},
    {DIAG_ID_TEMP_UNDERTEMPERATURE_DISCHARGE_MOL,               DIAG_SEN_EVENT_500, DIAG_INFO,          DIAG_DELAY_DISCARD, DIAG_EVALUATION_ENABLED,    DIAG_ErrorUndertemperatureDischarge},
    {DIAG_ID_OVERCURRENT_CHARGE_CELL_MSL,                       DIAG_SEN_EVENT_10,  DIAG_FATAL_ERROR,   DIAG_DELAY_100ms,   DIAG_EVALUATION_ENABLED,    DIAG_ErrorOvercurrentCharge},
    {DIAG_ID_OVERCURRENT_CHARGE_CELL_RSL,                       DIAG_SEN_EVENT_10,  DIAG_WARNING,       DIAG_DELAY_DISCARD, DIAG_EVALUATION_ENABLED,    DIAG_ErrorOvercurrentCharge},
    {DIAG_ID_OVERCURRENT_CHARGE_CELL_MOL,                       DIAG_SEN_EVENT_10,  DIAG_INFO,          DIAG_DELAY_DISCARD, DIAG_EVALUATION_ENABLED,    DIAG_ErrorOvercurrentCharge},
    {DIAG_ID_OVERCURRENT_DISCHARGE_CELL_MSL,                    DIAG_SEN_EVENT_10,  DIAG_FATAL_ERROR,   DIAG_DELAY_100ms,   DIAG_EVALUATION_ENABLED,    DIAG_ErrorOvercurrentDischarge},
    {DIAG_ID_OVERCURRENT_DISCHARGE_CELL_RSL,                    DIAG_SEN_EVENT_10,  DIAG_WARNING,       DIAG_DELAY_DISCARD, DIAG_EVALUATION_ENABLED,    DIAG_ErrorOvercurrentDischarge},
    {DIAG_ID_OVERCURRENT_DISCHARGE_CELL_MOL,                    DIAG_SEN_EVENT_10,  DIAG_INFO,          DIAG_DELAY_DISCARD, DIAG_EVALUATION_ENABLED,    DIAG_ErrorOvercurrentDischarge},
    {DIAG_ID_STRING_OVERCURRENT_CHARGE_MSL,                     DIAG_SEN_EVENT_10,  DIAG_FATAL_ERROR,   DIAG_DELAY_100ms,   DIAG_EVALUATION_ENABLED,    DIAG_ErrorOvercurrentCharge},
    {DIAG_ID_STRING_OVERCURRENT_CHARGE_RSL,                     DIAG_SEN_EVENT_10,  DIAG_WARNING,       DIAG_DELAY_DISCARD, DIAG_EVALUATION_ENABLED,    DIAG_ErrorOvercurrentCharge},
    {DIAG_ID_STRING_OVERCURRENT_CHARGE_MOL,                     DIAG_SEN_EVENT_10,  DIAG_INFO,          DIAG_DELAY_DISCARD, DIAG_EVALUATION_ENABLED,    DIAG_ErrorOvercurrentCharge},
    {DIAG_ID_STRING_OVERCURRENT_DISCHARGE_MSL,                  DIAG_SEN_EVENT_10,  DIAG_FATAL_ERROR,   DIAG_DELAY_100ms,   DIAG_EVALUATION_ENABLED,    DIAG_ErrorOvercurrentDischarge},
    {DIAG_ID_STRING_OVERCURRENT_DISCHARGE_RSL,                  DIAG_SEN_EVENT_10,  DIAG_WARNING,       DIAG_DELAY_DISCARD, DIAG_EVALUATION_ENABLED,    DIAG_ErrorOvercurrentDischarge},
    {DIAG_ID_STRING_OVERCURRENT_DISCHARGE_MOL,                  DIAG_SEN_EVENT_10,  DIAG_INFO,          DIAG_DELAY_DISCARD, DIAG_EVALUATION_ENABLED,    DIAG_ErrorOvercurrentDischarge},
    {DIAG_ID_PACK_OVERCURRENT_DISCHARGE_MSL,                    DIAG_SEN_EVENT_10,  DIAG_FATAL_ERROR,   DIAG_DELAY_100ms,   DIAG_EVALUATION_ENABLED,    DIAG_ErrorOvercurrentDischarge},
    {DIAG_ID_PACK_OVERCURRENT_CHARGE_MSL,                       DIAG_SEN_EVENT_10,  DIAG_FATAL_ERROR,   DIAG_DELAY_100ms,   DIAG_EVALUATION_ENABLED,    DIAG_ErrorOvercurrentCharge},
    {DIAG_ID_CURRENT_ON_OPEN_STRING,                            DIAG_SEN_EVENT_10,  DIAG_FATAL_ERROR,   DIAG_DELAY_100ms,   DIAG_EVALUATION_ENABLED,    DIAG_ErrorCurrentOnOpenString},
    {DIAG_ID_AFE_SPI,                                           DIAG_SEN_EVENT_5,   DIAG_FATAL_ERROR,   DIAG_DELAY_100ms,   DIAG_EVALUATION_ENABLED,    DIAG_ErrorAfeDriver},
    {DIAG_ID_AFE_COMMUNICATION_INTEGRITY,                       DIAG_SEN_EVENT_5,   DIAG_FATAL_ERROR,   DIAG_DELAY_100ms,   DIAG_EVALUATION_ENABLED,    DIAG_ErrorAfeDriver},
    {DIAG_ID_AFE_MUX,                                           DIAG_SEN_EVENT_5,   DIAG_FATAL_ERROR,   DIAG_DELAY_100ms,   DIAG_EVALUATION_ENABLED,    DIAG_ErrorAfeDriver},
    {DIAG_ID_AFE_CONFIG,                                        DIAG_SEN_EVENT_1,   DIAG_FATAL_ERROR,   DIAG_DELAY_100ms,   DIAG_EVALUATION_ENABLED,    DIAG_ErrorAfeDriver},
    {DIAG_ID_AFE_OPEN_WIRE,                                     DIAG_SEN_EVENT_1,   DIAG_WARNING,       DIAG_DELAY_DISCARD, DIAG_EVALUATION_ENABLED,    DIAG_ErrorAfeDriver},
    {DIAG_ID_CAN_TIMING,                                        DIAG_SEN_EVENT_100, DIAG_FATAL_ERROR,   DIAG_DELAY_200ms,   DIAG_CAN_TIMING,            DIAG_ErrorCanTiming},
    {DIAG_ID_CAN_RX_QUEUE_FULL,                                 DIAG_SEN_EVENT_1,   DIAG_WARNING,       DIAG_DELAY_DISCARD, DIAG_EVALUATION_ENABLED,    DIAG_ErrorCanRxQueueFull},
    {DIAG_ID_CAN_TX_QUEUE_FULL,                                 DIAG_SEN_EVENT_1,   DIAG_WARNING,       DIAG_DELAY_DISCARD, DIAG_EVALUATION_ENABLED,    DIAG_ErrorCanTxQueueFull},
    {DIAG_ID_CURRENT_SENSOR_CC_RESPONDING,                      DIAG_SEN_EVENT_100, DIAG_FATAL_ERROR,   DIAG_DELAY_2000ms,  DIAG_CAN_SENSOR_PRESENT,    DIAG_ErrorCurrentSensor},
    {DIAG_ID_CURRENT_SENSOR_EC_RESPONDING,                      DIAG_SEN_EVENT_100, DIAG_FATAL_ERROR,   DIAG_DELAY_2000ms,  DIAG_CAN_SENSOR_PRESENT,    DIAG_ErrorCurrentSensor},
    {DIAG_ID_CURRENT_SENSOR_RESPONDING,                         DIAG_SEN_EVENT_100, DIAG_FATAL_ERROR,   DIAG_DELAY_200ms,   DIAG_CAN_SENSOR_PRESENT,    DIAG_ErrorCurrentSensor},
    {DIAG_ID_SBC_FIN_ERROR,                                     DIAG_SEN_EVENT_1,   DIAG_FATAL_ERROR,   DIAG_DELAY_100ms,   DIAG_EVALUATION_ENABLED,    DIAG_Sbc},
    {DIAG_ID_SBC_RSTB_ERROR,                                    DIAG_SEN_EVENT_1,   DIAG_FATAL_ERROR,   DIAG_DELAY_100ms,   DIAG_EVALUATION_ENABLED,    DIAG_Sbc},
    {DIAG_ID_PLAUSIBILITY_PACK_VOLTAGE,                         DIAG_SEN_EVENT_10,  DIAG_FATAL_ERROR,   DIAG_DELAY_100ms,   DIAG_EVALUATION_ENABLED,    DIAG_ErrorPlausibility},
    {DIAG_ID_STRING_MINUS_CONTACTOR_FEEDBACK,                   DIAG_SEN_EVENT_20,  DIAG_FATAL_ERROR,   DIAG_DELAY_100ms,   DIAG_EVALUATION_ENABLED,    DIAG_StringContactorFeedback},
    {DIAG_ID_STRING_PLUS_CONTACTOR_FEEDBACK,                    DIAG_SEN_EVENT_20,  DIAG_FATAL_ERROR,   DIAG_DELAY_100ms,   DIAG_EVALUATION_ENABLED,    DIAG_StringContactorFeedback},
    {DIAG_ID_PRECHARGE_CONTACTOR_FEEDBACK,                      DIAG_SEN_EVENT_20,  DIAG_FATAL_ERROR,   DIAG_DELAY_100ms,   DIAG_EVALUATION_ENABLED,    DIAG_PrechargeContactorFeedback},
    {DIAG_ID_PLAUSIBILITY_CELL_VOLTAGE,                         DIAG_SEN_EVENT_1,   DIAG_WARNING,       DIAG_DELAY_DISCARD, DIAG_EVALUATION_ENABLED,    DIAG_PlausibilityCheck},
    {DIAG_ID_PLAUSIBILITY_CELL_VOLTAGE_SPREAD,                  DIAG_SEN_EVENT_1,   DIAG_WARNING,       DIAG_DELAY_DISCARD, DIAG_EVALUATION_ENABLED,    DIAG_PlausibilityCheck},
    {DIAG_ID_PLAUSIBILITY_CELL_TEMP,                            DIAG_SEN_EVENT_1,   DIAG_WARNING,       DIAG_DELAY_DISCARD, DIAG_EVALUATION_ENABLED,    DIAG_PlausibilityCheck},
    {DIAG_ID_PLAUSIBILITY_CELL_TEMPERATURE_SPREAD,              DIAG_SEN_EVENT_1,   DIAG_WARNING,       DIAG_DELAY_DISCARD, DIAG_EVALUATION_ENABLED,    DIAG_PlausibilityCheck},
    {DIAG_ID_AFE_CELL_VOLTAGE_MEAS_ERROR,                       DIAG_SEN_EVENT_1,   DIAG_WARNING,       DIAG_DELAY_DISCARD, DIAG_EVALUATION_ENABLED,    DIAG_ErrorAfe},
    {DIAG_ID_AFE_CELL_TEMPERATURE_MEAS_ERROR,                   DIAG_SEN_EVENT_1,   DIAG_WARNING,       DIAG_DELAY_DISCARD, DIAG_EVALUATION_ENABLED,    DIAG_ErrorAfe},
    {DIAG_ID_BASE_CELL_VOLTAGE_MEASUREMENT_TIMEOUT,             DIAG_SEN_EVENT_1,   DIAG_WARNING,       DIAG_DELAY_DISCARD, DIAG_EVALUATION_ENABLED,    DIAG_ErrorAfe},
    {DIAG_ID_REDUNDANCY0_CELL_VOLTAGE_MEASUREMENT_TIMEOUT,      DIAG_SEN_EVENT_1,   DIAG_WARNING,       DIAG_DELAY_DISCARD, DIAG_EVALUATION_ENABLED,    DIAG_ErrorAfe},
    {DIAG_ID_BASE_CELL_TEMPERATURE_MEASUREMENT_TIMEOUT,         DIAG_SEN_EVENT_1,   DIAG_WARNING,       DIAG_DELAY_DISCARD, DIAG_EVALUATION_ENABLED,    DIAG_ErrorAfe},
    {DIAG_ID_REDUNDANCY0_CELL_TEMPERATURE_MEASUREMENT_TIMEOUT,  DIAG_SEN_EVENT_1,   DIAG_WARNING,       DIAG_DELAY_DISCARD, DIAG_EVALUATION_ENABLED,    DIAG_ErrorAfe},
    {DIAG_ID_DEEP_DISCHARGE_DETECTED,                           DIAG_SEN_EVENT_1,   DIAG_FATAL_ERROR,   DIAG_DELAY_100ms,   DIAG_EVALUATION_ENABLED,    DIAG_ErrorDeepDischarge},
    {DIAG_ID_CURRENT_MEASUREMENT_TIMEOUT,                       DIAG_SEN_EVENT_1,   DIAG_FATAL_ERROR,   DIAG_DELAY_100ms,   DIAG_EVALUATION_ENABLED,    DIAG_ErrorCurrentMeasurement},
    {DIAG_ID_CURRENT_MEASUREMENT_ERROR,                         DIAG_SEN_EVENT_1,   DIAG_FATAL_ERROR,   DIAG_DELAY_100ms,   DIAG_EVALUATION_ENABLED,    DIAG_ErrorCurrentMeasurement},
    {DIAG_ID_PRECHARGE_ABORT_REASON_VOLTAGE,                    DIAG_SEN_EVENT_1,   DIAG_WARNING,       DIAG_DELAY_DISCARD, DIAG_EVALUATION_ENABLED,    DIAG_PrechargeProcess},
    {DIAG_ID_PRECHARGE_ABORT_REASON_CURRENT,                    DIAG_SEN_EVENT_1,   DIAG_WARNING,       DIAG_DELAY_DISCARD, DIAG_EVALUATION_ENABLED,    DIAG_PrechargeProcess},
    {DIAG_ID_CURRENT_SENSOR_V1_MEASUREMENT_TIMEOUT,             DIAG_SEN_EVENT_1,   DIAG_FATAL_ERROR,   DIAG_DELAY_100ms,   DIAG_CAN_SENSOR_PRESENT,    DIAG_ErrorHighVoltageMeasurement},
    {DIAG_ID_CURRENT_SENSOR_V2_MEASUREMENT_TIMEOUT,             DIAG_SEN_EVENT_1,   DIAG_FATAL_ERROR,   DIAG_DELAY_100ms,   DIAG_CAN_SENSOR_PRESENT,    DIAG_ErrorHighVoltageMeasurement},
    {DIAG_ID_CURRENT_SENSOR_V3_MEASUREMENT_TIMEOUT,             DIAG_SEN_EVENT_1,   DIAG_FATAL_ERROR,   DIAG_DELAY_100ms,   DIAG_CAN_SENSOR_PRESENT,    DIAG_ErrorHighVoltageMeasurement},
    {DIAG_ID_CURRENT_SENSOR_POWER_MEASUREMENT_TIMEOUT,          DIAG_SEN_EVENT_1,   DIAG_FATAL_ERROR,   DIAG_DELAY_100ms,   DIAG_CAN_SENSOR_PRESENT,    DIAG_ErrorPowerMeasurement},
    {DIAG_ID_POWER_MEASUREMENT_ERROR,                           DIAG_SEN_EVENT_1,   DIAG_FATAL_ERROR,   DIAG_DELAY_100ms,   DIAG_EVALUATION_ENABLED,    DIAG_ErrorPowerMeasurement},
    {DIAG_ID_INSULATION_MEASUREMENT_VALID,                      DIAG_SEN_EVENT_1,   DIAG_WARNING,       DIAG_DELAY_DISCARD, DIAG_EVALUATION_ENABLED,    DIAG_Insulation},
    {DIAG_ID_LOW_INSULATION_RESISTANCE_ERROR,                   DIAG_SEN_EVENT_5,   DIAG_WARNING,       DIAG_DELAY_DISCARD, DIAG_EVALUATION_ENABLED,    DIAG_Insulation},
    {DIAG_ID_LOW_INSULATION_RESISTANCE_WARNING,                 DIAG_SEN_EVENT_5,   DIAG_WARNING,       DIAG_DELAY_DISCARD, DIAG_EVALUATION_ENABLED,    DIAG_Insulation},
    {DIAG_ID_INSULATION_GROUND_ERROR,                           DIAG_SEN_EVENT_1,   DIAG_WARNING,       DIAG_DELAY_DISCARD, DIAG_EVALUATION_ENABLED,    DIAG_Insulation},
    {DIAG_ID_I2C_PEX_ERROR,                                     DIAG_SEN_EVENT_1,   DIAG_INFO,          DIAG_NO_DELAY,      DIAG_EVALUATION_ENABLED,    DIAG_I2c},
    {DIAG_ID_I2C_RTC_ERROR,                                     DIAG_SEN_EVENT_1,   DIAG_INFO,          DIAG_NO_DELAY,      DIAG_EVALUATION_ENABLED,    DIAG_I2c},
    {DIAG_ID_RTC_CLOCK_INTEGRITY_ERROR,                         DIAG_SEN_EVENT_1,   DIAG_INFO,          DIAG_NO_DELAY,      DIAG_EVALUATION_ENABLED,    DIAG_Rtc},
    {DIAG_ID_RTC_BATTERY_LOW_ERROR,                             DIAG_SEN_EVENT_1,   DIAG_INFO,          DIAG_NO_DELAY,      DIAG_EVALUATION_ENABLED,    DIAG_Rtc},
    {DIAG_ID_FRAM_READ_CRC_ERROR,                               DIAG_SEN_EVENT_1,   DIAG_INFO,          DIAG_NO_DELAY,      DIAG_EVALUATION_ENABLED,    DIAG_FramError},
    {DIAG_ID_ALERT_MODE,                                        DIAG_SEN_EVENT_1,   DIAG_FATAL_ERROR,   DIAG_NO_DELAY,      DIAG_EVALUATION_ENABLED,    DIAG_AlertFlag},
    {DIAG_ID_AEROSOL_ALERT,                                     DIAG_SEN_EVENT_1,   DIAG_WARNING,       DIAG_NO_DELAY,      DIAG_EVALUATION_ENABLED,    DIAG_AerosolAlert},
    /* clang-format on */
};

DIAG_DEV_s diag_device = {
    .nrOfConfiguredDiagnosisEntries   = sizeof(diag_diagnosisIdConfiguration) / sizeof(DIAG_ID_CFG_s),
    .pConfigurationOfDiagnosisEntries = &diag_diagnosisIdConfiguration[0],
    .numberOfFatalErrors              = 0u,
    .pFatalErrorLinkTable             = {GEN_REPEAT_U(NULL_PTR, GEN_STRIP(DIAG_ID_MAX_FOR_INIT))},
};

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/
void DIAG_UpdateFlags(void) {
    DATA_WRITE_DATA(&diag_tableErrorFlags, &diag_tableMolFlags, &diag_tableRslFlags, &diag_tableMslFlags);
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
