/**
 *
 * @copyright &copy; 2010 - 2023, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * - &Prime;This product uses parts of foxBMS&reg;&Prime;
 * - &Prime;This product includes parts of foxBMS&reg;&Prime;
 * - &Prime;This product is derived from foxBMS&reg;&Prime;
 *
 */

/**
 * @file    test_adi_ades1830_helpers.c
 * @author  foxBMS Team
 * @date    2022-12-07 (date of creation)
 * @updated 2023-10-12 (date of last update)
 * @version v1.6.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Test of some module
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockos.h"
#include "Mockspi.h"

#include "adi_ades1830_helpers_test-data-rdb.h"
#include "adi_ades1830_helpers_test-data-wdb.h"
#include "adi_ades183x_buffers.h"           /* use the real buffer configuration */
#include "adi_ades183x_commands.h"          /* use the real command config */
#include "adi_ades183x_commands_voltages.h" /* use the real command config */
#include "adi_ades183x_defs.h"
#include "adi_ades183x_helpers.h"
#include "adi_ades183x_pec.h"
#include "spi_cfg-helper.h"
#include "test_assert_helper.h"

#include <stdbool.h>
#include <stdint.h>

/*========== Unit Testing Framework Directives ==============================*/
/* contains the expected output matrix for ADI_ReadDataBits */
TEST_SOURCE_FILE("adi_ades1830_helpers_test-data-rdb.c")
TEST_SOURCE_FILE("adi_ades1830_helpers_test-data-wdb.c")
TEST_SOURCE_FILE("adi_ades183x_buffers.c")
TEST_SOURCE_FILE("adi_ades183x_commands.c")
TEST_SOURCE_FILE("adi_ades183x_commands_voltages.c")
TEST_SOURCE_FILE("adi_ades183x_helpers.c")

TEST_INCLUDE_PATH("../../src/app/application/config")
TEST_INCLUDE_PATH("../../src/app/driver/afe/adi/ades1830")
TEST_INCLUDE_PATH("../../src/app/driver/afe/adi/common/ades183x")
TEST_INCLUDE_PATH("../../src/app/driver/afe/adi/common/ades183x/config")
TEST_INCLUDE_PATH("../../src/app/driver/afe/adi/common/ades183x/pec")
TEST_INCLUDE_PATH("../../src/app/driver/afe/api")
TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/dma")
TEST_INCLUDE_PATH("../../src/app/driver/io")
TEST_INCLUDE_PATH("../../src/app/driver/pex")
TEST_INCLUDE_PATH("../../src/app/driver/rtc")
TEST_INCLUDE_PATH("../../src/app/driver/spi")
TEST_INCLUDE_PATH("../../src/app/driver/ts/api")
TEST_INCLUDE_PATH("../../src/app/engine/database")
TEST_INCLUDE_PATH("../../src/app/engine/diag")
TEST_INCLUDE_PATH("../../src/app/task/config")
TEST_INCLUDE_PATH("../../src/app/task/ftask")

/* contains the expected output matrix for ADI_WriteDataBits */

/*========== Definitions and Implementations for Unit Test ==================*/

/** SPI data configuration struct for ADI communication */
static spiDAT1_t spi_kAdiDataConfig[BS_NR_OF_STRINGS] = {
    {                      /* struct is implemented in the TI HAL and uses uppercase true and false */
     .CS_HOLD = TRUE,      /* If true, HW chip select kept active between words */
     .WDEL    = FALSE,     /* Activation of delay between words */
     .DFSEL   = SPI_FMT_0, /* Data word format selection */
     .CSNR    = SPI_HARDWARE_CHIP_SELECT_2_ACTIVE},
};

/**
 * SPI interface configuration for ADI communication
 * This is a list of structs because of multi string
 */
SPI_INTERFACE_CONFIG_s spi_adiInterface[BS_NR_OF_STRINGS] = {
    {
        .pConfig  = &spi_kAdiDataConfig[0u],
        .pNode    = spiREG1,
        .pGioPort = &(spiREG1->PC3),
        .csPin    = 2u,
        .csType   = SPI_CHIP_SELECT_HARDWARE,
    },
};

static uint16_t adi_bufferRxPecTest[ADI_N_BYTES_FOR_DATA_TRANSMISSION] = {0};
static uint16_t adi_bufferTxPecTest[ADI_N_BYTES_FOR_DATA_TRANSMISSION] = {0};
static ADI_ERROR_TABLE_s adi_errorTableTest                            = {0};

static ADI_STATE_s adi_stateBaseTest = {
    .data.txBuffer   = adi_bufferTxPecTest,
    .data.rxBuffer   = adi_bufferRxPecTest,
    .data.errorTable = &adi_errorTableTest,
};

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/

void testADI_TransmitCommand(void) {
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        adi_stateBaseTest.currentString = s;
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            /* Initialize command counter to a definite value */
            adi_stateBaseTest.data.commandCounter[s][m] = 3u;
        }
    }

    SPI_TransmitDummyByte_IgnoreAndReturn(STD_OK);
    SPI_TransmitData_IgnoreAndReturn(STD_OK);

    /* TransmitCommand(): used for commands without data */
    /* SNAP: command without data , must increase command counter */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        adi_stateBaseTest.currentString = s;
        ADI_CopyCommandBits(adi_cmdSnap, adi_command);
        ADI_TransmitCommand(adi_command, &adi_stateBaseTest);
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            /* Check that command counter was increased */
            TEST_ASSERT_EQUAL(4u, adi_stateBaseTest.data.commandCounter[s][m]);
        }
    }
}

void testADI_CopyCommandBits(void) {
    const uint16_t testSourceCommand[ADI_COMMAND_DEFINITION_LENGTH] = {1u, 2u, 3u, 4u};
    uint16_t testDestinationCommand[ADI_COMMAND_DEFINITION_LENGTH]  = {0u, 0u, 0u, 0u};

    /* invalid sourceCommand */
    TEST_ASSERT_FAIL_ASSERT(ADI_CopyCommandBits(NULL_PTR, NULL_PTR));
    /* invalid destinationCommand */
    TEST_ASSERT_FAIL_ASSERT(ADI_CopyCommandBits(testSourceCommand, NULL_PTR));

    /* copy command and test that after calling the function both arrays have
       the same values for each index */
    ADI_CopyCommandBits(testSourceCommand, testDestinationCommand);
    for (uint8_t i = 0; i < ADI_COMMAND_DEFINITION_LENGTH; i++) {
        TEST_ASSERT_EQUAL(testSourceCommand[i], testDestinationCommand[i]);
    }
}

void testADI_IncrementCommandCounter(void) {
    /* Test increment without roll over */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        adi_stateBaseTest.currentString = s;
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            /* Initialize command counter to a definite value */
            adi_stateBaseTest.data.commandCounter[s][m] = 3u;
        }
        /* Increment command counter */
        TEST_ADI_IncrementCommandCounter(&adi_stateBaseTest);
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            /* Check that command counter was incremented correctly */
            TEST_ASSERT_EQUAL(4u, adi_stateBaseTest.data.commandCounter[s][m]);
        }
    }

    /* Test increment with roll over */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        adi_stateBaseTest.currentString = s;
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            /* Initialize command counter to maximum value */
            adi_stateBaseTest.data.commandCounter[s][m] = ADI_COMMAND_COUNTER_MAX_VALUE;
        }
        /* Increment command counter */
        TEST_ADI_IncrementCommandCounter(&adi_stateBaseTest);
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            /* Check that command counter was incremented correctly */
            TEST_ASSERT_EQUAL(ADI_COMMAND_COUNTER_RESTART_VALUE, adi_stateBaseTest.data.commandCounter[s][m]);
        }
    }
}

