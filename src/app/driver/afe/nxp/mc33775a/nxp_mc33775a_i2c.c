/**
 *
 * @copyright &copy; 2010 - 2025, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * - "This product uses parts of foxBMS&reg;"
 * - "This product includes parts of foxBMS&reg;"
 * - "This product is derived from foxBMS&reg;"
 *
 */

/**
 * @file    nxp_mc33775a_i2c.c
 * @author  foxBMS Team
 * @date    2025-02-03 (date of creation)
 * @updated 2025-03-31 (date of last update)
 * @version v1.9.0
 * @ingroup DRIVERS
 * @prefix  N775
 *
 * @brief   I2C functions of the MC33775A analog front-end driver.
 * @details TODO
 */

/*========== Includes =======================================================*/
#include "nxp_mc33775a_i2c.h"
/* clang-format off */
#include "nxp_mc33775a-ll.h"
/* clang-format on */
#include "nxp_mc33775a.h"

#pragma diag_push
#pragma diag_suppress 232
#include "MC33775A.h"
#pragma diag_pop

#include "afe.h"
#include "fstd_types.h"
#include "ftask.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/

extern void N775_InitializeI2c(N775_STATE_s *pState) {
    FAS_ASSERT(pState != NULL_PTR);

    /* Enable the I2C module and select 400 kHz */
    N775_CommunicationWrite(
        N775_BROADCAST_ADDRESS,
        MC33775_I2C_CFG_OFFSET,
        (MC33775_I2C_CFG_EN_ENABLED_ENUM_VAL << MC33775_I2C_CFG_EN_POS) +
            (MC33775_I2C_CFG_CLKSEL_F_400K_ENUM_VAL << MC33775_I2C_CFG_CLKSEL_POS),
        pState->pSpiTxSequence);
}

