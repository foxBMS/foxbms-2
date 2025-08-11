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
 * @file    test_boot.c
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
#include "Mockboot_helper.h"
#include "Mockcan.h"
#include "Mockcan_cbs.h"
#include "Mockcan_cfg.h"
#include "Mockcrc.h"
#include "Mockflash.h"
#include "Mockfsystem.h"

#include "boot_cfg.h"

#include "boot.h"

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("boot.c")

TEST_INCLUDE_PATH("../../src/bootloader/driver/can")
TEST_INCLUDE_PATH("../../src/bootloader/driver/config")
TEST_INCLUDE_PATH("../../src/bootloader/main")
TEST_INCLUDE_PATH("../../src/bootloader/main/include")
TEST_INCLUDE_PATH("../../src/bootloader/driver/crc")
TEST_INCLUDE_PATH("../../src/bootloader/driver/flash")
TEST_INCLUDE_PATH("../../src/bootloader/driver/foxmath")
TEST_INCLUDE_PATH("../../src/bootloader/engine/boot")
TEST_INCLUDE_PATH("../../src/bootloader/engine/can")
TEST_INCLUDE_PATH("C:/ti/Hercules/F021 Flash API/02.01.01/include")

/*========== Definitions and Implementations for Unit Test ==================*/
/** The global variable can_stateOfCanCommunication and can_infoOfDataTransfer
 * are defined here, because after mocking the can_cfg.c, the definition of these
 * two global variables do not exist in Mockcan_cfg.c.
  */
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

const FLASH_FLASH_SECTOR_s flash_kFlashSectorsInvalid =
    {false, 0u, {FLASH_FLASH, Fapi_FlashBank0, NULL, NULL}, NULL, NULL};
/** Copy of can data transfer */
CAN_DATA_TRANSFER_INFO_s can_copyOfInfoOfDataTransfer = {0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, {0u, 0u, 0u, 0u}, 0u};

/** This variable stores the number of times the function BOOT_WriteAndValidateCurrentSector
 * has been entered, during the loading session of one sector.
*/
uint8_t boot_numOfCurrentCrcValidation = 0u;

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
void testBOOT_GetBootState(void) {
    /* ======= Routine tests =============================================== */
    /* ======= RT1/13: CAN_FSM_STATE_NO_COMMUNICATION */
    can_stateOfCanCommunication    = CAN_FSM_STATE_NO_COMMUNICATION;
    BOOT_FSM_STATES_e bootFsmState = BOOT_GetBootState();
    TEST_ASSERT_EQUAL(BOOT_FSM_STATE_WAIT, bootFsmState);

    /* ======= RT2/13: CAN_FSM_STATE_WAIT_FOR_INFO */
    can_stateOfCanCommunication = CAN_FSM_STATE_WAIT_FOR_INFO;
    bootFsmState                = BOOT_GetBootState();
    TEST_ASSERT_EQUAL(BOOT_FSM_STATE_LOAD, bootFsmState);

    /* ======= RT3/13: CAN_FSM_STATE_WAIT_FOR_DATA_LOOPS */
    can_stateOfCanCommunication = CAN_FSM_STATE_WAIT_FOR_DATA_LOOPS;
    bootFsmState                = BOOT_GetBootState();
    TEST_ASSERT_EQUAL(BOOT_FSM_STATE_LOAD, bootFsmState);

    /* ======= RT4/13: CAN_FSM_STATE_RECEIVED_LOOP_NUMBER */
    can_stateOfCanCommunication = CAN_FSM_STATE_RECEIVED_LOOP_NUMBER;
    bootFsmState                = BOOT_GetBootState();
    TEST_ASSERT_EQUAL(BOOT_FSM_STATE_LOAD, bootFsmState);

    /* ======= RT5/13: CAN_FSM_STATE_RECEIVED_8_BYTES_DATA */
    can_stateOfCanCommunication = CAN_FSM_STATE_RECEIVED_8_BYTES_DATA;
    bootFsmState                = BOOT_GetBootState();
    TEST_ASSERT_EQUAL(BOOT_FSM_STATE_LOAD, bootFsmState);

    /* ======= RT6/13: CAN_FSM_STATE_RECEIVED_8_BYTES_CRC */
    can_stateOfCanCommunication = CAN_FSM_STATE_RECEIVED_8_BYTES_CRC;
    bootFsmState                = BOOT_GetBootState();
    TEST_ASSERT_EQUAL(BOOT_FSM_STATE_LOAD, bootFsmState);

    /* ======= RT7/13: CAN_FSM_STATE_FINISHED_FINAL_VALIDATION */
    can_stateOfCanCommunication = CAN_FSM_STATE_FINISHED_FINAL_VALIDATION;
    bootFsmState                = BOOT_GetBootState();
    TEST_ASSERT_EQUAL(BOOT_FSM_STATE_LOAD, bootFsmState);

    /* ======= RT8/13: CAN_FSM_STATE_FINISHED_TRANSFER_VECTOR_TABLE */
    can_stateOfCanCommunication = CAN_FSM_STATE_FINISHED_TRANSFER_VECTOR_TABLE;
    bootFsmState                = BOOT_GetBootState();
    TEST_ASSERT_EQUAL(BOOT_FSM_STATE_LOAD, bootFsmState);

    /* ======= RT9/13: CAN_FSM_STATE_VALIDATED_VECTOR_TABLE */
    can_stateOfCanCommunication = CAN_FSM_STATE_VALIDATED_VECTOR_TABLE;
    bootFsmState                = BOOT_GetBootState();
    TEST_ASSERT_EQUAL(BOOT_FSM_STATE_LOAD, bootFsmState);

    /* ======= RT10/13: CAN_FSM_STATE_ERROR */
    can_stateOfCanCommunication = CAN_FSM_STATE_ERROR;
    bootFsmState                = BOOT_GetBootState();
    TEST_ASSERT_EQUAL(BOOT_FSM_STATE_ERROR, bootFsmState);

    /* ======= RT11/13: CAN_FSM_STATE_RESET_BOOT */
    can_stateOfCanCommunication = CAN_FSM_STATE_RESET_BOOT;
    bootFsmState                = BOOT_GetBootState();
    TEST_ASSERT_EQUAL(BOOT_FSM_STATE_RESET, bootFsmState);

    /* ======= RT12/13: CAN_FSM_STATE_RUN_PROGRAM */
    can_stateOfCanCommunication = CAN_FSM_STATE_RUN_PROGRAM;
    bootFsmState                = BOOT_GetBootState();
    TEST_ASSERT_EQUAL(BOOT_FSM_STATE_RUN, bootFsmState);

    /* ======= RT13/13: CAN_FSM_STATE_RUN_PROGRAM */
    can_stateOfCanCommunication = 13;
    bootFsmState                = BOOT_GetBootState();
    TEST_ASSERT_EQUAL(BOOT_FSM_STATE_ERROR, bootFsmState);
}

