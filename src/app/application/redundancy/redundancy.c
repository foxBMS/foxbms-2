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
 * @file    redundancy.c
 * @author  foxBMS Team
 * @date    2020-07-31 (date of creation)
 * @updated 2021-12-08 (date of last update)
 * @ingroup APPLICATION
 * @prefix  MRC
 *
 * @brief   Source file for handling redundancy between redundant cell voltage
 *          and cell temperature measurements
 *
 */

/*========== Includes =======================================================*/
#include "redundancy.h"

#include "bms.h"
#include "database.h"
#include "diag.h"
#include "foxmath.h"
#include "os.h"
#include "plausibility.h"

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/
/** local copies of database tables */
/**@{*/
static DATA_BLOCK_MIN_MAX_s mrc_tableMinimumMaximumValues      = {.header.uniqueId = DATA_BLOCK_ID_MIN_MAX};
static DATA_BLOCK_CELL_VOLTAGE_s mrc_tableCellVoltages         = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE};
static DATA_BLOCK_CELL_TEMPERATURE_s mrc_tableCellTemperatures = {.header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE};
static DATA_BLOCK_PACK_VALUES_s mrc_tablePackValues            = {.header.uniqueId = DATA_BLOCK_ID_PACK_VALUES};
/**@}*/

/** state of the redundancy module */
static MRC_STATE_s mrc_state = {
    .lastBaseCellvoltageTimestamp            = 0u,
    .lastRedundancy0CellvoltageTimestamp     = 0u,
    .lastBaseCelltemperatureTimestamp        = 0u,
    .lastRedundancy0CelltemperatureTimestamp = 0u,
    .lastStringCurrentTimestamp              = {0u},
};

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/
/**
 * @brief   Check timestamp if measurement has been updated at least once.
 * @param[in]  timestamp          timestamp of last measurement update
 * @param[in]  previousTimestamp  timestamp of previously updated measurement
 * @return true if measurement has been updated at least once, otherwise false
 */
static bool MRC_MeasurementUpdatedAtLeastOnce(uint32_t timestamp, uint32_t previousTimestamp);

/**
 * @brief   Check timestamp if measurement has been updated recently.
 * @param[in]  timestamp          timestamp of last measurement update
 * @param[in]  previousTimestamp  timestamp of previously updated measurement
 * @param[in]  timeInterval       in systicks (type: uint32_t)
 * @return true if measurement has recently been updated, otherwise false
 */
static STD_RETURN_TYPE_e MRC_MeasurementUpdatedRecently(
    uint32_t timestamp,
    uint32_t previousTimestamp,
    uint32_t timeInterval);

/**
 * @brief Function to validate results of cell voltage measurement
 * @param[in] pCellVoltageBase         base cell voltage measurement
 * @param[in] pCellVoltageRedundancy0  redundant cell voltage measurement
 * @return bool true, if measurement has been validated successfully and
 *         database entry needs to be updated, otherwise false.
 */
static bool MRC_ValidateCellVoltageMeasurement(
    DATA_BLOCK_CELL_VOLTAGE_s *pCellVoltageBase,
    DATA_BLOCK_CELL_VOLTAGE_s *pCellVoltageRedundancy0);

/**
 * @brief Function to validate results of cell temperature measurement
 * @param[in] pCellTemperatureBase         base cell temperature measurement
 * @param[in] pCellTemperatureRedundancy0  redundant cell temperature measurement
 * @return bool true, if measurement has been validated successfully and
 *         database entry needs to be updated, otherwise false.
 */
static bool MRC_ValidateCellTemperatureMeasurement(
    DATA_BLOCK_CELL_TEMPERATURE_s *pCellTemperatureBase,
    DATA_BLOCK_CELL_TEMPERATURE_s *pCellTemperatureRedundancy0);

/**
 * @brief Function to validate results of current measurement
 * @param[in] pTableCurrentSensor   pointer to current measurements
 */
static void MRC_ValidateCurrentMeasurement(DATA_BLOCK_CURRENT_SENSOR_s *pTableCurrentSensor);

/**
 * @brief Function to validate results of string voltage measurement
 * @param[in] pTableCurrentSensor   pointer current sensor high voltage measurements
 * @param[in] pTableCellVoltage     pointer to cell voltage measurements
 */
static void MRC_ValidateStringVoltageMeasurement(
    DATA_BLOCK_CURRENT_SENSOR_s *pTableCurrentSensor,
    DATA_BLOCK_CELL_VOLTAGE_s *pTableCellVoltage);

/**
 * @brief Function to validate HV battery voltage measurement
 */
static void MRC_ValidateBatteryVoltageMeasurement(void);

/**
 * @brief Function to validate results of high voltage measurement and calculate
 *        battery voltage and high voltage bus voltage.
 * @param[in] pTableCurrentSensor   pointer current sensor high voltage measurements
 */
static void MRC_ValidateHighVoltageBusMeasurement(DATA_BLOCK_CURRENT_SENSOR_s *pTableCurrentSensor);

/**
 * @brief Function to validate results of power measurement
 * @param[in] pTableCurrentSensor   pointer to power/current measurements
 */
static void MRC_ValidatePowerMeasurement(DATA_BLOCK_CURRENT_SENSOR_s *pTableCurrentSensor);

/**
 * @brief Function calculates minimum, maximum and average cell voltages.
 * @param[in] pValidatedVoltages     validated voltages from base and/or redundant measurements
 * @param[out] pMinMaxAverageValues  calculated cell voltage min/max/average values
 * @return #STD_NOT_OK if all cell voltage are invalid and no minimum, maximum
 *         and average cell voltage values could be calculated, otherwise #STD_OK
 */
static STD_RETURN_TYPE_e MRC_CalculateCellVoltageMinMaxAverage(
    const DATA_BLOCK_CELL_VOLTAGE_s *const pValidatedVoltages,
    DATA_BLOCK_MIN_MAX_s *pMinMaxAverageValues);

/**
 * @brief Function calculates minimum, maximum and average cell temperatures.
 * @param[in] pValidatedTemperatures validated temperatures from base and/or redundant measurements
 * @param[out] pMinMaxAverageValues  calculated cell temperature min/max/average values
 * @return #STD_NOT_OK if all cell temperatures are invalid and no minimum, maximum
 *         and average cell temperature values could be calculated, otherwise #STD_OK
 */
static STD_RETURN_TYPE_e MRC_CalculateCellTemperatureMinMaxAverage(
    DATA_BLOCK_CELL_TEMPERATURE_s *pValidatedTemperatures,
    DATA_BLOCK_MIN_MAX_s *pMinMaxAverageValues);

/**
 * @brief Function compares cell voltage measurements from base measurement with
 *        one redundant measurement and writes result in pValidatedVoltages.
 * @param[in] pCellvoltageBase         base cell voltage measurement
 * @param[in] pCellvoltageRedundancy0  redundant cell voltage measurement
 * @param[out] pValidatedVoltages      validated voltages from redundant measurement values
 * @return #STD_NOT_OK if not all cell voltages could be validated, otherwise
 *         #STD_OK
 */
static STD_RETURN_TYPE_e MRC_ValidateCellVoltage(
    DATA_BLOCK_CELL_VOLTAGE_s *pCellvoltageBase,
    DATA_BLOCK_CELL_VOLTAGE_s *pCellvoltageRedundancy0,
    DATA_BLOCK_CELL_VOLTAGE_s *pValidatedVoltages);

/**
 * @brief Function updates validated cell voltage measurement with data from a
 *        single measurement source. This can be the case if no redundancy is
 *        used at all or if one or more of the redundant measurements are not
 *        working properly.
 * @param[in] pCellvoltage         cell voltage measurement
 * @param[out] pValidatedVoltages  validated voltage values
 * @return #STD_NOT_OK if not all cell voltages could be validated, otherwise
 *         #STD_OK
 */
static STD_RETURN_TYPE_e MRC_UpdateCellVoltageValidation(
    DATA_BLOCK_CELL_VOLTAGE_s *pCellvoltage,
    DATA_BLOCK_CELL_VOLTAGE_s *pValidatedVoltages);

/**
 * @brief Function compares cell temperature measurements from base measurement
 *        with one redundant measurement and writes result in pValidatedTemperatures.
 * @param[in] pCelltemperatureBase         base cell temperature measurement
 * @param[in] pCelltemperatureRedundancy0  redundant cell temperature measurement
 * @param[out] pValidatedTemperatures      validated temperatures from redundant measurement values
 * @return #STD_NOT_OK if not all cell voltages could be validated, otherwise
 *         #STD_OK
 */
static STD_RETURN_TYPE_e MRC_ValidateCellTemperature(
    DATA_BLOCK_CELL_TEMPERATURE_s *pCelltemperatureBase,
    DATA_BLOCK_CELL_TEMPERATURE_s *pCelltemperatureRedundancy0,
    DATA_BLOCK_CELL_TEMPERATURE_s *pValidatedTemperatures);

