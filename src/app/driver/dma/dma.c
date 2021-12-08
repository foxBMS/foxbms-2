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
 * @file    dma.c
 * @author  foxBMS Team
 * @date    2019-12-12 (date of creation)
 * @updated 2021-12-08 (date of last update)
 * @ingroup DRIVERS
 * @prefix  DMA
 *
 * @brief   Driver for the DMA module.
 *
 */

/*========== Includes =======================================================*/
#include "dma.h"

#include "afe_dma.h"
#include "i2c.h"
#include "spi.h"

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/

void DMA_Initialize(void) {
    /* DMA control packets configuration for SPI  */
    g_dmaCTRL dma_controlPacketSpiTx = {
        .SADD      = 0u,                               /* source address             */
        .DADD      = 0u,                               /* destination  address       */
        .CHCTRL    = 0u,                               /* channel chain control      */
        .FRCNT     = 0u,                               /* frame count                */
        .ELCNT     = 1u,                               /* element count              */
        .ELDOFFSET = 0u,                               /* element destination offset */
        .ELSOFFSET = 0u,                               /* element destination offset */
        .FRDOFFSET = 0u,                               /* frame destination offset   */
        .FRSOFFSET = 0u,                               /* frame destination offset   */
        .PORTASGN  = (uint32_t)PORTA_READ_PORTB_WRITE, /* port assignment            */
        .RDSIZE    = (uint32_t)ACCESS_16_BIT,          /* read size                  */
        .WRSIZE    = (uint32_t)ACCESS_16_BIT,          /* write size                 */
        .TTYPE     = (uint32_t)FRAME_TRANSFER,         /* transfer type              */
        .ADDMODERD = (uint32_t)ADDR_INC1,              /* address mode read          */
        .ADDMODEWR = (uint32_t)ADDR_FIXED,             /* address mode write         */
        .AUTOINIT  = (uint32_t)AUTOINIT_OFF            /* autoinit                   */
    };

    g_dmaCTRL dma_controlPacketSpiRx = {
        .SADD      = 0u,                               /* source address             */
        .DADD      = 0u,                               /* destination  address       */
        .CHCTRL    = 0u,                               /* channel chain control      */
        .FRCNT     = 0u,                               /* frame count                */
        .ELCNT     = 1u,                               /* element count              */
        .ELDOFFSET = 0u,                               /* element destination offset */
        .ELSOFFSET = 0u,                               /* element destination offset */
        .FRDOFFSET = 0u,                               /* frame destination offset   */
        .FRSOFFSET = 0u,                               /* frame destination offset   */
        .PORTASGN  = (uint32_t)PORTB_READ_PORTA_WRITE, /* port assignment            */
        .RDSIZE    = ACCESS_16_BIT,                    /* read size                  */
        .WRSIZE    = ACCESS_16_BIT,                    /* write size                 */
        .TTYPE     = FRAME_TRANSFER,                   /* transfer type              */
        .ADDMODERD = ADDR_FIXED,                       /* address mode read          */
        .ADDMODEWR = ADDR_INC1,                        /* address mode write         */
        .AUTOINIT  = AUTOINIT_OFF                      /* autoinit                   */
    };

    /* DMA control packets configuration for I2C1  */
    g_dmaCTRL dma_controlPacketI2cTx = {
        .SADD      = 0u,                               /* source address             */
        .DADD      = 0u,                               /* destination  address       */
        .CHCTRL    = 0u,                               /* channel chain control      */
        .FRCNT     = 0u,                               /* frame count                */
        .ELCNT     = 1u,                               /* element count              */
        .ELDOFFSET = 0u,                               /* element destination offset */
        .ELSOFFSET = 0u,                               /* element destination offset */
        .FRDOFFSET = 0u,                               /* frame destination offset   */
        .FRSOFFSET = 0u,                               /* frame destination offset   */
        .PORTASGN  = (uint32_t)PORTA_READ_PORTB_WRITE, /* port assignment            */
        .RDSIZE    = (uint32_t)ACCESS_8_BIT,           /* read size                  */
        .WRSIZE    = (uint32_t)ACCESS_8_BIT,           /* write size                 */
        .TTYPE     = (uint32_t)FRAME_TRANSFER,         /* transfer type              */
        .ADDMODERD = (uint32_t)ADDR_INC1,              /* address mode read          */
        .ADDMODEWR = (uint32_t)ADDR_FIXED,             /* address mode write         */
        .AUTOINIT  = (uint32_t)AUTOINIT_OFF            /* autoinit                   */
    };

    g_dmaCTRL dma_controlPacketI2cRx = {
        .SADD      = 0u,                               /* source address             */
        .DADD      = 0u,                               /* destination  address       */
        .CHCTRL    = 0u,                               /* channel chain control      */
        .FRCNT     = 0u,                               /* frame count                */
        .ELCNT     = 1u,                               /* element count              */
        .ELDOFFSET = 0u,                               /* element destination offset */
        .ELSOFFSET = 0u,                               /* element destination offset */
        .FRDOFFSET = 0u,                               /* frame destination offset   */
        .FRSOFFSET = 0u,                               /* frame destination offset   */
        .PORTASGN  = (uint32_t)PORTB_READ_PORTA_WRITE, /* port assignment            */
        .RDSIZE    = (uint32_t)ACCESS_8_BIT,           /* read size                  */
        .WRSIZE    = (uint32_t)ACCESS_8_BIT,           /* write size                 */
        .TTYPE     = (uint32_t)FRAME_TRANSFER,         /* transfer type              */
        .ADDMODERD = (uint32_t)ADDR_FIXED,             /* address mode read          */
        .ADDMODEWR = (uint32_t)ADDR_INC1,              /* address mode write         */
        .AUTOINIT  = (uint32_t)AUTOINIT_OFF            /* autoinit                   */
    };

    dmaEnable();

    /* Configuration for SPI */

    for (uint8_t i = 0u; i < DMA_NUMBER_SPI_INTERFACES; i++) {
        /* assign dma request to Tx channel */
        dmaReqAssign((dmaChannel_t)dma_spiDmaChannels[i].txChannel, (dmaRequest_t)dma_spiDmaRequests[i].txRequest);
        /* assign dma request to Rx channel */
        dmaReqAssign((dmaChannel_t)dma_spiDmaChannels[i].rxChannel, (dmaRequest_t)dma_spiDmaRequests[i].rxRequest);

        /* Enable Interrupt after reception of data
           Group A - Interrupts (FTC, LFS, HBC, and BTC) are routed to the ARM CPU
           User software should configure only Group A interrupts */
        /**
         * Use Tx interrupt to transfer last word with CSHOLD = 0
         * DO NOT ACTIVATE FOR SLAVE SPI NODES (here SPI4 used as slave)
         * */
        if (i != SPI_GetSpiIndex(spiREG4)) {
            dmaEnableInterrupt(
                (dmaChannel_t)(dmaChannel_t)dma_spiDmaChannels[i].txChannel,
                (dmaInterrupt_t)BTC,
                (dmaIntGroup_t)DMA_INTA);
        }
        /* Use Rx to determine when SPI over DMA transaction is finished */
        dmaEnableInterrupt(
            (dmaChannel_t)(dmaChannel_t)dma_spiDmaChannels[i].rxChannel, (dmaInterrupt_t)BTC, (dmaIntGroup_t)DMA_INTA);

        dma_controlPacketSpiTx.DADD = (uint32_t)(&(dma_spiInterfaces[i]->DAT1)) + DMA_BIG_ENDIAN_ADDRESS_16BIT;
        dma_controlPacketSpiRx.SADD = (uint32_t)(&(dma_spiInterfaces[i]->BUF)) + DMA_BIG_ENDIAN_ADDRESS_16BIT;

        /* Set dma control packet for Tx */
        dmaSetCtrlPacket((dmaChannel_t)dma_spiDmaChannels[i].txChannel, dma_controlPacketSpiTx);

        /* Set dma control packet for Rx */
        dmaSetCtrlPacket((dmaChannel_t)dma_spiDmaChannels[i].rxChannel, dma_controlPacketSpiRx);

        /* Set the dma channels to trigger on h/w request */
        dmaSetChEnable((dmaChannel_t)dma_spiDmaChannels[i].txChannel, (dmaTriggerType_t)DMA_HW);
        dmaSetChEnable((dmaChannel_t)dma_spiDmaChannels[i].rxChannel, (dmaTriggerType_t)DMA_HW);
    }

    /* Configuration for I2C1 */

    /* assign dma request to Tx channel */
    dmaReqAssign((dmaChannel_t)DMA_CHANNEL_I2C_TX, (dmaRequest_t)DMA_REQ_LINE_I2C_TX);
    /* assign dma request to Rx channel */
    dmaReqAssign((dmaChannel_t)DMA_CHANNEL_I2C_RX, (dmaRequest_t)DMA_REQ_LINE_I2C_RX);

    /* Enable Interrupt after reception of data
       Group A - Interrupts (FTC, LFS, HBC, and BTC) are routed to the ARM CPU
       User software should configure only Group A interrupts */
    dmaEnableInterrupt((dmaChannel_t)(dmaChannel_t)DMA_CHANNEL_I2C_TX, (dmaInterrupt_t)BTC, (dmaIntGroup_t)DMA_INTA);
    dmaEnableInterrupt((dmaChannel_t)(dmaChannel_t)DMA_CHANNEL_I2C_RX, (dmaInterrupt_t)BTC, (dmaIntGroup_t)DMA_INTA);

    dma_controlPacketI2cTx.DADD = (uint32_t)(&(i2cREG1->DXR)) + DMA_BIG_ENDIAN_ADDRESS_8BIT;
    dma_controlPacketI2cRx.SADD = (uint32_t)(&(i2cREG1->DRR)) + DMA_BIG_ENDIAN_ADDRESS_8BIT;

    /* Set dma control packet for Tx */
    dmaSetCtrlPacket((dmaChannel_t)DMA_CHANNEL_I2C_TX, dma_controlPacketI2cTx);

    /* Set dma control packet for Rx */
    dmaSetCtrlPacket((dmaChannel_t)DMA_CHANNEL_I2C_RX, dma_controlPacketI2cRx);

    /* Set the dma channels to trigger on h/w request */
    dmaSetChEnable((dmaChannel_t)DMA_CHANNEL_I2C_TX, (dmaTriggerType_t)DMA_HW);
    dmaSetChEnable((dmaChannel_t)DMA_CHANNEL_I2C_RX, (dmaTriggerType_t)DMA_HW);
}

