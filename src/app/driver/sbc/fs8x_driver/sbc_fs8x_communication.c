/*
 * Copyright (c) 2016 - 2018, NXP Semiconductors, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/** @file sbc_fs8x_communication.c
 * @brief Implementation of communication logic for NXP SBC FS8x.
 *
 * @author nxf44615
 * @version 1.1
 * @date 9-Oct-2018
 * @copyright Copyright (c) 2016 - 2018, NXP Semiconductors, Inc.
 *
 * @updated 2020-03-18 (date of last update)
 * Adapted driver to pass interface configuration as parameter into the
 * functions to enable the usage of multiple ICs in the system.
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include <stdint.h>
#include <stddef.h>

#include "sbc_fs8x_map.h"
#include "sbc_fs8x_assert.h"
#include "sbc_fs8x_common.h"
#include "sbc_fs8x_communication.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* Data frame (SPI or I2C). */
/* #define FS8x_COMM_FRAME_SIZE (0x04U) */ /*!< Length of the communication frame */ /* Moved to sbc_fs8x_communication.h file */
#define FS8x_REG_ADDR_MASK 0x3FU    /*!< Register address mask */
#define FS8x_REG_ADDR_SHIFT 0x01U   /*!< SPI register address shift */

/* I2C frame. */
#define FS8x_I2C_READ_FRAME_LENGTH 1U   /*!< Length of the data frame for I2C read command. */
#define FS8x_I2C_ADDRESS_BASE 0x20U     /*!< I2C device base address */
#define FS8x_I2C_FRAME_SIZE 0x05U       /*!< Length of the complete I2C frame */
#define FS8x_I2C_RX_SIZE 0x03U          /*!< Length of the received I2C data frame */

/* CRC polynomial used for SPI and I2C communication. */
#define FS8x_CRC_TBL_SIZE    256U   /*!< Size of CRC table. */
#define FS8x_COM_CRC_POLYNOM 0x1DU  /*!< CRC polynom. */
#define FS8x_COM_CRC_INIT 0xFFU     /*!< CRC initial value. */

/*******************************************************************************
 * Constants
 ******************************************************************************/

/** @brief CRC lookup table. */
static const uint8_t FS8x_CRC_TABLE[FS8x_CRC_TBL_SIZE] = {
    0x00U, 0x1DU, 0x3AU, 0x27U, 0x74U, 0x69U, 0x4EU, 0x53U, 0xE8U, 0xF5U, 0xD2U, 0xCFU, 0x9CU,
    0x81U, 0xA6U, 0xBBU, 0xCDU, 0xD0U, 0xF7U, 0xEAU, 0xB9U, 0xA4U, 0x83U, 0x9EU, 0x25U, 0x38U,
    0x1FU, 0x02U, 0x51U, 0x4CU, 0x6BU, 0x76U, 0x87U, 0x9AU, 0xBDU, 0xA0U, 0xF3U, 0xEEU, 0xC9U,
    0xD4U, 0x6FU, 0x72U, 0x55U, 0x48U, 0x1BU, 0x06U, 0x21U, 0x3CU, 0x4AU, 0x57U, 0x70U, 0x6DU,
    0x3EU, 0x23U, 0x04U, 0x19U, 0xA2U, 0xBFU, 0x98U, 0x85U, 0xD6U, 0xCBU, 0xECU, 0xF1U, 0x13U,
    0x0EU, 0x29U, 0x34U, 0x67U, 0x7AU, 0x5DU, 0x40U, 0xFBU, 0xE6U, 0xC1U, 0xDCU, 0x8FU, 0x92U,
    0xB5U, 0xA8U, 0xDEU, 0xC3U, 0xE4U, 0xF9U, 0xAAU, 0xB7U, 0x90U, 0x8DU, 0x36U, 0x2BU, 0x0CU,
    0x11U, 0x42U, 0x5FU, 0x78U, 0x65U, 0x94U, 0x89U, 0xAEU, 0xB3U, 0xE0U, 0xFDU, 0xDAU, 0xC7U,
    0x7CU, 0x61U, 0x46U, 0x5BU, 0x08U, 0x15U, 0x32U, 0x2FU, 0x59U, 0x44U, 0x63U, 0x7EU, 0x2DU,
    0x30U, 0x17U, 0x0AU, 0xB1U, 0xACU, 0x8BU, 0x96U, 0xC5U, 0xD8U, 0xFFU, 0xE2U, 0x26U, 0x3BU,
    0x1CU, 0x01U, 0x52U, 0x4FU, 0x68U, 0x75U, 0xCEU, 0xD3U, 0xF4U, 0xE9U, 0xBAU, 0xA7U, 0x80U,
    0x9DU, 0xEBU, 0xF6U, 0xD1U, 0xCCU, 0x9FU, 0x82U, 0xA5U, 0xB8U, 0x03U, 0x1EU, 0x39U, 0x24U,
    0x77U, 0x6AU, 0x4DU, 0x50U, 0xA1U, 0xBCU, 0x9BU, 0x86U, 0xD5U, 0xC8U, 0xEFU, 0xF2U, 0x49U,
    0x54U, 0x73U, 0x6EU, 0x3DU, 0x20U, 0x07U, 0x1AU, 0x6CU, 0x71U, 0x56U, 0x4BU, 0x18U, 0x05U,
    0x22U, 0x3FU, 0x84U, 0x99U, 0xBEU, 0xA3U, 0xF0U, 0xEDU, 0xCAU, 0xD7U, 0x35U, 0x28U, 0x0FU,
    0x12U, 0x41U, 0x5CU, 0x7BU, 0x66U, 0xDDU, 0xC0U, 0xE7U, 0xFAU, 0xA9U, 0xB4U, 0x93U, 0x8EU,
    0xF8U, 0xE5U, 0xC2U, 0xDFU, 0x8CU, 0x91U, 0xB6U, 0xABU, 0x10U, 0x0DU, 0x2AU, 0x37U, 0x64U,
    0x79U, 0x5EU, 0x43U, 0xB2U, 0xAFU, 0x88U, 0x95U, 0xC6U, 0xDBU, 0xFCU, 0xE1U, 0x5AU, 0x47U,
    0x60U, 0x7DU, 0x2EU, 0x33U, 0x14U, 0x09U, 0x7FU, 0x62U, 0x45U, 0x58U, 0x0BU, 0x16U, 0x31U,
    0x2CU, 0x97U, 0x8AU, 0xADU, 0xB0U, 0xE3U, 0xFEU, 0xD9U, 0xC4U
};

