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
 * @file FreeRTOS_DNS.c
 * @brief Implements the Domain Name System for the FreeRTOS+TCP network stack.
 */

/* Standard includes. */
#include <stdint.h>
#include <stdio.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* FreeRTOS+TCP includes. */
#include "FreeRTOS_IP.h"
#include "FreeRTOS_IP_Timers.h"
#include "FreeRTOS_IP_Private.h"
#include "FreeRTOS_UDP_IP.h"
#include "FreeRTOS_DNS.h"
#include "FreeRTOS_DHCP.h"
#include "NetworkBufferManagement.h"
#include "FreeRTOS_Routing.h"
#include "NetworkInterface.h"

#include "FreeRTOS_DNS_Globals.h"
#include "FreeRTOS_DNS_Cache.h"
#include "FreeRTOS_DNS_Parser.h"
#include "FreeRTOS_DNS_Networking.h"
#include "FreeRTOS_DNS_Callback.h"


/** @brief The MAC address used for LLMNR. */
const MACAddress_t xLLMNR_MacAddress = { { 0x01, 0x00, 0x5e, 0x00, 0x00, 0xfc } };

/** @brief The IPv6 link-scope multicast MAC address */
const MACAddress_t xLLMNR_MacAddressIPv6 = { { 0x33, 0x33, 0x00, 0x01, 0x00, 0x03 } };

#if ( ( ipconfigUSE_LLMNR != 0 ) && ( ipconfigUSE_IPv6 != 0 ) )
/** @brief The IPv6 link-scope multicast address */
    const IPv6_Address_t ipLLMNR_IP_ADDR_IPv6 =
    {
        { /* ff02::1:3 */
            0xff, 0x02,
            0x00, 0x00,
            0x00, 0x00,
            0x00, 0x00,
            0x00, 0x00,
            0x00, 0x00,
            0x00, 0x01,
            0x00, 0x03,
        }
    };
#endif /* ( ( ipconfigUSE_LLMNR != 0 ) && ( ipconfigUSE_IPv6 != 0 ) ) */

/** @brief The MAC address used for MDNS. */
const MACAddress_t xMDNS_MacAddress = { { 0x01, 0x00, 0x5e, 0x00, 0x00, 0xfb } };

/** @brief The IPv6 multicast DNS MAC address. */
const MACAddress_t xMDNS_MacAddressIPv6 = { { 0x33, 0x33, 0x00, 0x00, 0x00, 0xFB } };

#if ( ( ipconfigUSE_MDNS != 0 ) && ( ipconfigUSE_IPv6 != 0 ) )
/** @brief multicast DNS IPv6 address */
    const IPv6_Address_t ipMDNS_IP_ADDR_IPv6 =
    {
        { /* ff02::fb */
            0xff, 0x02,
            0x00, 0x00,
            0x00, 0x00,
            0x00, 0x00,
            0x00, 0x00,
            0x00, 0x00,
            0x00, 0x00,
            0x00, 0xfb,
        }
    };
#endif /* ( ( ipconfigUSE_MDNS != 0 ) && ( ipconfigUSE_IPv6 != 0 ) ) */

/* Exclude the entire file if DNS is not enabled. */
#if ( ipconfigUSE_DNS != 0 )

/*
 * Create the DNS message in the zero copy buffer passed in the first parameter.
 */
    static size_t prvCreateDNSMessage( uint8_t * pucUDPPayloadBuffer,
                                       const char * pcHostName,
                                       TickType_t uxIdentifier,
                                       UBaseType_t uxHostType );


/*
 * Check if hostname is already known. If not, call prvGetHostByName() to send a DNS request.
 */
    #if ( ipconfigDNS_USE_CALLBACKS == 1 )
        static uint32_t prvPrepareLookup( const char * pcHostName,
                                          struct freertos_addrinfo ** ppxAddressInfo,
                                          BaseType_t xFamily, /* FREERTOS_AF_INET4 / 6. */
                                          FOnDNSEvent pCallbackFunction,
                                          void * pvSearchID,
                                          TickType_t uxTimeout );
    #else
        static uint32_t prvPrepareLookup( const char * pcHostName,
                                          struct freertos_addrinfo ** ppxAddressInfo,
                                          BaseType_t xFamily ); /* FREERTOS_AF_INET4 / 6. */
    #endif

/*
 * Prepare and send a message to a DNS server.  'uxReadTimeOut_ticks' will be passed as
 * zero, in case the user has supplied a call-back function.
 */
    static uint32_t prvGetHostByName( const char * pcHostName,
                                      TickType_t uxIdentifier,
                                      TickType_t uxReadTimeOut_ticks,
                                      struct freertos_addrinfo ** ppxAddressInfo,
                                      BaseType_t xFamily );

/*-----------------------------------------------------------*/

/** @brief This global variable is being used to indicate to the driver which IP type
 *         is preferred for name service lookup, either IPv6 or IPv4. */
    IPPreference_t xDNS_IP_Preference = xPreferenceIPv4;

/*-----------------------------------------------------------*/

/**
 * @brief A DNS query consists of a header, as described in 'struct xDNSMessage'
 *        It is followed by 1 or more queries, each one consisting of a name and a tail,
 *        with two fields: type and class
 */
    #include "pack_struct_start.h"
    struct xDNSTail
    {
        uint16_t usType;  /**< Type of DNS message. */
        uint16_t usClass; /**< Class of DNS message. */
    }
    #include "pack_struct_end.h"
    typedef struct xDNSTail DNSTail_t;

    #if ( ipconfigUSE_IPv4 != 0 )
/** @brief Increment the field 'ucDNSIndex', which is an index in the array */
        static void prvIncreaseDNS4Index( NetworkEndPoint_t * pxEndPoint );
    #endif

    #if ( ipconfigUSE_IPv6 != 0 )
/** @brief Increment the field 'ucDNSIndex', which is an index in the array */
        static void prvIncreaseDNS6Index( NetworkEndPoint_t * pxEndPoint );
    #endif

/*-----------------------------------------------------------*/

    #if ( ipconfigDNS_USE_CALLBACKS == 1 )

/**
 * @brief Define FreeRTOS_gethostbyname() as a normal blocking call.
 * @param[in] pcHostName The hostname whose IP address is being searched for.
 * @return The IP-address of the hostname.
 */
        uint32_t FreeRTOS_gethostbyname( const char * pcHostName )
        {
            return FreeRTOS_gethostbyname_a( pcHostName, NULL, ( void * ) NULL, 0U );
        }
    #endif /* ipconfigDNS_USE_CALLBACKS == 1 */
/*-----------------------------------------------------------*/

    #if ( ipconfigDNS_USE_CALLBACKS == 1 )

/** @brief Initialise the list of call-back structures.
 */
        void vDNSInitialise( void )
        {
            vDNSCallbackInitialise();
        }
    #endif /* ipconfigDNS_USE_CALLBACKS == 1 */
/*-----------------------------------------------------------*/

    #if ( ipconfigDNS_USE_CALLBACKS == 1 )

/**
 * @brief Remove the entry defined by the search ID to cancel a DNS request.
 * @param[in] pvSearchID The search ID of the callback function associated with
 *                        the DNS request being cancelled. Note that the value of
 *                        the pointer matters, not the pointee.
 */
        void FreeRTOS_gethostbyname_cancel( void * pvSearchID )
        {
            vDNSCheckCallBack( pvSearchID );
        }

    #endif /* ipconfigDNS_USE_CALLBACKS == 1 */
/*-----------------------------------------------------------*/

    #if ( ipconfigDNS_USE_CALLBACKS == 1 )

/**
 * @brief Look-up the IP-address of a host.
 *
 * @param[in] pcName The name of the node or device
 * @param[in] pcService Ignored for now.
 * @param[in] pxHints If not NULL preferences. Can be used to indicate the preferred type if IP ( v4 or v6 ).
 * @param[out] ppxResult An allocated struct, containing the results.
 *
 * @return Zero when the operation was successful, otherwise a negative errno value.
 */
        BaseType_t FreeRTOS_getaddrinfo( const char * pcName,                      /* The name of the node or device */
                                         const char * pcService,                   /* Ignored for now. */
                                         const struct freertos_addrinfo * pxHints, /* If not NULL: preferences. */
                                         struct freertos_addrinfo ** ppxResult )   /* An allocated struct, containing the results. */
        {
            /* Call the asynchronous version with NULL parameters. */
            return FreeRTOS_getaddrinfo_a( pcName, pcService, pxHints, ppxResult, NULL, NULL, 0U );
        }
    #endif /* ( ipconfigDNS_USE_CALLBACKS == 1 ) */