extern STD_RETURN_TYPE_e N775_TransmitI2c(N775_STATE_s *pState) {
    FAS_ASSERT(pState != NULL_PTR);
    uint8_t i2cAddressByte                    = 0u;
    uint16_t readValue                        = 0u;
    uint16_t tries                            = 0u;
    STD_RETURN_TYPE_e retVal                  = STD_OK;
    N775_COMMUNICATION_STATUS_e returnedValue = N775_COMMUNICATION_OK;
    AFE_I2C_QUEUE_s transactionData;

    if (ftsk_allQueuesCreated == true) {
        if (OS_ReceiveFromQueue(ftsk_afeToI2cQueue, (void *)&transactionData, 0u) == OS_SUCCESS) {
            /* Queue was not empty */
            i2cAddressByte            = (transactionData.deviceAddress << 1u) & 0xFEu; /* I2C address has only 7 bits */
            uint8_t nack              = 0u;
            uint8_t registerIncrement = 0u;
            uint16_t registerStartAddress = 0u;
            uint16_t dataToWrite          = 0u;
            uint8_t byteIndex             = 0u;
            uint8_t bytesWritten          = 0u;
            uint8_t msbIndex              = 1u;
            uint8_t lsbIndex              = 1u;

            switch (transactionData.transferType) {
                case AFE_I2C_TRANSFER_TYPE_WRITE:
                    transactionData.transferType = AFE_I2C_TRANSFER_TYPE_WRITE_FAIL;
                    /* First prepare data to send on I2C bus in registers */
                    i2cAddressByte |= N775_I2C_WRITE;
                    /* Set I2C device address for write access followed by first byte of data */
                    N775_CommunicationWrite(
                        transactionData.module + 1u,
                        MC33775_I2C_DATA0_OFFSET,
                        (i2cAddressByte << MC33775_I2C_DATA0_BYTE0_POS) |
                            (transactionData.writeData[0u] << MC33775_I2C_DATA0_BYTE1_POS),
                        pState->pSpiTxSequence);

                    if (transactionData.writeDataLength > 1u) {
                        /* Now set data to be written to I2C device */
                        registerStartAddress = MC33775_I2C_DATA1_OFFSET;
                        registerIncrement    = 0u;
                        byteIndex            = 1u;
                        dataToWrite          = 0u;
                        while (byteIndex < transactionData.writeDataLength) {
                            /* Each subsequent data register contains two bytes of read data */

                            if ((byteIndex % 2u) != 0u) {
                                dataToWrite |= (uint16_t)(transactionData.writeData[byteIndex]) & 0xFFu;
                                bytesWritten++;
                            } else {
                                dataToWrite |= ((uint16_t)(transactionData.writeData[byteIndex]) << 8u) & 0xFF00u;
                                bytesWritten++;
                            }
                            byteIndex++;
                            if ((bytesWritten == 2u) || (byteIndex == transactionData.writeDataLength)) {
                                N775_CommunicationWrite(
                                    transactionData.module + 1u,
                                    registerStartAddress + registerIncrement,
                                    dataToWrite,
                                    pState->pSpiTxSequence);
                                registerIncrement++;
                                bytesWritten = 0u;
                                dataToWrite  = 0u;
                            }
                        }
                    }
                    /* Data to write ready, now start transmission */
                    /* Write into the control register to start transaction */
                    N775_CommunicationWrite(
                        transactionData.module + 1u,
                        MC33775_I2C_CTRL_OFFSET,
                        /* transactionData.writeDataLength + 1u: data + I2C device address byte */
                        ((transactionData.writeDataLength + 1u) << MC33775_I2C_CTRL_START_POS) |
                            ((MC33775_I2C_CTRL_STPAFTER_STOP_ENUM_VAL << MC33775_I2C_CTRL_STPAFTER_POS) +
                             (0u << MC33775_I2C_CTRL_RDAFTER_POS)),
                        pState->pSpiTxSequence);
                    /* Wait until transaction ends */
                    tries = N775_FLAG_READY_TRIES;
                    do {
                        returnedValue = N775_CommunicationRead(
                            transactionData.module + 1u, MC33775_I2C_STAT_OFFSET, &readValue, pState);
                        tries--;
                        N775_Wait(2u);
                    } while ((readValue & MC33775_I2C_STAT_PENDING_MSK) && (returnedValue == N775_COMMUNICATION_OK) &&
                             (tries > 0u));

                    if ((returnedValue == N775_COMMUNICATION_OK) && (tries > 0u)) {
                        retVal                       = STD_OK;
                        transactionData.transferType = AFE_I2C_TRANSFER_TYPE_WRITE_SUCCESS;
                    }
                    if (OS_SendToBackOfQueue(ftsk_afeFromI2cQueue, (void *)&transactionData, 0u) == OS_SUCCESS) {
                        /* Queue is not full */
                    } else {
                        /* Queue is full */
                        retVal = STD_NOT_OK;
                    }
                    break;

                case AFE_I2C_TRANSFER_TYPE_READ:
                    transactionData.transferType = AFE_I2C_TRANSFER_TYPE_READ_FAIL;
                    i2cAddressByte |= N775_I2C_READ;
                    /* First prepare address to send on I2C bus in registers */
                    N775_CommunicationWrite(
                        transactionData.module + 1u,
                        MC33775_I2C_DATA0_OFFSET,
                        i2cAddressByte << MC33775_I2C_DATA0_BYTE0_POS,
                        pState->pSpiTxSequence);
                    /* Write into the control register to start transaction */
                    /* Stop condition after transfer, no repeated start */
                    N775_CommunicationWrite(
                        transactionData.module + 1u,
                        MC33775_I2C_CTRL_OFFSET,
                        /* transactionData.dataLength + 1u: data + I2C device address byte */
                        ((transactionData.readDataLength + 1u) << MC33775_I2C_CTRL_START_POS) |
                            ((1u << MC33775_I2C_CTRL_STPAFTER_POS) + (0u << MC33775_I2C_CTRL_RDAFTER_POS)),
                        pState->pSpiTxSequence);
                    /* Wait until transaction ends */
                    tries = N775_FLAG_READY_TRIES;
                    do {
                        returnedValue = N775_CommunicationRead(
                            transactionData.module + 1u, MC33775_I2C_STAT_OFFSET, &readValue, pState);
                        tries--;
                        N775_Wait(2u);
                    } while ((readValue & MC33775_I2C_STAT_PENDING_MSK) && (returnedValue == N775_COMMUNICATION_OK) &&
                             (tries > 0u));
                    /* Now retrieve read data */
                    nack = readValue & MC33775_I2C_STAT_NACKRCV_MSK;
                    if ((returnedValue == N775_COMMUNICATION_OK) && (tries > 0u) && (nack == 0u)) {
                        uint16_t readData[13u] = {0u};
                        /* In data registers, bytes0 contains the address, read data begins at byte1 */
                        /* First data register contains byte1, second data register byte2 and byte3, ... */
                        uint16_t nrOfRegisterToRead = (transactionData.readDataLength / 2u) + 1u;
                        returnedValue               = N775_CommunicationReadMultiple(
                            transactionData.module + 1u,
                            nrOfRegisterToRead,
                            4u,
                            MC33775_I2C_DATA0_OFFSET,
                            readData,
                            pState);

                        /* First data register only contains one byte of the read data */
                        transactionData.readData[0u] = (uint8_t)((readData[0u] & 0xFF00) >> 8u);
                        byteIndex                    = 1u;
                        msbIndex                     = 1u;
                        lsbIndex                     = 1u;
                        while (byteIndex < transactionData.readDataLength) {
                            /* Each subsequent data register contains two bytes of read data */
                            if ((byteIndex % 2u) != 0u) {
                                transactionData.readData[byteIndex] = (uint8_t)(readData[lsbIndex] & 0xFFu);
                                lsbIndex++;
                            } else {
                                transactionData.readData[byteIndex] = (uint8_t)((readData[msbIndex] & 0xFF00u) >> 8u);
                                msbIndex++;
                            }
                            byteIndex++;
                        }
                        retVal                       = STD_OK;
                        transactionData.transferType = AFE_I2C_TRANSFER_TYPE_READ_SUCCESS;
                    }
                    if (OS_SendToBackOfQueue(ftsk_afeFromI2cQueue, (void *)&transactionData, 0u) == OS_SUCCESS) {
                        /* Queue is not full */
                    } else {
                        /* Queue is full */
                        retVal = STD_NOT_OK;
                    }
                    break;

                case AFE_I2C_TRANSFER_TYPE_WRITEREAD:
                    transactionData.transferType = AFE_I2C_TRANSFER_TYPE_READ_FAIL;
                    /* First prepare address to send on I2C bus in registers */
                    N775_CommunicationWrite(
                        transactionData.module + 1u,
                        MC33775_I2C_DATA0_OFFSET,
                        ((i2cAddressByte | N775_I2C_WRITE) << MC33775_I2C_DATA0_BYTE0_POS) |
                            (transactionData.writeData[0u] << MC33775_I2C_DATA0_BYTE1_POS),
                        pState->pSpiTxSequence);

                    if (transactionData.writeDataLength > 1u) {
                        /* Now set data to be written to I2C device */
                        registerStartAddress                                       = MC33775_I2C_DATA1_OFFSET;
                        registerIncrement                                          = 0u;
                        byteIndex                                                  = 1u;
                        dataToWrite                                                = 0u;
                        transactionData.writeData[transactionData.writeDataLength] = i2cAddressByte | N775_I2C_READ;
                        while (byteIndex < (transactionData.writeDataLength + 1)) {
                            /* Each subsequent data register contains two bytes of read data */

                            if ((byteIndex % 2u) != 0u) {
                                dataToWrite |= (uint16_t)(transactionData.writeData[byteIndex]) & 0xFFu;
                                bytesWritten++;
                            } else {
                                dataToWrite |= ((uint16_t)(transactionData.writeData[byteIndex]) << 8u) & 0xFF00u;
                                bytesWritten++;
                            }
                            byteIndex++;
                            if ((bytesWritten == 2u) || (byteIndex == (transactionData.writeDataLength + 1))) {
                                N775_CommunicationWrite(
                                    transactionData.module + 1u,
                                    registerStartAddress + registerIncrement,
                                    dataToWrite,
                                    pState->pSpiTxSequence);
                                registerIncrement++;
                                bytesWritten = 0u;
                                dataToWrite  = 0u;
                            }
                        }
                    } else {
                        N775_CommunicationWrite(
                            transactionData.module + 1u,
                            MC33775_I2C_DATA1_OFFSET,
                            ((i2cAddressByte | N775_I2C_READ) << MC33775_I2C_DATA1_BYTE2_POS),
                            pState->pSpiTxSequence);
                    }

                    /* Write into the control register to start transaction */
                    /* Stop condition after transfer, repeated start */
                    N775_CommunicationWrite(
                        transactionData.module + 1u,
                        MC33775_I2C_CTRL_OFFSET,
                        /* transaction length: I2C device address byte for write + data to write
                                   + I2C device address byte for read + data to read */
                        ((transactionData.writeDataLength + transactionData.readDataLength + 2u)
                         << MC33775_I2C_CTRL_START_POS) |
                            ((1u << MC33775_I2C_CTRL_STPAFTER_POS) +
                             ((1u + transactionData.writeDataLength) << MC33775_I2C_CTRL_RDAFTER_POS)),
                        pState->pSpiTxSequence);
                    /* Wait until transaction ends */
                    tries = N775_FLAG_READY_TRIES;
                    do {
                        returnedValue = N775_CommunicationRead(
                            transactionData.module + 1u, MC33775_I2C_STAT_OFFSET, &readValue, pState);
                        tries--;
                        N775_Wait(2u);
                    } while ((readValue & MC33775_I2C_STAT_PENDING_MSK) && (returnedValue == N775_COMMUNICATION_OK) &&
                             (tries > 0u));
                    /* Now retrieve read data */
                    nack = readValue & MC33775_I2C_STAT_NACKRCV_MSK;
                    if ((returnedValue == N775_COMMUNICATION_OK) && (tries > 0u) && (nack == 0u)) {
                        uint16_t readData[13u] = {0u};
                        /* First data to read is at least in this register */
                        uint16_t registerOffset = MC33775_I2C_DATA0_OFFSET;
                        /* Find offset of first register to read */
                        registerOffset += (transactionData.writeDataLength + 2u) / 2u;
                        /* In data registers, byte0 contains the device address, byte1 the first byte written */
                        /* I2C device address byte for read is present before the first read byte */
                        uint16_t nrOfRegisterToRead = 0u;
                        if ((transactionData.writeDataLength % 2u) == 0u) {
                            nrOfRegisterToRead = (transactionData.readDataLength + 1u) / 2u;
                        } else {
                            nrOfRegisterToRead = (transactionData.readDataLength / 2u) + 1u;
                        }

                        returnedValue = N775_CommunicationReadMultiple(
                            transactionData.module + 1u, nrOfRegisterToRead, 4u, registerOffset, readData, pState);

                        /* Second data register only contains one byte of the read data (byte3)
                           Read data starts at second register because:
                           byte0: I2C device address for write access
                           byte1: first byte written
                           I2C device address byte for read is present before the first read byte
                           */
                        if (((transactionData.writeDataLength + 1) % 2u) != 0u) {
                            byteIndex = 0u;
                            msbIndex  = 0u;
                            lsbIndex  = 0u;
                            while (byteIndex < transactionData.readDataLength) {
                                /* Each subsequent data register contains two bytes of read data */
                                if ((byteIndex % 2u) == 0u) {
                                    transactionData.readData[byteIndex] = (uint8_t)(readData[lsbIndex] & 0xFFu);
                                    lsbIndex++;
                                } else {
                                    transactionData.readData[byteIndex] =
                                        (uint8_t)((readData[msbIndex] & 0xFF00u) >> 8u);
                                    msbIndex++;
                                }
                                byteIndex++;
                            }
                        } else {
                            byteIndex = 0u;
                            msbIndex  = 0u;
                            lsbIndex  = 1u;
                            while (byteIndex < transactionData.readDataLength) {
                                /* Each subsequent data register contains two bytes of read data */
                                if ((byteIndex % 2u) != 0u) {
                                    transactionData.readData[byteIndex] = (uint8_t)(readData[lsbIndex] & 0xFFu);
                                    lsbIndex++;
                                } else {
                                    transactionData.readData[byteIndex] =
                                        (uint8_t)((readData[msbIndex] & 0xFF00u) >> 8u);
                                    msbIndex++;
                                }
                                byteIndex++;
                            }
                        }
                        retVal                       = STD_OK;
                        transactionData.transferType = AFE_I2C_TRANSFER_TYPE_READ_SUCCESS;
                    }
                    if (OS_SendToBackOfQueue(ftsk_afeFromI2cQueue, (void *)&transactionData, 0u) == OS_SUCCESS) {
                        /* Queue is not full */
                    } else {
                        /* Queue is full */
                        retVal = STD_NOT_OK;
                    }
                    break;

                default:
                    /* Invalid value transfer type */
                    FAS_ASSERT(FAS_TRAP);
                    break;
            }
        }
    }

    return retVal;
}

