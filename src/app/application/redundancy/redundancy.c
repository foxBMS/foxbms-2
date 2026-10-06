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
 * @file    redundancy.c
 * @author  foxBMS Team
 * @date    2020-07-31 (date of creation)
 * @updated 2026-10-06 (date of last update)
 * @version v1.12.0
 * @ingroup APPLICATION
 * @prefix  MRC
 *
 * @brief   Source file for handling redundancy between redundant cell voltage
 *          and cell temperature measurements
 * @details Paired measurements are checked for plausibility when both sources
 *          provide new data. If either source times out, the other source is
 *          copied as-is because no comparison is possible.
 */

/*========== Includes =======================================================*/
#include "redundancy.h"

#include "battery_system_cfg.h"
#include "database_cfg.h"
#include "redundancy_cfg.h"

#include "bms.h"
#include "database.h"
#include "diag.h"
#include "fassert.h"
#include "foxmath.h"
#include "fstd_types.h"
#include "os.h"
#include "plausibility.h"
#include "redundancy-validation.h"

#include <math.h>
#include <stdbool.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/
/**
 * This structure contains all the variables relevant for the redundancy state machine.
 */
typedef struct {
    uint32_t lastBaseCellVoltageTimestamp;
    uint32_t lastRedundancy0CellVoltageTimestamp;
    uint32_t lastBaseCellTemperatureTimestamp;
    uint32_t lastRedundancy0CellTemperatureTimestamp;
} MRC_STATE_s;

/*========== Static Constant and Variable Definitions =======================*/

/** state of the redundancy module */
static MRC_STATE_s mrc_state = {
    .lastBaseCellVoltageTimestamp            = 0u,
    .lastRedundancy0CellVoltageTimestamp     = 0u,
    .lastBaseCellTemperatureTimestamp        = 0u,
    .lastRedundancy0CellTemperatureTimestamp = 0u,

};

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/**
 * @brief Function to validate results of cell voltage measurement
 * @param[in] pTableCellVoltages       validated cell voltage measurement
 * @param[in] pCellVoltageBase         base cell voltage measurement
 * @param[in] pCellVoltageRedundancy0  redundant cell voltage measurement
 * @return true  if measurement has been validated successfully and
 *         database entry needs to be updated, otherwise false.
 */
static bool MRC_ValidateCellVoltageMeasurement(
    DATA_BLOCK_CELL_VOLTAGE_s *pTableCellVoltages,
    DATA_BLOCK_CELL_VOLTAGE_s *pCellVoltageBase,
    DATA_BLOCK_CELL_VOLTAGE_s *pCellVoltageRedundancy0);

/**
 * @brief Function to validate results of cell temperature measurement
 * @param[in] pTableCellTemperatures       validated cell temperature
 *                                         measurement
 * @param[in] pCellTemperatureBase         base cell temperature measurement
 * @param[in] pCellTemperatureRedundancy0  redundant cell temperature measurement
 * @return true  if measurement has been validated successfully and
 *         database entry needs to be updated, otherwise false.
 */
static bool MRC_ValidateCellTemperatureMeasurement(
    DATA_BLOCK_CELL_TEMPERATURE_s *pTableCellTemperatures,
    DATA_BLOCK_CELL_TEMPERATURE_s *pCellTemperatureBase,
    DATA_BLOCK_CELL_TEMPERATURE_s *pCellTemperatureRedundancy0);

/**
 * @brief Check whether a measurement timed out and report its diagnostic event
 * @param[in] tableHeader  database entry header of the measurement
 * @param[in] diagnosticId diagnostic ID for the measurement timeout
 * @return true if the measurement timed out, otherwise false
 */
static bool MRC_CheckMeasurementTimeout(DATA_BLOCK_HEADER_s tableHeader, DIAG_ID_e diagnosticId);

