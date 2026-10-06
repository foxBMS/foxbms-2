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
 * @file    bal_strategy_history.c
 * @author  foxBMS Team
 * @date    2020-05-29 (date of creation)
 * @updated 2026-10-06 (date of last update)
 * @version v1.12.0
 * @ingroup APPLICATION
 * @prefix  BAL
 *
 * @brief   Driver for the Balancing module
 * @details TODO
 */

/*========== Includes =======================================================*/
#include "battery_cell_cfg.h"
#include "bms-slave_cfg.h"

#include "bal.h"
#include "bms.h"
#include "database.h"
#include "os.h"
#include "state_estimation.h"

#include <math.h>
#include <stdbool.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/
/** local storage of the #DATA_BLOCK_BALANCING_CONTROL_s table */
static DATA_BLOCK_BALANCING_CONTROL_s bal_tableBalancingControl = {.header.uniqueId = DATA_BLOCK_ID_BALANCING_CONTROL};
/** local storage of the #DATA_BLOCK_CELL_VOLTAGE_s table */
static DATA_BLOCK_CELL_VOLTAGE_s bal_tableCellVoltage = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE};

/** contains the state of the contactor state machine */
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
 * @brief   Activate history-based balancing for eligible cells
 * @details Decrease the charge difference of each cell that is currently
 *          being balanced.
 */
static void BAL_ActivateBalancing(void);

/**
 * @brief   Deactivate history-based balancing
 * @details Deactivate balancing for all cells, clear their charge
 *          differences, and clear the balancing enable state.
 */
static void BAL_Deactivate(void);

/**
 * @brief   Process the balancing permission state
 * @details Handle global balancing permission and transfer the state
 *          machine to imbalance checking or deactivate balancing.
 */
static void BAL_ProcessStateCheckBalancing(void);

/**
 * @brief   Process the history-based balancing state
 * @details Check safety conditions and activate balancing for eligible
 *          cells.
 */
static void BAL_ProcessStateBalancing(void);

/**
 * @brief   Check for existing balancing imbalances
 * @details The balancing calculation stores the remaining charge to remove
 *          from each eligible cell in
 *          #DATA_BLOCK_BALANCING_CONTROL_s::deltaCharge_mAs.
 *          Scan all cells in all strings and return true when at least one
 *          cell has a positive remaining charge difference.
 *          This allows the state machine to continue balancing without
 *          recalculating imbalances. The table is not modified by this
 *          check.
 */
static bool BAL_CheckImbalances(void);

/**
 * @brief   Compute charge differences for history-based balancing
 * @details The cell with the lowest relaxed voltage is used as the reference
 *          because it represents the most discharged cell in the string. The
 *          relaxed voltage of every cell is converted to a state-of-charge
 *          value. The difference between the reference cell's estimated
 *          depth of discharge and each eligible cell's depth of discharge is
 *          stored in mAs, which gives the balancing state machine the amount
 *          of charge that has to be removed from that cell. The balancing
 *          threshold and hysteresis prevent cells with insignificant voltage
 *          differences from being balanced.
 */
static void BAL_ComputeImbalances(void);

/*========== Static Function Implementations ================================*/

static void BAL_ActivateBalancing(void) {
    float_t cellBalancingCurrent = 0.0f;
    uint32_t difference          = 0;

    DATA_READ_DATA(&bal_tableCellVoltage, &bal_tableBalancingControl);

    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        uint16_t nrBalancedCells = 0u;
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t cb = 0u; cb < BS_NR_OF_CELL_BLOCKS_PER_MODULE; cb++) {
                if (bal_state.balancingAllowed == false) {
                    bal_tableBalancingControl.activateBalancing[s][m][cb] = false;
                } else {
                    const bool mustBalance = bal_tableBalancingControl.deltaCharge_mAs[s][m][cb] > 0u;
                    if (mustBalance) {
                        bal_tableBalancingControl.activateBalancing[s][m][cb] = true;
                        nrBalancedCells++;
                        cellBalancingCurrent = ((float_t)(bal_tableCellVoltage.cellVoltage_mV[s][m][cb])) /
                                               SLV_BALANCING_RESISTANCE_ohm;
                        difference           = (BAL_FSM_BALANCING_TIME_100ms / 10u) * (uint32_t)(cellBalancingCurrent);
                        bal_state.active     = true;
                        bal_tableBalancingControl.enableBalancing = true;
                        /* we are working with unsigned integers */
                        if (difference > bal_tableBalancingControl.deltaCharge_mAs[s][m][cb]) {
                            bal_tableBalancingControl.deltaCharge_mAs[s][m][cb] = 0u;
                        } else {
                            bal_tableBalancingControl.deltaCharge_mAs[s][m][cb] -= difference;
                        }
                    } else {
                        bal_tableBalancingControl.activateBalancing[s][m][cb] = false;
                    }
                }
            }
        }
        bal_tableBalancingControl.nrBalancedCells[s] = nrBalancedCells;
    }

    DATA_WRITE_DATA(&bal_tableBalancingControl);
}

