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
 * @file FreeRTOS_TCP_State_Handling.c
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
#if ipconfigUSE_TCP == 1

/*
 *  Called to handle the closure of a TCP connection.
 */
    static BaseType_t prvTCPHandleFin( FreeRTOS_Socket_t * pxSocket,
                                       const NetworkBufferDescriptor_t * pxNetworkBuffer );

/*
 * Called from prvTCPHandleState() as long as the TCP status is eSYN_RECEIVED to
 * eCONNECT_SYN.
 */
    static BaseType_t prvHandleSynReceived( FreeRTOS_Socket_t * pxSocket,
                                            const NetworkBufferDescriptor_t * pxNetworkBuffer,
                                            uint32_t ulReceiveLength,
                                            UBaseType_t uxOptionsLength );

/*
 * Called from prvTCPHandleState() as long as the TCP status is eESTABLISHED.
 */
    static BaseType_t prvHandleEstablished( FreeRTOS_Socket_t * pxSocket,
                                            NetworkBufferDescriptor_t ** ppxNetworkBuffer,
                                            uint32_t ulReceiveLength,
                                            UBaseType_t uxOptionsLength );


/**
 * @brief Check whether the socket is active or not.
 *
 * @param[in] eStatus The status of the socket.
 *
 * @return pdTRUE if the socket must be checked. Non-active sockets
 *         are waiting for user action, either connect() or close().
 */
    BaseType_t prvTCPSocketIsActive( eIPTCPState_t eStatus )
    {
        BaseType_t xResult;

        switch( eStatus )
        {
            case eCLOSED:
            case eCLOSE_WAIT:
            case eFIN_WAIT_2:
            case eCLOSING:
            case eTIME_WAIT:
                xResult = pdFALSE;
                break;

            case eTCP_LISTEN:
            case eCONNECT_SYN:
            case eSYN_FIRST:
            case eSYN_RECEIVED:
            case eESTABLISHED:
            case eFIN_WAIT_1:
            case eLAST_ACK:
            default:
                xResult = pdTRUE;
                break;
        }

        return xResult;
    }
/*-----------------------------------------------------------*/



    #if ( ipconfigTCP_HANG_PROTECTION == 1 )

/**
 * @brief Some of the TCP states may only last a certain amount of time.
 *        This function checks if the socket is 'hanging', i.e. staying
 *        too long in the same state.
 *
 * @param[in] pxSocket the socket to be checked.
 *
 * @return pdFALSE if no checks are needed, pdTRUE if checks were done, or negative
 *         in case the socket has reached a critical time-out. The socket will go to
 *         the eCLOSE_WAIT state.
 */
        BaseType_t prvTCPStatusAgeCheck( FreeRTOS_Socket_t * pxSocket )
        {
            BaseType_t xResult;

            eIPTCPState_t eState = pxSocket->u.xTCP.eTCPState;

            switch( eState )
            {
                case eESTABLISHED:

                    /* If the 'ipconfigTCP_KEEP_ALIVE' option is enabled, sockets in
                     *  state ESTABLISHED can be protected using keep-alive messages. */
                    xResult = pdFALSE;
                    break;

                case eCLOSED:
                case eTCP_LISTEN:
                case eCLOSE_WAIT:
                    /* These 3 states may last for ever, up to the owner. */
                    xResult = pdFALSE;
                    break;

                case eCONNECT_SYN:
                case eSYN_FIRST:
                case eSYN_RECEIVED:
                case eFIN_WAIT_1:
                case eFIN_WAIT_2:
                case eCLOSING:
                case eLAST_ACK:
                case eTIME_WAIT:
                default:

                    /* All other (non-connected) states will get anti-hanging
                     * protection. */
                    xResult = pdTRUE;
                    break;
            }

            if( xResult != pdFALSE )
            {
                /* How much time has past since the last active moment which is
                 * defined as A) a state change or B) a packet has arrived. */
                TickType_t xAge = xTaskGetTickCount() - pxSocket->u.xTCP.xLastActTime;

                /* ipconfigTCP_HANG_PROTECTION_TIME is in units of seconds. */
                if( xAge > ( ( TickType_t ) ipconfigTCP_HANG_PROTECTION_TIME * ( TickType_t ) configTICK_RATE_HZ ) )
                {
                    #if ( ipconfigHAS_DEBUG_PRINTF == 1 )
                    {
                        FreeRTOS_debug_printf( ( "Inactive socket closed: port %u rem %xip:%u status %s\n",
                                                 pxSocket->usLocalPort,
                                                 ( unsigned ) pxSocket->u.xTCP.xRemoteIP.ulIP_IPv4,
                                                 pxSocket->u.xTCP.usRemotePort,
                                                 FreeRTOS_GetTCPStateName( ( UBaseType_t ) pxSocket->u.xTCP.eTCPState ) ) );
                    }
                    #endif /* ipconfigHAS_DEBUG_PRINTF */

                    /* Move to eCLOSE_WAIT, user may close the socket. */
                    vTCPStateChange( pxSocket, eCLOSE_WAIT );

                    /* When 'bPassQueued' true, this socket is an orphan until it
                     * gets connected. */
                    if( pxSocket->u.xTCP.bits.bPassQueued != pdFALSE_UNSIGNED )
                    {
                        /* vTCPStateChange() has called vSocketCloseNextTime()
                         * in case the socket is not yet owned by the application.
                         * Return a negative value to inform the caller that
                         * the socket will be closed in the next cycle. */
                        xResult = -1;
                    }
                }
            }

            return xResult;
        }
        /*-----------------------------------------------------------*/

    #endif /* if ( ipconfigTCP_HANG_PROTECTION == 1 ) */

