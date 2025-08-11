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
 * @file    mxm_afe.c
 * @author  foxBMS Team
 * @date    2020-06-16 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup DRIVERS
 * @prefix  AFE
 *
 * @brief   AFE driver implementation
 *  @details TODO
 */

/*========== Includes =======================================================*/

#include "afe.h"
#include "mxm_17841b.h"
#include "mxm_1785x.h"
#include "mxm_battery_management.h"
#include "os.h"

#include <stdbool.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/** state variable for the MAX17841B driver */
/* AXIVION Next Codeline Style Generic-InitializeAllVariables: state variable has an initializer function */
static MXM_41B_INSTANCE_s mxm_41bState;

/** state variable for the Battery Management protocol driver */
/* AXIVION Next Codeline Style Generic-InitializeAllVariables: state variable has an initializer function */
static MXM_5X_INSTANCE_s mxm_5xState;

/**
 * @brief Local cell voltage data block
 *
 * This local instance stores the measured cell voltages. In contrast to
 * #MXM_MONITORING_INSTANCE_s::localVoltages, the layout of this structure
 * changes with the defined battery-system as it is described by the
 * database_cfg.h. Mapping the values from
 * #MXM_MONITORING_INSTANCE_s::localVoltages to #mxm_tableCellVoltages and copying
 * these entries into the database is handled by #MXM_ParseVoltagesIntoDB().
 */
static DATA_BLOCK_CELL_VOLTAGE_s mxm_tableCellVoltages = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE_BASE};

/**
 * @brief   Local cell temperature data block
 * @details This local instance stores the measured cell temperatures.
 */
static DATA_BLOCK_CELL_TEMPERATURE_s mxm_tableCellTemperatures = {
    .header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE_BASE};

/**
 * @brief   Balancing control
 * @details This variable stores information about which cells need balancing
 */
static DATA_BLOCK_BALANCING_CONTROL_s mxm_tableBalancingControl = {.header.uniqueId = DATA_BLOCK_ID_BALANCING_CONTROL};

/** @brief Local data structure for openWire results. */
static DATA_BLOCK_OPEN_WIRE_s mxm_tableOpenWire = {.header.uniqueId = DATA_BLOCK_ID_OPEN_WIRE_BASE};

/** balancing state variable */
static MXM_BALANCING_STATE_s mxm_balancingState = {
    .pBalancingControl_table = &mxm_tableBalancingControl,
};

/** state variable for the Maxim monitoring driver */
static MXM_MONITORING_INSTANCE_s mxm_state = {
    .pBalancingState                    = &mxm_balancingState,
    .pInstance41B                       = &mxm_41bState,
    .pInstance5X                        = &mxm_5xState,
    .pCellVoltages_table                = &mxm_tableCellVoltages,
    .pCellTemperatures_table            = &mxm_tableCellTemperatures,
    .pOpenWire_table                    = &mxm_tableOpenWire,
    .selfCheck.crc                      = STD_NOT_OK,
    .selfCheck.conversion               = STD_NOT_OK,
    .selfCheck.firstSetBit              = STD_NOT_OK,
    .selfCheck.extractValueFromRegister = STD_NOT_OK,
    .selfCheck.parseVoltageReadall      = STD_NOT_OK,
    .selfCheck.addressSpaceChecker      = STD_NOT_OK,
};

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/**
 * @brief   Tick function
 * @details This function is the central entry-point for this driver. It calls
 *          the state-machines via the functions #MXM_StateMachine(),
 *          #MXM_5XStateMachine() and #MXM_41BStateMachine().
 *
 *          The state-machines are called in such a way that the main blocking
 *          point is the SPI interface. (That means the order is optimized so
 *          that every cycle a SPI command should be available for sending or
 *          receiving as this is the limiting resource.)
 */
static void MXM_Tick(void);

/**
 * @brief   Initialize the state structs
 * @details This function sets default values to the members of the state
 *          structs. It can be used to reset the driver to a default value.
 */
static void MXM_SetStateStructDefaultValues(void);

/*========== Static Function Implementations ================================*/

