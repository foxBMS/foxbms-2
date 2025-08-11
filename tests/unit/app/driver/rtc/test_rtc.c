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
 * @file    test_rtc.c
 * @author  foxBMS Team
 * @date    2020-04-01 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Tests for the rtc module
 * @details TODO
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "MockHL_i2c.h"
#include "MockHL_sys_dma.h"
#include "Mockdatabase.h"
#include "Mockdiag.h"
#include "Mocki2c.h"
#include "Mockos.h"

#include "rtc.h"
#include "test_assert_helper.h"

#include <stdbool.h>
#include <time.h>

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("rtc.c")

TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/foxmath")
TEST_INCLUDE_PATH("../../src/app/driver/i2c")
TEST_INCLUDE_PATH("../../src/app/driver/rtc")
TEST_INCLUDE_PATH("../../src/app/engine/diag")
TEST_INCLUDE_PATH("../../src/app/task/config")
TEST_INCLUDE_PATH("../../src/app/task/ftask")

/*========== Definitions and Implementations for Unit Test ==================*/

OS_QUEUE ftsk_rtcSetTimeQueue = NULL_PTR;

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/

void testRTC_Trigger(void) {
    uint8_t expectedI2cWriteBuffer[RTC_MAX_I2C_TRANSACTION_SIZE_IN_BYTES] = {0u};
    uint8_t expectedI2cReadBuffer[RTC_MAX_I2C_TRANSACTION_SIZE_IN_BYTES]  = {0u};
    uint32_t currentTime                                                  = 0u;

    OS_GetTickCount_ExpectAndReturn(0u);
    OS_GetTickCount_ExpectAndReturn(0u);

    expectedI2cWriteBuffer[0u] = 6u;
    I2C_Write_ExpectAndReturn(RTC_I2C_INTERFACE, RTC_I2C_ADDRESS, 1u, expectedI2cWriteBuffer, STD_OK);
    I2C_ReadDma_ExpectAndReturn(
        RTC_I2C_INTERFACE, RTC_I2C_ADDRESS, RTC_NUMBER_OF_TIME_DATA_BYTES, expectedI2cReadBuffer, STD_OK);
    DIAG_Handler_ExpectAndReturn(DIAG_ID_I2C_RTC_ERROR, DIAG_EVENT_OK, DIAG_SYSTEM, 0u, STD_OK);
    DIAG_Handler_ExpectAndReturn(DIAG_ID_RTC_CLOCK_INTEGRITY_ERROR, DIAG_EVENT_OK, DIAG_SYSTEM, 0u, STD_OK);
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    OS_DelayTaskUntil_Expect(&currentTime, 2u);
    DIAG_Handler_ExpectAndReturn(DIAG_ID_I2C_RTC_ERROR, DIAG_EVENT_OK, DIAG_SYSTEM, 0u, STD_OK);

    RTC_Trigger();
}

void testRTC_Initialize(void) {
    uint8_t expectedI2cWriteBuffer[RTC_MAX_I2C_TRANSACTION_SIZE_IN_BYTES] = {0};
    uint8_t expectedI2cReadBuffer[RTC_MAX_I2C_TRANSACTION_SIZE_IN_BYTES]  = {0};
    uint32_t currentTime                                                  = 0u;

    OS_GetTickCount_ExpectAndReturn(0u);
    expectedI2cWriteBuffer[0u] = 2u;
    I2C_WriteDma_ExpectAndReturn(
        RTC_I2C_INTERFACE,
        RTC_I2C_ADDRESS,
        RTC_WRITE_REGISTER_I2C_TRANSACTION_SIZE_IN_BYTES,
        expectedI2cWriteBuffer,
        STD_OK);
    OS_DelayTaskUntil_Expect(&currentTime, 2u);
    DIAG_Handler_ExpectAndReturn(DIAG_ID_I2C_RTC_ERROR, DIAG_EVENT_OK, DIAG_SYSTEM, 0u, STD_OK);
    RTC_Initialize();

    OS_GetTickCount_ExpectAndReturn(0u);
    expectedI2cWriteBuffer[0u] = 19u;
    I2C_WriteDma_ExpectAndReturn(
        RTC_I2C_INTERFACE,
        RTC_I2C_ADDRESS,
        RTC_WRITE_REGISTER_I2C_TRANSACTION_SIZE_IN_BYTES,
        expectedI2cWriteBuffer,
        STD_OK);
    OS_GetTickCount_ExpectAndReturn(0u);
    OS_DelayTaskUntil_Expect(&currentTime, 2u);
    DIAG_Handler_ExpectAndReturn(DIAG_ID_I2C_RTC_ERROR, DIAG_EVENT_OK, DIAG_SYSTEM, 0u, STD_OK);
    RTC_Initialize();

    OS_GetTickCount_ExpectAndReturn(0u);
    expectedI2cWriteBuffer[0u] = 19u;
    I2C_WriteDma_ExpectAndReturn(RTC_I2C_INTERFACE, RTC_I2C_ADDRESS, 1u, expectedI2cWriteBuffer, STD_OK);
    I2C_Read_ExpectAndReturn(RTC_I2C_INTERFACE, RTC_I2C_ADDRESS, 1u, expectedI2cReadBuffer, STD_OK);
    OS_DelayTaskUntil_Expect(&currentTime, 2u);
    DIAG_Handler_ExpectAndReturn(DIAG_ID_I2C_RTC_ERROR, DIAG_EVENT_OK, DIAG_SYSTEM, 0u, STD_OK);
    RTC_Initialize();
}

