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
 * @file    test_soc_lookup-table.c
 * @author  foxBMS Team
 * @date    2025-07-07 (date of creation)
 * @updated 2026-04-20 (date of last update)
 * @version v1.11.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Tests for SOC module responsible for calculation of SOC
 * @details Tests Get state of charge from voltage
 *                check database soc percentage limit
 *                update nvm values
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockbms.h"
#include "Mockdatabase.h"
#include "Mockfram.h"

#include "battery_cell_cfg.h"
#include "soc_lookup-table_cfg.h"

#include "foxmath.h"
#include "state_estimation.h"
#include "test_assert_helper.h"

#include <math.h>

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("soc_lookup-table.c")
TEST_SOURCE_FILE("soe_none.c")
TEST_SOURCE_FILE("soh_none.c")

TEST_INCLUDE_PATH("../../src/app/application/algorithm/state_estimation")
TEST_INCLUDE_PATH("../../src/app/application/algorithm/state_estimation/soc/lookup-table")
TEST_INCLUDE_PATH("../../src/app/application/bms")
TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/contactor")
TEST_INCLUDE_PATH("../../src/app/driver/foxmath")
TEST_INCLUDE_PATH("../../src/app/driver/fram")
TEST_INCLUDE_PATH("../../src/app/driver/sps")
TEST_INCLUDE_PATH("../../src/app/task/config")

/*========== Definitions and Implementations for Unit Test ==================*/
FRAM_SOC_s fram_soc = {0};
/**local copy of DATA_BLOCK_SOC_s table**/
static DATA_BLOCK_SOC_s cp_pTableSoc    = {.header.uniqueId = DATA_BLOCK_ID_SOC};
static DATA_BLOCK_SOC_s cp_pTableMinMax = {.header.uniqueId = DATA_BLOCK_ID_MIN_MAX};
/** Maximum SOC in percentage */
#define SOC_MAXIMUM_SOC_perc (100.0f)
/** Minimum SOC in percentage */
#define SOC_MINIMUM_SOC_perc (0.0f)

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
void testSE_InitializeStateOfCharge(void) {
    TEST_ASSERT_FAIL_ASSERT(SE_InitializeStateOfCharge(NULL_PTR, true, 0));
    TEST_ASSERT_FAIL_ASSERT(SE_InitializeStateOfCharge(&cp_pTableSoc, true, BS_NR_OF_STRINGS));
    SE_InitializeStateOfCharge(&cp_pTableSoc, true, 0);
}

void testSE_CalculateStateOfCharge(void) {

    DATA_Read1DataBlock_ExpectAndReturn(&cp_pTableMinMax, STD_OK);
    static DATA_BLOCK_SOC_s tableMinMaxTimestampMismatch = {
        .header.uniqueId = DATA_BLOCK_ID_MIN_MAX, .header.timestamp = 10};
    DATA_Read1DataBlock_ReturnThruPtr_pDataToReceiver0(&tableMinMaxTimestampMismatch);
    FRAM_WriteData_ExpectAndReturn(FRAM_BLOCK_ID_SOC, FRAM_ACCESS_OK);
    SE_CalculateStateOfCharge(&cp_pTableSoc);
}
void testSE_GetStateOfChargeFromVoltage(void) {
    /* LUT values*/
    TEST_ASSERT_EQUAL(100.0f, SE_GetStateOfChargeFromVoltage(4123));
    TEST_ASSERT_EQUAL(50.0f, SE_GetStateOfChargeFromVoltage(3636));
    TEST_ASSERT_EQUAL(26.0f, SE_GetStateOfChargeFromVoltage(3461));
    TEST_ASSERT_EQUAL(1.0f, SE_GetStateOfChargeFromVoltage(2716));
    /* Minimum value */
    TEST_ASSERT_EQUAL(SOC_MINIMUM_SOC_perc, SE_GetStateOfChargeFromVoltage(2700));
    /* Maximum value */
    TEST_ASSERT_EQUAL(SOC_MAXIMUM_SOC_perc, SE_GetStateOfChargeFromVoltage(4200));
}

