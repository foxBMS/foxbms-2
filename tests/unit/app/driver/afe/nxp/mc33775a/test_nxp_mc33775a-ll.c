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
 * @file    test_nxp_mc33775a-ll.c
 * @author  foxBMS Team
 * @date    2021-10-20 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Test of nxp_mc33775a-ll.c
 * @details TODO
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockdma.h"
#include "Mockio.h"
#include "Mockmcu.h"
#include "Mockos.h"
#include "Mockspi.h"

#include "nxp_mc3377x-ll.h"
#include "spi_cfg-helper.h"
#include "uc_msg_t.h"

/* clang-format off */
#include "test_assert_helper.h"
/* clang-format on */

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("nxp_mc3377x-ll.c")

TEST_INCLUDE_PATH("../../src/app/driver/afe/api")
TEST_INCLUDE_PATH("../../src/app/driver/afe/nxp/common/mc3377x")
TEST_INCLUDE_PATH("../../src/app/driver/afe/nxp/common/mc3377x/vendor")
TEST_INCLUDE_PATH("../../src/app/driver/afe/nxp/mc33775a")
TEST_INCLUDE_PATH("../../src/app/driver/afe/nxp/mc33775a/config")
TEST_INCLUDE_PATH("../../src/app/driver/afe/nxp/mc33775a/vendor")
TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/dma")
TEST_INCLUDE_PATH("../../src/app/driver/io")
TEST_INCLUDE_PATH("../../src/app/driver/spi")

/*========== Definitions and Implementations for Unit Test ==================*/
static DATA_BLOCK_CELL_VOLTAGE_s n77x_cellVoltage           = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE_BASE};
static DATA_BLOCK_CELL_TEMPERATURE_s n77x_cellTemperature   = {.header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE_BASE};
static DATA_BLOCK_MIN_MAX_s n77x_minMax                     = {.header.uniqueId = DATA_BLOCK_ID_MIN_MAX};
static DATA_BLOCK_BALANCING_CONTROL_s n77x_balancingControl = {.header.uniqueId = DATA_BLOCK_ID_BALANCING_CONTROL};
static DATA_BLOCK_ALL_GPIO_VOLTAGES_s n77x_allGpioVoltage   = {.header.uniqueId = DATA_BLOCK_ID_ALL_GPIO_VOLTAGES_BASE};
static DATA_BLOCK_BALANCING_FEEDBACK_s n77x_balancingFeedback = {
    .header.uniqueId = DATA_BLOCK_ID_BALANCING_FEEDBACK_BASE};
static DATA_BLOCK_SLAVE_CONTROL_s n77x_slaveControl = {.header.uniqueId = DATA_BLOCK_ID_SLAVE_CONTROL};
static DATA_BLOCK_OPEN_WIRE_s n77x_openWire         = {.header.uniqueId = DATA_BLOCK_ID_OPEN_WIRE_BASE};
static N77X_SUPPLY_CURRENT_s n77x_supplyCurrent     = {0};
static N77X_ERROR_TABLE_s n77x_errorTable           = {0};

N77X_STATE_s n77x_stateBase = {
    .firstMeasurementMade       = false,
    .currentString              = 0u,
    .pSpiTxSequenceStart        = NULL_PTR,
    .pSpiTxSequence             = NULL_PTR,
    .pSpiRxSequenceStart        = NULL_PTR,
    .pSpiRxSequence             = NULL_PTR,
    .currentMux                 = {0u},
    .pMuxSequenceStart          = NULL_PTR,
    .pMuxSequence               = NULL_PTR,
    .n77xData.cellVoltage       = &n77x_cellVoltage,
    .n77xData.cellTemperature   = &n77x_cellTemperature,
    .n77xData.allGpioVoltage    = &n77x_allGpioVoltage,
    .n77xData.minMax            = &n77x_minMax,
    .n77xData.balancingFeedback = &n77x_balancingFeedback,
    .n77xData.balancingControl  = &n77x_balancingControl,
    .n77xData.slaveControl      = &n77x_slaveControl,
    .n77xData.openWire          = &n77x_openWire,
    .n77xData.supplyCurrent     = &n77x_supplyCurrent,
    .n77xData.errorTable        = &n77x_errorTable,
};

/** struct containing the lock state of the SPI interfaces */
SPI_BUSY_STATE_e spi_busyFlags[] = {
    SPI_IDLE,
    SPI_IDLE,
    SPI_IDLE,
    SPI_IDLE,
    SPI_IDLE,
};

/** SPI data configuration struct for NXP MC3377X communication, Tx part */
static spiDAT1_t spi_kNxp77xDataConfigTx[BS_NR_OF_STRINGS] = {
    {.CS_HOLD = TRUE,      /* If true, HW chip select kept active */
     .WDEL    = TRUE,      /* Activation of delay between words */
     .DFSEL   = SPI_FMT_0, /* Data word format selection */
     .CSNR    = SPI_HARDWARE_CHIP_SELECT_2_ACTIVE},
};

