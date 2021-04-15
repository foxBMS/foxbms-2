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
 * @file    test_diag_cbs_sys-mon.c
 * @author  foxBMS Team
 * @date    2021-02-17 (date of creation)
 * @updated 2021-02-17 (date of last update)
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Test of the system monitoring diag handler implementation.
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockdiag_cfg.h"

#include "sys_mon_cfg.h"

#include "diag_cbs.h"
#include "test_assert_helper.h"

TEST_FILE("diag_cbs_sys-mon.c")

/*========== Definitions and Implementations for Unit Test ==================*/
/** local copy of the #DATA_BLOCK_ERRORSTATE_s table */
static DATA_BLOCK_ERRORSTATE_s test_tableErrorFlags = {.header.uniqueId = DATA_BLOCK_ID_ERRORSTATE};

/** local copy of the #DATA_BLOCK_MOL_FLAG_s table */
static DATA_BLOCK_MOL_FLAG_s test_tableMolFlags = {.header.uniqueId = DATA_BLOCK_ID_MOL_FLAG};

/** local copy of the #DATA_BLOCK_RSL_FLAG_s table */
static DATA_BLOCK_RSL_FLAG_s test_tableRslFlags = {.header.uniqueId = DATA_BLOCK_ID_RSL_FLAG};

/** local copy of the #DATA_BLOCK_MSL_FLAG_s table */
static DATA_BLOCK_MSL_FLAG_s test_tableMslFlags = {.header.uniqueId = DATA_BLOCK_ID_MSL_FLAG};

const DIAG_DATABASE_SHIM_s diag_kpkDatabaseShim = {
    .pTableError = &test_tableErrorFlags,
    .pTableMol   = &test_tableMolFlags,
    .pTableRsl   = &test_tableRslFlags,
    .pTableMsl   = &test_tableMslFlags,
};

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/

void testDIAG_ErrorSystemMonitoring(void) {
    /* reset event sets the CAN timing back in ok mode */
    SYSM_TASK_ID_e tasks[] = {
        SYSM_TASK_ID_ENGINE,
        SYSM_TASK_ID_CYCLIC_1ms,
        SYSM_TASK_ID_CYCLIC_10ms,
        SYSM_TASK_ID_CYCLIC_100ms,
        SYSM_TASK_ID_CYCLIC_ALGORITHM_100ms,
    };

    const uint8_t numberOfTasks = sizeof(tasks) / sizeof(tasks[0]);
    for (uint8_t i = 0; i < numberOfTasks; i++) {
        DIAG_ErrorSystemMonitoring(DIAG_ID_SYSTEMMONITORING, DIAG_EVENT_RESET, &diag_kpkDatabaseShim, i);
        DIAG_ErrorSystemMonitoring(DIAG_ID_SYSTEMMONITORING, DIAG_EVENT_NOT_OK, &diag_kpkDatabaseShim, i);
        DIAG_ErrorSystemMonitoring(DIAG_ID_SYSTEMMONITORING, DIAG_EVENT_OK, &diag_kpkDatabaseShim, i);
    }
}

void testDIAG_DoNothingOnWrongIdSystemMonitoringModule(void) {
    /* Use a wrong ID to make sure, that this does not alter the CAN entry */
    DIAG_ErrorSystemMonitoring(DIAG_ID_CELLVOLTAGE_OVERVOLTAGE_RSL, DIAG_EVENT_RESET, &diag_kpkDatabaseShim, 0u);
}

/** test against invalid input */
void testDIAG_ErrorSystemMonitoringInvalidInput0(void) {
    /* check the task ID is allowed */
    TEST_ASSERT_FAIL_ASSERT(
        DIAG_ErrorSystemMonitoring(DIAG_ID_SYSTEMMONITORING, DIAG_EVENT_OK, &diag_kpkDatabaseShim, 42u));
}
void testDIAG_ErrorSystemMonitoringInvalidInput1(void) {
    TEST_ASSERT_FAIL_ASSERT(DIAG_ErrorSystemMonitoring(DIAG_ID_MAX, DIAG_EVENT_OK, &diag_kpkDatabaseShim, 0u));
    TEST_ASSERT_FAIL_ASSERT(DIAG_ErrorSystemMonitoring(DIAG_ID_SYSTEMMONITORING, 42, &diag_kpkDatabaseShim, 0u));
    TEST_ASSERT_FAIL_ASSERT(DIAG_ErrorSystemMonitoring(DIAG_ID_SYSTEMMONITORING, DIAG_EVENT_OK, NULL_PTR, 0u));
}
