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
 * @file    adi_ades183x_voltages.c
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
#include "adi_ades183x_voltages.h"

#include "adi_ades183x_cfg.h"

#include "adi_ades183x_buffers.h"
#include "adi_ades183x_commands.h"
#include "adi_ades183x_commands_voltages.h"
#include "adi_ades183x_diagnostic.h"
#include "adi_ades183x_helpers.h"
#include "fassert.h"

#include <math.h>
#include <stdbool.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/**
 * @brief   Copies the related command bytes to read the average voltage
 *          registers
 * @param   commandBytesToReadVoltageRegisters  command bytes to be copied
 */
static void ADI_CopyCommandBytesAverageCellVoltageRegisters(
    ADI_COMMAND_READ_REGISTERS_s *commandBytesToReadVoltageRegisters);

/**
 * @brief   Copies the related command bytes to read the voltage registers
 * @param   commandBytesToReadVoltageRegisters  command bytes to be copied
 */
static void ADI_CopyCommandBytesCellVoltageRegister(ADI_COMMAND_READ_REGISTERS_s *commandBytesToReadVoltageRegisters);

/**
 * @brief   Copies the related command bytes to read the filtered voltage
 *          registers
 * @param   commandBytesToReadVoltageRegisters  command bytes to be copied
 */
static void ADI_CopyCommandBytesFilteredCellVoltageRegisters(
    ADI_COMMAND_READ_REGISTERS_s *commandBytesToReadVoltageRegisters);

/**
 * @brief   Copies the related command bytes to read the redundant voltage
 *          registers
 * @param   commandBytesToReadVoltageRegisters  command bytes to be copied
 */
static void ADI_CopyCommandBytesRedundantCellVoltageRegisters(
    ADI_COMMAND_READ_REGISTERS_s *commandBytesToReadVoltageRegisters);

/**
 * @brief   Copies the related command bytes to read the average voltage
 *          registers
 * @param   adiState                         state of the ADI driver
 * @param   commandBytesToReadVoltageRegisters  command bytes to be copied
 * @param   storeLocation                       location where read data has to
 *                                              be stored
 */
static void ADI_ReadAndStoreVoltages(
    ADI_STATE_s *adiState,
    ADI_COMMAND_READ_REGISTERS_s *commandBytesToReadVoltageRegisters,
    ADI_VOLTAGE_STORE_LOCATION_e storeLocation);

/**
 * @brief   Saves the cell voltage values read from the daisy-chain.
 * @details There are 6 register to read _(A, B, C, D, E, F) to get all cell
 *          voltages.
 *          This function is called to store the result from the transmission
 *          buffer to the appropriate location in the driver.
 * @param   adiState        state of the ADI driver
 * @param   data            receive buffer
 * @param   registerSet     voltage register that was read (voltage register A,
 *                          B, C, D, E or F)
 * @param   storeLocation   location where read data has to be stored
 */
static void ADI_SaveRxToCellVoltageBuffer(
    ADI_STATE_s *adiState,
    uint8_t *data,
    uint8_t registerSet,
    ADI_VOLTAGE_STORE_LOCATION_e storeLocation);

/*========== Static Function Implementations ================================*/

static void ADI_CopyCommandBytesCellVoltageRegister(ADI_COMMAND_READ_REGISTERS_s *commandBytesToReadVoltageRegisters) {
    FAS_ASSERT(commandBytesToReadVoltageRegisters != NULL_PTR);
    ADI_CopyCommandBits(adi_cmdRdcva, commandBytesToReadVoltageRegisters->registerA);
    ADI_CopyCommandBits(adi_cmdRdcvb, commandBytesToReadVoltageRegisters->registerB);
    ADI_CopyCommandBits(adi_cmdRdcvc, commandBytesToReadVoltageRegisters->registerC);
    ADI_CopyCommandBits(adi_cmdRdcvd, commandBytesToReadVoltageRegisters->registerD);
    ADI_CopyCommandBits(adi_cmdRdcve, commandBytesToReadVoltageRegisters->registerE);
    ADI_CopyCommandBits(adi_cmdRdcvf, commandBytesToReadVoltageRegisters->registerF);
}

