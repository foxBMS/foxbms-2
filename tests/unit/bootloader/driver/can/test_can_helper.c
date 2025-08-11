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
 * @file    test_can_helper.c
 * @author  foxBMS Team
 * @date    2024-09-17 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Test of some module
 * @details TODO
 */

/*========== Includes =======================================================*/
#include "unity.h"

#include "can_helper.h"
#include "test_assert_helper.h"

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("can_helper.c")

TEST_INCLUDE_PATH("../../src/bootloader/driver/can")
TEST_INCLUDE_PATH("../../src/bootloader/driver/config")
TEST_INCLUDE_PATH("../../src/bootloader/engine/boot")

/*========== Definitions and Implementations for Unit Test ==================*/

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
/**
 * @brief   Testing CAN_ConvertBitStartBigEndian
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/5: bitStart = 64
 *            - AT2/5: bitLength = 65
 *            - AT3/5: bitLength = 0
 *            - AT4/5: converted bitStart out of max signal size
 *            - AT5/5: signal reaches out of message
 *          - Routine validation:
 *            - RT1/1: bitStart = 15, bitLength = 16
 */
void test_CAN_ConvertbitStartBigEndian(void) {
    /* ======= Assertion tests ============================================= */
    uint64_t testBitStart  = 64u;
    uint64_t testBitLength = 1u;
    /* ======= AT1/5 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CAN_ConvertBitStartBigEndian(testBitStart, testBitLength));
    /* ======= AT2/5 ======= */
    testBitStart  = 0u;
    testBitLength = 65u;
    TEST_ASSERT_FAIL_ASSERT(TEST_CAN_ConvertBitStartBigEndian(testBitStart, testBitLength));
    /* ======= AT3/5 ======= */
    testBitStart  = 0u;
    testBitLength = 0u;
    TEST_ASSERT_FAIL_ASSERT(TEST_CAN_ConvertBitStartBigEndian(testBitStart, testBitLength));
    /* ======= AT4/5 ======= */
    testBitStart  = 48u;
    testBitLength = 16u;
    TEST_ASSERT_FAIL_ASSERT(TEST_CAN_ConvertBitStartBigEndian(testBitStart, testBitLength));
    /* ======= AT5/5 ======= */
    testBitStart  = 8u;
    testBitLength = 63u;
    TEST_ASSERT_FAIL_ASSERT(TEST_CAN_ConvertBitStartBigEndian(testBitStart, testBitLength));
    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    testBitStart  = 15u;
    testBitLength = 16u;
    /* ======= RT1/1: Call function under test */
    uint64_t testResult = TEST_CAN_ConvertBitStartBigEndian(testBitStart, testBitLength);
    /* ======= RT1/1: Test output verification */
    TEST_ASSERT_EQUAL(40u, testResult);
}

/**
 * @brief   Testing CAN_TxSetMessageDataWithSignalData
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/6: pMessage = NULL_PTR
 *            - AT2/6: endianness = 2
 *            - AT3/6: bitLength = 65
 *            - AT4/6: bitLength = 0
 *            - AT5/6: bitStart = 64
 *            - AT6/6: bitStart + bitLength > 64, little endian
 *          - Routine validation:
 *            - RT1/3: Set message data with little endian
 *            - RT2/3: Set message data with big endian
 *            - RT3/3: Set message data with 64 bit length
 */
