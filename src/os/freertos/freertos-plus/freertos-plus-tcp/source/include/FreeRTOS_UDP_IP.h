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

#ifndef FREERTOS_UDP_IP_H
#define FREERTOS_UDP_IP_H

/* Application level configuration options. */
#include "FreeRTOSIPConfig.h"
#include "FreeRTOSIPConfigDefaults.h"
#include "FreeRTOS_IP.h"

/* *INDENT-OFF* */
#ifdef __cplusplus
    extern "C" {
#endif
/* *INDENT-ON* */

/*
 * Called when the application has generated a UDP packet to send.
 */
void vProcessGeneratedUDPPacket( NetworkBufferDescriptor_t * const pxNetworkBuffer );

void vProcessGeneratedUDPPacket_IPv4( NetworkBufferDescriptor_t * const pxNetworkBuffer );
void vProcessGeneratedUDPPacket_IPv6( NetworkBufferDescriptor_t * const pxNetworkBuffer );

/*
 * The caller must ensure that pxNetworkBuffer->xDataLength is the UDP packet
 * payload size (excluding packet headers) and that the packet in pucEthernetBuffer
 * is at least the size of UDPPacket_t.
 */
BaseType_t xProcessReceivedUDPPacket( NetworkBufferDescriptor_t * pxNetworkBuffer,
                                      uint16_t usPort,
                                      BaseType_t * pxIsWaitingForResolution );

BaseType_t xProcessReceivedUDPPacket_IPv4( NetworkBufferDescriptor_t * pxNetworkBuffer,
                                           uint16_t usPort,
                                           BaseType_t * pxIsWaitingForARPResolution );

BaseType_t xProcessReceivedUDPPacket_IPv6( NetworkBufferDescriptor_t * pxNetworkBuffer,
                                           uint16_t usPort,
                                           BaseType_t * pxIsWaitingForNDResolution );

/* *INDENT-OFF* */
#ifdef __cplusplus
    } /* extern "C" */
#endif
/* *INDENT-ON* */

#endif /* FREERTOS_UDP_IP_H */
