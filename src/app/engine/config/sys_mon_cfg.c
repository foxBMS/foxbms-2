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
 * @file    sys_mon_cfg.c
 * @author  foxBMS Team
 * @date    2019-11-28 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup ENGINE_CONFIGURATION
 * @prefix  SYSM
 *
 * @brief   Configuration of the system monitoring module
 * @details Test functions:
 *          - SYSM_DummyCallback
 *
 */

/*========== Includes =======================================================*/
#include "sys_mon_cfg.h"

#include "ftask_cfg.h"

#include "fram.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/* pre-commit: needs to be defined before */
static void SYSM_DummyCallback(SYSM_TASK_ID_e taskId);

/*========== Extern Constant and Variable Definitions =======================*/
/* DOCUMENTATION marker - sys_mon config entry */
SYSM_MONITORING_CFG_s sysm_ch_cfg[] = {
    {SYSM_TASK_ID_ENGINE,
     SYSM_ENABLED,
     FTSK_TASK_ENGINE_CYCLE_TIME,
     FTSK_TASK_ENGINE_MAXIMUM_JITTER,
     SYSM_RECORDING_ENABLED,
     SYSM_HANDLING_SWITCH_OFF_CONTACTOR,
     SYSM_DummyCallback},
    {SYSM_TASK_ID_CYCLIC_1ms,
     SYSM_ENABLED,
     FTSK_TASK_CYCLIC_1MS_CYCLE_TIME,
     FTSK_TASK_CYCLIC_1MS_MAXIMUM_JITTER,
     SYSM_RECORDING_ENABLED,
     SYSM_HANDLING_SWITCH_OFF_CONTACTOR,
     SYSM_DummyCallback},
    {SYSM_TASK_ID_CYCLIC_10ms,
     SYSM_ENABLED,
     FTSK_TASK_CYCLIC_10MS_CYCLE_TIME,
     FTSK_TASK_CYCLIC_10MS_MAXIMUM_JITTER,
     SYSM_RECORDING_ENABLED,
     SYSM_HANDLING_SWITCH_OFF_CONTACTOR,
     SYSM_DummyCallback},
    {SYSM_TASK_ID_CYCLIC_100ms,
     SYSM_ENABLED,
     FTSK_TASK_CYCLIC_100MS_CYCLE_TIME,
     FTSK_TASK_CYCLIC_100MS_MAXIMUM_JITTER,
     SYSM_RECORDING_ENABLED,
     SYSM_HANDLING_SWITCH_OFF_CONTACTOR,
     SYSM_DummyCallback},
    {SYSM_TASK_ID_CYCLIC_ALGORITHM_100ms,
     SYSM_ENABLED,
     FTSK_TASK_CYCLIC_ALGORITHM_100MS_CYCLE_TIME,
     FTSK_TASK_CYCLIC_ALGORITHM_100MS_MAXIMUM_JITTER,
     SYSM_RECORDING_ENABLED,
     SYSM_HANDLING_SWITCH_OFF_CONTACTOR,
     SYSM_DummyCallback},
};
/* DOCUMENTATION marker - sys_mon config exit */

FAS_STATIC_ASSERT(
    (sizeof(sysm_ch_cfg) / sizeof(SYSM_MONITORING_CFG_s)) == (uint32_t)SYSM_TASK_ID_MAX,
    "invalid sys_mon configuration!");

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/
/**
 * @brief   dummy callback function of system monitoring error events
 */
static void SYSM_DummyCallback(SYSM_TASK_ID_e taskId) {
    /* this is a dummy implementation and not using the argument here is fine */
    (void)taskId;
    /* Dummy function therefore it is empty */
}

/*========== Extern Function Implementations ================================*/

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
void TEST_SYSM_DummyCallback(SYSM_TASK_ID_e taskId) {
    SYSM_DummyCallback(taskId);
}
#endif
