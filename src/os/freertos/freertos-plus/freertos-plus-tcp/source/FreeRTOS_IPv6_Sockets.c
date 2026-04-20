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
 * @file FreeRTOS_IPv6_Sockets.c
 * @brief Implements the Sockets API based on Berkeley sockets for the FreeRTOS+TCP network stack.
 *        Sockets are used by the application processes to interact with the IP-task which in turn
 *        interacts with the hardware.
 */

/* Standard includes. */
#include <stdint.h>
#include <stdio.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"

/* FreeRTOS+TCP includes. */
#include "FreeRTOS_IP.h"
#include "FreeRTOS_IPv6_Sockets.h"

/* *INDENT-OFF* */
#if( ipconfigUSE_IPv6 != 0 )
/* *INDENT-ON* */

#if ( ipconfigUSE_TCP == 1 )

/**
 * @brief Called by pxTCPSocketLookup(), this function will check if a socket
 *        is connected to a remote IP-address. It will be called from a loop
 *        iterating through all sockets.
 * @param[in] pxSocket The socket to be inspected.
 * @param[in] pxAddress The IPv4/IPv6 address.
 * @return The socket in case it is connected to the remote IP-address.
 */
    FreeRTOS_Socket_t * pxTCPSocketLookup_IPv6( FreeRTOS_Socket_t * pxSocket,
                                                const IPv46_Address_t * pxAddress )
    {
        FreeRTOS_Socket_t * pxResult = NULL;

        if( ( pxSocket != NULL ) && ( pxAddress != NULL ) )
        {
            if( pxSocket->bits.bIsIPv6 != pdFALSE_UNSIGNED )
            {
                if( pxAddress->xIs_IPv6 != pdFALSE )
                {
                    if( memcmp( pxSocket->u.xTCP.xRemoteIP.xIP_IPv6.ucBytes, pxAddress->xIPAddress.xIP_IPv6.ucBytes, ipSIZE_OF_IPv6_ADDRESS ) == 0 )
                    {
                        /* For sockets not in listening mode, find a match with
                         * uxLocalPort, ulRemoteIP AND uxRemotePort. */
                        pxResult = pxSocket;
                    }
                }
            }
            else
            {
                if( pxAddress->xIs_IPv6 == pdFALSE )
                {
                    if( pxSocket->u.xTCP.xRemoteIP.ulIP_IPv4 == pxAddress->xIPAddress.ulIP_IPv4 )
                    {
                        /* For sockets not in listening mode, find a match with
                         * uxLocalPort, ulRemoteIP AND uxRemotePort. */
                        pxResult = pxSocket;
                    }
                }
            }
        }

        return pxResult;
    }

#endif /* if ( ( ipconfigUSE_TCP == 1 ) */

/**
 * @brief Called by prvSendUDPPacket(), this function will UDP packet
 *        fields and IPv6 address for the packet to be send.
 * @param[in] pxNetworkBuffer  The packet to be sent.
 * @param[in] pxDestinationAddress The IPv4 socket address.
 * @return  Returns NULL, always.
 */
void * xSend_UDP_Update_IPv6( NetworkBufferDescriptor_t * pxNetworkBuffer,
                              const struct freertos_sockaddr * pxDestinationAddress )
{
    /* MISRA Ref 11.3.1 [Misaligned access] */
    /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
    /* coverity[misra_c_2012_rule_11_3_violation] */
    UDPPacket_IPv6_t * pxUDPPacket_IPv6 = ( ( UDPPacket_IPv6_t * ) pxNetworkBuffer->pucEthernetBuffer );

    pxNetworkBuffer->xIPAddress.ulIP_IPv4 = 0U;

    configASSERT( pxDestinationAddress != NULL );
    ( void ) memcpy( pxUDPPacket_IPv6->xIPHeader.xDestinationAddress.ucBytes, pxDestinationAddress->sin_address.xIP_IPv6.ucBytes, ipSIZE_OF_IPv6_ADDRESS );
    ( void ) memcpy( pxNetworkBuffer->xIPAddress.xIP_IPv6.ucBytes, pxDestinationAddress->sin_address.xIP_IPv6.ucBytes, ipSIZE_OF_IPv6_ADDRESS );
    pxUDPPacket_IPv6->xEthernetHeader.usFrameType = ipIPv6_FRAME_TYPE;

    return NULL;
}

/**
 * @brief Called by FreeRTOS_recvfrom(), this function will update socket
 *        address with IPv6 address from the packet received.
 * @param[in] pxNetworkBuffer  The packet received.
 * @param[in] pxSourceAddress The IPv4 socket address.
 * @return The Payload Offset.
 */
