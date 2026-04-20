/**
 *
 * @copyright &copy; 2010 - 2026, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * - "This product uses parts of foxBMS&reg;"
 * - "This product includes parts of foxBMS&reg;"
 * - "This product is derived from foxBMS&reg;"
 *
 */

/**
 * @file    Networkinterface.c
 * @author  foxBMS Team
 * @date    2025-04-02 (date of creation)
 * @updated 2026-04-20 (date of last update)
 * @version v1.11.0
 * @ingroup SOME_GROUP
 * @prefix  NIC
 *
 * @brief   Implementation of some software
 * @details Some detailed explanation
 */

/*========== Includes =======================================================*/
#include "FreeRTOS.h"
#include "FreeRTOSIPConfig.h"
#include "FreeRTOS_IP.h"

#include "database.h"
#include "emac.h"
#include "fassert.h"
#include "infinite-loop-helper.h"
#include "list.h"

/*========== Macros and Definitions =========================================*/
/* If ipconfigETHERNET_DRIVER_FILTERS_FRAME_TYPES is set to 1, then the Ethernet
 * driver will filter incoming packets and only pass the stack those packets it
 * considers need processing. */
#if ((ipconfigETHERNET_DRIVER_FILTERS_FRAME_TYPES == 0) || defined(COMPILE_FOR_UNIT_TEST))
#define NIC_CONSIDER_FRAME_FOR_PROCESSING(pucEthernetBuffer) eProcessBuffer
#else
#define NIC_CONSIDER_FRAME_FOR_PROCESSING(pucEthernetBuffer) eConsiderFrameForProcessing((pucEthernetBuffer))
#endif

#define BUFFER_SIZE         (ipTOTAL_ETHERNET_FRAME_SIZE + ipBUFFER_PADDING)
#define BUFFER_SIZE_ROUNDED ((BUFFER_SIZE + 7u) & ~0x0007u)

#define NIC_ETH_MIN_FRAME_BYTES (60u)

/*========== Static Constant and Variable Definitions =======================*/
/** Pointer to struct that holds driver data for the network interface
    (EMAC + PHY) */
#ifndef UNITY_UNIT_TEST
static NetworkInterface_t *nic_pInterface = NULL_PTR;
#else
extern NetworkInterface_t *nic_pInterface;
#endif

/** Separate RAM for FreeRTOS network buffers (both RX copies and TX) */
/* AXIVION Next Codeline IISB-LiteralSuffixesCheck: Content from FreeRTOS file */
static uint8_t nic_networkBuffers[ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS][BUFFER_SIZE_ROUNDED] = {0};
/** Local copy of PHY status database */
static DATA_BLOCK_PHY_s nic_tablePhy = {.header.uniqueId = DATA_BLOCK_ID_PHY};

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/
/**
 * @brief Format networkBufferDescriptor to pBuffer format for EMAC.
 *
 * @param networkBufferDescriptor FreeRTOS network buffer
 * @param pBuffer EMAC network buffer
 */
static void NIC_FormatBuffer(
    NetworkBufferDescriptor_t const *networkBufferDescriptor,
    EMAC_PACKET_BUFFER_s *const pBuffer);

/* AXIVION Next Codeline Style MisraC2012-1.2: Keep FreeRTOS naming */
/* AXIVION Next Codeline CodingStyle-Naming.Function: Keep FreeRTOS naming */
/**
 * @brief Initialize hardware.
 *
 * @param pxInterface Struct containing network interface access functions
 * @return BaseType_t  returns pdPASS if the initialisation was successful,
 * and returns pdFAIL if the initialisation fails.
 */
static BaseType_t xNetworkInterfaceInitialise(struct xNetworkInterface *pxInterface);

/* AXIVION Next Codeline Style MisraC2012-1.2: Keep FreeRTOS naming */
/* AXIVION Next Codeline CodingStyle-Naming.Function: Keep FreeRTOS naming */
/**
 * @brief Prepares and transmits data from the tcp stack to the EMAC driver.
 *
 * @param pxDescriptor  Struct containing network interface access functions
 * @param pxNetworkBuffer FreeRTOS network buffer
 * @param xReleaseAfterSend Free network buffer after send
 * @return BaseType_t
 */
