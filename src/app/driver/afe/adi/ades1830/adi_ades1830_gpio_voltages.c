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
 * @file    adi_ades1830_gpio_voltages.c
 * @author  foxBMS Team
 * @date    2019-08-27 (date of creation)
 * @updated 2023-10-12 (date of last update)
 * @version v1.6.0
 * @ingroup SOME_GROUP
 * @prefix  ADI
 *
 * @brief   Implementation of some software
 *
 */

/*========== Includes =======================================================*/
/* clang-format off */
#include "adi_ades183x_gpio_voltages.h"
/* clang-format on */

#include "adi_ades183x_cfg.h"

#include "adi_ades183x_buffers.h"
#include "adi_ades183x_commands.h"
#include "adi_ades183x_commands_voltages.h"
#include "adi_ades183x_defs.h"
#include "adi_ades183x_helpers.h"
#include "adi_ades183x_voltages.h"
#include "fassert.h"

#include <math.h>
#include <stdbool.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/
/**
 * @brief   Saves the GPIO voltage values read from the daisy-chain.
 * @details There are up to 5 registers to read _(A, B, C, D, E) to get all
 *          GPIO voltages.
 *          This function is called to store the result from the transmission
 *          buffer to the appropriate location in the driver.
 * @param   adiState        state of the ADI driver
 * @param   data            receive buffer
 * @param   registerSet     auxiliary register that was read (voltage register
 *                          A, B, C or D).
 * @param   storeLocation   location where read data has to be stored
 */
static void ADI_SaveRxToGpioVoltageBuffer(
    ADI_STATE_s *adiState,
    uint8_t *data,
    uint8_t registerSet,
    ADI_AUXILIARY_STORE_LOCATION_e storeLocation);