size_t xRecv_Update_IPv6( const NetworkBufferDescriptor_t * pxNetworkBuffer,
                          struct freertos_sockaddr * pxSourceAddress )
{
    /* MISRA Ref 11.3.1 [Misaligned access] */
    /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
    /* coverity[misra_c_2012_rule_11_3_violation] */
    const UDPPacket_IPv6_t * pxUDPPacketV6 = ( ( const UDPPacket_IPv6_t * ) pxNetworkBuffer->pucEthernetBuffer );
    size_t uxPayloadOffset = 0;

    if( pxUDPPacketV6->xEthernetHeader.usFrameType == ipIPv6_FRAME_TYPE )
    {
        if( pxSourceAddress != NULL )
        {
            ( void ) memcpy( ( void * ) pxSourceAddress->sin_address.xIP_IPv6.ucBytes,
                             ( const void * ) pxUDPPacketV6->xIPHeader.xSourceAddress.ucBytes,
                             ipSIZE_OF_IPv6_ADDRESS );
            pxSourceAddress->sin_family = ( uint8_t ) FREERTOS_AF_INET6;
            pxSourceAddress->sin_port = pxNetworkBuffer->usPort;
        }

        uxPayloadOffset = ipUDP_PAYLOAD_OFFSET_IPv6;
    }

    return uxPayloadOffset;
}


/**
 * @brief Converts a 4 bit (nibble) value to a readable hex character, e.g. 14 becomes 'e'.
 * @param usValue  The value to be converted, must be between 0 and 15.
 * @return The character, between '0' and '9', or between 'a' and 'f'.
 */
char cHexToChar( uint16_t usValue )
{
    char cReturn = '0';

    if( usValue <= 9U )
    {
        cReturn = ( char ) ( cReturn + usValue );
    }
    else if( usValue <= 15U )
    {
        cReturn = 'a';
        cReturn = ( char ) ( cReturn + ( usValue - ( uint16_t ) 10 ) );
    }
    else
    {
        /* The value passed to 'usValue' has been and-ed with 0x0f,
         * so this else clause should never be reached. */
        configASSERT( 0 == 1 );
    }

    return cReturn;
}

/*-----------------------------------------------------------*/

/**
 * @brief Convert a short numeric value to a hex string of at most 4 characters.
 *        The resulting string is **not** null-terminated. The resulting string
 *        will not have leading zero's, except when 'usValue' equals zero.
 * @param[in] pcBuffer  The buffer to which the string is written.
 * @param[in] uxBufferSize  The size of the buffer pointed to by 'pcBuffer'.
 * @param[in] usValue  The 16-bit value to be converted.
 * @return The number of bytes written to 'pcBuffer'.
 */
socklen_t uxHexPrintShort( char * pcBuffer,
                           size_t uxBufferSize,
                           uint16_t usValue )
{
    const size_t uxNibbleCount = 4U;
    size_t uxNibble;
    socklen_t uxIndex = 0U;
    uint16_t usShifter = usValue;
    BaseType_t xHadNonZero = pdFALSE;

    for( uxNibble = 0; uxNibble < uxNibbleCount; uxNibble++ )
    {
        uint16_t usNibble = ( usShifter >> 12 ) & 0x0FU;

        if( usNibble != 0U )
        {
            xHadNonZero = pdTRUE;
        }

        if( ( xHadNonZero != pdFALSE ) || ( uxNibble == ( uxNibbleCount - 1U ) ) )
        {
            if( uxIndex >= ( uxBufferSize - 1U ) )
            {
                break;
            }

            pcBuffer[ uxIndex ] = cHexToChar( usNibble );
            uxIndex++;
        }

        usShifter = ( uint16_t ) ( usShifter << 4 );
    }

    return uxIndex;
}
/*-----------------------------------------------------------*/

/**
 * @brief Scan the binary IPv6 address and find the longest train of consecutive zero's.
 *        The result of this search will be stored in 'xZeroStart' and 'xZeroLength'.
 * @param pxSet the set of parameters as used by FreeRTOS_inet_ntop6().
 */
