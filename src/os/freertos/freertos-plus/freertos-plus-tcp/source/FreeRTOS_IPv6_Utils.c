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
 * @file FreeRTOS_IPv6_Utils.c
 * @brief Implements the basic functionality for the FreeRTOS+TCP network stack functions for IPv6.
 */

/* Standard includes. */
#include <stdint.h>
#include <stdio.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"

/* FreeRTOS+TCP includes. */
#include "FreeRTOS_IP.h"

/*-----------------------------------------------------------*/

/* *INDENT-OFF* */
#if( ipconfigUSE_IPv6 != 0 )
/* *INDENT-ON* */

/**
 * @brief Set multicast MAC address.
 *
 * @param[in] pxAddress IPv6 address.
 * @param[out] pxMACAddress Pointer to MAC address.
 */
void vSetMultiCastIPv6MacAddress( const IPv6_Address_t * pxAddress,
                                  MACAddress_t * pxMACAddress )
{
    pxMACAddress->ucBytes[ 0 ] = ipMULTICAST_MAC_ADDRESS_IPv6_0;
    pxMACAddress->ucBytes[ 1 ] = ipMULTICAST_MAC_ADDRESS_IPv6_1;
    pxMACAddress->ucBytes[ 2 ] = pxAddress->ucBytes[ 12 ];
    pxMACAddress->ucBytes[ 3 ] = pxAddress->ucBytes[ 13 ];
    pxMACAddress->ucBytes[ 4 ] = pxAddress->ucBytes[ 14 ];
    pxMACAddress->ucBytes[ 5 ] = pxAddress->ucBytes[ 15 ];
}
/*-----------------------------------------------------------*/

/** @brief Do the first IPv6 length checks at the IP-header level.
 * @param[in] pucEthernetBuffer The buffer containing the packet.
 * @param[in] uxBufferLength The number of bytes to be sent or received.
 * @param[in] pxSet A struct describing this packet.
 *
 * @return Non-zero in case of an error.
 */
BaseType_t prvChecksumIPv6Checks( uint8_t * pucEthernetBuffer,
                                  size_t uxBufferLength,
                                  struct xPacketSummary * pxSet )
{
    BaseType_t xReturn = 0;
    size_t uxExtensionHeaderLength = 0;

    pxSet->xIsIPv6 = pdTRUE;

    pxSet->uxIPHeaderLength = ipSIZE_OF_IPv6_HEADER;

    /* Check for minimum packet size: ipSIZE_OF_ETH_HEADER + ipSIZE_OF_IPv6_HEADER (54 bytes) */
    if( uxBufferLength < sizeof( IPPacket_IPv6_t ) )
    {
        pxSet->usChecksum = ipINVALID_LENGTH;
        xReturn = 1;
    }
    else
    {
        uxExtensionHeaderLength = usGetExtensionHeaderLength( pucEthernetBuffer, uxBufferLength, &pxSet->ucProtocol );

        if( uxExtensionHeaderLength >= uxBufferLength )
        {
            /* Error detected when parsing extension header. */
            pxSet->usChecksum = ipINVALID_LENGTH;
            xReturn = 3;
        }
        else
        {
            size_t uxNeeded;

            /* MISRA Ref 11.3.1 [Misaligned access] */
            /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
            /* coverity[misra_c_2012_rule_11_3_violation] */
            pxSet->pxProtocolHeaders = ( ( ProtocolHeaders_t * ) &( pucEthernetBuffer[ ipSIZE_OF_ETH_HEADER + ipSIZE_OF_IPv6_HEADER + uxExtensionHeaderLength ] ) );
            pxSet->usPayloadLength = FreeRTOS_ntohs( pxSet->pxIPPacket_IPv6->usPayloadLength );
            /* For IPv6, the number of bytes in the protocol is indicated. */
            pxSet->usProtocolBytes = ( uint16_t ) ( pxSet->usPayloadLength - uxExtensionHeaderLength );

            uxNeeded = ( size_t ) pxSet->usPayloadLength;
            uxNeeded += ipSIZE_OF_ETH_HEADER + ipSIZE_OF_IPv6_HEADER;

            if( uxBufferLength < uxNeeded )
            {
                /* The packet does not contain a complete IPv6 packet. */
                pxSet->usChecksum = ipINVALID_LENGTH;
                xReturn = 2;
            }
        }
    }

    return xReturn;
}
/*-----------------------------------------------------------*/

/**
 * @brief Check the buffer lengths of an ICMPv6 packet.
 * @param[in] uxBufferLength The total length of the packet.
 * @param[in] pxSet A struct describing this packet.
 * @return Non-zero in case of an error.
 */
BaseType_t prvChecksumICMPv6Checks( size_t uxBufferLength,
                                    struct xPacketSummary * pxSet )
{
    BaseType_t xReturn = 0;
    size_t xICMPLength;

