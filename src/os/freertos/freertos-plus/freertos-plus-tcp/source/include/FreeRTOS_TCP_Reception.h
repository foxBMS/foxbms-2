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

#ifndef FREERTOS_TCP_RECEPTION_H
#define FREERTOS_TCP_RECEPTION_H

/* *INDENT-OFF* */
#ifdef __cplusplus
    extern "C" {
#endif
/* *INDENT-ON* */

/*
 * Called from xProcessReceivedTCPPacket. Parse the TCP option(s) received,
 * if present. This function returns pdFALSE if the options are not well formed.
 */
BaseType_t prvCheckOptions( FreeRTOS_Socket_t * pxSocket,
                            const NetworkBufferDescriptor_t * pxNetworkBuffer );

/*
 * Called from prvTCPHandleState().  Find the TCP payload data and check and
 * return its length.
 */
BaseType_t prvCheckRxData( const NetworkBufferDescriptor_t * pxNetworkBuffer,
                           uint8_t ** ppucRecvData );

/*
 * Called from prvTCPHandleState().  Check if the payload data may be accepted.
 * If so, it will be added to the socket's reception queue.
 */
BaseType_t prvStoreRxData( FreeRTOS_Socket_t * pxSocket,
                           const uint8_t * pucRecvData,
                           NetworkBufferDescriptor_t * pxNetworkBuffer,
                           uint32_t ulReceiveLength );

/* *INDENT-OFF* */
#ifdef __cplusplus
    } /* extern "C" */
#endif
/* *INDENT-ON* */

#endif /* FREERTOS_TCP_RECEPTION_H */
