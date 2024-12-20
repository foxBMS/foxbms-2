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
 * @file    test_diag.c
 * @author  foxBMS Team
 * @date    2020-04-02 (date of creation)
 * @updated 2024-12-20 (date of last update)
 * @version v1.8.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Tests for the diag module
 * @details Test functions:
 *          - testDIAG_Reset
 *          - testDIAG_Initialize
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockcan_cbs_tx_fatal-error.h"
#include "Mockdatabase.h"
#include "Mockdiag_cbs.h"
#include "Mocktimer.h"

#include "diag_cfg.h"

#include "diag.h"
#include "test_assert_helper.h"

/*========== Unit Testing Framework Directives ==============================*/
TEST_INCLUDE_PATH("../../src/app/engine/diag")
TEST_INCLUDE_PATH("../../src/app/engine/diag/cbs")
TEST_INCLUDE_PATH("../../src/app/driver/can/cbs/tx-async")
TEST_INCLUDE_PATH("../../src/os/freertos")
TEST_INCLUDE_PATH("../../src/app/task/timer")

/*========== Definitions and Implementations for Unit Test ==================*/

/*========== Setup and Teardown =============================================*/
void setUp(void) {
    TEST_DIAG_SetActiveFatalErrorArray(0);
    TEST_DIAG_SetActiveFatalErrorCounter(0);
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
void testDIAG_ResetErrorCount(void) {
    /* ======= Assertion tests ============================================= */

    /* ======= AT1/1 ======= */
    TEST_DIAG_SetDiagerrcnttotal(3u);
    TEST_DIAG_Reset();
    TEST_ASSERT_EQUAL(0u, TEST_DIAG_GetDiag()->errcnttotal);
}

void testDIAG_ResetOccurrenceCounter(void) {
    TEST_DIAG_SetDiagOccurrenceCounter(3u);
    TEST_DIAG_Reset();
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        for (uint32_t i = 0u; i < DIAG_ID_MAX; i++) {
            TEST_ASSERT_EQUAL(0u, TEST_DIAG_GetDiag()->occurrenceCounter[s][i]);
        }
    }
}

void testDIAG_SetFatalErrorByIdOutOfRange(void) {
    TEST_ASSERT_FAIL_ASSERT(TEST_DIAG_SetFatalErrorById(DIAG_ID_MAX + 1));
}

void testDIAG_SetFatalErrorByIdOnce(void) {
    CANTX_SendFatalErrorId_ExpectAndReturn(DIAG_ID_FLASHCHECKSUM, STD_OK);
    TIMER_Start_IgnoreAndReturn(STD_OK);
    TEST_DIAG_SetFatalErrorById(DIAG_ID_FLASHCHECKSUM);
    TEST_ASSERT_EQUAL(TEST_DIAG_GetFatalErrorCount(), 1);
    TEST_ASSERT_EQUAL(TEST_DIAG_GetFatalErrorArrayCount(DIAG_ID_FLASHCHECKSUM), 1);
}

void testDIAG_SetFatalErrorByIdDoubled(void) {
    CANTX_SendFatalErrorId_ExpectAndReturn(DIAG_ID_FLASHCHECKSUM, STD_OK);
    TIMER_Start_IgnoreAndReturn(STD_OK);
    TEST_DIAG_SetFatalErrorById(DIAG_ID_FLASHCHECKSUM);
    TEST_ASSERT_EQUAL(TEST_DIAG_GetFatalErrorCount(), 1);
    TEST_ASSERT_EQUAL(TEST_DIAG_GetFatalErrorArrayCount(DIAG_ID_FLASHCHECKSUM), 1);

    TEST_DIAG_SetFatalErrorById(DIAG_ID_FLASHCHECKSUM);
    TEST_ASSERT_EQUAL(TEST_DIAG_GetFatalErrorCount(), 1);
    TEST_ASSERT_EQUAL(TEST_DIAG_GetFatalErrorArrayCount(DIAG_ID_FLASHCHECKSUM), 1);
}

