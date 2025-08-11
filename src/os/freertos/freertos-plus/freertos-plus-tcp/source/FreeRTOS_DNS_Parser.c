/*
 * FreeRTOS+TCP V4.3.2
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
 * @file FreeRTOS_DNS_Parser.c
 * @brief Implements the DNS message parser
 */

/* FreeRTOS includes. */
#include "FreeRTOS.h"
/* FreeRTOS+TCP includes. */
#include "FreeRTOS_IP.h"
#include "FreeRTOS_IP_Private.h"

#include "FreeRTOS_DNS_Globals.h"
#include "FreeRTOS_DNS_Parser.h"
#include "FreeRTOS_DNS_Cache.h"
#include "FreeRTOS_DNS_Callback.h"

#include "NetworkBufferManagement.h"

#include <string.h>

#if ( ipconfigUSE_DNS != 0 )

    #if ( ( ipconfigUSE_DNS_CACHE != 0 ) || ( ipconfigDNS_USE_CALLBACKS != 0 ) || ( ipconfigUSE_MDNS != 0 ) || ( ipconfigUSE_LLMNR != 0 ) )

/**
 * @brief Read the Name field out of a DNS response packet.
 *
 * @param[in,out] pxSet a set of variables that are shared among the helper functions.
 * @param[in] uxDestLen Size of the pcName array.
 *
 * @return If a fully formed name was found, then return the number of bytes processed in pucByte.
 */
        size_t DNS_ReadNameField( ParseSet_t * pxSet,
                                  size_t uxDestLen )
        {
            size_t uxNameLen = 0U;
            size_t uxIndex = 0U;
            size_t uxSourceLen = pxSet->uxSourceBytesRemaining;
            const uint8_t * pucByte = pxSet->pucByte;

            /* uxCount gets the values from pucByte and counts down to 0.
             * No need to have a different type than that of pucByte */
            size_t uxCount;

            if( uxSourceLen == ( size_t ) 0U )
            {
                /* Return 0 value in case of error. */
                uxIndex = 0U;
            }

            /* Determine if the name is the fully coded name, or an offset to the name
             * elsewhere in the message. */
            else if( ( pucByte[ uxIndex ] & dnsNAME_IS_OFFSET ) == dnsNAME_IS_OFFSET )
            {
                /* Jump over the two byte offset. */
                if( uxSourceLen > sizeof( uint16_t ) )
                {
                    uxIndex += sizeof( uint16_t );
                }
                else
                {
                    uxIndex = 0U;
                }
            }
            else
            {
                /* 'uxIndex' points to the full name. Walk over the string. */
                while( ( uxIndex < uxSourceLen ) && ( pucByte[ uxIndex ] != ( uint8_t ) 0x00U ) )
                {
                    /* If this is not the first time through the loop, then add a
                     * separator in the output. */
                    if( ( uxNameLen > 0U ) )
                    {
                        /*
                         * uxNameLen can never be greater than uxDestLen, since there are checks
                         * outside this condition, so the check is removed.
                         */
                        pxSet->pcName[ uxNameLen ] = '.';
                        uxNameLen++;
                    }

                    /* Process the first/next sub-string. */
                    uxCount = ( size_t ) pucByte[ uxIndex ];

                    /* uxIndex should point to the first character now, unless uxCount
                     * is an offset field. */
                    uxIndex++;

                    if( ( uxIndex + uxCount ) > uxSourceLen )
                    {
                        uxIndex = 0U;
                        break;
                    }

                    if( ( uxNameLen + uxCount ) >= uxDestLen )
                    {
                        uxIndex = 0U;
                        break;
                    }

                    while( uxCount-- != 0U )
                    {
                        /*
                         * uxNameLen can never be greater than uxDestLen, since there are checks
                         * outside this condition, so the check is removed.
                         */
                        pxSet->pcName[ uxNameLen ] = ( char ) pucByte[ uxIndex ];
                        uxNameLen++;
                        uxIndex++;
                    }
                }

                /* Confirm that a fully formed name was found. */
                if( uxIndex > 0U )
                {
                    /* Here, there is no need to check for pucByte[ uxindex ] == 0 because:
                     * When we break out of the above while loop, uxIndex is made 0 thereby
                     * failing above check. Whenever we exit the loop otherwise, either
                     * pucByte[ uxIndex ] == 0 (which makes the check here unnecessary) or
                     * uxIndex >= uxSourceLen (which makes sure that we do not go in the 'if'
                     * case).
                     */
                    if( uxIndex < uxSourceLen )
                    {
                        pxSet->pcName[ uxNameLen ] = '\0';
                        uxIndex++;
                    }
                    else
                    {
                        uxIndex = 0U;
                    }
                }
            }

            return uxIndex;
        }
    #endif /* ipconfigUSE_DNS_CACHE || ipconfigDNS_USE_CALLBACKS || ipconfigUSE_MDNS || ipconfigUSE_LLMNR */

/**
 * @brief Simple routine that jumps over the NAME field of a resource record.
 *
 * @param[in] pucByte The pointer to the resource record.
 * @param[in] uxLength Length of the resource record.
 *
 * @return It returns the number of bytes read, or zero when an error has occurred.
 */
    size_t DNS_SkipNameField( const uint8_t * pucByte,
                              size_t uxLength )
    {
        size_t uxChunkLength;
        size_t uxSourceLenCpy = uxLength;
        size_t uxIndex = 0U;

        if( uxSourceLenCpy == 0U )
        {
            uxIndex = 0U;
        }

        /* Determine if the name is the fully coded name, or an offset to the name
         * elsewhere in the message. */
        else if( ( pucByte[ uxIndex ] & dnsNAME_IS_OFFSET ) == dnsNAME_IS_OFFSET )
        {
            /* Jump over the two byte offset. */
            if( uxSourceLenCpy > sizeof( uint16_t ) )
            {
                uxIndex += sizeof( uint16_t );
            }
            else
            {
                uxIndex = 0U;
            }
        }
        else
        {
            /* pucByte points to the full name. Walk over the string. */
            while( ( pucByte[ uxIndex ] != 0U ) && ( uxSourceLenCpy > 1U ) )
            {
                /* Conversion to size_t causes addition to be done
                 * in size_t */
                uxChunkLength = ( ( size_t ) pucByte[ uxIndex ] ) + 1U;

                if( uxSourceLenCpy > uxChunkLength )
                {
                    uxSourceLenCpy -= uxChunkLength;
                    uxIndex += uxChunkLength;
                }
                else
                {
                    uxIndex = 0U;
                    break;
                }
            }

            /* Confirm that a fully formed name was found. */
            if( uxIndex > 0U )
            {
                if( pucByte[ uxIndex ] == 0U )
                {
                    uxIndex++;
                }
                else
                {
                    uxIndex = 0U;
                }
            }
        }

        return uxIndex;
    }

