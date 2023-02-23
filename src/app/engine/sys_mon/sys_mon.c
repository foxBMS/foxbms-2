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
 * @file    sys_mon.c
 * @author  foxBMS Team
 * @date    2019-11-28 (date of creation)
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
 * @ingroup ENGINE
 * @prefix  SYSM
 *
 * @brief   system monitoring module
 */

/*========== Includes =======================================================*/
#include "sys_mon.h"

#include "diag.h"
#include "fram.h"
#include "os.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/
/** tracking variable for System monitoring notifications */
static SYSM_NOTIFICATION_s sysm_notifications[SYSM_TASK_ID_MAX];

/** local shadow copy of the FRAM entry */
static FRAM_SYS_MON_RECORD_s sysm_localFramCopy = {0};

/** flag, indicating that the FRAM entry has been changed and should be written */
static volatile bool sysm_flagFramCopyHasChanges = false;

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/
/**
 * @brief Records the violation of a timing in non volatile memory
 *
 * @param[in]   taskId          id of the violating task
 * @param[in]   taskDuration    duration of the task when recording the violation
 * @param[in]   timestampEnter  timestamp that the task has entered
 */
static void SYSM_RecordTimingViolation(SYSM_TASK_ID_e taskId, uint32_t taskDuration, uint32_t timestampEnter);

/**
 * @brief   Convert recorded timings into a decision (bool) whether the timing has been violated or not
 * @details Timing has been violated if both duration or timestamp are not equal to zero
 * @param[in]   duration        duration in ticks
 * @param[in]   timestampEnter  entry timestamp in ticks
 * @returns     bool describing whether this is a violation (true) or not (false)
 */
static bool SYSM_ConvertRecordedTimingsToViolation(uint32_t duration, uint32_t timestampEnter);

/*========== Static Function Implementations ================================*/
static void SYSM_RecordTimingViolation(SYSM_TASK_ID_e taskId, uint32_t taskDuration, uint32_t timestampEnter) {
    FAS_ASSERT(taskId < SYSM_TASK_ID_MAX);

    sysm_flagFramCopyHasChanges               = true;
    sysm_localFramCopy.anyTimingIssueOccurred = true;

    switch (taskId) {
        case SYSM_TASK_ID_ENGINE:
            sysm_localFramCopy.taskEngineViolatingDuration = taskDuration;
            sysm_localFramCopy.taskEngineEnterTimestamp    = timestampEnter;
            break;
        case SYSM_TASK_ID_CYCLIC_1ms:
            sysm_localFramCopy.task1msViolatingDuration = taskDuration;
            sysm_localFramCopy.task1msEnterTimestamp    = timestampEnter;
            break;
        case SYSM_TASK_ID_CYCLIC_10ms:
            sysm_localFramCopy.task10msViolatingDuration = taskDuration;
            sysm_localFramCopy.task10msEnterTimestamp    = timestampEnter;
            break;
        case SYSM_TASK_ID_CYCLIC_100ms:
            sysm_localFramCopy.task100msViolatingDuration = taskDuration;
            sysm_localFramCopy.task100msEnterTimestamp    = timestampEnter;
            break;
        case SYSM_TASK_ID_CYCLIC_ALGORITHM_100ms:
            sysm_localFramCopy.task100msAlgorithmViolatingDuration = taskDuration;
            sysm_localFramCopy.task100msAlgorithmEnterTimestamp    = timestampEnter;
            break;
        default:
            /* this should not be possible to happen */
            FAS_ASSERT(FAS_TRAP);
            break;
    }
}

static bool SYSM_ConvertRecordedTimingsToViolation(uint32_t duration, uint32_t timestampEnter) {
    bool returnValue = false;
    if ((duration != 0u) || (timestampEnter != 0u)) {
        returnValue = true;
    }
    return returnValue;
}

