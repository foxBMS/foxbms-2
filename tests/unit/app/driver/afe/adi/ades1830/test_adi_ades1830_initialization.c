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
 * @file    test_adi_ades1830_initialization.c
 * @author  foxBMS Team
 * @date    2022-12-07 (date of creation)
 * @updated 2025-03-31 (date of last update)
 * @version v1.9.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Test of some module
 * @details TODO
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
/* Wait at least one micro second for the AFE to wakeup */
#define ADI_AFE_WAKEUP_TIME     ((ADI_DAISY_CHAIN_WAKE_UP_TIME_us / ADI_COEFFICIENT_US_TO_MS) + 1u)
#define ADI_ISO_SPI_WAKEUP_TIME ((ADI_DAISY_CHAIN_READY_TIME_us / ADI_COEFFICIENT_US_TO_MS) + 1u)

static ADI_ERROR_TABLE_s adi_errorTable = {0}; /*!< init in ADI_ResetErrorTable-function */

ADI_STATE_s adi_stateBase = {
    .data.errorTable = &adi_errorTable,
};

void ADI_ClearAllFlagsInStatusRegisterGroupC_Expects(void) {
    adi_clearFlagData[ADI_REGISTER_OFFSET0] = 0u;
    ADI_WriteDataBits_Expect(&adi_clearFlagData[ADI_REGISTER_OFFSET0], 1u, ADI_STCR0_CS1FLT_POS, ADI_STCR0_CS1FLT_MASK);
    ADI_WriteDataBits_Expect(&adi_clearFlagData[ADI_REGISTER_OFFSET0], 1u, ADI_STCR0_CS2FLT_POS, ADI_STCR0_CS2FLT_MASK);
    ADI_WriteDataBits_Expect(&adi_clearFlagData[ADI_REGISTER_OFFSET0], 1u, ADI_STCR0_CS3FLT_POS, ADI_STCR0_CS3FLT_MASK);
    ADI_WriteDataBits_Expect(&adi_clearFlagData[ADI_REGISTER_OFFSET0], 1u, ADI_STCR0_CS4FLT_POS, ADI_STCR0_CS4FLT_MASK);
    ADI_WriteDataBits_Expect(&adi_clearFlagData[ADI_REGISTER_OFFSET0], 1u, ADI_STCR0_CS5FLT_POS, ADI_STCR0_CS5FLT_MASK);
    ADI_WriteDataBits_Expect(&adi_clearFlagData[ADI_REGISTER_OFFSET0], 1u, ADI_STCR0_CS6FLT_POS, ADI_STCR0_CS6FLT_MASK);
    ADI_WriteDataBits_Expect(&adi_clearFlagData[ADI_REGISTER_OFFSET0], 1u, ADI_STCR0_CS7FLT_POS, ADI_STCR0_CS7FLT_MASK);
    ADI_WriteDataBits_Expect(&adi_clearFlagData[ADI_REGISTER_OFFSET0], 1u, ADI_STCR0_CS8FLT_POS, ADI_STCR0_CS8FLT_MASK);
    adi_clearFlagData[ADI_REGISTER_OFFSET1] = 0u;
    ADI_WriteDataBits_Expect(&adi_clearFlagData[ADI_REGISTER_OFFSET1], 1u, ADI_STCR1_CS9FLT_POS, ADI_STCR1_CS9FLT_MASK);
    ADI_WriteDataBits_Expect(
        &adi_clearFlagData[ADI_REGISTER_OFFSET1], 1u, ADI_STCR1_CS10FLT_POS, ADI_STCR1_CS10FLT_MASK);
    ADI_WriteDataBits_Expect(
        &adi_clearFlagData[ADI_REGISTER_OFFSET1], 1u, ADI_STCR1_CS11FLT_POS, ADI_STCR1_CS11FLT_MASK);
    ADI_WriteDataBits_Expect(
        &adi_clearFlagData[ADI_REGISTER_OFFSET1], 1u, ADI_STCR1_CS12FLT_POS, ADI_STCR1_CS12FLT_MASK);
    ADI_WriteDataBits_Expect(
        &adi_clearFlagData[ADI_REGISTER_OFFSET1], 1u, ADI_STCR1_CS13FLT_POS, ADI_STCR1_CS13FLT_MASK);
    ADI_WriteDataBits_Expect(
        &adi_clearFlagData[ADI_REGISTER_OFFSET1], 1u, ADI_STCR1_CS14FLT_POS, ADI_STCR1_CS14FLT_MASK);
    ADI_WriteDataBits_Expect(
        &adi_clearFlagData[ADI_REGISTER_OFFSET1], 1u, ADI_STCR1_CS15FLT_POS, ADI_STCR1_CS15FLT_MASK);
    ADI_WriteDataBits_Expect(
        &adi_clearFlagData[ADI_REGISTER_OFFSET1], 1u, ADI_STCR1_CS16FLT_POS, ADI_STCR1_CS16FLT_MASK);
    adi_clearFlagData[ADI_REGISTER_OFFSET4] = 0u;
    ADI_WriteDataBits_Expect(&adi_clearFlagData[ADI_REGISTER_OFFSET4], 1u, ADI_STCR4_SMED_POS, ADI_STCR4_SMED_MASK);
    ADI_WriteDataBits_Expect(&adi_clearFlagData[ADI_REGISTER_OFFSET4], 1u, ADI_STCR4_SED_POS, ADI_STCR4_SED_MASK);
    ADI_WriteDataBits_Expect(&adi_clearFlagData[ADI_REGISTER_OFFSET4], 1u, ADI_STCR4_CMED_POS, ADI_STCR4_CMED_MASK);
    ADI_WriteDataBits_Expect(&adi_clearFlagData[ADI_REGISTER_OFFSET4], 1u, ADI_STCR4_CED_POS, ADI_STCR4_CED_MASK);
    ADI_WriteDataBits_Expect(&adi_clearFlagData[ADI_REGISTER_OFFSET4], 1u, ADI_STCR4_VD_UV_POS, ADI_STCR4_VD_UV_MASK);
    ADI_WriteDataBits_Expect(&adi_clearFlagData[ADI_REGISTER_OFFSET4], 1u, ADI_STCR4_VD_OV_POS, ADI_STCR4_VD_OV_MASK);
    ADI_WriteDataBits_Expect(&adi_clearFlagData[ADI_REGISTER_OFFSET4], 1u, ADI_STCR4_VA_UV_POS, ADI_STCR4_VA_UV_MASK);
    ADI_WriteDataBits_Expect(&adi_clearFlagData[ADI_REGISTER_OFFSET4], 1u, ADI_STCR4_VA_OV_POS, ADI_STCR4_VA_OV_MASK);
    adi_clearFlagData[ADI_REGISTER_OFFSET5] = 0u;
    ADI_WriteDataBits_Expect(&adi_clearFlagData[ADI_REGISTER_OFFSET5], 1u, ADI_STCR5_OSCCHK_POS, ADI_STCR5_OSCCHK_MASK);
    ADI_WriteDataBits_Expect(
        &adi_clearFlagData[ADI_REGISTER_OFFSET5], 1u, ADI_STCR5_TMODCHK_POS, ADI_STCR5_TMODCHK_MASK);
    ADI_WriteDataBits_Expect(&adi_clearFlagData[ADI_REGISTER_OFFSET5], 1u, ADI_STCR5_THSD_POS, ADI_STCR5_THSD_MASK);
    ADI_WriteDataBits_Expect(&adi_clearFlagData[ADI_REGISTER_OFFSET5], 1u, ADI_STCR5_SLEEP_POS, ADI_STCR5_SLEEP_MASK);
    ADI_WriteDataBits_Expect(&adi_clearFlagData[ADI_REGISTER_OFFSET5], 1u, ADI_STCR5_SPIFLT_POS, ADI_STCR5_SPIFLT_MASK);
    ADI_WriteDataBits_Expect(&adi_clearFlagData[ADI_REGISTER_OFFSET5], 1u, ADI_STCR5_COMP_POS, ADI_STCR5_COMP_MASK);
    ADI_WriteDataBits_Expect(&adi_clearFlagData[ADI_REGISTER_OFFSET5], 1u, ADI_STCR5_VDE_POS, ADI_STCR5_VDE_MASK);
    ADI_WriteDataBits_Expect(&adi_clearFlagData[ADI_REGISTER_OFFSET5], 1u, ADI_STCR5_VDEL_POS, ADI_STCR5_VDEL_MASK);
    ADI_WriteRegisterGlobal_Expect(adi_cmdClrflag, adi_clearFlagData, ADI_PEC_NO_FAULT_INJECTION, &adi_stateBase);
}

