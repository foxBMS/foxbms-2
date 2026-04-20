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
 * @file FreeRTOS_TCP_IP.c
 * @brief Module which handles the TCP connections for FreeRTOS+TCP.
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
#include "FreeRTOS_IP_Utils.h"
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



/** @brief When closing a socket an event is posted to the Network Event Queue.
 *         If the queue is full, then the event is not posted and the socket
 *         can be orphaned. To prevent this, the below variable is used to keep
 *         track of any socket which needs to be closed. This variable can be
 *         accessed by the IP task only. Thus, preventing any race condition.
 */
    /* MISRA Ref 8.9.1 [File scoped variables] */
    /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-89 */
    /* coverity[misra_c_2012_rule_8_9_violation] */
    static FreeRTOS_Socket_t * xSocketToClose = NULL;

/** @brief When a connection is coming in on a reusable socket, and the
 *         SYN phase times out, the socket must be put back into eTCP_LISTEN
 *         mode, so it can accept a new connection again.
 *         This variable can be accessed by the IP task only. Thus, preventing any
 *         race condition.
 */
    /* MISRA Ref 8.9.1 [File scoped variables] */
    /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-89 */
    /* coverity[misra_c_2012_rule_8_9_violation] */
    _static FreeRTOS_Socket_t * xSocketToListen = NULL;

    #if ( ipconfigHAS_DEBUG_PRINTF != 0 )

/*
 * For logging and debugging: make a string showing the TCP flags.
 */
        const char * prvTCPFlagMeaning( UBaseType_t xFlags );
    #endif /* ipconfigHAS_DEBUG_PRINTF != 0 */

    static IPv46_Address_t xGetSourceAddrFromBuffer( const uint8_t * const pucEthernetBuffer );

/*-----------------------------------------------------------*/


/** @brief Close the socket another time.
 *
 * @param[in] pxSocket The socket to be checked.
 */
    /* coverity[single_use] */
    void vSocketCloseNextTime( FreeRTOS_Socket_t * pxSocket )
    {
        if( ( xSocketToClose != NULL ) && ( xSocketToClose != pxSocket ) )
        {
            ( void ) vSocketClose( xSocketToClose );
        }

        xSocketToClose = pxSocket;
    }
    /*-----------------------------------------------------------*/

/** @brief Postpone a call to FreeRTOS_listen() to avoid recursive calls.
 *
 * @param[in] pxSocket The socket to be checked.
 */
    /* coverity[single_use] */
    void vSocketListenNextTime( FreeRTOS_Socket_t * pxSocket )
    {
        if( ( xSocketToListen != NULL ) && ( xSocketToListen != pxSocket ) )
        {
            ( void ) FreeRTOS_listen( ( Socket_t ) xSocketToListen, ( BaseType_t ) ( xSocketToListen->u.xTCP.usBacklog ) );
        }

        xSocketToListen = pxSocket;
    }
    /*-----------------------------------------------------------*/

/**
 * @brief As soon as a TCP socket timer expires, this function will be called
 *       (from xTCPTimerCheck). It can send a delayed ACK or new data.
 *
 * @param[in] pxSocket socket to be checked.
 *
 * @return 0 on success, a negative error code on failure. A negative value will be
 *         returned in case the hang-protection has put the socket in a wait-close state.
 *
 * @note Sequence of calling (normally) :
 *   IP-Task:
 *      xTCPTimerCheck()                // Check all sockets ( declared in FreeRTOS_Sockets.c )
 *      xTCPSocketCheck()               // Either send a delayed ACK or call prvTCPSendPacket()
 *      prvTCPSendPacket()              // Either send a SYN or call prvTCPSendRepeated ( regular messages )
 *      prvTCPSendRepeated()            // Send at most 8 messages on a row
 *          prvTCPReturnPacket()        // Prepare for returning
 *          xNetworkInterfaceOutput()   // Sends data to the NIC ( declared in portable/NetworkInterface/xxx )
 */
    BaseType_t xTCPSocketCheck( FreeRTOS_Socket_t * pxSocket )
    {
        BaseType_t xResult = 0;
        BaseType_t xReady = pdFALSE;

        if( ( pxSocket->u.xTCP.eTCPState >= eESTABLISHED ) && ( pxSocket->u.xTCP.txStream != NULL ) )
        {
            /* The API FreeRTOS_send() might have added data to the TX stream.  Add
             * this data to the windowing system so it can be transmitted. */
            prvTCPAddTxData( pxSocket );
        }

        #if ( ipconfigUSE_TCP_WIN == 1 )
        {
            if( pxSocket->u.xTCP.pxAckMessage != NULL )
            {
                /* The first task of this regular socket check is to send-out delayed
                 * ACK's. */
                if( pxSocket->u.xTCP.bits.bUserShutdown == pdFALSE_UNSIGNED )
                {
                    /* Earlier data was received but not yet acknowledged.  This
                     * function is called when the TCP timer for the socket expires, the
                     * ACK may be sent now. */
                    if( pxSocket->u.xTCP.eTCPState != eCLOSED )
                    {
                        if( ( xTCPWindowLoggingLevel > 1 ) && ipconfigTCP_MAY_LOG_PORT( pxSocket->usLocalPort ) )
                        {
                            FreeRTOS_debug_printf( ( "Send[%u->%u] del ACK %u SEQ %u (len %u)\n",
                                                     pxSocket->usLocalPort,
                                                     pxSocket->u.xTCP.usRemotePort,
                                                     ( unsigned ) ( pxSocket->u.xTCP.xTCPWindow.rx.ulCurrentSequenceNumber - pxSocket->u.xTCP.xTCPWindow.rx.ulFirstSequenceNumber ),
                                                     ( unsigned ) ( pxSocket->u.xTCP.xTCPWindow.ulOurSequenceNumber - pxSocket->u.xTCP.xTCPWindow.tx.ulFirstSequenceNumber ),
                                                     ( unsigned ) ( uxIPHeaderSizeSocket( pxSocket ) + ipSIZE_OF_TCP_HEADER ) ) );
                        }

                        prvTCPReturnPacket( pxSocket, pxSocket->u.xTCP.pxAckMessage, ( uint32_t ) ( uxIPHeaderSizeSocket( pxSocket ) + ipSIZE_OF_TCP_HEADER ), ipconfigZERO_COPY_TX_DRIVER );

                        #if ( ipconfigZERO_COPY_TX_DRIVER != 0 )
                        {
                            /* The ownership has been passed to the SEND routine,
                             * clear the pointer to it. */
                            pxSocket->u.xTCP.pxAckMessage = NULL;
                        }
                        #endif /* ipconfigZERO_COPY_TX_DRIVER */
                    }

                    if( prvTCPNextTimeout( pxSocket ) > 1U )
                    {
                        /* Tell the code below that this function is ready. */
                        xReady = pdTRUE;
                    }
                }
                else
                {
                    /* The user wants to perform an active shutdown(), skip sending
                     * the delayed ACK.  The function prvTCPSendPacket() will send the
                     * FIN along with the ACK's. */
                }

                if( pxSocket->u.xTCP.pxAckMessage != NULL )
                {
                    vReleaseNetworkBufferAndDescriptor( pxSocket->u.xTCP.pxAckMessage );
                    pxSocket->u.xTCP.pxAckMessage = NULL;
                }
            }
        }
        #endif /* ipconfigUSE_TCP_WIN */

        if( xReady == pdFALSE )
        {
            /* The second task of this regular socket check is sending out data. */
            if( ( pxSocket->u.xTCP.eTCPState >= eESTABLISHED ) ||
                ( pxSocket->u.xTCP.eTCPState == eCONNECT_SYN ) )
            {
                ( void ) prvTCPSendPacket( pxSocket );
            }

            /* Set the time-out for the next wakeup for this socket. */
            ( void ) prvTCPNextTimeout( pxSocket );

            #if ( ipconfigTCP_HANG_PROTECTION == 1 )
            {
                /* In all (non-connected) states in which keep-alive messages can not be sent
                 * the anti-hang protocol will close sockets that are 'hanging'. */
                xResult = prvTCPStatusAgeCheck( pxSocket );
            }
            #endif
        }

        return xResult;
    }
    /*-----------------------------------------------------------*/

