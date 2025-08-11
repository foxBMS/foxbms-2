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
 * @file    test_can_cbs_tx_debug-response.c
 * @author  foxBMS Team
 * @date    2022-08-17 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Tests for the CAN driver callbacks
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockcan.h"
#include "Mockcan_helper.h"
#include "Mockfoxmath.h"
#include "Mockmcu.h"
#include "Mockos.h"
#include "Mockrtc.h"
#include "Mockutils.h"

#include "database_cfg.h"

#include "can_cbs_tx_debug-response.h"
#include "can_cfg_tx-async-message-definitions.h"
#include "test_assert_helper.h"
#include "version.h"

#include <stdbool.h>

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("can_cbs_tx_debug-response.c")

TEST_INCLUDE_PATH("../../src/app/driver/can")
TEST_INCLUDE_PATH("../../src/app/driver/can/cbs")
TEST_INCLUDE_PATH("../../src/app/driver/can/cbs/tx-async")
TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/foxmath")
TEST_INCLUDE_PATH("../../src/app/driver/rtc")
TEST_INCLUDE_PATH("../../src/version")

/*========== Definitions and Implementations for Unit Test ==================*/
uint64_t testMessageData[10] = {0u, 1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u, 9u};

uint64_t testSignalData[3u] = {0u, 1u, 2u};

uint8_t testCanDataZeroArray[8u] = {0u};

uint8_t testCanDataFilled[8u] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

const CAN_NODE_s can_node1 = {
    .canNodeRegister = canREG1,
};

const CAN_NODE_s can_node2Isolated = {
    .canNodeRegister = canREG2,
};

/* Dummy for version file implementation */
VER_VERSION_s ver_versionInformation = {
    .underVersionControl     = true,
    .isDirty                 = true,
    .major                   = 13,
    .minor                   = 54,
    .patch                   = 7,
    .distanceFromLastRelease = 17,
    .commitHash              = "Test12deadbeef",
    .gitRemote               = "onTheDarkSideOfTheMoon.git",
};

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/

/**
 * @brief   Testing CANTX_TransmitBmsVersionInfo
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - None
 *          - Routine validation:
 *            - RT1/2: distance not capped, is dirty, is under version control
 *            - RT2/2: distance capped, isn't dirty, isn't under version control
 */
