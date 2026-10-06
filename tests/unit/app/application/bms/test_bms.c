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
 * @file    test_bms.c
 * @author  foxBMS Team
 * @date    2020-04-01 (date of creation)
 * @updated 2026-10-06 (date of last update)
 * @version v1.12.0
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
#include "fstd_types.h"
#include "test_assert_helper.h"

#include <stdbool.h>

/*========== Unit Testing Framework Directives ==============================*/

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
    .information = {
        .closedStrings         = {0u, 0u},
        .numberOfClosedStrings = 0u,
        .deactivatedStrings    = {0, 0},
        .minimumActiveDelay_ms = 0u,
    }};

static DATA_BLOCK_MIN_MAX_s *bms_tableMinMax             = NULL_PTR;
static DATA_BLOCK_PACK_VALUES_s *bms_tablePackValues     = NULL_PTR;
static DATA_BLOCK_OPEN_WIRE_s *bms_tableOpenWire         = NULL_PTR;
static uint16_t bms_initialNrOpenWires[BS_NR_OF_STRINGS] = {0u};

/*========== Setup and Teardown =============================================*/
void setUp(void) {
    bms_tableMinMax     = TEST_BMS_GetMinMaxTable();
    bms_tablePackValues = TEST_BMS_GetPackValuesTable();
    bms_tableOpenWire   = TEST_BMS_GetOpenWireTable();

    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        bms_initialNrOpenWires[s] = bms_tableOpenWire->nrOpenWires[s];
    }
}

void tearDown(void) {
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        bms_tableOpenWire->nrOpenWires[s] = bms_initialNrOpenWires[s];
    }
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
    bms_state.information.closedStrings[0]      = 0u;
    bms_state.information.closedStrings[1]      = 0u;
    bms_state.information.numberOfClosedStrings = 0u;
    bms_state.information.deactivatedStrings[0] = 0u;
    bms_state.information.deactivatedStrings[1] = 0u;
}

/*========== Test Cases =====================================================*/
#define NUM_PRECHARGE_TESTS 13
BMS_RESULT_PRECHARGE_PROCESS_e prechargeExpectedResults[BS_NR_OF_STRINGS][NUM_PRECHARGE_TESTS] = {0};
/*
 * mock callback in order to provide custom values to current_tab
 */
STD_RETURN_TYPE_e SetPrechargingValues(void *pDataToReceiver, int num_calls) {
    int32_t current   = 0;
    int32_t voltage_1 = 0;
    int32_t voltage_2 = 0;

    /* determine a value depending on num_calls (has to be synchronized with test) */
    switch (num_calls) {
        case 0:
            prechargeExpectedResults[0][0] = BMS_PRECHARGING_FINISHED;
            /* no current, no voltage difference --> expect OK */
            current   = 0;
            voltage_1 = 0;
            voltage_2 = 0;
            break;
        case 1:
            prechargeExpectedResults[0][1] = BMS_PRECHARGING_ONGOING;
            /* INT32_MAX current, no voltage difference --> expect NOK */
            current   = INT32_MAX;
            voltage_1 = 0;
            voltage_2 = 0;
            break;
        case 2:
            prechargeExpectedResults[0][2] = BMS_PRECHARGING_ONGOING;
            /* INT32_MIN current, no voltage difference --> expect NOK */
            current   = INT32_MIN;
            voltage_1 = 0;
            voltage_2 = 0;
            break;
        case 3:
            prechargeExpectedResults[0][3] = BMS_PRECHARGING_FINISHED;
            /* no current, no voltage difference --> expect OK */
            current   = 0;
            voltage_1 = INT32_MAX;
            voltage_2 = INT32_MAX;
            break;
        case 4:
            prechargeExpectedResults[0][4] = BMS_PRECHARGING_FINISHED;
            /* no current, no voltage difference --> expect OK */
            current   = 0;
            voltage_1 = INT32_MIN;
            voltage_2 = INT32_MIN;
            break;
        case 5:
            prechargeExpectedResults[0][5] = BMS_PRECHARGING_ONGOING;
            /* no current, maximum voltage difference --> expect NOK */
            current   = 0;
            voltage_1 = INT32_MAX;
            voltage_2 = INT32_MIN;
            break;
        case 6:
            prechargeExpectedResults[0][6] = BMS_PRECHARGING_ONGOING;
            /* no current, maximum voltage difference --> expect NOK */
            current   = 0;
            voltage_1 = INT32_MIN;
            voltage_2 = INT32_MAX;
            break;
        case 7:
            prechargeExpectedResults[0][7] = BMS_PRECHARGING_ONGOING;
            /* current exactly at threshold, no voltage difference --> expect NOK */
            current   = BMS_PRECHARGE_CURRENT_THRESHOLD_mA;
            voltage_1 = 0;
            voltage_2 = 0;
            break;
        case 8:
            prechargeExpectedResults[0][8] = BMS_PRECHARGING_ONGOING;
            /* no current, voltage difference exactly at threshold --> expect NOK */
            current   = 0;
            voltage_1 = BMS_PRECHARGE_VOLTAGE_THRESHOLD_mV;
            voltage_2 = 0;
            break;
        case 9:
            prechargeExpectedResults[0][9] = BMS_PRECHARGING_ONGOING;
            /* no current, voltage difference exactly at threshold --> expect NOK */
            current   = 0;
            voltage_1 = 0;
            voltage_2 = BMS_PRECHARGE_VOLTAGE_THRESHOLD_mV;
            break;
        case 10:
            prechargeExpectedResults[0][10] = BMS_PRECHARGING_FINISHED;
            /* current exactly 1 below threshold, no voltage difference --> expect OK */
            current   = BMS_PRECHARGE_CURRENT_THRESHOLD_mA - 1;
            voltage_1 = 0;
            voltage_2 = 0;
            break;
        case 11:
            prechargeExpectedResults[0][11] = BMS_PRECHARGING_FINISHED;
            /* no current, voltage difference exactly 1 below threshold --> expect OK */
            current   = 0;
            voltage_1 = BMS_PRECHARGE_VOLTAGE_THRESHOLD_mV - 1;
            voltage_2 = 0;
            break;
        case 12:
            prechargeExpectedResults[0][12] = BMS_PRECHARGING_FINISHED;
            /* no current, voltage difference exactly 1 below threshold --> expect OK */
            current   = 0;
            voltage_1 = 0;
            voltage_2 = BMS_PRECHARGE_VOLTAGE_THRESHOLD_mV - 1;
            break;
        default:
            TEST_FAIL_MESSAGE("Check code of stub. Something does not fit.");
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

void testBMS_CheckOpenSenseWire(void) {
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        bms_tableOpenWire->nrOpenWires[s] = 0u;
    }

    bms_tableOpenWire->nrOpenWires[0] = 1u;

    /* Expect DIAG_EVENT_NOT_OK for string 0 */
    DIAG_Handler_ExpectAndReturn(DIAG_ID_AFE_OPEN_WIRE, DIAG_EVENT_NOT_OK, DIAG_STRING, 0, DIAG_HANDLER_RETURN_OK);

    /* For remaining strings expect DIAG_EVENT_OK because table is zero */
    for (uint8_t s = 1u; s < BS_NR_OF_STRINGS; s++) {
        DIAG_Handler_ExpectAndReturn(DIAG_ID_AFE_OPEN_WIRE, DIAG_EVENT_OK, DIAG_STRING, s, DIAG_HANDLER_RETURN_OK);
    }

    TEST_BMS_CheckOpenSenseWire();
}

/**
 * @brief   Iterate over a function that supplies various scenarios and check if they work as expected
 * @details Use the callback #SetPrechargingValues() in order to inject current tables
 *          and voltage tables.
 */
void testBMS_MonitorPrechargeProcess(void) {
    DATA_BLOCK_PACK_VALUES_s tablePackValues = {.header.uniqueId = DATA_BLOCK_ID_PACK_VALUES};

    /* iterate until we have all covered cases from our stub processed
     * TODO: Fix unit test again for all test cases */
    for (uint8_t i = 0u; i < NUM_PRECHARGE_TESTS; i++) {
        /* Set precharging values to be tested */
        SetPrechargingValues(&tablePackValues, i);
        char buffer[30];
        snprintf(buffer, 30, "Loop iteration %d.", i);
        for (uint8_t s = 0; s < BS_NR_OF_STRINGS; s++) {
            STD_RETURN_TYPE_e currentBelowLimit = STD_NOT_OK;
            STD_RETURN_TYPE_e voltageBelowLimit = STD_NOT_OK;
            if ((tablePackValues.invalidStringCurrent[s] == 0u) &&
                ((MATH_AbsInt32_t(tablePackValues.stringCurrent_mA[s]) < BMS_PRECHARGE_CURRENT_THRESHOLD_mA))) {
                currentBelowLimit = STD_OK;
            }
            if ((tablePackValues.invalidStringVoltage[s] == 0u) && (tablePackValues.invalidHvBusVoltage == 0u)) {
                if ((MATH_AbsInt64_t(
                        (int64_t)tablePackValues.stringVoltage_mV[s] -
                        (int64_t)tablePackValues.highVoltageBusVoltage_mV)) < BMS_PRECHARGE_VOLTAGE_THRESHOLD_mV) {
                    voltageBelowLimit = STD_OK;
                }
            }
            if ((currentBelowLimit == STD_OK) && (voltageBelowLimit == STD_OK)) {
                /* BMS_PRECHARGING_FINISHED */
                DIAG_Handler_ExpectAndReturn(
                    DIAG_ID_PRECHARGE_ABORT_REASON_VOLTAGE, DIAG_EVENT_OK, DIAG_STRING, s, DIAG_HANDLER_RETURN_OK);
                DIAG_Handler_ExpectAndReturn(
                    DIAG_ID_PRECHARGE_ABORT_REASON_CURRENT, DIAG_EVENT_OK, DIAG_STRING, s, DIAG_HANDLER_RETURN_OK);
            } else {
                if (bms_state.information.currentSystick - bms_state.information.startOfPrecharging >
                    BMS_MAXIMUM_PRECHARGE_DURATION_ms) {
                    /* BMS_PRECHARGING_FAILED */
                    DIAG_ReportResultToHandler_ExpectAndReturn(
                        STD_OK, DIAG_ID_PRECHARGE_ABORT_REASON_CURRENT, DIAG_STRING, s, STD_OK);
                    DIAG_ReportResultToHandler_ExpectAndReturn(
                        STD_OK, DIAG_ID_PRECHARGE_ABORT_REASON_VOLTAGE, DIAG_STRING, s, STD_OK);
                }
                /* BMS_PRECHARGING_ONGOING */
            }
            TEST_ASSERT_EQUAL_MESSAGE(
                prechargeExpectedResults[s][i],
                TEST_BMS_MonitorPrechargeProcess(
                    &bms_state,
                    s,
                    &tablePackValues,
                    BS_PRECHARGE_MONITOR_CURRENT_AND_VOLTAGE,
                    BMS_MAXIMUM_PRECHARGE_DURATION_ms),
                buffer);
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

    /* Set the current to INT32_MAX */
#if (BS_POSITIVE_DISCHARGE_CURRENT == true)
    TEST_ASSERT_EQUAL(BMS_DISCHARGING, BMS_GetCurrentFlowDirection(INT32_MAX));
#else
    TEST_ASSERT_EQUAL(BMS_CHARGING, BMS_GetCurrentFlowDirection(INT32_MAX));
#endif

    /* Set the current to INT32_MIN */
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
    TEST_ASSERT_FAIL_ASSERT(TEST_BMS_MonitorPrechargeProcess(
        &bms_state,
        BS_NR_OF_STRINGS,
        &tablePackValues,
        BS_PRECHARGE_MONITOR_CURRENT_AND_VOLTAGE,
        BS_PRECHARGE_MONITOR_CURRENT_AND_VOLTAGE));

    /* Invalid string number */
    TEST_ASSERT_FAIL_ASSERT(TEST_BMS_MonitorPrechargeProcess(
        &bms_state,
        BS_NR_OF_STRINGS + 1u,
        &tablePackValues,
        BS_PRECHARGE_MONITOR_CURRENT_AND_VOLTAGE,
        BS_PRECHARGE_MONITOR_CURRENT_AND_VOLTAGE));

    /* Invalid string number */
    TEST_ASSERT_FAIL_ASSERT(TEST_BMS_MonitorPrechargeProcess(
        &bms_state,
        UINT8_MAX,
        &tablePackValues,
        BS_PRECHARGE_MONITOR_CURRENT_AND_VOLTAGE,
        BS_PRECHARGE_MONITOR_CURRENT_AND_VOLTAGE));

    /* Valid string number */
    DIAG_Handler_ExpectAndReturn(
        DIAG_ID_PRECHARGE_ABORT_REASON_VOLTAGE, DIAG_EVENT_OK, DIAG_STRING, 0u, DIAG_HANDLER_RETURN_OK);
    DIAG_Handler_ExpectAndReturn(
        DIAG_ID_PRECHARGE_ABORT_REASON_CURRENT, DIAG_EVENT_OK, DIAG_STRING, 0u, DIAG_HANDLER_RETURN_OK);
    TEST_ASSERT_PASS_ASSERT(TEST_BMS_MonitorPrechargeProcess(
        &bms_state,
        0u,
        &tablePackValues,
        BS_PRECHARGE_MONITOR_CURRENT_AND_VOLTAGE,
        BS_PRECHARGE_MONITOR_CURRENT_AND_VOLTAGE));
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
        bms_state.information.minimumActiveDelay_ms = 1u;
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
    TEST_BMS_IsBatterySystemStateOkay(&bms_state);
}

void testBMS_IsContactorFeedbackValid(void) {
    DATA_BLOCK_ERROR_STATE_s tableErrorFlags = {.header.uniqueId = DATA_BLOCK_ID_ERROR_STATE};
    /* ======= Routine tests =============================================== */
    uint8_t stringNumber = 0u;
    /* ======= RT1/3: Test implementation */
    DATA_Read1DataBlock_ExpectAndReturn(&tableErrorFlags, STD_OK);
    TEST_BMS_IsContactorFeedbackValid(stringNumber, CONT_PLUS);
    /* ======= RT2/3: Test implementation */
    DATA_Read1DataBlock_ExpectAndReturn(&tableErrorFlags, STD_OK);
    TEST_BMS_IsContactorFeedbackValid(stringNumber, CONT_MINUS);
    /* ======= RT3/3: Test implementation */
    DATA_Read1DataBlock_ExpectAndReturn(&tableErrorFlags, STD_OK);
    TEST_BMS_IsContactorFeedbackValid(stringNumber, CONT_PRECHARGE);
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
    bms_state.information.closedStrings[0]   = 0u;
    bms_state.information.closedStrings[1]   = 0u;
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
    bms_state.information.closedStrings[0]   = 0u;
    bms_state.information.closedStrings[1]   = 0u;
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
    tablePackValues.invalidStringVoltage[0]     = 0u;
    tablePackValues.stringVoltage_mV[0u]        = 98000;
    tablePackValues.invalidStringVoltage[1]     = 0u;
    tablePackValues.stringVoltage_mV[1u]        = 101000;
    tablePackValues.invalidHvBusVoltage         = 0u;
    tablePackValues.highVoltageBusVoltage_mV    = 100000;
    bms_state.information.closedStrings[0]      = 1u;
    bms_state.information.numberOfClosedStrings = 1u;
    bms_state.information.closedStrings[1]      = 0u;
    TEST_ASSERT_EQUAL(1u, TEST_BMS_GetClosestString(BMS_DO_NOT_TAKE_PRECHARGE_INTO_ACCOUNT, &tablePackValues));

    resetStaticVariablesToDefault();
}

static uint32_t OsGetTickCountStub(int cmock_num_calls) {
    (void)cmock_num_calls;
    return 0u;
}

static void OsEnterTaskCriticalStub(int cmock_num_calls) {
    (void)cmock_num_calls;
}

static void OsExitTaskCriticalStub(int cmock_num_calls) {
    (void)cmock_num_calls;
}

/** check that the asynchronous BmsState message is sent when currentState or currentSubstate change */
void testBMS_Trigger(void) {
    /* ======= Assertion tests ============================================= */

    /* ======= Routine tests =============================================== */
    /* ======= RT1/3: First trigger from initial state -> message transmitted */
    OS_GetTickCount_Stub(OsGetTickCountStub);
    OS_EnterTaskCritical_Stub(OsEnterTaskCriticalStub);
    OS_ExitTaskCritical_Stub(OsExitTaskCriticalStub);

    /* change state request to error */
    BMS_SetStateRequest(BMS_STATE_ERROR_REQUEST);
    /* State changes to Error state -> message transmitted */
    DATA_Read3DataBlocks_ExpectAndReturn(bms_tablePackValues, bms_tableOpenWire, bms_tableMinMax, STD_OK);
    SOA_CheckVoltages_Expect(bms_tableMinMax);
    SOA_CheckTemperatures_Expect(bms_tableMinMax, bms_tablePackValues);
    SOA_CheckCurrent_Expect(bms_tablePackValues);
    SOA_CheckSlaveTemperatures_Expect();
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        DIAG_Handler_ExpectAndReturn(DIAG_ID_AFE_OPEN_WIRE, DIAG_EVENT_OK, DIAG_STRING, s, DIAG_HANDLER_RETURN_OK);
    }
    CONT_CheckFeedback_Expect();
    CANTX_TransmitBmsState_ExpectAndReturn(STD_OK);
    BMS_Trigger();

    /* ======= RT2/3: Request initialization -> message transmitted */
    /* change currentState request to init */
    BMS_SetStateRequest(BMS_STATE_INITIALIZATION_REQUEST);
    /* State changes to Initialisation currentState -> message transmitted */
    CANTX_TransmitBmsState_ExpectAndReturn(STD_OK);
    BMS_Trigger();
}

/** check that the BMS_GetSubstate function works correctly */
void testBMS_GetSubstate(void) {
    TEST_ASSERT_EQUAL(1, BMS_GetSubstate());
}

/**
 * @brief   Testing static function #BMS_RunStateMachine with currentState INITIALIZED
 * @details The following cases need to be tested:
 *          - Routine validation:
 *            - RT1/2: Call function in ENTRY - Wait
 *            - RT2/2: Call function in BMS_FSM_SUBSTATE_INITIALIZATION_EXIT - proceed to BMS_FSM_STATE_IDLE
 *
 *
 */
void testBMS_RunStateMachine_Initialized(void) {
    /* ======= Routine tests =============================================== */
    /* ======= RT1/2: Test implementation */
    BMS_STATE_s state = {
        .timer           = 0u,
        .currentState    = BMS_FSM_STATE_INITIALIZED,
        .currentSubstate = BMS_FSM_SUBSTATE_ENTRY,
    };
    TEST_BMS_RunStateMachine(&state);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SHORTTIME, state.timer);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_INITIALIZATION_IMD, state.currentSubstate);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_INITIALIZED, state.currentState);

    /* ======= RT2/2: Test implementation */
    state = (BMS_STATE_s){
        .timer           = 0u,
        .currentState    = BMS_FSM_STATE_INITIALIZED,
        .currentSubstate = BMS_FSM_SUBSTATE_INITIALIZATION_EXIT};
    TEST_BMS_RunStateMachine(&state);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SHORTTIME, state.timer);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_IDLE, state.currentState);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_ENTRY, state.currentSubstate);
}

