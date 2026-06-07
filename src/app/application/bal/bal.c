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
 * @file    bal.c
 * @author  foxBMS Team
 * @date    2020-02-24 (date of creation)
 * @updated 2026-04-20 (date of last update)
 * @version v1.11.0
 * @ingroup APPLICATION
 * @prefix  BAL
 *
 * @brief   Driver for the Balancing module
 * @details 电池均衡模块（BAL）核心实现。包含状态机基础操作：状态历史保存、
 *          重入保护、状态请求转移与校验、初始化及各初始化阶段的状态处理。
 *
 * @see     BAL_SOFTWARE_REQUIREMENTS.md  软件需求规格说明
 * @see     BAL_REQUIREMENT_TRACEABILITY_MAPPING.md  需求追溯矩阵
 */

/*========== Includes =======================================================*/
#include "bal.h"

#include <stdbool.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/

/**
 * @brief   保存上一状态和子状态
 * @req     FR-3.1.1 — 保存上一状态与子状态
 * @details 当当前状态与上一状态不同时，同时更新 lastState 和 lastSubstate；
 *          当仅子状态发生变化时，仅更新 lastSubstate；否则不做更新。
 * @param   pBalancingState 指向均衡状态结构体的指针（不能为 NULL）
 * @pre     pBalancingState != NULL（通过 FAS_ASSERT 强制校验）
 * @post    lastState 反映最近一次状态变化时的状态值
 * @post    lastSubstate 反映最近一次状态或子状态变化时的子状态值
 * @note    被均衡策略模块调用，用于追踪状态变迁历史
 */
extern void BAL_SaveLastStates(BAL_STATE_s *pBalancingState) {
    FAS_ASSERT(pBalancingState != NULL_PTR); /* @req NFR-4.3.1 — 空指针保护 */
    if (pBalancingState->lastState != pBalancingState->state) {
        pBalancingState->lastState    = pBalancingState->state;
        pBalancingState->lastSubstate = pBalancingState->substate;
    } else if (pBalancingState->lastSubstate != pBalancingState->substate) {
        pBalancingState->lastSubstate = pBalancingState->substate;
    } else {
        /* Do not set new substate as nothing changed */
        ;
    }
}

/**
 * @brief   重入检查
 * @req     FR-3.2.1 — 重入检查
 * @details 通过 triggerEntry 计数器检测 BAL_Trigger() 是否已被其他执行上下文调用。
 *          在任务临界区内进行原子检查与递增操作。
 * @param   currentState 指向当前状态结构体的指针（不能为 NULL）
 * @return  0: 正常进入（triggerEntry 已递增）
 * @return  0xFF: 检测到重入，调用方应放弃本次执行
 * @pre     currentState != NULL（通过 FAS_ASSERT 强制校验）
 * @note    此函数不可重入，应在定时触发或事件触发下调用
 */
extern uint8_t BAL_CheckReEntrance(BAL_STATE_s *currentState) {
    FAS_ASSERT(currentState != NULL_PTR); /* @req NFR-4.3.1 — 空指针保护 */
    uint8_t retval = 0;

    OS_EnterTaskCritical(); /* @req NFR-4.3.3 — 临界区保护：读-修改-写操作 */
    if (!currentState->triggerEntry) {
        currentState->triggerEntry++;
    } else {
        retval = 0xFF; /* multiple calls of function */
    }
    OS_ExitTaskCritical();

    return retval;
}

/**
 * @brief   转移状态请求
 * @req     FR-3.3.1 — 状态请求转移
 * @details 从当前状态结构体中读取待处理的状态请求，在临界区内将其原子地重置为
 *          BAL_STATE_NO_REQUEST，实现请求的原子转移。
 * @param   currentState 指向当前状态结构体的指针（不能为 NULL）
 * @return  转移前的原始状态请求值
 * @pre     currentState != NULL（通过 FAS_ASSERT 强制校验）
 * @post    currentState->stateRequest == BAL_STATE_NO_REQUEST
 * @note    此操作在任务临界区内执行，保证原子性
 */
extern BAL_STATE_REQUEST_e BAL_TransferStateRequest(BAL_STATE_s *currentState) {
    FAS_ASSERT(currentState != NULL_PTR); /* @req NFR-4.3.1 — 空指针保护 */
    BAL_STATE_REQUEST_e retval = BAL_STATE_NO_REQUEST;

    OS_EnterTaskCritical(); /* @req NFR-4.3.3 — 临界区保护：原子读取并重置 */
    retval                     = currentState->stateRequest;
    currentState->stateRequest = BAL_STATE_NO_REQUEST;
    OS_ExitTaskCritical();

    return retval;
}

