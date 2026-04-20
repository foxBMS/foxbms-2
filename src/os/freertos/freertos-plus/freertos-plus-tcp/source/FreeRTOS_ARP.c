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
 * @file FreeRTOS_ARP.c
 * @brief Implements the Address Resolution Protocol for the FreeRTOS+TCP network stack.
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
#include "FreeRTOS_IP_Timers.h"
#include "FreeRTOS_Sockets.h"
#include "FreeRTOS_IP_Private.h"
#include "FreeRTOS_ARP.h"
#include "FreeRTOS_UDP_IP.h"
#include "FreeRTOS_DHCP.h"
#if ( ipconfigUSE_LLMNR == 1 )
    #include "FreeRTOS_DNS.h"
#endif /* ipconfigUSE_LLMNR */
#include "NetworkBufferManagement.h"
#include "NetworkInterface.h"
#include "FreeRTOS_Routing.h"

#if ( ipconfigUSE_IPv4 != 0 )

/** @brief When the age of an entry in the ARP table reaches this value (it counts down
 * to zero, so this is an old entry) an ARP request will be sent to see if the
 * entry is still valid and can therefore be refreshed. */
    #define arpMAX_ARP_AGE_BEFORE_NEW_ARP_REQUEST    ( 3U )

/** @brief The time between gratuitous ARPs. */
    #ifndef arpGRATUITOUS_ARP_PERIOD
        #define arpGRATUITOUS_ARP_PERIOD    ( pdMS_TO_TICKS( 20000U ) )
    #endif

/** @brief When there is another device which has the same IP address as the IP address
 * of this device, a defensive ARP request should be sent out. However, according to
 * RFC 5227 section 1.1, there must be a minimum interval of 10 seconds between
 * consecutive defensive ARP packets. */
    #ifndef arpIP_CLASH_RESET_TIMEOUT_MS
        #define arpIP_CLASH_RESET_TIMEOUT_MS    10000U
    #endif

/** @brief Maximum number of defensive ARPs to be sent for an ARP clash per
 * arpIP_CLASH_RESET_TIMEOUT_MS period. The retries are limited to one as outlined
 * by RFC 5227 section 2.4 part b.*/
    #ifndef arpIP_CLASH_MAX_RETRIES
        #define arpIP_CLASH_MAX_RETRIES    1U
    #endif


    static void vARPProcessPacketRequest( ARPPacket_t * pxARPFrame,
                                          NetworkEndPoint_t * pxTargetEndPoint,
                                          uint32_t ulSenderProtocolAddress );

    static void vARPProcessPacketReply( const ARPPacket_t * pxARPFrame,
                                        NetworkEndPoint_t * pxTargetEndPoint,
                                        uint32_t ulSenderProtocolAddress );

/*
 * Lookup an MAC address in the ARP cache from the IP address.
 */
    static eResolutionLookupResult_t prvCacheLookup( uint32_t ulAddressToLookup,
                                                     MACAddress_t * const pxMACAddress,
                                                     NetworkEndPoint_t ** ppxEndPoint );

    static eResolutionLookupResult_t eARPGetCacheEntryGateWay( uint32_t * pulIPAddress,
                                                               MACAddress_t * const pxMACAddress,
                                                               struct xNetworkEndPoint ** ppxEndPoint );

    static BaseType_t prvFindCacheEntry( const MACAddress_t * pxMACAddress,
                                         const uint32_t ulIPAddress,
                                         struct xNetworkEndPoint * pxEndPoint,
                                         CacheLocation_t * pxLocation );

/*-----------------------------------------------------------*/

/** @brief The ARP cache. */
    _static ARPCacheRow_t xARPCache[ ipconfigARP_CACHE_ENTRIES ];


/*
 * IP-clash detection is currently only used internally. When DHCP doesn't respond, the
 * driver can try out a random LinkLayer IP address (169.254.x.x).  It will send out a
 * gratuitous ARP message and, after a period of time, check the variables here below:
 */
    #if ( ipconfigARP_USE_CLASH_DETECTION != 0 )
        /* Becomes non-zero if another device responded to a gratuitous ARP message. */
        BaseType_t xARPHadIPClash;
        /* MAC-address of the other device containing the same IP-address. */
        MACAddress_t xARPClashMacAddress;
    #endif /* ipconfigARP_USE_CLASH_DETECTION */

/*-----------------------------------------------------------*/

/** @brief  The time at which the last gratuitous ARP was sent.  Gratuitous ARPs are used
 * to ensure ARP tables are up to date and to detect IP address conflicts. */
    static TickType_t xLastGratuitousARPTime = 0U;