/**
 * @brief   Testing static function #BMS_ProcessInitializedState
 * @details The following cases need to be tested:
 *          - Routine validation:
 *            - RT1/4: Call function in ENTRY - proceed to BMS_FSM_SUBSTATE_INITIALIZATION_IMD
 *            - RT2/4: Call function in BMS_FSM_SUBSTATE_INITIALIZATION_IMD with IMD_ILLEGAL_REQUEST - wait
 *            - RT3/4: Call function in BMS_FSM_SUBSTATE_INITIALIZATION_IMD - proceed to BMS_FSM_SUBSTATE_INITIALIZATION_EXIT
 *            - RT4/4: Call function in BMS_FSM_SUBSTATE_INITIALIZATION_EXIT - proceed to BMS_FSM_STATE_IDLE
 *
 *
 */
void testBMS_ProcessInitializedState(void) {
    /* ======= Routine tests =============================================== */
    /* ======= RT1/4: Test implementation */
    BMS_STATE_s state = {
        .timer           = 0u,
        .currentState    = BMS_FSM_STATE_INITIALIZED,
        .currentSubstate = BMS_FSM_SUBSTATE_ENTRY,
    };
    BMS_FSM_STATES_e result = TEST_BMS_ProcessInitializedState(&state);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SHORTTIME, state.timer);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_INITIALIZATION_IMD, state.currentSubstate);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_INITIALIZED, result);

    /* ======= RT2/4: Test implementation */
    state = (BMS_STATE_s){
        .timer           = 0u,
        .currentState    = BMS_FSM_STATE_INITIALIZED,
        .currentSubstate = BMS_FSM_SUBSTATE_INITIALIZATION_IMD,
    };
    IMD_RequestInsulationMeasurement_ExpectAndReturn(IMD_ILLEGAL_REQUEST);
    result = TEST_BMS_ProcessInitializedState(&state);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_LONGTIME, state.timer);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_INITIALIZATION_IMD, state.currentSubstate);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_INITIALIZED, result);

    /* ======= RT3/4: Test implementation */
    state = (BMS_STATE_s){
        .timer = 0u, .currentState = BMS_FSM_STATE_INITIALIZED, .currentSubstate = BMS_FSM_SUBSTATE_INITIALIZATION_IMD};
    IMD_RequestInsulationMeasurement_ExpectAndReturn(IMD_REQUEST_OK);
    result = TEST_BMS_ProcessInitializedState(&state);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SHORTTIME, state.timer);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_INITIALIZATION_EXIT, state.currentSubstate);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_INITIALIZED, result);

    /* ======= RT4/4: Test implementation */
    state = (BMS_STATE_s){
        .timer           = 0u,
        .currentState    = BMS_FSM_STATE_INITIALIZED,
        .currentSubstate = BMS_FSM_SUBSTATE_INITIALIZATION_EXIT};
    result = TEST_BMS_ProcessInitializedState(&state);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_IDLE, result);
}

/**
 * @brief   Testing static function #BMS_RunStateMachine with currentState IDLE
 * @details The following cases need to be tested:
 *          - Routine validation:
 *            - RT1/3: Call function in BMS_FSM_ENTRY - proceed to BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS
 *            - RT2/3: Call function in BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS and battery system currentState is not okay - proceed to BMS_FSM_STATE_OPEN_CONTACTORS
 *            - RT3/3: Call function in BMS_FSM_SUBSTATE_CHECK_STATE_REQUESTS and request is standby - proceed to BMS_FSM_STATE_STANDBY
 *
 *
 */
void testBMS_RunStateMachine_Idle(void) {
    /* ======= Routine tests =============================================== */
    /* ======= RT1/3: Test implementation */
    BMS_STATE_s state = {.timer = 0u, .currentState = BMS_FSM_STATE_IDLE, .currentSubstate = BMS_FSM_SUBSTATE_ENTRY};
    DATA_BLOCK_SYSTEM_STATE_s systemState = {.header.uniqueId = DATA_BLOCK_ID_SYSTEM_STATE};
    DATA_Read1DataBlock_ExpectAndReturn(&systemState, STD_OK);
    DATA_Write1DataBlock_ExpectAndReturn(&systemState, STD_OK);
    TEST_BMS_RunStateMachine(&state);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SHORTTIME, state.timer);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_IDLE, state.currentState);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS, state.currentSubstate);

    /* ======= RT2/3: Test implementation */
    state = (BMS_STATE_s){
        .timer                              = 0u,
        .currentState                       = BMS_FSM_STATE_IDLE,
        .currentSubstate                    = BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS,
        .information.transitionToErrorState = true};

    /*BMS_IsBatterySystemStateOkay*/
    OS_GetTickCount_ExpectAndReturn(1u);

    TEST_BMS_RunStateMachine(&state);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SHORTTIME, state.timer);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR, state.currentState);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_ENTRY, state.currentSubstate);

    /* ======= RT3/3: Test implementation */
    state = (BMS_STATE_s){
        .timer           = 0u,
        .currentState    = BMS_FSM_STATE_IDLE,
        .currentSubstate = BMS_FSM_SUBSTATE_CHECK_STATE_REQUESTS,
    };
    DATA_BLOCK_STATE_REQUEST_s request = {.header.uniqueId = DATA_BLOCK_ID_STATE_REQUEST};

    /*BMS_CheckCanRequest*/
    request.stateRequestViaCan = BMS_REQ_ID_STANDBY;
    DATA_Read1DataBlock_ExpectAndReturn(&request, STD_OK);
    DATA_Read1DataBlock_ReturnThruPtr_pDataToReceiver0(&request);

    TEST_BMS_RunStateMachine(&state);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SHORTTIME, state.timer);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_OPEN_CONTACTORS_TO_STANDBY, state.currentState);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_ENTRY, state.currentSubstate);
}

/**
 * @brief   Testing static function #BMS_ProcessIdleState
 * @details The following cases need to be tested:
 *          - Routine validation:
 *            - RT1/5: Call function in BMS_FSM_ENTRY - proceed to BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS
 *            - RT2/5: Call function in BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS and battery system currentState is okay - proceed to BMS_FSM_SUBSTATE_CHECK_STATE_REQUESTS
 *            - RT3/5: Call function in BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS and battery system currentState is not okay - proceed to BMS_FSM_STATE_OPEN_CONTACTORS
 *            - RT4/5: Call function in BMS_FSM_SUBSTATE_CHECK_STATE_REQUESTS and request is standby - proceed to BMS_FSM_STATE_STANDBY
 *            - RT5/5: Call function in BMS_FSM_SUBSTATE_CHECK_STATE_REQUESTS and request is not standby - proceed to BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS
 *
 *
 */
void testBMS_ProcessIdleState(void) {
    /* ======= Routine tests =============================================== */
    /* ======= RT1/5: Test implementation */
    BMS_STATE_s state = {.timer = 0u, .currentState = BMS_FSM_STATE_IDLE, .currentSubstate = BMS_FSM_SUBSTATE_ENTRY};
    DATA_BLOCK_SYSTEM_STATE_s systemState = {.header.uniqueId = DATA_BLOCK_ID_SYSTEM_STATE};
    DATA_Read1DataBlock_ExpectAndReturn(&systemState, STD_OK);
    DATA_Write1DataBlock_ExpectAndReturn(&systemState, STD_OK);
    BMS_FSM_STATES_e result = TEST_BMS_ProcessIdleState(&state);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SHORTTIME, state.timer);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_IDLE, result);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS, state.currentSubstate);
    /* ======= RT2/5: Test implementation */
    state = (BMS_STATE_s){
        .timer = 0u, .currentState = BMS_FSM_STATE_IDLE, .currentSubstate = BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS};

    /*BMS_IsBatterySystemStateOkay*/
    OS_GetTickCount_ExpectAndReturn(1u);

    result = TEST_BMS_ProcessIdleState(&state);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SHORTTIME, state.timer);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_IDLE, result);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_CHECK_STATE_REQUESTS, state.currentSubstate);

    /* ======= RT3/5: Test implementation */
    state = (BMS_STATE_s){
        .timer                              = 0u,
        .currentState                       = BMS_FSM_STATE_IDLE,
        .currentSubstate                    = BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS,
        .information.transitionToErrorState = true};

    /*BMS_IsBatterySystemStateOkay*/
    OS_GetTickCount_ExpectAndReturn(1u);

    result = TEST_BMS_ProcessIdleState(&state);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR, result);

    /* ======= RT4/5: Test implementation */
    state = (BMS_STATE_s){
        .timer           = 0u,
        .currentState    = BMS_FSM_STATE_IDLE,
        .currentSubstate = BMS_FSM_SUBSTATE_CHECK_STATE_REQUESTS,
    };
    DATA_BLOCK_STATE_REQUEST_s request = {.header.uniqueId = DATA_BLOCK_ID_STATE_REQUEST};

    /*BMS_CheckCanRequest*/
    request.stateRequestViaCan = BMS_REQ_ID_STANDBY;
    DATA_Read1DataBlock_ExpectAndReturn(&request, STD_OK);
    DATA_Read1DataBlock_ReturnThruPtr_pDataToReceiver0(&request);

    result = TEST_BMS_ProcessIdleState(&state);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_OPEN_CONTACTORS_TO_STANDBY, result);

    /* ======= RT5/5: Test implementation */
    state = (BMS_STATE_s){
        .timer           = 0u,
        .currentState    = BMS_FSM_STATE_IDLE,
        .currentSubstate = BMS_FSM_SUBSTATE_CHECK_STATE_REQUESTS,
    };

    /*BMS_CheckCanRequest*/
    request.stateRequestViaCan = BMS_REQ_ID_NOREQ;
    DATA_Read1DataBlock_ExpectAndReturn(&request, STD_OK);
    DATA_Read1DataBlock_ReturnThruPtr_pDataToReceiver0(&request);

    result = TEST_BMS_ProcessIdleState(&state);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SHORTTIME, state.timer);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_IDLE, result);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS, state.currentSubstate);
}

static STD_RETURN_TYPE_e OpenContactorStub(uint8_t stringNumber, CONT_TYPE_e contactor, int cmock_num) {
    (void)stringNumber;
    (void)contactor;
    (void)cmock_num;

    return STD_OK;
}

/**
 * @brief   Testing static function #BMS_RunStateMachine with currentState OPEN_CONTACTORS_TO_ERROR
 * @details The following cases need to be tested:
 *          - Routine validation:
 *            - RT1/2: Call function in BMS_FSM_ENTRY - proceed to BMS_FSM_SUBSTATE_HANDLE_SUPPLY_VOLTAGE_30C_LOSS
 *            - RT2/2: Call function in BMS_FSM_SUBSTATE_OPEN_STRINGS_EXIT - proceed to BMS_FSM_STATE_ERROR
 *
 *
 */
