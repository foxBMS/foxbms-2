/**
 *
 * @copyright &copy; 2010 - 2021, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * - &Prime;This product uses parts of foxBMS&reg;&Prime;
 * - &Prime;This product includes parts of foxBMS&reg;&Prime;
 * - &Prime;This product is derived from foxBMS&reg;&Prime;
 *
 */

/**
 * @file    debug_default.c
 * @author  foxBMS Team
 * @date    2020-09-17 (date of creation)
 * @updated 2021-06-09 (date of last update)
 * @ingroup DRIVER
 * @prefix  FAKE
 *
 * @brief   Driver implementation for the fake AFE
 *
 */

/*========== Includes =======================================================*/
#include "general.h"

#include "debug_default.h"

#include "battery_cell_cfg.h"
#include "battery_system_cfg.h"

#include "database.h"
#include "diag.h"
#include "os.h"

/*========== Macros and Definitions =========================================*/
/** faked cell voltage value for all cell voltages in mV */
#define FAKE_CELL_VOLTAGE_mV (BC_VOLTAGE_NOMINAL_mV)

/** faked cell temperature for all cell temperatures in deci &deg;C */
#define FAKE_CELL_TEMPERATURE_ddegC ((BC_TEMPERATURE_MAX_CHARGE_MOL_ddegC + BC_TEMPERATURE_MIN_CHARGE_MOL_ddegC) / 2u)

/**
 * statemachine short time definition in #FAKE_TriggerAfe calls
 * until next state is processed
 */
#define FAKE_FSM_SHORT_TIME (1u)

/**
 * statemachine medium time definition in #FAKE_TriggerAfe calls
 * until next state/substate is processed
 */
#define FAKE_FSM_MEDIUM_TIME (5u)

/**
 * statemachine long time definition in #FAKE_TriggerAfe calls
 * until next state/substate is processed
 */
#define FAKE_FSM_LONG_TIME (10u)

/** Symbolic names to check for multiple calls of #FAKE_TriggerAfe */
typedef enum FAKE_CHECK_MULTIPLE_CALLS {
    FAKE_MULTIPLE_CALLS_NO,  /*!< no multiple calls, OK */
    FAKE_MULTIPLE_CALLS_YES, /*!< multiple calls, not OK */
} FAKE_CHECK_MULTIPLE_CALLS_e;

/*========== Static Constant and Variable Definitions =======================*/

/** local copies of database tables */
/**@{*/
static DATA_BLOCK_CELL_VOLTAGE_s fake_cellVoltage         = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE_BASE};
static DATA_BLOCK_CELL_TEMPERATURE_s fake_cellTemperature = {.header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE_BASE};
static DATA_BLOCK_BALANCING_FEEDBACK_s fake_balancingFeedback = {
    .header.uniqueId = DATA_BLOCK_ID_BALANCING_FEEDBACK_BASE};
static DATA_BLOCK_BALANCING_CONTROL_s fake_balancingControl = {.header.uniqueId = DATA_BLOCK_ID_BALANCING_CONTROL};
static DATA_BLOCK_SLAVE_CONTROL_s fake_slaveControl         = {.header.uniqueId = DATA_BLOCK_ID_SLAVE_CONTROL};
static DATA_BLOCK_ALL_GPIO_VOLTAGES_s fake_allGpioVoltage   = {.header.uniqueId = DATA_BLOCK_ID_ALL_GPIO_VOLTAGES_BASE};
static DATA_BLOCK_OPEN_WIRE_s fake_openWire                 = {.header.uniqueId = DATA_BLOCK_ID_OPEN_WIRE_BASE};
/**@}*/

/*========== Extern Constant and Variable Definitions =======================*/

/** local instance of the driver-state */
FAKE_STATE_s fake_state = {
    .timer                    = 0,
    .triggerEntry             = 0,
    .nextState                = FAKE_FSM_STATE_HAS_NEVER_RUN,
    .currentState             = FAKE_FSM_STATE_HAS_NEVER_RUN,
    .previousState            = FAKE_FSM_STATE_HAS_NEVER_RUN,
    .nextSubstate             = FAKE_FSM_SUBSTATE_DUMMY,
    .currentSubstate          = FAKE_FSM_SUBSTATE_DUMMY,
    .previousSubstate         = FAKE_FSM_SUBSTATE_DUMMY,
    .firstMeasurementFinished = false,
    .data.allGpioVoltages     = &fake_allGpioVoltage,
    .data.balancingControl    = &fake_balancingControl,
    .data.balancingFeedback   = &fake_balancingFeedback,
    .data.cellTemperature     = &fake_cellTemperature,
    .data.cellVoltage         = &fake_cellVoltage,
    .data.openWire            = &fake_openWire,
    .data.slaveControl        = &fake_slaveControl,
};

