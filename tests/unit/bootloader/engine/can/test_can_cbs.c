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
 * @file    test_can_cbs.c
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
#include "MockHL_sys_core.h"
#include "Mockcan.h"
#include "Mockcan_cfg.h"
#include "Mockcan_helper.h"
#include "Mockcrc.h"
#include "Mockflash.h"

#include "boot_cfg.h"

#include "can_cbs.h"
#include "fstd_types.h"
#include "test_assert_helper.h"

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("can_cbs.c")

TEST_INCLUDE_PATH("../../src/bootloader/driver/can")
TEST_INCLUDE_PATH("../../src/bootloader/driver/config")
TEST_INCLUDE_PATH("../../src/bootloader/driver/crc")
TEST_INCLUDE_PATH("../../src/bootloader/driver/flash")
TEST_INCLUDE_PATH("../../src/bootloader/driver/foxmath")
TEST_INCLUDE_PATH("../../src/bootloader/engine/boot")
TEST_INCLUDE_PATH("../../src/bootloader/engine/can")
TEST_INCLUDE_PATH("../../src/bootloader/main/include")
TEST_INCLUDE_PATH("C:/ti/Hercules/F021 Flash API/02.01.01/include")

/*========== Definitions and Implementations for Unit Test ==================*/
/** Initialize the state machine of can module */
CAN_FSM_STATES_e can_stateOfCanCommunication = CAN_FSM_STATE_NO_COMMUNICATION;
/** The information of can data transfer */
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
    .vectorTable                           = {0},
    .crc64OfVectorTable                    = 0u,
};
/** Copy of CAN data transfer to back up the CAN data transfer variable, which will be
 * used to recover the value contained in can_infoOfDataTransfer if something goes wrong.
 */
CAN_DATA_TRANSFER_INFO_s can_copyOfInfoOfDataTransfer = {0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, {0u, 0u, 0u, 0u}, 0u};

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/

