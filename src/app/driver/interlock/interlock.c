/**
 *
 * @copyright &copy; 2010 - 2021, Fraunhofer-Gesellschaft zur Foerderung der
 *  angewandten Forschung e.V. All rights reserved.
 *
 * BSD 3-Clause License
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1.  Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of the copyright holder nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * We kindly request you to use one or more of the following phrases to refer
 * to foxBMS in your hardware, software, documentation or advertising
 * materials:
 *
 * &Prime;This product uses parts of foxBMS&reg;&Prime;
 *
 * &Prime;This product includes parts of foxBMS&reg;&Prime;
 *
 * &Prime;This product is derived from foxBMS&reg;&Prime;
 *
 */

/**
 * @file    interlock.c
 * @author  foxBMS Team
 * @date    2020-02-24 (date of creation)
 * @updated 2020-02-24 (date of last update)
 * @ingroup DRIVERS
 * @prefix  ILCK
 *
 * @brief   Driver for the interlock.
 *
 */

/*========== Includes =======================================================*/
#include "interlock.h"

#include "HL_gio.h"

#include "database.h"
#include "diag.h"
#include "os.h"

/*========== Macros and Definitions =========================================*/
/**
 * Saves the last state and the last substate
 */
#define ILCK_SAVELASTSTATES()                   \
    ilck_state.laststate    = ilck_state.state; \
    ilck_state.lastsubstate = ilck_state.substate

/** provides more clear name for #ILCK_SwitchInterlockOn() */
#define ILCK_CLOSEINTERLOCK() ILCK_SwitchInterlockOn()
/** provides more clear name for #ILCK_SwitchInterlockOff() */
#define ILCK_OPENINTERLOCK() ILCK_SwitchInterlockOff()

/*========== Static Constant and Variable Definitions =======================*/
/**
 * contains the state of the contactor state machine
 */
static ILCK_STATE_s ilck_state = {
    .timer             = 0,
    .statereq          = ILCK_STATE_NO_REQUEST,
    .state             = ILCK_STATEMACH_UNINITIALIZED,
    .substate          = ILCK_ENTRY,
    .laststate         = ILCK_STATEMACH_UNINITIALIZED,
    .lastsubstate      = ILCK_ENTRY,
    .triggerentry      = 0,
    .ErrRequestCounter = 0,
    .counter           = 0,
};

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/
static ILCK_RETURN_TYPE_e ILCK_CheckStateRequest(ILCK_STATE_REQUEST_e statereq);
/* static ILCK_STATE_REQUEST_e ILCK_GetStateRequest(void); */ /* Kept for compatibility */
static ILCK_STATE_REQUEST_e ILCK_TransferStateRequest(void);
static uint8_t ILCK_CheckReEntrance(void);
static void ILCK_CheckFeedback(void);
static ILCK_ELECTRICAL_STATE_TYPE_e ILCK_GetInterlockSetValue(void);
static STD_RETURN_TYPE_e ILCK_SetInterlockState(ILCK_ELECTRICAL_STATE_TYPE_e requestedInterlockState);
static STD_RETURN_TYPE_e ILCK_SwitchInterlockOff(void);
static STD_RETURN_TYPE_e ILCK_SwitchInterlockOn(void);
static void ILCK_Init_Pins(void);

/*========== Static Function Implementations ================================*/
/**
 * @brief   checks the state requests that are made.
 *
 * This function checks the validity of the state requests.
 * The results of the checked is returned immediately.
 *
 * @param   statereq    state request to be checked
 *
 * @return              result of the state request that was made, taken from ILCK_RETURN_TYPE_e
 */
static ILCK_RETURN_TYPE_e ILCK_CheckStateRequest(ILCK_STATE_REQUEST_e statereq) {
    if (statereq == ILCK_STATE_ERROR_REQUEST) {
        return ILCK_OK;
    }

    if (ilck_state.statereq == ILCK_STATE_NO_REQUEST) {
        /* init only allowed from the uninitialized state */
        if (statereq == ILCK_STATE_INIT_REQUEST) {
            if (ilck_state.state == ILCK_STATEMACH_UNINITIALIZED) {
                return ILCK_OK;
            } else {
                return ILCK_ALREADY_INITIALIZED;
            }
        }

        if ((statereq == ILCK_STATE_OPEN_REQUEST) || (statereq == ILCK_STATE_CLOSE_REQUEST)) {
            return ILCK_OK;
        } else {
            return ILCK_ILLEGAL_REQUEST;
        }
    } else {
        return ILCK_REQUEST_PENDING;
    }
}

