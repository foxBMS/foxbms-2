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
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 */

#ifndef FREERTOS_IP_PRIVATE_H
#define FREERTOS_IP_PRIVATE_H

/* Application level configuration options. */
#include "FreeRTOSIPConfig.h"
#include "FreeRTOSIPConfigDefaults.h"
#include "FreeRTOS_Sockets.h"
#include "FreeRTOS_Stream_Buffer.h"
#include "FreeRTOS_Routing.h"

#if ( ipconfigUSE_TCP == 1 )
    #include "FreeRTOS_TCP_WIN.h"
    #include "FreeRTOS_TCP_IP.h"
#endif

#include "semphr.h"

#include "event_groups.h"

/* *INDENT-OFF* */
#ifdef __cplusplus
    extern "C" {
#endif
/* *INDENT-ON* */

#ifdef TEST
    int ipFOREVER( void );
#else
    #define ipFOREVER()    1
#endif

/* Forward declaration. */
struct xNetworkEndPoint;

typedef enum eFrameProcessingResult
{
    eReleaseBuffer = 0,   /* Processing the frame did not find anything to do - just release the buffer. */
    eProcessBuffer,       /* An Ethernet frame has a valid address - continue process its contents. */
    eReturnEthernetFrame, /* The Ethernet frame contains an ARP or ICMP packet that can be returned to its source. */
    eFrameConsumed,       /* Processing the Ethernet packet contents resulted in the payload being sent to the stack. */
    eWaitingResolution    /* Frame is awaiting resolution. */
} eFrameProcessingResult_t;

typedef enum
{
    eNoEvent = -1,
    eNetworkDownEvent,    /* 0: The network interface has been lost and/or needs [re]connecting. */
    eNetworkRxEvent,      /* 1: The network interface has queued a received Ethernet frame. */
    eNetworkTxEvent,      /* 2: Let the IP-task send a network packet. */
    eARPTimerEvent,       /* 3: The ARP timer expired. */
    eNDTimerEvent,        /* 4: The ND timer expired. */
    eStackTxEvent,        /* 5: The software stack has queued a packet to transmit. */
    eDHCPEvent,           /* 6: Process the DHCP state machine. */
    eTCPTimerEvent,       /* 7: See if any TCP socket needs attention. */
    eTCPAcceptEvent,      /* 8: Client API FreeRTOS_accept() waiting for client connections. */
    eTCPNetStat,          /* 9: IP-task is asked to produce a netstat listing. */
    eSocketBindEvent,     /*10: Send a message to the IP-task to bind a socket to a port. */
    eSocketCloseEvent,    /*11: Send a message to the IP-task to close a socket. */
    eSocketSelectEvent,   /*12: Send a message to the IP-task for select(). */
    eSocketSignalEvent,   /*13: A socket must be signalled. */
    eSocketSetDeleteEvent /*14: A socket set must be deleted. */
} eIPEvent_t;

/**
 * Structure to hold the information about the Network parameters.
 */
typedef struct xNetworkAddressingParameters
{
    uint32_t ulDefaultIPAddress; /**< The default IP address */
    uint32_t ulNetMask;          /**< The netmask */
    uint32_t ulGatewayAddress;   /**< The gateway address */
    uint32_t ulDNSServerAddress; /**< The DNS server address */
    uint32_t ulBroadcastAddress; /**< The Broadcast address */
} NetworkAddressingParameters_t;

extern BaseType_t xTCPWindowLoggingLevel;
extern QueueHandle_t xNetworkEventQueue;
typedef struct xSOCKET FreeRTOS_Socket_t;

/*-----------------------------------------------------------*/
/* Protocol headers.                                         */
/*-----------------------------------------------------------*/

#include "pack_struct_start.h"
struct xETH_HEADER
{
    MACAddress_t xDestinationAddress; /**< Destination address  0 + 6 = 6  */
    MACAddress_t xSourceAddress;      /**< Source address       6 + 6 = 12 */
    uint16_t usFrameType;             /**< The EtherType field 12 + 2 = 14 */
}
#include "pack_struct_end.h"
typedef struct xETH_HEADER EthernetHeader_t;

#include "pack_struct_start.h"
struct xARP_HEADER
{
    uint16_t usHardwareType;              /**< Network Link Protocol type                     0 +  2 =  2 */
    uint16_t usProtocolType;              /**< The internetwork protocol                      2 +  2 =  4 */
    uint8_t ucHardwareAddressLength;      /**< Length in octets of a hardware address         4 +  1 =  5 */
    uint8_t ucProtocolAddressLength;      /**< Length in octets of the internetwork protocol  5 +  1 =  6 */
    uint16_t usOperation;                 /**< Operation that the sender is performing        6 +  2 =  8 */
    MACAddress_t xSenderHardwareAddress;  /**< Media address of the sender                    8 +  6 = 14 */
    uint8_t ucSenderProtocolAddress[ 4 ]; /**< Internetwork address of sender                14 +  4 = 18  */
    MACAddress_t xTargetHardwareAddress;  /**< Media address of the intended receiver        18 +  6 = 24  */
    uint32_t ulTargetProtocolAddress;     /**< Internetwork address of the intended receiver 24 +  4 = 28  */
}
#include "pack_struct_end.h"
typedef struct xARP_HEADER ARPHeader_t;

#include "pack_struct_start.h"
struct xICMP_HEADER
{
    uint8_t ucTypeOfMessage;   /**< The ICMP type                     0 + 1 = 1 */
    uint8_t ucTypeOfService;   /**< The ICMP subtype                  1 + 1 = 2 */
    uint16_t usChecksum;       /**< The checksum of whole ICMP packet 2 + 2 = 4 */
    uint16_t usIdentifier;     /**< Used in some types of ICMP        4 + 2 = 6 */
    uint16_t usSequenceNumber; /**< Used in some types of ICMP        6 + 2 = 8 */
}
#include "pack_struct_end.h"
typedef struct xICMP_HEADER ICMPHeader_t;

#include "pack_struct_start.h"
struct xICMPHeader_IPv6
{
    uint8_t ucTypeOfMessage;     /**< The message type.     0 +  1 = 1 */
    uint8_t ucTypeOfService;     /**< Type of service.      1 +  1 = 2 */
    uint16_t usChecksum;         /**< Checksum.             2 +  2 = 4 */
    uint32_t ulReserved;         /**< Reserved.             4 +  4 = 8 */
    IPv6_Address_t xIPv6Address; /**< The IPv6 address.     8 + 16 = 24 */
    uint8_t ucOptionType;        /**< The option type.     24 +  1 = 25 */
    uint8_t ucOptionLength;      /**< The option length.   25 +  1 = 26 */
    uint8_t ucOptionBytes[ 6 ];  /**< Option bytes.        26 +  6 = 32 */
}
#include "pack_struct_end.h"
typedef struct xICMPHeader_IPv6 ICMPHeader_IPv6_t;

#include "pack_struct_start.h"
struct xUDP_HEADER
{
    uint16_t usSourcePort;      /**< The source port                      0 + 2 = 2 */
    uint16_t usDestinationPort; /**< The destination port                 2 + 2 = 4 */
    uint16_t usLength;          /**< The size of the whole UDP packet     4 + 2 = 6 */
    uint16_t usChecksum;        /**< The checksum of the whole UDP Packet 6 + 2 = 8 */
}
#include "pack_struct_end.h"
typedef struct xUDP_HEADER UDPHeader_t;

#include "pack_struct_start.h"
struct xTCP_HEADER
{
    uint16_t usSourcePort;                       /**< The Source port                      +  2 =  2 */
    uint16_t usDestinationPort;                  /**< The destination port                 +  2 =  4 */
    uint32_t ulSequenceNumber;                   /**< The Sequence number                  +  4 =  8 */
    uint32_t ulAckNr;                            /**< The acknowledgement number           +  4 = 12 */
    uint8_t ucTCPOffset;                         /**< The value of TCP offset              +  1 = 13 */
    uint8_t ucTCPFlags;                          /**< The TCP-flags field                  +  1 = 14 */
    uint16_t usWindow;                           /**< The size of the receive window       +  2 = 15 */
    uint16_t usChecksum;                         /**< The checksum of the header           +  2 = 18 */
    uint16_t usUrgent;                           /**< Pointer to the last urgent data byte +  2 = 20 */
    #if ipconfigUSE_TCP == 1
        uint8_t ucOptdata[ ipSIZE_TCP_OPTIONS ]; /**< The options + 12 = 32 */
    #endif
}
#include "pack_struct_end.h"
typedef struct xTCP_HEADER TCPHeader_t;

#include "pack_struct_start.h"
struct xARP_PACKET
{
    EthernetHeader_t xEthernetHeader; /**< The ethernet header of an ARP Packet  0 + 14 = 14 */
    ARPHeader_t xARPHeader;           /**< The ARP header of an ARP Packet       14 + 28 = 42 */
}
#include "pack_struct_end.h"
typedef struct xARP_PACKET ARPPacket_t;


#include "FreeRTOS_IPv4_Private.h"
#include "FreeRTOS_IPv6_Private.h"

/**
 * Union for the protocol packet to save space. Any packet cannot have more than one
 * of the below protocol packets.
 */
typedef union XPROT_PACKET
{
    ARPPacket_t xARPPacket;   /**< Union member: ARP packet struct */
    TCPPacket_t xTCPPacket;   /**< Union member: TCP packet struct */
    UDPPacket_t xUDPPacket;   /**< Union member: UDP packet struct */
    ICMPPacket_t xICMPPacket; /**< Union member: ICMP packet struct */
} ProtocolPacket_t;

/**
 * Union for protocol headers to save space (RAM). Any packet cannot have more than one of
 * the below protocols.
 */
typedef union xPROT_HEADERS
{
    ICMPHeader_t xICMPHeader;          /**< Union member: ICMP header */
    UDPHeader_t xUDPHeader;            /**< Union member: UDP header */
    TCPHeader_t xTCPHeader;            /**< Union member: TCP header */
    ICMPHeader_IPv6_t xICMPHeaderIPv6; /**< Union member: ICMPv6 header */
} ProtocolHeaders_t;

/**
 * Structure for the information of the commands issued to the IP task.
 */
typedef struct IP_TASK_COMMANDS
{
    eIPEvent_t eEventType; /**< The event-type enum */
    void * pvData;         /**< The data in the event */
} IPStackEvent_t;

/** @brief This struct describes a packet, it is used by the function
 * usGenerateProtocolChecksum(). */
struct xPacketSummary
{
    BaseType_t xIsIPv6;                          /**< pdTRUE for IPv6 packets. */
    #if ipconfigUSE_IPv6
        const IPHeader_IPv6_t * pxIPPacket_IPv6; /**< A pointer to the IPv6 header. */
    #endif
    #if ( ipconfigHAS_DEBUG_PRINTF != 0 )
        const char * pcType;               /**< Just for logging purposes: the name of the protocol. */
    #endif
    size_t uxIPHeaderLength;               /**< Either 40 or 20, depending on the IP-type */
    size_t uxProtocolHeaderLength;         /**< Either 8, 20, or more or 20, depending on the protocol-type */
    uint16_t usChecksum;                   /**< Checksum accumulator. */
    uint8_t ucProtocol;                    /**< ipPROTOCOL_TCP, ipPROTOCOL_UDP, ipPROTOCOL_ICMP */
    const IPPacket_t * pxIPPacket;         /**< A pointer to the IPv4 header. */
    ProtocolHeaders_t * pxProtocolHeaders; /**< Points to first byte after IP-header */
    uint16_t usPayloadLength;              /**< Property of IP-header (for IPv4: length of IP-header included) */
    uint16_t usProtocolBytes;              /**< The total length of the protocol data. */
};

#define ipBROADCAST_IP_ADDRESS               0xffffffffU

/* Offset into the Ethernet frame that is used to temporarily store information
 * on the fragmentation status of the packet being sent.  The value is important,
 * as it is past the location into which the destination address will get placed. */
#define ipFRAGMENTATION_PARAMETERS_OFFSET    ( 6 )
#define ipSOCKET_OPTIONS_OFFSET              ( 6 )


#if ( ipconfigBYTE_ORDER == pdFREERTOS_LITTLE_ENDIAN )

/* Ethernet frame types. */
    #define ipARP_FRAME_TYPE                   ( 0x0608U )
    #define ipIPv4_FRAME_TYPE                  ( 0x0008U )

/* ARP related definitions. */
    #define ipARP_PROTOCOL_TYPE                ( 0x0008U )
    #define ipARP_HARDWARE_TYPE_ETHERNET       ( 0x0100U )
    #define ipARP_REQUEST                      ( 0x0100U )
    #define ipARP_REPLY                        ( 0x0200U )

/* The bits in the two byte IP header field that make up the fragment offset value. */
    #define ipFRAGMENT_OFFSET_BIT_MASK         ( ( uint16_t ) 0xff1fU )

/* The bits in the two byte IP header field that make up the flags value. */
    #define ipFRAGMENT_FLAGS_BIT_MASK          ( ( uint16_t ) 0x00e0U )

/* Don't Fragment Flag */
    #define ipFRAGMENT_FLAGS_DONT_FRAGMENT     ( ( uint16_t ) 0x0040U )

/* More Fragments Flag */
    #define ipFRAGMENT_FLAGS_MORE_FRAGMENTS    ( ( uint16_t ) 0x0020U )

#else /* if ( ipconfigBYTE_ORDER == pdFREERTOS_LITTLE_ENDIAN ) */

/* Ethernet frame types. */
    #define ipARP_FRAME_TYPE                   ( 0x0806U )
    #define ipIPv4_FRAME_TYPE                  ( 0x0800U )

/* ARP related definitions. */
    #define ipARP_PROTOCOL_TYPE                ( 0x0800U )
    #define ipARP_HARDWARE_TYPE_ETHERNET       ( 0x0001U )
    #define ipARP_REQUEST                      ( 0x0001 )
    #define ipARP_REPLY                        ( 0x0002 )

/* The bits in the two byte IP header field that make up the fragment offset value. */
    #define ipFRAGMENT_OFFSET_BIT_MASK         ( ( uint16_t ) 0x1fffU )

/* The bits in the two byte IP header field that make up the flags value. */
    #define ipFRAGMENT_FLAGS_BIT_MASK          ( ( uint16_t ) 0xe000U )

/* Don't Fragment Flag */
    #define ipFRAGMENT_FLAGS_DONT_FRAGMENT     ( ( uint16_t ) 0x4000U )

/* More Fragments Flag */
    #define ipFRAGMENT_FLAGS_MORE_FRAGMENTS    ( ( uint16_t ) 0x2000U )

#endif /* ipconfigBYTE_ORDER */

/* For convenience, a MAC address of all zeros and another of all 0xffs are
 * defined const for quick reference. */
extern const MACAddress_t xBroadcastMACAddress; /* all 0xff's */
extern uint16_t usPacketIdentifier;

/** @brief The list that contains mappings between sockets and port numbers.
 *         Accesses to this list must be protected by critical sections of
 *         some kind.
 */
extern List_t xBoundUDPSocketsList;

/* True when BufferAllocation_1.c was included, false for BufferAllocation_2.c */
extern const BaseType_t xBufferAllocFixedSize;

/* As FreeRTOS_Routing is included later, use forward declarations
 * of the two structs. */
struct xNetworkEndPoint;
struct xNetworkInterface;

/* A list of all network end-points: */
extern struct xNetworkEndPoint * pxNetworkEndPoints;

/* A list of all network interfaces: */
extern struct xNetworkInterface * pxNetworkInterfaces;

/* Defined in FreeRTOS_Sockets.c */
#if ( ipconfigUSE_TCP == 1 )
    extern List_t xBoundTCPSocketsList;
#endif

/* ICMP packets are sent using the same function as UDP packets.  The port
 * number is used to distinguish between the two, as 0 is an invalid UDP port. */
#define ipPACKET_CONTAINS_ICMP_DATA    ( 0 )

/* For now, the lower 8 bits in 'xEventBits' will be reserved for the above
 * socket events. */
#define SOCKET_EVENT_BIT_COUNT         8

#define vSetField16( pxBase, xType, xField, usValue )                                                        \
    do {                                                                                                     \
        ( ( uint8_t * ) ( pxBase ) )[ offsetof( xType, xField ) + 0 ] = ( uint8_t ) ( ( usValue ) >> 8 );    \
        ( ( uint8_t * ) ( pxBase ) )[ offsetof( xType, xField ) + 1 ] = ( uint8_t ) ( ( usValue ) & 0xffU ); \
    } while( ipFALSE_BOOL )

#define vSetField32( pxBase, xType, xField, ulValue )                                                                  \
    {                                                                                                                  \
        ( ( uint8_t * ) ( pxBase ) )[ offsetof( xType, xField ) + 0 ] = ( uint8_t ) ( ( ulValue ) >> 24 );             \
        ( ( uint8_t * ) ( pxBase ) )[ offsetof( xType, xField ) + 1 ] = ( uint8_t ) ( ( ( ulValue ) >> 16 ) & 0xffU ); \
        ( ( uint8_t * ) ( pxBase ) )[ offsetof( xType, xField ) + 2 ] = ( uint8_t ) ( ( ( ulValue ) >> 8 ) & 0xffU );  \
        ( ( uint8_t * ) ( pxBase ) )[ offsetof( xType, xField ) + 3 ] = ( uint8_t ) ( ( ulValue ) & 0xffU );           \
    }

#define vFlip_16( left, right )  \
    do {                         \
        uint16_t tmp = ( left ); \
        ( left ) = ( right );    \
        ( right ) = tmp;         \
    } while( ipFALSE_BOOL )

#define vFlip_32( left, right )  \
    do {                         \
        uint32_t tmp = ( left ); \
        ( left ) = ( right );    \
        ( right ) = tmp;         \
    } while( ipFALSE_BOOL )

/** @brief Macro calculates the number of elements in an array as a size_t. */
#ifndef ARRAY_SIZE_X
    #ifndef _WINDOWS_
        #define ARRAY_SIZE_X( x )    ( ( BaseType_t ) sizeof( x ) / ( BaseType_t ) sizeof( x[ 0 ] ) )
    #else
        #define ARRAY_SIZE_X( x )    ( sizeof( x ) / sizeof( x[ 0 ] ) )
    #endif
#endif

/* WARNING: Do NOT use this macro when the array was received as a parameter. */
#ifndef ARRAY_SIZE
    #define ARRAY_SIZE( x )    ( ( BaseType_t ) ( sizeof( x ) / sizeof( ( x )[ 0 ] ) ) )
#endif


#ifndef ARRAY_USIZE
    #define ARRAY_USIZE( x )    ( ( UBaseType_t ) ( sizeof( x ) / sizeof( ( x )[ 0 ] ) ) )
#endif

/*
 * Create a message that contains a command to initialise the network interface.
 * This is used during initialisation, and at any time the network interface
 * goes down thereafter.  The network interface hardware driver is responsible
 * for sending the message that contains the network interface down command/
 * event.
 *
 * Only use the FreeRTOS_NetworkDownFromISR() version if the function is to be
 * called from an interrupt service routine.  If FreeRTOS_NetworkDownFromISR()
 * returns a non-zero value then a context switch should be performed before
 * the interrupt is exited.
 */
void FreeRTOS_NetworkDown( struct xNetworkInterface * pxNetworkInterface );
BaseType_t FreeRTOS_NetworkDownFromISR( struct xNetworkInterface * pxNetworkInterface );

/*
 * Processes incoming ARP packets.
 */
eFrameProcessingResult_t eARPProcessPacket( const NetworkBufferDescriptor_t * pxNetworkBuffer );

/*
 * Inspect an Ethernet frame to see if it contains data that the stack needs to
 * process.  eProcessBuffer is returned if the frame should be processed by the
 * stack.  eReleaseBuffer is returned if the frame should be discarded.
 */
eFrameProcessingResult_t eConsiderFrameForProcessing( const uint8_t * const pucEthernetBuffer );

/*
 * Return the checksum generated over xDataLengthBytes from pucNextData.
 */
uint16_t usGenerateChecksum( uint16_t usSum,
                             const uint8_t * pucNextData,
                             size_t uxByteCount );

/* Socket related private functions. */

/*
 * Initialize the socket list data structures for TCP and UDP.
 */
void vNetworkSocketsInit( void );

/*
 * Returns pdTRUE if the IP task has been created and is initialised.  Otherwise
 * returns pdFALSE.
 */
BaseType_t xIPIsNetworkTaskReady( void );

#if ( ipconfigSOCKET_HAS_USER_WAKE_CALLBACK == 1 )
    struct xSOCKET;
    typedef void (* SocketWakeupCallback_t)( struct xSOCKET * pxSocket );
#endif

#if ( ipconfigUSE_TCP == 1 )

/*
 * Actually a user thing, but because xBoundTCPSocketsList, let it do by the
 * IP-task
 */
    #if ( ipconfigHAS_PRINTF != 0 )
        void vTCPNetStat( void );
    #endif

/*
 * At least one socket needs to check for timeouts
 */
    TickType_t xTCPTimerCheck( BaseType_t xWillSleep );

/**
 * About the TCP flags 'bPassQueued' and 'bPassAccept':
 *
 * When a new TCP connection request is received on a listening socket, the bPassQueued and
 * bPassAccept members of the newly created socket are updated as follows:
 *
 * 1. bPassQueued is set to indicate that the 3-way TCP handshake is in progress.
 * 2. When the 3-way TCP handshake is complete, bPassQueued is cleared. At the same time,
 *    bPassAccept is set to indicate that the socket is ready to be picked up by the task
 *    that called FreeRTOS_accept().
 * 3. When the socket is picked up by the task that called FreeRTOS_accept, the bPassAccept
 *    is cleared.
 */

/**
 * Every TCP socket has a buffer space just big enough to store
 * the last TCP header received.
 * As a reference of this field may be passed to DMA, force the
 * alignment to 8 bytes.
 */
    typedef union
    {
        struct
        {
            uint32_t ullAlignmentWord; /**< Increase the alignment of this union by adding a 32-bit variable. */
        } a;                           /**< A struct to increase alignment. */
        struct
        {
            /* The next field only serves to give 'ucLastPacket' a correct
             * alignment of 4 + 2.  See comments in FreeRTOS_IP.h */
            uint8_t ucFillPacket[ ipconfigPACKET_FILLER_SIZE ];
            uint8_t ucLastPacket[ TCP_PACKET_SIZE ];
        } u; /**< The structure to give an alignment of 4 + 2 */
    } LastTCPPacket_t;

/**
 * Note that the values of all short and long integers in these structs
 * are being stored in the native-endian way
 * Translation should take place when accessing any structure which defines
 * network packets, such as IPHeader_t and TCPHeader_t
 */
    typedef struct TCPSOCKET
    {
        IP_Address_t xRemoteIP; /**< IP address of remote machine */
        uint16_t usRemotePort;  /**< Port on remote machine */
        struct
        {
            /* Most compilers do like bit-flags */
            uint32_t
                bMssChange : 1,        /**< This socket has seen a change in MSS */
                bPassAccept : 1,       /**< See comment here above. */
                bPassQueued : 1,       /**< See comment here above. */
                bReuseSocket : 1,      /**< When a listening socket gets a connection, do not create a new instance but keep on using it */
                bCloseAfterSend : 1,   /**< As soon as the last byte has been transmitted, finalise the connection
                                        * Useful in e.g. FTP connections, where the last data bytes are sent along with the FIN flag */
                bUserShutdown : 1,     /**< User requesting a graceful shutdown */
                bCloseRequested : 1,   /**< Request to finalise the connection */
                bLowWater : 1,         /**< high-water level has been reached. Cleared as soon as 'rx-count < lo-water' */
                bWinChange : 1,        /**< The value of bLowWater has changed, must send a window update */
                bSendKeepAlive : 1,    /**< When this flag is true, a TCP keep-alive message must be send */
                bWaitKeepAlive : 1,    /**< When this flag is true, a TCP keep-alive reply is expected */
                bConnPrepared : 1,     /**< Connecting socket: Message has been prepared */
            #if ( ipconfigSUPPORT_SELECT_FUNCTION == 1 )
                bConnPassed : 1,       /**< Connecting socket: Socket has been passed in a successful select()  */
            #endif /* ipconfigSUPPORT_SELECT_FUNCTION */
            bFinAccepted : 1,          /**< This socket has received (or sent) a FIN and accepted it */
                bFinSent : 1,          /**< We've sent out a FIN */
                bFinRecv : 1,          /**< We've received a FIN from our peer */
                bFinAcked : 1,         /**< Our FIN packet has been acked */
                bFinLast : 1,          /**< The last ACK (after FIN and FIN+ACK) has been sent or will be sent by the peer */
                bRxStopped : 1,        /**< Application asked to temporarily stop reception */
                bMallocError : 1,      /**< There was an error allocating a stream */
                bWinScaling : 1;       /**< A TCP-Window Scaling option was offered and accepted in the SYN phase. */
        } bits;                        /**< The bits structure */
        uint32_t ulHighestRxAllowed;   /**< The highest sequence number that we can receive at any moment */
        uint16_t usTimeout;            /**< Time (in ticks) after which this socket needs attention */
        uint16_t usMSS;                /**< Current Maximum Segment Size */
        uint16_t usChildCount;         /**< In case of a listening socket: number of connections on this port number */
        uint16_t usBacklog;            /**< In case of a listening socket: maximum number of concurrent connections on this port number */
        uint8_t ucRepCount;            /**< Send repeat count, for retransmissions
                                        * This counter is separate from the xmitCount in the
                                        * TCP win segments */
        eIPTCPState_t eTCPState;       /**< TCP state: see eTCP_STATE */
        struct xSOCKET * pxPeerSocket; /**< for server socket: child, for child socket: parent */
        #if ( ipconfigTCP_KEEP_ALIVE == 1 )
            uint8_t ucKeepRepCount;
            TickType_t xLastAliveTime; /**< The last value of keepalive time.*/
        #endif /* ipconfigTCP_KEEP_ALIVE */
        #if ( ipconfigTCP_HANG_PROTECTION == 1 )
            TickType_t xLastActTime;                  /**< The last time when hang-protection was done.*/
        #endif /* ipconfigTCP_HANG_PROTECTION */
        size_t uxLittleSpace;                         /**< The value deemed as low amount of space. */
        size_t uxEnoughSpace;                         /**< The value deemed as enough space. */
        size_t uxRxStreamSize;                        /**< The Receive stream size */
        size_t uxTxStreamSize;                        /**< The transmit stream size */
        StreamBuffer_t * rxStream;                    /**< The pointer to the receive stream buffer. */
        StreamBuffer_t * txStream;                    /**< The pointer to the transmit stream buffer. */
        #if ( ipconfigUSE_TCP_WIN == 1 )
            NetworkBufferDescriptor_t * pxAckMessage; /**< The pointer to the ACK message */
        #endif /* ipconfigUSE_TCP_WIN */
        LastTCPPacket_t xPacket;                      /**< Buffer space to store the last TCP header received. */
        uint8_t tcpflags;                             /**< TCP flags */
        #if ( ipconfigUSE_TCP_WIN != 0 )
            uint8_t ucMyWinScaleFactor;               /**< Scaling factor of this device. */
            uint8_t ucPeerWinScaleFactor;             /**< Scaling factor of the peer. */
        #endif
        #if ( ipconfigUSE_CALLBACKS == 1 )
            FOnTCPReceive_t pxHandleReceive;  /**<
                                               * In case of a TCP socket:
                                               * typedef void (* FOnTCPReceive_t) (Socket_t xSocket, void *pData, size_t xLength );
                                               */
            FOnTCPSent_t pxHandleSent;        /**< Function pointer to handle a successful send event.  */
            FOnConnected_t pxHandleConnected; /**< Actually type: typedef void (* FOnConnected_t) (Socket_t xSocket, BaseType_t ulConnected ); */
        #endif /* ipconfigUSE_CALLBACKS */
        uint32_t ulWindowSize;                /**< Current Window size advertised by peer */
        size_t uxRxWinSize;                   /**< Fixed value: size of the TCP reception window */
        size_t uxTxWinSize;                   /**< Fixed value: size of the TCP transmit window */

        TCPWindow_t xTCPWindow;               /**< The TCP window struct*/
    } IPTCPSocket_t;

#endif /* ipconfigUSE_TCP */

/**
 * Structure to hold the information about a UDP socket.
 */
typedef struct UDPSOCKET
{
    List_t xWaitingPacketsList;   /**< Incoming packets */
    #if ( ipconfigUDP_MAX_RX_PACKETS > 0 )
        UBaseType_t uxMaxPackets; /**< Protection: limits the number of packets buffered per socket */
    #endif /* ipconfigUDP_MAX_RX_PACKETS */
    #if ( ipconfigUSE_CALLBACKS == 1 )
        FOnUDPReceive_t pxHandleReceive; /**<
                                          * In case of a UDP socket:
                                          * typedef void (* FOnUDPReceive_t) (Socket_t xSocket, void *pData, size_t xLength, struct freertos_sockaddr *pxAddr );
                                          */
        FOnUDPSent_t pxHandleSent;       /**< Function pointer to handle the events after a successful send. */
    #endif /* ipconfigUSE_CALLBACKS */
} IPUDPSocket_t;

/* Formally typedef'd as eSocketEvent_t. */
enum eSOCKET_EVENT
{
    eSOCKET_RECEIVE = 0x0001,
    eSOCKET_SEND = 0x0002,
    eSOCKET_ACCEPT = 0x0004,
    eSOCKET_CONNECT = 0x0008,
    eSOCKET_BOUND = 0x0010,
    eSOCKET_CLOSED = 0x0020,
    eSOCKET_INTR = 0x0040,
    eSOCKET_ALL = 0x007F
};


/**
 * Structure to hold information for a socket.
 */
struct xSOCKET
{
    EventBits_t xEventBits;         /**< The eventbits to keep track of events. */
    EventGroupHandle_t xEventGroup; /**< The event group for this socket. */

    /* Most compilers do like bit-flags */
    struct
    {
        uint32_t bIsIPv6 : 1; /**< Non-zero in case the connection is using IPv6. */
        uint32_t bSomeFlag : 1;
    }
    bits;

    ListItem_t xBoundSocketListItem;       /**< Used to reference the socket from a bound sockets list. */
    TickType_t xReceiveBlockTime;          /**< if recv[to] is called while no data is available, wait this amount of time. Unit in clock-ticks */
    TickType_t xSendBlockTime;             /**< if send[to] is called while there is not enough space to send, wait this amount of time. Unit in clock-ticks */

    IP_Address_t xLocalAddress;            /**< Local IP address */
    uint16_t usLocalPort;                  /**< Local port on this machine */
    uint8_t ucSocketOptions;               /**< Socket options */
    uint8_t ucProtocol;                    /**< choice of FREERTOS_IPPROTO_UDP/TCP */
    #if ( ipconfigSOCKET_HAS_USER_SEMAPHORE == 1 )
        SemaphoreHandle_t pxUserSemaphore; /**< The user semaphore */
    #endif /* ipconfigSOCKET_HAS_USER_SEMAPHORE */
    #if ( ipconfigSOCKET_HAS_USER_WAKE_CALLBACK == 1 )
        SocketWakeupCallback_t pxUserWakeCallback; /**< Pointer to the callback function. */
    #endif /* ipconfigSOCKET_HAS_USER_WAKE_CALLBACK */

    #if ( ipconfigSUPPORT_SELECT_FUNCTION == 1 )
        struct xSOCKET_SET * pxSocketSet; /**< Pointer to the socket set structure */
        EventBits_t xSelectBits;          /**< User may indicate which bits are interesting for this socket. */

        EventBits_t xSocketBits;          /**< These bits indicate the events which have actually occurred.
                                           * They are maintained by the IP-task */
    #endif /* ipconfigSUPPORT_SELECT_FUNCTION */
    struct xNetworkEndPoint * pxEndPoint; /**< The end-point to which the socket is bound. */

    /* This field is only only by the user, and can be accessed with
     * vSocketSetSocketID() / vSocketGetSocketID().
     * All fields of a socket will be cleared by memset() in FreeRTOS_socket().
     */
    void * pvSocketID;

    /* TCP/UDP specific fields: */
    /* Before accessing any member of this structure, it should be confirmed */
    /* that the protocol corresponds with the type of structure */

    union
    {
        IPUDPSocket_t xUDP;     /**< Union member: UDP socket*/
        #if ( ipconfigUSE_TCP == 1 )
            IPTCPSocket_t xTCP; /**< Union member: TCP socket */
        #endif /* ipconfigUSE_TCP */
    }
    u; /**< Union of TCP/UDP socket */
};

#if ( ipconfigUSE_TCP == 1 )

/*
 * Close the socket another time.
 */
    void vSocketCloseNextTime( FreeRTOS_Socket_t * pxSocket );

/*
 * Postpone a call to listen() by the IP-task.
 */
    void vSocketListenNextTime( FreeRTOS_Socket_t * pxSocket );

/*
 * Lookup a TCP socket, using a multiple matching: both port numbers and
 * return IP address.
 */
    FreeRTOS_Socket_t * pxTCPSocketLookup( uint32_t ulLocalIP,
                                           UBaseType_t uxLocalPort,
                                           IPv46_Address_t xRemoteIP,
                                           UBaseType_t uxRemotePort );

#endif /* ipconfigUSE_TCP */


/*
 * Look up a local socket by finding a match with the local port.
 */
FreeRTOS_Socket_t * pxUDPSocketLookup( UBaseType_t uxLocalPort );

/*
 * Calculate the upper-layer checksum
 * Works both for UDP, ICMP and TCP packages
 * bOut = true: checksum will be set in outgoing packets
 * bOut = false: checksum will be calculated for incoming packets
 *     returning 0xffff means: checksum was correct
 */
uint16_t usGenerateProtocolChecksum( uint8_t * pucEthernetBuffer,
                                     size_t uxBufferLength,
                                     BaseType_t xOutgoingPacket );

/*
 * An Ethernet frame has been updated (maybe it was an ARP request or a PING
 * request?) and is to be sent back to its source.
 */
void vReturnEthernetFrame( NetworkBufferDescriptor_t * pxNetworkBuffer,
                           BaseType_t xReleaseAfterSend );

/*
 * The internal version of bind()
 * If 'ulInternal' is true, it is called by the driver
 * The TCP driver needs to bind a socket at the moment a listening socket
 * creates a new connected socket
 */
BaseType_t vSocketBind( FreeRTOS_Socket_t * pxSocket,
                        struct freertos_sockaddr * pxBindAddress,
                        size_t uxAddressLength,
                        BaseType_t xInternal );

/*
 * Internal function to add streaming data to a TCP socket. If ulIn == true,
 * data will be added to the rxStream, otherwise to the tXStream.  Normally data
 * will be written with ulOffset == 0, meaning: at the end of the FIFO.  When
 * packet come in out-of-order, an offset will be used to put it in front and
 * the head will not change yet.
 */
int32_t lTCPAddRxdata( FreeRTOS_Socket_t * pxSocket,
                       size_t uxOffset,
                       const uint8_t * pcData,
                       uint32_t ulByteCount );

/*
 * Currently called for any important event.
 */
void vSocketWakeUpUser( FreeRTOS_Socket_t * pxSocket );

/*
 * Some helping function, their meaning should be clear.
 * Going by MISRA rules, these utility functions should not be defined
 * if they are not being used anywhere. But their use depends on the
 * application and hence these functions are defined unconditionally.
 */
extern uint32_t ulChar2u32( const uint8_t * pucPtr );

extern uint16_t usChar2u16( const uint8_t * pucPtr );

/* Check a single socket for retransmissions and timeouts */
BaseType_t xTCPSocketCheck( FreeRTOS_Socket_t * pxSocket );

BaseType_t xTCPCheckNewClient( FreeRTOS_Socket_t * pxSocket );

/* Defined in FreeRTOS_Sockets.c
 * Close a socket
 */
void * vSocketClose( FreeRTOS_Socket_t * pxSocket );

/*
 * Send the event eEvent to the IP task event queue, using a block time of
 * zero.  Return pdPASS if the message was sent successfully, otherwise return
 * pdFALSE.
 */
BaseType_t xSendEventToIPTask( eIPEvent_t eEvent );

/*
 * The same as above, but a struct as a parameter, containing:
 *      eIPEvent_t eEventType;
 *      void *pvData;
 */
BaseType_t xSendEventStructToIPTask( const IPStackEvent_t * pxEvent,
                                     TickType_t uxTimeout );

/*
 * Returns a pointer to the original NetworkBuffer from a pointer to a UDP
 * payload buffer.
 */
NetworkBufferDescriptor_t * pxUDPPayloadBuffer_to_NetworkBuffer( const void * pvBuffer );

/* Get the size of the IP-header.
 * 'usFrameType' must be filled in if IPv6is to be recognised. */
size_t uxIPHeaderSizePacket( const NetworkBufferDescriptor_t * pxNetworkBuffer );
/*-----------------------------------------------------------*/

/* Get the size of the IP-header.
 * The socket is checked for its type: IPv4 or IPv6. */
size_t uxIPHeaderSizeSocket( const FreeRTOS_Socket_t * pxSocket );
/*-----------------------------------------------------------*/

/*
 * Internal: Sets a new state for a TCP socket and performs the necessary
 * actions like calling a OnConnected handler to notify the socket owner.
 */
#if ( ipconfigUSE_TCP == 1 )
    void vTCPStateChange( FreeRTOS_Socket_t * pxSocket,
                          enum eTCP_STATE eTCPState );
#endif /* ipconfigUSE_TCP */

/* Returns pdTRUE is this function is called from the IP-task */
BaseType_t xIsCallingFromIPTask( void );

#if ( ipconfigSUPPORT_SELECT_FUNCTION == 1 )

/** @brief Structure for event groups of the Socket Select functions */
    typedef struct xSOCKET_SET
    {
        /** @brief Event group for the socket select function.
         */
        EventGroupHandle_t xSelectGroup;
    } SocketSelect_t;

    extern void vSocketSelect( const SocketSelect_t * pxSocketSet );

/** @brief Define the data that must be passed for a 'eSocketSelectEvent'. */
    typedef struct xSocketSelectMessage
    {
        TaskHandle_t xTaskhandle;     /**< Task handle for use in the socket select functionality. */
        SocketSelect_t * pxSocketSet; /**< The event group for the socket select functionality. */
    } SocketSelectMessage_t;

#endif /* ipconfigSUPPORT_SELECT_FUNCTION */

/* Send the network-up event and start the ARP/ND timers. */
void vIPNetworkUpCalls( struct xNetworkEndPoint * pxEndPoint );

/* Mark whether all interfaces are up or at least one interface is down. */
void vSetAllNetworksUp( BaseType_t xIsAllNetworksUp );

/* *INDENT-OFF* */
#ifdef __cplusplus
    } /* extern "C" */
#endif
/* *INDENT-ON* */

#endif /* FREERTOS_IP_PRIVATE_H */