void testADI_WriteCommandConfigurationBits(void) {
    /* Start with ADCV command, store all command bytes in one variable */
    uint16_t modifiedCommand = (((uint16_t)ADI_ADCV_BYTE0) << 8u) | ADI_ADCV_BYTE1;
    /* Modify configuration bits */
    modifiedCommand |= (uint16_t)((1u & ADI_ADCV_RD_LEN) << ADI_ADCV_RD_POS);
    modifiedCommand |= (uint16_t)((1u & ADI_ADCV_CONT_LEN) << ADI_ADCV_CONT_POS);
    modifiedCommand &= (uint16_t) ~((1u & ADI_ADCV_DCP_LEN) << ADI_ADCV_DCP_POS);
    modifiedCommand |= (uint16_t)((1u & ADI_ADCV_RSTF_LEN) << ADI_ADCV_RSTF_POS);
    modifiedCommand &= (uint16_t) ~((1u & ADI_ADCV_OW01_LEN) << ADI_ADCV_OW01_POS);

    /* Now modify same configuration bits with ADI_WriteCommandConfigurationBits() */
    ADI_CopyCommandBits(adi_cmdAdcv, adi_command);
    ADI_WriteCommandConfigurationBits(adi_command, ADI_ADCV_RD_POS, ADI_ADCV_RD_LEN, 1u);
    ADI_WriteCommandConfigurationBits(adi_command, ADI_ADCV_CONT_POS, ADI_ADCV_CONT_LEN, 1u);
    ADI_WriteCommandConfigurationBits(adi_command, ADI_ADCV_DCP_POS, ADI_ADCV_DCP_LEN, 0u);
    ADI_WriteCommandConfigurationBits(adi_command, ADI_ADCV_RSTF_POS, ADI_ADCV_RSTF_LEN, 1u);
    ADI_WriteCommandConfigurationBits(adi_command, ADI_ADCV_OW01_POS, ADI_ADCV_OW01_LEN, 0u);

    /* Check that function produces the same command as the manual modification */
    TEST_ASSERT_EQUAL((modifiedCommand & 0xFF00u) >> 8u, adi_command[0u]);
    TEST_ASSERT_EQUAL(modifiedCommand & 0xFFu, adi_command[1u]);
}

void testADI_ReadDataBits(void) {
    uint8_t dummyData             = 0u;
    uint8_t dummyDataStorage      = 0u;
    const uint8_t invalidPosition = ADI_MAX_BIT_POSITION_IN_BYTE + 1u;
    const uint8_t validPosition   = 0u;
    const uint8_t invalidMask     = 0u;
    const uint8_t validMask       = 1u;
    /* first argument 'receivedData' accepts the whole parameter range */
    /* invalid 'pDataToRead' pointer (second argument, FIRST assert) */
    TEST_ASSERT_FAIL_ASSERT(ADI_ReadDataBits(dummyData, NULL_PTR, validPosition, validMask));
    /* invalid 'position' (third argument, SECOND assert) */
    TEST_ASSERT_FAIL_ASSERT(ADI_ReadDataBits(dummyData, &dummyDataStorage, invalidPosition, validMask));
    /* invalid 'mask' (fourth argument, THIRD assert) */
    TEST_ASSERT_FAIL_ASSERT(ADI_ReadDataBits(dummyData, &dummyDataStorage, validPosition, invalidMask));

    /* tests with valid input */
    uint8_t result = 0u;
    for (uint8_t i = 0; i < TEST_MATRIX_SIZE_READ_DATA_BITS; i++) {
        for (uint8_t j = 0; j < TEST_MATRIX_SIZE_READ_DATA_BITS; j++) {
            for (uint8_t k = 0; k < TEST_MATRIX_SIZE_READ_DATA_BITS; k++) {
                ADI_ReadDataBits(
                    testReadDataBitsDataReceivedData[i],
                    &result,
                    testReadDataBitsDataPositions[j],
                    testReadDataBitsDataMasks[k]);
                TEST_ASSERT_EQUAL(testReadDataBitsExpectedDataResults[i][j][k], result);
            }
        }
    }
}

void testADI_ReadRegister(void) {
    SPI_TransmitDummyByte_IgnoreAndReturn(STD_OK);
    SPI_TransmitReceiveDataDma_IgnoreAndReturn(STD_OK);
    OS_WaitForNotification_IgnoreAndReturn(OS_SUCCESS);

    /* Test when PEC corresponds to data: all flags in error table must indicate that PEC is OK */
    /* Prepare read data */
    for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
        /* Six bytes of data */
        adi_bufferRxPecTest
            [ADI_COMMAND_AND_PEC_SIZE_IN_BYTES + 0u + (m * (ADI_MAX_REGISTER_SIZE_IN_BYTES + ADI_PEC_SIZE_IN_BYTES))] =
                0x12u;
        adi_bufferRxPecTest
            [ADI_COMMAND_AND_PEC_SIZE_IN_BYTES + 1u + (m * (ADI_MAX_REGISTER_SIZE_IN_BYTES + ADI_PEC_SIZE_IN_BYTES))] =
                0x34u;
        adi_bufferRxPecTest
            [ADI_COMMAND_AND_PEC_SIZE_IN_BYTES + 2u + (m * (ADI_MAX_REGISTER_SIZE_IN_BYTES + ADI_PEC_SIZE_IN_BYTES))] =
                0x56u;
        adi_bufferRxPecTest
            [ADI_COMMAND_AND_PEC_SIZE_IN_BYTES + 3u + (m * (ADI_MAX_REGISTER_SIZE_IN_BYTES + ADI_PEC_SIZE_IN_BYTES))] =
                0x78u;
        adi_bufferRxPecTest
            [ADI_COMMAND_AND_PEC_SIZE_IN_BYTES + 4u + (m * (ADI_MAX_REGISTER_SIZE_IN_BYTES + ADI_PEC_SIZE_IN_BYTES))] =
                0x9Au;
        adi_bufferRxPecTest
            [ADI_COMMAND_AND_PEC_SIZE_IN_BYTES + 5u + (m * (ADI_MAX_REGISTER_SIZE_IN_BYTES + ADI_PEC_SIZE_IN_BYTES))] =
                0xBCu;
        /* In first byte of PEC, command counter (first 6 bits) + two MSB bits of PEC*/
        adi_bufferRxPecTest
            [ADI_COMMAND_AND_PEC_SIZE_IN_BYTES + 6u + (m * (ADI_MAX_REGISTER_SIZE_IN_BYTES + ADI_PEC_SIZE_IN_BYTES))] =
                0xDCu;
        /* Second byte of PEC */
        adi_bufferRxPecTest
            [ADI_COMMAND_AND_PEC_SIZE_IN_BYTES + 7u + (m * (ADI_MAX_REGISTER_SIZE_IN_BYTES + ADI_PEC_SIZE_IN_BYTES))] =
                0x53u;
    }
    /* Call read function */
    ADI_CopyCommandBits(adi_cmdRdcva, adi_command);
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        adi_stateBaseTest.currentString = s;
        ADI_ReadRegister(adi_command, adi_dataReceive, &adi_stateBaseTest);
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            /* All PEC values must be OK */
            TEST_ASSERT_EQUAL(true, adi_stateBaseTest.data.errorTable->crcIsOk[s][m]);
        }
    }

    /* Test with wrong PEC*/
    /* Modify data for module 0, without updating PEC; 0x78u is the correct value corresponding to the PEC */
    adi_bufferRxPecTest
        [ADI_COMMAND_AND_PEC_SIZE_IN_BYTES + 3u + (0u * (ADI_MAX_REGISTER_SIZE_IN_BYTES + ADI_PEC_SIZE_IN_BYTES))] =
            0x78u + 1u;
    /* Call read function */
    ADI_CopyCommandBits(adi_cmdRdcva, adi_command);
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        adi_stateBaseTest.currentString = s;
        ADI_ReadRegister(adi_command, adi_dataReceive, &adi_stateBaseTest);
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            /* All PEC values must be OK */
            if (m > 0u) {
                /* All but first module: there must be no PEC error */
                TEST_ASSERT_EQUAL(true, adi_stateBaseTest.data.errorTable->crcIsOk[s][m]);
            } else {
                /* First module: there must be a PEC error */
                TEST_ASSERT_EQUAL(false, adi_stateBaseTest.data.errorTable->crcIsOk[s][m]);
            }
        }
    }

    /* Test command counter correct value */
    /* Set value stored by driver */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        adi_stateBaseTest.currentString = s;
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            /* 0xDC is the value stored in the first PEC byte in the prepared received data.
               There the first six bits corresponds to the command counter so the mask 0xFC
               is used to extract the command counter bits. */
            adi_stateBaseTest.data.commandCounter[s][m] = (0xDCu & 0xFCu) >> 2u;
            /* Command counter flags are set at initialization, so it must be done before the test */
            adi_stateBaseTest.data.errorTable->commandCounterIsOk[s][m] = true;
        }
    }
    /* Call read function */
    ADI_CopyCommandBits(adi_cmdRdcva, adi_command);
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        adi_stateBaseTest.currentString = s;
        ADI_ReadRegister(adi_command, adi_dataReceive, &adi_stateBaseTest);
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            /* All command counter values must be OK */
            TEST_ASSERT_EQUAL(true, adi_stateBaseTest.data.errorTable->commandCounterIsOk[s][m]);
        }
    }

    /* Test command counter wrong value */
    /* Set value stored by driver */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        adi_stateBaseTest.currentString = s;
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            /* 0xDC is the value stored in the first PEC byte in the prepared received data.
               There the first six bits corresponds to the command counter so the mask 0xFC
               is used to extract the command counter bits. */
            adi_stateBaseTest.data.commandCounter[s][m] = (0xDCu & 0xFCu) >> 2u;
            if (m == 0u) {
                /* Set wrong command counter for first module */
                adi_stateBaseTest.data.commandCounter[s][m]++;
            }
            /* Command counter flags are set at initialization, so it must be done before the test */
            adi_stateBaseTest.data.errorTable->commandCounterIsOk[s][m] = true;
        }
    }
    /* Call read function */
    ADI_CopyCommandBits(adi_cmdRdcva, adi_command);
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        adi_stateBaseTest.currentString = s;
        ADI_ReadRegister(adi_command, adi_dataReceive, &adi_stateBaseTest);
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            /* Test command counter values */
            if (m > 0u) {
                /* All but first module: there must be no command counter error */
                TEST_ASSERT_EQUAL(true, adi_stateBaseTest.data.errorTable->commandCounterIsOk[s][m]);
            } else {
                /* First module: there must be a command counter error */
                TEST_ASSERT_EQUAL(false, adi_stateBaseTest.data.errorTable->commandCounterIsOk[s][m]);
            }
        }
    }
}