/*-----------------------------------------------------------*/

/**
 * @brief Internal function: allocate and initialise a new struct of type freertos_addrinfo.
 *
 * @param[in] pcName the name of the host.
 * @param[in] xFamily the type of IP-address: FREERTOS_AF_INET4 or FREERTOS_AF_INET6.
 * @param[in] pucAddress The IP-address of the host.
 *
 * @return A pointer to the newly allocated struct, or NULL in case malloc failed..
 */
    struct freertos_addrinfo * pxNew_AddrInfo( const char * pcName,
                                               BaseType_t xFamily,
                                               const uint8_t * pucAddress )
    {
        struct freertos_addrinfo * pxAddrInfo = NULL;
        void * pvBuffer;

        /* 'xFamily' might not be used when IPv6 is disabled. */
        ( void ) xFamily;
        /* 'pcName' might not be used when DNS cache is disabled. */
        ( void ) pcName;

        pvBuffer = pvPortMalloc( sizeof( *pxAddrInfo ) );

        if( pvBuffer != NULL )
        {
            pxAddrInfo = ( struct freertos_addrinfo * ) pvBuffer;

            ( void ) memset( pxAddrInfo, 0, sizeof( *pxAddrInfo ) );
            #if ( ipconfigUSE_DNS_CACHE != 0 )
                pxAddrInfo->ai_canonname = pxAddrInfo->xPrivateStorage.ucName;
                ( void ) strncpy( pxAddrInfo->xPrivateStorage.ucName, pcName, sizeof( pxAddrInfo->xPrivateStorage.ucName ) - 1U );
                pxAddrInfo->xPrivateStorage.ucName[ sizeof( pxAddrInfo->xPrivateStorage.ucName ) - 1U ] = '\0';
            #endif /* (ipconfigUSE_DNS_CACHE != 0 ) */

            pxAddrInfo->ai_addr = ( ( struct freertos_sockaddr * ) &( pxAddrInfo->xPrivateStorage.sockaddr ) );

            switch( xFamily )
            {
                #if ( ipconfigUSE_IPv4 != 0 )
                    case FREERTOS_AF_INET4:
                       {
                           /* ulChar2u32 reads from big-endian to host-endian. */
                           uint32_t ulIPAddress = ulChar2u32( pucAddress );
                           /* Translate to network-endian. */
                           pxAddrInfo->ai_addr->sin_address.ulIP_IPv4 = FreeRTOS_htonl( ulIPAddress );
                           pxAddrInfo->ai_family = FREERTOS_AF_INET4;
                           pxAddrInfo->ai_addrlen = ipSIZE_OF_IPv4_ADDRESS;
                       }
                       break;
                #endif /* ( ipconfigUSE_IPv4 != 0 ) */

                #if ( ipconfigUSE_IPv6 != 0 )
                    case FREERTOS_AF_INET6:
                        pxAddrInfo->ai_family = FREERTOS_AF_INET6;
                        pxAddrInfo->ai_addrlen = ipSIZE_OF_IPv6_ADDRESS;
                        ( void ) memcpy( pxAddrInfo->xPrivateStorage.sockaddr.sin_address.xIP_IPv6.ucBytes, pucAddress, ipSIZE_OF_IPv6_ADDRESS );
                        break;
                #endif /* ( ipconfigUSE_IPv6 != 0 ) */

                default:
                    /* MISRA 16.4 Compliance */
                    FreeRTOS_debug_printf( ( "pxNew_AddrInfo: Undefined xFamily Type \n" ) );

                    vPortFree( pvBuffer );
                    pxAddrInfo = NULL;

                    break;
            }
        }

        return pxAddrInfo;
    }
/*-----------------------------------------------------------*/

/**
 * @brief Free a chain of structs of type 'freertos_addrinfo'.
 * @param[in] pxInfo The first find result.
 */
    void FreeRTOS_freeaddrinfo( struct freertos_addrinfo * pxInfo )
    {
        struct freertos_addrinfo * pxNext;
        struct freertos_addrinfo * pxIterator = pxInfo;

        if( pxInfo != NULL )
        {
            while( pxIterator != NULL )
            {
                pxNext = pxIterator->ai_next;
                vPortFree( pxIterator );
                pxIterator = pxNext;
            }
        }
    }
/*-----------------------------------------------------------*/

    #if ( ipconfigDNS_USE_CALLBACKS == 1 )

/**
 * @brief Asynchronous version of getaddrinfo().
 *
 * @param[in] pcName The name of the node or device
 * @param[in] pcService Ignored for now.
 * @param[in] pxHints If not NULL preferences. Can be used to indicate the preferred type if IP ( v4 or v6 ).
 * @param[out] ppxResult An allocated struct, containing the results.
 * @param[in] pCallback A user-defined function which will be called on completion, either when found or after a time-out.
 * @param[in] pvSearchID A user provided void pointer that will be communicated on completion.
 * @param[in] uxTimeout The maximum number of clock ticks that must be waited for a reply.
 *
 * @return Zero when the operation was successful, otherwise a negative errno value.
 */
        BaseType_t FreeRTOS_getaddrinfo_a( const char * pcName,                      /* The name of the node or device */
                                           const char * pcService,                   /* Ignored for now. */
                                           const struct freertos_addrinfo * pxHints, /* If not NULL: preferences. */
                                           struct freertos_addrinfo ** ppxResult,    /* An allocated struct, containing the results. */
                                           FOnDNSEvent pCallback,
                                           void * pvSearchID,
                                           TickType_t uxTimeout )
    #else

/**
 * @brief Look-up the IP-address of a host.
 * @param[in] pcName The name of the node or device
 * @param[in] pcService Ignored for now.
 * @param[in] pxHints If not NULL preferences. Can be used to indicate the preferred type if IP ( v4 or v6 ).
 * @param[out] ppxResult An allocated struct, containing the results.
 * @return Zero when the operation was successful, otherwise a negative errno value.
 */
        BaseType_t FreeRTOS_getaddrinfo( const char * pcName,                      /* The name of the node or device */
                                         const char * pcService,                   /* Ignored for now. */
                                         const struct freertos_addrinfo * pxHints, /* If not NULL: preferences. */
                                         struct freertos_addrinfo ** ppxResult )   /* An allocated struct, containing the results. */
    #endif /* ipconfigDNS_USE_CALLBACKS == 1 */
    {
        BaseType_t xReturn = 0;
        uint32_t ulResult;
        BaseType_t xFamily = FREERTOS_AF_INET4;

        ( void ) pcService;
        ( void ) pxHints;

        if( ppxResult != NULL )
        {
            *( ppxResult ) = NULL;

            #if ( ipconfigUSE_IPv6 != 0 )
                if( pxHints != NULL )
                {
                    if( pxHints->ai_family == FREERTOS_AF_INET6 )
                    {
                        xFamily = FREERTOS_AF_INET6;
                    }
                    else if( pxHints->ai_family != FREERTOS_AF_INET4 )
                    {
                        xReturn = -pdFREERTOS_ERRNO_EINVAL;
                    }
                    else
                    {
                        /* This is FREERTOS_AF_INET4, carry on. */
                    }
                }
            #endif /* ( ipconfigUSE_IPv6 == 0 ) */

            #if ( ipconfigUSE_IPv6 != 0 )
                if( xReturn == 0 )
            #endif
            {
                #if ( ipconfigDNS_USE_CALLBACKS == 1 )
                {
                    ulResult = prvPrepareLookup( pcName, ppxResult, xFamily, pCallback, pvSearchID, uxTimeout );
                }
                #else
                {
                    ulResult = prvPrepareLookup( pcName, ppxResult, xFamily );
                }
                #endif /* ( ipconfigDNS_USE_CALLBACKS == 1 ) */

                if( ulResult != 0U )
                {
                    if( *( ppxResult ) != NULL )
                    {
                        xReturn = 0;
                    }
                    else
                    {
                        xReturn = -pdFREERTOS_ERRNO_ENOMEM;
                    }
                }
                else
                {
                    xReturn = -pdFREERTOS_ERRNO_ENOENT;
                }
            }
        }
        else
        {
            xReturn = -pdFREERTOS_ERRNO_EINVAL;
        }

        return xReturn;
    }
/*-----------------------------------------------------------*/

    #if ( ipconfigDNS_USE_CALLBACKS == 0 )

/**
 * @brief Get the IP-address corresponding to the given hostname.
 * @param[in] pcHostName The hostname whose IP address is being queried.
 * @return The IP-address corresponding to the hostname. 0 is returned in
 *         case of failure.
 */
        uint32_t FreeRTOS_gethostbyname( const char * pcHostName )
        {
            return prvPrepareLookup( pcHostName, NULL, FREERTOS_AF_INET4 );
        }
    #else

