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
 * @file    test_debug_default.c
 * @author  foxBMS Team
 * @date    2020-09-17 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Test of the afe.c module
 * @details TODO
 *
 */

/*========== Includes =======================================================*/

#include "unity.h"
#include "Mockdatabase.h"
#include "Mockos.h"

#include "battery_cell_cfg.h"

#include "afe.h"
#include "debug_default.h"
#include "test_assert_helper.h"

#include <stdbool.h>
#include <stdint.h>

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("debug_default.c")

TEST_INCLUDE_PATH("../../src/app/driver/afe/api")
TEST_INCLUDE_PATH("../../src/app/driver/afe/debug/default")
TEST_INCLUDE_PATH("../../src/app/engine/diag")

/*========== Definitions and Implementations for Unit Test ==================*/
#define FAKE_CELL_VOLTAGE_mV (BC_VOLTAGE_NOMINAL_mV)

#define FAKE_CELL_TEMPERATURE_ddegC ((BC_TEMPERATURE_MAX_CHARGE_MOL_ddegC + BC_TEMPERATURE_MIN_CHARGE_MOL_ddegC) / 2u)

static void TEST_assertEqualFakeState(FAKE_STATE_s *pExpected, FAKE_STATE_s *pActual);

/** assert that the values and values behind the pointers in #FAKE_STATE_s are the same */
static void TEST_assertEqualFakeState(FAKE_STATE_s *pExpected, FAKE_STATE_s *pActual) {
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(
        88,
        sizeof(*pExpected),
        "The data structure changed, update tests: "
        "1: Add tests for the added/changed data structure. "
        "2: Update the data structure length.");

    /* compare memory contents of database entries */
    TEST_ASSERT_EQUAL_MEMORY(
        pExpected->data.cellVoltage, pActual->data.cellVoltage, sizeof(*pExpected->data.cellVoltage));
    TEST_ASSERT_EQUAL_MEMORY(
        pExpected->data.cellTemperature, pActual->data.cellTemperature, sizeof(*pExpected->data.cellTemperature));
    TEST_ASSERT_EQUAL_MEMORY(
        pExpected->data.balancingFeedback, pActual->data.balancingFeedback, sizeof(*pExpected->data.balancingFeedback));
    TEST_ASSERT_EQUAL_MEMORY(
        pExpected->data.balancingControl, pActual->data.balancingControl, sizeof(*pExpected->data.balancingControl));
    TEST_ASSERT_EQUAL_MEMORY(
        pExpected->data.slaveControl, pActual->data.slaveControl, sizeof(*pExpected->data.slaveControl));
    TEST_ASSERT_EQUAL_MEMORY(
        pExpected->data.allGpioVoltages, pActual->data.allGpioVoltages, sizeof(*pExpected->data.allGpioVoltages));
    TEST_ASSERT_EQUAL_MEMORY(pExpected->data.openWire, pActual->data.openWire, sizeof(*pExpected->data.openWire));

    TEST_ASSERT_EQUAL_UINT16(pExpected->timer, pActual->timer);
    TEST_ASSERT_EQUAL_UINT8(pExpected->triggerEntry, pActual->triggerEntry);
    TEST_ASSERT_EQUAL_UINT8(pExpected->currentState, pActual->currentState);
    TEST_ASSERT_EQUAL_UINT8(pExpected->previousState, pActual->previousState);
    TEST_ASSERT_EQUAL_UINT8(pExpected->nextSubstate, pActual->nextSubstate);
    TEST_ASSERT_EQUAL_UINT8(pExpected->currentSubstate, pActual->currentSubstate);
    TEST_ASSERT_EQUAL_UINT8(pExpected->previousSubstate, pActual->previousSubstate);
    TEST_ASSERT_EQUAL_UINT8(pExpected->firstMeasurementFinished, pActual->firstMeasurementFinished);
}

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/