void ADI_StartContinuousCellVoltageMeasurements_Expects(void) {
    /* Mute balancing before starting cell voltage measurements */
    ADI_CopyCommandBits_Expect(adi_cmdMute, adi_command);
    ADI_TransmitCommand_Expect(adi_command, &adi_stateBase);

    /* SM_VCELL_RED */
    ADI_CopyCommandBits_Expect(adi_cmdAdcv, adi_command);
    ADI_WriteCommandConfigurationBits_Expect(adi_command, ADI_ADCV_RD_POS, ADI_ADCV_RD_LEN, 1u);
    ADI_WriteCommandConfigurationBits_Expect(adi_command, ADI_ADCV_CONT_POS, ADI_ADCV_CONT_LEN, 1u);
    ADI_WriteCommandConfigurationBits_Expect(adi_command, ADI_ADCV_DCP_POS, ADI_ADCV_DCP_LEN, 0u);
    ADI_WriteCommandConfigurationBits_Expect(adi_command, ADI_ADCV_RSTF_POS, ADI_ADCV_RSTF_LEN, 1u);
    ADI_WriteCommandConfigurationBits_Expect(adi_command, ADI_ADCV_OW01_POS, ADI_ADCV_OW01_LEN, 0u);
    ADI_TransmitCommand_Expect(adi_command, &adi_stateBase);
    ADI_Wait_Expect(ADI_IIR_SETTLING_TIME_ms);
}

