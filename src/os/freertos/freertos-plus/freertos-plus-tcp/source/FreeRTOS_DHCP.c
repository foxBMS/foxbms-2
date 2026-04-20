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
 * @file FreeRTOS_DHCP.c
 * @brief Implements the Dynamic Host Configuration Protocol for the FreeRTOS+TCP network stack.
 */

/* Standard includes. */
#include <stdint.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* FreeRTOS+TCP includes. */
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"
#include "FreeRTOS_IP_Private.h"
#include "FreeRTOS_UDP_IP.h"
#include "FreeRTOS_DHCP.h"
#include "FreeRTOS_ARP.h"
#include "FreeRTOS_IP_Timers.h"


/* Exclude the entire file if DHCP is not enabled. */
#if ( ipconfigUSE_DHCP != 0 )

    #include "NetworkInterface.h"
    #include "NetworkBufferManagement.h"

    #include "FreeRTOS_Routing.h"

    #define EP_DHCPData         pxEndPoint->xDHCPData               /**< Temporary define to make /single source similar to /multi version. */
    #define EP_IPv4_SETTINGS    pxEndPoint->ipv4_settings           /**< Temporary define to make /single source similar to /multi version. */



/** @brief The UDP socket used for all incoming and outgoing DHCP traffic. */
    _static Socket_t xDHCPv4Socket;

    #if ( ipconfigDHCP_FALL_BACK_AUTO_IP != 0 )
        /* Define the Link Layer IP address: 169.254.x.x */
        #define LINK_LAYER_ADDRESS_0    169
        #define LINK_LAYER_ADDRESS_1    254

/* Define the netmask used: 255.255.0.0 */
        #define LINK_LAYER_NETMASK_0    255
        #define LINK_LAYER_NETMASK_1    255
        #define LINK_LAYER_NETMASK_2    0
        #define LINK_LAYER_NETMASK_3    0
    #endif

/*-----------------------------------------------------------*/

/**
 * @brief The number of end-points that are making use of the UDP-socket.
 */
    _static BaseType_t xDHCPSocketUserCount = 0;

/*
 * Generate a DHCP discover message and send it on the DHCP socket.
 */
    static BaseType_t prvSendDHCPDiscover( NetworkEndPoint_t * pxEndPoint );

/*
 * Interpret message received on the DHCP socket.
 */
    static BaseType_t prvProcessDHCPReplies( BaseType_t xExpectedMessageType,
                                             NetworkEndPoint_t * pxEndPoint );

/*
 * Generate a DHCP request packet, and send it on the DHCP socket.
 */
    static BaseType_t prvSendDHCPRequest( NetworkEndPoint_t * pxEndPoint );

/*
 * Prepare to start a DHCP transaction.  This initialises some state variables
 * and creates the DHCP socket if necessary.
 */
    static void prvInitialiseDHCP( NetworkEndPoint_t * pxEndPoint );

/*
 * Creates the part of outgoing DHCP messages that are common to all outgoing
 * DHCP messages.
 */
    static uint8_t * prvCreatePartDHCPMessage( struct freertos_sockaddr * pxAddress,
                                               BaseType_t xOpcode,
                                               const uint8_t * const pucOptionsArray,
                                               size_t * pxOptionsArraySize,
                                               const NetworkEndPoint_t * pxEndPoint );

/*
 * Create the DHCP socket, if it has not been created already.
 */
    _static void prvCreateDHCPSocket( NetworkEndPoint_t * pxEndPoint );

/*
 * Close the DHCP socket, only when not in use anymore (i.e. xDHCPSocketUserCount = 0).
 */
    static void prvCloseDHCPSocket( NetworkEndPoint_t * pxEndPoint );

    static void vDHCPProcessEndPoint( BaseType_t xReset,
                                      BaseType_t xDoCheck,
                                      NetworkEndPoint_t * pxEndPoint );

    static BaseType_t xHandleWaitingOffer( NetworkEndPoint_t * pxEndPoint,
                                           BaseType_t xDoCheck );

    static void vHandleWaitingAcknowledge( NetworkEndPoint_t * pxEndPoint,
                                           BaseType_t xDoCheck );

    static BaseType_t xHandleWaitingFirstDiscover( NetworkEndPoint_t * pxEndPoint );

    static void prvHandleWaitingeLeasedAddress( NetworkEndPoint_t * pxEndPoint );

    static void vProcessHandleOption( NetworkEndPoint_t * pxEndPoint,
                                      ProcessSet_t * pxSet,
                                      BaseType_t xExpectedMessageType );
    static BaseType_t xProcessCheckOption( ProcessSet_t * pxSet );


/*-----------------------------------------------------------*/

/**
 * @brief Check whether a given socket is the DHCP socket or not.
 *
 * @param[in] xSocket The socket to be checked.
 *
 * @return If the socket given as parameter is the DHCP socket - return
 *         pdTRUE, else pdFALSE.
 */
    BaseType_t xIsDHCPSocket( const ConstSocket_t xSocket )
    {
        BaseType_t xReturn;

        if( xDHCPv4Socket == xSocket )
        {
            xReturn = pdTRUE;
        }
        else
        {
            xReturn = pdFALSE;
        }

        return xReturn;
    }
    /*-----------------------------------------------------------*/


/**
 * @brief Process the DHCP state machine based on current state.
 *
 * @param[in] xReset Is the DHCP state machine starting over? pdTRUE/pdFALSE.
 * @param[in] pxEndPoint The end-point for which the DHCP state machine should
 *                        make one cycle.
 */
    void vDHCPProcess( BaseType_t xReset,
                       struct xNetworkEndPoint * pxEndPoint )
    {
        BaseType_t xDoProcess = pdTRUE;

        /* The function is called by the IP-task, so pxEndPoint
         * should be non-NULL. */
        configASSERT( pxEndPoint != NULL );
        configASSERT( pxEndPoint->bits.bIPv6 == 0 );

        /* Is DHCP starting over? */
        if( xReset != pdFALSE )
        {
            EP_DHCPData.eDHCPState = eInitialWait;
        }

        if( ( EP_DHCPData.eDHCPState != EP_DHCPData.eExpectedState ) && ( xReset == pdFALSE ) )
        {
            /* When the DHCP event was generated, the DHCP client was
            * in a different state.  Therefore, ignore this event. */
            FreeRTOS_debug_printf( ( "DHCP wrong state: expect: %d got: %d : ignore\n",
                                     EP_DHCPData.eExpectedState, EP_DHCPData.eDHCPState ) );
        }
        else if( EP_DHCPData.xDHCPSocket != NULL ) /* If there is a socket, check for incoming messages first. */
        {
            /* No need to initialise 'pucUDPPayload', it just looks nicer. */
            uint8_t * pucUDPPayload = NULL;
            const DHCPMessage_IPv4_t * pxDHCPMessage;
            int32_t lBytes;

            while( EP_DHCPData.xDHCPSocket != NULL )
            {
                BaseType_t xRecvFlags = FREERTOS_ZERO_COPY + FREERTOS_MSG_PEEK;
                NetworkEndPoint_t * pxIterator = NULL;

                /* Peek the next UDP message. */
                lBytes = FreeRTOS_recvfrom( EP_DHCPData.xDHCPSocket, &( pucUDPPayload ), 0, xRecvFlags, NULL, NULL );

                if( lBytes < ( ( int32_t ) sizeof( DHCPMessage_IPv4_t ) ) )
                {
                    if( ( lBytes < 0 ) && ( lBytes != -pdFREERTOS_ERRNO_EAGAIN ) )
                    {
                        FreeRTOS_printf( ( "vDHCPProcess: FreeRTOS_recvfrom returns %d\n", ( int ) lBytes ) );
                    }

                    break;
                }

                /* Map a DHCP structure onto the received data. */
                /* MISRA Ref 11.3.1 [Misaligned access] */
                /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
                /* coverity[misra_c_2012_rule_11_3_violation] */
                pxDHCPMessage = ( ( const DHCPMessage_IPv4_t * ) pucUDPPayload );

                /* Sanity check. */
                if( ( pxDHCPMessage->ulDHCPCookie == dhcpCOOKIE ) && ( pxDHCPMessage->ucOpcode == dhcpREPLY_OPCODE ) )
                {
                    pxIterator = pxNetworkEndPoints;

                    /* Find the end-point with given transaction ID. */
                    while( pxIterator != NULL )
                    {
                        if( pxDHCPMessage->ulTransactionID == FreeRTOS_htonl( pxIterator->xDHCPData.ulTransactionId ) )
                        {
                            break;
                        }

                        pxIterator = pxIterator->pxNext;
                    }
                }

                if( ( pxIterator != NULL ) && ( pxIterator->xDHCPData.eDHCPState == eLeasedAddress ) )
                {
                    /* No DHCP messages are expected while in eLeasedAddress state. */
                    pxIterator = NULL;
                }

                if( pxIterator != NULL )
                {
                    /* The second parameter pdTRUE tells to check for a UDP message. */
                    vDHCPProcessEndPoint( pdFALSE, pdTRUE, pxIterator );

                    if( pxEndPoint == pxIterator )
                    {
                        xDoProcess = pdFALSE;
                    }
                }
                else
                {
                    /* Target not found, fetch the message and delete it. */
                    /* PAss the address of a pointer pucUDPPayload, because zero-copy is used. */
                    lBytes = FreeRTOS_recvfrom( EP_DHCPData.xDHCPSocket, &( pucUDPPayload ), 0, FREERTOS_ZERO_COPY, NULL, NULL );

                    if( ( lBytes > 0 ) && ( pucUDPPayload != NULL ) )
                    {
                        /* Remove it now, destination not found. */
                        FreeRTOS_ReleaseUDPPayloadBuffer( pucUDPPayload );
                        FreeRTOS_printf( ( "vDHCPProcess: Removed a %d-byte message: target not found\n", ( int ) lBytes ) );
                    }
                }
            }
        }
        else
        {
            /* do nothing, coverity happy */
        }

        if( xDoProcess != pdFALSE )
        {
            /* Process the end-point, but do not expect incoming packets. */
            vDHCPProcessEndPoint( xReset, pdFALSE, pxEndPoint );
        }
    }