/*******************************************************************************
 * Local Functions Prototypes
 ******************************************************************************/

/** @brief This function calculates CRC value of passed data array.
 *         Takes bytes in inverted order due to frame format.
 * @param [in] data     Data array.
 * @param [in] dataLen  Length of the data array.
 * @return CRC8 */
static uint8_t FS8x_CalcCRC(const uint8_t* data, uint8_t dataLen);

/** @brief Performs CRC check of the data array.
 *
 * CRC is expected in the first array item (index 0).
 * @param [in] data     Data array.
 * @param [in] dataLen  Length of the data array.
 * @return Returns @ref fs8xStatusOk if CRC is correct. */
static fs8x_status_t FS8x_CheckCRC(const uint8_t* data, uint8_t dataLen);

#if (FS8x_COMM_TYPE == FS8x_COMM_SPI || FS8x_COMM_TYPE == FS8x_COMM_BOTH)

/** @brief Performs SPI transfer of the txData. Received frame is saved into
 *         rxData structure.
 * @param [in,out] pSpiInterface  SPI communication interface config
 * @param [in]  txData           TX data.
 * @param [out] rxData           RX data. Can be NULL in case of write-only command.
 * @return @ref fs8x_status_t "Status return code." */
static fs8x_status_t FS8x_SPI_TransferData(SPI_INTERFACE_CONFIG_s* pSpiInterface, fs8x_tx_frame_t* txData,
        fs8x_rx_frame_t* rxData);

/** @brief Creates a raw frame for SPI transfer.
 * @param [in]  txData  TX data.
 * @param [out] txFrame TX frame for SPI transfer. */
static void FS8x_SPI_CreateSendFrame(fs8x_tx_frame_t* txData, uint8_t* txFrame);

#endif

#if (FS8x_COMM_TYPE == FS8x_COMM_I2C || FS8x_COMM_TYPE == FS8x_COMM_BOTH)

/** @brief Performs I2C register read.
 * @param [in]  drvData Driver run-time data.
 * @param [in]  txData TX data.
 * @param [out] rxData RX data.
 * @return @ref fs8x_status_t "Status return code." */
static fs8x_status_t FS8x_I2C_ReadRegister(fs8x_drv_data_t* drvData, fs8x_tx_frame_t* txData,
        fs8x_rx_frame_t* rxData);