extern STD_RETURN_TYPE_e N775_I2cRead(uint8_t module, uint8_t deviceAddress, uint8_t *pData, uint8_t dataLength) {
    FAS_ASSERT(pData != NULL_PTR);
    FAS_ASSERT((dataLength > 0u) && (dataLength <= 13u));
    STD_RETURN_TYPE_e retVal = STD_NOT_OK;
    AFE_I2C_QUEUE_s transactionData;

    transactionData.module         = module;
    transactionData.deviceAddress  = deviceAddress;
    transactionData.readDataLength = dataLength;
    transactionData.transferType   = AFE_I2C_TRANSFER_TYPE_READ;
    if (OS_SendToBackOfQueue(ftsk_afeToI2cQueue, (void *)&transactionData, 0u) == OS_SUCCESS) {
        /* queue is not full */
        /* waiting for transfer to finish */
        if (OS_ReceiveFromQueue(ftsk_afeFromI2cQueue, (void *)&transactionData, N775_I2C_FINISHED_TIMEOUT_ms) ==
            OS_SUCCESS) {
            if (transactionData.transferType == AFE_I2C_TRANSFER_TYPE_READ_SUCCESS) {
                for (uint8_t i = 0u; i < dataLength; i++) {
                    pData[i] = transactionData.readData[i];
                }
                retVal = STD_OK;
            }
        }
    } else {
        /* queue is full */
    }

    return retVal;
}

