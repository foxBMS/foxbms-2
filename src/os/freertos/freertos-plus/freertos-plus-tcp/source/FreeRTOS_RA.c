/*
 * FreeRTOS+TCP V4.2.1
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
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
 * @file FreeRTOS_RA.c
 * @brief A client implementation of Router advertisement protocol.
 */

/* Standard includes. */
#include <stdint.h>
#include <stdio.h>


/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"

/* FreeRTOS+TCP includes. */
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"
#include "FreeRTOS_IP_Private.h"
#include "FreeRTOS_IP_Timers.h"
#include "FreeRTOS_ARP.h"
#include "FreeRTOS_UDP_IP.h"
#include "FreeRTOS_Routing.h"
#include "FreeRTOS_ND.h"
#if ( ipconfigUSE_LLMNR == 1 )
    #include "FreeRTOS_DNS.h"
#endif /* ipconfigUSE_LLMNR */
#include "NetworkBufferManagement.h"

/* This define may exclude the entire source file. */
#if ( ipconfigUSE_IPv6 != 0 ) && ( ipconfigUSE_RA != 0 )

/*-----------------------------------------------------------*/

/** A block time of 0 simply means "don't block". */
    #define raDONT_BLOCK                       ( ( TickType_t ) 0 )

/** The default value for the IPv6-field 'ucVersionTrafficClass'. */
    #define raDEFAULT_VERSION_TRAFFIC_CLASS    0x60U

/** The default value for the IPv6-field 'ucHopLimit'. */
    #define raDEFAULT_HOP_LIMIT                255U

/*-----------------------------------------------------------*/

/* Initialise the Router Advertisement process for a given end-point. */
    static void vRAProcessInit( NetworkEndPoint_t * pxEndPoint );

/* Find a link-local address that is bound to a given interface. */
    static BaseType_t xGetLinkLocalAddress( const NetworkInterface_t * pxInterface,
                                            IPv6_Address_t * pxAddress );

/* Read the reply received from the RA server. */
    static ICMPPrefixOption_IPv6_t * vReceiveRA_ReadReply( const NetworkBufferDescriptor_t * pxNetworkBuffer );

/* Handle the states that are limited by a timer. See if any of the timers has expired. */
    static TickType_t xRAProcess_HandleWaitStates( NetworkEndPoint_t * pxEndPoint,
                                                   TickType_t uxReloadTime );

/* Handle the other states. */
    static TickType_t xRAProcess_HandleOtherStates( NetworkEndPoint_t * pxEndPoint,
                                                    TickType_t uxReloadTime );


/*-----------------------------------------------------------*/

/**
 * @brief Find a link-local address that is bound to a given interface.
 *
 * @param[in] pxInterface The interface for which a link-local address is looked up.
 * @param[out] pxAddress The IP address will be copied to this parameter.
 *
 * @return pdPASS in case a link-local address was found, otherwise pdFAIL.
 */
    static BaseType_t xGetLinkLocalAddress( const NetworkInterface_t * pxInterface,
                                            IPv6_Address_t * pxAddress )
    {
        BaseType_t xResult = pdFAIL;
        NetworkEndPoint_t * pxEndPoint;

        for( pxEndPoint = FreeRTOS_FirstEndPoint( pxInterface );
             pxEndPoint != NULL;
             pxEndPoint = FreeRTOS_NextEndPoint( pxInterface, pxEndPoint ) )
        {
            /* Check if it has the link-local prefix FE80::/10 */
            if( ( pxEndPoint->ipv6_settings.xIPAddress.ucBytes[ 0 ] == 0xfeU ) &&
                ( ( pxEndPoint->ipv6_settings.xIPAddress.ucBytes[ 1 ] & 0xc0U ) == 0x80U ) )
            {
                ( void ) memcpy( pxAddress->ucBytes, pxEndPoint->ipv6_settings.xIPAddress.ucBytes, ipSIZE_OF_IPv6_ADDRESS );
                xResult = pdPASS;
                break;
            }
        }

        return xResult;
    }
/*-----------------------------------------------------------*/