void prv_ntop6_search_zeros( struct sNTOP6_Set * pxSet )
{
    BaseType_t xIndex = 0;            /* The index in the IPv6 address: 0..7. */
    BaseType_t xCurStart = 0;         /* The position of the first zero found so far. */
    BaseType_t xCurLength = 0;        /* The number of zero's seen so far. */
    const BaseType_t xShortCount = 8; /* An IPv6 address consists of 8 shorts. */

    /* Default: when xZeroStart is negative, it won't match with any xIndex. */
    pxSet->xZeroStart = -1;

    /* Look for the longest train of zero's 0:0:0:... */
    for( ; xIndex < xShortCount; xIndex++ )
    {
        uint16_t usValue = pxSet->pusAddress[ xIndex ];

        if( usValue == 0U )
        {
            if( xCurLength == 0 )
            {
                /* Remember the position of the first zero. */
                xCurStart = xIndex;
            }

            /* Count consecutive zeros. */
            xCurLength++;
        }

        if( ( usValue != 0U ) || ( xIndex == ( xShortCount - 1 ) ) )
        {
            /* Has a longer train of zero's been found? */
            if( ( xCurLength > 1 ) && ( pxSet->xZeroLength < xCurLength ) )
            {
                /* Remember the number of consecutive zeros. */
                pxSet->xZeroLength = xCurLength;
                /* Remember the index of the first zero found. */
                pxSet->xZeroStart = xCurStart;
            }

            /* Reset the counter of consecutive zeros. */
            xCurLength = 0;
        }
    }
}
/*-----------------------------------------------------------*/

/**
 * @brief The location is now at the longest train of zero's. Two colons have to
 *        be printed without a numeric value, e.g. "ff02::1".
 * @param pcDestination the output buffer where the colons will be printed.
 * @param uxSize the remaining length of the output buffer.
 * @param pxSet the set of parameters as used by FreeRTOS_inet_ntop6().
 * @return pdPASS in case the output buffer is big enough to contain the colons.
 * @note uxSize must be at least 2, enough to print "::". The string will get
 *       null-terminated later on.
 */
static BaseType_t prv_ntop6_write_zeros( char * pcDestination,
                                         size_t uxSize,
                                         struct sNTOP6_Set * pxSet )
{
    BaseType_t xReturn = pdPASS;
    const BaseType_t xShortCount = 8; /* An IPv6 address consists of 8 shorts. */

    if( pxSet->uxTargetIndex <= ( uxSize - 1U ) )
    {
        pcDestination[ pxSet->uxTargetIndex ] = ':';
        pxSet->uxTargetIndex++;

        if( ( pxSet->xIndex + pxSet->xZeroLength ) == xShortCount )
        {
            /* Reached the last index, write a second ":". */
            if( pxSet->uxTargetIndex <= ( uxSize - 1U ) )
            {
                pcDestination[ pxSet->uxTargetIndex ] = ':';
                pxSet->uxTargetIndex++;
            }
            else
            {
                /* Can not write the second colon. */
                xReturn = pdFAIL;
            }
        }
        else
        {
            /* Otherwise the function prv_ntop6_write_short() will places the second colon. */
        }
    }
    else
    {
        /* Can not write the first colon. */
        xReturn = pdFAIL;
    }

    return xReturn;
}
/*-----------------------------------------------------------*/

/**
 * @brief Write a short value, as a hex number with at most 4 characters. E.g. the
 *        value 15 will be printed as "f".
 * @param pcDestination the output buffer where the hex number is to be printed.
 * @param uxSize the remaining length of the output buffer.
 * @param pxSet the set of parameters as used by FreeRTOS_inet_ntop6().
 * @return pdPASS in case the output buffer is big enough to contain the string.
 * @note uxSize must be at least 4, enough to print "abcd". The string will get
 *       null-terminated later on.
 */
static BaseType_t prv_ntop6_write_short( char * pcDestination,
                                         size_t uxSize,
                                         struct sNTOP6_Set * pxSet )
{
    socklen_t uxLength;
    BaseType_t xReturn = pdPASS;
    const size_t uxBytesPerShortValue = 4U;

    if( pxSet->xIndex > 0 )
    {
        if( pxSet->uxTargetIndex >= ( uxSize - 1U ) )
        {
            xReturn = pdFAIL;
        }
        else
        {
            pcDestination[ pxSet->uxTargetIndex ] = ':';
            pxSet->uxTargetIndex++;
        }
    }

    if( xReturn == pdPASS )
    {
        /* If there is enough space to write a short. */
        if( pxSet->uxTargetIndex <= ( uxSize - uxBytesPerShortValue ) )
        {
            /* Write hex value of short. at most 4 + 1 bytes. */
            uxLength = uxHexPrintShort( &( pcDestination[ pxSet->uxTargetIndex ] ),
                                        uxBytesPerShortValue + 1U,
                                        FreeRTOS_ntohs( pxSet->pusAddress[ pxSet->xIndex ] ) );

            /* uxLength will be non zero and positive always. */
            pxSet->uxTargetIndex += uxLength;
        }
        else
        {
            xReturn = pdFAIL;
        }
    }

