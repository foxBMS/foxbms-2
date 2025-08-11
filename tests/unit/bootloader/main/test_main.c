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
 * @file    test_main.c
 * @author  foxBMS Team
 * @date    2020-04-01 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Tests of the basic initialization of the hardware done in main
 * @details This is a test of the basic initialization. However, this basic
 *          initialization is performed **after** the low-level initialization
 *          in the startup implementation.
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "MockHL_gio.h"
#include "MockHL_pinmux.h"
#include "MockHL_rti.h"
#include "MockHL_sys_core.h"
#include "Mockboot.h"
#include "Mockcan.h"
#include "Mockinfinite-loop-helper.h"
#include "Mockrti.h"

#include "boot_cfg.h"

#include "fstd_types.h"
#include "main.h"

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("main.c")

TEST_INCLUDE_PATH("../../src/bootloader/driver/can")
TEST_INCLUDE_PATH("../../src/bootloader/driver/config")
TEST_INCLUDE_PATH("../../src/bootloader/driver/rti")
TEST_INCLUDE_PATH("../../src/bootloader/engine/boot")
TEST_INCLUDE_PATH("../../src/bootloader/main")

/*========== Definitions and Implementations for Unit Test ==================*/
/* Define the following variables in the following context to prevent unittest
error. */
uint32_t main_textLoadStartFlashC     = 0u;
uint32_t main_textSizeFlashC          = 0u;
uint32_t main_textRunStartFlashC      = 0u;
uint32_t main_constLoadStartFlashCfgC = 0u;
uint32_t main_constSizeFlashCfgC      = 0u;
uint32_t main_constRunStartFlashCfgC  = 0u;

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/

