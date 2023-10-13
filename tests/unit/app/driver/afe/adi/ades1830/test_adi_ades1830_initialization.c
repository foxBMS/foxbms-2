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
 * @file    test_adi_ades1830_initialization.c
 * @author  foxBMS Team
 * @date    2022-12-07 (date of creation)
 * @updated 2023-10-12 (date of last update)
 * @version v1.6.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Test of some module
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockadi_ades183x_helpers.h"

#include "adi_ades183x_buffers.h"  /* use the real command config */
#include "adi_ades183x_commands.h" /* use the real buffer configuration */
#include "adi_ades183x_initialization.h"
#include "test_assert_helper.h"

#include <stdbool.h>
#include <stdint.h>

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("adi_ades183x_buffers.c")
TEST_SOURCE_FILE("adi_ades183x_initialization.c")

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
static ADI_ERROR_TABLE_s adi_errorTable = {0}; /*!< init in ADI_ResetErrorTable-function */

ADI_STATE_s adi_stateBase = {
    .data.errorTable = &adi_errorTable,
};

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/

/*========== Externalized Static Function Test Cases ========================*/
void testADI_ClearAllFlagsInStatusRegisterGroupC(void) {
    /* invalid pointer */
    TEST_ASSERT_FAIL_ASSERT(TEST_ADI_ClearAllFlagsInStatusRegisterGroupC(NULL_PTR));

    const uint8_t numberExpectedCalls = 4u * 8u;
    for (uint8_t i = 0; i < numberExpectedCalls; i++) {
        ADI_WriteDataBits_Ignore();
    }
    ADI_WriteRegisterGlobal_Ignore();

    TEST_ADI_ClearAllFlagsInStatusRegisterGroupC(&adi_stateBase);
}

void testADI_DisableBalancingOnStartup(void) {
    /* invalid pointer */
    TEST_ASSERT_FAIL_ASSERT(TEST_ADI_DisableBalancingOnStartup(NULL_PTR));

    ADI_WriteRegisterGlobal_Ignore();
    ADI_WriteRegisterGlobal_Ignore();
    TEST_ADI_DisableBalancingOnStartup(&adi_stateBase);
}

void testADI_GetSerialIdsOfAllIcsInString(void) {
    /* invalid pointer */
    TEST_ASSERT_FAIL_ASSERT(TEST_ADI_GetSerialIdsOfAllIcsInString(NULL_PTR));

    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        adi_stateBase.currentString = s;
        /* First test: buffer contains cleared values */
        /* Prepare voltage data */
        for (uint16_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            /* Set receive buffer with raw data */
            adi_dataReceive[0u + (m * ADI_MAX_REGISTER_SIZE_IN_BYTES)] = 0xAAu;
            adi_dataReceive[1u + (m * ADI_MAX_REGISTER_SIZE_IN_BYTES)] = 0xBBu;
            adi_dataReceive[2u + (m * ADI_MAX_REGISTER_SIZE_IN_BYTES)] = 0xCCu;
            adi_dataReceive[3u + (m * ADI_MAX_REGISTER_SIZE_IN_BYTES)] = 0xDDu;
            adi_dataReceive[4u + (m * ADI_MAX_REGISTER_SIZE_IN_BYTES)] = 0xEEu;
            adi_dataReceive[5u + (m * ADI_MAX_REGISTER_SIZE_IN_BYTES)] = 0xFFu;
            ADI_CopyCommandBits_Ignore();
            ADI_ReadRegister_Ignore();
            TEST_ADI_GetSerialIdsOfAllIcsInString(&adi_stateBase);
            /* Check that Serial ID was extracted correctly from raw data */
            TEST_ASSERT_EQUAL(0xFFEEDDCCBBAAu, adi_stateBase.serialId[s][m]);
        }
    }
}

void testADI_GetRevisionOfAllIcsInString(void) {
    /* invalid pointer */
    TEST_ASSERT_FAIL_ASSERT(TEST_ADI_GetRevisionOfAllIcsInString(NULL_PTR));

    ADI_CopyCommandBits_Ignore();
    ADI_ReadRegister_Ignore();
    for (uint16_t m = 0; m < ADI_N_ADI; m++) {
        ADI_ReadDataBits_Ignore();
    }
    TEST_ADI_GetRevisionOfAllIcsInString(&adi_stateBase);
}

