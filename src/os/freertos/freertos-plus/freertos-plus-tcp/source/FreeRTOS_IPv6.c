/*
 * FreeRTOS+TCP V4.2.1
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
 * @file FreeRTOS_IPv6.c
 * @brief Implements the basic functionality for the FreeRTOS+TCP network stack.
 */

/* Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"

/* FreeRTOS+TCP includes. */
#include "FreeRTOS_IP.h"

/* *INDENT-OFF* */
#if( ipconfigUSE_IPv6 != 0 )
/* *INDENT-ON* */

/** @brief Get the scope field in IPv6 multicast address. */
#define IPv6MC_GET_SCOPE_VALUE( pxIPv6Address )                  ( ( ( pxIPv6Address )->ucBytes[ 1 ] ) & 0x0FU )

/** @brief Get the flags field in IPv6 multicast address. */
#define IPv6MC_GET_FLAGS_VALUE( pxIPv6Address )                  ( ( ( pxIPv6Address )->ucBytes[ 1 ] ) & 0xF0U )

/** @brief Get the group ID field in IPv6 multicast address. */
#define IPv6MC_GET_GROUP_ID( pxIPv6Address, pxReturnGroupID )    ( xGetIPv6MulticastGroupID( pxIPv6Address, pxReturnGroupID ) )

/**
 * This variable is initialized by the system to contain the wildcard IPv6 address.
 */
const struct xIPv6_Address FreeRTOS_in6addr_any = { 0 };

/**
 * This variable is initialized by the system to contain the loopback IPv6 address.
 */
/* MISRA Ref 8.9.1 [File scoped variables] */
/* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-89 */
/* coverity[misra_c_2012_rule_8_9_violation] */
const struct xIPv6_Address FreeRTOS_in6addr_loopback = { { 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 1U } };

#if ( ipconfigDRIVER_INCLUDED_RX_IP_CHECKSUM == 1 )
    /* Check IPv6 packet length. */
    static BaseType_t xCheckIPv6SizeFields( const void * const pvEthernetBuffer,
                                            size_t uxBufferLength );
#endif /* ( ipconfigDRIVER_INCLUDED_RX_IP_CHECKSUM == 1 ) */

#if ( ipconfigDRIVER_INCLUDED_RX_IP_CHECKSUM == 1 )
    /* Check if ucNextHeader is an extension header. */
    static BaseType_t xIsExtHeader( uint8_t ucNextHeader );
#endif /* ( ipconfigDRIVER_INCLUDED_RX_IP_CHECKSUM == 1 ) */

#if ( ipconfigDRIVER_INCLUDED_RX_IP_CHECKSUM == 1 )

