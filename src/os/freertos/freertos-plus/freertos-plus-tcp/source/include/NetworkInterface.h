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

#ifndef NETWORK_INTERFACE_H
#define NETWORK_INTERFACE_H

/* *INDENT-OFF* */
#ifdef __cplusplus
    extern "C" {
#endif
/* *INDENT-ON* */

#include "FreeRTOS_IP.h"

/* INTERNAL API FUNCTIONS. */

/* Since there are multiple interfaces, there are multiple versions
 * of the following functions.
 * These are now declared static in NetworkInterface.c and their addresses
 * are stored in a struct NetworkInterfaceDescriptor_t.
 *
 *  BaseType_t xNetworkInterfaceInitialise( struct xNetworkInterface *pxInterface );
 *  BaseType_t xGetPhyLinkStatus( struct xNetworkInterface *pxInterface );
 */

/* The following function is defined only when BufferAllocation_1.c is linked in the project. */
size_t uxNetworkInterfaceAllocateRAMToBuffers( NetworkBufferDescriptor_t pxNetworkBuffers[ ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS ] );

BaseType_t xGetPhyLinkStatus( struct xNetworkInterface * pxInterface );

#define MAC_IS_MULTICAST( pucMACAddressBytes )    ( ( pucMACAddressBytes[ 0 ] & 1U ) != 0U )
#define MAC_IS_UNICAST( pucMACAddressBytes )      ( ( pucMACAddressBytes[ 0 ] & 1U ) == 0U )

/* *INDENT-OFF* */
#ifdef __cplusplus
    } /* extern "C" */
#endif
/* *INDENT-ON* */

#endif /* NETWORK_INTERFACE_H */