/** SPI data configuration struct for NXP MC3377X communication, Rx part */
static spiDAT1_t spi_kNxp77xDataConfigRx[BS_NR_OF_STRINGS] = {
    {.CS_HOLD = TRUE,      /* If true, HW chip select kept active */
     .WDEL    = TRUE,      /* Activation of delay between words */
     .DFSEL   = SPI_FMT_0, /* Data word format selection */
     .CSNR    = SPI_HARDWARE_CHIP_SELECT_0_ACTIVE},
};

/** SPI interface configuration for N77X communication Tx part */
SPI_INTERFACE_CONFIG_s spi_nxp77xInterfaceTx[BS_NR_OF_STRINGS] = {
    {
        .pConfig  = &spi_kNxp77xDataConfigTx[0u],
        .pNode    = spiREG1,
        .pGioPort = &(spiREG1->PC3),
        .csPin    = 2u,
        .csType   = SPI_CHIP_SELECT_HARDWARE,
    },
};

/** SPI interface configuration for N775X communication, Rx part */
SPI_INTERFACE_CONFIG_s spi_nxp77xInterfaceRx[BS_NR_OF_STRINGS] = {
    {
        .pConfig  = &spi_kNxp77xDataConfigRx[0u],
        .pNode    = spiREG4,
        .pGioPort = &(spiREG4->PC3),
        .csPin    = 0u,
        .csType   = SPI_CHIP_SELECT_HARDWARE,
    },
};

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/

void testN77x_CommunicationWrite(void) {
    uint16_t testDeviceAddress        = 0;
    uint16_t testRegisterAddress      = 0;
    uint16_t testValue                = 0;
    uint16_t testN77xToTplTxBuffer[4] = {0x8400u};
    uint16_t testN77xToTplRxBuffer[4] = {0u};
    uint32_t testNotifiedValueTx      = N77X_NO_NOTIFIED_VALUE;

    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(N77x_CommunicationWrite(testDeviceAddress, testRegisterAddress, testValue, NULL_PTR));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/1 ======= */
    OS_ClearNotificationIndexed_ExpectAndReturn(N77X_NOTIFICATION_TX_INDEX, OS_SUCCESS);
    SPI_TransmitReceiveDataDma_ExpectAndReturn(
        spi_nxp77xInterfaceTx, testN77xToTplTxBuffer, testN77xToTplRxBuffer, 4u, STD_OK);
    OS_WaitForNotificationIndexed_ExpectAndReturn(
        N77X_NOTIFICATION_TX_INDEX, &testNotifiedValueTx, N77X_NOTIFICATION_TX_TIMEOUT_ms, OS_SUCCESS);
    SPI_GetSpiIndex_ExpectAndReturn(spiREG1, 0);
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    TEST_ASSERT_PASS_ASSERT(
        N77x_CommunicationWrite(testDeviceAddress, testRegisterAddress, testValue, spi_nxp77xInterfaceTx));
}

void testN77x_CommunicationRead(void) {
    uint16_t testDeviceAddress   = 0;
    uint16_t testRegisterAddress = 0;
    uint16_t testValue           = 0;

    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(N77x_CommunicationRead(testDeviceAddress, testRegisterAddress, &testValue, NULL_PTR));
    /* ======= AT2/4 ======= */
    n77x_stateBase.pSpiTxSequence = NULL_PTR;
    n77x_stateBase.pSpiRxSequence = spi_nxp77xInterfaceRx;
    TEST_ASSERT_FAIL_ASSERT(
        N77x_CommunicationRead(testDeviceAddress, testRegisterAddress, &testValue, &n77x_stateBase));
    /* ======= AT3/4 ======= */
    n77x_stateBase.pSpiTxSequence = spi_nxp77xInterfaceTx;
    n77x_stateBase.pSpiRxSequence = NULL_PTR;
    TEST_ASSERT_FAIL_ASSERT(
        N77x_CommunicationRead(testDeviceAddress, testRegisterAddress, &testValue, &n77x_stateBase));
    /* ======= AT4/4 ======= */
    n77x_stateBase.pSpiRxSequence = spi_nxp77xInterfaceRx;
    TEST_ASSERT_FAIL_ASSERT(N77x_CommunicationRead(testDeviceAddress, testRegisterAddress, NULL_PTR, &n77x_stateBase));
}