/*========== Static Function Implementations ================================*/
/* RequirementId: D7.1 V0R4 FUN-2.10.01.02 */
static void ADI_SaveRxToGpioVoltageBuffer(
    ADI_STATE_s *adiState,
    uint8_t *data,
    uint8_t registerSet,
    ADI_AUXILIARY_STORE_LOCATION_e storeLocation) {
    FAS_ASSERT(adiState != NULL_PTR);
    FAS_ASSERT(data != NULL_PTR);
    FAS_ASSERT(
        (registerSet == ADI_AUXILIARY_RESULT_REGISTER_SET_A) || (registerSet == ADI_AUXILIARY_RESULT_REGISTER_SET_B) ||
        (registerSet == ADI_AUXILIARY_RESULT_REGISTER_SET_C) || (registerSet == ADI_AUXILIARY_RESULT_REGISTER_SET_D) ||
        (registerSet == ADI_AUXILIARY_RESULT_REGISTER_SET_E));
    FAS_ASSERT(
        (storeLocation == ADI_AUXILIARY_VOLTAGE) || (storeLocation == ADI_REDUNDANT_AUXILIARY_VOLTAGE) ||
        (storeLocation == ADI_AUXILIARY_VOLTAGE_OPEN_WIRE));

    uint16_t cellOffset                               = 0u;
    uint16_t voltageIndex                             = 0u;
    uint16_t rawValue                                 = 0u;
    int16_t signedValue                               = 0;
    float_t floatVoltage                              = 0.0f;
    int16_t voltage                                   = 0;
    uint16_t bufferLSB                                = 0u;
    uint16_t bufferMSB                                = 0u;
    uint8_t numberOfVoltagesInRegister                = 0u;
    uint8_t voltageStartNumber                        = 0u;
    DATA_BLOCK_ALL_GPIO_VOLTAGES_s *pGpioVoltageTable = NULL_PTR;

    switch (storeLocation) {
        case ADI_AUXILIARY_VOLTAGE:
            pGpioVoltageTable = adiState->data.allGpioVoltages;
            break;
        case ADI_REDUNDANT_AUXILIARY_VOLTAGE:
            pGpioVoltageTable = adiState->data.allGpioVoltagesRedundant;
            break;
        case ADI_AUXILIARY_VOLTAGE_OPEN_WIRE:
            pGpioVoltageTable = adiState->data.allGpioVoltageOpenWire;
            break;
        default:                  /* LCOV_EXCL_LINE */
            FAS_ASSERT(FAS_TRAP); /* LCOV_EXCL_LINE */
            break;                /* LCOV_EXCL_LINE */
    }

    /* Default: 3 GPIO voltages in AUX registers */
    numberOfVoltagesInRegister = ADI_MAX_NUMBER_OF_GPIO_VOLTAGES_IN_REGISTER;

    switch (registerSet) {
        case ADI_AUXILIARY_RESULT_REGISTER_SET_A:
            /* RDAUXA command -> auxiliary register group A */
            cellOffset = ADI_VOLTAGE_00_02_OFFSET;
            break;
        case ADI_AUXILIARY_RESULT_REGISTER_SET_B:
            /* RDAUXB command -> auxiliary register group B */
            cellOffset = ADI_VOLTAGE_03_05_OFFSET;
            break;
        case ADI_AUXILIARY_RESULT_REGISTER_SET_C:
            /* RDAUXC command -> auxiliary register group C */
            cellOffset = ADI_VOLTAGE_06_08_OFFSET;
            break;
        case ADI_AUXILIARY_RESULT_REGISTER_SET_D:
            /* RDAUXD command -> auxiliary register group D */
            cellOffset                 = ADI_VOLTAGE_09_11_OFFSET;
            numberOfVoltagesInRegister = ADI_NUMBER_OF_GPIO_VOLTAGES_IN_REGISTER_D;
            break;
        default:                  /* LCOV_EXCL_LINE */
            FAS_ASSERT(FAS_TRAP); /* LCOV_EXCL_LINE */
            break;                /* LCOV_EXCL_LINE */
    }

    for (uint16_t m = 0u; m < ADI_N_ADI; m++) {
        /* parse voltages contained in one register */
        for (uint16_t gpio = voltageStartNumber; gpio < numberOfVoltagesInRegister; gpio++) {
            voltageIndex = gpio + cellOffset;
            if (voltageIndex < BS_NR_OF_GPIOS_PER_MODULE) {
                bufferMSB = (uint16_t)(data
                                           [(ADI_RAW_VOLTAGE_SIZE_IN_BYTES * gpio) +
                                            (m * ADI_MAX_REGISTER_SIZE_IN_BYTES) + 1u]);
                bufferLSB =
                    (uint16_t)(data[(ADI_RAW_VOLTAGE_SIZE_IN_BYTES * gpio) + (m * ADI_MAX_REGISTER_SIZE_IN_BYTES)]);
                rawValue     = bufferLSB | (bufferMSB << ADI_BYTE_SHIFT);
                signedValue  = (int16_t)rawValue;
                floatVoltage = ((float_t)signedValue * ADI_VOLTAGE_CONVERSION_FACTOR * ADI_VOLTAGE_CONVERSION_UNIT) +
                               ADI_VOLTAGE_CONVERSION_OFFSET;
                voltage = (int16_t)floatVoltage; /* Unit mV */

                /* RequirementId: D7.1 V0R4 SIF-4.40.02.01 */
                /* Check that register does not contain cleared value */
                if (rawValue != ADI_REGISTER_CLEARED_VALUE) {
                    adiState->data.errorTable->auxiliaryRegisterContentIsNotStuck[adiState->currentString][m] = true;
                    /* Check PEC for every IC in the daisy-chain */
                    if (adiState->data.errorTable->crcIsOk[adiState->currentString][m] == true) {
                        pGpioVoltageTable->gpioVoltages_mV[adiState->currentString]
                                                          [voltageIndex + (m * BS_NR_OF_GPIOS_PER_MODULE)] = voltage;
                    }
                } else {
                    adiState->data.errorTable->auxiliaryRegisterContentIsNotStuck[adiState->currentString][m] = false;
                }
            }
        }
    }
}

