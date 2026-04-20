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
 * @file FreeRTOS_TCP_Reception.c
 * @brief Module which processes the packet received from a socket for FreeRTOS+TCP.
 *
 * Endianness: in this module all ports and IP addresses are stored in
 * host byte-order, except fields in the IP-packets
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
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"
#include "FreeRTOS_IP_Private.h"
#include "FreeRTOS_UDP_IP.h"
#include "FreeRTOS_DHCP.h"
#include "NetworkInterface.h"
#include "NetworkBufferManagement.h"
#include "FreeRTOS_TCP_Transmission.h"
#include "FreeRTOS_TCP_Reception.h"

/* Just make sure the contents doesn't get compiled if TCP is not enabled. */
#if ipconfigUSE_TCP == 1

/*
 * Identify and deal with a single TCP header option, advancing the pointer to
 * the header. This function returns pdTRUE or pdFALSE depending on whether the
 * caller should continue to parse more header options or break the loop.
 */
    static int32_t prvSingleStepTCPHeaderOptions( const uint8_t * const pucPtr,
                                                  size_t uxTotalLength,
                                                  FreeRTOS_Socket_t * const pxSocket,
                                                  BaseType_t xHasSYNFlag );

    #if ( ipconfigUSE_TCP_WIN == 1 )

/*
 * Skip past TCP header options when doing Selective ACK, until there are no
 * more options left.
 */
        static void prvReadSackOption( const uint8_t * const pucPtr,
                                       size_t uxIndex,
                                       FreeRTOS_Socket_t * const pxSocket );
    #endif /* ( ipconfigUSE_TCP_WIN == 1 ) */

/**
 * @brief Parse the TCP option(s) received, if present.
 *
 * @param[in] pxSocket The socket handling the connection.
 * @param[in] pxNetworkBuffer The network buffer containing the TCP
 *                             packet.
 *
 * @return: If the options are well formed and processed successfully
 *          then pdPASS is returned; else a pdFAIL is returned.
 *
 * @note It has already been verified that:
 *       ((pxTCPHeader->ucTCPOffset & 0xf0) > 0x50), meaning that
 *       the TP header is longer than the usual 20 (5 x 4) bytes.
 */
    BaseType_t prvCheckOptions( FreeRTOS_Socket_t * pxSocket,
                                const NetworkBufferDescriptor_t * pxNetworkBuffer )
    {
        size_t uxTCPHeaderOffset = ipSIZE_OF_ETH_HEADER + uxIPHeaderSizePacket( pxNetworkBuffer );

        /* MISRA Ref 11.3.1 [Misaligned access] */
        /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
        /* coverity[misra_c_2012_rule_11_3_violation] */
        const ProtocolHeaders_t * pxProtocolHeaders = ( ( ProtocolHeaders_t * )
                                                        &( pxNetworkBuffer->pucEthernetBuffer[ uxTCPHeaderOffset ] ) );
        const TCPHeader_t * pxTCPHeader;
        const uint8_t * pucPtr;
        BaseType_t xHasSYNFlag;
        BaseType_t xReturn = pdPASS;
        /* Offset in the network packet where the first option byte is stored. */
        size_t uxOptionOffset = uxTCPHeaderOffset + ipSIZE_OF_TCP_HEADER;
        size_t uxOptionsLength;
        int32_t lResult;
        uint8_t ucLength;

        pxTCPHeader = &( pxProtocolHeaders->xTCPHeader );


        /* A character pointer to iterate through the option data */
        pucPtr = pxTCPHeader->ucOptdata;

        if( pxTCPHeader->ucTCPOffset <= ( 5U << 4U ) )
        {
            /* Avoid integer underflow in computation of ucLength. */
        }
        else
        {
            ucLength = ( uint8_t ) ( ( ( pxTCPHeader->ucTCPOffset >> 4U ) - 5U ) << 2U );
            uxOptionsLength = ( size_t ) ucLength;

            if( pxNetworkBuffer->xDataLength > uxOptionOffset )
            {
                /* Validate options size calculation. */
                if( uxOptionsLength <= ( pxNetworkBuffer->xDataLength - uxOptionOffset ) )
                {
                    if( ( pxTCPHeader->ucTCPFlags & tcpTCP_FLAG_SYN ) != ( uint8_t ) 0U )
                    {
                        xHasSYNFlag = pdTRUE;
                    }
                    else
                    {
                        xHasSYNFlag = pdFALSE;
                    }

                    /* The length check is only necessary in case the option data are
                     *  corrupted, we don't like to run into invalid memory and crash. */
                    for( ; ; )
                    {
                        if( uxOptionsLength == 0U )
                        {
                            /* coverity[break_stmt] : Break statement terminating the loop */
                            break;
                        }

                        lResult = prvSingleStepTCPHeaderOptions( pucPtr, uxOptionsLength, pxSocket, xHasSYNFlag );

                        if( lResult < 0 )
                        {
                            xReturn = pdFAIL;
                            break;
                        }

                        if( lResult == 0 )
                        {
                            break;
                        }

                        uxOptionsLength -= ( size_t ) lResult;
                        pucPtr = &( pucPtr[ lResult ] );
                    }
                }
            }
        }

        return xReturn;
    }
    /*-----------------------------------------------------------*/

