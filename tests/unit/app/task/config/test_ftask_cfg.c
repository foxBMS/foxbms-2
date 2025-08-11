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
 * @file    test_ftask_cfg.c
 * @author  foxBMS Team
 * @date    2020-04-02 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Tests for the ftask_cfg driver
 * @details TODO
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "MockHL_gio.h"
#include "MockHL_mdio.h"
#include "Mockadc.h"
#include "Mockafe.h"
#include "Mockalgorithm.h"
#include "Mockbal.h"
#include "Mockbms.h"
#include "Mockcan.h"
#include "Mockcontactor.h"
#include "Mockdatabase.h"
#include "Mockdiag.h"
#include "Mockdiag_cfg.h"
#include "Mockdp83869.h"
#include "Mockfram.h"
#include "Mockhtsensor.h"
#include "Mocki2c.h"
#include "Mockimd.h"
#include "Mockinterlock.h"
#include "Mockled.h"
#include "Mockmaster_info.h"
#include "Mockmeas.h"
#include "Mockmpu_prototypes.h"
#include "Mockos.h"
#include "Mockpex.h"
#include "Mockredundancy.h"
#include "Mockrtc.h"
#include "Mocksbc.h"
#include "Mocksof_trapezoid.h"
#include "Mocksps.h"
#include "Mockstate_estimation.h"
#include "Mocksys.h"
#include "Mocksys_mon.h"

#include "fram_cfg.h"
#include "ftask_cfg.h"
#include "pex_cfg.h"
#include "sys_mon_cfg.h"

#include "fassert.h"
#include "ftask.h"
#include "test_assert_helper.h"

/*========== Unit Testing Framework Directives ==============================*/
TEST_INCLUDE_PATH("../../src/app/application/algorithm")
TEST_INCLUDE_PATH("../../src/app/application/algorithm/config")
TEST_INCLUDE_PATH("../../src/app/application/algorithm/state_estimation")
TEST_INCLUDE_PATH("../../src/app/application/algorithm/state_estimation/sof/trapezoid")
TEST_INCLUDE_PATH("../../src/app/application/bal")
TEST_INCLUDE_PATH("../../src/app/application/bms")
TEST_INCLUDE_PATH("../../src/app/application/redundancy")
TEST_INCLUDE_PATH("../../src/app/driver/adc")
TEST_INCLUDE_PATH("../../src/app/driver/afe/api")
TEST_INCLUDE_PATH("../../src/app/driver/can")
TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/contactor")
TEST_INCLUDE_PATH("../../src/app/driver/dma")
TEST_INCLUDE_PATH("../../src/app/driver/fram")
TEST_INCLUDE_PATH("../../src/app/driver/htsensor")
TEST_INCLUDE_PATH("../../src/app/driver/i2c")
TEST_INCLUDE_PATH("../../src/app/driver/imd")
TEST_INCLUDE_PATH("../../src/app/driver/interlock")
TEST_INCLUDE_PATH("../../src/app/driver/led")
TEST_INCLUDE_PATH("../../src/app/driver/meas")
TEST_INCLUDE_PATH("../../src/app/driver/pex")
TEST_INCLUDE_PATH("../../src/app/driver/phy")
TEST_INCLUDE_PATH("../../src/app/driver/rtc")
TEST_INCLUDE_PATH("../../src/app/driver/sbc")
TEST_INCLUDE_PATH("../../src/app/driver/sbc/fs8x_driver")
TEST_INCLUDE_PATH("../../src/app/driver/spi")
TEST_INCLUDE_PATH("../../src/app/driver/sps")
TEST_INCLUDE_PATH("../../src/app/engine/diag")
TEST_INCLUDE_PATH("../../src/app/engine/hw_info")
TEST_INCLUDE_PATH("../../src/app/engine/sys")
TEST_INCLUDE_PATH("../../src/app/engine/sys_mon")
TEST_INCLUDE_PATH("../../src/app/task/config")
TEST_INCLUDE_PATH("../../src/app/task/ftask")

/*========== Definitions and Implementations for Unit Test ==================*/
OS_TASK_HANDLE ftsk_taskHandleAfe;

#define FTSK_DATA_QUEUE_LENGTH      (1u)
#define FTSK_DATA_QUEUE_ITEM_SIZE   (sizeof(DATA_QUEUE_MESSAGE_s))
#define FTSK_IMD_QUEUE_LENGTH       (5u)
#define FTSK_IMD_QUEUE_ITEM_SIZE    (sizeof(CAN_BUFFER_ELEMENT_s))
#define FTSK_CAN_RX_QUEUE_LENGTH    (50u)
#define FTSK_CAN_RX_QUEUE_ITEM_SIZE (sizeof(CAN_BUFFER_ELEMENT_s))

volatile OS_BOOT_STATE_e os_boot = OS_OFF;
volatile OS_TIMER_s os_timer     = {0, 0, 0, 0, 0, 0, 0};
uint32_t os_schedulerStartTime   = 0;
SBC_STATE_s sbc_stateMcuSupervisor;

SYS_STATE_s sys_state = {0};

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
void testFTSK_RunUserCodeIdle(void) {
    TEST_ASSERT_PASS_ASSERT(FTSK_RunUserCodeIdle());
}