/**
 * @brief Get the IP-address corresponding to the given hostname.
 * @param[in] pcHostName The hostname whose IP address is being queried.
 * @param[in] pCallback The callback function which will be called upon DNS response. It will be called
 *                       with pcHostName, pvSearchID and pxAddressInfo which points to address info.
 *                       The pxAddressInfo should be freed by the application once the callback
 *                       has been called by the FreeRTOS_freeaddrinfo().
 *                       In case of timeouts pxAddressInfo can be NULL.
 * @param[in] pvSearchID Search ID for the callback function.
 * @param[in] uxTimeout Timeout for the callback function.
 * @return The IP-address corresponding to the hostname. 0 is returned in case of
 *         failure.
 */
        uint32_t FreeRTOS_gethostbyname_a( const char * pcHostName,
                                           FOnDNSEvent pCallback,
                                           void * pvSearchID,
                                           TickType_t uxTimeout )
        {
            uint32_t ulResult;
            struct freertos_addrinfo * pxAddressInfo = NULL;

            ulResult = prvPrepareLookup( pcHostName, &( pxAddressInfo ), FREERTOS_AF_INET4, pCallback, pvSearchID, uxTimeout );

            if( pxAddressInfo != NULL )
            {
                FreeRTOS_freeaddrinfo( pxAddressInfo );
            }

            return ulResult;
        }
    #endif /* if ( ipconfigDNS_USE_CALLBACKS == 0 ) */

    #if ( ipconfigINCLUDE_FULL_INET_ADDR == 1 )

/**
 * @brief See if pcHostName contains a valid IPv4 or IPv6 IP-address.
 * @param[in] pcHostName The name to be looked up
 * @param[in] xFamily the IP-type, either FREERTOS_AF_INET4 or FREERTOS_AF_INET6.
 * @param[in] ppxAddressInfo A pointer to a pointer where the find results will
 *                            be stored.
 * @return Either 0 or an IP=address.
 */
        static uint32_t prvPrepare_ReadIPAddress( const char * pcHostName,
                                                  BaseType_t xFamily,
                                                  struct freertos_addrinfo ** ppxAddressInfo )
        {
            uint32_t ulIPAddress = 0U;

            ( void ) xFamily;

            /* Check if the hostname given is actually an IP-address. */
            switch( xFamily ) /* LCOV_EXCL_BR_LINE - Family is always either FREERTOS_AF_INET or FREERTOS_AF_INET6. */
            {
                #if ( ipconfigUSE_IPv4 != 0 )
                    case FREERTOS_AF_INET:
                        ulIPAddress = FreeRTOS_inet_addr( pcHostName );

                        if( ( ulIPAddress != 0U ) && ( ppxAddressInfo != NULL ) )
                        {
                            const uint8_t * ucBytes = ( uint8_t * ) &( ulIPAddress );

                            *( ppxAddressInfo ) = pxNew_AddrInfo( pcHostName, FREERTOS_AF_INET4, ucBytes );
                        }
                        break;
                #endif /* ( ipconfigUSE_IPv4 != 0 ) */

                #if ( ipconfigUSE_IPv6 != 0 )
                    case FREERTOS_AF_INET6:
                       {
                           IPv6_Address_t xAddress_IPv6;
                           BaseType_t xResult;

                           /* ulIPAddress does not represent an IPv4 address here. It becomes non-zero when the look-up succeeds. */
                           xResult = FreeRTOS_inet_pton6( pcHostName, xAddress_IPv6.ucBytes );

                           if( xResult == 1 )
                           {
                               /* This function returns either a valid IPv4 address, or
                                * in case of an IPv6 lookup, it will return a non-zero */
                               ulIPAddress = 1U;

                               /* ppxAddressInfo is always non-NULL in IPv6 case. */
                               *( ppxAddressInfo ) = pxNew_AddrInfo( pcHostName, FREERTOS_AF_INET6, xAddress_IPv6.ucBytes );
                           }
                       }
                       break;
                #endif /* ( ipconfigUSE_IPv6 != 0 ) */

                default: /* LCOV_EXCL_LINE - Family is always either FREERTOS_AF_INET or FREERTOS_AF_INET6. */
                    /* MISRA 16.4 Compliance */
                    FreeRTOS_debug_printf( ( "prvPrepare_ReadIPAddress: Undefined xFamily Type \n" ) );
                    break; /* LCOV_EXCL_LINE - Family is always either FREERTOS_AF_INET or FREERTOS_AF_INET6. */
            }

            return ulIPAddress;
        }
    #endif /* ( ipconfigINCLUDE_FULL_INET_ADDR == 1 ) */
/*-----------------------------------------------------------*/

    #if ( ipconfigDNS_USE_CALLBACKS == 1 )

/**
 * @brief Check if hostname is already known. If not, call prvGetHostByName() to send a DNS request.
 *
 * @param[in] pcHostName The hostname whose IP address is being queried.
 * @param[in,out] ppxAddressInfo A pointer to a pointer where the find results
 *                will be stored.
 * @param [in] xFamily indicate what type of record is needed:
 *             FREERTOS_AF_INET4 or FREERTOS_AF_INET6.
 * @param[in] pCallbackFunction The callback function which will be called upon DNS response.
 * @param[in] pvSearchID Search ID for the callback function.
 * @param[in] uxTimeout Timeout for the callback function.
 * @return The IP-address corresponding to the hostname.
 */
        static uint32_t prvPrepareLookup( const char * pcHostName,
                                          struct freertos_addrinfo ** ppxAddressInfo,
                                          BaseType_t xFamily,
                                          FOnDNSEvent pCallbackFunction,
                                          void * pvSearchID,
                                          TickType_t uxTimeout )
    #else