/**
 * @brief Check IPv6 packet length.
 *
 * @param[in] pvEthernetBuffer The Ethernet packet received.
 * @param[in] uxBufferLength The total number of bytes received.
 *
 * @return pdPASS when the length fields in the packet OK, pdFAIL when the packet
 *         should be dropped.
 */
    static BaseType_t xCheckIPv6SizeFields( const void * const pvEthernetBuffer,
                                            size_t uxBufferLength )
    {
        BaseType_t xResult = pdFAIL;
        uint16_t ucVersionTrafficClass;
        uint16_t usPayloadLength;
        uint8_t ucNextHeader;
        size_t uxMinimumLength;
        size_t uxExtHeaderLength = 0;
        const IPExtHeader_IPv6_t * pxExtHeader = NULL;
        const uint8_t * const pucEthernetBuffer = ( uint8_t * ) pvEthernetBuffer;

        /* Map the buffer onto a IPv6-Packet struct to easily access the
         * fields of the IPv6 packet. */
        const IPPacket_IPv6_t * const pxIPv6Packet = ( IPPacket_IPv6_t * ) pucEthernetBuffer;

        DEBUG_DECLARE_TRACE_VARIABLE( BaseType_t, xLocation, 0 );

        do
        {
            /* Check for minimum packet size: Ethernet header and an IPv6-header, 54 bytes */
            if( uxBufferLength < sizeof( IPHeader_IPv6_t ) )
            {
                DEBUG_SET_TRACE_VARIABLE( xLocation, 1 );
                break;
            }

            ucVersionTrafficClass = pxIPv6Packet->xIPHeader.ucVersionTrafficClass;

            /* Test if the IP-version is 6. */
            if( ( ( ucVersionTrafficClass & ( uint8_t ) 0xF0U ) >> 4 ) != 6U )
            {
                DEBUG_SET_TRACE_VARIABLE( xLocation, 2 );
                break;
            }

            /* Check if the IPv6-header is transferred. */
            if( uxBufferLength < ( ipSIZE_OF_ETH_HEADER + ipSIZE_OF_IPv6_HEADER ) )
            {
                DEBUG_SET_TRACE_VARIABLE( xLocation, 3 );
                break;
            }

            /* Check if the complete IPv6-header plus protocol data have been transferred: */
            usPayloadLength = FreeRTOS_ntohs( pxIPv6Packet->xIPHeader.usPayloadLength );

            if( uxBufferLength != ( size_t ) ( ipSIZE_OF_ETH_HEADER + ipSIZE_OF_IPv6_HEADER + ( size_t ) usPayloadLength ) )
            {
                DEBUG_SET_TRACE_VARIABLE( xLocation, 4 );
                break;
            }

            /* Identify the next protocol. */
            ucNextHeader = pxIPv6Packet->xIPHeader.ucNextHeader;

            while( xIsExtHeader( ucNextHeader ) )
            {
                pxExtHeader = ( const IPExtHeader_IPv6_t * ) ( &( pucEthernetBuffer[ ipSIZE_OF_ETH_HEADER + ipSIZE_OF_IPv6_HEADER + uxExtHeaderLength ] ) );
                /* The definition of length in extension header - Length of this header in 8-octet units, not including the first 8 octets. */
                uxExtHeaderLength += ( size_t ) ( ( 8 * pxExtHeader->ucHeaderExtLength ) + 8 );

                ucNextHeader = pxExtHeader->ucNextHeader;

                if( ipSIZE_OF_ETH_HEADER + ipSIZE_OF_IPv6_HEADER + uxExtHeaderLength >= uxBufferLength )
                {
                    break;
                }
            }

            if( ipSIZE_OF_ETH_HEADER + ipSIZE_OF_IPv6_HEADER + uxExtHeaderLength >= uxBufferLength )
            {
                DEBUG_SET_TRACE_VARIABLE( xLocation, 7 );
                break;
            }

            /* Switch on the Layer 3/4 protocol. */
            if( ucNextHeader == ( uint8_t ) ipPROTOCOL_UDP )
            {
                /* Expect at least a complete UDP header. */
                uxMinimumLength = ipSIZE_OF_ETH_HEADER + ipSIZE_OF_IPv6_HEADER + uxExtHeaderLength + ipSIZE_OF_UDP_HEADER;
            }
            else if( ucNextHeader == ( uint8_t ) ipPROTOCOL_TCP )
            {
                uxMinimumLength = ipSIZE_OF_ETH_HEADER + ipSIZE_OF_IPv6_HEADER + uxExtHeaderLength + ipSIZE_OF_TCP_HEADER;
            }
            else if( ucNextHeader == ( uint8_t ) ipPROTOCOL_ICMP_IPv6 )
            {
                uint8_t ucTypeOfMessage;

                uxMinimumLength = ipSIZE_OF_ETH_HEADER + ipSIZE_OF_IPv6_HEADER + uxExtHeaderLength;

                ucTypeOfMessage = pucEthernetBuffer[ uxMinimumLength ];

                if( ( ucTypeOfMessage == ipICMP_PING_REQUEST_IPv6 ) ||
                    ( ucTypeOfMessage == ipICMP_PING_REPLY_IPv6 ) )
                {
                    uxMinimumLength += sizeof( ICMPEcho_IPv6_t );
                }
                else if( ucTypeOfMessage == ipICMP_ROUTER_SOLICITATION_IPv6 )
                {
                    uxMinimumLength += sizeof( ICMPRouterSolicitation_IPv6_t );
                }
                else if( ucTypeOfMessage == ipICMP_ROUTER_ADVERTISEMENT_IPv6 )
                {
                    uxMinimumLength += sizeof( ICMPRouterAdvertisement_IPv6_t );
                }
                else
                {
                    uxMinimumLength += ipSIZE_OF_ICMPv6_HEADER;
                }
            }
            else
            {
                /* Unhandled protocol, other than ICMP, IGMP, UDP, or TCP. */
                DEBUG_SET_TRACE_VARIABLE( xLocation, 5 );
                break;
            }

            if( uxBufferLength < uxMinimumLength )
            {
                DEBUG_SET_TRACE_VARIABLE( xLocation, 6 );
                break;
            }

            xResult = pdPASS;
        } while( ipFALSE_BOOL );

        if( xResult != pdPASS )
        {
            /* NOP if ipconfigHAS_PRINTF != 1 */
            FreeRTOS_printf( ( "xCheckIPv6SizeFields: location %ld\n", xLocation ) );
        }

        return xResult;
    }