void testDIAG_ClearFatalErrorByIdOutOfRange(void) {
    TEST_ASSERT_FAIL_ASSERT(TEST_DIAG_ClearFatalErrorById(DIAG_ID_MAX + 1));
}

void testDIAG_ClearFatalErrorByIdOnce(void) {
    CANTX_SendFatalErrorId_ExpectAndReturn(DIAG_ID_FLASHCHECKSUM, STD_OK);
    TIMER_Start_IgnoreAndReturn(STD_OK);
    TEST_DIAG_SetFatalErrorById(DIAG_ID_FLASHCHECKSUM);
    TEST_ASSERT_EQUAL(TEST_DIAG_GetFatalErrorCount(), 1);
    TEST_ASSERT_EQUAL(TEST_DIAG_GetFatalErrorArrayCount(DIAG_ID_FLASHCHECKSUM), 1);

    TIMER_Stop_IgnoreAndReturn(STD_OK);
    CANTX_SendFatalErrorId_ExpectAndReturn(DIAG_ID_MAX, STD_OK);
    TEST_DIAG_ClearFatalErrorById(DIAG_ID_FLASHCHECKSUM);
    TEST_ASSERT_EQUAL(TEST_DIAG_GetFatalErrorCount(), 0);
    TEST_ASSERT_EQUAL(TEST_DIAG_GetFatalErrorArrayCount(DIAG_ID_FLASHCHECKSUM), 0);
}

void testDIAG_ClearFatalErrorByIdDoubled(void) {
    CANTX_SendFatalErrorId_ExpectAndReturn(DIAG_ID_FLASHCHECKSUM, STD_OK);
    TIMER_Start_IgnoreAndReturn(STD_OK);
    TEST_DIAG_SetFatalErrorById(DIAG_ID_FLASHCHECKSUM);
    TEST_ASSERT_EQUAL(TEST_DIAG_GetFatalErrorCount(), 1);
    TEST_ASSERT_EQUAL(TEST_DIAG_GetFatalErrorArrayCount(DIAG_ID_FLASHCHECKSUM), 1);

    TIMER_Stop_IgnoreAndReturn(STD_OK);
    CANTX_SendFatalErrorId_ExpectAndReturn(DIAG_ID_MAX, STD_OK);
    TEST_DIAG_ClearFatalErrorById(DIAG_ID_FLASHCHECKSUM);
    TEST_ASSERT_EQUAL(TEST_DIAG_GetFatalErrorCount(), 0);
    TEST_ASSERT_EQUAL(TEST_DIAG_GetFatalErrorArrayCount(DIAG_ID_FLASHCHECKSUM), 0);

    TEST_DIAG_ClearFatalErrorById(DIAG_ID_FLASHCHECKSUM);
    TEST_ASSERT_EQUAL(TEST_DIAG_GetFatalErrorCount(), 0);
    TEST_ASSERT_EQUAL(TEST_DIAG_GetFatalErrorArrayCount(DIAG_ID_FLASHCHECKSUM), 0);
}

void testDIAG_ClearFatalErrorByIdNotSetBefore(void) {
    TEST_DIAG_ClearFatalErrorById(DIAG_ID_FLASHCHECKSUM);
    TEST_ASSERT_EQUAL(TEST_DIAG_GetFatalErrorCount(), 0);
    TEST_ASSERT_EQUAL(TEST_DIAG_GetFatalErrorArrayCount(DIAG_ID_FLASHCHECKSUM), 0);
}