/**
 * @brief prvTCPHandleFin() will be called to handle connection closure. The
 *        closure starts when either a FIN has been received and accepted,
 *        or when the socket has sent a FIN flag to the peer. Before being
 *        called, it has been checked that both reception and transmission
 *        are complete.
 *
 * @param[in] pxSocket Socket owning the the connection.
 * @param[in] pxNetworkBuffer The network buffer carrying the TCP packet.
 *
 * @return Length of the packet to be sent.
 */
    static BaseType_t prvTCPHandleFin( FreeRTOS_Socket_t * pxSocket,
                                       const NetworkBufferDescriptor_t * pxNetworkBuffer )
    {
        /* Map the ethernet buffer onto the ProtocolHeader_t struct for easy access to the fields. */

        /* MISRA Ref 11.3.1 [Misaligned access] */
        /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
        /* coverity[misra_c_2012_rule_11_3_violation] */
        ProtocolHeaders_t * pxProtocolHeaders = ( ( ProtocolHeaders_t * )
                                                  &( pxNetworkBuffer->pucEthernetBuffer[ ipSIZE_OF_ETH_HEADER + uxIPHeaderSizePacket( pxNetworkBuffer ) ] ) );
        TCPHeader_t * pxTCPHeader = &( pxProtocolHeaders->xTCPHeader );
        uint8_t ucIntermediateResult = 0, ucTCPFlags = pxTCPHeader->ucTCPFlags;
        TCPWindow_t * pxTCPWindow = &pxSocket->u.xTCP.xTCPWindow;
        BaseType_t xSendLength = 0;
        uint32_t ulAckNr = FreeRTOS_ntohl( pxTCPHeader->ulAckNr );

        if( ( ucTCPFlags & tcpTCP_FLAG_FIN ) != 0U )
        {
            pxTCPWindow->rx.ulCurrentSequenceNumber = pxTCPWindow->rx.ulFINSequenceNumber + 1U;
        }

        if( pxSocket->u.xTCP.bits.bFinSent == pdFALSE_UNSIGNED )
        {
            /* We haven't yet replied with a FIN, do so now. */
            pxTCPWindow->tx.ulFINSequenceNumber = pxTCPWindow->tx.ulCurrentSequenceNumber;
            pxSocket->u.xTCP.bits.bFinSent = pdTRUE_UNSIGNED;
        }
        else
        {
            /* We did send a FIN already, see if it's ACK'd. */
            if( ulAckNr == ( pxTCPWindow->tx.ulFINSequenceNumber + 1U ) )
            {
                pxSocket->u.xTCP.bits.bFinAcked = pdTRUE_UNSIGNED;
            }
        }

        if( pxSocket->u.xTCP.bits.bFinAcked == pdFALSE_UNSIGNED )
        {
            pxTCPWindow->tx.ulCurrentSequenceNumber = pxTCPWindow->tx.ulFINSequenceNumber;
            pxTCPHeader->ucTCPFlags = ( uint8_t ) tcpTCP_FLAG_ACK | ( uint8_t ) tcpTCP_FLAG_FIN;

            /* And wait for the final ACK. */
            vTCPStateChange( pxSocket, eLAST_ACK );
        }
        else
        {
            /* Our FIN has been ACK'd, the outgoing sequence number is now fixed. */
            pxTCPWindow->tx.ulCurrentSequenceNumber = pxTCPWindow->tx.ulFINSequenceNumber + 1U;

            if( pxSocket->u.xTCP.bits.bFinRecv == pdFALSE_UNSIGNED )
            {
                /* We have sent out a FIN but the peer hasn't replied with a FIN
                 * yet. Do nothing for the moment. */
                pxTCPHeader->ucTCPFlags = 0U;
            }
            else
            {
                if( pxSocket->u.xTCP.bits.bFinLast == pdFALSE_UNSIGNED )
                {
                    /* This is the third of the three-way hand shake: the last
                     * ACK. */
                    pxTCPHeader->ucTCPFlags = tcpTCP_FLAG_ACK;
                }
                else
                {
                    /* The other party started the closure, so we just wait for the
                     * last ACK. */
                    pxTCPHeader->ucTCPFlags = 0U;
                }

                /* And wait for the user to close this socket. */
                vTCPStateChange( pxSocket, eCLOSE_WAIT );
            }
        }

        pxTCPWindow->ulOurSequenceNumber = pxTCPWindow->tx.ulCurrentSequenceNumber;

        if( pxTCPHeader->ucTCPFlags != 0U )
        {
            ucIntermediateResult = ( uint8_t ) ( uxIPHeaderSizeSocket( pxSocket ) + ipSIZE_OF_TCP_HEADER + pxTCPWindow->ucOptionLength );
            xSendLength = ( BaseType_t ) ucIntermediateResult;
        }

        pxTCPHeader->ucTCPOffset = ( uint8_t ) ( ( ipSIZE_OF_TCP_HEADER + pxTCPWindow->ucOptionLength ) << 2 );

        if( xTCPWindowLoggingLevel != 0 )
        {
            FreeRTOS_debug_printf( ( "TCP: send FIN+ACK (ack %u, cur/nxt %u/%u) ourSeqNr %u | Rx %u\n",
                                     ( unsigned ) ( ulAckNr - pxTCPWindow->tx.ulFirstSequenceNumber ),
                                     ( unsigned ) ( pxTCPWindow->tx.ulCurrentSequenceNumber - pxTCPWindow->tx.ulFirstSequenceNumber ),
                                     ( unsigned ) ( pxTCPWindow->ulNextTxSequenceNumber - pxTCPWindow->tx.ulFirstSequenceNumber ),
                                     ( unsigned ) ( pxTCPWindow->ulOurSequenceNumber - pxTCPWindow->tx.ulFirstSequenceNumber ),
                                     ( unsigned ) ( pxTCPWindow->rx.ulCurrentSequenceNumber - pxTCPWindow->rx.ulFirstSequenceNumber ) ) );
        }

        return xSendLength;
    }
    /*-----------------------------------------------------------*/


