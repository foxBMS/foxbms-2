/*
 * FreeRTOS+TCP V2.3.1
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

#ifndef FREERTOS_ROUTING_H
    #define FREERTOS_ROUTING_H

    #include "FreeRTOS.h"
    #include "FreeRTOS_IP.h"
    #include "FreeRTOS_Sockets.h"

    #if ( ipconfigUSE_DHCP != 0 )
        #include "FreeRTOS_DHCP.h"
    #endif

    #if ( ipconfigUSE_IPv6 != 0 )
        #include "FreeRTOS_DHCPv6.h"
    #endif

    #ifdef __cplusplus
    extern "C" {
    #endif

/* Every NetworkInterface needs a set of access functions: */

/* Initialise the interface. */
    typedef BaseType_t ( * NetworkInterfaceInitialiseFunction_t ) ( struct xNetworkInterface * pxDescriptor );

/* Send out an Ethernet packet. */
    typedef BaseType_t ( * NetworkInterfaceOutputFunction_t ) ( struct xNetworkInterface * pxDescriptor,
                                                                NetworkBufferDescriptor_t * const pxNetworkBuffer,
                                                                BaseType_t xReleaseAfterSend );

/* Return true as long as the LinkStatus on the PHY is present. */
    typedef BaseType_t ( * GetPhyLinkStatusFunction_t ) ( struct xNetworkInterface * pxDescriptor );

/* Functions that manipulate what MAC addresses are received by this interface */
    typedef void ( * NetworkInterfaceMACFilterFunction_t ) ( struct xNetworkInterface * pxInterface,
                                                             const uint8_t * pucMacAddressBytes );

/** @brief These NetworkInterface access functions are collected in a struct: */
    typedef struct xNetworkInterface
    {
        const char * pcName;                               /**< Just for logging, debugging. */
        void * pvArgument;                                 /**< Will be passed to the access functions. */
        NetworkInterfaceInitialiseFunction_t pfInitialise; /**< This function will be called upon initialisation and repeated until it returns pdPASS. */
        NetworkInterfaceOutputFunction_t pfOutput;         /**< This function is supposed to send out a packet. */
        GetPhyLinkStatusFunction_t pfGetPhyLinkStatus;     /**< This function will return pdTRUE as long as the PHY Link Status is high. */

        /*
         * pfAddAllowedMAC and pfRemoveAllowedMAC form the network driver's address filtering API.
         * The network stack uses these functions to alter which MAC addresses will be received.
         * The MAC addresses passed to the functions can be unicast or multicast. It is important
         * to note that the stack may call these functions multiple times for the the same MAC address.
         * For example, if two sockets subscribe to the same multicast group, pfAddAllowedMAC()
         * will be called twice with the same MAC address. The network driver is responsible for
         * keeping track of these calls. The network driver should continue receiving that
         * particular MAC address until pfRemoveAllowedMAC() is called the same number of times.
         *
         * Most EMAC hardware nowadays can filter frames based on both specific MAC address matching
         * and hash matching. Specific address matching is ideal because as the name suggests,
         * only frames with the exact MAC address are received. Usually however, the number of
         * specific MAC addresses is limited ( to 4 in many cases ) and is sometimes not enough for
         * all the MAC addresses that the network stack needs to receive.
         * Hash matching is usually based around a 64-bit hash table. For every incoming frame,
         * the EMAC calculates a hash value (mod 64) of the destination MAC address.
         * The hash value is looked up in the 64-bit hash table and if that bit is set, the frame is
         * received. If the bit is clear, the frame is dropped. With hash matching, multiple
         * MAC addresses are represented by a single bit. It is the responsibility of the network
         * driver to manage both the hash address matching and specific address matching capabilities
         * of the EMAC hardware.
         * A quick and dirty implementation option is to receive all MAC addresses and set both
         * pfAddAllowedMAC and pfRemoveAllowedMAC to NULL. This results in an interface running
         * in promiscuous mode and the entire burden of MAC filtering falls on the network stack.
         * For a more realistic implementation, check out
         * "portable/NetworkInterface/DriverSAM/NetworkInterface.c" It demonstrates the use of both
         * specific and hash address matching as well as keeping count of how many time the
         * individual registers/bits have been used. That implementation's init functions also
         * demonstrates the use of prvAddAllowedMACAddress() function to register all end-point's
         * MAC addresses whether the endpoints used the same or different MAC addresses.
         */
        NetworkInterfaceMACFilterFunction_t pfAddAllowedMAC;
        NetworkInterfaceMACFilterFunction_t pfRemoveAllowedMAC;

        struct
        {
            uint32_t
                bInterfaceUp : 1,             /**< Non-zero as soon as the interface is up. */
                bCallDownEvent : 1;           /**< The down-event must be called. */
        } bits;                               /**< A collection of boolean flags. */

        struct xNetworkEndPoint * pxEndPoint; /**< A list of end-points bound to this interface. */
        struct xNetworkInterface * pxNext;    /**< The next interface in a linked list. */
    } NetworkInterface_t;

