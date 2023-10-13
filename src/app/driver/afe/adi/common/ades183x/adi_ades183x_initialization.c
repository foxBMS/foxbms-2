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
 * @file    adi_ades183x_initialization.c
 * @author  foxBMS Team
 * @date    2019-08-27 (date of creation)
 * @updated 2023-10-12 (date of last update)
 * @version v1.6.0
 * @ingroup DRIVER
 * @prefix  ADI
 *
 * @brief   Implementation of some software
 *
 */

/*========== Includes =======================================================*/

#include "adi_ades183x_initialization.h"

#include "adi_ades183x_buffers.h"
#include "adi_ades183x_commands.h"
#include "adi_ades183x_defs.h"
#include "adi_ades183x_helpers.h"

#include <stdbool.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/
/* Wait at least one micro second for the AFE to wakeup */
#define ADI_AFE_WAKEUP_TIME     ((ADI_DAISY_CHAIN_WAKE_UP_TIME_us / ADI_COEFFICIENT_US_TO_MS) + 1u)
#define ADI_ISO_SPI_WAKEUP_TIME ((ADI_DAISY_CHAIN_READY_TIME_us / ADI_COEFFICIENT_US_TO_MS) + 1u)

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/**
 * @brief   TODO.
 * @details TODO
 * @param   adiState state of the driver
 */
static void ADI_ClearAllFlagsInStatusRegisterGroupC(ADI_STATE_s *adiState);

/**
 * @brief   Disable balancing.
 * @details TODO
 * @param   adiState state of the driver
 */
static void ADI_DisableBalancingOnStartup(ADI_STATE_s *adiState);

/**
 * @brief   TODO.
 * @details TODO
 * @param   adiState state of the driver
 */
static void ADI_GetSerialIdsOfAllIcsInString(ADI_STATE_s *adiState);

/**
 * @brief   TODO.
 * @details TODO
 * @param   adiState state of the driver
 */
static void ADI_GetRevisionOfAllIcsInString(ADI_STATE_s *adiState);

/**
 * @brief   Initialize the driver configuration.
 * @details TODO
 * @param   adiState state of the driver
 */
static void ADI_InitializeConfiguration(ADI_STATE_s *adiState);

/**
 * @brief   Resets the error table.
 * @details This function should be called during initialization or before
 *          starting a new measurement cycle
 * @param   adiState state of the driver
 */
static void ADI_ResetErrorTable(ADI_STATE_s *adiState);

/**
 * @brief   Reset TODO.
 * @details TODO
 * @param   adiState state of the driver
 */
static void ADI_ResetIirFilterOnStartup(ADI_STATE_s *adiState);

/**
 * @brief   Start cell voltage measurement.
 * @details This function should be called during initialization or before
 *          starting a new measurement cycle
 * @param   adiState state of the driver
 */
static void ADI_StartContinuousCellVoltageMeasurements(ADI_STATE_s *adiState);

/**
 * @brief   Wake up the AFE.
 * @details TODO
 * @param   adiState state of the driver
 */
static void ADI_WakeUpDaisyChain(ADI_STATE_s *adiState);

/*========== Static Function Implementations ================================*/

