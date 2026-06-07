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
 * @file    bal.h
 * @author  foxBMS Team
 * @date    2020-02-24 (date of creation)
 * @updated 2026-04-20 (date of last update)
 * @version v1.11.0
 * @ingroup APPLICATION
 * @prefix  BAL
 *
 * @brief   Header for the driver for balancing
 * @details 电池均衡模块（BAL）头文件。定义了状态机相关的枚举类型（状态、子状态、
 *          请求类型、返回值）、状态结构体以及所有公开 API 的函数原型。
 *
 * @see     BAL_SOFTWARE_REQUIREMENTS.md  软件需求规格说明
 * @see     BAL_REQUIREMENT_TRACEABILITY_MAPPING.md  需求追溯矩阵
 */

#ifndef FOXBMS__BAL_H_
#define FOXBMS__BAL_H_

/*========== Includes =======================================================*/
#include "bal_cfg.h"

#include "database.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Extern Constant and Variable Declarations ======================*/

/**
 * @req 第 6.2 节 — BAL_FSM_e：状态机状态枚举
 *
 * States of the BAL state machine
 *
 * 状态转移关系见 @ref BAL_SOFTWARE_REQUIREMENTS.md 第 3.7 节"BAL 状态机状态转移图"
 */
typedef enum {
    BAL_FSM_UNINITIALIZED,   /*!< 未初始化 — 上电初始状态 @req FR-3.6.1 */
    BAL_FSM_INITIALIZATION,  /*!< 初始化中 — 执行初始化序列 @req FR-3.6.2 */
    BAL_FSM_INITIALIZED,     /*!< 已初始化 — 初始化完成，准备进入运行 @req FR-3.6.3 */
    BAL_FSM_CHECK_BALANCING, /*!< 检查均衡 — 运行主循环入口 @req 第 3.6.3 节 */
    BAL_FSM_BALANCE,         /*!< 执行均衡 — 正在执行电池均衡 */
    BAL_FSM_NO_BALANCING,    /*!< 禁止均衡 — 条件不满足，禁止均衡 */
    BAL_FSM_ALLOW_BALANCING, /*!< 允许均衡 — 条件满足，允许均衡 */
    BAL_FSM_GLOBAL_DISABLE,  /*!< 全局禁用 — 全局均衡禁用 @req FR-3.4.1 */
    BAL_FSM_GLOBAL_ENABLE,   /*!< 全局使能 — 全局均衡使能 @req FR-3.4.1 */
    BAL_FSM_UNDEFINED,       /*!< undefined state */
    BAL_FSM_RESERVED1,       /*!< reserved state */
    BAL_FSM_ERROR,           /*!< 错误状态 — 任意状态下接收到 ERROR_REQUEST 进入 @req FR-3.4.1 */
} BAL_FSM_e;

/**
 * @req 第 6.2 节 — BAL_FSM_SUB_e：状态机子状态枚举
 *
 * Substates of the BAL state machine
 */
typedef enum {
    BAL_ENTRY,                /*!< 入口子状态 — 状态的入口点 */
    BAL_CHECK_IMBALANCES,     /*!< 检查不均衡 — 检查均衡是否已初始化 */
    BAL_COMPUTE_IMBALANCES,   /*!< 计算不均衡量 — 计算各单体间不均衡 */
    BAL_ACTIVATE_BALANCING,   /*!< 激活均衡电阻 — 激活均衡回路 */
    BAL_CHECK_LOWEST_VOLTAGE, /*!< 检查最低电压 — 检查最低单体电压是否高于下限 */
    BAL_CHECK_CURRENT,        /*!< 检查电流 — 检查电流是否低于上限 */
} BAL_FSM_SUB_e;

/**
 * @req 第 6.2 节 — BAL_STATE_REQUEST_e：状态请求枚举
 *
 * State requests for the BAL state machine
 *
 * 请求合法性校验参见 @ref BAL_CheckStateRequest() (FR-3.4.1 / FR-3.4.2)
 */
typedef enum {
    BAL_STATE_INIT_REQUEST,            /*!< 初始化请求 — 需要从 UNINITIALIZED 状态发起 @req FR-3.4.2 */
    BAL_STATE_ERROR_REQUEST,           /*!< 进入错误状态请求 — 始终合法 @req FR-3.4.1 */
    BAL_STATE_NO_BALANCING_REQUEST,    /*!< 禁止均衡请求 — 始终合法 @req FR-3.4.1 */
    BAL_STATE_ALLOW_BALANCING_REQUEST, /*!< 允许均衡请求 — 始终合法 @req FR-3.4.1 */
    BAL_STATE_GLOBAL_DISABLE_REQUEST,  /*!< 全局禁用请求 — 立即清除 balancingGlobalAllowed @req FR-3.4.1 */
    BAL_STATE_GLOBAL_ENABLE_REQUEST,   /*!< 全局使能请求 — 立即设置 balancingGlobalAllowed @req FR-3.4.1 */
    BAL_STATE_NO_REQUEST,              /*!< 无请求（默认） */
} BAL_STATE_REQUEST_e;

