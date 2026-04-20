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
 * @file FreeRTOS_IP.c
 * @brief Implements the basic functionality for the FreeRTOS+TCP network stack.
 */

/* Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* FreeRTOS+TCP includes. */
#include "FreeRTOS_IP.h"
#include "FreeRTOS_ICMP.h"
#include "FreeRTOS_IP_Timers.h"
#include "FreeRTOS_IP_Utils.h"
#include "FreeRTOS_Sockets.h"
#include "FreeRTOS_IP_Private.h"
#include "FreeRTOS_ARP.h"
#include "FreeRTOS_ND.h"
#include "FreeRTOS_UDP_IP.h"
#include "FreeRTOS_DHCP.h"
#if ( ipconfigUSE_DHCPv6 == 1 )
    #include "FreeRTOS_DHCPv6.h"
#endif
#include "NetworkInterface.h"
#include "NetworkBufferManagement.h"
#include "FreeRTOS_DNS.h"
#include "FreeRTOS_Routing.h"

/** @brief Time delay between repeated attempts to initialise the network hardware. */
#ifndef ipINITIALISATION_RETRY_DELAY
    #define ipINITIALISATION_RETRY_DELAY    ( pdMS_TO_TICKS( 3000U ) )
#endif

#if ( ipconfigUSE_TCP_MEM_STATS != 0 )
    #include "tcp_mem_stats.h"
#endif

/** @brief Maximum time to wait for an ARP resolution while holding a packet. */
#ifndef ipARP_RESOLUTION_MAX_DELAY
    #define ipARP_RESOLUTION_MAX_DELAY    ( pdMS_TO_TICKS( 2000U ) )
#endif

/** @brief Maximum time to wait for a ND resolution while holding a packet. */
#ifndef ipND_RESOLUTION_MAX_DELAY
    #define ipND_RESOLUTION_MAX_DELAY    ( pdMS_TO_TICKS( 2000U ) )
#endif

/** @brief Defines how often the ARP resolution timer callback function is executed.  The time is
 * shorter in the Windows simulator as simulated time is not real time. */
#ifndef ipARP_TIMER_PERIOD_MS
    #ifdef _WINDOWS_
        #define ipARP_TIMER_PERIOD_MS    ( 500U ) /* For windows simulator builds. */
    #else
        #define ipARP_TIMER_PERIOD_MS    ( 10000U )
    #endif
#endif

/** @brief Defines how often the ND resolution timer callback function is executed.  The time is
 * shorter in the Windows simulator as simulated time is not real time. */
#ifndef ipND_TIMER_PERIOD_MS
    #ifdef _WINDOWS_
        #define ipND_TIMER_PERIOD_MS    ( 500U ) /* For windows simulator builds. */
    #else
        #define ipND_TIMER_PERIOD_MS    ( 10000U )
    #endif
#endif

#if ( ( ipconfigUSE_TCP == 1 ) && !defined( ipTCP_TIMER_PERIOD_MS ) )
    /** @brief When initialising the TCP timer, give it an initial time-out of 1 second. */
    #define ipTCP_TIMER_PERIOD_MS    ( 1000U )
#endif

#ifndef iptraceIP_TASK_STARTING
    #define iptraceIP_TASK_STARTING()    do {} while( ipFALSE_BOOL ) /**< Empty definition in case iptraceIP_TASK_STARTING is not defined. */
#endif

/** @brief The frame type field in the Ethernet header must have a value greater than 0x0600.
 * If the configuration option ipconfigFILTER_OUT_NON_ETHERNET_II_FRAMES is enabled, the stack
 * will discard packets with a frame type value less than or equal to 0x0600.
 * However, if this option is disabled, the stack will continue to process these packets. */
#define ipIS_ETHERNET_FRAME_TYPE_INVALID( usFrameType )    ( ( usFrameType ) <= 0x0600U )

static void prvCallDHCP_RA_Handler( NetworkEndPoint_t * pxEndPoint );

static void prvIPTask_Initialise( void );

static void prvIPTask_CheckPendingEvents( void );

/*-----------------------------------------------------------*/

/** @brief The pointer to buffer with packet waiting for ARP resolution. */
#if ipconfigIS_ENABLED( ipconfigUSE_IPv4 )
    NetworkBufferDescriptor_t * pxARPWaitingNetworkBuffer = NULL;
#endif

/** @brief The pointer to buffer with packet waiting for ND resolution. */
#if ipconfigIS_ENABLED( ipconfigUSE_IPv6 )
    NetworkBufferDescriptor_t * pxNDWaitingNetworkBuffer = NULL;
#endif

/*-----------------------------------------------------------*/

static void prvProcessIPEventsAndTimers( void );

/*
 * The main TCP/IP stack processing task.  This task receives commands/events
 * from the network hardware drivers and tasks that are using sockets.  It also
 * maintains a set of protocol timers.
 */
static void prvIPTask( void * pvParameters );

/*
 * Called when new data is available from the network interface.
 */
static void prvProcessEthernetPacket( NetworkBufferDescriptor_t * const pxNetworkBuffer );

#if ( ipconfigPROCESS_CUSTOM_ETHERNET_FRAMES != 0 )

/*
 * The stack will call this user hook for all Ethernet frames that it
 * does not support, i.e. other than IPv4, IPv6 and ARP ( for the moment )
 * If this hook returns eReleaseBuffer or eProcessBuffer, the stack will
 * release and reuse the network buffer.  If this hook returns
 * eReturnEthernetFrame, that means user code has reused the network buffer
 * to generate a response and the stack will send that response out.
 * If this hook returns eFrameConsumed, the user code has ownership of the
 * network buffer and has to release it when it's done.
 */
    extern eFrameProcessingResult_t eApplicationProcessCustomFrameHook( NetworkBufferDescriptor_t * const pxNetworkBuffer );
#endif /* ( ipconfigPROCESS_CUSTOM_ETHERNET_FRAMES != 0 ) */

/*
 * Process incoming IP packets.
 */
static eFrameProcessingResult_t prvProcessIPPacket( const IPPacket_t * pxIPPacket,
                                                    NetworkBufferDescriptor_t * const pxNetworkBuffer );

/*
 * The network card driver has received a packet.  In the case that it is part
 * of a linked packet chain, walk through it to handle every message.
 */
static void prvHandleEthernetPacket( NetworkBufferDescriptor_t * pxBuffer );

/* Handle the 'eNetworkTxEvent': forward a packet from an application to the NIC. */
static void prvForwardTxPacket( NetworkBufferDescriptor_t * pxNetworkBuffer,
                                BaseType_t xReleaseAfterSend );

static eFrameProcessingResult_t prvProcessUDPPacket( NetworkBufferDescriptor_t * const pxNetworkBuffer );

/*-----------------------------------------------------------*/

/** @brief The queue used to pass events into the IP-task for processing. */
QueueHandle_t xNetworkEventQueue = NULL;

/** @brief The IP packet ID. */
uint16_t usPacketIdentifier = 0U;

/** @brief For convenience, a MAC address of all 0xffs is defined const for quick
 * reference. */
const MACAddress_t xBroadcastMACAddress = { { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff } };

/** @brief Used to ensure network down events cannot be missed when they cannot be
 * posted to the network event queue because the network event queue is already
 * full. */
static volatile BaseType_t xNetworkDownEventPending = pdFALSE;

/** @brief Stores the handle of the task that handles the stack.  The handle is used
 * (indirectly) by some utility function to determine if the utility function is
 * being called by a task (in which case it is ok to block) or by the IP task
 * itself (in which case it is not ok to block). */

static TaskHandle_t xIPTaskHandle = NULL;

/** @brief Set to pdTRUE when the IP task is ready to start processing packets. */
static BaseType_t xIPTaskInitialised = pdFALSE;

#if ( ipconfigCHECK_IP_QUEUE_SPACE != 0 )
    /** @brief Keep track of the lowest amount of space in 'xNetworkEventQueue'. */
    static UBaseType_t uxQueueMinimumSpace = ipconfigEVENT_QUEUE_LENGTH;
#endif

/*-----------------------------------------------------------*/

/* Coverity wants to make pvParameters const, which would make it incompatible. Leave the
 * function signature as is. */

/**
 * @brief The IP task handles all requests from the user application and the
 *        network interface. It receives messages through a FreeRTOS queue called
 *        'xNetworkEventQueue'. prvIPTask() is the only task which has access to
 *        the data of the IP-stack, and so it has no need of using mutexes.
 *
 * @param[in] pvParameters Not used.
 */

/** @brief Stores interface structures. */

/* MISRA Ref 8.13.1 [Not decorating a pointer to const parameter with const] */
/* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-813 */
/* coverity[misra_c_2012_rule_8_13_violation] */
static void prvIPTask( void * pvParameters )
{
    /* Just to prevent compiler warnings about unused parameters. */
    ( void ) pvParameters;

    prvIPTask_Initialise();

    FreeRTOS_debug_printf( ( "prvIPTask started\n" ) );

    /* Loop, processing IP events. */
    while( ipFOREVER() == pdTRUE )
    {
        prvProcessIPEventsAndTimers();
    }
}
/*-----------------------------------------------------------*/

/**
 * @brief Process the events sent to the IP task and process the timers.
 */