static void MXM_Tick(void) {
    MXM_CheckIfErrorCounterCanBeReset(&mxm_state);
    MXM_StateMachine(&mxm_state);
    MXM_5XStateMachine(mxm_state.pInstance41B, mxm_state.pInstance5X);
    MXM_41BStateMachine(mxm_state.pInstance41B);
    /* execute battery management state machine once (low cost and prepares for next cycle) */
    MXM_5XStateMachine(mxm_state.pInstance41B, mxm_state.pInstance5X);

    /* check if init is stuck */
    if ((mxm_state.state != MXM_STATEMACHINE_STATES_UNINITIALIZED) &&
        (mxm_state.state != MXM_STATEMACHINE_STATES_IDLE) && (mxm_state.state != MXM_STATEMACHINE_STATES_OPERATION) &&
        (mxm_state.resetNecessary == false)) {
        /* check if we spend too long in initialization and try to resolve with a reset */
        if (OS_CheckTimeHasPassed(mxm_state.timestampInit, MXM_MAXIMUM_INIT_TIME_ms) == true) {
            MXM_ErrorHandlerReset(&mxm_state, true);
        }
    }
}

static void MXM_SetStateStructDefaultValues(void) {
    MXM_41BInitializeStateStruct(&mxm_41bState);
    MXM_5X_InitializeStateStruct(&mxm_5xState);
    MXM_InitializeStateStruct(&mxm_balancingState, &mxm_state);
}

/*========== Extern Function Implementations ================================*/
extern STD_RETURN_TYPE_e AFE_TriggerIc(void) {
    if (mxm_state.resetNecessary == true) {
        /* a reset has been requested */

        OS_EnterTaskCritical();
        /* save allowStartup and operationRequested */
        const bool allowStartup         = mxm_state.allowStartup;
        const bool operationRequested   = mxm_state.operationRequested;
        const bool firstMeasurementDone = mxm_state.firstMeasurementDone;
        OS_ExitTaskCritical();

        (void)AFE_Initialize();

        /* restore relevant flags */
        mxm_state.allowStartup = allowStartup;
        /* if a first measurement has been done, operation has been requested before */
        mxm_state.operationRequested = (operationRequested || firstMeasurementDone);
    } else {
        MXM_Tick();
    }

    return STD_OK;
}

extern STD_RETURN_TYPE_e AFE_Initialize(void) {
    MXM_SetStateStructDefaultValues();
    MXM_InitializeMonitoringPins();
    /* call pre init self check so that we can do these costly tests before the main cycle begins (results are stored)
     */
    (void)MXM_PreInitSelfCheck(&mxm_state);
    return STD_OK;
}

extern STD_RETURN_TYPE_e AFE_StartMeasurement(void) {
    STD_RETURN_TYPE_e retval = STD_OK;

    OS_EnterTaskCritical();
    mxm_state.allowStartup       = true;
    mxm_state.operationRequested = true;
    OS_ExitTaskCritical();

    return retval;
}

extern bool AFE_IsFirstMeasurementCycleFinished(void) {
    OS_EnterTaskCritical();
    const bool firstMeasurementDone = mxm_state.firstMeasurementDone;
    OS_ExitTaskCritical();
    return firstMeasurementDone;
}

extern STD_RETURN_TYPE_e AFE_RequestTemperatureRead(uint8_t string) {
    FAS_ASSERT(string < BS_NR_OF_STRINGS);
    return STD_NOT_OK;
}

extern STD_RETURN_TYPE_e AFE_RequestBalancingFeedbackRead(uint8_t string) {
    FAS_ASSERT(string < BS_NR_OF_STRINGS);
    return STD_NOT_OK;
}

extern STD_RETURN_TYPE_e AFE_RequestEepromRead(uint8_t string) {
    FAS_ASSERT(string < BS_NR_OF_STRINGS);
    return STD_NOT_OK;
}

extern STD_RETURN_TYPE_e AFE_RequestEepromWrite(uint8_t string) {
    FAS_ASSERT(string < BS_NR_OF_STRINGS);
    return STD_NOT_OK;
}

extern STD_RETURN_TYPE_e AFE_RequestOpenWireCheck(uint8_t string) {
    FAS_ASSERT(string < BS_NR_OF_STRINGS);
    STD_RETURN_TYPE_e retval = STD_OK;

    mxm_state.openWireRequested = true;

    return retval;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