/**
 * @brief Function compares cell voltage measurements from base measurement with
 *        one redundant measurement and writes result in pValidatedVoltages.
 * @param[in] pCellVoltageBase         base cell voltage measurement
 * @param[in] pCellVoltageRedundancy0  redundant cell voltage measurement
 * @param[out] pValidatedVoltages      validated voltages from redundant measurement values
 * @return #STD_NOT_OK if not all cell voltages could be validated, otherwise
 *         #STD_OK
 */
static STD_RETURN_TYPE_e MRC_ValidateCellVoltage(
    DATA_BLOCK_CELL_VOLTAGE_s *pCellVoltageBase,
    DATA_BLOCK_CELL_VOLTAGE_s *pCellVoltageRedundancy0,
    DATA_BLOCK_CELL_VOLTAGE_s *pValidatedVoltages);

/**
 * @brief Function copies cell voltage data from the source that is still
 *        updating when its peer has timed out. The copied values cannot be
 *        cross-validated against the failed source.
 * @param[in] pCellVoltage         cell voltage measurement
 * @param[out] pValidatedVoltages  validated voltage values
 * @return #STD_NOT_OK if not all cell voltages could be validated, otherwise
 *         #STD_OK
 */
static STD_RETURN_TYPE_e MRC_UpdateCellVoltageValidation(
    DATA_BLOCK_CELL_VOLTAGE_s *pCellVoltage,
    DATA_BLOCK_CELL_VOLTAGE_s *pValidatedVoltages);

/**
 * @brief Function compares cell temperature measurements from base measurement
 *        with one redundant measurement and writes result in pValidatedTemperatures.
 * @param[in] pCellTemperatureBase         base cell temperature measurement
 * @param[in] pCellTemperatureRedundancy0  redundant cell temperature measurement
 * @param[out] pValidatedTemperatures      validated temperatures from redundant measurement values
 * @return #STD_NOT_OK if not all cell voltages could be validated, otherwise
 *         #STD_OK
 */
static STD_RETURN_TYPE_e MRC_ValidateCellTemperature(
    DATA_BLOCK_CELL_TEMPERATURE_s *pCellTemperatureBase,
    DATA_BLOCK_CELL_TEMPERATURE_s *pCellTemperatureRedundancy0,
    DATA_BLOCK_CELL_TEMPERATURE_s *pValidatedTemperatures);

/**
 * @brief Function copies cell temperature data from the source that is still
 *        updating when its peer has timed out. The copied values cannot be
 *        cross-validated against the failed source.
 * @param[in] pCellTemperature         cell temperature measurement
 * @param[out] pValidatedTemperature   validated temperature values
 * @return #STD_NOT_OK if not all cell voltages could be validated, otherwise
 *         #STD_OK
 */
static STD_RETURN_TYPE_e MRC_UpdateCellTemperatureValidation(
    DATA_BLOCK_CELL_TEMPERATURE_s *pCellTemperature,
    DATA_BLOCK_CELL_TEMPERATURE_s *pValidatedTemperature);

/*========== Static Function Implementations ================================*/
static bool MRC_CheckMeasurementTimeout(DATA_BLOCK_HEADER_s tableHeader, DIAG_ID_e diagnosticId) {
    const bool measurementTimeoutReached =
        (DATA_EntryUpdatedWithinInterval(tableHeader, MRC_AFE_MEASUREMENT_PERIOD_TIMEOUT_ms) == false);
    DIAG_EVENT_e diagnosticEvent = DIAG_EVENT_OK;
    if (measurementTimeoutReached) {
        diagnosticEvent = DIAG_EVENT_NOT_OK;
    }

    (void)DIAG_Handler(diagnosticId, diagnosticEvent, DIAG_SYSTEM, 0u);

    return measurementTimeoutReached;
}

