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
 * @file    os.c
 * @author  foxBMS Team
 * @date    2019-08-27 (date of creation)
 * @updated 2021-12-01 (date of last update)
 * @ingroup OS
 * @prefix  OS
 *
 * @brief   Implementation of the tasks and resources used by the system
 *
 */

/*========== Includes =======================================================*/
#include "os.h"

#include "ftask.h"

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/** timer counter since the 1ms task has started */
static OS_TIMER_s os_timer = {0u, 0u, 0u, 0u, 0u, 0u, 0u};

/*========== Extern Constant and Variable Definitions =======================*/
/** boot state of the OS */
volatile OS_BOOT_STATE_e os_boot = OS_OFF;
/** timestamp of the scheduler start */
uint32_t os_schedulerStartTime = 0u;

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/

void OS_InitializeOperatingSystem(void) {
    /* operating system configuration (Queues, Tasks) */
    os_boot = OS_CREATE_QUEUES;
    FTSK_CreateQueues();
    os_boot = OS_CREATE_TASKS;
    FTSK_CreateTasks();
    os_boot = OS_INIT_PRE_OS;
}

/* AXIVION Disable Style Generic-MaxNesting: The program flow is simple
   although it exceeds the maximum nesting level and changing the
   implementation would not be beneficial for understanding the function. */
extern void OS_IncrementTimer(void) {
    if (++os_timer.timer_1ms > 9u) { /* 10ms */
        os_timer.timer_1ms = 0u;
        if (++os_timer.timer_10ms > 9u) { /* 100ms */
            os_timer.timer_10ms = 0u;
            if (++os_timer.timer_100ms > 9u) { /* 1s */
                os_timer.timer_100ms = 0u;
                if (++os_timer.timer_sec > 59u) { /* 1min */
                    os_timer.timer_sec = 0u;
                    if (++os_timer.timer_min > 59u) { /* 1h */
                        os_timer.timer_min = 0u;
                        if (++os_timer.timer_h > 23u) { /* 1d */
                            os_timer.timer_h = 0u;
                            ++os_timer.timer_d;
                        }
                    }
                }
            }
        }
    }
}
/* AXIVION Enable Style Generic-MaxNesting: */

/* AXIVION Next Line Style Generic-MissingParameterAssert: The function is designed and tested for full range */
extern bool OS_CheckTimeHasPassedWithTimestamp(
    uint32_t oldTimeStamp_ms,
    uint32_t currentTimeStamp_ms,
    uint32_t timeToPass_ms) {
    /* AXIVION Next Line Style MisraC2012Directive-4.1: correct handling of underflows is checked with unit tests */
    const uint32_t timeDifference_ms = currentTimeStamp_ms - oldTimeStamp_ms;
    bool timehasPassed               = false;

    if (timeToPass_ms == 0u) {
        timehasPassed = true;
    } else if (timeDifference_ms == 0u) {
        /* case timeToPass_ms is 0u has already been extracted
           therefore, only the cases where full UINT32_MAX time has passed
           remain. By definition we will assume in this case that no time has
           passed between these two timestamps and will therefore return false.
         */
    } else if (timeDifference_ms >= timeToPass_ms) {
        timehasPassed = true;
    } else {
        /* timehasPassed is already false, nothing to do */
    }

    return timehasPassed;
}

/* AXIVION Next Line Style Generic-MissingParameterAssert: The function is designed and tested for full range */
extern bool OS_CheckTimeHasPassed(uint32_t oldTimeStamp_ms, uint32_t timeToPass_ms) {
    return OS_CheckTimeHasPassedWithTimestamp(oldTimeStamp_ms, OS_GetTickCount(), timeToPass_ms);
}

extern STD_RETURN_TYPE_e OS_CheckTimeHasPassedSelfTest(void) {
    STD_RETURN_TYPE_e selfCheckReturnValue = STD_OK;

    /* AXIVION Next Line Style MisraC2012-2.2 MisraC2012-14.3: If the code works as expected, this self test function is expected to always return the same value */
    if (OS_CheckTimeHasPassedWithTimestamp(0u, 0u, 0u) != true) {
        /* AXIVION Next Line Style FaultDetection-UnusedAssignments: All cases collected, using each not necessary. */
        selfCheckReturnValue = STD_NOT_OK;
    }

    if (OS_CheckTimeHasPassedWithTimestamp(0u, 1u, 1u) != true) {
        /* AXIVION Next Line Style FaultDetection-UnusedAssignments: All cases collected, using each not necessary. */
        selfCheckReturnValue = STD_NOT_OK;
    }

    if (OS_CheckTimeHasPassedWithTimestamp(1u, 2u, 2u) != false) {
        /* AXIVION Next Line Style FaultDetection-UnusedAssignments: All cases collected, using each not necessary. */
        selfCheckReturnValue = STD_NOT_OK;
    }

    if (OS_CheckTimeHasPassedWithTimestamp(0u, 1u, 0u) != true) {
        /* AXIVION Next Line Style FaultDetection-UnusedAssignments: All cases collected, using each not necessary. */
        selfCheckReturnValue = STD_NOT_OK;
    }

    if (OS_CheckTimeHasPassedWithTimestamp(1u, 0u, 1u) != true) {
        selfCheckReturnValue = STD_NOT_OK;
    }

    return selfCheckReturnValue;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
extern OS_TIMER_s *TEST_OS_GetOsTimer() {
    return &os_timer;
}
#endif