/**
 * @brief Identify and deal with a single TCP header option, advancing the pointer to
 *        the header.
 *
 * @param[in] pucPtr Pointer to the TCP packet options.
 * @param[in] uxTotalLength Length of the TCP packet options.
 * @param[in] pxSocket Socket handling the connection.
 * @param[in] xHasSYNFlag Whether the header has SYN flag or not.
 *
 * @return This function returns index of the next option if the current option is
 *         successfully processed and it is not the end of options whereafter the caller
 *         should continue to process more options.
 *         If the options have ended, this function will return a zero whereafter the
 *         caller should stop parsing options and continue further processing.
 *         If the current option has erroneous value, then the function returns a
 *         negative value wherein the calling function should not process this packet any
 *         further and drop it.
 */
    static int32_t prvSingleStepTCPHeaderOptions( const uint8_t * const pucPtr,
                                                  size_t uxTotalLength,
                                                  FreeRTOS_Socket_t * const pxSocket,
                                                  BaseType_t xHasSYNFlag )
    {
        UBaseType_t uxNewMSS;
        size_t uxRemainingOptionsBytes = uxTotalLength;
        uint8_t ucLen;
        int32_t lIndex = 0;
        TCPWindow_t * pxTCPWindow = &( pxSocket->u.xTCP.xTCPWindow );
        BaseType_t xReturn = pdFALSE;

        if( pucPtr[ 0U ] == tcpTCP_OPT_END )
        {
            /* End of options. */
            lIndex = 0;
        }
        else if( pucPtr[ 0U ] == tcpTCP_OPT_NOOP )
        {
            /* NOP option, inserted to make the length a multiple of 4. */
            lIndex = 1;
        }
        else if( uxRemainingOptionsBytes < 2U )
        {
            /* Any other well-formed option must be at least two bytes: the option
             * type byte followed by a length byte. */
            lIndex = -1;
        }

        #if ( ipconfigUSE_TCP_WIN != 0 )
            else if( pucPtr[ 0 ] == tcpTCP_OPT_WSOPT )
            {
                /* The TCP Window Scale Option. */
                /* Confirm that the option fits in the remaining buffer space. */
                if( ( uxRemainingOptionsBytes < tcpTCP_OPT_WSOPT_LEN ) || ( pucPtr[ 1 ] != tcpTCP_OPT_WSOPT_LEN ) )
                {
                    lIndex = -1;
                }
                else
                {
                    /* Option is only valid in SYN phase. */
                    if( xHasSYNFlag != 0 )
                    {
                        /* From RFC7323 - section 2.3, we should limit the WSopt not larger than 14. */
                        if( pucPtr[ 2 ] > tcpTCP_OPT_WSOPT_MAXIMUM_VALUE )
                        {
                            FreeRTOS_debug_printf( ( "The WSopt(%u) from SYN packet is larger than maximum value.", pucPtr[ 2 ] ) );
                            pxSocket->u.xTCP.ucPeerWinScaleFactor = tcpTCP_OPT_WSOPT_MAXIMUM_VALUE;
                        }
                        else
                        {
                            pxSocket->u.xTCP.ucPeerWinScaleFactor = pucPtr[ 2 ];
                        }

                        pxSocket->u.xTCP.bits.bWinScaling = pdTRUE_UNSIGNED;
                    }

                    lIndex = ( int32_t ) tcpTCP_OPT_WSOPT_LEN;
                }
            }
        #endif /* ipconfigUSE_TCP_WIN */
        else if( pucPtr[ 0 ] == tcpTCP_OPT_MSS )
        {
            /* Confirm that the option fits in the remaining buffer space. */
            if( ( uxRemainingOptionsBytes < tcpTCP_OPT_MSS_LEN ) || ( pucPtr[ 1 ] != tcpTCP_OPT_MSS_LEN ) )
            {
                lIndex = -1;
            }
            else
            {
                /* An MSS option with the correct option length.  FreeRTOS_htons()
                 * is not needed here because usChar2u16() already returns a host
                 * endian number. */
                uxNewMSS = usChar2u16( &( pucPtr[ 2 ] ) );

                if( pxSocket->u.xTCP.usMSS != uxNewMSS )
                {
                    /* Perform a basic check on the the new MSS. */
                    if( uxNewMSS == 0U )
                    {
                        lIndex = -1;

                        /* Return Condition found. */
                        xReturn = pdTRUE;
                    }
                    else
                    {
                        FreeRTOS_debug_printf( ( "MSS change %u -> %u\n", pxSocket->u.xTCP.usMSS, ( unsigned ) uxNewMSS ) );
                    }
                }

                /* If a 'return' condition has not been found. */
                if( xReturn == pdFALSE )
                {
                    /* Restrict the minimum value of segment length to the ( Minimum IP MTU (576) - IP header(20) - TCP Header(20) ).
                     * See "RFC 791 section 3.1 Total Length" for more details. */
                    if( uxNewMSS < tcpMINIMUM_SEGMENT_LENGTH )
                    {
                        uxNewMSS = tcpMINIMUM_SEGMENT_LENGTH;
                    }

                    if( pxSocket->u.xTCP.usMSS > uxNewMSS )
                    {
                        /* our MSS was bigger than the MSS of the other party: adapt it. */
                        pxSocket->u.xTCP.bits.bMssChange = pdTRUE_UNSIGNED;

                        if( pxSocket->u.xTCP.usMSS > uxNewMSS )
                        {
                            /* The peer advertises a smaller MSS than this socket was
                             * using.  Use that as well. */
                            FreeRTOS_debug_printf( ( "Change mss %d => %u\n", pxSocket->u.xTCP.usMSS, ( unsigned ) uxNewMSS ) );
                        }

                        pxTCPWindow->xSize.ulRxWindowLength = ( ( uint32_t ) uxNewMSS ) * ( pxTCPWindow->xSize.ulRxWindowLength / ( ( uint32_t ) uxNewMSS ) );
                        pxTCPWindow->usMSSInit = ( uint16_t ) uxNewMSS;
                        pxTCPWindow->usMSS = ( uint16_t ) uxNewMSS;
                        pxSocket->u.xTCP.usMSS = ( uint16_t ) uxNewMSS;
                    }

                    lIndex = ( int32_t ) tcpTCP_OPT_MSS_LEN;
                }
            }
        }
        else
        {
            /* All other options have a length field, so that we easily
             * can skip past them. */
            ucLen = pucPtr[ 1 ];
            lIndex = 0;

            if( ( ucLen < ( uint8_t ) 2U ) || ( uxRemainingOptionsBytes < ( size_t ) ucLen ) )
            {
                /* If the length field is too small or too big, the options are
                 * malformed, don't process them further.
                 */
                lIndex = -1;
            }
            else
            {
                #if ( ipconfigUSE_TCP_WIN == 1 )
                {
                    /* Selective ACK: the peer has received a packet but it is missing
                     * earlier packets. At least this packet does not need retransmission
                     * anymore. ulTCPWindowTxSack( ) takes care of this administration.
                     */
                    if( pucPtr[ 0U ] == tcpTCP_OPT_SACK_A )
                    {
                        ucLen = ( uint8_t ) ( ucLen - 2U );
                        lIndex += 2;

                        while( ucLen >= ( uint8_t ) 8U )
                        {
                            prvReadSackOption( pucPtr, ( size_t ) lIndex, pxSocket );
                            lIndex += 8;
                            ucLen = ( uint8_t ) ( ucLen - 8U );
                        }

                        /* ucLen should be 0 by now. */
                    }
                }
                #endif /* ipconfigUSE_TCP_WIN == 1 */

                lIndex += ( int32_t ) ucLen;
            }
        }

        #if ( ipconfigUSE_TCP_WIN == 0 )
            /* Avoid compiler warnings when TCP window is not used. */
            ( void ) xHasSYNFlag;
        #endif

        return lIndex;
    }
    /*-----------------------------------------------------------*/

    #if ( ipconfigUSE_TCP_WIN == 1 )