void testCAN_RxActionRequest(void) {
    uint32_t retVal                      = 1u;
    CAN_MESSAGE_PROPERTIES_s testMessage = {
        .id         = CAN_RX_REQUEST_MESSAGE_ID,
        .idType     = CAN_STANDARD_IDENTIFIER_11_BIT,
        .dlc        = CAN_DEFAULT_DLC,
        .endianness = CAN_LITTLE_ENDIAN};
    uint8_t testCanData[CAN_DEFAULT_DLC] = {0u};
    uint64_t messageData                 = 0u;

    /* ======= Assertion tests =============================================== */
    testMessage.id         = 0x100u;
    testMessage.idType     = CAN_STANDARD_IDENTIFIER_11_BIT;
    testMessage.dlc        = CAN_DEFAULT_DLC;
    testMessage.endianness = CAN_LITTLE_ENDIAN;
    TEST_ASSERT_FAIL_ASSERT(CAN_RxActionRequest(testMessage, testCanData));
    testMessage.id         = CAN_RX_REQUEST_MESSAGE_ID;
    testMessage.idType     = CAN_INVALID_TYPE;
    testMessage.dlc        = CAN_DEFAULT_DLC;
    testMessage.endianness = CAN_LITTLE_ENDIAN;
    TEST_ASSERT_FAIL_ASSERT(CAN_RxActionRequest(testMessage, testCanData));
    testMessage.id         = CAN_RX_REQUEST_MESSAGE_ID;
    testMessage.idType     = CAN_STANDARD_IDENTIFIER_11_BIT;
    testMessage.dlc        = 7u;
    testMessage.endianness = CAN_LITTLE_ENDIAN;
    TEST_ASSERT_FAIL_ASSERT(CAN_RxActionRequest(testMessage, testCanData));
    testMessage.id         = CAN_RX_REQUEST_MESSAGE_ID;
    testMessage.idType     = CAN_STANDARD_IDENTIFIER_11_BIT;
    testMessage.dlc        = CAN_DEFAULT_DLC;
    testMessage.endianness = CAN_BIG_ENDIAN;
    TEST_ASSERT_FAIL_ASSERT(CAN_RxActionRequest(testMessage, testCanData));
    testMessage.id         = CAN_RX_REQUEST_MESSAGE_ID;
    testMessage.idType     = CAN_STANDARD_IDENTIFIER_11_BIT;
    testMessage.dlc        = CAN_DEFAULT_DLC;
    testMessage.endianness = CAN_LITTLE_ENDIAN;
    TEST_ASSERT_FAIL_ASSERT(CAN_RxActionRequest(testMessage, NULL));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/9: CMD_TO_TRANSFER_PROGRAM, CAN_FSM_STATE_NO_COMMUNICATION */
    /* Reset the message for the following tests */
    testMessage.id         = CAN_RX_REQUEST_MESSAGE_ID;
    testMessage.idType     = CAN_STANDARD_IDENTIFIER_11_BIT;
    testMessage.dlc        = CAN_DEFAULT_DLC;
    testMessage.endianness = CAN_LITTLE_ENDIAN;

    /* Reset the CAN data for the following tests */
    for (uint8_t iByte = 0u; iByte < CAN_DEFAULT_DLC; iByte++) {
        testCanData[iByte] = 0u;
    }

    /* Prepare the can signal that is supposed to be received */
    uint64_t canSignal = CMD_TO_TRANSFER_PROGRAM;

    /* CAN FSM STATE */
    can_stateOfCanCommunication = CAN_FSM_STATE_NO_COMMUNICATION;

    CAN_RxGetMessageDataFromCanData_Expect(&messageData, testCanData, CAN_LITTLE_ENDIAN);

    uint64_t canSignalMock = 0u;
    CAN_RxGetSignalDataFromMessageData_Expect(
        0u,
        CAN_RX_BOOTLOADER_ACTION_REQUEST_BOOTLOADER_ACTION_START_BIT,
        CAN_RX_BOOTLOADER_ACTION_REQUEST_BOOTLOADER_ACTION_LENGTH,
        &canSignalMock,
        CAN_LITTLE_ENDIAN);
    CAN_RxGetSignalDataFromMessageData_ReturnThruPtr_pCanSignal(&canSignal);

    CAN_SendAcknowledgeMessage_ExpectAndReturn(
        RECEIVED, RECEIVED_CMD_TO_TRANSFER_PROGRAM, RECEIVED_BUT_NOT_PROCESSED, NO, STD_OK);
    CAN_SendAcknowledgeMessage_ExpectAndReturn(
        RECEIVED, RECEIVED_CMD_TO_TRANSFER_PROGRAM, RECEIVED_AND_IN_PROCESSING, NO, STD_OK);
    CAN_SendAcknowledgeMessage_ExpectAndReturn(
        RECEIVED, RECEIVED_CMD_TO_TRANSFER_PROGRAM, RECEIVED_AND_PROCESSED, NO, STD_OK);

    retVal = CAN_RxActionRequest(testMessage, testCanData);
    TEST_ASSERT_EQUAL(0u, retVal);
    TEST_ASSERT_EQUAL(CMD_TO_TRANSFER_PROGRAM, canSignal);
    TEST_ASSERT_EQUAL(CAN_FSM_STATE_WAIT_FOR_INFO, can_stateOfCanCommunication);

    /* ======= RT2/9: CMD_TO_TRANSFER_PROGRAM, not CAN_FSM_STATE_NO_COMMUNICATION */
    /* CAN FSM STATE */
    can_stateOfCanCommunication = CAN_FSM_STATE_ERROR;

    CAN_RxGetMessageDataFromCanData_Expect(&messageData, testCanData, CAN_LITTLE_ENDIAN);

    CAN_RxGetSignalDataFromMessageData_Expect(
        0u,
        CAN_RX_BOOTLOADER_ACTION_REQUEST_BOOTLOADER_ACTION_START_BIT,
        CAN_RX_BOOTLOADER_ACTION_REQUEST_BOOTLOADER_ACTION_LENGTH,
        &canSignalMock,
        CAN_LITTLE_ENDIAN);
    CAN_RxGetSignalDataFromMessageData_ReturnThruPtr_pCanSignal(&canSignal);

    CAN_SendAcknowledgeMessage_ExpectAndReturn(
        RECEIVED, RECEIVED_CMD_TO_TRANSFER_PROGRAM, RECEIVED_BUT_NOT_PROCESSED, NO, STD_OK);

    retVal = CAN_RxActionRequest(testMessage, testCanData);
    TEST_ASSERT_EQUAL(0u, retVal);
    TEST_ASSERT_EQUAL(CMD_TO_TRANSFER_PROGRAM, canSignal);
    TEST_ASSERT_EQUAL(CAN_FSM_STATE_ERROR, can_stateOfCanCommunication);

    /* ======= RT3/9: CMD_TO_RUN_PROGRAM, CAN_FSM_STATE_NO_COMMUNICATION */
    /* Prepare the can signal that is supposed to be received */
    canSignal = CMD_TO_RUN_PROGRAM;

    /* CAN FSM STATE */
    can_stateOfCanCommunication = CAN_FSM_STATE_NO_COMMUNICATION;

    CAN_RxGetMessageDataFromCanData_Expect(&messageData, testCanData, CAN_LITTLE_ENDIAN);

    CAN_RxGetSignalDataFromMessageData_Expect(
        0u,
        CAN_RX_BOOTLOADER_ACTION_REQUEST_BOOTLOADER_ACTION_START_BIT,
        CAN_RX_BOOTLOADER_ACTION_REQUEST_BOOTLOADER_ACTION_LENGTH,
        &canSignalMock,
        CAN_LITTLE_ENDIAN);
    CAN_RxGetSignalDataFromMessageData_ReturnThruPtr_pCanSignal(&canSignal);

    CAN_SendAcknowledgeMessage_ExpectAndReturn(
        RECEIVED, RECEIVED_CMD_TO_RUN_PROGRAM, RECEIVED_BUT_NOT_PROCESSED, NO, STD_OK);
    CAN_SendAcknowledgeMessage_ExpectAndReturn(
        RECEIVED, RECEIVED_CMD_TO_RUN_PROGRAM, RECEIVED_AND_IN_PROCESSING, NO, STD_OK);

    retVal = CAN_RxActionRequest(testMessage, testCanData);
    TEST_ASSERT_EQUAL(0u, retVal);
    TEST_ASSERT_EQUAL(CMD_TO_RUN_PROGRAM, canSignal);
    TEST_ASSERT_EQUAL(CAN_FSM_STATE_RUN_PROGRAM, can_stateOfCanCommunication);

    /* ======= RT4/9: CMD_TO_RUN_PROGRAM, not CAN_FSM_STATE_NO_COMMUNICATION */
    /* CAN FSM STATE */
    can_stateOfCanCommunication = CAN_FSM_STATE_ERROR;

    CAN_RxGetMessageDataFromCanData_Expect(&messageData, testCanData, CAN_LITTLE_ENDIAN);

    CAN_RxGetSignalDataFromMessageData_Expect(
        0u,
        CAN_RX_BOOTLOADER_ACTION_REQUEST_BOOTLOADER_ACTION_START_BIT,
        CAN_RX_BOOTLOADER_ACTION_REQUEST_BOOTLOADER_ACTION_LENGTH,
        &canSignalMock,
        CAN_LITTLE_ENDIAN);
    CAN_RxGetSignalDataFromMessageData_ReturnThruPtr_pCanSignal(&canSignal);

    CAN_SendAcknowledgeMessage_ExpectAndReturn(
        RECEIVED, RECEIVED_CMD_TO_RUN_PROGRAM, RECEIVED_BUT_NOT_PROCESSED, NO, STD_OK);

    retVal = CAN_RxActionRequest(testMessage, testCanData);
    TEST_ASSERT_EQUAL(0u, retVal);
    TEST_ASSERT_EQUAL(CMD_TO_RUN_PROGRAM, canSignal);
    TEST_ASSERT_EQUAL(CAN_FSM_STATE_ERROR, can_stateOfCanCommunication);

    /* ======= RT5/9: CMD_TO_RESET_BOOT_PROCESS */
    /* Prepare the can signal that is supposed to be received */
    canSignal = CMD_TO_RESET_BOOT_PROCESS;

    /* CAN FSM STATE */
    can_stateOfCanCommunication = CAN_FSM_STATE_NO_COMMUNICATION;

    CAN_RxGetMessageDataFromCanData_Expect(&messageData, testCanData, CAN_LITTLE_ENDIAN);

    CAN_RxGetSignalDataFromMessageData_Expect(
        0u,
        CAN_RX_BOOTLOADER_ACTION_REQUEST_BOOTLOADER_ACTION_START_BIT,
        CAN_RX_BOOTLOADER_ACTION_REQUEST_BOOTLOADER_ACTION_LENGTH,
        &canSignalMock,
        CAN_LITTLE_ENDIAN);
    CAN_RxGetSignalDataFromMessageData_ReturnThruPtr_pCanSignal(&canSignal);

    CAN_SendAcknowledgeMessage_ExpectAndReturn(
        RECEIVED, RECEIVED_CMD_TO_RESET_BOOT_PROCESS, RECEIVED_BUT_NOT_PROCESSED, NO, STD_OK);
    CAN_SendAcknowledgeMessage_ExpectAndReturn(
        RECEIVED, RECEIVED_CMD_TO_RESET_BOOT_PROCESS, RECEIVED_AND_IN_PROCESSING, NO, STD_OK);

    retVal = CAN_RxActionRequest(testMessage, testCanData);
    TEST_ASSERT_EQUAL(0u, retVal);
    TEST_ASSERT_EQUAL(CMD_TO_RESET_BOOT_PROCESS, canSignal);
    TEST_ASSERT_EQUAL(CAN_FSM_STATE_RESET_BOOT, can_stateOfCanCommunication);

    /* ======= RT6/9: CMD_TO_GET_BOOTLOADER_INFO */
    /* Prepare the can signal that is supposed to be received */
    canSignal = CMD_TO_GET_BOOTLOADER_INFO;

    /* CAN FSM STATE */
    can_stateOfCanCommunication = CAN_FSM_STATE_NO_COMMUNICATION;

    CAN_RxGetMessageDataFromCanData_Expect(&messageData, testCanData, CAN_LITTLE_ENDIAN);

    CAN_RxGetSignalDataFromMessageData_Expect(
        0u,
        CAN_RX_BOOTLOADER_ACTION_REQUEST_BOOTLOADER_ACTION_START_BIT,
        CAN_RX_BOOTLOADER_ACTION_REQUEST_BOOTLOADER_ACTION_LENGTH,
        &canSignalMock,
        CAN_LITTLE_ENDIAN);
    CAN_RxGetSignalDataFromMessageData_ReturnThruPtr_pCanSignal(&canSignal);

    CAN_SendBootloaderInfo_ExpectAndReturn(STD_OK);

    retVal = CAN_RxActionRequest(testMessage, testCanData);
    TEST_ASSERT_EQUAL(0u, retVal);
    TEST_ASSERT_EQUAL(CMD_TO_GET_BOOTLOADER_INFO, canSignal);
    TEST_ASSERT_EQUAL(CAN_FSM_STATE_NO_COMMUNICATION, can_stateOfCanCommunication);

    /* ======= RT7/9: CMD_TO_GET_DATA_TRANSFER_INFO */
    /* Prepare the can signal that is supposed to be received */
    canSignal = CMD_TO_GET_DATA_TRANSFER_INFO;

    /* CAN FSM STATE */
    can_stateOfCanCommunication = CAN_FSM_STATE_NO_COMMUNICATION;

    CAN_RxGetMessageDataFromCanData_Expect(&messageData, testCanData, CAN_LITTLE_ENDIAN);

    CAN_RxGetSignalDataFromMessageData_Expect(
        0u,
        CAN_RX_BOOTLOADER_ACTION_REQUEST_BOOTLOADER_ACTION_START_BIT,
        CAN_RX_BOOTLOADER_ACTION_REQUEST_BOOTLOADER_ACTION_LENGTH,
        &canSignalMock,
        CAN_LITTLE_ENDIAN);
    CAN_RxGetSignalDataFromMessageData_ReturnThruPtr_pCanSignal(&canSignal);

    CAN_SendDataTransferInfo_ExpectAndReturn(STD_OK);

    retVal = CAN_RxActionRequest(testMessage, testCanData);
    TEST_ASSERT_EQUAL(0u, retVal);
    TEST_ASSERT_EQUAL(CMD_TO_GET_DATA_TRANSFER_INFO, canSignal);
    TEST_ASSERT_EQUAL(CAN_FSM_STATE_NO_COMMUNICATION, can_stateOfCanCommunication);

    /* ======= RT8/9: CMD_TO_GET_VERSION_INFO */
    /* Prepare the can signal that is supposed to be received */
    canSignal = CMD_TO_GET_VERSION_INFO;

    /* CAN FSM STATE */
    can_stateOfCanCommunication = CAN_FSM_STATE_NO_COMMUNICATION;

    CAN_RxGetMessageDataFromCanData_Expect(&messageData, testCanData, CAN_LITTLE_ENDIAN);

    CAN_RxGetSignalDataFromMessageData_Expect(
        0u,
        CAN_RX_BOOTLOADER_ACTION_REQUEST_BOOTLOADER_ACTION_START_BIT,
        CAN_RX_BOOTLOADER_ACTION_REQUEST_BOOTLOADER_ACTION_LENGTH,
        &canSignalMock,
        CAN_LITTLE_ENDIAN);
    CAN_RxGetSignalDataFromMessageData_ReturnThruPtr_pCanSignal(&canSignal);

    CAN_SendBootloaderVersionInfo_ExpectAndReturn(STD_OK);
    CAN_SendBootloaderCommitHash_ExpectAndReturn(STD_OK);

    retVal = CAN_RxActionRequest(testMessage, testCanData);
    TEST_ASSERT_EQUAL(0u, retVal);
    TEST_ASSERT_EQUAL(CMD_TO_GET_VERSION_INFO, canSignal);
    TEST_ASSERT_EQUAL(CAN_FSM_STATE_NO_COMMUNICATION, can_stateOfCanCommunication);

    /* ======= RT9/9: not registered request code */
    /* Prepare the can signal that is supposed to be received */
    canSignal = 10u;

    /* CAN FSM STATE */
    can_stateOfCanCommunication = CAN_FSM_STATE_NO_COMMUNICATION;

    CAN_RxGetMessageDataFromCanData_Expect(&messageData, testCanData, CAN_LITTLE_ENDIAN);

    CAN_RxGetSignalDataFromMessageData_Expect(
        0u,
        CAN_RX_BOOTLOADER_ACTION_REQUEST_BOOTLOADER_ACTION_START_BIT,
        CAN_RX_BOOTLOADER_ACTION_REQUEST_BOOTLOADER_ACTION_LENGTH,
        &canSignalMock,
        CAN_LITTLE_ENDIAN);
    CAN_RxGetSignalDataFromMessageData_ReturnThruPtr_pCanSignal(&canSignal);

    retVal = CAN_RxActionRequest(testMessage, testCanData);
    TEST_ASSERT_EQUAL(0u, retVal);
    TEST_ASSERT_EQUAL(10u, canSignal);
    TEST_ASSERT_EQUAL(CAN_FSM_STATE_NO_COMMUNICATION, can_stateOfCanCommunication);
}

