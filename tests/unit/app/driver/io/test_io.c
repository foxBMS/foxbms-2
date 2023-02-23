/**
 *
 * @copyright &copy; 2010 - 2023, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * @file    test_io.c
 * @author  foxBMS Team
 * @date    2020-06-10 (date of creation)
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Tests for the io module
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockmcu.h"

#include "io.h"
#include "test_assert_helper.h"

TEST_FILE("io.c")

/*========== Definitions and Implementations for Unit Test ==================*/

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/

void testIO_SetPinDirectionToOutput(void) {
    volatile uint32_t registerValue = 1u;
    TEST_ASSERT_FAIL_ASSERT(IO_SetPinDirectionToOutput(NULL_PTR, 0u));
    TEST_ASSERT_FAIL_ASSERT(IO_SetPinDirectionToOutput(&registerValue, MCU_LARGEST_PIN_NUMBER + 1u));

    /* 0 -> 1 */
    IO_SetPinDirectionToOutput(&registerValue, 0u);
    TEST_ASSERT_EQUAL(1u, registerValue);

    /* 1 -> 1 */
    IO_SetPinDirectionToOutput(&registerValue, 0u);
    TEST_ASSERT_EQUAL(1u, registerValue);
}

void testIO_SetPinDirectionToInput(void) {
    volatile uint32_t registerValue = 1u;
    TEST_ASSERT_FAIL_ASSERT(IO_SetPinDirectionToInput(NULL_PTR, 0u));
    TEST_ASSERT_FAIL_ASSERT(IO_SetPinDirectionToInput(&registerValue, MCU_LARGEST_PIN_NUMBER + 1u));

    /* 1 -> 0 */
    IO_SetPinDirectionToInput(&registerValue, 0u);
    TEST_ASSERT_EQUAL(0u, registerValue);

    /* 0 -> 0 */
    IO_SetPinDirectionToInput(&registerValue, 0u);
    TEST_ASSERT_EQUAL(0u, registerValue);
}

void testIO_PinSet(void) {
    volatile uint32_t registerValue = 0u;
    TEST_ASSERT_FAIL_ASSERT(IO_PinSet(NULL_PTR, 0u));
    TEST_ASSERT_FAIL_ASSERT(IO_PinSet(&registerValue, MCU_LARGEST_PIN_NUMBER + 1u));

    /* 0 -> 1 */
    IO_PinSet(&registerValue, 0u);
    TEST_ASSERT_EQUAL(1u, registerValue);

    /* 1 -> 1 */
    IO_PinSet(&registerValue, 0u);
    TEST_ASSERT_EQUAL(1u, registerValue);
}

void testIO_PinReset(void) {
    volatile uint32_t registerValue = 1u;
    TEST_ASSERT_FAIL_ASSERT(IO_PinReset(NULL_PTR, 0u));
    TEST_ASSERT_FAIL_ASSERT(IO_PinReset(&registerValue, MCU_LARGEST_PIN_NUMBER + 1u));

    /* 1 -> 0 */
    IO_PinReset(&registerValue, 0u);
    TEST_ASSERT_EQUAL(0u, registerValue);

    /* 0 -> 0 */
    IO_PinReset(&registerValue, 0u);
    TEST_ASSERT_EQUAL(0u, registerValue);
}

void testIO_PinGet(void) {
    volatile uint32_t registerValue = 1u;
    /* invalid argument tests */
    TEST_ASSERT_FAIL_ASSERT(IO_PinGet(NULL_PTR, 0u));
    TEST_ASSERT_FAIL_ASSERT(IO_PinGet(&registerValue, MCU_LARGEST_PIN_NUMBER + 1u));

    TEST_ASSERT_EQUAL(STD_PIN_LOW, IO_PinGet(&registerValue, 1u));
    TEST_ASSERT_EQUAL(STD_PIN_HIGH, IO_PinGet(&registerValue, 0u));
}
