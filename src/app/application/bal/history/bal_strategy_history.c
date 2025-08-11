/**
 *
 * @copyright &copy; 2010 - 2025, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
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
static DATA_BLOCK_BALANCING_CONTROL_s bal_balancing = {.header.uniqueId = DATA_BLOCK_ID_BALANCING_CONTROL};
/** local storage of the #DATA_BLOCK_CELL_VOLTAGE_s table */
static DATA_BLOCK_CELL_VOLTAGE_s bal_cellVoltage = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE};

/** contains the state of the contactor state machine */
static BAL_STATE_s bal_state = {
    .timer                  = 0,
    .stateRequest           = BAL_STATE_NO_REQUEST,
    .state                  = BAL_STATEMACH_UNINITIALIZED,
    .substate               = BAL_ENTRY,
    .lastState              = BAL_STATEMACH_UNINITIALIZED,
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
/** Activates history based balancing */
static void BAL_ActivateBalancing(void);

/**
 * @brief   Deactivates history based balancing
 * @details The balancing state of all cells in all strings set to inactivate
 *          (that is 0) and the delta charge is set to 0 As. The balancing
 *          enable bit is deactivate (that is 0).
 */
static void BAL_Deactivate(void);

/**
 * @brief   State machine subfunction to check if balancing is allowed
 * @details Checks if balancing is allowed. If it is it transfers in the actual
 *          balancing state.
 */
static void BAL_ProcessStateCheckBalancing(void);

/** State machine subfunction to balance the battery cell */
static void BAL_ProcessStateBalancing(void);

/** State machine subfunction to check for voltage imbalances */
static bool BAL_CheckImbalances(void);

/** State machine subfunction to compute the imbalance of all cells */
static void BAL_ComputeImbalances(void);

/*========== Static Function Implementations ================================*/

static void BAL_ActivateBalancing(void) {
    float_t cellBalancingCurrent = 0.0f;
    uint32_t difference          = 0;

    DATA_READ_DATA(&bal_balancing, &bal_cellVoltage);

    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        uint16_t nrBalancedCells = 0u;
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint16_t cb = 0u; cb < BS_NR_OF_CELL_BLOCKS_PER_MODULE; cb++) {
                if (bal_state.balancingAllowed == false) {
                    bal_balancing.activateBalancing[s][m][cb] = false;
                } else {
                    if (bal_balancing.deltaCharge_mAs[s][m][cb] > 0u) {
                        bal_balancing.activateBalancing[s][m][cb] = true;
                        nrBalancedCells++;
                        cellBalancingCurrent = ((float_t)(bal_cellVoltage.cellVoltage_mV[s][m][cb])) /
                                               SLV_BALANCING_RESISTANCE_ohm;
                        difference = (BAL_STATEMACH_BALANCING_TIME_100ms / 10u) * (uint32_t)(cellBalancingCurrent);
                        bal_state.active              = true;
                        bal_balancing.enableBalancing = true;
                        /* we are working with unsigned integers */
                        if (difference > bal_balancing.deltaCharge_mAs[s][m][cb]) {
                            bal_balancing.deltaCharge_mAs[s][m][cb] = 0u;
                        } else {
                            bal_balancing.deltaCharge_mAs[s][m][cb] -= difference;
                        }
                    } else {
                        bal_balancing.activateBalancing[s][m][cb] = false;
                    }
                }
            }
        }
        bal_balancing.nrBalancedCells[s] = nrBalancedCells;
    }

    DATA_WRITE_DATA(&bal_balancing);
}

static void BAL_Deactivate(void) {
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint16_t cb = 0u; cb < BS_NR_OF_CELL_BLOCKS_PER_MODULE; cb++) {
                bal_balancing.activateBalancing[s][m][cb] = false;
                bal_balancing.deltaCharge_mAs[s][m][cb]   = 0u;
            }
        }
        bal_balancing.nrBalancedCells[s] = 0u;
    }
    bal_balancing.enableBalancing = false;
    bal_state.active              = false;

    DATA_WRITE_DATA(&bal_balancing);
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

        bal_state.timer = BAL_STATEMACH_SHORTTIME_100ms;
        return;
    } else if (bal_state.substate == BAL_CHECK_IMBALANCES) {
        if (bal_state.active == true) {
            BAL_Deactivate();
        }
        if (BAL_CheckImbalances() == true) {
            bal_state.state    = BAL_STATEMACH_BALANCE;
            bal_state.substate = BAL_ENTRY;
        } else {
            bal_state.substate = BAL_COMPUTE_IMBALANCES;
        }
        bal_state.timer = BAL_STATEMACH_SHORTTIME_100ms;
        return;
    } else if (bal_state.substate == BAL_COMPUTE_IMBALANCES) {
        if (BMS_GetBatterySystemState() == BMS_AT_REST) {
            BAL_ComputeImbalances();
            bal_state.state    = BAL_STATEMACH_BALANCE;
            bal_state.substate = BAL_ENTRY;
        } else {
            bal_state.substate = BAL_CHECK_IMBALANCES;
        }
        bal_state.timer = BAL_STATEMACH_SHORTTIME_100ms;
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
            bal_state.substate = (BAL_STATEMACH_SUB_e)BAL_STATEMACH_CHECK_BALANCING;
        } else {
            bal_state.substate = BAL_ACTIVATE_BALANCING;
        }
        bal_state.timer = BAL_STATEMACH_SHORTTIME_100ms;
        return;
    } else if (bal_state.substate == BAL_ACTIVATE_BALANCING) {
        DATA_BLOCK_MIN_MAX_s bal_minMax = {.header.uniqueId = DATA_BLOCK_ID_MIN_MAX};
        DATA_READ_DATA(&bal_minMax);
        bal_state.timer = BAL_STATEMACH_BALANCING_TIME_100ms;
        /* do not balance under a certain voltage level */
        for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
            if ((bal_minMax.minimumCellVoltage_mV[s] <= BAL_LOWER_VOLTAGE_LIMIT_mV) ||
                (bal_minMax.maximumTemperature_ddegC[s] >= BAL_UPPER_TEMPERATURE_LIMIT_ddegC) ||
                (BAL_CheckImbalances() == false) || (bal_state.balancingGlobalAllowed == false)) {
                activateBalancing = false;
                if (bal_state.active == true) {
                    BAL_Deactivate();
                }
                bal_state.state    = BAL_STATEMACH_CHECK_BALANCING;
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
    bool returnValue = false;

    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint16_t cb = 0u; cb < BS_NR_OF_CELL_BLOCKS_PER_MODULE; cb++) {
                if (bal_balancing.deltaCharge_mAs[s][m][cb] > 0) {
                    returnValue = true;
                }
            }
        }
    }
    return returnValue;
}

