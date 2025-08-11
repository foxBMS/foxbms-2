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
 * @file    test_can.c
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
#include "MockHL_can.h"
#include "MockHL_het.h"
#include "MockHL_reg_can.h"
#include "MockHL_reg_system.h"
#include "Mockcan_bootloader-version-info.h"
#include "Mockcan_cbs.h"
#include "Mockcan_cfg.h"
#include "Mockcan_helper.h"
#include "Mockio.h"
#include "Mockrti.h"

#include "boot_cfg.h"

#include "can.h"
#include "mcu.h"
#include "test_assert_helper.h"

#include <stdint.h>

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("can.c")

TEST_INCLUDE_PATH("../../src/bootloader/driver/can")
TEST_INCLUDE_PATH("../../src/bootloader/driver/config")
TEST_INCLUDE_PATH("../../src/bootloader/driver/rti")
TEST_INCLUDE_PATH("../../src/bootloader/driver/mcu")
TEST_INCLUDE_PATH("../../src/bootloader/driver/io")
TEST_INCLUDE_PATH("../../src/bootloader/engine/boot")
TEST_INCLUDE_PATH("../../src/bootloader/engine/can")

/*========== Definitions and Implementations for Unit Test ==================*/
#define CAN_NODE_0 (CAN_NODE_s *)0

const CAN_NODE_s can_node1 = {
    .pCanNodeRegister = canREG1,
};

const CAN_NODE_s can_node2 = {
    .pCanNodeRegister = canREG2,
};

#define CAN_NODE_2 ((CAN_NODE_s *)&can_node2)

CAN_DATA_TRANSFER_INFO_s can_infoOfDataTransfer = {
    .totalNumOfDataTransferLoops           = 0u,
    .numOfCurrentLoop                      = 0u,
    .programLengthInBytes                  = 0u,
    .programStartAddressU8                 = BOOT_PROGRAM_START_ADDRESS,
    .programCurrentAddressU8               = BOOT_PROGRAM_START_ADDRESS,
    .programCurrentSectorAddressU8         = BOOT_PROGRAM_START_ADDRESS,
    .programCrc8Bytes                      = 0u,
    .programCrc8BytesOnBoard               = 0u,
    .sectorBufferCurrentAddressU8          = BOOT_SECTOR_BUFFER_START_ADDRESS,
    .numOfReceivedVectorTableDataIn64Bytes = 0u,
    .vectorTable                           = {0u, 0u, 0u, 0u},
    .crc64OfVectorTable                    = 0u,
};

const CAN_RX_MESSAGE_TYPE_s can_rxMessages[CAN_NUMBER_OF_RX_MESSAGES] = {
    {CAN_NODE_1,
     {.id         = CAN_RX_REQUEST_MESSAGE_ID,
      .idType     = CAN_STANDARD_IDENTIFIER_11_BIT,
      .dlc        = CAN_DEFAULT_DLC,
      .endianness = CAN_LITTLE_ENDIAN},
     .callbackFunction = &CAN_RxActionRequest},
    {CAN_NODE_1,
     {.id         = CAN_RX_TRANSFER_PROCESS_INFO_ID,
      .idType     = CAN_STANDARD_IDENTIFIER_11_BIT,
      .dlc        = CAN_DEFAULT_DLC,
      .endianness = CAN_LITTLE_ENDIAN},
     .callbackFunction = &CAN_RxTransferProcessInfo},
    {CAN_NODE_1,
     {.id         = CAN_RX_LOOP_INFO_ID,
      .idType     = CAN_STANDARD_IDENTIFIER_11_BIT,
      .dlc        = CAN_DEFAULT_DLC,
      .endianness = CAN_LITTLE_ENDIAN},
     .callbackFunction = &CAN_RxLoopInfo},
    {CAN_NODE_1,
     {.id         = CAN_RX_DATA_8_BYTES_ID,
      .idType     = CAN_STANDARD_IDENTIFIER_11_BIT,
      .dlc        = CAN_DEFAULT_DLC,
      .endianness = CAN_LITTLE_ENDIAN},
     .callbackFunction = &CAN_RxData8Bytes},
    {CAN_NODE_1,
     {.id         = CAN_RX_CRC_8_BYTES_ID,
      .idType     = CAN_STANDARD_IDENTIFIER_11_BIT,
      .dlc        = CAN_DEFAULT_DLC,
      .endianness = CAN_LITTLE_ENDIAN},
     .callbackFunction = &CAN_RxCrc8Bytes},
};