/*
 *  // As an example:
 *  NetworkInterface_t xZynqDescriptor = {
 *      .pcName					= "Zynq-GEM",
 *      .pvArgument				= ( void * )1,
 *      .pfInitialise           = xZynqGEMInitialise,
 *      .pfOutput               = xZynqGEMOutput,
 *      .pfGetPhyLinkStatus     = xZynqGEMGetPhyLinkStatus,
 *  };
 */

/** @brief The network settings for IPv4. */
    typedef struct xIPV4Parameters
    {
        uint32_t ulIPAddress;                                                /**< The actual IPv4 address. Will be 0 as long as end-point is still down. */
        uint32_t ulNetMask;                                                  /**< The netmask. */
        uint32_t ulGatewayAddress;                                           /**< The IP-address of the gateway. */
        uint32_t ulDNSServerAddresses[ ipconfigENDPOINT_DNS_ADDRESS_COUNT ]; /**< IP-addresses of DNS servers. */
        uint32_t ulBroadcastAddress;                                         /**< The local broadcast address, e.g. '192.168.1.255'. */
        uint8_t ucDNSIndex;                                                  /**< The index of the next DNS address to be used. */
    } IPV4Parameters_t;

    #if ( ipconfigUSE_IPv6 != 0 )
        typedef struct xIPV6Parameters
        {
            IPv6_Address_t xIPAddress;      /* The actual IPv4 address. Will be 0 as long as end-point is still down. */
            size_t uxPrefixLength;          /* Number of valid bytes in the network prefix. */
            IPv6_Address_t xPrefix;         /* The network prefix, e.g. fe80::/10 */
            IPv6_Address_t xGatewayAddress; /* Gateway to the web. */
            IPv6_Address_t xDNSServerAddresses[ ipconfigENDPOINT_DNS_ADDRESS_COUNT ];
            uint8_t ucDNSIndex;             /**< The index of the next DNS address to be used. */
        } IPV6Parameters_t;
    #endif

    #if ( ipconfigUSE_RA != 0 )
/* Router Advertisement (RA). End-points can obtain their IP-address by asking for a RA. */
        typedef enum xRAState
        {
            eRAStateApply,    /* Send a Router Solicitation. */
            eRAStateWait,     /* Wait for a Router Advertisement. */
            eRAStateIPTest,   /* Take a random IP address, test if another device is using it already. */
            eRAStateIPWait,   /* Wait for a reply, if any */
            eRAStatePreLease, /* The device is ready to go to the 'eRAStateLease' state. */
            eRAStateLease,    /* The device is up, repeat the RA-process when timer expires. */
            eRAStateFailed
        } eRAState_t;

        struct xRA_DATA
        {
            struct
            {
                uint32_t
                    bRouterReplied : 1,
                    bIPAddressInUse : 1;
            }
            bits;
            TickType_t ulPreferredLifeTime;
            UBaseType_t uxRetryCount;
            /* Maintains the RA state machine state. */
            eRAState_t eRAState;
        };

        typedef struct xRA_DATA RAData_t;
    #endif /* ( ipconfigUSE_RA != 0 ) */