void testBMS_RunStateMachine_OpenContactorsToError(void) {
    /* ======= Routine tests =============================================== */
    /* ======= RT1/2: Test implementation */
    BMS_STATE_s state = {
        .timer = 0u, .currentState = BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR, .currentSubstate = BMS_FSM_SUBSTATE_ENTRY};
    BAL_SetStateRequest_ExpectAndReturn(BAL_STATE_NO_BALANCING_REQUEST, BAL_OK);
    DIAG_GetDiagnosisEntryState_ExpectAndReturn(DIAG_ID_SUPPLY_VOLTAGE_CLAMP_30C_LOST, STD_NOT_OK);
    TEST_BMS_RunStateMachine(&state);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SHORTTIME, state.timer);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR, state.currentState);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_HANDLE_SUPPLY_VOLTAGE_30C_LOSS, state.currentSubstate);

    /* ======= RT2/2: Test implementation */
    state = (BMS_STATE_s){
        .timer           = 0u,
        .currentState    = BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR,
        .currentSubstate = BMS_FSM_SUBSTATE_OPEN_STRINGS_EXIT,
    };
    TEST_BMS_RunStateMachine(&state);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SHORTTIME, state.timer);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_ERROR, state.currentState);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_ENTRY, state.currentSubstate);
}

/**
 * @brief   Testing static function #BMS_ProcessOpenContactorsToErrorState
 * @details The following cases need to be tested:
 *          - Routine validation:
 *            - RT01/16: Call function in BMS_FSM_ENTRY and supply voltage loss - proceed to BMS_FSM_SUBSTATE_HANDLE_SUPPLY_VOLTAGE_30C_LOSS
 *            - RT02/16: Call function in BMS_FSM_ENTRY and no supply voltage loss - proceed to BMS_FSM_SUBSTATE_OPEN_ALL_PRECHARGE_CONTACTORS
 *            - RT03/16: Call function in BMS_FSM_SUBSTATE_OPEN_ALL_PRECHARGE_CONTACTORS - proceed to BMS_FSM_SUBSTATE_OPEN_FIRST_STRING_CONTACTOR
 *            - RT04/16: Call function in BMS_FSM_SUBSTATE_OPEN_FIRST_STRING_CONTACTOR and Current is valid - proceed to BMS_FSM_SUBSTATE_OPEN_SECOND_STRING_CONTACTOR
 *            - RT05/16: Call function in BMS_FSM_SUBSTATE_OPEN_FIRST_STRING_CONTACTOR and Current is invalid, fuse not triggered - proceed to BMS_FSM_SUBSTATE_OPEN_SECOND_STRING_CONTACTOR
 *            - RT06/16: Call function in BMS_FSM_SUBSTATE_OPEN_SECOND_STRING_CONTACTOR and contactor currentState is OFF - proceed to BMS_FSM_SUBSTATE_CHECK_SECOND_STRING_CONTACTOR
 *            - RT07/16: Call function in BMS_FSM_SUBSTATE_OPEN_SECOND_STRING_CONTACTOR and contactor currentState is ON - proceed to BMS_FSM_SUBSTATE_OPEN_SECOND_STRING_CONTACTOR
 *            - RT08/16: Call function in BMS_FSM_SUBSTATE_CHECK_SECOND_STRING_CONTACTOR and contactor currentState is OFF, string number > 0 - proceed to BMS_FSM_SUBSTATE_OPEN_FIRST_STRING_CONTACTOR
 *            - RT09/16: Call function in BMS_FSM_SUBSTATE_CHECK_SECOND_STRING_CONTACTOR and contactor currentState is OFF, sting number = 0 - proceed to BMS_FSM_SUBSTATE_OPEN_STRINGS_EXIT
 *            - RT10/16: Call function in BMS_FSM_SUBSTATE_CHECK_SECOND_STRING_CONTACTOR and contactor currentState is ON, and no timeout - proceed to BMS_FSM_SUBSTATE_OPEN_FIRST_STRING_CONTACTOR
 *            - RT11/16: Call function in BMS_FSM_SUBSTATE_CHECK_SECOND_STRING_CONTACTOR and contactor currentState is ON, and timeout - proceed to BMS_FSM_SUBSTATE_CHECK_SECOND_STRING_CONTACTOR
 *            - RT12/16: Call function in BMS_FSM_SUBSTATE_HANDLE_SUPPLY_VOLTAGE_30C_LOSS - proceed to BMS_FSM_SUBSTATE_OPEN_STRINGS_EXIT
 *            - RT13/16: Call function in BMS_FSM_SUBSTATE_OPEN_STRINGS_EXIT and next currentState is error - proceed to BMS_FSM_STATE_ERROR
 *            - RT14/16: Call function in BMS_FSM_SUBSTATE_OPEN_FIRST_STRING_CONTACTOR and current is above threshold but fuse delay not elapsed
 *            - RT15/16: Call function in BMS_FSM_SUBSTATE_OPEN_SECOND_STRING_CONTACTOR and contactor feedback invalid - proceed to BMS_FSM_SUBSTATE_CHECK_SECOND_STRING_CONTACTOR
 *            - RT16/16: Call function in BMS_FSM_SUBSTATE_CHECK_SECOND_STRING_CONTACTOR and contactor feedback invalid - proceed to BMS_FSM_SUBSTATE_OPEN_FIRST_STRING_CONTACTOR
 *
 *
 */
void testBMS_ProcessOpenContactorsToErrorState(void) {
    /* ======= Routine tests =============================================== */
    /* ======= RT1/16: Test implementation */
    BMS_STATE_s state = {
        .timer = 0u, .currentState = BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR, .currentSubstate = BMS_FSM_SUBSTATE_ENTRY};
    BAL_SetStateRequest_ExpectAndReturn(BAL_STATE_NO_BALANCING_REQUEST, BAL_OK);
    DIAG_GetDiagnosisEntryState_ExpectAndReturn(DIAG_ID_SUPPLY_VOLTAGE_CLAMP_30C_LOST, STD_NOT_OK);
    BMS_FSM_STATES_e result = TEST_BMS_ProcessOpenContactorsToErrorState(&state);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SHORTTIME, state.timer);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR, result);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_HANDLE_SUPPLY_VOLTAGE_30C_LOSS, state.currentSubstate);

    /* ======= RT2/16: Test implementation */
    state = (BMS_STATE_s){
        .timer = 0u, .currentState = BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR, .currentSubstate = BMS_FSM_SUBSTATE_ENTRY};
    BAL_SetStateRequest_ExpectAndReturn(BAL_STATE_NO_BALANCING_REQUEST, BAL_OK);
    DIAG_GetDiagnosisEntryState_ExpectAndReturn(DIAG_ID_SUPPLY_VOLTAGE_CLAMP_30C_LOST, STD_OK);
    result = TEST_BMS_ProcessOpenContactorsToErrorState(&state);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SHORTTIME, state.timer);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR, result);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_OPEN_ALL_PRECHARGE_CONTACTORS, state.currentSubstate);

    /* ======= RT3/16: Test implementation */
    state = (BMS_STATE_s){
        .timer           = 0u,
        .currentState    = BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR,
        .currentSubstate = BMS_FSM_SUBSTATE_OPEN_ALL_PRECHARGE_CONTACTORS};
    CONT_OpenAllPrechargeContactors_Expect();
    result = TEST_BMS_ProcessOpenContactorsToErrorState(&state);
    TEST_ASSERT_EQUAL_INT(BMS_TIME_WAIT_AFTER_OPENING_PRECHARGE, state.timer);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR, result);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_OPEN_FIRST_STRING_CONTACTOR, state.currentSubstate);

    /* ======= RT4/16: Test implementation */
    state = (BMS_STATE_s){
        .timer                    = 0u,
        .currentState             = BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR,
        .currentSubstate          = BMS_FSM_SUBSTATE_OPEN_FIRST_STRING_CONTACTOR,
        .information.stringNumber = 0};
    bms_tablePackValues->invalidStringCurrent[0] = 0u;
    bms_tablePackValues->stringCurrent_mA[0]     = 0u;
    TEST_SetTablePackValues(bms_tablePackValues);
    CONT_OpenContactor_Stub(OpenContactorStub);
    result = TEST_BMS_ProcessOpenContactorsToErrorState(&state);
    TEST_ASSERT_EQUAL_INT(BMS_WAIT_TIME_AFTER_OPENING_STRING_CONTACTOR, state.timer);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR, result);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_OPEN_SECOND_STRING_CONTACTOR, state.currentSubstate);

    /* ======= RT5/16: Test implementation */
    state = (BMS_STATE_s){
        .timer                                         = 0u,
        .currentState                                  = BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR,
        .currentSubstate                               = BMS_FSM_SUBSTATE_OPEN_FIRST_STRING_CONTACTOR,
        .information.stringNumber                      = 0,
        .information.timeAboveContactorBreakCurrent_ms = BS_MAIN_FUSE_MAXIMUM_TRIGGER_DURATION_ms,
    };
    bms_tablePackValues->invalidStringCurrent[0] = 1u;
    TEST_SetTablePackValues(bms_tablePackValues);
    DIAG_Handler_ExpectAndReturn(DIAG_ID_ALERT_MODE, DIAG_EVENT_NOT_OK, DIAG_SYSTEM, 0u, DIAG_HANDLER_RETURN_OK);
    CONT_OpenContactor_Stub(OpenContactorStub);
    result = TEST_BMS_ProcessOpenContactorsToErrorState(&state);
    TEST_ASSERT_EQUAL_INT(BMS_WAIT_TIME_AFTER_OPENING_STRING_CONTACTOR, state.timer);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR, result);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_OPEN_SECOND_STRING_CONTACTOR, state.currentSubstate);

    /* ======= RT6/16: Test implementation */
    state = (BMS_STATE_s){
        .timer                           = 0u,
        .currentState                    = BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR,
        .currentSubstate                 = BMS_FSM_SUBSTATE_OPEN_SECOND_STRING_CONTACTOR,
        .information.contactorToBeOpened = 0u,
        .information.stringToBeOpened    = 0u,
    };
    CONT_GetContactorState_ExpectAndReturn(0u, 0u, CONT_SWITCH_OFF);

    /*BMS_IsContactorFeedbackValid*/
    DATA_BLOCK_STATE_REQUEST_s request = {.header.uniqueId = DATA_BLOCK_ID_ERROR_STATE};
    DATA_Read1DataBlock_ExpectAndReturn(&request, STD_OK);

    CONT_OpenContactor_Stub(OpenContactorStub);
    result = TEST_BMS_ProcessOpenContactorsToErrorState(&state);
    TEST_ASSERT_EQUAL_INT(BMS_WAIT_TIME_AFTER_OPENING_STRING_CONTACTOR, state.timer);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR, result);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_CHECK_SECOND_STRING_CONTACTOR, state.currentSubstate);

    /* ======= RT7/16: Test implementation */
    state = (BMS_STATE_s){
        .timer                           = 0u,
        .currentState                    = BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR,
        .currentSubstate                 = BMS_FSM_SUBSTATE_OPEN_SECOND_STRING_CONTACTOR,
        .information.contactorToBeOpened = 0u,
        .information.stringToBeOpened    = 0u,
    };
    CONT_GetContactorState_ExpectAndReturn(0u, 0u, CONT_SWITCH_ON);

    /*BMS_IsContactorFeedbackValid*/
    DATA_Read1DataBlock_ExpectAndReturn(&request, STD_OK);

    CONT_OpenContactor_Stub(OpenContactorStub);
    result = TEST_BMS_ProcessOpenContactorsToErrorState(&state);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SHORTTIME, state.timer);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR, result);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_OPEN_SECOND_STRING_CONTACTOR, state.currentSubstate);

    /* ======= RT8/16: Test implementation */
    state = (BMS_STATE_s){
        .timer                    = 0u,
        .currentState             = BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR,
        .currentSubstate          = BMS_FSM_SUBSTATE_CHECK_SECOND_STRING_CONTACTOR,
        .information.stringNumber = 1u,
    };
    CONT_GetContactorState_ExpectAndReturn(0u, 0u, CONT_SWITCH_OFF);

    /*BMS_IsContactorFeedbackValid*/
    DATA_Read1DataBlock_ExpectAndReturn(&request, STD_OK);

    result = TEST_BMS_ProcessOpenContactorsToErrorState(&state);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SHORTTIME, state.timer);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR, result);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_OPEN_FIRST_STRING_CONTACTOR, state.currentSubstate);

    /* ======= RT9/16: Test implementation */
    state = (BMS_STATE_s){
        .timer                    = 0u,
        .currentState             = BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR,
        .currentSubstate          = BMS_FSM_SUBSTATE_CHECK_SECOND_STRING_CONTACTOR,
        .information.stringNumber = 0u,
    };
    CONT_GetContactorState_ExpectAndReturn(0u, 0u, CONT_SWITCH_OFF);

    /*BMS_IsContactorFeedbackValid*/
    DATA_Read1DataBlock_ExpectAndReturn(&request, STD_OK);

    result = TEST_BMS_ProcessOpenContactorsToErrorState(&state);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SHORTTIME, state.timer);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR, result);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_OPEN_STRINGS_EXIT, state.currentSubstate);

    /* ======= RT10/16: Test implementation */
    state = (BMS_STATE_s){
        .timer                         = 0u,
        .currentState                  = BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR,
        .currentSubstate               = BMS_FSM_SUBSTATE_CHECK_SECOND_STRING_CONTACTOR,
        .information.stringOpenTimeout = 0u,
    };
    CONT_GetContactorState_ExpectAndReturn(0u, 0u, CONT_SWITCH_ON);

    /*BMS_IsContactorFeedbackValid*/
    DATA_Read1DataBlock_ExpectAndReturn(&request, STD_OK);

    result = TEST_BMS_ProcessOpenContactorsToErrorState(&state);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SHORTTIME, state.timer);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR, result);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_OPEN_FIRST_STRING_CONTACTOR, state.currentSubstate);

    /* ======= RT11/16: Test implementation */
    state = (BMS_STATE_s){
        .timer                         = 0u,
        .currentState                  = BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR,
        .currentSubstate               = BMS_FSM_SUBSTATE_CHECK_SECOND_STRING_CONTACTOR,
        .information.stringOpenTimeout = 1u,
    };
    CONT_GetContactorState_ExpectAndReturn(0u, 0u, CONT_SWITCH_ON);

    /*BMS_IsContactorFeedbackValid*/
    DATA_Read1DataBlock_ExpectAndReturn(&request, STD_OK);

    CONT_OpenContactor_Stub(OpenContactorStub);
    result = TEST_BMS_ProcessOpenContactorsToErrorState(&state);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SHORTTIME, state.timer);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR, result);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_CHECK_SECOND_STRING_CONTACTOR, state.currentSubstate);

    /* ======= RT12/16: Test implementation */
    state = (BMS_STATE_s){
        .timer           = 0u,
        .currentState    = BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR,
        .currentSubstate = BMS_FSM_SUBSTATE_HANDLE_SUPPLY_VOLTAGE_30C_LOSS,
    };
    CONT_OpenAllContactors_Expect();
    SPS_SwitchOffAllGeneralIoChannels_Expect();
    result = TEST_BMS_ProcessOpenContactorsToErrorState(&state);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SHORTTIME, state.timer);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR, result);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_OPEN_STRINGS_EXIT, state.currentSubstate);

    /* ======= RT13/16: Test implementation */
    state = (BMS_STATE_s){
        .timer           = 0u,
        .currentState    = BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR,
        .currentSubstate = BMS_FSM_SUBSTATE_OPEN_STRINGS_EXIT,
    };
    result = TEST_BMS_ProcessOpenContactorsToErrorState(&state);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_ERROR, result);

    /* ======= RT14/16: Test implementation */
    state = (BMS_STATE_s){
        .timer                                         = 0u,
        .currentState                                  = BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR,
        .currentSubstate                               = BMS_FSM_SUBSTATE_OPEN_FIRST_STRING_CONTACTOR,
        .information.stringNumber                      = 0u,
        .information.timeAboveContactorBreakCurrent_ms = 0u,
    };
    bms_tablePackValues->invalidStringCurrent[0] = 1u;
    TEST_SetTablePackValues(bms_tablePackValues);
    result = TEST_BMS_ProcessOpenContactorsToErrorState(&state);
    TEST_ASSERT_EQUAL_INT(BMS_STATEMACHINE_TASK_CYCLE_CONTEXT_MS, state.information.timeAboveContactorBreakCurrent_ms);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR, result);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_OPEN_FIRST_STRING_CONTACTOR, state.currentSubstate);

    /* ======= RT15/16: Test implementation */
    state = (BMS_STATE_s){
        .timer                           = 0u,
        .currentState                    = BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR,
        .currentSubstate                 = BMS_FSM_SUBSTATE_OPEN_SECOND_STRING_CONTACTOR,
        .information.stringNumber        = 1u,
        .information.contactorToBeOpened = CONT_PLUS,
        .information.stringToBeOpened    = 1u,
    };
    CONT_GetContactorState_ExpectAndReturn(1u, CONT_PLUS, CONT_SWITCH_ON);
    DATA_BLOCK_ERROR_STATE_s tableErrorFlags                         = {.header.uniqueId = DATA_BLOCK_ID_ERROR_STATE};
    tableErrorFlags.contactorInPositivePathOfStringFeedbackError[1u] = true;
    DATA_Read1DataBlock_ExpectAndReturn(&tableErrorFlags, STD_OK);
    DATA_Read1DataBlock_ReturnThruPtr_pDataToReceiver0(&tableErrorFlags);
    CONT_OpenContactor_Stub(OpenContactorStub);
    result = TEST_BMS_ProcessOpenContactorsToErrorState(&state);
    TEST_ASSERT_EQUAL_INT(BMS_WAIT_TIME_AFTER_OPENING_STRING_CONTACTOR, state.timer);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR, result);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_CHECK_SECOND_STRING_CONTACTOR, state.currentSubstate);

    /* ======= RT16/16: Test implementation */
    state = (BMS_STATE_s){
        .timer                           = 0u,
        .currentState                    = BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR,
        .currentSubstate                 = BMS_FSM_SUBSTATE_CHECK_SECOND_STRING_CONTACTOR,
        .information.stringNumber        = 1u,
        .information.contactorToBeOpened = CONT_PLUS,
        .information.stringToBeOpened    = 1u,
    };
    CONT_GetContactorState_ExpectAndReturn(1u, CONT_PLUS, CONT_SWITCH_ON);
    tableErrorFlags.contactorInPositivePathOfStringFeedbackError[1u] = true;
    DATA_Read1DataBlock_ExpectAndReturn(&tableErrorFlags, STD_OK);
    DATA_Read1DataBlock_ReturnThruPtr_pDataToReceiver0(&tableErrorFlags);
    result = TEST_BMS_ProcessOpenContactorsToErrorState(&state);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SHORTTIME, state.timer);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR, result);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_OPEN_FIRST_STRING_CONTACTOR, state.currentSubstate);
}

