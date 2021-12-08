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
 * @file    spi.h
 * @author  foxBMS Team
 * @date    2019-12-12 (date of creation)
 * @updated 2021-12-08 (date of last update)
 * @ingroup DRIVERS
 * @prefix  SPI
 *
 * @brief   Headers for the driver for the SPI module.
 *
 */

#ifndef FOXBMS__SPI_H_
#define FOXBMS__SPI_H_

/*========== Includes =======================================================*/
#include "spi_cfg.h"

/*========== Macros and Definitions =========================================*/

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/

/* INCLUDE MARKER FOR THE DOCUMENTATION; DO NOT MOVE spi-documentation-dummy-start-include */
/**
 * @brief   Sends a dummy byte to wake up the SPI interface.
 *
 * @param   pSpiInterface pointer to SPI interface configuration
 * @param   delay delay to wait after dummy byte transfer
 *
 * @return  status of the SPI transfer
 */
extern STD_RETURN_TYPE_e SPI_TransmitDummyByte(SPI_INTERFACE_CONFIG_s *pSpiInterface, uint32_t delay);
/* INCLUDE MARKER FOR THE DOCUMENTATION; DO NOT MOVE spi-documentation-dummy-stop-include */

/* INCLUDE MARKER FOR THE DOCUMENTATION; DO NOT MOVE spi-documentation-transmit-start-include */
/**
 * @brief   Transmits data on SPI without DMA.
 * @details This function can be used to send and receive data via SPI. SPI
 *          communication is performed in blocking mode and chip select is
 *          set/reset automatically.
 * @param   pSpiInterface pointer to SPI interface configuration
 * @param   pTxBuff pointer to data that is transmitted by the SPI interface
 * @param   frameLength number of bytes to be transmitted by the SPI interface
 * @return  status of the SPI transfer
 */
extern STD_RETURN_TYPE_e SPI_TransmitData(SPI_INTERFACE_CONFIG_s *pSpiInterface, uint16 *pTxBuff, uint32 frameLength);
/* INCLUDE MARKER FOR THE DOCUMENTATION; DO NOT MOVE spi-documentation-transmit-stop-include */

/* INCLUDE MARKER FOR THE DOCUMENTATION; DO NOT MOVE spi-documentation-transmit-receive-start-include */
/**
 * @brief   Transmits and receives data on SPI without DMA.
 * @details This function can be used to send and receive data via SPI. SPI
 *          communication is performed in blocking mode and chip select is
 *          set/reset automatically.
 * @param   pSpiInterface pointer to SPI interface configuration
 * @param   pTxBuff pointer to data that is transmitted by the SPI interface
 * @param   pRxBuff pointer to data that is received by the SPI interface
 * @param   frameLength number of bytes to be transmitted by the SPI interface
 * @return  status of the SPI transfer
 */
extern STD_RETURN_TYPE_e SPI_TransmitReceiveData(
    SPI_INTERFACE_CONFIG_s *pSpiInterface,
    uint16 *pTxBuff,
    uint16 *pRxBuff,
    uint32 frameLength);
/* INCLUDE MARKER FOR THE DOCUMENTATION; DO NOT MOVE spi-documentation-transmit-receive-stop-include */

/* INCLUDE MARKER FOR THE DOCUMENTATION; DO NOT MOVE spi-documentation-fram-start-include */
/**
 * @brief   Transmits and receives data on SPI without DMA, wrappe for FRAM
 * @details This function can be used to send and receive data via SPI. SPI
 *          communication is performed in blocking mode and chip select is
 *          set/reset automatically.
 *          It does not drive the Chip Select (neither hardware nor software)
 *          as this is done directly in the FRAM functions.
 * @param   pSpiInterface pointer to SPI interface configuration
 * @param   pTxBuff pointer to data that is transmitted by the SPI interface
 * @param   pRxBuff pointer to data that is received by the SPI interface
 * @param   frameLength number of bytes to be transmitted by the SPI interface
 * @return  status of the SPI transfer
 */
extern void SPI_FramTransmitReceiveData(
    SPI_INTERFACE_CONFIG_s *pSpiInterface,
    uint16 *pTxBuff,
    uint16 *pRxBuff,
    uint32 frameLength);
/* INCLUDE MARKER FOR THE DOCUMENTATION; DO NOT MOVE spi-documentation-fram-stop-include */

