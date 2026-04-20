/*
* Copyright (C) 2009-2018 Texas Instruments Incorporated - www.ti.com
*
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*    Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
*
*    Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the
*    distribution.
*
*    Neither the name of Texas Instruments Incorporated nor the names of
*    its contributors may be used to endorse or promote products derived
*    from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
*  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
*  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
*  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
*  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
*  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
*  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
*  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/

/**
 * @file    spi_cfg_nxp.c
 * @date    11-Dec-2018
 * @updated 2026-04-20 (date of last update)
 * @version v1.11.0
 * @ingroup DRIVERS
 * @prefix  SPI
 *
 * @brief   Configuration of the SPI module.
 * @details The function 'spiInit' is created by TI HALCoGen 04.07.01 in
 *          'HL_spi.c' and adapted as 'SPI_InitializeAfeSpecificSpiInterfaces'
 *          to foxBMS specific needs.
 *          Changes are:
 *          - add 'section'-markers
 *          - add a prefix
 *          - add code justifications for SPA (Axivion)
 *          - make code unit testable ('UNITY_UNIT_TEST')
 *
 */

/*========== Includes =======================================================*/
#include "HL_reg_spi.h"
#include "HL_spi.h"

#include "spi_cfg_initialization.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/* cspell:disable */
/** @struct g_spiPacket
*   @brief globals
*
*/
static volatile struct g_spiPacket {
    spiDAT1_t g_spiDataFormat;
    uint32 tx_length;
    uint32 rx_length;
    uint16 *txdata_ptr;
    uint16 *rxdata_ptr;
    SpiDataStatus_t tx_data_status;
    SpiDataStatus_t rx_data_status;
} g_spiPacket_t[5U];
/* cspell:enable */

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/

