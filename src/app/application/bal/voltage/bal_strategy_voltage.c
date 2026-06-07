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
 * @file    bal_strategy_voltage.c
 * @author  foxBMS Team
 * @date    2020-05-29 (date of creation)
 * @updated 2026-04-20 (date of last update)
 * @version v1.11.0
 * @ingroup APPLICATION
 * @prefix  BAL
 *
 * @brief   Driver for the Balancing module
 * @details 基于电压的电池均衡策略实现。实现了 BAL 状态机的 CHECK_BALANCING 和
 *          BALANCE 状态的处理逻辑，包括均衡激活、均衡停用、四子状态执行流程。
 * @requirements FR-3.1.1, FR-3.1.2, FR-3.1.3, FR-3.2.1, FR-3.3.1, FR-3.4.1 ~ FR-3.4.5, FR-3.5.1 ~ FR-3.5.4, FR-3.6.1, NFR-4.1.3, NFR-4.2.1, NFR-4.3.1, NFR-4.3.2, NFR-4.3.3, NFR-4.5.1
 *
 * @see     BAL_STRATEGY_VOLTAGE_SOFTWARE_REQUIREMENTS.md  软件需求规格说明
 */

/*========== Includes =======================================================*/
#include "battery_cell_cfg.h"

#include "bal.h"
#include "bms.h"
#include "database.h"
#include "os.h"

#include <stdbool.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/
/** local storage of the #DATA_BLOCK_BALANCING_CONTROL_s table
 *  @req NFR-4.2.1 — 数据块静态分配：放置于数据段而非栈上 */
static DATA_BLOCK_BALANCING_CONTROL_s bal_tableBalancingControl = {.header.uniqueId = DATA_BLOCK_ID_BALANCING_CONTROL};

/**
 * @brief   contains the state of the contactor state machine
 * @details 均衡状态机的完整运行状态实例。包含定时器、状态、子状态、许可标志等
 *          所有状态机运行所需的变量。
 * @req     FR-3.1.1, FR-3.3.1, FR-3.4.1, FR-3.5.2, FR-3.5.3
 */
static BAL_STATE_s bal_state = {
    .timer                  = 0,
    .stateRequest           = BAL_STATE_NO_REQUEST,
    .state                  = BAL_FSM_UNINITIALIZED,
    .substate               = BAL_ENTRY,
    .lastState              = BAL_FSM_UNINITIALIZED,
    .lastSubstate           = 0,
    .triggerEntry           = 0,
    .errorRequestCounter    = 0,
    .initializationFinished = STD_NOT_OK,
    .active                 = false,
    .balancingThreshold     = BAL_DEFAULT_THRESHOLD_mV + BAL_HYSTERESIS_mV,
    .balancingAllowed       = true,
    .balancingGlobalAllowed = false,
};

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/
/**
 * @brief   Activates voltage based balancing
 * @req     FR-3.1.1 — 基于电压阈值的均衡激活
 * @req     FR-3.1.2 — 均衡阈值迟滞设置
 * @req     FR-3.1.3 — 均衡完成判断
 * @details 遍历所有串、模块、电池块，将电压超过（该串最低电压 + 均衡阈值）的
 *          电池块激活均衡。首次激活时去除迟滞，使所有需要均衡的电池块一同参与。
 */
static bool BAL_ActivateBalancing(void);

/**
 * @brief   Deactivates voltage based balancing
 * @req     FR-3.2.1 — 全局均衡停用
 * @details The balancing state of all cells in all strings set to inactivate
 *          (that is 0) and the delta charge is set to 0 As. The balancing
 *          enable bit is deactivate (that is 0).
 */
static void BAL_Deactivate(void);

/**
 * @brief   State machine subfunction to check if balancing is allowed
 * @req     FR-3.3.1 — 均衡许可评估与状态转移
 * @details Checks if balancing is allowed. If it is it transfers in the actual
 *          balancing state.
 */
static void BAL_ProcessStateCheckBalancing(BAL_STATE_REQUEST_e state_request);