/**
 * @brief Check if hostname is already known. If not, call prvGetHostByName() to send a DNS request.
 * @param[in] pcHostName The hostname whose IP address is being queried.
 * @return The IP-address corresponding to the hostname.
 */
        static uint32_t prvPrepareLookup( const char * pcHostName,
                                          struct freertos_addrinfo ** ppxAddressInfo,
                                          BaseType_t xFamily )
    #endif /* if ( ipconfigDNS_USE_CALLBACKS == 1 ) */
    {
        uint32_t ulIPAddress = 0U;
        TickType_t uxReadTimeOut_ticks = ipconfigDNS_RECEIVE_BLOCK_TIME_TICKS;

        /* Generate a unique identifier for this query. Keep it in a local variable
         * as gethostbyname() may be called from different threads */
        BaseType_t xHasRandom = pdFALSE;
        TickType_t uxIdentifier = 0U;

        #if ( ipconfigDNS_USE_CALLBACKS == 1 )
            BaseType_t xReturnSetCallback = pdPASS;
        #endif

        #if ( ipconfigUSE_DNS_CACHE != 0 )
            BaseType_t xLengthOk = pdFALSE;
        #endif

        #if ( ipconfigUSE_DNS_CACHE != 0 )
        {
            if( pcHostName != NULL )
            {
                size_t uxLength = strlen( pcHostName ) + 1U;

                if( uxLength <= ipconfigDNS_CACHE_NAME_LENGTH )
                {
                    /* The name is not too long. */
                    xLengthOk = pdTRUE;
                }
                else
                {
                    FreeRTOS_printf( ( "prvPrepareLookup: name is too long ( %u > %u )\n",
                                       ( unsigned ) uxLength,
                                       ( unsigned ) ipconfigDNS_CACHE_NAME_LENGTH ) );
                }
            }
        }

        if( ( pcHostName != NULL ) && ( xLengthOk != pdFALSE ) )
        #else /* if ( ipconfigUSE_DNS_CACHE != 0 ) */
            if( pcHostName != NULL )
        #endif /* ( ipconfigUSE_DNS_CACHE != 0 ) */
        {
            /* If the supplied hostname is an IP address, put it in ppxAddressInfo
             * and return. */
            #if ( ipconfigINCLUDE_FULL_INET_ADDR == 1 )
            {
                ulIPAddress = prvPrepare_ReadIPAddress( pcHostName, xFamily, ppxAddressInfo );
            }
            #endif /* ipconfigINCLUDE_FULL_INET_ADDR == 1 */

            /* If a DNS cache is used then check the cache before issuing another DNS
             * request. */
            #if ( ipconfigUSE_DNS_CACHE == 1 )
                /* Check the cache before issuing another DNS request. */
                if( ulIPAddress == 0U )
                {
                    ulIPAddress = Prepare_CacheLookup( pcHostName, xFamily, ppxAddressInfo );

                    if( ulIPAddress != 0UL )
                    {
                        #if ( ipconfigUSE_IPv6 != 0 )
                            if( ( ppxAddressInfo != NULL ) && ( *ppxAddressInfo != NULL ) && ( ( *ppxAddressInfo )->ai_family == FREERTOS_AF_INET6 ) )
                            {
                                FreeRTOS_printf( ( "prvPrepareLookup: found '%s' in cache: %pip\n",
                                                   pcHostName, ( void * ) ( *ppxAddressInfo )->xPrivateStorage.sockaddr.sin_address.xIP_IPv6.ucBytes ) );
                            }
                            else
                        #endif
                        {
                            FreeRTOS_printf( ( "prvPrepareLookup: found '%s' in cache: %xip\n", pcHostName, ( unsigned ) ulIPAddress ) );
                        }
                    }
                }
            #endif /* ipconfigUSE_DNS_CACHE == 1 */

            /* Generate a unique identifier. */
            if( ulIPAddress == 0U )
            {
                uint32_t ulNumber;

                xHasRandom = xApplicationGetRandomNumber( &( ulNumber ) );
                /* DNS identifiers are 16-bit. */
                uxIdentifier = ( TickType_t ) ( ulNumber & 0xffffU );
            }

            #if ( ipconfigDNS_USE_CALLBACKS == 1 )
            {
                if( pCallbackFunction != NULL )
                {
                    if( ulIPAddress == 0U )
                    {
                        /* The user has provided a callback function, so do not block on recvfrom() */
                        if( xHasRandom != pdFALSE )
                        {
                            uxReadTimeOut_ticks = 0U;
                            xReturnSetCallback = xDNSSetCallBack( pcHostName,
                                                                  pvSearchID,
                                                                  pCallbackFunction,
                                                                  uxTimeout,
                                                                  ( TickType_t ) uxIdentifier,
                                                                  ( xFamily == FREERTOS_AF_INET6 ) ? pdTRUE : pdFALSE );
                        }
                    }
                    else     /* When ipconfigDNS_USE_CALLBACKS enabled, ppxAddressInfo is always non null. */
                    {
                        /* The IP address is known, do the call-back now. */
                        pCallbackFunction( pcHostName, pvSearchID, *( ppxAddressInfo ) );
                    }
                }
            }
            #endif /* if ( ipconfigDNS_USE_CALLBACKS == 1 ) */

            if( ( ulIPAddress == 0U ) &&

                #if ( ipconfigDNS_USE_CALLBACKS == 1 )
                    ( xReturnSetCallback == pdPASS ) &&
                #endif

                ( xHasRandom != pdFALSE ) )
            {
                ulIPAddress = prvGetHostByName( pcHostName,
                                                uxIdentifier,
                                                uxReadTimeOut_ticks,
                                                ppxAddressInfo,
                                                xFamily );
            }
        }

        return ulIPAddress;
    }
    /*-----------------------------------------------------------*/

    #if ( ipconfigUSE_IPv6 != 0 )

/**
 * @brief Increment the field 'ucDNSIndex', which is an index in the array
 *        of DNS addresses.
 * @param[in] pxEndPoint The end-point of which the DNS index should be
 *                        incremented.
 */
        static void prvIncreaseDNS6Index( NetworkEndPoint_t * pxEndPoint )
        {
            uint8_t ucIndex = pxEndPoint->ipv6_settings.ucDNSIndex;
            uint8_t ucInitialIndex = ucIndex;

            for( ; ; )
            {
                ucIndex++;

                if( ucIndex >= ( uint8_t ) ipconfigENDPOINT_DNS_ADDRESS_COUNT )
                {
                    ucIndex = 0U;
                }

                if( ( pxEndPoint->ipv6_settings.xDNSServerAddresses[ ucIndex ].ucBytes[ 0 ] != 0U ) ||
                    ( ucInitialIndex == ucIndex ) )
                {
                    break;
                }
            }

            FreeRTOS_printf( ( "prvIncreaseDNS6Index: from %d to %d\n", ( int ) ucInitialIndex, ( int ) ucIndex ) );
            pxEndPoint->ipv6_settings.ucDNSIndex = ucIndex;
        }
    #endif /* ( ipconfigUSE_IPv6 != 0 ) */
/*-----------------------------------------------------------*/

    #if ( ipconfigUSE_IPv4 != 0 )

/**
 * @brief Increment the field 'ucDNSIndex', which is an index in the array
 *        of DNS addresses.
 * @param[in] pxEndPoint The end-point of which the DNS index should be
 *                        incremented.
 */
        static void prvIncreaseDNS4Index( NetworkEndPoint_t * pxEndPoint )
        {
            uint8_t ucIndex = pxEndPoint->ipv4_settings.ucDNSIndex;
            uint8_t ucInitialIndex = ucIndex;

            for( ; ; )
            {
                ucIndex++;

                if( ucIndex >= ( uint8_t ) ipconfigENDPOINT_DNS_ADDRESS_COUNT )
                {
                    ucIndex = 0U;
                }

                if( ( pxEndPoint->ipv4_settings.ulDNSServerAddresses[ ucIndex ] != 0U ) ||
                    ( ucInitialIndex == ucIndex ) )
                {
                    break;
                }
            }

            FreeRTOS_printf( ( "prvIncreaseDNS4Index: from %d to %d\n", ( int ) ucInitialIndex, ( int ) ucIndex ) );
            pxEndPoint->ipv4_settings.ucDNSIndex = ucIndex;
        }
/*-----------------------------------------------------------*/
    #endif /* #if ( ipconfigUSE_IPv4 != 0 ) */

/*!
 * @brief create a payload buffer and return it through the parameter
 * @param [out] ppxNetworkBuffer network buffer to create
 * @param [in] pcHostName hostname to get its length
 * @param [in] uxHeaderBytes Size of the header (IPv4/IPv6)
 * @returns pointer address to the payload buffer
 *
 */
    static uint8_t * prvGetPayloadBuffer( NetworkBufferDescriptor_t ** ppxNetworkBuffer,
                                          const char * pcHostName,
                                          size_t uxHeaderBytes )
    {
        size_t uxExpectedPayloadLength;
        uint8_t * pucUDPPayloadBuffer = NULL;

        uxExpectedPayloadLength = sizeof( DNSMessage_t ) +
                                  strlen( pcHostName ) +
                                  sizeof( uint16_t ) +
                                  sizeof( uint16_t ) + 2U;

        /* Get a buffer.  This uses a maximum delay, but the delay will be
         * capped to ipconfigUDP_MAX_SEND_BLOCK_TIME_TICKS so the return value
         * still needs to be tested. */
        *ppxNetworkBuffer = pxGetNetworkBufferWithDescriptor( uxExpectedPayloadLength +
                                                              uxHeaderBytes,
                                                              0U );

        if( *ppxNetworkBuffer != NULL )
        {
            pucUDPPayloadBuffer = &( ( *ppxNetworkBuffer )->pucEthernetBuffer[ uxHeaderBytes ] );
        }

        return pucUDPPayloadBuffer;
    }