static bool MRC_ValidateCellVoltageMeasurement(
    DATA_BLOCK_CELL_VOLTAGE_s *pTableCellVoltages,
    DATA_BLOCK_CELL_VOLTAGE_s *pCellVoltageBase,
    DATA_BLOCK_CELL_VOLTAGE_s *pCellVoltageRedundancy0) {
    FAS_ASSERT(pTableCellVoltages != NULL_PTR);
    FAS_ASSERT(pCellVoltageBase != NULL_PTR);
    FAS_ASSERT(pCellVoltageRedundancy0 != NULL_PTR);

    bool updatedValidatedVoltageDatabaseEntry = false;

    const bool baseCellVoltageMeasurementTimeoutReached =
        MRC_CheckMeasurementTimeout(pCellVoltageBase->header, DIAG_ID_BASE_CELL_VOLTAGE_MEASUREMENT_TIMEOUT);
    const bool redundancy0CellVoltageMeasurementTimeoutReached = MRC_CheckMeasurementTimeout(
        pCellVoltageRedundancy0->header, DIAG_ID_REDUNDANCY0_CELL_VOLTAGE_MEASUREMENT_TIMEOUT);
    const bool baseCellVoltageUpdated = (mrc_state.lastBaseCellVoltageTimestamp != pCellVoltageBase->header.timestamp);
    const bool redundancy0CellVoltageUpdated =
        (mrc_state.lastRedundancy0CellVoltageTimestamp != pCellVoltageRedundancy0->header.timestamp);

    /* ----------------- Validate cell voltages ---------------------------- */

    /*
     * Compare sources only after both timestamps advance. If just one advances,
     * keep that sample pending rather than compare it with stale data. The
     * timeout branch below handles a peer that has stopped supplying samples.
     */
    if (baseCellVoltageUpdated && redundancy0CellVoltageUpdated) {
        /* Update timestamp */
        mrc_state.lastBaseCellVoltageTimestamp        = pCellVoltageBase->header.timestamp;
        mrc_state.lastRedundancy0CellVoltageTimestamp = pCellVoltageRedundancy0->header.timestamp;

        /* Validate cell voltages */
        MRC_ValidateCellVoltage(pCellVoltageBase, pCellVoltageRedundancy0, pTableCellVoltages);
        /* Set to true for following minimum, maximum and average calculation */
        updatedValidatedVoltageDatabaseEntry = true;
    } else if (baseCellVoltageUpdated || redundancy0CellVoltageUpdated) {
        /*
         * A single fresh sample is used only after its peer times out. This
         * avoids treating normal timing skew as a failure; once a timeout is
         * confirmed, the available sample is copied without plausibility
         * validation because there is no second current value to compare.
         */
        DATA_BLOCK_CELL_VOLTAGE_s *pUpdatedCellVoltages = NULL_PTR;
        if (baseCellVoltageUpdated && redundancy0CellVoltageMeasurementTimeoutReached) {
            pUpdatedCellVoltages = pCellVoltageBase;
        } else if (redundancy0CellVoltageUpdated && baseCellVoltageMeasurementTimeoutReached) {
            pUpdatedCellVoltages = pCellVoltageRedundancy0;
        }
        if (pUpdatedCellVoltages != NULL_PTR) {
            MRC_UpdateCellVoltageValidation(pUpdatedCellVoltages, pTableCellVoltages);
            updatedValidatedVoltageDatabaseEntry = true;
        }
    } else {
        /* No cell voltage measurement has been updated -> do nothing */
    }

    return updatedValidatedVoltageDatabaseEntry;
}

