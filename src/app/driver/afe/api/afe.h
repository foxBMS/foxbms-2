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
 * @file    afe.h
 * @author  foxBMS Team
 * @date    2020-05-08 (date of creation)
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
 * @ingroup DRIVER
 * @prefix  AFE
 *
 * @brief   AFE driver header
 *
 * @details TODO
 */

#ifndef FOXBMS__AFE_H_
#define FOXBMS__AFE_H_

/*========== Includes =======================================================*/

#include "fstd_types.h"

#include <stdbool.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/** Requests for the AFE driver */
typedef enum {
    AFE_START_REQUEST, /*!<    */
    AFE_STOP_REQUEST,  /*!<    */
    AFE_NO_REQUEST,    /*!<    */
    AFE_REQUEST_E_MAX,
} AFE_REQUEST_e;

/** Type of I2C transfer over AFE slave */
typedef enum {
    AFE_I2C_TRANSFER_TYPE_READ,
    AFE_I2C_TRANSFER_TYPE_WRITEREAD,
    AFE_I2C_TRANSFER_TYPE_READ_SUCCESS,
    AFE_I2C_TRANSFER_TYPE_READ_FAIL,
    AFE_I2C_TRANSFER_TYPE_WRITE,
    AFE_I2C_TRANSFER_TYPE_WRITE_SUCCESS,
    AFE_I2C_TRANSFER_TYPE_WRITE_FAIL,
} AFE_I2C_TRANSFER_TYPE_e;

/** Data to transmit on I2C bus over AFE slave */
typedef struct {
    uint8_t module;
    AFE_I2C_TRANSFER_TYPE_e transferType;
    uint8_t deviceAddress;
    uint8_t registerAddress;
    uint8_t readData[13u];
    uint8_t readDataLength;
    uint8_t writeData[13u];
    uint8_t writeDataLength;
} AFE_I2C_QUEUE_s;

/** period for open wire measurement */
#define AFE_ERROR_OPEN_WIRE_PERIOD_ms (30000u)

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/
/** tick function, this function is called to advance the state machine */
extern STD_RETURN_TYPE_e AFE_TriggerIc(void);
/** initializer, this function is called in order to initialize the AFE */
extern STD_RETURN_TYPE_e AFE_Initialize(void);
/** this function should tell the AFE that it should start measuring */
extern STD_RETURN_TYPE_e AFE_StartMeasurement(void);
/** this function returns whether the first measurement cycle has finished */
extern bool AFE_IsFirstMeasurementCycleFinished(void);

/**
 * @brief   Checks if the first AFE measurement cycle was made
 * @return  true is the first measurement cycle was made, false
 *          otherwise
 */
extern bool AFE_IsFirstMeasurementCycleFinished(void);

/**
 * @brief   Makes the initialization request to the AFE state machine
 * @return  true or false
 */
extern STD_RETURN_TYPE_e AFE_StartMeasurement(void);

/**
 * @brief           Makes the request to the AFE state machine to write to the
 *                  IO port-expander
 * @param   string  string addressed by the request
 */
extern STD_RETURN_TYPE_e AFE_RequestIoWrite(uint8_t string);

/**
 * @brief           Makes the request to the AFE state machine to read from the
 *                  IO port-expander
 * @param   string  string addressed by the request
 */
extern STD_RETURN_TYPE_e AFE_RequestIoRead(uint8_t string);

/**
 * @brief           Makes the request to the AFE state machine to read from the
 *                  external temperature sensor on slaves
 * @param   string  string addressed by the request
 */
extern STD_RETURN_TYPE_e AFE_RequestTemperatureRead(uint8_t string);

/**
 * @brief           Makes the request to the AFE state machine to read
 *                  balancing feedback from the slaves
 * @param   string  string addressed by the request
 */
extern STD_RETURN_TYPE_e AFE_RequestBalancingFeedbackRead(uint8_t string);

/**
 * @brief           Makes the request to the AFE state machine to read from the
 *                  external EEPROM on slaves
 * @param   string  string addressed by the request
 */
extern STD_RETURN_TYPE_e AFE_RequestEepromRead(uint8_t string);

/**
 * @brief           Makes the request to the AFE state machine to write to the
 *                  external EEPROM on slaves
 * @param   string  string addressed by the request
 */
extern STD_RETURN_TYPE_e AFE_RequestEepromWrite(uint8_t string);

/**
 * @brief           Makes the request to the AFE state machine to perform
 *                  open-wire check
 * @param   string  string addressed by the request
 */
extern STD_RETURN_TYPE_e AFE_RequestOpenWireCheck(uint8_t string);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
#endif

#endif /* FOXBMS__AFE_H_ */