/**
 * @brief Process the ARP packets.
 *
 * @param[in] pxNetworkBuffer The network buffer with the packet to be processed.
 *
 * @return An enum which says whether to return the frame or to release it.
 */
    eFrameProcessingResult_t eARPProcessPacket( const NetworkBufferDescriptor_t * pxNetworkBuffer )
    {
        /* MISRA Ref 11.3.1 [Misaligned access] */
        /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
        /* coverity[misra_c_2012_rule_11_3_violation] */
        ARPPacket_t * pxARPFrame = ( ( ARPPacket_t * ) pxNetworkBuffer->pucEthernetBuffer );
        eFrameProcessingResult_t eReturn = eReleaseBuffer;
        const ARPHeader_t * pxARPHeader;
        uint32_t ulTargetProtocolAddress, ulSenderProtocolAddress;

        /* memcpy() helper variables for MISRA Rule 21.15 compliance*/
        const void * pvCopySource;
        void * pvCopyDest;
        NetworkEndPoint_t * pxTargetEndPoint = pxNetworkBuffer->pxEndPoint;

        /* Next defensive request must not be sent for arpIP_CLASH_RESET_TIMEOUT_MS
         * period. */
        static TickType_t uxARPClashTimeoutPeriod = pdMS_TO_TICKS( arpIP_CLASH_RESET_TIMEOUT_MS );

        /* This local variable is used to keep track of number of ARP requests sent and
         * also to limit the requests to arpIP_CLASH_MAX_RETRIES per arpIP_CLASH_RESET_TIMEOUT_MS
         * period. */
        static UBaseType_t uxARPClashCounter = 0U;
        /* The time at which the last ARP clash was sent. */
        static TimeOut_t xARPClashTimeOut;

        pxARPHeader = &( pxARPFrame->xARPHeader );

        /* Only Ethernet hardware type is supported.
         * Only IPv4 address can be present in the ARP packet.
         * The hardware length (the MAC address) must be 6 bytes. And,
         * The Protocol address length must be 4 bytes as it is IPv4. */
        if( ( pxARPHeader->usHardwareType == ipARP_HARDWARE_TYPE_ETHERNET ) &&
            ( pxARPHeader->usProtocolType == ipARP_PROTOCOL_TYPE ) &&
            ( pxARPHeader->ucHardwareAddressLength == ipMAC_ADDRESS_LENGTH_BYTES ) &&
            ( pxARPHeader->ucProtocolAddressLength == ipIP_ADDRESS_LENGTH_BYTES ) )
        {
            /* The field ucSenderProtocolAddress is badly aligned, copy byte-by-byte. */

            /*
             * Use helper variables for memcpy() to remain
             * compliant with MISRA Rule 21.15.  These should be
             * optimized away.
             */
            pvCopySource = pxARPHeader->ucSenderProtocolAddress;
            pvCopyDest = &ulSenderProtocolAddress;
            ( void ) memcpy( pvCopyDest, pvCopySource, sizeof( ulSenderProtocolAddress ) );
            /* The field ulTargetProtocolAddress is well-aligned, a 32-bits copy. */
            ulTargetProtocolAddress = pxARPHeader->ulTargetProtocolAddress;

            if( uxARPClashCounter != 0U )
            {
                /* Has the timeout been reached? */
                if( xTaskCheckForTimeOut( &xARPClashTimeOut, &uxARPClashTimeoutPeriod ) == pdTRUE )
                {
                    /* We have waited long enough, reset the counter. */
                    uxARPClashCounter = 0;
                }
            }

            /* Check whether the lowest bit of the highest byte is 1 to check for
             * multicast address or even a broadcast address (FF:FF:FF:FF:FF:FF). */
            if( ( pxARPHeader->xSenderHardwareAddress.ucBytes[ 0 ] & 0x01U ) == 0x01U )
            {
                /* Senders address is a multicast OR broadcast address which is not
                 * allowed for an ARP packet. Drop the packet. See RFC 1812 section
                 * 3.3.2. */
                iptraceDROPPED_INVALID_ARP_PACKET( pxARPHeader );
            }
            else if( ( ipFIRST_LOOPBACK_IPv4 <= ( FreeRTOS_ntohl( ulSenderProtocolAddress ) ) ) &&
                     ( ( FreeRTOS_ntohl( ulSenderProtocolAddress ) ) < ipLAST_LOOPBACK_IPv4 ) )
            {
                /* The local loopback addresses must never appear outside a host. See RFC 1122
                 * section 3.2.1.3. */
                iptraceDROPPED_INVALID_ARP_PACKET( pxARPHeader );
            }
            /* Check whether there is a clash with another device for this IP address. */
            else if( ( pxTargetEndPoint != NULL ) && ( ulSenderProtocolAddress == pxTargetEndPoint->ipv4_settings.ulIPAddress ) )
            {
                if( uxARPClashCounter < arpIP_CLASH_MAX_RETRIES )
                {
                    /* Increment the counter. */
                    uxARPClashCounter++;

                    /* Send out a defensive ARP request. */
                    FreeRTOS_OutputARPRequest_Multi( pxTargetEndPoint, pxTargetEndPoint->ipv4_settings.ulIPAddress );

                    /* Since an ARP Request for this IP was just sent, do not send a gratuitous
                     * ARP for arpGRATUITOUS_ARP_PERIOD. */
                    xLastGratuitousARPTime = xTaskGetTickCount();

                    /* Note the time at which this request was sent. */
                    vTaskSetTimeOutState( &xARPClashTimeOut );

                    /* Reset the time-out period to the given value. */
                    uxARPClashTimeoutPeriod = pdMS_TO_TICKS( arpIP_CLASH_RESET_TIMEOUT_MS );
                }

                /* Process received ARP frame to see if there is a clash. */
                #if ( ipconfigARP_USE_CLASH_DETECTION != 0 )
                {
                    NetworkEndPoint_t * pxSourceEndPoint = FreeRTOS_FindEndPointOnIP_IPv4( ulSenderProtocolAddress );

                    if( ( pxSourceEndPoint != NULL ) && ( pxSourceEndPoint->ipv4_settings.ulIPAddress == ulSenderProtocolAddress ) )
                    {
                        xARPHadIPClash = pdTRUE;
                        /* Remember the MAC-address of the other device which has the same IP-address. */
                        ( void ) memcpy( xARPClashMacAddress.ucBytes, pxARPHeader->xSenderHardwareAddress.ucBytes, sizeof( xARPClashMacAddress.ucBytes ) );
                    }
                }
                #endif /* ipconfigARP_USE_CLASH_DETECTION */
            }
            else
            {
                iptraceARP_PACKET_RECEIVED();

                /* Some extra logging while still testing. */
                #if ( ipconfigHAS_DEBUG_PRINTF != 0 )
                    if( pxARPHeader->usOperation == ( uint16_t ) ipARP_REPLY )
                    {
                        FreeRTOS_debug_printf( ( "ipARP_REPLY from %xip to %xip end-point %xip\n",
                                                 ( unsigned ) FreeRTOS_ntohl( ulSenderProtocolAddress ),
                                                 ( unsigned ) FreeRTOS_ntohl( ulTargetProtocolAddress ),
                                                 ( unsigned ) FreeRTOS_ntohl( ( pxTargetEndPoint != NULL ) ? pxTargetEndPoint->ipv4_settings.ulIPAddress : 0U ) ) );
                    }
                #endif /* ( ipconfigHAS_DEBUG_PRINTF != 0 ) */

                #if ( ipconfigHAS_DEBUG_PRINTF != 0 )
                    if( ( pxARPHeader->usOperation == ( uint16_t ) ipARP_REQUEST ) &&
                        ( ulSenderProtocolAddress != ulTargetProtocolAddress ) &&
                        ( pxTargetEndPoint != NULL ) )
                    {
                        FreeRTOS_debug_printf( ( "ipARP_REQUEST from %xip to %xip end-point %xip\n",
                                                 ( unsigned ) FreeRTOS_ntohl( ulSenderProtocolAddress ),
                                                 ( unsigned ) FreeRTOS_ntohl( ulTargetProtocolAddress ),
                                                 ( unsigned ) ( FreeRTOS_ntohl( ( pxTargetEndPoint != NULL ) ? pxTargetEndPoint->ipv4_settings.ulIPAddress : 0U ) ) ) );
                    }
                #endif /* ( ipconfigHAS_DEBUG_PRINTF != 0 ) */

                /* ulTargetProtocolAddress won't be used unless logging is enabled. */
                ( void ) ulTargetProtocolAddress;

                /* Don't do anything if the local IP address is zero because
                 * that means a DHCP request has not completed. */
                if( ( pxTargetEndPoint != NULL ) && ( pxTargetEndPoint->bits.bEndPointUp != pdFALSE_UNSIGNED ) )
                {
                    switch( pxARPHeader->usOperation )
                    {
                        case ipARP_REQUEST:

                            if( ulTargetProtocolAddress == pxTargetEndPoint->ipv4_settings.ulIPAddress )
                            {
                                if( memcmp( pxTargetEndPoint->xMACAddress.ucBytes,
                                            pxARPHeader->xSenderHardwareAddress.ucBytes,
                                            ipMAC_ADDRESS_LENGTH_BYTES ) != 0 )
                                {
                                    vARPProcessPacketRequest( pxARPFrame, pxTargetEndPoint, ulSenderProtocolAddress );
                                    eReturn = eReturnEthernetFrame;
                                }
                            }
                            /* Check if its a Gratuitous ARP request and verify if it belongs to same subnet mask. */
                            else if( ( ulSenderProtocolAddress == ulTargetProtocolAddress ) &&
                                     ( ( ulSenderProtocolAddress & pxTargetEndPoint->ipv4_settings.ulNetMask ) == ( pxTargetEndPoint->ipv4_settings.ulNetMask & pxTargetEndPoint->ipv4_settings.ulIPAddress ) ) )
                            {
                                const MACAddress_t xGARPTargetAddress = { { 0, 0, 0, 0, 0, 0 } };

                                /* Make sure target MAC address is either ff:ff:ff:ff:ff:ff or 00:00:00:00:00:00 and senders MAC
                                 * address is not matching with the endpoint MAC address. */
                                if( ( ( memcmp( pxARPHeader->xTargetHardwareAddress.ucBytes, xBroadcastMACAddress.ucBytes, ipMAC_ADDRESS_LENGTH_BYTES ) == 0 ) ||
                                      ( ( memcmp( pxARPHeader->xTargetHardwareAddress.ucBytes, xGARPTargetAddress.ucBytes, ipMAC_ADDRESS_LENGTH_BYTES ) == 0 ) ) ) &&
                                    ( memcmp( pxTargetEndPoint->xMACAddress.ucBytes, pxARPHeader->xSenderHardwareAddress.ucBytes, ipMAC_ADDRESS_LENGTH_BYTES ) != 0 ) )
                                {
                                    MACAddress_t xHardwareAddress;
                                    NetworkEndPoint_t * pxCachedEndPoint;

                                    pxCachedEndPoint = NULL;

                                    /* The request is a Gratuitous ARP message.
                                     * Refresh the entry if it already exists. */
                                    /* Determine the ARP cache status for the requested IP address. */
                                    if( eARPGetCacheEntry( &( ulSenderProtocolAddress ), &( xHardwareAddress ), &( pxCachedEndPoint ) ) == eResolutionCacheHit )
                                    {
                                        /* Check if the endpoint matches with the one present in the ARP cache */
                                        if( pxCachedEndPoint == pxTargetEndPoint )
                                        {
                                            vARPRefreshCacheEntry( &( pxARPHeader->xSenderHardwareAddress ), ulSenderProtocolAddress, pxTargetEndPoint );
                                        }
                                    }
                                }
                            }
                            else
                            {
                                /* do nothing, coverity happy */
                            }

                            break;

                        case ipARP_REPLY:
                            vARPProcessPacketReply( pxARPFrame, pxTargetEndPoint, ulSenderProtocolAddress );
                            break;

                        default:
                            /* Invalid. */
                            break;
                    }
                }
            }
        }
        else
        {
            iptraceDROPPED_INVALID_ARP_PACKET( pxARPHeader );
        }

        return eReturn;
    }
