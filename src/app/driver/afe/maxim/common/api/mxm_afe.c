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
 * @file    mxm_afe.c
 * @author  foxBMS Team
 * @date    2020-06-16 (date of creation)
 * @updated 2021-07-14 (date of last update)
 * @ingroup DRIVER
 * @prefix  AFE
 *
 * @brief   AFE driver implementation
 */

/*========== Includes =======================================================*/
#include "general.h"

#include "afe.h"
#include "mxm_17841b.h"
#include "mxm_1785x.h"
#include "mxm_battery_management.h"
#include "os.h"

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/** state variable for the MAX17841B driver */
static MXM_41B_INSTANCE_s mxm_41b_state = {
    .state              = MXM_STATEMACH_41B_UNINITIALIZED,
    .substate           = MXM_41B_ENTRY_SUBSTATE,
    .aliveCounter       = 0,
    .extendMessageBytes = 0,
    .waitCounter        = 0,
    .regRXIntEnable     = 0x00u,
    .regTXIntEnable     = 0x00u,
    .regRXStatus        = 0x00u,
    .regTXStatus        = 0x00u,
    .regConfig1         = 0x60u,
    .regConfig2         = 0x10u,
    .regConfig3         = 0x0Fu,
    .spiRXBuffer        = {0},
    .spiTXBuffer        = {0},
};

/** state variable for the Battery Management protocol driver */
static MXM_5X_INSTANCE_s mxm_5x_state = {
    .state    = MXM_STATEMACH_5X_UNINITIALIZED,
    .substate = MXM_5X_ENTRY_SUBSTATE,
    .commandPayload =
        {
            .regAddress = (MXM_REG_NAME_e)0x00u,
            .lsb        = 0x00u,
            .msb        = 0x00u,
        },
    .status41b                = MXM_41B_STATE_UNSENT,
    .numberOfSatellites       = 0,
    .numberOfSatellitesIsGood = STD_NOT_OK,
    .lastDCByte               = 0,
};

/** state variable for the Maxim monitoring driver */
static MXM_MONITORING_INSTANCE_s mxm_state = {
    .state                 = MXM_STATEMACHINE_STATES_UNINITIALIZED,
    .operationSubstate     = MXM_INIT_DEVCFG1,
    .allowStartup          = false,
    .operationRequested    = false,
    .firstMeasurementDone  = false,
    .stopRequested         = false,
    .openwireRequested     = false,
    .undervoltageAlert     = false,
    .muxCounter            = 0u,
    .dcByte                = MXM_DC_EMPTY,
    .mxmVoltageCellCounter = 0,
    .highest5xDevice       = 0,
    .requestStatus5x       = MXM_5X_STATE_UNSENT,
    .batteryCmdBuffer =
        {
            .regAddress    = (MXM_REG_NAME_e)0x00,
            .lsb           = 0x00,
            .msb           = 0x00,
            .deviceAddress = 0x00,
            .blocksize     = 0,
        },
    .resultSelfCheck = STD_NOT_OK,
    .selfCheck =
        {
            .crc                      = STD_NOT_OK,
            .conv                     = STD_NOT_OK,
            .firstSetBit              = STD_NOT_OK,
            .extractValueFromRegister = STD_NOT_OK,
            .parseVoltageReadall      = STD_NOT_OK,
            .addressSpaceChecker      = STD_NOT_OK,
            .fmeaStatusASCI           = STD_NOT_OK,
        },
    .pInstance41B  = &mxm_41b_state,
    .pInstance5X   = &mxm_5x_state,
    .localVoltages = {0},
};

/** counter variable for the number of all instances of this driver */
static int32_t mxm_numberOfInstances = 0;

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
 *          receiving as this is the limiting ressource.)
 */
static void MXM_Tick(void);

/*========== Static Function Implementations ================================*/

static void MXM_Tick(void) {
    FAS_ASSERT(mxm_numberOfInstances == 0);
    mxm_numberOfInstances++;
    /* TODO let statemachines recover from hangups */
    /* TODO best execution order of statemachines? */
    MXM_StateMachine(&mxm_state);
    MXM_5XStateMachine(mxm_state.pInstance41B, mxm_state.pInstance5X);
    MXM_StateMachine(&mxm_state);
    MXM_5XStateMachine(mxm_state.pInstance41B, mxm_state.pInstance5X);
    MXM_StateMachine(&mxm_state);
    MXM_5XStateMachine(mxm_state.pInstance41B, mxm_state.pInstance5X);
    MXM_41BStateMachine(mxm_state.pInstance41B);
    MXM_5XStateMachine(mxm_state.pInstance41B, mxm_state.pInstance5X);
    MXM_StateMachine(&mxm_state);
    MXM_5XStateMachine(mxm_state.pInstance41B, mxm_state.pInstance5X);
    MXM_StateMachine(&mxm_state);
    MXM_StateMachine(&mxm_state);
    MXM_5XStateMachine(mxm_state.pInstance41B, mxm_state.pInstance5X);
    mxm_numberOfInstances--;
}

/*========== Extern Function Implementations ================================*/
extern STD_RETURN_TYPE_e AFE_TriggerIc(void) {
    MXM_Tick();
    return STD_OK;
}

extern STD_RETURN_TYPE_e AFE_Initialize(void) {
    MXM_MonitoringPinInit();
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

extern STD_RETURN_TYPE_e AFE_RequestIoWrite(uint8_t string) {
    FAS_ASSERT(string < BS_NR_OF_STRINGS);
    return STD_NOT_OK;
}

extern STD_RETURN_TYPE_e AFE_RequestIoRead(uint8_t string) {
    FAS_ASSERT(string < BS_NR_OF_STRINGS);
    return STD_NOT_OK;
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

    mxm_state.openwireRequested = true;

    return retval;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
