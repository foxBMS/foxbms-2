/**
 *
 * @copyright &copy; 2010 - 2023, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * We kindly request you to use one or more of the following phrases to refer to
 * foxBMS in your hardware, software, documentation or advertising materials:
 *
 * - &Prime;This product uses parts of foxBMS&reg;&Prime;
 * - &Prime;This product includes parts of foxBMS&reg;&Prime;
 * - &Prime;This product is derived from foxBMS&reg;&Prime;
 *
 */

/**
 * @file    i2c.h
 * @author  foxBMS Team
 * @date    2021-07-22 (date of creation)
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
 * @ingroup DRIVERS
 * @prefix  I2C
 *
 * @brief   Header for the driver for the I2C module
 *
 */

#ifndef FOXBMS__I2C_H_
#define FOXBMS__I2C_H_

/*========== Includes =======================================================*/

#include "HL_i2c.h"

#include "fstd_types.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/* Timeout in microseconds to avoid infinite loops when waiting for flags */
#define I2C_TIMEOUT_us (1000u)

/* Transmitter DMA enable */
#define I2C_TXDMAEN (0x2u)
/* Receiver  DMA enable */
#define I2C_RXDMAEN (0x1u)

/* Mask to select data byte from Data Receive Register */
#define I2C_DDR_REGISTER_DATA_MASK (0xFFu)

/* Handle of task in which I2C transactions with DMA are running, used for notifications */
#define I2C_TASK_HANDLE (ftsk_taskHandleI2c)

/* Index used for FreeRTOS notification sent when TX DMA interrupt comes. Arbitrary unique value. */
#define I2C_NOTIFICATION_TX_INDEX (1u)
/* Index used for FreeRTOS notification sent when RX DMA interrupt comes. Arbitrary unique value. */
#define I2C_NOTIFICATION_RX_INDEX (2u)
/* Default notification value, signals that no notification was received. Arbitrary unique value. */
#define I2C_NO_NOTIFIED_VALUE (0x0u)
/* Notification value sent when TX DMA interrupt comes. Arbitrary unique value. */
#define I2C_TX_NOTIFIED_VALUE (0x51u)
/* Notification value sent when RX DMA interrupt comes. Arbitrary unique value. */
#define I2C_RX_NOTIFIED_VALUE (0x61u)
/* Notification value sent when RX DMA interrupt comes but last byte cannot be read. Arbitrary unique value. */
#define I2C_RX_NOTCOME_VALUE (0x62u)
/**
 * Time in ms to wait for DMA notification, after this delay the transaction
 * is considered to have failed
 */
#define I2C_NOTIFICATION_TIMEOUT_ms (2u)
/* Mask to select prescaler value in prescaler register */
#define I2C_PRESCALER_MASK (0xFFu)
/* Factor to transform MHz to Hz */
#define I2C_FACTOR_MHZ_TO_HZ (1000000u)
/* Factor to transform seconds in microseconds to Hz */
#define I2C_FACTOR_S_TO_US (1000000u)
/* Number of bits in one I2C frame (byte + ACK bit) */
#define I2C_FACTOR_WORD_TO_BITS (9u)
/**
 * Value of d factor in I2C configuration, depends on prescaler value
 * See Technical Reference Manual p.1769 eq.65, (SPNU563A - March 2018)
 * @{
 */
#define I2C_DFACTOR_VALUE_PRESCALER_0     (7u)
#define I2C_DFACTOR_VALUE_PRESCALER_1     (6u)
#define I2C_DFACTOR_VALUE_PRESCALER_OTHER (5u)
/**@}*/
/* Margin to add to the computed time for the last TX byte to be sent */
#define I2C_TX_TIME_MARGIN_us (5u)

/*========== Extern Constant and Variable Declarations ======================*/

extern uint8_t i2c_rxLastByteInterface1;
extern uint8_t i2c_rxLastByteInterface2;

/*========== Extern Function Prototypes =====================================*/
/**
 * @brief   Initialize the I2C hardware with dedicated HAL functions.
 *          Has to be called before any call to the rest of this API.
 */
extern void I2C_Initialize(void);

/**
 * @brief   reads from an I2C slave, no register address written first, blocking.
 * @param   pI2cInterface I2C interface to use
 * @param   slaveAddress  address of slave to communicate with
 * @param   nrBytes       number of registers to read
 * @param   readData      buffer containing the read data
 * @return  STD_OK if transmission OK, STD_NOT_OK otherwise
 */
