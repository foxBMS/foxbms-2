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
 * @file    diag_cbs_current.c
 * @author  foxBMS Team
 * @date    2021-02-17 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup ENGINE
 * @prefix  DIAG
 *
 * @brief   Diagnosis driver implementation
 * @details TODO
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
extern void DIAG_ErrorOvercurrentCharge(
    DIAG_ID_e diagId,
    DIAG_EVENT_e event,
    const DIAG_DATABASE_SHIM_s *const kpkDiagShim,
    uint32_t stringNumber) {
    FAS_ASSERT(
        (diagId == DIAG_ID_OVERCURRENT_CHARGE_CELL_MSL) || (diagId == DIAG_ID_OVERCURRENT_CHARGE_CELL_RSL) ||
        (diagId == DIAG_ID_OVERCURRENT_CHARGE_CELL_MOL) || (diagId == DIAG_ID_STRING_OVERCURRENT_CHARGE_MSL) ||
        (diagId == DIAG_ID_STRING_OVERCURRENT_CHARGE_RSL) || (diagId == DIAG_ID_STRING_OVERCURRENT_CHARGE_MOL) ||
        (diagId == DIAG_ID_PACK_OVERCURRENT_CHARGE_MSL));
    FAS_ASSERT((event == DIAG_EVENT_OK) || (event == DIAG_EVENT_NOT_OK) || (event == DIAG_EVENT_RESET));
    FAS_ASSERT(kpkDiagShim != NULL_PTR);
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);

    switch (diagId) {
        case DIAG_ID_OVERCURRENT_CHARGE_CELL_MSL:
            if (event == DIAG_EVENT_RESET) {
                kpkDiagShim->pTableMsl->cellChargeOvercurrent[stringNumber] = 0;
            } else if (event == DIAG_EVENT_NOT_OK) {
                kpkDiagShim->pTableMsl->cellChargeOvercurrent[stringNumber] = 1;
            } else {
                /* no relevant event, do nothing */
            }
            break;
        case DIAG_ID_OVERCURRENT_CHARGE_CELL_RSL:
            if (event == DIAG_EVENT_RESET) {
                kpkDiagShim->pTableRsl->cellChargeOvercurrent[stringNumber] = 0;
            } else if (event == DIAG_EVENT_NOT_OK) {
                kpkDiagShim->pTableRsl->cellChargeOvercurrent[stringNumber] = 1;
            } else {
                /* no relevant event, do nothing */
            }
            break;
        case DIAG_ID_OVERCURRENT_CHARGE_CELL_MOL:
            if (event == DIAG_EVENT_RESET) {
                kpkDiagShim->pTableMol->cellChargeOvercurrent[stringNumber] = 0;
            } else if (event == DIAG_EVENT_NOT_OK) {
                kpkDiagShim->pTableMol->cellChargeOvercurrent[stringNumber] = 1;
            } else {
                /* no relevant event, do nothing */
            }
            break;
        case DIAG_ID_STRING_OVERCURRENT_CHARGE_MSL:
            if (event == DIAG_EVENT_RESET) {
                kpkDiagShim->pTableMsl->stringChargeOvercurrent[stringNumber] = 0;
            } else if (event == DIAG_EVENT_NOT_OK) {
                kpkDiagShim->pTableMsl->stringChargeOvercurrent[stringNumber] = 1;
            } else {
                /* no relevant event, do nothing */
            }
            break;
        case DIAG_ID_STRING_OVERCURRENT_CHARGE_RSL:
            if (event == DIAG_EVENT_RESET) {
                kpkDiagShim->pTableRsl->stringChargeOvercurrent[stringNumber] = 0;
            } else if (event == DIAG_EVENT_NOT_OK) {
                kpkDiagShim->pTableRsl->stringChargeOvercurrent[stringNumber] = 1;
            } else {
                /* no relevant event, do nothing */
            }
            break;
        case DIAG_ID_STRING_OVERCURRENT_CHARGE_MOL:
            if (event == DIAG_EVENT_RESET) {
                kpkDiagShim->pTableMol->stringChargeOvercurrent[stringNumber] = 0;
            } else if (event == DIAG_EVENT_NOT_OK) {
                kpkDiagShim->pTableMol->stringChargeOvercurrent[stringNumber] = 1;
            } else {
                /* no relevant event, do nothing */
            }
            break;

        case DIAG_ID_PACK_OVERCURRENT_CHARGE_MSL:
            if (event == DIAG_EVENT_RESET) {
                kpkDiagShim->pTableMsl->packChargeOvercurrent = 0;
            } else if (event == DIAG_EVENT_NOT_OK) {
                kpkDiagShim->pTableMsl->packChargeOvercurrent = 1;
            } else {
                /* no relevant event, do nothing */
            }
            break;

        default:
            FAS_ASSERT(FAS_TRAP);
            break; /* LCOV_EXCL_LINE */
    }
}

