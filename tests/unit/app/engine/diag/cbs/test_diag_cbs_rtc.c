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
 * @file    test_diag_cbs_rtc.c
 * @author  foxBMS Team
 * @date    2022-11-25 (date of creation)
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Test of the RTC diag handler implementation.
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockdiag_cfg.h"

#include "diag_cbs.h"
#include "test_assert_helper.h"

#include <stdbool.h>
#include <stdint.h>

TEST_FILE("diag_cbs_rtc.c")

/*========== Definitions and Implementations for Unit Test ==================*/
/** local copy of the #DATA_BLOCK_ERROR_STATE_s table */
static DATA_BLOCK_ERROR_STATE_s test_tableErrorFlags = {.header.uniqueId = DATA_BLOCK_ID_ERROR_STATE};

const DIAG_DATABASE_SHIM_s diag_kpkDatabaseShim = {
    .pTableError = &test_tableErrorFlags,
};

/*========== Setup and Teardown =============================================*/
void setUp(void) {
    diag_kpkDatabaseShim.pTableError->rtcBatteryLowError     = false;
    diag_kpkDatabaseShim.pTableError->rtcClockIntegrityError = false;
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
void testDIAG_RtcDiagnosisCaseClockIntegrity(void) {
    /* ok event -> database entry must not change */
    DIAG_Rtc(DIAG_ID_RTC_CLOCK_INTEGRITY_ERROR, DIAG_EVENT_OK, &diag_kpkDatabaseShim, 0u);
    TEST_ASSERT_FALSE(diag_kpkDatabaseShim.pTableError->rtcClockIntegrityError);
    /* not ok event -> database entry must change to indicate an error, i.e., be set to true */
    DIAG_Rtc(DIAG_ID_RTC_CLOCK_INTEGRITY_ERROR, DIAG_EVENT_NOT_OK, &diag_kpkDatabaseShim, 0u);
    TEST_ASSERT_TRUE(diag_kpkDatabaseShim.pTableError->rtcClockIntegrityError);
    /* reset event -> database entry must change back to no error, i.e., false */
    DIAG_Rtc(DIAG_ID_RTC_CLOCK_INTEGRITY_ERROR, DIAG_EVENT_RESET, &diag_kpkDatabaseShim, 0u);
    TEST_ASSERT_FALSE(diag_kpkDatabaseShim.pTableError->rtcClockIntegrityError);
}

void testDIAG_RtcDiagnosisCaseBatteryLow(void) {
    /* ok event -> database entry must not change */
    DIAG_Rtc(DIAG_ID_RTC_BATTERY_LOW_ERROR, DIAG_EVENT_OK, &diag_kpkDatabaseShim, 0u);
    TEST_ASSERT_FALSE(diag_kpkDatabaseShim.pTableError->rtcBatteryLowError);
    /* not ok event -> database entry must change to indicate an error, i.e., be set to true */
    DIAG_Rtc(DIAG_ID_RTC_BATTERY_LOW_ERROR, DIAG_EVENT_NOT_OK, &diag_kpkDatabaseShim, 0u);
    TEST_ASSERT_TRUE(diag_kpkDatabaseShim.pTableError->rtcBatteryLowError);
    /* reset event -> database entry must change back to no error, i.e., false */
    DIAG_Rtc(DIAG_ID_RTC_BATTERY_LOW_ERROR, DIAG_EVENT_RESET, &diag_kpkDatabaseShim, 0u);
    TEST_ASSERT_FALSE(diag_kpkDatabaseShim.pTableError->rtcBatteryLowError);
}

void testDIAG_RtcInvalidInput(void) {
    /* assert on wrong diagnosis id */
    TEST_ASSERT_FAIL_ASSERT(DIAG_Rtc(DIAG_ID_MAX, DIAG_EVENT_OK, &diag_kpkDatabaseShim, 0u));

    /* assert on unsupported diagnosis event */
    TEST_ASSERT_FAIL_ASSERT(DIAG_Rtc(DIAG_ID_RTC_CLOCK_INTEGRITY_ERROR, 42, &diag_kpkDatabaseShim, 0u));

    /* assert if the database pointer is not valid */
    TEST_ASSERT_FAIL_ASSERT(DIAG_Rtc(DIAG_ID_RTC_CLOCK_INTEGRITY_ERROR, DIAG_EVENT_OK, NULL_PTR, 0u));

    /* parameter 'data' is ignored for this diagnosis */
}