static void prvProcessIPEventsAndTimers( void )
{
    IPStackEvent_t xReceivedEvent;
    TickType_t xNextIPSleep;
    FreeRTOS_Socket_t * pxSocket;
    struct freertos_sockaddr xAddress;

    ipconfigWATCHDOG_TIMER();

    /* Check the Resolution, DHCP and TCP timers to see if there is any periodic
     * or timeout processing to perform. */
    vCheckNetworkTimers();

    /* Calculate the acceptable maximum sleep time. */
    xNextIPSleep = xCalculateSleepTime();

    /* Wait until there is something to do. If the following call exits
     * due to a time out rather than a message being received, set a
     * 'NoEvent' value. */
    if( xQueueReceive( xNetworkEventQueue, ( void * ) &xReceivedEvent, xNextIPSleep ) == pdFALSE )
    {
        xReceivedEvent.eEventType = eNoEvent;
    }

    #if ( ipconfigCHECK_IP_QUEUE_SPACE != 0 )
    {
        if( xReceivedEvent.eEventType != eNoEvent )
        {
            UBaseType_t uxCount;

            uxCount = uxQueueSpacesAvailable( xNetworkEventQueue );

            if( uxQueueMinimumSpace > uxCount )
            {
                uxQueueMinimumSpace = uxCount;
            }
        }
    }
    #endif /* ipconfigCHECK_IP_QUEUE_SPACE */

    iptraceNETWORK_EVENT_RECEIVED( xReceivedEvent.eEventType );

    switch( xReceivedEvent.eEventType )
    {
        case eNetworkDownEvent:
            /* Attempt to establish a connection. */
            prvProcessNetworkDownEvent( ( ( NetworkInterface_t * ) xReceivedEvent.pvData ) );
            break;

        case eNetworkRxEvent:

            /* The network hardware driver has received a new packet.  A
             * pointer to the received buffer is located in the pvData member
             * of the received event structure. */
            prvHandleEthernetPacket( ( NetworkBufferDescriptor_t * ) xReceivedEvent.pvData );
            break;

        case eNetworkTxEvent:

            /* Send a network packet. The ownership will  be transferred to
             * the driver, which will release it after delivery. */
            prvForwardTxPacket( ( ( NetworkBufferDescriptor_t * ) xReceivedEvent.pvData ), pdTRUE );
            break;

        case eARPTimerEvent:
            /* The ARP Resolution timer has expired, process the cache. */
            #if ipconfigIS_ENABLED( ipconfigUSE_IPv4 )
                vARPAgeCache();
            #endif /* ( ipconfigUSE_IPv4 != 0 ) */
            break;

        case eNDTimerEvent:
            /* The ND Resolution timer has expired, process the cache. */
            #if ipconfigIS_ENABLED( ipconfigUSE_IPv6 )
                vNDAgeCache();
            #endif /* ( ipconfigUSE_IPv6 != 0 ) */
            break;

        case eSocketBindEvent:

            /* FreeRTOS_bind (a user API) wants the IP-task to bind a socket
             * to a port. The port number is communicated in the socket field
             * usLocalPort. vSocketBind() will actually bind the socket and the
             * API will unblock as soon as the eSOCKET_BOUND event is
             * triggered. */
            pxSocket = ( ( FreeRTOS_Socket_t * ) xReceivedEvent.pvData );
            xAddress.sin_len = ( uint8_t ) sizeof( xAddress );

            switch( pxSocket->bits.bIsIPv6 ) /* LCOV_EXCL_BR_LINE */
            {
                #if ( ipconfigUSE_IPv4 != 0 )
                    case pdFALSE_UNSIGNED:
                        xAddress.sin_family = FREERTOS_AF_INET;
                        xAddress.sin_address.ulIP_IPv4 = FreeRTOS_htonl( pxSocket->xLocalAddress.ulIP_IPv4 );
                        /* 'ulLocalAddress' will be set again by vSocketBind(). */
                        pxSocket->xLocalAddress.ulIP_IPv4 = 0;
                        break;
                #endif /* ( ipconfigUSE_IPv4 != 0 ) */

                #if ( ipconfigUSE_IPv6 != 0 )
                    case pdTRUE_UNSIGNED:
                        xAddress.sin_family = FREERTOS_AF_INET6;
                        ( void ) memcpy( xAddress.sin_address.xIP_IPv6.ucBytes, pxSocket->xLocalAddress.xIP_IPv6.ucBytes, sizeof( xAddress.sin_address.xIP_IPv6.ucBytes ) );
                        /* 'ulLocalAddress' will be set again by vSocketBind(). */
                        ( void ) memset( pxSocket->xLocalAddress.xIP_IPv6.ucBytes, 0, sizeof( pxSocket->xLocalAddress.xIP_IPv6.ucBytes ) );
                        break;
                #endif /* ( ipconfigUSE_IPv6 != 0 ) */

                default:
                    /* MISRA 16.4 Compliance */
                    break;
            }

            xAddress.sin_port = FreeRTOS_ntohs( pxSocket->usLocalPort );
            /* 'usLocalPort' will be set again by vSocketBind(). */
            pxSocket->usLocalPort = 0U;
            ( void ) vSocketBind( pxSocket, &xAddress, sizeof( xAddress ), pdFALSE );

            /* Before 'eSocketBindEvent' was sent it was tested that
             * ( xEventGroup != NULL ) so it can be used now to wake up the
             * user. */
            pxSocket->xEventBits |= ( EventBits_t ) eSOCKET_BOUND;
            vSocketWakeUpUser( pxSocket );
            break;

        case eSocketCloseEvent:

            /* The user API FreeRTOS_closesocket() has sent a message to the
             * IP-task to actually close a socket. This is handled in
             * vSocketClose().  As the socket gets closed, there is no way to
             * report back to the API, so the API won't wait for the result */
            ( void ) vSocketClose( ( ( FreeRTOS_Socket_t * ) xReceivedEvent.pvData ) );
            break;

        case eStackTxEvent:

            /* The network stack has generated a packet to send.  A
             * pointer to the generated buffer is located in the pvData
             * member of the received event structure. */
            vProcessGeneratedUDPPacket( ( NetworkBufferDescriptor_t * ) xReceivedEvent.pvData );
            break;

        case eDHCPEvent:
            prvCallDHCP_RA_Handler( ( ( NetworkEndPoint_t * ) xReceivedEvent.pvData ) );
            break;

        case eSocketSelectEvent:

            /* FreeRTOS_select() has got unblocked by a socket event,
             * vSocketSelect() will check which sockets actually have an event
             * and update the socket field xSocketBits. */
            #if ( ipconfigSUPPORT_SELECT_FUNCTION == 1 )
            #if ( ipconfigSELECT_USES_NOTIFY != 0 )
                {
                    SocketSelectMessage_t * pxMessage = ( ( SocketSelectMessage_t * ) xReceivedEvent.pvData );
                    vSocketSelect( pxMessage->pxSocketSet );
                    ( void ) xTaskNotifyGive( pxMessage->xTaskhandle );
                }
            #else
                {
                    vSocketSelect( ( ( SocketSelect_t * ) xReceivedEvent.pvData ) );
                }
            #endif /* ( ipconfigSELECT_USES_NOTIFY != 0 ) */
            #endif /* ipconfigSUPPORT_SELECT_FUNCTION == 1 */
            break;

        case eSocketSignalEvent:
            #if ( ipconfigSUPPORT_SIGNALS != 0 )

                /* Some task wants to signal the user of this socket in
                 * order to interrupt a call to recv() or a call to select(). */
                ( void ) FreeRTOS_SignalSocket( ( Socket_t ) xReceivedEvent.pvData );
            #endif /* ipconfigSUPPORT_SIGNALS */
            break;

        case eTCPTimerEvent:
            #if ( ipconfigUSE_TCP == 1 )

                /* Simply mark the TCP timer as expired so it gets processed
                 * the next time prvCheckNetworkTimers() is called. */
                vIPSetTCPTimerExpiredState( pdTRUE );
            #endif /* ipconfigUSE_TCP */
            break;

        case eTCPAcceptEvent:

            /* The API FreeRTOS_accept() was called, the IP-task will now
             * check if the listening socket (communicated in pvData) actually
             * received a new connection. */
            #if ( ipconfigUSE_TCP == 1 )
                pxSocket = ( ( FreeRTOS_Socket_t * ) xReceivedEvent.pvData );

                if( xTCPCheckNewClient( pxSocket ) != pdFALSE )
                {
                    pxSocket->xEventBits |= ( EventBits_t ) eSOCKET_ACCEPT;
                    vSocketWakeUpUser( pxSocket );
                }
            #endif /* ipconfigUSE_TCP */
            break;

        case eTCPNetStat:

            /* FreeRTOS_netstat() was called to have the IP-task print an
             * overview of all sockets and their connections */
            #if ( ( ipconfigUSE_TCP == 1 ) && ( ipconfigHAS_PRINTF == 1 ) )
                vTCPNetStat();
            #endif /* ipconfigUSE_TCP */
            break;

        case eSocketSetDeleteEvent:
            #if ( ipconfigSUPPORT_SELECT_FUNCTION == 1 )
            {
                SocketSelect_t * pxSocketSet = ( SocketSelect_t * ) ( xReceivedEvent.pvData );

                iptraceMEM_STATS_DELETE( pxSocketSet );
                vEventGroupDelete( pxSocketSet->xSelectGroup );
                vPortFree( ( void * ) pxSocketSet );
            }
            #endif /* ipconfigSUPPORT_SELECT_FUNCTION == 1 */
            break;

        case eNoEvent:
            /* xQueueReceive() returned because of a normal time-out. */
            break;

        default:
            /* Should not get here. */
            break;
    }

    prvIPTask_CheckPendingEvents();
}

/*-----------------------------------------------------------*/

/**
 * @brief Helper function for prvIPTask, it does the first initializations
 *        at start-up. No parameters, no return type.
 */
static void prvIPTask_Initialise( void )
{
    NetworkInterface_t * pxInterface;

    /* A possibility to set some additional task properties. */
    iptraceIP_TASK_STARTING();

    /* Generate a dummy message to say that the network connection has gone
     * down.  This will cause this task to initialise the network interface.  After
     * this it is the responsibility of the network interface hardware driver to
     * send this message if a previously connected network is disconnected. */

    vNetworkTimerReload( pdMS_TO_TICKS( ipINITIALISATION_RETRY_DELAY ) );

    for( pxInterface = pxNetworkInterfaces; pxInterface != NULL; pxInterface = pxInterface->pxNext )
    {
        /* Post a 'eNetworkDownEvent' for every interface. */
        FreeRTOS_NetworkDown( pxInterface );
    }

    #if ( ipconfigUSE_TCP == 1 )
    {
        /* Initialise the TCP timer. */
        vTCPTimerReload( pdMS_TO_TICKS( ipTCP_TIMER_PERIOD_MS ) );
    }
    #endif

    #if ipconfigIS_ENABLED( ipconfigUSE_IPv4 )
        /* Mark the ARP timer as inactive since we are not waiting on any resolution as of now. */
        vIPSetARPResolutionTimerEnableState( pdFALSE );
    #endif

    #if ipconfigIS_ENABLED( ipconfigUSE_IPv6 )
        /* Mark the ND timer as inactive since we are not waiting on any resolution as of now. */
        vIPSetNDResolutionTimerEnableState( pdFALSE );
    #endif

    #if ( ( ipconfigDNS_USE_CALLBACKS != 0 ) && ( ipconfigUSE_DNS != 0 ) )
    {
        /* The following function is declared in FreeRTOS_DNS.c and 'private' to
         * this library */
        vDNSInitialise();
    }
    #endif /* ( ipconfigDNS_USE_CALLBACKS != 0 ) && ( ipconfigUSE_DNS != 0 ) */

    #if ( ( ipconfigUSE_DNS_CACHE != 0 ) && ( ipconfigUSE_DNS != 0 ) )
    {
        /* Clear the DNS cache once only. */
        FreeRTOS_dnsclear();
    }
    #endif /* ( ( ipconfigUSE_DNS_CACHE != 0 ) && ( ipconfigUSE_DNS != 0 ) ) */

    /* Initialisation is complete and events can now be processed. */
    xIPTaskInitialised = pdTRUE;
}
/*-----------------------------------------------------------*/

/**
 * @brief Check the value of 'xNetworkDownEventPending'. When non-zero, pending
 *        network-down events will be handled.
 */
static void prvIPTask_CheckPendingEvents( void )
{
    NetworkInterface_t * pxInterface;

    if( xNetworkDownEventPending != pdFALSE )
    {
        /* A network down event could not be posted to the network event
         * queue because the queue was full.
         * As this code runs in the IP-task, it can be done directly by
         * calling prvProcessNetworkDownEvent(). */
        xNetworkDownEventPending = pdFALSE;

        for( pxInterface = FreeRTOS_FirstNetworkInterface();
             pxInterface != NULL;
             pxInterface = FreeRTOS_NextNetworkInterface( pxInterface ) )
        {
            if( pxInterface->bits.bCallDownEvent != pdFALSE_UNSIGNED )
            {
                prvProcessNetworkDownEvent( pxInterface );
                pxInterface->bits.bCallDownEvent = pdFALSE_UNSIGNED;
            }
        }
    }
}

/*-----------------------------------------------------------*/

/**
 * @brief Call the state machine of either DHCP, DHCPv6, or RA, whichever is activated.
 *
 * @param[in] pxEndPoint The end-point for which the state-machine will be called.
 */
static void prvCallDHCP_RA_Handler( NetworkEndPoint_t * pxEndPoint )
{
    BaseType_t xIsIPv6 = pdFALSE;

    #if ( ( ipconfigUSE_DHCP == 1 ) || ( ipconfigUSE_DHCPv6 == 1 ) || ( ipconfigUSE_RA == 1 ) )
        if( pxEndPoint->bits.bIPv6 != pdFALSE_UNSIGNED )
        {
            xIsIPv6 = pdTRUE;
        }
    #endif
    /* The DHCP state machine needs processing. */
    #if ( ipconfigUSE_DHCP == 1 )
    {
        if( ( pxEndPoint->bits.bWantDHCP != pdFALSE_UNSIGNED ) && ( xIsIPv6 == pdFALSE ) )
        {
            /* Process DHCP messages for a given end-point. */
            vDHCPProcess( pdFALSE, pxEndPoint );
        }
    }
    #endif /* ipconfigUSE_DHCP */
    #if ( ( ipconfigUSE_DHCPv6 == 1 ) && ( ipconfigUSE_IPv6 != 0 ) )
    {
        if( ( xIsIPv6 == pdTRUE ) && ( pxEndPoint->bits.bWantDHCP != pdFALSE_UNSIGNED ) )
        {
            /* Process DHCPv6 messages for a given end-point. */
            vDHCPv6Process( pdFALSE, pxEndPoint );
        }
    }
    #endif /* ipconfigUSE_DHCPv6 */
    #if ( ( ipconfigUSE_RA == 1 ) && ( ipconfigUSE_IPv6 != 0 ) )
    {
        if( ( xIsIPv6 == pdTRUE ) && ( pxEndPoint->bits.bWantRA != pdFALSE_UNSIGNED ) )
        {
            /* Process RA messages for a given end-point. */
            vRAProcess( pdFALSE, pxEndPoint );
        }
    }
    #endif /* ( ( ipconfigUSE_RA == 1 ) && ( ipconfigUSE_IPv6 != 0 ) ) */

    /* Mention pxEndPoint and xIsIPv6 in case they have not been used. */
    ( void ) pxEndPoint;
    ( void ) xIsIPv6;
}
/*-----------------------------------------------------------*/

/**
 * @brief The variable 'xIPTaskHandle' is declared static.  This function
 *        gives read-only access to it.
 *
 * @return The handle of the IP-task.
 */
TaskHandle_t FreeRTOS_GetIPTaskHandle( void )
{
    return xIPTaskHandle;
}
/*-----------------------------------------------------------*/

/**
 * @brief Perform all the required tasks when the network gets connected.
 *
 * @param pxEndPoint The end-point which goes up.
 */
void vIPNetworkUpCalls( struct xNetworkEndPoint * pxEndPoint )
{
    if( pxEndPoint->bits.bIPv6 == pdTRUE_UNSIGNED )
    {
        /* IPv6 end-points have a solicited-node address that needs extra housekeeping. */
        #if ( ipconfigIS_ENABLED( ipconfigUSE_IPv6 ) )
            vManageSolicitedNodeAddress( pxEndPoint, pdTRUE );
        #endif
    }

    pxEndPoint->bits.bEndPointUp = pdTRUE_UNSIGNED;

    #if ( ipconfigUSE_NETWORK_EVENT_HOOK == 1 )
    #if ( ipconfigIPv4_BACKWARD_COMPATIBLE == 1 )
        {
            vApplicationIPNetworkEventHook( eNetworkUp );
        }
    #else
        {
            vApplicationIPNetworkEventHook_Multi( eNetworkUp, pxEndPoint );
        }
    #endif
    #endif /* ipconfigUSE_NETWORK_EVENT_HOOK */

    /* Set remaining time to 0 so it will become active immediately. */
    if( pxEndPoint->bits.bIPv6 == pdTRUE_UNSIGNED )
    {
        #if ipconfigIS_ENABLED( ipconfigUSE_IPv6 )
            vNDTimerReload( pdMS_TO_TICKS( ipND_TIMER_PERIOD_MS ) );
        #endif
    }
    else
    {
        #if ipconfigIS_ENABLED( ipconfigUSE_IPv4 )
            vARPTimerReload( pdMS_TO_TICKS( ipARP_TIMER_PERIOD_MS ) );
        #endif
    }
}
/*-----------------------------------------------------------*/

/**
 * @brief Handle the incoming Ethernet packets.
 *
 * @param[in] pxBuffer Linked/un-linked network buffer descriptor(s)
 *                      to be processed.
 */