/**
 * @brief Function updates validated cell temperature measurement with data from a
 *        single measurement source. This can be the case if no redundancy is
 *        used at all or if one or more of the redundant measurements are not
 *        working properly.
 * @param[in] pCellTemperature         cell temperature measurement
 * @param[out] pValidatedTemperature   validated temperature values
 * @return #STD_NOT_OK if not all cell voltages could be validated, otherwise
 *         #STD_OK
 */
static STD_RETURN_TYPE_e MRC_UpdateCellTemperatureValidation(
    DATA_BLOCK_CELL_TEMPERATURE_s *pCellTemperature,
    DATA_BLOCK_CELL_TEMPERATURE_s *pValidatedTemperature);

/*========== Static Function Implementations ================================*/
static bool MRC_MeasurementUpdatedAtLeastOnce(uint32_t timestamp, uint32_t previousTimestamp) {
    bool retval = false;
    if (!((timestamp == 0u) && (previousTimestamp == 0u))) {
        /* Only possibility for timestamp AND previous timestamp to be 0 is, if
           the measurement has never been updated. Thus, if this is not the case
           the measurement must have been updated */
        retval = true;
    }
    return retval;
}

static STD_RETURN_TYPE_e MRC_MeasurementUpdatedRecently(
    uint32_t timestamp,
    uint32_t previousTimestamp,
    uint32_t timeInterval) {
    STD_RETURN_TYPE_e retval  = STD_NOT_OK;
    uint32_t currentTimestamp = OS_GetTickCount();

    /* Unsigned integer arithmetic also works correctly if currentTimestap is
       larger than pHeader->timestamp (timer overflow), thus no need to use abs() */
    if (((currentTimestamp - timestamp) <= timeInterval) &&
        (MRC_MeasurementUpdatedAtLeastOnce(timestamp, previousTimestamp) == true)) {
        /* Difference between current timestamp and last update timestamp is
           smaller than passed time interval */
        retval = STD_OK;
    }
    return retval;
}

static bool MRC_ValidateCellVoltageMeasurement(
    DATA_BLOCK_CELL_VOLTAGE_s *pCellVoltageBase,
    DATA_BLOCK_CELL_VOLTAGE_s *pCellVoltageRedundancy0) {
    FAS_ASSERT(pCellVoltageBase != NULL_PTR);
    FAS_ASSERT(pCellVoltageRedundancy0 != NULL_PTR);

    bool updatedValidatedVoltageDatbaseEntry = false;

    bool baseCellvoltageUpdated = false;

    bool useCellvoltageRedundancy                        = false;
    bool baseCellvoltageMeasurementTimeoutReached        = true;
    bool redundancy0CellvoltageMeasurementTimeoutReached = true;

    /* Following decision will be undertaken:
     *
     * - Use redundancy only if timestamp of redundant database entry != 0
     * - Perform redundant check if both timestamps have been updated
     * - Add timeout and trigger error if timestamps have not been updated in the last XXXXms
     * - If timeout reached and only one of the two redundant measurements has been updated use this measurement value
     *   but throw an error that one measurement has not been updated recently.
     */

    /* -------------- Check if cell voltage redundant measurement is used -- */
    /* Use redundant cell voltage measurements if measurement values have been acquired once */
    useCellvoltageRedundancy = DATA_DatabaseEntryUpdatedAtLeastOnce(pCellVoltageRedundancy0->header);

    /* ----------------- Check timestamp of base measurements--------------- */
    if (DATA_EntryUpdatedWithinInterval(pCellVoltageBase->header, MRC_AFE_MEASUREMENT_PERIOD_TIMEOUT_ms) == true) {
        baseCellvoltageMeasurementTimeoutReached = false;
        (void)DIAG_Handler(DIAG_ID_BASE_CELL_VOLTAGE_MESUREMENT_TIMEOUT, DIAG_EVENT_OK, DIAG_SYSTEM, 0u);
    } else {
        baseCellvoltageMeasurementTimeoutReached = true;
        /* Set error flag */
        (void)DIAG_Handler(DIAG_ID_BASE_CELL_VOLTAGE_MESUREMENT_TIMEOUT, DIAG_EVENT_NOT_OK, DIAG_SYSTEM, 0u);
    }

    /* Check if base measurement values have been updated since last MRC */
    if (mrc_state.lastBaseCellvoltageTimestamp != pCellVoltageBase->header.timestamp) {
        baseCellvoltageUpdated = true;
    } else {
        baseCellvoltageUpdated = false;
    }

    /* ----------------- Check timestamp of redundant measurements --------- */
    if ((DATA_EntryUpdatedWithinInterval(pCellVoltageRedundancy0->header, MRC_AFE_MEASUREMENT_PERIOD_TIMEOUT_ms) ==
         false) &&
        (useCellvoltageRedundancy == true)) {
        redundancy0CellvoltageMeasurementTimeoutReached = true;
        /* Set error flag */
        (void)DIAG_Handler(DIAG_ID_REDUNDANCY0_CELL_VOLTAGE_MESUREMENT_TIMEOUT, DIAG_EVENT_NOT_OK, DIAG_SYSTEM, 0u);
    } else {
        redundancy0CellvoltageMeasurementTimeoutReached = false;
        (void)DIAG_Handler(DIAG_ID_REDUNDANCY0_CELL_VOLTAGE_MESUREMENT_TIMEOUT, DIAG_EVENT_OK, DIAG_SYSTEM, 0u);
    }

    /* ----------------- Validate cell voltages ---------------------------- */
    if (useCellvoltageRedundancy == true) {
        bool redundany0CellvoltageUpdated = false;
        /* Check if redundant measurement values have been updated since last MRC */
        if (mrc_state.lastRedundancy0CellvoltageTimestamp != pCellVoltageRedundancy0->header.timestamp) {
            redundany0CellvoltageUpdated = true;
        } else {
            redundany0CellvoltageUpdated = false;
        }
        /* Make sure cell voltage timestamps have been updated since last call */
        if ((baseCellvoltageUpdated == true) && (redundany0CellvoltageUpdated == true)) {
            /* Update timestamp */
            mrc_state.lastBaseCellvoltageTimestamp        = pCellVoltageBase->header.timestamp;
            mrc_state.lastRedundancy0CellvoltageTimestamp = pCellVoltageRedundancy0->header.timestamp;

            /* Validate cell voltages */
            MRC_ValidateCellVoltage(pCellVoltageBase, pCellVoltageRedundancy0, &mrc_tableCellVoltages);
            /* Set to true for following minimum, maximum and average calculation */
            updatedValidatedVoltageDatbaseEntry = true;

        } else if ((baseCellvoltageUpdated == true) || (redundany0CellvoltageUpdated == true)) {
            /* At least one measurement has been updated */
            if (baseCellvoltageUpdated == true) {
                /* Has redundant measurement timeout been reached? If yes, update. */
                if (redundancy0CellvoltageMeasurementTimeoutReached == true) {
                    /* Copy cell voltage base measurement values into validated database struct */
                    MRC_UpdateCellVoltageValidation(pCellVoltageBase, &mrc_tableCellVoltages);
                    /* Set to true for following minimum, maximum and average calculation */
                    updatedValidatedVoltageDatbaseEntry = true;
                }
            } else if (redundany0CellvoltageUpdated == true) {
                /* Has base measurement timeout been reached? If yes, update.  */
                if (baseCellvoltageMeasurementTimeoutReached == true) {
                    /* Copy cell voltage base measurement values into validated database struct */
                    MRC_UpdateCellVoltageValidation(pCellVoltageRedundancy0, &mrc_tableCellVoltages);
                    /* Set to true for following minimum, maximum and average calculation */
                    updatedValidatedVoltageDatbaseEntry = true;
                }
            } else {
                FAS_ASSERT(FAS_TRAP);
            }
        } else {
            /* No cell voltage measurement has been updated -> do nothing */
        }
    } else {
        if (baseCellvoltageUpdated == true) {
            /* Only update database entries if new raw data has been acquired */
            /* Update timestamp */
            mrc_state.lastBaseCellvoltageTimestamp = pCellVoltageBase->header.timestamp;

            /* Copy cell voltage base measurement values into validated database struct */
            MRC_UpdateCellVoltageValidation(pCellVoltageBase, &mrc_tableCellVoltages);

            /* Set to true for following minimum, maximum and average calculation */
            updatedValidatedVoltageDatbaseEntry = true;
        }
    }

    if (updatedValidatedVoltageDatbaseEntry == true) {
        /* Calculate min/max/average cell voltages */
        MRC_CalculateCellVoltageMinMaxAverage(&mrc_tableCellVoltages, &mrc_tableMinimumMaximumValues);

        /* Individual cell voltages validated and min/max/average calculated -> check voltage spread */
        if (STD_NOT_OK == PL_CheckVoltageSpread(&mrc_tableCellVoltages, &mrc_tableMinimumMaximumValues)) {
            /* Recalculate min/max/average cell voltages as at least one cell voltage has been detected as invalid */
            MRC_CalculateCellVoltageMinMaxAverage(&mrc_tableCellVoltages, &mrc_tableMinimumMaximumValues);
        }
    }

    return updatedValidatedVoltageDatbaseEntry;
}

