/**
 *
 * @copyright &copy; 2010 - 2024, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * @file    test_spi_nxp.c
 * @author  foxBMS Team
 * @date    2020-10-20 (date of creation)
 * @updated 2024-08-08 (date of last update)
 * @version v1.7.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Tests for the spi module
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "MockHL_spi.h"
#include "MockHL_sys_dma.h"
#include "Mockdma_cfg.h"
#include "Mockio.h"
#include "Mockmcu.h"
#include "Mockos.h"
#include "Mockspi_cfg.h"

#include "spi.h"
#include "spi_cfg-helper.h"
#include "test_assert_helper.h"

#include <stdbool.h>

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("spi.c")

TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/dma")
TEST_INCLUDE_PATH("../../src/app/driver/io")
TEST_INCLUDE_PATH("../../src/app/driver/spi")

/*========== Definitions and Implementations for Unit Test ==================*/

long FSYS_RaisePrivilege(void) {
    return 0;
}

/** SPI enumeration for DMA */
spiBASE_t *dma_spiInterfaces[DMA_NUMBER_SPI_INTERFACES] = {
    spiREG1, /*!< SPI1 */
    spiREG2, /*!< SPI2 */
    spiREG3, /*!< SPI3 */
    spiREG4, /*!< SPI4 */
    spiREG5, /*!< SPI5 */
};

/** DMA channel configuration for SPI communication */
DMA_CHANNEL_CONFIG_s dma_spiDmaChannels[DMA_NUMBER_SPI_INTERFACES] = {
    {DMA_CH0, DMA_CH1}, /*!< SPI1 */
    {DMA_CH2, DMA_CH3}, /*!< SPI2 */
    {DMA_CH4, DMA_CH5}, /*!< SPI3 */
    {DMA_CH6, DMA_CH7}, /*!< SPI4 */
    {DMA_CH8, DMA_CH9}, /*!< SPI5 */
};

/** SPI data configuration struct for NXP MC33775A communication, Tx part */
static spiDAT1_t spi_kNxp775DataConfigTx[BS_NR_OF_STRINGS] = {
    {.CS_HOLD = TRUE,      /* If true, HW chip select kept active */
     .WDEL    = TRUE,      /* Activation of delay between words */
     .DFSEL   = SPI_FMT_2, /* Data word format selection */
     /* Hardware chip select is configured automatically depending on configuration in #SPI_INTERFACE_CONFIG_s */
     .CSNR = SPI_HARDWARE_CHIP_SELECT_DISABLE_ALL},
};

/** SPI data configuration struct for NXP MC33775A communication, Rx part */
static spiDAT1_t spi_kNxp775DataConfigRx[BS_NR_OF_STRINGS] = {
    {.CS_HOLD = TRUE,      /* If true, HW chip select kept active */
     .WDEL    = TRUE,      /* Activation of delay between words */
     .DFSEL   = SPI_FMT_2, /* Data word format selection */
     /* Hardware chip select is configured automatically depending on configuration in #SPI_INTERFACE_CONFIG_s */
     .CSNR = SPI_HARDWARE_CHIP_SELECT_DISABLE_ALL},
};

/** SPI data configuration struct for FRAM communication */
static spiDAT1_t spi_kFramDataConfig = {
    /* struct is implemented in the TI HAL and uses uppercase true and false */
    .CS_HOLD = TRUE,      /* If true, HW chip select kept active */
    .WDEL    = TRUE,      /* Activation of delay between words */
    .DFSEL   = SPI_FMT_1, /* Data word format selection */
    /* Hardware chip select is configured automatically depending on configuration in #SPI_INTERFACE_CONFIG_s */
    .CSNR = SPI_HARDWARE_CHIP_SELECT_DISABLE_ALL,
};

/** SPI data configuration struct for SPS communication in low speed (4MHz) */
static spiDAT1_t spi_kSpsDataConfigLowSpeed = {
    /* struct is implemented in the TI HAL and uses uppercase true and false */
    .CS_HOLD = TRUE,      /* If true, HW chip select kept active */
    .WDEL    = TRUE,      /* Activation of delay between words */
    .DFSEL   = SPI_FMT_1, /* Data word format selection */
    /* Hardware chip select is configured automatically depending on configuration in #SPI_INTERFACE_CONFIG_s */
    .CSNR = SPI_HARDWARE_CHIP_SELECT_DISABLE_ALL,
};

/** SPI configuration struct for SBC communication */
static spiDAT1_t spi_kSbcDataConfig = {
    /* struct is implemented in the TI HAL and uses uppercase true and false */
    .CS_HOLD = TRUE,      /* If true, HW chip select kept active */
    .WDEL    = TRUE,      /* Activation of delay between words */
    .DFSEL   = SPI_FMT_0, /* Data word format selection */
    /* Hardware chip select is configured automatically depending on configuration in #SPI_INTERFACE_CONFIG_s */
    .CSNR = SPI_HARDWARE_CHIP_SELECT_DISABLE_ALL,
};

