/*
 * FreeRTOS+TCP V4.3.3
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
 * @file FreeRTOS_Routing.c
 * @brief Implements endpoint interfaces functions and utilities.
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
#include "FreeRTOS_ARP.h"
#include "FreeRTOS_ND.h"
#include "FreeRTOS_UDP_IP.h"
#include "FreeRTOS_DHCP.h"
#include "NetworkBufferManagement.h"
#if ( ipconfigUSE_LLMNR == 1 )
    #include "FreeRTOS_DNS.h"
#endif /* ipconfigUSE_LLMNR */
#include "FreeRTOS_Routing.h"

/** @brief A list of all network end-points.  Each element has a next pointer. */
struct xNetworkEndPoint * pxNetworkEndPoints = NULL;

/** @brief A list of all network interfaces: */
struct xNetworkInterface * pxNetworkInterfaces = NULL;

/*
 * Add a new IP-address to a Network Interface.  The object pointed to by
 * 'pxEndPoint' and the interface must continue to exist.
 */
static NetworkEndPoint_t * FreeRTOS_AddEndPoint( NetworkInterface_t * pxInterface,
                                                 NetworkEndPoint_t * pxEndPoint );

/** @brief A util struct to list the IPv6 IP types, prefix and type bit mask */
struct xIPv6_Couple
{
    IPv6_Type_t eType;   /**< IPv6 IP type enum */
    uint16_t usMask;     /**< IPv6 IP type bit mask */
    uint16_t usExpected; /**< IPv6 IP type prefix */
};
/*-----------------------------------------------------------*/


#if ( ipconfigUSE_IPv4 != 0 )

/**
 * @brief Configure and install a new IPv4 end-point.
 *
 * @param[in] pxNetworkInterface The interface to which it belongs.
 * @param[in] pxEndPoint Space for the new end-point. This memory is dedicated for the
 *                        end-point and should not be freed or get any other purpose.
 * @param[in] ucIPAddress The IP-address.
 * @param[in] ucNetMask The prefix which shall be used for this end-point.
 * @param[in] ucGatewayAddress The IP-address of a device on the LAN which can serve as
 *                              as a gateway to the Internet.
 * @param[in] ucDNSServerAddress The IP-address of a DNS server.
 * @param[in] ucMACAddress The MAC address of the end-point.
 */
    void FreeRTOS_FillEndPoint( NetworkInterface_t * pxNetworkInterface,
                                NetworkEndPoint_t * pxEndPoint,
                                const uint8_t ucIPAddress[ ipIP_ADDRESS_LENGTH_BYTES ],
                                const uint8_t ucNetMask[ ipIP_ADDRESS_LENGTH_BYTES ],
                                const uint8_t ucGatewayAddress[ ipIP_ADDRESS_LENGTH_BYTES ],
                                const uint8_t ucDNSServerAddress[ ipIP_ADDRESS_LENGTH_BYTES ],
                                const uint8_t ucMACAddress[ ipMAC_ADDRESS_LENGTH_BYTES ] )
    {
        uint32_t ulIPAddress;

        if( ( pxNetworkInterface == NULL ) || ( pxEndPoint == NULL ) )
        {
            /* Invalid input. */
            FreeRTOS_printf( ( "FreeRTOS_FillEndPoint: Invalid input, netif=%p, endpoint=%p\n",
                               ( void * ) pxNetworkInterface,
                               ( void * ) pxEndPoint ) );
        }
        else
        {
            /* Fill in and add an end-point to a network interface.
             * The user must make sure that the object pointed to by 'pxEndPoint'
             * will remain to exist. */

            /* As the endpoint might be part of a linked list,
             * protect the field pxNext from being overwritten. */
            NetworkEndPoint_t * pxNext = pxEndPoint->pxNext;
            ( void ) memset( pxEndPoint, 0, sizeof( *pxEndPoint ) );
            pxEndPoint->pxNext = pxNext;

            ulIPAddress = FreeRTOS_inet_addr_quick( ucIPAddress[ 0 ], ucIPAddress[ 1 ], ucIPAddress[ 2 ], ucIPAddress[ 3 ] );
            pxEndPoint->ipv4_settings.ulNetMask = FreeRTOS_inet_addr_quick( ucNetMask[ 0 ], ucNetMask[ 1 ], ucNetMask[ 2 ], ucNetMask[ 3 ] );
            pxEndPoint->ipv4_settings.ulGatewayAddress = FreeRTOS_inet_addr_quick( ucGatewayAddress[ 0 ], ucGatewayAddress[ 1 ], ucGatewayAddress[ 2 ], ucGatewayAddress[ 3 ] );
            pxEndPoint->ipv4_settings.ulDNSServerAddresses[ 0 ] = FreeRTOS_inet_addr_quick( ucDNSServerAddress[ 0 ], ucDNSServerAddress[ 1 ], ucDNSServerAddress[ 2 ], ucDNSServerAddress[ 3 ] );
            pxEndPoint->ipv4_settings.ulBroadcastAddress = ulIPAddress | ~( pxEndPoint->ipv4_settings.ulNetMask );

            /* Copy the current values to the default values. */
            ( void ) memcpy( &( pxEndPoint->ipv4_defaults ), &( pxEndPoint->ipv4_settings ), sizeof( pxEndPoint->ipv4_defaults ) );

            /* The default IP-address will be used in case DHCP is not used, or also if DHCP has failed, or
             * when the user chooses to use the default IP-address. */
            pxEndPoint->ipv4_defaults.ulIPAddress = ulIPAddress;

            /* The field 'ipv4_settings.ulIPAddress' will be set later on. */

            ( void ) memcpy( pxEndPoint->xMACAddress.ucBytes, ucMACAddress, sizeof( pxEndPoint->xMACAddress ) );
            ( void ) FreeRTOS_AddEndPoint( pxNetworkInterface, pxEndPoint );
        }
    }
/*-----------------------------------------------------------*/
#endif /* ( ipconfigUSE_IPv4 != 0 ) */

#if ( ipconfigCOMPATIBLE_WITH_SINGLE == 0 )

/**
 * @brief Add a network interface to the list of interfaces.  Check if the interface was
 *        already added in an earlier call.
 *
 * @param[in] pxInterface The address of the new interface.
 *
 * @return The value of the parameter 'pxInterface'.
 */
    NetworkInterface_t * FreeRTOS_AddNetworkInterface( NetworkInterface_t * pxInterface )
    {
        NetworkInterface_t * pxIterator = NULL;

        if( pxInterface != NULL )
        {
            if( pxNetworkInterfaces == NULL )
            {
                /* No other interfaces are set yet, so this is the first in the list. */
                pxNetworkInterfaces = pxInterface;
                pxInterface->pxNext = NULL;
            }
            else
            {
                /* Other interfaces are already defined, so iterate to the end of the
                 * list. */
                pxIterator = pxNetworkInterfaces;

                for( ; ; )
                {
                    if( pxIterator == pxInterface )
                    {
                        /* This interface was already added. */
                        break;
                    }

                    if( pxIterator->pxNext == NULL )
                    {
                        pxIterator->pxNext = pxInterface;
                        pxInterface->pxNext = NULL;
                        break;
                    }

                    pxIterator = pxIterator->pxNext;
                }
            }
        }

        return pxInterface;
    }
/*-----------------------------------------------------------*/

/**
 * @brief Get the first Network Interface, or NULL if none has been added.
 *
 * @return The first interface, or NULL if none has been added
 */
    NetworkInterface_t * FreeRTOS_FirstNetworkInterface( void )
    {
        return pxNetworkInterfaces;
    }
/*-----------------------------------------------------------*/