const CAN_TX_MESSAGE_TYPE_s can_txMessages[CAN_NUMBER_OF_TX_MESSAGES] = {
    {CAN_NODE_1,
     {.id         = CAN_TX_ACKNOWLEDGE_MESSAGE_ID,
      .idType     = CAN_STANDARD_IDENTIFIER_11_BIT,
      .dlc        = CAN_DEFAULT_DLC,
      .endianness = CAN_LITTLE_ENDIAN},
     .callbackFunction = NULL_PTR,
     .pMuxId           = NULL_PTR},
    {CAN_NODE_1,
     {.id         = CAN_TX_BOOTLOADER_FSM_STATES_ID,
      .idType     = CAN_STANDARD_IDENTIFIER_11_BIT,
      .dlc        = CAN_DEFAULT_DLC,
      .endianness = CAN_LITTLE_ENDIAN},
     .callbackFunction = NULL_PTR,
     .pMuxId           = NULL_PTR},
    {CAN_NODE_1,
     {.id         = CAN_TX_DATA_TRANSFER_INFO_ID,
      .idType     = CAN_STANDARD_IDENTIFIER_11_BIT,
      .dlc        = CAN_DEFAULT_DLC,
      .endianness = CAN_LITTLE_ENDIAN},
     .callbackFunction = NULL_PTR,
     .pMuxId           = NULL_PTR},
};

CAN_FSM_STATES_e can_stateOfCanCommunication = CAN_FSM_STATE_NO_COMMUNICATION;

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
/**
 * @brief   Testing CAN_SendMessage
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/2: pCanNode = NULL_PTR
 *            - AT2/2: idType = CAN_INVALID_TYPE
 *          - Routine validation:
 *            - RT1/3: message written successfully to message box via
 *                     CAN_NODE_1
 *            - RT2/3: message not written successfully to message box
 *            - RT3/3: no free message box found
 */