    return xReturn;
}
/*-----------------------------------------------------------*/

/**
 * @brief This function converts a binary IPv6 address to a human readable notation.
 *
 * @param[in] pvSource The binary address, 16 bytes long..
 * @param[out] pcDestination The human-readable ( hexadecimal ) notation of the
 *                            address.
 * @param[in] uxSize The size of pvDestination. A value of 40 is recommended.
 *
 * @return pdPASS if the translation was successful or else pdFAIL.
 */
const char * FreeRTOS_inet_ntop6( const void * pvSource,
                                  char * pcDestination,
                                  socklen_t uxSize )
{
    const char * pcReturn;  /* The return value, which is either 'pcDestination' or NULL. */
    struct sNTOP6_Set xSet; /* A set of values for easy exchange with the helper functions prv_ntop6_xxx(). */

    ( void ) memset( &( xSet ), 0, sizeof( xSet ) );

    xSet.pusAddress = pvSource;

    if( uxSize < 3U )
    {
        /* Can not even print :: */
    }
    else
    {
        prv_ntop6_search_zeros( &( xSet ) );

        while( xSet.xIndex < 8 )
        {
            if( xSet.xIndex == xSet.xZeroStart )
            {
                if( prv_ntop6_write_zeros( pcDestination, uxSize, &( xSet ) ) == pdFAIL )
                {
                    break;
                }

                xSet.xIndex += xSet.xZeroLength;
            }
            else
            {
                if( prv_ntop6_write_short( pcDestination, uxSize, &( xSet ) ) == pdFAIL )
                {
                    break;
                }

                xSet.xIndex++;
            }
        }
    }

    if( xSet.xIndex < 8 )
    {
        /* Didn't reach the last nibble: clear the string. */
        pcReturn = NULL;
    }
    else
    {
        pcDestination[ xSet.uxTargetIndex ] = '\0';
        pcReturn = pcDestination;
    }

    return pcReturn;
}
/*-----------------------------------------------------------*/

/**
 * @brief Converting a readable IPv6 address to its binary form, add one nibble.
 *
 * @param[in] pxSet  A set of variables describing the conversion.
 * @param[in] ucNew  The hex value, between 0 and 15
 * @param[in] ch  The character, such as '5', 'f', or ':'.
 *
 * @return pdTRUE when the nibble was added, otherwise pdFALSE.
 */
static BaseType_t prv_inet_pton6_add_nibble( struct sPTON6_Set * pxSet,
                                             uint8_t ucNew,
                                             char ch )
{
    BaseType_t xReturn = pdPASS;

    if( ucNew != ( uint8_t ) socketINVALID_HEX_CHAR )
    {
        /* Shift in 4 bits. */
        pxSet->ulValue <<= 4;
        pxSet->ulValue |= ( uint32_t ) ucNew;

        /* Remember that ulValue is valid now. */
        pxSet->xHadDigit = pdTRUE;

        /* Check if the number is not becoming larger than 16 bits. */
        if( pxSet->ulValue > 0xffffU )
        {
            /* The highest nibble has already been set,
             * an overflow would occur.  Break out of the for-loop. */
            xReturn = pdFAIL;
        }
    }
    else if( ch == ':' )
    {
        if( pxSet->xHadDigit == pdFALSE )
        {
            /* A "::" sequence has been received. Check if it is not a third colon. */
            if( pxSet->xColon >= 0 )
            {
                xReturn = pdFAIL;
            }
            else
            {
                /* Two or more zero's are expected, starting at position 'xColon'. */
                pxSet->xColon = pxSet->xTargetIndex;
            }
        }
        else
        {
            if( pxSet->xTargetIndex <= pxSet->xHighestIndex )
            {
                /* Store a short value at position 'xTargetIndex'. */
                pxSet->pucTarget[ pxSet->xTargetIndex ] = ( uint8_t ) ( ( pxSet->ulValue >> 8 ) & 0xffU );
                pxSet->pucTarget[ pxSet->xTargetIndex + 1 ] = ( uint8_t ) ( pxSet->ulValue & 0xffU );
                pxSet->xTargetIndex += 2;
                pxSet->xHadDigit = pdFALSE;
                pxSet->ulValue = 0U;
            }
            else
            {
                xReturn = pdFAIL;
            }
        }
    }
    else
    {
        /* When an IPv4 address or rubbish is provided, this statement will be reached. */
        xReturn = pdFAIL;
    }

    return xReturn;
}
/*-----------------------------------------------------------*/