/*-----------------------------------------------------------*/

/**
 * @brief Process an ARP request packets.
 *
 * @param[in] pxARPFrame the complete ARP-frame.
 * @param[in] pxTargetEndPoint the end-point that handles the peer's address.
 * @param[in] ulSenderProtocolAddress the IP-address of the sender.
 *
 */
    static void vARPProcessPacketRequest( ARPPacket_t * pxARPFrame,
                                          NetworkEndPoint_t * pxTargetEndPoint,
                                          uint32_t ulSenderProtocolAddress )
    {
        ARPHeader_t * pxARPHeader = &( pxARPFrame->xARPHeader );
/* memcpy() helper variables for MISRA Rule 21.15 compliance*/
        const void * pvCopySource;
        void * pvCopyDest;


        /* The packet contained an ARP request.  Was it for the IP
         * address of one of the end-points? */
        /* It has been confirmed that pxTargetEndPoint is not NULL. */
        iptraceSENDING_ARP_REPLY( ulSenderProtocolAddress );

        /* The request is for the address of this node.  Add the
         * entry into the ARP cache, or refresh the entry if it
         * already exists. */
        vARPRefreshCacheEntry( &( pxARPHeader->xSenderHardwareAddress ), ulSenderProtocolAddress, pxTargetEndPoint );

        /* Generate a reply payload in the same buffer. */
        pxARPHeader->usOperation = ( uint16_t ) ipARP_REPLY;

        /* A double IP address cannot be detected here, it is taken care in the Process ARP Packets path */

        /*
         * Use helper variables for memcpy() to remain
         * compliant with MISRA Rule 21.15.  These should be
         * optimized away.
         */
        pvCopySource = pxARPHeader->xSenderHardwareAddress.ucBytes;
        pvCopyDest = pxARPHeader->xTargetHardwareAddress.ucBytes;
        ( void ) memcpy( pvCopyDest, pvCopySource, sizeof( MACAddress_t ) );
        pxARPHeader->ulTargetProtocolAddress = ulSenderProtocolAddress;

        /*
         * Use helper variables for memcpy() to remain
         * compliant with MISRA Rule 21.15.  These should be
         * optimized away.
         */
        pvCopySource = pxTargetEndPoint->xMACAddress.ucBytes;
        pvCopyDest = pxARPHeader->xSenderHardwareAddress.ucBytes;
        ( void ) memcpy( pvCopyDest, pvCopySource, sizeof( MACAddress_t ) );
        pvCopySource = &( pxTargetEndPoint->ipv4_settings.ulIPAddress );
        pvCopyDest = pxARPHeader->ucSenderProtocolAddress;
        ( void ) memcpy( pvCopyDest, pvCopySource, sizeof( pxARPHeader->ucSenderProtocolAddress ) );
    }
/*-----------------------------------------------------------*/

/**
 * @brief A device has sent an ARP reply, process it.
 * @param[in] pxARPFrame The ARP packet received.
 * @param[in] pxTargetEndPoint The end-point on which it is received.
 * @param[in] ulSenderProtocolAddress The IPv4 address involved.
 */
    static void vARPProcessPacketReply( const ARPPacket_t * pxARPFrame,
                                        NetworkEndPoint_t * pxTargetEndPoint,
                                        uint32_t ulSenderProtocolAddress )
    {
        const ARPHeader_t * pxARPHeader = &( pxARPFrame->xARPHeader );
        uint32_t ulTargetProtocolAddress = pxARPHeader->ulTargetProtocolAddress;

        /* If the packet is meant for this device or if the entry already exists. */
        if( ( ulTargetProtocolAddress == pxTargetEndPoint->ipv4_settings.ulIPAddress ) ||
            ( xIsIPInARPCache( ulSenderProtocolAddress ) == pdTRUE ) )
        {
            iptracePROCESSING_RECEIVED_ARP_REPLY( ulTargetProtocolAddress );
            vARPRefreshCacheEntry( &( pxARPHeader->xSenderHardwareAddress ), ulSenderProtocolAddress, pxTargetEndPoint );
        }

        if( ( pxARPWaitingNetworkBuffer != NULL ) &&
            ( uxIPHeaderSizePacket( pxARPWaitingNetworkBuffer ) == ipSIZE_OF_IPv4_HEADER ) )
        {
            /* MISRA Ref 11.3.1 [Misaligned access] */
/* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
            /* coverity[misra_c_2012_rule_11_3_violation] */
            const IPPacket_t * pxARPWaitingIPPacket = ( ( IPPacket_t * ) pxARPWaitingNetworkBuffer->pucEthernetBuffer );
            const IPHeader_t * pxARPWaitingIPHeader = &( pxARPWaitingIPPacket->xIPHeader );

            if( ulSenderProtocolAddress == pxARPWaitingIPHeader->ulSourceIPAddress )
            {
                IPStackEvent_t xEventMessage;
                const TickType_t xDontBlock = ( TickType_t ) 0;

                xEventMessage.eEventType = eNetworkRxEvent;
                xEventMessage.pvData = ( void * ) pxARPWaitingNetworkBuffer;

                if( xSendEventStructToIPTask( &xEventMessage, xDontBlock ) != pdPASS )
                {
                    /* Failed to send the message, so release the network buffer. */
                    vReleaseNetworkBufferAndDescriptor( pxARPWaitingNetworkBuffer );
                }

                /* Clear the buffer. */
                pxARPWaitingNetworkBuffer = NULL;

                /* Found an ARP resolution, disable ARP resolution timer. */
                vIPSetARPResolutionTimerEnableState( pdFALSE );

                iptrace_DELAYED_ARP_REQUEST_REPLIED();
            }
        }
    }
/*-----------------------------------------------------------*/

/**
 * @brief Check whether an IP address is in the ARP cache.
 *
 * @param[in] ulAddressToLookup The 32-bit representation of an IP address to
 *                    check for.
 *
 * @return When the IP-address is found: pdTRUE, else pdFALSE.
 */
    BaseType_t xIsIPInARPCache( uint32_t ulAddressToLookup )
    {
        BaseType_t x, xReturn = pdFALSE;

        /* Loop through each entry in the ARP cache. */
        for( x = 0; x < ipconfigARP_CACHE_ENTRIES; x++ )
        {
            /* Does this row in the ARP cache table hold an entry for the IP address
             * being queried? */
            if( xARPCache[ x ].ulIPAddress == ulAddressToLookup )
            {
                xReturn = pdTRUE;

                /* A matching valid entry was found. */
                if( xARPCache[ x ].ucValid == ( uint8_t ) pdFALSE )
                {
                    /* This entry is waiting an ARP reply, so is not valid. */
                    xReturn = pdFALSE;
                }

                break;
            }
        }

        return xReturn;
    }