    switch( pxSet->pxProtocolHeaders->xICMPHeaderIPv6.ucTypeOfMessage )
    {
        case ipICMP_PING_REQUEST_IPv6:
        case ipICMP_PING_REPLY_IPv6:
            xICMPLength = sizeof( ICMPEcho_IPv6_t );
            break;

        case ipICMP_ROUTER_SOLICITATION_IPv6:
            xICMPLength = sizeof( ICMPRouterSolicitation_IPv6_t );
            break;

        default:
            xICMPLength = ipSIZE_OF_ICMPv6_HEADER;
            break;
    }

    if( uxBufferLength < ( ipSIZE_OF_ETH_HEADER + pxSet->uxIPHeaderLength + xICMPLength ) )
    {
        pxSet->usChecksum = ipINVALID_LENGTH;
        xReturn = 10;
    }

    if( xReturn == 0 )
    {
        pxSet->uxProtocolHeaderLength = xICMPLength;
        #if ( ipconfigHAS_DEBUG_PRINTF != 0 )
        {
            pxSet->pcType = "ICMP_IPv6";
        }
        #endif /* ipconfigHAS_DEBUG_PRINTF != 0 */
    }

    return xReturn;
}
/*-----------------------------------------------------------*/

/**
 * @brief Get total length of all extension headers in IPv6 packet.
 *
 * @param[in] pucEthernetBuffer The buffer containing the packet.
 * @param[in] uxBufferLength The number of bytes to be sent or received.
 * @param[out] pucProtocol The L4 protocol, such as TCP/UDP/ICMPv6.
 *
 * @return The total length of all extension headers, or whole buffer length when error detected.
 */
size_t usGetExtensionHeaderLength( const uint8_t * pucEthernetBuffer,
                                   size_t uxBufferLength,
                                   uint8_t * pucProtocol )
{
    uint8_t ucCurrentHeader;
    const IPPacket_IPv6_t * pxIPPacket_IPv6;
    uint8_t ucNextHeader = 0U;
    size_t uxIndex = ipSIZE_OF_ETH_HEADER + ipSIZE_OF_IPv6_HEADER;
    size_t uxHopSize = 0U;
    BaseType_t xCurrentOrder = 0;
    BaseType_t xNextOrder = 0;
    size_t uxReturn = uxBufferLength;

    if( ( pucEthernetBuffer != NULL ) && ( pucProtocol != NULL ) )
    {
        /* MISRA Ref 11.3.1 [Misaligned access] */
        /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
        /* coverity[misra_c_2012_rule_11_3_violation] */
        pxIPPacket_IPv6 = ( ( const IPPacket_IPv6_t * ) pucEthernetBuffer );
        ucCurrentHeader = pxIPPacket_IPv6->xIPHeader.ucNextHeader;

        /* Check if packet has extension header. */
        if( xGetExtensionOrder( ucCurrentHeader, 0U ) > 0 )
        {
            while( ( uxIndex + 8U ) < uxBufferLength )
            {
                ucNextHeader = pucEthernetBuffer[ uxIndex ];

                xCurrentOrder = xGetExtensionOrder( ucCurrentHeader, ucNextHeader );

                /* To avoid compile warning if debug print is disabled. */
                ( void ) xCurrentOrder;

                /* Read the length expressed in number of octets. */
                uxHopSize = ( size_t ) pucEthernetBuffer[ uxIndex + 1U ];
                /* And multiply by 8 and add the minimum size of 8. */
                uxHopSize = ( uxHopSize * 8U ) + 8U;

                if( ( uxIndex + uxHopSize ) >= uxBufferLength )
                {
                    FreeRTOS_debug_printf( ( "The length %u + %u of extension header is larger than buffer size %u \n", ( unsigned ) uxIndex, ( unsigned ) uxHopSize, ( unsigned ) uxBufferLength ) );
                    break;
                }

                uxIndex = uxIndex + uxHopSize;

                if( ( ucNextHeader == ipPROTOCOL_TCP ) ||
                    ( ucNextHeader == ipPROTOCOL_UDP ) ||
                    ( ucNextHeader == ipPROTOCOL_ICMP_IPv6 ) )
                {
                    FreeRTOS_debug_printf( ( "Stop at header %u\n", ucNextHeader ) );

                    uxReturn = uxIndex - ( ipSIZE_OF_ETH_HEADER + ipSIZE_OF_IPv6_HEADER );
                    *pucProtocol = ucNextHeader;
                    break;
                }

                xNextOrder = xGetExtensionOrder( ucNextHeader, pucEthernetBuffer[ uxIndex ] );

                FreeRTOS_debug_printf( ( "Going from header %2u (%d) to %2u (%d)\n",
                                         ucCurrentHeader,
                                         ( int ) xCurrentOrder,
                                         ucNextHeader,
                                         ( int ) xNextOrder ) );

                /*
                 * IPv6 nodes must accept and attempt to process extension headers in
                 * any order and occurring any number of times in the same packet,
                 * except for the Hop-by-Hop Options header which is restricted to
                 * appear immediately after an IPv6 header only. Outlined
                 * by RFC 2460 section 4.1  Extension Header Order.
                 */
                if( xNextOrder == 1 ) /* ipIPv6_EXT_HEADER_HOP_BY_HOP */
                {
                    FreeRTOS_printf( ( "Wrong order. Hop-by-Hop Options header restricted to appear immediately after an IPv6 header\n" ) );
                    break;
                }
                else if( xNextOrder < 0 )
                {
                    FreeRTOS_printf( ( "Invalid extension header detected\n" ) );
                    break;
                }
                else
                {
                    /* Do nothing, coverity happy. */
                }

                ucCurrentHeader = ucNextHeader;
            }
        }
        else
        {
            /* No extension headers. */
            *pucProtocol = ucCurrentHeader;
            uxReturn = 0;
        }
    }

    return uxReturn;
}
/*-----------------------------------------------------------*/