/**
 * @brief   Testing static function #BMS_RunStateMachine with currentState OPEN_CONTACTORS_TO_STANDBY
 * @details The following cases need to be tested:
 *          - Routine validation:
 *            - RT1/2: Call function in BMS_FSM_ENTRY - proceed to BMS_FSM_SUBSTATE_OPEN_ALL_PRECHARGE_CONTACTORS
 *            - RT2/2: Call function in BMS_FSM_SUBSTATE_OPEN_STRINGS_EXIT - proceed to BMS_FSM_STATE_STANDBY
 *
 *
 */
void testBMS_RunStateMachine_OpenContactorsToStandby(void) {
    /* ======= Routine tests =============================================== */
    /* ======= RT1/2: Test implementation */
    BMS_STATE_s state = (BMS_STATE_s){
        .timer           = 0u,
        .currentState    = BMS_FSM_STATE_OPEN_CONTACTORS_TO_STANDBY,
        .currentSubstate = BMS_FSM_SUBSTATE_ENTRY};
    BAL_SetStateRequest_ExpectAndReturn(BAL_STATE_NO_BALANCING_REQUEST, BAL_OK);
    TEST_BMS_RunStateMachine(&state);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SHORTTIME, state.timer);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_OPEN_CONTACTORS_TO_STANDBY, state.currentState);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_OPEN_ALL_PRECHARGE_CONTACTORS, state.currentSubstate);

    /* ======= RT2/2: Test implementation */
    state = (BMS_STATE_s){
        .timer           = 0u,
        .currentState    = BMS_FSM_STATE_OPEN_CONTACTORS_TO_STANDBY,
        .currentSubstate = BMS_FSM_SUBSTATE_OPEN_STRINGS_EXIT,
    };
    TEST_BMS_RunStateMachine(&state);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SHORTTIME, state.timer);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_STANDBY, state.currentState);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_ENTRY, state.currentSubstate);
}

/**
 * @brief   Testing static function #BMS_ProcessOpenContactorsToStandbyState
 * @details The following cases need to be tested:
 *          - Routine validation:
 *            - RT01/15: Call function in BMS_FSM_ENTRY and no supply voltage loss - proceed to BMS_FSM_SUBSTATE_OPEN_ALL_PRECHARGE_CONTACTORS
 *            - RT02/15: Call function in BMS_FSM_SUBSTATE_OPEN_ALL_PRECHARGE_CONTACTORS - proceed to BMS_FSM_SUBSTATE_OPEN_FIRST_STRING_CONTACTOR
 *            - RT03/15: Call function in BMS_FSM_SUBSTATE_OPEN_FIRST_STRING_CONTACTOR and Current is valid - proceed to BMS_FSM_SUBSTATE_OPEN_SECOND_STRING_CONTACTOR
 *            - RT04/15: Call function in BMS_FSM_SUBSTATE_OPEN_FIRST_STRING_CONTACTOR and Current is invalid, fuse not triggered - proceed to BMS_FSM_SUBSTATE_OPEN_SECOND_STRING_CONTACTOR
 *            - RT05/15: Call function in BMS_FSM_SUBSTATE_OPEN_SECOND_STRING_CONTACTOR and contactor currentState is OFF - proceed to BMS_FSM_SUBSTATE_CHECK_SECOND_STRING_CONTACTOR
 *            - RT06/15: Call function in BMS_FSM_SUBSTATE_OPEN_SECOND_STRING_CONTACTOR and contactor currentState is ON - proceed to BMS_FSM_SUBSTATE_OPEN_SECOND_STRING_CONTACTOR
 *            - RT07/15: Call function in BMS_FSM_SUBSTATE_CHECK_SECOND_STRING_CONTACTOR and contactor currentState is OFF, string number > 0 - proceed to BMS_FSM_SUBSTATE_OPEN_FIRST_STRING_CONTACTOR
 *            - RT08/15: Call function in BMS_FSM_SUBSTATE_CHECK_SECOND_STRING_CONTACTOR and contactor currentState is OFF, sting number = 0 - proceed to BMS_FSM_SUBSTATE_OPEN_STRINGS_EXIT
 *            - RT09/15: Call function in BMS_FSM_SUBSTATE_CHECK_SECOND_STRING_CONTACTOR and contactor currentState is ON, and no timeout - proceed to BMS_FSM_SUBSTATE_OPEN_FIRST_STRING_CONTACTOR
 *            - RT10/15: Call function in BMS_FSM_SUBSTATE_CHECK_SECOND_STRING_CONTACTOR and contactor currentState is ON, and timeout - proceed to BMS_FSM_SUBSTATE_CHECK_SECOND_STRING_CONTACTOR
 *            - RT11/15: Call function in BMS_FSM_SUBSTATE_HANDLE_SUPPLY_VOLTAGE_30C_LOSS - proceed to BMS_FSM_SUBSTATE_OPEN_STRINGS_EXIT
 *            - RT12/15: Call function in BMS_FSM_SUBSTATE_OPEN_STRINGS_EXIT and next currentState is standby - proceed to BMS_FSM_STATE_STANDBY
 *            - RT13/15: Call function in BMS_FSM_SUBSTATE_OPEN_FIRST_STRING_CONTACTOR and current is above threshold but fuse delay not elapsed
 *            - RT14/15: Call function in BMS_FSM_SUBSTATE_OPEN_SECOND_STRING_CONTACTOR and contactor feedback invalid - proceed to BMS_FSM_SUBSTATE_CHECK_SECOND_STRING_CONTACTOR
 *            - RT15/15: Call function in BMS_FSM_SUBSTATE_CHECK_SECOND_STRING_CONTACTOR and contactor feedback invalid - proceed to BMS_FSM_SUBSTATE_OPEN_FIRST_STRING_CONTACTOR
 *
 *
 */