static void prvHandleEthernetPacket( NetworkBufferDescriptor_t * pxBuffer )
{
    #if ( ipconfigUSE_LINKED_RX_MESSAGES == 0 )
    {
        /* When ipconfigUSE_LINKED_RX_MESSAGES is set to 0 then only one
         * buffer will be sent at a time.  This is the default way for +TCP to pass
         * messages from the MAC to the TCP/IP stack. */
        if( pxBuffer != NULL )
        {
            prvProcessEthernetPacket( pxBuffer );
        }
    }
    #else /* ipconfigUSE_LINKED_RX_MESSAGES */
    {
        NetworkBufferDescriptor_t * pxNextBuffer;

        /* An optimisation that is useful when there is high network traffic.
         * Instead of passing received packets into the IP task one at a time the
         * network interface can chain received packets together and pass them into
         * the IP task in one go.  The packets are chained using the pxNextBuffer
         * member.  The loop below walks through the chain processing each packet
         * in the chain in turn. */

        /* While there is another packet in the chain. */
        while( pxBuffer != NULL )
        {
            /* Store a pointer to the buffer after pxBuffer for use later on. */
            pxNextBuffer = pxBuffer->pxNextBuffer;

            /* Make it NULL to avoid using it later on. */
            pxBuffer->pxNextBuffer = NULL;

            prvProcessEthernetPacket( pxBuffer );
            pxBuffer = pxNextBuffer;
        }
    }
    #endif /* ipconfigUSE_LINKED_RX_MESSAGES */
}
/*-----------------------------------------------------------*/

/**
 * @brief Send a network packet.
 *
 * @param[in] pxNetworkBuffer The message buffer.
 * @param[in] xReleaseAfterSend When true, the network interface will own the buffer and is responsible for it's release.
 */
static void prvForwardTxPacket( NetworkBufferDescriptor_t * pxNetworkBuffer,
                                BaseType_t xReleaseAfterSend )
{
    iptraceNETWORK_INTERFACE_OUTPUT( pxNetworkBuffer->xDataLength, pxNetworkBuffer->pucEthernetBuffer );

    if( pxNetworkBuffer->pxInterface != NULL )
    {
        ( void ) pxNetworkBuffer->pxInterface->pfOutput( pxNetworkBuffer->pxInterface, pxNetworkBuffer, xReleaseAfterSend );
    }
}
/*-----------------------------------------------------------*/

/**
 * @brief Send a network down event to the IP-task. If it fails to post a message,
 *         the failure will be noted in the variable 'xNetworkDownEventPending'
 *         and later on a 'network-down' event, it will be executed.
 *
 * @param[in] pxNetworkInterface The interface that goes down.
 */
void FreeRTOS_NetworkDown( struct xNetworkInterface * pxNetworkInterface )
{
    IPStackEvent_t xNetworkDownEvent;
    const TickType_t xDontBlock = ( TickType_t ) 0;

    pxNetworkInterface->bits.bInterfaceUp = pdFALSE_UNSIGNED;
    xNetworkDownEvent.eEventType = eNetworkDownEvent;
    xNetworkDownEvent.pvData = pxNetworkInterface;

    /* Simply send the network task the appropriate event. */
    if( xSendEventStructToIPTask( &xNetworkDownEvent, xDontBlock ) != pdPASS )
    {
        /* Could not send the message, so it is still pending. */
        pxNetworkInterface->bits.bCallDownEvent = pdTRUE;
        xNetworkDownEventPending = pdTRUE;
    }
    else
    {
        /* Message was sent so it is not pending. */
        pxNetworkInterface->bits.bCallDownEvent = pdFALSE;
    }

    iptraceNETWORK_DOWN();
}
/*-----------------------------------------------------------*/

/**
 * @brief Utility function. Process Network Down event from ISR.
 *        This function is supposed to be called form an ISR. It is recommended
 * - *        use 'FreeRTOS_NetworkDown()', when calling from a normal task.
 *
 * @param[in] pxNetworkInterface The interface that goes down.
 *
 * @return If the event was processed successfully, then return pdTRUE.
 *         Else pdFALSE.
 */
BaseType_t FreeRTOS_NetworkDownFromISR( struct xNetworkInterface * pxNetworkInterface )
{
    IPStackEvent_t xNetworkDownEvent;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    xNetworkDownEvent.eEventType = eNetworkDownEvent;
    xNetworkDownEvent.pvData = pxNetworkInterface;

    /* Simply send the network task the appropriate event. */
    if( xQueueSendToBackFromISR( xNetworkEventQueue, &xNetworkDownEvent, &xHigherPriorityTaskWoken ) != pdPASS )
    {
        /* Could not send the message, so it is still pending. */
        pxNetworkInterface->bits.bCallDownEvent = pdTRUE;
        xNetworkDownEventPending = pdTRUE;
    }
    else
    {
        /* Message was sent so it is not pending. */
        pxNetworkInterface->bits.bCallDownEvent = pdFALSE;
        xNetworkDownEventPending = pdFALSE;
    }

    iptraceNETWORK_DOWN();

    return xHigherPriorityTaskWoken;
}
/*-----------------------------------------------------------*/

#if ( ipconfigIPv4_BACKWARD_COMPATIBLE == 1 )

/**
 * @brief Obtain a buffer big enough for a UDP payload of given size.
 *        NOTE: This function is kept for backward compatibility and will
 *        only allocate IPv4 payload buffers. Newer designs should use
 *        FreeRTOS_GetUDPPayloadBuffer_Multi(), which can
 *        allocate a IPv4 or IPv6 buffer based on ucIPType parameter .
 *
 * @param[in] uxRequestedSizeBytes The size of the UDP payload.
 * @param[in] uxBlockTimeTicks Maximum amount of time for which this call
 *            can block. This value is capped internally.
 *
 * @return If a buffer was created then the pointer to that buffer is returned,
 *         else a NULL pointer is returned.
 */
    void * FreeRTOS_GetUDPPayloadBuffer( size_t uxRequestedSizeBytes,
                                         TickType_t uxBlockTimeTicks )
    {
        return FreeRTOS_GetUDPPayloadBuffer_Multi( uxRequestedSizeBytes, uxBlockTimeTicks, ipTYPE_IPv4 );
    }
#endif /* if ( ipconfigIPv4_BACKWARD_COMPATIBLE == 1 ) */
/*-----------------------------------------------------------*/

/**
 * @brief Obtain a buffer big enough for a UDP payload of given size and
 *        given IP type.
 *
 * @param[in] uxRequestedSizeBytes The size of the UDP payload.
 * @param[in] uxBlockTimeTicks Maximum amount of time for which this call
 *            can block. This value is capped internally.
 * @param[in] ucIPType Either ipTYPE_IPv4 (0x40) or ipTYPE_IPv6 (0x60)
 *
 * @return If a buffer was created then the pointer to that buffer is returned,
 *         else a NULL pointer is returned.
 */
void * FreeRTOS_GetUDPPayloadBuffer_Multi( size_t uxRequestedSizeBytes,
                                           TickType_t uxBlockTimeTicks,
                                           uint8_t ucIPType )
{
    NetworkBufferDescriptor_t * pxNetworkBuffer;
    void * pvReturn = NULL;
    TickType_t uxBlockTime = uxBlockTimeTicks;
    size_t uxPayloadOffset = 0U;

    configASSERT( ( ucIPType == ipTYPE_IPv6 ) || ( ucIPType == ipTYPE_IPv4 ) );

    /* Cap the block time.  The reason for this is explained where
     * ipconfigUDP_MAX_SEND_BLOCK_TIME_TICKS is defined (assuming an official
     * FreeRTOSIPConfig.h header file is being used). */
    if( uxBlockTime > ipconfigUDP_MAX_SEND_BLOCK_TIME_TICKS )
    {
        uxBlockTime = ipconfigUDP_MAX_SEND_BLOCK_TIME_TICKS;
    }

    switch( ucIPType ) /* LCOV_EXCL_BR_LINE */
    {
        #if ( ipconfigUSE_IPv4 != 0 )
            case ipTYPE_IPv4:
                uxPayloadOffset = sizeof( UDPPacket_t );
                break;
        #endif /* ( ipconfigUSE_IPv4 != 0 ) */

        #if ( ipconfigUSE_IPv6 != 0 )
            case ipTYPE_IPv6:
                uxPayloadOffset = sizeof( UDPPacket_IPv6_t );
                break;
        #endif /* ( ipconfigUSE_IPv6 != 0 ) */

        default:
            /* Shouldn't reach here. */
            /* MISRA 16.4 Compliance */
            break;
    }

    if( uxPayloadOffset != 0U )
    {
        /* Obtain a network buffer with the required amount of storage. */
        pxNetworkBuffer = pxGetNetworkBufferWithDescriptor( uxPayloadOffset + uxRequestedSizeBytes, uxBlockTime );

        if( pxNetworkBuffer != NULL )
        {
            uint8_t * pucIPType;
            size_t uxIndex = ipUDP_PAYLOAD_IP_TYPE_OFFSET;
            BaseType_t xPayloadIPTypeOffset = ( BaseType_t ) uxIndex;

            /* Set the actual packet size in case a bigger buffer was returned. */
            pxNetworkBuffer->xDataLength = uxPayloadOffset + uxRequestedSizeBytes;

            /* Skip 3 headers. */
            pvReturn = ( void * ) &( pxNetworkBuffer->pucEthernetBuffer[ uxPayloadOffset ] );

            /* Later a pointer to a UDP payload is used to retrieve a NetworkBuffer.
             * Store the packet type at 48 bytes before the start of the UDP payload. */
            pucIPType = ( uint8_t * ) pvReturn;
            pucIPType = &( pucIPType[ -xPayloadIPTypeOffset ] );

            /* For a IPv4 packet, pucIPType points to 6 bytes before the
             * pucEthernetBuffer, for a IPv6 packet, pucIPType will point to the
             * first byte of the IP-header: 'ucVersionTrafficClass'. */
            *pucIPType = ucIPType;
        }
    }

    return ( void * ) pvReturn;
}
/*-----------------------------------------------------------*/

/*_RB_ Should we add an error or assert if the task priorities are set such that the servers won't function as expected? */

/*_HT_ There was a bug in FreeRTOS_TCP_IP.c that only occurred when the applications' priority was too high.
 * As that bug has been repaired, there is not an urgent reason to warn.
 * It is better though to use the advised priority scheme. */

#if ( ipconfigIPv4_BACKWARD_COMPATIBLE == 1 ) && ( ipconfigUSE_IPv4 != 0 )

/* Provide backward-compatibility with the earlier FreeRTOS+TCP which only had
 * single network interface. */
    BaseType_t FreeRTOS_IPInit( const uint8_t ucIPAddress[ ipIP_ADDRESS_LENGTH_BYTES ],
                                const uint8_t ucNetMask[ ipIP_ADDRESS_LENGTH_BYTES ],
                                const uint8_t ucGatewayAddress[ ipIP_ADDRESS_LENGTH_BYTES ],
                                const uint8_t ucDNSServerAddress[ ipIP_ADDRESS_LENGTH_BYTES ],
                                const uint8_t ucMACAddress[ ipMAC_ADDRESS_LENGTH_BYTES ] )
    {
        static NetworkInterface_t xInterfaces[ 1 ];
        static NetworkEndPoint_t xEndPoints[ 1 ];

        /* IF the following function should be declared in the NetworkInterface.c
         * linked in the project. */
        ( void ) pxFillInterfaceDescriptor( 0, &( xInterfaces[ 0 ] ) );
        FreeRTOS_FillEndPoint( &( xInterfaces[ 0 ] ), &( xEndPoints[ 0 ] ), ucIPAddress, ucNetMask, ucGatewayAddress, ucDNSServerAddress, ucMACAddress );
        #if ( ipconfigUSE_DHCP != 0 )
        {
            xEndPoints[ 0 ].bits.bWantDHCP = pdTRUE;
        }
        #endif /* ipconfigUSE_DHCP */
        return FreeRTOS_IPInit_Multi();
    }
#endif /* if ( ipconfigIPv4_BACKWARD_COMPATIBLE == 1 ) && ( ipconfigUSE_IPv4 != 0 ) */
/*-----------------------------------------------------------*/

/**
 * @brief Initialise the FreeRTOS-Plus-TCP network stack and initialise the IP-task.
 *        Before calling this function, at least 1 interface and 1 end-point must
 *        have been set-up.
 */