extern void DIAG_ErrorOvercurrentDischarge(
    DIAG_ID_e diagId,
    DIAG_EVENT_e event,
    const DIAG_DATABASE_SHIM_s *const kpkDiagShim,
    uint32_t stringNumber) {
    FAS_ASSERT(
        (diagId == DIAG_ID_OVERCURRENT_DISCHARGE_CELL_MSL) || (diagId == DIAG_ID_OVERCURRENT_DISCHARGE_CELL_RSL) ||
        (diagId == DIAG_ID_OVERCURRENT_DISCHARGE_CELL_MOL) || (diagId == DIAG_ID_STRING_OVERCURRENT_DISCHARGE_MSL) ||
        (diagId == DIAG_ID_STRING_OVERCURRENT_DISCHARGE_RSL) || (diagId == DIAG_ID_STRING_OVERCURRENT_DISCHARGE_MOL) ||
        (diagId == DIAG_ID_PACK_OVERCURRENT_DISCHARGE_MSL));
    FAS_ASSERT((event == DIAG_EVENT_OK) || (event == DIAG_EVENT_NOT_OK) || (event == DIAG_EVENT_RESET));
    FAS_ASSERT(kpkDiagShim != NULL_PTR);
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);

    switch (diagId) {
        case DIAG_ID_OVERCURRENT_DISCHARGE_CELL_MSL:
            if (event == DIAG_EVENT_RESET) {
                kpkDiagShim->pTableMsl->cellDischargeOvercurrent[stringNumber] = 0;
            } else if (event == DIAG_EVENT_NOT_OK) {
                kpkDiagShim->pTableMsl->cellDischargeOvercurrent[stringNumber] = 1;
            } else {
                /* no relevant event, do nothing */
            }
            break;
        case DIAG_ID_OVERCURRENT_DISCHARGE_CELL_RSL:
            if (event == DIAG_EVENT_RESET) {
                kpkDiagShim->pTableRsl->cellDischargeOvercurrent[stringNumber] = 0;
            } else if (event == DIAG_EVENT_NOT_OK) {
                kpkDiagShim->pTableRsl->cellDischargeOvercurrent[stringNumber] = 1;
            } else {
                /* no relevant event, do nothing */
            }
            break;
        case DIAG_ID_OVERCURRENT_DISCHARGE_CELL_MOL:
            if (event == DIAG_EVENT_RESET) {
                kpkDiagShim->pTableMol->cellDischargeOvercurrent[stringNumber] = 0;
            } else if (event == DIAG_EVENT_NOT_OK) {
                kpkDiagShim->pTableMol->cellDischargeOvercurrent[stringNumber] = 1;
            } else {
                /* no relevant event, do nothing */
            }
            break;
        case DIAG_ID_STRING_OVERCURRENT_DISCHARGE_MSL:
            if (event == DIAG_EVENT_RESET) {
                kpkDiagShim->pTableMsl->stringDischargeOvercurrent[stringNumber] = 0;
            } else if (event == DIAG_EVENT_NOT_OK) {
                kpkDiagShim->pTableMsl->stringDischargeOvercurrent[stringNumber] = 1;
            } else {
                /* no relevant event, do nothing */
            }
            break;
        case DIAG_ID_STRING_OVERCURRENT_DISCHARGE_RSL:
            if (event == DIAG_EVENT_RESET) {
                kpkDiagShim->pTableRsl->stringDischargeOvercurrent[stringNumber] = 0;
            } else if (event == DIAG_EVENT_NOT_OK) {
                kpkDiagShim->pTableRsl->stringDischargeOvercurrent[stringNumber] = 1;
            } else {
                /* no relevant event, do nothing */
            }
            break;
        case DIAG_ID_STRING_OVERCURRENT_DISCHARGE_MOL:
            if (event == DIAG_EVENT_RESET) {
                kpkDiagShim->pTableMol->stringDischargeOvercurrent[stringNumber] = 0;
            } else if (event == DIAG_EVENT_NOT_OK) {
                kpkDiagShim->pTableMol->stringDischargeOvercurrent[stringNumber] = 1;
            } else {
                /* no relevant event, do nothing */
            }
            break;
        case DIAG_ID_PACK_OVERCURRENT_DISCHARGE_MSL:
            if (event == DIAG_EVENT_RESET) {
                kpkDiagShim->pTableMsl->packDischargeOvercurrent = 0;
            } else if (event == DIAG_EVENT_NOT_OK) {
                kpkDiagShim->pTableMsl->packDischargeOvercurrent = 1;
            } else {
                /* no relevant event, do nothing */
            }
            break;

        default:
            FAS_ASSERT(FAS_TRAP);
            break; /* LCOV_EXCL_LINE */
    }
}

