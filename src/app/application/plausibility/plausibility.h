/**
 *
 * @copyright &copy; 2010 - 2026, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * @file    plausibility.h
 * @author  foxBMS Team
 * @date    2020-02-24 (date of creation)
 * @updated 2026-10-06 (date of last update)
 * @version v1.12.0
 * @ingroup APPLICATION
 * @prefix  PL
 *
 * @brief   Interface of plausibility validation for voltage, current, power,
 *          and temperature signals
 * @details Declares the plausibility API used by application-level validation
 *          of measurement data.
 *          The interface covers:
 *          - string/battery/high-voltage-bus validation,
 *          - current and power measurement validation,
 *          - cell-voltage and cell-temperature spread checks.
 *          The corresponding implementations update validity flags, derived
 *          aggregate values, and diagnostic events based on configured
 *          thresholds and timeout rules.
 */

#ifndef FOXBMS__PLAUSIBILITY_H_
#define FOXBMS__PLAUSIBILITY_H_

/*========== Includes =======================================================*/
#include "plausibility_cfg.h"

#include "bms-values.h"
#include "database.h"
#include "fstd_types.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/

/**
 * @brief   Validate and aggregate battery voltage from string voltages
 * @details The routine forms an average from valid string voltages.
 *
 *          Selection of input strings:
 *          - If at least one string is connected, only valid and connected
 *            string voltages are used.
 *          - If no string is connected, all valid string voltages are used.
 *
 *          Output behavior:
 *          - If at least one selected valid value exists, writes the average
 *            to #DATA_BLOCK_PACK_VALUES_s.batteryVoltage_mV and clears
 *            #DATA_BLOCK_PACK_VALUES_s.invalidBatteryVoltage.
 *          - If no selected valid value exists, sets
 *            #DATA_BLOCK_PACK_VALUES_s.batteryVoltage_mV to INT32_MAX and
 *            marks #DATA_BLOCK_PACK_VALUES_s.invalidBatteryVoltage.
 * @param[in,out] pTablePackValues  Pack-values database entry
 */
extern void PL_ValidateBatteryVoltageMeasurement(DATA_BLOCK_PACK_VALUES_s *pTablePackValues);

/**
 * @brief   Validate the cell voltage spread plausibility
 * @details Checks if the difference between the cell voltage and the average
 *          cell voltage is within the defined tolerance
 *          (#PL_CELL_VOLTAGE_SPREAD_TOLERANCE_mV).
 *          For each valid cell-voltage entry, the routine compares the value
 *          against the string-average cell voltage.
 *          If the difference exceeds the tolerance, the individual cell
 *          voltage is marked invalid.
 *          The number of valid cell voltages per string is updated and a
 *          diagnostic event is reported per string.
 * @param[in,out] pCellVoltages     cell voltage database entry
 * @param[in] pMinMaxAverageValues  minimum/maximum/average database entry
 * @return  #STD_OK if no issue detected, otherwise #STD_NOT_OK
 */
extern STD_RETURN_TYPE_e PL_CheckVoltageSpread(
    DATA_BLOCK_CELL_VOLTAGE_s *pCellVoltages,
    const DATA_BLOCK_MIN_MAX_s *pMinMaxAverageValues);

/**
 * @brief   Validate the cell temperature spread plausibility
 * @details Checks if the difference between the cell temperature and the
 *          average cell temperature is within the defined tolerance
 *          (#PL_CELL_TEMPERATURE_SPREAD_TOLERANCE_dK).
 *          For each valid cell-temperature entry, the routine compares the
 *          value against the string-average temperature.
 *          If the difference exceeds the tolerance, the individual cell
 *          temperature is marked invalid.
 *          The number of valid temperatures per string is updated and a
 *          diagnostic event is reported per string.
 * @param[in,out] pCellTemperatures cell temperature database entry
 * @param[in] pMinMaxAverageValues  minimum/maximum/average database entry
 * @return  #STD_OK if no issue detected, otherwise #STD_NOT_OK
 */
extern STD_RETURN_TYPE_e PL_CheckTemperatureSpread(
    DATA_BLOCK_CELL_TEMPERATURE_s *pCellTemperatures,
    const DATA_BLOCK_MIN_MAX_s *pMinMaxAverageValues);

/**
 * @brief   Validate results of current measurement
 * @details For each string, this routine checks whether the measurement is
 *          updated within #PL_CURRENT_MEASUREMENT_PERIOD_TIMEOUT_ms and
 *          whether the value is valid.
 *          Updated and valid measurements are copied to
 *          #DATA_BLOCK_PACK_VALUES_s.stringCurrent_mA and marked valid.
 *          Stale or invalid measurements are marked invalid and reported via
 *          diagnostics.
 *          The pack current is calculated as sum of valid string currents.
 *          If any string current is invalid, pack current is marked invalid.
 * @param[in,out] pBmsvlState       system state
 * @param[in,out] pTablePackValues  pack values database entry
 * @param[in] pTableCurrent         current measurements database entry
 */
