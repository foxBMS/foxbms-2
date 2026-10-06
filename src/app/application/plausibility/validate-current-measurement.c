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
 * @file    validate-current-measurement.c
 * @author  foxBMS Team
 * @date    2026-07-23 (date of creation)
 * @updated 2026-10-06 (date of last update)
 * @version v1.12.0
 * @ingroup APPLICATION
 * @prefix  PL
 *
 * @brief   Validate current-measurement plausibility
 * @details Validate per-string current measurements using timeout
 *          and validity flags.
 *          Fresh valid measurements are stored, stale or invalid measurements
 *          are marked invalid and reported via diagnostics.
 *          The pack current is computed as sum of all valid string currents
 *          and marked invalid if any string current is invalid.
 *
 */

/*========== Includes =======================================================*/
#include "battery_system_cfg.h"
#include "plausibility_cfg.h"

#include "bms-values.h"
#include "diag.h"
#include "fassert.h"
#include "fstd_types.h"
#include "plausibility.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/
extern void PL_ValidateCurrentMeasurement(
    BMSVL_STATE_s *pBmsvlState,
    DATA_BLOCK_PACK_VALUES_s *pTablePackValues,
    const DATA_BLOCK_CURRENT_s *pTableCurrent) {
    FAS_ASSERT(pBmsvlState != NULL_PTR);
    FAS_ASSERT(pTablePackValues != NULL_PTR);
    FAS_ASSERT(pTableCurrent != NULL_PTR);

    int32_t packCurrent_mA = 0;

    /* Validate pack current. Will be invalidated if not all current measurement values are valid */
    pTablePackValues->invalidPackCurrent = 0u;

    /* Iterate over all strings to calculate pack current */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        /* Check timestamp of measurement */
        bool noTimeout = DATA_DatabaseBlockUpdatedWithinInterval(
            pTableCurrent->timestamp, pTableCurrent->previousTimestamp, s, PL_CURRENT_MEASUREMENT_PERIOD_TIMEOUT_ms);
        (void)DIAG_ReportResultToHandler(
            STD_BoolToStdReturnType(noTimeout), DIAG_ID_CURRENT_MEASUREMENT_TIMEOUT, DIAG_STRING, s);

        if (noTimeout) {
            /* Check if current entry has been updated since last call */
            if (pBmsvlState->lastStringCurrentTimestamp[s] != pTableCurrent->timestamp[s]) {
                pBmsvlState->lastStringCurrentTimestamp[s] = pTableCurrent->timestamp[s];
                pTablePackValues->stringCurrent_mA[s]      = pTableCurrent->current_mA[s];
                if (pTableCurrent->invalidMeasurement[s] == 0u) {
                    /* String current measurement valid -> set valid flag */
                    pTablePackValues->invalidStringCurrent[s] = 0u;
                    (void)DIAG_Handler(DIAG_ID_CURRENT_MEASUREMENT_ERROR, DIAG_EVENT_OK, DIAG_STRING, s);
                } else {
                    /* String current measurement invalid -> set invalid flag */
                    (void)DIAG_Handler(DIAG_ID_CURRENT_MEASUREMENT_ERROR, DIAG_EVENT_NOT_OK, DIAG_STRING, s);
                    pTablePackValues->invalidStringCurrent[s] = 1u;
                }
            } else {
                /* Nothing to do. Measurement has not been updated but still within timeout */
            }
        } else {
            /* Measurement timeout reached -> set string current invalid */
            pTablePackValues->invalidStringCurrent[s] = 1u;
        }

        if (pTablePackValues->invalidStringCurrent[s] == 0u) {
            packCurrent_mA += pTablePackValues->stringCurrent_mA[s];
        } else {
            /* One string current is invalid -> pack current cannot be correct.
             * Set pack current invalid */
            pTablePackValues->invalidPackCurrent = 1u;
        }
    }
    pTablePackValues->packCurrent_mA = packCurrent_mA;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