/*========== Extern Function Implementations ================================*/
STD_RETURN_TYPE_e SYSM_Init(void) {
    /* no need to check for the configuration as it is already checked with a
       static assert */

    /* read from FRAM interface the sys mon violations */
    (void)FRAM_ReadData(FRAM_BLOCK_ID_SYS_MON_RECORD);
    /* copy FRAM into local shadow copy */
    SYSM_CopyFramStruct(&fram_sys_mon_record, &sysm_localFramCopy);

    return STD_OK;
}

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

    for (SYSM_TASK_ID_e taskId = (SYSM_TASK_ID_e)0; taskId < SYSM_TASK_ID_MAX; taskId++) {
        if (sysm_ch_cfg[taskId].enable == SYSM_ENABLED) {
            /* check that the task gets called within its timing threshold plus jitter and
               then check that the tasks duration is shorter than tasks cycle time */
            time_since_last_call = local_timer - sysm_notifications[taskId].timestampEnter;
            max_allowed_jitter   = sysm_ch_cfg[taskId].cycleTime + sysm_ch_cfg[taskId].maxJitter;
            if ((time_since_last_call > max_allowed_jitter) &&
                (sysm_notifications[taskId].duration > sysm_ch_cfg[taskId].cycleTime)) {
                /* module not running within its timed limits */
                DIAG_Handler(DIAG_ID_SYSTEM_MONITORING, DIAG_EVENT_NOT_OK, DIAG_SYSTEM, (uint32_t)taskId);
                if (sysm_ch_cfg[taskId].enableRecording == SYSM_RECORDING_ENABLED) {
                    SYSM_RecordTimingViolation(
                        taskId, sysm_notifications[taskId].duration, sysm_notifications[taskId].timestampEnter);
                }
                sysm_ch_cfg[taskId].callbackFunction(taskId);
            }
        }
    }
}

void SYSM_Notify(SYSM_TASK_ID_e taskId, SYSM_NOTIFY_TYPE_e state, uint32_t timestamp) {
    FAS_ASSERT(taskId < SYSM_TASK_ID_MAX);
    FAS_ASSERT((state == SYSM_NOTIFY_ENTER) || (state == SYSM_NOTIFY_EXIT));
    OS_EnterTaskCritical();
    sysm_notifications[taskId].state = state;
    if (state == SYSM_NOTIFY_ENTER) {
        sysm_notifications[taskId].timestampEnter = timestamp;
    } else if (state == SYSM_NOTIFY_EXIT) {
        sysm_notifications[taskId].timestampExit = timestamp;
        sysm_notifications[taskId].duration      = timestamp - sysm_notifications[taskId].timestampEnter;
    } else {
        /* state has an illegal value */
        FAS_ASSERT(FAS_TRAP);
    }
    OS_ExitTaskCritical();
}

extern void SYSM_GetRecordedTimingViolations(SYSM_TIMING_VIOLATION_RESPONSE_s *pAnswer) {
    FAS_ASSERT(pAnswer != NULL_PTR);

    OS_EnterTaskCritical();
    FRAM_SYS_MON_RECORD_s localSysMonRecord = {0};
    SYSM_CopyFramStruct(&fram_sys_mon_record, &localSysMonRecord);
    OS_ExitTaskCritical();

    if (localSysMonRecord.anyTimingIssueOccurred == false) {
        /* no timing violations recorded, abort */
        pAnswer->recordedViolationAny       = false;
        pAnswer->recordedViolationEngine    = false;
        pAnswer->recordedViolation1ms       = false;
        pAnswer->recordedViolation10ms      = false;
        pAnswer->recordedViolation100ms     = false;
        pAnswer->recordedViolation100msAlgo = false;
    } else {
        /* timing violation, find it */
        pAnswer->recordedViolationAny    = true;
        pAnswer->recordedViolationEngine = SYSM_ConvertRecordedTimingsToViolation(
            localSysMonRecord.taskEngineViolatingDuration, localSysMonRecord.taskEngineEnterTimestamp);
        pAnswer->recordedViolation1ms = SYSM_ConvertRecordedTimingsToViolation(
            localSysMonRecord.task1msViolatingDuration, localSysMonRecord.task1msEnterTimestamp);
        pAnswer->recordedViolation10ms = SYSM_ConvertRecordedTimingsToViolation(
            localSysMonRecord.task10msViolatingDuration, localSysMonRecord.task10msEnterTimestamp);
        pAnswer->recordedViolation100ms = SYSM_ConvertRecordedTimingsToViolation(
            localSysMonRecord.task100msViolatingDuration, localSysMonRecord.task100msEnterTimestamp);
        pAnswer->recordedViolation100msAlgo = SYSM_ConvertRecordedTimingsToViolation(
            localSysMonRecord.task100msAlgorithmViolatingDuration, localSysMonRecord.task100msAlgorithmEnterTimestamp);
    }
}

