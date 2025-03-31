/*
 * FreeRTOS+TCP V4.2.1
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
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
 * @file FreeRTOS_ND.c
 * @brief Implements a few functions that handle Neighbour Discovery and other ICMPv6 messages.
 */

/* Standard includes. */
#include <stdint.h>
#include <stdio.h>


/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"

/* FreeRTOS+TCP includes. */
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"
#include "FreeRTOS_IP_Private.h"
#include "FreeRTOS_ARP.h"
#include "FreeRTOS_UDP_IP.h"
#include "FreeRTOS_Routing.h"
#include "FreeRTOS_ND.h"
#include "FreeRTOS_IP_Timers.h"

#if ( ipconfigUSE_LLMNR == 1 )
    #include "FreeRTOS_DNS.h"
#endif /* ipconfigUSE_LLMNR */
#include "NetworkBufferManagement.h"

/* The entire module FreeRTOS_ND.c is skipped when IPv6 is not used. */
#if ( ipconfigUSE_IPv6 != 0 )

/** @brief Type of Neighbour Advertisement packets - SOLICIT. */
    #define ndICMPv6_FLAG_SOLICITED                       0x40000000U
/** @brief Type of Neighbour Advertisement packets - UPDATE. */
    #define ndICMPv6_FLAG_UPDATE                          0x20000000U

/** @brief A block time of 0 simply means "don't block". */
    #define ndDONT_BLOCK                                  ( ( TickType_t ) 0 )

/** @brief The character used to fill ICMP echo requests, and therefore also the
 *         character expected to fill ICMP echo replies.
 */
    #define ndECHO_DATA_FILL_BYTE                         'x'

/** @brief When ucAge becomes 3 or less, it is time for a new
 * neighbour solicitation.
 */
    #define ndMAX_CACHE_AGE_BEFORE_NEW_ND_SOLICITATION    ( 3U )

/** @brief All nodes on the local network segment: IP address. */
    const uint8_t pcLOCAL_ALL_NODES_MULTICAST_IP[ ipSIZE_OF_IPv6_ADDRESS ] = { 0xffU, 0x02U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x01U }; /* ff02::1 */
/** @brief All nodes on the local network segment: MAC address. */
    const uint8_t pcLOCAL_ALL_NODES_MULTICAST_MAC[ ipMAC_ADDRESS_LENGTH_BYTES ] = { 0x33U, 0x33U, 0x00U, 0x00U, 0x00U, 0x01U };

/** @brief See if the MAC-address can be resolved because it is a multi-cast address. */
    static eARPLookupResult_t prvMACResolve( const IPv6_Address_t * pxAddressToLookup,
                                             MACAddress_t * const pxMACAddress,
                                             NetworkEndPoint_t ** ppxEndPoint );

/** @brief Lookup an MAC address in the ND cache from the IP address. */
    static eARPLookupResult_t prvNDCacheLookup( const IPv6_Address_t * pxAddressToLookup,
                                                MACAddress_t * const pxMACAddress,
                                                NetworkEndPoint_t ** ppxEndPoint );

    #if ( ipconfigHAS_PRINTF == 1 )
        static const char * pcMessageType( BaseType_t xType );
    #endif

/** @brief Find the first end-point of type IPv6. */
    static NetworkEndPoint_t * pxFindLocalEndpoint( void );

/** @brief The ND cache. */
    static NDCacheRow_t xNDCache[ ipconfigND_CACHE_ENTRIES ];

/*-----------------------------------------------------------*/

/*
 *  ff02::1: All IPv6 devices
 *  ff02::2: All IPv6 routers
 *  ff02::5: All OSPFv3 routers
 *  ff02::a: All EIGRP (IPv6) routers
 */

/**
 * @brief Find the first end-point of type IPv6.
 *
 * @return The first IPv6 end-point found.
 */
    static NetworkEndPoint_t * pxFindLocalEndpoint( void )
    {
        NetworkEndPoint_t * pxEndPoint;

        for( pxEndPoint = FreeRTOS_FirstEndPoint( NULL );
             pxEndPoint != NULL;
             pxEndPoint = FreeRTOS_NextEndPoint( NULL, pxEndPoint ) )
        {
            if( pxEndPoint->bits.bIPv6 == pdTRUE_UNSIGNED )
            {
                IPv6_Type_t eType = xIPv6_GetIPType( &( pxEndPoint->ipv6_settings.xIPAddress ) );

                if( eType == eIPv6_LinkLocal )
                {
                    break;
                }
            }
        }

        return pxEndPoint;
    }

/**
 * @brief See if the MAC-address can be resolved because it is a multi-cast address.
 *
 * @param[in] pxAddressToLookup The IP-address to look-up.
 * @param[out] pxMACAddress The resulting MAC-address is stored here.
 * @param[out] ppxEndPoint A pointer to an end-point pointer where the end-point will be stored.
 *
 * @return An enum, either eARPCacheHit or eARPCacheMiss.
 */
    static eARPLookupResult_t prvMACResolve( const IPv6_Address_t * pxAddressToLookup,
                                             MACAddress_t * const pxMACAddress,
                                             NetworkEndPoint_t ** ppxEndPoint )
    {
        eARPLookupResult_t eReturn;

        /* Mostly used multi-cast address is ff02::. */
        if( xIsIPv6AllowedMulticast( pxAddressToLookup ) != pdFALSE )
        {
            vSetMultiCastIPv6MacAddress( pxAddressToLookup, pxMACAddress );

            if( ppxEndPoint != NULL )
            {
                *ppxEndPoint = pxFindLocalEndpoint();
            }

            eReturn = eARPCacheHit;
        }
        else
        {
            /* Not a multicast IP address. */
            eReturn = eARPCacheMiss;
        }

        return eReturn;
    }
/*-----------------------------------------------------------*/

/**
 * @brief Find the MAC-address of an IPv6 address.  It will first determine if is a multicast
 *        address, if not, it will check the ND cache.
 *
 * @param[in] pxIPAddress The IPv6 address to be looked up.
 * @param[out] pxMACAddress The MAC-address found.
 * @param[out] ppxEndPoint A pointer to a pointer to an end-point, where the end-point will be stored.
 *
 * @return An enum which says whether the address was found: eARPCacheHit or eARPCacheMiss.
 */
    eARPLookupResult_t eNDGetCacheEntry( IPv6_Address_t * pxIPAddress,
                                         MACAddress_t * const pxMACAddress,
                                         struct xNetworkEndPoint ** ppxEndPoint )
    {
        eARPLookupResult_t eReturn;
        NetworkEndPoint_t * pxEndPoint;

        /* Multi-cast addresses can be resolved immediately. */
        eReturn = prvMACResolve( pxIPAddress, pxMACAddress, ppxEndPoint );

        if( eReturn == eARPCacheMiss )
        {
            /* See if the IP-address has an entry in the cache. */
            eReturn = prvNDCacheLookup( pxIPAddress, pxMACAddress, ppxEndPoint );
        }

        if( eReturn == eARPCacheMiss )
        {
            FreeRTOS_printf( ( "eNDGetCacheEntry: lookup %pip miss\n", ( void * ) pxIPAddress->ucBytes ) );
        }

        if( eReturn == eARPCacheMiss )
        {
            IPv6_Type_t eIPType = xIPv6_GetIPType( pxIPAddress );

            pxEndPoint = FreeRTOS_FindEndPointOnIP_IPv6( pxIPAddress );

            if( pxEndPoint != NULL )
            {
                if( ppxEndPoint != NULL )
                {
                    *( ppxEndPoint ) = pxEndPoint;
                }

                FreeRTOS_printf( ( "eNDGetCacheEntry: FindEndPointOnIP failed for %pip (endpoint %pip)\n",
                                   ( void * ) pxIPAddress->ucBytes,
                                   ( void * ) pxEndPoint->ipv6_settings.xIPAddress.ucBytes ) );
            }
            else
            {
                if( eIPType == eIPv6_LinkLocal )
                {
                    for( pxEndPoint = FreeRTOS_FirstEndPoint( NULL );
                         pxEndPoint != NULL;
                         pxEndPoint = FreeRTOS_NextEndPoint( NULL, pxEndPoint ) )
                    {
                        IPv6_Type_t eMyType = xIPv6_GetIPType( &( pxEndPoint->ipv6_settings.xIPAddress ) );

                        if( eMyType == eIPType )
                        {
                            eReturn = prvNDCacheLookup( pxIPAddress, pxMACAddress, ppxEndPoint );
                            break;
                        }
                    }

                    FreeRTOS_printf( ( "eNDGetCacheEntry: LinkLocal %pip \"%s\"\n", ( void * ) pxIPAddress->ucBytes,
                                       ( eReturn == eARPCacheHit ) ? "hit" : "miss" ) );
                }
                else
                {
                    pxEndPoint = FreeRTOS_FindGateWay( ( BaseType_t ) ipTYPE_IPv6 );

                    if( pxEndPoint != NULL )
                    {
                        ( void ) memcpy( pxIPAddress->ucBytes, pxEndPoint->ipv6_settings.xGatewayAddress.ucBytes, ipSIZE_OF_IPv6_ADDRESS );
                        FreeRTOS_printf( ( "eNDGetCacheEntry: Using gw %pip\n", ( void * ) pxIPAddress->ucBytes ) );
                        FreeRTOS_printf( ( "eNDGetCacheEntry: From addr %pip\n", ( void * ) pxEndPoint->ipv6_settings.xIPAddress.ucBytes ) );

                        /* See if the gateway has an entry in the cache. */
                        eReturn = prvNDCacheLookup( pxIPAddress, pxMACAddress, ppxEndPoint );

                        if( *ppxEndPoint != NULL )
                        {
                            FreeRTOS_printf( ( "eNDGetCacheEntry: found end-point %pip\n", ( void * ) ( *ppxEndPoint )->ipv6_settings.xIPAddress.ucBytes ) );
                        }

                        *( ppxEndPoint ) = pxEndPoint;
                    }
                }
            }
        }

        return eReturn;
    }