static bool MRC_ValidateCellTemperatureMeasurement(
    DATA_BLOCK_CELL_TEMPERATURE_s *pCellTemperatureBase,
    DATA_BLOCK_CELL_TEMPERATURE_s *pCellTemperatureRedundancy0) {
    FAS_ASSERT(pCellTemperatureBase != NULL_PTR);
    FAS_ASSERT(pCellTemperatureRedundancy0 != NULL_PTR);

    bool baseCelltemperatureUpdated              = false;
    bool useCelltemperatureRedundancy            = false;
    bool updatedValidatedTemperatureDatbaseEntry = false;

    bool baseCellTemperatureMeasurementTimeoutReached        = true;
    bool redundancy0CellTemperatureMeasurementTimeoutReached = true;

    /* Following decision will be undertaken:
     *
     * - Use redundancy only if timestamp of redundant database entry != 0
     * - Perform redundant check if both timestamps have been updated
     * - Add timeout and trigger error if timestamps have not been updated in the last XXXXms
     * - If timeout reached and only one of the two redundant measurements has been updated use this measurement value
     *   but throw an error that one measurement has not been updated recently.
     */

    /* -------------- Check if cell cell temperature redundant measurement is used ---------- */
    /* Use redundant cell voltage measurements if measurement values have been acquired once */
    useCelltemperatureRedundancy = DATA_DatabaseEntryUpdatedAtLeastOnce(pCellTemperatureRedundancy0->header);

    /* ----------------- Check timestamp of base measurements--------------- */
    if (DATA_EntryUpdatedWithinInterval(pCellTemperatureBase->header, MRC_AFE_MEASUREMENT_PERIOD_TIMEOUT_ms) == true) {
        baseCellTemperatureMeasurementTimeoutReached = false;
        (void)DIAG_Handler(DIAG_ID_BASE_CELL_TEMPERATURE_MESUREMENT_TIMEOUT, DIAG_EVENT_OK, DIAG_SYSTEM, 0u);
    } else {
        baseCellTemperatureMeasurementTimeoutReached = true;
        /* Set error flag */
        (void)DIAG_Handler(DIAG_ID_BASE_CELL_TEMPERATURE_MESUREMENT_TIMEOUT, DIAG_EVENT_NOT_OK, DIAG_SYSTEM, 0u);
    }
    /* Check if base measurement values have been updated since last MRC */
    if (mrc_state.lastBaseCelltemperatureTimestamp != pCellTemperatureBase->header.timestamp) {
        baseCelltemperatureUpdated = true;
    } else {
        baseCelltemperatureUpdated = false;
    }

    /* ----------------- Check timestamp of redundant measurements --------- */
    if ((DATA_EntryUpdatedWithinInterval(pCellTemperatureRedundancy0->header, MRC_AFE_MEASUREMENT_PERIOD_TIMEOUT_ms) ==
         false) &&
        (useCelltemperatureRedundancy == true)) {
        redundancy0CellTemperatureMeasurementTimeoutReached = true;
        /* Set error flag */
        (void)DIAG_Handler(DIAG_ID_REDUNDANCY0_CELL_TEMPERATURE_MESUREMENT_TIMEOUT, DIAG_EVENT_NOT_OK, DIAG_SYSTEM, 0u);
    } else {
        redundancy0CellTemperatureMeasurementTimeoutReached = false;
        (void)DIAG_Handler(DIAG_ID_REDUNDANCY0_CELL_TEMPERATURE_MESUREMENT_TIMEOUT, DIAG_EVENT_OK, DIAG_SYSTEM, 0u);
    }

    /* ----------------- Validate cell temperatures ------------------------ */
    if (useCelltemperatureRedundancy == true) {
        bool redundancy0CelltemperatureUpdated = false;
        /* Check if redundant measurement values have been updated since last MRC */
        if (mrc_state.lastRedundancy0CelltemperatureTimestamp != pCellTemperatureRedundancy0->header.timestamp) {
            redundancy0CelltemperatureUpdated = true;
        } else {
            redundancy0CelltemperatureUpdated = false;
        }
        /* Make sure cell voltage timestamps have been updated since last call */
        if ((baseCelltemperatureUpdated == true) && (redundancy0CelltemperatureUpdated == true)) {
            /* Update timestamp */
            mrc_state.lastBaseCelltemperatureTimestamp        = pCellTemperatureBase->header.timestamp;
            mrc_state.lastRedundancy0CelltemperatureTimestamp = pCellTemperatureRedundancy0->header.timestamp;

            /* Validate cell temperatures */
            MRC_ValidateCellTemperature(pCellTemperatureBase, pCellTemperatureRedundancy0, &mrc_tableCellTemperatures);
            /* Set to true for following minimum, maximum and average calculation */
            updatedValidatedTemperatureDatbaseEntry = true;

        } else if ((baseCelltemperatureUpdated == true) || (redundancy0CelltemperatureUpdated == true)) {
            /* At least one measurement has been updated */
            if (baseCelltemperatureUpdated == true) {
                /* Has redundant measurement timeout been reached? If yes, update. */
                if (redundancy0CellTemperatureMeasurementTimeoutReached == true) {
                    /* Copy cell temperature base measurement values into validated database struct */
                    MRC_UpdateCellTemperatureValidation(pCellTemperatureBase, &mrc_tableCellTemperatures);
                    /* Set to true for following minimum, maximum and average calculation */
                    updatedValidatedTemperatureDatbaseEntry = true;
                }
            } else if (redundancy0CelltemperatureUpdated == true) {
                /* Has base measurement timeout been reached? If yes, update.  */
                if (baseCellTemperatureMeasurementTimeoutReached == true) {
                    /* Copy cell temperature redundant measurement values into validated database struct */
                    MRC_UpdateCellTemperatureValidation(pCellTemperatureRedundancy0, &mrc_tableCellTemperatures);
                    /* Set to true for following minimum, maximum and average calculation */
                    updatedValidatedTemperatureDatbaseEntry = true;
                }
            } else {
                FAS_ASSERT(FAS_TRAP);
            }
        } else {
            /* No cell temperature measurement has been updated -> do nothing */
        }
    } else { /* useCelltemperatureRedundancy == true */
        if (baseCelltemperatureUpdated == true) {
            /* Only update database entries if new raw data has been acquired */
            /* Update timestamp */
            mrc_state.lastBaseCelltemperatureTimestamp = pCellTemperatureBase->header.timestamp;

            /* Copy cell temperature base measurement values into validated database struct */
            MRC_UpdateCellTemperatureValidation(pCellTemperatureBase, &mrc_tableCellTemperatures);

            /* Set to true for following minimum, maximum and average calculation */
            updatedValidatedTemperatureDatbaseEntry = true;
        }
    }

    if (updatedValidatedTemperatureDatbaseEntry == true) {
        /* Calculate min/max/average cell temperatures */
        MRC_CalculateCellTemperatureMinMaxAverage(&mrc_tableCellTemperatures, &mrc_tableMinimumMaximumValues);

        /* Individual cell temperatures validated and min/max/average calculated -> check temperature spread */
        if (STD_NOT_OK == PL_CheckTemperatureSpread(&mrc_tableCellTemperatures, &mrc_tableMinimumMaximumValues)) {
            /* Recalculate min/max/average temperatures as at least one temperature has been detected as invalid */
            MRC_CalculateCellTemperatureMinMaxAverage(&mrc_tableCellTemperatures, &mrc_tableMinimumMaximumValues);
        }
    }

    return updatedValidatedTemperatureDatbaseEntry;
}

