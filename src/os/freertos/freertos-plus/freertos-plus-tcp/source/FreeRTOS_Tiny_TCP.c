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

/*=============================================================================
 *
 *       #####   #                      #####   ####  ######
 *       # # #   #                      # # #  #    #  #    #
 *         #                              #   #     #  #    #
 *         #   ###   #####  #    #        #   #        #    #
 *         #     #   #    # #    #        #   #        #####
 *         #     #   #    # #    # ####   #   #        #
 *         #     #   #    # #    #        #   #     #  #
 *         #     #   #    #  ####         #    #    #  #
 *        #### ##### #    #     #        ####   ####  ####
 *                             #
 *                          ###
 * Tiny-TCP: TCP without sliding windows.
 *
 *=============================================================================*/

/**
 * @file FreeRTOS_TINY_TCP.c
 * @brief Module which handles TCP when windowing is disabled
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

    #if ( ipconfigUSE_TCP_WIN == 0 )

/** @brief Logging verbosity level. */
        BaseType_t xTCPWindowLoggingLevel = 0;

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

            return uxDiff * portTICK_PERIOD_MS;
        }
/*-----------------------------------------------------------*/

/**
 * @brief Data was received at 'ulSequenceNumber'. See if it was expected
 *        and if there is enough space to store the new data.
 *
 * @param[in] pxWindow The window to be checked.
 * @param[in] ulSequenceNumber Sequence number of the data received.
 * @param[in] ulLength Length of the data received.
 * @param[in] ulSpace Space in the buffer.
 *
 * @return A 0 is returned if there is enough space and the sequence number is correct,
 *         if not then a -1 is returned.
 *
 * @note if true may be passed directly to user (segment expected and window is empty).
 *        But pxWindow->ackno should always be used to set "BUF->ackno".
 */
        int32_t lTCPWindowRxCheck( TCPWindow_t * pxWindow,
                                   uint32_t ulSequenceNumber,
                                   uint32_t ulLength,
                                   uint32_t ulSpace,
                                   uint32_t * pulSkipCount )
        {
            int32_t lReturn = -1;

            *pulSkipCount = 0;

            /* Data was received at 'ulSequenceNumber'.  See if it was expected
             * and if there is enough space to store the new data. */
            if( ( pxWindow->rx.ulCurrentSequenceNumber != ulSequenceNumber ) || ( ulSpace < ulLength ) )
            {
                lReturn = -1;
            }
            else
            {
                pxWindow->rx.ulCurrentSequenceNumber += ( uint32_t ) ulLength;
                lReturn = 0;
            }

            return lReturn;
        }
    #endif /* ipconfigUSE_TCP_WIN == 0 */
/*-----------------------------------------------------------*/

    #if ( ipconfigUSE_TCP_WIN == 0 )

/**
 * @brief Add data to the Tx Window.
 *
 * @param[in] pxWindow The window to which the data is to be added.
 * @param[in] ulLength The length of the data to be added.
 * @param[in] lPosition Position in the stream.
 * @param[in] lMax Size of the Tx stream.
 *
 * @return The data actually added.
 */
        int32_t lTCPWindowTxAdd( TCPWindow_t * pxWindow,
                                 uint32_t ulLength,
                                 int32_t lPosition,
                                 int32_t lMax )
        {
            TCPSegment_t * pxSegment = &( pxWindow->xTxSegment );
            int32_t lResult;

            /* Data is being scheduled for transmission. */

            /* lMax would indicate the size of the txStream. */
            ( void ) lMax;

            /* This is tiny TCP: there is only 1 segment for outgoing data.
             * As long as 'lDataLength' is unequal to zero, the segment is still occupied. */
            if( pxSegment->lDataLength > 0 )
            {
                lResult = 0L;
            }
            else
            {
                if( ulLength > ( uint32_t ) pxSegment->lMaxLength )
                {
                    if( ( xTCPWindowLoggingLevel != 0 ) && ( ipconfigTCP_MAY_LOG_PORT( pxWindow->usOurPortNumber ) != pdFALSE ) )
                    {
                        FreeRTOS_debug_printf( ( "lTCPWindowTxAdd: can only store %u / %d bytes\n", ( unsigned ) ulLength, ( int ) pxSegment->lMaxLength ) );
                    }

                    ulLength = ( uint32_t ) pxSegment->lMaxLength;
                }

                if( ( xTCPWindowLoggingLevel != 0 ) && ( ipconfigTCP_MAY_LOG_PORT( pxWindow->usOurPortNumber ) != pdFALSE ) )
                {
                    FreeRTOS_debug_printf( ( "lTCPWindowTxAdd: SeqNr %u (%u) Len %u\n",
                                             ( unsigned ) ( pxWindow->ulNextTxSequenceNumber - pxWindow->tx.ulFirstSequenceNumber ),
                                             ( unsigned ) ( pxWindow->tx.ulCurrentSequenceNumber - pxWindow->tx.ulFirstSequenceNumber ),
                                             ( unsigned ) ulLength ) );
                }

                /* The sequence number of the first byte in this packet. */
                pxSegment->ulSequenceNumber = pxWindow->ulNextTxSequenceNumber;
                pxSegment->lDataLength = ( int32_t ) ulLength;
                pxSegment->lStreamPos = lPosition;
                pxSegment->u.ulFlags = 0U;
                vTCPTimerSet( &( pxSegment->xTransmitTimer ) );

                /* Increase the sequence number of the next data to be stored for
                 * transmission. */
                pxWindow->ulNextTxSequenceNumber += ulLength;
                lResult = ( int32_t ) ulLength;
            }

            return lResult;
        }
    #endif /* ipconfigUSE_TCP_WIN == 0 */