#endif /* ( ipconfigDRIVER_INCLUDED_RX_IP_CHECKSUM == 1 ) */
/*-----------------------------------------------------------*/


#if ( ipconfigDRIVER_INCLUDED_RX_IP_CHECKSUM == 1 )

/**
 * @brief Check if ucNextHeader is an extension header.
 *
 * @param[in] ucNextHeader Next header, such as ipIPv6_EXT_HEADER_HOP_BY_HOP.
 *
 * @return pdTRUE if it's extension header, otherwise pdFALSE.
 */
    static BaseType_t xIsExtHeader( uint8_t ucNextHeader )
    {
        BaseType_t xReturn = pdFALSE;

        switch( ucNextHeader )
        {
            case ipIPv6_EXT_HEADER_HOP_BY_HOP:
            case ipIPv6_EXT_HEADER_ROUTING_HEADER:
            case ipIPv6_EXT_HEADER_FRAGMENT_HEADER:
            case ipIPv6_EXT_HEADER_SECURE_PAYLOAD:
            case ipIPv6_EXT_HEADER_AUTHEN_HEADER:
            case ipIPv6_EXT_HEADER_DESTINATION_OPTIONS:
            case ipIPv6_EXT_HEADER_MOBILITY_HEADER:
                xReturn = pdTRUE;
        }

        return xReturn;
    }


#endif /* ( ipconfigDRIVER_INCLUDED_RX_IP_CHECKSUM == 1 ) */
/*-----------------------------------------------------------*/

/**
 * @brief Get the group ID and stored into IPv6_Address_t.
 *
 * @param[in] pxIPv6Address The multicast address to filter group ID.
 * @param[out] pxReturnGroupID The buffer to store group ID.
 */
static void xGetIPv6MulticastGroupID( const IPv6_Address_t * pxIPv6Address,
                                      IPv6_Address_t * pxReturnGroupID )
{
    configASSERT( pxIPv6Address != NULL );
    configASSERT( pxReturnGroupID != NULL );

    pxReturnGroupID->ucBytes[ 0 ] = 0U;
    pxReturnGroupID->ucBytes[ 1 ] = 0U;
    ( void ) memcpy( &( pxReturnGroupID->ucBytes[ 2 ] ), &( pxIPv6Address->ucBytes[ 2 ] ), 14 );
}


/*-----------------------------------------------------------*/

/**
 * @brief Check if the IP-address is an IPv6 loopback address.
 *
 * @param[in] pxAddress The IP-address being checked.
 *
 * @return pdTRUE if the IP-address is a loopback address or else, pdFALSE.
 */
BaseType_t xIsIPv6Loopback( const IPv6_Address_t * pxAddress )
{
    BaseType_t xReturn = pdFALSE;

    if( memcmp( pxAddress->ucBytes, FreeRTOS_in6addr_loopback.ucBytes, ipSIZE_OF_IPv6_ADDRESS ) == 0 )
    {
        xReturn = pdTRUE;
    }

    return xReturn;
}

#if ( ipconfigETHERNET_DRIVER_FILTERS_PACKETS == 0 )

/**
 * @brief Check if the packet is an illegal loopback packet.
 *
 * @param[in] pxIPv6Header The IP-header of the packet.
 *
 * @return Returns pdTRUE if the packet should be stopped, because either the source
 *         or the target address is a loopback address.
 */
