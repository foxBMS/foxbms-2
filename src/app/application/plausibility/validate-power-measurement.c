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
 * @file    validate-power-measurement.c
 * @author  foxBMS Team
 * @date    2026-07-23 (date of creation)
 * @updated 2026-10-06 (date of last update)
 * @version v1.12.0
 * @ingroup APPLICATION
 * @prefix  PL
 *
 * @brief   Validate power-measurement plausibility
 * @details Validate per-string power measurements using timeout,
 *          timestamp freshness, and measurement validity flags.
 *          If a fresh valid measurement is unavailable, a fallback power value
 *          can be calculated from valid string current and string voltage.
 *          String-level diagnostics are updated and pack power is calculated
 *          as sum of valid string powers, otherwise marked invalid.
 *
 */

/*========== Includes =======================================================*/
#include "battery_system_cfg.h"
#include "plausibility_cfg.h"

#include "bms-values.h"
#include "database_helper.h"
#include "diag.h"
#include "fassert.h"
#include "foxmath.h"
#include "fstd_types.h"
#include "plausibility.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/
extern void PL_ValidatePowerMeasurement(
    BMSVL_STATE_s *pBmsvlState,
    DATA_BLOCK_PACK_VALUES_s *pTablePackValues,
    DATA_BLOCK_POWER_s *pTablePower) {
    FAS_ASSERT(pBmsvlState != NULL_PTR);
    FAS_ASSERT(pTablePackValues != NULL_PTR);
    FAS_ASSERT(pTablePower != NULL_PTR);

    bool calculatePower = false;
    int32_t packPower_W = 0;

    /* Validate pack power. Will be invalidated if not all power measurement values are valid */
    pTablePackValues->invalidPackPower = 0u;

    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        /* Check timeout of current sensor measurement */
        bool noTimeout = DATA_DatabaseBlockUpdatedWithinInterval(
            pTablePower->timestamp, pTablePower->previousTimestamp, s, PL_CURRENT_SENSOR_MEASUREMENT_TIMEOUT_ms);
        DIAG_ReportResultToHandler(
            STD_BoolToStdReturnType(noTimeout), DIAG_ID_CURRENT_SENSOR_POWER_MEASUREMENT_TIMEOUT, DIAG_STRING, s);

        if (noTimeout) {
            /* Check if current sensor measurement has been updated */
            if (pTablePower->timestamp[s] != pBmsvlState->lastStringPowerTimestamp[s]) {
                pBmsvlState->lastStringPowerTimestamp[s] = pTablePower->timestamp[s];
                /* Check if measurement is valid */
                if (pTablePower->invalidMeasurement[s] == 0u) {
                    pTablePackValues->stringPower_W[s]      = pTablePower->power_W[s];
                    pTablePackValues->invalidStringPower[s] = 0u;
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
            calculatePower                          = true;
            pTablePackValues->invalidStringPower[s] = 1u;
        }
        if ((calculatePower == true) && (pTablePackValues->invalidStringCurrent[s] == 0u) &&
            (pTablePackValues->invalidStringVoltage[s] == 0u)) {
            /* Power measurement is invalid, but current and string voltage measurement are valid */
            const float_t stringCurrent_A           = (float_t)pTablePackValues->stringCurrent_mA[s] /
                                                      UNIT_CONVERSION_FACTOR_1000_FLOAT;
            const float_t stringVoltage_V           = (float_t)pTablePackValues->stringVoltage_mV[s] /
                                                      UNIT_CONVERSION_FACTOR_1000_FLOAT;
            pTablePackValues->stringPower_W[s]      = (int32_t)(stringCurrent_A * stringVoltage_V);
            pTablePackValues->invalidStringPower[s] = 0u;
        }
        if (pTablePackValues->invalidStringPower[s] == 0u) {
            packPower_W += pTablePackValues->stringPower_W[s];
            DIAG_Handler(DIAG_ID_POWER_MEASUREMENT_ERROR, DIAG_EVENT_OK, DIAG_STRING, s);
        } else {
            /* One string power is invalid -> pack power cannot be correct.
             * Set pack power invalid */
            pTablePackValues->invalidPackPower = 1u;
            DIAG_Handler(DIAG_ID_POWER_MEASUREMENT_ERROR, DIAG_EVENT_NOT_OK, DIAG_STRING, s);
        }
    }
    pTablePackValues->packPower_W = packPower_W;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