/*-----------------------------------------------------------*/

    #if ( ipconfigUSE_TCP_WIN == 0 )

/**
 * @brief Fetches data to be sent.
 *
 * @param[in] pxWindow The window for the connection.
 * @param[in] ulWindowSize The size of the window.
 * @param[out] plPosition plPosition will point to a location with the circular data buffer: txStream.
 *
 * @return return the amount of data which may be sent along with the position in the txStream.
 */
        uint32_t ulTCPWindowTxGet( TCPWindow_t * pxWindow,
                                   uint32_t ulWindowSize,
                                   int32_t * plPosition )
        {
            TCPSegment_t * pxSegment = &( pxWindow->xTxSegment );
            uint32_t ulLength = ( uint32_t ) pxSegment->lDataLength;
            uint32_t ulMaxTime;

            if( ulLength != 0U )
            {
                /* _HT_ Still under investigation */
                ( void ) ulWindowSize;

                if( pxSegment->u.bits.bOutstanding != pdFALSE_UNSIGNED )
                {
                    /* As 'ucTransmitCount' has a minimum of 1, take 2 * RTT */
                    ulMaxTime = ( ( uint32_t ) 1U ) << pxSegment->u.bits.ucTransmitCount;
                    ulMaxTime *= ( uint32_t ) pxWindow->lSRTT;

                    if( ulTimerGetAge( &( pxSegment->xTransmitTimer ) ) < ulMaxTime )
                    {
                        ulLength = 0U;
                    }
                }

                if( ulLength != 0U )
                {
                    pxSegment->u.bits.bOutstanding = pdTRUE_UNSIGNED;
                    pxSegment->u.bits.ucTransmitCount++;
                    vTCPTimerSet( &pxSegment->xTransmitTimer );
                    pxWindow->ulOurSequenceNumber = pxSegment->ulSequenceNumber;
                    *plPosition = pxSegment->lStreamPos;
                }
            }

            return ulLength;
        }
    #endif /* ipconfigUSE_TCP_WIN == 0 */
/*-----------------------------------------------------------*/

    #if ( ipconfigUSE_TCP_WIN == 0 )

/**
 * @brief Has the transmission completed.
 *
 * @param[in] pxWindow The window whose transmission window is to be checked.
 *
 * @return If there is no outstanding data then pdTRUE is returned,
 *         else pdFALSE.
 */
        BaseType_t xTCPWindowTxDone( const TCPWindow_t * pxWindow )
        {
            BaseType_t xReturn;

            /* Has the outstanding data been sent because user wants to shutdown? */
            if( pxWindow->xTxSegment.lDataLength == 0 )
            {
                xReturn = pdTRUE;
            }
            else
            {
                xReturn = pdFALSE;
            }

            return xReturn;
        }
    #endif /* ipconfigUSE_TCP_WIN == 0 */
/*-----------------------------------------------------------*/

    #if ( ipconfigUSE_TCP_WIN == 0 )
        static BaseType_t prvTCPWindowTxHasSpace( TCPWindow_t const * pxWindow,
                                                  uint32_t ulWindowSize );

/**
 * @brief Check if the window has space for one message.
 *
 * @param[in] pxWindow The window to be checked.
 * @param[in] ulWindowSize Size of the window.
 *
 * @return pdTRUE if the window has space, pdFALSE otherwise.
 */
        static BaseType_t prvTCPWindowTxHasSpace( TCPWindow_t const * pxWindow,
                                                  uint32_t ulWindowSize )
        {
            BaseType_t xReturn;

            if( ulWindowSize >= pxWindow->usMSSInit )
            {
                xReturn = pdTRUE;
            }
            else
            {
                xReturn = pdFALSE;
            }

            return xReturn;
        }
    #endif /* ipconfigUSE_TCP_WIN == 0 */
/*-----------------------------------------------------------*/

    #if ( ipconfigUSE_TCP_WIN == 0 )