void testCAN_RxTransferProcessInfo(void) {
    CAN_MESSAGE_PROPERTIES_s testMessage = {
        .id         = CAN_RX_TRANSFER_PROCESS_INFO_ID,
        .idType     = CAN_STANDARD_IDENTIFIER_11_BIT,
        .dlc        = CAN_DEFAULT_DLC,
        .endianness = CAN_LITTLE_ENDIAN};
    uint8_t testCanData[CAN_DEFAULT_DLC] = {0u};

    /* ======= Assertion tests =============================================== */
    testMessage.id         = 0x100u;
    testMessage.idType     = CAN_STANDARD_IDENTIFIER_11_BIT;
    testMessage.dlc        = CAN_DEFAULT_DLC;
    testMessage.endianness = CAN_LITTLE_ENDIAN;
    TEST_ASSERT_FAIL_ASSERT(CAN_RxTransferProcessInfo(testMessage, testCanData));
    testMessage.id         = CAN_RX_TRANSFER_PROCESS_INFO_ID;
    testMessage.idType     = CAN_INVALID_TYPE;
    testMessage.dlc        = CAN_DEFAULT_DLC;
    testMessage.endianness = CAN_LITTLE_ENDIAN;
    TEST_ASSERT_FAIL_ASSERT(CAN_RxTransferProcessInfo(testMessage, testCanData));
    testMessage.id         = CAN_RX_TRANSFER_PROCESS_INFO_ID;
    testMessage.idType     = CAN_STANDARD_IDENTIFIER_11_BIT;
    testMessage.dlc        = 7u;
    testMessage.endianness = CAN_LITTLE_ENDIAN;
    TEST_ASSERT_FAIL_ASSERT(CAN_RxTransferProcessInfo(testMessage, testCanData));
    testMessage.id         = CAN_RX_TRANSFER_PROCESS_INFO_ID;
    testMessage.idType     = CAN_STANDARD_IDENTIFIER_11_BIT;
    testMessage.dlc        = CAN_DEFAULT_DLC;
    testMessage.endianness = CAN_BIG_ENDIAN;
    TEST_ASSERT_FAIL_ASSERT(CAN_RxTransferProcessInfo(testMessage, testCanData));
    testMessage.id         = CAN_RX_TRANSFER_PROCESS_INFO_ID;
    testMessage.idType     = CAN_STANDARD_IDENTIFIER_11_BIT;
    testMessage.dlc        = CAN_DEFAULT_DLC;
    testMessage.endianness = CAN_LITTLE_ENDIAN;
    TEST_ASSERT_FAIL_ASSERT(CAN_RxTransferProcessInfo(testMessage, NULL));

    /* ======= Routine tests =============================================== */
    uint8_t testData[CAN_MAX_DLC] = {0};

    /* ======= RT1/4: if the current can fsm state is not the one that is required
    by this callback function */
    for (uint8_t canFsmState = 0u; canFsmState < 13; canFsmState++) {
        can_stateOfCanCommunication = canFsmState;

        if (can_stateOfCanCommunication == CAN_FSM_STATE_WAIT_FOR_INFO) {
            continue;
        }
        CAN_SendAcknowledgeMessage_ExpectAndReturn(
            RECEIVED, RECEIVED_PROGRAM_INFO, RECEIVED_BUT_NOT_PROCESSED, NO, STD_OK);

        CAN_RxTransferProcessInfo(testMessage, testData);
        TEST_ASSERT_EQUAL(canFsmState, can_stateOfCanCommunication);
    }

    /* ======= RT2/4: if the current can fsm state is the one that is required
    by this callback function, the received program has no larger size than
    BOOT_PROGRAM_SIZE_MAX, and the ratio between the length of program in bytes
    and the number of transfer loops is right  */
    can_stateOfCanCommunication                        = CAN_FSM_STATE_WAIT_FOR_INFO;
    can_infoOfDataTransfer.totalNumOfDataTransferLoops = 0u;
    can_infoOfDataTransfer.programLengthInBytes        = 0u;
    CAN_SendAcknowledgeMessage_ExpectAndReturn(RECEIVED, RECEIVED_PROGRAM_INFO, RECEIVED_BUT_NOT_PROCESSED, NO, STD_OK);
    CAN_SendAcknowledgeMessage_ExpectAndReturn(RECEIVED, RECEIVED_PROGRAM_INFO, RECEIVED_AND_IN_PROCESSING, NO, STD_OK);

    uint64_t messageData = 0u;
    uint64_t canSignal   = 0u;
    CAN_RxGetMessageDataFromCanData_Expect(&messageData, testData, testMessage.endianness);
    CAN_RxGetMessageDataFromCanData_ReturnThruPtr_pMessage(&messageData);

    uint64_t lenOfProgramInBytes = 0x00200000;
    uint64_t numOfTransferLoops  = 0x00200000 / 8;
    CAN_RxGetSignalDataFromMessageData_Expect(
        messageData,
        CAN_RX_BOOTLOADER_TRANSFER_PROCESS_INFO_PROGRAM_LENGTH_START_BIT,
        CAN_RX_BOOTLOADER_TRANSFER_PROCESS_INFO_PROGRAM_LENGTH_LENGTH,
        &canSignal,
        testMessage.endianness);
    CAN_RxGetSignalDataFromMessageData_ReturnThruPtr_pCanSignal(&lenOfProgramInBytes);
    CAN_RxGetSignalDataFromMessageData_Expect(
        messageData,
        CAN_RX_BOOTLOADER_TRANSFER_PROCESS_INFO_REQUIRED_TRANSFER_LOOPS_START_BIT,
        CAN_RX_BOOTLOADER_TRANSFER_PROCESS_INFO_REQUIRED_TRANSFER_LOOPS_LENGTH,
        &canSignal,
        testMessage.endianness);
    CAN_RxGetSignalDataFromMessageData_ReturnThruPtr_pCanSignal(&numOfTransferLoops);
    CAN_SendAcknowledgeMessage_ExpectAndReturn(RECEIVED, RECEIVED_PROGRAM_INFO, RECEIVED_AND_PROCESSED, YES, STD_OK);

    CAN_RxTransferProcessInfo(testMessage, testData);
    TEST_ASSERT_EQUAL(CAN_FSM_STATE_WAIT_FOR_DATA_LOOPS, can_stateOfCanCommunication);
    TEST_ASSERT_EQUAL(lenOfProgramInBytes, can_infoOfDataTransfer.programLengthInBytes);
    TEST_ASSERT_EQUAL(numOfTransferLoops, can_infoOfDataTransfer.totalNumOfDataTransferLoops);

    /* ======= RT3/4: if the current can fsm state is the one that is required
    by this callback function, the received program has no larger size than
    BOOT_PROGRAM_SIZE_MAX, but the ratio between the length of program in bytes
    and the number of transfer loops is not right  */
    can_stateOfCanCommunication                        = CAN_FSM_STATE_WAIT_FOR_INFO;
    can_infoOfDataTransfer.totalNumOfDataTransferLoops = 0u;
    can_infoOfDataTransfer.programLengthInBytes        = 0u;
    CAN_SendAcknowledgeMessage_ExpectAndReturn(RECEIVED, RECEIVED_PROGRAM_INFO, RECEIVED_BUT_NOT_PROCESSED, NO, STD_OK);
    CAN_SendAcknowledgeMessage_ExpectAndReturn(RECEIVED, RECEIVED_PROGRAM_INFO, RECEIVED_AND_IN_PROCESSING, NO, STD_OK);

    messageData = 0u;
    canSignal   = 0u;
    CAN_RxGetMessageDataFromCanData_Expect(&messageData, testData, testMessage.endianness);
    CAN_RxGetMessageDataFromCanData_ReturnThruPtr_pMessage(&messageData);

    lenOfProgramInBytes = 0x00200000;
    numOfTransferLoops  = 0x00200000 / 4;
    CAN_RxGetSignalDataFromMessageData_Expect(
        messageData,
        CAN_RX_BOOTLOADER_TRANSFER_PROCESS_INFO_PROGRAM_LENGTH_START_BIT,
        CAN_RX_BOOTLOADER_TRANSFER_PROCESS_INFO_PROGRAM_LENGTH_LENGTH,
        &canSignal,
        testMessage.endianness);
    CAN_RxGetSignalDataFromMessageData_ReturnThruPtr_pCanSignal(&lenOfProgramInBytes);
    CAN_RxGetSignalDataFromMessageData_Expect(
        messageData,
        CAN_RX_BOOTLOADER_TRANSFER_PROCESS_INFO_REQUIRED_TRANSFER_LOOPS_START_BIT,
        CAN_RX_BOOTLOADER_TRANSFER_PROCESS_INFO_REQUIRED_TRANSFER_LOOPS_LENGTH,
        &canSignal,
        testMessage.endianness);
    CAN_RxGetSignalDataFromMessageData_ReturnThruPtr_pCanSignal(&numOfTransferLoops);
    CAN_SendAcknowledgeMessage_ExpectAndReturn(RECEIVED, RECEIVED_PROGRAM_INFO, RECEIVED_AND_PROCESSED, NO, STD_OK);

    CAN_RxTransferProcessInfo(testMessage, testData);
    TEST_ASSERT_EQUAL(CAN_FSM_STATE_WAIT_FOR_INFO, can_stateOfCanCommunication);
    TEST_ASSERT_EQUAL(0u, can_infoOfDataTransfer.programLengthInBytes);
    TEST_ASSERT_EQUAL(0u, can_infoOfDataTransfer.totalNumOfDataTransferLoops);

    /* ======= RT4/4: if the current can fsm state is the one that is required
    by this callback function, the received program has larger size than
    BOOT_PROGRAM_SIZE_MAX, but the ratio between the length of program in bytes
    and the number of transfer loops is right  */
    can_stateOfCanCommunication                        = CAN_FSM_STATE_WAIT_FOR_INFO;
    can_infoOfDataTransfer.totalNumOfDataTransferLoops = 0u;
    can_infoOfDataTransfer.programLengthInBytes        = 0u;
    CAN_SendAcknowledgeMessage_ExpectAndReturn(RECEIVED, RECEIVED_PROGRAM_INFO, RECEIVED_BUT_NOT_PROCESSED, NO, STD_OK);
    CAN_SendAcknowledgeMessage_ExpectAndReturn(RECEIVED, RECEIVED_PROGRAM_INFO, RECEIVED_AND_IN_PROCESSING, NO, STD_OK);

    messageData = 0u;
    canSignal   = 0u;
    CAN_RxGetMessageDataFromCanData_Expect(&messageData, testData, testMessage.endianness);
    CAN_RxGetMessageDataFromCanData_ReturnThruPtr_pMessage(&messageData);

    lenOfProgramInBytes = 0x00400000;
    numOfTransferLoops  = 0x00400000 / 8;
    CAN_RxGetSignalDataFromMessageData_Expect(
        messageData,
        CAN_RX_BOOTLOADER_TRANSFER_PROCESS_INFO_PROGRAM_LENGTH_START_BIT,
        CAN_RX_BOOTLOADER_TRANSFER_PROCESS_INFO_PROGRAM_LENGTH_LENGTH,
        &canSignal,
        testMessage.endianness);
    CAN_RxGetSignalDataFromMessageData_ReturnThruPtr_pCanSignal(&lenOfProgramInBytes);
    CAN_RxGetSignalDataFromMessageData_Expect(
        messageData,
        CAN_RX_BOOTLOADER_TRANSFER_PROCESS_INFO_REQUIRED_TRANSFER_LOOPS_START_BIT,
        CAN_RX_BOOTLOADER_TRANSFER_PROCESS_INFO_REQUIRED_TRANSFER_LOOPS_LENGTH,
        &canSignal,
        testMessage.endianness);
    CAN_RxGetSignalDataFromMessageData_ReturnThruPtr_pCanSignal(&numOfTransferLoops);
    CAN_SendAcknowledgeMessage_ExpectAndReturn(RECEIVED, RECEIVED_PROGRAM_INFO, RECEIVED_AND_PROCESSED, NO, STD_OK);

    CAN_RxTransferProcessInfo(testMessage, testData);
    TEST_ASSERT_EQUAL(CAN_FSM_STATE_WAIT_FOR_INFO, can_stateOfCanCommunication);
    TEST_ASSERT_EQUAL(0u, can_infoOfDataTransfer.programLengthInBytes);
    TEST_ASSERT_EQUAL(0u, can_infoOfDataTransfer.totalNumOfDataTransferLoops);
}

