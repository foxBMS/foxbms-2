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
 * @file    emac.c
 * @date    2024-09-23 (date of creation)
 * @updated 2026-04-20 (date of last update)
 * @version v1.11.0
 * @ingroup DRIVERS
 * @prefix  EMAC
 *
 * @brief   Implementation of emac driver
 * @details Some detailed explanation
 */

/*
 * The origin of this header is the file 'HL_emac.c' created by HALCoGen
 * 04.07.01 when selecting tab 'TMS570LC4357ZWT_FreeRTOS' -> 'DRIVER ENABLE'
 * and then choosing the option 'EMAC'.
 * Fraunhofer IISB changed the file in order to support the DP83869 PHY
 * as follows:
 * - Split the driver into low level and high level driver
 * - Change used types to conform to MISRA-C
 * - Use foxBMS style naming conventions (e.g., function names follow the
 *   Verb-Noun-pattern, variable names do **not** use underscore, do not
 *   abbreviate)
 * - Use foxBMS style doxygen comments
 * - Use foxBMS style prefixes
 */

/* cspell:ignore GMII TXHDP MIDO */

/*========== Includes =======================================================*/
#include "foxbms_config.h"

#include "emac.h"

#include "phy_cfg.h"

#include "HL_mdio.h"

#include "dp83869.h"
#include "emac-low-level.h"
#include "foxmath.h"
#include "ftask.h"

/*========== Macros and Definitions =========================================*/
#define EMAC_UNICAST_DISABLE (0xFFu)

#define EMAC_BUFFER_LENGTH_MASK (0x0000FFFFu)

#define EMAC_PACKET_LENGTH_MASK (0x0000FFFFu)

struct EmacTxBufferDescriptor {
    volatile struct EmacTxBufferDescriptor *next;
    volatile uint32_t pBuffer;               /* Pointer to the actual Buffer storing the data to be transmitted. */
    volatile uint32_t bufferOffsetAndLength; /*Buffer Offset and Buffer Length (16 bits each) */
    volatile uint32_t flagsAndPacketLength;  /* Status flags and Packet Length. (16 bits each)*/
} EMAC_TX_BUFFER_DESCRIPTOR_s;

/*========== Static Constant and Variable Definitions =======================*/
/* AXIVION Disable Style IISB-LiteralSuffixesCheck: Content from HALCoGen generated configuration file */
#define BUFFER_SIZE         (ipTOTAL_ETHERNET_FRAME_SIZE + ipBUFFER_PADDING)
#define BUFFER_SIZE_ROUNDED ((BUFFER_SIZE + 7u) & ~0x0007u)
/* AXIVION Disable Style IISB-LiteralSuffixesCheck: */

#ifndef UNITY_UNIT_TEST
static uint8_t emac_rxBuffers[ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS][BUFFER_SIZE_ROUNDED] = {0};

static EMAC_PACKET_BUFFER_s emac_txBuffer[EMAC_MAX_TX_PBUF_ALLOC] = {0};
static EMAC_HDKIF_s emac_hdkifData[EMAC_MAX_INSTANCE];

#else
extern uint8_t emac_rxBuffers[ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS][BUFFER_SIZE_ROUNDED];
extern EMAC_HDKIF_s emac_hdkifData[EMAC_MAX_INSTANCE];
extern EMAC_PACKET_BUFFER_s emac_txBuffer[EMAC_MAX_TX_PBUF_ALLOC];
#endif

static uint8_t emac_txBuffers[EMAC_MAX_TX_PBUF_ALLOC][BUFFER_SIZE_ROUNDED] = {0};

/* Base Addresses */
/**@{*/
/* Addresses are remapped for unit tests to work */
#ifndef UNITY_UNIT_TEST
#define EMAC_CTRL_RAM_0_BASE (0xFC520000u) /*!< Starting Address of the EMAC RAM */
#define EMAC_TX0HDP          (0xFCF78600u) /*!< TX head descriptor pointer */
#else
extern uint32_t EMAC_CTRL_RAM_0_BASE;
extern uint32_t *EMAC_TX0HDP;
#endif
#define EMAC_0_BASE      (0xFCF78000u) /*!< EMAC register base address */
#define EMAC_CTRL_0_BASE (0xFCF78800u) /*!< EMAC control register base address */
#define EMAC_MDIO_0_BASE (0xFCF78900u) /*!< MIDO register base address */
/**@}*/

/* EMAC Flags */
/**@{*/
#define EMAC_BUF_DESC_OWNER \
    (0x20000000u)                       /*!< Ownership of the buffer descriptor.
This flag is cleared by the EMAC once it is finished with a given set of descriptors,
associated with a received packet.*/
#define EMAC_BUF_DESC_SOP (0x80000000u) /*!< Buffer descriptor start of packet */
#define EMAC_BUF_DESC_EOP (0x40000000u) /*!< Buffer descriptor end of packet */
#define EMAC_BUF_DESC_EOQ \
    (0x10000000u) /*!< Buffer descriptor end of queue.
The EOQ flag is set by the EMAC on the last descriptor of a packet when the descriptor?s ?next? pointer is NULL */
/**@}*/

