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
 * @updated 2021-12-08 (date of last update)
 * @ingroup DRIVERS
 * @prefix  SPI
 *
 * @brief   Driver for the SPI module.
 *
 */

/*========== Includes =======================================================*/
#include "spi.h"

#include "HL_reg_spi.h"
#include "HL_spi.h"
#include "HL_sys_common.h"

#include "dma.h"
#include "fsystem.h"
#include "io.h"
#include "mcu.h"
#include "os.h"

static uint32_t spi_txLastWord[DMA_NUMBER_SPI_INTERFACES] = {0};

/*========== Macros and Definitions =========================================*/
/** Bitfield to check for transmission errors in SPI FLAG register */
#define SPI_FLAG_REGISTER_TRANSMISSION_ERRORS (0x5Fu)

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/

extern STD_RETURN_TYPE_e SPI_TransmitDummyByte(SPI_INTERFACE_CONFIG_s *pSpiInterface, uint32_t delay) {
    FAS_ASSERT(pSpiInterface != NULL_PTR);
    uint16_t txDummy[1]      = {0x00};
    STD_RETURN_TYPE_e retVal = SPI_TransmitData(pSpiInterface, txDummy, 1u);
    MCU_delay_us(delay);
    return retVal;
}

extern STD_RETURN_TYPE_e SPI_TransmitData(SPI_INTERFACE_CONFIG_s *pSpiInterface, uint16 *pTxBuff, uint32 frameLength) {
    FAS_ASSERT(pSpiInterface != NULL_PTR);
    FAS_ASSERT(pTxBuff != NULL_PTR);
    FAS_ASSERT(frameLength > 0u);
    STD_RETURN_TYPE_e retval = STD_NOT_OK;

    /* Lock SPI hardware to prevent concurrent read/write commands */
    if (STD_OK == SPI_Lock(SPI_GetSpiIndex(pSpiInterface->pNode))) {
        pSpiInterface->pNode->GCR1 |= SPIEN_BIT;

        /** SW Chip Select */
        if (pSpiInterface->csType == SPI_CHIP_SELECT_SOFTWARE) {
            /** Set SPI Chip Select pins as GIOs */
            pSpiInterface->pNode->PC0 &= SPI_PC0_CLEAR_HW_CS_MASK;
            /** Activate Chip Select */
            IO_PinReset(pSpiInterface->pGioPort, pSpiInterface->csPin);
        }
        /** HW Chip Select */
        if (pSpiInterface->csType == SPI_CHIP_SELECT_HARDWARE) {
            /**
             *  Activate HW Chip Select according to bitmask register CSNR
             *  by setting pins as SPI functional pins
             */
            /** First deactivate all HW Chip Selects */
            pSpiInterface->pNode->PC0 &= SPI_PC0_CLEAR_HW_CS_MASK;
            for (uint8_t csNumber = 0u; csNumber < SPI_MAX_NUMBER_HW_CS; csNumber++) {
                if (((pSpiInterface->pConfig->CSNR >> csNumber) & 0x1u) == 0u) {
                    /** Bitmask = 0 --> HW CS active
                     *  --> write  to PC0 to set pin as SPI pin (and not GIO)
                     */
                    pSpiInterface->pNode->PC0 |= (uint32_t)1u << csNumber;
                }
            }
        }
        uint32_t spiRetval = spiTransmitData(pSpiInterface->pNode, pSpiInterface->pConfig, frameLength, pTxBuff);
        /** SW Chip Select */
        if (pSpiInterface->csType == SPI_CHIP_SELECT_SOFTWARE) {
            /** Deactivate Chip Select */
            IO_PinSet(pSpiInterface->pGioPort, pSpiInterface->csPin);
        }

        /* Unlock SPI hardware */
        SPI_Unlock(SPI_GetSpiIndex(pSpiInterface->pNode));

        /* Transmission successful */
        if ((spiRetval & SPI_FLAG_REGISTER_TRANSMISSION_ERRORS) == 0u) {
            retval = STD_OK;
        }
    }
    return retval;
}

