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
 * @file    bms-values.c
 * @author  foxBMS Team
 * @date    2024-07-23 (date of creation)
 * @updated 2026-10-06 (date of last update)
 * @version v1.12.0
 * @ingroup APPLICATION
 * @prefix  BMSVL
 *
 * @brief   BMS values management
 * @details Handle updating and management of system-wide
 *          battery values.
 *          If this module is compiled *with* redundant voltage and temperature
 *          measurement support, it will call the redundancy module to validate
 *          the measurements against each other and then use the plausibility
 *          module to check the plausibility of the measurements.
 *          If it is compiled *without* redundant voltage and temperature
 *          measurement support, it will directly use the plausibility module
 *          to check the plausibility of the measurements.
 */

/*========== Includes =======================================================*/
#include "foxbms_config_redundant_v_t_measurement.h"

#if (defined(FOXBMS_HAVE_REDUNDANT_V_T_MEASUREMENT) && (FOXBMS_HAVE_REDUNDANT_V_T_MEASUREMENT == 1))
#include "database_helper.h"
#include "redundancy.h"
#endif /* FOXBMS_HAVE_REDUNDANT_V_T_MEASUREMENT == 1 */
#include "bms-values.h"
#include "fassert.h"
#include "plausibility.h"

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/
/** System values for the pack; this table holds the latest, validated values */
static DATA_BLOCK_MIN_MAX_s bmsvl_tableMinimumMaximumValues      = {.header.uniqueId = DATA_BLOCK_ID_MIN_MAX};
static DATA_BLOCK_CELL_VOLTAGE_s bmsvl_tableCellVoltages         = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE};
static DATA_BLOCK_CELL_TEMPERATURE_s bmsvl_tableCellTemperatures = {.header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE};
static DATA_BLOCK_PACK_VALUES_s bmsvl_tablePackValues            = {.header.uniqueId = DATA_BLOCK_ID_PACK_VALUES};

static BMSVL_STATE_s bmsvl_state = {
    .lastStringPowerTimestamp   = {0u},
    .lastStringCurrentTimestamp = {0u},
};

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/
/**
 * @brief   Derive the minimum and maximum values for cells
 * @details Derive the minimum and maximum values for cell voltage and cell
 *          temperature from the validated measurements and stores them in the
 *          #DATA_BLOCK_MIN_MAX_s database entry.
 */
static void BMSVL_DeriveMinimumMaximumValues(void);

/**
 * @brief   Calculate the minimum, maximum and average cell voltage values
 * @details Calculate the minimum, maximum and average cell voltage values from
 *          the validated measurements and stores them in the
 *          #DATA_BLOCK_MIN_MAX_s database entry.
 * @param   pValidatedVoltages     validated cell voltage measurements
 * @param   pMinMaxAverageValues   derived minimum, maximum and average values
 * @return  #STD_OK if values are successfully derived, #STD_NOT_OK otherwise
 */
static STD_RETURN_TYPE_e BMSVL_CalculateCellVoltageMinMaxAverage(
    const DATA_BLOCK_CELL_VOLTAGE_s *const pValidatedVoltages,
    DATA_BLOCK_MIN_MAX_s *pMinMaxAverageValues);

/**
 * @brief   Calculate the minimum, maximum and average cell temperature values
 * @details Calculate the minimum, maximum and average cell temperature values
 *          from the validated measurements and stores them in the
 *          #DATA_BLOCK_MIN_MAX_s database entry.
 * @param   pValidatedTemperatures validated cell temperature measurements
 * @param   pMinMaxAverageValues   derived minimum, maximum and average values
 * @return  #STD_OK if values are successfully derived, #STD_NOT_OK otherwise
 */
static STD_RETURN_TYPE_e BMSVL_CalculateCellTemperatureMinMaxAverage(
    const DATA_BLOCK_CELL_TEMPERATURE_s *const pValidatedTemperatures,
    DATA_BLOCK_MIN_MAX_s *pMinMaxAverageValues);

#if (defined(FOXBMS_HAVE_REDUNDANT_V_T_MEASUREMENT) && (FOXBMS_HAVE_REDUNDANT_V_T_MEASUREMENT == 0))
/**
 * @brief   Copy base measurement values into the validated database entries.
 * @details Read the base cell voltage and cell temperature database entries
 *          and copies their content into the passed validated database
 *          entries. The copy is only performed if the respective database
 *          entry has already been updated at least once by the AFE driver.
 *          Otherwise, invalid (initial) values would be written into the
 *          validated entries. The header of the destination entry is preserved
 *          so that the unique ID and timestamps of the validated entry remain
 *          valid.
 * @param[in,out] pTableCellVoltages     pointer to the validated cell voltage
 *                                       database entry that is updated with
 *                                       the base measurement values
 * @param[in,out] pTableCellTemperatures pointer to the validated cell
 *                                       temperature database entry that is
 *                                       updated with the base measurement
 *                                       values
 */