/**
 * @brief Send an ICMPv6 message of the type: Router Solicitation.
 *
 * @param[in] pxNetworkBuffer The network buffer which can be used for this.
 * @param[in] pxIPAddress The target address, normally ff02::2
 *
 */
    void vNDSendRouterSolicitation( NetworkBufferDescriptor_t * pxNetworkBuffer,
                                    IPv6_Address_t * pxIPAddress )
    {
        ICMPPacket_IPv6_t * pxICMPPacket;
        ICMPRouterSolicitation_IPv6_t * xRASolicitationRequest;
        const NetworkEndPoint_t * pxEndPoint = pxNetworkBuffer->pxEndPoint;
        const size_t uxNeededSize = ipSIZE_OF_ETH_HEADER + ipSIZE_OF_IPv6_HEADER + sizeof( ICMPRouterSolicitation_IPv6_t );
        MACAddress_t xMultiCastMacAddress;
        NetworkBufferDescriptor_t * pxDescriptor = pxNetworkBuffer;
        IPv6_Address_t xSourceAddress;
        BaseType_t xHasLocal;
        NetworkBufferDescriptor_t * pxNewDescriptor = NULL;

        configASSERT( pxEndPoint != NULL );
        configASSERT( pxEndPoint->bits.bIPv6 != pdFALSE_UNSIGNED );

        xHasLocal = xGetLinkLocalAddress( pxEndPoint->pxNetworkInterface, &( xSourceAddress ) );

        if( xHasLocal == pdFAIL )
        {
            FreeRTOS_printf( ( "RA: can not find a Link-local address\n" ) );
            ( void ) memset( xSourceAddress.ucBytes, 0, ipSIZE_OF_IPv6_ADDRESS );
        }
        else
        {
            FreeRTOS_printf( ( "RA: source %pip\n", ( void * ) xSourceAddress.ucBytes ) );
        }

        if( pxDescriptor->xDataLength < uxNeededSize )
        {
            pxNewDescriptor = pxDuplicateNetworkBufferWithDescriptor( pxDescriptor, uxNeededSize );
            vReleaseNetworkBufferAndDescriptor( pxDescriptor );
            pxDescriptor = pxNewDescriptor;
        }

        if( pxDescriptor != NULL )
        {
            /* MISRA Ref 11.3.1 [Misaligned access] */
            /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
            /* coverity[misra_c_2012_rule_11_3_violation] */
            pxICMPPacket = ( ( ICMPPacket_IPv6_t * ) pxDescriptor->pucEthernetBuffer );

            /* MISRA Ref 11.3.1 [Misaligned access] */
            /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
            /* coverity[misra_c_2012_rule_11_3_violation] */
            xRASolicitationRequest = ( ( ICMPRouterSolicitation_IPv6_t * ) &( pxICMPPacket->xICMPHeaderIPv6 ) );

            pxDescriptor->xDataLength = uxNeededSize;

            ( void ) eNDGetCacheEntry( pxIPAddress, &( xMultiCastMacAddress ), NULL );

            /* Set Ethernet header. Will be swapped. */
            ( void ) memcpy( pxICMPPacket->xEthernetHeader.xSourceAddress.ucBytes, xMultiCastMacAddress.ucBytes, ipMAC_ADDRESS_LENGTH_BYTES );
            ( void ) memcpy( pxICMPPacket->xEthernetHeader.xDestinationAddress.ucBytes, pxEndPoint->xMACAddress.ucBytes, ipMAC_ADDRESS_LENGTH_BYTES );
            pxICMPPacket->xEthernetHeader.usFrameType = ipIPv6_FRAME_TYPE;

            /* Set IP-header. */
            pxICMPPacket->xIPHeader.ucVersionTrafficClass = raDEFAULT_VERSION_TRAFFIC_CLASS;
            pxICMPPacket->xIPHeader.ucTrafficClassFlow = 0U;
            pxICMPPacket->xIPHeader.usFlowLabel = 0U;
            pxICMPPacket->xIPHeader.usPayloadLength = FreeRTOS_htons( sizeof( ICMPRouterSolicitation_IPv6_t ) );
            pxICMPPacket->xIPHeader.ucNextHeader = ipPROTOCOL_ICMP_IPv6;
            pxICMPPacket->xIPHeader.ucHopLimit = raDEFAULT_HOP_LIMIT;

            /* Normally, the source address is set as 'ipv6_settings.xIPAddress'.
             * But is some routers will not accept a public IP-address, the original
             * default address will be used. It must be a link-local address. */
            ( void ) memcpy( pxICMPPacket->xIPHeader.xSourceAddress.ucBytes, xSourceAddress.ucBytes, ipSIZE_OF_IPv6_ADDRESS );

            ( void ) memcpy( pxICMPPacket->xIPHeader.xDestinationAddress.ucBytes, pxIPAddress->ucBytes, ipSIZE_OF_IPv6_ADDRESS );

            /* Set ICMP header. */
            ( void ) memset( xRASolicitationRequest, 0, sizeof( *xRASolicitationRequest ) );
            xRASolicitationRequest->ucTypeOfMessage = ipICMP_ROUTER_SOLICITATION_IPv6;

            /*  __XX__ revisit on why commented out
             *  xRASolicitationRequest->ucOptionType = ndICMP_SOURCE_LINK_LAYER_ADDRESS;
             *  xRASolicitationRequest->ucOptionLength = 1;
             *  ( void ) memcpy( xRASolicitationRequest->ucOptionBytes, pxEndPoint->xMACAddress.ucBytes, ipMAC_ADDRESS_LENGTH_BYTES );
             */

            /* Checksums. */
            #if ( ipconfigDRIVER_INCLUDED_TX_IP_CHECKSUM == 0 )
            {
                /* calculate the ICMPv6 checksum for outgoing package */
                ( void ) usGenerateProtocolChecksum( pxDescriptor->pucEthernetBuffer, pxDescriptor->xDataLength, pdTRUE );
            }
            #else
            {
                /* Many EMAC peripherals will only calculate the ICMP checksum
                 * correctly if the field is nulled beforehand. */
                xRASolicitationRequest->usChecksum = 0U;
            }
            #endif

            /* This function will fill in the eth addresses and send the packet */
            vReturnEthernetFrame( pxDescriptor, pdTRUE );
        }
    }