/**
 * @brief Stop the DHCP process. Close the DHCP socket when it's no longer used by any end-point.
 *
 * @param[in] pxEndPoint The end-point for which we want to stop the DHCP process.
 */
    void vDHCPStop( struct xNetworkEndPoint * pxEndPoint )
    {
        /* Disable the DHCP timer. */
        vIPSetDHCP_RATimerEnableState( pxEndPoint, pdFALSE );

        /* Close socket to ensure packets don't queue on it. */
        prvCloseDHCPSocket( pxEndPoint );
    }

/**
 * @brief Called by vDHCPProcessEndPoint(), this function handles the state 'eWaitingOffer'.
 *        If there is a reply, it will be examined, if there is a time-out, there may be a new
 *        new attempt, or it will give up.
 * @param[in] pxEndPoint The end-point that is getting an IP-address from a DHCP server
 * @param[in] xDoCheck When true, the function must handle any replies.
 * @return It returns pdTRUE in case the DHCP process must be given up.
 */
    static BaseType_t xHandleWaitingOffer( NetworkEndPoint_t * pxEndPoint,
                                           BaseType_t xDoCheck )
    {
        BaseType_t xGivingUp = pdFALSE;

        #if ( ipconfigUSE_DHCP_HOOK != 0 )
            eDHCPCallbackAnswer_t eAnswer;
            #if ( ipconfigIPv4_BACKWARD_COMPATIBLE != 1 )
                IP_Address_t xIPAddress;
            #endif
        #endif

        /* Look for offers coming in. */
        if( xDoCheck != pdFALSE )
        {
            if( prvProcessDHCPReplies( dhcpMESSAGE_TYPE_OFFER, pxEndPoint ) == pdPASS )
            {
                #if ( ipconfigUSE_DHCP_HOOK != 0 )
                    /* Ask the user if a DHCP request is required. */
                    #if ( ipconfigIPv4_BACKWARD_COMPATIBLE == 1 )
                        eAnswer = xApplicationDHCPHook( eDHCPPhasePreRequest, EP_DHCPData.ulOfferedIPAddress );
                    #else /* ( ipconfigIPv4_BACKWARD_COMPATIBLE == 1 ) */
                        xIPAddress.ulIP_IPv4 = EP_DHCPData.ulOfferedIPAddress;
                        eAnswer = xApplicationDHCPHook_Multi( eDHCPPhasePreRequest, pxEndPoint, &xIPAddress );
                    #endif /* ( ipconfigIPv4_BACKWARD_COMPATIBLE == 1 ) */

                    if( eAnswer == eDHCPContinue )
                #endif /* ipconfigUSE_DHCP_HOOK */
                {
                    /* An offer has been made, the user wants to continue,
                     * generate the request. */
                    if( prvSendDHCPRequest( pxEndPoint ) == pdPASS )
                    {
                        EP_DHCPData.xDHCPTxTime = xTaskGetTickCount();
                        EP_DHCPData.xDHCPTxPeriod = dhcpINITIAL_DHCP_TX_PERIOD;
                        EP_DHCPData.eDHCPState = eWaitingAcknowledge;
                    }
                    else
                    {
                        /* Either the creation of a message buffer failed, or sendto().
                         * Try again in the next cycle. */
                        FreeRTOS_debug_printf( ( "Send failed during eWaitingOffer/1.\n" ) );
                        EP_DHCPData.eDHCPState = eSendDHCPRequest;
                    }
                }

                #if ( ipconfigUSE_DHCP_HOOK != 0 )
                    else
                    {
                        if( eAnswer == eDHCPUseDefaults )
                        {
                            ( void ) memcpy( &( pxEndPoint->ipv4_settings ), &( pxEndPoint->ipv4_defaults ), sizeof( pxEndPoint->ipv4_settings ) );
                        }

                        /* The user indicates that the DHCP process does not continue. */
                        xGivingUp = pdTRUE;
                    }
                #endif /* ipconfigUSE_DHCP_HOOK */
            }
        }

        /* Is it time to send another Discover? */
        else if( ( xTaskGetTickCount() - EP_DHCPData.xDHCPTxTime ) > EP_DHCPData.xDHCPTxPeriod )
        {
            /* It is time to send another Discover.  Increase the time
             * period, and if it has not got to the point of giving up - send
             * another discovery. */
            EP_DHCPData.xDHCPTxPeriod <<= 1;

            if( EP_DHCPData.xDHCPTxPeriod <= ( ( TickType_t ) ipconfigMAXIMUM_DISCOVER_TX_PERIOD ) )
            {
                if( xApplicationGetRandomNumber( &( EP_DHCPData.ulTransactionId ) ) != pdFALSE )
                {
                    EP_DHCPData.xDHCPTxTime = xTaskGetTickCount();

                    if( EP_DHCPData.xUseBroadcast != pdFALSE )
                    {
                        EP_DHCPData.xUseBroadcast = pdFALSE;
                    }
                    else
                    {
                        EP_DHCPData.xUseBroadcast = pdTRUE;
                    }

                    if( prvSendDHCPDiscover( pxEndPoint ) == pdPASS )
                    {
                        FreeRTOS_debug_printf( ( "vDHCPProcess: timeout %lu ticks\n", EP_DHCPData.xDHCPTxPeriod ) );
                    }
                    else
                    {
                        /* Either the creation of a message buffer failed, or sendto().
                         * Try again in the next cycle. */
                        FreeRTOS_debug_printf( ( "Send failed during eWaitingOffer/2.\n" ) );
                        EP_DHCPData.eDHCPState = eInitialWait;
                    }
                }
                else
                {
                    FreeRTOS_debug_printf( ( "vDHCPProcess: failed to generate a random Transaction ID\n" ) );
                }
            }
            else
            {
                FreeRTOS_debug_printf( ( "vDHCPProcess: giving up %lu > %lu ticks\n", EP_DHCPData.xDHCPTxPeriod, ipconfigMAXIMUM_DISCOVER_TX_PERIOD ) );

                #if ( ipconfigDHCP_FALL_BACK_AUTO_IP != 0 )
                {
                    /* Only use a fake Ack if the default IP address == 0x00
                     * and the link local addressing is used.  Start searching
                     * a free LinkLayer IP-address.  Next state will be
                     * 'eGetLinkLayerAddress'. */
                    prvPrepareLinkLayerIPLookUp( pxEndPoint );

                    /* Setting an IP address manually so set to not using
                     * leased address mode. */
                    EP_DHCPData.eDHCPState = eGetLinkLayerAddress;
                }
                #else
                {
                    xGivingUp = pdTRUE;
                }
                #endif /* ipconfigDHCP_FALL_BACK_AUTO_IP */
            }
        }
        else
        {
            /* There was no DHCP reply, there was no time-out, just keep on waiting. */
        }

        return xGivingUp;
    }
/*-----------------------------------------------------------*/

