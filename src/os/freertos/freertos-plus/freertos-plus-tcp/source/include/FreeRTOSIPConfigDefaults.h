/*
 * FreeRTOS+TCP V4.3.3
 * Copyright (C) 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
 * @file FreeRTOSIPConfigDefaults.h
 * @brief File that provides default values for configuration options that are
 *        missing from FreeRTOSIPConfig.h. The complete documentation of the
 *        configuration parameters can be found here:
 *        https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html
 */

#ifndef FREERTOS_IP_CONFIG_DEFAULTS_H
#define FREERTOS_IP_CONFIG_DEFAULTS_H

#ifndef FREERTOS_CONFIG_H
    #error FreeRTOSConfig.h has not been included yet
#endif

#ifndef FREERTOS_IP_CONFIG_H
    #error FreeRTOSIPConfig.h has not been included yet
#endif

#include "FreeRTOSIPDeprecatedDefinitions.h"

#include <stdint.h>

/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/*                                  MACROS                                   */
/*===========================================================================*/

/*---------------------------------------------------------------------------*/

/*
 * MACROS details :
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html
 */

/*---------------------------------------------------------------------------*/

/*
 * Compile time assertion with zero runtime effects.
 * It will assert on 'e' not being zero, as it tries to divide by it.
 */

#ifdef static_assert
    #define STATIC_ASSERT( e )    static_assert( e, "FreeRTOS-Plus-TCP Error" )
#elif defined( _Static_assert )
    #define STATIC_ASSERT( e )    _Static_assert( e, "FreeRTOS-Plus-TCP Error" )
#else
/* MISRA Ref 20.10.1 [Lack of sizeof operator and compile time error checking] */
/* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-2010 */
/* coverity[misra_c_2012_rule_20_10_violation] */
    #define ASSERT_CONCAT_( a, b )    a ## b
    #define ASSERT_CONCAT( a, b )     ASSERT_CONCAT_( a, b )
    #ifdef __COUNTER__
        #define STATIC_ASSERT( e ) \
    enum { ASSERT_CONCAT( static_assert_, __COUNTER__ ) = ( 1 / ( int ) ( !!( e ) ) ) }
    #else
        #define STATIC_ASSERT( e ) \
    enum { ASSERT_CONCAT( assert_line_, __LINE__ ) = ( 1 / ( int ) ( !!( e ) ) ) }
    #endif
#endif /* ifdef static_assert */

/*---------------------------------------------------------------------------*/

/*
 * Used to define away static keyword for CBMC proofs
 */

#ifndef _static
    /* suppressing the use of _static as it is used for other tools like cbmc */
    /* coverity[misra_c_2012_rule_21_1_violation] */
    /* coverity[misra_c_2012_rule_21_2_violation] */
    #define _static    static
#endif

/*---------------------------------------------------------------------------*/

/*
 * Since all code is made compatible with the MISRA rules, the inline functions
 * disappear. Normally defined in portmacro.h or FreeRTOSConfig.h.
 */

#ifndef portINLINE
    #define portINLINE    inline
#endif

/*---------------------------------------------------------------------------*/

/*
 * Used to standardize macro checks since ( MACRO == 1 ) and ( MACRO != 0 )
 * are used inconsistently.
 */

#ifndef ipconfigENABLE
    #define ipconfigENABLE    ( 1 )
#endif

#ifndef ipconfigDISABLE
    #define ipconfigDISABLE    ( 0 )
#endif

#ifndef ipconfigIS_ENABLED
    #define ipconfigIS_ENABLED( x )    ( ( x ) != ipconfigDISABLE )
#endif

#ifndef ipconfigIS_DISABLED
    #define ipconfigIS_DISABLED( x )    ( ( x ) == ipconfigDISABLE )
#endif

/*---------------------------------------------------------------------------*/

/*===========================================================================*/
/*                                  MACROS                                   */
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/*                           FreeRTOSConfig CHECKS                           */
/*===========================================================================*/

/*---------------------------------------------------------------------------*/

/*
 * pdFREERTOS_ERRNO_EAFNOSUPPORT
 *
 * Address family not supported by protocol.
 *
 * Note: pdFREERTOS_ERRNO_EAFNOSUPPORT is now included in
 * FreeRTOS-Kernel/projdefs.h, defined here for backwards compatibility.
 */

#ifndef pdFREERTOS_ERRNO_EAFNOSUPPORT
    #define pdFREERTOS_ERRNO_EAFNOSUPPORT    97
#endif

#if ( INCLUDE_vTaskDelay == 0 )
    #error INCLUDE_vTaskDelay must be set to 1
#endif

#if ( INCLUDE_xTaskGetCurrentTaskHandle == 0 )
    #error INCLUDE_xTaskGetCurrentTaskHandle must be set to 1
#endif

#if ( configSUPPORT_DYNAMIC_ALLOCATION == 0 )
    #error configSUPPORT_DYNAMIC_ALLOCATION must be set to 1
#endif

#if ( configUSE_COUNTING_SEMAPHORES == 0 )
    #error configUSE_COUNTING_SEMAPHORES must be set to 1
#endif

/*---------------------------------------------------------------------------*/

/*===========================================================================*/
/*                           FreeRTOSConfig CHECKS                           */
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/*                                 IP CONFIG                                 */
/*===========================================================================*/

/*---------------------------------------------------------------------------*/

/*
 * ipconfigIPv4_BACKWARD_COMPATIBLE
 *
 * Type: BaseType_t ( ipconfigENABLE | ipconfigDISABLE )
 *
 * Enables the APIs that are backward compatible with single end point IPv4
 * version V3.x.x or older.
 */

#ifndef ipconfigIPv4_BACKWARD_COMPATIBLE
    #define ipconfigIPv4_BACKWARD_COMPATIBLE    ipconfigDISABLE
#endif

#if ( ( ipconfigIPv4_BACKWARD_COMPATIBLE != ipconfigDISABLE ) && ( ipconfigIPv4_BACKWARD_COMPATIBLE != ipconfigENABLE ) )
    #error Invalid ipconfigIPv4_BACKWARD_COMPATIBLE configuration
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigUSE_IPv4
 *
 * Type: BaseType_t ( ipconfigENABLE | ipconfigDISABLE )
 *
 * Include support for the IPv4 protocol.
 *
 * If disabled, the application must enable IPv6.
 */

#ifndef ipconfigUSE_IPv4
    #define ipconfigUSE_IPv4    ipconfigENABLE
#endif

#if ( ( ipconfigUSE_IPv4 != ipconfigDISABLE ) && ( ipconfigUSE_IPv4 != ipconfigENABLE ) )
    #error Invalid ipconfigUSE_IPv4 configuration
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigUSE_IPv6
 *
 * Type: BaseType_t ( ipconfigENABLE | ipconfigDISABLE )
 *
 * Include support for the IPv6 protocol.
 *
 * If disabled, the application must enable IPv4.
 */

#ifndef ipconfigUSE_IPv6
    #define ipconfigUSE_IPv6    ipconfigENABLE
#endif

#if ( ( ipconfigUSE_IPv6 != ipconfigDISABLE ) && ( ipconfigUSE_IPv6 != ipconfigENABLE ) )
    #error Invalid ipconfigUSE_IPv6 configuration
#endif

#if ( ipconfigIS_DISABLED( ipconfigUSE_IPv4 ) && ipconfigIS_DISABLED( ipconfigUSE_IPv6 ) )
    #error Either ipconfigUSE_IPv4 or ipconfigUSE_IPv6 must be enabled
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigUSE_RA
 *
 * Type: BaseType_t ( ipconfigENABLE | ipconfigDISABLE )
 *
 * Include support for Router Advertisement (RA).
 */

#ifndef ipconfigUSE_RA
    #define ipconfigUSE_RA    ipconfigENABLE
#endif

#if ( ( ipconfigUSE_RA != ipconfigDISABLE ) && ( ipconfigUSE_RA != ipconfigENABLE ) )
    #error Invalid ipconfigUSE_RA configuration
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigRA_SEARCH_COUNT
 *
 * Type: UBaseType_t
 * Minimum: 0
 *
 * Sets the amount of times a router solicitation message can
 * be retransmitted after timing out.
 *
 * A Router Solicitation will be sent. It will wait for ipconfigRA_SEARCH_TIME_OUT_MSEC ms.
 * When there is no response, it will be repeated ipconfigRA_SEARCH_COUNT times.
 * Then it will be checked if the chosen IP-address already exists, repeating this
 * ipconfigRA_IP_TEST_COUNT times, each time with a timeout of ipconfigRA_IP_TEST_TIME_OUT_MSEC ms.
 * Finally the end-point will go in the UP state.
 */

#ifndef ipconfigRA_SEARCH_COUNT
    #define ipconfigRA_SEARCH_COUNT    ( 3 )
#endif

#if ( ipconfigRA_SEARCH_COUNT < 0 )
    #error ipconfigRA_SEARCH_COUNT must be at least 0
#endif

#if ( ipconfigRA_SEARCH_COUNT > UINT_FAST8_MAX )
    #error ipconfigRA_SEARCH_COUNT overflows a UBaseType_t
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigRA_SEARCH_TIME_OUT_MSEC
 *
 * Type: TickType_t
 * Unit: milliseconds
 * Minimum: 0
 * Maximum: portMAX_DELAY * portTICK_PERIOD_MS
 *
 * Sets the timeout to wait for a response to a router
 * solicitation message.
 */

#ifndef ipconfigRA_SEARCH_TIME_OUT_MSEC
    #define ipconfigRA_SEARCH_TIME_OUT_MSEC    ( 10000 )
#endif

#if ( ipconfigRA_SEARCH_TIME_OUT_MSEC < 0 )
    #error ipconfigRA_SEARCH_TIME_OUT_MSEC must be at least 0
#endif

STATIC_ASSERT( ipconfigRA_SEARCH_TIME_OUT_MSEC <= ( portMAX_DELAY * portTICK_PERIOD_MS ) );

/*---------------------------------------------------------------------------*/

/*
 * ipconfigRA_IP_TEST_COUNT
 *
 * Type: UBaseType_t
 * Unit: max iterations of RA wait state process
 * Minimum: 0
 *
 * Sets the amount of times a neighbour solicitation message can be
 * retransmitted after timing out.
 */

#ifndef ipconfigRA_IP_TEST_COUNT
    #define ipconfigRA_IP_TEST_COUNT    ( 3 )
#endif

#if ( ipconfigRA_IP_TEST_COUNT < 0 )
    #error ipconfigRA_IP_TEST_COUNT must be at least 0
#endif

#if ( ipconfigRA_IP_TEST_COUNT > UINT_FAST8_MAX )
    #error ipconfigRA_IP_TEST_COUNT overflows a UBaseType_t
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigRA_IP_TEST_TIME_OUT_MSEC
 *
 * Type: TickType_t
 * Unit: milliseconds
 * Minimum: 0
 * Maximum: portMAX_DELAY * portTICK_PERIOD_MS
 *
 * Sets the timeout to wait for a response to a neighbour solicitation message.
 */

#ifndef ipconfigRA_IP_TEST_TIME_OUT_MSEC
    #define ipconfigRA_IP_TEST_TIME_OUT_MSEC    ( 1500 )
#endif

#if ( ipconfigRA_IP_TEST_TIME_OUT_MSEC < 0 )
    #error ipconfigRA_IP_TEST_TIME_OUT_MSEC must be at least 0
#endif

STATIC_ASSERT( ipconfigRA_IP_TEST_TIME_OUT_MSEC <= ( portMAX_DELAY * portTICK_PERIOD_MS ) );

/*---------------------------------------------------------------------------*/

/*
 * ipconfigENDPOINT_DNS_ADDRESS_COUNT
 *
 * Type: uint8_t
 * Unit: max count of IP addresses of DNS servers
 * Minimum: 1
 *
 * Sets the length of the array of addresses of Domain Name Servers for each
 * endpoint.
 *
 * The field ucDNSIndex in the IP parameters of a NetworkEndPoint_t will point
 * to the DNS in use. When a DNS times out, ucDNSIndex will be moved to the
 * next available DNS.
 */

#ifndef ipconfigENDPOINT_DNS_ADDRESS_COUNT
    #define ipconfigENDPOINT_DNS_ADDRESS_COUNT    ( 2U )
#endif

#if ( ipconfigENDPOINT_DNS_ADDRESS_COUNT < 1 )
    #error ipconfigENDPOINT_DNS_ADDRESS_COUNT must be at least 1
#endif

#if ( ipconfigENDPOINT_DNS_ADDRESS_COUNT > UINT8_MAX )
    #error ipconfigENDPOINT_DNS_ADDRESS_COUNT overflows a uint8_t
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigFORCE_IP_DONT_FRAGMENT
 *
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html#ipconfigFORCE_IP_DONT_FRAGMENT
 *
 * Type: BaseType_t ( ipconfigENABLE | ipconfigDISABLE )
 *
 * Sets the ipFRAGMENT_FLAGS_DONT_FRAGMENT flag in an IP header.
 *
 * When sending an IP-packet over the internet, a big packet may be split up
 * into smaller parts which are then combined by the receiver. The sender can
 * determine if this fragmentation is allowed or not.
 *
 * Note that the FreeRTOS-Plus-TCP stack does not accept received fragmented
 * packets.
 */

#ifndef ipconfigFORCE_IP_DONT_FRAGMENT
    #define ipconfigFORCE_IP_DONT_FRAGMENT    ipconfigDISABLE
#endif

#if ( ( ipconfigFORCE_IP_DONT_FRAGMENT != ipconfigDISABLE ) && ( ipconfigFORCE_IP_DONT_FRAGMENT != ipconfigENABLE ) )
    #error Invalid ipconfigFORCE_IP_DONT_FRAGMENT configuration
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigIP_PASS_PACKETS_WITH_IP_OPTIONS
 *
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html#ipconfigIP_PASS_PACKETS_WITH_IP_OPTIONS
 *
 * Type: BaseType_t ( ipconfigENABLE | ipconfigDISABLE )
 *
 * Determines if IP packets with IP options are accepted (but not processed).
 */

#ifndef ipconfigIP_PASS_PACKETS_WITH_IP_OPTIONS
    #define ipconfigIP_PASS_PACKETS_WITH_IP_OPTIONS    ipconfigENABLE
#endif

#if ( ( ipconfigIP_PASS_PACKETS_WITH_IP_OPTIONS != ipconfigDISABLE ) && ( ipconfigIP_PASS_PACKETS_WITH_IP_OPTIONS != ipconfigENABLE ) )
    #error Invalid ipconfigIP_PASS_PACKETS_WITH_IP_OPTIONS configuration
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigMAX_IP_TASK_SLEEP_TIME
 *
 * Type: TickType_t
 * Unit: Ticks
 * Minimum: 0
 * Maximum: portMAX_DELAY
 *
 * The maximum time the IP task is allowed to remain in the Blocked state if no
 * events are posted to the network event queue.
 */

#ifndef ipconfigMAX_IP_TASK_SLEEP_TIME
    #define ipconfigMAX_IP_TASK_SLEEP_TIME    pdMS_TO_TICKS( 10000 )
#endif

STATIC_ASSERT( ipconfigMAX_IP_TASK_SLEEP_TIME <= portMAX_DELAY );

/*---------------------------------------------------------------------------*/

/*===========================================================================*/
/*                                 IP CONFIG                                 */
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/*                               DRIVER CONFIG                               */
/*===========================================================================*/

/*---------------------------------------------------------------------------*/