/**
 * @brief 'Touch' the socket to keep it alive/updated.
 *
 * @param[in] pxSocket The socket to be updated.
 *
 * @note This is used for anti-hanging protection and TCP keep-alive messages.
 *       Called in two places: after receiving a packet and after a state change.
 *       The socket's alive timer may be reset.
 */
    void prvTCPTouchSocket( struct xSOCKET * pxSocket )
    {
        #if ( ipconfigTCP_HANG_PROTECTION == 1 )
        {
            pxSocket->u.xTCP.xLastActTime = xTaskGetTickCount();
        }
        #endif

        #if ( ipconfigTCP_KEEP_ALIVE == 1 )
        {
            pxSocket->u.xTCP.bits.bWaitKeepAlive = pdFALSE_UNSIGNED;
            pxSocket->u.xTCP.bits.bSendKeepAlive = pdFALSE_UNSIGNED;
            pxSocket->u.xTCP.ucKeepRepCount = 0U;
            pxSocket->u.xTCP.xLastAliveTime = xTaskGetTickCount();
        }
        #endif

        ( void ) pxSocket;
    }
    /*-----------------------------------------------------------*/

    static BaseType_t vTCPRemoveTCPChild( const FreeRTOS_Socket_t * pxChildSocket )
    {
        BaseType_t xReturn = pdFALSE;

        /* MISRA Ref 11.3.1 [Misaligned access] */
        /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
        /* coverity[misra_c_2012_rule_11_3_violation] */
        const ListItem_t * pxEnd = ( ( const ListItem_t * ) &( xBoundTCPSocketsList.xListEnd ) );

        /* MISRA Ref 11.3.1 [Misaligned access] */
        /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
        /* coverity[misra_c_2012_rule_11_3_violation] */
        const ListItem_t * pxIterator = ( const ListItem_t * ) listGET_HEAD_ENTRY( &xBoundTCPSocketsList );

        while( pxIterator != pxEnd )
        {
            FreeRTOS_Socket_t * pxSocket;
            pxSocket = ( ( FreeRTOS_Socket_t * ) listGET_LIST_ITEM_OWNER( pxIterator ) );
            pxIterator = ( ListItem_t * ) listGET_NEXT( pxIterator );

            if( ( pxSocket != pxChildSocket ) && ( pxSocket->usLocalPort == pxChildSocket->usLocalPort ) )
            {
                if( pxSocket->u.xTCP.pxPeerSocket == pxChildSocket ) /**< for server socket: child, for child socket: parent */
                {
                    pxSocket->u.xTCP.pxPeerSocket = NULL;
                    xReturn = pdTRUE;
                    break;
                }
            }
        }

        return xReturn;
    }

