/*
 * FreeRTOS+TCP V4.3.3
 * Copyright (C) 2022 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
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
 * @file FreeRTOS_DHCPv6.c
 * @brief A DHCPv6 client.
 */

/* Standard includes. */
#include <stdio.h>
#include <ctype.h>

/* FreeRTOS includes. */
#include <FreeRTOS.h>
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "semphr.h"

/* FreeRTOS+TCP includes. */
#include "FreeRTOS_IP.h"

/* *INDENT-OFF* */
    #if ( ipconfigUSE_IPv6 != 0 ) && ( ipconfigUSE_DHCPv6 != 0 )
/* *INDENT-ON* */

#include "FreeRTOS_Sockets.h"
#include "FreeRTOS_DHCPv6.h"
#include "FreeRTOS_DNS.h"
#include "NetworkBufferManagement.h"
#include "FreeRTOS_Sockets.h"
#include "FreeRTOS_IP_Private.h"
#include "FreeRTOS_IP_Timers.h"
#include "FreeRTOS_BitConfig.h"
#include "FreeRTOS_Routing.h"

/* Timer parameters */
#ifndef dhcpINITIAL_DHCP_TX_PERIOD
    /** @brief DHCP timer period in ms */
    #define dhcpINITIAL_TIMER_PERIOD      ( pdMS_TO_TICKS( 250U ) )
    /** @brief DHCP transmit period in ms */
    #define dhcpINITIAL_DHCP_TX_PERIOD    ( pdMS_TO_TICKS( 5000U ) )
#endif

/** @brief The following define is temporary and serves to make the /single source
 * code more similar to the /multi version. */

#define EP_DHCPData                    pxEndPoint->xDHCPData
/** @brief Macro to access the IPv6 settings from the pxEndPoint */
#define EP_IPv6_SETTINGS               pxEndPoint->ipv6_settings

/** @brief The maximum size of send buffer. */
#define DHCPv6_SEND_MAX_BUFFER_SIZE    ( 256 )

/** @brief When a reply is received, some options are mandatory for this driver. */
#define dhcpMANDATORY_OPTIONS                                      \
    ( ( ( ( uint32_t ) 1U ) << DHCPv6_Option_Client_Identifier ) | \
      ( ( ( uint32_t ) 1U ) << DHCPv6_Option_Server_Identifier ) )

/** @brief The UDP socket which is shared by all end-points that need DHCPv6. */
_static Socket_t xDHCPv6Socket;

/** @brief A reference count makes sure that the UDP socket will be deleted when it
 * is not used anymore. */
_static BaseType_t xDHCPv6SocketUserCount;

static BaseType_t prvIsOptionLengthValid( uint16_t usOption,
                                          size_t uxOptionLength,
                                          size_t uxRemainingSize );

static BaseType_t prvDHCPv6Analyse( struct xNetworkEndPoint * pxEndPoint,
                                    const uint8_t * pucAnswer,
                                    size_t uxTotalLength,
                                    DHCPMessage_IPv6_t * pxDHCPMessage );

static void vDHCPv6ProcessEndPoint( BaseType_t xReset,
                                    NetworkEndPoint_t * pxEndPoint,
                                    DHCPMessage_IPv6_t * pxDHCPMessage );

static void prvInitialiseDHCPv6( NetworkEndPoint_t * pxEndPoint );

static void prvSendDHCPMessage( NetworkEndPoint_t * pxEndPoint );

/*
 * Create the DHCP socket, if it has not been created already.
 */
static void prvCreateDHCPv6Socket( NetworkEndPoint_t * pxEndPoint );

/*
 * Close the DHCP socket, only when not in use anymore (i.e. xDHCPv6SocketUserCount = 0).
 */
static void prvCloseDHCPv6Socket( NetworkEndPoint_t * pxEndPoint );

#if ( ipconfigHAS_DEBUG_PRINTF == 1 )
    static const char * prvStateName( eDHCPState_t eState );
#endif

static BaseType_t xDHCPv6Process_PassReplyToEndPoint( struct xNetworkEndPoint * pxEndPoint );

static void vDHCPv6ProcessEndPoint_HandleReply( NetworkEndPoint_t * pxEndPoint,
                                                DHCPMessage_IPv6_t * pxDHCPMessage );


static BaseType_t xDHCPv6ProcessEndPoint_HandleAdvertise( NetworkEndPoint_t * pxEndPoint,
                                                          DHCPMessage_IPv6_t * pxDHCPMessage );

static BaseType_t xDHCPv6ProcessEndPoint_HandleState( NetworkEndPoint_t * pxEndPoint,
                                                      DHCPMessage_IPv6_t * pxDHCPMessage );

static BaseType_t prvDHCPv6_subOption( uint16_t usOption,
                                       const DHCPOptionSet_t * pxSet,
                                       DHCPMessage_IPv6_t * pxDHCPMessage,
                                       BitConfig_t * pxMessage );

static BaseType_t prvDHCPv6_handleOption( struct xNetworkEndPoint * pxEndPoint,
                                          uint16_t usOption,
                                          const DHCPOptionSet_t * pxSet,
                                          DHCPMessage_IPv6_t * pxDHCPMessage,
                                          BitConfig_t * pxMessage );


/*-----------------------------------------------------------*/

/**
 * @brief DHCP IPv6 message object
 */
_static DHCPMessage_IPv6_t xDHCPMessage;

/**
 * @brief Get the DHCP state from a given endpoint.
 *
 * @param[in] pxEndPoint The end-point for which vDHCPv6Process() is called.
 *
 * @return DHCP state of the given endpoint
 *
 */
eDHCPState_t eGetDHCPv6State( struct xNetworkEndPoint * pxEndPoint )
{
    configASSERT( pxEndPoint );
    return EP_DHCPData.eDHCPState;
}
/*-----------------------------------------------------------*/

/**
 * @brief Check if option length is less than buffer size and larger than minimum requirement.
 *
 * @param[in] usOption The option code.
 * @param[in] uxOptionLength The option length to check.
 * @param[in] uxRemainingSize Remaining size in the buffer.
 *
 * @return pdTRUE if the length is valid, otherwise pdFALSE.
 */
static BaseType_t prvIsOptionLengthValid( uint16_t usOption,
                                          size_t uxOptionLength,
                                          size_t uxRemainingSize )
{
    BaseType_t xReturn = pdTRUE;
    size_t uxMinOptLength = 0U;

    switch( usOption )
    {
        case DHCPv6_Option_NonTemporaryAddress:
            /* Refer to RFC3315 - sec 22.4, the length of IA_NA should never less than 12. */
            uxMinOptLength = 12U;
            break;

        case DHCPv6_Option_TemporaryAddress:
            /* Refer to RFC3315 - sec 22.5, the length of IA_TA should never less than 4. */
            uxMinOptLength = 4U;
            break;

        case DHCPv6_Option_IA_Address:
            /* Refer to RFC3315 - sec 22.6, the length of IA should never less than 24. */
            uxMinOptLength = 24U;
            break;

        case DHCPv6_Option_Preference:
            /* Refer to RFC3315 - sec 22.8, the length of IA should never less than 1. */
            uxMinOptLength = 1U;
            break;

        case DHCPv6_Option_Status_Code:
            /* Refer to RFC3315 - sec 22.13, the length of status code should never less than 2. */
            uxMinOptLength = 2U;
            break;

        case DHCPv6_Option_IA_for_Prefix_Delegation:
            /* Refer to RFC3633 - sec 9, the length of IA_PD should never less than 12. */
            uxMinOptLength = 12U;
            break;

        case DHCPv6_Option_IA_Prefix:
            /* Refer to RFC3633 - sec 10, the length of status code should never less than 25. */
            uxMinOptLength = 25U;
            break;

        default:
            /* No constrain for other options. */
            uxMinOptLength = 0U;
            break;
    }

    if( uxOptionLength < uxMinOptLength )
    {
        FreeRTOS_printf( ( "prvIsOptionLengthValid: Length %u of option %u is less than minimum requirement %u\n",
                           ( unsigned ) uxOptionLength,
                           usOption,
                           ( unsigned ) uxMinOptLength ) );
        xReturn = pdFALSE;
    }
    else if( uxOptionLength > uxRemainingSize )
    {
        FreeRTOS_printf( ( "prvIsOptionLengthValid: Length %u of option %u is larger than remaining buffer size %u\n",
                           ( unsigned ) uxOptionLength,
                           usOption,
                           ( unsigned ) uxRemainingSize ) );
        xReturn = pdFALSE;
    }
    else
    {
        /* Do nothing. */
    }

    return xReturn;
}

/**
 * @brief A DHCP packet has a list of options, one of them is Status Code. This function is used to parse it.
 * @param[in] uxLength Total length for status code.
 * @param[in] pxMessage The raw packet as it was received.
 *
 * @return pdTRUE if status is success, otherwise pdFALSE.
 */