/* MISRA Ref 8.9.1 [File scoped variables] */
/* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-89 */
/* coverity[misra_c_2012_rule_8_9_violation] */
/* coverity[single_use] */
    BaseType_t xBadIPv6Loopback( const IPHeader_IPv6_t * const pxIPv6Header )
    {
        BaseType_t xReturn = pdFALSE;
        const NetworkEndPoint_t * pxEndPoint = FreeRTOS_FindEndPointOnIP_IPv6( &( pxIPv6Header->xSourceAddress ) );

        /* Allow loopback packets from this node itself only. */
        if( pxEndPoint != NULL )
        {
            BaseType_t x1 = ( xIsIPv6Loopback( &( pxIPv6Header->xDestinationAddress ) ) != 0 ) ? pdTRUE : pdFALSE;
            BaseType_t x2 = ( xIsIPv6Loopback( &( pxIPv6Header->xSourceAddress ) ) != 0 ) ? pdTRUE : pdFALSE;

            if( x1 != x2 )
            {
                /* Either source or the destination address is a loopback address. */
                xReturn = pdTRUE;
            }
        }

        return xReturn;
    }

#endif /* ipconfigETHERNET_DRIVER_FILTERS_PACKETS == 0 */


/*-----------------------------------------------------------*/

/**
 * @brief Check whether this IPv6 address is an allowed multicast address or not.
 *
 * @param[in] pxIPAddress The IP address to be checked.
 *
 * @return Returns pdTRUE if pxIPAddress is an allowed multicast address, pdFALSE if not.
 */
BaseType_t xIsIPv6AllowedMulticast( const IPv6_Address_t * pxIPAddress )
{
    BaseType_t xReturn = pdFALSE;
    IPv6_Address_t xGroupIDAddress;

    if( pxIPAddress->ucBytes[ 0 ] == 0xffU )
    {
        IPv6MC_GET_GROUP_ID( pxIPAddress, &xGroupIDAddress );

        /* From RFC4291 - sec 2.7, packets from multicast address whose scope field is 0
         * should be silently dropped. */
        if( IPv6MC_GET_SCOPE_VALUE( pxIPAddress ) == 0U )
        {
            xReturn = pdFALSE;
        }

        /* From RFC4291 - sec 2.7.1, packets from predefined multicast address should never be used.
         * - 0xFF00::
         * - 0xFF01::
         * - ..
         * - 0xFF0F:: */
        else if( ( IPv6MC_GET_FLAGS_VALUE( pxIPAddress ) == 0U ) &&
                 ( memcmp( xGroupIDAddress.ucBytes, FreeRTOS_in6addr_any.ucBytes, sizeof( IPv6_Address_t ) ) == 0 ) )
        {
            xReturn = pdFALSE;
        }
        else
        {
            xReturn = pdTRUE;
        }
    }

    return xReturn;
}


/*-----------------------------------------------------------*/

/**
 * @brief Compares 2 IPv6 addresses and checks if the one
 * on the left can handle the one on right. Note that 'xCompareIPv6_Address' will also check if 'pxRight' is
 * the special unicast address: ff02::1:ffnn:nnnn, where nn:nnnn are
 * the last 3 bytes of the IPv6 address.
 *
 * @param[in] pxLeft First IP address.
 * @param[in] pxRight Second IP address.
 * @param[in] uxPrefixLength The IP address prefix length in bits.
 *
 * @return Returns 0 if it can handle it, else non zero .
 */