/*
 * ipconfigBUFFER_PADDING
 *
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html#ipconfigBUFFER_PADDING
 *
 * Type: size_t
 * Unit: bytes
 * Minimum: 0
 *
 * Advanced driver implementation use only.
 *
 * When the application requests a network buffer, the size of the network
 * buffer is specified by the application writer, but the size of the network
 * buffer actually obtained is increased by ipconfigBUFFER_PADDING bytes. The
 * first ipconfigBUFFER_PADDING bytes of the buffer is then used to hold
 * metadata about the buffer, and the area that actually stores the data
 * follows the metadata. This mechanism is transparent to the user as the user
 * only sees a pointer to the area within the buffer actually used to hold
 * network data.
 *
 * Some network hardware has very specific byte alignment requirements, so
 * ipconfigBUFFER_PADDING is provided as a configurable parameter to allow the
 * writer of the network driver to influence the alignment of the start of the
 * data that follows the metadata.
 *
 * When defined as zero ( default ), the driver will determine the optimal
 * padding as:
 *
 *     #define ipBUFFER_PADDING    ( 8U + ipconfigPACKET_FILLER_SIZE )
 *
 * See ipconfigPACKET_FILLER_SIZE.
 */

#ifndef ipconfigBUFFER_PADDING
    #define ipconfigBUFFER_PADDING    ( 0 )
#endif

#if ( ipconfigBUFFER_PADDING < 0 )
    #error ipconfigBUFFER_PADDING must be at least 0
#endif

#if ( ipconfigBUFFER_PADDING > SIZE_MAX )
    #error ipconfigBUFFER_PADDING overflows a size_t
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigPACKET_FILLER_SIZE
 *
 * Type: size_t
 * Unit: bytes
 * Minimum: 0
 *
 * In most projects, network buffers are 32-bit aligned plus 16 bits.
 * The two extra bytes are called "filler bytes". They make sure that the
 * IP-header starts at a 32-bit aligned address. That makes the code
 * very efficient and easy to maintain. An 'uint32_t' can be assigned/
 * changed without having to worry about alignment.
 *
 * See ipconfigBUFFER_PADDING.
 */

#ifndef ipconfigPACKET_FILLER_SIZE
    #define ipconfigPACKET_FILLER_SIZE    ( 2 )
#endif

#if ( ipconfigPACKET_FILLER_SIZE < 0 )
    #error ipconfigPACKET_FILLER_SIZE must be at least 0
#endif

#if ( ipconfigPACKET_FILLER_SIZE > SIZE_MAX )
    #error ipconfigPACKET_FILLER_SIZE overflows a size_t
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigBYTE_ORDER
 *
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html#ipconfigBYTE_ORDER
 *
 * Type: BaseType_t ( pdFREERTOS_LITTLE_ENDIAN | pdFREERTOS_BIG_ENDIAN )
 *
 * If the microcontroller on which FreeRTOS-Plus-TCP is running is big endian
 * then ipconfigBYTE_ORDER must be set to pdFREERTOS_BIG_ENDIAN. If the
 * microcontroller is little endian then ipconfigBYTE_ORDER must be set to
 * pdFREERTOS_LITTLE_ENDIAN. The Byte Order and Endian section of the Embedded
 * Networking Basics and Glossary page provides an explanation of byte order
 * considerations in IP networks.
 */

#ifndef ipconfigBYTE_ORDER
    #error ipconfigBYTE_ORDER must be defined
#endif

#if ( ( ipconfigBYTE_ORDER != pdFREERTOS_LITTLE_ENDIAN ) && ( ipconfigBYTE_ORDER != pdFREERTOS_BIG_ENDIAN ) )
    #error ipconfigBYTE_ORDER should be pdFREERTOS_LITTLE_ENDIAN or pdFREERTOS_BIG_ENDIAN
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigDRIVER_INCLUDED_RX_IP_CHECKSUM
 *
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html#ipconfigDRIVER_INCLUDED_RX_IP_CHECKSUM
 *
 * Type: BaseType_t ( ipconfigENABLE | ipconfigDISABLE )
 *
 * When ipconfigDRIVER_INCLUDED_RX_IP_CHECKSUM is enabled, the network interface
 * is responsible for checking the checksums of the incoming packets. If hardware
 * supports checking TCP checksum only, the network interface layer should handle
 * the same for other protocols, such as IP/UDP/ICMP/etc, and give the checksum
 * verified packets to the FreeRTOS-plus-TCP stack. otherwise set
 * ipconfigDRIVER_INCLUDED_RX_IP_CHECKSUM to ipconfigDISABLE.
 *
 * Throughput and processor load are greatly improved by implementing drivers
 * that make use of hardware checksum calculations, so-called "checksum offloading".
 *
 * Note: From FreeRTOS-Plus-TCP V2.3.0, the length is checked in software even
 * when it has already been checked in hardware.
 */

#ifndef ipconfigDRIVER_INCLUDED_RX_IP_CHECKSUM
    #define ipconfigDRIVER_INCLUDED_RX_IP_CHECKSUM    ipconfigDISABLE
#endif

#if ( ( ipconfigDRIVER_INCLUDED_RX_IP_CHECKSUM != ipconfigDISABLE ) && ( ipconfigDRIVER_INCLUDED_RX_IP_CHECKSUM != ipconfigENABLE ) )
    #error Invalid ipconfigDRIVER_INCLUDED_RX_IP_CHECKSUM configuration
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigDRIVER_INCLUDED_TX_IP_CHECKSUM
 *
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html#ipconfigDRIVER_INCLUDED_TX_IP_CHECKSUM
 *
 * Type: BaseType_t ( ipconfigENABLE | ipconfigDISABLE )
 *
 * If the network driver or network hardware is calculating the IP, TCP and UDP
 * checksums of outgoing packets then set
 * ipconfigDRIVER_INCLUDED_TX_IP_CHECKSUM to 1, otherwise set
 * ipconfigDRIVER_INCLUDED_TX_IP_CHECKSUM to 0.
 *
 * Throughput and processor load are greatly improved by implementing drivers
 * that make use of hardware checksum calculations.
 */

#ifndef ipconfigDRIVER_INCLUDED_TX_IP_CHECKSUM
    #define ipconfigDRIVER_INCLUDED_TX_IP_CHECKSUM    ipconfigDISABLE
#endif

#if ( ( ipconfigDRIVER_INCLUDED_TX_IP_CHECKSUM != ipconfigDISABLE ) && ( ipconfigDRIVER_INCLUDED_TX_IP_CHECKSUM != ipconfigENABLE ) )
    #error Invalid ipconfigDRIVER_INCLUDED_TX_IP_CHECKSUM configuration
#endif

/*---------------------------------------------------------------------------*/

/*
 * A MISRA note: The macros 'ipconfigETHERNET_DRIVER_FILTERS_FRAME_TYPES'
 * and 'ipconfigETHERNET_DRIVER_FILTERS_PACKETS' are too long: the first 32
 * bytes are equal, which might cause problems for some compilers.
 */

/*
 * ipconfigETHERNET_DRIVER_FILTERS_FRAME_TYPES
 *
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html#ipconfigETHERNET_DRIVER_FILTERS_FRAME_TYPES
 *
 * Type: BaseType_t ( ipconfigENABLE | ipconfigDISABLE )
 *
 * Ethernet/hardware MAC addresses are used to address Ethernet frames. If the
 * network driver or hardware is discarding packets that do not contain a MAC
 * address of interest then set ipconfigETHERNET_DRIVER_FILTERS_FRAME_TYPES to
 * 1. Otherwise set ipconfigETHERNET_DRIVER_FILTERS_FRAME_TYPES to 0.
 *
 * Throughput and processor load are greatly improved by implementing network
 * address filtering in hardware. Most network interfaces allow multiple MAC
 * addresses to be defined so filtering can allow through the unique hardware
 * address of the node, the broadcast address, and various multicast addresses.
 *
 * When disabled, the IP-task will call 'eConsiderFrameForProcessing()'
 * to check incoming packets.
 */

#ifndef ipconfigETHERNET_DRIVER_FILTERS_FRAME_TYPES
    #define ipconfigETHERNET_DRIVER_FILTERS_FRAME_TYPES    ipconfigENABLE
#endif

#if ( ( ipconfigETHERNET_DRIVER_FILTERS_FRAME_TYPES != ipconfigDISABLE ) && ( ipconfigETHERNET_DRIVER_FILTERS_FRAME_TYPES != ipconfigENABLE ) )
    #error Invalid ipconfigETHERNET_DRIVER_FILTERS_FRAME_TYPES configuration
#endif

/*---------------------------------------------------------------------------*/

/*
 * A MISRA note: The macros 'ipconfigETHERNET_DRIVER_FILTERS_FRAME_TYPES'
 * and 'ipconfigETHERNET_DRIVER_FILTERS_PACKETS' are too long: the first 32
 * bytes are equal, which might cause problems for some compilers.
 */

/*
 * ipconfigETHERNET_DRIVER_FILTERS_PACKETS
 *
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html#ipconfigETHERNET_DRIVER_FILTERS_PACKETS
 *
 * Type: BaseType_t ( ipconfigENABLE | ipconfigDISABLE )
 *
 * For expert users only.
 *
 * Whereas ipconfigETHERNET_DRIVER_FILTERS_FRAME_TYPES is used to specify
 * whether or not the network driver or hardware filters Ethernet frames,
 * ipconfigETHERNET_DRIVER_FILTERS_PACKETS is used to specify whether or not
 * the network driver filters the IP, UDP or TCP data within the Ethernet
 * frame.
 *
 * The TCP/IP stack is only interested in receiving data that is either
 * addresses to a socket (IP address and port number) on the local node, or is
 * a broadcast or multicast packet. Throughput and process load can be greatly
 * improved by preventing packets that do not meet these criteria from being
 * sent to the TCP/IP stack. FreeRTOS provides some features that allow such
 * filtering to take place in the network driver. For example,
 * xPortHasUDPSocket() can be used as follows:
 *
 * if( ( xPortHasUdpSocket( xUDPHeader->usDestinationPort ) )
 *     #if( ipconfigUSE_DNS == 1 ) * DNS is also UDP. *
 *         || ( xUDPHeader->usSourcePort == FreeRTOS_ntohs( ipDNS_PORT ) )
 *     #endif
 *     #if( ipconfigUSE_LLMNR == 1 ) * LLMNR is also UDP. *
 *         || ( xUDPHeader->usDestinationPort == FreeRTOS_ntohs( ipLLMNR_PORT ) )
 *     #endif
 *     #if( ipconfigUSE_NBNS == 1 ) * NBNS is also UDP. *
 *         || ( xUDPHeader->usDestinationPort == FreeRTOS_ntohs( ipNBNS_PORT ) )
 *     #endif
 *    )
 * {
 *     * Forward packet to the IP-stack. *
 * }
 * else
 * {
 *     * Discard the UDP packet. *
 * }
 *
 * When disabled, the IP-task will perform sanity checks on the IP-header,
 * also checking the target IP address. Also when disabled, xPortHasUDPSocket()
 * won't be included. That means that the IP-task can access the
 * 'xBoundUDPSocketsList' without locking.
 */

#ifndef ipconfigETHERNET_DRIVER_FILTERS_PACKETS
    #define ipconfigETHERNET_DRIVER_FILTERS_PACKETS    ipconfigDISABLE
#endif

#if ( ( ipconfigETHERNET_DRIVER_FILTERS_PACKETS != ipconfigDISABLE ) && ( ipconfigETHERNET_DRIVER_FILTERS_PACKETS != ipconfigENABLE ) )
    #error Invalid ipconfigETHERNET_DRIVER_FILTERS_PACKETS configuration
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigETHERNET_MINIMUM_PACKET_BYTES
 *
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html#ipconfigETHERNET_MINIMUM_PACKET_BYTES
 *
 * Type: size_t
 * Unit: minimum size of packet in bytes
 * Minimum: 0
 *
 * When the device is connected to a LAN, it is strongly recommended to give
 * each outgoing packet a minimum length of 60 bytes (plus 4 bytes CRC). The
 * macro ipconfigETHERNET_MINIMUM_PACKET_BYTES determines the minimum length.
 * By default, it is defined as zero, meaning that packets will be sent as they
 * are. A minimum packet length is needed to be able to detect collisions of
 * short packets as well. By default, packets of any size can be sent.
 */

#ifndef ipconfigETHERNET_MINIMUM_PACKET_BYTES
    #define ipconfigETHERNET_MINIMUM_PACKET_BYTES    ( 0 )
#endif

#if ( ipconfigETHERNET_MINIMUM_PACKET_BYTES < 0 )
    #error ipconfigETHERNET_MINIMUM_PACKET_BYTES must be at least 0
#endif

#if ( ipconfigETHERNET_MINIMUM_PACKET_BYTES > SIZE_MAX )
    #error ipconfigETHERNET_MINIMUM_PACKET_BYTES overflows a size_t
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigFILTER_OUT_NON_ETHERNET_II_FRAMES
 *
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html#ipconfigFILTER_OUT_NON_ETHERNET_II_FRAMES
 *
 * Type: BaseType_t ( ipconfigENABLE | ipconfigDISABLE )
 *
 * If ipconfigFILTER_OUT_NON_ETHERNET_II_FRAMES is set to 1 then Ethernet
 * frames that are not in Ethernet II format will be dropped. This option
 * is included for potential future IP stack developments.
 *
 * When enabled, the function 'eConsiderFrameForProcessing()' will also
 * check if the Ethernet frame type is acceptable.
 */

#ifndef ipconfigFILTER_OUT_NON_ETHERNET_II_FRAMES
    #define ipconfigFILTER_OUT_NON_ETHERNET_II_FRAMES    ipconfigENABLE
#endif

#if ( ( ipconfigFILTER_OUT_NON_ETHERNET_II_FRAMES != ipconfigDISABLE ) && ( ipconfigFILTER_OUT_NON_ETHERNET_II_FRAMES != ipconfigENABLE ) )
    #error Invalid ipconfigFILTER_OUT_NON_ETHERNET_II_FRAMES configuration
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigNETWORK_MTU
 *
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html#ipconfigNETWORK_MTU
 *
 * Type: size_t
 * Unit: bytes
 * Minimum: 46
 *
 * The MTU is the maximum number of bytes the payload of a network frame can
 * contain. For normal Ethernet V2 frames the maximum MTU is 1500 (although a
 * lower number may be required for Internet routing). Setting a lower value
 * can save RAM, depending on the buffer management scheme used.
 */

#ifndef ipconfigNETWORK_MTU
    #define ipconfigNETWORK_MTU    ( 1500 )
#endif

#if ( ipconfigNETWORK_MTU < 46 )
    #error ipconfigNETWORK_MTU must be at least 46
#endif

#if ( ipconfigNETWORK_MTU > SIZE_MAX )
    #error ipconfigNETWORK_MTU overflows a size_t
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS
 *
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html#ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS
 *
 * Type: size_t
 * Unit: Count of network buffers
 * Minimum: 1
 *
 * Defines the total number of network buffers that are available to the TCP/IP
 * stack. The total number of network buffers is limited to ensure the total
 * amount of RAM that can be consumed by the TCP/IP stack is capped to a
 * pre-determinable value. How the storage area is actually allocated to the
 * network buffer structures is not fixed, but part of the portable layer.
 * The simplest scheme simply allocates the exact amount of storage as it is
 * required.
 */

#ifndef ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS
    #define ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS    ( 45 )
#endif

#if ( ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS < 1 )
    #error ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS must be at least 1
#endif

#if ( ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS > SIZE_MAX )
    #error ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS overflows a size_t
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigUSE_LINKED_RX_MESSAGES
 *
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html#ipconfigUSE_LINKED_RX_MESSAGES
 *
 * Type: BaseType_t ( ipconfigENABLE | ipconfigDISABLE )
 *
 * Advanced users only.
 *
 * When enabled it is possible to reduce CPU load during periods of heavy
 * network traffic by linking multiple received packets together, then passing
 * all the linked packets to the IP RTOS task in one go.
 *
 * By default packets will be sent one-by-one. If 'ipconfigUSE_LINKED_RX_MESSAGES'
 * is non-zero, each message buffer gets a 'pxNextBuffer' field, to that linked
 * packets can be passed to the IP-task in a single call to 'xSendEventStructToIPTask()'.
 * Note that this only works if the Network Interface also supports this
 * option.
 */

#ifndef ipconfigUSE_LINKED_RX_MESSAGES
    #define ipconfigUSE_LINKED_RX_MESSAGES    ipconfigDISABLE
#endif

