/*
 * FreeRTOS+TCP V4.3.2
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
 * @file FreeRTOS_TCP_WIN.c
 * @brief Module which handles the TCP windowing schemes for FreeRTOS+TCP.  Many
 * functions have two versions - one for FreeRTOS+TCP (full) and one for
 * FreeRTOS+TCP (lite).
 *
 * In this module all ports and IP addresses and sequence numbers are
 * being stored in host byte-order.
 */

/* Standard includes. */
#include <stdint.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"

/* FreeRTOS+TCP includes. */
#include "FreeRTOS_IP.h"
#include "FreeRTOS_UDP_IP.h"
#include "FreeRTOS_Sockets.h"
#include "FreeRTOS_IP_Private.h"

#if ( ipconfigUSE_TCP == 1 )

/* Constants used for Smoothed Round Trip Time (SRTT). */
    #define winSRTT_INCREMENT_NEW        2                                     /**< New increment for the smoothed RTT. */
    #define winSRTT_INCREMENT_CURRENT    6                                     /**< Current increment for the smoothed RTT. */
    #define winSRTT_DECREMENT_NEW        1                                     /**< New decrement for the smoothed RTT. */
    #define winSRTT_DECREMENT_CURRENT    7                                     /**< Current decrement for the smoothed RTT. */
    #define winSRTT_CAP_mS               ( ipconfigTCP_SRTT_MINIMUM_VALUE_MS ) /**< Cap in milliseconds. */

    #if ( ipconfigUSE_TCP_WIN == 1 )

/** @brief Create a new Rx window. */
        #define xTCPWindowRxNew( pxWindow, ulSequenceNumber, lCount )    xTCPWindowNew( pxWindow, ulSequenceNumber, lCount, pdTRUE )

/** @brief Create a new Tx window. */
        #define xTCPWindowTxNew( pxWindow, ulSequenceNumber, lCount )    xTCPWindowNew( pxWindow, ulSequenceNumber, lCount, pdFALSE )

/** @brief The code to send a single Selective ACK (SACK):
 * NOP (0x01), NOP (0x01), SACK (0x05), LEN (0x0a),
 * followed by a lower and a higher sequence number,
 * where LEN is 2 + 2*4 = 10 bytes. */
        #if ( ipconfigBYTE_ORDER == pdFREERTOS_BIG_ENDIAN )
            #define OPTION_CODE_SINGLE_SACK    ( 0x0101050aU )
        #else
            #define OPTION_CODE_SINGLE_SACK    ( 0x0a050101U )
        #endif

/** @brief Normal retransmission:
 * A packet will be retransmitted after a Retransmit Time-Out (RTO).
 * Fast retransmission:
 * When 3 packets with a higher sequence number have been acknowledged
 * by the peer, it is very unlikely a current packet will ever arrive.
 * It will be retransmitted far before the RTO.
 */
        #define DUPLICATE_ACKS_BEFORE_FAST_RETRANSMIT    ( 3U )

/** @brief If there have been several retransmissions (4), decrease the
 * size of the transmission window to at most 2 times MSS.
 */
        #define MAX_TRANSMIT_COUNT_USING_LARGE_WINDOW    ( 4U )

    #endif /* configUSE_TCP_WIN */
/*-----------------------------------------------------------*/

    #if ( ipconfigUSE_TCP_WIN == 1 )
        static void vListInsertGeneric( List_t * const pxList,
                                        ListItem_t * const pxNewListItem,
                                        MiniListItem_t * pxWhere );
    #endif

/*
 * All TCP sockets share a pool of segment descriptors (TCPSegment_t)
 * Available descriptors are stored in the 'xSegmentList'
 * When a socket owns a descriptor, it will either be stored in
 * 'xTxSegments' or 'xRxSegments'
 * As soon as a package has been confirmed, the descriptor will be returned
 * to the segment pool
 */
    #if ( ipconfigUSE_TCP_WIN == 1 )
        static BaseType_t prvCreateSectors( void );
    #endif /* ipconfigUSE_TCP_WIN == 1 */

/*
 * Find a segment with a given sequence number in the list of received
 * segments: 'pxWindow->xRxSegments'.
 */
    #if ( ipconfigUSE_TCP_WIN == 1 )
        static TCPSegment_t * xTCPWindowRxFind( const TCPWindow_t * pxWindow,
                                                uint32_t ulSequenceNumber );
    #endif /* ipconfigUSE_TCP_WIN == 1 */

/*
 * Allocate a new segment
 * The socket will borrow all segments from a common pool: 'xSegmentList',
 * which is a list of 'TCPSegment_t'
 */
    #if ( ipconfigUSE_TCP_WIN == 1 )
        static TCPSegment_t * xTCPWindowNew( TCPWindow_t * pxWindow,
                                             uint32_t ulSequenceNumber,
                                             int32_t lCount,
                                             BaseType_t xIsForRx );
    #endif /* ipconfigUSE_TCP_WIN == 1 */

/*
 * Detaches and returns the head of a queue
 */
    #if ( ipconfigUSE_TCP_WIN == 1 )
        static TCPSegment_t * xTCPWindowGetHead( const List_t * pxList );
    #endif /* ipconfigUSE_TCP_WIN == 1 */

/*
 * Returns the head of a queue but it won't be detached
 */
    #if ( ipconfigUSE_TCP_WIN == 1 )
        static TCPSegment_t * xTCPWindowPeekHead( const List_t * pxList );
    #endif /* ipconfigUSE_TCP_WIN == 1 */

/*
 * Free entry pxSegment because it's not used anymore
 * The ownership will be passed back to the segment pool
 */
    #if ( ipconfigUSE_TCP_WIN == 1 )
        static void vTCPWindowFree( TCPSegment_t * pxSegment );
    #endif /* ipconfigUSE_TCP_WIN == 1 */

/*
 * A segment has been received with sequence number 'ulSequenceNumber', where
 * 'ulCurrentSequenceNumber == ulSequenceNumber', which means that exactly this
 * segment was expected.  xTCPWindowRxConfirm() will check if there is already
 * another segment with a sequence number between (ulSequenceNumber) and
 * (ulSequenceNumber+xLength).  Normally none will be found, because the next Rx
 * segment should have a sequence number equal to '(ulSequenceNumber+xLength)'.
 */
    #if ( ipconfigUSE_TCP_WIN == 1 )
        static TCPSegment_t * xTCPWindowRxConfirm( const TCPWindow_t * pxWindow,
                                                   uint32_t ulSequenceNumber,
                                                   uint32_t ulLength );
    #endif /* ipconfigUSE_TCP_WIN == 1 */

/*
 * FreeRTOS+TCP stores data in circular buffers.  Calculate the next position to
 * store.
 */
    #if ( ipconfigUSE_TCP_WIN == 1 )
        static int32_t lTCPIncrementTxPosition( int32_t lPosition,
                                                int32_t lMax,
                                                int32_t lCount );
    #endif /* ipconfigUSE_TCP_WIN == 1 */

/*
 * This function will look if there is new transmission data.  It will return
 * true if there is data to be sent.
 */
    #if ( ipconfigUSE_TCP_WIN == 1 )
        static BaseType_t prvTCPWindowTxHasSpace( TCPWindow_t const * pxWindow,
                                                  uint32_t ulWindowSize );
    #endif /* ipconfigUSE_TCP_WIN == 1 */

/*
 * An acknowledge was received.  See if some outstanding data may be removed
 * from the transmission queue(s).
 */
    #if ( ipconfigUSE_TCP_WIN == 1 )
        static uint32_t prvTCPWindowTxCheckAck( TCPWindow_t * pxWindow,
                                                uint32_t ulFirst,
                                                uint32_t ulLast );
    #endif /* ipconfigUSE_TCP_WIN == 1 */

/*
 * A higher Tx block has been acknowledged.  Now iterate through the xWaitQueue
 * to find a possible condition for a FAST retransmission.
 */
    #if ( ipconfigUSE_TCP_WIN == 1 )
        static uint32_t prvTCPWindowFastRetransmit( TCPWindow_t * pxWindow,
                                                    uint32_t ulFirst );
    #endif /* ipconfigUSE_TCP_WIN == 1 */

/*-----------------------------------------------------------*/

/**< TCP segment pool. */
    #if ( ipconfigUSE_TCP_WIN == 1 )
        static TCPSegment_t * xTCPSegments = NULL;
    #endif /* ipconfigUSE_TCP_WIN == 1 */

/**< List of free TCP segments. */
    #if ( ipconfigUSE_TCP_WIN == 1 )
        _static List_t xSegmentList;
    #endif

    #if ( ipconfigUSE_TCP_WIN == 1 )
/** @brief Logging verbosity level. */
        BaseType_t xTCPWindowLoggingLevel = 0;
    #endif

    #if ( ipconfigUSE_TCP_WIN == 1 )
        /* Some 32-bit arithmetic: comparing sequence numbers */
        static portINLINE BaseType_t xSequenceLessThanOrEqual( uint32_t a,
                                                               uint32_t b );

/**
 * @brief Check if a <= b.
 *
 * @param[in] a The value on the left-hand side.
 * @param[in] b The value on the right-hand side.
 *
 * @return pdTRUE when "( b - a ) < 0x80000000". Else, pdFALSE.
 */
        static portINLINE BaseType_t xSequenceLessThanOrEqual( uint32_t a,
                                                               uint32_t b )
        {
            BaseType_t xResult = pdFALSE;

            /* Test if a <= b
             * Return true if the unsigned subtraction of (b-a) doesn't generate an
             * arithmetic overflow. */
            if( ( ( b - a ) & 0x80000000U ) == 0U )
            {
                xResult = pdTRUE;
            }

            return xResult;
        }

    #endif /* ipconfigUSE_TCP_WIN */
/*-----------------------------------------------------------*/

/**
 * @brief Check if a < b.
 *
 * @param[in] a The value on the left-hand side.
 * @param[in] b The value on the right-hand side.
 *
 * @return pdTRUE when "( b - ( a + 1 ) ) < 0x80000000", else pdFALSE.
 */
    BaseType_t xSequenceLessThan( uint32_t a,
                                  uint32_t b )
    {
        BaseType_t xResult = pdFALSE;

        /* Test if a < b */
        if( ( ( b - ( a + 1U ) ) & 0x80000000U ) == 0U )
        {
            xResult = pdTRUE;
        }

        return xResult;
    }

/*-----------------------------------------------------------*/

/**
 * @brief Check if a > b.
 *
 * @param[in] a The value on the left-hand side.
 * @param[in] b The value on the right-hand side.
 *
 * @return pdTRUE when "( a - b ) < 0x80000000", else pdFALSE.
 */
    BaseType_t xSequenceGreaterThan( uint32_t a,
                                     uint32_t b )
    {
        BaseType_t xResult = pdFALSE;

        /* Test if a > b */
        if( ( ( a - ( b + 1U ) ) & 0x80000000U ) == 0U )
        {
            xResult = pdTRUE;
        }

        return xResult;
    }


/*-----------------------------------------------------------*/
    static portINLINE BaseType_t xSequenceGreaterThanOrEqual( uint32_t a,
                                                              uint32_t b );