extern STD_RETURN_TYPE_e I2C_Read(i2cBASE_t *pI2cInterface, uint32_t slaveAddress, uint32_t nrBytes, uint8_t *readData);

/**
 * @brief   writes to an I2C slave, no register address written first, blocking.
 * @param   pI2cInterface I2C interface to use
 * @param   slaveAddress  address of slave to communicate with
 * @param   nrBytes       number of registers to write
 * @param   writeData     buffer containing the data to write
 * @return  STD_OK if transmission OK, STD_NOT_OK otherwise
 */
extern STD_RETURN_TYPE_e I2C_Write(
    i2cBASE_t *pI2cInterface,
    uint32_t slaveAddress,
    uint32_t nrBytes,
    uint8_t *writeData);

/**
 * @brief   reads from an I2C slave, blocking.
 * @param   pI2cInterface I2C interface to use
 * @param   slaveAddress address of slave to communicate with
 * @param   nrBytesWrite number of registers to read
 * @param   writeData     buffer containing the read data
 * @param   nrBytesRead  number of registers to read
 * @param   readData     buffer containing the read data
 * @return  STD_OK if transmission OK, STD_NOT_OK otherwise
 */
extern STD_RETURN_TYPE_e I2C_WriteRead(
    i2cBASE_t *pI2cInterface,
    uint32_t slaveAddress,
    uint32_t nrBytesWrite,
    uint8_t *writeData,
    uint32_t nrBytesRead,
    uint8_t *readData);

/**
 * @brief   reads from an I2C slave, no register address written first, using DMA.
 * @param   pI2cInterface I2C interface to use
 * @param   slaveAddress address of slave to communicate with
 * @param   nrBytes      number of registers to read
 * @param   readData     buffer containing the read data
 * @return  STD_OK if transmission OK, STD_NOT_OK otherwise
 */
extern STD_RETURN_TYPE_e I2C_ReadDma(
    i2cBASE_t *pI2cInterface,
    uint32_t slaveAddress,
    uint32_t nrBytes,
    uint8_t *readData);

/**
 * @brief   writes to an I2C slave, no register address written first, using DMA.
 * @param   pI2cInterface I2C interface to use
 * @param   slaveAddress  address of slave to communicate with
 * @param   nrBytes       number of registers to write
 * @param   writeData     buffer containing the data to write
 * @return  retVal        STD_OK if transmission OK, STD_NOT_OK otherwise
 */
extern STD_RETURN_TYPE_e I2C_WriteDma(
    i2cBASE_t *pI2cInterface,
    uint32_t slaveAddress,
    uint32_t nrBytes,
    uint8_t *writeData);

/**
 * @brief   reads from an I2C slave, using DMA.
 * @param   pI2cInterface I2C interface to use
 * @param   slaveAddress address of slave to communicate with
 * @param   nrBytesWrite number of registers to read
 * @param   writeData     buffer containing the read data
 * @param   nrBytesRead  number of registers to read
 * @param   readData     buffer containing the read data
 * @return  STD_OK if transmission OK, STD_NOT_OK otherwise
 */
extern STD_RETURN_TYPE_e I2C_WriteReadDma(
    i2cBASE_t *pI2cInterface,
    uint32_t slaveAddress,
    uint32_t nrBytesWrite,
    uint8_t *writeData,
    uint32_t nrBytesRead,
    uint8_t *readData);

/**
 * @brief   Used to read last byte received per I2C.
 * @details This function must be called in the DMA Rx callback.
 *          It returns the last byte received per I2C
 * @param   pI2cInterface    I2C interface used
 * @return  last byte received on the I2C bus
 */
extern uint8_t I2C_ReadLastRxByte(i2cBASE_t *pI2cInterface);

/**
 * @brief   Waits for the I2C Rx buffer to be full.
 * @details When the buffer is full, the received byte can be read.
 *          If the buffer is full before timeout_us microseconds are elapsed,
 *          the function returns true, false otherwise.
 *          The function also returns false if a NACK condition
 *          is detected.
 * @param   pI2cInterface I2C interface to use
 * @param   timeout_us  time in microseconds to wait until the buffer is full
 * @return  true if buffer is full within timeout, false otherwise
 */
extern bool I2C_WaitReceive(i2cBASE_t *pI2cInterface, uint32_t timeout_us);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
#endif

#endif /* FOXBMS__I2C_H_ */