static void ADI_ClearAllFlagsInStatusRegisterGroupC(ADI_STATE_s *adiState) {
    FAS_ASSERT(adiState != NULL_PTR);

    adi_clearFlagData[ADI_REGISTER_OFFSET0] = 0u;
    ADI_WriteDataBits(&adi_clearFlagData[ADI_REGISTER_OFFSET0], 1u, ADI_STCR0_CS1FLT_POS, ADI_STCR0_CS1FLT_MASK);
    ADI_WriteDataBits(&adi_clearFlagData[ADI_REGISTER_OFFSET0], 1u, ADI_STCR0_CS2FLT_POS, ADI_STCR0_CS2FLT_MASK);
    ADI_WriteDataBits(&adi_clearFlagData[ADI_REGISTER_OFFSET0], 1u, ADI_STCR0_CS3FLT_POS, ADI_STCR0_CS3FLT_MASK);
    ADI_WriteDataBits(&adi_clearFlagData[ADI_REGISTER_OFFSET0], 1u, ADI_STCR0_CS4FLT_POS, ADI_STCR0_CS4FLT_MASK);
    ADI_WriteDataBits(&adi_clearFlagData[ADI_REGISTER_OFFSET0], 1u, ADI_STCR0_CS5FLT_POS, ADI_STCR0_CS5FLT_MASK);
    ADI_WriteDataBits(&adi_clearFlagData[ADI_REGISTER_OFFSET0], 1u, ADI_STCR0_CS6FLT_POS, ADI_STCR0_CS6FLT_MASK);
    ADI_WriteDataBits(&adi_clearFlagData[ADI_REGISTER_OFFSET0], 1u, ADI_STCR0_CS7FLT_POS, ADI_STCR0_CS7FLT_MASK);
    ADI_WriteDataBits(&adi_clearFlagData[ADI_REGISTER_OFFSET0], 1u, ADI_STCR0_CS8FLT_POS, ADI_STCR0_CS8FLT_MASK);
    adi_clearFlagData[ADI_REGISTER_OFFSET1] = 0u;
    ADI_WriteDataBits(&adi_clearFlagData[ADI_REGISTER_OFFSET1], 1u, ADI_STCR1_CS9FLT_POS, ADI_STCR1_CS9FLT_MASK);
    ADI_WriteDataBits(&adi_clearFlagData[ADI_REGISTER_OFFSET1], 1u, ADI_STCR1_CS10FLT_POS, ADI_STCR1_CS10FLT_MASK);
    ADI_WriteDataBits(&adi_clearFlagData[ADI_REGISTER_OFFSET1], 1u, ADI_STCR1_CS11FLT_POS, ADI_STCR1_CS11FLT_MASK);
    ADI_WriteDataBits(&adi_clearFlagData[ADI_REGISTER_OFFSET1], 1u, ADI_STCR1_CS12FLT_POS, ADI_STCR1_CS12FLT_MASK);
    ADI_WriteDataBits(&adi_clearFlagData[ADI_REGISTER_OFFSET1], 1u, ADI_STCR1_CS13FLT_POS, ADI_STCR1_CS13FLT_MASK);
    ADI_WriteDataBits(&adi_clearFlagData[ADI_REGISTER_OFFSET1], 1u, ADI_STCR1_CS14FLT_POS, ADI_STCR1_CS14FLT_MASK);
    ADI_WriteDataBits(&adi_clearFlagData[ADI_REGISTER_OFFSET1], 1u, ADI_STCR1_CS15FLT_POS, ADI_STCR1_CS15FLT_MASK);
    ADI_WriteDataBits(&adi_clearFlagData[ADI_REGISTER_OFFSET1], 1u, ADI_STCR1_CS16FLT_POS, ADI_STCR1_CS16FLT_MASK);
    adi_clearFlagData[ADI_REGISTER_OFFSET4] = 0u;
    ADI_WriteDataBits(&adi_clearFlagData[ADI_REGISTER_OFFSET4], 1u, ADI_STCR4_SMED_POS, ADI_STCR4_SMED_MASK);
    ADI_WriteDataBits(&adi_clearFlagData[ADI_REGISTER_OFFSET4], 1u, ADI_STCR4_SED_POS, ADI_STCR4_SED_MASK);
    ADI_WriteDataBits(&adi_clearFlagData[ADI_REGISTER_OFFSET4], 1u, ADI_STCR4_CMED_POS, ADI_STCR4_CMED_MASK);
    ADI_WriteDataBits(&adi_clearFlagData[ADI_REGISTER_OFFSET4], 1u, ADI_STCR4_CED_POS, ADI_STCR4_CED_MASK);
    ADI_WriteDataBits(&adi_clearFlagData[ADI_REGISTER_OFFSET4], 1u, ADI_STCR4_VD_UV_POS, ADI_STCR4_VD_UV_MASK);
    ADI_WriteDataBits(&adi_clearFlagData[ADI_REGISTER_OFFSET4], 1u, ADI_STCR4_VD_OV_POS, ADI_STCR4_VD_OV_MASK);
    ADI_WriteDataBits(&adi_clearFlagData[ADI_REGISTER_OFFSET4], 1u, ADI_STCR4_VA_UV_POS, ADI_STCR4_VA_UV_MASK);
    ADI_WriteDataBits(&adi_clearFlagData[ADI_REGISTER_OFFSET4], 1u, ADI_STCR4_VA_OV_POS, ADI_STCR4_VA_OV_MASK);
    adi_clearFlagData[ADI_REGISTER_OFFSET5] = 0u;
    ADI_WriteDataBits(&adi_clearFlagData[ADI_REGISTER_OFFSET5], 1u, ADI_STCR5_OSCCHK_POS, ADI_STCR5_OSCCHK_MASK);
    ADI_WriteDataBits(&adi_clearFlagData[ADI_REGISTER_OFFSET5], 1u, ADI_STCR5_TMODCHK_POS, ADI_STCR5_TMODCHK_MASK);
    ADI_WriteDataBits(&adi_clearFlagData[ADI_REGISTER_OFFSET5], 1u, ADI_STCR5_THSD_POS, ADI_STCR5_THSD_MASK);
    ADI_WriteDataBits(&adi_clearFlagData[ADI_REGISTER_OFFSET5], 1u, ADI_STCR5_SLEEP_POS, ADI_STCR5_SLEEP_MASK);
    ADI_WriteDataBits(&adi_clearFlagData[ADI_REGISTER_OFFSET5], 1u, ADI_STCR5_SPIFLT_POS, ADI_STCR5_SPIFLT_MASK);
    ADI_WriteDataBits(&adi_clearFlagData[ADI_REGISTER_OFFSET5], 1u, ADI_STCR5_COMP_POS, ADI_STCR5_COMP_MASK);
    ADI_WriteDataBits(&adi_clearFlagData[ADI_REGISTER_OFFSET5], 1u, ADI_STCR5_VDE_POS, ADI_STCR5_VDE_MASK);
    ADI_WriteDataBits(&adi_clearFlagData[ADI_REGISTER_OFFSET5], 1u, ADI_STCR5_VDEL_POS, ADI_STCR5_VDEL_MASK);
    ADI_WriteRegisterGlobal(adi_cmdClrflag, adi_clearFlagData, ADI_PEC_NO_FAULT_INJECTION, adiState);
}