static void BAL_Deactivate(void) {
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t cb = 0u; cb < BS_NR_OF_CELL_BLOCKS_PER_MODULE; cb++) {
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

static void BAL_ProcessStateCheckBalancing(void) {
    if (bal_state.substate == BAL_ENTRY) {
        if (bal_state.balancingGlobalAllowed == false) {
            if (bal_state.active == true) {
                BAL_Deactivate();
            }
            bal_state.active   = false;
            bal_state.substate = BAL_ENTRY;
        } else {
            bal_state.substate = BAL_CHECK_IMBALANCES;
        }

        bal_state.timer = BAL_FSM_SHORTTIME_100ms;
        return;
    } else if (bal_state.substate == BAL_CHECK_IMBALANCES) {
        if (bal_state.active == true) {
            BAL_Deactivate();
        }
        if (BAL_CheckImbalances() == true) {
            bal_state.state    = BAL_FSM_BALANCE;
            bal_state.substate = BAL_ENTRY;
        } else {
            bal_state.substate = BAL_COMPUTE_IMBALANCES;
        }
        bal_state.timer = BAL_FSM_SHORTTIME_100ms;
        return;
    } else if (bal_state.substate == BAL_COMPUTE_IMBALANCES) {
        if (BMS_GetBatterySystemState() == BMS_AT_REST) {
            BAL_ComputeImbalances();
            bal_state.state    = BAL_FSM_BALANCE;
            bal_state.substate = BAL_ENTRY;
        } else {
            bal_state.substate = BAL_CHECK_IMBALANCES;
        }
        bal_state.timer = BAL_FSM_SHORTTIME_100ms;
        return;
    }
}

static void BAL_ProcessStateBalancing(void) {
    bool activateBalancing = true;

    if (bal_state.substate == BAL_ENTRY) {
        if (bal_state.balancingGlobalAllowed == false) {
            if (bal_state.active == true) {
                BAL_Deactivate();
            }
            bal_state.active   = false;
            bal_state.substate = (BAL_FSM_SUB_e)BAL_FSM_CHECK_BALANCING;
        } else {
            bal_state.substate = BAL_ACTIVATE_BALANCING;
        }
        bal_state.timer = BAL_FSM_SHORTTIME_100ms;
        return;
    } else if (bal_state.substate == BAL_ACTIVATE_BALANCING) {
        DATA_BLOCK_MIN_MAX_s bal_minMax = {.header.uniqueId = DATA_BLOCK_ID_MIN_MAX};
        DATA_READ_DATA(&bal_minMax);
        bal_state.timer = BAL_FSM_BALANCING_TIME_100ms;
        /* do not balance under a certain voltage level */
        for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
            if ((bal_minMax.minimumCellVoltage_mV[s] <= BAL_LOWER_VOLTAGE_LIMIT_mV) ||
                (bal_minMax.maximumTemperature_ddegC[s] >= BAL_UPPER_TEMPERATURE_LIMIT_ddegC) ||
                (BAL_CheckImbalances() == false) || (bal_state.balancingGlobalAllowed == false)) {
                activateBalancing = false;
                if (bal_state.active == true) {
                    BAL_Deactivate();
                }
                bal_state.state    = BAL_FSM_CHECK_BALANCING;
                bal_state.substate = BAL_ENTRY;
                return;
            }
        }

        if (activateBalancing == true) {
            BAL_ActivateBalancing();
        }
        return;
    }
}
static bool BAL_CheckImbalances(void) {
    bool imbalancesExist = false;

    /* A positive charge difference means that this cell still needs balancing. */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t cb = 0u; cb < BS_NR_OF_CELL_BLOCKS_PER_MODULE; cb++) {
                if (bal_tableBalancingControl.deltaCharge_mAs[s][m][cb] > 0) {
                    /* Record remaining work and continue checking the other cells. */
                    imbalancesExist = true;
                }
            }
        }
    }
    return imbalancesExist;
}

