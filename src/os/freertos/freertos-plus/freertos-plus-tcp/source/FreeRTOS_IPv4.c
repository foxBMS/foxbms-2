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
 * @file FreeRTOS_IPv4.c
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
#include "FreeRTOS_IPv4.h"

/* IPv4 multi-cast addresses range from 224.0.0.0.0 to 240.0.0.0. */
#define ipFIRST_MULTI_CAST_IPv4    0xE0000000U          /**< Lower bound of the IPv4 multicast address. */
#define ipLAST_MULTI_CAST_IPv4     0xF0000000U          /**< Higher bound of the IPv4 multicast address. */

/* Just make sure the contents doesn't get compiled if IPv4 is not enabled. */
/* *INDENT-OFF* */
    #if( ipconfigUSE_IPv4 != 0 )
/* *INDENT-ON* */

#if ( ipconfigDRIVER_INCLUDED_RX_IP_CHECKSUM == 1 )
    /* Check IPv4 packet length. */
    static BaseType_t xCheckIPv4SizeFields( const void * const pvEthernetBuffer,
                                            size_t uxBufferLength );
#endif /* ( ipconfigDRIVER_INCLUDED_RX_IP_CHECKSUM == 1 ) */


#if ( ipconfigDRIVER_INCLUDED_RX_IP_CHECKSUM == 1 )