BaseType_t FreeRTOS_IPInit_Multi( void )
{
    BaseType_t xReturn = pdFALSE;

    /* There must be at least one interface and one end-point. */
    configASSERT( FreeRTOS_FirstNetworkInterface() != NULL );

    /* Check that the configuration values are correct and that the IP-task has not
     * already been initialized. */
    vPreCheckConfigs();

    /* Attempt to create the queue used to communicate with the IP task. */
    #if ( configSUPPORT_STATIC_ALLOCATION == 1 )
    {
        static StaticQueue_t xNetworkEventStaticQueue;
        static uint8_t ucNetworkEventQueueStorageArea[ ipconfigEVENT_QUEUE_LENGTH * sizeof( IPStackEvent_t ) ];
        xNetworkEventQueue = xQueueCreateStatic( ipconfigEVENT_QUEUE_LENGTH,
                                                 sizeof( IPStackEvent_t ),
                                                 ucNetworkEventQueueStorageArea,
                                                 &xNetworkEventStaticQueue );
    }
    #else
    {
        xNetworkEventQueue = xQueueCreate( ipconfigEVENT_QUEUE_LENGTH, sizeof( IPStackEvent_t ) );
        configASSERT( xNetworkEventQueue != NULL );
    }
    #endif /* configSUPPORT_STATIC_ALLOCATION */

    if( xNetworkEventQueue != NULL )
    {
        #if ( configQUEUE_REGISTRY_SIZE > 0 )
        {
            /* A queue registry is normally used to assist a kernel aware
             * debugger.  If one is in use then it will be helpful for the debugger
             * to show information about the network event queue. */
            vQueueAddToRegistry( xNetworkEventQueue, "NetEvnt" );
        }
        #endif /* configQUEUE_REGISTRY_SIZE */

        if( xNetworkBuffersInitialise() == pdPASS )
        {
            /* Prepare the sockets interface. */
            vNetworkSocketsInit();

            /* Create the task that processes Ethernet and stack events. */
            #if ( configSUPPORT_STATIC_ALLOCATION == 1 )
            {
                static StaticTask_t xIPTaskBuffer;
                static StackType_t xIPTaskStack[ ipconfigIP_TASK_STACK_SIZE_WORDS ];
                xIPTaskHandle = xTaskCreateStatic( &prvIPTask,
                                                   "IP-Task",
                                                   ipconfigIP_TASK_STACK_SIZE_WORDS,
                                                   NULL,
                                                   ipconfigIP_TASK_PRIORITY,
                                                   xIPTaskStack,
                                                   &xIPTaskBuffer );

                if( xIPTaskHandle != NULL )
                {
                    xReturn = pdTRUE;
                }
            }
            #else /* if ( configSUPPORT_STATIC_ALLOCATION == 1 ) */
            {
                xReturn = xTaskCreate( &prvIPTask,
                                       "IP-task",
                                       ipconfigIP_TASK_STACK_SIZE_WORDS,
                                       NULL,
                                       ipconfigIP_TASK_PRIORITY,
                                       &( xIPTaskHandle ) );
            }
            #endif /* configSUPPORT_STATIC_ALLOCATION */
        }
        else
        {
            FreeRTOS_debug_printf( ( "FreeRTOS_IPInit_Multi: xNetworkBuffersInitialise() failed\n" ) );

            /* Clean up. */
            vQueueDelete( xNetworkEventQueue );
            xNetworkEventQueue = NULL;
        }
    }
    else
    {
        FreeRTOS_debug_printf( ( "FreeRTOS_IPInit_Multi: Network event queue could not be created\n" ) );
    }

    return xReturn;
}
/*-----------------------------------------------------------*/

/**
 * @brief Release the UDP payload buffer.
 *
 * @param[in] pvBuffer Pointer to the UDP buffer that is to be released.
 */
void FreeRTOS_ReleaseUDPPayloadBuffer( void const * pvBuffer )
{
    NetworkBufferDescriptor_t * pxBuffer;

    pxBuffer = pxUDPPayloadBuffer_to_NetworkBuffer( pvBuffer );
    configASSERT( pxBuffer != NULL );
    vReleaseNetworkBufferAndDescriptor( pxBuffer );
}
/*-----------------------------------------------------------*/

#if ( ipconfigUSE_IPv4 != 0 )

/**
 * @brief Get the current IPv4 address configuration. Only non-NULL pointers will
 *        be filled in. pxEndPoint must be non-NULL.
 *
 * @param[out] pulIPAddress The current IP-address assigned.
 * @param[out] pulNetMask The netmask used for current subnet.
 * @param[out] pulGatewayAddress The gateway address.
 * @param[out] pulDNSServerAddress The DNS server address.
 * @param[in] pxEndPoint The end-point which is being questioned.
 */
    void FreeRTOS_GetEndPointConfiguration( uint32_t * pulIPAddress,
                                            uint32_t * pulNetMask,
                                            uint32_t * pulGatewayAddress,
                                            uint32_t * pulDNSServerAddress,
                                            const struct xNetworkEndPoint * pxEndPoint )
    {
        if( ENDPOINT_IS_IPv4( pxEndPoint ) )
        {
            /* Return the address configuration to the caller. */

            if( pulIPAddress != NULL )
            {
                *pulIPAddress = pxEndPoint->ipv4_settings.ulIPAddress;
            }

            if( pulNetMask != NULL )
            {
                *pulNetMask = pxEndPoint->ipv4_settings.ulNetMask;
            }

            if( pulGatewayAddress != NULL )
            {
                *pulGatewayAddress = pxEndPoint->ipv4_settings.ulGatewayAddress;
            }

            if( pulDNSServerAddress != NULL )
            {
                *pulDNSServerAddress = pxEndPoint->ipv4_settings.ulDNSServerAddresses[ 0 ]; /*_RB_ Only returning the address of the first DNS server. */
            }
        }
    }
/*-----------------------------------------------------------*/

#endif /* ( ipconfigUSE_IPv4 != 0 ) */

#if ( ipconfigIPv4_BACKWARD_COMPATIBLE == 1 ) && ( ipconfigUSE_IPv4 != 0 )

/**
 * @brief Get the current IPv4 address configuration of the first endpoint.
 *        Only non-NULL pointers will be filled in.
 *        NOTE: This function is kept for backward compatibility. Newer
 *        designs should use FreeRTOS_SetEndPointConfiguration().
 *
 * @param[out] pulIPAddress The current IP-address assigned.
 * @param[out] pulNetMask The netmask used for current subnet.
 * @param[out] pulGatewayAddress The gateway address.
 * @param[out] pulDNSServerAddress The DNS server address.
 */
    void FreeRTOS_GetAddressConfiguration( uint32_t * pulIPAddress,
                                           uint32_t * pulNetMask,
                                           uint32_t * pulGatewayAddress,
                                           uint32_t * pulDNSServerAddress )
    {
        NetworkEndPoint_t * pxEndPoint;

        /* Get first end point. */
        pxEndPoint = FreeRTOS_FirstEndPoint( NULL );

        if( pxEndPoint != NULL )
        {
            FreeRTOS_GetEndPointConfiguration( pulIPAddress, pulNetMask,
                                               pulGatewayAddress, pulDNSServerAddress, pxEndPoint );
        }
    }
#endif /* if ( ipconfigIPv4_BACKWARD_COMPATIBLE == 1 ) && ( ipconfigUSE_IPv4 != 0 ) */
/*-----------------------------------------------------------*/

#if ( ipconfigUSE_IPv4 != 0 )

/**
 * @brief Set the current IPv4 network address configuration. Only non-NULL pointers will
 *        pointers will be used. pxEndPoint must pointer to a valid end-point.
 *
 * @param[in] pulIPAddress The current IP-address assigned.
 * @param[in] pulNetMask The netmask used for current subnet.
 * @param[in] pulGatewayAddress The gateway address.
 * @param[in] pulDNSServerAddress The DNS server address.
 * @param[in] pxEndPoint The end-point which is being questioned.
 */
    void FreeRTOS_SetEndPointConfiguration( const uint32_t * pulIPAddress,
                                            const uint32_t * pulNetMask,
                                            const uint32_t * pulGatewayAddress,
                                            const uint32_t * pulDNSServerAddress,
                                            struct xNetworkEndPoint * pxEndPoint )
    {
        /* Update the address configuration. */

        if( ENDPOINT_IS_IPv4( pxEndPoint ) )
        {
            if( pulIPAddress != NULL )
            {
                pxEndPoint->ipv4_settings.ulIPAddress = *pulIPAddress;
            }

            if( pulNetMask != NULL )
            {
                pxEndPoint->ipv4_settings.ulNetMask = *pulNetMask;
            }

            if( pulGatewayAddress != NULL )
            {
                pxEndPoint->ipv4_settings.ulGatewayAddress = *pulGatewayAddress;
            }

            if( pulDNSServerAddress != NULL )
            {
                pxEndPoint->ipv4_settings.ulDNSServerAddresses[ 0 ] = *pulDNSServerAddress;
            }
        }
    }
/*-----------------------------------------------------------*/

#endif /* ( ipconfigUSE_IPv4 != 0 ) */

#if ( ipconfigIPv4_BACKWARD_COMPATIBLE == 1 ) && ( ipconfigUSE_IPv4 != 0 )

/**
 * @brief Set the current IPv4 network address configuration. Only non-NULL
 *        pointers will be used.
 *        NOTE: This function is kept for backward compatibility. Newer
 *        designs should use FreeRTOS_SetEndPointConfiguration().
 *
 * @param[in] pulIPAddress The current IP-address assigned.
 * @param[in] pulNetMask The netmask used for current subnet.
 * @param[in] pulGatewayAddress The gateway address.
 * @param[in] pulDNSServerAddress The DNS server address.
 */
    void FreeRTOS_SetAddressConfiguration( const uint32_t * pulIPAddress,
                                           const uint32_t * pulNetMask,
                                           const uint32_t * pulGatewayAddress,
                                           const uint32_t * pulDNSServerAddress )
    {
        NetworkEndPoint_t * pxEndPoint;

        /* Get first end point. */
        pxEndPoint = FreeRTOS_FirstEndPoint( NULL );

        if( pxEndPoint != NULL )
        {
            FreeRTOS_SetEndPointConfiguration( pulIPAddress, pulNetMask,
                                               pulGatewayAddress, pulDNSServerAddress, pxEndPoint );
        }
    }
#endif /* if ( ipconfigIPv4_BACKWARD_COMPATIBLE == 1 ) && ( ipconfigUSE_IPv4 != 0 ) */
/*-----------------------------------------------------------*/

#if ( ipconfigUSE_TCP == 1 )

/**
 * @brief Release the memory that was previously obtained by calling FreeRTOS_recv()
 *        with the flag 'FREERTOS_ZERO_COPY'.
 *
 * @param[in] xSocket The socket that was read from.
 * @param[in] pvBuffer The buffer returned in the call to FreeRTOS_recv().
 * @param[in] xByteCount The number of bytes that have been used.
 *
 * @return pdPASS if the buffer was released successfully, otherwise pdFAIL is returned.
 */
    BaseType_t FreeRTOS_ReleaseTCPPayloadBuffer( Socket_t xSocket,
                                                 void const * pvBuffer,
                                                 BaseType_t xByteCount )
    {
        BaseType_t xByteCountReleased;
        BaseType_t xReturn = pdFAIL;
        uint8_t * pucData;
        size_t uxBytesAvailable = uxStreamBufferGetPtr( xSocket->u.xTCP.rxStream, &( pucData ) );

        /* Make sure the pointer is correct. */
        configASSERT( pucData == ( uint8_t * ) pvBuffer );

        /* Avoid releasing more bytes than available. */
        configASSERT( uxBytesAvailable >= ( size_t ) xByteCount );

        if( ( pucData == pvBuffer ) && ( uxBytesAvailable >= ( size_t ) xByteCount ) )
        {
            /* Call recv with NULL pointer to advance the circular buffer. */
            xByteCountReleased = FreeRTOS_recv( xSocket,
                                                NULL,
                                                ( size_t ) xByteCount,
                                                FREERTOS_MSG_DONTWAIT );

            configASSERT( xByteCountReleased == xByteCount );

            if( xByteCountReleased == xByteCount )
            {
                xReturn = pdPASS;
            }
        }

        return xReturn;
    }
#endif /* ( ipconfigUSE_TCP == 1 ) */
/*-----------------------------------------------------------*/

#if ( ipconfigSUPPORT_OUTGOING_PINGS == 1 )

/**
 * @brief Send a ping request to the given IP address. After receiving a reply,
 *        IP-task will call a user-supplied function 'vApplicationPingReplyHook()'.
 *
 * @param[in] ulIPAddress The IP address to which the ping is to be sent.
 * @param[in] uxNumberOfBytesToSend Number of bytes in the ping request.
 * @param[in] uxBlockTimeTicks Maximum number of ticks to wait.
 *
 * @return If successfully sent to IP task for processing then the sequence
 *         number of the ping packet or else, pdFAIL.
 */
    BaseType_t FreeRTOS_SendPingRequest( uint32_t ulIPAddress,
                                         size_t uxNumberOfBytesToSend,
                                         TickType_t uxBlockTimeTicks )
    {
        NetworkBufferDescriptor_t * pxNetworkBuffer;
        ICMPHeader_t * pxICMPHeader;
        EthernetHeader_t * pxEthernetHeader;
        BaseType_t xReturn = pdFAIL;
        static uint16_t usSequenceNumber = 0;
        uint8_t * pucChar;
        size_t uxTotalLength;
        BaseType_t xEnoughSpace;
        IPStackEvent_t xStackTxEvent = { eStackTxEvent, NULL };

        uxTotalLength = uxNumberOfBytesToSend + sizeof( ICMPPacket_t );

        if( uxNumberOfBytesToSend < ( ipconfigNETWORK_MTU - ( sizeof( IPHeader_t ) + sizeof( ICMPHeader_t ) ) ) )
        {
            xEnoughSpace = pdTRUE;
        }
        else
        {
            xEnoughSpace = pdFALSE;
        }

        if( ( uxGetNumberOfFreeNetworkBuffers() >= 4U ) && ( uxNumberOfBytesToSend >= 1U ) && ( xEnoughSpace != pdFALSE ) )
        {
            pxNetworkBuffer = pxGetNetworkBufferWithDescriptor( uxTotalLength, uxBlockTimeTicks );

            if( pxNetworkBuffer != NULL )
            {
                pxEthernetHeader = ( ( EthernetHeader_t * ) pxNetworkBuffer->pucEthernetBuffer );
                pxEthernetHeader->usFrameType = ipIPv4_FRAME_TYPE;

                pxICMPHeader = ( ( ICMPHeader_t * ) &( pxNetworkBuffer->pucEthernetBuffer[ ipIP_PAYLOAD_OFFSET ] ) );
                usSequenceNumber++;

                /* Fill in the basic header information. */
                pxICMPHeader->ucTypeOfMessage = ipICMP_ECHO_REQUEST;
                pxICMPHeader->ucTypeOfService = 0;
                pxICMPHeader->usIdentifier = usSequenceNumber;
                pxICMPHeader->usSequenceNumber = usSequenceNumber;

                /* Find the start of the data. */
                pucChar = ( uint8_t * ) pxICMPHeader;
                pucChar = &( pucChar[ sizeof( ICMPHeader_t ) ] );

                /* Just memset the data to a fixed value. */
                ( void ) memset( pucChar, ( int ) ipECHO_DATA_FILL_BYTE, uxNumberOfBytesToSend );

                /* The message is complete, IP and checksum's are handled by
                 * vProcessGeneratedUDPPacket */
                pxNetworkBuffer->pucEthernetBuffer[ ipSOCKET_OPTIONS_OFFSET ] = FREERTOS_SO_UDPCKSUM_OUT;
                pxNetworkBuffer->xIPAddress.ulIP_IPv4 = ulIPAddress;
                pxNetworkBuffer->usPort = ipPACKET_CONTAINS_ICMP_DATA;
                /* xDataLength is the size of the total packet, including the Ethernet header. */
                pxNetworkBuffer->xDataLength = uxTotalLength;

                /* Send to the stack. */
                xStackTxEvent.pvData = pxNetworkBuffer;

                if( xSendEventStructToIPTask( &( xStackTxEvent ), uxBlockTimeTicks ) != pdPASS )
                {
                    vReleaseNetworkBufferAndDescriptor( pxNetworkBuffer );
                    iptraceSTACK_TX_EVENT_LOST( ipSTACK_TX_EVENT );
                }
                else
                {
                    xReturn = ( BaseType_t ) usSequenceNumber;
                }
            }
        }
        else
        {
            /* The requested number of bytes will not fit in the available space
             * in the network buffer. */
        }

        return xReturn;
    }

