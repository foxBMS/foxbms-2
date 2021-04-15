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
 * @file    spi.c
 * @author  foxBMS Team
 * @date    2019-12-12 (date of creation)
 * @updated 2019-12-12 (date of last update)
 * @ingroup DRIVERS
 * @prefix  SPI
 *
 * @brief   Driver for the SPI module.
 *
 */

/*========== Includes =======================================================*/
#include "spi.h"

#include "dma.h"
#include "fsystem.h"
#include "io.h"
#include "mcu.h"
#include "os.h"

/*========== Macros and Definitions =========================================*/
/** Bitfield to check for transmission errors in SPI FLAG register */
#define SPI_FLAG_REGISTER_TRANSMISSION_ERRORS (0x5Fu)

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/

extern STD_RETURN_TYPE_e SPI_TransmitDummyByte(SPI_INTERFACE_CONFIG_s *pSpiInterface, uint32_t delay) {
    STD_RETURN_TYPE_e retval = STD_NOT_OK;
    uint16_t spi_cmdDummy[1] = {0x00};

    /* Lock SPI hardware to prevent concurrent read/write commands */
    if (STD_OK == SPI_Lock(pSpiInterface->channel)) {
        IO_PinReset((uint32_t *)pSpiInterface->pGioPort, pSpiInterface->csPin);
        uint32_t spiRetval =
            spiTransmitData(pSpiInterface->pNode, ((spiDAT1_t *)pSpiInterface->pConfig), 1u, spi_cmdDummy);
        IO_PinSet((uint32_t *)pSpiInterface->pGioPort, pSpiInterface->csPin);

        /* Unlock SPI hardware */
        SPI_Unlock(pSpiInterface->channel);

        /* Transmission successful */
        if ((spiRetval & SPI_FLAG_REGISTER_TRANSMISSION_ERRORS) == 0u) {
            retval = STD_OK;
        }
        MCU_delay_us(delay);
    }
    return retval;
}

STD_RETURN_TYPE_e SPI_TransmitData(SPI_INTERFACE_CONFIG_s *pSpiInterface, uint16_t *pTxBuff, uint32_t frameLength) {
    STD_RETURN_TYPE_e retval = STD_NOT_OK;

    /* Lock SPI hardware to prevent concurrent read/write commands */
    if (STD_OK == SPI_Lock(pSpiInterface->channel)) {
        IO_PinReset((uint32_t *)pSpiInterface->pGioPort, pSpiInterface->csPin);
        uint32_t spiRetval =
            spiTransmitData(pSpiInterface->pNode, ((spiDAT1_t *)pSpiInterface->pConfig), frameLength, pTxBuff);
        IO_PinSet((uint32_t *)pSpiInterface->pGioPort, pSpiInterface->csPin);

        /* Unlock SPI hardware */
        SPI_Unlock(pSpiInterface->channel);

        /* Transmission successful */
        if ((spiRetval & SPI_FLAG_REGISTER_TRANSMISSION_ERRORS) == 0u) {
            retval = STD_OK;
        }
    }
    return retval;
}

STD_RETURN_TYPE_e SPI_TransmitDataWithDummy(
    SPI_INTERFACE_CONFIG_s *pSpiInterface,
    uint32_t delay,
    uint16_t *pTxBuff,
    uint32_t frameLength) {
    STD_RETURN_TYPE_e retval = STD_OK;

    /* Transmit dummy byte */
    if (SPI_TransmitDummyByte(pSpiInterface, delay) == STD_OK) {
        /* Transmit data only if transmission of dummy byte was successful */
        retval = SPI_TransmitData(pSpiInterface, pTxBuff, frameLength);
    } else {
        retval = STD_NOT_OK;
    }
    return retval;
}

