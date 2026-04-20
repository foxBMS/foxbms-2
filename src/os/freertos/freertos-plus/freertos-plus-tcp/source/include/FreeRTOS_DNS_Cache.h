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
 * https://github.com/FreeRTOS
 * https://www.FreeRTOS.org
 */

#ifndef FREERTOS_DNS_CACHE_H
#define FREERTOS_DNS_CACHE_H

/* FreeRTOS includes. */
#include "FreeRTOS.h"

#include "FreeRTOS_DNS_Globals.h"

/* Standard includes. */
#include <stdint.h>

#if ( ( ipconfigUSE_DNS_CACHE == 1 ) && ( ipconfigUSE_DNS != 0 ) )

/**
 * @brief cache entry format structure
 */
    typedef struct xDNS_CACHE_TABLE_ROW
    {
        IPv46_Address_t xAddresses[ ipconfigDNS_CACHE_ADDRESSES_PER_ENTRY ]; /*!< The IP address(es) of a DNS cache entry. */
        char pcName[ ipconfigDNS_CACHE_NAME_LENGTH ];                        /*!< The name of the host */
        uint32_t ulTTL;                                                      /*!< Time-to-Live (in seconds) from the DNS server. */
        uint32_t ulTimeWhenAddedInSeconds;                                   /*!< time at which the entry was added */
        #if ( ipconfigDNS_CACHE_ADDRESSES_PER_ENTRY > 1 )
            uint8_t ucNumIPAddresses;                                        /*!< number of ip addresses for the same entry */
            uint8_t ucCurrentIPAddress;                                      /*!< current ip address index */
        #endif
    } DNSCacheRow_t;

/* Look for the indicated host name in the DNS cache. Returns the IPv4
 * address if present, or 0x0 otherwise. */
    uint32_t FreeRTOS_dnslookup( const char * pcHostName );

    void FreeRTOS_dnsclear( void );

/**
 * @brief For debugging only: prints the contents of the DNS cache table.
 */
    void vShowDNSCacheTable( void );

    BaseType_t FreeRTOS_dns_update( const char * pcName,
                                    IPv46_Address_t * pxIP,
                                    uint32_t ulTTL,
                                    BaseType_t xLookUp,
                                    struct freertos_addrinfo ** ppxAddressInfo );

    BaseType_t FreeRTOS_ProcessDNSCache( const char * pcName,
                                         IPv46_Address_t * pxIP,
                                         uint32_t ulTTL,
                                         BaseType_t xLookUp,
                                         struct freertos_addrinfo ** ppxAddressInfo );

    uint32_t Prepare_CacheLookup( const char * pcHostName,
                                  BaseType_t xFamily,
                                  struct freertos_addrinfo ** ppxAddressInfo );
#endif /* if ( ipconfigUSE_DNS_CACHE == 1 ) */

#endif /* FREERTOS_DNS_CACHE_H */