/**
 * @brief prvHandleSynReceived(): called from prvTCPHandleState(). Called
 *        from the states: eSYN_RECEIVED and eCONNECT_SYN. If the flags
 *        received are correct, the socket will move to eESTABLISHED.
 *
 * @param[in] pxSocket The socket handling the connection.
 * @param[in] pxNetworkBuffer The pointer to the network buffer carrying
 *                             the packet.
 * @param[in] ulReceiveLength Length in bytes of the data received.
 * @param[in] uxOptionsLength Length of the TCP options in bytes.
 *
 * @return Length of the data to be sent.
 */
    static BaseType_t prvHandleSynReceived( FreeRTOS_Socket_t * pxSocket,
                                            const NetworkBufferDescriptor_t * pxNetworkBuffer,
                                            uint32_t ulReceiveLength,
                                            UBaseType_t uxOptionsLength )
    {
        /* Map the ethernet buffer onto the ProtocolHeader_t struct for easy access to the fields. */

        /* MISRA Ref 11.3.1 [Misaligned access] */
        /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
        /* coverity[misra_c_2012_rule_11_3_violation] */
        ProtocolHeaders_t * pxProtocolHeaders = ( ( ProtocolHeaders_t * )
                                                  &( pxNetworkBuffer->pucEthernetBuffer[ ipSIZE_OF_ETH_HEADER + uxIPHeaderSizeSocket( pxSocket ) ] ) );
        TCPHeader_t * pxTCPHeader = &pxProtocolHeaders->xTCPHeader;
        TCPWindow_t * pxTCPWindow = &pxSocket->u.xTCP.xTCPWindow;
        uint8_t ucTCPFlags = pxTCPHeader->ucTCPFlags;
        uint32_t ulSequenceNumber = FreeRTOS_ntohl( pxTCPHeader->ulSequenceNumber );
        BaseType_t xSendLength = 0;
        UBaseType_t uxIntermediateResult = 0U;

        /* Either expect a ACK or a SYN+ACK. */
        uint8_t ucExpect = tcpTCP_FLAG_ACK;
        const uint8_t ucFlagsMask = tcpTCP_FLAG_ACK | tcpTCP_FLAG_RST | tcpTCP_FLAG_SYN | tcpTCP_FLAG_FIN;

        if( pxSocket->u.xTCP.eTCPState == eCONNECT_SYN )
        {
            ucExpect |= tcpTCP_FLAG_SYN;
        }

        if( ( ucTCPFlags & ucFlagsMask ) != ucExpect )
        {
            /* eSYN_RECEIVED: flags 0010 expected, not 0002. */
            /* eSYN_RECEIVED: flags ACK  expected, not SYN. */
            FreeRTOS_debug_printf( ( "%s: flags %04X expected, not %04X\n",
                                     ( pxSocket->u.xTCP.eTCPState == ( uint8_t ) eSYN_RECEIVED ) ? "eSYN_RECEIVED" : "eCONNECT_SYN",
                                     ucExpect, ucTCPFlags ) );

            /* In case pxSocket is not yet owned by the application, a closure
             * of the socket will be scheduled for the next cycle. */
            vTCPStateChange( pxSocket, eCLOSE_WAIT );

            /* Send RST with the expected sequence and ACK numbers,
             * otherwise the packet will be ignored. */
            pxTCPWindow->ulOurSequenceNumber = FreeRTOS_htonl( pxTCPHeader->ulAckNr );
            pxTCPWindow->rx.ulCurrentSequenceNumber = ulSequenceNumber;

            pxTCPHeader->ucTCPFlags |= tcpTCP_FLAG_RST;

            uxIntermediateResult = uxIPHeaderSizeSocket( pxSocket ) + ipSIZE_OF_TCP_HEADER + uxOptionsLength;
            xSendLength = ( BaseType_t ) uxIntermediateResult;

            pxTCPHeader->ucTCPOffset = ( uint8_t ) ( ( ipSIZE_OF_TCP_HEADER + uxOptionsLength ) << 2 );
        }
        else
        {
            pxTCPWindow->usPeerPortNumber = pxSocket->u.xTCP.usRemotePort;
            pxTCPWindow->usOurPortNumber = pxSocket->usLocalPort;

            if( pxSocket->u.xTCP.eTCPState == eCONNECT_SYN )
            {
                /* Map the Last packet onto the ProtocolHeader_t struct for easy access to the fields. */

                /* MISRA Ref 11.3.1 [Misaligned access] */
                /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
                /* coverity[misra_c_2012_rule_11_3_violation] */
                ProtocolHeaders_t * pxLastHeaders = ( ( ProtocolHeaders_t * )
                                                      &( pxSocket->u.xTCP.xPacket.u.ucLastPacket[ ipSIZE_OF_ETH_HEADER + uxIPHeaderSizeSocket( pxSocket ) ] ) );

                /* Clear the SYN flag in lastPacket. */
                pxLastHeaders->xTCPHeader.ucTCPFlags = tcpTCP_FLAG_ACK;
                pxProtocolHeaders->xTCPHeader.ucTCPFlags = tcpTCP_FLAG_ACK;

                /* This socket was the one connecting actively so now perform the
                 * synchronisation. */
                vTCPWindowInit( &pxSocket->u.xTCP.xTCPWindow,
                                ulSequenceNumber, pxSocket->u.xTCP.xTCPWindow.ulOurSequenceNumber, ( uint32_t ) pxSocket->u.xTCP.usMSS );
                pxTCPWindow->rx.ulHighestSequenceNumber = ulSequenceNumber + 1U;
                pxTCPWindow->rx.ulCurrentSequenceNumber = ulSequenceNumber + 1U;
                pxTCPWindow->tx.ulCurrentSequenceNumber++; /* because we send a TCP_SYN [ | TCP_ACK ]; */
                pxTCPWindow->ulNextTxSequenceNumber++;
            }
            else if( ulReceiveLength == 0U )
            {
                pxTCPWindow->rx.ulCurrentSequenceNumber = ulSequenceNumber;
            }
            else
            {
                /* Nothing. */
            }

            /* The SYN+ACK has been confirmed, increase the next sequence number by
             * 1. */
            pxTCPWindow->ulOurSequenceNumber = pxTCPWindow->tx.ulFirstSequenceNumber + 1U;

            #if ( ipconfigUSE_TCP_WIN == 1 )
            {
                char pcBuffer[ 40 ]; /* Space to print an IP-address. */
                ( void ) FreeRTOS_inet_ntop( ( pxSocket->bits.bIsIPv6 != 0U ) ? FREERTOS_AF_INET6 : FREERTOS_AF_INET,
                                             ( void * ) pxSocket->u.xTCP.xRemoteIP.xIP_IPv6.ucBytes,
                                             pcBuffer,
                                             sizeof( pcBuffer ) );
                FreeRTOS_debug_printf( ( "TCP: %s %u => %s port %u set ESTAB (scaling %u)\n",
                                         ( pxSocket->u.xTCP.eTCPState == ( uint8_t ) eCONNECT_SYN ) ? "active" : "passive",
                                         pxSocket->usLocalPort,
                                         pcBuffer,
                                         pxSocket->u.xTCP.usRemotePort,
                                         ( unsigned ) pxSocket->u.xTCP.bits.bWinScaling ) );
            }
            #endif /* ipconfigUSE_TCP_WIN */

            if( ( pxSocket->u.xTCP.eTCPState == eCONNECT_SYN ) || ( ulReceiveLength != 0U ) )
            {
                pxTCPHeader->ucTCPFlags = tcpTCP_FLAG_ACK;

                uxIntermediateResult = uxIPHeaderSizeSocket( pxSocket ) + ( size_t ) ipSIZE_OF_TCP_HEADER + uxOptionsLength;
                xSendLength = ( BaseType_t ) uxIntermediateResult;
                pxTCPHeader->ucTCPOffset = ( uint8_t ) ( ( ipSIZE_OF_TCP_HEADER + uxOptionsLength ) << 2 );
            }

            #if ( ipconfigUSE_TCP_WIN != 0 )
            {
                if( pxSocket->u.xTCP.bits.bWinScaling == pdFALSE_UNSIGNED )
                {
                    /* The other party did not send a scaling factor.
                     * A shifting factor in this side must be canceled. */
                    pxSocket->u.xTCP.ucMyWinScaleFactor = 0;
                    pxSocket->u.xTCP.ucPeerWinScaleFactor = 0;
                }
            }
            #endif /* ipconfigUSE_TCP_WIN */

            /* This was the third step of connecting: SYN, SYN+ACK, ACK so now the
             * connection is established. */
            vTCPStateChange( pxSocket, eESTABLISHED );
        }

        return xSendLength;
    }
    /*-----------------------------------------------------------*/

