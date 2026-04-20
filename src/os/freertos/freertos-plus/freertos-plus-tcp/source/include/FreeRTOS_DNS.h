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

#ifndef FREERTOS_DNS_H
#define FREERTOS_DNS_H

#include "FreeRTOS.h"

/* Application level configuration options. */
#include "FreeRTOS_DNS_Globals.h"
#include "FreeRTOS_DNS_Callback.h"
#include "FreeRTOS_DNS_Cache.h"

/* *INDENT-OFF* */
#ifdef __cplusplus
    extern "C" {
#endif
/* *INDENT-ON* */

/* The LLMNR MAC address is 01:00:5e:00:00:fc */
extern const MACAddress_t xLLMNR_MacAddress;

/* The LLMNR IPv6 MAC address is 33:33:00:01:00:03 */
extern const MACAddress_t xLLMNR_MacAddressIPv6;

#if ( ( ipconfigUSE_LLMNR != 0 ) && ( ipconfigUSE_IPv6 != 0 ) )
/* The LLMNR IPv6 address is ff02::1:3 */
    extern const IPv6_Address_t ipLLMNR_IP_ADDR_IPv6;
#endif /* ( ( ipconfigUSE_LLMNR != 0 ) && ( ipconfigUSE_IPv6 != 0 ) ) */

/* The MDNS MAC address is 01:00:5e:00:00:fc */
extern const MACAddress_t xMDNS_MacAddress;

/* The MDNS IPv6 MAC address is 33:33:00:01:00:03 */

/* This type-name was formally "misspelled" as
 * xMDNS_MACAddressIPv6 with "MAC": */
extern const MACAddress_t xMDNS_MacAddressIPv6;
/* Guarantee backward compatibility. */
#define xMDNS_MACAddressIPv6    xMDNS_MacAddressIPv6

#if ( ( ipconfigUSE_MDNS != 0 ) && ( ipconfigUSE_IPv6 != 0 ) )
/* The MDNS IPv6 address is ff02::1:3 */
    extern const IPv6_Address_t ipMDNS_IP_ADDR_IPv6;
#endif /* ( ( ipconfigUSE_MDNS != 0 ) && ( ipconfigUSE_IPv6 != 0 ) ) */

/** @brief While doing integration tests, it is necessary to influence the choice
 * between DNS/IPv4 and DNS/IPv4.  Depending on this, a DNS server will be
 * addressed via IPv4 or IPv6 messages. */
typedef enum xIPPreference
{
    xPreferenceNone,
    xPreferenceIPv4
    #if ( ipconfigUSE_IPv6 != 0 )
        ,
        xPreferenceIPv6
    #endif
} IPPreference_t;

/** @brief This variable determines he choice of DNS server, either IPv4 or IPv6. */
extern IPPreference_t xDNS_IP_Preference;

/*
 * LLMNR is very similar to DNS, so is handled by the DNS routines.
 */
uint32_t ulDNSHandlePacket( const NetworkBufferDescriptor_t * pxNetworkBuffer );

#if ( ipconfigUSE_NBNS != 0 )

/*
 * Inspect a NetBIOS Names-Service message.  If the name matches with ours
 * (xApplicationDNSQueryHook returns true) an answer will be sent back.
 * Note that LLMNR is a better protocol for name services on a LAN as it is
 * less polluted
 */
    uint32_t ulNBNSHandlePacket( NetworkBufferDescriptor_t * pxNetworkBuffer );

#endif /* ipconfigUSE_NBNS */

#if ( ipconfigDNS_USE_CALLBACKS != 0 )

/*
 * Asynchronous version of gethostbyname()
 * xTimeout is in units of ms.
 */
    uint32_t FreeRTOS_gethostbyname_a( const char * pcHostName,
                                       FOnDNSEvent pCallback,
                                       void * pvSearchID,
                                       TickType_t uxTimeout );
    void FreeRTOS_gethostbyname_cancel( void * pvSearchID );

/* The asynchronous versions of FreeRTOS_getaddrinfo(). */
    BaseType_t FreeRTOS_getaddrinfo_a( const char * pcName,                      /* The name of the node or device */
                                       const char * pcService,                   /* Ignored for now. */
                                       const struct freertos_addrinfo * pxHints, /* If not NULL: preferences. */
                                       struct freertos_addrinfo ** ppxResult,    /* An allocated struct, containing the results. */
                                       FOnDNSEvent pCallback,
                                       void * pvSearchID,
                                       TickType_t uxTimeout );

#endif /* if ( ipconfigDNS_USE_CALLBACKS != 0 ) */

/*
 * Lookup a IPv4 node in a blocking-way.
 * It returns a 32-bit IP-address, 0 when not found.
 * gethostbyname() is already deprecated.
 */
uint32_t FreeRTOS_gethostbyname( const char * pcHostName );

/* _HT_ Although this function is private to the library, it needs a global declaration. */
struct freertos_addrinfo * pxNew_AddrInfo( const char * pcName,
                                           BaseType_t xFamily,
                                           const uint8_t * pucAddress );

/*
 * FreeRTOS_getaddrinfo() replaces FreeRTOS_gethostbyname().
 * When 'ipconfigUSE_IPv6' is defined, it can also retrieve IPv6 addresses,
 * in case pxHints->ai_family equals FREERTOS_AF_INET6.
 * Otherwise, or when pxHints is NULL, only IPv4 addresses will be returned.
 */
BaseType_t FreeRTOS_getaddrinfo( const char * pcName,                      /* The name of the node or device */
                                 const char * pcService,                   /* Ignored for now. */
                                 const struct freertos_addrinfo * pxHints, /* If not NULL: preferences. */
                                 struct freertos_addrinfo ** ppxResult );  /* An allocated struct, containing the results. */

/* When FreeRTOS_getaddrinfo() is successful, ppxResult will point to an
 * allocated structure.  This pointer must be released by the user by calling
 * FreeRTOS_freeaddrinfo().
 */
void FreeRTOS_freeaddrinfo( struct freertos_addrinfo * pxInfo );

/* Sets the DNS IP preference while doing DNS lookup to indicate the preference
 * for a DNS server: either IPv4 or IPv6. Defaults to xPreferenceIPv4 */
BaseType_t FreeRTOS_SetDNSIPPreference( IPPreference_t eIPPreference );

#if ( ipconfigDNS_USE_CALLBACKS == 1 )

/*
 * The function vDNSInitialise() initialises the DNS module.
 * It will be called "internally", by the IP-task.
 */
    void vDNSInitialise( void );
#endif /* ( ipconfigDNS_USE_CALLBACKS == 1 ) */

/* *INDENT-OFF* */
#ifdef __cplusplus
    } /* extern "C" */
#endif
/* *INDENT-ON* */

#endif /* FREERTOS_DNS_H */