static void ADI_CopyCommandBytesAverageCellVoltageRegisters(
    ADI_COMMAND_READ_REGISTERS_s *commandBytesToReadVoltageRegisters) {
    FAS_ASSERT(commandBytesToReadVoltageRegisters != NULL_PTR);
    ADI_CopyCommandBits(adi_cmdRdaca, commandBytesToReadVoltageRegisters->registerA);
    ADI_CopyCommandBits(adi_cmdRdacb, commandBytesToReadVoltageRegisters->registerB);
    ADI_CopyCommandBits(adi_cmdRdacc, commandBytesToReadVoltageRegisters->registerC);
    ADI_CopyCommandBits(adi_cmdRdacd, commandBytesToReadVoltageRegisters->registerD);
    ADI_CopyCommandBits(adi_cmdRdace, commandBytesToReadVoltageRegisters->registerE);
    ADI_CopyCommandBits(adi_cmdRdacf, commandBytesToReadVoltageRegisters->registerF);
}

static void ADI_CopyCommandBytesFilteredCellVoltageRegisters(
    ADI_COMMAND_READ_REGISTERS_s *commandBytesToReadVoltageRegisters) {
    FAS_ASSERT(commandBytesToReadVoltageRegisters != NULL_PTR);
    ADI_CopyCommandBits(adi_cmdRdfca, commandBytesToReadVoltageRegisters->registerA);
    ADI_CopyCommandBits(adi_cmdRdfcb, commandBytesToReadVoltageRegisters->registerB);
    ADI_CopyCommandBits(adi_cmdRdfcc, commandBytesToReadVoltageRegisters->registerC);
    ADI_CopyCommandBits(adi_cmdRdfcd, commandBytesToReadVoltageRegisters->registerD);
    ADI_CopyCommandBits(adi_cmdRdfce, commandBytesToReadVoltageRegisters->registerE);
    ADI_CopyCommandBits(adi_cmdRdfcf, commandBytesToReadVoltageRegisters->registerF);
}

static void ADI_CopyCommandBytesRedundantCellVoltageRegisters(
    ADI_COMMAND_READ_REGISTERS_s *commandBytesToReadVoltageRegisters) {
    FAS_ASSERT(commandBytesToReadVoltageRegisters != NULL_PTR);
    ADI_CopyCommandBits(adi_cmdRdsva, commandBytesToReadVoltageRegisters->registerA);
    ADI_CopyCommandBits(adi_cmdRdsvb, commandBytesToReadVoltageRegisters->registerB);
    ADI_CopyCommandBits(adi_cmdRdsvc, commandBytesToReadVoltageRegisters->registerC);
    ADI_CopyCommandBits(adi_cmdRdsvd, commandBytesToReadVoltageRegisters->registerD);
    ADI_CopyCommandBits(adi_cmdRdsve, commandBytesToReadVoltageRegisters->registerE);
    ADI_CopyCommandBits(adi_cmdRdsvf, commandBytesToReadVoltageRegisters->registerF);
}