/** @brief The description of an end-point. */
    typedef struct xNetworkEndPoint
    {
        #if ( ipconfigUSE_IPv4 != 0 )
            IPV4Parameters_t ipv4_settings; /**< Actual IPv4 settings used by the end-point. */
            IPV4Parameters_t ipv4_defaults; /**< Use values form "ipv4_defaults" in case DHCP has failed. */
        #endif
        #if ( ipconfigUSE_IPv6 != 0 )
            IPV6Parameters_t ipv6_settings; /**< Actual IPv6 settings used by the end-point. */
            IPV6Parameters_t ipv6_defaults; /**< Use values form "ipv6_defaults" in case DHCP has failed. */
        #endif
        MACAddress_t xMACAddress;           /**< The MAC-address assigned to this end-point. */
        struct
        {
            uint32_t
                bIsDefault : 1, /**< This bit will be removed. */
            #if ( ipconfigUSE_DHCP != 0 ) || ( ipconfigUSE_DHCPv6 != 0 )
                bWantDHCP : 1,  /**< This end-point wants to use DHCPv4 to obtain an IP-address. */
            #endif /* ipconfigUSE_DHCP */
            #if ( ipconfigUSE_RA != 0 )
                bWantRA : 1,         /**< This end-point wants to use RA/SLAAC to obtain an IP-address. */
            #endif /* ipconfigUSE_RA */
            bIPv6 : 1,               /**< This end-point has an IP-address of type IPv6. */
            #if ( ipconfigUSE_NETWORK_EVENT_HOOK != 0 )
                bCallDownHook : 1,   /**< The network down hook-must be called for this end-point. */
            #endif /* ipconfigUSE_NETWORK_EVENT_HOOK */
            bEndPointUp : 1;         /**< The end-point is up. */
        } bits;                      /**< A collection of boolean properties. */
        uint8_t usDNSType;           /**< A LLMNR/mDNS lookup is being done for an IPv6 address.
                                      * This field is only valid while xApplicationDNSQueryHook() is called. */
        #if ( ipconfigUSE_DHCP != 0 ) || ( ipconfigUSE_RA != 0 )
            IPTimer_t xDHCP_RATimer; /**<  The timer used to call the DHCP/DHCPv6/RA state machine. */
        #endif /* ( ipconfigUSE_DHCP != 0 ) || ( ipconfigUSE_RA != 0 ) */
        #if ( ipconfigUSE_DHCP != 0 ) || ( ipconfigUSE_DHCPv6 != 0 )
            DHCPData_t xDHCPData; /**< A description of the DHCP client state machine. */
        #endif /* ( ipconfigUSE_DHCP != 0 ) || ( ipconfigUSE_DHCPv6 != 0 ) */
        #if ( ipconfigUSE_IPv6 != 0 )
            DHCPMessage_IPv6_t * pxDHCPMessage; /**< A description of the DHCPv6 client state machine. */
        #endif
        #if ( ipconfigUSE_RA != 0 )
            RAData_t xRAData;                    /**< A description of the Router Advertisement ( RA ) client state machine. */
        #endif /* ( ipconfigUSE_RA != 0 ) */
        NetworkInterface_t * pxNetworkInterface; /**< The network interface that owns this end-point. */
        struct xNetworkEndPoint * pxNext;        /**< The next end-point in the chain. */
    } NetworkEndPoint_t;

    #define END_POINT_USES_DHCP( pxEndPoint )    ( ( pxEndPoint )->bits.bWantDHCP != pdFALSE_UNSIGNED )
    #define END_POINT_USES_RA( pxEndPoint )      ( ( ( pxEndPoint )->bits.bIPv6 != pdFALSE_UNSIGNED ) && ( ( pxEndPoint )->bits.bWantRA != pdFALSE_UNSIGNED ) )

    #define ENDPOINT_IS_IPv4( pxEndPoint )       ( ( ( pxEndPoint ) != NULL ) && ( ( pxEndPoint )->bits.bIPv6 == 0U ) )
    #define ENDPOINT_IS_IPv6( pxEndPoint )       ( ( ( pxEndPoint ) != NULL ) && ( ( pxEndPoint )->bits.bIPv6 != 0U ) )