void testADI_ResetIirFilterOnStartup(void) {
    /* invalid pointer */
    TEST_ASSERT_FAIL_ASSERT(TEST_ADI_ResetIirFilterOnStartup(NULL_PTR));

    ADI_StoredConfigurationFillRegisterDataGlobal_Ignore();
    ADI_StoredConfigurationWriteToAfe_IgnoreAndReturn(STD_OK);
    TEST_ADI_ResetIirFilterOnStartup(&adi_stateBase);
}
void testADI_StartContinuousCellVoltageMeasurements(void) {
    /* invalid pointer */
    TEST_ASSERT_FAIL_ASSERT(TEST_ADI_StartContinuousCellVoltageMeasurements(NULL_PTR));

    ADI_CopyCommandBits_Ignore();
    ADI_WriteCommandConfigurationBits_Ignore();
    ADI_WriteCommandConfigurationBits_Ignore();
    ADI_WriteCommandConfigurationBits_Ignore();
    ADI_WriteCommandConfigurationBits_Ignore();
    ADI_WriteCommandConfigurationBits_Ignore();
    ADI_TransmitCommand_Ignore();
    ADI_Wait_Ignore();
    TEST_ADI_StartContinuousCellVoltageMeasurements(&adi_stateBase);
}

void testADI_WakeUpDaisyChain(void) {
    /* invalid pointer */
    TEST_ASSERT_FAIL_ASSERT(TEST_ADI_WakeUpDaisyChain(NULL_PTR));

    ADI_SpiTransmitReceiveData_Ignore();
    ADI_Wait_Ignore();
    ADI_SpiTransmitReceiveData_Ignore();
    ADI_Wait_Ignore();
    TEST_ADI_WakeUpDaisyChain(&adi_stateBase);
}

void testADI_InitializeConfiguration(void) {
    /* invalid pointer */
    TEST_ASSERT_FAIL_ASSERT(TEST_ADI_InitializeConfiguration(NULL_PTR));

    /* Configuration register A */
    const uint8_t numberExpectedRegisterACalls = 12u;
    for (uint8_t i = 0; i < numberExpectedRegisterACalls; i++) {
        ADI_StoredConfigurationFillRegisterDataGlobal_Ignore();
    }
    /* Configuration register B */
    const uint8_t numberExpectedRegisterBCalls = 9u;
    for (uint8_t i = 0; i < numberExpectedRegisterBCalls; i++) {
        ADI_StoredConfigurationFillRegisterDataGlobal_Ignore();
    }

    TEST_ADI_InitializeConfiguration(&adi_stateBase);
}