void testN77x_CommunicationReadMultiple(void) {
    uint16_t testDeviceAddress   = 0;
    uint16_t testNumberOfItems   = 0;
    uint16_t testResponseLength  = 0;
    uint16_t testRegisterAddress = 0;
    uint16_t testValue           = 0;

    /* ======= Assertion tests ============================================= */
    /* ======= AT1/4 ======= */
    TEST_ASSERT_FAIL_ASSERT(N77x_CommunicationReadMultiple(
        testDeviceAddress, testNumberOfItems, testResponseLength, testRegisterAddress, &testValue, NULL_PTR));
    /* ======= AT2/4 ======= */
    n77x_stateBase.pSpiTxSequence = spi_nxp77xInterfaceTx;
    n77x_stateBase.pSpiRxSequence = NULL_PTR;
    TEST_ASSERT_FAIL_ASSERT(N77x_CommunicationReadMultiple(
        testDeviceAddress, testNumberOfItems, testResponseLength, testRegisterAddress, &testValue, &n77x_stateBase));
    /* ======= AT3/4 ======= */
    n77x_stateBase.pSpiTxSequence = NULL_PTR;
    n77x_stateBase.pSpiRxSequence = spi_nxp77xInterfaceRx;
    TEST_ASSERT_FAIL_ASSERT(N77x_CommunicationReadMultiple(
        testDeviceAddress, testNumberOfItems, testResponseLength, testRegisterAddress, &testValue, &n77x_stateBase));
    /* ======= AT4/4 ======= */
    n77x_stateBase.pSpiTxSequence = spi_nxp77xInterfaceTx;
    TEST_ASSERT_FAIL_ASSERT(N77x_CommunicationReadMultiple(
        testDeviceAddress, testNumberOfItems, testResponseLength, testRegisterAddress, NULL_PTR, &n77x_stateBase));
}

void testN77x_CommunicationComposeMessage(void) {
    uint16_t testCmd             = 0;
    uint16_t testMasterAddress   = 0;
    uint16_t testDeviceAddress   = 0;
    uint16_t testRegisterAddress = 0;
    uint16_t testLength          = 0;
    uint16_t testValue           = 0;
    uc_msg_t testMsg             = {0};

    /* ======= Assertion tests ============================================= */
    /* ======= AT1/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(N77x_CommunicationComposeMessage(
        testCmd, testMasterAddress, testDeviceAddress, testRegisterAddress, testLength, NULL_PTR, &testMsg));
    /* ======= AT2/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(N77x_CommunicationComposeMessage(
        testCmd, testMasterAddress, testDeviceAddress, testRegisterAddress, testLength, &testValue, NULL_PTR));
}

void testN77x_CommunicationDecomposeMssage(void) {
    uc_msg_t testMessage         = {0};
    uint16_t testCommand         = 0;
    uint16_t testMasterAddress   = 0;
    uint16_t testDeviceAddress   = 0;
    uint16_t testRegisterAddress = 0;
    uint16_t testLength          = 0;
    uint16_t testValue           = 0;
    uint8_t testString           = 0;

    /* ======= Assertion tests ============================================= */
    /* ======= AT1/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(N77x_CommunicationDecomposeMessage(
        NULL_PTR,
        &testCommand,
        &testMasterAddress,
        &testDeviceAddress,
        &testRegisterAddress,
        &testLength,
        &testValue,
        testString));
    /* ======= AT1/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(N77x_CommunicationDecomposeMessage(
        &testMessage,
        NULL_PTR,
        &testMasterAddress,
        &testDeviceAddress,
        &testRegisterAddress,
        &testLength,
        &testValue,
        testString));
    /* ======= AT1/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(N77x_CommunicationDecomposeMessage(
        &testMessage,
        &testCommand,
        NULL_PTR,
        &testDeviceAddress,
        &testRegisterAddress,
        &testLength,
        &testValue,
        testString));
    /* ======= AT1/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(N77x_CommunicationDecomposeMessage(
        &testMessage,
        &testCommand,
        &testMasterAddress,
        NULL_PTR,
        &testRegisterAddress,
        &testLength,
        &testValue,
        testString));
    /* ======= AT1/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(N77x_CommunicationDecomposeMessage(
        &testMessage,
        &testCommand,
        &testMasterAddress,
        &testDeviceAddress,
        NULL_PTR,
        &testLength,
        &testValue,
        testString));
    /* ======= AT1/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(N77x_CommunicationDecomposeMessage(
        &testMessage,
        &testCommand,
        &testMasterAddress,
        &testDeviceAddress,
        &testRegisterAddress,
        NULL_PTR,
        &testValue,
        testString));
    /* ======= AT1/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(N77x_CommunicationDecomposeMessage(
        &testMessage,
        &testCommand,
        &testMasterAddress,
        &testDeviceAddress,
        &testRegisterAddress,
        &testLength,
        NULL_PTR,
        testString));
    /* ======= AT1/2 ======= */
    TEST_ASSERT_PASS_ASSERT(N77x_CommunicationDecomposeMessage(
        &testMessage,
        &testCommand,
        &testMasterAddress,
        &testDeviceAddress,
        &testRegisterAddress,
        &testLength,
        &testValue,
        testString));
}