/**
 * @brief Called by vDHCPProcessEndPoint(), this function handles the state 'eWaitingAcknowledge'.
 *        If there is a reply, it will be examined, if there is a time-out, there may be a new
 *        new attempt, or it will give up.
 *        After the acknowledge, the leasing of an IP-address will start.
 * @param[in] pxEndPoint The end-point that is getting an IP-address from a DHCP server
 * @param[in] xDoCheck When true, the function must handle any replies.
 */
    static void vHandleWaitingAcknowledge( NetworkEndPoint_t * pxEndPoint,
                                           BaseType_t xDoCheck )
    {
        if( xDoCheck == pdFALSE )
        {
            /* Is it time to send another Discover? */
            if( ( xTaskGetTickCount() - EP_DHCPData.xDHCPTxTime ) > EP_DHCPData.xDHCPTxPeriod )
            {
                /* Increase the time period, and if it has not got to the
                 * point of giving up - send another request. */
                EP_DHCPData.xDHCPTxPeriod <<= 1;

                if( EP_DHCPData.xDHCPTxPeriod <= ( TickType_t ) ipconfigMAXIMUM_DISCOVER_TX_PERIOD )
                {
                    EP_DHCPData.xDHCPTxTime = xTaskGetTickCount();

                    if( prvSendDHCPRequest( pxEndPoint ) == pdPASS )
                    {
                        /* The message is sent. Stay in state 'eWaitingAcknowledge'. */
                    }
                    else
                    {
                        /* Either the creation of a message buffer failed, or sendto().
                         * Try again in the next cycle. */
                        FreeRTOS_debug_printf( ( "Send failed during eWaitingAcknowledge.\n" ) );
                        EP_DHCPData.eDHCPState = eSendDHCPRequest;
                    }
                }
                else
                {
                    /* Give up, start again. */
                    EP_DHCPData.eDHCPState = eInitialWait;
                }
            }
        }
        else if( prvProcessDHCPReplies( dhcpMESSAGE_TYPE_ACK, pxEndPoint ) == pdPASS )
        {
            FreeRTOS_debug_printf( ( "vDHCPProcess: acked %xip\n", ( unsigned int ) FreeRTOS_ntohl( EP_DHCPData.ulOfferedIPAddress ) ) );

            /* DHCP completed.  The IP address can now be used, and the
             * timer set to the lease timeout time. */
            EP_IPv4_SETTINGS.ulIPAddress = EP_DHCPData.ulOfferedIPAddress;

            /* Setting the 'local' broadcast address, something like
             * '192.168.1.255'. */
            EP_IPv4_SETTINGS.ulBroadcastAddress = EP_DHCPData.ulOfferedIPAddress | ~( EP_IPv4_SETTINGS.ulNetMask );
            EP_DHCPData.eDHCPState = eLeasedAddress;

            iptraceDHCP_SUCCEEDED( EP_DHCPData.ulOfferedIPAddress );

            /* DHCP failed, the default configured IP-address will be used
             * Now call vIPNetworkUpCalls() to send the network-up event and
             * start the ARP timer. */
            vIPNetworkUpCalls( pxEndPoint );
            /* Close socket to ensure packets don't queue on it. */
            prvCloseDHCPSocket( pxEndPoint );

            if( EP_DHCPData.ulLeaseTime == 0U )
            {
                EP_DHCPData.ulLeaseTime = dhcpDEFAULT_LEASE_TIME;
            }
            else if( EP_DHCPData.ulLeaseTime < dhcpMINIMUM_LEASE_TIME )
            {
                EP_DHCPData.ulLeaseTime = dhcpMINIMUM_LEASE_TIME;
            }
            else
            {
                /* The lease time is already valid. */
            }

            /* Check for clashes. */
            vARPSendGratuitous();
            vDHCP_RATimerReload( ( struct xNetworkEndPoint * ) pxEndPoint, EP_DHCPData.ulLeaseTime );
        }
        else
        {
            /* There are no replies yet. */
        }
    }

/**
 * @brief Called by vDHCPProcessEndPoint(), this function handles the state 'eWaitingSendFirstDiscover'.
 *        If will send a DISCOVER message to a DHCP server, and move to the next status 'eWaitingOffer'.
 * @param[in] pxEndPoint The end-point that is getting an IP-address from a DHCP server
 * @return xGivingUp: when pdTRUE, there was a fatal error and the process can not continue;
 */
    static BaseType_t xHandleWaitingFirstDiscover( NetworkEndPoint_t * pxEndPoint )
    {
        BaseType_t xGivingUp = pdFALSE;

        /* Ask the user if a DHCP discovery is required. */
        #if ( ipconfigUSE_DHCP_HOOK != 0 )
            #if ( ipconfigIPv4_BACKWARD_COMPATIBLE == 1 )
                eDHCPCallbackAnswer_t eAnswer = xApplicationDHCPHook( eDHCPPhasePreDiscover, pxEndPoint->ipv4_defaults.ulIPAddress );
            #else /* ( ipconfigIPv4_BACKWARD_COMPATIBLE == 1 ) */
                IP_Address_t xIPAddress;
                eDHCPCallbackAnswer_t eAnswer;

                xIPAddress.ulIP_IPv4 = pxEndPoint->ipv4_defaults.ulIPAddress;
                eAnswer = xApplicationDHCPHook_Multi( eDHCPPhasePreDiscover, pxEndPoint, &xIPAddress );
            #endif /* ( ipconfigIPv4_BACKWARD_COMPATIBLE == 1 ) */

            if( eAnswer == eDHCPContinue )
        #endif /* ipconfigUSE_DHCP_HOOK */
        {
            /* See if prvInitialiseDHCP() has creates a socket. */
            if( EP_DHCPData.xDHCPSocket == NULL )
            {
                xGivingUp = pdTRUE;
            }
            else
            {
                /* Put 'ulIPAddress' to zero to indicate that the end-point is down. */
                EP_IPv4_SETTINGS.ulIPAddress = 0U;

                /* Send the first discover request. */
                EP_DHCPData.xDHCPTxTime = xTaskGetTickCount();

                if( prvSendDHCPDiscover( pxEndPoint ) == pdPASS )
                {
                    EP_DHCPData.eDHCPState = eWaitingOffer;
                }
                else
                {
                    /* Either the creation of a message buffer failed, or sendto().
                     * Try again in the next cycle. */
                    FreeRTOS_debug_printf( ( "Send failed during eWaitingSendFirstDiscover\n" ) );
                }
            }
        }

        #if ( ipconfigUSE_DHCP_HOOK != 0 )
            else
            {
                if( eAnswer == eDHCPUseDefaults )
                {
                    ( void ) memcpy( &( pxEndPoint->ipv4_settings ), &( pxEndPoint->ipv4_defaults ), sizeof( pxEndPoint->ipv4_settings ) );
                }

                /* The user indicates that the DHCP process does not continue. */
                xGivingUp = pdTRUE;
            }
        #endif /* ipconfigUSE_DHCP_HOOK */

        return xGivingUp;
    }
/*-----------------------------------------------------------*/

/**
 * @brief Called by vDHCPProcessEndPoint(), this function handles the state 'eLeasedAddress'.
 *        If waits until the lease must be renewed, and then send a new request.
 * @param[in] pxEndPoint The end-point that is getting an IP-address from a DHCP server
 */
    static void prvHandleWaitingeLeasedAddress( NetworkEndPoint_t * pxEndPoint )
    {
        if( FreeRTOS_IsEndPointUp( pxEndPoint ) != 0 )
        {
            /* Resend the request at the appropriate time to renew the lease. */
            prvCreateDHCPSocket( pxEndPoint );

            if( EP_DHCPData.xDHCPSocket != NULL )
            {
                uint32_t ulID = 0U;

                if( xApplicationGetRandomNumber( &( ulID ) ) != pdFALSE )
                {
                    EP_DHCPData.ulTransactionId = ulID;
                }

                EP_DHCPData.xDHCPTxTime = xTaskGetTickCount();
                EP_DHCPData.xDHCPTxPeriod = dhcpINITIAL_DHCP_TX_PERIOD;

                if( prvSendDHCPRequest( pxEndPoint ) == pdPASS )
                {
                    /* The packet was sent successfully, wait for an acknowledgement. */
                    EP_DHCPData.eDHCPState = eWaitingAcknowledge;
                }
                else
                {
                    /* The packet was not sent, try sending it later. */
                    EP_DHCPData.eDHCPState = eSendDHCPRequest;
                    FreeRTOS_debug_printf( ( "Send failed eLeasedAddress.\n" ) );
                }

                /* From now on, we should be called more often */
                vDHCP_RATimerReload( pxEndPoint, dhcpINITIAL_TIMER_PERIOD );
            }
        }
        else
        {
            /* See PR #53 on github/freertos/freertos */
            FreeRTOS_printf( ( "DHCP: lease time finished but network is down\n" ) );
            vDHCP_RATimerReload( ( struct xNetworkEndPoint * ) pxEndPoint, pdMS_TO_TICKS( 5000U ) );
        }
    }
/*-----------------------------------------------------------*/