extern STD_RETURN_TYPE_e SPI_TransmitReceiveData(
    SPI_INTERFACE_CONFIG_s *pSpiInterface,
    uint16 *pTxBuff,
    uint16 *pRxBuff,
    uint32 frameLength) {
    STD_RETURN_TYPE_e retval = STD_NOT_OK;

    /* Lock SPI hardware to prevent concurrent read/write commands */
    if (STD_OK == SPI_Lock(pSpiInterface->channel)) {
        IO_PinReset((uint32_t *)pSpiInterface->pGioPort, pSpiInterface->csPin);
        uint32_t spiRetval = SPI_DirectlyTransmitReceiveData(pSpiInterface, pTxBuff, pRxBuff, frameLength);
        IO_PinSet((uint32_t *)pSpiInterface->pGioPort, pSpiInterface->csPin);

        /* Unlock SPI hardware */
        SPI_Unlock(pSpiInterface->channel);

        /* Transmission successful */
        if ((spiRetval & SPI_FLAG_REGISTER_TRANSMISSION_ERRORS) == 0u) {
            retval = STD_OK;
        }
    }
    return retval;
}

extern STD_RETURN_TYPE_e SPI_DirectlyTransmitReceiveData(
    SPI_INTERFACE_CONFIG_s *pSpiInterface,
    uint16 *pTxBuff,
    uint16 *pRxBuff,
    uint32 frameLength) {
    STD_RETURN_TYPE_e retval = STD_NOT_OK;

    uint32_t spiRetval = spiTransmitAndReceiveData(
        pSpiInterface->pNode, ((spiDAT1_t *)pSpiInterface->pConfig), frameLength, pTxBuff, pRxBuff);

    if ((spiRetval & SPI_FLAG_REGISTER_TRANSMISSION_ERRORS) == 0u) {
        /* No error flag set during communication */
        retval = STD_OK;
    }
    return retval;
}

extern STD_RETURN_TYPE_e SPI_TransmitReceiveDataDma(
    SPI_INTERFACE_CONFIG_s *pSpiInterface,
    uint16_t *pTxBuff,
    uint16_t *pRxBuff,
    uint32_t frameLength) {
    FAS_ASSERT(pSpiInterface != NULL_PTR);
    FAS_ASSERT(pTxBuff != NULL_PTR);
    FAS_ASSERT(pRxBuff != NULL_PTR);

    STD_RETURN_TYPE_e retVal = STD_NOT_OK;

    OS_EnterTaskCritical();
    /* Lock SPI hardware to prevent concurrent read/write commands */
    if ((*(spi_busyFlags + pSpiInterface->channel) == SPI_IDLE) && (pSpiInterface->channel < spi_nrBusyFlags)) {
        *(spi_busyFlags + pSpiInterface->channel) = SPI_BUSY;
        retVal                                    = STD_OK;
        /* Check that not SPI transmission over DMA is taking place */
        if ((pSpiInterface->pNode->INT0 & DMAREQEN_BIT) == 0x0) {
            /* Go to privilege mode to write DMA config registers */
            FSYS_RaisePrivilege();

            /* Set Tx buffer address */
            dmaRAMREG->PCP[(dmaChannel_t)dma_spiDmaChannels[pSpiInterface->channel].txChannel].ISADDR =
                (uint32_t)pTxBuff;
            /* Set number of Tx bytes to send */
            dmaRAMREG->PCP[(dmaChannel_t)dma_spiDmaChannels[pSpiInterface->channel].txChannel].ITCOUNT =
                (frameLength << 16U) | 1U;

            /* Set Rx buffer address */
            dmaRAMREG->PCP[(dmaChannel_t)dma_spiDmaChannels[pSpiInterface->channel].rxChannel].IDADDR =
                (uint32_t)pRxBuff;
            /* Set number of Rx bytes to receive */
            dmaRAMREG->PCP[(dmaChannel_t)dma_spiDmaChannels[pSpiInterface->channel].rxChannel].ITCOUNT =
                (frameLength << 16U) | 1U;

            uint8_t spiFmtRegister = 0U;
            switch (pSpiInterface->pConfig->DFSEL) {
                case 0U:
                    spiFmtRegister = 0U;
                    break;
                case 1U:
                    spiFmtRegister = 1U;
                    break;
                case 2U:
                    spiFmtRegister = 2U;
                    break;
                case 3U:
                    spiFmtRegister = 3U;
                    break;
                default:
                    spiFmtRegister = 0U;
                    break;
            }

            /* Re-enable channels; because auto-init is disabled */
            /* Disable otherwise transmission  is constantly ongoping */
            dmaSetChEnable(
                (dmaChannel_t)dma_spiDmaChannels[pSpiInterface->channel].txChannel, (dmaTriggerType_t)DMA_HW);
            dmaSetChEnable(
                (dmaChannel_t)dma_spiDmaChannels[pSpiInterface->channel].rxChannel, (dmaTriggerType_t)DMA_HW);

            /* Store the CS pin to be deactivated in DMA callback */
            spi_dmaTransmission[pSpiInterface->channel].channel  = pSpiInterface->channel;
            spi_dmaTransmission[pSpiInterface->channel].pConfig  = pSpiInterface->pConfig;
            spi_dmaTransmission[pSpiInterface->channel].pNode    = pSpiInterface->pNode;
            spi_dmaTransmission[pSpiInterface->channel].pGioPort = pSpiInterface->pGioPort;
            spi_dmaTransmission[pSpiInterface->channel].csPin    = pSpiInterface->csPin;

            /* DMA seems to only be able to use FMT0, save FMT0 config */
            spi_saveFmt0[pSpiInterface->channel] = pSpiInterface->pNode->FMT0;

            /* DMA seems to only be able to use FMT0, write actual FMT in FMT0 */
            switch (spiFmtRegister) {
                case 0U:
                    break;
                case 1U:
                    pSpiInterface->pNode->FMT0 = pSpiInterface->pNode->FMT1;
                    break;
                case 2U:
                    pSpiInterface->pNode->FMT0 = pSpiInterface->pNode->FMT2;
                    break;
                case 3U:
                    pSpiInterface->pNode->FMT0 = pSpiInterface->pNode->FMT3;
                    break;
                default:
                    break;
            }

            /* DMA config registers written, leave privilege mode */
            FSYS_SwitchToUserMode();

            OS_ExitTaskCritical();

            /* Software activate CS */
            IO_PinReset((uint32_t *)pSpiInterface->pGioPort, pSpiInterface->csPin);
            /* DMA_REQ_Enable */
            /* Starts DMA requests if SPIEN is also set to 1 */
            pSpiInterface->pNode->INT0 |= DMAREQEN_BIT;

            retVal = STD_OK;
        }
    } else {
        OS_ExitTaskCritical();
    }

    return retVal;
}