/* TODO: check if code still necessary */
/**
 * @brief   gets the current state request.
 * @details This function is used in the functioning of the ILCK state machine.
 * It is kept as comment for compatibility.
 * @return  retval  current state request, taken from #ILCK_STATE_REQUEST_e
 */
/* static ILCK_STATE_REQUEST_e ILCK_GetStateRequest(void) {
    ILCK_STATE_REQUEST_e retval = ILCK_STATE_NO_REQUEST;

    OS_EnterTaskCritical();
    retval    = ilck_state.statereq;
    OS_ExitTaskCritical();

    return retval;
} */

/**
 * @brief   transfers the current state request to the state machine.
 *
 * This function takes the current state request from ilck_state and transfers it to the state machine.
 * It resets the value from ilck_state to ILCK_STATE_NO_REQUEST
 *
 * @return  retVal          current state request, taken from ILCK_STATE_REQUEST_e
 */
static ILCK_STATE_REQUEST_e ILCK_TransferStateRequest(void) {
    ILCK_STATE_REQUEST_e retval = ILCK_STATE_NO_REQUEST;

    OS_EnterTaskCritical();
    retval              = ilck_state.statereq;
    ilck_state.statereq = ILCK_STATE_NO_REQUEST;
    OS_ExitTaskCritical();

    return retval;
}

/**
 * @brief   re-entrance check of ILCK state machine trigger function
 *
 * This function is not re-entrant and should only be called time- or event-triggered.
 * It increments the triggerentry counter from the state variable ilck_state.
 * It should never be called by two different processes, so if it is the case, triggerentry
 * should never be higher than 0 when this function is called.
 *
 * @return  retval  0 if no further instance of the function is active, 0xff else
 */
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

/**
 * @brief   checks interlock feedback
 * @details This function is used to check interlock feedback.
 */
static void ILCK_CheckFeedback(void) {
    STD_RETURN_TYPE_e result                         = STD_NOT_OK;
    DATA_BLOCK_INTERLOCK_FEEDBACK_s ilckfeedback_tab = {.header.uniqueId = DATA_BLOCK_ID_INTERLOCK_FEEDBACK};

    ILCK_ELECTRICAL_STATE_TYPE_e interlockFeedback = ILCK_GetInterlockFeedback();
    ilckfeedback_tab.interlockFeedback             = interlockFeedback;

    DATA_WRITE_DATA(&ilckfeedback_tab);

    if (interlockFeedback == ILCK_GetInterlockSetValue()) {
        result = STD_OK;
    }
    DIAG_CheckEvent(result, DIAG_ID_INTERLOCK_FEEDBACK, DIAG_SYSTEM, 0u);
}

/**
 * @brief   Gets the latest value (#ILCK_SWITCH_ON, #ILCK_SWITCH_OFF) the interlock was set to.
 *
 * Meaning of the return value:
 *   - #ILCK_SWITCH_OFF means interlock was set to be opened
 *   - #ILCK_SWITCH_ON means interlock was set to be closed
 *
 * @return  setInformation (type: #ILCK_ELECTRICAL_STATE_TYPE_e)
 */
static ILCK_ELECTRICAL_STATE_TYPE_e ILCK_GetInterlockSetValue() {
    ILCK_ELECTRICAL_STATE_TYPE_e interlockSetInformation = ILCK_SWITCH_UNDEF;
    OS_EnterTaskCritical();
    interlockSetInformation = ilck_interlock_state.set;
    OS_ExitTaskCritical();
    return interlockSetInformation;
}