/*========== Static Function Prototypes =====================================*/
/**
 * @brief   check for multiple calls of state machine trigger function
 * @details The trigger function is not reentrant, which means it cannot
 *          be called multiple times. This functions increments the
 *          triggerEntry counter once and must be called each time the
 *          trigger function is called. If triggerEntry is greater than
 *          one, there were multiple calls. For this function to work,
 *          triggerEntry must be decremented each time the trigger function
 *          is called, even if no processing do because the timer is
 *          non-zero.
 * @param   pFakeState state of the fake state machine
 * @return  true if there were multiple calls, false otherwise
 */
static bool FAKE_CheckMultipleCalls(FAKE_STATE_s *pFakeState);

/**
 * @brief   Sets the next state, the next substate and the timer value
 *          of the state variable.
 * @param   pFakeState     state of the example state machine
 * @param   nextState      state to be transferred into
 * @param   nextSubstate   substate to be transferred into
 * @param   idleTime       wait time for the state machine
 */
static void FAKE_SetState(
    FAKE_STATE_s *pFakeState,
    FAKE_FSM_STATES_e nextState,
    FAKE_FSM_SUBSTATES_e nextSubstate,
    uint16_t idleTime);

/**
 * @brief   Sets the next substate and the timer value
 *          of the state variable.
 * @param   pFakeState     state of the example state machine
 * @param   nextSubstate   substate to be transferred into
 * @param   idleTime       wait time for the state machine
 */
static void FAKE_SetSubstate(FAKE_STATE_s *pFakeState, FAKE_FSM_SUBSTATES_e nextSubstate, uint16_t idleTime);

/**
 * @brief   Sets the indicator that one full measurement cycles was successful
 * @param   pFakeState   state of the fake state machine
 * @return  true if it is a reentrance, false otherwise
 */
static void FAKE_SetFirstMeasurementCycleFinished(FAKE_STATE_s *pFakeState);

/**
 * @brief   Write voltage measurement data
 * @param   pFakeState   state of the fake state machine
 * @return  #STD_OK if successful, #STD_NOT_OK otherwise
 */
static STD_RETURN_TYPE_e FAKE_SaveFakeVoltageMeasurementData(FAKE_STATE_s *pFakeState);

/**
 * @brief   Write temperature measurement data
 * @param   pFakeState   state of the fake state machine
 * @return  #STD_OK if successful, #STD_NOT_OK otherwise
 */
static STD_RETURN_TYPE_e FAKE_SaveFakeTemperatureMeasurementData(FAKE_STATE_s *pFakeState);

/**
 * @brief   Processes the initialization state
 * @param   pFakeState   state of the fake state machine
 * @return  Always #STD_OK
 */
static FAKE_FSM_STATES_e FAKE_ProcessInitializationState(FAKE_STATE_s *pFakeState);

/**
 * @brief   Processes the running state
 * @param   pFakeState   state of the fake state machine
 * @return  Always #STD_OK
 */
static FAKE_FSM_STATES_e FAKE_ProcessRunningState(FAKE_STATE_s *pFakeState);

/**
 * @brief   Defines the state transitions
 * @details This function contains the implementation of the state
 *          machine, i.e., the sequence of states and substates.
 *          It is called by the trigger function every time
 *          the state machine timer has a non-zero value.
 * @param   pFakeState state of the example state machine
 * @return  Always #STD_OK
 */
static STD_RETURN_TYPE_e FAKE_RunStateMachine(FAKE_STATE_s *pFakeState);

/*========== Static Function Implementations ================================*/

static bool FAKE_CheckMultipleCalls(FAKE_STATE_s *pFakeState) {
    FAS_ASSERT(pFakeState != NULL_PTR);
    bool reentrance = false;
    OS_EnterTaskCritical();
    if (pFakeState->triggerEntry == 0u) {
        pFakeState->triggerEntry++;
    } else {
        reentrance = true; /* multiple calls of function */
    }
    OS_ExitTaskCritical();
    return reentrance;
}

