/**
 *
 * @copyright &copy; 2010 - 2021, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * - &Prime;This product uses parts of foxBMS&reg;&Prime;
 * - &Prime;This product includes parts of foxBMS&reg;&Prime;
 * - &Prime;This product is derived from foxBMS&reg;&Prime;
 *
 */

/**
 * @file    soa_cfg.c
 * @author  foxBMS Team
 * @date    2020-10-14 (date of creation)
 * @updated 2020-10-14 (date of last update)
 * @ingroup APPLICATION_CONFIGURATION
 * @prefix  SOA
 *
 * @brief   Configuration for safe-operating area check
 *
 */

/*========== Includes =======================================================*/
#include "soa_cfg.h"

#include "battery_cell_cfg.h"
#include "battery_system_cfg.h"

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/
bool SOA_IsPackCurrentLimitViolated(uint32_t current_mA, BMS_CURRENT_FLOW_STATE_e currentDirection) {
    bool currentLimitViolated = false;
    /* Only check if battery is not at rest. No overcurrent possible if battery is resting */
    if ((BMS_CHARGING == currentDirection) || (BMS_DISCHARGING == currentDirection)) {
        if (current_mA > BS_MAXIMUM_PACK_CURRENT_mA) {
            currentLimitViolated = true;
        }
    }
    return currentLimitViolated;
}

bool SOA_IsStringCurrentLimitViolated(uint32_t current_mA, BMS_CURRENT_FLOW_STATE_e currentDirection) {
    bool currentLimitViolated = false;
    /* Only check if battery is not at rest. No overcurrent possible if battery is resting */
    if ((BMS_CHARGING == currentDirection) || (BMS_DISCHARGING == currentDirection)) {
        if (current_mA > BS_MAXIMUM_STRING_CURRENT_mA) {
            currentLimitViolated = true;
        }
    }
    return currentLimitViolated;
}

bool SOA_IsCellCurrentLimitViolated(uint32_t current_mA, BMS_CURRENT_FLOW_STATE_e currentDirection) {
    bool currentLimitViolated = false;
    /* Only check if battery is not at rest. No overcurrent possible if battery is resting */
    if (BMS_CHARGING == currentDirection) {
        if (current_mA > (BS_NR_OF_PARALLEL_CELLS_PER_MODULE * BC_CURRENT_MAX_CHARGE_MSL_mA)) {
            currentLimitViolated = true;
        }
    } else if (BMS_DISCHARGING == currentDirection) {
        if (current_mA > (BS_NR_OF_PARALLEL_CELLS_PER_MODULE * BC_CURRENT_MAX_DISCHARGE_MSL_mA)) {
            currentLimitViolated = true;
        }
    } else {
        /* BMS_RELAXATION or BMS_AT_REST */
        currentLimitViolated = false;
    }
    return currentLimitViolated;
}

bool SOA_IsCurrentOnOpenString(BMS_CURRENT_FLOW_STATE_e currentDirection, uint8_t stringNumber) {
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);
    bool currentOnOpenString = false;
    /* Current is flowing as soon as a current direction detected */
    if ((BMS_CHARGING == currentDirection) || (BMS_DISCHARGING == currentDirection)) {
        const bool stringClosed      = BMS_IsStringClosed(stringNumber);
        const bool stringPrecharging = BMS_IsStringPrecharging(stringNumber);
        /* String appears to be open */
        if ((false == stringClosed) && (false == stringPrecharging)) {
            currentOnOpenString = true;
        }
    }
    return currentOnOpenString;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
