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

#ifndef FREERTOS_DHCP_H
#define FREERTOS_DHCP_H

#include "FreeRTOS.h"

/* Application level configuration options. */
#include "FreeRTOSIPConfig.h"

#include "FreeRTOS_Sockets.h"

/* *INDENT-OFF* */
#ifdef __cplusplus
    extern "C" {
#endif

#if ( ipconfigUSE_DHCP != 0 ) && ( ipconfigNETWORK_MTU < 586U )

/* DHCP must be able to receive an options field of 312 bytes, the fixed
 * part of the DHCP packet is 240 bytes, and the IP/UDP headers take 28 bytes. */
    #error ipconfigNETWORK_MTU needs to be at least 586 to use DHCP
#endif

/* Parameter widths in the DHCP packet. */
#define dhcpCLIENT_HARDWARE_ADDRESS_LENGTH    16      /**< Client hardware address length.*/
#define dhcpSERVER_HOST_NAME_LENGTH           64      /**< Server host name length. */
#define dhcpBOOT_FILE_NAME_LENGTH             128     /**< Boot file name length. */

/* Timer parameters */
#ifndef dhcpINITIAL_TIMER_PERIOD
    /** @brief The interval at which the DHCP state handler is called. */
    #define dhcpINITIAL_TIMER_PERIOD    ( pdMS_TO_TICKS( 250U ) )
#endif

#ifndef dhcpINITIAL_DHCP_TX_PERIOD

/** @brief The initial amount of time to wait for a DHCP reply.  When repeating an
 * unanswered request, this time-out shall be multiplied by 2. */
    #define dhcpINITIAL_DHCP_TX_PERIOD    ( pdMS_TO_TICKS( 5000U ) )
#endif

#define dhcpIPv6_CLIENT_DUID_LENGTH                ( 14U )

/* Codes of interest found in the DHCP options field. */
#define dhcpIPv4_ZERO_PAD_OPTION_CODE              ( 0U )      /**< Used to pad other options to make them aligned. See RFC 2132. */
#define dhcpIPv4_SUBNET_MASK_OPTION_CODE           ( 1U )      /**< Subnet mask. See RFC 2132. */
#define dhcpIPv4_GATEWAY_OPTION_CODE               ( 3U )      /**< Available routers. See RFC 2132. */
#define dhcpIPv4_DNS_SERVER_OPTIONS_CODE           ( 6U )      /**< Domain name server. See RFC 2132. */
#define dhcpIPv4_DNS_HOSTNAME_OPTIONS_CODE         ( 12U )     /**< Host name. See RFC 2132. */
#define dhcpIPv4_REQUEST_IP_ADDRESS_OPTION_CODE    ( 50U )     /**< Requested IP-address. See RFC 2132. */
#define dhcpIPv4_LEASE_TIME_OPTION_CODE            ( 51U )     /**< IP-address lease time. See RFC 2132. */
#define dhcpIPv4_MESSAGE_TYPE_OPTION_CODE          ( 53U )     /**< DHCP message type. See RFC 2132. */
#define dhcpIPv4_SERVER_IP_ADDRESS_OPTION_CODE     ( 54U )     /**< Server Identifier. See RFC 2132. */
#define dhcpIPv4_PARAMETER_REQUEST_OPTION_CODE     ( 55U )     /**< Parameter Request list. See RFC 2132. */
#define dhcpIPv4_CLIENT_IDENTIFIER_OPTION_CODE     ( 61U )     /**<  Client Identifier. See RFC 2132. */

/* The four DHCP message types of interest. */
#define dhcpMESSAGE_TYPE_DISCOVER                  ( 1 )     /**< DHCP discover message. */
#define dhcpMESSAGE_TYPE_OFFER                     ( 2 )     /**< DHCP offer message. */
#define dhcpMESSAGE_TYPE_REQUEST                   ( 3 )     /**< DHCP request message. */
#define dhcpMESSAGE_TYPE_ACK                       ( 5 )     /**< DHCP acknowledgement. */
#define dhcpMESSAGE_TYPE_NACK                      ( 6 )     /**< DHCP NACK. (Negative acknowledgement) */

/* Offsets into the transmitted DHCP options fields at which various parameters
 * are located. */
#define dhcpCLIENT_IDENTIFIER_OFFSET               ( 6U )      /**< Offset for the client ID option. */
#define dhcpREQUESTED_IP_ADDRESS_OFFSET            ( 14U )     /**< Offset for the requested IP-address option. */
#define dhcpDHCP_SERVER_IP_ADDRESS_OFFSET          ( 20U )     /**< Offset for the server IP-address option. */
#define dhcpOPTION_50_OFFSET                       ( 12U )     /**< Offset of option-50. */
#define dhcpOPTION_50_SIZE                         ( 6U )      /**< Number of bytes included in option-50. */


/* Values used in the DHCP packets. */
#define dhcpREQUEST_OPCODE              ( 1U )                /**< DHCP request opcode. */
#define dhcpREPLY_OPCODE                ( 2U )                /**< DHCP reply opcode. */
#define dhcpADDRESS_TYPE_ETHERNET       ( 1U )                /**< Address type: ethernet opcode. */
#define dhcpETHERNET_ADDRESS_LENGTH     ( 6U )                /**< Ethernet address length opcode. */

/** @brief If a lease time is not received, use the default of two days (48 hours in ticks).
 * Can not use pdMS_TO_TICKS() as integer overflow can occur. */
#define dhcpDEFAULT_LEASE_TIME          ( ( 48UL * 60UL * 60UL ) * configTICK_RATE_HZ )

/** @brief Don't allow the lease time to be too short. */
#define dhcpMINIMUM_LEASE_TIME          ( pdMS_TO_TICKS( 60000UL ) )            /* 60 seconds in ticks. */

/** @brief Marks the end of the variable length options field in the DHCP packet. */
#define dhcpOPTION_END_BYTE             0xffu

/** @brief Offset into a DHCP message at which the first byte of the options is
 * located. */
#define dhcpFIRST_OPTION_BYTE_OFFSET    ( 0xf0U )

/* Standard DHCP port numbers and magic cookie value.
 * DHCPv4 uses UDP port number 68 for clients and port number 67 for servers.
 */
#if ( ipconfigBYTE_ORDER == pdFREERTOS_LITTLE_ENDIAN )
    #define dhcpCLIENT_PORT_IPv4    0x4400U          /**< Little endian representation of port 68. */
    #define dhcpSERVER_PORT_IPv4    0x4300U          /**< Little endian representation of port 67. */
    #define dhcpCOOKIE              0x63538263UL     /**< Little endian representation of magic cookie. */
    #define dhcpBROADCAST           0x0080U          /**< Little endian representation of broadcast flag. */
#else
    #define dhcpCLIENT_PORT_IPv4    0x0044U          /**< Big endian representation of port 68. */
    #define dhcpSERVER_PORT_IPv4    0x0043U          /**< Big endian representation of port 67. */
    #define dhcpCOOKIE              0x63825363UL     /**< Big endian representation of magic cookie. */
    #define dhcpBROADCAST           0x8000U          /**< Big endian representation of broadcast flag. */
#endif /* ( ipconfigBYTE_ORDER == pdFREERTOS_LITTLE_ENDIAN ) */

#include "pack_struct_start.h"
struct xDHCPMessage_IPv4
{
    uint8_t ucOpcode;                                                      /**< Operation Code: Specifies the general type of message. */
    uint8_t ucAddressType;                                                 /**< Hardware type used on the local network. */
    uint8_t ucAddressLength;                                               /**< Hardware Address Length: Specifies how long hardware
                                                                            * addresses are in this message. */
    uint8_t ucHops;                                                        /**< Hops. */
    uint32_t ulTransactionID;                                              /**< A 32-bit identification field generated by the client,
                                                                            * to allow it to match up the request with replies received
                                                                            * from DHCP servers. */
    uint16_t usElapsedTime;                                                /**< Number of seconds elapsed since a client began an attempt to acquire or renew a lease. */
    uint16_t usFlags;                                                      /**< Just one bit used to indicate broadcast. */
    uint32_t ulClientIPAddress_ciaddr;                                     /**< Client's IP address if it has one or 0 is put in this field. */
    uint32_t ulYourIPAddress_yiaddr;                                       /**< The IP address that the server is assigning to the client. */
    uint32_t ulServerIPAddress_siaddr;                                     /**< The DHCP server address that the client should use. */
    uint32_t ulRelayAgentIPAddress_giaddr;                                 /**< Gateway IP address in case the server client are on different subnets. */
    uint8_t ucClientHardwareAddress[ dhcpCLIENT_HARDWARE_ADDRESS_LENGTH ]; /**< The client hardware address. */
    uint8_t ucServerHostName[ dhcpSERVER_HOST_NAME_LENGTH ];               /**< Server's hostname. */
    uint8_t ucBootFileName[ dhcpBOOT_FILE_NAME_LENGTH ];                   /**< Boot file full directory path. */
    uint32_t ulDHCPCookie;                                                 /**< Magic cookie option. */
    /* Option bytes from here on. */
}
#include "pack_struct_end.h"
typedef struct xDHCPMessage_IPv4 DHCPMessage_IPv4_t;


#if ( ipconfigUSE_DHCP_HOOK != 0 )
    /* Used in the DHCP callback if ipconfigUSE_DHCP_HOOK is set to 1. */
    typedef enum eDHCP_PHASE
    {
        eDHCPPhasePreDiscover, /**< Driver is about to send a DHCP discovery. */
        eDHCPPhasePreRequest   /**< Driver is about to request DHCP an IP address. */
    } eDHCPCallbackPhase_t;

/** @brief Used in the DHCP callback if ipconfigUSE_DHCP_HOOK is set to 1. */
    typedef enum eDHCP_ANSWERS
    {
        eDHCPContinue,      /**< Continue the DHCP process */
        eDHCPUseDefaults,   /**< Stop DHCP and use the static defaults. */
        eDHCPStopNoChanges  /**< Stop DHCP and continue with current settings. */
    } eDHCPCallbackAnswer_t;
#endif /* #if( ipconfigUSE_DHCP_HOOK != 0 ) */

/** @brief DHCP state machine states. */
typedef enum
{
    eInitialWait = 0,          /**< Initial state: open a socket and wait a short time. */
    eWaitingSendFirstDiscover, /**< Send a discover the first time it is called, and reset all timers. */
    eWaitingOffer,             /**< Either resend the discover, or, if the offer is forthcoming, send a request. */
    eWaitingAcknowledge,       /**< Either resend the request. */
    eSendDHCPRequest,          /**< Sendto failed earlier, resend the request to lease the IP-address offered. */
    #if ( ipconfigDHCP_FALL_BACK_AUTO_IP != 0 )
        eGetLinkLayerAddress,  /**< When DHCP didn't respond, try to obtain a LinkLayer address 168.254.x.x. */
    #endif
    eLeasedAddress,            /**< Resend the request at the appropriate time to renew the lease. */
    eNotUsingLeasedAddress     /**< DHCP failed, and a default IP address is being used. */
} eDHCPState_t;

/** @brief Hold information in between steps in the DHCP state machine. */
struct xDHCP_DATA
{
    uint32_t ulTransactionId;      /**< The ID used in all transactions. */
    uint32_t ulOfferedIPAddress;   /**< The IP-address offered by the DHCP server. */
    uint32_t ulPreferredIPAddress; /**< A preferred IP address */
    uint32_t ulDHCPServerAddress;  /**< The IP-address of the DHCP server. */
    uint32_t ulLeaseTime;          /**< The maximum time that the IP-address can be leased. */
    /* Hold information on the current timer state. */
    TickType_t xDHCPTxTime;        /**< The time at which a request was sent, initialised with xTaskGetTickCount(). */
    TickType_t xDHCPTxPeriod;      /**< The maximum time to wait for a response. */
    /* Try both without and with the broadcast flag */
    BaseType_t xUseBroadcast;      /**< pdTRUE if the broadcast bit 'dhcpBROADCAST' must be set. */
    /* Maintains the DHCP state machine state. */
    eDHCPState_t eDHCPState;       /**< The current state of the DHCP state machine. */
    eDHCPState_t eExpectedState;   /**< If the state is not equal the the expected state, no cycle needs to be done. */
    Socket_t xDHCPSocket;
    /**< Record latest client ID for DHCPv6. */
    uint8_t ucClientDUID[ dhcpIPv6_CLIENT_DUID_LENGTH ];
};

typedef struct xDHCP_DATA DHCPData_t;

/** brief: a set of parameters that are passed to helper functions. */
typedef struct xProcessSet
{
    uint8_t ucOptionCode;       /**< The code currently being handled. */
    size_t uxIndex;             /**< The index within 'pucByte'. */
    size_t uxPayloadDataLength; /**< The number of bytes in the UDP payload. */
    size_t uxLength;            /**< The size of the current option. */
    uint32_t ulParameter;       /**< The uint32 value of the answer, if available. */
    uint32_t ulProcessed;       /**< The number of essential options that were parsed. */
    const uint8_t * pucByte;    /**< A pointer to the data to be analysed. */
} ProcessSet_t;


/*
 * NOT A PUBLIC API FUNCTION.
 * It will be called when the DHCP timer expires, or when
 * data has been received on the DHCP socket.
 */
void vDHCPProcess( BaseType_t xReset,
                    struct xNetworkEndPoint * pxEndPoint );

/*
 * NOT A PUBLIC API FUNCTION.
 * It will be called when the network interface, that the endpoint is associated with, goes down.
 */
void vDHCPStop( struct xNetworkEndPoint * pxEndPoint );

/* Internal call: returns true if socket is the current DHCP socket */
BaseType_t xIsDHCPSocket( const ConstSocket_t xSocket );


#if ( ipconfigUSE_DHCP_HOOK != 0 )

/* Prototype of the hook (or callback) function that must be provided by the
 * application if ipconfigUSE_DHCP_HOOK is set to 1.  See the following URL for
 * usage information:
 * http://www.FreeRTOS.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html#ipconfigUSE_DHCP_HOOK
 */
    #if ( ipconfigIPv4_BACKWARD_COMPATIBLE == 1 )
        eDHCPCallbackAnswer_t xApplicationDHCPHook( eDHCPCallbackPhase_t eDHCPPhase,
                                                    uint32_t ulIPAddress );
    #else /* ( ipconfigIPv4_BACKWARD_COMPATIBLE == 1 ) */
        eDHCPCallbackAnswer_t xApplicationDHCPHook_Multi( eDHCPCallbackPhase_t eDHCPPhase,
                                                          struct xNetworkEndPoint * pxEndPoint,
                                                          IP_Address_t * pxIPAddress );
    #endif /* ( ipconfigIPv4_BACKWARD_COMPATIBLE == 1 ) */
#endif /* ( ipconfigUSE_DHCP_HOOK != 0 ) */

#if ( ipconfigDHCP_FALL_BACK_AUTO_IP != 0 )
    struct xNetworkEndPoint;

/**
 * @brief When DHCP has failed, the code can assign a Link-Layer
 *        address, and check if another device already uses the IP-address.
 *
 * param[in] pxEndPoint: The end-point that wants to obtain a link-layer address.
 */
    void prvPrepareLinkLayerIPLookUp( struct xNetworkEndPoint * pxEndPoint );
#endif

/* *INDENT-OFF* */
#ifdef __cplusplus
    } /* extern "C" */
#endif
/* *INDENT-ON* */

#endif /* FREERTOS_DHCP_H */
