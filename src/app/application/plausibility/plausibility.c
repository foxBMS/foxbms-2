/**
 *
 * @copyright &copy; 2010 - 2025, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * @file    plausibility.c
 * @author  foxBMS Team
 * @date    2020-02-24 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup APPLICATION
 * @prefix  PL
 *
 * @brief   Plausibility checks for cell voltage and cell temperatures
 * @details TODO
 *
 */

/*========== Includes =======================================================*/
#include "plausibility.h"

#include "battery_system_cfg.h"

#include "diag.h"
#include "foxmath.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/
extern STD_RETURN_TYPE_e PL_CheckStringVoltage(int32_t voltageAfe_mV, int32_t voltageCurrentSensor_mV) {
    STD_RETURN_TYPE_e result = STD_NOT_OK;

    /* Get deviation between these two measurements */
    int32_t diff_mV = voltageAfe_mV - voltageCurrentSensor_mV;

    if (abs(diff_mV) < PL_STRING_VOLTAGE_TOLERANCE_mV) {
        result = STD_OK;
    }
    return result;
}

extern STD_RETURN_TYPE_e PL_CheckCellVoltage(
    int16_t baseCellVoltage,
    int16_t redundancy0CellVoltage,
    int16_t *pCellVoltage) {
    /* AXIVION Routine Generic-MissingParameterAssert: baseCellVoltage: parameter accepts whole range */
    /* AXIVION Routine Generic-MissingParameterAssert: redundancy0CellVoltage: parameter accepts whole range */
    /* Pointer validity check */
    FAS_ASSERT(pCellVoltage != NULL_PTR);
    STD_RETURN_TYPE_e retval = STD_OK;

    if (abs(baseCellVoltage - redundancy0CellVoltage) > PL_CELL_VOLTAGE_TOLERANCE_mV) {
        retval = STD_NOT_OK;
    }
    /* Take the average value of base and redundant measurement value */
    *pCellVoltage = (baseCellVoltage + redundancy0CellVoltage) / 2;
    return retval;
}

extern STD_RETURN_TYPE_e PL_CheckCellTemperature(
    int16_t baseCellTemperature,
    int16_t redundancy0CellTemperature,
    int16_t *pCellTemperature) {
    /* AXIVION Routine Generic-MissingParameterAssert: baseCellTemperature: parameter accepts whole range */
    /* AXIVION Routine Generic-MissingParameterAssert: redundancy0CellTemperature: parameter accepts whole range */
    /* Pointer validity check */
    FAS_ASSERT(pCellTemperature != NULL_PTR);

    STD_RETURN_TYPE_e retval = STD_OK;

    if (abs(baseCellTemperature - redundancy0CellTemperature) > PL_CELL_TEMPERATURE_TOLERANCE_dK) {
        retval = STD_NOT_OK;
    }
    /* Take the average value of base and redundant measurement value */
    *pCellTemperature = (baseCellTemperature + redundancy0CellTemperature) / 2;
    return retval;
}

extern STD_RETURN_TYPE_e PL_CheckVoltageSpread(
    DATA_BLOCK_CELL_VOLTAGE_s *pCellVoltages,
    DATA_BLOCK_MIN_MAX_s *pMinMaxAverageValues) {
    /* Pointer validity check */
    FAS_ASSERT(pCellVoltages != NULL_PTR);
    FAS_ASSERT(pMinMaxAverageValues != NULL_PTR);

    STD_RETURN_TYPE_e retval = STD_OK;

    /* Iterate over all cells */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        STD_RETURN_TYPE_e plausibilityIssueDetected = STD_OK;
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t cb = 0u; cb < BS_NR_OF_CELL_BLOCKS_PER_MODULE; cb++) {
                /* Only do check for valid voltages */
                if (pCellVoltages->invalidCellVoltage[s][m][cb] == false) {
                    if (abs(pCellVoltages->cellVoltage_mV[s][m][cb] - pMinMaxAverageValues->averageCellVoltage_mV[s]) >
                        PL_CELL_VOLTAGE_SPREAD_TOLERANCE_mV) {
                        /* Voltage difference too large */
                        plausibilityIssueDetected = STD_NOT_OK;
                        retval                    = STD_NOT_OK;
                        /* Set this cell voltage invalid */
                        pCellVoltages->invalidCellVoltage[s][m][cb] = true;
                    }
                }
            }
        }
        DIAG_CheckEvent(plausibilityIssueDetected, DIAG_ID_PLAUSIBILITY_CELL_VOLTAGE_SPREAD, DIAG_STRING, s);
    }
    return retval;
}

extern STD_RETURN_TYPE_e PL_CheckTemperatureSpread(
    DATA_BLOCK_CELL_TEMPERATURE_s *pCellTemperatures,
    DATA_BLOCK_MIN_MAX_s *pMinMaxAverageValues) {
    /* Pointer validity check */
    FAS_ASSERT(pCellTemperatures != NULL_PTR);
    FAS_ASSERT(pMinMaxAverageValues != NULL_PTR);

    STD_RETURN_TYPE_e retval = STD_OK;

    /* Iterate over all cells */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        STD_RETURN_TYPE_e plausibilityIssueDetected = STD_OK;
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t ts = 0u; ts < BS_NR_OF_TEMP_SENSORS_PER_MODULE; ts++) {
                /* Only do check for valid temperatures */
                if (pCellTemperatures->invalidCellTemperature[s][m][ts] == false) {
                    if (abs(pCellTemperatures->cellTemperature_ddegC[s][m][ts] -
                            (int16_t)pMinMaxAverageValues->averageTemperature_ddegC[s]) >
                        PL_CELL_TEMPERATURE_SPREAD_TOLERANCE_dK) {
                        /* temperature difference too large */
                        plausibilityIssueDetected = STD_NOT_OK;
                        retval                    = STD_NOT_OK;
                        /* Set this cell temperature invalid */
                        pCellTemperatures->invalidCellTemperature[s][m][ts] = true;
                    } else {
                        pCellTemperatures->nrValidTemperatures[s]++;
                    }
                }
            }
        }
        DIAG_CheckEvent(plausibilityIssueDetected, DIAG_ID_PLAUSIBILITY_CELL_TEMPERATURE_SPREAD, DIAG_STRING, s);
    }
    return retval;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
