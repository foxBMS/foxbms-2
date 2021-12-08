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
 * @updated 2021-12-06 (date of last update)
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
 * Entry point for the module is the function #MXM_Tick() in mxm_afe.c. It handles
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

/** threshold of iterations after which a diagnostic cycle is entered by the state-machine */
#define MXM_THRESHOLD_DIAGNOSTIC_AFTER_CYCLES (10u)

/** length of voltage-read array */
#define MXM_VOLTAGE_READ_ARRAY_LENGTH (MXM_MAXIMUM_NR_OF_CELLS_PER_MODULE + 3u)

/** number of cells that are in the LSB register of i.e. pin open diagnostic */
#define MXM_CELLS_IN_LSB (8u)

/** unipolar full-scale reference value for cell voltage in mV */
#define MXM_REF_UNIPOLAR_CELL_mV (5000u)

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

/**
 * @brief   Initializes the state structs with default values
 * @details This function is called through the startup of the driver in order
 *          to ensure proper default values.
 * @param[out]  pBalancingInstance  instance of the balancing state struct that shall be initialized
 * @param[out]  pMonitoringInstance instance of the monitoring state struct that shall be initialized
 */
extern void MXM_InitializeStateStruct(
    MXM_BALANCING_STATE_s *pBalancingInstance,
    MXM_MONITORING_INSTANCE_s *pMonitoringInstance);

/**
 * @brief   Function that checks if the error counter can be reset
 * @param[in,out]   pInstance   pointer to the state struct
 */
extern void MXM_CheckIfErrorCounterCanBeReset(MXM_MONITORING_INSTANCE_s *pInstance);

/**
 * @brief           Fill the balancing datastructure
 * @details         This function fills the data-structure that describes
 *                  which balancing channels of the monitoring ICs should be
 *                  activated.
 * @param[in,out]   pBalancingInstance   pointer to the balancing state
 * @return          #STD_NOT_OK in case of invalid access
 */
extern STD_RETURN_TYPE_e MXM_ConstructBalancingBuffer(MXM_BALANCING_STATE_s *pBalancingInstance);

/**
 * @brief           Handle the statemachine-transactions for a WRITEALL
 * @details         Before calling this function, update the command buffer of
 *                  the state-variable. Then call this function and pass on the
 *                  state-variable and the next state. The function will
 *                  handle the communication with the lower state-machine and
 *                  will transition into the next state, if the command has
 *                  been sent successfully.
 * @param[in,out]   pInstance   pointer to instance of the mxm
 *                              monitoring state-machine
 * @param[in]       nextState   state that should be entered upon successful
 *                              completion
 */
extern void MXM_HandleStateWriteall(
    MXM_MONITORING_INSTANCE_s *pInstance,
    MXM_STATEMACHINE_OPERATION_STATES_e nextState);

/**
 * @brief           Handle the statemachine-transactions for a READALL
 * @details         Call this function and pass on the state-variable, the
 *                  register to be read and the next state. The function will
 *                  handle the communication with the lower state-machine and
 *                  will transition into the next state, if the command has
 *                  been sent successfully. Moreover it will return true when
 *                  transitioning. The return value has to be checked and used
 *                  to execute additional code if necessary.
 * @param[in,out]   pInstance   pointer to instance of the mxm
 *                              monitoring state-machine
 * @param[in]       registerName    register that should be read
 * @param[in]       nextState   state that should be entered upon successful
 *                              completion
 * @return          true when the state has been handled, false otherwise, use
 *                  this to execute additional code when the message has been
 *                  read.
 */
extern bool must_check_return MXM_HandleStateReadall(
    MXM_MONITORING_INSTANCE_s *pInstance,
    MXM_REG_NAME_e registerName,
    MXM_STATEMACHINE_OPERATION_STATES_e nextState);

/**
 * @brief           Processes the retrieved information on openwire
 * @details         Parses through a retrieved RX buffer and writes into the
 *                  database.
 * @param[in,out]   kpkInstance     pointer to instance of the Maxim monitoring
 *                                  state-machine
 * @return returns the return value of the database write function #DATA_Write_1_DataBlock()
 */