#endif /* ipconfigSUPPORT_OUTGOING_PINGS == 1 */
/*-----------------------------------------------------------*/

/**
 * @brief Send an event to the IP task. It calls 'xSendEventStructToIPTask' internally.
 *
 * @param[in] eEvent The event to be sent.
 *
 * @return pdPASS if the event was sent (or the desired effect was achieved). Else, pdFAIL.
 */
BaseType_t xSendEventToIPTask( eIPEvent_t eEvent )
{
    IPStackEvent_t xEventMessage;
    const TickType_t xDontBlock = ( TickType_t ) 0;

    xEventMessage.eEventType = eEvent;
    xEventMessage.pvData = ( void * ) NULL;

    return xSendEventStructToIPTask( &xEventMessage, xDontBlock );
}
/*-----------------------------------------------------------*/

/**
 * @brief Send an event (in form of struct) to the IP task to be processed.
 *
 * @param[in] pxEvent The event to be sent.
 * @param[in] uxTimeout Timeout for waiting in case the queue is full. 0 for non-blocking calls.
 *
 * @return pdPASS if the event was sent (or the desired effect was achieved). Else, pdFAIL.
 */
BaseType_t xSendEventStructToIPTask( const IPStackEvent_t * pxEvent,
                                     TickType_t uxTimeout )
{
    BaseType_t xReturn, xSendMessage;
    TickType_t uxUseTimeout = uxTimeout;

    if( ( xIPIsNetworkTaskReady() == pdFALSE ) && ( pxEvent->eEventType != eNetworkDownEvent ) )
    {
        /* Only allow eNetworkDownEvent events if the IP task is not ready
         * yet.  Not going to attempt to send the message so the send failed. */
        xReturn = pdFAIL;
    }
    else
    {
        xSendMessage = pdTRUE;

        #if ( ipconfigUSE_TCP == 1 )
        {
            if( pxEvent->eEventType == eTCPTimerEvent )
            {
                /* TCP timer events are sent to wake the timer task when
                 * xTCPTimer has expired, but there is no point sending them if the
                 * IP task is already awake processing other message. */
                vIPSetTCPTimerExpiredState( pdTRUE );

                if( uxQueueMessagesWaiting( xNetworkEventQueue ) != 0U )
                {
                    /* Not actually going to send the message but this is not a
                     * failure as the message didn't need to be sent. */
                    xSendMessage = pdFALSE;
                }
            }
        }
        #endif /* ipconfigUSE_TCP */

        if( xSendMessage != pdFALSE )
        {
            /* The IP task cannot block itself while waiting for itself to
             * respond. */
            if( ( xIsCallingFromIPTask() == pdTRUE ) && ( uxUseTimeout > ( TickType_t ) 0U ) )
            {
                uxUseTimeout = ( TickType_t ) 0;
            }

            xReturn = xQueueSendToBack( xNetworkEventQueue, pxEvent, uxUseTimeout );

            if( xReturn == pdFAIL )
            {
                /* A message should have been sent to the IP task, but wasn't. */
                FreeRTOS_debug_printf( ( "xSendEventStructToIPTask: CAN NOT ADD %d\n", pxEvent->eEventType ) );
                iptraceSTACK_TX_EVENT_LOST( pxEvent->eEventType );
            }
        }
        else
        {
            /* It was not necessary to send the message to process the event so
             * even though the message was not sent the call was successful. */
            xReturn = pdPASS;
        }
    }

    return xReturn;
}
/*-----------------------------------------------------------*/

/**
 * @brief Decide whether this packet should be processed or not based on the IP address in the packet.
 *
 * @param[in] pucEthernetBuffer The ethernet packet under consideration.
 *
 * @return Enum saying whether to release or to process the packet.
 */
eFrameProcessingResult_t eConsiderFrameForProcessing( const uint8_t * const pucEthernetBuffer )
{
    eFrameProcessingResult_t eReturn = eReleaseBuffer;

    do
    {
        const EthernetHeader_t * pxEthernetHeader = NULL;
        const NetworkEndPoint_t * pxEndPoint = NULL;
        uint16_t usFrameType;

        /* First, check the packet buffer is non-null. */
        if( pucEthernetBuffer == NULL )
        {
            /* The packet buffer was null - release it. */
            break;
        }

        /* Map the buffer onto Ethernet Header struct for easy access to fields. */
        /* MISRA Ref 11.3.1 [Misaligned access] */
        /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
        /* coverity[misra_c_2012_rule_11_3_violation] */
        pxEthernetHeader = ( ( const EthernetHeader_t * ) pucEthernetBuffer );
        usFrameType = pxEthernetHeader->usFrameType;

        /* Second, filter based on ethernet frame type. */
        /* The frame type field in the Ethernet header must have a value greater than 0x0600. */
        if( ipIS_ETHERNET_FRAME_TYPE_INVALID( FreeRTOS_ntohs( usFrameType ) ) )
        {
            /* The packet was not an Ethernet II frame */
            #if ipconfigIS_ENABLED( ipconfigFILTER_OUT_NON_ETHERNET_II_FRAMES )
                /* filtering is enabled - release it. */
                break;
            #else
                /* filtering is disabled - continue filter checks. */
            #endif
        }
        else if( usFrameType == ipARP_FRAME_TYPE )
        {
            /* The frame is an ARP type */
            #if ipconfigIS_DISABLED( ipconfigUSE_IPv4 )
                /* IPv4 is disabled - release it. */
                break;
            #else
                /*  IPv4 is enabled - Continue filter checks. */
            #endif
        }
        else if( usFrameType == ipIPv4_FRAME_TYPE )
        {
            /* The frame is an IPv4 type */
            #if ipconfigIS_DISABLED( ipconfigUSE_IPv4 )
                /* IPv4 is disabled - release it. */
                break;
            #else
                /* IPv4 is enabled - Continue filter checks. */
            #endif
        }
        else if( usFrameType == ipIPv6_FRAME_TYPE )
        {
            /* The frame is an IPv6 type */
            #if ipconfigIS_DISABLED( ipconfigUSE_IPv6 )
                /* IPv6 is disabled - release it. */
                break;
            #else
                /* IPv6 is enabled - Continue filter checks. */
            #endif
        }
        else
        {
            /* The frame is an unsupported Ethernet II type */
            #if ipconfigIS_ENABLED( ipconfigPROCESS_CUSTOM_ETHERNET_FRAMES )

                /* Processing custom Ethernet frames is enabled. No need for any further testing.
                 * Accept the frame whether it's a unicast, multicast, or broadcast. */
                eReturn = eProcessBuffer;
            #endif
            break;
        }

        /* Third, filter based on destination mac address. */
        pxEndPoint = FreeRTOS_FindEndPointOnMAC( &( pxEthernetHeader->xDestinationAddress ), NULL );

        if( pxEndPoint != NULL )
        {
            /* A destination endpoint was found - Continue filter checks. */
        }
        else if( memcmp( xBroadcastMACAddress.ucBytes, pxEthernetHeader->xDestinationAddress.ucBytes, sizeof( MACAddress_t ) ) == 0 )
        {
            /* The packet was a broadcast - Continue filter checks. */
        }
        else if( memcmp( xLLMNR_MacAddress.ucBytes, pxEthernetHeader->xDestinationAddress.ucBytes, sizeof( MACAddress_t ) ) == 0 )
        {
            /* The packet is a request for LLMNR using IPv4 */
            #if ( ipconfigIS_DISABLED( ipconfigUSE_DNS ) || ipconfigIS_DISABLED( ipconfigUSE_LLMNR ) || ipconfigIS_DISABLED( ipconfigUSE_IPv4 ) )
                /* DNS, LLMNR, or IPv4 is disabled - release it. */
                break;
            #else
                /* DNS, LLMNR, and IPv4 are enabled - Continue filter checks. */
            #endif
        }
        else if( memcmp( xLLMNR_MacAddressIPv6.ucBytes, pxEthernetHeader->xDestinationAddress.ucBytes, sizeof( MACAddress_t ) ) == 0 )
        {
            /* The packet is a request for LLMNR using IPv6 */
            #if ( ipconfigIS_DISABLED( ipconfigUSE_DNS ) || ipconfigIS_DISABLED( ipconfigUSE_LLMNR ) || ipconfigIS_DISABLED( ipconfigUSE_IPv6 ) )
                /* DNS, LLMNR, or IPv6 is disabled - release it. */
                break;
            #else
                /* DNS, LLMNR, and IPv6 are enabled - Continue filter checks. */
            #endif
        }
        else if( memcmp( xMDNS_MacAddress.ucBytes, pxEthernetHeader->xDestinationAddress.ucBytes, sizeof( MACAddress_t ) ) == 0 )
        {
            /* The packet is a request for MDNS using IPv4 */
            #if ( ipconfigIS_DISABLED( ipconfigUSE_DNS ) || ipconfigIS_DISABLED( ipconfigUSE_MDNS ) || ipconfigIS_DISABLED( ipconfigUSE_IPv4 ) )
                /* DNS, MDNS, or IPv4 is disabled - release it. */
                break;
            #else
                /* DNS, MDNS, and IPv4 are enabled - Continue filter checks. */
            #endif
        }
        else if( memcmp( xMDNS_MacAddressIPv6.ucBytes, pxEthernetHeader->xDestinationAddress.ucBytes, sizeof( MACAddress_t ) ) == 0 )
        {
            /* The packet is a request for MDNS using IPv6 */
            #if ( ipconfigIS_DISABLED( ipconfigUSE_DNS ) || ipconfigIS_DISABLED( ipconfigUSE_MDNS ) || ipconfigIS_DISABLED( ipconfigUSE_IPv6 ) )
                /* DNS, MDNS, or IPv6 is disabled - release it. */
                break;
            #else
                /* DNS, MDNS, and IPv6 are enabled - Continue filter checks. */
            #endif
        }
        else if( ( pxEthernetHeader->xDestinationAddress.ucBytes[ 0 ] == ipMULTICAST_MAC_ADDRESS_IPv4_0 ) &&
                 ( pxEthernetHeader->xDestinationAddress.ucBytes[ 1 ] == ipMULTICAST_MAC_ADDRESS_IPv4_1 ) &&
                 ( pxEthernetHeader->xDestinationAddress.ucBytes[ 2 ] == ipMULTICAST_MAC_ADDRESS_IPv4_2 ) &&
                 ( pxEthernetHeader->xDestinationAddress.ucBytes[ 3 ] <= 0x7fU ) )
        {
            /* The packet is an IPv4 Multicast */
            #if ipconfigIS_DISABLED( ipconfigUSE_IPv4 )
                /* IPv4 is disabled - release it. */
                break;
            #else
                /* IPv4 is enabled - Continue filter checks. */
            #endif
        }
        else if( ( pxEthernetHeader->xDestinationAddress.ucBytes[ 0 ] == ipMULTICAST_MAC_ADDRESS_IPv6_0 ) &&
                 ( pxEthernetHeader->xDestinationAddress.ucBytes[ 1 ] == ipMULTICAST_MAC_ADDRESS_IPv6_1 ) )
        {
            /* The packet is an IPv6 Multicast */
            #if ipconfigIS_DISABLED( ipconfigUSE_IPv6 )
                /* IPv6 is disabled - release it. */
                break;
            #else
                /* IPv6 is enabled - Continue filter checks. */
            #endif
        }
        else
        {
            /* The packet was not a broadcast, or for this node - release it */
            break;
        }

        /* All checks have been passed, process the packet. */
        eReturn = eProcessBuffer;
    } while( ipFALSE_BOOL );

    return eReturn;
}
/*-----------------------------------------------------------*/

/**
 * @brief Process the Ethernet packet.
 *
 * @param[in,out] pxNetworkBuffer the network buffer containing the ethernet packet. If the
 *                                 buffer is large enough, it may be reused to send a reply.
 */
