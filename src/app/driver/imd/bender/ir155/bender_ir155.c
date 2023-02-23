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
 * @file    bender_ir155.c
 * @author  foxBMS Team
 * @date    2014-02-11 (date of creation)
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
 * @ingroup DRIVERS
 * @prefix  IR155
 *
 * @brief   Driver for the insulation monitoring
 */

/*========== Includes =======================================================*/
#include "bender_ir155.h"

#include "bender_ir155_cfg.h"

#include "bender_ir155_helper.h"
#include "database.h"
#include "fram.h"
#include "io.h"

#include <stdbool.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/**
 * Dutycycle - resistance table
 * for Bender IR155-3204 (Art. No. B91068139)
 *
 *  dc | resistance/kOhm  |  description           |
 *  ---|------------------|------------------------|
 *  100| -63,16           |  not valid             |
 *  98 | -38,71           |  invalid               |
 *  95 | =0,00            |  invalid               |
 *  95 | >0,00            |  shortcut              |
 *  90 | 70,59            |  very low resistance   |
 *  80 | 240,00           |  low resistance        |
 *  70 | 461,54           |  low resistance        |
 *  60 | 763,64           |  lowmid resistance     |
 *  50 | 1200,00          |  highmid resistance    |
 *  40 | 1885,71          |  highmid resistance    |
 *  30 | 3120,00          |  high resistance       |
 *  20 | 6000,00          |  high resistance       |
 *  10 | 20400,00         |  very high resistance  |
 *  5  | oo               |  very high resistance  |
 *  3  |-55200,00         |  invalid               |
 *  0  |-22800,00         |  invalid               |
 *
 */
#define IR155_MINIMUM_INSULATION_RESISTANCE_kOhm (0u)

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/
/** Initialize function for Bender IR155 driver */
static IMD_FSM_STATES_e IR155_InitializeModule(void);

/** Acquire measurement results from Bender IR155 and write them into the database */
static IMD_FSM_STATES_e IR155_MeasureInsulation(DATA_BLOCK_INSULATION_MONITORING_s *pTableInsulationMonitoring);

/*========== Static Function Implementations ================================*/
static IMD_FSM_STATES_e IR155_InitializeModule(void) {
    /* Initialize Software-Module */
    IR155_Initialize(IMD_PERIODIC_CALL_TIME_ms);
    return IMD_FSM_STATE_IMD_ENABLE;
}