void testADI_StoredConfigurationWriteToAfe(void) {
    /* Test invalid register set */
    TEST_ASSERT_FAIL_ASSERT(ADI_StoredConfigurationWriteToAfe(ADI_CFG_REGISTER_SET_E_MAX, &adi_stateBaseTest));
    /* Test invalid state */
    TEST_ASSERT_FAIL_ASSERT(ADI_StoredConfigurationWriteToAfe(ADI_CFG_REGISTER_SET_A, NULL_PTR));

    uint8_t frameLength = ADI_COMMAND_AND_PEC_SIZE_IN_BYTES + ((ADI_RDCFGA_LEN + ADI_PEC_SIZE_IN_BYTES) * ADI_N_ADI);
    /* SPI functions called by write register */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        adi_stateBaseTest.currentString = s;

        /* First test: matching data in written and read configuration */

        for (uint16_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t byte = 0; byte < ADI_MAX_REGISTER_SIZE_IN_BYTES; byte++) {
                /* Write same data to both table for configuration register A */
                adi_configurationRegisterAgroup[s][byte + (m * ADI_WRCFGA_LEN)]     = byte;
                adi_readConfigurationRegisterAgroup[s][byte + (m * ADI_WRCFGA_LEN)] = byte;
                /* Write same data to both table for configuration register B */
                adi_configurationRegisterBgroup[s][byte + (m * ADI_WRCFGB_LEN)]     = byte;
                adi_readConfigurationRegisterBgroup[s][byte + (m * ADI_WRCFGB_LEN)] = byte;
            }
        }
        for (uint16_t i = 4u; i < ADI_N_BYTES_FOR_DATA_TRANSMISSION; i += 8) {
            /* Set receive buffer with raw data */
            adi_bufferRxPecTest[i]      = 0u;
            adi_bufferRxPecTest[i + 1u] = 1u;
            adi_bufferRxPecTest[i + 2u] = 2u;
            adi_bufferRxPecTest[i + 3u] = 3u;
            adi_bufferRxPecTest[i + 4u] = 4u;
            adi_bufferRxPecTest[i + 5u] = 5u;
            /* Two PEC bytes; check will fail but unused in this test */
            adi_bufferRxPecTest[i + 6u] = 0u;
            adi_bufferRxPecTest[i + 7u] = 0u;
        }

        /* SPI functions called by write register */
        SPI_TransmitDummyByte_ExpectAndReturn(
            &spi_adiInterface[adi_stateBaseTest.currentString], ADI_SPI_WAKEUP_WAIT_TIME_US, STD_OK);
        SPI_TransmitReceiveDataDma_ExpectAndReturn(
            &spi_adiInterface[adi_stateBaseTest.currentString],
            adi_stateBaseTest.data.txBuffer,
            adi_stateBaseTest.data.rxBuffer,
            frameLength,
            STD_OK);
        OS_WaitForNotification_IgnoreAndReturn(OS_SUCCESS);
        /* SPI functions called by read register */
        SPI_TransmitDummyByte_ExpectAndReturn(
            &spi_adiInterface[adi_stateBaseTest.currentString], ADI_SPI_WAKEUP_WAIT_TIME_US, STD_OK);
        SPI_TransmitReceiveDataDma_ExpectAndReturn(
            &spi_adiInterface[adi_stateBaseTest.currentString],
            adi_stateBaseTest.data.txBuffer,
            adi_stateBaseTest.data.rxBuffer,
            frameLength,
            STD_OK);
        OS_WaitForNotification_IgnoreAndReturn(OS_SUCCESS);
        /* Matching written and read data: function must return STD_OK */
        TEST_ASSERT_EQUAL(STD_OK, ADI_StoredConfigurationWriteToAfe(ADI_CFG_REGISTER_SET_A, &adi_stateBaseTest));
        /* SPI functions called by write register */
        SPI_TransmitDummyByte_ExpectAndReturn(
            &spi_adiInterface[adi_stateBaseTest.currentString], ADI_SPI_WAKEUP_WAIT_TIME_US, STD_OK);
        SPI_TransmitReceiveDataDma_ExpectAndReturn(
            &spi_adiInterface[adi_stateBaseTest.currentString],
            adi_stateBaseTest.data.txBuffer,
            adi_stateBaseTest.data.rxBuffer,
            frameLength,
            STD_OK);
        OS_WaitForNotification_IgnoreAndReturn(OS_SUCCESS);
        /* SPI functions called by read register */
        SPI_TransmitDummyByte_ExpectAndReturn(
            &spi_adiInterface[adi_stateBaseTest.currentString], ADI_SPI_WAKEUP_WAIT_TIME_US, STD_OK);
        SPI_TransmitReceiveDataDma_ExpectAndReturn(
            &spi_adiInterface[adi_stateBaseTest.currentString],
            adi_stateBaseTest.data.txBuffer,
            adi_stateBaseTest.data.rxBuffer,
            frameLength,
            STD_OK);
        /* Matching written and read data: function must return STD_OK */
        TEST_ASSERT_EQUAL(STD_OK, ADI_StoredConfigurationWriteToAfe(ADI_CFG_REGISTER_SET_B, &adi_stateBaseTest));

        /* Second test: unmatching data in written and read configuration */

        for (uint16_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t byte = 0; byte < ADI_MAX_REGISTER_SIZE_IN_BYTES; byte++) {
                /* Write different data to tables for configuration register A */
                adi_configurationRegisterAgroup[s][byte + (m * ADI_WRCFGA_LEN)]     = byte;
                adi_readConfigurationRegisterAgroup[s][byte + (m * ADI_WRCFGA_LEN)] = byte + 1u;
                /* Write different data to tables for configuration register B */
                adi_configurationRegisterBgroup[s][byte + (m * ADI_WRCFGB_LEN)]     = byte;
                adi_readConfigurationRegisterBgroup[s][byte + (m * ADI_WRCFGB_LEN)] = byte + 1u;
            }
        }
        for (uint16_t i = 4u; i < ADI_N_BYTES_FOR_DATA_TRANSMISSION; i += 8) {
            /* Set receive buffer with raw data */
            adi_bufferRxPecTest[i]      = 0u + 1u;
            adi_bufferRxPecTest[i + 1u] = 1u + 1u;
            adi_bufferRxPecTest[i + 2u] = 2u + 1u;
            adi_bufferRxPecTest[i + 3u] = 3u + 1u;
            adi_bufferRxPecTest[i + 4u] = 4u + 1u;
            adi_bufferRxPecTest[i + 5u] = 5u + 1u;
            /* Two PEC bytes; check will fail but unused in this test */
            adi_bufferRxPecTest[i + 6u] = 0u;
            adi_bufferRxPecTest[i + 7u] = 0u;
        }

        /* SPI functions called by write register */
        SPI_TransmitDummyByte_ExpectAndReturn(
            &spi_adiInterface[adi_stateBaseTest.currentString], ADI_SPI_WAKEUP_WAIT_TIME_US, STD_OK);
        SPI_TransmitReceiveDataDma_ExpectAndReturn(
            &spi_adiInterface[adi_stateBaseTest.currentString],
            adi_stateBaseTest.data.txBuffer,
            adi_stateBaseTest.data.rxBuffer,
            frameLength,
            STD_OK);
        OS_WaitForNotification_IgnoreAndReturn(OS_SUCCESS);
        /* SPI functions called by read register */
        SPI_TransmitDummyByte_ExpectAndReturn(
            &spi_adiInterface[adi_stateBaseTest.currentString], ADI_SPI_WAKEUP_WAIT_TIME_US, STD_OK);
        SPI_TransmitReceiveDataDma_ExpectAndReturn(
            &spi_adiInterface[adi_stateBaseTest.currentString],
            adi_stateBaseTest.data.txBuffer,
            adi_stateBaseTest.data.rxBuffer,
            frameLength,
            STD_OK);
        OS_WaitForNotification_IgnoreAndReturn(OS_SUCCESS);
        /* Mismatching written and read data: function must return STD_NOT_OK */
        TEST_ASSERT_EQUAL(STD_NOT_OK, ADI_StoredConfigurationWriteToAfe(ADI_CFG_REGISTER_SET_A, &adi_stateBaseTest));
        /* SPI functions called by write register */
        SPI_TransmitDummyByte_ExpectAndReturn(
            &spi_adiInterface[adi_stateBaseTest.currentString], ADI_SPI_WAKEUP_WAIT_TIME_US, STD_OK);
        SPI_TransmitReceiveDataDma_ExpectAndReturn(
            &spi_adiInterface[adi_stateBaseTest.currentString],
            adi_stateBaseTest.data.txBuffer,
            adi_stateBaseTest.data.rxBuffer,
            frameLength,
            STD_OK);
        OS_WaitForNotification_IgnoreAndReturn(OS_SUCCESS);
        /* SPI functions called by read register */
        SPI_TransmitDummyByte_ExpectAndReturn(
            &spi_adiInterface[adi_stateBaseTest.currentString], ADI_SPI_WAKEUP_WAIT_TIME_US, STD_OK);
        SPI_TransmitReceiveDataDma_ExpectAndReturn(
            &spi_adiInterface[adi_stateBaseTest.currentString],
            adi_stateBaseTest.data.txBuffer,
            adi_stateBaseTest.data.rxBuffer,
            frameLength,
            STD_OK);
        /* Mismatching written and read data: function must return STD_NOT_OK */
        TEST_ASSERT_EQUAL(STD_NOT_OK, ADI_StoredConfigurationWriteToAfe(ADI_CFG_REGISTER_SET_B, &adi_stateBaseTest));
    }
}

