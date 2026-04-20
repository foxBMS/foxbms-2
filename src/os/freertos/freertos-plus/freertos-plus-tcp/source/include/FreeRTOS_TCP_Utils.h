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

#ifndef FREERTOS_TCP_UTILS_H
#define FREERTOS_TCP_UTILS_H

/* *INDENT-OFF* */
#ifdef __cplusplus
    extern "C" {
#endif
/* *INDENT-ON* */


/*
 * For logging and debugging: make a string showing the TCP flags.
 */
#if ( ipconfigHAS_DEBUG_PRINTF != 0 )
    const char * prvTCPFlagMeaning( UBaseType_t xFlags );
#endif /* ipconfigHAS_DEBUG_PRINTF != 0 */

/*
 * Set the initial value for MSS (Maximum Segment Size) to be used.
 */
void prvSocketSetMSS( FreeRTOS_Socket_t * pxSocket );

/*
 * Set the initial value for MSS (Maximum Segment Size) to be used.
 */
void prvSocketSetMSS_IPV4( FreeRTOS_Socket_t * pxSocket );

/*
 * Set the initial value for MSS (Maximum Segment Size) to be used.
 */
void prvSocketSetMSS_IPV6( FreeRTOS_Socket_t * pxSocket );

/* *INDENT-OFF* */
#ifdef __cplusplus
    } /* extern "C" */
#endif
/* *INDENT-ON* */

#endif /* FREERTOS_TCP_UTILS_H */