static void MRC_ValidateCurrentMeasurement(DATA_BLOCK_CURRENT_SENSOR_s *pTableCurrentSensor) {
    FAS_ASSERT(pTableCurrentSensor != NULL_PTR);
    int32_t packCurrent_mA = 0;

    /* Validate pack current. Will be invalidated if not all current measurement values are valid */
    mrc_tablePackValues.invalidPackCurrent = 0u;

    /* Iterate over all strings to calculate pack current */
    for (uint8_t stringNumber = 0u; stringNumber < BS_NR_OF_STRINGS; stringNumber++) {
        /* Check timestamp of measurement */
        STD_RETURN_TYPE_e noTimeout = MRC_MeasurementUpdatedRecently(
            pTableCurrentSensor->timestampCurrent[stringNumber],
            pTableCurrentSensor->previousTimestampCurrent[stringNumber],
            MRC_CURRENT_MEASUREMENT_PERIOD_TIMEOUT_ms);
        (void)DIAG_CheckEvent(noTimeout, DIAG_ID_CURRENT_MEASUREMENT_TIMEOUT, DIAG_STRING, stringNumber);

        if (STD_OK == noTimeout) {
            /* Check if current entry has been updated since last call */
            if (mrc_state.lastStringCurrentTimestamp[stringNumber] !=
                pTableCurrentSensor->timestampCurrent[stringNumber]) {
                mrc_state.lastStringCurrentTimestamp[stringNumber] =
                    pTableCurrentSensor->timestampCurrent[stringNumber];
                mrc_tablePackValues.stringCurrent_mA[stringNumber] = pTableCurrentSensor->current_mA[stringNumber];
                if (pTableCurrentSensor->invalidCurrentMeasurement[stringNumber] == 0u) {
                    /* String current measurement valid -> set valid flag */
                    mrc_tablePackValues.invalidStringCurrent[stringNumber] = 0u;
                    (void)DIAG_Handler(DIAG_ID_CURRENT_MEASUREMENT_ERROR, DIAG_EVENT_OK, DIAG_STRING, stringNumber);
                } else {
                    /* String current measurement invalid -> set invalid flag */
                    (void)DIAG_Handler(DIAG_ID_CURRENT_MEASUREMENT_ERROR, DIAG_EVENT_NOT_OK, DIAG_STRING, stringNumber);
                    mrc_tablePackValues.invalidStringCurrent[stringNumber] = 1u;
                }
            } else {
                /* Nothing to do. Measurement has not been updated but still within timeout */
            }
        } else {
            /* Measurement timeout reached -> set string current invalid */
            mrc_tablePackValues.invalidStringCurrent[stringNumber] = 1u;
        }

        if (0u == mrc_tablePackValues.invalidStringCurrent[stringNumber]) {
            packCurrent_mA += mrc_tablePackValues.stringCurrent_mA[stringNumber];
        } else {
            /* One string current is invalid -> pack current cannot be correct.
             * Set pack current invalid */
            mrc_tablePackValues.invalidPackCurrent = 1u;
        }
    }
    mrc_tablePackValues.packCurrent_mA = packCurrent_mA;
}

static void MRC_ValidateStringVoltageMeasurement(
    DATA_BLOCK_CURRENT_SENSOR_s *pTableCurrentSensor,
    DATA_BLOCK_CELL_VOLTAGE_s *pTableCellVoltage) {
    /* Pointer validity check */
    FAS_ASSERT(pTableCurrentSensor != NULL_PTR);
    FAS_ASSERT(pTableCellVoltage != NULL_PTR);

    for (uint8_t stringNumber = 0u; stringNumber < BS_NR_OF_STRINGS; stringNumber++) {
        /* Check timeout of current sensor measurement */
        STD_RETURN_TYPE_e updatedMeasurement = MRC_MeasurementUpdatedRecently(
            pTableCurrentSensor->timestampHighVoltage[stringNumber][0u],
            pTableCurrentSensor->previousTimestampHighVoltage[stringNumber][0u],
            MRC_CURRENT_SENSOR_MEASUREMENT_TIMEOUT_ms);
        DIAG_CheckEvent(updatedMeasurement, DIAG_ID_CURRENT_SENSOR_V1_MEASUREMENT_TIMEOUT, DIAG_STRING, stringNumber);

        /* Perform plausibility check if AFE and new current sensor measurement is valid */
        if ((STD_OK == updatedMeasurement) &&
            (pTableCurrentSensor->invalidHighVoltageMeasurement[stringNumber][0u] == 0u) &&
            (pTableCellVoltage->nrValidCellVoltages[stringNumber] == BS_NR_OF_BAT_CELLS)) {
            STD_RETURN_TYPE_e voltagePlausible = PL_CheckStringVoltage(
                pTableCellVoltage->packVoltage_mV[stringNumber], pTableCurrentSensor->highVoltage_mV[stringNumber][0u]);
            (void)DIAG_CheckEvent(voltagePlausible, DIAG_ID_PLAUSIBILITY_PACK_VOLTAGE, DIAG_STRING, stringNumber);

            /* Use current sensor measurement */ /* TODO: use really current sensor? Average of both? AFE measurement? */
            mrc_tablePackValues.stringVoltage_mV[stringNumber] = pTableCurrentSensor->highVoltage_mV[stringNumber][0u];

            if (STD_OK == voltagePlausible) {
                mrc_tablePackValues.invalidStringVoltage[stringNumber] = 0u;
            } else {
                mrc_tablePackValues.invalidStringVoltage[stringNumber] = 1u;
            }
        } else {
            /* Plausibility check cannot be performed if we do not have valid
             * values from AFE and current sensor measurement */
            (void)DIAG_CheckEvent(STD_NOT_OK, DIAG_ID_PLAUSIBILITY_PACK_VOLTAGE, DIAG_STRING, stringNumber);

            if ((STD_OK == updatedMeasurement) &&
                (pTableCurrentSensor->invalidHighVoltageMeasurement[stringNumber][0u] == 0u)) {
                /* Current sensor measurement valid -> use this measurement */
                mrc_tablePackValues.stringVoltage_mV[stringNumber] =
                    pTableCurrentSensor->highVoltage_mV[stringNumber][0u];
                mrc_tablePackValues.invalidStringVoltage[stringNumber] = 0u;
            } else if (pTableCellVoltage->nrValidCellVoltages[stringNumber] == BS_NR_OF_BAT_CELLS) {
                /* AFE measurement valid -> use this measurement */
                mrc_tablePackValues.stringVoltage_mV[stringNumber] = pTableCellVoltage->packVoltage_mV[stringNumber];
                mrc_tablePackValues.invalidStringVoltage[stringNumber] = 0u;
            } else {
                /* AFE and current sensor measurement invalid -> try to construct
                 * a valid from the number of valid cell voltages and substitute
                 * invalid cell voltages with the average cell voltage. */
                uint16_t numberInvalidCellVoltages =
                    (BS_NR_OF_BAT_CELLS - pTableCellVoltage->nrValidCellVoltages[stringNumber]);

                mrc_tablePackValues.stringVoltage_mV[stringNumber] =
                    pTableCellVoltage->packVoltage_mV[stringNumber] +
                    (mrc_tableMinimumMaximumValues.averageCellVoltage_mV[stringNumber] *
                     (int16_t)numberInvalidCellVoltages);

                /* Only use this as valid value if not more than five cell voltages are invalid */
                if (numberInvalidCellVoltages > MRC_ALLOWED_NUMBER_OF_INVALID_CELL_VOLTAGES) {
                    mrc_tablePackValues.invalidStringVoltage[stringNumber] = 1u;
                } else {
                    mrc_tablePackValues.invalidStringVoltage[stringNumber] = 0u;
                }
            }
        }
    }
}

static void MRC_ValidateBatteryVoltageMeasurement(void) {
    int64_t sumOfStringValues_mV       = 0;
    int8_t numberOfValidStringVoltages = 0;
    uint8_t numberOfConnectedStrings   = BMS_GetNumberOfConnectedStrings();

    if (0u != numberOfConnectedStrings) {
        /* Iterate over all strings to see which strings are connected */
        for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
            bool isStringConnected = BMS_IsStringClosed(s);
            if ((0u == mrc_tablePackValues.invalidStringVoltage[s]) && (true == isStringConnected)) {
                /* AXIVION Disable Style MisraC2012Directive-4.1: Values start with 0, iteration is less than UINT8_MAX; overflow impossible */
                sumOfStringValues_mV += mrc_tablePackValues.stringVoltage_mV[s];
                numberOfValidStringVoltages++;
                /* AXIVION Enable Style MisraC2012Directive-4.1: */
            }
        }
    } else {
        /* Take average of all strings if no strings are connected */
        for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
            if (0u == mrc_tablePackValues.invalidStringVoltage[s]) {
                /* AXIVION Disable Style MisraC2012Directive-4.1: Values start with 0, iteration is less than UINT8_MAX; overflow impossible */
                sumOfStringValues_mV += mrc_tablePackValues.stringVoltage_mV[s];
                numberOfValidStringVoltages++;
                /* AXIVION Enable Style MisraC2012Directive-4.1: */
            }
        }
    }

    /* Only calculate average if at least one string voltage is valid */
    if (0 != numberOfValidStringVoltages) {
        /* AXIVION Disable Style MisraC2012Directive-4.1: truncation impossible; we sum INT32 values x times and divide by x, resulting in INT32 */
        mrc_tablePackValues.batteryVoltage_mV = (int32_t)(sumOfStringValues_mV / numberOfValidStringVoltages);
        /* AXIVION Enable Style MisraC2012Directive-4.1: */
    } else {
        mrc_tablePackValues.batteryVoltage_mV = INT32_MAX;
    }
}

