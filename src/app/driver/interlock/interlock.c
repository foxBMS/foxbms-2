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
 * @file    interlock.c
 * @author  foxBMS Team
 * @date    2020-02-24 (date of creation)
 * @updated 2021-10-18 (date of last update)
 * @ingroup DRIVERS
 * @prefix  ILCK
 *
 * @brief   Driver for the interlock.
 * @details The interlock driver measures the relevant hardware signals from
 *          the interlock circuit. For details on available hardware signals
 *          please refer to the section on this module in the foxBMS
 *          documentation.
 *
 *          In reference to the names in the foxBMS schematic, this module uses
 *          the following names:
 *
 *          shorthand | meaning
 *          --------- | ---------
 *          IL        | interlock
 *          HS        | high-side
 *          LS        | low-side
 *          VS        | voltage sense
 *          CS        | current sense
 *
 */

/*========== Includes =======================================================*/
#include "interlock.h"

#include "database.h"
#include "diag.h"
#include "io.h"
#include "os.h"

/*========== Macros and Definitions =========================================*/
/**
 * Saves the last state and the last substate
 */
#define ILCK_SAVELASTSTATES()                   \
    ilck_state.laststate    = ilck_state.state; \
    ilck_state.lastsubstate = ilck_state.substate

/*========== Static Constant and Variable Definitions =======================*/
/**
 * contains the state of the contactor state machine
 */
static ILCK_STATE_s ilck_state = {
    .timer             = 0,
    .statereq          = ILCK_STATE_NO_REQUEST,
    .state             = ILCK_STATEMACHINE_UNINITIALIZED,
    .substate          = ILCK_ENTRY,
    .laststate         = ILCK_STATEMACHINE_UNINITIALIZED,
    .lastsubstate      = ILCK_ENTRY,
    .triggerentry      = 0,
    .ErrRequestCounter = 0,
    .counter           = 0,
};

/** Local variable containing interlock feedback */
static DATA_BLOCK_INTERLOCK_FEEDBACK_s ilck_tableFeedback = {.header.uniqueId = DATA_BLOCK_ID_INTERLOCK_FEEDBACK};

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/
/**
 * @brief   checks the state requests that are made.
 * @details This function checks the validity of the state requests. The
 *          results of the checked is returned immediately.
 * @param   statereq    state request to be checked
 * @return              result of the state request that was made, taken from
 *                      #ILCK_RETURN_TYPE_e
 */
static ILCK_RETURN_TYPE_e ILCK_CheckStateRequest(ILCK_STATE_REQUEST_e statereq);

/**
 * @brief   transfers the current state request to the state machine.
 * @details This function takes the current state request from ilck_state and
 *          transfers it to the state machine. It resets the value from
 *          ilck_state to ILCK_STATE_NO_REQUEST
 * @return  current state request, taken from ILCK_STATE_REQUEST_e
 */
static ILCK_STATE_REQUEST_e ILCK_TransferStateRequest(void);

/**
 * @brief   re-entrance check of ILCK state machine trigger function
 * @details This function is not re-entrant and should only be called time- or
 *          event-triggered. It increments the triggerentry counter from the
 *          state variable ilck_state. It should never be called by two
 *          different processes, so if it is the case, triggerentry should
 *          never be higher than 0 when this function is called.
 * @return  0 if no further instance of the function is active, 0xFF else
 */
static uint8_t ILCK_CheckReEntrance(void);

/**
 * @brief   Initializes required pins for interlock evaluation
 */
static void ILCK_InitializePins(void);

/**
 * @brief   Reads the feedback pin of the interlock and returns its current value
 *          (ILCK_SWITCH_OFF/ILCK_SWITCH_ON)
 * @return  measuredInterlockState (type: ILCK_ELECTRICAL_STATE_TYPE_e)
 */
static ILCK_ELECTRICAL_STATE_TYPE_e ILCK_GetInterlockFeedback(void);

