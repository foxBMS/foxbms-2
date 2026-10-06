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
 * @file    validate-string-voltage.c
 * @author  foxBMS Team
 * @date    2026-07-23 (date of creation)
 * @updated 2026-10-06 (date of last update)
 * @version v1.12.0
 * @ingroup APPLICATION
 * @prefix  PL
 *
 * @brief   Validate string-voltage plausibility
 * @details Validate string-voltage values using current-sensor
 *          high-voltage measurements and AFE-derived string voltages.
 *          If complete and fresh measurements are available, a plausibility
 *          check between both sources is performed.
 *          Otherwise, the routine falls back to valid source selection or a
 *          reconstructed value based on average cell voltage and number of
 *          invalid cell voltages.
 *
 */

/*========== Includes =======================================================*/
#include "battery_system_cfg.h"
#include "diag_cfg.h"
#include "plausibility_cfg.h"

#include "database_helper.h"
#include "diag.h"
#include "fassert.h"
#include "plausibility.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/
/**
 * @brief   Check plausibility between AFE and current-sensor string voltage
 * @details Computes the absolute difference between both measurements and
 *          compares it against #PL_STRING_VOLTAGE_TOLERANCE_mV.
 *          The result is plausible only for strictly smaller differences.
 * @param[in] voltageAfe_mV            String voltage from AFE in mV
 * @param[in] voltageCurrentSensor_mV  String voltage from current sensor in mV
 * @return  #STD_OK if difference is below tolerance,
 *          otherwise #STD_NOT_OK
 */
static STD_RETURN_TYPE_e PL_CheckStringVoltage(int32_t voltageAfe_mV, int32_t voltageCurrentSensor_mV);

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/
static STD_RETURN_TYPE_e PL_CheckStringVoltage(int32_t voltageAfe_mV, int32_t voltageCurrentSensor_mV) {
    STD_RETURN_TYPE_e result     = STD_NOT_OK;
    int32_t voltageDifference_mV = abs(voltageAfe_mV - voltageCurrentSensor_mV);
    if (voltageDifference_mV < PL_STRING_VOLTAGE_TOLERANCE_mV) {
        result = STD_OK;
    }
    return result;
}

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/
extern void PL_ValidateStringVoltageMeasurement(
    DATA_BLOCK_PACK_VALUES_s *pTablePackValues,
    const DATA_BLOCK_MIN_MAX_s *pTableMinimumMaximumValues,
    const DATA_BLOCK_SYSTEM_VOLTAGE_1_s *pTableSystemVoltage1,
    const DATA_BLOCK_CELL_VOLTAGE_s *pTableCellVoltage) {
    FAS_ASSERT(pTablePackValues != NULL_PTR);
    FAS_ASSERT(pTableMinimumMaximumValues != NULL_PTR);
    FAS_ASSERT(pTableSystemVoltage1 != NULL_PTR);
    FAS_ASSERT(pTableCellVoltage != NULL_PTR);

    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        /* Check timeout of current sensor measurement */
        bool noTimeout = DATA_DatabaseBlockUpdatedWithinInterval(
            pTableSystemVoltage1->timestamp,
            pTableSystemVoltage1->previousTimestamp,
            s,
            PL_CURRENT_SENSOR_MEASUREMENT_TIMEOUT_ms);
        DIAG_ReportResultToHandler(
            STD_BoolToStdReturnType(noTimeout), DIAG_ID_CURRENT_SENSOR_V1_MEASUREMENT_TIMEOUT, DIAG_STRING, s);

        /* Perform plausibility check if AFE and new current sensor measurement is valid */
        if (noTimeout && (pTableSystemVoltage1->invalidMeasurement[s] == 0u) &&
            (pTableCellVoltage->nrValidCellVoltages[s] == BS_NR_OF_CELL_BLOCKS_PER_STRING)) {
            STD_RETURN_TYPE_e voltagePlausible =
                PL_CheckStringVoltage(pTableCellVoltage->stringVoltage_mV[s], pTableSystemVoltage1->highVoltage_mV[s]);
            (void)DIAG_ReportResultToHandler(voltagePlausible, DIAG_ID_BMS_VALUES_PACK_VOLTAGE, DIAG_STRING, s);

            /* Use current sensor measurement */ /* TODO: use really current sensor? Average of both? AFE measurement?
                                                  */
            pTablePackValues->stringVoltage_mV[s] = pTableSystemVoltage1->highVoltage_mV[s];

            if (voltagePlausible == STD_OK) {
                pTablePackValues->invalidStringVoltage[s] = 0u;
            } else {
                pTablePackValues->invalidStringVoltage[s] = 1u;
            }
        } else {
            /* Plausibility check cannot be performed if we do not have valid
             * values from AFE and current sensor measurement */
            (void)DIAG_ReportResultToHandler(STD_NOT_OK, DIAG_ID_BMS_VALUES_PACK_VOLTAGE, DIAG_STRING, s);

            if (noTimeout && (pTableSystemVoltage1->invalidMeasurement[s] == 0u)) {
                /* Current sensor measurement valid -> use this measurement */
                pTablePackValues->stringVoltage_mV[s]     = pTableSystemVoltage1->highVoltage_mV[s];
                pTablePackValues->invalidStringVoltage[s] = 0u;
            } else if (pTableCellVoltage->nrValidCellVoltages[s] == BS_NR_OF_CELL_BLOCKS_PER_STRING) {
                /* AFE measurement valid -> use this measurement */
                pTablePackValues->stringVoltage_mV[s]     = pTableCellVoltage->stringVoltage_mV[s];
                pTablePackValues->invalidStringVoltage[s] = 0u;
            } else {
                /* AFE and current sensor measurement invalid -> try to construct
                 * a valid from the number of valid cell voltages and substitute
                 * invalid cell voltages with the average cell voltage. */
                uint16_t numberInvalidCellVoltages =
                    (BS_NR_OF_CELL_BLOCKS_PER_STRING - pTableCellVoltage->nrValidCellVoltages[s]);

                pTablePackValues->stringVoltage_mV[s] =
                    pTableCellVoltage->stringVoltage_mV[s] +
                    (pTableMinimumMaximumValues->averageCellVoltage_mV[s] * (int16_t)numberInvalidCellVoltages);

                /* Only use this as valid value if not more than five cell voltages are invalid */
                if (numberInvalidCellVoltages > PL_ALLOWED_NUMBER_OF_INVALID_CELL_VOLTAGES) {
                    pTablePackValues->invalidStringVoltage[s] = 1u;
                } else {
                    pTablePackValues->invalidStringVoltage[s] = 0u;
                }
            }
        }
    }
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
extern STD_RETURN_TYPE_e TEST_PL_CheckStringVoltage(int32_t voltageAfe_mV, int32_t voltageCurrentSensor_mV) {
    return PL_CheckStringVoltage(voltageAfe_mV, voltageCurrentSensor_mV);
}
#endif
