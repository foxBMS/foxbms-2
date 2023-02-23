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
 * @file    test_nxpfs85xx.c
 * @author  foxBMS Team
 * @date    2020-04-06 (date of creation)
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  SBC
 *
 * @brief   Tests for the sbc module
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockdiag.h"
#include "Mockdma.h"
#include "Mockfassert.h"
#include "Mockfram.h"
#include "Mockio.h"
#include "Mockmasterinfo.h"
#include "Mockmcu.h"
#include "Mocksbc_fs8x.h"
#include "Mocksbc_fs8x_communication.h"
#include "Mockspi.h"

#include "nxpfs85xx.h"
#include "test_assert_helper.h"

#include <stdbool.h>

TEST_FILE("nxpfs85xx.c")

/*========== Definitions and Implementations for Unit Test ==================*/
#define REGISTER_TEST_VALUE (1234u)

#define MAIN_REGISTER     fs85xx_mcuSupervisor.mainRegister
#define FAILSAFE_REGISTER fs85xx_mcuSupervisor.fsRegister

static const spiDAT1_t spi_kSbcDataConfig = {
    /* struct is implemented in the TI HAL and uses uppercase true and false */
    .CS_HOLD = FALSE,     /* The HW chip select signal is deactivated */
    .WDEL    = TRUE,      /* No delay will be inserted */
    .DFSEL   = SPI_FMT_0, /* Data word format select */
    .CSNR    = 0x0,       /* Chip select (CS) number; 0x01h for CS[0] */
};

SPI_INTERFACE_CONFIG_s spi_sbcMcuInterface = {
    .pConfig  = &spi_kSbcDataConfig,
    .pNode    = spiREG2,
    .pGioPort = &(spiREG2->PC3),
    .csPin    = 0u,
};

FRAM_SBC_INIT_s fram_sbcInit = {
    .phase    = 0u,
    .finState = STD_NOT_OK,
};

long FSYS_RaisePrivilege(void) {
    return 0;
}

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/

void testDoSomething(void) {
}

void testSBC_CheckRegisterValues(void) {
    TEST_ASSERT_EQUAL(STD_OK, TEST_SBC_CheckRegisterValues(0u, 0u));
    TEST_ASSERT_EQUAL(STD_OK, TEST_SBC_CheckRegisterValues(UINT32_MAX, UINT32_MAX));
    TEST_ASSERT_EQUAL(STD_NOT_OK, TEST_SBC_CheckRegisterValues(0u, 1u));
    TEST_ASSERT_EQUAL(STD_NOT_OK, TEST_SBC_CheckRegisterValues(0u, UINT32_MAX));
}

void testSBC_UpdateRegister(void) {
    TEST_SBC_UpdateRegister(&fs85xx_mcuSupervisor, true, 0u, REGISTER_TEST_VALUE);
    TEST_SBC_UpdateRegister(&fs85xx_mcuSupervisor, false, 0u, REGISTER_TEST_VALUE);
    TEST_ASSERT_FAIL_ASSERT(TEST_SBC_UpdateRegister(NULL_PTR, true, 0u, REGISTER_TEST_VALUE));
    TEST_ASSERT_FAIL_ASSERT(
        TEST_SBC_UpdateRegister(&fs85xx_mcuSupervisor, true, FS8X_M_DEVICEID_ADDR + 1u, REGISTER_TEST_VALUE));
}