/**
 * @brief   State machine subfunction to balance the battery cell
 * @req     FR-3.4.1 — 全局禁用优先处理
 * @req     FR-3.4.2 — ENTRY 子状态处理
 * @req     FR-3.4.3 — CHECK_LOWEST_VOLTAGE 子状态处理
 * @req     FR-3.4.4 — CHECK_CURRENT 子状态处理
 * @req     FR-3.4.5 — ACTIVATE_BALANCING 子状态处理
 * @details 实现四子状态均衡执行流程：ENTRY → CHECK_LOWEST_VOLTAGE →
 *          CHECK_CURRENT → ACTIVATE_BALANCING。包含多级安全检查和全局禁用处理。
 */
static void BAL_ProcessStateBalancing(BAL_STATE_REQUEST_e state_request);

/*========== Static Function Implementations ================================*/
static bool BAL_ActivateBalancing(void) {
    bool finished               = true;
    DATA_BLOCK_MIN_MAX_s minMax = {.header.uniqueId = DATA_BLOCK_ID_MIN_MAX};
    /* Database entry is declared static, to place it in the data segment and not on the stack */
    static DATA_BLOCK_CELL_VOLTAGE_s cellVoltage = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE};

    /* FR-3.1.1: 读取单体电压和最小/最大值数据 */
    DATA_READ_DATA(&cellVoltage, &minMax);

    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        int16_t min              = minMax.minimumCellVoltage_mV[s];
        uint16_t nrBalancedCells = 0u;
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t cb = 0u; cb < BS_NR_OF_CELL_BLOCKS_PER_MODULE; cb++) {
                /* FR-3.1.1: 电压超过（该串最低电压 + 均衡阈值） → 激活均衡 */
                if (cellVoltage.cellVoltage_mV[s][m][cb] > (min + bal_state.balancingThreshold)) {
                    bal_tableBalancingControl.activateBalancing[s][m][cb] = true;
                    finished                                              = false;
                    /* FR-3.1.2: 首次激活时去除迟滞，使所有需要均衡的电池块一同参与 */
                    bal_state.balancingThreshold              = BAL_GetBalancingThreshold_mV();
                    bal_state.active                          = true;
                    bal_tableBalancingControl.enableBalancing = true;
                    nrBalancedCells++;
                } else {
                    bal_tableBalancingControl.activateBalancing[s][m][cb] = false;
                }
            }
        }
        bal_tableBalancingControl.nrBalancedCells[s] = nrBalancedCells;
    }
    DATA_WRITE_DATA(&bal_tableBalancingControl);

    return finished;
}

static void BAL_Deactivate(void) {
    /* FR-3.2.1: 遍历所有串、模块、电池块，停用均衡并清零 delta charge */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint16_t cb = 0u; cb < BS_NR_OF_CELL_BLOCKS_PER_MODULE; cb++) {
                bal_tableBalancingControl.activateBalancing[s][m][cb] = false;
                bal_tableBalancingControl.deltaCharge_mAs[s][m][cb]   = 0u;
            }
        }
        bal_tableBalancingControl.nrBalancedCells[s] = 0u;
    }
    bal_tableBalancingControl.enableBalancing = false;
    bal_state.active                          = false;

    DATA_WRITE_DATA(&bal_tableBalancingControl);
}

static void BAL_ProcessStateCheckBalancing(BAL_STATE_REQUEST_e state_request) {
    if (state_request == BAL_STATE_NO_BALANCING_REQUEST) {
        bal_state.balancingAllowed = false;
    }
    if (state_request == BAL_STATE_ALLOW_BALANCING_REQUEST) {
        bal_state.balancingAllowed = true;
    }

    bal_state.timer = BAL_FSM_SHORTTIME_100ms;

    /* FR-3.3.1: 若均衡不被允许（局部或全局），停用均衡 */
    if ((bal_state.balancingAllowed == false) || (bal_state.balancingGlobalAllowed == false)) {
        BAL_Deactivate();
        bal_state.active = false;
    } else {
        /* FR-3.3.1: 仅当电池处于静置状态时进入均衡 */
        if (BMS_GetBatterySystemState() == BMS_AT_REST) {
            bal_state.state    = BAL_FSM_BALANCE;
            bal_state.substate = BAL_ENTRY;
        }
    }
}

