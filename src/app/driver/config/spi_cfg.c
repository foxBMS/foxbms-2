/**
 *
 * @copyright &copy; 2010 - 2021, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * @file    spi_cfg.c
 * @author  foxBMS Team
 * @date    2020-03-05 (date of creation)
 * @updated 2021-12-08 (date of last update)
 * @ingroup DRIVERS_CONFIGURATION
 * @prefix  SPI
 *
 * @brief   Configuration for the SPI module
 *
 *
 *
 */

/*========== Includes =======================================================*/
#include "spi_cfg.h"

#include "spi_cfg-helper.h"

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/** @defgroup spi_data_format SPI data configuration structs
 *
 * These data configuration structs are used below in the
 * #SPI_INTERFACE_CONFIG_s structures. Each data configuration structs refers
 * to a format selection (e.g. SPI_FMT_0). These are defined in the HAL for
 * each SPI channel (1-5). In order to limit potential confusion the data
 * formats on SPI1 and SPI4 (the SPI interfaces that are routed to the
 * interface board that contains the AFE) have to be configured the same.
 * This way, the developer can switch between SPI1 and SPI4 without having to
 * change the SPI data format.
 *
 * AFE     | SPI data format
 * ------- | ---------------
 * LTC     | SPI_FMT_0
 * MXM     | SPI_FMT_1
 * NXP     | SPI_FMT_2
 * unused  | SPI_FMT_3
 *
 * For hardware Chip Select, setting one bit to 0 in the CSNR field of the
 * SPIDAT1 regsiter activates the corresponding Chip Select pin.
 */

/** SPI data configuration struct for LTC communication */
static spiDAT1_t spi_kLtcDataConfig[BS_NR_OF_STRINGS] = {
    {                      /* struct is implemented in the TI HAL and uses uppercase true and false */
     .CS_HOLD = TRUE,      /* If true, HW chip select kept active between words */
     .WDEL    = FALSE,     /* Activation of delay between words */
     .DFSEL   = SPI_FMT_0, /* Data word format selection */
     .CSNR    = SPI_HARDWARE_CHIP_SELECT_2_ACTIVE},
    {                      /* struct is implemented in the TI HAL and uses uppercase true and false */
     .CS_HOLD = TRUE,      /* If true, HW chip select kept active */
     .WDEL    = FALSE,     /* Activation of delay between words */
     .DFSEL   = SPI_FMT_0, /* Data word format selection */
     .CSNR    = SPI_HARDWARE_CHIP_SELECT_2_ACTIVE},
    {                      /* struct is implemented in the TI HAL and uses uppercase true and false */
     .CS_HOLD = TRUE,      /* If true, HW chip select kept active */
     .WDEL    = FALSE,     /* Activation of delay between words */
     .DFSEL   = SPI_FMT_0, /* Data word format selection */
     .CSNR    = SPI_HARDWARE_CHIP_SELECT_2_ACTIVE},
};

/** SPI data configuration struct for MXM communication */
static spiDAT1_t spi_kMxmDataConfig = {
    .CS_HOLD = TRUE,      /* If true, HW chip select kept active */
    .WDEL    = TRUE,      /* Activation of delay between words */
    .DFSEL   = SPI_FMT_1, /* Data word format selection */
    .CSNR    = SPI_HARDWARE_CHIP_SELECT_0_ACTIVE};

/** SPI data configuration struct for NXP MC33775A communication, Tx part */
static spiDAT1_t spi_kNxp775DataConfigTx[BS_NR_OF_STRINGS] = {
    {.CS_HOLD = TRUE,      /* If true, HW chip select kept active */
     .WDEL    = TRUE,      /* Activation of delay between words */
     .DFSEL   = SPI_FMT_0, /* Data word format selection */
     .CSNR    = SPI_HARDWARE_CHIP_SELECT_2_ACTIVE},
    {.CS_HOLD = TRUE,      /* If true, HW chip select kept active */
     .WDEL    = TRUE,      /* Activation of delay between words */
     .DFSEL   = SPI_FMT_0, /* Data word format selection */
     .CSNR    = SPI_HARDWARE_CHIP_SELECT_2_ACTIVE},
    {.CS_HOLD = TRUE,      /* If true, HW chip select kept active */
     .WDEL    = TRUE,      /* Activation of delay between words */
     .DFSEL   = SPI_FMT_0, /* Data word format selection */
     .CSNR    = SPI_HARDWARE_CHIP_SELECT_0_ACTIVE},
};