void testSBC_UpdateFailSafeRegister(void) {
    TEST_SBC_UpdateFailSafeRegister(
        &(fs85xx_mcuSupervisor.fsRegister), FS8X_FS_GRL_FLAGS_ADDR, REGISTER_TEST_VALUE + 0u);
    TEST_SBC_UpdateFailSafeRegister(
        &(fs85xx_mcuSupervisor.fsRegister), FS8X_FS_I_OVUV_SAFE_REACTION1_ADDR, REGISTER_TEST_VALUE + 1u);
    TEST_SBC_UpdateFailSafeRegister(
        &(fs85xx_mcuSupervisor.fsRegister), FS8X_FS_I_OVUV_SAFE_REACTION2_ADDR, REGISTER_TEST_VALUE + 2u);
    TEST_SBC_UpdateFailSafeRegister(
        &(fs85xx_mcuSupervisor.fsRegister), FS8X_FS_I_WD_CFG_ADDR, REGISTER_TEST_VALUE + 3u);
    TEST_SBC_UpdateFailSafeRegister(
        &(fs85xx_mcuSupervisor.fsRegister), FS8X_FS_I_SAFE_INPUTS_ADDR, REGISTER_TEST_VALUE + 4u);
    TEST_SBC_UpdateFailSafeRegister(&(fs85xx_mcuSupervisor.fsRegister), FS8X_FS_I_FSSM_ADDR, REGISTER_TEST_VALUE + 5u);
    TEST_SBC_UpdateFailSafeRegister(&(fs85xx_mcuSupervisor.fsRegister), FS8X_FS_I_SVS_ADDR, REGISTER_TEST_VALUE + 6u);
    TEST_SBC_UpdateFailSafeRegister(
        &(fs85xx_mcuSupervisor.fsRegister), FS8X_FS_WD_WINDOW_ADDR, REGISTER_TEST_VALUE + 7u);
    TEST_SBC_UpdateFailSafeRegister(&(fs85xx_mcuSupervisor.fsRegister), FS8X_FS_WD_SEED_ADDR, REGISTER_TEST_VALUE + 8u);
    TEST_SBC_UpdateFailSafeRegister(
        &(fs85xx_mcuSupervisor.fsRegister), FS8X_FS_WD_ANSWER_ADDR, REGISTER_TEST_VALUE + 9u);
    TEST_SBC_UpdateFailSafeRegister(
        &(fs85xx_mcuSupervisor.fsRegister), FS8X_FS_OVUVREG_STATUS_ADDR, REGISTER_TEST_VALUE + 10u);
    TEST_SBC_UpdateFailSafeRegister(
        &(fs85xx_mcuSupervisor.fsRegister), FS8X_FS_RELEASE_FS0B_ADDR, REGISTER_TEST_VALUE + 11u);
    TEST_SBC_UpdateFailSafeRegister(
        &(fs85xx_mcuSupervisor.fsRegister), FS8X_FS_SAFE_IOS_ADDR, REGISTER_TEST_VALUE + 12u);
    TEST_SBC_UpdateFailSafeRegister(
        &(fs85xx_mcuSupervisor.fsRegister), FS8X_FS_DIAG_SAFETY_ADDR, REGISTER_TEST_VALUE + 13u);
    TEST_SBC_UpdateFailSafeRegister(
        &(fs85xx_mcuSupervisor.fsRegister), FS8X_FS_INTB_MASK_ADDR, REGISTER_TEST_VALUE + 14u);
    TEST_SBC_UpdateFailSafeRegister(&(fs85xx_mcuSupervisor.fsRegister), FS8X_FS_STATES_ADDR, REGISTER_TEST_VALUE + 15u);

    TEST_ASSERT_EQUAL(FAILSAFE_REGISTER.grl_flags, REGISTER_TEST_VALUE + 0u);
    TEST_ASSERT_EQUAL(FAILSAFE_REGISTER.iOvervoltageUndervoltageSafeReaction1, REGISTER_TEST_VALUE + 1u);
    TEST_ASSERT_EQUAL(FAILSAFE_REGISTER.iOvervoltageUndervoltageSafeReaction2, REGISTER_TEST_VALUE + 2u);
    TEST_ASSERT_EQUAL(FAILSAFE_REGISTER.iWatchdogConfiguration, REGISTER_TEST_VALUE + 3u);
    TEST_ASSERT_EQUAL(FAILSAFE_REGISTER.i_safe_inputs, REGISTER_TEST_VALUE + 4u);
    TEST_ASSERT_EQUAL(FAILSAFE_REGISTER.iFailSafeSateMachine, REGISTER_TEST_VALUE + 5u);
    TEST_ASSERT_EQUAL(FAILSAFE_REGISTER.i_svs, REGISTER_TEST_VALUE + 6u);
    TEST_ASSERT_EQUAL(FAILSAFE_REGISTER.watchdogWindow, REGISTER_TEST_VALUE + 7u);
    TEST_ASSERT_EQUAL(FAILSAFE_REGISTER.watchdogSeed, REGISTER_TEST_VALUE + 8u);
    TEST_ASSERT_EQUAL(FAILSAFE_REGISTER.watchdogAnswer, REGISTER_TEST_VALUE + 9u);
    TEST_ASSERT_EQUAL(FAILSAFE_REGISTER.overvoltageUndervoltageRegisterStatus, REGISTER_TEST_VALUE + 10u);
    TEST_ASSERT_EQUAL(FAILSAFE_REGISTER.releaseFs0bPin, REGISTER_TEST_VALUE + 11u);
    TEST_ASSERT_EQUAL(FAILSAFE_REGISTER.safeIos, REGISTER_TEST_VALUE + 12u);
    TEST_ASSERT_EQUAL(FAILSAFE_REGISTER.diag_safety, REGISTER_TEST_VALUE + 13u);
    TEST_ASSERT_EQUAL(FAILSAFE_REGISTER.intb_mask, REGISTER_TEST_VALUE + 14u);
    TEST_ASSERT_EQUAL(FAILSAFE_REGISTER.states, REGISTER_TEST_VALUE + 15u);
}