void ADI_InitializeConfiguration_Expects(void) {
    /* Configuration register A */
    /* CFGRA0 */
    ADI_StoredConfigurationFillRegisterDataGlobal_Expect(
        ADI_CFG_REGISTER_SET_A,
        ADI_REGISTER_OFFSET0,
        ADI_DEFAULT_CTH_COMPARISON_THRESHOLD,
        ADI_CFGRA0_CTH_0_2_POS,
        ADI_CFGRA0_CTH_0_2_MASK,
        &adi_stateBase);
    ADI_StoredConfigurationFillRegisterDataGlobal_Expect(
        ADI_CFG_REGISTER_SET_A,
        ADI_REGISTER_OFFSET0,
        ADI_DEFAULT_REFON_SETUP,
        ADI_CFGRA0_REFON_POS,
        ADI_CFGRA0_REFON_MASK,
        &adi_stateBase);
    /* CFGRA1 */
    ADI_StoredConfigurationFillRegisterDataGlobal_Expect(
        ADI_CFG_REGISTER_SET_A,
        ADI_REGISTER_OFFSET1,
        ADI_DEFAULT_FLAG_D_SETUP,
        ADI_CFGRA1_FLAG_D_0_7_POS,
        ADI_CFGRA1_FLAG_D_0_7_MASK,
        &adi_stateBase);
    /* CFGRA2 */
    ADI_StoredConfigurationFillRegisterDataGlobal_Expect(
        ADI_CFG_REGISTER_SET_A,
        ADI_REGISTER_OFFSET2,
        ADI_DEFAULT_OWA_SETUP,
        ADI_CFGRA2_OWA_0_2_POS,
        ADI_CFGRA2_OWA_0_2_MASK,
        &adi_stateBase);
    ADI_StoredConfigurationFillRegisterDataGlobal_Expect(
        ADI_CFG_REGISTER_SET_A,
        ADI_REGISTER_OFFSET2,
        ADI_DEFAULT_OWRNG_SETUP,
        ADI_CFGRA2_OWRNG_POS,
        ADI_CFGRA2_OWRNG_MASK,
        &adi_stateBase);
    ADI_StoredConfigurationFillRegisterDataGlobal_Expect(
        ADI_CFG_REGISTER_SET_A,
        ADI_REGISTER_OFFSET2,
        ADI_DEFAULT_SOAKON_SETUP,
        ADI_CFGRA2_SOAKON_POS,
        ADI_CFGRA2_SOAKON_MASK,
        &adi_stateBase);
    /* CFGRA3 */
    ADI_StoredConfigurationFillRegisterDataGlobal_Expect(
        ADI_CFG_REGISTER_SET_A,
        ADI_REGISTER_OFFSET3,
        ADI_DEFAULT_GPO_1_8_SETUP,
        ADI_CFGRA3_GPO_1_8_POS,
        ADI_CFGRA3_GPO_1_8_MASK,
        &adi_stateBase);
    /* CFGRA4 */
    ADI_StoredConfigurationFillRegisterDataGlobal_Expect(
        ADI_CFG_REGISTER_SET_A,
        ADI_REGISTER_OFFSET4,
        ADI_DEFAULT_GPO_9_10_SETUP,
        ADI_CFGRA4_GPO_9_10_POS,
        ADI_CFGRA4_GPO_9_10_MASK,
        &adi_stateBase);
    /* CFGRA5 */
    ADI_StoredConfigurationFillRegisterDataGlobal_Expect(
        ADI_CFG_REGISTER_SET_A,
        ADI_REGISTER_OFFSET5,
        ADI_DEFAULT_IIR_SETUP,
        ADI_CFGRA5_FC_0_2_POS,
        ADI_CFGRA5_FC_0_2_MASK,
        &adi_stateBase);
    ADI_StoredConfigurationFillRegisterDataGlobal_Expect(
        ADI_CFG_REGISTER_SET_A,
        ADI_REGISTER_OFFSET5,
        ADI_DEFAULT_COMM_BK_SETUP,
        ADI_CFGRA5_COMM_BK_POS,
        ADI_CFGRA5_COMM_BK_MASK,
        &adi_stateBase);
    ADI_StoredConfigurationFillRegisterDataGlobal_Expect(
        ADI_CFG_REGISTER_SET_A,
        ADI_REGISTER_OFFSET5,
        ADI_DEFAULT_MUTE_ST_SETUP,
        ADI_CFGRA5_MUTE_ST_POS,
        ADI_CFGRA5_MUTE_ST_MASK,
        &adi_stateBase);
    ADI_StoredConfigurationFillRegisterDataGlobal_Expect(
        ADI_CFG_REGISTER_SET_A,
        ADI_REGISTER_OFFSET5,
        ADI_DEFAULT_SNAP_ST_SETUP,
        ADI_CFGRA5_SNAP_ST_POS,
        ADI_CFGRA5_SNAP_ST_MASK,
        &adi_stateBase);

    /* Configuration register B */
    /* CFGRB0 */
    ADI_StoredConfigurationFillRegisterDataGlobal_Expect(
        ADI_CFG_REGISTER_SET_B,
        ADI_REGISTER_OFFSET0,
        ADI_DEFAULT_VUV_0_7_SETUP,
        ADI_CFGRB0_VUV_0_7_POS,
        ADI_CFGRB0_VUV_0_7_MASK,
        &adi_stateBase);
    /* CFGRB1 */
    ADI_StoredConfigurationFillRegisterDataGlobal_Expect(
        ADI_CFG_REGISTER_SET_B,
        ADI_REGISTER_OFFSET1,
        ADI_DEFAULT_VUV_8_11_SETUP,
        ADI_CFGRB1_VUV_8_11_POS,
        ADI_CFGRB1_VUV_8_11_MASK,
        &adi_stateBase);
    ADI_StoredConfigurationFillRegisterDataGlobal_Expect(
        ADI_CFG_REGISTER_SET_B,
        ADI_REGISTER_OFFSET1,
        ADI_DEFAULT_VOV_0_3_SETUP,
        ADI_CFGRB1_VOV_0_3_POS,
        ADI_CFGRB1_VOV_0_3_MASK,
        &adi_stateBase);
    /* CFGRB2 */
    ADI_StoredConfigurationFillRegisterDataGlobal_Expect(
        ADI_CFG_REGISTER_SET_B,
        ADI_REGISTER_OFFSET2,
        ADI_DEFAULT_VOV_4_11_SETUP,
        ADI_CFGRB2_VOV_4_11_POS,
        ADI_CFGRB2_VOV_4_11_MASK,
        &adi_stateBase);
    /* CFGRB3 */
    ADI_StoredConfigurationFillRegisterDataGlobal_Expect(
        ADI_CFG_REGISTER_SET_B,
        ADI_REGISTER_OFFSET3,
        ADI_DEFAULT_DCT0_0_5_SETUP,
        ADI_CFGRB3_DCT0_0_5_POS,
        ADI_CFGRB3_DCT0_0_5_MASK,
        &adi_stateBase);
    ADI_StoredConfigurationFillRegisterDataGlobal_Expect(
        ADI_CFG_REGISTER_SET_B,
        ADI_REGISTER_OFFSET3,
        ADI_DEFAULT_DTRNG_SETUP,
        ADI_CFGRB3_DTRNG_POS,
        ADI_CFGRB3_DTRNG_MASK,
        &adi_stateBase);
    ADI_StoredConfigurationFillRegisterDataGlobal_Expect(
        ADI_CFG_REGISTER_SET_B,
        ADI_REGISTER_OFFSET3,
        ADI_DEFAULT_DTMEN_SETUP,
        ADI_CFGRB3_DTMEN_POS,
        ADI_CFGRB3_DTMEN_MASK,
        &adi_stateBase);
    /* CFGRB4 */
    ADI_StoredConfigurationFillRegisterDataGlobal_Expect(
        ADI_CFG_REGISTER_SET_B,
        ADI_REGISTER_OFFSET4,
        ADI_DEFAULT_DCC_1_8_SETUP,
        ADI_CFGRB4_DCC_1_8_POS,
        ADI_CFGRB4_DCC_1_8_MASK,
        &adi_stateBase);
    /* CFGRB5 */
    ADI_StoredConfigurationFillRegisterDataGlobal_Expect(
        ADI_CFG_REGISTER_SET_B,
        ADI_REGISTER_OFFSET5,
        ADI_DEFAULT_DCC_9_16_SETUP,
        ADI_CFGRB5_DCC_9_16_POS,
        ADI_CFGRB5_DCC_9_16_MASK,
        &adi_stateBase);
}

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

    ADI_ClearAllFlagsInStatusRegisterGroupC_Expects();

    TEST_ADI_ClearAllFlagsInStatusRegisterGroupC(&adi_stateBase);
}