static void FAKE_SetState(
    FAKE_STATE_s *pFakeState,
    FAKE_FSM_STATES_e nextState,
    FAKE_FSM_SUBSTATES_e nextSubstate,
    uint16_t idleTime) {
    FAS_ASSERT(pFakeState != NULL_PTR);
    bool earlyExit = false;

    pFakeState->timer = idleTime;

    if ((pFakeState->currentState == nextState) && (pFakeState->currentSubstate == nextSubstate)) {
        /* Next state and next substate equal to current state and substate: nothing to do */
        pFakeState->nextState    = FAKE_FSM_STATE_DUMMY;    /* no state transistion required -> reset */
        pFakeState->nextSubstate = FAKE_FSM_SUBSTATE_DUMMY; /* no substate transistion required -> reset */
        earlyExit                = true;
    }

    if (earlyExit == false) {
        if (pFakeState->currentState != nextState) {
            /* Next state is different: switch to it and set substate to entry value */
            pFakeState->previousState    = pFakeState->currentState;
            pFakeState->currentState     = nextState;
            pFakeState->previousSubstate = pFakeState->currentSubstate;
            pFakeState->currentSubstate  = FAKE_FSM_SUBSTATE_ENTRY; /* Use entry state after a top level state change */
            pFakeState->nextState        = FAKE_FSM_STATE_DUMMY;    /* no state transistion required -> reset */
            pFakeState->nextSubstate     = FAKE_FSM_SUBSTATE_DUMMY; /* no substate transistion required -> reset */
        } else if (pFakeState->currentSubstate != nextSubstate) {
            /* Only the next substate is different, switch to it */
            FAKE_SetSubstate(pFakeState, nextSubstate, idleTime);
        } else {
            ;
        }
    }
}

static void FAKE_SetSubstate(FAKE_STATE_s *pFakeState, FAKE_FSM_SUBSTATES_e nextSubstate, uint16_t idleTime) {
    FAS_ASSERT(pFakeState != NULL_PTR);
    pFakeState->timer            = idleTime;
    pFakeState->previousSubstate = pFakeState->currentSubstate;
    pFakeState->currentSubstate  = nextSubstate;
    pFakeState->nextSubstate = FAKE_FSM_SUBSTATE_DUMMY; /* substate has been set, now reset value for nextSubstate */
}

static void FAKE_SetFirstMeasurementCycleFinished(FAKE_STATE_s *pFakeState) {
    FAS_ASSERT(pFakeState != NULL_PTR);
    OS_EnterTaskCritical();

    uint16_t i = 0;

    for (uint8_t stringNumber = 0u; stringNumber < BS_NR_OF_STRINGS; stringNumber++) {
        pFakeState->data.cellVoltage->packVoltage_mV[stringNumber] = FAKE_CELL_VOLTAGE_mV * BS_NR_OF_BAT_CELLS;
        for (i = 0; i < BS_NR_OF_BAT_CELLS; i++) {
            pFakeState->data.cellVoltage->cellVoltage_mV[stringNumber][i] = FAKE_CELL_VOLTAGE_mV;
        }

        pFakeState->data.cellVoltage->state     = 0;
        pFakeState->data.cellTemperature->state = 0;
        for (i = 0; i < BS_NR_OF_TEMP_SENSORS_PER_STRING; i++) {
            pFakeState->data.cellTemperature->cellTemperature_ddegC[stringNumber][i] = FAKE_CELL_TEMPERATURE_ddegC;
        }

        pFakeState->data.balancingFeedback->state = 0;
        for (i = 0; i < BS_NR_OF_BAT_CELLS; i++) {
            pFakeState->data.balancingControl->balancingState[stringNumber][i] = 0;
        }
        pFakeState->data.balancingControl->nrBalancedCells[stringNumber] = 0u;
        for (i = 0; i < BS_NR_OF_MODULES; i++) {
            pFakeState->data.balancingFeedback->value[stringNumber][i] = 0;
        }

        pFakeState->data.slaveControl->state = 0;
        for (i = 0; i < BS_NR_OF_MODULES; i++) {
            pFakeState->data.slaveControl->ioValueIn[i]                 = 0;
            pFakeState->data.slaveControl->ioValueOut[i]                = 0;
            pFakeState->data.slaveControl->externalTemperatureSensor[i] = 0;
            pFakeState->data.slaveControl->eepromValueRead[i]           = 0;
            pFakeState->data.slaveControl->eepromValueWrite[i]          = 0;
        }
        pFakeState->data.slaveControl->eepromReadAddressLastUsed  = 0xFFFFFFFF;
        pFakeState->data.slaveControl->eepromReadAddressToUse     = 0xFFFFFFFF;
        pFakeState->data.slaveControl->eepromWriteAddressLastUsed = 0xFFFFFFFF;
        pFakeState->data.slaveControl->eepromWriteAddressToUse    = 0xFFFFFFFF;

        pFakeState->data.allGpioVoltages->state = 0;
        for (i = 0; i < (BS_NR_OF_MODULES * BS_NR_OF_GPIOS_PER_MODULE); i++) {
            pFakeState->data.allGpioVoltages->gpioVoltages_mV[stringNumber][i] = 0;
        }

        for (i = 0; i < (BS_NR_OF_MODULES * (BS_NR_OF_CELLS_PER_MODULE + 1)); i++) {
            pFakeState->data.openWire->openwire[stringNumber][i] = 0;
        }
        pFakeState->data.openWire->state = 0;
    }

    pFakeState->firstMeasurementFinished = true;
    OS_ExitTaskCritical();
    DATA_WRITE_DATA(
        pFakeState->data.cellVoltage,
        pFakeState->data.cellTemperature,
        pFakeState->data.balancingFeedback,
        pFakeState->data.balancingControl);
    DATA_WRITE_DATA(pFakeState->data.slaveControl, pFakeState->data.openWire);
}