/**
 * @brief Test if a>=b. This function is required since the sequence numbers can roll over.
 *
 * @param[in] a The first sequence number.
 * @param[in] b The second sequence number.
 *
 * @return pdTRUE if a>=b, else pdFALSE.
 */
    static portINLINE BaseType_t xSequenceGreaterThanOrEqual( uint32_t a,
                                                              uint32_t b )
    {
        BaseType_t xResult = pdFALSE;

        /* Test if a >= b */
        if( ( ( a - b ) & 0x80000000U ) == 0U )
        {
            xResult = pdTRUE;
        }

        return xResult;
    }
/*-----------------------------------------------------------*/

    #if ( ipconfigUSE_TCP_WIN == 1 )
        static portINLINE void vListInsertFifo( List_t * const pxList,
                                                ListItem_t * const pxNewListItem );

/**
 * @brief Insert the given item in the list in FIFO manner.
 *
 * @param[in] pxList The list in which the item is to inserted.
 * @param[in] pxNewListItem The item to be inserted.
 */
        static portINLINE void vListInsertFifo( List_t * const pxList,
                                                ListItem_t * const pxNewListItem )
        {
            vListInsertGeneric( pxList, pxNewListItem, &pxList->xListEnd );
        }
    #endif
/*-----------------------------------------------------------*/

    static portINLINE void vTCPTimerSet( TCPTimer_t * pxTimer );

/**
 * @brief Set the timer's "born" time.
 *
 * @param[in] pxTimer The TCP timer.
 */
    static portINLINE void vTCPTimerSet( TCPTimer_t * pxTimer )
    {
        pxTimer->uxBorn = xTaskGetTickCount();
    }
/*-----------------------------------------------------------*/

    static portINLINE uint32_t ulTimerGetAge( const TCPTimer_t * pxTimer );

/**
 * @brief Get the timer age in milliseconds.
 *
 * @param[in] pxTimer The timer whose age is to be fetched.
 *
 * @return The time in milliseconds since the timer was born.
 */
    static portINLINE uint32_t ulTimerGetAge( const TCPTimer_t * pxTimer )
    {
        TickType_t uxNow = xTaskGetTickCount();
        TickType_t uxDiff = uxNow - pxTimer->uxBorn;

        return ( uint32_t ) ( uxDiff * portTICK_PERIOD_MS );
    }
/*-----------------------------------------------------------*/

/**
 * @brief Insert a new list item into a list.
 *
 * @param[in] pxList The list in which the item is to be inserted.
 * @param[in] pxNewListItem The item to be inserted.
 * @param[in] pxWhere Where should the item be inserted.
 */
    #if ( ipconfigUSE_TCP_WIN == 1 )
        static void vListInsertGeneric( List_t * const pxList,
                                        ListItem_t * const pxNewListItem,
                                        MiniListItem_t * pxWhere )
        {
            /* Insert a new list item into pxList, it does not sort the list,
             * but it puts the item just before xListEnd, so it will be the last item
             * returned by listGET_HEAD_ENTRY() */

            /* MISRA Ref 11.3.1 [Misaligned access] */
            /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
            /* coverity[misra_c_2012_rule_11_3_violation] */
            pxNewListItem->pxNext = ( ( ListItem_t * ) pxWhere );

            pxNewListItem->pxPrevious = pxWhere->pxPrevious;
            pxWhere->pxPrevious->pxNext = pxNewListItem;
            pxWhere->pxPrevious = pxNewListItem;

            /* Remember which list the item is in. */
            listLIST_ITEM_CONTAINER( pxNewListItem ) = ( struct xLIST * configLIST_VOLATILE ) pxList;

            ( pxList->uxNumberOfItems )++;
        }
    #endif /* if ( ipconfigUSE_TCP_WIN == 1 ) */
/*-----------------------------------------------------------*/

    #if ( ipconfigUSE_TCP_WIN == 1 )

/**
 * @brief Creates a pool of 'ipconfigTCP_WIN_SEG_COUNT' sector buffers. Should be called once only.
 *
 * @return When the allocation was successful: pdPASS, otherwise pdFAIL.
 */
        static BaseType_t prvCreateSectors( void )
        {
            BaseType_t xIndex;
            BaseType_t xReturn;

            /* Allocate space for 'xTCPSegments' and store them in 'xSegmentList'. */

            vListInitialise( &xSegmentList );
            xTCPSegments = ( ( TCPSegment_t * ) pvPortMallocLarge( ( size_t ) ipconfigTCP_WIN_SEG_COUNT * sizeof( xTCPSegments[ 0 ] ) ) );

            if( xTCPSegments == NULL )
            {
                FreeRTOS_debug_printf( ( "prvCreateSectors: malloc %u failed\n",
                                         ( unsigned ) ( ipconfigTCP_WIN_SEG_COUNT * sizeof( xTCPSegments[ 0 ] ) ) ) );

                xReturn = pdFAIL;
            }
            else
            {
                /* Clear the allocated space. */
                ( void ) memset( xTCPSegments, 0, ( size_t ) ipconfigTCP_WIN_SEG_COUNT * sizeof( xTCPSegments[ 0 ] ) );

                for( xIndex = 0; xIndex < ipconfigTCP_WIN_SEG_COUNT; xIndex++ )
                {
                    /* Could call vListInitialiseItem here but all data has been
                    * nulled already.  Set the owner to a segment descriptor. */

                    #if ( configUSE_LIST_DATA_INTEGRITY_CHECK_BYTES == 1 )
                    {
                        vListInitialiseItem( &( xTCPSegments[ xIndex ].xSegmentItem ) );
                        vListInitialiseItem( &( xTCPSegments[ xIndex ].xQueueItem ) );
                    }
                    #endif

                    listSET_LIST_ITEM_OWNER( &( xTCPSegments[ xIndex ].xSegmentItem ), ( void * ) &( xTCPSegments[ xIndex ] ) );
                    listSET_LIST_ITEM_OWNER( &( xTCPSegments[ xIndex ].xQueueItem ), ( void * ) &( xTCPSegments[ xIndex ] ) );

                    /* And add it to the pool of available segments */
                    vListInsertFifo( &xSegmentList, &( xTCPSegments[ xIndex ].xSegmentItem ) );
                }

                xReturn = pdPASS;
            }

            return xReturn;
        }
    #endif /* ipconfigUSE_TCP_WIN == 1 */
/*-----------------------------------------------------------*/

    #if ( ipconfigUSE_TCP_WIN == 1 )

/**
 * @brief Find a segment with a given sequence number in the list of received segments.
 *
 * @param[in] pxWindow The descriptor of the TCP sliding windows.
 * @param[in] ulSequenceNumber the sequence number to look-up
 *
 * @return The address of the segment descriptor found, or NULL when not found.
 */
        static TCPSegment_t * xTCPWindowRxFind( const TCPWindow_t * pxWindow,
                                                uint32_t ulSequenceNumber )
        {
            const ListItem_t * pxIterator;
            const ListItem_t * pxEnd;
            TCPSegment_t * pxSegment, * pxReturn = NULL;

            /* Find a segment with a given sequence number in the list of received
             * segments. */

            /* MISRA Ref 11.3.1 [Misaligned access] */
/* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
            /* coverity[misra_c_2012_rule_11_3_violation] */
            pxEnd = ( ( const ListItem_t * ) &( pxWindow->xRxSegments.xListEnd ) );

            for( pxIterator = listGET_NEXT( pxEnd );
                 pxIterator != pxEnd;
                 pxIterator = listGET_NEXT( pxIterator ) )
            {
                pxSegment = ( ( TCPSegment_t * ) listGET_LIST_ITEM_OWNER( pxIterator ) );

                if( pxSegment->ulSequenceNumber == ulSequenceNumber )
                {
                    pxReturn = pxSegment;
                    break;
                }
            }

            return pxReturn;
        }
    #endif /* ipconfigUSE_TCP_WIN == 1 */
/*-----------------------------------------------------------*/

    #if ( ipconfigUSE_TCP_WIN == 1 )

/**
 * @brief Allocate a new segment object, either for transmission or reception.
 *
 * @param[in] pxWindow The descriptor of the TCP sliding windows.
 * @param[in] ulSequenceNumber The sequence number.
 * @param[in] lCount The number of bytes stored in this segment.
 * @param[in] xIsForRx True when this is a reception segment.
 *
 * @return Allocate and initialise a segment descriptor, or NULL when none was available.
 */
        static TCPSegment_t * xTCPWindowNew( TCPWindow_t * pxWindow,
                                             uint32_t ulSequenceNumber,
                                             int32_t lCount,
                                             BaseType_t xIsForRx )
        {
            TCPSegment_t * pxSegment;
            ListItem_t * pxItem;

            /* Allocate a new segment.  The socket will borrow all segments from a
             * common pool: 'xSegmentList', which is a list of 'TCPSegment_t' */
            if( listLIST_IS_EMPTY( &xSegmentList ) != pdFALSE )
            {
                /* If the TCP-stack runs out of segments, you might consider
                 * increasing 'ipconfigTCP_WIN_SEG_COUNT'. */
                FreeRTOS_debug_printf( ( "xTCPWindow%cxNew: Error: all segments occupied\n", ( xIsForRx != 0 ) ? 'R' : 'T' ) );
                pxSegment = NULL;
            }
            else
            {
                /* Pop the item at the head of the list.  Semaphore protection is
                * not required as only the IP task will call these functions.  */
                pxItem = ( ListItem_t * ) listGET_HEAD_ENTRY( &xSegmentList );
                pxSegment = ( ( TCPSegment_t * ) listGET_LIST_ITEM_OWNER( pxItem ) );

                configASSERT( pxItem != NULL );
                configASSERT( pxSegment != NULL );

                /* Remove the item from xSegmentList. */
                ( void ) uxListRemove( pxItem );

                /* Add it to either the connections' Rx or Tx queue. */
                if( xIsForRx != 0 )
                {
                    vListInsertFifo( &pxWindow->xRxSegments, pxItem );
                }
                else
                {
                    vListInsertFifo( &pxWindow->xTxSegments, pxItem );
                }

                /* And set the segment's timer to zero */
                vTCPTimerSet( &pxSegment->xTransmitTimer );

                pxSegment->u.ulFlags = 0;
                pxSegment->u.bits.bIsForRx = ( xIsForRx != 0 ) ? 1U : 0U;
                pxSegment->lMaxLength = lCount;
                pxSegment->lDataLength = lCount;
                pxSegment->ulSequenceNumber = ulSequenceNumber;
                #if ( ipconfigHAS_DEBUG_PRINTF != 0 )
                {
                    static UBaseType_t xLowestLength = ipconfigTCP_WIN_SEG_COUNT;
                    UBaseType_t xLength = listCURRENT_LIST_LENGTH( &xSegmentList );

                    if( xLowestLength > xLength )
                    {
                        xLowestLength = xLength;
                    }
                }
                #endif /* ipconfigHAS_DEBUG_PRINTF */
            }

            return pxSegment;
        }
    #endif /* ipconfigUSE_TCP_WIN == 1 */
/*-----------------------------------------------------------*/

    #if ( ipconfigUSE_TCP_WIN == 1 )

