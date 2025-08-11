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
 * @file    test_boot_helper.c
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
#include "Mockflash.h"
#include "Mockfsystem.h"

#include "boot_cfg.h"

#include "boot_helper.h"
#include "fstd_types.h"

#include <string.h>

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("boot_helper.c")

TEST_INCLUDE_PATH("../../src/bootloader/driver/can")
TEST_INCLUDE_PATH("../../src/bootloader/driver/config")
TEST_INCLUDE_PATH("../../src/bootloader/driver/flash")
TEST_INCLUDE_PATH("../../src/bootloader/driver/foxmath")
TEST_INCLUDE_PATH("../../src/bootloader/engine/boot")
TEST_INCLUDE_PATH("../../src/bootloader/main")
TEST_INCLUDE_PATH("../../src/bootloader/main/include")
TEST_INCLUDE_PATH("C:/ti/Hercules/F021 Flash API/02.01.01/include")

/*========== Definitions and Implementations for Unit Test ==================*/
/** Global variable to safely reserve a memory space for program info sector in
 * unittest*/
uint8_t testFlashProgramInfo[BOOT_PROGRAM_INFO_SECTOR_SIZE] = {0u};

/** Global variable to safely reserve a memory space for Sector buffer in unittest */
uint8_t testRamSectorBuffer[BOOT_SECTOR_BUFFER_SIZE] = {0u};

extern volatile uint32_t BOOT_SYSTEM_REG1_SYSECR;

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
void testBOOT_LoadProgramInfoFromFlash(void) {
    /* ======= RT1/2: magic number is right */
    memset((uint8_t *)BOOT_PROGRAM_INFO_ADDRESS_MAGIC_NUM, 0xAAu, BOOT_PROGRAM_INFO_SIZE_MAGIC_NUM);

    /* make sure the magic number has been written */
    for (uint8_t i = 0u; i < BOOT_PROGRAM_INFO_SIZE_MAGIC_NUM; i++) {
        TEST_ASSERT_EQUAL(0xAAu, testFlashProgramInfo[i]);
    }
    TEST_ASSERT_EQUAL(BOOT_PROGRAM_INFO_MAGIC_NUM, *((uint32_t *)BOOT_PROGRAM_INFO_ADDRESS_MAGIC_NUM));

    /* Prepare the program info to be tested */
    uint32_t programLength        = 0x1Bu;
    uint32_t programStartAddress  = 0x1Cu;
    uint64_t programCrc8Bytes     = 0x1Du;
    uint64_t vectorTableCrc8Bytes = 0x1Eu;
    uint32_t isProgramAvailable   = 0x1Fu;
    memset((uint32_t *)BOOT_PROGRAM_INFO_ADDRESS_PROGRAM_LEN, programLength, 1u);
    memset((uint32_t *)BOOT_PROGRAM_INFO_ADDRESS_PROGRAM_START, programStartAddress, 1u);
    memset((uint64_t *)BOOT_PROGRAM_INFO_ADDRESS_CRC_8_BYTES, programCrc8Bytes, 1u);
    memset((uint64_t *)BOOT_PROGRAM_INFO_ADDRESS_VECTOR_CRC_8_BYTES, vectorTableCrc8Bytes, 1u);
    memset((uint32_t *)BOOT_PROGRAM_INFO_ADDRESS_IS_PROGRAM_AVAILABLE, isProgramAvailable, 1u);
    memset((uint32_t *)BOOT_VECTOR_TABLE_BACKUP_ADDRESS, 0xDDu, BOOT_VECTOR_TABLE_SIZE);

    _disable_IRQ_interrupt__Expect();

    BOOT_LoadProgramInfoFromFlash();
    TEST_ASSERT_EQUAL(programLength, boot_infoOfLastFlashedProgram.programLength);
    TEST_ASSERT_EQUAL(programStartAddress, boot_infoOfLastFlashedProgram.programStartAddress);
    TEST_ASSERT_EQUAL(programCrc8Bytes, boot_infoOfLastFlashedProgram.programCrc8Bytes);
    TEST_ASSERT_EQUAL(vectorTableCrc8Bytes, boot_infoOfLastFlashedProgram.vectorTableCrc8Bytes);
    TEST_ASSERT_EQUAL(isProgramAvailable, boot_infoOfLastFlashedProgram.isProgramAvailable);

    /* ======= RT1/2: magic number is wrong */
    memset((uint8_t *)BOOT_PROGRAM_INFO_ADDRESS_MAGIC_NUM, 0x0u, BOOT_PROGRAM_INFO_SIZE_MAGIC_NUM);

    _disable_IRQ_interrupt__Expect();

    BOOT_LoadProgramInfoFromFlash();
    TEST_ASSERT_EQUAL(0u, boot_infoOfLastFlashedProgram.programLength);
    TEST_ASSERT_EQUAL(0u, boot_infoOfLastFlashedProgram.programStartAddress);
    TEST_ASSERT_EQUAL(0u, boot_infoOfLastFlashedProgram.programCrc8Bytes);
    TEST_ASSERT_EQUAL(0u, boot_infoOfLastFlashedProgram.vectorTableCrc8Bytes);
    TEST_ASSERT_EQUAL(0u, boot_infoOfLastFlashedProgram.isProgramAvailable);
}

