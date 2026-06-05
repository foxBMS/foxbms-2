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
 * @file    bms.c
 * @author  foxBMS Team
 * @date    2020-02-24 (date of creation)
 * @updated 2026-04-20 (date of last update)
 * @version v1.11.0
 * @ingroup ENGINE
 * @prefix  BMS
 *
 * @brief   Bms driver implementation
 * @details Implements the state machine that controls the BMS
 *
 */

/*========== Includes =======================================================*/
#include "bms.h"

#include "battery_cell_cfg.h"

#include "afe.h"
#include "bal.h"
#include "can_cbs_tx_cyclic.h"
#include "database.h"
#include "diag.h"
#include "foxmath.h"
#include "imd.h"
#include "led.h"
#include "meas.h"
#include "os.h"
#include "soa.h"
#include "sps.h"

#include <stdbool.h>
#include <stdint.h>

/*========== 宏定义和常量定义 =========================================*/
/** 未设置的活跃延迟时间的默认值 */
/** @需求 需求-051 系统应支持毫秒单位的可配置延迟时间 */
#define BMS_NO_ACTIVE_DELAY_TIME_ms (UINT32_MAX)

/**
 * 保存最后的状态和最后的子状态
 * @需求 需求-005 系统应跟踪前一个状态和子状态
 */
#define BMS_SAVE_LAST_STATES()                \
    bms_state.lastState    = bms_state.state; \
    bms_state.lastSubstate = bms_state.substate

/*========== 静态常数和变量定义 =======================*/

/**
 * 包含BMS状态机的状态
 * @需求 需求-004 系统应在不同BMS状态之间进行转换
 * @需求 需求-005 系统应跟踪前一个状态和子状态
 * @需求 需求-006 系统应支持状态的进入、保持和退出子状态
 * @需求 需求-032 系统应记录哪些字符串处于闭合状态
 * @需求 需求-033 系统应记录哪些字符串的预充电接触器处于闭合状态
 * @需求 需求-034 系统应记录已关闭的字符串数量
 * @需求 需求-035 系统应记录已停用的字符串
 * @需求 需求-036 系统应记录首个闭合字符串的标识符
 * @需求 需求-037 系统应为字符串打开操作设置超时
 * @需求 需求-038 系统应为字符串关闭操作设置超时
 * @需求 需求-039 系统应为下一个字符串闭合设置计时器
 * @需求 需求-045 系统应维护当前系统计时器值
 * @需求 需求-046 系统应支持配置松弛期（Relaxation Period）
 * @需求 需求-047 系统应记录当前的流动状态
 * @需求 需求-048 系统应追踪接触器断路电流超出的时间
 * @需求 需求-052 系统应使用静态变量存储BMS状态
 * @需求 需求-055 系统应设置允许的错误请求计数器
 * @需求 需求-056 系统应在初始化完成后设置初始化标志
 * @需求 需求-057 系统应支持振荡超时检测机制
 */
static BMS_STATE_s bms_state = {
    .currentSystick                    = 0u,
    .timer                             = 0u,
    .stateRequest                      = BMS_STATE_NO_REQUEST,
    .state                             = BMS_FSM_STATE_UNINITIALIZED,
    .substate                          = BMS_FSM_SUBSTATE_ENTRY,
    .lastState                         = BMS_FSM_STATE_UNINITIALIZED,
    .lastSubstate                      = BMS_FSM_SUBSTATE_ENTRY,
    .triggerentry                      = 0u,
    .ErrRequestCounter                 = 0u,
    .initFinished                      = STD_NOT_OK,
    .counter                           = 0u,
    .OscillationTimeout                = 0u,
    .prechargeTryCounter               = 0u,
    .powerPath                         = BMS_POWER_PATH_OPEN,
    .closedStrings                     = {GEN_REPEAT_U(false, GEN_STRIP(BS_NR_OF_STRINGS))},
    .closedPrechargeContactors         = {GEN_REPEAT_U(false, GEN_STRIP(BS_NR_OF_STRINGS))},
    .numberOfClosedStrings             = 0u,
    .deactivatedStrings                = {GEN_REPEAT_U(false, GEN_STRIP(BS_NR_OF_STRINGS))},
    .firstClosedString                 = 0u,
    .stringOpenTimeout                 = 0u,
    .nextStringClosedTimer             = 0u,
    .stringCloseTimeout                = 0u,
    .nextState                         = BMS_FSM_STATE_STANDBY,
    .restTimer_10ms                    = BS_RELAXATION_PERIOD_10ms,
    .currentFlowState                  = BMS_RELAXATION,
    .remainingDelay_ms                 = BMS_NO_ACTIVE_DELAY_TIME_ms,
    .minimumActiveDelay_ms             = BMS_NO_ACTIVE_DELAY_TIME_ms,
    .startOfPrecharging                = 0u,
    .transitionToErrorState            = false,
    .timeAboveContactorBreakCurrent_ms = 0u,
    .stringToBeOpened                  = 0u,
    .contactorToBeOpened               = CONT_UNDEFINED,
};

/** 本地数据库表的副本 */
/**@{*/
/** @需求 需求-012 系统应维护本地数据库表副本，包括最小/最大值表 */
static DATA_BLOCK_MIN_MAX_s bms_tableMinMax         = {.header.uniqueId = DATA_BLOCK_ID_MIN_MAX};
/** @需求 需求-012 系统应维护本地数据库表副本，包括开路检测表 */
/** @需求 需求-013 系统应检测任何电压感知线路的开路状况 */
static DATA_BLOCK_OPEN_WIRE_s bms_tableOpenWire     = {.header.uniqueId = DATA_BLOCK_ID_OPEN_WIRE_BASE};
/** @需求 需求-012 系统应维护本地数据库表副本，包括电池包值表 */
static DATA_BLOCK_PACK_VALUES_s bms_tablePackValues = {.header.uniqueId = DATA_BLOCK_ID_PACK_VALUES};
/**@}*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/**
 * @brief       checks the state requests that are made.
 * @details     This function checks the validity of the state requests. The
 *              results of the checked is returned immediately.
 * @param[in]   statereq    state request to be checked
 * @return      result of the state request that was made
 * @req REQ-001 System should check and validate state request validity
 * @req REQ-055 System should set allowed error request counter
 */
static BMS_RETURN_TYPE_e BMS_CheckStateRequest(BMS_STATE_REQUEST_e statereq);

/**
 * @简述 将当前状态请求转移到状态机
 * @详情 此函数从#bms_state中获取当前状态请求，
 *       将其转移到状态机。它将#bms_state中的值
 *       重置为#BMS_STATE_NO_REQUEST
 * @返回 当前状态请求
 * @需求 需求-002 系统应将经过验证的状态请求转移到状态机中
 */
static BMS_STATE_REQUEST_e BMS_TransferStateRequest(void);

/**
 * @简述 SYS状态机触发函数的重进入检查
 * @详情 此函数不可重入，应仅由时间或事件触发调用。
 *       它从状态变量bms_state中增加triggerentry计数器。
 *       绝对不应该由两个不同的进程同时调用。
 * @返回 retval  如果没有进一步的函数实例处于活动状态返回0，
 *               否则返回0xff
 * @需求 需求-040 系统应检查状态机触发函数的重进入情况
 * @需求 需求-041 系统应在重进入时返回错误状态（0xff）
 * @需求 需求-042 状态机触发函数不应该被多个不同进程同时调用
 */
static uint8_t BMS_CheckReEntrance(void);

/**
 * @简述 检查对BMS状态机所做的状态请求
 * @详情 检查数据库中的状态请求并将此值设置为返回值
 * @返回 请求的状态
 * @需求 需求-043 系统应从数据库中检查CAN总线请求
 * @需求 需求-044 系统应将CAN请求转换为状态请求
 */
static uint8_t BMS_CheckCanRequests(void);

/**
 * @简述 检查诊断模块中严重级别为#DIAG_FATAL_ERROR的所有错误标志
 * @详情 检查诊断模块中严重级别为#DIAG_FATAL_ERROR的所有错误标志。
 *       此外，设置bms_state变量的minimumActiveDelay_ms参数。
 * @返回 如果设置了错误标志返回true，否则返回false
 * @需求 需求-007 系统应监测诊断模块中所有严重级别为DIAG_FATAL_ERROR的错误标志
 * @需求 需求-008 系统应在检测到致命错误时设置minimumActiveDelay_ms参数
 * @需求 需求-009 系统应在多个错误同时激活时使用最短延迟时间
 */
static bool BMS_IsAnyFatalErrorFlagSet(void);

/**
 * @简述 检查是否有任何错误标志被设置，并处理延迟直到接触器需要打开
 * @详情 检查所有严重级别为#DIAG_FATAL_ERROR的诊断条目，
 *       并处理配置的延迟时间直到接触器需要打开。
 *       如果同时有多个错误处于活动状态，使用最短的延迟。
 * @返回 如果检测到错误且延迟时间已过期返回#STD_NOT_OK，否则返回#STD_OK
 * @需求 需求-010 系统应检测电池系统状态是否正常，并在延迟时间到期时触发接触器打开
 */
static STD_RETURN_TYPE_e BMS_IsBatterySystemStateOkay(void);

/**
 * @简述 检查特定接触器的反馈是否有效
 * @详情 读取错误标志数据库条目，检查此特定接触器的反馈是否有效
 * @返回 如果没有检测到错误反馈有效则返回true，否则返回false
 * @需求 需求-015 系统应验证特定接触器的反馈有效性
 * @需求 需求-016 系统应从错误标志数据库条目中读取接触器反馈状态
 * @需求 需求-017 系统应支持检查指定字符串和接触器类型的反馈
 */
static bool BMS_IsContactorFeedbackValid(uint8_t stringNumber, CONT_TYPE_e contactorType);

/** 获取最新数据库条目供静态模块变量使用 */
/** @需求 需求-011 系统应从数据库中获取最新的测量值 */
static void BMS_GetMeasurementValues(void);

/**
 * @简述 检查任何开路电压感知线
 * @需求 需求-013 系统应检测任何电压感知线路的开路状况
 * @需求 需求-014 系统应在检测到开路时记录故障诊断信息
 */
static void BMS_CheckOpenSenseWire(void);

/**
 * @简述 检查电流限制是否被违反
 * @参数[输入] stringNumber          要处理的字符串
 * @参数[输入] pPackValues           指向电池包值数据库条目的指针
 * @参数[输入] monitoringParameters
 * @参数[输入] timeout_ms
 * @返回 如果预充电成功返回BMS_PRECHARGING_SUCCESSFUL
 *       如果预充电正在进行返回BMS_PRECHARGING_ONGOING
 *       如果达到超时且预充电未成功返回BMS_PRECHARGING_FAILED
 *       (类型: #BMS_RESULT_PRECHARGE_PROCESS_e)
 * @需求 需求-021 系统应监控预充电过程并检测其成功/失败状态
 * @需求 需求-022 系统应支持预充电超时配置
 * @需求 需求-023 系统应支持预充电重试计数
 * @需求 需求-024 系统应返回预充电过程的相应状态
 */
static BMS_RESULT_PRECHARGE_PROCESS_e BMS_MonitorPrechargeProcess(
    uint8_t stringNumber,
    const DATA_BLOCK_PACK_VALUES_s *pPackValues,
    BS_PRECHARGE_MONITORING_e monitoringParameters,
    uint32_t timeout_ms);

/**
 * @简述 检查传入的电池电流是否低于限制
 * @参数[输入] stringNumber 要处理的字符串
 * @参数[输入] pPackValues  指向电池包值数据库条目的指针
 * @返回 如果电池电流低于限制返回#STD_OK，否则返回#STD_NOT_OK
 * @需求 需求-025 系统应检查预充电电流是否低于配置的限制值
 * @需求 需求-027 系统应在检查失败时返回STD_NOT_OK
 */
static STD_RETURN_TYPE_e BMS_IsPrechargeCurrentBelowLimit(
    uint8_t stringNumber,
    const DATA_BLOCK_PACK_VALUES_s *pPackValues);

/**
 * @简述 检查电流限制是否被违反
 * @参数[输入] stringNumber 要处理的字符串
 * @参数[输入] pPackValues  指向电池包值数据库条目的指针
 * @返回 如果电池和DC链路电压之间的差异低于限制则返回true，否则返回false
 * @需求 需求-026 系统应检查电池电压与DC链路电压之间的差异是否低于限制值
 * @需求 需求-027 系统应在检查失败时返回STD_NOT_OK
 */
static STD_RETURN_TYPE_e BMS_IsPrechargeVoltageBelowLimit(
    uint8_t stringNumber,
    const DATA_BLOCK_PACK_VALUES_s *pPackValues);

