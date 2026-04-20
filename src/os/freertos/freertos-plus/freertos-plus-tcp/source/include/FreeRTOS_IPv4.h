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

#ifndef FREERTOS_IPV4_H
#define FREERTOS_IPV4_H

#include "FreeRTOS.h"
#include "task.h"

/* Application level configuration options. */
#include "FreeRTOSIPConfig.h"
#include "FreeRTOSIPConfigDefaults.h"

/* *INDENT-OFF* */
#ifdef __cplusplus
    extern "C" {
#endif
/* *INDENT-ON* */

/* Forward declarations. */
struct xNETWORK_BUFFER;
struct xIP_PACKET;

#define ipSIZE_OF_IPv4_HEADER               20U
#define ipSIZE_OF_IPv4_ADDRESS              4U
#define ipSIZE_OF_ICMPv4_HEADER             8U
#define ipTYPE_IPv4                         ( 0x40U )

#define ipFIRST_LOOPBACK_IPv4               0x7F000000UL         /**< Lowest IPv4 loopback address (including). */
#define ipLAST_LOOPBACK_IPv4                0x80000000UL         /**< Highest IPv4 loopback address (excluding). */

/* The first byte in the IPv4 header combines the IP version (4) with
 * with the length of the IP header. */
#define ipIPV4_VERSION_HEADER_LENGTH_MIN    0x45U /**< Minimum IPv4 header length. */
#define ipIPV4_VERSION_HEADER_LENGTH_MAX    0x4FU /**< Maximum IPv4 header length. */

/* IPv4 multicast MAC address starts with 01-00-5E. */
#define ipMULTICAST_MAC_ADDRESS_IPv4_0      0x01U
#define ipMULTICAST_MAC_ADDRESS_IPv4_1      0x00U
#define ipMULTICAST_MAC_ADDRESS_IPv4_2      0x5EU

/*
 *  These functions come from the IPv4-only library.
 *  void FreeRTOS_SetIPAddress( uint32_t ulIPAddress );
 *  void FreeRTOS_SetNetmask( uint32_t ulNetmask );
 *  void FreeRTOS_SetGatewayAddress( uint32_t ulGatewayAddress );
 *  uint32_t FreeRTOS_GetGatewayAddress( void );
 *  uint32_t FreeRTOS_GetDNSServerAddress( void );
 *  uint32_t FreeRTOS_GetNetmask( void );
 */

void FreeRTOS_SetIPAddress( uint32_t ulIPAddress );
void FreeRTOS_SetNetmask( uint32_t ulNetmask );
void FreeRTOS_SetGatewayAddress( uint32_t ulGatewayAddress );
uint32_t FreeRTOS_GetGatewayAddress( void );
uint32_t FreeRTOS_GetDNSServerAddress( void );
uint32_t FreeRTOS_GetNetmask( void );
uint32_t FreeRTOS_GetIPAddress( void );

/* Return pdTRUE if the IPv4 address is a multicast address. */
BaseType_t xIsIPv4Multicast( uint32_t ulIPAddress );

/* Return pdTRUE if the IPv4 address is a loopback address. */
BaseType_t xIsIPv4Loopback( uint32_t ulAddress );

/*
 * Return pdTRUE if either source or destination is a loopback address.
 * A loopback IP-address may only communicate internally with another
 * loopback IP-address.
 */
BaseType_t xBadIPv4Loopback( const IPHeader_t * const pxIPHeader );

/* *INDENT-OFF* */
#ifdef __cplusplus
    } /* extern "C" */
#endif
/* *INDENT-ON* */

#endif /* FREERTOS_IPV4_H */
