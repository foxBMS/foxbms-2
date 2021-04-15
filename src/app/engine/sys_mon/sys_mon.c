/**
 *
 * @copyright &copy; 2010 - 2021, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * @file    sys_mon.c
 * @author  foxBMS Team
 * @date    2019-11-28 (date of creation)
 * @updated 2020-05-28 (date of last update)
 * @ingroup ENGINE
 * @prefix  SYSM
 *
 * @brief   TODO
 */

/*========== Includes =======================================================*/
#include "sys_mon.h"

#include "diag.h"
#include "os.h"

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/
/** tracking variable for System monitoring notifications */
static SYSM_NOTIFICATION_s sysm_notifications[SYSM_TASK_ID_MAX];

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/
void SYSM_CheckNotifications(void) {
    static uint32_t sysm_timestamp = 0;

    /** early exit if nothing to do */
    uint32_t local_timer = OS_GetTickCount();
    if (sysm_timestamp == local_timer) {
        return;
    }
    sysm_timestamp = local_timer;

    uint32_t time_since_last_call = 0;
    uint32_t max_allowed_jitter   = 0;

    for (SYSM_TASK_ID_e tsk_id = (SYSM_TASK_ID_e)0; tsk_id < SYSM_TASK_ID_MAX; tsk_id++) {
        if (sysm_ch_cfg[tsk_id].enable == SYSM_ENABLED) {
            /* check that the task gets called within its timing threshold plus jitter and
               then check that the tasks duration is shorter than tasks cycle time */
            time_since_last_call = local_timer - sysm_notifications[tsk_id].timestampEnter;
            max_allowed_jitter   = sysm_ch_cfg[tsk_id].cycleTime + sysm_ch_cfg[tsk_id].maxJitter;
            if ((time_since_last_call > max_allowed_jitter) &&
                (sysm_notifications[tsk_id].duration > sysm_ch_cfg[tsk_id].cycleTime)) {
                /* module not running within its timed limits */
                DIAG_Handler(DIAG_ID_SYSTEMMONITORING, DIAG_EVENT_NOT_OK, DIAG_SYSTEM, tsk_id);
                if (sysm_ch_cfg[tsk_id].enableRecording == SYSM_RECORDING_ENABLED) {
                    /* TODO add recording function (when MRAM/FRAM are available) */
                }
                sysm_ch_cfg[tsk_id].callbackfunc(tsk_id);
            }
        }
    }
}

void SYSM_Notify(SYSM_TASK_ID_e tsk_id, SYSM_NOTIFY_TYPE_e state, uint32_t time) {
    if (tsk_id < SYSM_TASK_ID_MAX) {
        sysm_notifications[tsk_id].state = state;
        OS_EnterTaskCritical();
        if (SYSM_NOTIFY_ENTER == state) {
            sysm_notifications[tsk_id].timestampEnter = time;
        } else if (SYSM_NOTIFY_EXIT == state) {
            sysm_notifications[tsk_id].timestampExit = time;
            sysm_notifications[tsk_id].duration      = time - sysm_notifications[tsk_id].timestampEnter;
        } else {
            /* state has an illegal value */
            FAS_ASSERT(FAS_TRAP);
        }
        OS_ExitTaskCritical();
    }
}

/*========== Getter for static Variables (Unit Test) ========================*/
#ifdef UNITY_UNIT_TEST
extern SYSM_NOTIFICATION_s *TEST_SYSM_GetNotifications(void) {
    return sysm_notifications;
}
#endif

/*========== Externalized Static Function Implementations (Unit Test) =======*/