static void MRC_ValidateHighVoltageBusMeasurement(DATA_BLOCK_CURRENT_SENSOR_s *pTableCurrentSensor) {
    FAS_ASSERT(pTableCurrentSensor != NULL_PTR);

    int32_t sum_mV        = 0;
    uint8_t validVoltages = 0u;
    for (uint8_t stringNumber = 0u; stringNumber < BS_NR_OF_STRINGS; stringNumber++) {
        /* Check timeout of current sensor measurement */
        STD_RETURN_TYPE_e updatedMeasurement = MRC_MeasurementUpdatedRecently(
            pTableCurrentSensor->timestampHighVoltage[stringNumber][2u],
            pTableCurrentSensor->previousTimestampHighVoltage[stringNumber][2u],
            MRC_CURRENT_SENSOR_MEASUREMENT_TIMEOUT_ms);
        DIAG_CheckEvent(updatedMeasurement, DIAG_ID_CURRENT_SENSOR_V3_MEASUREMENT_TIMEOUT, DIAG_STRING, stringNumber);

        const bool stringClosed      = BMS_IsStringClosed(stringNumber);
        const bool stringPrecharging = BMS_IsStringPrecharging(stringNumber);
        if (((true == stringPrecharging) || (true == stringClosed)) && (STD_OK == updatedMeasurement)) {
            /* Only voltages of connected strings can be used */
            if (0u == pTableCurrentSensor->invalidHighVoltageMeasurement[stringNumber][2]) {
                /* Measured high voltage is valid */
                validVoltages++;
                sum_mV += pTableCurrentSensor->highVoltage_mV[stringNumber][2];
            }
        }
    }

    if (validVoltages > 0u) {
        mrc_tablePackValues.highVoltageBusVoltage_mV = (sum_mV / (int32_t)validVoltages);
        mrc_tablePackValues.invalidHvBusVoltage      = 0;
    } else {
        /* TODO: do we want to write special data if no valid values can be read? */
        mrc_tablePackValues.invalidHvBusVoltage = 1u;
    }
}

static void MRC_ValidatePowerMeasurement(DATA_BLOCK_CURRENT_SENSOR_s *pTableCurrentSensor) {
    FAS_ASSERT(pTableCurrentSensor != NULL_PTR);
    bool calculatePower = false;
    int32_t packPower_W = 0;

    /* Validate pack power. Will be invalidated if not all power measurement values are valid */
    mrc_tablePackValues.invalidPackPower = 0u;

    for (uint8_t stringNumber = 0u; stringNumber < BS_NR_OF_STRINGS; stringNumber++) {
        /* Check timeout of current sensor measurement */
        STD_RETURN_TYPE_e noTimeout = MRC_MeasurementUpdatedRecently(
            pTableCurrentSensor->timestampPower[stringNumber],
            pTableCurrentSensor->previousTimestampPower[stringNumber],
            MRC_CURRENT_SENSOR_MEASUREMENT_TIMEOUT_ms);
        DIAG_CheckEvent(noTimeout, DIAG_ID_CURRENT_SENSOR_POWER_MEASUREMENT_TIMEOUT, DIAG_STRING, stringNumber);

        if (STD_OK == noTimeout) {
            /* Check if current sensor measurement has been updated */
            if (pTableCurrentSensor->timestampPower[stringNumber] != mrc_state.lastStringPowerTimestamp[stringNumber]) {
                mrc_state.lastStringPowerTimestamp[stringNumber] = pTableCurrentSensor->timestampPower[stringNumber];
                /* Check if measurement is valid */
                if (pTableCurrentSensor->invalidPowerMeasurement[stringNumber] == 0u) {
                    mrc_tablePackValues.stringPower_W[stringNumber]      = pTableCurrentSensor->power_W[stringNumber];
                    mrc_tablePackValues.invalidStringPower[stringNumber] = 0u;
                } else {
                    /* Measurement has been updated but value is invalid -> calculate from current and string voltage */
                    calculatePower = true;
                    /* TODO: do we want to calculate values by hand if we are within time limit but value is invalid? */
                }
            } else {
                /* Nothing to do. Measurement has not been updated but still within timeout */
            }
        } else {
            /* Timeout reached. Set invalid flag */
            calculatePower                                       = true;
            mrc_tablePackValues.invalidStringPower[stringNumber] = 1u;
        }
        if ((true == calculatePower) && (0u == mrc_tablePackValues.invalidStringCurrent[stringNumber]) &&
            (0u == mrc_tablePackValues.invalidStringVoltage[stringNumber])) {
            /* Power measurement is invalid, but current and string voltage measurement are valid */
            int64_t stringPower_W =
                ((mrc_tablePackValues.stringCurrent_mA[stringNumber] *
                  mrc_tablePackValues.stringVoltage_mV[stringNumber]) /
                 (UNIT_CONVERSION_FACTOR_1000_INTEGER *  /* convert: mV -> V */
                  UNIT_CONVERSION_FACTOR_1000_INTEGER)); /* convert: mA -> A */
            mrc_tablePackValues.stringPower_W[stringNumber]      = (int32_t)stringPower_W;
            mrc_tablePackValues.invalidStringPower[stringNumber] = 0u;
        }
        if (0u == mrc_tablePackValues.invalidStringPower[stringNumber]) {
            packPower_W += mrc_tablePackValues.stringPower_W[stringNumber];
            DIAG_Handler(DIAG_ID_POWER_MEASUREMENT_ERROR, DIAG_EVENT_OK, DIAG_STRING, stringNumber);
        } else {
            /* One string power is invalid -> pack power cannot be correct.
             * Set pack power invalid */
            mrc_tablePackValues.invalidPackPower = 1u;
            DIAG_Handler(DIAG_ID_POWER_MEASUREMENT_ERROR, DIAG_EVENT_NOT_OK, DIAG_STRING, stringNumber);
        }
    }
    mrc_tablePackValues.packPower_W = packPower_W;
}

static STD_RETURN_TYPE_e MRC_CalculateCellVoltageMinMaxAverage(
    const DATA_BLOCK_CELL_VOLTAGE_s *const pValidatedVoltages,
    DATA_BLOCK_MIN_MAX_s *pMinMaxAverageValues) {
    /* Pointer validity check */
    FAS_ASSERT(pValidatedVoltages != NULL_PTR);
    FAS_ASSERT(pMinMaxAverageValues != NULL_PTR);

    STD_RETURN_TYPE_e retval = STD_OK;

    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        uint16_t nrValidCellvoltages = 0u;
        int16_t min                  = INT16_MAX;
        int16_t max                  = INT16_MIN;
        int32_t sum                  = 0;
        uint16_t moduleNumberMinimum = 0u;
        uint16_t cellNumberMinimum   = 0u;
        uint16_t moduleNumberMaximum = 0u;
        uint16_t cellNumberMaximum   = 0u;
        /* Iterate over all cells in each string */
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES; m++) {
            for (uint8_t c = 0u; c < BS_NR_OF_CELLS_PER_MODULE; c++) {
                if ((pValidatedVoltages->invalidCellVoltage[s][m] & (0x01u << c)) == 0u) {
                    /* Cell voltage is valid -> use this voltage for subsequent calculations */
                    nrValidCellvoltages++;
                    sum += pValidatedVoltages->cellVoltage_mV[s][(m * BS_NR_OF_CELLS_PER_MODULE) + c];

                    if (pValidatedVoltages->cellVoltage_mV[s][(m * BS_NR_OF_CELLS_PER_MODULE) + c] < min) {
                        min = pValidatedVoltages->cellVoltage_mV[s][(m * BS_NR_OF_CELLS_PER_MODULE) + c];
                        moduleNumberMinimum = m;
                        cellNumberMinimum   = c;
                    }
                    if (pValidatedVoltages->cellVoltage_mV[s][(m * BS_NR_OF_CELLS_PER_MODULE) + c] > max) {
                        max = pValidatedVoltages->cellVoltage_mV[s][(m * BS_NR_OF_CELLS_PER_MODULE) + c];
                        moduleNumberMaximum = m;
                        cellNumberMaximum   = c;
                    }
                }
            }
        }
        pMinMaxAverageValues->minimumCellVoltage_mV[s]      = min;
        pMinMaxAverageValues->nrCellMinimumCellVoltage[s]   = cellNumberMinimum;
        pMinMaxAverageValues->nrModuleMinimumCellVoltage[s] = moduleNumberMinimum;
        pMinMaxAverageValues->maximumCellVoltage_mV[s]      = max;
        pMinMaxAverageValues->nrCellMaximumCellVoltage[s]   = cellNumberMaximum;
        pMinMaxAverageValues->nrModuleMaximumCellVoltage[s] = moduleNumberMaximum;
        pMinMaxAverageValues->validMeasuredCellVoltages[s]  = nrValidCellvoltages;

        /* Prevent division by 0, if all cell voltages are invalid */
        if (nrValidCellvoltages > 0u) {
            pMinMaxAverageValues->averageCellVoltage_mV[s] = (int16_t)(sum / (int32_t)nrValidCellvoltages);
        } else {
            pMinMaxAverageValues->averageCellVoltage_mV[s] = 0;
            retval                                         = STD_NOT_OK;
        }
    }
    return retval;
}