/**
 * @brief Check IPv4 packet length.
 *
 * @param[in] pvEthernetBuffer The Ethernet packet received.
 * @param[in] uxBufferLength The total number of bytes received.
 *
 * @return pdPASS when the length fields in the packet OK, pdFAIL when the packet
 *         should be dropped.
 */
    static BaseType_t xCheckIPv4SizeFields( const void * const pvEthernetBuffer,
                                            size_t uxBufferLength )
    {
        size_t uxLength;
        UBaseType_t uxIPHeaderLength;
        uint8_t ucProtocol;
        uint16_t usLength;
        uint16_t ucVersionHeaderLength;
        size_t uxMinimumLength;
        BaseType_t xResult = pdFAIL;

        /* Map the buffer onto a IP-Packet struct to easily access the
         * fields of the IP packet. */
        const IPPacket_t * const pxIPPacket = ( (  IPPacket_t *  ) pvEthernetBuffer );

        DEBUG_DECLARE_TRACE_VARIABLE( BaseType_t, xLocation, 0 );

        do
        {
            /* Check for minimum packet size: Ethernet header and an IP-header, 34 bytes */
            if( uxBufferLength < sizeof( IPPacket_t ) )
            {
                DEBUG_SET_TRACE_VARIABLE( xLocation, 1 );
                break;
            }

            ucVersionHeaderLength = pxIPPacket->xIPHeader.ucVersionHeaderLength;

            /* Test if the length of the IP-header is between 20 and 60 bytes,
             * and if the IP-version is 4. */
            if( ( ucVersionHeaderLength < ipIPV4_VERSION_HEADER_LENGTH_MIN ) ||
                ( ucVersionHeaderLength > ipIPV4_VERSION_HEADER_LENGTH_MAX ) )
            {
                DEBUG_SET_TRACE_VARIABLE( xLocation, 2 );
                break;
            }

            ucVersionHeaderLength = ( uint16_t ) ( ( ucVersionHeaderLength & ( uint8_t ) 0x0FU ) << 2U );
            uxIPHeaderLength = ( UBaseType_t ) ucVersionHeaderLength;

            /* Check if the complete IP-header is transferred. */
            if( uxBufferLength < ( ipSIZE_OF_ETH_HEADER + uxIPHeaderLength ) )
            {
                DEBUG_SET_TRACE_VARIABLE( xLocation, 3 );
                break;
            }

            /* Check if the complete IP-header plus protocol data have been transferred: */
            usLength = pxIPPacket->xIPHeader.usLength;
            usLength = FreeRTOS_ntohs( usLength );

            if( uxBufferLength < ( size_t ) ( ipSIZE_OF_ETH_HEADER + ( size_t ) usLength ) )
            {
                DEBUG_SET_TRACE_VARIABLE( xLocation, 4 );
                break;
            }

            /* Identify the next protocol. */
            ucProtocol = pxIPPacket->xIPHeader.ucProtocol;

            /* Switch on the Layer 3/4 protocol. */
            if( ucProtocol == ( uint8_t ) ipPROTOCOL_UDP )
            {
                /* Expect at least a complete UDP header. */
                uxMinimumLength = uxIPHeaderLength + ipSIZE_OF_ETH_HEADER + ipSIZE_OF_UDP_HEADER;
            }
            else if( ucProtocol == ( uint8_t ) ipPROTOCOL_TCP )
            {
                uxMinimumLength = uxIPHeaderLength + ipSIZE_OF_ETH_HEADER + ipSIZE_OF_TCP_HEADER;
            }
            else if( ( ucProtocol == ( uint8_t ) ipPROTOCOL_ICMP ) ||
                     ( ucProtocol == ( uint8_t ) ipPROTOCOL_IGMP ) )
            {
                uxMinimumLength = uxIPHeaderLength + ipSIZE_OF_ETH_HEADER + ipSIZE_OF_ICMPv4_HEADER;
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

            uxLength = ( size_t ) usLength;
            uxLength -= ( ( uint16_t ) uxIPHeaderLength ); /* normally, minus 20. */

            if( ( uxLength < ( ( size_t ) sizeof( UDPHeader_t ) ) ) ||
                ( uxLength > ( ( size_t ) ipconfigNETWORK_MTU - ( size_t ) uxIPHeaderLength ) ) )
            {
                /* For incoming packets, the length is out of bound: either
                 * too short or too long. For outgoing packets, there is a
                 * serious problem with the format/length. */
                DEBUG_SET_TRACE_VARIABLE( xLocation, 7 );
                break;
            }

            xResult = pdPASS;
        } while( ipFALSE_BOOL );

        if( xResult != pdPASS )
        {
            /* NOP if ipconfigHAS_PRINTF != 1 */
            FreeRTOS_printf( ( "xCheckIPv4SizeFields: location %ld\n", xLocation ) );
        }

        return xResult;
    }

#endif /* ( ipconfigDRIVER_INCLUDED_RX_IP_CHECKSUM == 1 ) */
/*-----------------------------------------------------------*/

/**
 * @brief Is the IP address an IPv4 multicast address.
 *
 * @param[in] ulIPAddress The IP address being checked.
 *
 * @return pdTRUE if the IP address is a multicast address or else, pdFALSE.
 */
BaseType_t xIsIPv4Multicast( uint32_t ulIPAddress )
{
    BaseType_t xReturn;
    uint32_t ulIP = FreeRTOS_ntohl( ulIPAddress );

    if( ( ulIP >= ipFIRST_MULTI_CAST_IPv4 ) && ( ulIP < ipLAST_MULTI_CAST_IPv4 ) )
    {
        xReturn = pdTRUE;
    }
    else
    {
        xReturn = pdFALSE;
    }

    return xReturn;
}
/*-----------------------------------------------------------*/

/**
 * @brief Check if the packet is an illegal loopback packet.
 *
 * @param[in] pxIPHeader The IP-header being checked.
 *
 * @return Returns pdTRUE if the packet should be stopped, because either the source
 *         or the target address is a loopback address.
 */
BaseType_t xBadIPv4Loopback( const IPHeader_t * const pxIPHeader )
{
    BaseType_t xReturn = pdFALSE;
    BaseType_t x1 = ( xIsIPv4Loopback( pxIPHeader->ulDestinationIPAddress ) != 0 ) ? pdTRUE : pdFALSE;
    BaseType_t x2 = ( xIsIPv4Loopback( pxIPHeader->ulSourceIPAddress ) != 0 ) ? pdTRUE : pdFALSE;

    if( x1 != x2 )
    {
        /* Either the source or the destination address is an IPv4 loopback address. */
        xReturn = pdTRUE;
    }

    return xReturn;
}
/*-----------------------------------------------------------*/

/**
 * @brief Is the IP address an IPv4 loopback address.
 *
 * @param[in] ulAddress The IP address being checked.
 *
 * @return pdTRUE if the IP address is a loopback address or else, pdFALSE.
 */
BaseType_t xIsIPv4Loopback( uint32_t ulAddress )
{
    BaseType_t xReturn = pdFALSE;
    uint32_t ulIP = FreeRTOS_ntohl( ulAddress );

    if( ( ulIP >= ipFIRST_LOOPBACK_IPv4 ) &&
        ( ulIP < ipLAST_LOOPBACK_IPv4 ) )
    {
        xReturn = pdTRUE;
    }

    return xReturn;
}
/*-----------------------------------------------------------*/

/**
 * @brief Check whether this IPv4 packet is to be allowed or to be dropped.
 *
 * @param[in] pxIPPacket The IP packet under consideration.
 * @param[in] pxNetworkBuffer The whole network buffer.
 * @param[in] uxHeaderLength The length of the header.
 *
 * @return Whether the packet should be processed or dropped.
 */
enum eFrameProcessingResult prvAllowIPPacketIPv4( const struct xIP_PACKET * const pxIPPacket,
                                                  const struct xNETWORK_BUFFER * const pxNetworkBuffer,
                                                  UBaseType_t uxHeaderLength )
{
    eFrameProcessingResult_t eReturn = eProcessBuffer;

    #if ( ( ipconfigETHERNET_DRIVER_FILTERS_PACKETS == 0 ) || ( ipconfigDRIVER_INCLUDED_RX_IP_CHECKSUM == 0 ) )
        const IPHeader_t * pxIPHeader = &( pxIPPacket->xIPHeader );
    #else

        /* or else, the parameter won't be used and the function will be optimised
         * away */
        ( void ) pxIPPacket;
    #endif

    #if ( ipconfigETHERNET_DRIVER_FILTERS_PACKETS == 0 )
    {
        /* In systems with a very small amount of RAM, it might be advantageous
         * to have incoming messages checked earlier, by the network card driver.
         * This method may decrease the usage of sparse network buffers. */
        uint32_t ulDestinationIPAddress = pxIPHeader->ulDestinationIPAddress;
        uint32_t ulSourceIPAddress = pxIPHeader->ulSourceIPAddress;

        /* Ensure that the incoming packet is not fragmented because the stack
         * doesn't not support IP fragmentation. All but the last fragment coming in will have their
         * "more fragments" flag set and the last fragment will have a non-zero offset.
         * We need to drop the packet in either of those cases. */
        if( ( ( pxIPHeader->usFragmentOffset & ipFRAGMENT_OFFSET_BIT_MASK ) != 0U ) || ( ( pxIPHeader->usFragmentOffset & ipFRAGMENT_FLAGS_MORE_FRAGMENTS ) != 0U ) )
        {
            /* Can not handle, fragmented packet. */
            eReturn = eReleaseBuffer;
        }

        /* Test if the length of the IP-header is between 20 and 60 bytes,
         * and if the IP-version is 4. */
        else if( ( pxIPHeader->ucVersionHeaderLength < ipIPV4_VERSION_HEADER_LENGTH_MIN ) ||
                 ( pxIPHeader->ucVersionHeaderLength > ipIPV4_VERSION_HEADER_LENGTH_MAX ) )
        {
            /* Can not handle, unknown or invalid header version. */
            eReturn = eReleaseBuffer;
        }
        else if( ( xIsIPv4Loopback( ulDestinationIPAddress ) == pdTRUE ) ||
                 ( xIsIPv4Loopback( ulSourceIPAddress ) == pdTRUE ) )
        {
            /* source OR destination is a loopback address. Make sure they BOTH are. */
            if( xBadIPv4Loopback( &( pxIPPacket->xIPHeader ) ) == pdTRUE )
            {
                /* The local loopback addresses must never appear outside a host. See RFC 1122
                 * section 3.2.1.3. */
                eReturn = eReleaseBuffer;
            }
        }
        else if(
            ( FreeRTOS_FindEndPointOnIP_IPv4( ulDestinationIPAddress, 4 ) == NULL ) &&
            /* Is it an IPv4 broadcast address x.x.x.255 ? */
            ( ( FreeRTOS_ntohl( ulDestinationIPAddress ) & 0xffU ) != 0xffU ) &&
            ( xIsIPv4Multicast( ulDestinationIPAddress ) == pdFALSE ) &&
            /* Or (during DHCP negotiation) we have no IP-address yet? */
            ( FreeRTOS_IsNetworkUp() != pdFALSE ) )
        {
            /* Packet is not for this node, release it */
            eReturn = eReleaseBuffer;
        }
        /* Is the source address correct? */
        else if( ( FreeRTOS_ntohl( ulSourceIPAddress ) & 0xffU ) == 0xffU )
        {
            /* The source address cannot be broadcast address. Replying to this
             * packet may cause network storms. Drop the packet. */
            eReturn = eReleaseBuffer;
        }
        else if( ( memcmp( xBroadcastMACAddress.ucBytes,
                           pxIPPacket->xEthernetHeader.xDestinationAddress.ucBytes,
                           sizeof( MACAddress_t ) ) == 0 ) &&
                 ( ( FreeRTOS_ntohl( ulDestinationIPAddress ) & 0xffU ) != 0xffU ) )
        {
            /* Ethernet address is a broadcast address, but the IP address is not a
             * broadcast address. */
            eReturn = eReleaseBuffer;
        }
        else if( memcmp( xBroadcastMACAddress.ucBytes,
                         pxIPPacket->xEthernetHeader.xSourceAddress.ucBytes,
                         sizeof( MACAddress_t ) ) == 0 )
        {
            /* Ethernet source is a broadcast address. Drop the packet. */
            eReturn = eReleaseBuffer;
        }
        else if( xIsIPv4Multicast( ulSourceIPAddress ) == pdTRUE )
        {
            /* Source is a multicast IP address. Drop the packet in conformity with RFC 1112 section 7.2. */
            eReturn = eReleaseBuffer;
        }
        else
        {
            /* Packet is not fragmented, destination is this device, source IP and MAC
             * addresses are correct. */
        }
    }
    #endif /* ipconfigETHERNET_DRIVER_FILTERS_PACKETS */

    #if ( ipconfigDRIVER_INCLUDED_RX_IP_CHECKSUM == 0 )
    {
        /* Some drivers of NIC's with checksum-offloading will enable the above
         * define, so that the checksum won't be checked again here */
        if( eReturn == eProcessBuffer )
        {
            const NetworkEndPoint_t * pxEndPoint = FreeRTOS_FindEndPointOnMAC( &( pxIPPacket->xEthernetHeader.xSourceAddress ), NULL );

            /* Do not check the checksum of loop-back messages. */
            if( pxEndPoint == NULL )
            {
                /* Is the IP header checksum correct?
                 *
                 * NOTE: When the checksum of IP header is calculated while not omitting
                 * the checksum field, the resulting value of the checksum always is 0xffff
                 * which is denoted by ipCORRECT_CRC. See this wiki for more information:
                 * https://en.wikipedia.org/wiki/IPv4_header_checksum#Verifying_the_IPv4_header_checksum
                 * and this RFC: https://tools.ietf.org/html/rfc1624#page-4
                 */
                if( usGenerateChecksum( 0U, ( const uint8_t * ) &( pxIPHeader->ucVersionHeaderLength ), ( size_t ) uxHeaderLength ) != ipCORRECT_CRC )
                {
                    /* Check sum in IP-header not correct. */
                    eReturn = eReleaseBuffer;
                }
                /* Is the upper-layer checksum (TCP/UDP/ICMP) correct? */
                else if( usGenerateProtocolChecksum( ( uint8_t * ) ( pxNetworkBuffer->pucEthernetBuffer ), pxNetworkBuffer->xDataLength, pdFALSE ) != ipCORRECT_CRC )
                {
                    /* Protocol checksum not accepted. */
                    eReturn = eReleaseBuffer;
                }
                else
                {
                    /* The checksum of the received packet is OK. */
                }
            }
        }
    }
    #else /* if ( ipconfigDRIVER_INCLUDED_RX_IP_CHECKSUM == 0 ) */
    {
        if( eReturn == eProcessBuffer )
        {
            if( xCheckIPv4SizeFields( pxNetworkBuffer->pucEthernetBuffer, pxNetworkBuffer->xDataLength ) != pdPASS )
            {
                /* Some of the length checks were not successful. */
                eReturn = eReleaseBuffer;
            }
        }

        #if ( ipconfigUDP_PASS_ZERO_CHECKSUM_PACKETS == 0 )
        {
            /* Check if this is a UDP packet without a checksum. */
            if( eReturn == eProcessBuffer )
            {
                uint8_t ucProtocol;
                const ProtocolHeaders_t * pxProtocolHeaders;

                /* ipconfigUDP_PASS_ZERO_CHECKSUM_PACKETS is defined as 0,
                 * and so UDP packets carrying a protocol checksum of 0, will
                 * be dropped. */
                ucProtocol = pxIPPacket->xIPHeader.ucProtocol;
                /* MISRA Ref 11.3.1 [Misaligned access] */
                /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
                /* coverity[misra_c_2012_rule_11_3_violation] */
                pxProtocolHeaders = ( ( ProtocolHeaders_t * ) &( pxNetworkBuffer->pucEthernetBuffer[ ipSIZE_OF_ETH_HEADER + ( size_t ) ipSIZE_OF_IPv4_HEADER ] ) );

                /* Identify the next protocol. */
                if( ucProtocol == ( uint8_t ) ipPROTOCOL_UDP )
                {
                    if( pxProtocolHeaders->xUDPHeader.usChecksum == ( uint16_t ) 0U )
                    {
                        #if ( ipconfigHAS_PRINTF != 0 )
                        {
                            static BaseType_t xCount = 0;

                            /* Exclude this from branch coverage as this is only used for debugging. */
                            if( xCount < 5 ) /* LCOV_EXCL_BR_LINE */
                            {
                                FreeRTOS_printf( ( "prvAllowIPPacket: UDP packet from %xip without CRC dropped\n",
                                                   FreeRTOS_ntohl( pxIPPacket->xIPHeader.ulSourceIPAddress ) ) );
                                xCount++;
                            }
                        }
                        #endif /* ( ipconfigHAS_PRINTF != 0 ) */

                        /* Protocol checksum not accepted. */
                        eReturn = eReleaseBuffer;
                    }
                }
            }
        }
        #endif /* ( ipconfigUDP_PASS_ZERO_CHECKSUM_PACKETS == 0 ) */

        /* to avoid warning unused parameters */
        ( void ) pxNetworkBuffer;
        ( void ) uxHeaderLength;
    }
    #endif /* ipconfigDRIVER_INCLUDED_RX_IP_CHECKSUM == 0 */

    return eReturn;
}

/*-----------------------------------------------------------*/


/** @brief Check if the IP-header is carrying options.
 * @param[in] pxNetworkBuffer the network buffer that contains the packet.
 *
 * @return Either 'eProcessBuffer' or 'eReleaseBuffer'
 */
enum eFrameProcessingResult prvCheckIP4HeaderOptions( struct xNETWORK_BUFFER * const pxNetworkBuffer )
{
    eFrameProcessingResult_t eReturn = eProcessBuffer;

    /* This function is only called for IPv4 packets, with an IP-header
     * which is larger than 20 bytes.  The extra space is used for IP-options.
     * The options will either be removed, or the packet shall be dropped,
     * depending on a user define. */

    #if ( ipconfigIP_PASS_PACKETS_WITH_IP_OPTIONS != 0 )
    {
        /* MISRA Ref 11.3.1 [Misaligned access] */
        /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
        /* coverity[misra_c_2012_rule_11_3_violation] */
        IPHeader_t * pxIPHeader = ( ( IPHeader_t * ) &( pxNetworkBuffer->pucEthernetBuffer[ ipSIZE_OF_ETH_HEADER ] ) );

        /* All structs of headers expect a IP header size of 20 bytes
         * IP header options were included, we'll ignore them and cut them out. */
        size_t uxLength = ( size_t ) pxIPHeader->ucVersionHeaderLength;

        /* Check if the IP headers are acceptable and if it has our destination.
         * The lowest four bits of 'ucVersionHeaderLength' indicate the IP-header
         * length in multiples of 4. */
        size_t uxHeaderLength = ( size_t ) ( ( uxLength & 0x0FU ) << 2 );

        /* Number of bytes contained in IPv4 header options. */
        const size_t optlen = ( ( size_t ) uxHeaderLength ) - ipSIZE_OF_IPv4_HEADER;
        /* From: the previous start of UDP/ICMP/TCP data. */
        const uint8_t * pucSource = ( const uint8_t * ) &( pxNetworkBuffer->pucEthernetBuffer[ sizeof( EthernetHeader_t ) + uxHeaderLength ] );
        /* To: the usual start of UDP/ICMP/TCP data at offset 20 (decimal ) from IP header. */
        uint8_t * pucTarget = ( uint8_t * ) &( pxNetworkBuffer->pucEthernetBuffer[ sizeof( EthernetHeader_t ) + ipSIZE_OF_IPv4_HEADER ] );
        /* How many: total length minus the options and the lower headers. */
        const size_t xMoveLen = pxNetworkBuffer->xDataLength - ( optlen + ipSIZE_OF_IPv4_HEADER + ipSIZE_OF_ETH_HEADER );

        ( void ) memmove( pucTarget, pucSource, xMoveLen );
        pxNetworkBuffer->xDataLength -= optlen;
        /* Update the total length of the IP packet after removing options. */
        pxIPHeader->usLength = FreeRTOS_htons( FreeRTOS_ntohs( pxIPHeader->usLength ) - optlen );

        /* Rewrite the Version/IHL byte to indicate that this packet has no IP options. */
        pxIPHeader->ucVersionHeaderLength = ( uint8_t ) ( ( pxIPHeader->ucVersionHeaderLength & 0xF0U ) | /* High nibble is the version. */
                                                          ( ( ipSIZE_OF_IPv4_HEADER >> 2 ) & 0x0FU ) );
    }
    #else /* if ( ipconfigIP_PASS_PACKETS_WITH_IP_OPTIONS != 0 ) */
    {
        /* 'ipconfigIP_PASS_PACKETS_WITH_IP_OPTIONS' is not set, so packets carrying
         * IP-options will be dropped. */
        eReturn = eReleaseBuffer;
    }
    #endif /* if ( ipconfigIP_PASS_PACKETS_WITH_IP_OPTIONS != 0 ) */

    return eReturn;
}
/*-----------------------------------------------------------*/

/* *INDENT-OFF* */
    #endif /* ipconfigUSE_IPv4 != 0 ) */
/* *INDENT-ON* */
