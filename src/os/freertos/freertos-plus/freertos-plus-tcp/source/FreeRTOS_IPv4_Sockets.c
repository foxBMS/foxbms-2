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
 * @file FreeRTOS_IPv4_Sockets.c
 * @brief Implements the Sockets API based on Berkeley sockets for the FreeRTOS+TCP network stack.
 *        Sockets are used by the application processes to interact with the IP-task which in turn
 *        interacts with the hardware.
 */

/* Standard includes. */
#include <stdint.h>
#include <stdio.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* FreeRTOS+TCP includes. */
#include "FreeRTOS_UDP_IP.h"
#include "FreeRTOS_IP.h"
#include "FreeRTOS_IPv4_Sockets.h"


/** @brief The number of octets that make up an IP address. */
#define socketMAX_IP_ADDRESS_OCTETS    ( 4U )

/* Just make sure the contents doesn't get compiled if IPv4 is not enabled. */
/* *INDENT-OFF* */
    #if( ipconfigUSE_IPv4 != 0 )
/* *INDENT-ON* */

/**
 * @brief This function converts the character string pcSource into a network address
 *        structure, then copies the network address structure to pvDestination.
 *        pvDestination is written in network byte order.
 *
 * @param[in] pcSource The character string in holding the IP address.
 * @param[out] pvDestination The returned network address in 32-bit network-endian format.
 *
 * @return pdPASS if the translation was successful or else pdFAIL.
 */
BaseType_t FreeRTOS_inet_pton4( const char * pcSource,
                                void * pvDestination )
{
    const uint32_t ulDecimalBase = 10U;
    uint8_t ucOctet[ socketMAX_IP_ADDRESS_OCTETS ];
    uint32_t ulReturn = 0U, ulValue;
    UBaseType_t uxOctetNumber;
    BaseType_t xResult = pdPASS;
    const char * pcIPAddress = pcSource;
    const void * pvCopySource;

    ( void ) memset( pvDestination, 0, sizeof( ulReturn ) );

    /* Translate "192.168.2.100" to a 32-bit number, network-endian. */
    for( uxOctetNumber = 0U; uxOctetNumber < socketMAX_IP_ADDRESS_OCTETS; uxOctetNumber++ )
    {
        ulValue = 0U;

        if( pcIPAddress[ 0 ] == '0' )
        {
            /* Test for the sequence "0[0-9]", which would make it an octal representation. */
            if( ( pcIPAddress[ 1 ] >= '0' ) && ( pcIPAddress[ 1 ] <= '9' ) )
            {
                FreeRTOS_printf( ( "Octal representation of IP-addresses is not supported." ) );
                /* Don't support octal numbers. */
                xResult = pdFAIL;
                break;
            }
        }

        while( ( *pcIPAddress >= '0' ) && ( *pcIPAddress <= '9' ) )
        {
            BaseType_t xChar;

            /* Move previous read characters into the next decimal
             * position. */
            ulValue *= ulDecimalBase;

            /* Add the binary value of the ascii character. */
            xChar = ( BaseType_t ) pcIPAddress[ 0 ];
            xChar = xChar - ( BaseType_t ) '0';
            ulValue += ( uint32_t ) xChar;

            /* Move to next character in the string. */
            pcIPAddress++;
        }

        /* Check characters were read. */
        if( pcIPAddress == pcSource )
        {
            xResult = pdFAIL;
        }

        /* Check the value fits in an 8-bit number. */
        if( ulValue > 0xffU )
        {
            xResult = pdFAIL;
        }
        else
        {
            ucOctet[ uxOctetNumber ] = ( uint8_t ) ulValue;

            /* Check the next character is as expected. */
            if( uxOctetNumber < ( socketMAX_IP_ADDRESS_OCTETS - 1U ) )
            {
                if( *pcIPAddress != '.' )
                {
                    xResult = pdFAIL;
                }
                else
                {
                    /* Move past the dot. */
                    pcIPAddress++;
                }
            }
        }

        if( xResult == pdFAIL )
        {
            /* No point going on. */
            break;
        }
    }

    if( *pcIPAddress != ( char ) 0 )
    {
        /* Expected the end of the string. */
        xResult = pdFAIL;
    }

    if( uxOctetNumber != socketMAX_IP_ADDRESS_OCTETS )
    {
        /* Didn't read enough octets. */
        xResult = pdFAIL;
    }

    if( xResult == pdPASS )
    {
        /* lint: ucOctet has been set because xResult == pdPASS. */
        ulReturn = FreeRTOS_inet_addr_quick( ucOctet[ 0 ], ucOctet[ 1 ], ucOctet[ 2 ], ucOctet[ 3 ] );
    }
    else
    {
        ulReturn = 0U;
    }

    if( xResult == pdPASS )
    {
        pvCopySource = ( const void * ) &ulReturn;
        ( void ) memcpy( pvDestination, pvCopySource, sizeof( ulReturn ) );
    }

    return xResult;
}
/*-----------------------------------------------------------*/

