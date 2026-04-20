/**
 *
 * @copyright &copy; 2010 - 2026, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * @file    test_nxp_mc33775a_alarm.c
 * @author  foxBMS Team
 * @date    2025-10-10 (date of creation)
 * @updated 2026-04-20 (date of last update)
 * @version v1.11.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Test of nxp_mc33775a_alarm.c
 * @details TODO
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockdatabase.h"
#include "Mockdiag.h"
#include "Mocknxp_mc3377x-ll.h"

#include "nxp_mc3377x_alarm.h"
#include "nxp_mc3377x_reg_def.h"
#include "spi_cfg-helper.h"

/* clang-format off */
#include "test_assert_helper.h"
/* clang-format on */

#include <stdbool.h>
#include <stdint.h>

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("nxp_mc33775a_alarm.c")

TEST_INCLUDE_PATH("../../src/app/driver/afe/nxp/common/mc3377x")
TEST_INCLUDE_PATH("../../src/app/driver/afe/nxp/common/mc3377x/vendor")
TEST_INCLUDE_PATH("../../src/app/driver/afe/nxp/mc33775a")
TEST_INCLUDE_PATH("../../src/app/driver/afe/nxp/mc33775a/config")
TEST_INCLUDE_PATH("../../src/app/driver/afe/nxp/mc33775a/vendor")
TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/spi")
TEST_INCLUDE_PATH("../../src/app/engine/diag")

/*========== Definitions and Implementations for Unit Test ==================*/

/** SPI data configuration struct for NXP MC3377X communication, Tx part */
static spiDAT1_t spi_kNxp77xDataConfigTx[BS_NR_OF_STRINGS] = {
    {.CS_HOLD = TRUE,      /* If true, HW chip select kept active */
     .WDEL    = TRUE,      /* Activation of delay between words */
     .DFSEL   = SPI_FMT_0, /* Data word format selection */
     .CSNR    = SPI_HARDWARE_CHIP_SELECT_2_ACTIVE},
};

/** SPI interface configuration for N77X communication Tx part */
SPI_INTERFACE_CONFIG_s spi_nxp77xInterfaceTx[BS_NR_OF_STRINGS] = {
    {
        .pConfig  = &spi_kNxp77xDataConfigTx[0u],
        .pNode    = spiREG1,
        .pGioPort = &(spiREG1->PC3),
        .csPin    = 2u,
        .csType   = SPI_CHIP_SELECT_HARDWARE,
    },
};

N77X_STATE_s n77x_testState = {
    .currentString  = 0u,
    .pSpiTxSequence = spi_nxp77xInterfaceTx,
};