/** @brief Performs I2C register write.
 * @param [in] drvData Driver run-time data.
 * @param [in] txData TX data.
 * @return @ref fs8x_status_t "Status return code." */
static fs8x_status_t FS8x_I2C_WriteRegister(fs8x_drv_data_t* drvData, fs8x_tx_frame_t* txData);

/** @brief Returns 7-bit I2C address.
 * @param [in] drvData  Driver run-time data.
 * @param [in] txData   TX data.
 * @return FS8x I2C address. */
static uint8_t GetI2CAddress(fs8x_drv_data_t* drvData, fs8x_tx_frame_t* txData);

#endif

/*******************************************************************************
 * Local Variables
 ******************************************************************************/

/*******************************************************************************
 * Global Variables
 ******************************************************************************/

/*******************************************************************************
 * Local Functions - Implementation
 ******************************************************************************/

#if (FS8x_COMM_TYPE == FS8x_COMM_SPI || FS8x_COMM_TYPE == FS8x_COMM_BOTH)

/* Performs SPI transfer of the txData. Received frame is saved into
 * rxData structure. */
static fs8x_status_t FS8x_SPI_TransferData(SPI_INTERFACE_CONFIG_s* pSpiInterface, fs8x_tx_frame_t* txData,
        fs8x_rx_frame_t* rxData)
{
    fs8x_status_t status = fs8xStatusOk;
    uint8_t txFrame[FS8x_COMM_FRAME_SIZE] = {0};
    uint8_t rxFrame[FS8x_COMM_FRAME_SIZE] = {0};

    FS_ASSERT(txData != NULL);

    FS8x_SPI_CreateSendFrame(txData, txFrame);
    status = MCU_SPI_TransferData(pSpiInterface, txFrame, FS8x_COMM_FRAME_SIZE, rxFrame);
    if (status != fs8xStatusOk)
    {
        return status;
    }

    status = FS8x_CheckCRC(rxFrame, FS8x_COMM_FRAME_SIZE);
    if (status != fs8xStatusOk)
    {
        return status;
    }

    if (rxData != NULL)
    {
        rxData->deviceStatus = rxFrame[3];
        rxData->readData = (uint16_t)(rxFrame[2] << 8U | rxFrame[1]);
    }
    return status;
}

/* Creates a raw frame for SPI transfer. */
static void FS8x_SPI_CreateSendFrame(fs8x_tx_frame_t* txData, uint8_t* txFrame)
{
    FS_ASSERT(txData != NULL);
    FS_ASSERT(txFrame != NULL);

    /* Selects register set (main x fail-safe). */
    txFrame[3] |= txData->isFailSafe ? 0x80U : 0x00U;

    /* Sets address of the register. */
    txFrame[3] |= FS8x_BO_SETVAL_EXT(txFrame[3], txData->registerAddress, \
            FS8x_REG_ADDR_MASK, FS8x_REG_ADDR_SHIFT);

    /* check if register is r or w */
    switch (txData->commandType)
    {
      case fs8xRegRead:
          /* Creates read command. */
          txFrame[3] = FS8x_BO_SETVAL(txFrame[3], 0x00U, 0x01U);
          /* No data to be sent. */
          txFrame[2] = 0x00;
          txFrame[1] = 0x00;
          break;

      case fs8xRegWrite:
          /* Creates write command. */
          txFrame[3] = FS8x_BO_SETVAL(txFrame[3], 0x01U, 0x01U);
          /* Sets data - MSB first. */
          txFrame[2] = (uint8_t)(txData->writeData >> 8);
          txFrame[1] = (uint8_t)(txData->writeData);
          break;
    }

    /* Sets CRC. */
    txFrame[0] = FS8x_CalcCRC(txFrame, FS8x_COMM_FRAME_SIZE);
}

#endif

#if (FS8x_COMM_TYPE == FS8x_COMM_I2C || FS8x_COMM_TYPE == FS8x_COMM_BOTH)

