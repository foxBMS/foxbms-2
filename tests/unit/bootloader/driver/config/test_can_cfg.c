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
 * @file    test_can_cfg.c
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
#include "Mockcan_cbs.h"

#include "test_assert_helper.h"

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("can_cfg.c")

TEST_INCLUDE_PATH("../../src/bootloader/driver/can")
TEST_INCLUDE_PATH("../../src/bootloader/driver/config")
TEST_INCLUDE_PATH("../../src/bootloader/engine/boot")
TEST_INCLUDE_PATH("../../src/bootloader/engine/can")

/*========== Definitions and Implementations for Unit Test ==================*/

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/

void testCAN_ResetVectorTableRelevantVariables(void) {
    for (uint8_t i = 0; i < 4u; i++) {
        can_infoOfDataTransfer.vectorTable[i] = i;
    }
    can_infoOfDataTransfer.numOfReceivedVectorTableDataIn64Bytes = 1u;
    can_infoOfDataTransfer.crc64OfVectorTable                    = 2u;
    CAN_ResetVectorTableRelevantVariables();
    for (uint8_t i = 0; i < 4u; i++) {
        TEST_ASSERT_EQUAL(0u, can_infoOfDataTransfer.vectorTable[i]);
    }
    TEST_ASSERT_EQUAL(0u, can_infoOfDataTransfer.numOfReceivedVectorTableDataIn64Bytes);
    TEST_ASSERT_EQUAL(0u, can_infoOfDataTransfer.crc64OfVectorTable);
}

void testCAN_ResetCanCommunication(void) {
    can_stateOfCanCommunication                          = CAN_FSM_STATE_WAIT_FOR_INFO;
    can_infoOfDataTransfer.totalNumOfDataTransferLoops   = 1u;
    can_infoOfDataTransfer.numOfCurrentLoop              = 2u;
    can_infoOfDataTransfer.programLengthInBytes          = 3u;
    can_infoOfDataTransfer.programStartAddressU8         = 0xFFu;
    can_infoOfDataTransfer.programCurrentAddressU8       = 0xFFu;
    can_infoOfDataTransfer.programCurrentSectorAddressU8 = 0xFFu;
    can_infoOfDataTransfer.programCrc8Bytes              = 4u;
    can_infoOfDataTransfer.programCrc8BytesOnBoard       = 5u;
    can_infoOfDataTransfer.sectorBufferCurrentAddressU8  = BOOT_SECTOR_BUFFER_END_ADDRESS;

    for (uint8_t i = 0; i < 4u; i++) {
        can_infoOfDataTransfer.vectorTable[i] = i;
    }
    can_infoOfDataTransfer.numOfReceivedVectorTableDataIn64Bytes = 1u;
    can_infoOfDataTransfer.crc64OfVectorTable                    = 2u;

    CAN_ResetCanCommunication();

    TEST_ASSERT_EQUAL(CAN_FSM_STATE_NO_COMMUNICATION, can_stateOfCanCommunication);
    TEST_ASSERT_EQUAL(0u, can_infoOfDataTransfer.totalNumOfDataTransferLoops);
    TEST_ASSERT_EQUAL(0u, can_infoOfDataTransfer.numOfCurrentLoop);
    TEST_ASSERT_EQUAL(0u, can_infoOfDataTransfer.programLengthInBytes);
    TEST_ASSERT_EQUAL(BOOT_PROGRAM_START_ADDRESS, can_infoOfDataTransfer.programStartAddressU8);
    TEST_ASSERT_EQUAL(BOOT_PROGRAM_START_ADDRESS, can_infoOfDataTransfer.programCurrentAddressU8);
    TEST_ASSERT_EQUAL(BOOT_PROGRAM_START_ADDRESS, can_infoOfDataTransfer.programCurrentSectorAddressU8);
    TEST_ASSERT_EQUAL(0u, can_infoOfDataTransfer.programCrc8Bytes);
    TEST_ASSERT_EQUAL(0u, can_infoOfDataTransfer.programCrc8BytesOnBoard);
    TEST_ASSERT_EQUAL(BOOT_SECTOR_BUFFER_START_ADDRESS, can_infoOfDataTransfer.sectorBufferCurrentAddressU8);

    for (uint8_t i = 0; i < 4u; i++) {
        TEST_ASSERT_EQUAL(0u, can_infoOfDataTransfer.vectorTable[i]);
    }
    TEST_ASSERT_EQUAL(0u, can_infoOfDataTransfer.numOfReceivedVectorTableDataIn64Bytes);
    TEST_ASSERT_EQUAL(0u, can_infoOfDataTransfer.crc64OfVectorTable);
}

void testCAN_CopyCanDataTransferInfo(void) {
    CAN_DATA_TRANSFER_INFO_s original = {0, .vectorTable = {0u, 0u, 0u, 0u}};
    CAN_DATA_TRANSFER_INFO_s copy     = {0, .vectorTable = {0u, 0u, 0u, 0u}};
    TEST_ASSERT_FAIL_ASSERT(CAN_CopyCanDataTransferInfo(NULL_PTR, &copy));
    TEST_ASSERT_FAIL_ASSERT(CAN_CopyCanDataTransferInfo(&original, NULL_PTR));

    original.numOfCurrentLoop = 10;
    copy.numOfCurrentLoop     = 20;

    CAN_CopyCanDataTransferInfo(&original, &copy);
    TEST_ASSERT_EQUAL_MEMORY(&original, &copy, sizeof(original));
}