/**
 * @brief Process the DHCP state machine based on current state.
 *
 * @param[in] xReset Is the DHCP state machine starting over? pdTRUE/pdFALSE.
 * @param[in] xDoCheck true when an incoming message is to be expected, and
 *                      prvProcessDHCPReplies() will be called.
 * @param[in] pxEndPoint The end-point for which the DHCP state machine should
 *                        make one cycle.
 */
    static void vDHCPProcessEndPoint( BaseType_t xReset,
                                      BaseType_t xDoCheck,
                                      NetworkEndPoint_t * pxEndPoint )
    {
        BaseType_t xGivingUp = pdFALSE;

        configASSERT( pxEndPoint != NULL );

        /* Is DHCP starting over? */
        if( xReset != pdFALSE )
        {
            EP_DHCPData.eDHCPState = eInitialWait;
        }

        if( ( EP_DHCPData.eDHCPState != EP_DHCPData.eExpectedState ) && ( xReset == pdFALSE ) )
        {
            /* When the DHCP event was generated, the DHCP client was
            * in a different state.  Therefore, ignore this event. */
            FreeRTOS_debug_printf( ( "vDHCPProcessEndPoint: wrong state: expect: %d got: %d : ignore\n",
                                     EP_DHCPData.eExpectedState, EP_DHCPData.eDHCPState ) );
        }
        else
        {
            {
                static eDHCPState_t eLastState = eNotUsingLeasedAddress;

                if( eLastState != EP_DHCPData.eDHCPState )
                {
                    eLastState = EP_DHCPData.eDHCPState;
                    FreeRTOS_debug_printf( ( "vDHCPProcessEndPoint: enter %d\n", EP_DHCPData.eDHCPState ) );
                }
            }

            switch( EP_DHCPData.eDHCPState )
            {
                case eInitialWait:

                    /* Initial state.  Create the DHCP socket, timer, etc. if they
                     * have not already been created. */

                    /* Initial state.  Create the DHCP socket, timer, etc. if they
                     * have not already been created. */
                    prvInitialiseDHCP( pxEndPoint );
                    EP_DHCPData.eDHCPState = eWaitingSendFirstDiscover;
                    break;

                case eWaitingSendFirstDiscover:
                    xGivingUp = xHandleWaitingFirstDiscover( pxEndPoint );
                    break;

                case eSendDHCPRequest:

                    if( prvSendDHCPRequest( pxEndPoint ) == pdPASS )
                    {
                        /* Send succeeded, go to state 'eWaitingAcknowledge'. */
                        EP_DHCPData.xDHCPTxTime = xTaskGetTickCount();
                        EP_DHCPData.xDHCPTxPeriod = dhcpINITIAL_DHCP_TX_PERIOD;
                        EP_DHCPData.eDHCPState = eWaitingAcknowledge;
                    }
                    else
                    {
                        /* Either the creation of a message buffer failed, or sendto().
                         * Try again in the next cycle. */
                        FreeRTOS_debug_printf( ( "Send failed during eSendDHCPRequest.\n" ) );
                    }

                    break;

                case eWaitingOffer:
                    xGivingUp = xHandleWaitingOffer( pxEndPoint, xDoCheck );
                    break;

                case eWaitingAcknowledge:
                    vHandleWaitingAcknowledge( pxEndPoint, xDoCheck );
                    break;

                    #if ( ipconfigDHCP_FALL_BACK_AUTO_IP != 0 )
                        case eGetLinkLayerAddress:

                            if( ( xTaskGetTickCount() - EP_DHCPData.xDHCPTxTime ) > EP_DHCPData.xDHCPTxPeriod )
                            {
                                if( xARPHadIPClash == pdFALSE )
                                {
                                    /* ARP OK. proceed. */
                                    iptraceDHCP_SUCCEEDED( EP_DHCPData.ulOfferedIPAddress );

                                    /* Auto-IP succeeded, the default configured IP-address will
                                     * be used.  Now call vIPNetworkUpCalls() to send the
                                     * network-up event and start the ARP timer. */
                                    vIPNetworkUpCalls( pxEndPoint );

                                    EP_DHCPData.eDHCPState = eNotUsingLeasedAddress;
                                }
                                else
                                {
                                    /* ARP clashed - try another IP address. */
                                    prvPrepareLinkLayerIPLookUp( pxEndPoint );

                                    /* Setting an IP address manually so set to not using leased
                                     * address mode. */
                                    EP_DHCPData.eDHCPState = eGetLinkLayerAddress;
                                }
                            }
                            break;
                    #endif /* ipconfigDHCP_FALL_BACK_AUTO_IP */

                case eLeasedAddress:
                    prvHandleWaitingeLeasedAddress( pxEndPoint );
                    break;

                case eNotUsingLeasedAddress:

                    vIPSetDHCP_RATimerEnableState( pxEndPoint, pdFALSE );
                    break;

                default:
                    /* Lint: all options are included. */
                    break;
            }

            {
                static eDHCPState_t eLastState = eNotUsingLeasedAddress;

                if( eLastState != EP_DHCPData.eDHCPState )
                {
                    eLastState = EP_DHCPData.eDHCPState;
                    FreeRTOS_debug_printf( ( "vDHCPProcessEndPoint: exit %d\n", EP_DHCPData.eDHCPState ) );
                }
            }

            if( xGivingUp != pdFALSE )
            {
                /* xGivingUp became true either because of a time-out, or because
                 * xApplicationDHCPHook() returned another value than 'eDHCPContinue',
                 * meaning that the conversion is cancelled from here. */

                /* Revert to static IP address. */
                taskENTER_CRITICAL();
                {
                    EP_IPv4_SETTINGS.ulIPAddress = pxEndPoint->ipv4_defaults.ulIPAddress;
                    iptraceDHCP_REQUESTS_FAILED_USING_DEFAULT_IP_ADDRESS( pxEndPoint->ipv4_defaults.ulIPAddress );
                }
                taskEXIT_CRITICAL();

                EP_DHCPData.eDHCPState = eNotUsingLeasedAddress;
                vIPSetDHCP_RATimerEnableState( pxEndPoint, pdFALSE );

                /* DHCP failed, the default configured IP-address will be used. Now
                 * call vIPNetworkUpCalls() to send the network-up event and start the ARP
                 * timer. */
                vIPNetworkUpCalls( pxEndPoint );

                /* Close socket to ensure packets don't queue on it. */
                prvCloseDHCPSocket( pxEndPoint );
            }
        }
    }
/*-----------------------------------------------------------*/

/**
 * @brief Close the DHCP socket, but only when there are no other end-points
 *        using it.
 * @param[in] pxEndPoint The end-point that stops using the socket.
 */
    static void prvCloseDHCPSocket( NetworkEndPoint_t * pxEndPoint )
    {
        if( ( EP_DHCPData.xDHCPSocket == NULL ) || ( EP_DHCPData.xDHCPSocket != xDHCPv4Socket ) )
        {
            /* the socket can not be closed. */
        }
        else if( xDHCPSocketUserCount > 0 )
        {
            xDHCPSocketUserCount--;

            if( xDHCPSocketUserCount == 0 )
            {
                /* This modules runs from the IP-task. Use the internal
                 * function 'vSocketClose()` to close the socket. */
                ( void ) vSocketClose( xDHCPv4Socket );
                xDHCPv4Socket = NULL;
            }

            EP_DHCPData.xDHCPSocket = NULL;
        }
        else
        {
            /* Strange: there is a socket, but there are no users. */
        }

        FreeRTOS_printf( ( "prvCloseDHCPSocket[%02x-%02x]: %s, user count %d\n",
                           pxEndPoint->xMACAddress.ucBytes[ 4 ],
                           pxEndPoint->xMACAddress.ucBytes[ 5 ],
                           ( xDHCPv4Socket != NULL ) ? "open" : "closed",
                           ( int ) xDHCPSocketUserCount ) );
    }
    /*-----------------------------------------------------------*/

/**
 * @brief Create a DHCP socket with the defined timeouts. The same socket
 *        will be shared among all end-points that need DHCP.
 */
    _static void prvCreateDHCPSocket( NetworkEndPoint_t * pxEndPoint )
    {
        struct freertos_sockaddr xAddress;
        BaseType_t xReturn;
        TickType_t xTimeoutTime = ( TickType_t ) 0;

        if( ( xDHCPv4Socket != NULL ) && ( EP_DHCPData.xDHCPSocket == xDHCPv4Socket ) )
        {
            /* the socket is still valid. */
        }
        else if( xDHCPv4Socket == NULL ) /* Create the socket, if it has not already been created. */
        {
            xDHCPv4Socket = FreeRTOS_socket( FREERTOS_AF_INET, FREERTOS_SOCK_DGRAM, FREERTOS_IPPROTO_UDP );
            configASSERT( xSocketValid( xDHCPv4Socket ) == pdTRUE );

            /* MISRA Ref 11.4.1 [Socket error and integer to pointer conversion] */
            /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-114 */
            /* coverity[misra_c_2012_rule_11_4_violation] */
            if( xSocketValid( xDHCPv4Socket ) == pdTRUE )
            {
                /* Ensure the Rx and Tx timeouts are zero as the DHCP executes in the
                 * context of the IP task. */
                ( void ) FreeRTOS_setsockopt( xDHCPv4Socket, 0, FREERTOS_SO_RCVTIMEO, &( xTimeoutTime ), sizeof( TickType_t ) );
                ( void ) FreeRTOS_setsockopt( xDHCPv4Socket, 0, FREERTOS_SO_SNDTIMEO, &( xTimeoutTime ), sizeof( TickType_t ) );

                ( void ) memset( &xAddress, 0, sizeof( xAddress ) );
                xAddress.sin_family = FREERTOS_AF_INET4;
                xAddress.sin_len = ( uint8_t ) sizeof( xAddress );
                /* Bind to the standard DHCP client port. */
                xAddress.sin_port = ( uint16_t ) dhcpCLIENT_PORT_IPv4;
                xReturn = vSocketBind( xDHCPv4Socket, &xAddress, sizeof( xAddress ), pdFALSE );
                xDHCPSocketUserCount = 1;
                EP_DHCPData.xDHCPSocket = xDHCPv4Socket;
                FreeRTOS_printf( ( "DHCP-socket[%02x-%02x]: DHCP Socket Create\n",
                                   pxEndPoint->xMACAddress.ucBytes[ 4 ],
                                   pxEndPoint->xMACAddress.ucBytes[ 5 ] ) );

                if( xReturn != 0 )
                {
                    /* Binding failed, close the socket again. */
                    prvCloseDHCPSocket( pxEndPoint );
                }
            }
            else
            {
                /* Change to NULL for easier testing. */
                xDHCPv4Socket = NULL;
                EP_DHCPData.xDHCPSocket = NULL;
            }
        }
        else
        {
            xDHCPSocketUserCount++;
            EP_DHCPData.xDHCPSocket = xDHCPv4Socket;
        }

        FreeRTOS_printf( ( "prvCreateDHCPSocket[%02x-%02x]: %s, user count %d\n",
                           pxEndPoint->xMACAddress.ucBytes[ 4 ],
                           pxEndPoint->xMACAddress.ucBytes[ 5 ],
                           ( xDHCPv4Socket != NULL ) ? "open" : "closed",
                           ( int ) xDHCPSocketUserCount ) );
    }
    /*-----------------------------------------------------------*/

