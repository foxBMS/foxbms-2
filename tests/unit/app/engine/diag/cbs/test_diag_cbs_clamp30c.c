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
 * @file    test_diag_cbs_clamp30c.c
 * @author  foxBMS Team
 * @date    2024-05-22 (date of creation)
 * @updated 2025-03-31 (date of last update)
 * @version v1.9.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Test of the supply clamp 30C diag handler implementation.
 * @details TODO
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockdiag_cfg.h"

#include "diag_cbs.h"
#include "test_assert_helper.h"

#include <stdbool.h>
#include <stdint.h>

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("diag_cbs_clamp30c.c")

TEST_INCLUDE_PATH("../../src/app/engine/diag/cbs")

/*========== Definitions and Implementations for Unit Test ==================*/
/** local copy of the #DATA_BLOCK_ERROR_STATE_s table */
static DATA_BLOCK_ERROR_STATE_s test_tableErrorFlags = {.header.uniqueId = DATA_BLOCK_ID_ERROR_STATE};

const DIAG_DATABASE_SHIM_s diag_kpkDatabaseShim = {
    .pTableError = &test_tableErrorFlags,
};

/*========== Setup and Teardown =============================================*/
void setUp(void) {
    diag_kpkDatabaseShim.pTableError->supplyVoltageClamp30cError = false;
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
void testDIAG_SupplyVoltageClamp30c(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1: Assert */
    TEST_ASSERT_FAIL_ASSERT(DIAG_SupplyVoltageClamp30c(DIAG_ID_MAX, DIAG_EVENT_OK, &diag_kpkDatabaseShim, 0u));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: OK event -> database entry must not change */
    DIAG_SupplyVoltageClamp30c(DIAG_ID_SUPPLY_VOLTAGE_CLAMP_30C_LOST, DIAG_EVENT_OK, &diag_kpkDatabaseShim, 0u);
    TEST_ASSERT_FALSE(diag_kpkDatabaseShim.pTableError->supplyVoltageClamp30cError);

    /* ======= RT1/2: not ok event -> database entry must change to indicate an error, i.e., be set to true */
    DIAG_SupplyVoltageClamp30c(DIAG_ID_SUPPLY_VOLTAGE_CLAMP_30C_LOST, DIAG_EVENT_NOT_OK, &diag_kpkDatabaseShim, 0u);
    TEST_ASSERT_TRUE(diag_kpkDatabaseShim.pTableError->supplyVoltageClamp30cError);

    /* ======= RT1/3: reset event -> database entry must change back to no error, i.e., false */
    DIAG_SupplyVoltageClamp30c(DIAG_ID_SUPPLY_VOLTAGE_CLAMP_30C_LOST, DIAG_EVENT_RESET, &diag_kpkDatabaseShim, 0u);
    TEST_ASSERT_FALSE(diag_kpkDatabaseShim.pTableError->supplyVoltageClamp30cError);
}
