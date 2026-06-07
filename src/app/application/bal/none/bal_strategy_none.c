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
 * @file    bal_strategy_none.c
 * @author  foxBMS Team
 * @date    2020-07-02 (date of creation)
 * @updated 2026-04-20 (date of last update)
 * @version v1.11.0
 * @ingroup APPLICATION
 * @prefix  BAL
 * @requirements REQ-001, REQ-002, REQ-003, REQ-004, REQ-005, REQ-006
 *
 * @brief   WEAK Driver for the Balancing module for ICs that to not support
 *          balancing
 * @details TODO
 */

/*========== Includes =======================================================*/
#include "bal.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/
/**
 * @brief   获取均衡初始化状态
 * @req     REQ-001
 * @details 在无均衡策略下，始终返回 STD_OK，表示初始化已完成。
 *          因为无任何均衡硬件需要初始化。
 * @return  STD_OK 始终返回已初始化状态
 */
extern STD_RETURN_TYPE_e BAL_GetInitializationState(void) {
    /* REQ-001: 始终返回已初始化 — 无均衡硬件需初始化 */
    return STD_OK;
}

/**
 * @brief   设置均衡状态请求
 * @req     REQ-002, REQ-006
 * @details 在无均衡策略下，无条件接受所有状态请求并返回 BAL_OK，
 *          但不对请求做任何实际处理。
 * @param   stateRequest 待设置的状态请求（本策略忽略此参数）
 * @return  BAL_OK 始终返回操作成功
 */
extern BAL_RETURN_TYPE_e BAL_SetStateRequest(BAL_STATE_REQUEST_e stateRequest) {
    /* this is a dummy implementation and not using the argument here is fine */
    (void)stateRequest; /**< REQ-006: 标准 C 写法抑制未使用参数警告 */
    /* REQ-002: 无条件接受所有请求，返回成功 */
    return BAL_OK;
}

/**
 * @brief   均衡状态机触发函数
 * @req     REQ-003, REQ-004
 * @details 在无均衡策略下，此函数体为空。不执行任何均衡相关操作。
 *          该接口的存在保证了上层调用方无需针对不同策略编写条件编译代码。
 *          期望编译器优化（-O2）后，此函数被内联或完全优化掉。
 */
extern void BAL_Trigger(void) {
    /* REQ-003: 空操作 — 无均衡硬件，无需任何处理 */
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