static void BMSVL_CopyBaseMeasurementsToValidatedMeasurements(
    DATA_BLOCK_CELL_VOLTAGE_s *pTableCellVoltages,
    DATA_BLOCK_CELL_TEMPERATURE_s *pTableCellTemperatures);
#endif /* FOXBMS_HAVE_REDUNDANT_V_T_MEASUREMENT == 0 */

/*========== Static Function Implementations ================================*/
#if (defined(FOXBMS_HAVE_REDUNDANT_V_T_MEASUREMENT) && (FOXBMS_HAVE_REDUNDANT_V_T_MEASUREMENT == 0))
static void BMSVL_CopyBaseMeasurementsToValidatedMeasurements(
    DATA_BLOCK_CELL_VOLTAGE_s *pTableCellVoltages,
    DATA_BLOCK_CELL_TEMPERATURE_s *pTableCellTemperatures) {
    FAS_ASSERT(pTableCellVoltages != NULL_PTR);
    FAS_ASSERT(pTableCellTemperatures != NULL_PTR);

    static DATA_BLOCK_CELL_VOLTAGE_s bmsvl_tableCellVoltageBase = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE_BASE};
    static DATA_BLOCK_CELL_TEMPERATURE_s bmsvl_tableCellTemperatureBase = {
        .header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE_BASE};

    DATA_READ_DATA(&bmsvl_tableCellVoltageBase, &bmsvl_tableCellTemperatureBase);

    /* only copy if the database entries have been updated once by the AFE
     * driver as otherwise we store invalid values in the validated entry */
    if (DATA_DatabaseEntryUpdatedAtLeastOnce(pTableCellVoltages->header)) {
        DATA_BLOCK_HEADER_s tmpHeader = pTableCellVoltages->header; /* Save header */
        *pTableCellVoltages           = bmsvl_tableCellVoltageBase; /* Copy whole database entry */
        pTableCellVoltages->header    = tmpHeader;                  /* Restore previous header */
    }

    if (DATA_DatabaseEntryUpdatedAtLeastOnce(pTableCellTemperatures->header)) {
        DATA_BLOCK_HEADER_s tmpHeader  = pTableCellTemperatures->header; /* Save header */
        *pTableCellTemperatures        = bmsvl_tableCellTemperatureBase; /* Copy whole database entry */
        pTableCellTemperatures->header = tmpHeader;                      /* Restore previous header */
    }
}
#endif /* FOXBMS_HAVE_REDUNDANT_V_T_MEASUREMENT == 0 */