void testADI_DisableBalancingOnStartup(void) {
    /* invalid pointer */
    TEST_ASSERT_FAIL_ASSERT(TEST_ADI_DisableBalancingOnStartup(NULL_PTR));

    ADI_WriteRegisterGlobal_Expect(adi_cmdWrpwma, adi_writeGlobal, ADI_PEC_NO_FAULT_INJECTION, &adi_stateBase);
    ADI_WriteRegisterGlobal_Expect(adi_cmdWrpwmb, adi_writeGlobal, ADI_PEC_NO_FAULT_INJECTION, &adi_stateBase);
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
            ADI_CopyCommandBits_Expect(adi_cmdRdsid, adi_command);
            ADI_ReadRegister_Expect(adi_command, adi_dataReceive, &adi_stateBase);
            TEST_ADI_GetSerialIdsOfAllIcsInString(&adi_stateBase);
            /* Check that Serial ID was extracted correctly from raw data */
            TEST_ASSERT_EQUAL(0xFFEEDDCCBBAAu, adi_stateBase.serialId[s][m]);
        }
    }
}

void testADI_GetRevisionOfAllIcsInString(void) {
    /* invalid pointer */
    TEST_ASSERT_FAIL_ASSERT(TEST_ADI_GetRevisionOfAllIcsInString(NULL_PTR));

    ADI_CopyCommandBits_Expect(adi_cmdRdstate, adi_command);
    ADI_ReadRegister_Expect(adi_command, adi_dataReceive, &adi_stateBase);
    for (uint16_t m = 0; m < ADI_N_ADI; m++) {
        ADI_ReadDataBits_Expect(
            adi_dataReceive[(m * ADI_RDSTATE_LEN) + ADI_REGISTER_OFFSET5],
            &(adi_stateBase.revision[adi_stateBase.currentString][m]),
            ADI_STER5_REV_0_3_POS,
            ADI_STER5_REV_0_3_MASK);
    }
    TEST_ADI_GetRevisionOfAllIcsInString(&adi_stateBase);
}