/**
 * @brief Convert an ASCII character to its corresponding hexadecimal value.
 *        A :: block was found, now fill in the zero's.
 * @param[in] pxSet  A set of variables describing the conversion.
 */
static void prv_inet_pton6_set_zeros( struct sPTON6_Set * pxSet )
{
    /* The number of bytes that were written after the :: */
    const BaseType_t xCount = pxSet->xTargetIndex - pxSet->xColon;
    const BaseType_t xTopIndex = ( BaseType_t ) ipSIZE_OF_IPv6_ADDRESS;
    BaseType_t xIndex;
    BaseType_t xTarget = xTopIndex - 1;
    BaseType_t xSource = pxSet->xColon + ( xCount - 1 );

    /* Inserting 'xCount' zero's. */
    for( xIndex = 0; xIndex < xCount; xIndex++ )
    {
        pxSet->pucTarget[ xTarget ] = pxSet->pucTarget[ xSource ];
        pxSet->pucTarget[ xSource ] = 0;
        xTarget--;
        xSource--;
    }

    pxSet->xTargetIndex = ( BaseType_t ) ipSIZE_OF_IPv6_ADDRESS;
}
/*-----------------------------------------------------------*/

/**
 * @brief Convert an IPv6 address in hexadecimal notation to a binary format of 16 bytes.
 *
 * @param[in] pcSource The address in hexadecimal notation.
 * @param[out] pvDestination The address in binary format, 16 bytes long.
 *
 * @return The 32-bit representation of IP(v4) address.
 */
BaseType_t FreeRTOS_inet_pton6( const char * pcSource,
                                void * pvDestination )
{
    char ch;
    uint8_t ucNew;
    BaseType_t xResult;
    struct sPTON6_Set xSet;

    const char * pcIterator = pcSource;

    ( void ) memset( &( xSet ), 0, sizeof( xSet ) );
    xSet.xColon = -1;
    xSet.pucTarget = pvDestination;

    ( void ) memset( xSet.pucTarget, 0, ipSIZE_OF_IPv6_ADDRESS );

    xResult = 0;

    /* Leading :: requires some special handling. */
    if( strcmp( pcIterator, "::" ) == 0 )
    {
        xResult = 1;
    }
    else
    {
        if( pcIterator[ 0 ] == ':' )
        {
            pcIterator++;
        }

        /* The last bytes will be written at position 14 and 15. */
        xSet.xHighestIndex = ( BaseType_t ) ipSIZE_OF_IPv6_ADDRESS;
        xSet.xHighestIndex -= ( BaseType_t ) sizeof( uint16_t );

        /* The value in ulValue is not yet valid. */
        xSet.xHadDigit = pdFALSE;
        xSet.ulValue = 0U;

        for( ; ; )
        {
            ch = *( pcIterator );
            pcIterator++;

            if( ch == ( char ) '\0' )
            {
                /* The string is parsed now.
                 * Store the last short, if present. */
                if( ( xSet.xHadDigit != pdFALSE ) &&
                    ( xSet.xTargetIndex <= xSet.xHighestIndex ) )
                {
                    /* Add the last value seen, network byte order ( MSB first ). */
                    xSet.pucTarget[ xSet.xTargetIndex ] = ( uint8_t ) ( ( xSet.ulValue >> 8 ) & 0xffU );
                    xSet.pucTarget[ xSet.xTargetIndex + 1 ] = ( uint8_t ) ( xSet.ulValue & 0xffU );
                    xSet.xTargetIndex += 2;
                }

                /* Break out of the for-ever loop. */
                break;
            }

            /* Convert from a readable character to a hex value. */
            ucNew = ucASCIIToHex( ch );
            /* See if this is a digit or a colon. */
            xResult = prv_inet_pton6_add_nibble( &( xSet ), ucNew, ch );

            if( xResult == pdFALSE )
            {
                /* The new character was not accepted. */
                break;
            }
        } /* for( ;; ) */

        if( xSet.xColon >= 0 )
        {
            /* The address contains a block of zero. */
            prv_inet_pton6_set_zeros( &( xSet ) );
        }

        if( xSet.xTargetIndex == ( BaseType_t ) ipSIZE_OF_IPv6_ADDRESS )
        {
            xResult = 1;
        }
    }

    if( xResult != 1 )
    {
        xSet.pucTarget = ( uint8_t * ) pvDestination;
        ( void ) memset( xSet.pucTarget, 0, ipSIZE_OF_IPv6_ADDRESS );
    }

    return xResult;
}

/*-----------------------------------------------------------*/
/* *INDENT-OFF* */
#endif /* ipconfigUSE_IPv6 != 0 */
/* *INDENT-ON* */