static IMD_FSM_STATES_e IR155_MeasureInsulation(DATA_BLOCK_INSULATION_MONITORING_s *pTableInsulationMonitoring) {
    FAS_ASSERT(pTableInsulationMonitoring != NULL_PTR);
    pTableInsulationMonitoring->isInsulationMeasurementValid   = true;
    pTableInsulationMonitoring->insulationResistance_kOhm      = IR155_MINIMUM_INSULATION_RESISTANCE_kOhm;
    pTableInsulationMonitoring->areDeviceFlagsValid            = true;
    pTableInsulationMonitoring->dfIsCriticalResistanceDetected = false;
    pTableInsulationMonitoring->dfIsChassisFaultDetected       = false;
    pTableInsulationMonitoring->dfIsDeviceErrorDetected        = false;
    /* Bender IR155 does not support a digital warning threshold -> check measured insulation resistance */
    pTableInsulationMonitoring->dfIsWarnableResistanceDetected = false;

    /* Evaluate input pwm signal */
    ir155_state.measurement = IR155_GetMeasurementValues();

    /* Measurement is not valid, either because of startup delay or detected ground error before startup */
    if (ir155_state.timeUntilValidMeasurement_ms > 0u) {
        if (ir155_state.timeUntilValidMeasurement_ms >= ir155_state.periodTriggerTime_ms) {
            ir155_state.timeUntilValidMeasurement_ms -= ir155_state.periodTriggerTime_ms;
            ir155_state.measurement.measurementState = IR155_MEASUREMENT_NOT_VALID;
        } else {
            /* 0 < hysteresisCounter < measCycleTime
             * Measurement is valid from that moment on */
            ir155_state.timeUntilValidMeasurement_ms = 0u;
        }

        if (ir155_state.timeUntilValidMeasurement_ms == 0u) {
            /* If hysteresis is over, reset diag flag and reset grounderror flag */
            /* TODO: How to handle the ground error fault after reset? Do this in diag callback?*/
            /* Reset non-volatile ground error flag - Do this in diag callback? */
            fram_insulationFlags.groundErrorDetected = false;
        }
    }

    /* Evaluate if resistance measurement is valid or not */
    if (ir155_state.measurement.measurementState == IR155_MEASUREMENT_NOT_VALID) {
        /* Measurement result is not valid */
        pTableInsulationMonitoring->isInsulationMeasurementValid = false;
    } else {
        pTableInsulationMonitoring->isInsulationMeasurementValid = true;
    }

    /* Set measured resistance value */
    pTableInsulationMonitoring->insulationResistance_kOhm = ir155_state.measurement.resistance_kOhm;

    /* Evaluate device state */
    if ((ir155_state.measurement.measurementState == IR155_IMD_ERROR_MEASUREMENT) ||
        (ir155_state.measurement.measurementState == IR155_IMD_ERROR_MEASUREMENT_UNKNOWN)) {
        /* Mark device flags and insulation measurement as invalid if a device error is detected */
        pTableInsulationMonitoring->areDeviceFlagsValid          = false;
        pTableInsulationMonitoring->isInsulationMeasurementValid = false;
    } else {
        pTableInsulationMonitoring->areDeviceFlagsValid = true;
    }

    /* Set error flag if Pin OHKS detects an error
     * Pin state high: No fault; Insulation resistance > response value
     * Pin state low:  Insulation resistance <= response value detected;
     *                 Device error; Fault in the earth connection
     *                 Undervoltage detected or device switched off
     */
    if (ir155_state.measurement.digitalStatusPin == STD_PIN_LOW) {
        pTableInsulationMonitoring->dfIsCriticalResistanceDetected = true;
    } else {
        pTableInsulationMonitoring->dfIsCriticalResistanceDetected = false;
    }

    /* Check for device error: invalid measurement period or duty-cycle detected */
    if ((ir155_state.measurement.measurementState == IR155_RESISTANCE_MEASUREMENT_UNKNOWN) ||
        (ir155_state.measurement.measurementState == IR155_RESISTANCE_ESTIMATION_UNKNOWN) ||
        (ir155_state.measurement.measurementState == IR155_UNDERVOLTAGE_MEASUREMENT_UNKNOWN) ||
        (ir155_state.measurement.measurementState == IR155_GROUND_ERROR_MODE_UNKNOWN)) {
        pTableInsulationMonitoring->dfIsDeviceErrorDetected = true;
    } else {
        pTableInsulationMonitoring->dfIsDeviceErrorDetected = false;
    }

    /* Check for chassis fault */
    pTableInsulationMonitoring->dfIsChassisShortToHvPlus  = false; /* This feature is not supported by the device */
    pTableInsulationMonitoring->dfIsChassisShortToHvMinus = false; /* This feature is not supported by the device */
    if (ir155_state.measurement.measurementState == IR155_GROUND_ERROR_MODE) {
        pTableInsulationMonitoring->dfIsChassisFaultDetected = true;
    } else {
        pTableInsulationMonitoring->dfIsDeviceErrorDetected = false;
    }

    /* No digital signal for warning threshold available for this device -> use measured insulation resistance */
    if (pTableInsulationMonitoring->insulationResistance_kOhm <= IMD_WARNING_THRESHOLD_INSULATION_RESISTANCE_kOhm) {
        pTableInsulationMonitoring->dfIsWarnableResistanceDetected = true;
    } else {
        pTableInsulationMonitoring->dfIsWarnableResistanceDetected = false;
    }

    /* TODO: bool dfIsMeasurementUpToDate; Check if measurement result is up to data */
    return IMD_FSM_STATE_RUNNING;
}

/*========== Extern Function Implementations ================================*/
extern IMD_FSM_STATES_e IMD_ProcessInitializationState(void) {
    return IR155_InitializeModule();
}

extern IMD_FSM_STATES_e IMD_ProcessEnableState(void) {
    /* Enable supply for Bender */
    IO_PinSet(&IR155_SUPPLY_ENABLE_PORT->DOUT, IR155_SUPPLY_ENABLE_PIN);
    return IMD_FSM_STATE_RUNNING;
}

extern IMD_FSM_STATES_e IMD_ProcessRunningState(DATA_BLOCK_INSULATION_MONITORING_s *pTableInsulationMonitoring) {
    FAS_ASSERT(pTableInsulationMonitoring != NULL_PTR);
    return IR155_MeasureInsulation(pTableInsulationMonitoring);
}

extern IMD_FSM_STATES_e IMD_ProcessShutdownState(void) {
    /* Disable supply for Bender */
    IO_PinReset(&IR155_SUPPLY_ENABLE_PORT->DOUT, IR155_SUPPLY_ENABLE_PIN);
    return IMD_FSM_STATE_IMD_ENABLE;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