static BaseType_t xNetworkInterfaceOutput(
    struct xNetworkInterface *pxDescriptor,
    NetworkBufferDescriptor_t *const pxNetworkBuffer,
    BaseType_t xReleaseAfterSend);

/**
 * @brief Transfers the received ethernet packet to the tcp-stack.
 * @details The emac network buffer descriptor is transfered to the freertos
 * network buffer descriptor.
 */
static void NIC_TransferToTcp(void);

/**
 * @brief Processes the received ethernet packet.
 */
static void NIC_ProcessRxPacket(void);

/*========== Static Function Implementations ================================*/

static void NIC_FormatBuffer(
    NetworkBufferDescriptor_t const *networkBufferDescriptor,
    EMAC_PACKET_BUFFER_s *const pBuffer) {
    FAS_ASSERT(networkBufferDescriptor != NULL_PTR);
    FAS_ASSERT(pBuffer != NULL_PTR);

    uint16_t bufferLength = (uint16_t)networkBufferDescriptor->xDataLength;
    /* Check lenght to prevent overflows */
    FAS_ASSERT(bufferLength <= (uint16_t)BUFFER_SIZE_ROUNDED);

    /* Copy data from FreeRTOS-Buffer to EMAC-TX-Puffer */
    (void)memcpy(pBuffer->pPayload, networkBufferDescriptor->pucEthernetBuffer, bufferLength);

    /* Do some padding if the packet is too small */
    if (bufferLength < NIC_ETH_MIN_FRAME_BYTES) {
        memset((pBuffer->pPayload + bufferLength), 0, (60 - bufferLength));
        bufferLength = NIC_ETH_MIN_FRAME_BYTES;
    }

    /* Currently only single packet messages are supportet.
  This means total length and packet length are the same  */
    pBuffer->totalLength = bufferLength;
    pBuffer->length      = bufferLength;
}

/* AXIVION Next Codeline Style MisraC2012-1.2: Keep FreeRTOS naming */
/* AXIVION Next Codeline CodingStyle-Naming.Function: Keep FreeRTOS naming */
static BaseType_t xNetworkInterfaceInitialise(struct xNetworkInterface *pxInterface) {
    FAS_ASSERT(pxInterface != NULL_PTR);

    BaseType_t xReturn = pdPASS;
    /* Call the EMAC hardware initialize and with that included the PHY initialize function */
    if (EMAC_InitializeHardware(pxInterface->pxEndPoint->xMACAddress.ucBytes) != EMAC_OK) {
        xReturn = pdFAIL;
    }

    return xReturn;
}

/* AXIVION Next Codeline Style MisraC2012-1.2: Keep FreeRTOS naming */
/* AXIVION Next Codeline CodingStyle-Naming.Function: Keep FreeRTOS naming */
static BaseType_t xNetworkInterfaceOutput(
    struct xNetworkInterface *pxDescriptor,
    NetworkBufferDescriptor_t *const pxNetworkBuffer,
    BaseType_t xReleaseAfterSend) {
    FAS_ASSERT(pxDescriptor != NULL_PTR);
    FAS_ASSERT(pxNetworkBuffer != NULL_PTR);
    FAS_ASSERT((xReleaseAfterSend == pdTRUE) || (xReleaseAfterSend == pdFALSE));

    /* As we are currently using only one network interface, we dont need
   * pxDescriptor */
    (void)pxDescriptor;

    EMAC_PACKET_BUFFER_s *pBuffer = EMAC_GetNextTxDescriptor();

    NIC_FormatBuffer(pxNetworkBuffer, pBuffer);
    BaseType_t transmissionSuccess = (BaseType_t)EMAC_Transmit(pBuffer);

    if (transmissionSuccess == pdTRUE) {
        iptraceNETWORK_INTERFACE_TRANSMIT((unsigned)pxNetworkBuffer->xDataLength);
    }

    if (xReleaseAfterSend == pdTRUE) {
        /* EMACTansmit copies the data from the buffer. The Ethernet buffer is
       therefore no longer needed, and must be freed for re-use. */
        vReleaseNetworkBufferAndDescriptor(pxNetworkBuffer);
    }

    return transmissionSuccess;
}

