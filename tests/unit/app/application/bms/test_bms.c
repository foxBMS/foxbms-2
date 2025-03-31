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
 * @file    test_bms.c
 * @author  foxBMS Team
 * @date    2020-04-01 (date of creation)
 * @updated 2025-03-31 (date of last update)
 * @version v1.9.0
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
#include "Mockcan_cbs_tx_cyclic.h"
#include "Mockcontactor.h"
#include "Mockdatabase.h"
#include "Mockdiag.h"
#include "Mockfassert.h"
#include "Mockimd.h"
#include "Mockinterlock.h"
#include "Mockled.h"
#include "Mockmeas.h"
#include "Mockos.h"
#include "Mockplausibility.h"
#include "Mocksoa.h"
#include "Mocksps.h"

#include "database_cfg.h"
#include "sps_cfg.h"

#include "bms.h"
#include "diag.h"
#include "foxmath.h"
#include "test_assert_helper.h"

#include <stdbool.h>

/*========== Unit Testing Framework Directives ==============================*/
TEST_INCLUDE_PATH("../../src/app/application/bal")
TEST_INCLUDE_PATH("../../src/app/application/bms")
TEST_INCLUDE_PATH("../../src/app/application/plausibility")
TEST_INCLUDE_PATH("../../src/app/application/soa")
TEST_INCLUDE_PATH("../../src/app/driver/afe/api")
TEST_INCLUDE_PATH("../../src/app/driver/can")
TEST_INCLUDE_PATH("../../src/app/driver/can/cbs")
TEST_INCLUDE_PATH("../../src/app/driver/can/cbs/tx-cyclic")
TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/contactor")
TEST_INCLUDE_PATH("../../src/app/driver/foxmath")
TEST_INCLUDE_PATH("../../src/app/driver/fram")
TEST_INCLUDE_PATH("../../src/app/driver/imd")
TEST_INCLUDE_PATH("../../src/app/driver/interlock")
TEST_INCLUDE_PATH("../../src/app/driver/led")
TEST_INCLUDE_PATH("../../src/app/driver/meas")
TEST_INCLUDE_PATH("../../src/app/driver/sps")
TEST_INCLUDE_PATH("../../src/app/engine/diag")
TEST_INCLUDE_PATH("../../src/app/engine/sys_mon")
TEST_INCLUDE_PATH("../../src/app/task/config")

/*========== Definitions and Implementations for Unit Test ==================*/

DIAG_ID_CFG_s diag_diagnosisIdConfiguration[] = {0};

DIAG_DEV_s diag_device = {
    .nrOfConfiguredDiagnosisEntries   = sizeof(diag_diagnosisIdConfiguration) / sizeof(DIAG_ID_CFG_s),
    .pConfigurationOfDiagnosisEntries = &diag_diagnosisIdConfiguration[0],
    .numberOfFatalErrors              = 0u,
};

BS_STRING_PRECHARGE_PRESENT_e bs_stringsWithPrecharge[BS_NR_OF_STRINGS] = {
    BS_STRING_WITH_PRECHARGE,
    BS_STRING_WITHOUT_PRECHARGE,
};

CONT_CONTACTOR_STATE_s cont_contactorStates[] = {
    /* String 0 contactors configuration */
    {CONT_SWITCH_OFF,
     CONT_SWITCH_OFF,
     CONT_FEEDBACK_NORMALLY_OPEN,
     BS_STRING0,
     CONT_PLUS,
     SPS_CHANNEL_0,
     CONT_CHARGING_DIRECTION},
    {CONT_SWITCH_OFF,
     CONT_SWITCH_OFF,
     CONT_FEEDBACK_NORMALLY_OPEN,
     BS_STRING0,
     CONT_MINUS,
     SPS_CHANNEL_1,
     CONT_DISCHARGING_DIRECTION},
    /* Precharge contactors configuration */
    {CONT_SWITCH_OFF,
     CONT_SWITCH_OFF,
     CONT_HAS_NO_FEEDBACK,
     BS_STRING0,
     CONT_PRECHARGE,
     SPS_CHANNEL_2,
     CONT_BIDIRECTIONAL},
    /* String 1 contactors configuration */
    {CONT_SWITCH_OFF,
     CONT_SWITCH_OFF,
     CONT_FEEDBACK_NORMALLY_OPEN,
     BS_STRING1,
     CONT_PLUS,
     SPS_CHANNEL_3,
     CONT_CHARGING_DIRECTION},
    {CONT_SWITCH_OFF,
     CONT_SWITCH_OFF,
     CONT_FEEDBACK_NORMALLY_OPEN,
     BS_STRING1,
     CONT_MINUS,
     SPS_CHANNEL_4,
     CONT_DISCHARGING_DIRECTION},
};