static BaseType_t prvDHCPv6_handleStatusCode( size_t uxLength,
                                              BitConfig_t * pxMessage )
{
    BaseType_t xReturn = pdTRUE;
    /* Since length is checked before entering, we can read message directly. */
    uint16_t usStatus = usBitConfig_read_16( pxMessage );
    uint8_t ucMessage[ 50 ];
    /* Minus 2 because we read 2 bytes for usStatus. */
    size_t uxReadLength = uxLength - 2U;

    FreeRTOS_printf( ( "Got status code %u\n",
                       usStatus ) );

    if( uxReadLength > sizeof( ucMessage ) - 1U )
    {
        uxReadLength = sizeof( ucMessage ) - 1U;
    }

    ( void ) xBitConfig_read_uc( pxMessage, ucMessage, uxReadLength );
    ucMessage[ uxReadLength ] = 0;
    FreeRTOS_printf( ( "Msg: '%s'\n", ucMessage ) );

    /* Read the remainder, if present. */
    if( uxLength > uxReadLength + 2U )
    {
        uxReadLength = uxLength - ( uxReadLength + 2U );
        ( void ) xBitConfig_read_uc( pxMessage, NULL, uxReadLength );
    }

    /* A status of 0 means: Success. (RFC 3315 - sec 24.4). */
    if( usStatus != 0U )
    {
        xReturn = pdFALSE;
    }

    return xReturn;
}

/**
 * @brief A DHCPv6 reply has been received. See to which end-point it belongs and pass it.
 *
 * @param[in] pxEndPoint The end-point for which vDHCPv6Process() is called.
 *
 * @return In case the message is passed to 'pxEndPoint', return pdFALSE, meaning that
 *         the it has done its periodic processing.
 */
static BaseType_t xDHCPv6Process_PassReplyToEndPoint( struct xNetworkEndPoint * pxEndPoint )
{
    uint32_t ulCompareResult = pdTRUE;
    BaseType_t xDoProcess = pdTRUE;
    struct xNetworkEndPoint * pxIterator;

    pxIterator = pxNetworkEndPoints;

    /* Find the end-point with given transaction ID. */
    while( pxIterator != NULL )
    {
        if( ( pxIterator->bits.bIPv6 != pdFALSE_UNSIGNED ) && ( pxIterator->bits.bWantDHCP != pdFALSE_UNSIGNED ) )
        {
            FreeRTOS_printf( ( "vDHCPProcess: 0x%06X == 0x%06X ?\n",
                               ( unsigned int ) xDHCPMessage.ulTransactionID,
                               ( unsigned int ) pxIterator->xDHCPData.ulTransactionId ) );

            if( ( xDHCPMessage.ulTransactionID == pxIterator->xDHCPData.ulTransactionId ) &&
                ( pxIterator->xDHCPData.eDHCPState != eLeasedAddress ) )
            {
                break;
            }
        }

        pxIterator = pxIterator->pxNext;
    }

    if( pxIterator != NULL )
    {
        if( pxIterator->pxDHCPMessage->xServerID.usDUIDType != 0U )
        {
            /* Check if the ID-type, the length and the contents are equal. */
            if( pxIterator->pxDHCPMessage->xServerID.uxLength > DHCPv6_MAX_CLIENT_SERVER_ID_LENGTH )
            {
                FreeRTOS_printf( ( "DHCPv6 invalid uxLength.\n" ) );
                ulCompareResult = pdFAIL;
            }
            else if( ( xDHCPMessage.xServerID.usDUIDType != pxIterator->pxDHCPMessage->xServerID.usDUIDType ) ||
                     ( xDHCPMessage.xServerID.uxLength != pxIterator->pxDHCPMessage->xServerID.uxLength ) ||
                     ( memcmp( xDHCPMessage.xServerID.pucID, pxIterator->pxDHCPMessage->xServerID.pucID, pxIterator->pxDHCPMessage->xServerID.uxLength ) != 0 ) )
            {
                FreeRTOS_printf( ( "DHCPv6 reply contains an unknown ID.\n" ) );
                ulCompareResult = pdFAIL;
            }
            else
            {
                /* do nothing, coverity happy */
            }
        }

        if( ulCompareResult == pdPASS )
        {
            ( void ) memcpy( ( void * ) pxIterator->pxDHCPMessage, ( const void * ) &xDHCPMessage, sizeof( xDHCPMessage ) );

            /* The second parameter pdTRUE tells to check for a UDP message. */
            vDHCPv6ProcessEndPoint( pdFALSE, pxIterator, pxIterator->pxDHCPMessage );
            pxIterator->pxDHCPMessage->ucHasUID = 0U;

            if( pxEndPoint == pxIterator )
            {
                xDoProcess = pdFALSE;
            }
        }
    }

    return xDoProcess;
}
/*-----------------------------------------------------------*/

/**
 * @brief Check the DHCP socket and run one cycle of the DHCP state machine.
 *
 * @param[in] xReset When pdTRUE, the state machine needs to be reset.  This may happen
 *            when the end-point has just become up.
 * @param[in] pxEndPoint The end-point that wants a DHCPv6 address.
 */
void vDHCPv6Process( BaseType_t xReset,
                     struct xNetworkEndPoint * pxEndPoint )
{
    BaseType_t xDoProcess = pdTRUE;

    configASSERT( pxEndPoint != NULL );

    /* Is DHCP starting over? */
    if( xReset != pdFALSE )
    {
        EP_DHCPData.eDHCPState = eInitialWait;

        if( pxEndPoint->pxDHCPMessage == NULL )
        {
            pxEndPoint->pxDHCPMessage = pvPortMalloc( sizeof( *pxEndPoint->pxDHCPMessage ) );

            if( pxEndPoint->pxDHCPMessage != NULL )
            {
                ( void ) memset( pxEndPoint->pxDHCPMessage, 0, sizeof( *pxEndPoint->pxDHCPMessage ) );
            }
            else
            {
                FreeRTOS_printf( ( "vDHCPv6Process: malloc failed %u bytes\n", ( unsigned int ) sizeof( *pxEndPoint->pxDHCPMessage ) ) );

                /* Use static IP address. */
                taskENTER_CRITICAL();
                {
                    ( void ) memcpy( EP_IPv6_SETTINGS.xIPAddress.ucBytes, pxEndPoint->ipv6_defaults.xIPAddress.ucBytes, ipSIZE_OF_IPv6_ADDRESS );
                    iptraceDHCP_REQUESTS_FAILED_USING_DEFAULT_IPv6_ADDRESS( EP_IPv6_SETTINGS.xIPAddress );
                }
                taskEXIT_CRITICAL();

                EP_DHCPData.eDHCPState = eNotUsingLeasedAddress;
                vIPSetDHCP_RATimerEnableState( pxEndPoint, pdFALSE );
                xDoProcess = pdFALSE;
            }
        }
    }

    /* If there is a socket, check for incoming messages first. */
    if( ( xDoProcess != pdFALSE ) && ( EP_DHCPData.xDHCPSocket != NULL ) )
    {
        uint8_t * pucUDPPayload;

        BaseType_t lBytes;
        size_t uxLength;

        for( ; ; )
        {
            BaseType_t xResult;
            BaseType_t xRecvFlags = ( BaseType_t ) FREERTOS_ZERO_COPY;

            /* Get the next UDP message. */
            lBytes = FreeRTOS_recvfrom( EP_DHCPData.xDHCPSocket, &( pucUDPPayload ), 0, xRecvFlags, NULL, NULL );

            if( lBytes <= 0 )
            {
                if( ( lBytes < 0 ) && ( lBytes != -pdFREERTOS_ERRNO_EAGAIN ) )
                {
                    FreeRTOS_printf( ( "vDHCPProcess: FreeRTOS_recvfrom returns %d\n", ( int ) lBytes ) );
                }

                break;
            }

            uxLength = ( size_t ) lBytes;

            xResult = prvDHCPv6Analyse( pxEndPoint, pucUDPPayload, uxLength, &( xDHCPMessage ) );

            FreeRTOS_printf( ( "prvDHCPv6Analyse: %s\n", ( xResult == pdPASS ) ? "Pass" : "Fail" ) );

            if( xResult == pdPASS )
            {
                xDoProcess = xDHCPv6Process_PassReplyToEndPoint( pxEndPoint );
            }
        }
    }

    if( xDoProcess != pdFALSE )
    {
        /* Process the end-point, but do not expect incoming packets. */
        vDHCPv6ProcessEndPoint( xReset, pxEndPoint, pxEndPoint->pxDHCPMessage );
    }
}

/**
 * @brief Stop the DHCP process. Close the DHCP socket when it's no longer used by any end-point.
 *
 * @param[in] pxEndPoint The end-point for which we want to stop the DHCP process.
 */
void vDHCPv6Stop( struct xNetworkEndPoint * pxEndPoint )
{
    /* Disable the DHCP timer. */
    vIPSetDHCP_RATimerEnableState( pxEndPoint, pdFALSE );

    /* Close socket to ensure packets don't queue on it. */
    prvCloseDHCPv6Socket( pxEndPoint );
}
/*-----------------------------------------------------------*/