static void ADI_ReadAndStoreVoltages(
    ADI_STATE_s *adiState,
    ADI_COMMAND_READ_REGISTERS_s *commandBytesToReadVoltageRegisters,
    ADI_VOLTAGE_STORE_LOCATION_e storeLocation) {
    FAS_ASSERT(adiState != NULL_PTR);
    FAS_ASSERT(commandBytesToReadVoltageRegisters != NULL_PTR);
    FAS_ASSERT(storeLocation < ADI_VOLTAGE_STORE_LOCATION_E_MAX);

    ADI_ReadRegister(commandBytesToReadVoltageRegisters->registerA, adi_dataReceive, adiState);
    ADI_SaveRxToCellVoltageBuffer(adiState, adi_dataReceive, ADI_RESULT_REGISTER_SET_A, storeLocation);

    ADI_ReadRegister(commandBytesToReadVoltageRegisters->registerB, adi_dataReceive, adiState);
    ADI_SaveRxToCellVoltageBuffer(adiState, adi_dataReceive, ADI_RESULT_REGISTER_SET_B, storeLocation);

    ADI_ReadRegister(commandBytesToReadVoltageRegisters->registerC, adi_dataReceive, adiState);
    ADI_SaveRxToCellVoltageBuffer(adiState, adi_dataReceive, ADI_RESULT_REGISTER_SET_C, storeLocation);

    ADI_ReadRegister(commandBytesToReadVoltageRegisters->registerD, adi_dataReceive, adiState);
    ADI_SaveRxToCellVoltageBuffer(adiState, adi_dataReceive, ADI_RESULT_REGISTER_SET_D, storeLocation);

    ADI_ReadRegister(commandBytesToReadVoltageRegisters->registerE, adi_dataReceive, adiState);
    ADI_SaveRxToCellVoltageBuffer(adiState, adi_dataReceive, ADI_RESULT_REGISTER_SET_E, storeLocation);

    ADI_ReadRegister(commandBytesToReadVoltageRegisters->registerF, adi_dataReceive, adiState);
    ADI_SaveRxToCellVoltageBuffer(adiState, adi_dataReceive, ADI_RESULT_REGISTER_SET_F, storeLocation);
}