void testADI_StoredConfigurationWriteToAfeGlobal(void) {
    /* Test invalid state */
    TEST_ASSERT_FAIL_ASSERT(ADI_StoredConfigurationWriteToAfeGlobal(NULL_PTR));

    uint8_t frameLength = ADI_COMMAND_AND_PEC_SIZE_IN_BYTES + ((ADI_RDCFGA_LEN + ADI_PEC_SIZE_IN_BYTES) * ADI_N_ADI);
    /* SPI functions called by write register */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        adi_stateBaseTest.currentString = s;

        /* First test: matching data in written and read configuration */

        for (uint16_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            /* Reset all configuration error flags to no error */
            adi_stateBaseTest.data.errorTable->configurationAIsOk[s][m] = true;
            adi_stateBaseTest.data.errorTable->configurationBIsOk[s][m] = true;
            for (uint8_t byte = 0; byte < ADI_MAX_REGISTER_SIZE_IN_BYTES; byte++) {
                /* Write same data to both table for configuration register A */
                adi_configurationRegisterAgroup[s][byte + (m * ADI_WRCFGA_LEN)]     = byte;
                adi_readConfigurationRegisterAgroup[s][byte + (m * ADI_WRCFGA_LEN)] = byte;
                /* Write same data to both table for configuration register B */
                adi_configurationRegisterBgroup[s][byte + (m * ADI_WRCFGB_LEN)]     = byte;
                adi_readConfigurationRegisterBgroup[s][byte + (m * ADI_WRCFGB_LEN)] = byte;
            }
        }
        for (uint16_t i = 4u; i < ADI_N_BYTES_FOR_DATA_TRANSMISSION; i += 8) {
            /* Set receive buffer with raw data */
            adi_bufferRxPecTest[i]      = 0u;
            adi_bufferRxPecTest[i + 1u] = 1u;
            adi_bufferRxPecTest[i + 2u] = 2u;
            adi_bufferRxPecTest[i + 3u] = 3u;
            adi_bufferRxPecTest[i + 4u] = 4u;
            adi_bufferRxPecTest[i + 5u] = 5u;
            /* Two PEC bytes; check will fail but unused in this test */
            adi_bufferRxPecTest[i + 6u] = 0u;
            adi_bufferRxPecTest[i + 7u] = 0u;
        }

        /* SPI functions called by write register */
        SPI_TransmitDummyByte_ExpectAndReturn(
            &spi_adiInterface[adi_stateBaseTest.currentString], ADI_SPI_WAKEUP_WAIT_TIME_US, STD_OK);
        SPI_TransmitReceiveDataDma_ExpectAndReturn(
            &spi_adiInterface[adi_stateBaseTest.currentString],
            adi_stateBaseTest.data.txBuffer,
            adi_stateBaseTest.data.rxBuffer,
            frameLength,
            STD_OK);
        OS_WaitForNotification_IgnoreAndReturn(OS_SUCCESS);
        /* SPI functions called by read register */
        SPI_TransmitDummyByte_ExpectAndReturn(
            &spi_adiInterface[adi_stateBaseTest.currentString], ADI_SPI_WAKEUP_WAIT_TIME_US, STD_OK);
        SPI_TransmitReceiveDataDma_ExpectAndReturn(
            &spi_adiInterface[adi_stateBaseTest.currentString],
            adi_stateBaseTest.data.txBuffer,
            adi_stateBaseTest.data.rxBuffer,
            frameLength,
            STD_OK);
        OS_WaitForNotification_IgnoreAndReturn(OS_SUCCESS);
        /* SPI functions called by write register */
        SPI_TransmitDummyByte_ExpectAndReturn(
            &spi_adiInterface[adi_stateBaseTest.currentString], ADI_SPI_WAKEUP_WAIT_TIME_US, STD_OK);
        SPI_TransmitReceiveDataDma_ExpectAndReturn(
            &spi_adiInterface[adi_stateBaseTest.currentString],
            adi_stateBaseTest.data.txBuffer,
            adi_stateBaseTest.data.rxBuffer,
            frameLength,
            STD_OK);
        OS_WaitForNotification_IgnoreAndReturn(OS_SUCCESS);
        /* SPI functions called by read register */
        SPI_TransmitDummyByte_ExpectAndReturn(
            &spi_adiInterface[adi_stateBaseTest.currentString], ADI_SPI_WAKEUP_WAIT_TIME_US, STD_OK);
        SPI_TransmitReceiveDataDma_ExpectAndReturn(
            &spi_adiInterface[adi_stateBaseTest.currentString],
            adi_stateBaseTest.data.txBuffer,
            adi_stateBaseTest.data.rxBuffer,
            frameLength,
            STD_OK);
        ADI_StoredConfigurationWriteToAfeGlobal(&adi_stateBaseTest);
        /* Matching written and read data: error flags must not be set */
        for (uint16_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            /* Reset all configuration error flags to no error */
            adi_stateBaseTest.data.errorTable->configurationAIsOk[s][m] = true;
            adi_stateBaseTest.data.errorTable->configurationBIsOk[s][m] = true;
        }

        /* Second test: unmatching data in written and read configuration */

        for (uint16_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            /* Reset all configuration error flags to no error */
            adi_stateBaseTest.data.errorTable->configurationAIsOk[s][m] = true;
            adi_stateBaseTest.data.errorTable->configurationBIsOk[s][m] = true;
            for (uint8_t byte = 0; byte < ADI_MAX_REGISTER_SIZE_IN_BYTES; byte++) {
                /* Write different data to tables for configuration register A */
                adi_configurationRegisterAgroup[s][byte + (m * ADI_WRCFGA_LEN)]     = byte;
                adi_readConfigurationRegisterAgroup[s][byte + (m * ADI_WRCFGA_LEN)] = byte + 1u;
                /* Write different data to tables for configuration register B */
                adi_configurationRegisterBgroup[s][byte + (m * ADI_WRCFGB_LEN)]     = byte;
                adi_readConfigurationRegisterBgroup[s][byte + (m * ADI_WRCFGB_LEN)] = byte + 1u;
            }
        }
        for (uint16_t i = 4u; i < ADI_N_BYTES_FOR_DATA_TRANSMISSION; i += 8) {
            /* Set receive buffer with raw data */
            adi_bufferRxPecTest[i]      = 0u + 1u;
            adi_bufferRxPecTest[i + 1u] = 1u + 1u;
            adi_bufferRxPecTest[i + 2u] = 2u + 1u;
            adi_bufferRxPecTest[i + 3u] = 3u + 1u;
            adi_bufferRxPecTest[i + 4u] = 4u + 1u;
            adi_bufferRxPecTest[i + 5u] = 5u + 1u;
            /* Two PEC bytes; check will fail but unused in this test */
            adi_bufferRxPecTest[i + 6u] = 0u;
            adi_bufferRxPecTest[i + 7u] = 0u;
        }

        /* Mismatching written and read data: a retry is made once for each register
           so the write and read functions are called twice */
        for (uint8_t try = 0u; try < 2u; try++) { /* SPI functions called by write register */
                                                  /* SPI functions called by write register */
            SPI_TransmitDummyByte_ExpectAndReturn(
                &spi_adiInterface[adi_stateBaseTest.currentString], ADI_SPI_WAKEUP_WAIT_TIME_US, STD_OK);
            SPI_TransmitReceiveDataDma_ExpectAndReturn(
                &spi_adiInterface[adi_stateBaseTest.currentString],
                adi_stateBaseTest.data.txBuffer,
                adi_stateBaseTest.data.rxBuffer,
                frameLength,
                STD_OK);
            OS_WaitForNotification_IgnoreAndReturn(OS_SUCCESS);
            /* SPI functions called by read register */
            SPI_TransmitDummyByte_ExpectAndReturn(
                &spi_adiInterface[adi_stateBaseTest.currentString], ADI_SPI_WAKEUP_WAIT_TIME_US, STD_OK);
            SPI_TransmitReceiveDataDma_ExpectAndReturn(
                &spi_adiInterface[adi_stateBaseTest.currentString],
                adi_stateBaseTest.data.txBuffer,
                adi_stateBaseTest.data.rxBuffer,
                frameLength,
                STD_OK);
            OS_WaitForNotification_IgnoreAndReturn(OS_SUCCESS);
            /* SPI functions called by write register */
            SPI_TransmitDummyByte_ExpectAndReturn(
                &spi_adiInterface[adi_stateBaseTest.currentString], ADI_SPI_WAKEUP_WAIT_TIME_US, STD_OK);
            SPI_TransmitReceiveDataDma_ExpectAndReturn(
                &spi_adiInterface[adi_stateBaseTest.currentString],
                adi_stateBaseTest.data.txBuffer,
                adi_stateBaseTest.data.rxBuffer,
                frameLength,
                STD_OK);
            OS_WaitForNotification_IgnoreAndReturn(OS_SUCCESS);
            /* SPI functions called by read register */
            SPI_TransmitDummyByte_ExpectAndReturn(
                &spi_adiInterface[adi_stateBaseTest.currentString], ADI_SPI_WAKEUP_WAIT_TIME_US, STD_OK);
            SPI_TransmitReceiveDataDma_ExpectAndReturn(
                &spi_adiInterface[adi_stateBaseTest.currentString],
                adi_stateBaseTest.data.txBuffer,
                adi_stateBaseTest.data.rxBuffer,
                frameLength,
                STD_OK);
        }
        ADI_StoredConfigurationWriteToAfeGlobal(&adi_stateBaseTest);
        /* Mismatching written and read data: error flags must be set */
        for (uint16_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            /* Reset all configuration error flags to no error */
            adi_stateBaseTest.data.errorTable->configurationAIsOk[s][m] = false;
            adi_stateBaseTest.data.errorTable->configurationBIsOk[s][m] = false;
        }
    }
}