static void BAL_ComputeImbalances(void) {
    int16_t minimumCellVoltage_mV              = 0;
    int16_t cellVoltage_mV                     = 0;
    int16_t minimumCellVoltageWithThreshold_mV = 0;
    uint16_t minimumVoltageModuleIndex         = 0u;
    uint16_t minimumVoltageCellBlockIndex      = 0u;
    float_t stateOfChargeFraction              = 0.0f;
    uint32_t depthOfDischarge_mAs              = 0u;
    uint32_t maximumDepthOfDischarge_mAs       = 0u;
    uint32_t chargeDifference_mAs              = 0u;

    DATA_READ_DATA(&bal_tableBalancingControl, &bal_tableCellVoltage);

    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        /* Find the most discharged cell to use as the string reference. */
        minimumCellVoltage_mV        = INT16_MAX;
        minimumVoltageModuleIndex    = 0u;
        minimumVoltageCellBlockIndex = 0u;
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t cb = 0u; cb < BS_NR_OF_CELL_BLOCKS_PER_MODULE; cb++) {
                if (bal_tableCellVoltage.cellVoltage_mV[s][m][cb] <= minimumCellVoltage_mV) {
                    minimumCellVoltage_mV        = bal_tableCellVoltage.cellVoltage_mV[s][m][cb];
                    minimumVoltageModuleIndex    = m;
                    minimumVoltageCellBlockIndex = cb;
                }
            }
        }

        /* Estimate the reference cell's remaining charge from its relaxed
         * voltage. Its depth of discharge is the maximum required balancing
         * amount against which all other cells in this string are compared.
         */
        const int16_t referenceCellVoltage_mV =
            bal_tableCellVoltage.cellVoltage_mV[s][minimumVoltageModuleIndex][minimumVoltageCellBlockIndex];
        stateOfChargeFraction       = SE_GetStateOfChargeFromVoltage(referenceCellVoltage_mV) / 100.0f;
        maximumDepthOfDischarge_mAs = BC_CAPACITY_mAh * (uint32_t)((1.0f - stateOfChargeFraction) * 3600.0f);
        bal_tableBalancingControl.deltaCharge_mAs[s][minimumVoltageModuleIndex][minimumVoltageCellBlockIndex] = 0u;

        /* Ignore voltage differences smaller than the balancing hysteresis. */
        bal_state.balancingThreshold       = BAL_GetBalancingThreshold_mV() + BAL_HYSTERESIS_mV;
        minimumCellVoltageWithThreshold_mV = minimumCellVoltage_mV + bal_state.balancingThreshold;

        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t cb = 0u; cb < BS_NR_OF_CELL_BLOCKS_PER_MODULE; cb++) {
                if ((m != minimumVoltageModuleIndex) || (cb != minimumVoltageCellBlockIndex)) {
                    cellVoltage_mV = bal_tableCellVoltage.cellVoltage_mV[s][m][cb];
                    if (cellVoltage_mV >= minimumCellVoltageWithThreshold_mV) {
                        /* A higher state of charge means this cell has more
                         * charge to remove than the reference cell. Store the
                         * difference as the balancing work in mAs.
                         */
                        stateOfChargeFraction = SE_GetStateOfChargeFromVoltage(cellVoltage_mV) / 100.0f;
                        depthOfDischarge_mAs  = BC_CAPACITY_mAh * (uint32_t)((1.0f - stateOfChargeFraction) * 3600.0f);
                        chargeDifference_mAs  = maximumDepthOfDischarge_mAs - depthOfDischarge_mAs;
                        bal_tableBalancingControl.deltaCharge_mAs[s][m][cb] = chargeDifference_mAs;
                    }
                }
            }
        }
    }

    DATA_WRITE_DATA(&bal_tableBalancingControl);
}

/*========== Extern Function Implementations ================================*/
extern STD_RETURN_TYPE_e BAL_GetInitializationState(void) {
    return bal_state.initializationFinished;
}

extern BAL_RETURN_TYPE_e BAL_SetStateRequest(BAL_STATE_REQUEST_e stateRequest) {
    BAL_RETURN_TYPE_e returnValue = BAL_OK;

    OS_EnterTaskCritical();
    returnValue = BAL_CheckStateRequest(&bal_state, stateRequest);

    if (returnValue == BAL_OK) {
        bal_state.stateRequest = stateRequest;
    }
    OS_ExitTaskCritical();

    return returnValue;
}

extern void BAL_Trigger(void) {
    BAL_STATE_REQUEST_e stateRequest = BAL_STATE_NO_REQUEST;

    /* Check re-entrance of function */
    if (BAL_CheckReEntrance(&bal_state) > 0u) {
        return;
    }

    if (bal_state.timer > 0u) {
        if ((--bal_state.timer) > 0) {
            bal_state.triggerEntry--;
            return; /* handle state machine only if timer has elapsed */
        }
    }

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
            BAL_SaveLastStates(&bal_state);
            BAL_ProcessStateCheckBalancing();
            break;
        case BAL_FSM_BALANCE:
            BAL_SaveLastStates(&bal_state);
            BAL_ProcessStateBalancing();
            break;
        default:
            /* invalid state */
            FAS_ASSERT(FAS_TRAP);
            break;
    }
    bal_state.triggerEntry--;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
extern BAL_FSM_e BAL_GetState(void) {
    return bal_state.state;
}

extern DATA_BLOCK_BALANCING_CONTROL_s *TEST_BAL_GetBalancingControl(void) {
    return &bal_tableBalancingControl;
}

extern DATA_BLOCK_CELL_VOLTAGE_s *TEST_BAL_GetCellVoltage(void) {
    return &bal_tableCellVoltage;
}

extern void TEST_BAL_ComputeImbalances(void) {
    BAL_ComputeImbalances();
}

extern BAL_STATE_s *TEST_BAL_GetBalancingState(void) {
    return &bal_state;
}
#endif
