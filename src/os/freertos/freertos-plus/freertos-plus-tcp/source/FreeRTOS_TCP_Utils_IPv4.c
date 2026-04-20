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
 * @file FreeRTOS_TCP_Utils_IPv4.c
 * @brief Module contains utility functions used by FreeRTOS+TCP module.
 *
 * Endianness: in this module all ports and IP addresses are stored in
 * host byte-order, except fields in the IP-packets
 */
/* Standard includes. */
#include <stdint.h>
#include <stdio.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"

/* FreeRTOS+TCP includes. */
#include "FreeRTOS_IP.h"
#include "FreeRTOS_IP_Private.h"

#include "FreeRTOS_TCP_Utils.h"

/* Just make sure the contents doesn't get compiled if TCP is not enabled. */
/* *INDENT-OFF* */
#if( ipconfigUSE_IPv4 != 0 ) && ( ipconfigUSE_TCP == 1 )
/* *INDENT-ON* */

/**
 * @brief Set the MSS (Maximum segment size) associated with the given socket.
 *
 * @param[in] pxSocket The socket whose MSS is to be set.
 */
void prvSocketSetMSS_IPV4( FreeRTOS_Socket_t * pxSocket )
{
    uint32_t ulMSS = ipconfigTCP_MSS;
    const NetworkEndPoint_t * pxEndPoint = pxSocket->pxEndPoint;

    if( pxEndPoint != NULL )
    {
        /* Do not allow MSS smaller than tcpMINIMUM_SEGMENT_LENGTH. */
        #if ( ipconfigTCP_MSS >= tcpMINIMUM_SEGMENT_LENGTH )
        {
            ulMSS = ipconfigTCP_MSS;
        }
        #else
        {
            ulMSS = tcpMINIMUM_SEGMENT_LENGTH;
        }
        #endif

        /* Check if the remote IP-address belongs to the same netmask. */
        if( ( ( FreeRTOS_ntohl( pxSocket->u.xTCP.xRemoteIP.ulIP_IPv4 ) ^ pxEndPoint->ipv4_settings.ulIPAddress ) & pxEndPoint->ipv4_settings.ulNetMask ) != 0U )
        {
            /* Data for this peer will pass through a router, and maybe through
             * the internet.  Limit the MSS to 1400 bytes or less. */
            ulMSS = FreeRTOS_min_uint32( ( uint32_t ) tcpREDUCED_MSS_THROUGH_INTERNET, ulMSS );
        }
    }

    FreeRTOS_debug_printf( ( "prvSocketSetMSS: %u bytes for %xip port %u\n", ( unsigned ) ulMSS, ( unsigned ) pxSocket->u.xTCP.xRemoteIP.ulIP_IPv4, pxSocket->u.xTCP.usRemotePort ) );

    pxSocket->u.xTCP.usMSS = ( uint16_t ) ulMSS;
}
/*-----------------------------------------------------------*/

/* *INDENT-OFF* */
#endif /* ( ipconfigUSE_IPv4 != 0 ) && ( ipconfigUSE_TCP == 1 ) */
/* *INDENT-ON* */