void testTEST_FAKE_CheckMultipleCalls(void) {
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();

    FAKE_STATE_s state = {0};
    bool reentrance    = 0;
    reentrance         = TEST_FAKE_CheckMultipleCalls(&state);
    TEST_ASSERT_EQUAL_UINT8(false, reentrance);

    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    state.triggerEntry = 1;
    reentrance         = TEST_FAKE_CheckMultipleCalls(&state);
    TEST_ASSERT_EQUAL_UINT8(true, reentrance);
}

/**
 * @brief   Testing extern function #FAKE_SetState
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: invalid pFakeState; assert
 *          - Routine validation:
 *            - RT1/x: TODO
 */
void testFAKE_SetState(void) {
    /* ======= Assertion tests ============================================= */
    FAKE_FSM_STATES_e nextState       = FAKE_FSM_STATE_HAS_NEVER_RUN;
    FAKE_FSM_SUBSTATES_e nextSubstate = FAKE_FSM_SUBSTATE_DUMMY;
    uint8_t idleTime                  = 2u;
    TEST_ASSERT_FAIL_ASSERT(TEST_FAKE_SetState(NULL_PTR, nextState, nextSubstate, idleTime));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/3: Test implementation */
    FAKE_STATE_s state = {0};
    TEST_FAKE_SetState(&state, nextState, nextSubstate, idleTime);

    /* ======= RT2/3: Test implementation */
    state.currentState    = FAKE_FSM_STATE_HAS_NEVER_RUN;
    state.currentSubstate = FAKE_FSM_SUBSTATE_DUMMY;
    TEST_FAKE_SetState(&state, nextState, nextSubstate, idleTime);

    /* ======= RT3/3: Test implementation */
    state.currentState    = FAKE_FSM_STATE_HAS_NEVER_RUN;
    state.currentSubstate = FAKE_FSM_SUBSTATE_ENTRY;
    TEST_FAKE_SetState(&state, nextState, nextSubstate, idleTime);
}