static void BAL_ProcessStateBalancing(BAL_STATE_REQUEST_e state_request) {
    if (state_request == BAL_STATE_NO_BALANCING_REQUEST) {
        bal_state.balancingAllowed = false;
    }
    if (state_request == BAL_STATE_ALLOW_BALANCING_REQUEST) {
        bal_state.balancingAllowed = true;
    }

    /* FR-3.4.1: 全局禁用优先处理 — 任何子状态前检查 */
    if (bal_state.balancingGlobalAllowed == false) {
        if (bal_state.active == true) {
            BAL_Deactivate();
        }
        bal_state.active   = false;
        bal_state.state    = BAL_FSM_CHECK_BALANCING;
        bal_state.substate = BAL_ENTRY;
        bal_state.timer    = BAL_FSM_SHORTTIME_100ms;
        return;
    }

    /* FR-3.4.2: ENTRY 子状态 — 检查均衡许可标志 */
    if (bal_state.substate == BAL_ENTRY) {
        if (bal_state.balancingAllowed == false) {
            if (bal_state.active == true) {
                BAL_Deactivate();
            }
            bal_state.active   = false;
            bal_state.state    = BAL_FSM_CHECK_BALANCING;
            bal_state.substate = BAL_ENTRY;
        } else {
            bal_state.substate = BAL_CHECK_LOWEST_VOLTAGE;
        }
        bal_state.timer = BAL_FSM_SHORTTIME_100ms;
        return;
    } else if (bal_state.substate == BAL_CHECK_LOWEST_VOLTAGE) {
        /* FR-3.4.3: 检查最低电压和最高温度是否在安全范围内 */
        bal_state.substate               = BAL_CHECK_CURRENT;
        DATA_BLOCK_MIN_MAX_s checkMinMax = {.header.uniqueId = DATA_BLOCK_ID_MIN_MAX};
        DATA_READ_DATA(&checkMinMax);
        /* FR-3.4.3: 若电压低于下限或温度超过上限，停止均衡 */
        for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
            if ((checkMinMax.minimumCellVoltage_mV[s] <= BAL_LOWER_VOLTAGE_LIMIT_mV) ||
                (checkMinMax.maximumTemperature_ddegC[s] >= BAL_UPPER_TEMPERATURE_LIMIT_ddegC)) {
                if (bal_state.active == true) {
                    BAL_Deactivate();
                }
                bal_state.state    = BAL_FSM_CHECK_BALANCING;
                bal_state.substate = BAL_ENTRY;
            }
        }
        bal_state.timer = BAL_FSM_BALANCING_TIME_100ms;
        return;
    } else if (bal_state.substate == BAL_CHECK_CURRENT) {
        /* FR-3.4.4: 确认电池系统仍处于静置状态 */
        if (BMS_GetBatterySystemState() == BMS_AT_REST) {
            bal_state.substate = BAL_ACTIVATE_BALANCING;
        } else {
            if (bal_state.active == true) {
                BAL_Deactivate();
            }
            bal_state.state    = BAL_FSM_CHECK_BALANCING;
            bal_state.substate = BAL_ENTRY;
        }
        bal_state.timer = BAL_FSM_BALANCING_TIME_100ms;
        return;
    } else if (bal_state.substate == BAL_ACTIVATE_BALANCING) {
        /* FR-3.4.5: 激活均衡 — 均衡完成时施加迟滞，未完成则继续 */
        if (bal_state.balancingAllowed == false) {
            if (bal_state.active == true) {
                BAL_Deactivate();
            }
            bal_state.active   = false;
            bal_state.state    = BAL_FSM_CHECK_BALANCING;
            bal_state.substate = BAL_ENTRY;
        } else {
            if (BAL_ActivateBalancing() == true) {
                /* FR-3.4.5: 均衡完成，施加迟滞防止过早重新激活 */
                bal_state.balancingThreshold = BAL_GetBalancingThreshold_mV() + BAL_HYSTERESIS_mV;
                bal_state.state              = BAL_FSM_CHECK_BALANCING;
                bal_state.substate           = BAL_ENTRY;
            } else {
                /* FR-3.4.5: 均衡未完成，保持在 BALANCE 状态继续下一轮 */
                bal_state.state    = BAL_FSM_BALANCE;
                bal_state.substate = BAL_ENTRY;
            }
        }
        bal_state.timer = BAL_FSM_BALANCING_TIME_100ms;
        return;
    }
}