/* Performs I2C register read. */
static fs8x_status_t FS8x_I2C_ReadRegister(fs8x_drv_data_t* drvData, fs8x_tx_frame_t* txData,
        fs8x_rx_frame_t* rxData)
{
    fs8x_status_t status = fs8xStatusOk;
    uint8_t i2cAddress;
    uint8_t txFrame[FS8x_I2C_READ_FRAME_LENGTH] = {0};
    uint8_t rxFrame[FS8x_I2C_FRAME_SIZE] = {0};

    FS_ASSERT(drvData != NULL);
    FS_ASSERT(txData != NULL);
    FS_ASSERT(rxData != NULL);

    i2cAddress = GetI2CAddress(drvData, txData);
    txFrame[0] = txData->registerAddress;

    /* Send I2C write command with register address only */
    status |= MCU_I2C_SendData(txFrame, FS8x_I2C_READ_FRAME_LENGTH, i2cAddress);
    if (status != fs8xStatusOk)
    {
        return status;
    }

    /* Read I2C response. */
    status |= MCU_I2C_ReceiveData(FS8x_I2C_RX_SIZE, i2cAddress, rxFrame);
    if (status != fs8xStatusOk)
    {
        return status;
    }

    /* I2C response frame contains just data and CRC.
     * The Device address and Register address is added
     * for proper CRC check. */
    rxFrame[3] = txData->registerAddress;
    rxFrame[4] = (uint8_t)((i2cAddress << 1U) | 0x01U); /* R/W bit is 1 for I2C read */

    status = FS8x_CheckCRC(rxFrame, FS8x_I2C_FRAME_SIZE);
    if (status != fs8xStatusOk)
    {
        return status;
    }
    rxData->readData = (uint16_t)(rxFrame[2] << 8U | rxFrame[1]);

    return status;
}

/* Performs I2C register write. */
static fs8x_status_t FS8x_I2C_WriteRegister(fs8x_drv_data_t* drvData, fs8x_tx_frame_t* txData)
{
    uint8_t txFrame[FS8x_I2C_FRAME_SIZE] = {0};
    uint8_t i2cAddress = FS8x_I2C_ADDRESS_BASE;

    FS_ASSERT(drvData != NULL);
    FS_ASSERT(txData != NULL);

    i2cAddress = GetI2CAddress(drvData, txData);

    txFrame[4] = (uint8_t)(i2cAddress << 1U); /* R/W bit is 0 for write */

    /* Sets address of the register (first two bits are ignored). */
    txFrame[3] = txData->registerAddress & FS8x_REG_ADDR_MASK;

    /* Sets data - MSB first. */
    txFrame[2] = (uint8_t)(txData->writeData >> 8);
    txFrame[1] = (uint8_t)(txData->writeData);

    /* Sets CRC. */
    txFrame[0] = FS8x_CalcCRC(txFrame, FS8x_I2C_FRAME_SIZE);

    return MCU_I2C_SendData(txFrame, FS8x_COMM_FRAME_SIZE, i2cAddress);
}

/* Returns 7-bit I2C address. */
static uint8_t GetI2CAddress(fs8x_drv_data_t* drvData, fs8x_tx_frame_t* txData)
{
    uint8_t i2cAddress = FS8x_I2C_ADDRESS_BASE;

    FS_ASSERT(drvData != NULL);
    FS_ASSERT(txData != NULL);

    i2cAddress = FS8x_BO_SETVAL(i2cAddress, drvData->i2cAddressOtp << 1, 0x1EU);
    if (txData->isFailSafe)
    {
        /* Set bit 33 if register is FS. */
        i2cAddress = FS8x_BO_SETVAL(i2cAddress, 0x01U, 0x01U);
    }
    return i2cAddress;
}

#endif

/* This function calculates CRC value of passed data array.
 * Takes bytes in inverted order due to frame format. */
static uint8_t FS8x_CalcCRC(const uint8_t* data, uint8_t dataLen)
{
    uint8_t crc;      /* Result. */
    uint8_t tableIdx; /* Index to the CRC table. */
    uint8_t dataIdx;  /* Index to the data array (memory). */

    FS_ASSERT(data != NULL);
    FS_ASSERT(dataLen > 0);

    /* Set CRC seed value. */
    crc = FS8x_COM_CRC_INIT;

    for (dataIdx = dataLen - 1; dataIdx > 0; dataIdx--)
    {
        tableIdx = crc ^ data[dataIdx];
        crc = FS8x_CRC_TABLE[tableIdx];
    }
    return crc;
}

/* Performs CRC check of the data array. */
static fs8x_status_t FS8x_CheckCRC(const uint8_t* data, uint8_t dataLen)
{
    uint8_t frameCrc;  /* CRC value from resp. */
    uint8_t compCrc;   /* Computed CRC value. */

    FS_ASSERT(data != NULL);
    FS_ASSERT(dataLen > 0);

    /* Check CRC. */
    frameCrc = data[0];
    compCrc = FS8x_CalcCRC(data, dataLen);
    return (compCrc != frameCrc) ? fs8xStatusError : fs8xStatusOk;
}