void testCAN_SendMessage(void) {
    /* ======= Assertion tests ============================================= */
    CAN_BUFFER_ELEMENT_s testMessage = {
        .pCanNode = CAN_NODE_1,
        .id       = 0x100u,
        .idType   = CAN_STANDARD_IDENTIFIER_11_BIT,
        .data     = {0x01u, 0x23u, 0x45u, 0x67u, 0x89u, 0xABu, 0xCDu, 0xEFu},
    };
    /* ======= AT1/3 ======= */
    testMessage.pCanNode = NULL_PTR;
    TEST_ASSERT_FAIL_ASSERT(CAN_SendMessage(testMessage));

    /* ======= AT2/3 ======= */
    testMessage.pCanNode = CAN_NODE_2;
    testMessage.idType   = CAN_INVALID_TYPE;
    TEST_ASSERT_FAIL_ASSERT(CAN_SendMessage(testMessage));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/3: Test implementation */
    testMessage.pCanNode = CAN_NODE_1;
    canIsTxMessagePending_ExpectAndReturn(canREG1, 1u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 2u, 0u);
    canUpdateID_Expect(canREG1, 2u, 0x24000000);
    canTransmit_ExpectAndReturn(canREG1, 2u, testMessage.data, 1u);
    /* ======= RT1/3: Call function under test */
    STD_RETURN_TYPE_e testResult = CAN_SendMessage(testMessage);
    /* ======= RT1/3: Test output verification */
    TEST_ASSERT_EQUAL(STD_OK, testResult);

    /* ======= RT2/3: Test implementation */
    testMessage.pCanNode = CAN_NODE_1;
    canIsTxMessagePending_ExpectAndReturn(canREG1, 1u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 2u, 0u);
    canUpdateID_Expect(canREG1, 2u, 0x24000000);
    canTransmit_ExpectAndReturn(canREG1, 2u, testMessage.data, 0u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 3u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 4u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 5u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 6u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 7u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 8u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 9u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 10u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 11u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 12u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 13u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 14u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 15u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 16u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 17u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 18u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 19u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 20u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 21u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 22u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 23u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 24u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 25u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 26u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 27u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 28u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 29u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 30u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 31u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 32u, 1u);
    /* ======= RT2/3: Call function under test */
    testResult = CAN_SendMessage(testMessage);
    /* ======= RT2/3: Test output verification */
    TEST_ASSERT_EQUAL(STD_NOT_OK, testResult);

    /* ======= RT3/3: Test implementation */
    canIsTxMessagePending_ExpectAndReturn(canREG1, 1u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 2u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 3u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 4u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 5u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 6u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 7u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 8u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 9u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 10u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 11u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 12u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 13u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 14u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 15u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 16u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 17u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 18u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 19u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 20u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 21u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 22u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 23u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 24u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 25u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 26u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 27u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 28u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 29u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 30u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 31u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 32u, 1u);
    /* ======= RT3/3: Call function under test */
    testResult = CAN_SendMessage(testMessage);
    /* ======= RT3/3: Test output verification */
    TEST_ASSERT_EQUAL(STD_NOT_OK, testResult);
}

/**
 * @brief   Testing CAN_Initialize
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - None
 *          - Routine validation:
 *            - RT1/1: canInit gets called
 */
void testCAN_Initialize(void) {
    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    canInit_Expect();
    IO_SetPinDirectionToOutput_Expect(&CAN_CAN1_IO_REG_DIR, CAN_CAN1_ENABLE_PIN);
    IO_SetPinDirectionToOutput_Expect(&CAN_CAN1_IO_REG_DIR, CAN_CAN1_STANDBY_PIN);
    IO_PinSet_Expect(&CAN_CAN1_IO_REG_DOUT, CAN_CAN1_ENABLE_PIN);
    IO_PinSet_Expect(&CAN_CAN1_IO_REG_DOUT, CAN_CAN1_STANDBY_PIN);
    /* ======= RT1/1: Call function under test */
    CAN_Initialize();
    /* ======= RT1/1: Test output verification */
}

/**
 * @brief   Testing CAN_SendBootloaderVersionInfo
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - None
 *          - Routine validation:
 *            - RT1/2: CANTX_VersionInfo returns STD_OK
 *            - RT2/2: CANTX_VersionInfo returns STD_NOT_OK
 */
void testCAN_SendBootloaderVersionInfo(void) {
    /* ======= Routine tests =============================================== */
    /* ======= RT1/2: Test implementation */
    CANTX_VersionInfo_ExpectAndReturn(CANTX_VERSION_INFO_TRANSMIT_BOOTLOADER_VERSION_INFO, STD_OK);
    /* ======= RT1/2: Call function under test */
    STD_RETURN_TYPE_e testResult = CAN_SendBootloaderVersionInfo();
    /* ======= RT1/2: Test output verification */
    TEST_ASSERT_EQUAL(STD_OK, testResult);

    /* ======= RT2/2: Test implementation */
    CANTX_VersionInfo_ExpectAndReturn(CANTX_VERSION_INFO_TRANSMIT_BOOTLOADER_VERSION_INFO, STD_NOT_OK);
    /* ======= RT2/2: Call function under test */
    testResult = CAN_SendBootloaderVersionInfo();
    /* ======= RT2/2: Test output verification */
    TEST_ASSERT_EQUAL(STD_NOT_OK, testResult);
}