BaseType_t xCompareIPv6_Address( const IPv6_Address_t * pxLeft,
                                 const IPv6_Address_t * pxRight,
                                 size_t uxPrefixLength )
{
    BaseType_t xResult;
    /* This variable is initialized by the system to contain the IPv6 multicast address for all nodes. */
    static const struct xIPv6_Address FreeRTOS_in6addr_allnodes = { { 0xff, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 } };

    /* 0    2    4    6    8    10   12   14 */
    /* ff02:0000:0000:0000:0000:0001:ff66:4a81 */
    if( ( pxRight->ucBytes[ 0 ] == 0xffU ) &&
        ( pxRight->ucBytes[ 1 ] == 0x02U ) &&
        ( pxRight->ucBytes[ 12 ] == 0xffU ) )
    {
        /* This is an LLMNR address. */
        xResult = memcmp( &( pxLeft->ucBytes[ 13 ] ), &( pxRight->ucBytes[ 13 ] ), 3 );
    }
    else
    if( memcmp( pxRight->ucBytes, FreeRTOS_in6addr_allnodes.ucBytes, sizeof( IPv6_Address_t ) ) == 0 )
    {
        /* FF02::1 is all node address to reach out all nodes in the same link. */
        xResult = 0;
    }
    else
    if( ( pxRight->ucBytes[ 0 ] == 0xfeU ) &&
        ( pxRight->ucBytes[ 1 ] == 0x80U ) &&
        ( pxLeft->ucBytes[ 0 ] == 0xfeU ) &&
        ( pxLeft->ucBytes[ 1 ] == 0x80U ) )
    {
        /* Both are local addresses. */
        xResult = 0;
    }
    else
    {
        if( uxPrefixLength == 0U )
        {
            xResult = 0;
        }
        else if( uxPrefixLength == ( 8U * ipSIZE_OF_IPv6_ADDRESS ) )
        {
            xResult = memcmp( pxLeft->ucBytes, pxRight->ucBytes, ipSIZE_OF_IPv6_ADDRESS );
        }
        else
        {
            size_t uxLength = uxPrefixLength / 8U;

            xResult = 0;

            if( uxLength > 0U )
            {
                xResult = memcmp( pxLeft->ucBytes, pxRight->ucBytes, uxLength );
            }

            if( ( xResult == 0 ) && ( ( uxPrefixLength % 8U ) != 0U ) )
            {
                /* One byte has both a network- and a host-address. */
                size_t uxBits = uxPrefixLength % 8U;
                size_t uxHostLen = 8U - uxBits;
                uint32_t uxHostMask = ( ( ( uint32_t ) 1U ) << uxHostLen ) - 1U;
                uint8_t ucNetMask = ( uint8_t ) ~( uxHostMask );

                if( ( pxLeft->ucBytes[ uxLength ] & ucNetMask ) != ( pxRight->ucBytes[ uxLength ] & ucNetMask ) )
                {
                    xResult = 1;
                }
            }
        }
    }

    return xResult;
}


/*-----------------------------------------------------------*/


/**
 * @brief Check whether this IPv6 packet is to be allowed or to be dropped.
 *
 * @param[in] pxIPv6Header The IP packet under consideration.
 * @param[in] pxNetworkBuffer The whole network buffer.
 * @param[in] uxHeaderLength The length of the header.
 *
 * @return Whether the packet should be processed or dropped.
 */