static void ADI_DisableBalancingOnStartup(ADI_STATE_s *adiState) {
    FAS_ASSERT(adiState != NULL_PTR);

    /* balancing: Set all PWM values to 0 for all AFEs in the daisy-chain */
    adi_writeGlobal[ADI_REGISTER_OFFSET0] = 0u;
    adi_writeGlobal[ADI_REGISTER_OFFSET1] = 0u;
    adi_writeGlobal[ADI_REGISTER_OFFSET2] = 0u;
    adi_writeGlobal[ADI_REGISTER_OFFSET3] = 0u;
    adi_writeGlobal[ADI_REGISTER_OFFSET4] = 0u;
    adi_writeGlobal[ADI_REGISTER_OFFSET5] = 0u;
    ADI_WriteRegisterGlobal(adi_cmdWrpwma, adi_writeGlobal, ADI_PEC_NO_FAULT_INJECTION, adiState);
    ADI_WriteRegisterGlobal(adi_cmdWrpwmb, adi_writeGlobal, ADI_PEC_NO_FAULT_INJECTION, adiState);
}

static void ADI_GetSerialIdsOfAllIcsInString(ADI_STATE_s *adiState) {
    FAS_ASSERT(adiState != NULL_PTR);

    ADI_CopyCommandBits(adi_cmdRdsid, adi_command);
    ADI_ReadRegister(adi_command, adi_dataReceive, adiState);
    for (uint16_t m = 0; m < ADI_N_ADI; m++) {
        adiState->serialId[adiState->currentString][m] = 0u;
        for (uint8_t byte = 0u; byte < ADI_RDSID_LEN; byte++) {
            adiState->serialId[adiState->currentString][m] |= ((uint64_t)adi_dataReceive[(m * ADI_RDSID_LEN) + byte])
                                                              << (ADI_BYTE_SHIFT * byte);
        }
    }
}

static void ADI_GetRevisionOfAllIcsInString(ADI_STATE_s *adiState) {
    FAS_ASSERT(adiState != NULL_PTR);

    ADI_CopyCommandBits(adi_cmdRdstate, adi_command);
    ADI_ReadRegister(adi_command, adi_dataReceive, adiState);
    for (uint16_t m = 0; m < ADI_N_ADI; m++) {
        ADI_ReadDataBits(
            adi_dataReceive[(m * ADI_RDSTATE_LEN) + ADI_REGISTER_OFFSET5],
            &(adiState->revision[adiState->currentString][m]),
            ADI_STER5_REV_0_3_POS,
            ADI_STER5_REV_0_3_MASK);
    }
}