static EMAC_PACKET_BUFFER_s *emac_pFreeTxBufferDescriptor = NULL_PTR;
static EMAC_PACKET_BUFFER_s *emac_pTailTxBufferDescriptor = NULL_PTR;

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/
/**
* @brief    Setup the instance parameters inside the interface
* @param    pHdkif Network interface structure
*/
static void EMAC_ConfigureInstance(EMAC_HDKIF_s *pHdkif);

/**
* @brief    Setup the link.
* @details  AutoNegotiate with the phy for link setup and set the EMAC with the
            result of autonegotiation.
* @param    pHdkif Network interface structure.
* @return   EMAC_OK if everything passed, other values if not passed
*/
static EMAC_RETURN_TYPE_e EMAC_SetupLink(EMAC_HDKIF_s *pHdkif);

/*========== Static Function Implementations ================================*/

static void EMAC_ConfigureInstance(EMAC_HDKIF_s *pHdkif) {
    /* Fill hdkif with its inital values. Remaining pointer are addressed in
    #EMAC_InitializeDma. */
    pHdkif->emacBaseAddress           = EMAC_0_BASE;
    pHdkif->emacControllerBaseAddress = EMAC_CTRL_0_BASE;
    pHdkif->emacControllerRamAddress  = EMAC_CTRL_RAM_0_BASE;
    pHdkif->mdioBaseAddress           = EMAC_MDIO_0_BASE;
    pHdkif->phyAddress                = PHY_ADDRESS;
    pHdkif->pPhyAutonegotiate         = &PHY_AutoNegotiate;
    pHdkif->pPhyPartnerAbility        = &PHY_GetPartnerAbility;
}

static EMAC_RETURN_TYPE_e EMAC_SetupLink(EMAC_HDKIF_s *pHdkif) {
    FAS_ASSERT(pHdkif != NULL_PTR);

    EMAC_RETURN_TYPE_e linkStatus = EMAC_ERROR_CONNECT;
    uint16_t partnerAbility       = 0u;
    uint32_t phyDuplexMode        = EMAC_DUPLEX_HALF;

    if (pHdkif->pPhyAutonegotiate(
            (uint32_t)pHdkif->mdioBaseAddress,
            (uint32_t)pHdkif->phyAddress,
            (uint16_t)((uint16_t)PHY_100BTX | (uint16_t)PHY_100BTX_FD | (uint16_t)PHY_10BT | (uint16_t)PHY_10BT_FD)) ==
        STD_OK) {
        linkStatus = EMAC_OK;
        (void)pHdkif->pPhyPartnerAbility(pHdkif->mdioBaseAddress, pHdkif->phyAddress, &partnerAbility);

        /* Check for 100 Mbps and duplex capability */
        if ((partnerAbility & PHY_100BTX_FD) != 0u) {
            phyDuplexMode = EMAC_DUPLEX_FULL;
        }
    } else {
        linkStatus = EMAC_ERROR_CONNECT;
    }

    /* Set the EMAC with the negotiation results if it is successful */
    if (linkStatus == EMAC_OK) {
        EMAC_SetDuplexMode(pHdkif->emacBaseAddress, phyDuplexMode);
        (void)phyDuplexMode;
    }

    /* Wait for the MII to settle down */
    OS_DelayTask(EMAC_MII_SETTLE_DOWN_TIME_MS);

    return linkStatus;
}

/*========== Extern Function Implementations ================================*/

extern bool EMAC_EmacIsOwner(volatile EMAC_RX_BUFFER_DESCRIPTOR_s *pBufferDescriptor) {
    FAS_ASSERT(pBufferDescriptor != NULL_PTR);
    return (MATH_SwapBytesUint32(pBufferDescriptor->flagsAndPacketLength) & EMAC_BUF_DESC_OWNER) == EMAC_BUF_DESC_OWNER;
}