void testCAN_TxSetMessageDAtaWithSignalData(void) {
    /* ======= Assertion tests ============================================= */
    uint64_t testMessageData = 0u;
    uint64_t bitStart        = 0u;
    uint8_t bitLength        = 1u;
    uint64_t canSignal       = 1u;
    /* ======= AT1/6 ======= */
    TEST_ASSERT_FAIL_ASSERT(
        CAN_TxSetMessageDataWithSignalData(NULL_PTR, bitStart, bitLength, canSignal, CAN_LITTLE_ENDIAN));
    /* ======= AT2/6 ======= */
    TEST_ASSERT_FAIL_ASSERT(CAN_TxSetMessageDataWithSignalData(&testMessageData, bitStart, bitLength, canSignal, 2u));
    /* ======= AT3/6 ======= */
    TEST_ASSERT_FAIL_ASSERT(
        CAN_TxSetMessageDataWithSignalData(&testMessageData, bitStart, 65u, canSignal, CAN_LITTLE_ENDIAN));
    /* ======= AT4/6 ======= */
    TEST_ASSERT_FAIL_ASSERT(
        CAN_TxSetMessageDataWithSignalData(&testMessageData, bitStart, 0u, canSignal, CAN_LITTLE_ENDIAN));
    /* ======= AT5/6 ======= */
    TEST_ASSERT_FAIL_ASSERT(
        CAN_TxSetMessageDataWithSignalData(&testMessageData, 64u, bitLength, canSignal, CAN_LITTLE_ENDIAN));
    /* ======= AT6/6 ======= */
    TEST_ASSERT_FAIL_ASSERT(
        CAN_TxSetMessageDataWithSignalData(&testMessageData, 56u, 9u, canSignal, CAN_LITTLE_ENDIAN));
    /* ======= Routine tests =============================================== */
    /* ======= RT1/3: Test implementation */
    testMessageData = 0x0000112233445566u;
    bitStart        = 48u;
    bitLength       = 16u;
    canSignal       = 0xFEDC;
    /* ======= RT1/3: Call function under test */
    CAN_TxSetMessageDataWithSignalData(&testMessageData, bitStart, bitLength, canSignal, CAN_LITTLE_ENDIAN);
    /* ======= RT1/3: Test output verification */
    TEST_ASSERT_EQUAL(0xFEDC112233445566u, testMessageData);

    /* ======= RT2/3: Test implementation */
    bitStart = 07u;
    /* ======= RT2/3: Call function under test */
    CAN_TxSetMessageDataWithSignalData(&testMessageData, bitStart, bitLength, canSignal, CAN_BIG_ENDIAN);
    /* ======= RT2/3: Test output verification */
    TEST_ASSERT_EQUAL(0xFEDC112233445566u, testMessageData);

    /* ======= RT2/3: Test implementation */
    bitStart        = 0u;
    bitLength       = 64u;
    testMessageData = 0u;
    /* ======= RT2/3: Call function under test */
    CAN_TxSetMessageDataWithSignalData(&testMessageData, bitStart, bitLength, canSignal, CAN_LITTLE_ENDIAN);
    /* ======= RT2/3: Test output verification */
    TEST_ASSERT_EQUAL(0x000000000000FEDCu, testMessageData);
}

/**
 * @brief   Testing CAN_TxSetCanDataWithMessageData
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/2: pCanData is NULL_PTR
 *            - AT2/2: endianness is 2
 *          - Routine validation:
 *            - RT1/2: Set can data in CAN_LITTLE_ENDIAN
 *            - RT2/2: set can data in CAN_BIG_ENDIAN
 */
void testCAN_TxSetCanDataWithMessageData(void) {
    /* ======= Assertion tests ============================================= */
    uint64_t testMessageData = 0u;
    uint8_t testCanData[8u]  = {0u};
    /* ======= AT1/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(CAN_TxSetCanDataWithMessageData(testMessageData, NULL_PTR, CAN_LITTLE_ENDIAN));
    /* ======= AT2/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(CAN_TxSetCanDataWithMessageData(testMessageData, testCanData, 2u));
    /* ======= Routine tests =============================================== */
    /* ======= RT1/2: Test implementation */
    testMessageData = 0x0123456789ABCDEF;
    /* ======= RT1/2: Call function under test */
    CAN_TxSetCanDataWithMessageData(testMessageData, testCanData, CAN_LITTLE_ENDIAN);
    /* ======= RT1/2: Test output verification */
    TEST_ASSERT_EQUAL(0xEF, testCanData[CAN_BYTE_0_POSITION]);
    TEST_ASSERT_EQUAL(0xCD, testCanData[CAN_BYTE_1_POSITION]);
    TEST_ASSERT_EQUAL(0xAB, testCanData[CAN_BYTE_2_POSITION]);
    TEST_ASSERT_EQUAL(0x89, testCanData[CAN_BYTE_3_POSITION]);
    TEST_ASSERT_EQUAL(0x67, testCanData[CAN_BYTE_4_POSITION]);
    TEST_ASSERT_EQUAL(0x45, testCanData[CAN_BYTE_5_POSITION]);
    TEST_ASSERT_EQUAL(0x23, testCanData[CAN_BYTE_6_POSITION]);
    TEST_ASSERT_EQUAL(0x01, testCanData[CAN_BYTE_7_POSITION]);

    /* ======= RT2/2: Test implementation */
    for (uint8_t i = 0u; i < 8u; i++) {
        testCanData[i] = 0u;
    }
    /* ======= RT2/2: Call function under test */
    CAN_TxSetCanDataWithMessageData(testMessageData, testCanData, CAN_BIG_ENDIAN);
    /* ======= RT2/2: Test output verification */
    TEST_ASSERT_EQUAL(0x01, testCanData[CAN_BYTE_0_POSITION]);
    TEST_ASSERT_EQUAL(0x23, testCanData[CAN_BYTE_1_POSITION]);
    TEST_ASSERT_EQUAL(0x45, testCanData[CAN_BYTE_2_POSITION]);
    TEST_ASSERT_EQUAL(0x67, testCanData[CAN_BYTE_3_POSITION]);
    TEST_ASSERT_EQUAL(0x89, testCanData[CAN_BYTE_4_POSITION]);
    TEST_ASSERT_EQUAL(0xAB, testCanData[CAN_BYTE_5_POSITION]);
    TEST_ASSERT_EQUAL(0xCD, testCanData[CAN_BYTE_6_POSITION]);
    TEST_ASSERT_EQUAL(0xEF, testCanData[CAN_BYTE_7_POSITION]);
}

