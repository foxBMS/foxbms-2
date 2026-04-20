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

#ifndef FREERTOS_IPV6_UTILS_H
#define FREERTOS_IPV6_UTILS_H

/**
 * @file FreeRTOS_IPv6_Utils.h
 * @brief Implements the utility functions for FreeRTOS_IP.c
 */

/* Standard includes. */
#include <stdint.h>
#include <stdio.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"

/* FreeRTOS+TCP includes. */
#include "FreeRTOS_IP.h"


/* *INDENT-OFF* */
#ifdef __cplusplus
    extern "C" {
#endif
/* *INDENT-ON* */

/* Set the MAC-address that belongs to a given IPv6 multi-cast address. */
void vSetMultiCastIPv6MacAddress( const IPv6_Address_t * pxAddress,
                                  MACAddress_t * pxMACAddress );

extern BaseType_t prvChecksumIPv6Checks( uint8_t * pucEthernetBuffer,
                                         size_t uxBufferLength,
                                         struct xPacketSummary * pxSet );

extern BaseType_t prvChecksumICMPv6Checks( size_t uxBufferLength,
                                           struct xPacketSummary * pxSet );

/* Get total length of all extension headers in IPv6 packet. */
size_t usGetExtensionHeaderLength( const uint8_t * pucEthernetBuffer,
                                   size_t uxBufferLength,
                                   uint8_t * pucProtocol );

void vManageSolicitedNodeAddress( const struct xNetworkEndPoint * pxEndPoint,
                                  BaseType_t xNetworkGoingUp );

/* *INDENT-OFF* */
#ifdef __cplusplus
    } /* extern "C" */
#endif
/* *INDENT-ON* */

#endif /* FREERTOS_IPV6_UTILS_H */