/*-----------------------------------------------------------*/

/**
 * @brief Receive a NA ( Neighbour Advertisement ) message to see if a chosen IP-address is already in use.
 *
 * @param[in] pxNetworkBuffer The buffer that contains the message.
 */
    void vReceiveNA( const NetworkBufferDescriptor_t * pxNetworkBuffer )
    {
        const NetworkInterface_t * pxInterface = pxNetworkBuffer->pxInterface;
        NetworkEndPoint_t * pxPoint;

        /* MISRA Ref 11.3.1 [Misaligned access] */
        /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
        /* coverity[misra_c_2012_rule_11_3_violation] */
        const ICMPPacket_IPv6_t * pxICMPPacket = ( ( const ICMPPacket_IPv6_t * ) pxNetworkBuffer->pucEthernetBuffer );
        const ICMPHeader_IPv6_t * pxICMPHeader_IPv6 = ( ( const ICMPHeader_IPv6_t * ) &( pxICMPPacket->xICMPHeaderIPv6 ) );

        for( pxPoint = FreeRTOS_FirstEndPoint( pxInterface );
             pxPoint != NULL;
             pxPoint = FreeRTOS_NextEndPoint( pxInterface, pxPoint ) )
        {
            if( ( pxPoint->bits.bWantRA != pdFALSE_UNSIGNED ) && ( pxPoint->xRAData.eRAState == eRAStateIPWait ) )
            {
                if( memcmp( pxPoint->ipv6_settings.xIPAddress.ucBytes, pxICMPHeader_IPv6->xIPv6Address.ucBytes, ipSIZE_OF_IPv6_ADDRESS ) == 0 )
                {
                    pxPoint->xRAData.bits.bIPAddressInUse = pdTRUE_UNSIGNED;
                    vDHCP_RATimerReload( pxPoint, 100U );
                }
            }
        }
    }
/*-----------------------------------------------------------*/