/*!
 * @brief fill  pxAddress from pucUDPPayloadBuffer
 * @param [out] pxAddress ip address and port ... structure
 * @param [in]  pcHostName hostname to get its length
 * @return The end-point that holds the DNS address.
 */
    static NetworkEndPoint_t * prvFillSockAddress( struct freertos_sockaddr * pxAddress,
                                                   const char * pcHostName )
    {
        NetworkEndPoint_t * pxEndPoint = NULL;

        /* If LLMNR is being used then determine if the host name includes a '.' -
         * if not then LLMNR can be used as the lookup method. */
        /* For local resolution, mDNS uses names ending with the string ".local" */
        BaseType_t bHasDot = pdFALSE;
        BaseType_t bHasLocal = pdFALSE;
        const char * pcDot = ( const char * ) strchr( pcHostName, ( int32_t ) '.' );

        #if ( ipconfigUSE_LLMNR != 1 )
            ( void ) pcHostName;
        #endif

        /* Make sure all fields of the 'sockaddr' are cleared. */
        ( void ) memset( ( void * ) pxAddress, 0, sizeof( *pxAddress ) );

        /* And set the address type to IPv4.
         * It may change to IPv6 in case an IPv6 DNS server will be used. */
        pxAddress->sin_family = FREERTOS_AF_INET;

        /* 'sin_len' doesn't really matter, 'sockaddr' and 'sockaddr6'
         * have the same size. */
        pxAddress->sin_len = ( uint8_t ) sizeof( struct freertos_sockaddr );
        /* Use the DNS port by default, this may be changed later. */
        pxAddress->sin_port = dnsDNS_PORT;

        if( pcDot != NULL )
        {
            bHasDot = pdTRUE;

            if( strcmp( pcDot, ".local" ) == 0 )
            {
                bHasLocal = pdTRUE;
            }
            else
            {
                /* a DNS look-up of a public URL with at least one dot. */
            }
        }

        /* Is this a local lookup? */
        if( ( bHasDot == pdFALSE ) || ( bHasLocal == pdTRUE ) )
        {
            /* Looking for e.g. "mydevice" or "mydevice.local",
             * while using either mDNS or LLMNR. */
            #if ( ipconfigUSE_MDNS == 1 )
            {
                if( bHasLocal )
                {
                    /* Looking up a name like "mydevice.local".
                     * Use mDNS addresses. */

                    pxAddress->sin_port = ipMDNS_PORT;
                    pxAddress->sin_port = FreeRTOS_ntohs( pxAddress->sin_port );

                    switch( xDNS_IP_Preference ) /* LCOV_EXCL_BR_LINE - xDNS_IP_Preference can be either xPreferenceIPv4 or xPreferenceIPv6 */
                    {
                        #if ( ipconfigUSE_IPv4 != 0 )
                            case xPreferenceIPv4:
                                pxAddress->sin_address.ulIP_IPv4 = ipMDNS_IP_ADDRESS;     /* Is in network byte order. */
                                /* sin_family is default set to FREERTOS_AF_INET */
                                break;
                        #endif /* ( ipconfigUSE_IPv4 != 0 ) */

                        #if ( ipconfigUSE_IPv6 != 0 )
                            case xPreferenceIPv6:
                                memcpy( pxAddress->sin_address.xIP_IPv6.ucBytes,
                                        ipMDNS_IP_ADDR_IPv6.ucBytes,
                                        ipSIZE_OF_IPv6_ADDRESS );
                                pxAddress->sin_family = FREERTOS_AF_INET6;
                                break;
                        #endif /* ( ipconfigUSE_IPv6 != 0 ) */

                        default: /* LCOV_EXCL_LINE - xDNS_IP_Preference can be either xPreferenceIPv4 or xPreferenceIPv6 */
                            /* MISRA 16.4 Compliance */
                            FreeRTOS_debug_printf( ( "prvFillSockAddress: Undefined xDNS_IP_Preference \n" ) );
                            break; /* LCOV_EXCL_LINE - xDNS_IP_Preference can be either xPreferenceIPv4 or xPreferenceIPv6 */
                    }
                }
            }
            #endif /* if ( ipconfigUSE_MDNS == 1 ) */
            #if ( ipconfigUSE_LLMNR == 1 )
            {
                /* The hostname doesn't have a dot. */
                if( bHasDot == pdFALSE )
                {
                    /* Use LLMNR addressing. */
                    pxAddress->sin_port = ipLLMNR_PORT;
                    pxAddress->sin_port = FreeRTOS_ntohs( pxAddress->sin_port );

                    switch( xDNS_IP_Preference ) /* LCOV_EXCL_LINE - xDNS_IP_Preference can be either xPreferenceIPv4 or xPreferenceIPv6 */
                    {
                        #if ( ipconfigUSE_IPv4 != 0 )
                            case xPreferenceIPv4:
                                pxAddress->sin_address.ulIP_IPv4 = ipLLMNR_IP_ADDR;     /* Is in network byte order. */
                                pxAddress->sin_family = FREERTOS_AF_INET;
                                break;
                        #endif /* ( ipconfigUSE_IPv4 != 0 ) */

                        #if ( ipconfigUSE_IPv6 != 0 )
                            case xPreferenceIPv6:
                                memcpy( pxAddress->sin_address.xIP_IPv6.ucBytes,
                                        ipLLMNR_IP_ADDR_IPv6.ucBytes,
                                        ipSIZE_OF_IPv6_ADDRESS );
                                pxAddress->sin_family = FREERTOS_AF_INET6;
                                break;
                        #endif /* ( ipconfigUSE_IPv6 != 0 ) */

                        default: /* LCOV_EXCL_LINE - xDNS_IP_Preference can be either xPreferenceIPv4 or xPreferenceIPv6 */
                            /* MISRA 16.4 Compliance */
                            FreeRTOS_debug_printf( ( "prvFillSockAddress: Undefined xDNS_IP_Preference (LLMNR) \n" ) );
                            break; /* LCOV_EXCL_LINE - xDNS_IP_Preference can be either xPreferenceIPv4 or xPreferenceIPv6 */
                    }
                }
            }
            #endif /* if ( ipconfigUSE_LLMNR == 1 ) */

            #if ( ipconfigUSE_MDNS == 1 ) || ( ipconfigUSE_LLMNR == 1 )
                for( pxEndPoint = FreeRTOS_FirstEndPoint( NULL );
                     pxEndPoint != NULL;
                     pxEndPoint = FreeRTOS_NextEndPoint( NULL, pxEndPoint ) )
                {
                    #if ( ipconfigUSE_IPv6 != 0 )
                        if( xDNS_IP_Preference == xPreferenceIPv6 )
                        {
                            if( pxEndPoint->bits.bIPv6 != 0U )
                            {
                                break;
                            }
                        }
                        else
                        {
                            #if ( ipconfigUSE_IPv4 != 0 )
                                if( pxEndPoint->bits.bIPv6 == 0U )
                                {
                                    break;
                                }
                            #endif /* if ( ipconfigUSE_IPv4 != 0 ) */
                        }
                    #else /* if ( ipconfigUSE_IPv6 != 0 ) */
                        /* IPv6 is not included, so all end-points are IPv4. */
                        break;
                    #endif /* if ( ipconfigUSE_IPv6 != 0 ) */
                }
            #endif /* if ( ipconfigUSE_MDNS == 1 ) || ( ipconfigUSE_LLMNR == 1 ) */
        }
        else
        {
            BaseType_t xBreakLoop = pdFALSE;

            /* Look for an end-point that has defined a DNS server address. */
            for( pxEndPoint = FreeRTOS_FirstEndPoint( NULL );
                 pxEndPoint != NULL;
                 pxEndPoint = FreeRTOS_NextEndPoint( NULL, pxEndPoint ) )
            {
                switch( xDNS_IP_Preference ) /* LCOV_EXCL_LINE - xDNS_IP_Preference can be either xPreferenceIPv4 or xPreferenceIPv6 */
                {
                    #if ( ipconfigUSE_IPv4 != 0 )
                        case xPreferenceIPv4:

                            if( pxEndPoint->bits.bIPv6 == 0U )
                            {
                                uint32_t ulIPAddress;
                                uint8_t ucIndex = pxEndPoint->ipv4_settings.ucDNSIndex;
                                configASSERT( ucIndex < ipconfigENDPOINT_DNS_ADDRESS_COUNT );
                                ulIPAddress = pxEndPoint->ipv4_settings.ulDNSServerAddresses[ ucIndex ];

                                if( ( ulIPAddress != 0U ) && ( ulIPAddress != ipBROADCAST_IP_ADDRESS ) )
                                {
                                    pxAddress->sin_family = FREERTOS_AF_INET;
                                    pxAddress->sin_len = ( uint8_t ) sizeof( struct freertos_sockaddr );
                                    pxAddress->sin_address.ulIP_IPv4 = ulIPAddress;
                                    xBreakLoop = pdTRUE;
                                }
                            }
                            break;
                    #endif /* ( ipconfigUSE_IPv4 != 0 ) */

                    #if ( ipconfigUSE_IPv6 != 0 )
                        case xPreferenceIPv6:

                            if( pxEndPoint->bits.bIPv6 != 0U )
                            {
                                const uint8_t * ucBytes;
                                uint8_t ucIndex = pxEndPoint->ipv6_settings.ucDNSIndex;
                                configASSERT( ucIndex < ipconfigENDPOINT_DNS_ADDRESS_COUNT );
                                ucBytes = pxEndPoint->ipv6_settings.xDNSServerAddresses[ ucIndex ].ucBytes;

                                /* Test if the DNS entry is in used. */
                                if( ( ucBytes[ 0 ] != 0U ) && ( ucBytes[ 1 ] != 0U ) )
                                {
                                    pxAddress->sin_family = FREERTOS_AF_INET6;
                                    pxAddress->sin_len = ( uint8_t ) sizeof( struct freertos_sockaddr );
                                    ( void ) memcpy( pxAddress->sin_address.xIP_IPv6.ucBytes,
                                                     pxEndPoint->ipv6_settings.xDNSServerAddresses[ ucIndex ].ucBytes,
                                                     ipSIZE_OF_IPv6_ADDRESS );
                                    xBreakLoop = pdTRUE;
                                }
                            }
                            break;
                    #endif /* ( ipconfigUSE_IPv6 != 0 ) */

                    default: /* LCOV_EXCL_LINE - xDNS_IP_Preference can be either xPreferenceIPv4 or xPreferenceIPv6 */
                        /* MISRA 16.4 Compliance */
                        FreeRTOS_debug_printf( ( "prvFillSockAddress: Undefined xDNS_IP_Preference \n" ) );
                        break; /* LCOV_EXCL_LINE - xDNS_IP_Preference can be either xPreferenceIPv4 or xPreferenceIPv6 */
                }

                if( xBreakLoop == pdTRUE )
                {
                    break;
                }
            }
        }

        return pxEndPoint;
    }