/**
 * @brief Initialise the DHCP state machine by creating DHCP socket and
 *        begin the transaction.
 *
 * @param[in] pxEndPoint The end-point that needs DHCP.
 */
    static void prvInitialiseDHCP( NetworkEndPoint_t * pxEndPoint )
    {
        /* Initialise the parameters that will be set by the DHCP process. Per
         * https://www.ietf.org/rfc/rfc2131.txt Transaction ID should be a random
         * value chosen by the client. */

        /* Check for random number generator API failure. */
        if( xApplicationGetRandomNumber( &( EP_DHCPData.ulTransactionId ) ) != pdFALSE )
        {
            EP_DHCPData.xUseBroadcast = 0;
            EP_DHCPData.ulOfferedIPAddress = 0U;
            EP_DHCPData.ulDHCPServerAddress = 0U;
            EP_DHCPData.xDHCPTxPeriod = dhcpINITIAL_DHCP_TX_PERIOD;

            /* Create the DHCP socket if it has not already been created. */
            prvCreateDHCPSocket( pxEndPoint );
            FreeRTOS_debug_printf( ( "prvInitialiseDHCP: start after %lu ticks\n", dhcpINITIAL_TIMER_PERIOD ) );
            vDHCP_RATimerReload( pxEndPoint, dhcpINITIAL_TIMER_PERIOD );
        }
        else
        {
            FreeRTOS_debug_printf( ( "prvInitialiseDHCP: failed to generate a random Transaction ID\n" ) );
        }
    }
/*-----------------------------------------------------------*/

/**
 * @brief Called by prvProcessDHCPReplies(), which walks through an array of DHCP options,
 *        this function will check a single option.
 * @param[in] pxEndPoint The end-point that needs an IP-address.
 * @param[in] pxSet A set of variables that describe the parsing process.
 * @param[in] xExpectedMessageType The type of message expected in the
 *                                  dhcpIPv4_MESSAGE_TYPE_OPTION_CODE option.
 */
    static void vProcessHandleOption( NetworkEndPoint_t * pxEndPoint,
                                      ProcessSet_t * pxSet,
                                      BaseType_t xExpectedMessageType )
    {
        /* Option-specific handling. */

        switch( pxSet->ucOptionCode )
        {
            case dhcpIPv4_MESSAGE_TYPE_OPTION_CODE:

                if( pxSet->pucByte[ pxSet->uxIndex ] == ( uint8_t ) xExpectedMessageType )
                {
                    /* The message type is the message type the
                     * state machine is expecting. */
                    pxSet->ulProcessed++;
                }
                else
                {
                    if( pxSet->pucByte[ pxSet->uxIndex ] == ( uint8_t ) dhcpMESSAGE_TYPE_NACK )
                    {
                        if( xExpectedMessageType == ( BaseType_t ) dhcpMESSAGE_TYPE_ACK )
                        {
                            /* Start again. */
                            EP_DHCPData.eDHCPState = eInitialWait;
                        }
                    }

                    /* Stop processing further options. */
                    pxSet->uxLength = 0;
                }

                break;

            case dhcpIPv4_SUBNET_MASK_OPTION_CODE:

                if( pxSet->uxLength == sizeof( uint32_t ) )
                {
                    EP_IPv4_SETTINGS.ulNetMask = pxSet->ulParameter;
                }

                break;

            case dhcpIPv4_GATEWAY_OPTION_CODE:

                /* The DHCP server may send more than 1 gateway addresses. */
                if( pxSet->uxLength >= sizeof( uint32_t ) )
                {
                    /* ulProcessed is not incremented in this case
                     * because the gateway is not essential. */
                    EP_IPv4_SETTINGS.ulGatewayAddress = pxSet->ulParameter;
                }

                break;

            case dhcpIPv4_DNS_SERVER_OPTIONS_CODE:

                /* ulProcessed is not incremented in this case
                 * because the DNS server is not essential.  Only the
                 * first DNS server address is taken. */
                if( pxSet->uxLength >= sizeof( uint32_t ) )
                {
                    size_t uxSourceIndex;
                    size_t uxTargetIndex = 0;
                    size_t uxDNSCount = pxSet->uxLength / sizeof( uint32_t );
                    size_t uxByteIndex = pxSet->uxIndex;

                    void * pvCopyDest = &( pxSet->ulParameter );

                    /* Just to try-out for CBMC. */
                    if( uxDNSCount > ipconfigENDPOINT_DNS_ADDRESS_COUNT )
                    {
                        uxDNSCount = ipconfigENDPOINT_DNS_ADDRESS_COUNT;
                    }

                    for( uxSourceIndex = 0U; uxSourceIndex < uxDNSCount; uxSourceIndex++ )
                    {
                        const void * pvCopySource = &( pxSet->pucByte[ uxByteIndex ] );
                        ( void ) memcpy( pvCopyDest, pvCopySource, sizeof( pxSet->ulParameter ) );

                        if( ( pxSet->ulParameter != FREERTOS_INADDR_ANY ) && ( pxSet->ulParameter != ipBROADCAST_IP_ADDRESS ) )
                        {
                            EP_IPv4_SETTINGS.ulDNSServerAddresses[ uxTargetIndex ] = pxSet->ulParameter;
                            uxTargetIndex++;
                            /* uxDNSCount <= ipconfigENDPOINT_DNS_ADDRESS_COUNT , hence check is removed. */
                        }

                        uxByteIndex += sizeof( uint32_t );
                    }

                    /* Clear the remaining entries. */
                    while( uxTargetIndex < ipconfigENDPOINT_DNS_ADDRESS_COUNT )
                    {
                        EP_IPv4_SETTINGS.ulDNSServerAddresses[ uxTargetIndex ] = 0U;
                        uxTargetIndex++;
                    }

                    /* For the next lookup, start using the first DNS entry. */
                    EP_IPv4_SETTINGS.ucDNSIndex = 0U;
                }

                break;

            case dhcpIPv4_SERVER_IP_ADDRESS_OPTION_CODE:

                if( pxSet->uxLength == sizeof( uint32_t ) )
                {
                    if( xExpectedMessageType == ( BaseType_t ) dhcpMESSAGE_TYPE_OFFER )
                    {
                        /* Offers state the replying server. */
                        pxSet->ulProcessed++;
                        EP_DHCPData.ulDHCPServerAddress = pxSet->ulParameter;
                    }
                    else
                    {
                        /* The ack must come from the expected server. */
                        if( EP_DHCPData.ulDHCPServerAddress == pxSet->ulParameter )
                        {
                            pxSet->ulProcessed++;
                        }
                    }
                }

                break;

            case dhcpIPv4_LEASE_TIME_OPTION_CODE:

                if( pxSet->uxLength == sizeof( EP_DHCPData.ulLeaseTime ) )
                {
                    /* ulProcessed is not incremented in this case
                     * because the lease time is not essential. */

                    /* The DHCP parameter is in seconds, convert
                     * to host-endian format. */
                    EP_DHCPData.ulLeaseTime = FreeRTOS_ntohl( pxSet->ulParameter );

                    /* Divide the lease time by two to ensure a renew
                     * request is sent before the lease actually expires. */
                    EP_DHCPData.ulLeaseTime >>= 1U;

                    /* Multiply with configTICK_RATE_HZ to get clock ticks. */
                    EP_DHCPData.ulLeaseTime = ( uint32_t ) configTICK_RATE_HZ * ( uint32_t ) EP_DHCPData.ulLeaseTime;
                }

                break;

            default:

                /* Not interested in this field. */

                break;
        }
    }
/*-----------------------------------------------------------*/

/**
 * @brief Check whether the DHCP response from the server has all valid
 *        invariant parameters and valid (non broadcast and non localhost)
 *        IP address being assigned to the device.
 *
 * @param[in] pxDHCPMessage  The DHCP message.
 *
 * @return pdPASS if the DHCP response has correct parameters; pdFAIL otherwise.
 */
    static BaseType_t prvIsValidDHCPResponse( const DHCPMessage_IPv4_t * pxDHCPMessage )
    {
        BaseType_t xReturn = pdPASS;

        if( ( pxDHCPMessage->ulDHCPCookie != ( uint32_t ) dhcpCOOKIE ) ||
            ( pxDHCPMessage->ucOpcode != ( uint8_t ) dhcpREPLY_OPCODE ) ||
            ( pxDHCPMessage->ucAddressType != ( uint8_t ) dhcpADDRESS_TYPE_ETHERNET ) ||
            ( pxDHCPMessage->ucAddressLength != ( uint8_t ) dhcpETHERNET_ADDRESS_LENGTH ) ||
            ( ( FreeRTOS_ntohl( pxDHCPMessage->ulYourIPAddress_yiaddr ) & 0xFFU ) == 0xFFU ) ||
            ( ( ( pxDHCPMessage->ulYourIPAddress_yiaddr & 0x7FU ) ^ 0x7FU ) == 0x00U ) )
        {
            /* Invalid cookie OR
             * Unexpected opcode OR
             * Incorrect address type OR
             * Incorrect address length OR
             * The DHCP server is trying to assign a broadcast address to the device OR
             * The DHCP server is trying to assign a localhost address to the device. */
            xReturn = pdFAIL;
        }

        return xReturn;
    }
