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
 * @file    validate-high-voltage-bus.c
 * @author  foxBMS Team
 * @date    2026-07-23 (date of creation)
 * @updated 2026-10-06 (date of last update)
 * @version v1.12.0
 * @ingroup APPLICATION
 * @prefix  PL
 *
 * @brief   Validate high-voltage-bus voltage plausibility
 * @details Validate high-voltage-bus measurements from current
 *          sensor data.
 *          Only strings that are connected (closed or precharging), updated
 *          within timeout, and marked valid are used.
 *          The bus voltage is calculated as average of selected string
 *          voltages, or marked invalid if no valid value is available.
 *
 */

/*========== Includes =======================================================*/

#include "diag_cfg.h"
#include "plausibility_cfg.h"

#include "bms.h"
#include "database_helper.h"
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
extern void PL_ValidateHighVoltageBusMeasurement(
    DATA_BLOCK_PACK_VALUES_s *pTablePackValues,
    const DATA_BLOCK_SYSTEM_VOLTAGE_3_s *pTableSystemVoltage3) {
    FAS_ASSERT(pTablePackValues != NULL_PTR);
    FAS_ASSERT(pTableSystemVoltage3 != NULL_PTR);

    int32_t sum_mV        = 0;
    uint8_t validVoltages = 0u;
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        /* Check timeout of current sensor measurement */
        bool noTimeout = DATA_DatabaseBlockUpdatedWithinInterval(
            pTableSystemVoltage3->timestamp,
            pTableSystemVoltage3->previousTimestamp,
            s,
            PL_CURRENT_SENSOR_MEASUREMENT_TIMEOUT_ms);
        DIAG_ReportResultToHandler(
            STD_BoolToStdReturnType(noTimeout), DIAG_ID_CURRENT_SENSOR_V3_MEASUREMENT_TIMEOUT, DIAG_STRING, s);

        const bool isStringClosed      = BMS_IsStringClosed(s);
        const bool isStringPrecharging = BMS_IsStringPrecharging(s);
        if ((isStringPrecharging || isStringClosed) && noTimeout) {
            /* Only voltages of connected strings can be used */
            if (pTableSystemVoltage3->invalidMeasurement[s] == 0u) {
                /* Measured high voltage is valid */
                validVoltages++;
                sum_mV += pTableSystemVoltage3->highVoltage_mV[s];
            }
        }
    }

    if (validVoltages > 0u) {
        pTablePackValues->highVoltageBusVoltage_mV = (sum_mV / (int32_t)validVoltages);
        pTablePackValues->invalidHvBusVoltage      = 0;
    } else {
        /* TODO: do we want to write special data if no valid values can be read? */
        pTablePackValues->invalidHvBusVoltage = 1u;
    }
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
