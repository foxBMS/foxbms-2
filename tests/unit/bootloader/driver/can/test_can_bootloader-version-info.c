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
 * @file    test_can_bootloader-version-info.c
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
#include "general.h"

#include "unity.h"
#include "Mockcan.h"
#include "Mockcan_cfg.h"
#include "Mockcan_helper.h"
#include "Mockfoxmath.h"
#include "Mockutils.h"

#include "can_bootloader-version-info.h"
#include "fstd_types.h"
#include "test_assert_helper.h"
#include "version.h"

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("can_cfg.c")

TEST_INCLUDE_PATH("../../src/bootloader/driver/can")
TEST_INCLUDE_PATH("../../src/bootloader/driver/config")
TEST_INCLUDE_PATH("../../src/bootloader/driver/foxmath")
TEST_INCLUDE_PATH("../../src/bootloader/main/include")
TEST_INCLUDE_PATH("../../src/bootloader/engine/can")
TEST_INCLUDE_PATH("../../src/bootloader/engine/boot")
TEST_INCLUDE_PATH("../../src/version")

/*========== Definitions and Implementations for Unit Test ==================*/
const CAN_NODE_s can_node1 = {
    .pCanNodeRegister = canREG1,
};

uint64_t testMessageData[10u] = {0u, 1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u, 0x0123456789ABCEFu};

uint64_t testSignalData[2u] = {0u, 1u};

