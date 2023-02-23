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
 * @file    spi.c
 * @author  foxBMS Team
 * @date    2019-12-12 (date of creation)
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
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
#include "fstd_types.h"
#include "fsystem.h"
#include "io.h"
#include "mcu.h"
#include "os.h"
#include "spi_cfg-helper.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/
/** Bitfield to check for transmission errors in SPI FLAG register */
#define SPI_FLAG_REGISTER_TRANSMISSION_ERRORS (0x5Fu)

/*========== Static Constant and Variable Definitions =======================*/
static uint32_t spi_txLastWord[DMA_NUMBER_SPI_INTERFACES] = {0};

/** Defines for hardware chip select pins @{ */
#define SPI_HARDWARE_CHIP_SELECT_PIN_0 (0u)
#define SPI_HARDWARE_CHIP_SELECT_PIN_1 (1u)
#define SPI_HARDWARE_CHIP_SELECT_PIN_2 (2u)
#define SPI_HARDWARE_CHIP_SELECT_PIN_3 (3u)
#define SPI_HARDWARE_CHIP_SELECT_PIN_4 (4u)
#define SPI_HARDWARE_CHIP_SELECT_PIN_5 (5u)
/**@}*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/
static void SPI_InitializeChipSelects(void);
static uint8_t SPI_GetChipSelectPin(SPI_CHIP_SELECT_TYPE_e chipSelectType, uint32_t chipSelectPin);
static uint8_t SPI_GetHardwareChipSelectPin(uint8_t chipSelectPin);

/*========== Static Function Implementations ================================*/
static uint8_t SPI_GetHardwareChipSelectPin(uint8_t chipSelectPin) {
    FAS_ASSERT(chipSelectPin < SPI_MAX_NUMBER_HW_CS);

    uint8_t mappedHardwareChipSelectPin = SPI_HARDWARE_CHIP_SELECT_DISABLE_ALL;
    switch (chipSelectPin) {
        case SPI_HARDWARE_CHIP_SELECT_PIN_0:
            mappedHardwareChipSelectPin = SPI_HARDWARE_CHIP_SELECT_0_ACTIVE;
            break;
        case SPI_HARDWARE_CHIP_SELECT_PIN_1:
            mappedHardwareChipSelectPin = SPI_HARDWARE_CHIP_SELECT_1_ACTIVE;
            break;
        case SPI_HARDWARE_CHIP_SELECT_PIN_2:
            mappedHardwareChipSelectPin = SPI_HARDWARE_CHIP_SELECT_2_ACTIVE;
            break;
        case SPI_HARDWARE_CHIP_SELECT_PIN_3:
            mappedHardwareChipSelectPin = SPI_HARDWARE_CHIP_SELECT_3_ACTIVE;
            break;
        case SPI_HARDWARE_CHIP_SELECT_PIN_4:
            mappedHardwareChipSelectPin = SPI_HARDWARE_CHIP_SELECT_4_ACTIVE;
            break;
        case SPI_HARDWARE_CHIP_SELECT_PIN_5:
            mappedHardwareChipSelectPin = SPI_HARDWARE_CHIP_SELECT_5_ACTIVE;
            break;
        default:                  /* something went really wrong */
            FAS_ASSERT(FAS_TRAP); /* LCOV_EXCL_LINE */
            break;                /* LCOV_EXCL_LINE */
    }
    return mappedHardwareChipSelectPin;
}
static uint8_t SPI_GetChipSelectPin(SPI_CHIP_SELECT_TYPE_e chipSelectType, uint32_t chipSelectPin) {
    FAS_ASSERT((chipSelectType == SPI_CHIP_SELECT_HARDWARE) || (chipSelectType == SPI_CHIP_SELECT_SOFTWARE));
    FAS_ASSERT(chipSelectPin <= MCU_LARGEST_PIN_NUMBER);

    uint8_t mappedChipSelectPin = (uint8_t)chipSelectPin;
    /* for software chips select, there is nothing more to sanitize or map, therefore it is just needed to further
       analyze the hardware chip select setting */
    if (chipSelectType == SPI_CHIP_SELECT_HARDWARE) {
        mappedChipSelectPin = SPI_GetHardwareChipSelectPin(mappedChipSelectPin);
    }

    return mappedChipSelectPin;
}
static void SPI_InitializeChipSelects(void) {
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        spi_ltcInterface[s].pConfig->CSNR = SPI_GetChipSelectPin(spi_ltcInterface[s].csType, spi_ltcInterface[s].csPin);
        spi_nxp775InterfaceTx[s].pConfig->CSNR =
            SPI_GetChipSelectPin(spi_nxp775InterfaceTx[s].csType, spi_nxp775InterfaceTx[s].csPin);
        spi_nxp775InterfaceRx[s].pConfig->CSNR =
            SPI_GetChipSelectPin(spi_nxp775InterfaceRx[s].csType, spi_nxp775InterfaceRx[s].csPin);
    }
    spi_mxmInterface.pConfig->CSNR    = SPI_GetChipSelectPin(spi_mxmInterface.csType, spi_mxmInterface.csPin);
    spi_framInterface.pConfig->CSNR   = SPI_GetChipSelectPin(spi_framInterface.csType, spi_framInterface.csPin);
    spi_spsInterface.pConfig->CSNR    = SPI_GetChipSelectPin(spi_spsInterface.csType, spi_spsInterface.csPin);
    spi_sbcMcuInterface.pConfig->CSNR = SPI_GetChipSelectPin(spi_sbcMcuInterface.csType, spi_sbcMcuInterface.csPin);
}

/*========== Extern Function Implementations ================================*/
extern void SPI_Initialize(void) {
    spiInit();
    SPI_InitializeChipSelects();
}