void testBOOT_GetBootStateDuringError(void) {
    /* ======= Routine tests =============================================== */
    /* ======= RT1/12: CAN_FSM_STATE_NO_COMMUNICATION */
    can_stateOfCanCommunication    = CAN_FSM_STATE_NO_COMMUNICATION;
    BOOT_FSM_STATES_e bootFsmState = BOOT_GetBootStateDuringError();
    TEST_ASSERT_EQUAL(BOOT_FSM_STATE_ERROR, bootFsmState);

    /* ======= RT2/12: CAN_FSM_STATE_WAIT_FOR_INFO */
    can_stateOfCanCommunication = CAN_FSM_STATE_WAIT_FOR_INFO;
    bootFsmState                = BOOT_GetBootStateDuringError();
    TEST_ASSERT_EQUAL(BOOT_FSM_STATE_ERROR, bootFsmState);

    /* ======= RT3/12: CAN_FSM_STATE_WAIT_FOR_DATA_LOOPS */
    can_stateOfCanCommunication = CAN_FSM_STATE_WAIT_FOR_DATA_LOOPS;
    bootFsmState                = BOOT_GetBootStateDuringError();
    TEST_ASSERT_EQUAL(BOOT_FSM_STATE_ERROR, bootFsmState);

    /* ======= RT4/12: CAN_FSM_STATE_RECEIVED_LOOP_NUMBER */
    can_stateOfCanCommunication = CAN_FSM_STATE_RECEIVED_LOOP_NUMBER;
    bootFsmState                = BOOT_GetBootStateDuringError();
    TEST_ASSERT_EQUAL(BOOT_FSM_STATE_ERROR, bootFsmState);

    /* ======= RT5/12: CAN_FSM_STATE_RECEIVED_8_BYTES_DATA */
    can_stateOfCanCommunication = CAN_FSM_STATE_RECEIVED_8_BYTES_DATA;
    bootFsmState                = BOOT_GetBootStateDuringError();
    TEST_ASSERT_EQUAL(BOOT_FSM_STATE_ERROR, bootFsmState);

    /* ======= RT6/12: CAN_FSM_STATE_RECEIVED_8_BYTES_CRC */
    can_stateOfCanCommunication = CAN_FSM_STATE_RECEIVED_8_BYTES_CRC;
    bootFsmState                = BOOT_GetBootStateDuringError();
    TEST_ASSERT_EQUAL(BOOT_FSM_STATE_ERROR, bootFsmState);

    /* ======= RT7/12: CAN_FSM_STATE_FINISHED_FINAL_VALIDATION */
    can_stateOfCanCommunication = CAN_FSM_STATE_FINISHED_FINAL_VALIDATION;
    bootFsmState                = BOOT_GetBootStateDuringError();
    TEST_ASSERT_EQUAL(BOOT_FSM_STATE_ERROR, bootFsmState);

    /* ======= RT8/12: CAN_FSM_STATE_FINISHED_TRANSFER_VECTOR_TABLE */
    can_stateOfCanCommunication = CAN_FSM_STATE_FINISHED_TRANSFER_VECTOR_TABLE;
    bootFsmState                = BOOT_GetBootStateDuringError();
    TEST_ASSERT_EQUAL(BOOT_FSM_STATE_ERROR, bootFsmState);

    /* ======= RT9/12: CAN_FSM_STATE_VALIDATED_VECTOR_TABLE */
    can_stateOfCanCommunication = CAN_FSM_STATE_VALIDATED_VECTOR_TABLE;
    bootFsmState                = BOOT_GetBootStateDuringError();
    TEST_ASSERT_EQUAL(BOOT_FSM_STATE_ERROR, bootFsmState);

    /* ======= RT10/12: CAN_FSM_STATE_ERROR */
    can_stateOfCanCommunication = CAN_FSM_STATE_ERROR;
    bootFsmState                = BOOT_GetBootStateDuringError();
    TEST_ASSERT_EQUAL(BOOT_FSM_STATE_ERROR, bootFsmState);

    /* ======= RT11/12: CAN_FSM_STATE_RESET_BOOT */
    can_stateOfCanCommunication = CAN_FSM_STATE_RESET_BOOT;
    bootFsmState                = BOOT_GetBootStateDuringError();
    TEST_ASSERT_EQUAL(BOOT_FSM_STATE_RESET, bootFsmState);

    /* ======= RT12/12: CAN_FSM_STATE_RUN_PROGRAM */
    can_stateOfCanCommunication = CAN_FSM_STATE_RUN_PROGRAM;
    bootFsmState                = BOOT_GetBootStateDuringError();
    TEST_ASSERT_EQUAL(BOOT_FSM_STATE_ERROR, bootFsmState);
}