static void BAL_ComputeImbalances(void) {
    int16_t voltageMin_mV                   = 0;
    uint16_t minVoltageModuleIndex          = 0u;
    uint16_t minVoltageModuleCellBlockIndex = 0u;
    float_t SOC                             = 0.0f;
    uint32_t DOD                            = 0u;
    uint32_t maxDOD                         = 0u;

    DATA_READ_DATA(&bal_balancing, &bal_cellVoltage);

    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        /* Assign first cell voltage to*/
        voltageMin_mV                  = INT16_MAX;
        minVoltageModuleIndex          = 0u;
        minVoltageModuleCellBlockIndex = 0u;
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint16_t cb = 0u; cb < BS_NR_OF_CELL_BLOCKS_PER_MODULE; cb++) {
                if (bal_cellVoltage.cellVoltage_mV[s][m][cb] <= voltageMin_mV) {
                    voltageMin_mV                  = bal_cellVoltage.cellVoltage_mV[s][m][cb];
                    minVoltageModuleIndex          = m;
                    minVoltageModuleCellBlockIndex = cb;
                }
            }
        }

        SOC = SE_GetStateOfChargeFromVoltage(
            ((float_t)(bal_cellVoltage.cellVoltage_mV[s][minVoltageModuleIndex][minVoltageModuleCellBlockIndex])) /
            1000.0f);
        maxDOD = BC_CAPACITY_mAh * (uint32_t)((1.0f - SOC) * 3600.0f);
        bal_balancing.deltaCharge_mAs[s][minVoltageModuleIndex][minVoltageModuleCellBlockIndex] = 0u;

        /* update balancing threshold */
        bal_state.balancingThreshold = BAL_GetBalancingThreshold_mV() + BAL_HYSTERESIS_mV;

        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint16_t cb = 0u; cb < BS_NR_OF_CELL_BLOCKS_PER_MODULE; cb++) {
                if ((m != minVoltageModuleIndex) || (cb != minVoltageModuleCellBlockIndex)) {
                    if (bal_cellVoltage.cellVoltage_mV[s][m][cb] >= (voltageMin_mV + bal_state.balancingThreshold)) {
                        SOC = SE_GetStateOfChargeFromVoltage(
                            ((float_t)(bal_cellVoltage.cellVoltage_mV[s][m][cb])) / 1000.0f);
                        DOD                                     = BC_CAPACITY_mAh * (uint32_t)((1.0f - SOC) * 3600.0f);
                        bal_balancing.deltaCharge_mAs[s][m][cb] = (maxDOD - DOD);
                    }
                }
            }
        }
    }

    DATA_WRITE_DATA(&bal_balancing);
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
        case BAL_STATEMACH_UNINITIALIZED:
            BAL_SaveLastStates(&bal_state);
            stateRequest = BAL_TransferStateRequest(&bal_state);
            BAL_ProcessStateUninitialized(&bal_state, stateRequest);
            break;
        case BAL_STATEMACH_INITIALIZATION:
            BAL_SaveLastStates(&bal_state);
            BAL_Init(&bal_balancing);
            BAL_ProcessStateInitialization(&bal_state);
            break;
        case BAL_STATEMACH_INITIALIZED:
            BAL_SaveLastStates(&bal_state);
            BAL_ProcessStateInitialized(&bal_state);
            break;
        case BAL_STATEMACH_CHECK_BALANCING:
            BAL_SaveLastStates(&bal_state);
            BAL_ProcessStateCheckBalancing();
            break;
        case BAL_STATEMACH_BALANCE:
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
extern BAL_STATEMACH_e BAL_GetState(void) {
    return bal_state.state;
}

extern DATA_BLOCK_BALANCING_CONTROL_s *TEST_BAL_GetBalancingControl(void) {
    return &bal_balancing;
}

extern BAL_STATE_s *TEST_BAL_GetBalancingState(void) {
    return &bal_state;
}
#endif
