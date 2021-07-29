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
 * @file    algorithm.c
 * @author  foxBMS Team
 * @date    2017-12-18 (date of creation)
 * @updated 2020-06-30 (date of last update)
 * @ingroup ALGORITHMS
 * @prefix  ALGO
 *
 * @brief   Main module to handle the execution of algorithms
 *
 *
 */

/*========== Includes =======================================================*/
#include "algorithm.h"

#include "os.h"

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/
/**
 * This is a signal that skips initialization of #ALGO_Initialization()
 * until it has been requested.
 */
static bool algo_initializationRequested = false;

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/
/**
 * @brief   initializes local variables and module internals needed to use the
 *          algorithm module
 */
static void ALGO_Initialization(void);

/*========== Static Function Implementations ================================*/
static void ALGO_Initialization(void) {
    /* iterate over all algorithms */
    for (uint16_t i = 0u; i < algo_length; i++) {
        /* check if the cycle time is valid */
        FAS_ASSERT((algo_algorithms[i].cycleTime_ms % ALGO_TICK_MS) == 0u);

        /* check only uninitialized algorithms */
        if (ALGO_UNINITIALIZED == algo_algorithms[i].state) {
            /* directly make ready when init function is a null pointer otherwise run init */
            if (NULL_PTR == algo_algorithms[i].fpInitialization) {
                algo_algorithms[i].state = ALGO_READY;
            } else {
                const STD_RETURN_TYPE_e result = algo_algorithms[i].fpInitialization();
                FAS_ASSERT((STD_OK == result) || (STD_NOT_OK == result));
                if (STD_OK == result) {
                    algo_algorithms[i].state = ALGO_READY;
                } else {
                    algo_algorithms[i].state = ALGO_FAILED_INIT;
                }
            }
        }
    }
}

/*========== Extern Function Implementations ================================*/

extern void ALGO_UnlockInitialization(void) {
    OS_EnterTaskCritical();
    algo_initializationRequested = true;
    OS_ExitTaskCritical();
}

extern void ALGO_MainFunction(void) {
    OS_EnterTaskCritical();
    const bool initializationRequested = algo_initializationRequested;
    OS_ExitTaskCritical();
    if (true == initializationRequested) {
        ALGO_Initialization();
        OS_EnterTaskCritical();
        algo_initializationRequested = false;
        OS_ExitTaskCritical();
    }

    static uint32_t counter_ticks = 0u;

    for (uint16_t i = 0u; i < algo_length; i++) {
        const bool runAlgorithmAsap = (algo_algorithms[i].cycleTime_ms == 0u);
        const bool runAlgorithmCycleElapsed =
            ((algo_algorithms[i].cycleTime_ms != 0u) && ((counter_ticks % algo_algorithms[i].cycleTime_ms) == 0u));
        if ((runAlgorithmAsap != false) || (runAlgorithmCycleElapsed != false)) {
            /* Cycle time elapsed -> call function */
            if (ALGO_READY == algo_algorithms[i].state) {
                /* Set state to running -> reset to READY before leaving algo function */
                algo_algorithms[i].state     = ALGO_RUNNING;
                algo_algorithms[i].startTime = OS_GetTickCount();
                algo_algorithms[i].fpAlgorithm();
                ALGO_MarkAsDone(i);
            }
        }
    }

    counter_ticks += ALGO_TICK_MS;
}

extern void ALGO_MonitorExecutionTime(void) {
    const uint32_t timestamp = OS_GetTickCount();

    for (uint16_t i = 0u; i < algo_length; i++) {
        if ((algo_algorithms[i].startTime != 0u) && (ALGO_RUNNING == algo_algorithms[i].state) &&
            ((algo_algorithms[i].startTime + algo_algorithms[i].maxCalculationDuration_ms) < timestamp)) {
            /* Block task from further execution because of runtime violation, but task will finish its execution */
            algo_algorithms[i].state = ALGO_BLOCKED;

            /* TODO: Add diag call to notify error in algorithm module */
        }
    }
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
extern void TEST_ALGO_ResetInitializationRequest() {
    algo_initializationRequested = false;
}
#endif /* UNITY_UNIT_TEST */
