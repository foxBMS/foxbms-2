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
 * @file FreeRTOS_TCP_State_Handling_IPv4.c
 * @brief Module which handles the TCP protocol state transition for FreeRTOS+TCP.
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
#include "FreeRTOS_UDP_IP.h"
#include "FreeRTOS_DHCP.h"
#include "NetworkInterface.h"
#include "NetworkBufferManagement.h"

#include "FreeRTOS_TCP_Reception.h"
#include "FreeRTOS_TCP_Transmission.h"
#include "FreeRTOS_TCP_State_Handling.h"
#include "FreeRTOS_TCP_Utils.h"

/* Just make sure the contents doesn't get compiled if TCP is not enabled. */
/* *INDENT-OFF* */
#if( ipconfigUSE_IPv4 != 0 ) && ( ipconfigUSE_TCP == 1 )
/* *INDENT-ON* */

/**
 * @brief Handle 'listen' event on the given socket.
 *
 * @param[in] pxSocket The socket on which the listen occurred.
 * @param[in] pxNetworkBuffer The network buffer carrying the packet.
 *
 * @return If a new socket/duplicate socket is created, then the pointer to
 *         that socket is returned or else, a NULL pointer is returned.
 */
FreeRTOS_Socket_t * prvHandleListen_IPV4( FreeRTOS_Socket_t * pxSocket,
                                          NetworkBufferDescriptor_t * pxNetworkBuffer )
{
    /* Map the ethernet buffer onto a TCPPacket_t struct for easy access to the fields. */

    const TCPPacket_t * pxTCPPacket = NULL;
    FreeRTOS_Socket_t * pxReturn = NULL;
    uint32_t ulInitialSequenceNumber = 0U;
    const NetworkEndPoint_t * pxEndpoint = NULL;
    BaseType_t xIsNewSocket = pdFALSE;

    if( ( pxSocket != NULL ) && ( pxNetworkBuffer != NULL ) )
    {
        /* Initialize pointers if inputs are valid. */
        /* MISRA Ref 11.3.1 [Misaligned access] */
        /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
        /* coverity[misra_c_2012_rule_11_3_violation] */
        pxTCPPacket = ( ( const TCPPacket_t * ) pxNetworkBuffer->pucEthernetBuffer );
        pxEndpoint = pxNetworkBuffer->pxEndPoint;
    }

    /* Silently discard a SYN packet which was not specifically sent for this node. */
    if( ( pxEndpoint != NULL ) && ( pxTCPPacket->xIPHeader.ulDestinationIPAddress == pxEndpoint->ipv4_settings.ulIPAddress ) )
    {
        /* Assume that a new Initial Sequence Number will be required. Request
         * it now in order to fail out if necessary. */
        ulInitialSequenceNumber = ulApplicationGetNextSequenceNumber( pxTCPPacket->xIPHeader.ulDestinationIPAddress,
                                                                      pxSocket->usLocalPort,
                                                                      pxTCPPacket->xIPHeader.ulSourceIPAddress,
                                                                      pxTCPPacket->xTCPHeader.usSourcePort );
    }
    else
    {
        /* Set the sequence number to 0 to avoid further processing. */
        ulInitialSequenceNumber = 0U;
    }

    /* A pure SYN (without ACK) has come in, create a new socket to answer
     * it. */
    if( ulInitialSequenceNumber != 0U )
    {
        if( pxSocket->u.xTCP.bits.bReuseSocket != pdFALSE_UNSIGNED )
        {
            /* The flag bReuseSocket indicates that the same instance of the
             * listening socket should be used for the connection. */
            pxReturn = pxSocket;
            pxSocket->u.xTCP.bits.bPassQueued = pdTRUE_UNSIGNED;
            pxSocket->u.xTCP.pxPeerSocket = pxSocket;
        }
        else
        {
            /* The socket does not have the bReuseSocket flag set meaning create a
             * new socket when a connection comes in. */
            pxReturn = NULL;

            if( pxSocket->u.xTCP.usChildCount >= pxSocket->u.xTCP.usBacklog )
            {
                FreeRTOS_printf( ( "Check: Socket %u already has %u / %u child%s\n",
                                   pxSocket->usLocalPort,
                                   pxSocket->u.xTCP.usChildCount,
                                   pxSocket->u.xTCP.usBacklog,
                                   ( pxSocket->u.xTCP.usChildCount == 1U ) ? "" : "ren" ) );
                ( void ) prvTCPSendReset( pxNetworkBuffer );
            }
            else
            {
                FreeRTOS_Socket_t * pxNewSocket = ( FreeRTOS_Socket_t * )
                                                  FreeRTOS_socket( FREERTOS_AF_INET, FREERTOS_SOCK_STREAM, FREERTOS_IPPROTO_TCP );

                /* MISRA Ref 11.4.1 [Socket error and integer to pointer conversion] */
                /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-114 */
                /* coverity[misra_c_2012_rule_11_4_violation] */
                if( ( pxNewSocket == NULL ) || ( pxNewSocket == FREERTOS_INVALID_SOCKET ) )
                {
                    FreeRTOS_debug_printf( ( "TCP: Listen: new socket failed\n" ) );
                    ( void ) prvTCPSendReset( pxNetworkBuffer );
                }
                else if( prvTCPSocketCopy( pxNewSocket, pxSocket ) != pdFALSE )
                {
                    /* The socket will be connected immediately, no time for the
                     * owner to setsockopt's, therefore copy properties of the server
                     * socket to the new socket.  Only the binding might fail (due to
                     * lack of resources). */
                    pxReturn = pxNewSocket;
                    xIsNewSocket = pdTRUE;
                }
                else
                {
                    /* Copying failed somehow. */
                }
            }
        }
    }

    if( ( ulInitialSequenceNumber != 0U ) && ( pxReturn != NULL ) )
    {
        do
        {
            size_t xCopyLength;
            BaseType_t xReturnCreateWindow;

            /* Map the byte stream onto the ProtocolHeaders_t for easy access to the fields. */

            /* MISRA Ref 11.3.1 [Misaligned access] */
            /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
            /* coverity[misra_c_2012_rule_11_3_violation] */
            const ProtocolHeaders_t * pxProtocolHeaders = ( ( const ProtocolHeaders_t * )
                                                            &( pxNetworkBuffer->pucEthernetBuffer[ ipSIZE_OF_ETH_HEADER + uxIPHeaderSizePacket( pxNetworkBuffer ) ] ) );

            /* The endpoint in network buffer must be valid in this condition. */
            pxReturn->pxEndPoint = pxNetworkBuffer->pxEndPoint;
            pxReturn->bits.bIsIPv6 = pdFALSE_UNSIGNED;
            pxReturn->u.xTCP.usRemotePort = FreeRTOS_htons( pxTCPPacket->xTCPHeader.usSourcePort );
            pxReturn->u.xTCP.xRemoteIP.ulIP_IPv4 = FreeRTOS_htonl( pxTCPPacket->xIPHeader.ulSourceIPAddress );
            pxReturn->u.xTCP.xTCPWindow.ulOurSequenceNumber = ulInitialSequenceNumber;

            /* Here is the SYN action. */
            pxReturn->u.xTCP.xTCPWindow.rx.ulCurrentSequenceNumber = FreeRTOS_ntohl( pxProtocolHeaders->xTCPHeader.ulSequenceNumber );
            prvSocketSetMSS( pxReturn );

            xReturnCreateWindow = prvTCPCreateWindow( pxReturn );

            /* Did allocating TCP sectors fail? */
            if( xReturnCreateWindow != pdPASS )
            {
                /* Close the socket if it was newly created. */
                if( xIsNewSocket == pdTRUE )
                {
                    ( void ) vSocketClose( pxReturn );
                }

                pxReturn = NULL;
                break;
            }

            vTCPStateChange( pxReturn, eSYN_FIRST );

            /* Make a copy of the header up to the TCP header.  It is needed later
             * on, whenever data must be sent to the peer. */
            if( pxNetworkBuffer->xDataLength > sizeof( pxReturn->u.xTCP.xPacket.u.ucLastPacket ) )
            {
                xCopyLength = sizeof( pxReturn->u.xTCP.xPacket.u.ucLastPacket );
            }
            else
            {
                xCopyLength = pxNetworkBuffer->xDataLength;
            }

            ( void ) memcpy( ( void * ) pxReturn->u.xTCP.xPacket.u.ucLastPacket,
                             ( const void * ) pxNetworkBuffer->pucEthernetBuffer,
                             xCopyLength );
        } while( ipFALSE_BOOL );
    }

    return pxReturn;
}
/*-----------------------------------------------------------*/

/* *INDENT-OFF* */
#endif /* ( ipconfigUSE_IPv4 != 0 ) && ( ipconfigUSE_TCP == 1 ) */
/* *INDENT-ON* */