static void NIC_TransferToTcp(void) {
    /* See how much data was received */
    uint32_t xBytesReceived = EMAC_ReceivedDataLengthInBytes();

    /* Allocate a network buffer descriptor that points to a buffer
    large enough to hold the received frame. */
    NetworkBufferDescriptor_t *pBufferDescriptor = pxGetNetworkBufferWithDescriptor(xBytesReceived, 0);

    if (pBufferDescriptor != NULL) {
        /* pBufferDescriptor->pucEthernetBuffer now points to an Ethernet
        buffer large enough to hold the received data. Now the FreeRTOS buffer
        descriptor has point to the data stored in the EMAC buffer descriptor. */
        volatile EMAC_RX_BUFFER_DESCRIPTOR_s const *pCurrentBufferDescriptor = EMAC_GetRxActiveHead();
        uint8_t *pEmacEthernetBuffer = EMAC_GetEthernetBuffer(pCurrentBufferDescriptor);

        /* Safety: ensure we don't copy beyond our buffer */
        /* AXIVION Next Codeline IISB-LiteralSuffixesCheck: Content from FreeRTOS file */
        FAS_ASSERT(xBytesReceived <= ipTOTAL_ETHERNET_FRAME_SIZE);

        /* Copy from EMAC RX buffer into the FreeRTOS buffer */
        memcpy(pBufferDescriptor->pucEthernetBuffer, pEmacEthernetBuffer, xBytesReceived);

        /* Tell the stack where and how much data */
        pBufferDescriptor->xDataLength = xBytesReceived;
        pBufferDescriptor->pxInterface = nic_pInterface;
        pBufferDescriptor->pxEndPoint = FreeRTOS_MatchingEndpoint(nic_pInterface, pBufferDescriptor->pucEthernetBuffer);

        if (pBufferDescriptor->pxEndPoint != NULL) {
            /* The event about to be sent to the TCP/IP is an Rx event. */
            IPStackEvent_t xRxEvent = {
                .eEventType = eNetworkRxEvent,
                /* pvData is used to point to the network buffer descriptor that now references the received data. */
                .pvData = (void *)pBufferDescriptor};

            /* Send the data to the TCP/IP stack. */
            if (xSendEventStructToIPTask(&xRxEvent, 0) == pdFALSE) {
                /* The buffer could not be sent to the IP task so the buffer must be released. */
                vReleaseNetworkBufferAndDescriptor(pBufferDescriptor);
                /* Make a call to the standard trace macro to log the occurrence. */
                iptraceETHERNET_RX_EVENT_LOST();
            } else {
                /* The message was successfully sent to the TCP/IP stack.
                    Call the standard trace macro to log the occurrence. */
                iptraceNETWORK_INTERFACE_RECEIVE();
            }
        } else {
            /* No matching endpoint was found. */
            vReleaseNetworkBufferAndDescriptor(pBufferDescriptor);
        }
    } else {
        /* The event was lost because a network buffer was not available.
        Call the standard trace macro to log the occurrence. */
        iptraceFAILED_TO_OBTAIN_NETWORK_BUFFER();
        /* The Ethernet frame can be dropped, nothing to free here */
    }
}