extern STD_RETURN_TYPE_e N775_I2cWrite(uint8_t module, uint8_t deviceAddress, uint8_t *pData, uint8_t dataLength) {
    FAS_ASSERT(pData != NULL_PTR);
    FAS_ASSERT((dataLength > 0u) && (dataLength <= 13u));
    STD_RETURN_TYPE_e retVal = STD_NOT_OK;
    AFE_I2C_QUEUE_s transactionData;

    transactionData.module          = module;
    transactionData.deviceAddress   = deviceAddress;
    transactionData.writeDataLength = dataLength;
    transactionData.transferType    = AFE_I2C_TRANSFER_TYPE_WRITE;
    for (uint8_t i = 0u; i < dataLength; i++) {
        transactionData.writeData[i] = pData[i];
    }
    if (OS_SendToBackOfQueue(ftsk_afeToI2cQueue, (void *)&transactionData, 0u) == OS_SUCCESS) {
        /* queue is not full */
        /* waiting for transfer to finish */
        if (OS_ReceiveFromQueue(ftsk_afeFromI2cQueue, (void *)&transactionData, N775_I2C_FINISHED_TIMEOUT_ms) ==
            OS_SUCCESS) {
            if (transactionData.transferType == AFE_I2C_TRANSFER_TYPE_WRITE_SUCCESS) {
                retVal = STD_OK;
            }
        }
    } else {
        /* queue is full */
    }

    return retVal;
}

