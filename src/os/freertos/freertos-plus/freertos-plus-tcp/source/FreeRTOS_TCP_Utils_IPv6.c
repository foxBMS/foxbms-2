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
 * @file FreeRTOS_TCP_Utils_IPv6.c
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
#if( ipconfigUSE_IPv6 != 0 ) && ( ipconfigUSE_TCP == 1 )
/* *INDENT-ON* */

/**
 * @brief Set the MSS (Maximum segment size) associated with the given socket.
 *
 * @param[in] pxSocket The socket whose MSS is to be set.
 */
void prvSocketSetMSS_IPV6( FreeRTOS_Socket_t * pxSocket )
{
    uint32_t ulMSS = ipconfigTCP_MSS;

    #if ( ipconfigHAS_DEBUG_PRINTF == 1 )
        char cIPv6Address[ 40 ];
    #endif

    const NetworkEndPoint_t * pxEndPoint = NULL;

    do
    {
        if( pxSocket == NULL )
        {
            /* If NULL socket handler, skip all following steps. */
            FreeRTOS_debug_printf( ( "prvSocketSetMSS_IPV6: NULL socket handler\n" ) );

            break;
        }

        pxEndPoint = pxSocket->pxEndPoint;

        if( pxEndPoint != NULL )
        {
            IPv6_Type_t eType;

            /* Compared to IPv4, an IPv6 header is 20 bytes longer.
             * It must be subtracted from the MSS. */
            size_t uxDifference = ipSIZE_OF_IPv6_HEADER - ipSIZE_OF_IPv4_HEADER;

            /* Because ipconfigTCP_MSS is guaranteed not less than tcpMINIMUM_SEGMENT_LENGTH by FreeRTOSIPConfigDefaults.h,
             * it's unnecessary to check if xSocket->u.xTCP.usMSS is greater than difference. */
            ulMSS = ( uint32_t ) ( ipconfigTCP_MSS - uxDifference );
            eType = xIPv6_GetIPType( &( pxSocket->u.xTCP.xRemoteIP.xIP_IPv6 ) );

            if( eType == eIPv6_Global )
            {
                /* The packet will travel through Internet, make the MSS
                 * smaller. */
                ulMSS = FreeRTOS_min_uint32( ( uint32_t ) tcpREDUCED_MSS_THROUGH_INTERNET, ulMSS );
            }
        }

        #if ( ipconfigHAS_DEBUG_PRINTF == 1 )
        {
            ( void ) FreeRTOS_inet_ntop( FREERTOS_AF_INET6, ( const void * ) pxSocket->u.xTCP.xRemoteIP.xIP_IPv6.ucBytes, cIPv6Address, sizeof( cIPv6Address ) );
            FreeRTOS_debug_printf( ( "prvSocketSetMSS: %u bytes for %s ip port %u\n", ( unsigned ) ulMSS, cIPv6Address, pxSocket->u.xTCP.usRemotePort ) );
        }
        #endif

        pxSocket->u.xTCP.usMSS = ( uint16_t ) ulMSS;
    } while( ipFALSE_BOOL );
}
/*-----------------------------------------------------------*/

/* *INDENT-OFF* */
#endif /* ( ipconfigUSE_IPv6 != 0 ) && ( ipconfigUSE_TCP == 1 ) */
/* *INDENT-ON* */
