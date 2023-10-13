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
 * @file    test_adi_ades1830_gpio_voltages.c
 * @author  foxBMS Team
 * @date    2022-12-08 (date of creation)
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
#include "Mockadi_ades183x_cfg.h"
#include "Mockos.h"
#include "Mockspi.h"

#include "adi_ades1830_defs.h"
#include "adi_ades183x_buffers.h"  /* use the real command config */
#include "adi_ades183x_commands.h" /* use the real buffer configuration */
#include "adi_ades183x_commands_voltages.h"
#include "adi_ades183x_defs.h"
#include "adi_ades183x_gpio_voltages.h"
#include "adi_ades183x_helpers.h"
#include "adi_ades183x_pec.h"
#include "spi_cfg-helper.h"
#include "test_assert_helper.h"

#include <stdbool.h>
#include <stdint.h>

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("adi_ades1830_gpio_voltages.c")
TEST_SOURCE_FILE("adi_ades183x_buffers.c")
TEST_SOURCE_FILE("adi_ades183x_voltages.c")

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

/*========== Definitions and Implementations for Unit Test ==================*/

static DATA_BLOCK_ALL_GPIO_VOLTAGES_s adi_allGpioVoltage = {.header.uniqueId = DATA_BLOCK_ID_ALL_GPIO_VOLTAGES_BASE};
static DATA_BLOCK_ALL_GPIO_VOLTAGES_s adi_allGpioVoltageRedundant = {
    .header.uniqueId = DATA_BLOCK_ID_ALL_GPIO_VOLTAGES_BASE};
static DATA_BLOCK_ALL_GPIO_VOLTAGES_s adi_allGpioVoltageOpenWire = {
    .header.uniqueId = DATA_BLOCK_ID_ALL_GPIO_VOLTAGES_BASE};
static ADI_ERROR_TABLE_s adi_errorTable = {0}; /*!< init in ADI_ResetErrorTable-function */
static uint16_t adi_bufferRxPecTest[ADI_N_BYTES_FOR_DATA_TRANSMISSION] = {0};
static uint16_t adi_bufferTxPecTest[ADI_N_BYTES_FOR_DATA_TRANSMISSION] = {0};

ADI_STATE_s adi_stateBase = {
    .data.txBuffer                 = adi_bufferTxPecTest,
    .data.rxBuffer                 = adi_bufferRxPecTest,
    .data.allGpioVoltages          = &adi_allGpioVoltage,
    .data.allGpioVoltagesRedundant = &adi_allGpioVoltageRedundant,
    .data.allGpioVoltageOpenWire   = &adi_allGpioVoltageOpenWire,
    .data.errorTable               = &adi_errorTable,
};

/* If this configuration is changed, testADI_GetStoredVoltageIndex() must be adapted */
const uint8_t adi_voltageInputsUsed[ADI_MAX_SUPPORTED_CELLS] = {
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
};

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