/**
 * @brief See if the peer has more packets for this node, before allowing to shut down the connection.
 *
 * @param[in] pxWindow The descriptor of the TCP sliding windows.
 *
 * @return pdTRUE if the connection can be closed. Else, pdFALSE.
 */
        BaseType_t xTCPWindowRxEmpty( const TCPWindow_t * pxWindow )
        {
            BaseType_t xReturn;

            /* When the peer has a close request (FIN flag), the driver will check
             * if there are missing packets in the Rx-queue.  It will accept the
             * closure of the connection if both conditions are true:
             * - the Rx-queue is empty
             * - the highest Rx sequence number has been ACK'ed */
            if( listLIST_IS_EMPTY( ( &pxWindow->xRxSegments ) ) == pdFALSE )
            {
                /* Rx data has been stored while earlier packets were missing. */
                xReturn = pdFALSE;
            }
            else if( xSequenceGreaterThanOrEqual( pxWindow->rx.ulCurrentSequenceNumber + 1U, pxWindow->rx.ulHighestSequenceNumber ) != pdFALSE )
            {
                /* No Rx packets are being stored and the highest sequence number
                 * that has been received has been ACKed. */
                xReturn = pdTRUE;
            }
            else
            {
                FreeRTOS_debug_printf( ( "xTCPWindowRxEmpty: cur %u highest %u (empty)\n",
                                         ( unsigned ) ( pxWindow->rx.ulCurrentSequenceNumber - pxWindow->rx.ulFirstSequenceNumber ),
                                         ( unsigned ) ( pxWindow->rx.ulHighestSequenceNumber - pxWindow->rx.ulFirstSequenceNumber ) ) );
                xReturn = pdFALSE;
            }

            return xReturn;
        }
    #endif /* ipconfigUSE_TCP_WIN == 1 */
/*-----------------------------------------------------------*/

    #if ( ipconfigUSE_TCP_WIN == 1 )

/**
 * @brief Remove the head item of a list (generic function).
 *
 * @param[in] pxList The list of segment descriptors.
 *
 * @return The address of the segment descriptor, or NULL when not found.
 */
        static TCPSegment_t * xTCPWindowGetHead( const List_t * pxList )
        {
            TCPSegment_t * pxSegment;
            ListItem_t * pxItem;

            /* Detaches and returns the head of a queue. */
            if( listLIST_IS_EMPTY( pxList ) != pdFALSE )
            {
                pxSegment = NULL;
            }
            else
            {
                pxItem = ( ListItem_t * ) listGET_HEAD_ENTRY( pxList );
                pxSegment = ( ( TCPSegment_t * ) listGET_LIST_ITEM_OWNER( pxItem ) );

                ( void ) uxListRemove( pxItem );
            }

            return pxSegment;
        }
    #endif /* ipconfigUSE_TCP_WIN == 1 */
/*-----------------------------------------------------------*/

    #if ( ipconfigUSE_TCP_WIN == 1 )

/**
 * @brief Return the head item of a list (generic function).
 *
 * @param[in] pxList The list of segment descriptors.
 *
 * @return The address of the segment descriptor, or NULL when the list is empty.
 */
        static TCPSegment_t * xTCPWindowPeekHead( const List_t * pxList )
        {
            const ListItem_t * pxItem;
            TCPSegment_t * pxReturn;

            /* Returns the head of a queue but it won't be detached. */
            if( listLIST_IS_EMPTY( pxList ) != pdFALSE )
            {
                pxReturn = NULL;
            }
            else
            {
                pxItem = ( ListItem_t * ) listGET_HEAD_ENTRY( pxList );
                pxReturn = ( ( TCPSegment_t * ) listGET_LIST_ITEM_OWNER( pxItem ) );
            }

            return pxReturn;
        }
    #endif /* ipconfigUSE_TCP_WIN == 1 */
/*-----------------------------------------------------------*/

    #if ( ipconfigUSE_TCP_WIN == 1 )

/**
 * @brief Release a segment object, return it to the list of available segment holders.
 *
 * @param[in] pxSegment The segment descriptor that must be freed.
 */
        static void vTCPWindowFree( TCPSegment_t * pxSegment )
        {
            /*  Free entry pxSegment because it's not used any more.  The ownership
             * will be passed back to the segment pool.
             *
             * Unlink it from one of the queues, if any. */
            if( listLIST_ITEM_CONTAINER( &( pxSegment->xQueueItem ) ) != NULL )
            {
                ( void ) uxListRemove( &( pxSegment->xQueueItem ) );
            }

            pxSegment->ulSequenceNumber = 0U;
            pxSegment->lDataLength = 0;
            pxSegment->u.ulFlags = 0U;

            /* Take it out of xRxSegments/xTxSegments */
            if( listLIST_ITEM_CONTAINER( &( pxSegment->xSegmentItem ) ) != NULL )
            {
                ( void ) uxListRemove( &( pxSegment->xSegmentItem ) );
            }

            /* Return it to xSegmentList */
            vListInsertFifo( &xSegmentList, &( pxSegment->xSegmentItem ) );
        }
    #endif /* ipconfigUSE_TCP_WIN == 1 */
/*-----------------------------------------------------------*/

    #if ( ipconfigUSE_TCP_WIN == 1 )

/**
 * @brief Return all segment descriptor to the poll of descriptors, before deleting a socket.
 *
 * @param[in] pxWindow The descriptor of the TCP sliding windows.
 */
        void vTCPWindowDestroy( TCPWindow_t const * pxWindow )
        {
            const List_t * pxSegments;
            BaseType_t xRound;
            TCPSegment_t * pxSegment;

            /*  Destroy a window.  A TCP window doesn't serve any more.  Return all
             * owned segments to the pool.  In order to save code, it will make 2 rounds,
             * one to remove the segments from xRxSegments, and a second round to clear
             * xTxSegments*/
            for( xRound = 0; xRound < 2; xRound++ )
            {
                if( xRound != 0 )
                {
                    pxSegments = &( pxWindow->xRxSegments );
                }
                else
                {
                    pxSegments = &( pxWindow->xTxSegments );
                }

                if( listLIST_IS_INITIALISED( pxSegments ) )
                {
                    while( listCURRENT_LIST_LENGTH( pxSegments ) > 0U )
                    {
                        pxSegment = ( ( TCPSegment_t * ) listGET_OWNER_OF_HEAD_ENTRY( pxSegments ) );
                        vTCPWindowFree( pxSegment );
                    }
                }
            }
        }
    #endif /* ipconfigUSE_TCP_WIN == 1 */
/*-----------------------------------------------------------*/

/**
 * @brief Create a window for TCP.
 *
 * @param[in] pxWindow The window to be created.
 * @param[in] ulRxWindowLength The length of the receive window.
 * @param[in] ulTxWindowLength The length of the transmit window.
 * @param[in] ulAckNumber The first ACK number.
 * @param[in] ulSequenceNumber The first sequence number.
 * @param[in] ulMSS The MSS of the connection.
 */
    BaseType_t xTCPWindowCreate( TCPWindow_t * pxWindow,
                                 uint32_t ulRxWindowLength,
                                 uint32_t ulTxWindowLength,
                                 uint32_t ulAckNumber,
                                 uint32_t ulSequenceNumber,
                                 uint32_t ulMSS )
    {
        BaseType_t xReturn = pdPASS;

        /* Create and initialize a window. */

        #if ( ipconfigUSE_TCP_WIN == 1 )
        {
            if( xTCPSegments == NULL )
            {
                xReturn = prvCreateSectors();
            }

            vListInitialise( &( pxWindow->xTxSegments ) );
            vListInitialise( &( pxWindow->xRxSegments ) );

            vListInitialise( &( pxWindow->xPriorityQueue ) ); /* Priority queue: segments which must be sent immediately */
            vListInitialise( &( pxWindow->xTxQueue ) );       /* Transmit queue: segments queued for transmission */
            vListInitialise( &( pxWindow->xWaitQueue ) );     /* Waiting queue:  outstanding segments */
        }
        #endif /* ipconfigUSE_TCP_WIN == 1 */

        if( xTCPWindowLoggingLevel != 0 )
        {
            FreeRTOS_debug_printf( ( "xTCPWindowCreate: for WinLen = Rx/Tx: %u/%u\n",
                                     ( unsigned ) ulRxWindowLength, ( unsigned ) ulTxWindowLength ) );
        }

        pxWindow->xSize.ulRxWindowLength = ulRxWindowLength;
        pxWindow->xSize.ulTxWindowLength = ulTxWindowLength;

        vTCPWindowInit( pxWindow, ulAckNumber, ulSequenceNumber, ulMSS );

        return xReturn;
    }
/*-----------------------------------------------------------*/

/**
 * @brief Initialise a TCP window.
 *
 * @param[in] pxWindow The window to be initialised.
 * @param[in] ulAckNumber The number of the first ACK.
 * @param[in] ulSequenceNumber The first sequence number.
 * @param[in] ulMSS The MSS of the connection.
 */
    void vTCPWindowInit( TCPWindow_t * pxWindow,
                         uint32_t ulAckNumber,
                         uint32_t ulSequenceNumber,
                         uint32_t ulMSS )
    {
        const int32_t l500ms = 500;

        pxWindow->u.ulFlags = 0U;
        pxWindow->u.bits.bHasInit = pdTRUE_UNSIGNED;

        if( ulMSS != 0U )
        {
            if( pxWindow->usMSSInit != 0U )
            {
                pxWindow->usMSSInit = ( uint16_t ) ulMSS;
            }

            if( ( ulMSS < ( uint32_t ) pxWindow->usMSS ) || ( pxWindow->usMSS == 0U ) )
            {
                pxWindow->xSize.ulRxWindowLength = ( pxWindow->xSize.ulRxWindowLength / ulMSS ) * ulMSS;
                pxWindow->usMSS = ( uint16_t ) ulMSS;
            }
        }

        #if ( ipconfigUSE_TCP_WIN == 0 )
        {
            pxWindow->xTxSegment.lMaxLength = ( int32_t ) pxWindow->usMSS;
        }
        #endif /* ipconfigUSE_TCP_WIN == 1 */

        /*Start with a timeout of 2 * 500 ms (1 sec). */
        pxWindow->lSRTT = l500ms;

        /* Just for logging, to print relative sequence numbers. */
        pxWindow->rx.ulFirstSequenceNumber = ulAckNumber;

        /* The segment asked for in the next transmission. */
        pxWindow->rx.ulCurrentSequenceNumber = ulAckNumber;

        /* The right-hand side of the receive window. */
        pxWindow->rx.ulHighestSequenceNumber = ulAckNumber;

        pxWindow->tx.ulFirstSequenceNumber = ulSequenceNumber;

        /* The segment asked for in next transmission. */
        pxWindow->tx.ulCurrentSequenceNumber = ulSequenceNumber;

        /* The sequence number given to the next outgoing byte to be added is
         * maintained by lTCPWindowTxAdd(). */
        pxWindow->ulNextTxSequenceNumber = ulSequenceNumber;

        /* The right-hand side of the transmit window. */
        pxWindow->tx.ulHighestSequenceNumber = ulSequenceNumber;
        pxWindow->ulOurSequenceNumber = ulSequenceNumber;
    }
/*-----------------------------------------------------------*/

    #if ( ipconfigUSE_TCP_WIN == 1 )

/**
 * @brief Free the space occupied by the pool of segment descriptors, normally never used
 */
        void vTCPSegmentCleanup( void )
        {
            /* Free and clear the TCP segments pointer. This function should only be called
             * once FreeRTOS+TCP will no longer be used. No thread-safety is provided for this
             * function. */
            if( xTCPSegments != NULL )
            {
                vPortFreeLarge( xTCPSegments );
                xTCPSegments = NULL;
            }
        }
    #endif /* ipconfigUSE_TCP_WIN == 1 */
