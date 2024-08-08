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
 * @file    test_can_cbs_tx_debug-response.c
 * @author  foxBMS Team
 * @date    2022-08-17 (date of creation)
 * @updated 2024-08-08 (date of last update)
 * @version v1.7.0
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
#include "Mockrtc.h"

#include "database_cfg.h"
#include "version_cfg.h"

#include "can_cbs_tx_debug-response.h"
#include "can_cfg_tx-async-message-definitions.h"

#include <stdbool.h>

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("can_cbs_tx_debug-response.c")

TEST_INCLUDE_PATH("../../src/app/driver/can")
TEST_INCLUDE_PATH("../../src/app/driver/can/cbs")
TEST_INCLUDE_PATH("../../src/app/driver/can/cbs/tx-async")
TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/foxmath")
TEST_INCLUDE_PATH("../../src/app/driver/rtc")

/*========== Definitions and Implementations for Unit Test ==================*/
const CAN_NODE_s can_node1 = {
    .canNodeRegister = canREG1,
};

const CAN_NODE_s can_node2Isolated = {
    .canNodeRegister = canREG2,
};

/* Dummy for version file implementation */
const VER_VERSION_s ver_foxbmsVersionInformation = {
    .underVersionControl     = true,
    .isDirty                 = true,
    .major                   = 13,
    .minor                   = 54,
    .patch                   = 7,
    .distanceFromLastRelease = 42,
    .commitHash              = "deadbeef",
    .gitRemote               = "onTheDarkSideOfTheMoon.git",
};

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
void testTransmitBmsVersionInfo(void) {
    uint8_t testCanDataZeroArray[CANTX_DEBUG_RESPONSE_DLC] = {0u};
    uint64_t testMessageData                               = 0u;

    MATH_MinimumOfTwoUint16_t_ExpectAndReturn(42, 31u, 31u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData, 7u, 8u, 0x00u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData, 15u, 8u, 0x0Du, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData, 23u, 8u, 0x36u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData, 31u, 8u, 0x07u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData, 39u, 5u, 0x1Fu, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData, 34u, 1u, 0x01u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData, 33u, 1u, 0x01u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData, 32u, 1u, 0x01u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData, testCanDataZeroArray, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_1, CANTX_DEBUG_RESPONSE_ID, CAN_STANDARD_IDENTIFIER_11_BIT, testCanDataZeroArray, STD_OK);

    CANTX_DebugResponse(CANTX_DEBUG_RESPONSE_TRANSMIT_BMS_VERSION_INFO);
}

void testTransmitBootMagicStart(void) {
    uint8_t testCanDataZeroArray[CANTX_DEBUG_RESPONSE_DLC] = {0u};
    uint64_t testMessageData                               = 0u;

    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData, 7u, 8u, 0x0Fu, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData, 15u, 56u, 0xFEFEFEFEFEFEFEuLL, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData, testCanDataZeroArray, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_1, CANTX_DEBUG_RESPONSE_ID, CAN_STANDARD_IDENTIFIER_11_BIT, testCanDataZeroArray, STD_OK);

    CANTX_DebugResponse(CANTX_DEBUG_RESPONSE_TRANSMIT_BOOT_MAGIC_START);
}

void testTransmitBootMagicEnd(void) {
    uint8_t testCanDataZeroArray[CANTX_DEBUG_RESPONSE_DLC] = {0u};
    uint64_t testMessageData                               = 0u;

    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData, 7u, 8u, 0x0Fu, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData, 15u, 56u, 0x01010101010101uLL, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData, testCanDataZeroArray, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_1, CANTX_DEBUG_RESPONSE_ID, CAN_STANDARD_IDENTIFIER_11_BIT, testCanDataZeroArray, STD_OK);

    CANTX_DebugResponse(CANTX_DEBUG_RESPONSE_TRANSMIT_BOOT_MAGIC_END);
}

void testTransmitRtcTime(void) {
    uint8_t testCanDataZeroArray[CANTX_DEBUG_RESPONSE_DLC] = {0u};
    uint64_t testMessageData                               = 0u;
    RTC_TIME_DATA_s testRtcTime                            = {
                                   .hundredthOfSeconds = 69u,
                                   .seconds            = 07u,
                                   .minutes            = 14u,
                                   .hours              = 3u,
                                   .weekday            = 2u,
                                   .day                = 19u,
                                   .month              = 1u,
                                   .year               = 38u,
    };

    RTC_GetSystemTimeRtcFormat_ExpectAndReturn(testRtcTime);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData, 7u, 8u, 0x04u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData, 15u, 7u, 0x45u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData, 8u, 6u, 0x07u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData, 18u, 6u, 0x0Eu, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData, 28u, 5u, 0x03u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData, 39u, 3u, 0x02u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData, 36u, 5u, 0x13u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData, 47u, 4u, 0x01u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData, 43u, 7u, 0x26u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData, testCanDataZeroArray, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_1, CANTX_DEBUG_RESPONSE_ID, CAN_STANDARD_IDENTIFIER_11_BIT, testCanDataZeroArray, STD_OK);
    CANTX_DebugResponse(CANTX_DEBUG_RESPONSE_TRANSMIT_RTC_TIME);
}

void testTransmitCommitHash(void) {
    uint8_t testCanDataZeroArray[CANTX_DEBUG_RESPONSE_DLC] = {0u};
    uint64_t testMessageData                               = 0u;

    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData, 7u, 8u, 0x05u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData, 15u, 8u, 0x64u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData, 23u, 8u, 0x65u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData, 31u, 8u, 0x61u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData, 39u, 8u, 0x64u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData, 47u, 8u, 0x62u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData, 55u, 8u, 0x65u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData, 63u, 8u, 0x65u, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData, testCanDataZeroArray, CANTX_DEBUG_RESPONSE_ENDIANNESS);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_1, CANTX_DEBUG_RESPONSE_ID, CAN_STANDARD_IDENTIFIER_11_BIT, testCanDataZeroArray, STD_OK);
    CANTX_DebugResponse(CANTX_DEBUG_RESPONSE_TRANSMIT_COMMIT_HASH);
}
