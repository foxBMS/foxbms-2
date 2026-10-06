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
 * @file    test_io.c
 * @author  foxBMS Team
 * @date    2020-06-10 (date of creation)
 * @updated 2026-10-06 (date of last update)
 * @version v1.12.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Tests for the io module
 * @details Test functions:
 *          - testIO_SetPinDirectionToOutput
 *          - testIO_SetPinDirectionToInput
 *          - testIO_PinSet
 *          - testIO_PinReset
 *          - testIO_PinGet
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockmcu.h"

#include "io.h"
#include "test_assert_helper.h"

/*========== Unit Testing Framework Directives ==============================*/

/*========== Definitions and Implementations for Unit Test ==================*/

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/

/**
 * @brief   Test of IO_SetPinDirectionToOutput
 * @details Cases:
 *          - Argument validation:
 *            - AT1/2: null register address -> assert
 *            - AT2/2: pin above MCU_LARGEST_PIN_NUMBER -> assert
 *          - Routine validation:
 *            - RT1/3: bit 0 changes from 0 to 1
 *            - RT2/3: bit 0 stays 1 when already set
 *            - RT3/3: highest valid pin is set without changing other bits
 */
void testIO_SetPinDirectionToOutput(void) {
    const uint32_t highestPinMask   = (uint32_t)1u << MCU_LARGEST_PIN_NUMBER;
    volatile uint32_t registerValue = 0u;

    /* ======= Assertion tests ============================================= */
    /* ======= AT1/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(IO_SetPinDirectionToOutput(NULL_PTR, 0u));

    /* ======= AT2/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(IO_SetPinDirectionToOutput(&registerValue, MCU_LARGEST_PIN_NUMBER + 1u));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/3: Test implementation ================================== */
    /* ======= RT1/3: Call function under test ============================= */
    IO_SetPinDirectionToOutput(&registerValue, 0u);

    /* ======= RT1/3: Test output verification ============================= */
    TEST_ASSERT_EQUAL(1u, registerValue);

    /* ======= RT2/3: Test implementation ================================== */
    /* ======= RT2/3: Call function under test ============================= */
    IO_SetPinDirectionToOutput(&registerValue, 0u);

    /* ======= RT2/3: Test output verification ============================= */
    TEST_ASSERT_EQUAL(1u, registerValue);

    /* ======= RT3/3: Test implementation ================================== */
    registerValue = 0x13579BDFu;

    /* ======= RT3/3: Call function under test ============================= */
    IO_SetPinDirectionToOutput(&registerValue, MCU_LARGEST_PIN_NUMBER);

    /* ======= RT3/3: Test output verification ============================= */
    TEST_ASSERT_EQUAL((0x13579BDFu | highestPinMask), registerValue);
}

/**
 * @brief   Test of IO_SetPinDirectionToInput
 * @details Cases:
 *          - Argument validation:
 *            - AT1/2: null register address -> assert
 *            - AT2/2: pin above MCU_LARGEST_PIN_NUMBER -> assert
 *          - Routine validation:
 *            - RT1/3: bit 0 changes from 1 to 0
 *            - RT2/3: bit 0 stays 0 when already cleared
 *            - RT3/3: highest valid pin is cleared without changing other bits
 */
void testIO_SetPinDirectionToInput(void) {
    const uint32_t highestPinMask   = (uint32_t)1u << MCU_LARGEST_PIN_NUMBER;
    volatile uint32_t registerValue = 1u;

    /* ======= Assertion tests ============================================= */
    /* ======= AT1/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(IO_SetPinDirectionToInput(NULL_PTR, 0u));

    /* ======= AT2/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(IO_SetPinDirectionToInput(&registerValue, MCU_LARGEST_PIN_NUMBER + 1u));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/3: Test implementation ================================== */
    /* ======= RT1/3: Call function under test ============================= */
    IO_SetPinDirectionToInput(&registerValue, 0u);

    /* ======= RT1/3: Test output verification ============================= */
    TEST_ASSERT_EQUAL(0u, registerValue);

    /* ======= RT2/3: Test implementation ================================== */
    /* ======= RT2/3: Call function under test ============================= */
    IO_SetPinDirectionToInput(&registerValue, 0u);

    /* ======= RT2/3: Test output verification ============================= */
    TEST_ASSERT_EQUAL(0u, registerValue);

    /* ======= RT3/3: Test implementation ================================== */
    registerValue = (0x13579BDFu | highestPinMask);

    /* ======= RT3/3: Call function under test ============================= */
    IO_SetPinDirectionToInput(&registerValue, MCU_LARGEST_PIN_NUMBER);

    /* ======= RT3/3: Test output verification ============================= */
    TEST_ASSERT_EQUAL(0x13579BDFu, registerValue);
}

/**
 * @brief   Test of IO_PinSet
 * @details Cases:
 *          - Argument validation:
 *            - AT1/2: null register address -> assert
 *            - AT2/2: pin above MCU_LARGEST_PIN_NUMBER -> assert
 *          - Routine validation:
 *            - RT1/3: bit 0 changes from 0 to 1
 *            - RT2/3: bit 0 stays 1 when already set
 *            - RT3/3: highest valid pin is set without changing other bits
 */
