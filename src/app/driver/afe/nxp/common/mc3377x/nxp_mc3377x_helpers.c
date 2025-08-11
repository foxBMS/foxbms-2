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
 * @file    nxp_mc3377x_helpers.c
 * @author  foxBMS Team
 * @date    2025-04-01 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup DRIVERS
 * @prefix  N77X
 *
 * @brief   Helper functions of the MC3377X analog front-end driver.
 * @details TODO
 */

/*========== Includes =======================================================*/
#include "nxp_mc3377x_helpers.h"

#include "tsi.h"

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/
extern void N77x_ErrorHandling(N77X_STATE_s *pState, N77X_COMMUNICATION_STATUS_e returnedValue, uint8_t module) {
    FAS_ASSERT(pState != NULL_PTR);

    if (returnedValue == N77X_COMMUNICATION_OK) {
        pState->n77xData.errorTable->communicationOk[pState->currentString][module]        = true;
        pState->n77xData.errorTable->noCommunicationTimeout[pState->currentString][module] = true;
        pState->n77xData.errorTable->crcIsValid[pState->currentString][module]             = true;
    } else {
        pState->n77xData.errorTable->communicationOk[pState->currentString][module] = false;
        switch (returnedValue) {
            case N77X_COMMUNICATION_ERROR_TIMEOUT:
                pState->n77xData.errorTable->noCommunicationTimeout[pState->currentString][module] = false;
                break;
            case N77X_COMMUNICATION_ERROR_WRONG_CRC:
                pState->n77xData.errorTable->crcIsValid[pState->currentString][module] = false;
                break;
            default:
                pState->n77xData.errorTable->communicationOk[pState->currentString][module] = false;
                break;
        }
    }
}

extern void N77x_Wait(uint32_t milliseconds) {
    uint32_t current_time = OS_GetTickCount();
    /* Block task without possibility to wake up */
    OS_DelayTaskUntil(&current_time, milliseconds);
}

extern int16_t N77x_ConvertVoltagesToTemperatures(uint16_t adcVoltage_mV) {
    return TSI_GetTemperature(adcVoltage_mV); /* Convert degree Celsius to deci degree Celsius */
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