/**
 * @brief prvHandleEstablished(): called from prvTCPHandleState()
 *        Called if the status is eESTABLISHED. Data reception has been handled
 *        earlier. Here the ACK's from peer will be checked, and if a FIN is received,
 *        the code will check if it may be accepted, i.e. if all expected data has been
 *        completely received.
 *
 * @param[in] pxSocket The socket owning the connection.
 * @param[in,out] ppxNetworkBuffer Pointer to pointer to the network buffer.
 * @param[in] ulReceiveLength The length of the received packet.
 * @param[in] uxOptionsLength Length of TCP options.
 *
 * @return The send length of the packet to be sent.
 */
    static BaseType_t prvHandleEstablished( FreeRTOS_Socket_t * pxSocket,
                                            NetworkBufferDescriptor_t ** ppxNetworkBuffer,
                                            uint32_t ulReceiveLength,
                                            UBaseType_t uxOptionsLength )
    {
        /* Map the buffer onto the ProtocolHeader_t struct for easy access to the fields. */

        /* MISRA Ref 11.3.1 [Misaligned access] */
        /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
        /* coverity[misra_c_2012_rule_11_3_violation] */
        ProtocolHeaders_t * pxProtocolHeaders = ( ( ProtocolHeaders_t * )
                                                  &( ( *ppxNetworkBuffer )->pucEthernetBuffer[ ipSIZE_OF_ETH_HEADER + uxIPHeaderSizeSocket( pxSocket ) ] ) );
        TCPHeader_t * pxTCPHeader = &pxProtocolHeaders->xTCPHeader;
        TCPWindow_t * pxTCPWindow = &pxSocket->u.xTCP.xTCPWindow;
        uint8_t ucTCPFlags = pxTCPHeader->ucTCPFlags;
        uint32_t ulSequenceNumber = FreeRTOS_ntohl( pxTCPHeader->ulSequenceNumber ), ulCount, ulIntermediateResult = 0;
        BaseType_t xSendLength = 0, xMayClose = pdFALSE, bRxComplete, bTxDone;
        int32_t lDistance, lSendResult;
        uint16_t usWindow;
        UBaseType_t uxIntermediateResult = 0;

        /* Remember the window size the peer is advertising. */
        usWindow = FreeRTOS_ntohs( pxTCPHeader->usWindow );
        pxSocket->u.xTCP.ulWindowSize = ( uint32_t ) usWindow;
        #if ( ipconfigUSE_TCP_WIN != 0 )
        {
            pxSocket->u.xTCP.ulWindowSize =
                ( pxSocket->u.xTCP.ulWindowSize << pxSocket->u.xTCP.ucPeerWinScaleFactor );
        }
        #endif /* ipconfigUSE_TCP_WIN */

        if( ( ucTCPFlags & ( uint8_t ) tcpTCP_FLAG_ACK ) == 0U )
        {
            /* RFC793: If ACK bit is not set at this state, the segment should
             * be dropped
             */
        }
        else
        {
            ulCount = ulTCPWindowTxAck( pxTCPWindow, FreeRTOS_ntohl( pxTCPHeader->ulAckNr ) );

            /* ulTCPWindowTxAck() returns the number of bytes which have been acked,
             * starting at 'tx.ulCurrentSequenceNumber'.  Advance the tail pointer in
             * txStream. */
            if( ( pxSocket->u.xTCP.txStream != NULL ) && ( ulCount > 0U ) )
            {
                /* Just advancing the tail index, 'ulCount' bytes have been
                 * confirmed, and because there is new space in the txStream, the
                 * user/owner should be woken up. */
                /* _HT_ : only in case the socket's waiting? */
                if( uxStreamBufferGet( pxSocket->u.xTCP.txStream, 0U, NULL, ( size_t ) ulCount, pdFALSE ) != 0U )
                {
                    pxSocket->xEventBits |= ( EventBits_t ) eSOCKET_SEND;

                    #if ipconfigSUPPORT_SELECT_FUNCTION == 1
                    {
                        if( ( pxSocket->xSelectBits & ( ( EventBits_t ) eSELECT_WRITE ) ) != 0U )
                        {
                            pxSocket->xEventBits |= ( ( EventBits_t ) eSELECT_WRITE ) << SOCKET_EVENT_BIT_COUNT;
                        }
                    }
                    #endif

                    /* In case the socket owner has installed an OnSent handler,
                     * call it now. */
                    #if ( ipconfigUSE_CALLBACKS == 1 )
                    {
                        if( ipconfigIS_VALID_PROG_ADDRESS( pxSocket->u.xTCP.pxHandleSent ) )
                        {
                            pxSocket->u.xTCP.pxHandleSent( ( Socket_t ) pxSocket, ulCount );
                        }
                    }
                    #endif /* ipconfigUSE_CALLBACKS == 1  */
                }
            }

            /* If this socket has a stream for transmission, add the data to the
             * outgoing segment(s). */
            if( pxSocket->u.xTCP.txStream != NULL )
            {
                prvTCPAddTxData( pxSocket );
            }

            pxSocket->u.xTCP.xTCPWindow.ulOurSequenceNumber = pxTCPWindow->tx.ulCurrentSequenceNumber;

            if( ( pxSocket->u.xTCP.bits.bFinAccepted != pdFALSE_UNSIGNED ) || ( ( ucTCPFlags & ( uint8_t ) tcpTCP_FLAG_FIN ) != 0U ) )
            {
                /* Peer is requesting to stop, see if we're really finished. */
                xMayClose = pdTRUE;
                ulIntermediateResult = ulSequenceNumber + ulReceiveLength - pxTCPWindow->rx.ulCurrentSequenceNumber;
                lDistance = ( int32_t ) ulIntermediateResult;

                /* Checks are only necessary if we haven't sent a FIN yet. */
                if( pxSocket->u.xTCP.bits.bFinSent == pdFALSE_UNSIGNED )
                {
                    /* xTCPWindowTxDone returns true when all Tx queues are empty. */
                    bRxComplete = xTCPWindowRxEmpty( pxTCPWindow );
                    bTxDone = xTCPWindowTxDone( pxTCPWindow );

                    if( ( bRxComplete == 0 ) || ( bTxDone == 0 ) )
                    {
                        /* Refusing FIN: Rx incomplete 1 optlen 4 tx done 1. */
                        FreeRTOS_debug_printf( ( "Refusing FIN[%u,%u]: RxCompl %d tx done %d\n",
                                                 pxSocket->usLocalPort,
                                                 pxSocket->u.xTCP.usRemotePort,
                                                 ( int ) bRxComplete,
                                                 ( int ) bTxDone ) );
                        xMayClose = pdFALSE;

                        /* This action is necessary to ensure proper handling of any subsequent packets that
                         * may arrive after the refused FIN packet. Note that we only update it when the sequence
                         * of FIN packet is correct. Otherwise, we wait for re-transmission. */
                        if( lDistance <= 1 )
                        {
                            pxTCPWindow->rx.ulCurrentSequenceNumber = pxTCPWindow->rx.ulFINSequenceNumber + 1U;
                        }
                    }
                    else if( lDistance > 1 )
                    {
                        FreeRTOS_debug_printf( ( "Refusing FIN: Rx not complete %d (cur %u high %u)\n",
                                                 ( int ) lDistance,
                                                 ( unsigned ) ( pxTCPWindow->rx.ulCurrentSequenceNumber - pxTCPWindow->rx.ulFirstSequenceNumber ),
                                                 ( unsigned ) ( pxTCPWindow->rx.ulHighestSequenceNumber - pxTCPWindow->rx.ulFirstSequenceNumber ) ) );

                        xMayClose = pdFALSE;
                    }
                    else
                    {
                        /* Empty else marker. */
                    }
                }

                if( xTCPWindowLoggingLevel > 0 )
                {
                    FreeRTOS_debug_printf( ( "TCP: FIN received, mayClose = %d (Rx %u Len %d, Tx %u)\n",
                                             ( int ) xMayClose,
                                             ( unsigned ) ( ulSequenceNumber - pxSocket->u.xTCP.xTCPWindow.rx.ulFirstSequenceNumber ),
                                             ( unsigned ) ulReceiveLength,
                                             ( unsigned ) ( pxTCPWindow->tx.ulCurrentSequenceNumber - pxSocket->u.xTCP.xTCPWindow.tx.ulFirstSequenceNumber ) ) );
                }

                if( xMayClose != pdFALSE )
                {
                    pxSocket->u.xTCP.bits.bFinAccepted = pdTRUE_UNSIGNED;
                    xSendLength = prvTCPHandleFin( pxSocket, *ppxNetworkBuffer );
                }
            }

            if( xMayClose == pdFALSE )
            {
                pxTCPHeader->ucTCPFlags = tcpTCP_FLAG_ACK;

                if( ulReceiveLength != 0U )
                {
                    uxIntermediateResult = uxIPHeaderSizeSocket( pxSocket ) + ipSIZE_OF_TCP_HEADER + uxOptionsLength;
                    xSendLength = ( BaseType_t ) uxIntermediateResult;
                    /* TCP-offset equals '( ( length / 4 ) << 4 )', resulting in a shift-left 2 */
                    pxTCPHeader->ucTCPOffset = ( uint8_t ) ( ( ipSIZE_OF_TCP_HEADER + uxOptionsLength ) << 2 );

                    if( pxSocket->u.xTCP.bits.bFinSent != pdFALSE_UNSIGNED )
                    {
                        pxTCPWindow->tx.ulCurrentSequenceNumber = pxTCPWindow->tx.ulFINSequenceNumber;
                    }
                }

                /* Now get data to be transmitted. */

                /* _HT_ patch: since the MTU has be fixed at 1500 in stead of 1526, TCP
                 * can not send-out both TCP options and also a full packet. Sending
                 * options (SACK) is always more urgent than sending data, which can be
                 * sent later. */
                if( uxOptionsLength == 0U )
                {
                    /* prvTCPPrepareSend might allocate a bigger network buffer, if
                     * necessary. */
                    lSendResult = prvTCPPrepareSend( pxSocket, ppxNetworkBuffer, uxOptionsLength );

                    if( lSendResult > 0 )
                    {
                        xSendLength = ( BaseType_t ) lSendResult;
                    }
                }
            }
        }

        return xSendLength;
    }
    /*-----------------------------------------------------------*/


