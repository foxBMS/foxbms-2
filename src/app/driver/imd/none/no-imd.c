/**
 *
 * @copyright &copy; 2010 - 2023, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * @file    no-imd.c
 * @author  foxBMS Team
 * @date    2020-11-20 (date of creation)
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
 * @ingroup DRIVERS
 * @prefix  NOIMD
 *
 * @brief   Driver for the dummy insulation monitoring driver
 *
 * @details Dummy driver that provides a default resistance and returns that
 *          the measurement is always valid.
 */

/*========== Includes =======================================================*/
#include "no-imd.h"

#include "database.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/
/** Initialize function */
static IMD_FSM_STATES_e NOIMD_Initialize(void);

/** Enable IMD function */
static IMD_FSM_STATES_e NOIMD_EnableImd(void);

/**
 * @brief   Function to write dummy values into the database
 * @param   pTableInsulationMonitoring pointer to database entry
 */
static IMD_FSM_STATES_e NOIMD_MeasureInsulation(DATA_BLOCK_INSULATION_MONITORING_s *pTableInsulationMonitoring);

/** Enable IMD function */
static IMD_FSM_STATES_e NOIMD_DisableImd(void);

/*========== Static Function Implementations ================================*/
static IMD_FSM_STATES_e NOIMD_Initialize(void) {
    return IMD_FSM_STATE_IMD_ENABLE;
}

static IMD_FSM_STATES_e NOIMD_EnableImd(void) {
    return IMD_FSM_STATE_RUNNING;
}

static IMD_FSM_STATES_e NOIMD_MeasureInsulation(DATA_BLOCK_INSULATION_MONITORING_s *pTableInsulationMonitoring) {
    pTableInsulationMonitoring->isInsulationMeasurementValid   = true;
    pTableInsulationMonitoring->areDeviceFlagsValid            = true;
    pTableInsulationMonitoring->insulationResistance_kOhm      = 1000u;
    pTableInsulationMonitoring->dfIsCriticalResistanceDetected = false;
    pTableInsulationMonitoring->dfIsChassisFaultDetected       = false;
    pTableInsulationMonitoring->dfIsDeviceErrorDetected        = false;
    pTableInsulationMonitoring->dfIsWarnableResistanceDetected = false;
    return IMD_FSM_STATE_RUNNING;
}

static IMD_FSM_STATES_e NOIMD_DisableImd(void) {
    return IMD_FSM_STATE_IMD_ENABLE;
}

/*========== Extern Function Implementations ================================*/
extern IMD_FSM_STATES_e IMD_ProcessInitializationState(void) {
    return NOIMD_Initialize();
}

extern IMD_FSM_STATES_e IMD_ProcessEnableState(void) {
    return NOIMD_EnableImd();
}

extern IMD_FSM_STATES_e IMD_ProcessRunningState(DATA_BLOCK_INSULATION_MONITORING_s *pTableInsulationMonitoring) {
    FAS_ASSERT(pTableInsulationMonitoring != NULL_PTR);
    return NOIMD_MeasureInsulation(pTableInsulationMonitoring);
}

extern IMD_FSM_STATES_e IMD_ProcessShutdownState(void) {
    return NOIMD_DisableImd();
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
extern IMD_FSM_STATES_e TEST_NOIMD_Initialize(void) {
    return NOIMD_Initialize();
}
extern IMD_FSM_STATES_e TEST_NOIMD_EnableImd(void) {
    return NOIMD_EnableImd();
}
extern IMD_FSM_STATES_e TEST_NOIMD_MeasureInsulation(DATA_BLOCK_INSULATION_MONITORING_s *pTableInsulationMonitoring) {
    return NOIMD_MeasureInsulation(pTableInsulationMonitoring);
}
extern IMD_FSM_STATES_e TEST_NOIMD_DisableImd(void) {
    return NOIMD_DisableImd();
}
#endif
