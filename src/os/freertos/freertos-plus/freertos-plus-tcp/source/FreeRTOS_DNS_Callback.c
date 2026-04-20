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
 * @file FreeRTOS_DNS_Callback.c
 * @brief File that handles the DNS Callback option
 */

#include "FreeRTOS_DNS_Callback.h"

#include "FreeRTOS_IP.h"
#include "FreeRTOS_IP_Private.h"
#include "FreeRTOS_DNS_Globals.h"
#include "FreeRTOS_IP_Timers.h"

#if ( ( ipconfigDNS_USE_CALLBACKS == 1 ) && ( ipconfigUSE_DNS != 0 ) )

/**
 * @brief list of callbacks to send
 */
    static List_t xCallbackList;

/**
 * @brief A DNS reply was received, see if there is any matching entry and
 *        call the handler.
 *
 * @param[in,out] pxSet a set of variables that are shared among the helper functions.
 * @param[in] pxAddress Pointer to address info ( IPv4/IPv6 ) obtained from the DNS server.
 *
 * @return Returns pdTRUE if uxIdentifier was recognized.
 */
    BaseType_t xDNSDoCallback( ParseSet_t * pxSet,
                               struct freertos_addrinfo * pxAddress )
    {
        BaseType_t xResult = pdFALSE;
        const ListItem_t * pxIterator;
        const ListItem_t * xEnd = listGET_END_MARKER( &xCallbackList );
        TickType_t uxIdentifier = ( TickType_t ) pxSet->pxDNSMessageHeader->usIdentifier;

        /* While iterating through the list, the scheduler is suspended.
         * Remember which function shall be called once the scheduler is
         * running again. */
        FOnDNSEvent pCallbackFunction = NULL;
        void * pvSearchID = NULL;

        vTaskSuspendAll();
        {
            for( pxIterator = ( const ListItem_t * ) listGET_NEXT( xEnd );
                 pxIterator != ( const ListItem_t * ) xEnd;
                 pxIterator = ( const ListItem_t * ) listGET_NEXT( pxIterator ) )
            {
                BaseType_t xMatching;
                DNSCallback_t * pxCallback = ( ( DNSCallback_t * ) listGET_LIST_ITEM_OWNER( pxIterator ) );
                #if ( ipconfigUSE_MDNS == 1 )
                    /* mDNS port 5353. Host byte order comparison. */
                    if( pxSet->usPortNumber == ipMDNS_PORT )
                    {
                        /* In mDNS, the query ID field is ignored and the
                         * hostname will be compared with outstanding requests. */

                        xMatching = ( strcasecmp( pxCallback->pcName, pxSet->pcName ) == 0 ) ? pdTRUE : pdFALSE;
                    }
                    else
                #endif /* if ( ipconfigUSE_MDNS == 1 ) */
                {
                    xMatching = ( listGET_LIST_ITEM_VALUE( pxIterator ) == uxIdentifier ) ? pdTRUE : pdFALSE;
                }

                if( xMatching == pdTRUE )
                {
                    pvSearchID = pxCallback->pvSearchID;
                    pCallbackFunction = pxCallback->pCallbackFunction;
                    ( void ) uxListRemove( &pxCallback->xListItem );
                    vPortFree( pxCallback );

                    if( listLIST_IS_EMPTY( &xCallbackList ) != pdFALSE )
                    {
                        /* The list of outstanding requests is empty. No need for periodic polling. */
                        vIPSetDNSTimerEnableState( pdFALSE );
                    }

                    xResult = pdTRUE;
                    break;
                }
            }
        }
        ( void ) xTaskResumeAll();

        if( pCallbackFunction != NULL )
        {
            pCallbackFunction( pxSet->pcName, pvSearchID, pxAddress );
        }

        return xResult;
    }

/**
 * @brief FreeRTOS_gethostbyname_a() was called along with callback parameters.
 *        Store them in a list for later reference.
 *
 * @param[in] pcHostName The hostname whose IP address is being searched for.
 * @param[in] pvSearchID The search ID of the DNS callback function to set.
 * @param[in] pCallbackFunction The callback function pointer.
 * @param[in] uxTimeout Timeout of the callback function.
 * @param[in] uxIdentifier Random number used as ID in the DNS message.
 * @param[in] xIsIPv6 pdTRUE if the address type should be IPv6.
 */
    BaseType_t xDNSSetCallBack( const char * pcHostName,
                                void * pvSearchID,
                                FOnDNSEvent pCallbackFunction,
                                TickType_t uxTimeout,
                                TickType_t uxIdentifier,
                                BaseType_t xIsIPv6 )
    {
        BaseType_t xReturn = pdPASS;
        size_t lLength = strlen( pcHostName );

        /* MISRA Ref 4.12.1 [Use of dynamic memory]. */
        /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#directive-412. */
        /* coverity[misra_c_2012_directive_4_12_violation] */
        DNSCallback_t * pxCallback = ( ( DNSCallback_t * ) pvPortMalloc( sizeof( *pxCallback ) + lLength ) );

        /* Translate from ms to number of clock ticks. */
        uxTimeout /= portTICK_PERIOD_MS;

        if( pxCallback != NULL )
        {
            if( listLIST_IS_EMPTY( &xCallbackList ) != pdFALSE )
            {
                /* This is the first one, start the DNS timer to check for timeouts */
                vDNSTimerReload( FreeRTOS_min_uint32( 1000U, ( uint32_t ) uxTimeout ) );
            }

            ( void ) strncpy( pxCallback->pcName, pcHostName, lLength + 1U );
            pxCallback->pCallbackFunction = pCallbackFunction;
            pxCallback->pvSearchID = pvSearchID;
            pxCallback->uxRemainingTime = uxTimeout;
            pxCallback->xIsIPv6 = xIsIPv6;

            vTaskSetTimeOutState( &( pxCallback->uxTimeoutState ) );
            listSET_LIST_ITEM_OWNER( &( pxCallback->xListItem ), ( void * ) pxCallback );
            listSET_LIST_ITEM_VALUE( &( pxCallback->xListItem ), uxIdentifier );
            vTaskSuspendAll();
            {
                vListInsertEnd( &xCallbackList, &pxCallback->xListItem );
            }
            ( void ) xTaskResumeAll();
        }
        else
        {
            xReturn = pdFAIL;
            FreeRTOS_debug_printf( ( " xDNSSetCallBack : Could not allocate memory: %u bytes",
                                     ( unsigned ) ( sizeof( *pxCallback ) + lLength ) ) );
        }

        return xReturn;
    }