/**
 * @brief Get the next interface.
 *
 * @return The interface that comes after 'pxInterface'. NULL when either 'pxInterface'
 *         is NULL, or when 'pxInterface' is the last interface.
 */
    NetworkInterface_t * FreeRTOS_NextNetworkInterface( const NetworkInterface_t * pxInterface )
    {
        NetworkInterface_t * pxReturn;

        if( pxInterface != NULL )
        {
            pxReturn = pxInterface->pxNext;
        }
        else
        {
            pxReturn = NULL;
        }

        return pxReturn;
    }
/*-----------------------------------------------------------*/

/**
 * @brief Add an end-point to a given interface.
 *
 * @param[in] pxInterface The interface that gets a new end-point.
 * @param[in] pxEndPoint The end-point to be added.
 *
 * @return The value of the parameter 'pxEndPoint'.
 */
    static NetworkEndPoint_t * FreeRTOS_AddEndPoint( NetworkInterface_t * pxInterface,
                                                     NetworkEndPoint_t * pxEndPoint )
    {
        NetworkEndPoint_t * pxIterator = NULL;

        /* Double link between the NetworkInterface_t that is using the addressing
         * defined by this NetworkEndPoint_t structure. */
        pxEndPoint->pxNetworkInterface = pxInterface;

        if( pxInterface->pxEndPoint == NULL )
        {
            /*_RB_ When would pxInterface->pxEndPoint ever not be NULL unless this is called twice? */
            /*_HT_ It may be called twice. */
            pxInterface->pxEndPoint = pxEndPoint;
        }

        if( pxNetworkEndPoints == NULL )
        {
            /* No other end points are defined yet - so this is the first in the
             * list. */
            pxEndPoint->pxNext = NULL;
            pxNetworkEndPoints = pxEndPoint;
        }
        else
        {
            /* Other end points are already defined so iterate to the end of the
             * list. */
            pxIterator = pxNetworkEndPoints;

            for( ; ; )
            {
                if( pxIterator == pxEndPoint )
                {
                    /* This end-point has already been added to the list. */
                    break;
                }

                if( pxIterator->pxNext == NULL )
                {
                    pxEndPoint->pxNext = NULL;
                    pxIterator->pxNext = pxEndPoint;
                    break;
                }

                pxIterator = pxIterator->pxNext;
            }
        }

        #if ( ipconfigUSE_IPv6 != 0 )
            if( pxEndPoint->bits.bIPv6 == pdTRUE_UNSIGNED )
            {
                FreeRTOS_printf( ( "FreeRTOS_AddEndPoint: MAC: %02x-%02x IPv6: %pip\n",
                                   pxEndPoint->xMACAddress.ucBytes[ 4 ],
                                   pxEndPoint->xMACAddress.ucBytes[ 5 ],
                                   ( void * ) pxEndPoint->ipv6_defaults.xIPAddress.ucBytes ) );
            }
        #endif /* ( ipconfigUSE_IPv6 != 0 ) */
        #if ( ipconfigUSE_IPv4 != 0 )
            if( pxEndPoint->bits.bIPv6 == pdFALSE_UNSIGNED )
            {
                FreeRTOS_printf( ( "FreeRTOS_AddEndPoint: MAC: %02x-%02x IPv4: %xip\n",
                                   pxEndPoint->xMACAddress.ucBytes[ 4 ],
                                   pxEndPoint->xMACAddress.ucBytes[ 5 ],
                                   ( unsigned ) FreeRTOS_ntohl( pxEndPoint->ipv4_defaults.ulIPAddress ) ) );
            }
        #endif /* ( ipconfigUSE_IPv4 != 0 ) */

        return pxEndPoint;
    }
/*-----------------------------------------------------------*/

/**
 * @brief Find the first end-point bound to a given interface.
 *
 * @param[in] pxInterface The interface whose first end-point will be returned.
 *
 * @return The first end-point that is found to the interface, or NULL when the
 *         interface doesn't have any end-point yet.
 */
    NetworkEndPoint_t * FreeRTOS_FirstEndPoint( const NetworkInterface_t * pxInterface )
    {
        NetworkEndPoint_t * pxEndPoint = pxNetworkEndPoints;

        /* Find and return the NetworkEndPoint_t structure that is associated with
         * the pxInterface NetworkInterface_t. *//*_RB_ Could this be made a two way link, so the NetworkEndPoint_t can just be read from the NetworkInterface_t structure?  Looks like there is a pointer in the struct already. */
        while( pxEndPoint != NULL )
        {
            if( ( pxInterface == NULL ) || ( pxEndPoint->pxNetworkInterface == pxInterface ) )
            {
                break;
            }

            pxEndPoint = pxEndPoint->pxNext;
        }

        return pxEndPoint;
    }
/*-----------------------------------------------------------*/

/**
 * @brief Get the next end-point.  The parameter 'pxInterface' may be NULL, which means:
 *        don't care which interface the end-point is bound to.
 *
 * @param[in] pxInterface An interface of interest, or NULL when iterating through all
 *                         end-points.
 * @param[in] pxEndPoint This is the current end-point.
 *
 * @return The end-point that is found, or NULL when there are no more end-points in the list.
 */
    NetworkEndPoint_t * FreeRTOS_NextEndPoint( const NetworkInterface_t * pxInterface,
                                               NetworkEndPoint_t * pxEndPoint )
    {
        NetworkEndPoint_t * pxResult = pxEndPoint;

        if( pxResult != NULL )
        {
            pxResult = pxResult->pxNext;

            while( pxResult != NULL )
            {
                if( ( pxInterface == NULL ) || ( pxResult->pxNetworkInterface == pxInterface ) )
                {
                    break;
                }

                pxResult = pxResult->pxNext;
            }
        }
        else
        {
            pxResult = FreeRTOS_FirstEndPoint( pxInterface );
        }

        return pxResult;
    }
/*-----------------------------------------------------------*/

/**
 * @brief Find the end-point which has a given IPv4 address.
 *
 * @param[in] ulIPAddress The IP-address of interest, or 0 if any IPv4 end-point may be returned.
 *
 * @return The end-point found or NULL.
 */
    NetworkEndPoint_t * FreeRTOS_FindEndPointOnIP_IPv4( uint32_t ulIPAddress )
    {
        NetworkEndPoint_t * pxEndPoint = pxNetworkEndPoints;

        while( pxEndPoint != NULL )
        {
            #if ( ipconfigUSE_IPv4 != 0 )
                #if ( ipconfigUSE_IPv6 != 0 )
                    if( pxEndPoint->bits.bIPv6 == 0U )
                #endif
                {
                    if( ( ulIPAddress == 0U ) ||
                        ( pxEndPoint->ipv4_settings.ulIPAddress == ulIPAddress ) )
                    {
                        break;
                    }
                }
            #endif /* ( ipconfigUSE_IPv4 != 0 ) */

            pxEndPoint = pxEndPoint->pxNext;
        }

        ( void ) ulIPAddress;

        return pxEndPoint;
    }
/*-----------------------------------------------------------*/

    #if ( ipconfigUSE_IPv6 != 0 )

/**
 * @brief Find the end-point which handles a given IPv6 address.
 *
 * @param[in] pxIPAddress The IP-address of interest.
 *
 * @return The end-point found or NULL.
 */
        NetworkEndPoint_t * FreeRTOS_FindEndPointOnIP_IPv6( const IPv6_Address_t * pxIPAddress )
        {
            return FreeRTOS_InterfaceEPInSameSubnet_IPv6( NULL, pxIPAddress );
        }
    #endif /* ipconfigUSE_IPv6 */
/*-----------------------------------------------------------*/