#if ( ( ipconfigUSE_LINKED_RX_MESSAGES != ipconfigDISABLE ) && ( ipconfigUSE_LINKED_RX_MESSAGES != ipconfigENABLE ) )
    #error Invalid ipconfigUSE_LINKED_RX_MESSAGES configuration
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigZERO_COPY_RX_DRIVER
 *
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html#ipconfigZERO_COPY_RX_DRIVER
 *
 * Type: BaseType_t ( ipconfigENABLE | ipconfigDISABLE )
 *
 * Advanced users only.
 *
 * When enabled the network interface will assign network buffers
 * NetworkBufferDescriptor_t::pucEthernetBuffer to the DMA of the EMAC.
 * When a packet is received, no data is copied. Instead, the
 * buffer is sent directly to the IP-task. If the RX zero-copy option is
 * disabled, every received packet will be copied from the DMA buffer to the
 * network buffer of type NetworkBufferDescriptor_t.
 */

#ifndef ipconfigZERO_COPY_RX_DRIVER
    #define ipconfigZERO_COPY_RX_DRIVER    ipconfigDISABLE
#endif

#if ( ( ipconfigZERO_COPY_RX_DRIVER != ipconfigDISABLE ) && ( ipconfigZERO_COPY_RX_DRIVER != ipconfigENABLE ) )
    #error Invalid ipconfigZERO_COPY_RX_DRIVER configuration
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigZERO_COPY_TX_DRIVER
 *
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html#ipconfigZERO_COPY_TX_DRIVER
 *
 * Type: BaseType_t ( ipconfigENABLE | ipconfigDISABLE )
 *
 * Advanced users only.
 *
 * When enabled the driver function xNetworkInterfaceOutput() will always be
 * called with its bReleaseAfterSend parameter set to pdTRUE - meaning it is
 * always the driver that is responsible for freeing the network buffer and
 * network buffer descriptor.
 *
 * This is useful if the driver implements a zero-copy scheme whereby the
 * packet data is sent directly from within the network buffer (for example by
 * pointing a DMA descriptor at the data within the network buffer), instead of
 * copying the data out of the network buffer before the data is sent (for
 * example by copying the data into a separate pre-allocated DMA descriptor).
 * In such cases the driver needs to take ownership of the network buffer
 * because the network buffer can only be freed after the data has actually
 * been transmitted - which might be some time after the
 * xNetworkInterfaceOutput() function returns. See the examples on the Porting
 * FreeRTOS to a Different Microcontroller documentation page for worked
 * examples.
 */

#ifndef ipconfigZERO_COPY_TX_DRIVER
    #define ipconfigZERO_COPY_TX_DRIVER    ipconfigDISABLE
#endif

#if ( ( ipconfigZERO_COPY_TX_DRIVER != ipconfigDISABLE ) && ( ipconfigZERO_COPY_TX_DRIVER != ipconfigENABLE ) )
    #error Invalid ipconfigZERO_COPY_TX_DRIVER configuration
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigSUPPORT_NETWORK_DOWN_EVENT
 *
 * Type: BaseType_t ( ipconfigENABLE | ipconfigDISABLE )
 *
 * Set to 1 if you want to receive eNetworkDown notification via vApplicationIPNetworkEventHook_Multi() callback.
 * Not all drivers support this feature.
 */

#ifndef ipconfigSUPPORT_NETWORK_DOWN_EVENT
    #define ipconfigSUPPORT_NETWORK_DOWN_EVENT    ipconfigDISABLE
#endif

#if ( ( ipconfigSUPPORT_NETWORK_DOWN_EVENT != ipconfigDISABLE ) && ( ipconfigSUPPORT_NETWORK_DOWN_EVENT != ipconfigENABLE ) )
    #error Invalid ipconfigSUPPORT_NETWORK_DOWN_EVENT configuration
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigPHY_LS_HIGH_CHECK_TIME_MS
 *
 * Type: uint32_t
 * Unit: milliseconds
 *
 * Interval in which to check if the LinkStatus in the PHY is still high after
 * not receiving packets.
 */

#ifndef ipconfigPHY_LS_HIGH_CHECK_TIME_MS
    #define ipconfigPHY_LS_HIGH_CHECK_TIME_MS    ( 15000 )
#endif

#if ( ipconfigPHY_LS_HIGH_CHECK_TIME_MS < 0 )
    #error ipconfigPHY_LS_HIGH_CHECK_TIME_MS must be at least 0
#endif

STATIC_ASSERT( pdMS_TO_TICKS( ipconfigPHY_LS_HIGH_CHECK_TIME_MS ) <= portMAX_DELAY );

/*---------------------------------------------------------------------------*/

/*
 * ipconfigPHY_LS_LOW_CHECK_TIME_MS
 *
 * Type: uint32_t
 * Unit: milliseconds
 *
 * Interval in which to check if the LinkStatus in the PHY is still low.
 */

#ifndef ipconfigPHY_LS_LOW_CHECK_TIME_MS
    #define ipconfigPHY_LS_LOW_CHECK_TIME_MS    ( 1000 )
#endif

#if ( ipconfigPHY_LS_LOW_CHECK_TIME_MS < 0 )
    #error ipconfigPHY_LS_LOW_CHECK_TIME_MS must be at least 0
#endif

STATIC_ASSERT( pdMS_TO_TICKS( ipconfigPHY_LS_LOW_CHECK_TIME_MS ) <= portMAX_DELAY );

/*---------------------------------------------------------------------------*/

/*
 * ipconfigHAS_TX_CRC_OFFLOADING
 *
 * Type: BaseType_t ( ipconfigENABLE | ipconfigDISABLE )
 */

#ifndef ipconfigHAS_TX_CRC_OFFLOADING
    #define ipconfigHAS_TX_CRC_OFFLOADING    ipconfigDISABLE
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigPHY_MAX_PORTS
 *
 * Type: size_t
 * Unit: count of ports
 * Minimum: 1
 * Maximum: 32
 *
 * There can be at most 32 PHY ports, but in most cases there are 4 or less.
 */

#ifndef ipconfigPHY_MAX_PORTS
    #define ipconfigPHY_MAX_PORTS    ( 4 )
#endif

#if ( ipconfigPHY_MAX_PORTS < 1 )
    #error ipconfigPHY_MAX_PORTS must be at least 1
#endif

#if ( ipconfigPHY_MAX_PORTS > 32 )
    #error ipconfigPHY_MAX_PORTS must be at most 32
#endif

/*---------------------------------------------------------------------------*/

/*===========================================================================*/
/*                               DRIVER CONFIG                               */
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/*                             TCP/IP TASK CONFIG                            */
/*===========================================================================*/

/*---------------------------------------------------------------------------*/

/*
 * ipconfigEVENT_QUEUE_LENGTH
 *
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html#ipconfigEVENT_QUEUE_LENGTH
 *
 * Type: size_t
 * Unit: count of queue spaces
 * Minimum: 1
 *
 * A FreeRTOS queue is used to send events from application tasks to the IP
 * stack. ipconfigEVENT_QUEUE_LENGTH sets the maximum number of events that can
 * be queued for processing at any one time. The event queue must be a minimum
 * of 5 greater than the total number of network buffers.
 *
 * The actual number of items in the event queue can be monitored.
 * See 'ipconfigCHECK_IP_QUEUE_SPACE' described here below.
 *
 * Tasks should never have to wait for space in the 'xNetworkEventQueue'.
 */

#ifndef ipconfigEVENT_QUEUE_LENGTH
    #define ipconfigEVENT_QUEUE_LENGTH    ( ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS + 5 )
#endif

#if ( ipconfigEVENT_QUEUE_LENGTH < ( ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS + 5 ) )
    #error ipconfigEVENT_QUEUE_LENGTH must be at least ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS + 5
#endif

#if ( ipconfigEVENT_QUEUE_LENGTH > SIZE_MAX )
    #error ipconfigEVENT_QUEUE_LENGTH overflows a size_t
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigIP_TASK_PRIORITY
 *
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html#ipconfigIP_TASK_PRIORITY
 *
 * Type: UBaseType_t
 * Unit: task priority
 * Minimum: 0
 * Maximum: configMAX_PRIORITIES - 1
 *
 * The TCP/IP stack executes in its own RTOS task (although any application
 * RTOS task can make use of its services through the published sockets API).
 * ipconfigIP_TASK_PRIORITY sets the priority of the RTOS task that executes
 * the TCP/IP stack.
 *
 * The priority is a standard FreeRTOS task priority so it can take any value
 * from 0 (the lowest priority) to (configMAX_PRIORITIES - 1)
 * (the highest priority). configMAX_PRIORITIES is a standard FreeRTOS
 * configuration parameter defined in FreeRTOSConfig.h, not FreeRTOSIPConfig.h.
 *
 * Consideration needs to be given as to the priority assigned to the RTOS task
 * executing the TCP/IP stack relative to the priority assigned to tasks that
 * use the TCP/IP stack.
 *
 * It is recommended to assign the following task priorities:
 *
 * Higher : EMAC task "Deferred interrupt handler"
 * Medium : IP-task
 * Lower  : User tasks that make use of the TCP/IP stack
 */

#ifndef ipconfigIP_TASK_PRIORITY
    #define ipconfigIP_TASK_PRIORITY    ( configMAX_PRIORITIES - 2 )
#endif

#if ( ipconfigIP_TASK_PRIORITY < 0 )
    #error ipconfigIP_TASK_PRIORITY must be at least 0
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigIP_TASK_STACK_SIZE_WORDS
 *
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html#ipconfigIP_TASK_STACK_SIZE_WORDS
 *
 * Type: size_t
 * Unit: words
 * Minimum: configMINIMAL_STACK_SIZE
 *
 * The size, in words (not bytes), of the stack allocated to the
 * FreeRTOS-Plus-TCP RTOS task. FreeRTOS includes optional stack overflow
 * detection.
 */

#ifndef ipconfigIP_TASK_STACK_SIZE_WORDS
    #define ipconfigIP_TASK_STACK_SIZE_WORDS    configMINIMAL_STACK_SIZE
#endif

STATIC_ASSERT( ipconfigIP_TASK_STACK_SIZE_WORDS >= configMINIMAL_STACK_SIZE );

STATIC_ASSERT( ipconfigIP_TASK_STACK_SIZE_WORDS <= SIZE_MAX );

/*---------------------------------------------------------------------------*/

/*
 * ipconfigPROCESS_CUSTOM_ETHERNET_FRAMES
 *
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html#ipconfigPROCESS_CUSTOM_ETHERNET_FRAMES
 *
 * Type: BaseType_t ( ipconfigENABLE | ipconfigDISABLE )
 *
 * Enables usage of an application defined hook to process any unknown frame,
 * that is, any frame that expects ARP or IP. If set to 1, the user must define
 * eApplicationProcessCustomFrameHook function which will be called by the stack
 * for any frame with an unsupported EtherType.
 *
 * Function prototype:
 *
 * eFrameProcessingResult_t eApplicationProcessCustomFrameHook( NetworkBufferDescriptor_t * const pxNetworkBuffer )
 */

#ifndef ipconfigPROCESS_CUSTOM_ETHERNET_FRAMES
    #define ipconfigPROCESS_CUSTOM_ETHERNET_FRAMES    ipconfigDISABLE
#endif

#if ( ( ipconfigPROCESS_CUSTOM_ETHERNET_FRAMES != ipconfigDISABLE ) && ( ipconfigPROCESS_CUSTOM_ETHERNET_FRAMES != ipconfigENABLE ) )
    #error Invalid ipconfigPROCESS_CUSTOM_ETHERNET_FRAMES configuration
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigUSE_NETWORK_EVENT_HOOK
 *
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html#ipconfigUSE_NETWORK_EVENT_HOOK
 *
 * Type: BaseType_t ( ipconfigENABLE | ipconfigDISABLE )
 *
 * Enables usage of a hook to process network events. The function will be called when
 * the network goes up and when it goes down.
 *
 * This hook is affected by ipconfigIPv4_BACKWARD_COMPATIBLE.
 *
 * Function prototype if ipconfigIPv4_BACKWARD_COMPATIBLE is enabled:
 *
 * void vApplicationIPNetworkEventHook( eIPCallbackEvent_t eNetworkEvent )
 *
 * Function prototype by default:
 *
 * void vApplicationIPNetworkEventHook_Multi( eIPCallbackEvent_t eNetworkEvent,
 *                                            struct xNetworkEndPoint * pxEndPoint )
 */

#ifndef ipconfigUSE_NETWORK_EVENT_HOOK
    #define ipconfigUSE_NETWORK_EVENT_HOOK    ipconfigDISABLE
#endif

#if ( ( ipconfigUSE_NETWORK_EVENT_HOOK != ipconfigDISABLE ) && ( ipconfigUSE_NETWORK_EVENT_HOOK != ipconfigENABLE ) )
    #error Invalid ipconfigUSE_NETWORK_EVENT_HOOK configuration
#endif

/*---------------------------------------------------------------------------*/

/*===========================================================================*/
/*                             TCP/IP TASK CONFIG                            */
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/*                                TCP CONFIG                                 */
/*===========================================================================*/

/*---------------------------------------------------------------------------*/

/*
 * ipconfigUSE_TCP
 *
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html#ipconfigUSE_TCP
 *
 * Type: BaseType_t ( ipconfigENABLE | ipconfigDISABLE )
 *
 * Include support for TCP.
 */

#ifndef ipconfigUSE_TCP
    #define ipconfigUSE_TCP    ipconfigENABLE
#endif

#if ( ( ipconfigUSE_TCP != ipconfigDISABLE ) && ( ipconfigUSE_TCP != ipconfigENABLE ) )
    #error Invalid ipconfigUSE_TCP configuration
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigIGNORE_UNKNOWN_PACKETS
 *
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html#ipconfigIGNORE_UNKNOWN_PACKETS
 *
 * Type: BaseType_t ( ipconfigENABLE | ipconfigDISABLE )
 *
 * Advanced users only.
 *
 * Prevents sending RESET responses to TCP packets that have a bad or unknown
 * destination. This is an option used for testing.  It is recommended to
 * define it as '0'.
 */

#ifndef ipconfigIGNORE_UNKNOWN_PACKETS
    #define ipconfigIGNORE_UNKNOWN_PACKETS    ipconfigDISABLE
#endif

#if ( ( ipconfigIGNORE_UNKNOWN_PACKETS != ipconfigDISABLE ) && ( ipconfigIGNORE_UNKNOWN_PACKETS != ipconfigENABLE ) )
    #error Invalid ipconfigIGNORE_UNKNOWN_PACKETS configuration
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigTCP_HANG_PROTECTION
 *
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html#ipconfigTCP_HANG_PROTECTION
 *
 * Type: BaseType_t ( ipconfigENABLE | ipconfigDISABLE )
 *
 * Enables automatic closure of a TCP socket after a timeout of no status
 * changes. It can help reduce the impact of SYN floods.
 * When a SYN packet comes in, it will first be checked if there is a listening
 * socket for that port number. If not, it will be replied to with a RESET packet.
 * If there is a listing socket for that port number, a new socket will be created.
 * This socket will be owned temporarily by the IP-task.  Only when the SYN/ACK
 * handshake is finished, the new socket will be passed to the application,
 * resulting in a successful call to FreeRTOS_accept().
 * The option 'ipconfigTCP_HANG_PROTECTION' will make sure that the socket will be
 * deleted in case the SYN-handshake doesn't come to a good end.
 *
 * See ipconfigTCP_HANG_PROTECTION_TIME
 */

#ifndef ipconfigTCP_HANG_PROTECTION
    #define ipconfigTCP_HANG_PROTECTION    ipconfigENABLE
#endif