static bool MRC_ValidateCellTemperatureMeasurement(
    DATA_BLOCK_CELL_TEMPERATURE_s *pTableCellTemperatures,
    DATA_BLOCK_CELL_TEMPERATURE_s *pCellTemperatureBase,
    DATA_BLOCK_CELL_TEMPERATURE_s *pCellTemperatureRedundancy0) {
    FAS_ASSERT(pTableCellTemperatures != NULL_PTR);
    FAS_ASSERT(pCellTemperatureBase != NULL_PTR);
    FAS_ASSERT(pCellTemperatureRedundancy0 != NULL_PTR);

    bool updatedValidatedTemperatureDatabaseEntry = false;

    const bool baseCellTemperatureMeasurementTimeoutReached =
        MRC_CheckMeasurementTimeout(pCellTemperatureBase->header, DIAG_ID_BASE_CELL_TEMPERATURE_MEASUREMENT_TIMEOUT);
    const bool redundancy0CellTemperatureMeasurementTimeoutReached = MRC_CheckMeasurementTimeout(
        pCellTemperatureRedundancy0->header, DIAG_ID_REDUNDANCY0_CELL_TEMPERATURE_MEASUREMENT_TIMEOUT);
    const bool baseCellTemperatureUpdated =
        (mrc_state.lastBaseCellTemperatureTimestamp != pCellTemperatureBase->header.timestamp);
    const bool redundancy0CellTemperatureUpdated =
        (mrc_state.lastRedundancy0CellTemperatureTimestamp != pCellTemperatureRedundancy0->header.timestamp);

    /* ----------------- Validate cell temperatures ------------------------ */
    /*
     * Compare sources only after both timestamps advance. If just one advances,
     * keep that sample pending rather than compare it with stale data. The
     * timeout branch below handles a peer that has stopped supplying samples.
     */
    if (baseCellTemperatureUpdated && redundancy0CellTemperatureUpdated) {
        /* Update timestamp */
        mrc_state.lastBaseCellTemperatureTimestamp        = pCellTemperatureBase->header.timestamp;
        mrc_state.lastRedundancy0CellTemperatureTimestamp = pCellTemperatureRedundancy0->header.timestamp;

        /* Validate cell temperatures */
        MRC_ValidateCellTemperature(pCellTemperatureBase, pCellTemperatureRedundancy0, pTableCellTemperatures);
        /* Set to true for following minimum, maximum and average calculation */
        updatedValidatedTemperatureDatabaseEntry = true;
    } else if (baseCellTemperatureUpdated || redundancy0CellTemperatureUpdated) {
        /*
         * A single fresh sample is used only after its peer times out. This
         * avoids treating normal timing skew as a failure; once a timeout is
         * confirmed, the available sample is copied without plausibility
         * validation because there is no second current value to compare.
         */
        DATA_BLOCK_CELL_TEMPERATURE_s *pUpdatedCellTemperatures = NULL_PTR;
        if (baseCellTemperatureUpdated && redundancy0CellTemperatureMeasurementTimeoutReached) {
            pUpdatedCellTemperatures = pCellTemperatureBase;
        } else if (redundancy0CellTemperatureUpdated && baseCellTemperatureMeasurementTimeoutReached) {
            pUpdatedCellTemperatures = pCellTemperatureRedundancy0;
        }
        if (pUpdatedCellTemperatures != NULL_PTR) {
            MRC_UpdateCellTemperatureValidation(pUpdatedCellTemperatures, pTableCellTemperatures);
            updatedValidatedTemperatureDatabaseEntry = true;
        }
    } else {
        /* No cell temperature measurement has been updated -> do nothing */
    }

    return updatedValidatedTemperatureDatabaseEntry;
}

