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

#ifndef FREERTOS_IPV6_SOCKETS_H
    #define FREERTOS_IPV6_SOCKETS_H

/* Standard includes. */
    #include <string.h>

/* FreeRTOS includes. */
    #include "FreeRTOS.h"
    #include "FreeRTOS_IP_Common.h"

    #ifdef __cplusplus
    extern "C" {
    #endif

/** @brief When ucASCIIToHex() can not convert a character,
 *         the value 255 will be returned.
 */
    #define socketINVALID_HEX_CHAR    ( 0xffU )

/** @brief The struct sNTOP6_Set is a set of parameters used by  the function FreeRTOS_inet_ntop6().
 * It passes this set to a few helper functions. */
    struct sNTOP6_Set
    {
        const uint16_t * pusAddress; /**< The network address, 8 short values. */
        BaseType_t xZeroStart;       /**< The position of the first byte of the longest train of zero values. */
        BaseType_t xZeroLength;      /**< The number of short values in the longest train of zero values. */
        BaseType_t xIndex;           /**< The read index in the array of short values, the network address. */
        socklen_t uxTargetIndex;     /**< The write index in 'pcDestination'. */
    };

/** @brief The struct sNTOP6_Set is a set of parameters used by  the function FreeRTOS_inet_ntop6().
 * It passes this set to a few helper functions.
 */
    struct sPTON6_Set
    {
        uint32_t ulValue;         /**< A 32-bit accumulator, only 16 bits are used. */
        BaseType_t xHadDigit;     /**< Becomes pdTRUE as soon as ulValue has valid data. */
        BaseType_t xTargetIndex;  /**< The index in the array pucTarget to write the next byte. */
        BaseType_t xColon;        /**< The position in the output where the train of zero's will start. */
        BaseType_t xHighestIndex; /**< The highest allowed value of xTargetIndex. */
        uint8_t * pucTarget;      /**< The array of bytes in which the resulting IPv6 address is written. */
    };

/**
 * @brief Convert an ASCII character to its corresponding hexadecimal value.
 *        Accepted characters are 0-9, a-f, and A-F.
 */
    uint8_t ucASCIIToHex( char cChar );

/* @brief Converts a hex value to a readable hex character, e.g. 14 becomes 'e'.
 */
    char cHexToChar( uint16_t usValue );

/** @brief Converts a hex value to a readable hex character, *
 *         e.g. 14 becomes 'e'.static char cHexToChar( unsigned short usValue );
 */
    socklen_t uxHexPrintShort( char * pcBuffer,
                               size_t uxBufferSize,
                               uint16_t usValue );

/** @brief Scan the binary IPv6 address and find the longest train of consecutive zero's.
 *         The result of this search will be stored in 'xZeroStart' and 'xZeroLength'.
 */
    void prv_ntop6_search_zeros( struct sNTOP6_Set * pxSet );


/** @brief Called by pxTCPSocketLookup(), this function will check if a socket
 *         is connected to a remote IP-address. It will be called from a loop
 *         iterating through all sockets. */
    FreeRTOS_Socket_t * pxTCPSocketLookup_IPv6( FreeRTOS_Socket_t * pxSocket,
                                                const IPv46_Address_t * pxAddress );

/**
 * @brief Called by prvSendUDPPacket(), this function will UDP packet
 *        fields and IPv6 address for the packet to be send.
 */
    void * xSend_UDP_Update_IPv6( NetworkBufferDescriptor_t * pxNetworkBuffer,
                                  const struct freertos_sockaddr * pxDestinationAddress );

/**
 * @brief Called by FreeRTOS_recvfrom(), this function will update socket
 *        address with IPv6 address from the packet received.
 */
    size_t xRecv_Update_IPv6( const NetworkBufferDescriptor_t * pxNetworkBuffer,
                              struct freertos_sockaddr * pxSourceAddress );

    #ifdef __cplusplus
}     /* extern "C" */
    #endif

#endif /* FREERTOS_IPV6_SOCKETS_H */