#if ( ( ipconfigTCP_HANG_PROTECTION != ipconfigDISABLE ) && ( ipconfigTCP_HANG_PROTECTION != ipconfigENABLE ) )
    #error Invalid ipconfigTCP_HANG_PROTECTION configuration
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigTCP_HANG_PROTECTION_TIME
 *
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html#ipconfigTCP_HANG_PROTECTION_TIME
 *
 * Type: TickType_t
 * Unit: seconds
 * Minimum: 0
 * Maximum: portMAX_DELAY / configTICK_RATE_HZ
 *
 * If ipconfigTCP_HANG_PROTECTION is set to 1 then
 * ipconfigTCP_HANG_PROTECTION_TIME sets the interval in seconds
 * between the status of a socket last changing and the anti-hang
 * mechanism marking the socket as closed. It is the maximum time that a socket
 * stays in one of these "in-between" states: eCONNECT_SYN, eSYN_FIRST,
 * eSYN_RECEIVED, eFIN_WAIT_1, eFIN_WAIT_2, eCLOSING, eLAST_ACK, or eTIME_WAIT.
 */

#ifndef ipconfigTCP_HANG_PROTECTION_TIME
    #define ipconfigTCP_HANG_PROTECTION_TIME    ( 30 )
#endif

#if ( ipconfigTCP_HANG_PROTECTION_TIME < 0 )
    #error ipconfigTCP_HANG_PROTECTION_TIME must be at least 0
#endif

STATIC_ASSERT( ipconfigTCP_HANG_PROTECTION_TIME <= ( portMAX_DELAY / configTICK_RATE_HZ ) );

/*---------------------------------------------------------------------------*/

/*
 * ipconfigTCP_KEEP_ALIVE
 *
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html#ipconfigTCP_KEEP_ALIVE
 *
 * Type: BaseType_t ( ipconfigENABLE | ipconfigDISABLE )
 *
 * Sockets that are connected but do not transmit any data for an extended
 * period can be disconnected by routers or firewalls that time out. This
 * can be avoided at the application level by ensuring the application
 * periodically sends a packet. Alternatively FreeRTOS-Plus-TCP can be
 * configured to automatically send keep alive messages when it detects
 * that a connection is dormant. Note that, while having FreeRTOS-Plus-TCP
 * automatically send keep alive messages is the more convenient method, it
 * is also the least reliable method because some routers will discard keep
 * alive messages.
 *
 * Set ipconfigTCP_KEEP_ALIVE to 1 to have FreeRTOS-Plus-TCP periodically
 * send keep alive messages on connected but dormant sockets. Set
 * ipconfigTCP_KEEP_ALIVE to 0 to prevent the automatic transmission of
 * keep alive messages.
 *
 * If FreeRTOS-Plus-TCP does not receive a reply to a keep alive message
 * then the connection will be broken and the socket will be marked as
 * closed. Subsequent FreeRTOS_recv() calls on the socket will return
 * -pdFREERTOS_ERRNO_ENOTCONN.
 */

#ifndef ipconfigTCP_KEEP_ALIVE
    #define ipconfigTCP_KEEP_ALIVE    ipconfigDISABLE
#endif

#if ( ( ipconfigTCP_KEEP_ALIVE != ipconfigDISABLE ) && ( ipconfigTCP_KEEP_ALIVE != ipconfigENABLE ) )
    #error Invalid ipconfigTCP_KEEP_ALIVE configuration
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigTCP_KEEP_ALIVE_INTERVAL
 *
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html#ipconfigTCP_KEEP_ALIVE_INTERVAL
 *
 * Type: TickType_t
 * Unit: Seconds
 * Minimum: 0
 * Maximum: portMAX_DELAY / configTICK_RATE_HZ
 *
 * If ipconfigTCP_KEEP_ALIVE is set to 1 then
 * ipconfigTCP_KEEP_ALIVE_INTERVAL sets the interval in seconds between
 * successive keep alive messages. Keep alive messages are not sent at
 * all unless ipconfigTCP_KEEP_ALIVE_INTERVAL seconds have passed since
 * the last packet was sent or received.
 */

#ifndef ipconfigTCP_KEEP_ALIVE_INTERVAL
    #define ipconfigTCP_KEEP_ALIVE_INTERVAL    ( 20 )
#endif

#if ( ipconfigTCP_KEEP_ALIVE_INTERVAL < 0 )
    #error ipconfigTCP_KEEP_ALIVE_INTERVAL must be at least 0
#endif

STATIC_ASSERT( ipconfigTCP_KEEP_ALIVE_INTERVAL <= ( portMAX_DELAY / configTICK_RATE_HZ ) );

/*---------------------------------------------------------------------------*/

/*
 * ipconfigTCP_MSS
 *
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html#ipconfigTCP_MSS
 *
 * Type: size_t
 * Unit: bytes
 * Minimum: 536 ( tcpMINIMUM_SEGMENT_LENGTH )
 *
 * Sets the MSS value (in bytes) for all TCP packets.
 *
 * Note that FreeRTOS-Plus-TCP contains checks that the defined
 * ipconfigNETWORK_MTU and ipconfigTCP_MSS values are consistent with each
 * other.
 *
 * The default definition of 'ipconfigTCP_MSS' works well for most projects.
 * Note that IPv6 headers are larger than IPv4 headers, leaving less space for
 * the TCP payload. In prvSocketSetMSS_IPV6(), 20 bytes will be subtracted from
 * 'ipconfigTCP_MSS'.
 *
 * The default is derived from MTU - ( ipconfigNETWORK_MTU + ipSIZE_OF_TCP_HEADER )
 * Where ipconfigNETWORK_MTU + ipSIZE_OF_TCP_HEADER is 40 bytes.
 */

#ifndef ipconfigTCP_MSS
    #define ipconfigTCP_MSS    ( ipconfigNETWORK_MTU - 40U )
#endif

#if ( ipconfigTCP_MSS < 536 )
    #error ipconfigTCP_MSS must be at least 536 ( tcpMINIMUM_SEGMENT_LENGTH )
#endif

#if ( ipconfigTCP_MSS > SIZE_MAX )
    #error ipconfigTCP_MSS overflows a size_t
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigTCP_RX_BUFFER_LENGTH
 *
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html#ipconfigTCP_RX_BUFFER_LENGTH
 *
 * Type: size_t
 * Unit: size of StreamBuffer_t in bytes
 * Minimum: 0
 *
 * Each TCP socket has a circular stream buffer for reception and a separate
 * buffer for transmission.
 *
 * FreeRTOS_setsockopt() can be used with the FREERTOS_SO_RCVBUF and
 * FREERTOS_SO_SNDBUF parameters to set the receive and send buffer sizes
 * respectively - but this must be done between the time that the socket is
 * created and the buffers used by the socket are created. The receive
 * buffer is not created until data is actually received, and the transmit
 * buffer is not created until data is actually sent to the socket for
 * transmission. Once the buffers have been created their sizes cannot be
 * changed.
 *
 * If a listening socket creates a new socket in response to an incoming
 * connect request then the new socket will inherit the buffers sizes of
 * the listening socket.
 *
 * The buffer length defaults to 5840 bytes.
 */

#ifndef ipconfigTCP_RX_BUFFER_LENGTH
    #define ipconfigTCP_RX_BUFFER_LENGTH    ( 4 * ipconfigTCP_MSS )
#endif

#if ( ipconfigTCP_RX_BUFFER_LENGTH < 0 )
    #error ipconfigTCP_RX_BUFFER_LENGTH must be at least 0
#endif

#if ( ipconfigTCP_RX_BUFFER_LENGTH > SIZE_MAX )
    #error ipconfigTCP_RX_BUFFER_LENGTH overflows a size_t
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigTCP_TX_BUFFER_LENGTH
 *
 * Type: size_t
 * Unit: size of StreamBuffer_t in bytes
 * Minimum: 0
 *
 * Define the size of Tx stream buffer for TCP sockets.
 * See ipconfigTCP_RX_BUFFER_LENGTH
 */

#ifndef ipconfigTCP_TX_BUFFER_LENGTH
    #define ipconfigTCP_TX_BUFFER_LENGTH    ( 4 * ipconfigTCP_MSS )
#endif

#if ( ipconfigTCP_TX_BUFFER_LENGTH < 0 )
    #error ipconfigTCP_TX_BUFFER_LENGTH must be at least 0
#endif

#if ( ipconfigTCP_TX_BUFFER_LENGTH > SIZE_MAX )
    #error ipconfigTCP_TX_BUFFER_LENGTH overflows a size_t
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigTCP_TIME_TO_LIVE
 *
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html#ipconfigTCP_TIME_TO_LIVE
 *
 * Type: uint8_t
 * Unit: 'hops'
 * Minimum: 0
 *
 * Defines the Time To Live (TTL) values used in outgoing TCP packets.
 */

#ifndef ipconfigTCP_TIME_TO_LIVE
    #define ipconfigTCP_TIME_TO_LIVE    ( 128 )
#endif

#if ( ipconfigTCP_TIME_TO_LIVE < 0 )
    #error ipconfigTCP_TIME_TO_LIVE must be at least 0
#endif

#if ( ipconfigTCP_TIME_TO_LIVE > UINT8_MAX )
    #error ipconfigTCP_TIME_TO_LIVE overflows a uint8_t
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigUSE_TCP_WIN
 *
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html#ipconfigUSE_TCP_WIN
 *
 * Type: BaseType_t ( ipconfigENABLE | ipconfigDISABLE )
 *
 * Sliding Windows allows messages to arrive out-of-order.
 *
 * Set ipconfigUSE_TCP_WIN to 1 to include sliding window behaviour in TCP
 * sockets. Set ipconfigUSE_TCP_WIN to 0 to exclude sliding window
 * behaviour in TCP sockets.
 *
 * Sliding windows can increase throughput while minimizing network traffic
 * at the expense of consuming more RAM.
 *
 * The size of the sliding window can be changed from its default using the
 * FREERTOS_SO_WIN_PROPERTIES parameter to FreeRTOS_setsockopt(). The
 * sliding window size is specified in units of MSS (so if the MSS is set
 * to 200 bytes then a sliding window size of 2 is equal to 400 bytes) and
 * must always be smaller than or equal to the size of the internal buffers
 * in both directions.
 *
 * If a listening socket creates a new socket in response to an incoming
 * connect request then the new socket will inherit the sliding window
 * sizes of the listening socket.
 */

#ifndef ipconfigUSE_TCP_WIN
    #define ipconfigUSE_TCP_WIN    ipconfigENABLE
#endif

#if ( ( ipconfigUSE_TCP_WIN != ipconfigDISABLE ) && ( ipconfigUSE_TCP_WIN != ipconfigENABLE ) )
    #error Invalid ipconfigUSE_TCP_WIN configuration
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigTCP_SRTT_MINIMUM_VALUE_MS
 *
 * Type: int32_t
 * Unit: milliseconds
 * Minimum: 0
 *
 * When measuring the Smoothed Round Trip Time (SRTT),
 * the result will be rounded up to a minimum value.
 * The default has always been 50, but a value of 1000
 * is recommended ( see RFC6298 ) because hosts often delay the
 * sending of ACK packets with 200 ms.
 */

#ifndef ipconfigTCP_SRTT_MINIMUM_VALUE_MS
    #define ipconfigTCP_SRTT_MINIMUM_VALUE_MS    ( 50 )
#endif

#if ( ipconfigTCP_SRTT_MINIMUM_VALUE_MS < 0 )
    #error ipconfigTCP_SRTT_MINIMUM_VALUE_MS must be at least 0
#endif

#if ( ipconfigTCP_SRTT_MINIMUM_VALUE_MS > UINT32_MAX )
    #error ipconfigTCP_SRTT_MINIMUM_VALUE_MS overflows a uint32_t
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigTCP_WIN_SEG_COUNT
 *
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html#ipconfigTCP_WIN_SEG_COUNT
 *
 * Type: size_t
 * Unit: count of sliding windows
 * Minimum: 1
 *
 * If ipconfigUSE_TCP_WIN is set to 1 then each socket will use a
 * sliding window. Sliding windows allow messages to arrive out-of
 * order, and FreeRTOS-Plus-TCP uses window descriptors to track
 * information about the packets in a window.
 *
 * A pool of descriptors is allocated when the first TCP connection is
 * made. The descriptors are shared between all the sockets.
 * ipconfigTCP_WIN_SEG_COUNT sets the number of descriptors in the
 * pool, and each descriptor is approximately 64 bytes.
 *
 * As an example: If a system will have at most 16 simultaneous TCP
 * connections, and each connection will have an Rx and Tx window of at
 * most 8 segments, then the worst case maximum number of descriptors
 * that will be required is 256 ( 16 * 2 * 8 ). However, the practical
 * worst case is normally much lower than this as most packets will
 * arrive in order.
 */

#ifndef ipconfigTCP_WIN_SEG_COUNT
    #define ipconfigTCP_WIN_SEG_COUNT    ( 256 )
#endif

#if ( ipconfigTCP_WIN_SEG_COUNT < 1 )
    #error ipconfigTCP_WIN_SEG_COUNT must be at least 1
#endif

#if ( ipconfigTCP_WIN_SEG_COUNT > SIZE_MAX )
    #error ipconfigTCP_WIN_SEG_COUNT overflows a size_t
#endif

/*---------------------------------------------------------------------------*/

/*
 * pvPortMallocLarge / vPortFreeLarge
 *
 * Malloc functions specific to large TCP buffers for Rx/Tx.
 */

#ifndef pvPortMallocLarge
    #define pvPortMallocLarge( size )    pvPortMalloc( size )
#endif

#ifndef vPortFreeLarge
    #define vPortFreeLarge( ptr )    vPortFree( ptr )
#endif

/*---------------------------------------------------------------------------*/

/*===========================================================================*/
/*                                TCP CONFIG                                 */
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/*                                UDP CONFIG                                 */
/*===========================================================================*/

/*---------------------------------------------------------------------------*/

/*
 * ipconfigUDP_MAX_RX_PACKETS
 *
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html#ipconfigUDP_MAX_RX_PACKETS
 *
 * Type: UBaseType_t
 * Unit: rx packets
 * Minimum: 0
 *
 * ipconfigUDP_MAX_RX_PACKETS defines the maximum number of packets that can
 * exist in the Rx queue of a UDP socket. For example, if
 * ipconfigUDP_MAX_RX_PACKETS is set to 5 and there are already 5 packets
 * queued on the UDP socket then subsequent packets received on that socket
 * will be dropped until the queue length is less than 5 again.
 * Can be overridden with the socket option 'FREERTOS_SO_UDP_MAX_RX_PACKETS'.
 */

#ifndef ipconfigUDP_MAX_RX_PACKETS
    #define ipconfigUDP_MAX_RX_PACKETS    ( 0 )
#endif

#if ( ipconfigUDP_MAX_RX_PACKETS < 0 )
    #error ipconfigUDP_MAX_RX_PACKETS must be at least 0
#endif

#if ( ipconfigUDP_MAX_RX_PACKETS > UINT_FAST8_MAX )
    #error ipconfigUDP_MAX_RX_PACKETS overflows a UBaseType_t
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigUDP_MAX_SEND_BLOCK_TIME_TICKS
 *
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html#ipconfigUDP_MAX_SEND_BLOCK_TIME_TICKS
 *
 * Type: TickType_t
 * Unit: Ticks
 * Minimum: 0
 * Maximum: portMAX_DELAY
 *
 * Sockets have a send block time attribute. If FreeRTOS_sendto() is called but
 * a network buffer cannot be obtained, then the calling RTOS task is held in
 * the Blocked state (so other tasks can continue to executed) until either a
 * network buffer becomes available or the send block time expires. If the send
 * block time expires then the send operation is aborted.
 *
 * The maximum allowable send block time is capped to the value set by
 * ipconfigUDP_MAX_SEND_BLOCK_TIME_TICKS. Capping the maximum allowable send
 * block time prevents prevents a deadlock occurring when all the network
 * buffers are in use and the tasks that process (and subsequently free) the
 * network buffers are themselves blocked waiting for a network buffer.
 *
 * ipconfigUDP_MAX_SEND_BLOCK_TIME_TICKS is specified in RTOS ticks. A time in
 * milliseconds can be converted to a time in ticks by dividing the time in
 * milliseconds by portTICK_PERIOD_MS.
 */

#ifndef ipconfigUDP_MAX_SEND_BLOCK_TIME_TICKS
    #define ipconfigUDP_MAX_SEND_BLOCK_TIME_TICKS    pdMS_TO_TICKS( 20 )