/**
 * @brief Skip past TCP header options when doing Selective ACK, until there are no
 *        more options left.
 *
 * @param[in] pucPtr Pointer to the TCP packet options.
 * @param[in] uxIndex Index of options in the TCP packet options.
 * @param[in] pxSocket Socket handling the TCP connection.
 */
        static void prvReadSackOption( const uint8_t * const pucPtr,
                                       size_t uxIndex,
                                       FreeRTOS_Socket_t * const pxSocket )
        {
            uint32_t ulFirst = ulChar2u32( &( pucPtr[ uxIndex ] ) );
            uint32_t ulLast = ulChar2u32( &( pucPtr[ uxIndex + 4U ] ) );
            uint32_t ulCount = ulTCPWindowTxSack( &( pxSocket->u.xTCP.xTCPWindow ), ulFirst, ulLast );

            /* ulTCPWindowTxSack( ) returns the number of bytes which have been acked
             * starting from the head position.  Advance the tail pointer in txStream.
             */
            if( ( pxSocket->u.xTCP.txStream != NULL ) && ( ulCount > 0U ) )
            {
                /* Just advancing the tail index, 'ulCount' bytes have been confirmed. */
                ( void ) uxStreamBufferGet( pxSocket->u.xTCP.txStream, 0, NULL, ( size_t ) ulCount, pdFALSE );
                pxSocket->xEventBits |= ( EventBits_t ) eSOCKET_SEND;

                #if ipconfigSUPPORT_SELECT_FUNCTION == 1
                {
                    if( ( pxSocket->xSelectBits & ( EventBits_t ) eSELECT_WRITE ) != 0U )
                    {
                        /* The field 'xEventBits' is used to store regular socket events
                         * (at most 8), as well as 'select events', which will be left-shifted.
                         */
                        pxSocket->xEventBits |= ( ( EventBits_t ) eSELECT_WRITE ) << SOCKET_EVENT_BIT_COUNT;
                    }
                }
                #endif

                /* In case the socket owner has installed an OnSent handler,
                 * call it now. */
                #if ( ipconfigUSE_CALLBACKS == 1 )
                {
                    if( ipconfigIS_VALID_PROG_ADDRESS( pxSocket->u.xTCP.pxHandleSent ) )
                    {
                        pxSocket->u.xTCP.pxHandleSent( pxSocket, ulCount );
                    }
                }
                #endif /* ipconfigUSE_CALLBACKS == 1  */
            }
        }

    #endif /* ( ipconfigUSE_TCP_WIN != 0 ) */
    /*-----------------------------------------------------------*/