void testFAKE_SetFirstMeasurementCycleFinished(void) {
    static DATA_BLOCK_CELL_VOLTAGE_s test_fake_cellVoltage = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE_BASE};
    static DATA_BLOCK_CELL_TEMPERATURE_s test_fake_cellTemperature = {
        .header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE_BASE};
    static DATA_BLOCK_BALANCING_FEEDBACK_s test_fake_balancingFeedback = {
        .header.uniqueId = DATA_BLOCK_ID_BALANCING_FEEDBACK_BASE};
    static DATA_BLOCK_BALANCING_CONTROL_s test_fake_balancingControl = {
        .header.uniqueId = DATA_BLOCK_ID_BALANCING_CONTROL};
    static DATA_BLOCK_SLAVE_CONTROL_s test_fake_slaveControl       = {.header.uniqueId = DATA_BLOCK_ID_SLAVE_CONTROL};
    static DATA_BLOCK_ALL_GPIO_VOLTAGES_s test_fake_allGpioVoltage = {
        .header.uniqueId = DATA_BLOCK_ID_ALL_GPIO_VOLTAGES_BASE};
    static DATA_BLOCK_OPEN_WIRE_s test_fake_openWire = {.header.uniqueId = DATA_BLOCK_ID_OPEN_WIRE_BASE};
    FAKE_STATE_s test_fake_state                     = {
                            .timer                    = 0,
                            .firstMeasurementFinished = false,
                            .triggerEntry             = 0,
                            .nextState                = FAKE_FSM_STATE_HAS_NEVER_RUN,
                            .currentState             = FAKE_FSM_STATE_HAS_NEVER_RUN,
                            .previousState            = FAKE_FSM_STATE_HAS_NEVER_RUN,
                            .nextSubstate             = FAKE_FSM_SUBSTATE_DUMMY,
                            .currentSubstate          = FAKE_FSM_SUBSTATE_DUMMY,
                            .previousSubstate         = FAKE_FSM_SUBSTATE_DUMMY,
                            .data.allGpioVoltages     = &test_fake_allGpioVoltage,
                            .data.balancingControl    = &test_fake_balancingControl,
                            .data.balancingFeedback   = &test_fake_balancingFeedback,
                            .data.cellTemperature     = &test_fake_cellTemperature,
                            .data.cellVoltage         = &test_fake_cellVoltage,
                            .data.openWire            = &test_fake_openWire,
                            .data.slaveControl        = &test_fake_slaveControl,
    };

    static DATA_BLOCK_CELL_VOLTAGE_s test_fake_cellVoltageCompare = {
        .header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE_BASE};
    static DATA_BLOCK_CELL_TEMPERATURE_s test_fake_cellTemperatureCompare = {
        .header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE_BASE};
    static DATA_BLOCK_BALANCING_FEEDBACK_s test_fake_balancingFeedbackCompare = {
        .header.uniqueId = DATA_BLOCK_ID_BALANCING_FEEDBACK_BASE};
    static DATA_BLOCK_BALANCING_CONTROL_s test_fake_balancingControlCompare = {
        .header.uniqueId = DATA_BLOCK_ID_BALANCING_CONTROL};
    static DATA_BLOCK_SLAVE_CONTROL_s test_fake_slaveControlCompare = {.header.uniqueId = DATA_BLOCK_ID_SLAVE_CONTROL};
    static DATA_BLOCK_ALL_GPIO_VOLTAGES_s test_fake_allGpioVoltageCompare = {
        .header.uniqueId = DATA_BLOCK_ID_ALL_GPIO_VOLTAGES_BASE};
    static DATA_BLOCK_OPEN_WIRE_s test_fake_openWireCompare = {.header.uniqueId = DATA_BLOCK_ID_OPEN_WIRE_BASE};

    FAKE_STATE_s test_fake_stateCompare = {
        .timer                    = 0,
        .firstMeasurementFinished = true,
        .triggerEntry             = 0,
        .nextState                = FAKE_FSM_STATE_HAS_NEVER_RUN,
        .currentState             = FAKE_FSM_STATE_HAS_NEVER_RUN,
        .previousState            = FAKE_FSM_STATE_HAS_NEVER_RUN,
        .nextSubstate             = FAKE_FSM_SUBSTATE_DUMMY,
        .currentSubstate          = FAKE_FSM_SUBSTATE_DUMMY,
        .previousSubstate         = FAKE_FSM_SUBSTATE_DUMMY,
        .data.allGpioVoltages     = &test_fake_allGpioVoltageCompare,
        .data.balancingControl    = &test_fake_balancingControlCompare,
        .data.balancingFeedback   = &test_fake_balancingFeedbackCompare,
        .data.cellTemperature     = &test_fake_cellTemperatureCompare,
        .data.cellVoltage         = &test_fake_cellVoltageCompare,
        .data.openWire            = &test_fake_openWireCompare,
        .data.slaveControl        = &test_fake_slaveControlCompare,
    };

    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    DATA_Write4DataBlocks_ExpectAndReturn(
        test_fake_stateCompare.data.cellVoltage,
        test_fake_stateCompare.data.cellTemperature,
        test_fake_stateCompare.data.balancingFeedback,
        test_fake_stateCompare.data.balancingControl,
        STD_OK);
    DATA_Write2DataBlocks_ExpectAndReturn(
        test_fake_stateCompare.data.slaveControl, test_fake_stateCompare.data.openWire, STD_OK);

    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t cb = 0u; cb < BS_NR_OF_CELL_BLOCKS_PER_MODULE; cb++) {
                test_fake_stateCompare.data.cellVoltage->cellVoltage_mV[s][m][cb] = FAKE_CELL_VOLTAGE_mV;
            }
        }
        test_fake_stateCompare.data.cellVoltage->stringVoltage_mV[s] = FAKE_CELL_VOLTAGE_mV *
                                                                       BS_NR_OF_CELL_BLOCKS_PER_STRING;

        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t ts = 0; ts < BS_NR_OF_TEMP_SENSORS_PER_MODULE; ts++) {
                test_fake_stateCompare.data.cellTemperature->cellTemperature_ddegC[s][m][ts] =
                    FAKE_CELL_TEMPERATURE_ddegC;
            }
        }

        test_fake_stateCompare.data.slaveControl->eepromReadAddressLastUsed  = 0xFFFFFFFF;
        test_fake_stateCompare.data.slaveControl->eepromReadAddressToUse     = 0xFFFFFFFF;
        test_fake_stateCompare.data.slaveControl->eepromWriteAddressLastUsed = 0xFFFFFFFF;
        test_fake_stateCompare.data.slaveControl->eepromWriteAddressToUse    = 0xFFFFFFFF;
    }
    test_fake_stateCompare.firstMeasurementFinished = true;

    test_fake_state.firstMeasurementFinished = false;
    TEST_FAKE_SetFirstMeasurementCycleFinished(&test_fake_state);

    TEST_ASSERT_EQUAL_UINT8(true, test_fake_state.firstMeasurementFinished);
    TEST_assertEqualFakeState(&test_fake_stateCompare, &test_fake_state);
}

