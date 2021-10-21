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
 * @file    meas.h
 * @author  foxBMS Team
 * @date    2020-02-24 (date of creation)
 * @updated 2021-03-24 (date of last update)
 * @ingroup DRIVERS
 * @prefix  MEAS
 *
 * @brief   Headers for the driver for the measurements needed by the BMS
 *          (e.g., I,V,T).
 *
 * @details TODO
 */

#ifndef FOXBMS__MEAS_H_
#define FOXBMS__MEAS_H_

/*========== Includes =======================================================*/
#include "general.h"

/*========== Macros and Definitions =========================================*/

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/

/**
 * Calls all initialization functions.
 */
extern STD_RETURN_TYPE_e MEAS_Initialize(void);

/**
 * @brief   Control function that is called from the task every 1ms
 * @details This control function or tick function is called every 1ms from the
 *          tasks. It calls the tick, trigger or control functions of the
 *          state machines belonging to the measurement module.
 */
extern void MEAS_Control(void);

/**
 * @brief   Checks if the first AFE measurement cycle was made
 * @return  true if the first measurement cycle was made, false otherwise
 */
extern bool MEAS_IsFirstMeasurementCycleFinished(void);

/**
 * @brief   Makes the initialization request to the AFE state machine
 */
extern STD_RETURN_TYPE_e MEAS_StartMeasurement(void);

/**
 * @brief   Makes the request to the AFE state machine to write to the IO
 *          port-expander
 * @param   string      string addressed by the request
 */
extern STD_RETURN_TYPE_e MEAS_RequestIoWrite(uint8_t string);

/**
 * @brief   Makes the request to the AFE state machine to read from the IO
 *          port-expander
 * @param   string      string addressed by the request
 */
extern STD_RETURN_TYPE_e MEAS_RequestIoRead(uint8_t string);

/**
 * @brief   Makes the request to the AFE state machine to read from the
 *          external temperature sensor on slaves
 * @param   string      string addressed by the request
 */
extern STD_RETURN_TYPE_e MEAS_RequestTemperatureRead(uint8_t string);

/**
 * @brief   Makes the request to the AFE state machine to read balancing
 *          feedback from the slaves
 * @param   string      string addressed by the request
 */
extern STD_RETURN_TYPE_e MEAS_RequestBalancingFeedbackRead(uint8_t string);

/**
 * @brief   Makes the request to the AFE state machine to read from the
 *          external EEPROM on slaves
 * @param   string      string addressed by the request
 */
extern STD_RETURN_TYPE_e MEAS_RequestEepromRead(uint8_t string);

/**
 * @brief   Makes the request to the AFE state machine to write to the external
 *          EEPROM on slaves
 * @param   string      string addressed by the request
 */
extern STD_RETURN_TYPE_e MEAS_RequestEepromWrite(uint8_t string);

/**
 * @brief   Makes the request to the AFE state machine to perform open-wire check
 * @param   string      string addressed by the request
 */
extern STD_RETURN_TYPE_e MEAS_RequestOpenWireCheck(uint8_t string);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/

#endif /* FOXBMS__MEAS_H_ */