extern EMAC_RETURN_TYPE_e EMAC_InitializeHardware(uint8_t const *pEmacAddress) {
    FAS_ASSERT(pEmacAddress != NULL_PTR);
    EMAC_HDKIF_s *const pHdkif          = &emac_hdkifData[0u];
    EMAC_RETURN_TYPE_e returnValue      = EMAC_OK;
    EMAC_RX_CHANNEL_s *const pRxChannel = &(pHdkif->pRxChannel);

    EMAC_ConfigureInstance(pHdkif);

    /* Set MAC hardware address */
    for (uint8_t idx = 0u; idx < EMAC_HARDWARE_ADDRESS_LENGTH; idx++) {
        pHdkif->macAddress[idx] = pEmacAddress[(EMAC_HARDWARE_ADDRESS_LENGTH - 1u) - idx];
    }

    /* Initialize the EMAC, EMAC Control and MDIO modules. */
    EMAC_InitializeRegisters(pHdkif->emacControllerBaseAddress, pHdkif->emacBaseAddress);
    MDIOInit(pHdkif->mdioBaseAddress, MDIO_FREQ_INPUT, MDIO_FREQ_OUTPUT);

    /* Set the MAC Addresses in EMAC hardware */

    EMAC_SetMacSourceAddress(pHdkif->emacBaseAddress, pHdkif->macAddress);
    for (uint8_t channel = 0u; channel < EMAC_NUMBER_OF_CHANNELS; channel++) {
        EMAC_SetMacAddress(pHdkif->emacBaseAddress, channel, pHdkif->macAddress, EMAC_MAC_ADDRESS_MATCH);
    }

    if (EMAC_SetupLink(pHdkif) != EMAC_OK) {
        returnValue = EMAC_ERROR_CONNECT;
    }

    /* Acknowledge receive and transmit interrupts for proper interrupt pulsing*/
    EMAC_AcknowledgeControlCoreInterrupt(pHdkif->emacBaseAddress, (uint32_t)EMAC_INT_CORE0_RX);
    EMAC_AcknowledgeControlCoreInterrupt(pHdkif->emacBaseAddress, (uint32_t)EMAC_INT_CORE0_TX);

    /* Enable GMII bit in the MACCONTROL Register */
    EMAC_EnableMii(pHdkif->emacBaseAddress);

    /* Enable Broadcast */
    EMAC_EnableRxBroadCast(pHdkif->emacBaseAddress, (uint32_t)EMAC_CHANNELNUMBER);
    /* Enable Multicast */
    EMAC_EnableRxMultiCast(pHdkif->emacBaseAddress, (uint32_t)EMAC_CHANNELNUMBER);
    /* Enable Unicast */
    EMAC_SetRxUnicast(pHdkif->emacBaseAddress, (uint32_t)EMAC_CHANNELNUMBER);

    /*Enable Full Duplex */
    EMAC_SetDuplexMode(pHdkif->emacBaseAddress, (uint32_t)EMAC_DUPLEX_FULL);

    /* Enable Loopback */
#if (EMAC_LOOPBACK_ENABLE == 1u)
    EMAC_EnableLoopback(pHdkif->emacBaseAddress);
#else
    EMAC_DisableLoopback(pHdkif->emacBaseAddress);
#endif

    /* Enable Transmit and Transmit Interrupt */
    EMAC_EnableTx(pHdkif->emacBaseAddress);
    EMAC_EnableTxInterruptPulse(
        pHdkif->emacBaseAddress,
        pHdkif->emacControllerBaseAddress,
        (uint32_t)EMAC_CHANNELNUMBER,
        (uint32_t)EMAC_CHANNELNUMBER);

    /* Enable Receive and Receive Interrupt. Then start receiving by writing to the HDP register. */
    EMAC_SetNumberFreeBuffer(pHdkif->emacBaseAddress, (uint32_t)EMAC_CHANNELNUMBER, (uint32_t)EMAC_MAX_RX_PBUF_ALLOC);
    EMAC_EnableRx(pHdkif->emacBaseAddress);
    EMAC_EnableRxInterruptPulse(
        pHdkif->emacBaseAddress,
        pHdkif->emacControllerBaseAddress,
        (uint32_t)EMAC_CHANNELNUMBER,
        (uint32_t)EMAC_CHANNELNUMBER);

    /* Write to the RX HDP for channel 0 */
    EMAC_WriteRxHeaderDescriptorPointer(
        pHdkif->emacBaseAddress, (uint32_t)pRxChannel->pActiveHead, (uint32_t)EMAC_CHANNELNUMBER);
    if (PHY_Initialize(pHdkif->mdioBaseAddress) == STD_NOT_OK) {
        returnValue = EMAC_ERROR_PHY_INITIALIZATION;
    }

    return returnValue;
}

