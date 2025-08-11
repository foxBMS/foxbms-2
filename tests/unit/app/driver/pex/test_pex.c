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
 * @file    test_pex.c
 * @author  foxBMS Team
 * @date    2021-09-29 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Tests for the port expander module
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "MockHL_i2c.h"
#include "Mockdatabase.h"
#include "Mockdiag.h"
#include "Mockdiag_cfg.h"
#include "Mocki2c.h"
#include "Mockos.h"
#include "Mockportmacro.h"

#include "pex_cfg.h"

#include "fstd_types.h"
#include "pex.h"
#include "test_assert_helper.h"

#include <stdint.h>

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("pex.c")
TEST_SOURCE_FILE("pex_cfg.c")

TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/i2c")
TEST_INCLUDE_PATH("../../src/app/driver/pex")
TEST_INCLUDE_PATH("../../src/app/engine/diag")

/*========== Definitions and Implementations for Unit Test ==================*/

/** Initial value: all 1 */
#define PEX_DEFAULT_VALUE_ALL_1 (0xFFu)
/** Initial value: all 0 */
#define PEX_DEFAULT_VALUE_ALL_0 (0x0u)

#define TEST_PEX_I2C_INTERFACE                    (i2cREG1)
#define TEST_PEX_INPUT_PORT0_REGISTER_ADDRESS     (0x0u)
#define TEST_PEX_OUTPUT_PORT0_REGISTER_ADDRESS    (0x2u)
#define TEST_PEX_POL_INV_PORT0_REGISTER_ADDRESS   (0x4u)
#define TEST_PEX_DIRECTION_PORT0_REGISTER_ADDRESS (0x6u)

long FSYS_RaisePrivilege(void) {
    return 0;
}

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/

/**
 * @brief   Testing static function #PEX_ReadInputs
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - none
 *          - Routine validation:
 *            - RT1/2: function calls expected subroutine and it succeeds
 *            - RT2/2: function calls expected subroutine and it fails
 */
void testPEX_ReadInputs(void) {
    /* ======= Assertion tests ============================================= */
    /* none */

    /* ======= Routine tests =============================================== */

    uint8_t expectedI2cDataWrite[1u] = {TEST_PEX_INPUT_PORT0_REGISTER_ADDRESS};
    uint8_t expectedI2cDataRead[2u]  = {0u, 0u};

    /* ======= RT1/2: Test implementation */
    for (uint8_t i = 0; i < PEX_NR_OF_PORT_EXPANDERS; i++) {
        I2C_WriteReadDma_ExpectAndReturn(
            TEST_PEX_I2C_INTERFACE, pex_addressList[i], 1u, expectedI2cDataWrite, 2u, expectedI2cDataRead, STD_OK);
    }

    /* ======= RT1/2: call function under test */
    const STD_RETURN_TYPE_e succeeded = TEST_PEX_ReadInputs();

    /* ======= RT1/2: test output verification */
    TEST_ASSERT_EQUAL(STD_OK, succeeded);

    /* ======= RT2/2: Test implementation */
    for (uint8_t i = 0; i < PEX_NR_OF_PORT_EXPANDERS; i++) {
        I2C_WriteReadDma_ExpectAndReturn(
            TEST_PEX_I2C_INTERFACE, pex_addressList[i], 1u, expectedI2cDataWrite, 2u, expectedI2cDataRead, STD_NOT_OK);
    }

    /* ======= RT2/2: call function under test */
    const STD_RETURN_TYPE_e failed = TEST_PEX_ReadInputs();

    /* ======= RT2/2: test output verification */
    TEST_ASSERT_EQUAL(STD_NOT_OK, failed);
}

/**
 * @brief   Testing static function #PEX_WriteOutputs
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - none
 *          - Routine validation:
 *            - RT1/2: function calls expected subroutine and it succeeds
 *            - RT2/2: function calls expected subroutine and it fails
 */
