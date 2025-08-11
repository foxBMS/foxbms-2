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
 * @file    sys_mon.h
 * @author  foxBMS Team
 * @date    2019-11-28 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup ENGINE
 * @prefix  SYSM
 *
 * @brief   System monitoring module
 * @details TODO
 */

#ifndef FOXBMS__SYS_MON_H_
#define FOXBMS__SYS_MON_H_

/*========== Includes =======================================================*/
#include "sys_mon_cfg.h"

#include "fram.h"

#include <stdbool.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/
/** defines entry or exit */
typedef enum {
    SYSM_NOTIFY_ENTER, /**< entry into a task */
    SYSM_NOTIFY_EXIT,  /**< exit from a task */
} SYSM_NOTIFY_TYPE_e;

/** state (in summary) used for task or function notification */
typedef struct {
    SYSM_NOTIFY_TYPE_e state; /**< state of the task */
    uint32_t timestampEnter;  /**< timestamp of entry into state */
    uint32_t timestampExit;   /**< timestamp of exit from state */
    uint32_t duration;        /**< duration between last complete entry and exit cycle */
} SYSM_NOTIFICATION_s;

/** compact data format for reporting violation states */
typedef struct {
    bool recordedViolationAny;       /*!< compound flag indicating if any violation occurred */
    bool recordedViolationEngine;    /*!< flag indicating if a engine violation is recorded */
    bool recordedViolation1ms;       /*!< flag indicating if a 1ms violation is recorded */
    bool recordedViolation10ms;      /*!< flag indicating if a 10ms violation is recorded */
    bool recordedViolation100ms;     /*!< flag indicating if a 100ms violation is recorded */
    bool recordedViolation100msAlgo; /*!< flag indicating if a 100ms algorithm violation is recorded */
} SYSM_TIMING_VIOLATION_RESPONSE_s;

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/
/**
 * @brief   initialization function for system monitoring
 * @details Has to be called once after startup to initialize and check system
 *          monitoring related functionality
 * @return  #STD_OK if no configuration error detected, otherwise #STD_NOT_OK
 */
extern STD_RETURN_TYPE_e SYSM_Initialize(void);

/**
 * @brief   overall system monitoring
 * @details checks notifications (state and timestamps) of all system-relevant
 *          tasks or functions all checks should be customized corresponding
 *          to its timing and state requirements
 */
extern void SYSM_CheckNotifications(void);

/**
 * @brief   Sets needed bits to indicate that a task is running
 * @details Has to be called in every function using the system monitoring.
 * @param   taskId      task id to notify system monitoring
 * @param   state       whether function has been called at entry or exit
 * @param   timestamp   time
 */
extern void SYSM_Notify(SYSM_TASK_ID_e taskId, SYSM_NOTIFY_TYPE_e state, uint32_t timestamp);

/**
 * @brief   Returns the timing violation flags determined from fram state
 * @details This function reads the timing states and determines if a violation
 *          has occurred.
 * @param[out]  pAnswer pointer to a #SYSM_TIMING_VIOLATION_RESPONSE_s struct that will be filled by the function
 */
extern void SYSM_GetRecordedTimingViolations(SYSM_TIMING_VIOLATION_RESPONSE_s *pAnswer);

/**
 * @brief   Clears all timing violations (both current and recorded)
 * @details This function clears all indicators that a timing violation has
 *          happened.
 */
extern void SYSM_ClearAllTimingViolations(void);

/**
 * @brief   Commits the stored changes to FRAM if necessary
 * @details Writing to FRAM is costly (in terms of computation time), therefore
 *          it is decoupled from the main task of the sys mon module.
 *          When called, this function checks the flag sysm_flagFramCopyHasChanges
 *          and writes to FRAM if there are changes.
 */
extern void SYSM_UpdateFramData(void);

/**
 * @brief   Copy from the from entry to the to entry.
 *
 * @param[in]   kpkFrom pointer to the origin struct
 * @param[out]  pTo     pointer to the destination struct
 */
extern void SYSM_CopyFramStruct(const FRAM_SYS_MON_RECORD_s *const kpkFrom, FRAM_SYS_MON_RECORD_s *pTo);

/*========== Getter for static Variables (Unit Test) ========================*/
#ifdef UNITY_UNIT_TEST
extern SYSM_NOTIFICATION_s *TEST_SYSM_GetNotifications(void);
#endif

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
extern void TEST_SYSM_RecordTimingViolation(SYSM_TASK_ID_e taskId, uint32_t taskDuration, uint32_t timestampEnter);
/* Helper functions */
extern bool TEST_SYSM_GetStaticVariableFlagFramCopyHasChanges(void);
extern bool TEST_SYSM_SetStaticVariableFlagFramCopyHasChanges(bool value);
#endif

#endif /* FOXBMS__SYS_MON_H_ */
