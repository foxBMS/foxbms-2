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

#ifndef FREERTOS_SOCKETS_H
    #define FREERTOS_SOCKETS_H

/* Standard includes. */
    #include <string.h>

/* FreeRTOS includes. */
    #include "FreeRTOS.h"
    #include "task.h"

/* Application level configuration options. */
    #include "FreeRTOSIPConfig.h"
    #include "FreeRTOSIPConfigDefaults.h"

    #ifndef FREERTOS_IP_CONFIG_H
        #error FreeRTOSIPConfig.h has not been included yet
    #endif

    #include "FreeRTOS_IP_Common.h"

/* Event bit definitions are required by the select functions. */
    #include "event_groups.h"

    #ifdef __cplusplus
    extern "C" {
    #endif

    #ifndef INC_FREERTOS_H
        #error FreeRTOS.h must be included before FreeRTOS_Sockets.h.
    #endif

    #ifndef INC_TASK_H
        #ifndef TASK_H /* For compatibility with older FreeRTOS versions. */
            #error The FreeRTOS header file task.h must be included before FreeRTOS_Sockets.h.
        #endif
    #endif

/* Assigned to an Socket_t variable when the socket is not valid, probably
 * because it could not be created. */
    #define FREERTOS_INVALID_SOCKET          ( ( Socket_t ) ~0U )

/* API function error values.  As errno is supported, the FreeRTOS sockets
 * functions return error codes rather than just a pass or fail indication.
 *
 * Like in errno.h, the error codes are defined as positive numbers.
 * However, in case of an error, API 's will still negative values, e.g.
 * return -pdFREERTOS_ERRNO_EWOULDBLOCK;
 * in case an operation would block.
 *
 * The following defines are obsolete, please use -pdFREERTOS_ERRNO_Exxx. */
    #define FREERTOS_SOCKET_ERROR            ( -1 )
    #define FREERTOS_EWOULDBLOCK             ( -pdFREERTOS_ERRNO_EWOULDBLOCK )
    #define FREERTOS_EINVAL                  ( -pdFREERTOS_ERRNO_EINVAL )
    #define FREERTOS_EADDRNOTAVAIL           ( -pdFREERTOS_ERRNO_EADDRNOTAVAIL )
    #define FREERTOS_EADDRINUSE              ( -pdFREERTOS_ERRNO_EADDRINUSE )
    #define FREERTOS_ENOBUFS                 ( -pdFREERTOS_ERRNO_ENOBUFS )
    #define FREERTOS_ENOPROTOOPT             ( -pdFREERTOS_ERRNO_ENOPROTOOPT )
    #define FREERTOS_ECLOSED                 ( -pdFREERTOS_ERRNO_ENOTCONN )

/* Values for the parameters to FreeRTOS_socket(), inline with the Berkeley
 * standard.  See the documentation of FreeRTOS_socket() for more information. */
    #define FREERTOS_AF_INET                 ( 2 )
    #define FREERTOS_AF_INET6                ( 10 )
    #define FREERTOS_SOCK_DGRAM              ( 2 )
    #define FREERTOS_IPPROTO_UDP             ( 17 )
    #define FREERTOS_SOCK_STREAM             ( 1 )
    #define FREERTOS_IPPROTO_TCP             ( 6 )
    #define FREERTOS_SOCK_DEPENDENT_PROTO    ( 0 )

    #define FREERTOS_AF_INET4                FREERTOS_AF_INET
/* Values for xFlags parameter of Receive/Send functions. */
    #define FREERTOS_ZERO_COPY               ( 1 )  /* Can be used with recvfrom(), sendto() and recv(),
                                                     * Indicates that the zero copy interface is being used.
                                                     * See the documentation for FreeRTOS_sockets() for more information. */
    #define FREERTOS_MSG_OOB                 ( 2 )  /* Not used. */
    #define FREERTOS_MSG_PEEK                ( 4 )  /* Can be used with recvfrom() and recv(). */
    #define FREERTOS_MSG_DONTROUTE           ( 8 )  /* Not used. */
    #define FREERTOS_MSG_DONTWAIT            ( 16 ) /* Can be used with recvfrom(), sendto(), recv() and send(). */

/* Values that can be passed in the option name parameter of calls to
 * FreeRTOS_setsockopt(). */
    #define FREERTOS_SO_RCVTIMEO             ( 0 ) /* Used to set the receive time out. */
    #define FREERTOS_SO_SNDTIMEO             ( 1 ) /* Used to set the send time out. */
    #define FREERTOS_SO_UDPCKSUM_OUT         ( 2 ) /* Used to turn the use of the UDP checksum
                                                    * by a socket on or off.  This also doubles
                                                    * as part of an 8-bit bitwise socket option. */
    #if ( ipconfigSOCKET_HAS_USER_SEMAPHORE == 1 )
        #define FREERTOS_SO_SET_SEMAPHORE    ( 3 ) /* Used to set a user's semaphore. */
    #endif

    #if ( ipconfigUSE_TCP == 1 )
        #define FREERTOS_SO_SNDBUF    ( 4 ) /* Set the size of the send buffer (TCP only). */
        #define FREERTOS_SO_RCVBUF    ( 5 ) /* Set the size of the receive buffer (TCP only). */
    #endif

    #if ( ipconfigUSE_CALLBACKS == 1 )

/* Supply pointer to 'F_TCP_UDP_Handler_t' for pvOptionValue parameter in
 * FreeRTOS_setsockopt() */
        #define FREERTOS_SO_TCP_CONN_HANDLER    ( 6 )  /* Install a callback for (dis) connection events. */
        #define FREERTOS_SO_TCP_RECV_HANDLER    ( 7 )  /* Install a callback for receiving TCP data. */
        #define FREERTOS_SO_TCP_SENT_HANDLER    ( 8 )  /* Install a callback for sending TCP data. */
        #define FREERTOS_SO_UDP_RECV_HANDLER    ( 9 )  /* Install a callback for receiving UDP data. */
        #define FREERTOS_SO_UDP_SENT_HANDLER    ( 10 ) /* Install a callback for sending UDP data. */
    #endif

    #if ( ipconfigUSE_TCP == 1 )
        #define FREERTOS_SO_REUSE_LISTEN_SOCKET    ( 11 ) /* When a listening socket gets connected, do not create a new one but re-use it. */
        #define FREERTOS_SO_CLOSE_AFTER_SEND       ( 12 ) /* As soon as the last byte has been transmitted, finalise the connection. */
        #define FREERTOS_SO_WIN_PROPERTIES         ( 13 ) /* Set all buffer and window properties in one call, parameter is pointer to WinProperties_t. */
        #define FREERTOS_SO_SET_FULL_SIZE          ( 14 ) /* Refuse to send packets smaller than MSS. */
        #define FREERTOS_SO_STOP_RX                ( 15 ) /* Temporarily hold up reception, used by streaming client. */
    #endif

    #if ( ipconfigUDP_MAX_RX_PACKETS > 0 )
        #define FREERTOS_SO_UDP_MAX_RX_PACKETS    ( 16 ) /* This option helps to limit the maximum number of packets a UDP socket will buffer. */
    #endif

    #if ( ipconfigSOCKET_HAS_USER_WAKE_CALLBACK == 1 )
        #define FREERTOS_SO_WAKEUP_CALLBACK    ( 17 )
    #endif

    #if ( ipconfigUSE_TCP == 1 )
        #define FREERTOS_SO_SET_LOW_HIGH_WATER            ( 18 )
    #endif
    #define FREERTOS_INADDR_ANY                           ( 0U ) /* The 0.0.0.0 IPv4 address. */

    #if ( 0 )                                                    /* Not Used */
        #define FREERTOS_NOT_LAST_IN_FRAGMENTED_PACKET    ( 0x80 )
        #define FREERTOS_FRAGMENTED_PACKET                ( 0x40 )
    #endif

    #if ( ipconfigUSE_TCP == 1 )
/* Values for 'xHow' flag of FreeRTOS_shutdown(), currently ignored. */
        #define FREERTOS_SHUT_RD      ( 0 )
        #define FREERTOS_SHUT_WR      ( 1 )
        #define FREERTOS_SHUT_RDWR    ( 2 )
    #endif

/* For compatibility with the expected Berkeley sockets naming. */
    #define socklen_t    uint32_t

/**
 * For this limited implementation, only two members are required in the
 * Berkeley style sockaddr structure.
 */
    struct freertos_sockaddr
    {
        uint8_t sin_len;          /**< length of this structure. */
        uint8_t sin_family;       /**< FREERTOS_AF_INET. */
        uint16_t sin_port;        /**< The port. */
        uint32_t sin_flowinfo;    /**< IPv6 flow information, not used in this library. */
        IP_Address_t sin_address; /**< The IPv4/IPv6 address. */
    };

    #if ( ipconfigIPv4_BACKWARD_COMPATIBLE == 1 )

        #define sin_addr    sin_address.ulIP_IPv4

    #endif

/** Introduce a short name to make casting easier. */
    typedef struct freertos_sockaddr   xFreertosSocAddr;

/* The socket type itself. */
    struct xSOCKET;
    typedef struct xSOCKET             * Socket_t;
    typedef struct xSOCKET const       * ConstSocket_t;

    extern BaseType_t xSocketValid( const ConstSocket_t xSocket );

/**
 * FULL, UP-TO-DATE AND MAINTAINED REFERENCE DOCUMENTATION FOR ALL THESE
 * FUNCTIONS IS AVAILABLE ON THE FOLLOWING URL:
 * https://freertos.org/Documentation/03-Libraries/02-FreeRTOS-plus/02-FreeRTOS-plus-TCP/09-API-reference/01-FreeRTOS-plus-TCP-APIs
 */

/* Common Socket Attributes. */

/* Create a TCP or UDP socket. */
    Socket_t FreeRTOS_socket( BaseType_t xDomain,
                              BaseType_t xType,
                              BaseType_t xProtocol );

/* Binds a socket to a local port number. */
    BaseType_t FreeRTOS_bind( Socket_t xSocket,
                              struct freertos_sockaddr const * pxAddress,
                              socklen_t xAddressLength );

/* Sets a socket option. */
    BaseType_t FreeRTOS_setsockopt( Socket_t xSocket,
                                    int32_t lLevel,
                                    int32_t lOptionName,
                                    const void * pvOptionValue,
                                    size_t uxOptionLength );

/* Close a socket. */
    BaseType_t FreeRTOS_closesocket( Socket_t xSocket );

    #if ( ipconfigSUPPORT_SIGNALS != 0 )
/* Send a signal to the task which is waiting for a given socket. */
        BaseType_t FreeRTOS_SignalSocket( Socket_t xSocket );

/* Send a signal to the task which reads from this socket (FromISR version). */
        BaseType_t FreeRTOS_SignalSocketFromISR( Socket_t xSocket,
                                                 BaseType_t * pxHigherPriorityTaskWoken );
    #endif

/* This option adds the possibility to have a user-ID attached to a socket.
 * The type of this ID is a void *.  Both UDP and TCP sockets have
 * this ID. It has a default value of NULL.
 */
    BaseType_t xSocketSetSocketID( const Socket_t xSocket,
                                   void * pvSocketID );

    void * pvSocketGetSocketID( const ConstSocket_t xSocket );

/* Get the type of IP: either 'ipTYPE_IPv4' or 'ipTYPE_IPv6'. */
    BaseType_t FreeRTOS_GetIPType( ConstSocket_t xSocket );

/* End Common Socket Attributes */


/* UDP Socket Attributes. */

/* Send data to a UDP socket. */
    int32_t FreeRTOS_sendto( Socket_t xSocket,
                             const void * pvBuffer,
                             size_t uxTotalDataLength,
                             BaseType_t xFlags,
                             const struct freertos_sockaddr * pxDestinationAddress,
                             socklen_t xDestinationAddressLength );

/* Receive data from a UDP socket */
    int32_t FreeRTOS_recvfrom( const ConstSocket_t xSocket,
                               void * pvBuffer,
                               size_t uxBufferLength,
                               BaseType_t xFlags,
                               struct freertos_sockaddr * pxSourceAddress,
                               socklen_t * pxSourceAddressLength );


/* Function to get the local address and IP port. */
    size_t FreeRTOS_GetLocalAddress( ConstSocket_t xSocket,
                                     struct freertos_sockaddr * pxAddress );

    #if ( ipconfigETHERNET_DRIVER_FILTERS_PACKETS == 1 )
/* Returns true if an UDP socket exists bound to mentioned port number. */
        BaseType_t xPortHasUDPSocket( uint16_t usPortNr );
    #endif

/* End UDP Socket Attributes */


    #if ( ipconfigUSE_TCP == 1 )

/* TCP Socket Attributes. */

/**
 * Structure to hold the properties of Tx/Rx buffers and windows.
 */
        typedef struct xWIN_PROPS
        {
            /* Properties of the Tx buffer and Tx window. */
            int32_t lTxBufSize; /**< Unit: bytes. */
            int32_t lTxWinSize; /**< Unit: MSS. */

            /* Properties of the Rx buffer and Rx window. */
            int32_t lRxBufSize; /**< Unit: bytes. */
            int32_t lRxWinSize; /**< Unit: MSS. */
        } WinProperties_t;

/**
 * Structure to pass for the 'FREERTOS_SO_SET_LOW_HIGH_WATER' option.
 */
        typedef struct xLOW_HIGH_WATER
        {
            size_t uxLittleSpace; /**< Send a STOP when buffer space drops below X bytes */
            size_t uxEnoughSpace; /**< Send a GO when buffer space grows above X bytes */
        } LowHighWater_t;

/* Connect a TCP socket to a remote socket. */
        BaseType_t FreeRTOS_connect( Socket_t xClientSocket,
                                     const struct freertos_sockaddr * pxAddress,
                                     socklen_t xAddressLength );

/* Places a TCP socket into a state where it is listening for and can accept
 * incoming connection requests from remote sockets. */
        BaseType_t FreeRTOS_listen( Socket_t xSocket,
                                    BaseType_t xBacklog );

/* Accept a connection on a TCP socket. */
        Socket_t FreeRTOS_accept( Socket_t xServerSocket,
                                  struct freertos_sockaddr * pxAddress,
                                  socklen_t * pxAddressLength );

/* Send data to a TCP socket. */
        BaseType_t FreeRTOS_send( Socket_t xSocket,
                                  const void * pvBuffer,
                                  size_t uxDataLength,
                                  BaseType_t xFlags );

/* Receive data from a TCP socket */
        BaseType_t FreeRTOS_recv( Socket_t xSocket,
                                  void * pvBuffer,
                                  size_t uxBufferLength,
                                  BaseType_t xFlags );

/* Disable reads and writes on a connected TCP socket. */
        BaseType_t FreeRTOS_shutdown( Socket_t xSocket,
                                      BaseType_t xHow );

/* Release a TCP payload buffer that was obtained by
 * calling FreeRTOS_recv() with the FREERTOS_ZERO_COPY flag,
 * and a pointer to a void pointer. */
        BaseType_t FreeRTOS_ReleaseTCPPayloadBuffer( Socket_t xSocket,
                                                     void const * pvBuffer,
                                                     BaseType_t xByteCount );

/* Returns the number of bytes available in the Rx buffer. */
        BaseType_t FreeRTOS_rx_size( ConstSocket_t xSocket );

        #define FreeRTOS_recvcount( xSocket )    FreeRTOS_rx_size( xSocket )

/* Returns the free space in the Tx buffer. */
        BaseType_t FreeRTOS_tx_space( ConstSocket_t xSocket );

        #define FreeRTOS_outstanding( xSocket )    FreeRTOS_tx_size( xSocket )

/* Returns the number of bytes stored in the Tx buffer. */
        BaseType_t FreeRTOS_tx_size( ConstSocket_t xSocket );

/* Returns pdTRUE if TCP socket is connected. */
        BaseType_t FreeRTOS_issocketconnected( ConstSocket_t xSocket );

/* Return the remote address and IP port of a connected TCP Socket. */
        BaseType_t FreeRTOS_GetRemoteAddress( ConstSocket_t xSocket,
                                              struct freertos_sockaddr * pxAddress );

/* Returns the number of bytes that may be added to txStream. */
        BaseType_t FreeRTOS_maywrite( ConstSocket_t xSocket );

/* Returns the actual size of MSS being used. */
        BaseType_t FreeRTOS_mss( ConstSocket_t xSocket );

/* For internal use only: return the connection status. */
        BaseType_t FreeRTOS_connstatus( ConstSocket_t xSocket );

/* For advanced applications only:
 * Get a direct pointer to the beginning of the circular transmit buffer.
 * In case the buffer was not yet created, it will be created in
 * this call.
 */
        uint8_t * FreeRTOS_get_tx_base( Socket_t xSocket );

/* For advanced applications only:
 * Get a direct pointer to the circular transmit buffer.
 * '*pxLength' will contain the number of bytes that may be written. */
        uint8_t * FreeRTOS_get_tx_head( Socket_t xSocket,
                                        BaseType_t * pxLength );

/* For the web server: borrow the circular Rx buffer for inspection
 * HTML driver wants to see if a sequence of 13/10/13/10 is available. */
        const struct xSTREAM_BUFFER * FreeRTOS_get_rx_buf( ConstSocket_t xSocket );

        void FreeRTOS_netstat( void );

/* End TCP Socket Attributes. */

    #endif /* ( ipconfigUSE_TCP == 1 ) */

    #if ( ipconfigUSE_CALLBACKS == 1 )

/*
 * Callback handlers for a socket
 * User-provided functions will be called for each sockopt callback defined
 * For example:
 * static void xOnTCPConnect( Socket_t xSocket, BaseType_t ulConnected ) {}
 * static BaseType_t xOnTCPReceive( Socket_t xSocket, void * pData, size_t uxLength )
 * {
 *     // handle the message
 *     return pdFALSE; // Not Used
 * }
 * static void xOnTCPSent( Socket_t xSocket, size_t xLength ) {}
 * static BaseType_t xOnUDPReceive( Socket_t xSocket, void * pData, size_t xLength, const struct freertos_sockaddr * pxFrom, const struct freertos_sockaddr * pxDest )
 * {
 *     // handle the message
 *     return pdTRUE; // message processing is finished, don't store
 * }
 * static void xOnUDPSent( Socket_t xSocket, size_t xLength ) {}
 * F_TCP_UDP_Handler_t xHand = { xOnTCPConnect, xOnTCPReceive, xOnTCPSent, xOnUDPReceive, xOnUDPSent };
 * FreeRTOS_setsockopt( sock, 0, FREERTOS_SO_TCP_CONN_HANDLER, ( void * ) &xHand, 0 );
 */

/* Connected callback handler for a TCP Socket. */
        typedef void (* FOnConnected_t )( Socket_t xSocket,
                                          BaseType_t ulConnected );

/* Received callback handler for a TCP Socket.
 * Return value is not currently used. */
        typedef BaseType_t (* FOnTCPReceive_t )( Socket_t xSocket,
                                                 void * pData,
                                                 size_t xLength );

/* Sent callback handler for a TCP Socket. */
        typedef void (* FOnTCPSent_t )( Socket_t xSocket,
                                        size_t xLength );

/* Received callback handler for a UDP Socket.
 * If a positive number is returned, the messages will not be stored in
 * xWaitingPacketsList for later processing by recvfrom(). */
        typedef BaseType_t (* FOnUDPReceive_t ) ( Socket_t xSocket,
                                                  void * pData,
                                                  size_t xLength,
                                                  const struct freertos_sockaddr * pxFrom,
                                                  const struct freertos_sockaddr * pxDest );

/* Sent callback handler for a UDP Socket */
        typedef void (* FOnUDPSent_t )( Socket_t xSocket,
                                        size_t xLength );

/* The following values are used in the lOptionName parameter of setsockopt()
 * to set the callback handlers options. */
        typedef struct xTCP_UDP_HANDLER
        {
            FOnConnected_t pxOnTCPConnected; /* FREERTOS_SO_TCP_CONN_HANDLER */
            FOnTCPReceive_t pxOnTCPReceive;  /* FREERTOS_SO_TCP_RECV_HANDLER */
            FOnTCPSent_t pxOnTCPSent;        /* FREERTOS_SO_TCP_SENT_HANDLER */
            FOnUDPReceive_t pxOnUDPReceive;  /* FREERTOS_SO_UDP_RECV_HANDLER */
            FOnUDPSent_t pxOnUDPSent;        /* FREERTOS_SO_UDP_SENT_HANDLER */
        } F_TCP_UDP_Handler_t;

    #endif /* ( ipconfigUSE_CALLBACKS == 1 ) */

/* Conversion Functions */

/* Converts an IP address expressed as a 32-bit number in network byte order
 * to a string in decimal dot notation. */
    extern const char * FreeRTOS_inet_ntoa( uint32_t ulIPAddress,
                                            char * pcBuffer );

    #if ( ipconfigBYTE_ORDER == pdFREERTOS_LITTLE_ENDIAN )

/* Converts an IP address expressed as four separate numeric octets into an
 * IP address expressed as a 32-bit number in network byte order */
        #define FreeRTOS_inet_addr_quick( ucOctet0, ucOctet1, ucOctet2, ucOctet3 ) \
    ( ( ( ( uint32_t ) ( ucOctet3 ) ) << 24 ) |                                    \
      ( ( ( uint32_t ) ( ucOctet2 ) ) << 16 ) |                                    \
      ( ( ( uint32_t ) ( ucOctet1 ) ) << 8 ) |                                     \
      ( ( uint32_t ) ( ucOctet0 ) ) )

    #else /* ( ipconfigBYTE_ORDER == pdFREERTOS_BIG_ENDIAN ) */

        #define FreeRTOS_inet_addr_quick( ucOctet0, ucOctet1, ucOctet2, ucOctet3 ) \
    ( ( ( ( uint32_t ) ( ucOctet0 ) ) << 24 ) |                                    \
      ( ( ( uint32_t ) ( ucOctet1 ) ) << 16 ) |                                    \
      ( ( ( uint32_t ) ( ucOctet2 ) ) << 8 ) |                                     \
      ( ( uint32_t ) ( ucOctet3 ) ) )

    #endif /* ( ipconfigBYTE_ORDER == pdFREERTOS_LITTLE_ENDIAN ) */

/* Convert a null-terminated string in dot-decimal-notation (d.d.d.d)
 * to a 32-bit unsigned integer. */
    uint32_t FreeRTOS_inet_addr( const char * pcIPAddress );

    BaseType_t FreeRTOS_inet_pton( BaseType_t xAddressFamily,
                                   const char * pcSource,
                                   void * pvDestination );

    const char * FreeRTOS_inet_ntop( BaseType_t xAddressFamily,
                                     const void * pvSource,
                                     char * pcDestination,
                                     socklen_t uxSize );

/** @brief This function converts a human readable string, representing an 48-bit MAC address,
 * into a 6-byte address. Valid inputs are e.g. "62:48:5:83:A0:b2" and "0-12-34-fe-dc-ba". */
    BaseType_t FreeRTOS_EUI48_pton( const char * pcSource,
                                    uint8_t * pucTarget );

/** @brief This function converts a 48-bit MAC address to a human readable string. */
    void FreeRTOS_EUI48_ntop( const uint8_t * pucSource,
                              char * pcTarget,
                              char cTen,
                              char cSeparator );

/* End Conversion Functions */

    #if ( ipconfigSUPPORT_SELECT_FUNCTION == 1 )

/* The SocketSet_t type is the equivalent to the fd_set type used by the
 * Berkeley API. */
        struct xSOCKET_SET;
        typedef struct xSOCKET_SET         * SocketSet_t;
        typedef struct xSOCKET_SET const   * ConstSocketSet_t;

/* Create a socket set for use with the FreeRTOS_select() function */
        SocketSet_t FreeRTOS_CreateSocketSet( void );

        void FreeRTOS_DeleteSocketSet( SocketSet_t xSocketSet );

/* Block on a "socket set" until an event of interest occurs on a
 * socket within the set. */
        BaseType_t FreeRTOS_select( SocketSet_t xSocketSet,
                                    TickType_t xBlockTimeTicks );

/* For FD_SET and FD_CLR, a combination of the following bits can be used: */
        typedef enum eSELECT_EVENT
        {
            eSELECT_READ = 0x0001,
            eSELECT_WRITE = 0x0002,
            eSELECT_EXCEPT = 0x0004,
            eSELECT_INTR = 0x0008,
            eSELECT_ALL = 0x000F,
            /* Reserved for internal use: */
            eSELECT_CALL_IP = 0x0010
                              /* end */
        } eSelectEvent_t;

/* Add a socket to a socket set, and set the event bits of interest
 * for the added socket. */
        void FreeRTOS_FD_SET( Socket_t xSocket,
                              SocketSet_t xSocketSet,
                              EventBits_t xBitsToSet );

/* Clear a set event bit of interest for a socket of the socket set.
 * If all the event bits are clear then the socket will be removed
 * from the socket set. */
        void FreeRTOS_FD_CLR( Socket_t xSocket,
                              SocketSet_t xSocketSet,
                              EventBits_t xBitsToClear );

/* Check if a socket in a socket set has an event bit set. */
        EventBits_t FreeRTOS_FD_ISSET( const ConstSocket_t xSocket,
                                       const ConstSocketSet_t xSocketSet );

    #endif /* ( ipconfigSUPPORT_SELECT_FUNCTION == 1 ) */


    #if ipconfigUSE_IPv4
        /* Translate from dot-decimal notation (example 192.168.1.1) to a 32-bit number. */
        BaseType_t FreeRTOS_inet_pton4( const char * pcSource,
                                        void * pvDestination );

/* Translate 32-bit IPv4 address representation dot-decimal notation. */
        const char * FreeRTOS_inet_ntop4( const void * pvSource,
                                          char * pcDestination,
                                          socklen_t uxSize );
    #endif

    #if ipconfigUSE_IPv6
        /* Translate hexadecimal IPv6 address to 16 bytes binary format */
        BaseType_t FreeRTOS_inet_pton6( const char * pcSource,
                                        void * pvDestination );

/*
 * Convert a string like 'fe80::8d11:cd9b:8b66:4a80'
 * to a 16-byte IPv6 address
 */
        const char * FreeRTOS_inet_ntop6( const void * pvSource,
                                          char * pcDestination,
                                          socklen_t uxSize );

    #endif

    #ifdef __cplusplus
}     /* extern "C" */
    #endif

#endif /* FREERTOS_SOCKETS_H */
