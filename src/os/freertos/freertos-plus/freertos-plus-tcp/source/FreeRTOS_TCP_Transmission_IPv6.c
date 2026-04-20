/*
 * FreeRTOS+TCP V4.3.3
 * Copyright (C) 2022 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://aws.amazon.com/freertos
 * http://www.FreeRTOS.org
 */

/**
 * @file FreeRTOS_TCP_Transmission_IPv6.c
 * @brief Module which prepares the packet to be sent through
 * a socket for FreeRTOS+TCP.
 * It depends on  FreeRTOS_TCP_WIN.c, which handles the TCP windowing
 * schemes.
 *
 * Endianness: in this module all ports and IP addresses are stored in
 * host byte-order, except fields in the IP-packets
 */

/* Standard includes. */
#include <stdint.h>
#include <stdio.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* FreeRTOS+TCP includes. */
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"
#include "FreeRTOS_IP_Private.h"
#include "NetworkInterface.h"
#include "NetworkBufferManagement.h"
#include "FreeRTOSIPConfigDefaults.h"
#include "FreeRTOS_ND.h"

#include "FreeRTOS_TCP_IP.h"
#include "FreeRTOS_TCP_Reception.h"
#include "FreeRTOS_TCP_Transmission.h"
#include "FreeRTOS_TCP_State_Handling.h"
#include "FreeRTOS_TCP_Utils.h"

/* Just make sure the contents doesn't get compiled if TCP is not enabled. */
/* *INDENT-OFF* */
#if( ipconfigUSE_IPv6 != 0 ) && ( ipconfigUSE_TCP == 1 )
/* *INDENT-ON* */

/*------------------------------------------------------------------------*/


/**
 * @brief  Return (or send) a packet to the peer. The data is stored in pxBuffer,
 *         which may either point to a real network buffer or to a TCP socket field
 *         called 'xTCP.xPacket'. A temporary xNetworkBuffer will be used to pass
 *         the data to the NIC.
 *
 * @param[in] pxSocket The socket owning the connection.
 * @param[in] pxDescriptor The network buffer descriptor carrying the packet.
 * @param[in] ulLen Length of the packet being sent.
 * @param[in] xReleaseAfterSend pdTRUE if the ownership of the descriptor is
 *                               transferred to the network interface.
 */