/*-----------------------------------------------------------*/

/**
 * @brief Store a combination of IP-address, MAC-address and an end-point in a free location
 *        in the ND cache.
 *
 * @param[in] pxMACAddress The MAC-address
 * @param[in] pxIPAddress The IP-address
 * @param[in] pxEndPoint The end-point through which the IP-address can be reached.
 *
 */
    void vNDRefreshCacheEntry( const MACAddress_t * pxMACAddress,
                               const IPv6_Address_t * pxIPAddress,
                               NetworkEndPoint_t * pxEndPoint )
    {
        BaseType_t x;
        BaseType_t xFreeEntry = -1, xEntryFound = -1;
        uint16_t xOldestValue = ipconfigMAX_ARP_AGE + 1;
        BaseType_t xOldestEntry = 0;

        /* For each entry in the ND cache table. */
        for( x = 0; x < ipconfigND_CACHE_ENTRIES; x++ )
        {
            if( xNDCache[ x ].ucValid == ( uint8_t ) pdFALSE )
            {
                if( xFreeEntry == -1 )
                {
                    xFreeEntry = x;
                }
            }
            else if( memcmp( xNDCache[ x ].xIPAddress.ucBytes, pxIPAddress->ucBytes, ipSIZE_OF_IPv6_ADDRESS ) == 0 )
            {
                xEntryFound = x;
                break;
            }
            else
            {
                /* Entry is valid but the IP-address doesn't match. */

                /* Keep track of the oldest entry in case we need to overwrite it. The problem we are trying to avoid is
                 * that there may be a queued packet in pxARPWaitingNetworkBuffer and we may have just received the
                 * neighbor advertisement needed for that packet. If we don't store this network advertisement in cache,
                 * the parting of the frame from pxARPWaitingNetworkBuffer will cause the sending of neighbor solicitation
                 * and stores the frame in pxARPWaitingNetworkBuffer. This becomes a vicious circle with thousands of
                 * neighbor solicitation/advertisement packets going back and forth because the ND cache is full.
                 * Overwriting the oldest cache entry is not a fool-proof solution, but it's something. */
                if( xNDCache[ x ].ucAge < xOldestValue )
                {
                    xOldestValue = xNDCache[ x ].ucAge;
                    xOldestEntry = x;
                }
            }
        }

        if( xEntryFound < 0 )
        {
            /* The IP-address was not found, use the first free location. */
            if( xFreeEntry >= 0 )
            {
                xEntryFound = xFreeEntry;
            }
            else
            {
                /* No free location. Overwrite the oldest. */
                xEntryFound = xOldestEntry;
                FreeRTOS_printf( ( "vNDRefreshCacheEntry: Cache FULL! Overwriting oldest entry %i with %02X-%02X-%02X-%02X-%02X-%02X\n", ( int ) xEntryFound, pxMACAddress->ucBytes[ 0 ], pxMACAddress->ucBytes[ 1 ], pxMACAddress->ucBytes[ 2 ], pxMACAddress->ucBytes[ 3 ], pxMACAddress->ucBytes[ 4 ], pxMACAddress->ucBytes[ 5 ] ) );
            }
        }

        /* At this point, xEntryFound is always a valid index. */
        /* Copy the IP-address. */
        ( void ) memcpy( xNDCache[ xEntryFound ].xIPAddress.ucBytes, pxIPAddress->ucBytes, ipSIZE_OF_IPv6_ADDRESS );
        /* Copy the MAC-address. */
        ( void ) memcpy( xNDCache[ xEntryFound ].xMACAddress.ucBytes, pxMACAddress->ucBytes, sizeof( MACAddress_t ) );
        xNDCache[ xEntryFound ].pxEndPoint = pxEndPoint;
        xNDCache[ xEntryFound ].ucAge = ( uint8_t ) ipconfigMAX_ARP_AGE;
        xNDCache[ xEntryFound ].ucValid = ( uint8_t ) pdTRUE;
    }
/*-----------------------------------------------------------*/

/**
 * @brief Reduce the age counter in each entry within the ND cache.  An entry is no
 * longer considered valid and is deleted if its age reaches zero.
 * Just before getting to zero, 3 times a neighbour solicitation will be sent.
 */
    void vNDAgeCache( void )
    {
        BaseType_t x;

        /* Loop through each entry in the ND cache. */
        for( x = 0; x < ipconfigND_CACHE_ENTRIES; x++ )
        {
            BaseType_t xDoSolicitate = pdFALSE;

            /* If the entry is valid (its age is greater than zero). */
            if( xNDCache[ x ].ucAge > 0U )
            {
                /* Decrement the age value of the entry in this ND cache table row.
                 * When the age reaches zero it is no longer considered valid. */
                ( xNDCache[ x ].ucAge )--;

                if( xNDCache[ x ].ucAge == 0U )
                {
                    /* The entry is no longer valid.  Wipe it out. */
                    iptraceND_TABLE_ENTRY_EXPIRED( xNDCache[ x ].xIPAddress );
                    ( void ) memset( &( xNDCache[ x ] ), 0, sizeof( xNDCache[ x ] ) );
                }
                else
                {
                    /* If the entry is not yet valid, then it is waiting an ND
                     * advertisement, and the ND solicitation should be retransmitted. */
                    if( xNDCache[ x ].ucValid == ( uint8_t ) pdFALSE )
                    {
                        xDoSolicitate = pdTRUE;
                    }
                    else if( xNDCache[ x ].ucAge <= ( uint8_t ) ndMAX_CACHE_AGE_BEFORE_NEW_ND_SOLICITATION )
                    {
                        /* This entry will get removed soon.  See if the MAC address is
                         * still valid to prevent this happening. */
                        iptraceND_TABLE_ENTRY_WILL_EXPIRE( xNDCache[ x ].xIPAddress );
                        xDoSolicitate = pdTRUE;
                    }
                    else
                    {
                        /* The age has just ticked down, with nothing to do. */
                    }

                    if( xDoSolicitate != pdFALSE )
                    {
                        size_t uxNeededSize;
                        NetworkBufferDescriptor_t * pxNetworkBuffer;

                        uxNeededSize = ipSIZE_OF_ETH_HEADER + ipSIZE_OF_IPv6_HEADER + sizeof( ICMPHeader_IPv6_t );
                        pxNetworkBuffer = pxGetNetworkBufferWithDescriptor( uxNeededSize, 0U );

                        if( pxNetworkBuffer != NULL )
                        {
                            pxNetworkBuffer->pxEndPoint = xNDCache[ x ].pxEndPoint;
                            /* _HT_ From here I am suspecting a network buffer leak */
                            vNDSendNeighbourSolicitation( pxNetworkBuffer, &( xNDCache[ x ].xIPAddress ) );
                        }
                    }
                }
            }
        }
    }