/**
 * @brief Read a received RA reply and return the prefix option from the packet.
 *
 * @param[in] pxNetworkBuffer The buffer that contains the message.
 *
 * @returns Returns the ICMP prefix option pointer, pointing to its location in the
 *          input RA reply message buffer.
 */
    static ICMPPrefixOption_IPv6_t * vReceiveRA_ReadReply( const NetworkBufferDescriptor_t * pxNetworkBuffer )
    {
        size_t uxIndex = 0U;
        const size_t uxICMPSize = sizeof( ICMPRouterAdvertisement_IPv6_t );
        const size_t uxNeededSize = ipSIZE_OF_ETH_HEADER + ipSIZE_OF_IPv6_HEADER + uxICMPSize;
        /* uxLast points to the first byte after the buffer. */
        const size_t uxLast = pxNetworkBuffer->xDataLength - uxNeededSize;
        uint8_t * pucBytes = &( pxNetworkBuffer->pucEthernetBuffer[ uxNeededSize ] );
        ICMPPrefixOption_IPv6_t * pxPrefixOption = NULL;

        while( ( uxIndex + 1U ) < uxLast )
        {
            uint8_t ucType = pucBytes[ uxIndex ];
            size_t uxPrefixLength = ( size_t ) pucBytes[ uxIndex + 1U ];
            size_t uxLength = uxPrefixLength * 8U;

            if( uxPrefixLength == 0U )
            {
                /* According to RFC 4861, length of the option value 0 is invalid. Hence returning from here */
                FreeRTOS_printf( ( "RA: Invalid length of the option value as zero. " ) );
                break;
            }

            if( uxLast < ( uxIndex + uxLength ) )
            {
                FreeRTOS_printf( ( "RA: Not enough bytes ( %u > %u )\n", ( unsigned ) ( uxIndex + uxLength ), ( unsigned ) uxLast ) );
                break;
            }

            switch( ucType )
            {
                case ndICMP_SOURCE_LINK_LAYER_ADDRESS: /* 1 */
                    FreeRTOS_printf( ( "RA: Source = %02x-%02x-%02x-%02x-%02x-%02x\n",
                                       pucBytes[ uxIndex + 2U ],
                                       pucBytes[ uxIndex + 3U ],
                                       pucBytes[ uxIndex + 4U ],
                                       pucBytes[ uxIndex + 5U ],
                                       pucBytes[ uxIndex + 6U ],
                                       pucBytes[ uxIndex + 7U ] ) );
                    break;

                case ndICMP_TARGET_LINK_LAYER_ADDRESS: /* 2 */
                    break;

                case ndICMP_PREFIX_INFORMATION: /* 3 */
                    /* MISRA Ref 11.3.1 [Misaligned access] */
                    /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
                    /* coverity[misra_c_2012_rule_11_3_violation] */
                    pxPrefixOption = ( ( ICMPPrefixOption_IPv6_t * ) &( pucBytes[ uxIndex ] ) );

                    FreeRTOS_printf( ( "RA: Prefix len %d Life %u, %u (%pip)\n",
                                       pxPrefixOption->ucPrefixLength,
                                       ( unsigned ) FreeRTOS_ntohl( pxPrefixOption->ulValidLifeTime ),
                                       ( unsigned ) FreeRTOS_ntohl( pxPrefixOption->ulPreferredLifeTime ),
                                       ( void * ) pxPrefixOption->ucPrefix ) );
                    break;

                case ndICMP_REDIRECTED_HEADER: /* 4 */
                    break;

                case ndICMP_MTU_OPTION: /* 5 */
                   {
                       uint32_t ulMTU = 0u;
                       ( void ) ulMTU;

                       /* ulChar2u32 returns host-endian numbers. */
                       ulMTU = ulChar2u32( &( pucBytes[ uxIndex + 4U ] ) );
                       FreeRTOS_printf( ( "RA: MTU = %u\n", ( unsigned int ) ulMTU ) );
                   }
                   break;

                default:
                    FreeRTOS_printf( ( "RA: Type 0x%02x not implemented\n", ucType ) );
                    break;
            }

            uxIndex = uxIndex + uxLength;
        } /* while( ( uxIndex + 1 ) < uxLast ) */

        return pxPrefixOption;
    }
/*-----------------------------------------------------------*/