void prvTCPReturnPacket_IPV6( FreeRTOS_Socket_t * pxSocket,
                              NetworkBufferDescriptor_t * pxDescriptor,
                              uint32_t ulLen,
                              BaseType_t xReleaseAfterSend )
{
    TCPPacket_IPv6_t * pxTCPPacket = NULL;
    ProtocolHeaders_t * pxProtocolHeaders = NULL;
    IPHeader_IPv6_t * pxIPHeader = NULL;
    BaseType_t xDoRelease = xReleaseAfterSend;
    EthernetHeader_t * pxEthernetHeader = NULL;
    NetworkBufferDescriptor_t * pxNetworkBuffer = pxDescriptor;
    NetworkBufferDescriptor_t xTempBuffer;
    /* memcpy() helper variables for MISRA Rule 21.15 compliance*/
    MACAddress_t xMACAddress;
    const void * pvCopySource = NULL;
    void * pvCopyDest = NULL;
    const size_t uxIPHeaderSize = ipSIZE_OF_IPv6_HEADER;
    IPv6_Address_t xDestinationIPAddress;

    do
    {
        /* Use do/while to be able to break out of the flow */
        if( ( pxNetworkBuffer == NULL ) && ( pxSocket == NULL ) )
        {
            /* Either 'pxNetworkBuffer' or 'pxSocket' should be defined. */
            break;
        }

        /* For sending, a pseudo network buffer will be used, as explained above. */

        if( pxNetworkBuffer == NULL )
        {
            pxNetworkBuffer = &xTempBuffer;

            ( void ) memset( &xTempBuffer, 0, sizeof( xTempBuffer ) );
            #if ( ipconfigUSE_LINKED_RX_MESSAGES != 0 )
            {
                pxNetworkBuffer->pxNextBuffer = NULL;
            }
            #endif
            pxNetworkBuffer->pucEthernetBuffer = pxSocket->u.xTCP.xPacket.u.ucLastPacket;
            pxNetworkBuffer->xDataLength = sizeof( pxSocket->u.xTCP.xPacket.u.ucLastPacket );
            xDoRelease = pdFALSE;
        }

        #if ( ipconfigZERO_COPY_TX_DRIVER != 0 )
        {
            if( xDoRelease == pdFALSE )
            {
                /* A zero-copy network driver wants to pass the packet buffer
                 * to DMA, so a new buffer must be created. */
                pxNetworkBuffer = pxDuplicateNetworkBufferWithDescriptor( pxNetworkBuffer, ( size_t ) pxNetworkBuffer->xDataLength );

                if( pxNetworkBuffer != NULL )
                {
                    xDoRelease = pdTRUE;
                }
                else
                {
                    FreeRTOS_debug_printf( ( "prvTCPReturnPacket: duplicate failed\n" ) );
                }
            }
        }
        #endif /* ipconfigZERO_COPY_TX_DRIVER */

        #ifndef __COVERITY__
            if( pxNetworkBuffer != NULL ) /* LCOV_EXCL_BR_LINE the 2nd branch will never be reached */
        #endif
        {
            eResolutionLookupResult_t eResult;
            NetworkInterface_t * pxInterface;

            configASSERT( pxNetworkBuffer->pucEthernetBuffer != NULL );

            /* MISRA Ref 11.3.1 [Misaligned access] */
            /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
            /* coverity[misra_c_2012_rule_11_3_violation] */
            pxIPHeader = ( ( IPHeader_IPv6_t * ) &( pxNetworkBuffer->pucEthernetBuffer[ ipSIZE_OF_ETH_HEADER ] ) );

            /* Map the Ethernet buffer onto a TCPPacket_t struct for easy access to the fields. */

            /* MISRA Ref 11.3.1 [Misaligned access] */
            /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
            /* coverity[misra_c_2012_rule_11_3_violation] */
            pxTCPPacket = ( TCPPacket_IPv6_t * ) pxNetworkBuffer->pucEthernetBuffer;
            pxEthernetHeader = ( EthernetHeader_t * ) &( pxTCPPacket->xEthernetHeader );
            /* MISRA Ref 11.3.1 [Misaligned access] */
            /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
            /* coverity[misra_c_2012_rule_11_3_violation] */
            pxProtocolHeaders = ( ProtocolHeaders_t * ) &( pxNetworkBuffer->pucEthernetBuffer[ ipSIZE_OF_ETH_HEADER + uxIPHeaderSize ] );

            if( pxNetworkBuffer->pxEndPoint == NULL )
            {
                prvTCPReturn_SetEndPoint( pxSocket, pxNetworkBuffer, uxIPHeaderSize );

                if( pxNetworkBuffer->pxEndPoint == NULL )
                {
                    if( xDoRelease != pdFALSE )
                    {
                        vReleaseNetworkBufferAndDescriptor( pxNetworkBuffer );
                    }

                    pxNetworkBuffer = NULL;
                    break;
                }
            }

            /* Fill the packet, swapping from- and to-addresses. */
            if( pxSocket != NULL )
            {
                prvTCPReturn_CheckTCPWindow( pxSocket, pxNetworkBuffer, uxIPHeaderSize );
                prvTCPReturn_SetSequenceNumber( pxSocket, pxNetworkBuffer, uxIPHeaderSize, ulLen );
                ( void ) memcpy( pxIPHeader->xDestinationAddress.ucBytes, pxSocket->u.xTCP.xRemoteIP.xIP_IPv6.ucBytes, ipSIZE_OF_IPv6_ADDRESS );
                ( void ) memcpy( pxIPHeader->xSourceAddress.ucBytes, pxNetworkBuffer->pxEndPoint->ipv6_settings.xIPAddress.ucBytes, ipSIZE_OF_IPv6_ADDRESS );
            }
            else
            {
                IPv6_Address_t xTempAddress;

                /* Sending data without a socket, probably replying with a RST flag
                 * Just swap the two sequence numbers. */
                vFlip_32( pxProtocolHeaders->xTCPHeader.ulSequenceNumber, pxProtocolHeaders->xTCPHeader.ulAckNr );
                ( void ) memcpy( xTempAddress.ucBytes, pxIPHeader->xDestinationAddress.ucBytes, ipSIZE_OF_IPv6_ADDRESS );
                ( void ) memcpy( pxIPHeader->xDestinationAddress.ucBytes, pxIPHeader->xSourceAddress.ucBytes, ipSIZE_OF_IPv6_ADDRESS );
                ( void ) memcpy( pxIPHeader->xSourceAddress.ucBytes, xTempAddress.ucBytes, ipSIZE_OF_IPv6_ADDRESS );
            }

            /* In IPv6, the "payload length" does not include the size of the IP-header */
            pxIPHeader->usPayloadLength = FreeRTOS_htons( ulLen - sizeof( IPHeader_IPv6_t ) );

            #if ( ipconfigDRIVER_INCLUDED_TX_IP_CHECKSUM == 0 )
            {
                /* calculate the TCP checksum for an outgoing packet. */
                uint32_t ulTotalLength = ulLen + ipSIZE_OF_ETH_HEADER;
                ( void ) usGenerateProtocolChecksum( ( uint8_t * ) pxNetworkBuffer->pucEthernetBuffer, ulTotalLength, pdTRUE );
            }
            #endif /* ipconfigDRIVER_INCLUDED_TX_IP_CHECKSUM == 0 */

            vFlip_16( pxProtocolHeaders->xTCPHeader.usSourcePort, pxProtocolHeaders->xTCPHeader.usDestinationPort );

            /* Important: tell NIC driver how many bytes must be sent. */
            pxNetworkBuffer->xDataLength = ( size_t ) ulLen;
            pxNetworkBuffer->xDataLength += ipSIZE_OF_ETH_HEADER;

            #if ( ipconfigUSE_LINKED_RX_MESSAGES != 0 )
            {
                pxNetworkBuffer->pxNextBuffer = NULL;
            }
            #endif

            ( void ) memcpy( xDestinationIPAddress.ucBytes, pxIPHeader->xDestinationAddress.ucBytes, ipSIZE_OF_IPv6_ADDRESS );

            eResult = eNDGetCacheEntry( &xDestinationIPAddress, &xMACAddress, &( pxNetworkBuffer->pxEndPoint ) );

            if( eResult == eResolutionCacheHit )
            {
                pvCopySource = &xMACAddress;
            }
            else
            {
                pvCopySource = &pxEthernetHeader->xSourceAddress;
            }

            /* Fill in the destination MAC addresses. */
            pvCopyDest = &pxEthernetHeader->xDestinationAddress;
            ( void ) memcpy( pvCopyDest, pvCopySource, sizeof( pxEthernetHeader->xDestinationAddress ) );

            /*
             * Use helper variables for memcpy() to remain
             * compliant with MISRA Rule 21.15.  These should be
             * optimized away.
             */
            pvCopySource = pxNetworkBuffer->pxEndPoint->xMACAddress.ucBytes;
            pvCopyDest = &pxEthernetHeader->xSourceAddress;
            ( void ) memcpy( pvCopyDest, pvCopySource, ( size_t ) ipMAC_ADDRESS_LENGTH_BYTES );

            #if ( ipconfigETHERNET_MINIMUM_PACKET_BYTES > 0 )
            {
                if( pxNetworkBuffer->xDataLength < ( size_t ) ipconfigETHERNET_MINIMUM_PACKET_BYTES )
                {
                    BaseType_t xIndex;

                    for( xIndex = ( BaseType_t ) pxNetworkBuffer->xDataLength; xIndex < ( BaseType_t ) ipconfigETHERNET_MINIMUM_PACKET_BYTES; xIndex++ )
                    {
                        pxNetworkBuffer->pucEthernetBuffer[ xIndex ] = 0U;
                    }

                    pxNetworkBuffer->xDataLength = ( size_t ) ipconfigETHERNET_MINIMUM_PACKET_BYTES;
                }
            }
            #endif /* if( ipconfigETHERNET_MINIMUM_PACKET_BYTES > 0 ) */

            /* Send! */
            iptraceNETWORK_INTERFACE_OUTPUT( pxNetworkBuffer->xDataLength, pxNetworkBuffer->pucEthernetBuffer );

            configASSERT( pxNetworkBuffer->pxEndPoint->pxNetworkInterface != NULL );
            configASSERT( pxNetworkBuffer->pxEndPoint->pxNetworkInterface->pfOutput != NULL );

            pxInterface = pxNetworkBuffer->pxEndPoint->pxNetworkInterface;
            ( void ) pxInterface->pfOutput( pxInterface, pxNetworkBuffer, xDoRelease );

            if( xDoRelease == pdFALSE )
            {
                /* Swap-back some fields, as pxBuffer probably points to a socket field
                 * containing the packet header. */
                vFlip_16( pxTCPPacket->xTCPHeader.usSourcePort, pxTCPPacket->xTCPHeader.usDestinationPort );

                ( void ) memcpy( pxIPHeader->xSourceAddress.ucBytes, pxIPHeader->xDestinationAddress.ucBytes, ipSIZE_OF_IPv6_ADDRESS );
            }
            else
            {
                /* Nothing to do: the buffer has been passed to DMA and will be released after use */
            }
        } /* if( pxNetworkBuffer != NULL ) */
    } while( ipFALSE_BOOL );
}
/*-----------------------------------------------------------*/

