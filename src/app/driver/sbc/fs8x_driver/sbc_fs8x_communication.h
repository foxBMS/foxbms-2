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

/** @file sbc_fs8x_communication.h
 * @brief This file contains functions for SPI/I2C communication.
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

#ifndef SBC_FS8x_COMMUNICATION_H_
#define SBC_FS8x_COMMUNICATION_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include <stdint.h>
#include <stdbool.h>

#include "sbc_fs8x_common.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* Data frame (SPI or I2C). */
#define FS8x_COMM_FRAME_SIZE (0x04U) /*!< Length of the communication frame */

/*******************************************************************************
 * Global Variables
 ******************************************************************************/

/*******************************************************************************
 * Prototypes of extern functions
 ******************************************************************************/
/** @defgroup Extern MCU specific functions
 * @brief Functions in this group must be implemented by the user.
 * @{ */
#if FS8x_COMM_TYPE == FS8x_COMM_SPI || FS8x_COMM_TYPE == FS8x_COMM_BOTH
/** @brief This function transfers single frame through blocking SPI communication
 *         in both directions. MCU specific.
 *
 * This function must be implemented if SPI communication is used. The txFrame must be
 * sent to the SPI bus from the last byte to the first (e.g. txFrame[0] will be sent
 * last). The FS8x driver expects incoming data in reversed order, e.g. rxFrame[0] = CRC,
 * rxFrame[1] = LSB ...
 * @warning This function must be implemented as blocking as there is not synchronization
 * mechanism implemented in the driver.
 * @param [in,out] pSpiInterface    SPI communication interface config
 * @param [in]  txFrame             Frame to be send.
 * @param [in]  frameLengthBytes    Bytes Length of the frame in bytes.
 * @param [out] rxFrame             Received frame.
 * @return @ref fs8x_status_t "Status return code." */
extern fs8x_status_t MCU_SPI_TransferData(SPI_INTERFACE_CONFIG_s* pSpiInterface, uint8_t* txFrame,
                uint16_t frameLengthBytes, uint8_t* rxFrame);
#endif

#if FS8x_COMM_TYPE == FS8x_COMM_I2C || FS8x_COMM_TYPE == FS8x_COMM_BOTH
/** @brief This function sends single frame to I2C bus.
 *         MCU specific.
 *
 * This function must be implemented if I2C communication is used. The txFrame must be
 * sent to the I2C bus from the last byte to the first (e.g. txFrame[0] will be sent
 * last).
 * @warning This function must be implemented as blocking as there is not synchronization
 * mechanism implemented in the driver.
 * @param [in] txFrame           Frame to be send.
 * @param [in] frameLengthBytes  Frame size in bytes.
 * @param [in] i2cAddress        7-bit I2C address of the FS8x device.
 * @return @ref fs8x_status_t "Status return code." */
extern fs8x_status_t MCU_I2C_SendData(uint8_t* txFrame, uint8_t frameLengthBytes, uint8_t i2cAddress);

/** @brief This function receives single frame from I2C bus.
 *         MCU specific.
 *
 * This function must be implemented if I2C communication is used. The FS8x driver expects
 * incoming data in reversed order, e.g. rxFrame[0] = CRC, rxFrame[1] = LSB ...
 * @warning This function must be implemented as blocking as there is not synchronization
 * mechanism implemented in the driver.
 * @param [in]  frameLengthBytes  Frame size in bytes.
 * @param [in]  i2cAddress        7-bit I2C address of the FS8x device.
 * @param [out] rxFrame           Received frame.
 * @return @ref fs8x_status_t "Status return code." */
extern fs8x_status_t MCU_I2C_ReceiveData(uint8_t frameLengthBytes, uint8_t i2cAddress, uint8_t* rxFrame);
#endif
/** @} */
/*******************************************************************************
 * API
 ******************************************************************************/
/** @addtogroup API
 * @{ */
/** @brief Performs a read from a single FS8x register.
 *
 * Performs a single read register based on provided address.
 * The response is returned in @ref fs8x_rx_frame_t structure.
 * @param [in,out] pSpiInterface  SPI communication interface config
 * @param [in]  drvData           Driver run-time data.
 * @param [in]  isFailSafe        true if the register is Fail Safe.
 * @param [in]  address           Register address.
 * @param [out] rxData            Structure holding the response from SBC.
 * @return @ref fs8x_status_t "Status return code." */
fs8x_status_t FS8x_ReadRegister(SPI_INTERFACE_CONFIG_s* pSpiInterface, fs8x_drv_data_t* drvData, bool isFailSafe,
        uint8_t address, fs8x_rx_frame_t* rxData);

/** @brief Sends write command to the FS8x.
 * @param [in,out] pSpiInterface  SPI communication interface config
 * @param [in] drvData            Driver run-time data.
 * @param [in] isFailSafe         true: Reading from FS register, false: Reading from Main register.
 * @param [in] address            Register address.
 * @param [in] writeData          Register write value.
 * @return @ref fs8x_status_t "Status return code." */
fs8x_status_t FS8x_WriteRegister(SPI_INTERFACE_CONFIG_s* pSpiInterface, fs8x_drv_data_t* drvData, bool isFailSafe,
        uint8_t address, uint16_t writeData);

/** @brief Performs a write to a single FS8x FS init register (during the INIT_FS phase only).
 * @param [in,out] pSpiInterface  SPI communication interface config
 * @param [in] drvData            Driver run-time data.
 * @param [in] address            Register address.
 * @param [in] writeData          Register write value.
 * @return @ref fs8x_status_t "Status return code." */
fs8x_status_t FS8x_WriteRegisterInit(SPI_INTERFACE_CONFIG_s* pSpiInterface, fs8x_drv_data_t* drvData, uint8_t address,
        uint16_t writeData);

/** @brief Performs update of a single register. It affects bits specified by a bit mask.
 * @param [in,out] pSpiInterface  SPI communication interface config
 * @param [in] drvData            Driver run-time data.
 * @param [in] isFailSafe         true: Reading from FS register, false: Reading from Main register.
 * @param [in] address            Register address.
 * @param [in] mask               Register write mask.
 * @param [in] writeData          Register write value.
 * @return @ref fs8x_status_t "Status return code." */
fs8x_status_t FS8x_UpdateRegister(SPI_INTERFACE_CONFIG_s* pSpiInterface, fs8x_drv_data_t* drvData, bool isFailSafe,
        uint8_t address, int16_t mask, uint16_t writeData);
/** @} */
#endif /* SBC_FS8x_COMMUNICATION_H_ */