static STD_RETURN_TYPE_e BMSVL_CalculateCellVoltageMinMaxAverage(
    const DATA_BLOCK_CELL_VOLTAGE_s *const pValidatedVoltages,
    DATA_BLOCK_MIN_MAX_s *pMinMaxAverageValues) {
    FAS_ASSERT(pValidatedVoltages != NULL_PTR);
    FAS_ASSERT(pMinMaxAverageValues != NULL_PTR);

    STD_RETURN_TYPE_e retval = STD_OK;

    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        uint16_t nrValidCellVoltages = 0u;
        int16_t min                  = INT16_MAX;
        int16_t max                  = INT16_MIN;
        int32_t sum                  = 0;
        uint16_t moduleNumberMinimum = 0u;
        uint16_t cellNumberMinimum   = 0u;
        uint16_t moduleNumberMaximum = 0u;
        uint16_t cellNumberMaximum   = 0u;
        /* Iterate over all cells in each string */
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t cb = 0u; cb < BS_NR_OF_CELL_BLOCKS_PER_MODULE; cb++) {
                if (pValidatedVoltages->invalidCellVoltage[s][m][cb] == false) {
                    /* Cell voltage is valid -> use this voltage for subsequent calculations */
                    const int16_t cellVoltage_mV = pValidatedVoltages->cellVoltage_mV[s][m][cb];

                    nrValidCellVoltages++;
                    sum += cellVoltage_mV;

                    if (cellVoltage_mV < min) {
                        min                 = cellVoltage_mV;
                        moduleNumberMinimum = m;
                        cellNumberMinimum   = cb;
                    }
                    if (cellVoltage_mV > max) {
                        max                 = cellVoltage_mV;
                        moduleNumberMaximum = m;
                        cellNumberMaximum   = cb;
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
        pMinMaxAverageValues->validMeasuredCellVoltages[s]  = nrValidCellVoltages;

        /* Prevent division by 0, if all cell voltages are invalid */
        if (nrValidCellVoltages > 0u) {
            pMinMaxAverageValues->averageCellVoltage_mV[s] = (int16_t)(sum / (int32_t)nrValidCellVoltages);
        } else {
            pMinMaxAverageValues->averageCellVoltage_mV[s] = 0;
            retval                                         = STD_NOT_OK;
        }
    }
    return retval;
}

static STD_RETURN_TYPE_e BMSVL_CalculateCellTemperatureMinMaxAverage(
    const DATA_BLOCK_CELL_TEMPERATURE_s *const pValidatedTemperatures,
    DATA_BLOCK_MIN_MAX_s *pMinMaxAverageValues) {
    FAS_ASSERT(pValidatedTemperatures != NULL_PTR);
    FAS_ASSERT(pMinMaxAverageValues != NULL_PTR);

    STD_RETURN_TYPE_e retval = STD_OK;

    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        uint16_t moduleNumberMinimum     = 0u;
        uint16_t sensorNumberMinimum     = 0u;
        uint16_t moduleNumberMaximum     = 0u;
        uint16_t sensorNumberMaximum     = 0u;
        uint16_t nrValidCellTemperatures = 0u;
        int16_t min                      = INT16_MAX;
        int16_t max                      = INT16_MIN;
        float_t sum_ddegC                = 0.0f;

        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t ts = 0u; ts < BS_NR_OF_TEMP_SENSORS_PER_MODULE; ts++) {
                if (pValidatedTemperatures->invalidCellTemperature[s][m][ts] == false) {
                    /* Cell temperature is valid -> use this value for subsequent calculations */
                    const int16_t temperature_ddegC = pValidatedTemperatures->cellTemperature_ddegC[s][m][ts];

                    nrValidCellTemperatures++;
                    sum_ddegC += (float_t)temperature_ddegC;

                    if (temperature_ddegC < min) {
                        min                 = temperature_ddegC;
                        moduleNumberMinimum = m;
                        sensorNumberMinimum = ts;
                    }
                    if (temperature_ddegC > max) {
                        max                 = temperature_ddegC;
                        moduleNumberMaximum = m;
                        sensorNumberMaximum = ts;
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
        pMinMaxAverageValues->validMeasuredCellTemperatures[s] = nrValidCellTemperatures;

        /* Prevent division by 0, if all cell temperatures are invalid */
        if (nrValidCellTemperatures > 0u) {
            pMinMaxAverageValues->averageTemperature_ddegC[s] = sum_ddegC / (float_t)nrValidCellTemperatures;
        } else {
            pMinMaxAverageValues->averageTemperature_ddegC[s] = 0.0f;
            retval                                            = STD_NOT_OK;
        }
    }
    return retval;
}

static void BMSVL_DeriveMinimumMaximumValues(void) {
    /* Calculate min/max/average cell voltages */
    BMSVL_CalculateCellVoltageMinMaxAverage(&bmsvl_tableCellVoltages, &bmsvl_tableMinimumMaximumValues);
    /* Individual cell voltages validated and min/max/average calculated -> check voltage spread */
    if (STD_NOT_OK == PL_CheckVoltageSpread(&bmsvl_tableCellVoltages, &bmsvl_tableMinimumMaximumValues)) {
        /* Recalculate min/max/average cell voltages as at least one cell voltage has been detected as invalid to
            assure that min/max voltage is not an invalid one and average is calculated only using valid voltages*/
        BMSVL_CalculateCellVoltageMinMaxAverage(&bmsvl_tableCellVoltages, &bmsvl_tableMinimumMaximumValues);
    }

    /* Calculate min/max/average cell temperatures */
    BMSVL_CalculateCellTemperatureMinMaxAverage(&bmsvl_tableCellTemperatures, &bmsvl_tableMinimumMaximumValues);

    /* Individual cell temperatures validated and min/max/average calculated -> check temperature spread */
    if (STD_NOT_OK == PL_CheckTemperatureSpread(&bmsvl_tableCellTemperatures, &bmsvl_tableMinimumMaximumValues)) {
        /* Recalculate min/max/average temperatures as at least one temperature has been detected as invalid assure
            that min/max cell temperature is not an invalid one and average is calculated only using valid temperatures*/
        BMSVL_CalculateCellTemperatureMinMaxAverage(&bmsvl_tableCellTemperatures, &bmsvl_tableMinimumMaximumValues);
    }
}

/*========== Extern Function Implementations ================================*/
extern void BMSVL_Initialize(void) {
    STD_RETURN_TYPE_e retval = STD_NOT_OK;
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            /* Invalidate cell voltage values */
            for (uint8_t cb = 0; cb < BS_NR_OF_CELL_BLOCKS_PER_MODULE; cb++) {
                bmsvl_tableCellVoltages.invalidCellVoltage[s][m][cb] = true;
            }
            bmsvl_tableCellVoltages.invalidModuleVoltage[s][m] = true;
            /* Invalidate cell temperature values */
            for (uint8_t ts = 0; ts < BS_NR_OF_TEMP_SENSORS_PER_MODULE; ts++) {
                bmsvl_tableCellTemperatures.invalidCellTemperature[s][m][ts] = true;
            }
        }
        bmsvl_tableCellVoltages.invalidStringVoltage[s] = true;
        /* Invalidate string values */
        bmsvl_tablePackValues.invalidStringVoltage[s] = 0x01u;
        bmsvl_tablePackValues.invalidStringCurrent[s] = 0x01u;
        bmsvl_tablePackValues.invalidStringPower[s]   = 0x01u;
    }
    /* Invalidate pack values */
    bmsvl_tablePackValues.invalidPackCurrent    = 0x01u; /*!< bitmask if current is valid. 0->valid, 1->invalid */
    bmsvl_tablePackValues.invalidBatteryVoltage = 0x01u; /*!< bitmask if voltage is valid. 0->valid, 1->invalid */
    bmsvl_tablePackValues.invalidHvBusVoltage   = 0x01u; /*!< bitmask if voltage is valid. 0->valid, 1->invalid */
    bmsvl_tablePackValues.invalidPackPower      = 0x01u; /*!< bitmask if power is valid. 0->valid, 1->invalid */

    retval = DATA_WRITE_DATA(&bmsvl_tableCellVoltages, &bmsvl_tableCellTemperatures, &bmsvl_tablePackValues);
    FAS_ASSERT(retval == STD_OK);
}