extern STD_RETURN_TYPE_e SPI_TransmitDummyByte(SPI_INTERFACE_CONFIG_s *pSpiInterface, uint32_t delay) {
    FAS_ASSERT(pSpiInterface != NULL_PTR);
    /* AXIVION Routine Generic-MissingParameterAssert: delay: parameter accepts whole range */

    uint16_t txDummy[1]      = {0x00};
    STD_RETURN_TYPE_e retVal = SPI_TransmitData(pSpiInterface, txDummy, 1u);
    MCU_Delay_us(delay);
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
    const uint8_t spiIndex = SPI_GetSpiIndex(pSpiInterface->pNode);
    FAS_ASSERT(spiIndex < spi_nrBusyFlags);

    STD_RETURN_TYPE_e retVal = STD_NOT_OK;

    OS_EnterTaskCritical();
    /* Lock SPI hardware to prevent concurrent read/write commands */
    if (spi_busyFlags[spiIndex] == SPI_IDLE) {
        spi_busyFlags[spiIndex] = SPI_BUSY;

        /* Check that not SPI transmission over DMA is taking place */
        if ((pSpiInterface->pNode->INT0 & DMAREQEN_BIT) == 0x0) {
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

            spi_txLastWord[spiIndex] = pTxBuff[frameLength - 1u];
            spi_txLastWord[spiIndex] |= ((uint32)DataFormat << SPI_DATA_FORMAT_FIELD_POSITION) |
                                        ((uint32)ChipSelect << SPI_HARDWARE_CHIP_SELECT_FIELD_POSITION) | (WDelay);

            /* Set Tx buffer address */
            dmaRAMREG->PCP[(dmaChannel_t)dma_spiDmaChannels[spiIndex].txChannel].ISADDR =
                (uint32_t)(&pTxBuff[1u]); /* First word sent manually to write configuration in SPIDAT1 register */
            /**
              *  Set number of Tx words to send
              *  Last word sent in ISR to set CSHOLD = 0
              */
            dmaRAMREG->PCP[(dmaChannel_t)dma_spiDmaChannels[spiIndex].txChannel].ITCOUNT =
                ((frameLength - 2u) << 16U) | 1U; /* Last word sent manually to write CSHOLD in SPIDAT1 register */

            /* Set Rx buffer address */
            dmaRAMREG->PCP[(dmaChannel_t)dma_spiDmaChannels[spiIndex].rxChannel].IDADDR = (uint32_t)pRxBuff;
            /* Set number of Rx words to receive */
            dmaRAMREG->PCP[(dmaChannel_t)dma_spiDmaChannels[spiIndex].rxChannel].ITCOUNT = (frameLength << 16U) | 1U;

            /* Re-enable channels; because auto-init is disabled */
            /* Disable otherwise transmission  is constantly ongoing */
            dmaSetChEnable((dmaChannel_t)dma_spiDmaChannels[spiIndex].txChannel, (dmaTriggerType_t)DMA_HW);
            dmaSetChEnable((dmaChannel_t)dma_spiDmaChannels[spiIndex].rxChannel, (dmaTriggerType_t)DMA_HW);

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
    /* AXIVION Routine Generic-MissingParameterAssert: spi: parameter accepts whole range */

    STD_RETURN_TYPE_e retVal = STD_NOT_OK;

    OS_EnterTaskCritical();
    if ((spi < spi_nrBusyFlags) && (spi_busyFlags[spi] == SPI_IDLE)) {
        spi_busyFlags[spi] = SPI_BUSY;
        retVal             = STD_OK;
    } else {
        retVal = STD_NOT_OK;
    }
    OS_ExitTaskCritical();

    return retVal;
}

extern void SPI_Unlock(uint8_t spi) {
    /* AXIVION Routine Generic-MissingParameterAssert: spi: parameter accepts whole range */

    OS_EnterTaskCritical();
    if (spi < spi_nrBusyFlags) {
        spi_busyFlags[spi] = SPI_IDLE;
    }
    OS_ExitTaskCritical();
}

extern void SPI_SetFunctional(spiBASE_t *pNode, enum spiPinSelect bit, bool hardwareControlled) {
    FAS_ASSERT(pNode != NULL_PTR);
    FAS_ASSERT(bit <= (enum spiPinSelect)MCU_LARGEST_PIN_NUMBER);

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

    /* Write FMT configuration in DAT1; as SPI is configured as slave, this does not provoke a transmission */
    SPIDATAFMT_t DataFormat = pSpiInterface->pConfig->DFSEL;
    pSpiInterface->pNode->DAT1 |= ((uint32)DataFormat << SPI_DATA_FORMAT_FIELD_POSITION);

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
    /* Disable otherwise transmission is constantly on going */
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

/* AXIVION Next Codeline Style Linker-Multiple_Definition: TI HAL only provides a weak implementation */
/* Doxygen comment needs to be here, as this is from a TI generated HAL header */
/**
 * @brief   SPI Interrupt callback
 * @param[in]   spi     spi device
 * @param       flags   flags to be passed
 */
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
#ifdef UNITY_UNIT_TEST
extern void TEST_SPI_InitializeChipSelects(void) {
    return SPI_InitializeChipSelects();
}
extern uint8_t TEST_SPI_GetChipSelectPin(SPI_CHIP_SELECT_TYPE_e csType, uint32_t csPin) {
    return SPI_GetChipSelectPin(csType, csPin);
}
extern uint8_t TEST_SPI_GetHardwareChipSelectPin(uint8_t csPin) {
    return SPI_GetHardwareChipSelectPin(csPin);
}
#endif