/**
 * @简述 返回总电压最高的字符串ID
 * @详情 这用于在请求放电时选择要闭合的第一个字符串
 * @参数[输入] precharge   如果#BMS_DO_NOT_TAKE_PRECHARGE_INTO_ACCOUNT，
 *                        则忽略字符串的预充电可用性。
 *                        如果#BMS_TAKE_PRECHARGE_INTO_ACCOUNT，仅选择
 *                        有预充电可用的字符串。
 * @参数[输入] pPackValues 指向电池包值数据库条目的指针
 * @返回 具有最高电压的字符串索引。如果没有字符串可用，
 *       返回#BMS_NO_STRING_AVAILABLE。
 * @需求 需求-028 系统应能识别具有最高总电压的字符串
 */
static uint8_t BMS_GetHighestString(BMS_CONSIDER_PRECHARGE_e precharge, DATA_BLOCK_PACK_VALUES_s *pPackValues);

/**
 * @简述 返回电压最接近首个闭合字符串电压的字符串ID
 * @详情 这用于在驾驶中闭合进一步的字符串
 * @参数[输入] precharge   如果#BMS_DO_NOT_TAKE_PRECHARGE_INTO_ACCOUNT，
 *                        则忽略字符串的预充电可用性。
 *                        如果#BMS_TAKE_PRECHARGE_INTO_ACCOUNT，仅选择
 *                        有预充电可用的字符串。
 * @参数[输入] pPackValues 指向电池包值数据库条目的指针
 * @返回 电压最接近首个闭合字符串电压的字符串索引。
 *       如果没有字符串可用，返回#BMS_NO_STRING_AVAILABLE。
 * @需求 需求-029 系统应能识别与首个闭合字符串电压最接近的字符串
 */
static uint8_t BMS_GetClosestString(BMS_CONSIDER_PRECHARGE_e precharge, DATA_BLOCK_PACK_VALUES_s *pPackValues);

/**
 * @简述 返回总电压最低的字符串ID
 * @详情 这用于在请求充电时选择要闭合的第一个字符串。
 *
 * @参数[输入] precharge   如果为0，则忽略字符串的预充电可用性。
 *                        如果为1，仅选择有预充电可用的字符串。
 * @参数[输入] pPackValues 指向电池包值数据库条目的指针
 * @返回 具有最低电压的字符串索引。如果没有字符串可用，
 *       返回#BMS_NO_STRING_AVAILABLE。
 * @需求 需求-030 系统应能识别具有最低总电压的字符串
 */
static uint8_t BMS_GetLowestString(BMS_CONSIDER_PRECHARGE_e precharge, DATA_BLOCK_PACK_VALUES_s *pPackValues);

/**
 * @简述 返回首个闭合字符串与字符串ID之间的电压差
 * @详情 此函数用于检查尝试闭合进一步字符串时的电压
 * @参数[输入] string      ID，必须与首个闭合字符串进行比较
 * @参数[输入] pPackValues 指向电池包值数据库条目的指针
 * @返回 电压差（单位mV），如果电压无效且无法计算差值则返回INT32_MAX
 * @需求 需求-031 系统应计算首个闭合字符串与其他字符串之间的电压差
 */
static int32_t BMS_GetStringVoltageDifference(uint8_t string, const DATA_BLOCK_PACK_VALUES_s *pPackValues);

/**
 * @简述 返回通过所有字符串流动的平均电流
 * @详情 此函数用于闭合字符串时
 * @参数[输入] pPackValues 指向电池包值数据库条目的指针
 * @返回 考虑所有字符串的平均电流（单位mA）。
 *       如果没有有效的电流测量值则返回INT32_MAX
 * @需求 需求-048 系统应追踪接触器断路电流超出的时间
 */
static int32_t BMS_GetAverageStringCurrent(DATA_BLOCK_PACK_VALUES_s *pPackValues);

/**
 * @简述 根据测量/最近的电流值更新电池系统状态变量
 * @参数[输入] pPackValues  来自电流传感器的最近测量值
 * @需求 需求-047 系统应记录当前的流动状态（BMS_RELAXATION或BMS_ACTIVE）
 */
static void BMS_UpdateBatterySystemState(DATA_BLOCK_PACK_VALUES_s *pPackValues);

/**
 * @简述 获取应根据实际电流流动方向打开的首个字符串接触器
 * @详情 检查接触器的安装方向，打开安装在首选电流流动方向的接触器。
 *       如果没有安装在首选方向的接触器，则首先打开正极接触器。
 *       这可能是因为两个接触器都安装在相同方向，或者
 *       接触器是双向的。
 * @参数 stringNumber         将被打开的字符串
 * @参数 flowDirection        电流流动方向（充电或放电）
 * @返回 #CONT_TYPE_e 应该打开的接触器
 * @需求 需求-020 系统应记录需要打开的接触器及其对应的字符串
 */
static CONT_TYPE_e BMS_GetFirstContactorToBeOpened(uint8_t stringNumber, BMS_CURRENT_FLOW_STATE_e flowDirection);

/**
 * @简述 获取应打开的第二个字符串接触器
 * @详情 对于第二个接触器，不需要检查安装方向，因为
 *       电流已经通过打开首个接触器而中断。
 * @参数 stringNumber             将被打开的字符串
 * @参数 firstOpenedContactorType 已打开的首个接触器的类型
 * @返回 #CONT_TYPE_e 应该打开的接触器
 * @需求 需求-018 系统应追踪电源路径状态（打开/闭合）
 * @需求 需求-019 系统应管理多个电池字符串的接触器状态
 * @需求 需求-020 系统应记录需要打开的接触器及其对应的字符串
 */
static CONT_TYPE_e BMS_GetSecondContactorToBeOpened(uint8_t stringNumber, CONT_TYPE_e firstOpenedContactorType);

/*========== Static Function Implementations ================================*/

/**
 * Implementation of BMS_CheckStateRequest
 * @implements REQ-001 System should check and validate state request validity
 * @implements REQ-055 System should maintain allowed error request counter
 */
static BMS_RETURN_TYPE_e BMS_CheckStateRequest(BMS_STATE_REQUEST_e statereq) {
    if (statereq == BMS_STATE_ERROR_REQUEST) {
        return BMS_OK;
    }

    if (bms_state.stateRequest == BMS_STATE_NO_REQUEST) {
        /* init only allowed from the uninitialized state */
        if (statereq == BMS_STATE_INITIALIZATION_REQUEST) {
            if (bms_state.state == BMS_FSM_STATE_UNINITIALIZED) {
                return BMS_OK;
            } else {
                return BMS_ALREADY_INITIALIZED;
            }
        } else {
            return BMS_ILLEGAL_REQUEST;
        }
    } else {
        return BMS_REQUEST_PENDING;
    }
}

/**
 * BMS_CheckReEntrance的实现
 * @实现 需求-040 系统应检查状态机触发函数的重进入情况
 * @实现 需求-041 系统应在重进入时返回错误状态（0xff）
 * @实现 需求-042 状态机触发函数不应该被多个不同进程同时调用
 */
static uint8_t BMS_CheckReEntrance(void) {
    uint8_t retval = 0;
    OS_EnterTaskCritical();
    if (!bms_state.triggerentry) {
        bms_state.triggerentry++;
    } else {
        retval = 0xFF; /* multiple calls of function */
    }
    OS_ExitTaskCritical();
    return retval;
}

/**
 * BMS_TransferStateRequest的实现
 * @实现 需求-002 系统应将经过验证的状态请求转移到状态机中
 */
static BMS_STATE_REQUEST_e BMS_TransferStateRequest(void) {
    BMS_STATE_REQUEST_e retval = BMS_STATE_NO_REQUEST;

    OS_EnterTaskCritical();
    retval                 = bms_state.stateRequest;
    bms_state.stateRequest = BMS_STATE_NO_REQUEST;
    OS_ExitTaskCritical();
    return retval;
}

/**
 * BMS_GetMeasurementValues的实现
 * @实现 需求-011 系统应从数据库中获取最新的测量值
 * @实现 需求-012 系统应维护本地数据库表副本
 */
static void BMS_GetMeasurementValues(void) {
    DATA_READ_DATA(&bms_tablePackValues, &bms_tableOpenWire, &bms_tableMinMax);
}

/**
 * BMS_CheckCanRequests的实现
 * @实现 需求-043 系统应从数据库中检查CAN总线请求
 * @实现 需求-044 系统应将CAN请求转换为状态请求
 */
static uint8_t BMS_CheckCanRequests(void) {
    uint8_t retVal                     = BMS_REQ_ID_NOREQ;
    DATA_BLOCK_STATE_REQUEST_s request = {.header.uniqueId = DATA_BLOCK_ID_STATE_REQUEST};

    DATA_READ_DATA(&request);

    if (request.stateRequestViaCan == BMS_REQ_ID_STANDBY) {
        retVal = BMS_REQ_ID_STANDBY;
    } else if (request.stateRequestViaCan == BMS_REQ_ID_NORMAL) {
        retVal = BMS_REQ_ID_NORMAL;
    } else if (request.stateRequestViaCan == BMS_REQ_ID_CHARGE) {
        retVal = BMS_REQ_ID_CHARGE;
    } else if (request.stateRequestViaCan == BMS_REQ_ID_NOREQ) {
        retVal = BMS_REQ_ID_NOREQ;
    } else {
        /* invalid or no request, default to BMS_REQ_ID_NOREQ (already set) */
    }

    return retVal;
}

/**
 * BMS_CheckOpenSenseWire的实现
 * @实现 需求-013 系统应检测任何电压感知线路的开路状况
 * @实现 需求-014 系统应在检测到开路时记录故障诊断信息
 */
static void BMS_CheckOpenSenseWire(void) {
    uint8_t openWireDetected = 0;

    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        /* Iterate over all modules */
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            /* Iterate over all voltage sense wires: cells per module + 1 */
            for (uint8_t wire = 0u; wire < (BS_NR_OF_CELL_BLOCKS_PER_MODULE + 1); wire++) {
                /* open wire detected */
                if (bms_tableOpenWire.openWire[s][(wire + (m * (BS_NR_OF_CELL_BLOCKS_PER_MODULE + 1))) == 1] > 0u) {
                    openWireDetected++;

                    /* Add additional error handling here */
                }
            }
        }
        /* Set error if open wire detected */
        if (openWireDetected == 0u) {
            DIAG_Handler(DIAG_ID_AFE_OPEN_WIRE, DIAG_EVENT_OK, DIAG_STRING, s);
        } else {
            DIAG_Handler(DIAG_ID_AFE_OPEN_WIRE, DIAG_EVENT_NOT_OK, DIAG_STRING, s);
        }
    }
}

/**
 * BMS_MonitorPrechargeProcess的实现
 * @实现 需求-021 系统应监控预充电过程并检测其成功/失败状态
 * @实现 需求-022 系统应支持预充电超时配置
 * @实现 需求-023 系统应支持预充电重试计数
 * @实现 需求-024 系统应返回预充电过程的相应状态
 */
static BMS_RESULT_PRECHARGE_PROCESS_e BMS_MonitorPrechargeProcess(
    uint8_t stringNumber,
    const DATA_BLOCK_PACK_VALUES_s *pPackValues,
    BS_PRECHARGE_MONITORING_e monitoringParameters,
    uint32_t timeout_ms) {
    /* 确保不访问数据库表中超出界限的数组 */
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);
    FAS_ASSERT(pPackValues != NULL_PTR);
    FAS_ASSERT(
        (monitoringParameters == BS_PRECHARGE_MONITOR_CURRENT) ||
        (monitoringParameters == BS_PRECHARGE_MONITOR_VOLTAGE) ||
        (monitoringParameters == BS_PRECHARGE_MONITOR_CURRENT_AND_VOLTAGE));
    /* AXIVION Routine Generic-MissingParameterAssert: timeout_ms: parameter accepts whole range */
    /* Indicate precharging ongoing until it has been finished or precharging failed */
    BMS_RESULT_PRECHARGE_PROCESS_e prechargingState = BMS_PRECHARGING_ONGOING;
    STD_RETURN_TYPE_e currentPrecharged             = STD_NOT_OK;
    STD_RETURN_TYPE_e voltagePrecharged             = STD_NOT_OK;
    if (monitoringParameters == BS_PRECHARGE_MONITOR_CURRENT) {
        currentPrecharged = BMS_IsPrechargeCurrentBelowLimit(stringNumber, pPackValues);
        voltagePrecharged = STD_OK;
    } else if (monitoringParameters == BS_PRECHARGE_MONITOR_VOLTAGE) {
        voltagePrecharged = BMS_IsPrechargeVoltageBelowLimit(stringNumber, pPackValues);
        currentPrecharged = STD_OK;
    } else {
        /* monitoringParameters == BS_PRECHARGE_MONITOR_CURRENT_AND_VOLTAGE */
        currentPrecharged = BMS_IsPrechargeCurrentBelowLimit(stringNumber, pPackValues);
        voltagePrecharged = BMS_IsPrechargeVoltageBelowLimit(stringNumber, pPackValues);
    }

    if ((currentPrecharged == STD_OK) && (voltagePrecharged == STD_OK)) {
        prechargingState = BMS_PRECHARGING_FINISHED;
        (void)DIAG_Handler(DIAG_ID_PRECHARGE_ABORT_REASON_VOLTAGE, DIAG_EVENT_OK, DIAG_STRING, stringNumber);
        (void)DIAG_Handler(DIAG_ID_PRECHARGE_ABORT_REASON_CURRENT, DIAG_EVENT_OK, DIAG_STRING, stringNumber);
    } else {
        /* Check if precharging timeout has reached to indicate a failure or not */
        if (bms_state.currentSystick - bms_state.startOfPrecharging > timeout_ms) {
            prechargingState = BMS_PRECHARGING_FAILED;
            DIAG_CheckEvent(currentPrecharged, DIAG_ID_PRECHARGE_ABORT_REASON_CURRENT, DIAG_STRING, stringNumber);
            DIAG_CheckEvent(voltagePrecharged, DIAG_ID_PRECHARGE_ABORT_REASON_VOLTAGE, DIAG_STRING, stringNumber);
        }
    }
    return prechargingState;
}