void testADI_CheckConfigurationRegister(void) {
    /* Test invalid register set */
    TEST_ASSERT_FAIL_ASSERT(ADI_CheckConfigurationRegister(ADI_CFG_REGISTER_SET_E_MAX, &adi_stateBaseTest));
    /* Test invalid state */
    TEST_ASSERT_FAIL_ASSERT(ADI_CheckConfigurationRegister(ADI_CFG_REGISTER_SET_A, NULL_PTR));

    /* adi_configurationRegisterAgroup[] and adi_configurationRegisterBgroup[] contain the driver copy
       of the configuration register. The configuration must be read from the daisy-chain and stored in
       adi_readConfigurationRegisterAgroup[] and adi_readConfigurationRegisterBgroup[].
       ADI_CheckConfigurationRegister() compares both tables for each configuration register. */

    /* Test for identical data */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        adi_stateBaseTest.currentString = s;
        for (uint16_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            /* Reset all configuration error flags to no error */
            adi_stateBaseTest.data.errorTable->configurationAIsOk[s][m] = true;
            adi_stateBaseTest.data.errorTable->configurationBIsOk[s][m] = true;
            for (uint8_t byte = 0; byte < ADI_MAX_REGISTER_SIZE_IN_BYTES; byte++) {
                /* Write same data to both table for configuration register A */
                adi_configurationRegisterAgroup[s][byte + (m * ADI_WRCFGA_LEN)]     = byte;
                adi_readConfigurationRegisterAgroup[s][byte + (m * ADI_WRCFGA_LEN)] = byte;
                /* Write same data to both table for configuration register B */
                adi_configurationRegisterBgroup[s][byte + (m * ADI_WRCFGB_LEN)]     = byte;
                adi_readConfigurationRegisterBgroup[s][byte + (m * ADI_WRCFGB_LEN)] = byte;
            }
        }
        /* Call check function */
        ADI_CheckConfigurationRegister(ADI_CFG_REGISTER_SET_A, &adi_stateBaseTest);
        ADI_CheckConfigurationRegister(ADI_CFG_REGISTER_SET_B, &adi_stateBaseTest);
        for (uint16_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            /* There must be no configuration error */
            TEST_ASSERT_EQUAL(true, adi_stateBaseTest.data.errorTable->configurationAIsOk[s][m]);
            TEST_ASSERT_EQUAL(true, adi_stateBaseTest.data.errorTable->configurationBIsOk[s][m]);
        }
    }

    /* Test for mismatching data */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        adi_stateBaseTest.currentString = s;
        for (uint16_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            /* Reset all configuration error flags to no error */
            adi_stateBaseTest.data.errorTable->configurationAIsOk[s][m] = true;
            adi_stateBaseTest.data.errorTable->configurationBIsOk[s][m] = true;
            for (uint8_t byte = 0; byte < ADI_MAX_REGISTER_SIZE_IN_BYTES; byte++) {
                if (m > 0u) {
                    /* Write matching data for all modules except the first 0 */
                    /* Register A */
                    adi_configurationRegisterAgroup[s][byte + (m * ADI_WRCFGA_LEN)]     = byte;
                    adi_readConfigurationRegisterAgroup[s][byte + (m * ADI_WRCFGA_LEN)] = byte;
                    /* Register B */
                    adi_configurationRegisterBgroup[s][byte + (m * ADI_WRCFGA_LEN)]     = byte;
                    adi_readConfigurationRegisterBgroup[s][byte + (m * ADI_WRCFGA_LEN)] = byte;
                } else {
                    /* Write mismatching data for module 0 */
                    /* Register A */
                    adi_configurationRegisterAgroup[s][byte + (m * ADI_WRCFGA_LEN)]     = byte;
                    adi_readConfigurationRegisterAgroup[s][byte + (m * ADI_WRCFGA_LEN)] = byte + 1u;
                    /* Register B */
                    adi_configurationRegisterBgroup[s][byte + (m * ADI_WRCFGA_LEN)]     = byte;
                    adi_readConfigurationRegisterBgroup[s][byte + (m * ADI_WRCFGA_LEN)] = byte + 1u;
                }
            }
        }
        /* Call check function */
        ADI_CheckConfigurationRegister(ADI_CFG_REGISTER_SET_A, &adi_stateBaseTest);
        ADI_CheckConfigurationRegister(ADI_CFG_REGISTER_SET_B, &adi_stateBaseTest);
        for (uint16_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            if (m > 0u) {
                /* There must be no configuration error for modules after module 0 */
                TEST_ASSERT_EQUAL(true, adi_stateBaseTest.data.errorTable->configurationAIsOk[s][m]);
                TEST_ASSERT_EQUAL(true, adi_stateBaseTest.data.errorTable->configurationBIsOk[s][m]);
            } else {
                /* There must be a configuration error for module 0 */
                TEST_ASSERT_EQUAL(false, adi_stateBaseTest.data.errorTable->configurationAIsOk[s][m]);
                TEST_ASSERT_EQUAL(false, adi_stateBaseTest.data.errorTable->configurationBIsOk[s][m]);
            }
        }
    }
}