/**
 * @brief Find the end-point that has a certain MAC-address.
 *
 * @param[in] pxMACAddress The Ethernet packet.
 * @param[in] pxInterface The interface on which the packet was received, or NULL when unknown.
 *
 * @return The end-point that has the given MAC-address.
 */
    NetworkEndPoint_t * FreeRTOS_FindEndPointOnMAC( const MACAddress_t * pxMACAddress,
                                                    const NetworkInterface_t * pxInterface )
    {
        NetworkEndPoint_t * pxEndPoint = pxNetworkEndPoints;

        /* If input MAC address is NULL, return NULL. */
        if( pxMACAddress == NULL )
        {
            pxEndPoint = NULL;
        }

        /*_RB_ Question - would it be more efficient to store the mac addresses in
         * uin64_t variables for direct comparison instead of using memcmp()?  [don't
         * know if there is a quick way of creating a 64-bit number from the 48-byte
         * MAC address without getting junk in the top 2 bytes]. */

        /* Find the end-point with given MAC-address. */
        while( pxEndPoint != NULL )
        {
            if( ( pxInterface == NULL ) || ( pxInterface == pxEndPoint->pxNetworkInterface ) )
            {
                if( memcmp( pxEndPoint->xMACAddress.ucBytes, pxMACAddress->ucBytes, ipMAC_ADDRESS_LENGTH_BYTES ) == 0 )
                {
                    break;
                }
            }

            pxEndPoint = pxEndPoint->pxNext;
        }

        return pxEndPoint;
    }
/*-----------------------------------------------------------*/

/**
 * @brief Find an end-point that handles a given IPv4-address.
 *
 * @param[in] ulIPAddress The IP-address for which an end-point is looked-up.
 *
 * @return An end-point that has the same network mask as the given IP-address.
 */
    NetworkEndPoint_t * FreeRTOS_FindEndPointOnNetMask( uint32_t ulIPAddress )
    {
        return FreeRTOS_InterfaceEndPointOnNetMask( NULL, ulIPAddress );
    }
/*-----------------------------------------------------------*/

/**
 * @brief Find an end-point that handles a given IPv4-address.
 *
 * @param[in] pxInterface Only end-points that have this interface are returned, unless
 *                         pxInterface is NULL.
 * @param[in] ulIPAddress The IP-address for which an end-point is looked-up.
 *
 * @return An end-point that has the same network mask as the given IP-address.
 */
    NetworkEndPoint_t * FreeRTOS_InterfaceEndPointOnNetMask( const NetworkInterface_t * pxInterface,
                                                             uint32_t ulIPAddress )
    {
        NetworkEndPoint_t * pxEndPoint = pxNetworkEndPoints;

        /* Find the best fitting end-point to reach a given IP-address. */

        /*_RB_ Presumably then a broadcast reply could go out on a different end point to that on
         * which the broadcast was received - although that should not be an issue if the nodes are
         * on the same LAN it could be an issue if the nodes are on separate LAN's. */

        while( pxEndPoint != NULL )
        {
            if( ( pxInterface == NULL ) || ( pxEndPoint->pxNetworkInterface == pxInterface ) )
            {
                #if ( ipconfigUSE_IPv4 != 0 )
                    #if ( ipconfigUSE_IPv6 != 0 )
                        if( pxEndPoint->bits.bIPv6 == pdFALSE_UNSIGNED )
                    #endif
                    {
                        if( ( ulIPAddress == ~0U ) ||
                            ( ( ulIPAddress & pxEndPoint->ipv4_settings.ulNetMask ) == ( pxEndPoint->ipv4_settings.ulIPAddress & pxEndPoint->ipv4_settings.ulNetMask ) ) )
                        {
                            /* Found a match. */
                            break;
                        }
                    }
                #endif /* if ( ipconfigUSE_IPv4 != 0 ) */
            }

            pxEndPoint = pxEndPoint->pxNext;
        }

        /* This was only for debugging. */
        if( pxEndPoint == NULL )
        {
            FreeRTOS_debug_printf( ( "FreeRTOS_FindEndPointOnNetMask: No match for %xip\n",
                                     ( unsigned ) FreeRTOS_ntohl( ulIPAddress ) ) );
        }

        return pxEndPoint;
    }
/*-----------------------------------------------------------*/

    #if ( ipconfigUSE_IPv6 != 0 )

/**
 * @brief Finds an endpoint on the given interface which is in the same subnet as the
 * given IP address. If NULL is passed for pxInterface, it looks through all the
 * interfaces to find an endpoint in the same subnet as the given IP address.
 *
 * @param[in] pxInterface Only end-points that have this interface are returned, unless
 *                         pxInterface is NULL.
 * @param[in] pxIPAddress The IPv6-address for which an end-point is looked-up.
 * @return An end-point that is in the same subnet as the given IP-address.
 */
        NetworkEndPoint_t * FreeRTOS_InterfaceEPInSameSubnet_IPv6( const NetworkInterface_t * pxInterface,
                                                                   const IPv6_Address_t * pxIPAddress )
        {
            NetworkEndPoint_t * pxEndPoint = pxNetworkEndPoints;

            /* Find the best fitting end-point to reach a given IP-address. */

            while( pxEndPoint != NULL )
            {
                if( ( pxInterface == NULL ) || ( pxEndPoint->pxNetworkInterface == pxInterface ) )
                {
                    if( pxEndPoint->bits.bIPv6 != pdFALSE_UNSIGNED )
                    {
                        if( xCompareIPv6_Address( &( pxEndPoint->ipv6_settings.xIPAddress ), pxIPAddress, pxEndPoint->ipv6_settings.uxPrefixLength ) == 0 )
                        {
                            /* Found a match. */
                            break;
                        }
                    }
                }

                pxEndPoint = pxEndPoint->pxNext;
            }

            return pxEndPoint;
        }

/**
 * @brief Configure and install a new IPv6 end-point.
 *
 * @param[in] pxNetworkInterface The interface to which it belongs.
 * @param[in] pxEndPoint Space for the new end-point. This memory is dedicated for the
 *                        end-point and should not be freed or get any other purpose.
 * @param[in] pxIPAddress The IP-address.
 * @param[in] pxNetPrefix The prefix which shall be used for this end-point.
 * @param[in] uxPrefixLength The length of the above end-point.
 * @param[in] pxGatewayAddress The IP-address of a device on the LAN which can serve as
 *                              as a gateway to the Internet.
 * @param[in] pxDNSServerAddress The IP-address of a DNS server.
 * @param[in] ucMACAddress The MAC address of the end-point.
 */
        void FreeRTOS_FillEndPoint_IPv6( NetworkInterface_t * pxNetworkInterface,
                                         NetworkEndPoint_t * pxEndPoint,
                                         const IPv6_Address_t * pxIPAddress,
                                         const IPv6_Address_t * pxNetPrefix,
                                         size_t uxPrefixLength,
                                         const IPv6_Address_t * pxGatewayAddress,
                                         const IPv6_Address_t * pxDNSServerAddress,
                                         const uint8_t ucMACAddress[ ipMAC_ADDRESS_LENGTH_BYTES ] )
        {
            if( ( pxNetworkInterface == NULL ) ||
                ( pxEndPoint == NULL ) ||
                ( pxIPAddress == NULL ) ||
                ( ucMACAddress == NULL ) )
            {
                /* Invalid input. */
                FreeRTOS_printf( ( "FreeRTOS_FillEndPoint_IPv6: Invalid input, netif=%p, endpoint=%p, pxIPAddress=%p, ucMACAddress=%p\n",
                                   ( void * ) pxNetworkInterface,
                                   ( void * ) pxEndPoint,
                                   ( void * ) pxIPAddress,
                                   ( void * ) ucMACAddress ) );
            }
            else
            {
                ( void ) memset( pxEndPoint, 0, sizeof( *pxEndPoint ) );

                pxEndPoint->bits.bIPv6 = pdTRUE_UNSIGNED;

                pxEndPoint->ipv6_settings.uxPrefixLength = uxPrefixLength;

                if( pxGatewayAddress != NULL )
                {
                    ( void ) memcpy( pxEndPoint->ipv6_settings.xGatewayAddress.ucBytes, pxGatewayAddress->ucBytes, ipSIZE_OF_IPv6_ADDRESS );
                }

                if( pxDNSServerAddress != NULL )
                {
                    ( void ) memcpy( pxEndPoint->ipv6_settings.xDNSServerAddresses[ 0 ].ucBytes, pxDNSServerAddress->ucBytes, ipSIZE_OF_IPv6_ADDRESS );
                }

                if( pxNetPrefix != NULL )
                {
                    ( void ) memcpy( pxEndPoint->ipv6_settings.xPrefix.ucBytes, pxNetPrefix->ucBytes, ipSIZE_OF_IPv6_ADDRESS );
                }

                /* Copy the current values to the default values. */
                ( void ) memcpy( &( pxEndPoint->ipv6_defaults ), &( pxEndPoint->ipv6_settings ), sizeof( pxEndPoint->ipv6_defaults ) );

                ( void ) memcpy( pxEndPoint->ipv6_defaults.xIPAddress.ucBytes, pxIPAddress->ucBytes, ipSIZE_OF_IPv6_ADDRESS );

                ( void ) memcpy( pxEndPoint->xMACAddress.ucBytes, ucMACAddress, ipMAC_ADDRESS_LENGTH_BYTES );
                ( void ) FreeRTOS_AddEndPoint( pxNetworkInterface, pxEndPoint );
            }
        }
    #endif /* if ( ipconfigUSE_IPv6 != 0 ) */