extern void SYSM_ClearAllTimingViolations(void) {
    /* clear all diag entries */
    for (SYSM_TASK_ID_e taskId = (SYSM_TASK_ID_e)0; taskId < SYSM_TASK_ID_MAX; taskId++) {
        DIAG_Handler(DIAG_ID_SYSTEM_MONITORING, DIAG_EVENT_OK, DIAG_SYSTEM, (uint32_t)taskId);
    }
    /* clear local FRAM copy */
    OS_EnterTaskCritical();
    sysm_localFramCopy.anyTimingIssueOccurred              = false;
    sysm_localFramCopy.taskEngineEnterTimestamp            = 0u;
    sysm_localFramCopy.taskEngineViolatingDuration         = 0u;
    sysm_localFramCopy.task1msEnterTimestamp               = 0u;
    sysm_localFramCopy.task1msViolatingDuration            = 0u;
    sysm_localFramCopy.task10msEnterTimestamp              = 0u;
    sysm_localFramCopy.task10msViolatingDuration           = 0u;
    sysm_localFramCopy.task100msEnterTimestamp             = 0u;
    sysm_localFramCopy.task100msViolatingDuration          = 0u;
    sysm_localFramCopy.task100msAlgorithmEnterTimestamp    = 0u;
    sysm_localFramCopy.task100msAlgorithmViolatingDuration = 0u;
    sysm_flagFramCopyHasChanges                            = true;
    OS_ExitTaskCritical();
    /* commit to FRAM */
    SYSM_UpdateFramData();
}

extern void SYSM_UpdateFramData(void) {
    OS_EnterTaskCritical();
    const bool updateNecessary = sysm_flagFramCopyHasChanges;
    OS_ExitTaskCritical();
    if (updateNecessary == true) {
        OS_EnterTaskCritical();
        SYSM_CopyFramStruct(&sysm_localFramCopy, &fram_sys_mon_record);
        sysm_flagFramCopyHasChanges = false;
        OS_ExitTaskCritical();

        FRAM_WriteData(FRAM_BLOCK_ID_SYS_MON_RECORD);
    }
}

extern void SYSM_CopyFramStruct(const FRAM_SYS_MON_RECORD_s *const kpkFrom, FRAM_SYS_MON_RECORD_s *pTo) {
    FAS_ASSERT(kpkFrom != NULL_PTR);
    FAS_ASSERT(pTo != NULL_PTR);

    /* copy all elements in order to make sure that we are always compatible (independently of C-version) */
    pTo->anyTimingIssueOccurred              = kpkFrom->anyTimingIssueOccurred;
    pTo->task100msAlgorithmEnterTimestamp    = kpkFrom->task100msAlgorithmEnterTimestamp;
    pTo->task100msAlgorithmViolatingDuration = kpkFrom->task100msAlgorithmViolatingDuration;
    pTo->task100msEnterTimestamp             = kpkFrom->task100msEnterTimestamp;
    pTo->task100msViolatingDuration          = kpkFrom->task100msViolatingDuration;
    pTo->task10msEnterTimestamp              = kpkFrom->task10msEnterTimestamp;
    pTo->task10msViolatingDuration           = kpkFrom->task10msViolatingDuration;
    pTo->task1msEnterTimestamp               = kpkFrom->task1msEnterTimestamp;
    pTo->task1msViolatingDuration            = kpkFrom->task1msViolatingDuration;
    pTo->taskEngineEnterTimestamp            = kpkFrom->taskEngineEnterTimestamp;
    pTo->taskEngineViolatingDuration         = kpkFrom->taskEngineViolatingDuration;
}

/*========== Getter for static Variables (Unit Test) ========================*/
#ifdef UNITY_UNIT_TEST
extern SYSM_NOTIFICATION_s *TEST_SYSM_GetNotifications(void) {
    return sysm_notifications;
}
#endif

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