/*-----------------------------------------------------------*/

/**
 * @brief Clear the Neighbour Discovery cache.
 */
    void FreeRTOS_ClearND( void )
    {
        ( void ) memset( xNDCache, 0, sizeof( xNDCache ) );
    }
/*-----------------------------------------------------------*/

/**
 * @brief Look-up an IPv6 address in the cache.
 *
 * @param[in] pxAddressToLookup The IPv6 address to look-up.Ethernet packet.
 * @param[out] pxMACAddress The resulting MAC-address will be stored here.
 * @param[out] ppxEndPoint A pointer to a pointer to an end-point, where the end-point will be stored.
 *
 * @return An enum: either eARPCacheHit or eARPCacheMiss.
 */
    static eARPLookupResult_t prvNDCacheLookup( const IPv6_Address_t * pxAddressToLookup,
                                                MACAddress_t * const pxMACAddress,
                                                NetworkEndPoint_t ** ppxEndPoint )
    {
        BaseType_t x;
        eARPLookupResult_t eReturn = eARPCacheMiss;

        /* For each entry in the ND cache table. */
        for( x = 0; x < ipconfigND_CACHE_ENTRIES; x++ )
        {
            if( xNDCache[ x ].ucValid == ( uint8_t ) pdFALSE )
            {
                /* Skip invalid entries. */
            }
            else if( memcmp( xNDCache[ x ].xIPAddress.ucBytes, pxAddressToLookup->ucBytes, ipSIZE_OF_IPv6_ADDRESS ) == 0 )
            {
                ( void ) memcpy( pxMACAddress->ucBytes, xNDCache[ x ].xMACAddress.ucBytes, sizeof( MACAddress_t ) );
                eReturn = eARPCacheHit;

                if( ppxEndPoint != NULL )
                {
                    *ppxEndPoint = xNDCache[ x ].pxEndPoint;
                }

                FreeRTOS_debug_printf( ( "prvCacheLookup6[ %d ] %pip with %02x:%02x:%02x:%02x:%02x:%02x\n",
                                         ( int ) x,
                                         ( void * ) pxAddressToLookup->ucBytes,
                                         pxMACAddress->ucBytes[ 0 ],
                                         pxMACAddress->ucBytes[ 1 ],
                                         pxMACAddress->ucBytes[ 2 ],
                                         pxMACAddress->ucBytes[ 3 ],
                                         pxMACAddress->ucBytes[ 4 ],
                                         pxMACAddress->ucBytes[ 5 ] ) );
                break;
            }
            else
            {
                /* Entry is valid but the MAC-address doesn't match. */
            }
        }

        if( eReturn == eARPCacheMiss )
        {
            FreeRTOS_printf( ( "prvNDCacheLookup %pip Miss\n", ( void * ) pxAddressToLookup->ucBytes ) );

            if( ppxEndPoint != NULL )
            {
                *ppxEndPoint = NULL;
            }
        }

        return eReturn;
    }
/*-----------------------------------------------------------*/

    #if ( ( ipconfigHAS_PRINTF != 0 ) || ( ipconfigHAS_DEBUG_PRINTF != 0 ) )

/**
 * @brief Print the contents of the ND cache, for debugging only.
 */
        void FreeRTOS_PrintNDCache( void )
        {
            BaseType_t x, xCount = 0;
            char pcBuffer[ 40 ];

            /* Loop through each entry in the ND cache. */
            for( x = 0; x < ipconfigND_CACHE_ENTRIES; x++ )
            {
                if( xNDCache[ x ].ucValid != ( uint8_t ) 0U )
                {
                    /* See if the MAC-address also matches, and we're all happy */

                    FreeRTOS_printf( ( "ND %2d: age %3u - %pip MAC %02x-%02x-%02x-%02x-%02x-%02x endPoint %s\n",
                                       ( int ) x,
                                       xNDCache[ x ].ucAge,
                                       ( void * ) xNDCache[ x ].xIPAddress.ucBytes,
                                       xNDCache[ x ].xMACAddress.ucBytes[ 0 ],
                                       xNDCache[ x ].xMACAddress.ucBytes[ 1 ],
                                       xNDCache[ x ].xMACAddress.ucBytes[ 2 ],
                                       xNDCache[ x ].xMACAddress.ucBytes[ 3 ],
                                       xNDCache[ x ].xMACAddress.ucBytes[ 4 ],
                                       xNDCache[ x ].xMACAddress.ucBytes[ 5 ],
                                       pcEndpointName( xNDCache[ x ].pxEndPoint, pcBuffer, sizeof( pcBuffer ) ) ) );
                    xCount++;
                }
            }

            FreeRTOS_printf( ( "Arp has %ld entries\n", xCount ) );
        }

    #endif /* ( ipconfigHAS_PRINTF != 0 ) || ( ipconfigHAS_DEBUG_PRINTF != 0 ) */
/*-----------------------------------------------------------*/

/**
 * @brief Return an ICMPv6 packet to the peer.
 *
 * @param[in] pxNetworkBuffer The Ethernet packet.
 * @param[in] uxICMPSize The number of bytes to be sent.
 */
    static void prvReturnICMP_IPv6( NetworkBufferDescriptor_t * const pxNetworkBuffer,
                                    size_t uxICMPSize )
    {
        const NetworkEndPoint_t * pxEndPoint = pxNetworkBuffer->pxEndPoint;

        /* MISRA Ref 11.3.1 [Misaligned access] */
        /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
        /* coverity[misra_c_2012_rule_11_3_violation] */
        ICMPPacket_IPv6_t * pxICMPPacket = ( ( ICMPPacket_IPv6_t * ) pxNetworkBuffer->pucEthernetBuffer );

        ( void ) memcpy( pxICMPPacket->xIPHeader.xDestinationAddress.ucBytes, pxICMPPacket->xIPHeader.xSourceAddress.ucBytes, ipSIZE_OF_IPv6_ADDRESS );
        ( void ) memcpy( pxICMPPacket->xIPHeader.xSourceAddress.ucBytes, pxEndPoint->ipv6_settings.xIPAddress.ucBytes, ipSIZE_OF_IPv6_ADDRESS );
        pxICMPPacket->xIPHeader.usPayloadLength = FreeRTOS_htons( uxICMPSize );

        /* Important: tell NIC driver how many bytes must be sent */
        pxNetworkBuffer->xDataLength = ( size_t ) ( ipSIZE_OF_ETH_HEADER + ipSIZE_OF_IPv6_HEADER + uxICMPSize );

        #if ( ipconfigDRIVER_INCLUDED_TX_IP_CHECKSUM == 0 )
        {
            /* calculate the ICMPv6 checksum for outgoing package */
            ( void ) usGenerateProtocolChecksum( pxNetworkBuffer->pucEthernetBuffer, pxNetworkBuffer->xDataLength, pdTRUE );
        }
        #else
        {
            /* Many EMAC peripherals will only calculate the ICMP checksum
             * correctly if the field is nulled beforehand. */
            pxICMPPacket->xICMPHeaderIPv6.usChecksum = 0;
        }
        #endif

        /* This function will fill in the Ethernet addresses and send the packet */
        vReturnEthernetFrame( pxNetworkBuffer, pdFALSE );
    }