extern STD_RETURN_TYPE_e N775_I2cWriteRead(
    uint8_t module,
    uint8_t deviceAddress,
    uint8_t *pDataWrite,
    uint8_t writeDataLength,
    uint8_t *pDataRead,
    uint8_t readDataLength) {
    FAS_ASSERT(pDataWrite != NULL_PTR);
    FAS_ASSERT(pDataRead != NULL_PTR);
    FAS_ASSERT(writeDataLength > 0u);
    FAS_ASSERT(readDataLength > 0u);
    FAS_ASSERT((uint16_t)(writeDataLength + readDataLength) <= 12u);
    STD_RETURN_TYPE_e retVal = STD_NOT_OK;
    AFE_I2C_QUEUE_s transactionData;

    transactionData.module          = module;
    transactionData.deviceAddress   = deviceAddress;
    transactionData.writeDataLength = writeDataLength;
    transactionData.readDataLength  = readDataLength;
    transactionData.transferType    = AFE_I2C_TRANSFER_TYPE_WRITEREAD;
    for (uint8_t i = 0u; i < writeDataLength; i++) {
        transactionData.writeData[i] = pDataWrite[i];
    }
    if (OS_SendToBackOfQueue(ftsk_afeToI2cQueue, (void *)&transactionData, 0u) == OS_SUCCESS) {
        /* queue is not full */
        /* waiting for transfer to finish */
        if (OS_ReceiveFromQueue(ftsk_afeFromI2cQueue, (void *)&transactionData, N775_I2C_FINISHED_TIMEOUT_ms) ==
            OS_SUCCESS) {
            if (transactionData.transferType == AFE_I2C_TRANSFER_TYPE_READ_SUCCESS) {
                for (uint8_t i = 0u; i < readDataLength; i++) {
                    pDataRead[i] = transactionData.readData[i];
                }
                retVal = STD_OK;
            }
        }
    } else {
        /* queue is full */
    }

    return retVal;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
extern void TEST_N775_InitializeI2c(N775_STATE_s *pState) {
    N775_InitializeI2c(pState);
}
extern STD_RETURN_TYPE_e TEST_N775_TransmitI2c(N775_STATE_s *pState) {
    return N775_TransmitI2c(pState);
}
#endif