/**
 * @brief Receive and analyse a RA ( Router Advertisement ) message.
 *        If the reply is satisfactory, the end-point will do SLAAC: choose an IP-address using the
 *        prefix offered, and completed with random bits.  It will start testing if another device
 *        already exists that uses the same IP-address.
 *
 * @param[in] pxNetworkBuffer The buffer that contains the message.
 */
    void vReceiveRA( const NetworkBufferDescriptor_t * pxNetworkBuffer )
    {
        /* MISRA Ref 11.3.1 [Misaligned access] */
        /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
        /* coverity[misra_c_2012_rule_11_3_violation] */
        const ICMPPacket_IPv6_t * pxICMPPacket = ( ( const ICMPPacket_IPv6_t * ) pxNetworkBuffer->pucEthernetBuffer );
        const ICMPPrefixOption_IPv6_t * pxPrefixOption = NULL;
        const size_t uxICMPSize = sizeof( ICMPRouterAdvertisement_IPv6_t );
        const size_t uxNeededSize = ipSIZE_OF_ETH_HEADER + ipSIZE_OF_IPv6_HEADER + uxICMPSize;

        /* A Router Advertisement was received, handle it here. */
        if( uxNeededSize > pxNetworkBuffer->xDataLength )
        {
            FreeRTOS_printf( ( "vReceiveRA: The buffer provided is too small\n" ) );
        }
        else
        {
            /* MISRA Ref 11.3.1 [Misaligned access] */
            /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
            /* coverity[misra_c_2012_rule_11_3_violation] */
            const ICMPRouterAdvertisement_IPv6_t * pxAdvertisement = ( ( const ICMPRouterAdvertisement_IPv6_t * ) &( pxICMPPacket->xICMPHeaderIPv6 ) );
            FreeRTOS_printf( ( "RA: Type %02x Srv %02x Checksum %04x Hops %d Flags %02x Life %d\n",
                               pxAdvertisement->ucTypeOfMessage,
                               pxAdvertisement->ucTypeOfService,
                               FreeRTOS_ntohs( pxAdvertisement->usChecksum ),
                               pxAdvertisement->ucHopLimit,
                               pxAdvertisement->ucFlags,
                               FreeRTOS_ntohs( pxAdvertisement->usLifetime ) ) );

            if( pxAdvertisement->usLifetime != 0U )
            {
                pxPrefixOption = vReceiveRA_ReadReply( pxNetworkBuffer );

                configASSERT( pxNetworkBuffer->pxInterface != NULL );

                if( pxPrefixOption != NULL )
                {
                    NetworkEndPoint_t * pxEndPoint;

                    for( pxEndPoint = FreeRTOS_FirstEndPoint( pxNetworkBuffer->pxInterface );
                         pxEndPoint != NULL;
                         pxEndPoint = FreeRTOS_NextEndPoint( pxNetworkBuffer->pxInterface, pxEndPoint ) )
                    {
                        if( ( pxEndPoint->bits.bWantRA != pdFALSE_UNSIGNED ) && ( pxEndPoint->xRAData.eRAState == eRAStateWait ) )
                        {
                            pxEndPoint->ipv6_settings.uxPrefixLength = pxPrefixOption->ucPrefixLength;
                            ( void ) memcpy( pxEndPoint->ipv6_settings.xPrefix.ucBytes, pxPrefixOption->ucPrefix, ipSIZE_OF_IPv6_ADDRESS );
                            ( void ) memcpy( pxEndPoint->ipv6_settings.xGatewayAddress.ucBytes, pxICMPPacket->xIPHeader.xSourceAddress.ucBytes, ipSIZE_OF_IPv6_ADDRESS );

                            pxEndPoint->xRAData.bits.bRouterReplied = pdTRUE_UNSIGNED;
                            pxEndPoint->xRAData.uxRetryCount = 0U;
                            pxEndPoint->xRAData.ulPreferredLifeTime = FreeRTOS_ntohl( pxPrefixOption->ulPreferredLifeTime );
                            /* Force taking a new random IP-address. */
                            pxEndPoint->xRAData.bits.bIPAddressInUse = pdTRUE_UNSIGNED;
                            pxEndPoint->xRAData.eRAState = eRAStateIPTest;
                            vRAProcess( pdFALSE, pxEndPoint );
                        }
                    }
                }
            }
            else
            {
                /* The life-time field contains zero. */
            }
        }
    }
