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
 * @file    test_interlock.c
 * @author  foxBMS Team
 * @date    2020-04-01 (date of creation)
 * @updated 2021-10-18 (date of last update)
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Tests for the interlock module
 *
 */

/*========== Includes =======================================================*/
#include "general.h"

#include "unity.h"
#include "MockHL_het.h"
#include "Mockdatabase.h"
#include "Mockdiag.h"
#include "Mockfassert.h"
#include "Mockio.h"
#include "Mockos.h"

#include "interlock_cfg.h"

#include "interlock.h"

/*========== Definitions and Implementations for Unit Test ==================*/

/*========== Setup and Teardown =============================================*/
void setUp(void) {
    /* reset the state of interlock before each test */
    static ILCK_STATE_s ilck_state = {
        .timer             = 0,
        .statereq          = ILCK_STATE_NO_REQUEST,
        .state             = ILCK_STATEMACHINE_UNINITIALIZED,
        .substate          = ILCK_ENTRY,
        .laststate         = ILCK_STATEMACHINE_UNINITIALIZED,
        .lastsubstate      = ILCK_ENTRY,
        .triggerentry      = 0,
        .ErrRequestCounter = 0,
        .counter           = 0,
    };
    TEST_ILCK_SetStateStruct(ilck_state);
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
void testILCK_GetState(void) {
    /* checks whether GetState returns the state (should be uninitialized when
       first called) */
    TEST_ASSERT_EQUAL(ILCK_STATEMACHINE_UNINITIALIZED, ILCK_GetState());
}

void testILCK_SetStateRequestLegalValuesILCK_STATE_INIT_REQUEST(void) {
    /* test legal value ILCK_STATE_INIT_REQUEST for the state-request */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    TEST_ASSERT_EQUAL(ILCK_OK, ILCK_SetStateRequest(ILCK_STATE_INITIALIZATION_REQUEST));
}

void testILCK_SetStateRequestLegalValuesILCK_STATE_NO_REQUEST(void) {
    /* test legal value ILCK_STATE_NO_REQUEST for the state-request */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    /* even though this value is legal, it will return illegal request */
    TEST_ASSERT_EQUAL(ILCK_ILLEGAL_REQUEST, ILCK_SetStateRequest(INT8_MAX));
}

void testILCK_SetStateRequestIllegalValue(void) {
    /* test illegal value INT8_MAX for the state-request */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    TEST_ASSERT_EQUAL(ILCK_ILLEGAL_REQUEST, ILCK_SetStateRequest(INT8_MAX));
}

void testILCK_SetStateRequestDoubleInitializationWithoutStatemachine(void) {
    /* run initialization twice, but statemachine not in between */
    IO_SetPinDirectionToOutput_Ignore();
    IO_SetPinDirectionToInput_Ignore();
    IO_PinReset_Ignore();
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    TEST_ASSERT_EQUAL(ILCK_OK, ILCK_SetStateRequest(ILCK_STATE_INITIALIZATION_REQUEST));
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    TEST_ASSERT_EQUAL(ILCK_REQUEST_PENDING, ILCK_SetStateRequest(ILCK_STATE_INITIALIZATION_REQUEST));
}

void testILCK_SetStateRequestDoubleInitialization(void) {
    /* run initialization twice and call statemachine between these requests */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();

    TEST_ASSERT_EQUAL(ILCK_OK, ILCK_SetStateRequest(ILCK_STATE_INITIALIZATION_REQUEST));

    /* This group is called by the reentrance check */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();

    /* This group is called by transfer state request */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();

    /* This is the pin initialization */
    IO_SetPinDirectionToOutput_Ignore();
    IO_SetPinDirectionToInput_Ignore();
    IO_PinReset_Ignore();

    ILCK_Trigger();

    TEST_ASSERT_EQUAL(ILCK_STATEMACHINE_INITIALIZED, ILCK_GetState());

    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    TEST_ASSERT_EQUAL(ILCK_ALREADY_INITIALIZED, ILCK_SetStateRequest(ILCK_STATE_INITIALIZATION_REQUEST));
}

void testRunStatemachineWithoutRequest(void) {
    /* This group is called by the reentrance check */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();

    /* This group is called by transfer state request */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();

    ILCK_Trigger();

    TEST_ASSERT_EQUAL(ILCK_STATEMACHINE_UNINITIALIZED, ILCK_GetState());
}

void testInitializeStatemachine(void) {
    /* run initialization */
    /* since we are checking only for the statemachine passing through these
    states, we ignore all unnecessary functions */

    OS_EnterTaskCritical_Ignore();
    OS_ExitTaskCritical_Ignore();
    IO_SetPinDirectionToOutput_Ignore();
    IO_SetPinDirectionToInput_Ignore();
    IO_PinReset_Ignore();

    DATA_Read_1_DataBlock_IgnoreAndReturn(STD_OK);
    DATA_Write_1_DataBlock_IgnoreAndReturn(STD_OK);
    DIAG_CheckEvent_IgnoreAndReturn(STD_OK);

    TEST_ASSERT_EQUAL(ILCK_OK, ILCK_SetStateRequest(ILCK_STATE_INITIALIZATION_REQUEST));

    IO_PinGet_IgnoreAndReturn(STD_PIN_LOW);
    DIAG_Handler_IgnoreAndReturn(DIAG_HANDLER_RETURN_OK);

    for (uint8_t i = 0u; i < 10; i++) {
        /* iterate calling this statemachine 10 times (one shorttime) */
        ILCK_Trigger();
    }

    TEST_ASSERT_EQUAL(ILCK_STATEMACHINE_INITIALIZED, ILCK_GetState());
}

void testILCK_SetStateRequestIllegalValueAndThenRunStatemachine(void) {
    /* test illegal value INT8_MAX for the state-request */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    TEST_ASSERT_EQUAL(ILCK_ILLEGAL_REQUEST, ILCK_SetStateRequest(INT8_MAX));

    /* This group is called by the reentrance check */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();

    /* This group is called by transfer state request */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();

    ILCK_Trigger();

    /* Statemachine should stay uninitialized with illegal state request */
    TEST_ASSERT_EQUAL(ILCK_STATEMACHINE_UNINITIALIZED, ILCK_GetState());
}

void testILCK_GetInterlockFeedbackFeedbackOn(void) {
    /* check if on is returned if the pin says on */
    /* we don't care about these functions in this test */
    OS_EnterTaskCritical_Ignore();
    OS_ExitTaskCritical_Ignore();

    /* set the return value to 1, which means interlock on */
    IO_PinSet_Ignore();
    IO_PinReset_Ignore();
    IO_PinGet_ExpectAndReturn(&ILCK_IO_REG_PORT->DIN, ILCK_INTERLOCK_FEEDBACK_PIN_IL_STATE, STD_PIN_LOW);
    /* gioGetBit_ExpectAndReturn(ILCK_IO_REG, ILCK_INTERLOCK_FEEDBACK, 1u); */
    DATA_Read_1_DataBlock_IgnoreAndReturn(STD_OK);
    DATA_Write_1_DataBlock_IgnoreAndReturn(STD_OK);

    TEST_ASSERT_EQUAL(ILCK_SWITCH_ON, TEST_ILCK_GetInterlockFeedback());
}

void testILCK_GetInterlockFeedbackFeedbackOff(void) {
    /* check if off is returned if the pin says off */
    /* we don't care about these functions in this test */
    OS_EnterTaskCritical_Ignore();
    OS_ExitTaskCritical_Ignore();

    IO_PinSet_Ignore();
    IO_PinReset_Ignore();
    /* set the return value to 0, which means interlock off */
    IO_PinGet_ExpectAndReturn(&ILCK_IO_REG_PORT->DIN, ILCK_INTERLOCK_FEEDBACK_PIN_IL_STATE, STD_PIN_HIGH);
    /* gioGetBit_ExpectAndReturn(ILCK_IO_REG, ILCK_INTERLOCK_FEEDBACK, 0u); */
    DATA_Read_1_DataBlock_IgnoreAndReturn(STD_OK);
    DATA_Write_1_DataBlock_IgnoreAndReturn(STD_OK);

    TEST_ASSERT_EQUAL(ILCK_SWITCH_OFF, TEST_ILCK_GetInterlockFeedback());
}