/** test the interface of CAN_RxGetSignalDataFromMessageData for null pointer and invalid parameters */
void testCAN_RxGetSignalDataFromMessageDataInterfaceNullPointer(void) {
    uint64_t message            = 0;
    uint64_t bitStart           = 0;
    uint8_t bitLength           = 0;
    uint64_t pCanSignal         = 0;
    CAN_ENDIANNESS_e endianness = CAN_LITTLE_ENDIAN;

    TEST_ASSERT_FAIL_ASSERT(CAN_RxGetSignalDataFromMessageData(message, bitStart, bitLength, &pCanSignal, endianness));

    TEST_ASSERT_FAIL_ASSERT(CAN_RxGetSignalDataFromMessageData(message, 1u, 1u, NULL_PTR, endianness));

    TEST_ASSERT_FAIL_ASSERT(CAN_RxGetSignalDataFromMessageData(message, 64u, 0u, &pCanSignal, endianness));
    TEST_ASSERT_FAIL_ASSERT(CAN_RxGetSignalDataFromMessageData(message, 1u, 65u, &pCanSignal, endianness));
    TEST_ASSERT_FAIL_ASSERT(CAN_RxGetSignalDataFromMessageData(message, 65u, 64u, &pCanSignal, endianness));
    TEST_ASSERT_FAIL_ASSERT(CAN_RxGetSignalDataFromMessageData(message, 10u, 64u, &pCanSignal, endianness));

    TEST_ASSERT_PASS_ASSERT(CAN_RxGetSignalDataFromMessageData(message, 1u, 1u, &pCanSignal, CAN_LITTLE_ENDIAN));
    TEST_ASSERT_PASS_ASSERT(CAN_RxGetSignalDataFromMessageData(message, 1u, 1u, &pCanSignal, CAN_BIG_ENDIAN));
    TEST_ASSERT_FAIL_ASSERT(CAN_RxGetSignalDataFromMessageData(message, 1u, 1u, &pCanSignal, INT8_MAX));
}

/**
 * @brief   Testing CAN_RxGetSignalDataFromMessageData
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/6: pCanSignal = NULL_PTR
 *            - AT2/6: endianness = 2
 *            - AT3/6: bitLength = 65
 *            - AT4/6: bitLength = 0
 *            - AT5/6: bitStart = 64
 *            - AT6/6: bitStart + bitLength > 64, little endian
 *          - Routine validation:
 *            - RT1/3: Get signal data with little endian
 *            - RT2/3: Get signal data with big endian
 *            - RT3/3: Get signal data with 64 bit length
 */