/*******************************************************************************
 * API - Implementation
 ******************************************************************************/

/* Performs a single read register based on provided address.
 * The response is returned in @ref fs8x_rx_frame_t structure. */
fs8x_status_t FS8x_ReadRegister(SPI_INTERFACE_CONFIG_s* pSpiInterface, fs8x_drv_data_t* drvData, bool isFailSafe,
        uint8_t address, fs8x_rx_frame_t* rxData)
{
    fs8x_tx_frame_t txData = {
        .registerAddress = address,
        .commandType = fs8xRegRead,
        .isFailSafe = isFailSafe };

    FS_ASSERT(drvData != NULL);
    FS_ASSERT(rxData != NULL);

    switch (drvData->communicationMode)
    {
#if FS8x_COMM_TYPE == FS8x_COMM_SPI || FS8x_COMM_TYPE == FS8x_COMM_BOTH
        case fs8xSPI:
            return FS8x_SPI_TransferData(pSpiInterface, &txData, rxData);
#endif
#if FS8x_COMM_TYPE == FS8x_COMM_I2C || FS8x_COMM_TYPE == FS8x_COMM_BOTH
        case fs8xI2C:
            return FS8x_I2C_ReadRegister(drvData, &txData, rxData);
#endif
        default:
            return fs8xStatusError;
    }
}

/* Sends write command to the FS8x. */
fs8x_status_t FS8x_WriteRegister(SPI_INTERFACE_CONFIG_s* pSpiInterface, fs8x_drv_data_t* drvData, bool isFailSafe,
        uint8_t address, uint16_t writeData)
{
    fs8x_tx_frame_t txData = {
        .registerAddress = address,
        .writeData = writeData,
        .commandType = fs8xRegWrite,
        .isFailSafe = isFailSafe };

    FS_ASSERT(drvData != NULL);

    switch (drvData->communicationMode)
    {
#if FS8x_COMM_TYPE == FS8x_COMM_SPI || FS8x_COMM_TYPE == FS8x_COMM_BOTH
        case fs8xSPI:
            return FS8x_SPI_TransferData(pSpiInterface, &txData, NULL);
#endif
#if FS8x_COMM_TYPE == FS8x_COMM_I2C || FS8x_COMM_TYPE == FS8x_COMM_BOTH
        case fs8xI2C:
            return FS8x_I2C_WriteRegister(drvData, &txData);
#endif
        default:
            return fs8xStatusError;
    }
}

/* Performs a write to a single FS8x FS init register (during the INIT_FS phase only). */
fs8x_status_t FS8x_WriteRegisterInit(SPI_INTERFACE_CONFIG_s* pSpiInterface, fs8x_drv_data_t* drvData, uint8_t address,
        uint16_t writeData)
{
    fs8x_status_t status = fs8xStatusOk;
    uint16_t writeDataInv;  /* Inverted register data. */
    uint8_t addressNot;     /* Address of _NOT_ register. */

    FS_ASSERT(drvData != NULL);

    status = FS8x_WriteRegister(pSpiInterface, drvData, true, address, writeData);

    /* _NOT_ registers has address +1 from normal register. */
    addressNot = (uint8_t)(address + 1);
    /* Inverted value should be written to _NOT_ register. */
    writeDataInv = (uint16_t)~writeData;

    status |= FS8x_WriteRegister(pSpiInterface, drvData, true, addressNot, writeDataInv);

    return status;
}

/* Performs update of a single register. It affects bits specified by a bit mask. */
fs8x_status_t FS8x_UpdateRegister(SPI_INTERFACE_CONFIG_s* pSpiInterface, fs8x_drv_data_t* drvData, bool isFailSafe,
        uint8_t address, int16_t mask, uint16_t writeData)
{
    fs8x_rx_frame_t rxTemp;
    fs8x_status_t status;

    FS_ASSERT(drvData != NULL);

    status = FS8x_ReadRegister(pSpiInterface, drvData, isFailSafe, address, &rxTemp);
    if (status != fs8xStatusOk)
    {
        return status;
    }

    /* Update register value. */
    rxTemp.readData = FS8x_BO_SETVAL(rxTemp.readData, writeData, mask);

    return FS8x_WriteRegister(pSpiInterface, drvData, isFailSafe, address, rxTemp.readData);
}