extern void EMAC_InitializeDma(void) {
    uint32_t numberBufferDescriptors                                 = 0u;
    EMAC_HDKIF_s *pHdkif                                             = &emac_hdkifData[0u];
    pEmacTxBufferDescriptor pCurrentTxBufferDescriptor               = NULL_PTR;
    pEmacTxBufferDescriptor pLastTxBufferDescriptor                  = NULL_PTR;
    volatile EMAC_RX_BUFFER_DESCRIPTOR_s *pCurrentRxBufferDescriptor = NULL_PTR;
    volatile EMAC_RX_BUFFER_DESCRIPTOR_s *pLastRxBufferDescriptor    = NULL_PTR;
    EMAC_TX_CHANNEL_s *txChannelDma                                  = NULL_PTR;
    EMAC_RX_CHANNEL_s *rxChannelDma                                  = NULL_PTR;
    uint8_t *pBuffer                                                 = NULL_PTR;

    /* Initialize pHdkif if it has not happened before */
    EMAC_ConfigureInstance(pHdkif);

    /*------------- Initialize the Descriptor Memory For TX -------------------------------*/
    txChannelDma                                 = &(pHdkif->pTxChannel);
    txChannelDma->pFreeHead                      = (pEmacTxBufferDescriptor)(pHdkif->emacControllerRamAddress);
    txChannelDma->pNextBufferDescriptorToProcess = txChannelDma->pFreeHead;
    txChannelDma->pActiveTail                    = NULL;

    /* Set current and last buffer descriptor to free head */
    pCurrentTxBufferDescriptor = txChannelDma->pFreeHead;
    pLastTxBufferDescriptor    = pCurrentTxBufferDescriptor;

    /* Initialize half of the EMAC RAM with TX buffer Descriptors */
    numberBufferDescriptors = (EMAC_SIZE_CTRL_RAM >> 1u) / sizeof(EMAC_TX_BUFFER_DESCRIPTOR_s);
    while (numberBufferDescriptors != 0u) {
        /* Next buffer descriptor is incremented struct pointer. */
        pCurrentTxBufferDescriptor->next =
            (pEmacTxBufferDescriptor)MATH_SwapBytesUint32((uint32_t)(pCurrentTxBufferDescriptor + 1u));
        pCurrentTxBufferDescriptor->flagsAndPacketLength  = 0u; /* Set Status flags and Packet Length to 0 */
        pCurrentTxBufferDescriptor->pBuffer               = 0u;
        pCurrentTxBufferDescriptor->bufferOffsetAndLength = 0u;
        pLastTxBufferDescriptor                           = pCurrentTxBufferDescriptor;
        pCurrentTxBufferDescriptor =
            (pEmacTxBufferDescriptor)MATH_SwapBytesUint32((uint32_t)pCurrentTxBufferDescriptor->next);
        numberBufferDescriptors--;
    }

    /* Link last buffer descriptor to first one*/
    pLastTxBufferDescriptor->next = (pEmacTxBufferDescriptor)MATH_SwapBytesUint32((uint32_t)txChannelDma->pFreeHead);

    /*------------- Initialize the Descriptor Memory For RX -------------------------------*/
    /* Save pointer to pHdkif */
    rxChannelDma = &(pHdkif->pRxChannel);
    /* RX buffer is placed right after TX buffer  */
    pCurrentTxBufferDescriptor++;
    rxChannelDma->pActiveHead = (EMAC_RX_BUFFER_DESCRIPTOR_s *)pCurrentTxBufferDescriptor;

    /* Initialize linked list */
    rxChannelDma->pFreeHead    = NULL;
    pCurrentRxBufferDescriptor = (EMAC_RX_BUFFER_DESCRIPTOR_s *)rxChannelDma->pActiveHead;
    pLastRxBufferDescriptor    = pCurrentRxBufferDescriptor;

    uint32_t bufferLength = MATH_SwapBytesUint32(BUFFER_SIZE_ROUNDED);
    uint32_t status       = MATH_SwapBytesUint32(EMAC_BUF_DESC_OWNER);

    for (uint8_t indexNetworkBuffer = 0; indexNetworkBuffer < EMAC_MAX_RX_PBUF_ALLOC; indexNetworkBuffer++) {
        /* AXIVION Next Codeline IISB-LiteralSuffixesCheck:Content from FreeRTOS file */
        pBuffer                                           = &emac_rxBuffers[indexNetworkBuffer][ipBUFFER_PADDING];
        pCurrentRxBufferDescriptor->pBuffer               = (uint32_t)MATH_SwapBytesUint32((uint32_t)pBuffer);
        pCurrentRxBufferDescriptor->bufferOffsetAndLength = bufferLength;
        pCurrentRxBufferDescriptor->flagsAndPacketLength  = status;

        if (indexNetworkBuffer == (EMAC_MAX_RX_PBUF_ALLOC - 1u)) {
            /* last buffer descriptor */
            pCurrentRxBufferDescriptor->next = NULL;
            pLastRxBufferDescriptor          = pCurrentRxBufferDescriptor;
        } else {
            pCurrentRxBufferDescriptor->next =
                (EMAC_RX_BUFFER_DESCRIPTOR_s *)MATH_SwapBytesUint32((uint32_t)(pCurrentRxBufferDescriptor + 1u));
            pCurrentRxBufferDescriptor++;
            pLastRxBufferDescriptor = pCurrentRxBufferDescriptor;
        }
    }
    pLastRxBufferDescriptor->next = NULL;
    rxChannelDma->pActiveTail     = pLastRxBufferDescriptor;
}