void testSBC_UpdateMainRegister(void) {
    TEST_SBC_UpdateMainRegister(&(fs85xx_mcuSupervisor.mainRegister), FS8X_M_FLAG_ADDR, REGISTER_TEST_VALUE + 0u);
    TEST_SBC_UpdateMainRegister(&(fs85xx_mcuSupervisor.mainRegister), FS8X_M_MODE_ADDR, REGISTER_TEST_VALUE + 1u);
    TEST_SBC_UpdateMainRegister(&(fs85xx_mcuSupervisor.mainRegister), FS8X_M_REG_CTRL1_ADDR, REGISTER_TEST_VALUE + 2u);
    TEST_SBC_UpdateMainRegister(&(fs85xx_mcuSupervisor.mainRegister), FS8X_M_REG_CTRL2_ADDR, REGISTER_TEST_VALUE + 3u);
    TEST_SBC_UpdateMainRegister(&(fs85xx_mcuSupervisor.mainRegister), FS8X_M_AMUX_ADDR, REGISTER_TEST_VALUE + 4u);
    TEST_SBC_UpdateMainRegister(&(fs85xx_mcuSupervisor.mainRegister), FS8X_M_CLOCK_ADDR, REGISTER_TEST_VALUE + 5u);
    TEST_SBC_UpdateMainRegister(&(fs85xx_mcuSupervisor.mainRegister), FS8X_M_INT_MASK1_ADDR, REGISTER_TEST_VALUE + 6u);
    TEST_SBC_UpdateMainRegister(&(fs85xx_mcuSupervisor.mainRegister), FS8X_M_INT_MASK2_ADDR, REGISTER_TEST_VALUE + 7u);
    TEST_SBC_UpdateMainRegister(&(fs85xx_mcuSupervisor.mainRegister), FS8X_M_FLAG1_ADDR, REGISTER_TEST_VALUE + 8u);
    TEST_SBC_UpdateMainRegister(&(fs85xx_mcuSupervisor.mainRegister), FS8X_M_FLAG2_ADDR, REGISTER_TEST_VALUE + 9u);
    TEST_SBC_UpdateMainRegister(&(fs85xx_mcuSupervisor.mainRegister), FS8X_M_VMON_REGX_ADDR, REGISTER_TEST_VALUE + 10u);
    TEST_SBC_UpdateMainRegister(&(fs85xx_mcuSupervisor.mainRegister), FS8X_M_LVB1_SVS_ADDR, REGISTER_TEST_VALUE + 11u);
    TEST_SBC_UpdateMainRegister(&(fs85xx_mcuSupervisor.mainRegister), FS8X_M_MEMORY0_ADDR, REGISTER_TEST_VALUE + 12u);
    TEST_SBC_UpdateMainRegister(&(fs85xx_mcuSupervisor.mainRegister), FS8X_M_MEMORY1_ADDR, REGISTER_TEST_VALUE + 13u);
    TEST_SBC_UpdateMainRegister(&(fs85xx_mcuSupervisor.mainRegister), FS8X_M_DEVICEID_ADDR, REGISTER_TEST_VALUE + 14u);

    TEST_ASSERT_EQUAL(MAIN_REGISTER.flag, REGISTER_TEST_VALUE + 0u);
    TEST_ASSERT_EQUAL(MAIN_REGISTER.mode, REGISTER_TEST_VALUE + 1u);
    TEST_ASSERT_EQUAL(MAIN_REGISTER.registerControl1, REGISTER_TEST_VALUE + 2u);
    TEST_ASSERT_EQUAL(MAIN_REGISTER.registerControl2, REGISTER_TEST_VALUE + 3u);
    TEST_ASSERT_EQUAL(MAIN_REGISTER.analogMultiplexer, REGISTER_TEST_VALUE + 4u);
    TEST_ASSERT_EQUAL(MAIN_REGISTER.clock, REGISTER_TEST_VALUE + 5u);
    TEST_ASSERT_EQUAL(MAIN_REGISTER.int_mask1, REGISTER_TEST_VALUE + 6u);
    TEST_ASSERT_EQUAL(MAIN_REGISTER.int_mask2, REGISTER_TEST_VALUE + 7u);
    TEST_ASSERT_EQUAL(MAIN_REGISTER.flag1, REGISTER_TEST_VALUE + 8u);
    TEST_ASSERT_EQUAL(MAIN_REGISTER.flag2, REGISTER_TEST_VALUE + 9u);
    TEST_ASSERT_EQUAL(MAIN_REGISTER.vmon_regx, REGISTER_TEST_VALUE + 10u);
    TEST_ASSERT_EQUAL(MAIN_REGISTER.lvb1_svs, REGISTER_TEST_VALUE + 11u);
    TEST_ASSERT_EQUAL(MAIN_REGISTER.memory0, REGISTER_TEST_VALUE + 12u);
    TEST_ASSERT_EQUAL(MAIN_REGISTER.memory1, REGISTER_TEST_VALUE + 13u);
    TEST_ASSERT_EQUAL(MAIN_REGISTER.deviceId, REGISTER_TEST_VALUE + 14u);
}