/**
 * @req 第 6.2 节 — BAL_RETURN_TYPE_e：状态请求返回类型枚举
 *
 * Possible return values when state requests are made to the BAL state machine
 */
typedef enum {
    BAL_OK,                  /*!< 操作成功 @req FR-3.4.1 / FR-3.4.2 */
    BAL_BUSY_OK,             /*!< 忙，但操作可继续 */
    BAL_REQUEST_PENDING,     /*!< 存在待处理请求，当前请求被拒绝 @req FR-3.4.2 */
    BAL_ILLEGAL_REQUEST,     /*!< 在当前状态下该请求非法 @req FR-3.4.2 */
    BAL_INIT_ERROR,          /*!< 初始化错误 */
    BAL_OK_FROM_ERROR,       /*!< 从错误中恢复 */
    BAL_ERROR,               /*!< 一般性错误 */
    BAL_ALREADY_INITIALIZED, /*!< 已初始化，拒绝重复初始化 @req FR-3.4.2 */
    BAL_ILLEGAL_TASK_TYPE,   /*!< 非法任务类型 */
} BAL_RETURN_TYPE_e;

/**
 * @req 第 6.1 节 — BAL_STATE_s：均衡状态结构体
 *
 * This structure contains all the variables relevant for the BAL state machine.
 * The user can get the current state of the BAL state machine with this variable
 *
 * @note 结构体共 13 个字段，保持紧凑以满足嵌入式系统要求 @req NFR-4.2.1
 */
typedef struct {
    /** 状态机定时器，单位为 100ms 计数 (@req NFR-4.1.1) */
    uint16_t timer;
    /** 当前待处理的状态请求 (@req FR-3.3.1) */
    BAL_STATE_REQUEST_e stateRequest;
    /** 状态机当前状态 (@req 第 3.7 节) */
    BAL_FSM_e state;
    /** 状态机当前子状态 */
    BAL_FSM_SUB_e substate;
    /** 上一状态 — 历史记录 (@req FR-3.1.1) */
    BAL_FSM_e lastState;
    /** 上一子状态 — 历史记录 (@req FR-3.1.1) */
    uint8_t lastSubstate;
    /** 重入保护计数器（0=未运行，非0=正在运行）(@req FR-3.2.1 / NFR-4.3.2) */
    uint8_t triggerEntry;
    /** 非法请求累计计数 (@req FR-3.6.1) */
    uint32_t errorRequestCounter;
    /** 初始化完成标志：STD_OK 表示已完成 (@req FR-3.6.3) */
    STD_RETURN_TYPE_e initializationFinished;
    /** 均衡是否激活 */
    bool active;
    /** 有效均衡阈值（mV）(@req NFR-4.3.4) */
    int32_t balancingThreshold;
    /** 是否允许均衡（局部标志）*/
    bool balancingAllowed;
    /** 是否全局允许均衡 (@req FR-3.4.1) */
    bool balancingGlobalAllowed;
} BAL_STATE_s;

/*========== Extern Function Prototypes =====================================*/

/**
 * @brief   保存上一状态和子状态
 * @req     FR-3.1.1 — 保存上一状态与子状态
 * @param   pBalancingState 指向均衡状态结构体的指针（不能为 NULL）
 * @pre     通过 FAS_ASSERT 进行空指针校验 @req NFR-4.3.1
 */
extern void BAL_SaveLastStates(BAL_STATE_s *pBalancingState);

/**
 * @brief   重入检查
 * @req     FR-3.2.1 — 重入检查
 * @details BAL 状态机触发函数的重入保护。递增 triggerEntry 计数器。
 *          此函数不可重入，应仅由定时或事件触发调用。
 *          若被两个不同进程同时调用，triggerEntry 应始终为 0。
 * @param   currentState 指向当前状态结构体的指针（不能为 NULL）
 * @return  0: 无其他实例活跃
 * @return  0xFF: 检测到重入
 * @pre     通过 FAS_ASSERT 进行空指针校验 @req NFR-4.3.1
 */
extern uint8_t BAL_CheckReEntrance(BAL_STATE_s *currentState);

/**
 * @brief   转移状态请求到状态机
 * @req     FR-3.3.1 — 状态请求转移
 * @details 从 currentState 中读取当前状态请求，然后将其重置为 BAL_STATE_NO_REQUEST。
 *          操作在临界区内执行以保证原子性。@req NFR-4.3.3
 * @param   currentState 指向当前状态结构体的指针（不能为 NULL）
 * @return  转移前的原始状态请求
 * @pre     通过 FAS_ASSERT 进行空指针校验 @req NFR-4.3.1
 */
extern BAL_STATE_REQUEST_e BAL_TransferStateRequest(BAL_STATE_s *currentState);