/* RequirementId: D7.1 V0R4 FUN-1.10.01.03 */
static void ADI_SaveRxToCellVoltageBuffer(
    ADI_STATE_s *adiState,
    uint8_t *data,
    uint8_t registerSet,
    ADI_VOLTAGE_STORE_LOCATION_e storeLocation) {
    FAS_ASSERT(adiState != NULL_PTR);
    FAS_ASSERT(data != NULL_PTR);
    FAS_ASSERT(
        (registerSet == ADI_RESULT_REGISTER_SET_A) || (registerSet == ADI_RESULT_REGISTER_SET_B) ||
        (registerSet == ADI_RESULT_REGISTER_SET_C) || (registerSet == ADI_RESULT_REGISTER_SET_D) ||
        (registerSet == ADI_RESULT_REGISTER_SET_E) || (registerSet == ADI_RESULT_REGISTER_SET_F));
    FAS_ASSERT(storeLocation < ADI_VOLTAGE_STORE_LOCATION_E_MAX);

    uint16_t cellOffset                      = 0u;
    uint16_t registerVoltageIndex            = 0u;
    uint16_t storedVoltageIndex              = 0u;
    uint16_t rawValue                        = 0u;
    int16_t signedValue                      = 0;
    float_t floatVoltage                     = 0.0f;
    int16_t voltage                          = 0;
    uint16_t bufferLSB                       = 0u;
    uint16_t bufferMSB                       = 0u;
    DATA_BLOCK_CELL_VOLTAGE_s *pVoltageTable = NULL_PTR;
    static uint16_t numberValidMeasurements  = 0;

    if ((storeLocation == ADI_CELL_VOLTAGE) && (registerSet == ADI_RESULT_REGISTER_SET_A)) {
        numberValidMeasurements = 0u;
    }

    switch (storeLocation) {
        case ADI_CELL_VOLTAGE:
            pVoltageTable = adiState->data.cellVoltage;
            break;
        case ADI_AVERAGE_CELL_VOLTAGE:
            pVoltageTable = adiState->data.cellVoltageAverage;
            break;
        case ADI_FILTERED_CELL_VOLTAGE:
            pVoltageTable = adiState->data.cellVoltageFiltered;
            break;
        case ADI_REDUNDANT_CELL_VOLTAGE:
            pVoltageTable = adiState->data.cellVoltageRedundant;
            break;
        case ADI_CELL_VOLTAGE_OPEN_WIRE_EVEN:
            pVoltageTable = adiState->data.cellVoltageOpenWireEven;
            break;
        case ADI_CELL_VOLTAGE_OPEN_WIRE_ODD:
            pVoltageTable = adiState->data.cellVoltageOpenWireOdd;
            break;
        case ADI_CELL_VOLTAGE_AVERAGE_OPEN_WIRE:
            pVoltageTable = adiState->data.cellVoltageAverageOpenWire;
            break;
        case ADI_CELL_VOLTAGE_REDUNDANT_OPEN_WIRE:
            pVoltageTable = adiState->data.cellVoltageRedundantOpenWire;
            break;
        default:                  /* LCOV_EXCL_LINE */
            FAS_ASSERT(FAS_TRAP); /* LCOV_EXCL_LINE */
            break;                /* LCOV_EXCL_LINE */
    }

    switch (registerSet) {
        case ADI_RESULT_REGISTER_SET_A: /* Voltage register group A: 3 voltages */
            cellOffset = ADI_VOLTAGE_00_02_OFFSET;
            break;
        case ADI_RESULT_REGISTER_SET_B: /* Voltage register group B: 3 voltages */
            cellOffset = ADI_VOLTAGE_03_05_OFFSET;
            break;
        case ADI_RESULT_REGISTER_SET_C: /* Voltage register group C: 3 voltages */
            cellOffset = ADI_VOLTAGE_06_08_OFFSET;
            break;
        case ADI_RESULT_REGISTER_SET_D: /* Voltage register group D: 3 voltages */
            cellOffset = ADI_VOLTAGE_09_11_OFFSET;
            break;
        case ADI_RESULT_REGISTER_SET_E: /* Voltage register group E: 3 voltages */
            cellOffset = ADI_VOLTAGE_12_14_OFFSET;
            break;
        case ADI_RESULT_REGISTER_SET_F: /* Voltage register group F: */
            cellOffset = ADI_VOLTAGE_15_18_OFFSET;
            break;
        default:                  /* LCOV_EXCL_LINE */
            FAS_ASSERT(FAS_TRAP); /* LCOV_EXCL_LINE */
            break;                /* LCOV_EXCL_LINE */
    }

    for (uint16_t m = 0u; m < ADI_N_ADI; m++) {
        /* Parse all voltages contained in one register */
        for (uint16_t c = 0u; c < ADI_MAX_NUMBER_OF_VOLTAGES_IN_REGISTER; c++) {
            registerVoltageIndex = c + cellOffset;
            if (registerVoltageIndex < ADI_MAX_SUPPORTED_CELLS) {
                /* Check if cell voltage input is used */
                if (adi_voltageInputsUsed[registerVoltageIndex] == 1u) {
                    /* If input used, translate number to store voltage at the appropriate location */
                    storedVoltageIndex = ADI_GetStoredVoltageIndex(registerVoltageIndex);
                    if (storedVoltageIndex < BS_NR_OF_CELL_BLOCKS_PER_MODULE) {
                        bufferMSB   = (uint16_t)(data
                                                   [(ADI_RAW_VOLTAGE_SIZE_IN_BYTES * c) +
                                                    (m * ADI_MAX_REGISTER_SIZE_IN_BYTES) + 1u]);
                        bufferLSB   = (uint16_t)(data
                                                   [(ADI_RAW_VOLTAGE_SIZE_IN_BYTES * c) +
                                                    (m * ADI_MAX_REGISTER_SIZE_IN_BYTES)]);
                        rawValue    = bufferLSB | (bufferMSB << ADI_BYTE_SHIFT);
                        signedValue = (int16_t)rawValue;
                        floatVoltage =
                            ((float_t)signedValue * ADI_VOLTAGE_CONVERSION_FACTOR * ADI_VOLTAGE_CONVERSION_UNIT) +
                            ADI_VOLTAGE_CONVERSION_OFFSET;
                        voltage = (int16_t)floatVoltage; /* Unit mV */

                        pVoltageTable->cellVoltage_mV[adiState->currentString][m][storedVoltageIndex] = voltage;
                        /* RequirementId: D7.1 V0R4 SIF-4.40.01.01 */
                        /* Check that register does not contain cleared value */
                        if (rawValue != ADI_REGISTER_CLEARED_VALUE) {
                            adiState->data.errorTable->voltageRegisterContentIsNotStuck[adiState->currentString][m] =
                                true;
                        } else {
                            adiState->data.errorTable->voltageRegisterContentIsNotStuck[adiState->currentString][m] =
                                false;
                        }

                        if (storeLocation == ADI_CELL_VOLTAGE) {
                            if (ADI_EvaluateDiagnosticCellVoltages(adiState, m) == false) {
                                adiState->data.cellVoltage->invalidCellVoltage[adiState->currentString][m] |=
                                    (0x01u << storedVoltageIndex);
                            } else {
                                adiState->data.cellVoltage->invalidCellVoltage[adiState->currentString][m] &=
                                    (~0x01u << storedVoltageIndex);
                                numberValidMeasurements++;
                            }
                        }
                    }
                }
            }
        }
    }
    if ((storeLocation == ADI_CELL_VOLTAGE) && (registerSet == ADI_RESULT_REGISTER_SET_F)) {
        adiState->data.cellVoltage->nrValidCellVoltages[adiState->currentString] = numberValidMeasurements;
    }
}