void testSOC_UpdateNvmValues(void) {
    TEST_ASSERT_FAIL_ASSERT(TEST_SOC_UpdateNvmValues(&cp_pTableSoc, BS_NR_OF_STRINGS));
    for (uint8_t s = 0; s < BS_NR_OF_STRINGS; s++) {
        TEST_ASSERT_FAIL_ASSERT(TEST_SOC_UpdateNvmValues(NULL_PTR, s));
        TEST_ASSERT_PASS_ASSERT(TEST_SOC_UpdateNvmValues(&cp_pTableSoc, s));

        cp_pTableSoc.averageSoc_perc[s] = 1.0f;
        TEST_SOC_UpdateNvmValues(&cp_pTableSoc, s);
        TEST_ASSERT_EQUAL_FLOAT(1.0f, fram_soc.averageSoc_perc[s]);

        cp_pTableSoc.minimumSoc_perc[s] = 1.0f;
        TEST_SOC_UpdateNvmValues(&cp_pTableSoc, s);
        TEST_ASSERT_EQUAL_FLOAT(1.0f, fram_soc.minimumSoc_perc[s]);

        cp_pTableSoc.maximumSoc_perc[s] = 1.0f;
        TEST_SOC_UpdateNvmValues(&cp_pTableSoc, s);
        TEST_ASSERT_EQUAL_FLOAT(1.0f, fram_soc.maximumSoc_perc[s]);

        cp_pTableSoc.chargeThroughput_As[s] = 1.0f;
        TEST_SOC_UpdateNvmValues(&cp_pTableSoc, s);
        TEST_ASSERT_EQUAL_FLOAT(1.0f, fram_soc.chargeThroughput_As[s]);

        cp_pTableSoc.dischargeThroughput_As[s] = 1.0f;
        TEST_SOC_UpdateNvmValues(&cp_pTableSoc, s);
        TEST_ASSERT_EQUAL_FLOAT(1.0f, fram_soc.dischargeThroughput_As[s]);
    }
}

void testSE_GetSocStateInitialized(void) {
    TEST_ASSERT_EQUAL(true, TEST_SE_GetSocStateInitialized());
}

void testSOC_CheckDatabaseSocPercentageLimits(void) {
    TEST_ASSERT_FAIL_ASSERT(TEST_SOC_CheckDatabaseSocPercentageLimits(&cp_pTableSoc, BS_NR_OF_STRINGS));
    for (uint8_t s = 0; s < BS_NR_OF_STRINGS; s++) {
        TEST_ASSERT_FAIL_ASSERT(TEST_SOC_CheckDatabaseSocPercentageLimits(NULL_PTR, s));
        TEST_ASSERT_PASS_ASSERT(TEST_SOC_CheckDatabaseSocPercentageLimits(&cp_pTableSoc, s));

        cp_pTableSoc.averageSoc_perc[s] = 101.0f;
        TEST_SOC_CheckDatabaseSocPercentageLimits(&cp_pTableSoc, s);
        TEST_ASSERT_EQUAL_FLOAT(SOC_MAXIMUM_SOC_perc, cp_pTableSoc.averageSoc_perc[s]);
        cp_pTableSoc.averageSoc_perc[s] = -1.0f;
        TEST_SOC_CheckDatabaseSocPercentageLimits(&cp_pTableSoc, s);
        TEST_ASSERT_EQUAL_FLOAT(SOC_MINIMUM_SOC_perc, cp_pTableSoc.averageSoc_perc[s]);

        cp_pTableSoc.minimumSoc_perc[s] = 101.0f;
        TEST_SOC_CheckDatabaseSocPercentageLimits(&cp_pTableSoc, s);
        TEST_ASSERT_EQUAL_FLOAT(SOC_MAXIMUM_SOC_perc, cp_pTableSoc.minimumSoc_perc[s]);
        cp_pTableSoc.minimumSoc_perc[s] = -1.0f;
        TEST_SOC_CheckDatabaseSocPercentageLimits(&cp_pTableSoc, s);
        TEST_ASSERT_EQUAL_FLOAT(SOC_MINIMUM_SOC_perc, cp_pTableSoc.minimumSoc_perc[s]);

        cp_pTableSoc.maximumSoc_perc[s] = 101.0f;
        TEST_SOC_CheckDatabaseSocPercentageLimits(&cp_pTableSoc, s);
        TEST_ASSERT_EQUAL_FLOAT(SOC_MAXIMUM_SOC_perc, cp_pTableSoc.maximumSoc_perc[s]);
        cp_pTableSoc.maximumSoc_perc[s] = -1.0f;
        TEST_SOC_CheckDatabaseSocPercentageLimits(&cp_pTableSoc, s);
        TEST_ASSERT_EQUAL_FLOAT(SOC_MINIMUM_SOC_perc, cp_pTableSoc.maximumSoc_perc[s]);
    }
}