/* GCOVR_EXCL_START */
/* cspell:disable */
/* 'SPI_InitializeAfeSpecificSpiInterfaces' is a rename of 'spiInit' */
/** @fn void SPI_InitializeAfeSpecificSpiInterfaces(void)
*   @brief Initializes the SPI Driver
*
*   This function initializes the SPI module.
*/
/* SourceId : SPI_SourceId_001 */
/* DesignId : SPI_DesignId_001 */
/* Requirements : HL_CONQ_SPI_SR7, HL_CONQ_SPI_SR8 */
void SPI_InitializeAfeSpecificSpiInterfaces(void) {
    /* USER CODE BEGIN (2) */
    /* USER CODE END */

    /** @b initialize @b SPI1 */

    /** bring SPI out of reset */
    spiREG1->GCR0 = 0U;
    spiREG1->GCR0 = 1U;

    /** SPI1 master mode and clock configuration */
    spiREG1->GCR1 = (spiREG1->GCR1 & 0xFFFFFFFCU) | ((uint32)((uint32)1U << 1U) /* CLOKMOD */
                                                     | 1U);                     /* MASTER */

    /** SPI1 enable pin configuration */
    spiREG1->INT0 = (spiREG1->INT0 & 0xFEFFFFFFU) | (uint32)((uint32)1U << 24U); /* ENABLE HIGHZ */

    /** - Delays */
    spiREG1->DELAY = (uint32)((uint32)173U << 24U)  /* C2TDELAY */
                     | (uint32)((uint32)99U << 16U) /* T2CDELAY */
                     | (uint32)((uint32)0U << 8U)   /* T2EDELAY */
                     | (uint32)((uint32)0U << 0U);  /* C2EDELAY */

    /** - Data Format 0 */
    spiREG1->FMT0 = (uint32)((uint32)0U << 24U)   /* wdelay */
                    | (uint32)((uint32)0U << 23U) /* parity Polarity */
                    | (uint32)((uint32)0U << 22U) /* parity enable */
                    | (uint32)((uint32)0U << 21U) /* wait on enable */
                    | (uint32)((uint32)0U << 20U) /* shift direction */
                    | (uint32)((uint32)0U << 17U) /* clock polarity */
                    | (uint32)((uint32)1U << 16U) /* clock phase */
                    | (uint32)((uint32)99U << 8U) /* baudrate prescale */
                    | (uint32)((uint32)8U << 0U); /* data word length */

    /** - Data Format 1 */
    spiREG1->FMT1 = (uint32)((uint32)0U << 24U)   /* wdelay */
                    | (uint32)((uint32)0U << 23U) /* parity Polarity */
                    | (uint32)((uint32)0U << 22U) /* parity enable */
                    | (uint32)((uint32)0U << 21U) /* wait on enable */
                    | (uint32)((uint32)0U << 20U) /* shift direction */
                    | (uint32)((uint32)0U << 17U) /* clock polarity */
                    | (uint32)((uint32)1U << 16U) /* clock phase */
                    | (uint32)((uint32)24U << 8U) /* baudrate prescale */
                    | (uint32)((uint32)8U << 0U); /* data word length */

    /** - Data Format 2 */
    spiREG1->FMT2 = (uint32)((uint32)0U << 24U)    /* wdelay */
                    | (uint32)((uint32)0U << 23U)  /* parity Polarity */
                    | (uint32)((uint32)0U << 22U)  /* parity enable */
                    | (uint32)((uint32)0U << 21U)  /* wait on enable */
                    | (uint32)((uint32)0U << 20U)  /* shift direction */
                    | (uint32)((uint32)0U << 17U)  /* clock polarity */
                    | (uint32)((uint32)0U << 16U)  /* clock phase */
                    | (uint32)((uint32)49U << 8U)  /* baudrate prescale */
                    | (uint32)((uint32)16U << 0U); /* data word length */

    /** - Data Format 3 */
    spiREG1->FMT3 = (uint32)((uint32)0U << 24U)   /* wdelay */
                    | (uint32)((uint32)0U << 23U) /* parity Polarity */
                    | (uint32)((uint32)0U << 22U) /* parity enable */
                    | (uint32)((uint32)0U << 21U) /* wait on enable */
                    | (uint32)((uint32)0U << 20U) /* shift direction */
                    | (uint32)((uint32)0U << 17U) /* clock polarity */
                    | (uint32)((uint32)1U << 16U) /* clock phase */
                    | (uint32)((uint32)32U << 8U) /* baudrate prescale */
                    | (uint32)((uint32)8U << 0U); /* data word length */

    /** - set interrupt levels */
    spiREG1->LVL = (uint32)((uint32)0U << 9U)    /* TXINT */
                   | (uint32)((uint32)0U << 8U)  /* RXINT */
                   | (uint32)((uint32)0U << 6U)  /* OVRNINT */
                   | (uint32)((uint32)0U << 4U)  /* BITERR */
                   | (uint32)((uint32)0U << 3U)  /* DESYNC */
                   | (uint32)((uint32)0U << 2U)  /* PARERR */
                   | (uint32)((uint32)0U << 1U)  /* TIMEOUT */
                   | (uint32)((uint32)0U << 0U); /* DLENERR */

    /** - clear any pending interrupts */
    spiREG1->FLG |= 0xFFFFU;

    /** - enable interrupts */
    spiREG1->INT0 = (spiREG1->INT0 & 0xFFFF0000U) | (uint32)((uint32)0U << 9U) /* TXINT */
                    | (uint32)((uint32)1U << 8U)                               /* RXINT */
                    | (uint32)((uint32)1U << 6U)                               /* OVRNINT */
                    | (uint32)((uint32)1U << 4U)                               /* BITERR */
                    | (uint32)((uint32)1U << 3U)                               /* DESYNC */
                    | (uint32)((uint32)1U << 2U)                               /* PARERR */
                    | (uint32)((uint32)1U << 1U)                               /* TIMEOUT */
                    | (uint32)((uint32)1U << 0U);                              /* DLENERR */

    /** @b initialize @b SPI1 @b Port */

    /** - SPI1 Port output values */
    spiREG1->PC3 = (uint32)((uint32)1U << 0U)     /* SCS[0] */
                   | (uint32)((uint32)1U << 1U)   /* SCS[1] */
                   | (uint32)((uint32)1U << 2U)   /* SCS[2] */
                   | (uint32)((uint32)1U << 3U)   /* SCS[3] */
                   | (uint32)((uint32)0U << 4U)   /* SCS[4] */
                   | (uint32)((uint32)1U << 5U)   /* SCS[5] */
                   | (uint32)((uint32)1U << 8U)   /* ENA */
                   | (uint32)((uint32)0U << 9U)   /* CLK */
                   | (uint32)((uint32)0U << 10U)  /* SIMO[0] */
                   | (uint32)((uint32)0U << 11U)  /* SOMI[0] */
                   | (uint32)((uint32)0U << 17U)  /* SIMO[1] */
                   | (uint32)((uint32)0U << 25U); /* SOMI[1] */

    /** - SPI1 Port direction */
    spiREG1->PC1 = (uint32)((uint32)1U << 0U)     /* SCS[0] */
                   | (uint32)((uint32)1U << 1U)   /* SCS[1] */
                   | (uint32)((uint32)1U << 2U)   /* SCS[2] */
                   | (uint32)((uint32)1U << 3U)   /* SCS[3] */
                   | (uint32)((uint32)0U << 4U)   /* SCS[4] */
                   | (uint32)((uint32)1U << 5U)   /* SCS[5] */
                   | (uint32)((uint32)0U << 8U)   /* ENA */
                   | (uint32)((uint32)1U << 9U)   /* CLK */
                   | (uint32)((uint32)1U << 10U)  /* SIMO[0] */
                   | (uint32)((uint32)0U << 11U)  /* SOMI[0] */
                   | (uint32)((uint32)1U << 17U)  /* SIMO[1] */
                   | (uint32)((uint32)0U << 25U); /* SOMI[1] */

    /** - SPI1 Port open drain enable */
    spiREG1->PC6 = (uint32)((uint32)0U << 0U)     /* SCS[0] */
                   | (uint32)((uint32)0U << 1U)   /* SCS[1] */
                   | (uint32)((uint32)0U << 2U)   /* SCS[2] */
                   | (uint32)((uint32)0U << 3U)   /* SCS[3] */
                   | (uint32)((uint32)0U << 4U)   /* SCS[4] */
                   | (uint32)((uint32)0U << 5U)   /* SCS[5] */
                   | (uint32)((uint32)1U << 8U)   /* ENA */
                   | (uint32)((uint32)0U << 9U)   /* CLK */
                   | (uint32)((uint32)0U << 10U)  /* SIMO[0] */
                   | (uint32)((uint32)1U << 11U)  /* SOMI[0] */
                   | (uint32)((uint32)0U << 17U)  /* SIMO[1] */
                   | (uint32)((uint32)1U << 25U); /* SOMI[1] */

    /** - SPI1 Port pullup / pulldown selection */
    spiREG1->PC8 = (uint32)((uint32)1U << 0U)     /* SCS[0] */
                   | (uint32)((uint32)1U << 1U)   /* SCS[1] */
                   | (uint32)((uint32)1U << 2U)   /* SCS[2] */
                   | (uint32)((uint32)1U << 3U)   /* SCS[3] */
                   | (uint32)((uint32)1U << 4U)   /* SCS[4] */
                   | (uint32)((uint32)1U << 5U)   /* SCS[5] */
                   | (uint32)((uint32)1U << 8U)   /* ENA */
                   | (uint32)((uint32)1U << 9U)   /* CLK */
                   | (uint32)((uint32)1U << 10U)  /* SIMO[0] */
                   | (uint32)((uint32)1U << 11U)  /* SOMI[0] */
                   | (uint32)((uint32)1U << 17U)  /* SIMO[1] */
                   | (uint32)((uint32)1U << 25U); /* SOMI[1] */

    /** - SPI1 Port pullup / pulldown enable*/
    spiREG1->PC7 = (uint32)((uint32)1U << 0U)     /* SCS[0] */
                   | (uint32)((uint32)1U << 1U)   /* SCS[1] */
                   | (uint32)((uint32)1U << 2U)   /* SCS[2] */
                   | (uint32)((uint32)1U << 3U)   /* SCS[3] */
                   | (uint32)((uint32)1U << 4U)   /* SCS[4] */
                   | (uint32)((uint32)1U << 5U)   /* SCS[5] */
                   | (uint32)((uint32)0U << 8U)   /* ENA */
                   | (uint32)((uint32)1U << 9U)   /* CLK */
                   | (uint32)((uint32)1U << 10U)  /* SIMO[0] */
                   | (uint32)((uint32)0U << 11U)  /* SOMI[0] */
                   | (uint32)((uint32)1U << 17U)  /* SIMO[1] */
                   | (uint32)((uint32)0U << 25U); /* SOMI[1] */

    /* SPI1 set all pins to functional */
    spiREG1->PC0 = (uint32)((uint32)0U << 0U)     /* SCS[0] */
                   | (uint32)((uint32)0U << 1U)   /* SCS[1] */
                   | (uint32)((uint32)0U << 2U)   /* SCS[2] */
                   | (uint32)((uint32)0U << 3U)   /* SCS[3] */
                   | (uint32)((uint32)0U << 4U)   /* SCS[4] */
                   | (uint32)((uint32)0U << 5U)   /* SCS[5] */
                   | (uint32)((uint32)1U << 8U)   /* ENA */
                   | (uint32)((uint32)1U << 9U)   /* CLK */
                   | (uint32)((uint32)1U << 10U)  /* SIMO[0] */
                   | (uint32)((uint32)1U << 11U)  /* SOMI[0] */
                   | (uint32)((uint32)1U << 17U)  /* SIMO[1] */
                   | (uint32)((uint32)1U << 25U); /* SOMI[1] */

    /** - Initialize TX and RX data buffer Status */
    g_spiPacket_t[0U].tx_data_status = SPI_READY;
    g_spiPacket_t[0U].rx_data_status = SPI_READY;

    /** - Finally start SPI1 */
    spiREG1->GCR1 = (spiREG1->GCR1 & 0xFEFFFFFFU) | 0x01000000U;

    /** @b initialize @b SPI2 */
    /* SPI2 is is generic; see the spi_cfg_generic.c implementation file */

    /** @b initialize @b SPI3 */
    /* SPI3 is is generic; see the spi_cfg_generic.c implementation file */

    /** @b initialize @b SPI4 */

    /** bring SPI out of reset */
    spiREG4->GCR0 = 0U;
    spiREG4->GCR0 = 1U;

    /** SPI4 master mode and clock configuration */
    spiREG4->GCR1 = (spiREG4->GCR1 & 0xFFFFFFFCU) | ((uint32)((uint32)0U << 1U) /* CLOKMOD */
                                                     | 0U);                     /* MASTER */

    /** SPI4 enable pin configuration */
    spiREG4->INT0 = (spiREG4->INT0 & 0xFEFFFFFFU) | (uint32)((uint32)0U << 24U); /* ENABLE HIGHZ */

    /** - Delays */
    spiREG4->DELAY = (uint32)((uint32)0U << 24U)   /* C2TDELAY */
                     | (uint32)((uint32)0U << 16U) /* T2CDELAY */
                     | (uint32)((uint32)0U << 8U)  /* T2EDELAY */
                     | (uint32)((uint32)0U << 0U); /* C2EDELAY */

    /** - Data Format 0 */
    spiREG4->FMT0 = (uint32)((uint32)0U << 24U)   /* wdelay */
                    | (uint32)((uint32)0U << 23U) /* parity Polarity */
                    | (uint32)((uint32)0U << 22U) /* parity enable */
                    | (uint32)((uint32)0U << 21U) /* wait on enable */
                    | (uint32)((uint32)0U << 20U) /* shift direction */
                    | (uint32)((uint32)0U << 17U) /* clock polarity */
                    | (uint32)((uint32)1U << 16U) /* clock phase */
                    | (uint32)((uint32)99U << 8U) /* baudrate prescale */
                    | (uint32)((uint32)8U << 0U); /* data word length */

    /** - Data Format 1 */
    spiREG4->FMT1 = (uint32)((uint32)0U << 24U)   /* wdelay */
                    | (uint32)((uint32)0U << 23U) /* parity Polarity */
                    | (uint32)((uint32)0U << 22U) /* parity enable */
                    | (uint32)((uint32)0U << 21U) /* wait on enable */
                    | (uint32)((uint32)0U << 20U) /* shift direction */
                    | (uint32)((uint32)0U << 17U) /* clock polarity */
                    | (uint32)((uint32)1U << 16U) /* clock phase */
                    | (uint32)((uint32)24U << 8U) /* baudrate prescale */
                    | (uint32)((uint32)8U << 0U); /* data word length */

    /** - Data Format 2 */
    spiREG4->FMT2 = (uint32)((uint32)0U << 24U)    /* wdelay */
                    | (uint32)((uint32)0U << 23U)  /* parity Polarity */
                    | (uint32)((uint32)0U << 22U)  /* parity enable */
                    | (uint32)((uint32)0U << 21U)  /* wait on enable */
                    | (uint32)((uint32)0U << 20U)  /* shift direction */
                    | (uint32)((uint32)0U << 17U)  /* clock polarity */
                    | (uint32)((uint32)0U << 16U)  /* clock phase */
                    | (uint32)((uint32)49U << 8U)  /* baudrate prescale */
                    | (uint32)((uint32)16U << 0U); /* data word length */

    /** - Data Format 3 */
    spiREG4->FMT3 = (uint32)((uint32)0U << 24U)   /* wdelay */
                    | (uint32)((uint32)0U << 23U) /* parity Polarity */
                    | (uint32)((uint32)0U << 22U) /* parity enable */
                    | (uint32)((uint32)0U << 21U) /* wait on enable */
                    | (uint32)((uint32)0U << 20U) /* shift direction */
                    | (uint32)((uint32)0U << 17U) /* clock polarity */
                    | (uint32)((uint32)1U << 16U) /* clock phase */
                    | (uint32)((uint32)32U << 8U) /* baudrate prescale */
                    | (uint32)((uint32)8U << 0U); /* data word length */

    /** - set interrupt levels */
    spiREG4->LVL = (uint32)((uint32)0U << 9U)    /* TXINT */
                   | (uint32)((uint32)0U << 8U)  /* RXINT */
                   | (uint32)((uint32)0U << 6U)  /* OVRNINT */
                   | (uint32)((uint32)0U << 4U)  /* BITERR */
                   | (uint32)((uint32)0U << 3U)  /* DESYNC */
                   | (uint32)((uint32)0U << 2U)  /* PARERR */
                   | (uint32)((uint32)0U << 1U)  /* TIMEOUT */
                   | (uint32)((uint32)0U << 0U); /* DLENERR */

    /** - clear any pending interrupts */
    spiREG4->FLG |= 0xFFFFU;

    /** - enable interrupts */
    spiREG4->INT0 = (spiREG4->INT0 & 0xFFFF0000U) | (uint32)((uint32)0U << 9U) /* TXINT */
                    | (uint32)((uint32)1U << 8U)                               /* RXINT */
                    | (uint32)((uint32)1U << 6U)                               /* OVRNINT */
                    | (uint32)((uint32)1U << 4U)                               /* BITERR */
                    | (uint32)((uint32)1U << 3U)                               /* DESYNC */
                    | (uint32)((uint32)1U << 2U)                               /* PARERR */
                    | (uint32)((uint32)1U << 1U)                               /* TIMEOUT */
                    | (uint32)((uint32)1U << 0U);                              /* DLENERR */

    /** @b initialize @b SPI4 @b Port */

    /** - SPI4 Port output values */
    spiREG4->PC3 = (uint32)((uint32)1U << 0U)     /* SCS[0] */
                   | (uint32)((uint32)1U << 1U)   /* SCS[1] */
                   | (uint32)((uint32)1U << 2U)   /* SCS[2] */
                   | (uint32)((uint32)1U << 3U)   /* SCS[3] */
                   | (uint32)((uint32)1U << 4U)   /* SCS[4] */
                   | (uint32)((uint32)1U << 5U)   /* SCS[5] */
                   | (uint32)((uint32)1U << 8U)   /* ENA */
                   | (uint32)((uint32)1U << 9U)   /* CLK */
                   | (uint32)((uint32)1U << 10U)  /* SIMO */
                   | (uint32)((uint32)1U << 11U); /* SOMI */

    /** - SPI4 Port direction */
    spiREG4->PC1 = (uint32)((uint32)0U << 0U)     /* SCS[0] */
                   | (uint32)((uint32)0U << 1U)   /* SCS[1] */
                   | (uint32)((uint32)0U << 2U)   /* SCS[2] */
                   | (uint32)((uint32)0U << 3U)   /* SCS[3] */
                   | (uint32)((uint32)0U << 4U)   /* SCS[4] */
                   | (uint32)((uint32)0U << 5U)   /* SCS[5] */
                   | (uint32)((uint32)0U << 8U)   /* ENA */
                   | (uint32)((uint32)0U << 9U)   /* CLK */
                   | (uint32)((uint32)0U << 10U)  /* SIMO */
                   | (uint32)((uint32)1U << 11U); /* SOMI */

    /** - SPI4 Port open drain enable */
    spiREG4->PC6 = (uint32)((uint32)0U << 0U)     /* SCS[0] */
                   | (uint32)((uint32)0U << 1U)   /* SCS[1] */
                   | (uint32)((uint32)0U << 2U)   /* SCS[2] */
                   | (uint32)((uint32)0U << 3U)   /* SCS[3] */
                   | (uint32)((uint32)0U << 4U)   /* SCS[4] */
                   | (uint32)((uint32)0U << 5U)   /* SCS[5] */
                   | (uint32)((uint32)1U << 8U)   /* ENA */
                   | (uint32)((uint32)0U << 9U)   /* CLK */
                   | (uint32)((uint32)0U << 10U)  /* SIMO */
                   | (uint32)((uint32)0U << 11U); /* SOMI */

    /** - SPI4 Port pullup / pulldown selection */
    spiREG4->PC8 = (uint32)((uint32)1U << 0U)     /* SCS[0] */
                   | (uint32)((uint32)1U << 1U)   /* SCS[1] */
                   | (uint32)((uint32)1U << 2U)   /* SCS[2] */
                   | (uint32)((uint32)1U << 3U)   /* SCS[3] */
                   | (uint32)((uint32)1U << 4U)   /* SCS[4] */
                   | (uint32)((uint32)1U << 5U)   /* SCS[5] */
                   | (uint32)((uint32)1U << 8U)   /* ENA */
                   | (uint32)((uint32)1U << 9U)   /* CLK */
                   | (uint32)((uint32)1U << 10U)  /* SIMO */
                   | (uint32)((uint32)1U << 11U); /* SOMI */

    /** - SPI4 Port pullup / pulldown enable*/
    spiREG4->PC7 = (uint32)((uint32)0U << 0U)     /* SCS[0] */
                   | (uint32)((uint32)0U << 1U)   /* SCS[1] */
                   | (uint32)((uint32)0U << 2U)   /* SCS[2] */
                   | (uint32)((uint32)0U << 3U)   /* SCS[3] */
                   | (uint32)((uint32)0U << 4U)   /* SCS[4] */
                   | (uint32)((uint32)0U << 5U)   /* SCS[5] */
                   | (uint32)((uint32)0U << 8U)   /* ENA */
                   | (uint32)((uint32)1U << 9U)   /* CLK */
                   | (uint32)((uint32)1U << 10U)  /* SIMO */
                   | (uint32)((uint32)1U << 11U); /* SOMI */

    /* SPI4 set all pins to functional */
    spiREG4->PC0 = (uint32)((uint32)0U << 0U)     /* SCS[0] */
                   | (uint32)((uint32)0U << 1U)   /* SCS[1] */
                   | (uint32)((uint32)0U << 2U)   /* SCS[2] */
                   | (uint32)((uint32)0U << 3U)   /* SCS[3] */
                   | (uint32)((uint32)0U << 4U)   /* SCS[4] */
                   | (uint32)((uint32)0U << 5U)   /* SCS[5] */
                   | (uint32)((uint32)1U << 8U)   /* ENA */
                   | (uint32)((uint32)1U << 9U)   /* CLK */
                   | (uint32)((uint32)1U << 10U)  /* SIMO */
                   | (uint32)((uint32)1U << 11U); /* SOMI */

    /** - Initialize TX and RX data buffer Status */
    g_spiPacket_t[3U].tx_data_status = SPI_READY;
    g_spiPacket_t[3U].rx_data_status = SPI_READY;

    /** - Finally start SPI4 */
    spiREG4->GCR1 = (spiREG4->GCR1 & 0xFEFFFFFFU) | 0x01000000U;

    /** @b initialize @b SPI5 */
    /* SPI5 is is generic; see the spi_cfg_generic.c implementation file */

    /* USER CODE BEGIN (3) */
    /* USER CODE END */
}
/* cspell:enable */
/* GCOVR_EXCL_STOP */

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
