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
 * @file    validate-cell-temperature.c
 * @author  foxBMS Team
 * @date    2020-02-24 (date of creation)
 * @updated 2026-10-06 (date of last update)
 * @version v1.12.0
 * @ingroup APPLICATION
 * @prefix  PL
 *
 * @brief   Validate cell-temperature plausibility
 * @details Validate cell-temperature plausibility per string by
 *          comparing each valid sensor value against the string-average
 *          temperature.
 *          Values outside #PL_CELL_TEMPERATURE_SPREAD_TOLERANCE_dK are marked
 *          invalid.
 *          The number of valid temperatures and diagnostics are updated for
 *          each string.
 *
 */

/*========== Includes =======================================================*/
#include "battery_system_cfg.h"

#include "diag.h"
#include "fassert.h"
#include "fstd_types.h"
#include "plausibility.h"

#include <stdint.h>
#include <stdlib.h>

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/
/**
 * @brief   Check the individual cell temperature against the average
 *          temperature
 * @param   cellTemperature_ddegC       cell temperature
 * @param   averageTemperature_ddegC    average cell temperature
 * @return  #STD_OK if the individual cell temperature is within the tolerance
 *          of the average cell temperature, otherwise #STD_NOT_OK.
 */
static STD_RETURN_TYPE_e PL_CheckIndividualCellTemperature(
    int16_t cellTemperature_ddegC,
    int16_t averageTemperature_ddegC);

/*========== Static Function Implementations ================================*/
static STD_RETURN_TYPE_e PL_CheckIndividualCellTemperature(
    int16_t cellTemperature_ddegC,
    int16_t averageTemperature_ddegC) {
    STD_RETURN_TYPE_e result            = STD_OK;
    int16_t temperatureDifference_ddegC = abs(cellTemperature_ddegC - averageTemperature_ddegC);
    if (temperatureDifference_ddegC > PL_CELL_TEMPERATURE_SPREAD_TOLERANCE_dK) {
        result = STD_NOT_OK;
    }
    return result;
}

/*========== Extern Function Implementations ================================*/
extern STD_RETURN_TYPE_e PL_CheckTemperatureSpread(
    DATA_BLOCK_CELL_TEMPERATURE_s *pCellTemperatures,
    const DATA_BLOCK_MIN_MAX_s *pMinMaxAverageValues) {
    FAS_ASSERT(pCellTemperatures != NULL_PTR);
    FAS_ASSERT(pMinMaxAverageValues != NULL_PTR);

    STD_RETURN_TYPE_e noPlausibilityIssueDetected = STD_OK;

    /* Iterate over all cells */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        STD_RETURN_TYPE_e plausibilityIssueDetectedInString = STD_OK;
        uint16_t nrValidTemperatures                        = 0u;
        int16_t averageTemperature_ddegC                    = pMinMaxAverageValues->averageTemperature_ddegC[s];
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t ts = 0u; ts < BS_NR_OF_TEMP_SENSORS_PER_MODULE; ts++) {
                /* Only do check for valid temperatures */
                if (pCellTemperatures->invalidCellTemperature[s][m][ts] == false) {
                    STD_RETURN_TYPE_e plausibilityIssueDetected = PL_CheckIndividualCellTemperature(
                        pCellTemperatures->cellTemperature_ddegC[s][m][ts], averageTemperature_ddegC);
                    if (plausibilityIssueDetected == STD_NOT_OK) {
                        plausibilityIssueDetectedInString                   = plausibilityIssueDetected;
                        pCellTemperatures->invalidCellTemperature[s][m][ts] = true;
                        noPlausibilityIssueDetected                         = STD_NOT_OK;
                    } else {
                        nrValidTemperatures++;
                    }
                }
            }
        }
        pCellTemperatures->nrValidTemperatures[s] = nrValidTemperatures;
        DIAG_ReportResultToHandler(
            plausibilityIssueDetectedInString, DIAG_ID_BMS_VALUES_CELL_TEMPERATURE_SPREAD, DIAG_STRING, s);
    }
    return noPlausibilityIssueDetected;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
extern STD_RETURN_TYPE_e TEST_PL_CheckIndividualCellTemperature(
    int16_t cellTemperature_ddegC,
    int16_t averageTemperature_ddegC) {
    return PL_CheckIndividualCellTemperature(cellTemperature_ddegC, averageTemperature_ddegC);
}
#endif /* UNITY_UNIT_TEST */