/**
 * @brief   校验状态请求合法性
 * @req     FR-3.4.1 — 直接通过类请求校验
 * @req     FR-3.4.2 — 初始化请求校验
 * @details 分两级检查状态请求：
 *          1. 直接通过类（ERROR/GLOBAL_ENABLE/GLOBAL_DISABLE/NO_BALANCING/
 *             ALLOW_BALANCING）：始终合法，无需检查当前状态；
 *          2. 初始化请求（INIT_REQUEST）：仅当无待处理请求且当前为
 *             BAL_FSM_UNINITIALIZED 状态时合法。
 * @param   pCurrentState 指向当前状态结构体的指针（不能为 NULL）
 * @param   stateRequest  待校验的状态请求
 * @return  BAL_OK: 请求合法
 * @return  BAL_REQUEST_PENDING: 存在待处理请求
 * @return  BAL_ALREADY_INITIALIZED: 状态机已初始化，拒绝重复初始化
 * @return  BAL_ILLEGAL_REQUEST: 在当前状态下该请求非法
 * @pre     pCurrentState != NULL（通过 FAS_ASSERT 强制校验）
 * @post    全局使能/禁用请求会立即修改 balancingGlobalAllowed 标志
 */
extern BAL_RETURN_TYPE_e BAL_CheckStateRequest(BAL_STATE_s *pCurrentState, BAL_STATE_REQUEST_e stateRequest) {
    FAS_ASSERT(pCurrentState != NULL_PTR); /* @req NFR-4.3.1 — 空指针保护 */

    /* @req FR-3.4.1 — 直接通过类请求：ERROR_REQUEST 始终合法 */
    if (stateRequest == BAL_STATE_ERROR_REQUEST) {
        return BAL_OK;
    }
    /* @req FR-3.4.1 — 直接通过类请求：GLOBAL_ENABLE_REQUEST 设置标志后通过 */
    if (stateRequest == BAL_STATE_GLOBAL_ENABLE_REQUEST) {
        pCurrentState->balancingGlobalAllowed = true;
        return BAL_OK;
    }
    /* @req FR-3.4.1 — 直接通过类请求：GLOBAL_DISABLE_REQUEST 清除标志后通过 */
    if (stateRequest == BAL_STATE_GLOBAL_DISABLE_REQUEST) {
        pCurrentState->balancingGlobalAllowed = false;
        return BAL_OK;
    }
    /* @req FR-3.4.1 — 直接通过类请求：NO_BALANCING / ALLOW_BALANCING 始终合法 */
    if ((stateRequest == BAL_STATE_NO_BALANCING_REQUEST) || (stateRequest == BAL_STATE_ALLOW_BALANCING_REQUEST)) {
        return BAL_OK;
    }

    /* @req FR-3.4.2 — 初始化请求校验：需要无待处理请求 */
    if (pCurrentState->stateRequest == BAL_STATE_NO_REQUEST) {
        /* @req FR-3.4.2 — 初始化仅允许从未初始化状态发起 */
        if (stateRequest == BAL_STATE_INIT_REQUEST) {
            if (pCurrentState->state == BAL_FSM_UNINITIALIZED) {
                return BAL_OK;
            } else {
                return BAL_ALREADY_INITIALIZED;
            }
            /* request to forbid balancing */
        } else {
            return BAL_ILLEGAL_REQUEST;
        }
    } else {
        return BAL_REQUEST_PENDING;
    }
}

/**
 * @brief   均衡模块通用初始化
 * @req     FR-3.5.1 — 均衡模块初始化
 * @details 从数据库读取均衡控制数据块，设置 enableBalancing = false 禁用均衡，
 *          并将数据写回数据库。
 * @param   pControl 指向均衡控制数据块的指针（不能为 NULL）
 * @return  STD_OK: 初始化成功
 * @pre     pControl != NULL（通过 FAS_ASSERT 强制校验）
 * @post    enableBalancing == false（均衡功能已禁用）
 * @post    数据已通过 DATA_WRITE_DATA() 同步回数据库
 */
extern STD_RETURN_TYPE_e BAL_Init(DATA_BLOCK_BALANCING_CONTROL_s *pControl) {
    FAS_ASSERT(pControl != NULL_PTR); /* @req NFR-4.3.1 — 空指针保护 */
    DATA_READ_DATA(pControl);
    pControl->enableBalancing = false;
    DATA_WRITE_DATA(pControl);
    return STD_OK;
}