void testADI_ResetIirFilterOnStartup(void) {
    /* invalid pointer */
    TEST_ASSERT_FAIL_ASSERT(TEST_ADI_ResetIirFilterOnStartup(NULL_PTR));

    ADI_StoredConfigurationFillRegisterDataGlobal_Expect(
        ADI_CFG_REGISTER_SET_A,
        ADI_REGISTER_OFFSET5,
        ADI_IIR_FILTER_PARAMETER_2,
        ADI_CFGRA5_FC_0_2_POS,
        ADI_CFGRA5_FC_0_2_MASK,
        &adi_stateBase);
    ADI_StoredConfigurationWriteToAfe_ExpectAndReturn(ADI_CFG_REGISTER_SET_A, &adi_stateBase, STD_OK);
    TEST_ADI_ResetIirFilterOnStartup(&adi_stateBase);
}
void testADI_StartContinuousCellVoltageMeasurements(void) {
    /* invalid pointer */
    TEST_ASSERT_FAIL_ASSERT(TEST_ADI_StartContinuousCellVoltageMeasurements(NULL_PTR));

    ADI_StartContinuousCellVoltageMeasurements_Expects();

    TEST_ADI_StartContinuousCellVoltageMeasurements(&adi_stateBase);
}

void testADI_WakeUpDaisyChain(void) {
    /* invalid pointer */
    TEST_ASSERT_FAIL_ASSERT(TEST_ADI_WakeUpDaisyChain(NULL_PTR));

    uint16_t txData = 0u;
    ADI_SpiTransmitReceiveData_Expect(&adi_stateBase, &txData, NULL_PTR, 0u);
    ADI_Wait_Expect(ADI_AFE_WAKEUP_TIME);
    ADI_SpiTransmitReceiveData_Expect(&adi_stateBase, &txData, NULL_PTR, 0u);
    ADI_Wait_Expect(ADI_AFE_WAKEUP_TIME);
    TEST_ADI_WakeUpDaisyChain(&adi_stateBase);
}