static STD_RETURN_TYPE_e MRC_CalculateCellTemperatureMinMaxAverage(
    DATA_BLOCK_CELL_TEMPERATURE_s *pValidatedTemperatures,
    DATA_BLOCK_MIN_MAX_s *pMinMaxAverageValues) {
    /* Pointer validity check */
    FAS_ASSERT(pValidatedTemperatures != NULL_PTR);
    FAS_ASSERT(pMinMaxAverageValues != NULL_PTR);

    STD_RETURN_TYPE_e retval = STD_OK;

    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        uint16_t moduleNumberMinimum     = 0u;
        uint16_t sensorNumberMinimum     = 0u;
        uint16_t moduleNumberMaximum     = 0u;
        uint16_t sensorNumberMaximum     = 0u;
        uint16_t nrValidCelltemperatures = 0u;
        int16_t min                      = INT16_MAX;
        int16_t max                      = INT16_MIN;
        float sum_ddegC                  = 0.0f;

        for (uint8_t m = 0u; m < BS_NR_OF_MODULES; m++) {
            for (uint8_t c = 0u; c < BS_NR_OF_TEMP_SENSORS_PER_MODULE; c++) {
                if ((pValidatedTemperatures->invalidCellTemperature[s][m] & (0x01u << c)) == 0u) {
                    /* Cell temperature is valid -> use this voltage for subsequent calculations */
                    nrValidCelltemperatures++;
                    sum_ddegC += (float)pValidatedTemperatures
                                     ->cellTemperature_ddegC[s][(m * BS_NR_OF_TEMP_SENSORS_PER_MODULE) + c];

                    if (pValidatedTemperatures->cellTemperature_ddegC[s][(m * BS_NR_OF_TEMP_SENSORS_PER_MODULE) + c] <
                        min) {
                        min = pValidatedTemperatures
                                  ->cellTemperature_ddegC[s][(m * BS_NR_OF_TEMP_SENSORS_PER_MODULE) + c];
                        moduleNumberMinimum = m;
                        sensorNumberMinimum = c;
                    }
                    if (pValidatedTemperatures->cellTemperature_ddegC[s][(m * BS_NR_OF_TEMP_SENSORS_PER_MODULE) + c] >
                        max) {
                        max = pValidatedTemperatures
                                  ->cellTemperature_ddegC[s][(m * BS_NR_OF_TEMP_SENSORS_PER_MODULE) + c];
                        moduleNumberMaximum = m;
                        sensorNumberMaximum = c;
                    }
                }
            }
        }
        pMinMaxAverageValues->minimumTemperature_ddegC[s]      = min;
        pMinMaxAverageValues->nrSensorMinimumTemperature[s]    = sensorNumberMinimum;
        pMinMaxAverageValues->nrModuleMinimumTemperature[s]    = moduleNumberMinimum;
        pMinMaxAverageValues->maximumTemperature_ddegC[s]      = max;
        pMinMaxAverageValues->nrSensorMaximumTemperature[s]    = sensorNumberMaximum;
        pMinMaxAverageValues->nrModuleMaximumTemperature[s]    = moduleNumberMaximum;
        pMinMaxAverageValues->validMeasuredCellTemperatures[s] = nrValidCelltemperatures;

        /* Prevent division by 0, if all cell voltages are invalid */
        if (nrValidCelltemperatures > 0u) {
            pMinMaxAverageValues->averageTemperature_ddegC[s] = (sum_ddegC / (float)nrValidCelltemperatures);
        } else {
            pMinMaxAverageValues->averageTemperature_ddegC[s] = 0.0f;
            retval                                            = STD_NOT_OK;
        }
    }
    return retval;
}

static STD_RETURN_TYPE_e MRC_ValidateCellVoltage(
    DATA_BLOCK_CELL_VOLTAGE_s *pCellvoltageBase,
    DATA_BLOCK_CELL_VOLTAGE_s *pCellvoltageRedundancy0,
    DATA_BLOCK_CELL_VOLTAGE_s *pValidatedVoltages) {
    /* Pointer validity check */
    FAS_ASSERT(pCellvoltageBase != NULL_PTR);
    FAS_ASSERT(pCellvoltageRedundancy0 != NULL_PTR);
    FAS_ASSERT(pValidatedVoltages != NULL_PTR);

    uint16_t numberValidMeasurements              = 0u;
    STD_RETURN_TYPE_e noPlausibilityIssueDetected = STD_OK; /* Flag if implausable value detected */
    STD_RETURN_TYPE_e retval                      = STD_OK;

    /* Iterate over all cell measurements */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        int32_t sum = 0;
        for (uint8_t m = 0; m < BS_NR_OF_MODULES; m++) {
            for (uint8_t c = 0; c < BS_NR_OF_CELLS_PER_MODULE; c++) {
                if (((pCellvoltageBase->invalidCellVoltage[s][m] & (1u << c)) == 0u) &&
                    ((pCellvoltageRedundancy0->invalidCellVoltage[s][m] & (1u << c)) == 0u)) {
                    /* Check if cell voltage of base AND redundant measurement is valid -> do plausibility check */
                    if (STD_OK == PL_CheckCellvoltage(
                                      pCellvoltageBase->cellVoltage_mV[s][(m * BS_NR_OF_CELLS_PER_MODULE) + c],
                                      pCellvoltageRedundancy0->cellVoltage_mV[s][(m * BS_NR_OF_CELLS_PER_MODULE) + c],
                                      &pValidatedVoltages->cellVoltage_mV[s][(m * BS_NR_OF_CELLS_PER_MODULE) + c])) {
                        /* Clear valid flag */
                        pValidatedVoltages->invalidCellVoltage[s][m] = pValidatedVoltages->invalidCellVoltage[s][m] &
                                                                       (~(1u << c));
                        numberValidMeasurements++;
                        sum += pValidatedVoltages->cellVoltage_mV[s][(m * BS_NR_OF_CELLS_PER_MODULE) + c];
                    } else {
                        /* Set invalid flag */
                        noPlausibilityIssueDetected = STD_NOT_OK;
                        pValidatedVoltages->invalidCellVoltage[s][m] |= (1u << c);
                        /* Set return value to #STD_NOT_OK as not all cell voltages have a valid measurement value */
                        retval = STD_NOT_OK;
                    }
                } else if ((pCellvoltageBase->invalidCellVoltage[s][m] & (1u << c)) == 0u) {
                    /* Only base measurement value is valid -> use this voltage without further plausibility checks */
                    pValidatedVoltages->cellVoltage_mV[s][(m * BS_NR_OF_CELLS_PER_MODULE) + c] =
                        pCellvoltageBase->cellVoltage_mV[s][(m * BS_NR_OF_CELLS_PER_MODULE) + c];
                    /* Reset valid flag */
                    pValidatedVoltages->invalidCellVoltage[s][m] = pValidatedVoltages->invalidCellVoltage[s][m] &
                                                                   (~(1u << c));
                    numberValidMeasurements++;
                    sum += pValidatedVoltages->cellVoltage_mV[s][(m * BS_NR_OF_CELLS_PER_MODULE) + c];
                } else if ((pCellvoltageRedundancy0->invalidCellVoltage[s][m] & (1u << c)) == 0u) {
                    /* Only redundant measurement value is valid -> use this voltage without further plausibility checks */
                    pValidatedVoltages->cellVoltage_mV[s][(m * BS_NR_OF_CELLS_PER_MODULE) + c] =
                        pCellvoltageRedundancy0->cellVoltage_mV[s][(m * BS_NR_OF_CELLS_PER_MODULE) + c];
                    /* Reset valid flag */
                    pValidatedVoltages->invalidCellVoltage[s][m] = pValidatedVoltages->invalidCellVoltage[s][m] &
                                                                   (~(1u << c));
                    numberValidMeasurements++;
                    sum += pValidatedVoltages->cellVoltage_mV[s][(m * BS_NR_OF_CELLS_PER_MODULE) + c];
                } else {
                    /* Both, base and redundant measurement value are invalid */
                    /* Save average cell voltage value of base and redundant */
                    pValidatedVoltages->cellVoltage_mV[s][(m * BS_NR_OF_CELLS_PER_MODULE) + c] =
                        (pCellvoltageBase->cellVoltage_mV[s][(m * BS_NR_OF_CELLS_PER_MODULE) + c] +
                         pCellvoltageRedundancy0->cellVoltage_mV[s][(m * BS_NR_OF_CELLS_PER_MODULE) + c]) /
                        2u;
                    /* Set invalid flag */
                    pValidatedVoltages->invalidCellVoltage[s][m] |= (1u << c);
                    /* Set return value to #STD_NOT_OK as not all cell voltages have a valid measurement value */
                    retval = STD_NOT_OK;
                }
            }
        }
        pValidatedVoltages->nrValidCellVoltages[s] = numberValidMeasurements;
        pValidatedVoltages->packVoltage_mV[s]      = sum;
        numberValidMeasurements                    = 0u; /* Reset counter for next string */

        (void)DIAG_CheckEvent(noPlausibilityIssueDetected, DIAG_ID_PLAUSIBILITY_CELL_VOLTAGE, DIAG_STRING, s);
        noPlausibilityIssueDetected = STD_OK; /* Reset flag for next string */
    }
    return retval;
}