extern bool EMAC_Transmit(EMAC_PACKET_BUFFER_s *pDmaDescriptor) {
    FAS_ASSERT(pDmaDescriptor != NULL_PTR);
    EMAC_HDKIF_s *pHdkif = &emac_hdkifData[0u];
    bool retValue        = FALSE;

    if ((pHdkif != NULL)) {
        EMAC_TX_CHANNEL_s *const pTxChannel       = &(pHdkif->pTxChannel);
        uint16_t txBufferDescriptorLength         = 0u;
        EMAC_PACKET_BUFFER_s *pTxBufferDescriptor = pDmaDescriptor;

        /* Get the buffer descriptor which is free to transmit */
        volatile pEmacTxBufferDescriptor pCurrentTxBufferDescriptor = pTxChannel->pFreeHead;
        volatile pEmacTxBufferDescriptor pLastBufferDescriptor      = pCurrentTxBufferDescriptor;
        volatile pEmacTxBufferDescriptor pActiveHead                = pCurrentTxBufferDescriptor;

        pCurrentTxBufferDescriptor->flagsAndPacketLength = 0u;

        /* Update the total packet length */
        uint16_t totalPacketLength    = pDmaDescriptor->totalLength;
        uint32_t flagsAndPacketLength = ((uint32_t)(totalPacketLength) | (EMAC_BUF_DESC_SOP | EMAC_BUF_DESC_OWNER));

        /* Indicate the start of the packet */
        pCurrentTxBufferDescriptor->flagsAndPacketLength = MATH_SwapBytesUint32(flagsAndPacketLength);

        while (pTxBufferDescriptor != NULL) {
            /* Initialize the buffer pointer and length. Chain buffer descriptors if necessary */
            pCurrentTxBufferDescriptor->pBuffer = MATH_SwapBytesUint32((uint32_t)(pTxBufferDescriptor->pPayload));
            txBufferDescriptorLength            = (uint16_t)(pTxBufferDescriptor->length);
            pCurrentTxBufferDescriptor->bufferOffsetAndLength =
                (uint32_t)MATH_SwapBytesUint32(((uint32_t)(txBufferDescriptorLength)&EMAC_BUFFER_LENGTH_MASK));
            pLastBufferDescriptor = pCurrentTxBufferDescriptor;
            pCurrentTxBufferDescriptor =
                (pEmacTxBufferDescriptor)MATH_SwapBytesUint32((uint32_t)pCurrentTxBufferDescriptor->next);
            pTxBufferDescriptor = pTxBufferDescriptor->next;
        }

        /* Indicate the end of the packet */
        pLastBufferDescriptor->next = NULL;
        pLastBufferDescriptor->flagsAndPacketLength |= MATH_SwapBytesUint32(EMAC_BUF_DESC_EOP);

        pTxChannel->pFreeHead = pCurrentTxBufferDescriptor;

        /* For the first time, write the HDP with the filled bd */
        if (pTxChannel->pActiveTail == NULL) {
            EMAC_WriteTxHeaderDescriptorPointer(
                pHdkif->emacBaseAddress, (intptr_t)(pActiveHead), (uint32_t)EMAC_CHANNELNUMBER);
        }

        /* Chain the bd's. If the DMA engine, already reached the end of the
         * chain, the EOQ will be set. In that case, the HDP shall be written
         * again. */
        else {
            pCurrentTxBufferDescriptor = pTxChannel->pActiveTail;
            /* Wait for the EOQ bit is set */
            bool endOfQueue =
                EMAC_BUF_DESC_EOQ ==
                (MATH_SwapBytesUint32(pCurrentTxBufferDescriptor->flagsAndPacketLength) & EMAC_BUF_DESC_EOQ);
            while (endOfQueue == false) {
                endOfQueue =
                    EMAC_BUF_DESC_EOQ ==
                    (MATH_SwapBytesUint32(pCurrentTxBufferDescriptor->flagsAndPacketLength) & EMAC_BUF_DESC_EOQ);
            }

            /* Do not write to TXHDP0 until it turns to zero.
             * This happens when the send process is completed. */
            bool sendCompleted = ((uint32_t)0u == *((uint32_t *)EMAC_TX0HDP));
            while (sendCompleted == false) {
                sendCompleted = ((uint32_t)0u == *((uint32_t *)EMAC_TX0HDP));
            }

            pCurrentTxBufferDescriptor->next = (pEmacTxBufferDescriptor)MATH_SwapBytesUint32((uint32_t)pActiveHead);

            endOfQueue = EMAC_BUF_DESC_EOQ ==
                         (MATH_SwapBytesUint32(pCurrentTxBufferDescriptor->flagsAndPacketLength) & EMAC_BUF_DESC_EOQ);
            if (endOfQueue == true) {
                /* Write the Header Descriptor Pointer and start DMA */
                EMAC_WriteTxHeaderDescriptorPointer(
                    pHdkif->emacBaseAddress, (intptr_t)(pActiveHead), (uint32_t)EMAC_CHANNELNUMBER);
            }
        }
        pTxChannel->pActiveTail = pLastBufferDescriptor;
        retValue                = TRUE;
    } else {
        retValue = FALSE;
    }
    return retValue;
}

