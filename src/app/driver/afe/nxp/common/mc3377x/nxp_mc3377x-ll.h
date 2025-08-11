/* Copyright 2019 NXP
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted
 * provided that the following terms are met:
 * 1. Redistributions of source code must retain the above copyright notice, this list of conditions
 * and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions,
 * and the following disclaimer in the documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ?AS IS? AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA; OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file    nxp_mc3377x-ll.h
 * @author  NXP
 * @date    2022-07-29 (date of creation)
 * @updated 2025-06-25 (date of last update)
 * @version v1.10.0
 * @ingroup DRIVERS
 * @prefix  N77X
 *
 * @brief   Header for the low level driver for the MC3377X
 * @details TODO
 */

#ifndef FOXBMS__NXP_MC3377X_LL_H_
#define FOXBMS__NXP_MC3377X_LL_H_

/*========== Includes =======================================================*/
#include "nxp_mc3377x_cfg.h"

#include "spi.h"
#include "uc_msg_t.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/** defines for addresses */
#define N77X_ALL_CHAINS  (0x1C0u)
#define N77X_ALL_DEVICES (0x03Fu)

#define N77X_TX_MIRROR_OFFSET (4u)
#define N77X_RX_FRAME_LENGTH  (4u)

/** Error register address */
#define N77X_ERROR_REGISTER_ADDRESS 0x3FFF

typedef enum N77X_COMMUNICATION_STATUS {
    N77X_COMMUNICATION_OK,
    N77X_COMMUNICATION_ERROR_SHORT_MESSAGE,
    N77X_COMMUNICATION_ERROR_WRONG_CRC,
    N77X_COMMUNICATION_ERROR_WRONG_MESSAGE_COUNT,
    N77X_COMMUNICATION_ERROR_TIMEOUT,
    N77X_COMMUNICATION_ERROR_NO_ACCESS,
    N77X_COMMUNICATION_ERROR_NO_RESPONSE,
    N77X_COMMUNICATION_ERROR_NOT_MATCHING_DEVICE_ADDRESS,
    N77X_COMMUNICATION_ERROR_NOT_MATCHING_REGISTER_ADDRESS,
} N77X_COMMUNICATION_STATUS_e;

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/
/**
 * @brief   Write a value into a specific register in a specific device
 * @param   deviceAddress
 * @param   registerAddress
 * @param   value
 * @param   pSpiInterface
 */
void N77x_CommunicationWrite(
    uint16_t deviceAddress,
    uint16_t registerAddress,
    uint16_t value,
    SPI_INTERFACE_CONFIG_s *pSpiInterface);

/**
 * @brief   Read a value from a specific register in a specific device
 * @param   deviceAddress
 * @param   registerAddress
 * @param   pValue
 * @param   pState
 * @return  returns 0 on success and >=1 in error case
 */
N77X_COMMUNICATION_STATUS_e N77x_CommunicationRead(
    uint16_t deviceAddress,
    uint16_t registerAddress,
    uint16_t *pValue,
    N77X_STATE_s *pState);

/**
 * @brief   Read multiple values from specific registers in a specific device
 * @param   deviceAddress
 * @param   numberOfItems
 * @param   responseLength
 * @param   registerAddress
 * @param   pValues
 * @param   pState
 * @return  returns 0 on success and >=1 in error case
 */
N77X_COMMUNICATION_STATUS_e N77x_CommunicationReadMultiple(
    uint16_t deviceAddress,
    uint16_t numberOfItems,
    uint16_t responseLength,
    uint16_t registerAddress,
    uint16_t *pValues,
    N77X_STATE_s *pState);

/**
 * @brief   Reset the message counter for one or all devices
 * @param   deviceAddress
 * @param   string
 */
void N77x_ResetMessageCounter(uint16_t deviceAddress, uint8_t string);

/**
 * @brief   Composes a message
 * @param   cmd
 * @param   masterAddress
 * @param   deviceAddress
 * @param   registerAddress
 * @param   length
 * @param   pValue
 * @param   pMessage
 */
void N77x_CommunicationComposeMessage(
    uint16_t cmd,
    uint16_t masterAddress,
    uint16_t deviceAddress,
    uint16_t registerAddress,
    uint16_t length,
    uint16_t *pValue,
    uc_msg_t *pMessage);

/**
 * @brief   Decomposes and analysis a message
 * @param   pMessage
 * @param   pCommand
 * @param   pMasterAddress
 * @param   pDeviceAddress
 * @param   pRegisterAddress
 * @param   pLength
 * @param   pValue
 * @param   string
 */
N77X_COMMUNICATION_STATUS_e N77x_CommunicationDecomposeMessage(
    uc_msg_t *pMessage,
    uint16_t *pCommand,
    uint16_t *pMasterAddress,
    uint16_t *pDeviceAddress,
    uint16_t *pRegisterAddress,
    uint16_t *pLength,
    uint16_t *pValue,
    uint8_t string);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
#endif

#endif /* FOXBMS__NXP_MC3377X_LL_H_ */