static STD_RETURN_TYPE_e MRC_UpdateCellVoltageValidation(
    DATA_BLOCK_CELL_VOLTAGE_s *pCellvoltage,
    DATA_BLOCK_CELL_VOLTAGE_s *pValidatedVoltages) {
    /* Pointer validity check */
    FAS_ASSERT(pCellvoltage != NULL_PTR);
    FAS_ASSERT(pValidatedVoltages != NULL_PTR);

    /* Save header struct to copy to correct database entry */
    DATA_BLOCK_HEADER_s tmpHeader = pValidatedVoltages->header;
    /* Copy whole database entry */
    *pValidatedVoltages = *pCellvoltage;
    /* Restore previous header */
    pValidatedVoltages->header = tmpHeader;

    return STD_OK;
}

static STD_RETURN_TYPE_e MRC_ValidateCellTemperature(
    DATA_BLOCK_CELL_TEMPERATURE_s *pCelltemperatureBase,
    DATA_BLOCK_CELL_TEMPERATURE_s *pCelltemperatureRedundancy0,
    DATA_BLOCK_CELL_TEMPERATURE_s *pValidatedTemperatures) {
    /* Pointer validity check */
    FAS_ASSERT(pCelltemperatureBase != NULL_PTR);
    FAS_ASSERT(pCelltemperatureRedundancy0 != NULL_PTR);
    FAS_ASSERT(pValidatedTemperatures != NULL_PTR);

    uint16_t numberValidMeasurements              = 0u;
    STD_RETURN_TYPE_e noPlausibilityIssueDetected = STD_OK; /* Flag if implausable value detected */
    STD_RETURN_TYPE_e retval                      = STD_OK;

    /* Iterate over all cell measurements */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES; m++) {
            for (uint8_t c = 0u; c < BS_NR_OF_TEMP_SENSORS_PER_MODULE; c++) {
                if (((pCelltemperatureBase->invalidCellTemperature[s][m] & (1u << c)) == 0u) &&
                    ((pCelltemperatureRedundancy0->invalidCellTemperature[s][m] & (1u << c)) == 0u)) {
                    /* Check if cell voltage of base AND redundant measurement is valid -> do plausibility check */
                    if (STD_OK ==
                        PL_CheckCelltemperature(
                            pCelltemperatureBase->cellTemperature_ddegC[s][(m * BS_NR_OF_TEMP_SENSORS_PER_MODULE) + c],
                            pCelltemperatureRedundancy0
                                ->cellTemperature_ddegC[s][(m * BS_NR_OF_TEMP_SENSORS_PER_MODULE) + c],
                            &pValidatedTemperatures
                                 ->cellTemperature_ddegC[s][(m * BS_NR_OF_TEMP_SENSORS_PER_MODULE) + c])) {
                        /* Clear valid flag */
                        pValidatedTemperatures->invalidCellTemperature[s][m] =
                            pValidatedTemperatures->invalidCellTemperature[s][m] & (~(1u << c));
                        numberValidMeasurements++;
                    } else {
                        /* Set invalid flag */
                        noPlausibilityIssueDetected = STD_NOT_OK;
                        pValidatedTemperatures->invalidCellTemperature[s][m] |= (1u << c);
                        /* Set return value to #STD_NOT_OK as not all cell temperatures have a valid measurement value */
                        retval = STD_NOT_OK;
                    }
                } else if ((pCelltemperatureBase->invalidCellTemperature[s][m] & (1u << c)) == 0u) {
                    /* Only base measurement value is valid -> use this temperature without further plausibility checks */
                    pValidatedTemperatures->cellTemperature_ddegC[s][(m * BS_NR_OF_TEMP_SENSORS_PER_MODULE) + c] =
                        pCelltemperatureBase->cellTemperature_ddegC[s][(m * BS_NR_OF_TEMP_SENSORS_PER_MODULE) + c];
                    /* Reset valid flag */
                    pValidatedTemperatures->invalidCellTemperature[s][m] =
                        pValidatedTemperatures->invalidCellTemperature[s][m] & (~(1u << c));
                    numberValidMeasurements++;
                } else if ((pCelltemperatureRedundancy0->invalidCellTemperature[s][m] & (1u << c)) == 0u) {
                    /* Only redundant measurement value is valid -> use this temperature without further plausibility checks */
                    pValidatedTemperatures->cellTemperature_ddegC[s][(m * BS_NR_OF_TEMP_SENSORS_PER_MODULE) + c] =
                        pCelltemperatureRedundancy0
                            ->cellTemperature_ddegC[s][(m * BS_NR_OF_TEMP_SENSORS_PER_MODULE) + c];
                    /* Reset valid flag */
                    pValidatedTemperatures->invalidCellTemperature[s][m] =
                        pValidatedTemperatures->invalidCellTemperature[s][m] & (~(1u << c));
                    numberValidMeasurements++;
                } else {
                    /* Both, base and redundant measurement value are invalid */
                    /* Save average cell voltage value of base and redundant */
                    pValidatedTemperatures->cellTemperature_ddegC[s][(m * BS_NR_OF_TEMP_SENSORS_PER_MODULE) + c] =
                        (pCelltemperatureBase->cellTemperature_ddegC[s][(m * BS_NR_OF_TEMP_SENSORS_PER_MODULE) + c] +
                         pCelltemperatureRedundancy0
                             ->cellTemperature_ddegC[s][(m * BS_NR_OF_TEMP_SENSORS_PER_MODULE) + c]) /
                        2u;
                    /* Set invalid flag */
                    pValidatedTemperatures->invalidCellTemperature[s][m] |= (1u << c);
                    /* Set return value to #STD_NOT_OK as not all cell temperatures have a valid measurement value */
                    retval = STD_NOT_OK;
                }
            }
        }
        pValidatedTemperatures->nrValidTemperatures[s] = numberValidMeasurements;
        numberValidMeasurements                        = 0u; /* Reset counter for next string */

        (void)DIAG_CheckEvent(noPlausibilityIssueDetected, DIAG_ID_PLAUSIBILITY_CELL_TEMP, DIAG_STRING, s);
        noPlausibilityIssueDetected = STD_OK; /* Reset flag for next string */
    }

    return retval;
}

static STD_RETURN_TYPE_e MRC_UpdateCellTemperatureValidation(
    DATA_BLOCK_CELL_TEMPERATURE_s *pCellTemperature,
    DATA_BLOCK_CELL_TEMPERATURE_s *pValidatedTemperature) {
    /* Pointer validity check */
    FAS_ASSERT(pCellTemperature != NULL_PTR);
    FAS_ASSERT(pValidatedTemperature != NULL_PTR);

    /* Save header struct to copy to correct database entry */
    DATA_BLOCK_HEADER_s tmpHeader = pValidatedTemperature->header;
    /* Copy whole database entry */
    *pValidatedTemperature = *pCellTemperature;
    /* Restore previous header */
    pValidatedTemperature->header = tmpHeader;
    return STD_OK;
}

/*========== Extern Function Implementations ================================*/
extern STD_RETURN_TYPE_e MRC_Initialize(void) {
    STD_RETURN_TYPE_e retval = STD_NOT_OK;
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES; m++) {
            /* Invalidate cell voltage values */
            mrc_tableCellVoltages.invalidCellVoltage[s][m] = 0xFFFFFFFFFFFFFFFFULL;
            mrc_tableCellVoltages.validModuleVoltage[s][m] = false;
            /* Invalidate cell temperature values */
            mrc_tableCellTemperatures.invalidCellTemperature[s][m] = 0xFFFF;
        }
        /* Invalidate string values */
        mrc_tablePackValues.invalidStringVoltage[s] = 0x01;
        mrc_tablePackValues.invalidStringCurrent[s] = 0x01;
        mrc_tablePackValues.invalidStringPower[s]   = 0x01;
    }
    /* Invalidate pack values */
    mrc_tablePackValues.invalidPackCurrent    = 0x01; /*!< bitmask if current is valid. 0->valid, 1->invalid */
    mrc_tablePackValues.invalidBatteryVoltage = 0x01; /*!< bitmask if voltage is valid. 0->valid, 1->invalid */
    mrc_tablePackValues.invalidHvBusVoltage   = 0x01; /*!< bitmask if voltage is valid. 0->valid, 1->invalid */
    mrc_tablePackValues.invalidPackPower      = 0x01; /*!< bitmask if power is valid. 0->valid, 1->invalid */

    retval = DATA_WRITE_DATA(&mrc_tableCellVoltages, &mrc_tableCellTemperatures, &mrc_tablePackValues);
    return retval;
}