/**
 * @brief Let ND look-up the MAC-address of the peer and initialise the first SYN
 *        packet.
 *
 * @param[in] pxSocket The socket owning the TCP connection. The first packet shall
 *               be created in this socket.
 *
 * @return pdTRUE: if the packet was successfully created and the first SYN can be sent.
 *         Else pdFALSE.
 *
 * @note Connecting sockets have a special state: eCONNECT_SYN. In this phase,
 *       the Ethernet address of the target will be found using ND. In case the
 *       target IP address is not within the netmask, the hardware address of the
 *       gateway will be used.
 */
BaseType_t prvTCPPrepareConnect_IPV6( FreeRTOS_Socket_t * pxSocket )
{
    TCPPacket_IPv6_t * pxTCPPacket = NULL;
    IPHeader_IPv6_t * pxIPHeader = NULL;
    eResolutionLookupResult_t eReturned;
    IP_Address_t xRemoteIP;
    MACAddress_t xEthAddress;
    BaseType_t xReturn = pdTRUE;
    uint32_t ulInitialSequenceNumber = 0;
    ProtocolHeaders_t * pxProtocolHeaders = NULL;
    NetworkEndPoint_t * pxEndPoint = NULL;

    #if ( ipconfigHAS_PRINTF != 0 )
    {
        /* Only necessary for nicer logging. */
        ( void ) memset( xEthAddress.ucBytes, 0, sizeof( xEthAddress.ucBytes ) );
    }
    #endif /* ipconfigHAS_PRINTF != 0 */

    ( void ) memset( xRemoteIP.xIP_IPv6.ucBytes, 0, ipSIZE_OF_IPv6_ADDRESS );
    ( void ) memcpy( xRemoteIP.xIP_IPv6.ucBytes, pxSocket->u.xTCP.xRemoteIP.xIP_IPv6.ucBytes, ipSIZE_OF_IPv6_ADDRESS );
    eReturned = eNDGetCacheEntry( &( xRemoteIP.xIP_IPv6 ), &( xEthAddress ), &( pxEndPoint ) );
    FreeRTOS_printf( ( "eNDGetCacheEntry: %d with end-point %p\n", eReturned, ( void * ) pxEndPoint ) );

    if( pxEndPoint != NULL )
    {
        pxSocket->pxEndPoint = pxEndPoint;
    }

    /* MISRA Ref 11.3.1 [Misaligned access] */
    /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
    /* coverity[misra_c_2012_rule_11_3_violation] */
    pxProtocolHeaders = ( ( ProtocolHeaders_t * )
                          &( pxSocket->u.xTCP.xPacket.u.ucLastPacket[ ipSIZE_OF_ETH_HEADER + uxIPHeaderSizeSocket( pxSocket ) ] ) );

    switch( eReturned )
    {
        case eResolutionCacheHit:  /* An ND table lookup found a valid entry. */
            break;                 /* We can now prepare the SYN packet. */

        case eResolutionCacheMiss: /* An ND table lookup did not find a valid entry. */
        case eResolutionFailed:    /* There is no IP address, or an ND is still in progress. */
        default:
            /* Count the number of times it could not find the ND address. */
            pxSocket->u.xTCP.ucRepCount++;

            FreeRTOS_printf( ( "Looking up %pip with%s end-point\n", ( void * ) xRemoteIP.xIP_IPv6.ucBytes, ( pxEndPoint != NULL ) ? "" : "out" ) );

            if( pxEndPoint != NULL )
            {
                size_t uxNeededSize;
                NetworkBufferDescriptor_t * pxNetworkBuffer;

                uxNeededSize = ipSIZE_OF_ETH_HEADER + ipSIZE_OF_IPv6_HEADER + sizeof( ICMPHeader_IPv6_t );
                pxNetworkBuffer = pxGetNetworkBufferWithDescriptor( uxNeededSize, 0U );

                if( pxNetworkBuffer != NULL )
                {
                    pxNetworkBuffer->pxEndPoint = pxEndPoint;
                    vNDSendNeighbourSolicitation( pxNetworkBuffer, &( xRemoteIP.xIP_IPv6 ) );
                }
            }

            xReturn = pdFALSE;
            break;
    }

    if( xReturn != pdFALSE )
    {
        /* Get a difficult-to-predict initial sequence number for this 4-tuple. */
        ulInitialSequenceNumber = ulApplicationGetNextSequenceNumber( pxSocket->xLocalAddress.ulIP_IPv4,
                                                                      pxSocket->usLocalPort,
                                                                      pxSocket->u.xTCP.xRemoteIP.ulIP_IPv4,
                                                                      pxSocket->u.xTCP.usRemotePort );

        /* Check for a random number generation error. */
        if( ulInitialSequenceNumber == 0U )
        {
            xReturn = pdFALSE;
        }
    }

    if( xReturn != pdFALSE )
    {
        /* The MAC-address of the peer (or gateway) has been found,
         * now prepare the initial TCP packet and some fields in the socket. Map
         * the buffer onto the TCPPacket_IPv6_t struct to easily access it's field. */

        /* MISRA Ref 11.3.1 [Misaligned access] */
        /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
        /* coverity[misra_c_2012_rule_11_3_violation] */
        pxTCPPacket = ( ( TCPPacket_IPv6_t * ) pxSocket->u.xTCP.xPacket.u.ucLastPacket );
        pxIPHeader = &( pxTCPPacket->xIPHeader );

        /* reset the retry counter to zero. */
        pxSocket->u.xTCP.ucRepCount = 0U;

        /* And remember that the connect/SYN data are prepared. */
        pxSocket->u.xTCP.bits.bConnPrepared = pdTRUE_UNSIGNED;

        /* Now that the Ethernet address is known, the initial packet can be
         * prepared. */
        ( void ) memset( pxSocket->u.xTCP.xPacket.u.ucLastPacket, 0, sizeof( pxSocket->u.xTCP.xPacket.u.ucLastPacket ) );

        /* Write the Ethernet address in Source, because it will be swapped by
         * prvTCPReturnPacket(). */
        ( void ) memcpy( ( void * ) ( &( pxTCPPacket->xEthernetHeader.xSourceAddress ) ), ( const void * ) ( &xEthAddress ), sizeof( xEthAddress ) );

        if( pxSocket->bits.bIsIPv6 != pdFALSE_UNSIGNED )
        {
            /* 'ipIPv4_FRAME_TYPE' is already in network-byte-order. */
            pxTCPPacket->xEthernetHeader.usFrameType = ipIPv6_FRAME_TYPE;

            pxIPHeader->ucVersionTrafficClass = ( uint8_t ) 0x60U;
            pxIPHeader->ucTrafficClassFlow = ( uint8_t ) 0x00;
            pxIPHeader->usFlowLabel = ( uint16_t ) 0x0000U;
            pxIPHeader->usPayloadLength = FreeRTOS_htons( sizeof( TCPHeader_t ) );
            pxIPHeader->ucNextHeader = ( uint8_t ) ipPROTOCOL_TCP;
            pxIPHeader->ucHopLimit = 128;
            /* The Source and Destination addresses will be swapped later. */
            ( void ) memcpy( pxIPHeader->xSourceAddress.ucBytes, pxSocket->u.xTCP.xRemoteIP.xIP_IPv6.ucBytes, sizeof( pxIPHeader->xSourceAddress.ucBytes ) );
            ( void ) memcpy( pxIPHeader->xDestinationAddress.ucBytes, pxSocket->xLocalAddress.xIP_IPv6.ucBytes, sizeof( pxIPHeader->xDestinationAddress.ucBytes ) );
            pxEndPoint = pxSocket->pxEndPoint;
        }
    }

    if( pxEndPoint != NULL )
    {
        pxSocket->pxEndPoint = pxEndPoint;

        pxProtocolHeaders->xTCPHeader.usSourcePort = FreeRTOS_htons( pxSocket->u.xTCP.usRemotePort );
        pxProtocolHeaders->xTCPHeader.usDestinationPort = FreeRTOS_htons( pxSocket->usLocalPort );

        /* We are actively connecting, so the peer's Initial Sequence Number (ISN)
         * isn't known yet. */
        pxSocket->u.xTCP.xTCPWindow.rx.ulCurrentSequenceNumber = 0U;

        /* Start with ISN (Initial Sequence Number). */
        pxSocket->u.xTCP.xTCPWindow.ulOurSequenceNumber = ulInitialSequenceNumber;

        /* The TCP header size is 20 bytes, divided by 4 equals 5, which is put in
         * the high nibble of the TCP offset field. */
        pxProtocolHeaders->xTCPHeader.ucTCPOffset = 0x50U;

        /* Only set the SYN flag. */
        pxProtocolHeaders->xTCPHeader.ucTCPFlags = tcpTCP_FLAG_SYN;

        /* Set the value of usMSS for this socket. */
        prvSocketSetMSS( pxSocket );

        /* The initial sequence numbers at our side are known.  Later
         * vTCPWindowInit() will be called to fill in the peer's sequence numbers, but
         * first wait for a SYN+ACK reply. */
        if( prvTCPCreateWindow( pxSocket ) != pdTRUE )
        {
            xReturn = pdFAIL;
        }
    }
    else
    {
        FreeRTOS_printf( ( "prvTCPPrepareConnect: No pxEndPoint yet?\n" ) );
        xReturn = pdFAIL;
    }

    return xReturn;
}
/*-----------------------------------------------------------*/