#endif

STATIC_ASSERT( ipconfigUDP_MAX_SEND_BLOCK_TIME_TICKS <= portMAX_DELAY );

/*---------------------------------------------------------------------------*/

/*
 * ipconfigUDP_PASS_ZERO_CHECKSUM_PACKETS
 *
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html#ipconfigUDP_PASS_ZERO_CHECKSUM_PACKETS
 *
 * Type: BaseType_t ( ipconfigENABLE | ipconfigDISABLE )
 *
 * If enabled then UDP packets that have their checksum value set to 0 will be
 * accepted, which is in compliance with the UDP specification. Otherwise they
 * will be dropped, which deviates from the UDP specification, but is safer.
 */

#ifndef ipconfigUDP_PASS_ZERO_CHECKSUM_PACKETS
    #define ipconfigUDP_PASS_ZERO_CHECKSUM_PACKETS    ipconfigDISABLE
#endif

#if ( ( ipconfigUDP_PASS_ZERO_CHECKSUM_PACKETS != ipconfigDISABLE ) && ( ipconfigUDP_PASS_ZERO_CHECKSUM_PACKETS != ipconfigENABLE ) )
    #error Invalid ipconfigUDP_PASS_ZERO_CHECKSUM_PACKETS configuration
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigUDP_TIME_TO_LIVE
 *
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html#ipconfigUDP_TIME_TO_LIVE
 *
 * Type: uint8_t
 * Units: 'hops'
 * Minimum: 0
 *
 * Defines the Time To Live (TTL) values used in outgoing UDP packets.
 */

#ifndef ipconfigUDP_TIME_TO_LIVE
    #define ipconfigUDP_TIME_TO_LIVE    ( 128 )
#endif

#if ( ipconfigUDP_TIME_TO_LIVE < 0 )
    #error ipconfigUDP_TIME_TO_LIVE must be at least 0
#endif

#if ( ipconfigUDP_TIME_TO_LIVE > UINT8_MAX )
    #error ipconfigUDP_TIME_TO_LIVE overflows a uint8_t
#endif

/*---------------------------------------------------------------------------*/

/*===========================================================================*/
/*                                UDP CONFIG                                 */
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/*                              SOCKET CONFIG                                */
/*===========================================================================*/

/*---------------------------------------------------------------------------*/

/*
 * ipconfigALLOW_SOCKET_SEND_WITHOUT_BIND
 *
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html#ipconfigALLOW_SOCKET_SEND_WITHOUT_BIND
 *
 * Type: BaseType_t ( ipconfigENABLE | ipconfigDISABLE )
 *
 * The address of a socket is the combination of its IP address and its port
 * number. FreeRTOS_bind() is used to manually allocate a port number to a
 * socket (to 'bind' the socket to a port), but manual binding is not normally
 * necessary for client sockets (those sockets that initiate outgoing
 * connections rather than wait for incoming connections on a known port
 * number). If ipconfigALLOW_SOCKET_SEND_WITHOUT_BIND is set to 1 then calling
 * FreeRTOS_sendto() on a socket that has not yet been bound will result in the
 * IP stack automatically binding the socket to a port number from the range
 * socketAUTO_PORT_ALLOCATION_START_NUMBER to 0xffff. If
 * ipconfigALLOW_SOCKET_SEND_WITHOUT_BIND is set to 0 then calling
 * FreeRTOS_sendto() on a socket that has not yet been bound will result
 * in the send operation being aborted.
 */

#ifndef ipconfigALLOW_SOCKET_SEND_WITHOUT_BIND
    #define ipconfigALLOW_SOCKET_SEND_WITHOUT_BIND    ipconfigENABLE
#endif

#if ( ( ipconfigALLOW_SOCKET_SEND_WITHOUT_BIND != ipconfigDISABLE ) && ( ipconfigALLOW_SOCKET_SEND_WITHOUT_BIND != ipconfigENABLE ) )
    #error Invalid ipconfigALLOW_SOCKET_SEND_WITHOUT_BIND configuration
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigSUPPORT_SELECT_FUNCTION
 *
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html#ipconfigSUPPORT_SELECT_FUNCTION
 *
 * Type: BaseType_t ( ipconfigENABLE | ipconfigDISABLE )
 *
 * Include support for FreeRTOS_select() and associated API functions.
 */

#ifndef ipconfigSUPPORT_SELECT_FUNCTION
    #define ipconfigSUPPORT_SELECT_FUNCTION    ipconfigDISABLE
#endif

#if ( ( ipconfigSUPPORT_SELECT_FUNCTION != ipconfigDISABLE ) && ( ipconfigSUPPORT_SELECT_FUNCTION != ipconfigENABLE ) )
    #error Invalid ipconfigSUPPORT_SELECT_FUNCTION configuration
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigSELECT_USES_NOTIFY
 *
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html#ipconfigSELECT_USES_NOTIFY
 *
 * Type: BaseType_t ( ipconfigENABLE | ipconfigDISABLE )
 *
 * This option is only used in case the socket-select functions are
 * activated (when ipconfigSUPPORT_SELECT_FUNCTION is non-zero). When
 * calling select() for a given socket from the same task, this macro is
 * not required. Only when there are multiple tasks using select on the
 * same sockets, this option may prevent a dead-lock. The problem is that
 * the event bit eSELECT_CALL_IP is waited for and cleared by multiple
 * tasks.
 */

#ifndef ipconfigSELECT_USES_NOTIFY
    #define ipconfigSELECT_USES_NOTIFY    ipconfigDISABLE
#endif

#if ( ( ipconfigSELECT_USES_NOTIFY != ipconfigDISABLE ) && ( ipconfigSELECT_USES_NOTIFY != ipconfigENABLE ) )
    #error Invalid ipconfigSELECT_USES_NOTIFY configuration
#endif

#if ipconfigIS_ENABLED( ipconfigSELECT_USES_NOTIFY )
    #if ( configUSE_TASK_NOTIFICATIONS == 0 )
        #error configUSE_TASK_NOTIFICATIONS must be 1 if ipconfigSELECT_USES_NOTIFY is enabled
    #endif
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigSOCK_DEFAULT_RECEIVE_BLOCK_TIME
 *
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html#ipconfigSOCK_DEFAULT_RECEIVE_BLOCK_TIME
 *
 * Type: TickType_t
 * Unit: Ticks
 * Minimum: 0
 * Maximum: portMAX_DELAY
 *
 * API functions used to read data from a socket can block to wait for data to
 * become available. ipconfigSOCK_DEFAULT_RECEIVE_BLOCK_TIME sets the default
 * block time defined in RTOS ticks. If ipconfigSOCK_DEFAULT_RECEIVE_BLOCK_TIME
 * is not defined then the default block time will be set to portMAX_DELAY -
 * meaning an RTOS task that is blocked on a socket read will not leave the
 * Blocked state until data is available. Note that tasks in the Blocked state
 * do not consume any CPU time.
 *
 * ipconfigSOCK_DEFAULT_RECEIVE_BLOCK_TIME is specified in ticks. The macros
 * pdMS_TO_TICKS() and portTICK_PERIOD_MS can both be used to convert a time
 * specified in milliseconds to a time specified in ticks, e.g. 2000 ms:
 *
 * #define ipconfigSOCK_DEFAULT_RECEIVE_BLOCK_TIME    pdMS_TO_TICKS( 2000U )
 *
 * The timeout time can be changed at any time using the FREERTOS_SO_RCVTIMEO
 * parameter with FreeRTOS_setsockopt(). Note: Infinite block times should be
 * used with extreme care in order to avoid a situation where all tasks are
 * blocked indefinitely to wait for another RTOS task (which is also blocked
 * indefinitely) to free a network buffer.
 *
 * A socket can be set to non-blocking mode by setting both the send and
 * receive block time to 0. This might be desirable when an RTOS task is using
 * more than one socket - in which case blocking can instead by performed on
 * all the sockets at once using FreeRTOS_select(), or the RTOS task can
 * set ipconfigSOCKET_HAS_USER_SEMAPHORE to one, then block on its own
 * semaphore.
 */

#ifndef ipconfigSOCK_DEFAULT_RECEIVE_BLOCK_TIME
    #define ipconfigSOCK_DEFAULT_RECEIVE_BLOCK_TIME    portMAX_DELAY
#endif

STATIC_ASSERT( ipconfigSOCK_DEFAULT_RECEIVE_BLOCK_TIME <= portMAX_DELAY );

/*---------------------------------------------------------------------------*/

/*
 * ipconfigSOCK_DEFAULT_SEND_BLOCK_TIME
 *
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html#ipconfigSOCK_DEFAULT_SEND_BLOCK_TIME
 *
 * Type: TickType_t
 * Unit: Ticks
 * Minimum: 0
 * Maximum: portMAX_DELAY
 *
 * When writing to a socket, the write may not be able to proceed immediately.
 * For example, depending on the configuration, a write might have to wait for
 * a network buffer to become available.
 * It determines the number of clock ticks that FreeRTOS_send() must wait
 * for space in the transmission buffer. For FreeRTOS_sendto(), it limits
 * how long the application should wait for a network buffer to become available.
 *
 * See ipconfigSOCK_DEFAULT_RECEIVE_BLOCK_TIME
 */

#ifndef ipconfigSOCK_DEFAULT_SEND_BLOCK_TIME
    #define ipconfigSOCK_DEFAULT_SEND_BLOCK_TIME    portMAX_DELAY
#endif

STATIC_ASSERT( ipconfigSOCK_DEFAULT_SEND_BLOCK_TIME <= portMAX_DELAY );

/*---------------------------------------------------------------------------*/

/*
 * ipconfigSOCKET_HAS_USER_SEMAPHORE
 *
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html#ipconfigSOCKET_HAS_USER_SEMAPHORE
 *
 * Type: BaseType_t ( ipconfigENABLE | ipconfigDISABLE )
 *
 * Allows a semaphore to be provided that will be given after important events
 * to wake up the user.
 *
 * Set by calling FreeRTOS_setsockopt() with the FREERTOS_SO_SET_SEMAPHORE
 * option and a pointer to a semaphore. Once set, the semaphore will be signalled
 * after every important socket event: READ, WRITE, EXCEPTION.
 * Note that a READ event is also generated for a TCP socket in listen mode,
 * and a WRITE event is generated when a call to connect() has succeeded.
 * Beside that, READ and WRITE are the normal events that occur when
 * data has been received or delivered.
 *
 * Can be used with non-blocking sockets as an alternative to
 * FreeRTOS_select in order to handle multiple sockets at once.
 */

#ifndef ipconfigSOCKET_HAS_USER_SEMAPHORE
    #define ipconfigSOCKET_HAS_USER_SEMAPHORE    ipconfigDISABLE
#endif

#if ( ( ipconfigSOCKET_HAS_USER_SEMAPHORE != ipconfigDISABLE ) && ( ipconfigSOCKET_HAS_USER_SEMAPHORE != ipconfigENABLE ) )
    #error Invalid ipconfigSOCKET_HAS_USER_SEMAPHORE configuration
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigSOCKET_HAS_USER_WAKE_CALLBACK
 *
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html#ipconfigSOCKET_HAS_USER_WAKE_CALLBACK
 *
 * Type: BaseType_t ( ipconfigENABLE | ipconfigDISABLE )
 *
 * Enables usage of an application defined hook that will be called after
 * one of the following events:
 *
 * eSOCKET_RECEIVE = 0x0001, * Reception of new data. *
 * eSOCKET_SEND    = 0x0002, * Some data has been sent. *
 * eSOCKET_ACCEPT  = 0x0004, * A new TCP client was detected, please call accept(). *
 * eSOCKET_CONNECT = 0x0008, * A TCP connect has succeeded or timed-out. *
 * eSOCKET_BOUND   = 0x0010, * A socket got bound. *
 * eSOCKET_CLOSED  = 0x0020, * A TCP connection got closed. *
 * eSOCKET_INTR    = 0x0040, * A blocking API call got interrupted, because
 *                           * the function FreeRTOS_SignalSocket() was called. *
 *
 * It is not a good idea to do a lot of processing in any of the application
 * hooks. Normally the hook will only notify the task that owns the socket so
 * that the socket gets immediate attention.
 *
 * Function prototype:
 *
 * typedef void (* SocketWakeupCallback_t)( Socket_t pxSocket )
 */

#ifndef ipconfigSOCKET_HAS_USER_WAKE_CALLBACK
    #define ipconfigSOCKET_HAS_USER_WAKE_CALLBACK    ipconfigDISABLE
#endif

#if ( ( ipconfigSOCKET_HAS_USER_WAKE_CALLBACK != ipconfigDISABLE ) && ( ipconfigSOCKET_HAS_USER_WAKE_CALLBACK != ipconfigENABLE ) )
    #error Invalid ipconfigSOCKET_HAS_USER_WAKE_CALLBACK configuration
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigSUPPORT_SIGNALS
 *
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html#ipconfigSUPPORT_SIGNALS
 *
 * Type: BaseType_t ( ipconfigENABLE | ipconfigDISABLE )
 *
 * Advanced users only.
 *
 * Include support for FreeRTOS_SignalSocket().
 *
 * Used to interrupt a socket during a blocked read allowing the task proceed.
 * A signal interruption can be distinguished from a read through the return
 * value -pdFREERTOS_ERRNO_EINTR.
 */

#ifndef ipconfigSUPPORT_SIGNALS
    #define ipconfigSUPPORT_SIGNALS    ipconfigDISABLE
#endif

#if ( ( ipconfigSUPPORT_SIGNALS != ipconfigDISABLE ) && ( ipconfigSUPPORT_SIGNALS != ipconfigENABLE ) )
    #error Invalid ipconfigSUPPORT_SIGNALS configuration
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigUSE_CALLBACKS
 *
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html#ipconfigUSE_CALLBACKS
 *
 * Type: BaseType_t ( ipconfigENABLE | ipconfigDISABLE )
 *
 * Advanced users only.
 *
 * Allows usage of application defined hooks for socket events.
 *
 * Can be set by calling FreeRTOS_setsockopt() with any of the following
 * options and a pointer to a F_TCP_UDP_Handler_t.
 *
 * FREERTOS_SO_TCP_CONN_HANDLER  * Callback for (dis)connection events. *
 * FREERTOS_SO_TCP_RECV_HANDLER  * Callback for receiving TCP data. *
 * FREERTOS_SO_TCP_SENT_HANDLER  * Callback for sending TCP data. *
 * FREERTOS_SO_UDP_RECV_HANDLER  * Callback for receiving UDP data. *
 * FREERTOS_SO_UDP_SENT_HANDLER  * Callback for sending UDP data. *
 *
 * typedef struct xTCP_UDP_HANDLER
 * {
 *      FOnConnected_t pxOnTCPConnected; * FREERTOS_SO_TCP_CONN_HANDLER *
 *      FOnTCPReceive_t pxOnTCPReceive;  * FREERTOS_SO_TCP_RECV_HANDLER *
 *      FOnTCPSent_t pxOnTCPSent;        * FREERTOS_SO_TCP_SENT_HANDLER *
 *      FOnUDPReceive_t pxOnUDPReceive;  * FREERTOS_SO_UDP_RECV_HANDLER *
 *      FOnUDPSent_t pxOnUDPSent;        * FREERTOS_SO_UDP_SENT_HANDLER *
 * } F_TCP_UDP_Handler_t
 *
 * Function Prototypes:
 *
 * typedef void (* FOnConnected_t )( Socket_t xSocket,
 *                                   BaseType_t ulConnected )
 *
 * typedef BaseType_t (* FOnTCPReceive_t )( Socket_t xSocket,
 *                                          void * pData,
 *                                          size_t xLength )
 *
 * typedef void (* FOnTCPSent_t )( Socket_t xSocket,
 *                                 size_t xLength )
 *
 * typedef BaseType_t (* FOnUDPReceive_t ) ( Socket_t xSocket,
 *                                           void * pData,
 *                                           size_t xLength,
 *                                           const struct freertos_sockaddr * pxFrom,
 *                                           const struct freertos_sockaddr * pxDest )
 *
 * typedef void (* FOnUDPSent_t )( Socket_t xSocket,
 *                                      size_t xLength );
 */