/*!
 * @brief return ip address from the dns reply message
 * @param [in] pxReceiveBuffer received buffer from the DNS server
 * @param[in,out] ppxAddressInfo A pointer to a pointer where the find results
 *                will be stored.
 * @param [in] uxIdentifier matches sent and received packets
 * @param [in] usPort Port from which DNS reply was read
 * @returns ip address or zero on error
 *
 */
    static uint32_t prvDNSReply( const struct xDNSBuffer * pxReceiveBuffer,
                                 struct freertos_addrinfo ** ppxAddressInfo,
                                 TickType_t uxIdentifier,
                                 uint16_t usPort )
    {
        uint32_t ulIPAddress = 0U;
        BaseType_t xExpected;

        /* MISRA Ref 11.3.1 [Misaligned access] */
        /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
        /* coverity[misra_c_2012_rule_11_3_violation] */
        const DNSMessage_t * pxDNSMessageHeader = ( const DNSMessage_t * ) pxReceiveBuffer->pucPayloadBuffer;

        #if ( ipconfigUSE_MDNS == 1 )
            /* _HT_ changed 'pxReceiveBuffer->sin_port' to 'usPort' */
            if( FreeRTOS_ntohs( usPort ) == ipMDNS_PORT )             /* mDNS port 5353. */
            {
                /* In mDNS, the query ID field is ignored. */
                xExpected = pdTRUE;
            }
            else
        #endif /* if ( ipconfigUSE_MDNS == 1 ) */

        /* See if the identifiers match. */
        if( uxIdentifier == ( TickType_t ) pxDNSMessageHeader->usIdentifier )
        {
            xExpected = pdTRUE;
        }
        else
        {
            xExpected = pdFALSE;
        }

        /* The reply was received.  Process it. */
        #if ( ipconfigDNS_USE_CALLBACKS == 0 )

            /* It is useless to analyse the unexpected reply
             * unless asynchronous look-ups are enabled. */
            if( xExpected != pdFALSE )
        #endif /* ipconfigDNS_USE_CALLBACKS == 0 */
        {
            ulIPAddress = DNS_ParseDNSReply( pxReceiveBuffer->pucPayloadBuffer,
                                             pxReceiveBuffer->uxPayloadLength,
                                             ppxAddressInfo,
                                             xExpected,
                                             usPort );
        }

        return ulIPAddress;
    }

/*!
 * @brief prepare the buffer before sending
 * @param [in] pcHostName hostname to be looked up
 * @param [in] uxIdentifier  matches sent and received packets
 * @param [in] xDNSSocket a valid socket
 * @param [in] xFamily indicate what type of record is needed:
 *             FREERTOS_AF_INET4 or FREERTOS_AF_INET6.
 * @param [in] pxAddress address structure
 * @returns pdTRUE if sending the data was successful, pdFALSE otherwise.
 */
    static BaseType_t prvSendBuffer( const char * pcHostName,
                                     TickType_t uxIdentifier,
                                     Socket_t xDNSSocket,
                                     BaseType_t xFamily,
                                     const struct freertos_sockaddr * pxAddress )
    {
        BaseType_t xReturn = pdFAIL;
        struct xDNSBuffer xDNSBuf = { 0 };
        NetworkBufferDescriptor_t * pxNetworkBuffer = NULL;
        size_t uxHeaderBytes;
        UBaseType_t uxHostType;

        /* Calculate the size of the headers. */
        if( pxAddress->sin_family == ( uint8_t ) FREERTOS_AF_INET6 )
        {
            uxHeaderBytes = ipSIZE_OF_ETH_HEADER + ipSIZE_OF_IPv6_HEADER + ipSIZE_OF_UDP_HEADER;
        }
        else
        {
            uxHeaderBytes = ipSIZE_OF_ETH_HEADER + ipSIZE_OF_IPv4_HEADER + ipSIZE_OF_UDP_HEADER;
        }

        if( xFamily == FREERTOS_AF_INET6 )
        {
            /* Note that 'dnsTYPE_ANY_HOST' could be used here as well,
             * but for testing, we want an IPv6 address. */
            uxHostType = dnsTYPE_AAAA_HOST;
        }
        else
        {
            uxHostType = dnsTYPE_A_HOST;
        }

        /*get dns message buffer */
        xDNSBuf.pucPayloadBuffer = prvGetPayloadBuffer( &pxNetworkBuffer,
                                                        pcHostName, uxHeaderBytes );

        if( xDNSBuf.pucPayloadBuffer != NULL )
        {
            /* A two-step conversion to conform to MISRA. */
            size_t uxIndex = ipUDP_PAYLOAD_IP_TYPE_OFFSET;
            BaseType_t xIndex = ( BaseType_t ) uxIndex;

            #if ( ipconfigUSE_LLMNR == 1 )
            {
                if( FreeRTOS_ntohs( pxAddress->sin_port ) == ipLLMNR_PORT )
                {
                    /* MISRA Ref 11.3.1 [Misaligned access] */
                    /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
                    /* coverity[misra_c_2012_rule_11_3_violation] */
                    ( ( ( DNSMessage_t * ) xDNSBuf.pucPayloadBuffer ) )->usFlags = 0;
                }
            }
            #endif

            /* Later when translating form UDP payload to a Network Buffer,
             * it is important to know whether this is an IPv4 packet. */
            if( pxAddress->sin_family == ( uint8_t ) FREERTOS_AF_INET6 )
            {
                xDNSBuf.pucPayloadBuffer[ -xIndex ] = ( uint8_t ) ipTYPE_IPv6;
            }
            else
            {
                xDNSBuf.pucPayloadBuffer[ -xIndex ] = ( uint8_t ) ipTYPE_IPv4;
            }

            xDNSBuf.uxPayloadLength = prvCreateDNSMessage( xDNSBuf.pucPayloadBuffer,
                                                           pcHostName,
                                                           uxIdentifier,
                                                           uxHostType );

            /* ipLLMNR_IP_ADDR is in network byte order. */
            if( ( pxAddress->sin_address.ulIP_IPv4 == ipLLMNR_IP_ADDR ) || ( pxAddress->sin_address.ulIP_IPv4 == ipMDNS_IP_ADDRESS ) )
            {
                /* Use LLMNR addressing. */
                /* MISRA Ref 11.3.1 [Misaligned access] */
                /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
                /* coverity[misra_c_2012_rule_11_3_violation] */
                ( ( ( DNSMessage_t * ) xDNSBuf.pucPayloadBuffer ) )->usFlags = 0;
            }

            /* send the dns message */
            xReturn = DNS_SendRequest( xDNSSocket,
                                       pxAddress,
                                       &xDNSBuf );

            if( xReturn == pdFAIL )
            {
                vReleaseNetworkBufferAndDescriptor( pxNetworkBuffer );
            }
        }

        return xReturn;
    }