void testADI_WriteRegisterGlobal(void) {
    /* Test invalid state */
    TEST_ASSERT_FAIL_ASSERT(
        ADI_WriteRegisterGlobal(adi_cmdWrpwma, adi_writeGlobal, ADI_PEC_NO_FAULT_INJECTION, NULL_PTR));
    /* Test invalid data to write */
    TEST_ASSERT_FAIL_ASSERT(
        ADI_WriteRegisterGlobal(adi_cmdWrpwma, NULL_PTR, ADI_PEC_NO_FAULT_INJECTION, &adi_stateBaseTest));
    /* Test invalid command */
    TEST_ASSERT_FAIL_ASSERT(
        ADI_WriteRegisterGlobal(NULL_PTR, adi_writeGlobal, ADI_PEC_NO_FAULT_INJECTION, &adi_stateBaseTest));

    /* Write register global calls write register for all module
       Before calling, the byte in the passed data buffer (adi_writeGlobal, size = max_register_size) are copied
       to the buffer for all modules (adi_dataTransmit, size = max_register_size * number_of_modules) */
    /* Test that the copy is made correctly */
    SPI_TransmitDummyByte_IgnoreAndReturn(STD_OK);
    SPI_TransmitReceiveDataDma_IgnoreAndReturn(STD_OK);
    OS_WaitForNotification_IgnoreAndReturn(OS_SUCCESS);
    /* Prepare global data */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        adi_stateBaseTest.currentString = s;
        for (uint16_t byte = 0u; byte < adi_cmdWrpwma[ADI_COMMAND_DATA_LENGTH_POSITION]; byte++) {
            adi_writeGlobal[byte] = byte;
        }
        ADI_WriteRegisterGlobal(adi_cmdWrpwma, adi_writeGlobal, ADI_PEC_NO_FAULT_INJECTION, &adi_stateBaseTest);
        for (uint16_t i = 0u; i < BS_NR_OF_MODULES_PER_STRING; i++) {
            for (uint8_t byte = 0; byte < adi_cmdWrpwma[ADI_COMMAND_DATA_LENGTH_POSITION]; byte++) {
                /* Check that global data was copied successfully for all modules */
                TEST_ASSERT_EQUAL(
                    adi_writeGlobal[byte],
                    adi_dataTransmit[byte + (i * adi_cmdWrpwma[ADI_COMMAND_DATA_LENGTH_POSITION])]);
            }
        }
    }
}

void testADI_ClearCommandCounter(void) {
    /* Test invalid state */
    TEST_ASSERT_FAIL_ASSERT(ADI_ClearCommandCounter(NULL_PTR));

    SPI_TransmitDummyByte_IgnoreAndReturn(STD_OK);
    SPI_TransmitData_IgnoreAndReturn(STD_OK);

    /* Test command counter clear */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        adi_stateBaseTest.currentString = s;
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            /* Set value stored by driver to a value different that reset value */
            adi_stateBaseTest.data.commandCounter[s][m] = ADI_COMMAND_COUNTER_RESET_VALUE + 1u;
            /* Set command counter flags to error */
            adi_stateBaseTest.data.errorTable->commandCounterIsOk[s][m] = false;
        }
    }
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        adi_stateBaseTest.currentString = s;
        /* Clear command counter */
        ADI_ClearCommandCounter(&adi_stateBaseTest);
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            /* Value stored by driver must be set to reset value */
            TEST_ASSERT_EQUAL(ADI_COMMAND_COUNTER_RESET_VALUE, adi_stateBaseTest.data.commandCounter[s][m]);
            /* Flags must be reset to no error */
            TEST_ASSERT_EQUAL(true, adi_stateBaseTest.data.errorTable->commandCounterIsOk[s][m]);
        }
    }
}

