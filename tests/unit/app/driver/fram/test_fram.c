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
 * @file    test_fram.c
 * @author  foxBMS Team
 * @date    2020-04-01 (date of creation)
 * @updated 2025-03-31 (date of last update)
 * @version v1.9.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Tests for the fram module
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "MockHL_spi.h"
#include "Mockcrc.h"
#include "Mockdiag.h"
#include "Mockdiag_cfg.h"
#include "Mockio.h"
#include "Mockmcu.h"
#include "Mockspi.h"

#include "fram_cfg.h"
#include "spi_cfg.h"

#include "fassert.h"
#include "fram.h"
#include "test_assert_helper.h"
#include "version.h"

#include <stdbool.h>

/*========== Unit Testing Framework Directives ==============================*/
TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/crc")
TEST_INCLUDE_PATH("../../src/app/driver/foxmath")
TEST_INCLUDE_PATH("../../src/app/driver/fram")
TEST_INCLUDE_PATH("../../src/app/driver/io")
TEST_INCLUDE_PATH("../../src/app/driver/spi")
TEST_INCLUDE_PATH("../../src/app/engine/diag")
TEST_INCLUDE_PATH("../../src/version")

/*========== Definitions and Implementations for Unit Test ==================*/
VER_VERSION_s ver_versionInformation VER_VERSION_INFORMATION = {
    .underVersionControl     = true,
    .isDirty                 = true,
    .major                   = 120,
    .minor                   = 121,
    .patch                   = 122,
    .distanceFromLastRelease = 22,
    .commitHash              = "abcdefgh",
    .gitRemote               = "git@remote-repo.de:foxbms-2.git",
};

#define FRAM_WRITE_COMMAND        (0x02u)
#define FRAM_READ_COMMAND         (0x03u)
#define FRAM_WRITE_ENABLE_COMMAND (0x06u)

/** delay in &micro;s after writing the FRAM */
#define FRAM_DELAY_AFTER_WRITE_ENABLE_US (5u)

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/

/**
 * @brief   Testing extern function #FRAM_Initialize
 * @details The following cases need to be tested:
 *          - Argument validation:
 *          - Routine validation:
 *            - RT1/1: TODO
 */
void testFRAM_Initialize(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= Routine tests =============================================== */
    uint16_t read = 0u;

    /* ======= RT1/1: Test implementation */
    DIAG_Handler_ExpectAndReturn(DIAG_ID_FRAM_READ_CRC_ERROR, DIAG_EVENT_OK, DIAG_SYSTEM, 0u, STD_OK);
    SPI_GetSpiIndex_ExpectAndReturn(spi_framInterface.pNode, SPI_SPI1_INDEX);
    SPI_Lock_ExpectAndReturn(0u, STD_OK);

    uint32_t address = (fram_databaseHeader[FRAM_BLOCK_ID_VERSION]).address;
    IO_PinReset_Expect(spi_framInterface.pGioPort, spi_framInterface.csPin);
    /* send write command */
    uint16_t writeCommand = 3u;
    SPI_FramTransmitReceiveData_Expect(&spi_framInterface, &writeCommand, &read, 1u);
    /* send upper part of address */
    uint16_t writeUpper = (address & 0x3F0000u) >> 16u;
    SPI_FramTransmitReceiveData_Expect(&spi_framInterface, &writeUpper, &read, 1u);
    /* send middle part of address */
    uint16_t writeMiddle = (address & 0x00FF00u) >> 8u;
    SPI_FramTransmitReceiveData_Expect(&spi_framInterface, &writeMiddle, &read, 1u);
    /* send lower part of address */
    uint16_t writeLower = address & 0x0000FFu;
    SPI_FramTransmitReceiveData_Expect(&spi_framInterface, &writeLower, &read, 1u);

    /* read CRC */
    uint16_t writeCrc = 0u;
    for (uint8_t i = 0u; i < FRAM_CRC_HEADER_SIZE; i++) {
        SPI_FramTransmitReceiveData_Expect(&spi_framInterface, &writeCrc, &read, 1u);
    }
    /* read data */
    uint16_t writeData = 0u;
    for (uint8_t i = 0u; i < (fram_databaseHeader[FRAM_BLOCK_ID_VERSION]).datalength; i++) {
        SPI_FramTransmitReceiveData_Expect(&spi_framInterface, &writeData, &read, 1u);
    }

    IO_PinSet_Expect(spi_framInterface.pGioPort, spi_framInterface.csPin);
    SPI_GetSpiIndex_ExpectAndReturn(spi_framInterface.pNode, SPI_SPI1_INDEX);
    SPI_Unlock_Expect(0u);

    uint8_t *pRead         = (uint8_t *)(fram_databaseHeader[FRAM_BLOCK_ID_VERSION].blockptr);
    uint32_t size          = (fram_databaseHeader[FRAM_BLOCK_ID_VERSION]).datalength;
    uint64_t crcCalculated = 0u;
    CRC_CalculateCrc_ExpectAndReturn(&crcCalculated, pRead, size, STD_OK);

    FRAM_Initialize();
}