void testBOOT_GetBootStateDuringLoad(void) {
    /* ======= Routine tests =============================================== */
    FLASH_FLASH_SECTOR_s currentSector = flash_kFlashSectorsInvalid;
    uint32_t lenOfCurrentSector        = (uint32_t)currentSector.pU8SectorAddressEnd -
                                  (uint32_t)currentSector.pU32SectorAddressStart + 1u;
    uint64_t crc_root = can_infoOfDataTransfer.programCrc8BytesOnBoard;

    /* ======= RT1/16: CAN_FSM_STATE_NO_COMMUNICATION */
    can_stateOfCanCommunication    = CAN_FSM_STATE_NO_COMMUNICATION;
    boot_numOfCurrentCrcValidation = 1u;
    _disable_IRQ_interrupt__Expect();
    _enable_IRQ_interrupt__Expect();
    BOOT_FSM_STATES_e bootFsmState = BOOT_GetBootStateDuringLoad();
    TEST_ASSERT_EQUAL(BOOT_FSM_STATE_ERROR, bootFsmState);
    TEST_ASSERT_EQUAL(0u, boot_numOfCurrentCrcValidation);

    /* ======= RT2/16: CAN_FSM_STATE_WAIT_FOR_INFO */
    can_stateOfCanCommunication    = CAN_FSM_STATE_WAIT_FOR_INFO;
    boot_numOfCurrentCrcValidation = 1u;
    _disable_IRQ_interrupt__Expect();
    _enable_IRQ_interrupt__Expect();
    bootFsmState = BOOT_GetBootStateDuringLoad();
    TEST_ASSERT_EQUAL(BOOT_FSM_STATE_LOAD, bootFsmState);
    TEST_ASSERT_EQUAL(0u, boot_numOfCurrentCrcValidation);

    /* ======= RT3/16: CAN_FSM_STATE_WAIT_FOR_DATA_LOOPS */
    can_stateOfCanCommunication    = CAN_FSM_STATE_WAIT_FOR_DATA_LOOPS;
    boot_numOfCurrentCrcValidation = 1u;
    _disable_IRQ_interrupt__Expect();
    _enable_IRQ_interrupt__Expect();
    bootFsmState = BOOT_GetBootStateDuringLoad();
    TEST_ASSERT_EQUAL(BOOT_FSM_STATE_LOAD, bootFsmState);
    TEST_ASSERT_EQUAL(0u, boot_numOfCurrentCrcValidation);

    /* ======= RT4/16: CAN_FSM_STATE_RECEIVED_LOOP_NUMBER */
    can_stateOfCanCommunication    = CAN_FSM_STATE_RECEIVED_LOOP_NUMBER;
    boot_numOfCurrentCrcValidation = 1u;
    _disable_IRQ_interrupt__Expect();
    _enable_IRQ_interrupt__Expect();
    bootFsmState = BOOT_GetBootStateDuringLoad();
    TEST_ASSERT_EQUAL(BOOT_FSM_STATE_LOAD, bootFsmState);
    TEST_ASSERT_EQUAL(0u, boot_numOfCurrentCrcValidation);

    /* ======= RT5/16: CAN_FSM_STATE_RECEIVED_8_BYTES_DATA */
    can_stateOfCanCommunication    = CAN_FSM_STATE_RECEIVED_8_BYTES_DATA;
    boot_numOfCurrentCrcValidation = 1u;
    _disable_IRQ_interrupt__Expect();
    _enable_IRQ_interrupt__Expect();
    bootFsmState = BOOT_GetBootStateDuringLoad();
    TEST_ASSERT_EQUAL(BOOT_FSM_STATE_LOAD, bootFsmState);
    TEST_ASSERT_EQUAL(0u, boot_numOfCurrentCrcValidation);

    /* ======= RT6/16: CAN_FSM_STATE_RECEIVED_8_BYTES_CRC,
    boot_numOfCurrentCrcValidation=0u : the function BOOT_WriteAndValidateCurrentSector
    has not been entered yet */
    can_stateOfCanCommunication    = CAN_FSM_STATE_RECEIVED_8_BYTES_CRC;
    boot_numOfCurrentCrcValidation = 0u;

    FLASH_FLASH_SECTOR_s flashSector = {
        true,
        15u,
        {FLASH_FLASH, Fapi_FlashBank0, (uint32_t *)0x00000000, (uint8_t *)0x001FFFFF},
        (uint32_t *)0x001C0000,
        (uint8_t *)0x001FFFFF};

    _disable_IRQ_interrupt__Expect();
    _disable_IRQ_interrupt__Expect();
    DetectOperationMode_ExpectAndReturn(FSYS_M_BITS_SYSTEM_MODE);
    _disable_IRQ_interrupt__Expect();
    FLASH_GetFlashSector_ExpectAndReturn((uint32_t *)can_infoOfDataTransfer.programCurrentSectorAddressU8, flashSector);
    lenOfCurrentSector = (uint32_t)flashSector.pU8SectorAddressEnd - (uint32_t)flashSector.pU32SectorAddressStart + 1u;
    _disable_IRQ_interrupt__Expect();
    FLASH_WriteFlashSector_ExpectAndReturn(
        (uint32_t *)can_infoOfDataTransfer.programCurrentSectorAddressU8,
        (uint8_t *)BOOT_SECTOR_BUFFER_START_ADDRESS,
        lenOfCurrentSector,
        1u);
    DetectOperationMode_ExpectAndReturn(FSYS_M_BITS_SYSTEM_MODE);
    CRC_SemiAutoCrcCalculation_ExpectAndReturn(
        (uint32_t)can_infoOfDataTransfer.programCurrentSectorAddressU8,
        (uint32_t)(lenOfCurrentSector / BOOT_NUM_OF_BYTES_IN_64_BITS),
        (uint64_t)crc_root,
        0u);
    CAN_CopyCanDataTransferInfo_Expect(&can_copyOfInfoOfDataTransfer, &can_infoOfDataTransfer);
    CAN_SendAcknowledgeMessage_ExpectAndReturn(RECEIVED, RECEIVED_8_BYTES_CRC, RECEIVED_AND_PROCESSED, NO, STD_OK);
    _enable_IRQ_interrupt__Expect();
    bootFsmState = BOOT_GetBootStateDuringLoad();
    TEST_ASSERT_EQUAL(BOOT_FSM_STATE_ERROR, bootFsmState);
    TEST_ASSERT_EQUAL(0u, boot_numOfCurrentCrcValidation);

    /* ======= RT7/16: CAN_FSM_STATE_RECEIVED_8_BYTES_CRC,
    boot_numOfCurrentCrcValidation=0u : the function BOOT_WriteAndValidateCurrentSector
    has already been entered, in this case nothing will happen */
    can_stateOfCanCommunication    = CAN_FSM_STATE_RECEIVED_8_BYTES_CRC;
    boot_numOfCurrentCrcValidation = 1u;
    _disable_IRQ_interrupt__Expect();
    _enable_IRQ_interrupt__Expect();
    bootFsmState = BOOT_GetBootStateDuringLoad();
    TEST_ASSERT_EQUAL(BOOT_FSM_STATE_LOAD, bootFsmState);
    TEST_ASSERT_EQUAL(1u, boot_numOfCurrentCrcValidation);

    /* ======= RT7/16: CAN_FSM_STATE_RECEIVED_8_BYTES_CRC,
    boot_numOfCurrentCrcValidation=0u : the function BOOT_WriteAndValidateCurrentSector
    has been run successfully */
    can_stateOfCanCommunication    = CAN_FSM_STATE_RECEIVED_8_BYTES_CRC;
    boot_numOfCurrentCrcValidation = 0u;
    _disable_IRQ_interrupt__Expect();

    /* BOOT_WriteAndValidateCurrentSector run successfully */
    can_stateOfCanCommunication                          = CAN_FSM_STATE_RECEIVED_8_BYTES_CRC;
    can_infoOfDataTransfer.programCurrentSectorAddressU8 = 0x001C0000;
    can_infoOfDataTransfer.programCrc8Bytes              = 121u;
    can_infoOfDataTransfer.numOfCurrentLoop              = 100u;
    can_infoOfDataTransfer.totalNumOfDataTransferLoops   = 100u;

    _disable_IRQ_interrupt__Expect();

    /* Flash sector that is going to be written and validated */
    DetectOperationMode_ExpectAndReturn(FSYS_M_BITS_SYSTEM_MODE);
    _disable_IRQ_interrupt__Expect();
    FLASH_GetFlashSector_ExpectAndReturn((uint32_t *)can_infoOfDataTransfer.programCurrentSectorAddressU8, flashSector);
    _disable_IRQ_interrupt__Expect();
    FLASH_WriteFlashSector_ExpectAndReturn(
        (uint32_t *)can_infoOfDataTransfer.programCurrentSectorAddressU8,
        (uint8_t *)BOOT_SECTOR_BUFFER_START_ADDRESS,
        (uint32_t)0x40000,
        0u);

    DetectOperationMode_ExpectAndReturn(FSYS_M_BITS_SYSTEM_MODE);
    CRC_SemiAutoCrcCalculation_ExpectAndReturn(
        (uint32_t)can_infoOfDataTransfer.programCurrentSectorAddressU8,
        (uint32_t)(lenOfCurrentSector / BOOT_NUM_OF_BYTES_IN_64_BITS),
        (uint64_t)crc_root,
        121u);
    CAN_SendAcknowledgeMessage_ExpectAndReturn(RECEIVED, RECEIVED_8_BYTES_CRC, RECEIVED_AND_PROCESSED, YES, STD_OK);

    _enable_IRQ_interrupt__Expect();

    bootFsmState = BOOT_GetBootStateDuringLoad();
    TEST_ASSERT_EQUAL(BOOT_FSM_STATE_LOAD, bootFsmState);
    TEST_ASSERT_EQUAL(1u, boot_numOfCurrentCrcValidation);

    /* ======= RT8/16: CAN_FSM_STATE_FINISHED_FINAL_VALIDATION */
    can_stateOfCanCommunication    = CAN_FSM_STATE_FINISHED_FINAL_VALIDATION;
    boot_numOfCurrentCrcValidation = 1u;
    _disable_IRQ_interrupt__Expect();
    _enable_IRQ_interrupt__Expect();
    bootFsmState = BOOT_GetBootStateDuringLoad();
    TEST_ASSERT_EQUAL(BOOT_FSM_STATE_LOAD, bootFsmState);
    TEST_ASSERT_EQUAL(0u, boot_numOfCurrentCrcValidation);

    /* ======= RT9/16: CAN_FSM_STATE_FINISHED_TRANSFER_VECTOR_TABLE */
    can_stateOfCanCommunication    = CAN_FSM_STATE_FINISHED_TRANSFER_VECTOR_TABLE;
    boot_numOfCurrentCrcValidation = 1u;
    _disable_IRQ_interrupt__Expect();
    _enable_IRQ_interrupt__Expect();
    bootFsmState = BOOT_GetBootStateDuringLoad();
    TEST_ASSERT_EQUAL(BOOT_FSM_STATE_LOAD, bootFsmState);
    TEST_ASSERT_EQUAL(0u, boot_numOfCurrentCrcValidation);

    /* ======= RT10/16: CAN_FSM_STATE_VALIDATED_VECTOR_TABLE, there is no program
    loaded previously, there is a vector table received via can module in
    can_infoOfDataTransfer.vectorTable, successfully update the program info
    into flash, but the code after BOOT_SoftwareResetMcu() has been reached */
    can_stateOfCanCommunication                      = CAN_FSM_STATE_VALIDATED_VECTOR_TABLE;
    boot_numOfCurrentCrcValidation                   = 1u;
    boot_infoOfLastFlashedProgram.isProgramAvailable = 0u;
    for (uint8_t i_canVector = 0u; i_canVector < 4u; i_canVector++) {
        can_infoOfDataTransfer.vectorTable[i_canVector] = 1u;
    }
    can_infoOfDataTransfer.vectorTable[0]  = 1u;
    can_infoOfDataTransfer.vectorTable[1]  = 2u;
    can_infoOfDataTransfer.vectorTable[2]  = 3u;
    can_infoOfDataTransfer.vectorTable[3]  = 4u;
    boot_backupVectorTable.vectorTable[0]  = 0u;
    boot_backupVectorTable.vectorTable[1]  = 0u;
    boot_backupVectorTable.vectorTable[2]  = 0u;
    boot_backupVectorTable.vectorTable[3]  = 0u;
    boot_currentVectorTable.vectorTable[0] = 11u;
    boot_currentVectorTable.vectorTable[1] = 12u;
    boot_currentVectorTable.vectorTable[2] = 13u;
    boot_currentVectorTable.vectorTable[3] = 14u;

    _disable_IRQ_interrupt__Expect();
    BOOT_LoadProgramInfoFromFlash_Expect();
    BOOT_UpdateProgramInfoIntoFlash_ExpectAndReturn(STD_OK);
    BOOT_CleanUpRam_Expect();
    BOOT_SoftwareResetMcu_Expect();
    _enable_IRQ_interrupt__Expect();

    bootFsmState = BOOT_GetBootStateDuringLoad();
    TEST_ASSERT_EQUAL(BOOT_FSM_STATE_ERROR, bootFsmState);
    TEST_ASSERT_EQUAL(0u, boot_numOfCurrentCrcValidation);
    /* The vector table of the bootloader should be successfully backed up */
    TEST_ASSERT_EQUAL(11u, boot_backupVectorTable.vectorTable[0]);
    TEST_ASSERT_EQUAL(12u, boot_backupVectorTable.vectorTable[1]);
    TEST_ASSERT_EQUAL(13u, boot_backupVectorTable.vectorTable[2]);
    TEST_ASSERT_EQUAL(14u, boot_backupVectorTable.vectorTable[3]);
    /* The current vector table should be the same as in can_infoOfDataTransfer */
    TEST_ASSERT_EQUAL(can_infoOfDataTransfer.vectorTable[0], boot_currentVectorTable.vectorTable[0]);
    TEST_ASSERT_EQUAL(can_infoOfDataTransfer.vectorTable[1], boot_currentVectorTable.vectorTable[1]);
    TEST_ASSERT_EQUAL(can_infoOfDataTransfer.vectorTable[2], boot_currentVectorTable.vectorTable[2]);
    TEST_ASSERT_EQUAL(can_infoOfDataTransfer.vectorTable[3], boot_currentVectorTable.vectorTable[3]);

    /* ======= RT11/16: CAN_FSM_STATE_VALIDATED_VECTOR_TABLE, there is a program
    loaded previously, there is a vector table received via can module in
    can_infoOfDataTransfer.vectorTable, successfully update the program info
    into flash , but the code after BOOT_SoftwareResetMcu() has been reached */
    can_stateOfCanCommunication                      = CAN_FSM_STATE_VALIDATED_VECTOR_TABLE;
    boot_numOfCurrentCrcValidation                   = 1u;
    boot_infoOfLastFlashedProgram.isProgramAvailable = BOOT_PROGRAM_IS_AVAILABLE;
    can_infoOfDataTransfer.vectorTable[0]            = 1u;
    can_infoOfDataTransfer.vectorTable[1]            = 2u;
    can_infoOfDataTransfer.vectorTable[2]            = 3u;
    can_infoOfDataTransfer.vectorTable[3]            = 4u;
    boot_backupVectorTable.vectorTable[0]            = 21u;
    boot_backupVectorTable.vectorTable[1]            = 22u;
    boot_backupVectorTable.vectorTable[2]            = 23u;
    boot_backupVectorTable.vectorTable[3]            = 24u;
    boot_currentVectorTable.vectorTable[0]           = 11u;
    boot_currentVectorTable.vectorTable[1]           = 12u;
    boot_currentVectorTable.vectorTable[2]           = 13u;
    boot_currentVectorTable.vectorTable[3]           = 14u;

    _disable_IRQ_interrupt__Expect();
    BOOT_LoadProgramInfoFromFlash_Expect();
    BOOT_UpdateProgramInfoIntoFlash_ExpectAndReturn(STD_OK);
    BOOT_CleanUpRam_Expect();
    BOOT_SoftwareResetMcu_Expect();
    _enable_IRQ_interrupt__Expect();

    bootFsmState = BOOT_GetBootStateDuringLoad();
    TEST_ASSERT_EQUAL(BOOT_FSM_STATE_ERROR, bootFsmState);
    TEST_ASSERT_EQUAL(0u, boot_numOfCurrentCrcValidation);
    /* The old backup vector table should be as it is */
    TEST_ASSERT_EQUAL(21u, boot_backupVectorTable.vectorTable[0]);
    TEST_ASSERT_EQUAL(22u, boot_backupVectorTable.vectorTable[1]);
    TEST_ASSERT_EQUAL(23u, boot_backupVectorTable.vectorTable[2]);
    TEST_ASSERT_EQUAL(24u, boot_backupVectorTable.vectorTable[3]);
    /* The current vector table should be the same as in can_infoOfDataTransfer */
    TEST_ASSERT_EQUAL(can_infoOfDataTransfer.vectorTable[0], boot_currentVectorTable.vectorTable[0]);
    TEST_ASSERT_EQUAL(can_infoOfDataTransfer.vectorTable[1], boot_currentVectorTable.vectorTable[1]);
    TEST_ASSERT_EQUAL(can_infoOfDataTransfer.vectorTable[2], boot_currentVectorTable.vectorTable[2]);
    TEST_ASSERT_EQUAL(can_infoOfDataTransfer.vectorTable[3], boot_currentVectorTable.vectorTable[3]);

    /* ======= RT12/16: CAN_FSM_STATE_VALIDATED_VECTOR_TABLE, there is a program
    loaded previously, the values of can_infoOfDataTransfer.vectorTable are however
    all zeros */
    can_stateOfCanCommunication                      = CAN_FSM_STATE_VALIDATED_VECTOR_TABLE;
    boot_numOfCurrentCrcValidation                   = 1u;
    boot_infoOfLastFlashedProgram.isProgramAvailable = BOOT_PROGRAM_IS_AVAILABLE;
    can_infoOfDataTransfer.vectorTable[0]            = 0u;
    can_infoOfDataTransfer.vectorTable[1]            = 0u;
    can_infoOfDataTransfer.vectorTable[2]            = 0u;
    can_infoOfDataTransfer.vectorTable[3]            = 0u;
    boot_backupVectorTable.vectorTable[0]            = 21u;
    boot_backupVectorTable.vectorTable[1]            = 22u;
    boot_backupVectorTable.vectorTable[2]            = 23u;
    boot_backupVectorTable.vectorTable[3]            = 24u;
    boot_currentVectorTable.vectorTable[0]           = 11u;
    boot_currentVectorTable.vectorTable[1]           = 12u;
    boot_currentVectorTable.vectorTable[2]           = 13u;
    boot_currentVectorTable.vectorTable[3]           = 14u;

    _disable_IRQ_interrupt__Expect();
    BOOT_LoadProgramInfoFromFlash_Expect();
    _enable_IRQ_interrupt__Expect();

    bootFsmState = BOOT_GetBootStateDuringLoad();
    TEST_ASSERT_EQUAL(BOOT_FSM_STATE_ERROR, bootFsmState);
    TEST_ASSERT_EQUAL(0u, boot_numOfCurrentCrcValidation);
    /* The old backup vector table should be as it is */
    TEST_ASSERT_EQUAL(21u, boot_backupVectorTable.vectorTable[0]);
    TEST_ASSERT_EQUAL(22u, boot_backupVectorTable.vectorTable[1]);
    TEST_ASSERT_EQUAL(23u, boot_backupVectorTable.vectorTable[2]);
    TEST_ASSERT_EQUAL(24u, boot_backupVectorTable.vectorTable[3]);
    /* The current vector table should not be changed */
    TEST_ASSERT_EQUAL(11u, boot_currentVectorTable.vectorTable[0]);
    TEST_ASSERT_EQUAL(12u, boot_currentVectorTable.vectorTable[1]);
    TEST_ASSERT_EQUAL(13u, boot_currentVectorTable.vectorTable[2]);
    TEST_ASSERT_EQUAL(14u, boot_currentVectorTable.vectorTable[3]);

    /* ======= RT13/16: CAN_FSM_STATE_VALIDATED_VECTOR_TABLE, there is a program
    loaded previously, the values of can_infoOfDataTransfer.vectorTable are not zeros,
    but there are some issues while trying to update the program info into flash */
    can_stateOfCanCommunication                      = CAN_FSM_STATE_VALIDATED_VECTOR_TABLE;
    boot_numOfCurrentCrcValidation                   = 1u;
    boot_infoOfLastFlashedProgram.isProgramAvailable = BOOT_PROGRAM_IS_AVAILABLE;
    can_infoOfDataTransfer.vectorTable[0]            = 1u;
    can_infoOfDataTransfer.vectorTable[1]            = 2u;
    can_infoOfDataTransfer.vectorTable[2]            = 3u;
    can_infoOfDataTransfer.vectorTable[3]            = 4u;
    boot_backupVectorTable.vectorTable[0]            = 21u;
    boot_backupVectorTable.vectorTable[1]            = 22u;
    boot_backupVectorTable.vectorTable[2]            = 23u;
    boot_backupVectorTable.vectorTable[3]            = 24u;
    boot_currentVectorTable.vectorTable[0]           = 11u;
    boot_currentVectorTable.vectorTable[1]           = 12u;
    boot_currentVectorTable.vectorTable[2]           = 13u;
    boot_currentVectorTable.vectorTable[3]           = 14u;

    _disable_IRQ_interrupt__Expect();
    BOOT_LoadProgramInfoFromFlash_Expect();
    BOOT_UpdateProgramInfoIntoFlash_ExpectAndReturn(STD_NOT_OK);
    _enable_IRQ_interrupt__Expect();

    bootFsmState = BOOT_GetBootStateDuringLoad();
    TEST_ASSERT_EQUAL(BOOT_FSM_STATE_ERROR, bootFsmState);
    TEST_ASSERT_EQUAL(0u, boot_numOfCurrentCrcValidation);
    /* The old backup vector table should be as it is */
    TEST_ASSERT_EQUAL(21u, boot_backupVectorTable.vectorTable[0]);
    TEST_ASSERT_EQUAL(22u, boot_backupVectorTable.vectorTable[1]);
    TEST_ASSERT_EQUAL(23u, boot_backupVectorTable.vectorTable[2]);
    TEST_ASSERT_EQUAL(24u, boot_backupVectorTable.vectorTable[3]);
    /* The current vector table should be the same as in can_infoOfDataTransfer */
    TEST_ASSERT_EQUAL(can_infoOfDataTransfer.vectorTable[0], boot_currentVectorTable.vectorTable[0]);
    TEST_ASSERT_EQUAL(can_infoOfDataTransfer.vectorTable[1], boot_currentVectorTable.vectorTable[1]);
    TEST_ASSERT_EQUAL(can_infoOfDataTransfer.vectorTable[2], boot_currentVectorTable.vectorTable[2]);
    TEST_ASSERT_EQUAL(can_infoOfDataTransfer.vectorTable[3], boot_currentVectorTable.vectorTable[3]);

    /* ======= RT14/16: CAN_FSM_STATE_ERROR */
    can_stateOfCanCommunication    = CAN_FSM_STATE_ERROR;
    boot_numOfCurrentCrcValidation = 1u;
    _disable_IRQ_interrupt__Expect();
    _enable_IRQ_interrupt__Expect();
    bootFsmState = BOOT_GetBootStateDuringLoad();
    TEST_ASSERT_EQUAL(BOOT_FSM_STATE_ERROR, bootFsmState);
    TEST_ASSERT_EQUAL(0u, boot_numOfCurrentCrcValidation);

    /* ======= RT15/16: CAN_FSM_STATE_RESET_BOOT */
    can_stateOfCanCommunication    = CAN_FSM_STATE_RESET_BOOT;
    boot_numOfCurrentCrcValidation = 1u;
    _disable_IRQ_interrupt__Expect();
    _enable_IRQ_interrupt__Expect();
    bootFsmState = BOOT_GetBootStateDuringLoad();
    TEST_ASSERT_EQUAL(BOOT_FSM_STATE_RESET, bootFsmState);
    TEST_ASSERT_EQUAL(0u, boot_numOfCurrentCrcValidation);

    /* ======= RT16/16: CAN_FSM_STATE_RUN_PROGRAM */
    can_stateOfCanCommunication    = CAN_FSM_STATE_RUN_PROGRAM;
    boot_numOfCurrentCrcValidation = 1u;
    _disable_IRQ_interrupt__Expect();
    _enable_IRQ_interrupt__Expect();
    bootFsmState = BOOT_GetBootStateDuringLoad();
    TEST_ASSERT_EQUAL(BOOT_FSM_STATE_ERROR, bootFsmState);
    TEST_ASSERT_EQUAL(0u, boot_numOfCurrentCrcValidation);
}