eFrameProcessingResult_t prvAllowIPPacketIPv6( const IPHeader_IPv6_t * const pxIPv6Header,
                                               const NetworkBufferDescriptor_t * const pxNetworkBuffer,
                                               UBaseType_t uxHeaderLength )
{
    eFrameProcessingResult_t eReturn;

    #if ( ipconfigETHERNET_DRIVER_FILTERS_PACKETS == 0 )
    {
        /* In systems with a very small amount of RAM, it might be advantageous
         * to have incoming messages checked earlier, by the network card driver.
         * This method may decrease the usage of sparse network buffers. */
        const IPv6_Address_t * pxDestinationIPAddress = &( pxIPv6Header->xDestinationAddress );
        const IPv6_Address_t * pxSourceIPAddress = &( pxIPv6Header->xSourceAddress );
        BaseType_t xHasUnspecifiedAddress = pdFALSE;

        /* Drop if packet has unspecified IPv6 address (defined in RFC4291 - sec 2.5.2)
         * either in source or destination address. */
        if( ( memcmp( pxDestinationIPAddress->ucBytes, FreeRTOS_in6addr_any.ucBytes, sizeof( IPv6_Address_t ) ) == 0 ) ||
            ( memcmp( pxSourceIPAddress->ucBytes, FreeRTOS_in6addr_any.ucBytes, sizeof( IPv6_Address_t ) ) == 0 ) )
        {
            xHasUnspecifiedAddress = pdTRUE;
        }

        /* Is the packet for this IP address? */
        if( ( xHasUnspecifiedAddress == pdFALSE ) &&
            ( pxNetworkBuffer->pxEndPoint != NULL ) &&
            ( memcmp( pxDestinationIPAddress->ucBytes, pxNetworkBuffer->pxEndPoint->ipv6_settings.xIPAddress.ucBytes, sizeof( IPv6_Address_t ) ) == 0 ) )
        {
            eReturn = eProcessBuffer;
        }
        /* Is it the legal multicast address? */
        else if( ( ( xHasUnspecifiedAddress == pdFALSE ) &&
                   ( xBadIPv6Loopback( pxIPv6Header ) == pdFALSE ) ) &&
                 ( ( xIsIPv6AllowedMulticast( pxDestinationIPAddress ) != pdFALSE ) ||
                   /* Or (during DHCP negotiation) we have no IP-address yet? */
                   ( FreeRTOS_IsNetworkUp() == 0 ) ) )
        {
            eReturn = eProcessBuffer;
        }
        else
        {
            /* Packet is not for this node, or the network is still not up,
             * release it */
            eReturn = eReleaseBuffer;
            FreeRTOS_printf( ( "prvAllowIPPacketIPv6: drop %pip (from %pip)\n", pxDestinationIPAddress->ucBytes, pxIPv6Header->xSourceAddress.ucBytes ) );
        }
    }
    #else /* if ( ipconfigETHERNET_DRIVER_FILTERS_PACKETS == 0 ) */
    {
        ( void ) pxIPv6Header;
        /* The packet has been checked by the network interface. */
        eReturn = eProcessBuffer;
    }
    #endif /* ipconfigETHERNET_DRIVER_FILTERS_PACKETS */

    #if ( ipconfigDRIVER_INCLUDED_RX_IP_CHECKSUM == 0 )
    {
        /* Some drivers of NIC's with checksum-offloading will enable the above
         * define, so that the checksum won't be checked again here */
        if( eReturn == eProcessBuffer )
        {
            /* MISRA Ref 11.3.1 [Misaligned access] */
            /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
            /* coverity[misra_c_2012_rule_11_3_violation] */
            const IPPacket_t * pxIPPacket = ( ( const IPPacket_t * ) pxNetworkBuffer->pucEthernetBuffer );
            const NetworkEndPoint_t * pxEndPoint = FreeRTOS_FindEndPointOnMAC( &( pxIPPacket->xEthernetHeader.xSourceAddress ), NULL );

            /* IPv6 does not have a separate checksum in the IP-header */
            /* Is the upper-layer checksum (TCP/UDP/ICMP) correct? */
            /* Do not check the checksum of loop-back messages. */
            if( pxEndPoint == NULL )
            {
                if( usGenerateProtocolChecksum( ( uint8_t * ) ( pxNetworkBuffer->pucEthernetBuffer ), pxNetworkBuffer->xDataLength, pdFALSE ) != ipCORRECT_CRC )
                {
                    /* Protocol checksum not accepted. */
                    eReturn = eReleaseBuffer;
                }
            }
        }
    }
    #else /* if ( ipconfigDRIVER_INCLUDED_RX_IP_CHECKSUM == 0 ) */
    {
        if( eReturn == eProcessBuffer )
        {
            if( xCheckIPv6SizeFields( pxNetworkBuffer->pucEthernetBuffer, pxNetworkBuffer->xDataLength ) != pdPASS )
            {
                /* Some of the length checks were not successful. */
                eReturn = eReleaseBuffer;
            }
        }

        /* to avoid warning unused parameters */
        ( void ) pxNetworkBuffer;
    }
    #endif /* ipconfigDRIVER_INCLUDED_RX_IP_CHECKSUM == 0 */
    ( void ) uxHeaderLength;

    return eReturn;
}

/*-----------------------------------------------------------*/

/**
 * @brief Check extension header and next header and return their order.
 *
 * @param[in] ucProtocol Extension header ID.
 * @param[in] ucNextHeader Next header ID.
 *
 * @return Extension header order in the packet.
 */
BaseType_t xGetExtensionOrder( uint8_t ucProtocol,
                               uint8_t ucNextHeader )
{
    BaseType_t xReturn;

    switch( ucProtocol )
    {
        case ipIPv6_EXT_HEADER_HOP_BY_HOP:
            xReturn = 1;
            break;

        case ipIPv6_EXT_HEADER_DESTINATION_OPTIONS:
            xReturn = 7;

            if( ucNextHeader == ipIPv6_EXT_HEADER_ROUTING_HEADER )
            {
                xReturn = 2;
            }

            break;

        case ipIPv6_EXT_HEADER_ROUTING_HEADER:
            xReturn = 3;
            break;

        case ipIPv6_EXT_HEADER_FRAGMENT_HEADER:
            xReturn = 4;
            break;

        case ipIPv6_EXT_HEADER_AUTHEN_HEADER:
            xReturn = 5;
            break;

        case ipIPv6_EXT_HEADER_SECURE_PAYLOAD:
            xReturn = 6;
            break;

        /* Destination options may follow here in case there are no routing options. */
        case ipIPv6_EXT_HEADER_MOBILITY_HEADER:
            xReturn = 8;
            break;

        default:
            xReturn = -1;
            break;
    }

    return xReturn;
}


