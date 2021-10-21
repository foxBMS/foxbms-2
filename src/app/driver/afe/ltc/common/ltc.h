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
 * @file    ltc.h
 * @author  foxBMS Team
 * @date    2015-09-01 (date of creation)
 * @updated 2015-09-01 (date of last update)
 * @ingroup DRIVERS
 * @prefix  LTC
 *
 * @brief   Headers for the driver for the LTC monitoring chip.
 *
 */

#ifndef FOXBMS__LTC_H_
#define FOXBMS__LTC_H_

/*========== Includes =======================================================*/
#include "ltc_defs.h"

/*========== Macros and Definitions =========================================*/

/*========== Extern Constant and Variable Declarations ======================*/

/**
 * This variable contains the internal state of the LTC state machine.
 */
extern LTC_STATE_s ltc_stateBase;

/*========== Extern Function Prototypes =====================================*/

/**
 * @brief   trigger function for the LTC driver state machine.
 *
 * This function contains the sequence of events in the LTC state machine.
 * It must be called time-triggered, every 1ms.
 */
extern void LTC_Trigger(LTC_STATE_s *ltc_state);

/**
 * @brief   sets the current state request of the state variable ltc_state.
 *
 * This function is used to make a state request to the state machine,e.g, start voltage measurement,
 * read result of voltage measurement, re-initialization
 * It calls LTC_CheckStateRequest() to check if the request is valid.
 * The state request is rejected if is not valid.
 * The result of the check is returned immediately, so that the requester can act in case
 * it made a non-valid state request.
 *
 * @param  ltc_state  state of the ltc state machine
 * @param  statereq    state request to set
 *
 * @return current state request, taken from LTC_STATE_REQUEST_e
 */
extern LTC_RETURN_TYPE_e LTC_SetStateRequest(LTC_STATE_s *ltc_state, LTC_REQUEST_s statereq);

/**
 * @brief  gets the measurement initialization status.
 *
 * @param  ltc_state  state of the ltc state machine
 *
 * @return true if a first measurement cycle was made, false otherwise
 *
 */
extern bool LTC_IsFirstMeasurementCycleFinished(LTC_STATE_s *ltc_state);

/**
 * @brief   stores the measured voltages in the database.
 *
 * This function loops through the data of all modules in the LTC daisy-chain that are
 * stored in the LTC_CellVoltages buffer and writes them in the database.
 * At each write iteration, the variable named "state" and related to voltages in the
 * database is incremented.
 *
 * @param  ltc_state  state of the ltc state machine
 *
 * @param  stringNumber    string addressed
 *
 */
extern void LTC_SaveVoltages(LTC_STATE_s *ltc_state, uint8_t stringNumber);

/**
 * @brief   stores the measured temperatures and the measured multiplexer feedbacks in the database.
 *
 * This function loops through the temperature and multiplexer feedback data of all modules
 * in the LTC daisy-chain that are stored in the LTC_MultiplexerVoltages buffer and writes
 * them in the database.
 * At each write iteration, the variables named "state" and related to temperatures and multiplexer feedbacks
 * in the database are incremented.
 *
 * @param   ltc_state                    state of the ltc state machine
 * @param  stringNumber    string addressed
 *
 */
extern void LTC_SaveTemperatures(LTC_STATE_s *ltc_state, uint8_t stringNumber);

/**
 * @brief   stores the measured GPIOs in the database.
 *
 * This function loops through the data of all modules in the LTC daisy-chain that are
 * stored in the ltc_allgpiovoltage buffer and writes them in the database.
 * At each write iteration, the variable named "state" and related to voltages in the
 * database is incremented.
 *
 * @param  ltc_state:  state of the ltc state machine
 *
 */
extern void LTC_SaveAllGPIOMeasurement(LTC_STATE_s *ltc_state);

/**
 * @brief   gets the current state request.
 *
 * This function is used in the functioning of the LTC state machine.
 *
 * @param  ltc_state:  state of the ltc state machine
 *
 * @return  retval  current state request, taken from LTC_STATE_REQUEST_e
 */
extern LTC_REQUEST_s LTC_GetStateRequest(LTC_STATE_s *ltc_state);

/**
 * @brief   gets the current state.
 *
 * This function is used in the functioning of the LTC state machine.
 *
 * @param  ltc_state:  state of the ltc state machine
 *
 * @return  current state, taken from LTC_STATEMACH_e
 */
extern LTC_STATEMACH_e LTC_GetState(LTC_STATE_s *ltc_state);

/**
 * @brief   Sets the transceiver pins to enable LTC6820 IC.
 *
 */
extern void LTC_monitoringPinInit(void);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
extern uint8_t TEST_LTC_CheckReEntrance();
extern void TEST_LTC_SetFirstMeasurementCycleFinished(LTC_STATE_s *ltc_state);

/** this define is used for creating the declaration of a function for variable extraction
 *  deviate from style guide in order to make the variable name better recognizable
 */
