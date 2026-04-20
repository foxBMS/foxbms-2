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
 * @file    spi_cfg_generic.c
 * @date    11-Dec-2018
 * @updated 2026-04-20 (date of last update)
 * @version v1.11.0
 * @ingroup DRIVERS
 * @prefix  SPI
 *
 * @brief   Configuration of the SPI module.
 * @details The function 'spiInit' is created by TI HALCoGen 04.07.01 in
 *          'HL_spi.c' and adapted as 'SPI_InitializeSpiInterfaces' to foxBMS
 *          specific needs.
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
/* 'SPI_InitializeSpiInterfaces' is a rename of 'spiInit' */
/** @fn void SPI_InitializeSpiInterfaces(void)
*   @brief Initializes the SPI Driver
*
*   This function initializes the SPI module.
*/
/* SourceId : SPI_SourceId_001 */
/* DesignId : SPI_DesignId_001 */
/* Requirements : HL_CONQ_SPI_SR7, HL_CONQ_SPI_SR8 */
void SPI_InitializeSpiInterfaces(void) {
    /* USER CODE BEGIN (2) */
    /* USER CODE END */

    /** @b initialize @b SPI1 */
    /* SPI1 is used for the AFEs; see the AFE specific implementation files */

    /** @b initialize @b SPI2 */

    /** bring SPI out of reset */
    spiREG2->GCR0 = 0U;
    spiREG2->GCR0 = 1U;

    /** SPI2 master mode and clock configuration */
    spiREG2->GCR1 = (spiREG2->GCR1 & 0xFFFFFFFCU) | ((uint32)((uint32)1U << 1U) /* CLOKMOD */
                                                     | 1U);                     /* MASTER */

    /** SPI2 enable pin configuration */
    spiREG2->INT0 = (spiREG2->INT0 & 0xFEFFFFFFU) | (uint32)((uint32)1U << 24U); /* ENABLE HIGHZ */

    /** - Delays */
    spiREG2->DELAY = (uint32)((uint32)0U << 24U)   /* C2TDELAY */
                     | (uint32)((uint32)0U << 16U) /* T2CDELAY */
                     | (uint32)((uint32)0U << 8U)  /* T2EDELAY */
                     | (uint32)((uint32)0U << 0U); /* C2EDELAY */

    /** - Data Format 0 */
    spiREG2->FMT0 = (uint32)((uint32)0U << 24U)   /* wdelay */
                    | (uint32)((uint32)0U << 23U) /* parity Polarity */
                    | (uint32)((uint32)0U << 22U) /* parity enable */
                    | (uint32)((uint32)0U << 21U) /* wait on enable */
                    | (uint32)((uint32)0U << 20U) /* shift direction */
                    | (uint32)((uint32)0U << 17U) /* clock polarity */
                    | (uint32)((uint32)0U << 16U) /* clock phase */
                    | (uint32)((uint32)9U << 8U)  /* baudrate prescale */
                    | (uint32)((uint32)8U << 0U); /* data word length */
    /** - Data Format 1 */
    spiREG2->FMT1 = (uint32)((uint32)0U << 24U)    /* wdelay */
                    | (uint32)((uint32)0U << 23U)  /* parity Polarity */
                    | (uint32)((uint32)0U << 22U)  /* parity enable */
                    | (uint32)((uint32)0U << 21U)  /* wait on enable */
                    | (uint32)((uint32)0U << 20U)  /* shift direction */
                    | (uint32)((uint32)0U << 17U)  /* clock polarity */
                    | (uint32)((uint32)0U << 16U)  /* clock phase */
                    | (uint32)((uint32)24U << 8U)  /* baudrate prescale */
                    | (uint32)((uint32)16U << 0U); /* data word length */

    /** - Data Format 2 */
    spiREG2->FMT2 = (uint32)((uint32)0U << 24U)    /* wdelay */
                    | (uint32)((uint32)0U << 23U)  /* parity Polarity */
                    | (uint32)((uint32)0U << 22U)  /* parity enable */
                    | (uint32)((uint32)0U << 21U)  /* wait on enable */
                    | (uint32)((uint32)0U << 20U)  /* shift direction */
                    | (uint32)((uint32)0U << 17U)  /* clock polarity */
                    | (uint32)((uint32)0U << 16U)  /* clock phase */
                    | (uint32)((uint32)9U << 8U)   /* baudrate prescale */
                    | (uint32)((uint32)16U << 0U); /* data word length */

    /** - Data Format 3 */
    spiREG2->FMT3 = (uint32)((uint32)0U << 24U)    /* wdelay */
                    | (uint32)((uint32)0U << 23U)  /* parity Polarity */
                    | (uint32)((uint32)0U << 22U)  /* parity enable */
                    | (uint32)((uint32)0U << 21U)  /* wait on enable */
                    | (uint32)((uint32)0U << 20U)  /* shift direction */
                    | (uint32)((uint32)0U << 17U)  /* clock polarity */
                    | (uint32)((uint32)0U << 16U)  /* clock phase */
                    | (uint32)((uint32)99U << 8U)  /* baudrate prescale */
                    | (uint32)((uint32)16U << 0U); /* data word length */

    /** - set interrupt levels */
    spiREG2->LVL = (uint32)((uint32)0U << 9U)    /* TXINT */
                   | (uint32)((uint32)0U << 8U)  /* RXINT */
                   | (uint32)((uint32)0U << 6U)  /* OVRNINT */
                   | (uint32)((uint32)0U << 4U)  /* BITERR */
                   | (uint32)((uint32)0U << 3U)  /* DESYNC */
                   | (uint32)((uint32)0U << 2U)  /* PARERR */
                   | (uint32)((uint32)0U << 1U)  /* TIMEOUT */
                   | (uint32)((uint32)0U << 0U); /* DLENERR */

    /** - clear any pending interrupts */
    spiREG2->FLG |= 0xFFFFU;

    /** - enable interrupts */
    spiREG2->INT0 = (spiREG2->INT0 & 0xFFFF0000U) | (uint32)((uint32)0U << 9U) /* TXINT */
                    | (uint32)((uint32)1U << 8U)                               /* RXINT */
                    | (uint32)((uint32)1U << 6U)                               /* OVRNINT */
                    | (uint32)((uint32)1U << 4U)                               /* BITERR */
                    | (uint32)((uint32)1U << 3U)                               /* DESYNC */
                    | (uint32)((uint32)1U << 2U)                               /* PARERR */
                    | (uint32)((uint32)1U << 1U)                               /* TIMEOUT */
                    | (uint32)((uint32)1U << 0U);                              /* DLENERR */

    /** @b initialize @b SPI2 @b Port */

    /** - SPI2 Port output values */
    spiREG2->PC3 = (uint32)((uint32)1U << 0U)     /* SCS[0] */
                   | (uint32)((uint32)1U << 1U)   /* SCS[1] */
                   | (uint32)((uint32)0U << 8U)   /* ENA */
                   | (uint32)((uint32)0U << 9U)   /* CLK */
                   | (uint32)((uint32)0U << 10U)  /* SIMO */
                   | (uint32)((uint32)0U << 11U); /* SOMI */

    /** - SPI2 Port direction */
    spiREG2->PC1 = (uint32)((uint32)1U << 0U)     /* SCS[0] */
                   | (uint32)((uint32)1U << 1U)   /* SCS[1] */
                   | (uint32)((uint32)0U << 8U)   /* ENA */
                   | (uint32)((uint32)1U << 9U)   /* CLK */
                   | (uint32)((uint32)1U << 10U)  /* SIMO */
                   | (uint32)((uint32)0U << 11U); /* SOMI */

    /** - SPI2 Port open drain enable */
    spiREG2->PC6 = (uint32)((uint32)0U << 0U)     /* SCS[0] */
                   | (uint32)((uint32)0U << 1U)   /* SCS[1] */
                   | (uint32)((uint32)1U << 8U)   /* ENA */
                   | (uint32)((uint32)0U << 9U)   /* CLK */
                   | (uint32)((uint32)0U << 10U)  /* SIMO */
                   | (uint32)((uint32)1U << 11U); /* SOMI */

    /** - SPI2 Port pullup / pulldown selection */
    spiREG2->PC8 = (uint32)((uint32)1U << 0U)     /* SCS[0] */
                   | (uint32)((uint32)1U << 1U)   /* SCS[1] */
                   | (uint32)((uint32)1U << 8U)   /* ENA */
                   | (uint32)((uint32)1U << 9U)   /* CLK */
                   | (uint32)((uint32)1U << 10U)  /* SIMO */
                   | (uint32)((uint32)1U << 11U); /* SOMI */

    /** - SPI2 Port pullup / pulldown enable*/
    spiREG2->PC7 = (uint32)((uint32)1U << 0U)     /* SCS[0] */
                   | (uint32)((uint32)1U << 1U)   /* SCS[1] */
                   | (uint32)((uint32)0U << 8U)   /* ENA */
                   | (uint32)((uint32)1U << 9U)   /* CLK */
                   | (uint32)((uint32)1U << 10U)  /* SIMO */
                   | (uint32)((uint32)0U << 11U); /* SOMI */

    /* SPI2 set all pins to functional */
    spiREG2->PC0 = (uint32)((uint32)0U << 0U)     /* SCS[0] */
                   | (uint32)((uint32)0U << 1U)   /* SCS[1] */
                   | (uint32)((uint32)1U << 8U)   /* ENA */
                   | (uint32)((uint32)1U << 9U)   /* CLK */
                   | (uint32)((uint32)1U << 10U)  /* SIMO */
                   | (uint32)((uint32)1U << 11U); /* SOMI */

    /** - Initialize TX and RX data buffer Status */
    g_spiPacket_t[1U].tx_data_status = SPI_READY;
    g_spiPacket_t[1U].rx_data_status = SPI_READY;

    /** - Finally start SPI2 */
    spiREG2->GCR1 = (spiREG2->GCR1 & 0xFEFFFFFFU) | 0x01000000U;

    /** @b initialize @b SPI3 */

    /** bring SPI out of reset */
    spiREG3->GCR0 = 0U;
    spiREG3->GCR0 = 1U;

    /** SPI3 master mode and clock configuration */
    spiREG3->GCR1 = (spiREG3->GCR1 & 0xFFFFFFFCU) | ((uint32)((uint32)1U << 1U) /* CLOKMOD */
                                                     | 1U);                     /* MASTER */

    /** SPI3 enable pin configuration */
    spiREG3->INT0 = (spiREG3->INT0 & 0xFEFFFFFFU) | (uint32)((uint32)1U << 24U); /* ENABLE HIGHZ */

    /** - Delays */
    spiREG3->DELAY = (uint32)((uint32)0U << 24U)   /* C2TDELAY */
                     | (uint32)((uint32)0U << 16U) /* T2CDELAY */
                     | (uint32)((uint32)0U << 8U)  /* T2EDELAY */
                     | (uint32)((uint32)0U << 0U); /* C2EDELAY */

    /** - Data Format 0 */
    spiREG3->FMT0 = (uint32)((uint32)0U << 24U)    /* wdelay */
                    | (uint32)((uint32)0U << 23U)  /* parity Polarity */
                    | (uint32)((uint32)0U << 22U)  /* parity enable */
                    | (uint32)((uint32)0U << 21U)  /* wait on enable */
                    | (uint32)((uint32)0U << 20U)  /* shift direction */
                    | (uint32)((uint32)0U << 17U)  /* clock polarity */
                    | (uint32)((uint32)0U << 16U)  /* clock phase */
                    | (uint32)((uint32)24U << 8U)  /* baudrate prescale */
                    | (uint32)((uint32)16U << 0U); /* data word length */

    /** - Data Format 1 */
    spiREG3->FMT1 = (uint32)((uint32)0U << 24U)   /* wdelay */
                    | (uint32)((uint32)0U << 23U) /* parity Polarity */
                    | (uint32)((uint32)0U << 22U) /* parity enable */
                    | (uint32)((uint32)0U << 21U) /* wait on enable */
                    | (uint32)((uint32)0U << 20U) /* shift direction */
                    | (uint32)((uint32)0U << 17U) /* clock polarity */
                    | (uint32)((uint32)0U << 16U) /* clock phase */
                    | (uint32)((uint32)2U << 8U)  /* baudrate prescale */
                    | (uint32)((uint32)8U << 0U); /* data word length */

    /** - Data Format 2 */
    spiREG3->FMT2 = (uint32)((uint32)0U << 24U)    /* wdelay */
                    | (uint32)((uint32)0U << 23U)  /* parity Polarity */
                    | (uint32)((uint32)0U << 22U)  /* parity enable */
                    | (uint32)((uint32)0U << 21U)  /* wait on enable */
                    | (uint32)((uint32)0U << 20U)  /* shift direction */
                    | (uint32)((uint32)0U << 17U)  /* clock polarity */
                    | (uint32)((uint32)0U << 16U)  /* clock phase */
                    | (uint32)((uint32)3U << 8U)   /* baudrate prescale */
                    | (uint32)((uint32)16U << 0U); /* data word length */

    /** - Data Format 3 */
    spiREG3->FMT3 = (uint32)((uint32)0U << 24U)    /* wdelay */
                    | (uint32)((uint32)0U << 23U)  /* parity Polarity */
                    | (uint32)((uint32)0U << 22U)  /* parity enable */
                    | (uint32)((uint32)0U << 21U)  /* wait on enable */
                    | (uint32)((uint32)0U << 20U)  /* shift direction */
                    | (uint32)((uint32)0U << 17U)  /* clock polarity */
                    | (uint32)((uint32)0U << 16U)  /* clock phase */
                    | (uint32)((uint32)99U << 8U)  /* baudrate prescale */
                    | (uint32)((uint32)16U << 0U); /* data word length */

    /** - set interrupt levels */
    spiREG3->LVL = (uint32)((uint32)0U << 9U)    /* TXINT */
                   | (uint32)((uint32)0U << 8U)  /* RXINT */
                   | (uint32)((uint32)0U << 6U)  /* OVRNINT */
                   | (uint32)((uint32)0U << 4U)  /* BITERR */
                   | (uint32)((uint32)0U << 3U)  /* DESYNC */
                   | (uint32)((uint32)0U << 2U)  /* PARERR */
                   | (uint32)((uint32)0U << 1U)  /* TIMEOUT */
                   | (uint32)((uint32)0U << 0U); /* DLENERR */

    /** - clear any pending interrupts */
    spiREG3->FLG |= 0xFFFFU;

    /** - enable interrupts */
    spiREG3->INT0 = (spiREG3->INT0 & 0xFFFF0000U) | (uint32)((uint32)0U << 9U) /* TXINT */
                    | (uint32)((uint32)1U << 8U)                               /* RXINT */
                    | (uint32)((uint32)1U << 6U)                               /* OVRNINT */
                    | (uint32)((uint32)1U << 4U)                               /* BITERR */
                    | (uint32)((uint32)1U << 3U)                               /* DESYNC */
                    | (uint32)((uint32)1U << 2U)                               /* PARERR */
                    | (uint32)((uint32)1U << 1U)                               /* TIMEOUT */
                    | (uint32)((uint32)1U << 0U);                              /* DLENERR */

    /** @b initialize @b SPI3 @b Port */

    /** - SPI3 Port output values */
    spiREG3->PC3 = (uint32)((uint32)1U << 0U)     /* SCS[0] */
                   | (uint32)((uint32)1U << 1U)   /* SCS[1] */
                   | (uint32)((uint32)1U << 2U)   /* SCS[2] */
                   | (uint32)((uint32)1U << 3U)   /* SCS[3] */
                   | (uint32)((uint32)1U << 4U)   /* SCS[4] */
                   | (uint32)((uint32)1U << 5U)   /* SCS[5] */
                   | (uint32)((uint32)1U << 8U)   /* ENA */
                   | (uint32)((uint32)1U << 9U)   /* CLK */
                   | (uint32)((uint32)1U << 10U)  /* SIMO */
                   | (uint32)((uint32)1U << 11U); /* SOMI */

    /** - SPI3 Port direction */
    spiREG3->PC1 = (uint32)((uint32)1U << 0U)     /* SCS[0] */
                   | (uint32)((uint32)1U << 1U)   /* SCS[1] */
                   | (uint32)((uint32)1U << 2U)   /* SCS[2] */
                   | (uint32)((uint32)1U << 3U)   /* SCS[3] */
                   | (uint32)((uint32)1U << 4U)   /* SCS[4] */
                   | (uint32)((uint32)1U << 5U)   /* SCS[5] */
                   | (uint32)((uint32)0U << 8U)   /* ENA */
                   | (uint32)((uint32)1U << 9U)   /* CLK */
                   | (uint32)((uint32)1U << 10U)  /* SIMO */
                   | (uint32)((uint32)0U << 11U); /* SOMI */

    /** - SPI3 Port open drain enable */
    spiREG3->PC6 = (uint32)((uint32)0U << 0U)     /* SCS[0] */
                   | (uint32)((uint32)0U << 1U)   /* SCS[1] */
                   | (uint32)((uint32)0U << 2U)   /* SCS[2] */
                   | (uint32)((uint32)0U << 3U)   /* SCS[3] */
                   | (uint32)((uint32)0U << 4U)   /* SCS[4] */
                   | (uint32)((uint32)0U << 5U)   /* SCS[5] */
                   | (uint32)((uint32)1U << 8U)   /* ENA */
                   | (uint32)((uint32)0U << 9U)   /* CLK */
                   | (uint32)((uint32)0U << 10U)  /* SIMO */
                   | (uint32)((uint32)1U << 11U); /* SOMI */

    /** - SPI3 Port pullup / pulldown selection */
    spiREG3->PC8 = (uint32)((uint32)1U << 0U)     /* SCS[0] */
                   | (uint32)((uint32)1U << 1U)   /* SCS[1] */
                   | (uint32)((uint32)1U << 2U)   /* SCS[2] */
                   | (uint32)((uint32)1U << 3U)   /* SCS[3] */
                   | (uint32)((uint32)1U << 4U)   /* SCS[4] */
                   | (uint32)((uint32)1U << 5U)   /* SCS[5] */
                   | (uint32)((uint32)1U << 8U)   /* ENA */
                   | (uint32)((uint32)1U << 9U)   /* CLK */
                   | (uint32)((uint32)1U << 10U)  /* SIMO */
                   | (uint32)((uint32)1U << 11U); /* SOMI */

    /** - SPI3 Port pullup / pulldown enable*/
    spiREG3->PC7 = (uint32)((uint32)1U << 0U)     /* SCS[0] */
                   | (uint32)((uint32)1U << 1U)   /* SCS[1] */
                   | (uint32)((uint32)1U << 2U)   /* SCS[2] */
                   | (uint32)((uint32)1U << 3U)   /* SCS[3] */
                   | (uint32)((uint32)1U << 4U)   /* SCS[4] */
                   | (uint32)((uint32)1U << 5U)   /* SCS[5] */
                   | (uint32)((uint32)0U << 8U)   /* ENA */
                   | (uint32)((uint32)1U << 9U)   /* CLK */
                   | (uint32)((uint32)1U << 10U)  /* SIMO */
                   | (uint32)((uint32)0U << 11U); /* SOMI */

    /* SPI3 set all pins to functional */
    spiREG3->PC0 = (uint32)((uint32)0U << 0U)     /* SCS[0] */
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
    g_spiPacket_t[2U].tx_data_status = SPI_READY;
    g_spiPacket_t[2U].rx_data_status = SPI_READY;

    /** - Finally start SPI3 */
    spiREG3->GCR1 = (spiREG3->GCR1 & 0xFEFFFFFFU) | 0x01000000U;

    /** @b initialize @b SPI4 */
    /* SPI4 is used for the AFEs; see the AFE specific implementation files */

    /** @b initialize @b SPI5 */

    /** bring SPI out of reset */
    spiREG5->GCR0 = 0U;
    spiREG5->GCR0 = 1U;

    /** SPI5 master mode and clock configuration */
    spiREG5->GCR1 = (spiREG5->GCR1 & 0xFFFFFFFCU) | ((uint32)((uint32)1U << 1U) /* CLOKMOD */
                                                     | 0U);                     /* MASTER */

    /** SPI5 enable pin configuration */
    spiREG5->INT0 = (spiREG5->INT0 & 0xFEFFFFFFU) | (uint32)((uint32)1U << 24U); /* ENABLE HIGHZ */

    /** - Delays */
    spiREG5->DELAY = (uint32)((uint32)0U << 24U)   /* C2TDELAY */
                     | (uint32)((uint32)0U << 16U) /* T2CDELAY */
                     | (uint32)((uint32)0U << 8U)  /* T2EDELAY */
                     | (uint32)((uint32)0U << 0U); /* C2EDELAY */

    /** - Data Format 0 */
    spiREG5->FMT0 = (uint32)((uint32)0U << 24U)    /* wdelay */
                    | (uint32)((uint32)0U << 23U)  /* parity Polarity */
                    | (uint32)((uint32)0U << 22U)  /* parity enable */
                    | (uint32)((uint32)0U << 21U)  /* wait on enable */
                    | (uint32)((uint32)0U << 20U)  /* shift direction */
                    | (uint32)((uint32)0U << 17U)  /* clock polarity */
                    | (uint32)((uint32)0U << 16U)  /* clock phase */
                    | (uint32)((uint32)49U << 8U)  /* baudrate prescale */
                    | (uint32)((uint32)16U << 0U); /* data word length */

    /** - Data Format 1 */
    spiREG5->FMT1 = (uint32)((uint32)0U << 24U)    /* wdelay */
                    | (uint32)((uint32)0U << 23U)  /* parity Polarity */
                    | (uint32)((uint32)0U << 22U)  /* parity enable */
                    | (uint32)((uint32)0U << 21U)  /* wait on enable */
                    | (uint32)((uint32)0U << 20U)  /* shift direction */
                    | (uint32)((uint32)0U << 17U)  /* clock polarity */
                    | (uint32)((uint32)0U << 16U)  /* clock phase */
                    | (uint32)((uint32)99U << 8U)  /* baudrate prescale */
                    | (uint32)((uint32)16U << 0U); /* data word length */

    /** - Data Format 2 */
    spiREG5->FMT2 = (uint32)((uint32)0U << 24U)    /* wdelay */
                    | (uint32)((uint32)0U << 23U)  /* parity Polarity */
                    | (uint32)((uint32)0U << 22U)  /* parity enable */
                    | (uint32)((uint32)0U << 21U)  /* wait on enable */
                    | (uint32)((uint32)0U << 20U)  /* shift direction */
                    | (uint32)((uint32)0U << 17U)  /* clock polarity */
                    | (uint32)((uint32)0U << 16U)  /* clock phase */
                    | (uint32)((uint32)99U << 8U)  /* baudrate prescale */
                    | (uint32)((uint32)16U << 0U); /* data word length */

    /** - Data Format 3 */
    spiREG5->FMT3 = (uint32)((uint32)0U << 24U)    /* wdelay */
                    | (uint32)((uint32)0U << 23U)  /* parity Polarity */
                    | (uint32)((uint32)0U << 22U)  /* parity enable */
                    | (uint32)((uint32)0U << 21U)  /* wait on enable */
                    | (uint32)((uint32)0U << 20U)  /* shift direction */
                    | (uint32)((uint32)0U << 17U)  /* clock polarity */
                    | (uint32)((uint32)0U << 16U)  /* clock phase */
                    | (uint32)((uint32)99U << 8U)  /* baudrate prescale */
                    | (uint32)((uint32)16U << 0U); /* data word length */

    /** - set interrupt levels */
    spiREG5->LVL = (uint32)((uint32)0U << 9U)    /* TXINT */
                   | (uint32)((uint32)0U << 8U)  /* RXINT */
                   | (uint32)((uint32)0U << 6U)  /* OVRNINT */
                   | (uint32)((uint32)0U << 4U)  /* BITERR */
                   | (uint32)((uint32)0U << 3U)  /* DESYNC */
                   | (uint32)((uint32)0U << 2U)  /* PARERR */
                   | (uint32)((uint32)0U << 1U)  /* TIMEOUT */
                   | (uint32)((uint32)0U << 0U); /* DLENERR */

    /** - clear any pending interrupts */
    spiREG5->FLG |= 0xFFFFU;

    /** - enable interrupts */
    spiREG5->INT0 = (spiREG5->INT0 & 0xFFFF0000U) | (uint32)((uint32)0U << 9U) /* TXINT */
                    | (uint32)((uint32)1U << 8U)                               /* RXINT */
                    | (uint32)((uint32)1U << 6U)                               /* OVRNINT */
                    | (uint32)((uint32)1U << 4U)                               /* BITERR */
                    | (uint32)((uint32)1U << 3U)                               /* DESYNC */
                    | (uint32)((uint32)1U << 2U)                               /* PARERR */
                    | (uint32)((uint32)1U << 1U)                               /* TIMEOUT */
                    | (uint32)((uint32)1U << 0U);                              /* DLENERR */

    /** @b initialize @b SPI5 @b Port */

    /** - SPI5 Port output values */
    spiREG5->PC3 = (uint32)((uint32)1U << 0U)     /* SCS[0] */
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
                   | (uint32)((uint32)1U << 18U)  /* SIMO[2] */
                   | (uint32)((uint32)1U << 19U)  /* SIMO[3] */
                   | (uint32)((uint32)1U << 25U)  /* SOMI[1] */
                   | (uint32)((uint32)1U << 26U)  /* SOMI[2] */
                   | (uint32)((uint32)1U << 27U); /* SOMI[3] */

    /** - SPI5 Port direction */
    spiREG5->PC1 = (uint32)((uint32)1U << 0U)     /* SCS[0] */
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
                   | (uint32)((uint32)1U << 18U)  /* SIMO[2] */
                   | (uint32)((uint32)1U << 19U)  /* SIMO[3] */
                   | (uint32)((uint32)0U << 25U)  /* SOMI[1] */
                   | (uint32)((uint32)0U << 26U)  /* SOMI[2] */
                   | (uint32)((uint32)0U << 27U); /* SOMI[3] */

    /** - SPI5 Port open drain enable */
    spiREG5->PC6 = (uint32)((uint32)0U << 0U)     /* SCS[0] */
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
                   | (uint32)((uint32)0U << 18U)  /* SIMO[2] */
                   | (uint32)((uint32)0U << 19U)  /* SIMO[3] */
                   | (uint32)((uint32)1U << 25U)  /* SOMI[1] */
                   | (uint32)((uint32)1U << 26U)  /* SOMI[2] */
                   | (uint32)((uint32)1U << 27U); /* SOMI[3] */

    /** - SPI5 Port pullup / pulldown selection */
    spiREG5->PC8 = (uint32)((uint32)1U << 0U)     /* SCS[0] */
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
                   | (uint32)((uint32)1U << 18U)  /* SIMO[2] */
                   | (uint32)((uint32)1U << 19U)  /* SIMO[3] */
                   | (uint32)((uint32)1U << 25U)  /* SOMI[1] */
                   | (uint32)((uint32)1U << 26U)  /* SOMI[2] */
                   | (uint32)((uint32)1U << 27U); /* SOMI[3] */

    /** - SPI5 Port pullup / pulldown enable*/
    spiREG5->PC7 = (uint32)((uint32)1U << 0U)     /* SCS[0] */
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
                   | (uint32)((uint32)1U << 18U)  /* SIMO[2] */
                   | (uint32)((uint32)1U << 19U)  /* SIMO[3] */
                   | (uint32)((uint32)0U << 25U)  /* SOMI[1] */
                   | (uint32)((uint32)0U << 26U)  /* SOMI[2] */
                   | (uint32)((uint32)0U << 27U); /* SOMI[3] */

    /* SPI5 set all pins to functional */
    spiREG5->PC0 = (uint32)((uint32)1U << 0U)     /* SCS[0] */
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
                   | (uint32)((uint32)1U << 18U)  /* SIMO[2] */
                   | (uint32)((uint32)1U << 19U)  /* SIMO[3] */
                   | (uint32)((uint32)1U << 25U)  /* SOMI[1] */
                   | (uint32)((uint32)1U << 26U)  /* SOMI[2] */
                   | (uint32)((uint32)1U << 27U); /* SOMI[3] */

    /** - Initialize TX and RX data buffer Status */
    g_spiPacket_t[4U].tx_data_status = SPI_READY;
    g_spiPacket_t[4U].rx_data_status = SPI_READY;

    /** - Finally start SPI5 */
    spiREG5->GCR1 = (spiREG5->GCR1 & 0xFEFFFFFFU) | 0x01000000U;

    /* USER CODE BEGIN (3) */
    /* USER CODE END */
}
/* cspell:enable */
/* GCOVR_EXCL_STOP */

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
