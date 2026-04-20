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
 * @file FreeRTOS_Stream_Buffer.c
 * @brief Provides the API for managing/creating the stream buffers in the FreeRTOS+TCP network stack.
 */

/* Standard includes. */
#include <stdint.h>
#include <string.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"

/* FreeRTOS+TCP includes. */
#include "FreeRTOS_IP.h"


/**
 * @brief Get the space between lower and upper value provided to the function.
 * @param[in] pxBuffer The circular stream buffer.
 * @param[in] uxLower The lower value.
 * @param[in] uxUpper The upper value.
 * @return The space between uxLower and uxUpper, which equals to the distance
 *         minus 1.
 */
size_t uxStreamBufferSpace( const StreamBuffer_t * const pxBuffer,
                            size_t uxLower,
                            size_t uxUpper )
{
    const size_t uxLength = pxBuffer->LENGTH;
    size_t uxCount = uxLength + uxUpper - uxLower - 1U;

    if( uxCount >= uxLength )
    {
        uxCount -= uxLength;
    }

    return uxCount;
}
/*-----------------------------------------------------------*/

/**
 * @brief Get the distance between lower and upper value provided to the function.
 * @param[in] pxBuffer The circular stream buffer.
 * @param[in] uxLower The lower value.
 * @param[in] uxUpper The upper value.
 * @return The distance between uxLower and uxUpper.
 */
size_t uxStreamBufferDistance( const StreamBuffer_t * const pxBuffer,
                               size_t uxLower,
                               size_t uxUpper )
{
    const size_t uxLength = pxBuffer->LENGTH;
    size_t uxCount = uxLength + uxUpper - uxLower;

    if( uxCount >= uxLength )
    {
        uxCount -= uxLength;
    }

    return uxCount;
}
/*-----------------------------------------------------------*/

/**
 * @brief Get the number of items which can be added to the buffer at
 *        the head before reaching the tail.
 * @param[in] pxBuffer The circular stream buffer.
 * @return The number of items which can still be added to uxHead
 *         before hitting on uxTail
 */
size_t uxStreamBufferGetSpace( const StreamBuffer_t * const pxBuffer )
{
    return uxStreamBufferSpace( pxBuffer, pxBuffer->uxHead, pxBuffer->uxTail );
}
/*-----------------------------------------------------------*/

/**
 * @brief Get the distance between the pointer in free space and the tail.
 * @param[in] pxBuffer The circular stream buffer.
 * @return Distance between uxFront and uxTail or the number of items
 *         which can still be added to uxFront, before hitting on uxTail.
 */
size_t uxStreamBufferFrontSpace( const StreamBuffer_t * const pxBuffer )
{
    return uxStreamBufferSpace( pxBuffer, pxBuffer->uxFront, pxBuffer->uxTail );
}
/*-----------------------------------------------------------*/

/**
 * @brief Get the number of items which can be read from the tail before
 *        reaching the head.
 * @param[in] pxBuffer The circular stream buffer.
 * @return The number of items which can be read from the tail before
 *        reaching the head.
 */
size_t uxStreamBufferGetSize( const StreamBuffer_t * const pxBuffer )
{
    return uxStreamBufferDistance( pxBuffer, pxBuffer->uxTail, pxBuffer->uxHead );
}
/*-----------------------------------------------------------*/

/**
 * @brief Get the space between the mid pointer and the head in the stream
 *        buffer.
 * @param[in] pxBuffer The circular stream buffer.
 * @return The space between the mid pointer and the head.
 */
size_t uxStreamBufferMidSpace( const StreamBuffer_t * const pxBuffer )
{
    return uxStreamBufferDistance( pxBuffer, pxBuffer->uxMid, pxBuffer->uxHead );
}
/*-----------------------------------------------------------*/

/**
 * @brief Clear the stream buffer.
 * @param[in] pxBuffer The circular stream buffer.
 */
void vStreamBufferClear( StreamBuffer_t * const pxBuffer )
{
    /* Make the circular buffer empty */
    pxBuffer->uxHead = 0U;
    pxBuffer->uxTail = 0U;
    pxBuffer->uxFront = 0U;
    pxBuffer->uxMid = 0U;
}

/*-----------------------------------------------------------*/

/**
 * @brief Move the mid pointer forward by given byte count
 * @param[in] pxBuffer The circular stream buffer.
 * @param[in] uxCount The byte count by which the mid pointer is to be moved.
 */