static STD_RETURN_TYPE_e FAKE_SaveFakeVoltageMeasurementData(FAKE_STATE_s *pFakeState) {
    FAS_ASSERT(pFakeState != NULL_PTR);
    STD_RETURN_TYPE_e successfullSave = STD_OK;

    for (uint8_t stringNumber = 0u; stringNumber < BS_NR_OF_STRINGS; stringNumber++) {
        for (uint16_t i = 0u; i < BS_NR_OF_BAT_CELLS; i++) {
            pFakeState->data.cellVoltage->cellVoltage_mV[stringNumber][i] = FAKE_CELL_VOLTAGE_mV;
        }
    }

    DATA_WRITE_DATA(pFakeState->data.cellVoltage);

    return successfullSave;
}

static STD_RETURN_TYPE_e FAKE_SaveFakeTemperatureMeasurementData(FAKE_STATE_s *pFakeState) {
    FAS_ASSERT(pFakeState != NULL_PTR);
    STD_RETURN_TYPE_e successfullSave = STD_OK;

    for (uint8_t stringNumber = 0u; stringNumber < BS_NR_OF_STRINGS; stringNumber++) {
        for (uint16_t i = 0u; i < BS_NR_OF_TEMP_SENSORS_PER_STRING; i++) {
            pFakeState->data.cellTemperature->cellTemperature_ddegC[stringNumber][i] = FAKE_CELL_TEMPERATURE_ddegC;
        }
    }

    DATA_WRITE_DATA(pFakeState->data.cellTemperature);

    return successfullSave;
}

static FAKE_FSM_STATES_e FAKE_ProcessInitializationState(FAKE_STATE_s *pFakeState) {
    FAKE_FSM_STATES_e nextState      = FAKE_FSM_STATE_INITIALIZATION; /* default behavior: stay in state */
    static uint8_t waitForDataSaving = 0;
    switch (pFakeState->currentSubstate) {
        case FAKE_FSM_SUBSTATE_ENTRY:
            /* Nothing to do, just transfer to next substate */
            FAKE_SetSubstate(
                pFakeState, FAKE_FSM_SUBSTATE_INITIALIZATION_FINISH_FIRST_MEASUREMENT, FAKE_FSM_SHORT_TIME);
            break;

        case FAKE_FSM_SUBSTATE_INITIALIZATION_FINISH_FIRST_MEASUREMENT:
            if (true == FAKE_IsFirstMeasurementCycleFinished(pFakeState)) {
                FAKE_SetSubstate(
                    pFakeState, FAKE_FSM_SUBSTATE_INITIALIZATION_FIRST_MEASUREMENT_FINISHED, FAKE_FSM_SHORT_TIME);
            } else {
                if (waitForDataSaving == 0u) {
                    if (STD_OK == FAKE_SaveFakeVoltageMeasurementData(pFakeState)) {
                        waitForDataSaving++;
                    } else {
                        /* Voltages could not be saved, transfer to error state */
                        nextState = FAKE_FSM_STATE_ERROR;
                    }
                } else if (waitForDataSaving == 1u) {
                    if (STD_OK == FAKE_SaveFakeTemperatureMeasurementData(pFakeState)) {
                        waitForDataSaving = 0u;
                        FAKE_SetFirstMeasurementCycleFinished(pFakeState);
                    } else {
                        /* First measurement cycle could not be finished, transfer to error state */
                        nextState = FAKE_FSM_STATE_ERROR;
                    }
                } else {
                    /* must never happen */
                    nextState = FAKE_FSM_STATE_ERROR;
                }
            }
            break;

        case FAKE_FSM_SUBSTATE_INITIALIZATION_FIRST_MEASUREMENT_FINISHED:
            /* Nothing to do, just transfer to next substate */
            FAKE_SetSubstate(pFakeState, FAKE_FSM_SUBSTATE_INITIALIZATION_EXIT, FAKE_FSM_SHORT_TIME);
            break;

        case FAKE_FSM_SUBSTATE_INITIALIZATION_EXIT:
            /* Nothing to do, just transfer to next state */
            nextState = FAKE_FSM_STATE_RUNNING;
            break;

        default:
            FAS_ASSERT(FAS_TRAP);
            break;
    }
    return nextState;
}

