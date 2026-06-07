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
 * @file    bal_cfg.h
 * @author  foxBMS Team
 * @date    2020-02-24 (date of creation)
 * @updated 2026-04-20 (date of last update)
 * @version v1.11.0
 * @ingroup DRIVERS_CONFIGURATION
 * @prefix  BAL
 *
 * @brief   Header for the configuration for the driver for balancing
 * @details 电池均衡模块（BAL）的配置头文件。集中定义所有可调参数和阈值宏。
 *
 * @req     NFR-4.4.1 — 配置参数集中管理
 *
 * @see     BAL_SOFTWARE_REQUIREMENTS.md  软件需求规格说明
 * @see     BAL_REQUIREMENT_TRACEABILITY_MAPPING.md  需求追溯矩阵
 */

#ifndef FOXBMS__BAL_CFG_H_
#define FOXBMS__BAL_CFG_H_

/*========== Includes =======================================================*/

#include <stdint.h>

/*========== Macros and Definitions =========================================*/
/**
 * @req NFR-4.4.1 — 以下所有宏定义均为 BAL 模块可调参数，集中管理
 *
 * 参数汇总（对应 @ref BAL_SOFTWARE_REQUIREMENTS.md 第 4.4 节）：
 * - BAL_FSM_SHORTTIME_100ms  = 1u  → 100ms 短延时 (@req NFR-4.1.1)
 * - BAL_FSM_LONGTIME_100ms   = 50u → 5s   长延时  (@req NFR-4.1.2)
 * - BAL_FSM_BALANCING_TIME_100ms = 10u → 1s 均衡时间 (@req NFR-4.1.3)
 */

/** BAL state machine short time definition in 100*ms
 *  @req NFR-4.1.1 — 状态机触发周期：100ms (1 × 100ms) */
#define BAL_FSM_SHORTTIME_100ms (1u)

/** BAL state machine long time definition in 100*ms
 *  @req NFR-4.1.2 — 长延时周期：5s (50 × 100ms) */
#define BAL_FSM_LONGTIME_100ms (50u)

/** BAL state machine balancing time in 100*ms
 *  @req NFR-4.1.3 — 均衡执行时间：1s (10 × 100ms) */
#define BAL_FSM_BALANCING_TIME_100ms (10u)

/** default value for the BAL voltage threshold
 *  @req NFR-4.4.1 — 默认均衡阈值：200mV */
#define BAL_DEFAULT_THRESHOLD_mV (200)

/** maximum value that BAL voltage threshold may take
 *  @req NFR-4.3.4 — 均衡阈值上限：5000mV */
#define BAL_MAXIMUM_THRESHOLD_mV (5000)

/** minimum value that BAL voltage threshold may take
 *  @req NFR-4.3.4 — 均衡阈值下限：0mV */
#define BAL_MINIMUM_THRESHOLD_mV (0)

/** BAL hysteresis for voltage threshold when balancing was finished in mV
 *  @req NFR-4.4.1 — 均衡结束迟滞：200mV */
#define BAL_HYSTERESIS_mV (200)

/** BAL lower voltage limit in MV
 *  @req NFR-4.4.1 — 均衡电压下限：2000mV */
#define BAL_LOWER_VOLTAGE_LIMIT_mV (2000)

/** BAL upper temperature limit in deci &deg;C
 *  @req NFR-4.4.1 — 均衡温度上限：700 (70.0°C) */
#define BAL_UPPER_TEMPERATURE_LIMIT_ddegC (700)

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/
/**
 * @brief   设置均衡阈值
 * @req     NFR-4.3.4 — 输入边界限定：阈值限定在 [BAL_MINIMUM_THRESHOLD_mV, BAL_MAXIMUM_THRESHOLD_mV]
 * @param   threshold_mV 期望的阈值（mV），将被边界限定后写入
 */
extern void BAL_SetBalancingThreshold(int32_t threshold_mV);

/**
 * @brief   获取均衡阈值
 * @req     NFR-4.3.4 — 返回当前有效均衡阈值
 * @return  当前均衡阈值（mV）
 */
extern int32_t BAL_GetBalancingThreshold_mV(void);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
/** @req NFR-4.5.1 — 单元测试支持：通过条件编译暴露静态函数 */
#ifdef UNITY_UNIT_TEST
#endif

#endif /* FOXBMS__BAL_CFG_H_ */