/**
 * @brief Every IPv6 end-point has a solicited node multicast address and a corresponding
 * multicast MAC address. The IPv6 solicited node address also has an MLD reports associated
 * with it. This function manages both the MAC address and the MLD report associated with
 * the end-point's solicited-node address. On network UP, this function registers the MAC address
 * with the network driver's filter and creates and MLD report for the UPv6 multicast address.
 * On network DOWN, the function unregisters the MAC address and removes the MLD report.
 * This is a "convenience" function that keeps all these tasks under one roof for easier maintenance.
 *
 * @param[in] pxEndPoint The end-point for which a network up/down event is being handled.
 * @param[in] xNetworkGoingUp pdTRUE when the network goes UP, pdFALSE when the network goes DOWN.
 */
void vManageSolicitedNodeAddress( const struct xNetworkEndPoint * pxEndPoint,
                                  BaseType_t xNetworkGoingUp )
{
    IPv6_Type_t xAddressType;
    MACAddress_t xMACAddress;

    configASSERT( pxEndPoint != NULL );
    configASSERT( pxEndPoint->pxNetworkInterface != NULL );

    /* do{}while(0) to allow for the use of break statements */
    do
    {
        /* During the very first network DOWN event, pxEndPoint->ipv6_settings does not yet hold the proper address and
         * therefore the calculated MAC address will be incorrect. Nothing bad will happen though, because the address
         * type check below will kick us out before the call to pfRemoveAllowedMAC(). Without the check below, the network
         * driver ends up being called once to register 33:33:FF:00:00:00 and that MAC never gets unregistered. */

        /* Solicited-node multicast addresses only apply to normal unicast non-loopback addresses. */
        xAddressType = xIPv6_GetIPType( &( pxEndPoint->ipv6_settings.xIPAddress ) );

        if( ( xAddressType != eIPv6_LinkLocal ) && ( xAddressType != eIPv6_SiteLocal ) && ( xAddressType != eIPv6_Global ) )
        {
            /* The address of this end-point is something other than a normal unicast address... Maybe it's the
             * loopback address or maybe this is an error scenario. In any case, there is no corresponding
             * solicited-node multicast address that we need to manage. Do nothing.*/
            break;
        }

        /* Calculate the multicast MAC that corresponds to this endpoint's IPv6 address. */
        xMACAddress.ucBytes[ 0 ] = ipMULTICAST_MAC_ADDRESS_IPv6_0;
        xMACAddress.ucBytes[ 1 ] = ipMULTICAST_MAC_ADDRESS_IPv6_0;
        xMACAddress.ucBytes[ 2 ] = 0xFFU;
        xMACAddress.ucBytes[ 3 ] = pxEndPoint->ipv6_settings.xIPAddress.ucBytes[ 13 ];
        xMACAddress.ucBytes[ 4 ] = pxEndPoint->ipv6_settings.xIPAddress.ucBytes[ 14 ];
        xMACAddress.ucBytes[ 5 ] = pxEndPoint->ipv6_settings.xIPAddress.ucBytes[ 15 ];

        /* Update the network driver filter */
        if( xNetworkGoingUp == pdTRUE )
        {
            if( pxEndPoint->pxNetworkInterface->pfAddAllowedMAC != NULL )
            {
                pxEndPoint->pxNetworkInterface->pfAddAllowedMAC( pxEndPoint->pxNetworkInterface, xMACAddress.ucBytes );
            }
        }
        else
        {
            if( pxEndPoint->pxNetworkInterface->pfRemoveAllowedMAC != NULL )
            {
                pxEndPoint->pxNetworkInterface->pfRemoveAllowedMAC( pxEndPoint->pxNetworkInterface, xMACAddress.ucBytes );
            }
        }
    } while( pdFALSE );
}
/*-----------------------------------------------------------*/

/* *INDENT-OFF* */
#endif /* ( ipconfigUSE_IPv6 != 0 ) */
/* *INDENT-ON* */