/**
 * BMS_IsPrechargeCurrentBelowLimit的实现
 * @实现 需求-025 系统应检查预充电电流是否低于配置的限制值
 * @实现 需求-027 系统应在检查失败时返回STD_NOT_OK
 */
static STD_RETURN_TYPE_e BMS_IsPrechargeCurrentBelowLimit(
    uint8_t stringNumber,
    const DATA_BLOCK_PACK_VALUES_s *pPackValues) {
    /* AXIVION Routine Generic-MissingParameterAssert: stringNumber: 函数参数由调用者检查 */
    /* AXIVION Routine Generic-MissingParameterAssert: pPackValues: 函数参数由调用者检查 */
    STD_RETURN_TYPE_e retval = STD_NOT_OK;
    /* Only current, not the current direction is checked */
    if ((pPackValues->invalidStringCurrent[stringNumber] == 0u) &&
        ((MATH_AbsInt32_t(pPackValues->stringCurrent_mA[stringNumber]) < BMS_PRECHARGE_CURRENT_THRESHOLD_mA))) {
        retval = STD_OK;
    }
    return retval;
}

/**
 * Implementation of BMS_IsPrechargeVoltageBelowLimit
 * @implements REQ-026 System should check if voltage difference between battery and DC link is below limit
 * @implements REQ-027 System should return STD_NOT_OK on check failure
 */
static STD_RETURN_TYPE_e BMS_IsPrechargeVoltageBelowLimit(
    uint8_t stringNumber,
    const DATA_BLOCK_PACK_VALUES_s *pPackValues) {
    /* AXIVION Routine Generic-MissingParameterAssert: stringNumber: function parameters are checked by caller */
    /* AXIVION Routine Generic-MissingParameterAssert: pPackValues: function parameters are checked by caller */
    STD_RETURN_TYPE_e retval = STD_NOT_OK;
    if ((pPackValues->invalidStringVoltage[stringNumber] == 0u) && (pPackValues->invalidHvBusVoltage == 0u)) {
        const int64_t cont_prechargeVoltDiff_mV = MATH_AbsInt64_t(
            (int64_t)pPackValues->stringVoltage_mV[stringNumber] - (int64_t)pPackValues->highVoltageBusVoltage_mV);
        if (cont_prechargeVoltDiff_mV < BMS_PRECHARGE_VOLTAGE_THRESHOLD_mV) {
            retval = STD_OK;
        }
    }
    return retval;
}

/**
 * Implementation of BMS_IsAnyFatalErrorFlagSet
 * @implements REQ-007 System should monitor all error flags with DIAG_FATAL_ERROR severity
 * @implements REQ-008 System should set minimumActiveDelay_ms parameter on fatal error detection
 * @implements REQ-009 System should use shortest delay when multiple errors are active
 */
static bool BMS_IsAnyFatalErrorFlagSet(void) {
    bool fatalErrorActive = false;

    for (uint16_t entry = 0u; entry < diag_device.numberOfFatalErrors; entry++) {
        const STD_RETURN_TYPE_e diagnosisState =
            DIAG_GetDiagnosisEntryState(diag_device.pFatalErrorLinkTable[entry]->id);
        if (STD_NOT_OK == diagnosisState) {
            /* Fatal error detected -> get delay of this error until contactors shall be opened */
            const uint32_t kDelay_ms = DIAG_GetDelay(diag_device.pFatalErrorLinkTable[entry]->id);
            /* Check if delay of detected failure is smaller than the delay of a previously detected failure */
            if (bms_state.minimumActiveDelay_ms > kDelay_ms) {
                bms_state.minimumActiveDelay_ms = kDelay_ms;
            }
            fatalErrorActive = true;
        }
    }
    return fatalErrorActive;
}

/**
 * Implementation of BMS_IsBatterySystemStateOkay
 * @implements REQ-010 System should detect battery system state and trigger contactor opening on delay expiration
 */
static STD_RETURN_TYPE_e BMS_IsBatterySystemStateOkay(void) {
    STD_RETURN_TYPE_e retVal          = STD_OK; /* is set to STD_NOT_OK if error detected */
    static uint32_t previousTimestamp = 0u;
    uint32_t timestamp                = OS_GetTickCount();

    /* Check if any fatal error is detected */
    const bool isErrorActive = BMS_IsAnyFatalErrorFlagSet();

    /** Check if a fatal error has been detected previously. If yes, check delay */
    if (bms_state.transitionToErrorState == true) {
        /* Decrease active delay since last call */
        const uint32_t timeSinceLastCall_ms = timestamp - previousTimestamp;
        if (timeSinceLastCall_ms <= bms_state.remainingDelay_ms) {
            bms_state.remainingDelay_ms -= timeSinceLastCall_ms;
        } else {
            bms_state.remainingDelay_ms = 0u;
        }

        /* Check if delay from a new error is shorter then active delay from
         * previously detected error in BMS state machine */
        if (bms_state.remainingDelay_ms >= bms_state.minimumActiveDelay_ms) {
            bms_state.remainingDelay_ms = bms_state.minimumActiveDelay_ms;
        }
    } else {
        /* Delay is not active, check if it should be activated */
        if (isErrorActive == true) {
            bms_state.transitionToErrorState = true;
            bms_state.remainingDelay_ms      = bms_state.minimumActiveDelay_ms;
        }
    }

    /** Set previous timestamp for next call */
    previousTimestamp = timestamp;

    /* Check if bms state machine should switch to error state. This is the case
     * if the delay is activated and the remaining delay is down to 0 */
    if ((bms_state.transitionToErrorState == true) && (bms_state.remainingDelay_ms == 0u)) {
        retVal = STD_NOT_OK;
    }

    return retVal;
}

/**
 * Implementation of BMS_IsContactorFeedbackValid
 * @implements REQ-015 System should verify specific contactor feedback validity
 * @implements REQ-016 System should read contactor feedback state from error flag database
 * @implements REQ-017 System should support checking feedback for specified string and contactor type
 */
static bool BMS_IsContactorFeedbackValid(uint8_t stringNumber, CONT_TYPE_e contactorType) {
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);
    FAS_ASSERT(contactorType != CONT_UNDEFINED);
    bool feedbackValid = false;
    /* Read latest error flags from database */
    DATA_BLOCK_ERROR_STATE_s tableErrorFlags = {.header.uniqueId = DATA_BLOCK_ID_ERROR_STATE};
    DATA_READ_DATA(&tableErrorFlags);
    /* Check if contactor feedback is valid */
    switch (contactorType) {
        case CONT_PLUS:
            if (tableErrorFlags.contactorInPositivePathOfStringFeedbackError[stringNumber] == false) {
                feedbackValid = true;
            }
            break;
        case CONT_MINUS:
            if (tableErrorFlags.contactorInNegativePathOfStringFeedbackError[stringNumber] == false) {
                feedbackValid = true;
            }
            break;
        case CONT_PRECHARGE:
            if (tableErrorFlags.prechargeContactorFeedbackError[stringNumber] == false) {
                feedbackValid = true;
            }
            break;
        default:
            /* CONT_UNDEFINED already prevent via assert */
            break;
    }
    return feedbackValid;
}

/**
 * Implementation of BMS_GetHighestString
 * @implements REQ-028 System should identify string with highest total voltage for drive-off mode
 */
static uint8_t BMS_GetHighestString(BMS_CONSIDER_PRECHARGE_e precharge, DATA_BLOCK_PACK_VALUES_s *pPackValues) {
    FAS_ASSERT(pPackValues != NULL_PTR);
    uint8_t highest_string_index = BMS_NO_STRING_AVAILABLE;
    int32_t max_stringVoltage_mV = INT32_MIN;

    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        if ((pPackValues->stringVoltage_mV[s] >= max_stringVoltage_mV) &&
            (pPackValues->invalidStringVoltage[s] == 0u)) {
            if (bms_state.deactivatedStrings[s] == false) {
                if (precharge == BMS_DO_NOT_TAKE_PRECHARGE_INTO_ACCOUNT) {
                    max_stringVoltage_mV = pPackValues->stringVoltage_mV[s];
                    highest_string_index = s;
                } else {
                    if (bs_stringsWithPrecharge[s] == BS_STRING_WITH_PRECHARGE) {
                        max_stringVoltage_mV = pPackValues->stringVoltage_mV[s];
                        highest_string_index = s;
                    }
                }
            }
        }
    }

    return highest_string_index;
}

/**
 * Implementation of BMS_GetClosestString
 * @implements REQ-029 System should identify string with voltage closest to first closed string for drive mode
 */
static uint8_t BMS_GetClosestString(BMS_CONSIDER_PRECHARGE_e precharge, DATA_BLOCK_PACK_VALUES_s *pPackValues) {
    FAS_ASSERT(pPackValues != NULL_PTR);
    uint8_t closestStringIndex     = BMS_NO_STRING_AVAILABLE;
    int32_t closedStringVoltage_mV = 0;
    bool searchString              = false;

    /* Get voltage of first closed string */
    if (pPackValues->invalidStringVoltage[bms_state.firstClosedString] == 0u) {
        closedStringVoltage_mV = pPackValues->stringVoltage_mV[bms_state.firstClosedString];
        searchString           = true;
    } else if (pPackValues->invalidHvBusVoltage == 0u) {
        /* Use high voltage bus voltage if measured string voltage is invalid */
        closedStringVoltage_mV = pPackValues->highVoltageBusVoltage_mV;
        searchString           = true;
    } else {
        /* Do not search for next string  if no valid voltages could be measured */
        searchString = false;
    }

    if (searchString == true) {
        for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
            const bool isStringClosed          = BMS_IsStringClosed(s);
            const uint8_t isStringVoltageValid = pPackValues->invalidStringVoltage[s];
            if ((isStringClosed == false) && (isStringVoltageValid == 0u)) {
                /* Only check open strings with valid voltages */
                int32_t minimumVoltageDifference_mV = INT32_MAX;
                int32_t voltageDifference_mV        = labs(closedStringVoltage_mV - pPackValues->stringVoltage_mV[s]);
                if (voltageDifference_mV <= minimumVoltageDifference_mV) {
                    if (bms_state.deactivatedStrings[s] == false) {
                        if (precharge == BMS_TAKE_PRECHARGE_INTO_ACCOUNT) {
                            if (bs_stringsWithPrecharge[s] == BS_STRING_WITH_PRECHARGE) {
                                minimumVoltageDifference_mV = voltageDifference_mV;
                                closestStringIndex          = s;
                            }
                        } else {
                            minimumVoltageDifference_mV = voltageDifference_mV;
                            closestStringIndex          = s;
                        }
                    }
                }
            }
        }
    }
    return closestStringIndex;
}

/**
 * Implementation of BMS_GetLowestString
 * @implements REQ-030 System should identify string with lowest total voltage for charge-off mode
 */