void testBOOT_IsProgramAvailableAndValidated(void) {
    bool retVal;

    /* ======= Routine tests =============================================== */
    /* ======= RT1/5: true case, program is available on board and validation
    step is successful */
    boot_infoOfLastFlashedProgram.isProgramAvailable   = BOOT_PROGRAM_IS_AVAILABLE;
    boot_infoOfLastFlashedProgram.programCrc8Bytes     = 121u;
    boot_infoOfLastFlashedProgram.vectorTableCrc8Bytes = 232u;
    _disable_IRQ_interrupt__Expect();
    BOOT_LoadProgramInfoFromFlash_Expect();
    DetectOperationMode_ExpectAndReturn(FSYS_M_BITS_USER_MODE);
    FSYS_RaisePrivilegeToSystemModeSWI_Expect();
    DetectOperationMode_ExpectAndReturn(FSYS_M_BITS_SYSTEM_MODE);
    CRC_SemiAutoCrcCalculation_ExpectAndReturn(
        (uint32_t)BOOT_PROGRAM_START_ADDRESS,
        (uint32_t)boot_infoOfLastFlashedProgram.programLength / BOOT_NUM_OF_BYTES_IN_64_BITS,
        (uint64_t)0u,
        121u);
    CRC_CalculateCrc64_ExpectAndReturn(
        (const uint64_t *)boot_currentVectorTable.vectorTable, BOOT_NUM_OF_VECTOR_TABLE_8_BYTES, 0u, 232u);
    CAN_SendAcknowledgeMessage_ExpectAndReturn(
        RECEIVED, RECEIVED_CMD_TO_RUN_PROGRAM, RECEIVED_AND_PROCESSED, YES, STD_OK);
    _enable_IRQ_interrupt__Expect();
    retVal = BOOT_IsProgramAvailableAndValidated();
    TEST_ASSERT_EQUAL(true, retVal);

    /* ======= RT2/5: false case, program is available on board but raising privilege
    fails */
    boot_infoOfLastFlashedProgram.isProgramAvailable = BOOT_PROGRAM_IS_AVAILABLE;
    _disable_IRQ_interrupt__Expect();
    BOOT_LoadProgramInfoFromFlash_Expect();
    DetectOperationMode_ExpectAndReturn(FSYS_M_BITS_USER_MODE);
    FSYS_RaisePrivilegeToSystemModeSWI_Expect();
    DetectOperationMode_ExpectAndReturn(FSYS_M_BITS_USER_MODE);
    _enable_IRQ_interrupt__Expect();
    retVal = BOOT_IsProgramAvailableAndValidated();
    TEST_ASSERT_EQUAL(false, retVal);

    /* ======= RT3/5: false case, program is not available on board */
    boot_infoOfLastFlashedProgram.isProgramAvailable = 0u;
    _disable_IRQ_interrupt__Expect();
    BOOT_LoadProgramInfoFromFlash_Expect();
    CAN_SendAcknowledgeMessage_ExpectAndReturn(
        RECEIVED, RECEIVED_CMD_TO_RUN_PROGRAM, RECEIVED_AND_PROCESSED, NO, STD_OK);
    _enable_IRQ_interrupt__Expect();
    retVal = BOOT_IsProgramAvailableAndValidated();
    TEST_ASSERT_EQUAL(false, retVal);

    /* ======= RT4/5: false case, program is available on board but the value of
       received CRC of vector table does not match the one that is calculated on board */
    boot_infoOfLastFlashedProgram.isProgramAvailable   = BOOT_PROGRAM_IS_AVAILABLE;
    boot_infoOfLastFlashedProgram.programCrc8Bytes     = 121u;
    boot_infoOfLastFlashedProgram.vectorTableCrc8Bytes = 232u;
    _disable_IRQ_interrupt__Expect();
    BOOT_LoadProgramInfoFromFlash_Expect();
    DetectOperationMode_ExpectAndReturn(FSYS_M_BITS_SYSTEM_MODE);
    CRC_SemiAutoCrcCalculation_ExpectAndReturn(
        (uint32_t)BOOT_PROGRAM_START_ADDRESS,
        (uint32_t)boot_infoOfLastFlashedProgram.programLength / BOOT_NUM_OF_BYTES_IN_64_BITS,
        (uint64_t)0u,
        121u);
    CRC_CalculateCrc64_ExpectAndReturn(
        (const uint64_t *)boot_currentVectorTable.vectorTable, BOOT_NUM_OF_VECTOR_TABLE_8_BYTES, 0u, 121u);
    CAN_SendAcknowledgeMessage_ExpectAndReturn(
        RECEIVED, RECEIVED_CMD_TO_RUN_PROGRAM, RECEIVED_AND_PROCESSED, NO, STD_OK);
    _enable_IRQ_interrupt__Expect();
    retVal = BOOT_IsProgramAvailableAndValidated();
    TEST_ASSERT_EQUAL(false, retVal);

    /* ======= RT5/5: false case, program is available on board but the value of
       received CRC of program does not match the one that is calculated on board */
    boot_infoOfLastFlashedProgram.isProgramAvailable   = BOOT_PROGRAM_IS_AVAILABLE;
    boot_infoOfLastFlashedProgram.programCrc8Bytes     = 121u;
    boot_infoOfLastFlashedProgram.vectorTableCrc8Bytes = 232u;
    _disable_IRQ_interrupt__Expect();
    BOOT_LoadProgramInfoFromFlash_Expect();
    DetectOperationMode_ExpectAndReturn(FSYS_M_BITS_SYSTEM_MODE);
    CRC_SemiAutoCrcCalculation_ExpectAndReturn(
        (uint32_t)BOOT_PROGRAM_START_ADDRESS,
        (uint32_t)boot_infoOfLastFlashedProgram.programLength / BOOT_NUM_OF_BYTES_IN_64_BITS,
        (uint64_t)0u,
        232u);
    CRC_CalculateCrc64_ExpectAndReturn(
        (const uint64_t *)boot_currentVectorTable.vectorTable, BOOT_NUM_OF_VECTOR_TABLE_8_BYTES, 0u, 232u);
    CAN_SendAcknowledgeMessage_ExpectAndReturn(
        RECEIVED, RECEIVED_CMD_TO_RUN_PROGRAM, RECEIVED_AND_PROCESSED, NO, STD_OK);
    _enable_IRQ_interrupt__Expect();
    retVal = BOOT_IsProgramAvailableAndValidated();
    TEST_ASSERT_EQUAL(false, retVal);
}

