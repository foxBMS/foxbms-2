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
 * @file    nxp_mc33775a.h
 * @author  foxBMS Team
 * @date    2020-05-08 (date of creation)
 * @updated 2023-10-12 (date of last update)
 * @version v1.6.0
 * @ingroup DRIVERS
 * @prefix  N775
 *
 * @brief   Headers for the driver for the MC33775A analog front-end.
 * @details Defines the interface to perform measurements with the MC33775A:
 *          It further defines functions to write to and read from the I2C bus
 *          that is implement of the AFE.
 */

#ifndef FOXBMS__NXP_MC33775A_H_
#define FOXBMS__NXP_MC33775A_H_

/*========== Includes =======================================================*/
#include "nxp_mc33775a_cfg.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Extern Constant and Variable Declarations ======================*/

/** state of the state machine for the MC33775A driver */
extern N775_STATE_s n775_stateBase;

/*========== Extern Function Prototypes =====================================*/

/**
 * @brief   trigger a read on the I2C bus of the slave.
 * @param   module          module number to address in the daisy-chain
 * @param   deviceAddress   address of the I2C device addressed
 * @param   pData           data read on I2C bus
 * @param   dataLength      number of bytes to read
 */
extern STD_RETURN_TYPE_e N775_I2cRead(uint8_t module, uint8_t deviceAddress, uint8_t *pData, uint8_t dataLength);

/**
 * @brief  trigger a write on the I2C bus of the slave.
 * @param  module         module number to address in the daisy-chain
 * @param  deviceAddress  address of the I2C device addressed
 * @param  pData          data to write on I2C bus
 * @param  dataLength     number of bytes to write
 */
extern STD_RETURN_TYPE_e N775_I2cWrite(uint8_t module, uint8_t deviceAddress, uint8_t *pData, uint8_t dataLength);

/**
 * @brief   trigger a read on the I2C bus of the slave, first write address of
 *          register to read.
 * @param   module          module number to address in the daisy-chain
 * @param   deviceAddress   address of the I2C device addressed
 * @param   pDataWrite      data written on I2C bus
 * @param   writeDataLength number of bytes to write
 * @param   pDataRead       data read on I2C bus
 * @param   readDataLength  number of bytes to read
 */
extern STD_RETURN_TYPE_e N775_I2cWriteRead(
    uint8_t module,
    uint8_t deviceAddress,
    uint8_t *pDataWrite,
    uint8_t writeDataLength,
    uint8_t *pDataRead,
    uint8_t readDataLength);

/**
 * @brief   gets the measurement initialization status.
 * @param   pState state of the N775A driver
 * @return  true if a first measurement cycle was made, false otherwise
 */
extern bool N775_IsFirstMeasurementCycleFinished(N775_STATE_s *pState);

/**
 * @brief   trigger function for the N775 driver state machine.
 * @details This function contains the sequence of events in the N775 state
 *          machine.
 *          It must be called time-triggered, every 1ms.
 * @param   pState state of the N775A driver
 */
extern void N775_Measure(N775_STATE_s *pState);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
#include "nxp_mc33775a-ll.h"
extern void TEST_N775_BalanceControl(N775_STATE_s *pState);
extern void TEST_N775_BalanceSetup(N775_STATE_s *pState);
extern void TEST_N775_CaptureMeasurement(N775_STATE_s *pState);
extern STD_RETURN_TYPE_e TEST_N775_Enumerate(N775_STATE_s *pState);
extern void TEST_N775_ErrorHandling(N775_STATE_s *pState, N775_COMMUNICATION_STATUS_e returnedValue, uint8_t module);
extern void TEST_N775_IncrementMuxIndex(N775_STATE_s *pState);
extern void TEST_N775_IncrementStringSequence(N775_STATE_s *pState);
extern void TEST_N775_Initialize(N775_STATE_s *pState);
extern void TEST_N775_InitializeDatabase(N775_STATE_s *pState);
extern void TEST_N775_InitializeI2c(N775_STATE_s *pState);
extern void TEST_N775_ResetStringSequence(N775_STATE_s *pState);
extern void TEST_N775_ResetMuxIndex(N775_STATE_s *pState);
extern void TEST_N775_SetFirstMeasurementCycleFinished(N775_STATE_s *pState);
extern STD_RETURN_TYPE_e TEST_N775_SetMuxChannel(N775_STATE_s *pState);
extern void TEST_N775_StartMeasurement(N775_STATE_s *pState);
extern STD_RETURN_TYPE_e TEST_N775_TransmitI2c(N775_STATE_s *pState);
extern void TEST_N775_Wait(uint32_t milliseconds);
#endif

#endif /* FOXBMS__NXP_MC33775A_H_ */