void testIO_PinSet(void) {
    const uint32_t highestPinMask   = (uint32_t)1u << MCU_LARGEST_PIN_NUMBER;
    volatile uint32_t registerValue = 0u;

    /* ======= Assertion tests ============================================= */
    /* ======= AT1/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(IO_PinSet(NULL_PTR, 0u));

    /* ======= AT2/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(IO_PinSet(&registerValue, MCU_LARGEST_PIN_NUMBER + 1u));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/3: Test implementation ================================== */
    /* ======= RT1/3: Call function under test ============================= */
    IO_PinSet(&registerValue, 0u);

    /* ======= RT1/3: Test output verification ============================= */
    TEST_ASSERT_EQUAL(1u, registerValue);

    /* ======= RT2/3: Test implementation ================================== */
    /* ======= RT2/3: Call function under test ============================= */
    IO_PinSet(&registerValue, 0u);

    /* ======= RT2/3: Test output verification ============================= */
    TEST_ASSERT_EQUAL(1u, registerValue);

    /* ======= RT3/3: Test implementation ================================== */
    registerValue = 0x2468ACE0u;

    /* ======= RT3/3: Call function under test ============================= */
    IO_PinSet(&registerValue, MCU_LARGEST_PIN_NUMBER);

    /* ======= RT3/3: Test output verification ============================= */
    TEST_ASSERT_EQUAL((0x2468ACE0u | highestPinMask), registerValue);
}

/**
 * @brief   Test of IO_PinReset
 * @details Cases:
 *          - Argument validation:
 *            - AT1/2: null register address -> assert
 *            - AT2/2: pin above MCU_LARGEST_PIN_NUMBER -> assert
 *          - Routine validation:
 *            - RT1/3: bit 0 changes from 1 to 0
 *            - RT2/3: bit 0 stays 0 when already cleared
 *            - RT3/3: highest valid pin is cleared without changing other bits
 */
void testIO_PinReset(void) {
    const uint32_t highestPinMask   = (uint32_t)1u << MCU_LARGEST_PIN_NUMBER;
    volatile uint32_t registerValue = 1u;

    /* ======= Assertion tests ============================================= */
    /* ======= AT1/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(IO_PinReset(NULL_PTR, 0u));

    /* ======= AT2/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(IO_PinReset(&registerValue, MCU_LARGEST_PIN_NUMBER + 1u));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/3: Test implementation ================================== */
    /* ======= RT1/3: Call function under test ============================= */
    IO_PinReset(&registerValue, 0u);

    /* ======= RT1/3: Test output verification ============================= */
    TEST_ASSERT_EQUAL(0u, registerValue);

    /* ======= RT2/3: Test implementation ================================== */
    /* ======= RT2/3: Call function under test ============================= */
    IO_PinReset(&registerValue, 0u);

    /* ======= RT2/3: Test output verification ============================= */
    TEST_ASSERT_EQUAL(0u, registerValue);

    /* ======= RT3/3: Test implementation ================================== */
    registerValue = (0x2468ACE0u | highestPinMask);

    /* ======= RT3/3: Call function under test ============================= */
    IO_PinReset(&registerValue, MCU_LARGEST_PIN_NUMBER);

    /* ======= RT3/3: Test output verification ============================= */
    TEST_ASSERT_EQUAL(0x2468ACE0u, registerValue);
}

/**
 * @brief   Test of IO_PinGet
 * @details Cases:
 *          - Argument validation:
 *            - AT1/2: null register address -> assert
 *            - AT2/2: pin above MCU_LARGEST_PIN_NUMBER -> assert
 *          - Routine validation:
 *            - RT1/4: bit 0 reads low when cleared
 *            - RT2/4: bit 1 reads high when set
 *            - RT3/4: highest valid pin reads high when set
 *            - RT4/4: highest valid pin reads low when cleared
 */
void testIO_PinGet(void) {
    const uint32_t highestPinMask   = (uint32_t)1u << MCU_LARGEST_PIN_NUMBER;
    volatile uint32_t registerValue = (0x00000002u | highestPinMask);

    /* ======= Assertion tests ============================================= */
    /* ======= AT1/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(IO_PinGet(NULL_PTR, 0u));

    /* ======= AT2/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(IO_PinGet(&registerValue, MCU_LARGEST_PIN_NUMBER + 1u));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/4: Test implementation ================================== */
    /* ======= RT1/4: Call function under test ============================= */
    TEST_ASSERT_EQUAL(STD_PIN_LOW, IO_PinGet(&registerValue, 0u));

    /* ======= RT2/4: Test implementation ================================== */
    /* ======= RT2/4: Call function under test ============================= */
    TEST_ASSERT_EQUAL(STD_PIN_HIGH, IO_PinGet(&registerValue, 1u));

    /* ======= RT3/4: Test implementation ================================== */
    /* ======= RT3/4: Call function under test ============================= */
    TEST_ASSERT_EQUAL(STD_PIN_HIGH, IO_PinGet(&registerValue, MCU_LARGEST_PIN_NUMBER));

    /* ======= RT4/4: Test implementation ================================== */
    registerValue = 0x7FFFFFFFu;

    /* ======= RT4/4: Call function under test ============================= */
    TEST_ASSERT_EQUAL(STD_PIN_LOW, IO_PinGet(&registerValue, MCU_LARGEST_PIN_NUMBER));
}
