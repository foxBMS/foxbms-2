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
 * @file    adi_ades183x_diagnostic_w.c
 * @author  foxBMS Team
 * @date    2023-10-09 (date of creation)
 * @updated 2024-12-20 (date of last update)
 * @version v1.8.0
 * @ingroup DRIVERS
 * @prefix  ADI
 *
 * @brief   Dummy diagnostics driver for the ADI analog front end.
 * @details TODO
 */

/*========== Includes =======================================================*/
#include "adi_ades183x_defs.h"
#include "adi_ades183x_diagnostic.h"
#include "diag.h"
#include "fassert.h"

#include <stdbool.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/

extern void ADI_Diagnostic(ADI_STATE_s *adiState) {
    FAS_ASSERT(adiState != NULL_PTR);
}

extern bool ADI_EvaluateDiagnosticCellVoltages(ADI_STATE_s *adiState, uint16_t moduleNumber) {
    FAS_ASSERT(adiState != NULL_PTR);
    FAS_ASSERT(moduleNumber < BS_NR_OF_MODULES_PER_STRING);
    bool retVal = false;
    if ((adiState->data.errorTable->crcIsOk[adiState->currentString][moduleNumber] == true) &&
        (adiState->data.errorTable->voltageRegisterContentIsNotStuck[adiState->currentString][moduleNumber] == true)) {
        retVal = true;
        DIAG_Handler(DIAG_ID_AFE_COMMUNICATION_INTEGRITY, DIAG_EVENT_OK, DIAG_STRING, adiState->currentString);
    } else {
        DIAG_Handler(DIAG_ID_AFE_COMMUNICATION_INTEGRITY, DIAG_EVENT_NOT_OK, DIAG_STRING, adiState->currentString);
    }
    return retVal;
}

extern bool ADI_EvaluateDiagnosticGpioVoltages(ADI_STATE_s *adiState, uint16_t moduleNumber) {
    FAS_ASSERT(adiState != NULL_PTR);
    FAS_ASSERT(moduleNumber < BS_NR_OF_MODULES_PER_STRING);
    bool retVal = false;
    if (adiState->data.errorTable->crcIsOk[adiState->currentString][moduleNumber] == true) {
        retVal = true;
        DIAG_Handler(DIAG_ID_AFE_COMMUNICATION_INTEGRITY, DIAG_EVENT_OK, DIAG_STRING, adiState->currentString);
    } else {
        DIAG_Handler(DIAG_ID_AFE_COMMUNICATION_INTEGRITY, DIAG_EVENT_NOT_OK, DIAG_STRING, adiState->currentString);
    }
    return retVal;
}

extern bool ADI_EvaluateDiagnosticStringAndModuleVoltages(ADI_STATE_s *adiState, uint16_t moduleNumber) {
    FAS_ASSERT(adiState != NULL_PTR);
    FAS_ASSERT(moduleNumber < BS_NR_OF_MODULES_PER_STRING);
    bool retVal = false;
    if ((adiState->data.errorTable->crcIsOk[adiState->currentString][moduleNumber] == true) &&
        (adiState->data.errorTable->auxiliaryRegisterContentIsNotStuck[adiState->currentString][moduleNumber] ==
         true)) {
        retVal = true;
    }
    return retVal;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST

#endif