static FAKE_FSM_STATES_e FAKE_ProcessRunningState(FAKE_STATE_s *pFakeState) {
    FAKE_FSM_STATES_e nextState = FAKE_FSM_STATE_RUNNING; /* default behavior: stay in state */

    switch (pFakeState->currentSubstate) {
        case FAKE_FSM_SUBSTATE_ENTRY:
            /* Nothing to do, just transfer to next substate */
            FAKE_SetSubstate(pFakeState, FAKE_FSM_SUBSTATE_RUNNING_SAVE_VOLTAGE_MEASUREMENT_DATA, FAKE_FSM_SHORT_TIME);
            break;

        case FAKE_FSM_SUBSTATE_RUNNING_SAVE_VOLTAGE_MEASUREMENT_DATA:
            if (STD_OK == FAKE_SaveFakeVoltageMeasurementData(pFakeState)) {
                FAKE_SetSubstate(
                    pFakeState, FAKE_FSM_SUBSTATE_RUNNING_SAVE_TEMPERATURE_MEASUREMENT_DATA, FAKE_FSM_LONG_TIME);
            } else {
                nextState = FAKE_FSM_STATE_ERROR;
            }
            break;

        case FAKE_FSM_SUBSTATE_RUNNING_SAVE_TEMPERATURE_MEASUREMENT_DATA:
            if (STD_OK == FAKE_SaveFakeVoltageMeasurementData(pFakeState)) {
                FAKE_SetSubstate(
                    pFakeState, FAKE_FSM_SUBSTATE_RUNNING_SAVE_VOLTAGE_MEASUREMENT_DATA, FAKE_FSM_LONG_TIME);
            } else {
                nextState = FAKE_FSM_STATE_ERROR;
            }
            break;

        default:
            FAS_ASSERT(FAS_TRAP);
            break;
    }

    return nextState;
}