/*-----------------------------------------------------------*/

/**
 * @brief Check an incoming DHCP option.
 *
 * @param[in] pxSet A set of variables needed to parse the DHCP reply.
 *
 * @return pdPASS: 1 when the option must be analysed, 0 when the option
 *                 must be skipped, and -1 when parsing must stop.
 */
    static BaseType_t xProcessCheckOption( ProcessSet_t * pxSet )
    {
        BaseType_t xResult = -1;

        do
        {
            if( pxSet->ucOptionCode == ( uint8_t ) dhcpOPTION_END_BYTE )
            {
                /* Ready, the last byte has been seen.
                 * Return -1 so that the parsing will stop. */
                break;
            }

            if( pxSet->ucOptionCode == ( uint8_t ) dhcpIPv4_ZERO_PAD_OPTION_CODE )
            {
                /* The value zero is used as a pad byte,
                 * it is not followed by a length byte. */
                pxSet->uxIndex++;
                /* Return zero to skip this option. */
                xResult = 0;
                break;
            }

            /* Stop if the response is malformed. */
            if( ( pxSet->uxIndex + 1U ) >= pxSet->uxPayloadDataLength )
            {
                /* The length byte is missing, stop parsing. */
                break;
            }

            /* Fetch the length byte. */
            pxSet->uxLength = ( size_t ) pxSet->pucByte[ pxSet->uxIndex + 1U ];
            pxSet->uxIndex = pxSet->uxIndex + 2U;

            if( !( ( ( pxSet->uxIndex + pxSet->uxLength ) - 1U ) < pxSet->uxPayloadDataLength ) )
            {
                /* There are not as many bytes left as there should be. */
                break;
            }

            /* In most cases, a 4-byte network-endian parameter follows,
             * just get it once here and use later. */
            if( pxSet->uxLength >= sizeof( pxSet->ulParameter ) )
            {
                /*
                 * Use helper variables for memcpy() to remain
                 * compliant with MISRA Rule 21.15.  These should be
                 * optimized away.
                 */
                const void * pvCopySource = &( pxSet->pucByte[ pxSet->uxIndex ] );
                void * pvCopyDest = &( pxSet->ulParameter );
                ( void ) memcpy( pvCopyDest, pvCopySource, sizeof( pxSet->ulParameter ) );
                /* 'uxIndex' will be increased at the end of this loop. */
            }
            else
            {
                pxSet->ulParameter = 0;
            }

            /* Confirm uxIndex is still a valid index after adjustments to uxIndex above */
            if( !( pxSet->uxIndex < pxSet->uxPayloadDataLength ) )
            {
                break;
            }

            /* Return 1 so that the option will be processed. */
            xResult = 1;
            /* Try to please CBMC with a break statement here. */
            break;
        } while( ipFALSE_BOOL );

        return xResult;
    }
/*-----------------------------------------------------------*/

/**
 * @brief Process the DHCP replies.
 *
 * @param[in] xExpectedMessageType The type of the message the DHCP state machine is expecting.
 *                                  Messages of different type will be dropped.
 * @param[in] pxEndPoint The end-point to whom the replies are addressed.
 *
 * @return pdPASS: if DHCP options are received correctly; pdFAIL: Otherwise.
 */
    static BaseType_t prvProcessDHCPReplies( BaseType_t xExpectedMessageType,
                                             NetworkEndPoint_t * pxEndPoint )
    {
        uint8_t * pucUDPPayload;
        int32_t lBytes;
        const DHCPMessage_IPv4_t * pxDHCPMessage;
        BaseType_t xReturn = pdFALSE;
        const uint32_t ulMandatoryOptions = 2U; /* DHCP server address, and the correct DHCP message type must be present in the options. */
        ProcessSet_t xSet;

        ( void ) memset( &( xSet ), 0, sizeof( xSet ) );

        /* Passing the address of a pointer (pucUDPPayload) because FREERTOS_ZERO_COPY is used. */
        lBytes = FreeRTOS_recvfrom( EP_DHCPData.xDHCPSocket, &pucUDPPayload, 0U, FREERTOS_ZERO_COPY, NULL, NULL );

        if( lBytes > 0 )
        {
            /* Map a DHCP structure onto the received data. */

            /* MISRA Ref 11.3.1 [Misaligned access] */
            /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
            /* coverity[misra_c_2012_rule_11_3_violation] */
            pxDHCPMessage = ( ( DHCPMessage_IPv4_t * ) pucUDPPayload );

            /* Sanity check. */
            if( lBytes < ( int32_t ) sizeof( DHCPMessage_IPv4_t ) )
            {
                /* Not enough bytes. */
            }
            else if( prvIsValidDHCPResponse( pxDHCPMessage ) == pdFAIL )
            {
                /* Invalid values in DHCP response. */
            }
            else if( ( pxDHCPMessage->ulTransactionID != FreeRTOS_htonl( EP_DHCPData.ulTransactionId ) ) )
            {
                /* Transaction ID does not match. */
            }
            else /* Looks like a valid DHCP response, with the same transaction ID. */
            {
                if( memcmp( pxDHCPMessage->ucClientHardwareAddress,
                            pxEndPoint->xMACAddress.ucBytes,
                            sizeof( MACAddress_t ) ) != 0 )
                {
                    /* Target MAC address doesn't match. */
                }
                else
                {
                    /* None of the essential options have been processed yet. */
                    xSet.ulProcessed = 0U;

                    /* Walk through the options until the dhcpOPTION_END_BYTE byte
                     * is found, taking care not to walk off the end of the options. */
                    xSet.pucByte = &( pucUDPPayload[ sizeof( DHCPMessage_IPv4_t ) ] );
                    xSet.uxIndex = 0;
                    xSet.uxPayloadDataLength = ( ( size_t ) lBytes ) - sizeof( DHCPMessage_IPv4_t );

                    while( xSet.uxIndex < xSet.uxPayloadDataLength )
                    {
                        BaseType_t xResult;
                        xSet.ucOptionCode = xSet.pucByte[ xSet.uxIndex ];

                        xResult = xProcessCheckOption( &( xSet ) );

                        if( xResult > 0 )
                        {
                            vProcessHandleOption( pxEndPoint, &( xSet ), xExpectedMessageType );
                        }

                        if( xResult != 0 )
                        {
                            if( ( xSet.uxLength == 0U ) || ( xResult < 0 ) )
                            {
                                break;
                            }

                            xSet.uxIndex += xSet.uxLength;
                        }
                    }

                    /* Were all the mandatory options received? */
                    if( xSet.ulProcessed >= ulMandatoryOptions )
                    {
                        /* HT:endian: used to be network order */
                        EP_DHCPData.ulOfferedIPAddress = pxDHCPMessage->ulYourIPAddress_yiaddr;
                        FreeRTOS_printf( ( "vDHCPProcess: offer %xip for MAC address %02x-%02x\n",
                                           ( unsigned ) FreeRTOS_ntohl( EP_DHCPData.ulOfferedIPAddress ),
                                           pxEndPoint->xMACAddress.ucBytes[ 4 ],
                                           pxEndPoint->xMACAddress.ucBytes[ 5 ] ) );
                        xReturn = pdPASS;
                    }
                }
            }

            if( pucUDPPayload != NULL )
            {
                FreeRTOS_ReleaseUDPPayloadBuffer( pucUDPPayload );
            }
        } /* if( lBytes > 0 ) */

        return xReturn;
    }
    /*-----------------------------------------------------------*/