/*!
 * @brief main dns operation description function
 * @param [in] pcHostName hostname to get its ip address
 * @param [in] uxIdentifier Identifier to match sent and received packets
 * @param [in] xDNSSocket socket
 * @param[in,out] ppxAddressInfo A pointer to a pointer where the find results
 *                will be stored.
 * @param[in] xFamily Either FREERTOS_AF_INET4 or FREERTOS_AF_INET6.
 * @param[in] uxReadTimeOut_ticks The timeout in ticks for waiting. In case the user has supplied
 *                                 a call-back function, this value should be zero.
 * @returns ip address or zero on error
 */
    static uint32_t prvGetHostByNameOp( const char * pcHostName,
                                        TickType_t uxIdentifier,
                                        Socket_t xDNSSocket,
                                        struct freertos_addrinfo ** ppxAddressInfo,
                                        BaseType_t xFamily,
                                        TickType_t uxReadTimeOut_ticks )
    {
        uint32_t ulIPAddress = 0;
        struct freertos_sockaddr xAddress;
        struct freertos_sockaddr xRecvAddress;
        DNSBuffer_t xReceiveBuffer = { 0 };
        BaseType_t xReturn = pdFAIL;
        BaseType_t xBytes;
        NetworkEndPoint_t * pxEndPoint;

        /* Make sure all fields of the 'sockaddr' are cleared. */
        ( void ) memset( ( void * ) &xAddress, 0, sizeof( xAddress ) );

        pxEndPoint = prvFillSockAddress( &xAddress, pcHostName );

        if( pxEndPoint != NULL )
        {
            do
            {
                if( xDNSSocket->usLocalPort == 0U )
                {
                    /* Bind the client socket to a random port number. */
                    uint16_t usPort = 0U;
                    #if ( ipconfigUSE_MDNS == 1 )
                        if( xAddress.sin_port == FreeRTOS_htons( ipMDNS_PORT ) )
                        {
                            /* For a mDNS lookup, bind to the mDNS port 5353. */
                            usPort = FreeRTOS_htons( ipMDNS_PORT );
                        }
                    #endif

                    if( DNS_BindSocket( xDNSSocket, usPort ) != 0 )
                    {
                        FreeRTOS_printf( ( "DNS bind to %u failed\n", FreeRTOS_ntohs( usPort ) ) );
                        break;
                    }
                }

                xReturn = prvSendBuffer( pcHostName,
                                         uxIdentifier,
                                         xDNSSocket,
                                         xFamily,
                                         &xAddress );

                if( xReturn == pdFAIL )
                {
                    break;
                }

                /* Create the message in the obtained buffer. */

                /* receive a dns reply message */
                xBytes = DNS_ReadReply( xDNSSocket,
                                        &xRecvAddress,
                                        &xReceiveBuffer );

                if( ( uxReadTimeOut_ticks > 0U ) &&
                    ( ( xBytes == -pdFREERTOS_ERRNO_EWOULDBLOCK ) ||
                      ( xBytes == 0 ) ) )
                {
                    /* This search timed out, next time try with a different DNS. */
                    switch( xAddress.sin_family ) /* LCOV_EXCL_BR_LINE - This is filled by static function, default case is impossible to reach. */
                    {
                        #if ( ipconfigUSE_IPv4 != 0 )
                            case FREERTOS_AF_INET:
                                prvIncreaseDNS4Index( pxEndPoint );
                                break;
                        #endif /* ( ipconfigUSE_IPv4 != 0 ) */

                        #if ( ipconfigUSE_IPv6 != 0 )
                            case FREERTOS_AF_INET6:
                                prvIncreaseDNS6Index( pxEndPoint );
                                break;
                        #endif /* ( ipconfigUSE_IPv6 != 0 ) */

                        default: /* LCOV_EXCL_LINE - This is filled by static function, default case is impossible to reach. */
                            /* MISRA 16.4 Compliance */
                            FreeRTOS_debug_printf( ( "prvGetHostByNameOp: Undefined sin_family \n" ) );
                            break; /* LCOV_EXCL_LINE - This is filled by static function, default case is impossible to reach. */
                    }
                }

                if( xReceiveBuffer.pucPayloadBuffer != NULL )
                {
                    if( xBytes > 0 )
                    {
                        xReceiveBuffer.uxPayloadLength = ( size_t ) xBytes;

                        /* MISRA Ref 4.14.2 [The validity of values received from external sources]. */
                        /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#directive-414. */
                        /* coverity[misra_c_2012_directive_4_14_violation] */
                        ulIPAddress = prvDNSReply( &xReceiveBuffer, ppxAddressInfo, uxIdentifier, xRecvAddress.sin_port );
                    }

                    /* Finished with the buffer.  The zero copy interface
                     * is being used, so the buffer must be freed by the
                     * task. */
                    FreeRTOS_ReleaseUDPPayloadBuffer( xReceiveBuffer.pucPayloadBuffer );
                }
            } while( ipFALSE_BOOL );
        }
        else
        {
            /* No endpoint was found that defines a DNS address. */
            FreeRTOS_printf( ( "Can not find a DNS address, along with an end-point.\n" ) );
        }

        return ulIPAddress;
    }

/*!
 * @brief helper function to prvGetHostByNameOP with multiple retries equal to
 *        ipconfigDNS_REQUEST_ATTEMPTS
 *
 * @param [in] pcHostName hostname to get its ip address
 * @param [in] uxIdentifier Identifier to match sent and received packets
 * @param [in] xDNSSocket socket
 * @param[in,out] ppxAddressInfo A pointer to a pointer where the find results
 *                will be stored.
 * @param[in] xFamily Either FREERTOS_AF_INET4 or FREERTOS_AF_INET6.
 * @param[in] uxReadTimeOut_ticks The timeout in ticks for waiting. In case the user has supplied
 *                                 a call-back function, this value should be zero.
 * @returns ip address or zero on error
 *
 */
    static uint32_t prvGetHostByNameOp_WithRetry( const char * pcHostName,
                                                  TickType_t uxIdentifier,
                                                  Socket_t xDNSSocket,
                                                  struct freertos_addrinfo ** ppxAddressInfo,
                                                  BaseType_t xFamily,
                                                  TickType_t uxReadTimeOut_ticks )
    {
        uint32_t ulIPAddress = 0;
        BaseType_t xAttempt;

        for( xAttempt = 0; xAttempt < ipconfigDNS_REQUEST_ATTEMPTS; xAttempt++ )
        {
            ulIPAddress = prvGetHostByNameOp( pcHostName,
                                              uxIdentifier,
                                              xDNSSocket,
                                              ppxAddressInfo,
                                              xFamily,
                                              uxReadTimeOut_ticks );

            if( ulIPAddress != 0U )
            { /* ip found, no need to retry */
                break;
            }
        }

        return ulIPAddress;
    }


/**
 * @brief Prepare and send a message to a DNS server.  'uxReadTimeOut_ticks' will be passed as
 *        zero, in case the user has supplied a call-back function.
 *
 * @param[in] pcHostName The hostname for which an IP address is required.
 * @param[in] uxIdentifier Identifier to match sent and received packets
 * @param[in] uxReadTimeOut_ticks The timeout in ticks for waiting. In case the user has supplied
 *                                 a call-back function, this value should be zero.
 * @param[in,out] ppxAddressInfo A pointer to a pointer where the find results
 *                will be stored.
 * @param[in] xFamily Either FREERTOS_AF_INET4 or FREERTOS_AF_INET6.
 * @return The IPv4 IP address for the hostname being queried. It will be zero if there is no reply.
 */
    static uint32_t prvGetHostByName( const char * pcHostName,
                                      TickType_t uxIdentifier,
                                      TickType_t uxReadTimeOut_ticks,
                                      struct freertos_addrinfo ** ppxAddressInfo,
                                      BaseType_t xFamily )
    {
        Socket_t xDNSSocket;
        uint32_t ulIPAddress = 0U;


        xDNSSocket = DNS_CreateSocket( uxReadTimeOut_ticks );

        if( xDNSSocket != NULL )
        {
            if( uxReadTimeOut_ticks == 0U )
            {
                /* xRetryIndex is negative to tell that the socket is non-blocking. */
                ulIPAddress = prvGetHostByNameOp( pcHostName,
                                                  uxIdentifier,
                                                  xDNSSocket,
                                                  ppxAddressInfo,
                                                  xFamily,
                                                  uxReadTimeOut_ticks );
            }
            else
            {
                ulIPAddress = prvGetHostByNameOp_WithRetry( pcHostName,
                                                            uxIdentifier,
                                                            xDNSSocket,
                                                            ppxAddressInfo,
                                                            xFamily,
                                                            uxReadTimeOut_ticks );
            }

            /* Finished with the socket. */
            DNS_CloseSocket( xDNSSocket );
        }

        return ulIPAddress;
    }
    /*-----------------------------------------------------------*/