void testPEX_WriteOutputs(void) {
    /* ======= Assertion tests ============================================= */
    /* none */

    /* ======= Routine tests =============================================== */

    uint8_t expectedI2cDataWrite[3u] = {TEST_PEX_OUTPUT_PORT0_REGISTER_ADDRESS, 0u, 0u};

    /* ======= RT1/2: Test implementation */
    for (uint8_t i = 0; i < PEX_NR_OF_PORT_EXPANDERS; i++) {
        I2C_WriteDma_ExpectAndReturn(TEST_PEX_I2C_INTERFACE, pex_addressList[i], 3u, expectedI2cDataWrite, STD_OK);
    }

    /* ======= RT1/2: call function under test */
    const STD_RETURN_TYPE_e succeeded = TEST_PEX_WriteOutputs();

    /* ======= RT1/2: test output verification */
    TEST_ASSERT_EQUAL(STD_OK, succeeded);

    /* ======= RT2/2: Test implementation */
    for (uint8_t i = 0; i < PEX_NR_OF_PORT_EXPANDERS; i++) {
        I2C_WriteDma_ExpectAndReturn(TEST_PEX_I2C_INTERFACE, pex_addressList[i], 3u, expectedI2cDataWrite, STD_NOT_OK);
    }

    /* ======= RT2/2: call function under test */
    const STD_RETURN_TYPE_e failed = TEST_PEX_WriteOutputs();

    /* ======= RT2/2: test output verification */
    TEST_ASSERT_EQUAL(STD_NOT_OK, failed);
}

/**
 * @brief   Testing static function #PEX_WriteConfigPolarity
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - none
 *          - Routine validation:
 *            - RT1/2: function calls expected subroutine and it succeeds
 *            - RT2/2: function calls expected subroutine and it fails
 */
void testPEX_WriteConfigPolarity(void) {
    /* ======= Assertion tests ============================================= */
    /* none */

    /* ======= Routine tests =============================================== */
    const uint8_t expectedNrOfBytesToWrite = 3u;
    uint8_t expectedI2cDataWrite[3u]       = {TEST_PEX_POL_INV_PORT0_REGISTER_ADDRESS, 0u, 0u};

    /* ======= RT1/2: Test implementation */
    for (uint8_t i = 0; i < PEX_NR_OF_PORT_EXPANDERS; i++) {
        I2C_WriteDma_ExpectAndReturn(
            TEST_PEX_I2C_INTERFACE, pex_addressList[i], expectedNrOfBytesToWrite, expectedI2cDataWrite, STD_OK);
    }

    /* ======= RT1/2: call function under test */
    const STD_RETURN_TYPE_e succeeded = TEST_PEX_WriteConfigPolarity();

    /* ======= RT1/2: test output verification */
    TEST_ASSERT_EQUAL(STD_OK, succeeded);

    /* ======= RT2/2: Test implementation */
    for (uint8_t i = 0; i < PEX_NR_OF_PORT_EXPANDERS; i++) {
        I2C_WriteDma_ExpectAndReturn(
            TEST_PEX_I2C_INTERFACE, pex_addressList[i], expectedNrOfBytesToWrite, expectedI2cDataWrite, STD_NOT_OK);
    }

    /* ======= RT2/2: call function under test */
    const STD_RETURN_TYPE_e failed = TEST_PEX_WriteConfigPolarity();

    /* ======= RT2/2: test output verification */
    TEST_ASSERT_EQUAL(STD_NOT_OK, failed);
}

/**
 * @brief   Testing static function #PEX_WriteConfigDirection
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - none
 *          - Routine validation:
 *            - RT1/2: function calls expected subroutine and it succeeds
 *            - RT2/2: function calls expected subroutine and it fails
 */