/*-----------------------------------------------------------*/

/**
 * @brief Handles the RA wait state and calculates the new timer reload value
 *        based on the wait state. Also checks if any timer has expired. If its found that
 *        there is no other device using the same IP-address vIPNetworkUpCalls() is called
 *        to send the network up event.
 *
 * @param[in] pxEndPoint The end point for which RA assignment is required.
 * @param[out] uxReloadTime Timer reload value in ticks.
 *
 * @return New timer reload value.
 */
    static TickType_t xRAProcess_HandleWaitStates( NetworkEndPoint_t * pxEndPoint,
                                                   TickType_t uxReloadTime )
    {
        TickType_t uxNewReloadTime = uxReloadTime;

        if( pxEndPoint->xRAData.eRAState == eRAStateWait )
        {
            /* A Router Solicitation has been sent, waited for a reply, but no came.
             * All replies will be handled in the function vReceiveRA(). */
            pxEndPoint->xRAData.uxRetryCount++;

            if( pxEndPoint->xRAData.uxRetryCount < ( UBaseType_t ) ipconfigRA_SEARCH_COUNT )
            {
                pxEndPoint->xRAData.eRAState = eRAStateApply;
            }
            else
            {
                FreeRTOS_printf( ( "RA: Giving up waiting for a Router.\n" ) );
                ( void ) memcpy( &( pxEndPoint->ipv6_settings ), &( pxEndPoint->ipv6_defaults ), sizeof( pxEndPoint->ipv6_settings ) );

                pxEndPoint->xRAData.bits.bRouterReplied = pdFALSE_UNSIGNED;
                pxEndPoint->xRAData.uxRetryCount = 0U;
                /* Force taking a new random IP-address. */
                pxEndPoint->xRAData.bits.bIPAddressInUse = pdTRUE_UNSIGNED;
                pxEndPoint->xRAData.eRAState = eRAStateIPTest;
            }
        }
        else if( pxEndPoint->xRAData.eRAState == eRAStateIPWait )
        {
            /* A Neighbour Solicitation has been sent, waited for a reply.
             * Repeat this 'ipconfigRA_IP_TEST_COUNT' times to be sure. */
            if( pxEndPoint->xRAData.bits.bIPAddressInUse != pdFALSE_UNSIGNED )
            {
                /* Another device has responded with the same IPv4 address. */
                pxEndPoint->xRAData.uxRetryCount = 0U;
                pxEndPoint->xRAData.eRAState = eRAStateIPTest;
                uxNewReloadTime = pdMS_TO_TICKS( ipconfigRA_IP_TEST_TIME_OUT_MSEC );
            }
            else if( pxEndPoint->xRAData.uxRetryCount < ( UBaseType_t ) ipconfigRA_IP_TEST_COUNT )
            {
                /* Try again. */
                pxEndPoint->xRAData.uxRetryCount++;
                pxEndPoint->xRAData.eRAState = eRAStateIPTest;
                uxNewReloadTime = pdMS_TO_TICKS( ipconfigRA_IP_TEST_TIME_OUT_MSEC );
            }
            else
            {
                /* Now it is assumed that there is no other device using the same IP-address. */
                if( pxEndPoint->xRAData.bits.bRouterReplied != pdFALSE_UNSIGNED )
                {
                    /* Obtained configuration from a router. */
                    uxNewReloadTime = pdMS_TO_TICKS( ( 1000U * ( uint64_t ) pxEndPoint->xRAData.ulPreferredLifeTime ) );
                    pxEndPoint->xRAData.eRAState = eRAStatePreLease;
                    iptraceRA_SUCCEEDED( &( pxEndPoint->ipv6_settings.xIPAddress ) );
                    FreeRTOS_printf( ( "RA: succeeded, using IP address %pip Reload after %u seconds\n",
                                       ( void * ) pxEndPoint->ipv6_settings.xIPAddress.ucBytes,
                                       ( unsigned ) pxEndPoint->xRAData.ulPreferredLifeTime ) );
                }
                else
                {
                    /* Using the default network parameters. */
                    pxEndPoint->xRAData.eRAState = eRAStateFailed;

                    iptraceRA_REQUESTS_FAILED_USING_DEFAULT_IP_ADDRESS( &( pxEndPoint->ipv6_settings.xIPAddress ) );

                    FreeRTOS_printf( ( "RA: failed, using default parameters and IP address %pip\n", ( void * ) pxEndPoint->ipv6_settings.xIPAddress.ucBytes ) );
                    /* Disable the timer. */
                    uxNewReloadTime = 0U;
                }

                /* Now call vIPNetworkUpCalls() to send the network-up event and
                 * start the ARP timer. */
                vIPNetworkUpCalls( pxEndPoint );
            }
        }
        else
        {
            /* Do nothing */
        }

        return uxNewReloadTime;
    }