/*-----------------------------------------------------------*/

/*=============================================================================
 *
 *                ######        #    #
 *                 #    #       #    #
 *                 #    #       #    #
 *                 #    #        ####
 *                 ######         ##
 *                 #  ##         ####
 *                 #   #        #    #
 *                 #    #       #    #
 *                ###  ##       #    #
 * Rx functions
 *
 *=============================================================================*/

    #if ( ipconfigUSE_TCP_WIN == 1 )

/**
 * @brief A expected segment has been received, see if there is overlap with earlier segments.
 *
 * @param[in] pxWindow The descriptor of the TCP sliding windows.
 * @param[in] ulSequenceNumber The sequence number of the segment that was received.
 * @param[in] ulLength The number of bytes that were received.
 *
 * @return The first segment descriptor involved, or NULL when no matching descriptor was found.
 */
        static TCPSegment_t * xTCPWindowRxConfirm( const TCPWindow_t * pxWindow,
                                                   uint32_t ulSequenceNumber,
                                                   uint32_t ulLength )
        {
            TCPSegment_t * pxBest = NULL;
            const ListItem_t * pxIterator;
            uint32_t ulNextSequenceNumber = ulSequenceNumber + ulLength;

            /* MISRA Ref 11.3.1 [Misaligned access] */
/* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
            /* coverity[misra_c_2012_rule_11_3_violation] */
            const ListItem_t * pxEnd = ( ( const ListItem_t * ) &( pxWindow->xRxSegments.xListEnd ) );
            TCPSegment_t * pxSegment;

            /* A segment has been received with sequence number 'ulSequenceNumber',
             * where 'ulCurrentSequenceNumber == ulSequenceNumber', which means that
             * exactly this segment was expected.  xTCPWindowRxConfirm() will check if
             * there is already another segment with a sequence number between (ulSequenceNumber)
             * and (ulSequenceNumber+ulLength).  Normally none will be found, because
             * the next RX segment should have a sequence number equal to
             * '(ulSequenceNumber+ulLength)'. */

            /* Iterate through all RX segments that are stored: */
            for( pxIterator = listGET_NEXT( pxEnd );
                 pxIterator != pxEnd;
                 pxIterator = listGET_NEXT( pxIterator ) )
            {
                pxSegment = ( ( TCPSegment_t * ) listGET_LIST_ITEM_OWNER( pxIterator ) );

                /* And see if there is a segment for which:
                 * 'ulSequenceNumber' <= 'pxSegment->ulSequenceNumber' < 'ulNextSequenceNumber'
                 * If there are more matching segments, the one with the lowest sequence number
                 * shall be taken */
                if( ( xSequenceGreaterThanOrEqual( pxSegment->ulSequenceNumber, ulSequenceNumber ) != 0 ) &&
                    ( xSequenceLessThan( pxSegment->ulSequenceNumber, ulNextSequenceNumber ) != 0 ) )
                {
                    if( ( pxBest == NULL ) || ( xSequenceLessThan( pxSegment->ulSequenceNumber, pxBest->ulSequenceNumber ) != 0 ) )
                    {
                        pxBest = pxSegment;
                    }
                }
            }

            if( ( pxBest != NULL ) &&
                ( ( pxBest->ulSequenceNumber != ulSequenceNumber ) || ( pxBest->lDataLength != ( int32_t ) ulLength ) ) )
            {
                FreeRTOS_debug_printf( ( "xTCPWindowRxConfirm[%u]: search %u (+%u=%u) found %u (+%d=%u)\n",
                                         pxWindow->usPeerPortNumber,
                                         ( unsigned ) ( ulSequenceNumber - pxWindow->rx.ulFirstSequenceNumber ),
                                         ( unsigned ) ulLength,
                                         ( unsigned ) ( ulSequenceNumber + ulLength - pxWindow->rx.ulFirstSequenceNumber ),
                                         ( unsigned ) ( pxBest->ulSequenceNumber - pxWindow->rx.ulFirstSequenceNumber ),
                                         ( int ) pxBest->lDataLength,
                                         ( unsigned ) ( pxBest->ulSequenceNumber + ( ( uint32_t ) pxBest->lDataLength ) - pxWindow->rx.ulFirstSequenceNumber ) ) );
            }

            return pxBest;
        }
    #endif /* ipconfigUSE_TCP_WIN == 1 */
/*-----------------------------------------------------------*/

    #if ( ipconfigUSE_TCP_WIN == 1 )

/**
 * @brief Data has been received with the correct ( expected  ) sequence number.
 *        It can be added to the RX stream buffer.
 * @param[in] pxWindow The TCP sliding window data of the socket.
 * @param[in] ulLength The number of bytes that can be added.
 */
        static void prvTCPWindowRx_ExpectedRX( TCPWindow_t * pxWindow,
                                               uint32_t ulLength )
        {
            uint32_t ulSequenceNumber = pxWindow->rx.ulCurrentSequenceNumber;
            uint32_t ulCurrentSequenceNumber = ulSequenceNumber + ulLength;

            if( listCURRENT_LIST_LENGTH( &( pxWindow->xRxSegments ) ) != 0U )
            {
                uint32_t ulSavedSequenceNumber = ulCurrentSequenceNumber;
                TCPSegment_t * pxFound;

                /* Clean up all sequence received between ulSequenceNumber and ulSequenceNumber + ulLength since they are duplicated.
                 * If the server is forced to retransmit packets several time in a row it might send a batch of concatenated packet for speed.
                 * So we cannot rely on the packets between ulSequenceNumber and ulSequenceNumber + ulLength to be sequential and it is better to just
                 * clean them out. */
                do
                {
                    pxFound = xTCPWindowRxConfirm( pxWindow, ulSequenceNumber, ulLength );

                    if( pxFound != NULL )
                    {
                        /* Remove it because it will be passed to user directly. */
                        vTCPWindowFree( pxFound );
                    }
                } while( pxFound != NULL );

                /*  Check for following segments that are already in the
                 * queue and increment ulCurrentSequenceNumber. */
                for( ; ; )
                {
                    pxFound = xTCPWindowRxFind( pxWindow, ulCurrentSequenceNumber );

                    if( pxFound == NULL )
                    {
                        break;
                    }

                    ulCurrentSequenceNumber += ( uint32_t ) pxFound->lDataLength;

                    /* As all packet below this one have been passed to the
                     * user it can be discarded. */
                    vTCPWindowFree( pxFound );
                }

                if( ulSavedSequenceNumber != ulCurrentSequenceNumber )
                {
                    /*  After the current data-package, there is more data
                     * to be popped. */
                    pxWindow->ulUserDataLength = ulCurrentSequenceNumber - ulSavedSequenceNumber;

                    if( xTCPWindowLoggingLevel >= 1 )
                    {
                        FreeRTOS_debug_printf( ( "lTCPWindowRxCheck[%u,%u]: retran %u (Found %u bytes at %u cnt %d)\n",
                                                 pxWindow->usPeerPortNumber,
                                                 pxWindow->usOurPortNumber,
                                                 ( unsigned ) ( ulSequenceNumber - pxWindow->rx.ulFirstSequenceNumber ),
                                                 ( unsigned ) pxWindow->ulUserDataLength,
                                                 ( unsigned ) ( ulSavedSequenceNumber - pxWindow->rx.ulFirstSequenceNumber ),
                                                 ( int ) listCURRENT_LIST_LENGTH( &pxWindow->xRxSegments ) ) );
                    }
                }
            }

            pxWindow->rx.ulCurrentSequenceNumber = ulCurrentSequenceNumber;
        }
    #endif /* ipconfigUSE_TCP_WIN == 1 */
/*-----------------------------------------------------------*/

    #if ( ipconfigUSE_TCP_WIN == 1 )

/**
 * @brief Data has been received with a non-expected sequence number.
 *        This function will check if the RX data can be accepted.
 * @param[in] pxWindow The TCP sliding window data of the socket.
 * @param[in] ulSequenceNumber The sequence number at which the data should be placed.
 * @param[in] ulLength The number of bytes that can be added.
 * @return Return -1 if the data must be refused, otherwise it returns the
 *         offset ( from the head ) at which the data can be placed.
 */
        static int32_t prvTCPWindowRx_UnexpectedRX( TCPWindow_t * pxWindow,
                                                    uint32_t ulSequenceNumber,
                                                    uint32_t ulLength )
        {
            int32_t lReturn = -1;
            uint32_t ulLast = ulSequenceNumber + ulLength;
            uint32_t ulCurrentSequenceNumber = pxWindow->rx.ulCurrentSequenceNumber;
            const TCPSegment_t * pxFound;

            /* See if there is more data in a contiguous block to make the
             * SACK describe a longer range of data. */

            /* SACK's may also be delayed for a short period
             * This is useful because subsequent packets will be SACK'd with
             * single one message
             */
            for( ; ; )
            {
                pxFound = xTCPWindowRxFind( pxWindow, ulLast );

                if( pxFound == NULL )
                {
                    break;
                }

                ulLast += ( uint32_t ) pxFound->lDataLength;
            }

            if( xTCPWindowLoggingLevel >= 1 )
            {
                FreeRTOS_debug_printf( ( "lTCPWindowRxCheck[%d,%d]: seqnr %u exp %u (dist %d) SACK to %u\n",
                                         ( int ) pxWindow->usPeerPortNumber,
                                         ( int ) pxWindow->usOurPortNumber,
                                         ( unsigned ) ( ulSequenceNumber - pxWindow->rx.ulFirstSequenceNumber ),
                                         ( unsigned ) ( ulCurrentSequenceNumber - pxWindow->rx.ulFirstSequenceNumber ),
                                         ( int ) ( ulSequenceNumber - ulCurrentSequenceNumber ), /* want this signed */
                                         ( unsigned ) ( ulLast - pxWindow->rx.ulFirstSequenceNumber ) ) );
            }

            /* Now prepare the SACK message.
             * Code OPTION_CODE_SINGLE_SACK already in network byte order. */
            pxWindow->ulOptionsData[ 0 ] = OPTION_CODE_SINGLE_SACK;

            /* First sequence number that we received. */
            pxWindow->ulOptionsData[ 1 ] = FreeRTOS_htonl( ulSequenceNumber );

            /* Last + 1 */
            pxWindow->ulOptionsData[ 2 ] = FreeRTOS_htonl( ulLast );

            /* Which make 12 (3*4) option bytes. */
            pxWindow->ucOptionLength = ( uint8_t ) ( 3U * sizeof( pxWindow->ulOptionsData[ 0 ] ) );

            pxFound = xTCPWindowRxFind( pxWindow, ulSequenceNumber );

            if( pxFound != NULL )
            {
                /* This out-of-sequence packet has been received for a
                 * second time.  It is already stored but do send a SACK
                 * again. */
                /* A negative value will be returned to indicate than error. */
            }
            else
            {
                pxFound = xTCPWindowRxNew( pxWindow, ulSequenceNumber, ( int32_t ) ulLength );

                if( pxFound == NULL )
                {
                    /* Can not send a SACK, because the segment cannot be
                     * stored. */
                    pxWindow->ucOptionLength = 0U;

                    /* Needs to be stored but there is no segment
                     * available. A negative value will be returned. */
                }
                else
                {
                    uint32_t ulIntermediateResult;

                    if( xTCPWindowLoggingLevel != 0 )
                    {
                        FreeRTOS_debug_printf( ( "lTCPWindowRxCheck[%u,%u]: seqnr %u (cnt %u)\n",
                                                 pxWindow->usPeerPortNumber,
                                                 pxWindow->usOurPortNumber,
                                                 ( unsigned ) ( ulSequenceNumber - pxWindow->rx.ulFirstSequenceNumber ),
                                                 ( unsigned ) listCURRENT_LIST_LENGTH( &pxWindow->xRxSegments ) ) );
                    }

                    /* Return a positive value.  The packet may be accepted
                    * and stored but an earlier packet is still missing. */
                    ulIntermediateResult = ulSequenceNumber - ulCurrentSequenceNumber;
                    lReturn = ( int32_t ) ulIntermediateResult;
                }
            }

            return lReturn;
        }
    #endif /* ipconfigUSE_TCP_WIN == 1 */