/*-----------------------------------------------------------*/



/**
 * @brief Handle the IPv6 extension headers.
 *
 * @param[in,out] pxNetworkBuffer The received packet that contains IPv6 extension headers.
 * @param[in] xDoRemove Function removes the extension header if xDoRemove is set to pdTRUE.
 *
 * @return eProcessBuffer in case the options are removed successfully, otherwise
 *         eReleaseBuffer.
 */
eFrameProcessingResult_t eHandleIPv6ExtensionHeaders( NetworkBufferDescriptor_t * const pxNetworkBuffer,
                                                      BaseType_t xDoRemove )
{
    eFrameProcessingResult_t eResult = eReleaseBuffer;
    const size_t uxMaxLength = pxNetworkBuffer->xDataLength;
    /* MISRA Ref 11.3.1 [Misaligned access] */
    /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
    /* coverity[misra_c_2012_rule_11_3_violation] */
    IPPacket_IPv6_t * pxIPPacket_IPv6 = ( ( IPPacket_IPv6_t * ) pxNetworkBuffer->pucEthernetBuffer );
    size_t xMoveLen = 0U;
    size_t uxRemovedBytes = 0U;
    uint8_t ucNextHeader = 0U;
    size_t uxIndex = 0U;

    uxRemovedBytes = usGetExtensionHeaderLength( pxNetworkBuffer->pucEthernetBuffer, pxNetworkBuffer->xDataLength, &ucNextHeader );
    uxIndex = ipSIZE_OF_ETH_HEADER + ipSIZE_OF_IPv6_HEADER + uxRemovedBytes;

    if( uxIndex < uxMaxLength )
    {
        uint8_t * pucTo;
        const uint8_t * pucFrom;
        uint16_t usPayloadLength = FreeRTOS_ntohs( pxIPPacket_IPv6->xIPHeader.usPayloadLength );

        if( uxRemovedBytes >= ( size_t ) usPayloadLength )
        {
            /* Can not remove more bytes than the payload length. */
        }
        else if( xDoRemove == pdTRUE )
        {
            pxIPPacket_IPv6->xIPHeader.ucNextHeader = ucNextHeader;
            pucTo = &( pxNetworkBuffer->pucEthernetBuffer[ ipSIZE_OF_ETH_HEADER + ipSIZE_OF_IPv6_HEADER ] );
            pucFrom = &( pxNetworkBuffer->pucEthernetBuffer[ uxIndex ] );
            xMoveLen = uxMaxLength - uxIndex;
            ( void ) memmove( pucTo, pucFrom, xMoveLen );
            pxNetworkBuffer->xDataLength -= uxRemovedBytes;

            usPayloadLength = ( uint16_t ) ( usPayloadLength - uxRemovedBytes );
            pxIPPacket_IPv6->xIPHeader.usPayloadLength = FreeRTOS_htons( usPayloadLength );
            eResult = eProcessBuffer;
        }
        else
        {
            /* xDoRemove is false, so the function is not supposed to
             * remove extension headers. */
        }
    }

    FreeRTOS_printf( ( "Extension headers : %s Truncated %u bytes. Removed %u, Payload %u xDataLength now %u\n",
                       ( eResult == eProcessBuffer ) ? "good" : "bad",
                       ( unsigned ) xMoveLen,
                       ( unsigned ) uxRemovedBytes,
                       FreeRTOS_ntohs( pxIPPacket_IPv6->xIPHeader.usPayloadLength ),
                       ( unsigned ) pxNetworkBuffer->xDataLength ) );
    return eResult;
}


/*-----------------------------------------------------------*/

/* *INDENT-OFF* */
#endif /* ( ipconfigUSE_IPv6 != 0 ) */
/* *INDENT-ON* */
