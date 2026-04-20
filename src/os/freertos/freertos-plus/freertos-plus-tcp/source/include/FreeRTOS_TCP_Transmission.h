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

#ifndef FREERTOS_TCP_TRANSMISSION_H
#define FREERTOS_TCP_TRANSMISSION_H

/* *INDENT-OFF* */
#ifdef __cplusplus
    extern "C" {
#endif
/* *INDENT-ON* */

/*
 * Either sends a SYN or calls prvTCPSendRepeated (for regular messages).
 */
int32_t prvTCPSendPacket( FreeRTOS_Socket_t * pxSocket );

/*
 * Try to send a series of messages.
 */
int32_t prvTCPSendRepeated( FreeRTOS_Socket_t * pxSocket,
                            NetworkBufferDescriptor_t ** ppxNetworkBuffer );

/*
 * Return or send a packet to the other party.
 */
void prvTCPReturnPacket( FreeRTOS_Socket_t * pxSocket,
                         NetworkBufferDescriptor_t * pxDescriptor,
                         uint32_t ulLen,
                         BaseType_t xReleaseAfterSend );

/**
 * Called by prvTCPReturnPacket(), this function will set the the window
 * size on this side: 'xTCPHeader.usWindow'.
 */
void prvTCPReturn_CheckTCPWindow( FreeRTOS_Socket_t * pxSocket,
                                  const NetworkBufferDescriptor_t * pxNetworkBuffer,
                                  size_t uxIPHeaderSize );

/*
 * Called by prvTCPReturnPacket(), this function sets the sequence and ack numbers
 * in the TCP-header.
 */
void prvTCPReturn_SetSequenceNumber( FreeRTOS_Socket_t * pxSocket,
                                     const NetworkBufferDescriptor_t * pxNetworkBuffer,
                                     size_t uxIPHeaderSize,
                                     uint32_t ulLen );

/*
 * Return or send a packet to the other party.
 */
void prvTCPReturnPacket_IPV4( FreeRTOS_Socket_t * pxSocket,
                              NetworkBufferDescriptor_t * pxDescriptor,
                              uint32_t ulLen,
                              BaseType_t xReleaseAfterSend );

/*
 * Return or send a packet to the other party.
 */
void prvTCPReturnPacket_IPV6( FreeRTOS_Socket_t * pxSocket,
                              NetworkBufferDescriptor_t * pxDescriptor,
                              uint32_t ulLen,
                              BaseType_t xReleaseAfterSend );

void prvTCPReturn_SetEndPoint( const FreeRTOS_Socket_t * pxSocket,
                               NetworkBufferDescriptor_t * pxNetworkBuffer,
                               size_t uxIPHeaderSize );

/*
 * Let ARP look-up the MAC-address of the peer and initialise the first SYN
 * packet.
 */
BaseType_t prvTCPPrepareConnect_IPV4( FreeRTOS_Socket_t * pxSocket );

/*
 * Let ND look-up the MAC-address of the peer and initialise the first SYN
 * packet.
 */
BaseType_t prvTCPPrepareConnect_IPV6( FreeRTOS_Socket_t * pxSocket );

/*
 * Initialise the data structures which keep track of the TCP windowing system.
 */
BaseType_t prvTCPCreateWindow( FreeRTOS_Socket_t * pxSocket );

/*
 * Set the initial properties in the options fields, like the preferred
 * value of MSS and whether SACK allowed.  Will be transmitted in the state
 * 'eCONNECT_SYN'.
 */
UBaseType_t prvSetSynAckOptions( FreeRTOS_Socket_t * pxSocket,
                                 TCPHeader_t * pxTCPHeader );

/*
 * Prepare an outgoing message, if anything has to be sent.
 */
int32_t prvTCPPrepareSend( FreeRTOS_Socket_t * pxSocket,
                           NetworkBufferDescriptor_t ** ppxNetworkBuffer,
                           UBaseType_t uxOptionsLength );

/*
 * The API FreeRTOS_send() adds data to the TX stream.  Add
 * this data to the windowing system to it can be transmitted.
 */
void prvTCPAddTxData( FreeRTOS_Socket_t * pxSocket );

/*
 * Set the TCP options (if any) for the outgoing packet.
 */
UBaseType_t prvSetOptions( FreeRTOS_Socket_t * pxSocket,
                           const NetworkBufferDescriptor_t * pxNetworkBuffer );

/*
 * Called from prvTCPHandleState().  There is data to be sent.
 * If ipconfigUSE_TCP_WIN is defined, and if only an ACK must be sent, it will
 * be checked if it would better be postponed for efficiency.
 */
BaseType_t prvSendData( FreeRTOS_Socket_t * pxSocket,
                        NetworkBufferDescriptor_t ** ppxNetworkBuffer,
                        uint32_t ulReceiveLength,
                        BaseType_t xByteCount );

/*
 * A "challenge ACK" is as per https://tools.ietf.org/html/rfc5961#section-3.2,
 * case #3. In summary, an RST was received with a sequence number that is
 * unexpected but still within the window.
 */
BaseType_t prvTCPSendChallengeAck( NetworkBufferDescriptor_t * pxNetworkBuffer );

/*
 * Reply to a peer with the RST flag on, in case a packet can not be handled.
 */
BaseType_t prvTCPSendReset( NetworkBufferDescriptor_t * pxNetworkBuffer );

/*
 *  Check if the size of a network buffer is big enough to hold the outgoing message.
 *  Allocate a new bigger network buffer when necessary.
 */
NetworkBufferDescriptor_t * prvTCPBufferResize( const FreeRTOS_Socket_t * pxSocket,
                                                NetworkBufferDescriptor_t * pxNetworkBuffer,
                                                int32_t lDataLen,
                                                UBaseType_t uxOptionsLength );
/* *INDENT-OFF* */
#ifdef __cplusplus
    } /* extern "C" */
#endif
/* *INDENT-ON* */

#endif /* FREERTOS_TCP_TRANSMISSION_H */