/*-----------------------------------------------------------*/

/**
 * @brief Handles the RA states other than the wait states.
 *
 * @param[in] pxEndPoint The end point for which RA assignment is required.
 * @param[out] uxReloadTime Timer reload value in ticks.
 *
 * @return New timer reload value.
 */
    static TickType_t xRAProcess_HandleOtherStates( NetworkEndPoint_t * pxEndPoint,
                                                    TickType_t uxReloadTime )
    {
        TickType_t uxNewReloadTime = uxReloadTime;

        switch( pxEndPoint->xRAData.eRAState )
        {
            case eRAStateApply:
               {
                   IPv6_Address_t xIPAddress;
                   size_t uxNeededSize;
                   NetworkBufferDescriptor_t * pxNetworkBuffer;

                   /* Send a Router Solicitation to ff02::2 */
                   ( void ) memset( xIPAddress.ucBytes, 0, sizeof( xIPAddress.ucBytes ) );
                   xIPAddress.ucBytes[ 0 ] = 0xffU;
                   xIPAddress.ucBytes[ 1 ] = 0x02U;
                   xIPAddress.ucBytes[ 15 ] = 0x02U;
                   uxNeededSize = ipSIZE_OF_ETH_HEADER + ipSIZE_OF_IPv6_HEADER + sizeof( ICMPRouterSolicitation_IPv6_t );
                   pxNetworkBuffer = pxGetNetworkBufferWithDescriptor( uxNeededSize, raDONT_BLOCK );

                   if( pxNetworkBuffer != NULL )
                   {
                       pxNetworkBuffer->pxEndPoint = pxEndPoint;
                       vNDSendRouterSolicitation( pxNetworkBuffer, &( xIPAddress ) );
                   }

                   FreeRTOS_printf( ( "vRAProcess: Router Solicitation, attempt %lu/%u\n",
                                      pxEndPoint->xRAData.uxRetryCount + 1U,
                                      ipconfigRA_SEARCH_COUNT ) );
                   /* Wait a configurable time for a router advertisement. */
                   uxNewReloadTime = pdMS_TO_TICKS( ipconfigRA_SEARCH_TIME_OUT_MSEC );
                   pxEndPoint->xRAData.eRAState = eRAStateWait;
               }
               break;

            case eRAStateIPTest: /* Assuming an IP address, test if another device is using it already. */
               {
                   size_t uxNeededSize;
                   NetworkBufferDescriptor_t * pxNetworkBuffer;

                   /* Get an IP-address, using the network prefix and a random host address. */
                   if( pxEndPoint->xRAData.bits.bIPAddressInUse != 0U )
                   {
                       pxEndPoint->xRAData.bits.bIPAddressInUse = pdFALSE_UNSIGNED;

                       ( void ) FreeRTOS_CreateIPv6Address( &pxEndPoint->ipv6_settings.xIPAddress, &pxEndPoint->ipv6_settings.xPrefix, pxEndPoint->ipv6_settings.uxPrefixLength, pdTRUE );

                       FreeRTOS_printf( ( "RA: Creating a random IP-address\n" ) );
                   }

                   FreeRTOS_printf( ( "RA: Neighbour solicitation for %pip\n", ( void * ) pxEndPoint->ipv6_settings.xIPAddress.ucBytes ) );

                   uxNeededSize = ipSIZE_OF_ETH_HEADER + ipSIZE_OF_IPv6_HEADER + sizeof( ICMPHeader_IPv6_t );
                   pxNetworkBuffer = pxGetNetworkBufferWithDescriptor( uxNeededSize, raDONT_BLOCK );

                   if( pxNetworkBuffer != NULL )
                   {
                       pxNetworkBuffer->pxEndPoint = pxEndPoint;
                       vNDSendNeighbourSolicitation( pxNetworkBuffer, &( pxEndPoint->ipv6_settings.xIPAddress ) );
                   }

                   uxNewReloadTime = pdMS_TO_TICKS( 1000U );
                   pxEndPoint->xRAData.eRAState = eRAStateIPWait;
               }
               break;

            case eRAStatePreLease:
                pxEndPoint->xRAData.eRAState = eRAStateLease;
                break;

            case eRAStateLease:

                vRAProcessInit( pxEndPoint );
                uxNewReloadTime = pdMS_TO_TICKS( 1000U );

                break;

            case eRAStateFailed:
                break;

            default:
                /* All states were handled. */
                break;
        }

        return uxNewReloadTime;
    }