#ifndef ipconfigUSE_CALLBACKS
    #define ipconfigUSE_CALLBACKS    ipconfigDISABLE
#endif

#if ( ( ipconfigUSE_CALLBACKS != ipconfigDISABLE ) && ( ipconfigUSE_CALLBACKS != ipconfigENABLE ) )
    #error Invalid ipconfigUSE_CALLBACKS configuration
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigIS_VALID_PROG_ADDRESS
 *
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html#ipconfigIS_VALID_PROG_ADDRESS
 *
 * Type: Macro Function
 * Value: BaseType_t ( pdTRUE | pdFALSE )
 *
 * Verifies that a given address refers to valid (instruction) memory.
 *
 * Used to check if application defined hooks are valid.
 *
 * Example:
 * if( ipconfigIS_VALID_PROG_ADDRESS( pxSocket->u.xTCP.pxHandleSent ) )
 * {
 *     pxSocket->u.xTCP.pxHandleSent( pxSocket, ulCount );
 * }
 */

#ifndef ipconfigIS_VALID_PROG_ADDRESS
    #define ipconfigIS_VALID_PROG_ADDRESS( pxAddress )    ( ( pxAddress ) != NULL )
#endif

/*---------------------------------------------------------------------------*/

/*
 * pvPortMallocSocket/vPortFreeSocket
 *
 * Malloc functions specific to sockets.
 */

#ifndef pvPortMallocSocket
    #define pvPortMallocSocket( size )    pvPortMalloc( size )
#endif

#ifndef vPortFreeSocket
    #define vPortFreeSocket( ptr )    vPortFree( ptr )
#endif

/*---------------------------------------------------------------------------*/

/*===========================================================================*/
/*                              SOCKET CONFIG                                */
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/*                               DHCP CONFIG                                 */
/*===========================================================================*/

/*---------------------------------------------------------------------------*/

/*
 * ipconfigUSE_DHCP
 *
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html#ipconfigUSE_DHCP
 *
 * Type: BaseType_t ( ipconfigENABLE | ipconfigDISABLE )
 *
 * Allows DHCP to be enabled by setting by setting `endpoint->bits.bWantDHCP`.
 *
 * When successful, DHCP will assign an IP-address, a netmask, a gateway
 * address, and one or more DNS addresses to the endpoint. DHCP must be able to
 * receive an options field of 312 bytes, the fixed part of the DHCP packet is
 * 240 bytes, and the IP/UDP headers take 28 bytes.
 */

#ifndef ipconfigUSE_DHCP
    #define ipconfigUSE_DHCP    ipconfigENABLE
#endif

#if ( ( ipconfigUSE_DHCP != ipconfigDISABLE ) && ( ipconfigUSE_DHCP != ipconfigENABLE ) )
    #error Invalid ipconfigUSE_DHCP configuration
#endif

#if ( ( ipconfigUSE_DHCP != ipconfigDISABLE ) && ( ipconfigNETWORK_MTU < 586 ) )
    #error ipconfigNETWORK_MTU needs to be at least 586 to use DHCP
#endif

#if ( ipconfigIS_ENABLED( ipconfigUSE_DHCP ) && ipconfigIS_DISABLED( ipconfigUSE_IPv4 ) )
    #error DHCP Cannot be enabled without IPv4
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigUSE_DHCPv6
 *
 * Type: BaseType_t ( ipconfigENABLE | ipconfigDISABLE )
 *
 * Include support for DHCPv6.
 *
 * The use of DHCP can be enabled per endpoint by setting
 * `endpoint->bits.bWantDHCP`.
 *
 * An alternative way of obtaining an IP-address is Router Advertisement ("RA").
 * As RA is generally preferred above DHCP, ipconfigUSE_RA is enabled and
 * ipconfigUSE_DHCPv6 is disabled by default.
 */

#ifndef ipconfigUSE_DHCPv6
    #define ipconfigUSE_DHCPv6    ipconfigDISABLE
#endif

#if ( ( ipconfigUSE_DHCPv6 != ipconfigDISABLE ) && ( ipconfigUSE_DHCPv6 != ipconfigENABLE ) )
    #error Invalid ipconfigUSE_DHCPv6 configuration
#endif

#if ( ipconfigIS_ENABLED( ipconfigUSE_DHCPv6 ) && ipconfigIS_DISABLED( ipconfigUSE_IPv6 ) )
    #error DHCPv6 Cannot be enabled without IPv6
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigDHCP_REGISTER_HOSTNAME
 *
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html#ipconfigDHCP_REGISTER_HOSTNAME
 *
 * Type: BaseType_t ( ipconfigENABLE | ipconfigDISABLE )
 *
 * Enables usage of an application defined hook to provide a hostname to a DHCP
 * server.
 *
 * Function prototype:
 *
 * const char *pcApplicationHostnameHook( void )
 */

#ifndef ipconfigDHCP_REGISTER_HOSTNAME
    #define ipconfigDHCP_REGISTER_HOSTNAME    ipconfigDISABLE
#endif

#if ( ( ipconfigDHCP_REGISTER_HOSTNAME != ipconfigDISABLE ) && ( ipconfigDHCP_REGISTER_HOSTNAME != ipconfigENABLE ) )
    #error Invalid ipconfigDHCP_REGISTER_HOSTNAME configuration
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigUSE_DHCP_HOOK
 *
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html#ipconfigUSE_DHCP_HOOK
 *
 * Type: BaseType_t ( ipconfigENABLE | ipconfigDISABLE )
 *
 * Enables usage of an application defined hook during the DHCP process before
 * the initial discovery packet is sent, and after a DHCP offer has been
 * received.
 *
 * Function Prototype:
 *
 * Function prototype if ipconfigIPv4_BACKWARD_COMPATIBLE is enabled:
 *
 * eDHCPCallbackAnswer_t xApplicationDHCPHook( eDHCPCallbackPhase_t eDHCPPhase,
 *                                             uint32_t ulIPAddress )
 *
 * Function prototype by default:
 *
 * eDHCPCallbackAnswer_t xApplicationDHCPHook_Multi( eDHCPCallbackPhase_t eDHCPPhase,
 *                                                   struct xNetworkEndPoint * pxEndPoint,
 *                                                   IP_Address_t * pxIPAddress );
 */

#ifndef ipconfigUSE_DHCP_HOOK
    #define ipconfigUSE_DHCP_HOOK    ipconfigENABLE
#endif

#if ( ( ipconfigUSE_DHCP_HOOK != ipconfigDISABLE ) && ( ipconfigUSE_DHCP_HOOK != ipconfigENABLE ) )
    #error Invalid ipconfigUSE_DHCP_HOOK configuration
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigDHCP_FALL_BACK_AUTO_IP
 *
 * Type: BaseType_t ( ipconfigENABLE | ipconfigDISABLE )
 *
 * If no DHCP server responds, allocate a random LinkLayer IP address.
 *
 * Requires ipconfigARP_USE_CLASH_DETECTION to be enabled as well in order to
 * test if it is still available.
 *
 * An alternative is to resort to a static IP address.
 */

#ifndef ipconfigDHCP_FALL_BACK_AUTO_IP
    #define ipconfigDHCP_FALL_BACK_AUTO_IP    ipconfigDISABLE
#endif

#if ( ( ipconfigDHCP_FALL_BACK_AUTO_IP != ipconfigDISABLE ) && ( ipconfigDHCP_FALL_BACK_AUTO_IP != ipconfigENABLE ) )
    #error Invalid ipconfigDHCP_FALL_BACK_AUTO_IP configuration
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigMAXIMUM_DISCOVER_TX_PERIOD
 *
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html#ipconfigMAXIMUM_DISCOVER_TX_PERIOD
 *
 * Type: TickType_t
 * Unit: Ticks
 * Minimum: 0
 * Maximum: portMAX_DELAY
 *
 * Sets the max interval allowed between transmissions of DHCP requests before
 * the default IP address will be used.
 *
 * When 'endpoint->bits.bWantDHCP' is set, DHCP requests will be sent out at
 * increasing time intervals until either a reply is received from a DHCP
 * server and accepted, or the interval between transmissions reaches
 * ipconfigMAXIMUM_DISCOVER_TX_PERIOD. If no reply is received, the TCP/IP
 * stack will revert to using the default IP address of the endpoint
 * 'endpoint->ipv4_defaults.ulIPAddress' for IPv4 address or
 * 'endpoint->ipv6_defaults.xIPAddress' for IPv6 address.
 */

#ifndef ipconfigMAXIMUM_DISCOVER_TX_PERIOD
    #ifdef _WINDOWS_
        #define ipconfigMAXIMUM_DISCOVER_TX_PERIOD    pdMS_TO_TICKS( 999 )
    #else
        #define ipconfigMAXIMUM_DISCOVER_TX_PERIOD    pdMS_TO_TICKS( 30000 )
    #endif
#endif

STATIC_ASSERT( ipconfigMAXIMUM_DISCOVER_TX_PERIOD <= portMAX_DELAY );

/*---------------------------------------------------------------------------*/

/*===========================================================================*/
/*                               DHCP CONFIG                                 */
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/*                                DNS CONFIG                                 */
/*===========================================================================*/

/*---------------------------------------------------------------------------*/

/*
 * ipconfigUSE_DNS
 *
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html#ipconfigUSE_DNS
 *
 * Type: BaseType_t ( ipconfigENABLE | ipconfigDISABLE )
 *
 * Enable ipconfigUSE_DNS to include a basic DNS client/resolver. DNS is used
 * through functions like FreeRTOS_getaddrinfo() and FreeRTOS_gethostbyname().
 *
 * Allows name discovery protocols like mDNS, LLMNR and NBNS to be enabled as
 * well.
 *
 * See: ipconfigUSE_MDNS, ipconfigUSE_LLMNR, ipconfigUSE_NBNS
 */

#ifndef ipconfigUSE_DNS
    #define ipconfigUSE_DNS    ipconfigENABLE
#endif

#if ( ( ipconfigUSE_DNS != ipconfigDISABLE ) && ( ipconfigUSE_DNS != ipconfigENABLE ) )
    #error Invalid ipconfigUSE_DNS configuration
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigUSE_DNS_CACHE
 *
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html#ipconfigUSE_DNS_CACHE
 *
 * Type: BaseType_t ( ipconfigENABLE | ipconfigDISABLE )
 *
 * Enables usage of the DNS cache.
 */

#ifndef ipconfigUSE_DNS_CACHE
    #define ipconfigUSE_DNS_CACHE    ipconfigENABLE
#endif

#if ( ( ipconfigUSE_DNS_CACHE != ipconfigDISABLE ) && ( ipconfigUSE_DNS_CACHE != ipconfigENABLE ) )
    #error Invalid ipconfigUSE_DNS_CACHE configuration
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigDNS_CACHE_ENTRIES
 *
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html#ipconfigDNS_CACHE_ENTRIES
 *
 * Type: size_t
 * Unit: count of DNS cache entries
 * Minimum: 1
 *
 * Defines the number of entries in the DNS cache.
 */

#ifndef ipconfigDNS_CACHE_ENTRIES
    #define ipconfigDNS_CACHE_ENTRIES    ( 1U )
#endif

#if ( ipconfigDNS_CACHE_ENTRIES < 1 )
    #error ipconfigDNS_CACHE_ENTRIES must be at least 1
#endif

#if ( ipconfigDNS_CACHE_ENTRIES > SIZE_MAX )
    #error ipconfigDNS_CACHE_ENTRIES overflows a size_t
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigDNS_CACHE_NAME_LENGTH
 *
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html#ipconfigDNS_CACHE_NAME_LENGTH
 *
 * Type: size_t
 * Unit: count of hostname chars
 * Minimum: 1
 *
 * The maximum number of characters a DNS host name can take, including
 * the NULL terminator.
 *
 * Stack warning: the function DNS_ParseDNSReply() declares a local object
 * of type 'ParseSet_t', which contains a copy of an URL:
 *
 *     char pcName[ ipconfigDNS_CACHE_NAME_LENGTH ];
 *
 * plus another 52 bytes.
 */

#ifndef ipconfigDNS_CACHE_NAME_LENGTH
    #define ipconfigDNS_CACHE_NAME_LENGTH    ( 254U )
#endif

#if ( ipconfigDNS_CACHE_NAME_LENGTH < 1 )
    #error ipconfigDNS_CACHE_NAME_LENGTH must be at least 1
#endif

#if ( ipconfigDNS_CACHE_NAME_LENGTH > SIZE_MAX )
    #error ipconfigDNS_CACHE_NAME_LENGTH overflows a size_t
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigDNS_CACHE_ADDRESSES_PER_ENTRY
 *
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html#ipconfigDNS_CACHE_ADDRESSES_PER_ENTRY
 *
 * Type: size_t
 * Unit: count of IP addresses
 * Minimum: 1
 *
 * Sets how many IP addresses can be stored when looking up a URL.
 */

#ifndef ipconfigDNS_CACHE_ADDRESSES_PER_ENTRY
    #define ipconfigDNS_CACHE_ADDRESSES_PER_ENTRY    ( 1 )
#endif

#if ( ipconfigDNS_CACHE_ADDRESSES_PER_ENTRY < 1 )
    #error ipconfigDNS_CACHE_ADDRESSES_PER_ENTRY must be at least 1
#endif

#if ( ipconfigDNS_CACHE_ADDRESSES_PER_ENTRY > SIZE_MAX )
    #error ipconfigDNS_CACHE_ADDRESSES_PER_ENTRY overflows a size_t
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigDNS_REQUEST_ATTEMPTS
 *
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html#ipconfigDNS_REQUEST_ATTEMPTS
 *
 * Type: size_t
 * Unit: count of request attempts
 * Minimum: 1
 *
 * Sets the most amount of times the library can send a DNS result and wait for
 * a result when looking up a host.
 *
 * See ipconfigDNS_RECEIVE_BLOCK_TIME_TICKS & ipconfigDNS_SEND_BLOCK_TIME_TICKS
 */

#ifndef ipconfigDNS_REQUEST_ATTEMPTS
    #define ipconfigDNS_REQUEST_ATTEMPTS    ( 5 )
#endif

#if ( ipconfigDNS_REQUEST_ATTEMPTS < 1 )
    #error ipconfigDNS_REQUEST_ATTEMPTS must be at least 1
#endif

#if ( ipconfigDNS_REQUEST_ATTEMPTS > SIZE_MAX )
    #error ipconfigDNS_REQUEST_ATTEMPTS overflows a size_t
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigDNS_RECEIVE_BLOCK_TIME_TICKS
 *
 * Type: TickType_t
 * Unit: Ticks
 * Minimum: 0
 * Maximum: portMAX_DELAY
 *
 * When looking up a host with DNS, this macro determines how long the
 * call to FreeRTOS_sendto() will wait for a reply. When there is no
 * reply, the request will be repeated up to 'ipconfigDNS_REQUEST_ATTEMPTS'
 * attempts.
 * See ipconfigSOCK_DEFAULT_RECEIVE_BLOCK_TIME
 *
 * Applies to DNS socket only.
 */

#ifndef ipconfigDNS_RECEIVE_BLOCK_TIME_TICKS
    #define ipconfigDNS_RECEIVE_BLOCK_TIME_TICKS    pdMS_TO_TICKS( 5000 )
#endif

STATIC_ASSERT( ipconfigDNS_RECEIVE_BLOCK_TIME_TICKS <= portMAX_DELAY );

/*---------------------------------------------------------------------------*/

/*
 * ipconfigDNS_SEND_BLOCK_TIME_TICKS
 *
 * Type: TickType_t
 * Unit: Ticks
 * Minimum: 0
 * Maximum: portMAX_DELAY
 *
 * When looking up a host with DNS, this macro determines how long the
 * call to FreeRTOS_sendto() will block to wait for a free buffer.
 * See ipconfigSOCK_DEFAULT_SEND_BLOCK_TIME
 *
 * Applies to DNS socket only.
 */

