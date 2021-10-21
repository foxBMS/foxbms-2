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
 * @file    test_debug_default.c
 * @author  foxBMS Team
 * @date    2020-09-17 (date of creation)
 * @updated 2021-06-09 (date of last update)
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Test of the afe.c module
 *
 */

/*========== Includes =======================================================*/
#include "general.h"

#include "unity.h"
#include "Mockdatabase.h"
#include "Mockos.h"

#include "battery_cell_cfg.h"
#include "debug_default_cfg.h"

#include "afe.h"
#include "debug_default.h"

/* it's important to mention the implementation in debug_default.c
here in order to test the correct implementation */
TEST_FILE("debug_default.c")

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

void testFAKE_SetFirstMeasurementCycleFinished(void) {
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    DATA_Write_2_DataBlocks_IgnoreAndReturn(STD_OK);
    DATA_Write_4_DataBlocks_IgnoreAndReturn(STD_OK);

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

    uint16_t i = 0;

    for (uint8_t stringNumber = 0u; stringNumber < BS_NR_OF_STRINGS; stringNumber++) {
        for (i = 0; i < BS_NR_OF_BAT_CELLS; i++) {
            test_fake_stateCompare.data.cellVoltage->cellVoltage_mV[stringNumber][i] = FAKE_CELL_VOLTAGE_mV;
        }
        test_fake_stateCompare.data.cellVoltage->packVoltage_mV[stringNumber] = FAKE_CELL_VOLTAGE_mV *
                                                                                BS_NR_OF_BAT_CELLS;

        for (i = 0; i < BS_NR_OF_TEMP_SENSORS_PER_STRING; i++) {
            test_fake_stateCompare.data.cellTemperature->cellTemperature_ddegC[stringNumber][i] =
                FAKE_CELL_TEMPERATURE_ddegC;
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

void testFAKE_Initialize(void) {
    TEST_ASSERT_EQUAL(STD_OK, FAKE_Initialize());
}

void testFAKE_IsFirstMeasurementCycleFinished(void) {
}

void testFAKE_TriggerAfe(void) {
}