void testCAN_RxGetSignalDataFromMessageData(void) {
    /* ======= Assertion tests ============================================= */
    uint64_t testMessageData = 0u;
    uint64_t testBitStart    = 0u;
    uint8_t testBitLength    = 1u;
    uint64_t testCanSignal   = 0u;

    /* ======= AT1/6 ======= */
    TEST_ASSERT_FAIL_ASSERT(
        CAN_RxGetSignalDataFromMessageData(testMessageData, testBitStart, testBitLength, NULL_PTR, CAN_LITTLE_ENDIAN));
    /* ======= AT2/6 ======= */
    TEST_ASSERT_FAIL_ASSERT(
        CAN_RxGetSignalDataFromMessageData(testMessageData, testBitStart, testBitLength, &testCanSignal, 2u));
    /* ======= AT3/6 ======= */
    testBitLength = 65u;
    TEST_ASSERT_FAIL_ASSERT(CAN_RxGetSignalDataFromMessageData(
        testMessageData, testBitStart, testBitLength, &testCanSignal, CAN_LITTLE_ENDIAN));
    /* ======= AT4/6 ======= */
    testBitLength = 0u;
    TEST_ASSERT_FAIL_ASSERT(CAN_RxGetSignalDataFromMessageData(
        testMessageData, testBitStart, testBitLength, &testCanSignal, CAN_LITTLE_ENDIAN));
    /* ======= AT5/6 ======= */
    testBitLength = 1u;
    testBitStart  = 64u;
    TEST_ASSERT_FAIL_ASSERT(CAN_RxGetSignalDataFromMessageData(
        testMessageData, testBitStart, testBitLength, &testCanSignal, CAN_LITTLE_ENDIAN));
    /* ======= AT6/6 ======= */
    testBitStart  = 63u;
    testBitLength = 2u;
    TEST_ASSERT_FAIL_ASSERT(CAN_RxGetSignalDataFromMessageData(
        testMessageData, testBitStart, testBitLength, &testCanSignal, CAN_LITTLE_ENDIAN));
    /* ======= Routine tests =============================================== */
    /* ======= RT1/3: Test implementation */
    testMessageData = 0x0123456789ABCDEFu;
    testBitStart    = 8u;
    testBitLength   = 16u;
    /* ======= RT1/3: Call function under test */
    CAN_RxGetSignalDataFromMessageData(testMessageData, testBitStart, testBitLength, &testCanSignal, CAN_LITTLE_ENDIAN);
    /* ======= RT1/3: Test output verification */
    TEST_ASSERT_EQUAL(0xABCD, testCanSignal);

    /* ======= RT2/3: Test implementation */
    testCanSignal = 0u;
    testBitStart  = 7u;
    /* ======= RT2/3: Call function under test */
    CAN_RxGetSignalDataFromMessageData(testMessageData, testBitStart, testBitLength, &testCanSignal, CAN_BIG_ENDIAN);
    /* ======= RT2/3: Test output verification */
    TEST_ASSERT_EQUAL(0x0123, testCanSignal);

    /* ======= RT3/3: Test implementation */
    testCanSignal = 0u;
    testBitStart  = 0u;
    testBitLength = 64u;
    /* ======= RT3/3: Call function under test */
    CAN_RxGetSignalDataFromMessageData(testMessageData, testBitStart, testBitLength, &testCanSignal, CAN_LITTLE_ENDIAN);
    /* ======= RT3/3: Test output verification */
    TEST_ASSERT_EQUAL(0x0123456789ABCDEFu, testCanSignal);
}

/**
 * @brief   Testing CAN_RxGetMessageDataFromCanData
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/3: pMessage is NULL_PTR
 *            - AT2/3: kpkCanData is NULL_PTR
 *            - AT3/3: endianness = 2
 *          - Routine validation:
 *            - RT1/2: Extract message data in little endian
 *            - RT2/2: Extract message data in big endian
 */
void testCAN_RxGetMessageDataFromCanData(void) {
    /* ======= Assertion tests ============================================= */
    uint64_t testMessageData = 0u;
    uint8_t testCanData[8u]  = {0xEFu, 0xCD, 0xAB, 0x89, 0x67, 0x45, 0x23, 0x01};

    /* ======= AT1/3 ======= */
    TEST_ASSERT_FAIL_ASSERT(CAN_RxGetMessageDataFromCanData(NULL_PTR, testCanData, CAN_LITTLE_ENDIAN));
    /* ======= AT2/3 ======= */
    TEST_ASSERT_FAIL_ASSERT(CAN_RxGetMessageDataFromCanData(&testMessageData, NULL_PTR, CAN_LITTLE_ENDIAN));
    /* ======= AT3/3 ======= */
    TEST_ASSERT_FAIL_ASSERT(CAN_RxGetMessageDataFromCanData(&testMessageData, testCanData, 2u));
    /* ======= Routine tests =============================================== */
    /* ======= RT1/2: Test implementation */
    /* ======= RT1/2: Call function under test */
    CAN_RxGetMessageDataFromCanData(&testMessageData, testCanData, CAN_LITTLE_ENDIAN);
    /* ======= RT1/2: Test output verification */
    TEST_ASSERT_EQUAL(0x0123456789ABCDEFu, testMessageData);

    /* ======= RT2/2: Test implementation */
    testMessageData = 0u;
    /* ======= RT2/2: Call function under test */
    CAN_RxGetMessageDataFromCanData(&testMessageData, testCanData, CAN_BIG_ENDIAN);
    /* ======= RT2/2: Test output verification */
    TEST_ASSERT_EQUAL(0xEFCDAB8967452301u, testMessageData);
}
