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
 * @file FreeRTOS_IPv4_Utils.c
 * @brief Implements the basic functionality for the FreeRTOS+TCP network stack functions for IPv4.
 */

/* Standard includes. */
#include <stdint.h>
#include <stdio.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"

/* FreeRTOS+TCP includes. */
#include "FreeRTOS_IP.h"

/* Just make sure the contents doesn't get compiled if IPv4 is not enabled. */
/* *INDENT-OFF* */
    #if( ipconfigUSE_IPv4 != 0 )
/* *INDENT-ON* */


/*-----------------------------------------------------------*/

/**
 * @brief Set multicast MAC address.
 *
 * @param[in] ulIPAddress IP address.
 * @param[out] pxMACAddress Pointer to MAC address.
 */
void vSetMultiCastIPv4MacAddress( uint32_t ulIPAddress,
                                  MACAddress_t * pxMACAddress )
{
    uint32_t ulIP = FreeRTOS_ntohl( ulIPAddress );

    pxMACAddress->ucBytes[ 0 ] = ( uint8_t ) ipMULTICAST_MAC_ADDRESS_IPv4_0;
    pxMACAddress->ucBytes[ 1 ] = ( uint8_t ) ipMULTICAST_MAC_ADDRESS_IPv4_1;
    pxMACAddress->ucBytes[ 2 ] = ( uint8_t ) ipMULTICAST_MAC_ADDRESS_IPv4_2;
    pxMACAddress->ucBytes[ 3 ] = ( uint8_t ) ( ( ulIP >> 16 ) & 0x7fU ); /* Use 7 bits. */
    pxMACAddress->ucBytes[ 4 ] = ( uint8_t ) ( ( ulIP >> 8 ) & 0xffU );  /* Use 8 bits. */
    pxMACAddress->ucBytes[ 5 ] = ( uint8_t ) ( ( ulIP ) & 0xffU );       /* Use 8 bits. */
}
/*-----------------------------------------------------------*/

/** @brief Do the first IPv4 length checks at the IP-header level.
 * @param[in] pucEthernetBuffer The buffer containing the packet.
 * @param[in] uxBufferLength The number of bytes to be sent or received.
 * @param[in] pxSet A struct describing this packet.
 *
 * @return Non-zero in case of an error.
 */
BaseType_t prvChecksumIPv4Checks( uint8_t * pucEthernetBuffer,
                                  size_t uxBufferLength,
                                  struct xPacketSummary * pxSet )
{
    BaseType_t xReturn = 0;
    uint8_t ucVersion;
    uint16_t usLength;

    pxSet->xIsIPv6 = pdFALSE;

    usLength = pxSet->pxIPPacket->xIPHeader.usLength;
    usLength = FreeRTOS_ntohs( usLength );

    /* IPv4 : the lower nibble in 'ucVersionHeaderLength' indicates the length
     * of the IP-header, expressed in number of 4-byte words. Usually 5 words.
     */
    ucVersion = pxSet->pxIPPacket->xIPHeader.ucVersionHeaderLength & ( uint8_t ) 0x0FU;
    pxSet->uxIPHeaderLength = ( size_t ) ucVersion;
    pxSet->uxIPHeaderLength *= 4U;

    if( usLength < pxSet->uxIPHeaderLength )
    {
        pxSet->usChecksum = ipINVALID_LENGTH;
        xReturn = 3;
    }

    /* Check for minimum packet size. */
    if( xReturn == 0 )
    {
        if( uxBufferLength < sizeof( IPPacket_t ) )
        {
            pxSet->usChecksum = ipINVALID_LENGTH;
            xReturn = 4;
        }
    }

    if( xReturn == 0 )
    {
        /* Check for minimum packet size. */
        if( uxBufferLength < ( ipSIZE_OF_ETH_HEADER + pxSet->uxIPHeaderLength ) )
        {
            /* The packet does not contain the full IP-headers so drop it. */
            pxSet->usChecksum = ipINVALID_LENGTH;
            xReturn = 5;
        }
    }

    if( xReturn == 0 )
    {
        size_t uxNeeded;
        /* xIPHeader.usLength is the total length, minus the Ethernet header. */
        pxSet->usPayloadLength = FreeRTOS_ntohs( pxSet->pxIPPacket->xIPHeader.usLength );

        uxNeeded = pxSet->usPayloadLength;
        uxNeeded += ipSIZE_OF_ETH_HEADER;

        if( uxBufferLength < uxNeeded )
        {
            /* The payload is longer than the packet appears to contain. */
            pxSet->usChecksum = ipINVALID_LENGTH;
            xReturn = 6;
        }
    }

    if( xReturn == 0 )
    {
        /* Identify the next protocol. */
        pxSet->ucProtocol = pxSet->pxIPPacket->xIPHeader.ucProtocol;
        /* MISRA Ref 11.3.1 [Misaligned access] */
        /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
        /* coverity[misra_c_2012_rule_11_3_violation] */
        pxSet->pxProtocolHeaders = ( ( ProtocolHeaders_t * ) &( pucEthernetBuffer[ pxSet->uxIPHeaderLength + ipSIZE_OF_ETH_HEADER ] ) );
        /* For IPv4, the number of bytes in IP-header + the protocol is indicated. */
        pxSet->usProtocolBytes = ( uint16_t ) ( pxSet->usPayloadLength - pxSet->uxIPHeaderLength );
    }

    return xReturn;
}
/*-----------------------------------------------------------*/

/* *INDENT-OFF* */
    #endif /* ipconfigUSE_IPv4 != 0 ) */
/* *INDENT-ON* */
