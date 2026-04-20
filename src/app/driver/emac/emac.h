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
 * @file    emac.h
 * @date    2024-09-23 (date of creation)
 * @updated 2026-04-20 (date of last update)
 * @version v1.11.0
 * @ingroup DRIVERS
 * @prefix  EMAC
 *
 * @brief   Implementation of emac driver
 * @details Implements the handling of the ethernet packets.
 */

#ifndef FOXBMS__EMAC_H_
#define FOXBMS__EMAC_H_

/*
 * The origin of this header is the file 'HL_emac.h' created by HALCoGen
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

/*========== Includes =======================================================*/

#include <stdbool.h>

#if (defined(FOXBMS_TCP_SUPPORT) && (FOXBMS_TCP_SUPPORT == 1))
#include "FreeRTOS_IP.h"
#endif
#include "emac_cfg.h"

#include "fstd_types.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/** Possible return values for EMAC functions */
typedef enum {
    EMAC_OK,            /*!< No error, everything OK. */
    EMAC_ERROR_CONNECT, /*!< Not connected.  */
    EMAC_ERROR_PHY_INITIALIZATION,
} EMAC_RETURN_TYPE_e;

/** EMAC TX Buffer descriptor data structure - Refer TRM for details about the
 *  buffer descriptor structure.
 */
typedef struct EmacTxBufferDescriptor *pEmacTxBufferDescriptor;

/** EMAC RX Buffer descriptor data structure - Refer TRM for details about the
 *  buffer descriptor structure.
 */
typedef struct EmacRxBufferDescriptor {
    volatile struct EmacRxBufferDescriptor
        *next;                               /*Used as a pointer for next element in the linked list of descriptors.*/
    volatile uint32_t pBuffer;               /*Pointer to the actual Buffer which will store the received data.*/
    volatile uint32_t bufferOffsetAndLength; /*Buffer Offset and Buffer Length (16 bits each)*/
    volatile uint32_t flagsAndPacketLength;  /*Status flags and Packet Length. (16 bits each)*/
} EMAC_RX_BUFFER_DESCRIPTOR_s;

/** Helper struct to hold the data used to operate on a particular
 *  receive channel
 */
typedef struct {
    volatile EMAC_RX_BUFFER_DESCRIPTOR_s
        *pFreeHead; /*Used to point to the free buffer descriptor which can receive new data.*/
    volatile EMAC_RX_BUFFER_DESCRIPTOR_s
        *pActiveHead; /*Used to point to the active descriptor in the chain which is receiving.*/
    volatile EMAC_RX_BUFFER_DESCRIPTOR_s *pActiveTail; /*Used to point to the last descriptor in the chain.*/
} EMAC_RX_CHANNEL_s;

/** Helper struct to hold the data used to operate on a particular transmit
 * channel
 */
typedef struct {
    volatile pEmacTxBufferDescriptor
        pFreeHead; /*Used to point to the free buffer descriptor which can transmit new data.*/
    volatile pEmacTxBufferDescriptor
        pNextBufferDescriptorToProcess;           /*Used to point to the next descriptor in the chain to be processed.*/
    volatile pEmacTxBufferDescriptor pActiveTail; /*Used to point to the last descriptor in the chain.*/
} EMAC_TX_CHANNEL_s;

/** Helper struct to hold private data used to operate the ethernet interface */
typedef struct Hdkif {
    /* MAC Address of the Module. */
    uint8_t macAddress[EMAC_HARDWARE_ADDRESS_LENGTH];

    /* EMAC base address */
    uintptr_t emacBaseAddress;

    /* EMAC controller base address */
    volatile uintptr_t emacControllerBaseAddress;
    volatile uintptr_t emacControllerRamAddress;

    /* MDIO base address */
    volatile uintptr_t mdioBaseAddress;

    /* phy parameters for this instance - for future use */
    uint32_t phyAddress;
    STD_RETURN_TYPE_e (*pPhyAutonegotiate)(uint32_t param1, uint32_t param2, uint16_t param3);
    bool (*pPhyPartnerAbility)(uint32_t param4, uint32_t param5, uint16_t *pParam6);

    /* The tx/rx channels for the interface */
    EMAC_TX_CHANNEL_s pTxChannel;
    EMAC_RX_CHANNEL_s pRxChannel;
} EMAC_HDKIF_s;