void testBOOT_ResetBootloader(void) {
    bool retVal;
    /* ======= Routine tests =============================================== */
    /* ======= RT1/4: BOOT_ResetBootInfo fails -> it leads in real scenario to the
    software reset */
    _disable_IRQ_interrupt__Expect();
    CAN_ResetCanCommunication_Expect();
    BOOT_ResetBootInfo_ExpectAndReturn(STD_NOT_OK);
    BOOT_CleanUpRam_Expect();
    BOOT_SoftwareResetMcu_Expect();
    _enable_IRQ_interrupt__Expect();
    retVal = BOOT_ResetBootloader();
    TEST_ASSERT_EQUAL(STD_NOT_OK, retVal);
    TEST_ASSERT_EQUAL(CAN_FSM_STATE_NO_COMMUNICATION, can_stateOfCanCommunication);

    /* ======= RT2/4: BOOT_ResetBootInfo successful, FSYS_RaisePrivilegeToSystemMode
    not successful  */
    _disable_IRQ_interrupt__Expect();
    CAN_ResetCanCommunication_Expect();
    BOOT_ResetBootInfo_ExpectAndReturn(STD_OK);
    BOOT_CleanUpRam_Expect();

    DetectOperationMode_ExpectAndReturn(FSYS_M_BITS_USER_MODE);
    FSYS_RaisePrivilegeToSystemModeSWI_Expect();
    DetectOperationMode_ExpectAndReturn(FSYS_M_BITS_USER_MODE);

    _enable_IRQ_interrupt__Expect();
    retVal = BOOT_ResetBootloader();
    TEST_ASSERT_EQUAL(STD_NOT_OK, retVal);
    TEST_ASSERT_EQUAL(CAN_FSM_STATE_NO_COMMUNICATION, can_stateOfCanCommunication);

    /* ======= RT3/4: BOOT_ResetBootInfo successful, FSYS_RaisePrivilegeToSystemMode
    successful, FLASH_EraseFlashForApp not successful */
    _disable_IRQ_interrupt__Expect();
    CAN_ResetCanCommunication_Expect();
    BOOT_ResetBootInfo_ExpectAndReturn(STD_OK);
    BOOT_CleanUpRam_Expect();

    DetectOperationMode_ExpectAndReturn(FSYS_M_BITS_SYSTEM_MODE);

    _disable_IRQ_interrupt__Expect();
    FLASH_EraseFlashForApp_ExpectAndReturn(false);

    _enable_IRQ_interrupt__Expect();
    retVal = BOOT_ResetBootloader();
    TEST_ASSERT_EQUAL(STD_NOT_OK, retVal);
    TEST_ASSERT_EQUAL(CAN_FSM_STATE_NO_COMMUNICATION, can_stateOfCanCommunication);

    /* ======= RT4/4: BOOT_ResetBootInfo successful, FSYS_RaisePrivilegeToSystemMode
    successful, FLASH_EraseFlashForApp successful -> in real scenario, PC will
    jump to 0 instead of return STD_NOT_OK */
    _disable_IRQ_interrupt__Expect();
    CAN_ResetCanCommunication_Expect();
    BOOT_ResetBootInfo_ExpectAndReturn(STD_OK);
    BOOT_CleanUpRam_Expect();

    DetectOperationMode_ExpectAndReturn(FSYS_M_BITS_SYSTEM_MODE);

    _disable_IRQ_interrupt__Expect();
    FLASH_EraseFlashForApp_ExpectAndReturn(true);

    BOOT_SoftwareResetMcu_Expect();

    _enable_IRQ_interrupt__Expect();
    retVal = BOOT_ResetBootloader();
    TEST_ASSERT_EQUAL(STD_NOT_OK, retVal);
    TEST_ASSERT_EQUAL(CAN_FSM_STATE_NO_COMMUNICATION, can_stateOfCanCommunication);
}