/*========== Static Function Implementations ================================*/
static ILCK_RETURN_TYPE_e ILCK_CheckStateRequest(ILCK_STATE_REQUEST_e statereq) {
    ILCK_RETURN_TYPE_e stateRequestCheck = ILCK_ILLEGAL_REQUEST;
    if (!((statereq == ILCK_STATE_INITIALIZATION_REQUEST) || (statereq == ILCK_STATE_NO_REQUEST))) {
        stateRequestCheck = ILCK_ILLEGAL_REQUEST;
    } else if (ilck_state.statereq == ILCK_STATE_NO_REQUEST) {
        /* init only allowed from the uninitialized state */
        if (statereq == ILCK_STATE_INITIALIZATION_REQUEST) {
            if (ilck_state.state == ILCK_STATEMACHINE_UNINITIALIZED) {
                stateRequestCheck = ILCK_OK;
            } else {
                stateRequestCheck = ILCK_ALREADY_INITIALIZED;
            }
        }
    } else {
        stateRequestCheck = ILCK_REQUEST_PENDING;
    }
    return stateRequestCheck;
}

static ILCK_STATE_REQUEST_e ILCK_TransferStateRequest(void) {
    ILCK_STATE_REQUEST_e retval = ILCK_STATE_NO_REQUEST;

    OS_EnterTaskCritical();
    retval              = ilck_state.statereq;
    ilck_state.statereq = ILCK_STATE_NO_REQUEST;
    OS_ExitTaskCritical();

    return retval;
}

static uint8_t ILCK_CheckReEntrance(void) {
    uint8_t retval = 0;

    OS_EnterTaskCritical();
    if (!ilck_state.triggerentry) {
        ilck_state.triggerentry++;
    } else {
        retval = 0xFF; /* multiple calls of function */
    }
    OS_ExitTaskCritical();

    return retval;
}

static void ILCK_InitializePins(void) {
    /* Configure diagnostic supply enable pin as output */
    IO_SetPinDirectionToOutput(&ILCK_IO_REG_DIR, ILCK_INTERLOCK_CONTROL_PIN_IL_HS_ENABLE);
    /* Disable diagnostic power supply as component is currently NOT available */
    IO_PinReset(&ILCK_IO_REG_PORT->DOUT, ILCK_INTERLOCK_CONTROL_PIN_IL_HS_ENABLE);
    /* Configure interlock feedback pin as input */
    IO_SetPinDirectionToInput(&ILCK_IO_REG_DIR, ILCK_INTERLOCK_FEEDBACK_PIN_IL_STATE);
}

static ILCK_ELECTRICAL_STATE_TYPE_e ILCK_GetInterlockFeedback(void) {
    ILCK_ELECTRICAL_STATE_TYPE_e measuredInterlockState = ILCK_SWITCH_UNDEF;

    OS_EnterTaskCritical();
    const STD_PIN_STATE_e pinState = IO_PinGet(&ILCK_IO_REG_PORT->DIN, ILCK_INTERLOCK_FEEDBACK_PIN_IL_STATE);
    OS_ExitTaskCritical();

    /** Local variable containing voltages measured on TMS570 ADC1 inputs */
    DATA_BLOCK_ADC_VOLTAGE_s ilck_tableAdcVoltages = {.header.uniqueId = DATA_BLOCK_ID_ADC_VOLTAGE};
    DATA_READ_DATA(&ilck_tableAdcVoltages);

    /** Pin low: interlock closed, pin high: interlock open */
    if (pinState == STD_PIN_HIGH) {
        measuredInterlockState = ILCK_SWITCH_OFF;
    } else if (pinState == STD_PIN_LOW) {
        measuredInterlockState = ILCK_SWITCH_ON;
    }

    ilck_tableFeedback.interlockVoltageFeedback_IL_HS_VS_mV =
        ilck_tableAdcVoltages.adc1ConvertedVoltages_mV[ILCK_ADC_INPUT_HIGH_SIDE_VOLTAGE_SENSE] *
        ILCK_VOLTAGE_DIVIDER_FACTOR;
    ilck_tableFeedback.interlockVoltageFeedback_IL_LS_VS_mV =
        ilck_tableAdcVoltages.adc1ConvertedVoltages_mV[ILCK_ADC_INPUT_LOW_SIDE_VOLTAGE_SENSE] *
        ILCK_VOLTAGE_DIVIDER_FACTOR;
    ilck_tableFeedback.interlockCurrentFeedback_IL_HS_CS_mA =
        ilck_tableAdcVoltages.adc1ConvertedVoltages_mV[ILCK_ADC_INPUT_HIGH_SIDE_CURRENT_SENSE] *
        ILCK_FACTOR_IL_HS_CS_1_ohm;
    ilck_tableFeedback.interlockCurrentFeedback_IL_LS_CS_mA =
        ilck_tableAdcVoltages.adc1ConvertedVoltages_mV[ILCK_ADC_INPUT_LOW_SIDE_CURRENT_SENSE] *
        ILCK_FACTOR_IL_LS_CS_1_ohm;

    ilck_tableFeedback.interlockFeedback_IL_STATE = measuredInterlockState;

    DATA_WRITE_DATA(&ilck_tableFeedback);

#if (BS_IGNORE_INTERLOCK_FEEDBACK == true)
    measuredInterlockState = ILCK_SWITCH_ON;
#endif

    return measuredInterlockState;
}