static STD_RETURN_TYPE_e MRC_ValidateCellVoltage(
    DATA_BLOCK_CELL_VOLTAGE_s *pCellVoltageBase,
    DATA_BLOCK_CELL_VOLTAGE_s *pCellVoltageRedundancy0,
    DATA_BLOCK_CELL_VOLTAGE_s *pValidatedVoltages) {
    FAS_ASSERT(pCellVoltageBase != NULL_PTR);
    FAS_ASSERT(pCellVoltageRedundancy0 != NULL_PTR);
    FAS_ASSERT(pValidatedVoltages != NULL_PTR);

    uint16_t numberValidMeasurements              = 0u;
    STD_RETURN_TYPE_e noPlausibilityIssueDetected = STD_OK; /* Flag if implausible value detected */
    STD_RETURN_TYPE_e retval                      = STD_OK;

    /* Iterate over all cell measurements */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        int32_t sum = 0;
        for (uint8_t m = 0; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t cb = 0; cb < BS_NR_OF_CELL_BLOCKS_PER_MODULE; cb++) {
                if ((pCellVoltageBase->invalidCellVoltage[s][m][cb] == false) &&
                    (pCellVoltageRedundancy0->invalidCellVoltage[s][m][cb] == false)) {
                    /* Check if cell voltage of base AND redundant measurement is valid -> do plausibility check */
                    if (STD_OK == MCR_CheckCellVoltage(
                                      pCellVoltageBase->cellVoltage_mV[s][m][cb],
                                      pCellVoltageRedundancy0->cellVoltage_mV[s][m][cb],
                                      &pValidatedVoltages->cellVoltage_mV[s][m][cb])) {
                        /* Clear valid flag */
                        pValidatedVoltages->invalidCellVoltage[s][m][cb] = false;
                        numberValidMeasurements++;
                        sum += pValidatedVoltages->cellVoltage_mV[s][m][cb];
                    } else {
                        /* Set invalid flag */
                        noPlausibilityIssueDetected                      = STD_NOT_OK;
                        pValidatedVoltages->invalidCellVoltage[s][m][cb] = true;
                        /* Keep the helper's average for diagnostics, but mark it unusable. */
                        /* Set return value to #STD_NOT_OK as not all cell voltages have a valid measurement value */
                        retval = STD_NOT_OK;
                    }
                } else if (pCellVoltageBase->invalidCellVoltage[s][m][cb] == false) {
                    /* Only base measurement value is valid -> use this voltage without further plausibility checks */
                    pValidatedVoltages->cellVoltage_mV[s][m][cb] = pCellVoltageBase->cellVoltage_mV[s][m][cb];
                    /* Reset valid flag */
                    pValidatedVoltages->invalidCellVoltage[s][m][cb] = false;
                    numberValidMeasurements++;
                    sum += pValidatedVoltages->cellVoltage_mV[s][m][cb];
                } else if (pCellVoltageRedundancy0->invalidCellVoltage[s][m][cb] == false) {
                    /* Only redundant measurement value is valid -> use this voltage without further plausibility checks
                     */
                    pValidatedVoltages->cellVoltage_mV[s][m][cb] = pCellVoltageRedundancy0->cellVoltage_mV[s][m][cb];
                    /* Reset valid flag */
                    pValidatedVoltages->invalidCellVoltage[s][m][cb] = false;
                    numberValidMeasurements++;
                    sum += pValidatedVoltages->cellVoltage_mV[s][m][cb];
                } else {
                    /* Both, base and redundant measurement value are invalid */
                    /* Save average cell voltage value of base and redundant */
                    pValidatedVoltages->cellVoltage_mV[s][m][cb] = (pCellVoltageBase->cellVoltage_mV[s][m][cb] +
                                                                    pCellVoltageRedundancy0->cellVoltage_mV[s][m][cb]) /
                                                                   2;
                    /* Set invalid flag */
                    pValidatedVoltages->invalidCellVoltage[s][m][cb] = true;
                    /* Set return value to #STD_NOT_OK as not all cell voltages have a valid measurement value */
                    retval = STD_NOT_OK;
                }
            }
        }
        pValidatedVoltages->nrValidCellVoltages[s] = numberValidMeasurements;
        pValidatedVoltages->stringVoltage_mV[s]    = sum;
        numberValidMeasurements                    = 0u; /* Reset counter for next string */

        (void)DIAG_ReportResultToHandler(noPlausibilityIssueDetected, DIAG_ID_REDUNDANCY_CELL_VOLTAGE, DIAG_STRING, s);
        noPlausibilityIssueDetected = STD_OK; /* Reset flag for next string */
    }
    return retval;
}

static STD_RETURN_TYPE_e MRC_UpdateCellVoltageValidation(
    DATA_BLOCK_CELL_VOLTAGE_s *pCellVoltage,
    DATA_BLOCK_CELL_VOLTAGE_s *pValidatedVoltages) {
    FAS_ASSERT(pCellVoltage != NULL_PTR);
    FAS_ASSERT(pValidatedVoltages != NULL_PTR);

    /* Preserve the destination header while replacing the measurement payload. */
    DATA_BLOCK_HEADER_s tmpHeader = pValidatedVoltages->header;
    *pValidatedVoltages           = *pCellVoltage; /* Copy whole database entry */
    pValidatedVoltages->header    = tmpHeader;     /* Restore previous header */

    return STD_OK;
}