uint32_t N77X_ALARM_PORT[8] = {0u};

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
void testN77x_InitializeAlarm(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(N77x_InitializeAlarm(NULL_PTR));

    /* ======= Routine tests =============================================== */
    uint16_t alarmOutCfg0  = 0x4807;
    uint16_t supplyFltCfg0 = 0x3FFF;
    uint16_t supplyFltCfg1 = 0x003F;
    uint16_t anaFltCfg     = 0x0007;
    uint16_t comFltCfg     = 0x0017;
    uint16_t measFltCfg    = 0x0007;

    /* ======= RT1/2 ======= */

    /* Enable pins for checking OV & UV */
    N77x_CommunicationWrite_Expect(
        N77X_BROADCAST_ADDRESS,
        MC3377X_PRMM_VC_OV_UV_CFG_OFFSET,
        N77X_PRMM_VC_OV_UV_CFG,
        n77x_testState.pSpiTxSequence);

    /* Set thresholds according to config */
    N77x_CommunicationWrite_Expect(
        N77X_BROADCAST_ADDRESS,
        MC3377X_PRMM_VC_OV_TH_CFG_OFFSET,
        N77X_PRMM_VC_OV_TH_CFG,
        n77x_testState.pSpiTxSequence);
    N77x_CommunicationWrite_Expect(
        N77X_BROADCAST_ADDRESS,
        MC3377X_PRMM_VC_UV0_TH_CFG_OFFSET,
        N77X_PRMM_VC_UV0_TH_CFG,
        n77x_testState.pSpiTxSequence);
    N77x_CommunicationWrite_Expect(
        N77X_BROADCAST_ADDRESS,
        MC3377X_PRMM_VC_UV1_TH_CFG_OFFSET,
        N77X_PRMM_VC_UV1_TH_CFG,
        n77x_testState.pSpiTxSequence);

    /* From N77x_ConfigureAlarmFaults*/
    N77x_CommunicationWrite_Expect(
        N77X_BROADCAST_ADDRESS, MC3377X_FEH_SUPPLY_FLT_EVT_CFG0_OFFSET, supplyFltCfg0, n77x_testState.pSpiTxSequence);
    N77x_CommunicationWrite_Expect(
        N77X_BROADCAST_ADDRESS, MC3377X_FEH_SUPPLY_FLT_EVT_CFG1_OFFSET, supplyFltCfg1, n77x_testState.pSpiTxSequence);
    N77x_CommunicationWrite_Expect(
        N77X_BROADCAST_ADDRESS, MC3377X_FEH_ANA_FLT_EVT_CFG_OFFSET, anaFltCfg, n77x_testState.pSpiTxSequence);
    N77x_CommunicationWrite_Expect(
        N77X_BROADCAST_ADDRESS, MC3377X_FEH_COM_FLT_EVT_CFG_OFFSET, comFltCfg, n77x_testState.pSpiTxSequence);
    N77x_CommunicationWrite_Expect(
        N77X_BROADCAST_ADDRESS, MC3377X_FEH_MEAS_FLT_EVT_CFG_OFFSET, measFltCfg, n77x_testState.pSpiTxSequence);
    N77x_CommunicationWrite_Expect(
        N77X_BROADCAST_ADDRESS, MC3377X_FEH_ALARM_OUT_CFG_OFFSET, alarmOutCfg0, n77x_testState.pSpiTxSequence);

    /* Enable alarm input and output for all */
    N77x_CommunicationWrite_Expect(
        N77X_BROADCAST_ADDRESS, MC3377X_FEH_ALARM_CFG_OFFSET, 0x22, n77x_testState.pSpiTxSequence);

    /* Disable alarm input for last slave */
    N77x_CommunicationWrite_Expect(
        BS_NR_OF_MODULES_PER_STRING, MC3377X_FEH_ALARM_CFG_OFFSET, 0x20, n77x_testState.pSpiTxSequence);

    /**
     * Read to reset all alarms occurred during initialization
     * At least the last slave triggered an alarm for not having ALARMIN disabled
     * -> Reset from last to first
     */
    uint16_t reasonVal = 0u;
    for (uint8_t i = BS_NR_OF_MODULES_PER_STRING; i >= 1; i--) {
        N77x_CommunicationRead_ExpectAndReturn(
            i, MC3377X_FEH_ALARM_OUT_REASON_OFFSET, &reasonVal, &n77x_testState, N77X_COMMUNICATION_OK);
    }

    TEST_ASSERT_PASS_ASSERT(N77x_InitializeAlarm(&n77x_testState));
}

void testN77x_HandleAlarm(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(N77x_HandleAlarm(NULL_PTR));

    /* ======= Routine tests =============================================== */
    /* Handled in CheckAlarm */
}

void testN77x_CheckAlarm(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(N77x_CheckAlarm(NULL_PTR));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/4 ======= */
    /* No alarm */
    ((gioPORT_t *)N77X_ALARM_PORT)->DIN = 1u << 4u;
    n77x_testState.firstMeasurementMade = false;
    TEST_ASSERT_PASS_ASSERT(N77x_CheckAlarm(&n77x_testState));

    /* ======= RT2/4 ======= */
    /* No alarm */
    ((gioPORT_t *)N77X_ALARM_PORT)->DIN = 0u << 4u;
    n77x_testState.firstMeasurementMade = false;
    TEST_ASSERT_PASS_ASSERT(N77x_CheckAlarm(&n77x_testState));

    /* ======= RT3/4 ======= */
    /* No alarm */
    ((gioPORT_t *)N77X_ALARM_PORT)->DIN = 1u << 4u;
    n77x_testState.firstMeasurementMade = true;
    TEST_ASSERT_PASS_ASSERT(N77x_CheckAlarm(&n77x_testState));

    /* ======= RT4/4 ======= */
    /* alarm */
    DIAG_Handler_ExpectAndReturn(DIAG_ID_AFE_ALARM, DIAG_EVENT_NOT_OK, DIAG_SYSTEM, 0u, DIAG_HANDLER_RETURN_OK);

    ((gioPORT_t *)N77X_ALARM_PORT)->DIN = 0u << 4u;
    n77x_testState.firstMeasurementMade = true;

    /* From N77x_HandleAlarm */
    for (uint8_t s = 0; s < BS_NR_OF_STRINGS; s++) {
        for (uint8_t m = 0; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            N77x_CommunicationRead_ExpectAndReturn(
                m + 1,
                MC3377X_FEH_ALARM_OUT_REASON_OFFSET,
                &n77x_testState.alarm.alarmOutReason[s][m],
                &n77x_testState,
                N77X_COMMUNICATION_OK);
            N77x_CommunicationRead_ExpectAndReturn(
                m + 1,
                MC3377X_FEH_SUPPLY_FLT_STAT0_OFFSET,
                &n77x_testState.alarm.supplyFaultState0[s][m],
                &n77x_testState,
                N77X_COMMUNICATION_OK);
            N77x_CommunicationRead_ExpectAndReturn(
                m + 1,
                MC3377X_FEH_SUPPLY_FLT_STAT1_OFFSET,
                &n77x_testState.alarm.supplyFaultState1[s][m],
                &n77x_testState,
                N77X_COMMUNICATION_OK);
            N77x_CommunicationRead_ExpectAndReturn(
                m + 1,
                MC3377X_FEH_ANA_FLT_STAT_OFFSET,
                &n77x_testState.alarm.anaFaultState[s][m],
                &n77x_testState,
                N77X_COMMUNICATION_OK);
            N77x_CommunicationRead_ExpectAndReturn(
                m + 1,
                MC3377X_FEH_COM_FLT_STAT_OFFSET,
                &n77x_testState.alarm.comFaultState[s][m],
                &n77x_testState,
                N77X_COMMUNICATION_OK);
            N77x_CommunicationRead_ExpectAndReturn(
                m + 1,
                MC3377X_FEH_MEAS_FLT_STAT_OFFSET,
                &n77x_testState.alarm.measFaultState[s][m],
                &n77x_testState,
                N77X_COMMUNICATION_OK);
        }
    }

    TEST_ASSERT_PASS_ASSERT(N77x_CheckAlarm(&n77x_testState));
}