/*-----------------------------------------------------------*/

/**
 * @brief Send out an ND request for the IPv6 address contained in pxNetworkBuffer, and
 *        add an entry into the ND table that indicates that an ND reply is outstanding
 *        so re-transmissions can be generated.
 *
 * @param[in] pxNetworkBuffer The network buffer in which the message shall be stored.
 * @param[in] pxIPAddress The IPv6 address that is asked to send a Neighbour Advertisement.
 *
 * @note Send out an ND request for the IPv6 address contained in pxNetworkBuffer, and
 * add an entry into the ND table that indicates that an ND reply is
 * outstanding so re-transmissions can be generated.
 */

    void vNDSendNeighbourSolicitation( NetworkBufferDescriptor_t * pxNetworkBuffer,
                                       const IPv6_Address_t * pxIPAddress )
    {
        ICMPPacket_IPv6_t * pxICMPPacket;
        ICMPHeader_IPv6_t * pxICMPHeader_IPv6;
        const NetworkEndPoint_t * pxEndPoint = pxNetworkBuffer->pxEndPoint;
        size_t uxNeededSize;
        IPv6_Address_t xTargetIPAddress;
        MACAddress_t xMultiCastMacAddress;
        NetworkBufferDescriptor_t * pxDescriptor = pxNetworkBuffer;
        NetworkBufferDescriptor_t * pxNewDescriptor = NULL;
        BaseType_t xReleased = pdFALSE;

        if( ( pxEndPoint != NULL ) && ( pxEndPoint->bits.bIPv6 != pdFALSE_UNSIGNED ) )
        {
            uxNeededSize = ipSIZE_OF_ETH_HEADER + ipSIZE_OF_IPv6_HEADER + sizeof( ICMPHeader_IPv6_t );

            if( pxDescriptor->xDataLength < uxNeededSize )
            {
                pxNewDescriptor = pxDuplicateNetworkBufferWithDescriptor( pxDescriptor, uxNeededSize );
                vReleaseNetworkBufferAndDescriptor( pxDescriptor );
                pxDescriptor = pxNewDescriptor;
            }

            if( pxDescriptor != NULL )
            {
                const uint32_t ulPayloadLength = 32U;

                /* MISRA Ref 11.3.1 [Misaligned access] */
                /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
                /* coverity[misra_c_2012_rule_11_3_violation] */
                pxICMPPacket = ( ( ICMPPacket_IPv6_t * ) pxDescriptor->pucEthernetBuffer );
                pxICMPHeader_IPv6 = ( ( ICMPHeader_IPv6_t * ) &( pxICMPPacket->xICMPHeaderIPv6 ) );

                pxDescriptor->xDataLength = uxNeededSize;

                /* Set the multi-cast MAC-address. */
                xMultiCastMacAddress.ucBytes[ 0 ] = 0x33U;
                xMultiCastMacAddress.ucBytes[ 1 ] = 0x33U;
                xMultiCastMacAddress.ucBytes[ 2 ] = 0xffU;
                xMultiCastMacAddress.ucBytes[ 3 ] = pxIPAddress->ucBytes[ 13 ];
                xMultiCastMacAddress.ucBytes[ 4 ] = pxIPAddress->ucBytes[ 14 ];
                xMultiCastMacAddress.ucBytes[ 5 ] = pxIPAddress->ucBytes[ 15 ];

                /* Set Ethernet header. Source and Destination will be swapped. */
                ( void ) memcpy( pxICMPPacket->xEthernetHeader.xSourceAddress.ucBytes, xMultiCastMacAddress.ucBytes, ipMAC_ADDRESS_LENGTH_BYTES );
                ( void ) memcpy( pxICMPPacket->xEthernetHeader.xDestinationAddress.ucBytes, pxEndPoint->xMACAddress.ucBytes, ipMAC_ADDRESS_LENGTH_BYTES );
                pxICMPPacket->xEthernetHeader.usFrameType = ipIPv6_FRAME_TYPE;

                /* Set IP-header. */
                pxICMPPacket->xIPHeader.ucVersionTrafficClass = 0x60U;
                pxICMPPacket->xIPHeader.ucTrafficClassFlow = 0U;
                pxICMPPacket->xIPHeader.usFlowLabel = 0U;
                pxICMPPacket->xIPHeader.usPayloadLength = FreeRTOS_htons( ulPayloadLength );
                pxICMPPacket->xIPHeader.ucNextHeader = ipPROTOCOL_ICMP_IPv6;
                pxICMPPacket->xIPHeader.ucHopLimit = 255U;

                /* Source address */
                ( void ) memcpy( pxICMPPacket->xIPHeader.xSourceAddress.ucBytes, pxEndPoint->ipv6_settings.xIPAddress.ucBytes, ipSIZE_OF_IPv6_ADDRESS );

                /*ff02::1:ff5a:afe7 */
                ( void ) memset( xTargetIPAddress.ucBytes, 0, sizeof( xTargetIPAddress.ucBytes ) );
                xTargetIPAddress.ucBytes[ 0 ] = 0xff;
                xTargetIPAddress.ucBytes[ 1 ] = 0x02;
                xTargetIPAddress.ucBytes[ 11 ] = 0x01;
                xTargetIPAddress.ucBytes[ 12 ] = 0xff;
                xTargetIPAddress.ucBytes[ 13 ] = pxIPAddress->ucBytes[ 13 ];
                xTargetIPAddress.ucBytes[ 14 ] = pxIPAddress->ucBytes[ 14 ];
                xTargetIPAddress.ucBytes[ 15 ] = pxIPAddress->ucBytes[ 15 ];
                ( void ) memcpy( pxICMPPacket->xIPHeader.xDestinationAddress.ucBytes, xTargetIPAddress.ucBytes, ipSIZE_OF_IPv6_ADDRESS );

                /* Set ICMP header. */
                ( void ) memset( pxICMPHeader_IPv6, 0, sizeof( *pxICMPHeader_IPv6 ) );
                pxICMPHeader_IPv6->ucTypeOfMessage = ipICMP_NEIGHBOR_SOLICITATION_IPv6;
                ( void ) memcpy( pxICMPHeader_IPv6->xIPv6Address.ucBytes, pxIPAddress->ucBytes, ipSIZE_OF_IPv6_ADDRESS );
                pxICMPHeader_IPv6->ucOptionType = ndICMP_SOURCE_LINK_LAYER_ADDRESS;
                pxICMPHeader_IPv6->ucOptionLength = 1U; /* times 8 bytes. */
                ( void ) memcpy( pxICMPHeader_IPv6->ucOptionBytes, pxEndPoint->xMACAddress.ucBytes, ipMAC_ADDRESS_LENGTH_BYTES );

                /* Checksums. */
                #if ( ipconfigDRIVER_INCLUDED_TX_IP_CHECKSUM == 0 )
                {
                    /* calculate the ICMPv6 checksum for outgoing package */
                    ( void ) usGenerateProtocolChecksum( pxDescriptor->pucEthernetBuffer, pxDescriptor->xDataLength, pdTRUE );
                }
                #else
                {
                    /* Many EMAC peripherals will only calculate the ICMP checksum
                     * correctly if the field is nulled beforehand. */
                    pxICMPHeader_IPv6->usChecksum = 0U;
                }
                #endif

                /* This function will fill in the eth addresses and send the packet */
                vReturnEthernetFrame( pxDescriptor, pdTRUE );
                xReleased = pdTRUE;
            }
        }

        if( ( pxDescriptor != NULL ) && ( xReleased == pdFALSE ) )
        {
            vReleaseNetworkBufferAndDescriptor( pxDescriptor );
        }
    }
/*-----------------------------------------------------------*/

    #if ( ipconfigSUPPORT_OUTGOING_PINGS == 1 )