void testPEX_WriteConfigDirection(void) {
    /* ======= Assertion tests ============================================= */
    /* none */

    /* ======= Routine tests =============================================== */
    const uint8_t expectedNrOfBytesToWrite = 3u;
    uint8_t expectedI2cDataWrite[3u]       = {TEST_PEX_DIRECTION_PORT0_REGISTER_ADDRESS, 0u, 0u};

    /* ======= RT1/2: Test implementation */
    for (uint8_t i = 0; i < PEX_NR_OF_PORT_EXPANDERS; i++) {
        I2C_WriteDma_ExpectAndReturn(
            TEST_PEX_I2C_INTERFACE, pex_addressList[i], expectedNrOfBytesToWrite, expectedI2cDataWrite, STD_OK);
    }

    /* ======= RT1/2: call function under test */
    const STD_RETURN_TYPE_e succeeded = TEST_PEX_WriteConfigDirection();

    /* ======= RT1/2: test output verification */
    TEST_ASSERT_EQUAL(STD_OK, succeeded);

    /* ======= RT2/2: Test implementation */
    for (uint8_t i = 0; i < PEX_NR_OF_PORT_EXPANDERS; i++) {
        I2C_WriteDma_ExpectAndReturn(
            TEST_PEX_I2C_INTERFACE, pex_addressList[i], expectedNrOfBytesToWrite, expectedI2cDataWrite, STD_NOT_OK);
    }

    /* ======= RT2/2: call function under test */
    const STD_RETURN_TYPE_e failed = TEST_PEX_WriteConfigDirection();

    /* ======= RT2/2: test output verification */
    TEST_ASSERT_EQUAL(STD_NOT_OK, failed);
}

/**
 * @brief   Testing static function #PEX_CopyToLocalVariable
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - none
 *          - Routine validation:
 *            - RT1/1: Test Function
 */
void testPEX_CopyToLocalVariable(void) {
    /* ======= Assertion tests ============================================= */
    /* none */

    /* ======= Routine tests =============================================== */

    /* ======= RT1/1: Test implementation */
    /* Expect calls */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();

    /* ======= RT1/1: call function under test */
    TEST_PEX_CopyToLocalVariable();
}

/**
 * @brief   Testing static function #PEX_GetFromLocalVariable
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - none
 *          - Routine validation:
 *            - RT1/1: Test Function
 */
void testPEX_GetFromLocalVariable(void) {
    /* ======= Assertion tests ============================================= */
    /* none */

    /* ======= Routine tests =============================================== */

    /* ======= RT1/1: Test implementation */
    /* Expect calls */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();

    /* ======= RT1/1: call function under test */
    TEST_PEX_GetFromLocalVariable();
}

/**
 * @brief   Testing extern function #PEX_Initialize
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - none
 *          - Routine validation:
 *            - TODO
 */
void testPEX_Initialize(void) {
    /* ======= Assertion tests ============================================= */

    /* ======= Routine tests =============================================== */
    PEX_Initialize();
}

/**
 * @brief   Testing extern function #PEX_Trigger
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - none
 *          - Routine validation:
 *            - TODO
 */
void testPEX_Trigger(void) {
    /* ======= Assertion tests ============================================= */

    /* ======= Routine tests =============================================== */
    const uint8_t expectedNrOfBytesToWrite    = 3u;
    uint8_t expectedI2cDataWriteDirection[3u] = {TEST_PEX_DIRECTION_PORT0_REGISTER_ADDRESS, 0u, 0u};
    uint32_t current_time                     = 0u;

    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();

    OS_GetTickCount_ExpectAndReturn(current_time);

    for (uint8_t i = 0; i < PEX_NR_OF_PORT_EXPANDERS; i++) {
        I2C_WriteDma_ExpectAndReturn(
            TEST_PEX_I2C_INTERFACE,
            pex_addressList[i],
            expectedNrOfBytesToWrite,
            expectedI2cDataWriteDirection,
            STD_OK);
    }
    OS_DelayTaskUntil_Expect(&current_time, 2u);

    uint8_t expectedI2cDataWritePolarity[3u] = {TEST_PEX_POL_INV_PORT0_REGISTER_ADDRESS, 0u, 0u};
    for (uint8_t i = 0; i < PEX_NR_OF_PORT_EXPANDERS; i++) {
        I2C_WriteDma_ExpectAndReturn(
            TEST_PEX_I2C_INTERFACE, pex_addressList[i], expectedNrOfBytesToWrite, expectedI2cDataWritePolarity, STD_OK);
    }
    OS_DelayTaskUntil_Expect(&current_time, 2u);

    uint8_t expectedI2cDataWriteInput[1u] = {TEST_PEX_INPUT_PORT0_REGISTER_ADDRESS};
    uint8_t expectedI2cDataRead[2u]       = {0u, 0u};
    for (uint8_t i = 0; i < PEX_NR_OF_PORT_EXPANDERS; i++) {
        I2C_WriteReadDma_ExpectAndReturn(
            TEST_PEX_I2C_INTERFACE, pex_addressList[i], 1u, expectedI2cDataWriteInput, 2u, expectedI2cDataRead, STD_OK);
    }
    OS_DelayTaskUntil_Expect(&current_time, 2u);

    uint8_t expectedI2cDataWriteOutput[3u] = {TEST_PEX_OUTPUT_PORT0_REGISTER_ADDRESS, 0u, 0u};
    for (uint8_t i = 0; i < PEX_NR_OF_PORT_EXPANDERS; i++) {
        I2C_WriteDma_ExpectAndReturn(
            TEST_PEX_I2C_INTERFACE, pex_addressList[i], 3u, expectedI2cDataWriteOutput, STD_OK);
    }
    OS_DelayTaskUntil_Expect(&current_time, 2u);

    DIAG_Handler_ExpectAndReturn(DIAG_ID_I2C_PEX_ERROR, DIAG_EVENT_OK, DIAG_SYSTEM, 0u, STD_OK);

    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();

    PEX_Trigger();
}

