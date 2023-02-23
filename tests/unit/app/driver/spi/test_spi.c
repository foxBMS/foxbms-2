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
 * @file    test_spi.c
 * @author  foxBMS Team
 * @date    2020-04-01 (date of creation)
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
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

TEST_FILE("spi.c")

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

/** SPI data configuration struct for LTC communication */
static spiDAT1_t spi_kLtcDataConfig[BS_NR_OF_STRINGS] = {
    {                      /* struct is implemented in the TI HAL and uses uppercase true and false */
     .CS_HOLD = TRUE,      /* If true, HW chip select kept active between words */
     .WDEL    = FALSE,     /* Activation of delay between words */
     .DFSEL   = SPI_FMT_0, /* Data word format selection */
     /* Hardware chip select is configured automatically depending on configuration in #SPI_INTERFACE_CONFIG_s */
     .CSNR = SPI_HARDWARE_CHIP_SELECT_DISABLE_ALL},
};

/** SPI data configuration struct for MXM communication */
static spiDAT1_t spi_kMxmDataConfig = {
    .CS_HOLD = TRUE,      /* If true, HW chip select kept active */
    .WDEL    = TRUE,      /* Activation of delay between words */
    .DFSEL   = SPI_FMT_1, /* Data word format selection */
    /* Hardware chip select is configured automatically depending on configuration in #SPI_INTERFACE_CONFIG_s */
    .CSNR = SPI_HARDWARE_CHIP_SELECT_DISABLE_ALL,
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

/** SPI data configuration struct for SPS communication in high speed (10MHz) */
static spiDAT1_t spi_kSpsDataConfigHighSpeed = {
    /* struct is implemented in the TI HAL and uses uppercase true and false */
    .CS_HOLD = TRUE,      /* If true, HW chip select kept active */
    .WDEL    = TRUE,      /* Activation of delay between words */
    .DFSEL   = SPI_FMT_2, /* Data word format selection */
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

/**
 * SPI interface configuration for LTC communication
 * This is a list of structs because of multi string
 */
SPI_INTERFACE_CONFIG_s spi_ltcInterface[BS_NR_OF_STRINGS] = {
    {
        .pConfig  = &spi_kLtcDataConfig[0u],
        .pNode    = spiREG1,
        .pGioPort = &(spiREG1->PC3),
        .csPin    = 1u,
        .csType   = SPI_CHIP_SELECT_HARDWARE,
    },
};

/** SPI interface configuration for MXM communication */
SPI_INTERFACE_CONFIG_s spi_mxmInterface = {
    .pConfig  = &spi_kMxmDataConfig,
    .pNode    = spiREG4,
    .pGioPort = &(spiREG4->PC3),
    .csPin    = 0u,
    .csType   = SPI_CHIP_SELECT_HARDWARE,
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
    TEST_ASSERT_EQUAL(SPI_HARDWARE_CHIP_SELECT_1_ACTIVE, spi_ltcInterface[currentString].pConfig->CSNR);
    TEST_ASSERT_EQUAL(SPI_HARDWARE_CHIP_SELECT_1_ACTIVE, spi_nxp775InterfaceTx[currentString].pConfig->CSNR);
    TEST_ASSERT_EQUAL(SPI_HARDWARE_CHIP_SELECT_0_ACTIVE, spi_nxp775InterfaceRx[currentString].pConfig->CSNR);
    TEST_ASSERT_EQUAL(SPI_HARDWARE_CHIP_SELECT_0_ACTIVE, spi_mxmInterface.pConfig->CSNR);
    TEST_ASSERT_EQUAL(1u, spi_framInterface.pConfig->CSNR);
    TEST_ASSERT_EQUAL(1u, spi_spsInterface.pConfig->CSNR);
    TEST_ASSERT_EQUAL(SPI_HARDWARE_CHIP_SELECT_0_ACTIVE, spi_sbcMcuInterface.pConfig->CSNR);
}

/**
 * @brief   Testing static function #SPI_GetChipSelectPin
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/2: invalid chip select type -> assert
 *            - AT2/2: invalid chip select pin number -> assert
 *          - Routine validation:
 *            - RT1/2: software chip select pin in the valid range is verbatim
 *                     piped through the function
 *            - RT1/2: hardware chip select pin in the valid range is mapped to
 *                     the correct value
 */
void testSPI_GetChipSelectPin(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/2 ======= */
    const SPI_CHIP_SELECT_TYPE_e invalidChipSelectType = SPI_CHIP_SELECT_MAX;
    const uint32_t validCsPin                          = 0u;
    TEST_ASSERT_FAIL_ASSERT(TEST_SPI_GetChipSelectPin(invalidChipSelectType, validCsPin));
    /* ======= AT2/2 ======= */
    const SPI_CHIP_SELECT_TYPE_e validChipSelectType = SPI_CHIP_SELECT_SOFTWARE;
    const uint32_t invalidCsPin                      = MCU_LARGEST_PIN_NUMBER + 1u;
    TEST_ASSERT_FAIL_ASSERT(TEST_SPI_GetChipSelectPin(validChipSelectType, invalidCsPin));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/2: Test implementation */
    const uint32_t validSoftwareChipSelectPin = MCU_LARGEST_PIN_NUMBER / 2u;
    /* ======= RT1/2: call function under test */
    const uint8_t softwareChipSelectPin =
        TEST_SPI_GetChipSelectPin(SPI_CHIP_SELECT_SOFTWARE, validSoftwareChipSelectPin);
    /* ======= RT1/2: test output verification */
    TEST_ASSERT_EQUAL(validSoftwareChipSelectPin, softwareChipSelectPin);

    /* ======= RT2/2: Test implementation */
    const uint32_t validHardwareChipSelectPin = 2u;
    /* ======= RT2/2: call function under test */
    const uint8_t hardwareChipSelectPin =
        TEST_SPI_GetChipSelectPin(SPI_CHIP_SELECT_HARDWARE, validHardwareChipSelectPin);
    /* ======= RT2/2: test output verification */
    TEST_ASSERT_EQUAL(SPI_HARDWARE_CHIP_SELECT_2_ACTIVE, hardwareChipSelectPin);
}

/**
 * @brief   Testing static function #SPI_GetHardwareChipSelectPin
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: invalid pin number -> assert
 *          - Routine validation:
 *            - RT1/1: check that only valid hardware chip select
 *                     configurations are returned
 */
void testSPI_GetHardwareChipSelectPin(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    const uint32_t invalidHwCsPin = 7u;
    TEST_ASSERT_FAIL_ASSERT(TEST_SPI_GetHardwareChipSelectPin(invalidHwCsPin));

    /* ======= Routine tests =============================================== */
    const uint8_t csPin0 = 0u;
    const uint8_t csPin1 = 1u;
    const uint8_t csPin2 = 2u;
    const uint8_t csPin3 = 3u;
    const uint8_t csPin4 = 4u;
    const uint8_t csPin5 = 5u;
    /* ======= RT1/1: Test implementation */
    /* ======= RT1/1: call function under test */
    const uint8_t mappedHardwareChipSelectPin0 = TEST_SPI_GetHardwareChipSelectPin(csPin0);
    const uint8_t mappedHardwareChipSelectPin1 = TEST_SPI_GetHardwareChipSelectPin(csPin1);
    const uint8_t mappedHardwareChipSelectPin2 = TEST_SPI_GetHardwareChipSelectPin(csPin2);
    const uint8_t mappedHardwareChipSelectPin3 = TEST_SPI_GetHardwareChipSelectPin(csPin3);
    const uint8_t mappedHardwareChipSelectPin4 = TEST_SPI_GetHardwareChipSelectPin(csPin4);
    const uint8_t mappedHardwareChipSelectPin5 = TEST_SPI_GetHardwareChipSelectPin(csPin5);
    /* ======= RT1/1: test output verification */
    TEST_ASSERT_EQUAL(SPI_HARDWARE_CHIP_SELECT_0_ACTIVE, mappedHardwareChipSelectPin0);
    TEST_ASSERT_EQUAL(SPI_HARDWARE_CHIP_SELECT_1_ACTIVE, mappedHardwareChipSelectPin1);
    TEST_ASSERT_EQUAL(SPI_HARDWARE_CHIP_SELECT_2_ACTIVE, mappedHardwareChipSelectPin2);
    TEST_ASSERT_EQUAL(SPI_HARDWARE_CHIP_SELECT_3_ACTIVE, mappedHardwareChipSelectPin3);
    TEST_ASSERT_EQUAL(SPI_HARDWARE_CHIP_SELECT_4_ACTIVE, mappedHardwareChipSelectPin4);
    TEST_ASSERT_EQUAL(SPI_HARDWARE_CHIP_SELECT_5_ACTIVE, mappedHardwareChipSelectPin5);
}

/** simple API test that function guards against null pointer */
void testSPI_SetFunctionalNullPointer(void) {
    TEST_ASSERT_FAIL_ASSERT(SPI_SetFunctional(NULL_PTR, 0, false));
}

/** test intended function of SPI_SetFunctional() for setting a bit */
void testSPI_SetFunctionalTestIntendedFunctionSet(void) {
    /** fake a config register that is null and inject into function */
    spiMockConfigRegister.CONFIG_PC0 = 0;
    spi1GetConfigValue_Expect(NULL_PTR, CurrentValue);
    spi1GetConfigValue_IgnoreArg_config_reg();
    spi1GetConfigValue_ReturnThruPtr_config_reg(&spiMockConfigRegister);

    /* the function should call spiSetFunctional with a 1 at bit 10 */
    spiSetFunctional_Expect(spiREG1, ((uint32_t)1u << 10u));

    SPI_SetFunctional(spiREG1, 10, true);
}

/** test intended function of SPI_SetFunctional() for clearing a bit */
void testSPI_SetFunctionalTestIntendedFunctionClear(void) {
    /** fake a config register that is UINT32_MAX and inject into function */
    spiMockConfigRegister.CONFIG_PC0 = UINT32_MAX;
    spi1GetConfigValue_Expect(NULL_PTR, CurrentValue);
    spi1GetConfigValue_IgnoreArg_config_reg();
    spi1GetConfigValue_ReturnThruPtr_config_reg(&spiMockConfigRegister);

    /* the function should call spiSetFunctional with a 0 at bit 10 */
    spiSetFunctional_Expect(spiREG1, ~((uint32_t)1u << 10u));

    SPI_SetFunctional(spiREG1, 10, false);
}

/** test usage of right API functions for SPI1 */
void testSPI_SetFunctionalRightApiSpi1(void) {
    /* this test will fail if another function than the intended function is
    called */
    spi1GetConfigValue_Ignore();
    spiSetFunctional_Ignore();
    SPI_SetFunctional(spiREG1, 0, false);
}

/** test usage of right API functions for SPI2 */
void testSPI_SetFunctionalRightApiSpi2(void) {
    /* this test will fail if another function than the intended function is
    called */
    spi2GetConfigValue_Ignore();
    spiSetFunctional_Ignore();
    SPI_SetFunctional(spiREG2, 0, false);
}

/** test usage of right API functions for SPI3 */
void testSPI_SetFunctionalRightApiSpi3(void) {
    /* this test will fail if another function than the intended function is
    called */
    spi3GetConfigValue_Ignore();
    spiSetFunctional_Ignore();
    SPI_SetFunctional(spiREG3, 0, false);
}

/** test usage of right API functions for SPI4 */
void testSPI_SetFunctionalRightApiSpi4(void) {
    /* this test will fail if another function than the intended function is
    called */
    spi4GetConfigValue_Ignore();
    spiSetFunctional_Ignore();
    SPI_SetFunctional(spiREG4, 0, false);
}

/** test usage of right API functions for SPI5 */
void testSPI_SetFunctionalRightApiSpi5(void) {
    /* this test will fail if another function than the intended function is
    called */
    spi5GetConfigValue_Ignore();
    spiSetFunctional_Ignore();
    SPI_SetFunctional(spiREG5, 0, false);
}

/** test invalid input to SPI_CheckInterfaceAvailable */
void testSPI_CheckInterfaceAvailableInvalidInput(void) {
    TEST_ASSERT_FAIL_ASSERT(SPI_CheckInterfaceAvailable(NULL_PTR));
}

/** test all return codes from HAL with SPI_CheckInterfaceAvailable */
void testSPI_CheckInterfaceAvailable(void) {
    SpiTxStatus_ExpectAndReturn(spiREG1, SPI_READY);
    TEST_ASSERT_EQUAL(STD_OK, SPI_CheckInterfaceAvailable(spiREG1));

    SpiTxStatus_ExpectAndReturn(spiREG1, SPI_PENDING);
    TEST_ASSERT_EQUAL(STD_NOT_OK, SPI_CheckInterfaceAvailable(spiREG1));

    SpiTxStatus_ExpectAndReturn(spiREG1, SPI_COMPLETED);
    TEST_ASSERT_EQUAL(STD_OK, SPI_CheckInterfaceAvailable(spiREG1));
}

/**
 * @brief   Testing extern function #SPI_GetSpiIndex
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: NULL_PTR for pNode -> assert
 *          - Routine validation:
 *            - RT1/2: check that the correct SPI index is returned
 *            - RT2/2: provide an invalid SPI register -> assert
 */
void testSPI_GetSpiIndex(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(SPI_GetSpiIndex(NULL_PTR));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/2: Test implementation */

    /* ======= RT1/2: call function under test */
    const uint8_t spiIndex1 = SPI_GetSpiIndex(spiREG1);
    const uint8_t spiIndex2 = SPI_GetSpiIndex(spiREG2);
    const uint8_t spiIndex3 = SPI_GetSpiIndex(spiREG3);
    const uint8_t spiIndex4 = SPI_GetSpiIndex(spiREG4);
    const uint8_t spiIndex5 = SPI_GetSpiIndex(spiREG5);

    /* ======= RT1/2: test output verification */
    TEST_ASSERT_EQUAL(0u, spiIndex1);
    TEST_ASSERT_EQUAL(1u, spiIndex2);
    TEST_ASSERT_EQUAL(2u, spiIndex3);
    TEST_ASSERT_EQUAL(3u, spiIndex4);
    TEST_ASSERT_EQUAL(4u, spiIndex5);

    /* ======= RT2/2: Test implementation */
    spiBASE_t *invalidSpiRegister = (spiBASE_t *)0xFFF7FBE8u;

    /* ======= RT2/2: call function under test */
    TEST_ASSERT_FAIL_ASSERT(SPI_GetSpiIndex(invalidSpiRegister));

    /* ======= RT2/2: test output verification */
    /* nothing to be verified */
}
