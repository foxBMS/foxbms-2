/*
 * FreeRTOS+TCP V2.3.1
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

#ifndef FREERTOS_IPV6_H
#define FREERTOS_IPV6_H

#include "FreeRTOS.h"
#include "task.h"
#include "FreeRTOS_IP.h"
#include "FreeRTOS_IP_Common.h"

/* *INDENT-OFF* */
#ifdef __cplusplus
    extern "C" {
#endif
/* *INDENT-ON* */

/* Some constants defining the sizes of several parts of a packet.
 * These defines come before including the configuration header files. */
#define ipSIZE_OF_IPv6_HEADER                    40U
#define ipSIZE_OF_ICMPv6_HEADER                  24U

#define ipSIZE_OF_IPv6_ADDRESS                   16U

#define ipPROTOCOL_ICMP_IPv6                     ( 58U )
#define ipTYPE_IPv6                              ( 0x60U )

/* Some IPv6 ICMP requests. */
#define ipICMP_DEST_UNREACHABLE_IPv6             ( ( uint8_t ) 1U )
#define ipICMP_PACKET_TOO_BIG_IPv6               ( ( uint8_t ) 2U )
#define ipICMP_TIME_EXCEEDED_IPv6                ( ( uint8_t ) 3U )
#define ipICMP_PARAMETER_PROBLEM_IPv6            ( ( uint8_t ) 4U )
#define ipICMP_PING_REQUEST_IPv6                 ( ( uint8_t ) 128U )
#define ipICMP_PING_REPLY_IPv6                   ( ( uint8_t ) 129U )
#define ipICMP_ROUTER_SOLICITATION_IPv6          ( ( uint8_t ) 133U )
#define ipICMP_ROUTER_ADVERTISEMENT_IPv6         ( ( uint8_t ) 134U )
#define ipICMP_NEIGHBOR_SOLICITATION_IPv6        ( ( uint8_t ) 135U )
#define ipICMP_NEIGHBOR_ADVERTISEMENT_IPv6       ( ( uint8_t ) 136U )


#define ipIPv6_EXT_HEADER_HOP_BY_HOP             0U
#define ipIPv6_EXT_HEADER_ROUTING_HEADER         43U
#define ipIPv6_EXT_HEADER_FRAGMENT_HEADER        44U
#define ipIPv6_EXT_HEADER_SECURE_PAYLOAD         50U
#define ipIPv6_EXT_HEADER_AUTHEN_HEADER          51U
#define ipIPv6_EXT_HEADER_DESTINATION_OPTIONS    60U
/* Destination options may follow here in case there are no routing options. */
#define ipIPv6_EXT_HEADER_MOBILITY_HEADER        135U

extern const struct xIPv6_Address FreeRTOS_in6addr_any;
extern const struct xIPv6_Address FreeRTOS_in6addr_loopback;

/* IPv6 multicast MAC address starts with 33-33-. */
#define ipMULTICAST_MAC_ADDRESS_IPv6_0    0x33U
#define ipMULTICAST_MAC_ADDRESS_IPv6_1    0x33U


/* A forward declaration of 'struct xNetworkEndPoint' and 'xNetworkInterface'.
 * The actual declaration can be found in FreeRTOS_Routing.h which is included
 * as the last +TCP header file. */
struct xNetworkEndPoint;
struct xNetworkInterface;

/* The function 'prvAllowIPPacket()' checks if a IPv6 packets should be processed. */
eFrameProcessingResult_t prvAllowIPPacketIPv6( const IPHeader_IPv6_t * const pxIPv6Header,
                                               const NetworkBufferDescriptor_t * const pxNetworkBuffer,
                                               UBaseType_t uxHeaderLength );

#if ( ipconfigETHERNET_DRIVER_FILTERS_PACKETS == 0 )

/*
 * Return pdTRUE if either source or destination is a loopback address.
 * A loopback IP-address may only communicate internally with another
 * loopback IP-address.
 */
    BaseType_t xBadIPv6Loopback( const IPHeader_IPv6_t * const pxIPv6Header );
#endif /* ipconfigETHERNET_DRIVER_FILTERS_PACKETS == 0 */

/*
 * Check if the address is a loopback IP-address.
 */
BaseType_t xIsIPv6Loopback( const IPv6_Address_t * pxAddress );

/** @brief Handle the IPv6 extension headers. */
eFrameProcessingResult_t eHandleIPv6ExtensionHeaders( NetworkBufferDescriptor_t * const pxNetworkBuffer,
                                                      BaseType_t xDoRemove );

extern void FreeRTOS_ClearND( void );

/* Check whether this IPv6 address is an allowed multicast address or not. */
BaseType_t xIsIPv6AllowedMulticast( const IPv6_Address_t * pxIPAddress );

/* Note that 'xCompareIPv6_Address' will also check if 'pxRight' is
 * the special unicast address: ff02::1:ffnn:nnnn, where nn:nnnn are
 * the last 3 bytes of the IPv6 address. */
BaseType_t xCompareIPv6_Address( const IPv6_Address_t * pxLeft,
                                 const IPv6_Address_t * pxRight,
                                 size_t uxPrefixLength );

/* FreeRTOS_dnslookup6() returns pdTRUE when a host has been found. */
uint32_t FreeRTOS_dnslookup6( const char * pcHostName,
                              IPv6_Address_t * pxAddress_IPv6 );

/* Return IPv6 header extension order number */
BaseType_t xGetExtensionOrder( uint8_t ucProtocol,
                               uint8_t ucNextHeader );

/* *INDENT-OFF* */
#ifdef __cplusplus
    } /* extern "C" */
#endif
/* *INDENT-ON* */

#endif /* FREERTOS_IPV6_H */