static void prvProcessEthernetPacket( NetworkBufferDescriptor_t * const pxNetworkBuffer )
{
    const EthernetHeader_t * pxEthernetHeader;
    eFrameProcessingResult_t eReturned = eReleaseBuffer;

    /* Use do{}while(pdFALSE) to allow the use of break; */
    do
    {
        /* prvHandleEthernetPacket() already checked for ( pxNetworkBuffer != NULL ) so
         * it is safe to break out of the do{}while() and let the second half of this
         * function handle the releasing of pxNetworkBuffer */

        if( ( pxNetworkBuffer->pxInterface == NULL ) || ( pxNetworkBuffer->pxEndPoint == NULL ) )
        {
            break;
        }

        /* Beyond this point,
         * ( pxNetworkBuffer != NULL ),
         * ( pxNetworkBuffer->pxInterface != NULL ),
         * ( pxNetworkBuffer->pxEndPoint != NULL ),
         * Additionally, FreeRTOS_FillEndPoint() and FreeRTOS_FillEndPoint_IPv6() guarantee
         * that endpoints always have a valid interface assigned to them, and consequently:
         * ( pxNetworkBuffer->pxEndPoint->pxInterface != NULL )
         * None of the above need to be checked again in code that handles incoming packets. */

        iptraceNETWORK_INTERFACE_INPUT( pxNetworkBuffer->xDataLength, pxNetworkBuffer->pucEthernetBuffer );

        /* Interpret the Ethernet frame. */
        if( pxNetworkBuffer->xDataLength < sizeof( EthernetHeader_t ) )
        {
            break;
        }

        /* Map the buffer onto the Ethernet Header struct for easy access to the fields. */

        /* MISRA Ref 11.3.1 [Misaligned access] */
        /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
        /* coverity[misra_c_2012_rule_11_3_violation] */
        pxEthernetHeader = ( ( const EthernetHeader_t * ) pxNetworkBuffer->pucEthernetBuffer );

        /* The condition "eReturned == eProcessBuffer" must be true. */
        #if ( ipconfigETHERNET_DRIVER_FILTERS_FRAME_TYPES == 0 )
            if( eConsiderFrameForProcessing( pxNetworkBuffer->pucEthernetBuffer ) == eProcessBuffer )
        #endif
        {
            /* Interpret the received Ethernet packet. */
            switch( pxEthernetHeader->usFrameType )
            {
                #if ( ipconfigUSE_IPv4 != 0 )
                    case ipARP_FRAME_TYPE:

                        /* The Ethernet frame contains an ARP packet. */
                        if( pxNetworkBuffer->xDataLength >= sizeof( ARPPacket_t ) )
                        {
                            /* MISRA Ref 11.3.1 [Misaligned access] */
                            /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
                            /* coverity[misra_c_2012_rule_11_3_violation] */
                            eReturned = eARPProcessPacket( pxNetworkBuffer );
                        }
                        else
                        {
                            eReturned = eReleaseBuffer;
                        }
                        break;
                #endif /* ( ipconfigUSE_IPv4 != 0 ) */

                case ipIPv4_FRAME_TYPE:
                case ipIPv6_FRAME_TYPE:

                    /* The Ethernet frame contains an IP packet. */
                    if( pxNetworkBuffer->xDataLength >= sizeof( IPPacket_t ) )
                    {
                        /* MISRA Ref 11.3.1 [Misaligned access] */
                        /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
                        /* coverity[misra_c_2012_rule_11_3_violation] */
                        eReturned = prvProcessIPPacket( ( ( IPPacket_t * ) pxNetworkBuffer->pucEthernetBuffer ), pxNetworkBuffer );
                    }
                    else
                    {
                        eReturned = eReleaseBuffer;
                    }

                    break;

                default:
                    #if ( ipconfigPROCESS_CUSTOM_ETHERNET_FRAMES != 0 )
                        /* Custom frame handler. */
                        eReturned = eApplicationProcessCustomFrameHook( pxNetworkBuffer );
                    #else
                        /* No other packet types are handled.  Nothing to do. */
                        eReturned = eReleaseBuffer;
                    #endif
                    break;
            } /* switch( pxEthernetHeader->usFrameType ) */
        }
    } while( pdFALSE );

    /* Perform any actions that resulted from processing the Ethernet frame. */
    switch( eReturned )
    {
        case eReturnEthernetFrame:

            /* The Ethernet frame will have been updated (maybe it was
             * a resolution request or a PING request?) and should be sent back to
             * its source. */
            vReturnEthernetFrame( pxNetworkBuffer, pdTRUE );

            /* parameter pdTRUE: the buffer must be released once
             * the frame has been transmitted */
            break;

        case eFrameConsumed:

            /* The frame is in use somewhere, don't release the buffer
             * yet. */
            break;

        case eWaitingResolution:

            if( ( pxEthernetHeader->usFrameType == ipIPv4_FRAME_TYPE ) || ( pxEthernetHeader->usFrameType == ipARP_FRAME_TYPE ) )
            {
                #if ipconfigIS_ENABLED( ipconfigUSE_IPv4 )
                    if( pxARPWaitingNetworkBuffer == NULL )
                    {
                        pxARPWaitingNetworkBuffer = pxNetworkBuffer;
                        vIPTimerStartARPResolution( ipARP_RESOLUTION_MAX_DELAY );

                        iptraceDELAYED_ARP_REQUEST_STARTED();
                    }
                    else
                #endif /* if ipconfigIS_ENABLED( ipconfigUSE_IPv4 ) */
                {
                    /* We are already waiting on one resolution. This frame will be dropped. */
                    vReleaseNetworkBufferAndDescriptor( pxNetworkBuffer );

                    iptraceDELAYED_ARP_BUFFER_FULL();
                }

                break;
            }
            else if( pxEthernetHeader->usFrameType == ipIPv6_FRAME_TYPE )
            {
                #if ipconfigIS_ENABLED( ipconfigUSE_IPv6 )
                    if( pxNDWaitingNetworkBuffer == NULL )
                    {
                        pxNDWaitingNetworkBuffer = pxNetworkBuffer;
                        vIPTimerStartNDResolution( ipND_RESOLUTION_MAX_DELAY );

                        iptraceDELAYED_ND_REQUEST_STARTED();
                    }
                    else
                #endif /* if ipconfigIS_ENABLED( ipconfigUSE_IPv6 ) */
                {
                    /* We are already waiting on one resolution. This frame will be dropped. */
                    vReleaseNetworkBufferAndDescriptor( pxNetworkBuffer );

                    iptraceDELAYED_ND_BUFFER_FULL();
                }

                break;
            }
            else
            {
                /* Unknown frame type, drop the packet. */
                vReleaseNetworkBufferAndDescriptor( pxNetworkBuffer );
            }

            break;

        case eReleaseBuffer:
        case eProcessBuffer:
        default:

            /* The frame is not being used anywhere, and the
             * NetworkBufferDescriptor_t structure containing the frame should
             * just be released back to the list of free buffers. */
            vReleaseNetworkBufferAndDescriptor( pxNetworkBuffer );
            break;
    }
}
/*-----------------------------------------------------------*/

/**
 * @brief Check the sizes of the UDP packet and forward it to the UDP module
 *        ( xProcessReceivedUDPPacket() )
 * @param[in] pxNetworkBuffer The network buffer containing the UDP packet.
 * @return eReleaseBuffer ( please release the buffer ).
 *         eFrameConsumed ( the buffer has now been released ).
 */

static eFrameProcessingResult_t prvProcessUDPPacket( NetworkBufferDescriptor_t * const pxNetworkBuffer )
{
    eFrameProcessingResult_t eReturn = eReleaseBuffer;
    BaseType_t xIsWaitingResolution = pdFALSE;
    /* The IP packet contained a UDP frame. */
    /* MISRA Ref 11.3.1 [Misaligned access] */
    /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
    /* coverity[misra_c_2012_rule_11_3_violation] */
    const UDPPacket_t * pxUDPPacket = ( ( UDPPacket_t * ) pxNetworkBuffer->pucEthernetBuffer );
    const UDPHeader_t * pxUDPHeader = &( pxUDPPacket->xUDPHeader );

    size_t uxMinSize = ipSIZE_OF_ETH_HEADER + ( size_t ) uxIPHeaderSizePacket( pxNetworkBuffer ) + ipSIZE_OF_UDP_HEADER;
    size_t uxLength;
    uint16_t usLength;

    #if ( ipconfigUSE_IPv6 != 0 )
        if( pxUDPPacket->xEthernetHeader.usFrameType == ipIPv6_FRAME_TYPE )
        {
            const ProtocolHeaders_t * pxProtocolHeaders;

            /* MISRA Ref 11.3.1 [Misaligned access] */
            /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
            /* coverity[misra_c_2012_rule_11_3_violation] */
            pxProtocolHeaders = ( ( ProtocolHeaders_t * ) &( pxNetworkBuffer->pucEthernetBuffer[ ipSIZE_OF_ETH_HEADER + ipSIZE_OF_IPv6_HEADER ] ) );
            pxUDPHeader = &( pxProtocolHeaders->xUDPHeader );
        }
    #endif /* ( ipconfigUSE_IPv6 != 0 ) */

    usLength = FreeRTOS_ntohs( pxUDPHeader->usLength );
    uxLength = ( size_t ) usLength;

    /* Note the header values required prior to the checksum
     * generation as the checksum pseudo header may clobber some of
     * these values. */
    #if ( ipconfigUSE_IPv4 != 0 )
        if( ( pxUDPPacket->xEthernetHeader.usFrameType == ipIPv4_FRAME_TYPE ) &&
            ( usLength > ( FreeRTOS_ntohs( pxUDPPacket->xIPHeader.usLength ) - uxIPHeaderSizePacket( pxNetworkBuffer ) ) ) )
        {
            eReturn = eReleaseBuffer;
        }
        else
    #endif /* ( ipconfigUSE_IPv4 != 0 ) */

    if( ( pxNetworkBuffer->xDataLength >= uxMinSize ) &&
        ( uxLength >= sizeof( UDPHeader_t ) ) )
    {
        size_t uxPayloadSize_1, uxPayloadSize_2;

        /* Ensure that downstream UDP packet handling has the lesser
         * of: the actual network buffer Ethernet frame length, or
         * the sender's UDP packet header payload length, minus the
         * size of the UDP header.
         *
         * The size of the UDP packet structure in this implementation
         * includes the size of the Ethernet header, the size of
         * the IP header, and the size of the UDP header. */
        uxPayloadSize_1 = pxNetworkBuffer->xDataLength - uxMinSize;
        uxPayloadSize_2 = uxLength - ipSIZE_OF_UDP_HEADER;

        if( uxPayloadSize_1 > uxPayloadSize_2 )
        {
            pxNetworkBuffer->xDataLength = uxPayloadSize_2 + uxMinSize;
        }

        pxNetworkBuffer->usPort = pxUDPHeader->usSourcePort;
        pxNetworkBuffer->xIPAddress.ulIP_IPv4 = pxUDPPacket->xIPHeader.ulSourceIPAddress;

        /* ipconfigDRIVER_INCLUDED_RX_IP_CHECKSUM:
         * In some cases, the upper-layer checksum has been calculated
         * by the NIC driver. */

        /* Pass the packet payload to the UDP sockets
         * implementation. */
        if( xProcessReceivedUDPPacket( pxNetworkBuffer,
                                       pxUDPHeader->usDestinationPort,
                                       &( xIsWaitingResolution ) ) == pdPASS )
        {
            eReturn = eFrameConsumed;
        }
        else
        {
            /* Is this packet to be set aside for resolution. */
            if( xIsWaitingResolution == pdTRUE )
            {
                eReturn = eWaitingResolution;
            }
        }
    }
    else
    {
        /* Length checks failed, the buffer will be released. */
    }

    return eReturn;
}
/*-----------------------------------------------------------*/

/**
 * @brief Process an IP-packet.
 *
 * @param[in] pxIPPacket The IP packet to be processed.
 * @param[in] pxNetworkBuffer The networkbuffer descriptor having the IP packet.
 *
 * @return An enum to show whether the packet should be released/kept/processed etc.
 */
