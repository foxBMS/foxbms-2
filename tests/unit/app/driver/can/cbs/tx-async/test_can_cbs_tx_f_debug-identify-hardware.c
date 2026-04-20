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
 * @file    test_can_cbs_tx_f_debug-identify-hardware.c
 * @author  foxBMS Team
 * @date    2025-07-17 (date of creation)
 * @updated 2026-04-20 (date of last update)
 * @version v1.11.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Test of some module
 * @details Detailed Test Description
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockafe.h"
#include "Mockcan.h"
#include "Mockcan_helper.h"

#include "can_cfg.h"

#include "can_cbs_tx_f_debug-identify-hardware.h"
#include "can_cfg_tx-async-message-definitions.h"
#include "test_assert_helper.h"

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("can_cbs_tx_f_debug-identify-hardware.c")

TEST_INCLUDE_PATH("../../src/app/application/config")
TEST_INCLUDE_PATH("../../src/app/driver/afe/api")
TEST_INCLUDE_PATH("../../src/app/driver/can")
TEST_INCLUDE_PATH("../../src/app/driver/can/cbs")
TEST_INCLUDE_PATH("../../src/app/driver/can/cbs/tx-async")
TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/foxmath")
TEST_INCLUDE_PATH("../../src/app/driver/rtc")
TEST_INCLUDE_PATH("../../src/app/task/config")
TEST_INCLUDE_PATH("../../src/app/task/ftask")

/*========== Definitions and Implementations for Unit Test ==================*/

OS_QUEUE ftsk_imdCanDataQueue = NULL_PTR;

uint64_t testMessageData[9u] = {0u, 1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u};

/* dummy data array filled with zero */
uint8_t testCanDataZeroArray[CAN_MAX_DLC] = {0};

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/

/**
 * @brief   Testing CANTX_DebugIdentifyHardwareMux
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/5: NULL_PTR for pMessageData -> assert
 *            - AT2/5: invalid value for muxValue -> assert
 *            - AT3/5: invalid value for muxValue -> assert
 *            - AT4/5: invalid value for muxValue -> assert
 *            - AT5/5: invalid value for muxValue -> assert
 *          - Routine validation:
 *            - RT1/1: Function sets expected values in the message data
 */
void testCANTX_DebugIdentifyHardwareMux(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/5 ======= */
    uint64_t muxValue    = 0u;
    uint64_t messageData = 0u;
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_DebugIdentifyHardwareMux(NULL_PTR, muxValue));

    /* ======= AT2/5 ======= */
    muxValue = 0x1Fu;
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_DebugIdentifyHardwareMux(&messageData, muxValue));

    /* ======= AT3/5 ======= */
    muxValue = 0x29u;
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_DebugIdentifyHardwareMux(&messageData, muxValue));

    /* ======= AT4/5 ======= */
    muxValue = 0x2Fu;
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_DebugIdentifyHardwareMux(&messageData, muxValue));

    /* ======= AT5/5 ======= */
    muxValue = 0x38u;
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_DebugIdentifyHardwareMux(&messageData, muxValue));

    /* ======= Routine tests =============================================== */
    muxValue = 0x01u;
    /* ======= RT1/1: Test implementation */
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 1u, 0x01u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    /* ======= RT1/1: Call function under test */
    TEST_CANTX_DebugIdentifyHardwareMux(&messageData, muxValue);
    /* ======= RT1/1: Test output verification */
    TEST_ASSERT_EQUAL(testMessageData[1u], messageData);
}

/**
 * @brief   Testing CANTX_SendMasterIdentification
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - None
 *          - Routine validation:
 *            - RT1/1: Function sets expected values in the message data and sends message
 */
void testCANTX_SendMasterIdentification(void) {
    /* TODO when application message is implemented*/
    /* ======= Routine tests =============================================== */
    uint8_t testCanDataFilled[CAN_MAX_DLC] = {0x12, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
    /* ======= RT1/1: Test implementation */
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 1u, 0x00u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataZeroArray, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_IDENTIFY_HARDWARE_ID,
        CANTX_DEBUG_IDENTIFY_HARDWARE_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    /* ======= RT1/1: Call function under test */
    STD_RETURN_TYPE_e testResult = TEST_CANTX_SendMasterIdentification();
    /* ======= RT1/1: Test output verification */
    TEST_ASSERT_EQUAL(STD_OK, testResult);
}

/**
 * @brief   Testing CANTX_SendSlaveIdentification
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - None
 *          - Routine validation:
 *            - RT1/1: Function sets expected values in the message data and sends message
 */
void testCANTX_SendSlaveIdentification(void) {
    /* ======= Routine tests =============================================== */
    uint8_t testCanDataFilled[CAN_MAX_DLC] = {0x12, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
    uint64_t *oldValue                     = serialIds;
    uint64_t newValue                      = 123u;
    /* Set serialIds with test value */
    serialIds = &newValue;

    /* ======= RT1/1: Test implementation */
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 1u, 0x01u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[1u], 6u, 15u, 0x00u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[2u]);

    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[2u], 23u, 48u, 0x7Bu, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[3u]);

    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[3u], testCanDataZeroArray, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_IDENTIFY_HARDWARE_ID,
        CANTX_DEBUG_IDENTIFY_HARDWARE_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    /* ======= RT1/1: Call function under test */
    STD_RETURN_TYPE_e testResult = TEST_CANTX_SendSlaveIdentification(0u);
    /* restore serialIds */
    serialIds = oldValue;
    /* ======= RT1/1: Test output verification */
    TEST_ASSERT_EQUAL(STD_OK, testResult);
}