static STD_RETURN_TYPE_e FAKE_RunStateMachine(FAKE_STATE_s *pFakeState) {
    STD_RETURN_TYPE_e ranStateMachine = STD_OK;
    FAKE_FSM_STATES_e nextState       = FAKE_FSM_STATE_DUMMY;
    switch (pFakeState->currentState) {
        /********************************************** STATE: HAS NEVER RUN */
        case FAKE_FSM_STATE_HAS_NEVER_RUN:
            /* Nothing to do, just transfer */
            FAKE_SetState(pFakeState, FAKE_FSM_STATE_UNINITIALIZED, FAKE_FSM_SUBSTATE_ENTRY, FAKE_FSM_SHORT_TIME);
            break;

        /********************************************** STATE: UNINITIALIZED */
        case FAKE_FSM_STATE_UNINITIALIZED:
            /* Nothing to do, just transfer */
            FAKE_SetState(pFakeState, FAKE_FSM_STATE_INITIALIZATION, FAKE_FSM_SUBSTATE_ENTRY, FAKE_FSM_SHORT_TIME);
            break;

        /********************************************* STATE: INITIALIZATION */
        case FAKE_FSM_STATE_INITIALIZATION:
            nextState = FAKE_ProcessInitializationState(pFakeState);
            if (nextState == FAKE_FSM_STATE_INITIALIZATION) {
                /* staying in state, processed by state function */
            } else if (nextState == FAKE_FSM_STATE_ERROR) {
                FAKE_SetState(pFakeState, FAKE_FSM_STATE_ERROR, FAKE_FSM_SUBSTATE_ENTRY, FAKE_FSM_SHORT_TIME);
            } else if (nextState == FAKE_FSM_STATE_RUNNING) {
                FAKE_SetState(pFakeState, FAKE_FSM_STATE_RUNNING, FAKE_FSM_SUBSTATE_ENTRY, FAKE_FSM_SHORT_TIME);
            } else {
                FAS_ASSERT(FAS_TRAP); /* Something went wrong */
            }
            break;

        /**************************************************** STATE: RUNNING */
        case FAKE_FSM_STATE_RUNNING:
            nextState = FAKE_ProcessRunningState(pFakeState);
            if (nextState == FAKE_FSM_STATE_RUNNING) {
                /* staying in state, processed by state function */
            } else if (nextState == FAKE_FSM_STATE_ERROR) {
                FAKE_SetState(pFakeState, FAKE_FSM_STATE_ERROR, FAKE_FSM_SUBSTATE_ENTRY, FAKE_FSM_SHORT_TIME);
            } else {
                FAS_ASSERT(FAS_TRAP); /* Something went wrong */
            }
            break;

        /****************************************************** STATE: ERROR */
        case FAKE_FSM_STATE_ERROR:
            /* this case must never happen for the dummy AFE, as all cases are processed */
            FAS_ASSERT(FAS_TRAP);
            break;

        /**************************************************** STATE: DEFAULT */
        default:
            /* all cases must be processed, trap if unknown state arrives */
            FAS_ASSERT(FAS_TRAP);
            break;
    }

    return ranStateMachine;
}

/*========== Extern Function Implementations ================================*/
extern STD_RETURN_TYPE_e FAKE_Initialize(void) {
    return STD_OK;
}

extern bool FAKE_IsFirstMeasurementCycleFinished(FAKE_STATE_s *pFakeState) {
    FAS_ASSERT(pFakeState != NULL_PTR);
    bool returnValue = false;
    OS_EnterTaskCritical();
    returnValue = pFakeState->firstMeasurementFinished;
    OS_ExitTaskCritical();
    return returnValue;
}

extern STD_RETURN_TYPE_e FAKE_TriggerAfe(FAKE_STATE_s *pFakeState) {
    FAS_ASSERT(pFakeState != NULL_PTR);
    bool earlyExit                = false;
    STD_RETURN_TYPE_e returnValue = STD_OK;
    FAS_ASSERT(pFakeState != NULL_PTR);

    /* Check re-entrance of function */
    if (FAKE_MULTIPLE_CALLS_YES == FAKE_CheckMultipleCalls(pFakeState)) {
        returnValue = STD_NOT_OK;
        earlyExit   = true;
    }

    if (earlyExit == false) {
        if (pFakeState->timer > 0u) {
            if ((--pFakeState->timer) > 0u) {
                pFakeState->triggerEntry--;
                returnValue = STD_OK;
                earlyExit   = true;
            }
        }
    }

    if (earlyExit == false) {
        FAKE_RunStateMachine(pFakeState);
        pFakeState->triggerEntry--;
    }
    return returnValue;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
extern bool TEST_FAKE_CheckMultipleCalls(FAKE_STATE_s *pFakeState) {
    return FAKE_CheckMultipleCalls(pFakeState);
}

extern void TEST_FAKE_SetFirstMeasurementCycleFinished(FAKE_STATE_s *pFakeState) {
    FAKE_SetFirstMeasurementCycleFinished(pFakeState);
}

extern void TEST_FAKE_SetState(
    FAKE_STATE_s *pFakeState,
    FAKE_FSM_STATES_e nextState,
    FAKE_FSM_SUBSTATES_e nextSubstate,
    uint16_t idleTime) {
    FAKE_SetState(pFakeState, nextState, nextSubstate, idleTime);
}

extern STD_RETURN_TYPE_e TEST_FAKE_SaveFakeVoltageMeasurementData(FAKE_STATE_s *pFakeState) {
    return FAKE_SaveFakeVoltageMeasurementData(pFakeState);
}

extern STD_RETURN_TYPE_e TEST_FAKE_SaveFakeTemperatureMeasurementData(FAKE_STATE_s *pFakeState) {
    return FAKE_SaveFakeTemperatureMeasurementData(pFakeState);
}

#endif