void testBMS_ProcessOpenContactorsToStandbyState(void) {
    /* ======= Routine tests =============================================== */
    /* ======= RT1/14: Test implementation */
    BMS_STATE_s state = (BMS_STATE_s){
        .timer           = 0u,
        .currentState    = BMS_FSM_STATE_OPEN_CONTACTORS_TO_STANDBY,
        .currentSubstate = BMS_FSM_SUBSTATE_ENTRY};
    BAL_SetStateRequest_ExpectAndReturn(BAL_STATE_NO_BALANCING_REQUEST, BAL_OK);
    BMS_FSM_STATES_e result = TEST_BMS_ProcessOpenContactorsToStandbyState(&state);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SHORTTIME, state.timer);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_OPEN_CONTACTORS_TO_STANDBY, result);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_OPEN_ALL_PRECHARGE_CONTACTORS, state.currentSubstate);

    /* ======= RT2/14: Test implementation */
    state = (BMS_STATE_s){
        .timer           = 0u,
        .currentState    = BMS_FSM_STATE_OPEN_CONTACTORS_TO_STANDBY,
        .currentSubstate = BMS_FSM_SUBSTATE_OPEN_ALL_PRECHARGE_CONTACTORS};
    CONT_OpenAllPrechargeContactors_Expect();
    result = TEST_BMS_ProcessOpenContactorsToStandbyState(&state);
    TEST_ASSERT_EQUAL_INT(BMS_TIME_WAIT_AFTER_OPENING_PRECHARGE, state.timer);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_OPEN_CONTACTORS_TO_STANDBY, result);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_OPEN_FIRST_STRING_CONTACTOR, state.currentSubstate);

    /* ======= RT3/14: Test implementation */
    state = (BMS_STATE_s){
        .timer                    = 0u,
        .currentState             = BMS_FSM_STATE_OPEN_CONTACTORS_TO_STANDBY,
        .currentSubstate          = BMS_FSM_SUBSTATE_OPEN_FIRST_STRING_CONTACTOR,
        .information.stringNumber = 0};
    bms_tablePackValues->invalidStringCurrent[0] = 0u;
    bms_tablePackValues->stringCurrent_mA[0]     = 0u;
    TEST_SetTablePackValues(bms_tablePackValues);
    CONT_OpenContactor_Stub(OpenContactorStub);
    result = TEST_BMS_ProcessOpenContactorsToStandbyState(&state);
    TEST_ASSERT_EQUAL_INT(BMS_WAIT_TIME_AFTER_OPENING_STRING_CONTACTOR, state.timer);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_OPEN_CONTACTORS_TO_STANDBY, result);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_OPEN_SECOND_STRING_CONTACTOR, state.currentSubstate);

    /* ======= RT4/14: Test implementation */
    state = (BMS_STATE_s){
        .timer                                         = 0u,
        .currentState                                  = BMS_FSM_STATE_OPEN_CONTACTORS_TO_STANDBY,
        .currentSubstate                               = BMS_FSM_SUBSTATE_OPEN_FIRST_STRING_CONTACTOR,
        .information.stringNumber                      = 0,
        .information.timeAboveContactorBreakCurrent_ms = BS_MAIN_FUSE_MAXIMUM_TRIGGER_DURATION_ms,
    };
    bms_tablePackValues->invalidStringCurrent[0] = 1u;
    TEST_SetTablePackValues(bms_tablePackValues);
    DIAG_Handler_ExpectAndReturn(DIAG_ID_ALERT_MODE, DIAG_EVENT_NOT_OK, DIAG_SYSTEM, 0u, DIAG_HANDLER_RETURN_OK);
    CONT_OpenContactor_Stub(OpenContactorStub);
    result = TEST_BMS_ProcessOpenContactorsToStandbyState(&state);
    TEST_ASSERT_EQUAL_INT(BMS_WAIT_TIME_AFTER_OPENING_STRING_CONTACTOR, state.timer);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_OPEN_CONTACTORS_TO_STANDBY, result);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_OPEN_SECOND_STRING_CONTACTOR, state.currentSubstate);

    /* ======= RT5/14: Test implementation */
    state = (BMS_STATE_s){
        .timer                           = 0u,
        .currentState                    = BMS_FSM_STATE_OPEN_CONTACTORS_TO_STANDBY,
        .currentSubstate                 = BMS_FSM_SUBSTATE_OPEN_SECOND_STRING_CONTACTOR,
        .information.contactorToBeOpened = 0u,
        .information.stringToBeOpened    = 0u,
    };
    CONT_GetContactorState_ExpectAndReturn(0u, 0u, CONT_SWITCH_OFF);

    /*BMS_IsContactorFeedbackValid*/
    DATA_BLOCK_STATE_REQUEST_s request = {.header.uniqueId = DATA_BLOCK_ID_ERROR_STATE};
    DATA_Read1DataBlock_ExpectAndReturn(&request, STD_OK);

    CONT_OpenContactor_Stub(OpenContactorStub);
    result = TEST_BMS_ProcessOpenContactorsToStandbyState(&state);
    TEST_ASSERT_EQUAL_INT(BMS_WAIT_TIME_AFTER_OPENING_STRING_CONTACTOR, state.timer);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_OPEN_CONTACTORS_TO_STANDBY, result);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_CHECK_SECOND_STRING_CONTACTOR, state.currentSubstate);

    /* ======= RT6/14: Test implementation */
    state = (BMS_STATE_s){
        .timer                           = 0u,
        .currentState                    = BMS_FSM_STATE_OPEN_CONTACTORS_TO_STANDBY,
        .currentSubstate                 = BMS_FSM_SUBSTATE_OPEN_SECOND_STRING_CONTACTOR,
        .information.contactorToBeOpened = 0u,
        .information.stringToBeOpened    = 0u,
    };
    CONT_GetContactorState_ExpectAndReturn(0u, 0u, CONT_SWITCH_ON);

    /*BMS_IsContactorFeedbackValid*/
    DATA_Read1DataBlock_ExpectAndReturn(&request, STD_OK);

    CONT_OpenContactor_Stub(OpenContactorStub);
    result = TEST_BMS_ProcessOpenContactorsToStandbyState(&state);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SHORTTIME, state.timer);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_OPEN_CONTACTORS_TO_STANDBY, result);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_OPEN_SECOND_STRING_CONTACTOR, state.currentSubstate);

    /* ======= RT7/14: Test implementation */
    state = (BMS_STATE_s){
        .timer                    = 0u,
        .currentState             = BMS_FSM_STATE_OPEN_CONTACTORS_TO_STANDBY,
        .currentSubstate          = BMS_FSM_SUBSTATE_CHECK_SECOND_STRING_CONTACTOR,
        .information.stringNumber = 1u,
    };
    CONT_GetContactorState_ExpectAndReturn(0u, 0u, CONT_SWITCH_OFF);

    /*BMS_IsContactorFeedbackValid*/
    DATA_Read1DataBlock_ExpectAndReturn(&request, STD_OK);

    result = TEST_BMS_ProcessOpenContactorsToStandbyState(&state);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SHORTTIME, state.timer);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_OPEN_CONTACTORS_TO_STANDBY, result);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_OPEN_FIRST_STRING_CONTACTOR, state.currentSubstate);

    /* ======= RT8/14: Test implementation */
    state = (BMS_STATE_s){
        .timer                    = 0u,
        .currentState             = BMS_FSM_STATE_OPEN_CONTACTORS_TO_STANDBY,
        .currentSubstate          = BMS_FSM_SUBSTATE_CHECK_SECOND_STRING_CONTACTOR,
        .information.stringNumber = 0u,
    };
    CONT_GetContactorState_ExpectAndReturn(0u, 0u, CONT_SWITCH_OFF);

    /*BMS_IsContactorFeedbackValid*/
    DATA_Read1DataBlock_ExpectAndReturn(&request, STD_OK);

    result = TEST_BMS_ProcessOpenContactorsToStandbyState(&state);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SHORTTIME, state.timer);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_OPEN_CONTACTORS_TO_STANDBY, result);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_OPEN_STRINGS_EXIT, state.currentSubstate);

    /* ======= RT9/14: Test implementation */
    state = (BMS_STATE_s){
        .timer                         = 0u,
        .currentState                  = BMS_FSM_STATE_OPEN_CONTACTORS_TO_STANDBY,
        .currentSubstate               = BMS_FSM_SUBSTATE_CHECK_SECOND_STRING_CONTACTOR,
        .information.stringOpenTimeout = 0u,
    };
    CONT_GetContactorState_ExpectAndReturn(0u, 0u, CONT_SWITCH_ON);

    /*BMS_IsContactorFeedbackValid*/
    DATA_Read1DataBlock_ExpectAndReturn(&request, STD_OK);

    result = TEST_BMS_ProcessOpenContactorsToStandbyState(&state);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SHORTTIME, state.timer);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_OPEN_CONTACTORS_TO_STANDBY, result);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_OPEN_FIRST_STRING_CONTACTOR, state.currentSubstate);

    /* ======= RT10/14: Test implementation */
    state = (BMS_STATE_s){
        .timer                         = 0u,
        .currentState                  = BMS_FSM_STATE_OPEN_CONTACTORS_TO_STANDBY,
        .currentSubstate               = BMS_FSM_SUBSTATE_CHECK_SECOND_STRING_CONTACTOR,
        .information.stringOpenTimeout = 1u,
    };
    CONT_GetContactorState_ExpectAndReturn(0u, 0u, CONT_SWITCH_ON);

    /*BMS_IsContactorFeedbackValid*/
    DATA_Read1DataBlock_ExpectAndReturn(&request, STD_OK);

    CONT_OpenContactor_Stub(OpenContactorStub);
    result = TEST_BMS_ProcessOpenContactorsToStandbyState(&state);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SHORTTIME, state.timer);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_OPEN_CONTACTORS_TO_STANDBY, result);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_CHECK_SECOND_STRING_CONTACTOR, state.currentSubstate);

    /* ======= RT11/14: Test implementation */
    state = (BMS_STATE_s){
        .timer           = 0u,
        .currentState    = BMS_FSM_STATE_OPEN_CONTACTORS_TO_STANDBY,
        .currentSubstate = BMS_FSM_SUBSTATE_OPEN_STRINGS_EXIT,
    };
    result = TEST_BMS_ProcessOpenContactorsToStandbyState(&state);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_STANDBY, result);

    /* ======= RT12/14: Test implementation */
    state = (BMS_STATE_s){
        .timer                                         = 0u,
        .currentState                                  = BMS_FSM_STATE_OPEN_CONTACTORS_TO_STANDBY,
        .currentSubstate                               = BMS_FSM_SUBSTATE_OPEN_FIRST_STRING_CONTACTOR,
        .information.stringNumber                      = 0u,
        .information.timeAboveContactorBreakCurrent_ms = 0u,
    };
    bms_tablePackValues->invalidStringCurrent[0] = 1u;
    TEST_SetTablePackValues(bms_tablePackValues);
    result = TEST_BMS_ProcessOpenContactorsToStandbyState(&state);
    TEST_ASSERT_EQUAL_INT(BMS_STATEMACHINE_TASK_CYCLE_CONTEXT_MS, state.information.timeAboveContactorBreakCurrent_ms);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_OPEN_CONTACTORS_TO_STANDBY, result);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_OPEN_FIRST_STRING_CONTACTOR, state.currentSubstate);

    /* ======= RT13/14: Test implementation */
    state = (BMS_STATE_s){
        .timer                           = 0u,
        .currentState                    = BMS_FSM_STATE_OPEN_CONTACTORS_TO_STANDBY,
        .currentSubstate                 = BMS_FSM_SUBSTATE_OPEN_SECOND_STRING_CONTACTOR,
        .information.stringNumber        = 1u,
        .information.contactorToBeOpened = CONT_PLUS,
        .information.stringToBeOpened    = 1u,
    };
    CONT_GetContactorState_ExpectAndReturn(1u, CONT_PLUS, CONT_SWITCH_ON);
    DATA_BLOCK_ERROR_STATE_s tableErrorFlags                         = {.header.uniqueId = DATA_BLOCK_ID_ERROR_STATE};
    tableErrorFlags.contactorInPositivePathOfStringFeedbackError[1u] = true;
    DATA_Read1DataBlock_ExpectAndReturn(&tableErrorFlags, STD_OK);
    DATA_Read1DataBlock_ReturnThruPtr_pDataToReceiver0(&tableErrorFlags);
    CONT_OpenContactor_Stub(OpenContactorStub);
    result = TEST_BMS_ProcessOpenContactorsToStandbyState(&state);
    TEST_ASSERT_EQUAL_INT(BMS_WAIT_TIME_AFTER_OPENING_STRING_CONTACTOR, state.timer);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_OPEN_CONTACTORS_TO_STANDBY, result);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_CHECK_SECOND_STRING_CONTACTOR, state.currentSubstate);

    /* ======= RT14/14: Test implementation */
    state = (BMS_STATE_s){
        .timer                           = 0u,
        .currentState                    = BMS_FSM_STATE_OPEN_CONTACTORS_TO_STANDBY,
        .currentSubstate                 = BMS_FSM_SUBSTATE_CHECK_SECOND_STRING_CONTACTOR,
        .information.stringNumber        = 1u,
        .information.contactorToBeOpened = CONT_PLUS,
        .information.stringToBeOpened    = 1u,
    };
    CONT_GetContactorState_ExpectAndReturn(1u, CONT_PLUS, CONT_SWITCH_ON);
    tableErrorFlags.contactorInPositivePathOfStringFeedbackError[1u] = true;
    DATA_Read1DataBlock_ExpectAndReturn(&tableErrorFlags, STD_OK);
    DATA_Read1DataBlock_ReturnThruPtr_pDataToReceiver0(&tableErrorFlags);
    result = TEST_BMS_ProcessOpenContactorsToStandbyState(&state);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SHORTTIME, state.timer);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_OPEN_CONTACTORS_TO_STANDBY, result);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_OPEN_FIRST_STRING_CONTACTOR, state.currentSubstate);
}

/**
 * @brief   Testing static function #BMS_RunStateMachine with currentState OPEN_CONTACTORS_TO_STANDBY
 * @details The following cases need to be tested:
 *          - Routine validation:
 *            - RT1/3: Call function in BMS_FSM_SUBSTATE_ENTRY - proceed to BMS_FSM_SUBSTATE_HANDLE_SUPPLY_VOLTAGE_30C_LOSS
 *            - RT2/3: Call function in BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS - proceed to BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR
 *            - RT3/3: Call function in BMS_FSM_SUBSTATE_CHECK_STATE_REQUESTS - proceed to BMS_FSM_STATE_PRECHARGE
 *
 *
 */
void testBMS_RunStateMachine_Standby(void) {
    /* ======= Routine tests =============================================== */
    /* ======= RT1/3: Test implementation */
    BMS_STATE_s state = {.timer = 0u, .currentState = BMS_FSM_STATE_STANDBY, .currentSubstate = BMS_FSM_SUBSTATE_ENTRY};
    DATA_BLOCK_SYSTEM_STATE_s systemState = {.header.uniqueId = DATA_BLOCK_ID_SYSTEM_STATE};
    BAL_SetStateRequest_ExpectAndReturn(BAL_STATE_ALLOW_BALANCING_REQUEST, BAL_OK);
    DATA_Read1DataBlock_ExpectAndReturn(&systemState, STD_OK);
    DATA_Write1DataBlock_ExpectAndReturn(&systemState, STD_OK);
    TEST_BMS_RunStateMachine(&state);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_MEDIUM_TIME, state.timer);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_STANDBY, state.currentState);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS_INTERLOCK, state.currentSubstate);

    /* ======= RT2/3: Test implementation */
    state = (BMS_STATE_s){
        .timer                              = 0u,
        .currentState                       = BMS_FSM_STATE_STANDBY,
        .currentSubstate                    = BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS,
        .information.transitionToErrorState = true};

    /*BMS_IsBatterySystemStateOkay*/
    OS_GetTickCount_ExpectAndReturn(1u);

    TEST_BMS_RunStateMachine(&state);

    TEST_ASSERT_EQUAL_INT(BMS_FSM_SHORTTIME, state.timer);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR, state.currentState);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_ENTRY, state.currentSubstate);

    /* ======= RT3/3: Test implementation */
    state = (BMS_STATE_s){
        .timer           = 0u,
        .currentState    = BMS_FSM_STATE_STANDBY,
        .currentSubstate = BMS_FSM_SUBSTATE_CHECK_STATE_REQUESTS,
    };

    DATA_BLOCK_STATE_REQUEST_s request = {.header.uniqueId = DATA_BLOCK_ID_STATE_REQUEST};

    /*BMS_CheckCanRequest*/
    request.stateRequestViaCan = BMS_REQ_ID_CHARGE;
    DATA_Read1DataBlock_ExpectAndReturn(&request, STD_OK);
    DATA_Read1DataBlock_ReturnThruPtr_pDataToReceiver0(&request);

    /*BMS_CheckCanRequest*/
    DATA_Read1DataBlock_ExpectAndReturn(&request, STD_OK);
    DATA_Read1DataBlock_ReturnThruPtr_pDataToReceiver0(&request);

    TEST_BMS_RunStateMachine(&state);

    TEST_ASSERT_EQUAL_INT(BMS_POWER_PATH_1, state.information.powerPath);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SHORTTIME, state.timer);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_PRECHARGE, state.currentState);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_ENTRY, state.currentSubstate);
}

/**
 * @brief   Testing static function #BMS_ProcessStandbyState
 * @details The following cases need to be tested:
 *          - Routine validation:
 *            - RT1/9: Call function in BMS_FSM_SUBSTATE_ENTRY - proceed to BMS_FSM_SUBSTATE_HANDLE_SUPPLY_VOLTAGE_30C_LOSS
 *            - RT2/9: Call function in BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS_INTERLOCK and battery system currentState is not okay - proceed to BMS_FSM_STATE_ERROR
 *            - RT3/9: Call function in BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS_INTERLOCK and battery system currentState is okay - proceed to BMS_FSM_SUBSTATE_INTERLOCK_CHECKED
 *            - RT4/9: Call function in BMS_FSM_SUBSTATE_INTERLOCK_CHECKED - proceed to BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS
 *            - RT5/9: Call function in BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS and battery system currentState is not okay - proceed to BMS_FSM_STATE_ERROR
 *            - RT6/9: Call function in BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS and battery system currentState is okay - proceed to BMS_FSM_SUBSTATE_CHECK_STATE_REQUESTS
 *            - RT7/9: Call function in BMS_FSM_SUBSTATE_CHECK_STATE_REQUESTS and req normal - proceed to BMS_FSM_STATE_PRECHARGE
 *            - RT8/9: Call function in BMS_FSM_SUBSTATE_CHECK_STATE_REQUESTS and req charge - proceed to BMS_FSM_STATE_PRECHARGE
 *            - RT9/9: Call function in BMS_FSM_SUBSTATE_CHECK_STATE_REQUESTS and other req - proceed to BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS
 *
 *
 */