extern STD_RETURN_TYPE_e SPI_TransmitReceiveDataWithDummyDma(
    SPI_INTERFACE_CONFIG_s *pSpiInterface,
    uint32_t delay,
    uint16_t *pTxBuff,
    uint16_t *pRxBuff,
    uint32_t frameLength) {
    STD_RETURN_TYPE_e retVal = STD_NOT_OK;
    uint16_t spi_cmdDummy[1] = {0x00};

    OS_EnterTaskCritical();
    /* Lock SPI hardware to prevent concurrent read/write commands */
    if ((*(spi_busyFlags + pSpiInterface->channel) == SPI_IDLE) && (pSpiInterface->channel < spi_nrBusyFlags)) {
        *(spi_busyFlags + pSpiInterface->channel) = SPI_BUSY;
        retVal                                    = STD_OK;

        /* Check that not SPI transmission over DMA is taking place */
        if ((pSpiInterface->pNode->INT0 & DMAREQEN_BIT) == 0x0) {
            /* Go to privilege mode to write DMA config registers */
            FSYS_RaisePrivilege();

            /* Set Tx buffer address */
            dmaRAMREG->PCP[(dmaChannel_t)dma_spiDmaChannels[pSpiInterface->channel].txChannel].ISADDR =
                (uint32_t)pTxBuff;
            /* Set number of Tx bytes to transmit */
            dmaRAMREG->PCP[(dmaChannel_t)dma_spiDmaChannels[pSpiInterface->channel].txChannel].ITCOUNT =
                (frameLength << 16U) | 1U;

            /* Set Rx buffer address */
            dmaRAMREG->PCP[(dmaChannel_t)dma_spiDmaChannels[pSpiInterface->channel].rxChannel].IDADDR =
                (uint32_t)pRxBuff;
            /* Set number of Rx bytes to receive */
            dmaRAMREG->PCP[(dmaChannel_t)dma_spiDmaChannels[pSpiInterface->channel].rxChannel].ITCOUNT =
                (frameLength << 16U) | 1U;

            uint8_t spiFmtRegister = 0U;
            switch (pSpiInterface->pConfig->DFSEL) {
                case 0U:
                    spiFmtRegister = 0U;
                    break;
                case 1U:
                    spiFmtRegister = 1U;
                    break;
                case 2U:
                    spiFmtRegister = 2U;
                    break;
                case 3U:
                    spiFmtRegister = 3U;
                    break;
                default:
                    spiFmtRegister = 0U;
                    break;
            }

            /* Re-enable channels; because auto-init is disabled */
            /* Disable otherwise transmission  is constantly ongoping */
            dmaSetChEnable(
                (dmaChannel_t)dma_spiDmaChannels[pSpiInterface->channel].txChannel, (dmaTriggerType_t)DMA_HW);
            dmaSetChEnable(
                (dmaChannel_t)dma_spiDmaChannels[pSpiInterface->channel].rxChannel, (dmaTriggerType_t)DMA_HW);

            /* Store the CS pin to be deactivated in DMA callback */
            spi_dmaTransmission[pSpiInterface->channel].channel  = pSpiInterface->channel;
            spi_dmaTransmission[pSpiInterface->channel].pConfig  = pSpiInterface->pConfig;
            spi_dmaTransmission[pSpiInterface->channel].pNode    = pSpiInterface->pNode;
            spi_dmaTransmission[pSpiInterface->channel].pGioPort = pSpiInterface->pGioPort;
            spi_dmaTransmission[pSpiInterface->channel].csPin    = pSpiInterface->csPin;

            /* DMA seems to only be able to use FMT0, save FMT0 config */
            spi_saveFmt0[pSpiInterface->channel] = pSpiInterface->pNode->FMT0;

            /* DMA seems to only be able to use FMT0, write actual FMT in FMT0 */
            switch (spiFmtRegister) {
                case 0U:
                    break;
                case 1U:
                    pSpiInterface->pNode->FMT0 = pSpiInterface->pNode->FMT1;
                    break;
                case 2U:
                    pSpiInterface->pNode->FMT0 = pSpiInterface->pNode->FMT2;
                    break;
                case 3U:
                    pSpiInterface->pNode->FMT0 = pSpiInterface->pNode->FMT3;
                    break;
                default:
                    break;
            }

            /* DMA config registers written, leave privilege mode */
            FSYS_SwitchToUserMode();

            OS_ExitTaskCritical();

            IO_PinReset((uint32_t *)pSpiInterface->pGioPort, pSpiInterface->csPin);
            uint32_t spiRetval =
                spiTransmitData(pSpiInterface->pNode, ((spiDAT1_t *)pSpiInterface->pConfig), 1u, spi_cmdDummy);
            IO_PinSet((uint32_t *)pSpiInterface->pGioPort, pSpiInterface->csPin);
            if ((spiRetval & SPI_FLAG_REGISTER_TRANSMISSION_ERRORS) == 0u) {
                /* No error flag set during communication */

                MCU_delay_us(delay);

                /* Software activate CS */
                IO_PinReset((uint32_t *)pSpiInterface->pGioPort, pSpiInterface->csPin);
                /* DMA_REQ_Enable */
                /* Starts DMA requests if SPIEN is also set to 1 */
                pSpiInterface->pNode->INT0 |= DMAREQEN_BIT;
            }
            retVal = STD_OK;
        }
    } else {
        OS_ExitTaskCritical();
    }

    return retVal;
}

extern STD_RETURN_TYPE_e SPI_Lock(uint8_t spi) {
    STD_RETURN_TYPE_e retVal = STD_NOT_OK;

    OS_EnterTaskCritical();
    if ((*(spi_busyFlags + spi) == SPI_IDLE) && (spi < spi_nrBusyFlags)) {
        *(spi_busyFlags + spi) = SPI_BUSY;
        retVal                 = STD_OK;
    } else {
        retVal = STD_NOT_OK;
    }
    OS_ExitTaskCritical();

    return retVal;
}

extern void SPI_Unlock(uint8_t spi) {
    OS_EnterTaskCritical();
    if (spi < spi_nrBusyFlags) {
        *(spi_busyFlags + spi) = SPI_IDLE;
    }
    OS_ExitTaskCritical();
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