void testBOOT_CleanUpRam(void) {
    _disable_IRQ_interrupt__Expect();
    BOOT_CleanUpRam();
}

void testBOOT_UpdateProgramInfoIntoFlash(void) {
    STD_RETURN_TYPE_e retVal;

    for (uint8_t iVec = 0u; iVec < 4u; iVec++) {
        boot_backupVectorTable.vectorTable[iVec] = 0xEEu;
    }
    for (uint8_t iVec = 0u; iVec < 4u; iVec++) {
        boot_currentVectorTable.vectorTable[iVec] = 0xEEu;
    }

    /* ======= RT1/4: there is one zero contained in vector table */
    boot_backupVectorTable.vectorTable[0] = 0u;
    _disable_IRQ_interrupt__Expect();
    retVal = BOOT_UpdateProgramInfoIntoFlash();
    TEST_ASSERT_EQUAL(STD_NOT_OK, retVal);

    boot_backupVectorTable.vectorTable[0]  = 0xEEu;
    boot_currentVectorTable.vectorTable[0] = 0u;
    _disable_IRQ_interrupt__Expect();
    retVal = BOOT_UpdateProgramInfoIntoFlash();
    TEST_ASSERT_EQUAL(STD_NOT_OK, retVal);
    boot_currentVectorTable.vectorTable[0] = 0xEEu;

    /* ======= RT2/4: if raise privilege fails */
    _disable_IRQ_interrupt__Expect();
    DetectOperationMode_ExpectAndReturn(FSYS_M_BITS_USER_MODE);
    FSYS_RaisePrivilegeToSystemModeSWI_Expect();
    DetectOperationMode_ExpectAndReturn(FSYS_M_BITS_USER_MODE);
    retVal = BOOT_UpdateProgramInfoIntoFlash();
    TEST_ASSERT_EQUAL(STD_NOT_OK, retVal);

    /* ======= RT3/4: if raise privilege successfully but the return value of
    FLASH_WriteFlashSector is not 0 */
    _disable_IRQ_interrupt__Expect();
    DetectOperationMode_ExpectAndReturn(FSYS_M_BITS_SYSTEM_MODE);
    _disable_IRQ_interrupt__Expect();
    FLASH_WriteFlashSector_ExpectAndReturn(
        (uint32_t *)BOOT_PROGRAM_INFO_SECTOR_ADDRESS,
        (uint8_t *)BOOT_SECTOR_BUFFER_START_ADDRESS,
        BOOT_PROGRAM_INFO_SECTOR_SIZE,
        1u);
    retVal = BOOT_UpdateProgramInfoIntoFlash();
    TEST_ASSERT_EQUAL(STD_NOT_OK, retVal);

    /* ======= RT4/4: if raise privilege successfully and the return value of
    FLASH_WriteFlashSector is 0 */
    for (uint8_t iVec = 0u; iVec < 4u; iVec++) {
        boot_backupVectorTable.vectorTable[0] = 0xEEu;
    }
    for (uint8_t iVec = 0u; iVec < 4u; iVec++) {
        boot_currentVectorTable.vectorTable[0] = 0xEEu;
    }
    _disable_IRQ_interrupt__Expect();
    DetectOperationMode_ExpectAndReturn(FSYS_M_BITS_SYSTEM_MODE);
    _disable_IRQ_interrupt__Expect();
    FLASH_WriteFlashSector_ExpectAndReturn(
        (uint32_t *)BOOT_PROGRAM_INFO_SECTOR_ADDRESS,
        (uint8_t *)BOOT_SECTOR_BUFFER_START_ADDRESS,
        BOOT_PROGRAM_INFO_SECTOR_SIZE,
        0u);
    retVal = BOOT_UpdateProgramInfoIntoFlash();
    TEST_ASSERT_EQUAL(STD_OK, retVal);
}