/*-----------------------------------------------------------*/

    #if ( ipconfigUSE_TCP_WIN == 1 )

/**
 * @brief Check what to do with a new incoming packet: store or ignore.
 *
 * @param[in] pxWindow The descriptor of the TCP sliding windows.
 * @param[in] ulSequenceNumber The sequence number of the packet received.
 * @param[in] ulLength The number of bytes received.
 * @param[in] ulSpace The available space in the RX stream buffer.
 * @param[out] pulSkipCount the number of bytes to skip in the receive buffer.
 *
 * @return 0 or positive value indicating the offset at which the packet is to
 *         be stored, -1 if the packet is to be ignored.
 */
        int32_t lTCPWindowRxCheck( TCPWindow_t * pxWindow,
                                   uint32_t ulSequenceNumber,
                                   uint32_t ulLength,
                                   uint32_t ulSpace,
                                   uint32_t * pulSkipCount )
        {
            uint32_t ulCurrentSequenceNumber;
            uint32_t ulIntermediateResult;
            int32_t lReturn = -1;
            int32_t lStartDistance;
            int32_t lLastDistance;
            uint32_t ulLast;
            uint32_t ulRxSequenceNumber = ulSequenceNumber;
            uint32_t ulRxLength = ulLength;

            /* If lTCPWindowRxCheck( ) returns == 0, the packet will be passed
             * directly to user (segment is expected).  If it returns a positive
             * number, an earlier packet is missing, but this packet may be stored.
             * If negative, the packet has already been stored, or it is out-of-order,
             * or there is not enough space.
             *
             * As a side-effect, pxWindow->ulUserDataLength will get set to non-zero,
             * if more Rx data may be passed to the user after this packet. */

            /* Only in an exceptional case, where a packet starts before
             * ulCurrentSequenceNumber, and ends after it, the skip-count
             * will be set. See below. */

            *( pulSkipCount ) = 0U;

            ulCurrentSequenceNumber = pxWindow->rx.ulCurrentSequenceNumber;

            ulLast = ulRxSequenceNumber + ulRxLength;
            ulIntermediateResult = ulLast - ulCurrentSequenceNumber;
            /* The cast from unsigned long to signed long is on purpose. */
            lLastDistance = ( int32_t ) ulIntermediateResult;

            ulIntermediateResult = ulRxSequenceNumber - ulCurrentSequenceNumber;
            lStartDistance = ( int32_t ) ulIntermediateResult;

            if( ( lStartDistance < 0 ) && ( lLastDistance > 0 ) )
            {
                FreeRTOS_debug_printf( ( "lTCPWindowRxCheck: Received +%u bytes for %u, only using %d\n",
                                         ( unsigned ) ulRxLength,
                                         ( unsigned ) ( ulRxSequenceNumber - pxWindow->rx.ulFirstSequenceNumber ),
                                         ( int ) lLastDistance ) );
                /* Increase the sequence number, decrease the length. */
                ulRxSequenceNumber += ( uint32_t ) ( -lStartDistance );
                ulRxLength += ( uint32_t ) lStartDistance;

                /* Tell the caller that the first 'pulSkipCount' bytes don't
                 * need to be stored. */
                *( pulSkipCount ) = ( uint32_t ) ( -lStartDistance );
            }

            /* For Selective Ack (SACK), used when out-of-sequence data come in. */
            pxWindow->ucOptionLength = 0U;

            /* Non-zero if TCP-windows contains data which must be popped. */
            pxWindow->ulUserDataLength = 0U;

            if( ulCurrentSequenceNumber == ulRxSequenceNumber )
            {
                /* This is the packet with the lowest sequence number we're waiting
                 * for.  It can be passed directly to the rx stream. */
                if( ulRxLength > ulSpace )
                {
                    FreeRTOS_debug_printf( ( "lTCPWindowRxCheck: Refuse %u bytes, due to lack of space (%u)\n", ( unsigned ) ulRxLength, ( unsigned ) ulSpace ) );
                }
                else
                {
                    /* Packet was expected, may be passed directly to the socket
                     * buffer or application.  Store the packet at offset 0. */
                    prvTCPWindowRx_ExpectedRX( pxWindow, ulRxLength );
                    lReturn = 0;
                }
            }
            else if( ulCurrentSequenceNumber == ( ulRxSequenceNumber + 1U ) )
            {
                /* Looks like a TCP keep-alive message.  Do not accept/store Rx data
                 * ulUserDataLength = 0. Not packet out-of-sync.  Just reply to it. */
            }
            else
            {
                /* The packet is not the one expected.  See if it falls within the Rx
                 * window so it can be stored. */

                /*  An "out-of-sequence" segment was received, must have missed one.
                 * Prepare a SACK (Selective ACK). */

                if( lLastDistance <= 0 )
                {
                    /* An earlier packet has been received, must be a retransmission of a
                     * packet that has been accepted already.  No need to send out a
                     * Selective ACK (SACK). */
                }
                else if( lLastDistance > ( int32_t ) ulSpace )
                {
                    /* The new segment is ahead of rx.ulCurrentSequenceNumber.  The
                     * sequence number of this packet is too far ahead, ignore it. */
                    FreeRTOS_debug_printf( ( "lTCPWindowRxCheck: Refuse %d+%u bytes, due to lack of space (%u)\n",
                                             ( int ) lLastDistance,
                                             ( unsigned ) ulRxLength,
                                             ( unsigned ) ulSpace ) );
                }
                else
                {
                    lReturn = prvTCPWindowRx_UnexpectedRX( pxWindow, ulRxSequenceNumber, ulRxLength );
                }
            }

            return lReturn;
        }
    #endif /* ipconfigUSE_TCP_WIN == 1 */
/*-----------------------------------------------------------*/

/*=============================================================================
 *
 *                    #########   #    #
 *                    #   #   #   #    #
 *                        #       #    #
 *                        #        ####
 *                        #         ##
 *                        #        ####
 *                        #       #    #
 *                        #       #    #
 *                      #####     #    #
 *
 * Tx functions
 *
 *=============================================================================*/

    #if ( ipconfigUSE_TCP_WIN == 1 )

/**
 * @brief Increment the position in a circular buffer of size 'lMax'.
 *
 * @param[in] lPosition The current index in the buffer.
 * @param[in] lMax The total number of items in this buffer.
 * @param[in] lCount The number of bytes that must be advanced.
 *
 * @return The new incremented position, or "( lPosition + lCount ) % lMax".
 */
        static int32_t lTCPIncrementTxPosition( int32_t lPosition,
                                                int32_t lMax,
                                                int32_t lCount )
        {
            int32_t lReturn;


            /* +TCP stores data in circular buffers.  Calculate the next position to
             * store. */
            lReturn = lPosition + lCount;

            if( lReturn >= lMax )
            {
                lReturn -= lMax;
            }

            return lReturn;
        }
    #endif /* ipconfigUSE_TCP_WIN == 1 */
/*-----------------------------------------------------------*/

    #if ( ipconfigUSE_TCP_WIN == 1 )

/**
 * @brief Adding data to a segment that was already in the TX queue.  It
 *        will be filled-up to a maximum of MSS ( maximum segment size ).
 *
 * @param[in] pxWindow The descriptor of the TCP sliding windows.
 * @param[in] pxSegment The TX segment with the highest sequence number,
 *                       i.e. the "front segment".
 * @param[in] lBytesLeft The number of bytes that must be added.
 *
 * @return lToWrite: the number of bytes added to the segment.
 */
        static int32_t prvTCPWindowTxAdd_FrontSegment( TCPWindow_t * pxWindow,
                                                       TCPSegment_t * pxSegment,
                                                       int32_t lBytesLeft )
        {
            int32_t lToWrite = FreeRTOS_min_int32( lBytesLeft, pxSegment->lMaxLength - pxSegment->lDataLength );

            pxSegment->lDataLength += lToWrite;

            if( pxSegment->lDataLength >= pxSegment->lMaxLength )
            {
                /* This segment is full, don't add more bytes. */
                pxWindow->pxHeadSegment = NULL;
            }

            /* ulNextTxSequenceNumber is the sequence number of the next byte to
             * be stored for transmission. */
            pxWindow->ulNextTxSequenceNumber += ( uint32_t ) lToWrite;

            /* Some detailed logging, for those who're interested. */
            if( ( xTCPWindowLoggingLevel >= 2 ) && ipconfigTCP_MAY_LOG_PORT( pxWindow->usOurPortNumber ) )
            {
                FreeRTOS_debug_printf( ( "lTCPWindowTxAdd: Add %4d bytes for seqNr %u len %4d (nxt %u) pos %d\n",
                                         ( int ) lBytesLeft,
                                         ( unsigned ) ( pxSegment->ulSequenceNumber - pxWindow->tx.ulFirstSequenceNumber ),
                                         ( int ) pxSegment->lDataLength,
                                         ( unsigned ) ( pxWindow->ulNextTxSequenceNumber - pxWindow->tx.ulFirstSequenceNumber ),
                                         ( int ) pxSegment->lStreamPos ) );
            }

            return lToWrite;
        }
    #endif /* ipconfigUSE_TCP_WIN == 1 */
/*-----------------------------------------------------------*/

    #if ( ipconfigUSE_TCP_WIN == 1 )