/**
 * @brief   Sets the interlock state to its requested state, if the interlock is at that time not in the requested state.
 * It returns #STD_OK if the requested state was successfully set or if the interlock was at the requested state before.
 * @param   requestedInterlockState     open or close interlock
 * @return  retVal                      #STD_OK if no error, #STD_NOT_OK otherwise
 */
static STD_RETURN_TYPE_e ILCK_SetInterlockState(ILCK_ELECTRICAL_STATE_TYPE_e requestedInterlockState) {
    STD_RETURN_TYPE_e retVal = STD_OK;

    if (requestedInterlockState == ILCK_SWITCH_ON) {
        ilck_interlock_state.set = ILCK_SWITCH_ON;
        /* gioSetBit(ILCK_IO_REG, ilck_interlock_config.control_pin, 1); */ /* TODO: activate if pin wired correctly on HW */
    } else if (requestedInterlockState == ILCK_SWITCH_OFF) {
        ilck_interlock_state.set = ILCK_SWITCH_OFF;
        /* gioSetBit(ILCK_IO_REG, ilck_interlock_config.control_pin, 0); */ /* TODO: activate if pin wired correctly on HW */
    } else {
        retVal = STD_NOT_OK;
    }

    return retVal;
}

/**
 * @brief   Switches the interlock off and returns #STD_NOT_OK on success.
 * @return  retVal (type: #STD_RETURN_TYPE_e)
 */
static STD_RETURN_TYPE_e ILCK_SwitchInterlockOff(void) {
    STD_RETURN_TYPE_e retVal = STD_NOT_OK;
    retVal                   = ILCK_SetInterlockState(ILCK_SWITCH_OFF);
    return retVal;
}

/**
 * @brief   Switches the interlock on and returns STD_OK on success.
 * @return  retVal (type: #STD_RETURN_TYPE_e)
 */
static STD_RETURN_TYPE_e ILCK_SwitchInterlockOn(void) {
    STD_RETURN_TYPE_e retVal = STD_NOT_OK;
    retVal                   = ILCK_SetInterlockState(ILCK_SWITCH_ON);
    return retVal;
}

/** initializes the pins of the interlock to default state */
static void ILCK_Init_Pins(void) {
    gioSetBit(ILCK_IO_REG, ILCK_INTERLOCK_CONTROL, 1u);
    gioSetBit(ILCK_IO_REG, ILCK_INTERLOCK_FEEDBACK, 0u);
}

/*========== Extern Function Implementations ================================*/
ILCK_ELECTRICAL_STATE_TYPE_e ILCK_GetInterlockFeedback(void) {
    ILCK_ELECTRICAL_STATE_TYPE_e measuredInterlockState = ILCK_SWITCH_UNDEF;
    uint8_t pinstate                                    = 0U;
    OS_EnterTaskCritical();
    pinstate = gioGetBit(ILCK_IO_REG, ilck_interlock_config.feedback_pin);
    OS_ExitTaskCritical();
    if (pinstate == 1U) {
        measuredInterlockState = ILCK_SWITCH_ON;
    } else if (pinstate == 0U) {
        measuredInterlockState = ILCK_SWITCH_OFF;
    }
    ilck_interlock_state.feedback = measuredInterlockState;
    return measuredInterlockState;
}

ILCK_STATEMACH_e ILCK_GetState(void) {
    return ilck_state.state;
}

ILCK_RETURN_TYPE_e ILCK_SetStateRequest(ILCK_STATE_REQUEST_e statereq) {
    ILCK_RETURN_TYPE_e retVal = ILCK_OK;

    OS_EnterTaskCritical();
    retVal = ILCK_CheckStateRequest(statereq);

    if (retVal == ILCK_OK) {
        ilck_state.statereq = statereq;
    }
    OS_ExitTaskCritical();

    return retVal;
}