void testBOOT_ResetBootInfo(void) {
    STD_RETURN_TYPE_e retVal;

    /* Prepare the program info to be tested */
    uint32_t programLength        = 0x1Bu;
    uint32_t programStartAddress  = 0x1Cu;
    uint64_t programCrc8Bytes     = 0x1Du;
    uint64_t vectorTableCrc8Bytes = 0x1Eu;
    uint32_t isProgramAvailable   = 0xCCu;

    memset((uint32_t *)BOOT_PROGRAM_INFO_ADDRESS_PROGRAM_LEN, programLength, 1u);
    memset((uint32_t *)BOOT_PROGRAM_INFO_ADDRESS_PROGRAM_START, programStartAddress, 1u);
    memset((uint64_t *)BOOT_PROGRAM_INFO_ADDRESS_CRC_8_BYTES, programCrc8Bytes, 1u);
    memset((uint64_t *)BOOT_PROGRAM_INFO_ADDRESS_VECTOR_CRC_8_BYTES, vectorTableCrc8Bytes, 1u);
    memset((uint32_t *)BOOT_PROGRAM_INFO_ADDRESS_IS_PROGRAM_AVAILABLE, isProgramAvailable, 1u);
    memset((uint32_t *)BOOT_VECTOR_TABLE_BACKUP_ADDRESS, 0xDDu, BOOT_VECTOR_TABLE_SIZE);
    memset((uint32_t *)BOOT_VECTOR_TABLE_START_ADDRESS, 0xDDu, BOOT_VECTOR_TABLE_SIZE);

    /* ======= RT1/3: The program is not available based on the variable
    boot_infoOfLastFlashedProgram.isProgramAvailable, which is updated
    via the function BOOT_LoadProgramInfoFromFlash. In this case, the
    backup vector table has been set to all zeros, while the variable
    that carries the current vector table maintains the same value.
    Due to the protection mechanism of BOOT_UpdateProgramInfoIntoFlash,
    the program info sector will not be updated. As a result, only the
    relevant global variable will be reset to zeros. */

    _disable_IRQ_interrupt__Expect();

    /* load program info from flash, program is not available */
    memset((uint8_t *)BOOT_PROGRAM_INFO_ADDRESS_MAGIC_NUM, 0u, BOOT_PROGRAM_INFO_SIZE_MAGIC_NUM);
    _disable_IRQ_interrupt__Expect();

    /* updating the program info into flash is not successful */
    _disable_IRQ_interrupt__Expect();

    retVal = BOOT_ResetBootInfo();
    TEST_ASSERT_EQUAL(STD_NOT_OK, retVal);

    TEST_ASSERT_EQUAL(0u, boot_infoOfLastFlashedProgram.programLength);
    TEST_ASSERT_EQUAL(0u, boot_infoOfLastFlashedProgram.programStartAddress);
    TEST_ASSERT_EQUAL(0u, boot_infoOfLastFlashedProgram.programCrc8Bytes);
    TEST_ASSERT_EQUAL(0u, boot_infoOfLastFlashedProgram.vectorTableCrc8Bytes);
    TEST_ASSERT_EQUAL(0u, boot_infoOfLastFlashedProgram.isProgramAvailable);

    TEST_ASSERT_EQUAL(0u, boot_backupVectorTable.vectorTable[0]);
    TEST_ASSERT_EQUAL(0u, boot_backupVectorTable.vectorTable[1]);
    TEST_ASSERT_EQUAL(0u, boot_backupVectorTable.vectorTable[2]);
    TEST_ASSERT_EQUAL(0u, boot_backupVectorTable.vectorTable[3]);

    /* ======= RT2/3: if the program is available, backup vector table does not
    contain zero,  updating the program info into flash is successful */
    _disable_IRQ_interrupt__Expect();

    /* load program info fro flash,  program is available,  backup vector table does not
    contain zero */
    _disable_IRQ_interrupt__Expect();
    memset((uint8_t *)BOOT_PROGRAM_INFO_ADDRESS_MAGIC_NUM, 0xAA, BOOT_PROGRAM_INFO_SIZE_MAGIC_NUM);
    memset((uint32_t *)BOOT_PROGRAM_INFO_ADDRESS_IS_PROGRAM_AVAILABLE, 0xCC, 4u);
    for (uint8_t iVector = 0u; iVector < BOOT_NUM_OF_VECTOR_TABLE_8_BYTES; iVector++) {
        boot_backupVectorTable.vectorTable[iVector]  = 0xDDu;
        boot_currentVectorTable.vectorTable[iVector] = 0xEEu;
    }
    /* updating the program info into flash is successful */
    _disable_IRQ_interrupt__Expect();
    DetectOperationMode_ExpectAndReturn(FSYS_M_BITS_SYSTEM_MODE);
    _disable_IRQ_interrupt__Expect();
    FLASH_WriteFlashSector_ExpectAndReturn(
        (uint32_t *)BOOT_PROGRAM_INFO_SECTOR_ADDRESS,
        (uint8_t *)BOOT_SECTOR_BUFFER_START_ADDRESS,
        BOOT_PROGRAM_INFO_SECTOR_SIZE,
        0u);

    retVal = BOOT_ResetBootInfo();
    TEST_ASSERT_EQUAL(STD_OK, retVal);
    for (uint8_t iVector = 0u; iVector < BOOT_NUM_OF_VECTOR_TABLE_8_BYTES; iVector++) {
        TEST_ASSERT_EQUAL(boot_currentVectorTable.vectorTable[iVector], 0xDDu);
    }

    /* ======= RT3/3: if the program is available,  backup vector table contains
    zero,  updating the program info into flash is not successful */
    _disable_IRQ_interrupt__Expect();

    /* load program info fro flash,  program is available,  backup vector table does not
    contain zero */
    _disable_IRQ_interrupt__Expect();
    memset((uint8_t *)BOOT_PROGRAM_INFO_ADDRESS_MAGIC_NUM, 0xAA, BOOT_PROGRAM_INFO_SIZE_MAGIC_NUM);
    memset((uint32_t *)BOOT_PROGRAM_INFO_ADDRESS_IS_PROGRAM_AVAILABLE, 0xCC, 4u);
    for (uint8_t iVector = 0u; iVector < BOOT_NUM_OF_VECTOR_TABLE_8_BYTES; iVector++) {
        boot_backupVectorTable.vectorTable[iVector]  = 0x0u;
        boot_currentVectorTable.vectorTable[iVector] = 0xEEu;
    }

    /* updating the program info into flash is successful */
    _disable_IRQ_interrupt__Expect();
    retVal = BOOT_ResetBootInfo();
    TEST_ASSERT_EQUAL(STD_NOT_OK, retVal);
    for (uint8_t iVector = 0u; iVector < BOOT_NUM_OF_VECTOR_TABLE_8_BYTES; iVector++) {
        TEST_ASSERT_EQUAL(boot_currentVectorTable.vectorTable[iVector], 0xEEu);
    }
}

void testBOOT_SoftwareResetMcu(void) {
    /* ======= RT1/2: raise privilege successfully */
    _disable_IRQ_interrupt__Expect();

    DetectOperationMode_ExpectAndReturn(FSYS_M_BITS_SYSTEM_MODE);

    BOOT_SoftwareResetMcu();

    /* ======= RT2/2: raise privilege not successfully */
    _disable_IRQ_interrupt__Expect();

    DetectOperationMode_ExpectAndReturn(FSYS_M_BITS_USER_MODE);
    FSYS_RaisePrivilegeToSystemModeSWI_Expect();
    DetectOperationMode_ExpectAndReturn(FSYS_M_BITS_USER_MODE);

    BOOT_SoftwareResetMcu();
    TEST_ASSERT_EQUAL(BOOT_RESET_VALUE_TO_WRITE << BOOT_RESET_SHIFT_BITS_NUMBER, BOOT_SYSTEM_REG1_SYSECR);
}
