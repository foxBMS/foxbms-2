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
 * @file    test_ftask_emac.c
 * @author  foxBMS Team
 * @date    2020-11-14 (date of creation)
 * @updated 2026-10-06 (date of last update)
 * @version v1.12.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Test of the ftask module
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
#include "Mockemac.h"
#include "Mockfram.h"
#include "Mockftask_cfg.h"
#include "Mockhtsensor.h"
#include "Mocki2c.h"
#include "Mockimd.h"
#include "Mockinfinite-loop-helper.h"
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

#include "sys_mon_cfg.h"

#include "fassert.h"
#include "ftask.h"
#include "test_assert_helper.h"

/*========== Unit Testing Framework Directives ==============================*/

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
uint32_t fram_voltage            = 0u;
FRAM_SOC_s fram_soc              = {0};
SBC_STATE_s sbc_stateMcuSupervisor;

SYS_STATE_s sys_state = {0};

OS_TASK_DEFINITION_s ftsk_taskDefinitionCyclic1ms = {
    FTSK_TASK_CYCLIC_1MS_PRIORITY,
    FTSK_TASK_CYCLIC_1MS_PHASE,
    FTSK_TASK_CYCLIC_1MS_CYCLE_TIME,
    FTSK_TASK_CYCLIC_1MS_STACK_SIZE_IN_BYTES,
    FTSK_TASK_CYCLIC_1MS_PV_PARAMETERS};
OS_TASK_DEFINITION_s ftsk_taskDefinitionCyclic10ms = {
    FTSK_TASK_CYCLIC_10MS_PRIORITY,
    FTSK_TASK_CYCLIC_10MS_PHASE,
    FTSK_TASK_CYCLIC_10MS_CYCLE_TIME,
    FTSK_TASK_CYCLIC_10MS_STACK_SIZE_IN_BYTES,
    FTSK_TASK_CYCLIC_10MS_PV_PARAMETERS};
OS_TASK_DEFINITION_s ftsk_taskDefinitionCyclic100ms = {
    FTSK_TASK_CYCLIC_100MS_PRIORITY,
    FTSK_TASK_CYCLIC_100MS_PHASE,
    FTSK_TASK_CYCLIC_100MS_CYCLE_TIME,
    FTSK_TASK_CYCLIC_100MS_STACK_SIZE_IN_BYTES,
    FTSK_TASK_CYCLIC_100MS_PV_PARAMETERS};
OS_TASK_DEFINITION_s ftsk_taskDefinitionCyclicAlgorithm100ms = {
    FTSK_TASK_CYCLIC_ALGORITHM_100MS_PRIORITY,
    FTSK_TASK_CYCLIC_ALGORITHM_100MS_PHASE,
    FTSK_TASK_CYCLIC_ALGORITHM_100MS_CYCLE_TIME,
    FTSK_TASK_CYCLIC_ALGORITHM_100MS_STACK_SIZE_IN_BYTES,
    FTSK_TASK_CYCLIC_ALGORITHM_100MS_PV_PARAMETERS};

/*========== Setup and Teardown =============================================*/
void setUp(void) {
    os_boot               = OS_OFF;
    os_schedulerStartTime = 1u;
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/

void testFTSK_CreateTaskCyclic100ms(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    uint32_t dummy = 1u;
    TEST_ASSERT_FAIL_ASSERT(FTSK_CreateTaskCyclic100ms(&dummy));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    /* tasks requires engine task to be running, otherwise, we wait forever */
    os_boot               = OS_PRE_CYCLIC_INITIALIZATION_HAS_FINISHED;
    os_schedulerStartTime = 1u;
    OS_MarkTaskAsRequiringFpuContext_Expect();

    OS_DelayTaskUntil_Expect(&os_schedulerStartTime, ftsk_taskDefinitionCyclic100ms.phase);
    uint32_t currentTimeCreateTaskCyclic100ms = 1u;
    OS_GetTickCount_ExpectAndReturn(currentTimeCreateTaskCyclic100ms);

    FOREVER_ExpectAndReturn(1);
    uint32_t tickCount = 2u;
    OS_GetTickCount_ExpectAndReturn(tickCount);
    SYSM_Notify_Expect(SYSM_TASK_ID_CYCLIC_100ms, SYSM_NOTIFY_ENTER, tickCount);
    FTSK_RunUserCodeCyclic100ms_Expect();
    tickCount = 3u;
    OS_GetTickCount_ExpectAndReturn(tickCount);
    SYSM_Notify_Expect(SYSM_TASK_ID_CYCLIC_100ms, SYSM_NOTIFY_EXIT, tickCount);
    OS_DelayTaskUntil_Expect(&currentTimeCreateTaskCyclic100ms, ftsk_taskDefinitionCyclic100ms.cycleTime);

    FOREVER_ExpectAndReturn(0);

    /* ======= RT1/1: call function under test */
    FTSK_CreateTaskCyclic100ms(NULL_PTR);
    /* ======= RT1/1: test output verification */
    TEST_ASSERT_EQUAL(os_boot, OS_PRE_CYCLIC_INITIALIZATION_HAS_FINISHED);
}

void testFTSK_CreateTaskEmac(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    uint32_t dummy = 1u; /* no pvParameters shall be passed */
    TEST_ASSERT_FAIL_ASSERT(FTSK_CreateTaskEmac(&dummy));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    OS_MarkTaskAsRequiringFpuContext_Expect();
    /* tasks requires engine task to be running, otherwise, we wait forever */
    os_boot               = OS_PRE_CYCLIC_INITIALIZATION_HAS_FINISHED;
    os_schedulerStartTime = 1u;

    FOREVER_ExpectAndReturn(1);
    FTSK_RunUserCodeEmac_Expect();
    FOREVER_ExpectAndReturn(0);

    /* ======= RT1/1: call function under test */
    FTSK_CreateTaskEmac(NULL_PTR);
    /* ======= RT1/1: test output verification */
    /* no output to verify */
}