static BMS_STATE_s bms_state = {
    .closedStrings         = {0u, 0u},
    .numberOfClosedStrings = 0u,
    .deactivatedStrings    = {0, 0},
    .minimumActiveDelay_ms = 0u,
};

static DATA_BLOCK_MIN_MAX_s bms_tableMinMax         = {.header.uniqueId = DATA_BLOCK_ID_MIN_MAX};
static DATA_BLOCK_PACK_VALUES_s bms_tablePackValues = {.header.uniqueId = DATA_BLOCK_ID_PACK_VALUES};
static DATA_BLOCK_OPEN_WIRE_s bms_tableOpenWire     = {.header.uniqueId = DATA_BLOCK_ID_OPEN_WIRE_BASE};

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/* This function SHALL be called at the end of a test function if alterations
 * to any of the global variables has been made. */
void resetStaticVariablesToDefault(void) {
    diag_device.nrOfConfiguredDiagnosisEntries   = sizeof(diag_diagnosisIdConfiguration) / sizeof(DIAG_ID_CFG_s);
    diag_device.pConfigurationOfDiagnosisEntries = &diag_diagnosisIdConfiguration[0];
    diag_device.numberOfFatalErrors              = 0u;

    bs_stringsWithPrecharge[0] = BS_STRING_WITH_PRECHARGE;
    bs_stringsWithPrecharge[1] = BS_STRING_WITHOUT_PRECHARGE;

    /* String 0 - Main plus contactor configuration */
    cont_contactorStates[0].currentSet = CONT_SWITCH_OFF;
    cont_contactorStates[0].feedback   = CONT_SWITCH_OFF;

    /* String 0 - Main minus contactor configuration */
    cont_contactorStates[1].currentSet = CONT_SWITCH_OFF;
    cont_contactorStates[1].feedback   = CONT_SWITCH_OFF;

    /* String 0 - Precharge contactor configuration */
    cont_contactorStates[2].currentSet = CONT_SWITCH_OFF;
    cont_contactorStates[2].feedback   = CONT_SWITCH_OFF;

    /* String 1 - Main plus contactor configuration */
    cont_contactorStates[3].currentSet = CONT_SWITCH_OFF;
    cont_contactorStates[3].feedback   = CONT_SWITCH_OFF;

    /* String 1 - Main minus contactor configuration */
    cont_contactorStates[4].currentSet = CONT_SWITCH_OFF;
    cont_contactorStates[4].feedback   = CONT_SWITCH_OFF;

    /* All contactors opened - No errors */
    bms_state.closedStrings[0]      = 0u;
    bms_state.closedStrings[1]      = 0u;
    bms_state.numberOfClosedStrings = 0u;
    bms_state.deactivatedStrings[0] = 0u;
    bms_state.deactivatedStrings[1] = 0u;
}

/*========== Test Cases =====================================================*/
#define NUM_PRECHARGE_TESTS 13
STD_RETURN_TYPE_e prechargeExpectedResults[BS_NR_OF_STRINGS][NUM_PRECHARGE_TESTS] = {0};
/*
 * mock callback in order to provide custom values to current_tab
 */