static STD_RETURN_TYPE_e MRC_ValidateCellTemperature(
    DATA_BLOCK_CELL_TEMPERATURE_s *pCellTemperatureBase,
    DATA_BLOCK_CELL_TEMPERATURE_s *pCellTemperatureRedundancy0,
    DATA_BLOCK_CELL_TEMPERATURE_s *pValidatedTemperatures) {
    FAS_ASSERT(pCellTemperatureBase != NULL_PTR);
    FAS_ASSERT(pCellTemperatureRedundancy0 != NULL_PTR);
    FAS_ASSERT(pValidatedTemperatures != NULL_PTR);

    uint16_t numberValidMeasurements              = 0u;
    STD_RETURN_TYPE_e noPlausibilityIssueDetected = STD_OK; /* Flag if implausible value detected */
    STD_RETURN_TYPE_e retval                      = STD_OK;

    /* Iterate over all cell measurements */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t ts = 0u; ts < BS_NR_OF_TEMP_SENSORS_PER_MODULE; ts++) {
                if ((pCellTemperatureBase->invalidCellTemperature[s][m][ts] == false) &&
                    (pCellTemperatureRedundancy0->invalidCellTemperature[s][m][ts] == false)) {
                    /* Check if cell voltage of base AND redundant measurement is valid -> do plausibility check */
                    if (STD_OK == MCR_CheckCellTemperature(
                                      pCellTemperatureBase->cellTemperature_ddegC[s][m][ts],
                                      pCellTemperatureRedundancy0->cellTemperature_ddegC[s][m][ts],
                                      &pValidatedTemperatures->cellTemperature_ddegC[s][m][ts])) {
                        /* Reset invalid flag */
                        pValidatedTemperatures->invalidCellTemperature[s][m][ts] = false;
                        numberValidMeasurements++;
                    } else {
                        /* Set invalid flag */
                        noPlausibilityIssueDetected                              = STD_NOT_OK;
                        pValidatedTemperatures->invalidCellTemperature[s][m][ts] = true;
                        /* Keep the helper's average for diagnostics, but mark it unusable. */
                        /* Set return value to #STD_NOT_OK as not all cell temperatures have a valid measurement value
                         */
                        retval = STD_NOT_OK;
                    }
                } else if (pCellTemperatureBase->invalidCellTemperature[s][m][ts] == false) {
                    /* Only base measurement value is valid -> use this temperature without further plausibility checks
                     */
                    pValidatedTemperatures->cellTemperature_ddegC[s][m][ts] =
                        pCellTemperatureBase->cellTemperature_ddegC[s][m][ts];
                    /* Reset invalid flag */
                    pValidatedTemperatures->invalidCellTemperature[s][m][ts] = false;
                    numberValidMeasurements++;
                } else if (pCellTemperatureRedundancy0->invalidCellTemperature[s][m][ts] == false) {
                    /* Only redundant measurement value is valid -> use this temperature without further plausibility
                     * checks */
                    pValidatedTemperatures->cellTemperature_ddegC[s][m][ts] =
                        pCellTemperatureRedundancy0->cellTemperature_ddegC[s][m][ts];
                    /* Reset invalid flag */
                    pValidatedTemperatures->invalidCellTemperature[s][m][ts] = false;
                    numberValidMeasurements++;
                } else {
                    /* Both, base and redundant measurement value are invalid */
                    /* Save average cell voltage value of base and redundant */
                    pValidatedTemperatures->cellTemperature_ddegC[s][m][ts] =
                        (pCellTemperatureBase->cellTemperature_ddegC[s][m][ts] +
                         pCellTemperatureRedundancy0->cellTemperature_ddegC[s][m][ts]) /
                        2u;
                    /* Set invalid flag */
                    pValidatedTemperatures->invalidCellTemperature[s][m][ts] = true;
                    /* Set return value to #STD_NOT_OK as not all cell temperatures have a valid measurement value */
                    retval = STD_NOT_OK;
                }
            }
        }
        pValidatedTemperatures->nrValidTemperatures[s] = numberValidMeasurements;
        numberValidMeasurements                        = 0u; /* Reset counter for next string */

        (void)DIAG_ReportResultToHandler(
            noPlausibilityIssueDetected, DIAG_ID_REDUNDANCY_CELL_TEMPERATURE, DIAG_STRING, s);
        noPlausibilityIssueDetected = STD_OK; /* Reset flag for next string */
    }

    return retval;
}