/**
 * @brief   Testing extern function #FRAM_WriteData
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: invalid blockId &rarr; assert
 *          - Routine validation:
 *            - RT1/x: TODO
 */
void testFRAM_WriteData(void) {
    /* ======= Assertion tests ============================================= */
    const FRAM_BLOCK_ID_e invalidFramBlockId = (FRAM_BLOCK_ID_e)(FRAM_BLOCK_MAX + 1u);
    /* ======= AT1/1: Assertion test */
    TEST_ASSERT_FAIL_ASSERT(FRAM_WriteData(invalidFramBlockId));
    /* ======= AT2/2: Assertion test */
    spi_framInterface.csType = SPI_CHIP_SELECT_HARDWARE;
    TEST_ASSERT_FAIL_ASSERT(FRAM_ReadData(invalidFramBlockId));
/* reset for tests */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-value"
    spi_framInterface.csType == SPI_CHIP_SELECT_SOFTWARE;
#pragma GCC diagnostic pop

    /* ======= Routine tests =============================================== */
    uint16_t read = 0u;

    /* ======= RT1/2: Test implementation */
    uint64_t crc           = 0u;
    uint8_t pWrite         = 0u;
    uint32_t lengthInBytes = 6u;
    CRC_CalculateCrc_ExpectAndReturn(&crc, &pWrite, lengthInBytes, STD_NOT_OK);
    FRAM_RETURN_TYPE_e retFail = FRAM_WriteData(FRAM_BLOCK_ID_VERSION);
    TEST_ASSERT_EQUAL(FRAM_ACCESS_CRC_BUSY, retFail);

    /* ======= RT2/2: Test implementation */
    CRC_CalculateCrc_ExpectAndReturn(&crc, &pWrite, lengthInBytes, STD_OK);
    SPI_GetSpiIndex_ExpectAndReturn(spi_framInterface.pNode, SPI_SPI1_INDEX);
    SPI_Lock_ExpectAndReturn(0u, STD_OK);
    IO_PinReset_Expect(spi_framInterface.pGioPort, spi_framInterface.csPin);

    uint16_t writeEnable = FRAM_WRITE_ENABLE_COMMAND;
    SPI_FramTransmitReceiveData_Expect(&spi_framInterface, &writeEnable, &read, 1u);
    IO_PinSet_Expect(spi_framInterface.pGioPort, spi_framInterface.csPin);
    MCU_Delay_us_Expect(FRAM_DELAY_AFTER_WRITE_ENABLE_US);
    IO_PinReset_Expect(spi_framInterface.pGioPort, spi_framInterface.csPin);

    uint16_t writeCommand = FRAM_WRITE_COMMAND;
    SPI_FramTransmitReceiveData_Expect(&spi_framInterface, &writeCommand, &read, 1u);

    uint32_t address = (fram_databaseHeader[FRAM_BLOCK_ID_VERSION]).address;
    /* send upper part of address */
    uint16_t writeUpper = (address & 0x3F0000u) >> 16u;
    SPI_FramTransmitReceiveData_Expect(&spi_framInterface, &writeUpper, &read, 1u);
    /* send middle part of address */
    uint16_t writeMiddle = (address & 0x00FF00u) >> 8u;
    SPI_FramTransmitReceiveData_Expect(&spi_framInterface, &writeMiddle, &read, 1u);
    /* send lower part of address */
    uint16_t writeLower = address & 0x0000FFu;
    SPI_FramTransmitReceiveData_Expect(&spi_framInterface, &writeLower, &read, 1u);

    /* send CRC */
    uint16_t writeCrc = 0u;
    for (uint8_t i = 0u; i < FRAM_CRC_HEADER_SIZE; i++) {
        SPI_FramTransmitReceiveData_Expect(&spi_framInterface, &writeCrc, &read, 1u);
    }
    /* send data */
    uint16_t writeData = 0u;
    for (uint8_t i = 0u; i < (fram_databaseHeader[FRAM_BLOCK_ID_VERSION]).datalength; i++) {
        SPI_FramTransmitReceiveData_Expect(&spi_framInterface, &writeData, &read, 1u);
    }

    IO_PinSet_Expect(spi_framInterface.pGioPort, spi_framInterface.csPin);
    SPI_GetSpiIndex_ExpectAndReturn(spi_framInterface.pNode, SPI_SPI1_INDEX);
    SPI_Unlock_Expect(0u);

    FRAM_RETURN_TYPE_e retSuccess = FRAM_WriteData(FRAM_BLOCK_ID_VERSION);
    TEST_ASSERT_EQUAL(FRAM_ACCESS_OK, retSuccess);
}