static void NIC_ProcessRxPacket(void) {
    /* Get the buffer descriptors which contain the earliest filled data */
    volatile EMAC_RX_BUFFER_DESCRIPTOR_s *currentBufferDescriptor = EMAC_GetRxActiveHead();
    volatile EMAC_RX_BUFFER_DESCRIPTOR_s *lastBufferDescriptor    = EMAC_GetRxActiveTail();

    /* Start processing once the packet is loaded. Ownership flag is reset */
    if (EMAC_EmacIsOwner(currentBufferDescriptor) == false) {

        if (NIC_CONSIDER_FRAME_FOR_PROCESSING(EMAC_GetEthernetBuffer(currentBufferDescriptor)) == eProcessBuffer) {
            /* This buffer descriptor chain will be freed after processing. Save start of the chain internally */
            volatile EMAC_RX_BUFFER_DESCRIPTOR_s *firstBufferDescriptor = currentBufferDescriptor;

            /* Currently the buffers are statically allocated by the network driver in compile time,
                        at the maximal size a packet can have 1500 bytes + padding. So we do not consider a fragmented
                        frame over more than one buffer. */
            NIC_TransferToTcp();

            /* Currently only accepting single descriptor messages */
            /* TODO: Chain multiple receive buffers here to accept fragmented packages. */

            /* Save last buffer descriptor */
            lastBufferDescriptor = currentBufferDescriptor;
            EMAC_UpdateLinkedList(currentBufferDescriptor);

            /* Acknowledge that this packet is processed */
            EMAC_AcknowledgePacket(lastBufferDescriptor);

            /* The processed descriptor is now the tail of the linked list.
             * Link it to the previous head. */
            EMAC_SetNextPointerOfCurrentTail(firstBufferDescriptor);

            /* The last element in the already processed Rx descriptor chain is now the end of list. */
            lastBufferDescriptor->next = NULL_PTR;
        } else {
            /* Just acknowledge the packet. */
            EMAC_AcknowledgePacket(currentBufferDescriptor);
        }
        EMAC_EndOfReception(lastBufferDescriptor);
    }
    /* Acknowledge the EMAC Core Rx Interrupts */
    EMAC_AcknowledgeRxInterrupt();
}

/*========== Extern Function Implementations ================================*/

/* AXIVION Next Codeline Style MisraC2012-1.2: Keep FreeRTOS naming */
/* AXIVION Next Codeline CodingStyle-Naming.Function: Keep FreeRTOS naming */
extern NetworkInterface_t *NIC_FillInterfaceDescriptor(BaseType_t xEMACIndex, NetworkInterface_t *pxInterface) {
    FAS_ASSERT(xEMACIndex < (BaseType_t)EMAC_MAX_INSTANCE);
    FAS_ASSERT(pxInterface != NULL_PTR);

    nic_pInterface          = pxInterface;
    pxInterface->pcName     = "TMS570";
    pxInterface->pvArgument = (void *)xEMACIndex;
    /* The initialisation function of this driver. */
    pxInterface->pfInitialise = xNetworkInterfaceInitialise;
    /* The output function of this driver. */
    pxInterface->pfOutput = xNetworkInterfaceOutput;
    /* The query status function of this driver. */
    pxInterface->pfGetPhyLinkStatus = xGetPhyLinkStatus;

    (void)FreeRTOS_AddNetworkInterface(pxInterface);

    return pxInterface;
}

extern void NIC_Receive(void) {
    /* AXIVION Next Line Style MisraC2012-2.2 FaultDetection-DeadBranches: non-blocking driver requires an infinite
     * loop for the driver implementation */
    while (FOREVER()) {
        /* Wait for the Ethernet MAC interrupt to indicate that another packet
         * has been received.  The task notification is used in a similar way
         * to a counting semaphore to count Rx events, but is a lot more
         * efficient than a semaphore. */
        uint32_t ticks          = (EMAC_PHY_LINK_STATUS_RECHECK_MS / OS_TICK_RATE_MS);
        uint32_t nNotifications = OS_NotifyTake(pdFALSE, ticks);
        if (nNotifications > 0u) {
            /* Get the buffer descriptor which contain the earliest filled data */
            volatile EMAC_RX_BUFFER_DESCRIPTOR_s *currentBufferDescriptor = EMAC_GetRxActiveHead();

            /* Process the descriptors as long as data is available
             * when the DMA is receiving data, SOP flag will be set */
            /* Start of new Packet */
            while (EMAC_StartOfPacket(currentBufferDescriptor) == true) {
                NIC_ProcessRxPacket();
            }
        } else {
            /* long time no packet -> check if the connection is still up */
            (void)xGetPhyLinkStatus(nic_pInterface);
        }
    }
}