void ILCK_Trigger(void) {
    ILCK_STATE_REQUEST_e statereq = ILCK_STATE_NO_REQUEST;

    /* Check re-entrance of function */
    if (ILCK_CheckReEntrance() > 0u) {
        return;
    }

    /****Happens every time the state machine is triggered**************/
    if (ilck_state.state != ILCK_STATEMACH_UNINITIALIZED) {
        ILCK_CheckFeedback();
    }

    if (ilck_state.timer > 0u) {
        if ((--ilck_state.timer) > 0u) {
            ilck_state.triggerentry--;
            return; /* handle state machine only if timer has elapsed */
        }
    }

    switch (ilck_state.state) {
        /****************************UNINITIALIZED***********************************/
        case ILCK_STATEMACH_UNINITIALIZED:
            /* waiting for Initialization Request */
            statereq = ILCK_TransferStateRequest();
            if (statereq == ILCK_STATE_INIT_REQUEST) {
                ILCK_SAVELASTSTATES();
                ILCK_Init_Pins();
                ilck_state.timer    = ILCK_STATEMACH_SHORTTIME_MS;
                ilck_state.state    = ILCK_STATEMACH_INITIALIZATION;
                ilck_state.substate = ILCK_ENTRY;
            } else if (statereq == ILCK_STATE_NO_REQUEST) {
                /* no actual request pending   */
            } else {
                ilck_state.ErrRequestCounter++; /* illegal request pending */
            }
            break;

        /****************************INITIALIZATION**********************************/
        case ILCK_STATEMACH_INITIALIZATION:
            ILCK_SAVELASTSTATES();
            ILCK_OPENINTERLOCK();

            ilck_state.timer    = ILCK_STATEMACH_SHORTTIME_MS;
            ilck_state.state    = ILCK_STATEMACH_INITIALIZED;
            ilck_state.substate = ILCK_ENTRY;
            break;

        /****************************INITIALIZED*************************************/
        case ILCK_STATEMACH_INITIALIZED:
            ILCK_SAVELASTSTATES();
            ilck_state.timer    = ILCK_STATEMACH_SHORTTIME_MS;
            ilck_state.state    = ILCK_STATEMACH_WAIT_FIRST_REQUEST;
            ilck_state.substate = ILCK_ENTRY;
            break;

        /****************************INITIALIZED*************************************/
        case ILCK_STATEMACH_WAIT_FIRST_REQUEST:
            ILCK_SAVELASTSTATES();
            statereq = ILCK_TransferStateRequest();
            if (statereq == ILCK_STATE_OPEN_REQUEST) {
                ilck_state.timer    = ILCK_STATEMACH_SHORTTIME_MS;
                ilck_state.state    = ILCK_STATEMACH_OPEN;
                ilck_state.substate = ILCK_ENTRY;
            } else if (statereq == ILCK_STATE_CLOSE_REQUEST) {
                ilck_state.timer    = ILCK_STATEMACH_SHORTTIME_MS;
                ilck_state.state    = ILCK_STATEMACH_CLOSED;
                ilck_state.substate = ILCK_ENTRY;
            } else {
                ilck_state.timer = ILCK_STATEMACH_SHORTTIME_MS;
            }
            break;

        /****************************OPEN*************************************/
        case ILCK_STATEMACH_OPEN:
            ILCK_SAVELASTSTATES();
            ILCK_OPENINTERLOCK();
            ilck_state.timer = ILCK_STATEMACH_SHORTTIME_MS;
            statereq         = ILCK_TransferStateRequest();
            if (statereq == ILCK_STATE_CLOSE_REQUEST) {
                ilck_state.timer    = ILCK_STATEMACH_SHORTTIME_MS;
                ilck_state.state    = ILCK_STATEMACH_CLOSED;
                ilck_state.substate = ILCK_ENTRY;
                break;
            }
            break;

        /****************************CLOSED*************************************/
        case ILCK_STATEMACH_CLOSED:
            ILCK_SAVELASTSTATES();
            ILCK_CLOSEINTERLOCK();
            ilck_state.timer = ILCK_STATEMACH_SHORTTIME_MS;
            statereq         = ILCK_TransferStateRequest();
            if (statereq == ILCK_STATE_OPEN_REQUEST) {
                ilck_state.timer    = ILCK_STATEMACH_SHORTTIME_MS;
                ilck_state.state    = ILCK_STATEMACH_OPEN;
                ilck_state.substate = ILCK_ENTRY;
                break;
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
    return;
}
#endif

/*========== Externalized Static Function Implementations (Unit Test) =======*/