void testADI_ResetErrorTable(void) {
    /* Set all error flags */
    for (uint8_t stringNumber = 0u; stringNumber < BS_NR_OF_STRINGS; stringNumber++) {
        adi_stateBase.data.errorTable->spiIsOk[stringNumber] = false;
        for (uint16_t m = 0; m < ADI_N_ADI; m++) {
            adi_stateBase.data.errorTable->crcIsOk[stringNumber][m]                                       = false;
            adi_stateBase.data.errorTable->commandCounterIsOk[stringNumber][m]                            = false;
            adi_stateBase.data.errorTable->configurationAIsOk[stringNumber][m]                            = false;
            adi_stateBase.data.errorTable->configurationBIsOk[stringNumber][m]                            = false;
            adi_stateBase.data.errorTable->voltageRegisterContentIsNotStuck[stringNumber][m]              = false;
            adi_stateBase.data.errorTable->auxiliaryRegisterContentIsNotStuck[stringNumber][m]            = false;
            adi_stateBase.data.errorTable->adcComparisonIsOk[stringNumber][m]                             = false;
            adi_stateBase.data.errorTable->compFlagIsCleared[stringNumber][m]                             = false;
            adi_stateBase.data.errorTable->spiFltFlagIsCleared[stringNumber][m]                           = false;
            adi_stateBase.data.errorTable->filteredVoltageRangeIsOk[stringNumber][m]                      = false;
            adi_stateBase.data.errorTable->evenWireIsOk[stringNumber][m]                                  = false;
            adi_stateBase.data.errorTable->oddWireIsOk[stringNumber][m]                                   = false;
            adi_stateBase.data.errorTable->evenWireHasNoLatentFault[stringNumber][m]                      = false;
            adi_stateBase.data.errorTable->oddWireHasNoLatentFault[stringNumber][m]                       = false;
            adi_stateBase.data.errorTable->adcCompFlagsHaveNoLatentFault[stringNumber][m]                 = false;
            adi_stateBase.data.errorTable->auxComparisonIsOk[stringNumber][m]                             = false;
            adi_stateBase.data.errorTable->auxVoltageRangeIsOk[stringNumber][m]                           = false;
            adi_stateBase.data.errorTable->analogSupplyOvervoltageFlagIsCleared[stringNumber][m]          = false;
            adi_stateBase.data.errorTable->analogSupplyUndervoltageFlagIsCleared[stringNumber][m]         = false;
            adi_stateBase.data.errorTable->digitalSupplyOvervoltageFlagIsCleared[stringNumber][m]         = false;
            adi_stateBase.data.errorTable->digitalSupplyUndervoltageFlagIsCleared[stringNumber][m]        = false;
            adi_stateBase.data.errorTable->vdeFlagIsCleared[stringNumber][m]                              = false;
            adi_stateBase.data.errorTable->oscchkFlagIsCleared[stringNumber][m]                           = false;
            adi_stateBase.data.errorTable->analogSupplyOvervoltageFlagHasNoLatentFault[stringNumber][m]   = false;
            adi_stateBase.data.errorTable->analogSupplyUndervoltageFlagHasNoLatentFault[stringNumber][m]  = false;
            adi_stateBase.data.errorTable->digitalSupplyOvervoltageFlagHasNoLatentFault[stringNumber][m]  = false;
            adi_stateBase.data.errorTable->digitalSupplyUndervoltageFlagHasNoLatentFault[stringNumber][m] = false;
            adi_stateBase.data.errorTable->vdeFlagHasNoLatentFault[stringNumber][m]                       = false;
            adi_stateBase.data.errorTable->vdelFlagHasNoLatentFault[stringNumber][m]                      = false;
            adi_stateBase.data.errorTable->analogSupplyRangeIsOk[stringNumber][m]                         = false;
            adi_stateBase.data.errorTable->digitalSupplyRangeIsOk[stringNumber][m]                        = false;
            adi_stateBase.data.errorTable->vresRangeIsOk[stringNumber][m]                                 = false;
            adi_stateBase.data.errorTable->dieTemperatureIsOk[stringNumber][m]                            = false;
            adi_stateBase.data.errorTable->oscchkFlagHasNoLatentFault[stringNumber][m]                    = false;
            adi_stateBase.data.errorTable->ocCounterFasterFlagHasNoLatentFault[stringNumber][m]           = false;
            adi_stateBase.data.errorTable->ocCounterSlowerFlagHasNoLatentFault[stringNumber][m]           = false;
            adi_stateBase.data.errorTable->thsdFlagHasNoLatentFault[stringNumber][m]                      = false;
            adi_stateBase.data.errorTable->tmodchkFlagHasNoLatentFault[stringNumber][m]                   = false;
            adi_stateBase.data.errorTable->tmodchkFlagIsCleared[stringNumber][m]                          = false;
            adi_stateBase.data.errorTable->cmedFlagIsCleared[stringNumber][m]                             = false;
            adi_stateBase.data.errorTable->smedFlagIsCleared[stringNumber][m]                             = false;
            adi_stateBase.data.errorTable->sleepFlagIsCleared[stringNumber][m]                            = false;
            adi_stateBase.data.errorTable->cmedFlagHasNoLatentFault[stringNumber][m]                      = false;
            adi_stateBase.data.errorTable->smedFlagHasNoLatentFault[stringNumber][m]                      = false;
            adi_stateBase.data.errorTable->initResetAuxValuesAreOk[stringNumber][m]                       = false;
            adi_stateBase.data.errorTable->initClearedAuxValuesAreOk[stringNumber][m]                     = false;
        }
    }
    /* Reset error flags */
    TEST_ADI_ResetErrorTable(&adi_stateBase);
    /* Check that error flags have been reset */
    for (uint8_t stringNumber = 0u; stringNumber < BS_NR_OF_STRINGS; stringNumber++) {
        TEST_ASSERT_EQUAL(true, adi_stateBase.data.errorTable->spiIsOk[stringNumber]);
        for (uint16_t m = 0; m < ADI_N_ADI; m++) {
            TEST_ASSERT_EQUAL(true, adi_stateBase.data.errorTable->crcIsOk[stringNumber][m]);
            TEST_ASSERT_EQUAL(true, adi_stateBase.data.errorTable->commandCounterIsOk[stringNumber][m]);
            TEST_ASSERT_EQUAL(true, adi_stateBase.data.errorTable->configurationAIsOk[stringNumber][m]);
            TEST_ASSERT_EQUAL(true, adi_stateBase.data.errorTable->configurationBIsOk[stringNumber][m]);
            TEST_ASSERT_EQUAL(true, adi_stateBase.data.errorTable->voltageRegisterContentIsNotStuck[stringNumber][m]);
            TEST_ASSERT_EQUAL(true, adi_stateBase.data.errorTable->auxiliaryRegisterContentIsNotStuck[stringNumber][m]);
            TEST_ASSERT_EQUAL(true, adi_stateBase.data.errorTable->adcComparisonIsOk[stringNumber][m]);
            TEST_ASSERT_EQUAL(true, adi_stateBase.data.errorTable->compFlagIsCleared[stringNumber][m]);
            TEST_ASSERT_EQUAL(true, adi_stateBase.data.errorTable->spiFltFlagIsCleared[stringNumber][m]);
            TEST_ASSERT_EQUAL(true, adi_stateBase.data.errorTable->filteredVoltageRangeIsOk[stringNumber][m]);
            TEST_ASSERT_EQUAL(true, adi_stateBase.data.errorTable->evenWireIsOk[stringNumber][m]);
            TEST_ASSERT_EQUAL(true, adi_stateBase.data.errorTable->oddWireIsOk[stringNumber][m]);
            TEST_ASSERT_EQUAL(true, adi_stateBase.data.errorTable->evenWireHasNoLatentFault[stringNumber][m]);
            TEST_ASSERT_EQUAL(true, adi_stateBase.data.errorTable->oddWireHasNoLatentFault[stringNumber][m]);
            TEST_ASSERT_EQUAL(true, adi_stateBase.data.errorTable->adcCompFlagsHaveNoLatentFault[stringNumber][m]);
            TEST_ASSERT_EQUAL(true, adi_stateBase.data.errorTable->auxComparisonIsOk[stringNumber][m]);
            TEST_ASSERT_EQUAL(true, adi_stateBase.data.errorTable->auxVoltageRangeIsOk[stringNumber][m]);
            TEST_ASSERT_EQUAL(
                true, adi_stateBase.data.errorTable->analogSupplyOvervoltageFlagIsCleared[stringNumber][m]);
            TEST_ASSERT_EQUAL(
                true, adi_stateBase.data.errorTable->analogSupplyUndervoltageFlagIsCleared[stringNumber][m]);
            TEST_ASSERT_EQUAL(
                true, adi_stateBase.data.errorTable->digitalSupplyOvervoltageFlagIsCleared[stringNumber][m]);
            TEST_ASSERT_EQUAL(
                true, adi_stateBase.data.errorTable->digitalSupplyUndervoltageFlagIsCleared[stringNumber][m]);
            TEST_ASSERT_EQUAL(true, adi_stateBase.data.errorTable->vdeFlagIsCleared[stringNumber][m]);
            TEST_ASSERT_EQUAL(true, adi_stateBase.data.errorTable->oscchkFlagIsCleared[stringNumber][m]);
            TEST_ASSERT_EQUAL(
                true, adi_stateBase.data.errorTable->analogSupplyOvervoltageFlagHasNoLatentFault[stringNumber][m]);
            TEST_ASSERT_EQUAL(
                true, adi_stateBase.data.errorTable->analogSupplyUndervoltageFlagHasNoLatentFault[stringNumber][m]);
            TEST_ASSERT_EQUAL(
                true, adi_stateBase.data.errorTable->digitalSupplyOvervoltageFlagHasNoLatentFault[stringNumber][m]);
            TEST_ASSERT_EQUAL(
                true, adi_stateBase.data.errorTable->digitalSupplyUndervoltageFlagHasNoLatentFault[stringNumber][m]);
            TEST_ASSERT_EQUAL(true, adi_stateBase.data.errorTable->vdeFlagHasNoLatentFault[stringNumber][m]);
            TEST_ASSERT_EQUAL(true, adi_stateBase.data.errorTable->vdelFlagHasNoLatentFault[stringNumber][m]);
            TEST_ASSERT_EQUAL(true, adi_stateBase.data.errorTable->analogSupplyRangeIsOk[stringNumber][m]);
            TEST_ASSERT_EQUAL(true, adi_stateBase.data.errorTable->digitalSupplyRangeIsOk[stringNumber][m]);
            TEST_ASSERT_EQUAL(true, adi_stateBase.data.errorTable->vresRangeIsOk[stringNumber][m]);
            TEST_ASSERT_EQUAL(true, adi_stateBase.data.errorTable->dieTemperatureIsOk[stringNumber][m]);
            TEST_ASSERT_EQUAL(true, adi_stateBase.data.errorTable->oscchkFlagHasNoLatentFault[stringNumber][m]);
            TEST_ASSERT_EQUAL(
                true, adi_stateBase.data.errorTable->ocCounterFasterFlagHasNoLatentFault[stringNumber][m]);
            TEST_ASSERT_EQUAL(
                true, adi_stateBase.data.errorTable->ocCounterSlowerFlagHasNoLatentFault[stringNumber][m]);
            TEST_ASSERT_EQUAL(true, adi_stateBase.data.errorTable->thsdFlagHasNoLatentFault[stringNumber][m]);
            TEST_ASSERT_EQUAL(true, adi_stateBase.data.errorTable->tmodchkFlagHasNoLatentFault[stringNumber][m]);
            TEST_ASSERT_EQUAL(true, adi_stateBase.data.errorTable->tmodchkFlagIsCleared[stringNumber][m]);
            TEST_ASSERT_EQUAL(true, adi_stateBase.data.errorTable->cmedFlagIsCleared[stringNumber][m]);
            TEST_ASSERT_EQUAL(true, adi_stateBase.data.errorTable->smedFlagIsCleared[stringNumber][m]);
            TEST_ASSERT_EQUAL(true, adi_stateBase.data.errorTable->sleepFlagIsCleared[stringNumber][m]);
            TEST_ASSERT_EQUAL(true, adi_stateBase.data.errorTable->cmedFlagHasNoLatentFault[stringNumber][m]);
            TEST_ASSERT_EQUAL(true, adi_stateBase.data.errorTable->smedFlagHasNoLatentFault[stringNumber][m]);
            TEST_ASSERT_EQUAL(true, adi_stateBase.data.errorTable->initResetAuxValuesAreOk[stringNumber][m]);
            TEST_ASSERT_EQUAL(true, adi_stateBase.data.errorTable->initClearedAuxValuesAreOk[stringNumber][m]);
        }
    }
}