static void ADI_InitializeConfiguration(ADI_STATE_s *adiState) {
    FAS_ASSERT(adiState != NULL_PTR);

    /* Configuration register A */
    /* CFGRA0 */
    /**
     *  SM_VCELL_RED: Cell Voltage Measurement Redundancy
     *  CTH[2:0]=0b010 -> S-ADC comparison threshold set to 9 mV
     */
    ADI_StoredConfigurationFillRegisterDataGlobal(
        ADI_CFG_REGISTER_SET_A,
        ADI_REGISTER_OFFSET0,
        ADI_DEFAULT_CTH_COMPARISON_THRESHOLD,
        ADI_CFGRA0_CTH_0_2_POS,
        ADI_CFGRA0_CTH_0_2_MASK,
        adiState);
    ADI_StoredConfigurationFillRegisterDataGlobal(
        ADI_CFG_REGISTER_SET_A,
        ADI_REGISTER_OFFSET0,
        ADI_DEFAULT_REFON_SETUP,
        ADI_CFGRA0_REFON_POS,
        ADI_CFGRA0_REFON_MASK,
        adiState);
    /* CFGRA1 */
    /* Do not assert any fault flags. Field will be used in diagnostic functions */
    ADI_StoredConfigurationFillRegisterDataGlobal(
        ADI_CFG_REGISTER_SET_A,
        ADI_REGISTER_OFFSET1,
        ADI_DEFAULT_FLAG_D_SETUP,
        ADI_CFGRA1_FLAG_D_0_7_POS,
        ADI_CFGRA1_FLAG_D_0_7_MASK,
        adiState);
    /* CFGRA2 */
    ADI_StoredConfigurationFillRegisterDataGlobal(
        ADI_CFG_REGISTER_SET_A,
        ADI_REGISTER_OFFSET2,
        ADI_DEFAULT_OWA_SETUP,
        ADI_CFGRA2_OWA_0_2_POS,
        ADI_CFGRA2_OWA_0_2_MASK,
        adiState);
    ADI_StoredConfigurationFillRegisterDataGlobal(
        ADI_CFG_REGISTER_SET_A,
        ADI_REGISTER_OFFSET2,
        ADI_DEFAULT_OWRNG_SETUP,
        ADI_CFGRA2_OWRNG_POS,
        ADI_CFGRA2_OWRNG_MASK,
        adiState);
    /* Disable soak time on AUX ADCs */
    ADI_StoredConfigurationFillRegisterDataGlobal(
        ADI_CFG_REGISTER_SET_A,
        ADI_REGISTER_OFFSET2,
        ADI_DEFAULT_SOAKON_SETUP,
        ADI_CFGRA2_SOAKON_POS,
        ADI_CFGRA2_SOAKON_MASK,
        adiState);
    /* CFGRA3 */
    ADI_StoredConfigurationFillRegisterDataGlobal(
        ADI_CFG_REGISTER_SET_A,
        ADI_REGISTER_OFFSET3,
        ADI_DEFAULT_GPO_1_8_SETUP,
        ADI_CFGRA3_GPO_1_8_POS,
        ADI_CFGRA3_GPO_1_8_MASK,
        adiState);
    /* CFGRA4 */
    ADI_StoredConfigurationFillRegisterDataGlobal(
        ADI_CFG_REGISTER_SET_A,
        ADI_REGISTER_OFFSET4,
        ADI_DEFAULT_GPO_9_10_SETUP,
        ADI_CFGRA4_GPO_9_10_POS,
        ADI_CFGRA4_GPO_9_10_MASK,
        adiState);
    /* CFGRA5 */
    ADI_StoredConfigurationFillRegisterDataGlobal(
        ADI_CFG_REGISTER_SET_A,
        ADI_REGISTER_OFFSET5,
        ADI_DEFAULT_IIR_SETUP,
        ADI_CFGRA5_FC_0_2_POS,
        ADI_CFGRA5_FC_0_2_MASK,
        adiState);
    ADI_StoredConfigurationFillRegisterDataGlobal(
        ADI_CFG_REGISTER_SET_A,
        ADI_REGISTER_OFFSET5,
        ADI_DEFAULT_COMM_BK_SETUP,
        ADI_CFGRA5_COMM_BK_POS,
        ADI_CFGRA5_COMM_BK_MASK,
        adiState);
    ADI_StoredConfigurationFillRegisterDataGlobal(
        ADI_CFG_REGISTER_SET_A,
        ADI_REGISTER_OFFSET5,
        ADI_DEFAULT_MUTE_ST_SETUP,
        ADI_CFGRA5_MUTE_ST_POS,
        ADI_CFGRA5_MUTE_ST_MASK,
        adiState);
    ADI_StoredConfigurationFillRegisterDataGlobal(
        ADI_CFG_REGISTER_SET_A,
        ADI_REGISTER_OFFSET5,
        ADI_DEFAULT_SNAP_ST_SETUP,
        ADI_CFGRA5_SNAP_ST_POS,
        ADI_CFGRA5_SNAP_ST_MASK,
        adiState);

    /* Configuration register B */
    /* CFGRB0 */
    ADI_StoredConfigurationFillRegisterDataGlobal(
        ADI_CFG_REGISTER_SET_B,
        ADI_REGISTER_OFFSET0,
        ADI_DEFAULT_VUV_0_7_SETUP,
        ADI_CFGRB0_VUV_0_7_POS,
        ADI_CFGRB0_VUV_0_7_MASK,
        adiState);
    /* CFGRB1 */
    ADI_StoredConfigurationFillRegisterDataGlobal(
        ADI_CFG_REGISTER_SET_B,
        ADI_REGISTER_OFFSET1,
        ADI_DEFAULT_VUV_8_11_SETUP,
        ADI_CFGRB1_VUV_8_11_POS,
        ADI_CFGRB1_VUV_8_11_MASK,
        adiState);
    ADI_StoredConfigurationFillRegisterDataGlobal(
        ADI_CFG_REGISTER_SET_B,
        ADI_REGISTER_OFFSET1,
        ADI_DEFAULT_VOV_0_3_SETUP,
        ADI_CFGRB1_VOV_0_3_POS,
        ADI_CFGRB1_VOV_0_3_MASK,
        adiState);
    /* CFGRB2 */
    ADI_StoredConfigurationFillRegisterDataGlobal(
        ADI_CFG_REGISTER_SET_B,
        ADI_REGISTER_OFFSET2,
        ADI_DEFAULT_VOV_4_11_SETUP,
        ADI_CFGRB2_VOV_4_11_POS,
        ADI_CFGRB2_VOV_4_11_MASK,
        adiState);
    /* CFGRB3 */
    ADI_StoredConfigurationFillRegisterDataGlobal(
        ADI_CFG_REGISTER_SET_B,
        ADI_REGISTER_OFFSET3,
        ADI_DEFAULT_DCT0_0_5_SETUP,
        ADI_CFGRB3_DCT0_0_5_POS,
        ADI_CFGRB3_DCT0_0_5_MASK,
        adiState);
    ADI_StoredConfigurationFillRegisterDataGlobal(
        ADI_CFG_REGISTER_SET_B,
        ADI_REGISTER_OFFSET3,
        ADI_DEFAULT_DTRNG_SETUP,
        ADI_CFGRB3_DTRNG_POS,
        ADI_CFGRB3_DTRNG_MASK,
        adiState);
    ADI_StoredConfigurationFillRegisterDataGlobal(
        ADI_CFG_REGISTER_SET_B,
        ADI_REGISTER_OFFSET3,
        ADI_DEFAULT_DTMEN_SETUP,
        ADI_CFGRB3_DTMEN_POS,
        ADI_CFGRB3_DTMEN_MASK,
        adiState);
    /* CFGRB4 */
    ADI_StoredConfigurationFillRegisterDataGlobal(
        ADI_CFG_REGISTER_SET_B,
        ADI_REGISTER_OFFSET4,
        ADI_DEFAULT_DCC_1_8_SETUP,
        ADI_CFGRB4_DCC_1_8_POS,
        ADI_CFGRB4_DCC_1_8_MASK,
        adiState);
    /* CFGRB5 */
    ADI_StoredConfigurationFillRegisterDataGlobal(
        ADI_CFG_REGISTER_SET_B,
        ADI_REGISTER_OFFSET5,
        ADI_DEFAULT_DCC_9_16_SETUP,
        ADI_CFGRB5_DCC_9_16_POS,
        ADI_CFGRB5_DCC_9_16_MASK,
        adiState);
}