/**
 * @brief Changing to a new state. Centralised here to do specific actions such as
 *        resetting the alive timer, calling the user's OnConnect handler to notify
 *        that a socket has got (dis)connected, and setting bit to unblock a call to
 *        FreeRTOS_select().
 *
 * @param[in] pxSocket The socket whose state we are trying to change.
 * @param[in] eTCPState The state to which we want to change to.
 */
    void vTCPStateChange( FreeRTOS_Socket_t * pxSocket,
                          enum eTCP_STATE eTCPState )
    {
        FreeRTOS_Socket_t * xParent = pxSocket;
        BaseType_t bBefore = tcpNOW_CONNECTED( ( BaseType_t ) pxSocket->u.xTCP.eTCPState ); /* Was it connected ? */
        BaseType_t bAfter = tcpNOW_CONNECTED( ( BaseType_t ) eTCPState );                   /* Is it connected now ? */

        eIPTCPState_t xPreviousState = pxSocket->u.xTCP.eTCPState;

        #if ( ipconfigUSE_CALLBACKS == 1 )
            FreeRTOS_Socket_t * xConnected = NULL;
        #endif

        if( ( ( xPreviousState == eCONNECT_SYN ) ||
              ( xPreviousState == eSYN_FIRST ) ||
              ( xPreviousState == eSYN_RECEIVED ) ) &&
            ( eTCPState == eCLOSE_WAIT ) )
        {
            /* A socket was in the connecting phase but something
             * went wrong and it should be closed. */
            #if ( ipconfigHAS_DEBUG_PRINTF != 0 )
                FreeRTOS_debug_printf( ( "Move from %s to %s\n",
                                         FreeRTOS_GetTCPStateName( ( UBaseType_t ) xPreviousState ),
                                         FreeRTOS_GetTCPStateName( eTCPState ) ) );
            #endif

            /* Set the flag to show that it was connected before and that the
             * status has changed now. This will cause the control flow to go
             * in the below if condition.*/
            bBefore = pdTRUE;
        }

        /* Has the connected status changed? */
        if( bBefore != bAfter )
        {
            /* if bPassQueued is true, this socket is an orphan until it gets connected. */
            if( pxSocket->u.xTCP.bits.bPassQueued != pdFALSE_UNSIGNED )
            {
                /* Find it's parent if the reuse bit is not set. */
                if( pxSocket->u.xTCP.bits.bReuseSocket == pdFALSE_UNSIGNED )
                {
                    xParent = pxSocket->u.xTCP.pxPeerSocket;
                    configASSERT( xParent != NULL );
                }
            }

            /* Is the socket connected now ? */
            if( bAfter != pdFALSE )
            {
                /* if bPassQueued is true, this socket is an orphan until it gets connected. */
                if( pxSocket->u.xTCP.bits.bPassQueued != pdFALSE_UNSIGNED )
                {
                    if( xParent != NULL )
                    {
                        /* The child socket has got connected.  See if the parent
                         * ( the listening socket ) should be signalled, or if a
                         * call-back must be made, in which case 'xConnected' will
                         * be set to the parent socket. */

                        if( xParent->u.xTCP.pxPeerSocket == NULL )
                        {
                            xParent->u.xTCP.pxPeerSocket = pxSocket;
                        }

                        xParent->xEventBits |= ( EventBits_t ) eSOCKET_ACCEPT;

                        #if ( ipconfigSUPPORT_SELECT_FUNCTION == 1 )
                        {
                            /* Library support FreeRTOS_select().  Receiving a new
                             * connection is being translated as a READ event. */
                            if( ( xParent->xSelectBits & ( ( EventBits_t ) eSELECT_READ ) ) != 0U )
                            {
                                xParent->xEventBits |= ( ( EventBits_t ) eSELECT_READ ) << SOCKET_EVENT_BIT_COUNT;
                            }
                        }
                        #endif

                        #if ( ipconfigUSE_CALLBACKS == 1 )
                        {
                            if( ( ipconfigIS_VALID_PROG_ADDRESS( xParent->u.xTCP.pxHandleConnected ) ) &&
                                ( xParent->u.xTCP.bits.bReuseSocket == pdFALSE_UNSIGNED ) )
                            {
                                /* The listening socket does not become connected itself, in stead
                                 * a child socket is created.
                                 * Postpone a call the OnConnect event until the end of this function. */
                                xConnected = xParent;
                            }
                        }
                        #endif
                    }

                    /* Don't need to access the parent socket anymore, so the
                     * reference 'pxPeerSocket' may be cleared. */
                    pxSocket->u.xTCP.pxPeerSocket = NULL;
                    pxSocket->u.xTCP.bits.bPassQueued = pdFALSE_UNSIGNED;

                    /* When true, this socket may be returned in a call to accept(). */
                    pxSocket->u.xTCP.bits.bPassAccept = pdTRUE_UNSIGNED;
                }
                else
                {
                    /* An active connect() has succeeded. In this case there is no
                     * ( listening ) parent socket. Signal the now connected socket. */

                    pxSocket->xEventBits |= ( EventBits_t ) eSOCKET_CONNECT;

                    #if ( ipconfigSUPPORT_SELECT_FUNCTION == 1 )
                    {
                        if( ( pxSocket->xSelectBits & ( ( EventBits_t ) eSELECT_WRITE ) ) != 0U )
                        {
                            pxSocket->xEventBits |= ( ( EventBits_t ) eSELECT_WRITE ) << SOCKET_EVENT_BIT_COUNT;
                        }
                    }
                    #endif
                }
            }
            else /* bAfter == pdFALSE, connection is closed. */
            {
                /* Notify/wake-up the socket-owner by setting the event bits. */
                xParent->xEventBits |= ( EventBits_t ) eSOCKET_CLOSED;

                #if ( ipconfigSUPPORT_SELECT_FUNCTION == 1 )
                {
                    if( ( xParent->xSelectBits & ( EventBits_t ) eSELECT_EXCEPT ) != 0U )
                    {
                        xParent->xEventBits |= ( ( EventBits_t ) eSELECT_EXCEPT ) << SOCKET_EVENT_BIT_COUNT;
                    }
                }
                #endif
            }

            #if ( ipconfigUSE_CALLBACKS == 1 )
            {
                if( ( ipconfigIS_VALID_PROG_ADDRESS( pxSocket->u.xTCP.pxHandleConnected ) ) && ( xConnected == NULL ) )
                {
                    /* The 'connected' state has changed, call the user handler. */
                    xConnected = pxSocket;
                }
            }
            #endif /* ipconfigUSE_CALLBACKS */

            if( prvTCPSocketIsActive( pxSocket->u.xTCP.eTCPState ) == 0 )
            {
                /* Now the socket isn't in an active state anymore so it
                 * won't need further attention of the IP-task.
                 * Setting time-out to zero means that the socket won't get checked during
                 * timer events. */
                pxSocket->u.xTCP.usTimeout = 0U;
            }
        }

        /* Fill in the new state. */
        pxSocket->u.xTCP.eTCPState = eTCPState;

        if( ( eTCPState == eCLOSED ) ||
            ( eTCPState == eCLOSE_WAIT ) )
        {
            BaseType_t xMustClear = pdFALSE;
            BaseType_t xHasCleared = pdFALSE;

            if( ( xParent == pxSocket ) && ( pxSocket->u.xTCP.pxPeerSocket != NULL ) )
            {
                xParent = pxSocket->u.xTCP.pxPeerSocket;
            }

            if( ( xParent->u.xTCP.pxPeerSocket != NULL ) &&
                ( xParent->u.xTCP.pxPeerSocket == pxSocket ) )
            {
                xMustClear = pdTRUE;
                ( void ) xMustClear;
            }

            /* Socket goes to status eCLOSED because of a RST.
             * When nobody owns the socket yet, delete it. */
            FreeRTOS_printf( ( "vTCPStateChange: Closing (Queued %d, Accept %d Reuse %d)\n",
                               pxSocket->u.xTCP.bits.bPassQueued,
                               pxSocket->u.xTCP.bits.bPassAccept,
                               pxSocket->u.xTCP.bits.bReuseSocket ) );
            FreeRTOS_printf( ( "vTCPStateChange: me %p parent %p peer %p clear %d\n",
                               ( void * ) pxSocket,
                               ( void * ) xParent,
                               xParent ? ( void * ) xParent->u.xTCP.pxPeerSocket : NULL,
                               ( int ) xMustClear ) );

            vTaskSuspendAll();
            {
                if( ( pxSocket->u.xTCP.bits.bPassQueued != pdFALSE_UNSIGNED ) ||
                    ( pxSocket->u.xTCP.bits.bPassAccept != pdFALSE_UNSIGNED ) )
                {
                    if( pxSocket->u.xTCP.bits.bReuseSocket == pdFALSE_UNSIGNED )
                    {
                        xHasCleared = vTCPRemoveTCPChild( pxSocket );
                        ( void ) xHasCleared;

                        pxSocket->u.xTCP.bits.bPassQueued = pdFALSE_UNSIGNED;
                        pxSocket->u.xTCP.bits.bPassAccept = pdFALSE_UNSIGNED;
                        configASSERT( xIsCallingFromIPTask() != pdFALSE );
                        vSocketCloseNextTime( pxSocket );
                    }
                }
            }
            ( void ) xTaskResumeAll();
            FreeRTOS_printf( ( "vTCPStateChange: xHasCleared = %d\n",
                               ( int ) xHasCleared ) );
        }

        if( ( eTCPState == eCLOSE_WAIT ) && ( pxSocket->u.xTCP.bits.bReuseSocket == pdTRUE_UNSIGNED ) )
        {
            switch( xPreviousState )
            {
                case eSYN_FIRST:    /* 3 (server) Just created, must ACK the SYN request */
                case eSYN_RECEIVED: /* 4 (server) waiting for a confirming connection request */
                    FreeRTOS_debug_printf( ( "Restoring a reuse socket port %u\n", pxSocket->usLocalPort ) );

                    /* Go back into listening mode. Set the TCP status to 'eCLOSED',
                     * otherwise FreeRTOS_listen() will refuse the action. */
                    pxSocket->u.xTCP.eTCPState = eCLOSED;

                    /* vSocketListenNextTime() makes sure that FreeRTOS_listen() will be called
                     * before the IP-task handles any new message. */
                    vSocketListenNextTime( pxSocket );
                    break;

                default:
                    /* Nothing to do. */
                    break;
            }
        }

        /* Touch the alive timers because moving to another state. */
        prvTCPTouchSocket( pxSocket );

        #if ( ipconfigHAS_DEBUG_PRINTF == 1 )
        {
            if( ( xTCPWindowLoggingLevel >= 0 ) && ( ipconfigTCP_MAY_LOG_PORT( pxSocket->usLocalPort ) ) )
            {
                char pcBuffer[ 40 ];

                switch( pxSocket->bits.bIsIPv6 ) /* LCOV_EXCL_BR_LINE */
                {
                    #if ( ipconfigUSE_IPv4 != 0 )
                        case pdFALSE_UNSIGNED:
                           {
                               uint32_t ulIPAddress = FreeRTOS_ntohl( pxSocket->u.xTCP.xRemoteIP.ulIP_IPv4 );
                               FreeRTOS_inet_ntop( FREERTOS_AF_INET4,
                                                   ( const uint8_t * ) &ulIPAddress,
                                                   pcBuffer,
                                                   sizeof( pcBuffer ) );
                           }
                           break;
                    #endif /* ( ipconfigUSE_IPv4 != 0 ) */

                    #if ( ipconfigUSE_IPv6 != 0 )
                        case pdTRUE_UNSIGNED:
                            FreeRTOS_inet_ntop( FREERTOS_AF_INET6,
                                                pxSocket->u.xTCP.xRemoteIP.xIP_IPv6.ucBytes,
                                                pcBuffer,
                                                sizeof( pcBuffer ) );
                            break;
                    #endif /* ( ipconfigUSE_IPv6 != 0 ) */

                    default:   /* LCOV_EXCL_LINE */
                        /* MISRA 16.4 Compliance */
                        break; /* LCOV_EXCL_LINE */
                }

                FreeRTOS_debug_printf( ( "Socket %u -> [%s]:%u State %s->%s\n",
                                         pxSocket->usLocalPort,
                                         pcBuffer,
                                         pxSocket->u.xTCP.usRemotePort,
                                         FreeRTOS_GetTCPStateName( ( UBaseType_t ) xPreviousState ),
                                         FreeRTOS_GetTCPStateName( ( UBaseType_t ) eTCPState ) ) );
            }
        }
        #endif /* ipconfigHAS_DEBUG_PRINTF */

        #if ( ipconfigUSE_CALLBACKS == 1 )
        {
            if( xConnected != NULL )
            {
                /* The 'connected' state has changed, call the OnConnect handler of the parent. */
                xConnected->u.xTCP.pxHandleConnected( ( Socket_t ) xConnected, bAfter );
            }
        }
        #endif

        if( xParent != NULL )
        {
            vSocketWakeUpUser( xParent );
        }
    }
    /*-----------------------------------------------------------*/