/**
 * @brief The DHCP process is about ready: the server sends a confirmation that the
 *        assigned IPv6 address may be used. The settings will be copied to 'pxEndPoint->ipv6_settings'.
 * @param[in] pxEndPoint The end-point that is asking for an IP-address.
 * @param[in] pxDHCPMessage The reply received from the DHCP server.
 */
static void vDHCPv6ProcessEndPoint_HandleReply( NetworkEndPoint_t * pxEndPoint,
                                                DHCPMessage_IPv6_t * pxDHCPMessage )
{
    size_t uxDNSIndex;

    FreeRTOS_printf( ( "vDHCPProcess: acked %xip\n", ( unsigned ) FreeRTOS_ntohl( EP_DHCPData.ulOfferedIPAddress ) ) );

    /* DHCP completed.  The IP address can now be used, and the
     * timer set to the lease timeout time. */
    pxEndPoint->ipv6_settings.uxPrefixLength = pxDHCPMessage->ucprefixLength;                                                             /* Number of valid bytes in the network prefix. */
    ( void ) memcpy( pxEndPoint->ipv6_settings.xIPAddress.ucBytes, pxDHCPMessage->xIPAddress.xIP_IPv6.ucBytes, ipSIZE_OF_IPv6_ADDRESS );
    ( void ) memcpy( pxEndPoint->ipv6_settings.xPrefix.ucBytes, pxDHCPMessage->xPrefixAddress.xIP_IPv6.ucBytes, ipSIZE_OF_IPv6_ADDRESS ); /* The network prefix, e.g. fe80::/10 */
    /*pxEndPoint->xGatewayAddress;	/ * Gateway to the web. * / */

    for( uxDNSIndex = 0; uxDNSIndex < pxDHCPMessage->uxDNSCount; uxDNSIndex++ )
    {
        ( void ) memcpy( pxEndPoint->ipv6_settings.xDNSServerAddresses[ uxDNSIndex ].ucBytes, pxDHCPMessage->xDNSServers[ uxDNSIndex ].xIP_IPv6.ucBytes, ipSIZE_OF_IPv6_ADDRESS );
    }

    EP_DHCPData.eDHCPState = eLeasedAddress;

    iptraceDHCP_SUCCEEDED( EP_DHCPData.ulOfferedIPAddress );

    /* Close socket to ensure packets don't queue on it. */
    prvCloseDHCPv6Socket( pxEndPoint );

    if( EP_DHCPData.ulLeaseTime == 0U )
    {
        EP_DHCPData.ulLeaseTime = dhcpv6DEFAULT_LEASE_TIME;
    }
    else if( EP_DHCPData.ulLeaseTime < dhcpv6MINIMUM_LEASE_TIME )
    {
        EP_DHCPData.ulLeaseTime = dhcpv6MINIMUM_LEASE_TIME;
    }
    else
    {
        /* The lease time is already valid. */
    }

    /* Check for clashes. */

    vDHCP_RATimerReload( ( struct xNetworkEndPoint * ) pxEndPoint, EP_DHCPData.ulLeaseTime );

    /* DHCP failed, the default configured IP-address will be used
     * Now call vIPNetworkUpCalls() to send the network-up event and
     * start the Address Resolution timer. */
    vIPNetworkUpCalls( pxEndPoint );
}
/*-----------------------------------------------------------*/

/**
 * @brief An advertise packet has been received. Ask the application if
 *        it it shall send a request to obtain this IP-address.
 * @param[in] pxEndPoint The end-point that is asking for an IP-address.
 * @param[in] pxDHCPMessage The advertisement received from the DHCP server.
 * @return When the request will be send, pdFALSE will be returned.
 */
static BaseType_t xDHCPv6ProcessEndPoint_HandleAdvertise( NetworkEndPoint_t * pxEndPoint,
                                                          DHCPMessage_IPv6_t * pxDHCPMessage )
{
    BaseType_t xGivingUp = pdFALSE;

    #if ( ipconfigUSE_DHCP_HOOK != 0 ) && ( ipconfigIPv4_BACKWARD_COMPATIBLE != 1 )
        eDHCPCallbackAnswer_t eAnswer;
    #endif /* ( ipconfigUSE_DHCP_HOOK != 0 ) && ( ipconfigIPv4_BACKWARD_COMPATIBLE != 1 ) */

    #if ( ipconfigUSE_DHCP_HOOK != 0 ) && ( ipconfigIPv4_BACKWARD_COMPATIBLE != 1 )
        /* Ask the user if a DHCP request is required. */
        eAnswer = xApplicationDHCPHook_Multi( eDHCPPhasePreRequest, pxEndPoint, &( pxDHCPMessage->xIPAddress ) );

        if( eAnswer == eDHCPContinue )
    #endif /* ( ipconfigUSE_DHCP_HOOK != 0 ) && ( ipconfigIPv4_BACKWARD_COMPATIBLE != 1 ) */
    {
        /* An offer has been made, the user wants to continue,
         * generate the request. */
        EP_DHCPData.xDHCPTxTime = xTaskGetTickCount();
        EP_DHCPData.xDHCPTxPeriod = dhcpINITIAL_DHCP_TX_PERIOD;
        /* Force creating a new transaction ID. */
        pxDHCPMessage->ucHasUID = 0U;
        prvSendDHCPMessage( pxEndPoint );
        EP_DHCPData.eDHCPState = eWaitingAcknowledge;
    }

    #if ( ipconfigUSE_DHCP_HOOK != 0 ) && ( ipconfigIPv4_BACKWARD_COMPATIBLE != 1 )
        else
        {
            if( eAnswer == eDHCPUseDefaults )
            {
                ( void ) memcpy( &( pxEndPoint->ipv6_settings ), &( pxEndPoint->ipv6_defaults ), sizeof( pxEndPoint->ipv6_settings ) );
            }

            /* The user indicates that the DHCP process does not continue. */
            FreeRTOS_debug_printf( ( "xGivingUp because call-back 2\n" ) );
            xGivingUp = pdTRUE;
        }
    #endif /* ( ipconfigUSE_DHCP_HOOK != 0 ) && ( ipconfigIPv4_BACKWARD_COMPATIBLE != 1 ) */

    return xGivingUp;
}
/*-----------------------------------------------------------*/

/**
 * @brief This function is called periodically, or when a message was received for this end-point.
 * @param[in] pxEndPoint The end-point that is asking for an IP-address.
 * @param[in] pxDHCPMessage when not NULL, a message that was received for this end-point.
 * @return It returns pdTRUE in case the DHCP process is to be cancelled.
 */
static BaseType_t xDHCPv6ProcessEndPoint_HandleState( NetworkEndPoint_t * pxEndPoint,
                                                      DHCPMessage_IPv6_t * pxDHCPMessage )

