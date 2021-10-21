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
 * @file    redundancy.h
 * @author  foxBMS Team
 * @date    2020-07-31 (date of creation)
 * @updated 2021-10-14 (date of last update)
 * @ingroup APPLICATION
 * @prefix  MRC
 *
 * @brief   Header fileS for handling redundancy between redundant cell voltage
 *          and cell temperature measurements
 *
 */

#ifndef FOXBMS__REDUNDANCY_H_
#define FOXBMS__REDUNDANCY_H_

/*========== Includes =======================================================*/
#include "general.h"

#include "database.h"

/*========== Macros and Definitions =========================================*/
/**
 * Maximum time between measurements before the
 * redundancy module raises an error because a
 * measurement is not updated anymore.
 *
 * The redundancy module will wait a maximum of this time for new current
 * values. If no new values are updated within this timeframe it
 * will invalidate the measurement values.
 */
#define MRC_CURRENT_MEASUREMENT_PERIOD_TIMEOUT_ms (250u)

/**
 * Maximum time between AFE measurements before the
 * redundancy module raises an error because a
 * measurement is not updated anymore.
 *
 * The redundancy module will wait a maximum of this
 * time for new values from the base AFE measurement and
 * AFE redundant measurements. If no new values are updated
 * from both measurement sources within this timeframe
 * it will validate the measurement values it has up to
 * this point if possible.
 */
#define MRC_AFE_MEASUREMENT_PERIOD_TIMEOUT_ms (250u)

/**
 * Maximum time between current sensor high voltage, current
 * and power measurements before the redundancy module raises
 * an error because a measurement is not updated anymore.
 *
 * The redundancy module will wait a maximum of this
 * time for new values from the current sensor. If no
 * new values are updated within this timeframe it will
 * validate the measurement values it has up to this point
 * if possible.
 */
#define MRC_CURRENT_SENSOR_MEASUREMENT_TIMEOUT_ms (300u)

/**
 * If both, the current sensor and the AFE measurement have no valid values
 * we try to construct the string voltage by replacing invalid cell voltage
 * measurements with the average cell voltage in this string. The result of
 * this estimation will be flagged as invalid if more than the number of
 * allowed invalid cell voltages are detected. The result will be markes as
 * valid if less then this number of cells are detected as invalid.
 */
#define MRC_ALLOWED_NUMBER_OF_INVALID_CELL_VOLTAGES (5u)

/*========== Extern Constant and Variable Declarations ======================*/
/**
 * This structure contains all the variables relevant for the redundancy state machine.
 */
typedef struct MRC_STATE {
    uint32_t lastBaseCellvoltageTimestamp;
    uint32_t lastRedundancy0CellvoltageTimestamp;
    uint32_t lastBaseCelltemperatureTimestamp;
    uint32_t lastRedundancy0CelltemperatureTimestamp;
    uint32_t lastStringCurrentTimestamp[BS_NR_OF_STRINGS];
    uint32_t lastStringPowerTimestamp[BS_NR_OF_STRINGS];
} MRC_STATE_s;

/*========== Extern Function Prototypes =====================================*/
/**
 * @brief Function to initalize redundancy module
 *
 * @return #STD_OK if module has been initialized successfully, otherwise #STD_NOT_OK
 */
extern STD_RETURN_TYPE_e MRC_Initialize(void);

/**
 * @brief Function to validate the measurement between redundant measurement
 *        values for cell voltage and cell temperature
 *
 * @return #STD_OK if measurement has been validated successfully, otherwise i.e.
 *         if no new values have been measured since the last call #STD_NOT_OK
 */
extern STD_RETURN_TYPE_e MRC_ValidateAfeMeasurement(void);

/**
 * @brief Function to validate the measurements of pack values (string values,
 *        pack values)
 *
 * @return #STD_OK if measurement has been validated successfully, otherwise i.e.
 *         if no new values have been measured since the last call #STD_NOT_OK
 */
extern STD_RETURN_TYPE_e MRC_ValidatePackMeasurement(void);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
extern bool TEST_MRC_MeasurementUpdatedAtLeastOnce(uint32_t timestamp, uint32_t previousTimestamp);
extern STD_RETURN_TYPE_e TEST_MRC_MeasurementUpdatedRecently(
    uint32_t timestamp,
    uint32_t previousTimestamp,
    uint32_t timeInterval);
extern bool TEST_MRC_ValidateCellVoltageMeasurement(
    DATA_BLOCK_CELL_VOLTAGE_s *pCellVoltageBase,
    DATA_BLOCK_CELL_VOLTAGE_s *pCellVoltageRedundancy0);
extern bool TEST_MRC_ValidateCellTemperatureMeasurement(
    DATA_BLOCK_CELL_TEMPERATURE_s *pCellTemperatureBase,
    DATA_BLOCK_CELL_TEMPERATURE_s *pCellTemperatureRedundancy0);
extern void TEST_MRC_ValidateCurrentMeasurement(DATA_BLOCK_CURRENT_SENSOR_s *pTableCurrentSensor);
extern void TEST_MRC_ValidatePowerMeasurement(DATA_BLOCK_CURRENT_SENSOR_s *pTableCurrentSensor);
extern void TEST_MRC_ValidateStringVoltageMeasurement(
    DATA_BLOCK_CURRENT_SENSOR_s *pTableCurrentSensor,
    DATA_BLOCK_CELL_VOLTAGE_s *pTableCellVoltage);
extern void TEST_MRC_ValidateBatteryVoltageMeasurement(void);
extern void TEST_MRC_ValidateHighVoltageMeasurement(DATA_BLOCK_CURRENT_SENSOR_s *pTableCurrentSensor);
extern STD_RETURN_TYPE_e TEST_MRC_CalculateCellVoltageMinMaxAverage(
    DATA_BLOCK_CELL_VOLTAGE_s *pValidatedVoltages,
    DATA_BLOCK_MIN_MAX_s *pMinMaxAverageValues);
extern STD_RETURN_TYPE_e TEST_MRC_CalculateCellTemperatureMinMaxAverage(
    DATA_BLOCK_CELL_TEMPERATURE_s *pValidatedTemperatures,
    DATA_BLOCK_MIN_MAX_s *pMinMaxAverageValues);
extern STD_RETURN_TYPE_e TEST_MRC_ValidateCellVoltage(
    DATA_BLOCK_CELL_VOLTAGE_s *pCellvoltageBase,
    DATA_BLOCK_CELL_VOLTAGE_s *pCellvoltageRedundancy0,
    DATA_BLOCK_CELL_VOLTAGE_s *pValidatedVoltages);
extern STD_RETURN_TYPE_e TEST_MRC_UpdateCellVoltageValidation(
    DATA_BLOCK_CELL_VOLTAGE_s *pCellvoltage,
    DATA_BLOCK_CELL_VOLTAGE_s *pValidatedVoltages);
extern STD_RETURN_TYPE_e TEST_MRC_ValidateCellTemperature(
    DATA_BLOCK_CELL_TEMPERATURE_s *pCelltemperatureBase,
    DATA_BLOCK_CELL_TEMPERATURE_s *pCelltemperatureRedundancy0,
    DATA_BLOCK_CELL_TEMPERATURE_s *pValidatedTemperatures);
extern STD_RETURN_TYPE_e TEST_MRC_UpdateCellTemperatureValidation(
    DATA_BLOCK_CELL_TEMPERATURE_s *pCellTemperature,
    DATA_BLOCK_CELL_TEMPERATURE_s *pValidatedTemperature);
#endif

#endif /* FOXBMS__REDUNDANCY_H_ */