/*
 * Add a new physical Network Interface.  The object pointed to by 'pxInterface'
 * must continue to exist.
 * Only the Network Interface function xx_FillInterfaceDescriptor() shall call this function.
 */
    NetworkInterface_t * FreeRTOS_AddNetworkInterface( NetworkInterface_t * pxInterface );

/*
 * Get the first Network Interface.
 */
    NetworkInterface_t * FreeRTOS_FirstNetworkInterface( void );

/*
 * Get the next Network Interface.
 */
    NetworkInterface_t * FreeRTOS_NextNetworkInterface( const NetworkInterface_t * pxInterface );

/*
 * Get the first end-point belonging to a given interface.  When pxInterface is
 * NULL, the very first end-point will be returned.
 */
    NetworkEndPoint_t * FreeRTOS_FirstEndPoint( const NetworkInterface_t * pxInterface );

/*
 * Get the next end-point.  When pxInterface is null, all end-points can be
 * iterated.
 */
    NetworkEndPoint_t * FreeRTOS_NextEndPoint( const NetworkInterface_t * pxInterface,
                                               NetworkEndPoint_t * pxEndPoint );

/*
 * Find the end-point with given IP-address.
 */
    NetworkEndPoint_t * FreeRTOS_FindEndPointOnIP_IPv4( uint32_t ulIPAddress,
                                                        uint32_t ulWhere );

    #if ( ipconfigUSE_IPv6 != 0 )
        /* Find the end-point with given IP-address. */
        NetworkEndPoint_t * FreeRTOS_FindEndPointOnIP_IPv6( const IPv6_Address_t * pxIPAddress );
    #endif /* ipconfigUSE_IPv6 */

/*
 * Find the end-point with given MAC-address.
 * The search can be limited by supplying a particular interface.
 */
    NetworkEndPoint_t * FreeRTOS_FindEndPointOnMAC( const MACAddress_t * pxMACAddress,
                                                    const NetworkInterface_t * pxInterface );

/*
 * Find the best fitting end-point to reach a given IP-address.
 * Find an end-point whose IP-address is in the same network as the IP-address provided.
 * 'ulWhere' is temporary and or debugging only.
 */
    NetworkEndPoint_t * FreeRTOS_FindEndPointOnNetMask( uint32_t ulIPAddress,
                                                        uint32_t ulWhere );

/*
 * Find the best fitting IPv4 end-point to reach a given IP-address on a given interface
 * 'ulWhere' is temporary and or debugging only.
 */
    NetworkEndPoint_t * FreeRTOS_InterfaceEndPointOnNetMask( const NetworkInterface_t * pxInterface,
                                                             uint32_t ulIPAddress,
                                                             uint32_t ulWhere );

/*
 * Finds an endpoint on the given interface which is in the same subnet as the
 * given IP address. If NULL is passed for pxInterface, it looks through all the
 * interfaces to find an endpoint in the same subnet as the given IP address.
 */
    NetworkEndPoint_t * FreeRTOS_InterfaceEPInSameSubnet_IPv6( const NetworkInterface_t * pxInterface,
                                                               const IPv6_Address_t * pxIPAddress );

    #if ( ipconfigUSE_IPv6 != 0 )
        NetworkEndPoint_t * FreeRTOS_FindEndPointOnNetMask_IPv6( const IPv6_Address_t * pxIPv6Address );
    #endif /* ipconfigUSE_IPv6 */

    #if ( ipconfigUSE_IPv6 != 0 )

/* Get the first end-point belonging to a given interface.
 * When pxInterface is NULL, the very first end-point will be returned. */
        NetworkEndPoint_t * FreeRTOS_FirstEndPoint_IPv6( const NetworkInterface_t * pxInterface );
    #endif /* ipconfigUSE_IPv6 */

/* A ethernet packet has come in on a certain network interface.
 * Find the best matching end-point. */
    NetworkEndPoint_t * FreeRTOS_MatchingEndpoint( const NetworkInterface_t * pxNetworkInterface,
                                                   const uint8_t * pucEthernetBuffer );

/* Find an end-point that has a defined gateway.
 * xIPType should equal ipTYPE_IPv4 or ipTYPE_IPv6. */
    NetworkEndPoint_t * FreeRTOS_FindGateWay( BaseType_t xIPType );