/**
 * @brief   处理未初始化状态
 * @req     FR-3.6.1 — 未初始化状态处理
 * @details 在 BAL_FSM_UNINITIALIZED 状态下处理传入的状态请求：
 *          - INIT_REQUEST: 转换到 BAL_FSM_INITIALIZATION 状态，设置 100ms 定时器
 *          - NO_REQUEST: 不做任何操作
 *          - 其他: 递增 errorRequestCounter（非法请求计数器）
 * @param   pCurrentState 指向当前状态结构体的指针（不能为 NULL）
 * @param   stateRequest  传入的状态请求
 * @pre     pCurrentState != NULL（通过 FAS_ASSERT 强制校验）
 * @post    若请求为初始化：状态机进入 BAL_FSM_INITIALIZATION（子状态 BAL_ENTRY）
 * @post    若请求非法：errorRequestCounter 递增
 */
extern void BAL_ProcessStateUninitialized(BAL_STATE_s *pCurrentState, BAL_STATE_REQUEST_e stateRequest) {
    FAS_ASSERT(pCurrentState != NULL_PTR); /* @req NFR-4.3.1 — 空指针保护 */
    if (stateRequest == BAL_STATE_INIT_REQUEST) {
        /* @req NFR-4.1.1 — 状态机触发周期：短延时 100ms */
        pCurrentState->timer    = BAL_FSM_SHORTTIME_100ms;
        pCurrentState->state    = BAL_FSM_INITIALIZATION;
        pCurrentState->substate = BAL_ENTRY;
    } else if (stateRequest == BAL_STATE_NO_REQUEST) {
        /* no actual request pending */
    } else {
        pCurrentState->errorRequestCounter++; /* illegal request pending */
    }
}

/**
 * @brief   处理初始化状态
 * @req     FR-3.6.2 — 初始化状态处理
 * @details 在 BAL_FSM_INITIALIZATION 状态下直接转换到 BAL_FSM_INITIALIZED 状态，
 *          设置 100ms 定时器，下一触发周期进入已初始化状态处理。
 * @param   currentState 指向当前状态结构体的指针（不能为 NULL）
 * @pre     currentState != NULL（通过 FAS_ASSERT 强制校验）
 * @post    状态机进入 BAL_FSM_INITIALIZED（子状态 BAL_ENTRY）
 * @post    timer = BAL_FSM_SHORTTIME_100ms（100ms 后触发下一状态）
 */
extern void BAL_ProcessStateInitialization(BAL_STATE_s *currentState) {
    FAS_ASSERT(currentState != NULL_PTR); /* @req NFR-4.3.1 — 空指针保护 */
    /* @req NFR-4.1.1 — 状态机触发周期：短延时 100ms */
    currentState->timer    = BAL_FSM_SHORTTIME_100ms;
    currentState->state    = BAL_FSM_INITIALIZED;
    currentState->substate = BAL_ENTRY;
}

/**
 * @brief   处理已初始化状态
 * @req     FR-3.6.3 — 已初始化状态处理
 * @details 在 BAL_FSM_INITIALIZED 状态下完成初始化确认：
 *          设置 initializationFinished = STD_OK，并转换到均衡检查状态
 *          BAL_FSM_CHECK_BALANCING。
 * @param   currentState 指向当前状态结构体的指针（不能为 NULL）
 * @pre     currentState != NULL（通过 FAS_ASSERT 强制校验）
 * @post    initializationFinished == STD_OK（外部可通过 BAL_GetInitializationState() 查询）
 * @post    状态机进入 BAL_FSM_CHECK_BALANCING（子状态 BAL_ENTRY）
 * @note    此后状态机进入运行主循环（检查均衡 → 均衡/禁止均衡）
 */
extern void BAL_ProcessStateInitialized(BAL_STATE_s *currentState) {
    FAS_ASSERT(currentState != NULL_PTR); /* @req NFR-4.3.1 — 空指针保护 */
    currentState->initializationFinished = STD_OK;
    /* @req NFR-4.1.1 — 状态机触发周期：短延时 100ms */
    currentState->timer                  = BAL_FSM_SHORTTIME_100ms;
    currentState->state                  = BAL_FSM_CHECK_BALANCING;
    currentState->substate               = BAL_ENTRY;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
/** @req NFR-4.5.1 — 单元测试支持：通过条件编译暴露内部函数 */
#ifdef UNITY_UNIT_TEST
#endif