extern void PL_ValidateCurrentMeasurement(
    BMSVL_STATE_s *pBmsvlState,
    DATA_BLOCK_PACK_VALUES_s *pTablePackValues,
    const DATA_BLOCK_CURRENT_s *pTableCurrent);

/**
 * @brief   Validate results of power measurement
 * @details For each string, this routine checks whether the power
 *          measurement is updated within
 *          #PL_CURRENT_SENSOR_MEASUREMENT_TIMEOUT_ms.
 *          If a fresh and valid power value is available, it is used
 *          directly.
 *          If the value is stale or flagged invalid, the routine attempts a
 *          fallback calculation from string current and string voltage, if
 *          both are valid.
 *          String-level validity and diagnostic events are updated per string.
 *          The pack power is calculated as sum of valid string powers.
 *          If any string power is invalid, pack power is marked invalid.
 * @param[in,out] pBmsvlState       system state
 * @param[in,out] pTablePackValues  pack values database entry
 * @param[in] pTablePower           power measurements database entry
 */
extern void PL_ValidatePowerMeasurement(
    BMSVL_STATE_s *pBmsvlState,
    DATA_BLOCK_PACK_VALUES_s *pTablePackValues,
    DATA_BLOCK_POWER_s *pTablePower);

/**
 * @brief   Validate high-voltage-bus measurement and compute bus voltage
 * @details For each string, this routine checks current-sensor voltage update
 *          timeout against #PL_CURRENT_SENSOR_MEASUREMENT_TIMEOUT_ms.
 *          Only strings that are connected (closed or precharging), updated,
 *          and not flagged invalid are used.
 *          The high-voltage bus value is calculated as average over all valid
 *          selected string voltages.
 *          If no valid value is available, the high-voltage bus is marked
 *          invalid.
 * @param[out] pTablePackValues     pack values database entry
 * @param[in] pTableSystemVoltage3  high voltage bus measurement database entry
 */
extern void PL_ValidateHighVoltageBusMeasurement(
    DATA_BLOCK_PACK_VALUES_s *pTablePackValues,
    const DATA_BLOCK_SYSTEM_VOLTAGE_3_s *pTableSystemVoltage3);

/**
 * @brief   Validate results of string voltage measurement
 * @details The string voltage measurement is validated by checking the
 *          plausibility between current-sensor-based string voltage and
 *          AFE-based string voltage.
 *          If fresh current-sensor data and complete valid cell-voltage data
 *          are available, a plausibility check is performed.
 *          If plausibility cannot be checked, the routine falls back to:
 *          - valid current-sensor measurement, or
 *          - valid AFE string voltage, or
 *          - reconstructed string voltage using average cell voltage and the
 *            number of invalid cell voltages.
 *          Reconstructed values are marked invalid when more than
 *          #PL_ALLOWED_NUMBER_OF_INVALID_CELL_VOLTAGES cells are invalid.
 *          Diagnostic events are updated per string.
 * @param[out] pTablePackValues           pack values database entry
 * @param[in,out] pTableMinimumMaximumValues minimum/maximum database entry
 * @param[in] pTableSystemVoltage1           high voltage measurement database entry
 * @param[in] pTableCellVoltage              cell voltage measurement database entry
 */
extern void PL_ValidateStringVoltageMeasurement(
    DATA_BLOCK_PACK_VALUES_s *pTablePackValues,
    const DATA_BLOCK_MIN_MAX_s *pTableMinimumMaximumValues,
    const DATA_BLOCK_SYSTEM_VOLTAGE_1_s *pTableSystemVoltage1,
    const DATA_BLOCK_CELL_VOLTAGE_s *pTableCellVoltage);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
extern STD_RETURN_TYPE_e TEST_PL_CheckIndividualCellTemperature(
    int16_t cellTemperature_ddegC,
    int16_t averageTemperature_ddegC);
extern STD_RETURN_TYPE_e TEST_PL_CheckIndividualCellVoltage(int16_t cellVoltage_mV, int16_t averageCellVoltage_mV);
extern STD_RETURN_TYPE_e TEST_PL_CheckStringVoltage(int32_t voltageAfe_mV, int32_t voltageCurrentSensor_mV);

#endif

#endif /* FOXBMS__PLAUSIBILITY_H_ */