/**
 * @brief Create the DNS message in the zero copy buffer passed in the first parameter.
 * @param[in,out] pucUDPPayloadBuffer The zero copy buffer where the DNS message will be created.
 * @param[in] pcHostName Hostname to be looked up.
 * @param[in] uxIdentifier Identifier to match sent and received packets
 * @param[in] uxHostType dnsTYPE_A_HOST ( IPv4 ) or dnsTYPE_AAAA_HOST ( IPv6 ).
 * @return Total size of the generated message, which is the space from the last written byte
 *         to the beginning of the buffer.
 */
    static size_t prvCreateDNSMessage( uint8_t * pucUDPPayloadBuffer,
                                       const char * pcHostName,
                                       TickType_t uxIdentifier,
                                       UBaseType_t uxHostType )
    {
        DNSMessage_t * pxDNSMessageHeader;
        size_t uxStart, uxIndex;
        DNSTail_t const * pxTail;
        static const DNSMessage_t xDefaultPartDNSHeader =
        {
            0,                 /* The identifier will be overwritten. */
            dnsOUTGOING_FLAGS, /* Flags set for standard query. */
            dnsONE_QUESTION,   /* One question is being asked. */
            0,                 /* No replies are included. */
            0,                 /* No authorities. */
            0                  /* No additional authorities. */
        };

        /* memcpy() helper variables for MISRA Rule 21.15 compliance*/
        const void * pvCopySource;
        void * pvCopyDest;

        ( void ) uxHostType;

        /* Copy in the const part of the header. Intentionally using different
         * pointers with memcpy() to put the information in to correct place. */

        /*
         * Use helper variables for memcpy() to remain
         * compliant with MISRA Rule 21.15.  These should be
         * optimized away.
         */
        pvCopySource = &xDefaultPartDNSHeader;
        pvCopyDest = pucUDPPayloadBuffer;
        ( void ) memcpy( pvCopyDest, pvCopySource, sizeof( xDefaultPartDNSHeader ) );

        /* Write in a unique identifier. Cast the Payload Buffer to DNSMessage_t
         * to easily access fields of the DNS Message. */

        /* MISRA Ref 11.3.1 [Misaligned access] */
        /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
        /* coverity[misra_c_2012_rule_11_3_violation] */
        pxDNSMessageHeader = ( ( DNSMessage_t * ) pucUDPPayloadBuffer );
        pxDNSMessageHeader->usIdentifier = ( uint16_t ) uxIdentifier;

        /* Create the resource record at the end of the header.  First
         * find the end of the header. */
        uxStart = sizeof( xDefaultPartDNSHeader );

        /* Leave a gap for the first length byte. */
        uxIndex = uxStart + 1U;

        /* Copy in the host name. */
        ( void ) strncpy( ( char * ) &( pucUDPPayloadBuffer[ uxIndex ] ), pcHostName, strlen( pcHostName ) + 1U );

        /* Walk through the string to replace the '.' characters with byte
         * counts.  pucStart holds the address of the byte count.  Walking the
         * string starts after the byte count position. */
        uxIndex = uxStart;

        do
        {
            size_t uxLength;

            /* Skip the length byte. */
            uxIndex++;

            while( ( pucUDPPayloadBuffer[ uxIndex ] != ( uint8_t ) 0U ) &&
                   ( pucUDPPayloadBuffer[ uxIndex ] != ( uint8_t ) ASCII_BASELINE_DOT ) )
            {
                uxIndex++;
            }

            /* Fill in the byte count, then move the pucStart pointer up to
             * the found byte position. */
            uxLength = uxIndex - ( uxStart + 1U );
            pucUDPPayloadBuffer[ uxStart ] = ( uint8_t ) uxLength;

            uxStart = uxIndex;
        } while( pucUDPPayloadBuffer[ uxIndex ] != ( uint8_t ) 0U );

        /* Finish off the record. Cast the record onto DNSTail_t structure to easily
         * access the fields of the DNS Message. */

        /* MISRA Ref 11.3.1 [Misaligned access] */
        /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
        /* coverity[misra_c_2012_rule_11_3_violation] */
        pxTail = ( ( DNSTail_t * ) &( pucUDPPayloadBuffer[ uxStart + 1U ] ) );

        #if defined( _lint ) || defined( __COVERITY__ )
            ( void ) pxTail;
        #else
            vSetField16( pxTail, DNSTail_t, usType, ( uint16_t ) uxHostType );
            vSetField16( pxTail, DNSTail_t, usClass, dnsCLASS_IN );
        #endif

        /* Return the total size of the generated message, which is the space from
         * the last written byte to the beginning of the buffer. */
        return uxIndex + sizeof( DNSTail_t ) + 1U;
    }

/*-----------------------------------------------------------*/

/* The function below will only be called :
 * when ipconfigDNS_USE_CALLBACKS == 1
 * when ipconfigUSE_LLMNR == 1
 * for testing purposes, by the module test_freertos_tcp.c
 */

/**
 * @brief Perform some preliminary checks and then parse the DNS packet.
 * @param[in] pxNetworkBuffer The network buffer to be parsed.
 * @return Always pdFAIL to indicate that the packet was not consumed and must
 *         be released by the caller.
 */
    uint32_t ulDNSHandlePacket( const NetworkBufferDescriptor_t * pxNetworkBuffer )
    {
        uint8_t * pucPayLoadBuffer;
        size_t uxPayloadSize;
        size_t uxUDPPacketSize = ipSIZE_OF_ETH_HEADER + uxIPHeaderSizePacket( pxNetworkBuffer ) + ipSIZE_OF_UDP_HEADER;

        /* Only proceed if the payload length indicated in the header
         * appears to be valid. */
        if( pxNetworkBuffer->xDataLength >= uxUDPPacketSize )
        {
            uxPayloadSize = pxNetworkBuffer->xDataLength - uxUDPPacketSize;

            if( uxPayloadSize >= sizeof( DNSMessage_t ) )
            {
                struct freertos_addrinfo * pxAddressInfo = NULL;
                pucPayLoadBuffer = &( pxNetworkBuffer->pucEthernetBuffer[ uxUDPPacketSize ] );

                /* The parameter pdFALSE indicates that the reply was not expected. */
                ( void ) DNS_ParseDNSReply( pucPayLoadBuffer,
                                            uxPayloadSize,
                                            &( pxAddressInfo ),
                                            pdFALSE,
                                            FreeRTOS_ntohs( pxNetworkBuffer->usPort ) );

                if( pxAddressInfo != NULL )
                {
                    FreeRTOS_freeaddrinfo( pxAddressInfo );
                }
            }
        }

        /* The packet was not consumed. */
        return pdFAIL;
    }
    /*-----------------------------------------------------------*/


    #if ( ipconfigUSE_NBNS == 1 )

/**
 * @brief Handle an NBNS packet.
 * @param[in] pxNetworkBuffer The network buffer holding the NBNS packet.
 * @return pdFAIL to show that the packet was not consumed.
 */
        uint32_t ulNBNSHandlePacket( NetworkBufferDescriptor_t * pxNetworkBuffer )
        {
            UDPPacket_t * pxUDPPacket = ( ( UDPPacket_t * )
                                          pxNetworkBuffer->pucEthernetBuffer );
            uint8_t * pucUDPPayloadBuffer = &( pxNetworkBuffer->pucEthernetBuffer[ sizeof( *pxUDPPacket ) ] );

            size_t uxBytesNeeded = sizeof( UDPPacket_t ) + sizeof( NBNSRequest_t );

            /* Check for minimum buffer size. */
            if( pxNetworkBuffer->xDataLength >= uxBytesNeeded )
            {
                DNS_TreatNBNS( pucUDPPayloadBuffer,
                               pxNetworkBuffer->xDataLength,
                               pxUDPPacket->xIPHeader.ulSourceIPAddress );
            }

            /* The packet was not consumed. */
            return pdFAIL;
        }

    #endif /* ipconfigUSE_NBNS */

/*-----------------------------------------------------------*/

/**
 * @brief Sets the DNS IP preference while doing DNS lookup to indicate the preference
 * for a DNS server: either IPv4 or IPv6. Defaults to xPreferenceIPv4
 * @param[in] eIPPreference IP preference, can be either xPreferenceIPv4 or
 * xPreferenceIPv6
 * @return pdPASS on success and pdFAIL on failure.
 */
    BaseType_t FreeRTOS_SetDNSIPPreference( IPPreference_t eIPPreference )
    {
        BaseType_t xReturn = pdPASS;

        switch( eIPPreference )
        {
            #if ( ipconfigUSE_IPv4 != 0 )
                case xPreferenceIPv4:
                    xDNS_IP_Preference = xPreferenceIPv4;
                    break;
            #endif

            #if ( ipconfigUSE_IPv6 != 0 )
                case xPreferenceIPv6:
                    xDNS_IP_Preference = xPreferenceIPv6;
                    break;
            #endif

            default:
                xReturn = pdFAIL;
                FreeRTOS_printf( ( "Invalid DNS IPPreference_t\n" ) );
                break;
        }

        return xReturn;
    }

/*-----------------------------------------------------------*/

#endif /* ipconfigUSE_DNS != 0 */

/*-----------------------------------------------------------*/

/* Provide access to private members for testing. */
#ifdef FREERTOS_ENABLE_UNIT_TESTS
    #include "freertos_tcp_test_access_dns_define.h"
#endif