/**
 * @brief Check data to be sent and calculate the time period the process may sleep.
 *
 * @param[in] pxWindow The window to be checked.
 * @param[in] ulWindowSize Size of the window.
 * @param[out] pulDelay The time period (in ticks) that the process may sleep.
 *
 * @return pdTRUE if the process should sleep or pdFALSE.
 */
        BaseType_t xTCPWindowTxHasData( TCPWindow_t const * pxWindow,
                                        uint32_t ulWindowSize,
                                        TickType_t * pulDelay )
        {
            TCPSegment_t const * pxSegment = &( pxWindow->xTxSegment );
            BaseType_t xReturn;
            TickType_t ulAge, ulMaxAge;

            /* Check data to be sent. */
            *pulDelay = ( TickType_t ) 0;

            if( pxSegment->lDataLength == 0 )
            {
                /* Got nothing to send right now. */
                xReturn = pdFALSE;
            }
            else
            {
                if( pxSegment->u.bits.bOutstanding != pdFALSE_UNSIGNED )
                {
                    ulAge = ulTimerGetAge( &pxSegment->xTransmitTimer );
                    ulMaxAge = ( ( TickType_t ) 1U << pxSegment->u.bits.ucTransmitCount ) * ( ( uint32_t ) pxWindow->lSRTT );

                    if( ulMaxAge > ulAge )
                    {
                        *pulDelay = ulMaxAge - ulAge;
                    }

                    xReturn = pdTRUE;
                }
                else if( prvTCPWindowTxHasSpace( pxWindow, ulWindowSize ) == pdFALSE )
                {
                    /* Too many outstanding messages. */
                    xReturn = pdFALSE;
                }
                else
                {
                    xReturn = pdTRUE;
                }
            }

            return xReturn;
        }
    #endif /* ipconfigUSE_TCP_WIN == 0 */
/*-----------------------------------------------------------*/

    #if ( ipconfigUSE_TCP_WIN == 0 )

/**
 * @brief Receive a normal ACK.
 *
 * @param[in] pxWindow The window for this particular connection.
 * @param[in] ulSequenceNumber The sequence number of the packet.
 *
 * @return Number of bytes to send.
 */
        uint32_t ulTCPWindowTxAck( TCPWindow_t * pxWindow,
                                   uint32_t ulSequenceNumber )
        {
            TCPSegment_t * pxSegment = &( pxWindow->xTxSegment );
            uint32_t ulDataLength = ( uint32_t ) pxSegment->lDataLength;

            /* Receive a normal ACK */

            if( ulDataLength != 0U )
            {
                if( ulSequenceNumber < ( pxWindow->tx.ulCurrentSequenceNumber + ulDataLength ) )
                {
                    if( ipconfigTCP_MAY_LOG_PORT( pxWindow->usOurPortNumber ) != pdFALSE )
                    {
                        FreeRTOS_debug_printf( ( "win_tx_ack: acked %u expc %u len %u\n",
                                                 ( unsigned ) ( ulSequenceNumber - pxWindow->tx.ulFirstSequenceNumber ),
                                                 ( unsigned ) ( pxWindow->tx.ulCurrentSequenceNumber - pxWindow->tx.ulFirstSequenceNumber ),
                                                 ( unsigned ) ulDataLength ) );
                    }

                    /* Nothing to send right now. */
                    ulDataLength = 0U;
                }
                else
                {
                    pxWindow->tx.ulCurrentSequenceNumber += ulDataLength;

                    if( ( xTCPWindowLoggingLevel != 0 ) && ( ipconfigTCP_MAY_LOG_PORT( pxWindow->usOurPortNumber ) != pdFALSE ) )
                    {
                        FreeRTOS_debug_printf( ( "win_tx_ack: acked seqnr %u len %u\n",
                                                 ( unsigned ) ( ulSequenceNumber - pxWindow->tx.ulFirstSequenceNumber ),
                                                 ( unsigned ) ulDataLength ) );
                    }

                    pxSegment->lDataLength = 0;
                }
            }

            return ulDataLength;
        }
    #endif /* ipconfigUSE_TCP_WIN == 0 */
/*-----------------------------------------------------------*/

    #if ( ipconfigUSE_TCP_WIN == 0 )

/**
 * @brief This function will be called as soon as a FIN is received to check
 *        whether all transmit queues are empty or not.
 *
 * @param[in] pxWindow The window to be checked.
 *
 * @return It will return true if there are no 'open' reception segments.
 */
        BaseType_t xTCPWindowRxEmpty( const TCPWindow_t * pxWindow )
        {
            /* Return true if 'ulCurrentSequenceNumber >= ulHighestSequenceNumber'
             * 'ulCurrentSequenceNumber' is the highest sequence number stored,
             * 'ulHighestSequenceNumber' is the highest sequence number seen. */
            return xSequenceGreaterThanOrEqual( pxWindow->rx.ulCurrentSequenceNumber, pxWindow->rx.ulHighestSequenceNumber );
        }
    #endif /* ipconfigUSE_TCP_WIN == 0 */
/*-----------------------------------------------------------*/

    #if ( ipconfigUSE_TCP_WIN == 0 )

/**
 * @brief Destroy a window.
 *
 * @param[in] pxWindow Pointer to the window to be destroyed.
 *
 * @return Always returns a NULL.
 */
        void vTCPWindowDestroy( const TCPWindow_t * pxWindow )
        {
            /* As in tiny TCP there are no shared segments descriptors, there is
             * nothing to release. */
            ( void ) pxWindow;
        }
    #endif /* ipconfigUSE_TCP_WIN == 0 */
/*-----------------------------------------------------------*/

#endif /* ipconfigUSE_TCP == 1 */
