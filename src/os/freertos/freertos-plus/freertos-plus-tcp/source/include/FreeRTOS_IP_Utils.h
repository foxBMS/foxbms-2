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

/**
 * @file FreeRTOS_IP_Utils.h
 * @brief Implements the utility functions for FreeRTOS_IP.c
 */

#ifndef FREERTOS_IP_UTILS_H
#define FREERTOS_IP_UTILS_H

/* Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* FreeRTOS+TCP includes. */
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"
#include "FreeRTOS_IP_Private.h"
#include "FreeRTOS_UDP_IP.h"
#include "FreeRTOS_DHCP.h"
#include "NetworkInterface.h"
#include "NetworkBufferManagement.h"
#include "FreeRTOS_DNS.h"

#include "FreeRTOS_IPv4_Utils.h"
#include "FreeRTOS_IPv6_Utils.h"

/* *INDENT-OFF* */
#ifdef __cplusplus
    extern "C" {
#endif
/* *INDENT-ON* */

/* Forward declaration. */
struct xNetworkInterface;
struct xNetworkEndPoint;

#if ( ( ipconfigUSE_DHCPv6 == 1 ) || ( ipconfigUSE_DHCP == 1 ) || ( ipconfigUSE_RA == 1 ) )

/**
 * @brief Create a DHCP event.
 *
 * @return pdPASS or pdFAIL, depending on whether xSendEventStructToIPTask()
 *         succeeded.
 */
    BaseType_t xSendDHCPEvent( struct xNetworkEndPoint * pxEndPoint );
#endif

#if ( ( ipconfigUSE_DHCPv6 == 1 ) || ( ipconfigUSE_DHCP == 1 ) )

/* Returns the current state of a DHCP process. */
    eDHCPState_t eGetDHCPState( const struct xNetworkEndPoint * pxEndPoint );

#endif

#if ( ipconfigZERO_COPY_TX_DRIVER != 0 ) || ( ipconfigZERO_COPY_RX_DRIVER != 0 )

/**
 * @brief Get the network buffer from the packet buffer.
 *
 * @param[in] pvBuffer: Pointer to the packet buffer.
 *
 * @return The network buffer if the alignment is correct. Else a NULL is returned.
 */
    NetworkBufferDescriptor_t * pxPacketBuffer_to_NetworkBuffer( const void * pvBuffer );
#endif

/**
 * @brief Check the values of configuration options and assert on it. Also verify that the IP-task
 *        has not already been initialized.
 */
void vPreCheckConfigs( void );

/**
 * @brief Called to create a network connection when the stack is first
 *        started, or when the network connection is lost.
 */
void prvProcessNetworkDownEvent( struct xNetworkInterface * pxInterface );


/* *INDENT-OFF* */
#ifdef __cplusplus
    } /* extern "C" */
#endif
/* *INDENT-ON* */

#endif /* FREERTOS_IP_UTILS_H */