/**
 * @brief   Testing extern function #FRAM_ReadData
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/2: invalid blockId &rarr; assert
 *            - AT2/2: invalid chip select type
 *          - Routine validation:
 *            - RT1/x: TODO
 */
void testFRAM_ReadData(void) {
    /* ======= Assertion tests ============================================= */
    const FRAM_BLOCK_ID_e invalidFramBlockId = (FRAM_BLOCK_ID_e)(FRAM_BLOCK_MAX + 1u);
    /* ======= AT1/2: Assertion test */
    TEST_ASSERT_FAIL_ASSERT(FRAM_ReadData(invalidFramBlockId));
    /* ======= AT2/2: Assertion test */
    spi_framInterface.csType = SPI_CHIP_SELECT_HARDWARE;
    TEST_ASSERT_FAIL_ASSERT(FRAM_ReadData(invalidFramBlockId));
/* reset for tests */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-value"
    spi_framInterface.csType == SPI_CHIP_SELECT_SOFTWARE;
#pragma GCC diagnostic pop

    /* ======= Routine tests =============================================== */
    uint16_t read = 0u;

    /* ======= RT1/x: Test implementation */
    SPI_GetSpiIndex_ExpectAndReturn(spi_framInterface.pNode, SPI_SPI1_INDEX);
    SPI_Lock_ExpectAndReturn(0u, STD_OK);

    uint32_t address = (fram_databaseHeader[FRAM_BLOCK_ID_VERSION]).address;
    IO_PinReset_Expect(spi_framInterface.pGioPort, spi_framInterface.csPin);
    /* send write command */
    uint16_t writeCommand = 3u;
    SPI_FramTransmitReceiveData_Expect(&spi_framInterface, &writeCommand, &read, 1u);
    /* send upper part of address */
    uint16_t writeUpper = (address & 0x3F0000u) >> 16u;
    SPI_FramTransmitReceiveData_Expect(&spi_framInterface, &writeUpper, &read, 1u);
    /* send middle part of address */
    uint16_t writeMiddle = (address & 0x00FF00u) >> 8u;
    SPI_FramTransmitReceiveData_Expect(&spi_framInterface, &writeMiddle, &read, 1u);
    /* send lower part of address */
    uint16_t writeLower = address & 0x0000FFu;
    SPI_FramTransmitReceiveData_Expect(&spi_framInterface, &writeLower, &read, 1u);

    uint16_t writeCrc = 0u;
    for (uint8_t i = 0u; i < FRAM_CRC_HEADER_SIZE; i++) {
        SPI_FramTransmitReceiveData_Expect(&spi_framInterface, &writeCrc, &read, 1u);
    }
    uint16_t writeData = 0u;
    for (uint8_t i = 0u; i < (fram_databaseHeader[FRAM_BLOCK_ID_VERSION]).datalength; i++) {
        SPI_FramTransmitReceiveData_Expect(&spi_framInterface, &writeData, &read, 1u);
    }

    IO_PinSet_Expect(spi_framInterface.pGioPort, spi_framInterface.csPin);
    SPI_GetSpiIndex_ExpectAndReturn(spi_framInterface.pNode, SPI_SPI1_INDEX);
    SPI_Unlock_Expect(0u);

    uint8_t *pRead         = (uint8_t *)(fram_databaseHeader[FRAM_BLOCK_ID_VERSION].blockptr);
    uint32_t size          = (fram_databaseHeader[FRAM_BLOCK_ID_VERSION]).datalength;
    uint64_t crcCalculated = 0u;
    CRC_CalculateCrc_ExpectAndReturn(&crcCalculated, pRead, size, STD_OK);

    FRAM_ReadData(FRAM_BLOCK_ID_VERSION);
}
