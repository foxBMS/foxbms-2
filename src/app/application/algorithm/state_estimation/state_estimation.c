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
 * @file    state_estimation.c
 * @author  foxBMS Team
 * @date    2020-10-07 (date of creation)
 * @updated 2025-03-31 (date of last update)
 * @version v1.9.0
 * @ingroup APPLICATION
 * @prefix  SE
 *
 * @brief   State-estimation module responsible for the estimation of state-of-
 *          charge (SOC), state-of-energy (SOE) and state-of-health (SOH).
 *          Functions as a wrapper for the individual state-estimation
 *          algorithms.
 * @details TODO
 */

/*========== Includes =======================================================*/
#include "state_estimation.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/
static DATA_BLOCK_SOC_s se_tableSocEstimation = {.header.uniqueId = DATA_BLOCK_ID_SOC};
static DATA_BLOCK_SOH_s se_tableSohEstimation = {.header.uniqueId = DATA_BLOCK_ID_SOH};
static DATA_BLOCK_SOE_s se_tableSoeEstimation = {.header.uniqueId = DATA_BLOCK_ID_SOE};

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/
extern void SE_InitializeSoc(bool ccPresent, uint8_t stringNumber) {
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);
    SE_InitializeStateOfCharge(&se_tableSocEstimation, ccPresent, stringNumber);
    DATA_WRITE_DATA(&se_tableSocEstimation);
}

extern void SE_InitializeSoe(bool ec_present, uint8_t stringNumber) {
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);
    SE_InitializeStateOfEnergy(&se_tableSoeEstimation, ec_present, stringNumber);
    DATA_WRITE_DATA(&se_tableSoeEstimation);
}

extern void SE_InitializeSoh(uint8_t stringNumber) {
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);
    SE_InitializeStateOfHealth(&se_tableSohEstimation, stringNumber);
    DATA_WRITE_DATA(&se_tableSohEstimation);
}

extern void SE_RunStateEstimations(void) {
    SE_CalculateStateOfCharge(&se_tableSocEstimation);
    SE_CalculateStateOfEnergy(&se_tableSoeEstimation);
    SE_CalculateStateOfHealth(&se_tableSohEstimation);

    DATA_WRITE_DATA(&se_tableSocEstimation, &se_tableSohEstimation, &se_tableSoeEstimation);
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