#define TEST_ADI_AUXILIARY_RESULT_REGISTER_SET_INVALID_VALUE ((ADI_AUXILIARY_RESULT_REGISTER_SET_E) + 1u)

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
void testADI_GetGpioVoltages(void) {
    /* Invalid state */
    TEST_ASSERT_FAIL_ASSERT(ADI_GetGpioVoltages(NULL_PTR, ADI_AUXILIARY_REGISTER, ADI_AUXILIARY_VOLTAGE));
    /* Invalid register type */
    TEST_ASSERT_FAIL_ASSERT(
        ADI_GetGpioVoltages(&adi_stateBase, ADI_AUXILIARY_REGISTER_TYPE_E_MAX, ADI_AUXILIARY_VOLTAGE));
    /* Invalid store location */
    TEST_ASSERT_FAIL_ASSERT(
        ADI_GetGpioVoltages(&adi_stateBase, ADI_AUXILIARY_REGISTER, ADI_AUXILIARY_STORE_LOCATION_E_MAX));

    /* Test reading of GPIO voltages */

    /* SPI and OS functions are ignored */
    SPI_TransmitDummyByte_IgnoreAndReturn(STD_OK);
    SPI_TransmitReceiveDataDma_IgnoreAndReturn(STD_OK);
    OS_WaitForNotification_IgnoreAndReturn(OS_SUCCESS);
    DATA_BLOCK_ALL_GPIO_VOLTAGES_s *pGpioVoltageTable = NULL_PTR;

    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        adi_stateBase.currentString = s;

        /* Prepare voltage data to simulate data read from AFE registers */
        for (uint16_t i = 4u; i < ADI_N_BYTES_FOR_DATA_TRANSMISSION; i += 8) {
            /* Set raw data, corresponds to a cell voltage of 1884mV */
            uint8_t bufferMSB = 0x0Au;
            uint8_t bufferLSB = 0x01u;
            /* Set receive buffer with raw data */
            adi_bufferRxPecTest[i]      = bufferLSB;
            adi_bufferRxPecTest[i + 1u] = bufferMSB;
            adi_bufferRxPecTest[i + 2u] = bufferLSB;
            adi_bufferRxPecTest[i + 3u] = bufferMSB;
            adi_bufferRxPecTest[i + 4u] = bufferLSB;
            adi_bufferRxPecTest[i + 5u] = bufferMSB;
            /* Two PEC bytes */
            adi_bufferRxPecTest[i + 6u] = 0x00u;
            adi_bufferRxPecTest[i + 7u] = 0x2Bu;
        }

        for (ADI_AUXILIARY_STORE_LOCATION_e storeLocation = ADI_AUXILIARY_VOLTAGE;
             storeLocation < ADI_AUXILIARY_STORE_LOCATION_E_MAX;
             storeLocation++) {
            for (ADI_AUXILIARY_REGISTER_TYPE_e registerType = ADI_AUXILIARY_REGISTER;
                 registerType < ADI_AUXILIARY_REGISTER_TYPE_E_MAX;
                 registerType++) {
                /* Set store buffer with store location */
                switch (storeLocation) {
                    case ADI_AUXILIARY_VOLTAGE:
                        pGpioVoltageTable = adi_stateBase.data.allGpioVoltages;
                        break;
                    case ADI_REDUNDANT_AUXILIARY_VOLTAGE:
                        pGpioVoltageTable = adi_stateBase.data.allGpioVoltagesRedundant;
                        break;
                    case ADI_AUXILIARY_VOLTAGE_OPEN_WIRE:
                        pGpioVoltageTable = adi_stateBase.data.allGpioVoltageOpenWire;
                        break;
                    default:
                        break;
                }
                /* Reset cell voltage values */
                for (uint16_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
                    for (uint8_t c = 0u; c < BS_NR_OF_GPIOS_PER_MODULE; c++) {
                        pGpioVoltageTable->gpioVoltages_mV[s][c + (m * BS_NR_OF_GPIOS_PER_MODULE)] = 0;
                    }
                }
                /* Reset PEC error flags */
                for (uint16_t m = 0; m < BS_NR_OF_MODULES_PER_STRING; m++) {
                    adi_stateBase.data.errorTable->crcIsOk[s][m] = true;
                }

                /* Now get voltages by reading data (mocked) and storing it to the voltage table */
                ADI_GetGpioVoltages(&adi_stateBase, registerType, storeLocation);
                for (uint16_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
                    for (uint8_t c = 0u; c < BS_NR_OF_GPIOS_PER_MODULE; c++) {
                        /* Everything OK, the values must be stored */
                        TEST_ASSERT_EQUAL(
                            1884, pGpioVoltageTable->gpioVoltages_mV[s][c + (m * BS_NR_OF_GPIOS_PER_MODULE)]);
                    }
                }
            }
        }
    }
}