/**
 * @brief Check incoming packets for valid data and handle the state of the
 *        TCP connection and respond according to the situation.
 *
 * @param[in] pxSocket The socket whose connection state is being handled.
 * @param[in] ppxNetworkBuffer The network buffer descriptor holding the
 *            packet received from the peer.
 *
 * @return If the data is correct and some packet was sent to the peer, then
 *         the number of bytes sent is returned, or else a negative value is
 *         returned indicating an error.
 *
 * @note prvTCPHandleState() is the most important function of this TCP stack
 * We've tried to keep it (relatively short) by putting a lot of code in
 * the static functions above:
 *
 *      prvCheckRxData()
 *      prvStoreRxData()
 *      prvSetOptions()
 *      prvHandleSynReceived()
 *      prvHandleEstablished()
 *      prvSendData()
 *
 * As these functions are declared static, and they're called from one location
 * only, most compilers will inline them, thus avoiding a call and return.
 */
    BaseType_t prvTCPHandleState( FreeRTOS_Socket_t * pxSocket,
                                  NetworkBufferDescriptor_t ** ppxNetworkBuffer )
    {
        /* Map the buffer onto the ProtocolHeader_t struct for easy access to the fields. */

        /* MISRA Ref 11.3.1 [Misaligned access] */
        /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
        /* coverity[misra_c_2012_rule_11_3_violation] */
        ProtocolHeaders_t * pxProtocolHeaders = ( ( ProtocolHeaders_t * )
                                                  &( ( *ppxNetworkBuffer )->pucEthernetBuffer[ ipSIZE_OF_ETH_HEADER + uxIPHeaderSizePacket( *ppxNetworkBuffer ) ] ) );
        TCPHeader_t * pxTCPHeader = &( pxProtocolHeaders->xTCPHeader );
        BaseType_t xSendLength = 0;
        uint32_t ulReceiveLength; /* Number of bytes contained in the TCP message. */
        uint8_t * pucRecvData;
        uint32_t ulSequenceNumber = FreeRTOS_ntohl( pxTCPHeader->ulSequenceNumber );

        /* uxOptionsLength: the size of the options to be sent (always a multiple of
         * 4 bytes)
         * 1. in the SYN phase, we shall communicate the MSS
         * 2. in case of a SACK, Selective ACK, ack a segment which comes in
         * out-of-order. */
        UBaseType_t uxOptionsLength = 0U;
        uint8_t ucTCPFlags = pxTCPHeader->ucTCPFlags;
        TCPWindow_t * pxTCPWindow = &( pxSocket->u.xTCP.xTCPWindow );
        UBaseType_t uxIntermediateResult = 0;
        uint32_t ulSum;

        /* First get the length and the position of the received data, if any.
         * pucRecvData will point to the first byte of the TCP payload. */
        ulReceiveLength = ( uint32_t ) prvCheckRxData( *ppxNetworkBuffer, &pucRecvData );

        if( pxSocket->u.xTCP.eTCPState >= eESTABLISHED )
        {
            if( pxTCPWindow->rx.ulCurrentSequenceNumber == ( ulSequenceNumber + 1U ) )
            {
                /* This is most probably a keep-alive message from peer.  Setting
                 * 'bWinChange' doesn't cause a window-size-change, the flag is used
                 * here to force sending an immediate ACK. */
                pxSocket->u.xTCP.bits.bWinChange = pdTRUE_UNSIGNED;
            }
        }

        /* Keep track of the highest sequence number that might be expected within
         * this connection. */
        ulSum = ulSequenceNumber + ulReceiveLength - pxTCPWindow->rx.ulHighestSequenceNumber;

        if( ( ( int32_t ) ulSum ) > 0 )
        {
            pxTCPWindow->rx.ulHighestSequenceNumber = ulSequenceNumber + ulReceiveLength;
        }

        /* Storing data may result in a fatal error if malloc() fails. */
        if( prvStoreRxData( pxSocket, pucRecvData, *ppxNetworkBuffer, ulReceiveLength ) < 0 )
        {
            xSendLength = -1;
        }
        else
        {
            eIPTCPState_t eState;

            uxOptionsLength = prvSetOptions( pxSocket, *ppxNetworkBuffer );

            if( ( pxSocket->u.xTCP.eTCPState == eSYN_RECEIVED ) && ( ( ucTCPFlags & ( uint8_t ) tcpTCP_FLAG_CTRL ) == ( uint8_t ) tcpTCP_FLAG_SYN ) )
            {
                FreeRTOS_debug_printf( ( "eSYN_RECEIVED: ACK expected, not SYN: peer missed our SYN+ACK\n" ) );

                /* In eSYN_RECEIVED a simple ACK is expected, but apparently the
                 * 'SYN+ACK' didn't arrive.  Step back to the previous state in which
                 * a first incoming SYN is handled.  The SYN was counted already so
                 * decrease it first. */
                vTCPStateChange( pxSocket, eSYN_FIRST );
            }

            if( ( ( ucTCPFlags & tcpTCP_FLAG_FIN ) != 0U ) && ( pxSocket->u.xTCP.bits.bFinRecv == pdFALSE_UNSIGNED ) )
            {
                /* It's the first time a FIN has been received, remember its
                 * sequence number. */
                pxTCPWindow->rx.ulFINSequenceNumber = ulSequenceNumber + ulReceiveLength;
                pxSocket->u.xTCP.bits.bFinRecv = pdTRUE_UNSIGNED;

                /* Was peer the first one to send a FIN? */
                if( pxSocket->u.xTCP.bits.bFinSent == pdFALSE_UNSIGNED )
                {
                    /* If so, don't send the-last-ACK. */
                    pxSocket->u.xTCP.bits.bFinLast = pdTRUE_UNSIGNED;
                }
            }

            eState = ( eIPTCPState_t ) pxSocket->u.xTCP.eTCPState;

            switch( eState )
            {
                case eCLOSED: /* (server + client) no connection state at all. */

                    /* Nothing to do for a closed socket, except waiting for the
                     * owner. */
                    break;

                case eTCP_LISTEN: /* (server) waiting for a connection request from
                                   * any remote TCP and port. */

                    /* The listen state was handled in xProcessReceivedTCPPacket().
                     * Should not come here. */
                    break;

                case eSYN_FIRST: /* (server) Just received a SYN request for a server
                                  * socket. */

                    /* A new socket has been created, reply with a SYN+ACK.
                     * Acknowledge with seq+1 because the SYN is seen as pseudo data
                     * with len = 1. */
                    uxOptionsLength = prvSetSynAckOptions( pxSocket, pxTCPHeader );
                    pxTCPHeader->ucTCPFlags = ( uint8_t ) tcpTCP_FLAG_SYN | ( uint8_t ) tcpTCP_FLAG_ACK;

                    uxIntermediateResult = uxIPHeaderSizeSocket( pxSocket ) + ipSIZE_OF_TCP_HEADER + uxOptionsLength;
                    xSendLength = ( BaseType_t ) uxIntermediateResult;

                    /* Set the TCP offset field:  ipSIZE_OF_TCP_HEADER equals 20 and
                     * uxOptionsLength is a multiple of 4.  The complete expression is:
                     * ucTCPOffset = ( ( ipSIZE_OF_TCP_HEADER + uxOptionsLength ) / 4 ) << 4 */
                    pxTCPHeader->ucTCPOffset = ( uint8_t ) ( ( ipSIZE_OF_TCP_HEADER + uxOptionsLength ) << 2 );
                    vTCPStateChange( pxSocket, eSYN_RECEIVED );

                    pxTCPWindow->rx.ulHighestSequenceNumber = ulSequenceNumber + 1U;
                    pxTCPWindow->rx.ulCurrentSequenceNumber = ulSequenceNumber + 1U;
                    pxTCPWindow->ulNextTxSequenceNumber = pxTCPWindow->tx.ulFirstSequenceNumber + 1U;
                    pxTCPWindow->tx.ulCurrentSequenceNumber = pxTCPWindow->tx.ulFirstSequenceNumber + 1U; /* because we send a TCP_SYN. */
                    break;

                case eCONNECT_SYN:  /* (client) also called SYN_SENT: we've just send a
                                     * SYN, expect a SYN+ACK and send a ACK now. */
                /* Fall through */
                case eSYN_RECEIVED: /* (server) we've had a SYN, replied with SYN+SCK
                                     * expect a ACK and do nothing. */
                    xSendLength = prvHandleSynReceived( pxSocket, *( ppxNetworkBuffer ), ulReceiveLength, uxOptionsLength );
                    break;

                case eESTABLISHED: /* (server + client) an open connection, data
                                    * received can be delivered to the user. The normal
                                    * state for the data transfer phase of the connection
                                    * The closing states are also handled here with the
                                    * use of some flags. */
                    xSendLength = prvHandleEstablished( pxSocket, ppxNetworkBuffer, ulReceiveLength, uxOptionsLength );
                    break;

                case eLAST_ACK:   /* (server + client) waiting for an acknowledgement
                                   * of the connection termination request previously
                                   * sent to the remote TCP (which includes an
                                   * acknowledgement of its connection termination
                                   * request). */
                /* Fall through */
                case eFIN_WAIT_1: /* (server + client) waiting for a connection termination request from the remote TCP,
                                   * or an acknowledgement of the connection termination request previously sent. */
                /* Fall through */
                case eFIN_WAIT_2: /* (server + client) waiting for a connection termination request from the remote TCP. */
                    xSendLength = prvTCPHandleFin( pxSocket, *ppxNetworkBuffer );
                    break;

                case eCLOSE_WAIT: /* (server + client) waiting for a connection
                                   * termination request from the local user.  Nothing to
                                   * do, connection is closed, wait for owner to close
                                   * this socket. */
                    break;

                case eCLOSING: /* (server + client) waiting for a connection
                                * termination request acknowledgement from the remote
                                * TCP. */
                    break;

                case eTIME_WAIT: /* (either server or client) waiting for enough time
                                  * to pass to be sure the remote TCP received the
                                  * acknowledgement of its connection termination
                                  * request. [According to RFC 793 a connection can stay
                                  * in TIME-WAIT for a maximum of four minutes known as
                                  * a MSL (maximum segment lifetime).]  These states are
                                  * implemented implicitly by settings flags like
                                  * 'bFinSent', 'bFinRecv', and 'bFinAcked'. */
                    break;

                default:
                    /* No more known states. */
                    break;
            }
        }

        if( xSendLength > 0 )
        {
            xSendLength = prvSendData( pxSocket, ppxNetworkBuffer, ulReceiveLength, xSendLength );
        }

        return xSendLength;
    }
    /*-----------------------------------------------------------*/

