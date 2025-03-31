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
 * @file    test_soc_none.c
 * @author  foxBMS Team
 * @date    2020-10-14 (date of creation)
 * @updated 2025-03-31 (date of last update)
 * @version v1.9.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Tests for SOC module responsible for calculation of state-of-charge
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockdatabase.h"

#include "state_estimation.h"
#include "test_assert_helper.h"

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("soc_none.c")
TEST_SOURCE_FILE("soe_none.c")
TEST_SOURCE_FILE("soh_none.c")

TEST_INCLUDE_PATH("../../src/app/application/algorithm/state_estimation")
TEST_INCLUDE_PATH("../../src/app/task/config")

/*========== Definitions and Implementations for Unit Test ==================*/
DATA_BLOCK_SOC_s table_testSoc = {.header.uniqueId = DATA_BLOCK_ID_SOC};

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
/**
 * @brief   Testing extern function #SE_InitializeStateOfCharge
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/3: NULL_PTR for pSocValues -> assert
 *            - AT2/3: invalid energy counter option -> assert
 *            - AT3/3: invalid string number -> assert
 *          - Routine validation:
 *            - RT1/1: function shall do nothing
 */
void testSE_InitializeStateOfCharge(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/3 ======= */
    TEST_ASSERT_FAIL_ASSERT(SE_InitializeStateOfCharge(NULL_PTR, true, BS_NR_OF_STRINGS - 1u));
    /* ======= AT2/3 ======= */
    /* true/false not testable due to implicit cast */
    /* ======= AT3/3 ======= */
    TEST_ASSERT_FAIL_ASSERT(SE_InitializeStateOfCharge(&table_testSoc, true, BS_NR_OF_STRINGS));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    /* ======= RT1/1: call function under test */
    SE_InitializeStateOfCharge(&table_testSoc, true, BS_NR_OF_STRINGS - 1u);
    /* ======= RT1/1: test output verification */
    /* nothing to be verified */
}

/**
 * @brief   Testing extern function #SE_CalculateStateOfCharge
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: NULL_PTR for pSohValues -> assert
 *          - Routine validation:
 *            - RT1/1: function shall do nothing
 */
void testSE_CalculateStateOfCharge(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(SE_CalculateStateOfCharge(NULL_PTR));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    /* ======= RT1/1: call function under test */
    SE_CalculateStateOfCharge(&table_testSoc);
    /* ======= RT1/1: test output verification */
    /* nothing to be verified */
}

/**
 * @brief   Testing extern function #SE_GetStateOfChargeFromVoltage
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - none (void function)
 *          - Routine validation:
 *            - RT1/1: function shall return 0.0f
 */
void testSE_GetStateOfChargeFromVoltage(void) {
    /* ======= Assertion tests ============================================= */
    /* none */

    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    const float_t expectedSocFromVoltage = 0.0f;
    /* ======= RT1/1: call function under test */
    const float_t socFromVoltage = SE_GetStateOfChargeFromVoltage(1);
    /* ======= RT1/1: test output verification */
    TEST_ASSERT_EQUAL_FLOAT(expectedSocFromVoltage, socFromVoltage);
}
