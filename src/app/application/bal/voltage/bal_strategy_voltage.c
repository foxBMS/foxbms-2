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
 * @file    bal_strategy_voltage.c
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
#include "bal_strategy_voltage.h"

#include "battery_cell_cfg.h"

#include "bms.h"
#include "database.h"
#include "os.h"

#include <stdbool.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/
/** local storage of the #DATA_BLOCK_BALANCING_CONTROL_s table */
static DATA_BLOCK_BALANCING_CONTROL_s bal_balancing = {.header.uniqueId = DATA_BLOCK_ID_BALANCING_CONTROL};

/**
 * @brief   contains the state of the contactor state machine
 */
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
/**
 * @brief   Activates voltage based balancing
 * @details TODO
 */
static bool BAL_ActivateBalancing(void);

/**
 * @brief   Deactivates voltage based balancing
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
static void BAL_ProcessStateCheckBalancing(BAL_STATE_REQUEST_e state_request);

/**
 * @brief   State machine subfunction to balance the battery cell
 * @details TODO
 */
static void BAL_ProcessStateBalancing(BAL_STATE_REQUEST_e state_request);

/*========== Static Function Implementations ================================*/
static bool BAL_ActivateBalancing(void) {
    bool finished               = true;
    DATA_BLOCK_MIN_MAX_s minMax = {.header.uniqueId = DATA_BLOCK_ID_MIN_MAX};
    /* Database entry is declared static, to place it in the data segment and not on the stack */
    static DATA_BLOCK_CELL_VOLTAGE_s cellVoltage = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE};

    DATA_READ_DATA(&cellVoltage, &minMax);

    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        int16_t min              = minMax.minimumCellVoltage_mV[s];
        uint16_t nrBalancedCells = 0u;
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t cb = 0u; cb < BS_NR_OF_CELL_BLOCKS_PER_MODULE; cb++) {
                if (cellVoltage.cellVoltage_mV[s][m][cb] > (min + bal_state.balancingThreshold)) {
                    bal_balancing.activateBalancing[s][m][cb] = true;
                    finished                                  = false;
                    /* set without hysteresis so that we now balance all cells that are below the initial threshold */
                    bal_state.balancingThreshold  = BAL_GetBalancingThreshold_mV();
                    bal_state.active              = true;
                    bal_balancing.enableBalancing = true;
                    nrBalancedCells++;
                } else {
                    bal_balancing.activateBalancing[s][m][cb] = false;
                }
            }
        }
        bal_balancing.nrBalancedCells[s] = nrBalancedCells;
    }
    DATA_WRITE_DATA(&bal_balancing);

    return finished;
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

static void BAL_ProcessStateCheckBalancing(BAL_STATE_REQUEST_e state_request) {
    if (state_request == BAL_STATE_NO_BALANCING_REQUEST) {
        bal_state.balancingAllowed = false;
    }
    if (state_request == BAL_STATE_ALLOW_BALANCING_REQUEST) {
        bal_state.balancingAllowed = true;
    }

    bal_state.timer = BAL_STATEMACH_SHORTTIME_100ms;

    if ((bal_state.balancingAllowed == false) || (bal_state.balancingGlobalAllowed == false)) {
        BAL_Deactivate();
        bal_state.active = false;
    } else {
        if (BMS_GetBatterySystemState() == BMS_AT_REST) {
            bal_state.state    = BAL_STATEMACH_BALANCE;
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

    if (bal_state.balancingGlobalAllowed == false) {
        if (bal_state.active == true) {
            BAL_Deactivate();
        }
        bal_state.active   = false;
        bal_state.state    = BAL_STATEMACH_CHECK_BALANCING;
        bal_state.substate = BAL_ENTRY;
        bal_state.timer    = BAL_STATEMACH_SHORTTIME_100ms;
        return;
    }

    if (bal_state.substate == BAL_ENTRY) {
        if (bal_state.balancingAllowed == false) {
            if (bal_state.active == true) {
                BAL_Deactivate();
            }
            bal_state.active   = false;
            bal_state.state    = BAL_STATEMACH_CHECK_BALANCING;
            bal_state.substate = BAL_ENTRY;
        } else {
            bal_state.substate = BAL_CHECK_LOWEST_VOLTAGE;
        }
        bal_state.timer = BAL_STATEMACH_SHORTTIME_100ms;
        return;
    } else if (bal_state.substate == BAL_CHECK_LOWEST_VOLTAGE) {
        bal_state.substate               = BAL_CHECK_CURRENT;
        DATA_BLOCK_MIN_MAX_s checkMinMax = {.header.uniqueId = DATA_BLOCK_ID_MIN_MAX};
        DATA_READ_DATA(&checkMinMax);
        /* stop balancing if minimum voltage is below minimum threshold or
         * maximum cell temperature breached upper temperature limit */
        for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
            if ((checkMinMax.minimumCellVoltage_mV[s] <= BAL_LOWER_VOLTAGE_LIMIT_mV) ||
                (checkMinMax.maximumTemperature_ddegC[s] >= BAL_UPPER_TEMPERATURE_LIMIT_ddegC)) {
                if (bal_state.active == true) {
                    BAL_Deactivate();
                }
                bal_state.state    = BAL_STATEMACH_CHECK_BALANCING;
                bal_state.substate = BAL_ENTRY;
            }
        }
        bal_state.timer = BAL_STATEMACH_BALANCING_TIME_100ms;
        return;
    } else if (bal_state.substate == BAL_CHECK_CURRENT) {
        if (BMS_GetBatterySystemState() == BMS_AT_REST) {
            bal_state.substate = BAL_ACTIVATE_BALANCING;
        } else {
            if (bal_state.active == true) {
                BAL_Deactivate();
            }
            bal_state.state    = BAL_STATEMACH_CHECK_BALANCING;
            bal_state.substate = BAL_ENTRY;
        }
        bal_state.timer = BAL_STATEMACH_BALANCING_TIME_100ms;
        return;
    } else if (bal_state.substate == BAL_ACTIVATE_BALANCING) {
        if (bal_state.balancingAllowed == false) {
            if (bal_state.active == true) {
                BAL_Deactivate();
            }
            bal_state.active   = false;
            bal_state.state    = BAL_STATEMACH_CHECK_BALANCING;
            bal_state.substate = BAL_ENTRY;
        } else {
            if (BAL_ActivateBalancing() == true) {
                /* set threshold with hysteresis in order to prevent too early re-enabling of balancing */
                bal_state.balancingThreshold = BAL_GetBalancingThreshold_mV() + BAL_HYSTERESIS_mV;
                bal_state.state              = BAL_STATEMACH_CHECK_BALANCING;
                bal_state.substate           = BAL_ENTRY;
            } else {
                bal_state.state    = BAL_STATEMACH_BALANCE;
                bal_state.substate = BAL_ENTRY;
            }
        }
        bal_state.timer = BAL_STATEMACH_BALANCING_TIME_100ms;
        return;
    }
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
            stateRequest = BAL_TransferStateRequest(&bal_state);
            BAL_ProcessStateCheckBalancing(stateRequest);
            break;
        case BAL_STATEMACH_BALANCE:
            BAL_SaveLastStates(&bal_state);
            /* Check if balancing is still allowed */
            stateRequest = BAL_TransferStateRequest(&bal_state);
            BAL_ProcessStateBalancing(stateRequest);
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
