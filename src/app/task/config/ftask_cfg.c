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
 * @file    ftask_cfg.c
 * @author  foxBMS Team
 * @date    2019-08-26 (date of creation)
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
 * @ingroup TASK_CONFIGURATION
 * @prefix  FTSK
 *
 * @brief   Task configuration
 * @details
 */

/*========== Includes =======================================================*/
#include "ftask_cfg.h"

#include "HL_gio.h"
#include "HL_het.h"

#include "adc.h"
#include "algorithm.h"
#include "bal.h"
#include "bms.h"
#include "can.h"
#include "contactor.h"
#include "database.h"
#include "diag.h"
#include "dma.h"
#include "fram.h"
#include "htsensor.h"
#include "i2c.h"
#include "imd.h"
#include "interlock.h"
#include "led.h"
#include "meas.h"
#include "pex.h"
#include "redundancy.h"
#include "rtc.h"
#include "sbc.h"
#include "sof_trapezoid.h"
#include "spi.h"
#include "sps.h"
#include "state_estimation.h"
#include "sys.h"
#include "sys_mon.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/** counter value for 50ms in 10ms task */
#define TASK_10MS_COUNTER_FOR_50MS (5u)

/** counter value for 1s in 100ms task */
#define TASK_100MS_COUNTER_FOR_1S (10u)

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/**
 * @brief   Definition of the engine task
 * @details Task is not delayed after the scheduler starts. This task  must
 *          have the highest priority.
 * @warning Do not change the configuration of this task. This will very
 *          likely break the system.
 */
OS_TASK_DEFINITION_s ftsk_taskDefinitionEngine = {
    FTSK_TASK_ENGINE_PRIORITY,
    FTSK_TASK_ENGINE_PHASE,
    FTSK_TASK_ENGINE_CYCLE_TIME,
    FTSK_TASK_ENGINE_STACK_SIZE_IN_BYTES,
    FTSK_TASK_ENGINE_PV_PARAMETERS};
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
OS_TASK_DEFINITION_s ftsk_taskDefinitionI2c = {
    FTSK_TASK_I2C_PRIORITY,
    FTSK_TASK_I2C_PHASE,
    FTSK_TASK_I2C_CYCLE_TIME,
    FTSK_TASK_I2C_STACK_SIZE_IN_BYTES,
    FTSK_TASK_I2C_PV_PARAMETERS};
OS_TASK_DEFINITION_s ftsk_taskDefinitionAfe = {
    FTSK_TASK_AFE_PRIORITY,
    FTSK_TASK_AFE_PHASE,
    FTSK_TASK_AFE_CYCLE_TIME,
    FTSK_TASK_AFE_STACK_SIZE_IN_BYTES,
    FTSK_TASK_AFE_PV_PARAMETERS};

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/
extern void FTSK_InitializeUserCodeEngine(void) {
    /* Warning: Do not change the content of this function */
    /* See function definition doxygen comment for details */
    STD_RETURN_TYPE_e retval = DATA_Initialize();

    if (retval == E_NOT_OK) {
        /* Fatal error! */
        FAS_ASSERT(FAS_TRAP);
    }

    /* Suspend AFE task if unused, otherwise it will preempt all lower priority tasks */
#if (FOXBMS_AFE_DRIVER_TYPE_FSM == 1)
    vTaskSuspend(ftsk_taskHandleAfe);
#endif

    /* Init FRAM */
    FRAM_Initialize();

    retval = SYSM_Init();

    if (retval == E_NOT_OK) {
        /* Fatal error! */
        FAS_ASSERT(FAS_TRAP);
    }

    /* Warning: Do not change the content of this function */
    /* See function definition doxygen comment for details */
}

extern void FTSK_RunUserCodeEngine(void) {
    /* Warning: Do not change the content of this function */
    /* See function definition doxygen comment for details */
    DATA_Task();               /* Call database manager */
    SYSM_CheckNotifications(); /* Check notifications from tasks */
    /* Warning: Do not change the content of this function */
    /* See function definition doxygen comment for details */
}