void testBMS_ProcessStandbyState(void) {
    /* ======= Routine tests =============================================== */
    /* ======= RT1/9: Test implementation */
    BMS_STATE_s currentState = {
        .timer = 0u, .currentState = BMS_FSM_STATE_STANDBY, .currentSubstate = BMS_FSM_SUBSTATE_ENTRY};
    DATA_BLOCK_SYSTEM_STATE_s systemState = {.header.uniqueId = DATA_BLOCK_ID_SYSTEM_STATE};
    BAL_SetStateRequest_ExpectAndReturn(BAL_STATE_ALLOW_BALANCING_REQUEST, BAL_OK);
    DATA_Read1DataBlock_ExpectAndReturn(&systemState, STD_OK);
    DATA_Write1DataBlock_ExpectAndReturn(&systemState, STD_OK);
    BMS_FSM_STATES_e result = TEST_BMS_ProcessStandbyState(&currentState);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_MEDIUM_TIME, currentState.timer);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_STANDBY, result);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS_INTERLOCK, currentState.currentSubstate);

    /* ======= RT2/9: Test implementation */
    currentState = (BMS_STATE_s){
        .timer                              = 0u,
        .currentState                       = BMS_FSM_STATE_STANDBY,
        .currentSubstate                    = BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS_INTERLOCK,
        .information.transitionToErrorState = true};

    /*BMS_IsBatterySystemStateOkay*/
    OS_GetTickCount_ExpectAndReturn(1u);

    result = TEST_BMS_ProcessStandbyState(&currentState);

    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR, result);

    /* ======= RT3/9: Test implementation */
    currentState = (BMS_STATE_s){
        .timer                              = 0u,
        .currentState                       = BMS_FSM_STATE_STANDBY,
        .currentSubstate                    = BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS_INTERLOCK,
        .information.transitionToErrorState = false};

    /*BMS_IsBatterySystemStateOkay*/
    OS_GetTickCount_ExpectAndReturn(1u);

    result = TEST_BMS_ProcessStandbyState(&currentState);

    TEST_ASSERT_EQUAL_INT(BMS_FSM_SHORTTIME, currentState.timer);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_STANDBY, result);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_INTERLOCK_CHECKED, currentState.currentSubstate);

    /* ======= RT4/9: Test implementation */
    currentState = (BMS_STATE_s){
        .timer = 0u, .currentState = BMS_FSM_STATE_STANDBY, .currentSubstate = BMS_FSM_SUBSTATE_INTERLOCK_CHECKED};
    result = TEST_BMS_ProcessStandbyState(&currentState);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SHORTTIME, currentState.timer);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_STANDBY, result);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS, currentState.currentSubstate);

    /* ======= RT5/9: Test implementation */
    currentState = (BMS_STATE_s){
        .timer                              = 0u,
        .currentState                       = BMS_FSM_STATE_STANDBY,
        .currentSubstate                    = BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS,
        .information.transitionToErrorState = true};

    /*BMS_IsBatterySystemStateOkay*/
    OS_GetTickCount_ExpectAndReturn(1u);

    result = TEST_BMS_ProcessStandbyState(&currentState);

    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR, result);

    /* ======= RT6/9: Test implementation */
    currentState = (BMS_STATE_s){
        .timer                              = 0u,
        .currentState                       = BMS_FSM_STATE_STANDBY,
        .currentSubstate                    = BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS,
        .information.transitionToErrorState = false};

    /*BMS_IsBatterySystemStateOkay*/
    OS_GetTickCount_ExpectAndReturn(1u);

    result = TEST_BMS_ProcessStandbyState(&currentState);

    TEST_ASSERT_EQUAL_INT(BMS_FSM_SHORTTIME, currentState.timer);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_STANDBY, result);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_CHECK_STATE_REQUESTS, currentState.currentSubstate);

    /* ======= RT7/9: Test implementation */
    currentState = (BMS_STATE_s){
        .timer           = 0u,
        .currentState    = BMS_FSM_STATE_STANDBY,
        .currentSubstate = BMS_FSM_SUBSTATE_CHECK_STATE_REQUESTS,
    };

    DATA_BLOCK_STATE_REQUEST_s request = {.header.uniqueId = DATA_BLOCK_ID_STATE_REQUEST};

    /*BMS_CheckCanRequest*/
    request.stateRequestViaCan = BMS_REQ_ID_NORMAL;
    DATA_Read1DataBlock_ExpectAndReturn(&request, STD_OK);
    DATA_Read1DataBlock_ReturnThruPtr_pDataToReceiver0(&request);

    result = TEST_BMS_ProcessStandbyState(&currentState);

    TEST_ASSERT_EQUAL_INT(BMS_POWER_PATH_0, currentState.information.powerPath);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_PRECHARGE, result);

    /* ======= RT8/9: Test implementation */
    currentState = (BMS_STATE_s){
        .timer           = 0u,
        .currentState    = BMS_FSM_STATE_STANDBY,
        .currentSubstate = BMS_FSM_SUBSTATE_CHECK_STATE_REQUESTS,
    };

    /*BMS_CheckCanRequest*/
    request.stateRequestViaCan = BMS_REQ_ID_CHARGE;
    DATA_Read1DataBlock_ExpectAndReturn(&request, STD_OK);
    DATA_Read1DataBlock_ReturnThruPtr_pDataToReceiver0(&request);

    /*BMS_CheckCanRequest*/
    DATA_Read1DataBlock_ExpectAndReturn(&request, STD_OK);
    DATA_Read1DataBlock_ReturnThruPtr_pDataToReceiver0(&request);

    result = TEST_BMS_ProcessStandbyState(&currentState);

    TEST_ASSERT_EQUAL_INT(BMS_POWER_PATH_1, currentState.information.powerPath);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_PRECHARGE, result);

    /* ======= RT9/9: Test implementation */
    currentState = (BMS_STATE_s){
        .timer           = 0u,
        .currentState    = BMS_FSM_STATE_STANDBY,
        .currentSubstate = BMS_FSM_SUBSTATE_CHECK_STATE_REQUESTS,
    };

    /*BMS_CheckCanRequest*/
    request.stateRequestViaCan = BMS_REQ_ID_NOREQ;
    DATA_Read1DataBlock_ExpectAndReturn(&request, STD_OK);
    DATA_Read1DataBlock_ReturnThruPtr_pDataToReceiver0(&request);
    DATA_Read1DataBlock_ExpectAndReturn(&request, STD_OK);
    DATA_Read1DataBlock_ReturnThruPtr_pDataToReceiver0(&request);

    result = TEST_BMS_ProcessStandbyState(&currentState);

    TEST_ASSERT_EQUAL_INT(BMS_FSM_SHORTTIME, currentState.timer);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_STANDBY, result);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS, currentState.currentSubstate);
}

static STD_RETURN_TYPE_e CloseContactorRetVal = STD_OK;

static STD_RETURN_TYPE_e CloseContactorStub(uint8_t stringNumber, CONT_TYPE_e contactor, int cmock_num) {
    (void)stringNumber;
    (void)contactor;
    (void)cmock_num;

    return CloseContactorRetVal;
}

/**
 * @brief   Testing static function #BMS_RunStateMachine with currentState BMS_FSM_STATE_PRECHARGE
 * @details The following cases need to be tested:
 *          - Routine validation:
 *              - 1/3 follow path to BMS_FSM_STATE_PRECHARGE
 *              - 2/3 follow path to BMS_FSM_STATE_NORMAL
 *              - 3/3 follow path to BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR
 *
 */
void testBMS_RunStateMachine_Precharge(void) {
    /* ======= RT1/3: Test implementation */
    BMS_STATE_s state = {
        .timer                          = 0u,
        .currentState                   = BMS_FSM_STATE_PRECHARGE,
        .currentSubstate                = BMS_FSM_SUBSTATE_ENTRY,
        .nextState                      = BMS_FSM_STATE_NORMAL,
        .information.OscillationTimeout = 0u,
    };
    DATA_BLOCK_SYSTEM_STATE_s systemState = {.header.uniqueId = DATA_BLOCK_ID_SYSTEM_STATE};
    DATA_Read1DataBlock_ExpectAndReturn(&systemState, STD_OK);
    DATA_Write1DataBlock_ExpectAndReturn(&systemState, STD_OK);
    CONT_CloseContactor_Stub(CloseContactorStub);
    TEST_BMS_RunStateMachine(&state);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_PRECHARGE, state.currentState);

    /* ======= RT2/3: Test implementation */
    state = (BMS_STATE_s){
        .timer                         = 0u,
        .currentState                  = BMS_FSM_STATE_PRECHARGE,
        .currentSubstate               = BMS_FSM_SUBSTATE_PRECHARGE_CHECK_OPEN_PRECHARGE,
        .information.firstClosedString = 0u,
    };
    CONT_GetContactorState_ExpectAndReturn(0u, CONT_PRECHARGE, CONT_SWITCH_OFF);
    TEST_BMS_RunStateMachine(&state);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SHORTTIME, state.timer);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_NORMAL, state.currentState);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_ENTRY, state.currentSubstate);

    /* ======= RT3/3: Test implementation */
    state = (BMS_STATE_s){
        .timer                          = 0u,
        .currentState                   = BMS_FSM_STATE_PRECHARGE,
        .currentSubstate                = BMS_FSM_SUBSTATE_ENTRY,
        .nextState                      = BMS_FSM_STATE_NORMAL,
        .information.OscillationTimeout = 0u,
    };
    DATA_Read1DataBlock_ExpectAndReturn(&systemState, STD_OK);
    DATA_Write1DataBlock_ExpectAndReturn(&systemState, STD_OK);
    CloseContactorRetVal = STD_NOT_OK;
    CONT_CloseContactor_Stub(CloseContactorStub);
    TEST_BMS_RunStateMachine(&state);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR, state.currentState);
}

/**
 * @brief   Testing static function #BMS_ProcessPrechargeState
 * @details The following cases need to be tested:
 *          - Routine validation:
 *            - RT01/22: Call function in BMS_FSM_SUBSTATE_ENTRY and nextState is charge - proceed to BMS_FSM_SUBSTATE_PRECHARGE_CLOSE_PRECHARGE
 *            - RT02/22: Call function in BMS_FSM_SUBSTATE_ENTRY and nextState is not charge - proceed to BMS_FSM_SUBSTATE_PRECHARGE_CLOSE_PRECHARGE
 *            - RT03/22: Call function in BMS_FSM_SUBSTATE_ENTRY and CloseContactor fails - proceed to BMS_FSM_STATE_OPEN_CONTACTOR
 *            - RT04/22: Call function in BMS_FSM_SUBSTATE_ENTRY and OscillationTimeout is bigger than 0 but BatterySystemState is not okay - proceed to BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR
 *            - RT05/22: Call function in BMS_FSM_SUBSTATE_ENTRY and OscillationTimeout is bigger than 0 and BatterySystemState is okay - do nothing
 *            - RT06/22: Call function in BMS_FSM_SUBSTATE_PRECHARGE_CLOSE_PRECHARGE and contactorState is ON, Contactor closes successfully  - proceed to BMS_FSM_SUBSTATE_PRECHARGE_CHECK_PRECHARGE_PROCESS
 *            - RT07/22: Call function in BMS_FSM_SUBSTATE_PRECHARGE_CLOSE_PRECHARGE and contactorState is ON, Contactor fails to close - proceed to BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR
 *            - RT08/22: Call function in BMS_FSM_SUBSTATE_PRECHARGE_CLOSE_PRECHARGE and contactorState is OFF, stringCloseTimeout is 0 - proceed to BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR
 *            - RT09/22: Call function in BMS_FSM_SUBSTATE_PRECHARGE_CLOSE_PRECHARGE and contactorState is OFF, stringCloseTimeout is bigger than 0  - do nothing
 *            - RT10/22: Call function in BMS_FSM_SUBSTATE_PRECHARGE_CHECK_PRECHARGE_PROCESS and battery system currentState is not okay - proceed to BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR
 *            - RT11/22: Call function in BMS_FSM_SUBSTATE_PRECHARGE_CHECK_PRECHARGE_PROCESS and request is standby - proceed to BMS_FSM_STATE_OPEN_CONTACTORS_TO_STANDBY
 *            - RT12/22: Call function in BMS_FSM_SUBSTATE_CHECK_CLOSE_SECOND_STRING_CONTACTOR_PRECHARGE_STATE and plus contactor is ON
 *            - RT13/22: Call function in BMS_FSM_SUBSTATE_CHECK_CLOSE_SECOND_STRING_CONTACTOR_PRECHARGE_STATE and close timeout elapsed
 *            - RT14/22: Call function in BMS_FSM_SUBSTATE_CHECK_CLOSE_SECOND_STRING_CONTACTOR_PRECHARGE_STATE and plus contactor not yet closed
 *            - RT15/22: Call function in BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS_PRECHARGE_FIRST_STRING and battery system currentState is okay
 *            - RT16/22: Call function in BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS_PRECHARGE_FIRST_STRING and battery system currentState is not okay
 *            - RT17/22: Call function in BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS_PRECHARGE_CLOSING_STRINGS and battery system currentState is okay
 *            - RT18/22: Call function in BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS_PRECHARGE_CLOSING_STRINGS and battery system currentState is not okay
 *            - RT19/22: Call function in BMS_FSM_SUBSTATE_PRECHARGE_OPEN_PRECHARGE and open precharge succeeds
 *            - RT20/22: Call function in BMS_FSM_SUBSTATE_PRECHARGE_OPEN_PRECHARGE and open precharge fails
 *            - RT21/22: Call function in BMS_FSM_SUBSTATE_PRECHARGE_CHECK_OPEN_PRECHARGE and precharge contactor is OFF
 *            - RT22/22: Call function in BMS_FSM_SUBSTATE_PRECHARGE_CHECK_OPEN_PRECHARGE and precharge contactor remains ON
 *
 *
 */