/**
 * @brief Process a response packet from a DNS server, or an LLMNR reply.
 *
 * @param[in] pucUDPPayloadBuffer The DNS response received as a UDP
 *                                 payload.
 * @param[in] uxBufferLength Length of the UDP payload buffer.
 * @param[in] ppxAddressInfo A pointer to a pointer where the results will be stored.
 * @param[in] xExpected indicates whether the identifier in the reply
 *                       was expected, and thus if the DNS cache may be
 *                       updated with the reply.
 * @param[in] usPort The server port number in order to identify the protocol.
 *
 *
 * @return The IP address in the DNS response if present and if xExpected is set to pdTRUE.
 *         An error code (dnsPARSE_ERROR) if there was an error in the DNS response.
 *         0 if xExpected set to pdFALSE.
 */
    uint32_t DNS_ParseDNSReply( uint8_t * pucUDPPayloadBuffer,
                                size_t uxBufferLength,
                                struct freertos_addrinfo ** ppxAddressInfo,
                                BaseType_t xExpected,
                                uint16_t usPort )
    {
        ParseSet_t xSet;
        uint16_t x;
        BaseType_t xReturn = pdTRUE;
        uint32_t ulIPAddress = 0U;
        BaseType_t xDNSHookReturn = 0U;

        ( void ) memset( &( xSet ), 0, sizeof( xSet ) );
        xSet.usPortNumber = usPort;
        xSet.ppxLastAddress = &( xSet.pxLastAddress );

        #if ( ipconfigUSE_DNS_CACHE == 1 ) || ( ipconfigDNS_USE_CALLBACKS == 1 )
            xSet.xDoStore = xExpected;
        #endif

        /* Ensure that the buffer is of at least minimal DNS message length. */
        if( uxBufferLength < sizeof( DNSMessage_t ) )
        {
            ( void ) xDNSHookReturn;
            xReturn = pdFALSE;
        }
        else
        {
            xSet.uxBufferLength = uxBufferLength;
            xSet.uxSourceBytesRemaining = uxBufferLength;

            /* Parse the DNS message header. Map the byte stream onto a structure
             * for easier access. */

            /* MISRA Ref 11.3.1 [Misaligned access] */
            /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
            /* coverity[misra_c_2012_rule_11_3_violation] */
            xSet.pxDNSMessageHeader = ( ( DNSMessage_t * )
                                        pucUDPPayloadBuffer );

            /* Introduce a do {} while (0) to allow the use of breaks. */
            do
            {
                size_t uxBytesRead = 0U;
                size_t uxResult;
                BaseType_t xIsResponse = pdFALSE;

                /* Start at the first byte after the header. */
                xSet.pucUDPPayloadBuffer = pucUDPPayloadBuffer;
                /* Skip 12-byte header. */
                xSet.pucByte = &( pucUDPPayloadBuffer[ sizeof( DNSMessage_t ) ] );
                xSet.uxSourceBytesRemaining -= sizeof( DNSMessage_t );

                /* The number of questions supplied. */
                xSet.usQuestions = FreeRTOS_ntohs( xSet.pxDNSMessageHeader->usQuestions );
                /* The number of answer records. */
                xSet.usAnswers = FreeRTOS_ntohs( xSet.pxDNSMessageHeader->usAnswers );

                if( ( xSet.pxDNSMessageHeader->usFlags & dnsRX_FLAGS_MASK ) == dnsEXPECTED_RX_FLAGS )
                {
                    xIsResponse = pdTRUE;

                    if( xSet.usAnswers == 0U )
                    {
                        /* This is a response that does not include answers. */
                        xReturn = pdFALSE;
                        break;
                    }

                    if( xSet.usQuestions == 0U )
                    {
                        #if ( ( ipconfigUSE_LLMNR == 1 ) || ( ipconfigUSE_MDNS == 1 ) )
                        {
                            xSet.pcRequestedName = ( char * ) xSet.pucByte;
                        }
                        #endif

                        #if ( ipconfigUSE_DNS_CACHE == 1 ) || ( ipconfigDNS_USE_CALLBACKS == 1 )
                            uxResult = DNS_ReadNameField( &xSet,
                                                          sizeof( xSet.pcName ) );
                            ( void ) uxResult;
                        #endif
                    }
                }
                else
                {
                    if( xSet.usQuestions == 0U )
                    {
                        /* This is a query that does not include any question. */
                        xReturn = pdFALSE;
                        break;
                    }
                }

                for( x = 0U; x < xSet.usQuestions; x++ )
                {
                    #if ( ( ipconfigUSE_LLMNR == 1 ) || ( ipconfigUSE_MDNS == 1 ) )
                    {
                        if( x == 0U )
                        {
                            xSet.pcRequestedName = ( char * ) xSet.pucByte;
                        }
                    }
                    #endif

                    #if ( ( ipconfigUSE_DNS_CACHE != 0 ) || ( ipconfigDNS_USE_CALLBACKS != 0 ) || ( ipconfigUSE_MDNS != 0 ) || ( ipconfigUSE_LLMNR != 0 ) )
                        if( x == 0U )
                        {
                            uxResult = DNS_ReadNameField( &xSet,
                                                          sizeof( xSet.pcName ) );
                            ( void ) uxResult;
                        }
                        else
                    #endif /* ipconfigUSE_DNS_CACHE || ipconfigDNS_USE_CALLBACKS || ipconfigUSE_MDNS || ipconfigUSE_LLMNR */
                    {
                        /* Skip the variable length pcName field. */
                        uxResult = DNS_SkipNameField( xSet.pucByte,
                                                      xSet.uxSourceBytesRemaining );
                    }

                    /* Check for a malformed response. */
                    if( uxResult == 0U )
                    {
                        xReturn = pdFALSE;
                        break;
                    }

                    uxBytesRead += uxResult;
                    xSet.pucByte = &( xSet.pucByte[ uxResult ] );
                    xSet.uxSourceBytesRemaining -= uxResult;

                    /* Check the remaining buffer size. */
                    if( xSet.uxSourceBytesRemaining >= sizeof( uint32_t ) )
                    {
                        #if ( ( ipconfigUSE_LLMNR == 1 ) || ( ipconfigUSE_MDNS == 1 ) )
                        {
                            /* usChar2u16 returns value in host endianness. */
                            xSet.usType = usChar2u16( xSet.pucByte );
                            xSet.usClass = usChar2u16( &( xSet.pucByte[ 2 ] ) );
                        }
                        #endif /* ipconfigUSE_LLMNR */

                        /* Skip the type and class fields. */
                        xSet.pucByte = &( xSet.pucByte[ sizeof( uint32_t ) ] );
                        xSet.uxSourceBytesRemaining -= sizeof( uint32_t );
                    }
                    else
                    {
                        xReturn = pdFALSE;
                        break;
                    }
                } /* for( x = 0U; x < xSet.usQuestions; x++ ) */

                if( xReturn == pdFALSE )
                {
                    /* No need to proceed. Break out of the do-while loop. */
                    break;
                }

                if( xIsResponse == pdTRUE )
                {
                    /* Search through the answer records. */
                    ulIPAddress = parseDNSAnswer( &( xSet ), ppxAddressInfo, &uxBytesRead );
                }

                #if ( ( ipconfigUSE_LLMNR == 1 ) || ( ipconfigUSE_MDNS == 1 ) )

                    /* No need to check that pcRequestedName != NULL since sQuestions != 0, then
                     * pcRequestedName is assigned with this statement
                     * "pcRequestedName = ( char * ) pucByte;" */
                    /* No need to check that usQuestions != 0, since the check is done before */
                    else if( ( ( xSet.usType == dnsTYPE_A_HOST ) || ( xSet.usType == dnsTYPE_AAAA_HOST ) ) &&
                             ( xSet.usClass == dnsCLASS_IN ) )
                    {
                        NetworkBufferDescriptor_t * pxNetworkBuffer;
                        NetworkEndPoint_t * pxEndPoint, xEndPoint;
                        size_t uxUDPOffset;

                        pxNetworkBuffer = pxUDPPayloadBuffer_to_NetworkBuffer( pucUDPPayloadBuffer );

                        /* This test could be replaced with a assert(). */
                        if( pxNetworkBuffer == NULL )
                        {
                            /* _HT_ just while testing. When the program gets here,
                             * pucUDPPayloadBuffer was invalid. */
                            FreeRTOS_printf( ( "DNS_ParseDNSReply: pucUDPPayloadBuffer was invalid\n" ) );
                            break;
                        }

                        uxUDPOffset = ( size_t ) ( pucUDPPayloadBuffer - pxNetworkBuffer->pucEthernetBuffer );
                        configASSERT( ( uxUDPOffset == ipUDP_PAYLOAD_OFFSET_IPv4 ) || ( uxUDPOffset == ipUDP_PAYLOAD_OFFSET_IPv6 ) );

                        if( pxNetworkBuffer->pxEndPoint == NULL )
                        {
                            break;
                        }

                        pxEndPoint = pxNetworkBuffer->pxEndPoint;

                        /* Make a copy of the end-point because xApplicationDNSQueryHook() is allowed
                         * to write into it. */
                        ( void ) memcpy( &( xEndPoint ), pxEndPoint, sizeof( xEndPoint ) );

                        #if ( ipconfigUSE_IPv6 != 0 )
                        {
                            /*logging*/
                            FreeRTOS_printf( ( "prvParseDNS_HandleLLMNRRequest[%s]: type %04X\n", xSet.pcName, xSet.usType ) );

                            xEndPoint.usDNSType = ( uint8_t ) xSet.usType;
                        }
                        #endif /* ( ipconfigUSE_IPv6 != 0 ) */

                        /* If this is not a reply to our DNS request, it might be an mDNS or an LLMNR
                         * request. Ask the application if it uses the name. */
                        #if ( ipconfigIPv4_BACKWARD_COMPATIBLE == 1 )
                            xDNSHookReturn = xApplicationDNSQueryHook( xSet.pcName );
                        #else
                            xDNSHookReturn = xApplicationDNSQueryHook_Multi( &xEndPoint, xSet.pcName );
                        #endif

                        /* During the early stages of boot or after a DHCP lease expires, our end-point
                         * may have an IP address of 0.0.0.0. Do not respond to name queries with that address. */
                        if( ( xDNSHookReturn != pdFALSE ) && ( xEndPoint.bits.bIPv6 == pdFALSE ) && ( xEndPoint.ipv4_settings.ulIPAddress == 0U ) )
                        {
                            xDNSHookReturn = pdFALSE;
                        }

                        if( xDNSHookReturn != pdFALSE )
                        {
                            int16_t usLength;
                            NetworkBufferDescriptor_t * pxNewBuffer = NULL;
                            LLMNRAnswer_t * pxAnswer;
                            uint8_t * pucNewBuffer = NULL;
                            size_t uxExtraLength;
                            size_t uxDataLength = uxBufferLength +
                                                  sizeof( UDPHeader_t ) +
                                                  sizeof( EthernetHeader_t ) +
                                                  uxIPHeaderSizePacket( pxNetworkBuffer );

                            #if ( ipconfigUSE_IPv6 != 0 )
                                if( xSet.usType == dnsTYPE_AAAA_HOST )
                                {
                                    uxExtraLength = sizeof( LLMNRAnswer_t ) + ipSIZE_OF_IPv6_ADDRESS - sizeof( pxAnswer->ulIPAddress );
                                }
                                else
                            #endif /* ( ipconfigUSE_IPv6 != 0 ) */
                            #if ( ipconfigUSE_IPv4 != 0 )
                            {
                                uxExtraLength = sizeof( LLMNRAnswer_t );
                            }
                            #else /* ( ipconfigUSE_IPv4 != 0 ) */
                            {
                                /* do nothing, coverity happy */
                            }
                            #endif /* ( ipconfigUSE_IPv4 != 0 ) */

                            if( xBufferAllocFixedSize == pdFALSE )
                            {
                                /* Set the size of the outgoing packet. */
                                pxNetworkBuffer->xDataLength = uxDataLength;
                                pxNewBuffer = pxDuplicateNetworkBufferWithDescriptor( pxNetworkBuffer,
                                                                                      uxDataLength +
                                                                                      uxExtraLength );

                                if( pxNewBuffer != NULL )
                                {
                                    BaseType_t xOffset1, xOffset2;

                                    xOffset1 = ( BaseType_t ) ( xSet.pucByte - pucUDPPayloadBuffer );
                                    xOffset2 = ( BaseType_t ) ( ( ( uint8_t * ) xSet.pcRequestedName ) - pucUDPPayloadBuffer );

                                    pxNetworkBuffer = pxNewBuffer;
                                    pucNewBuffer = &( pxNetworkBuffer->pucEthernetBuffer[ uxUDPOffset ] );

                                    xSet.pucByte = &( pucNewBuffer[ xOffset1 ] );
                                    xSet.pcRequestedName = ( char * ) &( pucNewBuffer[ xOffset2 ] );
                                    xSet.pxDNSMessageHeader = ( ( DNSMessage_t * ) pucNewBuffer );
                                }
                                else
                                {
                                    /* Just to indicate that the message may not be answered. */
                                    pxNetworkBuffer = NULL;
                                }
                            }
                            else
                            {
                                /* When xBufferAllocFixedSize is TRUE, check if the buffer size is big enough to
                                 * store the answer. */
                                if( ( uxDataLength + uxExtraLength ) <= ipconfigNETWORK_MTU + ipSIZE_OF_ETH_HEADER )
                                {
                                    pucNewBuffer = &( pxNetworkBuffer->pucEthernetBuffer[ uxUDPOffset ] );
                                }
                                else
                                {
                                    /* Just to indicate that the message may not be answered. */
                                    pxNetworkBuffer = NULL;
                                }
                            }

                            if( ( pxNetworkBuffer != NULL ) )
                            {
                                pxAnswer = ( ( LLMNRAnswer_t * ) xSet.pucByte );
                                /* We leave 'usIdentifier' and 'usQuestions' untouched */
                                vSetField16( xSet.pxDNSMessageHeader, DNSMessage_t, usFlags, dnsLLMNR_FLAGS_IS_RESPONSE ); /* Set the response flag */
                                vSetField16( xSet.pxDNSMessageHeader, DNSMessage_t, usAnswers, 1 );                        /* Provide a single answer */
                                vSetField16( xSet.pxDNSMessageHeader, DNSMessage_t, usAuthorityRRs, 0 );                   /* No authority */
                                vSetField16( xSet.pxDNSMessageHeader, DNSMessage_t, usAdditionalRRs, 0 );                  /* No additional info */

                                pxAnswer->ucNameCode = dnsNAME_IS_OFFSET;
                                pxAnswer->ucNameOffset = ( uint8_t ) ( xSet.pcRequestedName - ( char * ) pucNewBuffer );

                                vSetField16( pxAnswer, LLMNRAnswer_t, usType, xSet.usType );  /* Type A or AAAA: host */
                                vSetField16( pxAnswer, LLMNRAnswer_t, usClass, dnsCLASS_IN ); /* 1: Class IN */
                                vSetField32( pxAnswer, LLMNRAnswer_t, ulTTL, dnsLLMNR_TTL_VALUE );

                                usLength = ( int16_t ) ( sizeof( *pxAnswer ) + ( size_t ) ( xSet.pucByte - pucNewBuffer ) );

                                #if ( ipconfigUSE_IPv6 != 0 )
                                    if( xSet.usType == dnsTYPE_AAAA_HOST )
                                    {
                                        size_t uxDistance;
                                        vSetField16( pxAnswer, LLMNRAnswer_t, usDataLength, ipSIZE_OF_IPv6_ADDRESS );
                                        ( void ) memcpy( &( pxAnswer->ulIPAddress ), xEndPoint.ipv6_settings.xIPAddress.ucBytes, ipSIZE_OF_IPv6_ADDRESS );
                                        uxDistance = ( size_t ) ( xSet.pucByte - pucNewBuffer );
                                        /* An extra 12 bytes will be sent compared to an A-record. */
                                        usLength = ( int16_t ) ( sizeof( *pxAnswer ) + uxDistance + ipSIZE_OF_IPv6_ADDRESS - sizeof( pxAnswer->ulIPAddress ) );
                                    }
                                    else
                                #endif /* ( ipconfigUSE_IPv6 != 0 ) */
                                {
                                    size_t uxDistance;
                                    vSetField16( pxAnswer, LLMNRAnswer_t, usDataLength, ( uint16_t ) sizeof( pxAnswer->ulIPAddress ) );
                                    vSetField32( pxAnswer, LLMNRAnswer_t, ulIPAddress, FreeRTOS_ntohl( xEndPoint.ipv4_settings.ulIPAddress ) );
                                    uxDistance = ( size_t ) ( xSet.pucByte - pucNewBuffer );
                                    usLength = ( int16_t ) ( sizeof( *pxAnswer ) + uxDistance );
                                }

                                prepareReplyDNSMessage( pxNetworkBuffer, usLength );
                                /* This function will fill in the eth addresses and send the packet */
                                vReturnEthernetFrame( pxNetworkBuffer, pdFALSE );

                                if( pxNewBuffer != NULL )
                                {
                                    vReleaseNetworkBufferAndDescriptor( pxNewBuffer );
                                }
                            }
                        }
                        else
                        {
                            /* Not an expected reply. */
                        }
                    }
                #endif /* ipconfigUSE_LLMNR == 1 */
                ( void ) uxBytesRead;
            } while( ipFALSE_BOOL );
        }

        if( xReturn == pdFALSE )
        {
            /* There was an error while parsing the DNS response. Return error code. */
            ulIPAddress = ( uint32_t ) dnsPARSE_ERROR;
        }
        else if( xExpected == pdFALSE )
        {
            /* Do not return a valid IP-address in case the reply was not expected. */
            ulIPAddress = 0U;
        }
        else
        {
            /* The IP-address found will be returned. */
        }

        return ulIPAddress;
    }