#define TEST_LTC_DECLARE_GET(VARIABLE) extern void TEST_LTC_Get_##VARIABLE(uint8_t data[4])
TEST_LTC_DECLARE_GET(ltc_cmdWRCFG);
TEST_LTC_DECLARE_GET(ltc_cmdWRCFG2);
TEST_LTC_DECLARE_GET(ltc_cmdRDCFG);
TEST_LTC_DECLARE_GET(ltc_cmdRDCVA);
TEST_LTC_DECLARE_GET(ltc_cmdRDCVB);
TEST_LTC_DECLARE_GET(ltc_cmdRDCVC);
TEST_LTC_DECLARE_GET(ltc_cmdRDCVD);
TEST_LTC_DECLARE_GET(ltc_cmdRDCVE);
TEST_LTC_DECLARE_GET(ltc_cmdRDCVF);
TEST_LTC_DECLARE_GET(ltc_cmdWRCOMM);
TEST_LTC_DECLARE_GET(ltc_cmdSTCOMM);
TEST_LTC_DECLARE_GET(ltc_cmdRDCOMM);
TEST_LTC_DECLARE_GET(ltc_cmdRDAUXA);
TEST_LTC_DECLARE_GET(ltc_cmdRDAUXB);
TEST_LTC_DECLARE_GET(ltc_cmdRDAUXC);
TEST_LTC_DECLARE_GET(ltc_cmdRDAUXD);
TEST_LTC_DECLARE_GET(ltc_cmdADCV_normal_DCP0);
TEST_LTC_DECLARE_GET(ltc_cmdADCV_normal_DCP1);
TEST_LTC_DECLARE_GET(ltc_cmdADCV_filtered_DCP0);
TEST_LTC_DECLARE_GET(ltc_cmdADCV_filtered_DCP1);
TEST_LTC_DECLARE_GET(ltc_cmdADCV_fast_DCP0);
TEST_LTC_DECLARE_GET(ltc_cmdADCV_fast_DCP1);
TEST_LTC_DECLARE_GET(ltc_cmdADCV_fast_DCP0_twocells);
TEST_LTC_DECLARE_GET(ltc_cmdADAX_normal_GPIO1);
TEST_LTC_DECLARE_GET(ltc_cmdADAX_filtered_GPIO1);
TEST_LTC_DECLARE_GET(ltc_cmdADAX_fast_GPIO1);
TEST_LTC_DECLARE_GET(ltc_cmdADAX_normal_GPIO2);
TEST_LTC_DECLARE_GET(ltc_cmdADAX_filtered_GPIO2);
TEST_LTC_DECLARE_GET(ltc_cmdADAX_fast_GPIO2);
TEST_LTC_DECLARE_GET(ltc_cmdADAX_normal_GPIO3);
TEST_LTC_DECLARE_GET(ltc_cmdADAX_filtered_GPIO3);
TEST_LTC_DECLARE_GET(ltc_cmdADAX_fast_GPIO3);
TEST_LTC_DECLARE_GET(ltc_cmdADAX_normal_ALLGPIOS);
TEST_LTC_DECLARE_GET(ltc_cmdADAX_filtered_ALLGPIOS);
TEST_LTC_DECLARE_GET(ltc_cmdADAX_fast_ALLGPIOS);
TEST_LTC_DECLARE_GET(ltc_BC_cmdADOW_PUP_normal_DCP0);
TEST_LTC_DECLARE_GET(ltc_BC_cmdADOW_PDOWN_normal_DCP0);
TEST_LTC_DECLARE_GET(ltc_BC_cmdADOW_PUP_filtered_DCP0);
TEST_LTC_DECLARE_GET(ltc_BC_cmdADOW_PDOWN_filtered_DCP0);
TEST_LTC_DECLARE_GET(ltc_cmdRDCVA_Fuelcell);
TEST_LTC_DECLARE_GET(ltc_cmdRDCVB_Fuelcell);
TEST_LTC_DECLARE_GET(ltc_cmdRDCVC_Fuelcell);
TEST_LTC_DECLARE_GET(ltc_cmdRDCVD_Fuelcell);
TEST_LTC_DECLARE_GET(ltc_cmdRDCVE_Fuelcell);
TEST_LTC_DECLARE_GET(ltc_cmdRDCVF_Fuelcell);
TEST_LTC_DECLARE_GET(ltc_cmdRDCVG_Fuelcell);
TEST_LTC_DECLARE_GET(ltc_cmdRDCVH_Fuelcell);
TEST_LTC_DECLARE_GET(ltc_cmdRDCVI_Fuelcell);
TEST_LTC_DECLARE_GET(ltc_cmdADCV_normal_Fuelcell);
TEST_LTC_DECLARE_GET(ltc_BC_cmdADOW_PUP_100ms_fuelcell);
TEST_LTC_DECLARE_GET(ltc_BC_cmdADOW_PDOWN_100ms_fuelcell);
#endif

#endif /* FOXBMS__LTC_H_ */