/**
 * @brief Check whether a packet needs ARP resolution if it is on local subnet. If required send an ARP request.
 *
 * @param[in] pxNetworkBuffer The network buffer with the packet to be checked.
 *
 * @return pdTRUE if the packet needs ARP resolution, pdFALSE otherwise.
 */
    BaseType_t xCheckRequiresARPResolution( const NetworkBufferDescriptor_t * pxNetworkBuffer )
    {
        BaseType_t xNeedsARPResolution = pdFALSE;

        /* MISRA Ref 11.3.1 [Misaligned access] */
        /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
        /* coverity[misra_c_2012_rule_11_3_violation] */
        const IPPacket_t * pxIPPacket = ( ( const IPPacket_t * ) pxNetworkBuffer->pucEthernetBuffer );
        const IPHeader_t * pxIPHeader = &( pxIPPacket->xIPHeader );
        const IPV4Parameters_t * pxIPv4Settings = &( pxNetworkBuffer->pxEndPoint->ipv4_settings );

        configASSERT( ( pxIPPacket->xEthernetHeader.usFrameType == ipIPv4_FRAME_TYPE ) || ( pxIPPacket->xEthernetHeader.usFrameType == ipARP_FRAME_TYPE ) );

        if( ( pxIPHeader->ulSourceIPAddress & pxIPv4Settings->ulNetMask ) == ( pxIPv4Settings->ulIPAddress & pxIPv4Settings->ulNetMask ) )
        {
            /* If the IP is on the same subnet and we do not have an ARP entry already,
             * then we should send out ARP for finding the MAC address. */
            if( xIsIPInARPCache( pxIPHeader->ulSourceIPAddress ) == pdFALSE )
            {
                FreeRTOS_OutputARPRequest_Multi( pxNetworkBuffer->pxEndPoint, pxIPHeader->ulSourceIPAddress );

                /* This packet needs resolution since this is on the same subnet
                 * but not in the ARP cache. */
                xNeedsARPResolution = pdTRUE;
            }
        }

        return xNeedsARPResolution;
    }

    #if ( ipconfigUSE_ARP_REMOVE_ENTRY != 0 )

/**
 * @brief Remove an ARP cache entry that matches with .pxMACAddress.
 *
 * @param[in] pxMACAddress Pointer to the MAC address whose entry shall
 *                          be removed.
 * @return When the entry was found and remove: the IP-address, otherwise zero.
 */
        uint32_t ulARPRemoveCacheEntryByMac( const MACAddress_t * pxMACAddress )
        {
            BaseType_t x;
            uint32_t lResult = 0;

            configASSERT( pxMACAddress != NULL );

            /* For each entry in the ARP cache table. */
            for( x = 0; x < ipconfigARP_CACHE_ENTRIES; x++ )
            {
                if( ( memcmp( xARPCache[ x ].xMACAddress.ucBytes, pxMACAddress->ucBytes, sizeof( pxMACAddress->ucBytes ) ) == 0 ) )
                {
                    lResult = xARPCache[ x ].ulIPAddress;
                    ( void ) memset( &xARPCache[ x ], 0, sizeof( xARPCache[ x ] ) );
                    break;
                }
            }

            return lResult;
        }

    #endif /* ipconfigUSE_ARP_REMOVE_ENTRY != 0 */
/*-----------------------------------------------------------*/

/**
 * @brief Look for an IP-MAC couple in ARP cache and reset the 'age' field. If no match
 *        is found then no action will be taken.
 *
 * @param[in] pxMACAddress Pointer to the MAC address whose entry needs to be updated.
 * @param[in] ulIPAddress the IP address whose corresponding entry needs to be updated.
 */
    void vARPRefreshCacheEntryAge( const MACAddress_t * pxMACAddress,
                                   const uint32_t ulIPAddress )
    {
        BaseType_t x;

        if( pxMACAddress != NULL )
        {
            /* Loop through each entry in the ARP cache. */
            for( x = 0; x < ipconfigARP_CACHE_ENTRIES; x++ )
            {
                /* Does this line in the cache table hold an entry for the IP
                 * address being queried? */
                if( xARPCache[ x ].ulIPAddress == ulIPAddress )
                {
                    /* Does this cache entry have the same MAC address? */
                    if( memcmp( xARPCache[ x ].xMACAddress.ucBytes, pxMACAddress->ucBytes, sizeof( pxMACAddress->ucBytes ) ) == 0 )
                    {
                        /* The IP address and the MAC matched, update this entry age. */
                        xARPCache[ x ].ucAge = ( uint8_t ) ipconfigMAX_ARP_AGE;
                        break;
                    }
                }
            }
        }
    }
/*-----------------------------------------------------------*/

/**
 * @brief Add/update the ARP cache entry MAC-address to IP-address mapping.
 *
 * @param[in] pxMACAddress Pointer to the MAC address whose mapping is being
 *                          updated.
 * @param[in] ulIPAddress 32-bit representation of the IP-address whose mapping
 *                         is being updated.
 * @param[in] pxEndPoint The end-point stored in the table.
 */
    void vARPRefreshCacheEntry( const MACAddress_t * pxMACAddress,
                                const uint32_t ulIPAddress,
                                struct xNetworkEndPoint * pxEndPoint )
    {
        #if ( ipconfigARP_STORES_REMOTE_ADDRESSES == 0 )
            /* Only process the IP address if it is on the local network. */
            BaseType_t xAddressIsLocal = ( FreeRTOS_FindEndPointOnNetMask( ulIPAddress ) != NULL ) ? 1 : 0; /* ARP remote address. */

            /* Only process the IP address if it matches with one of the end-points. */
            if( xAddressIsLocal != 0 )
        #else

            /* If ipconfigARP_STORES_REMOTE_ADDRESSES is non-zero, IP addresses with
             * a different netmask will also be stored.  After when replying to a UDP
             * message from a different netmask, the IP address can be looped up and a
             * reply sent.  This option is useful for systems with multiple gateways,
             * the reply will surely arrive.  If ipconfigARP_STORES_REMOTE_ADDRESSES is
             * zero the the gateway address is the only option. */

            if( pdTRUE )
        #endif
        {
            CacheLocation_t xLocation;
            BaseType_t xReady;

            xReady = prvFindCacheEntry( pxMACAddress, ulIPAddress, pxEndPoint, &( xLocation ) );

            if( xReady == pdFALSE )
            {
                if( xLocation.xMacEntry >= 0 )
                {
                    xLocation.xUseEntry = xLocation.xMacEntry;

                    if( xLocation.xIpEntry >= 0 )
                    {
                        /* Both the MAC address as well as the IP address were found in
                         * different locations: clear the entry which matches the
                         * IP-address */
                        ( void ) memset( &( xARPCache[ xLocation.xIpEntry ] ), 0, sizeof( ARPCacheRow_t ) );
                    }
                }
                else if( xLocation.xIpEntry >= 0 )
                {
                    /* An entry containing the IP-address was found, but it had a different MAC address */
                    xLocation.xUseEntry = xLocation.xIpEntry;
                }
                else
                {
                    /* No matching entry found. */
                }

                /* If the entry was not found, we use the oldest entry and set the IPaddress */
                xARPCache[ xLocation.xUseEntry ].ulIPAddress = ulIPAddress;

                if( pxMACAddress != NULL )
                {
                    ( void ) memcpy( xARPCache[ xLocation.xUseEntry ].xMACAddress.ucBytes, pxMACAddress->ucBytes, sizeof( pxMACAddress->ucBytes ) );

                    iptraceARP_TABLE_ENTRY_CREATED( ulIPAddress, ( *pxMACAddress ) );
                    /* And this entry does not need immediate attention */
                    xARPCache[ xLocation.xUseEntry ].ucAge = ( uint8_t ) ipconfigMAX_ARP_AGE;
                    xARPCache[ xLocation.xUseEntry ].ucValid = ( uint8_t ) pdTRUE;
                    xARPCache[ xLocation.xUseEntry ].pxEndPoint = pxEndPoint;
                }
                else if( xLocation.xIpEntry < 0 )
                {
                    xARPCache[ xLocation.xUseEntry ].ucAge = ( uint8_t ) ipconfigMAX_ARP_RETRANSMISSIONS;
                    xARPCache[ xLocation.xUseEntry ].ucValid = ( uint8_t ) pdFALSE;
                }
                else
                {
                    /* Nothing will be stored. */
                }
            }
        }
    }
/*-----------------------------------------------------------*/