void testCAN_RxLoopInfo(void) {
    CAN_MESSAGE_PROPERTIES_s testMessage = {
        .id         = CAN_RX_LOOP_INFO_ID,
        .idType     = CAN_STANDARD_IDENTIFIER_11_BIT,
        .dlc        = CAN_DEFAULT_DLC,
        .endianness = CAN_LITTLE_ENDIAN};
    uint8_t testCanData[CAN_DEFAULT_DLC] = {0u};

    /* ======= Assertion tests =============================================== */
    testMessage.id         = 0x100u;
    testMessage.idType     = CAN_STANDARD_IDENTIFIER_11_BIT;
    testMessage.dlc        = CAN_DEFAULT_DLC;
    testMessage.endianness = CAN_LITTLE_ENDIAN;
    TEST_ASSERT_FAIL_ASSERT(CAN_RxLoopInfo(testMessage, testCanData));
    testMessage.id         = CAN_RX_LOOP_INFO_ID;
    testMessage.idType     = CAN_INVALID_TYPE;
    testMessage.dlc        = CAN_DEFAULT_DLC;
    testMessage.endianness = CAN_LITTLE_ENDIAN;
    TEST_ASSERT_FAIL_ASSERT(CAN_RxLoopInfo(testMessage, testCanData));
    testMessage.id         = CAN_RX_LOOP_INFO_ID;
    testMessage.idType     = CAN_STANDARD_IDENTIFIER_11_BIT;
    testMessage.dlc        = 7u;
    testMessage.endianness = CAN_LITTLE_ENDIAN;
    TEST_ASSERT_FAIL_ASSERT(CAN_RxLoopInfo(testMessage, testCanData));
    testMessage.id         = CAN_RX_LOOP_INFO_ID;
    testMessage.idType     = CAN_STANDARD_IDENTIFIER_11_BIT;
    testMessage.dlc        = CAN_DEFAULT_DLC;
    testMessage.endianness = CAN_BIG_ENDIAN;
    TEST_ASSERT_FAIL_ASSERT(CAN_RxLoopInfo(testMessage, testCanData));
    testMessage.id         = CAN_RX_LOOP_INFO_ID;
    testMessage.idType     = CAN_STANDARD_IDENTIFIER_11_BIT;
    testMessage.dlc        = CAN_DEFAULT_DLC;
    testMessage.endianness = CAN_LITTLE_ENDIAN;
    TEST_ASSERT_FAIL_ASSERT(CAN_RxLoopInfo(testMessage, NULL));

    /* ======= Routine tests =============================================== */
    uint8_t testData[CAN_MAX_DLC] = {0};
    uint64_t messageData          = 0u;
    uint64_t canSignal            = 0u;
    uint64_t canLoopNumber        = 1u;

    /* ======= RT1/5: if the current can fsm state is not one of the states that
    are required by this callback function -> the can fsm state will not be changed
    by this callback function */
    can_infoOfDataTransfer.numOfCurrentLoop = 0u;

    for (uint8_t canFsmState = 0u; canFsmState < 13u; canFsmState++) {
        can_stateOfCanCommunication = canFsmState;
        if ((can_stateOfCanCommunication == CAN_FSM_STATE_WAIT_FOR_DATA_LOOPS) ||
            (can_stateOfCanCommunication == CAN_FSM_STATE_RECEIVED_LOOP_NUMBER) ||
            (can_stateOfCanCommunication == CAN_FSM_STATE_RECEIVED_8_BYTES_DATA) ||
            (can_stateOfCanCommunication == CAN_FSM_STATE_RECEIVED_8_BYTES_CRC)) {
            continue;
        }

        CAN_RxLoopInfo(testMessage, testData);
        TEST_ASSERT_EQUAL(0u, can_infoOfDataTransfer.numOfCurrentLoop);
        TEST_ASSERT_EQUAL(canFsmState, can_stateOfCanCommunication);
    }

    /* ======= RT2/5: if this is the first received loop number:
    can_infoOfDataTransfer.numOfCurrentLoop == numOfLoops,
    isThisLoopTheFirstInDataLoops=true,
    isJustFinishedValidationStep=false,
    isToRepeatThisLoop=false,
    #isThisLoopTheFirstInSectorLoops=true,
    isEveryThingNormal=false
    */
    can_infoOfDataTransfer.numOfCurrentLoop = 0u;
    can_stateOfCanCommunication             = CAN_FSM_STATE_WAIT_FOR_DATA_LOOPS;

    CAN_RxGetMessageDataFromCanData_Expect(&messageData, testData, testMessage.endianness);
    canLoopNumber = 1u;
    CAN_RxGetSignalDataFromMessageData_Expect(
        messageData,
        CAN_RX_BOOTLOADER_LOOP_INFO_LOOP_NUMBER_START_BIT,
        CAN_RX_BOOTLOADER_LOOP_INFO_LOOP_NUMBER_LENGTH,
        &canSignal,
        testMessage.endianness);
    CAN_RxGetSignalDataFromMessageData_ReturnThruPtr_pCanSignal(&canLoopNumber);
    CAN_CopyCanDataTransferInfo_Expect(&can_infoOfDataTransfer, &can_copyOfInfoOfDataTransfer);

    CAN_RxLoopInfo(testMessage, testData);
    TEST_ASSERT_EQUAL(CAN_FSM_STATE_RECEIVED_LOOP_NUMBER, can_stateOfCanCommunication);
    TEST_ASSERT_EQUAL(1, can_infoOfDataTransfer.numOfCurrentLoop);

    /* ======= RT2/5: if this is the first received loop number, and is a repeat loop number
    can_infoOfDataTransfer.numOfCurrentLoop == numOfLoops,
    isThisLoopTheFirstInDataLoops=false,
    isJustFinishedValidationStep=false,
    isToRepeatThisLoop=true,
    #isThisLoopTheFirstInSectorLoops=false,
    isEveryThingNormal=false
    */
    can_infoOfDataTransfer.numOfCurrentLoop = 1u;
    can_stateOfCanCommunication             = CAN_FSM_STATE_RECEIVED_8_BYTES_DATA;

    CAN_RxGetMessageDataFromCanData_Expect(&messageData, testData, testMessage.endianness);
    canLoopNumber = 1u;
    CAN_RxGetSignalDataFromMessageData_Expect(
        messageData,
        CAN_RX_BOOTLOADER_LOOP_INFO_LOOP_NUMBER_START_BIT,
        CAN_RX_BOOTLOADER_LOOP_INFO_LOOP_NUMBER_LENGTH,
        &canSignal,
        testMessage.endianness);
    CAN_RxGetSignalDataFromMessageData_ReturnThruPtr_pCanSignal(&canLoopNumber);

    CAN_RxLoopInfo(testMessage, testData);
    TEST_ASSERT_EQUAL(CAN_FSM_STATE_RECEIVED_LOOP_NUMBER, can_stateOfCanCommunication);
    TEST_ASSERT_EQUAL(1, can_infoOfDataTransfer.numOfCurrentLoop);

    /* ======= RT3/5: just fished validation for one sector:
    can_infoOfDataTransfer.numOfCurrentLoop == numOfLoops,
    isThisLoopTheFirstInDataLoops=false,
    isJustFinishedValidationStep=true,
    isToRepeatThisLoop=false,
    #isThisLoopTheFirstInSectorLoops=true,
    isEveryThingNormal=false
    */
    can_infoOfDataTransfer.numOfCurrentLoop = 1025u;
    can_stateOfCanCommunication             = CAN_FSM_STATE_RECEIVED_8_BYTES_CRC;

    CAN_RxGetMessageDataFromCanData_Expect(&messageData, testData, testMessage.endianness);
    canLoopNumber = 1025u;
    CAN_RxGetSignalDataFromMessageData_Expect(
        messageData,
        CAN_RX_BOOTLOADER_LOOP_INFO_LOOP_NUMBER_START_BIT,
        CAN_RX_BOOTLOADER_LOOP_INFO_LOOP_NUMBER_LENGTH,
        &canSignal,
        testMessage.endianness);
    CAN_RxGetSignalDataFromMessageData_ReturnThruPtr_pCanSignal(&canLoopNumber);
    CAN_CopyCanDataTransferInfo_Expect(&can_infoOfDataTransfer, &can_copyOfInfoOfDataTransfer);

    CAN_RxLoopInfo(testMessage, testData);
    TEST_ASSERT_EQUAL(CAN_FSM_STATE_RECEIVED_LOOP_NUMBER, can_stateOfCanCommunication);

    /* ======= RT4/5: this loop number has been sent once but the corresponding data
    has not been received (it could happens if pc side program get stuck after sending
    the loop number and must start sending again), repeat sending the same loop number:
    can_infoOfDataTransfer.numOfCurrentLoop == numOfLoops,
    isThisLoopTheFirstInDataLoops=false,
    isJustFinishedValidationStep=false,
    isToRepeatThisLoop=true,
    #isThisLoopTheFirstInSectorLoops=false,
    isEveryThingNormal=false
    */
    can_infoOfDataTransfer.numOfCurrentLoop = 1025u;
    can_stateOfCanCommunication             = CAN_FSM_STATE_RECEIVED_LOOP_NUMBER;

    CAN_RxGetMessageDataFromCanData_Expect(&messageData, testData, testMessage.endianness);
    canLoopNumber = 1025u;
    CAN_RxGetSignalDataFromMessageData_Expect(
        messageData,
        CAN_RX_BOOTLOADER_LOOP_INFO_LOOP_NUMBER_START_BIT,
        CAN_RX_BOOTLOADER_LOOP_INFO_LOOP_NUMBER_LENGTH,
        &canSignal,
        testMessage.endianness);
    CAN_RxGetSignalDataFromMessageData_ReturnThruPtr_pCanSignal(&canLoopNumber);

    CAN_RxLoopInfo(testMessage, testData);
    TEST_ASSERT_EQUAL(CAN_FSM_STATE_RECEIVED_LOOP_NUMBER, can_stateOfCanCommunication);

    /* ======= RT5/5: normal case: after receiving the previous loop number and the
    corresponding data, the current fsm state will be CAN_FSM_STATE_RECEIVED_8_BYTES_DATA:
    can_infoOfDataTransfer.numOfCurrentLoop == numOfLoops,
    isThisLoopTheFirstInDataLoops=false,
    isJustFinishedValidationStep=false,
    isToRepeatThisLoop=false,
    #isThisLoopTheFirstInSectorLoops=false,
    isEveryThingNormal=true
    */
    can_infoOfDataTransfer.numOfCurrentLoop = 10u;
    can_stateOfCanCommunication             = CAN_FSM_STATE_RECEIVED_8_BYTES_DATA;

    CAN_RxGetMessageDataFromCanData_Expect(&messageData, testData, testMessage.endianness);
    canLoopNumber = 10u;
    CAN_RxGetSignalDataFromMessageData_Expect(
        messageData,
        CAN_RX_BOOTLOADER_LOOP_INFO_LOOP_NUMBER_START_BIT,
        CAN_RX_BOOTLOADER_LOOP_INFO_LOOP_NUMBER_LENGTH,
        &canSignal,
        testMessage.endianness);
    CAN_RxGetSignalDataFromMessageData_ReturnThruPtr_pCanSignal(&canLoopNumber);

    CAN_RxLoopInfo(testMessage, testData);
    TEST_ASSERT_EQUAL(CAN_FSM_STATE_RECEIVED_LOOP_NUMBER, can_stateOfCanCommunication);

    /* ======= RT5/5: the received loop number does not match that is should be,
    others are normal, the current fsm state will be CAN_FSM_STATE_RECEIVED_8_BYTES_DATA:
    can_infoOfDataTransfer.numOfCurrentLoop == numOfLoops,
    isThisLoopTheFirstInDataLoops=false,
    isJustFinishedValidationStep=false,
    isToRepeatThisLoop=false,
    #isThisLoopTheFirstInSectorLoops=false,
    isEveryThingNormal=true
    */
    can_infoOfDataTransfer.numOfCurrentLoop = 10u;
    can_stateOfCanCommunication             = CAN_FSM_STATE_RECEIVED_8_BYTES_DATA;

    CAN_RxGetMessageDataFromCanData_Expect(&messageData, testData, testMessage.endianness);
    canLoopNumber = 12u;
    CAN_RxGetSignalDataFromMessageData_Expect(
        messageData,
        CAN_RX_BOOTLOADER_LOOP_INFO_LOOP_NUMBER_START_BIT,
        CAN_RX_BOOTLOADER_LOOP_INFO_LOOP_NUMBER_LENGTH,
        &canSignal,
        testMessage.endianness);
    CAN_RxGetSignalDataFromMessageData_ReturnThruPtr_pCanSignal(&canLoopNumber);

    CAN_RxLoopInfo(testMessage, testData);
    TEST_ASSERT_EQUAL(CAN_FSM_STATE_RECEIVED_8_BYTES_DATA, can_stateOfCanCommunication);

    /* ======= RT5/5: loop number does match but others not:
    can_infoOfDataTransfer.numOfCurrentLoop == numOfLoops,
    isThisLoopTheFirstInDataLoops=false,
    isJustFinishedValidationStep=false,
    isToRepeatThisLoop=false,
    #isThisLoopTheFirstInSectorLoops=false,
    isEveryThingNormal=false
    */
    can_infoOfDataTransfer.numOfCurrentLoop = 10u;
    can_stateOfCanCommunication             = CAN_FSM_STATE_WAIT_FOR_DATA_LOOPS;

    CAN_RxGetMessageDataFromCanData_Expect(&messageData, testData, testMessage.endianness);
    canLoopNumber = 10u;
    CAN_RxGetSignalDataFromMessageData_Expect(
        messageData,
        CAN_RX_BOOTLOADER_LOOP_INFO_LOOP_NUMBER_START_BIT,
        CAN_RX_BOOTLOADER_LOOP_INFO_LOOP_NUMBER_LENGTH,
        &canSignal,
        testMessage.endianness);
    CAN_RxGetSignalDataFromMessageData_ReturnThruPtr_pCanSignal(&canLoopNumber);

    CAN_RxLoopInfo(testMessage, testData);
    TEST_ASSERT_EQUAL(CAN_FSM_STATE_WAIT_FOR_DATA_LOOPS, can_stateOfCanCommunication);

    /* ======= RT5/5: loop number match but others not:
    can_infoOfDataTransfer.numOfCurrentLoop == numOfLoops,
    isThisLoopTheFirstInDataLoops=false,
    isJustFinishedValidationStep=false,
    isToRepeatThisLoop=false,
    #isThisLoopTheFirstInSectorLoops=false,
    isEveryThingNormal=false
    */
    can_infoOfDataTransfer.numOfCurrentLoop = 1;
    can_stateOfCanCommunication             = CAN_FSM_STATE_RECEIVED_8_BYTES_DATA;

    CAN_RxGetMessageDataFromCanData_Expect(&messageData, testData, testMessage.endianness);
    canLoopNumber = 5;
    CAN_RxGetSignalDataFromMessageData_Expect(
        messageData,
        CAN_RX_BOOTLOADER_LOOP_INFO_LOOP_NUMBER_START_BIT,
        CAN_RX_BOOTLOADER_LOOP_INFO_LOOP_NUMBER_LENGTH,
        &canSignal,
        testMessage.endianness);
    CAN_RxGetSignalDataFromMessageData_ReturnThruPtr_pCanSignal(&canLoopNumber);

    CAN_RxLoopInfo(testMessage, testData);
    TEST_ASSERT_EQUAL(CAN_FSM_STATE_RECEIVED_8_BYTES_DATA, can_stateOfCanCommunication);

    /* ======= RT5/5: loop number match but others not:
    can_infoOfDataTransfer.numOfCurrentLoop == numOfLoops,
    isThisLoopTheFirstInDataLoops=false,
    isJustFinishedValidationStep=true,
    isToRepeatThisLoop=false,
    #isThisLoopTheFirstInSectorLoops=true,
    isEveryThingNormal=false
    */
    can_infoOfDataTransfer.numOfCurrentLoop = 1;
    can_stateOfCanCommunication             = CAN_FSM_STATE_RECEIVED_8_BYTES_CRC;

    CAN_RxGetMessageDataFromCanData_Expect(&messageData, testData, testMessage.endianness);
    canLoopNumber = 1;
    CAN_RxGetSignalDataFromMessageData_Expect(
        messageData,
        CAN_RX_BOOTLOADER_LOOP_INFO_LOOP_NUMBER_START_BIT,
        CAN_RX_BOOTLOADER_LOOP_INFO_LOOP_NUMBER_LENGTH,
        &canSignal,
        testMessage.endianness);
    CAN_RxGetSignalDataFromMessageData_ReturnThruPtr_pCanSignal(&canLoopNumber);
    CAN_CopyCanDataTransferInfo_Expect(&can_infoOfDataTransfer, &can_copyOfInfoOfDataTransfer);

    CAN_RxLoopInfo(testMessage, testData);
    TEST_ASSERT_EQUAL(CAN_FSM_STATE_RECEIVED_LOOP_NUMBER, can_stateOfCanCommunication);
}