/*-----------------------------------------------------------*/

/**
 * @brief Initialise the RA state machine.
 *
 * @param[in] pxEndPoint The end-point for which Router Advertisement is required.
 */
    static void vRAProcessInit( NetworkEndPoint_t * pxEndPoint )
    {
        pxEndPoint->xRAData.uxRetryCount = 0U;
        pxEndPoint->xRAData.eRAState = eRAStateApply;
    }

/**
 * @brief Do a single cycle of the RA state machine.
 *
 * @param[in] xDoReset pdTRUE if the state machine must be reset.
 * @param[in] pxEndPoint The end-point for which a RA assignment is required.
 */
    void vRAProcess( BaseType_t xDoReset,
                     NetworkEndPoint_t * pxEndPoint )
    {
        TickType_t uxReloadTime = pdMS_TO_TICKS( 5000U );

        #if ( ipconfigHAS_PRINTF == 1 )
            eRAState_t eRAState;
        #endif

        configASSERT( pxEndPoint != NULL );

        #if ( ipconfigHAS_PRINTF == 1 )
            /* Remember the initial state, just for logging. */
            eRAState = pxEndPoint->xRAData.eRAState;
        #endif

        if( xDoReset != pdFALSE )
        {
            vRAProcessInit( pxEndPoint );
        }

        /* First handle the states that are limited by a timer. See if some
         * timer has expired. */
        uxReloadTime = xRAProcess_HandleWaitStates( pxEndPoint, uxReloadTime );

        /* Now handle the other states. */
        uxReloadTime = xRAProcess_HandleOtherStates( pxEndPoint, uxReloadTime );

        #if ( ipconfigHAS_PRINTF == 1 )
        {
            FreeRTOS_printf( ( "vRAProcess( %ld, %pip) bRouterReplied=%d bIPAddressInUse=%d state %d -> %d\n",
                               xDoReset,
                               ( void * ) pxEndPoint->ipv6_defaults.xIPAddress.ucBytes,
                               pxEndPoint->xRAData.bits.bRouterReplied,
                               pxEndPoint->xRAData.bits.bIPAddressInUse,
                               eRAState,
                               pxEndPoint->xRAData.eRAState ) );
        }
        #endif /* ( ipconfigHAS_PRINTF == 1 ) */

        if( uxReloadTime != 0U )
        {
            FreeRTOS_printf( ( "RA: Reload %u seconds\n", ( unsigned ) ( uxReloadTime / 1000U ) ) );
            vDHCP_RATimerReload( pxEndPoint, uxReloadTime );
        }
        else
        {
            /* Disable the timer, this function vRAProcess() won't be called anymore for this end-point. */
            FreeRTOS_printf( ( "RA: Disabled timer.\n" ) );
            vIPSetDHCP_RATimerEnableState( pxEndPoint, pdFALSE );
        }
    }
/*-----------------------------------------------------------*/

#endif /* ( ipconfigUSE_IPv6 != 0 ) && ( ipconfigUSE_RA != 0 ) */