static uint8_t BMS_GetLowestString(BMS_CONSIDER_PRECHARGE_e precharge, DATA_BLOCK_PACK_VALUES_s *pPackValues) {
    FAS_ASSERT(pPackValues != NULL_PTR);
    uint8_t lowest_string_index  = BMS_NO_STRING_AVAILABLE;
    int32_t min_stringVoltage_mV = INT32_MAX;

    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        if ((pPackValues->stringVoltage_mV[s] <= min_stringVoltage_mV) &&
            (pPackValues->invalidStringVoltage[s] == 0u)) {
            if (bms_state.deactivatedStrings[s] == false) {
                if (precharge == BMS_DO_NOT_TAKE_PRECHARGE_INTO_ACCOUNT) {
                    min_stringVoltage_mV = pPackValues->stringVoltage_mV[s];
                    lowest_string_index  = s;
                } else {
                    if (bs_stringsWithPrecharge[s] == BS_STRING_WITH_PRECHARGE) {
                        min_stringVoltage_mV = pPackValues->stringVoltage_mV[s];
                        lowest_string_index  = s;
                    }
                }
            }
        }
    }
    return lowest_string_index;
}

/**
 * Implementation of BMS_GetStringVoltageDifference
 * @implements REQ-031 System should calculate voltage difference between first closed and other strings
 */
static int32_t BMS_GetStringVoltageDifference(uint8_t string, const DATA_BLOCK_PACK_VALUES_s *pPackValues) {
    FAS_ASSERT(string < BS_NR_OF_STRINGS);
    FAS_ASSERT(pPackValues != NULL_PTR);
    int32_t voltageDifference_mV = INT32_MAX;
    if ((pPackValues->invalidStringVoltage[string] == 0u) &&
        (pPackValues->invalidStringVoltage[bms_state.firstClosedString] == 0u)) {
        /* Calculate difference between string voltages */
        voltageDifference_mV = MATH_AbsInt32_t(
            pPackValues->stringVoltage_mV[string] - pPackValues->stringVoltage_mV[bms_state.firstClosedString]);
    } else if ((pPackValues->invalidStringVoltage[string] == 0u) && (pPackValues->invalidHvBusVoltage == 0u)) {
        /* Calculate difference between string and high voltage bus voltage */
        voltageDifference_mV =
            MATH_AbsInt32_t(pPackValues->stringVoltage_mV[string] - pPackValues->highVoltageBusVoltage_mV);
    } else {
        /* No valid voltages for comparison -> do not calculate difference and return INT32_MAX */
        voltageDifference_mV = INT32_MAX;
    }
    return voltageDifference_mV;
}

/**
 * Implementation of BMS_GetAverageStringCurrent
 * @implements REQ-048 System should track time when contactor break current exceeded
 */
static int32_t BMS_GetAverageStringCurrent(DATA_BLOCK_PACK_VALUES_s *pPackValues) {
    FAS_ASSERT(pPackValues != NULL_PTR);
    int32_t average_current = pPackValues->packCurrent_mA / (int32_t)BS_NR_OF_STRINGS;
    if (pPackValues->invalidPackCurrent == 1u) {
        average_current = INT32_MAX;
    }
    return average_current;
}

/**
 * Implementation of BMS_UpdateBatterySystemState
 * @implements REQ-047 System should record current flow state (BMS_RELAXATION or BMS_ACTIVE)
 */
static void BMS_UpdateBatterySystemState(DATA_BLOCK_PACK_VALUES_s *pPackValues) {
    FAS_ASSERT(pPackValues != NULL_PTR);

    /* Only update system state if current value is valid */
    if (pPackValues->invalidPackCurrent == 0u) {
        if (BS_POSITIVE_DISCHARGE_CURRENT == true) {
            /* Positive current values equal a discharge of the battery system */
            if (pPackValues->packCurrent_mA >= BS_REST_CURRENT_mA) { /* TODO: string use pack current */
                bms_state.currentFlowState = BMS_DISCHARGING;
                bms_state.restTimer_10ms   = BS_RELAXATION_PERIOD_10ms;
            } else if (pPackValues->packCurrent_mA <= -BS_REST_CURRENT_mA) {
                bms_state.currentFlowState = BMS_CHARGING;
                bms_state.restTimer_10ms   = BS_RELAXATION_PERIOD_10ms;
            } else {
                /* Current below rest current: either battery system is at rest
                 * or the relaxation process is still ongoing */
                if (bms_state.restTimer_10ms == 0u) {
                    /* Rest timer elapsed -> battery system at rest */
                    bms_state.currentFlowState = BMS_AT_REST;
                } else {
                    bms_state.restTimer_10ms--;
                    bms_state.currentFlowState = BMS_RELAXATION;
                }
            }
        } else {
            /* Negative current values equal a discharge of the battery system */
            if (pPackValues->packCurrent_mA <= -BS_REST_CURRENT_mA) {
                bms_state.currentFlowState = BMS_DISCHARGING;
                bms_state.restTimer_10ms   = BS_RELAXATION_PERIOD_10ms;
            } else if (pPackValues->packCurrent_mA >= BS_REST_CURRENT_mA) {
                bms_state.currentFlowState = BMS_CHARGING;
                bms_state.restTimer_10ms   = BS_RELAXATION_PERIOD_10ms;
            } else {
                /* Current below rest current: either battery system is at rest
                 * or the relaxation process is still ongoing */
                if (bms_state.restTimer_10ms == 0u) {
                    /* Rest timer elapsed -> battery system at rest */
                    bms_state.currentFlowState = BMS_AT_REST;
                } else {
                    bms_state.restTimer_10ms--;
                    bms_state.currentFlowState = BMS_RELAXATION;
                }
            }
        }
    }
}

/**
 * Implementation of BMS_GetFirstContactorToBeOpened
 * @implements REQ-020 System should record contactors that need to be opened and their corresponding strings
 */
static CONT_TYPE_e BMS_GetFirstContactorToBeOpened(uint8_t stringNumber, BMS_CURRENT_FLOW_STATE_e flowDirection) {
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);
    /* AXIVION Routine Generic-MissingParameterAssert: flowDirection: parameter accepts all defined enums */
    CONT_TYPE_e contactorToBeOpened                     = CONT_UNDEFINED;
    CONT_CURRENT_BREAKING_DIRECTION_e breakingDirection = CONT_BIDIRECTIONAL;
    /* Required preferred opening direction dependent on the current direction */
    if (flowDirection == BMS_CHARGING) {
        breakingDirection = CONT_CHARGING_DIRECTION;
    } else {
        breakingDirection = CONT_DISCHARGING_DIRECTION;
    }
    /* Iterate over contactor array and search for wanted contactor */
    uint8_t contactor = 0u;
    for (; contactor < BS_NR_OF_CONTACTORS; contactor++) {
        /* Search for:
         * 1. contactor from requested string
         * 2. contactor mounted in preferred opening direction or is bidirectional
         * 3. is no precharge contactor */
        bool correctString           = (bool)(stringNumber == cont_contactorStates[contactor].stringIndex);
        bool inPreferredDirection    = (bool)(breakingDirection == cont_contactorStates[contactor].breakingDirection);
        bool hasNoPreferredDirection = (bool)(cont_contactorStates[contactor].breakingDirection == CONT_BIDIRECTIONAL);
        bool noPrechargeContactor    = (bool)(cont_contactorStates[contactor].type != CONT_PRECHARGE);
        if (correctString && noPrechargeContactor && (inPreferredDirection || hasNoPreferredDirection)) {
            contactorToBeOpened = cont_contactorStates[contactor].type;
            break;
        }
    }
    if (contactor == BS_NR_OF_CONTACTORS) {
        /* No contactor mounted in preferred current direction found. Select
         * the PLUS contactor found in array cont_contactorStates from the
         * passed string */
        for (contactor = 0u; contactor < BS_NR_OF_CONTACTORS; contactor++) {
            /* Search for:
             * 1. contactor from requested string
             * 2. is PLUS contactor */
            if ((stringNumber == cont_contactorStates[contactor].stringIndex) &&
                (cont_contactorStates[contactor].type == CONT_PLUS)) {
                contactorToBeOpened = cont_contactorStates[contactor].type;
                break;
            }
        }
    }
    if (contactor == BS_NR_OF_CONTACTORS) {
        /* No PLUS contactor found. Select MINUS contactor found in array
         * cont_contactorStates from the passed string */
        for (contactor = 0u; contactor < BS_NR_OF_CONTACTORS; contactor++) {
            /* Search for:
             * 1. contactor from requested string
             * 2. is PLUS contactor */
            if ((stringNumber == cont_contactorStates[contactor].stringIndex) &&
                (cont_contactorStates[contactor].type == CONT_MINUS)) {
                contactorToBeOpened = cont_contactorStates[contactor].type;
                break;
            }
        }
    }
    if (contactor == BS_NR_OF_CONTACTORS) {
        /* No PLUS or MAIN_MINUS contactor found in requested string. */
        FAS_ASSERT(FAS_TRAP);
    }
    return contactorToBeOpened;
}

/**
 * Implementation of BMS_GetSecondContactorToBeOpened
 * @implements REQ-018 System should track power path state (open/closed)
 * @implements REQ-019 System should manage contactor states for multiple battery strings
 * @implements REQ-020 System should record contactors that need to be opened and their corresponding strings
 */
static CONT_TYPE_e BMS_GetSecondContactorToBeOpened(uint8_t stringNumber, CONT_TYPE_e firstOpenedContactorType) {
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);
    FAS_ASSERT((firstOpenedContactorType != CONT_UNDEFINED) && (firstOpenedContactorType != CONT_PRECHARGE));
    CONT_TYPE_e contactorToBeOpened = CONT_UNDEFINED;
    /* Check what contactor has already been opened and select the other one */
    if (firstOpenedContactorType == CONT_PLUS) {
        contactorToBeOpened = CONT_MINUS;
    } else {
        contactorToBeOpened = CONT_PLUS;
    }
    /* Iterate over contactor array and search for wanted contactor */
    uint8_t contactor = 0u;
    for (; contactor < BS_NR_OF_CONTACTORS; contactor++) {
        /* Search for specific contactor from requested string */
        if ((stringNumber == cont_contactorStates[contactor].stringIndex) &&
            (contactorToBeOpened == cont_contactorStates[contactor].type)) {
            contactorToBeOpened = cont_contactorStates[contactor].type;
            break;
        }
    }
    if (contactor == BS_NR_OF_CONTACTORS) {
        /* No PLUS or MAIN_MINUS contactor found in requested string.
         * Apparently, only one contactor has been defined for this string */
        FAS_ASSERT(FAS_TRAP);
    }
    return contactorToBeOpened;
}

/*========== 外部函数实现 ================================*/

/**
 * 获取BMS初始化状态
 * @实现 需求-056 系统应在初始化完成后设置初始化标志
 */
extern STD_RETURN_TYPE_e BMS_GetInitializationState(void) {
    return bms_state.initFinished;
}

/**
 * 获取当前BMS FSM状态
 * @实现 需求-004 系统应在不同BMS状态之间进行转换
 */
extern BMS_FSM_STATES_e BMS_GetState(void) {
    return bms_state.state;
}

/**
 * 获取当前BMS FSM子状态
 * @实现 需求-006 系统应支持状态的进入、保持和退出子状态
 */
extern BMS_FSM_SUB_e BMS_GetSubstate(void) {
    return bms_state.substate;
}

/**
 * 设置BMS状态请求
 * @实现 需求-001 系统应能检查并验证状态请求的有效性
 * @实现 需求-002 系统应将经过验证的状态请求转移到状态机中
 */
BMS_RETURN_TYPE_e BMS_SetStateRequest(BMS_STATE_REQUEST_e statereq) {
    BMS_RETURN_TYPE_e retVal = BMS_OK;

    OS_EnterTaskCritical();
    retVal = BMS_CheckStateRequest(statereq);

    if (retVal == BMS_OK) {
        bms_state.stateRequest = statereq;
    }
    OS_ExitTaskCritical();

    return retVal;
}

/**
 * BMS状态机触发函数
 * @实现 需求-004 系统应在不同BMS状态之间进行转换
 * @实现 需求-005 系统应跟踪前一个状态和子状态
 * @实现 需求-011 系统应从数据库中获取最新测量值
 * @实现 需求-013 系统应检测任何电压感知线路的开路状况
 * @实现 需求-040 系统应检查状态机触发函数的重进入情况
 * @实现 需求-041 系统应在重进入时返回错误状态
 * @实现 需求-045 系统应维护当前系统计时器值
 * @实现 需求-047 系统应记录当前的流动状态
 * @实现 需求-049 系统应支持时间或事件触发模式
 */