extern STD_RETURN_TYPE_e SPI_TransmitReceiveData(
    SPI_INTERFACE_CONFIG_s *pSpiInterface,
    uint16 *pTxBuff,
    uint16 *pRxBuff,
    uint32 frameLength) {
    FAS_ASSERT(pSpiInterface != NULL_PTR);
    FAS_ASSERT(pTxBuff != NULL_PTR);
    FAS_ASSERT(pRxBuff != NULL_PTR);
    FAS_ASSERT(frameLength > 0u);
    STD_RETURN_TYPE_e retval = STD_NOT_OK;

    /* Lock SPI hardware to prevent concurrent read/write commands */
    if (STD_OK == SPI_Lock(SPI_GetSpiIndex(pSpiInterface->pNode))) {
        pSpiInterface->pNode->GCR1 |= SPIEN_BIT;

        /** SW Chip Select */
        if (pSpiInterface->csType == SPI_CHIP_SELECT_SOFTWARE) {
            /** Set SPI Chip Select pins as GIOs */
            pSpiInterface->pNode->PC0 &= SPI_PC0_CLEAR_HW_CS_MASK;
            /** Activate Chip Select */
            IO_PinReset(pSpiInterface->pGioPort, pSpiInterface->csPin);
        }
        /** HW Chip Select */
        if (pSpiInterface->csType == SPI_CHIP_SELECT_HARDWARE) {
            /**
             *  Activate HW Chip Select according to bitmask register CSNR
             *  by setting pins as SPI functional pins
             */
            /** First deactivate all HW Chip Selects */
            pSpiInterface->pNode->PC0 &= SPI_PC0_CLEAR_HW_CS_MASK;
            for (uint8_t csNumber = 0u; csNumber < SPI_MAX_NUMBER_HW_CS; csNumber++) {
                if (((pSpiInterface->pConfig->CSNR >> csNumber) & 0x1u) == 0u) {
                    /** Bitmask = 0 --> HW CS active
                     *  --> write  to PC0 to set pin as SPI pin (and not GIO)
                     */
                    pSpiInterface->pNode->PC0 |= (uint32_t)1u << csNumber;
                }
            }
        }
        uint32_t spiRetval =
            spiTransmitAndReceiveData(pSpiInterface->pNode, pSpiInterface->pConfig, frameLength, pTxBuff, pRxBuff);
        /** SW Chip Select */
        if (pSpiInterface->csType == SPI_CHIP_SELECT_SOFTWARE) {
            /** Deactivate Chip Select */
            IO_PinSet(pSpiInterface->pGioPort, pSpiInterface->csPin);
        }

        /* Unlock SPI hardware */
        SPI_Unlock(SPI_GetSpiIndex(pSpiInterface->pNode));

        /* Transmission successful */
        if ((spiRetval & SPI_FLAG_REGISTER_TRANSMISSION_ERRORS) == 0u) {
            retval = STD_OK;
        }
    }
    return retval;
}

extern void SPI_FramTransmitReceiveData(
    SPI_INTERFACE_CONFIG_s *pSpiInterface,
    uint16 *pTxBuff,
    uint16 *pRxBuff,
    uint32 frameLength) {
    FAS_ASSERT(pSpiInterface != NULL_PTR);
    FAS_ASSERT(pTxBuff != NULL_PTR);
    FAS_ASSERT(pRxBuff != NULL_PTR);
    FAS_ASSERT(frameLength > 0u);

    (void)spiTransmitAndReceiveData(pSpiInterface->pNode, pSpiInterface->pConfig, frameLength, pTxBuff, pRxBuff);
}