#ifndef ipconfigDNS_SEND_BLOCK_TIME_TICKS
    #define ipconfigDNS_SEND_BLOCK_TIME_TICKS    pdMS_TO_TICKS( 500 )
#endif

STATIC_ASSERT( ipconfigDNS_SEND_BLOCK_TIME_TICKS <= portMAX_DELAY );

/*---------------------------------------------------------------------------*/

/*
 * ipconfigDNS_USE_CALLBACKS
 *
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html#ipconfigDNS_USE_CALLBACKS
 *
 * Type: BaseType_t ( ipconfigENABLE | ipconfigDISABLE )
 *
 * Enables callback functionality in the DNS process.
 *
 * Application defined callbacks can be provided as parameters in calls
 * to FreeRTOS_gethostbyname_a() & FreeRTOS_getaddrinfo_a(). These functions
 * will start a DNS-lookup and call the callback when either the URL has been
 * found, or when a time-out has been reached. These functions are
 * non-blocking, the suffix "_a" stands for asynchronous. The callback can be
 * canceled by using FreeRTOS_gethostbyname_cancel().
 *
 * Function Prototype:
 *
 * void (* FOnDNSEvent ) ( const char * pcName, void * pvSearchID, struct freertos_addrinfo * pxAddressInfo )
 */

#ifndef ipconfigDNS_USE_CALLBACKS
    #define ipconfigDNS_USE_CALLBACKS    ipconfigDISABLE
#endif

#if ( ( ipconfigDNS_USE_CALLBACKS != ipconfigDISABLE ) && ( ipconfigDNS_USE_CALLBACKS != ipconfigENABLE ) )
    #error Invalid ipconfigDNS_USE_CALLBACKS configuration
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigUSE_LLMNR
 *
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html#ipconfigUSE_LLMNR
 *
 * Type: BaseType_t ( ipconfigENABLE | ipconfigDISABLE )
 *
 * Include support for Link Local Multicast Name Resolution (LLMNR).
 */

#ifndef ipconfigUSE_LLMNR
    #define ipconfigUSE_LLMNR    ipconfigDISABLE
#endif

#if ( ( ipconfigUSE_LLMNR != ipconfigDISABLE ) && ( ipconfigUSE_LLMNR != ipconfigENABLE ) )
    #error Invalid ipconfigUSE_LLMNR configuration
#endif

#if ( ipconfigIS_ENABLED( ipconfigUSE_LLMNR ) && ipconfigIS_DISABLED( ipconfigUSE_DNS ) )
    #error When LLMNR is enabled, ipconfigUSE_DNS must also be enabled
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigUSE_NBNS
 *
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html#ipconfigUSE_NBNS
 *
 * Type: BaseType_t ( ipconfigENABLE | ipconfigDISABLE )
 *
 * Include support for NetBIOS Name Service (NBNS).
 */

#ifndef ipconfigUSE_NBNS
    #define ipconfigUSE_NBNS    ipconfigDISABLE
#endif

#if ( ( ipconfigUSE_NBNS != ipconfigDISABLE ) && ( ipconfigUSE_NBNS != ipconfigENABLE ) )
    #error Invalid ipconfigUSE_NBNS configuration
#endif

#if ( ipconfigIS_ENABLED( ipconfigUSE_NBNS ) && ipconfigIS_DISABLED( ipconfigUSE_DNS ) )
    #error When NBNS is enabled, ipconfigUSE_DNS must also be enabled
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigUSE_MDNS
 *
 * Type: BaseType_t ( ipconfigENABLE | ipconfigDISABLE )
 *
 * Include support for Multicast DNS (MDNS).
 */

#ifndef ipconfigUSE_MDNS
    #define ipconfigUSE_MDNS    ipconfigDISABLE
#endif

#if ( ( ipconfigUSE_MDNS != ipconfigDISABLE ) && ( ipconfigUSE_MDNS != ipconfigENABLE ) )
    #error Invalid ipconfigUSE_MDNS configuration
#endif

#if ( ipconfigIS_ENABLED( ipconfigUSE_MDNS ) && ipconfigIS_DISABLED( ipconfigUSE_DNS ) )
    #error When MDNS is enabled, ipconfigUSE_DNS must also be enabled
#endif

/*---------------------------------------------------------------------------*/

/*===========================================================================*/
/*                                DNS CONFIG                                 */
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/*                                ARP CONFIG                                 */
/*===========================================================================*/

/*---------------------------------------------------------------------------*/

/*
 * ipconfigARP_CACHE_ENTRIES
 *
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html#ipconfigARP_CACHE_ENTRIES
 *
 * Type: size_t
 * Unit: count of arp cache entries
 * Minimum: 1
 *
 * Defines the maximum number of entries that can exist in the ARP table at any
 * one time.
 *
 * The ARP cache is a table that maps IP addresses to MAC addresses. The IP
 * stack can only send a UDP message to a remove IP address if it knowns
 * the MAC address associated with the IP address, or the MAC address of the
 * router used to contact the remote IP address. When a UDP message is received
 * from a remote IP address, the MAC address and IP address are added to the
 * ARP cache. When a UDP message is sent to a remote IP address that does not
 * already appear in the ARP cache, then the UDP message is replaced by a ARP
 * message that solicits the required MAC address information.
 */

#ifndef ipconfigARP_CACHE_ENTRIES
    #define ipconfigARP_CACHE_ENTRIES    ( 10 )
#endif

#if ( ipconfigARP_CACHE_ENTRIES < 1 )
    #error ipconfigARP_CACHE_ENTRIES must be at least 1
#endif

#if ( ipconfigARP_CACHE_ENTRIES > SIZE_MAX )
    #error ipconfigARP_CACHE_ENTRIES overflows a size_t
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigARP_STORES_REMOTE_ADDRESSES
 *
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html#ipconfigARP_STORES_REMOTE_ADDRESSES
 *
 * Type: BaseType_t ( ipconfigENABLE | ipconfigDISABLE )
 *
 * Advanced users only.
 *
 * Normally, the ARP table will only store IP-addresses that are located
 * in the local subnet. This macro enables the storage of remote addresses
 * in the ARP table along with the associated MAC address from which the
 * message was received.
 *
 * Provided for the case when a message that requires a reply arrives from the
 * Internet, but from a computer attached to a LAN rather than via the defined
 * gateway. Before replying to the message, the TCP/IP stack RTOS task will
 * loop up the message's IP address in the ARP table. If disabled then ARP will
 * return the MAC address of the defined gateway because the destination
 * address is outside of the netmask, which might prevent the reply reaching
 * its intended destination. This macro can allow the message in this scenario
 * to be routed and delivered correctly.
 */

#ifndef ipconfigARP_STORES_REMOTE_ADDRESSES
    #define ipconfigARP_STORES_REMOTE_ADDRESSES    ipconfigDISABLE
#endif

#if ( ( ipconfigARP_STORES_REMOTE_ADDRESSES != ipconfigDISABLE ) && ( ipconfigARP_STORES_REMOTE_ADDRESSES != ipconfigENABLE ) )
    #error Invalid ipconfigARP_STORES_REMOTE_ADDRESSES configuration
#endif

/*---------------------------------------------------------------------------*/

#if ( ipconfigIS_ENABLED( ipconfigUSE_DHCP ) && ipconfigIS_ENABLED( ipconfigDHCP_FALL_BACK_AUTO_IP ) )

/*-----------------------------------------------------------------------*/

/*
 * ipconfigARP_USE_CLASH_DETECTION
 *
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html#ipconfigARP_USE_CLASH_DETECTION
 *
 * Type: BaseType_t ( ipconfigENABLE | ipconfigDISABLE )
 *
 * Enables the driver to test if an assigned link-layer address is already
 * taken by another device by sending ARP requests.
 *
 * ipconfigDHCP_FALL_BACK_AUTO_IP requires this feature to be enabled.
 */

    #ifndef ipconfigARP_USE_CLASH_DETECTION
        #define ipconfigARP_USE_CLASH_DETECTION    ipconfigENABLE
    #endif

    #if ( ( ipconfigARP_USE_CLASH_DETECTION != ipconfigDISABLE ) && ( ipconfigARP_USE_CLASH_DETECTION != ipconfigENABLE ) )
        #error Invalid ipconfigARP_USE_CLASH_DETECTION configuration
    #endif

    #if ( ipconfigIS_DISABLED( ipconfigARP_USE_CLASH_DETECTION ) )
        #error When ipconfigDHCP_FALL_BACK_AUTO_IP is enabled, ipconfigARP_USE_CLASH_DETECTION must also be enabled
    #endif

/*-----------------------------------------------------------------------*/

#else /* ( ( ipconfigIS_ENABLED( ipconfigUSE_DHCP ) && ipconfigIS_ENABLED( ipconfigDHCP_FALL_BACK_AUTO_IP ) ) */

/*-----------------------------------------------------------------------*/

    #ifndef ipconfigARP_USE_CLASH_DETECTION
        #define ipconfigARP_USE_CLASH_DETECTION    ipconfigDISABLE
    #endif

    #if ( ipconfigIS_ENABLED( ipconfigARP_USE_CLASH_DETECTION ) )
        #error ipconfigARP_USE_CLASH_DETECTION is unused when ipconfigDHCP_FALL_BACK_AUTO_IP is disabled
    #endif

/*-----------------------------------------------------------------------*/

#endif /* ( ipconfigIS_ENABLED( ipconfigUSE_DHCP ) && ipconfigIS_ENABLED( ipconfigDHCP_FALL_BACK_AUTO_IP ) ) */

/*---------------------------------------------------------------------------*/

/*
 * ipconfigMAX_ARP_AGE
 *
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html#ipconfigMAX_ARP_AGE
 *
 * Type: uint8_t
 * Unit: decaseconds
 * Minimum: 0
 *
 * Defines the maximum time between an entry in the ARP table being created or
 * refreshed and the entry being removed because it is stale. New ARP requests
 * are sent for ARP cache entries that are nearing their maximum age.
 * The maximum age of an entry in the ARP cache table can be
 * calculated as 'ipARP_TIMER_PERIOD_MS' x 'ipconfigMAX_ARP_AGE'.
 *
 * Units are derived from ipARP_TIMER_PERIOD_MS, which is 10000 ms or 10 sec.
 * So, a value of 150 is equal to 1500 seconds.
 */

#ifndef ipconfigMAX_ARP_AGE
    #define ipconfigMAX_ARP_AGE    ( 150 )
#endif

#if ( ipconfigMAX_ARP_AGE < 0 )
    #error ipconfigMAX_ARP_AGE must be at least 0
#endif

#if ( ipconfigMAX_ARP_AGE > UINT8_MAX )
    #error ipconfigMAX_ARP_AGE overflows a uint8_t
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigMAX_ARP_RETRANSMISSIONS
 *
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html#ipconfigMAX_ARP_RETRANSMISSIONS
 *
 * Type: uint8_t
 * Unit: count of retransmissions
 * Minimum: 0
 *
 * Sets the number of times an ARP request is sent when looking up an
 * IP-address. Also referred as the maximum amount of retransmissions
 * of ARP requests that do not result in an ARP response before the ARP
 * request is aborted.
 */

#ifndef ipconfigMAX_ARP_RETRANSMISSIONS
    #define ipconfigMAX_ARP_RETRANSMISSIONS    ( 5 )
#endif

#if ( ipconfigMAX_ARP_RETRANSMISSIONS < 0 )
    #error ipconfigMAX_ARP_RETRANSMISSIONS must be at least 0
#endif

#if ( ipconfigMAX_ARP_RETRANSMISSIONS > UINT8_MAX )
    #error ipconfigMAX_ARP_RETRANSMISSIONS overflows a uint8_t
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigUSE_ARP_REMOVE_ENTRY
 *
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html#ipconfigUSE_ARP_REMOVE_ENTRY
 *
 * Type: BaseType_t ( ipconfigENABLE | ipconfigDISABLE )
 *
 * Advanced users only.
 *
 * Include support for ulARPRemoveCacheEntryByMac() which uses a MAC address to
 * look up and remove an entry from the ARP cache. If the MAC address is found
 * in the ARP cache, then the IP address associated with the MAC address is
 * returned, otherwise 0 is returned.
 */

#ifndef ipconfigUSE_ARP_REMOVE_ENTRY
    #define ipconfigUSE_ARP_REMOVE_ENTRY    ipconfigDISABLE
#endif

#if ( ( ipconfigUSE_ARP_REMOVE_ENTRY != ipconfigDISABLE ) && ( ipconfigUSE_ARP_REMOVE_ENTRY != ipconfigENABLE ) )
    #error Invalid ipconfigUSE_ARP_REMOVE_ENTRY configuration
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigUSE_ARP_REVERSED_LOOKUP
 *
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html#ipconfigUSE_ARP_REVERSED_LOOKUP
 *
 * Type: BaseType_t ( ipconfigENABLE | ipconfigDISABLE )
 *
 * Advanced users only.
 *
 * Include support for eARPGetCacheEntryByMac() which looks up a MAC address
 * from an IP address.
 *
 * ARP normally does the reverse by looking up an IP address from a MAC
 * address.
 */

#ifndef ipconfigUSE_ARP_REVERSED_LOOKUP
    #define ipconfigUSE_ARP_REVERSED_LOOKUP    ipconfigDISABLE
#endif

#if ( ( ipconfigUSE_ARP_REVERSED_LOOKUP != ipconfigDISABLE ) && ( ipconfigUSE_ARP_REVERSED_LOOKUP != ipconfigENABLE ) )
    #error Invalid ipconfigUSE_ARP_REVERSED_LOOKUP configuration
#endif

/*---------------------------------------------------------------------------*/

/*===========================================================================*/
/*                                ARP CONFIG                                 */
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/*                                ND CONFIG                                  */
/*===========================================================================*/

/*---------------------------------------------------------------------------*/

/*
 * ipconfigND_CACHE_ENTRIES
 *
 * Type: size_t
 * Unit: length of NDCacheRow_t array
 * Minimum: 1
 *
 * Maximum number of entries in the Neighbour Discovery cache of IPv6 addresses
 * & MAC addresses
 */

#ifndef ipconfigND_CACHE_ENTRIES
    #define ipconfigND_CACHE_ENTRIES    ( 24 )
#endif

#if ( ipconfigND_CACHE_ENTRIES < 1 )
    #error ipconfigND_CACHE_ENTRIES must be at least 1
#endif

#if ( ipconfigND_CACHE_ENTRIES > SIZE_MAX )
    #error ipconfigND_CACHE_ENTRIES overflows a size_t
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigMAX_ND_AGE
 *
 * Type: uint8_t
 * Unit: decaseconds
 * Minimum: 0
 *
 * Defines the maximum time between an entry in the ND table being created or
 * refreshed and the entry being removed because it is stale. New ND requests
 * are sent for ND cache entries that are nearing their maximum age.
 * The maximum age of an entry in the ND cache table can be
 * calculated as 'ipND_TIMER_PERIOD_MS' x 'ipconfigMAX_ND_AGE'.
 *
 * Units are derived from ipND_TIMER_PERIOD_MS, which is 10000 ms or 10 sec.
 * So, a value of 150 is equal to 1500 seconds.
 */

#ifndef ipconfigMAX_ND_AGE
    #define ipconfigMAX_ND_AGE    ( 150 )
#endif

#if ( ipconfigMAX_ND_AGE < 0 )
    #error ipconfigMAX_ND_AGE must be at least 0
#endif

#if ( ipconfigMAX_ND_AGE > UINT8_MAX )
    #error ipconfigMAX_ND_AGE overflows a uint8_t
#endif

/*---------------------------------------------------------------------------*/

/*===========================================================================*/
/*                                ND CONFIG                                  */
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/*                               ICMP CONFIG                                 */
/*===========================================================================*/

/*---------------------------------------------------------------------------*/