/**
 * @brief   Testing CAN_SendBootloaderCommitHash
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - None
 *          - Routine validation:
 *            - RT1/2: CANTX_VersionInfo returns STD_OK
 *            - RT2/2: CANTX_VersionInfo returns STD_NOT_OK
 */
void testCAN_SendBootloaderCommitHash(void) {
    /* ======= Routine tests =============================================== */
    /* ======= RT1/2: Test implementation */
    CANTX_VersionInfo_ExpectAndReturn(CANTX_VERSION_INFO_TRANSMIT_COMMIT_HASH, STD_OK);
    /* ======= RT1/2: Call function under test */
    STD_RETURN_TYPE_e testResult = CAN_SendBootloaderCommitHash();
    /* ======= RT1/2: Test output verification */
    TEST_ASSERT_EQUAL(STD_OK, testResult);

    /* ======= RT2/2: Test implementation */
    CANTX_VersionInfo_ExpectAndReturn(CANTX_VERSION_INFO_TRANSMIT_COMMIT_HASH, STD_NOT_OK);
    /* ======= RT2/2: Call function under test */
    testResult = CAN_SendBootloaderCommitHash();
    /* ======= RT2/2: Test output verification */
    TEST_ASSERT_EQUAL(STD_NOT_OK, testResult);
}

/**
 * @brief   Testing CAN_SendDataTransferInfo
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - None
 *          - Routine validation:
 *            - RT1/2: Message is transmitted successfully
 *            - RT2/2: Message isn't transmitted successfully
 */
void testCAN_SendDataTransferInfo(void) {
    /* ======= Routine tests =============================================== */
    /* ======= RT1/2: Test implementation */
    can_infoOfDataTransfer.numOfCurrentLoop = 0x01234567u;
    uint64_t message                        = 0u;
    uint8_t data[8]                         = {0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u};
    CAN_TxSetMessageDataWithSignalData_Expect(
        &message,
        CAN_TX_BOOTLOADER_DATA_TRANSFER_INFO_CURRENT_LOOP_NUMBER_START_BIT,
        CAN_TX_BOOTLOADER_DATA_TRANSFER_INFO_CURRENT_LOOP_NUMBER_LENGTH,
        (uint64_t)can_infoOfDataTransfer.numOfCurrentLoop,
        can_txMessages[CAN_TX_BOOTLOADER_ACKNOWLEDGE_MESSAGE_INDEX].message.endianness);
    CAN_TxSetCanDataWithMessageData_Expect(
        message, data, can_txMessages[CAN_TX_BOOTLOADER_ACKNOWLEDGE_MESSAGE_INDEX].message.endianness);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 1u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 2u, 0u);
    canUpdateID_Expect(canREG1, 2u, 0x32400000u);
    canTransmit_ExpectAndReturn(canREG1, 2u, data, 1u);
    /* ======= RT1/2: Call function under test */
    STD_RETURN_TYPE_e testResult = CAN_SendDataTransferInfo();
    /* ======= RT1/2: Test output verification */
    TEST_ASSERT_EQUAL(STD_OK, testResult);

    /* ======= RT2/2: Test implementation */
    message = 0u;
    for (uint8_t i_data = 0u; i_data < 8u; i_data++) {
        data[i_data] = 0u;
    }
    CAN_TxSetMessageDataWithSignalData_Expect(
        &message,
        CAN_TX_BOOTLOADER_DATA_TRANSFER_INFO_CURRENT_LOOP_NUMBER_START_BIT,
        CAN_TX_BOOTLOADER_DATA_TRANSFER_INFO_CURRENT_LOOP_NUMBER_LENGTH,
        (uint64_t)can_infoOfDataTransfer.numOfCurrentLoop,
        can_txMessages[CAN_TX_BOOTLOADER_ACKNOWLEDGE_MESSAGE_INDEX].message.endianness);
    CAN_TxSetCanDataWithMessageData_Expect(
        message, data, can_txMessages[CAN_TX_BOOTLOADER_ACKNOWLEDGE_MESSAGE_INDEX].message.endianness);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 1u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 2u, 0u);
    canUpdateID_Expect(canREG1, 2u, 0x32400000u);
    canTransmit_ExpectAndReturn(canREG1, 2u, data, 0u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 3u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 4u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 5u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 6u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 7u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 8u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 9u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 10u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 11u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 12u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 13u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 14u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 15u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 16u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 17u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 18u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 19u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 20u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 21u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 22u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 23u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 24u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 25u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 26u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 27u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 28u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 29u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 30u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 31u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 32u, 1u);
    /* ======= RT2/2: Call function under test */
    testResult = CAN_SendDataTransferInfo();
    /* ======= RT2/2: Test output verification */
    TEST_ASSERT_EQUAL(STD_NOT_OK, testResult);
    can_infoOfDataTransfer.numOfCurrentLoop = 0x0u;
}

