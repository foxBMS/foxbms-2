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
 * @file    validate-cell-voltage.c
 * @author  foxBMS Team
 * @date    2020-02-24 (date of creation)
 * @updated 2026-10-06 (date of last update)
 * @version v1.12.0
 * @ingroup APPLICATION
 * @prefix  PL
 *
 * @brief   Validate cell-voltage plausibility
 * @details Validate cell-voltage plausibility per string by
 *          comparing each valid cell voltage against the string-average cell
 *          voltage.
 *          Values outside #PL_CELL_VOLTAGE_SPREAD_TOLERANCE_mV are marked
 *          invalid.
 *          The number of valid cell voltages and diagnostics are updated for
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
 * @brief   Check the individual cell voltage against the average cell voltage
 * @param   cellVoltage_mV          cell voltage
 * @param   averageCellVoltage_mV   average cell voltage
 * @return  #STD_OK if the individual cell voltage is within the tolerance of
 *          the average cell voltage, otherwise #STD_NOT_OK.
 */
static STD_RETURN_TYPE_e PL_CheckIndividualCellVoltage(int16_t cellVoltage_mV, int16_t averageCellVoltage_mV);

/*========== Static Function Implementations ================================*/
static STD_RETURN_TYPE_e PL_CheckIndividualCellVoltage(int16_t cellVoltage_mV, int16_t averageCellVoltage_mV) {
    STD_RETURN_TYPE_e result     = STD_OK;
    int16_t voltageDifference_mV = abs(cellVoltage_mV - averageCellVoltage_mV);
    if (voltageDifference_mV > PL_CELL_VOLTAGE_SPREAD_TOLERANCE_mV) {
        result = STD_NOT_OK;
    }
    return result;
}

/*========== Extern Function Implementations ================================*/
extern STD_RETURN_TYPE_e PL_CheckVoltageSpread(
    DATA_BLOCK_CELL_VOLTAGE_s *pCellVoltages,
    const DATA_BLOCK_MIN_MAX_s *pMinMaxAverageValues) {
    FAS_ASSERT(pCellVoltages != NULL_PTR);
    FAS_ASSERT(pMinMaxAverageValues != NULL_PTR);

    STD_RETURN_TYPE_e noPlausibilityIssueDetected = STD_OK;

    /* Iterate over all cells */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        STD_RETURN_TYPE_e plausibilityIssueDetectedInString = STD_OK;
        uint16_t nrValidCellVoltages                        = 0u;
        int16_t averageCellVoltage_mV                       = pMinMaxAverageValues->averageCellVoltage_mV[s];
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t cb = 0u; cb < BS_NR_OF_CELL_BLOCKS_PER_MODULE; cb++) {
                /* Only do check for valid voltages */
                if (pCellVoltages->invalidCellVoltage[s][m][cb] == false) {
                    STD_RETURN_TYPE_e plausibilityIssueDetected =
                        PL_CheckIndividualCellVoltage(pCellVoltages->cellVoltage_mV[s][m][cb], averageCellVoltage_mV);
                    if (plausibilityIssueDetected == STD_NOT_OK) {
                        plausibilityIssueDetectedInString           = plausibilityIssueDetected;
                        pCellVoltages->invalidCellVoltage[s][m][cb] = true;
                        noPlausibilityIssueDetected                 = STD_NOT_OK;
                    } else {
                        nrValidCellVoltages++;
                    }
                }
            }
        }
        pCellVoltages->nrValidCellVoltages[s] = nrValidCellVoltages;
        DIAG_ReportResultToHandler(
            plausibilityIssueDetectedInString, DIAG_ID_BMS_VALUES_CELL_VOLTAGE_SPREAD, DIAG_STRING, s);
    }
    return noPlausibilityIssueDetected;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
extern STD_RETURN_TYPE_e TEST_PL_CheckIndividualCellVoltage(int16_t cellVoltage_mV, int16_t averageCellVoltage_mV) {
    return PL_CheckIndividualCellVoltage(cellVoltage_mV, averageCellVoltage_mV);
}

#endif /* UNITY_UNIT_TEST */