/** Struct used to take packet data input from the user for transmit APIs */
typedef struct PacketBuffer {
    /** next pBuffer in singly linked pBuffer chain */
    struct PacketBuffer *next;

    /**
     * Pointer to the actual ethernet packet/packet fragment to be transmitted.
     * The packet needs to be in the following format:
     * |Destination MAC Address (6 bytes)| Source MAC Address (6 bytes)| Length/Type (2 bytes)| Data (46- 1500 bytes)
     * The data can be split up over multiple pbufs which are linked as a linked list.
     **/
    uint8_t *pPayload;

    /**
     * total length of this buffer and all next buffers in chain
     * belonging to the same packet.
     *
     * For non-queue packet chains this is the invariant:
     * p->totalLength == p->length + (p->next? p->next->totalLength: 0)
     */
    uint16_t totalLength;

    /** length of this buffer */
    uint16_t length;
} EMAC_PACKET_BUFFER_s;

/*****************************************************************************/

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/
/**
 * @brief Acknowledges the reception of an ethernet packet.
 * @param pBufferDescriptor buffer descriptor to acknowledge.
 */
extern void EMAC_AcknowledgePacket(volatile EMAC_RX_BUFFER_DESCRIPTOR_s *pBufferDescriptor);
/**
 * @brief Update the reception channel after processing a packet.
 * @param pBufferDescriptor last buffer descriptor of the packet-
 */
extern void EMAC_UpdateRxChannel(volatile EMAC_RX_BUFFER_DESCRIPTOR_s *pBufferDescriptor);
/**
 * @brief Checks if the reception has ended and acknowledges this
 * @param pBufferDescriptor  last buffer descriptor of reception.
 */
extern void EMAC_EndOfReception(volatile EMAC_RX_BUFFER_DESCRIPTOR_s *pBufferDescriptor);
/**
 * @brief Returns the owner status of the buffer descriptor.
 * @details The buffer descriptor must only accessed if the EMAC is *not* owner.
 * @param pBufferDescriptor buffer descriptor to check
 * @return Returns true if the EMAC is owner.
 */
extern bool EMAC_EmacIsOwner(volatile EMAC_RX_BUFFER_DESCRIPTOR_s *pBufferDescriptor);
/**
 * @brief Updates flags of buffer descriptors and links list anew.
 * @param pBufferDescriptor last processed buffer descriptor.
 */
extern void EMAC_UpdateLinkedList(volatile EMAC_RX_BUFFER_DESCRIPTOR_s *pBufferDescriptor);
/**
 * @brief Reads out the start of packet flag of a buffer descriptor
 * @param pBufferDescriptor
 * @return Returns true if buffer descriptor marks start of a packet.
 */
extern bool EMAC_StartOfPacket(volatile EMAC_RX_BUFFER_DESCRIPTOR_s *const pBufferDescriptor);
/**
 * @brief Reads the link status of the PHY.
 * @return Returns STD_OK if PHY is linked
 */
extern STD_RETURN_TYPE_e EMAC_GetPhyLinkStatus(void);

/**
 * @brief   Initializes the EMAC hardware including PHY.
 * @param   pEmacAddress MAC Address of the EMAC.
 * @return  STD_OK on success
 */
extern EMAC_RETURN_TYPE_e EMAC_InitializeHardware(uint8_t const *pEmacAddress);