/** SPI data configuration struct for NXP MC33775A communication, Rx part */
static spiDAT1_t spi_kNxp775DataConfigRx[BS_NR_OF_STRINGS] = {
    {.CS_HOLD = TRUE,      /* If true, HW chip select kept active */
     .WDEL    = TRUE,      /* Activation of delay between words */
     .DFSEL   = SPI_FMT_0, /* Data word format selection */
     .CSNR    = SPI_HARDWARE_CHIP_SELECT_0_ACTIVE},
    {.CS_HOLD = TRUE,      /* If true, HW chip select kept active */
     .WDEL    = TRUE,      /* Activation of delay between words */
     .DFSEL   = SPI_FMT_0, /* Data word format selection */
     .CSNR    = SPI_HARDWARE_CHIP_SELECT_0_ACTIVE},
    {.CS_HOLD = TRUE,      /* If true, HW chip select kept active */
     .WDEL    = TRUE,      /* Activation of delay between words */
     .DFSEL   = SPI_FMT_0, /* Data word format selection */
     .CSNR    = SPI_HARDWARE_CHIP_SELECT_0_ACTIVE},
};

/** SPI data configuration struct for FRAM communication */
static spiDAT1_t spi_kFramDataConfig = {
    /* struct is implemented in the TI HAL and uses uppercase true and false */
    .CS_HOLD = TRUE,      /* If true, HW chip select kept active */
    .WDEL    = TRUE,      /* Activation of delay between words */
    .DFSEL   = SPI_FMT_1, /* Data word format selection */
    .CSNR    = SPI_HARDWARE_CHIP_SELECT_DISABLE_ALL};

/** SPI data configuration struct for SPS communication in low speed (4MHz) */
static spiDAT1_t spi_kSpsDataConfigLowSpeed = {
    /* struct is implemented in the TI HAL and uses uppercase true and false */
    .CS_HOLD = TRUE,      /* If true, HW chip select kept active */
    .WDEL    = TRUE,      /* Activation of delay between words */
    .DFSEL   = SPI_FMT_1, /* Data word format selection */
    .CSNR    = SPI_HARDWARE_CHIP_SELECT_1_ACTIVE,
};

/** SPI data configuration struct for SPS communication in high speed (10MHz) */
static spiDAT1_t spi_kSpsDataConfigHighSpeed = {
    /* struct is implemented in the TI HAL and uses uppercase true and false */
    .CS_HOLD = TRUE,      /* If true, HW chip select kept active */
    .WDEL    = TRUE,      /* Activation of delay between words */
    .DFSEL   = SPI_FMT_2, /* Data word format selection */
    .CSNR    = SPI_HARDWARE_CHIP_SELECT_1_ACTIVE};

/** SPI data configuration struct for ADC communication */
static spiDAT1_t spi_kAdc0DataConfig = {
    /* struct is implemented in the TI HAL and uses uppercase true and false */
    .CS_HOLD = TRUE,      /* If true, HW chip select kept active */
    .WDEL    = TRUE,      /* Activation of delay between words */
    .DFSEL   = SPI_FMT_2, /* Data word format selection */
    .CSNR    = SPI_HARDWARE_CHIP_SELECT_4_ACTIVE};

/** SPI data configuration struct for ADC communication */
static spiDAT1_t spi_kAdc1DataConfig = {
    /* struct is implemented in the TI HAL and uses uppercase true and false */
    .CS_HOLD = TRUE,      /* If true, HW chip select kept active */
    .WDEL    = TRUE,      /* Activation of delay between words */
    .DFSEL   = SPI_FMT_2, /* Data word format selection */
    .CSNR    = SPI_HARDWARE_CHIP_SELECT_5_ACTIVE};

/** SPI configuration struct for SBC communication */
static spiDAT1_t spi_kSbcDataConfig = {
    /* struct is implemented in the TI HAL and uses uppercase true and false */
    .CS_HOLD = TRUE,      /* If true, HW chip select kept active */
    .WDEL    = TRUE,      /* Activation of delay between words */
    .DFSEL   = SPI_FMT_0, /* Data word format selection */
    .CSNR    = SPI_HARDWARE_CHIP_SELECT_1_ACTIVE};

