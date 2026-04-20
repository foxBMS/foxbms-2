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

#ifndef FREERTOS_TCP_STATE_HANDLING_H
#define FREERTOS_TCP_STATE_HANDLING_H

#include "FreeRTOS_TCP_IP.h"

/* *INDENT-OFF* */
#ifdef __cplusplus
    extern "C" {
#endif
/* *INDENT-ON* */

/*
 * Returns true if the socket must be checked.  Non-active sockets are waiting
 * for user action, either connect() or close().
 */
BaseType_t prvTCPSocketIsActive( eIPTCPState_t eStatus );

/*
 * prvTCPStatusAgeCheck() will see if the socket has been in a non-connected
 * state for too long.  If so, the socket will be closed, and -1 will be
 * returned.
 */
#if ( ipconfigTCP_HANG_PROTECTION == 1 )
    BaseType_t prvTCPStatusAgeCheck( FreeRTOS_Socket_t * pxSocket );
#endif

/*
 * The heart of all: check incoming packet for valid data and acks and do what
 * is necessary in each state.
 */
BaseType_t prvTCPHandleState( FreeRTOS_Socket_t * pxSocket,
                              NetworkBufferDescriptor_t ** ppxNetworkBuffer );

/*
 * Return either a newly created socket, or the current socket in a connected
 * state (depends on the 'bReuseSocket' flag).
 */
FreeRTOS_Socket_t * prvHandleListen( FreeRTOS_Socket_t * pxSocket,
                                     NetworkBufferDescriptor_t * pxNetworkBuffer );

/*
 * Return either a newly created socket, or the current socket in a connected
 * state (depends on the 'bReuseSocket' flag).
 */
FreeRTOS_Socket_t * prvHandleListen_IPV4( FreeRTOS_Socket_t * pxSocket,
                                          NetworkBufferDescriptor_t * pxNetworkBuffer );

/*
 * Return either a newly created socket, or the current socket in a connected
 * state (depends on the 'bReuseSocket' flag).
 */
FreeRTOS_Socket_t * prvHandleListen_IPV6( FreeRTOS_Socket_t * pxSocket,
                                          NetworkBufferDescriptor_t * pxNetworkBuffer );

/*
 * Common code for sending a TCP protocol control packet (i.e. no options, no
 * payload, just flags).
 */
BaseType_t prvTCPSendSpecialPacketHelper( NetworkBufferDescriptor_t * pxNetworkBuffer,
                                          uint8_t ucTCPFlags );

/*
 * Common code for sending a TCP protocol control packet (i.e. no options, no
 * payload, just flags).
 */
BaseType_t prvTCPSendSpecialPktHelper_IPV4( NetworkBufferDescriptor_t * pxNetworkBuffer,
                                            uint8_t ucTCPFlags );

/*
 * Common code for sending a TCP protocol control packet (i.e. no options, no
 * payload, just flags).
 */
BaseType_t prvTCPSendSpecialPktHelper_IPV6( NetworkBufferDescriptor_t * pxNetworkBuffer,
                                            uint8_t ucTCPFlags );

/*
 * After a listening socket receives a new connection, it may duplicate itself.
 * The copying takes place in prvTCPSocketCopy.
 */
BaseType_t prvTCPSocketCopy( FreeRTOS_Socket_t * pxNewSocket,
                             FreeRTOS_Socket_t * pxSocket );


/* *INDENT-OFF* */
#ifdef __cplusplus
    } /* extern "C" */
#endif
/* *INDENT-ON* */

#endif /* FREERTOS_TCP_STATE_HANDLING_H */