void testFAKE_SaveFakeVoltageMeasurementData(void) {
    /* ======= Assertion tests ============================================= */
    TEST_ASSERT_FAIL_ASSERT(TEST_FAKE_SaveFakeVoltageMeasurementData(NULL_PTR));

    /* ======= Routine tests =============================================== */
    static DATA_BLOCK_CELL_VOLTAGE_s test_fake_cellVoltage = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE_BASE};
    FAKE_STATE_s test_fake_state                           = {
                                  .data.cellVoltage = &test_fake_cellVoltage,
    };
    /* ======= RT1/1: Test implementation */
    DATA_Write1DataBlock_ExpectAndReturn(test_fake_state.data.cellVoltage, STD_OK);
    TEST_FAKE_SaveFakeVoltageMeasurementData(&test_fake_state);
}

void testFAKE_SaveFakeTemperatureMeasurementData(void) {
    /* ======= Assertion tests ============================================= */
    TEST_ASSERT_FAIL_ASSERT(TEST_FAKE_SaveFakeTemperatureMeasurementData(NULL_PTR));

    /* ======= Routine tests =============================================== */
    static DATA_BLOCK_CELL_TEMPERATURE_s test_fake_cellTemperature = {
        .header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE_BASE};
    FAKE_STATE_s test_fake_state = {
        .data.cellTemperature = &test_fake_cellTemperature,
    };
    /* ======= RT1/1: Test implementation */
    DATA_Write1DataBlock_ExpectAndReturn(test_fake_state.data.cellTemperature, STD_OK);
    TEST_FAKE_SaveFakeTemperatureMeasurementData(&test_fake_state);
}

/**
 * @brief   Testing extern function #FAKE_ProcessInitializationState
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: invalid pFakeState; assert
 *          - Routine validation:
 *            - RT1/5: default case
 *            - RT2/5: case FAKE_FSM_SUBSTATE_INITIALIZATION_EXIT
 *            - RT3/5: case FAKE_FSM_SUBSTATE_INITIALIZATION_FIRST_MEASUREMENT_FINISHED
 *            - RT4/5: case FAKE_FSM_SUBSTATE_INITIALIZATION_FINISH_FIRST_MEASUREMENT
 *            - RT5/5: case FAKE_FSM_SUBSTATE_ENTRY
 */
void testFAKE_ProcessInitializationState(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= Routine tests =============================================== */
    FAKE_STATE_s state = {0};
    /* ======= RT1/5: Test implementation */
    TEST_FAKE_ProcessInitializationState(&state);

    /* ======= RT2/5: Test implementation */
    state.firstMeasurementFinished = true;
    state.currentSubstate          = FAKE_FSM_SUBSTATE_INITIALIZATION_EXIT;
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    TEST_FAKE_ProcessInitializationState(&state);

    /* ======= RT3/5: Test implementation */
    state.currentSubstate = FAKE_FSM_SUBSTATE_INITIALIZATION_FIRST_MEASUREMENT_FINISHED;
    TEST_FAKE_ProcessInitializationState(&state);

    /* ======= RT4/5: Test implementation */
    state.currentSubstate = FAKE_FSM_SUBSTATE_INITIALIZATION_FINISH_FIRST_MEASUREMENT;
    TEST_FAKE_ProcessInitializationState(&state);

    /* ======= RT5/5: Test implementation */
    state.currentSubstate = FAKE_FSM_SUBSTATE_ENTRY;
    TEST_FAKE_ProcessInitializationState(&state);
}