/**
 * @brief   Testing extern function #PEX_SetPin
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/2: invalid port expander &rarr; assert
 *            - AT2/2: invalid pin &rarr; assert
 *          - Routine validation:
 *            - RT1/2: function calls with pin in range PEX_PORT_0_PIN_0 to PEX_PORT_0_PIN_7
 *            - RT2/2: function calls with pin in range PEX_PORT_1_PIN_0 to PEX_PORT_1_PIN_7
 */
void testPEX_SetPin(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(PEX_SetPin(5, PEX_PORT_0_PIN_0));
    /* ======= AT2/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(PEX_SetPin(PEX_PORT_EXPANDER1, 16u));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/2: Test implementation */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();

    /* ======= RT1/2: call function under test */
    PEX_SetPin(PEX_PORT_EXPANDER1, PEX_PORT_0_PIN_0);

    /* ======= RT2/2: Test implementation */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();

    /* ======= RT2/2: call function under test */
    PEX_SetPin(PEX_PORT_EXPANDER1, PEX_PORT_1_PIN_7);
}

/**
 * @brief   Testing extern function #PEX_ResetPin
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/2: invalid port expander &rarr; assert
 *            - AT2/2: invalid pin &rarr; assert
 *          - Routine validation:
 *            - TODO
 */
void testPEX_ResetPin(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(PEX_ResetPin(5, PEX_PORT_0_PIN_0));
    /* ======= AT2/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(PEX_ResetPin(PEX_PORT_EXPANDER1, 16u));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/2: Test implementation */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();

    /* ======= RT1/2: call function under test */
    PEX_ResetPin(PEX_PORT_EXPANDER1, PEX_PORT_0_PIN_0);

    /* ======= RT2/2: Test implementation */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();

    /* ======= RT2/2: call function under test */
    PEX_ResetPin(PEX_PORT_EXPANDER1, PEX_PORT_1_PIN_7);
}

/**
 * @brief   Testing extern function #PEX_GetPin
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/2: invalid port expander &rarr; assert
 *            - AT2/2: invalid pin &rarr; assert
 *          - Routine validation:
 *            - RT1/3: function calls with pin in range PEX_PORT_0_PIN_0 to PEX_PORT_0_PIN_7
 *            - RT2/3: function calls with pin in range PEX_PORT_1_PIN_0 to PEX_PORT_1_PIN_7
 *            - RT3/3: function calls with high pin state
 */