static eFrameProcessingResult_t prvProcessIPPacket( const IPPacket_t * pxIPPacket,
                                                    NetworkBufferDescriptor_t * const pxNetworkBuffer )
{
    eFrameProcessingResult_t eReturn;
    UBaseType_t uxHeaderLength = ipSIZE_OF_IPv4_HEADER;
    uint8_t ucProtocol = 0U;

    #if ( ipconfigUSE_IPv6 != 0 )
        const IPHeader_IPv6_t * pxIPHeader_IPv6 = NULL;
    #endif /* ( ipconfigUSE_IPv6 != 0 ) */

    #if ( ipconfigUSE_IPv4 != 0 )
        const IPHeader_t * pxIPHeader = &( pxIPPacket->xIPHeader );
    #endif /* ( ipconfigUSE_IPv4 != 0 ) */

    switch( pxIPPacket->xEthernetHeader.usFrameType )
    {
        #if ( ipconfigUSE_IPv6 != 0 )
            case ipIPv6_FRAME_TYPE:

                if( pxNetworkBuffer->xDataLength < sizeof( IPPacket_IPv6_t ) )
                {
                    /* The packet size is less than minimum IPv6 packet. */
                    eReturn = eReleaseBuffer;
                }
                else
                {
                    /* MISRA Ref 11.3.1 [Misaligned access] */
                    /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
                    /* coverity[misra_c_2012_rule_11_3_violation] */
                    pxIPHeader_IPv6 = ( ( const IPHeader_IPv6_t * ) &( pxNetworkBuffer->pucEthernetBuffer[ ipSIZE_OF_ETH_HEADER ] ) );

                    uxHeaderLength = ipSIZE_OF_IPv6_HEADER;
                    ucProtocol = pxIPHeader_IPv6->ucNextHeader;
                    /* MISRA Ref 11.3.1 [Misaligned access] */
                    /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
                    /* coverity[misra_c_2012_rule_11_3_violation] */
                    eReturn = prvAllowIPPacketIPv6( ( ( const IPHeader_IPv6_t * ) &( pxIPPacket->xIPHeader ) ), pxNetworkBuffer, uxHeaderLength );

                    /* The IP-header type is copied to a special reserved location a few bytes before the message
                     * starts. In the case of IPv6, this value is never actually used and the line below can safely be removed
                     * with no ill effects. We only store it to help with debugging. */
                    pxNetworkBuffer->pucEthernetBuffer[ 0 - ( BaseType_t ) ipIP_TYPE_OFFSET ] = pxIPHeader_IPv6->ucVersionTrafficClass;
                }
                break;
        #endif /* ( ipconfigUSE_IPv6 != 0 ) */

        #if ( ipconfigUSE_IPv4 != 0 )
            case ipIPv4_FRAME_TYPE:
               {
                   size_t uxLength = ( size_t ) pxIPHeader->ucVersionHeaderLength;

                   /* Check if the IP headers are acceptable and if it has our destination.
                    * The lowest four bits of 'ucVersionHeaderLength' indicate the IP-header
                    * length in multiples of 4. */
                   uxHeaderLength = ( size_t ) ( ( uxLength & 0x0FU ) << 2 );

                   if( ( uxHeaderLength > ( pxNetworkBuffer->xDataLength - ipSIZE_OF_ETH_HEADER ) ) ||
                       ( uxHeaderLength < ipSIZE_OF_IPv4_HEADER ) )
                   {
                       eReturn = eReleaseBuffer;
                   }
                   else
                   {
                       ucProtocol = pxIPPacket->xIPHeader.ucProtocol;
                       /* Check if the IP headers are acceptable and if it has our destination. */
                       eReturn = prvAllowIPPacketIPv4( pxIPPacket, pxNetworkBuffer, uxHeaderLength );

                       {
                           /* The IP-header type is copied to a special reserved location a few bytes before the
                            * messages starts.  It might be needed later on when a UDP-payload
                            * buffer is being used. */
                           pxNetworkBuffer->pucEthernetBuffer[ 0 - ( BaseType_t ) ipIP_TYPE_OFFSET ] = pxIPHeader->ucVersionHeaderLength;
                       }
                   }

                   break;
               }
        #endif /* ( ipconfigUSE_IPv4 != 0 ) */

        default:
            eReturn = eReleaseBuffer;
            FreeRTOS_debug_printf( ( "prvProcessIPPacket: Undefined Frame Type \n" ) );
            /* MISRA 16.4 Compliance */
            break;
    }

    /* MISRA Ref 14.3.1 [Configuration dependent invariant] */
    /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-143 */
    /* coverity[misra_c_2012_rule_14_3_violation] */
    /* coverity[cond_const] */
    if( eReturn == eProcessBuffer )
    {
        /* Are there IP-options. */
        /* Case default is never toggled because eReturn is not eProcessBuffer in previous step. */
        switch( pxIPPacket->xEthernetHeader.usFrameType ) /* LCOV_EXCL_BR_LINE */
        {
            #if ( ipconfigUSE_IPv4 != 0 )
                case ipIPv4_FRAME_TYPE:

                    if( uxHeaderLength > ipSIZE_OF_IPv4_HEADER )
                    {
                        /* The size of the IP-header is larger than 20 bytes.
                         * The extra space is used for IP-options. */
                        eReturn = prvCheckIP4HeaderOptions( pxNetworkBuffer );
                    }
                    break;
            #endif /* ( ipconfigUSE_IPv4 != 0 ) */

            #if ( ipconfigUSE_IPv6 != 0 )
                case ipIPv6_FRAME_TYPE:

                    if( xGetExtensionOrder( ucProtocol, 0U ) > 0 )
                    {
                        eReturn = eHandleIPv6ExtensionHeaders( pxNetworkBuffer, pdTRUE );

                        if( eReturn != eReleaseBuffer )
                        {
                            /* Ignore warning for `pxIPHeader_IPv6`. */
                            ucProtocol = pxIPHeader_IPv6->ucNextHeader;
                        }
                    }
                    break;
            #endif /* ( ipconfigUSE_IPv6 != 0 ) */

            /* Case default is never toggled because eReturn is not eProcessBuffer in previous step. */
            default:   /* LCOV_EXCL_LINE */
                /* MISRA 16.4 Compliance */
                break; /* LCOV_EXCL_LINE */
        }

        /* MISRA Ref 14.3.1 [Configuration dependent invariant] */
        /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-143 */
        /* coverity[misra_c_2012_rule_14_3_violation] */
        /* coverity[const] */
        if( eReturn != eReleaseBuffer )
        {
            /* Add the IP and MAC addresses to the cache if they are not
             * already there - otherwise refresh the age of the existing
             * entry. */
            if( ucProtocol != ( uint8_t ) ipPROTOCOL_UDP )
            {
                if( xCheckRequiresResolution( pxNetworkBuffer ) == pdTRUE )
                {
                    eReturn = eWaitingResolution;
                }
                else
                {
                    /* Refresh the cache with the IP/MAC-address of the received
                     * packet.  For UDP packets, this will be done later in
                     * xProcessReceivedUDPPacket(), as soon as it's know that the message
                     * will be handled.  This will prevent the cache getting
                     * overwritten with the IP address of useless broadcast packets. */
                    /* Case default is never toggled because eReturn is not eProcessBuffer in previous step. */
                    switch( pxIPPacket->xEthernetHeader.usFrameType ) /* LCOV_EXCL_BR_LINE */
                    {
                        #if ( ipconfigUSE_IPv6 != 0 )
                            case ipIPv6_FRAME_TYPE:
                                vNDRefreshCacheEntry( &( pxIPPacket->xEthernetHeader.xSourceAddress ), &( pxIPHeader_IPv6->xSourceAddress ), pxNetworkBuffer->pxEndPoint );
                                break;
                        #endif /* ( ipconfigUSE_IPv6 != 0 ) */

                        #if ( ipconfigUSE_IPv4 != 0 )
                            case ipIPv4_FRAME_TYPE:
                                /* Refresh the age of this cache entry since a packet was received. */
                                vARPRefreshCacheEntryAge( &( pxIPPacket->xEthernetHeader.xSourceAddress ), pxIPHeader->ulSourceIPAddress );
                                break;
                        #endif /* ( ipconfigUSE_IPv4 != 0 ) */

                        /* Case default is never toggled because eReturn is not eProcessBuffer in previous step. */
                        default:   /* LCOV_EXCL_LINE */
                            /* MISRA 16.4 Compliance */
                            break; /* LCOV_EXCL_LINE */
                    }
                }
            }

            if( eReturn != eWaitingResolution )
            {
                switch( ucProtocol )
                {
                    #if ( ipconfigUSE_IPv4 != 0 )
                        case ipPROTOCOL_ICMP:

                            /* The IP packet contained an ICMP frame.  Don't bother checking
                             * the ICMP checksum, as if it is wrong then the wrong data will
                             * also be returned, and the source of the ping will know something
                             * went wrong because it will not be able to validate what it
                             * receives. */
                            #if ( ipconfigREPLY_TO_INCOMING_PINGS == 1 ) || ( ipconfigSUPPORT_OUTGOING_PINGS == 1 )
                            {
                                eReturn = ProcessICMPPacket( pxNetworkBuffer );
                            }
                            #endif /* ( ipconfigREPLY_TO_INCOMING_PINGS == 1 ) || ( ipconfigSUPPORT_OUTGOING_PINGS == 1 ) */
                            break;
                    #endif /* ( ipconfigUSE_IPv4 != 0 ) */

                    #if ( ipconfigUSE_IPv6 != 0 )
                        case ipPROTOCOL_ICMP_IPv6:
                            eReturn = prvProcessICMPMessage_IPv6( pxNetworkBuffer );
                            break;
                    #endif /* ( ipconfigUSE_IPv6 != 0 ) */

                    case ipPROTOCOL_UDP:
                        /* The IP packet contained a UDP frame. */

                        eReturn = prvProcessUDPPacket( pxNetworkBuffer );
                        break;

                        #if ipconfigUSE_TCP == 1
                            case ipPROTOCOL_TCP:

                                if( xProcessReceivedTCPPacket( pxNetworkBuffer ) == pdPASS )
                                {
                                    eReturn = eFrameConsumed;
                                }
                                break;
                        #endif /* if ipconfigUSE_TCP == 1 */
                    default:
                        /* Not a supported frame type. */
                        eReturn = eReleaseBuffer;
                        break;
                }
            }
        }
    }

    return eReturn;
}

/*-----------------------------------------------------------*/

/* This function is used in other files, has external linkage e.g. in
 * FreeRTOS_DNS.c. Not to be made static. */

/**
 * @brief Send the Ethernet frame after checking for some conditions.
 *
 * @param[in,out] pxNetworkBuffer The network buffer which is to be sent.
 * @param[in] xReleaseAfterSend Whether this network buffer is to be released or not.
 */
void vReturnEthernetFrame( NetworkBufferDescriptor_t * pxNetworkBuffer,
                           BaseType_t xReleaseAfterSend )
{
    #if ( ipconfigZERO_COPY_TX_DRIVER != 0 )
        NetworkBufferDescriptor_t * pxNewBuffer;
    #endif

    #if ( ipconfigETHERNET_MINIMUM_PACKET_BYTES > 0 )
    {
        if( pxNetworkBuffer->xDataLength < ( size_t ) ipconfigETHERNET_MINIMUM_PACKET_BYTES )
        {
            BaseType_t xIndex;

            FreeRTOS_printf( ( "vReturnEthernetFrame: length %u\n", ( unsigned ) pxNetworkBuffer->xDataLength ) );

            for( xIndex = ( BaseType_t ) pxNetworkBuffer->xDataLength; xIndex < ( BaseType_t ) ipconfigETHERNET_MINIMUM_PACKET_BYTES; xIndex++ )
            {
                pxNetworkBuffer->pucEthernetBuffer[ xIndex ] = 0U;
            }

            pxNetworkBuffer->xDataLength = ( size_t ) ipconfigETHERNET_MINIMUM_PACKET_BYTES;
        }
    }
    #endif /* if( ipconfigETHERNET_MINIMUM_PACKET_BYTES > 0 ) */

    #if ( ipconfigZERO_COPY_TX_DRIVER != 0 )
        if( xReleaseAfterSend == pdFALSE )
        {
            pxNewBuffer = pxDuplicateNetworkBufferWithDescriptor( pxNetworkBuffer, pxNetworkBuffer->xDataLength );

            if( pxNewBuffer != NULL )
            {
                xReleaseAfterSend = pdTRUE;
                /* Want no rounding up. */
                pxNewBuffer->xDataLength = pxNetworkBuffer->xDataLength;
            }

            pxNetworkBuffer = pxNewBuffer;
        }

        if( pxNetworkBuffer != NULL )
    #endif /* if ( ipconfigZERO_COPY_TX_DRIVER != 0 ) */
    {
        /* MISRA Ref 11.3.1 [Misaligned access] */
        /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
        /* coverity[misra_c_2012_rule_11_3_violation] */
        IPPacket_t * pxIPPacket = ( ( IPPacket_t * ) pxNetworkBuffer->pucEthernetBuffer );
        /* memcpy() helper variables for MISRA Rule 21.15 compliance*/
        const void * pvCopySource = NULL;
        void * pvCopyDest;

        #if ( ipconfigUSE_IPv4 != 0 )
            MACAddress_t xMACAddress;
            eResolutionLookupResult_t eResult;
            uint32_t ulDestinationIPAddress = 0U;
        #endif /* ( ipconfigUSE_IPv4 != 0 ) */

        /* Send! */
        if( pxNetworkBuffer->pxEndPoint == NULL )
        {
            /* _HT_ I wonder if this ad-hoc search of an end-point it necessary. */
            FreeRTOS_printf( ( "vReturnEthernetFrame: No pxEndPoint yet for %x ip?\n", ( unsigned int ) FreeRTOS_ntohl( pxIPPacket->xIPHeader.ulDestinationIPAddress ) ) );

            /* MISRA Ref 11.3.1 [Misaligned access] */
            /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
            /* coverity[misra_c_2012_rule_11_3_violation] */
            switch( ( ( ( EthernetHeader_t * ) pxNetworkBuffer->pucEthernetBuffer ) )->usFrameType )
            {
                #if ( ipconfigUSE_IPv6 != 0 )
                    case ipIPv6_FRAME_TYPE:
                        /* No IPv6 endpoint found */
                        break;
                #endif /* ( ipconfigUSE_IPv6 != 0 ) */

                #if ( ipconfigUSE_IPv4 != 0 )
                    case ipIPv4_FRAME_TYPE:
                        pxNetworkBuffer->pxEndPoint = FreeRTOS_FindEndPointOnNetMask( pxIPPacket->xIPHeader.ulDestinationIPAddress );
                        break;
                #endif /* ( ipconfigUSE_IPv4 != 0 ) */

                default:
                    /* MISRA 16.4 Compliance */
                    break;
            }
        }

        if( pxNetworkBuffer->pxEndPoint != NULL )
        {
            NetworkInterface_t * pxInterface = pxNetworkBuffer->pxEndPoint->pxNetworkInterface; /*_RB_ Why not use the pxNetworkBuffer->pxNetworkInterface directly? */

            /* Interpret the Ethernet packet being sent. */
            switch( pxIPPacket->xEthernetHeader.usFrameType )
            {
                #if ( ipconfigUSE_IPv4 != 0 )
                    case ipIPv4_FRAME_TYPE:
                        ulDestinationIPAddress = pxIPPacket->xIPHeader.ulDestinationIPAddress;

                        /* Try to find a MAC address corresponding to the destination IP
                         * address. */
                        eResult = eARPGetCacheEntry( &ulDestinationIPAddress, &xMACAddress, &( pxNetworkBuffer->pxEndPoint ) );

                        if( eResult == eResolutionCacheHit )
                        {
                            /* Best case scenario - an address is found, use it. */
                            pvCopySource = &xMACAddress;
                        }
                        else
                        {
                            /* If an address is not found, just swap the source and destination MAC addresses. */
                            pvCopySource = &( pxIPPacket->xEthernetHeader.xSourceAddress );
                        }
                        break;
                #endif /* ( ipconfigUSE_IPv4 != 0 ) */

                case ipIPv6_FRAME_TYPE:
                case ipARP_FRAME_TYPE:
                default:
                    /* Just swap the source and destination MAC addresses. */
                    pvCopySource = &( pxIPPacket->xEthernetHeader.xSourceAddress );
                    break;
            }

            /*
             * Use helper variables for memcpy() to remain
             * compliant with MISRA Rule 21.15.  These should be
             * optimized away.
             */
            pvCopyDest = &( pxIPPacket->xEthernetHeader.xDestinationAddress );
            ( void ) memcpy( pvCopyDest, pvCopySource, sizeof( pxIPPacket->xEthernetHeader.xDestinationAddress ) );

            pvCopySource = pxNetworkBuffer->pxEndPoint->xMACAddress.ucBytes;
            pvCopyDest = &( pxIPPacket->xEthernetHeader.xSourceAddress );
            ( void ) memcpy( pvCopyDest, pvCopySource, ( size_t ) ipMAC_ADDRESS_LENGTH_BYTES );

            /* Send! */
            if( xIsCallingFromIPTask() == pdTRUE )
            {
                iptraceNETWORK_INTERFACE_OUTPUT( pxNetworkBuffer->xDataLength, pxNetworkBuffer->pucEthernetBuffer );
                ( void ) pxInterface->pfOutput( pxInterface, pxNetworkBuffer, xReleaseAfterSend );
            }
            else if( xReleaseAfterSend != pdFALSE )
            {
                IPStackEvent_t xSendEvent;

                /* Send a message to the IP-task to send this packet. */
                xSendEvent.eEventType = eNetworkTxEvent;
                xSendEvent.pvData = pxNetworkBuffer;

                if( xSendEventStructToIPTask( &xSendEvent, ( TickType_t ) portMAX_DELAY ) == pdFAIL )
                {
                    /* Failed to send the message, so release the network buffer. */
                    vReleaseNetworkBufferAndDescriptor( pxNetworkBuffer );
                }
            }
            else
            {
                /* This should never reach or the packet is gone. */
                configASSERT( pdFALSE );
            }
        }
    }
}
/*-----------------------------------------------------------*/