void testCAN_RxData8Bytes(void) {
    CAN_MESSAGE_PROPERTIES_s testMessage = {
        .id         = CAN_RX_DATA_8_BYTES_ID,
        .idType     = CAN_STANDARD_IDENTIFIER_11_BIT,
        .dlc        = CAN_DEFAULT_DLC,
        .endianness = CAN_LITTLE_ENDIAN};
    uint8_t testCanData[CAN_DEFAULT_DLC] = {0u};

    /* ======= Assertion tests =============================================== */
    testMessage.id         = 0x100u;
    testMessage.idType     = CAN_STANDARD_IDENTIFIER_11_BIT;
    testMessage.dlc        = CAN_DEFAULT_DLC;
    testMessage.endianness = CAN_LITTLE_ENDIAN;
    TEST_ASSERT_FAIL_ASSERT(CAN_RxData8Bytes(testMessage, testCanData));
    testMessage.id         = CAN_RX_DATA_8_BYTES_ID;
    testMessage.idType     = CAN_INVALID_TYPE;
    testMessage.dlc        = CAN_DEFAULT_DLC;
    testMessage.endianness = CAN_LITTLE_ENDIAN;
    TEST_ASSERT_FAIL_ASSERT(CAN_RxData8Bytes(testMessage, testCanData));
    testMessage.id         = CAN_RX_DATA_8_BYTES_ID;
    testMessage.idType     = CAN_STANDARD_IDENTIFIER_11_BIT;
    testMessage.dlc        = 7u;
    testMessage.endianness = CAN_LITTLE_ENDIAN;
    TEST_ASSERT_FAIL_ASSERT(CAN_RxData8Bytes(testMessage, testCanData));
    testMessage.id         = CAN_RX_DATA_8_BYTES_ID;
    testMessage.idType     = CAN_STANDARD_IDENTIFIER_11_BIT;
    testMessage.dlc        = CAN_DEFAULT_DLC;
    testMessage.endianness = CAN_BIG_ENDIAN;
    TEST_ASSERT_FAIL_ASSERT(CAN_RxData8Bytes(testMessage, testCanData));
    testMessage.id         = CAN_RX_DATA_8_BYTES_ID;
    testMessage.idType     = CAN_STANDARD_IDENTIFIER_11_BIT;
    testMessage.dlc        = CAN_DEFAULT_DLC;
    testMessage.endianness = CAN_LITTLE_ENDIAN;
    TEST_ASSERT_FAIL_ASSERT(CAN_RxData8Bytes(testMessage, NULL));

    /* ======= Routine tests =============================================== */
    uint8_t testData[CAN_MAX_DLC] = {0};

    /* ======= RT1/8: if the current can fsm state is not the one that is required
    by this callback function, nothing will happen */
    for (uint8_t canFsmState = 0u; canFsmState < 13u; canFsmState++) {
        can_stateOfCanCommunication = canFsmState;

        if ((can_stateOfCanCommunication == CAN_FSM_STATE_FINISHED_FINAL_VALIDATION) ||
            (can_stateOfCanCommunication == CAN_FSM_STATE_RECEIVED_LOOP_NUMBER)) {
            continue;
        }

        CAN_RxData8Bytes(testMessage, testData);
        TEST_ASSERT_EQUAL(canFsmState, can_stateOfCanCommunication);
    }

    /* ======= RT2/8: if the current can fsm state is CAN_FSM_STATE_RECEIVED_LOOP_NUMBER,
    can_infoOfDataTransfer.sectorBufferCurrentAddressU8 is in its address range,
    the current loop number is not the last loop number in the current sub sector. */
    can_stateOfCanCommunication                    = CAN_FSM_STATE_RECEIVED_LOOP_NUMBER;
    can_infoOfDataTransfer.programCurrentAddressU8 = BOOT_PROGRAM_START_ADDRESS +
                                                     8 * (BOOT_NUM_OF_LOOPS_IN_ONE_SUB_SECTOR - 2);
    can_infoOfDataTransfer.numOfCurrentLoop             = BOOT_NUM_OF_LOOPS_IN_ONE_SUB_SECTOR - 1;
    can_infoOfDataTransfer.totalNumOfDataTransferLoops  = 10 * BOOT_NUM_OF_LOOPS_IN_ONE_SUB_SECTOR;
    can_infoOfDataTransfer.sectorBufferCurrentAddressU8 = BOOT_SECTOR_BUFFER_START_ADDRESS +
                                                          8 * (BOOT_NUM_OF_LOOPS_IN_ONE_SUB_SECTOR - 2);

    uint64_t messageData = 0u;
    uint64_t canSignal   = 0u;
    CAN_RxGetMessageDataFromCanData_Expect(&messageData, testData, testMessage.endianness);
    CAN_RxGetSignalDataFromMessageData_Expect(
        messageData,
        CAN_RX_BOOTLOADER_DATA_8_BYTES_DATA_START_BIT,
        CAN_RX_BOOTLOADER_DATA_8_BYTES_DATA_LENGTH,
        &canSignal,
        testMessage.endianness);

    CAN_RxData8Bytes(testMessage, testData);
    TEST_ASSERT_EQUAL(BOOT_NUM_OF_LOOPS_IN_ONE_SUB_SECTOR, can_infoOfDataTransfer.numOfCurrentLoop);
    TEST_ASSERT_EQUAL(CAN_FSM_STATE_RECEIVED_8_BYTES_DATA, can_stateOfCanCommunication);
    TEST_ASSERT_EQUAL(
        BOOT_SECTOR_BUFFER_START_ADDRESS + 8 * (BOOT_NUM_OF_LOOPS_IN_ONE_SUB_SECTOR - 1),
        can_infoOfDataTransfer.sectorBufferCurrentAddressU8);
    TEST_ASSERT_EQUAL(
        BOOT_PROGRAM_START_ADDRESS + 8 * (BOOT_NUM_OF_LOOPS_IN_ONE_SUB_SECTOR - 1),
        can_infoOfDataTransfer.programCurrentAddressU8);

    /* ======= RT3/8: if the current can fsm state is CAN_FSM_STATE_RECEIVED_LOOP_NUMBER,
    can_infoOfDataTransfer.sectorBufferCurrentAddressU8 is larger than its upper limit,
    the current loop number is not the last loop number in the current sub sector. */
    can_stateOfCanCommunication                         = CAN_FSM_STATE_RECEIVED_LOOP_NUMBER;
    can_infoOfDataTransfer.numOfCurrentLoop             = BOOT_NUM_OF_LOOPS_IN_ONE_SUB_SECTOR - 1;
    can_infoOfDataTransfer.sectorBufferCurrentAddressU8 = 0x0806FFF9;

    messageData = 0u;
    canSignal   = 0u;
    CAN_RxGetMessageDataFromCanData_Expect(&messageData, testData, testMessage.endianness);
    CAN_RxGetSignalDataFromMessageData_Expect(
        messageData,
        CAN_RX_BOOTLOADER_DATA_8_BYTES_DATA_START_BIT,
        CAN_RX_BOOTLOADER_DATA_8_BYTES_DATA_LENGTH,
        &canSignal,
        testMessage.endianness);

    CAN_RxData8Bytes(testMessage, testData);
    TEST_ASSERT_EQUAL(CAN_FSM_STATE_ERROR, can_stateOfCanCommunication);

    /* ======= RT4/8: if the current can fsm state is CAN_FSM_STATE_RECEIVED_LOOP_NUMBER,
    can_infoOfDataTransfer.sectorBufferCurrentAddressU8 is smaller than its lower limit,
    the current loop number is not the last loop number in the current sub sector. */
    can_stateOfCanCommunication                         = CAN_FSM_STATE_RECEIVED_LOOP_NUMBER;
    can_infoOfDataTransfer.numOfCurrentLoop             = BOOT_NUM_OF_LOOPS_IN_ONE_SUB_SECTOR - 1;
    can_infoOfDataTransfer.sectorBufferCurrentAddressU8 = 0x08020000;

    messageData = 0u;
    canSignal   = 0u;
    CAN_RxGetMessageDataFromCanData_Expect(&messageData, testData, testMessage.endianness);
    CAN_RxGetSignalDataFromMessageData_Expect(
        messageData,
        CAN_RX_BOOTLOADER_DATA_8_BYTES_DATA_START_BIT,
        CAN_RX_BOOTLOADER_DATA_8_BYTES_DATA_LENGTH,
        &canSignal,
        testMessage.endianness);

    CAN_RxData8Bytes(testMessage, testData);
    TEST_ASSERT_EQUAL(CAN_FSM_STATE_ERROR, can_stateOfCanCommunication);

    /* ======= RT5/8: if the current can fsm state is CAN_FSM_STATE_RECEIVED_LOOP_NUMBER,
    can_infoOfDataTransfer.sectorBufferCurrentAddressU8 is in its address range,
    the current loop number is the last loop number in the current sub sector,
    it is not the last loop number among all data loops. */
    can_stateOfCanCommunication                    = CAN_FSM_STATE_RECEIVED_LOOP_NUMBER;
    can_infoOfDataTransfer.programCurrentAddressU8 = BOOT_PROGRAM_START_ADDRESS +
                                                     8 * (BOOT_NUM_OF_LOOPS_IN_ONE_SUB_SECTOR - 1);
    can_infoOfDataTransfer.numOfCurrentLoop             = BOOT_NUM_OF_LOOPS_IN_ONE_SUB_SECTOR;
    can_infoOfDataTransfer.totalNumOfDataTransferLoops  = 10 * BOOT_NUM_OF_LOOPS_IN_ONE_SUB_SECTOR;
    can_infoOfDataTransfer.sectorBufferCurrentAddressU8 = BOOT_SECTOR_BUFFER_START_ADDRESS +
                                                          8 * (BOOT_NUM_OF_LOOPS_IN_ONE_SUB_SECTOR - 1);

    messageData = 0u;
    canSignal   = 0u;
    CAN_RxGetMessageDataFromCanData_Expect(&messageData, testData, testMessage.endianness);
    CAN_RxGetSignalDataFromMessageData_Expect(
        messageData,
        CAN_RX_BOOTLOADER_DATA_8_BYTES_DATA_START_BIT,
        CAN_RX_BOOTLOADER_DATA_8_BYTES_DATA_LENGTH,
        &canSignal,
        testMessage.endianness);
    CAN_SendAcknowledgeMessage_ExpectAndReturn(RECEIVED, RECEIVED_SUB_SECTOR_DATA, RECEIVED_AND_PROCESSED, NO, STD_OK);

    CAN_RxData8Bytes(testMessage, testData);
    TEST_ASSERT_EQUAL(BOOT_NUM_OF_LOOPS_IN_ONE_SUB_SECTOR + 1, can_infoOfDataTransfer.numOfCurrentLoop);
    TEST_ASSERT_EQUAL(CAN_FSM_STATE_RECEIVED_8_BYTES_DATA, can_stateOfCanCommunication);
    TEST_ASSERT_EQUAL(
        BOOT_SECTOR_BUFFER_START_ADDRESS + 8 * BOOT_NUM_OF_LOOPS_IN_ONE_SUB_SECTOR,
        can_infoOfDataTransfer.sectorBufferCurrentAddressU8);
    TEST_ASSERT_EQUAL(
        BOOT_PROGRAM_START_ADDRESS + 8 * BOOT_NUM_OF_LOOPS_IN_ONE_SUB_SECTOR,
        can_infoOfDataTransfer.programCurrentAddressU8);

    /* ======= RT6/8: if the current can fsm state is CAN_FSM_STATE_RECEIVED_LOOP_NUMBER,
    can_infoOfDataTransfer.sectorBufferCurrentAddressU8 is in its address range,
    the current loop number is the last loop number among all data loops. */
    can_stateOfCanCommunication                         = CAN_FSM_STATE_RECEIVED_LOOP_NUMBER;
    can_infoOfDataTransfer.numOfCurrentLoop             = 10 * BOOT_NUM_OF_LOOPS_IN_ONE_SUB_SECTOR;
    can_infoOfDataTransfer.totalNumOfDataTransferLoops  = 10 * BOOT_NUM_OF_LOOPS_IN_ONE_SUB_SECTOR;
    can_infoOfDataTransfer.sectorBufferCurrentAddressU8 = BOOT_SECTOR_BUFFER_START_ADDRESS +
                                                          10 * (BOOT_NUM_OF_LOOPS_IN_ONE_SUB_SECTOR - 1);

    messageData = 0u;
    canSignal   = 0u;
    CAN_RxGetMessageDataFromCanData_Expect(&messageData, testData, testMessage.endianness);
    CAN_RxGetSignalDataFromMessageData_Expect(
        messageData,
        CAN_RX_BOOTLOADER_DATA_8_BYTES_DATA_START_BIT,
        CAN_RX_BOOTLOADER_DATA_8_BYTES_DATA_LENGTH,
        &canSignal,
        testMessage.endianness);
    CAN_SendAcknowledgeMessage_ExpectAndReturn(RECEIVED, RECEIVED_SUB_SECTOR_DATA, RECEIVED_AND_PROCESSED, YES, STD_OK);

    CAN_RxData8Bytes(testMessage, testData);
    TEST_ASSERT_EQUAL(10 * BOOT_NUM_OF_LOOPS_IN_ONE_SUB_SECTOR, can_infoOfDataTransfer.numOfCurrentLoop);
    TEST_ASSERT_EQUAL(CAN_FSM_STATE_RECEIVED_8_BYTES_DATA, can_stateOfCanCommunication);

    /* ======= RT7/8: if the current can fsm state is CAN_FSM_STATE_FINISHED_FINAL_VALIDATION,
    and there is no received vector table until yet. */
    can_stateOfCanCommunication                                  = CAN_FSM_STATE_FINISHED_FINAL_VALIDATION;
    can_infoOfDataTransfer.numOfReceivedVectorTableDataIn64Bytes = 0u;

    messageData = 0u;
    canSignal   = 0u;
    CAN_RxGetMessageDataFromCanData_Expect(&messageData, testData, testMessage.endianness);
    CAN_RxGetSignalDataFromMessageData_Expect(
        messageData,
        CAN_RX_BOOTLOADER_DATA_8_BYTES_DATA_START_BIT,
        CAN_RX_BOOTLOADER_DATA_8_BYTES_DATA_LENGTH,
        &canSignal,
        testMessage.endianness);

    CAN_RxData8Bytes(testMessage, testData);
    TEST_ASSERT_EQUAL(1u, can_infoOfDataTransfer.numOfReceivedVectorTableDataIn64Bytes);
    TEST_ASSERT_EQUAL(CAN_FSM_STATE_FINISHED_FINAL_VALIDATION, can_stateOfCanCommunication);

    /* ======= RT8/8: if the current can fsm state is CAN_FSM_STATE_FINISHED_FINAL_VALIDATION,
    and all parts of vector table have been received. */
    can_stateOfCanCommunication                                  = CAN_FSM_STATE_FINISHED_FINAL_VALIDATION;
    can_infoOfDataTransfer.numOfReceivedVectorTableDataIn64Bytes = 3u;

    messageData = 0u;
    canSignal   = 0u;
    CAN_RxGetMessageDataFromCanData_Expect(&messageData, testData, testMessage.endianness);
    CAN_RxGetSignalDataFromMessageData_Expect(
        messageData,
        CAN_RX_BOOTLOADER_DATA_8_BYTES_DATA_START_BIT,
        CAN_RX_BOOTLOADER_DATA_8_BYTES_DATA_LENGTH,
        &canSignal,
        testMessage.endianness);
    CAN_SendAcknowledgeMessage_ExpectAndReturn(RECEIVED, RECEIVED_VECTOR_TABLE, RECEIVED_AND_PROCESSED, NO, STD_OK);

    CAN_RxData8Bytes(testMessage, testData);
    TEST_ASSERT_EQUAL(4u, can_infoOfDataTransfer.numOfReceivedVectorTableDataIn64Bytes);
    TEST_ASSERT_EQUAL(CAN_FSM_STATE_FINISHED_TRANSFER_VECTOR_TABLE, can_stateOfCanCommunication);
}