/**
 * @brief   Testing CAN_SendBootloaderInfo
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - None
 *          - Routine validation:
 *            - RT1/2: Message is transmitted successfully
 *            - RT2/2: Message isn't transmitted successfully
 */
void testCAN_SendBootloaderInfo(void) {
    /* ======= Routine tests =============================================== */
    /* ======= RT1/2: Test implementation */
    uint64_t message = 0u;
    uint8_t data[8]  = {0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u};
    CAN_TxSetMessageDataWithSignalData_Expect(
        &message,
        CAN_TX_BOOTLOADER_FSM_STATES_CAN_FSM_STATE_START_BIT,
        CAN_TX_BOOTLOADER_FSM_STATES_CAN_FSM_STATE_LENGTH,
        (uint64_t)can_stateOfCanCommunication,
        can_txMessages[CAN_TX_BOOTLOADER_FSM_STATES_INDEX].message.endianness);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &message,
        CAN_TX_BOOTLOADER_FSM_STATES_BOOT_FSM_STATE_START_BIT,
        CAN_TX_BOOTLOADER_FSM_STATES_BOOT_FSM_STATE_LENGTH,
        (uint64_t)boot_state,
        can_txMessages[CAN_TX_BOOTLOADER_FSM_STATES_INDEX].message.endianness);
    CAN_TxSetCanDataWithMessageData_Expect(
        message, data, can_txMessages[CAN_TX_BOOTLOADER_ACKNOWLEDGE_MESSAGE_INDEX].message.endianness);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 1u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 2u, 0u);
    canUpdateID_Expect(canREG1, 2u, 0x32540000u);
    canTransmit_ExpectAndReturn(canREG1, 2u, data, 1u);
    /* ======= RT1/2: Call function under test */
    STD_RETURN_TYPE_e testResult = CAN_SendBootloaderInfo();
    /* ======= RT1/2: Test output verification */
    TEST_ASSERT_EQUAL(STD_OK, testResult);

    /* ======= RT2/2: Test implementation */
    message = 0u;
    for (uint8_t i_data = 0u; i_data < 8u; i_data++) {
        data[i_data] = 0u;
    }
    CAN_TxSetMessageDataWithSignalData_Expect(
        &message,
        CAN_TX_BOOTLOADER_FSM_STATES_CAN_FSM_STATE_START_BIT,
        CAN_TX_BOOTLOADER_FSM_STATES_CAN_FSM_STATE_LENGTH,
        (uint64_t)can_stateOfCanCommunication,
        can_txMessages[CAN_TX_BOOTLOADER_FSM_STATES_INDEX].message.endianness);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &message,
        CAN_TX_BOOTLOADER_FSM_STATES_BOOT_FSM_STATE_START_BIT,
        CAN_TX_BOOTLOADER_FSM_STATES_BOOT_FSM_STATE_LENGTH,
        (uint64_t)boot_state,
        can_txMessages[CAN_TX_BOOTLOADER_FSM_STATES_INDEX].message.endianness);
    CAN_TxSetCanDataWithMessageData_Expect(
        message, data, can_txMessages[CAN_TX_BOOTLOADER_ACKNOWLEDGE_MESSAGE_INDEX].message.endianness);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 1u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 2u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 3u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 4u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 5u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 6u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 7u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 8u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 9u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 10u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 11u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 12u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 13u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 14u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 15u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 16u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 17u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 18u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 19u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 20u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 21u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 22u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 23u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 24u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 25u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 26u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 27u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 28u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 29u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 30u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 31u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 32u, 1u);
    /* ======= RT2/2: Call function under test */
    testResult = CAN_SendBootloaderInfo();
    /* ======= RT2/2: Test output verification */
    TEST_ASSERT_EQUAL(STD_NOT_OK, testResult);
}

