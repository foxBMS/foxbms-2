/*
 * FreeRTOS+TCP V4.3.3
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

/**
 * @file FreeRTOS_BitConfig.c
 * @brief Some functions that help when analysing a binary stream of information.
 * It offers an alternative to using packet structs with unaligned data members.
 */

/* Standard includes. */
#include <stdint.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* FreeRTOS+TCP includes. */
#include "FreeRTOS_IP.h"
#include "FreeRTOS_UDP_IP.h"
#include "FreeRTOS_Sockets.h"
#include "FreeRTOS_BitConfig.h"

/*-----------------------------------------------------------*/

/**
 * @brief Initialise a bit-config struct.
 *
 * @param[in] pxConfig The structure containing a copy of the bits.
 * @param[in] pucData Not NULL if a bit-stream must be analysed, otherwise NULL.
 * @param[in] uxSize The length of the binary data stream.
 *
 * @return pdTRUE if the malloc was OK, otherwise pdFALSE.
 */
BaseType_t xBitConfig_init( BitConfig_t * pxConfig,
                            const uint8_t * pucData,
                            size_t uxSize )
{
    BaseType_t xResult = pdFALSE;

    ( void ) memset( ( void * ) pxConfig, 0, sizeof( *pxConfig ) );
    pxConfig->ucContents = ( uint8_t * ) pvPortMalloc( uxSize );

    if( pxConfig->ucContents != NULL )
    {
        pxConfig->uxSize = uxSize;

        if( pucData != NULL )
        {
            ( void ) memcpy( pxConfig->ucContents, pucData, uxSize );
        }
        else
        {
            ( void ) memset( pxConfig->ucContents, 0, uxSize );
        }

        xResult = pdTRUE;
    }
    else
    {
        pxConfig->xHasError = pdTRUE;
    }

    return xResult;
}
/*-----------------------------------------------------------*/

/**
 * @brief Read from a bit-config struct.
 *
 * @param[in] pxConfig The structure containing a copy of the bits.
 * @param[in] pucData Not NULL if a bit-stream must be analysed, otherwise NULL.
 * @param[in] uxSize The length of the binary data stream.
 *
 * @return pdTRUE if the malloc was OK, otherwise pdFALSE.
 */
BaseType_t xBitConfig_read_uc( BitConfig_t * pxConfig,
                               uint8_t * pucData,
                               size_t uxSize )
{
    BaseType_t xResult = pdFALSE;
    const size_t uxNeeded = uxSize;

    if( pxConfig->xHasError == pdFALSE )
    {
        if( ( pxConfig->uxIndex + uxNeeded ) <= pxConfig->uxSize )
        {
            if( pucData != NULL )
            {
                ( void ) memcpy( pucData, &( pxConfig->ucContents[ pxConfig->uxIndex ] ), uxNeeded );
            }
            else
            {
                /* Caller just wants to skip some bytes. */
            }

            pxConfig->uxIndex += uxNeeded;
            xResult = pdTRUE;
        }
        else
        {
            pxConfig->xHasError = pdTRUE;
        }
    }

    return xResult;
}
/*-----------------------------------------------------------*/

/**
 * @brief Peek the last byte from a bit-config struct.
 *
 * @param[in] pxConfig The structure containing a copy of the bits.
 * @param[in] pucData The buffer to stored peeked data.
 * @param[in] uxSize The length of the binary data stream.
 *
 * @return pdTRUE if the malloc was OK, otherwise pdFALSE.
 */
BaseType_t pucBitConfig_peek_last_index_uc( BitConfig_t * pxConfig,
                                            uint8_t * pucData,
                                            size_t uxSize )
{
    BaseType_t xResult = pdFALSE;
    const size_t uxNeeded = uxSize;

    if( pxConfig->xHasError == pdFALSE )
    {
        if( ( pxConfig->uxIndex >= uxNeeded ) && ( pucData != NULL ) )
        {
            ( void ) memcpy( pucData, &( pxConfig->ucContents[ pxConfig->uxIndex - uxNeeded ] ), uxNeeded );

            xResult = pdTRUE;
        }
        else
        {
            /* Not support to peek length larger than write. */
            pxConfig->xHasError = pdTRUE;
        }
    }

    return xResult;
}

/**
 * @brief Read a byte from the bit stream.
 *
 * @param[in] pxConfig The structure containing a copy of the bits.
 *
 * @return A byte value.  When there was not enough data, xHasError will be set.
 */
uint8_t ucBitConfig_read_8( BitConfig_t * pxConfig )
{
    uint8_t ucResult = 0xffU;
    const size_t uxNeeded = sizeof ucResult;
    uint8_t pucData[ sizeof ucResult ];

    if( xBitConfig_read_uc( pxConfig, pucData, uxNeeded ) != pdFALSE )
    {
        ucResult = pucData[ 0 ];
    }

    return ucResult;
}
/*-----------------------------------------------------------*/

/**
 * @brief Read 2 bytes from the bit stream.
 *
 * @param[in] pxConfig The structure containing a copy of the bits.
 *
 * @return A 16-bit value.  When there was not enough data, xHasError will be set.
 */