/* Fill-in the end-point structure. */
    void FreeRTOS_FillEndPoint( NetworkInterface_t * pxNetworkInterface,
                                NetworkEndPoint_t * pxEndPoint,
                                const uint8_t ucIPAddress[ ipIP_ADDRESS_LENGTH_BYTES ],
                                const uint8_t ucNetMask[ ipIP_ADDRESS_LENGTH_BYTES ],
                                const uint8_t ucGatewayAddress[ ipIP_ADDRESS_LENGTH_BYTES ],
                                const uint8_t ucDNSServerAddress[ ipIP_ADDRESS_LENGTH_BYTES ],
                                const uint8_t ucMACAddress[ ipMAC_ADDRESS_LENGTH_BYTES ] );

    #if ( ipconfigUSE_IPv6 != 0 )
        /* Fill-in the end-point structure. */
        void FreeRTOS_FillEndPoint_IPv6( NetworkInterface_t * pxNetworkInterface,
                                         NetworkEndPoint_t * pxEndPoint,
                                         const IPv6_Address_t * pxIPAddress,
                                         const IPv6_Address_t * pxNetPrefix,
                                         size_t uxPrefixLength,
                                         const IPv6_Address_t * pxGatewayAddress,
                                         const IPv6_Address_t * pxDNSServerAddress, /* Not used yet. */
                                         const uint8_t ucMACAddress[ ipMAC_ADDRESS_LENGTH_BYTES ] );
    #endif

    #if ( ipconfigHAS_ROUTING_STATISTICS == 1 )
/** @brief Some simple network statistics. */
        typedef struct xRoutingStats
        {
            UBaseType_t ulOnIp;             /**< The number of times 'FreeRTOS_FindEndPointOnIP_IPv4()' has been called. */
            UBaseType_t ulOnMAC;            /**< The number of times 'FreeRTOS_FindEndPointOnMAC()' has been called. */
            UBaseType_t ulOnNetMask;        /**< The number of times 'FreeRTOS_InterfaceEndPointOnNetMask()' has been called. */
            UBaseType_t ulMatching;         /**< The number of times 'FreeRTOS_MatchingEndpoint()' has been called. */
            UBaseType_t ulLocations[ 14 ];  /**< The number of times 'FreeRTOS_InterfaceEndPointOnNetMask()' has been called from a particular location. */
            UBaseType_t ulLocationsIP[ 8 ]; /**< The number of times 'FreeRTOS_FindEndPointOnIP_IPv4()' has been called from a particular location. */
        } RoutingStats_t;

        extern RoutingStats_t xRoutingStatistics;
    #endif /* ( ipconfigHAS_ROUTING_STATISTICS == 1 ) */

    NetworkEndPoint_t * pxGetSocketEndpoint( ConstSocket_t xSocket );
    void vSetSocketEndpoint( Socket_t xSocket,
                             NetworkEndPoint_t * pxEndPoint );

    #if ( ( ipconfigHAS_PRINTF != 0 ) || ( ipconfigHAS_DEBUG_PRINTF != 0 ) )
        const char * pcEndpointName( const NetworkEndPoint_t * pxEndPoint,
                                     char * pcBuffer,
                                     size_t uxSize );
    #endif /* ( ( ipconfigHAS_PRINTF != 0 ) || ( ipconfigHAS_DEBUG_PRINTF != 0 ) ) */

    typedef enum
    {
        eIPv6_Global,    /* 001           */
        eIPv6_LinkLocal, /* 1111 1110 10  */
        eIPv6_SiteLocal, /* 1111 1110 11  */
        eIPv6_Multicast, /* 1111 1111     */
        eIPv6_Loopback,  /* 1111 (::1)    */
        eIPv6_Unknown    /* Not implemented. */
    }
    IPv6_Type_t;

    #if ( ipconfigUSE_IPv6 != 0 )

/**
 * @brief Check the type of an IPv16 address.
 *
 * @return A value from enum IPv6_Type_t.
 */
        IPv6_Type_t xIPv6_GetIPType( const IPv6_Address_t * pxAddress );
    #endif

    #ifdef __cplusplus
}     /* extern "C" */
    #endif

#endif /* FREERTOS_ROUTING_H */
