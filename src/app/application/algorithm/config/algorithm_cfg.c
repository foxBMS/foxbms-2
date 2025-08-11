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
 * @file    algorithm_cfg.c
 * @author  foxBMS Team
 * @date    2017-12-18 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup ALGORITHMS_CONFIGURATION
 * @prefix  ALGO
 *
 * @brief   Configuration for the algorithm module
 * @details TODO
 */

/*========== Includes =======================================================*/
#include "algorithm_cfg.h"

#include "fassert.h"
#include "moving_average.h"
#include "os.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/
/** array of algorithms that should be executed */
ALGO_TASKS_s algo_algorithms[] = {
    {ALGO_UNINITIALIZED, 100, 1000, 0, NULL_PTR, &ALGO_MovingAverage},
};

const uint16_t algo_length = sizeof(algo_algorithms) / sizeof(algo_algorithms[0]);

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/

extern void ALGO_MarkAsDone(uint32_t algorithmIndex) {
    FAS_ASSERT(algorithmIndex < algo_length);
    if (algo_algorithms[algorithmIndex].state == ALGO_REINIT_REQUESTED) {
        /* do not alter state if a reinitialize request is pending */
    } else if (algo_algorithms[algorithmIndex].state != ALGO_BLOCKED) {
        algo_algorithms[algorithmIndex].state = ALGO_READY;
    } else {
        /* algorithm is in "blocked" state, nothing to do here */
    }
}

extern void ALGO_MarkAsReinit(uint32_t algorithmIndex) {
    FAS_ASSERT(algorithmIndex < algo_length);
    OS_EnterTaskCritical();
    algo_algorithms[algorithmIndex].state = ALGO_REINIT_REQUESTED;
    OS_ExitTaskCritical();
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
