/**
 *
 * @copyright &copy; 2010 - 2024, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * @file    master_info.c
 * @author  foxBMS Team
 * @date    2020-07-08 (date of creation)
 * @updated 2024-12-20 (date of last update)
 * @version v1.8.0
 * @ingroup ENGINE
 * @prefix  MINFO
 *
 * @brief   General foxBMS-master system information
 * @details This file contains the implementation to gather information on the
 *          foxBMS-Master and its connected peripherals on the master PCB.
 */

/*========== Includes =======================================================*/
#include "master_info.h"

#include "database.h"
#include "diag.h"
#include "fassert.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/
#define SUPPLY_VOLTAGE_CLAMP_30C_RESISTOR_DIVIDER_R1_ohm   (10000.0f)
#define SUPPLY_VOLTAGE_CLAMP_30C_RESISTOR_DIVIDER_R2_ohm   (866.0f)
#define SUPPLY_VOLTAGE_CLAMP_30C_SENSE_INPUT_ADC_INDEX     (6u)
#define SUPPLY_VOLTAGE_CLAMP_30C_UNDERVOLTAGE_THRESHOLD_mV (5000.0f)

/*========== Static Constant and Variable Definitions =======================*/
/** variable that tracks the state of the system */
static MINFO_MASTER_STATE_s minfo_state = {
    .resetSource              = NO_RESET,
    .debugProbe               = MINFO_DEBUG_PROBE_NOT_CONNECTED,
    .supplyVoltageClamp30c_mV = 0u,
};

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/
void MINFO_SetResetSource(resetSource_t resetSource) {
    /* Set system reset source */
    minfo_state.resetSource = resetSource;
}

resetSource_t MINFO_GetResetSource(void) {
    return minfo_state.resetSource;
}

void MINFO_SetDebugProbeConnectionState(MINFO_DEBUG_PROBE_CONNECTION_STATE_e state) {
    minfo_state.debugProbe = state;
}

MINFO_DEBUG_PROBE_CONNECTION_STATE_e MINFO_GetDebugProbeConnectionState(void) {
    return minfo_state.debugProbe;
}

void MINFO_CheckSupplyVoltageClamp30c(void) {
    DATA_BLOCK_ADC_VOLTAGE_s tableAdcVoltage = {.header.uniqueId = DATA_BLOCK_ID_ADC_VOLTAGE};
    (void)DATA_READ_DATA(&tableAdcVoltage);

    /* Supply voltage is measured using a voltage divider U = ((R1 + R2) / R2) * ADC voltage) */
    const float_t supplyVoltage_mV =
        ((SUPPLY_VOLTAGE_CLAMP_30C_RESISTOR_DIVIDER_R1_ohm + SUPPLY_VOLTAGE_CLAMP_30C_RESISTOR_DIVIDER_R2_ohm) /
         (SUPPLY_VOLTAGE_CLAMP_30C_RESISTOR_DIVIDER_R2_ohm)) *
        tableAdcVoltage.adc1ConvertedVoltages_mV[SUPPLY_VOLTAGE_CLAMP_30C_SENSE_INPUT_ADC_INDEX];

    if (supplyVoltage_mV >= SUPPLY_VOLTAGE_CLAMP_30C_UNDERVOLTAGE_THRESHOLD_mV) {
        (void)DIAG_Handler(DIAG_ID_SUPPLY_VOLTAGE_CLAMP_30C_LOST, DIAG_EVENT_OK, DIAG_SYSTEM, 0u);
    } else {
        (void)DIAG_Handler(DIAG_ID_SUPPLY_VOLTAGE_CLAMP_30C_LOST, DIAG_EVENT_NOT_OK, DIAG_SYSTEM, 0u);
    }
    minfo_state.supplyVoltageClamp30c_mV = (uint32_t)supplyVoltage_mV;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