/* AXIVION Next Codeline CodingStyle-Naming.Function: Keep FreeRTOS naming */
size_t uxNetworkInterfaceAllocateRAMToBuffers(
    NetworkBufferDescriptor_t pNetworkBuffers[ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS]) {
    FAS_ASSERT(pNetworkBuffers != NULL_PTR);

    /* Configure EMAC RAM for rx and tx. Uuses emac_rxBuffers internally only */
    EMAC_InitializeDma();

    /* Provide RAM for the FreeRTOS network buffers independent of EMAC RX RAM */
    /* AXIVION Next Codeline IISB-LiteralSuffixesCheck: Content from FreeRTOS file */
    for (uint32_t i = 0u; i < ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS; i++) {
        pNetworkBuffers[i].pucEthernetBuffer = &(nic_networkBuffers[i][ipBUFFER_PADDING]);
    }

    /* Initialize linked list of buffers to place later the tx messages */
    EMAC_InitializeTxBufferDescriptors();

    return BUFFER_SIZE_ROUNDED;
}

/* AXIVION Next Codeline Style MisraC2012-1.2: Keep FreeRTOS naming */
/* AXIVION Next Codeline CodingStyle-Naming.Function: Keep FreeRTOS naming */
BaseType_t xGetPhyLinkStatus(struct xNetworkInterface *pxInterface) {
    FAS_ASSERT(pxInterface != NULL_PTR);

    (void)DATA_READ_DATA(&nic_tablePhy);
    STD_RETURN_TYPE_e phyLinked = EMAC_GetPhyLinkStatus();
    bool lostConnection         = (phyLinked != STD_OK) && (nic_tablePhy.initialized == true);

    if (lostConnection == true) {
        FreeRTOS_printf(("NetworkInterface: Ethernet link is down"));
        /* Notify the IP stack that the network is down. The IP stack calls #xNetworkInterfaceInitialise() when
         * it receives eNetworkDownEvent event. */
        IPStackEvent_t rxEvent = {
            .eEventType = eNetworkDownEvent,
            .pvData     = (void *)pxInterface,
        };
        (void)xSendEventStructToIPTask(&rxEvent, (TickType_t)0);
        return pdFALSE;
    } else {
        return pdTRUE;
    }
}

/**
 * In the older version of the tcp stack the interface descriptors where initialized in #FreeRTOS_IPInit().
 * Now it has to be initialized separatly.
 * This happens in #ETH_Initialize().
 */
#if (ipconfigIPv4_BACKWARD_COMPATIBLE == 1)
NetworkInterface_t *pxFillInterfaceDescriptor(BaseType_t xEMACIndex, NetworkInterface_t *pxInterface) {
    return NIC_FillInterfaceDescriptor(xEMACIndex, pxInterface);
}
#endif

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
/* function pointer for the actual static functions used in NIC_FillInterfaceDescriptor */
BaseType_t (*pNetworkInterfaceInitialise)(struct xNetworkInterface *pxInterface) = xNetworkInterfaceInitialise;
BaseType_t (*pNetworkInterfaceOutput)(
    struct xNetworkInterface *pxDescriptor,
    NetworkBufferDescriptor_t *const pxNetworkBuffer,
    BaseType_t xReleaseAfterSend) = xNetworkInterfaceOutput;
void TEST_NIC_FormatBuffer(
    NetworkBufferDescriptor_t const *networkBufferDescriptor,
    EMAC_PACKET_BUFFER_s *const pBuffer) {
    NIC_FormatBuffer(networkBufferDescriptor, pBuffer);
}
BaseType_t TEST_xNetworkInterfaceInitialise(struct xNetworkInterface *pxInterface) {
    return xNetworkInterfaceInitialise(pxInterface);
}
BaseType_t TEST_xNetworkInterfaceOutput(
    struct xNetworkInterface *pxDescriptor,
    NetworkBufferDescriptor_t *const pxNetworkBuffer,
    BaseType_t xReleaseAfterSend) {
    return xNetworkInterfaceOutput(pxDescriptor, pxNetworkBuffer, xReleaseAfterSend);
}
void TEST_NIC_ProcessRxPacket(void) {
    NIC_ProcessRxPacket();
}
void TEST_NIC_TransferToTcp(void) {
    NIC_TransferToTcp();
}
#endif