/**
 * @brief Calculate after how much time this socket needs to be checked again.
 *
 * @param[in] pxSocket The socket to be checked.
 *
 * @return The number of clock ticks before the timer expires.
 */
    TickType_t prvTCPNextTimeout( struct xSOCKET * pxSocket )
    {
        TickType_t ulDelayMs = ( TickType_t ) tcpMAXIMUM_TCP_WAKEUP_TIME_MS;

        if( pxSocket->u.xTCP.eTCPState == eCONNECT_SYN )
        {
            /* The socket is actively connecting to a peer. */
            if( pxSocket->u.xTCP.bits.bConnPrepared != pdFALSE_UNSIGNED )
            {
                /* Ethernet address has been found, use progressive timeout for
                 * active connect(). */
                if( pxSocket->u.xTCP.ucRepCount < 3U )
                {
                    if( pxSocket->u.xTCP.ucRepCount == 0U )
                    {
                        ulDelayMs = 0U;
                    }
                    else
                    {
                        ulDelayMs = ( ( uint32_t ) 3000U ) << ( pxSocket->u.xTCP.ucRepCount - 1U );
                    }
                }
                else
                {
                    ulDelayMs = 11000U;
                }
            }
            else
            {
                /* Still in the Resolution phase: check every half second. */
                ulDelayMs = 500U;
            }

            FreeRTOS_debug_printf( ( "Connect[%xip:%u]: next timeout %u: %u ms\n",
                                     ( unsigned ) pxSocket->u.xTCP.xRemoteIP.ulIP_IPv4, pxSocket->u.xTCP.usRemotePort,
                                     pxSocket->u.xTCP.ucRepCount, ( unsigned ) ulDelayMs ) );
            pxSocket->u.xTCP.usTimeout = ( uint16_t ) ipMS_TO_MIN_TICKS( ulDelayMs );
        }
        else if( pxSocket->u.xTCP.usTimeout == 0U )
        {
            /* Let the sliding window mechanism decide what time-out is appropriate. */
            BaseType_t xResult = xTCPWindowTxHasData( &pxSocket->u.xTCP.xTCPWindow, pxSocket->u.xTCP.ulWindowSize, &ulDelayMs );

            if( ulDelayMs == 0U )
            {
                if( xResult != ( BaseType_t ) 0 )
                {
                    ulDelayMs = 1U;
                }
                else
                {
                    ulDelayMs = tcpMAXIMUM_TCP_WAKEUP_TIME_MS;
                }
            }
            else
            {
                /* ulDelayMs contains the time to wait before a re-transmission. */
            }

            pxSocket->u.xTCP.usTimeout = ( uint16_t ) ipMS_TO_MIN_TICKS( ulDelayMs ); /* LCOV_EXCL_BR_LINE ulDelayMs will not be smaller than 1 */
        }
        else
        {
            /* field '.usTimeout' has already been set (by the
             * keep-alive/delayed-ACK mechanism). */
        }

        /* Return the number of clock ticks before the timer expires. */
        return ( TickType_t ) pxSocket->u.xTCP.usTimeout;
    }
    /*-----------------------------------------------------------*/