/**
 * @brief The results of an ARP look-up shall be stored in the ARP cache.
 *        This helper function looks up the location.
 * @param[in] pxMACAddress The MAC-address belonging to the IP-address.
 * @param[in] ulIPAddress The IP-address of the entry.
 * @param[in] pxEndPoint The end-point that will stored in the table.
 * @param[out] pxLocation The results of this search are written in this struct.
 */
    static BaseType_t prvFindCacheEntry( const MACAddress_t * pxMACAddress,
                                         const uint32_t ulIPAddress,
                                         struct xNetworkEndPoint * pxEndPoint,
                                         CacheLocation_t * pxLocation )
    {
        BaseType_t x = 0;
        uint8_t ucMinAgeFound = 0U;
        BaseType_t xReturn = pdFALSE;

        #if ( ipconfigARP_STORES_REMOTE_ADDRESSES != 0 )
            BaseType_t xAddressIsLocal = ( FreeRTOS_FindEndPointOnNetMask( ulIPAddress ) != NULL ) ? 1 : 0; /* ARP remote address. */
        #endif

        /* Start with the maximum possible number. */
        ucMinAgeFound--;

        pxLocation->xIpEntry = -1;
        pxLocation->xMacEntry = -1;
        pxLocation->xUseEntry = 0;

        /* For each entry in the ARP cache table. */
        for( x = 0; x < ipconfigARP_CACHE_ENTRIES; x++ )
        {
            BaseType_t xMatchingMAC = pdFALSE;

            if( pxMACAddress != NULL )
            {
                if( memcmp( xARPCache[ x ].xMACAddress.ucBytes, pxMACAddress->ucBytes, sizeof( pxMACAddress->ucBytes ) ) == 0 )
                {
                    xMatchingMAC = pdTRUE;
                }
            }

            /* Does this line in the cache table hold an entry for the IP
             * address being queried? */
            if( xARPCache[ x ].ulIPAddress == ulIPAddress )
            {
                if( pxMACAddress == NULL )
                {
                    /* In case the parameter pxMACAddress is NULL, an entry will be reserved to
                     * indicate that there is an outstanding ARP request, This entry will have
                     * "ucValid == pdFALSE". */
                    pxLocation->xIpEntry = x;
                    break;
                }

                /* See if the MAC-address also matches. */
                if( xMatchingMAC != pdFALSE )
                {
                    /* This function will be called for each received packet
                     * This is by far the most common path. */
                    xARPCache[ x ].ucAge = ( uint8_t ) ipconfigMAX_ARP_AGE;
                    xARPCache[ x ].ucValid = ( uint8_t ) pdTRUE;
                    xARPCache[ x ].pxEndPoint = pxEndPoint;
                    /* Indicate to the caller that the entry is updated. */
                    xReturn = pdTRUE;
                    break;
                }

                /* Found an entry containing ulIPAddress, but the MAC address
                 * doesn't match.  Might be an entry with ucValid=pdFALSE, waiting
                 * for an ARP reply.  Still want to see if there is match with the
                 * given MAC address.ucBytes.  If found, either of the two entries
                 * must be cleared. */
                pxLocation->xIpEntry = x;
            }
            else if( xMatchingMAC != pdFALSE )
            {
                /* Found an entry with the given MAC-address, but the IP-address
                 * is different.  Continue looping to find a possible match with
                 * ulIPAddress. */
                #if ( ipconfigARP_STORES_REMOTE_ADDRESSES != 0 )
                {
                    /* If ARP stores the MAC address of IP addresses outside the
                     * network, than the MAC address of the gateway should not be
                     * overwritten. */
                    BaseType_t xOtherIsLocal = ( FreeRTOS_FindEndPointOnNetMask( xARPCache[ x ].ulIPAddress ) != NULL ) ? 1 : 0; /* ARP remote address. */

                    if( xAddressIsLocal == xOtherIsLocal )
                    {
                        pxLocation->xMacEntry = x;
                    }
                }
                #else /* if ( ipconfigARP_STORES_REMOTE_ADDRESSES != 0 ) */
                {
                    pxLocation->xMacEntry = x;
                }
                #endif /* if ( ipconfigARP_STORES_REMOTE_ADDRESSES != 0 ) */
            }

            /* _HT_
             * Shouldn't we test for xARPCache[ x ].ucValid == pdFALSE here ? */
            else if( xARPCache[ x ].ucAge < ucMinAgeFound )
            {
                /* As the table is traversed, remember the table row that
                 * contains the oldest entry (the lowest age count, as ages are
                 * decremented to zero) so the row can be re-used if this function
                 * needs to add an entry that does not already exist. */
                ucMinAgeFound = xARPCache[ x ].ucAge;
                pxLocation->xUseEntry = x;
            }
            else
            {
                /* Nothing happens to this cache entry for now. */
            }
        } /* for( x = 0; x < ipconfigARP_CACHE_ENTRIES; x++ ) */

        return xReturn;
    }
/*-----------------------------------------------------------*/

    #if ( ipconfigUSE_ARP_REVERSED_LOOKUP == 1 )

/**
 * @brief Retrieve an entry from the cache table
 *
 * @param[in] pxMACAddress The MAC-address of the entry of interest.
 * @param[out] pulIPAddress set to the IP-address found, or unchanged when not found.
 *
 * @return Either eResolutionCacheMiss or eResolutionCacheHit.
 */
        eResolutionLookupResult_t eARPGetCacheEntryByMac( const MACAddress_t * const pxMACAddress,
                                                          uint32_t * pulIPAddress,
                                                          struct xNetworkInterface ** ppxInterface )
        {
            BaseType_t x;
            eResolutionLookupResult_t eReturn = eResolutionCacheMiss;

            configASSERT( pxMACAddress != NULL );
            configASSERT( pulIPAddress != NULL );

            if( ppxInterface != NULL )
            {
                *( ppxInterface ) = NULL;
            }

            /* Loop through each entry in the ARP cache. */
            for( x = 0; x < ipconfigARP_CACHE_ENTRIES; x++ )
            {
                /* Does this row in the ARP cache table hold an entry for the MAC
                 * address being searched? */
                if( memcmp( pxMACAddress->ucBytes, xARPCache[ x ].xMACAddress.ucBytes, sizeof( MACAddress_t ) ) == 0 )
                {
                    *pulIPAddress = xARPCache[ x ].ulIPAddress;

                    if( ( ppxInterface != NULL ) &&
                        ( xARPCache[ x ].pxEndPoint != NULL ) )
                    {
                        *( ppxInterface ) = xARPCache[ x ].pxEndPoint->pxNetworkInterface;
                    }

                    eReturn = eResolutionCacheHit;
                    break;
                }
            }

            return eReturn;
        }
    #endif /* ipconfigUSE_ARP_REVERSED_LOOKUP */

/*-----------------------------------------------------------*/