#if (defined(FOXBMS_TCP_SUPPORT) && (FOXBMS_TCP_SUPPORT == 1))
/**
 * @brief   Initialize DMA parts of the EMAC.
 * @details Connect the receive buffers of the EMAC with the
 *          NetworkBufferDescriptor_t struct of the TCP Stack.
 *          Only single channel is supported for both TX and RX.
 * @param   pxNetworkBuffers Network buffer descriptors of the TCP Stack.
 */
extern void EMAC_InitializeDma(void);
#endif

/**
 * @brief   Perform the actual packet transmission.
 * @details The packet is contained in the pDmaDescriptor that is passed to the function.
 *          This pDmaDescriptor might be chained.
 *          That is, one pDmaDescriptor can span more than one tx buffer descriptors.
 * @param   pDmaDescriptor the dmaDescriptor structure which contains the data to be sent using
 *                  EMAC
 * @return  Return false if a Null pointer was passed for transmission and
 *          return true if valid data is sent and transmitted.
 */
extern bool EMAC_Transmit(EMAC_PACKET_BUFFER_s *pDmaDescriptor);

/** @brief  Initialize Tx buffers for the network interface. */
extern void EMAC_InitializeTxBufferDescriptors(void);

/**
 * @brief   Return the pointer to the next tx buffer to process.
 * @return  next Tx buffer pointer
 */
extern EMAC_PACKET_BUFFER_s *EMAC_GetNextTxDescriptor(void);

/**
 * @brief   Process the Tx buffer descriptors.
 * @param   pHdkif interface structure
 */
extern void EMAC_TxInterruptHandler(EMAC_HDKIF_s *pHdkif);

/**
 * @brief   Tx Notification function to which received packets are passed after
 *          processing
 * @param   pHdkif interface structure
 * @note    Naming scheme does not align to style rules, because it is a TI
 *          internal function.
 */
extern void emacTxNotification(EMAC_HDKIF_s *pHdkif);

/**
 * @brief   RX Notification function to which received packets are passed after
 *          processing
 * @param   pHdkif interface structure
 * @note    Naming scheme does not align to style rules, because it is a TI
 *          internal function.
 */
extern void emacRxNotification(EMAC_HDKIF_s *pHdkif);

/**
 * @brief   Returns the active head of the linked list of buffer descriptors
 * @return  rx buffer descriptor
 */
extern volatile EMAC_RX_BUFFER_DESCRIPTOR_s *EMAC_GetRxActiveHead(void);

/**
 * @brief   Returns the active tail of the linked list of buffer descriptors
 * @return  rx buffer descriptor
 */
extern volatile EMAC_RX_BUFFER_DESCRIPTOR_s *EMAC_GetRxActiveTail(void);

/**
 * @brief   Returns the size of the received ethernet packet.
 * @return  Packet size in bytes.
 */
extern uint16_t EMAC_ReceivedDataLengthInBytes(void);

/**
 * @brief   Links the list of buffer descriptors new as it sets the next
 *          pointer of the tail to the first buffer descriptor.
 */
extern void EMAC_SetNextPointerOfCurrentTail(volatile EMAC_RX_BUFFER_DESCRIPTOR_s *firstBufferDescriptor);
/**
 * @brief   Returns the ethernet buffer the buffer descriptor.
 * @details Also takes care of the byte order.
 * @return  Packet size in bytes.
 */
extern uint8_t *EMAC_GetEthernetBuffer(volatile EMAC_RX_BUFFER_DESCRIPTOR_s const *pCurrentBufferDescriptor);
/**
 * @brief   Acknowledge the Rx interrupt for Core0.
 */
extern void EMAC_AcknowledgeRxInterrupt(void);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
uint32_t TEST_EMAC_SetupLink(EMAC_HDKIF_s *pHdkif);
void TEST_EMAC_TxInterruptServiceRoutine(void);
void TEST_EMAC_RxInterruptServiceRoutine(void);
#endif

#endif /* FOXBMS__EMAC_H_ */
