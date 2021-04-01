/**
 *
 * @copyright &copy; 2010 - 2021, Fraunhofer-Gesellschaft zur Foerderung der
 *  angewandten Forschung e.V. All rights reserved.
 *
 * BSD 3-Clause License
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1.  Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of the copyright holder nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * We kindly request you to use one or more of the following phrases to refer
 * to foxBMS in your hardware, software, documentation or advertising
 * materials:
 *
 * &Prime;This product uses parts of foxBMS&reg;&Prime;
 *
 * &Prime;This product includes parts of foxBMS&reg;&Prime;
 *
 * &Prime;This product is derived from foxBMS&reg;&Prime;
 *
 */

/**
 * @file    spi_cfg.c
 * @author  foxBMS Team
 * @date    2020-03-05 (date of creation)
 * @updated 2021-03-24 (date of last update)
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

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/** SPI data configuration struct for LTC communication */
static const spiDAT1_t spi_kLtcDataConfig = {
    /* struct is implemented in the TI HAL and uses uppercase true and false */
    .CS_HOLD = FALSE,     /* The HW chip select signal is deactivated */
    .WDEL    = TRUE,      /* No delay will be inserted */
    .DFSEL   = SPI_FMT_0, /* Data word format select: Data format 0 (SPI1) */
    .CSNR    = 0x0,       /* Chip select (CS) number, 0x01h for CS[0] */
};

/*========== Extern Constant and Variable Definitions =======================*/
/**
 * SPI interface configuration for LTC communication
 * This is a list of structs because of multistring
 */
SPI_INTERFACE_CONFIG_s spi_ltcInterface[BS_NR_OF_STRINGS] = {
    {
        .channel  = SPI_Interface1,
        .pConfig  = &spi_kLtcDataConfig,
        .pNode    = spiREG1,
        .pGioPort = &(spiREG1->PC3),
        .csPin    = 2u,
    },
    {
        .channel  = SPI_Interface1,
        .pConfig  = &spi_kLtcDataConfig,
        .pNode    = spiREG1,
        .pGioPort = &(spiREG1->PC3),
        .csPin    = 2u,
    },
    {
        .channel  = SPI_Interface1,
        .pConfig  = &spi_kLtcDataConfig,
        .pNode    = spiREG1,
        .pGioPort = &(spiREG1->PC3),
        .csPin    = 2u,
    },
};

/** SPI data configuration struct for MXM communication */
static const spiDAT1_t spi_kMxmDataConfig = {
    .CS_HOLD = FALSE,     /*!< The HW chip select signal is deactivated */
    .WDEL    = TRUE,      /*!< No delay will be inserted */
    .DFSEL   = SPI_FMT_0, /*!< Data word format select: Data format 0 (SPI1) */
    .CSNR    = 0x00,      /*!< Chip select (CS) number, 0x01h for CS[0] */
};

/** SPI interface configuration for MXM communication */
SPI_INTERFACE_CONFIG_s spi_MxmInterface = {
    .channel  = SPI_Interface4,
    .pConfig  = &spi_kMxmDataConfig,
    .pNode    = spiREG4,
    .pGioPort = &(spiREG4->PC3),
    .csPin    = 0u,
};

/** SPI data configuration struct for NXP MC33775A communication */
const spiDAT1_t spi_kNxp775DataConfig = {
    .CS_HOLD = FALSE,     /*!< The HW chip select signal is deactivated */
    .WDEL    = TRUE,      /*!< No delay will be inserted */
    .DFSEL   = SPI_FMT_0, /*!< Data word format select */
    .CSNR    = 0x0,       /*!< Chip select (CS) number, 0x01h for CS[0] */
};

/** SPI interface configuration for N775 communication */
SPI_INTERFACE_CONFIG_s spi_nxp775Interface = {
    .channel  = SPI_Interface1,
    .pConfig  = &spi_kNxp775DataConfig,
    .pNode    = spiREG1,
    .pGioPort = &(spiREG1->PC3),
    .csPin    = 2u,
};

/** SPI data configuration struct for FRAM communication */
static const spiDAT1_t spi_kFramDataConfig = {
    /* struct is implemented in the TI HAL and uses uppercase true and false */
    .CS_HOLD = FALSE,     /*!< The HW chip select signal is deactivated */
    .WDEL    = TRUE,      /*!< No delay will be inserted */
    .DFSEL   = SPI_FMT_1, /*!< Data word format select: data format 1 (SPI3) */
    .CSNR    = 0x0,       /*!< Chip select (CS) number; 0x01h for CS[0] */
};

/** SPI interface configuration for FRAM communication */
SPI_INTERFACE_CONFIG_s spi_framInterface = {
    .channel  = SPI_Interface3,
    .pConfig  = &spi_kFramDataConfig,
    .pNode    = spiREG3,
    .pGioPort = &(spiREG3->PC3),
    .csPin    = 0u,
};