/*-----------------------------------------------------------*/

    #if ( ipconfigUSE_IPv6 != 0 )

/**
 * @brief Find an end-point that handles a given IPv6-address.
 *
 * @param[in] pxIPv6Address The IP-address for which an end-point is looked-up.
 *
 * @return An end-point that has the same network mask as the given IP-address.
 */
        NetworkEndPoint_t * FreeRTOS_FindEndPointOnNetMask_IPv6( const IPv6_Address_t * pxIPv6Address )
        {
            return FreeRTOS_InterfaceEPInSameSubnet_IPv6( NULL, pxIPv6Address );
        }
    #endif /* ipconfigUSE_IPv6 */
/*-----------------------------------------------------------*/

/**
 * @brief Check IP-type, IP- and MAC-address found in the network packet.
 */
    #define rMATCH_IP_ADDR      0   /**< Find an endpoint with a matching IP-address. */
    #define rMATCH_IPv6_TYPE    1   /**< Find an endpoint with a matching IPv6 type (both global or non global). */
    #define rMATCH_MAC_ADDR     2   /**< Find an endpoint with a matching MAC-address. */
    #define rMATCH_IP_TYPE      3   /**< Find an endpoint with a matching IP-type, v4 or v6. */
    #define rMATCH_COUNT        4   /**< The number of methods. */

    NetworkEndPoint_t * pxEasyFit( const NetworkInterface_t * pxNetworkInterface,
                                   const uint16_t usFrameType,
                                   const IP_Address_t * pxIPAddressFrom,
                                   const IP_Address_t * pxIPAddressTo,
                                   const MACAddress_t * pxMACAddress );

/**
 * @brief Find an end-point that handles an incoming packet based on its type, source/destination & MAC address.
 *
 * @param[in] pxNetworkInterface The interface via which the packet was received.
 * @param[in] usFrameType Frame type of the packet.
 * @param[in] pxIPAddressFrom Source IP address of the packet.
 * @param[in] pxIPAddressTo Destination IP address of the packet.
 * @param[in] pxMACAddress Destination MAC address of the packet.
 *
 * @return An end-point that handles the packet.
 */
    NetworkEndPoint_t * pxEasyFit( const NetworkInterface_t * pxNetworkInterface,
                                   const uint16_t usFrameType,
                                   const IP_Address_t * pxIPAddressFrom,
                                   const IP_Address_t * pxIPAddressTo,
                                   const MACAddress_t * pxMACAddress )
    {
        NetworkEndPoint_t * pxEndPoint;
        NetworkEndPoint_t * pxReturn = NULL;
        /* endpoints found for IP-type, IP-address, and MAC-address. */
        NetworkEndPoint_t * pxFound[ rMATCH_COUNT ] = { NULL, NULL, NULL, NULL };
        BaseType_t xCount[ rMATCH_COUNT ] = { 0, 0, 0, 0 };
        BaseType_t xIndex;
        BaseType_t xIsIPv6 = ( usFrameType == ipIPv6_FRAME_TYPE ) ? pdTRUE : pdFALSE;
        BaseType_t xGatewayTarget = pdFALSE;
        BaseType_t xTargetGlobal = pdFALSE;

        ( void ) pxIPAddressFrom;
        ( void ) xGatewayTarget;
        ( void ) xTargetGlobal;

        #if ( ipconfigUSE_IPv6 != 0 )
            if( xIsIPv6 == pdTRUE )
            {
                /* Generic GW address fe80::1. */
                static const uint8_t ucBytes[ 16 ] =
                {
                    0xfe, 0x80, 0x00, 0x00,
                    0x00, 0x00, 0x00, 0x00,
                    0x00, 0x00, 0x00, 0x00,
                    0x00, 0x00, 0x00, 0x01
                };
                xGatewayTarget = ( memcmp( ucBytes, pxIPAddressTo->xIP_IPv6.ucBytes, 16 ) == 0 ) ? pdTRUE : pdFALSE;

                if( xGatewayTarget == pdTRUE )
                {
                    FreeRTOS_debug_printf( ( " GW address %pip to %pip\n",
                                             ( void * ) pxIPAddressFrom->xIP_IPv6.ucBytes,
                                             ( void * ) pxIPAddressTo->xIP_IPv6.ucBytes ) );
                }

                xTargetGlobal = ( xIPv6_GetIPType( &( pxIPAddressTo->xIP_IPv6 ) ) == eIPv6_Global ) ? pdTRUE : pdFALSE;
            }
        #endif /* ( ipconfigUSE_IPv6 != 0 ) */

        for( pxEndPoint = FreeRTOS_FirstEndPoint( pxNetworkInterface );
             pxEndPoint != NULL;
             pxEndPoint = FreeRTOS_NextEndPoint( pxNetworkInterface, pxEndPoint ) )
        {
            BaseType_t xSameMACAddress = ( memcmp( pxEndPoint->xMACAddress.ucBytes, pxMACAddress->ucBytes, ipMAC_ADDRESS_LENGTH_BYTES ) == 0 ) ? pdTRUE : pdFALSE;

            if( xIsIPv6 == ( BaseType_t ) pxEndPoint->bits.bIPv6 )
            {
                pxFound[ rMATCH_IP_TYPE ] = pxEndPoint;
                xCount[ rMATCH_IP_TYPE ]++;

                /* Case default is impossible to reach because no endpoints for disabled IP type. */
                switch( xIsIPv6 ) /* LCOV_EXCL_BR_LINE */
                {
                    #if ( ipconfigUSE_IPv6 != 0 )
                        case ( BaseType_t ) pdTRUE:
                           {
                               IPv6_Type_t xEndpointType = xIPv6_GetIPType( &( pxEndPoint->ipv6_settings.xIPAddress ) );

                               if( xEndpointType != eIPv6_Unknown )
                               {
                                   BaseType_t xEndpointGlobal = ( xEndpointType == eIPv6_Global ) ? pdTRUE : pdFALSE;

                                   if( ( memcmp( pxEndPoint->ipv6_settings.xIPAddress.ucBytes, pxIPAddressTo->xIP_IPv6.ucBytes, ipSIZE_OF_IPv6_ADDRESS ) == 0 ) )
                                   {
                                       pxFound[ rMATCH_IP_ADDR ] = pxEndPoint;
                                       xCount[ rMATCH_IP_ADDR ]++;
                                   }
                                   else if( xTargetGlobal == xEndpointGlobal )
                                   {
                                       pxFound[ rMATCH_IPv6_TYPE ] = pxEndPoint;
                                       xCount[ rMATCH_IPv6_TYPE ]++;
                                   }
                                   else
                                   {
                                       /* do nothing, coverity happy */
                                   }
                               }
                               else
                               {
                                   /* do nothing, coverity happy */
                               }
                           }
                           break;
                    #endif /* ( ipconfigUSE_IPv6 != 0 ) */

                    case ( BaseType_t ) pdFALSE:
                    default:
                        #if ( ipconfigUSE_IPv4 != 0 )
                            if( pxEndPoint->ipv4_settings.ulIPAddress == pxIPAddressTo->ulIP_IPv4 )
                            {
                                pxFound[ rMATCH_IP_ADDR ] = pxEndPoint;
                                xCount[ rMATCH_IP_ADDR ]++;
                            }
                            else
                            {
                                /* do nothing, coverity happy */
                            }
                        #endif /* ( ipconfigUSE_IPv4 != 0 ) */

                        break;
                }

                if( xSameMACAddress == pdTRUE )
                {
                    xCount[ rMATCH_MAC_ADDR ]++;
                    pxFound[ rMATCH_MAC_ADDR ] = pxEndPoint;
                }
            }
        }

        for( xIndex = 0; xIndex < rMATCH_COUNT; xIndex++ )
        {
            if( xCount[ xIndex ] >= 1 )
            {
                pxReturn = pxFound[ xIndex ];
                break;
            }
        }

        #if ( ipconfigHAS_DEBUG_PRINTF != 0 )
            if( pxReturn == NULL )
            {
                char pcBufferFrom[ 40 ];
                char pcBufferTo[ 40 ];
                BaseType_t xFamily = ( usFrameType == ipIPv6_FRAME_TYPE ) ? FREERTOS_AF_INET6 : FREERTOS_AF_INET4;
                const char * xRetNtopTo;
                const char * xRetNtopFrom;
                xRetNtopTo = FreeRTOS_inet_ntop( xFamily,
                                                 ( void * ) pxIPAddressTo->xIP_IPv6.ucBytes,
                                                 pcBufferTo,
                                                 sizeof( pcBufferTo ) );
                xRetNtopFrom = FreeRTOS_inet_ntop( xFamily,
                                                   ( void * ) pxIPAddressFrom->xIP_IPv6.ucBytes,
                                                   pcBufferFrom,
                                                   sizeof( pcBufferFrom ) );

                FreeRTOS_debug_printf( ( "EasyFit[%x]: %d %d %d ( %s ->%s ) BAD\n",
                                         usFrameType,
                                         ( unsigned ) xCount[ 0 ],
                                         ( unsigned ) xCount[ 1 ],
                                         ( unsigned ) xCount[ 2 ],
                                         ( xRetNtopFrom == NULL ) ? "INVALID" : pcBufferFrom,
                                         ( xRetNtopTo == NULL ) ? "INVALID" : pcBufferTo ) );
            }
        #endif /* ( ipconfigHAS_DEBUG_PRINTF != 0 ) */

        return pxReturn;
    }