{
    BaseType_t xGivingUp = pdFALSE;

    #if ( ipconfigUSE_DHCP_HOOK != 0 ) && ( ipconfigIPv4_BACKWARD_COMPATIBLE != 1 )
        eDHCPCallbackAnswer_t eAnswer;
    #endif /* ( ipconfigUSE_DHCP_HOOK != 0 ) && ( ipconfigIPv4_BACKWARD_COMPATIBLE != 1 ) */

    configASSERT( pxDHCPMessage != NULL );

    switch( EP_DHCPData.eDHCPState )
    {
        case eInitialWait:

            /* Initial state.  Create the DHCP socket, timer, etc. if they
             * have not already been created. */
            prvInitialiseDHCPv6( pxEndPoint );
            EP_DHCPData.eDHCPState = eWaitingSendFirstDiscover;
            break;

        case eWaitingSendFirstDiscover:
            /* Ask the user if a DHCP discovery is required. */
            #if ( ipconfigUSE_DHCP_HOOK != 0 ) && ( ipconfigIPv4_BACKWARD_COMPATIBLE != 1 )
                eAnswer = xApplicationDHCPHook_Multi( eDHCPPhasePreDiscover, pxEndPoint, &( pxDHCPMessage->xIPAddress ) );

                if( eAnswer == eDHCPContinue )
            #endif /* ( ipconfigUSE_DHCP_HOOK != 0 ) && ( ipconfigIPv4_BACKWARD_COMPATIBLE != 1 ) */
            {
                /* See if prvInitialiseDHCPv6() has created a socket. */
                if( EP_DHCPData.xDHCPSocket == NULL )
                {
                    FreeRTOS_debug_printf( ( "xGivingUp because socket is closed\n" ) );
                    xGivingUp = pdTRUE;
                }
                else
                {
                    /* Send the first discover request. */
                    EP_DHCPData.xDHCPTxTime = xTaskGetTickCount();
                    prvSendDHCPMessage( pxEndPoint );
                    EP_DHCPData.eDHCPState = eWaitingOffer;
                }
            }

            #if ( ipconfigUSE_DHCP_HOOK != 0 ) && ( ipconfigIPv4_BACKWARD_COMPATIBLE != 1 )
                else
                {
                    if( eAnswer == eDHCPUseDefaults )
                    {
                        ( void ) memcpy( &( pxEndPoint->ipv6_settings ), &( pxEndPoint->ipv6_defaults ), sizeof( pxEndPoint->ipv6_settings ) );
                    }

                    /* The user indicates that the DHCP process does not continue. */
                    FreeRTOS_debug_printf( ( "xGivingUp because call-back\n" ) );
                    xGivingUp = pdTRUE;
                }
            #endif /* ( ipconfigUSE_DHCP_HOOK != 0 ) && ( ipconfigIPv4_BACKWARD_COMPATIBLE != 1 ) */
            break;

        case eWaitingOffer:

            xGivingUp = pdFALSE;

            /* Look for offers coming in. */
            if( pxDHCPMessage->uxMessageType == DHCPv6_message_Type_Advertise )
            {
                xGivingUp = xDHCPv6ProcessEndPoint_HandleAdvertise( pxEndPoint, pxDHCPMessage );
            }

            /* Is it time to send another Discover? */
            else if( ( xTaskGetTickCount() - EP_DHCPData.xDHCPTxTime ) > EP_DHCPData.xDHCPTxPeriod )
            {
                /* It is time to send another Discover.  Increase the time
                 * period, and if it has not got to the point of giving up - send
                 * another discovery. */
                EP_DHCPData.xDHCPTxPeriod <<= 1;

                if( EP_DHCPData.xDHCPTxPeriod <= ipconfigMAXIMUM_DISCOVER_TX_PERIOD )
                {
                    EP_DHCPData.xDHCPTxTime = xTaskGetTickCount();
                    /* Make sure that the DHCP solicit request will be sent. */
                    EP_DHCPData.eDHCPState = eWaitingSendFirstDiscover;
                    prvSendDHCPMessage( pxEndPoint );
                    EP_DHCPData.eDHCPState = eWaitingOffer;
                    FreeRTOS_debug_printf( ( "vDHCPProcess: timeout %lu ticks\n", EP_DHCPData.xDHCPTxPeriod ) );
                }
                else
                {
                    FreeRTOS_debug_printf( ( "vDHCPProcess: giving up %lu > %lu ticks\n", EP_DHCPData.xDHCPTxPeriod, ipconfigMAXIMUM_DISCOVER_TX_PERIOD ) );

                    xGivingUp = pdTRUE;
                }
            }
            else
            {
                /* There was no DHCP reply, there was no time-out, just keep on waiting. */
            }

            break;

        case eWaitingAcknowledge:

            if( pxDHCPMessage->uxMessageType == DHCPv6_message_Type_Reply )
            {
                /* DHCP completed.  The IP address can now be used, and the
                 * timer set to the lease timeout time. */
                vDHCPv6ProcessEndPoint_HandleReply( pxEndPoint, pxDHCPMessage );
            }
            else if( ( xTaskGetTickCount() - EP_DHCPData.xDHCPTxTime ) > EP_DHCPData.xDHCPTxPeriod )
            {
                /* It is time to send another Request.  Increase the time
                 * period, and if it has not got to the point of giving up - send
                 * another discovery. */
                EP_DHCPData.xDHCPTxPeriod <<= 1;

                if( EP_DHCPData.xDHCPTxPeriod <= ipconfigMAXIMUM_DISCOVER_TX_PERIOD )
                {
                    EP_DHCPData.xDHCPTxTime = xTaskGetTickCount();
                    /* Make sure that the DHCPv6_message_Type_Request will be sent. */
                    EP_DHCPData.eDHCPState = eWaitingOffer;
                    prvSendDHCPMessage( pxEndPoint );
                    EP_DHCPData.eDHCPState = eWaitingAcknowledge;
                    FreeRTOS_debug_printf( ( "vDHCPProcess: timeout %lu ticks\n", EP_DHCPData.xDHCPTxPeriod ) );
                }
                else
                {
                    /* Give up, start again. */
                    EP_DHCPData.eDHCPState = eInitialWait;
                    FreeRTOS_debug_printf( ( "vDHCPProcess: timeout period is out of bound, restart DHCPv6\n" ) );
                    vDHCP_RATimerReload( pxEndPoint, dhcpINITIAL_TIMER_PERIOD );
                }
            }

            break;

        case eLeasedAddress:

            /* Resend the request at the appropriate time to renew the lease. */
            prvCreateDHCPv6Socket( pxEndPoint );

            EP_DHCPData.xDHCPTxTime = xTaskGetTickCount();
            EP_DHCPData.xDHCPTxPeriod = dhcpINITIAL_DHCP_TX_PERIOD;
            prvSendDHCPMessage( pxEndPoint );
            EP_DHCPData.eDHCPState = eWaitingAcknowledge;

            /* From now on, we should be called more often */
            vDHCP_RATimerReload( pxEndPoint, dhcpINITIAL_TIMER_PERIOD );

            break;

        case eNotUsingLeasedAddress:

            vIPSetDHCP_RATimerEnableState( pxEndPoint, pdFALSE );
            break;

        default:
            /* Lint: all options are included. */
            break;
    }

    return xGivingUp;
}
/*-----------------------------------------------------------*/

/**
 * @brief Run one cycle of the DHCP state machine.
 *
 * @param[in] xReset pdTRUE is the state machine has to be reset.
 * @param[in] pxEndPoint The end-point that needs DHCP.
 * @param[in] pxDHCPMessage A DHCP message that has just been received, or NULL.
 */
static void vDHCPv6ProcessEndPoint( BaseType_t xReset,
                                    NetworkEndPoint_t * pxEndPoint,
                                    DHCPMessage_IPv6_t * pxDHCPMessage )
{
    BaseType_t xGivingUp = pdFALSE;

    /* Is DHCP starting over? */
    if( xReset != pdFALSE )
    {
        EP_DHCPData.eDHCPState = eInitialWait;
    }

    if( ( EP_DHCPData.eDHCPState != EP_DHCPData.eExpectedState ) && ( xReset == pdFALSE ) )
    {
        /* When the DHCP event was generated, the DHCP client was
        * in a different state.  Therefore, ignore this event. */
        FreeRTOS_debug_printf( ( "vDHCPv6ProcessEndPoint: wrong state: expect: %d got: %d : ignore\n",
                                 EP_DHCPData.eExpectedState, EP_DHCPData.eDHCPState ) );
    }
    else
    {
        #if ( ipconfigHAS_DEBUG_PRINTF == 1 )
        {
            static eDHCPState_t lastState = eNotUsingLeasedAddress;

            if( lastState != EP_DHCPData.eDHCPState )
            {
                lastState = EP_DHCPData.eDHCPState;
                FreeRTOS_debug_printf( ( "vDHCPv6ProcessEndPoint: enter %s (%d)\n", prvStateName( EP_DHCPData.eDHCPState ), EP_DHCPData.eDHCPState ) );
            }
        }
        #endif /* ( ipconfigHAS_DEBUG_PRINTF == 1 ) */

        xGivingUp = xDHCPv6ProcessEndPoint_HandleState( pxEndPoint, pxDHCPMessage );

        if( xGivingUp != pdFALSE )
        {
            FreeRTOS_debug_printf( ( "vDHCPv6ProcessEndPoint: Giving up\n" ) );

            /* xGivingUp became true either because of a time-out, or because
             * xApplicationDHCPHook_Multi() returned another value than 'eDHCPContinue',
             * meaning that the conversion is cancelled from here. */

            /* Revert to static IP address. */
            taskENTER_CRITICAL();
            {
                ( void ) memcpy( EP_IPv6_SETTINGS.xIPAddress.ucBytes, pxEndPoint->ipv6_defaults.xIPAddress.ucBytes, ipSIZE_OF_IPv6_ADDRESS );
                iptraceDHCP_REQUESTS_FAILED_USING_DEFAULT_IPv6_ADDRESS( EP_IPv6_SETTINGS.xIPAddress );
            }
            taskEXIT_CRITICAL();

            EP_DHCPData.eDHCPState = eNotUsingLeasedAddress;
            vIPSetDHCP_RATimerEnableState( pxEndPoint, pdFALSE );

            /* Close socket to ensure packets don't queue on it. */
            prvCloseDHCPv6Socket( pxEndPoint );

            /* DHCP failed, the default configured IP-address will be used. Now
             * call vIPNetworkUpCalls() to send the network-up event and start the
             * Address Resolution timer. */
            vIPNetworkUpCalls( pxEndPoint );
        }
    }
}
/*-----------------------------------------------------------*/

/**
 * @brief Close the shared UDP/DHCP socket.  This results in lowering the reference count.
 *        The last user of the socket will close it.
 *
 * @param[in] pxEndPoint The end-point that wants to close the socket.
 */