/**
 * @brief Will add data to be transmitted to the front of the segment fifo.
 *
 * @param[in] pxWindow The descriptor of the TCP sliding windows.
 * @param[in] ulLength The number of bytes that will be sent.
 * @param[in] lPosition The index in the TX stream buffer.
 * @param[in] lMax The size of the ( circular ) TX stream buffer.
 *
 * @return The number of bytes added to the sliding window for transmission.
 *
 */
        int32_t lTCPWindowTxAdd( TCPWindow_t * pxWindow,
                                 uint32_t ulLength,
                                 int32_t lPosition,
                                 int32_t lMax )
        {
            int32_t lBytesLeft = ( int32_t ) ulLength;
            int32_t lToWrite;
            int32_t lDone = 0;
            int32_t lBufferIndex = lPosition;
            TCPSegment_t * pxSegment = pxWindow->pxHeadSegment;

            /* Puts a message in the Tx-window (after buffer size has been
             * verified). */
            if( ( pxSegment != NULL ) &&
                ( pxSegment->lDataLength < pxSegment->lMaxLength ) &&
                ( pxSegment->u.bits.bOutstanding == pdFALSE_UNSIGNED ) &&
                ( pxSegment->lDataLength != 0 ) )
            {
                lToWrite = prvTCPWindowTxAdd_FrontSegment( pxWindow, pxSegment, lBytesLeft );
                lBytesLeft -= lToWrite;
                /* Increased the return value. */
                lDone += lToWrite;

                /* Calculate the next position in the circular data buffer, knowing
                 * its maximum length 'lMax'. */
                lBufferIndex = lTCPIncrementTxPosition( lBufferIndex, lMax, lToWrite );
            }

            while( lBytesLeft > 0 )
            {
                /* The current transmission segment is full, create new segments as
                 * needed. */
                pxSegment = xTCPWindowTxNew( pxWindow, pxWindow->ulNextTxSequenceNumber, ( int32_t ) pxWindow->usMSS );

                if( pxSegment != NULL )
                {
                    /* Store as many as needed, but no more than the maximum
                     * (MSS). */
                    lToWrite = FreeRTOS_min_int32( lBytesLeft, pxSegment->lMaxLength );

                    pxSegment->lDataLength = lToWrite;
                    pxSegment->lStreamPos = lBufferIndex;
                    lBytesLeft -= lToWrite;
                    lBufferIndex = lTCPIncrementTxPosition( lBufferIndex, lMax, lToWrite );
                    pxWindow->ulNextTxSequenceNumber += ( uint32_t ) lToWrite;
                    lDone += lToWrite;

                    /* Link this segment in the Tx-Queue. */
                    vListInsertFifo( &( pxWindow->xTxQueue ), &( pxSegment->xQueueItem ) );

                    /* Let 'pxHeadSegment' point to this segment if there is still
                     * space. */
                    if( pxSegment->lDataLength < pxSegment->lMaxLength )
                    {
                        pxWindow->pxHeadSegment = pxSegment;
                    }
                    else
                    {
                        pxWindow->pxHeadSegment = NULL;
                    }
                }
                else
                {
                    /* A sever situation: running out of segments for transmission.
                     * No more data can be sent at the moment. */
                    if( lDone != 0 )
                    {
                        FreeRTOS_debug_printf( ( "lTCPWindowTxAdd: Sorry all buffers full (cancel %d bytes)\n", ( int ) lBytesLeft ) );
                    }

                    break;
                }
            }

            return lDone;
        }
    #endif /* ipconfigUSE_TCP_WIN == 1 */
/*-----------------------------------------------------------*/

    #if ( ipconfigUSE_TCP_WIN == 1 )

/**
 * @brief Returns true if there are no more outstanding TX segments.
 *
 * @param[in] pxWindow The descriptor of the TCP sliding windows.
 *
 * @return pdTRUE if there are no more outstanding Tx segments, else pdFALSE.
 */
        BaseType_t xTCPWindowTxDone( const TCPWindow_t * pxWindow )
        {
            return listLIST_IS_EMPTY( ( &pxWindow->xTxSegments ) );
        }
    #endif /* ipconfigUSE_TCP_WIN == 1 */
/*-----------------------------------------------------------*/

    #if ( ipconfigUSE_TCP_WIN == 1 )

/**
 * @brief Find out if the peer is able to receive more data.
 *
 * @param[in] pxWindow The descriptor of the TCP sliding windows.
 * @param[in] ulWindowSize The number of bytes in this segment.
 *
 * @return True if the peer has space in it window to receive more data.
 */
        static BaseType_t prvTCPWindowTxHasSpace( TCPWindow_t const * pxWindow,
                                                  uint32_t ulWindowSize )
        {
            uint32_t ulTxOutstanding;
            BaseType_t xHasSpace;
            const TCPSegment_t * pxSegment;
            uint32_t ulNettSize;

            /* This function will look if there is new transmission data.  It will
             * return true if there is data to be sent. */

            pxSegment = xTCPWindowPeekHead( &( pxWindow->xTxQueue ) );

            if( pxSegment == NULL )
            {
                xHasSpace = pdFALSE;
            }
            else
            {
                /* How much data is outstanding, i.e. how much data has been sent
                 * but not yet acknowledged ? */
                if( pxWindow->tx.ulHighestSequenceNumber >= pxWindow->tx.ulCurrentSequenceNumber )
                {
                    ulTxOutstanding = pxWindow->tx.ulHighestSequenceNumber - pxWindow->tx.ulCurrentSequenceNumber;
                }
                else
                {
                    ulTxOutstanding = 0U;
                }

                /* Subtract this from the peer's space. */
                ulNettSize = ulWindowSize - FreeRTOS_min_uint32( ulWindowSize, ulTxOutstanding );

                /* See if the next segment may be sent. */
                if( ulNettSize >= ( uint32_t ) pxSegment->lDataLength )
                {
                    xHasSpace = pdTRUE;
                }
                else
                {
                    xHasSpace = pdFALSE;
                }

                /* If 'xHasSpace', it looks like the peer has at least space for 1
                 * more new segment of size MSS.  xSize.ulTxWindowLength is the self-imposed
                 * limitation of the transmission window (in case of many resends it
                 * may be decreased). */
                if( ( ulTxOutstanding != 0U ) &&
                    ( pxWindow->xSize.ulTxWindowLength <
                      ( ulTxOutstanding + ( ( uint32_t ) pxSegment->lDataLength ) ) ) )
                {
                    xHasSpace = pdFALSE;
                }
            }

            return xHasSpace;
        }
    #endif /* ipconfigUSE_TCP_WIN == 1 */
/*-----------------------------------------------------------*/

    #if ( ipconfigUSE_TCP_WIN == 1 )

/**
 * @brief Returns true if there is TX data that can be sent right now.
 *
 * @param[in] pxWindow The descriptor of the TCP sliding windows.
 * @param[in] ulWindowSize The current size of the sliding RX window of the peer.
 * @param[out] pulDelay The delay before the packet may be sent.
 *
 * @return pdTRUE if there is Tx data that can be sent, else pdFALSE.
 */
        BaseType_t xTCPWindowTxHasData( TCPWindow_t const * pxWindow,
                                        uint32_t ulWindowSize,
                                        TickType_t * pulDelay )
        {
            TCPSegment_t const * pxSegment;
            BaseType_t xReturn;
            TickType_t ulAge, ulMaxAge;

            *pulDelay = 0U;

            if( listLIST_IS_EMPTY( &pxWindow->xPriorityQueue ) == pdFALSE )
            {
                /* No need to look at retransmissions or new transmission as long as
                 * there are priority segments.  *pulDelay equals zero, meaning it must
                 * be sent out immediately. */
                xReturn = pdTRUE;
            }
            else
            {
                pxSegment = xTCPWindowPeekHead( &( pxWindow->xWaitQueue ) );

                if( pxSegment != NULL )
                {
                    uint32_t ulSRTT = ( uint32_t ) pxWindow->lSRTT;

                    /* There is an outstanding segment, see if it is time to resend
                     * it. */
                    ulAge = ulTimerGetAge( &pxSegment->xTransmitTimer );

                    /* After a packet has been sent for the first time, it will wait
                     * '1 * ulSRTT' ms for an ACK. A second time it will wait '2 * ulSRTT' ms,
                     * each time doubling the time-out */
                    ulMaxAge = ( ( uint32_t ) 1U << pxSegment->u.bits.ucTransmitCount );
                    ulMaxAge *= ulSRTT;

                    if( ulMaxAge > ulAge )
                    {
                        /* A segment must be sent after this amount of msecs */
                        *pulDelay = ulMaxAge - ulAge;
                    }

                    xReturn = pdTRUE;
                }
                else
                {
                    /* No priority segment, no outstanding data, see if there is new
                     * transmission data. */
                    pxSegment = xTCPWindowPeekHead( &pxWindow->xTxQueue );

                    /* See if it fits in the peer's reception window. */
                    if( pxSegment == NULL )
                    {
                        xReturn = pdFALSE;
                    }
                    else if( prvTCPWindowTxHasSpace( pxWindow, ulWindowSize ) == pdFALSE )
                    {
                        /* Too many outstanding messages. */
                        xReturn = pdFALSE;
                    }
                    else if( ( pxWindow->u.bits.bSendFullSize != pdFALSE_UNSIGNED ) &&
                             ( pxSegment->lDataLength < pxSegment->lMaxLength ) )
                    {
                        /* 'bSendFullSize' is a special optimisation.  If true, the
                         * driver will only sent completely filled packets (of MSS
                         * bytes). */
                        xReturn = pdFALSE;
                    }
                    else
                    {
                        xReturn = pdTRUE;
                    }
                }
            }

            return xReturn;
        }
    #endif /* ipconfigUSE_TCP_WIN == 1 */
/*-----------------------------------------------------------*/

    #if ( ipconfigUSE_TCP_WIN == 1 )

/**
 * @brief Three type of queues are used for transmission: priority, waiting, and
 *        the normal TX queue of unsent data.  Message in the waiting queue will
 *        be sent when their timer has expired.
 * @param[in] pxWindow The descriptor of the TCP sliding windows.
 */
        static TCPSegment_t * pxTCPWindowTx_GetWaitQueue( const TCPWindow_t * pxWindow )
        {
            TCPSegment_t * pxSegment = xTCPWindowPeekHead( &( pxWindow->xWaitQueue ) );

            if( pxSegment != NULL )
            {
                /* Do check the timing. */
                uint32_t ulMaxTime;

                ulMaxTime = ( ( uint32_t ) 1U ) << pxSegment->u.bits.ucTransmitCount;
                ulMaxTime *= ( uint32_t ) pxWindow->lSRTT;

                if( ulTimerGetAge( &pxSegment->xTransmitTimer ) > ulMaxTime )
                {
                    /* A normal (non-fast) retransmission.  Move it from the
                     * head of the waiting queue. */
                    pxSegment = xTCPWindowGetHead( &( pxWindow->xWaitQueue ) );
                    pxSegment->u.bits.ucDupAckCount = ( uint8_t ) pdFALSE_UNSIGNED;

                    /* Some detailed logging. */
                    if( ( xTCPWindowLoggingLevel != 0 ) && ( ipconfigTCP_MAY_LOG_PORT( pxWindow->usOurPortNumber ) ) )
                    {
                        FreeRTOS_debug_printf( ( "ulTCPWindowTxGet[%u,%u]: WaitQueue %d bytes for sequence number %u (0x%X)\n",
                                                 pxWindow->usPeerPortNumber,
                                                 pxWindow->usOurPortNumber,
                                                 ( int ) pxSegment->lDataLength,
                                                 ( unsigned ) ( pxSegment->ulSequenceNumber - pxWindow->tx.ulFirstSequenceNumber ),
                                                 ( unsigned ) pxSegment->ulSequenceNumber ) );
                    }
                }
                else
                {
                    pxSegment = NULL;
                }
            }

            return pxSegment;
        }
    #endif /* ipconfigUSE_TCP_WIN == 1 */

/*-----------------------------------------------------------*/

    #if ( ipconfigUSE_TCP_WIN == 1 )