/**
 * @brief Convert the 32-bit representation of the IP-address to the dotted decimal format.
 *
 * @param[in] pvSource The pointer to the 32-bit representation of the IP-address.
 * @param[out] pcDestination The pointer to a character array where the string of the
 *                           dotted decimal IP format.
 * @param[in] uxSize Size of the character array. This value makes sure that the code
 *                    doesn't write beyond it's bounds.
 *
 * @return The pointer to the string holding the dotted decimal format of the IP-address. If
 *         everything passes correctly, then the pointer being returned is the same as
 *         pcDestination, else a NULL is returned.
 */
const char * FreeRTOS_inet_ntop4( const void * pvSource,
                                  char * pcDestination,
                                  socklen_t uxSize )
{
    uint32_t ulIPAddress;
    void * pvCopyDest;
    const char * pcReturn;

    if( uxSize < 16U )
    {
        /* There must be space for "255.255.255.255". */
        pcReturn = NULL;
    }
    else
    {
        pvCopyDest = ( void * ) &ulIPAddress;
        ( void ) memcpy( pvCopyDest, pvSource, sizeof( ulIPAddress ) );
        ( void ) FreeRTOS_inet_ntoa( ulIPAddress, pcDestination );
        pcReturn = pcDestination;
    }

    return pcReturn;
}

/**
 * @brief Called by prvSendUDPPacket(), this function will UDP packet
 *        fields and IPv4 address for the packet to be send.
 * @param[in] pxNetworkBuffer  The packet to be sent.
 * @param[in] pxDestinationAddress The IPv4 socket address.
 * @return  Returns NULL, always.
 */
void * xSend_UDP_Update_IPv4( NetworkBufferDescriptor_t * pxNetworkBuffer,
                              const struct freertos_sockaddr * pxDestinationAddress )
{
    UDPPacket_t * pxUDPPacket;

    if( ( pxNetworkBuffer != NULL ) && ( pxDestinationAddress != NULL ) )
    {
        /* MISRA Ref 11.3.1 [Misaligned access] */
        /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
        /* coverity[misra_c_2012_rule_11_3_violation] */
        pxUDPPacket = ( ( UDPPacket_t * ) pxNetworkBuffer->pucEthernetBuffer );

        pxNetworkBuffer->xIPAddress.ulIP_IPv4 = pxDestinationAddress->sin_address.ulIP_IPv4;
        /* Map the UDP packet onto the start of the frame. */
        pxUDPPacket->xEthernetHeader.usFrameType = ipIPv4_FRAME_TYPE;
    }

    return NULL;
}

/**
 * @brief Called by FreeRTOS_recvfrom(), this function will update socket
 *        address with IPv4 address from the packet received.
 * @param[in] pxNetworkBuffer  The packet received.
 * @param[in] pxSourceAddress The IPv4 socket address.
 * @return  The Payload Offset.
 */
size_t xRecv_Update_IPv4( const NetworkBufferDescriptor_t * pxNetworkBuffer,
                          struct freertos_sockaddr * pxSourceAddress )
{
    size_t uxPayloadOffset = 0;

    if( ( pxNetworkBuffer != NULL ) && ( pxSourceAddress != NULL ) )
    {
        pxSourceAddress->sin_family = ( uint8_t ) FREERTOS_AF_INET;
        pxSourceAddress->sin_address.ulIP_IPv4 = pxNetworkBuffer->xIPAddress.ulIP_IPv4;
        pxSourceAddress->sin_port = pxNetworkBuffer->usPort;
    }

    uxPayloadOffset = ipUDP_PAYLOAD_OFFSET_IPv4;

    return uxPayloadOffset;
}

/*-----------------------------------------------------------*/

/* *INDENT-OFF* */
    #endif /* ipconfigUSE_IPv4 != 0 ) */
/* *INDENT-ON* */