/**
 * @brief Find out the best matching end-point given an incoming Ethernet packet.
 *
 * @param[in] pxNetworkInterface The interface on which the packet was received.
 * @param[in] pucEthernetBuffer The Ethernet packet that was just received.
 *
 * @return The end-point that should handle the incoming Ethernet packet.
 */
    NetworkEndPoint_t * FreeRTOS_MatchingEndpoint( const NetworkInterface_t * pxNetworkInterface,
                                                   const uint8_t * pucEthernetBuffer )
    {
        NetworkEndPoint_t * pxEndPoint = NULL;
        /* MISRA Ref 11.3.1 [Misaligned access] */
        /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
        /* coverity[misra_c_2012_rule_11_3_violation] */
        const ProtocolPacket_t * pxPacket = ( ( const ProtocolPacket_t * ) pucEthernetBuffer );

        #if ( ipconfigUSE_IPv6 != 0 )
            /* MISRA Ref 11.3.1 [Misaligned access] */
            /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
            /* coverity[misra_c_2012_rule_11_3_violation] */
            const IPPacket_IPv6_t * pxIPPacket_IPv6 = ( ( const IPPacket_IPv6_t * ) pucEthernetBuffer );
        #endif /* ( ipconfigUSE_IPv6 != 0 ) */

        configASSERT( pucEthernetBuffer != NULL );

        /* Check if 'pucEthernetBuffer()' has the expected alignment,
         * which is 32-bits + 2. */
        #ifndef _lint
        {
            /* MISRA Ref 11.4.3 [Casting pointer to int for verification] */
            /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-114 */
            /* coverity[misra_c_2012_rule_11_4_violation] */
            uintptr_t uxAddress = ( uintptr_t ) pucEthernetBuffer;
            uxAddress += 2U;
            configASSERT( ( uxAddress % 4U ) == 0U );
            /* And in case configASSERT is not defined. */
            ( void ) uxAddress;
        }
        #endif /* ifndef _lint */

        /* An Ethernet packet has been received. Inspect the contents to see which
         * defined end-point has the best match.
         */

        {
            uint16_t usFrameType = pxPacket->xUDPPacket.xEthernetHeader.usFrameType;
            IP_Address_t xIPAddressFrom;
            IP_Address_t xIPAddressTo;
            MACAddress_t xMACAddress;
            BaseType_t xDoProcessPacket = pdFALSE;

            ( void ) memset( xIPAddressFrom.xIP_IPv6.ucBytes, 0, ipSIZE_OF_IPv6_ADDRESS );
            ( void ) memset( xIPAddressTo.xIP_IPv6.ucBytes, 0, ipSIZE_OF_IPv6_ADDRESS );

            switch( usFrameType )
            {
                case ipIPv6_FRAME_TYPE:

                    /* Handle IPv6 frame types if ipconfigUSE_IPv6 != 0 */
                    #if ( ipconfigUSE_IPv6 != 0 )
                        ( void ) memcpy( xIPAddressFrom.xIP_IPv6.ucBytes, pxIPPacket_IPv6->xIPHeader.xSourceAddress.ucBytes, ipSIZE_OF_IPv6_ADDRESS );
                        ( void ) memcpy( xIPAddressTo.xIP_IPv6.ucBytes, pxIPPacket_IPv6->xIPHeader.xDestinationAddress.ucBytes, ipSIZE_OF_IPv6_ADDRESS );
                        xDoProcessPacket = pdTRUE;
                    #endif /* ( ipconfigUSE_IPv6 != 0 ) */

                    break;

                case ipARP_FRAME_TYPE:

                    /* Handle ARP frame types if ipconfigUSE_IPv4 != 0 */
                    #if ( ipconfigUSE_IPv4 != 0 )
                    {
                        /* MISRA Ref 11.3.1 [Misaligned access] */
                        /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
                        /* coverity[misra_c_2012_rule_11_3_violation] */
                        const ARPPacket_t * pxARPFrame = ( const ARPPacket_t * ) pucEthernetBuffer;

                        if( pxARPFrame->xARPHeader.usOperation == ( uint16_t ) ipARP_REQUEST )
                        {
                            ( void ) memcpy( xIPAddressFrom.xIP_IPv6.ucBytes, pxPacket->xARPPacket.xARPHeader.ucSenderProtocolAddress, sizeof( uint32_t ) );
                            xIPAddressTo.ulIP_IPv4 = pxPacket->xARPPacket.xARPHeader.ulTargetProtocolAddress;
                        }
                        else if( pxARPFrame->xARPHeader.usOperation == ( uint16_t ) ipARP_REPLY )
                        {
                            ( void ) memcpy( xIPAddressTo.xIP_IPv6.ucBytes, pxPacket->xARPPacket.xARPHeader.ucSenderProtocolAddress, sizeof( uint32_t ) );
                            xIPAddressFrom.ulIP_IPv4 = pxPacket->xARPPacket.xARPHeader.ulTargetProtocolAddress;
                        }
                        else
                        {
                            /* do nothing, coverity happy */
                        }

                        FreeRTOS_debug_printf( ( "pxEasyFit: ARP %xip -> %xip\n", ( unsigned ) FreeRTOS_ntohl( xIPAddressFrom.ulIP_IPv4 ), ( unsigned ) FreeRTOS_ntohl( xIPAddressTo.ulIP_IPv4 ) ) );
                    }
                    xDoProcessPacket = pdTRUE;
                    #endif /* ( ipconfigUSE_IPv4 != 0 ) */

                    break;

                case ipIPv4_FRAME_TYPE:

                    /* Handle IPv4 frame types if ipconfigUSE_IPv4 != 0 */
                    #if ( ipconfigUSE_IPv4 != 0 )
                        xIPAddressFrom.ulIP_IPv4 = pxPacket->xUDPPacket.xIPHeader.ulSourceIPAddress;
                        xIPAddressTo.ulIP_IPv4 = pxPacket->xUDPPacket.xIPHeader.ulDestinationIPAddress;
                        xDoProcessPacket = pdTRUE;
                    #endif /* ( ipconfigUSE_IPv4 != 0 ) */

                    break;

                default:
                    #if ( ipconfigPROCESS_CUSTOM_ETHERNET_FRAMES == 1 )
                        /* Custom frame types, match by MAC address only. */
                        xDoProcessPacket = pdTRUE;
                    #endif

                    break;
            }

            if( xDoProcessPacket == pdTRUE )
            {
                ( void ) memcpy( xMACAddress.ucBytes, pxPacket->xUDPPacket.xEthernetHeader.xDestinationAddress.ucBytes, ipMAC_ADDRESS_LENGTH_BYTES );
                pxEndPoint = pxEasyFit( pxNetworkInterface,
                                        usFrameType,
                                        &xIPAddressFrom,
                                        &xIPAddressTo,
                                        &xMACAddress );
            }
        }
        return pxEndPoint;
    }
