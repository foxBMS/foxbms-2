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
 * @file    diag_cfg.c
 * @author  foxBMS Team
 * @date    2019-11-28 (date of creation)
 * @updated 2026-10-06 (date of last update)
 * @version v1.12.0
 * @ingroup ENGINE_CONFIGURATION
 * @prefix  DIAG
 *
 * @brief   Diagnostic module configuration
 * @details The system monitoring configuration defined in diag_cfg.h is set in
 *          the array sysm_ch_cfg[]. The system monitoring is at the moment
 *          only used for supervising the cyclic/periodic tasks.
 *
 *          Every entry of the sysm_ch_cfg[] consists of
 *           - enum of monitored object
 *           - type of monitored object (at the moment only
 *             DIAG_SYSMON_CYCLICTASK is supported)
 *           - maximum delay in [ms] in which the object needs to call the
 *             SYSM_Notify function defined in diag.c
 *           - enabling of the recording for system monitoring
 *           - enabling of the system monitoring for the monitored object
 *           - callback function if system monitoring notices an error if
 *             wished, otherwise SYSM_DummyCallback
 */

/*========== Includes =======================================================*/
#include "general.h"

#include "diag_cfg.h"

#include "database.h"
#include "diag_cbs.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/
/** value of #DIAG_ID_MAX (as a define for the pre-processor) */
#define DIAG_ID_MAX_FOR_INIT (85u)

FAS_STATIC_ASSERT(DIAG_ID_MAX_FOR_INIT == (uint16_t)DIAG_ID_MAX, "Both values need to be identical.");

/*========== Static Constant and Variable Definitions =======================*/
/** local copy of the #DATA_BLOCK_ERROR_STATE_s table */
static DATA_BLOCK_ERROR_STATE_s diag_tableErrorFlags = {.header.uniqueId = DATA_BLOCK_ID_ERROR_STATE};

/** local copy of the #DATA_BLOCK_MOL_FLAG_s table */
static DATA_BLOCK_MOL_FLAG_s diag_tableMolFlags = {.header.uniqueId = DATA_BLOCK_ID_MOL_FLAG};

/** local copy of the #DATA_BLOCK_RSL_FLAG_s table */
static DATA_BLOCK_RSL_FLAG_s diag_tableRslFlags = {.header.uniqueId = DATA_BLOCK_ID_RSL_FLAG};

/** local copy of the #DATA_BLOCK_MSL_FLAG_s table */
static DATA_BLOCK_MSL_FLAG_s diag_tableMslFlags = {.header.uniqueId = DATA_BLOCK_ID_MSL_FLAG};

const DIAG_DATABASE_SHIM_s diag_kDatabaseShim = {
    .pTableError = &diag_tableErrorFlags,
    .pTableMol   = &diag_tableMolFlags,
    .pTableRsl   = &diag_tableRslFlags,
    .pTableMsl   = &diag_tableMslFlags,
};

/*========== Extern Constant and Variable Definitions =======================*/

DIAG_DEV_s diag_device = {
    .nrOfConfiguredDiagnosisEntries   = sizeof(diag_diagnosisIdConfiguration) / sizeof(DIAG_ID_CFG_s),
    .pConfigurationOfDiagnosisEntries = &diag_diagnosisIdConfiguration[0],
    .numberOfFatalErrors              = 0u,
    .pFatalErrorLinkTable             = {GEN_REPEAT_U(NULL_PTR, GEN_STRIP(DIAG_ID_MAX_FOR_INIT))},
};

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/
void DIAG_UpdateFlags(void) {
    DATA_WRITE_DATA(&diag_tableErrorFlags, &diag_tableMolFlags, &diag_tableRslFlags, &diag_tableMslFlags);
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