/**
 * @brief Handle 'listen' event on the given socket.
 *
 * @param[in] pxSocket The socket on which the listen occurred.
 * @param[in] pxNetworkBuffer The network buffer carrying the packet.
 *
 * @return If a new socket/duplicate socket is created, then the pointer to
 *         that socket is returned or else, a NULL pointer is returned.
 */
    FreeRTOS_Socket_t * prvHandleListen( FreeRTOS_Socket_t * pxSocket,
                                         NetworkBufferDescriptor_t * pxNetworkBuffer )
    {
        FreeRTOS_Socket_t * pxNewSocket = NULL;

        switch( uxIPHeaderSizePacket( pxNetworkBuffer ) )
        {
            #if ( ipconfigUSE_IPv4 != 0 )
                case ipSIZE_OF_IPv4_HEADER:
                    pxNewSocket = prvHandleListen_IPV4( pxSocket, pxNetworkBuffer );
                    break;
            #endif /* ( ipconfigUSE_IPv4 != 0 ) */

            #if ( ipconfigUSE_IPv6 != 0 )
                case ipSIZE_OF_IPv6_HEADER:
                    pxNewSocket = prvHandleListen_IPV6( pxSocket, pxNetworkBuffer );
                    break;
            #endif /* ( ipconfigUSE_IPv6 != 0 ) */

            default:
                /* Shouldn't reach here */
                /* MISRA 16.4 Compliance */
                break;
        }

        return pxNewSocket;
    }
    /*-----------------------------------------------------------*/


