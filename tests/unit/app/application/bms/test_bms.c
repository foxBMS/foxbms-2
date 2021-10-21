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
 * @file    test_bms.c
 * @author  foxBMS Team
 * @date    2020-04-01 (date of creation)
 * @updated 2021-10-05 (date of last update)
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Tests for the bms driver implementation
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockafe.h"
#include "Mockbal.h"
#include "Mockbattery_system_cfg.h"
#include "Mockcontactor.h"
#include "Mockdatabase.h"
#include "Mockdiag.h"
#include "Mockfassert.h"
#include "Mockinterlock.h"
#include "Mockled.h"
#include "Mockmeas.h"
#include "Mockos.h"
#include "Mockplausibility.h"
#include "Mocksoa.h"

#include "bms.h"
#include "foxmath.h"
#include "test_assert_helper.h"

/*========== Definitions and Implementations for Unit Test ==================*/
DIAG_ID_CFG_s DIAG_ID_cfg[] = {};

DIAG_DEV_s diag_device = {
    .nr_of_ch            = sizeof(DIAG_ID_cfg) / sizeof(DIAG_ID_CFG_s),
    .ch_cfg              = &DIAG_ID_cfg[0],
    .numberOfFatalErrors = 0u,
};

BS_STRING_PRECHARGE_PRESENT_e bs_stringsWithPrecharge[BS_NR_OF_STRINGS] = {
    BS_STRING_WITH_PRECHARGE,
    BS_STRING_WITH_PRECHARGE,
    BS_STRING_WITH_PRECHARGE,
};

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
#define NUM_PRECHARGE_TESTS 13
STD_RETURN_TYPE_e prechargeExpectedResults[BS_NR_OF_STRINGS][NUM_PRECHARGE_TESTS] = {0};
/*
 * mock callback in order to provide custom values to current_tab
 */