/*-----------------------------------------------------------*/

/**
 * @brief Find an end-point that defines a gateway of a certain type ( IPv4 or IPv6 ).
 *
 * @param[in] xIPType The type of Gateway to look for ( ipTYPE_IPv4 or ipTYPE_IPv6 ).
 *
 * @return The end-point that will lead to the gateway, or NULL when no gateway was found.
 */
    NetworkEndPoint_t * FreeRTOS_FindGateWay( BaseType_t xIPType )
    {
        NetworkEndPoint_t * pxEndPoint = pxNetworkEndPoints;

        while( pxEndPoint != NULL )
        {
            #if ( ipconfigUSE_IPv6 == 0 )
                ( void ) xIPType;

                if( pxEndPoint->ipv4_settings.ulGatewayAddress != 0U ) /* access to ipv4_settings is checked. */
                {
                    break;
                }
            #else
                if( ( xIPType == ( BaseType_t ) ipTYPE_IPv6 ) && ( pxEndPoint->bits.bIPv6 != pdFALSE_UNSIGNED ) )
                {
                    /* Check if the IP-address is non-zero. */
                    if( memcmp( FreeRTOS_in6addr_any.ucBytes, pxEndPoint->ipv6_settings.xGatewayAddress.ucBytes, ipSIZE_OF_IPv6_ADDRESS ) != 0 )
                    {
                        break;
                    }
                }

                #if ( ipconfigUSE_IPv4 != 0 )
                    else
                    if( ( xIPType == ( BaseType_t ) ipTYPE_IPv4 ) && ( pxEndPoint->bits.bIPv6 == pdFALSE_UNSIGNED ) )
                    {
                        if( pxEndPoint->ipv4_settings.ulGatewayAddress != 0U )
                        {
                            break;
                        }
                    }
                #endif /* ( ipconfigUSE_IPv4 != 0 ) */
                else
                {
                    /* This end-point is not the right IP-type. */
                }
            #endif /* ( ipconfigUSE_IPv6 != 0 ) */
            pxEndPoint = pxEndPoint->pxNext;
        }

        return pxEndPoint;
    }
/*-----------------------------------------------------------*/

    #if ( ipconfigUSE_IPv6 != 0 )

/* Get the first end-point belonging to a given interface.
 * When pxInterface is NULL, the very first end-point will be returned. */

/**
 * @brief Find the first IPv6 end-point.
 *
 * @param[in] pxInterface Either NULL ( don't care ), or a specific interface.
 *
 * @return The end-point found, or NULL when there are no end-points at all.
 */
        NetworkEndPoint_t * FreeRTOS_FirstEndPoint_IPv6( const NetworkInterface_t * pxInterface )
        {
            NetworkEndPoint_t * pxEndPoint = pxNetworkEndPoints;

            while( pxEndPoint != NULL )
            {
                if( ( ( pxInterface == NULL ) || ( pxEndPoint->pxNetworkInterface == pxInterface ) ) && ( pxEndPoint->bits.bIPv6 != pdFALSE_UNSIGNED ) )
                {
                    break;
                }

                pxEndPoint = pxEndPoint->pxNext;
            }

            return pxEndPoint;
        }
    #endif /* ipconfigUSE_IPv6 */
/*-----------------------------------------------------------*/

/**
 * @brief Get the end-point that is bound to a socket.
 *
 * @param[in] xSocket The socket of interest.
 *
 * @return An end-point or NULL in case the socket is not bound to an end-point.
 */
    NetworkEndPoint_t * pxGetSocketEndpoint( ConstSocket_t xSocket )
    {
        const FreeRTOS_Socket_t * pxSocket = ( const FreeRTOS_Socket_t * ) xSocket;
        NetworkEndPoint_t * pxResult;

        if( pxSocket != NULL )
        {
            pxResult = pxSocket->pxEndPoint;
        }
        else
        {
            pxResult = NULL;
        }

        return pxResult;
    }
/*-----------------------------------------------------------*/

/**
 * @brief Assign an end-point to a socket.
 *
 * @param[in] xSocket The socket to which an end-point will be assigned.
 * @param[in] pxEndPoint The end-point to be assigned.
 */
    void vSetSocketEndpoint( Socket_t xSocket,
                             NetworkEndPoint_t * pxEndPoint )
    {
        FreeRTOS_Socket_t * pxSocket = ( FreeRTOS_Socket_t * ) xSocket;

        if( pxSocket != NULL )
        {
            pxSocket->pxEndPoint = pxEndPoint;
        }
    }
/*-----------------------------------------------------------*/

#else /* ( ipconfigCOMPATIBLE_WITH_SINGLE == 0 ) */

/* Here below the most important function of FreeRTOS_Routing.c in a short
 * version: it is assumed that only 1 interface and 1 end-point will be created.
 * The reason for this is downward compatibility with the earlier release of
 * FreeRTOS+TCP, which had a single network interface only. */

/**
 * @brief Add a network interface to the list of interfaces.  Check if this will be
 *        first and only interface ( ipconfigCOMPATIBLE_WITH_SINGLE = 1 ).
 *
 * @param[in] pxInterface The address of the new interface.
 *
 * @return The value of the parameter 'pxInterface'.
 */
    NetworkInterface_t * FreeRTOS_AddNetworkInterface( NetworkInterface_t * pxInterface )
    {
        configASSERT( pxNetworkInterfaces == NULL );
        pxNetworkInterfaces = pxInterface;
        return pxInterface;
    }
/*-----------------------------------------------------------*/

