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
 * @file    test_diag_cbs_plausibility.c
 * @author  foxBMS Team
 * @date    2021-02-17 (date of creation)
 * @updated 2025-03-31 (date of last update)
 * @version v1.9.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Test of the plausibility diag handler implementation.
 * @details Test functions:
 *          - testDIAG_ErrorPlausibilityInvalidInput
 *          - testDIAG_PlausibilityCheckInvalidInput
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockdiag_cfg.h"

#include "diag_cbs.h"
#include "test_assert_helper.h"

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("diag_cbs_plausibility.c")

TEST_INCLUDE_PATH("../../src/app/engine/diag/cbs")

/*========== Definitions and Implementations for Unit Test ==================*/
/** local copy of the #DATA_BLOCK_ERROR_STATE_s table */
static DATA_BLOCK_ERROR_STATE_s test_tableErrorFlags = {.header.uniqueId = DATA_BLOCK_ID_ERROR_STATE};

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
/** tests invalid input values */
void testDIAG_ErrorPlausibilityInvalidInput(void) {
    TEST_ASSERT_FAIL_ASSERT(DIAG_ErrorPlausibility(DIAG_ID_MAX, DIAG_EVENT_OK, &diag_kpkDatabaseShim, 0u));
    TEST_ASSERT_FAIL_ASSERT(DIAG_ErrorPlausibility(DIAG_ID_PLAUSIBILITY_PACK_VOLTAGE, 42, &diag_kpkDatabaseShim, 0u));
    TEST_ASSERT_FAIL_ASSERT(DIAG_ErrorPlausibility(DIAG_ID_PLAUSIBILITY_PACK_VOLTAGE, DIAG_EVENT_OK, NULL_PTR, 0u));
    TEST_ASSERT_FAIL_ASSERT(DIAG_ErrorPlausibility(
        DIAG_ID_PLAUSIBILITY_PACK_VOLTAGE, DIAG_EVENT_OK, &diag_kpkDatabaseShim, BS_NR_OF_STRINGS));
}

/** tests invalid input values */
void testDIAG_PlausibilityCheckInvalidInput(void) {
    TEST_ASSERT_FAIL_ASSERT(DIAG_PlausibilityCheck(DIAG_ID_MAX, DIAG_EVENT_OK, &diag_kpkDatabaseShim, 0u));
    TEST_ASSERT_FAIL_ASSERT(DIAG_PlausibilityCheck(DIAG_ID_PLAUSIBILITY_CELL_VOLTAGE, 42, &diag_kpkDatabaseShim, 0u));
    TEST_ASSERT_FAIL_ASSERT(DIAG_PlausibilityCheck(DIAG_ID_PLAUSIBILITY_CELL_VOLTAGE, DIAG_EVENT_OK, NULL_PTR, 0u));
    TEST_ASSERT_FAIL_ASSERT(DIAG_PlausibilityCheck(
        DIAG_ID_PLAUSIBILITY_CELL_VOLTAGE, DIAG_EVENT_OK, &diag_kpkDatabaseShim, BS_NR_OF_STRINGS));
}

void testDIAG_ErrorPlausibility(void) {
    DIAG_ID_e diagId   = DIAG_ID_PLAUSIBILITY_PACK_VOLTAGE;
    DIAG_EVENT_e event = DIAG_EVENT_RESET;
    DIAG_ErrorPlausibility(diagId, event, &diag_kpkDatabaseShim, 0u);
    TEST_ASSERT_FALSE(diag_kpkDatabaseShim.pTableError->plausibilityCheckPackVoltageError[0u]);

    event = DIAG_EVENT_NOT_OK;
    DIAG_ErrorPlausibility(diagId, event, &diag_kpkDatabaseShim, 0u);
    TEST_ASSERT_TRUE(diag_kpkDatabaseShim.pTableError->plausibilityCheckPackVoltageError[0u]);
}

void testDIAG_PlausibilityCheck(void) {
    DIAG_ID_e diagId   = DIAG_ID_PLAUSIBILITY_CELL_VOLTAGE;
    DIAG_EVENT_e event = DIAG_EVENT_RESET;
    DIAG_PlausibilityCheck(diagId, event, &diag_kpkDatabaseShim, 0u);

    diagId = DIAG_ID_PLAUSIBILITY_CELL_TEMP;
    DIAG_PlausibilityCheck(diagId, event, &diag_kpkDatabaseShim, 0u);

    diagId = DIAG_ID_PLAUSIBILITY_CELL_VOLTAGE_SPREAD;
    DIAG_PlausibilityCheck(diagId, event, &diag_kpkDatabaseShim, 0u);

    diagId = DIAG_ID_PLAUSIBILITY_CELL_TEMPERATURE_SPREAD;
    DIAG_PlausibilityCheck(diagId, event, &diag_kpkDatabaseShim, 0u);

    /* --- */
    event = DIAG_EVENT_NOT_OK;

    DIAG_PlausibilityCheck(diagId, event, &diag_kpkDatabaseShim, 0u);

    diagId = DIAG_ID_PLAUSIBILITY_CELL_VOLTAGE;
    DIAG_PlausibilityCheck(diagId, event, &diag_kpkDatabaseShim, 0u);

    diagId = DIAG_ID_PLAUSIBILITY_CELL_TEMP;
    DIAG_PlausibilityCheck(diagId, event, &diag_kpkDatabaseShim, 0u);

    diagId = DIAG_ID_PLAUSIBILITY_CELL_VOLTAGE_SPREAD;
    DIAG_PlausibilityCheck(diagId, event, &diag_kpkDatabaseShim, 0u);

    /* else branch */
    diagId = 10u;
    DIAG_PlausibilityCheck(diagId, event, &diag_kpkDatabaseShim, 0u);
}