static STD_RETURN_TYPE_e MRC_UpdateCellTemperatureValidation(
    DATA_BLOCK_CELL_TEMPERATURE_s *pCellTemperature,
    DATA_BLOCK_CELL_TEMPERATURE_s *pValidatedTemperature) {
    FAS_ASSERT(pCellTemperature != NULL_PTR);
    FAS_ASSERT(pValidatedTemperature != NULL_PTR);

    /* Preserve the destination header while replacing the measurement payload. */
    DATA_BLOCK_HEADER_s tmpHeader = pValidatedTemperature->header;
    *pValidatedTemperature        = *pCellTemperature; /* Copy whole database entry */
    pValidatedTemperature->header = tmpHeader;         /* Restore previous header */
    return STD_OK;
}

/*========== Extern Function Implementations ================================*/
extern MRC_REQUIRED_UPDATES_s MRC_ValidateAfeMeasurement(
    DATA_BLOCK_CELL_VOLTAGE_s *pTableCellVoltages,
    DATA_BLOCK_CELL_TEMPERATURE_s *pTableCellTemperatures) {
    FAS_ASSERT(pTableCellVoltages != NULL_PTR);
    FAS_ASSERT(pTableCellTemperatures != NULL_PTR);

    static MRC_REQUIRED_UPDATES_s retval = {
        .updatedCellVoltageDatabaseEntry = false,
        .updatedTemperatureDatabaseEntry = false,
    };
    retval.updatedCellVoltageDatabaseEntry = false;
    retval.updatedTemperatureDatabaseEntry = false;

    /* Database entries are declared static, so that they are placed in the data segment and not on the stack */
    static DATA_BLOCK_CELL_VOLTAGE_s mrc_tableCellVoltageBase = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE_BASE};
    static DATA_BLOCK_CELL_VOLTAGE_s mrc_tableCellVoltageRedundancy0 = {
        .header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE_REDUNDANCY0};

    static DATA_BLOCK_CELL_TEMPERATURE_s mrc_tableCellTemperatureBase = {
        .header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE_BASE};
    static DATA_BLOCK_CELL_TEMPERATURE_s mrc_tableCellTemperatureRedundancy0 = {
        .header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE_REDUNDANCY0};

    /* Get measurement values */
    DATA_READ_DATA(
        &mrc_tableCellVoltageBase,
        &mrc_tableCellVoltageRedundancy0,
        &mrc_tableCellTemperatureBase,
        &mrc_tableCellTemperatureRedundancy0);

    /* Perform validation of cell voltage measurement */
    bool updateCellVoltages = MRC_ValidateCellVoltageMeasurement(
        pTableCellVoltages, &mrc_tableCellVoltageBase, &mrc_tableCellVoltageRedundancy0);

    /* Perform validation of cell temperature measurement */
    bool updateCellTemperatures = MRC_ValidateCellTemperatureMeasurement(
        pTableCellTemperatures, &mrc_tableCellTemperatureBase, &mrc_tableCellTemperatureRedundancy0);

    /* Update database entries if necessary */
    if ((updateCellVoltages == true) && (updateCellTemperatures == true)) {
        retval.updatedCellVoltageDatabaseEntry = true;
        retval.updatedTemperatureDatabaseEntry = true;
    } else if (updateCellVoltages == true) {
        retval.updatedCellVoltageDatabaseEntry = true;
    } else if (updateCellTemperatures == true) {
        retval.updatedTemperatureDatabaseEntry = true;
    }
    return retval;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
