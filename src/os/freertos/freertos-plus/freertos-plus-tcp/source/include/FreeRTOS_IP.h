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

#ifndef FREERTOS_IP_H
#define FREERTOS_IP_H

#include "FreeRTOS.h"
#include "task.h"

/* Application level configuration options. */
#include "FreeRTOSIPConfig.h"
#include "FreeRTOSIPConfigDefaults.h"
#include "FreeRTOS_IP_Common.h"

/* *INDENT-OFF* */
#ifdef __cplusplus
    extern "C" {
#endif
/* *INDENT-ON* */

/* Constants defining the current version of the FreeRTOS+TCP
 * network stack. */
#define ipFR_TCP_VERSION_NUMBER    "V4.3.999"
#define ipFR_TCP_VERSION_MAJOR     4
#define ipFR_TCP_VERSION_MINOR     3
/* Development builds are always version 999. */
#define ipFR_TCP_VERSION_BUILD     999

/* Some constants defining the sizes of several parts of a packet.
 * These defines come before including the configuration header files. */

/* The size of the Ethernet header is 14, meaning that 802.1Q VLAN tags
 * are not ( yet ) supported. */
#define ipSIZE_OF_ETH_HEADER     14U
#define ipSIZE_OF_IGMP_HEADER    8U
#define ipSIZE_OF_UDP_HEADER     8U
#define ipSIZE_OF_TCP_HEADER     20U

/* The maximum of int32 value. */
#define ipINT32_MAX_VALUE        ( ( int32_t ) 0x7FFFFFFFU )

/* The minimum of int32 value. */
#define ipINT32_MIN_VALUE        ( ( int32_t ) 0x80000000U )

/*
 * Generate a randomized TCP Initial Sequence Number per RFC.
 * This function must be provided by the application builder.
 */
/* This function is defined generally by the application. */
extern uint32_t ulApplicationGetNextSequenceNumber( uint32_t ulSourceAddress,
                                                    uint16_t usSourcePort,
                                                    uint32_t ulDestinationAddress,
                                                    uint16_t usDestinationPort );

/* The number of octets in the MAC and IP addresses respectively. */
#define ipMAC_ADDRESS_LENGTH_BYTES                 ( 6U )
#define ipIP_ADDRESS_LENGTH_BYTES                  ( 4U )

/* IP protocol definitions. */
#define ipPROTOCOL_ICMP                            ( 1U )
#define ipPROTOCOL_IGMP                            ( 2U )
#define ipPROTOCOL_TCP                             ( 6U )
#define ipPROTOCOL_UDP                             ( 17U )

/* The character used to fill ICMP echo requests, and therefore also the
 * character expected to fill ICMP echo replies. */
#define ipECHO_DATA_FILL_BYTE                      'x'

/* Dimensions the buffers that are filled by received Ethernet frames. */
#define ipSIZE_OF_ETH_CRC_BYTES                    ( 4UL )
#define ipSIZE_OF_ETH_OPTIONAL_802_1Q_TAG_BYTES    ( 4UL )
#define ipTOTAL_ETHERNET_FRAME_SIZE                ( ( ( uint32_t ) ipconfigNETWORK_MTU ) + ( ( uint32_t ) ipSIZE_OF_ETH_HEADER ) + ipSIZE_OF_ETH_CRC_BYTES + ipSIZE_OF_ETH_OPTIONAL_802_1Q_TAG_BYTES )


/* Space left at the beginning of a network buffer storage area to store a
 * pointer back to the network buffer.  Should be a multiple of 8 to ensure 8 byte
 * alignment is maintained on architectures that require it.
 *
 * In order to get a 32-bit or 64-bit alignment of network packets, an offset of 2 bytes
 * is ideal as defined by ipconfigPACKET_FILLER_SIZE. So the malloc'd
 * buffer will have the following contents:
 *
 * +---------+-----------+---------+
 * | Offset  | Alignment | Length  |
 * | 32 | 64 | 32  | 64  | 32 | 64 |
 * |----|----|-----|-----|----|----|
 * | 0  | 0  | 4   | 8   | 4  | 8  | uchar_8 * pointer;     // Points to the 'NetworkBufferDescriptor_t'.
 * | 4  | 8  | 4   | 8   | 6  | 6  | uchar_8   filler[6];   // To give the +2 byte offset.
 * |-------------------------------|
 * | 10 | 14 | 4+2 | 8+2 | 6  | 6  | uchar_8   dest_mac[6]; // Destination address.
 * | 16 | 20 | 4   | 8   | 6  | 6  | uchar_8   src_mac[6];  // Source address.
 * | 22 | 26 | 4+2 | 4+2 | 2  | 2  | uchar16_t ethertype;
 * | 24 | 28 | 4   | 4   | ~  | ~  | << IP-header >>        // word-aligned, either 4 or 8 bytes.
 *  uint8_t ucVersionHeaderLength;
 *  etc
 */

/* Use setting from FreeRTOS if defined and non-zero */
#if ( ipconfigBUFFER_PADDING != 0 )
    #define ipBUFFER_PADDING    ipconfigBUFFER_PADDING
#elif ( UINTPTR_MAX > 0xFFFFFFFFU )
    #define ipBUFFER_PADDING    ( 12U + ipconfigPACKET_FILLER_SIZE )
#else
    #define ipBUFFER_PADDING    ( 8U + ipconfigPACKET_FILLER_SIZE )
#endif

/* The offset of ucTCPFlags within the TCP header. */
#define ipTCP_FLAGS_OFFSET      13U

/** @brief Returned to indicate a valid checksum. */
#define ipCORRECT_CRC           0xffffU

/** @brief Returned to indicate incorrect checksum. */
#define ipWRONG_CRC             0x0000U

/** @brief Returned as the (invalid) checksum when the length of the data being checked
 * had an invalid length. */
#define ipINVALID_LENGTH        0x1234U

/** @brief Returned as the (invalid) checksum when the protocol being checked is not
 * handled.  The value is chosen simply to be easy to spot when debugging. */
#define ipUNHANDLED_PROTOCOL    0x4321U

/* Trace macros to aid in debugging, disabled if ipconfigHAS_PRINTF != 1 */
#if ( ipconfigHAS_PRINTF == 1 )
    #define DEBUG_DECLARE_TRACE_VARIABLE( type, var, init )    type var = ( init ) /**< Trace macro to set "type var = init". */
    #define DEBUG_SET_TRACE_VARIABLE( var, value )             var = ( value )     /**< Trace macro to set var = value. */
#else
    #define DEBUG_DECLARE_TRACE_VARIABLE( type, var, init )                        /**< Empty definition since ipconfigHAS_PRINTF != 1. */
    #define DEBUG_SET_TRACE_VARIABLE( var, value )                                 /**< Empty definition since ipconfigHAS_PRINTF != 1. */
#endif

/**
 * The structure used to store buffers and pass them around the network stack.
 * Buffers can be in use by the stack, in use by the network interface hardware
 * driver, or free (not in use).
 */
typedef struct xNETWORK_BUFFER
{
    ListItem_t xBufferListItem;                /**< Used to reference the buffer form the free buffer list or a socket. */
    IP_Address_t xIPAddress;                   /**< Source or destination IP address, depending on usage scenario. */
    uint8_t * pucEthernetBuffer;               /**< Pointer to the start of the Ethernet frame. */
    size_t xDataLength;                        /**< Starts by holding the total Ethernet frame length, then the UDP/TCP payload length. */
    struct xNetworkInterface * pxInterface;    /**< The interface on which the packet was received. */
    struct xNetworkEndPoint * pxEndPoint;      /**< The end-point through which this packet shall be sent. */
    uint16_t usPort;                           /**< Source or destination port, depending on usage scenario. */
    uint16_t usBoundPort;                      /**< The port to which a transmitting socket is bound. */
    #if ( ipconfigUSE_LINKED_RX_MESSAGES != 0 )
        struct xNETWORK_BUFFER * pxNextBuffer; /**< Possible optimisation for expert users - requires network driver support. */
    #endif

#define ul_IPAddress     xIPAddress.xIP_IPv4
#define x_IPv6Address    xIPAddress.xIP_IPv6
} NetworkBufferDescriptor_t;

#include "pack_struct_start.h"

/**
 * MAC address structure.
 */
struct xMAC_ADDRESS
{
    uint8_t ucBytes[ ipMAC_ADDRESS_LENGTH_BYTES ]; /**< Byte array of the MAC address */
}
#include "pack_struct_end.h"

typedef struct xMAC_ADDRESS MACAddress_t;

typedef enum eNETWORK_EVENTS
{
    eNetworkUp,  /* The network is configured. */
    eNetworkDown /* The network connection has been lost. */
} eIPCallbackEvent_t;

/* MISRA check: some modules refer to this typedef even though
 * ipconfigSUPPORT_OUTGOING_PINGS is not enabled. */
typedef enum ePING_REPLY_STATUS
{
    eSuccess = 0,     /**< A correct reply has been received for an outgoing ping. */
    eInvalidChecksum, /**< A reply was received for an outgoing ping but the checksum of the reply was incorrect. */
    eInvalidData      /**< A reply was received to an outgoing ping but the payload of the reply was not correct. */
} ePingReplyStatus_t;

/**
 * The software timer struct for various IP functions
 */
typedef struct xIP_TIMER
{
    uint32_t
        bActive : 1,            /**< This timer is running and must be processed. */
        bExpired : 1;           /**< Timer has expired and a task must be processed. */
    TimeOut_t xTimeOut;         /**< The timeout value. */
    TickType_t ulRemainingTime; /**< The amount of time remaining. */
    TickType_t ulReloadTime;    /**< The value of reload time. */
} IPTimer_t;


/* Endian related definitions. */
#if ( ipconfigBYTE_ORDER == pdFREERTOS_LITTLE_ENDIAN )

/* FreeRTOS_htons / FreeRTOS_htonl: some platforms might have built-in versions
 * using a single instruction so allow these versions to be overridden. */
    #ifndef FreeRTOS_htons
        #define FreeRTOS_htons( usIn )    ( ( uint16_t ) ( ( ( usIn ) << 8U ) | ( ( usIn ) >> 8U ) ) )
    #endif

    #ifndef FreeRTOS_htonl
        #define FreeRTOS_htonl( ulIn )                             \
    (                                                              \
        ( uint32_t )                                               \
        (                                                          \
            ( ( ( ( uint32_t ) ( ulIn ) ) ) << 24 ) |              \
            ( ( ( ( uint32_t ) ( ulIn ) ) & 0x0000ff00U ) << 8 ) | \
            ( ( ( ( uint32_t ) ( ulIn ) ) & 0x00ff0000U ) >> 8 ) | \
            ( ( ( ( uint32_t ) ( ulIn ) ) ) >> 24 )                \
        )                                                          \
    )
    #endif /* ifndef FreeRTOS_htonl */

#else /* ipconfigBYTE_ORDER */

    #define FreeRTOS_htons( x )    ( ( uint16_t ) ( x ) )
    #define FreeRTOS_htonl( x )    ( ( uint32_t ) ( x ) )

#endif /* ipconfigBYTE_ORDER == pdFREERTOS_LITTLE_ENDIAN */

#define FreeRTOS_ntohs( x )    FreeRTOS_htons( x )
#define FreeRTOS_ntohl( x )    FreeRTOS_htonl( x )

/* Translate a pdFREERTOS_ERRNO code to a human readable string. */
const char * FreeRTOS_strerror_r( BaseType_t xErrnum,
                                  char * pcBuffer,
                                  size_t uxLength );

/* Some simple helper functions. */
int32_t FreeRTOS_max_int32( int32_t a,
                            int32_t b );

uint32_t FreeRTOS_max_uint32( uint32_t a,
                              uint32_t b );

size_t FreeRTOS_max_size_t( size_t a,
                            size_t b );

int32_t FreeRTOS_min_int32( int32_t a,
                            int32_t b );

uint32_t FreeRTOS_min_uint32( uint32_t a,
                              uint32_t b );

size_t FreeRTOS_min_size_t( size_t a,
                            size_t b );

int32_t FreeRTOS_add_int32( int32_t a,
                            int32_t b );
int32_t FreeRTOS_multiply_int32( int32_t a,
                                 int32_t b );

uint32_t FreeRTOS_round_up( uint32_t a,
                            uint32_t d );
uint32_t FreeRTOS_round_down( uint32_t a,
                              uint32_t d );

#define ipMS_TO_MIN_TICKS( xTimeInMs )    ( ( pdMS_TO_TICKS( ( xTimeInMs ) ) < ( ( TickType_t ) 1U ) ) ? ( ( TickType_t ) 1U ) : pdMS_TO_TICKS( ( xTimeInMs ) ) )

/* For backward compatibility. */
#define pdMS_TO_MIN_TICKS( xTimeInMs )    ipMS_TO_MIN_TICKS( xTimeInMs )

#ifndef pdTRUE_SIGNED
    #define pdTRUE_SIGNED    pdTRUE
#endif /* pdTRUE_SIGNED */

#ifndef pdFALSE_SIGNED
    #define pdFALSE_SIGNED    pdFALSE
#endif /* pdFALSE_SIGNED */

#ifndef pdTRUE_UNSIGNED
    #define pdTRUE_UNSIGNED    ( 1U )
#endif /* pdTRUE_UNSIGNED */

#ifndef pdFALSE_UNSIGNED
    #define pdFALSE_UNSIGNED    ( 0U )
#endif /* pdFALSE_UNSIGNED */

#ifndef ipTRUE_BOOL
    #define ipTRUE_BOOL    ( pdPASS == pdPASS )
#endif /* ipTRUE_BOOL */

#ifndef ipFALSE_BOOL
    #define ipFALSE_BOOL    ( pdPASS == pdFAIL )
#endif /* ipFALSE_BOOL */

/*
 * FULL, UP-TO-DATE AND MAINTAINED REFERENCE DOCUMENTATION FOR ALL THESE
 * FUNCTIONS IS AVAILABLE ON THE FOLLOWING URL:
 * https://freertos.org/Documentation/03-Libraries/02-FreeRTOS-plus/02-FreeRTOS-plus-TCP/09-API-reference/01-FreeRTOS-plus-TCP-APIs
 */

/* FreeRTOS_IPInit_Multi() replaces the earlier FreeRTOS_IPInit().  It assumes
 * that network interfaces and IP-addresses have been added using the functions
 * from FreeRTOS_Routing.h. */
BaseType_t FreeRTOS_IPInit_Multi( void );

struct xNetworkInterface;

#if ( ipconfigIPv4_BACKWARD_COMPATIBLE == 1 )

/* Do not call the following function directly. It is there for downward compatibility.
 * The function FreeRTOS_IPInit() will call it to initialise the interface and end-point
 * objects.  See the description in FreeRTOS_Routing.h. */
    struct xNetworkInterface * pxFillInterfaceDescriptor( BaseType_t xEMACIndex,
                                                          struct xNetworkInterface * pxInterface );

/* The following function is only provided to allow backward compatibility
 * with the earlier version of FreeRTOS+TCP which had a single interface only. */
    BaseType_t FreeRTOS_IPInit( const uint8_t ucIPAddress[ ipIP_ADDRESS_LENGTH_BYTES ],
                                const uint8_t ucNetMask[ ipIP_ADDRESS_LENGTH_BYTES ],
                                const uint8_t ucGatewayAddress[ ipIP_ADDRESS_LENGTH_BYTES ],
                                const uint8_t ucDNSServerAddress[ ipIP_ADDRESS_LENGTH_BYTES ],
                                const uint8_t ucMACAddress[ ipMAC_ADDRESS_LENGTH_BYTES ] );

/* The following 2 functions also assume that there is only 1 network endpoint/interface.
 * The new function are called: FreeRTOS_GetEndPointConfiguration() and
 * FreeRTOS_SetEndPointConfiguration() */
    void FreeRTOS_GetAddressConfiguration( uint32_t * pulIPAddress,
                                           uint32_t * pulNetMask,
                                           uint32_t * pulGatewayAddress,
                                           uint32_t * pulDNSServerAddress );

    void FreeRTOS_SetAddressConfiguration( const uint32_t * pulIPAddress,
                                           const uint32_t * pulNetMask,
                                           const uint32_t * pulGatewayAddress,
                                           const uint32_t * pulDNSServerAddress );

    void * FreeRTOS_GetUDPPayloadBuffer( size_t uxRequestedSizeBytes,
                                         TickType_t uxBlockTimeTicks );

#endif /* if ( ipconfigIPv4_BACKWARD_COMPATIBLE == 1 ) */

/*
 * Returns the addresses stored in an end-point structure.
 * This function already existed in the release with the single-interface.
 * Only the first parameters is new: an end-point
 */
void FreeRTOS_GetEndPointConfiguration( uint32_t * pulIPAddress,
                                        uint32_t * pulNetMask,
                                        uint32_t * pulGatewayAddress,
                                        uint32_t * pulDNSServerAddress,
                                        const struct xNetworkEndPoint * pxEndPoint );

void FreeRTOS_SetEndPointConfiguration( const uint32_t * pulIPAddress,
                                        const uint32_t * pulNetMask,
                                        const uint32_t * pulGatewayAddress,
                                        const uint32_t * pulDNSServerAddress,
                                        struct xNetworkEndPoint * pxEndPoint );

TaskHandle_t FreeRTOS_GetIPTaskHandle( void );

void * FreeRTOS_GetUDPPayloadBuffer_Multi( size_t uxRequestedSizeBytes,
                                           TickType_t uxBlockTimeTicks,
                                           uint8_t ucIPType );

/* MISRA defining 'FreeRTOS_SendPingRequest' should be dependent on 'ipconfigSUPPORT_OUTGOING_PINGS'.
 * In order not to break some existing project, define it unconditionally. */
BaseType_t FreeRTOS_SendPingRequest( uint32_t ulIPAddress,
                                     size_t uxNumberOfBytesToSend,
                                     TickType_t uxBlockTimeTicks );

void FreeRTOS_ReleaseUDPPayloadBuffer( void const * pvBuffer );
const uint8_t * FreeRTOS_GetMACAddress( void );
void FreeRTOS_UpdateMACAddress( const uint8_t ucMACAddress[ ipMAC_ADDRESS_LENGTH_BYTES ] );
#if ( ipconfigUSE_NETWORK_EVENT_HOOK == 1 )
    /* This function shall be defined by the application. */
    #if ( ipconfigIPv4_BACKWARD_COMPATIBLE == 1 )
        void vApplicationIPNetworkEventHook( eIPCallbackEvent_t eNetworkEvent );
    #else
        void vApplicationIPNetworkEventHook_Multi( eIPCallbackEvent_t eNetworkEvent,
                                                   struct xNetworkEndPoint * pxEndPoint );
    #endif
#endif
#if ( ipconfigSUPPORT_OUTGOING_PINGS == 1 )
    void vApplicationPingReplyHook( ePingReplyStatus_t eStatus,
                                    uint16_t usIdentifier );
#endif

BaseType_t FreeRTOS_IsNetworkUp( void );

#if ( ipconfigCHECK_IP_QUEUE_SPACE != 0 )
    UBaseType_t uxGetMinimumIPQueueSpace( void );
#endif

BaseType_t xIsNetworkDownEventPending( void );

/*
 * Defined in FreeRTOS_Sockets.c
 * //_RB_ Don't think this comment is correct.  If this is for internal use only it should appear after all the public API functions and not start with FreeRTOS_.
 * Socket has had activity, reset the timer so it will not be closed
 * because of inactivity
 */
#if ( ( ipconfigHAS_DEBUG_PRINTF != 0 ) || ( ipconfigHAS_PRINTF != 0 ) )
    const char * FreeRTOS_GetTCPStateName( UBaseType_t ulState );
#endif

#if ( ipconfigDHCP_REGISTER_HOSTNAME == 1 )

/* DHCP has an option for clients to register their hostname.  It doesn't
 * have much use, except that a device can be found in a router along with its
 * name. If this option is used the callback below must be provided by the
 * application writer to return a const string, denoting the device's name. */
/* Typically this function is defined in a user module. */
    const char * pcApplicationHostnameHook( void );

#endif /* ipconfigDHCP_REGISTER_HOSTNAME */


/* This xApplicationGetRandomNumber() will set *pulNumber to a random number,
 * and return pdTRUE. When the random number generator is broken, it shall return
 * pdFALSE.
 * The function is defined in 'iot_secure_sockets.c'.
 * If that module is not included in the project, the application must provide an
 * implementation of it.
 * The macro's ipconfigRAND32() and configRAND32() are not in use anymore. */

/* "xApplicationGetRandomNumber" is declared but never defined, because it may
 * be defined in a user module. */
BaseType_t xApplicationGetRandomNumber( uint32_t * pulNumber );

/** @brief The pointers to buffers with packet waiting for resolution. These variables
 *  are defined in FreeRTOS_IP.c.
 *  These pointers are for internal use only. */
extern NetworkBufferDescriptor_t * pxARPWaitingNetworkBuffer;
extern NetworkBufferDescriptor_t * pxNDWaitingNetworkBuffer;

#if ( ipconfigENABLE_BACKWARD_COMPATIBILITY == 1 )
    #define xIPStackEvent_t               IPStackEvent_t
    #define xNetworkBufferDescriptor_t    NetworkBufferDescriptor_t
    #define xMACAddress_t                 MACAddress_t
    #define xWinProperties_t              WinProperties_t
    #define xSocket_t                     Socket_t
    #define xSocketSet_t                  SocketSet_t
    #define ipSIZE_OF_IP_HEADER           ipSIZE_OF_IPv4_HEADER

/* Since August 2016, the public types and fields below have changed name:
 * abbreviations TCP/UDP are now written in capitals, and type names now end with "_t". */
    #define FOnConnected                  FOnConnected_t
    #define FOnTcpReceive                 FOnTCPReceive_t
    #define FOnTcpSent                    FOnTCPSent_t
    #define FOnUdpReceive                 FOnUDPReceive_t
    #define FOnUdpSent                    FOnUDPSent_t

    #define pOnTcpConnected               pxOnTCPConnected
    #define pOnTcpReceive                 pxOnTCPReceive
    #define pOnTcpSent                    pxOnTCPSent
    #define pOnUdpReceive                 pxOnUDPReceive
    #define pOnUdpSent                    pxOnUDPSent

    #define FOnUdpSent                    FOnUDPSent_t
    #define FOnTcpSent                    FOnTCPSent_t
#endif /* ipconfigENABLE_BACKWARD_COMPATIBILITY */

#if ( ipconfigHAS_PRINTF != 0 )
    extern void vPrintResourceStats( void );
#else
    #define vPrintResourceStats()    do {} while( ipFALSE_BOOL )     /**< ipconfigHAS_PRINTF is not defined. Define vPrintResourceStats to a do-while( 0 ). */
#endif

#include "FreeRTOS_IP_Utils.h"
#include "FreeRTOS_IPv4.h"
#include "FreeRTOS_IPv6.h"

/* *INDENT-OFF* */
#ifdef __cplusplus
    } /* extern "C" */
#endif
/* *INDENT-ON* */

#endif /* FREERTOS_IP_H */
