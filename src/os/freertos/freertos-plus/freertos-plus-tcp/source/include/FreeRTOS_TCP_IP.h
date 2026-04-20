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

#ifndef FREERTOS_TCP_IP_H
#define FREERTOS_TCP_IP_H

/* *INDENT-OFF* */
#ifdef __cplusplus
    extern "C" {
#endif
/* *INDENT-ON* */

/**
 * @brief Process the received TCP packet.
 *
 */
BaseType_t xProcessReceivedTCPPacket( NetworkBufferDescriptor_t * pxDescriptor );

/**
 * @brief Process the received TCP packet.
 */
BaseType_t xProcessReceivedTCPPacket_IPV4( NetworkBufferDescriptor_t * pxDescriptor );

/**
 * @brief Process the received TCP packet.
 */
BaseType_t xProcessReceivedTCPPacket_IPV6( NetworkBufferDescriptor_t * pxDescriptor );

typedef enum eTCP_STATE
{
    /* Comments about the TCP states are borrowed from the very useful
     * Wiki page:
     * http://en.wikipedia.org/wiki/Transmission_Control_Protocol */
    eCLOSED = 0U,  /* 0 (server + client) no connection state at all. */
    eTCP_LISTEN,   /* 1 (server) waiting for a connection request
                    * from any remote TCP and port. */
    eCONNECT_SYN,  /* 2 (client) internal state: socket wants to send
                    * a connect */
    eSYN_FIRST,    /* 3 (server) Just created, must ACK the SYN request. */
    eSYN_RECEIVED, /* 4 (server) waiting for a confirming connection request
                    * acknowledgement after having both received and sent a connection request. */
    eESTABLISHED,  /* 5 (server + client) an open connection, data received can be
                    * delivered to the user. The normal state for the data transfer phase of the connection. */
    eFIN_WAIT_1,   /* 6 (server + client) waiting for a connection termination request from the remote TCP,
                    * or an acknowledgement of the connection termination request previously sent. */
    eFIN_WAIT_2,   /* 7 (server + client) waiting for a connection termination request from the remote TCP. */
    eCLOSE_WAIT,   /* 8 (server + client) waiting for a connection termination request from the local user. */
    eCLOSING,      /* 9 (server + client) waiting for a connection termination request acknowledgement from the remote TCP. */
    eLAST_ACK,     /*10 (server + client) waiting for an acknowledgement of the connection termination request
                    * previously sent to the remote TCP
                    * (which includes an acknowledgement of its connection termination request). */
    eTIME_WAIT     /*11 (either server or client) waiting for enough time to pass to be sure the remote TCP received the
                    * acknowledgement of its connection termination request. [According to RFC 793 a connection can
                    * stay in TIME-WAIT for a maximum of four minutes known as a MSL (maximum segment lifetime).] */
} eIPTCPState_t;

/*
 * The meaning of the TCP flags:
 */
#define tcpTCP_FLAG_FIN     ( ( uint8_t ) 0x01U )                           /**< No more data from sender. */
#define tcpTCP_FLAG_SYN     ( ( uint8_t ) 0x02U )                           /**< Synchronize sequence numbers. */
#define tcpTCP_FLAG_RST     ( ( uint8_t ) 0x04U )                           /**< Reset the connection. */
#define tcpTCP_FLAG_PSH     ( ( uint8_t ) 0x08U )                           /**< Push function: please push buffered data to the recv application. */
#define tcpTCP_FLAG_ACK     ( ( uint8_t ) 0x10U )                           /**< Acknowledgment field is significant. */
#define tcpTCP_FLAG_URG     ( ( uint8_t ) 0x20U )                           /**< Urgent pointer field is significant. */
#define tcpTCP_FLAG_ECN     ( ( uint8_t ) 0x40U )                           /**< ECN-Echo. */
#define tcpTCP_FLAG_CWR     ( ( uint8_t ) 0x80U )                           /**< Congestion Window Reduced. */

#define tcpTCP_FLAG_CTRL    ( ( uint8_t ) 0x1FU )                           /**< A mask to filter all protocol flags. */


/*
 * A few values of the TCP options:
 */
#define tcpTCP_OPT_END                    0U             /**< End of TCP options list. */
#define tcpTCP_OPT_NOOP                   1U             /**< "No-operation" TCP option. */
#define tcpTCP_OPT_MSS                    2U             /**< Maximum segment size TCP option. */
#define tcpTCP_OPT_WSOPT                  3U             /**< TCP Window Scale Option (3-byte long). */
#define tcpTCP_OPT_SACK_P                 4U             /**< Advertise that SACK is permitted. */
#define tcpTCP_OPT_SACK_A                 5U             /**< SACK option with first/last. */
#define tcpTCP_OPT_TIMESTAMP              8U             /**< Time-stamp option. */


#define tcpTCP_OPT_MSS_LEN                4U             /**< Length of TCP MSS option. */
#define tcpTCP_OPT_WSOPT_LEN              3U             /**< Length of TCP WSOPT option. */
#define tcpTCP_OPT_WSOPT_MAXIMUM_VALUE    ( 14U )        /**< Maximum value of TCP WSOPT option. */

#define tcpTCP_OPT_TIMESTAMP_LEN          10             /**< fixed length of the time-stamp option. */

/** @brief
 * Minimum segment length as outlined by RFC 791 section 3.1.
 * Minimum segment length ( 536 ) = Minimum MTU ( 576 ) - IP Header ( 20 ) - TCP Header ( 20 ).
 */
#define tcpMINIMUM_SEGMENT_LENGTH         536U

/** @brief
 * The macro tcpNOW_CONNECTED() is use to determine if the connection makes a
 * transition from connected to non-connected and vice versa.
 * tcpNOW_CONNECTED() returns true when the status has one of these values:
 * eESTABLISHED, eFIN_WAIT_1, eFIN_WAIT_2, eCLOSING, eLAST_ACK, eTIME_WAIT
 * Technically the connection status is closed earlier, but the library wants
 * to prevent that the socket will be deleted before the last ACK has been
 * and thus causing a 'RST' packet on either side.
 */
#define tcpNOW_CONNECTED( status ) \
    ( ( ( ( status ) >= ( BaseType_t ) eESTABLISHED ) && ( ( status ) != ( BaseType_t ) eCLOSE_WAIT ) ) ? 1 : 0 )

/** @brief
 * The highest 4 bits in the TCP offset byte indicate the total length of the
 * TCP header, divided by 4.
 */
#define tcpVALID_BITS_IN_TCP_OFFSET_BYTE    ( 0xF0U )

/*
 * Acknowledgements to TCP data packets may be delayed as long as more is being expected.
 * A normal delay would be 200ms. Here a much shorter delay of 20 ms is being used to
 * gain performance.
 */
#define tcpDELAYED_ACK_SHORT_DELAY_MS       ( 2 )           /**< Should not become smaller than 1. */
#define tcpDELAYED_ACK_LONGER_DELAY_MS      ( 20 )          /**< Longer delay for ACK. */


/** @brief
 * The MSS (Maximum Segment Size) will be taken as large as possible. However, packets with
 * an MSS of 1460 bytes won't be transported through the internet.  The MSS will be reduced
 * to 1400 bytes.
 */
#define tcpREDUCED_MSS_THROUGH_INTERNET    ( 1400 )

/** @brief
 * When there are no TCP options, the TCP offset equals 20 bytes, which is stored as
 * the number 5 (words) in the higher nibble of the TCP-offset byte.
 */
#define tcpTCP_OFFSET_LENGTH_BITS          ( 0xf0U )
#define tcpTCP_OFFSET_STANDARD_LENGTH      ( 0x50U )          /**< Standard TCP packet offset. */


/** @brief
 * Each TCP socket is checked regularly to see if it can send data packets.
 * By default, the maximum number of packets sent during one check is limited to 8.
 * This amount may be further limited by setting the socket's TX window size.
 */
#if ( !defined( SEND_REPEATED_COUNT ) )
    #define SEND_REPEATED_COUNT    ( 8 )
#endif /* !defined( SEND_REPEATED_COUNT ) */

/** @brief
 * Define a maximum period of time (ms) to leave a TCP-socket unattended.
 * When a TCP timer expires, retries and keep-alive messages will be checked.
 */
#ifndef tcpMAXIMUM_TCP_WAKEUP_TIME_MS
    #define tcpMAXIMUM_TCP_WAKEUP_TIME_MS    20000U
#endif

struct xSOCKET;

/*
 * For anti-hang protection and TCP keep-alive messages.  Called in two places:
 * after receiving a packet and after a state change.  The socket's alive timer
 * may be reset.
 */
void prvTCPTouchSocket( struct xSOCKET * pxSocket );

/*
 * Calculate when this socket needs to be checked to do (re-)transmissions.
 */
TickType_t prvTCPNextTimeout( struct xSOCKET * pxSocket );


/* *INDENT-OFF* */
#ifdef __cplusplus
    } /* extern "C" */
#endif
/* *INDENT-ON* */

#endif /* FREERTOS_TCP_IP_H */