#if ( ipconfigUSE_IPv4 != 0 )

/**
 * @brief Returns the IP address of the NIC.
 *
 * @return The IP address of the NIC.
 */
    uint32_t FreeRTOS_GetIPAddress( void )
    {
        NetworkEndPoint_t * pxEndPoint;
        uint32_t ulIPAddress;

        pxEndPoint = FreeRTOS_FirstEndPoint( NULL );

        #if ( ipconfigUSE_IPv6 != 0 )
            if( ENDPOINT_IS_IPv6( pxEndPoint ) )
            {
                for( ;
                     pxEndPoint != NULL;
                     pxEndPoint = FreeRTOS_NextEndPoint( NULL, pxEndPoint ) )
                {
                    /* Break if the endpoint is IPv4. */
                    if( pxEndPoint->bits.bIPv6 == 0U )
                    {
                        break;
                    }
                }
            }
        #endif /* ( ipconfigUSE_IPv6 != 0 ) */

        /* Returns the IP address of the NIC. */
        if( pxEndPoint == NULL )
        {
            ulIPAddress = 0U;
        }
        else if( pxEndPoint->ipv4_settings.ulIPAddress != 0U )
        {
            ulIPAddress = pxEndPoint->ipv4_settings.ulIPAddress;
        }
        else
        {
            ulIPAddress = pxEndPoint->ipv4_defaults.ulIPAddress;
        }

        return ulIPAddress;
    }
/*-----------------------------------------------------------*/

#endif /* #if ( ipconfigUSE_IPv4 != 0 ) */

#if ( ipconfigIPv4_BACKWARD_COMPATIBLE == 1 ) && ( ipconfigUSE_IPv4 != 0 )

/*
 * The helper functions here below assume that there is a single
 * interface and a single end-point (ipconfigIPv4_BACKWARD_COMPATIBLE)
 */

/**
 * @brief Sets the IP address of the NIC.
 *
 * @param[in] ulIPAddress IP address of the NIC to be set.
 */
    void FreeRTOS_SetIPAddress( uint32_t ulIPAddress )
    {
        /* Sets the IP address of the NIC. */
        NetworkEndPoint_t * pxEndPoint = FreeRTOS_FirstEndPoint( NULL );

        if( pxEndPoint != NULL )
        {
            pxEndPoint->ipv4_settings.ulIPAddress = ulIPAddress;
        }
    }
/*-----------------------------------------------------------*/

/**
 * @brief Get the gateway address of the subnet.
 *
 * @return The IP-address of the gateway, zero if a gateway is
 *         not used/defined.
 */
    uint32_t FreeRTOS_GetGatewayAddress( void )
    {
        uint32_t ulIPAddress = 0U;
        NetworkEndPoint_t * pxEndPoint = FreeRTOS_FirstEndPoint( NULL );

        if( pxEndPoint != NULL )
        {
            ulIPAddress = pxEndPoint->ipv4_settings.ulGatewayAddress;
        }

        return ulIPAddress;
    }
/*-----------------------------------------------------------*/

/**
 * @brief Get the DNS server address.
 *
 * @return The IP address of the DNS server.
 */
    uint32_t FreeRTOS_GetDNSServerAddress( void )
    {
        uint32_t ulIPAddress = 0U;
        NetworkEndPoint_t * pxEndPoint = FreeRTOS_FirstEndPoint( NULL );

        if( pxEndPoint != NULL )
        {
            ulIPAddress = pxEndPoint->ipv4_settings.ulDNSServerAddresses[ 0 ];
        }

        return ulIPAddress;
    }
/*-----------------------------------------------------------*/

/**
 * @brief Get the netmask for the subnet.
 *
 * @return The 32 bit netmask for the subnet.
 */
    uint32_t FreeRTOS_GetNetmask( void )
    {
        uint32_t ulIPAddress = 0U;
        NetworkEndPoint_t * pxEndPoint = FreeRTOS_FirstEndPoint( NULL );

        if( pxEndPoint != NULL )
        {
            ulIPAddress = pxEndPoint->ipv4_settings.ulNetMask;
        }

        return ulIPAddress;
    }
/*-----------------------------------------------------------*/

/**
 * @brief Update the MAC address.
 *
 * @param[in] ucMACAddress the MAC address to be set.
 */
    void FreeRTOS_UpdateMACAddress( const uint8_t ucMACAddress[ ipMAC_ADDRESS_LENGTH_BYTES ] )
    {
        NetworkEndPoint_t * pxEndPoint = FreeRTOS_FirstEndPoint( NULL );

        if( pxEndPoint != NULL )
        {
            /* Copy the MAC address at the start of the default packet header fragment. */
            ( void ) memcpy( pxEndPoint->xMACAddress.ucBytes, ( const void * ) ucMACAddress, ( size_t ) ipMAC_ADDRESS_LENGTH_BYTES );
        }
    }
/*-----------------------------------------------------------*/

/**
 * @brief Get the MAC address.
 *
 * @return The pointer to MAC address.
 */
    const uint8_t * FreeRTOS_GetMACAddress( void )
    {
        const uint8_t * pucReturn = NULL;
        NetworkEndPoint_t * pxEndPoint = FreeRTOS_FirstEndPoint( NULL );

        if( pxEndPoint != NULL )
        {
            /* Copy the MAC address at the start of the default packet header fragment. */
            pucReturn = pxEndPoint->xMACAddress.ucBytes;
        }

        return pucReturn;
    }
/*-----------------------------------------------------------*/

/**
 * @brief Set the netmask for the subnet.
 *
 * @param[in] ulNetmask The 32 bit netmask of the subnet.
 */
    void FreeRTOS_SetNetmask( uint32_t ulNetmask )
    {
        NetworkEndPoint_t * pxEndPoint = FreeRTOS_FirstEndPoint( NULL );

        if( pxEndPoint != NULL )
        {
            pxEndPoint->ipv4_settings.ulNetMask = ulNetmask;
        }
    }
/*-----------------------------------------------------------*/

/**
 * @brief Set the gateway address.
 *
 * @param[in] ulGatewayAddress The gateway address.
 */
    void FreeRTOS_SetGatewayAddress( uint32_t ulGatewayAddress )
    {
        NetworkEndPoint_t * pxEndPoint = FreeRTOS_FirstEndPoint( NULL );

        if( pxEndPoint != NULL )
        {
            pxEndPoint->ipv4_settings.ulGatewayAddress = ulGatewayAddress;
        }
    }
/*-----------------------------------------------------------*/
#endif /* if ( ipconfigIPv4_BACKWARD_COMPATIBLE == 1 )  && ( ipconfigUSE_IPv4 != 0 ) */

/**
 * @brief Returns whether the IP task is ready.
 *
 * @return pdTRUE if IP task is ready, else pdFALSE.
 */
BaseType_t xIPIsNetworkTaskReady( void )
{
    return xIPTaskInitialised;
}
/*-----------------------------------------------------------*/

/**
 * @brief Returns whether all end-points are up.
 *
 * @return pdTRUE if all defined end-points are up.
 */
BaseType_t FreeRTOS_IsNetworkUp( void )
{
    /* IsNetworkUp() is kept for backward compatibility. */
    return FreeRTOS_IsEndPointUp( NULL );
}
/*-----------------------------------------------------------*/

/**
 * @brief The variable 'xNetworkDownEventPending' is declared static.  This function
 *        gives read-only access to it.
 *
 * @return pdTRUE if there a network-down event pending. pdFALSE otherwise.
 */
BaseType_t xIsNetworkDownEventPending( void )
{
    return xNetworkDownEventPending;
}
/*-----------------------------------------------------------*/

/**
 * @brief Returns whether a particular end-point is up.
 *
 * @return pdTRUE if a particular end-points is up.
 */
BaseType_t FreeRTOS_IsEndPointUp( const struct xNetworkEndPoint * pxEndPoint )
{
    BaseType_t xReturn;

    if( pxEndPoint != NULL )
    {
        /* Is this particular end-point up? */
        xReturn = ( BaseType_t ) pxEndPoint->bits.bEndPointUp;
    }
    else
    {
        /* Are all end-points up? */
        xReturn = FreeRTOS_AllEndPointsUp( NULL );
    }

    return xReturn;
}
/*-----------------------------------------------------------*/

/**
 * @brief Return pdTRUE if all end-points belonging to a given interface are up.  When
 *        pxInterface is null, all end-points will be checked.
 *
 * @param[in] pxInterface The network interface of interest, or NULL to check all end-points.
 *
 * @return pdTRUE if all end-points are up, otherwise pdFALSE;
 */
BaseType_t FreeRTOS_AllEndPointsUp( const struct xNetworkInterface * pxInterface )
{
    BaseType_t xResult = pdTRUE;
    const NetworkEndPoint_t * pxEndPoint = pxNetworkEndPoints;

    while( pxEndPoint != NULL )
    {
        if( ( pxInterface == NULL ) ||
            ( pxEndPoint->pxNetworkInterface == pxInterface ) )

        {
            if( pxEndPoint->bits.bEndPointUp == pdFALSE_UNSIGNED )
            {
                xResult = pdFALSE;
                break;
            }
        }

        pxEndPoint = pxEndPoint->pxNext;
    }

    return xResult;
}
/*-----------------------------------------------------------*/

#if ( ipconfigCHECK_IP_QUEUE_SPACE != 0 )

/**
 * @brief Get the minimum space in the IP task queue.
 *
 * @return The minimum possible space in the IP task queue.
 */
    UBaseType_t uxGetMinimumIPQueueSpace( void )
    {
        return uxQueueMinimumSpace;
    }
#endif
/*-----------------------------------------------------------*/

/**
 * @brief Get the size of the IP-header, by checking the type of the network buffer.
 * @param[in] pxNetworkBuffer The network buffer.
 * @return The size of the corresponding IP-header.
 */
size_t uxIPHeaderSizePacket( const NetworkBufferDescriptor_t * pxNetworkBuffer )
{
    size_t uxResult;
    /* Map the buffer onto Ethernet Header struct for easy access to fields. */
    /* MISRA Ref 11.3.1 [Misaligned access] */
    /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
    /* coverity[misra_c_2012_rule_11_3_violation] */
    const EthernetHeader_t * pxHeader = ( ( const EthernetHeader_t * ) pxNetworkBuffer->pucEthernetBuffer );

    if( pxHeader->usFrameType == ( uint16_t ) ipIPv6_FRAME_TYPE )
    {
        uxResult = ipSIZE_OF_IPv6_HEADER;
    }
    else
    {
        uxResult = ipSIZE_OF_IPv4_HEADER;
    }

    return uxResult;
}
/*-----------------------------------------------------------*/

/**
 * @brief Get the size of the IP-header, by checking if the socket bIsIPv6 set.
 * @param[in] pxSocket The socket.
 * @return The size of the corresponding IP-header.
 */
size_t uxIPHeaderSizeSocket( const FreeRTOS_Socket_t * pxSocket )
{
    size_t uxResult;

    if( ( pxSocket != NULL ) && ( pxSocket->bits.bIsIPv6 != pdFALSE_UNSIGNED ) )
    {
        uxResult = ipSIZE_OF_IPv6_HEADER;
    }
    else
    {
        uxResult = ipSIZE_OF_IPv4_HEADER;
    }

    return uxResult;
}
/*-----------------------------------------------------------*/

/* Provide access to private members for verification. */
#ifdef FREERTOS_TCP_ENABLE_VERIFICATION
    #include "aws_freertos_ip_verification_access_ip_define.h"
#endif