void testPEX_GetPin(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(PEX_GetPin(5, PEX_PORT_0_PIN_0));
    /* ======= AT2/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(PEX_GetPin(PEX_PORT_EXPANDER1, 16u));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/3: Test implementation */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();

    /* ======= RT1/3: call function under test */
    PEX_GetPin(PEX_PORT_EXPANDER1, PEX_PORT_0_PIN_0);

    /* ======= RT2/3: Test implementation */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();

    /* ======= RT2/3: call function under test */
    PEX_GetPin(PEX_PORT_EXPANDER1, PEX_PORT_1_PIN_7);

    /* ======= RT3/3: Test implementation */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    PEX_GetPin(5, PEX_PORT_1_PIN_7);
}

/**
 * @brief   Testing extern function #PEX_SetPinDirectionInput
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/2: invalid port expander &rarr; assert
 *            - AT2/2: invalid pin &rarr; assert
 *          - Routine validation:
 *            - TODO
 */
void testPEX_SetPinDirectionInput(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(PEX_SetPinDirectionInput(5, PEX_PORT_0_PIN_0));
    /* ======= AT2/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(PEX_SetPinDirectionInput(PEX_PORT_EXPANDER1, 16u));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/2: Test implementation */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();

    /* ======= RT1/2: call function under test */
    PEX_SetPinDirectionInput(PEX_PORT_EXPANDER1, PEX_PORT_0_PIN_0);

    /* ======= RT2/2: Test implementation */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();

    /* ======= RT2/2: call function under test */
    PEX_SetPinDirectionInput(PEX_PORT_EXPANDER1, PEX_PORT_1_PIN_7);
}

/**
 * @brief   Testing extern function #PEX_SetPinDirectionOutput
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/2: invalid port expander &rarr; assert
 *            - AT2/2: invalid pin &rarr; assert
 *          - Routine validation:
 *            - TODO
 */
void testPEX_SetPinDirectionOutput(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(PEX_SetPinDirectionOutput(5, PEX_PORT_0_PIN_0));
    /* ======= AT2/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(PEX_SetPinDirectionOutput(PEX_PORT_EXPANDER1, 16u));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/2: Test implementation */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();

    /* ======= RT1/2: call function under test */
    PEX_SetPinDirectionOutput(PEX_PORT_EXPANDER1, PEX_PORT_0_PIN_0);

    /* ======= RT2/2: Test implementation */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();

    /* ======= RT2/2: call function under test */
    PEX_SetPinDirectionOutput(PEX_PORT_EXPANDER1, PEX_PORT_1_PIN_7);
}

/**
 * @brief   Testing extern function #PEX_SetPinPolarityInverted
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/2: invalid port expander &rarr; assert
 *            - AT2/2: invalid pin &rarr; assert
 *          - Routine validation:
 *            - TODO
 */
void testPEX_SetPinPolarityInverted(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(PEX_SetPinPolarityInverted(5, PEX_PORT_0_PIN_0));
    /* ======= AT2/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(PEX_SetPinPolarityInverted(PEX_PORT_EXPANDER1, 16u));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/2: Test implementation */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();

    /* ======= RT1/2: call function under test */
    PEX_SetPinPolarityInverted(PEX_PORT_EXPANDER1, PEX_PORT_0_PIN_0);

    /* ======= RT2/2: Test implementation */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();

    /* ======= RT2/2: call function under test */
    PEX_SetPinPolarityInverted(PEX_PORT_EXPANDER1, PEX_PORT_1_PIN_7);
}

/**
 * @brief   Testing extern function #PEX_SetPinPolarityRetained
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/2: invalid port expander &rarr; assert
 *            - AT2/2: invalid pin &rarr; assert
 *          - Routine validation:
 *            - TODO
 */
void testPEX_SetPinPolarityRetained(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(PEX_SetPinPolarityRetained(5, PEX_PORT_0_PIN_0));
    /* ======= AT2/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(PEX_SetPinPolarityRetained(PEX_PORT_EXPANDER1, 16u));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/2: Test implementation */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();

    /* ======= RT1/2: call function under test */
    PEX_SetPinPolarityRetained(PEX_PORT_EXPANDER1, PEX_PORT_0_PIN_0);

    /* ======= RT2/2: Test implementation */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();

    /* ======= RT2/2: call function under test */
    PEX_SetPinPolarityRetained(PEX_PORT_EXPANDER1, PEX_PORT_1_PIN_7);
}
