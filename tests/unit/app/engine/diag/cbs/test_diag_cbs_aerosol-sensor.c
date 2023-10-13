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
 * @file    test_diag_cbs_aerosol-sensor.c
 * @author  foxBMS Team
 * @date    2023-09-06 (date of creation)
 * @updated 2023-10-12 (date of last update)
 * @version v1.6.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Test of the current sensor diag handler implementation.
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockdiag_cfg.h"

#include "diag_cbs.h"
#include "test_assert_helper.h"

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("diag_cbs_aerosol-sensor.c")

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
/**
 * @brief   Testing DIAG_AerosolAlert
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/3: invalid diagnosis event ID for diagId -> assert
 *            - AT2/3: invalid event for event  -> assert
 *            - AT3/3: NULL_PTR for kpkDiagShim -> assert
 *          - Routine validation:
 *            - RT1/4: Function keeps alter value on false
 *            - RT2/4: Function sets alert value in the struct to true
 *            - RT3/4: Function keeps alter value on true after an error
 *                     occurred
 *            - RT4/4: Function resets alter value in the struct to from true
 *                     to false
 */
void testDIAG_AerosolSensorIllegalInput(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/3 ======= */
    TEST_ASSERT_FAIL_ASSERT(DIAG_AerosolAlert(DIAG_ID_MAX, DIAG_EVENT_OK, &diag_kpkDatabaseShim, 0u));
    /* ======= AT2/3 ======= */
    TEST_ASSERT_FAIL_ASSERT(DIAG_AerosolAlert(DIAG_ID_AEROSOL_ALERT, 42, &diag_kpkDatabaseShim, 0u));
    /* ======= AT3/3 ======= */
    TEST_ASSERT_FAIL_ASSERT(DIAG_AerosolAlert(DIAG_ID_AEROSOL_ALERT, DIAG_EVENT_OK, NULL_PTR, 0u));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/4: Test implementation */
    diag_kpkDatabaseShim.pTableError->aerosolAlert = false;
    /* ======= RT1/4: call function under test */
    DIAG_AerosolAlert(DIAG_ID_AEROSOL_ALERT, DIAG_EVENT_OK, &diag_kpkDatabaseShim, 0u);
    /* ======= RT1/4: test output verification */
    TEST_ASSERT_FALSE(diag_kpkDatabaseShim.pTableError->aerosolAlert);

    /* ======= RT2/4: Test implementation */
    diag_kpkDatabaseShim.pTableError->aerosolAlert = false;
    /* ======= RT2/4: call function under test */
    DIAG_AerosolAlert(DIAG_ID_AEROSOL_ALERT, DIAG_EVENT_NOT_OK, &diag_kpkDatabaseShim, 0u);
    /* ======= RT2/4: test output verification */
    TEST_ASSERT_TRUE(diag_kpkDatabaseShim.pTableError->aerosolAlert);

    /* ======= RT3/4: Test implementation */
    /* ======= RT3/4: call function under test */
    DIAG_AerosolAlert(DIAG_ID_AEROSOL_ALERT, DIAG_EVENT_OK, &diag_kpkDatabaseShim, 0u);
    /* ======= RT3/4: test output verification */
    TEST_ASSERT_TRUE(diag_kpkDatabaseShim.pTableError->aerosolAlert);

    /* ======= RT4/4: Test implementation */
    /* ======= RT4/4: call function under test */
    DIAG_AerosolAlert(DIAG_ID_AEROSOL_ALERT, DIAG_EVENT_RESET, &diag_kpkDatabaseShim, 0u);
    /* ======= RT4/4: test output verification */
    TEST_ASSERT_FALSE(diag_kpkDatabaseShim.pTableError->aerosolAlert);
}