extern void FTSK_InitializeUserCodePreCyclicTasks(void) {
    /* user code */
    SYS_RETURN_TYPE_e sys_retVal = SYS_ILLEGAL_REQUEST;

    /*  Init Sys */
    sys_retVal = SYS_SetStateRequest(SYS_STATE_INITIALIZATION_REQUEST);

    /* Init port expander */
    PEX_Initialize();

    /* Set 3rd PE pin to activate temperature/humidity sensor */
    PEX_SetPinDirectionOutput(PEX_PORT_EXPANDER3, PEX_PIN00);
    PEX_SetPin(PEX_PORT_EXPANDER3, PEX_PIN00);

    CONT_Initialize();
    SPS_Initialize();
    (void)MEAS_Initialize(); /* cast to void as the return value is unused */

    /* Initialize redundancy module */
    (void)MRC_Initialize();

    /* This function operates under the assumption that it is called when
     * the operating system is not yet running.
     * In this state the return value of #SYS_SetStateRequest should
     * always be #SYS_OK. Therefore we trap otherwise.
     */
    FAS_ASSERT(sys_retVal == SYS_OK);

    /* System started correctly -> Start toggling of debug LED */
    LED_SetToggleTime(LED_NORMAL_OPERATION_ON_OFF_TIME_ms);
}

extern void FTSK_RunUserCodeCyclic1ms(void) {
    /* Increment of operating system timer */
    /* This must not be changed, add user code only below */
    OS_IncrementTimer();
    DIAG_UpdateFlags();
    /* user code */
#if (FOXBMS_AFE_DRIVER_TYPE_FSM == 1)
    MEAS_Control();
#endif
    CAN_ReadRxBuffer();
}

extern void FTSK_RunUserCodeCyclic10ms(void) {
    static uint8_t ftsk_cyclic10msCounter = 0;
    /* user code */
    SYSM_UpdateFramData();
    SYS_Trigger(&sys_state);
    ILCK_Trigger();
    ADC_Control();
    SPS_Ctrl();
    CAN_MainFunction();
    SOF_Calculation();
    ALGO_MonitorExecutionTime();
    SBC_Trigger(&sbc_stateMcuSupervisor);

    if (ftsk_cyclic10msCounter == TASK_10MS_COUNTER_FOR_50MS) {
        MRC_ValidateAfeMeasurement();
        MRC_ValidatePackMeasurement();
        ftsk_cyclic10msCounter = 0;
    }
    /* Call BMS_Trigger function at the end of the 10ms task to allow previously
     * called modules in this task to update respectively evaluate their new.
     * This minimizes the delay between data evaluation and the reaction from
     * the BMS module */
    BMS_Trigger();
    ftsk_cyclic10msCounter++;
}

extern void FTSK_RunUserCodeCyclic100ms(void) {
    /* user code */
    static uint8_t ftsk_cyclic100msCounter = 0;

    /** Perform SOC and SOE calculations only every 1s. Not suited if analog
     *  integration of current sensor is NOT used. Manual integration of current
     *  requires a higher frequency.
     */
    if (ftsk_cyclic100msCounter == TASK_100MS_COUNTER_FOR_1S) {
        SE_RunStateEstimations();
        ftsk_cyclic100msCounter = 0;
    }

    BAL_Trigger();
    IMD_Trigger();
    LED_Trigger();

    ftsk_cyclic100msCounter++;
}

extern void FTSK_RunUserCodeCyclicAlgorithm100ms(void) {
    /* user code */
    static uint8_t ftsk_cyclicAlgorithm100msCounter = 0;

    ALGO_MainFunction();

    ftsk_cyclicAlgorithm100msCounter++;
}

void FTSK_RunUserCodeI2c(void) {
    /* user code */
    PEX_Trigger();
    HTSEN_Trigger();
    RTC_Trigger();
    uint32_t current_time = OS_GetTickCount();
    OS_DelayTaskUntil(&current_time, 2u);
}

void FTSK_RunUserCodeAfe(void) {
    /* user code */
#if (FOXBMS_AFE_DRIVER_TYPE_NO_FSM == 1)
    MEAS_Control();
#endif
}

extern void FTSK_RunUserCodeIdle(void) {
    /* user code */
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