extern void BMSVL_UpdateSystemValues(void) {
    static DATA_BLOCK_CURRENT_s tableCurrent                 = {.header.uniqueId = DATA_BLOCK_ID_CURRENT};
    static DATA_BLOCK_SYSTEM_VOLTAGE_1_s tableSystemVoltage1 = {.header.uniqueId = DATA_BLOCK_ID_SYSTEM_VOLTAGE_1};
    static DATA_BLOCK_SYSTEM_VOLTAGE_3_s tableSystemVoltage3 = {.header.uniqueId = DATA_BLOCK_ID_SYSTEM_VOLTAGE_3};
    static DATA_BLOCK_POWER_s tablePower                     = {.header.uniqueId = DATA_BLOCK_ID_POWER};

#if (defined(FOXBMS_HAVE_REDUNDANT_V_T_MEASUREMENT) && (FOXBMS_HAVE_REDUNDANT_V_T_MEASUREMENT == 1))
    MRC_REQUIRED_UPDATES_s requireUpdates = {
        .updatedCellVoltageDatabaseEntry = false,
        .updatedTemperatureDatabaseEntry = false,
    };
#endif /* FOXBMS_HAVE_REDUNDANT_V_T_MEASUREMENT == 1 */

    /* Read all relevant database entries are not defined (function and module
     * static) in one place at the beginning */
    DATA_READ_DATA(&tableCurrent, &tableSystemVoltage1, &tableSystemVoltage3, &tablePower);
    DATA_READ_DATA(&bmsvl_tableMinimumMaximumValues);

    /* whether redundant cell and temperature measurement is implemented or
     * not, we need to first have the base measurements available:
     * - *redundant*: reading and writing cell and temperature measurement is
     *   done through the redundancy module
     * - *not redundant*: reading and writing cell and temperature measurement
     *   is simply copied from the base measurement tables to the system value
     *   tables here
     */
#if (defined(FOXBMS_HAVE_REDUNDANT_V_T_MEASUREMENT) && (FOXBMS_HAVE_REDUNDANT_V_T_MEASUREMENT == 0))
    BMSVL_CopyBaseMeasurementsToValidatedMeasurements(&bmsvl_tableCellVoltages, &bmsvl_tableCellTemperatures);
#elif (defined(FOXBMS_HAVE_REDUNDANT_V_T_MEASUREMENT) && (FOXBMS_HAVE_REDUNDANT_V_T_MEASUREMENT == 1))
    requireUpdates = MRC_ValidateAfeMeasurement(&bmsvl_tableCellVoltages, &bmsvl_tableCellTemperatures);
#endif /* FOXBMS_HAVE_REDUNDANT_V_T_MEASUREMENT */

    /* Derive all cell-related minimum and maximum values as meaningful values
     * are now available by the previous step */
    BMSVL_DeriveMinimumMaximumValues();

    PL_ValidateCurrentMeasurement(&bmsvl_state, &bmsvl_tablePackValues, &tableCurrent);
    PL_ValidateStringVoltageMeasurement(
        &bmsvl_tablePackValues, &bmsvl_tableMinimumMaximumValues, &tableSystemVoltage1, &bmsvl_tableCellVoltages);
    PL_ValidateBatteryVoltageMeasurement(&bmsvl_tablePackValues);
    PL_ValidateHighVoltageBusMeasurement(&bmsvl_tablePackValues, &tableSystemVoltage3);
    PL_ValidatePowerMeasurement(&bmsvl_state, &bmsvl_tablePackValues, &tablePower);

#if (defined(FOXBMS_HAVE_REDUNDANT_V_T_MEASUREMENT) && (FOXBMS_HAVE_REDUNDANT_V_T_MEASUREMENT == 0))
    /* Write validated cell and temperature measurements to database */
    DATA_WRITE_DATA(
        &bmsvl_tableCellVoltages,
        &bmsvl_tableCellTemperatures,
        &bmsvl_tablePackValues,
        &bmsvl_tableMinimumMaximumValues);
#elif (defined(FOXBMS_HAVE_REDUNDANT_V_T_MEASUREMENT) && (FOXBMS_HAVE_REDUNDANT_V_T_MEASUREMENT == 1))
    if (requireUpdates.updatedCellVoltageDatabaseEntry == true &&
        requireUpdates.updatedTemperatureDatabaseEntry == true) {
        DATA_WRITE_DATA(&bmsvl_tableCellVoltages, &bmsvl_tableCellTemperatures);
    } else if (requireUpdates.updatedCellVoltageDatabaseEntry == true) {
        DATA_WRITE_DATA(&bmsvl_tableCellVoltages);
    } else if (requireUpdates.updatedTemperatureDatabaseEntry == true) {
        DATA_WRITE_DATA(&bmsvl_tableCellTemperatures);
    } else {
        /* If validation of both, cell voltage and cell temperature, failed, we
         * do not write anything to the database, as we do not have any valid
         * measurements. */
    }
    DATA_WRITE_DATA(&bmsvl_tablePackValues, &bmsvl_tableMinimumMaximumValues);
#endif /* FOXBMS_HAVE_REDUNDANT_V_T_MEASUREMENT */
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
extern void TEST_BMSVL_DeriveMinimumMaximumValues(void) {
    BMSVL_DeriveMinimumMaximumValues();
}
extern STD_RETURN_TYPE_e TEST_BMSVL_CalculateCellVoltageMinMaxAverage(
    const DATA_BLOCK_CELL_VOLTAGE_s *const pValidatedVoltages,
    DATA_BLOCK_MIN_MAX_s *pMinMaxAverageValues) {
    return BMSVL_CalculateCellVoltageMinMaxAverage(pValidatedVoltages, pMinMaxAverageValues);
}

extern STD_RETURN_TYPE_e TEST_BMSVL_CalculateCellTemperatureMinMaxAverage(
    const DATA_BLOCK_CELL_TEMPERATURE_s *const pValidatedTemperatures,
    DATA_BLOCK_MIN_MAX_s *pMinMaxAverageValues) {
    return BMSVL_CalculateCellTemperatureMinMaxAverage(pValidatedTemperatures, pMinMaxAverageValues);
}

#if (defined(FOXBMS_HAVE_REDUNDANT_V_T_MEASUREMENT) && (FOXBMS_HAVE_REDUNDANT_V_T_MEASUREMENT == 0))
extern void TEST_BMSVL_CopyBaseMeasurementsToValidatedMeasurements(
    DATA_BLOCK_CELL_VOLTAGE_s *pTableCellVoltages,
    DATA_BLOCK_CELL_TEMPERATURE_s *pTableCellTemperatures) {
    BMSVL_CopyBaseMeasurementsToValidatedMeasurements(pTableCellVoltages, pTableCellTemperatures);
}
#endif /* FOXBMS_HAVE_REDUNDANT_V_T_MEASUREMENT == 0 */

#endif /* UNITY_UNIT_TEST */