/**
 * @brief prvCheckRxData(): called from prvTCPHandleState(). The
 *        first thing that will be done is find the TCP payload data
 *        and check the length of this data.
 *
 * @param[in] pxNetworkBuffer The network buffer holding the received data.
 * @param[out] ppucRecvData It will point to first byte of the TCP payload.
 *
 * @return Length of the received buffer.
 */
    BaseType_t prvCheckRxData( const NetworkBufferDescriptor_t * pxNetworkBuffer,
                               uint8_t ** ppucRecvData )
    {
        /* Map the ethernet buffer onto the ProtocolHeader_t struct for easy access to the fields. */

        /* MISRA Ref 11.3.1 [Misaligned access] */
        /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
        /* coverity[misra_c_2012_rule_11_3_violation] */
        const ProtocolHeaders_t * pxProtocolHeaders = ( ( ProtocolHeaders_t * )
                                                        &( pxNetworkBuffer->pucEthernetBuffer[ ( size_t ) ipSIZE_OF_ETH_HEADER + uxIPHeaderSizePacket( pxNetworkBuffer ) ] ) );
        const TCPHeader_t * pxTCPHeader = &( pxProtocolHeaders->xTCPHeader );
        int32_t lLength, lTCPHeaderLength, lReceiveLength, lUrgentLength;

        /* Map the buffer onto an IPHeader_t struct for easy access to fields. */

        const size_t xIPHeaderLength = uxIPHeaderSizePacket( pxNetworkBuffer );
        uint16_t usLength;
        uint8_t ucIntermediateResult = 0;

        /* Determine the length and the offset of the user-data sent to this
         * node.
         *
         * The size of the TCP header is given in a multiple of 4-byte words (single
         * byte, needs no ntoh() translation).  A shift-right 2: is the same as
         * (offset >> 4) * 4. */
        ucIntermediateResult = ( pxTCPHeader->ucTCPOffset & tcpVALID_BITS_IN_TCP_OFFSET_BYTE ) >> 2;
        lTCPHeaderLength = ( int32_t ) ucIntermediateResult;

        /* Let pucRecvData point to the first byte received. */
        *ppucRecvData = &( pxNetworkBuffer->pucEthernetBuffer[ ( size_t ) ipSIZE_OF_ETH_HEADER + xIPHeaderLength + ( size_t ) lTCPHeaderLength ] );

        /* Calculate lReceiveLength - the length of the TCP data received.  This is
         * equal to the total packet length minus:
         * ( LinkLayer length (14) + IP header length (20) + size of TCP header(20 +) ).*/
        lReceiveLength = ( int32_t ) pxNetworkBuffer->xDataLength;
        lReceiveLength -= ( int32_t ) ipSIZE_OF_ETH_HEADER;

        /* MISRA Ref 11.3.1 [Misaligned access] */
        /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
        /* coverity[misra_c_2012_rule_11_3_violation] */
        switch( ( ( EthernetHeader_t * ) pxNetworkBuffer->pucEthernetBuffer )->usFrameType )
        {
            #if ( ipconfigUSE_IPv4 != 0 )
                case ipIPv4_FRAME_TYPE:
                   {
                       /* MISRA Ref 11.3.1 [Misaligned access] */
                       /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
                       /* coverity[misra_c_2012_rule_11_3_violation] */
                       const IPHeader_t * pxIPHeader = ( ( IPHeader_t * ) &( pxNetworkBuffer->pucEthernetBuffer[ ipSIZE_OF_ETH_HEADER ] ) );

                       usLength = FreeRTOS_htons( pxIPHeader->usLength );
                       lLength = ( int32_t ) usLength;
                   }
                   break;
            #endif /* ( ipconfigUSE_IPv4 != 0 ) */

            #if ( ipconfigUSE_IPv6 != 0 )
                case ipIPv6_FRAME_TYPE:
                   {
                       /* MISRA Ref 11.3.1 [Misaligned access] */
                       /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
                       /* coverity[misra_c_2012_rule_11_3_violation] */
                       const IPHeader_IPv6_t * pxIPHeader = ( ( IPHeader_IPv6_t * ) &( pxNetworkBuffer->pucEthernetBuffer[ ipSIZE_OF_ETH_HEADER ] ) );

                       /* For Coverity: conversion and cast in 2 steps. */
                       usLength = FreeRTOS_htons( pxIPHeader->usPayloadLength );
                       lLength = ( int32_t ) usLength;
                       /* Add the length of the TCP-header, because that was not included in 'usPayloadLength'. */
                       lLength += ( int32_t ) sizeof( IPHeader_IPv6_t );
                   }
                   break;
            #endif /* ( ipconfigUSE_IPv6 != 0 ) */

            default:
                /* Shouldn't reach here */
                lLength = 0;
                break;
        }

        if( lReceiveLength > lLength )
        {
            /* More bytes were received than the reported length, often because of
             * padding bytes at the end. */
            lReceiveLength = lLength;
        }

        /* Subtract the size of the TCP and IP headers and the actual data size is
         * known. */
        if( lReceiveLength > ( lTCPHeaderLength + ( int32_t ) xIPHeaderLength ) )
        {
            lReceiveLength -= ( lTCPHeaderLength + ( int32_t ) xIPHeaderLength );
        }
        else
        {
            lReceiveLength = 0;
        }

        /* Urgent Pointer:
         * This field communicates the current value of the urgent pointer as a
         * positive offset from the sequence number in this segment.  The urgent
         * pointer points to the sequence number of the octet following the urgent
         * data.  This field is only be interpreted in segments with the URG control
         * bit set. */
        if( ( pxTCPHeader->ucTCPFlags & tcpTCP_FLAG_URG ) != 0U )
        {
            /* Although we ignore the urgent data, we have to skip it. */
            lUrgentLength = ( int32_t ) FreeRTOS_htons( pxTCPHeader->usUrgent );

            /* MISRA Ref 18.4.1 [Usage of +, -, += and -= operators on expression of pointer type]. */
            /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-184. */
            /* coverity[misra_c_2012_rule_18_4_violation] */
            *ppucRecvData += lUrgentLength;
            lReceiveLength -= FreeRTOS_min_int32( lReceiveLength, lUrgentLength );
        }

        return ( BaseType_t ) lReceiveLength;
    }
    /*-----------------------------------------------------------*/