/*-----------------------------------------------------------*/

/**
 * @brief Iterate through the list of call-back structures and remove
 * old entries which have reached a timeout.
 * As soon as the list has become empty, the DNS timer will be stopped.
 * In case pvSearchID is supplied, the user wants to cancel a DNS request.
 *
 * @param[in] pvSearchID The search ID of callback function whose associated
 *                 DNS request is being cancelled. If non-ID specific checking of
 *                 all requests is required, then this field should be kept as NULL.
 */
    void vDNSCheckCallBack( void * pvSearchID )
    {
        const ListItem_t * pxIterator;
        const ListItem_t * xEnd = listGET_END_MARKER( &xCallbackList );

        /* When a DNS-search times out, the call-back function shall
         * be called. Store theses item in a temporary list.
         * Only when the scheduler is running, user functions
         * shall be called. */
        List_t xTempList;

        vListInitialise( &xTempList );

        vTaskSuspendAll();
        {
            for( pxIterator = ( const ListItem_t * ) listGET_NEXT( xEnd );
                 pxIterator != xEnd; )
            {
                DNSCallback_t * pxCallback = ( ( DNSCallback_t * ) listGET_LIST_ITEM_OWNER( pxIterator ) );
                /* Move to the next item because we might remove this item */
                pxIterator = ( const ListItem_t * ) listGET_NEXT( pxIterator );

                if( ( pvSearchID != NULL ) && ( pvSearchID == pxCallback->pvSearchID ) )
                {
                    ( void ) uxListRemove( &( pxCallback->xListItem ) );
                    vPortFree( pxCallback );
                }
                else if( xTaskCheckForTimeOut( &pxCallback->uxTimeoutState, &( pxCallback->uxRemainingTime ) ) != pdFALSE )
                {
                    /* A time-out occurred in the asynchronous search.
                     * Remove it from xCallbackList. */
                    ( void ) uxListRemove( &( pxCallback->xListItem ) );

                    /* Insert it in a temporary list. The function will be called
                     * once the scheduler is resumed. */
                    vListInsertEnd( &( xTempList ), &pxCallback->xListItem );
                }
                else
                {
                    /* This call-back is still waiting for a reply or a time-out. */
                }
            }
        }
        ( void ) xTaskResumeAll();

        if( listLIST_IS_EMPTY( &xTempList ) == pdFALSE )
        {
            /* There is at least one item in xTempList which must be removed and deleted. */
            xEnd = listGET_END_MARKER( &xTempList );

            for( pxIterator = ( const ListItem_t * ) listGET_NEXT( xEnd );
                 pxIterator != xEnd;
                 )
            {
                DNSCallback_t * pxCallback = ( ( DNSCallback_t * ) listGET_LIST_ITEM_OWNER( pxIterator ) );
                /* Move to the next item because we might remove this item */
                pxIterator = ( const ListItem_t * ) listGET_NEXT( pxIterator );

                /* A time-out occurred in the asynchronous search.
                 * Call the application hook with the proper information. */
                if( pxCallback->xIsIPv6 != 0 )
                {
                    pxCallback->pCallbackFunction( pxCallback->pcName, pxCallback->pvSearchID, NULL );
                }
                else
                {
                    pxCallback->pCallbackFunction( pxCallback->pcName, pxCallback->pvSearchID, 0U );
                }

                /* Remove it from 'xTempList' and free the memory. */
                ( void ) uxListRemove( &( pxCallback->xListItem ) );
                vPortFree( pxCallback );
            }
        }

        if( listLIST_IS_EMPTY( &xCallbackList ) != pdFALSE )
        {
            vIPSetDNSTimerEnableState( pdFALSE );
        }
    }
/*-----------------------------------------------------------*/

/**
 * @brief initialize the cache
 * @post will modify global list xCallbackList
 */
    void vDNSCallbackInitialise()
    {
        vListInitialise( &xCallbackList );
    }
#endif /* if ( ipconfigDNS_USE_CALLBACKS == 1 ) */