void testCAN_RxCrc8Bytes(void) {
    CAN_MESSAGE_PROPERTIES_s testMessage = {
        .id         = CAN_RX_CRC_8_BYTES_ID,
        .idType     = CAN_STANDARD_IDENTIFIER_11_BIT,
        .dlc        = CAN_DEFAULT_DLC,
        .endianness = CAN_LITTLE_ENDIAN};
    uint8_t testCanData[CAN_DEFAULT_DLC] = {0u};

    /* ======= Assertion tests =============================================== */
    testMessage.id         = 0x100u;
    testMessage.idType     = CAN_STANDARD_IDENTIFIER_11_BIT;
    testMessage.dlc        = CAN_DEFAULT_DLC;
    testMessage.endianness = CAN_LITTLE_ENDIAN;
    TEST_ASSERT_FAIL_ASSERT(CAN_RxCrc8Bytes(testMessage, testCanData));
    testMessage.id         = CAN_RX_CRC_8_BYTES_ID;
    testMessage.idType     = CAN_INVALID_TYPE;
    testMessage.dlc        = CAN_DEFAULT_DLC;
    testMessage.endianness = CAN_LITTLE_ENDIAN;
    TEST_ASSERT_FAIL_ASSERT(CAN_RxCrc8Bytes(testMessage, testCanData));
    testMessage.id         = CAN_RX_CRC_8_BYTES_ID;
    testMessage.idType     = CAN_STANDARD_IDENTIFIER_11_BIT;
    testMessage.dlc        = 7u;
    testMessage.endianness = CAN_LITTLE_ENDIAN;
    TEST_ASSERT_FAIL_ASSERT(CAN_RxCrc8Bytes(testMessage, testCanData));
    testMessage.id         = CAN_RX_CRC_8_BYTES_ID;
    testMessage.idType     = CAN_STANDARD_IDENTIFIER_11_BIT;
    testMessage.dlc        = CAN_DEFAULT_DLC;
    testMessage.endianness = CAN_BIG_ENDIAN;
    TEST_ASSERT_FAIL_ASSERT(CAN_RxCrc8Bytes(testMessage, testCanData));
    testMessage.id         = CAN_RX_CRC_8_BYTES_ID;
    testMessage.idType     = CAN_STANDARD_IDENTIFIER_11_BIT;
    testMessage.dlc        = CAN_DEFAULT_DLC;
    testMessage.endianness = CAN_LITTLE_ENDIAN;
    TEST_ASSERT_FAIL_ASSERT(CAN_RxCrc8Bytes(testMessage, NULL));

    /* ======= Routine tests =============================================== */
    uint8_t testData[CAN_MAX_DLC] = {0};

    /* ======= RT1/7: if the current can fsm state is not the one that is required
    by this callback function, nothing will happen */
    for (uint8_t canFsmState = 0u; canFsmState < 13u; canFsmState++) {
        can_stateOfCanCommunication = canFsmState;

        if ((can_stateOfCanCommunication == CAN_FSM_STATE_FINISHED_TRANSFER_VECTOR_TABLE) ||
            (can_stateOfCanCommunication == CAN_FSM_STATE_RECEIVED_8_BYTES_DATA)) {
            continue;
        }

        CAN_RxCrc8Bytes(testMessage, testData);
        TEST_ASSERT_EQUAL(canFsmState, can_stateOfCanCommunication);
    }

    /* ======= RT2/7: if the transfer of the current sector (the first sector to
    be flashed) has not been finished, isFinishedTransferCurrentSector=false,
    isThisTheLastDataLoop = false */
    FLASH_FLASH_SECTOR_s testFlashSector = {
        true,
        7u,
        {FLASH_FLASH, Fapi_FlashBank0, (uint32_t *)0x00000000, (uint8_t *)0x001FFFFF},
        (uint32_t *)0x00020000,
        (uint8_t *)0x0003FFFF};

    can_stateOfCanCommunication                          = CAN_FSM_STATE_RECEIVED_8_BYTES_DATA;
    can_infoOfDataTransfer.programCurrentSectorAddressU8 = 0x00020008;
    can_infoOfDataTransfer.sectorBufferCurrentAddressU8  = 0x08030008;
    can_infoOfDataTransfer.numOfCurrentLoop              = 2u;
    can_infoOfDataTransfer.totalNumOfDataTransferLoops   = 32 * BOOT_NUM_OF_LOOPS_IN_ONE_SUB_SECTOR;

    uint64_t messageData = 0u;
    uint64_t canSignal   = 0u;
    CAN_RxGetMessageDataFromCanData_Expect(&messageData, testData, testMessage.endianness);
    CAN_RxGetSignalDataFromMessageData_Expect(
        messageData,
        CAN_RX_BOOTLOADER_CRC_8_BYTES_CRC_START_BIT,
        CAN_RX_BOOTLOADER_CRC_8_BYTES_CRC_LENGTH,
        &canSignal,
        testMessage.endianness);
    _disable_IRQ_interrupt__Expect();
    FLASH_GetFlashSector_ExpectAndReturn(
        (uint32_t *)can_infoOfDataTransfer.programCurrentSectorAddressU8, testFlashSector);
    _enable_IRQ_interrupt__Expect();
    CAN_RxCrc8Bytes(testMessage, testData);
    TEST_ASSERT_EQUAL(CAN_FSM_STATE_RECEIVED_8_BYTES_DATA, can_stateOfCanCommunication);

    /* ======= RT3/7: isFinishedTransferCurrentSector = true,  isThisTheLastDataLoop = false */
    uint64_t crc8Bytes;
    can_stateOfCanCommunication                          = CAN_FSM_STATE_RECEIVED_8_BYTES_DATA;
    can_infoOfDataTransfer.programCurrentSectorAddressU8 = 0x00020000;
    can_infoOfDataTransfer.sectorBufferCurrentAddressU8  = 0x08050000;
    can_infoOfDataTransfer.numOfCurrentLoop              = 16385u;
    can_infoOfDataTransfer.totalNumOfDataTransferLoops   = 32768u;

    CAN_RxGetMessageDataFromCanData_Expect(&messageData, testData, testMessage.endianness);
    CAN_RxGetSignalDataFromMessageData_Expect(
        messageData,
        CAN_RX_BOOTLOADER_CRC_8_BYTES_CRC_START_BIT,
        CAN_RX_BOOTLOADER_CRC_8_BYTES_CRC_LENGTH,
        &canSignal,
        testMessage.endianness);
    CAN_RxGetSignalDataFromMessageData_ReturnThruPtr_pCanSignal(&crc8Bytes);
    _disable_IRQ_interrupt__Expect();
    FLASH_GetFlashSector_ExpectAndReturn(
        (uint32_t *)can_infoOfDataTransfer.programCurrentSectorAddressU8, testFlashSector);
    _enable_IRQ_interrupt__Expect();
    CAN_SendAcknowledgeMessage_ExpectAndReturn(RECEIVED, RECEIVED_8_BYTES_CRC, RECEIVED_AND_IN_PROCESSING, NO, STD_OK);
    crc8Bytes = 123u;
    CAN_RxCrc8Bytes(testMessage, testData);
    TEST_ASSERT_EQUAL(crc8Bytes, can_infoOfDataTransfer.programCrc8Bytes);
    TEST_ASSERT_EQUAL(CAN_FSM_STATE_RECEIVED_8_BYTES_CRC, can_stateOfCanCommunication);

    /* ======= RT4/7: isFinishedTransferCurrentSector = true,  isThisTheLastDataLoop = true  */
    can_stateOfCanCommunication                        = CAN_FSM_STATE_RECEIVED_8_BYTES_DATA;
    can_infoOfDataTransfer.numOfCurrentLoop            = 32768u;
    can_infoOfDataTransfer.totalNumOfDataTransferLoops = 32768u;

    CAN_RxGetMessageDataFromCanData_Expect(&messageData, testData, testMessage.endianness);
    CAN_RxGetSignalDataFromMessageData_Expect(
        messageData,
        CAN_RX_BOOTLOADER_CRC_8_BYTES_CRC_START_BIT,
        CAN_RX_BOOTLOADER_CRC_8_BYTES_CRC_LENGTH,
        &canSignal,
        testMessage.endianness);
    crc8Bytes = 123u;
    CAN_RxGetSignalDataFromMessageData_ReturnThruPtr_pCanSignal(&crc8Bytes);
    _disable_IRQ_interrupt__Expect();
    FLASH_GetFlashSector_ExpectAndReturn(
        (uint32_t *)can_infoOfDataTransfer.programCurrentSectorAddressU8, testFlashSector);
    _enable_IRQ_interrupt__Expect();
    CAN_SendAcknowledgeMessage_ExpectAndReturn(RECEIVED, RECEIVED_8_BYTES_CRC, RECEIVED_AND_IN_PROCESSING, NO, STD_OK);

    CAN_RxCrc8Bytes(testMessage, testData);
    TEST_ASSERT_EQUAL(crc8Bytes, can_infoOfDataTransfer.programCrc8Bytes);
    TEST_ASSERT_EQUAL(CAN_FSM_STATE_RECEIVED_8_BYTES_CRC, can_stateOfCanCommunication);

    /* ======= RT5/7: isFinishedTransferCurrentSector = false,  isThisTheLastDataLoop = true */
    can_stateOfCanCommunication                         = CAN_FSM_STATE_RECEIVED_8_BYTES_DATA;
    can_infoOfDataTransfer.sectorBufferCurrentAddressU8 = 0x08030000;
    can_infoOfDataTransfer.numOfCurrentLoop             = 32768u;
    can_infoOfDataTransfer.totalNumOfDataTransferLoops  = 32768u;

    CAN_RxGetMessageDataFromCanData_Expect(&messageData, testData, testMessage.endianness);
    CAN_RxGetSignalDataFromMessageData_Expect(
        messageData,
        CAN_RX_BOOTLOADER_CRC_8_BYTES_CRC_START_BIT,
        CAN_RX_BOOTLOADER_CRC_8_BYTES_CRC_LENGTH,
        &canSignal,
        testMessage.endianness);
    crc8Bytes = 123u;
    CAN_RxGetSignalDataFromMessageData_ReturnThruPtr_pCanSignal(&crc8Bytes);
    _disable_IRQ_interrupt__Expect();
    FLASH_GetFlashSector_ExpectAndReturn(
        (uint32_t *)can_infoOfDataTransfer.programCurrentSectorAddressU8, testFlashSector);
    _enable_IRQ_interrupt__Expect();
    CAN_SendAcknowledgeMessage_ExpectAndReturn(RECEIVED, RECEIVED_8_BYTES_CRC, RECEIVED_AND_IN_PROCESSING, NO, STD_OK);

    CAN_RxCrc8Bytes(testMessage, testData);
    TEST_ASSERT_EQUAL(crc8Bytes, can_infoOfDataTransfer.programCrc8Bytes);
    TEST_ASSERT_EQUAL(CAN_FSM_STATE_RECEIVED_8_BYTES_CRC, can_stateOfCanCommunication);

    /* ======= RT6/7: if vector table has been received and the can fsm state
    has been changed to CAN_FSM_STATE_FINISHED_TRANSFER_VECTOR_TABLE, meanwhile
    the calculated onboard crc of the vector table is the same as the received
    crc for vector table */
    can_stateOfCanCommunication                        = CAN_FSM_STATE_FINISHED_TRANSFER_VECTOR_TABLE;
    can_infoOfDataTransfer.numOfCurrentLoop            = 32768u;
    can_infoOfDataTransfer.totalNumOfDataTransferLoops = 32768u;
    can_infoOfDataTransfer.crc64OfVectorTable          = 0u;
    can_infoOfDataTransfer.vectorTable[0]              = 121;
    can_infoOfDataTransfer.vectorTable[1]              = 121;
    can_infoOfDataTransfer.vectorTable[2]              = 121;
    can_infoOfDataTransfer.vectorTable[3]              = 121;

    CAN_RxGetMessageDataFromCanData_Expect(&messageData, testData, testMessage.endianness);
    CAN_RxGetSignalDataFromMessageData_Expect(
        messageData,
        CAN_RX_BOOTLOADER_CRC_8_BYTES_CRC_START_BIT,
        CAN_RX_BOOTLOADER_CRC_8_BYTES_CRC_LENGTH,
        &canSignal,
        testMessage.endianness);
    crc8Bytes = 321u;
    CAN_RxGetSignalDataFromMessageData_ReturnThruPtr_pCanSignal(&crc8Bytes);
    CRC_CalculateCrc64_ExpectAndReturn(can_infoOfDataTransfer.vectorTable, BOOT_NUM_OF_VECTOR_TABLE_8_BYTES, 0, 321u);
    CAN_SendAcknowledgeMessage_ExpectAndReturn(
        RECEIVED, RECEIVED_CRC_OF_VECTOR_TABLE, RECEIVED_AND_PROCESSED, YES, STD_OK);

    CAN_RxCrc8Bytes(testMessage, testData);
    TEST_ASSERT_EQUAL(CAN_FSM_STATE_VALIDATED_VECTOR_TABLE, can_stateOfCanCommunication);

    /* ======= RT7/7: if vector table has been received and the can fsm state
    has been changed to CAN_FSM_STATE_FINISHED_TRANSFER_VECTOR_TABLE, but the
    calculated onboard crc of the vector table is not the same as the received
    crc of the vector table */
    can_stateOfCanCommunication                        = CAN_FSM_STATE_FINISHED_TRANSFER_VECTOR_TABLE;
    can_infoOfDataTransfer.numOfCurrentLoop            = 32768u;
    can_infoOfDataTransfer.totalNumOfDataTransferLoops = 32768u;
    can_infoOfDataTransfer.crc64OfVectorTable          = 0u;
    can_infoOfDataTransfer.vectorTable[0]              = 121;
    can_infoOfDataTransfer.vectorTable[1]              = 121;
    can_infoOfDataTransfer.vectorTable[2]              = 121;
    can_infoOfDataTransfer.vectorTable[3]              = 121;

    CAN_RxGetMessageDataFromCanData_Expect(&messageData, testData, testMessage.endianness);
    CAN_RxGetSignalDataFromMessageData_Expect(
        messageData,
        CAN_RX_BOOTLOADER_CRC_8_BYTES_CRC_START_BIT,
        CAN_RX_BOOTLOADER_CRC_8_BYTES_CRC_LENGTH,
        &canSignal,
        testMessage.endianness);
    crc8Bytes = 200u;
    CAN_RxGetSignalDataFromMessageData_ReturnThruPtr_pCanSignal(&crc8Bytes);
    CRC_CalculateCrc64_ExpectAndReturn(can_infoOfDataTransfer.vectorTable, BOOT_NUM_OF_VECTOR_TABLE_8_BYTES, 0, 321u);
    CAN_SendAcknowledgeMessage_ExpectAndReturn(
        RECEIVED, RECEIVED_CRC_OF_VECTOR_TABLE, RECEIVED_AND_PROCESSED, NO, STD_OK);
    CAN_ResetVectorTableRelevantVariables_Expect();

    CAN_RxCrc8Bytes(testMessage, testData);
    TEST_ASSERT_EQUAL(CAN_FSM_STATE_FINISHED_FINAL_VALIDATION, can_stateOfCanCommunication);
}