static void prvCloseDHCPv6Socket( NetworkEndPoint_t * pxEndPoint )
{
    if( ( EP_DHCPData.xDHCPSocket == NULL ) || ( EP_DHCPData.xDHCPSocket != xDHCPv6Socket ) )
    {
        /* the socket can not be closed. */
    }
    else if( xDHCPv6SocketUserCount > 0 )
    {
        xDHCPv6SocketUserCount--;

        if( xDHCPv6SocketUserCount == 0 )
        {
            /* This modules runs from the IP-task. Use the internal
             * function 'vSocketClose()` to close the socket. */
            ( void ) vSocketClose( xDHCPv6Socket );
            xDHCPv6Socket = NULL;
        }

        EP_DHCPData.xDHCPSocket = NULL;
    }
    else
    {
        /* Strange: there is a socket, but there are no users. */
    }

    FreeRTOS_printf( ( "DHCP-socket[%02x-%02x]: closed, user count %d\n",
                       pxEndPoint->xMACAddress.ucBytes[ 4 ],
                       pxEndPoint->xMACAddress.ucBytes[ 5 ],
                       ( int ) xDHCPv6SocketUserCount ) );
}

/**
 * @brief Return the UDP/DHCP socket, or create if it doesn't exist.
 *
 * @param[in] pxEndPoint The end-point that needs the socket.
 */
static void prvCreateDHCPv6Socket( NetworkEndPoint_t * pxEndPoint )
{
    struct freertos_sockaddr xAddress;
    BaseType_t xReturn;
    TickType_t xTimeoutTime = ( TickType_t ) 0;

    if( ( xDHCPv6Socket != NULL ) && ( EP_DHCPData.xDHCPSocket == xDHCPv6Socket ) )
    {
        /* the socket is still valid. */
    }
    else if( xDHCPv6Socket == NULL ) /* Create the socket, if it has not already been created. */
    {
        xDHCPv6Socket = FreeRTOS_socket( FREERTOS_AF_INET6, FREERTOS_SOCK_DGRAM, FREERTOS_IPPROTO_UDP );
        configASSERT( xSocketValid( xDHCPv6Socket ) == pdTRUE );

        /* Ensure the Rx and Tx timeouts are zero as the DHCP executes in the
         * context of the IP task. */
        ( void ) FreeRTOS_setsockopt( xDHCPv6Socket, 0, FREERTOS_SO_RCVTIMEO, &( xTimeoutTime ), sizeof( TickType_t ) );
        ( void ) FreeRTOS_setsockopt( xDHCPv6Socket, 0, FREERTOS_SO_SNDTIMEO, &( xTimeoutTime ), sizeof( TickType_t ) );

        memset( &xAddress, 0, sizeof( xAddress ) );
        xAddress.sin_family = FREERTOS_AF_INET6;
        xAddress.sin_len = ( uint8_t ) sizeof( xAddress );
        /* Bind to the standard DHCP client port. */
        xAddress.sin_port = FreeRTOS_htons( ipDHCPv6_CLIENT_PORT );
        xReturn = vSocketBind( xDHCPv6Socket, &xAddress, sizeof( xAddress ), pdFALSE );
        configASSERT( xReturn == 0 );
        xDHCPv6SocketUserCount = 1;
        FreeRTOS_printf( ( "DHCP-socket[%02x-%02x]: DHCP Socket Create\n",
                           pxEndPoint->xMACAddress.ucBytes[ 4 ],
                           pxEndPoint->xMACAddress.ucBytes[ 5 ] ) );

        ( void ) xReturn;
    }
    else
    {
        xDHCPv6SocketUserCount++;
    }

    EP_DHCPData.xDHCPSocket = xDHCPv6Socket;
}
/*-----------------------------------------------------------*/

/**
 * @brief Initialise the DHCP state machine of a given end-point.
 *
 * @param[in] pxEndPoint The end-point.
 */
static void prvInitialiseDHCPv6( NetworkEndPoint_t * pxEndPoint )
{
    /* Initialise the parameters that will be set by the DHCP process. Per
     * https://www.ietf.org/rfc/rfc2131.txt Transaction ID should be a random
     * value chosen by the client. */

    /* Check for random number generator API failure. */
    EP_DHCPData.ulOfferedIPAddress = 0U;
    EP_DHCPData.ulDHCPServerAddress = 0U;
    EP_DHCPData.xDHCPTxPeriod = dhcpINITIAL_DHCP_TX_PERIOD;
    /* Force creating a new transaction ID. */
    pxEndPoint->pxDHCPMessage->ucHasUID = 0U;

    /* Create the DHCP socket if it has not already been created. */
    prvCreateDHCPv6Socket( pxEndPoint );
    FreeRTOS_debug_printf( ( "prvInitialiseDHCPv6: start after %lu ticks\n", dhcpINITIAL_TIMER_PERIOD ) );
    vDHCP_RATimerReload( pxEndPoint, dhcpINITIAL_TIMER_PERIOD );
}
/*-----------------------------------------------------------*/

/**
 * @brief Send a DHCPv6 message to a DHCP server.
 *
 * @param[in] pxEndPoint The end-point for which a message will be sent.
 */