void testBOOT_WriteAndValidateCurrentSector(void) {
    /* ======= Routine tests =============================================== */
    FLASH_FLASH_SECTOR_s currentSector = flash_kFlashSectorsInvalid;
    uint32_t lenOfCurrentSector        = (uint32_t)currentSector.pU8SectorAddressEnd -
                                  (uint32_t)currentSector.pU32SectorAddressStart + 1u;
    uint64_t crc_root = can_infoOfDataTransfer.programCrc8BytesOnBoard;

    /* Flash sector that is going to be written and validated */
    FLASH_FLASH_SECTOR_s flashSector = {
        true,
        15u,
        {FLASH_FLASH, Fapi_FlashBank0, (uint32_t *)0x00000000, (uint8_t *)0x001FFFFF},
        (uint32_t *)0x001C0000,
        (uint8_t *)0x001FFFFF};

    /* ======= RT1/7: return STD_NOT_OK if raising privilege fails */
    can_stateOfCanCommunication = CAN_FSM_STATE_RECEIVED_8_BYTES_CRC;
    _disable_IRQ_interrupt__Expect();
    DetectOperationMode_ExpectAndReturn(FSYS_M_BITS_USER_MODE);
    FSYS_RaisePrivilegeToSystemModeSWI_Expect();
    DetectOperationMode_ExpectAndReturn(FSYS_M_BITS_USER_MODE);
    STD_RETURN_TYPE_e retVal = TEST_BOOT_WriteAndValidateCurrentSector();
    TEST_ASSERT_EQUAL(CAN_FSM_STATE_RECEIVED_8_BYTES_CRC, can_stateOfCanCommunication);
    TEST_ASSERT_EQUAL(STD_NOT_OK, retVal);

    /* ======= RT2/7: return STD_NOT_OK if obtained flash sector is invalid */
    can_stateOfCanCommunication = CAN_FSM_STATE_RECEIVED_8_BYTES_CRC;
    _disable_IRQ_interrupt__Expect();
    DetectOperationMode_ExpectAndReturn(FSYS_M_BITS_SYSTEM_MODE);
    _disable_IRQ_interrupt__Expect();
    FLASH_GetFlashSector_ExpectAndReturn(
        (uint32_t *)can_infoOfDataTransfer.programCurrentSectorAddressU8, flash_kFlashSectorsInvalid);

    retVal = TEST_BOOT_WriteAndValidateCurrentSector();
    TEST_ASSERT_EQUAL(CAN_FSM_STATE_RECEIVED_8_BYTES_CRC, can_stateOfCanCommunication);
    TEST_ASSERT_EQUAL(STD_NOT_OK, retVal);

    /* ======= RT3/7: write flash successfully, but raising privilege fails before
    calling CRC_SemiAutoCrcCalculation */
    can_stateOfCanCommunication                          = CAN_FSM_STATE_RECEIVED_8_BYTES_CRC;
    can_infoOfDataTransfer.programCurrentSectorAddressU8 = 0x001C0000;
    can_infoOfDataTransfer.programCrc8Bytes              = 121u;
    can_infoOfDataTransfer.numOfCurrentLoop              = 100u;
    can_infoOfDataTransfer.totalNumOfDataTransferLoops   = 100u;

    _disable_IRQ_interrupt__Expect();
    /* Flash sector that is going to be written and validated */
    DetectOperationMode_ExpectAndReturn(FSYS_M_BITS_SYSTEM_MODE);
    _disable_IRQ_interrupt__Expect();
    FLASH_GetFlashSector_ExpectAndReturn((uint32_t *)can_infoOfDataTransfer.programCurrentSectorAddressU8, flashSector);
    _disable_IRQ_interrupt__Expect();
    FLASH_WriteFlashSector_ExpectAndReturn(
        (uint32_t *)can_infoOfDataTransfer.programCurrentSectorAddressU8,
        (uint8_t *)BOOT_SECTOR_BUFFER_START_ADDRESS,
        (uint32_t)0x40000,
        0u);

    DetectOperationMode_ExpectAndReturn(FSYS_M_BITS_USER_MODE);
    FSYS_RaisePrivilegeToSystemModeSWI_Expect();
    DetectOperationMode_ExpectAndReturn(FSYS_M_BITS_USER_MODE);

    retVal = TEST_BOOT_WriteAndValidateCurrentSector();
    TEST_ASSERT_EQUAL(CAN_FSM_STATE_RECEIVED_8_BYTES_CRC, can_stateOfCanCommunication);
    TEST_ASSERT_EQUAL(STD_NOT_OK, retVal);

    /* ======= RT4/7: write flash not successfully, number of current loop is 1
    after the recovery opf can_infoOfDataTransfer by can_copyOfInfoOfDataTransfer. */
    can_stateOfCanCommunication                          = CAN_FSM_STATE_RECEIVED_8_BYTES_CRC;
    can_infoOfDataTransfer.programCurrentSectorAddressU8 = 0x001C0000;
    can_infoOfDataTransfer.programCrc8Bytes              = 121u;
    _disable_IRQ_interrupt__Expect();
    DetectOperationMode_ExpectAndReturn(FSYS_M_BITS_SYSTEM_MODE);
    _disable_IRQ_interrupt__Expect();
    FLASH_GetFlashSector_ExpectAndReturn((uint32_t *)can_infoOfDataTransfer.programCurrentSectorAddressU8, flashSector);
    lenOfCurrentSector = (uint32_t)flashSector.pU8SectorAddressEnd - (uint32_t)flashSector.pU32SectorAddressStart + 1u;
    _disable_IRQ_interrupt__Expect();
    FLASH_WriteFlashSector_ExpectAndReturn(
        (uint32_t *)can_infoOfDataTransfer.programCurrentSectorAddressU8,
        (uint8_t *)BOOT_SECTOR_BUFFER_START_ADDRESS,
        (uint32_t)0x40000u,
        1u);

    DetectOperationMode_ExpectAndReturn(FSYS_M_BITS_SYSTEM_MODE);
    CRC_SemiAutoCrcCalculation_ExpectAndReturn(
        (uint32_t)can_infoOfDataTransfer.programCurrentSectorAddressU8,
        (uint32_t)(lenOfCurrentSector / BOOT_NUM_OF_BYTES_IN_64_BITS),
        (uint64_t)crc_root,
        121u);
    CAN_CopyCanDataTransferInfo_Expect(&can_copyOfInfoOfDataTransfer, &can_infoOfDataTransfer);
    can_infoOfDataTransfer.numOfCurrentLoop = 1u;
    /* Send back a message with 'no' to indicate that this sector is invalid */
    CAN_SendAcknowledgeMessage_ExpectAndReturn(RECEIVED, RECEIVED_8_BYTES_CRC, RECEIVED_AND_PROCESSED, NO, STD_OK);

    retVal = TEST_BOOT_WriteAndValidateCurrentSector();
    TEST_ASSERT_EQUAL(CAN_FSM_STATE_WAIT_FOR_DATA_LOOPS, can_stateOfCanCommunication);
    TEST_ASSERT_EQUAL(STD_NOT_OK, retVal);

    /* ======= RT5/7: write flash successfully, the calculated CRC is not correct,
    the number of the current loop is exactly the same as the total number of data
    transfer loops */
    can_stateOfCanCommunication                          = CAN_FSM_STATE_RECEIVED_8_BYTES_CRC;
    can_infoOfDataTransfer.programCurrentSectorAddressU8 = 0x001C0000;
    can_infoOfDataTransfer.programCrc8Bytes              = 121u;
    can_infoOfDataTransfer.numOfCurrentLoop              = 100u;
    can_infoOfDataTransfer.totalNumOfDataTransferLoops   = 100u;

    _disable_IRQ_interrupt__Expect();
    /* Flash sector that is going to be written and validated */
    DetectOperationMode_ExpectAndReturn(FSYS_M_BITS_SYSTEM_MODE);
    _disable_IRQ_interrupt__Expect();
    FLASH_GetFlashSector_ExpectAndReturn((uint32_t *)can_infoOfDataTransfer.programCurrentSectorAddressU8, flashSector);
    _disable_IRQ_interrupt__Expect();
    FLASH_WriteFlashSector_ExpectAndReturn(
        (uint32_t *)can_infoOfDataTransfer.programCurrentSectorAddressU8,
        (uint8_t *)BOOT_SECTOR_BUFFER_START_ADDRESS,
        (uint32_t)0x40000,
        0u);

    DetectOperationMode_ExpectAndReturn(FSYS_M_BITS_SYSTEM_MODE);
    CRC_SemiAutoCrcCalculation_ExpectAndReturn(
        (uint32_t)can_infoOfDataTransfer.programCurrentSectorAddressU8,
        (uint32_t)(lenOfCurrentSector / BOOT_NUM_OF_BYTES_IN_64_BITS),
        (uint64_t)crc_root,
        200u);
    CAN_CopyCanDataTransferInfo_Expect(&can_copyOfInfoOfDataTransfer, &can_infoOfDataTransfer);
    CAN_SendAcknowledgeMessage_ExpectAndReturn(RECEIVED, RECEIVED_8_BYTES_CRC, RECEIVED_AND_PROCESSED, NO, STD_OK);

    retVal = TEST_BOOT_WriteAndValidateCurrentSector();
    TEST_ASSERT_EQUAL(CAN_FSM_STATE_RECEIVED_8_BYTES_CRC, can_stateOfCanCommunication);
    TEST_ASSERT_EQUAL(STD_NOT_OK, retVal);

    /* ======= RT6/7: write flash successfully, the calculated CRC is also correct,
    the number of the current loop is exactly the same as the total number of data
    transfer loops */
    can_stateOfCanCommunication                          = CAN_FSM_STATE_RECEIVED_8_BYTES_CRC;
    can_infoOfDataTransfer.programCurrentSectorAddressU8 = 0x001C0000;
    can_infoOfDataTransfer.programCrc8Bytes              = 121u;
    can_infoOfDataTransfer.numOfCurrentLoop              = 100u;
    can_infoOfDataTransfer.totalNumOfDataTransferLoops   = 100u;

    _disable_IRQ_interrupt__Expect();
    /* Flash sector that is going to be written and validated */
    DetectOperationMode_ExpectAndReturn(FSYS_M_BITS_SYSTEM_MODE);
    _disable_IRQ_interrupt__Expect();
    FLASH_GetFlashSector_ExpectAndReturn((uint32_t *)can_infoOfDataTransfer.programCurrentSectorAddressU8, flashSector);
    _disable_IRQ_interrupt__Expect();
    FLASH_WriteFlashSector_ExpectAndReturn(
        (uint32_t *)can_infoOfDataTransfer.programCurrentSectorAddressU8,
        (uint8_t *)BOOT_SECTOR_BUFFER_START_ADDRESS,
        (uint32_t)0x40000,
        0u);

    DetectOperationMode_ExpectAndReturn(FSYS_M_BITS_SYSTEM_MODE);
    CRC_SemiAutoCrcCalculation_ExpectAndReturn(
        (uint32_t)can_infoOfDataTransfer.programCurrentSectorAddressU8,
        (uint32_t)(lenOfCurrentSector / BOOT_NUM_OF_BYTES_IN_64_BITS),
        (uint64_t)crc_root,
        121u);
    CAN_SendAcknowledgeMessage_ExpectAndReturn(RECEIVED, RECEIVED_8_BYTES_CRC, RECEIVED_AND_PROCESSED, YES, STD_OK);

    retVal = TEST_BOOT_WriteAndValidateCurrentSector();
    TEST_ASSERT_EQUAL(CAN_FSM_STATE_FINISHED_FINAL_VALIDATION, can_stateOfCanCommunication);
    TEST_ASSERT_EQUAL(STD_OK, retVal);

    /* ======= RT7/7: write flash successfully, the calculated CRC is also correct,
    the number of the current loop does not reach the total number of data
    transfer loops */
    can_stateOfCanCommunication                          = CAN_FSM_STATE_RECEIVED_8_BYTES_CRC;
    can_infoOfDataTransfer.programCurrentSectorAddressU8 = 0x001C0000u;
    can_infoOfDataTransfer.programCrc8Bytes              = 121u;
    can_infoOfDataTransfer.numOfCurrentLoop              = 80u;
    can_infoOfDataTransfer.totalNumOfDataTransferLoops   = 100u;

    _disable_IRQ_interrupt__Expect();
    /* Flash sector that is going to be written and validated */
    DetectOperationMode_ExpectAndReturn(FSYS_M_BITS_SYSTEM_MODE);
    _disable_IRQ_interrupt__Expect();
    FLASH_GetFlashSector_ExpectAndReturn((uint32_t *)can_infoOfDataTransfer.programCurrentSectorAddressU8, flashSector);
    _disable_IRQ_interrupt__Expect();
    FLASH_WriteFlashSector_ExpectAndReturn(
        (uint32_t *)can_infoOfDataTransfer.programCurrentSectorAddressU8,
        (uint8_t *)BOOT_SECTOR_BUFFER_START_ADDRESS,
        (uint32_t)0x40000,
        0u);

    DetectOperationMode_ExpectAndReturn(FSYS_M_BITS_SYSTEM_MODE);
    CRC_SemiAutoCrcCalculation_ExpectAndReturn(
        (uint32_t)can_infoOfDataTransfer.programCurrentSectorAddressU8,
        (uint32_t)(lenOfCurrentSector / BOOT_NUM_OF_BYTES_IN_64_BITS),
        (uint64_t)crc_root,
        121u);
    CAN_SendAcknowledgeMessage_ExpectAndReturn(RECEIVED, RECEIVED_8_BYTES_CRC, RECEIVED_AND_PROCESSED, YES, STD_OK);

    retVal = TEST_BOOT_WriteAndValidateCurrentSector();
    TEST_ASSERT_EQUAL(STD_OK, retVal);
    TEST_ASSERT_EQUAL(CAN_FSM_STATE_RECEIVED_8_BYTES_CRC, can_stateOfCanCommunication);
    TEST_ASSERT_EQUAL(0x200000u, can_infoOfDataTransfer.programCurrentSectorAddressU8);
    TEST_ASSERT_EQUAL(BOOT_SECTOR_BUFFER_START_ADDRESS, can_infoOfDataTransfer.sectorBufferCurrentAddressU8);
}

void testBOOT_JumpInToLastFlashedProgram(void) {
    STD_RETURN_TYPE_e retVal;

    /* ======= RT1/2: raise privilege successfully */
    _disable_IRQ_interrupt__Expect();
    DetectOperationMode_ExpectAndReturn(FSYS_M_BITS_SYSTEM_MODE);
    _memInit__Expect();
    BOOT_CleanUpRam_Expect();
    _cacheDisable__Expect();
    _enable_IRQ_interrupt__Expect();
    retVal = BOOT_JumpInToLastFlashedProgram();
    TEST_ASSERT_EQUAL(STD_NOT_OK, retVal);

    /* ======= RT2/2: raise privilege not successfully */
    _disable_IRQ_interrupt__Expect();
    DetectOperationMode_ExpectAndReturn(FSYS_M_BITS_USER_MODE);
    FSYS_RaisePrivilegeToSystemModeSWI_Expect();
    DetectOperationMode_ExpectAndReturn(FSYS_M_BITS_USER_MODE);
    _enable_IRQ_interrupt__Expect();
    retVal = BOOT_JumpInToLastFlashedProgram();
    TEST_ASSERT_EQUAL(STD_NOT_OK, retVal);
}