/**
 * @brief Create a partial DHCP message by filling in all the 'constant' fields.
 *
 * @param[out] pxAddress Address to be filled in.
 * @param[out] xOpcode Opcode to be filled in the packet. Will always be 'dhcpREQUEST_OPCODE'.
 * @param[in] pucOptionsArray The options to be added to the packet.
 * @param[in,out] pxOptionsArraySize Byte count of the options. Its value might change.
 * @param[in] pxEndPoint The end-point for which the request will be sent.
 *
 * @return Ethernet buffer of the partially created DHCP packet.
 */
    static uint8_t * prvCreatePartDHCPMessage( struct freertos_sockaddr * pxAddress,
                                               BaseType_t xOpcode,
                                               const uint8_t * const pucOptionsArray,
                                               size_t * pxOptionsArraySize,
                                               const NetworkEndPoint_t * pxEndPoint )
    {
        DHCPMessage_IPv4_t * pxDHCPMessage;
        size_t uxRequiredBufferSize = sizeof( DHCPMessage_IPv4_t ) + *pxOptionsArraySize;
        const NetworkBufferDescriptor_t * pxNetworkBuffer;
        uint8_t * pucUDPPayloadBuffer = NULL;

        #if ( ipconfigDHCP_REGISTER_HOSTNAME == 1 )
            size_t uxNameLength = 0;
            const char * pucHostName = pcApplicationHostnameHook();

            if( pucHostName != NULL )
            {
                uxNameLength = strlen( pucHostName );
            }

            /* Two extra bytes for option code and length. */
            uxRequiredBufferSize += ( 2U + uxNameLength );
        #endif /* if ( ipconfigDHCP_REGISTER_HOSTNAME == 1 ) */

        /* Obtain a network buffer with the required amount of storage.  It doesn't make much sense
         * to use a time-out here, because that would cause the IP-task to wait for itself. */
        pxNetworkBuffer = pxGetNetworkBufferWithDescriptor( sizeof( UDPPacket_t ) + uxRequiredBufferSize, 0U );

        if( pxNetworkBuffer != NULL )
        {
            uint8_t * pucIPType;

            /* Leave space for the UDP header. */
            pucUDPPayloadBuffer = &( pxNetworkBuffer->pucEthernetBuffer[ ipUDP_PAYLOAD_OFFSET_IPv4 ] );

            /* MISRA Ref 11.3.1 [Misaligned access] */
            /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
            /* coverity[misra_c_2012_rule_11_3_violation] */
            pxDHCPMessage = ( ( DHCPMessage_IPv4_t * ) pucUDPPayloadBuffer );

            /* Store the IP type at a known location.
             * Later the type must be known to translate
             * a payload- to a network buffer.
             */

            /* MISRA Ref 18.4.1 [Usage of +, -, += and -= operators on expression of pointer type]. */
            /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-184. */
            /* coverity[misra_c_2012_rule_18_4_violation] */
            pucIPType = pucUDPPayloadBuffer - ipUDP_PAYLOAD_IP_TYPE_OFFSET;
            *pucIPType = ipTYPE_IPv4;

            /* Most fields need to be zero. */
            ( void ) memset( pxDHCPMessage, 0x00, sizeof( DHCPMessage_IPv4_t ) );

            /* Create the message. */
            pxDHCPMessage->ucOpcode = ( uint8_t ) xOpcode;
            pxDHCPMessage->ucAddressType = ( uint8_t ) dhcpADDRESS_TYPE_ETHERNET;
            pxDHCPMessage->ucAddressLength = ( uint8_t ) dhcpETHERNET_ADDRESS_LENGTH;
            pxDHCPMessage->ulTransactionID = FreeRTOS_htonl( EP_DHCPData.ulTransactionId );
            pxDHCPMessage->ulDHCPCookie = ( uint32_t ) dhcpCOOKIE;

            if( EP_DHCPData.xUseBroadcast != pdFALSE )
            {
                pxDHCPMessage->usFlags = ( uint16_t ) dhcpBROADCAST;
            }
            else
            {
                pxDHCPMessage->usFlags = 0U;
            }

            ( void ) memcpy( &( pxDHCPMessage->ucClientHardwareAddress[ 0 ] ), pxEndPoint->xMACAddress.ucBytes, sizeof( MACAddress_t ) );

            /* Copy in the const part of the options options. */
            ( void ) memcpy( &( pucUDPPayloadBuffer[ dhcpFIRST_OPTION_BYTE_OFFSET ] ), pucOptionsArray, *pxOptionsArraySize );

            #if ( ipconfigDHCP_REGISTER_HOSTNAME == 1 )
            {
                /* With this option, the hostname can be registered as well which makes
                 * it easier to lookup a device in a router's list of DHCP clients. */

                /* Point to where the OPTION_END was stored to add data. */
                uint8_t * pucPtr = &( pucUDPPayloadBuffer[ dhcpFIRST_OPTION_BYTE_OFFSET + ( *pxOptionsArraySize - 1U ) ] );
                pucPtr[ 0U ] = dhcpIPv4_DNS_HOSTNAME_OPTIONS_CODE;
                pucPtr[ 1U ] = ( uint8_t ) uxNameLength;

                /*
                 * Use helper variables for memcpy() to remain
                 * compliant with MISRA Rule 21.15.  These should be
                 * optimized away.
                 */
                if( pucHostName != NULL )
                {
                    /* memcpy() helper variables for MISRA Rule 21.15 compliance*/
                    const void * pvCopySource = pucHostName;
                    void * pvCopyDest = &pucPtr[ 2U ];

                    ( void ) memcpy( pvCopyDest, pvCopySource, uxNameLength );
                }

                pucPtr[ 2U + uxNameLength ] = ( uint8_t ) dhcpOPTION_END_BYTE;
                *pxOptionsArraySize += ( size_t ) ( 2U + uxNameLength );
            }
            #endif /* if ( ipconfigDHCP_REGISTER_HOSTNAME == 1 ) */

            /* Map in the client identifier. */
            ( void ) memcpy( &( pucUDPPayloadBuffer[ dhcpFIRST_OPTION_BYTE_OFFSET + dhcpCLIENT_IDENTIFIER_OFFSET ] ),
                             pxEndPoint->xMACAddress.ucBytes, sizeof( MACAddress_t ) );

            /* Set the addressing. */
            pxAddress->sin_address.ulIP_IPv4 = ipBROADCAST_IP_ADDRESS;
            pxAddress->sin_port = ( uint16_t ) dhcpSERVER_PORT_IPv4;
            pxAddress->sin_family = FREERTOS_AF_INET4;
        }

        return pucUDPPayloadBuffer;
    }
    /*-----------------------------------------------------------*/

/**
 * @brief Create and send a DHCP request message through the DHCP socket.
 *
 * @param[in] pxEndPoint The end-point for which the request will be sent.
 */
    static BaseType_t prvSendDHCPRequest( NetworkEndPoint_t * pxEndPoint )
    {
        BaseType_t xResult = pdFAIL;
        uint8_t * pucUDPPayloadBuffer;
        struct freertos_sockaddr xAddress;
        static const uint8_t ucDHCPRequestOptions[] =
        {
            /* Do not change the ordering without also changing
             * dhcpCLIENT_IDENTIFIER_OFFSET, dhcpREQUESTED_IP_ADDRESS_OFFSET and
             * dhcpDHCP_SERVER_IP_ADDRESS_OFFSET. */
            dhcpIPv4_MESSAGE_TYPE_OPTION_CODE,       1, dhcpMESSAGE_TYPE_REQUEST, /* Message type option. */
            dhcpIPv4_CLIENT_IDENTIFIER_OPTION_CODE,  7, 1, 0, 0, 0, 0, 0, 0,      /* Client identifier. */
            dhcpIPv4_REQUEST_IP_ADDRESS_OPTION_CODE, 4, 0, 0, 0, 0,               /* The IP address being requested. */
            dhcpIPv4_SERVER_IP_ADDRESS_OPTION_CODE,  4, 0, 0, 0, 0,               /* The IP address of the DHCP server. */
            dhcpOPTION_END_BYTE
        };
        size_t uxOptionsLength = sizeof( ucDHCPRequestOptions );
        /* memcpy() helper variables for MISRA Rule 21.15 compliance*/
        const void * pvCopySource;
        void * pvCopyDest;

        /* MISRA doesn't like uninitialised structs. */
        ( void ) memset( &( xAddress ), 0, sizeof( xAddress ) );
        pucUDPPayloadBuffer = prvCreatePartDHCPMessage( &xAddress,
                                                        ( BaseType_t ) dhcpREQUEST_OPCODE,
                                                        ucDHCPRequestOptions,
                                                        &( uxOptionsLength ),
                                                        pxEndPoint );

        if( ( xSocketValid( EP_DHCPData.xDHCPSocket ) == pdTRUE ) && ( pucUDPPayloadBuffer != NULL ) )
        {
            /* Copy in the IP address being requested. */

            /*
             * Use helper variables for memcpy() source & dest to remain
             * compliant with MISRA Rule 21.15.  These should be
             * optimized away.
             */
            pvCopySource = &EP_DHCPData.ulOfferedIPAddress;
            pvCopyDest = &pucUDPPayloadBuffer[ dhcpFIRST_OPTION_BYTE_OFFSET + dhcpREQUESTED_IP_ADDRESS_OFFSET ];
            ( void ) memcpy( pvCopyDest, pvCopySource, sizeof( EP_DHCPData.ulOfferedIPAddress ) );

            /* Copy in the address of the DHCP server being used. */
            pvCopySource = &EP_DHCPData.ulDHCPServerAddress;
            pvCopyDest = &pucUDPPayloadBuffer[ dhcpFIRST_OPTION_BYTE_OFFSET + dhcpDHCP_SERVER_IP_ADDRESS_OFFSET ];
            ( void ) memcpy( pvCopyDest, pvCopySource, sizeof( EP_DHCPData.ulDHCPServerAddress ) );

            FreeRTOS_debug_printf( ( "vDHCPProcess: reply %xip\n", ( unsigned ) FreeRTOS_ntohl( EP_DHCPData.ulOfferedIPAddress ) ) );
            iptraceSENDING_DHCP_REQUEST();

            EP_DHCPData.xDHCPSocket->pxEndPoint = pxEndPoint;

            if( FreeRTOS_sendto( EP_DHCPData.xDHCPSocket, pucUDPPayloadBuffer, sizeof( DHCPMessage_IPv4_t ) + uxOptionsLength, FREERTOS_ZERO_COPY, &xAddress, ( socklen_t ) sizeof( xAddress ) ) == 0 )
            {
                /* The packet was not successfully queued for sending and must be
                 * returned to the stack. */
                FreeRTOS_ReleaseUDPPayloadBuffer( pucUDPPayloadBuffer );
            }
            else
            {
                xResult = pdPASS;
            }
        }

        return xResult;
    }
    /*-----------------------------------------------------------*/