/** SPI interface configuration for N775 communication Tx part */
SPI_INTERFACE_CONFIG_s spi_nxp775InterfaceTx[BS_NR_OF_STRINGS] = {
    {
        .pConfig  = &spi_kNxp775DataConfigTx[0u],
        .pNode    = spiREG1,
        .pGioPort = &(spiREG1->PC3),
        .csPin    = 1u,
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

/** SPI interface configuration for FRAM communication */
SPI_INTERFACE_CONFIG_s spi_framInterface = {
    .pConfig  = &spi_kFramDataConfig,
    .pNode    = spiREG3,
    .pGioPort = &(spiREG3->PC3),
    .csPin    = 1u,
    .csType   = SPI_CHIP_SELECT_SOFTWARE,
};

/** SPI interface configuration for SPS communication */
SPI_INTERFACE_CONFIG_s spi_spsInterface = {
    .pConfig  = &spi_kSpsDataConfigLowSpeed,
    .pNode    = spiREG2,
    .pGioPort = &SPI_SPS_CS_GIOPORT,
    .csPin    = SPI_SPS_CS_PIN,
    .csType   = SPI_CHIP_SELECT_SOFTWARE,
};

/** SPI interface configuration for SBC communication */
SPI_INTERFACE_CONFIG_s spi_sbcMcuInterface = {
    .pConfig  = &spi_kSbcDataConfig,
    .pNode    = spiREG2,
    .pGioPort = &(spiREG2->PC3),
    .csPin    = 0u,
    .csType   = SPI_CHIP_SELECT_HARDWARE,
};

/** struct containing the lock state of the SPI interfaces */
SPI_BUSY_STATE_e spi_busyFlags[] = {
    SPI_IDLE,
    SPI_IDLE,
    SPI_IDLE,
    SPI_IDLE,
    SPI_IDLE,
};

const uint8_t spi_nrBusyFlags = sizeof(spi_busyFlags) / sizeof(SPI_BUSY_STATE_e);

/** mock for testing with an SPI handle */
spiBASE_t spiMockHandle = {0};

spi_config_reg_t spiMockConfigRegister = {0};

uint8_t spiTestArrayResultsNxpTx[BS_NR_OF_STRINGS] = {SPI_HARDWARE_CHIP_SELECT_1_ACTIVE};
uint8_t spiTestArrayResultsNxpRx[BS_NR_OF_STRINGS] = {SPI_HARDWARE_CHIP_SELECT_0_ACTIVE};

/*========== Setup and Teardown =============================================*/
void setUp(void) {
    /* make sure PC0 of config register is clean */
    spiMockConfigRegister.CONFIG_PC0 = 0;
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
/**
 * @brief   Testing static function #SPI_InitializeChipSelects
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - none (void function)
 *          - Routine validation:
 *            - RT1/1: software chip select pin in the valid range is verbatim
 *                     piped through the function
 */
void testSPI_InitializeChipSelects(void) {
    /* ======= Assertion tests ============================================= */
    /* none */

    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    const uint8_t currentString = 0u;

    /* ======= RT1/1: call function under test */
    TEST_SPI_InitializeChipSelects();

    /* ======= RT1/1: test output verification */
    TEST_ASSERT_EQUAL(SPI_HARDWARE_CHIP_SELECT_1_ACTIVE, spi_nxp775InterfaceTx[currentString].pConfig->CSNR);
    TEST_ASSERT_EQUAL(SPI_HARDWARE_CHIP_SELECT_0_ACTIVE, spi_nxp775InterfaceRx[currentString].pConfig->CSNR);
    TEST_ASSERT_EQUAL(1u, spi_framInterface.pConfig->CSNR);
    TEST_ASSERT_EQUAL(1u, spi_spsInterface.pConfig->CSNR);
    TEST_ASSERT_EQUAL(SPI_HARDWARE_CHIP_SELECT_0_ACTIVE, spi_sbcMcuInterface.pConfig->CSNR);
}

/**
 * @brief   Testing static function #SPI_InitializeChipSelectsAfe
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: invalid string -> assert
 *          - Routine validation:
 *            - RT1/1: configuration register is properly set
 */
void testSPI_InitializeChipSelectsAfeNxp(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_SPI_InitializeChipSelectsAfe(BS_NR_OF_STRINGS));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    /* ======= RT1/1: call function under test */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        TEST_SPI_InitializeChipSelectsAfe(s);
    }
    /* ======= RT1/1: test output verification */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        TEST_ASSERT_EQUAL(spiTestArrayResultsNxpTx[s], spi_nxp775InterfaceTx[s].pConfig->CSNR);
        TEST_ASSERT_EQUAL(spiTestArrayResultsNxpRx[s], spi_nxp775InterfaceRx[s].pConfig->CSNR);
    }
}