/**
 * @brief   校验状态请求合法性
 * @req     FR-3.4.1 — 直接通过类请求校验
 * @req     FR-3.4.2 — 初始化请求校验
 * @details 检查状态请求的有效性，结果立即返回。
 * @param   pCurrentState 指向当前状态结构体的指针（不能为 NULL）
 * @param   stateRequest  待校验的状态请求
 * @return  校验结果（BAL_OK / BAL_REQUEST_PENDING / BAL_ALREADY_INITIALIZED / BAL_ILLEGAL_REQUEST）
 * @pre     通过 FAS_ASSERT 进行空指针校验 @req NFR-4.3.1
 */
extern BAL_RETURN_TYPE_e BAL_CheckStateRequest(BAL_STATE_s *pCurrentState, BAL_STATE_REQUEST_e stateRequest);

/**
 * @brief   处理未初始化状态
 * @req     FR-3.6.1 — 未初始化状态处理
 * @details BAL_Trigger() 的子状态处理函数。处理 BAL_FSM_UNINITIALIZED 状态下的请求。
 * @param   pCurrentState 指向当前状态结构体的指针（不能为 NULL）
 * @param   stateRequest  待处理的状态请求
 * @pre     通过 FAS_ASSERT 进行空指针校验 @req NFR-4.3.1
 */
extern void BAL_ProcessStateUninitialized(BAL_STATE_s *pCurrentState, BAL_STATE_REQUEST_e stateRequest);

/**
 * @brief   处理初始化状态
 * @req     FR-3.6.2 — 初始化状态处理
 * @details BAL_Trigger() 的子状态处理函数。在 BAL_FSM_INITIALIZATION 状态下
 *          转换到 BAL_FSM_INITIALIZED。
 * @param   currentState 指向当前状态结构体的指针（不能为 NULL）
 * @pre     通过 FAS_ASSERT 进行空指针校验 @req NFR-4.3.1
 */
extern void BAL_ProcessStateInitialization(BAL_STATE_s *currentState);

/**
 * @brief   处理已初始化状态
 * @req     FR-3.6.3 — 已初始化状态处理
 * @details BAL_Trigger() 的子状态处理函数。从已初始化状态转换到运行状态
 *          （BAL_FSM_CHECK_BALANCING）。
 * @param   currentState 指向当前状态结构体的指针（不能为 NULL）
 * @pre     通过 FAS_ASSERT 进行空指针校验 @req NFR-4.3.1
 */
extern void BAL_ProcessStateInitialized(BAL_STATE_s *currentState);

/**
 * @brief   均衡模块通用初始化
 * @req     FR-3.5.1 — 均衡模块初始化
 * @param   pControl 指向均衡控制数据块的指针（不能为 NULL）
 * @return  STD_OK: 初始化成功
 * @pre     通过 FAS_ASSERT 进行空指针校验 @req NFR-4.3.1
 */
extern STD_RETURN_TYPE_e BAL_Init(DATA_BLOCK_BALANCING_CONTROL_s *pControl);

/**
 * @brief   设置状态机请求
 * @details 向状态机发起状态请求（如启动电压测量、读取结果、重新初始化等）。
 *          调用 BAL_CheckStateRequest() 校验请求合法性（@req FR-3.4.1 / FR-3.4.2）。
 *          若请求不合法则被拒绝，结果立即返回以便调用方处理。
 * @param   stateRequest 待设置的状态请求
 * @return  请求校验结果
 *
 * @note    此函数在均衡策略文件中实现（如 bal_strategy_voltage.c 等）
 */
extern BAL_RETURN_TYPE_e BAL_SetStateRequest(BAL_STATE_REQUEST_e stateRequest);

/**
 * @brief   获取初始化状态
 * @details 查询均衡初始化是否已完成。
 * @return  STD_OK: 已初始化完成（@req FR-3.6.3）
 * @return  STD_NOT_OK: 尚未完成初始化
 *
 * @note    此函数在均衡策略文件中实现（如 bal_strategy_voltage.c 等）
 */
extern STD_RETURN_TYPE_e BAL_GetInitializationState(void);

/**
 * @brief   BAL 状态机触发函数
 * @details 此函数包含 BAL 状态机的完整事件序列。必须每 100 毫秒定时调用一次。
 *          处理流程参见 @ref BAL_SOFTWARE_REQUIREMENTS.md 第 7.2 节。
 *
 * @req     NFR-4.1.1 — 状态机触发周期：100ms
 *
 * @note    此函数在均衡策略文件中实现（如 bal_strategy_voltage.c 等）
 * @note    由 bms.c 以 100ms 周期调用
 */
extern void BAL_Trigger(void);

/*========== Getter for static Variables (Unit Test) ========================*/
/** @req NFR-4.5.1 — 单元测试支持：通过条件编译暴露内部变量 */
#ifdef UNITY_UNIT_TEST
extern DATA_BLOCK_BALANCING_CONTROL_s *TEST_BAL_GetBalancingControl(void);
extern BAL_STATE_s *TEST_BAL_GetBalancingState(void);
#endif

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
/** @req NFR-4.5.1 — 单元测试支持：通过条件编译暴露静态函数 */
#ifdef UNITY_UNIT_TEST
extern BAL_FSM_e BAL_GetState(void);
#endif

#endif /* FOXBMS__BAL_H_ */