/* INCLUDE MARKER FOR THE DOCUMENTATION; DO NOT MOVE spi-documentation-transmit-receive-dma-start-include */
/**
 * @brief   Transmits and receives data on SPI with DMA.
 * @details This function can be used to send and receive data via SPI. SPI
 *          communication is performed in blocking mode and chip select is
 *          set/reset automatically..
 * @param   pSpiInterface pointer to SPI interface configuration
 * @param   pTxBuff pointer to data that is transmitted by the SPI interface
 * @param   pRxBuff pointer to data that is received by the SPI interface
 * @param   frameLength number of bytes to be transmitted by the SPI interface
 * @return  status of the SPI transfer
 */
extern STD_RETURN_TYPE_e SPI_TransmitReceiveDataDma(
    SPI_INTERFACE_CONFIG_s *pSpiInterface,
    uint16_t *pTxBuff,
    uint16_t *pRxBuff,
    uint32_t frameLength);
/* INCLUDE MARKER FOR THE DOCUMENTATION; DO NOT MOVE spi-documentation-transmit-receive-dma-stop-include */

/* INCLUDE MARKER FOR THE DOCUMENTATION; DO NOT MOVE spi-documentation-slave-receive-dma-start-include */
/**
 * @brief   Transmits and receives data on SPI with DMA.
 * @details This function can be used to send and receive data via SPI. SPI
 *          communication is performed in blocking mode and chip select is
 *          set/reset automatically..
 * @param   pSpiInterface pointer to SPI interface configuration
 * @param   pTxBuff pointer to data that is transmitted by the SPI interface
 * @param   pRxBuff pointer to data that is received by the SPI interface
 * @param   frameLength number of bytes to be transmitted by the SPI interface
 * @return  status of the SPI transfer
 */
extern STD_RETURN_TYPE_e SPI_SlaveSetReceiveDataDma(
    SPI_INTERFACE_CONFIG_s *pSpiInterface,
    uint16_t *pTxBuff,
    uint16_t *pRxBuff,
    uint32_t frameLength);
/* INCLUDE MARKER FOR THE DOCUMENTATION; DO NOT MOVE spi-documentation-slave-receive-dma-stop-include */

/**
 * @brief   Locks SPI interfaces.
 * @details This function is used to change the state of the SPI_busy_flags
 *          variable to "locked".
 * @param   spi  SPI interface to be locked (0-4 on the TMS570LC4357)
 * @return  #STD_OK if SPI interface could be locked, #STD_NOT_OK otherwise
 */
extern STD_RETURN_TYPE_e SPI_Lock(uint8_t spi);

/**
 * @brief   Unlocks SPI interfaces.
 * @details This function is used to change the state of the SPI_busy_flags
 *          variable to "unlocked".
 * @param   spi  SPI interface to be unlocked (0-4 on the TMS570LC4357)
 */
extern void SPI_Unlock(uint8_t spi);

/**
 * @brief   Sets the functional of a SPI pin
 * @details SPI pins in this HAL can have functional state SPI (for when it is
 *          controlled by the SPI hardware) and functional state GIO (for when
 *          it is controlled as a GIO pin).
 * @param[in,out]   pNode   handle of the SPI node that should be configured
 * @param[in]       bit     bit that should be manipulated, other pins remain
 *                          unchanged
 * @param[in]       hardwareControlled  Whether the bit should be hardware
 *                                      controlled (true) or not (false)
 */
extern void SPI_SetFunctional(spiBASE_t *pNode, enum spiPinSelect bit, bool hardwareControlled);

/**
 * @brief   Used to send last byte per SPI.
 * @details This function is called in the DMA Tx callback. It is used
 *          to send the last byte with CSHOLD = 0.
 * @param   spiIndex  SPI node in use
 */
extern void SPI_DmaSendLastByte(uint8_t spiIndex);

/**
 * @brief   Returns #STD_OK if the SPI interface can be used again
 * @details This function just checks for SPI_PENDING and groups SPI_READY and
 *          SPI_...
 * @param[in]   pNode   handle of the SPI node that should be checked
 * @returns     #STD_RETURN_TYPE_e indicating wether the interface is ok to be used.
 */
extern STD_RETURN_TYPE_e SPI_CheckInterfaceAvailable(spiBASE_t *pNode);

/**
 * @brief   Returns index of SPI node
 * @param[in]   pNode   handle of the SPI node that should be checked
 * @returns     index of SPI node
 */
extern uint8_t SPI_GetSpiIndex(spiBASE_t *pNode);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/

#endif /* FOXBMS__SPI_H_ */
