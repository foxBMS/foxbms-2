/**
 *
 * @copyright &copy; 2010 - 2024, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * @file    test_led.c
 * @author  foxBMS Team
 * @date    2020-10-05 (date of creation)
 * @updated 2024-12-20 (date of last update)
 * @version v1.8.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Tests for the led module
 * @details Tests SetDebugLed, Trigger and SetToggleTime
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "MockHL_reg_het.h"
#include "Mockio.h"
#include "Mockos.h"

#include "led.h"
#include "test_assert_helper.h"

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("led.c")

TEST_INCLUDE_PATH("../../src/app/driver/io")
TEST_INCLUDE_PATH("../../src/app/driver/led")
TEST_INCLUDE_PATH("../../src/app/task/config")

/*========== Definitions and Implementations for Unit Test ==================*/

/** HET1 GIO register that the Debug LED is connected to. */
#define LED_PORT (hetREG1)
/** Pin of HET1 that the Debug LED is connected to. */
#define LED_PIN (1u)

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/

void testLED_SetDebugLed(void) {
    IO_PinSet_Expect(&LED_PORT->DOUT, LED_PIN);
    LED_SetDebugLed();
}

void testLED_Trigger(void) {
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    TEST_LED_SetOnOffTime(0u);
    TEST_ASSERT_FAIL_ASSERT(LED_Trigger());

    TEST_LED_SetOnOffTime(100u);
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    IO_PinGet_ExpectAndReturn(&LED_PORT->DIN, LED_PIN, STD_PIN_HIGH);
    IO_PinReset_Expect(&LED_PORT->DOUT, LED_PIN);
    LED_Trigger();

    for (uint8_t i = 0; i < 9u; i++) {
        OS_EnterTaskCritical_Expect();
        OS_ExitTaskCritical_Expect();
        IO_PinGet_ExpectAndReturn(&LED_PORT->DIN, LED_PIN, STD_PIN_HIGH);
        IO_PinReset_Expect(&LED_PORT->DOUT, LED_PIN);
        LED_Trigger();
    }

    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    IO_PinGet_ExpectAndReturn(&LED_PORT->DIN, LED_PIN, STD_PIN_LOW);
    IO_PinSet_Expect(&LED_PORT->DOUT, LED_PIN);
    LED_Trigger();
}

void testLED_SetToggleTime(void) {
    TEST_ASSERT_FAIL_ASSERT(LED_SetToggleTime(0u));
    TEST_ASSERT_FAIL_ASSERT(LED_SetToggleTime(101u));

    TEST_LED_SetOnOffTime(0u); /* re-initialize on-off time */
    TEST_ASSERT_PASS_ASSERT(LED_SetToggleTime(100u));
    TEST_ASSERT_EQUAL(100u, TEST_LED_GetOnOffTime());

    TEST_ASSERT_PASS_ASSERT(LED_SetToggleTime(200u));
    TEST_ASSERT_EQUAL(200u, TEST_LED_GetOnOffTime());
}