/*
 * ipconfigREPLY_TO_INCOMING_PINGS
 *
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html#ipconfigREPLY_TO_INCOMING_PINGS
 *
 * Type: BaseType_t ( ipconfigENABLE | ipconfigDISABLE )
 *
 * Enables generation of replies to incoming ICMP echo (ping) requests.
 *
 * Normally it is quite desirable when embedded devices respond to a ping
 * request. Endpoints of the type IPv6 will reply to a ping request
 * unconditionally.
 */

#ifndef ipconfigREPLY_TO_INCOMING_PINGS
    #define ipconfigREPLY_TO_INCOMING_PINGS    ipconfigENABLE
#endif

#if ( ( ipconfigREPLY_TO_INCOMING_PINGS != ipconfigDISABLE ) && ( ipconfigREPLY_TO_INCOMING_PINGS != ipconfigENABLE ) )
    #error Invalid ipconfigREPLY_TO_INCOMING_PINGS configuration
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigICMP_TIME_TO_LIVE
 *
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html#ipconfigICMP_TIME_TO_LIVE
 *
 * Type: uint8_t
 * Unit: 'hops'
 * Minimum: 0
 *
 * Sets the value of the TTL field when replying to an ICMP packet.
 *
 * Only used when replying to an ICMP IPv4 ping request. The default of 64 is
 * recommended by RFC 1700.
 */

#ifndef ipconfigICMP_TIME_TO_LIVE
    #define ipconfigICMP_TIME_TO_LIVE    ( 64 )
#endif

#if ( ipconfigICMP_TIME_TO_LIVE < 0 )
    #error ipconfigICMP_TIME_TO_LIVE must be at least 0
#endif

#if ( ipconfigICMP_TIME_TO_LIVE > UINT8_MAX )
    #error ipconfigICMP_TIME_TO_LIVE overflows a uint8_t
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigSUPPORT_OUTGOING_PINGS
 *
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html#ipconfigSUPPORT_OUTGOING_PINGS
 *
 * Type: BaseType_t ( ipconfigENABLE | ipconfigDISABLE )
 *
 * Include support for FreeRTOS_SendPingRequest() and
 * FreeRTOS_SendPingRequestIPv6()
 */

#ifndef ipconfigSUPPORT_OUTGOING_PINGS
    #define ipconfigSUPPORT_OUTGOING_PINGS    ipconfigDISABLE
#endif

#if ( ( ipconfigSUPPORT_OUTGOING_PINGS != ipconfigDISABLE ) && ( ipconfigSUPPORT_OUTGOING_PINGS != ipconfigENABLE ) )
    #error Invalid ipconfigSUPPORT_OUTGOING_PINGS configuration
#endif

/*---------------------------------------------------------------------------*/

/*===========================================================================*/
/*                               ICMP CONFIG                                 */
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/*                              ROUTING CONFIG                               */
/*===========================================================================*/

/*---------------------------------------------------------------------------*/

/*
 * ipconfigCOMPATIBLE_WITH_SINGLE
 *
 * Type: BaseType_t ( ipconfigENABLE | ipconfigDISABLE )
 *
 * Retains compatibility with V3.x.x and older versions that only
 * supported one interface.
 */

#ifndef ipconfigCOMPATIBLE_WITH_SINGLE
    #define ipconfigCOMPATIBLE_WITH_SINGLE    ipconfigDISABLE
#endif

#if ( ( ipconfigCOMPATIBLE_WITH_SINGLE != ipconfigDISABLE ) && ( ipconfigCOMPATIBLE_WITH_SINGLE != ipconfigENABLE ) )
    #error Invalid ipconfigCOMPATIBLE_WITH_SINGLE configuration
#endif

/*---------------------------------------------------------------------------*/

/*===========================================================================*/
/*                              ROUTING CONFIG                               */
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/*                        DEBUG/TRACE/LOGGING CONFIG                         */
/*===========================================================================*/

/*---------------------------------------------------------------------------*/

/*
 * ipconfigCHECK_IP_QUEUE_SPACE
 *
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html#ipconfigCHECK_IP_QUEUE_SPACE
 *
 * Type: BaseType_t ( ipconfigENABLE | ipconfigDISABLE )
 *
 * Enables the IP-Task to track the run-time minimum free space that has
 * existed in the event queue (uxQueueMinimumSpace). This value can be
 * retrieved using the function uxGetMinimumIPQueueSpace().
 *
 * Enables vPrintResourceStats() to log warnings about shrinking queue space.
 *
 * See ipconfigEVENT_QUEUE_LENGTH for setting the length of the event queue.
 */

#ifndef ipconfigCHECK_IP_QUEUE_SPACE
    #define ipconfigCHECK_IP_QUEUE_SPACE    ipconfigDISABLE
#endif

#if ( ( ipconfigCHECK_IP_QUEUE_SPACE != ipconfigDISABLE ) && ( ipconfigCHECK_IP_QUEUE_SPACE != ipconfigENABLE ) )
    #error Invalid ipconfigCHECK_IP_QUEUE_SPACE configuration
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigHAS_DEBUG_PRINTF
 *
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html#ipconfigHAS_DEBUG_PRINTF
 *
 * Type: BaseType_t ( ipconfigENABLE | ipconfigDISABLE )
 *
 * ipconfigHAS_DEBUG_PRINTF enables usage of the macro FreeRTOS_debug_printf to
 * generate output messages mostly from the TCP/IP stack.
 *
 * Requires a reentrant application defined macro function
 * FreeRTOS_debug_printf with a return value that is ignored.
 *
 * The FreeRTOS_debug_printf() must be thread-safe but does not
 * have to be interrupt-safe.
 *
 * Example:
 *
 * void vLoggingPrintf( const char *pcFormatString, ... )
 *
 * #define ipconfigHAS_DEBUG_PRINTF    ipconfigENABLE
 * #define FreeRTOS_debug_printf( X )  if( ipconfigIS_ENABLED( ipconfigHAS_DEBUG_PRINTF ) vLoggingPrintf X
 *
 * FreeRTOS_debug_printf( ( "FunctionName: Failed with error code: %u\n", xErrorCode ) )
 */

#ifndef ipconfigHAS_DEBUG_PRINTF
    #define ipconfigHAS_DEBUG_PRINTF    ipconfigDISABLE
#endif

#if ( ( ipconfigHAS_DEBUG_PRINTF != ipconfigDISABLE ) && ( ipconfigHAS_DEBUG_PRINTF != ipconfigENABLE ) )
    #error Invalid ipconfigHAS_DEBUG_PRINTF configuration
#endif

#ifndef FreeRTOS_debug_printf
    #if ( ( ipconfigHAS_DEBUG_PRINTF == 1 ) && defined( configPRINTF ) )
        #define FreeRTOS_debug_printf( MSG )    do { configPRINTF( MSG ); } while( ipFALSE_BOOL )
    #else
        #define FreeRTOS_debug_printf( MSG )    do {} while( ipFALSE_BOOL )
    #endif
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigHAS_PRINTF
 *
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html#ipconfigHAS_PRINTF
 *
 * Type: BaseType_t ( ipconfigENABLE | ipconfigDISABLE )
 *
 * ipconfigHAS_PRINTF enables usage of the macro FreeRTOS_printf to
 * generate output messages mostly from TCP/IP stack demo applications.
 *
 * Requires a reentrant application defined macro function FreeRTOS_printf with
 * a return value that is ignored.
 *
 * The FreeRTOS_printf() must be thread-safe but does not have to be interrupt-safe.
 *
 * See ipconfigHAS_DEBUG_PRINTF
 */

#ifndef ipconfigHAS_PRINTF
    #define ipconfigHAS_PRINTF    ipconfigDISABLE
#endif

#if ( ( ipconfigHAS_PRINTF != ipconfigDISABLE ) && ( ipconfigHAS_PRINTF != ipconfigENABLE ) )
    #error Invalid ipconfigHAS_PRINTF configuration
#endif

#ifndef FreeRTOS_printf
    #if ( ( ipconfigHAS_PRINTF == 1 ) && defined( configPRINTF ) )
        #define FreeRTOS_printf( MSG )    do { configPRINTF( MSG ); } while( ipFALSE_BOOL )
    #else
        #define FreeRTOS_printf( MSG )    do {} while( ipFALSE_BOOL )
    #endif
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigTCP_IP_SANITY
 *
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html#ipconfigTCP_IP_SANITY
 *
 * Type: BaseType_t ( ipconfigENABLE | ipconfigDISABLE )
 *
 * Enables warnings when irregularities are detected when using
 * BufferAllocation_1.c.
 */

#ifndef ipconfigTCP_IP_SANITY
    #define ipconfigTCP_IP_SANITY    ipconfigDISABLE
#endif

#if ( ( ipconfigTCP_IP_SANITY != ipconfigDISABLE ) && ( ipconfigTCP_IP_SANITY != ipconfigENABLE ) )
    #error Invalid ipconfigTCP_IP_SANITY configuration
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigTCP_MAY_LOG_PORT
 *
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html#ipconfigTCP_MAY_LOG_PORT
 *
 * Type: Macro Function
 *
 * Specifies which port numbers should be logged by FreeRTOS_debug_printf().
 * The return value should be a BaseType_t ( pdFALSE | pdTRUE )
 * For example, the following definition will not generate log messages for
 * ports 23 or 2402:
 *
 * #define ipconfigTCP_MAY_LOG_PORT( xPort ) ( ( ( xPort ) != 23 ) && ( ( xPort ) != 2402 ) )
 */

#ifndef ipconfigTCP_MAY_LOG_PORT
    #define ipconfigTCP_MAY_LOG_PORT( xPort )    ( ( xPort ) != 23U )
#endif

#if ( ( ipconfigTCP_MAY_LOG_PORT( 0 ) != 0 ) && ( ipconfigTCP_MAY_LOG_PORT( 0 ) != 1 ) )
    #error ipconfigTCP_MAY_LOG_PORT() should equate to pdFALSE or pdTRUE
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigWATCHDOG_TIMER
 *
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html#ipconfigWATCHDOG_TIMER
 *
 * Type: Macro Function
 *
 * Macro that is called on each iteration of the IP task.
 *
 * May be useful if the application includes watchdog type functionality that
 * needs to know that the IP task is still cycling (although the fact that the
 * IP task is cycling does not necessarily indicate it is functioning
 * correctly). The return value is ignored.
 */

#ifndef ipconfigWATCHDOG_TIMER
    #define ipconfigWATCHDOG_TIMER()    do {} while( ipFALSE_BOOL )
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigUSE_DUMP_PACKETS
 *
 * Type: BaseType_t ( ipconfigENABLE | ipconfigDISABLE )
 *
 * See this utility: tools/tcp_utilities/tcp_dump_packets.md
 *
 * Allow inclusion of a utility that writes of network packets to files.
 *
 * Useful for testing and development. Assumes the presence of full stdio
 * disk access.
 */

#ifndef ipconfigUSE_DUMP_PACKETS
    #define ipconfigUSE_DUMP_PACKETS    ipconfigDISABLE
#endif

#if ( ( ipconfigUSE_DUMP_PACKETS != ipconfigDISABLE ) && ( ipconfigUSE_DUMP_PACKETS != ipconfigENABLE ) )
    #error Invalid ipconfigUSE_DUMP_PACKETS configuration
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigUSE_TCP_MEM_STATS
 *
 * Type: BaseType_t ( ipconfigENABLE | ipconfigDISABLE )
 *
 * See this utility: tools/tcp_utilities/tcp_mem_stats.md
 *
 * Allow inclusion of a utility that monitors all allocation and releases of
 * network-related resources.
 *
 * After running for a while, it will print all data
 * in a CSV format, which can be analysed in a spreadsheet program.
 */

#ifndef ipconfigUSE_TCP_MEM_STATS
    #define ipconfigUSE_TCP_MEM_STATS    ipconfigDISABLE
#endif

#if ( ( ipconfigUSE_TCP_MEM_STATS != ipconfigDISABLE ) && ( ipconfigUSE_TCP_MEM_STATS != ipconfigENABLE ) )
    #error Invalid ipconfigUSE_TCP_MEM_STATS configuration
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigTCP_MEM_STATS_MAX_ALLOCATION
 *
 * Type: size_t
 * Unit: count of TCP_ALLOCATION_t
 * Minimum: 1
 *
 * Defines the maximum number of allocations that can be stored/monitored.
 */

#ifndef ipconfigTCP_MEM_STATS_MAX_ALLOCATION
    #define ipconfigTCP_MEM_STATS_MAX_ALLOCATION    ( 128 )
#endif

#if ( ipconfigTCP_MEM_STATS_MAX_ALLOCATION < 1 )
    #error ipconfigTCP_MEM_STATS_MAX_ALLOCATION must be at least 1
#endif

#if ( ipconfigTCP_MEM_STATS_MAX_ALLOCATION > SIZE_MAX )
    #error ipconfigTCP_MEM_STATS_MAX_ALLOCATION overflows a size_t
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigENABLE_BACKWARD_COMPATIBILITY
 *
 * Type: BaseType_t ( ipconfigENABLE | ipconfigDISABLE )
 *
 * For backward compatibility define old structure names to the newer equivalent
 * structure name.
 */

#ifndef ipconfigENABLE_BACKWARD_COMPATIBILITY
    #define ipconfigENABLE_BACKWARD_COMPATIBILITY    ipconfigENABLE
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigPORT_SUPPRESS_WARNING
 *
 * Type: BaseType_t ( ipconfigENABLE | ipconfigDISABLE )
 *
 * For some use cases, users set configurations that issue warning messages.
 * This suppresses warnings in portable layers to make compilation clean.
 */

#ifndef ipconfigPORT_SUPPRESS_WARNING
    #define ipconfigPORT_SUPPRESS_WARNING    ipconfigDISABLE
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigINCLUDE_EXAMPLE_FREERTOS_PLUS_TRACE_CALLS
 *
 * Type: BaseType_t ( ipconfigENABLE | ipconfigDISABLE )
 *
 * The macro 'ipconfigINCLUDE_EXAMPLE_FREERTOS_PLUS_TRACE_CALLS' was
 * introduced to enable a tracing system. Currently it is only used in
 * BufferAllocation_2.c.
 */

#ifndef ipconfigINCLUDE_EXAMPLE_FREERTOS_PLUS_TRACE_CALLS
    #define ipconfigINCLUDE_EXAMPLE_FREERTOS_PLUS_TRACE_CALLS    ipconfigDISABLE
#endif

/*---------------------------------------------------------------------------*/

/*
 * ipconfigISO_STRICTNESS_VIOLATION_START, ipconfigISO_STRICTNESS_VIOLATION_END
 *
 * Type: compiler pragma injection macros
 *
 * These two macros enclose parts of the source that contain intentional
 * deviations from the ISO C standard. Users, and AI (I welcome our robot
 * overlords!), can use this to customize static analysis settings such as
 * the `-pedantic` flag in GCC. These should appear in very few places within
 * the FreeRTOS TCP source and should enclose only a line or two at a time.
 * When first introduced, these macros enclosed a single line of source code in
 * the sockets implementation.
 *
 * GCC example
 *
 * In gcc, to allow the Free RTOS TCP code to compile with `-pedantic` you can
 * define these macros as such:
 *
 * ```
 * // Last tested in GCC 10
 * #define ipconfigISO_STRICTNESS_VIOLATION_START _Pragma("GCC diagnostic push") \
 *  _Pragma("GCC diagnostic ignored \"-Wpedantic\"")
 *
 * #define ipconfigISO_STRICTNESS_VIOLATION_END _Pragma("GCC diagnostic pop")
 * ```
 */

#ifndef ipconfigISO_STRICTNESS_VIOLATION_START
    #define ipconfigISO_STRICTNESS_VIOLATION_START
#endif

#ifndef ipconfigISO_STRICTNESS_VIOLATION_END
    #define ipconfigISO_STRICTNESS_VIOLATION_END
#endif

/*---------------------------------------------------------------------------*/

/* Should only be included here, ensures trace config is set first. */
#include "IPTraceMacroDefaults.h"

/*---------------------------------------------------------------------------*/

/*===========================================================================*/
/*                        DEBUG/TRACE/LOGGING CONFIG                         */
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/

#endif /* FREERTOS_IP_CONFIG_DEFAULTS_H */