static void ADI_ResetErrorTable(ADI_STATE_s *adiState) {
    FAS_ASSERT(adiState != NULL_PTR);

    for (uint8_t stringNumber = 0u; stringNumber < BS_NR_OF_STRINGS; stringNumber++) {
        adiState->data.errorTable->spiIsOk[stringNumber] = true;
        for (uint16_t m = 0; m < ADI_N_ADI; m++) {
            adiState->data.errorTable->crcIsOk[stringNumber][m]                                       = true;
            adiState->data.errorTable->commandCounterIsOk[stringNumber][m]                            = true;
            adiState->data.errorTable->configurationAIsOk[stringNumber][m]                            = true;
            adiState->data.errorTable->configurationBIsOk[stringNumber][m]                            = true;
            adiState->data.errorTable->voltageRegisterContentIsNotStuck[stringNumber][m]              = true;
            adiState->data.errorTable->auxiliaryRegisterContentIsNotStuck[stringNumber][m]            = true;
            adiState->data.errorTable->adcComparisonIsOk[stringNumber][m]                             = true;
            adiState->data.errorTable->compFlagIsCleared[stringNumber][m]                             = true;
            adiState->data.errorTable->spiFltFlagIsCleared[stringNumber][m]                           = true;
            adiState->data.errorTable->filteredVoltageRangeIsOk[stringNumber][m]                      = true;
            adiState->data.errorTable->evenWireIsOk[stringNumber][m]                                  = true;
            adiState->data.errorTable->oddWireIsOk[stringNumber][m]                                   = true;
            adiState->data.errorTable->evenWireHasNoLatentFault[stringNumber][m]                      = true;
            adiState->data.errorTable->oddWireHasNoLatentFault[stringNumber][m]                       = true;
            adiState->data.errorTable->adcCompFlagsHaveNoLatentFault[stringNumber][m]                 = true;
            adiState->data.errorTable->auxComparisonIsOk[stringNumber][m]                             = true;
            adiState->data.errorTable->auxVoltageRangeIsOk[stringNumber][m]                           = true;
            adiState->data.errorTable->analogSupplyOvervoltageFlagIsCleared[stringNumber][m]          = true;
            adiState->data.errorTable->analogSupplyUndervoltageFlagIsCleared[stringNumber][m]         = true;
            adiState->data.errorTable->digitalSupplyOvervoltageFlagIsCleared[stringNumber][m]         = true;
            adiState->data.errorTable->digitalSupplyUndervoltageFlagIsCleared[stringNumber][m]        = true;
            adiState->data.errorTable->vdeFlagIsCleared[stringNumber][m]                              = true;
            adiState->data.errorTable->oscchkFlagIsCleared[stringNumber][m]                           = true;
            adiState->data.errorTable->analogSupplyOvervoltageFlagHasNoLatentFault[stringNumber][m]   = true;
            adiState->data.errorTable->analogSupplyUndervoltageFlagHasNoLatentFault[stringNumber][m]  = true;
            adiState->data.errorTable->digitalSupplyOvervoltageFlagHasNoLatentFault[stringNumber][m]  = true;
            adiState->data.errorTable->digitalSupplyUndervoltageFlagHasNoLatentFault[stringNumber][m] = true;
            adiState->data.errorTable->vdeFlagHasNoLatentFault[stringNumber][m]                       = true;
            adiState->data.errorTable->vdelFlagHasNoLatentFault[stringNumber][m]                      = true;
            adiState->data.errorTable->analogSupplyRangeIsOk[stringNumber][m]                         = true;
            adiState->data.errorTable->digitalSupplyRangeIsOk[stringNumber][m]                        = true;
            adiState->data.errorTable->vresRangeIsOk[stringNumber][m]                                 = true;
            adiState->data.errorTable->dieTemperatureIsOk[stringNumber][m]                            = true;
            adiState->data.errorTable->oscchkFlagHasNoLatentFault[stringNumber][m]                    = true;
            adiState->data.errorTable->ocCounterFasterFlagHasNoLatentFault[stringNumber][m]           = true;
            adiState->data.errorTable->ocCounterSlowerFlagHasNoLatentFault[stringNumber][m]           = true;
            adiState->data.errorTable->thsdFlagHasNoLatentFault[stringNumber][m]                      = true;
            adiState->data.errorTable->tmodchkFlagHasNoLatentFault[stringNumber][m]                   = true;
            adiState->data.errorTable->tmodchkFlagIsCleared[stringNumber][m]                          = true;
            adiState->data.errorTable->cmedFlagIsCleared[stringNumber][m]                             = true;
            adiState->data.errorTable->smedFlagIsCleared[stringNumber][m]                             = true;
            adiState->data.errorTable->sleepFlagIsCleared[stringNumber][m]                            = true;
            adiState->data.errorTable->cmedFlagHasNoLatentFault[stringNumber][m]                      = true;
            adiState->data.errorTable->smedFlagHasNoLatentFault[stringNumber][m]                      = true;
            adiState->data.errorTable->initResetAuxValuesAreOk[stringNumber][m]                       = true;
            adiState->data.errorTable->initClearedAuxValuesAreOk[stringNumber][m]                     = true;
        }
    }
}

