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
 * @file    test_nxp_mc33775a_i2c.c
 * @author  foxBMS Team
 * @date    2025-03-20 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Test of nxp_mc33775a_i2c.c
 * @details TODO
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockdatabase.h"
#include "Mockftask.h"
#include "Mocknxp_mc3377x-ll.h"
#include "Mocknxp_mc3377x_cfg.h"
#include "Mocknxp_mc3377x_helpers.h"
#include "Mockos.h"

#include "afe.h"
#include "nxp_mc3377x_i2c.h"
#include "nxp_mc3377x_reg_def.h"
#include "spi_cfg-helper.h"

/* clang-format off */
#include "test_assert_helper.h"
/* clang-format on */

#include <stdbool.h>
#include <stdint.h>

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("nxp_mc33775a_i2c.c")

TEST_INCLUDE_PATH("../../src/app/driver/afe/api")
TEST_INCLUDE_PATH("../../src/app/driver/afe/nxp/common/mc3377x")
TEST_INCLUDE_PATH("../../src/app/driver/afe/nxp/common/mc3377x/vendor")
TEST_INCLUDE_PATH("../../src/app/driver/afe/nxp/mc33775a")
TEST_INCLUDE_PATH("../../src/app/driver/afe/nxp/mc33775a/config")
TEST_INCLUDE_PATH("../../src/app/driver/afe/nxp/mc33775a/vendor")
TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/rtc")
TEST_INCLUDE_PATH("../../src/app/driver/spi")
TEST_INCLUDE_PATH("../../src/app/task/config")
TEST_INCLUDE_PATH("../../src/app/task/ftask")

/*========== Definitions and Implementations for Unit Test ==================*/