/**
 * @brief Look for ulIPAddress in the ARP cache.
 *
 * @param[in,out] pulIPAddress Pointer to the IP-address to be queried to the ARP cache.
 * @param[in,out] pxMACAddress Pointer to a MACAddress_t variable where the MAC address
 *                          will be stored, if found.
 * @param[out] ppxEndPoint Pointer to the end-point of the gateway will be stored.
 *
 * @return If the IP address exists, copy the associated MAC address into pxMACAddress,
 *         refresh the ARP cache entry's age, and return eResolutionCacheHit. If the IP
 *         address does not exist in the ARP cache return eResolutionCacheMiss. If the packet
 *         cannot be sent for any reason (maybe DHCP is still in process, or the
 *         addressing needs a gateway but there isn't a gateway defined) then return
 *         eResolutionFailed.
 */
    eResolutionLookupResult_t eARPGetCacheEntry( uint32_t * pulIPAddress,
                                                 MACAddress_t * const pxMACAddress,
                                                 struct xNetworkEndPoint ** ppxEndPoint )
    {
        eResolutionLookupResult_t eReturn = eResolutionFailed;
        uint32_t ulAddressToLookup;
        NetworkEndPoint_t * pxEndPoint = NULL;

        configASSERT( pxMACAddress != NULL );
        configASSERT( pulIPAddress != NULL );
        configASSERT( ppxEndPoint != NULL );

        *( ppxEndPoint ) = NULL;
        ulAddressToLookup = *pulIPAddress;
        pxEndPoint = FreeRTOS_FindEndPointOnIP_IPv4( ulAddressToLookup );

        if( xIsIPv4Loopback( ulAddressToLookup ) != 0 )
        {
            if( pxEndPoint != NULL )
            {
                /* For multi-cast, use the first IPv4 end-point. */
                ( void ) memcpy( pxMACAddress->ucBytes, pxEndPoint->xMACAddress.ucBytes, sizeof( pxMACAddress->ucBytes ) );
                *( ppxEndPoint ) = pxEndPoint;
                eReturn = eResolutionCacheHit;
            }
        }
        else if( xIsIPv4Multicast( ulAddressToLookup ) != 0 )
        {
            /* Get the lowest 23 bits of the IP-address. */
            vSetMultiCastIPv4MacAddress( ulAddressToLookup, pxMACAddress );

            pxEndPoint = FreeRTOS_FirstEndPoint( NULL );

            for( ;
                 pxEndPoint != NULL;
                 pxEndPoint = FreeRTOS_NextEndPoint( NULL, pxEndPoint ) )
            {
                if( pxEndPoint->bits.bIPv6 == 0U ) /*NULL End Point is checked in the for loop, no need for an extra check */
                {
                    /* For multi-cast, use the first IPv4 end-point. */
                    *( ppxEndPoint ) = pxEndPoint;
                    eReturn = eResolutionCacheHit;
                    break;
                }
            }
        }
        else if( ( FreeRTOS_htonl( ulAddressToLookup ) & 0xffU ) == 0xffU ) /* Is this a broadcast address like x.x.x.255 ? */
        {
            /* This is a broadcast so it uses the broadcast MAC address. */
            ( void ) memcpy( pxMACAddress->ucBytes, xBroadcastMACAddress.ucBytes, sizeof( MACAddress_t ) );
            pxEndPoint = FreeRTOS_FindEndPointOnNetMask( ulAddressToLookup );

            if( pxEndPoint != NULL )
            {
                *( ppxEndPoint ) = pxEndPoint;
            }

            eReturn = eResolutionCacheHit;
        }
        else
        {
            eReturn = eARPGetCacheEntryGateWay( pulIPAddress, pxMACAddress, ppxEndPoint );
        }

        return eReturn;
    }
/*-----------------------------------------------------------*/

/**
 * @brief The IPv4 address is apparently a web-address. Find a gateway..
 * @param[in] pulIPAddress The target IP-address. It may be replaced with the IP
 *                          address of a gateway.
 * @param[in] pxMACAddress In case the MAC-address is found in cache, it will be
 *                          stored to the buffer provided.
 * @param[out] ppxEndPoint The end-point of the gateway will be copy to the pointee.
 */
    static eResolutionLookupResult_t eARPGetCacheEntryGateWay( uint32_t * pulIPAddress,
                                                               MACAddress_t * const pxMACAddress,
                                                               struct xNetworkEndPoint ** ppxEndPoint )
    {
        eResolutionLookupResult_t eReturn = eResolutionCacheMiss;
        uint32_t ulAddressToLookup = *( pulIPAddress );
        NetworkEndPoint_t * pxEndPoint;
        uint32_t ulOriginal = *pulIPAddress;

        /* It is assumed that devices with the same netmask are on the same
         * LAN and don't need a gateway. */
        pxEndPoint = FreeRTOS_FindEndPointOnNetMask( ulAddressToLookup );

        if( pxEndPoint == NULL )
        {
            /* No matching end-point is found, look for a gateway. */
            #if ( ipconfigARP_STORES_REMOTE_ADDRESSES == 1 )
                eReturn = prvCacheLookup( ulAddressToLookup, pxMACAddress, ppxEndPoint );

                if( eReturn == eResolutionCacheHit )
                {
                    /* The stack is configured to store 'remote IP addresses', i.e. addresses
                     * belonging to a different the netmask.  prvCacheLookup() returned a hit, so
                     * the MAC address is known. */
                }
                else
            #endif
            {
                /* The IP address is off the local network, so look up the
                 * hardware address of the router, if any. */
                *( ppxEndPoint ) = FreeRTOS_FindGateWay( ( BaseType_t ) ipTYPE_IPv4 );

                if( *( ppxEndPoint ) != NULL )
                {
                    /* 'ipv4_settings' can be accessed safely, because 'ipTYPE_IPv4' was provided. */
                    ulAddressToLookup = ( *ppxEndPoint )->ipv4_settings.ulGatewayAddress;
                }
                else
                {
                    ulAddressToLookup = 0U;
                }
            }
        }
        else
        {
            /* The IP address is on the local network, so lookup the requested
             * IP address directly. */
            ulAddressToLookup = *pulIPAddress;
            *ppxEndPoint = pxEndPoint;
        }

        #if ( ipconfigARP_STORES_REMOTE_ADDRESSES == 1 )
            if( eReturn == eResolutionCacheMiss )
        #endif
        {
            if( ulAddressToLookup == 0U )
            {
                /* The address is not on the local network, and there is not a
                 * router. */
                eReturn = eResolutionFailed;
            }
            else
            {
                eReturn = prvCacheLookup( ulAddressToLookup, pxMACAddress, ppxEndPoint );

                if( ( eReturn != eResolutionCacheHit ) || ( ulOriginal != ulAddressToLookup ) )
                {
                    FreeRTOS_debug_printf( ( "ARP %xip %s using %xip\n",
                                             ( unsigned ) FreeRTOS_ntohl( ulOriginal ),
                                             ( eReturn == eResolutionCacheHit ) ? "hit" : "miss",
                                             ( unsigned ) FreeRTOS_ntohl( ulAddressToLookup ) ) );
                }

                /* It might be that the ARP has to go to the gateway. */
                *pulIPAddress = ulAddressToLookup;
            }
        }

        return eReturn;
    }
/*-----------------------------------------------------------*/

/**
 * @brief Lookup an IP address in the ARP cache.
 *
 * @param[in] ulAddressToLookup The 32-bit representation of an IP address to
 *                               lookup.
 * @param[out] pxMACAddress A pointer to MACAddress_t variable where, if there
 *                          is an ARP cache hit, the MAC address corresponding to
 *                          the IP address will be stored.
 * @param[in,out] ppxEndPoint a pointer to the end-point will be stored.
 *
 * @return When the IP-address is found: eResolutionCacheHit, when not found: eResolutionCacheMiss,
 *         and when waiting for a ARP reply: eResolutionFailed.
 */
    static eResolutionLookupResult_t prvCacheLookup( uint32_t ulAddressToLookup,
                                                     MACAddress_t * const pxMACAddress,
                                                     NetworkEndPoint_t ** ppxEndPoint )
    {
        BaseType_t x;
        eResolutionLookupResult_t eReturn = eResolutionCacheMiss;

        /* Loop through each entry in the ARP cache. */
        for( x = 0; x < ipconfigARP_CACHE_ENTRIES; x++ )
        {
            /* Does this row in the ARP cache table hold an entry for the IP address
             * being queried? */
            if( xARPCache[ x ].ulIPAddress == ulAddressToLookup )
            {
                /* A matching valid entry was found. */
                if( xARPCache[ x ].ucValid == ( uint8_t ) pdFALSE )
                {
                    /* This entry is waiting an ARP reply, so is not valid. */
                    eReturn = eResolutionFailed;
                }
                else
                {
                    /* A valid entry was found. */
                    ( void ) memcpy( pxMACAddress->ucBytes, xARPCache[ x ].xMACAddress.ucBytes, sizeof( MACAddress_t ) );
                    /* ppxEndPoint != NULL was tested in the only caller eARPGetCacheEntry(). */
                    *( ppxEndPoint ) = xARPCache[ x ].pxEndPoint;
                    eReturn = eResolutionCacheHit;
                }

                break;
            }
        }

        return eReturn;
    }
/*-----------------------------------------------------------*/

