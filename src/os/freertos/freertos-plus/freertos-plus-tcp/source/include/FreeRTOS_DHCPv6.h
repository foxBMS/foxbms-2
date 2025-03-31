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

#ifndef FREERTOS_DHCPV6_H
    #define FREERTOS_DHCPV6_H

/* Application level configuration options. */
    #include "FreeRTOS_DHCP.h"
    #include "FreeRTOSIPConfig.h"

    #ifdef __cplusplus
    extern "C" {
    #endif

/* IPv6 option numbers. */
/** @brief IPv6 DHCP option number - Solicit */
    #define DHCPv6_message_Type_Solicit      1U
/** @brief IPv6 DHCP option number - Advertise */
    #define DHCPv6_message_Type_Advertise    2U
/** @brief IPv6 DHCP option number - Request */
    #define DHCPv6_message_Type_Request      3U
/** @brief IPv6 DHCP option number - Confirm */
    #define DHCPv6_message_Type_Confirm      4U
/** @brief IPv6 DHCP option number - Renew  */
    #define DHCPv6_message_Type_Renew        5U
/** @brief IPv6 DHCP option number - Reply */
    #define DHCPv6_message_Type_Reply        7U
/** @brief IPv6 DHCP option number - Release */
    #define DHCPv6_message_Type_Release      8U
/** @brief IPv6 DHCP option number - Decline */
    #define DHCPv6_message_Type_Decline      9U

/* Note: IA stands for "Identity_Association". */
/** @brief IPv6 DHCP option - Client Identifier */
    #define DHCPv6_Option_Client_Identifier            1U
/** @brief IPv6 DHCP option - Server Identifier */
    #define DHCPv6_Option_Server_Identifier            2U
/** @brief IPv6 DHCP option - Non Temporary Address */
    #define DHCPv6_Option_NonTemporaryAddress          3U
/** @brief IPv6 DHCP option - Temporary Address */
    #define DHCPv6_Option_TemporaryAddress             4U
/** @brief IPv6 DHCP option - Identity_Association Address */
    #define DHCPv6_Option_IA_Address                   5U
/** @brief IPv6 DHCP option - Option */
    #define DHCPv6_Option_Option_List                  6U
/** @brief IPv6 DHCP option - Preference */
    #define DHCPv6_Option_Preference                   7U
/** @brief IPv6 DHCP option - Elapsed time */
    #define DHCPv6_Option_Elapsed_Time                 8U
/** @brief IPv6 DHCP option - Status code */
    #define DHCPv6_Option_Status_Code                  13U
/** @brief IPv6 DHCP option - Recursive name server */
    #define DHCPv6_Option_DNS_recursive_name_server    23U
/** @brief IPv6 DHCP option - Search list */
    #define DHCPv6_Option_Domain_Search_List           24U
/** @brief IPv6 DHCP option - IA for prefix delegation */
    #define DHCPv6_Option_IA_for_Prefix_Delegation     25U
/** @brief IPv6 DHCP option - IA Prefix */
    #define DHCPv6_Option_IA_Prefix                    26U

/** @brief DHCPv6 option request, used in combination with 'DHCPv6_Option_Option_List' */
    #define DHCP6_OPTION_REQUEST_DNS                   0x0017
/** @brief DHCPv6 option request domain search list, used in combination with 'DHCPv6_Option_Option_List' */
    #define DHCP6_OPTION_REQUEST_DOMAIN_SEARCH_LIST    0x0018

    #define DHCPv6_MAX_CLIENT_SERVER_ID_LENGTH         128

/** @brief The function time() counts since 1-1-1970.  The DHCPv6 time-stamp however
 * uses a time stamp that had zero on 1-1-2000. */
    #define SECS_FROM_1970_TILL_2000                   946684800U

/** @brief If a lease time is not received, use the default of two days.  48 hours in ticks.
 * Do not use the macro pdMS_TO_TICKS() here as integer overflow can occur. */
    #define dhcpv6DEFAULT_LEASE_TIME                   ( ( 48U * 60U * 60U ) * configTICK_RATE_HZ )

/** @brief Don't allow the lease time to be too short. */
    #define dhcpv6MINIMUM_LEASE_TIME                   ( pdMS_TO_TICKS( 60000U ) ) /* 60 seconds in ticks. */

/** @brief Default v6 DHCP client port. */
    #define ipDHCPv6_CLIENT_PORT                       546U
/** @brief Default v6 DHCP server port. */
    #define ipDHCPv6_SERVER_PORT                       547U

/** @brief The ID of a client or a server. */
    typedef struct xClientServerID
    {
        uint16_t usDUIDType;                                 /**< A DHCP Unique Identifier ( DUID ). */
        uint16_t usHardwareType;                             /**< The hardware type: 1 = Ethernet. */
        uint8_t pucID[ DHCPv6_MAX_CLIENT_SERVER_ID_LENGTH ]; /**< Universally Unique IDentifier (UUID) format. */
        size_t uxLength;                                     /**< The number of valid bytes within 'pucID'. */
    } ClientServerID_t;

/** @brief DHCPMessage_IPv6_t holds all data of a DHCP client. */
    typedef struct xDHCPMessage_IPv6
    {
        uint8_t uxMessageType;                                          /**< The type of the last message received: Advertise / Confirm / Reply / Decline */
        uint8_t ucTransactionID[ 3 ];                                   /**< ID of a transaction, shall be renewed when the transaction is ready ( and a reply has been received ). */
        uint32_t ulTransactionID;                                       /**< The same as above but now as a long integer. */
        IP_Address_t xDNSServers[ ipconfigENDPOINT_DNS_ADDRESS_COUNT ]; /**< The IP-address of the DNS server. */
        size_t uxDNSCount;                                              /**< The number of the DNS server stored in xDNSServers. */
        uint32_t ulPreferredLifeTime;                                   /**< The preferred life time. */
        uint32_t ulValidLifeTime;                                       /**< The valid life time. */
        uint32_t ulTimeStamp;                                           /**< DUID Time: seconds since 1-1-2000. */
        uint8_t ucprefixLength;                                         /**< The length of the prefix offered. */
        uint8_t ucHasUID;                                               /**< When pdFALSE: a transaction ID must be created. */
        IP_Address_t xPrefixAddress;                                    /**< The prefix offered. */
        IP_Address_t xIPAddress;                                        /**< The IP-address offered. */
        ClientServerID_t xClientID;                                     /**< The UUID of the client. */
        ClientServerID_t xServerID;                                     /**< The UUID of the server. */
    } DHCPMessage_IPv6_t;

/** @brief A struct describing an option. */
    typedef struct xDHCPOptionSet
    {
        size_t uxOptionLength; /**<  The length of the option being handled. */
        size_t uxStart;        /**<  The position in xMessage where the option starts. */
    } DHCPOptionSet_t;

    struct xNetworkEndPoint;

/* Returns the current state of a DHCP process. */
    eDHCPState_t eGetDHCPv6State( struct xNetworkEndPoint * pxEndPoint );

/*
 * NOT A PUBLIC API FUNCTION.
 * It will be called when the DHCP timer expires, or when
 * data has been received on the DHCP socket.
 */
    void vDHCPv6Process( BaseType_t xReset,
                         struct xNetworkEndPoint * pxEndPoint );

/*
 * NOT A PUBLIC API FUNCTION.
 * It will be called when the network interface, that the endpoint is associated with, goes down.
 */
    void vDHCPv6Stop( struct xNetworkEndPoint * pxEndPoint );

    #ifdef __cplusplus
}     /* extern "C" */
    #endif

/* The application should supply the following time-function.
 * It must return the number of seconds that have passed since
 * 1/1/1970. */
    extern uint32_t ulApplicationTimeHook( void );

#endif /* FREERTOS_DHCPV6_H */