/**
 * @brief   Testing CAN_SendAcknowledgeMessage
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - None
 *          - Routine validation:
 *            - RT1/5: YesNoFlag bit is set correctly
 *            - RT2/5: StatusCode bits are set correctly
 *            - RT3/5: AcknowledgeMessage bits are set correctly
 *            - RT4/5: AcknowledgeFlag bit is set correctly
 *            - RT5/5: Message isn't transmitted successfully
 */
void testCAN_SendAcknowledgeMessage(void) {
    /* ======= Routine tests =============================================== */
    CAN_BUFFER_ELEMENT_s testCanBuffer = {
        .pCanNode = can_txMessages[CAN_TX_BOOTLOADER_ACKNOWLEDGE_MESSAGE_INDEX].pCanNode,
        .id       = can_txMessages[CAN_TX_BOOTLOADER_ACKNOWLEDGE_MESSAGE_INDEX].message.id,
        .idType   = can_txMessages[CAN_TX_BOOTLOADER_ACKNOWLEDGE_MESSAGE_INDEX].message.idType,
        .data     = {0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u},
    };
    /* ======= RT1/5: Test implementation */
    uint64_t message = 0u;
    CAN_TxSetMessageDataWithSignalData_Expect(
        &message,
        CAN_TX_BOOTLOADER_ACKNOWLEDGE_MESSAGE_ACKNOWLEDGE_FLAG_START_BIT,
        CAN_TX_BOOTLOADER_ACKNOWLEDGE_MESSAGE_ACKNOWLEDGE_FLAG_LENGTH,
        NOT_RECEIVED,
        can_txMessages[CAN_TX_BOOTLOADER_ACKNOWLEDGE_MESSAGE_INDEX].message.endianness);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &message,
        CAN_TX_BOOTLOADER_ACKNOWLEDGE_MESSAGE_ACKNOWLEDGE_MESSAGE_START_BIT,
        CAN_TX_BOOTLOADER_ACKNOWLEDGE_MESSAGE_ACKNOWLEDGE_MESSAGE_LENGTH,
        RECEIVED_CMD_TO_TRANSFER_PROGRAM,
        can_txMessages[CAN_TX_BOOTLOADER_ACKNOWLEDGE_MESSAGE_INDEX].message.endianness);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &message,
        CAN_TX_BOOTLOADER_ACKNOWLEDGE_MESSAGE_STATUS_CODE_START_BIT,
        CAN_TX_BOOTLOADER_ACKNOWLEDGE_MESSAGE_STATUS_CODE_LENGTH,
        RECEIVED_BUT_NOT_PROCESSED,
        can_txMessages[CAN_TX_BOOTLOADER_ACKNOWLEDGE_MESSAGE_INDEX].message.endianness);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &message,
        CAN_TX_BOOTLOADER_ACKNOWLEDGE_MESSAGE_YES_NO_ANSWER_START_BIT,
        CAN_TX_BOOTLOADER_ACKNOWLEDGE_MESSAGE_YES_NO_ANSWER_LENGTH,
        YES,
        can_txMessages[CAN_TX_BOOTLOADER_ACKNOWLEDGE_MESSAGE_INDEX].message.endianness);
    CAN_TxSetCanDataWithMessageData_Expect(
        message, testCanBuffer.data, can_txMessages[CAN_TX_BOOTLOADER_ACKNOWLEDGE_MESSAGE_INDEX].message.endianness);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 1u, 0u);
    canUpdateID_Expect(canREG1, 1u, 0x32000000u);
    canTransmit_ExpectAndReturn(canREG1, 1u, testCanBuffer.data, 1u);
    /* ======= RT1/5: Call function under test */
    STD_RETURN_TYPE_e testResult =
        CAN_SendAcknowledgeMessage(NOT_RECEIVED, RECEIVED_CMD_TO_TRANSFER_PROGRAM, RECEIVED_BUT_NOT_PROCESSED, YES);
    /* ======= RT1/5: Test output verification */
    TEST_ASSERT_EQUAL(STD_OK, testResult);

    /* ======= RT2/5: Test implementation */
    CAN_TxSetMessageDataWithSignalData_Expect(
        &message,
        CAN_TX_BOOTLOADER_ACKNOWLEDGE_MESSAGE_ACKNOWLEDGE_FLAG_START_BIT,
        CAN_TX_BOOTLOADER_ACKNOWLEDGE_MESSAGE_ACKNOWLEDGE_FLAG_LENGTH,
        NOT_RECEIVED,
        can_txMessages[CAN_TX_BOOTLOADER_ACKNOWLEDGE_MESSAGE_INDEX].message.endianness);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &message,
        CAN_TX_BOOTLOADER_ACKNOWLEDGE_MESSAGE_ACKNOWLEDGE_MESSAGE_START_BIT,
        CAN_TX_BOOTLOADER_ACKNOWLEDGE_MESSAGE_ACKNOWLEDGE_MESSAGE_LENGTH,
        RECEIVED_CMD_TO_TRANSFER_PROGRAM,
        can_txMessages[CAN_TX_BOOTLOADER_ACKNOWLEDGE_MESSAGE_INDEX].message.endianness);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &message,
        CAN_TX_BOOTLOADER_ACKNOWLEDGE_MESSAGE_STATUS_CODE_START_BIT,
        CAN_TX_BOOTLOADER_ACKNOWLEDGE_MESSAGE_STATUS_CODE_LENGTH,
        RECEIVED_BUT_NOT_PROCESSED,
        can_txMessages[CAN_TX_BOOTLOADER_ACKNOWLEDGE_MESSAGE_INDEX].message.endianness);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &message,
        CAN_TX_BOOTLOADER_ACKNOWLEDGE_MESSAGE_YES_NO_ANSWER_START_BIT,
        CAN_TX_BOOTLOADER_ACKNOWLEDGE_MESSAGE_YES_NO_ANSWER_LENGTH,
        NO,
        can_txMessages[CAN_TX_BOOTLOADER_ACKNOWLEDGE_MESSAGE_INDEX].message.endianness);
    CAN_TxSetCanDataWithMessageData_Expect(
        message, testCanBuffer.data, can_txMessages[CAN_TX_BOOTLOADER_ACKNOWLEDGE_MESSAGE_INDEX].message.endianness);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 1u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 2u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 3u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 4u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 5u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 6u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 7u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 8u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 9u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 10u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 11u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 12u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 13u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 14u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 15u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 16u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 17u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 18u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 19u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 20u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 21u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 22u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 23u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 24u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 25u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 26u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 27u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 28u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 29u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 30u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 31u, 1u);
    canIsTxMessagePending_ExpectAndReturn(canREG1, 32u, 1u);
    /* ======= RT2/5: Call function under test */
    testResult =
        CAN_SendAcknowledgeMessage(NOT_RECEIVED, RECEIVED_CMD_TO_TRANSFER_PROGRAM, RECEIVED_BUT_NOT_PROCESSED, NO);
    /* ======= RT2/5: Test output verification */
    TEST_ASSERT_EQUAL(STD_NOT_OK, testResult);
}