/**
 * @brief And an end-point to an interface.  Note that when ipconfigCOMPATIBLE_WITH_SINGLE
 *        is defined, only one interface is allowed, which will have one end-point only.
 *
 * @param[in] pxInterface The interface to which the end-point is assigned.
 * @param[in] pxEndPoint The end-point to be assigned to the above interface.
 *
 * @return The value of the parameter 'pxEndPoint'.
 */
    static NetworkEndPoint_t * FreeRTOS_AddEndPoint( NetworkInterface_t * pxInterface,
                                                     NetworkEndPoint_t * pxEndPoint )
    {
        /* This code is in backward-compatibility mode.
         * Only one end-point is allowed, make sure that
         * no end-point has been defined yet. */
        configASSERT( pxNetworkEndPoints == NULL );

        /* This end point will go to the end of the list, so there is no pxNext
         * yet. */
        pxEndPoint->pxNext = NULL;

        /* Double link between the NetworkInterface_t that is using the addressing
         * defined by this NetworkEndPoint_t structure. */
        pxEndPoint->pxNetworkInterface = pxInterface;

        pxInterface->pxEndPoint = pxEndPoint;

        /* No other end points are defined yet - so this is the first in the
         * list. */
        pxNetworkEndPoints = pxEndPoint;

        return pxEndPoint;
    }
/*-----------------------------------------------------------*/

/**
 * @brief Find the end-point which has a given IPv4 address.
 *
 * @param[in] ulIPAddress The IP-address of interest, or 0 if any IPv4 end-point may be returned.
 *
 * @return The end-point found or NULL.
 */
    NetworkEndPoint_t * FreeRTOS_FindEndPointOnIP_IPv4( uint32_t ulIPAddress )
    {
        NetworkEndPoint_t * pxResult = NULL;

        ( void ) ulIPAddress;

        if( ( ulIPAddress == 0U ) || ( pxNetworkEndPoints->ipv4_settings.ulIPAddress == ulIPAddress ) )
        {
            pxResult = pxNetworkEndPoints;
        }

        return pxResult;
    }
/*-----------------------------------------------------------*/

/**
 * @brief Find the end-point that has a certain MAC-address.
 *
 * @param[in] pxMACAddress The Ethernet packet.
 * @param[in] pxInterface The interface on which the packet was received, or NULL when unknown.
 *
 * @return The end-point that has the given MAC-address.
 */
    NetworkEndPoint_t * FreeRTOS_FindEndPointOnMAC( const MACAddress_t * pxMACAddress,
                                                    const NetworkInterface_t * pxInterface )
    {
        NetworkEndPoint_t * pxResult = NULL;

        ( void ) pxMACAddress;
        ( void ) pxInterface;

        if( ( pxMACAddress != NULL ) && ( memcmp( pxNetworkEndPoints->xMACAddress.ucBytes, pxMACAddress->ucBytes, ipMAC_ADDRESS_LENGTH_BYTES ) == 0 ) )
        {
            pxResult = pxNetworkEndPoints;
        }

        return pxResult;
    }
/*-----------------------------------------------------------*/

/**
 * @brief Find an end-point that handles a given IPv4-address.
 *
 * @param[in] ulIPAddress The IP-address for which an end-point is looked-up.
 *
 * @return An end-point that has the same network mask as the given IP-address.
 */
    NetworkEndPoint_t * FreeRTOS_FindEndPointOnNetMask( uint32_t ulIPAddress )
    {
        return FreeRTOS_InterfaceEndPointOnNetMask( NULL, ulIPAddress );
    }
/*-----------------------------------------------------------*/

/**
 * @brief Find an end-point that defines a gateway of a certain type ( IPv4 or IPv6 ).
 *
 * @param[in] xIPType The type of Gateway to look for ( ipTYPE_IPv4 or ipTYPE_IPv6 ).
 *
 * @return The end-point that will lead to the gateway, or NULL when no gateway was found.
 */
    NetworkEndPoint_t * FreeRTOS_FindGateWay( BaseType_t xIPType )
    {
        NetworkEndPoint_t * pxReturn = NULL;

        ( void ) xIPType;

        if( pxNetworkEndPoints != NULL )
        {
            if( pxNetworkEndPoints->ipv4_settings.ulGatewayAddress != 0U )
            {
                pxReturn = pxNetworkEndPoints;
            }
        }

        return pxReturn;
    }
/*-----------------------------------------------------------*/

/**
 * @brief Find the first end-point bound to a given interface.
 *
 * @param[in] pxInterface The interface whose first end-point will be returned.
 *
 * @return The first end-point that is found to the interface, or NULL when the
 *         interface doesn't have any end-point yet.
 */
    NetworkEndPoint_t * FreeRTOS_FirstEndPoint( const NetworkInterface_t * pxInterface )
    {
        ( void ) pxInterface;

        /* ipconfigCOMPATIBLE_WITH_SINGLE is defined and this is the simplified version:
         * only one interface and one end-point is defined. */
        return pxNetworkEndPoints;
    }
/*-----------------------------------------------------------*/

/**
 * @brief Get the first Network Interface, or NULL if none has been added.
 *
 * @return The first interface, or NULL if none has been added
 */
    NetworkInterface_t * FreeRTOS_FirstNetworkInterface( void )
    {
        /* ipconfigCOMPATIBLE_WITH_SINGLE is defined: only one interface and
         * one end-point is defined. */
        return pxNetworkInterfaces;
    }
/*-----------------------------------------------------------*/

/**
 * @brief Find an end-point that handles a given IPv4-address.
 *
 * @param[in] pxInterface Ignored in this simplified version.
 * @param[in] ulIPAddress The IP-address for which an end-point is looked-up.
 *
 * @return An end-point that has the same network mask as the given IP-address.
 */
    NetworkEndPoint_t * FreeRTOS_InterfaceEndPointOnNetMask( const NetworkInterface_t * pxInterface,
                                                             uint32_t ulIPAddress )
    {
        NetworkEndPoint_t * pxResult = NULL;

        ( void ) pxInterface;

        if( ( ( ulIPAddress ^ pxNetworkEndPoints->ipv4_settings.ulIPAddress ) & pxNetworkEndPoints->ipv4_settings.ulNetMask ) == 0U )
        {
            pxResult = pxNetworkEndPoints;
        }

        return pxResult;
    }
/*-----------------------------------------------------------*/

/**
 * @brief Find out the best matching end-point given an incoming Ethernet packet.
 *
 * @param[in] pxNetworkInterface The interface on which the packet was received.
 * @param[in] pucEthernetBuffer The Ethernet packet that was just received.
 *
 * @return The end-point that should handle the incoming Ethernet packet.
 */
    NetworkEndPoint_t * FreeRTOS_MatchingEndpoint( const NetworkInterface_t * pxNetworkInterface,
                                                   const uint8_t * pucEthernetBuffer )
    {
        ( void ) pxNetworkInterface;
        ( void ) pucEthernetBuffer;

        /* ipconfigCOMPATIBLE_WITH_SINGLE is defined: only one interface and
         * one end-point is defined. */
        return pxNetworkEndPoints;
    }
/*-----------------------------------------------------------*/

/**
 * @brief Get the next end-point.  As this is the simplified version, it will always
 *        return NULL.
 *
 * @param[in] pxInterface An interface of interest, or NULL when iterating through all
 *                         end-points.
 * @param[in] pxEndPoint This is the current end-point.
 *
 * @return NULL because ipconfigCOMPATIBLE_WITH_SINGLE is defined.
 */
    NetworkEndPoint_t * FreeRTOS_NextEndPoint( const NetworkInterface_t * pxInterface,
                                               NetworkEndPoint_t * pxEndPoint )
    {
        ( void ) pxInterface;
        ( void ) pxEndPoint;

        return NULL;
    }
/*-----------------------------------------------------------*/

/**
 * @brief Get the next interface.
 *
 * @return NULL because ipconfigCOMPATIBLE_WITH_SINGLE is defined.
 */
    NetworkInterface_t * FreeRTOS_NextNetworkInterface( const NetworkInterface_t * pxInterface )
    {
        ( void ) pxInterface;

        return NULL;
    }
/*-----------------------------------------------------------*/

    #if ( ipconfigUSE_IPv6 != 0 )
        NetworkEndPoint_t * FreeRTOS_FindEndPointOnIP_IPv6( const IPv6_Address_t * pxIPAddress )
        {
            ( void ) pxIPAddress;
            return pxNetworkEndPoints;
        }