void testMain(void) {
    /* ======= Routine tests =============================================== */
    /* ======= RT1/11: wait if no any can request comes */
    boot_state = BOOT_FSM_STATE_WAIT;

    _enable_interrupt__Expect();

    muxInit_Expect();
    gioInit_Expect();
    CAN_Initialize_Expect();
    rtiInit_Expect();
    CAN_SendBootMessage_Expect();
    RTI_ResetFreeRunningCount_ExpectAndReturn(1);
    rtiStartCounter_Expect(rtiREG1, rtiCOUNTER_BLOCK0);
    RTI_GetFreeRunningCount_ExpectAndReturn(0u);

    for (uint8_t i_call = 0u; i_call < 10u; i_call++) {
        FOREVER_ExpectAndReturn(1);
        _enable_IRQ_interrupt__Expect();
        RTI_IsTimeElapsed_ExpectAndReturn(0u, MAIN_TIME_OUT_us, false);
        BOOT_GetBootState_ExpectAndReturn(BOOT_FSM_STATE_WAIT);
    }
    FOREVER_ExpectAndReturn(0);

    unit_test_main();
    TEST_ASSERT_EQUAL(BOOT_FSM_STATE_WAIT, boot_state);

    /* ======= RT2/11: normal loading process, everything runs well */
    resetTest();
    boot_state = BOOT_FSM_STATE_WAIT;

    _enable_interrupt__Expect();

    muxInit_Expect();
    gioInit_Expect();
    CAN_Initialize_Expect();
    rtiInit_Expect();

    CAN_SendBootMessage_Expect();
    RTI_ResetFreeRunningCount_ExpectAndReturn(1);
    rtiStartCounter_Expect(rtiREG1, rtiCOUNTER_BLOCK0);
    RTI_GetFreeRunningCount_ExpectAndReturn(0u);

    FOREVER_ExpectAndReturn(1);
    _enable_IRQ_interrupt__Expect();
    RTI_IsTimeElapsed_ExpectAndReturn(0u, MAIN_TIME_OUT_us, false);
    BOOT_GetBootState_ExpectAndReturn(BOOT_FSM_STATE_LOAD);
    for (uint8_t i_call = 0u; i_call < 8u; i_call++) {
        FOREVER_ExpectAndReturn(1);
        _enable_IRQ_interrupt__Expect();
        RTI_IsTimeElapsed_ExpectAndReturn(0u, MAIN_TIME_OUT_us, false);
        BOOT_GetBootStateDuringLoad_ExpectAndReturn(BOOT_FSM_STATE_LOAD);
    }
    FOREVER_ExpectAndReturn(1);
    _enable_IRQ_interrupt__Expect();
    RTI_IsTimeElapsed_ExpectAndReturn(0u, MAIN_TIME_OUT_us, false);
    BOOT_GetBootStateDuringLoad_ExpectAndReturn(BOOT_FSM_STATE_WAIT);
    FOREVER_ExpectAndReturn(0);

    unit_test_main();
    TEST_ASSERT_EQUAL(BOOT_FSM_STATE_WAIT, boot_state);

    /* ======= RT3/11: normal loading process, error appears during loading */
    resetTest();
    boot_state = BOOT_FSM_STATE_WAIT;

    _enable_interrupt__Expect();

    muxInit_Expect();
    gioInit_Expect();
    CAN_Initialize_Expect();
    rtiInit_Expect();

    CAN_SendBootMessage_Expect();
    RTI_ResetFreeRunningCount_ExpectAndReturn(1);
    rtiStartCounter_Expect(rtiREG1, rtiCOUNTER_BLOCK0);
    RTI_GetFreeRunningCount_ExpectAndReturn(0u);

    FOREVER_ExpectAndReturn(1);
    _enable_IRQ_interrupt__Expect();
    RTI_IsTimeElapsed_ExpectAndReturn(0u, MAIN_TIME_OUT_us, false);
    BOOT_GetBootState_ExpectAndReturn(BOOT_FSM_STATE_LOAD);
    FOREVER_ExpectAndReturn(1);
    _enable_IRQ_interrupt__Expect();
    RTI_IsTimeElapsed_ExpectAndReturn(0u, MAIN_TIME_OUT_us, false);
    BOOT_GetBootStateDuringLoad_ExpectAndReturn(BOOT_FSM_STATE_ERROR);
    for (uint8_t i_call = 0u; i_call < 8u; i_call++) {
        FOREVER_ExpectAndReturn(1);
        _enable_IRQ_interrupt__Expect();
        RTI_IsTimeElapsed_ExpectAndReturn(0u, MAIN_TIME_OUT_us, false);
        BOOT_GetBootStateDuringError_ExpectAndReturn(BOOT_FSM_STATE_ERROR);
    }
    FOREVER_ExpectAndReturn(0);

    unit_test_main();
    TEST_ASSERT_EQUAL(BOOT_FSM_STATE_ERROR, boot_state);

    /* ======= RT4/11: normal loading process, error appears during loading,
    incoming reset can request to correct the error, and the reset operation is
    successful */
    resetTest();
    boot_state = BOOT_FSM_STATE_WAIT;

    _enable_interrupt__Expect();

    muxInit_Expect();
    gioInit_Expect();
    CAN_Initialize_Expect();
    rtiInit_Expect();

    CAN_SendBootMessage_Expect();
    RTI_ResetFreeRunningCount_ExpectAndReturn(1);
    rtiStartCounter_Expect(rtiREG1, rtiCOUNTER_BLOCK0);
    RTI_GetFreeRunningCount_ExpectAndReturn(0u);

    FOREVER_ExpectAndReturn(1);
    _enable_IRQ_interrupt__Expect();
    RTI_IsTimeElapsed_ExpectAndReturn(0u, MAIN_TIME_OUT_us, false);
    BOOT_GetBootState_ExpectAndReturn(BOOT_FSM_STATE_LOAD);
    FOREVER_ExpectAndReturn(1);
    _enable_IRQ_interrupt__Expect();
    RTI_IsTimeElapsed_ExpectAndReturn(0u, MAIN_TIME_OUT_us, false);
    BOOT_GetBootStateDuringLoad_ExpectAndReturn(BOOT_FSM_STATE_ERROR);
    FOREVER_ExpectAndReturn(1);
    _enable_IRQ_interrupt__Expect();
    RTI_IsTimeElapsed_ExpectAndReturn(0u, MAIN_TIME_OUT_us, false);
    BOOT_GetBootStateDuringError_ExpectAndReturn(BOOT_FSM_STATE_RESET);
    FOREVER_ExpectAndReturn(1);
    _enable_IRQ_interrupt__Expect();
    RTI_IsTimeElapsed_ExpectAndReturn(0u, MAIN_TIME_OUT_us, false);
    BOOT_ResetBootloader_ExpectAndReturn(STD_OK);
    for (uint8_t i_call = 0u; i_call < 6u; i_call++) {
        FOREVER_ExpectAndReturn(1);
        _enable_IRQ_interrupt__Expect();
        RTI_IsTimeElapsed_ExpectAndReturn(0u, MAIN_TIME_OUT_us, false);
        BOOT_GetBootState_ExpectAndReturn(BOOT_FSM_STATE_WAIT);
    }
    FOREVER_ExpectAndReturn(0);

    unit_test_main();
    TEST_ASSERT_EQUAL(BOOT_FSM_STATE_WAIT, boot_state);

    /* ======= RT5/11: normal loading process, error appears during loading,
    incoming reset request to correct the error, but the reset operation is
    not successful */
    resetTest();
    boot_state = BOOT_FSM_STATE_WAIT;

    _enable_interrupt__Expect();

    muxInit_Expect();
    gioInit_Expect();
    CAN_Initialize_Expect();
    rtiInit_Expect();

    CAN_SendBootMessage_Expect();
    RTI_ResetFreeRunningCount_ExpectAndReturn(1);
    rtiStartCounter_Expect(rtiREG1, rtiCOUNTER_BLOCK0);
    RTI_GetFreeRunningCount_ExpectAndReturn(0u);

    FOREVER_ExpectAndReturn(1);
    _enable_IRQ_interrupt__Expect();
    RTI_IsTimeElapsed_ExpectAndReturn(0u, MAIN_TIME_OUT_us, false);
    BOOT_GetBootState_ExpectAndReturn(BOOT_FSM_STATE_LOAD);
    FOREVER_ExpectAndReturn(1);
    _enable_IRQ_interrupt__Expect();
    RTI_IsTimeElapsed_ExpectAndReturn(0u, MAIN_TIME_OUT_us, false);
    BOOT_GetBootStateDuringLoad_ExpectAndReturn(BOOT_FSM_STATE_ERROR);
    FOREVER_ExpectAndReturn(1);
    _enable_IRQ_interrupt__Expect();
    RTI_IsTimeElapsed_ExpectAndReturn(0u, MAIN_TIME_OUT_us, false);
    BOOT_GetBootStateDuringError_ExpectAndReturn(BOOT_FSM_STATE_RESET);
    FOREVER_ExpectAndReturn(1);
    _enable_IRQ_interrupt__Expect();
    RTI_IsTimeElapsed_ExpectAndReturn(0u, MAIN_TIME_OUT_us, false);
    BOOT_ResetBootloader_ExpectAndReturn(STD_NOT_OK);
    for (uint8_t i_call = 0u; i_call < 6u; i_call++) {
        FOREVER_ExpectAndReturn(1);
        _enable_IRQ_interrupt__Expect();
        RTI_IsTimeElapsed_ExpectAndReturn(0u, MAIN_TIME_OUT_us, false);
        BOOT_GetBootStateDuringError_ExpectAndReturn(BOOT_FSM_STATE_ERROR);
    }
    FOREVER_ExpectAndReturn(0);

    unit_test_main();
    TEST_ASSERT_EQUAL(BOOT_FSM_STATE_ERROR, boot_state);

    /* ======= RT6/11: run application, there is an valid program available and
    everything went well while jumping into the last flashed program */
    resetTest();
    boot_state = BOOT_FSM_STATE_WAIT;

    _enable_interrupt__Expect();

    muxInit_Expect();
    gioInit_Expect();
    CAN_Initialize_Expect();
    rtiInit_Expect();

    CAN_SendBootMessage_Expect();
    RTI_ResetFreeRunningCount_ExpectAndReturn(1);
    rtiStartCounter_Expect(rtiREG1, rtiCOUNTER_BLOCK0);
    RTI_GetFreeRunningCount_ExpectAndReturn(0u);

    FOREVER_ExpectAndReturn(1);
    _enable_IRQ_interrupt__Expect();
    RTI_IsTimeElapsed_ExpectAndReturn(0u, MAIN_TIME_OUT_us, false);
    BOOT_GetBootState_ExpectAndReturn(BOOT_FSM_STATE_RUN);
    FOREVER_ExpectAndReturn(1);
    _enable_IRQ_interrupt__Expect();
    RTI_IsTimeElapsed_ExpectAndReturn(0u, MAIN_TIME_OUT_us, false);
    BOOT_IsProgramAvailableAndValidated_ExpectAndReturn(true);
    BOOT_JumpInToLastFlashedProgram_ExpectAndReturn(STD_OK);
    for (uint8_t i_call = 0u; i_call < 8u; i_call++) {
        FOREVER_ExpectAndReturn(1);
        _enable_IRQ_interrupt__Expect();
        RTI_IsTimeElapsed_ExpectAndReturn(0u, MAIN_TIME_OUT_us, false);
        BOOT_GetBootState_ExpectAndReturn(BOOT_FSM_STATE_WAIT);
    }
    FOREVER_ExpectAndReturn(0);

    unit_test_main();
    TEST_ASSERT_EQUAL(BOOT_FSM_STATE_WAIT, boot_state);

    /* ======= RT7/11: run application, there is an valid program available but
    it fails while jumping into the last flashed program */
    resetTest();
    boot_state = BOOT_FSM_STATE_WAIT;

    _enable_interrupt__Expect();

    muxInit_Expect();
    gioInit_Expect();
    CAN_Initialize_Expect();
    rtiInit_Expect();

    CAN_SendBootMessage_Expect();
    RTI_ResetFreeRunningCount_ExpectAndReturn(1);
    rtiStartCounter_Expect(rtiREG1, rtiCOUNTER_BLOCK0);
    RTI_GetFreeRunningCount_ExpectAndReturn(0u);

    FOREVER_ExpectAndReturn(1);
    _enable_IRQ_interrupt__Expect();
    RTI_IsTimeElapsed_ExpectAndReturn(0u, MAIN_TIME_OUT_us, false);
    BOOT_GetBootState_ExpectAndReturn(BOOT_FSM_STATE_RUN);
    FOREVER_ExpectAndReturn(1);
    _enable_IRQ_interrupt__Expect();
    RTI_IsTimeElapsed_ExpectAndReturn(0u, MAIN_TIME_OUT_us, false);
    BOOT_IsProgramAvailableAndValidated_ExpectAndReturn(true);
    BOOT_JumpInToLastFlashedProgram_ExpectAndReturn(STD_NOT_OK);
    for (uint8_t i_call = 0u; i_call < 8u; i_call++) {
        FOREVER_ExpectAndReturn(1);
        _enable_IRQ_interrupt__Expect();
        RTI_IsTimeElapsed_ExpectAndReturn(0u, MAIN_TIME_OUT_us, false);
        BOOT_GetBootStateDuringError_ExpectAndReturn(BOOT_FSM_STATE_ERROR);
    }
    FOREVER_ExpectAndReturn(0);

    unit_test_main();
    TEST_ASSERT_EQUAL(BOOT_FSM_STATE_ERROR, boot_state);

    /* ======= RT8/11: run application, there is no valid program available */
    resetTest();
    boot_state = BOOT_FSM_STATE_WAIT;

    _enable_interrupt__Expect();

    muxInit_Expect();
    gioInit_Expect();
    CAN_Initialize_Expect();
    rtiInit_Expect();

    CAN_SendBootMessage_Expect();
    RTI_ResetFreeRunningCount_ExpectAndReturn(1);
    rtiStartCounter_Expect(rtiREG1, rtiCOUNTER_BLOCK0);
    RTI_GetFreeRunningCount_ExpectAndReturn(0u);

    FOREVER_ExpectAndReturn(1);
    _enable_IRQ_interrupt__Expect();
    RTI_IsTimeElapsed_ExpectAndReturn(0u, MAIN_TIME_OUT_us, false);
    BOOT_GetBootState_ExpectAndReturn(BOOT_FSM_STATE_RUN);
    FOREVER_ExpectAndReturn(1);
    _enable_IRQ_interrupt__Expect();
    RTI_IsTimeElapsed_ExpectAndReturn(0u, MAIN_TIME_OUT_us, false);
    BOOT_IsProgramAvailableAndValidated_ExpectAndReturn(false);
    for (uint8_t i_call = 0u; i_call < 8u; i_call++) {
        FOREVER_ExpectAndReturn(1);
        _enable_IRQ_interrupt__Expect();
        RTI_IsTimeElapsed_ExpectAndReturn(0u, MAIN_TIME_OUT_us, false);
        BOOT_GetBootState_ExpectAndReturn(BOOT_FSM_STATE_WAIT);
    }
    FOREVER_ExpectAndReturn(0);

    unit_test_main();
    TEST_ASSERT_EQUAL(BOOT_FSM_STATE_WAIT, boot_state);

    /* ======= RT9/11: unregistered state */
    resetTest();
    boot_state = 10u;

    _enable_interrupt__Expect();

    muxInit_Expect();
    gioInit_Expect();
    CAN_Initialize_Expect();
    rtiInit_Expect();

    CAN_SendBootMessage_Expect();
    RTI_ResetFreeRunningCount_ExpectAndReturn(1);
    rtiStartCounter_Expect(rtiREG1, rtiCOUNTER_BLOCK0);
    RTI_GetFreeRunningCount_ExpectAndReturn(0u);

    FOREVER_ExpectAndReturn(1);
    _enable_IRQ_interrupt__Expect();
    RTI_IsTimeElapsed_ExpectAndReturn(0u, MAIN_TIME_OUT_us, false);
    FOREVER_ExpectAndReturn(0);

    unit_test_main();
    TEST_ASSERT_EQUAL(BOOT_FSM_STATE_ERROR, boot_state);

    /* ======= RT9/11: timeout, boot_state is BOOT_FSM_STATE_WAIT */
    resetTest();
    boot_state = BOOT_FSM_STATE_WAIT;

    _enable_interrupt__Expect();

    muxInit_Expect();
    gioInit_Expect();
    CAN_Initialize_Expect();
    rtiInit_Expect();

    CAN_SendBootMessage_Expect();
    RTI_ResetFreeRunningCount_ExpectAndReturn(1);
    rtiStartCounter_Expect(rtiREG1, rtiCOUNTER_BLOCK0);
    RTI_GetFreeRunningCount_ExpectAndReturn(0u);

    FOREVER_ExpectAndReturn(1);
    _enable_IRQ_interrupt__Expect();
    RTI_IsTimeElapsed_ExpectAndReturn(0u, MAIN_TIME_OUT_us, true);
    rtiStopCounter_Expect(rtiREG1, rtiCOUNTER_BLOCK0);
    RTI_ResetFreeRunningCount_ExpectAndReturn(1);
    BOOT_IsProgramAvailableAndValidated_ExpectAndReturn(false);
    FOREVER_ExpectAndReturn(0);

    unit_test_main();
    TEST_ASSERT_EQUAL(BOOT_FSM_STATE_WAIT, boot_state);

    /* ======= RT10/11: timeout, boot_state is BOOT_FSM_STATE_WAIT, try to run app,
    but no validated app available */
    resetTest();
    boot_state = BOOT_FSM_STATE_WAIT;

    _enable_interrupt__Expect();

    muxInit_Expect();
    gioInit_Expect();
    CAN_Initialize_Expect();
    rtiInit_Expect();

    CAN_SendBootMessage_Expect();
    RTI_ResetFreeRunningCount_ExpectAndReturn(1);
    rtiStartCounter_Expect(rtiREG1, rtiCOUNTER_BLOCK0);
    RTI_GetFreeRunningCount_ExpectAndReturn(0u);

    FOREVER_ExpectAndReturn(1);
    _enable_IRQ_interrupt__Expect();
    RTI_IsTimeElapsed_ExpectAndReturn(0u, MAIN_TIME_OUT_us, true);
    rtiStopCounter_Expect(rtiREG1, rtiCOUNTER_BLOCK0);
    RTI_ResetFreeRunningCount_ExpectAndReturn(1);
    BOOT_IsProgramAvailableAndValidated_ExpectAndReturn(false);
    FOREVER_ExpectAndReturn(1);
    _enable_IRQ_interrupt__Expect();
    BOOT_GetBootState_ExpectAndReturn(BOOT_FSM_STATE_WAIT);
    FOREVER_ExpectAndReturn(0);

    unit_test_main();
    TEST_ASSERT_EQUAL(BOOT_FSM_STATE_WAIT, boot_state);

    /* ======= RT11/11: timeout, boot_state is not BOOT_FSM_STATE_WAIT */
    resetTest();
    boot_state = BOOT_FSM_STATE_ERROR;

    _enable_interrupt__Expect();

    muxInit_Expect();
    gioInit_Expect();
    CAN_Initialize_Expect();
    rtiInit_Expect();

    CAN_SendBootMessage_Expect();
    RTI_ResetFreeRunningCount_ExpectAndReturn(1);
    rtiStartCounter_Expect(rtiREG1, rtiCOUNTER_BLOCK0);
    RTI_GetFreeRunningCount_ExpectAndReturn(0u);

    FOREVER_ExpectAndReturn(1);
    _enable_IRQ_interrupt__Expect();
    RTI_IsTimeElapsed_ExpectAndReturn(0u, MAIN_TIME_OUT_us, true);
    BOOT_GetBootStateDuringError_ExpectAndReturn(BOOT_FSM_STATE_ERROR);
    FOREVER_ExpectAndReturn(0);
    unit_test_main();
    TEST_ASSERT_EQUAL(BOOT_FSM_STATE_ERROR, boot_state);
}