/*========== Extern Function Implementations ================================*/
/* RequirementId: D7.1 V0R4 FUN-1.10.01.01 */
extern void ADI_GetVoltages(
    ADI_STATE_s *adiState,
    ADI_VOLTAGE_REGISTER_TYPE_e registerType,
    ADI_VOLTAGE_STORE_LOCATION_e storeLocation) {
    FAS_ASSERT(adiState != NULL_PTR);
    FAS_ASSERT(
        (registerType == ADI_CELL_VOLTAGE_REGISTER) || (registerType == ADI_AVERAGE_CELL_VOLTAGE_REGISTER) ||
        (registerType == ADI_FILTERED_CELL_VOLTAGE_REGISTER) || (registerType == ADI_REDUNDANT_CELL_VOLTAGE_REGISTER));
    FAS_ASSERT(storeLocation < ADI_VOLTAGE_STORE_LOCATION_E_MAX);

    ADI_COMMAND_READ_REGISTERS_s commandBytesToReadVoltageRegisters = {
        .registerA = {0},
        .registerB = {0},
        .registerC = {0},
        .registerD = {0},
        .registerE = {0},
        .registerF = {0},
    };

    switch (registerType) {
        case ADI_CELL_VOLTAGE_REGISTER:
            ADI_CopyCommandBytesCellVoltageRegister(&commandBytesToReadVoltageRegisters);
            break;
        case ADI_AVERAGE_CELL_VOLTAGE_REGISTER:
            ADI_CopyCommandBytesAverageCellVoltageRegisters(&commandBytesToReadVoltageRegisters);
            break;
        case ADI_FILTERED_CELL_VOLTAGE_REGISTER:
            ADI_CopyCommandBytesFilteredCellVoltageRegisters(&commandBytesToReadVoltageRegisters);
            break;
        case ADI_REDUNDANT_CELL_VOLTAGE_REGISTER:
            ADI_CopyCommandBytesRedundantCellVoltageRegisters(&commandBytesToReadVoltageRegisters);
            break;
        default:                  /* LCOV_EXCL_LINE */
            FAS_ASSERT(FAS_TRAP); /* LCOV_EXCL_LINE */
            break;                /* LCOV_EXCL_LINE */
    }
    ADI_ReadAndStoreVoltages(adiState, &commandBytesToReadVoltageRegisters, storeLocation);
}

extern uint16_t ADI_GetStoredVoltageIndex(uint16_t registerVoltageIndex) {
    FAS_ASSERT(registerVoltageIndex < ADI_MAX_SUPPORTED_CELLS);

    uint16_t storedVoltageIndex = 0u;
    for (uint8_t c = 0; c < registerVoltageIndex; c++) {
        if (adi_voltageInputsUsed[c] == 1u) {
            storedVoltageIndex++;
        }
    }
    return storedVoltageIndex;
}