/*========== Extern Function Implementations ================================*/
ILCK_STATEMACH_e ILCK_GetState(void) {
    return ilck_state.state;
}

ILCK_RETURN_TYPE_e ILCK_SetStateRequest(ILCK_STATE_REQUEST_e statereq) {
    ILCK_RETURN_TYPE_e retVal = ILCK_ILLEGAL_REQUEST;

    OS_EnterTaskCritical();
    retVal = ILCK_CheckStateRequest(statereq);

    if (retVal != ILCK_ILLEGAL_REQUEST) {
        ilck_state.statereq = statereq;
    }
    OS_ExitTaskCritical();

    return retVal;
}

void ILCK_Trigger(void) {
    ILCK_STATE_REQUEST_e statereq               = ILCK_STATE_NO_REQUEST;
    ILCK_ELECTRICAL_STATE_TYPE_e interlockState = ILCK_SWITCH_UNDEF;

    /* Check re-entrance of function */
    if (ILCK_CheckReEntrance() > 0u) {
        return;
    }

    if (ilck_state.timer > 0u) {
        if ((--ilck_state.timer) > 0u) {
            ilck_state.triggerentry--;
            return; /* handle state machine only if timer has elapsed */
        }
    }

    switch (ilck_state.state) {
        /****************************UNINITIALIZED***********************************/
        case ILCK_STATEMACHINE_UNINITIALIZED:
            /* waiting for Initialization Request */
            statereq = ILCK_TransferStateRequest();
            if (statereq == ILCK_STATE_INITIALIZATION_REQUEST) {
                ILCK_SAVELASTSTATES();
                ILCK_InitializePins();
                ilck_state.timer    = ILCK_STATEMACH_SHORTTIME;
                ilck_state.state    = ILCK_STATEMACHINE_INITIALIZED;
                ilck_state.substate = ILCK_ENTRY;
            } else if (statereq == ILCK_STATE_NO_REQUEST) {
                /* no actual request pending   */
            } else {
                ilck_state.ErrRequestCounter++; /* illegal request pending */
            }
            break;

        /****************************INITIALIZED*************************************/
        case ILCK_STATEMACHINE_INITIALIZED:
            ILCK_SAVELASTSTATES();
            ilck_state.timer = ILCK_STATEMACH_SHORTTIME;
            interlockState   = ILCK_GetInterlockFeedback();
            if (interlockState == ILCK_SWITCH_ON) {
                (void)DIAG_Handler(DIAG_ID_INTERLOCK_FEEDBACK, DIAG_EVENT_OK, DIAG_SYSTEM, 0u);
            } else {
                (void)DIAG_Handler(DIAG_ID_INTERLOCK_FEEDBACK, DIAG_EVENT_NOT_OK, DIAG_SYSTEM, 0u);
            }
            break;

        default:
            /* this is an undefined state that should never be reached */
            FAS_ASSERT(FAS_TRAP);
            break;
    } /* end switch (ilck_state.state) */

    ilck_state.triggerentry--;
}

/*================== Setter for static Variables (Unit Test) ==============*/
#ifdef UNITY_UNIT_TEST
extern void TEST_ILCK_SetStateStruct(ILCK_STATE_s state) {
    ilck_state = state;
}
extern ILCK_ELECTRICAL_STATE_TYPE_e TEST_ILCK_GetInterlockFeedback(void) {
    return ILCK_GetInterlockFeedback();
}
#endif

/*========== Externalized Static Function Implementations (Unit Test) =======*/
