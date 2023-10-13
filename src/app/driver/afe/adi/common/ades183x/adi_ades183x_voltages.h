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
 * @file    adi_ades183x_voltages.h
 * @author  foxBMS Team
 * @date    2022-12-07 (date of creation)
 * @updated 2023-10-12 (date of last update)
 * @version v1.6.0
 * @ingroup DRIVERS
 * @prefix  ADI
 *
 * @brief   Header file of some software
 *
 */

#ifndef FOXBMS__ADI_ADES183X_VOLTAGES_H_
#define FOXBMS__ADI_ADES183X_VOLTAGES_H_

/*========== Includes =======================================================*/

#include "adi_ades183x_defs.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/
/** Struct to group the commands to read the registers A to F */
typedef struct {
    uint16_t registerA[ADI_COMMAND_DEFINITION_LENGTH];
    uint16_t registerB[ADI_COMMAND_DEFINITION_LENGTH];
    uint16_t registerC[ADI_COMMAND_DEFINITION_LENGTH];
    uint16_t registerD[ADI_COMMAND_DEFINITION_LENGTH];
    uint16_t registerE[ADI_COMMAND_DEFINITION_LENGTH];
    uint16_t registerF[ADI_COMMAND_DEFINITION_LENGTH];
} ADI_COMMAND_READ_REGISTERS_s;

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/

/**
 * @brief   Converts index of read voltages.
 * @details This function translates the indexes of read voltages so that
 *          if some inputs are unused, they do not appear in the final
 *          cell voltage table
 * @param   registerVoltageIndex    index of the read voltage ion the ades183x
 *                                  register
 * @return  index where the voltage must be stored if it is used
 */
extern uint16_t ADI_GetStoredVoltageIndex(uint16_t registerVoltageIndex);

/**
 * @brief   Reads and stores cell voltages.
 * @details This function, reads the raw values from the registers, and
 *          calls ADI_SaveRxToCellVoltageBuffer() which convert them into
 *          voltages and stores them.
 * @param   adiState        state of the ADI driver
 * @param   registerType    type of voltage register to read
 * @param   storeLocation   location where read data has to be stored
 */
extern void ADI_GetVoltages(
    ADI_STATE_s *adiState,
    ADI_VOLTAGE_REGISTER_TYPE_e registerType,
    ADI_VOLTAGE_STORE_LOCATION_e storeLocation);

/**
 * @brief   Stop cell voltage measurement.
 * @details This function should be called before activation of balancing
 * @param   adiState state of the driver
 */
extern void ADI_StopContinuousCellVoltageMeasurements(ADI_STATE_s *adiState);

/**
 * @brief   Restart cell voltage measurement.
 * @details This function should be called after deactivation of balancing
 * @param   adiState state of the driver
 */
extern void ADI_RestartContinuousCellVoltageMeasurements(ADI_STATE_s *adiState);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
extern void TEST_ADI_CopyCommandBytesCellVoltageRegister(
    ADI_COMMAND_READ_REGISTERS_s *commandBytesToReadVoltageRegisters);
extern void TEST_ADI_CopyCommandBytesAverageCellVoltageRegisters(
    ADI_COMMAND_READ_REGISTERS_s *commandBytesToReadVoltageRegisters);
extern void TEST_ADI_CopyCommandBytesFilteredCellVoltageRegisters(
    ADI_COMMAND_READ_REGISTERS_s *commandBytesToReadVoltageRegisters);
extern void TEST_ADI_CopyCommandBytesRedundantCellVoltageRegisters(
    ADI_COMMAND_READ_REGISTERS_s *commandBytesToReadVoltageRegisters);
extern void TEST_ADI_ReadAndStoreVoltages(
    ADI_STATE_s *adiState,
    ADI_COMMAND_READ_REGISTERS_s *commandBytesToReadVoltageRegisters,
    ADI_VOLTAGE_STORE_LOCATION_e storeLocation);
extern void TEST_ADI_SaveRxToCellVoltageBuffer(
    ADI_STATE_s *adiState,
    uint8_t *data,
    uint8_t registerSet,
    ADI_VOLTAGE_STORE_LOCATION_e storeLocation);
#endif

#endif /* FOXBMS__ADI_ADES183X_VOLTAGES_H_ */