/**
 * @brief Duplicates a socket after a listening socket receives a connection and bind
 *        the new socket to the same port as the listening socket.
 *        Also, let the new socket inherit all properties from the listening socket.
 *
 * @param[in] pxNewSocket Pointer to the new socket.
 * @param[in] pxSocket Pointer to the socket being duplicated.
 *
 * @return If all steps all successful, then pdTRUE is returned. Else, pdFALSE.
 */
    BaseType_t prvTCPSocketCopy( FreeRTOS_Socket_t * pxNewSocket,
                                 FreeRTOS_Socket_t * pxSocket )
    {
        struct freertos_sockaddr xAddress;
        BaseType_t xResult;

        pxNewSocket->xReceiveBlockTime = pxSocket->xReceiveBlockTime;
        pxNewSocket->xSendBlockTime = pxSocket->xSendBlockTime;
        pxNewSocket->ucSocketOptions = pxSocket->ucSocketOptions;
        pxNewSocket->u.xTCP.uxRxStreamSize = pxSocket->u.xTCP.uxRxStreamSize;
        pxNewSocket->u.xTCP.uxTxStreamSize = pxSocket->u.xTCP.uxTxStreamSize;
        pxNewSocket->u.xTCP.uxLittleSpace = pxSocket->u.xTCP.uxLittleSpace;
        pxNewSocket->u.xTCP.uxEnoughSpace = pxSocket->u.xTCP.uxEnoughSpace;
        pxNewSocket->u.xTCP.uxRxWinSize = pxSocket->u.xTCP.uxRxWinSize;
        pxNewSocket->u.xTCP.uxTxWinSize = pxSocket->u.xTCP.uxTxWinSize;

        #if ( ipconfigSOCKET_HAS_USER_SEMAPHORE == 1 )
        {
            pxNewSocket->pxUserSemaphore = pxSocket->pxUserSemaphore;
        }
        #endif /* ipconfigSOCKET_HAS_USER_SEMAPHORE */

        #if ( ipconfigUSE_CALLBACKS == 1 )
        {
            /* In case call-backs are used, copy them from parent to child. */
            pxNewSocket->u.xTCP.pxHandleConnected = pxSocket->u.xTCP.pxHandleConnected;
            pxNewSocket->u.xTCP.pxHandleReceive = pxSocket->u.xTCP.pxHandleReceive;
            pxNewSocket->u.xTCP.pxHandleSent = pxSocket->u.xTCP.pxHandleSent;
        }
        #endif /* ipconfigUSE_CALLBACKS */

        #if ( ipconfigSUPPORT_SELECT_FUNCTION == 1 )
        {
            /* Child socket of listening sockets will inherit the Socket Set
             * Otherwise the owner has no chance of including it into the set. */
            if( pxSocket->pxSocketSet != NULL )
            {
                pxNewSocket->pxSocketSet = pxSocket->pxSocketSet;
                pxNewSocket->xSelectBits = pxSocket->xSelectBits | ( ( EventBits_t ) eSELECT_READ ) | ( ( EventBits_t ) eSELECT_EXCEPT );
            }
        }
        #endif /* ipconfigSUPPORT_SELECT_FUNCTION */

        /* And bind it to the same local port as its parent. */
        ( void ) FreeRTOS_GetLocalAddress( pxSocket, &xAddress );

        #if ( ipconfigTCP_HANG_PROTECTION == 1 )
        {
            /* Only when there is anti-hanging protection, a socket may become an
             * orphan temporarily.  Once this socket is really connected, the owner of
             * the server socket will be notified. */

            /* When bPassQueued is true, the socket is an orphan until it gets
             * connected. */
            pxNewSocket->u.xTCP.bits.bPassQueued = pdTRUE_UNSIGNED;
            pxNewSocket->u.xTCP.pxPeerSocket = pxSocket;
        }
        #else
        {
            /* A reference to the new socket may be stored and the socket is marked
             * as 'passable'. */

            /* When bPassAccept is true, this socket may be returned in a call to
             * accept(). */
            pxNewSocket->u.xTCP.bits.bPassAccept = pdTRUE_UNSIGNED;

            if( pxSocket->u.xTCP.pxPeerSocket == NULL )
            {
                pxSocket->u.xTCP.pxPeerSocket = pxNewSocket;
            }
        }
        #endif /* if ( ipconfigTCP_HANG_PROTECTION == 1 ) */

        pxSocket->u.xTCP.usChildCount++;

        if( pxSocket->u.xTCP.pxPeerSocket == NULL )
        {
            pxSocket->u.xTCP.pxPeerSocket = pxNewSocket;
        }

        FreeRTOS_debug_printf( ( "Gain: Socket %u now has %u / %u child%s me: %p parent: %p peer: %p\n",
                                 pxSocket->usLocalPort,
                                 pxSocket->u.xTCP.usChildCount,
                                 pxSocket->u.xTCP.usBacklog,
                                 ( pxSocket->u.xTCP.usChildCount == 1U ) ? "" : "ren",
                                 ( void * ) pxNewSocket,
                                 ( void * ) pxSocket,
                                 pxSocket ? ( void * ) pxSocket->u.xTCP.pxPeerSocket : NULL ) );

        /* Now bind the child socket to the same port as the listening socket. */
        if( vSocketBind( pxNewSocket, &xAddress, sizeof( xAddress ), pdTRUE ) != 0 )
        {
            FreeRTOS_debug_printf( ( "TCP: Listen: new socket bind error\n" ) );
            ( void ) vSocketClose( pxNewSocket );
            xResult = pdFALSE;
        }
        else
        {
            xResult = pdTRUE;
        }

        return xResult;
    }
    /*-----------------------------------------------------------*/

    #if ( ( ipconfigHAS_DEBUG_PRINTF != 0 ) || ( ipconfigHAS_PRINTF != 0 ) )

        const char * FreeRTOS_GetTCPStateName( UBaseType_t ulState )
        {
            static const char * const pcStateNames[] =
            {
                "eCLOSED",
                "eTCP_LISTEN",
                "eCONNECT_SYN",
                "eSYN_FIRST",
                "eSYN_RECEIVED",
                "eESTABLISHED",
                "eFIN_WAIT_1",
                "eFIN_WAIT_2",
                "eCLOSE_WAIT",
                "eCLOSING",
                "eLAST_ACK",
                "eTIME_WAIT",
                "eUNKNOWN",
            };
            BaseType_t xIndex = ( BaseType_t ) ulState;

            if( ( xIndex < 0 ) || ( xIndex >= ARRAY_SIZE( pcStateNames ) ) )
            {
                /* The last item is called 'eUNKNOWN' */
                xIndex = ARRAY_SIZE( pcStateNames );
                xIndex--;
            }

            return pcStateNames[ xIndex ];
        }

    #endif /* ( ( ipconfigHAS_DEBUG_PRINTF != 0 ) || ( ipconfigHAS_PRINTF != 0 ) ) */
    /*-----------------------------------------------------------*/

#endif /* ipconfigUSE_TCP == 1 */