static void ADI_ResetIirFilterOnStartup(ADI_STATE_s *adiState) {
    FAS_ASSERT(adiState != NULL_PTR);

    /* IIR filter: Change FILT configuration for all AFEs */
    ADI_StoredConfigurationFillRegisterDataGlobal(
        ADI_CFG_REGISTER_SET_A,
        ADI_REGISTER_OFFSET5,
        ADI_IIR_FILTER_PARAMETER_2,
        ADI_CFGRA5_FC_0_2_POS,
        ADI_CFGRA5_FC_0_2_MASK,
        adiState);
    ADI_StoredConfigurationWriteToAfe(ADI_CFG_REGISTER_SET_A, adiState);
}

static void ADI_StartContinuousCellVoltageMeasurements(ADI_STATE_s *adiState) {
    FAS_ASSERT(adiState != NULL_PTR);

    /* Mute balancing before starting cell voltage measurements */
    ADI_CopyCommandBits(adi_cmdMute, adi_command);
    ADI_TransmitCommand(adi_command, adiState);

    /**
     *  SM_VCELL_RED: Cell Voltage Measurement Redundancy
     *  Set RD bit to enable redundant cell voltage measurements
     */
    ADI_CopyCommandBits(adi_cmdAdcv, adi_command);
    ADI_WriteCommandConfigurationBits(adi_command, ADI_ADCV_RD_POS, ADI_ADCV_RD_LEN, 1u);
    ADI_WriteCommandConfigurationBits(adi_command, ADI_ADCV_CONT_POS, ADI_ADCV_CONT_LEN, 1u);
    ADI_WriteCommandConfigurationBits(adi_command, ADI_ADCV_DCP_POS, ADI_ADCV_DCP_LEN, 0u);
    ADI_WriteCommandConfigurationBits(adi_command, ADI_ADCV_RSTF_POS, ADI_ADCV_RSTF_LEN, 1u);
    ADI_WriteCommandConfigurationBits(adi_command, ADI_ADCV_OW01_POS, ADI_ADCV_OW01_LEN, 0u);
    ADI_TransmitCommand(adi_command, adiState);
    ADI_Wait(ADI_IIR_SETTLING_TIME_ms);
}