/**
 * @brief See if there is a transmission in the normal TX queue. It is the
 *        first time these data are being sent. After sending they will move
 *        the waiting queue.
 * @param[in] pxWindow The descriptor of the TCP sliding windows.
 * @param[in] ulWindowSize The available space that the peer has in his
 *                          reception window.
 * @return Either a segment that has to be sent, or NULL.
 */
        static TCPSegment_t * pxTCPWindowTx_GetTXQueue( TCPWindow_t * pxWindow,
                                                        uint32_t ulWindowSize )
        {
            TCPSegment_t * pxSegment = xTCPWindowPeekHead( &( pxWindow->xTxQueue ) );

            if( pxSegment == NULL )
            {
                /* No segments queued. */
            }
            else if( ( pxWindow->u.bits.bSendFullSize != pdFALSE_UNSIGNED ) &&
                     ( pxSegment->lDataLength < pxSegment->lMaxLength ) )
            {
                /* A segment has been queued but the driver waits until it
                 * has a full size of MSS. */
                pxSegment = NULL;
            }
            else if( prvTCPWindowTxHasSpace( pxWindow, ulWindowSize ) == pdFALSE )
            {
                /* Peer has no more space at this moment. */
                pxSegment = NULL;
            }
            else
            {
                /* pxSegment was just obtained with a peek function,
                 * now remove it from of the Tx queue. */
                pxSegment = xTCPWindowGetHead( &( pxWindow->xTxQueue ) );

                /* Don't let pxHeadSegment point to this segment any more,
                 * so no more data will be added. */
                if( pxWindow->pxHeadSegment == pxSegment )
                {
                    pxWindow->pxHeadSegment = NULL;
                }

                /* pxWindow->tx.highest registers the highest sequence
                 * number in our transmission window. */
                pxWindow->tx.ulHighestSequenceNumber = pxSegment->ulSequenceNumber + ( ( uint32_t ) pxSegment->lDataLength );

                /* ...and more detailed logging */
                if( ( xTCPWindowLoggingLevel >= 2 ) && ( ipconfigTCP_MAY_LOG_PORT( pxWindow->usOurPortNumber ) ) )
                {
                    FreeRTOS_debug_printf( ( "ulTCPWindowTxGet[%u,%u]: XmitQueue %d bytes for sequence number %u (ws %u)\n",
                                             pxWindow->usPeerPortNumber,
                                             pxWindow->usOurPortNumber,
                                             ( int ) pxSegment->lDataLength,
                                             ( unsigned ) ( pxSegment->ulSequenceNumber - pxWindow->tx.ulFirstSequenceNumber ),
                                             ( unsigned ) ulWindowSize ) );
                }
            }

            return pxSegment;
        }
    #endif /* ipconfigUSE_TCP_WIN == 1 */
/*-----------------------------------------------------------*/

    #if ( ipconfigUSE_TCP_WIN == 1 )

/**
 * @brief Get data that can be transmitted right now. There are three types of
 *        outstanding segments: Priority queue, Waiting queue, Normal TX queue.
 *
 * @param[in] pxWindow The descriptor of the TCP sliding windows.
 * @param[in] ulWindowSize The current size of the sliding RX window of the peer.
 * @param[out] plPosition The index within the TX stream buffer of the first byte to be sent.
 *
 * @return The amount of data in bytes that can be transmitted right now.
 */
        uint32_t ulTCPWindowTxGet( TCPWindow_t * pxWindow,
                                   uint32_t ulWindowSize,
                                   int32_t * plPosition )
        {
            TCPSegment_t * pxSegment;
            uint32_t ulReturn = 0U;

            /* Fetches data to be sent-out now.
             *
             * Priority messages: segments with a resend need no check current sliding
             * window size. */
            pxSegment = xTCPWindowGetHead( &( pxWindow->xPriorityQueue ) );
            pxWindow->ulOurSequenceNumber = pxWindow->tx.ulHighestSequenceNumber;

            if( pxSegment != NULL )
            {
                /* There is a priority segment. It doesn't need any checking for
                 * space or timeouts. */
                if( xTCPWindowLoggingLevel != 0 )
                {
                    FreeRTOS_debug_printf( ( "ulTCPWindowTxGet[%u,%u]: PrioQueue %d bytes for sequence number %u (ws %u)\n",
                                             pxWindow->usPeerPortNumber,
                                             pxWindow->usOurPortNumber,
                                             ( int ) pxSegment->lDataLength,
                                             ( unsigned ) ( pxSegment->ulSequenceNumber - pxWindow->tx.ulFirstSequenceNumber ),
                                             ( unsigned ) ulWindowSize ) );
                }
            }
            else
            {
                /* Waiting messages: outstanding messages with a running timer
                 * neither check peer's reception window size because these packets
                 * have been sent earlier. */
                pxSegment = pxTCPWindowTx_GetWaitQueue( pxWindow );

                if( pxSegment == NULL )
                {
                    /* New messages: sent-out for the first time.  Check current
                     * sliding window size of peer. */
                    pxSegment = pxTCPWindowTx_GetTXQueue( pxWindow, ulWindowSize );
                }
            }

            /* See if it has already been determined to return 0. */
            if( pxSegment != NULL )
            {
                configASSERT( listLIST_ITEM_CONTAINER( &( pxSegment->xQueueItem ) ) == NULL );

                /* Now that the segment will be transmitted, add it to the tail of
                 * the waiting queue. */
                vListInsertFifo( &pxWindow->xWaitQueue, &pxSegment->xQueueItem );

                /* And mark it as outstanding. */
                pxSegment->u.bits.bOutstanding = pdTRUE_UNSIGNED;

                /* Administer the transmit count, needed for fast
                 * retransmissions. */
                ( pxSegment->u.bits.ucTransmitCount )++;

                /* If there have been several retransmissions (4), decrease the
                 * size of the transmission window to at most 2 times MSS. */
                if( ( pxSegment->u.bits.ucTransmitCount == MAX_TRANSMIT_COUNT_USING_LARGE_WINDOW ) &&
                    ( pxWindow->xSize.ulTxWindowLength > ( 2U * ( ( uint32_t ) pxWindow->usMSS ) ) ) )
                {
                    uint16_t usMSS2 = ( uint16_t ) ( pxWindow->usMSS * 2U );
                    FreeRTOS_debug_printf( ( "ulTCPWindowTxGet[%u - %u]: Change Tx window: %u -> %u\n",
                                             pxWindow->usPeerPortNumber,
                                             pxWindow->usOurPortNumber,
                                             ( unsigned ) pxWindow->xSize.ulTxWindowLength,
                                             usMSS2 ) );
                    pxWindow->xSize.ulTxWindowLength = usMSS2;
                }

                /* Clear the transmit timer. */
                vTCPTimerSet( &( pxSegment->xTransmitTimer ) );

                pxWindow->ulOurSequenceNumber = pxSegment->ulSequenceNumber;

                /* Inform the caller where to find the data within the queue. */
                *plPosition = pxSegment->lStreamPos;

                /* And return the length of the data segment */
                ulReturn = ( uint32_t ) pxSegment->lDataLength;
            }

            return ulReturn;
        }
    #endif /* ipconfigUSE_TCP_WIN == 1 */
/*-----------------------------------------------------------*/

    #if ( ipconfigUSE_TCP_WIN == 1 )

/**
 * @brief Data has been sent, and an ACK has been received. Make an estimate
 *        of the round-trip time, and calculate the new timeout for transmissions.
 *        More explanation in a comment here below.
 *
 * @param[in] pxWindow The descriptor of the TCP sliding windows.
 * @param[in] pxSegment The segment that was just acknowledged.
 */
        static void prvTCPWindowTxCheckAck_CalcSRTT( TCPWindow_t * pxWindow,
                                                     const TCPSegment_t * pxSegment )
        {
            int32_t mS = ( int32_t ) ulTimerGetAge( &( pxSegment->xTransmitTimer ) );
            int32_t lSum = 0;
            int32_t lWeight = 0;
            int32_t lDivisor = 0;

            mS = ( mS < 0 ) ? ipINT32_MAX_VALUE : mS;

            if( pxWindow->lSRTT >= mS )
            {
                /* RTT becomes smaller: adapt slowly. */
                lWeight = winSRTT_DECREMENT_CURRENT;
                lDivisor = winSRTT_DECREMENT_NEW + winSRTT_DECREMENT_CURRENT;
                mS = FreeRTOS_multiply_int32( mS,
                                              winSRTT_DECREMENT_NEW );
            }
            else
            {
                /* RTT becomes larger: adapt quicker */
                lWeight = winSRTT_INCREMENT_CURRENT;
                lDivisor = winSRTT_INCREMENT_NEW + winSRTT_INCREMENT_CURRENT;
                mS = FreeRTOS_multiply_int32( mS,
                                              winSRTT_INCREMENT_NEW );
            }

            lSum = FreeRTOS_multiply_int32( pxWindow->lSRTT, lWeight );
            lSum = FreeRTOS_add_int32( lSum, mS );
            pxWindow->lSRTT = lSum / lDivisor;

            /* Cap to the minimum of 50ms. */
            if( pxWindow->lSRTT < winSRTT_CAP_mS )
            {
                pxWindow->lSRTT = winSRTT_CAP_mS;
            }
        }
    #endif /* ipconfigUSE_TCP_WIN == 1 */
/*-----------------------------------------------------------*/

    #if ( ipconfigUSE_TCP_WIN == 1 )