extern STD_RETURN_TYPE_e SPI_TransmitReceiveDataDma(
    SPI_INTERFACE_CONFIG_s *pSpiInterface,
    uint16_t *pTxBuff,
    uint16_t *pRxBuff,
    uint32_t frameLength) {
    FAS_ASSERT(frameLength > 2u);
    FAS_ASSERT(pSpiInterface != NULL_PTR);
    FAS_ASSERT(pTxBuff != NULL_PTR);
    FAS_ASSERT(pRxBuff != NULL_PTR);
    /** SPI over DMA currently only compatible with HW Chip Select */
    FAS_ASSERT(pSpiInterface->csType == SPI_CHIP_SELECT_HARDWARE);
    FAS_ASSERT(SPI_GetSpiIndex(pSpiInterface->pNode) < spi_nrBusyFlags);

    STD_RETURN_TYPE_e retVal = STD_NOT_OK;

    OS_EnterTaskCritical();
    /* Lock SPI hardware to prevent concurrent read/write commands */
    if (spi_busyFlags[SPI_GetSpiIndex(pSpiInterface->pNode)] == SPI_IDLE) {
        spi_busyFlags[SPI_GetSpiIndex(pSpiInterface->pNode)] = SPI_BUSY;

        /* Check that not SPI transmission over DMA is taking place */
        if ((pSpiInterface->pNode->INT0 & DMAREQEN_BIT) == 0x0) {
            /* The upper 16 bits will be written in the SPI DAT1 register where they serve as configuration */
            uint32 Chip_Select_Hold = 0u;
            if (pSpiInterface->pConfig->CS_HOLD == TRUE) {
                Chip_Select_Hold = SPI_CSHOLD_BIT;
            } else {
                Chip_Select_Hold = 0U;
            }
            uint32 WDelay = 0u;
            if (pSpiInterface->pConfig->WDEL == TRUE) {
                WDelay = SPI_WDEL_BIT;
            } else {
                WDelay = 0U;
            }
            SPIDATAFMT_t DataFormat = pSpiInterface->pConfig->DFSEL;
            uint8 ChipSelect        = pSpiInterface->pConfig->CSNR;

            /* Go to privilege mode to write DMA config registers */
            (void)FSYS_RaisePrivilege();

            spi_txLastWord[SPI_GetSpiIndex(pSpiInterface->pNode)] = pTxBuff[frameLength - 1u];
            spi_txLastWord[SPI_GetSpiIndex(pSpiInterface->pNode)] |=
                ((uint32)DataFormat << SPI_DATA_FORMAT_FIELD_POSITION) |
                ((uint32)ChipSelect << SPI_HARDWARE_CHIP_SELECT_FIELD_POSITION) | (WDelay);

            /* Set Tx buffer address */
            dmaRAMREG->PCP[(dmaChannel_t)dma_spiDmaChannels[SPI_GetSpiIndex(pSpiInterface->pNode)].txChannel].ISADDR =
                (uint32_t)(&pTxBuff[1u]); /* First word sent manually to write configuration in SPIDAT1 register */
            /**
              *  Set number of Tx words to send
              *  Last word sent in ISR to set CSHOLD = 0
              */
            dmaRAMREG->PCP[(dmaChannel_t)dma_spiDmaChannels[SPI_GetSpiIndex(pSpiInterface->pNode)].txChannel].ITCOUNT =
                ((frameLength - 2u) << 16U) | 1U; /* Last word sent manually to write CSHOLD in SPIDAT1 register */

            /* Set Rx buffer address */
            dmaRAMREG->PCP[(dmaChannel_t)dma_spiDmaChannels[SPI_GetSpiIndex(pSpiInterface->pNode)].rxChannel].IDADDR =
                (uint32_t)pRxBuff;
            /* Set number of Rx words to receive */
            dmaRAMREG->PCP[(dmaChannel_t)dma_spiDmaChannels[SPI_GetSpiIndex(pSpiInterface->pNode)].rxChannel].ITCOUNT =
                (frameLength << 16U) | 1U;

            /* Re-enable channels; because auto-init is disabled */
            /* Disable otherwise transmission  is constantly ongoing */
            dmaSetChEnable(
                (dmaChannel_t)dma_spiDmaChannels[SPI_GetSpiIndex(pSpiInterface->pNode)].txChannel,
                (dmaTriggerType_t)DMA_HW);
            dmaSetChEnable(
                (dmaChannel_t)dma_spiDmaChannels[SPI_GetSpiIndex(pSpiInterface->pNode)].rxChannel,
                (dmaTriggerType_t)DMA_HW);

            /* DMA config registers written, leave privilege mode */
            FSYS_SwitchToUserMode();

            /* DMA_REQ_Enable */
            /* Starts DMA requests if SPIEN is also set to 1 */
            pSpiInterface->pNode->GCR1 |= SPIEN_BIT;
            uint32_t txBuffer = pTxBuff[0u];
            txBuffer |= ((uint32)DataFormat << 24U) | ((uint32)ChipSelect << 16U) | (WDelay) | (Chip_Select_Hold);
            /**
                        *  Send first word without DMA because when writing config to DAT1
                        *  the HW CS pin are asserted immediately, even if SPIEN bit in GCR1 is 0.
                        *  The C2TDELAY is then taken into account before the transmission.
                        */
            pSpiInterface->pNode->DAT1 = txBuffer;
            uint32_t timeoutIterations = SPI_TX_EMPTY_TIMEOUT_ITERATIONS;
            while (((pSpiInterface->pNode->FLG & (uint32)((uint32_t)1u << SPI_TX_BUFFER_EMPTY_FLAG_POSITION)) == 0u) &&
                   (timeoutIterations > 0u)) {
                timeoutIterations--;
            }
            pSpiInterface->pNode->INT0 |= DMAREQEN_BIT;

            retVal = STD_OK;
        }
    }
    OS_ExitTaskCritical();

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

extern void SPI_SetFunctional(spiBASE_t *pNode, enum spiPinSelect bit, bool hardwareControlled) {
    FAS_ASSERT(pNode != NULL_PTR);
    FAS_ASSERT(bit <= (enum spiPinSelect)LARGEST_PIN_NUMBER);

    /* retrieve current configuration */
    spi_config_reg_t configRegisterBuffer = {0};
    if (pNode == spiREG1) {
        spi1GetConfigValue(&configRegisterBuffer, CurrentValue);
    } else if (pNode == spiREG2) {
        spi2GetConfigValue(&configRegisterBuffer, CurrentValue);
    } else if (pNode == spiREG3) {
        spi3GetConfigValue(&configRegisterBuffer, CurrentValue);
    } else if (pNode == spiREG4) {
        spi4GetConfigValue(&configRegisterBuffer, CurrentValue);
    } else if (pNode == spiREG5) {
        spi5GetConfigValue(&configRegisterBuffer, CurrentValue);
    } else {
        /* invalid SPI node */
        FAS_ASSERT(FAS_TRAP);
    }
    uint32_t newPc0 = configRegisterBuffer.CONFIG_PC0;

    if (hardwareControlled == false) {
        /* bit has to be cleared */
        newPc0 &= ~(uint32_t)((uint32_t)1u << (uint8_t)(bit));
    } else {
        /* bit has to be set */
        newPc0 |= (uint32_t)((uint32_t)1u << (uint8_t)(bit));
    }

    /* set new port value */
    spiSetFunctional(pNode, newPc0);
}

extern STD_RETURN_TYPE_e SPI_SlaveSetReceiveDataDma(
    SPI_INTERFACE_CONFIG_s *pSpiInterface,
    uint16_t *pTxBuff,
    uint16_t *pRxBuff,
    uint32_t frameLength) {
    FAS_ASSERT(pSpiInterface != NULL_PTR);
    FAS_ASSERT(pTxBuff != NULL_PTR);
    FAS_ASSERT(pRxBuff != NULL_PTR);
    FAS_ASSERT(frameLength > 0u);
    /** SPI receive works only with HW Chip Select */
    FAS_ASSERT(pSpiInterface->csType == SPI_CHIP_SELECT_HARDWARE);

    STD_RETURN_TYPE_e retVal = STD_OK;

    OS_EnterTaskCritical();
    /* Go to privilege mode to write DMA config registers */
    (void)FSYS_RaisePrivilege();

    /* DMA_REQ Disable */
    pSpiInterface->pNode->INT0 &= ~DMAREQEN_BIT;
    pSpiInterface->pNode->GCR1 &= ~SPIEN_BIT;

    /* Set Tx buffer address */
    dmaRAMREG->PCP[(dmaChannel_t)dma_spiDmaChannels[SPI_GetSpiIndex(pSpiInterface->pNode)].txChannel].ISADDR =
        (uint32_t)pTxBuff;
    /* Set number of Tx bytes to send */
    dmaRAMREG->PCP[(dmaChannel_t)dma_spiDmaChannels[SPI_GetSpiIndex(pSpiInterface->pNode)].txChannel].ITCOUNT =
        (frameLength << 16U) | 1U;

    /* Set Rx buffer address */
    dmaRAMREG->PCP[(dmaChannel_t)dma_spiDmaChannels[SPI_GetSpiIndex(pSpiInterface->pNode)].rxChannel].IDADDR =
        (uint32_t)pRxBuff;
    /* Set number of Rx bytes to receive */
    dmaRAMREG->PCP[(dmaChannel_t)dma_spiDmaChannels[SPI_GetSpiIndex(pSpiInterface->pNode)].rxChannel].ITCOUNT =
        (frameLength << 16U) | 1U;

    /* Re-enable channels; because auto-init is disabled */
    /* Disable otherwise transmission  is constantly ongoping */
    dmaSetChEnable(
        (dmaChannel_t)dma_spiDmaChannels[SPI_GetSpiIndex(pSpiInterface->pNode)].txChannel, (dmaTriggerType_t)DMA_HW);
    dmaSetChEnable(
        (dmaChannel_t)dma_spiDmaChannels[SPI_GetSpiIndex(pSpiInterface->pNode)].rxChannel, (dmaTriggerType_t)DMA_HW);

    /* DMA config registers written, leave privilege mode */
    FSYS_SwitchToUserMode();

    OS_ExitTaskCritical();

    /* Activate chip selects according to bitmask register CSNR */
    for (uint8_t csNumber = 0u; csNumber < SPI_MAX_NUMBER_HW_CS; csNumber++) {
        if (((pSpiInterface->pConfig->CSNR >> csNumber) & 0x1u) == 0u) {
            /* Bitmask = 0 --> HW CS active --> set pin as SPI functional pin */
            pSpiInterface->pNode->PC0 |= (uint32_t)1u << csNumber;
        }
    }

    /* DMA_REQ Enable */
    /* Starts DMA requests if SPIEN is also set to 1 */
    pSpiInterface->pNode->GCR1 |= SPIEN_BIT;
    pSpiInterface->pNode->INT0 |= DMAREQEN_BIT;

    return retVal;
}

extern void SPI_DmaSendLastByte(uint8_t spiIndex) {
    FAS_ASSERT(spiIndex < DMA_NUMBER_SPI_INTERFACES);
    dma_spiInterfaces[spiIndex]->DAT1 = spi_txLastWord[spiIndex];
}

/* AXIVION Next Line Style Linker-Multiple_Definition: TI HAL only provides a weak implementation */
void UNIT_TEST_WEAK_IMPL spiNotification(spiBASE_t *spi, uint32 flags) {
}

extern STD_RETURN_TYPE_e SPI_CheckInterfaceAvailable(spiBASE_t *pNode) {
    FAS_ASSERT(pNode != NULL_PTR);
    const SpiDataStatus_t spiStatus = SpiTxStatus(pNode);
    STD_RETURN_TYPE_e retval        = STD_OK;
    if (spiStatus == SPI_PENDING) {
        retval = STD_NOT_OK;
    }
    return retval;
}

extern uint8_t SPI_GetSpiIndex(spiBASE_t *pNode) {
    FAS_ASSERT(pNode != NULL_PTR);
    uint8_t spiIndex = 0u;

    if (pNode == spiREG1) {
        spiIndex = SPI_SPI1_INDEX;
    } else if (pNode == spiREG2) {
        spiIndex = SPI_SPI2_INDEX;
    } else if (pNode == spiREG3) {
        spiIndex = SPI_SPI3_INDEX;
    } else if (pNode == spiREG4) {
        spiIndex = SPI_SPI4_INDEX;
    } else if (pNode == spiREG5) {
        spiIndex = SPI_SPI5_INDEX;
    } else {
        /** Invalid SPI node */
        FAS_ASSERT(FAS_TRAP);
    }

    return spiIndex;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