STD_RETURN_TYPE_e MockDATA_ReadBlock_Callback(void *pDataToReceiver, int num_calls) {
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

    /* Cast to correct struct in order to properly write current and other values,
     * additionally, copy test values for all strings */
    for (uint8_t s = 0; s < BS_NR_OF_STRINGS; s++) {
        for (uint8_t testNumber = 0; testNumber < NUM_PRECHARGE_TESTS; testNumber++) {
            prechargeExpectedResults[s][testNumber] = prechargeExpectedResults[0][testNumber];
        }

        ((DATA_BLOCK_PACK_VALUES_s *)pDataToReceiver)->stringCurrent_mA[s] = current;
        ((DATA_BLOCK_PACK_VALUES_s *)pDataToReceiver)->stringVoltage_mV[s] = voltage_1;
    }
    ((DATA_BLOCK_PACK_VALUES_s *)pDataToReceiver)->highVoltageBusVoltage_mV = voltage_2;

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
    DATA_Read1DataBlock_Stub(MockDATA_ReadBlock_Callback);

    DATA_BLOCK_PACK_VALUES_s tablePackValues = {.header.uniqueId = DATA_BLOCK_ID_PACK_VALUES};

    /* iterate until we have all covered cases from our stub processed */
    for (uint8_t i = 0u; i < NUM_PRECHARGE_TESTS; i++) {
        char buffer[30];
        snprintf(buffer, 30, "Loop iteration %d.", i);
        for (uint8_t s = 0; s < BS_NR_OF_STRINGS; s++) {
            DIAG_Handler_ExpectAndReturn(
                DIAG_ID_PRECHARGE_ABORT_REASON_VOLTAGE, DIAG_EVENT_OK, DIAG_STRING, s, DIAG_HANDLER_RETURN_OK);
            DIAG_Handler_ExpectAndReturn(
                DIAG_ID_PRECHARGE_ABORT_REASON_CURRENT, DIAG_EVENT_OK, DIAG_STRING, s, DIAG_HANDLER_RETURN_OK);
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

#if (BS_POSITIVE_DISCHARGE_CURRENT == true)
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
#if (BS_POSITIVE_DISCHARGE_CURRENT == true)
    TEST_ASSERT_EQUAL(BMS_DISCHARGING, BMS_GetCurrentFlowDirection(INT32_MAX));
#else
    TEST_ASSERT_EQUAL(BMS_CHARGING, BMS_GetCurrentFlowDirection(INT32_MAX));
#endif

    /* Set the current to #INT32_MIN */
#if (BS_POSITIVE_DISCHARGE_CURRENT == true)
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

    /* Invalid string number */
    TEST_ASSERT_FAIL_ASSERT(TEST_BMS_CheckPrecharge(BS_NR_OF_STRINGS, &tablePackValues));

    /* Invalid string number */
    TEST_ASSERT_FAIL_ASSERT(TEST_BMS_CheckPrecharge(BS_NR_OF_STRINGS + 1u, &tablePackValues));

    /* Invalid string number */
    TEST_ASSERT_FAIL_ASSERT(TEST_BMS_CheckPrecharge(UINT8_MAX, &tablePackValues));

    /* Valid string number */
    DIAG_Handler_ExpectAndReturn(
        DIAG_ID_PRECHARGE_ABORT_REASON_VOLTAGE, DIAG_EVENT_OK, DIAG_STRING, 0u, DIAG_HANDLER_RETURN_OK);
    DIAG_Handler_ExpectAndReturn(
        DIAG_ID_PRECHARGE_ABORT_REASON_CURRENT, DIAG_EVENT_OK, DIAG_STRING, 0u, DIAG_HANDLER_RETURN_OK);
    TEST_ASSERT_PASS_ASSERT(TEST_BMS_CheckPrecharge(0u, &tablePackValues));
}

void testBMS_CheckCanRequest(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= Routine tests =============================================== */
    DATA_BLOCK_STATE_REQUEST_s request = {.header.uniqueId = DATA_BLOCK_ID_STATE_REQUEST};
    /* ======= RT1/1: Test implementation */
    DATA_Read1DataBlock_ExpectAndReturn(&request, STD_OK);
    TEST_BMS_CheckCanRequests();
    /* TODO: Not Working Currently */
    /* ======= RT1/2: Test implementation */
    request.stateRequestViaCan = BMS_REQ_ID_CHARGE;
    DATA_Read1DataBlock_ExpectAndReturn(&request, STD_NOT_OK);
    TEST_BMS_CheckCanRequests();
    /* ======= RT1/3: Test implementation */
    request.stateRequestViaCan = BMS_REQ_ID_NORMAL;
    DATA_Read1DataBlock_ExpectAndReturn(&request, STD_NOT_OK);
    TEST_BMS_CheckCanRequests();
    /* ======= RT1/4: Test implementation */
    request.stateRequestViaCan = BMS_REQ_ID_STANDBY;
    DATA_Read1DataBlock_ExpectAndReturn(&request, STD_NOT_OK);
    TEST_BMS_CheckCanRequests();
}

void testBMS_IsAnyFatalErrorFlagSet(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    for (uint16_t entry = 0u; entry < diag_device.numberOfFatalErrors; entry++) {
        DIAG_GetDiagnosisEntryState_ExpectAndReturn(diag_device.pFatalErrorLinkTable[entry]->id, STD_NOT_OK);
        bms_state.minimumActiveDelay_ms = 1u;
        DIAG_GetDelay_ExpectAndReturn(diag_device.pFatalErrorLinkTable[entry]->id, 0u);
    }
    TEST_BMS_IsAnyFatalErrorFlagSet();

    /* ======= RT2/2: Test implementation */
    for (uint16_t entry = 0u; entry < diag_device.numberOfFatalErrors; entry++) {
        DIAG_GetDiagnosisEntryState_ExpectAndReturn(diag_device.pFatalErrorLinkTable[entry]->id, STD_OK);
    }
    TEST_BMS_IsAnyFatalErrorFlagSet();
}

void testBMS_IsBatterySystemStateOkay(void) {
    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    OS_GetTickCount_ExpectAndReturn(1u);
    TEST_BMS_IsBatterySystemStateOkay();
}

void testBMS_IsContactorFeedbackValid(void) {
    DATA_BLOCK_ERROR_STATE_s tableErrorFlags = {.header.uniqueId = DATA_BLOCK_ID_ERROR_STATE};
    /* ======= Routine tests =============================================== */
    uint8_t stringNumber      = 0u;
    CONT_TYPE_e contactorType = {0};
    /* ======= RT1/1: Test implementation */
    DATA_Read1DataBlock_ExpectAndReturn(&tableErrorFlags, STD_OK);
    TEST_BMS_IsContactorFeedbackValid(stringNumber, contactorType);
}

void testBMS_GetClosestString(void) {
    DATA_BLOCK_PACK_VALUES_s tablePackValues = {.header.uniqueId = DATA_BLOCK_ID_PACK_VALUES};
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/2: Test implementation ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_BMS_GetClosestString(BMS_TAKE_PRECHARGE_INTO_ACCOUNT, NULL_PTR));
    /* ======= AT2/2: Test implementation ======= */
    TEST_ASSERT_PASS_ASSERT(TEST_BMS_GetClosestString(BMS_TAKE_PRECHARGE_INTO_ACCOUNT, &tablePackValues));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/3: Test implementation */
    /* All voltages valid
     * String voltage 0: 98V
     * String 0: Precharge available
     * String voltage 1: 103V
     * String 1: No precharge available
     * HV Bus voltage:   100V
     * All strings open -> take precharge into account
     * -> String 0 should be selected */
    tablePackValues.invalidStringVoltage[0]  = 0u;
    tablePackValues.stringVoltage_mV[0u]     = 98000;
    tablePackValues.invalidStringVoltage[1]  = 0u;
    tablePackValues.stringVoltage_mV[1u]     = 102000;
    tablePackValues.invalidHvBusVoltage      = 0u;
    tablePackValues.highVoltageBusVoltage_mV = 100000;
    bms_state.closedStrings[0]               = 0u;
    bms_state.closedStrings[1]               = 0u;
    TEST_ASSERT_EQUAL(0u, TEST_BMS_GetClosestString(BMS_TAKE_PRECHARGE_INTO_ACCOUNT, &tablePackValues));

    /* ======= RT2/3: Test implementation */
    /* All voltages valid
     * String voltage 0: 98V
     * String 0: Precharge available
     * String voltage 1: 101V
     * String 1: No precharge available
     * HV Bus voltage 100V
     * All strings open -> take precharge into account
     * -> String 0 should be selected */
    tablePackValues.invalidStringVoltage[0]  = 0u;
    tablePackValues.stringVoltage_mV[0u]     = 98000;
    tablePackValues.invalidStringVoltage[1]  = 0u;
    tablePackValues.stringVoltage_mV[1u]     = 101000;
    tablePackValues.invalidHvBusVoltage      = 0u;
    tablePackValues.highVoltageBusVoltage_mV = 100000;
    bms_state.closedStrings[0]               = 0u;
    bms_state.closedStrings[1]               = 0u;
    TEST_ASSERT_EQUAL(0u, TEST_BMS_GetClosestString(BMS_TAKE_PRECHARGE_INTO_ACCOUNT, &tablePackValues));

    /* ======= RT3/3: Test implementation */
    /* All voltages valid
     * String voltage 0: 98V
     * String 0: Precharge available
     * String voltage 1: 101V
     * String 1: No precharge available
     * HV Bus voltage 100V
     * String 0 closed -> do not take precharge into account
     * -> String 0 should be selected */
    tablePackValues.invalidStringVoltage[0]  = 0u;
    tablePackValues.stringVoltage_mV[0u]     = 98000;
    tablePackValues.invalidStringVoltage[1]  = 0u;
    tablePackValues.stringVoltage_mV[1u]     = 101000;
    tablePackValues.invalidHvBusVoltage      = 0u;
    tablePackValues.highVoltageBusVoltage_mV = 100000;
    bms_state.closedStrings[0]               = 1u;
    bms_state.numberOfClosedStrings          = 1u;
    bms_state.closedStrings[1]               = 0u;
    TEST_ASSERT_EQUAL(1u, TEST_BMS_GetClosestString(BMS_DO_NOT_TAKE_PRECHARGE_INTO_ACCOUNT, &tablePackValues));

    resetStaticVariablesToDefault();
}

/** check that the asynchronous BmsState message is sent when state or substate change */
void testBmsStateMessageIsRequested(void) {
    OS_GetTickCount_ExpectAndReturn(0u);
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    BMS_Trigger();

    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    /* change state request to init */
    BMS_SetStateRequest(BMS_STATE_INIT_REQUEST);
    /* State changes to Initialisation state -> message transmitted */
    OS_GetTickCount_ExpectAndReturn(0u);
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    CANTX_TransmitBmsState_ExpectAndReturn(STD_OK);
    BMS_Trigger();

    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    /* change state request to error */
    BMS_SetStateRequest(BMS_STATE_ERROR_REQUEST);
    OS_GetTickCount_ExpectAndReturn(0u);
    /* State changes to Error state -> message transmitted */
    DATA_Read3DataBlocks_ExpectAndReturn(&bms_tablePackValues, &bms_tableOpenWire, &bms_tableMinMax, STD_OK);
    SOA_CheckVoltages_Expect(&bms_tableMinMax);
    SOA_CheckTemperatures_Expect(&bms_tableMinMax, &bms_tablePackValues);
    SOA_CheckCurrent_Expect(&bms_tablePackValues);
    SOA_CheckSlaveTemperatures_Expect();
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        DIAG_Handler_ExpectAndReturn(DIAG_ID_AFE_OPEN_WIRE, DIAG_EVENT_OK, DIAG_STRING, s, STD_OK);
    }
    CONT_CheckFeedback_Expect();

    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    DIAG_Handler_ExpectAndReturn(DIAG_ID_ALERT_MODE, DIAG_EVENT_OK, 0, 0, STD_OK);

    CANTX_TransmitBmsState_ExpectAndReturn(STD_OK);
    BMS_Trigger();
}

/** check that the BMS_GetSubstate function works correctly */
void testBMS_GetSubstate(void) {
    TEST_ASSERT_EQUAL(0, BMS_GetSubstate());
}