/*========== Extern Constant and Variable Definitions =======================*/

/**
 * SPI interface configuration for LTC communication
 * This is a list of structs because of multistring
 */
SPI_INTERFACE_CONFIG_s spi_ltcInterface[BS_NR_OF_STRINGS] = {
    {
        .pConfig  = &spi_kLtcDataConfig[0u],
        .pNode    = spiREG1,
        .pGioPort = &(spiREG1->PC3),
        .csPin    = 2u,
        .csType   = SPI_CHIP_SELECT_HARDWARE,
    },
    {
        .pConfig  = &spi_kLtcDataConfig[1u],
        .pNode    = spiREG1,
        .pGioPort = &(spiREG1->PC3),
        .csPin    = 2u,
        .csType   = SPI_CHIP_SELECT_HARDWARE,
    },
    {
        .pConfig  = &spi_kLtcDataConfig[2u],
        .pNode    = spiREG1,
        .pGioPort = &(spiREG1->PC3),
        .csPin    = 2u,
        .csType   = SPI_CHIP_SELECT_HARDWARE,
    },
};

/** SPI interface configuration for MXM communication */
SPI_INTERFACE_CONFIG_s spi_MxmInterface = {
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
        .csPin    = 2u,
        .csType   = SPI_CHIP_SELECT_HARDWARE,
    },
    {
        .pConfig  = &spi_kNxp775DataConfigTx[1u],
        .pNode    = spiREG1,
        .pGioPort = &(spiREG1->PC3),
        .csPin    = 2u,
        .csType   = SPI_CHIP_SELECT_HARDWARE,
    },
    {
        .pConfig  = &spi_kNxp775DataConfigTx[2u],
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
    {
        .pConfig  = &spi_kNxp775DataConfigRx[1u],
        .pNode    = spiREG4,
        .pGioPort = &(spiREG4->PC3),
        .csPin    = 0u,
        .csType   = SPI_CHIP_SELECT_HARDWARE,
    },
    {
        .pConfig  = &spi_kNxp775DataConfigRx[2u],
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
    .pGioPort = &SPS_SPI_CS_GIOPORT,
    .csPin    = SPS_SPI_CS_PIN,
    .csType   = SPI_CHIP_SELECT_SOFTWARE,
};

/** SPI interface configuration for ADC communication */
SPI_INTERFACE_CONFIG_s spi_adc0Interface = {
    .pConfig  = &spi_kAdc0DataConfig,
    .pNode    = spiREG3,
    .pGioPort = &(spiREG3->PC3),
    .csPin    = 4u,
    .csType   = SPI_CHIP_SELECT_SOFTWARE,
};

/** SPI interface configuration for ADC communication */
SPI_INTERFACE_CONFIG_s spi_adc1Interface = {
    .pConfig  = &spi_kAdc1DataConfig,
    .pNode    = spiREG3,
    .pGioPort = &(spiREG3->PC3),
    .csPin    = 5u,
    .csType   = SPI_CHIP_SELECT_SOFTWARE,
};

/** SPI interface configuration for SBC communication */
SPI_INTERFACE_CONFIG_s spi_kSbcMcuInterface = {
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

/** size of #spi_busyFlags */
const uint8_t spi_nrBusyFlags = sizeof(spi_busyFlags) / sizeof(SPI_BUSY_STATE_e);

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/
extern void SPI_SpsInterfaceSwitchToHighSpeed(SPI_INTERFACE_CONFIG_s *pSpiSpsInterface) {
    FAS_ASSERT(pSpiSpsInterface != NULL_PTR);
    pSpiSpsInterface->pConfig = &spi_kSpsDataConfigHighSpeed;
}

extern void SPI_SpsInterfaceSwitchToLowSpeed(SPI_INTERFACE_CONFIG_s *pSpiSpsInterface) {
    FAS_ASSERT(pSpiSpsInterface != NULL_PTR);
    pSpiSpsInterface->pConfig = &spi_kSpsDataConfigLowSpeed;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