void testBMS_ProcessPrechargeState(void) {
    /* ======= Routine tests =============================================== */
    /* ======= RT1/22: Test implementation */
    BMS_STATE_s state = {
        .timer                          = 0u,
        .currentState                   = BMS_FSM_STATE_PRECHARGE,
        .currentSubstate                = BMS_FSM_SUBSTATE_ENTRY,
        .nextState                      = BMS_FSM_STATE_NORMAL,
        .information.OscillationTimeout = 0u,
    };
    DATA_BLOCK_SYSTEM_STATE_s systemState = {.header.uniqueId = DATA_BLOCK_ID_SYSTEM_STATE};
    DATA_Read1DataBlock_ExpectAndReturn(&systemState, STD_OK);
    DATA_Write1DataBlock_ExpectAndReturn(&systemState, STD_OK);
    CloseContactorRetVal = STD_OK;
    CONT_CloseContactor_Stub(CloseContactorStub);
    BMS_FSM_STATES_e result = TEST_BMS_ProcessPrechargeState(&state);
    TEST_ASSERT_EQUAL_INT(BMS_WAIT_TIME_AFTER_CLOSING_STRING_CONTACTOR, state.timer);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_PRECHARGE, result);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_PRECHARGE_CLOSE_PRECHARGE, state.currentSubstate);

    /* ======= RT2/22: Test implementation */
    state = (BMS_STATE_s){
        .timer                          = 0u,
        .currentState                   = BMS_FSM_STATE_PRECHARGE,
        .currentSubstate                = BMS_FSM_SUBSTATE_ENTRY,
        .nextState                      = BMS_FSM_STATE_NORMAL,
        .information.OscillationTimeout = 0u,
    };
    DATA_Read1DataBlock_ExpectAndReturn(&systemState, STD_OK);
    DATA_Write1DataBlock_ExpectAndReturn(&systemState, STD_OK);
    CONT_CloseContactor_Stub(CloseContactorStub);
    result = TEST_BMS_ProcessPrechargeState(&state);
    TEST_ASSERT_EQUAL_INT(BMS_WAIT_TIME_AFTER_CLOSING_STRING_CONTACTOR, state.timer);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_PRECHARGE, result);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_PRECHARGE_CLOSE_PRECHARGE, state.currentSubstate);

    /* ======= RT3/22: Test implementation */
    state = (BMS_STATE_s){
        .timer                          = 0u,
        .currentState                   = BMS_FSM_STATE_PRECHARGE,
        .currentSubstate                = BMS_FSM_SUBSTATE_ENTRY,
        .nextState                      = BMS_FSM_STATE_NORMAL,
        .information.OscillationTimeout = 0u,
    };
    DATA_Read1DataBlock_ExpectAndReturn(&systemState, STD_OK);
    DATA_Write1DataBlock_ExpectAndReturn(&systemState, STD_OK);
    CloseContactorRetVal = STD_NOT_OK;
    CONT_CloseContactor_Stub(CloseContactorStub);
    result = TEST_BMS_ProcessPrechargeState(&state);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR, result);

    /* ======= RT4/22: Test implementation */
    state = (BMS_STATE_s){
        .timer                              = 0u,
        .currentState                       = BMS_FSM_STATE_PRECHARGE,
        .currentSubstate                    = BMS_FSM_SUBSTATE_ENTRY,
        .nextState                          = BMS_FSM_STATE_NORMAL,
        .information.OscillationTimeout     = 1u,
        .information.transitionToErrorState = true,
    };
    DATA_Read1DataBlock_ExpectAndReturn(&systemState, STD_OK);
    DATA_Write1DataBlock_ExpectAndReturn(&systemState, STD_OK);

    /*BMS_IsBatterySystemStateOkay*/
    OS_GetTickCount_ExpectAndReturn(1u);

    result = TEST_BMS_ProcessPrechargeState(&state);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR, result);

    /* ======= RT5/22: Test implementation */
    state = (BMS_STATE_s){
        .timer                              = 0u,
        .currentState                       = BMS_FSM_STATE_PRECHARGE,
        .currentSubstate                    = BMS_FSM_SUBSTATE_ENTRY,
        .nextState                          = BMS_FSM_STATE_NORMAL,
        .information.OscillationTimeout     = 1u,
        .information.transitionToErrorState = false,
    };
    DATA_Read1DataBlock_ExpectAndReturn(&systemState, STD_OK);
    DATA_Write1DataBlock_ExpectAndReturn(&systemState, STD_OK);

    /*BMS_IsBatterySystemStateOkay*/
    OS_GetTickCount_ExpectAndReturn(1u);

    result = TEST_BMS_ProcessPrechargeState(&state);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_PRECHARGE, result);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_ENTRY, state.currentSubstate);

    /* ======= RT6/22: Test implementation */
    state = (BMS_STATE_s){
        .timer                         = 0u,
        .currentState                  = BMS_FSM_STATE_PRECHARGE,
        .currentSubstate               = BMS_FSM_SUBSTATE_PRECHARGE_CLOSE_PRECHARGE,
        .information.firstClosedString = 0u,
    };
    CONT_GetContactorState_ExpectAndReturn(0u, CONT_MINUS, CONT_SWITCH_ON);

    CONT_ClosePrecharge_ExpectAndReturn(0u, STD_OK);

    result = TEST_BMS_ProcessPrechargeState(&state);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SHORTTIME, state.timer);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_PRECHARGE, result);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_PRECHARGE_CHECK_PRECHARGE_PROCESS, state.currentSubstate);

    /* ======= RT7/22: Test implementation */
    state = (BMS_STATE_s){
        .timer                         = 0u,
        .currentState                  = BMS_FSM_STATE_PRECHARGE,
        .currentSubstate               = BMS_FSM_SUBSTATE_PRECHARGE_CLOSE_PRECHARGE,
        .information.firstClosedString = 0u,
    };
    CONT_GetContactorState_ExpectAndReturn(0u, CONT_MINUS, CONT_SWITCH_ON);

    CONT_ClosePrecharge_ExpectAndReturn(0u, STD_NOT_OK);

    result = TEST_BMS_ProcessPrechargeState(&state);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR, result);

    /* ======= RT8/22: Test implementation */
    state = (BMS_STATE_s){
        .timer                          = 0u,
        .currentState                   = BMS_FSM_STATE_PRECHARGE,
        .currentSubstate                = BMS_FSM_SUBSTATE_PRECHARGE_CLOSE_PRECHARGE,
        .information.firstClosedString  = 0u,
        .information.stringCloseTimeout = 0u,
    };
    CONT_GetContactorState_ExpectAndReturn(0u, CONT_MINUS, CONT_SWITCH_OFF);

    result = TEST_BMS_ProcessPrechargeState(&state);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR, result);

    /* ======= RT9/9: Test implementation */
    state = (BMS_STATE_s){
        .timer                          = 0u,
        .currentState                   = BMS_FSM_STATE_PRECHARGE,
        .currentSubstate                = BMS_FSM_SUBSTATE_PRECHARGE_CLOSE_PRECHARGE,
        .information.firstClosedString  = 0u,
        .information.stringCloseTimeout = 1u,
    };
    CONT_GetContactorState_ExpectAndReturn(0u, CONT_MINUS, CONT_SWITCH_OFF);

    CONT_CloseContactor_Stub(CloseContactorStub);

    result = TEST_BMS_ProcessPrechargeState(&state);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SHORTTIME, state.timer);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_PRECHARGE, result);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_PRECHARGE_CLOSE_PRECHARGE, state.currentSubstate);

    /* ======= RT10/22: Test implementation */
    state = (BMS_STATE_s){
        .timer                              = 0u,
        .currentState                       = BMS_FSM_STATE_PRECHARGE,
        .currentSubstate                    = BMS_FSM_SUBSTATE_PRECHARGE_CHECK_PRECHARGE_PROCESS,
        .information.transitionToErrorState = true,
    };

    /*BMS_IsBatterySystemStateOkay*/
    OS_GetTickCount_ExpectAndReturn(1u);

    result = TEST_BMS_ProcessPrechargeState(&state);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR, result);

    /* ======= RT11/22: Test implementation */
    state = (BMS_STATE_s){
        .timer                              = 0u,
        .currentState                       = BMS_FSM_STATE_PRECHARGE,
        .currentSubstate                    = BMS_FSM_SUBSTATE_PRECHARGE_CHECK_PRECHARGE_PROCESS,
        .information.transitionToErrorState = false,
    };

    DATA_BLOCK_STATE_REQUEST_s request = {.header.uniqueId = DATA_BLOCK_ID_STATE_REQUEST};

    /*BMS_IsBatterySystemStateOkay*/
    OS_GetTickCount_ExpectAndReturn(1u);

    /*BMS_CheckCanRequest*/
    request.stateRequestViaCan = BMS_REQ_ID_STANDBY;
    DATA_Read1DataBlock_ExpectAndReturn(&request, STD_OK);
    DATA_Read1DataBlock_ReturnThruPtr_pDataToReceiver0(&request);

    result = TEST_BMS_ProcessPrechargeState(&state);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_OPEN_CONTACTORS_TO_STANDBY, result);

    /* ======= RT12/22: Test implementation */
    state = (BMS_STATE_s){
        .timer                             = 0u,
        .currentState                      = BMS_FSM_STATE_PRECHARGE,
        .currentSubstate                   = BMS_FSM_SUBSTATE_CHECK_CLOSE_SECOND_STRING_CONTACTOR_PRECHARGE_STATE,
        .information.firstClosedString     = 0u,
        .information.stringNumber          = 0u,
        .information.numberOfClosedStrings = 0u,
    };
    CONT_GetContactorState_ExpectAndReturn(0u, CONT_PLUS, CONT_SWITCH_ON);
    result = TEST_BMS_ProcessPrechargeState(&state);
    TEST_ASSERT_EQUAL_INT(BMS_WAIT_TIME_AFTER_CLOSING_STRING_CONTACTOR, state.timer);
    TEST_ASSERT_EQUAL_INT(1u, state.information.numberOfClosedStrings);
    TEST_ASSERT_EQUAL_INT(true, state.information.closedStrings[0u]);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_PRECHARGE, result);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS_PRECHARGE_CLOSING_STRINGS, state.currentSubstate);

    /* ======= RT13/22: Test implementation */
    state = (BMS_STATE_s){
        .timer                          = 0u,
        .currentState                   = BMS_FSM_STATE_PRECHARGE,
        .currentSubstate                = BMS_FSM_SUBSTATE_CHECK_CLOSE_SECOND_STRING_CONTACTOR_PRECHARGE_STATE,
        .information.firstClosedString  = 0u,
        .information.stringCloseTimeout = 0u,
    };
    CONT_GetContactorState_ExpectAndReturn(0u, CONT_PLUS, CONT_SWITCH_OFF);
    result = TEST_BMS_ProcessPrechargeState(&state);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR, result);

    /* ======= RT14/22: Test implementation */
    state = (BMS_STATE_s){
        .timer                          = 0u,
        .currentState                   = BMS_FSM_STATE_PRECHARGE,
        .currentSubstate                = BMS_FSM_SUBSTATE_CHECK_CLOSE_SECOND_STRING_CONTACTOR_PRECHARGE_STATE,
        .information.firstClosedString  = 0u,
        .information.stringCloseTimeout = 1u,
    };
    CONT_GetContactorState_ExpectAndReturn(0u, CONT_PLUS, CONT_SWITCH_OFF);
    CONT_CloseContactor_Stub(CloseContactorStub);
    result = TEST_BMS_ProcessPrechargeState(&state);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SHORTTIME, state.timer);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_PRECHARGE, result);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS_PRECHARGE_FIRST_STRING, state.currentSubstate);

    /* ======= RT15/22: Test implementation */
    state = (BMS_STATE_s){
        .timer                              = 0u,
        .currentState                       = BMS_FSM_STATE_PRECHARGE,
        .currentSubstate                    = BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS_PRECHARGE_FIRST_STRING,
        .information.transitionToErrorState = false,
    };
    OS_GetTickCount_ExpectAndReturn(1u);
    result = TEST_BMS_ProcessPrechargeState(&state);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SHORTTIME, state.timer);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_PRECHARGE, result);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_CHECK_CLOSE_SECOND_STRING_CONTACTOR_PRECHARGE_STATE, state.currentSubstate);

    /* ======= RT16/22: Test implementation */
    state = (BMS_STATE_s){
        .timer                              = 0u,
        .currentState                       = BMS_FSM_STATE_PRECHARGE,
        .currentSubstate                    = BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS_PRECHARGE_FIRST_STRING,
        .information.transitionToErrorState = true,
    };
    OS_GetTickCount_ExpectAndReturn(1u);
    result = TEST_BMS_ProcessPrechargeState(&state);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR, result);

    /* ======= RT17/22: Test implementation */
    state = (BMS_STATE_s){
        .timer                              = 0u,
        .currentState                       = BMS_FSM_STATE_PRECHARGE,
        .currentSubstate                    = BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS_PRECHARGE_CLOSING_STRINGS,
        .information.transitionToErrorState = false,
    };
    OS_GetTickCount_ExpectAndReturn(1u);
    result = TEST_BMS_ProcessPrechargeState(&state);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SHORTTIME, state.timer);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_PRECHARGE, result);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_PRECHARGE_OPEN_PRECHARGE, state.currentSubstate);

    /* ======= RT18/22: Test implementation */
    state = (BMS_STATE_s){
        .timer                              = 0u,
        .currentState                       = BMS_FSM_STATE_PRECHARGE,
        .currentSubstate                    = BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS_PRECHARGE_CLOSING_STRINGS,
        .information.transitionToErrorState = true,
    };
    OS_GetTickCount_ExpectAndReturn(1u);
    result = TEST_BMS_ProcessPrechargeState(&state);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR, result);

    /* ======= RT19/22: Test implementation */
    state = (BMS_STATE_s){
        .timer                         = 0u,
        .currentState                  = BMS_FSM_STATE_PRECHARGE,
        .currentSubstate               = BMS_FSM_SUBSTATE_PRECHARGE_OPEN_PRECHARGE,
        .information.firstClosedString = 0u,
        .information.stringNumber      = 0u,
    };
    CONT_OpenPrecharge_ExpectAndReturn(0u, STD_OK);
    result = TEST_BMS_ProcessPrechargeState(&state);
    TEST_ASSERT_EQUAL_INT(BMS_TIME_WAIT_AFTER_OPENING_PRECHARGE, state.timer);
    TEST_ASSERT_EQUAL_INT(false, state.information.closedPrechargeContactors[0u]);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_PRECHARGE, result);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_PRECHARGE_CHECK_OPEN_PRECHARGE, state.currentSubstate);

    /* ======= RT20/22: Test implementation */
    state = (BMS_STATE_s){
        .timer                         = 0u,
        .currentState                  = BMS_FSM_STATE_PRECHARGE,
        .currentSubstate               = BMS_FSM_SUBSTATE_PRECHARGE_OPEN_PRECHARGE,
        .information.firstClosedString = 0u,
    };
    CONT_OpenPrecharge_ExpectAndReturn(0u, STD_NOT_OK);
    result = TEST_BMS_ProcessPrechargeState(&state);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR, result);

    /* ======= RT21/22: Test implementation */
    state = (BMS_STATE_s){
        .timer                         = 0u,
        .currentState                  = BMS_FSM_STATE_PRECHARGE,
        .currentSubstate               = BMS_FSM_SUBSTATE_PRECHARGE_CHECK_OPEN_PRECHARGE,
        .information.firstClosedString = 0u,
    };
    CONT_GetContactorState_ExpectAndReturn(0u, CONT_PRECHARGE, CONT_SWITCH_OFF);
    result = TEST_BMS_ProcessPrechargeState(&state);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_NORMAL, result);

    /* ======= RT22/22: Test implementation */
    state = (BMS_STATE_s){
        .timer                          = 0u,
        .currentState                   = BMS_FSM_STATE_PRECHARGE,
        .currentSubstate                = BMS_FSM_SUBSTATE_PRECHARGE_CHECK_OPEN_PRECHARGE,
        .information.firstClosedString  = 0u,
        .information.stringCloseTimeout = 1u,
    };
    CONT_GetContactorState_ExpectAndReturn(0u, CONT_PRECHARGE, CONT_SWITCH_ON);
    CONT_OpenPrecharge_ExpectAndReturn(0u, STD_OK);
    result = TEST_BMS_ProcessPrechargeState(&state);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SHORTTIME, state.timer);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_PRECHARGE, result);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS_PRECHARGE_FIRST_STRING, state.currentSubstate);
}

/**
 * @brief   Testing static function #BMS_RunStateMachine with currentState BMS_FSM_STATE_PRECHARGE
 * @details The following cases need to be tested:
 *          - Routine validation:
 *            - RT1/3: Stay in BMS_FSM_STATE_NORMAL
 *            - RT2/3: Proceed to BMS_FSM_STATE_OPEN_CONTACTORS_TO_STANDBY
 *            - RT3/3: Proceed to BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR
 *
 */
void testBMS_RunStateMachine_Normal(void) {
    /* ======= RT1/3: Test implementation */
    BMS_STATE_s state = {
        .timer           = 0u,
        .currentState    = BMS_FSM_STATE_NORMAL,
        .currentSubstate = BMS_FSM_SUBSTATE_ENTRY,
        .nextState       = BMS_FSM_STATE_NORMAL,
    };
    DATA_BLOCK_SYSTEM_STATE_s systemState = {.header.uniqueId = DATA_BLOCK_ID_SYSTEM_STATE};
    DATA_Read1DataBlock_ExpectAndReturn(&systemState, STD_OK);
    DATA_Write1DataBlock_ExpectAndReturn(&systemState, STD_OK);
    TEST_BMS_RunStateMachine(&state);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SHORTTIME, state.timer);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_NORMAL, state.currentState);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS, state.currentSubstate);

    /* ======= RT2/3: Test implementation */
    state = (BMS_STATE_s){
        .timer           = 0u,
        .currentState    = BMS_FSM_STATE_NORMAL,
        .currentSubstate = BMS_FSM_SUBSTATE_CHECK_STATE_REQUESTS,
    };
    DATA_BLOCK_STATE_REQUEST_s request = {.header.uniqueId = DATA_BLOCK_ID_STATE_REQUEST};
    request.stateRequestViaCan         = BMS_REQ_ID_STANDBY;
    DATA_Read1DataBlock_ExpectAndReturn(&request, STD_OK);
    DATA_Read1DataBlock_ReturnThruPtr_pDataToReceiver0(&request);
    TEST_BMS_RunStateMachine(&state);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SHORTTIME, state.timer);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_OPEN_CONTACTORS_TO_STANDBY, state.currentState);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_ENTRY, state.currentSubstate);

    /* ======= RT3/3: Test implementation */
    state = (BMS_STATE_s){
        .timer                              = 0u,
        .currentState                       = BMS_FSM_STATE_NORMAL,
        .currentSubstate                    = BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS,
        .information.transitionToErrorState = true,
    };
    OS_GetTickCount_ExpectAndReturn(1u);
    TEST_BMS_RunStateMachine(&state);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SHORTTIME, state.timer);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR, state.currentState);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_ENTRY, state.currentSubstate);
}