void testADI_InitializeConfiguration(void) {
    /* invalid pointer */
    TEST_ASSERT_FAIL_ASSERT(TEST_ADI_InitializeConfiguration(NULL_PTR));

    ADI_InitializeConfiguration_Expects();

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
        uint16_t txData = 0u;
        for (uint8_t i = 0; i < 2u; i++) {
            ADI_SpiTransmitReceiveData_Expect(&adi_stateBase, &txData, NULL_PTR, 0u);
            ADI_Wait_Expect(ADI_AFE_WAKEUP_TIME);
        }
        ADI_CopyCommandBits_Expect(adi_cmdSrst, adi_command);
        ADI_TransmitCommand_Expect(adi_command, &adi_stateBase);
        ADI_Wait_Expect(ADI_TSOFTRESET_ms);
        for (uint8_t i = 0; i < 2u; i++) {
            ADI_SpiTransmitReceiveData_Expect(&adi_stateBase, &txData, NULL_PTR, 0u);
            ADI_Wait_Expect(ADI_AFE_WAKEUP_TIME);
        }

        for (uint16_t i = 0u; i < ADI_N_ADI; i++) {
            adi_stateBase.data.errorTable->commandCounterIsOk[adi_stateBase.currentString][i] = true;
            adi_stateBase.data.commandCounter[adi_stateBase.currentString][i] = ADI_COMMAND_COUNTER_RESET_VALUE;
        }
        ADI_ClearCommandCounter_Expect(&adi_stateBase);
        ADI_ClearCommandCounter_ReturnThruPtr_adiState(&adi_stateBase);

        ADI_InitializeConfiguration_Expects();
        ADI_StoredConfigurationFillRegisterDataGlobal_Expect(
            ADI_CFG_REGISTER_SET_A,
            ADI_REGISTER_OFFSET0,
            1u,
            ADI_CFGRA0_REFON_POS,
            ADI_CFGRA0_REFON_MASK,
            &adi_stateBase);
        ADI_StoredConfigurationWriteToAfeGlobal_Expect(&adi_stateBase);

        ADI_Wait_Expect(ADI_TREFUP_ms);

        ADI_WriteRegisterGlobal_Expect(adi_cmdWrpwma, adi_writeGlobal, ADI_PEC_NO_FAULT_INJECTION, &adi_stateBase);
        ADI_WriteRegisterGlobal_Expect(adi_cmdWrpwmb, adi_writeGlobal, ADI_PEC_NO_FAULT_INJECTION, &adi_stateBase);
        ADI_StoredConfigurationFillRegisterDataGlobal_Expect(
            ADI_CFG_REGISTER_SET_A,
            ADI_REGISTER_OFFSET5,
            ADI_IIR_FILTER_PARAMETER_2,
            ADI_CFGRA5_FC_0_2_POS,
            ADI_CFGRA5_FC_0_2_MASK,
            &adi_stateBase);
        ADI_StoredConfigurationWriteToAfe_ExpectAndReturn(ADI_CFG_REGISTER_SET_A, &adi_stateBase, STD_OK);

        ADI_ClearAllFlagsInStatusRegisterGroupC_Expects();

        ADI_CopyCommandBits_Expect(adi_cmdRdstatc, adi_command);
        ADI_ReadRegister_Expect(adi_command, adi_dataReceive, &adi_stateBase);
        for (uint16_t m = 0u; m < ADI_N_ADI; m++) {
            uint8_t statusData = adi_dataReceive[(m * ADI_RDSTATC_LEN) + ADI_REGISTER_OFFSET5];
            uint8_t flagComp   = 0u;
            if (flagComp != 1u) {
                adi_stateBase.data.errorTable->compFlagIsCleared[adi_stateBase.currentString][m] = false;
            }
            ADI_ReadDataBits_Expect(statusData, &flagComp, ADI_STCR5_COMP_POS, ADI_STCR5_COMP_MASK);
            ADI_ReadDataBits_ReturnArrayThruPtr_pDataToRead(&flagComp, 1);
        }

        /* Expects for Starting continuous cell voltage measurements */
        ADI_StartContinuousCellVoltageMeasurements_Expects();
        /* Read serial ID */
        ADI_CopyCommandBits_Expect(adi_cmdRdsid, adi_command);
        ADI_ReadRegister_Expect(adi_command, adi_dataReceive, &adi_stateBase);
        /* Read revision */
        ADI_CopyCommandBits_Expect(adi_cmdRdstate, adi_command);
        ADI_ReadRegister_Expect(adi_command, adi_dataReceive, &adi_stateBase);
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