void testRTC_InitializeSystemTimeWithRtc(void) {
    uint8_t expectedI2cWriteBuffer[RTC_MAX_I2C_TRANSACTION_SIZE_IN_BYTES] = {0u};
    uint8_t expectedI2cReadBuffer[RTC_MAX_I2C_TRANSACTION_SIZE_IN_BYTES]  = {0u};

    expectedI2cWriteBuffer[0u] = 6u;
    I2C_Write_ExpectAndReturn(RTC_I2C_INTERFACE, RTC_I2C_ADDRESS, 1u, expectedI2cWriteBuffer, STD_OK);
    I2C_ReadDma_ExpectAndReturn(
        RTC_I2C_INTERFACE, RTC_I2C_ADDRESS, RTC_NUMBER_OF_TIME_DATA_BYTES, expectedI2cReadBuffer, STD_OK);
    DIAG_Handler_ExpectAndReturn(DIAG_ID_I2C_RTC_ERROR, DIAG_EVENT_OK, DIAG_SYSTEM, 0u, STD_OK);
    DIAG_Handler_ExpectAndReturn(DIAG_ID_RTC_CLOCK_INTEGRITY_ERROR, DIAG_EVENT_OK, DIAG_SYSTEM, 0u, STD_OK);
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();

    RTC_InitializeSystemTimeWithRtc();
}

void testRTC_IncrementSystemTime(void) {
    /* save previous time to reset after test */
    RTC_SYSTEM_TIMER_EPOCH_s previousValue = TEST_RTC_GetRtcSystemTime();

    /* ======= Routine tests =============================================== */
    RTC_SYSTEM_TIMER_EPOCH_s rtcTime = {
        .secondsSinceEpoch = 0u,
        .milliseconds      = 0u,
    };
    /* ======= RT1/2: Test implementation */
    /* milliseconds are 0 */
    TEST_RTC_SetRtcSystemTime(rtcTime);

    /* ======= RT1/2: call function under test */
    RTC_IncrementSystemTime();

    /* ======= RT1/2: test output verification */
    rtcTime = TEST_RTC_GetRtcSystemTime();
    TEST_ASSERT_EQUAL(rtcTime.milliseconds, 1u);
    TEST_ASSERT_EQUAL(rtcTime.secondsSinceEpoch, 0u);

    /* ======= RT2/2: Test implementation */
    /* set to 999 milliseconds */
    rtcTime.secondsSinceEpoch = 0u;
    rtcTime.milliseconds      = 999u;
    TEST_RTC_SetRtcSystemTime(rtcTime);

    /* ======= RT2/2: call function under test */
    RTC_IncrementSystemTime();

    /* ======= RT2/2: test output verification */
    rtcTime = TEST_RTC_GetRtcSystemTime();
    TEST_ASSERT_EQUAL(rtcTime.milliseconds, 0u);
    TEST_ASSERT_EQUAL(rtcTime.secondsSinceEpoch, 1u);

    /* reset to old value */
    TEST_RTC_SetRtcSystemTime(previousValue);
}

void testRTC_SetSystemTimeRtcFormat(void) {
    RTC_TIME_DATA_s rtcTime = {
        .hundredthOfSeconds = 9u,
        .seconds            = 22u,
        .minutes            = 11u,
        .hours              = 12u,
        .day                = 13u,
        .weekday            = 1u,
        .year               = 55u,
    };

    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();

    RTC_SetSystemTimeRtcFormat(&rtcTime);
}

void testRTC_SetRtcRequestFlag(void) {
    RTC_SetRtcRequestFlag(1u);
}

void testRTC_GetSystemTimeRtcFormat(void) {
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();

    RTC_GetSystemTimeRtcFormat();
}

void testRTC_GetSystemStartUpTime(void) {
    RTC_GetSystemStartUpTime();
}

void testRTC_IsRtcModuleInitialized(void) {
    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    bool previousRtcModuleInitializationStatus = RTC_IsRtcModuleInitialized();

    TEST_RTC_SetRtcModuleInitializationStatus(true);
    bool rtcInitializationStatus = RTC_IsRtcModuleInitialized();
    TEST_ASSERT_TRUE(rtcInitializationStatus);

    TEST_RTC_SetRtcModuleInitializationStatus(previousRtcModuleInitializationStatus);
}
