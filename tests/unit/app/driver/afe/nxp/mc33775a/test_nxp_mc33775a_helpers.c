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
 * @file    test_nxp_mc33775a_helpers.c
 * @author  foxBMS Team
 * @date    2025-04-01 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Test of nxp_mc3377x_helpers.c
 * @details TODO
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockos.h"
#include "Mocktsi.h"

#include "nxp_mc3377x_helpers.h"

/* clang-format off */
#include "test_assert_helper.h"
/* clang-format on */

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("nxp_mc3377x_helpers.c")

TEST_INCLUDE_PATH("../../src/app/driver/afe/nxp/common/mc3377x")
TEST_INCLUDE_PATH("../../src/app/driver/afe/nxp/common/mc3377x/vendor")
TEST_INCLUDE_PATH("../../src/app/driver/afe/nxp/mc33775a")
TEST_INCLUDE_PATH("../../src/app/driver/afe/nxp/mc33775a/config")
TEST_INCLUDE_PATH("../../src/app/driver/afe/nxp/mc33775a/vendor")
TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/spi")
TEST_INCLUDE_PATH("../../src/app/driver/ts/api")

/*========== Definitions and Implementations for Unit Test ==================*/

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
void testN77x_ErrorHandling(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(N77x_ErrorHandling(NULL_PTR, N77X_COMMUNICATION_OK, 0u));

    /* ======= Routine tests =============================================== */
    N77X_ERROR_TABLE_s n77x_errorTable = {0};

    N77X_STATE_s n77xTestState = {
        .currentString       = 0u,
        .n77xData.errorTable = &n77x_errorTable,
    };

    /* ======= RT1/4: Test implementation */
    uint8_t currentModule = 0u;

    n77xTestState.n77xData.errorTable->communicationOk[n77xTestState.currentString][currentModule]        = false;
    n77xTestState.n77xData.errorTable->noCommunicationTimeout[n77xTestState.currentString][currentModule] = false;
    n77xTestState.n77xData.errorTable->crcIsValid[n77xTestState.currentString][currentModule]             = false;
    /* ======= RT1/4: call function under test */
    TEST_ASSERT_PASS_ASSERT(N77x_ErrorHandling(&n77xTestState, N77X_COMMUNICATION_OK, 0u));
    /* ======= RT1/4: test output verification */
    TEST_ASSERT_TRUE(n77xTestState.n77xData.errorTable->communicationOk[n77xTestState.currentString][currentModule]);
    TEST_ASSERT_TRUE(
        n77xTestState.n77xData.errorTable->noCommunicationTimeout[n77xTestState.currentString][currentModule]);
    TEST_ASSERT_TRUE(n77xTestState.n77xData.errorTable->crcIsValid[n77xTestState.currentString][currentModule]);

    /* ======= RT2/4: Test implementation */
    n77xTestState.n77xData.errorTable->communicationOk[n77xTestState.currentString][currentModule]        = true;
    n77xTestState.n77xData.errorTable->noCommunicationTimeout[n77xTestState.currentString][currentModule] = true;
    n77xTestState.n77xData.errorTable->crcIsValid[n77xTestState.currentString][currentModule]             = true;
    /* ======= RT2/4: call function under test */
    TEST_ASSERT_PASS_ASSERT(N77x_ErrorHandling(&n77xTestState, N77X_COMMUNICATION_ERROR_SHORT_MESSAGE, 0u));
    /* ======= RT2/4: test output verification */
    TEST_ASSERT_FALSE(n77xTestState.n77xData.errorTable->communicationOk[n77xTestState.currentString][currentModule]);
    TEST_ASSERT_TRUE(
        n77xTestState.n77xData.errorTable->noCommunicationTimeout[n77xTestState.currentString][currentModule]);
    TEST_ASSERT_TRUE(n77xTestState.n77xData.errorTable->crcIsValid[n77xTestState.currentString][currentModule]);

    /* ======= RT3/4: Test implementation */
    n77xTestState.n77xData.errorTable->communicationOk[n77xTestState.currentString][currentModule]        = true;
    n77xTestState.n77xData.errorTable->noCommunicationTimeout[n77xTestState.currentString][currentModule] = true;
    n77xTestState.n77xData.errorTable->crcIsValid[n77xTestState.currentString][currentModule]             = true;
    /* ======= RT3/4: call function under test */
    TEST_ASSERT_PASS_ASSERT(N77x_ErrorHandling(&n77xTestState, N77X_COMMUNICATION_ERROR_TIMEOUT, 0u));
    /* ======= RT3/4: test output verification */
    TEST_ASSERT_FALSE(n77xTestState.n77xData.errorTable->communicationOk[n77xTestState.currentString][currentModule]);
    TEST_ASSERT_FALSE(
        n77xTestState.n77xData.errorTable->noCommunicationTimeout[n77xTestState.currentString][currentModule]);
    TEST_ASSERT_TRUE(n77xTestState.n77xData.errorTable->crcIsValid[n77xTestState.currentString][currentModule]);

    /* ======= RT4/4: Test implementation */
    n77xTestState.n77xData.errorTable->communicationOk[n77xTestState.currentString][currentModule]        = true;
    n77xTestState.n77xData.errorTable->noCommunicationTimeout[n77xTestState.currentString][currentModule] = true;
    n77xTestState.n77xData.errorTable->crcIsValid[n77xTestState.currentString][currentModule]             = true;
    /* ======= RT4/4: call function under test */
    TEST_ASSERT_PASS_ASSERT(N77x_ErrorHandling(&n77xTestState, N77X_COMMUNICATION_ERROR_WRONG_CRC, 0u));
    /* ======= RT4/4: test output verification */
    TEST_ASSERT_FALSE(n77xTestState.n77xData.errorTable->communicationOk[n77xTestState.currentString][currentModule]);
    TEST_ASSERT_TRUE(
        n77xTestState.n77xData.errorTable->noCommunicationTimeout[n77xTestState.currentString][currentModule]);
    TEST_ASSERT_FALSE(n77xTestState.n77xData.errorTable->crcIsValid[n77xTestState.currentString][currentModule]);
}

void testN77x_Wait(void) {
    const uint32_t waitTime = 1u;
    uint32_t currentTime    = 2u;
    OS_GetTickCount_ExpectAndReturn(currentTime);
    OS_DelayTaskUntil_Expect(&currentTime, waitTime);
    TEST_ASSERT_PASS_ASSERT(N77x_Wait(waitTime));
}