/* externalized static functions tests */
void testADI_SaveRxToGpioVoltageBuffer(void) {
    /* Test invalid state */
    TEST_ASSERT_FAIL_ASSERT(TEST_ADI_SaveRxToGpioVoltageBuffer(
        NULL_PTR, adi_dataReceive, ADI_RESULT_REGISTER_SET_A, ADI_AUXILIARY_VOLTAGE));
    /* Test invalid data */
    TEST_ASSERT_FAIL_ASSERT(
        TEST_ADI_SaveRxToGpioVoltageBuffer(&adi_stateBase, NULL_PTR, ADI_RESULT_REGISTER_SET_A, ADI_AUXILIARY_VOLTAGE));
    /* Test invalid register set */
    TEST_ASSERT_FAIL_ASSERT(TEST_ADI_SaveRxToGpioVoltageBuffer(
        &adi_stateBase, adi_dataReceive, (ADI_RESULT_REGISTER_SET_E + 1u), ADI_AUXILIARY_VOLTAGE));
    /* Test invalid store location */
    TEST_ASSERT_FAIL_ASSERT(TEST_ADI_SaveRxToGpioVoltageBuffer(
        &adi_stateBase, adi_dataReceive, ADI_RESULT_REGISTER_SET_A, ADI_AUXILIARY_STORE_LOCATION_E_MAX));

    DATA_BLOCK_ALL_GPIO_VOLTAGES_s *pGpioVoltageTable = NULL_PTR;

    /* Test three cases for each store location:
       -raw buffer contains cleared values, values must not be stored
       -PEC error set, values must not be stored
       -everything OK, values must be stored */
    for (ADI_AUXILIARY_STORE_LOCATION_e storeLocation = ADI_AUXILIARY_VOLTAGE;
         storeLocation < ADI_AUXILIARY_STORE_LOCATION_E_MAX;
         storeLocation++) {
        /* Set store buffer with store location */
        switch (storeLocation) {
            case ADI_AUXILIARY_VOLTAGE:
                pGpioVoltageTable = adi_stateBase.data.allGpioVoltages;
                break;
            case ADI_REDUNDANT_AUXILIARY_VOLTAGE:
                pGpioVoltageTable = adi_stateBase.data.allGpioVoltagesRedundant;
                break;
            case ADI_AUXILIARY_VOLTAGE_OPEN_WIRE:
                pGpioVoltageTable = adi_stateBase.data.allGpioVoltageOpenWire;
                break;
            default:
                break;
        }
        for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
            adi_stateBase.currentString = s;
            /* First test: buffer contains cleared values */
            /* Prepare voltage data */
            for (uint16_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
                /* Parse all voltages contained in one register */
                for (uint16_t c = 0u; c < ADI_MAX_NUMBER_OF_GPIO_VOLTAGES_IN_REGISTER; c++) {
                    /* Set raw data, corresponds to cleared values */
                    uint8_t bufferMSB = (ADI_REGISTER_CLEARED_VALUE & 0xFF00u) >> 8u;
                    uint8_t bufferLSB = ADI_REGISTER_CLEARED_VALUE & 0xFFu;
                    /* Set receive buffer with raw data */
                    adi_dataReceive[(ADI_RAW_VOLTAGE_SIZE_IN_BYTES * c) + (m * ADI_MAX_REGISTER_SIZE_IN_BYTES) + 1u] =
                        bufferMSB;
                    adi_dataReceive[(ADI_RAW_VOLTAGE_SIZE_IN_BYTES * c) + (m * ADI_MAX_REGISTER_SIZE_IN_BYTES)] =
                        bufferLSB;
                }
            }
            /* Reset cell voltage values */
            for (uint16_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
                for (uint8_t c = 0u; c < BS_NR_OF_GPIOS_PER_MODULE; c++) {
                    pGpioVoltageTable->gpioVoltages_mV[s][c + (m * BS_NR_OF_GPIOS_PER_MODULE)] = 0;
                }
            }
            /* Reset PEC error flags */
            for (uint16_t m = 0; m < BS_NR_OF_MODULES_PER_STRING; m++) {
                adi_stateBase.data.errorTable->crcIsOk[s][m] = true;
            }
            /* Save buffer to store location */
            TEST_ADI_SaveRxToGpioVoltageBuffer(
                &adi_stateBase, adi_dataReceive, ADI_RESULT_REGISTER_SET_A, storeLocation);
            TEST_ADI_SaveRxToGpioVoltageBuffer(
                &adi_stateBase, adi_dataReceive, ADI_RESULT_REGISTER_SET_B, storeLocation);
            TEST_ADI_SaveRxToGpioVoltageBuffer(
                &adi_stateBase, adi_dataReceive, ADI_RESULT_REGISTER_SET_C, storeLocation);
            TEST_ADI_SaveRxToGpioVoltageBuffer(
                &adi_stateBase, adi_dataReceive, ADI_RESULT_REGISTER_SET_D, storeLocation);
            for (uint16_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
                for (uint8_t c = 0u; c < BS_NR_OF_GPIOS_PER_MODULE; c++) {
                    /* As cleared value are stored in buffer, they must not be stored in the voltage table */
                    TEST_ASSERT_EQUAL(0, pGpioVoltageTable->gpioVoltages_mV[s][c + (m * BS_NR_OF_GPIOS_PER_MODULE)]);
                }
            }
            /* Second test: wrong PEC */
            /* Prepare voltage data */
            for (uint16_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
                /* Parse all voltages contained in one register */
                for (uint16_t c = 0u; c < ADI_MAX_NUMBER_OF_GPIO_VOLTAGES_IN_REGISTER; c++) {
                    /* Set raw data, corresponds to a cell voltage of 1884mV */
                    uint8_t bufferMSB = 0x0Au;
                    uint8_t bufferLSB = 0x01u;
                    /* Set receive buffer with raw data */
                    adi_dataReceive[(ADI_RAW_VOLTAGE_SIZE_IN_BYTES * c) + (m * ADI_MAX_REGISTER_SIZE_IN_BYTES) + 1u] =
                        bufferMSB;
                    adi_dataReceive[(ADI_RAW_VOLTAGE_SIZE_IN_BYTES * c) + (m * ADI_MAX_REGISTER_SIZE_IN_BYTES)] =
                        bufferLSB;
                }
            }
            /* Reset cell voltage values */
            for (uint16_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
                for (uint8_t c = 0u; c < BS_NR_OF_GPIOS_PER_MODULE; c++) {
                    pGpioVoltageTable->gpioVoltages_mV[s][c + (m * BS_NR_OF_GPIOS_PER_MODULE)] = 0;
                }
            }
            /* Set PEC error flags */
            for (uint16_t m = 0; m < BS_NR_OF_MODULES_PER_STRING; m++) {
                adi_stateBase.data.errorTable->crcIsOk[s][m] = false;
            }
            /* Save buffer to store location */
            TEST_ADI_SaveRxToGpioVoltageBuffer(
                &adi_stateBase, adi_dataReceive, ADI_RESULT_REGISTER_SET_A, storeLocation);
            TEST_ADI_SaveRxToGpioVoltageBuffer(
                &adi_stateBase, adi_dataReceive, ADI_RESULT_REGISTER_SET_B, storeLocation);
            TEST_ADI_SaveRxToGpioVoltageBuffer(
                &adi_stateBase, adi_dataReceive, ADI_RESULT_REGISTER_SET_C, storeLocation);
            TEST_ADI_SaveRxToGpioVoltageBuffer(
                &adi_stateBase, adi_dataReceive, ADI_RESULT_REGISTER_SET_D, storeLocation);
            for (uint16_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
                for (uint8_t c = 0u; c < BS_NR_OF_GPIOS_PER_MODULE; c++) {
                    /* As PEC error is detected, the values must not be stored */
                    TEST_ASSERT_EQUAL(0, pGpioVoltageTable->gpioVoltages_mV[s][c + (m * BS_NR_OF_GPIOS_PER_MODULE)]);
                }
            }
            /* Third test: store values */
            /* Prepare voltage data */
            for (uint16_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
                /* Parse all voltages contained in one register */
                for (uint16_t c = 0u; c < ADI_MAX_NUMBER_OF_GPIO_VOLTAGES_IN_REGISTER; c++) {
                    /* Set raw data, corresponds to a cell voltage of 1884mV */
                    uint8_t bufferMSB = 0x0Au;
                    uint8_t bufferLSB = 0x01u;
                    /* Set receive buffer with raw data */
                    adi_dataReceive
                        [ADI_COMMAND_AND_PEC_SIZE_IN_BYTES + (ADI_RAW_VOLTAGE_SIZE_IN_BYTES * c) +
                         (m * (ADI_MAX_REGISTER_SIZE_IN_BYTES + ADI_PEC_SIZE_IN_BYTES)) + 1u] = bufferMSB;
                    adi_dataReceive
                        [ADI_COMMAND_AND_PEC_SIZE_IN_BYTES + (ADI_RAW_VOLTAGE_SIZE_IN_BYTES * c) +
                         (m * (ADI_MAX_REGISTER_SIZE_IN_BYTES + ADI_PEC_SIZE_IN_BYTES))] = bufferLSB;
                }
            }
            /* Reset cell voltage values */
            for (uint16_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
                for (uint8_t c = 0u; c < BS_NR_OF_GPIOS_PER_MODULE; c++) {
                    pGpioVoltageTable->gpioVoltages_mV[s][c + (m * BS_NR_OF_GPIOS_PER_MODULE)] = 0;
                }
            }
            /* Reset PEC error flags */
            for (uint16_t m = 0; m < BS_NR_OF_MODULES_PER_STRING; m++) {
                adi_stateBase.data.errorTable->crcIsOk[s][m] = true;
            }
            /* Save buffer to store location */
            TEST_ADI_SaveRxToGpioVoltageBuffer(
                &adi_stateBase, adi_dataReceive, ADI_RESULT_REGISTER_SET_A, storeLocation);
            TEST_ADI_SaveRxToGpioVoltageBuffer(
                &adi_stateBase, adi_dataReceive, ADI_RESULT_REGISTER_SET_B, storeLocation);
            TEST_ADI_SaveRxToGpioVoltageBuffer(
                &adi_stateBase, adi_dataReceive, ADI_RESULT_REGISTER_SET_C, storeLocation);
            TEST_ADI_SaveRxToGpioVoltageBuffer(
                &adi_stateBase, adi_dataReceive, ADI_RESULT_REGISTER_SET_D, storeLocation);
            for (uint16_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
                for (uint8_t c = 0u; c < BS_NR_OF_GPIOS_PER_MODULE; c++) {
                    /* Everything OK, the values must be stored */
                    TEST_ASSERT_EQUAL(1884, pGpioVoltageTable->gpioVoltages_mV[s][c + (m * BS_NR_OF_GPIOS_PER_MODULE)]);
                }
            }
        }
    }
}