void testFTSK_InitializeUserCodeEngine(void) {
    DATA_Initialize_ExpectAndReturn(STD_NOT_OK);
    TEST_ASSERT_FAIL_ASSERT(FTSK_InitializeUserCodeEngine())

    DATA_Initialize_ExpectAndReturn(STD_OK);
    FRAM_Initialize_Expect();
    SYSM_Initialize_ExpectAndReturn(STD_NOT_OK);
    TEST_ASSERT_FAIL_ASSERT(FTSK_InitializeUserCodeEngine())

    DATA_Initialize_ExpectAndReturn(STD_OK);
    FRAM_Initialize_Expect();
    SYSM_Initialize_ExpectAndReturn(STD_OK);
    FTSK_InitializeUserCodeEngine();
}

void testFTSK_RunUserCodeEngine(void) {
    DATA_Task_Expect();
    SYSM_CheckNotifications_Expect();
    FTSK_RunUserCodeEngine();
}

/**
 * @brief   Testing extern function #FTSK_InitializeUserCodePreCyclicTasks
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - none
 *          - Routine validation:
 *            - RT1/2: Fail assertion on initialize
 *            - RT2/2: Pass assertion on initialize
 */
void testFTSK_InitializeUserCodePreCyclicTasks(void) {
    /* ======= Routine tests =============================================== */
    /* ======= RT1/2: Test implementation */
    SYS_SetStateRequest_ExpectAndReturn(SYS_STATE_INITIALIZATION_REQUEST, STD_NOT_OK);
    PEX_Initialize_Expect();
    PEX_SetPinDirectionOutput_Expect(PEX_PORT_EXPANDER3, PEX_PORT_0_PIN_0);
    PEX_SetPin_Expect(PEX_PORT_EXPANDER3, PEX_PORT_0_PIN_0);

    CONT_Initialize_Expect();
    SPS_Initialize_Expect();
    MEAS_Initialize_ExpectAndReturn(STD_OK);
    MRC_Initialize_ExpectAndReturn(STD_OK);
    MDIOInit_Expect(0xFCF78900u, 100.00f * 1000000.00f, 1000000u);
    PHY_Initialize_ExpectAndReturn(0xFCF78900u, STD_OK);
    /* ======= RT1/2: Call function under test */
    TEST_ASSERT_FAIL_ASSERT(FTSK_InitializeUserCodePreCyclicTasks());

    /* ======= RT2/2: Test implementation */
    SYS_SetStateRequest_ExpectAndReturn(SYS_STATE_INITIALIZATION_REQUEST, STD_OK);
    PEX_Initialize_Expect();
    PEX_SetPinDirectionOutput_Expect(PEX_PORT_EXPANDER3, PEX_PORT_0_PIN_0);
    PEX_SetPin_Expect(PEX_PORT_EXPANDER3, PEX_PORT_0_PIN_0);
    CONT_Initialize_Expect();
    SPS_Initialize_Expect();
    MEAS_Initialize_ExpectAndReturn(STD_OK);
    MRC_Initialize_ExpectAndReturn(STD_OK);
    MDIOInit_Expect(0xFCF78900u, 100.00f * 1000000.00f, 1000000u);
    PHY_Initialize_ExpectAndReturn(0xFCF78900u, STD_OK);

    LED_SetToggleTime_Expect(LED_NORMAL_OPERATION_ON_OFF_TIME_ms);
    /* ======= RT2/2: Call function under test */
    FTSK_InitializeUserCodePreCyclicTasks();
}

void testFTSK_RunUserCodeCyclic1ms(void) {
    OS_IncrementTimer_Expect();
    DIAG_UpdateFlags_Expect();
    CAN_ReadRxBuffer_Expect();
    FTSK_RunUserCodeCyclic1ms();
}

void testFTSK_RunUserCodeCyclic10ms(void) {
}

void testFTSK_RunUserCodeCyclic100ms(void) {
    BAL_Trigger_Expect();
    IMD_Trigger_ExpectAndReturn(STD_OK);
    LED_Trigger_Expect();
    MINFO_CheckSupplyVoltageClamp30c_Expect();
    FTSK_RunUserCodeCyclic100ms();

    for (uint8_t i = 0u; i < 9u; i++) {
        BAL_Trigger_Expect();
        IMD_Trigger_ExpectAndReturn(STD_OK);
        LED_Trigger_Expect();
        MINFO_CheckSupplyVoltageClamp30c_Expect();
        FTSK_RunUserCodeCyclic100ms();
    }
    SE_RunStateEstimations_Expect();
    BAL_Trigger_Expect();
    IMD_Trigger_ExpectAndReturn(STD_OK);
    LED_Trigger_Expect();
    MINFO_CheckSupplyVoltageClamp30c_Expect();
    FTSK_RunUserCodeCyclic100ms();
}

void testFTSK_RunUserCodeCyclicAlgorithm100ms(void) {
    ALGO_MainFunction_Expect();
    FTSK_RunUserCodeCyclicAlgorithm100ms();
}

void testFTSK_RunUserCodeI2c(void) {
    PEX_Trigger_Expect();
    HTSEN_Trigger_Expect();
    RTC_Trigger_Expect();
    OS_GetTickCount_ExpectAndReturn(2u);
    uint32_t currentTime = 2u;
    OS_DelayTaskUntil_Expect(&currentTime, 2u);
    FTSK_RunUserCodeI2c();
}

void testFTSK_RunUserCodeAfe(void) {
    FTSK_RunUserCodeAfe();
}