/*========== Extern Function Implementations ================================*/
extern void ADI_GetGpioVoltages(
    ADI_STATE_s *adiState,
    ADI_AUXILIARY_REGISTER_TYPE_e registerType,
    ADI_AUXILIARY_STORE_LOCATION_e storeLocation) {
    FAS_ASSERT(adiState != NULL_PTR);
    FAS_ASSERT((registerType == ADI_AUXILIARY_REGISTER) || (registerType == ADI_REDUNDANT_AUXILIARY_REGISTER));
    FAS_ASSERT(
        (storeLocation == ADI_AUXILIARY_VOLTAGE) || (storeLocation == ADI_REDUNDANT_AUXILIARY_VOLTAGE) ||
        (storeLocation == ADI_AUXILIARY_VOLTAGE_OPEN_WIRE));
    uint16_t registerA[ADI_COMMAND_DEFINITION_LENGTH] = {0};
    uint16_t registerB[ADI_COMMAND_DEFINITION_LENGTH] = {0};
    uint16_t registerC[ADI_COMMAND_DEFINITION_LENGTH] = {0};
    uint16_t registerD[ADI_COMMAND_DEFINITION_LENGTH] = {0};
    uint16_t registerE[ADI_COMMAND_DEFINITION_LENGTH] = {0};

    switch (registerType) {
        case ADI_AUXILIARY_REGISTER:
            ADI_CopyCommandBits(adi_cmdRdauxa, registerA);
            ADI_CopyCommandBits(adi_cmdRdauxb, registerB);
            ADI_CopyCommandBits(adi_cmdRdauxc, registerC);
            ADI_CopyCommandBits(adi_cmdRdauxd, registerD);
            ADI_CopyCommandBits(adi_cmdRdauxe, registerE);
            break;
        case ADI_REDUNDANT_AUXILIARY_REGISTER:
            ADI_CopyCommandBits(adi_cmdRdraxa, registerA);
            ADI_CopyCommandBits(adi_cmdRdraxb, registerB);
            ADI_CopyCommandBits(adi_cmdRdraxc, registerC);
            ADI_CopyCommandBits(adi_cmdRdraxd, registerD);
            break;
        default:                  /* LCOV_EXCL_LINE */
            FAS_ASSERT(FAS_TRAP); /* LCOV_EXCL_LINE */
            break;                /* LCOV_EXCL_LINE */
    }

    ADI_CopyCommandBits(registerA, adi_command);
    ADI_ReadRegister(adi_command, adi_dataReceive, adiState);
    ADI_SaveRxToGpioVoltageBuffer(adiState, adi_dataReceive, ADI_AUXILIARY_RESULT_REGISTER_SET_A, storeLocation);

    ADI_CopyCommandBits(registerB, adi_command);
    ADI_ReadRegister(adi_command, adi_dataReceive, adiState);
    ADI_SaveRxToGpioVoltageBuffer(adiState, adi_dataReceive, ADI_AUXILIARY_RESULT_REGISTER_SET_B, storeLocation);

    ADI_CopyCommandBits(registerC, adi_command);
    ADI_ReadRegister(adi_command, adi_dataReceive, adiState);
    ADI_SaveRxToGpioVoltageBuffer(adiState, adi_dataReceive, ADI_AUXILIARY_RESULT_REGISTER_SET_C, storeLocation);

    ADI_CopyCommandBits(registerD, adi_command);
    ADI_ReadRegister(adi_command, adi_dataReceive, adiState);
    ADI_SaveRxToGpioVoltageBuffer(adiState, adi_dataReceive, ADI_AUXILIARY_RESULT_REGISTER_SET_D, storeLocation);
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
extern void TEST_ADI_SaveRxToGpioVoltageBuffer(
    ADI_STATE_s *adiState,
    uint8_t *data,
    uint8_t registerSet,
    ADI_AUXILIARY_STORE_LOCATION_e storeLocation) {
    ADI_SaveRxToGpioVoltageBuffer(adiState, data, registerSet, storeLocation);
}
#endif
