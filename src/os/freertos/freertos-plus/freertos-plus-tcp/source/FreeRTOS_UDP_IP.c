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
 * @file FreeRTOS_UDP_IP.c
 * @brief This file has the source code for the UDP-IP functionality of the FreeRTOS+TCP
 *        network stack.
 */

/* Standard includes. */
#include <stdint.h>
#include <stdio.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"
#include "list.h"

/* FreeRTOS+TCP includes. */
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"
#include "FreeRTOS_IP_Private.h"
#include "FreeRTOS_UDP_IP.h"
#include "FreeRTOS_DNS.h"
#include "FreeRTOS_DHCP.h"
#include "FreeRTOS_IP_Utils.h"
#include "NetworkInterface.h"
#include "NetworkBufferManagement.h"

#if ( ipconfigUSE_DNS == 1 )
    #include "FreeRTOS_DNS.h"
#endif

/**
 * @brief Process the generated UDP packet and do other checks before sending the
 *        packet such as cache check and address resolution.
 *
 * @param[in] pxNetworkBuffer The network buffer carrying the packet.
 */
void vProcessGeneratedUDPPacket( NetworkBufferDescriptor_t * const pxNetworkBuffer )
{
    const UDPPacket_t * pxUDPPacket;

    if( pxNetworkBuffer != NULL )
    {
        /* Map the UDP packet onto the start of the frame. */

        /* MISRA Ref 11.3.1 [Misaligned access] */
        /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
        /* coverity[misra_c_2012_rule_11_3_violation] */
        pxUDPPacket = ( ( UDPPacket_t * ) pxNetworkBuffer->pucEthernetBuffer );

        switch( pxUDPPacket->xEthernetHeader.usFrameType )
        {
            #if ( ipconfigUSE_IPv4 != 0 )
                case ipIPv4_FRAME_TYPE:
                    vProcessGeneratedUDPPacket_IPv4( pxNetworkBuffer );
                    break;
            #endif
            #if ( ipconfigUSE_IPv6 != 0 )
                case ipIPv6_FRAME_TYPE:
                    vProcessGeneratedUDPPacket_IPv6( pxNetworkBuffer );
                    break;
            #endif
            default:
                /* do nothing, coverity happy */
                break;
        }
    }
}
/*-----------------------------------------------------------*/

/**
 * @brief Process the received UDP packet.
 *
 * @param[in] pxNetworkBuffer The network buffer carrying the UDP packet.
 * @param[in] usPort The port number on which this packet was received.
 * @param[out] pxIsWaitingForResolution If the packet is awaiting resolution,
 *             this pointer will be set to pdTRUE. pdFALSE otherwise.
 *
 * @return pdPASS in case the UDP packet could be processed. Else pdFAIL is returned.
 */
BaseType_t xProcessReceivedUDPPacket( NetworkBufferDescriptor_t * pxNetworkBuffer,
                                      uint16_t usPort,
                                      BaseType_t * pxIsWaitingForResolution )
{
    /* Returning pdPASS means that the packet was consumed, released. */
    BaseType_t xReturn = pdFAIL;
    const UDPPacket_t * pxUDPPacket;

    configASSERT( pxNetworkBuffer != NULL );
    configASSERT( pxNetworkBuffer->pucEthernetBuffer != NULL );

    /* Map the ethernet buffer to the UDPPacket_t struct for easy access to the fields. */

    /* MISRA Ref 11.3.1 [Misaligned access] */
    /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
    /* coverity[misra_c_2012_rule_11_3_violation] */
    pxUDPPacket = ( ( const UDPPacket_t * ) pxNetworkBuffer->pucEthernetBuffer );

    switch( pxUDPPacket->xEthernetHeader.usFrameType )
    {
        #if ( ipconfigUSE_IPv4 != 0 )
            case ipIPv4_FRAME_TYPE:
                xReturn = xProcessReceivedUDPPacket_IPv4( pxNetworkBuffer,
                                                          usPort, pxIsWaitingForResolution );
                break;
        #endif
        #if ( ipconfigUSE_IPv6 != 0 )
            case ipIPv6_FRAME_TYPE:
                xReturn = xProcessReceivedUDPPacket_IPv6( pxNetworkBuffer,
                                                          usPort, pxIsWaitingForResolution );
                break;
        #endif
        default:
            /* do nothing, coverity happy */
            break;
    }

    return xReturn;
}
/*-----------------------------------------------------------*/