static void prvSendDHCPMessage( NetworkEndPoint_t * pxEndPoint )
{
    BaseType_t xRandomOk = pdTRUE;
    DHCPMessage_IPv6_t * pxDHCPMessage = NULL;

    configASSERT( pxEndPoint != NULL );

    pxDHCPMessage = pxEndPoint->pxDHCPMessage;

    if( pxDHCPMessage->ucHasUID == 0U )
    {
        uint32_t ulTransactionID = 0U;
        uint32_t ulCurTime;

        xRandomOk = xApplicationGetRandomNumber( &( ulTransactionID ) );
        ulTransactionID &= 0xffffffU;

        /* The application should supply the following time-function. */
        ulCurTime = ulApplicationTimeHook();

        pxDHCPMessage->ulTimeStamp = ulCurTime - ( uint32_t ) SECS_FROM_1970_TILL_2000;
        pxDHCPMessage->ucHasUID = 1U;
        pxDHCPMessage->ucTransactionID[ 0 ] = ( uint8_t ) ( ( ulTransactionID >> 16 ) & 0xffU );
        pxDHCPMessage->ucTransactionID[ 1 ] = ( uint8_t ) ( ( ulTransactionID >> 8 ) & 0xffU );
        pxDHCPMessage->ucTransactionID[ 2 ] = ( uint8_t ) ( ulTransactionID & 0xffU );
        EP_DHCPData.ulTransactionId = ulTransactionID;
        FreeRTOS_debug_printf( ( "Created transaction ID : 0x%06X\n", ( unsigned ) ulTransactionID ) );
    }

    if( ( xRandomOk == pdPASS ) && ( EP_DHCPData.xDHCPSocket != NULL ) )
    {
        BitConfig_t xMessage;
        struct freertos_sockaddr xAddress;
        uint8_t ucMessageType = 0;

        /* Not useful, but MISRA issues a mandatory warning. */
        ( void ) memset( &( xMessage ), 0, sizeof( xMessage ) );

        if( xBitConfig_init( &( xMessage ), NULL, DHCPv6_SEND_MAX_BUFFER_SIZE ) == pdTRUE )
        {
            switch( EP_DHCPData.eDHCPState )
            {
                case eWaitingSendFirstDiscover:
                    ucMessageType = DHCPv6_message_Type_Solicit;
                    break;

                case eWaitingOffer:
                case eLeasedAddress:
                    ucMessageType = DHCPv6_message_Type_Request;
                    break;

                default:
                    /* No message to be sent in this stage. */
                    break;
            }

            if( ucMessageType != 0U )
            {
                /* DHCPv6_Option_IA_for_Prefix_Delegation */
                const uint32_t ulIAID = 0x27fe8f95;
                const uint32_t ulTime_1 = 3600U;
                const uint32_t ulTime_2 = 5400U;

                /* DHCPv6_Option_IA_Prefix */
                const uint32_t ulPreferredLifeTime = 4500U;
                const uint32_t ulPValidLifeTime = 7200U;
                const uint8_t ucPrefixLength = ( uint8_t ) pxEndPoint->ipv6_settings.uxPrefixLength;

                struct freertos_sockaddr * pxAddress;

                vBitConfig_write_8( &( xMessage ), ucMessageType ); /* 1 Solicit, 3, request */
                vBitConfig_write_uc( &( xMessage ), pxDHCPMessage->ucTransactionID, 3 );

                pxDHCPMessage->xClientID.usDUIDType = 1U;
                pxDHCPMessage->xClientID.usHardwareType = 1U;

                /* DHCPv6_Option_Client_Identifier */
                vBitConfig_write_16( &( xMessage ), DHCPv6_Option_Client_Identifier );                             /* Option is 1: Client Identifier */
                vBitConfig_write_16( &( xMessage ), 14U );                                                         /* The length is 14 */
                vBitConfig_write_16( &( xMessage ), pxDHCPMessage->xClientID.usDUIDType );                         /* 1 : Link Layer address + time */
                vBitConfig_write_16( &( xMessage ), pxDHCPMessage->xClientID.usHardwareType );                     /* 1 : Ethernet */
                vBitConfig_write_32( &( xMessage ), pxDHCPMessage->ulTimeStamp );                                  /* DUID Time: seconds since 1-1-2000. */
                vBitConfig_write_uc( &( xMessage ), pxEndPoint->xMACAddress.ucBytes, ipMAC_ADDRESS_LENGTH_BYTES ); /* Link Layer address, 6 bytes */
                ( void ) pucBitConfig_peek_last_index_uc( &( xMessage ), EP_DHCPData.ucClientDUID, dhcpIPv6_CLIENT_DUID_LENGTH );

                if( pxDHCPMessage->xServerID.uxLength != 0U )
                {
                    uint16_t usLength = ( uint16_t ) pxDHCPMessage->xServerID.uxLength;
                    /* DHCPv6_Option_Server_Identifier */
                    vBitConfig_write_16( &( xMessage ), DHCPv6_Option_Server_Identifier );     /* Option is 1: Server Identifier */
                    vBitConfig_write_16( &( xMessage ), ( uint16_t ) ( usLength + 4U ) );      /* The length is 14 */
                    vBitConfig_write_16( &( xMessage ), pxDHCPMessage->xServerID.usDUIDType ); /* The type of DUID: 1, 2, or 3. */
                    vBitConfig_write_16( &( xMessage ), pxDHCPMessage->xServerID.usHardwareType );
                    vBitConfig_write_uc( &( xMessage ), pxDHCPMessage->xServerID.pucID, pxDHCPMessage->xServerID.uxLength );
                }

                if( ( EP_DHCPData.eDHCPState == eWaitingOffer ) || ( EP_DHCPData.eDHCPState == eLeasedAddress ) )
                {
                    /* DHCPv6_Option_Option_List */
                    vBitConfig_write_16( &( xMessage ), DHCPv6_Option_Option_List );               /* usOption;	Option is 6 */
                    vBitConfig_write_16( &( xMessage ), 4U );                                      /* usLength;	length is 4 */
                    vBitConfig_write_16( &( xMessage ), DHCP6_OPTION_REQUEST_DNS );                /* usOption_1;	00 17 : DNS Recursive name server. */
                    vBitConfig_write_16( &( xMessage ), DHCP6_OPTION_REQUEST_DOMAIN_SEARCH_LIST ); /* usOption_2;	00 18 : Domain search list. */
                }

                /* DHCPv6_Option_Elapsed_Time */
                vBitConfig_write_16( &( xMessage ), DHCPv6_Option_Elapsed_Time );                                        /* usOption;	Option is 8 * / */
                vBitConfig_write_16( &( xMessage ), 2U );                                                                /* usLength;	length is 2 * / */
                vBitConfig_write_16( &( xMessage ), 0x0000 );                                                            /* usTime;		00 00 : 0 ms. * / */

                vBitConfig_write_16( &( xMessage ), DHCPv6_Option_IA_for_Prefix_Delegation );                            /* usOption;	Option is 25 */
                vBitConfig_write_16( &( xMessage ), 41 );                                                                /* usLength;	length is 12 + 29 = 41 */
                vBitConfig_write_32( &( xMessage ), ulIAID );                                                            /* 27 fe 8f 95. */
                vBitConfig_write_32( &( xMessage ), ulTime_1 );                                                          /* 00 00 0e 10: 3600 sec */
                vBitConfig_write_32( &( xMessage ), ulTime_2 );                                                          /* 00 00 15 18: 5400 sec */

                vBitConfig_write_16( &( xMessage ), DHCPv6_Option_IA_Prefix );                                           /* usOption   Option is 26 */
                vBitConfig_write_16( &( xMessage ), 25 );                                                                /* usLength   length is 25 */
                vBitConfig_write_32( &( xMessage ), ulPreferredLifeTime );                                               /* 4500 */
                vBitConfig_write_32( &( xMessage ), ulPValidLifeTime );                                                  /* e.g. 7200 seconds. */
                vBitConfig_write_8( &( xMessage ), ucPrefixLength );                                                     /* e.g. 64 bits */
                vBitConfig_write_uc( &( xMessage ), pxEndPoint->ipv6_settings.xPrefix.ucBytes, ipSIZE_OF_IPv6_ADDRESS ); /* 2001:0:0:fe00:: */

                vBitConfig_write_16( &( xMessage ), DHCPv6_Option_NonTemporaryAddress );                                 /* usOption   Option is 3 */
                vBitConfig_write_16( &( xMessage ), 12 );                                                                /* usLength   length is 12 */
                vBitConfig_write_32( &( xMessage ), ulIAID );                                                            /* 27 fe 8f 95. */
                vBitConfig_write_32( &( xMessage ), ulPreferredLifeTime );                                               /* 4500 */
                vBitConfig_write_32( &( xMessage ), ulPValidLifeTime );                                                  /* 7200 */

                if( ( EP_DHCPData.eDHCPState == eWaitingOffer ) || ( EP_DHCPData.eDHCPState == eLeasedAddress ) )
                {
                    vBitConfig_write_16( &( xMessage ), DHCPv6_Option_DNS_recursive_name_server ); /* usOption   Option is 23 */
                    vBitConfig_write_16( &( xMessage ), 0U );                                      /* usLength   length is 0 */
                }

                ( void ) memset( &( xAddress ), 0, sizeof xAddress );
                ( void ) FreeRTOS_inet_pton6( "ff02::1:2", xAddress.sin_address.xIP_IPv6.ucBytes );
                xAddress.sin_len = ( uint8_t ) sizeof xAddress; /* length of this structure. */
                xAddress.sin_family = FREERTOS_AF_INET6;
                xAddress.sin_port = FreeRTOS_htons( ipDHCPv6_SERVER_PORT );

                pxAddress = &( xAddress );

                FreeRTOS_printf( ( "DHCP Sending request %u.\n", ucMessageType ) );
                ( void ) FreeRTOS_sendto( EP_DHCPData.xDHCPSocket, ( const void * ) xMessage.ucContents, xMessage.uxIndex, 0, pxAddress, sizeof xAddress );
            }

            vBitConfig_release( &( xMessage ) );
        } /* if( xBitConfig_init( &( xMessage ), NULL, 256 ) == pdTRUE ) */
    }     /* ( xRandomOk == pdPASS ) && ( EP_DHCPData.xDHCPSocket != NULL ) */
}
/*-----------------------------------------------------------*/

/**
 * @brief Either the option 'NonTemporaryAddress' or 'IA_for_Prefix_Delegation'
 *        was received.  This function will read sub options like 'IA_Address',
 *        IA_Prefix, and Status_Code.
 *        It parses the raw message and fills 'pxDHCPMessage'.
 * @param[in] usOption The DHCPv6 option that was received.
 * @param[in] pxSet It contains the length and offset of the DHCP option.
 * @param[out] pxDHCPMessage it will be filled with the information from the option.
 * @param[in] pxMessage The raw packet as it was received.
 *
 * @return It returns pdTRUE in case the option parsed successfully, otherwise pdFALSE.
 */
static BaseType_t prvDHCPv6_subOption( uint16_t usOption,
                                       const DHCPOptionSet_t * pxSet,
                                       DHCPMessage_IPv6_t * pxDHCPMessage,
                                       BitConfig_t * pxMessage )
{
    uint32_t ulIAID = ulBitConfig_read_32( pxMessage );
    uint32_t ulTime_1 = ulBitConfig_read_32( pxMessage );
    uint32_t ulTime_2 = ulBitConfig_read_32( pxMessage );
    size_t uxUsed = pxMessage->uxIndex - pxSet->uxStart;
    size_t uxRemain = 0U;
    uint16_t usOption2;
    uint16_t uxLength2;
    BaseType_t xReturn = pdTRUE;

    ( void ) ulIAID;
    ( void ) ulTime_1;
    ( void ) ulTime_2;
    ( void ) usOption;

    if( pxSet->uxOptionLength >= uxUsed )
    {
        uxRemain = pxSet->uxOptionLength - uxUsed;
    }
    else
    {
        FreeRTOS_printf( ( "prvDHCPv6_subOption: Option %u used length %u is larger than option length %u\n", usOption, ( unsigned ) uxUsed, ( unsigned ) pxSet->uxOptionLength ) );
        xReturn = pdFALSE;
    }

    while( uxRemain > 0U )
    {
        if( uxRemain < 4U )
        {
            /* Sub-option length is always larger than 4 to store option code and length. */
            FreeRTOS_printf( ( "prvDHCPv6_subOption: %s has invalid option with length %u\n",
                               ( usOption == DHCPv6_Option_NonTemporaryAddress ) ? "Address assignment" : "Prefix Delegation",
                               ( unsigned ) uxRemain ) );
            xReturn = pdFALSE;
            break;
        }

        usOption2 = usBitConfig_read_16( pxMessage );
        uxLength2 = usBitConfig_read_16( pxMessage );

        uxUsed = pxMessage->uxIndex - pxSet->uxStart;

        /* Check sub-option length. */
        if( prvIsOptionLengthValid( usOption2, uxLength2, pxMessage->uxSize - pxMessage->uxIndex ) != pdTRUE )
        {
            FreeRTOS_printf( ( "prvDHCPv6_subOption: %u has invalid length %u, remaining buffer size %u\n",
                               usOption2,
                               ( unsigned ) uxLength2,
                               ( unsigned ) ( pxMessage->uxSize - pxMessage->uxIndex ) ) );
            xReturn = pdFALSE;
            break;
        }

        switch( usOption2 )
        {
            case DHCPv6_Option_IA_Address:
                ( void ) xBitConfig_read_uc( pxMessage, pxDHCPMessage->xIPAddress.xIP_IPv6.ucBytes, ipSIZE_OF_IPv6_ADDRESS );
                pxDHCPMessage->ulPreferredLifeTime = ulBitConfig_read_32( pxMessage );
                pxDHCPMessage->ulValidLifeTime = ulBitConfig_read_32( pxMessage );
                FreeRTOS_printf( ( "IP Address %pip\n", ( void * ) pxDHCPMessage->xIPAddress.xIP_IPv6.ucBytes ) );
                break;

            case DHCPv6_Option_IA_Prefix:
                pxDHCPMessage->ulPreferredLifeTime = ulBitConfig_read_32( pxMessage );
                pxDHCPMessage->ulValidLifeTime = ulBitConfig_read_32( pxMessage );
                pxDHCPMessage->ucprefixLength = ucBitConfig_read_8( pxMessage );
                ( void ) xBitConfig_read_uc( pxMessage, pxDHCPMessage->xPrefixAddress.xIP_IPv6.ucBytes, ipSIZE_OF_IPv6_ADDRESS );
                FreeRTOS_printf( ( "Address prefix: %pip length %d\n", ( void * ) pxDHCPMessage->xPrefixAddress.xIP_IPv6.ucBytes, pxDHCPMessage->ucprefixLength ) );
                break;

            case DHCPv6_Option_Status_Code:
                xReturn = prvDHCPv6_handleStatusCode( uxLength2,
                                                      pxMessage );
                break;

            default:
                ( void ) xBitConfig_read_uc( pxMessage, NULL, uxLength2 - uxUsed );
                FreeRTOS_printf( ( "prvDHCPv6Analyse: skipped unknown option %u\n", usOption2 ) );
                break;
        }

        if( xReturn != pdTRUE )
        {
            FreeRTOS_printf( ( "prvDHCPv6_subOption: One of sub-options %d returns error\n", usOption2 ) );
            break;
        }

        /* Update remaining length. */
        uxUsed = pxMessage->uxIndex - pxSet->uxStart;
        uxRemain = 0U;

        if( pxSet->uxOptionLength > uxUsed )
        {
            uxRemain = pxSet->uxOptionLength - uxUsed;
        }
    }

    return xReturn;
}
/*-----------------------------------------------------------*/