/**
 * @brief A call to this function will update (or 'Age') the ARP cache entries.
 *        The function will also try to prevent a removal of entry by sending
 *        an ARP query. It will also check whether we are waiting on an ARP
 *        reply - if we are, then an ARP request will be re-sent.
 *        In case an ARP entry has 'Aged' to 0, it will be removed from the ARP
 *        cache.
 */
    void vARPAgeCache( void )
    {
        BaseType_t x;
        TickType_t xTimeNow;

        /* Loop through each entry in the ARP cache. */
        for( x = 0; x < ipconfigARP_CACHE_ENTRIES; x++ )
        {
            /* If the entry is valid (its age is greater than zero). */
            if( xARPCache[ x ].ucAge > 0U )
            {
                /* Decrement the age value of the entry in this ARP cache table row.
                 * When the age reaches zero it is no longer considered valid. */
                ( xARPCache[ x ].ucAge )--;

                /* If the entry is not yet valid, then it is waiting an ARP
                 * reply, and the ARP request should be retransmitted. */
                if( xARPCache[ x ].ucValid == ( uint8_t ) pdFALSE )
                {
                    FreeRTOS_OutputARPRequest( xARPCache[ x ].ulIPAddress );
                }
                else if( xARPCache[ x ].ucAge <= ( uint8_t ) arpMAX_ARP_AGE_BEFORE_NEW_ARP_REQUEST )
                {
                    /* This entry will get removed soon.  See if the MAC address is
                     * still valid to prevent this happening. */
                    iptraceARP_TABLE_ENTRY_WILL_EXPIRE( xARPCache[ x ].ulIPAddress );
                    FreeRTOS_OutputARPRequest( xARPCache[ x ].ulIPAddress );
                }
                else
                {
                    /* The age has just ticked down, with nothing to do. */
                }

                if( xARPCache[ x ].ucAge == 0U )
                {
                    /* The entry is no longer valid.  Wipe it out. */
                    iptraceARP_TABLE_ENTRY_EXPIRED( xARPCache[ x ].ulIPAddress );
                    xARPCache[ x ].ulIPAddress = 0U;
                }
            }
        }

        xTimeNow = xTaskGetTickCount();

        if( ( xLastGratuitousARPTime == ( TickType_t ) 0 ) || ( ( xTimeNow - xLastGratuitousARPTime ) > ( TickType_t ) arpGRATUITOUS_ARP_PERIOD ) )
        {
            NetworkEndPoint_t * pxEndPoint = pxNetworkEndPoints;

            while( pxEndPoint != NULL )
            {
                if( ( pxEndPoint->bits.bEndPointUp != pdFALSE_UNSIGNED ) && ( pxEndPoint->ipv4_settings.ulIPAddress != 0U ) )
                {
                    if( pxEndPoint->bits.bIPv6 == pdFALSE_UNSIGNED ) /* LCOV_EXCL_BR_LINE */
                    {
                        FreeRTOS_OutputARPRequest_Multi( pxEndPoint, pxEndPoint->ipv4_settings.ulIPAddress );
                    }
                }

                pxEndPoint = pxEndPoint->pxNext;
            }

            xLastGratuitousARPTime = xTimeNow;
        }
    }
/*-----------------------------------------------------------*/

/**
 * @brief Send a Gratuitous ARP packet to allow this node to announce the IP-MAC
 *        mapping to the entire network.
 */
    void vARPSendGratuitous( void )
    {
        /* Setting xLastGratuitousARPTime to 0 will force a gratuitous ARP the next
         * time vARPAgeCache() is called. */
        xLastGratuitousARPTime = ( TickType_t ) 0;

        /* Let the IP-task call vARPAgeCache(). */
        ( void ) xSendEventToIPTask( eARPTimerEvent );
    }

/*-----------------------------------------------------------*/

/**
 * @brief Create and send an ARP request packet to given IPv4 endpoint.
 *
 * @param[in] pxEndPoint Endpoint through which the requests should be sent.
 * @param[in] ulIPAddress A 32-bit representation of the IP-address whose
 *                         physical (MAC) address is required.
 */
    void FreeRTOS_OutputARPRequest_Multi( NetworkEndPoint_t * pxEndPoint,
                                          uint32_t ulIPAddress )
    {
        NetworkBufferDescriptor_t * pxNetworkBuffer;

        if( ( pxEndPoint->bits.bIPv6 == pdFALSE_UNSIGNED ) &&
            ( pxEndPoint->ipv4_settings.ulIPAddress != 0U ) )
        {
            /* This is called from the context of the IP event task, so a block time
             * must not be used. */
            pxNetworkBuffer = pxGetNetworkBufferWithDescriptor( sizeof( ARPPacket_t ), ( TickType_t ) 0U );

            if( pxNetworkBuffer != NULL )
            {
                pxNetworkBuffer->xIPAddress.ulIP_IPv4 = ulIPAddress;
                pxNetworkBuffer->pxEndPoint = pxEndPoint;
                pxNetworkBuffer->pxInterface = pxEndPoint->pxNetworkInterface;
                vARPGenerateRequestPacket( pxNetworkBuffer );

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

                if( xIsCallingFromIPTask() != pdFALSE )
                {
                    iptraceNETWORK_INTERFACE_OUTPUT( pxNetworkBuffer->xDataLength, pxNetworkBuffer->pucEthernetBuffer );

                    /* Only the IP-task is allowed to call this function directly. */
                    if( pxEndPoint->pxNetworkInterface != NULL )
                    {
                        ( void ) pxEndPoint->pxNetworkInterface->pfOutput( pxEndPoint->pxNetworkInterface, pxNetworkBuffer, pdTRUE );
                    }
                }
                else
                {
                    IPStackEvent_t xSendEvent;

                    /* Send a message to the IP-task to send this ARP packet. */
                    xSendEvent.eEventType = eNetworkTxEvent;
                    xSendEvent.pvData = pxNetworkBuffer;

                    if( xSendEventStructToIPTask( &xSendEvent, ( TickType_t ) portMAX_DELAY ) == pdFAIL )
                    {
                        /* Failed to send the message, so release the network buffer. */
                        vReleaseNetworkBufferAndDescriptor( pxNetworkBuffer );
                    }
                }
            }
        }
    }

/*-----------------------------------------------------------*/

/**
 * @brief Create and send an ARP request packet.
 *
 * @param[in] ulIPAddress A 32-bit representation of the IP-address whose
 *                         physical (MAC) address is required.
 */
    void FreeRTOS_OutputARPRequest( uint32_t ulIPAddress )
    {
        /* Its assumed that IPv4 endpoints belonging to different physical interface
         * in the system will have a different subnet, but endpoints on same interface
         * may have it. */
        NetworkEndPoint_t * pxEndPoint = FreeRTOS_FindEndPointOnNetMask( ulIPAddress );

        if( pxEndPoint != NULL )
        {
            FreeRTOS_OutputARPRequest_Multi( pxEndPoint, ulIPAddress );
        }
    }
/*-----------------------------------------------------------*/

/**
 * @brief  Wait for address resolution: look-up the IP-address in the ARP-cache, and if
 *         needed send an ARP request, and wait for a reply.  This function is useful when
 *         called before FreeRTOS_sendto().
 *
 * @param[in] ulIPAddress The IP-address to look-up.
 * @param[in] uxTicksToWait The maximum number of clock ticks to wait for a reply.
 *
 * @return Zero when successful.
 */
    BaseType_t xARPWaitResolution( uint32_t ulIPAddress,
                                   TickType_t uxTicksToWait )
    {
        BaseType_t xResult = -pdFREERTOS_ERRNO_EADDRNOTAVAIL;
        TimeOut_t xTimeOut;
        MACAddress_t xMACAddress;
        eResolutionLookupResult_t xLookupResult;
        NetworkEndPoint_t * pxEndPoint;
        size_t uxSendCount = ipconfigMAX_ARP_RETRANSMISSIONS;
        uint32_t ulIPAddressCopy = ulIPAddress;

        /* The IP-task is not supposed to call this function. */
        configASSERT( xIsCallingFromIPTask() == pdFALSE );

        xLookupResult = eARPGetCacheEntry( &( ulIPAddressCopy ), &( xMACAddress ), &( pxEndPoint ) );

        if( xLookupResult == eResolutionCacheMiss )
        {
            const TickType_t uxSleepTime = pdMS_TO_TICKS( 250U );

            /* We might use ipconfigMAX_ARP_RETRANSMISSIONS here. */
            vTaskSetTimeOutState( &xTimeOut );

            while( uxSendCount > 0U )
            {
                FreeRTOS_OutputARPRequest( ulIPAddressCopy );

                vTaskDelay( uxSleepTime );

                xLookupResult = eARPGetCacheEntry( &( ulIPAddressCopy ), &( xMACAddress ), &( pxEndPoint ) );

                if( ( xTaskCheckForTimeOut( &( xTimeOut ), &( uxTicksToWait ) ) == pdTRUE ) ||
                    ( xLookupResult != eResolutionCacheMiss ) )
                {
                    break;
                }

                /* Decrement the count. */
                uxSendCount--;
            }
        }

        if( xLookupResult == eResolutionCacheHit )
        {
            xResult = 0;
        }

        return xResult;
    }