/** SPI data configuration struct for SPS communication in low speed (4MHz) */
static const spiDAT1_t spi_kSpsDataConfigLowSpeed = {
    /* struct is implemented in the TI HAL and uses uppercase true and false */
    .CS_HOLD = FALSE,     /*!< The HW chip select signal is deactivated */
    .WDEL    = TRUE,      /*!< No delay will be inserted */
    .DFSEL   = SPI_FMT_1, /*!< Data word format select: data format 1 (SPI2) */
    .CSNR    = 0x0,       /*!< Chip select (CS) number; 0x01h for CS[0] */
};

/** SPI data configuration struct for SPS communication in high speed (10MHz) */
static const spiDAT1_t spi_kSpsDataConfigHighSpeed = {
    /* struct is implemented in the TI HAL and uses uppercase true and false */
    .CS_HOLD = FALSE,     /*!< The HW chip select signal is deactivated */
    .WDEL    = TRUE,      /*!< No delay will be inserted */
    .DFSEL   = SPI_FMT_2, /*!< Data word format select: data format 1 (SPI2) */
    .CSNR    = 0x0,       /*!< Chip select (CS) number; 0x01h for CS[0] */
};

/** SPI interface configuration for SPS communication */
SPI_INTERFACE_CONFIG_s spi_spsInterface = {
    .channel  = SPI_Interface2,
    .pConfig  = &spi_kSpsDataConfigLowSpeed,
    .pNode    = spiREG2,
    .pGioPort = &SPS_SPI_CS_GIOPORT,
    .csPin    = SPS_SPI_CS_PIN,
};

/** SPI data configuration struct for ADC communication */
static const spiDAT1_t spi_kAdcDataConfig = {
    /* struct is implemented in the TI HAL and uses uppercase true and false */
    .CS_HOLD = FALSE,     /*!< The HW chip select signal is deactivated */
    .WDEL    = TRUE,      /*!< No delay will be inserted */
    .DFSEL   = SPI_FMT_2, /*!< Data word format select: data format 2 (SPI3) */
    .CSNR    = 0x0,       /*!< Chip select (CS) number; 0x01h for CS[0] */
};

/** SPI interface configuration for ADC communication */
SPI_INTERFACE_CONFIG_s spi_adc0Interface = {
    .channel  = SPI_Interface3,
    .pConfig  = &spi_kAdcDataConfig,
    .pNode    = spiREG3,
    .pGioPort = &(spiREG3->PC3),
    .csPin    = 4u,
};

/** SPI interface configuration for ADC communication */
SPI_INTERFACE_CONFIG_s spi_adc1Interface = {
    .channel  = SPI_Interface3,
    .pConfig  = &spi_kAdcDataConfig,
    .pNode    = spiREG3,
    .pGioPort = &(spiREG3->PC3),
    .csPin    = 5u,
};

/** SPI configuration struct for SBC communication */
static const spiDAT1_t spi_kSbcDataConfig = {
    /* struct is implemented in the TI HAL and uses uppercase true and false */
    .CS_HOLD = FALSE,     /*!< The HW chip select signal is deactivated */
    .WDEL    = TRUE,      /*!< No delay will be inserted */
    .DFSEL   = SPI_FMT_0, /*!< Data word format select */
    .CSNR    = 0x0,       /*!< Chip select (CS) number; 0x01h for CS[0] */
};

/** SPI interface configuration for SBC communication */
SPI_INTERFACE_CONFIG_s spi_kSbcMcuInterface = {
    .channel  = SPI_Interface2,
    .pConfig  = &spi_kSbcDataConfig,
    .pNode    = spiREG2,
    .pGioPort = &(spiREG2->PC3),
    .csPin    = 0u,
};

/**
 * @brief   Variable used for SPI over DMA transmission. Retains the CS pin to
 *          deactivate in DMA callback
 */
SPI_INTERFACE_CONFIG_s spi_dmaTransmission[] = {
    {
        .channel  = SPI_Interface1,
        .pConfig  = &spi_kLtcDataConfig,
        .pNode    = spiREG1,
        .pGioPort = &(spiREG1->PC3),
        .csPin    = 2u,
    },
    {
        .channel  = SPI_Interface1,
        .pConfig  = &spi_kLtcDataConfig,
        .pNode    = spiREG1,
        .pGioPort = &(spiREG1->PC3),
        .csPin    = 2u,
    },
    {
        .channel  = SPI_Interface1,
        .pConfig  = &spi_kLtcDataConfig,
        .pNode    = spiREG1,
        .pGioPort = &(spiREG1->PC3),
        .csPin    = 2u,
    },
    {
        .channel  = SPI_Interface1,
        .pConfig  = &spi_kLtcDataConfig,
        .pNode    = spiREG1,
        .pGioPort = &(spiREG1->PC3),
        .csPin    = 2u,
    },
    {
        .channel  = SPI_Interface1,
        .pConfig  = &spi_kLtcDataConfig,
        .pNode    = spiREG1,
        .pGioPort = &(spiREG1->PC3),
        .csPin    = 2u,
    },
};

/**
 * @details With DMA, only FMT0 can be used. This table stores the existing
 *          FMT0. The wanted FMT is then copied in FMT0. In the DMA callback,
 *          the existing FMT0 is restored with this table.
 */
uint32_t spi_saveFmt0[] = {
    0U,
    0U,
    0U,
    0U,
    0U,
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