/**
 * @brief IP frame agnostic helper to obtain the source IP Address from a buffer.
 *
 * @param[in] pucEthernetBuffer The Ethernet buffer from which the source address will be retrieved.
 *
 * @return IPv46_Address_t struct containing the source IP address.
 */
    static IPv46_Address_t xGetSourceAddrFromBuffer( const uint8_t * const pucEthernetBuffer )
    {
        IPv46_Address_t xSourceAddr;

        /* Map the buffer onto Ethernet Header struct for easy access to fields. */
        /* MISRA Ref 11.3.1 [Misaligned access] */
        /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
        /* coverity[misra_c_2012_rule_11_3_violation] */
        const EthernetHeader_t * pxHeader = ( ( const EthernetHeader_t * ) pucEthernetBuffer );

        if( pxHeader->usFrameType == ( uint16_t ) ipIPv6_FRAME_TYPE )
        {
            /* Map the ethernet buffer onto the IPHeader_t struct for easy access to the fields. */
            /* MISRA Ref 11.3.1 [Misaligned access] */
            /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
            /* coverity[misra_c_2012_rule_11_3_violation] */
            const IPHeader_IPv6_t * const pxIPHeader_IPv6 = ( ( const IPHeader_IPv6_t * ) &( pucEthernetBuffer[ ipSIZE_OF_ETH_HEADER ] ) );
            xSourceAddr.xIs_IPv6 = pdTRUE;
            ( void ) memcpy( xSourceAddr.xIPAddress.xIP_IPv6.ucBytes, pxIPHeader_IPv6->xSourceAddress.ucBytes, sizeof( IPv6_Address_t ) );
        }
        else
        {
            /* Map the ethernet buffer onto the IPHeader_t struct for easy access to the fields. */
            /* MISRA Ref 11.3.1 [Misaligned access] */
            /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
            /* coverity[misra_c_2012_rule_11_3_violation] */
            const IPHeader_t * const pxIPHeader = ( ( const IPHeader_t * ) &( pucEthernetBuffer[ ipSIZE_OF_ETH_HEADER ] ) );
            xSourceAddr.xIs_IPv6 = pdFALSE;
            xSourceAddr.xIPAddress.ulIP_IPv4 = FreeRTOS_htonl( pxIPHeader->ulSourceIPAddress );
        }

        return xSourceAddr;
    }