/**
 * @brief Common code for sending a TCP protocol control packet (i.e. no options, no
 *        payload, just flags).
 *
 * @param[in] pxNetworkBuffer The network buffer received from the peer.
 * @param[in] ucTCPFlags The flags to determine what kind of packet this is.
 *
 * @return pdFAIL always indicating that the packet was not consumed.
 */
BaseType_t prvTCPSendSpecialPktHelper_IPV6( NetworkBufferDescriptor_t * pxNetworkBuffer,
                                            uint8_t ucTCPFlags )
{
    #if ( ipconfigIGNORE_UNKNOWN_PACKETS == 1 )
        /* Configured to ignore unknown packets just suppress a compiler warning. */
        ( void ) pxNetworkBuffer;
        ( void ) ucTCPFlags;
    #else
    {
        /* Map the ethernet buffer onto the TCPPacket_t struct for easy access to the fields. */

        /* MISRA Ref 11.3.1 [Misaligned access] */
        /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
        /* coverity[misra_c_2012_rule_11_3_violation] */
        TCPPacket_IPv6_t * pxTCPPacket = ( ( TCPPacket_IPv6_t * ) pxNetworkBuffer->pucEthernetBuffer );
        const uint32_t ulSendLength =
            ipSIZE_OF_IPv6_HEADER + ipSIZE_OF_TCP_HEADER; /* Plus 0 options. */

        uint8_t ucFlagsReceived = pxTCPPacket->xTCPHeader.ucTCPFlags;
        pxTCPPacket->xTCPHeader.ucTCPFlags = ucTCPFlags;
        pxTCPPacket->xTCPHeader.ucTCPOffset = ( ipSIZE_OF_TCP_HEADER ) << 2;

        if( ( ucFlagsReceived & tcpTCP_FLAG_SYN ) != 0U )
        {
            /* A synchronize packet is received. It counts as 1 pseudo byte of data,
             * so increase the variable with 1. Before sending a reply, the values of
             * 'ulSequenceNumber' and 'ulAckNr' will be swapped. */
            uint32_t ulSequenceNumber = FreeRTOS_ntohl( pxTCPPacket->xTCPHeader.ulSequenceNumber );
            ulSequenceNumber++;
            pxTCPPacket->xTCPHeader.ulSequenceNumber = FreeRTOS_htonl( ulSequenceNumber );
        }

        prvTCPReturnPacket( NULL, pxNetworkBuffer, ulSendLength, pdFALSE );
    }
    #endif /* !ipconfigIGNORE_UNKNOWN_PACKETS */

    /* The packet was not consumed. */
    return pdFAIL;
}
/*-----------------------------------------------------------*/


/* *INDENT-OFF* */
#endif /* ( ipconfigUSE_IPv6 != 0 ) && ( ipconfigUSE_TCP == 1 ) */
/* *INDENT-ON* */
