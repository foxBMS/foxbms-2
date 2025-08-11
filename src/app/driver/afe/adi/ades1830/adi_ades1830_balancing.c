/**
 *
 * @copyright &copy; 2010 - 2025, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * - "This product uses parts of foxBMS&reg;"
 * - "This product includes parts of foxBMS&reg;"
 * - "This product is derived from foxBMS&reg;"
 *
 */

/**
 * @file    adi_ades1830_balancing.c
 * @author  foxBMS Team
 * @date    2019-08-27 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup DRIVERS
 * @prefix  ADI
 *
 * @brief   Implementation of helpers for the balancing functionality
 * @details TODO
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
extern void ADI_DeactivateBalancing(ADI_STATE_s *pAdiState) {
    FAS_ASSERT(pAdiState != NULL_PTR);

    /* Deactivate balancing before sending unmute command */
    for (uint16_t m = 0; m < BS_NR_OF_MODULES_PER_STRING; m++) {
        const uint16_t reverseModuleNumber = BS_NR_OF_MODULES_PER_STRING - m - 1u;
        ADI_WriteDataBits(
            &adi_configurationRegisterBgroup[pAdiState->currentString]
                                            [(reverseModuleNumber * ADI_WRCFGB_LEN) + ADI_REGISTER_OFFSET4],
            0u,
            ADI_CFGRB4_DCC_1_8_POS,
            ADI_CFGRB4_DCC_1_8_MASK);
        ADI_WriteDataBits(
            &adi_configurationRegisterBgroup[pAdiState->currentString]
                                            [(reverseModuleNumber * ADI_WRCFGB_LEN) + ADI_REGISTER_OFFSET5],
            0u,
            ADI_CFGRB5_DCC_9_16_POS,
            ADI_CFGRB5_DCC_9_16_MASK);
    }
    ADI_StoredConfigurationWriteToAfeGlobal(pAdiState);
    /* Send Unmute command */
    ADI_CopyCommandBytes(adi_cmdUnmute, adi_command);
    ADI_TransmitCommand(adi_command, pAdiState);
}

extern void ADI_DetermineBalancingRegisterConfiguration(ADI_STATE_s *pAdiState) {
    FAS_ASSERT(pAdiState != NULL_PTR);
    uint16_t storedVoltageIndex = 0u;

    for (uint16_t m = 0; m < BS_NR_OF_MODULES_PER_STRING; m++) {
        const uint16_t reverseModuleNumber = BS_NR_OF_MODULES_PER_STRING - m - 1u;
        uint8_t dccRegisterLow             = 0u;
        uint8_t dccRegisterHigh            = 0u;
        for (uint8_t c = 0u; c < ADI_MAX_SUPPORTED_CELLS; c++) {
            if (adi_voltageInputsUsed[c] == 1u) {
                storedVoltageIndex = ADI_GetStoredVoltageIndex(c);
                if (pAdiState->data.balancingControl
                        ->activateBalancing[pAdiState->currentString][m][storedVoltageIndex] == true) {
                    if (c < ADI_CFGRB_NUMBER_OF_DCC_BITS_PER_BYTE) { /* 0 - 7 cells */
                        dccRegisterLow |= (uint8_t)(1u << c);
                    } else { /* 8 - 15 cells */
                        dccRegisterHigh |= (uint8_t)(1u << (c - ADI_CFGRB_NUMBER_OF_DCC_BITS_PER_BYTE));
                    }
                }
            }
        }
        ADI_WriteDataBits(
            &adi_configurationRegisterBgroup[pAdiState->currentString]
                                            [(reverseModuleNumber * ADI_WRCFGB_LEN) + ADI_REGISTER_OFFSET4],
            dccRegisterLow,
            ADI_CFGRB4_DCC_1_8_POS,
            ADI_CFGRB4_DCC_1_8_MASK);
        ADI_WriteDataBits(
            &adi_configurationRegisterBgroup[pAdiState->currentString]
                                            [(reverseModuleNumber * ADI_WRCFGB_LEN) + ADI_REGISTER_OFFSET5],
            dccRegisterHigh,
            ADI_CFGRB5_DCC_9_16_POS,
            ADI_CFGRB5_DCC_9_16_MASK);
    }
    ADI_StoredConfigurationWriteToAfeGlobal(pAdiState);
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