void BMS_Trigger(void) {
    BMS_STATE_REQUEST_e statereq                   = BMS_STATE_NO_REQUEST;
    DATA_BLOCK_SYSTEM_STATE_s systemState          = {.header.uniqueId = DATA_BLOCK_ID_SYSTEM_STATE};
    bms_state.currentSystick                       = OS_GetTickCount();
    static uint32_t nextOpenWireCheck              = 0;
    static uint8_t stringNumber                    = 0u;
    static uint8_t nextStringNumber                = 0u;
    CONT_ELECTRICAL_STATE_TYPE_e contactorState    = CONT_SWITCH_UNDEFINED;
    BMS_RESULT_PRECHARGE_PROCESS_e prechargeRetval = BMS_PRECHARING_HAS_NOT_STARTED;
    bool contactorFeedbackValid                    = false;
    STD_RETURN_TYPE_e contRetVal                   = STD_NOT_OK;

    if (bms_state.state != BMS_FSM_STATE_UNINITIALIZED) {
        BMS_GetMeasurementValues();
        BMS_UpdateBatterySystemState(&bms_tablePackValues);
        SOA_CheckVoltages(&bms_tableMinMax);
        SOA_CheckTemperatures(&bms_tableMinMax, &bms_tablePackValues);
        SOA_CheckCurrent(&bms_tablePackValues);
        SOA_CheckSlaveTemperatures();
        BMS_CheckOpenSenseWire();
        CONT_CheckFeedback();
    }
    /* Check re-entrance of function */
    if (BMS_CheckReEntrance() > 0u) {
        return;
    }

    if (bms_state.nextStringClosedTimer > 0u) {
        bms_state.nextStringClosedTimer--;
    }
    if (bms_state.stringOpenTimeout > 0u) {
        bms_state.stringOpenTimeout--;
    }

    if (bms_state.stringCloseTimeout > 0u) {
        bms_state.stringCloseTimeout--;
    }

    if (bms_state.OscillationTimeout > 0u) {
        bms_state.OscillationTimeout--;
    }

    if (bms_state.timer > 0u) {
        if ((--bms_state.timer) > 0u) {
            bms_state.triggerentry--;
            return; /* handle state machine only if timer has elapsed */
        }
    }

    /****Happens every time the state machine is triggered**************/
    switch (bms_state.state) {
        /****************************UNINITIALIZED****************************/
        case BMS_FSM_STATE_UNINITIALIZED:
            /* waiting for Initialization Request */
            statereq = BMS_TransferStateRequest();
            if (statereq == BMS_STATE_INITIALIZATION_REQUEST) {
                BMS_SAVE_LAST_STATES();
                bms_state.timer    = BMS_FSM_SHORTTIME;
                bms_state.state    = BMS_FSM_STATE_INITIALIZATION;
                bms_state.substate = BMS_FSM_SUBSTATE_ENTRY;
            } else if (statereq == BMS_STATE_NO_REQUEST) {
                /* no actual request pending */
            } else {
                bms_state.ErrRequestCounter++; /* illegal request pending */
            }
            break;

        /****************************INITIALIZATION***************************/
        case BMS_FSM_STATE_INITIALIZATION:
            BMS_SAVE_LAST_STATES();
            /* Reset ALERT mode flag */
            DIAG_Handler(DIAG_ID_ALERT_MODE, DIAG_EVENT_OK, DIAG_SYSTEM, 0u);
            bms_state.initFinished = STD_OK;
            bms_state.timer        = BMS_FSM_LONGTIME;
            bms_state.state        = BMS_FSM_STATE_INITIALIZED;
            bms_state.substate     = BMS_FSM_SUBSTATE_ENTRY;
            break;

        /****************************INITIALIZED******************************/
        case BMS_FSM_STATE_INITIALIZED:
            BMS_SAVE_LAST_STATES();
            if (IMD_RequestInsulationMeasurement() == IMD_ILLEGAL_REQUEST) {
                /* Initialization of IMD device not finished yet -> wait until this is finished before moving on */
                bms_state.timer = BMS_FSM_LONGTIME;
            } else {
                bms_state.timer    = BMS_FSM_SHORTTIME;
                bms_state.state    = BMS_FSM_STATE_IDLE;
                bms_state.substate = BMS_FSM_SUBSTATE_ENTRY;
            }
            break;

        /****************************IDLE*************************************/
        case BMS_FSM_STATE_IDLE:
            BMS_SAVE_LAST_STATES();

            if (bms_state.substate == BMS_FSM_SUBSTATE_ENTRY) {
                DATA_READ_DATA(&systemState);
                systemState.bmsCanState = BMS_CAN_STATE_IDLE;
                DATA_WRITE_DATA(&systemState);
                bms_state.timer    = BMS_FSM_SHORTTIME;
                bms_state.substate = BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS;
                break;
            } else if (bms_state.substate == BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS) {
                if (BMS_IsBatterySystemStateOkay() == STD_NOT_OK) {
                    bms_state.timer     = BMS_FSM_SHORTTIME;
                    bms_state.state     = BMS_FSM_STATE_OPEN_CONTACTORS;
                    bms_state.nextState = BMS_FSM_STATE_ERROR;
                    bms_state.substate  = BMS_FSM_SUBSTATE_ENTRY;
                    break;
                } else {
                    bms_state.timer    = BMS_FSM_SHORTTIME;
                    bms_state.substate = BMS_FSM_SUBSTATE_CHECK_STATE_REQUESTS;
                    break;
                }
            } else if (bms_state.substate == BMS_FSM_SUBSTATE_CHECK_STATE_REQUESTS) {
                if (BMS_CheckCanRequests() == BMS_REQ_ID_STANDBY) {
                    bms_state.timer     = BMS_FSM_SHORTTIME;
                    bms_state.state     = BMS_FSM_STATE_OPEN_CONTACTORS;
                    bms_state.nextState = BMS_FSM_STATE_STANDBY;
                    bms_state.substate  = BMS_FSM_SUBSTATE_ENTRY;
                    break;
                } else {
                    bms_state.timer    = BMS_FSM_SHORTTIME;
                    bms_state.substate = BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS;
                }
                break;
            }
            break;

        /****************************OPEN CONTACTORS**************************/
        case BMS_FSM_STATE_OPEN_CONTACTORS:
            BMS_SAVE_LAST_STATES();

            if (bms_state.substate == BMS_FSM_SUBSTATE_ENTRY) {
                BAL_SetStateRequest(BAL_STATE_NO_BALANCING_REQUEST);
                /* Check if the error reason is the loss of supply voltage clamp 30C */
                if (DIAG_GetDiagnosisEntryState(DIAG_ID_SUPPLY_VOLTAGE_CLAMP_30C_LOST) == STD_NOT_OK) {
                    bms_state.timer    = BMS_FSM_SHORTTIME;
                    bms_state.substate = BMS_FSM_SUBSTATE_HANDLE_SUPPLY_VOLTAGE_30C_LOSS;
                } else {
                    bms_state.timer    = BMS_FSM_SHORTTIME;
                    bms_state.substate = BMS_FSM_SUBSTATE_OPEN_ALL_PRECHARGE_CONTACTORS;
                }
                break;
            } else if (bms_state.substate == BMS_FSM_SUBSTATE_OPEN_ALL_PRECHARGE_CONTACTORS) {
                /* Precharge contactors can always be opened as the precharge
                 * resistor limits the maximum current */
                CONT_OpenAllPrechargeContactors();

                /* Regular string opening - Open one string after another,
                      * starting with highest string index */
                stringNumber       = BS_NR_OF_STRINGS - 1u;
                bms_state.substate = BMS_FSM_SUBSTATE_OPEN_FIRST_STRING_CONTACTOR;
                bms_state.timer    = BMS_TIME_WAIT_AFTER_OPENING_PRECHARGE;

            } else if (bms_state.substate == BMS_FSM_SUBSTATE_OPEN_FIRST_STRING_CONTACTOR) {
                /* Precharge contactors have been opened -> start opening first string contactor */
                /* TODO: Check if precharge contactors have been opened? */
                if ((bms_tablePackValues.invalidStringCurrent[stringNumber] == 0u) &&
                    (MATH_AbsInt32_t(bms_tablePackValues.stringCurrent_mA[stringNumber]) <
                     BS_MAIN_CONTACTORS_MAXIMUM_BREAK_CURRENT_mA)) {
                    /* Current is below maximum break current -> open first contactor
                     * Check the mounting direction of the contactors and open the contactor that is mounted in the
                     * preferred current flow direction. Open the plus contactor first if, there is no contactor
                     * in preferred direction to the current flow to open available. This may be either because both
                     * contactors are installed in the same direction or because the contactors are bidirectional.
                     */
                    const BMS_CURRENT_FLOW_STATE_e flowDirection =
                        BMS_GetCurrentFlowDirection(bms_tablePackValues.stringCurrent_mA[stringNumber]);
                    bms_state.contactorToBeOpened = BMS_GetFirstContactorToBeOpened(stringNumber, flowDirection);
                    bms_state.stringToBeOpened    = stringNumber;
                    /* Open first contactor */
                    CONT_OpenContactor(stringNumber, bms_state.contactorToBeOpened);
                    bms_state.timer             = BMS_WAIT_TIME_AFTER_OPENING_STRING_CONTACTOR;
                    bms_state.substate          = BMS_FSM_SUBSTATE_OPEN_SECOND_STRING_CONTACTOR;
                    bms_state.stringOpenTimeout = BMS_STRING_OPEN_TIMEOUT;
                } else {
                    /* Current is above maximum contactor break current -> contactor can not be opened */
                    bms_state.timeAboveContactorBreakCurrent_ms += BMS_STATEMACHINE_TASK_CYCLE_CONTEXT_MS;
                    if (bms_state.timeAboveContactorBreakCurrent_ms > BS_MAIN_FUSE_MAXIMUM_TRIGGER_DURATION_ms) {
                        /* Fuse should have been triggered by now but apparently has not yet. Do not wait any
                         * longer. Activate ALERT mode and nevertheless start opening the contactors */
                        DIAG_Handler(DIAG_ID_ALERT_MODE, DIAG_EVENT_NOT_OK, DIAG_SYSTEM, 0u);
                        const BMS_CURRENT_FLOW_STATE_e flowDirection =
                            BMS_GetCurrentFlowDirection(bms_tablePackValues.stringCurrent_mA[stringNumber]);
                        bms_state.contactorToBeOpened = BMS_GetFirstContactorToBeOpened(stringNumber, flowDirection);
                        bms_state.stringToBeOpened    = stringNumber;
                        /* Open first contactor */
                        CONT_OpenContactor(bms_state.stringToBeOpened, bms_state.contactorToBeOpened);
                        bms_state.timer    = BMS_WAIT_TIME_AFTER_OPENING_STRING_CONTACTOR;
                        bms_state.substate = BMS_FSM_SUBSTATE_OPEN_SECOND_STRING_CONTACTOR;
                    }
                }
                break;
            } else if (bms_state.substate == BMS_FSM_SUBSTATE_OPEN_SECOND_STRING_CONTACTOR) {
                /* Check if first contactor has been opened correctly */
                contactorState = CONT_GetContactorState(bms_state.stringToBeOpened, bms_state.contactorToBeOpened);
                contactorFeedbackValid =
                    BMS_IsContactorFeedbackValid(bms_state.stringToBeOpened, bms_state.contactorToBeOpened);
                /* If we want to open the contactors because of a feedback
                 * error for this contactor, the statement will never be true.
                 * Thus, also continue if a feedback error for this contactor
                 * is detected as we are not able to get a valid feedback
                 * information at this point */
                if ((contactorState == CONT_SWITCH_OFF) || (contactorFeedbackValid == false)) {
                    /* First contactor opened correctly.
                     * Open second contactor. Pass first opened contactor into function */
                    bms_state.contactorToBeOpened =
                        BMS_GetSecondContactorToBeOpened(stringNumber, bms_state.contactorToBeOpened);
                    /* Open second contactor */
                    CONT_OpenContactor(bms_state.stringToBeOpened, bms_state.contactorToBeOpened);
                    bms_state.timer    = BMS_WAIT_TIME_AFTER_OPENING_STRING_CONTACTOR;
                    bms_state.substate = BMS_FSM_SUBSTATE_CHECK_SECOND_STRING_CONTACTOR;
                } else {
                    /* String not opened, re-issue closing request */
                    CONT_OpenContactor(bms_state.stringToBeOpened, bms_state.contactorToBeOpened);
                    bms_state.timer = BMS_FSM_SHORTTIME;
                    /* TODO: add timeout */
                }
            } else if (bms_state.substate == BMS_FSM_SUBSTATE_CHECK_SECOND_STRING_CONTACTOR) {
                /* Check if second contactor has been opened correctly */
                contactorState = CONT_GetContactorState(bms_state.stringToBeOpened, bms_state.contactorToBeOpened);
                contactorFeedbackValid =
                    BMS_IsContactorFeedbackValid(bms_state.stringToBeOpened, bms_state.contactorToBeOpened);
                /* If we want to open the contactors because of a feedback
                 * error for this contactor, the statement will never be true.
                 * Thus, also continue if a feedback error for this contactor
                 * is detected as we are not able to get a valid feedback
                 * information at this point */
                if ((contactorState == CONT_SWITCH_OFF) || (contactorFeedbackValid == false)) {
                    /* Opening for this string finished. Reset state variables used for opening */
                    bms_state.contactorToBeOpened = CONT_UNDEFINED;
                    bms_state.stringToBeOpened    = 0u;
                    /* String opened. Decrement string counter */
                    if (bms_state.numberOfClosedStrings > 0u) {
                        bms_state.numberOfClosedStrings--;
                    }
                    bms_state.closedStrings[stringNumber] = false;
                    if (stringNumber > 0u) {
                        /* Not all strings opened yet -> open next string */
                        stringNumber--;
                        bms_state.substate = BMS_FSM_SUBSTATE_OPEN_FIRST_STRING_CONTACTOR;
                        bms_state.timer    = BMS_FSM_SHORTTIME;
                        break;
                    } else {
                        /* All strings opened -> prepare to leave state BMS_FSM_OPEN_CONTACTORS */
                        bms_state.substate = BMS_FSM_SUBSTATE_OPEN_STRINGS_EXIT;
                        bms_state.timer    = BMS_FSM_SHORTTIME;
                    }
                    break;
                } else if (bms_state.stringOpenTimeout == 0u) {
                    /* String takes too long to open, go to next string */
                    bms_state.timer    = BMS_FSM_SHORTTIME;
                    bms_state.substate = BMS_FSM_SUBSTATE_OPEN_FIRST_STRING_CONTACTOR;
                    break;
                } else {
                    /* String not opened, re-issue closing request */
                    CONT_OpenContactor(bms_state.stringToBeOpened, bms_state.contactorToBeOpened);
                    bms_state.timer = BMS_FSM_SHORTTIME;
                    break;
                }
            } else if (bms_state.substate == BMS_FSM_SUBSTATE_HANDLE_SUPPLY_VOLTAGE_30C_LOSS) {
                CONT_OpenAllContactors();
                SPS_SwitchOffAllGeneralIoChannels();
                bms_state.substate = BMS_FSM_SUBSTATE_OPEN_STRINGS_EXIT;
                bms_state.timer    = BMS_FSM_SHORTTIME;
                break;
            } else if (bms_state.substate == BMS_FSM_SUBSTATE_OPEN_STRINGS_EXIT) {
                if (bms_state.nextState == BMS_FSM_STATE_STANDBY) {
                    /* Opening due to STANDBY request -> switch to BMS_FSM_STANDBY */
                    bms_state.timer    = BMS_FSM_SHORTTIME;
                    bms_state.state    = BMS_FSM_STATE_STANDBY;
                    bms_state.substate = BMS_FSM_SUBSTATE_ENTRY;
                    break;
                } else {
                    /* Opening due to detected error -> switch to BMS_FSM_STATE_ERROR */
                    bms_state.timer    = BMS_FSM_SHORTTIME;
                    bms_state.state    = BMS_FSM_STATE_ERROR;
                    bms_state.substate = BMS_FSM_SUBSTATE_ENTRY;
                }
            } else {
                FAS_ASSERT(FAS_TRAP);
            }
            break;

        /****************************STANDBY**********************************/
        case BMS_FSM_STATE_STANDBY:
            BMS_SAVE_LAST_STATES();
            if (bms_state.substate == BMS_FSM_SUBSTATE_ENTRY) {
                BAL_SetStateRequest(BAL_STATE_ALLOW_BALANCING_REQUEST);
#if BS_STANDBY_PERIODIC_OPEN_WIRE_CHECK == TRUE
                nextOpenWireCheck = timestamp + BS_STANDBY_OPEN_WIRE_PERIOD_ms;
#endif /* BS_STANDBY_PERIODIC_OPEN_WIRE_CHECK == TRUE */
                bms_state.timer    = BMS_FSM_MEDIUM_TIME;
                bms_state.substate = BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS_INTERLOCK;
                DATA_READ_DATA(&systemState);
                systemState.bmsCanState = BMS_CAN_STATE_STANDBY;
                DATA_WRITE_DATA(&systemState);
                break;
            } else if (bms_state.substate == BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS_INTERLOCK) {
                if (BMS_IsBatterySystemStateOkay() == STD_NOT_OK) {
                    bms_state.timer     = BMS_FSM_SHORTTIME;
                    bms_state.state     = BMS_FSM_STATE_OPEN_CONTACTORS;
                    bms_state.nextState = BMS_FSM_STATE_ERROR;
                    bms_state.substate  = BMS_FSM_SUBSTATE_ENTRY;
                    break;
                } else {
                    bms_state.timer    = BMS_FSM_SHORTTIME;
                    bms_state.substate = BMS_FSM_SUBSTATE_INTERLOCK_CHECKED;
                    break;
                }
            } else if (bms_state.substate == BMS_FSM_SUBSTATE_INTERLOCK_CHECKED) {
                bms_state.timer    = BMS_FSM_SHORTTIME;
                bms_state.substate = BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS;
                break;
            } else if (bms_state.substate == BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS) {
                if (BMS_IsBatterySystemStateOkay() == STD_NOT_OK) {
                    bms_state.timer     = BMS_FSM_SHORTTIME;
                    bms_state.state     = BMS_FSM_STATE_OPEN_CONTACTORS;
                    bms_state.nextState = BMS_FSM_STATE_ERROR;
                    bms_state.substate  = BMS_FSM_SUBSTATE_ENTRY;
                    break;
                } else {
                    bms_state.timer    = BMS_FSM_SHORTTIME;
                    bms_state.substate = BMS_FSM_SUBSTATE_CHECK_STATE_REQUESTS;
                    break;
                }
            } else if (bms_state.substate == BMS_FSM_SUBSTATE_CHECK_STATE_REQUESTS) {
                if (BMS_CheckCanRequests() == BMS_REQ_ID_NORMAL) {
                    bms_state.powerPath = BMS_POWER_PATH_0;
                    bms_state.nextState = BMS_FSM_STATE_DISCHARGE;
                    bms_state.timer     = BMS_FSM_SHORTTIME;
                    bms_state.state     = BMS_FSM_STATE_PRECHARGE;
                    bms_state.substate  = BMS_FSM_SUBSTATE_ENTRY;
                    break;
                }
                if (BMS_CheckCanRequests() == BMS_REQ_ID_CHARGE) {
                    bms_state.powerPath = BMS_POWER_PATH_1;
                    bms_state.nextState = BMS_FSM_STATE_CHARGE;
                    bms_state.timer     = BMS_FSM_SHORTTIME;
                    bms_state.state     = BMS_FSM_STATE_PRECHARGE;
                    bms_state.substate  = BMS_FSM_SUBSTATE_ENTRY;
                    break;
                } else {
#if BS_STANDBY_PERIODIC_OPEN_WIRE_CHECK == TRUE
                    if (nextOpenWireCheck <= timestamp) {
                        MEAS_RequestOpenWireCheck();
                        nextOpenWireCheck = timestamp + BS_STANDBY_OPEN_WIRE_PERIOD_ms;
                    }
#endif /* BS_STANDBY_PERIODIC_OPEN_WIRE_CHECK == TRUE */
                    bms_state.timer    = BMS_FSM_SHORTTIME;
                    bms_state.substate = BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS;
                    break;
                }
            } else {
                FAS_ASSERT(FAS_TRAP);
            }
            break;

        /****************************PRECHARGE********************************/
        case BMS_FSM_STATE_PRECHARGE:
            BMS_SAVE_LAST_STATES();

            if (bms_state.substate == BMS_FSM_SUBSTATE_ENTRY) {
                DATA_READ_DATA(&systemState);
                systemState.bmsCanState = BMS_CAN_STATE_PRECHARGE;
                DATA_WRITE_DATA(&systemState);
                if (bms_state.nextState == BMS_FSM_STATE_CHARGE) {
                    stringNumber = BMS_GetLowestString(BMS_TAKE_PRECHARGE_INTO_ACCOUNT, &bms_tablePackValues);
                } else {
                    stringNumber = BMS_GetHighestString(BMS_TAKE_PRECHARGE_INTO_ACCOUNT, &bms_tablePackValues);
                }
                if (stringNumber == BMS_NO_STRING_AVAILABLE) {
                    bms_state.timer     = BMS_FSM_SHORTTIME;
                    bms_state.state     = BMS_FSM_STATE_OPEN_CONTACTORS;
                    bms_state.nextState = BMS_FSM_STATE_ERROR;
                    bms_state.substate  = BMS_FSM_SUBSTATE_ENTRY;
                    break;
                }
                bms_state.prechargeTryCounter = 0u;
                bms_state.firstClosedString   = stringNumber;
                if (bms_state.OscillationTimeout == 0u) {
                    /* Close MINUS string contactor */
                    if (CONT_CloseContactor(bms_state.firstClosedString, CONT_MINUS) == STD_OK) {
                        bms_state.stringCloseTimeout = BMS_STRING_CLOSE_TIMEOUT;
                        bms_state.timer              = BMS_WAIT_TIME_AFTER_CLOSING_STRING_CONTACTOR;
                        bms_state.substate           = BMS_FSM_SUBSTATE_PRECHARGE_CLOSE_PRECHARGE;
                    } else {
                        /* Invalid contactor requested */
                        bms_state.timer     = BMS_FSM_SHORTTIME;
                        bms_state.state     = BMS_FSM_STATE_OPEN_CONTACTORS;
                        bms_state.nextState = BMS_FSM_STATE_ERROR;
                        bms_state.substate  = BMS_FSM_SUBSTATE_ENTRY;
                    }
                } else if (BMS_IsBatterySystemStateOkay() == STD_NOT_OK) {
                    /* If precharge re-enter timeout not elapsed, wait (and check errors while waiting) */
                    bms_state.timer     = BMS_FSM_SHORTTIME;
                    bms_state.state     = BMS_FSM_STATE_OPEN_CONTACTORS;
                    bms_state.nextState = BMS_FSM_STATE_ERROR;
                    bms_state.substate  = BMS_FSM_SUBSTATE_ENTRY;
                    break;
                }
                break;
            } else if (bms_state.substate == BMS_FSM_SUBSTATE_PRECHARGE_CLOSE_PRECHARGE) {
                /* Check if MINUS contactor has been successfully closed */
                contactorState = CONT_GetContactorState(bms_state.firstClosedString, CONT_MINUS);
                if (contactorState == CONT_SWITCH_ON) {
                    bms_state.OscillationTimeout = BMS_OSCILLATION_TIMEOUT;
                    contRetVal                   = CONT_ClosePrecharge(bms_state.firstClosedString);
                    bms_state.closedPrechargeContactors[stringNumber] = true;
                    if (contRetVal == STD_OK) {
                        /* Minus Contactor closed successfully and request to close precharge contactor sent
                         * -> save timestamp and monitor precharging process */
                        bms_state.startOfPrecharging = bms_state.currentSystick;
                        bms_state.timer              = BMS_FSM_SHORTTIME;
                        bms_state.substate           = BMS_FSM_SUBSTATE_PRECHARGE_CHECK_PRECHARGE_PROCESS;
                    } else {
                        bms_state.timer     = BMS_FSM_SHORTTIME;
                        bms_state.state     = BMS_FSM_STATE_OPEN_CONTACTORS;
                        bms_state.nextState = BMS_FSM_STATE_ERROR;
                        bms_state.substate  = BMS_FSM_SUBSTATE_ENTRY;
                    }
                } else if (bms_state.stringCloseTimeout == 0u) {
                    /* String takes too long to close */
                    bms_state.timer     = BMS_FSM_SHORTTIME;
                    bms_state.state     = BMS_FSM_STATE_OPEN_CONTACTORS;
                    bms_state.nextState = BMS_FSM_STATE_ERROR;
                    bms_state.substate  = BMS_FSM_SUBSTATE_ENTRY;
                } else {
                    /* String not closed, re-issue closing request */
                    CONT_CloseContactor(bms_state.firstClosedString, CONT_MINUS);
                    bms_state.timer = BMS_FSM_SHORTTIME;
                }
                break;
            } else if (bms_state.substate == BMS_FSM_SUBSTATE_PRECHARGE_CHECK_PRECHARGE_PROCESS) {
                if (BMS_IsBatterySystemStateOkay() == STD_NOT_OK) {
                    /* Error detected: abort and do no proceed with monitoring precharge process */
                    bms_state.timer     = BMS_FSM_SHORTTIME;
                    bms_state.state     = BMS_FSM_STATE_OPEN_CONTACTORS;
                    bms_state.nextState = BMS_FSM_STATE_ERROR;
                    bms_state.substate  = BMS_FSM_SUBSTATE_ENTRY;
                    break;
                }
                if (BMS_CheckCanRequests() == BMS_REQ_ID_STANDBY) {
                    /* Contactor open request received: abort here and do no
                     * proceed with monitoring precharge process */
                    bms_state.timer     = BMS_FSM_SHORTTIME;
                    bms_state.state     = BMS_FSM_STATE_OPEN_CONTACTORS;
                    bms_state.nextState = BMS_FSM_STATE_STANDBY;
                    bms_state.substate  = BMS_FSM_SUBSTATE_ENTRY;
                    break;
                }

                contactorState = CONT_GetContactorState(bms_state.firstClosedString, CONT_PRECHARGE);
                /* Monitor precharging */
                prechargeRetval = BMS_MonitorPrechargeProcess(
                    bms_state.firstClosedString,
                    &bms_tablePackValues,
                    BMS_PRECHARGE_MONITORING_PARAMETERS,
                    BMS_MAXIMUM_PRECHARGE_DURATION_ms);
                /* Check if precharge contactor is closed and precharge is finished */
                if ((contactorState == CONT_SWITCH_ON) && (prechargeRetval == BMS_PRECHARGING_FINISHED)) {
                    /* Successfully precharged. Close string PLUS contactor */
                    CONT_CloseContactor(bms_state.firstClosedString, CONT_PLUS);
                    bms_state.stringCloseTimeout = BMS_STRING_CLOSE_TIMEOUT;
                    bms_state.timer              = BMS_WAIT_TIME_AFTER_CLOSING_STRING_CONTACTOR;
                    bms_state.substate           = BMS_FSM_SUBSTATE_CHECK_CLOSE_SECOND_STRING_CONTACTOR_PRECHARGE_STATE;
                    break;
                } else if (prechargeRetval == BMS_PRECHARGING_ONGOING) {
                    /* Stay in this state until precharging is successful
                     * or timeout reached */
                    bms_state.timer    = BMS_FSM_SHORTTIME;
                    bms_state.substate = BMS_FSM_SUBSTATE_PRECHARGE_CHECK_PRECHARGE_PROCESS;
                    break;
                } else {
                    /* Precharging failed. Timeout reached. Open precharge contactor. */
                    contRetVal = CONT_OpenPrecharge(bms_state.firstClosedString);
                    /* Check if retry limit has been reached */
                    if (bms_state.prechargeTryCounter < (BMS_PRECHARGE_TRIES - 1u)) {
                        bms_state.closedPrechargeContactors[stringNumber] = false;
                        if (contRetVal == STD_OK) {
                            bms_state.timer    = BMS_TIME_WAIT_AFTER_PRECHARGE_FAIL;
                            bms_state.substate = BMS_FSM_SUBSTATE_PRECHARGE_CLOSE_PRECHARGE;
                            bms_state.prechargeTryCounter++;
                        } else {
                            bms_state.timer     = BMS_FSM_SHORTTIME;
                            bms_state.state     = BMS_FSM_STATE_OPEN_CONTACTORS;
                            bms_state.nextState = BMS_FSM_STATE_ERROR;
                            bms_state.substate  = BMS_FSM_SUBSTATE_ENTRY;
                        }
                        break;
                    } else {
                        bms_state.closedPrechargeContactors[stringNumber] = false;
                        bms_state.timer                                   = BMS_FSM_SHORTTIME;
                        bms_state.state                                   = BMS_FSM_STATE_OPEN_CONTACTORS;
                        bms_state.nextState                               = BMS_FSM_STATE_ERROR;
                        bms_state.substate                                = BMS_FSM_SUBSTATE_ENTRY;
                        break;
                    }
                }
            } else if (bms_state.substate == BMS_FSM_SUBSTATE_CHECK_CLOSE_SECOND_STRING_CONTACTOR_PRECHARGE_STATE) {
                contactorState = CONT_GetContactorState(bms_state.firstClosedString, CONT_PLUS);
                if (contactorState == CONT_SWITCH_ON) {
                    bms_state.closedStrings[bms_state.firstClosedString] = true;
                    bms_state.numberOfClosedStrings++;
                    bms_state.timer    = BMS_WAIT_TIME_AFTER_CLOSING_STRING_CONTACTOR;
                    bms_state.substate = BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS_PRECHARGE_CLOSING_STRINGS;
                    break;
                } else if (bms_state.stringCloseTimeout == 0u) {
                    /* String takes too long to close */
                    bms_state.timer     = BMS_FSM_SHORTTIME;
                    bms_state.state     = BMS_FSM_STATE_OPEN_CONTACTORS;
                    bms_state.nextState = BMS_FSM_STATE_ERROR;
                    bms_state.substate  = BMS_FSM_SUBSTATE_ENTRY;
                    break;
                } else {
                    /* String not closed, re-issue closing request */
                    CONT_CloseContactor(bms_state.firstClosedString, CONT_PLUS);
                    bms_state.timer    = BMS_FSM_SHORTTIME;
                    bms_state.substate = BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS_PRECHARGE_FIRST_STRING;
                    break;
                }
            } else if (bms_state.substate == BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS_PRECHARGE_FIRST_STRING) {
                if (BMS_IsBatterySystemStateOkay() == STD_NOT_OK) {
                    bms_state.timer     = BMS_FSM_SHORTTIME;
                    bms_state.state     = BMS_FSM_STATE_OPEN_CONTACTORS;
                    bms_state.nextState = BMS_FSM_STATE_ERROR;
                    bms_state.substate  = BMS_FSM_SUBSTATE_ENTRY;
                    break;
                } else {
                    bms_state.timer    = BMS_FSM_SHORTTIME;
                    bms_state.substate = BMS_FSM_SUBSTATE_CHECK_CLOSE_SECOND_STRING_CONTACTOR_PRECHARGE_STATE;
                    break;
                }
            } else if (bms_state.substate == BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS_PRECHARGE_CLOSING_STRINGS) {
                /* Always make one error check after the first string was closed successfully */
                if (BMS_IsBatterySystemStateOkay() == STD_NOT_OK) {
                    bms_state.timer     = BMS_FSM_SHORTTIME;
                    bms_state.state     = BMS_FSM_STATE_OPEN_CONTACTORS;
                    bms_state.nextState = BMS_FSM_STATE_ERROR;
                    bms_state.substate  = BMS_FSM_SUBSTATE_ENTRY;
                    break;
                } else {
                    bms_state.timer    = BMS_FSM_SHORTTIME;
                    bms_state.substate = BMS_FSM_SUBSTATE_PRECHARGE_OPEN_PRECHARGE;
                    break;
                }
            } else if (bms_state.substate == BMS_FSM_SUBSTATE_PRECHARGE_OPEN_PRECHARGE) {
                contRetVal = CONT_OpenPrecharge(bms_state.firstClosedString);
                if (contRetVal == STD_OK) {
                    bms_state.closedPrechargeContactors[stringNumber] = false;
                    bms_state.timer                                   = BMS_TIME_WAIT_AFTER_OPENING_PRECHARGE;
                    bms_state.substate                                = BMS_FSM_SUBSTATE_PRECHARGE_CHECK_OPEN_PRECHARGE;
                } else {
                    bms_state.timer     = BMS_FSM_SHORTTIME;
                    bms_state.state     = BMS_FSM_STATE_OPEN_CONTACTORS;
                    bms_state.nextState = BMS_FSM_STATE_ERROR;
                    bms_state.substate  = BMS_FSM_SUBSTATE_ENTRY;
                }
                break;
            } else if (bms_state.substate == BMS_FSM_SUBSTATE_PRECHARGE_CHECK_OPEN_PRECHARGE) {
                contactorState = CONT_GetContactorState(bms_state.firstClosedString, CONT_PRECHARGE);
                if (contactorState == CONT_SWITCH_OFF) {
                    bms_state.timer    = BMS_FSM_SHORTTIME;
                    bms_state.state    = BMS_FSM_STATE_NORMAL;
                    bms_state.substate = BMS_FSM_SUBSTATE_ENTRY;
                    break;
                } else if (bms_state.stringCloseTimeout == 0u) {
                    /* Precharge contactor takes too long to open */
                    bms_state.timer     = BMS_FSM_SHORTTIME;
                    bms_state.state     = BMS_FSM_STATE_OPEN_CONTACTORS;
                    bms_state.nextState = BMS_FSM_STATE_ERROR;
                    bms_state.substate  = BMS_FSM_SUBSTATE_ENTRY;
                    break;
                } else {
                    /* Precharge contactor not opened, re-issue open request */
                    CONT_OpenPrecharge(bms_state.firstClosedString);
                    bms_state.timer    = BMS_FSM_SHORTTIME;
                    bms_state.substate = BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS_PRECHARGE_FIRST_STRING;
                    break;
                }
            } else {
                FAS_ASSERT(FAS_TRAP);
            }
            break;

        /****************************NORMAL**************************************/
        case BMS_FSM_STATE_NORMAL:
            BMS_SAVE_LAST_STATES();

            if (bms_state.substate == BMS_FSM_SUBSTATE_ENTRY) {
#if BS_NORMAL_PERIODIC_OPEN_WIRE_CHECK == TRUE
                nextOpenWireCheck = timestamp + BS_NORMAL_OPEN_WIRE_PERIOD_ms;
#endif /* BS_NORMAL_PERIODIC_OPEN_WIRE_CHECK == TRUE */
                DATA_READ_DATA(&systemState);
                if (bms_state.nextState == BMS_FSM_STATE_CHARGE) {
                    systemState.bmsCanState = BMS_CAN_STATE_CHARGE;
                } else {
                    systemState.bmsCanState = BMS_CAN_STATE_NORMAL;
                }
                DATA_WRITE_DATA(&systemState);
                bms_state.timer                 = BMS_FSM_SHORTTIME;
                bms_state.substate              = BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS;
                bms_state.nextStringClosedTimer = 0u;
                break;
            } else if (bms_state.substate == BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS) {
                if (BMS_IsBatterySystemStateOkay() == STD_NOT_OK) {
                    bms_state.timer     = BMS_FSM_SHORTTIME;
                    bms_state.state     = BMS_FSM_STATE_OPEN_CONTACTORS;
                    bms_state.nextState = BMS_FSM_STATE_ERROR;
                    bms_state.substate  = BMS_FSM_SUBSTATE_ENTRY;
                    break;
                } else {
                    bms_state.timer    = BMS_FSM_SHORTTIME;
                    bms_state.substate = BMS_FSM_SUBSTATE_CHECK_STATE_REQUESTS;
                    break;
                }
            } else if (bms_state.substate == BMS_FSM_SUBSTATE_CHECK_STATE_REQUESTS) {
                if (BMS_CheckCanRequests() == BMS_REQ_ID_STANDBY) {
                    bms_state.timer     = BMS_FSM_SHORTTIME;
                    bms_state.state     = BMS_FSM_STATE_OPEN_CONTACTORS;
                    bms_state.nextState = BMS_FSM_STATE_STANDBY;
                    bms_state.substate  = BMS_FSM_SUBSTATE_ENTRY;
                    break;
                } else {
#if BS_NORMAL_PERIODIC_OPEN_WIRE_CHECK == TRUE
                    if (nextOpenWireCheck <= timestamp) {
                        MEAS_RequestOpenWireCheck();
                        nextOpenWireCheck = timestamp + BS_NORMAL_OPEN_WIRE_PERIOD_ms;
                    }
#endif /* BS_NORMAL_PERIODIC_OPEN_WIRE_CHECK == TRUE */
                    bms_state.timer    = BMS_FSM_SHORTTIME;
                    bms_state.substate = BMS_FSM_SUBSTATE_NORMAL_CLOSE_NEXT_STRING;
                    break;
                }
            } else if (bms_state.substate == BMS_FSM_SUBSTATE_NORMAL_CLOSE_NEXT_STRING) {
                if (bms_state.nextStringClosedTimer == 0u) {
                    nextStringNumber =
                        BMS_GetClosestString(BMS_DO_NOT_TAKE_PRECHARGE_INTO_ACCOUNT, &bms_tablePackValues);
                    if (nextStringNumber == BMS_NO_STRING_AVAILABLE) {
                        bms_state.timer    = BMS_FSM_SHORTTIME;
                        bms_state.substate = BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS;
                        break;
                    } else if (
                        (BMS_GetStringVoltageDifference(nextStringNumber, &bms_tablePackValues) <=
                         BMS_NEXT_STRING_VOLTAGE_LIMIT_MV) &&
                        (BMS_GetAverageStringCurrent(&bms_tablePackValues) <= BMS_AVERAGE_STRING_CURRENT_LIMIT_MA)) {
                        /* Voltage/current conditions suitable to close a further string. Close first string contactor
                         */
                        CONT_CloseContactor(nextStringNumber, CONT_MINUS);
                        bms_state.nextStringClosedTimer = BMS_STRING_CLOSE_TIMEOUT;
                        bms_state.timer                 = BMS_WAIT_TIME_AFTER_CLOSING_STRING_CONTACTOR;
                        bms_state.substate              = BMS_FSM_SUBSTATE_NORMAL_CLOSE_SECOND_STRING_CONTACTOR;
                        break;
                    }
                } else {
                    bms_state.timer    = BMS_FSM_SHORTTIME;
                    bms_state.substate = BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS;
                    break;
                }
            } else if (bms_state.substate == BMS_FSM_SUBSTATE_NORMAL_CLOSE_SECOND_STRING_CONTACTOR) {
                contactorState = CONT_GetContactorState(nextStringNumber, CONT_MINUS);
                if (contactorState == CONT_SWITCH_ON) {
                    /* First string contactor closed. Close second string contactor */
                    CONT_CloseContactor(nextStringNumber, CONT_PLUS);
                    bms_state.timer    = BMS_WAIT_TIME_AFTER_CLOSING_STRING_CONTACTOR;
                    bms_state.substate = BMS_FSM_SUBSTATE_NORMAL_CLOSE_SECOND_STRING_CONTACTOR;
                } else if (bms_state.stringCloseTimeout == 0u) {
                    /* String takes too long to close */
                    bms_state.timer     = BMS_FSM_SHORTTIME;
                    bms_state.state     = BMS_FSM_STATE_OPEN_CONTACTORS;
                    bms_state.nextState = BMS_FSM_STATE_ERROR;
                    bms_state.substate  = BMS_FSM_SUBSTATE_ENTRY;
                    break;
                } else {
                    /* String minus contactor has not been closed successfully. Re-trigger closing */
                    CONT_CloseContactor(nextStringNumber, CONT_MINUS);
                    bms_state.timer = BMS_FSM_SHORTTIME;
                }
                break;
            } else if (bms_state.substate == BMS_FSM_SUBSTATE_CHECK_STRING_CLOSED) {
                contactorState = CONT_GetContactorState(nextStringNumber, CONT_PLUS);
                if (contactorState == CONT_SWITCH_ON) {
                    bms_state.numberOfClosedStrings++;
                    bms_state.closedStrings[nextStringNumber] = true;
                    bms_state.nextStringClosedTimer           = BMS_WAIT_TIME_BETWEEN_CLOSING_STRINGS;
                    /* Go to begin of NORMAL case to redo the full procedure with error check and request check */
                    bms_state.substate = BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS;
                    break;
                } else if (bms_state.stringCloseTimeout == 0u) {
                    /* String takes too long to close */
                    bms_state.timer     = BMS_FSM_SHORTTIME;
                    bms_state.state     = BMS_FSM_STATE_OPEN_CONTACTORS;
                    bms_state.nextState = BMS_FSM_STATE_ERROR;
                    bms_state.substate  = BMS_FSM_SUBSTATE_ENTRY;
                    break;
                } else if (BMS_IsBatterySystemStateOkay() == STD_NOT_OK) {
                    bms_state.timer     = BMS_FSM_SHORTTIME;
                    bms_state.state     = BMS_FSM_STATE_OPEN_CONTACTORS;
                    bms_state.nextState = BMS_FSM_STATE_ERROR;
                    bms_state.substate  = BMS_FSM_SUBSTATE_ENTRY;
                    break;
                } else if (BMS_CheckCanRequests() == BMS_REQ_ID_STANDBY) {
                    bms_state.timer     = BMS_FSM_SHORTTIME;
                    bms_state.state     = BMS_FSM_STATE_OPEN_CONTACTORS;
                    bms_state.nextState = BMS_FSM_STATE_STANDBY;
                    bms_state.substate  = BMS_FSM_SUBSTATE_ENTRY;
                    break;
                } else {
                    /* String not closed, re-issue closing request */
                    CONT_CloseContactor(nextStringNumber, CONT_PLUS);
                    bms_state.timer = BMS_FSM_SHORTTIME;
                    break;
                }
            } else {
                FAS_ASSERT(FAS_TRAP);
            }
            break;

        /****************************ERROR*************************************/
        case BMS_FSM_STATE_ERROR:
            BMS_SAVE_LAST_STATES();

            if (bms_state.substate == BMS_FSM_SUBSTATE_ENTRY) {
                /* Set BMS System state to error */
                DATA_READ_DATA(&systemState);
                systemState.bmsCanState = BMS_CAN_STATE_ERROR;
                DATA_WRITE_DATA(&systemState);
                /* Deactivate balancing */
                BAL_SetStateRequest(BAL_STATE_NO_BALANCING_REQUEST);
                /* Change LED toggle frequency to indicate an error */
                LED_SetToggleTime(LED_ERROR_OPERATION_ON_OFF_TIME_ms);
                /* Set timer for next open wire check */
                nextOpenWireCheck = bms_state.currentSystick + AFE_ERROR_OPEN_WIRE_PERIOD_ms;
                /* Switch to next substate */
                bms_state.timer    = BMS_FSM_SHORTTIME;
                bms_state.substate = BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS;
                break;
            } else if (bms_state.substate == BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS) {
                if (DIAG_IsAnyFatalErrorSet() == true) {
                    /* we stay already in requested state */
                    if (nextOpenWireCheck <= bms_state.currentSystick) {
                        /* Perform open-wire check periodically */
                        /* MEAS_RequestOpenWireCheck(); */ /*TODO: check with strings */
                        nextOpenWireCheck = bms_state.currentSystick + AFE_ERROR_OPEN_WIRE_PERIOD_ms;
                    }
                } else {
                    /* No error detected anymore - reset fatal error related variables */
                    bms_state.minimumActiveDelay_ms  = BMS_NO_ACTIVE_DELAY_TIME_ms;
                    bms_state.minimumActiveDelay_ms  = BMS_NO_ACTIVE_DELAY_TIME_ms;
                    bms_state.transitionToErrorState = false;
                    /* Check for STANDBY request */
                    bms_state.timer    = BMS_FSM_SHORTTIME;
                    bms_state.substate = BMS_FSM_SUBSTATE_CHECK_STATE_REQUESTS;
                    break;
                }
            } else if (bms_state.substate == BMS_FSM_SUBSTATE_CHECK_STATE_REQUESTS) {
                if (BMS_CheckCanRequests() == BMS_REQ_ID_STANDBY) {
                    /* Activate balancing again */
                    BAL_SetStateRequest(BAL_STATE_ALLOW_BALANCING_REQUEST);
                    /* Set LED frequency to normal operation as we leave error
                       state subsequently */
                    LED_SetToggleTime(LED_NORMAL_OPERATION_ON_OFF_TIME_ms);

                    /* Verify that all contactors are opened and switch to
                     * STANDBY state afterwards */
                    bms_state.state     = BMS_FSM_STATE_OPEN_CONTACTORS;
                    bms_state.nextState = BMS_FSM_STATE_STANDBY;
                    bms_state.substate  = BMS_FSM_SUBSTATE_ENTRY;
                    break;
                } else {
                    bms_state.timer    = BMS_FSM_SHORTTIME;
                    bms_state.substate = BMS_FSM_SUBSTATE_CHECK_ERROR_FLAGS;
                    break;
                }
            } else {
                /* invalid state -> this should never be reached */
                FAS_ASSERT(FAS_TRAP);
            }
            break;
        default:
            /* invalid state */
            FAS_ASSERT(FAS_TRAP);
            break;
    } /* end switch (bms_state.state) */

    /* Send an asynchronous bms state message if the state or substate changed*/
    if ((bms_state.state != bms_state.lastState) || (bms_state.substate != bms_state.lastSubstate)) {
        CANTX_TransmitBmsState();
    }

    bms_state.triggerentry--;
    bms_state.counter++;
}