/**
 * @brief Create and send a DHCP discover packet through the DHCP socket.
 *
 * @param[in] pxEndPoint the end-point for which the discover message will be sent.
 *
 * @return: pdPASS if the DHCP discover message was sent successfully, pdFAIL otherwise.
 */
    static BaseType_t prvSendDHCPDiscover( NetworkEndPoint_t * pxEndPoint )
    {
        BaseType_t xResult = pdFAIL;
        uint8_t * pucUDPPayloadBuffer;
        struct freertos_sockaddr xAddress;
        static const uint8_t ucDHCPDiscoverOptions[] =
        {
            /* Do not change the ordering without also changing dhcpCLIENT_IDENTIFIER_OFFSET. */
            dhcpIPv4_MESSAGE_TYPE_OPTION_CODE,       1, dhcpMESSAGE_TYPE_DISCOVER,                                                                        /* Message type option. */
            dhcpIPv4_CLIENT_IDENTIFIER_OPTION_CODE,  7, 1,                                0,                            0, 0, 0, 0, 0,                    /* Client identifier. */
            dhcpIPv4_REQUEST_IP_ADDRESS_OPTION_CODE, 4, 0,                                0,                            0, 0,                             /* The IP address being requested. */
            dhcpIPv4_PARAMETER_REQUEST_OPTION_CODE,  3, dhcpIPv4_SUBNET_MASK_OPTION_CODE, dhcpIPv4_GATEWAY_OPTION_CODE, dhcpIPv4_DNS_SERVER_OPTIONS_CODE, /* Parameter request option. */
            dhcpOPTION_END_BYTE
        };
        size_t uxOptionsLength = sizeof( ucDHCPDiscoverOptions );

        ( void ) memset( &( xAddress ), 0, sizeof( xAddress ) );
        pucUDPPayloadBuffer = prvCreatePartDHCPMessage( &xAddress,
                                                        ( BaseType_t ) dhcpREQUEST_OPCODE,
                                                        ucDHCPDiscoverOptions,
                                                        &( uxOptionsLength ),
                                                        pxEndPoint );

        /* MISRA Ref 11.4.1 [Socket error and integer to pointer conversion] */
        /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-114 */
        /* coverity[misra_c_2012_rule_11_4_violation] */
        if( ( xSocketValid( EP_DHCPData.xDHCPSocket ) == pdTRUE ) && ( pucUDPPayloadBuffer != NULL ) )
        {
            const void * pvCopySource;
            void * pvCopyDest;

            FreeRTOS_debug_printf( ( "vDHCPProcess: discover\n" ) );
            iptraceSENDING_DHCP_DISCOVER();

            if( pxEndPoint->xDHCPData.ulPreferredIPAddress != 0U )
            {
                /* Fill in the IPv4 address. */
                pvCopySource = &( pxEndPoint->xDHCPData.ulPreferredIPAddress );
                pvCopyDest = &( pucUDPPayloadBuffer[ dhcpFIRST_OPTION_BYTE_OFFSET + dhcpREQUESTED_IP_ADDRESS_OFFSET ] );
                ( void ) memcpy( pvCopyDest, pvCopySource, sizeof( EP_DHCPData.ulPreferredIPAddress ) );
            }
            else
            {
                /* Remove option-50 from the list because it is not used. */
                size_t uxCopyLength;
                /* Exclude this line from branch coverage as the not-taken condition will never happen unless the code is modified */
                configASSERT( uxOptionsLength > ( dhcpOPTION_50_OFFSET + dhcpOPTION_50_SIZE ) ); /* LCOV_EXCL_BR_LINE */
                uxCopyLength = uxOptionsLength - ( dhcpOPTION_50_OFFSET + dhcpOPTION_50_SIZE );
                pvCopySource = &( pucUDPPayloadBuffer[ dhcpFIRST_OPTION_BYTE_OFFSET + dhcpOPTION_50_OFFSET + dhcpOPTION_50_SIZE ] );
                pvCopyDest = &( pucUDPPayloadBuffer[ dhcpFIRST_OPTION_BYTE_OFFSET + dhcpOPTION_50_OFFSET ] );
                ( void ) memmove( pvCopyDest, pvCopySource, uxCopyLength );
                /* Send 6 bytes less than foreseen. */
                uxOptionsLength -= dhcpOPTION_50_SIZE;
            }

            EP_DHCPData.xDHCPSocket->pxEndPoint = pxEndPoint;

            if( FreeRTOS_sendto( EP_DHCPData.xDHCPSocket,
                                 pucUDPPayloadBuffer,
                                 sizeof( DHCPMessage_IPv4_t ) + uxOptionsLength,
                                 FREERTOS_ZERO_COPY,
                                 &( xAddress ),
                                 ( socklen_t ) sizeof( xAddress ) ) == 0 )
            {
                /* The packet was not successfully queued for sending and must be
                 * returned to the stack. */
                FreeRTOS_ReleaseUDPPayloadBuffer( pucUDPPayloadBuffer );
            }
            else
            {
                xResult = pdTRUE;
            }
        }

        return xResult;
    }
    /*-----------------------------------------------------------*/


    #if ( ipconfigDHCP_FALL_BACK_AUTO_IP != 0 )

/**
 * @brief When DHCP has failed, the code can assign a Link-Layer address, and check if
 *        another device already uses the IP-address.
 *
 * param[in] pxEndPoint The end-point that wants to obtain a link-layer address.
 */
        void prvPrepareLinkLayerIPLookUp( NetworkEndPoint_t * pxEndPoint )
        {
            uint8_t ucLinkLayerIPAddress[ 2 ];
            uint32_t ulNumbers[ 2 ];

            /* After DHCP has failed to answer, prepare everything to start
             * trying-out LinkLayer IP-addresses, using the random method. */
            EP_DHCPData.xDHCPTxTime = xTaskGetTickCount();

            ( void ) xApplicationGetRandomNumber( &( ulNumbers[ 0 ] ) );
            ( void ) xApplicationGetRandomNumber( &( ulNumbers[ 1 ] ) );
            ucLinkLayerIPAddress[ 0 ] = ( uint8_t ) ( 1 + ( ulNumbers[ 0 ] % 0xFDU ) ); /* get value 1..254 for IP-address 3rd byte of IP address to try. */
            ucLinkLayerIPAddress[ 1 ] = ( uint8_t ) ( 1 + ( ulNumbers[ 1 ] % 0xFDU ) ); /* get value 1..254 for IP-address 4th byte of IP address to try. */

            EP_IPv4_SETTINGS.ulGatewayAddress = 0U;

            /* prepare xDHCPData with data to test. */
            EP_DHCPData.ulOfferedIPAddress =
                FreeRTOS_inet_addr_quick( LINK_LAYER_ADDRESS_0, LINK_LAYER_ADDRESS_1, ucLinkLayerIPAddress[ 0 ], ucLinkLayerIPAddress[ 1 ] );

            EP_DHCPData.ulLeaseTime = dhcpDEFAULT_LEASE_TIME; /*  don't care about lease time. just put anything. */

            EP_IPv4_SETTINGS.ulNetMask =
                FreeRTOS_inet_addr_quick( LINK_LAYER_NETMASK_0, LINK_LAYER_NETMASK_1, LINK_LAYER_NETMASK_2, LINK_LAYER_NETMASK_3 );

            /* DHCP completed.  The IP address can now be used, and the
             * timer set to the lease timeout time. */
            EP_IPv4_SETTINGS.ulIPAddress = EP_DHCPData.ulOfferedIPAddress;

            /* Setting the 'local' broadcast address, something like 192.168.1.255' */
            EP_IPv4_SETTINGS.ulBroadcastAddress = ( EP_DHCPData.ulOfferedIPAddress & EP_IPv4_SETTINGS.ulNetMask ) | ~EP_IPv4_SETTINGS.ulNetMask;

            /* Close socket to ensure packets don't queue on it. not needed anymore as DHCP failed. but still need timer for ARP testing. */
            prvCloseDHCPSocket( pxEndPoint );

            xApplicationGetRandomNumber( &( ulNumbers[ 0 ] ) );
            EP_DHCPData.xDHCPTxPeriod = pdMS_TO_TICKS( 3000U + ( ulNumbers[ 0 ] & 0x3ffU ) ); /*  do ARP test every (3 + 0-1024mS) seconds. */

            xARPHadIPClash = pdFALSE;                                                         /* reset flag that shows if have ARP clash. */
            vARPSendGratuitous();
        }

    #endif /* ipconfigDHCP_FALL_BACK_AUTO_IP */
/*-----------------------------------------------------------*/

#endif /* ipconfigUSE_DHCP != 0 */