void vStreamBufferMoveMid( StreamBuffer_t * const pxBuffer,
                           const size_t uxCount )
{
    /* Increment uxMid, but no further than uxHead */
    const size_t uxLength = pxBuffer->LENGTH;
    const size_t uxSize = uxStreamBufferMidSpace( pxBuffer );
    size_t uxMid = pxBuffer->uxMid;
    size_t uxMoveCount = uxCount;

    if( uxMoveCount > uxSize )
    {
        uxMoveCount = uxSize;
    }

    uxMid += uxMoveCount;

    if( uxMid >= uxLength )
    {
        uxMid -= uxLength;
    }

    pxBuffer->uxMid = uxMid;
}
/*-----------------------------------------------------------*/

/**
 * @brief Check whether the value in left is less than or equal to the
 *        value in right from the perspective of the circular stream
 *        buffer.
 * @param[in] pxBuffer The circular stream buffer.
 * @param[in] uxLeft The left pointer in the stream buffer.
 * @param[in] uxRight The right value pointer in the stream buffer.
 * @return pdTRUE if uxLeft <= uxRight, else pdFALSE.
 */
BaseType_t xStreamBufferLessThenEqual( const StreamBuffer_t * const pxBuffer,
                                       size_t uxLeft,
                                       size_t uxRight )
{
    BaseType_t xReturn = pdFALSE;
    const size_t uxTail = pxBuffer->uxTail;

    if( ( uxLeft - uxTail ) <= ( uxRight - uxTail ) )
    {
        xReturn = pdTRUE;
    }

    return xReturn;
}
/*-----------------------------------------------------------*/

/**
 * @brief Get the pointer to data and the amount of data which can be read in one go.
 *
 * @param[in] pxBuffer The circular stream buffer.
 * @param[out] ppucData Pointer to the data pointer which will point to the
 *                       data which can be read.
 *
 * @return The number of bytes which can be read in one go (which might be less than
 *         actual number of available bytes since this is a circular buffer and tail
 *         can loop back to the start of the buffer).
 */
size_t uxStreamBufferGetPtr( StreamBuffer_t * const pxBuffer,
                             uint8_t ** const ppucData )
{
    const size_t uxNextTail = pxBuffer->uxTail;
    const size_t uxSize = uxStreamBufferGetSize( pxBuffer );

    /* MISRA Ref 18.4.1 [Usage of +, -, += and -= operators on expression of pointer type]. */
    /* More details at: https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/MISRA.md#rule-184. */
    /* coverity[misra_c_2012_rule_18_4_violation] */
    *ppucData = pxBuffer->ucArray + uxNextTail;

    return FreeRTOS_min_size_t( uxSize, pxBuffer->LENGTH - uxNextTail );
}
/*-----------------------------------------------------------*/

/**
 * @brief Adds data to a stream buffer.
 *
 * @param[in,out] pxBuffer The buffer to which the bytes will be added.
 * @param[in] uxOffset If uxOffset > 0, data will be written at an offset from uxHead
 *                      while uxHead will not be moved yet.
 * @param[in] pucData A pointer to the data to be added. If 'pucData' equals NULL,
 *                     the function is called to advance the 'Head' only.
 * @param[in] uxByteCount The number of bytes to add.
 *
 * @return The number of bytes added to the buffer.
 */