/*========== Extern Function Implementations ================================*/
/**
 * @req FR-3.6.1 — 初始化状态查询
 */
extern STD_RETURN_TYPE_e BAL_GetInitializationState(void) {
    return bal_state.initializationFinished;
}

extern BAL_RETURN_TYPE_e BAL_SetStateRequest(BAL_STATE_REQUEST_e stateRequest) {
    BAL_RETURN_TYPE_e returnValue = BAL_OK;

    /* NFR-4.3.1: 临界区保护 — 确保请求校验和写入的原子性 */
    OS_EnterTaskCritical();
    returnValue = BAL_CheckStateRequest(&bal_state, stateRequest);

    /* FR-3.5.4: 仅当校验通过时写入请求 */
    if (returnValue == BAL_OK) {
        bal_state.stateRequest = stateRequest;
    }
    OS_ExitTaskCritical();

    return returnValue;
}

extern void BAL_Trigger(void) {
    BAL_STATE_REQUEST_e stateRequest = BAL_STATE_NO_REQUEST;

    /* FR-3.5.1: 重入检查 — 检测到重入则立即返回 */
    if (BAL_CheckReEntrance(&bal_state) > 0u) {
        return;
    }

    /* FR-3.5.2: 定时器倒计时管理 — 定时器未归零则跳过本次处理 */
    if (bal_state.timer > 0u) {
        if ((--bal_state.timer) > 0) {
            bal_state.triggerEntry--;
            return; /* handle state machine only if timer has elapsed */
        }
    }

    /* FR-3.5.3: 状态分发 */
    switch (bal_state.state) {
        case BAL_FSM_UNINITIALIZED:
            BAL_SaveLastStates(&bal_state);
            stateRequest = BAL_TransferStateRequest(&bal_state);
            BAL_ProcessStateUninitialized(&bal_state, stateRequest);
            break;
        case BAL_FSM_INITIALIZATION:
            BAL_SaveLastStates(&bal_state);
            BAL_Init(&bal_tableBalancingControl);
            BAL_ProcessStateInitialization(&bal_state);
            break;
        case BAL_FSM_INITIALIZED:
            BAL_SaveLastStates(&bal_state);
            BAL_ProcessStateInitialized(&bal_state);
            break;
        case BAL_FSM_CHECK_BALANCING:
            /* FR-3.3.1: 均衡条件检查 */
            BAL_SaveLastStates(&bal_state);
            stateRequest = BAL_TransferStateRequest(&bal_state);
            BAL_ProcessStateCheckBalancing(stateRequest);
            break;
        case BAL_FSM_BALANCE:
            /* FR-3.4.x: 均衡执行状态机 */
            BAL_SaveLastStates(&bal_state);
            /* Check if balancing is still allowed */
            stateRequest = BAL_TransferStateRequest(&bal_state);
            BAL_ProcessStateBalancing(stateRequest);
            break;
        default:
            /* NFR-4.3.2: 非法状态断言保护 */
            FAS_ASSERT(FAS_TRAP);
            break;
    }
    bal_state.triggerEntry--;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
/** @req NFR-4.5.1 — 单元测试支持：通过条件编译暴露内部函数和变量 */
#ifdef UNITY_UNIT_TEST
extern BAL_FSM_e BAL_GetState(void) {
    /* NFR-4.5.1: 暴露状态机当前状态供测试验证 */
    return bal_state.state;
}

extern DATA_BLOCK_BALANCING_CONTROL_s *TEST_BAL_GetBalancingControl(void) {
    /* NFR-4.5.1: 暴露均衡控制数据块供测试验证 */
    return &bal_tableBalancingControl;
}

extern BAL_STATE_s *TEST_BAL_GetBalancingState(void) {
    /* NFR-4.5.1: 暴露均衡状态结构体供测试验证 */
    return &bal_state;
}
#endif