/**
 * @brief Send a PING request using an ICMPv6 format.
 *
 * @param[in] pxIPAddress Send an IPv6 PING request.
 * @param[in] uxNumberOfBytesToSend The number of bytes to be sent.
 * @param[in] uxBlockTimeTicks The maximum number of clock-ticks to wait while
 *            putting the message on the queue for the IP-task.
 *
 * @return When failed: pdFAIL, otherwise the PING sequence number.
 */
        BaseType_t FreeRTOS_SendPingRequestIPv6( const IPv6_Address_t * pxIPAddress,
                                                 size_t uxNumberOfBytesToSend,
                                                 TickType_t uxBlockTimeTicks )
        {
            NetworkBufferDescriptor_t * pxNetworkBuffer = NULL;
            EthernetHeader_t * pxEthernetHeader;
            ICMPPacket_IPv6_t * pxICMPPacket;
            ICMPEcho_IPv6_t * pxICMPHeader;
            BaseType_t xReturn = pdFAIL;
            static uint16_t usSequenceNumber = 0;
            uint8_t * pucChar;
            IPStackEvent_t xStackTxEvent = { eStackTxEvent, NULL };
            NetworkEndPoint_t * pxEndPoint = NULL;
            size_t uxPacketLength = 0U;
            BaseType_t xEnoughSpace;

            pxEndPoint = FreeRTOS_FindEndPointOnIP_IPv6( pxIPAddress );

            /* MISRA Ref 14.3.1 [Configuration dependent invariant] */
            /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-143 */
            /* coverity[misra_c_2012_rule_14_3_violation] */
            /* coverity[notnull] */
            if( pxEndPoint == NULL )
            {
                BaseType_t xWanted = ( xIPv6_GetIPType( pxIPAddress ) == eIPv6_Global ) ? 1 : 0;

                for( pxEndPoint = FreeRTOS_FirstEndPoint( NULL );
                     pxEndPoint != NULL;
                     pxEndPoint = FreeRTOS_NextEndPoint( NULL, pxEndPoint ) )
                {
                    if( pxEndPoint->bits.bIPv6 != 0U )
                    {
                        BaseType_t xGot = ( xIPv6_GetIPType( &( pxEndPoint->ipv6_settings.xIPAddress ) ) == eIPv6_Global ) ? 1 : 0;

                        if( xWanted == xGot )
                        {
                            break;
                        }
                    }
                }
            }

            if( uxNumberOfBytesToSend < ( ( ipconfigNETWORK_MTU - sizeof( IPHeader_IPv6_t ) ) - sizeof( ICMPEcho_IPv6_t ) ) )
            {
                xEnoughSpace = pdTRUE;
            }
            else
            {
                xEnoughSpace = pdFALSE;
            }

            if( pxEndPoint == NULL )
            {
                /* No endpoint found for the target IP-address. */
                FreeRTOS_printf( ( "SendPingRequestIPv6: no end-point found for %pip\n",
                                   ( void * ) pxIPAddress->ucBytes ) );
            }
            else if( ( uxGetNumberOfFreeNetworkBuffers() >= 3U ) && ( uxNumberOfBytesToSend >= 1U ) && ( xEnoughSpace != pdFALSE ) )
            {
                uxPacketLength = sizeof( EthernetHeader_t ) + sizeof( IPHeader_IPv6_t ) + sizeof( ICMPEcho_IPv6_t ) + uxNumberOfBytesToSend;

                /* MISRA Ref 11.3.1 [Misaligned access] */
                /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
                /* coverity[misra_c_2012_rule_11_3_violation] */
                pxNetworkBuffer = pxGetNetworkBufferWithDescriptor( BUFFER_FROM_WHERE_CALL( 181 ) uxPacketLength, uxBlockTimeTicks );

                if( pxNetworkBuffer != NULL )
                {
                    /* Probably not necessary to clear the buffer. */
                    ( void ) memset( pxNetworkBuffer->pucEthernetBuffer, 0, pxNetworkBuffer->xDataLength );

                    pxNetworkBuffer->pxEndPoint = pxEndPoint;
                    pxNetworkBuffer->pxInterface = pxEndPoint->pxNetworkInterface;

                    /* MISRA Ref 11.3.1 [Misaligned access] */
                    /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
                    /* coverity[misra_c_2012_rule_11_3_violation] */
                    pxICMPPacket = ( ( ICMPPacket_IPv6_t * ) pxNetworkBuffer->pucEthernetBuffer );

                    pxICMPHeader = ( ( ICMPEcho_IPv6_t * ) &( pxICMPPacket->xICMPHeaderIPv6 ) );
                    usSequenceNumber++;

                    pxICMPPacket->xEthernetHeader.usFrameType = ipIPv6_FRAME_TYPE;

                    pxICMPPacket->xIPHeader.usPayloadLength = FreeRTOS_htons( sizeof( ICMPEcho_IPv6_t ) + uxNumberOfBytesToSend );
                    ( void ) memcpy( pxICMPPacket->xIPHeader.xDestinationAddress.ucBytes, pxIPAddress->ucBytes, ipSIZE_OF_IPv6_ADDRESS );
                    ( void ) memcpy( pxICMPPacket->xIPHeader.xSourceAddress.ucBytes, pxEndPoint->ipv6_settings.xIPAddress.ucBytes, ipSIZE_OF_IPv6_ADDRESS );
                    FreeRTOS_printf( ( "ICMP send from %pip\n", ( void * ) pxICMPPacket->xIPHeader.xSourceAddress.ucBytes ) );

                    /* Fill in the basic header information. */
                    pxICMPHeader->ucTypeOfMessage = ipICMP_PING_REQUEST_IPv6;
                    pxICMPHeader->ucTypeOfService = 0;
                    pxICMPHeader->usIdentifier = FreeRTOS_htons( usSequenceNumber );
                    pxICMPHeader->usSequenceNumber = FreeRTOS_htons( usSequenceNumber );

                    /* Find the start of the data. */
                    pucChar = ( uint8_t * ) pxICMPHeader;
                    pucChar = &( pucChar[ sizeof( ICMPEcho_IPv6_t ) ] );

                    /* Just memset the data to a fixed value. */
                    ( void ) memset( pucChar, ( int32_t ) ndECHO_DATA_FILL_BYTE, uxNumberOfBytesToSend );

                    /* The message is complete, IP and checksum's are handled by
                     * vProcessGeneratedUDPPacket */
                    pxNetworkBuffer->pucEthernetBuffer[ ipSOCKET_OPTIONS_OFFSET ] = FREERTOS_SO_UDPCKSUM_OUT;
                    ( void ) memset( pxNetworkBuffer->xIPAddress.xIP_IPv6.ucBytes, 0, ipSIZE_OF_IPv6_ADDRESS );
                    ( void ) memcpy( pxNetworkBuffer->xIPAddress.xIP_IPv6.ucBytes, pxIPAddress->ucBytes, ipSIZE_OF_IPv6_ADDRESS );
                    /* Let vProcessGeneratedUDPPacket() know that this is an ICMP packet. */
                    pxNetworkBuffer->usPort = ipPACKET_CONTAINS_ICMP_DATA;
                    /* 'uxPacketLength' is initialised due to the flow of the program. */
                    pxNetworkBuffer->xDataLength = uxPacketLength;

                    /* MISRA Ref 11.3.1 [Misaligned access] */
                    /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
                    /* coverity[misra_c_2012_rule_11_3_violation] */
                    pxEthernetHeader = ( ( EthernetHeader_t * ) pxNetworkBuffer->pucEthernetBuffer );
                    pxEthernetHeader->usFrameType = ipIPv6_FRAME_TYPE;

                    /* Send to the stack. */
                    xStackTxEvent.pvData = pxNetworkBuffer;

                    if( xSendEventStructToIPTask( &xStackTxEvent, uxBlockTimeTicks ) != pdPASS )
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
                /* Either no proper end-pint found, or allocating the network buffer failed. */
            }

            return xReturn;
        }

    #endif /* ipconfigSUPPORT_OUTGOING_PINGS == 1 */
/*-----------------------------------------------------------*/


    #if ( ipconfigHAS_PRINTF == 1 )

/**
 * @brief Returns a printable string for the major ICMPv6 message types.  Used for
 *        debugging only.
 *
 * @param[in] xType The type of message.
 *
 * @return A null-terminated string that represents the type the kind of message.
 */
        static const char * pcMessageType( BaseType_t xType )
        {
            const char * pcReturn;

            switch( ( uint8_t ) xType )
            {
                case ipICMP_DEST_UNREACHABLE_IPv6:
                    pcReturn = "DEST_UNREACHABLE";
                    break;

                case ipICMP_PACKET_TOO_BIG_IPv6:
                    pcReturn = "PACKET_TOO_BIG";
                    break;

                case ipICMP_TIME_EXCEEDED_IPv6:
                    pcReturn = "TIME_EXCEEDED";
                    break;

                case ipICMP_PARAMETER_PROBLEM_IPv6:
                    pcReturn = "PARAMETER_PROBLEM";
                    break;

                case ipICMP_PING_REQUEST_IPv6:
                    pcReturn = "PING_REQUEST";
                    break;

                case ipICMP_PING_REPLY_IPv6:
                    pcReturn = "PING_REPLY";
                    break;

                case ipICMP_ROUTER_SOLICITATION_IPv6:
                    pcReturn = "ROUTER_SOL";
                    break;

                case ipICMP_ROUTER_ADVERTISEMENT_IPv6:
                    pcReturn = "ROUTER_ADV";
                    break;

                case ipICMP_NEIGHBOR_SOLICITATION_IPv6:
                    pcReturn = "NEIGHBOR_SOL";
                    break;

                case ipICMP_NEIGHBOR_ADVERTISEMENT_IPv6:
                    pcReturn = "NEIGHBOR_ADV";
                    break;

                default:
                    pcReturn = "UNKNOWN ICMP";
                    break;
            }

            return pcReturn;
        }
    #endif /* ( ipconfigHAS_PRINTF == 1 ) */
/*-----------------------------------------------------------*/

/**
 * @brief When a neighbour advertisement has been received, check if 'pxARPWaitingNetworkBuffer'
 *        was waiting for this new address look-up. If so, feed it to the IP-task as a new
 *        incoming packet.
 */
    static void prvCheckWaitingBuffer( const IPv6_Address_t * pxIPv6Address )
    {
        /* MISRA Ref 11.3.1 [Misaligned access] */
        /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
        /* coverity[misra_c_2012_rule_11_3_violation] */
        const IPPacket_IPv6_t * pxIPPacket = ( ( IPPacket_IPv6_t * ) pxARPWaitingNetworkBuffer->pucEthernetBuffer );
        const IPHeader_IPv6_t * pxIPHeader = &( pxIPPacket->xIPHeader );

        if( memcmp( pxIPv6Address->ucBytes, pxIPHeader->xSourceAddress.ucBytes, ipSIZE_OF_IPv6_ADDRESS ) == 0 )
        {
            IPStackEvent_t xEventMessage;
            const TickType_t xDontBlock = ( TickType_t ) 0;

            FreeRTOS_printf( ( "Waiting done\n" ) );

            xEventMessage.eEventType = eNetworkRxEvent;
            xEventMessage.pvData = ( void * ) pxARPWaitingNetworkBuffer;

            if( xSendEventStructToIPTask( &xEventMessage, xDontBlock ) != pdPASS )
            {
                /* Failed to send the message, so release the network buffer. */
                vReleaseNetworkBufferAndDescriptor( BUFFER_FROM_WHERE_CALL( 140 ) pxARPWaitingNetworkBuffer );
            }

            /* Clear the buffer. */
            pxARPWaitingNetworkBuffer = NULL;

            /* Found an ARP resolution, disable ARP resolution timer. */
            vIPSetARPResolutionTimerEnableState( pdFALSE );

            iptrace_DELAYED_ARP_REQUEST_REPLIED();
        }
    }
/*-----------------------------------------------------------*/

/**
 * @brief Process an ICMPv6 packet and send replies when applicable.
 *
 * @param[in] pxNetworkBuffer The Ethernet packet which contains an IPv6 message.
 *
 * @return A const value 'eReleaseBuffer' which means that the network must still be released.
 */
    eFrameProcessingResult_t prvProcessICMPMessage_IPv6( NetworkBufferDescriptor_t * const pxNetworkBuffer )
    {
        /* MISRA Ref 11.3.1 [Misaligned access] */
        /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
        /* coverity[misra_c_2012_rule_11_3_violation] */
        ICMPPacket_IPv6_t * pxICMPPacket = ( ( ICMPPacket_IPv6_t * ) pxNetworkBuffer->pucEthernetBuffer );
        /* coverity[misra_c_2012_rule_11_3_violation] */
        ICMPHeader_IPv6_t * pxICMPHeader_IPv6 = ( ( ICMPHeader_IPv6_t * ) &( pxICMPPacket->xICMPHeaderIPv6 ) );
        /* Note: pxNetworkBuffer->pxEndPoint is already verified to be non-NULL in prvProcessEthernetPacket() */
        NetworkEndPoint_t * pxEndPoint = pxNetworkBuffer->pxEndPoint;
        size_t uxNeededSize;

        #if ( ipconfigHAS_PRINTF == 1 )
        {
            if( pxICMPHeader_IPv6->ucTypeOfMessage != ipICMP_PING_REQUEST_IPv6 )
            {
                char pcAddress[ 40 ];
                FreeRTOS_printf( ( "ICMPv6_recv %d (%s) from %pip to %pip end-point = %s\n",
                                   pxICMPHeader_IPv6->ucTypeOfMessage,
                                   pcMessageType( ( BaseType_t ) pxICMPHeader_IPv6->ucTypeOfMessage ),
                                   ( void * ) pxICMPPacket->xIPHeader.xSourceAddress.ucBytes,
                                   ( void * ) pxICMPPacket->xIPHeader.xDestinationAddress.ucBytes,
                                   pcEndpointName( pxEndPoint, pcAddress, sizeof( pcAddress ) ) ) );
            }
        }
        #endif /* ( ipconfigHAS_PRINTF == 1 ) */

        if( pxEndPoint->bits.bIPv6 != pdFALSE_UNSIGNED )
        {
            switch( pxICMPHeader_IPv6->ucTypeOfMessage )
            {
                case ipICMP_DEST_UNREACHABLE_IPv6:
                case ipICMP_PACKET_TOO_BIG_IPv6:
                case ipICMP_TIME_EXCEEDED_IPv6:
                case ipICMP_PARAMETER_PROBLEM_IPv6:
                    /* These message types are not implemented. They are logged here above. */
                    break;

                case ipICMP_PING_REQUEST_IPv6:
                   {
                       size_t uxICMPSize;
                       uint16_t usICMPSize;

                       /* Lint would complain about casting '()' immediately. */
                       usICMPSize = FreeRTOS_ntohs( pxICMPPacket->xIPHeader.usPayloadLength );
                       uxICMPSize = ( size_t ) usICMPSize;
                       uxNeededSize = ( size_t ) ( ipSIZE_OF_ETH_HEADER + ipSIZE_OF_IPv6_HEADER + uxICMPSize );

                       if( uxNeededSize > pxNetworkBuffer->xDataLength )
                       {
                           FreeRTOS_printf( ( "Too small\n" ) );
                           break;
                       }

                       pxICMPHeader_IPv6->ucTypeOfMessage = ipICMP_PING_REPLY_IPv6;

                       /* MISRA Ref 4.14.1 [The validity of values received from external sources]. */
                       /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#directive-414. */
                       /* coverity[misra_c_2012_directive_4_14_violation] */
                       prvReturnICMP_IPv6( pxNetworkBuffer, uxICMPSize );
                   }
                   break;

                    #if ( ipconfigSUPPORT_OUTGOING_PINGS != 0 )
                        case ipICMP_PING_REPLY_IPv6:
                           {
                               ePingReplyStatus_t eStatus = eSuccess;
                               /* MISRA Ref 11.3.1 [Misaligned access] */
                               /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
                               /* coverity[misra_c_2012_rule_11_3_violation] */
                               const ICMPEcho_IPv6_t * pxICMPEchoHeader = ( ( const ICMPEcho_IPv6_t * ) pxICMPHeader_IPv6 );
                               size_t uxDataLength, uxCount;
                               const uint8_t * pucByte;

                               /* Find the total length of the IP packet. */
                               uxDataLength = ipNUMERIC_CAST( size_t, FreeRTOS_ntohs( pxICMPPacket->xIPHeader.usPayloadLength ) );
                               uxDataLength = uxDataLength - sizeof( *pxICMPEchoHeader );

                               /* Find the first byte of the data within the ICMP packet. */
                               pucByte = ( const uint8_t * ) pxICMPEchoHeader;
                               pucByte = &( pucByte[ sizeof( *pxICMPEchoHeader ) ] );

                               /* Check each byte. */
                               for( uxCount = 0; uxCount < uxDataLength; uxCount++ )
                               {
                                   if( *pucByte != ( uint8_t ) ipECHO_DATA_FILL_BYTE )
                                   {
                                       eStatus = eInvalidData;
                                       break;
                                   }

                                   pucByte++;
                               }

                               /* Call back into the application to pass it the result. */
                               vApplicationPingReplyHook( eStatus, pxICMPEchoHeader->usIdentifier );
                           }
                           break;
                    #endif /* ( ipconfigSUPPORT_OUTGOING_PINGS != 0 ) */
                case ipICMP_NEIGHBOR_SOLICITATION_IPv6:
                   {
                       size_t uxICMPSize;
                       BaseType_t xCompare;
                       const NetworkEndPoint_t * pxTargetedEndPoint = pxEndPoint;
                       const NetworkEndPoint_t * pxEndPointInSameSubnet = FreeRTOS_InterfaceEPInSameSubnet_IPv6( pxNetworkBuffer->pxInterface, &( pxICMPHeader_IPv6->xIPv6Address ) );

                       if( pxEndPointInSameSubnet != NULL )
                       {
                           pxTargetedEndPoint = pxEndPointInSameSubnet;
                       }
                       else
                       {
                           FreeRTOS_debug_printf( ( "prvProcessICMPMessage_IPv6: No match for %pip\n",
                                                    pxICMPHeader_IPv6->xIPv6Address.ucBytes ) );
                       }

                       uxICMPSize = sizeof( ICMPHeader_IPv6_t );
                       uxNeededSize = ( size_t ) ( ipSIZE_OF_ETH_HEADER + ipSIZE_OF_IPv6_HEADER + uxICMPSize );

                       if( uxNeededSize > pxNetworkBuffer->xDataLength )
                       {
                           FreeRTOS_printf( ( "Too small\n" ) );
                           break;
                       }

                       xCompare = memcmp( pxICMPHeader_IPv6->xIPv6Address.ucBytes, pxTargetedEndPoint->ipv6_settings.xIPAddress.ucBytes, ipSIZE_OF_IPv6_ADDRESS );

                       FreeRTOS_printf( ( "ND NS for %pip endpoint %pip %s\n",
                                          ( void * ) pxICMPHeader_IPv6->xIPv6Address.ucBytes,
                                          ( void * ) pxNetworkBuffer->pxEndPoint->ipv6_settings.xIPAddress.ucBytes,
                                          ( xCompare == 0 ) ? "Reply" : "Ignore" ) );

                       if( xCompare == 0 )
                       {
                           pxICMPHeader_IPv6->ucTypeOfMessage = ipICMP_NEIGHBOR_ADVERTISEMENT_IPv6;
                           pxICMPHeader_IPv6->ucTypeOfService = 0U;
                           pxICMPHeader_IPv6->ulReserved = ndICMPv6_FLAG_SOLICITED | ndICMPv6_FLAG_UPDATE;
                           pxICMPHeader_IPv6->ulReserved = FreeRTOS_htonl( pxICMPHeader_IPv6->ulReserved );

                           /* Type of option. */
                           pxICMPHeader_IPv6->ucOptionType = ndICMP_TARGET_LINK_LAYER_ADDRESS;
                           /* Length of option in units of 8 bytes. */
                           pxICMPHeader_IPv6->ucOptionLength = 1U;
                           ( void ) memcpy( pxICMPHeader_IPv6->ucOptionBytes, pxTargetedEndPoint->xMACAddress.ucBytes, sizeof( MACAddress_t ) );
                           pxICMPPacket->xIPHeader.ucHopLimit = 255U;
                           ( void ) memcpy( pxICMPHeader_IPv6->xIPv6Address.ucBytes, pxTargetedEndPoint->ipv6_settings.xIPAddress.ucBytes, sizeof( pxICMPHeader_IPv6->xIPv6Address.ucBytes ) );
                           prvReturnICMP_IPv6( pxNetworkBuffer, uxICMPSize );
                       }
                   }
                   break;

                case ipICMP_NEIGHBOR_ADVERTISEMENT_IPv6:
                    /* MISRA Ref 11.3.1 [Misaligned access] */
                    /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
                    /* coverity[misra_c_2012_rule_11_3_violation] */
                    vNDRefreshCacheEntry( ( ( const MACAddress_t * ) pxICMPHeader_IPv6->ucOptionBytes ),
                                          &( pxICMPHeader_IPv6->xIPv6Address ),
                                          pxEndPoint );
                    FreeRTOS_printf( ( "NEIGHBOR_ADV from %pip\n",
                                       ( void * ) pxICMPHeader_IPv6->xIPv6Address.ucBytes ) );

                    #if ( ipconfigUSE_RA != 0 )

                        /* Receive a NA ( Neighbour Advertisement ) message to see if a chosen IP-address is already in use.
                         * This is important during SLAAC. */
                        vReceiveNA( pxNetworkBuffer );
                    #endif

                    if( ( pxARPWaitingNetworkBuffer != NULL ) &&
                        ( uxIPHeaderSizePacket( pxARPWaitingNetworkBuffer ) == ipSIZE_OF_IPv6_HEADER ) )
                    {
                        prvCheckWaitingBuffer( &( pxICMPHeader_IPv6->xIPv6Address ) );
                    }

                    break;

                case ipICMP_ROUTER_SOLICITATION_IPv6:
                    break;

                    #if ( ipconfigUSE_RA != 0 )
                        case ipICMP_ROUTER_ADVERTISEMENT_IPv6:
                            vReceiveRA( pxNetworkBuffer );
                            break;
                    #endif /* ( ipconfigUSE_RA != 0 ) */

                default:
                    /* All possible values are included here above. */
                    break;
            } /* switch( pxICMPHeader_IPv6->ucTypeOfMessage ) */
        }     /* if( pxEndPoint->bits.bIPv6 != pdFALSE_UNSIGNED ) */

        return eReleaseBuffer;
    }
/*-----------------------------------------------------------*/

/**
 * @brief Send out a Neighbour Advertisement message.
 *
 * @param[in] pxEndPoint The end-point to use.
 */
    /* MISRA Ref 8.9.1 [File scoped variables] */
    /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-89 */
    /* coverity[misra_c_2012_rule_8_9_violation] */
    /* coverity[single_use] */
    void FreeRTOS_OutputAdvertiseIPv6( NetworkEndPoint_t * pxEndPoint )
    {
        NetworkBufferDescriptor_t * pxNetworkBuffer;
        ICMPPacket_IPv6_t * pxICMPPacket;
        NetworkInterface_t * pxInterface;
        ICMPHeader_IPv6_t * pxICMPHeader_IPv6;
        size_t uxICMPSize;
        size_t uxPacketSize;

        uxPacketSize = ipSIZE_OF_ETH_HEADER + ipSIZE_OF_IPv6_HEADER + sizeof( ICMPHeader_IPv6_t );

        /* This is called from the context of the IP event task, so a block time
         * must not be used. */
        pxNetworkBuffer = pxGetNetworkBufferWithDescriptor( uxPacketSize, ndDONT_BLOCK );

        if( pxNetworkBuffer != NULL )
        {
            ( void ) memset( pxNetworkBuffer->xIPAddress.xIP_IPv6.ucBytes, 0, ipSIZE_OF_IPv6_ADDRESS );
            pxNetworkBuffer->pxEndPoint = pxEndPoint;

            pxInterface = pxEndPoint->pxNetworkInterface;

            configASSERT( pxInterface != NULL );

            /* MISRA Ref 11.3.1 [Misaligned access] */
            /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
            /* coverity[misra_c_2012_rule_11_3_violation] */
            pxICMPPacket = ( ( ICMPPacket_IPv6_t * ) pxNetworkBuffer->pucEthernetBuffer );
            pxICMPHeader_IPv6 = ( ( ICMPHeader_IPv6_t * ) &( pxICMPPacket->xICMPHeaderIPv6 ) );

            ( void ) memcpy( pxICMPPacket->xEthernetHeader.xDestinationAddress.ucBytes, pcLOCAL_ALL_NODES_MULTICAST_MAC, ipMAC_ADDRESS_LENGTH_BYTES );
            ( void ) memcpy( pxICMPPacket->xEthernetHeader.xSourceAddress.ucBytes, pxEndPoint->xMACAddress.ucBytes, ipMAC_ADDRESS_LENGTH_BYTES );
            pxICMPPacket->xEthernetHeader.usFrameType = ipIPv6_FRAME_TYPE; /* 12 + 2 = 14 */

            pxICMPPacket->xIPHeader.ucVersionTrafficClass = 0x60;
            pxICMPPacket->xIPHeader.ucTrafficClassFlow = 0;
            pxICMPPacket->xIPHeader.usFlowLabel = 0;

            pxICMPPacket->xIPHeader.usPayloadLength = FreeRTOS_htons( sizeof( ICMPHeader_IPv6_t ) );
            pxICMPPacket->xIPHeader.ucNextHeader = ipPROTOCOL_ICMP_IPv6;
            pxICMPPacket->xIPHeader.ucHopLimit = 255;
            ( void ) memcpy( pxICMPPacket->xIPHeader.xSourceAddress.ucBytes, pxEndPoint->ipv6_settings.xIPAddress.ucBytes, ipSIZE_OF_IPv6_ADDRESS );
            ( void ) memcpy( pxICMPPacket->xIPHeader.xDestinationAddress.ucBytes, pcLOCAL_ALL_NODES_MULTICAST_IP, ipSIZE_OF_IPv6_ADDRESS );

            uxICMPSize = sizeof( ICMPHeader_IPv6_t );
            pxICMPHeader_IPv6->ucTypeOfMessage = ipICMP_NEIGHBOR_ADVERTISEMENT_IPv6;
            pxICMPHeader_IPv6->ucTypeOfService = 0;
            pxICMPHeader_IPv6->ulReserved = ndICMPv6_FLAG_SOLICITED | ndICMPv6_FLAG_UPDATE;
            pxICMPHeader_IPv6->ulReserved = FreeRTOS_htonl( pxICMPHeader_IPv6->ulReserved );

            /* Type of option. */
            pxICMPHeader_IPv6->ucOptionType = ndICMP_TARGET_LINK_LAYER_ADDRESS;
            /* Length of option in units of 8 bytes. */
            pxICMPHeader_IPv6->ucOptionLength = 1;
            ( void ) memcpy( pxICMPHeader_IPv6->ucOptionBytes, pxEndPoint->xMACAddress.ucBytes, sizeof( MACAddress_t ) );
            pxICMPPacket->xIPHeader.ucHopLimit = 255;
            ( void ) memcpy( pxICMPHeader_IPv6->xIPv6Address.ucBytes, pxEndPoint->ipv6_settings.xIPAddress.ucBytes, sizeof( pxICMPHeader_IPv6->xIPv6Address.ucBytes ) );

            /* Important: tell NIC driver how many bytes must be sent */
            pxNetworkBuffer->xDataLength = ( size_t ) ( ipSIZE_OF_ETH_HEADER + ipSIZE_OF_IPv6_HEADER + uxICMPSize );

            #if ( ipconfigDRIVER_INCLUDED_TX_IP_CHECKSUM == 0 )
            {
                /* calculate the ICMPv6 checksum for outgoing package */
                ( void ) usGenerateProtocolChecksum( pxNetworkBuffer->pucEthernetBuffer, pxNetworkBuffer->xDataLength, pdTRUE );
            }
            #else
            {
                /* Many EMAC peripherals will only calculate the ICMP checksum
                 * correctly if the field is nulled beforehand. */
                pxICMPHeader_IPv6->usChecksum = 0;
            }
            #endif

            /* Set the parameter 'bReleaseAfterSend'. */
            ( void ) pxInterface->pfOutput( pxInterface, pxNetworkBuffer, pdTRUE );
        }
    }
/*-----------------------------------------------------------*/

/**
 * @brief Create an IPv16 address, based on a prefix.
 *
 * @param[out] pxIPAddress The location where the new IPv6 address will be stored.
 * @param[in] pxPrefix The prefix to be used.
 * @param[in] uxPrefixLength The length of the prefix.
 * @param[in] xDoRandom A non-zero value if the bits after the prefix should have a random value.
 *
 * @return pdPASS if the operation was successful. Or pdFAIL in case xApplicationGetRandomNumber()
 *         returned an error.
 */
    BaseType_t FreeRTOS_CreateIPv6Address( IPv6_Address_t * pxIPAddress,
                                           const IPv6_Address_t * pxPrefix,
                                           size_t uxPrefixLength,
                                           BaseType_t xDoRandom )
    {
        uint32_t pulRandom[ 4 ];
        uint8_t * pucSource;
        BaseType_t xIndex, xResult = pdPASS;

        if( xDoRandom != pdFALSE )
        {
            /* Create an IP-address, based on a net prefix and a
             * random host address.
             * ARRAY_SIZE_X() returns the size of an array as a
             * signed value ( BaseType_t ).
             */
            for( xIndex = 0; xIndex < ARRAY_SIZE_X( pulRandom ); xIndex++ )
            {
                if( xApplicationGetRandomNumber( &( pulRandom[ xIndex ] ) ) == pdFAIL )
                {
                    xResult = pdFAIL;
                    break;
                }
            }
        }
        else
        {
            ( void ) memset( pulRandom, 0, sizeof( pulRandom ) );
        }

        if( xResult == pdPASS )
        {
            size_t uxIndex;
            /* A loopback IP-address has a prefix of 128. */
            configASSERT( ( uxPrefixLength > 0U ) && ( uxPrefixLength <= ( 8U * ipSIZE_OF_IPv6_ADDRESS ) ) );

            if( uxPrefixLength >= 8U )
            {
                ( void ) memcpy( pxIPAddress->ucBytes, pxPrefix->ucBytes, ( uxPrefixLength + 7U ) / 8U );
            }

            pucSource = ( uint8_t * ) pulRandom;
            uxIndex = uxPrefixLength / 8U;

            if( ( uxPrefixLength % 8U ) != 0U )
            {
                /* uxHostLen is between 1 and 7 bits long. */
                size_t uxHostLen = 8U - ( uxPrefixLength % 8U );
                uint32_t uxHostMask = ( ( ( uint32_t ) 1U ) << uxHostLen ) - 1U;
                uint8_t ucNetMask = ( uint8_t ) ~( uxHostMask );

                pxIPAddress->ucBytes[ uxIndex ] &= ucNetMask;
                pxIPAddress->ucBytes[ uxIndex ] |= ( pucSource[ 0 ] & ( ( uint8_t ) uxHostMask ) );
                pucSource = &( pucSource[ 1 ] );
                uxIndex++;
            }

            if( uxIndex < ipSIZE_OF_IPv6_ADDRESS )
            {
                ( void ) memcpy( &( pxIPAddress->ucBytes[ uxIndex ] ), pucSource, ipSIZE_OF_IPv6_ADDRESS - uxIndex );
            }
        }

        return xResult;
    }
/*-----------------------------------------------------------*/
#endif /* ipconfigUSE_IPv6 */
