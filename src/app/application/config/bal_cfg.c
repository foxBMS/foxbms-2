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
 * @file    bal_cfg.c
 * @author  foxBMS Team
 * @date    2022-02-26 (date of creation)
 * @updated 2026-04-20 (date of last update)
 * @version v1.11.0
 * @ingroup DRIVERS_CONFIGURATION
 * @prefix  BAL
 *
 * @brief   Implementation for the configuration for the driver for balancing
 * @details 电池均衡模块（BAL）的配置实现。包含均衡阈值的设置与获取函数，
 *          阈值设置时进行输入边界限定检查。
 *
 * @see     BAL_SOFTWARE_REQUIREMENTS.md  软件需求规格说明
 * @see     BAL_REQUIREMENT_TRACEABILITY_MAPPING.md  需求追溯矩阵
 */

/*========== Includes =======================================================*/
#include "bal_cfg.h"

#include "os.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/
/** balancing voltage threshold in mV
 *  @req NFR-4.4.1 — 默认值 BAL_DEFAULT_THRESHOLD_mV (200mV) */
static int32_t bal_threshold_mV = BAL_DEFAULT_THRESHOLD_mV;

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/

/**
 * @brief   设置均衡阈值
 * @req     NFR-4.3.4 — 输入边界限定
 * @details 对输入阈值进行边界检查，确保其在 [BAL_MINIMUM_THRESHOLD_mV,
 *          BAL_MAXIMUM_THRESHOLD_mV] 即 [0mV, 5000mV] 范围内。
 *          超出上限则取上限值，低于下限则取下限值。
 *          写操作在任务临界区内执行以保证原子性。
 * @param   threshold_mV 期望的阈值（mV）
 * @pre     边界检查后，实际写入值 boundedThreshold_mV ∈ [0, 5000]
 * @note    临界区保护 @req NFR-4.3.3
 */
extern void BAL_SetBalancingThreshold(int32_t threshold_mV) {
    int32_t boundedThreshold_mV = threshold_mV;
    /* @req NFR-4.3.4 — 上限边界限定：不超过 BAL_MAXIMUM_THRESHOLD_mV (5000mV) */
    if (boundedThreshold_mV > BAL_MAXIMUM_THRESHOLD_mV) {
        boundedThreshold_mV = BAL_MAXIMUM_THRESHOLD_mV;
    }
    /* @req NFR-4.3.4 — 下限边界限定：不低于 BAL_MINIMUM_THRESHOLD_mV (0mV) */
    if (boundedThreshold_mV < BAL_MINIMUM_THRESHOLD_mV) {
        boundedThreshold_mV = BAL_MINIMUM_THRESHOLD_mV;
    }
    /* @req NFR-4.3.3 — 临界区保护：原子写入 */
    OS_EnterTaskCritical();
    bal_threshold_mV = boundedThreshold_mV;
    OS_ExitTaskCritical();
}

/**
 * @brief   获取均衡阈值
 * @req     NFR-4.3.4 — 返回当前有效均衡阈值
 * @return  当前均衡阈值（mV）
 */
extern int32_t BAL_GetBalancingThreshold_mV(void) {
    return bal_threshold_mV;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
/** @req NFR-4.5.1 — 单元测试支持：通过条件编译暴露内部函数 */
#ifdef UNITY_UNIT_TEST
#endif