extern STD_RETURN_TYPE_e MRC_ValidateAfeMeasurement(void) {
    STD_RETURN_TYPE_e retval = STD_OK;

    /* Database entries are declared static, so that they are placed in the data segment and not on the stack */
    static DATA_BLOCK_CELL_VOLTAGE_s mrc_tableCellvoltageBase = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE_BASE};
    static DATA_BLOCK_CELL_VOLTAGE_s mrc_tableCellvoltageRedundancy0 = {
        .header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE_REDUNDANCY0};

    static DATA_BLOCK_CELL_TEMPERATURE_s mrc_tableCelltemperatureBase = {
        .header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE_BASE};
    static DATA_BLOCK_CELL_TEMPERATURE_s mrc_tableCelltemperatureRedundancy0 = {
        .header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE_REDUNDANCY0};

    /* Get measurement values */
    DATA_READ_DATA(
        &mrc_tableCellvoltageBase,
        &mrc_tableCellvoltageRedundancy0,
        &mrc_tableCelltemperatureBase,
        &mrc_tableCelltemperatureRedundancy0);

    /* Perform validation of cell voltage measurement */
    bool updateCellVoltages =
        MRC_ValidateCellVoltageMeasurement(&mrc_tableCellvoltageBase, &mrc_tableCellvoltageRedundancy0);

    /* Perform validation of cell temperature measurement */
    bool updateCellTemperatures =
        MRC_ValidateCellTemperatureMeasurement(&mrc_tableCelltemperatureBase, &mrc_tableCelltemperatureRedundancy0);

    /* Update database entries if necessary */
    if ((updateCellVoltages == true) && (updateCellTemperatures == true)) {
        DATA_WRITE_DATA(&mrc_tableCellVoltages, &mrc_tableCellTemperatures, &mrc_tableMinimumMaximumValues);
    } else if (updateCellVoltages == true) {
        DATA_WRITE_DATA(&mrc_tableCellVoltages, &mrc_tableMinimumMaximumValues);
    } else if (updateCellTemperatures == true) {
        DATA_WRITE_DATA(&mrc_tableCellTemperatures, &mrc_tableMinimumMaximumValues);
    } else {
        retval = STD_NOT_OK;
    }
    return retval;
}

extern STD_RETURN_TYPE_e MRC_ValidatePackMeasurement(void) {
    STD_RETURN_TYPE_e retval                       = STD_OK;
    DATA_BLOCK_CURRENT_SENSOR_s tableCurrentSensor = {.header.uniqueId = DATA_BLOCK_ID_CURRENT_SENSOR};

    DATA_READ_DATA(&tableCurrentSensor);

    MRC_ValidateCurrentMeasurement(&tableCurrentSensor);
    MRC_ValidateStringVoltageMeasurement(&tableCurrentSensor, &mrc_tableCellVoltages);
    MRC_ValidateBatteryVoltageMeasurement();
    MRC_ValidateHighVoltageBusMeasurement(&tableCurrentSensor);
    MRC_ValidatePowerMeasurement(&tableCurrentSensor);

    DATA_WRITE_DATA(&mrc_tablePackValues);

    return retval;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
extern bool TEST_MRC_MeasurementUpdatedAtLeastOnce(uint32_t timestamp, uint32_t previousTimestamp) {
    return MRC_MeasurementUpdatedAtLeastOnce(timestamp, previousTimestamp);
}
extern STD_RETURN_TYPE_e TEST_MRC_MeasurementUpdatedRecently(
    uint32_t timestamp,
    uint32_t previousTimestamp,
    uint32_t timeInterval) {
    return MRC_MeasurementUpdatedRecently(timestamp, previousTimestamp, timeInterval);
}
extern bool TEST_MRC_ValidateCellVoltageMeasurement(
    DATA_BLOCK_CELL_VOLTAGE_s *pCellVoltageBase,
    DATA_BLOCK_CELL_VOLTAGE_s *pCellVoltageRedundancy0) {
    return MRC_ValidateCellVoltageMeasurement(pCellVoltageBase, pCellVoltageRedundancy0);
}
extern bool TEST_MRC_ValidateCellTemperatureMeasurement(
    DATA_BLOCK_CELL_TEMPERATURE_s *pCellTemperatureBase,
    DATA_BLOCK_CELL_TEMPERATURE_s *pCellTemperatureRedundancy0) {
    return MRC_ValidateCellTemperatureMeasurement(pCellTemperatureBase, pCellTemperatureRedundancy0);
}

extern void TEST_MRC_ValidateCurrentMeasurement(DATA_BLOCK_CURRENT_SENSOR_s *pTableCurrentSensor) {
    MRC_ValidateCurrentMeasurement(pTableCurrentSensor);
}

extern void TEST_MRC_ValidatePowerMeasurement(DATA_BLOCK_CURRENT_SENSOR_s *pTableCurrentSensor) {
    MRC_ValidatePowerMeasurement(pTableCurrentSensor);
}
extern void TEST_MRC_ValidateStringVoltageMeasurement(
    DATA_BLOCK_CURRENT_SENSOR_s *pTableCurrentSensor,
    DATA_BLOCK_CELL_VOLTAGE_s *pTableCellVoltage) {
    MRC_ValidateStringVoltageMeasurement(pTableCurrentSensor, pTableCellVoltage);
}
extern void TEST_MRC_ValidateBatteryVoltageMeasurement(void) {
    MRC_ValidateBatteryVoltageMeasurement();
}
extern void TEST_MRC_ValidateHighVoltageMeasurement(DATA_BLOCK_CURRENT_SENSOR_s *pTableCurrentSensor) {
    MRC_ValidateHighVoltageBusMeasurement(pTableCurrentSensor);
}
extern STD_RETURN_TYPE_e TEST_MRC_CalculateCellVoltageMinMaxAverage(
    DATA_BLOCK_CELL_VOLTAGE_s *pValidatedVoltages,
    DATA_BLOCK_MIN_MAX_s *pMinMaxAverageValues) {
    return MRC_CalculateCellVoltageMinMaxAverage(pValidatedVoltages, pMinMaxAverageValues);
}
extern STD_RETURN_TYPE_e TEST_MRC_CalculateCellTemperatureMinMaxAverage(
    DATA_BLOCK_CELL_TEMPERATURE_s *pValidatedTemperatures,
    DATA_BLOCK_MIN_MAX_s *pMinMaxAverageValues) {
    return MRC_CalculateCellTemperatureMinMaxAverage(pValidatedTemperatures, pMinMaxAverageValues);
}
extern STD_RETURN_TYPE_e TEST_MRC_ValidateCellVoltage(
    DATA_BLOCK_CELL_VOLTAGE_s *pCellvoltageBase,
    DATA_BLOCK_CELL_VOLTAGE_s *pCellvoltageRedundancy0,
    DATA_BLOCK_CELL_VOLTAGE_s *pValidatedVoltages) {
    return MRC_ValidateCellVoltage(pCellvoltageBase, pCellvoltageRedundancy0, pValidatedVoltages);
}
extern STD_RETURN_TYPE_e TEST_MRC_UpdateCellVoltageValidation(
    DATA_BLOCK_CELL_VOLTAGE_s *pCellvoltage,
    DATA_BLOCK_CELL_VOLTAGE_s *pValidatedVoltages) {
    return MRC_UpdateCellVoltageValidation(pCellvoltage, pValidatedVoltages);
}
extern STD_RETURN_TYPE_e TEST_MRC_ValidateCellTemperature(
    DATA_BLOCK_CELL_TEMPERATURE_s *pCelltemperatureBase,
    DATA_BLOCK_CELL_TEMPERATURE_s *pCelltemperatureRedundancy0,
    DATA_BLOCK_CELL_TEMPERATURE_s *pValidatedTemperatures) {
    return MRC_ValidateCellTemperature(pCelltemperatureBase, pCelltemperatureRedundancy0, pValidatedTemperatures);
}
extern STD_RETURN_TYPE_e TEST_MRC_UpdateCellTemperatureValidation(
    DATA_BLOCK_CELL_TEMPERATURE_s *pCellTemperature,
    DATA_BLOCK_CELL_TEMPERATURE_s *pValidatedTemperature) {
    return MRC_UpdateCellTemperatureValidation(pCellTemperature, pValidatedTemperature);
}
#endif