void testADI_SpiTransmitReceiveData(void) {
    /* Test invalid state */
    TEST_ASSERT_FAIL_ASSERT(
        ADI_SpiTransmitReceiveData(NULL_PTR, adi_stateBaseTest.data.txBuffer, adi_stateBaseTest.data.rxBuffer, 1u));
    /* Test invalid Tx buffer */
    TEST_ASSERT_FAIL_ASSERT(
        ADI_SpiTransmitReceiveData(&adi_stateBaseTest, NULL_PTR, adi_stateBaseTest.data.rxBuffer, 1u));

    /* Test sending a dummy byte only */
    /* Rx = NULLPTR and length = 0u */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        adi_stateBaseTest.currentString = s;
        /* Only send dummy byte */
        SPI_TransmitDummyByte_ExpectAndReturn(&spi_adiInterface[adi_stateBaseTest.currentString], 0u, STD_OK);
        ADI_SpiTransmitReceiveData(&adi_stateBaseTest, adi_stateBaseTest.data.txBuffer, NULL_PTR, 0u);
    }

    /* Test sending a command without data */
    /* Rx = NULLPTR and length > 0u */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        adi_stateBaseTest.currentString = s;
        /* First send dummy byte */
        SPI_TransmitDummyByte_ExpectAndReturn(
            &spi_adiInterface[adi_stateBaseTest.currentString], ADI_SPI_WAKEUP_WAIT_TIME_US, STD_OK);
        /* Then transmit command bytes, without using DMA */
        SPI_TransmitData_ExpectAndReturn(
            &spi_adiInterface[adi_stateBaseTest.currentString], adi_stateBaseTest.data.txBuffer, 4u, STD_OK);
        ADI_SpiTransmitReceiveData(&adi_stateBaseTest, adi_stateBaseTest.data.txBuffer, NULL_PTR, 4u);
    }

    /* Test transmitting a command with data */
    /* Tx and Rx != NULLPTR, length > 0u */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        adi_stateBaseTest.currentString = s;
        /* First send dummy byte */
        SPI_TransmitDummyByte_ExpectAndReturn(
            &spi_adiInterface[adi_stateBaseTest.currentString], ADI_SPI_WAKEUP_WAIT_TIME_US, STD_OK);
        /* Then transmit command bytes and data (4 bytes command, 6 bytes data, 2 bytes PEC), using DMA */
        SPI_TransmitReceiveDataDma_ExpectAndReturn(
            &spi_adiInterface[adi_stateBaseTest.currentString],
            adi_stateBaseTest.data.txBuffer,
            adi_stateBaseTest.data.rxBuffer,
            4u + 6u + 2u,
            STD_OK);
        /* When using SPI with DMA, block task and wait for notification in DMA Rx complete interrupt */
        OS_WaitForNotification_IgnoreAndReturn(OS_SUCCESS);
        ADI_SpiTransmitReceiveData(
            &adi_stateBaseTest, adi_stateBaseTest.data.txBuffer, adi_stateBaseTest.data.rxBuffer, 4u + 6u + 2u);
    }
}

void testADI_StoredConfigurationFillRegisterData(void) {
    uint8_t module                     = 0u;
    ADI_CFG_REGISTER_SET_e registerSet = ADI_CFG_REGISTER_SET_A;
    uint8_t registerOffset             = 0u;
    uint8_t data                       = 0u;
    uint8_t position                   = 0u;
    uint8_t mask                       = 1u;

    /* Test invalid register set */
    registerSet = ADI_CFG_REGISTER_SET_E_MAX;
    TEST_ASSERT_FAIL_ASSERT(TEST_ADI_StoredConfigurationFillRegisterData(
        module, registerSet, registerOffset, data, position, mask, &adi_stateBaseTest));
    /* Test invalid register offset */
    registerOffset = ADI_REGISTER_OFFSET5 + 1u;
    TEST_ASSERT_FAIL_ASSERT(TEST_ADI_StoredConfigurationFillRegisterData(
        module, registerSet, registerOffset, data, position, mask, &adi_stateBaseTest));
    /* Test invalid position */
    position = ADI_MAX_BIT_POSITION_IN_BYTE + 1u;
    TEST_ASSERT_FAIL_ASSERT(TEST_ADI_StoredConfigurationFillRegisterData(
        module, registerSet, registerOffset, data, position, mask, &adi_stateBaseTest));
    /* Test invalid mask */
    mask = 0u;
    TEST_ASSERT_FAIL_ASSERT(TEST_ADI_StoredConfigurationFillRegisterData(
        module, registerSet, registerOffset, data, position, mask, &adi_stateBaseTest));
    /* Test invalid state */
    TEST_ASSERT_FAIL_ASSERT(TEST_ADI_StoredConfigurationFillRegisterData(
        module, registerSet, registerOffset, data, position, mask, NULL_PTR));

    /* Test writing value in configuration register values stored by driver */
    /* First prepare data */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        adi_stateBaseTest.currentString = s;
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t offset = 0u; offset <= ADI_REGISTER_OFFSET5; offset++) {
                /* Set configuration values stored by driver to 0 */
                adi_configurationRegisterAgroup[s][(m * ADI_WRCFGA_LEN) + offset] = 0u;
                adi_configurationRegisterBgroup[s][(m * ADI_WRCFGB_LEN) + offset] = 0u;
            }
        }
    }
    /* Write bit positions 0b00011100 with value 5 */
    data     = 5u;
    position = 2u;
    mask     = 0x1Cu;
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        adi_stateBaseTest.currentString = s;
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t offset = 0u; offset <= ADI_REGISTER_OFFSET5; offset++) {
                /* Write for all modules, all offsets */
                TEST_ADI_StoredConfigurationFillRegisterData(
                    m, ADI_CFG_REGISTER_SET_A, offset, data, position, mask, &adi_stateBaseTest);
                TEST_ADI_StoredConfigurationFillRegisterData(
                    m, ADI_CFG_REGISTER_SET_B, offset, data, position, mask, &adi_stateBaseTest);
                /* Target bits must be changed, other unchanged */
                TEST_ASSERT_EQUAL(0x14u, adi_configurationRegisterAgroup[s][(m * ADI_WRCFGA_LEN) + offset]);
                TEST_ASSERT_EQUAL(0x14u, adi_configurationRegisterBgroup[s][(m * ADI_WRCFGB_LEN) + offset]);
            }
        }
    }
    /* Now write 0 only to module 0 at offset 1 */
    module         = 0u;
    registerOffset = 1u;
    data           = 0;
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        adi_stateBaseTest.currentString = s;
        /* Write data */
        TEST_ADI_StoredConfigurationFillRegisterData(
            module, ADI_CFG_REGISTER_SET_A, registerOffset, data, position, mask, &adi_stateBaseTest);
        TEST_ADI_StoredConfigurationFillRegisterData(
            module, ADI_CFG_REGISTER_SET_B, registerOffset, data, position, mask, &adi_stateBaseTest);
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t offset = 0u; offset <= ADI_REGISTER_OFFSET5; offset++) {
                if ((m == 0u) && (offset == 1u)) {
                    /* Value must be set to 0 for module 0 and offset 1 */
                    TEST_ASSERT_EQUAL(0u, adi_configurationRegisterAgroup[s][(m * ADI_WRCFGA_LEN) + offset]);
                    TEST_ASSERT_EQUAL(0u, adi_configurationRegisterBgroup[s][(m * ADI_WRCFGB_LEN) + offset]);
                } else {
                    /* Value must be unchanged for other modules and offsets */
                    TEST_ASSERT_EQUAL(0x14u, adi_configurationRegisterAgroup[s][(m * ADI_WRCFGA_LEN) + offset]);
                    TEST_ASSERT_EQUAL(0x14u, adi_configurationRegisterBgroup[s][(m * ADI_WRCFGB_LEN) + offset]);
                }
            }
        }
    }
}