/**
 * @brief   Testing CAN_SendBootMessage
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/4: TransmitBootMagicStart returns STD_NOT_OK
 *            - AT2/4: TransmitBootloaderVersionInfo returns STD_NOT_OK
 *            - AT3/4: TransmitCommitHash returns STD_NOT_OK
 *            - AT4/4: TransmitBootMagicEnd returns STD_NOT_OK
 *          - Routine validation:
 *            - RT1/1: Boot message sent successfully
 */
void testCAN_SendBootMessage(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/4 ======= */
    CANTX_VersionInfo_ExpectAndReturn(CANTX_VERSION_INFO_TRANSMIT_BOOT_MAGIC_START, STD_NOT_OK);
    TEST_ASSERT_FAIL_ASSERT(CAN_SendBootMessage());
    /* ======= AT2/4 ======= */
    CANTX_VersionInfo_ExpectAndReturn(CANTX_VERSION_INFO_TRANSMIT_BOOT_MAGIC_START, STD_OK);
    CANTX_VersionInfo_ExpectAndReturn(CANTX_VERSION_INFO_TRANSMIT_BOOTLOADER_VERSION_INFO, STD_NOT_OK);
    TEST_ASSERT_FAIL_ASSERT(CAN_SendBootMessage());
    /* ======= AT3/4 ======= */
    CANTX_VersionInfo_ExpectAndReturn(CANTX_VERSION_INFO_TRANSMIT_BOOT_MAGIC_START, STD_OK);
    CANTX_VersionInfo_ExpectAndReturn(CANTX_VERSION_INFO_TRANSMIT_BOOTLOADER_VERSION_INFO, STD_OK);
    CANTX_VersionInfo_ExpectAndReturn(CANTX_VERSION_INFO_TRANSMIT_COMMIT_HASH, STD_NOT_OK);
    TEST_ASSERT_FAIL_ASSERT(CAN_SendBootMessage());
    /* ======= AT4/4 ======= */
    CANTX_VersionInfo_ExpectAndReturn(CANTX_VERSION_INFO_TRANSMIT_BOOT_MAGIC_START, STD_OK);
    CANTX_VersionInfo_ExpectAndReturn(CANTX_VERSION_INFO_TRANSMIT_BOOTLOADER_VERSION_INFO, STD_OK);
    CANTX_VersionInfo_ExpectAndReturn(CANTX_VERSION_INFO_TRANSMIT_COMMIT_HASH, STD_OK);
    CANTX_VersionInfo_ExpectAndReturn(CANTX_VERSION_INFO_TRANSMIT_BOOT_MAGIC_END, STD_NOT_OK);
    TEST_ASSERT_FAIL_ASSERT(CAN_SendBootMessage());
    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    CANTX_VersionInfo_ExpectAndReturn(CANTX_VERSION_INFO_TRANSMIT_BOOT_MAGIC_START, STD_OK);
    CANTX_VersionInfo_ExpectAndReturn(CANTX_VERSION_INFO_TRANSMIT_BOOTLOADER_VERSION_INFO, STD_OK);
    CANTX_VersionInfo_ExpectAndReturn(CANTX_VERSION_INFO_TRANSMIT_COMMIT_HASH, STD_OK);
    CANTX_VersionInfo_ExpectAndReturn(CANTX_VERSION_INFO_TRANSMIT_BOOT_MAGIC_END, STD_OK);
    /* ======= RT1/1: Call function under test */
    CAN_SendBootMessage();
}