extern STD_RETURN_TYPE_e MXM_ProcessOpenWire(const MXM_MONITORING_INSTANCE_s *const kpkInstance);

/**
 * @brief   This error handler is used as a last resort and tries a reset of the complete driver
 * @details A reset is done by setting the relevant flag in the state struct
 *          and waiting for the reset to occur (done by AFE on next tick).
 *          Before this last resort measure is taken, a error counter has to
 *          be counted up.
 * @param[in,out]   pInstance       pointer to the state struct in order to write the reset flag
 * @param[in]       immediateReset  if set to true, a reset will be requested independently of the error counter
 */
extern void MXM_ErrorHandlerReset(MXM_MONITORING_INSTANCE_s *pInstance, bool immediateReset);

/**
 * @brief           State-Machine implementation for operation state
 * @details         This state-machine contains the "program" with which the
 *                  connected monitoring satellites are controlled. It is
 *                  entered by #MXM_StateMachine() once the daisy-chain has
 *                  been initialized and is in operation state.
 * @param[in,out]   pState  used as both input and output (stores
 *                          state-information, requests and intermediate values)
 */
extern void MXM_StateMachineOperation(MXM_MONITORING_INSTANCE_s *pState);

/**
 * @brief           Encapsulation for reading voltages from a register
 * @details         This function encapsulates the request of state-changes and
 *                  following conversion for the reading of an arbitrary
 *                  measurement voltage of the daisy-chain. Its parameters are
 *                  a variable for tracking the state of the underlying
 *                  state-machines and the register address that has to be
 *                  queried. It returns whether the action has been successful
 *                  or not.
 *                  In order to obtain all cell voltages this function has to
 *                  be called for every relevant register address.
 * @param[in,out]   pState      pointer to the state-machine struct
 * @param[in]       regAddress  register address that shall be queried
 * @return          current state of the action:
 *                      - MXM_MON_STATE_PASS upon completion
 *                      - MXM_MON_STATE_PENDING as long as the action is
 *                        ongoing
 *                      - MXM_MON_STATE_FAIL if the function failed and could
 *                        not recover on its own
 */
extern MXM_MONITORING_STATE_e must_check_return
    MXM_MonGetVoltages(MXM_MONITORING_INSTANCE_s *pState, MXM_REG_NAME_e regAddress);

/**
 * @brief   Copies measured voltage data into the database.
 * @details This function copies the acquired voltage data from
 *          #MXM_MONITORING_INSTANCE_s::localVoltages into the database-struct
 *          #mxm_tableCellVoltages and copies this struct into the database.
 *          This action is required due to different data layouts. This driver
 *          always stores its cell-voltages in an array with 14*32 = 448 cells
 *          in order to reduce the amount of different configurations and
 *          variants.
 *
 *          This function maps these values into the database-struct which
 *          scales with the number of connected cells and monitoring ICs.
 * @param[in]   kpkInstance pointer to the #MXM_MONITORING_INSTANCE_s struct
 * @return  #STD_OK if the action was successful or #STD_NOT_OK otherwise
 */
extern STD_RETURN_TYPE_e MXM_ParseVoltagesIntoDB(const MXM_MONITORING_INSTANCE_s *const kpkInstance);

/**
 * @brief   returns the model ID of the daisy chain
 * @details this function is to be implemented by the driver in order to
 *          tell the rest of the driver which model is used. Currently, this
 *          implementation does not support mixed model daisy-chains.
 * @returns model ID of daisy-chain
 */
extern MXM_MODEL_ID_e MXM_GetModelIdOfDaisyChain(void);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
extern STD_RETURN_TYPE_e TEST_MXM_ParseVoltageReadallTest(MXM_MONITORING_INSTANCE_s *pInstance);
extern STD_RETURN_TYPE_e TEST_MXM_ParseVoltagesIntoDB(MXM_MONITORING_INSTANCE_s *pInstance);
extern STD_RETURN_TYPE_e TEST_MXM_ParseVoltageReadall(
    uint8_t *voltRxBuffer,
    uint16_t voltRxBufferLength,
    MXM_DATA_STORAGE_s *datastorage,
    MXM_CONVERSION_TYPE_e conversionType);
#endif

#endif /* FOXBMS__MXM_1785X_H_ */