void testADI_StoredConfigurationFillRegisterDataGlobal(void) {
    ADI_CFG_REGISTER_SET_e registerSet = ADI_CFG_REGISTER_SET_A;
    uint8_t registerOffset             = 0u;
    uint8_t data                       = 0u;
    uint8_t position                   = 0u;
    uint8_t mask                       = 1u;

    /* Test invalid register set */
    registerSet = ADI_CFG_REGISTER_SET_E_MAX;
    TEST_ASSERT_FAIL_ASSERT(ADI_StoredConfigurationFillRegisterDataGlobal(
        registerSet, registerOffset, data, position, mask, &adi_stateBaseTest));
    /* Test invalid register offset */
    registerOffset = ADI_REGISTER_OFFSET5 + 1u;
    TEST_ASSERT_FAIL_ASSERT(ADI_StoredConfigurationFillRegisterDataGlobal(
        registerSet, registerOffset, data, position, mask, &adi_stateBaseTest));
    /* Test invalid position */
    position = ADI_MAX_BIT_POSITION_IN_BYTE + 1u;
    TEST_ASSERT_FAIL_ASSERT(ADI_StoredConfigurationFillRegisterDataGlobal(
        registerSet, registerOffset, data, position, mask, &adi_stateBaseTest));
    /* Test invalid mask */
    mask = 0u;
    TEST_ASSERT_FAIL_ASSERT(ADI_StoredConfigurationFillRegisterDataGlobal(
        registerSet, registerOffset, data, position, mask, &adi_stateBaseTest));
    /* Test invalid state */
    TEST_ASSERT_FAIL_ASSERT(
        ADI_StoredConfigurationFillRegisterDataGlobal(registerSet, registerOffset, data, position, mask, NULL_PTR));

    /* Test writing value in configuration register values stored by driver */
    /* First prepare data */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        adi_stateBaseTest.currentString = s;
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t offset = 0u; offset <= ADI_REGISTER_OFFSET5; offset++) {
                /* Set configuration values stored by driver to 0 */
                adi_configurationRegisterAgroup[s][(m * ADI_WRCFGA_LEN) + offset] = 0u;
                adi_configurationRegisterBgroup[s][(m * ADI_WRCFGB_LEN) + offset] = 0u;
            }
        }
    }
    /* Write bit positions 0b00011100 with value 5 */
    data     = 5u;
    position = 2u;
    mask     = 0x1Cu;
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        adi_stateBaseTest.currentString = s;
        for (uint8_t offset = 0u; offset <= ADI_REGISTER_OFFSET5; offset++) {
            /* Write for all modules, all offsets */
            ADI_StoredConfigurationFillRegisterDataGlobal(
                ADI_CFG_REGISTER_SET_A, offset, data, position, mask, &adi_stateBaseTest);
            ADI_StoredConfigurationFillRegisterDataGlobal(
                ADI_CFG_REGISTER_SET_B, offset, data, position, mask, &adi_stateBaseTest);
            for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
                /* For all modules, target bits must be changed and other bits unchanged */
                TEST_ASSERT_EQUAL(0x14u, adi_configurationRegisterAgroup[s][(m * ADI_WRCFGA_LEN) + offset]);
                TEST_ASSERT_EQUAL(0x14u, adi_configurationRegisterBgroup[s][(m * ADI_WRCFGB_LEN) + offset]);
            }
        }
    }
}

void testADI_Wait(void) {
    /* invalid wait time test */
    TEST_ASSERT_FAIL_ASSERT(ADI_Wait(0u));

    /* dummy values */
    const uint32_t waitTime = 1u;
    uint32_t currentTime    = 2u;
    OS_GetTickCount_ExpectAndReturn(currentTime);
    OS_DelayTaskUntil_Expect(&currentTime, waitTime + 1);
    ADI_Wait(waitTime);
}

void testADI_WriteDataBits(void) {
    uint8_t pSentData             = 0u;
    uint8_t dummyData             = 0u;
    const uint8_t invalidPosition = ADI_MAX_BIT_POSITION_IN_BYTE + 1u;
    const uint8_t validPosition   = 0u;
    const uint8_t invalidMask     = 0u;
    const uint8_t validMask       = 1u;

    /* invalid 'pSentData' pointer (first argument, first assert) */
    TEST_ASSERT_FAIL_ASSERT(ADI_WriteDataBits(NULL_PTR, dummyData, validPosition, validMask));
    /* second argument 'dataToWrite' accepts the whole parameter range */
    /* invalid 'position' (third argument, SECOND assert) */
    TEST_ASSERT_FAIL_ASSERT(ADI_WriteDataBits(&pSentData, dummyData, invalidPosition, validMask));
    /* invalid 'mask' (fourth argument, THIRD assert) */
    TEST_ASSERT_FAIL_ASSERT(ADI_WriteDataBits(&pSentData, dummyData, validPosition, invalidMask));
}

void testADI_WriteRegister(void) {
    SPI_TransmitDummyByte_IgnoreAndReturn(STD_OK);
    SPI_TransmitReceiveDataDma_IgnoreAndReturn(STD_OK);
    OS_WaitForNotification_IgnoreAndReturn(OS_SUCCESS);

    /* Test command counter increase */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        adi_stateBaseTest.currentString = s;
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            /* Set value stored by driver */
            adi_stateBaseTest.data.commandCounter[s][m] = 13u;
        }
    }
    /* Call write configuration register group A function, increases command counter */
    ADI_CopyCommandBits(adi_cmdWrcfga, adi_command);
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        adi_stateBaseTest.currentString = s;
        /* No fault injection: command counter must increase */
        ADI_WriteRegister(adi_command, adi_dataTransmit, ADI_PEC_NO_FAULT_INJECTION, &adi_stateBaseTest);
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            /* Check that command counter was increased */
            TEST_ASSERT_EQUAL(14u, adi_stateBaseTest.data.commandCounter[s][m]);
        }
    }

    /* Test no command counter increase if command PEC fault injection */
    /* Call write configuration register group A function, increases command counter */
    ADI_CopyCommandBits(adi_cmdWrcfga, adi_command);
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        adi_stateBaseTest.currentString = s;
        ADI_WriteRegister(adi_command, adi_dataTransmit, ADI_COMMAND_PEC_FAULT_INJECTION, &adi_stateBaseTest);
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            /* Check that command counter was increased */
            TEST_ASSERT_EQUAL(14u, adi_stateBaseTest.data.commandCounter[s][m]);
        }
    }

    /* Test no command counter increase if data PEC fault injection */
    /* Call write configuration register group A function, increases command counter */
    ADI_CopyCommandBits(adi_cmdWrcfga, adi_command);
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        adi_stateBaseTest.currentString = s;
        ADI_WriteRegister(adi_command, adi_dataTransmit, ADI_DATA_PEC_FAULT_INJECTION, &adi_stateBaseTest);
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            /* Check that command counter was increased */
            TEST_ASSERT_EQUAL(14u, adi_stateBaseTest.data.commandCounter[s][m]);
        }
    }
}