/**
 * @brief prvStoreRxData(): called from prvTCPHandleState().
 *        The second thing is to do is check if the payload data may
 *        be accepted. If so, they will be added to the reception queue.
 *
 * @param[in] pxSocket The socket owning the connection.
 * @param[in] pucRecvData Pointer to received data.
 * @param[in] pxNetworkBuffer The network buffer descriptor.
 * @param[in] ulReceiveLength The length of the received data.
 *
 * @return 0 on success, -1 on failure of storing data.
 */
    BaseType_t prvStoreRxData( FreeRTOS_Socket_t * pxSocket,
                               const uint8_t * pucRecvData,
                               NetworkBufferDescriptor_t * pxNetworkBuffer,
                               uint32_t ulReceiveLength )
    {
        /* Map the ethernet buffer onto the ProtocolHeader_t struct for easy access to the fields. */
        size_t uxIPOffset = uxIPHeaderSizePacket( pxNetworkBuffer );
        /* MISRA Ref 11.3.1 [Misaligned access] */
/* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
        /* coverity[misra_c_2012_rule_11_3_violation] */
        const ProtocolHeaders_t * pxProtocolHeaders = ( ( const ProtocolHeaders_t * )
                                                        &( pxNetworkBuffer->pucEthernetBuffer[ ipSIZE_OF_ETH_HEADER + uxIPOffset ] ) );
        const TCPHeader_t * pxTCPHeader = &pxProtocolHeaders->xTCPHeader;
        TCPWindow_t * pxTCPWindow = &pxSocket->u.xTCP.xTCPWindow;
        uint32_t ulSequenceNumber, ulSpace;
        int32_t lOffset, lStored;
        BaseType_t xResult = 0;
        uint32_t ulRxLength = ulReceiveLength;
        const uint8_t * pucRxBuffer = &( pucRecvData[ 0 ] );

        ulSequenceNumber = FreeRTOS_ntohl( pxTCPHeader->ulSequenceNumber );

        if( ( ulRxLength > 0U ) && ( pxSocket->u.xTCP.eTCPState >= eSYN_RECEIVED ) )
        {
            uint32_t ulSkipCount = 0;

            /* See if way may accept the data contents and forward it to the socket
             * owner.
             *
             * If it can't be "accept"ed it may have to be stored and send a selective
             * ack (SACK) option to confirm it.  In that case, lTCPAddRxdata() will be
             * called later to store an out-of-order packet (in case lOffset is
             * negative). */
            if( pxSocket->u.xTCP.rxStream != NULL )
            {
                ulSpace = ( uint32_t ) uxStreamBufferGetSpace( pxSocket->u.xTCP.rxStream );
            }
            else
            {
                ulSpace = ( uint32_t ) pxSocket->u.xTCP.uxRxStreamSize;
            }

            lOffset = lTCPWindowRxCheck( pxTCPWindow, ulSequenceNumber, ulRxLength, ulSpace, &( ulSkipCount ) );

            if( lOffset >= 0 )
            {
                /* New data has arrived and may be made available to the user.  See
                 * if the head marker in rxStream may be advanced, only if lOffset == 0.
                 * In case the low-water mark is reached, bLowWater will be set
                 * "low-water" here stands for "little space". */
                if( ulSkipCount != 0U )
                {
                    /* A packet was received that starts before 'ulCurrentSequenceNumber',
                     * and that ends after it.  The first 'ulSkipCount' bytes shall be
                     * skipped. */
                    ulRxLength -= ulSkipCount;
                    pucRxBuffer = &( pucRecvData[ ulSkipCount ] );
                }

                lStored = lTCPAddRxdata( pxSocket, ( uint32_t ) lOffset, pucRxBuffer, ulRxLength );

                if( lStored != ( int32_t ) ulRxLength )
                {
                    FreeRTOS_debug_printf( ( "lTCPAddRxdata: stored %d / %u bytes? ?\n", ( int ) lStored, ( unsigned ) ulRxLength ) );

                    /* Received data could not be stored.  The socket's flag
                     * bMallocError has been set.  The socket now has the status
                     * eCLOSE_WAIT and a RST packet will be sent back. */
                    ( void ) prvTCPSendReset( pxNetworkBuffer );
                    xResult = -1;
                }
            }

            /* After a missing packet has come in, higher packets may be passed to
             * the user. */
            #if ( ipconfigUSE_TCP_WIN == 1 )
            {
                /* Now lTCPAddRxdata() will move the rxHead pointer forward
                 * so data becomes available to the user immediately
                 * In case the low-water mark is reached, bLowWater will be set. */
                if( ( xResult == 0 ) && ( pxTCPWindow->ulUserDataLength > 0U ) )
                {
                    ( void ) lTCPAddRxdata( pxSocket, 0U, NULL, pxTCPWindow->ulUserDataLength );
                    pxTCPWindow->ulUserDataLength = 0;
                }
            }
            #endif /* ipconfigUSE_TCP_WIN */
        }
        else
        {
            pxTCPWindow->ucOptionLength = 0U;
        }

        return xResult;
    }
    /*-----------------------------------------------------------*/

#endif /* ipconfigUSE_TCP == 1 */