/** Function called on DMA complete interrupts (TX and RX). Defined as weak in HAL. */
/* AXIVION Next Line Style Linker-Multiple_Definition: TI HAL only provides a weak implementation */
void UNIT_TEST_WEAK_IMPL dmaGroupANotification(dmaInterrupt_t inttype, uint32 channel) {
    if (inttype == (dmaInterrupt_t)BTC) {
        uint16_t timeoutIterations = 0u;
        uint8_t spiIndex           = 0u;
        switch (channel) {
            case DMA_CHANNEL_SPI1_TX:
            case DMA_CHANNEL_SPI2_TX:
            case DMA_CHANNEL_SPI3_TX:
            case DMA_CHANNEL_SPI4_TX:
            case DMA_CHANNEL_SPI5_TX:
                /* Search for SPI index with DMA TX channel */
                for (uint8_t i = 0u; i < DMA_NUMBER_SPI_INTERFACES; i++) {
                    if ((uint32_t)dma_spiDmaChannels[i].txChannel == channel) {
                        spiIndex = i;
                        break;
                    }
                }
                /**
                  * TX SPI DMA interrupt: last but one word transmitted,
                  * last word is transmitted manually (in order to write CSHOLD field)
                  */
                timeoutIterations = SPI_TX_EMPTY_TIMEOUT_ITERATIONS;
                /* Wait until TX buffer is free for the last word */
                while (((dma_spiInterfaces[spiIndex]->FLG &
                         (uint32)((uint32_t)1u << SPI_TX_BUFFER_EMPTY_FLAG_POSITION)) == 0u) &&
                       (timeoutIterations > 0u)) {
                    timeoutIterations--;
                }
                SPI_DmaSendLastByte(spiIndex);
                break;

            case DMA_CHANNEL_SPI1_RX:
            case DMA_CHANNEL_SPI2_RX:
            case DMA_CHANNEL_SPI3_RX:
            case DMA_CHANNEL_SPI4_RX:
            case DMA_CHANNEL_SPI5_RX:
                /* Search for SPI index with DMA RX channel */
                for (uint8_t i = 0u; i < DMA_NUMBER_SPI_INTERFACES; i++) {
                    if ((uint32_t)dma_spiDmaChannels[i].rxChannel == channel) {
                        spiIndex = i;
                        break;
                    }
                }
                /* RX SPI DMA interrupt: last word received, means SPI transmission is finished */
                if (spiIndex == SPI_GetSpiIndex(spiREG4)) { /** SPI configured as slave */
                    /* RX DMA interrupt, transmission finished, disable DMA */
                    dma_spiInterfaces[spiIndex]->INT0 &= ~DMAREQEN_BIT;
                    /* Disable SPI to prevent unwanted reception */
                    dma_spiInterfaces[spiIndex]->GCR1 &= ~SPIEN_BIT;
                    /* Set slave SPI Chip Select pins as GIO to deactivate slave SPI Chip Select pins */
                    dma_spiInterfaces[spiIndex]->PC0 &= SPI_PC0_CLEAR_HW_CS_MASK;

                    /* Specific call for measurement ICs */
                    AFE_DmaCallback(spiIndex);
                } else { /* SPI configured as master */
                    /* RX DMA interrupt, transmission finished, disable DMA */
                    dma_spiInterfaces[spiIndex]->INT0 &= ~DMAREQEN_BIT;

                    /* Specific call for measurement ICs */
                    if (spiIndex == SPI_GetSpiIndex(spiREG1)) {
                        AFE_DmaCallback(spiIndex);
                    }
                    spi_busyFlags[spiIndex] = SPI_IDLE;
                }
                break;

            case DMA_CHANNEL_I2C_TX:
                i2cREG1->DMACR &= ~(uint32)0x2u;
                /* Wait until Stop is detected */
                timeoutIterations = I2C_TIMEOUT_ITERATIONS;
                while ((i2cIsStopDetected(i2cREG1) == 0u) && (timeoutIterations > 0u)) {
                    timeoutIterations--;
                }
                /* Clear the Stop condition */
                i2cClearSCD(i2cREG1);
                break;
            case DMA_CHANNEL_I2C_RX:
                i2cREG1->DMACR &= ~(uint32)0x1u;
                /* Wait until Stop is detected */
                timeoutIterations = I2C_TIMEOUT_ITERATIONS;
                while ((i2cIsStopDetected(i2cREG1) == 0u) && (timeoutIterations > 0u)) {
                    timeoutIterations--;
                }
                /* Clear the Stop condition */
                i2cClearSCD(i2cREG1);
                break;
            default:
                break;
        }
    }
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
