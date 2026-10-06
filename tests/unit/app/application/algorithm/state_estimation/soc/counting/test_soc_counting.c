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
 * @file    test_soc_counting.c
 * @author  foxBMS Team
 * @date    2020-10-07 (date of creation)
 * @updated 2026-10-06 (date of last update)
 * @version v1.12.0
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

#include "battery_system_cfg.h"
#include "soc_counting_cfg.h"

#include "battery_cell_cfg_types.h"
#include "foxmath.h"
#include "state_estimation.h"
#include "test_assert_helper.h"

#include <math.h>

/*========== Unit Testing Framework Directives ==============================*/

/*========== Definitions and Implementations for Unit Test ==================*/
FRAM_SOC_s fram_soc = {0};
/**local copy of DATA_BLOCK_SOC_s table**/
static DATA_BLOCK_SOC_s cp_pTableSoc                           = {.header.uniqueId = DATA_BLOCK_ID_SOC};
static DATA_BLOCK_CURRENT_COUNTER_s cp_soc_tableCurrentCounter = {.header.uniqueId = DATA_BLOCK_ID_CURRENT_COUNTER};

/** Maximum SOC in percentage */
#define SOC_MAXIMUM_SOC_perc (100.0f)
/** Minimum SOC in percentage */
#define SOC_MINIMUM_SOC_perc (0.0f)

static SOC_STATE_s *soc_state = NULL_PTR;

/*========== Setup and Teardown =============================================*/
void setUp(void) {
    soc_state = TEST_GetSocStateValues();
}
void tearDown(void) {
}

/*========== Test Cases =====================================================*/
void testSE_GetStateOfChargeFromVoltage(void) {
    float_t test_soc        = -1.0f;
    int16_t test_voltage_mV = 3780;
    test_soc                = SE_GetStateOfChargeFromVoltage(test_voltage_mV);
    TEST_ASSERT_EQUAL(64.0f, test_soc);
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

void testSOC_SetValue(void) {
    TEST_ASSERT_FAIL_ASSERT(TEST_SOC_SetValue(NULL_PTR, 30.0f, 70.0f, 50.0f, 0u));

    /*test if SOC percentage values are set*/
    DATA_Read1DataBlock_ExpectAndReturn(&cp_soc_tableCurrentCounter, STD_OK);
    FRAM_WriteData_ExpectAndReturn(FRAM_BLOCK_ID_SOC, FRAM_ACCESS_OK);

    TEST_SOC_SetValue(&cp_pTableSoc, 30.0f, 70.0f, 50.0f, 0);
    TEST_ASSERT_EQUAL_FLOAT(30.0f, cp_pTableSoc.minimumSoc_perc[0]);
    TEST_ASSERT_EQUAL_FLOAT(70.0f, cp_pTableSoc.maximumSoc_perc[0]);
    TEST_ASSERT_EQUAL_FLOAT(50.0f, cp_pTableSoc.averageSoc_perc[0]);

    /*test if percentage upper limits are applied*/
    DATA_Read1DataBlock_ExpectAndReturn(&cp_soc_tableCurrentCounter, STD_OK);
    FRAM_WriteData_ExpectAndReturn(FRAM_BLOCK_ID_SOC, FRAM_ACCESS_OK);

    TEST_SOC_SetValue(&cp_pTableSoc, 330.0f, 110.0f, 150.0f, 0);
    TEST_ASSERT_EQUAL_FLOAT(100.0f, cp_pTableSoc.minimumSoc_perc[0]);
    TEST_ASSERT_EQUAL_FLOAT(100.0f, cp_pTableSoc.maximumSoc_perc[0]);
    TEST_ASSERT_EQUAL_FLOAT(100.0f, cp_pTableSoc.averageSoc_perc[0]);

    /*test if percentage lower limits are applied*/
    DATA_Read1DataBlock_ExpectAndReturn(&cp_soc_tableCurrentCounter, STD_OK);
    FRAM_WriteData_ExpectAndReturn(FRAM_BLOCK_ID_SOC, FRAM_ACCESS_OK);

    TEST_SOC_SetValue(&cp_pTableSoc, -30.0f, -110.0f, -50.0f, 0);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, cp_pTableSoc.minimumSoc_perc[0]);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, cp_pTableSoc.maximumSoc_perc[0]);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, cp_pTableSoc.averageSoc_perc[0]);

    /*test if ccScaling Values are set*/
    DATA_Read1DataBlock_ExpectAndReturn(&cp_soc_tableCurrentCounter, STD_OK);
    FRAM_WriteData_ExpectAndReturn(FRAM_BLOCK_ID_SOC, FRAM_ACCESS_OK);

    soc_state->sensorCcUsed[0] = true;

    TEST_SOC_SetValue(&cp_pTableSoc, 30.0f, 70.0f, 50.0f, 0);
    TEST_ASSERT_EQUAL_FLOAT(30.0f, cp_pTableSoc.minimumSoc_perc[0]);
    TEST_ASSERT_EQUAL_FLOAT(70.0f, cp_pTableSoc.maximumSoc_perc[0]);
    TEST_ASSERT_EQUAL_FLOAT(50.0f, cp_pTableSoc.averageSoc_perc[0]);

    TEST_ASSERT_EQUAL_FLOAT(30.0f, soc_state->ccScalingMinimum[0]);
    TEST_ASSERT_EQUAL_FLOAT(70.0f, soc_state->ccScalingMaximum[0]);
    TEST_ASSERT_EQUAL_FLOAT(50.0f, soc_state->ccScalingAverage[0]);
}

void testSOC_GetStringSocPercentageFromCharge(void) {

    /* test with realistic positive and negative values */
    float_t ccOffset_perc = TEST_SOC_GetStringSocPercentageFromCharge(600);
    TEST_ASSERT_EQUAL_FLOAT(4.7619, ccOffset_perc);

    ccOffset_perc = TEST_SOC_GetStringSocPercentageFromCharge(-6000);
    TEST_ASSERT_EQUAL_FLOAT(-47.619, ccOffset_perc);

    /* test with maximum and minimum and negative values */
    ccOffset_perc = TEST_SOC_GetStringSocPercentageFromCharge(INT32_MAX);
    TEST_ASSERT_EQUAL_FLOAT(17043520, ccOffset_perc);

    ccOffset_perc = TEST_SOC_GetStringSocPercentageFromCharge(INT32_MIN);
    TEST_ASSERT_EQUAL_FLOAT(-17043520, ccOffset_perc);
}