/**
 * @brief Process the received TCP packet.
 *
 * @param[in] pxDescriptor The descriptor in which the TCP packet is held.
 *
 * @return If the processing of the packet was successful, then pdPASS is returned
 *         or else pdFAIL.
 *
 * @note FreeRTOS_TCP_IP has only 2 public functions, this is the second one:
 *  xProcessReceivedTCPPacket()
 *      prvTCPHandleState()
 *          prvTCPPrepareSend()
 *              prvTCPReturnPacket()
 *              xNetworkInterfaceOutput()  // Sends data to the NIC
 *      prvTCPSendRepeated()
 *          prvTCPReturnPacket()        // Prepare for returning
 *          xNetworkInterfaceOutput()   // Sends data to the NIC
 */
    BaseType_t xProcessReceivedTCPPacket( NetworkBufferDescriptor_t * pxDescriptor )
    {
        BaseType_t xResult = pdPASS;
        /* Function might modify the parameter. */
        NetworkBufferDescriptor_t * pxNetworkBuffer;
        size_t uxIPHeaderOffset;

        configASSERT( pxDescriptor != NULL );
        configASSERT( pxDescriptor->pucEthernetBuffer != NULL );

        pxNetworkBuffer = pxDescriptor;
        uxIPHeaderOffset = ipSIZE_OF_ETH_HEADER + uxIPHeaderSizePacket( pxNetworkBuffer );

        /* Check for a minimum packet size. */
        if( pxNetworkBuffer->xDataLength < ( uxIPHeaderOffset + ipSIZE_OF_TCP_HEADER ) )
        {
            xResult = pdFAIL;
        }
        else
        {
            /* MISRA Ref 11.3.1 [Misaligned access] */
            /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
            /* coverity[misra_c_2012_rule_11_3_violation] */
            const TCPHeader_t * pxTCPHeader = ( ( const TCPHeader_t * )
                                                &( pxNetworkBuffer->pucEthernetBuffer[ uxIPHeaderOffset ] ) );

            const uint16_t ucTCPFlags = pxTCPHeader->ucTCPFlags;
            const uint16_t usLocalPort = FreeRTOS_htons( pxTCPHeader->usDestinationPort );
            const uint16_t usRemotePort = FreeRTOS_htons( pxTCPHeader->usSourcePort );
            const IPv46_Address_t xRemoteIP = xGetSourceAddrFromBuffer( pxNetworkBuffer->pucEthernetBuffer );

            /* Find the destination socket, and if not found: return a socket listening to
             * the destination PORT. */
            FreeRTOS_Socket_t * pxSocket = pxTCPSocketLookup( 0U, usLocalPort, xRemoteIP, usRemotePort );

            if( ( pxSocket == NULL ) || ( prvTCPSocketIsActive( pxSocket->u.xTCP.eTCPState ) == pdFALSE ) )
            {
                /* A TCP messages is received but either there is no socket with the
                 * given port number or the there is a socket, but it is in one of these
                 * non-active states:  eCLOSED, eCLOSE_WAIT, eFIN_WAIT_2, eCLOSING, or
                 * eTIME_WAIT. */

                FreeRTOS_debug_printf( ( "TCP: No active socket on port %d (%d)\n", usLocalPort, usRemotePort ) );

                /* Send a RST to all packets that can not be handled.  As a result
                 * the other party will get a ECONN error.  There are two exceptions:
                 * 1) A packet that already has the RST flag set.
                 * 2) A packet that only has the ACK flag set.
                 * A packet with only the ACK flag set might be the last ACK in
                 * a three-way hand-shake that closes a connection. */
                if( ( ( ucTCPFlags & tcpTCP_FLAG_CTRL ) != tcpTCP_FLAG_ACK ) &&
                    ( ( ucTCPFlags & tcpTCP_FLAG_RST ) == 0U ) )
                {
                    ( void ) prvTCPSendReset( pxNetworkBuffer );
                }

                /* The packet can't be handled. */
                xResult = pdFAIL;
            }
            else
            {
                pxSocket->u.xTCP.ucRepCount = 0U;

                if( pxSocket->u.xTCP.eTCPState == eTCP_LISTEN )
                {
                    /* The matching socket is in a listening state.  Test if the peer
                     * has set the SYN flag. */
                    if( ( ucTCPFlags & tcpTCP_FLAG_CTRL ) != tcpTCP_FLAG_SYN )
                    {
                        /* What happens: maybe after a reboot, a client doesn't know the
                         * connection had gone.  Send a RST in order to get a new connect
                         * request. */
                        #if ( ipconfigHAS_DEBUG_PRINTF == 1 )
                        {
                            FreeRTOS_debug_printf( ( "TCP: Server can't handle flags: %s from %u to port %u\n",
                                                     prvTCPFlagMeaning( ( UBaseType_t ) ucTCPFlags ), usRemotePort, usLocalPort ) );
                        }
                        #endif /* ipconfigHAS_DEBUG_PRINTF */

                        if( ( ucTCPFlags & tcpTCP_FLAG_RST ) == 0U )
                        {
                            ( void ) prvTCPSendReset( pxNetworkBuffer );
                        }

                        xResult = pdFAIL;
                    }
                    else
                    {
                        /* prvHandleListen() will either return a newly created socket
                         * (if bReuseSocket is false), otherwise it returns the current
                         * socket which will later get connected. */
                        pxSocket = prvHandleListen( pxSocket, pxNetworkBuffer );

                        if( pxSocket == NULL )
                        {
                            xResult = pdFAIL;
                        }
                    }
                } /* if( pxSocket->u.xTCP.eTCPState == eTCP_LISTEN ). */
                else
                {
                    /* This is not a socket in listening mode. Check for the RST
                     * flag. */
                    if( ( ucTCPFlags & tcpTCP_FLAG_RST ) != 0U )
                    {
                        FreeRTOS_debug_printf( ( "TCP: RST received from %u for %u\n", usRemotePort, usLocalPort ) );

                        /* Implement https://tools.ietf.org/html/rfc5961#section-3.2. */
                        if( pxSocket->u.xTCP.eTCPState == eCONNECT_SYN )
                        {
                            const uint32_t ulAckNumber = FreeRTOS_ntohl( pxTCPHeader->ulAckNr );

                            /* Per the above RFC, "In the SYN-SENT state ... the RST is
                             * acceptable if the ACK field acknowledges the SYN." */
                            if( ulAckNumber == ( pxSocket->u.xTCP.xTCPWindow.ulOurSequenceNumber + 1U ) )
                            {
                                vTCPStateChange( pxSocket, eCLOSED );
                            }
                        }
                        else
                        {
                            const uint32_t ulSequenceNumber = FreeRTOS_ntohl( pxTCPHeader->ulSequenceNumber );

                            /* Check whether the packet matches the next expected sequence number. */
                            if( ulSequenceNumber == pxSocket->u.xTCP.xTCPWindow.rx.ulCurrentSequenceNumber )
                            {
                                vTCPStateChange( pxSocket, eCLOSED );
                            }
                            /* Otherwise, check whether the packet is within the receive window. */
                            else if( ( xSequenceGreaterThan( ulSequenceNumber, pxSocket->u.xTCP.xTCPWindow.rx.ulCurrentSequenceNumber ) != pdFALSE ) &&
                                     ( xSequenceLessThan( ulSequenceNumber, pxSocket->u.xTCP.xTCPWindow.rx.ulCurrentSequenceNumber +
                                                          pxSocket->u.xTCP.xTCPWindow.xSize.ulRxWindowLength ) != pdFALSE ) )
                            {
                                /* Send a challenge ACK. */
                                ( void ) prvTCPSendChallengeAck( pxNetworkBuffer );
                            }
                            else
                            {
                                /* Nothing. */
                            }
                        }

                        /* Otherwise, do nothing. In any case, the packet cannot be handled. */
                        xResult = pdFAIL;
                    }
                    /* Check whether there is a pure SYN amongst the TCP flags while the connection is established. */
                    else if( ( ( ucTCPFlags & tcpTCP_FLAG_CTRL ) == tcpTCP_FLAG_SYN ) && ( pxSocket->u.xTCP.eTCPState >= eESTABLISHED ) )
                    {
                        /* SYN flag while this socket is already connected. */
                        FreeRTOS_debug_printf( ( "TCP: SYN unexpected from %u\n", usRemotePort ) );

                        /* The packet cannot be handled. */
                        xResult = pdFAIL;
                    }
                    else
                    {
                        /* Update the copy of the TCP header only (skipping eth and IP
                         * headers).  It might be used later on, whenever data must be sent
                         * to the peer. */
                        const size_t uxOffset = ipSIZE_OF_ETH_HEADER + uxIPHeaderSizeSocket( pxSocket );
                        ( void ) memcpy( ( void * ) ( &( pxSocket->u.xTCP.xPacket.u.ucLastPacket[ uxOffset ] ) ),
                                         ( const void * ) ( &( pxNetworkBuffer->pucEthernetBuffer[ uxOffset ] ) ),
                                         ipSIZE_OF_TCP_HEADER );
                        /* Clear flags that are set by the peer, and set the ACK flag. */
                        pxSocket->u.xTCP.xPacket.u.ucLastPacket[ uxOffset + ipTCP_FLAGS_OFFSET ] = tcpTCP_FLAG_ACK;
                    }
                }
            }

            if( xResult != pdFAIL )
            {
                uint16_t usWindow;

                /* pxSocket is not NULL when xResult != pdFAIL. */
                configASSERT( pxSocket != NULL ); /* LCOV_EXCL_LINE ,this branch will not be hit*/

                /* Touch the alive timers because we received a message for this
                 * socket. */
                prvTCPTouchSocket( pxSocket );

                /* Parse the TCP option(s), if present. */

                /* _HT_ : if we're in the SYN phase, and peer does not send a MSS option,
                 * then we MUST assume an MSS size of 536 bytes for backward compatibility. */

                /* When there are no TCP options, the TCP offset equals 20 bytes, which is stored as
                 * the number 5 (words) in the higher nibble of the TCP-offset byte. */
                if( ( pxTCPHeader->ucTCPOffset & tcpTCP_OFFSET_LENGTH_BITS ) > tcpTCP_OFFSET_STANDARD_LENGTH )
                {
                    xResult = prvCheckOptions( pxSocket, pxNetworkBuffer );
                }

                if( xResult != pdFAIL )
                {
                    usWindow = FreeRTOS_ntohs( pxTCPHeader->usWindow );
                    pxSocket->u.xTCP.ulWindowSize = ( uint32_t ) usWindow;
                    #if ( ipconfigUSE_TCP_WIN == 1 )
                    {
                        /* rfc1323 : The Window field in a SYN (i.e., a <SYN> or <SYN,ACK>)
                         * segment itself is never scaled. */
                        if( ( ucTCPFlags & ( uint8_t ) tcpTCP_FLAG_SYN ) == 0U )
                        {
                            pxSocket->u.xTCP.ulWindowSize =
                                ( pxSocket->u.xTCP.ulWindowSize << pxSocket->u.xTCP.ucPeerWinScaleFactor );
                        }
                    }
                    #endif /* ipconfigUSE_TCP_WIN */

                    /* In prvTCPHandleState() the incoming messages will be handled
                     * depending on the current state of the connection. */
                    if( prvTCPHandleState( pxSocket, &pxNetworkBuffer ) > 0 )
                    {
                        /* prvTCPHandleState() has sent a message, see if there are more to
                         * be transmitted. */
                        #if ( ipconfigUSE_TCP_WIN == 1 )
                        {
                            ( void ) prvTCPSendRepeated( pxSocket, &pxNetworkBuffer );
                        }
                        #endif /* ipconfigUSE_TCP_WIN */
                    }

                    if( pxNetworkBuffer != NULL )
                    {
                        /* We must check if the buffer is unequal to NULL, because the
                         * socket might keep a reference to it in case a delayed ACK must be
                         * sent. */
                        vReleaseNetworkBufferAndDescriptor( pxNetworkBuffer );
                        #ifndef _lint
                            /* Clear pointers that are freed. */
                            pxNetworkBuffer = NULL;
                        #endif
                    }

                    /* And finally, calculate when this socket wants to be woken up. */
                    ( void ) prvTCPNextTimeout( pxSocket );
                }
            }
        }

        /* pdPASS being returned means the buffer has been consumed. */
        return xResult;
    }
    /*-----------------------------------------------------------*/


