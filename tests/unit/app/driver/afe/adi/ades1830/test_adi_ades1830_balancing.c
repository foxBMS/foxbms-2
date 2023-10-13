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
 * @file    test_adi_ades1830_balancing.c
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
#include "unity.h"
#include "Mockadi_ades183x.h"
#include "Mockadi_ades183x_helpers.h"
#include "Mockadi_ades183x_pec.h"
#include "Mockadi_ades183x_voltages.h"
#include "Mockdatabase.h"
#include "Mockfassert.h"
#include "Mockftask.h"
#include "Mockio.h"
#include "Mockos.h"
#include "Mockpex.h"
#include "Mockspi.h"
#include "Mockspi_cfg.h"
#include "Mocktask.h"
#include "Mocktsi.h"

#include "adi_ades183x_balancing.h"
#include "adi_ades183x_buffers.h"
#include "adi_ades183x_commands.h" /* use the real buffer configuration */
#include "adi_ades183x_defs.h"
#include "spi_cfg-helper.h"
#include "test_assert_helper.h"

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("adi_ades1830_balancing.c")
TEST_SOURCE_FILE("adi_ades1830_cfg.c")

TEST_INCLUDE_PATH("../../src/app/application/config")
TEST_INCLUDE_PATH("../../src/app/driver/afe/adi/ades1830")
TEST_INCLUDE_PATH("../../src/app/driver/afe/adi/common/ades183x")
TEST_INCLUDE_PATH("../../src/app/driver/afe/adi/common/ades183x/config")
TEST_INCLUDE_PATH("../../src/app/driver/afe/adi/common/ades183x/pec")
TEST_INCLUDE_PATH("../../src/app/driver/afe/api")
TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/dma")
TEST_INCLUDE_PATH("../../src/app/driver/io")
TEST_INCLUDE_PATH("../../src/app/driver/pex")
TEST_INCLUDE_PATH("../../src/app/driver/rtc")
TEST_INCLUDE_PATH("../../src/app/driver/spi")
TEST_INCLUDE_PATH("../../src/app/driver/ts/api")
TEST_INCLUDE_PATH("../../src/app/engine/database")
TEST_INCLUDE_PATH("../../src/app/engine/diag")
TEST_INCLUDE_PATH("../../src/app/task/config")
TEST_INCLUDE_PATH("../../src/app/task/ftask")

/*========== Definitions and Implementations for Unit Test ==================*/
static DATA_BLOCK_BALANCING_CONTROL_s adi_balancingControl = {.header.uniqueId = DATA_BLOCK_ID_BALANCING_CONTROL};

ADI_STATE_s adi_stateBase = {
    .data.balancingControl = &adi_balancingControl,
};

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/

/**
 * @brief   Testing ADI_DeactivateBalancing
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: NULL_PTR for adiState -> assert
 *          - Routine validation:
 *            - RT1/1: Function calls expected subroutines
 */
void testADI_DeactivateBalancing(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(ADI_DeactivateBalancing(NULL_PTR));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    for (uint16_t m = 0; m < BS_NR_OF_MODULES_PER_STRING; m++) {
        const uint16_t reverseModuleNumber = BS_NR_OF_MODULES_PER_STRING - m - 1u;
        ADI_WriteDataBits_Expect(
            &adi_configurationRegisterBgroup[adi_stateBase.currentString]
                                            [(reverseModuleNumber * ADI_WRCFGB_LEN) + ADI_REGISTER_OFFSET4],
            0u,
            ADI_CFGRB4_DCC_1_8_POS,
            ADI_CFGRB4_DCC_1_8_MASK);
        ADI_WriteDataBits_Expect(
            &adi_configurationRegisterBgroup[adi_stateBase.currentString]
                                            [(reverseModuleNumber * ADI_WRCFGB_LEN) + ADI_REGISTER_OFFSET5],
            0u,
            ADI_CFGRB5_DCC_9_16_POS,
            ADI_CFGRB5_DCC_9_16_MASK);
    }
    ADI_StoredConfigurationWriteToAfeGlobal_Expect(&adi_stateBase);

    ADI_CopyCommandBits_Expect(adi_cmdUnmute, adi_command);
    ADI_TransmitCommand_Expect(adi_command, &adi_stateBase);

    /* ======= RT1/1: call function under test */
    ADI_DeactivateBalancing(&adi_stateBase);

    /* ======= RT1/1: test output verification */
    /* nothing to be verified */
}

void testADI_DetermineBalancingRegisterConfiguration(void) {
    /* invalid pointer */
    TEST_ASSERT_FAIL_ASSERT(ADI_DetermineBalancingRegisterConfiguration(NULL_PTR));

    typedef enum {
        TEST_ADI_BALANCING_VALUE0,
        TEST_ADI_BALANCING_VALUE1,
        TEST_ADI_BALANCING_VALUE_E_MAX,
    } TEST_ADI_BALANCING_e;
    uint8_t data = 0u;
    /* Test for 0xAA and 0x55 balancing patterns */
    for (TEST_ADI_BALANCING_e i = TEST_ADI_BALANCING_VALUE0; i < TEST_ADI_BALANCING_VALUE_E_MAX; i++) {
        switch (i) {
            case TEST_ADI_BALANCING_VALUE0:
                data = 0xAA;
                break;
            case TEST_ADI_BALANCING_VALUE1:
                data = 0x55;
                break;
            default:
                break;
        }
        for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
            adi_stateBase.currentString = s;
            for (uint16_t m = 0; m < BS_NR_OF_MODULES_PER_STRING; m++) {
                for (uint8_t c = 0u; c < ADI_MAX_SUPPORTED_CELLS; c++) {
                    if (adi_voltageInputsUsed[c] == 1u) {
                        /* All inputs used: function returns the cell index given as parameter */
                        ADI_GetStoredVoltageIndex_IgnoreAndReturn(c);
                    }
                    /* Reset balancing control table */
                    adi_stateBase.data.balancingControl
                        ->balancingState[s][(m * (BS_NR_OF_CELL_BLOCKS_PER_MODULE)) + c] = 0u;
                    /* Set balancing control table values for 0xAA pattern */
                    if (data == 0xAAu) {
                        if (c % 2u != 0u) {
                            adi_stateBase.data.balancingControl
                                ->balancingState[s][(m * (BS_NR_OF_CELL_BLOCKS_PER_MODULE)) + c] = 1u;
                        }
                    } /* Set balancing control table values for 0x55 pattern */
                    if (data == 0x55u) {
                        if (c % 2u == 0u) {
                            adi_stateBase.data.balancingControl
                                ->balancingState[s][(m * (BS_NR_OF_CELL_BLOCKS_PER_MODULE)) + c] = 1u;
                        }
                    }
                }
                /* cell 1 to 8 bits must be set to the tested value */
                ADI_WriteDataBits_Expect(NULL_PTR, data, ADI_CFGRB4_DCC_1_8_POS, ADI_CFGRB4_DCC_1_8_MASK);
                ADI_WriteDataBits_IgnoreArg_pSentData();
                /* cell 9 to 16 bits must be set to the tested value */
                ADI_WriteDataBits_Expect(NULL_PTR, data, ADI_CFGRB5_DCC_9_16_POS, ADI_CFGRB5_DCC_9_16_MASK);
                ADI_WriteDataBits_IgnoreArg_pSentData();
            }

            ADI_StoredConfigurationWriteToAfeGlobal_Ignore();

            /* ======= RT1/1: call function under test */
            ADI_DetermineBalancingRegisterConfiguration(&adi_stateBase);
        }
    }
}