extern BMS_CURRENT_FLOW_STATE_e BMS_GetBatterySystemState(void) {
    return bms_state.currentFlowState;
}

extern BMS_CURRENT_FLOW_STATE_e BMS_GetCurrentFlowDirection(int32_t current_mA) {
    /* AXIVION Routine Generic-MissingParameterAssert: current_mA: parameter accepts whole range */
    BMS_CURRENT_FLOW_STATE_e retVal = BMS_DISCHARGING;

    if (BS_POSITIVE_DISCHARGE_CURRENT == true) {
        if (current_mA >= BS_REST_CURRENT_mA) {
            retVal = BMS_DISCHARGING;
        } else if (current_mA <= -BS_REST_CURRENT_mA) {
            retVal = BMS_CHARGING;
        } else {
            retVal = BMS_AT_REST;
        }
    } else {
        if (current_mA <= -BS_REST_CURRENT_mA) {
            retVal = BMS_DISCHARGING;
        } else if (current_mA >= BS_REST_CURRENT_mA) {
            retVal = BMS_CHARGING;
        } else {
            retVal = BMS_AT_REST;
        }
    }
    return retVal;
}

extern bool BMS_IsStringClosed(uint8_t stringNumber) {
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);
    bool retval = false;
    if (bms_state.closedStrings[stringNumber] == true) {
        retval = true;
    }
    return retval;
}