/*-----------------------------------------------------------*/

        NetworkEndPoint_t * FreeRTOS_FindEndPointOnNetMask_IPv6( const IPv6_Address_t * pxIPv6Address )
        {
            ( void ) pxIPv6Address;
            return pxNetworkEndPoints;
        }

/*-----------------------------------------------------------*/

        NetworkEndPoint_t * FreeRTOS_FirstEndPoint_IPv6( const NetworkInterface_t * pxInterface )
        {
            ( void ) pxInterface;
            return pxNetworkEndPoints;
        }

/*-----------------------------------------------------------*/

/**
 * @brief If the device endpoint is in the same subnet as the given IP address, return the
 * endpoint. Otherwise, return NULL.
 *
 * @param[in] pxInterface Ignored in this simplified version for single endpoint.
 * @param[in] ulIPAddress The IP-address for which an end-point is looked-up.
 *
 * @return An end-point that is in the same subnet as the given IP-address.
 */
        NetworkEndPoint_t * FreeRTOS_InterfaceEPInSameSubnet_IPv6( const NetworkInterface_t * pxInterface,
                                                                   const IPv6_Address_t * pxIPAddress )
        {
            NetworkEndPoint_t * pxResult = NULL;

            ( void ) pxInterface;

            if( xCompareIPv6_Address( &( pxNetworkEndPoints->ipv6_settings.xIPAddress ), pxIPAddress, pxNetworkEndPoints->ipv6_settings.uxPrefixLength ) == 0 )
            {
                pxResult = pxNetworkEndPoints;
            }

            return pxResult;
        }
    #endif /* if ( ipconfigUSE_IPv6 != 0 ) */
/*-----------------------------------------------------------*/

#endif /* ( ipconfigCOMPATIBLE_WITH_SINGLE == 0 ) */

/**
 * @brief Returns the IP type of the given IPv6 address.
 *
 * @param[in] pxAddress The IPv6 address whose type needs to be returned.
 * @returns The IP type of the given address.
 */
#if ( ipconfigUSE_IPv6 != 0 )
    IPv6_Type_t xIPv6_GetIPType( const IPv6_Address_t * pxAddress )
    {
        IPv6_Type_t eResult = eIPv6_Unknown;
        BaseType_t xIndex;
        static const struct xIPv6_Couple xIPCouples[] =
        {
            /*    IP-type          Mask     Value */
            { eIPv6_Global,    0xE000U, 0x2000U }, /* 001 */
            { eIPv6_LinkLocal, 0xFFC0U, 0xFE80U }, /* 1111 1110 10 */
            { eIPv6_SiteLocal, 0xFFC0U, 0xFEC0U }, /* 1111 1110 11 */
            { eIPv6_Multicast, 0xFF00U, 0xFF00U }, /* 1111 1111 */
            { eIPv6_Loopback,  0xFFFFU, 0x0000U }, /* 0000 0000 ::1 */
        };

        if( pxAddress != NULL )
        {
            for( xIndex = 0; xIndex < ARRAY_SIZE_X( xIPCouples ); xIndex++ )
            {
                uint16_t usAddress =
                    ( uint16_t ) ( ( ( ( uint16_t ) pxAddress->ucBytes[ 0 ] ) << 8 ) |
                                   ( ( uint16_t ) pxAddress->ucBytes[ 1 ] ) );

                if( xIPCouples[ xIndex ].eType == eIPv6_Loopback )
                {
                    if( xIsIPv6Loopback( pxAddress ) != pdFALSE )
                    {
                        eResult = eIPv6_Loopback;
                        break;
                    }
                }

                if( ( usAddress & xIPCouples[ xIndex ].usMask ) == xIPCouples[ xIndex ].usExpected )
                {
                    eResult = xIPCouples[ xIndex ].eType;
                    break;
                }
            }
        }

        return eResult;
    }
#endif /* if ( ipconfigUSE_IPv6 != 0 ) */
/*-----------------------------------------------------------*/

#if ( ( ipconfigHAS_PRINTF != 0 ) || ( ipconfigHAS_DEBUG_PRINTF != 0 ) )

/**
 * @brief Returns the string representation of the IP address of the end point.
 *
 * @param[in] pxEndPoint End point for which IP address needs to be returned.
 * @param[in] pcBuffer A char buffer of required size to which the string will be written.
 * @param[in] uxSize Size of the char buffer - pcBuffer.
 *
 * @returns The pointer to the char buffer that contains the string representation of the end point IP address.
 *          The string will be "NULL" if the end point pointer is NULL.
 */
    const char * pcEndpointName( const NetworkEndPoint_t * pxEndPoint,
                                 char * pcBuffer,
                                 size_t uxSize )
    {
        if( pxEndPoint == NULL )
        {
            /* MISRA Ref 21.6.1 [snprintf and logging] */
            /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-216 */
            /* coverity[misra_c_2012_rule_21_6_violation] */
            ( void ) snprintf( pcBuffer, uxSize, "NULL" );
        }
        else
        {
            switch( pxEndPoint->bits.bIPv6 ) /* LCOV_EXCL_BR_LINE */
            {
                #if ( ipconfigUSE_IPv4 != 0 )
                    case pdFALSE_UNSIGNED:
                        ( void ) FreeRTOS_inet_ntop( FREERTOS_AF_INET4,
                                                     ( const void * ) &( pxEndPoint->ipv4_settings.ulIPAddress ),
                                                     pcBuffer,
                                                     ( socklen_t ) uxSize );
                        break;
                #endif /* ( ipconfigUSE_IPv4 != 0 ) */

                #if ( ipconfigUSE_IPv6 != 0 )
                    case pdTRUE_UNSIGNED:
                        ( void ) FreeRTOS_inet_ntop( FREERTOS_AF_INET6,
                                                     pxEndPoint->ipv6_settings.xIPAddress.ucBytes,
                                                     pcBuffer,
                                                     ( socklen_t ) uxSize );
                        break;
                #endif /* ( ipconfigUSE_IPv6 != 0 ) */

                default:
                    /* MISRA 16.4 Compliance */
                    /* MISRA Ref 21.6.1 [snprintf and logging] */
                    /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-216 */
                    /* coverity[misra_c_2012_rule_21_6_violation] */
                    ( void ) snprintf( pcBuffer, uxSize, "NULL" );
                    break;
            }
        }

        return pcBuffer;
    }
/*-----------------------------------------------------------*/
#endif /* ( ( ipconfigHAS_PRINTF != 0 ) || ( ipconfigHAS_DEBUG_PRINTF != 0 ) ) */

/**
 * @brief Check whether a packet needs resolution if it is on local subnet. If required send a request.
 *
 * @param[in] pxNetworkBuffer The network buffer with the packet to be checked.
 *
 * @return pdTRUE if the packet needs resolution, pdFALSE otherwise.
 */
BaseType_t xCheckRequiresResolution( const NetworkBufferDescriptor_t * pxNetworkBuffer )
{
    BaseType_t xNeedsResolution = pdFALSE;

    switch( uxIPHeaderSizePacket( pxNetworkBuffer ) )
    {
        #if ( ipconfigUSE_IPv4 != 0 )
            case ipSIZE_OF_IPv4_HEADER:
                xNeedsResolution = xCheckRequiresARPResolution( pxNetworkBuffer );
                break;
        #endif /* ( ipconfigUSE_IPv4 != 0 ) */

        #if ( ipconfigUSE_IPv6 != 0 )
            case ipSIZE_OF_IPv6_HEADER:
                xNeedsResolution = xCheckRequiresNDResolution( pxNetworkBuffer );
                break;
        #endif /* ( ipconfigUSE_IPv6 != 0 ) */

        default:
            /* Shouldn't reach here */
            /* MISRA 16.4 Compliance */
            break;
    }

    return xNeedsResolution;
}
/*-----------------------------------------------------------*/