VER_VERSION_s ver_versionInformation = {
    .underVersionControl     = false,
    .isDirty                 = false,
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
 * @brief   Testing CANTX_TransmitBootloaderVersionInfo
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - None
 *          - Routine validation:
 *            - RT1/2: underVersionControl, isDirty = false; no releaseDistanceOverflow
 *            - RT2/3: underVersionControl, isDirty = true; releaseDistanceOverflow
 */
void testCANTX_TransmitBootloaderVersionInfo(void) {
    /* ======= Routine tests =============================================== */
    /* ======= RT1/2: Test implementation */
    MATH_MinimumOfTwoUint16_t_ExpectAndReturn(17u, 31u, 17u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[1u], 15u, 8u, 13u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[2u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[2u], 23u, 8u, 54u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[3u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[3u], 31u, 8u, 7u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[4u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[4u], 39u, 5u, 17u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[5u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[5u], 34u, 1u, 0u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[6u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[6u], 33u, 1u, 0u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[7u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[7u], 32u, 1u, 0u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[8u]);
    /* ======= RT1/2: Call function under test */
    uint64_t testResult = TEST_CANTX_TransmitBootloaderVersionInfo();
    /* ======= RT1/2: Test output verification */
    TEST_ASSERT_EQUAL(8u, testResult);

    /* ======= RT2/2: Test implementation */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
    uint16_t *tmpDistanceFromLastRelease = &ver_versionInformation.distanceFromLastRelease;
    *tmpDistanceFromLastRelease          = 45u;
    bool *tmpIsDirty                     = &ver_versionInformation.isDirty;
    *tmpIsDirty                          = true;
    bool *tmpUnderVersionControl         = &ver_versionInformation.underVersionControl;
    *tmpUnderVersionControl              = true;
#pragma GCC diagnostic pop
    MATH_MinimumOfTwoUint16_t_ExpectAndReturn(45u, 31u, 31u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[1u], 15u, 8u, 13u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[2u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[2u], 23u, 8u, 54u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[3u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[3u], 31u, 8u, 7u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[4u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[4u], 39u, 5u, 31u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[5u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[5u], 34u, 1u, 1u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[6u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[6u], 33u, 1u, 1u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[7u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[7u], 32u, 1u, 1u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[8u]);
    /* ======= RT2/2: Call function under test */
    testResult = TEST_CANTX_TransmitBootloaderVersionInfo();
    /* ======= RT2/2: Test output verification */
    TEST_ASSERT_EQUAL(8u, testResult);
}

/**
 * @brief   Testing CANTX_TransmitBootMagic
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: messageData = 0u
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
    uint8_t testCanDataZeroArray[8u]           = {0u};
    uint8_t testCanDataHashHigh[8u]            = {0xEFu, 0xCDu, 0xABu, 0x89u, 0x67u, 0x45u, 0x23u, 0x01u};
    CAN_BUFFER_ELEMENT_s testCanBufferHashHigh = {
        .pCanNode = CAN_NODE_1,
        .id       = 0x410u,
        .idType   = CAN_STANDARD_IDENTIFIER_11_BIT,
        .data     = {0xEFu, 0xCDu, 0xABu, 0x89u, 0x67u, 0x45u, 0x23u, 0x01u},
    };
    uint8_t testCanDataHashLow[8u]            = {0x01u, 0x23u, 0x45u, 0x67u, 0x89u, 0xABu, 0xCDu, 0xEFu};
    CAN_BUFFER_ELEMENT_s testCanBufferHashLow = {
        .pCanNode = CAN_NODE_1,
        .id       = 0x410u,
        .idType   = CAN_STANDARD_IDENTIFIER_11_BIT,
        .data     = {0x01u, 0x23u, 0x45u, 0x67u, 0x89u, 0xABu, 0xCDu, 0xEFu},
    };
    /* ======= RT1/2: Test implementation */
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x05u, CAN_BIG_ENDIAN);
    UTIL_ExtractCharactersFromString_Expect(&testSignalData[0u], ver_versionInformation.commitHash, 14u, 0u, 7u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 15u, 56u, testSignalData[0u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1], testCanDataZeroArray, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnArrayThruPtr_pCanData(testCanDataHashHigh, 8);
    CAN_SendMessage_ExpectAndReturn(testCanBufferHashHigh, STD_NOT_OK);
    /* ======= RT1/2: Call function under test */
    uint64_t testResult = TEST_CANTX_TransmitCommitHash();
    /* ======= RT1/2: Test output verification */
    TEST_ASSERT_EQUAL(STD_NOT_OK, testResult);

    /* ======= RT2/2: Test implementation */
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x05u, CAN_BIG_ENDIAN);
    UTIL_ExtractCharactersFromString_Expect(&testSignalData[0u], ver_versionInformation.commitHash, 14u, 0u, 7u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 15u, 56u, testSignalData[0u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataZeroArray, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnArrayThruPtr_pCanData(testCanDataHashHigh, 8);
    CAN_SendMessage_ExpectAndReturn(testCanBufferHashHigh, STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x06u, CAN_BIG_ENDIAN);
    UTIL_ExtractCharactersFromString_Expect(&testSignalData[0u], ver_versionInformation.commitHash, 14u, 7u, 7u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 15u, 56u, testSignalData[0u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataZeroArray, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnArrayThruPtr_pCanData(testCanDataHashLow, 8);
    CAN_SendMessage_ExpectAndReturn(testCanBufferHashLow, STD_OK);
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
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x05u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    UTIL_ExtractCharactersFromString_Expect(&testSignalData[0u], ver_versionInformation.commitHash, 14u, 0u, 7u);
    UTIL_ExtractCharactersFromString_ReturnThruPtr_pExtractedCharacters(&testSignalData[1u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[1u], 15u, 56u, testSignalData[1u], CAN_BIG_ENDIAN);
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
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x06u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    UTIL_ExtractCharactersFromString_Expect(&testSignalData[0u], ver_versionInformation.commitHash, 14u, 7u, 7u);
    UTIL_ExtractCharactersFromString_ReturnThruPtr_pExtractedCharacters(&testSignalData[1u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[1u], 15u, 56u, testSignalData[1u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[2u]);
    /* ======= RT1/1: Call function under test */
    uint64_t testResult = TEST_CANTX_TransmitCommitHashLow();
    /* ======= RT1/1: Test output verification */
    TEST_ASSERT_EQUAL(testMessageData[2u], testResult);
}

/**
 * @brief   Testing CANTX_VersionInfoSendMessage
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - None
 *          - Routine validation:
 *            - RT1/2: message sent successfully
 *            - RT2/2: message not sent successfully
 */
void testCANTX_VersionInfoSendMessage(void) {
    /* ======= Routine tests =============================================== */
    CAN_BUFFER_ELEMENT_s testCanBuffer = {
        .id       = 0x410u,
        .idType   = CAN_STANDARD_IDENTIFIER_11_BIT,
        .pCanNode = CAN_NODE_1,
        .data     = {0x01u, 0x23u, 0x45u, 0x67u, 0x89u, 0xABu, 0xCDu, 0xEFu},
    };
    uint8_t testCanDataZeroArray[8u]   = {0u};
    uint8_t testCanDataFilledArray[8u] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
    /* ======= RT1/2: Test implementation */
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[9u], testCanDataZeroArray, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnArrayThruPtr_pCanData(&testCanDataFilledArray[0u], 8);
    CAN_SendMessage_ExpectAndReturn(testCanBuffer, STD_NOT_OK);
    /* ======= RT1/2: Call function under test */
    STD_RETURN_TYPE_e testResult = TEST_CANTX_VersionInfoSendMessage(testMessageData[9u]);
    /* ======= RT1/2: Test output verification */
    TEST_ASSERT_EQUAL(STD_NOT_OK, testResult);

    /* ======= RT2/2: Test implementation */
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[9u], testCanDataZeroArray, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnArrayThruPtr_pCanData(testCanBuffer.data, 8);
    CAN_SendMessage_ExpectAndReturn(testCanBuffer, STD_OK);
    /* ======= RT2/2: Call function under test */
    testResult = TEST_CANTX_VersionInfoSendMessage(testMessageData[9u]);
    /* ======= RT2/2: Test output verification */
    TEST_ASSERT_EQUAL(STD_OK, testResult);
}

/**
 * @brief   Testing CANTX_VersionInfo
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: Invalid action
 *          - Routine validation:
 *            - RT1/5: bootloader version info
 *            - RT2/5: boot magic start
 *            - RT3/5: boot magic end
 *            - RT4/5: commit hash
 *            - RT5/5: message not queued successfully
 */
void testCANTX_VersionInfo(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(CANTX_VersionInfo(4u));
    /* ======= Routine tests =============================================== */
    CAN_BUFFER_ELEMENT_s testCanBuffer = {
        .id       = 0x410u,
        .idType   = CAN_STANDARD_IDENTIFIER_11_BIT,
        .pCanNode = CAN_NODE_1,
        .data     = {0x01u, 0x23u, 0x45u, 0x67u, 0x89u, 0xABu, 0xCDu, 0xEFu},
    };
    uint8_t testCanDataZeroArray[8u]   = {0u};
    uint8_t testCanDataFilledArray[8u] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};

    uint8_t testCanDataHashHigh[8u]            = {0xEFu, 0xCDu, 0xABu, 0x89u, 0x67u, 0x45u, 0x23u, 0x01u};
    CAN_BUFFER_ELEMENT_s testCanBufferHashHigh = {
        .pCanNode = CAN_NODE_1,
        .id       = 0x410u,
        .idType   = CAN_STANDARD_IDENTIFIER_11_BIT,
        .data     = {0xEFu, 0xCDu, 0xABu, 0x89u, 0x67u, 0x45u, 0x23u, 0x01u},
    };
    uint8_t testCanDataHashLow[8u]            = {0x01u, 0x23u, 0x45u, 0x67u, 0x89u, 0xABu, 0xCDu, 0xEFu};
    CAN_BUFFER_ELEMENT_s testCanBufferHashLow = {
        .pCanNode = CAN_NODE_1,
        .id       = 0x410u,
        .idType   = CAN_STANDARD_IDENTIFIER_11_BIT,
        .data     = {0x01u, 0x23u, 0x45u, 0x67u, 0x89u, 0xABu, 0xCDu, 0xEFu},
    };
    /* ======= RT1/5: Test implementation */
    MATH_MinimumOfTwoUint16_t_ExpectAndReturn(45u, 31u, 31u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 15u, 8u, 13u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 23u, 8u, 54u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 31u, 8u, 7u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 39u, 5u, 31u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 34u, 1u, 1u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 33u, 1u, 1u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 32u, 1u, 1u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataZeroArray, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnArrayThruPtr_pCanData(&testCanDataFilledArray[0u], 8);
    CAN_SendMessage_ExpectAndReturn(testCanBuffer, STD_OK);
    /* ======= RT1/5: Call function under test */
    STD_RETURN_TYPE_e testResult = CANTX_VersionInfo(0u);
    /* ======= RT1/5: Test output verification */
    TEST_ASSERT_EQUAL(STD_OK, testResult);

    /* ======= RT2/5: Test implementation */
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x0F, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 15u, 56u, 0xFEFEFEFEFEFEFEuLL, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataZeroArray, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnArrayThruPtr_pCanData(&testCanDataFilledArray[0u], 8);
    CAN_SendMessage_ExpectAndReturn(testCanBuffer, STD_OK);
    /* ======= RT2/5: Call function under test */
    testResult = CANTX_VersionInfo(1u);
    /* ======= RT2/5: Test output verification */
    TEST_ASSERT_EQUAL(STD_OK, testResult);

    /* ======= RT3/5: Test implementation */
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x0F, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 15u, 56u, 0x01010101010101uLL, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataZeroArray, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnArrayThruPtr_pCanData(&testCanDataFilledArray[0u], 8);
    CAN_SendMessage_ExpectAndReturn(testCanBuffer, STD_OK);
    /* ======= RT3/5: Call function under test */
    testResult = CANTX_VersionInfo(2u);
    /* ======= RT3/5: Test output verification */
    TEST_ASSERT_EQUAL(STD_OK, testResult);

    /* ======= RT4/5: Test implementation */
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x05u, CAN_BIG_ENDIAN);
    UTIL_ExtractCharactersFromString_Expect(&testSignalData[0u], ver_versionInformation.commitHash, 14u, 0u, 7u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 15u, 56u, testSignalData[0u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataZeroArray, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnArrayThruPtr_pCanData(testCanDataHashHigh, 8);
    CAN_SendMessage_ExpectAndReturn(testCanBufferHashHigh, STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x06u, CAN_BIG_ENDIAN);
    UTIL_ExtractCharactersFromString_Expect(&testSignalData[0u], ver_versionInformation.commitHash, 14u, 7u, 7u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 15u, 56u, testSignalData[0u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataZeroArray, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnArrayThruPtr_pCanData(testCanDataHashLow, 8);
    CAN_SendMessage_ExpectAndReturn(testCanBufferHashLow, STD_OK);
    /* ======= RT4/5: Call function under test */
    testResult = CANTX_VersionInfo(3u);
    /* ======= RT4/5: Test output verification */
    TEST_ASSERT_EQUAL(STD_OK, testResult);

    /* ======= RT5/5: Test implementation */
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x0F, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 15u, 56u, 0x01010101010101uLL, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataZeroArray, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnArrayThruPtr_pCanData(&testCanDataFilledArray[0u], 8);
    CAN_SendMessage_ExpectAndReturn(testCanBuffer, STD_NOT_OK);
    /* ======= RT5/5: Call function under test */
    testResult = CANTX_VersionInfo(2u);
    /* ======= RT5/5: Test output verification */
    TEST_ASSERT_EQUAL(STD_NOT_OK, testResult);
}