STD_RETURN_TYPE_e MockDATA_ReadBlock_Callback(void *dataptrtoReceiver, int num_calls) {
    int32_t current   = 0;
    int32_t voltage_1 = 0;
    int32_t voltage_2 = 0;

    /* determine a value depending on num_calls (has to be synchronized with test) */
    switch (num_calls) {
        case 0:
            prechargeExpectedResults[0][0] = STD_OK;
            /* no current, no voltage difference --> expect OK */
            current   = 0;
            voltage_1 = 0;
            voltage_2 = 0;
            break;
        case 1:
            prechargeExpectedResults[0][1] = STD_NOT_OK;
            /* INT32_MAX current, no voltage difference --> expect NOK */
            current   = INT32_MAX;
            voltage_1 = 0;
            voltage_2 = 0;
            break;
        case 2:
            prechargeExpectedResults[0][2] = STD_NOT_OK;
            /* INT32_MIN current, no voltage difference --> expect NOK */
            current   = INT32_MIN;
            voltage_1 = 0;
            voltage_2 = 0;
            break;
        case 3:
            prechargeExpectedResults[0][3] = STD_OK;
            /* no current, no voltage difference --> expect OK */
            current   = 0;
            voltage_1 = INT32_MAX;
            voltage_2 = INT32_MAX;
            break;
        case 4:
            prechargeExpectedResults[0][4] = STD_OK;
            /* no current, no voltage difference --> expect OK */
            current   = 0;
            voltage_1 = INT32_MIN;
            voltage_2 = INT32_MIN;
            break;
        case 5:
            prechargeExpectedResults[0][5] = STD_NOT_OK;
            /* no current, maximum voltage difference --> expect NOK */
            current   = 0;
            voltage_1 = INT32_MAX;
            voltage_2 = INT32_MIN;
            break;
        case 6:
            prechargeExpectedResults[0][6] = STD_NOT_OK;
            /* no current, maximum voltage difference --> expect NOK */
            current   = 0;
            voltage_1 = INT32_MIN;
            voltage_2 = INT32_MAX;
            break;
        case 7:
            prechargeExpectedResults[0][7] = STD_NOT_OK;
            /* current exactly at threshold, no voltage difference --> expect NOK */
            current   = BMS_PRECHARGE_CURRENT_THRESHOLD_mA;
            voltage_1 = 0;
            voltage_2 = 0;
            break;
        case 8:
            prechargeExpectedResults[0][8] = STD_NOT_OK;
            /* no current, voltage difference exactly at threshold --> expect NOK */
            current   = 0;
            voltage_1 = BMS_PRECHARGE_VOLTAGE_THRESHOLD_mV;
            voltage_2 = 0;
            break;
        case 9:
            prechargeExpectedResults[0][9] = STD_NOT_OK;
            /* no current, voltage difference exactly at threshold --> expect NOK */
            current   = 0;
            voltage_1 = 0;
            voltage_2 = BMS_PRECHARGE_VOLTAGE_THRESHOLD_mV;
            break;
        case 10:
            prechargeExpectedResults[0][10] = STD_OK;
            /* current exactly 1 below threshold, no voltage difference --> expect OK */
            current   = BMS_PRECHARGE_CURRENT_THRESHOLD_mA - 1;
            voltage_1 = 0;
            voltage_2 = 0;
            break;
        case 11:
            prechargeExpectedResults[0][11] = STD_OK;
            /* no current, voltage difference exactly 1 below threshold --> expect OK */
            current   = 0;
            voltage_1 = BMS_PRECHARGE_VOLTAGE_THRESHOLD_mV - 1;
            voltage_2 = 0;
            break;
        case 12:
            prechargeExpectedResults[0][12] = STD_OK;
            /* no current, voltage difference exactly 1 below threshold --> expect OK */
            current   = 0;
            voltage_1 = 0;
            voltage_2 = BMS_PRECHARGE_VOLTAGE_THRESHOLD_mV - 1;
            break;
        default:
            TEST_FAIL_MESSAGE("DATA_ReadBlock_Callback was called too often");
    }
    /* ENTER HIGHEST CASE NUMBER IN EXPECT; checks whether all cases are used */
    TEST_ASSERT_EQUAL_MESSAGE(12, (NUM_PRECHARGE_TESTS - 1), "Check code of stub. Something does not fit.");

    if (num_calls >= NUM_PRECHARGE_TESTS) {
        TEST_FAIL_MESSAGE("This stub is fishy, prechargeExpectedResults is too short for the number of calls");
    }

    /* cast to correct struct in order to properly write current and other values */
    for (uint8_t s = 0; s < BS_NR_OF_STRINGS; s++) {
        for (uint8_t testNumber = 0; testNumber < NUM_PRECHARGE_TESTS; testNumber++) {
            prechargeExpectedResults[s][testNumber] = prechargeExpectedResults[0][testNumber];
        }

        ((DATA_BLOCK_PACK_VALUES_s *)dataptrtoReceiver)->stringCurrent_mA[0] = current;
        ((DATA_BLOCK_PACK_VALUES_s *)dataptrtoReceiver)->stringVoltage_mV[0] = voltage_1;
    }
    ((DATA_BLOCK_PACK_VALUES_s *)dataptrtoReceiver)->highVoltageBusVoltage_mV = voltage_2;

    return STD_OK;
}

/**
 * @brief   Iterate over a callback that supplies various scenarios and check if they work as expected
 * @details This function uses the callback #MockDATA_ReadBlock_Callback() in order to inject
 *          current tables and voltage tables into the returned database tables. The array
 *          #prechargeExpectedResults contains prepared return values against which the output
 *          of #TEST_BMS_CheckPrecharge() is compared.
 */
void testCheckPrechargeIterateStub(void) {
    /* tell CMock to use our callback */
    DATA_Read_1_DataBlock_Stub(MockDATA_ReadBlock_Callback);

    DATA_BLOCK_PACK_VALUES_s tablePackValues = {.header.uniqueId = DATA_BLOCK_ID_PACK_VALUES};

    /* iterate until we have all covered cases from our stub processed */
    for (uint8_t i = 0u; i < NUM_PRECHARGE_TESTS; i++) {
        char buffer[30];
        snprintf(buffer, 30, "Loop iteration %d.", i);
        for (uint8_t s = 0; s < BS_NR_OF_STRINGS; s++) {
            TEST_ASSERT_EQUAL_MESSAGE(
                prechargeExpectedResults[s][i], TEST_BMS_CheckPrecharge(s, &tablePackValues), buffer);
        }
    }
}