/**
 * @brief A DHCP packet has a list of options, each one starting with a type and a length
 *        field. This function parses a single DHCP option.
 * @param[in] pxEndPoint The end-point that wants a DHCPv6 address.
 * @param[in] usOption IPv6 DHCP option to be handled.
 * @param[in] pxSet It contains the length and offset of the DHCP option.
 * @param[out] pxDHCPMessage it will be filled with the information from the option.
 * @param[in] pxMessage The raw packet as it was received.
 */
static BaseType_t prvDHCPv6_handleOption( struct xNetworkEndPoint * pxEndPoint,
                                          uint16_t usOption,
                                          const DHCPOptionSet_t * pxSet,
                                          DHCPMessage_IPv6_t * pxDHCPMessage,
                                          BitConfig_t * pxMessage )
{
    BaseType_t xReady = pdFALSE;
    int32_t lIDSize = 0;
    uint8_t ucClientDUID[ dhcpIPv6_CLIENT_DUID_LENGTH ];

    if( prvIsOptionLengthValid( usOption, pxSet->uxOptionLength, pxMessage->uxSize - pxMessage->uxIndex ) != pdTRUE )
    {
        FreeRTOS_printf( ( "prvDHCPv6_handleOption: Option %u has invalid length %u, remaining buffer size %u\n",
                           usOption,
                           ( unsigned ) pxSet->uxOptionLength,
                           ( unsigned ) ( pxMessage->uxSize - pxMessage->uxIndex ) ) );
        xReady = pdTRUE;
    }

    if( xReady == pdFALSE )
    {
        switch( usOption )
        {
            case DHCPv6_Option_Status_Code:

                if( prvDHCPv6_handleStatusCode( pxSet->uxOptionLength, pxMessage ) != pdTRUE )
                {
                    pxMessage->xHasError = pdTRUE_UNSIGNED;
                }

                break;

            case DHCPv6_Option_Client_Identifier:
                lIDSize = ( int32_t ) ( pxSet->uxOptionLength ) - 4;

                if( lIDSize >= 0 )
                {
                    /*
                     *  1 : Link-layer address plus time (DUID-LLT)
                     *  2 : Vendor-assigned unique ID based on Enterprise Number (DUID-EN)
                     *  3 : Link-layer address (DUID-LL)
                     */
                    pxDHCPMessage->xClientID.uxLength = ( size_t ) lIDSize;
                    pxDHCPMessage->xClientID.usDUIDType = usBitConfig_read_16( pxMessage );     /* 0x0001 : Link Layer address + time */
                    pxDHCPMessage->xClientID.usHardwareType = usBitConfig_read_16( pxMessage ); /* 1 = Ethernet. */

                    if( ( size_t ) lIDSize <= sizeof( pxDHCPMessage->xClientID.pucID ) )
                    {
                        /* Refer to RFC3315 - sec 15.3, we need to discard packets with following conditions:
                         *  - the message does not include a Server Identifier option.
                         *  - the message does not include a Client Identifier option.
                         *  - the contents of the Client Identifier option does not match the client's DUID.
                         *  - the "transaction-id" field value does not match the value the client used in its Solicit message. */
                        ( void ) xBitConfig_read_uc( pxMessage, pxDHCPMessage->xClientID.pucID, ( size_t ) lIDSize ); /* Link Layer address, 6 bytes */

                        /* Check client DUID. */
                        if( ( pxSet->uxOptionLength != dhcpIPv6_CLIENT_DUID_LENGTH ) ||
                            ( pucBitConfig_peek_last_index_uc( pxMessage, ucClientDUID, pxSet->uxOptionLength ) != pdTRUE ) ||
                            ( memcmp( ucClientDUID, EP_DHCPData.ucClientDUID, dhcpIPv6_CLIENT_DUID_LENGTH ) != 0 ) )
                        {
                            FreeRTOS_printf( ( "prvDHCPv6Analyse: wrong client ID\n" ) );
                            pxMessage->xHasError = pdTRUE;
                        }
                    }
                    else
                    {
                        /* The value of DHCPv6_Option_Client_Identifier is invalid. */
                        FreeRTOS_printf( ( "prvDHCPv6Analyse: client ID too long\n" ) );
                        xReady = pdTRUE;
                    }
                }
                else
                {
                    /* The value of DHCPv6_Option_Client_Identifier is invalid. */
                    FreeRTOS_printf( ( "prvDHCPv6Analyse: client ID too short\n" ) );
                    xReady = pdTRUE;
                }

                break;

            case DHCPv6_Option_Server_Identifier:
                lIDSize = ( int32_t ) ( pxSet->uxOptionLength ) - 4;

                if( lIDSize >= 0 )
                {
                    /*
                     *  1 : Link-layer address plus time (DUID-LLT)
                     *  2 : Vendor-assigned unique ID based on Enterprise Number (DUID-EN)
                     *  3 : Link-layer address (DUID-LL)
                     */
                    pxDHCPMessage->xServerID.uxLength = ( size_t ) lIDSize;
                    pxDHCPMessage->xServerID.usDUIDType = usBitConfig_read_16( pxMessage );     /* 0x0001 : Link Layer address + time */
                    pxDHCPMessage->xServerID.usHardwareType = usBitConfig_read_16( pxMessage ); /* 1 = Ethernet. */

                    if( ( size_t ) lIDSize <= sizeof( pxDHCPMessage->xServerID.pucID ) )
                    {
                        ( void ) xBitConfig_read_uc( pxMessage, pxDHCPMessage->xServerID.pucID, ( size_t ) lIDSize ); /* Link Layer address, 6 bytes */
                    }
                    else
                    {
                        FreeRTOS_printf( ( "prvDHCPv6Analyse: server ID too long\n" ) );
                        xReady = pdTRUE;
                    }
                }
                else
                {
                    /* The value of DHCPv6_Option_Server_Identifier is invalid. */
                    FreeRTOS_printf( ( "prvDHCPv6Analyse: server ID too short\n" ) );
                    xReady = pdTRUE;
                }

                break;

            case DHCPv6_Option_Preference:
               {
                   uint8_t ucPreference = ucBitConfig_read_8( pxMessage );
                   ( void ) ucPreference;
                   break;
               }

            case DHCPv6_Option_DNS_recursive_name_server:
               {
                   size_t uDNSCount;

                   if( ( pxSet->uxOptionLength == 0U ) || ( ( pxSet->uxOptionLength % ipSIZE_OF_IPv6_ADDRESS ) != 0U ) )
                   {
                       FreeRTOS_printf( ( "prvDHCPv6Analyse: bad DNS length\n" ) );
                       xReady = pdTRUE;
                       break;
                   }

                   uDNSCount = pxSet->uxOptionLength / ipSIZE_OF_IPv6_ADDRESS;

                   while( uDNSCount > 0U )
                   {
                       if( pxDHCPMessage->uxDNSCount < ipconfigENDPOINT_DNS_ADDRESS_COUNT )
                       {
                           ( void ) xBitConfig_read_uc( pxMessage, pxDHCPMessage->xDNSServers[ pxDHCPMessage->uxDNSCount ].xIP_IPv6.ucBytes, ipSIZE_OF_IPv6_ADDRESS );
                           pxDHCPMessage->uxDNSCount++;
                       }
                       else
                       {
                           /* The DNS address can not be stored. Just advance the pointer. */
                           ( void ) xBitConfig_read_uc( pxMessage, NULL, ipSIZE_OF_IPv6_ADDRESS );
                       }

                       uDNSCount--;
                   }

                   break;
               }

            case DHCPv6_Option_Domain_Search_List:
                ( void ) xBitConfig_read_uc( pxMessage, NULL, pxSet->uxOptionLength );
                break;

            case DHCPv6_Option_NonTemporaryAddress:
            case DHCPv6_Option_IA_for_Prefix_Delegation:

                if( prvDHCPv6_subOption( usOption, pxSet, pxDHCPMessage, pxMessage ) != pdTRUE )
                {
                    FreeRTOS_printf( ( "prvDHCPv6_handleOption: prvDHCPv6_subOption returns error.\n" ) );
                    xReady = pdTRUE;
                }

                break;

            default:
                FreeRTOS_printf( ( "prvDHCPv6Analyse: Option %u not implemented\n", usOption ) );
                ( void ) xBitConfig_read_uc( pxMessage, NULL, pxSet->uxOptionLength );
                break;
        }
    }

    return xReady;
}
/*-----------------------------------------------------------*/