void test_DIAG_ResendFatalErrorsThree(void) {
    CANTX_SendFatalErrorId_ExpectAndReturn(DIAG_ID_FLASHCHECKSUM, STD_OK);
    TIMER_Start_IgnoreAndReturn(STD_OK);
    TEST_DIAG_SetFatalErrorById(DIAG_ID_FLASHCHECKSUM);

    CANTX_SendFatalErrorId_ExpectAndReturn(DIAG_ID_SYSTEM_MONITORING, STD_OK);
    TEST_DIAG_SetFatalErrorById(DIAG_ID_SYSTEM_MONITORING);

    CANTX_SendFatalErrorId_ExpectAndReturn(DIAG_ID_AFE_SPI, STD_OK);
    TEST_DIAG_SetFatalErrorById(DIAG_ID_AFE_SPI);

    CANTX_SendFatalErrorId_ExpectAndReturn(DIAG_ID_FLASHCHECKSUM, STD_OK);
    CANTX_SendFatalErrorId_ExpectAndReturn(DIAG_ID_SYSTEM_MONITORING, STD_OK);
    CANTX_SendFatalErrorId_ExpectAndReturn(DIAG_ID_AFE_SPI, STD_OK);
    TEST_DIAG_ResendFatalErrors();
}

void test_DIAG_ResendFatalErrorsNone(void) {
    TEST_DIAG_ResendFatalErrors();
}

void testDIAG_Initialize(void) {
    /* ======= Assertion tests ============================================= */

    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(DIAG_Initialize(NULL_PTR));
}

void testDiag_UpdateFlags(void) {
    /* ======= Assertion tests ============================================= */

    /* ======= AT1/1 ======= */
    DATA_Write4DataBlocks_IgnoreAndReturn(STD_OK);
    TEST_ASSERT_PASS_ASSERT(DIAG_UpdateFlags());
}

void testDIAG_SendOneFatalError(void) {
    TIMER_Create_IgnoreAndReturn(STD_OK);
    TEST_ASSERT_EQUAL(DIAG_Initialize(&diag_device), STD_OK);
    TEST_ASSERT_FALSE(DIAG_IsAnyFatalErrorSet());

    CANTX_SendFatalErrorId_ExpectAndReturn(DIAG_ID_FLASHCHECKSUM, STD_OK);
    DIAG_DummyCallback_Expect(DIAG_ID_FLASHCHECKSUM, DIAG_EVENT_NOT_OK, &diag_kDatabaseShim, 0);
    TIMER_Start_IgnoreAndReturn(STD_OK);
    DIAG_Handler(DIAG_ID_FLASHCHECKSUM, DIAG_EVENT_NOT_OK, DIAG_SYSTEM, 0);

    TEST_ASSERT(DIAG_IsAnyFatalErrorSet());
}

void testDIAG_SendMultipleFatalErrors(void) {
    TEST_DIAG_Reset();
    TIMER_Create_IgnoreAndReturn(STD_OK);
    TEST_ASSERT_EQUAL(DIAG_Initialize(&diag_device), STD_OK);

    TEST_ASSERT_FALSE(DIAG_IsAnyFatalErrorSet());

    TIMER_Start_IgnoreAndReturn(STD_OK);
    CANTX_SendFatalErrorId_ExpectAndReturn(DIAG_ID_SYSTEM_MONITORING, STD_OK);
    DIAG_ErrorSystemMonitoring_Expect(DIAG_ID_SYSTEM_MONITORING, DIAG_EVENT_NOT_OK, &diag_kDatabaseShim, 0);
    DIAG_Handler(DIAG_ID_SYSTEM_MONITORING, DIAG_EVENT_NOT_OK, DIAG_SYSTEM, 0);

    CANTX_SendFatalErrorId_ExpectAndReturn(DIAG_ID_POWER_MEASUREMENT_ERROR, STD_OK);
    DIAG_ErrorPowerMeasurement_Expect(DIAG_ID_POWER_MEASUREMENT_ERROR, DIAG_EVENT_NOT_OK, &diag_kDatabaseShim, 0);
    DIAG_Handler(DIAG_ID_POWER_MEASUREMENT_ERROR, DIAG_EVENT_NOT_OK, DIAG_SYSTEM, 0);

    TEST_ASSERT(DIAG_IsAnyFatalErrorSet());
    TEST_ASSERT_EQUAL(2, TEST_DIAG_GetFatalErrorCount());
}
