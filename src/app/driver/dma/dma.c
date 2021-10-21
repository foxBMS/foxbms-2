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
 * @updated 2021-09-28 (date of last update)
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
#include "io.h"
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
        .SADD      = (uint32_t)NULL_PTR,     /* source address             */
        .DADD      = (uint32_t)NULL_PTR,     /* destination  address       */
        .CHCTRL    = 0U,                     /* channel chain control      */
        .FRCNT     = 0U,                     /* frame count                */
        .ELCNT     = 1U,                     /* element count              */
        .ELDOFFSET = 0U,                     /* element destination offset */
        .ELSOFFSET = 0U,                     /* element destination offset */
        .FRDOFFSET = 0U,                     /* frame destination offset   */
        .FRSOFFSET = 0U,                     /* frame destination offset   */
        .PORTASGN  = PORTA_READ_PORTB_WRITE, /* port assignment            */
        .RDSIZE    = ACCESS_16_BIT,          /* read size                  */
        .WRSIZE    = ACCESS_16_BIT,          /* write size                 */
        .TTYPE     = FRAME_TRANSFER,         /* transfer type              */
        .ADDMODERD = ADDR_INC1,              /* address mode read          */
        .ADDMODEWR = ADDR_FIXED,             /* address mode write         */
        .AUTOINIT  = AUTOINIT_OFF            /* autoinit                   */
    };

    g_dmaCTRL dma_controlPacketSpiRx = {
        .SADD      = (uint32_t)NULL_PTR,     /* source address             */
        .DADD      = (uint32_t)NULL_PTR,     /* destination  address       */
        .CHCTRL    = 0U,                     /* channel chain control      */
        .FRCNT     = 0U,                     /* frame count                */
        .ELCNT     = 1U,                     /* element count              */
        .ELDOFFSET = 0U,                     /* element destination offset */
        .ELSOFFSET = 0U,                     /* element destination offset */
        .FRDOFFSET = 0U,                     /* frame destination offset   */
        .FRSOFFSET = 0U,                     /* frame destination offset   */
        .PORTASGN  = PORTB_READ_PORTA_WRITE, /* port assignment            */
        .RDSIZE    = ACCESS_16_BIT,          /* read size                  */
        .WRSIZE    = ACCESS_16_BIT,          /* write size                 */
        .TTYPE     = FRAME_TRANSFER,         /* transfer type              */
        .ADDMODERD = ADDR_FIXED,             /* address mode read          */
        .ADDMODEWR = ADDR_INC1,              /* address mode write         */
        .AUTOINIT  = AUTOINIT_OFF            /* autoinit                   */
    };

    /* DMA control packets configuration for I2C1  */
    g_dmaCTRL dma_controlPacketI2cTx = {
        .SADD      = (uint32_t)NULL_PTR,     /* source address             */
        .DADD      = (uint32_t)NULL_PTR,     /* destination  address       */
        .CHCTRL    = 0U,                     /* channel chain control      */
        .FRCNT     = 0U,                     /* frame count                */
        .ELCNT     = 1U,                     /* element count              */
        .ELDOFFSET = 0U,                     /* element destination offset */
        .ELSOFFSET = 0U,                     /* element destination offset */
        .FRDOFFSET = 0U,                     /* frame destination offset   */
        .FRSOFFSET = 0U,                     /* frame destination offset   */
        .PORTASGN  = PORTA_READ_PORTB_WRITE, /* port assignment            */
        .RDSIZE    = ACCESS_8_BIT,           /* read size                  */
        .WRSIZE    = ACCESS_8_BIT,           /* write size                 */
        .TTYPE     = FRAME_TRANSFER,         /* transfer type              */
        .ADDMODERD = ADDR_INC1,              /* address mode read          */
        .ADDMODEWR = ADDR_FIXED,             /* address mode write         */
        .AUTOINIT  = AUTOINIT_OFF            /* autoinit                   */
    };

    g_dmaCTRL dma_controlPacketI2cRx = {
        .SADD      = (uint32_t)NULL_PTR,     /* source address             */
        .DADD      = (uint32_t)NULL_PTR,     /* destination  address       */
        .CHCTRL    = 0U,                     /* channel chain control      */
        .FRCNT     = 0U,                     /* frame count                */
        .ELCNT     = 1U,                     /* element count              */
        .ELDOFFSET = 0U,                     /* element destination offset */
        .ELSOFFSET = 0U,                     /* element destination offset */
        .FRDOFFSET = 0U,                     /* frame destination offset   */
        .FRSOFFSET = 0U,                     /* frame destination offset   */
        .PORTASGN  = PORTB_READ_PORTA_WRITE, /* port assignment            */
        .RDSIZE    = ACCESS_8_BIT,           /* read size                  */
        .WRSIZE    = ACCESS_8_BIT,           /* write size                 */
        .TTYPE     = FRAME_TRANSFER,         /* transfer type              */
        .ADDMODERD = ADDR_FIXED,             /* address mode read          */
        .ADDMODEWR = ADDR_INC1,              /* address mode write         */
        .AUTOINIT  = AUTOINIT_OFF            /* autoinit                   */
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
        /* Only use Rx to determine when SPI over DMA transaction is finished */
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
void UNIT_TEST_WEAK_IMPL dmaGroupANotification(dmaInterrupt_t inttype, uint32 channel) {
    if (inttype == (dmaInterrupt_t)BTC) {
        uint8_t spiIndex = 0U;
        uint16_t timeout = I2C_TIMEOUT;
        switch (channel) {
            case DMA_CHANNEL_SPI1_TX:
            case DMA_CHANNEL_SPI1_RX:
            case DMA_CHANNEL_SPI2_TX:
            case DMA_CHANNEL_SPI2_RX:
            case DMA_CHANNEL_SPI3_TX:
            case DMA_CHANNEL_SPI3_RX:
            case DMA_CHANNEL_SPI4_TX:
            case DMA_CHANNEL_SPI4_RX:
            case DMA_CHANNEL_SPI5_TX:
            case DMA_CHANNEL_SPI5_RX:
                /* Search for SPI index with DAM Rx channel */
                for (uint8_t i = 0u; i < DMA_NUMBER_SPI_INTERFACES; i++) {
                    if ((uint32_t)dma_spiDmaChannels[i].rxChannel == channel) {
                        spiIndex = i;
                        break;
                    }
                }

                /* Software deactivate CS */
                IO_PinSet(spi_dmaTransmission[spiIndex].pGioPort, spi_dmaTransmission[spiIndex].csPin);

                /* Disable DMA_REQ_Enable */
                spi_dmaTransmission[spiIndex].pNode->INT0 &= ~DMAREQEN_BIT;
                if (spi_dmaTransmission[spiIndex].channel < spi_nrBusyFlags) {
                    *(spi_busyFlags + spi_dmaTransmission[spiIndex].channel) = SPI_IDLE;
                }

                /* DMA seems to only be able to use FMT0, restore saved FMT0 values */
                spi_dmaTransmission[spiIndex].pNode->FMT0 = spi_saveFmt0[spiIndex];

                /* Specific calls for AFEs */
                if (spiIndex == 0U) {
                    AFE_DmaCallback(inttype, channel);
                }
                break;
            case DMA_CHANNEL_I2C_TX:
                i2cREG1->DMACR &= ~(uint32)0x2u;
                /* Wait until Stop is detected */
                timeout = I2C_TIMEOUT;
                while ((i2cIsStopDetected(i2cREG1) == 0) && (timeout > 0u)) {
                    timeout--;
                }
                /* Clear the Stop condition */
                i2cClearSCD(i2cREG1);
                break;
            case DMA_CHANNEL_I2C_RX:
                i2cREG1->DMACR &= ~(uint32)0x1u;
                /* Wait until Stop is detected */
                timeout = I2C_TIMEOUT;
                while ((i2cIsStopDetected(i2cREG1) == 0) && (timeout > 0u)) {
                    timeout--;
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