/**
 * @brief Process DNS answer field in a DNS response packet from a DNS server.
 * @param[in] pxSet a set of variables that are shared among the helper functions.
 * @param[out] ppxAddressInfo a linked list storing the DNS answers.
 * @param[out] uxBytesRead total bytes consumed by the function
 * @return pdTRUE when successful, otherwise pdFALSE.
 */
    uint32_t parseDNSAnswer( ParseSet_t * pxSet,
                             struct freertos_addrinfo ** ppxAddressInfo,
                             size_t * uxBytesRead )
    {
        uint16_t x;
        size_t uxResult;
        uint32_t ulReturnIPAddress = 0U;
        const uint16_t usCount = ( uint16_t ) ipconfigDNS_CACHE_ADDRESSES_PER_ENTRY;
        BaseType_t xReturn = pdTRUE;
        const DNSAnswerRecord_t * pxDNSAnswerRecord;
        IPv46_Address_t xIP_Address;

        struct freertos_addrinfo * pxNewAddress = NULL;

        for( x = 0U; x < pxSet->usAnswers; x++ )
        {
            BaseType_t xDoAccept = pdFALSE;

            if( pxSet->usNumARecordsStored >= usCount )
            {
                /* Only count ipconfigDNS_CACHE_ADDRESSES_PER_ENTRY number of records. */
                break;
            }

            uxResult = DNS_SkipNameField( pxSet->pucByte,
                                          pxSet->uxSourceBytesRemaining );

            /* Check for a malformed response. */
            if( uxResult == 0U )
            {
                xReturn = pdFALSE;
                break;
            }

            if( uxBytesRead != NULL )
            {
                *uxBytesRead += uxResult;
            }

            pxSet->pucByte = &( pxSet->pucByte[ uxResult ] );
            pxSet->uxSourceBytesRemaining -= uxResult;

            /* Is there enough data for an IPv4 A record answer and, if so,
             * is this an A record? */
            if( pxSet->uxSourceBytesRemaining < sizeof( uint16_t ) )
            {
                xReturn = pdFALSE;
                break;
            }

            pxSet->usType = usChar2u16( pxSet->pucByte );

            if( pxSet->usType == ( uint16_t ) dnsTYPE_AAAA_HOST )
            {
                pxSet->uxAddressLength = ipSIZE_OF_IPv6_ADDRESS;

                if( pxSet->uxSourceBytesRemaining >= ( sizeof( DNSAnswerRecord_t ) + pxSet->uxAddressLength ) )
                {
                    xDoAccept = pdTRUE;
                }
            }
            else if( pxSet->usType == ( uint16_t ) dnsTYPE_A_HOST )
            {
                pxSet->uxAddressLength = ipSIZE_OF_IPv4_ADDRESS;

                if( pxSet->uxSourceBytesRemaining >= ( sizeof( DNSAnswerRecord_t ) + pxSet->uxAddressLength ) )
                {
                    /* Ignore responses containing an IP of 0.0.0.0.
                     * If we don't stop parsing this now, the code below will
                     * invoke the user callback and also store this invalid address in our cache. */
                    void * pvCopyDest;
                    const void * pvCopySource;
                    uint32_t ulTestAddress;

                    pvCopySource = &( pxSet->pucByte[ sizeof( DNSAnswerRecord_t ) ] );
                    pvCopyDest = &( ulTestAddress );
                    ( void ) memcpy( pvCopyDest, pvCopySource, pxSet->uxAddressLength );

                    if( ulTestAddress != 0U )
                    {
                        xDoAccept = pdTRUE;
                    }
                }
            }
            else
            {
                /* Unknown host type, AAAA nor A.
                 * 'xDoAccept' was already initialised as pdFALSE. */
            }

            if( xDoAccept != pdFALSE )
            {
                /* This is the required record type and is of sufficient size. */

                /* Mapping pucBuffer to a DNSAnswerRecord allows easy access of the
                 * fields of the structure. */

                /* MISRA Ref 11.3.1 [Misaligned access] */
                /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
                /* coverity[misra_c_2012_rule_11_3_violation] */
                pxDNSAnswerRecord = ( ( DNSAnswerRecord_t * ) pxSet->pucByte );

                /* Sanity check the data length of an IPv4 answer. */
                if( FreeRTOS_ntohs( pxDNSAnswerRecord->usDataLength ) ==
                    ( uint16_t ) pxSet->uxAddressLength )
                {
                    if( pxSet->uxAddressLength == ipSIZE_OF_IPv6_ADDRESS ) /*No check needed for pxSet->usType as uxAddressLength is set based on usType*/
                    {
                        ( void ) memcpy( xIP_Address.xIPAddress.xIP_IPv6.ucBytes,
                                         &( pxSet->pucByte[ sizeof( DNSAnswerRecord_t ) ] ),
                                         ipSIZE_OF_IPv6_ADDRESS );

                        if( ppxAddressInfo != NULL )
                        {
                            pxNewAddress = pxNew_AddrInfo( pxSet->pcName, FREERTOS_AF_INET6, xIP_Address.xIPAddress.xIP_IPv6.ucBytes );
                        }

                        xIP_Address.xIs_IPv6 = pdTRUE;

                        /* Return non-zero to inform the caller that a valid
                         * IPv6 address was found. */
                        pxSet->ulIPAddress = 1U;
                    }
                    else
                    {
                        void * pvCopyDest;
                        const void * pvCopySource;

                        /* Copy the IP address out of the record. Using different pointers
                         * to copy only the portion we want is intentional here. */

                        /*
                         * Use helper variables for memcpy() to remain
                         * compliant with MISRA Rule 21.15.  These should be
                         * optimized away.
                         */
                        pvCopySource = &( pxSet->pucByte[ sizeof( DNSAnswerRecord_t ) ] );
                        pvCopyDest = &( pxSet->ulIPAddress );
                        ( void ) memcpy( pvCopyDest, pvCopySource, pxSet->uxAddressLength );

                        if( ppxAddressInfo != NULL )
                        {
                            const uint8_t * ucBytes = ( uint8_t * ) &( pxSet->ulIPAddress );

                            pxNewAddress = pxNew_AddrInfo( pxSet->pcName, FREERTOS_AF_INET4, ucBytes );
                        }

                        xIP_Address.xIPAddress.ulIP_IPv4 = pxSet->ulIPAddress;
                        xIP_Address.xIs_IPv6 = pdFALSE;
                    }

                    if( pxNewAddress != NULL )
                    {
                        if( *( ppxAddressInfo ) == NULL )
                        {
                            /* For the first address found. */
                            *( ppxAddressInfo ) = pxNewAddress;
                        }
                        else
                        {
                            /* For the next address found. */
                            *( pxSet->ppxLastAddress ) = pxNewAddress;
                        }

                        pxSet->ppxLastAddress = &( pxNewAddress->ai_next );
                    }

                    #if ( ipconfigDNS_USE_CALLBACKS == 1 )
                    {
                        BaseType_t xCallbackResult;

                        xCallbackResult = xDNSDoCallback( pxSet, ( ppxAddressInfo != NULL ) ? *( ppxAddressInfo ) : NULL );

                        /* See if any asynchronous call was made to FreeRTOS_gethostbyname_a() */
                        if( xCallbackResult != pdFALSE )
                        {
                            /* This device has requested this DNS look-up.
                             * The result may be stored in the DNS cache. */
                            pxSet->xDoStore = pdTRUE;
                        }
                    }
                    #endif /* ipconfigDNS_USE_CALLBACKS == 1 */
                    #if ( ipconfigUSE_DNS_CACHE == 1 )
                    {
                        char cBuffer[ 40 ];

                        /* The reply will only be stored in the DNS cache when the
                         * request was issued by this device. */
                        if( pxSet->xDoStore != pdFALSE )
                        {
                            ( void ) FreeRTOS_dns_update(
                                pxSet->pcName,
                                &xIP_Address,
                                pxDNSAnswerRecord->ulTTL,
                                pdFALSE,
                                NULL );
                            pxSet->usNumARecordsStored++; /* Track # of A records stored */
                        }

                        if( pxSet->usType == ( uint16_t ) dnsTYPE_AAAA_HOST )
                        {
                            ( void ) FreeRTOS_inet_ntop( FREERTOS_AF_INET6, ( const void * ) xIP_Address.xIPAddress.xIP_IPv6.ucBytes, cBuffer, sizeof( cBuffer ) );
                            FreeRTOS_printf( ( "DNS[0x%04X]: The answer to '%s' (%s) will%s be stored\n",
                                               ( unsigned ) pxSet->pxDNSMessageHeader->usIdentifier,
                                               pxSet->pcName,
                                               cBuffer,
                                               ( pxSet->xDoStore != 0 ) ? "" : " NOT" ) );
                        }
                        else
                        {
                            ( void ) FreeRTOS_inet_ntop( FREERTOS_AF_INET,
                                                         ( const void * ) &( pxSet->ulIPAddress ),
                                                         cBuffer,
                                                         ( socklen_t ) sizeof( cBuffer ) );
                            /* Show what has happened. */
                            FreeRTOS_printf( ( "DNS[0x%04X]: The answer to '%s' (%s) will%s be stored\n",
                                               pxSet->pxDNSMessageHeader->usIdentifier,
                                               pxSet->pcName,
                                               cBuffer,
                                               ( pxSet->xDoStore != 0 ) ? "" : " NOT" ) );
                        }
                    }
                    #endif /* ipconfigUSE_DNS_CACHE */

                    if( ulReturnIPAddress == 0U )
                    {
                        /* Here pxSet->ulIPAddress should be not equal tp 0 since pxSet->ulIPAddress is copied from
                         * pxSet->pucByte[ sizeof( DNSAnswerRecord_t ) ] and os verified to be non zero above. */
                        /* Remember the first IP-address that is found. */
                        ulReturnIPAddress = pxSet->ulIPAddress;
                    }
                }
                else
                {
                    FreeRTOS_printf( ( "DNS sanity check failed: %u != %u\n",
                                       FreeRTOS_ntohs( pxDNSAnswerRecord->usDataLength ),
                                       ( unsigned ) pxSet->uxAddressLength ) );
                }

                pxSet->pucByte = &( pxSet->pucByte[ sizeof( DNSAnswerRecord_t ) + pxSet->uxAddressLength ] );
                pxSet->uxSourceBytesRemaining -= ( sizeof( DNSAnswerRecord_t ) + pxSet->uxAddressLength );
            }
            else if( pxSet->uxSourceBytesRemaining >= sizeof( DNSAnswerRecord_t ) )
            {
                uint16_t usDataLength;

                /* It's not an A record, so skip it. Get the header location
                 * and then jump over the header. */
                /* Cast the response to DNSAnswerRecord for easy access to fields of the DNS response. */

                /* MISRA Ref 11.3.1 [Misaligned access] */
                /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
                /* coverity[misra_c_2012_rule_11_3_violation] */
                pxDNSAnswerRecord = ( ( DNSAnswerRecord_t * ) pxSet->pucByte );

                pxSet->pucByte = &( pxSet->pucByte[ sizeof( DNSAnswerRecord_t ) ] );
                pxSet->uxSourceBytesRemaining -= sizeof( DNSAnswerRecord_t );

                /* Determine the length of the answer data from the header. */
                usDataLength = FreeRTOS_ntohs( pxDNSAnswerRecord->usDataLength );

                /* Jump over the answer. */
                if( pxSet->uxSourceBytesRemaining >= usDataLength )
                {
                    pxSet->pucByte = &( pxSet->pucByte[ usDataLength ] );
                    pxSet->uxSourceBytesRemaining -= usDataLength;
                }
                else
                {
                    /* Malformed response. */
                    xReturn = pdFALSE;
                    break;
                }
            }
            else
            {
                /* Do nothing */
            }
        }

        return ( xReturn != 0 ) ? ulReturnIPAddress : 0U;
    }

    #if ( ( ipconfigUSE_MDNS == 1 ) || ( ipconfigUSE_LLMNR == 1 ) || ( ipconfigUSE_NBNS == 1 ) )

