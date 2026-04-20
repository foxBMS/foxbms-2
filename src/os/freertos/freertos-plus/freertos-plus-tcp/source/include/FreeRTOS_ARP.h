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

#ifndef FREERTOS_ARP_H
#define FREERTOS_ARP_H

/* Application level configuration options. */
#include "FreeRTOSIPConfig.h"
#include "FreeRTOSIPConfigDefaults.h"

#include "FreeRTOS_IP.h"
#include "FreeRTOS_Routing.h"

/* *INDENT-OFF* */
#ifdef __cplusplus
    extern "C" {
#endif
/* *INDENT-ON* */

/*-----------------------------------------------------------*/
/* Miscellaneous structure and definitions. */
/*-----------------------------------------------------------*/

/* A forward declaration of 'xNetworkInterface' which is
 * declared in FreeRTOS_Routing.h */
struct xNetworkInterface;
struct xNetworkEndPoint;

/**
 * Structure for one row in the ARP cache table.
 */
typedef struct xARP_CACHE_TABLE_ROW
{
    uint32_t ulIPAddress;     /**< The IP address of an ARP cache entry. */
    MACAddress_t xMACAddress; /**< The MAC address of an ARP cache entry. */
    uint8_t ucAge;            /**< A value that is periodically decremented but can also be refreshed by active communication.  The ARP cache entry is removed if the value reaches zero. */
    uint8_t ucValid;          /**< pdTRUE: xMACAddress is valid, pdFALSE: waiting for ARP reply */
    struct xNetworkEndPoint
    * pxEndPoint;             /**< The end-point on which the MAC address was last seen. */
} ARPCacheRow_t;

/** @brief A structure used internally in FreeRTOS_ARP.c.
 * It is used as a parameter for the function prvFindCacheEntry().*/
typedef struct xCacheLocation
{
    BaseType_t xIpEntry;  /**< The index of the matching IP-address. */
    BaseType_t xMacEntry; /**< The index of the matching MAC-address. */
    BaseType_t xUseEntry; /**< The index of the first free location. */
} CacheLocation_t;

/**
 * Look for an IP-MAC couple in ARP cache and reset the 'age' field. If no match
 * is found then no action will be taken.
 */
void vARPRefreshCacheEntryAge( const MACAddress_t * pxMACAddress,
                               const uint32_t ulIPAddress );

/*
 * If ulIPAddress is already in the ARP cache table then reset the age of the
 * entry back to its maximum value.  If ulIPAddress is not already in the ARP
 * cache table then add it - replacing the oldest current entry if there is not
 * a free space available.
 */
void vARPRefreshCacheEntry( const MACAddress_t * pxMACAddress,
                            const uint32_t ulIPAddress,
                            struct xNetworkEndPoint * pxEndPoint );

#if ( ipconfigARP_USE_CLASH_DETECTION != 0 )
    /* Becomes non-zero if another device responded to a gratuitous ARP message. */
    extern BaseType_t xARPHadIPClash;
    /* MAC-address of the other device containing the same IP-address. */
    extern MACAddress_t xARPClashMacAddress;
#endif /* ipconfigARP_USE_CLASH_DETECTION */

#if ( ipconfigUSE_ARP_REMOVE_ENTRY != 0 )

/*
 * In some rare cases, it might be useful to remove a ARP cache entry of a
 * known MAC address to make sure it gets refreshed.
 */
    uint32_t ulARPRemoveCacheEntryByMac( const MACAddress_t * pxMACAddress );

#endif /* ipconfigUSE_ARP_REMOVE_ENTRY != 0 */


BaseType_t xIsIPInARPCache( uint32_t ulAddressToLookup );

BaseType_t xCheckRequiresARPResolution( const NetworkBufferDescriptor_t * pxNetworkBuffer );

/*
 * Look for ulIPAddress in the ARP cache.  If the IP address exists, copy the
 * associated MAC address into pxMACAddress, refresh the ARP cache entry's
 * age, and return eResolutionCacheHit.  If the IP address does not exist in the ARP
 * cache return eResolutionCacheMiss.  If the packet cannot be sent for any reason
 * (maybe DHCP is still in process, or the addressing needs a gateway but there
 * isn't a gateway defined) then return eResolutionFailed.
 */
eResolutionLookupResult_t eARPGetCacheEntry( uint32_t * pulIPAddress,
                                             MACAddress_t * const pxMACAddress,
                                             struct xNetworkEndPoint ** ppxEndPoint );

#if ( ipconfigUSE_ARP_REVERSED_LOOKUP != 0 )

/* Lookup an IP-address if only the MAC-address is known */
    eResolutionLookupResult_t eARPGetCacheEntryByMac( const MACAddress_t * const pxMACAddress,
                                                      uint32_t * pulIPAddress,
                                                      struct xNetworkInterface ** ppxInterface );

#endif

/*
 * Reduce the age count in each entry within the ARP cache.  An entry is no
 * longer considered valid and is deleted if its age reaches zero.
 */
void vARPAgeCache( void );

/*
 * After DHCP is ready and when changing IP address, force a quick send of our new IP
 * address
 */
void vARPSendGratuitous( void );

/*
 * Send out an ARP request for the IP address contained in pxNetworkBuffer, and
 * add an entry into the ARP table that indicates that an ARP reply is
 * outstanding so re-transmissions can be generated.
 */
void vARPGenerateRequestPacket( NetworkBufferDescriptor_t * const pxNetworkBuffer );

void FreeRTOS_OutputARPRequest( uint32_t ulIPAddress );

/*
 * Create and send an ARP request packet to IPv4 endpoints of an interface.
 */
void FreeRTOS_OutputARPRequest_Multi( NetworkEndPoint_t * pxEndPoint,
                                      uint32_t ulIPAddress );

/* xARPWaitResolution checks if an IPv4 address is already known. If not
 * it may send an ARP request and wait for a reply.  This function will
 * only be called from an application. */
BaseType_t xARPWaitResolution( uint32_t ulIPAddress,
                               TickType_t uxTicksToWait );

/* Clear all entries in the ARp cache. */
void FreeRTOS_ClearARP( const struct xNetworkEndPoint * pxEndPoint );

/* Show all valid ARP entries */
#if ( ipconfigHAS_PRINTF != 0 ) || ( ipconfigHAS_DEBUG_PRINTF != 0 )
    void FreeRTOS_PrintARPCache( void );
#endif

/* *INDENT-OFF* */
#ifdef __cplusplus
    } /* extern "C" */
#endif
/* *INDENT-ON* */

#endif /* FREERTOS_ARP_H */
