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
 * @updated 2025-03-31 (date of last update)
 * @version v1.9.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Test of nxp_mc33775a_i2c.c
 * @details TODO
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "MockHL_gio.h"
#include "MockHL_system.h"
#include "Mockafe_dma.h"
#include "Mockdatabase.h"
#include "Mockdiag.h"
#include "Mockftask.h"
#include "Mockio.h"
#include "Mockmcu.h"
#include "Mocknxp_mc33775a-ll.h"
#include "Mocknxp_mc33775a_cfg.h"
#include "Mockos.h"
#include "Mockspi.h"

#include "MC33775A.h"
#include "foxmath.h"
#include "nxp_mc33775a.h"
#include "nxp_mc33775a_balancing.h"
#include "nxp_mc33775a_database.h"
#include "nxp_mc33775a_i2c.h"
#include "nxp_mc33775a_mux.h"
#include "spi_cfg-helper.h"
#include "uc_msg_t.h"

/* clang-format off */
#include "test_assert_helper.h"
/* clang-format on */

#include <stdbool.h>
#include <stdint.h>

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("nxp_mc33775a_i2c.c")

TEST_INCLUDE_PATH("../../src/app/driver/afe/api")
TEST_INCLUDE_PATH("../../src/app/driver/afe/nxp/mc33775a")
TEST_INCLUDE_PATH("../../src/app/driver/afe/nxp/mc33775a/config")
TEST_INCLUDE_PATH("../../src/app/driver/afe/nxp/mc33775a/vendor")
TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/dma")
TEST_INCLUDE_PATH("../../src/app/driver/foxmath")
TEST_INCLUDE_PATH("../../src/app/driver/io")
TEST_INCLUDE_PATH("../../src/app/driver/rtc")
TEST_INCLUDE_PATH("../../src/app/driver/spi")
TEST_INCLUDE_PATH("../../src/app/engine/diag")
TEST_INCLUDE_PATH("../../src/app/task/config")
TEST_INCLUDE_PATH("../../src/app/task/ftask")

/*========== Definitions and Implementations for Unit Test ==================*/

/** SPI data configuration struct for NXP MC33775A communication, Tx part */
static spiDAT1_t spi_kNxp775DataConfigTx[BS_NR_OF_STRINGS] = {
    {.CS_HOLD = TRUE,      /* If true, HW chip select kept active */
     .WDEL    = TRUE,      /* Activation of delay between words */
     .DFSEL   = SPI_FMT_0, /* Data word format selection */
     .CSNR    = SPI_HARDWARE_CHIP_SELECT_2_ACTIVE},
};

/** SPI data configuration struct for NXP MC33775A communication, Rx part */
static spiDAT1_t spi_kNxp775DataConfigRx[BS_NR_OF_STRINGS] = {
    {.CS_HOLD = TRUE,      /* If true, HW chip select kept active */
     .WDEL    = TRUE,      /* Activation of delay between words */
     .DFSEL   = SPI_FMT_0, /* Data word format selection */
     .CSNR    = SPI_HARDWARE_CHIP_SELECT_0_ACTIVE},
};

/** SPI interface configuration for N775 communication Tx part */
SPI_INTERFACE_CONFIG_s spi_nxp775InterfaceTx[BS_NR_OF_STRINGS] = {
    {
        .pConfig  = &spi_kNxp775DataConfigTx[0u],
        .pNode    = spiREG1,
        .pGioPort = &(spiREG1->PC3),
        .csPin    = 2u,
        .csType   = SPI_CHIP_SELECT_HARDWARE,
    },
};

/** SPI interface configuration for N775 communication, Rx part */
SPI_INTERFACE_CONFIG_s spi_nxp775InterfaceRx[BS_NR_OF_STRINGS] = {
    {
        .pConfig  = &spi_kNxp775DataConfigRx[0u],
        .pNode    = spiREG4,
        .pGioPort = &(spiREG4->PC3),
        .csPin    = 0u,
        .csType   = SPI_CHIP_SELECT_HARDWARE,
    },
};

