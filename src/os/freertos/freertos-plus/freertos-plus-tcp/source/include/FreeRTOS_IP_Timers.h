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
 * @file FreeRTOS_IP_Timers.h
 * @brief Header file for IP Timers on FreeRTOS+TCP network stack.
 */

#ifndef FREERTOS_IP_TIMERS_H
#define FREERTOS_IP_TIMERS_H

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

/* *INDENT-OFF* */
#ifdef __cplusplus
    extern "C" {
#endif
/* *INDENT-ON* */

/*
 * Checks the ARP, ND, DHCP and TCP timers to see if any periodic or timeout
 * processing is required.
 */
void vCheckNetworkTimers( void );

/*
 * Determine how long the IP task can sleep for, which depends on when the next
 * periodic or timeout processing must be performed.
 */
TickType_t xCalculateSleepTime( void );

/*
 *  Enable/disable the TCP timer.
 */
void vIPSetTCPTimerExpiredState( BaseType_t xExpiredState );

#if ipconfigIS_ENABLED( ipconfigUSE_IPv4 )

/**
 * Sets the reload time of an ARP timer and restarts it.
 */
    void vARPTimerReload( TickType_t xTime );

/*
 * Start an ARP Resolution timer.
 */
    void vIPTimerStartARPResolution( TickType_t xTime );

/*
 * Enable/disable the ARP timer.
 */
    void vIPSetARPTimerEnableState( BaseType_t xEnableState );

/*
 * Enable or disable the ARP resolution timer.
 */
    void vIPSetARPResolutionTimerEnableState( BaseType_t xEnableState );
#endif /* if ipconfigIS_ENABLED( ipconfigUSE_IPv4 ) */

#if ipconfigIS_ENABLED( ipconfigUSE_IPv6 )

/**
 * Sets the reload time of an ND timer and restarts it.
 */
    void vNDTimerReload( TickType_t xTime );

/*
 * Start an ND Resolution timer.
 */
    void vIPTimerStartNDResolution( TickType_t xTime );

/*
 * Enable/disable the ND timer.
 */
    void vIPSetNDTimerEnableState( BaseType_t xEnableState );

/*
 * Enable or disable the ARP resolution timer.
 */
    void vIPSetNDResolutionTimerEnableState( BaseType_t xEnableState );
#endif /* if ipconfigIS_ENABLED( ipconfigUSE_IPv6 ) */

#if ( ipconfigUSE_DHCP == 1 ) || ( ipconfigUSE_RA == 1 )

/**
 * @brief Enable/disable the DHCP/RA timer.
 * @param[in] pxEndPoint: The end-point for which the timer will be called.
 * @param[in] xEnableState: pdTRUE - enable timer; pdFALSE - disable timer.
 */
    void vIPSetDHCP_RATimerEnableState( NetworkEndPoint_t * pxEndPoint,
                                        BaseType_t xEnableState );
#endif /* ( ipconfigUSE_DHCP == 1 ) || ( ipconfigUSE_RA == 1 ) */

#if ( ipconfigDNS_USE_CALLBACKS != 0 )

/**
 * @brief Enable/disable the DNS timer.
 * @param[in] xEnableState: pdTRUE - enable timer; pdFALSE - disable timer.
 */
    void vIPSetDNSTimerEnableState( BaseType_t xEnableState );
#endif /* ipconfigDNS_USE_CALLBACKS != 0 */

/**
 * Sets the reload time of an TCP timer and restarts it.
 */
void vTCPTimerReload( TickType_t xTime );

#if ( ipconfigUSE_DHCP == 1 ) || ( ipconfigUSE_RA == 1 )
    void vDHCP_RATimerReload( NetworkEndPoint_t * pxEndPoint,
                              TickType_t uxClockTicks );
#endif /* ( ipconfigUSE_DHCP == 1 ) || ( ipconfigUSE_RA == 1 ) */

#if ( ipconfigDNS_USE_CALLBACKS != 0 )

/**
 * Reload the DNS timer.
 */
    void vDNSTimerReload( uint32_t ulCheckTime );
#endif /* ipconfigDNS_USE_CALLBACKS != 0 */

/**
 * Reload the Network timer.
 */
void vNetworkTimerReload( TickType_t xTime );

/* *INDENT-OFF* */
#ifdef __cplusplus
    } /* extern "C" */
#endif
/* *INDENT-ON* */

#endif /* FREERTOS_IP_TIMERS_H */
