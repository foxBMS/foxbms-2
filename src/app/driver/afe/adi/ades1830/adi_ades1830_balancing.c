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
 * @file    adi_ades1830_balancing.c
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
#include "adi_ades183x_balancing.h"
/* clang-format on */

#include "adi_ades183x_cfg.h"

#include "adi_ades183x_buffers.h"
#include "adi_ades183x_commands.h"
#include "adi_ades183x_defs.h"
#include "adi_ades183x_helpers.h"
#include "adi_ades183x_voltages.h"

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/
extern void ADI_DeactivateBalancing(ADI_STATE_s *adiState) {
    FAS_ASSERT(adiState != NULL_PTR);

    /* Deactivate balancing before sending unmute command */
    for (uint16_t m = 0; m < BS_NR_OF_MODULES_PER_STRING; m++) {
        const uint16_t reverseModuleNumber = BS_NR_OF_MODULES_PER_STRING - m - 1u;
        ADI_WriteDataBits(
            &adi_configurationRegisterBgroup[adiState->currentString]
                                            [(reverseModuleNumber * ADI_WRCFGB_LEN) + ADI_REGISTER_OFFSET4],
            0u,
            ADI_CFGRB4_DCC_1_8_POS,
            ADI_CFGRB4_DCC_1_8_MASK);
        ADI_WriteDataBits(
            &adi_configurationRegisterBgroup[adiState->currentString]
                                            [(reverseModuleNumber * ADI_WRCFGB_LEN) + ADI_REGISTER_OFFSET5],
            0u,
            ADI_CFGRB5_DCC_9_16_POS,
            ADI_CFGRB5_DCC_9_16_MASK);
    }
    ADI_StoredConfigurationWriteToAfeGlobal(adiState);
    /* Send Unmute command */
    ADI_CopyCommandBits(adi_cmdUnmute, adi_command);
    ADI_TransmitCommand(adi_command, adiState);
}

extern void ADI_DetermineBalancingRegisterConfiguration(ADI_STATE_s *adiState) {
    FAS_ASSERT(adiState != NULL_PTR);
    uint16_t storedVoltageIndex = 0u;

    for (uint16_t m = 0; m < BS_NR_OF_MODULES_PER_STRING; m++) {
        const uint16_t reverseModuleNumber = BS_NR_OF_MODULES_PER_STRING - m - 1u;
        uint8_t dccRegisterLow             = 0u;
        uint8_t dccRegisterHigh            = 0u;
        for (uint8_t c = 0u; c < ADI_MAX_SUPPORTED_CELLS; c++) {
            if (adi_voltageInputsUsed[c] == 1u) {
                storedVoltageIndex = ADI_GetStoredVoltageIndex(c);
                if (adiState->data.balancingControl
                        ->balancingState[adiState->currentString]
                                        [(m * (BS_NR_OF_CELL_BLOCKS_PER_MODULE)) + storedVoltageIndex] == 1u) {
                    if (c < ADI_CFGRB_NUMBER_OF_DCC_BITS_PER_BYTE) { /* 0 - 7 cells */
                        dccRegisterLow |= (uint8_t)(1u << c);
                    } else { /* 8 - 15 cells */
                        dccRegisterHigh |= (uint8_t)(1u << (c - ADI_CFGRB_NUMBER_OF_DCC_BITS_PER_BYTE));
                    }
                }
            }
        }
        ADI_WriteDataBits(
            &adi_configurationRegisterBgroup[adiState->currentString]
                                            [(reverseModuleNumber * ADI_WRCFGB_LEN) + ADI_REGISTER_OFFSET4],
            dccRegisterLow,
            ADI_CFGRB4_DCC_1_8_POS,
            ADI_CFGRB4_DCC_1_8_MASK);
        ADI_WriteDataBits(
            &adi_configurationRegisterBgroup[adiState->currentString]
                                            [(reverseModuleNumber * ADI_WRCFGB_LEN) + ADI_REGISTER_OFFSET5],
            dccRegisterHigh,
            ADI_CFGRB5_DCC_9_16_POS,
            ADI_CFGRB5_DCC_9_16_MASK);
    }
    ADI_StoredConfigurationWriteToAfeGlobal(adiState);
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
