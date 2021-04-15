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
 * @file    algorithm_cfg.h
 * @author  foxBMS Team
 * @date    2017-12-18 (date of creation)
 * @updated 2020-06-30 (date of last update)
 * @ingroup ALGORITHMS_CONFIGURATION
 * @prefix  ALGO
 *
 * @brief   Headers for the configuration of the algorithm module
 *
 */

#ifndef FOXBMS__ALGORITHM_CFG_H_
#define FOXBMS__ALGORITHM_CFG_H_

/*========== Includes =======================================================*/
#include "general.h"

/*========== Macros and Definitions =========================================*/
/**
 * @ingroup ALGORITHMS_CONFIGURATION
 * @details task timeslot where the ALGO main function is called. Repetition
 *          time of algorithm cycle time must be multiple of this
 */
#define ALGO_TICK_MS (100u)

/**
 * function type for the initialization function of an algorithm
 * @returns     #STD_OK if the initialization has passed
 */
typedef STD_RETURN_TYPE_e ALGO_INITIALIZATION_FUNCTION_f(void);

/**
 * function type for the computation function of an algorithm
 */
typedef void ALGO_COMPUTATION_FUNCTION_f(void);

/** states that an algorithm can take */
typedef enum ALGO_STATE {
    ALGO_UNINITIALIZED, /*!< This is the default value indicating that initialization has not run yet */
    ALGO_READY,         /*!< This indicates that the algorithm is ready to be run on next time slot */
    ALGO_RUNNING,       /*!< This indicates that the algorithm is currently running.
    Note that it may not spend more than #ALGO_TASKS::maxCalculationDuration_ms in this state. */
    ALGO_BLOCKED,       /*!< This indicates that the algorithm has violated its maximum calculation duration. */
    ALGO_FAILED_INIT,   /*!< This indicates a failed initialization. */
} ALGO_STATE_e;

/** Struct representing the key parameters of an algorithm */
typedef struct ALGO_TASKS {
    ALGO_STATE_e state;                               /*!< current execution state */
    uint32_t cycleTime_ms;                            /*!< cycle time of algorithm */
    uint32_t maxCalculationDuration_ms;               /*!< maximum allowed calculation duration for task */
    uint32_t startTime;                               /*!< start time when executing algorithm */
    ALGO_INITIALIZATION_FUNCTION_f *fpInitialization; /*!< callback function for init;
        set to #NULL_PTR if not needed; return #STD_OK if init successful */
    ALGO_COMPUTATION_FUNCTION_f *fpAlgorithm;         /*!< callback function */
} ALGO_TASKS_s;

/*========== Extern Constant and Variable Declarations ======================*/
/** Array with pointer to the different algorithms */
extern ALGO_TASKS_s algo_algorithms[];

/** number of executed algorithms */
extern const uint16_t algo_length;

/*========== Extern Function Prototypes =====================================*/
/**
 * @brief   Mark the current algorithm as done (will reset to #ALGO_READY if possible)
 * @param[in]   algorithmIndex  index entry of the algorithm
 */
extern void ALGO_MarkAsDone(uint32_t algorithmIndex);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/

#endif /* FOXBMS__ALGORITHM_CFG_H_ */
