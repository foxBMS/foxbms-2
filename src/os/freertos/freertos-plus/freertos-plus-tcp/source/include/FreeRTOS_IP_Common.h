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

#ifndef FREERTOS_IP_COMMON_H
#define FREERTOS_IP_COMMON_H

/* *INDENT-OFF* */
#ifdef __cplusplus
    extern "C" {
#endif
/* *INDENT-ON* */

struct xIPv6_Address
{
    uint8_t ucBytes[ 16 ];
};

typedef struct xIPv6_Address IPv6_Address_t;

typedef union IP_Address
{
    uint32_t ulIP_IPv4;      /**< IPv4 address */
    IPv6_Address_t xIP_IPv6; /**< IPv6 address */
} IP_Address_t;

/** @brief A struct that can hold either an IPv4 or an IPv6 address. */
typedef struct xxIPv46_Address
{
    /* A struct that can hold either an IPv4 or an IPv6 address. */
    IP_Address_t xIPAddress; /**< IP address contains either IPv4 or IPv6. */
    BaseType_t xIs_IPv6;     /**< pdTRUE if IPv6 address. */
} IPv46_Address_t;

struct xNetworkEndPoint;
struct xNetworkInterface;

/* Return true if a given end-point is up and running.
* When FreeRTOS_IsNetworkUp() is called with NULL as a parameter,
* it will return pdTRUE when all end-points are up. */
BaseType_t FreeRTOS_IsEndPointUp( const struct xNetworkEndPoint * pxEndPoint );

/* Return pdTRUE if all end-points are up.
 * When pxInterface is null, all end-points will be checked. */
BaseType_t FreeRTOS_AllEndPointsUp( const struct xNetworkInterface * pxInterface );

/* *INDENT-OFF* */
#ifdef __cplusplus
    } /* extern "C" */
#endif
/* *INDENT-ON* */

#endif /* FREERTOS_IP_COMMON_H */