extern void ADI_RestartContinuousCellVoltageMeasurements(ADI_STATE_s *adiState) {
    FAS_ASSERT(adiState != NULL_PTR);

    ADI_CopyCommandBits(adi_cmdAdcv, adi_command);
    /**
     *  SM_VCELL_RED: Cell Voltage Measurement Redundancy
     *  Set RD bit to enable redundant cell voltage measurements
     */
    ADI_WriteCommandConfigurationBits(adi_command, ADI_ADCV_RD_POS, ADI_ADCV_RD_LEN, 1u);
    ADI_WriteCommandConfigurationBits(adi_command, ADI_ADCV_CONT_POS, ADI_ADCV_CONT_LEN, 1u);
    ADI_WriteCommandConfigurationBits(adi_command, ADI_ADCV_DCP_POS, ADI_ADCV_DCP_LEN, 0u);
    /* Restart measurement: do not reset IIR filter to keep current filtered values */
    ADI_WriteCommandConfigurationBits(adi_command, ADI_ADCV_RSTF_POS, ADI_ADCV_RSTF_LEN, 0u);
    ADI_WriteCommandConfigurationBits(adi_command, ADI_ADCV_OW01_POS, ADI_ADCV_OW01_LEN, 0u);
    ADI_TransmitCommand(adi_command, adiState);
    ADI_Wait(ADI_MEASUREMENT_RESTART_WAIT_TIME_ms);
}

extern void ADI_StopContinuousCellVoltageMeasurements(ADI_STATE_s *adiState) {
    FAS_ASSERT(adiState != NULL_PTR);

    ADI_CopyCommandBits(adi_cmdAdcv, adi_command);
    /* Start one single-shot measurement without redundancy, after the measurements are stopped */
    ADI_WriteCommandConfigurationBits(adi_command, ADI_ADCV_RD_POS, ADI_ADCV_RD_LEN, 0u);
    ADI_WriteCommandConfigurationBits(adi_command, ADI_ADCV_CONT_POS, ADI_ADCV_CONT_LEN, 0u);
    ADI_WriteCommandConfigurationBits(adi_command, ADI_ADCV_DCP_POS, ADI_ADCV_DCP_LEN, 0u);
    ADI_WriteCommandConfigurationBits(adi_command, ADI_ADCV_RSTF_POS, ADI_ADCV_RSTF_LEN, 0u);
    ADI_WriteCommandConfigurationBits(adi_command, ADI_ADCV_OW01_POS, ADI_ADCV_OW01_LEN, 0u);
    ADI_TransmitCommand(adi_command, adiState);
    ADI_Wait(ADI_MEASUREMENT_STOP_WAIT_TIME_ms);
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST

extern void TEST_ADI_CopyCommandBytesCellVoltageRegister(
    ADI_COMMAND_READ_REGISTERS_s *commandBytesToReadVoltageRegisters) {
    ADI_CopyCommandBytesCellVoltageRegister(commandBytesToReadVoltageRegisters);
}

extern void TEST_ADI_CopyCommandBytesAverageCellVoltageRegisters(
    ADI_COMMAND_READ_REGISTERS_s *commandBytesToReadVoltageRegisters) {
    ADI_CopyCommandBytesAverageCellVoltageRegisters(commandBytesToReadVoltageRegisters);
}

extern void TEST_ADI_CopyCommandBytesFilteredCellVoltageRegisters(
    ADI_COMMAND_READ_REGISTERS_s *commandBytesToReadVoltageRegisters) {
    ADI_CopyCommandBytesFilteredCellVoltageRegisters(commandBytesToReadVoltageRegisters);
}

extern void TEST_ADI_CopyCommandBytesRedundantCellVoltageRegisters(
    ADI_COMMAND_READ_REGISTERS_s *commandBytesToReadVoltageRegisters) {
    ADI_CopyCommandBytesRedundantCellVoltageRegisters(commandBytesToReadVoltageRegisters);
}

extern void TEST_ADI_ReadAndStoreVoltages(
    ADI_STATE_s *adiState,
    ADI_COMMAND_READ_REGISTERS_s *commandBytesToReadVoltageRegisters,
    ADI_VOLTAGE_STORE_LOCATION_e storeLocation) {
    ADI_ReadAndStoreVoltages(adiState, commandBytesToReadVoltageRegisters, storeLocation);
}

extern void TEST_ADI_SaveRxToCellVoltageBuffer(
    ADI_STATE_s *adiState,
    uint8_t *data,
    uint8_t registerSet,
    ADI_VOLTAGE_STORE_LOCATION_e storeLocation) {
    ADI_SaveRxToCellVoltageBuffer(adiState, data, registerSet, storeLocation);
}
#endif