/*========== Extern Function Test Cases =====================================*/

void testADI_InitializeMeasurement(void) {
    /* invalid pointer */
    TEST_ASSERT_FAIL_ASSERT(ADI_InitializeMeasurement(NULL_PTR));

    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        /* real test */
        /* first block */
        for (uint8_t i = 0; i < 2u; i++) {
            ADI_SpiTransmitReceiveData_Ignore();
            ADI_Wait_Ignore();
        }
        for (uint16_t i = 0u; i < ADI_N_ADI; i++) {
            adi_stateBase.data.errorTable->commandCounterIsOk[adi_stateBase.currentString][i] = true;
            adi_stateBase.data.commandCounter[adi_stateBase.currentString][i] = ADI_COMMAND_COUNTER_RESET_VALUE;
        }
        ADI_ClearCommandCounter_Expect(&adi_stateBase);
        ADI_ClearCommandCounter_ReturnThruPtr_adiState(&adi_stateBase);

        ADI_StoredConfigurationFillRegisterDataGlobal_Ignore();
        ADI_StoredConfigurationWriteToAfeGlobal_Ignore();

        ADI_Wait_Ignore();

        ADI_CopyCommandBits_Ignore();
        ADI_TransmitCommand_Ignore();
        for (uint8_t i = 0; i < 2u; i++) {
            ADI_CopyCommandBits_Ignore();
            ADI_ReadRegister_Ignore();
        }

        ADI_WriteRegisterGlobal_Ignore();
        ADI_WriteRegisterGlobal_Ignore();
        ADI_StoredConfigurationFillRegisterDataGlobal_Ignore();
        ADI_StoredConfigurationWriteToAfe_ExpectAndReturn(ADI_CFG_REGISTER_SET_A, &adi_stateBase, STD_OK);
        for (uint8_t i = 0; i < 4u; i++) {
            for (uint8_t j = 0; j < 8u; j++) {
                ADI_WriteDataBits_Ignore();
            }
        }
        ADI_WriteRegisterGlobal_Ignore();
        ADI_CopyCommandBits_Ignore();
        ADI_ReadRegister_Ignore();

        for (uint16_t m = 0u; m < ADI_N_ADI; m++) {
            uint8_t statusData = adi_dataReceive[(m * ADI_RDSTATC_LEN) + ADI_REGISTER_OFFSET5];
            uint8_t flagComp   = 0u;
            if (flagComp != 1u) {
                adi_stateBase.data.errorTable->compFlagIsCleared[adi_stateBase.currentString][m] = false;
            }
            ADI_ReadDataBits_Expect(statusData, &flagComp, ADI_STCR5_COMP_POS, ADI_STCR5_COMP_MASK);
            ADI_ReadDataBits_ReturnArrayThruPtr_pDataToRead(&flagComp, 1);
        }

        /* Start continuous cell voltage measurements */
        ADI_CopyCommandBits_Ignore();
        ADI_WriteCommandConfigurationBits_Expect(adi_command, ADI_ADCV_RD_POS, ADI_ADCV_RD_LEN, 1u);
        ADI_WriteCommandConfigurationBits_Expect(adi_command, ADI_ADCV_CONT_POS, ADI_ADCV_CONT_LEN, 1u);
        ADI_WriteCommandConfigurationBits_Expect(adi_command, ADI_ADCV_DCP_POS, ADI_ADCV_DCP_LEN, 0u);
        ADI_WriteCommandConfigurationBits_Expect(adi_command, ADI_ADCV_RSTF_POS, ADI_ADCV_RSTF_LEN, 1u);
        ADI_WriteCommandConfigurationBits_Expect(adi_command, ADI_ADCV_OW01_POS, ADI_ADCV_OW01_LEN, 0u);
        ADI_TransmitCommand_Ignore();
        ADI_Wait_Ignore();

        /* Read serial ID */
        ADI_CopyCommandBits_Ignore();
        ADI_ReadRegister_Ignore();

        /* Read revision */
        ADI_CopyCommandBits_Ignore();
        ADI_ReadRegister_Ignore();
        for (uint16_t m = 0; m < ADI_N_ADI; m++) {
            ADI_ReadDataBits_Expect(
                adi_dataReceive[(m * ADI_RDSTATE_LEN) + ADI_REGISTER_OFFSET5],
                &(adi_stateBase.revision[adi_stateBase.currentString][m]),
                ADI_STER5_REV_0_3_POS,
                ADI_STER5_REV_0_3_MASK);
        }
    }
    ADI_InitializeMeasurement(&adi_stateBase);
}