extern bool BMS_IsStringPrecharging(uint8_t stringNumber) {
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);
    bool retval = false;
    if (bms_state.closedPrechargeContactors[stringNumber] == true) {
        retval = true;
    }
    return retval;
}

extern uint8_t BMS_GetNumberOfConnectedStrings(void) {
    return bms_state.numberOfClosedStrings;
}

extern bool BMS_IsTransitionToErrorStateActive(void) {
    return bms_state.transitionToErrorState;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
extern BMS_RETURN_TYPE_e TEST_BMS_CheckStateRequest(BMS_STATE_REQUEST_e statereq) {
    return BMS_CheckStateRequest(statereq);
}
extern BMS_STATE_REQUEST_e TEST_BMS_TransferStateRequest(void) {
    return BMS_TransferStateRequest();
}
extern uint8_t TEST_BMS_CheckReEntrance(void) {
    return BMS_CheckReEntrance();
}
extern uint8_t TEST_BMS_CheckCanRequests(void) {
    return BMS_CheckCanRequests();
}
extern bool TEST_BMS_IsAnyFatalErrorFlagSet(void) {
    return BMS_IsAnyFatalErrorFlagSet();
}
extern STD_RETURN_TYPE_e TEST_BMS_IsBatterySystemStateOkay(void) {
    return BMS_IsBatterySystemStateOkay();
}
extern bool TEST_BMS_IsContactorFeedbackValid(uint8_t stringNumber, CONT_TYPE_e contactorType) {
    return BMS_IsContactorFeedbackValid(stringNumber, contactorType);
}
extern void TEST_BMS_GetMeasurementValues(void) {
    BMS_GetMeasurementValues();
}
extern void TEST_BMS_CheckOpenSenseWire(void) {
    BMS_CheckOpenSenseWire();
}
extern STD_RETURN_TYPE_e TEST_BMS_MonitorPrechargeProcess(
    uint8_t stringNumber,
    const DATA_BLOCK_PACK_VALUES_s *pPackValues,
    BS_PRECHARGE_MONITORING_e monitoringParameters,
    uint32_t timeout_ms) {
    return BMS_MonitorPrechargeProcess(stringNumber, pPackValues, monitoringParameters, timeout_ms);
}
extern uint8_t TEST_BMS_GetHighestString(BMS_CONSIDER_PRECHARGE_e precharge, DATA_BLOCK_PACK_VALUES_s *pPackValues) {
    return BMS_GetHighestString(precharge, pPackValues);
}
extern uint8_t TEST_BMS_GetClosestString(BMS_CONSIDER_PRECHARGE_e precharge, DATA_BLOCK_PACK_VALUES_s *pPackValues) {
    return BMS_GetClosestString(precharge, pPackValues);
}

extern uint8_t TEST_BMS_GetLowestString(BMS_CONSIDER_PRECHARGE_e precharge, DATA_BLOCK_PACK_VALUES_s *pPackValues) {
    return BMS_GetLowestString(precharge, pPackValues);
}
extern int32_t TEST_BMS_GetStringVoltageDifference(uint8_t string, DATA_BLOCK_PACK_VALUES_s *pPackValues) {
    return BMS_GetStringVoltageDifference(string, pPackValues);
}
extern int32_t TEST_BMS_GetAverageStringCurrent(DATA_BLOCK_PACK_VALUES_s *pPackValues) {
    return BMS_GetAverageStringCurrent(pPackValues);
}
extern void TEST_BMS_UpdateBatterySystemState(DATA_BLOCK_PACK_VALUES_s *pPackValues) {
    BMS_UpdateBatterySystemState(pPackValues);
}

#endif