/**
 * @brief In the API accept(), the user asks is there is a new client? As API's can
 *        not walk through the xBoundTCPSocketsList the IP-task will do this.
 *
 * @param[in] pxSocket The socket for which the bound socket list will be iterated.
 *
 * @return if there is a new client, then pdTRUE is returned or else, pdFALSE.
 */
    BaseType_t xTCPCheckNewClient( FreeRTOS_Socket_t * pxSocket )
    {
        TickType_t uxLocalPort = ( TickType_t ) FreeRTOS_htons( pxSocket->usLocalPort );
        const ListItem_t * pxIterator;
        FreeRTOS_Socket_t * pxFound;
        BaseType_t xResult = pdFALSE;

        /* MISRA Ref 11.3.1 [Misaligned access] */
        /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
        /* coverity[misra_c_2012_rule_11_3_violation] */
        const ListItem_t * pxEndTCP = ( ( const ListItem_t * ) &( xBoundTCPSocketsList.xListEnd ) );

        /* Here xBoundTCPSocketsList can be accessed safely IP-task is the only one
         * who has access. */
        for( pxIterator = ( const ListItem_t * ) listGET_HEAD_ENTRY( &xBoundTCPSocketsList );
             pxIterator != pxEndTCP;
             pxIterator = ( const ListItem_t * ) listGET_NEXT( pxIterator ) )
        {
            if( listGET_LIST_ITEM_VALUE( pxIterator ) == ( configLIST_VOLATILE TickType_t ) uxLocalPort )
            {
                pxFound = ( ( FreeRTOS_Socket_t * ) listGET_LIST_ITEM_OWNER( pxIterator ) );

                if( ( pxFound->ucProtocol == ( uint8_t ) FREERTOS_IPPROTO_TCP ) && ( pxFound->u.xTCP.bits.bPassAccept != pdFALSE_UNSIGNED ) )
                {
                    pxSocket->u.xTCP.pxPeerSocket = pxFound;
                    FreeRTOS_debug_printf( ( "xTCPCheckNewClient[0]: client on port %u\n", pxSocket->usLocalPort ) );
                    xResult = pdTRUE;
                    break;
                }
            }
        }

        return xResult;
    }
    /*-----------------------------------------------------------*/


#endif /* ipconfigUSE_TCP == 1 */

/* Provide access to private members for testing. */
#ifdef FREERTOS_ENABLE_UNIT_TESTS
    #include "freertos_tcp_test_access_tcp_define.h"
#endif

/* Provide access to private members for verification. */
#ifdef FREERTOS_TCP_ENABLE_VERIFICATION
    #include "aws_freertos_tcp_verification_access_tcp_define.h"
#endif
