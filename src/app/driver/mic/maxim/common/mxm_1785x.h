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
 * @file    mxm_1785x.h
 * @author  foxBMS Team
 * @date    2019-01-15 (date of creation)
 * @updated 2020-09-10 (date of last update)
 * @ingroup DRIVERS
 * @prefix  MXM
 *
 * @brief   Headers for the driver for the MAX17841B ASCI and MAX1785x monitoring chip
 *
 * @details def
 *
 */

/**
 * @brief Battery monitoring driver for MAX1785x battery monitoring ICs.
 *
 * This module supplies a driver for the Battery Monitoring ICs of the
 * MAX1785x-family by Maxim Integrated.
 *
 * Entry point for the module is the function #MXM_Tick() in mxm_mic.c. It handles
 * the measurement flow and the coordination of the underlying state-machines.
 * Below this layer two state-machines are implemented.
 *
 * The state-machine in mxm_battery_management.c is executed with the
 * #MXM_5XStateMachine()-function.
 * This state-machine exposes commands of the Maxim Battery Management Protocol
 * to the upper layers. Below, it translates these commands into state-changes
 * for the underlying state-machine.
 * This state-machine is implemented in mxm_17841b.c and executed with
 * #MXM_41BStateMachine(). It handles the register- and buffer-transactions
 * required for the MAX17841B communication interface (Maxim calls
 * this chip ASCI).
 *
 */

#ifndef FOXBMS__MXM_1785X_H_
#define FOXBMS__MXM_1785X_H_

/*========== Includes =======================================================*/
#include "database_cfg.h"
#include "mxm_cfg.h"

#include "mxm_17841b.h"
#include "mxm_1785x_tools.h"
#include "mxm_basic_defines.h"
#include "mxm_battery_management.h"
#include "mxm_crc8.h"

/*========== Macros and Definitions =========================================*/

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/
/**
 * @brief           Main state-machine implementation
 * @details         This state-machine handles the main state of the driver.
 *                  It runs the self-check, initializes communication and then
 *                  transitions into the operation state in which it
 *                  executes the state-machine described in
 *                  #MXM_StateMachineOperation().
 * @param[in,out]   pInstance   used as both input and output (stores
 *                              state-information, requests and intermediate
 *                              values)
 */
extern void MXM_StateMachine(MXM_MONITORING_INSTANCE_s *pInstance);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
extern STD_RETURN_TYPE_e TEST_MXM_ParseVoltageReadallTest(MXM_MONITORING_INSTANCE_s *pInstance);
extern STD_RETURN_TYPE_e TEST_MXM_ParseVoltagesIntoDB(MXM_MONITORING_INSTANCE_s *pInstance);
extern MXM_MONINTORING_STATE_e TEST_MXM_MonGetVoltages(MXM_MONITORING_INSTANCE_s *pInstance, MXM_REG_NAME_e regAddress);
extern void TEST_MXM_HandleStateWriteall(
    MXM_MONITORING_INSTANCE_s *pInstance,
    MXM_STATEMACHINE_OPERATION_STATES_e nextState);
extern bool TEST_MXM_HandleStateReadall(
    MXM_MONITORING_INSTANCE_s *pInstance,
    MXM_REG_NAME_e registerName,
    MXM_STATEMACHINE_OPERATION_STATES_e nextState);
extern STD_RETURN_TYPE_e TEST_MXM_ParseVoltageReadall(
    uint8_t *volt_rx_buffer,
    uint16_t volt_rx_buffer_len,
    MXM_DATA_STORAGE_s *datastorage,
    MXM_CONVERSION_TYPE_e conversionType);
extern void TEST_MXM_ProcessOpenWire(MXM_MONITORING_INSTANCE_s *pInstance, DATA_BLOCK_OPEN_WIRE_s *pDataOpenWire);
#endif

#endif /* FOXBMS__MXM_1785X_H_ */