static void ADI_WakeUpDaisyChain(ADI_STATE_s *adiState) {
    FAS_ASSERT(adiState != NULL_PTR);

    /* Send dummy bytes to wake up the daisy chain */
    uint16_t txData = 0u;
    /* Send dummy byte and wait at least T_wake for each device to wake up */
    ADI_SpiTransmitReceiveData(adiState, &txData, NULL_PTR, 0u);
    ADI_Wait(ADI_AFE_WAKEUP_TIME);

    /* Send dummy byte and wait at least T_ready for each isoSPI to get active */
    ADI_SpiTransmitReceiveData(adiState, &txData, NULL_PTR, 0u);
    ADI_Wait(ADI_ISO_SPI_WAKEUP_TIME);
}

/*========== Extern Function Implementations ================================*/
/* RequirementId: D7.1 V0R4 FUN-4.10.01.01 */
/* RequirementId: D7.1 V0R4 FUN-4.10.01.02 */
/* RequirementId: D7.1 V0R4 FUN-4.10.02.01 */
/* RequirementId: D7.1 V0R4 FUN-6.10.02.01 */
extern void ADI_InitializeMeasurement(ADI_STATE_s *adiState) {
    FAS_ASSERT(adiState != NULL_PTR);

    adiState->spiNumberInterfaces = BS_NR_OF_STRINGS;
    adiState->currentString       = 0u;

    ADI_ResetErrorTable(adiState);

    while (adiState->currentString < adiState->spiNumberInterfaces) {
        adiState->redundantAuxiliaryChannel[adiState->currentString] = ADI_START_AUX_CHANNEL;

        /* Wake up daisy-chain */
        ADI_WakeUpDaisyChain(adiState);
        /* Issue soft reset to be sure that all AFEs are in the reset state */
        ADI_CopyCommandBits(adi_cmdSrst, adi_command);
        ADI_TransmitCommand(adi_command, adiState);
        ADI_Wait(ADI_TSOFTRESET_ms);
        /* Wake up daisy-chain */
        ADI_WakeUpDaisyChain(adiState);
        ADI_ClearCommandCounter(adiState); /* reset command counter */

        /* Set default configuration in AFE Driver storage */
        ADI_InitializeConfiguration(adiState);
        /* REFON bit in configuration register A must be set before checking reset/cleared values */
        ADI_StoredConfigurationFillRegisterDataGlobal(
            ADI_CFG_REGISTER_SET_A, ADI_REGISTER_OFFSET0, 1u, ADI_CFGRA0_REFON_POS, ADI_CFGRA0_REFON_MASK, adiState);
        /* Write default configuration from AFE Driver storage to daisy-chain, for all registers */
        ADI_StoredConfigurationWriteToAfeGlobal(adiState);

        ADI_Wait(ADI_TREFUP_ms);

        ADI_DisableBalancingOnStartup(adiState);

        ADI_ResetIirFilterOnStartup(adiState);

        /* Clear all flags in Status Register Group C */
        ADI_ClearAllFlagsInStatusRegisterGroupC(adiState);

        /* TODO: why and what happens here?! */
        ADI_CopyCommandBits(adi_cmdRdstatc, adi_command);
        ADI_ReadRegister(adi_command, adi_dataReceive, adiState);
        for (uint16_t m = 0u; m < ADI_N_ADI; m++) {
            /* Get STR5 */
            uint8_t statusData = adi_dataReceive[(m * ADI_RDSTATC_LEN) + ADI_REGISTER_OFFSET5];
            /* Check COMP flag */
            uint8_t flagComp = 0u;
            ADI_ReadDataBits(statusData, &flagComp, ADI_STCR5_COMP_POS, ADI_STCR5_COMP_MASK);
            if (flagComp != 1u) {
                adiState->data.errorTable->compFlagIsCleared[adiState->currentString][m] = false;
            }
        }

        /* Start continuous cell voltage measurements */
        ADI_StartContinuousCellVoltageMeasurements(adiState);
        /* Read serial IDs of all ICs in string */
        ADI_GetSerialIdsOfAllIcsInString(adiState);
        /* Read revisions of all ICs in string */
        ADI_GetRevisionOfAllIcsInString(adiState);
        ++adiState->currentString;
    }
    adiState->currentString = 0u;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
extern void TEST_ADI_ClearAllFlagsInStatusRegisterGroupC(ADI_STATE_s *adiState) {
    ADI_ClearAllFlagsInStatusRegisterGroupC(adiState);
}
extern void TEST_ADI_DisableBalancingOnStartup(ADI_STATE_s *adiState) {
    ADI_DisableBalancingOnStartup(adiState);
}
extern void TEST_ADI_GetRevisionOfAllIcsInString(ADI_STATE_s *adiState) {
    ADI_GetRevisionOfAllIcsInString(adiState);
}
extern void TEST_ADI_GetSerialIdsOfAllIcsInString(ADI_STATE_s *adiState) {
    ADI_GetSerialIdsOfAllIcsInString(adiState);
}
extern void TEST_ADI_InitializeConfiguration(ADI_STATE_s *adiState) {
    ADI_InitializeConfiguration(adiState);
}
extern void TEST_ADI_ResetErrorTable(ADI_STATE_s *adiState) {
    ADI_ResetErrorTable(adiState);
}
extern void TEST_ADI_StartContinuousCellVoltageMeasurements(ADI_STATE_s *adiState) {
    ADI_StartContinuousCellVoltageMeasurements(adiState);
}
extern void TEST_ADI_WakeUpDaisyChain(ADI_STATE_s *adiState) {
    ADI_WakeUpDaisyChain(adiState);
}
extern void TEST_ADI_ResetIirFilterOnStartup(ADI_STATE_s *adiState) {
    ADI_ResetIirFilterOnStartup(adiState);
}
#endif
