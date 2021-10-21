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
 * @file    n775.h
 * @author  foxBMS Team
 * @date    2020-05-08 (date of creation)
 * @updated 2020-05-08 (date of last update)
 * @ingroup DRIVERS
 * @prefix  N775
 *
 * @brief   Headers for the driver for the MC33775A monitoring chip.
 *
 */

#ifndef FOXBMS__N775_H_
#define FOXBMS__N775_H_

/*========== Includes =======================================================*/
#include "n775_cfg.h"

/*========== Macros and Definitions =========================================*/

/*========== Extern Constant and Variable Declarations ======================*/
/**
 * Buffer used for SPI Rx transmissions for the communication with MC33775A.
 */
extern uint16_t n775_RXbuffer[N775_MAX_N_BYTES_FOR_DATA_RECEPTION];
/**
 * Buffer used for SPI Tx transmissions for the communication with MC33775A.
 */
extern uint16_t n775_TXbuffer[N775_TX_MESSAGE_LENGTH];

/**
 * Struct used for SPI Tx transmissions for the communicaiton with MC33775A.
 */
extern N775_MESSAGE_s n775_sentData;
/**
 * Struct used for SPI Rx transmissions for the communicaiton with MC33775A.
 */
extern N775_MESSAGE_s n775_receivedData;

/**
 * Variable containing the state machine state for the MC33775A driver.
 */
extern N775_STATE_s n775_state;

/*========== Extern Function Prototypes =====================================*/

/**
 * @brief   trigger function for the N775 driver state machine.
 *
 * This function contains the sequence of events in the N775 state machine.
 * It must be called time-triggered, every 1ms.
 */
extern void N775_Trigger(void);

/**
 * @brief   sets the current state request of the state variable n775_state.
 *
 * This function is used to make a state request to the state machine,e.g, start voltage measurement,
 * read result of voltage measurement, re-initialization
 * It calls N775_CheckStateRequest() to check if the request is valid.
 * The state request is rejected if is not valid.
 * The result of the check is returned immediately, so that the requester can act in case
 * it made a non-valid state request.
 *
 * @param   statereq    state request to set
 * @return  retVal      current state request, taken from N775_STATE_REQUEST_e
 */
extern N775_RETURN_TYPE_e N775_SetStateRequest(N775_STATE_REQUEST_e statereq);

extern bool N775_IsFirstMeasurementCycleFinished(void);

extern void N775_SaveVoltages(void);
extern void N775_SaveTemperatures(void);
extern N775_STATE_REQUEST_e N775_GetStateRequest(void);
extern N775_STATEMACH_e N775_GetState(void);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
/* Start test functions */
extern uint8_t TEST_N775_CheckReEntrance();
extern void TEST_N775_SetFirstMeasurementCycleFinished(N775_STATE_s *n775_state);
/* End test functions */
#endif

#endif /* FOXBMS__N775_H_ */