/**
 * @brief   Testing extern function #FAKE_ProcessRunningState
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: invalid pFakeState; assert
 *          - Routine validation:
 *            - RT1/4: default case
 *            - RT2/4: case FAKE_FSM_SUBSTATE_ENTRY
 */
void testFAKE_ProcessRunningState(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= Routine tests =============================================== */
    FAKE_STATE_s state = {0};
    /* ======= RT1/2: Test implementation */
    TEST_FAKE_ProcessRunningState(&state);
    /* ======= RT2/2: Test implementation */
    state.currentSubstate = FAKE_FSM_SUBSTATE_ENTRY;
    TEST_FAKE_ProcessRunningState(&state);
}

/**
 * @brief   Testing extern function #FAKE_RunStateMachine
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: invalid pFakeState; assert
 *          - Routine validation:
 *            - RT1/4: default case
 *            - RT2/4: case FAKE_FSM_SUBSTATE_ENTRY
 */
void testFAKE_RunStateMachine(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= Routine tests =============================================== */
    FAKE_STATE_s state = {0};
    /* ======= RT1/6: Test implementation */
    TEST_FAKE_RunStateMachine(&state);
    /* ======= RT2/6: Test implementation */
    state.currentState = FAKE_FSM_STATE_HAS_NEVER_RUN;
    TEST_FAKE_RunStateMachine(&state);
    /* ======= RT3/6: Test implementation */
    state.currentState = FAKE_FSM_STATE_UNINITIALIZED;
    TEST_FAKE_RunStateMachine(&state);
    /* ======= RT4/6: Test implementation */
    state.currentState = FAKE_FSM_STATE_INITIALIZATION;
    TEST_FAKE_RunStateMachine(&state);
    /* ======= RT5/6: Test implementation */
    state.currentState = FAKE_FSM_STATE_RUNNING;
    TEST_FAKE_RunStateMachine(&state);
    /* ======= RT6/6: Test implementation */
    state.currentState = FAKE_FSM_STATE_ERROR;
    TEST_FAKE_RunStateMachine(&state);
}

void testFAKE_Initialize(void) {
    TEST_ASSERT_EQUAL(STD_OK, FAKE_Initialize());
}

void testFAKE_IsFirstMeasurementCycleFinished(void) {
    /* ======= Assertion tests ============================================= */
    TEST_ASSERT_FAIL_ASSERT(FAKE_IsFirstMeasurementCycleFinished(NULL_PTR));

    /* ======= Routine tests =============================================== */
    FAKE_STATE_s state = {0};
    /* ======= RT1/1: Test implementation */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    FAKE_IsFirstMeasurementCycleFinished(&state);
}

void testFAKE_TriggerAfe(void) {
    /* ======= Assertion tests ============================================= */
    TEST_ASSERT_FAIL_ASSERT(FAKE_TriggerAfe(NULL_PTR));

    /* ======= Routine tests =============================================== */
    FAKE_STATE_s state = {0};
    /* ======= RT1/4: Test implementation */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    state.triggerEntry = 1u;
    FAKE_TriggerAfe(&state);
    state.triggerEntry = 0u;
    /* ======= RT2/4: Test implementation */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    FAKE_TriggerAfe(&state);
    /* ======= RT3/4: Test implementation */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    state.timer = 1u;
    FAKE_TriggerAfe(&state);
    /* ======= RT4/4: Test implementation */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    state.timer = 2u;
    FAKE_TriggerAfe(&state);
}