void testCANTX_DebugIdentifyHardware(void) {
    /* ======= Routine tests =============================================== */
    uint8_t testCanDataFilled[CAN_MAX_DLC] = {0x12, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
    uint64_t *oldValue                     = serialIds;
    uint64_t newValue                      = 123u;
    /* Set serialIds with test value */
    serialIds = &newValue;

    /* ======= RT1/3: Test implementation */
    /* Messages sent by CANTX_SendMasterIdentification */
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 1u, 0x00u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataZeroArray, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_IDENTIFY_HARDWARE_ID,
        CANTX_DEBUG_IDENTIFY_HARDWARE_ID_TYPE,
        testCanDataFilled,
        STD_NOT_OK);

    /* ======= RT1/3: Call function under test */
    STD_RETURN_TYPE_e testResult = CANTX_DebugIdentifyHardware();
    /* ======= RT1/3: Test output verification */
    TEST_ASSERT_EQUAL(STD_NOT_OK, testResult);

    /* ======= RT2/3: Test implementation */
    /* Messages sent by CANTX_SendMasterIdentification */
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 1u, 0x00u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataZeroArray, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_IDENTIFY_HARDWARE_ID,
        CANTX_DEBUG_IDENTIFY_HARDWARE_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    /* Messages sent by CANTX_SendSlaveIdentification */
    AFE_IdentifyAfes_ExpectAndReturn(serialIds);

    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 1u, 0x01u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[1u], 6u, 15u, 0x00u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[2u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[2u], 23u, 48u, 0x7Bu, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[3u]);

    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[3u], testCanDataZeroArray, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_IDENTIFY_HARDWARE_ID,
        CANTX_DEBUG_IDENTIFY_HARDWARE_ID_TYPE,
        testCanDataFilled,
        STD_NOT_OK);
    /* ======= RT2/3: Call function under test */
    testResult = CANTX_DebugIdentifyHardware();
    /* ======= RT2/3: Test output verification */
    TEST_ASSERT_EQUAL(STD_NOT_OK, testResult);

    /* ======= RT2/3: Test implementation */
    /* Messages sent by CANTX_SendMasterIdentification */
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 1u, 0x00u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataZeroArray, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_IDENTIFY_HARDWARE_ID,
        CANTX_DEBUG_IDENTIFY_HARDWARE_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    /* Messages sent by CANTX_SendSlaveIdentification */
    AFE_IdentifyAfes_ExpectAndReturn(serialIds);

    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 1u, 0x01u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[1u], 6u, 15u, 0x00u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[2u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[2u], 23u, 48u, 0x7Bu, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[3u]);

    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[3u], testCanDataZeroArray, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_IDENTIFY_HARDWARE_ID,
        CANTX_DEBUG_IDENTIFY_HARDWARE_ID_TYPE,
        testCanDataFilled,
        STD_NOT_OK);
    /* ======= RT2/3: Call function under test */
    testResult = CANTX_DebugIdentifyHardware();
    /* ======= RT2/3: Test output verification */
    TEST_ASSERT_EQUAL(STD_NOT_OK, testResult);

    /* ======= RT3/3: Test implementation */
    /* Messages sent by CANTX_SendMasterIdentification */
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 1u, 0x00u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataZeroArray, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_IDENTIFY_HARDWARE_ID,
        CANTX_DEBUG_IDENTIFY_HARDWARE_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    /* Messages sent by CANTX_SendSlaveIdentification */
    AFE_IdentifyAfes_ExpectAndReturn(serialIds);

    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 1u, 0x01u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[1u], 6u, 15u, 0x00u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[2u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[2u], 23u, 48u, 0x7Bu, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[3u]);

    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[3u], testCanDataZeroArray, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_IDENTIFY_HARDWARE_ID,
        CANTX_DEBUG_IDENTIFY_HARDWARE_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    /* ======= RT3/3: Call function under test */
    testResult = CANTX_DebugIdentifyHardware();
    /* restore serialIds */
    serialIds = oldValue;
    /* ======= RT3/3: Test output verification */
    TEST_ASSERT_EQUAL(STD_OK, testResult);
}
