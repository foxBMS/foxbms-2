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
 * @file    algorithm_cfg.h
 * @author  foxBMS Team
 * @date    2017-12-18 (date of creation)
 * @updated 2026-04-20 (date of last update)
 * @version v1.11.0
 * @ingroup ALGORITHMS_CONFIGURATION
 * @prefix  ALGO
 *
 * @brief   Headers for the configuration of the algorithm module
 * @details TODO
 * @requirements REQ-001, REQ-002, REQ-010, REQ-015
 */

#ifndef FOXBMS__ALGORITHM_CFG_H_
#define FOXBMS__ALGORITHM_CFG_H_

/*========== Includes =======================================================*/
#include "fstd_types.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/
/**
 * @ingroup ALGORITHMS_CONFIGURATION
 * @details task time slot where the ALGO main function is called. Repetition
 *          time of algorithm cycle time must be multiple of this
 */
#define ALGO_TICK_ms (100u) /**< REQ-010: 算法调度时间基准 (ms) */

/**
 * function type for the initialization function of an algorithm
 * @return  #STD_OK if the initialization has passed
 */
typedef STD_RETURN_TYPE_e ALGO_INITIALIZATION_FUNCTION_f(void);

/**
 * function type for the computation function of an algorithm
 */
typedef void ALGO_COMPUTATION_FUNCTION_f(void);

/** states that an algorithm can take */
typedef enum {
    ALGO_UNINITIALIZED,    /*!< REQ-001: 默认值，初始化尚未执行 */
    ALGO_READY,            /*!< REQ-001: 算法就绪，可在下一 Tick 执行 */
    ALGO_RUNNING,          /*!< REQ-001: 算法正在执行中。
    Note that it may not spend more than #ALGO_TASKS_s::maxCalculationDuration_ms in this state. */
    ALGO_BLOCKED,          /*!< REQ-001: 算法超出最大执行时长，已阻塞 */
    ALGO_FAILED_INIT,      /*!< REQ-001: 初始化失败 */
    ALGO_REINIT_REQUESTED, /*!< REQ-001: 已请求重新初始化 */
} ALGO_STATE_e;

/** Struct representing the key parameters of an algorithm */
typedef struct {
    ALGO_STATE_e state;                               /*!< REQ-002: 当前执行状态 */
    uint32_t cycleTime_ms;                            /*!< REQ-002: 算法执行周期 (ms)，0 表示 ASAP */
    uint32_t maxCalculationDuration_ms;               /*!< REQ-002: 最大允许执行时长 (ms) */
    uint32_t startTime;                               /*!< REQ-002: 算法启动执行时的时间戳 */
    ALGO_INITIALIZATION_FUNCTION_f *fpInitialization; /*!< REQ-002: 初始化回调函数指针;
        set to #NULL_PTR if not needed; return #STD_OK if init successful */
    ALGO_COMPUTATION_FUNCTION_f *fpAlgorithm;         /*!< REQ-002: 计算回调函数指针 */
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

/**
 * @brief   mark a algorithm as requiring a reinitialization.
 * @param[in]   algorithmIndex  index entry of the algorithm
 */
extern void ALGO_MarkAsReinit(uint32_t algorithmIndex);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
#endif

#endif /* FOXBMS__ALGORITHM_CFG_H_ */