/**
 * @brief Send a DNS message to be used in MDNS, LLMNR or NBNS.
 *
 * @param[in] pxNetworkBuffer The network buffer descriptor with the DNS message.
 * @param[in] lNetLength The length of the DNS message.
 */
        void prepareReplyDNSMessage( NetworkBufferDescriptor_t * pxNetworkBuffer,
                                     BaseType_t lNetLength )
        {
            UDPPacket_t * pxUDPPacket;
            IPHeader_t * pxIPHeader;
            UDPHeader_t * pxUDPHeader;
            size_t uxDataLength;
            NetworkEndPoint_t * pxEndPoint = pxNetworkBuffer->pxEndPoint;
            const size_t uxIPHeaderLength = uxIPHeaderSizePacket( pxNetworkBuffer );

            configASSERT( pxEndPoint != NULL );
            pxUDPPacket = ( ( UDPPacket_t * )
                            pxNetworkBuffer->pucEthernetBuffer );
            pxIPHeader = &pxUDPPacket->xIPHeader;

            #if ( ipconfigUSE_IPv6 != 0 )
                if( ( ( uxIPHeaderLength == ipSIZE_OF_IPv6_HEADER ) && ( ( pxIPHeader->ucVersionHeaderLength & 0xf0U ) == 0x60U ) ) )
                {
                    UDPPacket_IPv6_t * xUDPPacket_IPv6;
                    IPHeader_IPv6_t * pxIPHeader_IPv6;

                    xUDPPacket_IPv6 = ( ( UDPPacket_IPv6_t * ) pxNetworkBuffer->pucEthernetBuffer );
                    pxIPHeader_IPv6 = &( xUDPPacket_IPv6->xIPHeader );
                    pxUDPHeader = &xUDPPacket_IPv6->xUDPHeader;

                    pxIPHeader_IPv6->usPayloadLength = FreeRTOS_htons( ( uint16_t ) lNetLength + ipSIZE_OF_UDP_HEADER );

                    {
                        ( void ) memcpy( pxIPHeader_IPv6->xDestinationAddress.ucBytes, pxIPHeader_IPv6->xSourceAddress.ucBytes, ipSIZE_OF_IPv6_ADDRESS );
                        ( void ) memcpy( pxIPHeader_IPv6->xSourceAddress.ucBytes, pxEndPoint->ipv6_settings.xIPAddress.ucBytes, ipSIZE_OF_IPv6_ADDRESS );
                    }

                    xUDPPacket_IPv6->xUDPHeader.usLength = FreeRTOS_htons( ( uint16_t ) lNetLength + ipSIZE_OF_UDP_HEADER );
                    vFlip_16( pxUDPHeader->usSourcePort, pxUDPHeader->usDestinationPort );
                    uxDataLength = ( size_t ) lNetLength + ipSIZE_OF_IPv6_HEADER + ipSIZE_OF_UDP_HEADER + ipSIZE_OF_ETH_HEADER;
                }
                else
            #endif /* ( ipconfigUSE_IPv6 != 0 ) */
            {
                pxUDPHeader = &pxUDPPacket->xUDPHeader;
                /* HT: started using defines like 'ipSIZE_OF_xxx' */
                pxIPHeader->usLength = FreeRTOS_htons( ( uint16_t ) lNetLength +
                                                       uxIPHeaderLength +
                                                       ipSIZE_OF_UDP_HEADER );

                /* HT:endian: should not be translated, copying from packet to packet */
                if( pxIPHeader->ulDestinationIPAddress == ipMDNS_IP_ADDRESS )
                {
                    pxIPHeader->ucTimeToLive = ipMDNS_TIME_TO_LIVE;
                }
                else
                {
                    pxIPHeader->ulDestinationIPAddress = pxIPHeader->ulSourceIPAddress;
                    pxIPHeader->ucTimeToLive = ipconfigUDP_TIME_TO_LIVE;
                }

                pxIPHeader->ulSourceIPAddress = pxEndPoint->ipv4_settings.ulIPAddress;
                pxIPHeader->usIdentification = FreeRTOS_htons( usPacketIdentifier );

                /* The stack doesn't support fragments, so the fragment offset field must always be zero.
                 * The header was never memset to zero, so set both the fragment offset and fragmentation flags in one go.
                 */
                #if ( ipconfigFORCE_IP_DONT_FRAGMENT != 0 )
                    pxIPHeader->usFragmentOffset = ipFRAGMENT_FLAGS_DONT_FRAGMENT;
                #else
                    pxIPHeader->usFragmentOffset = 0U;
                #endif
                usPacketIdentifier++;
                pxUDPHeader->usLength = FreeRTOS_htons( ( uint32_t ) lNetLength +
                                                        ipSIZE_OF_UDP_HEADER );
                vFlip_16( pxUDPHeader->usSourcePort, pxUDPHeader->usDestinationPort );

                /* Important: tell NIC driver how many bytes must be sent */
                uxDataLength = ( ( size_t ) lNetLength ) + uxIPHeaderLength + ipSIZE_OF_UDP_HEADER + ipSIZE_OF_ETH_HEADER;
            }

            #if ( ipconfigDRIVER_INCLUDED_TX_IP_CHECKSUM == 0 )
            {
                #if ( ipconfigUSE_IPv6 != 0 )
                    /* IPv6 IP-headers have no checksum field. */
                    if( ( pxIPHeader->ucVersionHeaderLength & 0xf0U ) != 0x60U )
                #endif
                {
                    /* Calculate the IP header checksum. */
                    pxIPHeader->usHeaderChecksum = 0U;
                    pxIPHeader->usHeaderChecksum = usGenerateChecksum( 0U, ( uint8_t * ) &( pxIPHeader->ucVersionHeaderLength ), uxIPHeaderLength );
                    pxIPHeader->usHeaderChecksum = ( uint16_t ) ~FreeRTOS_htons( pxIPHeader->usHeaderChecksum );
                }

                /* calculate the UDP checksum for outgoing package */
                ( void ) usGenerateProtocolChecksum( ( uint8_t * ) pxUDPPacket, uxDataLength, pdTRUE );
            }
            #endif /* if ( ipconfigDRIVER_INCLUDED_TX_IP_CHECKSUM == 0 ) */

            /* Important: tell NIC driver how many bytes must be sent */
            pxNetworkBuffer->xDataLength = uxDataLength;
        }

    #endif /* ( ipconfigUSE_MDNS == 1 ) || ( ipconfigUSE_LLMNR == 1 ) || ( ipconfigUSE_NBNS == 1 ) */

    #if ( ipconfigUSE_NBNS == 1 )