extern bool TEST_MRC_ValidateCellVoltageMeasurement(
    DATA_BLOCK_CELL_VOLTAGE_s *pTableCellVoltages,
    DATA_BLOCK_CELL_VOLTAGE_s *pCellVoltageBase,
    DATA_BLOCK_CELL_VOLTAGE_s *pCellVoltageRedundancy0) {
    return MRC_ValidateCellVoltageMeasurement(pTableCellVoltages, pCellVoltageBase, pCellVoltageRedundancy0);
}
extern bool TEST_MRC_ValidateCellTemperatureMeasurement(
    DATA_BLOCK_CELL_TEMPERATURE_s *pTableCellTemperatures,
    DATA_BLOCK_CELL_TEMPERATURE_s *pCellTemperatureBase,
    DATA_BLOCK_CELL_TEMPERATURE_s *pCellTemperatureRedundancy0) {
    return MRC_ValidateCellTemperatureMeasurement(
        pTableCellTemperatures, pCellTemperatureBase, pCellTemperatureRedundancy0);
}
extern STD_RETURN_TYPE_e TEST_MRC_ValidateCellVoltage(
    DATA_BLOCK_CELL_VOLTAGE_s *pCellVoltageBase,
    DATA_BLOCK_CELL_VOLTAGE_s *pCellVoltageRedundancy0,
    DATA_BLOCK_CELL_VOLTAGE_s *pValidatedVoltages) {
    return MRC_ValidateCellVoltage(pCellVoltageBase, pCellVoltageRedundancy0, pValidatedVoltages);
}
extern STD_RETURN_TYPE_e TEST_MRC_UpdateCellVoltageValidation(
    DATA_BLOCK_CELL_VOLTAGE_s *pCellVoltage,
    DATA_BLOCK_CELL_VOLTAGE_s *pValidatedVoltages) {
    return MRC_UpdateCellVoltageValidation(pCellVoltage, pValidatedVoltages);
}
extern STD_RETURN_TYPE_e TEST_MRC_ValidateCellTemperature(
    DATA_BLOCK_CELL_TEMPERATURE_s *pCellTemperatureBase,
    DATA_BLOCK_CELL_TEMPERATURE_s *pCellTemperatureRedundancy0,
    DATA_BLOCK_CELL_TEMPERATURE_s *pValidatedTemperatures) {
    return MRC_ValidateCellTemperature(pCellTemperatureBase, pCellTemperatureRedundancy0, pValidatedTemperatures);
}
extern STD_RETURN_TYPE_e TEST_MRC_UpdateCellTemperatureValidation(
    DATA_BLOCK_CELL_TEMPERATURE_s *pCellTemperature,
    DATA_BLOCK_CELL_TEMPERATURE_s *pValidatedTemperature) {
    return MRC_UpdateCellTemperatureValidation(pCellTemperature, pValidatedTemperature);
}
extern void TEST_MRC_SetMrcState(
    uint32_t lastBaseCellVoltageTimestamp,
    uint32_t lastRedundancy0CellVoltageTimestamp,
    uint32_t lastBaseCellTemperatureTimestamp,
    uint32_t lastRedundancy0CellTemperatureTimestamp) {
    /** state of the redundancy module */
    mrc_state.lastBaseCellVoltageTimestamp            = lastBaseCellVoltageTimestamp;
    mrc_state.lastRedundancy0CellVoltageTimestamp     = lastRedundancy0CellVoltageTimestamp;
    mrc_state.lastBaseCellTemperatureTimestamp        = lastBaseCellTemperatureTimestamp;
    mrc_state.lastRedundancy0CellTemperatureTimestamp = lastRedundancy0CellTemperatureTimestamp;
}
#endif /* UNITY_UNIT_TEST */
