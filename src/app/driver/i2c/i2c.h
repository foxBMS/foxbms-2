/**
 *
 * @copyright &copy; 2010 - 2021, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * @updated 2021-12-08 (date of last update)
 * @ingroup DRIVERS
 * @prefix  I2C
 *
 * @brief   Header for the driver for the I2C module
 *
 */

#ifndef FOXBMS__I2C_H_
#define FOXBMS__I2C_H_

/*========== Includes =======================================================*/
#include "general.h"

#include "HL_i2c.h"

/*========== Macros and Definitions =========================================*/

/** Timeout to avoid infinite loops when waiting for flags */
#define I2C_TIMEOUT_ITERATIONS (1000u)

/** Transmitter DMA enable */
#define I2C_TXDMAEN (0x2u)
/** Receiver  DMA enable */
#define I2C_RXDMAEN (0x1u)

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/
/** Initialize the I2C hardware
 *
 * This function has to be called before any call to the rest of this API
 */
extern void I2C_Initialize(void);

/**
 * @brief   reads from an I2C slave, blocking.
 *
 * @param   slaveAddress: address of slave to communicate with
 * @param   readAddress:  address of first register to read from
 * @param   nrBytes:      number of registers to read
 * @param   readData:     buffer containing the read data
 *
 * @return  retVal:      STD_OK if transmission OK, STD_NOT_OK otherwise
 *
 */
extern STD_RETURN_TYPE_e I2C_Read(uint32_t slaveAddress, uint8_t readAddress, uint32_t nrBytes, uint8_t *readData);

/**
 * @brief   reads from an I2C slave, no register address written first, blocking.
 *
 * @param   slaveAddress: address of slave to communicate with
 * @param   nrBytes:      number of registers to read
 * @param   readData:     buffer containing the read data
 *
 * @return  retVal:      STD_OK if transmission OK, STD_NOT_OK otherwise
 *
 */
extern STD_RETURN_TYPE_e I2C_ReadDirect(uint32_t slaveAddress, uint32_t nrBytes, uint8_t *readData);

/**
 * @brief   writes to an I2C slave, blocking.
 *
 * @param   slaveAddress:  address of slave to communicate with
 * @param   writeAddress:  address of first register to write to
 * @param   nrBytes:       number of registers to write
 * @param   writeData:     buffer containing the data to write
 *
 * @return  retVal:       STD_OK if transmission OK, STD_NOT_OK otherwise
 *
 */
extern STD_RETURN_TYPE_e I2C_Write(uint32_t slaveAddress, uint8_t writeAddress, uint32_t nrBytes, uint8_t *writeData);

/**
 * @brief   writes to an I2C slave, no register address written first, blocking.
 *
 * @param   slaveAddress:  address of slave to communicate with
 * @param   nrBytes:       number of registers to write
 * @param   writeData:     buffer containing the data to write
 *
 * @return  retVal:       STD_OK if transmission OK, STD_NOT_OK otherwise
 *
 */
extern STD_RETURN_TYPE_e I2C_WriteDirect(uint32_t slaveAddress, uint32_t nrBytes, uint8_t *writeData);

/**
 * @brief   reads from an I2C slave, using DMA.
 *
 * @param   slaveAddress: address of slave to communicate with
 * @param   readAddress:  address of first register to read from
 * @param   nrBytes:      number of registers to read
 * @param   readData:     buffer containing the read data
 *
 * @return  retVal:      STD_OK if transmission OK, STD_NOT_OK otherwise
 *
 */
extern STD_RETURN_TYPE_e I2C_ReadDma(uint32_t slaveAddress, uint8_t readAddress, uint32_t nrBytes, uint8_t *readData);

/**
 * @brief   writes to an I2C slave, using DMA.
 *
 * @param   slaveAddress:  address of slave to communicate with
 * @param   writeAddress:  address of first register to write to
 * @param   nrBytes:       number of registers to write
 * @param   writeData:     buffer containing the data to write
 *
 * @return  retVal:       STD_OK if transmission OK, STD_NOT_OK otherwise
 *
 */
extern STD_RETURN_TYPE_e I2C_WriteDma(
    uint32_t slaveAddress,
    uint8_t writeAddress,
    uint32_t nrBytes,
    uint8_t *writeData);

/**
 * @brief   sets stop condition.
 *
 */
extern void I2C_SetStopNow(void);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST

#endif

#endif /* FOXBMS__I2C_H_ */