void testCANTX_TransmitBmsVersionInfo(void) {
    /* ======= Routine tests =============================================== */
    /* ======= RT1/2: Test implementation */
    MATH_MinimumOfTwoUint16_t_ExpectAndReturn(
        ver_versionInformation.distanceFromLastRelease, 31u, ver_versionInformation.distanceFromLastRelease);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x00, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[1u], 15u, 8u, ver_versionInformation.major, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[2u]);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[2u], 23u, 8u, ver_versionInformation.minor, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[3u]);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[3u], 31u, 8u, ver_versionInformation.patch, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[4u]);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[4u], 39u, 5u, ver_versionInformation.distanceFromLastRelease, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[5u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[5u], 34u, 1u, 0u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[6u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[6u], 33u, 1u, 1u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[7u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[7u], 32u, 1u, 1u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[8u]);
    /* ======= RT1/2: Call function under test */
    uint64_t testResult = TEST_CANTX_TransmitBmsVersionInfo();

    /* ======= RT1/2: Test output verification */
    TEST_ASSERT_EQUAL(testMessageData[8u], testResult);

    /* ======= RT2/2: Test implementation */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
    uint16_t *tmpDistanceFromLastRelease = &ver_versionInformation.distanceFromLastRelease;
    *tmpDistanceFromLastRelease          = 42u;
    bool *tmpIsDirty                     = &ver_versionInformation.isDirty;
    *tmpIsDirty                          = false;
    bool *tmpUnderVersionControl         = &ver_versionInformation.underVersionControl;
    *tmpUnderVersionControl              = false;
#pragma GCC diagnostic pop

    MATH_MinimumOfTwoUint16_t_ExpectAndReturn(42u, 31u, 31u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x00, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[1u], 15u, 8u, ver_versionInformation.major, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[2u]);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[2u], 23u, 8u, ver_versionInformation.minor, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[3u]);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[3u], 31u, 8u, ver_versionInformation.patch, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[4u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[4u], 39u, 5u, 31u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[5u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[5u], 34u, 1u, 1u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[6u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[6u], 33u, 1u, 0u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[7u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[7u], 32u, 1u, 0u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[8u]);

    /* ======= RT2/2: Call function under test */
    testResult = TEST_CANTX_TransmitBmsVersionInfo();

    /* ======= RT2/2: Test output verification */
    TEST_ASSERT_EQUAL(testMessageData[8u], testResult);

    *tmpIsDirty             = true;
    *tmpUnderVersionControl = true;
}

/**
 * @brief   Testing CANTX_TransmitMcuUniqueDieId
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - None
 *          - Routine validation:
 *            - RT1/1: Message data set as expected
 */
void testCANTX_TransmitMcuUniqueDieId(void) {
    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    MCU_GetDeviceRegister_ExpectAndReturn(0x12345u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x01, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[1u], 15u, 32u, 0x12345u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[2u]);

    /* ======= RT1/1: Call function under test */
    uint64_t testResult = TEST_CANTX_TransmitMcuUniqueDieId();
    /* ======= RT1/1: Test output verification */
    TEST_ASSERT_EQUAL(testMessageData[2u], testResult);
}

/**
 * @brief   Testing CANTX_TransmitMcuLotNumber
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - None
 *          - Routine validation:
 *            - RT1/1: Message data set as expected
 */
void testCANTX_TransmitMcuLotNumber(void) {
    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    MCU_GetDieIdHigh_ExpectAndReturn(0x23456u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x02, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[1u], 15u, 32u, 0x23456u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[2u]);

    /* ======= RT1/1: Call function under test */
    uint64_t testResult = TEST_CANTX_TransmitMcuLotNumber();
    /* ======= RT1/1: Test output verification */
    TEST_ASSERT_EQUAL(testMessageData[2u], testResult);
}

/**
 * @brief   Testing CANTX_TransmitMcuWaferInformation
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - None
 *          - Routine validation:
 *            - RT1/1: Message data set as expected
 */
void testCANTX_TransmitMcuWaferInformation(void) {
    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    MCU_GetDieIdLow_ExpectAndReturn(0x11AAABBBu);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x03, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[1u], 15u, 8u, 0x11u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[2u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[2u], 27u, 12u, 0xBBBu, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[3u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[3u], 23u, 12u, 0xAAAu, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[4u]);

    /* ======= RT1/1: Call function under test */
    uint64_t testResult = TEST_CANTX_TransmitMcuWaferInformation();
    /* ======= RT1/1: Test output verification */
    TEST_ASSERT_EQUAL(testMessageData[4u], testResult);
}

/**
 * @brief   Testing CANTX_TransmitBootMagic
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: invalid data
 *          - Routine validation:
 *            - RT1/2: set bit mask start
 *            - RT2/2: set bit mask end
 */
void testCANTX_TransmitBootMagic(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_TransmitBootMagic(0u));
    /* ======= Routine tests =============================================== */
    /* ======= RT1/2: Test implementation */
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x0F, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[1u], 15u, 56u, 0xFEFEFEFEFEFEFEuLL, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[2u]);
    /* ======= RT1/2: Call function under test */
    uint64_t testResult = TEST_CANTX_TransmitBootMagic(0xFEFEFEFEFEFEFEuLL);
    /* ======= RT1/2: Test output verification */
    TEST_ASSERT_EQUAL(testMessageData[2u], testResult);
    /* ======= RT2/2: Test implementation */
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x0F, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[1u], 15u, 56u, 0x01010101010101uLL, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[2u]);
    /* ======= RT2/2: Call function under test */
    testResult = TEST_CANTX_TransmitBootMagic(0x01010101010101uLL);
    /* ======= RT2/2: Test output verification */
    TEST_ASSERT_EQUAL(testMessageData[2u], testResult);
}

/**
 * @brief   Testing CANTX_TransmitBootMagicStart
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - None
 *          - Routine validation:
 *            - RT1/1: set bit mask start
 */
void testCANTX_TransmitBootMagicStart(void) {
    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x0F, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 15u, 56u, 0xFEFEFEFEFEFEFEuLL, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    /* ======= RT1/1: Call function under test */
    uint64_t testResult = TEST_CANTX_TransmitBootMagicStart();
    /* ======= RT1/1: Test output verification */
    TEST_ASSERT_EQUAL(testMessageData[1u], testResult);
}

/**
 * @brief   Testing CANTX_TransmitBootMagicEnd
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - None
 *          - Routine validation:
 *            - RT1/1: set bit mask end
 */
void testCANTX_TransmitBootMagicEnd(void) {
    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x0F, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 15u, 56u, 0x01010101010101uLL, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    /* ======= RT1/1: Call function under test */
    uint64_t testResult = TEST_CANTX_TransmitBootMagicEnd();
    /* ======= RT1/1: Test output verification */
    TEST_ASSERT_EQUAL(testMessageData[1u], testResult);
}

/**
 * @brief   Testing CANTX_TransmitBootTimeStamp
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - None
 *          - Routine validation:
 *            - RT1/1: Sets message data as expected
 */
void testCANTX_DebugResponseBootTimestamp(void) {
    /* ======= Routine tests =============================================== */
    RTC_TIME_DATA_s testRtcTime = {
        .hundredthOfSeconds = 69u,
        .seconds            = 7u,
        .minutes            = 14u,
        .hours              = 3u,
        .weekday            = 2u,
        .day                = 19u,
        .month              = 1u,
        .year               = 38u,
    };

    /* ======= RT1/1: Test implementation */
    RTC_GetSystemStartUpTime_ExpectAndReturn(testRtcTime);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x0Eu, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[1u], 15u, 7u, 0x45u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[2u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[2u], 8u, 6u, 0x07u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[3u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[3u], 18u, 6u, 0x0Eu, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[4u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[4u], 28u, 5u, 0x03u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[5u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[5u], 39u, 3u, 0x02u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[6u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[6u], 36u, 5u, 0x13u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[7u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[7u], 47u, 4u, 0x01u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[8u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[8u], 43u, 7u, 0x26u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[9u]);
    /* ======= RT1/1: Call function under test */
    uint64_t testResult = TEST_CANTX_TransmitBootTimeStamp();
    /* ======= RT1/1: Test output verification */
    TEST_ASSERT_EQUAL(testMessageData[9u], testResult);
}

/**
 * @brief   Testing CANTX_TransmitRtcTime
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - None
 *          - Routine validation:
 *            - RT1/1: Sets message data as expected
 */
void testCANTX_TransmitRtcTime(void) {
    /* ======= Routine tests =============================================== */
    RTC_TIME_DATA_s testRtcTime = {
        .hundredthOfSeconds = 69u,
        .seconds            = 07u,
        .minutes            = 14u,
        .hours              = 3u,
        .weekday            = 2u,
        .day                = 19u,
        .month              = 1u,
        .year               = 38u,
        .requestFlag        = 0u,
    };

    /* ======= RT1/1: Test implementation */
    RTC_GetSystemTimeRtcFormat_ExpectAndReturn(testRtcTime);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x04u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[1u], 15u, 7u, 0x45u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[2u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[2u], 8u, 6u, 0x07u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[3u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[3u], 18u, 6u, 0x0Eu, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[4u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[4u], 28u, 5u, 0x03u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[5u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[5u], 39u, 3u, 0x02u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[6u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[6u], 36u, 5u, 0x13u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[7u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[7u], 47u, 4u, 0x01u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[8u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[8u], 43u, 7u, 0x26u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[9u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[9u], 52u, 2u, 0x00u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[10u]);
    /* ======= RT1/1: Call function under test */
    uint64_t testResult = TEST_CANTX_TransmitRtcTime();
    /* ======= RT1/1: Test output verification */
    TEST_ASSERT_EQUAL(testMessageData[10u], testResult);
}

/**
 * @brief   Testing CANTX_TransmitUptime
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - None
 *          - Routine validation:
 *            - RT1/1: Sets message data as expected
 */
void testCANTX_TransmitUptime(void) {
    /* ======= Routine tests =============================================== */
    OS_TIMER_s testOsTime = {
        .timer_100ms = 22u,
        .timer_10ms  = 16u,
        .timer_1ms   = 69u,
        .timer_sec   = 07u,
        .timer_min   = 14u,
        .timer_h     = 3u,
        .timer_d     = 19u,
    };

    /* ======= RT1/1: Test implementation */
    OS_GetOsTimer_ExpectAndReturn(testOsTime);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x07u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[1u], 15u, 10u, 0x45u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[2u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[2u], 21u, 7u, 0x10u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[3u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[3u], 30u, 4u, 0x16u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[4u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[4u], 26u, 6u, 0x07u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[5u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[5u], 36u, 6u, 0x0Eu, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[6u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[6u], 46u, 5u, 0x03u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[7u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[7u], 41u, 5u, 0x13u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[8u]);

    /* ======= RT1/1: Call function under test */
    uint64_t testResult = TEST_CANTX_TransmitUptime();
    /* ======= RT1/1: Test output verification */
    TEST_ASSERT_EQUAL(testMessageData[8u], testResult);
}

/**
 * @brief   Testing CANTX_TransmitCommitHash
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - None
 *          - Routine validation:
 *            - RT1/2: commit hash high NOT queued successfully
 *            - RT2/2: messages sent successfully
 */
void testCANTX_TransmitCommitHash(void) {
    /* ======= Routine tests =============================================== */
    /* ======= RT1/2: Test implementation */
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x05u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    UTIL_ExtractCharactersFromString_Expect(&testSignalData[0u], ver_versionInformation.commitHash, 14u, 0u, 7u);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 56u, testSignalData[0u], CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1], testCanDataZeroArray, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_1, CANTX_DEBUG_RESPONSE_ID, CAN_STANDARD_IDENTIFIER_11_BIT, testCanDataZeroArray, STD_NOT_OK);
    /* ======= RT1/2: Call function under test */
    uint64_t testResult = TEST_CANTX_TransmitCommitHash();
    /* ======= RT1/2: Test output verification */
    TEST_ASSERT_EQUAL(STD_NOT_OK, testResult);

    /* ======= RT2/2: Test implementation */
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x05u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    UTIL_ExtractCharactersFromString_Expect(&testSignalData[0u], ver_versionInformation.commitHash, 14u, 0u, 7u);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 56u, testSignalData[0u], CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataZeroArray, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_1, CANTX_DEBUG_RESPONSE_ID, CAN_STANDARD_IDENTIFIER_11_BIT, testCanDataZeroArray, STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x06u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    UTIL_ExtractCharactersFromString_Expect(&testSignalData[0u], ver_versionInformation.commitHash, 14u, 7u, 7u);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 56u, testSignalData[0u], CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataZeroArray, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_1, CANTX_DEBUG_RESPONSE_ID, CAN_STANDARD_IDENTIFIER_11_BIT, testCanDataZeroArray, STD_OK);
    /* ======= RT2/2: Call function under test */
    testResult = TEST_CANTX_TransmitCommitHash();
    /* ======= RT2/2: Test output verification */
    TEST_ASSERT_EQUAL(STD_OK, testResult);
}

/**
 * @brief   Testing CANTX_TransmitCommitHashHigh
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - None
 *          - Routine validation:
 *            - RT1/1: sets message data as expected
 */
void testCANTX_TransmitCommitHashHigh(void) {
    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x05u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    UTIL_ExtractCharactersFromString_Expect(&testSignalData[0u], ver_versionInformation.commitHash, 14u, 0u, 7u);
    UTIL_ExtractCharactersFromString_ReturnThruPtr_pExtractedCharacters(&testSignalData[1u]);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[1u], 15u, 56u, testSignalData[1u], CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[2u]);
    /* ======= RT1/1: Call function under test */
    uint64_t testResult = TEST_CANTX_TransmitCommitHashHigh();
    /* ======= RT1/1: Test output verification */
    TEST_ASSERT_EQUAL(testMessageData[2u], testResult);
}

/**
 * @brief   Testing CANTX_TransmitCommitHashLow
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - None
 *          - Routine validation:
 *            - RT1/1: sets message data as expected
 */
void testCANTX_TransmitCommitHashLow(void) {
    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x06u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    UTIL_ExtractCharactersFromString_Expect(&testSignalData[0u], ver_versionInformation.commitHash, 14u, 7u, 7u);
    UTIL_ExtractCharactersFromString_ReturnThruPtr_pExtractedCharacters(&testSignalData[1u]);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[1u], 15u, 56u, testSignalData[1u], CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[2u]);
    /* ======= RT1/1: Call function under test */
    uint64_t testResult = TEST_CANTX_TransmitCommitHashLow();
    /* ======= RT1/1: Test output verification */
    TEST_ASSERT_EQUAL(testMessageData[2u], testResult);
}

/**
 * @brief   Testing CANTX_DebugResponseSendMessage
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - None
 *          - Routine validation:
 *            - RT1/1: sends message as expected
 */
void testCANTX_DebugResponseSendMessage(void) {
    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    CAN_TxSetCanDataWithMessageData_Expect(0x0123456789ABCDEF, testCanDataZeroArray, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_1, CANTX_DEBUG_RESPONSE_ID, CAN_STANDARD_IDENTIFIER_11_BIT, testCanDataFilled, STD_OK);
    /* ======= RT1/1: Call function under test */
    STD_RETURN_TYPE_e testResult = TEST_CANTX_DebugResponseSendMessage(0x0123456789ABCDEF);
    /* ======= RT1/1: Test output verification */
    TEST_ASSERT_EQUAL(STD_OK, testResult);
}

/**
 * @brief   Testing CANTX_DebugResponse
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: action undefined
 *          - Routine validation:
 *            - RT1/9: BMS_VERSION_INFO
 *            - RT2/9: MCU_UNIQUE_DIE_ID
 *            - RT3/9: MCU_LOT_NUMBER
 *            - RT4/9: MCU_WAFER_INFORMATION
 *            - RT5/9: BOOT_MAGIC_START
 *            - RT6/9: BOOT_MAGIC_END
 *            - RT7/9: RTC_TIME
 *            - RT8/9: UPTIME
 *            - RT8/9: COMMIT_HASH
 */
void testCANTX_DebugResponse(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(CANTX_DebugResponse(UINT16_MAX));
    /* ======= Routine tests =============================================== */
    /* ======= RT1/9: Test implementation */
    MATH_MinimumOfTwoUint16_t_ExpectAndReturn(
        ver_versionInformation.distanceFromLastRelease, 31u, ver_versionInformation.distanceFromLastRelease);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x00, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 8u, ver_versionInformation.major, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 23u, 8u, ver_versionInformation.minor, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 31u, 8u, ver_versionInformation.patch, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 39u, 5u, ver_versionInformation.distanceFromLastRelease, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 34u, 1u, 1u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 33u, 1u, 1u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 32u, 1u, 1u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataZeroArray, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_1, CANTX_DEBUG_RESPONSE_ID, CAN_STANDARD_IDENTIFIER_11_BIT, testCanDataZeroArray, STD_OK);
    /* ======= RT1/9: Call function under test */
    STD_RETURN_TYPE_e testResult = CANTX_DebugResponse(CANTX_DEBUG_RESPONSE_TRANSMIT_BMS_VERSION_INFO);
    /* ======= RT1/9: Test output verification */
    TEST_ASSERT_EQUAL(STD_OK, testResult);

    /* ======= RT2/9: Test implementation */
    MCU_GetDeviceRegister_ExpectAndReturn(0x12345u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x01, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 32u, 0x12345u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataZeroArray, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_1, CANTX_DEBUG_RESPONSE_ID, CAN_STANDARD_IDENTIFIER_11_BIT, testCanDataZeroArray, STD_OK);
    /* ======= RT2/9: Call function under test */
    testResult = CANTX_DebugResponse(CANTX_DEBUG_RESPONSE_TRANSMIT_MCU_UNIQUE_DIE_ID);
    /* ======= RT2/9: Test output verification */
    TEST_ASSERT_EQUAL(STD_OK, testResult);

    /* ======= RT3/9: Test implementation */
    MCU_GetDieIdHigh_ExpectAndReturn(0x23456u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x02, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 32u, 0x23456u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataZeroArray, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_1, CANTX_DEBUG_RESPONSE_ID, CAN_STANDARD_IDENTIFIER_11_BIT, testCanDataZeroArray, STD_OK);
    /* ======= RT3/9: Call function under test */
    testResult = CANTX_DebugResponse(CANTX_DEBUG_RESPONSE_TRANSMIT_MCU_LOT_NUMBER);
    /* ======= RT3/9: Test output verification */
    TEST_ASSERT_EQUAL(STD_OK, testResult);

    /* ======= RT4/9: Test implementation */
    MCU_GetDieIdLow_ExpectAndReturn(0x11AAABBBu);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x03, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 15u, 8u, 0x11u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 27u, 12u, 0xBBBu, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 23u, 12u, 0xAAAu, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataZeroArray, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_1, CANTX_DEBUG_RESPONSE_ID, CAN_STANDARD_IDENTIFIER_11_BIT, testCanDataZeroArray, STD_OK);
    /* ======= RT4/9: Call function under test */
    testResult = CANTX_DebugResponse(CANTX_DEBUG_RESPONSE_TRANSMIT_MCU_WAFER_INFORMATION);
    /* ======= RT4/9: Test output verification */
    TEST_ASSERT_EQUAL(STD_OK, testResult);

    /* ======= RT5/9: Test implementation */
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x0F, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 15u, 56u, 0xFEFEFEFEFEFEFEuLL, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataZeroArray, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_1, CANTX_DEBUG_RESPONSE_ID, CAN_STANDARD_IDENTIFIER_11_BIT, testCanDataZeroArray, STD_OK);
    /* ======= RT5/9: Call function under test */
    testResult = CANTX_DebugResponse(CANTX_DEBUG_RESPONSE_TRANSMIT_BOOT_MAGIC_START);
    /* ======= RT5/9: Test output verification */
    TEST_ASSERT_EQUAL(STD_OK, testResult);

    /* ======= RT6/9: Test implementation */
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x0F, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 15u, 56u, 0x01010101010101uLL, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataZeroArray, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_1, CANTX_DEBUG_RESPONSE_ID, CAN_STANDARD_IDENTIFIER_11_BIT, testCanDataZeroArray, STD_OK);
    /* ======= RT6/9: Call function under test */
    testResult = CANTX_DebugResponse(CANTX_DEBUG_RESPONSE_TRANSMIT_BOOT_MAGIC_END);
    /* ======= RT6/9: Test output verification */
    TEST_ASSERT_EQUAL(STD_OK, testResult);

    /* ======= RT7/9: Test implementation */
    RTC_TIME_DATA_s testRtcStartUpTime = {
        .hundredthOfSeconds = 69u,
        .seconds            = 07u,
        .minutes            = 14u,
        .hours              = 3u,
        .weekday            = 2u,
        .day                = 19u,
        .month              = 1u,
        .year               = 38u,
    };
    RTC_GetSystemStartUpTime_ExpectAndReturn(testRtcStartUpTime);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x0Eu, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 15u, 7u, 0x45u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 8u, 6u, 0x07u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 18u, 6u, 0x0Eu, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 28u, 5u, 0x03u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 39u, 3u, 0x02u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 36u, 5u, 0x13u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 47u, 4u, 0x01u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 43u, 7u, 0x26u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataZeroArray, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_1, CANTX_DEBUG_RESPONSE_ID, CAN_STANDARD_IDENTIFIER_11_BIT, testCanDataZeroArray, STD_OK);
    /* ======= RT8/10: Call function under test */
    testResult = CANTX_DebugResponse(CANTX_DEBUG_RESPONSE_TRANSMIT_BOOT_TIMESTAMP);
    /* ======= RT8/10: Test output verification */
    TEST_ASSERT_EQUAL(STD_OK, testResult);

    /* ======= RT8/9: Test implementation */
    RTC_TIME_DATA_s testRtcTime = {
        .hundredthOfSeconds = 69u,
        .seconds            = 07u,
        .minutes            = 14u,
        .hours              = 3u,
        .weekday            = 2u,
        .day                = 19u,
        .month              = 1u,
        .year               = 38u,
        .requestFlag        = 0u,
    };
    RTC_GetSystemTimeRtcFormat_ExpectAndReturn(testRtcTime);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x04u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 15u, 7u, 0x45u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 8u, 6u, 0x07u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 18u, 6u, 0x0Eu, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 28u, 5u, 0x03u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 39u, 3u, 0x02u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 36u, 5u, 0x13u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 47u, 4u, 0x01u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 43u, 7u, 0x26u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 52u, 2u, 0x00u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataZeroArray, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_1, CANTX_DEBUG_RESPONSE_ID, CAN_STANDARD_IDENTIFIER_11_BIT, testCanDataZeroArray, STD_OK);
    /* ======= RT8/10: Call function under test */
    testResult = CANTX_DebugResponse(CANTX_DEBUG_RESPONSE_TRANSMIT_RTC_TIME);
    /* ======= RT8/10: Test output verification */
    TEST_ASSERT_EQUAL(STD_OK, testResult);

    /* ======= RT8/9: Test implementation */
    OS_TIMER_s testOsTime = {
        .timer_100ms = 22u,
        .timer_10ms  = 16u,
        .timer_1ms   = 69u,
        .timer_sec   = 07u,
        .timer_min   = 14u,
        .timer_h     = 3u,
        .timer_d     = 19u,
    };

    /* ======= RT8/9: Test implementation */
    OS_GetOsTimer_ExpectAndReturn(testOsTime);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x07u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 15u, 10u, 0x45u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 21u, 7u, 0x10u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 30u, 4u, 0x16u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 26u, 6u, 0x07u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 36u, 6u, 0x0Eu, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 46u, 5u, 0x03u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 41u, 5u, 0x13u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataZeroArray, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_1, CANTX_DEBUG_RESPONSE_ID, CAN_STANDARD_IDENTIFIER_11_BIT, testCanDataZeroArray, STD_OK);
    /* ======= RT8/9: Call function under test */
    testResult = CANTX_DebugResponse(CANTX_DEBUG_RESPONSE_TRANSMIT_UPTIME);
    /* ======= RT8/9: Test output verification */
    TEST_ASSERT_EQUAL(STD_OK, testResult);

    /* ======= RT9/9: Test implementation */
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x05u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    UTIL_ExtractCharactersFromString_Expect(&testSignalData[0u], ver_versionInformation.commitHash, 14u, 0u, 7u);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 56u, testSignalData[0u], CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataZeroArray, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_1, CANTX_DEBUG_RESPONSE_ID, CAN_STANDARD_IDENTIFIER_11_BIT, testCanDataZeroArray, STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x06u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    UTIL_ExtractCharactersFromString_Expect(&testSignalData[0u], ver_versionInformation.commitHash, 14u, 7u, 7u);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 56u, testSignalData[0u], CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataZeroArray, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_1, CANTX_DEBUG_RESPONSE_ID, CAN_STANDARD_IDENTIFIER_11_BIT, testCanDataZeroArray, STD_NOT_OK);
    /* ======= RT9/9: Call function under test */
    testResult = CANTX_DebugResponse(CANTX_DEBUG_RESPONSE_TRANSMIT_COMMIT_HASH);
    /* ======= RT9/9: Test output verification */
    TEST_ASSERT_EQUAL(STD_NOT_OK, testResult);
}