/**
 * @brief Respond to an NBNS query or an NBNS reply.
 *
 * @param[in] pucPayload the UDP payload of the NBNS message.
 * @param[in] uxBufferLength Length of the Buffer.
 * @param[in] ulIPAddress IP address of the sender.
 */
        void DNS_TreatNBNS( uint8_t * pucPayload,
                            size_t uxBufferLength,
                            uint32_t ulIPAddress )
        {
            uint16_t usFlags, usType, usClass;
            uint8_t * pucSource, * pucTarget;
            uint8_t ucByte;
            uint8_t ucNBNSName[ 17 ];
            uint8_t * pucUDPPayloadBuffer = pucPayload;
            size_t uxSizeNeeded;
            NetworkBufferDescriptor_t * pxNetworkBuffer;
            size_t uxBytesNeeded = sizeof( UDPPacket_t ) + sizeof( NBNSRequest_t );
            BaseType_t xDNSHookReturn;
            uint16_t usLength;
            DNSMessage_t * pxMessage;
            NBNSAnswer_t * pxAnswer;

            /* Introduce a do {} while (0) loop to allow the use of breaks. */
            do
            {
                NetworkEndPoint_t xEndPoint;

                /* Check for minimum buffer size: 92 bytes. */
                if( uxBufferLength < uxBytesNeeded )
                {
                    break;
                }

                /* Is a valid payload/network buffer provided? */
                if( pucUDPPayloadBuffer == NULL )
                {
                    break;
                }

                pxNetworkBuffer = pxUDPPayloadBuffer_to_NetworkBuffer( pucUDPPayloadBuffer );

                if( pxNetworkBuffer == NULL )
                {
                    break;
                }

                /* Read the request flags in host endianness. */
                usFlags = usChar2u16( &( pucUDPPayloadBuffer[ offsetof( NBNSRequest_t, usFlags ) ] ) );

                if( ( usFlags & dnsNBNS_FLAGS_OPCODE_MASK ) != dnsNBNS_FLAGS_OPCODE_QUERY )
                {
                    /* No need to answer, this is not a query. */
                    break;
                }

                usType = usChar2u16( &( pucUDPPayloadBuffer[ offsetof( NBNSRequest_t, usType ) ] ) );
                usClass = usChar2u16( &( pucUDPPayloadBuffer[ offsetof( NBNSRequest_t, usClass ) ] ) );

                /* Not used for now */
                ( void ) usClass;

                /* For NBNS a name is 16 bytes long, written with capitals only.
                 * Make sure that the copy is terminated with a zero. */
                pucTarget = &( ucNBNSName[ sizeof( ucNBNSName ) - 2U ] );
                pucTarget[ 1 ] = ( uint8_t ) 0U;

                /* Start with decoding the last 2 bytes. */
                pucSource = &( pucUDPPayloadBuffer[ ( dnsNBNS_ENCODED_NAME_LENGTH - 2 ) +
                                                    offsetof( NBNSRequest_t, ucName ) ] );

                for( ; ; )
                {
                    /* Define the ASCII value of the capital "A". */
                    const uint8_t ucCharA = ( uint8_t ) 0x41U;

                    ucByte = ( uint8_t ) ( ( ( ( pucSource[ 0 ] - ucCharA ) & 0x0F ) << 4 ) |
                                           ( pucSource[ 1 ] - ucCharA ) );

                    /* Make sure there are no trailing spaces in the name. */
                    if( ( ucByte == ( uint8_t ) ' ' ) && ( pucTarget[ 1 ] == 0U ) )
                    {
                        ucByte = 0U;
                    }

                    *pucTarget = ucByte;

                    if( pucTarget == ucNBNSName )
                    {
                        break;
                    }

                    pucTarget -= 1;
                    pucSource -= 2;
                }

                #if ( ipconfigUSE_DNS_CACHE == 1 )
                {
                    if( ( usFlags & dnsNBNS_FLAGS_RESPONSE ) != 0U )
                    {
                        /* If this is a response from another device,
                         * add the name to the DNS cache */
                        IPv46_Address_t xIPAddress;

                        xIPAddress.xIPAddress.ulIP_IPv4 = ulIPAddress;
                        #if ( ipconfigUSE_IPv6 != 0 )
                        {
                            xIPAddress.xIs_IPv6 = pdFALSE;
                        }
                        #endif

                        ( void ) FreeRTOS_dns_update( ( char * ) ucNBNSName, &( xIPAddress ), 0, pdFALSE, NULL );
                    }
                }
                #else /* if ( ipconfigUSE_DNS_CACHE == 1 ) */
                {
                    /* Avoid compiler warnings. */
                    ( void ) ulIPAddress;
                }
                #endif /* ipconfigUSE_DNS_CACHE */

                if( ( usType != dnsNBNS_TYPE_NET_BIOS ) ||
                    ( ( usFlags & dnsNBNS_FLAGS_RESPONSE ) != 0U ) )
                {
                    /* The request is not for NBNS, or the response flag is set. */
                    break;
                }

                /* When pxUDPPayloadBuffer_to_NetworkBuffer fails, there
                 * is a real problem, like data corruption. */
                if( pxNetworkBuffer->pxEndPoint == NULL )
                {
                    /* Should have been asserted earlier in the call tree. */
                    break;
                }

                ( void ) memcpy( &xEndPoint, pxNetworkBuffer->pxEndPoint, sizeof( xEndPoint ) );

                /* NBNS only handles IPv4 or "A" records. */
                xEndPoint.bits.bIPv6 = pdFALSE_UNSIGNED;
                xEndPoint.usDNSType = dnsTYPE_A_HOST;

                #if ( ipconfigIPv4_BACKWARD_COMPATIBLE == 1 )
                    xDNSHookReturn = xApplicationDNSQueryHook( ( const char * ) ucNBNSName );
                #else
                    xDNSHookReturn = xApplicationDNSQueryHook_Multi( &( xEndPoint ), ( const char * ) ucNBNSName );
                #endif

                if( xDNSHookReturn == pdFALSE )
                {
                    /* The application informs that the name in 'ucNBNSName'
                     * does not refer to this host. */
                    break;
                }

                /* During the early stages of boot or after a DHCP lease expires, our end-point
                 * may have an IP address of 0.0.0.0. Do not respond to name queries with that address. */
                if( xEndPoint.ipv4_settings.ulIPAddress == 0U )
                {
                    break;
                }

                /* Someone is looking for a device with ucNBNSName,
                 * prepare a positive reply.
                 * The reply will be a bit longer than the request, so make some space.
                 * NBNSAnswer_t will be added, minus the two shorts 'usType' and 'usClass'
                 * that were already present. */
                uxSizeNeeded = pxNetworkBuffer->xDataLength + sizeof( NBNSAnswer_t ) - 2 * sizeof( uint16_t );

                if( xBufferAllocFixedSize == pdFALSE )
                {
                    /* We're linked with BufferAllocation_2.c
                     * pxResizeNetworkBufferWithDescriptor() will malloc a new bigger buffer,
                     * and memcpy the data. The old buffer will be free'd.
                     */
                    NetworkBufferDescriptor_t * pxNewBuffer = pxResizeNetworkBufferWithDescriptor( pxNetworkBuffer, uxSizeNeeded );

                    if( pxNewBuffer == NULL )
                    {
                        break;
                    }

                    /* pxNewBuffer and pxNetworkBuffer are now the same pointers.
                     * Only pucEthernetBuffer has been renewed. */
                    pxNetworkBuffer->xDataLength = uxSizeNeeded;
                    pucUDPPayloadBuffer = &( pxNetworkBuffer->pucEthernetBuffer[ ipUDP_PAYLOAD_OFFSET_IPv4 ] );
                }
                else
                {
                    /* BufferAllocation_1.c is used, the Network Buffers can contain at least
                     * ipconfigNETWORK_MTU + ipSIZE_OF_ETH_HEADER. */
                    if( uxSizeNeeded > ( ipconfigNETWORK_MTU + ipSIZE_OF_ETH_HEADER ) )
                    {
                        /* The buffer is too small to reply. Drop silently. */
                        break;
                    }
                }

                pxNetworkBuffer->xDataLength = uxSizeNeeded;

                pxMessage = ( ( DNSMessage_t * ) pucUDPPayloadBuffer );

                /* As the fields in the structures are not word-aligned, we have to
                 * copy the values byte-by-byte using macro's vSetField16() and vSetField32() */
                vSetField16( pxMessage, DNSMessage_t, usFlags, dnsNBNS_QUERY_RESPONSE_FLAGS ); /* 0x8500 */
                vSetField16( pxMessage, DNSMessage_t, usQuestions, 0 );
                vSetField16( pxMessage, DNSMessage_t, usAnswers, 1 );
                vSetField16( pxMessage, DNSMessage_t, usAuthorityRRs, 0 );
                vSetField16( pxMessage, DNSMessage_t, usAdditionalRRs, 0 );

                pxAnswer = ( ( NBNSAnswer_t * ) &( pucUDPPayloadBuffer[ offsetof( NBNSRequest_t, usType ) ] ) );

                vSetField16( pxAnswer, NBNSAnswer_t, usType, usType );            /* Type */
                vSetField16( pxAnswer, NBNSAnswer_t, usClass, dnsNBNS_CLASS_IN ); /* Class */
                vSetField32( pxAnswer, NBNSAnswer_t, ulTTL, dnsNBNS_TTL_VALUE );
                vSetField16( pxAnswer, NBNSAnswer_t, usDataLength, 6 );           /* 6 bytes including the length field */
                vSetField16( pxAnswer, NBNSAnswer_t, usNbFlags, dnsNBNS_NAME_FLAGS );
                /* The function vSetField32() expects host-endian values, that is why ntohl() is called. */
                vSetField32( pxAnswer, NBNSAnswer_t, ulIPAddress, FreeRTOS_ntohl( xEndPoint.ipv4_settings.ulIPAddress ) );

                usLength = ( uint16_t ) ( sizeof( NBNSAnswer_t ) + ( size_t ) offsetof( NBNSRequest_t, usType ) );

                prepareReplyDNSMessage( pxNetworkBuffer, ( BaseType_t ) usLength );

                /* This function will fill in the eth addresses and send the packet */
                vReturnEthernetFrame( pxNetworkBuffer, pdFALSE );

                /*pxNewBuffer and pxNetworkBuffer are now the same pointers.pxNetworkBuffer will be released elsewhere.
                 * so pxNewBuffer does not need to released, since they share a single memory location*/
            }  while( ipFALSE_BOOL );
        }
    #endif /* ( ipconfigUSE_NBNS == 1 ) */

#endif /* ipconfigUSE_DNS != 0 */