void testBMS_GetCurrentFlowDirectionWithTypicalValues(void) {
    /*
    WARNING: the function under test has code that is unaccessible
    in order to solve this situation it has to be refactored
    so that the branch in it does not always evaluate to true.

    However, the way it is implemented now, the unit test will be
    always valid for the currently active defines.
    */

#if (POSITIVE_DISCHARGE_CURRENT == true)
    /* discharge is positive */

    /* maximum positive current has to be discharge */
    TEST_ASSERT_EQUAL(BMS_DISCHARGING, BMS_GetCurrentFlowDirection(INT32_MAX));

    /* maximum negative current has to be charge */
    TEST_ASSERT_EQUAL(BMS_CHARGING, BMS_GetCurrentFlowDirection(INT32_MIN));
#else
    /* discharge is negative */

    /* maximum positive current has to be charge */
    TEST_ASSERT_EQUAL(BMS_CHARGING, BMS_GetCurrentFlowDirection(INT32_MAX));

    /* maximum negative current has to be discharge */
    TEST_ASSERT_EQUAL(BMS_DISCHARGING, BMS_GetCurrentFlowDirection(INT32_MIN));
#endif

    /* zero current has to be no charge */
    TEST_ASSERT_EQUAL(BMS_AT_REST, BMS_GetCurrentFlowDirection(0));

    /* positive current below/equal to resting current is no current too */
    TEST_ASSERT_EQUAL(BMS_AT_REST, BMS_GetCurrentFlowDirection(0 + BS_REST_CURRENT_mA - 1));

    /* negative current below/equal to resting current is no current too */
    TEST_ASSERT_EQUAL(BMS_AT_REST, BMS_GetCurrentFlowDirection(0 - BS_REST_CURRENT_mA + 1));

    /* function should have same behavior for #BS_CS_THRESHOLD_NO_CURRENT_mA */
    TEST_ASSERT_EQUAL(
        BMS_GetCurrentFlowDirection(0 - BS_CS_THRESHOLD_NO_CURRENT_mA + 1),
        BMS_GetCurrentFlowDirection(0 - BS_REST_CURRENT_mA + 1));
}

void testCheckCurrentValueDirectionWithCurrentZeroMaxAndMin(void) {
    /* Set the current to 0 */
    TEST_ASSERT_EQUAL(BMS_AT_REST, BMS_GetCurrentFlowDirection(0u));

    /* Set the current to #INT32_MAX */
#if (POSITIVE_DISCHARGE_CURRENT == true)
    TEST_ASSERT_EQUAL(BMS_DISCHARGING, BMS_GetCurrentFlowDirection(INT32_MAX));
#else
    TEST_ASSERT_EQUAL(BMS_CHARGING, BMS_GetCurrentFlowDirection(INT32_MAX));
#endif

    /* Set the current to #INT32_MIN */
#if (POSITIVE_DISCHARGE_CURRENT == true)
    TEST_ASSERT_EQUAL(BMS_CHARGING, BMS_GetCurrentFlowDirection(INT32_MIN));
#else
    TEST_ASSERT_EQUAL(BMS_DISCHARGING, BMS_GetCurrentFlowDirection(INT32_MIN));
#endif
}

/** check that invalid values to BMS_CheckPrecharge trip an assertion
 *
 * invalid values are all those that do not fall into
 * 0 <= stringNumber < #BS_NR_OF_STRINGS
 */
void testBMS_CheckPrechargeInvalidStringNumber(void) {
    DATA_BLOCK_PACK_VALUES_s tablePackValues = {.header.uniqueId = DATA_BLOCK_ID_PACK_VALUES};
    TEST_ASSERT_FAIL_ASSERT(TEST_BMS_CheckPrecharge(BS_NR_OF_STRINGS, &tablePackValues));
    TEST_ASSERT_FAIL_ASSERT(TEST_BMS_CheckPrecharge(BS_NR_OF_STRINGS + 1u, &tablePackValues));
    TEST_ASSERT_FAIL_ASSERT(TEST_BMS_CheckPrecharge(UINT8_MAX, &tablePackValues));

    TEST_ASSERT_PASS_ASSERT(TEST_BMS_CheckPrecharge(0u, &tablePackValues));
}