extern void EMAC_InitializeTxBufferDescriptors(void) {
    emac_pFreeTxBufferDescriptor = &emac_txBuffer[0];

    for (uint8_t bufferIndex = 0u; bufferIndex < EMAC_MAX_TX_PBUF_ALLOC; bufferIndex++) {
        /* Initialize the max number of tx buffers. They are at the beginning empty.*/
        emac_pTailTxBufferDescriptor              = &emac_txBuffer[bufferIndex];
        emac_pTailTxBufferDescriptor->length      = 0u;
        emac_pTailTxBufferDescriptor->totalLength = 0u;
        /* AXIVION Next Codeline IISB-LiteralSuffixesCheck: Content from FreeRTOS file */
        emac_pTailTxBufferDescriptor->pPayload = &emac_txBuffers[bufferIndex][ipBUFFER_PADDING];

        if (bufferIndex == (EMAC_MAX_TX_PBUF_ALLOC - 1u)) {
            /* Last buffer descriptor */
            emac_pTailTxBufferDescriptor->next = NULL_PTR;
        } else {
            /* Connect buffer descriptor to the next in the linked list. */
            emac_pTailTxBufferDescriptor->next = &emac_txBuffer[bufferIndex + 1u];
        }
    }
}

extern EMAC_PACKET_BUFFER_s *EMAC_GetNextTxDescriptor(void) {
    EMAC_PACKET_BUFFER_s *txBufferDescriptor = emac_pFreeTxBufferDescriptor;
    emac_pTailTxBufferDescriptor->next       = emac_pFreeTxBufferDescriptor;
    emac_pFreeTxBufferDescriptor             = emac_pFreeTxBufferDescriptor->next;

    /* Current buffer descriptor is new tail */
    emac_pTailTxBufferDescriptor = txBufferDescriptor;
    txBufferDescriptor->next     = NULL_PTR;
    return txBufferDescriptor;
}

extern bool EMAC_StartOfPacket(volatile EMAC_RX_BUFFER_DESCRIPTOR_s *const pBufferDescriptor) {
    FAS_ASSERT(pBufferDescriptor != NULL_PTR);
    return (MATH_SwapBytesUint32(pBufferDescriptor->flagsAndPacketLength) & EMAC_BUF_DESC_SOP) == EMAC_BUF_DESC_SOP;
}

extern void EMAC_AcknowledgePacket(volatile EMAC_RX_BUFFER_DESCRIPTOR_s *pBufferDescriptor) {
    FAS_ASSERT(pBufferDescriptor != NULL_PTR);
    EMAC_HDKIF_s const *pHdkif = &emac_hdkifData[0u];
    EMAC_WriteRxCompletionPointer(pHdkif->emacBaseAddress, (uint32_t)EMAC_CHANNELNUMBER, (uint32_t)pBufferDescriptor);
}

extern void EMAC_UpdateRxChannel(volatile EMAC_RX_BUFFER_DESCRIPTOR_s *pBufferDescriptor) {
    FAS_ASSERT(pBufferDescriptor != NULL_PTR);
    EMAC_HDKIF_s *pHdkif          = &emac_hdkifData[0u];
    EMAC_RX_CHANNEL_s *pRxChannel = &(pHdkif->pRxChannel);
    pRxChannel->pFreeHead   = (EMAC_RX_BUFFER_DESCRIPTOR_s *)MATH_SwapBytesUint32((uint32_t)pBufferDescriptor->next);
    pRxChannel->pActiveHead = (EMAC_RX_BUFFER_DESCRIPTOR_s *)MATH_SwapBytesUint32((uint32_t)pBufferDescriptor->next);
    pRxChannel->pActiveTail = pBufferDescriptor;
}

extern void EMAC_EndOfReception(volatile EMAC_RX_BUFFER_DESCRIPTOR_s *pBufferDescriptor) {
    FAS_ASSERT(pBufferDescriptor != NULL_PTR);
    EMAC_HDKIF_s *pHdkif          = &emac_hdkifData[0u];
    EMAC_RX_CHANNEL_s *pRxChannel = &(pHdkif->pRxChannel);
    /**
     * Check if the reception has ended. If the EOQ flag is set, the NULL
     * Pointer is taken by the DMA engine. So we need to write the RX HDP
     * with the next descriptor.
     */
    if ((MATH_SwapBytesUint32(pRxChannel->pActiveTail->flagsAndPacketLength) & EMAC_BUF_DESC_EOQ) ==
        EMAC_BUF_DESC_EOQ) {
        EMAC_WriteRxHeaderDescriptorPointer(
            pHdkif->emacBaseAddress, (uint32_t)(pRxChannel->pFreeHead), (uint32_t)EMAC_CHANNELNUMBER);
    }

    /* If the packet was discarded, this has no effect. */
    pRxChannel->pFreeHead   = (EMAC_RX_BUFFER_DESCRIPTOR_s *)MATH_SwapBytesUint32((uint32_t)pBufferDescriptor->next);
    pRxChannel->pActiveTail = pBufferDescriptor;
}

