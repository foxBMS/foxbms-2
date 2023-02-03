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

#ifndef FOXBMS__NXP_MC33775A_LL_H_
#define FOXBMS__NXP_MC33775A_LL_H_

/*========== Includes =======================================================*/
#include "nxp_mc33775a_cfg.h"

#include "spi.h"
#include "uc_msg_t.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/** defines for addresses */
#define N775_ALL_CHAINS  (0x1C0u)
#define N775_ALL_DEVICES (0x03Fu)

/** Error register address */
#define N775_ERROR_REGISTER_ADDRESS 0x3FFF

typedef enum N775_COMMUNICATION_STATUS {
    N775_COMMUNICATION_OK,
    N775_COMMUNICATION_ERROR_SHORT_MESSAGE,
    N775_COMMUNICATION_ERROR_WRONG_CRC,
    N775_COMMUNICATION_ERROR_WRONG_MESSAGE_COUNT,
    N775_COMMUNICATION_ERROR_TIMEOUT,
    N775_COMMUNICATION_ERROR_NO_ACCESS,
    N775_COMMUNICATION_ERROR_NO_RESPONSE,
    N775_COMMUNICATION_ERROR_NOT_MATCHING_DEVICE_ADDRESS,
    N775_COMMUNICATION_ERROR_NOT_MATCHING_REGISTER_ADDRESS,
} N775_COMMUNICATION_STATUS_e;

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/
/**
 * @brief   Write a value into a specific register in a specific device
 * @param   deviceAddress
 * @param   registerAddress
 * @param   value
 * @param   pSpiInterface
 */
void N775_CommunicationWrite(
    uint16_t deviceAddress,
    uint16_t registerAddress,
    uint16_t value,
    SPI_INTERFACE_CONFIG_s *pSpiInterface);

/**
 * @brief   Read a value from a specific register in a specific device
 * @param   deviceAddress
 * @param   registerAddress
 * @param   pValue
 * @param   n775_state
 * @return  returns 0 on success and >=1 in error case
 */
N775_COMMUNICATION_STATUS_e N775_CommunicationRead(
    uint16_t deviceAddress,
    uint16_t registerAddress,
    uint16_t *pValue,
    N775_STATE_s *n775_state);

/**
 * @brief   Read multiple values from specific registers in a specific device
 * @param   deviceAddress
 * @param   numberOfItems
 * @param   responseLength
 * @param   registerAddress
 * @param   pValues
 * @param   n775_state
 * @return  returns 0 on success and >=1 in error case
 */
N775_COMMUNICATION_STATUS_e N775_CommunicationReadMultiple(
    uint16_t deviceAddress,
    uint16_t numberOfItems,
    uint16_t responseLength,
    uint16_t registerAddress,
    uint16_t *pValues,
    N775_STATE_s *n775_state);

/**
 * @brief   Reset the message counter for one or all devices
 * @param   deviceAddress
 * @param   string
 */
void N775_ResetMessageCounter(uint16_t deviceAddress, uint8_t string);

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
void N775_CommunicationComposeMessage(
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
N775_COMMUNICATION_STATUS_e N775_CommunicationDecomposeMessage(
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

#endif /* FOXBMS__NXP_MC33775A_LL_H_ */
