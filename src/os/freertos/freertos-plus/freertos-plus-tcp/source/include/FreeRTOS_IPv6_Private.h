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
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 */

#ifndef FREERTOS_IPV6_PRIVATE_H
#define FREERTOS_IPV6_PRIVATE_H

/* Application level configuration options. */
#include "FreeRTOSIPConfig.h"
#include "FreeRTOSIPConfigDefaults.h"
#include "FreeRTOS_IP_Common.h"
#include "FreeRTOS_Sockets.h"
#include "FreeRTOS_Stream_Buffer.h"
#if ( ipconfigUSE_TCP == 1 )
    #include "FreeRTOS_TCP_WIN.h"
    #include "FreeRTOS_TCP_IP.h"
#endif

#include "semphr.h"

#include "event_groups.h"

/* *INDENT-OFF* */
#ifdef __cplusplus
    extern "C" {
#endif
/* *INDENT-ON* */

/* MISRA Ref 20.5.1 [Use of undef] */
/* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-2051 */
/* coverity[misra_c_2012_rule_20_5_violation] */
#undef TCP_PACKET_SIZE
#define TCP_PACKET_SIZE          ( sizeof( TCPPacket_IPv6_t ) )

/* The offset into an IP packet into which the IP data (payload) starts. */
#define ipIPv6_PAYLOAD_OFFSET    ( sizeof( IPPacket_IPv6_t ) )
/* The maximum UDP payload length. */
/* MISRA Ref 20.5.1 [Use of undef] */
/* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-2051 */
/* coverity[misra_c_2012_rule_20_5_violation] */
#undef ipMAX_UDP_PAYLOAD_LENGTH
#define ipMAX_UDP_PAYLOAD_LENGTH     ( ( ipconfigNETWORK_MTU - ipSIZE_OF_IPv6_HEADER ) - ipSIZE_OF_UDP_HEADER )
/* The offset into a UDP packet at which the UDP data (payload) starts. */
#define ipUDP_PAYLOAD_OFFSET_IPv6    ( sizeof( UDPPacket_IPv6_t ) )

#if ( ipconfigBYTE_ORDER == pdFREERTOS_LITTLE_ENDIAN )

    #define ipIPv6_FRAME_TYPE    ( 0xDD86U )               /* Ethernet frame types. */

#else /* if ( ipconfigBYTE_ORDER == pdFREERTOS_LITTLE_ENDIAN ) */

    #define ipIPv6_FRAME_TYPE    ( 0x86DDU )               /* Ethernet frame types. */

#endif /* ipconfigBYTE_ORDER */

/* In this library, there is often a cast from a character pointer
 * to a pointer to a struct.
 * In order to suppress MISRA warnings, do the cast within a macro,
 * which can be exempt from warnings:
 *
 * 3 required by MISRA:
 * -emacro(740,ipPOINTER_CAST)    // 750:  Unusual pointer cast (incompatible indirect types) [MISRA 2012 Rule 1.3, required])
 * -emacro(9005,ipPOINTER_CAST)   // 9005: attempt to cast away const/volatile from a pointer or reference [MISRA 2012 Rule 11.8, required]
 * -emacro(9087,ipPOINTER_CAST)   // 9087: cast performed between a pointer to object type and a pointer to a different object type [MISRA 2012 Rule 11.3, required]
 *
 * 2 advisory by MISRA:
 * -emacro(9079,ipPOINTER_CAST)   // 9079: conversion from pointer to void to pointer to other type [MISRA 2012 Rule 11.5, advisory])
 * --emacro((826),ipPOINTER_CAST) // 826:  Suspicious pointer-to-pointer conversion (area too small)
 *
 * The MISRA warnings can safely be suppressed because all casts are planned with care.
 */

#define ipPOINTER_CAST( TYPE, pointer )    ( ( TYPE ) ( pointer ) )

/* Sequence and ACK numbers are essentially unsigned (uint32_t). But when
 * a distance is calculated, it is useful to use signed numbers:
 * int32_t lDistance = ( int32_t ) ( ulSeq1 - ulSeq2 );
 *
 * 1 required by MISRA:
 * -emacro(9033,ipNUMERIC_CAST) // 9033: Impermissible cast of composite expression (different essential type categories) [MISRA 2012 Rule 10.8, required])
 *
 * 1 advisory by MISRA:
 * -emacro(9030,ipNUMERIC_CAST) // 9030: Impermissible cast; cannot cast from 'essentially Boolean' to 'essentially signed' [MISRA 2012 Rule 10.5, advisory])
 */

#define ipNUMERIC_CAST( TYPE, expression )    ( ( TYPE ) ( expression ) )


/** @brief The macros vSetField16() and vSetField32() will write either a short or a 32-bit
 * value into an array of bytes. They will be stored big-endian.
 * The helper functions do the actual work.
 */

/*extern void vSetField16helper( uint8_t * pucBase,
 *                             size_t uxOffset,
 *                             uint16_t usValue );
 #define vSetField16( pucBase, xType, xField, usValue ) \
 *  vSetField16helper( pucBase, offsetof( xType, xField ), usValue )
 *
 * extern void vSetField32helper( uint8_t * pucBase,
 *                             size_t uxOffset,
 *                             uint32_t ulValue );
 #define vSetField32( pucBase, xType, xField, ulValue ) \
 *  vSetField32helper( pucBase, offsetof( xType, xField ), ulValue )
 */

/* As FreeRTOS_Routing is included later, use forward declarations
 * of the two structs. */
struct xNetworkEndPoint;
struct xNetworkInterface;

#include "pack_struct_start.h"
struct xIP_HEADER_IPv6
{
    uint8_t ucVersionTrafficClass;      /**< The version field.                      0 +  1 =  1 */
    uint8_t ucTrafficClassFlow;         /**< Traffic class and flow.                 1 +  1 =  2 */
    uint16_t usFlowLabel;               /**< Flow label.                             2 +  2 =  4 */
    uint16_t usPayloadLength;           /**< Number of bytes after the IPv6 header.  4 +  2 =  6 */
    uint8_t ucNextHeader;               /**< Next header: TCP, UDP, or ICMP.         6 +  1 =  7 */
    uint8_t ucHopLimit;                 /**< Replaces the time to live from IPv4.    7 +  1 =  8 */
    IPv6_Address_t xSourceAddress;      /**< The IPv6 address of the sender.         8 + 16 = 24 */
    IPv6_Address_t xDestinationAddress; /**< The IPv6 address of the receiver.      24 + 16 = 40 */
}
#include "pack_struct_end.h"
typedef struct xIP_HEADER_IPv6 IPHeader_IPv6_t;

#include "pack_struct_start.h"
struct xIP_EXT_HEADER_IPv6
{
    uint8_t ucNextHeader;      /**< Next header: TCP, UDP, or ICMP.                                            0 +  1 =  1 */
    uint8_t ucHeaderExtLength; /**< Length of this header in 8-octet units, not including the first 8 octets.  1 +  1 =  2 */
}
#include "pack_struct_end.h"
typedef struct xIP_EXT_HEADER_IPv6 IPExtHeader_IPv6_t;

#include "pack_struct_start.h"
struct xICMPEcho_IPv6
{
    uint8_t ucTypeOfMessage;   /**< The message type.     0 +  1 = 1 */
    uint8_t ucTypeOfService;   /**< Type of service.      1 +  1 = 2 */
    uint16_t usChecksum;       /**< Checksum.             2 +  2 = 4 */
    uint16_t usIdentifier;     /**< Identifier.           4 +  2 = 6 */
    uint16_t usSequenceNumber; /**< Sequence number.      6 +  2 = 8 */
}
#include "pack_struct_end.h"
typedef struct xICMPEcho_IPv6 ICMPEcho_IPv6_t;

#include "pack_struct_start.h"
struct xICMPRouterSolicitation_IPv6
{
    uint8_t ucTypeOfMessage; /**<  0 +  1 =  1 */
    uint8_t ucTypeOfService; /**<  1 +  1 =  2 */
    uint16_t usChecksum;     /**<  2 +  2 =  4 */
    uint32_t ulReserved;     /**<  4 +  4 =  8 */
}
#include "pack_struct_end.h"
typedef struct xICMPRouterSolicitation_IPv6 ICMPRouterSolicitation_IPv6_t;

#include "pack_struct_start.h"
struct xICMPRouterAdvertisement_IPv6
{
    uint8_t ucTypeOfMessage;       /*  0 +  1 =  1 */
    uint8_t ucTypeOfService;       /*  1 +  1 =  2 */
    uint16_t usChecksum;           /*  2 +  2 =  4 */
    uint8_t ucHopLimit;            /*  4 +  1 =  5 */
    uint8_t ucFlags;               /*  5 +  1 =  6 */
    uint16_t usLifetime;           /*  6 +  2 =  8 */
    uint16_t usReachableTime[ 2 ]; /*  8 +  4 = 12 */
    uint16_t usRetransTime[ 2 ];   /* 12 +  4 = 16 */
}
#include "pack_struct_end.h"
typedef struct xICMPRouterAdvertisement_IPv6 ICMPRouterAdvertisement_IPv6_t;

#if ( ipconfigUSE_RA != 0 )
    #include "pack_struct_start.h"
    struct xICMPPrefixOption_IPv6
    {
        uint8_t ucType;               /*  0 +  1 =  1 */
        uint8_t ucLength;             /*  1 +  1 =  2 */
        uint8_t ucPrefixLength;       /*  2 +  1 =  3 */
        uint8_t ucFlags;              /*  3 +  1 =  4 */
        uint32_t ulValidLifeTime;     /*  4 +  4 =  8 */
        uint32_t ulPreferredLifeTime; /*  8 +  4 = 12 */
        uint32_t ulReserved;          /* 12 +  4 = 16 */
        uint8_t ucPrefix[ 16 ];       /* 16 + 16 = 32 */
    }
    #include "pack_struct_end.h"
    typedef struct xICMPPrefixOption_IPv6 ICMPPrefixOption_IPv6_t;
#endif /* ipconfigUSE_RA != 0 */

/*-----------------------------------------------------------*/
/* Nested protocol packets.                                  */
/*-----------------------------------------------------------*/

#include "pack_struct_start.h"
struct xIP_PACKET_IPv6
{
    EthernetHeader_t xEthernetHeader;
    IPHeader_IPv6_t xIPHeader;
}
#include "pack_struct_end.h"
typedef struct xIP_PACKET_IPv6 IPPacket_IPv6_t;

#include "pack_struct_start.h"
struct xICMP_PACKET_IPv6
{
    EthernetHeader_t xEthernetHeader;  /*  0 + 14 = 14 */
    IPHeader_IPv6_t xIPHeader;         /* 14 + 40 = 54 */
    ICMPHeader_IPv6_t xICMPHeaderIPv6; /* 54 +  8 = 62 */
}
#include "pack_struct_end.h"
typedef struct xICMP_PACKET_IPv6 ICMPPacket_IPv6_t;

#include "pack_struct_start.h"
struct xUDP_PACKET_IPv6
{
    EthernetHeader_t xEthernetHeader; /*  0 + 14 = 14 */
    IPHeader_IPv6_t xIPHeader;        /* 14 + 40 = 54 */
    UDPHeader_t xUDPHeader;           /* 54 +  8 = 62 */
}
#include "pack_struct_end.h"
typedef struct xUDP_PACKET_IPv6 UDPPacket_IPv6_t;

#include "pack_struct_start.h"
struct xTCP_PACKET_IPv6
{
    EthernetHeader_t xEthernetHeader; /*  0 + 14 = 14 */
    IPHeader_IPv6_t xIPHeader;        /* 14 + 40 = 54 */
    TCPHeader_t xTCPHeader;           /* 54 + 32 = 86 */
}
#include "pack_struct_end.h"
typedef struct xTCP_PACKET_IPv6 TCPPacket_IPv6_t;

/* prvProcessICMPMessage_IPv6() is declared in FreeRTOS_routing.c
 * It handles all ICMP messages except the PING requests. */
eFrameProcessingResult_t prvProcessICMPMessage_IPv6( NetworkBufferDescriptor_t * const pxNetworkBuffer );


#if ( ( ipconfigHAS_DEBUG_PRINTF != 0 ) || ( ipconfigHAS_PRINTF != 0 ) )
/* prepare a string which describes a socket, just for logging. */
    const char * prvSocketProps( FreeRTOS_Socket_t * pxSocket );
#endif /* ipconfigHAS_DEBUG_PRINTF || ipconfigHAS_PRINTF */

/* *INDENT-OFF* */
#ifdef __cplusplus
    } /* extern "C" */
#endif
/* *INDENT-ON* */

#endif /* FREERTOS_IPV6_PRIVATE_H */