/**
 * @brief   Testing static function #BMS_ProcessNormalState
 * @details The following cases need to be tested:
 *          - Routine validation:
 *            - RT1/8: Call function in BMS_FSM_SUBSTATE_ENTRY and nextState is normal - proceed to CHECK_ERROR_FLAGS
 *            - RT2/8: Call function in BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS and battery system currentState is not okay - proceed to ERROR
 *            - RT3/8: Call function in BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS and battery system currentState is okay - proceed to CHECK_STATE_REQUESTS
 *            - RT4/8: Call function in BMS_FSM_SUBSTATE_CHECK_STATE_REQUESTS and req standby - proceed to OPEN_CONTACTORS
 *            - RT5/8: Call function in BMS_FSM_SUBSTATE_CHECK_STATE_REQUESTS and req no request - proceed to NORMAL_CLOSE_NEXT_STRING
 *            - RT6/8: Call function in BMS_FSM_SUBSTATE_NORMAL_CLOSE_SECOND_STRING_CONTACTOR and cont state is on - proceed to BMS_FSM_SUBSTATE_NORMAL_CLOSE_SECOND_STRING_CONTACTOR
 *            - RT7/8: Call function in BMS_FSM_SUBSTATE_NORMAL_CLOSE_SECOND_STRING_CONTACTOR and cont state is off - proceed to BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR
 *            - RT8/8: Call function in BMS_FSM_SUBSTATE_NORMAL_CLOSE_SECOND_STRING_CONTACTOR and cont state is off, timeout != 0 - proceed to BMS_FSM_SUBSTATE_NORMAL_CLOSE_SECOND_STRING_CONTACTOR
 */
void testBMS_ProcessNormalState(void) {
    /* ======= Routine tests =============================================== */
    /* ======= RT1/8: Test implementation */
    BMS_STATE_s state = {
        .timer           = 0u,
        .currentState    = BMS_FSM_STATE_NORMAL,
        .currentSubstate = BMS_FSM_SUBSTATE_ENTRY,
        .nextState       = BMS_FSM_STATE_NORMAL,
    };
    DATA_BLOCK_SYSTEM_STATE_s systemState = {.header.uniqueId = DATA_BLOCK_ID_SYSTEM_STATE};
    DATA_Read1DataBlock_ExpectAndReturn(&systemState, STD_OK);
    DATA_Write1DataBlock_ExpectAndReturn(&systemState, STD_OK);
    BMS_FSM_STATES_e result = TEST_BMS_ProcessNormalState(&state);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SHORTTIME, state.timer);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_NORMAL, result);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS, state.currentSubstate);

    /* ======= RT2/8: Test implementation */
    state = (BMS_STATE_s){
        .timer                              = 0u,
        .currentState                       = BMS_FSM_STATE_NORMAL,
        .currentSubstate                    = BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS,
        .information.transitionToErrorState = true,
    };
    OS_GetTickCount_ExpectAndReturn(1u);
    result = TEST_BMS_ProcessNormalState(&state);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR, result);

    /* ======= RT3/8: Test implementation */
    state = (BMS_STATE_s){
        .timer                              = 0u,
        .currentState                       = BMS_FSM_STATE_NORMAL,
        .currentSubstate                    = BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS,
        .information.transitionToErrorState = false,
    };
    OS_GetTickCount_ExpectAndReturn(1u);
    result = TEST_BMS_ProcessNormalState(&state);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SHORTTIME, state.timer);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_NORMAL, result);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_CHECK_STATE_REQUESTS, state.currentSubstate);

    /* ======= RT4/8: Test implementation */
    state = (BMS_STATE_s){
        .timer           = 0u,
        .currentState    = BMS_FSM_STATE_NORMAL,
        .currentSubstate = BMS_FSM_SUBSTATE_CHECK_STATE_REQUESTS,
    };
    DATA_BLOCK_STATE_REQUEST_s request = {.header.uniqueId = DATA_BLOCK_ID_STATE_REQUEST};
    request.stateRequestViaCan         = BMS_REQ_ID_STANDBY;
    DATA_Read1DataBlock_ExpectAndReturn(&request, STD_OK);
    DATA_Read1DataBlock_ReturnThruPtr_pDataToReceiver0(&request);
    result = TEST_BMS_ProcessNormalState(&state);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_OPEN_CONTACTORS_TO_STANDBY, result);

    /* ======= RT5/8: Test implementation */
    state = (BMS_STATE_s){
        .timer           = 0u,
        .currentState    = BMS_FSM_STATE_NORMAL,
        .currentSubstate = BMS_FSM_SUBSTATE_CHECK_STATE_REQUESTS,
    };
    request.stateRequestViaCan = BMS_REQ_ID_NOREQ;
    DATA_Read1DataBlock_ExpectAndReturn(&request, STD_OK);
    DATA_Read1DataBlock_ReturnThruPtr_pDataToReceiver0(&request);
    result = TEST_BMS_ProcessNormalState(&state);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SHORTTIME, state.timer);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_NORMAL, result);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_NORMAL_CLOSE_NEXT_STRING, state.currentSubstate);

    /* ======= RT6/8: Test implementation */
    state = (BMS_STATE_s){
        .timer                        = 0u,
        .currentState                 = BMS_FSM_STATE_NORMAL,
        .currentSubstate              = BMS_FSM_SUBSTATE_NORMAL_CLOSE_SECOND_STRING_CONTACTOR,
        .information.nextStringNumber = 0u,
    };
    CONT_GetContactorState_ExpectAndReturn(0u, CONT_MINUS, CONT_SWITCH_ON);
    CONT_CloseContactor_ExpectAndReturn(0u, CONT_PLUS, STD_OK);

    result = TEST_BMS_ProcessNormalState(&state);

    TEST_ASSERT_EQUAL_INT(BMS_WAIT_TIME_AFTER_CLOSING_STRING_CONTACTOR, state.timer);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_NORMAL, result);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_NORMAL_CHECK_STRING_CLOSED, state.currentSubstate);

    /* ======= RT7/8: Test implementation */
    state = (BMS_STATE_s){
        .timer                          = 0u,
        .currentState                   = BMS_FSM_STATE_NORMAL,
        .currentSubstate                = BMS_FSM_SUBSTATE_NORMAL_CLOSE_SECOND_STRING_CONTACTOR,
        .information.nextStringNumber   = 0u,
        .information.stringCloseTimeout = 0u,
    };
    CONT_GetContactorState_ExpectAndReturn(0u, CONT_MINUS, CONT_SWITCH_OFF);

    result = TEST_BMS_ProcessNormalState(&state);

    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_OPEN_CONTACTORS_TO_ERROR, result);

    /* ======= RT8/8: Test implementation */
    state = (BMS_STATE_s){
        .timer                          = 0u,
        .currentState                   = BMS_FSM_STATE_NORMAL,
        .currentSubstate                = BMS_FSM_SUBSTATE_NORMAL_CLOSE_SECOND_STRING_CONTACTOR,
        .information.nextStringNumber   = 0u,
        .information.stringCloseTimeout = 1u,
    };
    CONT_GetContactorState_ExpectAndReturn(0u, CONT_MINUS, CONT_SWITCH_OFF);
    CONT_CloseContactor_ExpectAndReturn(0u, CONT_MINUS, STD_OK);

    result = TEST_BMS_ProcessNormalState(&state);

    TEST_ASSERT_EQUAL_INT(BMS_FSM_SHORTTIME, state.timer);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_NORMAL, result);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_NORMAL_CLOSE_SECOND_STRING_CONTACTOR, state.currentSubstate);
}

/**
 * @brief   Testing static function #BMS_RunStateMachine with currentState BMS_FSM_STATE_PRECHARGE
 * @details The following cases need to be tested:
 *          - Routine validation:
 *            - RT1/2: Stay in BMS_FSM_STATE_ERROR
 *            - RT2/2: Proceed to BMS_FSM_STATE_OPEN_CONTACTORS_TO_STANDBY
 *
 */
void testBMS_RunStateMachine_Error(void) {
    /* ======= RT1/2: Test implementation */
    BMS_STATE_s state = {.timer = 0u, .currentState = BMS_FSM_STATE_ERROR, .currentSubstate = BMS_FSM_SUBSTATE_ENTRY};
    DATA_BLOCK_SYSTEM_STATE_s systemState = {.header.uniqueId = DATA_BLOCK_ID_SYSTEM_STATE};
    DATA_Read1DataBlock_ExpectAndReturn(&systemState, STD_OK);
    DATA_Write1DataBlock_ExpectAndReturn(&systemState, STD_OK);
    BAL_SetStateRequest_ExpectAndReturn(BAL_STATE_NO_BALANCING_REQUEST, BAL_OK);
    LED_SetToggleTime_Expect(LED_ERROR_OPERATION_ON_OFF_TIME_ms);
    TEST_BMS_RunStateMachine(&state);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SHORTTIME, state.timer);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_ERROR, state.currentState);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS, state.currentSubstate);

    /* ======= RT2/2: Test implementation */
    state = (BMS_STATE_s){
        .timer           = 0u,
        .currentState    = BMS_FSM_STATE_ERROR,
        .currentSubstate = BMS_FSM_SUBSTATE_CHECK_STATE_REQUESTS,
    };
    DATA_BLOCK_STATE_REQUEST_s request = {.header.uniqueId = DATA_BLOCK_ID_STATE_REQUEST};
    request.stateRequestViaCan         = BMS_REQ_ID_STANDBY;
    DATA_Read1DataBlock_ExpectAndReturn(&request, STD_OK);
    DATA_Read1DataBlock_ReturnThruPtr_pDataToReceiver0(&request);
    BAL_SetStateRequest_ExpectAndReturn(BAL_STATE_ALLOW_BALANCING_REQUEST, BAL_OK);
    LED_SetToggleTime_Expect(LED_NORMAL_OPERATION_ON_OFF_TIME_ms);
    TEST_BMS_RunStateMachine(&state);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SHORTTIME, state.timer);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_OPEN_CONTACTORS_TO_STANDBY, state.currentState);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_ENTRY, state.currentSubstate);
}

/**
 * @brief   Testing static function #BMS_ProcessErrorState
 * @details The following cases need to be tested:
 *          - Routine validation:
 *            - RT1/4: Call function in BMS_FSM_SUBSTATE_ENTRY - proceed to CHECK_ERROR_FLAGS
 *            - RT2/4: Call function in BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS and no fatal error active - proceed to CHECK_STATE_REQUESTS
 *            - RT3/4: Call function in BMS_FSM_SUBSTATE_CHECK_STATE_REQUESTS and req standby - proceed to OPEN_CONTACTORS
 *            - RT4/4: Call function in BMS_FSM_SUBSTATE_CHECK_STATE_REQUESTS and req no request - proceed to CHECK_ERROR_FLAGS
 */
void testBMS_ProcessErrorState(void) {
    /* ======= Routine tests =============================================== */
    /* ======= RT1/4: Test implementation */
    BMS_STATE_s state = {.timer = 0u, .currentState = BMS_FSM_STATE_ERROR, .currentSubstate = BMS_FSM_SUBSTATE_ENTRY};
    DATA_BLOCK_SYSTEM_STATE_s systemState = {.header.uniqueId = DATA_BLOCK_ID_SYSTEM_STATE};
    DATA_Read1DataBlock_ExpectAndReturn(&systemState, STD_OK);
    DATA_Write1DataBlock_ExpectAndReturn(&systemState, STD_OK);
    BAL_SetStateRequest_ExpectAndReturn(BAL_STATE_NO_BALANCING_REQUEST, BAL_OK);
    LED_SetToggleTime_Expect(LED_ERROR_OPERATION_ON_OFF_TIME_ms);
    BMS_FSM_STATES_e result = TEST_BMS_ProcessErrorState(&state);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SHORTTIME, state.timer);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_ERROR, result);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS, state.currentSubstate);

    /* ======= RT2/4: Test implementation */
    state = (BMS_STATE_s){
        .timer                              = 0u,
        .currentState                       = BMS_FSM_STATE_ERROR,
        .currentSubstate                    = BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS,
        .information.minimumActiveDelay_ms  = 1u,
        .information.transitionToErrorState = true,
    };
    DIAG_IsAnyFatalErrorSet_ExpectAndReturn(false);
    result = TEST_BMS_ProcessErrorState(&state);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SHORTTIME, state.timer);
    TEST_ASSERT_EQUAL_INT(UINT32_MAX, state.information.minimumActiveDelay_ms);
    TEST_ASSERT_EQUAL_INT(false, state.information.transitionToErrorState);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_ERROR, result);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_CHECK_STATE_REQUESTS, state.currentSubstate);

    /* ======= RT3/4: Test implementation */
    state = (BMS_STATE_s){
        .timer           = 0u,
        .currentState    = BMS_FSM_STATE_ERROR,
        .currentSubstate = BMS_FSM_SUBSTATE_CHECK_STATE_REQUESTS,
    };
    DATA_BLOCK_STATE_REQUEST_s request = {.header.uniqueId = DATA_BLOCK_ID_STATE_REQUEST};
    request.stateRequestViaCan         = BMS_REQ_ID_STANDBY;
    DATA_Read1DataBlock_ExpectAndReturn(&request, STD_OK);
    DATA_Read1DataBlock_ReturnThruPtr_pDataToReceiver0(&request);
    BAL_SetStateRequest_ExpectAndReturn(BAL_STATE_ALLOW_BALANCING_REQUEST, BAL_OK);
    LED_SetToggleTime_Expect(LED_NORMAL_OPERATION_ON_OFF_TIME_ms);
    result = TEST_BMS_ProcessErrorState(&state);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_OPEN_CONTACTORS_TO_STANDBY, result);

    /* ======= RT4/4: Test implementation */
    state = (BMS_STATE_s){
        .timer           = 0u,
        .currentState    = BMS_FSM_STATE_ERROR,
        .currentSubstate = BMS_FSM_SUBSTATE_CHECK_STATE_REQUESTS,
    };
    request.stateRequestViaCan = BMS_REQ_ID_NOREQ;
    DATA_Read1DataBlock_ExpectAndReturn(&request, STD_OK);
    DATA_Read1DataBlock_ReturnThruPtr_pDataToReceiver0(&request);
    result = TEST_BMS_ProcessErrorState(&state);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SHORTTIME, state.timer);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_STATE_ERROR, result);
    TEST_ASSERT_EQUAL_INT(BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS, state.currentSubstate);
}
