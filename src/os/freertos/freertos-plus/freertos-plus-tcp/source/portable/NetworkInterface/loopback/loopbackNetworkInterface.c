/*
 * FreeRTOS+TCP V4.3.2
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
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

/* Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* FreeRTOS+TCP includes. */
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"
#include "FreeRTOS_IP_Private.h"
#include "FreeRTOS_DNS.h"
#include "FreeRTOS_Routing.h"
#include "FreeRTOS_ND.h"
#include "NetworkBufferManagement.h"
#include "NetworkInterface.h"

#if ( ipconfigUSE_LOOPBACK == 0 )
    #error Please define ipconfigUSE_LOOPBACK as 1 if you want to use the loop-back interface
#endif

#define ipICMP_ECHO_REQUEST    ( ( uint8_t ) 8 )
#define ipICMP_ECHO_REPLY      ( ( uint8_t ) 0 )

/*-----------------------------------------------------------*/

NetworkInterface_t * xLoopbackInterface;

static BaseType_t prvLoopback_Initialise( NetworkInterface_t * pxInterface );
static BaseType_t prvLoopback_Output( NetworkInterface_t * pxInterface,
                                      NetworkBufferDescriptor_t * const pxGivenDescriptor,
                                      BaseType_t bReleaseAfterSend );
static BaseType_t prvLoopback_GetPhyLinkStatus( NetworkInterface_t * pxInterface );

NetworkInterface_t * pxLoopback_FillInterfaceDescriptor( BaseType_t xEMACIndex,
                                                         NetworkInterface_t * pxInterface );

/*-----------------------------------------------------------*/

static BaseType_t prvLoopback_Initialise( NetworkInterface_t * pxInterface )
{
    /* When returning non-zero, the stack will become active and
     * start DHCP (in configured) */
    ( void ) pxInterface;
    return pdTRUE;
}
/*-----------------------------------------------------------*/

#if ( ipconfigIPv4_BACKWARD_COMPATIBLE != 0 )

/* Do not call the following function directly. It is there for downward compatibility.
 * The function FreeRTOS_IPInit() will call it to initialice the interface and end-point
 * objects.  See the description in FreeRTOS_Routing.h. */
    NetworkInterface_t * pxFillInterfaceDescriptor( BaseType_t xEMACIndex,
                                                    NetworkInterface_t * pxInterface )
    {
        return pxLoopback_FillInterfaceDescriptor( xEMACIndex, pxInterface );
    }

#endif /* ( ipconfigIPv4_BACKWARD_COMPATIBLE != 0 ) */
/*-----------------------------------------------------------*/

NetworkInterface_t * pxLoopback_FillInterfaceDescriptor( BaseType_t xEMACIndex,
                                                         NetworkInterface_t * pxInterface )
{
/* This function pxLoopback_FillInterfaceDescriptor() adds a network-interface.
 * Make sure that the object pointed to by 'pxInterface'
 * is declared static or global, and that it will remain to exist. */

    memset( pxInterface, '\0', sizeof( *pxInterface ) );
    pxInterface->pcName = "Loopback";                /* Just for logging, debugging. */
    pxInterface->pvArgument = ( void * ) xEMACIndex; /* Has only meaning for the driver functions. */
    pxInterface->pfInitialise = prvLoopback_Initialise;
    pxInterface->pfOutput = prvLoopback_Output;
    pxInterface->pfGetPhyLinkStatus = prvLoopback_GetPhyLinkStatus;

    FreeRTOS_AddNetworkInterface( pxInterface );
    xLoopbackInterface = pxInterface;

    return pxInterface;
}
/*-----------------------------------------------------------*/

static BaseType_t prvLoopback_GetPhyLinkStatus( NetworkInterface_t * pxInterface )
{
    /* This function returns true if the Link Status in the PHY is high. */
    ( void ) pxInterface;
    return pdTRUE;
}
/*-----------------------------------------------------------*/

static BaseType_t prvLoopback_Output( NetworkInterface_t * pxInterface,
                                      NetworkBufferDescriptor_t * const pxGivenDescriptor,
                                      BaseType_t bReleaseAfterSend )
{
    NetworkBufferDescriptor_t * pxDescriptor = pxGivenDescriptor;

    ( void ) pxInterface;

    IPPacket_t * a = ( IPPacket_t * ) ( pxDescriptor->pucEthernetBuffer );

    if( a->xEthernetHeader.usFrameType == ipIPv4_FRAME_TYPE )
    {
        usGenerateProtocolChecksum( pxDescriptor->pucEthernetBuffer, pxDescriptor->xDataLength, pdTRUE );
    }

    {
        const MACAddress_t * pxMACAddress = &( pxDescriptor->pxEndPoint->xMACAddress );

        if( pxDescriptor->pxEndPoint->bits.bIPv6 != 0 )
        {
            #if ( ipconfigUSE_IPv6 != 0 )
                if( xIsIPv6Loopback( &( pxDescriptor->xIPAddress.xIP_IPv6 ) ) != pdFALSE )
                {
                    vNDRefreshCacheEntry( pxMACAddress, &( pxDescriptor->xIPAddress.xIP_IPv6 ), pxDescriptor->pxEndPoint );
                }
            #endif
        }
        else
        {
            #if ( ipconfigUSE_IPv4 != 0 )
                if( xIsIPv4Loopback( pxDescriptor->xIPAddress.ulIP_IPv4 ) != pdFALSE )
                {
                    vARPRefreshCacheEntry( pxMACAddress, pxDescriptor->xIPAddress.ulIP_IPv4, pxDescriptor->pxEndPoint );
                }
            #endif
        }
    }

    if( bReleaseAfterSend == pdFALSE )
    {
        NetworkBufferDescriptor_t * pxNewDescriptor =
            pxDuplicateNetworkBufferWithDescriptor( pxDescriptor, pxDescriptor->xDataLength );
        pxDescriptor = pxNewDescriptor;
    }

    if( pxDescriptor != NULL )
    {
        IPStackEvent_t xRxEvent;

        xRxEvent.eEventType = eNetworkRxEvent;
        xRxEvent.pvData = ( void * ) pxDescriptor;

        pxDescriptor->pxInterface = xLoopbackInterface;
        pxDescriptor->pxEndPoint = FreeRTOS_MatchingEndpoint( xLoopbackInterface, pxDescriptor->pucEthernetBuffer );

        if( pxDescriptor->pxEndPoint == NULL )
        {
            vReleaseNetworkBufferAndDescriptor( pxDescriptor );
            iptraceETHERNET_RX_EVENT_LOST();
            FreeRTOS_printf( ( "prvLoopback_Output: Can not find a proper endpoint\n" ) );
        }
        else if( xSendEventStructToIPTask( &xRxEvent, 0u ) != pdTRUE )
        {
            /* Sending failed, release the descriptor. */
            vReleaseNetworkBufferAndDescriptor( pxDescriptor );
            iptraceETHERNET_RX_EVENT_LOST();
            FreeRTOS_printf( ( "prvLoopback_Output: Can not queue return packet!\n" ) );
        }
    }

    /* The return value is actually ignored by the IP-stack. */
    return pdTRUE;
}
/*-----------------------------------------------------------*/
