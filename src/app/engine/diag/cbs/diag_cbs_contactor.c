/**
 *
 * @copyright &copy; 2010 - 2023, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * @file    diag_cbs_contactor.c
 * @author  foxBMS Team
 * @date    2021-02-17 (date of creation)
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
 * @ingroup ENGINE
 * @prefix  DIAG
 *
 * @brief   Diagnosis driver implementation
 * @details This file implements the diagnosis callback functions for all
 *          contactor related events.
 */

/*========== Includes =======================================================*/
#include "diag_cbs.h"
#include "fstd_types.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/
extern void DIAG_StringContactorFeedback(
    DIAG_ID_e diagId,
    DIAG_EVENT_e event,
    const DIAG_DATABASE_SHIM_s *const kpkDiagShim,
    uint32_t stringNumber) {
    FAS_ASSERT(diagId < DIAG_ID_MAX);
    FAS_ASSERT((event == DIAG_EVENT_OK) || (event == DIAG_EVENT_NOT_OK) || (event == DIAG_EVENT_RESET));
    FAS_ASSERT(kpkDiagShim != NULL_PTR);
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);

    if (diagId == DIAG_ID_STRING_MINUS_CONTACTOR_FEEDBACK) {
        if (event == DIAG_EVENT_RESET) {
            kpkDiagShim->pTableError->contactorInNegativePathOfStringFeedbackError[stringNumber] = false;
        }
        if (event == DIAG_EVENT_NOT_OK) {
            kpkDiagShim->pTableError->contactorInNegativePathOfStringFeedbackError[stringNumber] = true;
        }
    } else if (diagId == DIAG_ID_STRING_PLUS_CONTACTOR_FEEDBACK) {
        if (event == DIAG_EVENT_RESET) {
            kpkDiagShim->pTableError->contactorInPositivePathOfStringFeedbackError[stringNumber] = false;
        }
        if (event == DIAG_EVENT_NOT_OK) {
            kpkDiagShim->pTableError->contactorInPositivePathOfStringFeedbackError[stringNumber] = true;
        }
    } else {
        /* This function should not be called with other diagnosis IDs */
        FAS_ASSERT(FAS_TRAP);
    }
}

extern void DIAG_PrechargeContactorFeedback(
    DIAG_ID_e diagId,
    DIAG_EVENT_e event,
    const DIAG_DATABASE_SHIM_s *const kpkDiagShim,
    uint32_t stringNumber) {
    FAS_ASSERT(diagId < DIAG_ID_MAX);
    FAS_ASSERT((event == DIAG_EVENT_OK) || (event == DIAG_EVENT_NOT_OK) || (event == DIAG_EVENT_RESET));
    FAS_ASSERT(kpkDiagShim != NULL_PTR);
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);

    if (diagId == DIAG_ID_PRECHARGE_CONTACTOR_FEEDBACK) {
        if (event == DIAG_EVENT_RESET) {
            kpkDiagShim->pTableError->prechargeContactorFeedbackError[stringNumber] = false;
        }
        if (event == DIAG_EVENT_NOT_OK) {
            kpkDiagShim->pTableError->prechargeContactorFeedbackError[stringNumber] = true;
        }
    } else {
        /* This function should not be called with other diagnosis IDs */
        FAS_ASSERT(FAS_TRAP);
    }
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