extern void DIAG_ErrorCurrentMeasurement(
    DIAG_ID_e diagId,
    DIAG_EVENT_e event,
    const DIAG_DATABASE_SHIM_s *const kpkDiagShim,
    uint32_t stringNumber) {
    FAS_ASSERT((diagId == DIAG_ID_CURRENT_MEASUREMENT_TIMEOUT) || (diagId == DIAG_ID_CURRENT_MEASUREMENT_ERROR));
    FAS_ASSERT((event == DIAG_EVENT_OK) || (event == DIAG_EVENT_NOT_OK) || (event == DIAG_EVENT_RESET));
    FAS_ASSERT(kpkDiagShim != NULL_PTR);
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);

    switch (diagId) {
        case DIAG_ID_CURRENT_MEASUREMENT_TIMEOUT:
            if (event == DIAG_EVENT_RESET) {
                kpkDiagShim->pTableError->currentMeasurementTimeoutError[stringNumber] = false;
            } else if (event == DIAG_EVENT_NOT_OK) {
                kpkDiagShim->pTableError->currentMeasurementTimeoutError[stringNumber] = true;
            } else {
                /* no relevant event, do nothing */
            }
            break;

        case DIAG_ID_CURRENT_MEASUREMENT_ERROR:
            if (event == DIAG_EVENT_RESET) {
                kpkDiagShim->pTableError->currentMeasurementInvalidError[stringNumber] = false;
            } else if (event == DIAG_EVENT_NOT_OK) {
                kpkDiagShim->pTableError->currentMeasurementInvalidError[stringNumber] = true;
            } else {
                /* no relevant event, do nothing */
            }
            break;

        default:
            FAS_ASSERT(FAS_TRAP);
            break; /* LCOV_EXCL_LINE */
    }
}

void DIAG_ErrorCurrentOnOpenString(
    DIAG_ID_e diagId,
    DIAG_EVENT_e event,
    const DIAG_DATABASE_SHIM_s *const kpkDiagShim,
    uint32_t stringNumber) {
    FAS_ASSERT(diagId == DIAG_ID_CURRENT_ON_OPEN_STRING);
    FAS_ASSERT((event == DIAG_EVENT_OK) || (event == DIAG_EVENT_NOT_OK) || (event == DIAG_EVENT_RESET));
    FAS_ASSERT(kpkDiagShim != NULL_PTR);
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);

    switch (diagId) {
        case DIAG_ID_CURRENT_ON_OPEN_STRING:
            if (event == DIAG_EVENT_RESET) {
                kpkDiagShim->pTableError->currentOnOpenStringDetectedError[stringNumber] = false;
            } else if (event == DIAG_EVENT_NOT_OK) {
                kpkDiagShim->pTableError->currentOnOpenStringDetectedError[stringNumber] = true;
            } else {
                /* no relevant event, do nothing */
            }
            break;

        default:
            FAS_ASSERT(FAS_TRAP);
            break; /* LCOV_EXCL_LINE */
    }
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