uint16_t usBitConfig_read_16( BitConfig_t * pxConfig )
{
    uint16_t usResult = 0xffffU;
    const size_t uxNeeded = sizeof usResult;
    uint8_t pucData[ sizeof usResult ];

    if( xBitConfig_read_uc( pxConfig, pucData, uxNeeded ) != pdFALSE )
    {
        usResult = ( uint16_t ) ( ( ( ( uint16_t ) pucData[ 0 ] ) << 8 ) |
                                  ( ( ( uint16_t ) pucData[ 1 ] ) ) );
    }

    return usResult;
}
/*-----------------------------------------------------------*/

/**
 * @brief Read 4 bytes from the bit stream.
 *
 * @param[in] pxConfig The structure containing a copy of the bits.
 *
 * @return A 32-bit value.  When there was not enough data, xHasError will be set.
 */
uint32_t ulBitConfig_read_32( BitConfig_t * pxConfig )
{
    uint32_t ulResult = 0xffffffffU;
    const size_t uxNeeded = sizeof ulResult;
    uint8_t pucData[ sizeof ulResult ];

    if( xBitConfig_read_uc( pxConfig, pucData, uxNeeded ) != pdFALSE )
    {
        ulResult = ( ( ( uint32_t ) pucData[ 0 ] ) << 24 ) |
                   ( ( ( uint32_t ) pucData[ 1 ] ) << 16 ) |
                   ( ( ( uint32_t ) pucData[ 2 ] ) << 8 ) |
                   ( ( ( uint32_t ) pucData[ 3 ] ) );
    }

    return ulResult;
}
/*-----------------------------------------------------------*/

/**
 * @brief Read any number bytes from the bit stream.
 *
 * @param[in] pxConfig The structure containing a copy of the bit stream.
 * @param[in] pucData The binary data to be written.
 * @param[in] uxSize The number of bytes to be written.
 *
 * There is no return value. If the operation has failed,
 *         the field xHasError will be set.
 */
void vBitConfig_write_uc( BitConfig_t * pxConfig,
                          const uint8_t * pucData,
                          size_t uxSize )
{
    const size_t uxNeeded = uxSize;

    if( pxConfig->xHasError == pdFALSE )
    {
        if( pxConfig->uxIndex <= ( pxConfig->uxSize - uxNeeded ) )
        {
            uint8_t * pucDestination = &( pxConfig->ucContents[ pxConfig->uxIndex ] );
            ( void ) memcpy( pucDestination, pucData, uxNeeded );
            pxConfig->uxIndex += uxNeeded;
        }
        else
        {
            pxConfig->xHasError = pdTRUE;
        }
    }
}
/*-----------------------------------------------------------*/

/**
 * @brief Write a byte to the bit stream.
 *
 * @param[in] pxConfig The structure containing a copy of the bits.
 * @param[in] ucValue The byte to be written.
 *
 * There is no return value. If the operation has failed,
 *         the field xHasError will be set.
 */
void vBitConfig_write_8( BitConfig_t * pxConfig,
                         uint8_t ucValue )
{
    const size_t uxNeeded = sizeof ucValue;

    vBitConfig_write_uc( pxConfig, &( ucValue ), uxNeeded );
}
/*-----------------------------------------------------------*/

/**
 * @brief Write a short word to the bit stream.
 *
 * @param[in] pxConfig The structure containing a copy of the bits.
 * @param[in] usValue The 16-bit value to be written.
 *
 * There is no return value. If the operation has failed,
 *         the field xHasError will be set.
 */
void vBitConfig_write_16( BitConfig_t * pxConfig,
                          uint16_t usValue )
{
    const size_t uxNeeded = sizeof usValue;
    uint8_t pucData[ sizeof usValue ];

    pucData[ 0 ] = ( uint8_t ) ( ( usValue >> 8 ) & 0xFFU );
    pucData[ 1 ] = ( uint8_t ) ( usValue & 0xFFU );
    vBitConfig_write_uc( pxConfig, pucData, uxNeeded );
}
/*-----------------------------------------------------------*/

/**
 * @brief Write a 32-bit word to the bit stream.
 *
 * @param[in] pxConfig The structure containing a copy of the bits.
 * @param[in] ulValue The 32-bit value to be written.
 *
 * There is no return value. If the operation has failed,
 *         the field xHasError will be set.
 */
void vBitConfig_write_32( BitConfig_t * pxConfig,
                          uint32_t ulValue )
{
    const size_t uxNeeded = sizeof ulValue;
    uint8_t pucData[ sizeof ulValue ];

    pucData[ 0 ] = ( uint8_t ) ( ( ulValue >> 24 ) & 0xFFU );
    pucData[ 1 ] = ( uint8_t ) ( ( ulValue >> 16 ) & 0xFFU );
    pucData[ 2 ] = ( uint8_t ) ( ( ulValue >> 8 ) & 0xFFU );
    pucData[ 3 ] = ( uint8_t ) ( ulValue & 0xFFU );

    vBitConfig_write_uc( pxConfig, pucData, uxNeeded );
}
/*-----------------------------------------------------------*/

/**
 * @brief Deallocate ( release ) the buffer, and clear the bit stream structure.
 *        Note that the struct must have be initialised before calling this function.
 *
 * @param[in] pxConfig The structure containing a copy of the bits.
 *
 */
void vBitConfig_release( BitConfig_t * pxConfig )
{
    if( pxConfig != NULL )
    {
        if( pxConfig->ucContents != NULL )
        {
            vPortFree( pxConfig->ucContents );
        }

        ( void ) memset( pxConfig, 0, sizeof( BitConfig_t ) );
    }
    else
    {
        /* Nothing to free */
    }
}
/*-----------------------------------------------------------*/