/*-----------------------------------------------------------*/

/**
 * @brief Generate an ARP request packet by copying various constant details to
 *        the buffer.
 *
 * @param[in,out] pxNetworkBuffer Pointer to the buffer which has to be filled with
 *                             the ARP request packet details.
 */
    void vARPGenerateRequestPacket( NetworkBufferDescriptor_t * const pxNetworkBuffer )
    {
/* Part of the Ethernet and ARP headers are always constant when sending an IPv4
 * ARP packet.  This array defines the constant parts, allowing this part of the
 * packet to be filled in using a simple memcpy() instead of individual writes. */
        static const uint8_t xDefaultPartARPPacketHeader[] =
        {
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, /* Ethernet destination address. */
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* Ethernet source address. */
            0x08, 0x06,                         /* Ethernet frame type (ipARP_FRAME_TYPE). */
            0x00, 0x01,                         /* usHardwareType (ipARP_HARDWARE_TYPE_ETHERNET). */
            0x08, 0x00,                         /* usProtocolType. */
            ipMAC_ADDRESS_LENGTH_BYTES,         /* ucHardwareAddressLength. */
            ipIP_ADDRESS_LENGTH_BYTES,          /* ucProtocolAddressLength. */
            0x00, 0x01,                         /* usOperation (ipARP_REQUEST). */
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* xSenderHardwareAddress. */
            0x00, 0x00, 0x00, 0x00,             /* ulSenderProtocolAddress. */
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00  /* xTargetHardwareAddress. */
        };

        ARPPacket_t * pxARPPacket;

/* memcpy() helper variables for MISRA Rule 21.15 compliance*/
        const void * pvCopySource;
        void * pvCopyDest;

        /* Buffer allocation ensures that buffers always have space
         * for an ARP packet. See buffer allocation implementations 1
         * and 2 under portable/BufferManagement. */
        configASSERT( pxNetworkBuffer != NULL );
        configASSERT( pxNetworkBuffer->xDataLength >= sizeof( ARPPacket_t ) );
        configASSERT( pxNetworkBuffer->pxEndPoint != NULL );

        /* MISRA Ref 11.3.1 [Misaligned access] */
/* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
        /* coverity[misra_c_2012_rule_11_3_violation] */
        pxARPPacket = ( ( ARPPacket_t * ) pxNetworkBuffer->pucEthernetBuffer );

        /* memcpy the const part of the header information into the correct
         * location in the packet.  This copies:
         *  xEthernetHeader.ulDestinationAddress
         *  xEthernetHeader.usFrameType;
         *  xARPHeader.usHardwareType;
         *  xARPHeader.usProtocolType;
         *  xARPHeader.ucHardwareAddressLength;
         *  xARPHeader.ucProtocolAddressLength;
         *  xARPHeader.usOperation;
         *  xARPHeader.xTargetHardwareAddress;
         */

        /*
         * Use helper variables for memcpy() to remain
         * compliant with MISRA Rule 21.15.  These should be
         * optimized away.
         */
        pvCopySource = xDefaultPartARPPacketHeader;
        pvCopyDest = pxARPPacket;
        ( void ) memcpy( pvCopyDest, pvCopySource, sizeof( xDefaultPartARPPacketHeader ) );

        pvCopySource = pxNetworkBuffer->pxEndPoint->xMACAddress.ucBytes;
        pvCopyDest = pxARPPacket->xEthernetHeader.xSourceAddress.ucBytes;
        ( void ) memcpy( pvCopyDest, pvCopySource, ipMAC_ADDRESS_LENGTH_BYTES );

        pvCopySource = pxNetworkBuffer->pxEndPoint->xMACAddress.ucBytes;
        pvCopyDest = pxARPPacket->xARPHeader.xSenderHardwareAddress.ucBytes;
        ( void ) memcpy( pvCopyDest, pvCopySource, ipMAC_ADDRESS_LENGTH_BYTES );

        pvCopySource = &( pxNetworkBuffer->pxEndPoint->ipv4_settings.ulIPAddress );
        pvCopyDest = pxARPPacket->xARPHeader.ucSenderProtocolAddress;
        ( void ) memcpy( pvCopyDest, pvCopySource, sizeof( pxARPPacket->xARPHeader.ucSenderProtocolAddress ) );
        pxARPPacket->xARPHeader.ulTargetProtocolAddress = pxNetworkBuffer->xIPAddress.ulIP_IPv4;

        pxNetworkBuffer->xDataLength = sizeof( ARPPacket_t );

        iptraceCREATING_ARP_REQUEST( pxNetworkBuffer->xIPAddress.ulIP_IPv4 );
    }
/*-----------------------------------------------------------*/

/**
 * @brief A call to this function will clear the ARP cache.
 * @param[in] pxEndPoint only clean entries with this end-point, or when NULL,
 *                        clear the entire ARP cache.
 */
    void FreeRTOS_ClearARP( const struct xNetworkEndPoint * pxEndPoint )
    {
        if( pxEndPoint != NULL )
        {
            BaseType_t x;

            for( x = 0; x < ipconfigARP_CACHE_ENTRIES; x++ )
            {
                if( xARPCache[ x ].pxEndPoint == pxEndPoint )
                {
                    ( void ) memset( &( xARPCache[ x ] ), 0, sizeof( ARPCacheRow_t ) );
                }
            }
        }
        else
        {
            ( void ) memset( xARPCache, 0, sizeof( xARPCache ) );
        }
    }
/*-----------------------------------------------------------*/

    #if ( ipconfigHAS_PRINTF != 0 ) || ( ipconfigHAS_DEBUG_PRINTF != 0 )

        void FreeRTOS_PrintARPCache( void )
        {
            BaseType_t x, xCount = 0;

            /* Loop through each entry in the ARP cache. */
            for( x = 0; x < ipconfigARP_CACHE_ENTRIES; x++ )
            {
                if( ( xARPCache[ x ].ulIPAddress != 0U ) && ( xARPCache[ x ].ucAge > ( uint8_t ) 0U ) )
                {
                    /* See if the MAC-address also matches, and we're all happy */
                    FreeRTOS_printf( ( "ARP %2d: %3u - %16xip : %02x:%02x:%02x : %02x:%02x:%02x\n",
                                       ( int ) x,
                                       xARPCache[ x ].ucAge,
                                       ( unsigned ) FreeRTOS_ntohl( xARPCache[ x ].ulIPAddress ),
                                       xARPCache[ x ].xMACAddress.ucBytes[ 0 ],
                                       xARPCache[ x ].xMACAddress.ucBytes[ 1 ],
                                       xARPCache[ x ].xMACAddress.ucBytes[ 2 ],
                                       xARPCache[ x ].xMACAddress.ucBytes[ 3 ],
                                       xARPCache[ x ].xMACAddress.ucBytes[ 4 ],
                                       xARPCache[ x ].xMACAddress.ucBytes[ 5 ] ) );
                    xCount++;
                }
            }

            FreeRTOS_printf( ( "Arp has %ld entries\n", xCount ) );
        }
    #endif /* ( ipconfigHAS_PRINTF != 0 ) || ( ipconfigHAS_DEBUG_PRINTF != 0 ) */

#endif /* ( ipconfigUSE_IPv4 != 0 ) */