/**
 * @brief An acknowledgement or a selective ACK (SACK) was received. See if some outstanding data
 *        may be removed from the transmission queue(s). All TX segments for which
 *        ( ( ulSequenceNumber >= ulFirst ) && ( ulSequenceNumber < ulLast ) in a contiguous block.
 *        Note that the segments are stored in xTxSegments in a strict sequential order.
 *
 * @param[in] pxWindow The TCP-window object of the current connection.
 * @param[in] ulFirst The sequence number of the first byte that was acknowledged.
 * @param[in] ulLast The sequence number of the last byte ( minus one ) that was acknowledged.
 *
 * @return number of bytes that the tail of txStream may be advanced.
 */
        static uint32_t prvTCPWindowTxCheckAck( TCPWindow_t * pxWindow,
                                                uint32_t ulFirst,
                                                uint32_t ulLast )
        {
            uint32_t ulBytesConfirmed = 0U;
            uint32_t ulSequenceNumber = ulFirst;
            uint32_t ulDataLength;
            const ListItem_t * pxIterator;

            /* MISRA Ref 11.3.1 [Misaligned access] */
            /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
            /* coverity[misra_c_2012_rule_11_3_violation] */
            const ListItem_t * pxEnd = ( ( const ListItem_t * ) &( pxWindow->xTxSegments.xListEnd ) );
            BaseType_t xDoUnlink;
            TCPSegment_t * pxSegment;

            /* An acknowledgement or a selective ACK (SACK) was received.  See if some outstanding data
             * may be removed from the transmission queue(s).
             * All TX segments for which
             * ( ( ulSequenceNumber >= ulFirst ) && ( ulSequenceNumber < ulLast ) in a
             * contiguous block.  Note that the segments are stored in xTxSegments in a
             * strict sequential order. */

            /* SRTT[i] = (1-a) * SRTT[i-1] + a * RTT
             *
             * 0 < a < 1; usually a = 1/8
             *
             * RTO = 2 * SRTT
             *
             * where:
             * RTT is Round Trip Time
             * SRTT is Smoothed RTT
             * RTO is Retransmit timeout
             *
             * A Smoothed RTT will increase quickly, but it is conservative when
             * becoming smaller. */

            pxIterator = listGET_NEXT( pxEnd );

            while( ( pxIterator != pxEnd ) && ( xSequenceLessThan( ulSequenceNumber, ulLast ) != 0 ) )
            {
                xDoUnlink = pdFALSE;
                pxSegment = ( ( TCPSegment_t * ) listGET_LIST_ITEM_OWNER( pxIterator ) );

                /* Move to the next item because the current item might get
                 * removed. */
                pxIterator = ( const ListItem_t * ) listGET_NEXT( pxIterator );

                /* Continue if this segment does not fall within the ACK'd range. */
                if( xSequenceGreaterThan( ulSequenceNumber, pxSegment->ulSequenceNumber ) != pdFALSE )
                {
                    continue;
                }

                /* Is it ready? */
                if( ulSequenceNumber != pxSegment->ulSequenceNumber )
                {
                    /* coverity[break_stmt] : Break statement terminating the loop */
                    break;
                }

                ulDataLength = ( uint32_t ) pxSegment->lDataLength;

                if( pxSegment->u.bits.bAcked == pdFALSE_UNSIGNED )
                {
                    if( xSequenceGreaterThan( pxSegment->ulSequenceNumber + ( uint32_t ) ulDataLength, ulLast ) != pdFALSE )
                    {
                        /* What happens?  Only part of this segment was accepted,
                         * probably due to WND limits
                         *
                         * AAAAAAA BBBBBBB << acked
                         * aaaaaaa aaaa    << sent */
                        #if ( ipconfigHAS_DEBUG_PRINTF != 0 )
                        {
                            uint32_t ulFirstSeq = pxSegment->ulSequenceNumber - pxWindow->tx.ulFirstSequenceNumber;
                            FreeRTOS_debug_printf( ( "prvTCPWindowTxCheckAck[%u.%u]: %u - %u Partial sequence number %u - %u\n",
                                                     pxWindow->usPeerPortNumber,
                                                     pxWindow->usOurPortNumber,
                                                     ( unsigned ) ( ulFirstSeq - pxWindow->tx.ulFirstSequenceNumber ),
                                                     ( unsigned ) ( ulLast - pxWindow->tx.ulFirstSequenceNumber ),
                                                     ( unsigned ) ulFirstSeq,
                                                     ( unsigned ) ( ulFirstSeq + ulDataLength ) ) );
                        }
                        #endif /* ( ipconfigHAS_DEBUG_PRINTF != 0 ) */

                        break;
                    }

                    /* This segment is fully ACK'd, set the flag. */
                    pxSegment->u.bits.bAcked = pdTRUE;

                    /* Calculate the RTT only if the segment was sent-out for the
                     * first time and if this is the last ACK'd segment in a range. */
                    if( ( pxSegment->u.bits.ucTransmitCount == 1U ) &&
                        ( ( pxSegment->ulSequenceNumber + ulDataLength ) == ulLast ) )
                    {
                        prvTCPWindowTxCheckAck_CalcSRTT( pxWindow, pxSegment );
                    }

                    /* Unlink it from the 3 queues, but do not destroy it (yet). */
                    xDoUnlink = pdTRUE;
                }

                /* pxSegment->u.bits.bAcked is now true.  Is it located at the left
                 * side of the transmission queue?  If so, it may be freed. */
                if( ulSequenceNumber == pxWindow->tx.ulCurrentSequenceNumber )
                {
                    if( ( xTCPWindowLoggingLevel >= 2 ) && ( ipconfigTCP_MAY_LOG_PORT( pxWindow->usOurPortNumber ) ) )
                    {
                        FreeRTOS_debug_printf( ( "prvTCPWindowTxCheckAck: %u - %u Ready sequence number %u\n",
                                                 ( unsigned ) ( ulFirst - pxWindow->tx.ulFirstSequenceNumber ),
                                                 ( unsigned ) ( ulLast - pxWindow->tx.ulFirstSequenceNumber ),
                                                 ( unsigned ) ( pxSegment->ulSequenceNumber - pxWindow->tx.ulFirstSequenceNumber ) ) );
                    }

                    /* Increase the left-hand value of the transmission window. */
                    pxWindow->tx.ulCurrentSequenceNumber += ulDataLength;

                    /* This function will return the number of bytes that the tail
                     * of txStream may be advanced. */
                    ulBytesConfirmed += ulDataLength;

                    /* All segments below tx.ulCurrentSequenceNumber may be freed. */
                    vTCPWindowFree( pxSegment );

                    /* No need to unlink it any more. */
                    xDoUnlink = pdFALSE;
                }

                if( ( xDoUnlink != pdFALSE ) && ( listLIST_ITEM_CONTAINER( &( pxSegment->xQueueItem ) ) != NULL ) )
                {
                    /* Remove item from its queues. */
                    ( void ) uxListRemove( &pxSegment->xQueueItem );
                }

                ulSequenceNumber += ulDataLength;
            }

            return ulBytesConfirmed;
        }
    #endif /* ipconfigUSE_TCP_WIN == 1 */
/*-----------------------------------------------------------*/

    #if ( ipconfigUSE_TCP_WIN == 1 )

/**
 * @brief See if there are segments that need a fast retransmission.
 *
 * @param[in] pxWindow The descriptor of the TCP sliding windows.
 * @param[in] ulFirst The sequence number of the first segment that must be checked.
 *
 * @return The number of segments that need a fast retransmission.
 */
        static uint32_t prvTCPWindowFastRetransmit( TCPWindow_t * pxWindow,
                                                    uint32_t ulFirst )
        {
            const ListItem_t * pxIterator;
            const ListItem_t * pxEnd;
            TCPSegment_t * pxSegment;
            uint32_t ulCount = 0U;

            /* A higher Tx block has been acknowledged.  Now iterate through the
             * xWaitQueue to find a possible condition for a FAST retransmission. */

            /* MISRA Ref 11.3.1 [Misaligned access] */
/* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-113 */
            /* coverity[misra_c_2012_rule_11_3_violation] */
            pxEnd = ( ( const ListItem_t * ) &( pxWindow->xWaitQueue.xListEnd ) );

            pxIterator = listGET_NEXT( pxEnd );

            while( pxIterator != pxEnd )
            {
                /* Get the owner, which is a TCP segment. */
                pxSegment = ( ( TCPSegment_t * ) listGET_LIST_ITEM_OWNER( pxIterator ) );

                /* Hop to the next item before the current gets unlinked. */
                pxIterator = listGET_NEXT( pxIterator );

                /* Fast retransmission:
                 * When 3 packets with a higher sequence number have been acknowledged
                 * by the peer, it is very unlikely a current packet will ever arrive.
                 * It will be retransmitted far before the RTO. */
                if( pxSegment->u.bits.bAcked == pdFALSE_UNSIGNED )
                {
                    if( xSequenceLessThan( pxSegment->ulSequenceNumber, ulFirst ) != pdFALSE )
                    {
                        pxSegment->u.bits.ucDupAckCount++;

                        if( pxSegment->u.bits.ucDupAckCount == DUPLICATE_ACKS_BEFORE_FAST_RETRANSMIT )
                        {
                            pxSegment->u.bits.ucTransmitCount = ( uint8_t ) pdFALSE;

                            /* Not clearing 'ucDupAckCount' yet as more SACK's might come in
                             * which might lead to a second fast rexmit. */
                            if( ( xTCPWindowLoggingLevel >= 0 ) && ( ipconfigTCP_MAY_LOG_PORT( pxWindow->usOurPortNumber ) ) )
                            {
                                FreeRTOS_debug_printf( ( "prvTCPWindowFastRetransmit: Requeue sequence number %u < %u\n",
                                                         ( unsigned ) ( pxSegment->ulSequenceNumber - pxWindow->tx.ulFirstSequenceNumber ),
                                                         ( unsigned ) ( ulFirst - pxWindow->tx.ulFirstSequenceNumber ) ) );
                            }

                            /* Remove it from xWaitQueue. */
                            ( void ) uxListRemove( &pxSegment->xQueueItem );

                            /* Add this segment to the priority queue so it gets
                             * retransmitted immediately. */
                            vListInsertFifo( &( pxWindow->xPriorityQueue ), &( pxSegment->xQueueItem ) );
                            ulCount++;
                        }
                    }
                }
            }

            return ulCount;
        }
    #endif /* ipconfigUSE_TCP_WIN == 1 */
/*-----------------------------------------------------------*/

    #if ( ipconfigUSE_TCP_WIN == 1 )

/**
 * @brief Receive a normal ACK.
 *
 * @param[in] pxWindow Window in which a data is receive.
 * @param[in] ulSequenceNumber The sequence number of the ACK.
 *
 * @return The location where the packet should be added.
 */
        uint32_t ulTCPWindowTxAck( TCPWindow_t * pxWindow,
                                   uint32_t ulSequenceNumber )
        {
            uint32_t ulFirstSequence;
            uint32_t ulReturn;

            /* Receive a normal ACK. */

            ulFirstSequence = pxWindow->tx.ulCurrentSequenceNumber;

            if( xSequenceLessThanOrEqual( ulSequenceNumber, ulFirstSequence ) != pdFALSE )
            {
                ulReturn = 0U;
            }
            else
            {
                ulReturn = prvTCPWindowTxCheckAck( pxWindow, ulFirstSequence, ulSequenceNumber );
            }

            return ulReturn;
        }
    #endif /* ipconfigUSE_TCP_WIN == 1 */
/*-----------------------------------------------------------*/

    #if ( ipconfigUSE_TCP_WIN == 1 )

/**
 * @brief Receive a SACK option.
 *
 * @param[in] pxWindow Window in which the data is received.
 * @param[in] ulFirst Index of starting position of options.
 * @param[in] ulLast Index of end position of the options.
 *
 * @return returns the number of bytes which have been acked starting from
 *         the head position.
 */
        uint32_t ulTCPWindowTxSack( TCPWindow_t * pxWindow,
                                    uint32_t ulFirst,
                                    uint32_t ulLast )
        {
            uint32_t ulAckCount;
            uint32_t ulCurrentSequenceNumber = pxWindow->tx.ulCurrentSequenceNumber;

            /* Receive a SACK option. */
            ulAckCount = prvTCPWindowTxCheckAck( pxWindow, ulFirst, ulLast );
            ( void ) prvTCPWindowFastRetransmit( pxWindow, ulFirst );

            if( ( xTCPWindowLoggingLevel >= 1 ) && ( xSequenceGreaterThan( ulFirst, ulCurrentSequenceNumber ) != pdFALSE ) )
            {
                FreeRTOS_debug_printf( ( "ulTCPWindowTxSack[%u,%u]: from %u to %u (ack = %u)\n",
                                         pxWindow->usPeerPortNumber,
                                         pxWindow->usOurPortNumber,
                                         ( unsigned ) ( ulFirst - pxWindow->tx.ulFirstSequenceNumber ),
                                         ( unsigned ) ( ulLast - pxWindow->tx.ulFirstSequenceNumber ),
                                         ( unsigned ) ( pxWindow->tx.ulCurrentSequenceNumber - pxWindow->tx.ulFirstSequenceNumber ) ) );
            }

            return ulAckCount;
        }
    #endif /* ipconfigUSE_TCP_WIN == 1 */
/*-----------------------------------------------------------*/

#endif /* ipconfigUSE_TCP == 1 */