size_t uxStreamBufferAdd( StreamBuffer_t * const pxBuffer,
                          size_t uxOffset,
                          const uint8_t * const pucData,
                          size_t uxByteCount )
{
    size_t uxCount;
    size_t uxSpace = uxStreamBufferGetSpace( pxBuffer );

    /* If uxOffset > 0, items can be placed in front of uxHead */
    if( uxSpace > uxOffset )
    {
        uxSpace -= uxOffset;
    }
    else
    {
        uxSpace = 0U;
    }

    /* The number of bytes that can be written is the minimum of the number of
     * bytes requested and the number available. */
    uxCount = FreeRTOS_min_size_t( uxSpace, uxByteCount );

    if( uxCount != 0U )
    {
        const size_t uxLength = pxBuffer->LENGTH;
        size_t uxNextHead = pxBuffer->uxHead;

        if( uxOffset != 0U )
        {
            /* ( uxOffset > 0 ) means: write in front if the uxHead marker */
            uxNextHead += uxOffset;

            if( uxNextHead >= uxLength )
            {
                uxNextHead -= uxLength;
            }
        }

        if( pucData != NULL )
        {
            /* Calculate the number of bytes that can be added in the first
            * write - which may be less than the total number of bytes that need
            * to be added if the buffer will wrap back to the beginning. */
            const size_t uxFirst = FreeRTOS_min_size_t( uxLength - uxNextHead, uxCount );

            /* Write as many bytes as can be written in the first write. */
            ( void ) memcpy( &( pxBuffer->ucArray[ uxNextHead ] ), pucData, uxFirst );

            /* If the number of bytes written was less than the number that
             * could be written in the first write... */
            if( uxCount > uxFirst )
            {
                /* ...then write the remaining bytes to the start of the
                 * buffer. */
                ( void ) memcpy( pxBuffer->ucArray, &( pucData[ uxFirst ] ), uxCount - uxFirst );
            }
        }

        /* The below update to the stream buffer members must happen
         * atomically. */
        taskENTER_CRITICAL();
        {
            if( uxOffset == 0U )
            {
                /* ( uxOffset == 0 ) means: write at uxHead position */
                uxNextHead += uxCount;

                if( uxNextHead >= uxLength )
                {
                    uxNextHead -= uxLength;
                }

                pxBuffer->uxHead = uxNextHead;
            }

            if( xStreamBufferLessThenEqual( pxBuffer, pxBuffer->uxFront, uxNextHead ) != pdFALSE )
            {
                /* Advance the front pointer */
                pxBuffer->uxFront = uxNextHead;
            }
        }
        taskEXIT_CRITICAL();
    }

    return uxCount;
}
/*-----------------------------------------------------------*/

/**
 * @brief Read bytes from stream buffer.
 *
 * @param[in] pxBuffer The buffer from which the bytes will be read.
 * @param[in] uxOffset can be used to read data located at a certain offset from 'lTail'.
 * @param[in,out] pucData If 'pucData' equals NULL, the function is called to advance 'lTail' only.
 * @param[in] uxMaxCount The number of bytes to read.
 * @param[in] xPeek if 'xPeek' is pdTRUE, or if 'uxOffset' is non-zero, the 'lTail' pointer will
 *                   not be advanced.
 *
 * @return The count of the bytes read.
 */
size_t uxStreamBufferGet( StreamBuffer_t * const pxBuffer,
                          size_t uxOffset,
                          uint8_t * const pucData,
                          size_t uxMaxCount,
                          BaseType_t xPeek )
{
    size_t uxCount;

    /* How much data is available? */
    size_t uxSize = uxStreamBufferGetSize( pxBuffer );

    if( uxSize > uxOffset )
    {
        uxSize -= uxOffset;
    }
    else
    {
        uxSize = 0U;
    }

    /* Use the minimum of the wanted bytes and the available bytes. */
    uxCount = FreeRTOS_min_size_t( uxSize, uxMaxCount );

    if( uxCount != 0U )
    {
        const size_t uxLength = pxBuffer->LENGTH;
        size_t uxNextTail = pxBuffer->uxTail;

        if( uxOffset != 0U )
        {
            uxNextTail += uxOffset;

            if( uxNextTail >= uxLength )
            {
                uxNextTail -= uxLength;
            }
        }

        if( pucData != NULL )
        {
            /* Calculate the number of bytes that can be read - which may be
             * less than the number wanted if the data wraps around to the start of
             * the buffer. */
            const size_t uxFirst = FreeRTOS_min_size_t( uxLength - uxNextTail, uxCount );

            /* Obtain the number of bytes it is possible to obtain in the first
             * read. */
            ( void ) memcpy( pucData, &( pxBuffer->ucArray[ uxNextTail ] ), uxFirst );

            /* If the total number of wanted bytes is greater than the number
             * that could be read in the first read... */
            if( uxCount > uxFirst )
            {
                /* ...then read the remaining bytes from the start of the buffer. */
                ( void ) memcpy( &( pucData[ uxFirst ] ), pxBuffer->ucArray, uxCount - uxFirst );
            }
        }

        if( ( xPeek == pdFALSE ) && ( uxOffset == 0U ) )
        {
            /* Move the tail pointer to effectively remove the data read from
             * the buffer. */
            uxNextTail += uxCount;

            if( uxNextTail >= uxLength )
            {
                uxNextTail -= uxLength;
            }

            pxBuffer->uxTail = uxNextTail;
        }
    }

    return uxCount;
}
/*-----------------------------------------------------------*/