/**
 * Default multiplexer measurement sequence
 * Must be adapted to the application
 */
N775_MUX_CH_CFG_s n775_muxSequence[N775_MUX_SEQUENCE_LENGTH] = {
    /*  multiplexer 0 measurement */
    {
        .muxId      = 0,
        .muxChannel = 0,
    },
    {
        .muxId      = 0,
        .muxChannel = 1,
    },
    {
        .muxId      = 0,
        .muxChannel = 2,
    },
    {
        .muxId      = 0,
        .muxChannel = 3,
    },
    {
        .muxId      = 0,
        .muxChannel = 4,
    },
    {
        .muxId      = 0,
        .muxChannel = 5,
    },
    {
        .muxId      = 0,
        .muxChannel = 6,
    },
    {
        .muxId      = 0,
        .muxChannel = 7,
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

void testN775_InitializeI2c(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_N775_InitializeI2c(NULL_PTR));
}

void testN775_TransmitI2c(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_N775_TransmitI2c(NULL_PTR));
}

/**
 * @brief   Testing extern function #N775_I2cRead
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/2: NULL_PTR for pData -> &rarr; assert
 *            - AT2/2: invalid data length -> &rarr; assert
 *          - Routine validation:
 *            - TODO
 */
void testN775_I2cRead(void) {
    /* ======= Assertion tests ============================================= */
    uint8_t data                            = 0u;
    const uint8_t validModuleNumber         = 0u;
    const uint8_t validDeviceAddress        = 0u;
    const uint8_t validDataLength           = 1u;
    const uint8_t invalidDataLengthTooSmall = 0u;
    const uint8_t invalidDataLengthTooLarge = 14u;
    /* ======= AT1/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(N775_I2cRead(validModuleNumber, validDeviceAddress, NULL_PTR, validDataLength));
    /* ======= AT1/2:1 ===== */
    TEST_ASSERT_FAIL_ASSERT(N775_I2cRead(validModuleNumber, validDeviceAddress, &data, invalidDataLengthTooSmall));
    /* ======= AT1/2:2 ===== */
    TEST_ASSERT_FAIL_ASSERT(N775_I2cRead(validModuleNumber, validDeviceAddress, &data, invalidDataLengthTooLarge));
    /* ======= Routine tests =============================================== */
}

void testN775_I2cWrite(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_N775_TransmitI2c(NULL_PTR));
}

/**
 * @brief   Testing extern function #N775_I2cRead
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
void testN775_I2cWriteRead(void) {
    /* ======= Assertion tests ============================================= */
    uint8_t writeData = 0u;
    uint8_t readData  = 0u;

    const uint8_t validModuleNumber  = 0u;
    const uint8_t validDeviceAddress = 0u;

    /* ======= AT1/5 ======= */
    TEST_ASSERT_FAIL_ASSERT(N775_I2cWriteRead(validModuleNumber, validDeviceAddress, NULL_PTR, 1u, &readData, 1u));
    /* ======= AT2/5 ======= */
    TEST_ASSERT_FAIL_ASSERT(N775_I2cWriteRead(validModuleNumber, validDeviceAddress, &writeData, 1u, NULL_PTR, 1u));
    /* ======= AT3/5 ======= */
    TEST_ASSERT_FAIL_ASSERT(N775_I2cWriteRead(validModuleNumber, validDeviceAddress, &writeData, 0u, &readData, 1u));
    /* ======= AT4/5 ======= */
    TEST_ASSERT_FAIL_ASSERT(N775_I2cWriteRead(validModuleNumber, validDeviceAddress, &writeData, 1u, &readData, 0u));
    /* ======= AT5/5 ======= */
    TEST_ASSERT_FAIL_ASSERT(N775_I2cWriteRead(validModuleNumber, validDeviceAddress, &writeData, 6u, &readData, 7u));

    /* ======= Routine tests =============================================== */
}