extern void EMAC_UpdateLinkedList(volatile EMAC_RX_BUFFER_DESCRIPTOR_s *pBufferDescriptor) {
    FAS_ASSERT(pBufferDescriptor != NULL_PTR);

    EMAC_HDKIF_s *pHdkif          = &emac_hdkifData[0u];
    EMAC_RX_CHANNEL_s *pRxChannel = &(pHdkif->pRxChannel);
    /* Updating the last descriptor (which contained the EOP flag) */
    pBufferDescriptor->flagsAndPacketLength  = MATH_SwapBytesUint32((uint32_t)EMAC_BUF_DESC_OWNER);
    pBufferDescriptor->bufferOffsetAndLength = MATH_SwapBytesUint32((uint32_t)BUFFER_SIZE_ROUNDED);

    /* Increment buffer descriptor once to point to the next free one */
    pBufferDescriptor = (EMAC_RX_BUFFER_DESCRIPTOR_s *)MATH_SwapBytesUint32((uint32_t)pBufferDescriptor->next);

    /* The next buffer descriptor is the new head of the linked list. */
    pRxChannel->pActiveHead = pBufferDescriptor;
}

extern volatile EMAC_RX_BUFFER_DESCRIPTOR_s *EMAC_GetRxActiveHead(void) {
    EMAC_HDKIF_s const *pHdkif = &emac_hdkifData[0u];
    /* Next buffer pointer is stored in hdkif active head. */
    const EMAC_RX_CHANNEL_s *pRxChannel = &(pHdkif->pRxChannel);
    return pRxChannel->pActiveHead;
}

extern volatile EMAC_RX_BUFFER_DESCRIPTOR_s *EMAC_GetRxActiveTail(void) {
    EMAC_HDKIF_s const *pHdkif = &emac_hdkifData[0u];
    /* Next buffer pointer is stored in hdkif active tail. */
    const EMAC_RX_CHANNEL_s *pRxChannel = &(pHdkif->pRxChannel);
    return pRxChannel->pActiveTail;
}

extern uint16_t EMAC_ReceivedDataLengthInBytes(void) {
    EMAC_HDKIF_s const *pHdkif                                     = &emac_hdkifData[0u];
    const EMAC_RX_CHANNEL_s *pRxChannel                            = &(pHdkif->pRxChannel);
    volatile EMAC_RX_BUFFER_DESCRIPTOR_s *pCurrentBufferDescriptor = pRxChannel->pActiveHead;

    /* Get the total length of the packet.
     * pCurrentBufferDescriptor points to the start of the packet.
     * This works because the interface is called as function. */
    return (uint16_t)(MATH_SwapBytesUint32((uint32_t)pCurrentBufferDescriptor->flagsAndPacketLength) &
                      EMAC_PACKET_LENGTH_MASK);
}

/**
 * @brief   ISR for EMAC Transmit Interrupt.
 * @details Calls another handler function for processing the descriptors.
 */
/* AXIVION Disable Style MisraC2012-1.2: Register ISR */
#pragma CODE_STATE(EMAC_TxInterruptServiceRoutine, 32)
#pragma INTERRUPT(EMAC_TxInterruptServiceRoutine, IRQ)
/* AXIVION Enable Style MisraC2012-1.2: */
void EMAC_TxInterruptServiceRoutine(void) {
    EMAC_HDKIF_s *const pHdkif = &emac_hdkifData[0u];

    EMAC_TxInterruptHandler(pHdkif);
    /* Acknowledge after Interrupt handler is called */
    EMAC_AcknowledgeControlCoreInterrupt(pHdkif->emacBaseAddress, (uint32_t)EMAC_INT_CORE0_TX);
}

/**
 * @brief   ISR for Receive interrupt.
 * @details Calls another function for processing the received packets.
 */
/* AXIVION Disable Style MisraC2012-1.2: Register ISR */
#pragma CODE_STATE(EMAC_RxInterruptServiceRoutine, 32)
#pragma INTERRUPT(EMAC_RxInterruptServiceRoutine, IRQ)
/* AXIVION Enable Style MisraC2012-1.2: */
void EMAC_RxInterruptServiceRoutine(void) {
    /* The xHigherPriorityTaskWoken parameter must be initialized to
     * pdFALSE as it will get set to pdTRUE inside the interrupt safe
     * API function if a context switch is required. */
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    /* Notify the receiving with counting notification */
    OS_NotifyGiveFromIsr(ftsk_taskHandleEmac, &xHigherPriorityTaskWoken);
}