/** SPI data configuration struct for NXP MC3377X communication, Tx part */
static spiDAT1_t spi_kNxp77xDataConfigTx[BS_NR_OF_STRINGS] = {
    {.CS_HOLD = TRUE,      /* If true, HW chip select kept active */
     .WDEL    = TRUE,      /* Activation of delay between words */
     .DFSEL   = SPI_FMT_0, /* Data word format selection */
     .CSNR    = SPI_HARDWARE_CHIP_SELECT_2_ACTIVE},
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

volatile bool ftsk_allQueuesCreated = false;
OS_QUEUE ftsk_afeToI2cQueue;
OS_QUEUE ftsk_afeFromI2cQueue;

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/

void testN77x_InitializeI2c(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(N77x_InitializeI2c(NULL_PTR));

    /* ======= Routine tests =============================================== */
    N77X_STATE_s n77xTestState = {
        .firstMeasurementMade = false,
        .pSpiTxSequence       = spi_nxp77xInterfaceTx,
    };

    /* ======= RT1/1 ======= */
    N77x_CommunicationWrite_Expect(
        N77X_BROADCAST_ADDRESS,
        MC3377X_I2C_CFG_OFFSET,
        (MC3377X_I2C_CFG_EN_ENABLED_ENUM_VAL << MC3377X_I2C_CFG_EN_POS) +
            (MC3377X_I2C_CFG_CLKSEL_F_400K_ENUM_VAL << MC3377X_I2C_CFG_CLKSEL_POS),
        n77xTestState.pSpiTxSequence);
    N77x_InitializeI2c(&n77xTestState);
}

void testN77x_TransmitI2c(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(N77x_TransmitI2c(NULL_PTR));

    /* ======= Routine tests =============================================== */
    AFE_I2C_QUEUE_s transactionData = {0};
    N77X_STATE_s n77xTestState      = {
             .firstMeasurementMade = false,
             .pSpiTxSequence       = spi_nxp77xInterfaceTx,
    };

    /* ======= RT1/1 ======= */
    /* ftsk_Queues not created yet */
    TEST_ASSERT_EQUAL(STD_OK, N77x_TransmitI2c(&n77xTestState));

    /* ======= RT2/2 ======= */
    /* No new I2C transmissions in queue */
    ftsk_allQueuesCreated = true;
    OS_ReceiveFromQueue_ExpectAndReturn(ftsk_afeToI2cQueue, (void *)&transactionData, 0u, OS_FAIL);
    TEST_ASSERT_EQUAL(STD_OK, N77x_TransmitI2c(&n77xTestState));
}

void testN77x_TransmitI2c_WRITE(void) {
    /* ======= Routine tests =============================================== */
    const uint8_t validModuleNumber  = 0u;
    const uint8_t validDeviceAddress = 0u;
    const uint8_t validDataLength    = 1u;
    uint8_t writeData[3]             = {0u, 0u, 0u};
    uint16_t readData                = 0u;

    AFE_I2C_QUEUE_s transactionData          = {0};
    AFE_I2C_QUEUE_s transactionData_returned = {
        .module          = validModuleNumber,
        .deviceAddress   = validDeviceAddress,
        .writeDataLength = validDataLength,
        .transferType    = AFE_I2C_TRANSFER_TYPE_WRITE,
    };
    N77X_STATE_s n77xTestState = {
        .firstMeasurementMade = false,
        .pSpiTxSequence       = spi_nxp77xInterfaceTx,
    };

    /* ======= RT1/1 ======= */
    /* Everything ok */
    OS_ReceiveFromQueue_ExpectAndReturn(ftsk_afeToI2cQueue, (void *)&transactionData, 0u, OS_SUCCESS);
    OS_ReceiveFromQueue_ReturnThruPtr_pvBuffer(&transactionData_returned);

    /* From N77x_I2cTransmitWrite */
    N77x_CommunicationWrite_Expect(
        validModuleNumber + 1,
        MC3377X_I2C_DATA0_OFFSET,
        (validDeviceAddress << MC3377X_I2C_DATA0_BYTE0_POS) | (writeData[0] << MC3377X_I2C_DATA0_BYTE1_POS),
        n77xTestState.pSpiTxSequence);
    N77x_CommunicationWrite_Expect(
        validModuleNumber + 1u,
        MC3377X_I2C_CTRL_OFFSET,
        /* transactionData.writeDataLength + 1u: data + I2C device address byte */
        ((validDataLength + 1u) << MC3377X_I2C_CTRL_START_POS) |
            ((MC3377X_I2C_CTRL_STPAFTER_STOP_ENUM_VAL << MC3377X_I2C_CTRL_STPAFTER_POS) +
             (0u << MC3377X_I2C_CTRL_RDAFTER_POS)),
        n77xTestState.pSpiTxSequence);
    N77x_CommunicationRead_ExpectAndReturn(
        validModuleNumber + 1u, MC3377X_I2C_STAT_OFFSET, &readData, &n77xTestState, N77X_COMMUNICATION_OK);
    N77x_Wait_Expect(2u);

    OS_SendToBackOfQueue_ExpectAndReturn(ftsk_afeFromI2cQueue, (void *)&transactionData, 0u, OS_SUCCESS);
    TEST_ASSERT_EQUAL(STD_OK, N77x_TransmitI2c(&n77xTestState));

    /* ======= RT2/2 ======= */
    /* Response queue full */
    OS_ReceiveFromQueue_ExpectAndReturn(ftsk_afeToI2cQueue, (void *)&transactionData, 0u, OS_SUCCESS);
    OS_ReceiveFromQueue_ReturnThruPtr_pvBuffer(&transactionData_returned);

    /* From N77x_I2cTransmitWrite */
    N77x_CommunicationWrite_Expect(
        validModuleNumber + 1,
        MC3377X_I2C_DATA0_OFFSET,
        (validDeviceAddress << MC3377X_I2C_DATA0_BYTE0_POS) | (writeData[0] << MC3377X_I2C_DATA0_BYTE1_POS),
        n77xTestState.pSpiTxSequence);
    N77x_CommunicationWrite_Expect(
        validModuleNumber + 1u,
        MC3377X_I2C_CTRL_OFFSET,
        /* transactionData.writeDataLength + 1u: data + I2C device address byte */
        ((validDataLength + 1u) << MC3377X_I2C_CTRL_START_POS) |
            ((MC3377X_I2C_CTRL_STPAFTER_STOP_ENUM_VAL << MC3377X_I2C_CTRL_STPAFTER_POS) +
             (0u << MC3377X_I2C_CTRL_RDAFTER_POS)),
        n77xTestState.pSpiTxSequence);
    N77x_CommunicationRead_ExpectAndReturn(
        validModuleNumber + 1u, MC3377X_I2C_STAT_OFFSET, &readData, &n77xTestState, N77X_COMMUNICATION_OK);
    N77x_Wait_Expect(2u);

    OS_SendToBackOfQueue_ExpectAndReturn(ftsk_afeFromI2cQueue, (void *)&transactionData, 0u, OS_FAIL);
    TEST_ASSERT_EQUAL(STD_NOT_OK, N77x_TransmitI2c(&n77xTestState));
}

void testN77x_TransmitI2c_READ(void) {
    /* ======= Routine tests =============================================== */
    const uint8_t validModuleNumber  = 0u;
    const uint8_t validDeviceAddress = 0u;
    const uint8_t validDataLength    = 1u;
    uint16_t readData                = 0u;

    AFE_I2C_QUEUE_s transactionData          = {0};
    AFE_I2C_QUEUE_s transactionData_returned = {
        .module          = validModuleNumber,
        .deviceAddress   = validDeviceAddress,
        .writeDataLength = validDataLength,
        .transferType    = AFE_I2C_TRANSFER_TYPE_READ,
    };
    N77X_STATE_s n77xTestState = {
        .firstMeasurementMade = false,
        .pSpiTxSequence       = spi_nxp77xInterfaceTx,
    };

    /* ======= RT1/1 ======= */
    /* Everything ok */
    OS_ReceiveFromQueue_ExpectAndReturn(ftsk_afeToI2cQueue, (void *)&transactionData, 0u, OS_SUCCESS);
    OS_ReceiveFromQueue_ReturnThruPtr_pvBuffer(&transactionData_returned);

    /* From N77x_I2cTransmitRead */
    N77x_CommunicationWrite_Expect(
        validModuleNumber + 1,
        MC3377X_I2C_DATA0_OFFSET,
        validDeviceAddress | N77X_I2C_READ << MC3377X_I2C_DATA0_BYTE0_POS,
        n77xTestState.pSpiTxSequence);
    N77x_CommunicationWrite_Expect(
        validModuleNumber + 1u,
        MC3377X_I2C_CTRL_OFFSET,
        /* transactionData.writeDataLength + 1u: data + I2C device address byte */
        ((validDataLength) << MC3377X_I2C_CTRL_START_POS) |
            ((1u << MC3377X_I2C_CTRL_STPAFTER_POS) + (0u << MC3377X_I2C_CTRL_RDAFTER_POS)),
        n77xTestState.pSpiTxSequence);
    N77x_CommunicationRead_ExpectAndReturn(
        validModuleNumber + 1u, MC3377X_I2C_STAT_OFFSET, &readData, &n77xTestState, N77X_COMMUNICATION_OK);
    N77x_Wait_Expect(2u);
    N77x_CommunicationReadMultiple_ExpectAndReturn(
        validModuleNumber + 1u,
        (validDataLength / 2u) + 1u,
        4u,
        MC3377X_I2C_DATA0_OFFSET,
        &readData,
        &n77xTestState,
        N77X_COMMUNICATION_OK);

    OS_SendToBackOfQueue_ExpectAndReturn(ftsk_afeFromI2cQueue, (void *)&transactionData, 0u, OS_SUCCESS);
    TEST_ASSERT_EQUAL(STD_OK, N77x_TransmitI2c(&n77xTestState));

    /* ======= RT2/2 ======= */
    /* Response queue full */
    OS_ReceiveFromQueue_ExpectAndReturn(ftsk_afeToI2cQueue, (void *)&transactionData, 0u, OS_SUCCESS);
    OS_ReceiveFromQueue_ReturnThruPtr_pvBuffer(&transactionData_returned);

    /* From N77x_I2cTransmitRead */
    N77x_CommunicationWrite_Expect(
        validModuleNumber + 1,
        MC3377X_I2C_DATA0_OFFSET,
        validDeviceAddress | N77X_I2C_READ << MC3377X_I2C_DATA0_BYTE0_POS,
        n77xTestState.pSpiTxSequence);
    N77x_CommunicationWrite_Expect(
        validModuleNumber + 1u,
        MC3377X_I2C_CTRL_OFFSET,
        /* transactionData.writeDataLength + 1u: data + I2C device address byte */
        ((validDataLength) << MC3377X_I2C_CTRL_START_POS) |
            ((1u << MC3377X_I2C_CTRL_STPAFTER_POS) + (0u << MC3377X_I2C_CTRL_RDAFTER_POS)),
        n77xTestState.pSpiTxSequence);
    N77x_CommunicationRead_ExpectAndReturn(
        validModuleNumber + 1u, MC3377X_I2C_STAT_OFFSET, &readData, &n77xTestState, N77X_COMMUNICATION_OK);
    N77x_Wait_Expect(2u);
    N77x_CommunicationReadMultiple_ExpectAndReturn(
        validModuleNumber + 1u,
        (validDataLength / 2u) + 1u,
        4u,
        MC3377X_I2C_DATA0_OFFSET,
        &readData,
        &n77xTestState,
        N77X_COMMUNICATION_OK);

    OS_SendToBackOfQueue_ExpectAndReturn(ftsk_afeFromI2cQueue, (void *)&transactionData, 0u, OS_FAIL);
    TEST_ASSERT_EQUAL(STD_NOT_OK, N77x_TransmitI2c(&n77xTestState));
}

void testN77x_TransmitI2c_WRITEREAD(void) {
    /* ======= Routine tests =============================================== */
    const uint8_t validModuleNumber  = 0u;
    const uint8_t validDeviceAddress = 0u;
    const uint8_t validDataLength    = 1u;
    uint16_t readData                = 0u;
    uint8_t writeData[3]             = {0u, 0u, 0u};

    AFE_I2C_QUEUE_s transactionData          = {0};
    AFE_I2C_QUEUE_s transactionData_returned = {
        .module          = validModuleNumber,
        .deviceAddress   = validDeviceAddress,
        .writeDataLength = validDataLength,
        .transferType    = AFE_I2C_TRANSFER_TYPE_WRITEREAD,
    };
    N77X_STATE_s n77xTestState = {
        .firstMeasurementMade = false,
        .pSpiTxSequence       = spi_nxp77xInterfaceTx,
    };

    /* ======= RT1/1 ======= */
    /* Everything ok */
    OS_ReceiveFromQueue_ExpectAndReturn(ftsk_afeToI2cQueue, (void *)&transactionData, 0u, OS_SUCCESS);
    OS_ReceiveFromQueue_ReturnThruPtr_pvBuffer(&transactionData_returned);

    N77x_CommunicationWrite_Expect(
        validModuleNumber + 1,
        MC3377X_I2C_DATA0_OFFSET,
        ((validDeviceAddress | N77X_I2C_WRITE) << MC3377X_I2C_DATA0_BYTE0_POS) |
            (writeData[0] << MC3377X_I2C_DATA0_BYTE1_POS),
        n77xTestState.pSpiTxSequence);
    N77x_CommunicationWrite_Expect(
        validModuleNumber + 1u,
        MC3377X_I2C_DATA1_OFFSET,
        ((validDeviceAddress | N77X_I2C_READ) << MC3377X_I2C_DATA1_BYTE2_POS),
        n77xTestState.pSpiTxSequence);

    N77x_CommunicationWrite_Expect(
        validModuleNumber + 1u,
        MC3377X_I2C_CTRL_OFFSET,
        /* transaction length: I2C device address byte for write + data to write
                                   + I2C device address byte for read + data to read */
        ((validDataLength + 2u) << MC3377X_I2C_CTRL_START_POS) |
            ((1u << MC3377X_I2C_CTRL_STPAFTER_POS) + ((1u + validDataLength) << MC3377X_I2C_CTRL_RDAFTER_POS)),
        n77xTestState.pSpiTxSequence);
    N77x_CommunicationRead_ExpectAndReturn(
        validModuleNumber + 1u, MC3377X_I2C_STAT_OFFSET, &readData, &n77xTestState, N77X_COMMUNICATION_OK);
    N77x_Wait_Expect(2u);

    N77x_CommunicationReadMultiple_ExpectAndReturn(
        validModuleNumber + 1u,
        (validDataLength / 2u) + 1u,
        4u,
        MC3377X_I2C_DATA0_OFFSET + 1,
        &readData,
        &n77xTestState,
        N77X_COMMUNICATION_OK);

    OS_SendToBackOfQueue_ExpectAndReturn(ftsk_afeFromI2cQueue, (void *)&transactionData, 0u, OS_SUCCESS);
    TEST_ASSERT_EQUAL(STD_OK, N77x_TransmitI2c(&n77xTestState));
}

/**
 * @brief   Testing extern function N77x_I2cRead
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/2: NULL_PTR for pData -> &rarr; assert
 *            - AT2/2: invalid data length -> &rarr; assert
 *          - Routine validation:
 *            - TODO
 */
void testN77x_I2cRead(void) {
    /* ======= Assertion tests ============================================= */
    uint8_t data                            = 0u;
    const uint8_t validModuleNumber         = 0u;
    const uint8_t validDeviceAddress        = 0u;
    const uint8_t validDataLength           = 1u;
    const uint8_t invalidDataLengthTooSmall = 0u;
    const uint8_t invalidDataLengthTooLarge = 14u;
    /* ======= AT1/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(N77x_I2cRead(validModuleNumber, validDeviceAddress, NULL_PTR, validDataLength));
    /* ======= AT1/2:1 ===== */
    TEST_ASSERT_FAIL_ASSERT(N77x_I2cRead(validModuleNumber, validDeviceAddress, &data, invalidDataLengthTooSmall));
    /* ======= AT1/2:2 ===== */
    TEST_ASSERT_FAIL_ASSERT(N77x_I2cRead(validModuleNumber, validDeviceAddress, &data, invalidDataLengthTooLarge));
    /* ======= Routine tests =============================================== */

    AFE_I2C_QUEUE_s transactionData = {
        .module          = validModuleNumber,
        .deviceAddress   = validDeviceAddress,
        .writeDataLength = validDataLength,
        .transferType    = AFE_I2C_TRANSFER_TYPE_WRITE,
    };
    AFE_I2C_QUEUE_s transactionData_returned = {
        .module          = validModuleNumber,
        .deviceAddress   = validDeviceAddress,
        .writeDataLength = validDataLength,
        .transferType    = AFE_I2C_TRANSFER_TYPE_READ_SUCCESS,
    };

    /* ======= RT1/1 ======= */
    /* Everything OK */
    OS_SendToBackOfQueue_ExpectAndReturn(ftsk_afeToI2cQueue, (void *)&transactionData, 0u, OS_SUCCESS);
    OS_ReceiveFromQueue_ExpectAndReturn(
        ftsk_afeFromI2cQueue, (void *)&transactionData_returned, N77X_I2C_FINISHED_TIMEOUT_ms, OS_SUCCESS);
    OS_ReceiveFromQueue_ReturnThruPtr_pvBuffer(&transactionData_returned);
    TEST_ASSERT_EQUAL(STD_OK, N77x_I2cRead(validModuleNumber, validDeviceAddress, &data, validDataLength));

    /* ======= RT2/4 ======= */
    /* ftsk_afeToI2cQueue full */
    OS_SendToBackOfQueue_ExpectAndReturn(ftsk_afeToI2cQueue, (void *)&transactionData, 0u, OS_FAIL);
    TEST_ASSERT_EQUAL(STD_NOT_OK, N77x_I2cRead(validModuleNumber, validDeviceAddress, &data, validDataLength));

    /* ======= RT3/4 ======= */
    /* ftsk_afeFromI2cQueue full */
    OS_SendToBackOfQueue_ExpectAndReturn(ftsk_afeToI2cQueue, (void *)&transactionData, 0u, OS_SUCCESS);
    OS_ReceiveFromQueue_ExpectAndReturn(
        ftsk_afeFromI2cQueue, (void *)&transactionData_returned, N77X_I2C_FINISHED_TIMEOUT_ms, OS_FAIL);
    TEST_ASSERT_EQUAL(STD_NOT_OK, N77x_I2cRead(validModuleNumber, validDeviceAddress, &data, validDataLength));

    /* ======= RT4/4 ======= */
    /* Transfer fail */
    transactionData_returned.transferType = AFE_I2C_TRANSFER_TYPE_READ_FAIL;
    OS_SendToBackOfQueue_ExpectAndReturn(ftsk_afeToI2cQueue, (void *)&transactionData, 0u, OS_SUCCESS);
    OS_ReceiveFromQueue_ExpectAndReturn(
        ftsk_afeFromI2cQueue, (void *)&transactionData_returned, N77X_I2C_FINISHED_TIMEOUT_ms, OS_SUCCESS);
    OS_ReceiveFromQueue_ReturnThruPtr_pvBuffer(&transactionData_returned);
    TEST_ASSERT_EQUAL(STD_NOT_OK, N77x_I2cRead(validModuleNumber, validDeviceAddress, &data, validDataLength));
}

void testN77x_I2cWrite(void) {
    uint8_t data                            = 0u;
    const uint8_t validModuleNumber         = 0u;
    const uint8_t validDeviceAddress        = 0u;
    const uint8_t validDataLength           = 1u;
    const uint8_t invalidDataLengthTooSmall = 0u;
    const uint8_t invalidDataLengthTooLarge = 14u;
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(N77x_I2cWrite(validModuleNumber, validDeviceAddress, NULL_PTR, validDataLength));
    /* ======= AT1/2:1 ===== */
    TEST_ASSERT_FAIL_ASSERT(N77x_I2cWrite(validModuleNumber, validDeviceAddress, &data, invalidDataLengthTooSmall));
    /* ======= AT1/2:2 ===== */
    TEST_ASSERT_FAIL_ASSERT(N77x_I2cWrite(validModuleNumber, validDeviceAddress, &data, invalidDataLengthTooLarge));

    /* ======= Routine tests =============================================== */

    AFE_I2C_QUEUE_s transactionData = {
        .module          = validModuleNumber,
        .deviceAddress   = validDeviceAddress,
        .writeDataLength = validDataLength,
        .transferType    = AFE_I2C_TRANSFER_TYPE_WRITE,
    };
    AFE_I2C_QUEUE_s transactionData_returned = {
        .module          = validModuleNumber,
        .deviceAddress   = validDeviceAddress,
        .writeDataLength = validDataLength,
        .transferType    = AFE_I2C_TRANSFER_TYPE_WRITE_SUCCESS,
    };

    /* ======= RT1/1 ======= */
    /* Everything OK */
    OS_SendToBackOfQueue_ExpectAndReturn(ftsk_afeToI2cQueue, (void *)&transactionData, 0u, OS_SUCCESS);
    OS_ReceiveFromQueue_ExpectAndReturn(
        ftsk_afeFromI2cQueue, (void *)&transactionData_returned, N77X_I2C_FINISHED_TIMEOUT_ms, OS_SUCCESS);
    OS_ReceiveFromQueue_ReturnThruPtr_pvBuffer(&transactionData_returned);
    TEST_ASSERT_EQUAL(STD_OK, N77x_I2cWrite(validModuleNumber, validDeviceAddress, &data, validDataLength));

    /* ======= RT2/4 ======= */
    /* ftsk_afeToI2cQueue full */
    OS_SendToBackOfQueue_ExpectAndReturn(ftsk_afeToI2cQueue, (void *)&transactionData, 0u, OS_FAIL);
    TEST_ASSERT_EQUAL(STD_NOT_OK, N77x_I2cWrite(validModuleNumber, validDeviceAddress, &data, validDataLength));

    /* ======= RT3/4 ======= */
    /* ftsk_afeFromI2cQueue full */
    OS_SendToBackOfQueue_ExpectAndReturn(ftsk_afeToI2cQueue, (void *)&transactionData, 0u, OS_SUCCESS);
    OS_ReceiveFromQueue_ExpectAndReturn(
        ftsk_afeFromI2cQueue, (void *)&transactionData_returned, N77X_I2C_FINISHED_TIMEOUT_ms, OS_FAIL);
    TEST_ASSERT_EQUAL(STD_NOT_OK, N77x_I2cWrite(validModuleNumber, validDeviceAddress, &data, validDataLength));

    /* ======= RT4/4 ======= */
    /* Transfer fail */
    transactionData_returned.transferType = AFE_I2C_TRANSFER_TYPE_WRITE_FAIL;
    OS_SendToBackOfQueue_ExpectAndReturn(ftsk_afeToI2cQueue, (void *)&transactionData, 0u, OS_SUCCESS);
    OS_ReceiveFromQueue_ExpectAndReturn(
        ftsk_afeFromI2cQueue, (void *)&transactionData_returned, N77X_I2C_FINISHED_TIMEOUT_ms, OS_SUCCESS);
    OS_ReceiveFromQueue_ReturnThruPtr_pvBuffer(&transactionData_returned);
    TEST_ASSERT_EQUAL(STD_NOT_OK, N77x_I2cWrite(validModuleNumber, validDeviceAddress, &data, validDataLength));
}

/**
 * @brief   Testing extern function N77x_I2cRead
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/5: NULL_PTR for pDataWrite -> &rarr; assert
 *            - AT2/5: NULL_PTR for pDataRead -> &rarr; assert
 *            - AT3/5: invalid write data length -> &rarr; assert
 *            - AT4/5: invalid read data length -> &rarr; assert
 *            - AT5/5: invalid data length combination -> &rarr; assert
 *          - Routine validation:
 *            - TODO
 */
void testN77x_I2cWriteRead(void) {
    /* ======= Assertion tests ============================================= */
    uint8_t writeData = 0u;
    uint8_t readData  = 0u;

    const uint8_t validModuleNumber         = 0u;
    const uint8_t validDeviceAddress        = 0u;
    const uint8_t validDataLength           = 1u;
    const uint8_t invalidDataLengthTooSmall = 0u;
    const uint8_t invalidDataLengthTooLarge = 14u;

    /* ======= AT1/6 ======= */
    TEST_ASSERT_FAIL_ASSERT(N77x_I2cWriteRead(validModuleNumber, validDeviceAddress, NULL_PTR, 1u, &readData, 1u));
    /* ======= AT2/6 ======= */
    TEST_ASSERT_FAIL_ASSERT(N77x_I2cWriteRead(validModuleNumber, validDeviceAddress, &writeData, 1u, NULL_PTR, 1u));
    /* ======= AT3/6 ======= */
    TEST_ASSERT_FAIL_ASSERT(N77x_I2cWriteRead(
        validModuleNumber, validDeviceAddress, &writeData, invalidDataLengthTooSmall, &readData, validDataLength));
    /* ======= AT4/6 ======= */
    TEST_ASSERT_FAIL_ASSERT(N77x_I2cWriteRead(
        validModuleNumber, validDeviceAddress, &writeData, validDataLength, &readData, invalidDataLengthTooSmall));
    /* ======= AT5/6 ======= */
    TEST_ASSERT_FAIL_ASSERT(N77x_I2cWriteRead(
        validModuleNumber, validDeviceAddress, &writeData, invalidDataLengthTooLarge, &readData, validDataLength));
    /* ======= AT6/6 ======= */
    TEST_ASSERT_FAIL_ASSERT(N77x_I2cWriteRead(
        validModuleNumber, validDeviceAddress, &writeData, validDataLength, &readData, invalidDataLengthTooLarge));

    /* ======= Routine tests =============================================== */
    AFE_I2C_QUEUE_s transactionData = {
        .module          = validModuleNumber,
        .deviceAddress   = validDeviceAddress,
        .writeDataLength = validDataLength,
        .transferType    = AFE_I2C_TRANSFER_TYPE_WRITE,
    };
    AFE_I2C_QUEUE_s transactionData_returned = {
        .module          = validModuleNumber,
        .deviceAddress   = validDeviceAddress,
        .writeDataLength = validDataLength,
        .transferType    = AFE_I2C_TRANSFER_TYPE_READ_SUCCESS,
    };

    /* ======= RT1/1 ======= */
    /* Everything OK */
    OS_SendToBackOfQueue_ExpectAndReturn(ftsk_afeToI2cQueue, (void *)&transactionData, 0u, OS_SUCCESS);
    OS_ReceiveFromQueue_ExpectAndReturn(
        ftsk_afeFromI2cQueue, (void *)&transactionData_returned, N77X_I2C_FINISHED_TIMEOUT_ms, OS_SUCCESS);
    OS_ReceiveFromQueue_ReturnThruPtr_pvBuffer(&transactionData_returned);
    TEST_ASSERT_EQUAL(
        STD_OK,
        N77x_I2cWriteRead(
            validModuleNumber, validDeviceAddress, &writeData, validDataLength, &readData, validDataLength));

    /* ======= RT2/4 ======= */
    /* ftsk_afeToI2cQueue full */
    OS_SendToBackOfQueue_ExpectAndReturn(ftsk_afeToI2cQueue, (void *)&transactionData, 0u, OS_FAIL);
    TEST_ASSERT_EQUAL(
        STD_NOT_OK,
        N77x_I2cWriteRead(
            validModuleNumber, validDeviceAddress, &writeData, validDataLength, &readData, validDataLength));

    /* ======= RT3/4 ======= */
    /* ftsk_afeFromI2cQueue full */
    OS_SendToBackOfQueue_ExpectAndReturn(ftsk_afeToI2cQueue, (void *)&transactionData, 0u, OS_SUCCESS);
    OS_ReceiveFromQueue_ExpectAndReturn(
        ftsk_afeFromI2cQueue, (void *)&transactionData_returned, N77X_I2C_FINISHED_TIMEOUT_ms, OS_FAIL);
    TEST_ASSERT_EQUAL(
        STD_NOT_OK,
        N77x_I2cWriteRead(
            validModuleNumber, validDeviceAddress, &writeData, validDataLength, &readData, validDataLength));

    /* ======= RT4/4 ======= */
    /* Transfer fail */
    transactionData_returned.transferType = AFE_I2C_TRANSFER_TYPE_WRITE_FAIL;
    OS_SendToBackOfQueue_ExpectAndReturn(ftsk_afeToI2cQueue, (void *)&transactionData, 0u, OS_SUCCESS);
    OS_ReceiveFromQueue_ExpectAndReturn(
        ftsk_afeFromI2cQueue, (void *)&transactionData_returned, N77X_I2C_FINISHED_TIMEOUT_ms, OS_SUCCESS);
    OS_ReceiveFromQueue_ReturnThruPtr_pvBuffer(&transactionData_returned);
    TEST_ASSERT_EQUAL(
        STD_NOT_OK,
        N77x_I2cWriteRead(
            validModuleNumber, validDeviceAddress, &writeData, validDataLength, &readData, validDataLength));
}

void testN77x_I2cTransmitWrite(void) {
    /* ======= Assertion tests ============================================= */
    uint8_t writeData[3]       = {0u, 0u, 0u};
    uint16_t readData          = 0u;
    N77X_STATE_s n77xTestState = {
        .firstMeasurementMade = false,
        .pSpiTxSequence       = spi_nxp77xInterfaceTx,
    };

    const uint8_t validModuleNumber  = 0u;
    const uint8_t validDeviceAddress = 0u;
    uint8_t validDataLength          = 1u;

    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(
        TEST_N77x_I2cTransmitWrite(validModuleNumber, validDeviceAddress, writeData, validDataLength, NULL_PTR));

    /* ======= Routine tests =============================================== */

    /* ======= RT1/3 ======= */
    /* 1 Byte Data, everything ok */
    N77x_CommunicationWrite_Expect(
        validModuleNumber + 1,
        MC3377X_I2C_DATA0_OFFSET,
        (validDeviceAddress << MC3377X_I2C_DATA0_BYTE0_POS) | (writeData[0] << MC3377X_I2C_DATA0_BYTE1_POS),
        n77xTestState.pSpiTxSequence);
    N77x_CommunicationWrite_Expect(
        validModuleNumber + 1u,
        MC3377X_I2C_CTRL_OFFSET,
        /* transactionData.writeDataLength + 1u: data + I2C device address byte */
        ((validDataLength + 1u) << MC3377X_I2C_CTRL_START_POS) |
            ((MC3377X_I2C_CTRL_STPAFTER_STOP_ENUM_VAL << MC3377X_I2C_CTRL_STPAFTER_POS) +
             (0u << MC3377X_I2C_CTRL_RDAFTER_POS)),
        n77xTestState.pSpiTxSequence);
    N77x_CommunicationRead_ExpectAndReturn(
        validModuleNumber + 1u, MC3377X_I2C_STAT_OFFSET, &readData, &n77xTestState, N77X_COMMUNICATION_OK);
    N77x_Wait_Expect(2u);
    TEST_ASSERT_EQUAL(
        STD_OK,
        TEST_N77x_I2cTransmitWrite(validModuleNumber, validDeviceAddress, writeData, validDataLength, &n77xTestState));

    /* ======= RT2/3 ======= */
    /* >1 Byte Data, everything ok */
    validDataLength = 3u;
    N77x_CommunicationWrite_Expect(
        validModuleNumber + 1,
        MC3377X_I2C_DATA0_OFFSET,
        (validDeviceAddress << MC3377X_I2C_DATA0_BYTE0_POS) | (writeData[0] << MC3377X_I2C_DATA0_BYTE1_POS),
        n77xTestState.pSpiTxSequence);

    N77x_CommunicationWrite_Expect(
        validModuleNumber + 1u, MC3377X_I2C_DATA1_OFFSET, writeData[1], n77xTestState.pSpiTxSequence);

    N77x_CommunicationWrite_Expect(
        validModuleNumber + 1u,
        MC3377X_I2C_CTRL_OFFSET,
        /* transactionData.writeDataLength + 1u: data + I2C device address byte */
        ((validDataLength + 1u) << MC3377X_I2C_CTRL_START_POS) |
            ((MC3377X_I2C_CTRL_STPAFTER_STOP_ENUM_VAL << MC3377X_I2C_CTRL_STPAFTER_POS) +
             (0u << MC3377X_I2C_CTRL_RDAFTER_POS)),
        n77xTestState.pSpiTxSequence);
    N77x_CommunicationRead_ExpectAndReturn(
        validModuleNumber + 1u, MC3377X_I2C_STAT_OFFSET, &readData, &n77xTestState, N77X_COMMUNICATION_OK);
    N77x_Wait_Expect(2u);
    TEST_ASSERT_EQUAL(
        STD_OK,
        TEST_N77x_I2cTransmitWrite(validModuleNumber, validDeviceAddress, writeData, validDataLength, &n77xTestState));

    /* ======= RT3/3 ======= */
    /* First read not ok */
    N77x_CommunicationWrite_Expect(
        validModuleNumber + 1,
        MC3377X_I2C_DATA0_OFFSET,
        (validDeviceAddress << MC3377X_I2C_DATA0_BYTE0_POS) | (writeData[0] << MC3377X_I2C_DATA0_BYTE1_POS),
        n77xTestState.pSpiTxSequence);

    N77x_CommunicationWrite_Expect(
        validModuleNumber + 1u, MC3377X_I2C_DATA1_OFFSET, writeData[1], n77xTestState.pSpiTxSequence);

    N77x_CommunicationWrite_Expect(
        validModuleNumber + 1u,
        MC3377X_I2C_CTRL_OFFSET,
        /* transactionData.writeDataLength + 1u: data + I2C device address byte */
        ((validDataLength + 1u) << MC3377X_I2C_CTRL_START_POS) |
            ((MC3377X_I2C_CTRL_STPAFTER_STOP_ENUM_VAL << MC3377X_I2C_CTRL_STPAFTER_POS) +
             (0u << MC3377X_I2C_CTRL_RDAFTER_POS)),
        n77xTestState.pSpiTxSequence);
    N77x_CommunicationRead_ExpectAndReturn(
        validModuleNumber + 1u, MC3377X_I2C_STAT_OFFSET, &readData, &n77xTestState, N77X_COMMUNICATION_ERROR_TIMEOUT);
    N77x_Wait_Expect(2u);
    TEST_ASSERT_EQUAL(
        STD_NOT_OK,
        TEST_N77x_I2cTransmitWrite(validModuleNumber, validDeviceAddress, writeData, validDataLength, &n77xTestState));
}

void testN77x_I2cTransmitRead(void) {
    /* ======= Assertion tests ============================================= */
    uint8_t writeData[3]       = {0u, 0u, 0u};
    uint16_t readData          = 0u;
    uint16_t readData_returned = 0x4u; /* Set NACK to 1 */

    N77X_STATE_s n77xTestState = {
        .firstMeasurementMade = false,
        .pSpiTxSequence       = spi_nxp77xInterfaceTx,
    };

    const uint8_t validModuleNumber  = 0u;
    const uint8_t validDeviceAddress = 0u | N77X_I2C_READ;
    uint8_t validDataLength          = 1u;

    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(
        TEST_N77x_I2cTransmitRead(validModuleNumber, validDeviceAddress, writeData, validDataLength, NULL_PTR));

    /* ======= Routine tests =============================================== */

    /* ======= RT1/3 ======= */
    /* everything ok */
    N77x_CommunicationWrite_Expect(
        validModuleNumber + 1,
        MC3377X_I2C_DATA0_OFFSET,
        validDeviceAddress << MC3377X_I2C_DATA0_BYTE0_POS,
        n77xTestState.pSpiTxSequence);
    N77x_CommunicationWrite_Expect(
        validModuleNumber + 1u,
        MC3377X_I2C_CTRL_OFFSET,
        /* transactionData.writeDataLength + 1u: data + I2C device address byte */
        ((validDataLength + 1u) << MC3377X_I2C_CTRL_START_POS) |
            ((1u << MC3377X_I2C_CTRL_STPAFTER_POS) + (0u << MC3377X_I2C_CTRL_RDAFTER_POS)),
        n77xTestState.pSpiTxSequence);
    N77x_CommunicationRead_ExpectAndReturn(
        validModuleNumber + 1u, MC3377X_I2C_STAT_OFFSET, &readData, &n77xTestState, N77X_COMMUNICATION_OK);
    N77x_Wait_Expect(2u);
    N77x_CommunicationReadMultiple_ExpectAndReturn(
        validModuleNumber + 1u,
        (validDataLength / 2u) + 1u,
        4u,
        MC3377X_I2C_DATA0_OFFSET,
        &readData,
        &n77xTestState,
        N77X_COMMUNICATION_OK);

    TEST_ASSERT_EQUAL(
        STD_OK,
        TEST_N77x_I2cTransmitRead(validModuleNumber, validDeviceAddress, writeData, validDataLength, &n77xTestState));

    /* ======= RT2/3 ======= */
    /* Communication not ok */
    N77x_CommunicationWrite_Expect(
        validModuleNumber + 1,
        MC3377X_I2C_DATA0_OFFSET,
        validDeviceAddress << MC3377X_I2C_DATA0_BYTE0_POS,
        n77xTestState.pSpiTxSequence);
    N77x_CommunicationWrite_Expect(
        validModuleNumber + 1u,
        MC3377X_I2C_CTRL_OFFSET,
        /* transactionData.writeDataLength + 1u: data + I2C device address byte */
        ((validDataLength + 1u) << MC3377X_I2C_CTRL_START_POS) |
            ((1u << MC3377X_I2C_CTRL_STPAFTER_POS) + (0u << MC3377X_I2C_CTRL_RDAFTER_POS)),
        n77xTestState.pSpiTxSequence);
    N77x_CommunicationRead_ExpectAndReturn(
        validModuleNumber + 1u, MC3377X_I2C_STAT_OFFSET, &readData, &n77xTestState, N77X_COMMUNICATION_ERROR_TIMEOUT);
    N77x_Wait_Expect(2u);
    TEST_ASSERT_EQUAL(
        STD_NOT_OK,
        TEST_N77x_I2cTransmitRead(validModuleNumber, validDeviceAddress, writeData, validDataLength, &n77xTestState));

    /* ======= RT3/3 ======= */
    /* First NACK is 1 */
    N77x_CommunicationWrite_Expect(
        validModuleNumber + 1,
        MC3377X_I2C_DATA0_OFFSET,
        validDeviceAddress << MC3377X_I2C_DATA0_BYTE0_POS,
        n77xTestState.pSpiTxSequence);
    N77x_CommunicationWrite_Expect(
        validModuleNumber + 1u,
        MC3377X_I2C_CTRL_OFFSET,
        /* transactionData.writeDataLength + 1u: data + I2C device address byte */
        ((validDataLength + 1u) << MC3377X_I2C_CTRL_START_POS) |
            ((1u << MC3377X_I2C_CTRL_STPAFTER_POS) + (0u << MC3377X_I2C_CTRL_RDAFTER_POS)),
        n77xTestState.pSpiTxSequence);
    N77x_CommunicationRead_ExpectAndReturn(
        validModuleNumber + 1u, MC3377X_I2C_STAT_OFFSET, &readData, &n77xTestState, N77X_COMMUNICATION_OK);
    N77x_CommunicationRead_ReturnThruPtr_pValue(&readData_returned);
    N77x_Wait_Expect(2u);

    TEST_ASSERT_EQUAL(
        STD_NOT_OK,
        TEST_N77x_I2cTransmitRead(validModuleNumber, validDeviceAddress, writeData, validDataLength, &n77xTestState));
}