void testN77x_ConfigureAlarmCellOvUv(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_N77x_ConfigureAlarmCellOvUv(NULL_PTR));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/1 ======= */
    /* Enable pins for checking OV & UV */

    /* From N77x_ConfigureAlarmCellOvUv */
    N77x_CommunicationWrite_Expect(
        N77X_BROADCAST_ADDRESS,
        MC3377X_PRMM_VC_OV_UV_CFG_OFFSET,
        N77X_PRMM_VC_OV_UV_CFG,
        n77x_testState.pSpiTxSequence);

    /* Set thresholds according to config */
    N77x_CommunicationWrite_Expect(
        N77X_BROADCAST_ADDRESS,
        MC3377X_PRMM_VC_OV_TH_CFG_OFFSET,
        N77X_PRMM_VC_OV_TH_CFG,
        n77x_testState.pSpiTxSequence);
    N77x_CommunicationWrite_Expect(
        N77X_BROADCAST_ADDRESS,
        MC3377X_PRMM_VC_UV0_TH_CFG_OFFSET,
        N77X_PRMM_VC_UV0_TH_CFG,
        n77x_testState.pSpiTxSequence);
    N77x_CommunicationWrite_Expect(
        N77X_BROADCAST_ADDRESS,
        MC3377X_PRMM_VC_UV1_TH_CFG_OFFSET,
        N77X_PRMM_VC_UV1_TH_CFG,
        n77x_testState.pSpiTxSequence);

    TEST_ASSERT_PASS_ASSERT(TEST_N77x_ConfigureAlarmCellOvUv(&n77x_testState));
}

void testN77x_ConfigureAlarmFaults(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_N77x_ConfigureAlarmFaults(NULL_PTR));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/1 ======= */
    uint16_t alarmOutCfg0  = 0x4807;
    uint16_t supplyFltCfg0 = 0x3FFF;
    uint16_t supplyFltCfg1 = 0x003F;
    uint16_t anaFltCfg     = 0x0007;
    uint16_t comFltCfg     = 0x0017;
    uint16_t measFltCfg    = 0x0007;

    N77x_CommunicationWrite_Expect(
        N77X_BROADCAST_ADDRESS, MC3377X_FEH_SUPPLY_FLT_EVT_CFG0_OFFSET, supplyFltCfg0, n77x_testState.pSpiTxSequence);
    N77x_CommunicationWrite_Expect(
        N77X_BROADCAST_ADDRESS, MC3377X_FEH_SUPPLY_FLT_EVT_CFG1_OFFSET, supplyFltCfg1, n77x_testState.pSpiTxSequence);
    N77x_CommunicationWrite_Expect(
        N77X_BROADCAST_ADDRESS, MC3377X_FEH_ANA_FLT_EVT_CFG_OFFSET, anaFltCfg, n77x_testState.pSpiTxSequence);
    N77x_CommunicationWrite_Expect(
        N77X_BROADCAST_ADDRESS, MC3377X_FEH_COM_FLT_EVT_CFG_OFFSET, comFltCfg, n77x_testState.pSpiTxSequence);
    N77x_CommunicationWrite_Expect(
        N77X_BROADCAST_ADDRESS, MC3377X_FEH_MEAS_FLT_EVT_CFG_OFFSET, measFltCfg, n77x_testState.pSpiTxSequence);
    N77x_CommunicationWrite_Expect(
        N77X_BROADCAST_ADDRESS, MC3377X_FEH_ALARM_OUT_CFG_OFFSET, alarmOutCfg0, n77x_testState.pSpiTxSequence);
    TEST_ASSERT_PASS_ASSERT(TEST_N77x_ConfigureAlarmFaults(&n77x_testState));
}
