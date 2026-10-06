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
 * @file    validate-battery-voltage.c
 * @author  foxBMS Team
 * @date    2020-02-24 (date of creation)
 * @updated 2026-10-06 (date of last update)
 * @version v1.12.0
 * @ingroup APPLICATION
 * @prefix  PL
 *
 * @brief   Validate battery and string-voltage consistency
 * @details Provide plausibility logic used by the voltage
 *          validation flow:
 *          - A helper that compares two string-voltage measurements with a
 *            configured tolerance.
 *          - A battery-voltage aggregation that derives one representative
 *            battery-voltage value from available string voltages.
 *
 *          Depending on contactor state, the aggregation either:
 *          - averages only connected strings (closed contactors), or
 *          - averages all valid strings when no string is connected.
 *
 *          If no valid input string voltage is available, the battery voltage
 *          is marked invalid and set to INT32_MAX.
 *
 */

/*========== Includes =======================================================*/

#include "plausibility_cfg.h"

#include "bms.h"
#include "fassert.h"
#include "fstd_types.h"
#include "plausibility.h"

#include <stdint.h>
#include <stdlib.h>

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/

extern void PL_ValidateBatteryVoltageMeasurement(DATA_BLOCK_PACK_VALUES_s *pTablePackValues) {
    FAS_ASSERT(pTablePackValues != NULL_PTR);

    int64_t sumOfStringValues_mV       = 0;
    int8_t numberOfValidStringVoltages = 0;
    uint8_t numberOfConnectedStrings   = BMS_GetNumberOfConnectedStrings();

    if (0u != numberOfConnectedStrings) {
        /* Iterate over all strings to see which strings are connected */
        for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
            bool isStringConnected = BMS_IsStringClosed(s);
            if ((pTablePackValues->invalidStringVoltage[s] == 0u) && (isStringConnected == true)) {
                /* AXIVION Disable Style MisraC2012Directive-4.1: Values start with 0, iteration is less than UINT8_MAX;
                 * overflow impossible */
                sumOfStringValues_mV += pTablePackValues->stringVoltage_mV[s];
                numberOfValidStringVoltages++;
                /* AXIVION Enable Style MisraC2012Directive-4.1: */
            }
        }
    } else {
        /* Take average of all strings if no strings are connected */
        for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
            if (pTablePackValues->invalidStringVoltage[s] == 0u) {
                /* AXIVION Disable Style MisraC2012Directive-4.1: Values start with 0, iteration is less than UINT8_MAX;
                 * overflow impossible */
                sumOfStringValues_mV += pTablePackValues->stringVoltage_mV[s];
                numberOfValidStringVoltages++;
                /* AXIVION Enable Style MisraC2012Directive-4.1: */
            }
        }
    }

    /* Only calculate average if at least one string voltage is valid */
    if (0 != numberOfValidStringVoltages) {
        /* AXIVION Next Codeline Style MisraC2012Directive-4.1: truncation impossible;
           we sum INT32 values x times and divide by x, resulting in INT32 */
        pTablePackValues->batteryVoltage_mV     = (int32_t)(sumOfStringValues_mV / numberOfValidStringVoltages);
        pTablePackValues->invalidBatteryVoltage = 0u;
    } else {
        pTablePackValues->batteryVoltage_mV     = INT32_MAX;
        pTablePackValues->invalidBatteryVoltage = 1u;
    }
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