extern void EMAC_TxInterruptHandler(EMAC_HDKIF_s *pHdkif) {
    FAS_ASSERT(pHdkif != NULL_PTR);
    EMAC_TX_CHANNEL_s *pTxChannel                                   = &(pHdkif->pTxChannel);
    volatile pEmacTxBufferDescriptor pNextBufferDescriptorToProcess = pTxChannel->pNextBufferDescriptorToProcess;
    volatile pEmacTxBufferDescriptor pCurrentBufferDescriptor       = pNextBufferDescriptorToProcess;

    /* Check for correct start of packet */
    bool startOfPacket = (MATH_SwapBytesUint32(pCurrentBufferDescriptor->flagsAndPacketLength) & EMAC_BUF_DESC_SOP) ==
                         EMAC_BUF_DESC_SOP;
    while (startOfPacket == true) {
        /* Make sure that the transmission is over */
        bool emacIsOwner = (MATH_SwapBytesUint32(pCurrentBufferDescriptor->flagsAndPacketLength) &
                            EMAC_BUF_DESC_OWNER) == EMAC_BUF_DESC_OWNER;
        while (emacIsOwner == true) {
            emacIsOwner = (MATH_SwapBytesUint32(pCurrentBufferDescriptor->flagsAndPacketLength) &
                           EMAC_BUF_DESC_OWNER) == EMAC_BUF_DESC_OWNER;
        }

        /* Traverse till the end of packet is reached */
        bool endOfPacket = (MATH_SwapBytesUint32(pCurrentBufferDescriptor->flagsAndPacketLength) & EMAC_BUF_DESC_EOP) ==
                           EMAC_BUF_DESC_EOP;
        while (endOfPacket == false) {
            pCurrentBufferDescriptor =
                (pEmacTxBufferDescriptor)MATH_SwapBytesUint32((uint32_t)pCurrentBufferDescriptor->next);
            endOfPacket = (MATH_SwapBytesUint32(pCurrentBufferDescriptor->flagsAndPacketLength) & EMAC_BUF_DESC_EOP) ==
                          EMAC_BUF_DESC_EOP;
        }

        pNextBufferDescriptorToProcess->flagsAndPacketLength &= ~(MATH_SwapBytesUint32(EMAC_BUF_DESC_SOP));

        pCurrentBufferDescriptor->flagsAndPacketLength &= ~(MATH_SwapBytesUint32(EMAC_BUF_DESC_EOP));

        /* If there are no more data transmitted, the next interrupt
         * shall happen with the pbuf associated with the pFreeHead */
        if (pCurrentBufferDescriptor->next == NULL_PTR) {
            pTxChannel->pNextBufferDescriptorToProcess = pTxChannel->pFreeHead;
        } else {
            pTxChannel->pNextBufferDescriptorToProcess =
                (pEmacTxBufferDescriptor)MATH_SwapBytesUint32((uint32_t)pCurrentBufferDescriptor->next);
        }

        /* Acknowledge the EMAC and free the corresponding pbuf */
        EMAC_WriteTxCompletionPointer(
            pHdkif->emacBaseAddress, (uint32_t)EMAC_CHANNELNUMBER, (intptr_t)pCurrentBufferDescriptor);

        pNextBufferDescriptorToProcess = pTxChannel->pNextBufferDescriptorToProcess;
        pCurrentBufferDescriptor       = pNextBufferDescriptorToProcess;
        startOfPacket = (MATH_SwapBytesUint32(pCurrentBufferDescriptor->flagsAndPacketLength) & EMAC_BUF_DESC_SOP) ==
                        EMAC_BUF_DESC_SOP;
    }
}

extern STD_RETURN_TYPE_e EMAC_GetPhyLinkStatus(void) {
    EMAC_HDKIF_s const *pHdkif = &emac_hdkifData[0u];
    /* Wrap the PHY function with the correct address */
    return PHY_GetLinkStatus(pHdkif->mdioBaseAddress, pHdkif->phyAddress);
}

extern void EMAC_SetNextPointerOfCurrentTail(volatile EMAC_RX_BUFFER_DESCRIPTOR_s *pFirstBufferDescriptor) {
    FAS_ASSERT(pFirstBufferDescriptor != NULL_PTR);
    /* The processed descriptor is now the tail of the linked list.
             * Link it to the previous head. */
    volatile EMAC_RX_BUFFER_DESCRIPTOR_s *currentTail = EMAC_GetRxActiveTail();
    currentTail->next = (EMAC_RX_BUFFER_DESCRIPTOR_s *)MATH_SwapBytesUint32((uint32_t)pFirstBufferDescriptor);
}

extern uint8_t *EMAC_GetEthernetBuffer(volatile EMAC_RX_BUFFER_DESCRIPTOR_s const *pCurrentBufferDescriptor) {
    FAS_ASSERT(pCurrentBufferDescriptor != NULL_PTR);
    /* Just extract the buffer pointer and manage the byte order */
    return (uint8_t *)MATH_SwapBytesUint32((uint32_t)pCurrentBufferDescriptor->pBuffer);
}

extern void EMAC_AcknowledgeRxInterrupt(void) {
    EMAC_HDKIF_s const *pHdkif = &emac_hdkifData[0u];
    /* Acknowledge the Rx interrupt for Core0 */
    EMAC_AcknowledgeControlCoreInterrupt(pHdkif->emacBaseAddress, EMAC_INT_CORE0_RX);
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
uint32_t TEST_EMAC_SetupLink(EMAC_HDKIF_s *pHdkif) {
    return EMAC_SetupLink(pHdkif);
}
void TEST_EMAC_TxInterruptServiceRoutine(void) {
    EMAC_TxInterruptServiceRoutine();
}
void TEST_EMAC_RxInterruptServiceRoutine(void) {
    EMAC_RxInterruptServiceRoutine();
}
#endif