/**
 * @brief Analyse the reply from a DHCP server.
 *
 * @param[in] pxEndPoint The end-point that wants a DHCPv6 address.
 * @param[in] pucAnswer The payload text of the incoming packet.
 * @param[in] uxTotalLength The number of valid bytes in pucAnswer.
 * @param[in] pxDHCPMessage The DHCP object of the end-point.
 *
 * @return pdTRUE if the analysis was successful.
 */
static BaseType_t prvDHCPv6Analyse( struct xNetworkEndPoint * pxEndPoint,
                                    const uint8_t * pucAnswer,
                                    size_t uxTotalLength,
                                    DHCPMessage_IPv6_t * pxDHCPMessage )
{
    BitConfig_t xMessage;
    BaseType_t xResult = pdPASS;
    uint32_t ulOptionsReceived = 0U;

    /* Allocate enough space in 'xMessage' and clear it. */
    if( xBitConfig_init( &xMessage, pucAnswer, uxTotalLength ) != pdFAIL )
    {
        BaseType_t xReady = pdFALSE;

        ( void ) memset( pxDHCPMessage, 0, sizeof( *pxDHCPMessage ) );

        pxDHCPMessage->uxMessageType = ucBitConfig_read_8( &xMessage );
        ( void ) xBitConfig_read_uc( &xMessage, pxDHCPMessage->ucTransactionID, 3 );

        pxDHCPMessage->ulTransactionID =
            ( ( ( uint32_t ) pxDHCPMessage->ucTransactionID[ 0 ] ) << 16 ) |
            ( ( ( uint32_t ) pxDHCPMessage->ucTransactionID[ 1 ] ) << 8 ) |
            ( ( ( uint32_t ) pxDHCPMessage->ucTransactionID[ 2 ] ) );

        if( EP_DHCPData.ulTransactionId != pxDHCPMessage->ulTransactionID )
        {
            FreeRTOS_printf( ( "prvDHCPv6Analyse: Message %u transaction ID 0x%06X is different from sent ID 0x%06X\n",
                               ( unsigned ) pxDHCPMessage->uxMessageType,
                               ( unsigned ) pxDHCPMessage->ulTransactionID,
                               ( unsigned ) EP_DHCPData.ulTransactionId ) );

            xResult = pdFAIL;
        }

        FreeRTOS_printf( ( "prvDHCPv6Analyse: Message %u ID theirs 0x%06X\n",
                           ( unsigned ) pxDHCPMessage->uxMessageType,
                           ( unsigned ) pxDHCPMessage->ulTransactionID ) );

        switch( pxDHCPMessage->uxMessageType )
        {
            case DHCPv6_message_Type_Advertise:
            case DHCPv6_message_Type_Confirm:
            case DHCPv6_message_Type_Reply:
            case DHCPv6_message_Type_Decline:
                break;

            default:
                FreeRTOS_printf( ( "prvDHCPv6Analyse: Can not handle message type %u\n",
                                   ( unsigned ) pxDHCPMessage->uxMessageType ) );
                xResult = pdFAIL;
                break;
        }

        if( xResult == pdFAIL )
        {
            /* Ignore the reply because the message type is wrong. */
            xReady = pdTRUE;
        }
        else if( xMessage.xHasError != pdFALSE )
        {
            xReady = pdTRUE;
            xResult = pdFAIL;
        }
        else
        {
            /* xResult still equals 'pdPASS', carry on. */
        }

        while( xReady == pdFALSE )
        {
            uint16_t usOption;
            DHCPOptionSet_t xSet;

            ( void ) memset( &( xSet ), 0, sizeof( xSet ) );

            usOption = usBitConfig_read_16( &xMessage );
            xSet.uxOptionLength = ( size_t ) usBitConfig_read_16( &xMessage );
            xSet.uxStart = xMessage.uxIndex;

            if( xMessage.xHasError != pdFALSE )
            {
                FreeRTOS_printf( ( "prvDHCPv6Analyse: bad input\n" ) );
                xReady = pdTRUE;
                xResult = pdFAIL;
            }
            else
            {
                /* ulOptionsReceived has only 32-bits, it's not allowed to shift more than 32-bits on it. */
                if( usOption < 32 )
                {
                    /* Store the option by bit-map only if it's less than 32. */
                    ulOptionsReceived |= ( ( ( uint32_t ) 1U ) << usOption );
                }

                xReady = prvDHCPv6_handleOption( pxEndPoint, usOption, &( xSet ), pxDHCPMessage, &( xMessage ) );
            }

            if( xMessage.xHasError != pdFALSE )
            {
                FreeRTOS_printf( ( "prvDHCPv6Analyse: bad input\n" ) );
                xReady = pdTRUE;
                xResult = pdFAIL;
            }
            else if( xReady == pdTRUE )
            {
                /* xReady might be set to pdTRUE by prvDHCPv6_handleOption when there happens any error on parsing options. */
                FreeRTOS_printf( ( "prvDHCPv6Analyse: prvDHCPv6_handleOption returns error.\n" ) );
                xResult = pdFAIL;
            }
            else if( xMessage.uxIndex == xMessage.uxSize )
            {
                xReady = pdTRUE;
            }
            else
            {
                /* More options will follow. */
            }
        } /* while( xReady == pdFALSE ) */

        vBitConfig_release( &( xMessage ) );

        if( xResult == pdPASS )
        {
            ulOptionsReceived &= dhcpMANDATORY_OPTIONS;

            if( ulOptionsReceived != dhcpMANDATORY_OPTIONS )
            {
                xResult = pdFAIL;
                FreeRTOS_printf( ( "prvDHCPv6Analyse: Missing options: %X not equal to %X\n", ( unsigned int ) ulOptionsReceived, ( unsigned int ) dhcpMANDATORY_OPTIONS ) );
            }
        }
    } /* if( xBitConfig_init() ) */
    else
    {
        xResult = pdFAIL;
    }

    return xResult;
}
/*-----------------------------------------------------------*/

/**
 * @brief transform a state number in a descriptive string.
 *
 * @param[in] eState The DHCP state number.
 *
 * @return A descriptive string.
 */
#if ( ipconfigHAS_DEBUG_PRINTF == 1 )
    static const char * prvStateName( eDHCPState_t eState )
    {
        const char * pcName;

        switch( eState )
        {
            case eInitialWait:
                pcName = "eInitialWait";
                break;

            case eWaitingSendFirstDiscover:
                pcName = "eWaitingSendFirstDiscover";
                break;

            case eWaitingOffer:
                pcName = "eWaitingOffer";
                break;

            case eWaitingAcknowledge:
                pcName = "eWaitingAcknowledge";
                break;

            case eLeasedAddress:
                pcName = "eLeasedAddress";
                break;

            case eNotUsingLeasedAddress:
                pcName = "eNotUsingLeasedAddress";
                break;

            case eSendDHCPRequest:
                pcName = "eSendDHCPRequest";
                break;

            default:
                pcName = "Unknown state";
                break;
        }

        return pcName;
    }
#endif /* ( ipconfigHAS_DEBUG_PRINTF == 1 ) */

/* *INDENT-OFF* */
    #endif /* ( ipconfigUSE_IPv6 != 0 ) && ( ipconfigUSE_DHCPv6 != 0 ) */
/* *INDENT-ON* */
