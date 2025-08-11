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
 * @file    adi_ades183x.c
 * @author  foxBMS Team
 * @date    2020-12-09 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup DRIVERS
 * @prefix  ADI
 *
 * @brief   Driver for the ADI analog front-end.
 * @details TODO
 */

/*========== Includes =======================================================*/
/* clang-format off */
#include "adi_ades183x.h"
/* clang-format on */

#include "adi_ades183x_balancing.h"
#include "adi_ades183x_buffers.h"
#include "adi_ades183x_commands.h"
#include "adi_ades183x_diagnostic.h"
#include "adi_ades183x_gpio_voltages.h"
#include "adi_ades183x_helpers.h"
#include "adi_ades183x_initialization.h"
#include "adi_ades183x_pec.h"
#include "adi_ades183x_temperatures.h"
#include "adi_ades183x_voltages.h"
#include "database.h"
#include "ftask.h"
#include "infinite-loop-helper.h"
#include "os.h"
#include "pex.h"

#include <stdbool.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/
/**
 * PEC buffer for RX and TX
 * @{
 */
/* AXIVION Disable Style MisraC2012-1.2: The Pec buffer must be put in the shared RAM section for performance reasons */
#pragma SET_DATA_SECTION(".sharedRAM")
static uint16_t adi_bufferRxPec[ADI_N_BYTES_FOR_DATA_TRANSMISSION] = {0};
static uint16_t adi_bufferTxPec[ADI_N_BYTES_FOR_DATA_TRANSMISSION] = {0};
#pragma SET_DATA_SECTION()
/* AXIVION Enable Style MisraC2012-1.2 */
/**@}*/

/** local copies of database tables */
/**@{*/
static DATA_BLOCK_CELL_VOLTAGE_s adi_cellVoltage           = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE_BASE};
static DATA_BLOCK_CELL_VOLTAGE_s adi_cellVoltageAverage    = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE_BASE};
static DATA_BLOCK_CELL_VOLTAGE_s adi_cellVoltageFiltered   = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE_BASE};
static DATA_BLOCK_CELL_VOLTAGE_s adi_cellVoltageRedundant  = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE_BASE};
static DATA_BLOCK_CELL_TEMPERATURE_s adi_cellTemperature   = {.header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE_BASE};
static DATA_BLOCK_BALANCING_CONTROL_s adi_balancingControl = {.header.uniqueId = DATA_BLOCK_ID_BALANCING_CONTROL};
static DATA_BLOCK_ALL_GPIO_VOLTAGES_s adi_allGpioVoltage   = {.header.uniqueId = DATA_BLOCK_ID_ALL_GPIO_VOLTAGES_BASE};
static DATA_BLOCK_ALL_GPIO_VOLTAGES_s adi_allGpioVoltageRedundant = {
    .header.uniqueId = DATA_BLOCK_ID_ALL_GPIO_VOLTAGES_BASE};
static DATA_BLOCK_CELL_VOLTAGE_s adi_cellVoltageOpenWireEven    = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE_BASE};
static DATA_BLOCK_CELL_VOLTAGE_s adi_cellVoltageOpenWireOdd     = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE_BASE};
static DATA_BLOCK_CELL_VOLTAGE_s adi_cellVoltageAverageOpenWire = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE_BASE};
static DATA_BLOCK_CELL_VOLTAGE_s adi_cellVoltageRedundantOpenWire = {
    .header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE_BASE};
static DATA_BLOCK_ALL_GPIO_VOLTAGES_s adi_allGpioVoltageOpenWire = {
    .header.uniqueId = DATA_BLOCK_ID_ALL_GPIO_VOLTAGES_BASE};
static DATA_BLOCK_OPEN_WIRE_s adi_openWire = {.header.uniqueId = DATA_BLOCK_ID_OPEN_WIRE_BASE};
static ADI_ERROR_TABLE_s adi_errorTable    = {0}; /*!< init in ADI_ResetErrorTable-function */
/**@}*/

/*========== Extern Constant and Variable Definitions =======================*/

ADI_STATE_s adi_stateBase = {
    .measurementStarted                = false,
    .firstMeasurementMade              = false,
    .firstDiagnosticMade               = false,
    .currentString                     = 0u,
    .redundantAuxiliaryChannel         = {0u},
    .data.txBuffer                     = adi_bufferTxPec,
    .data.rxBuffer                     = adi_bufferRxPec,
    .data.cellVoltage                  = &adi_cellVoltage,
    .data.cellVoltageAverage           = &adi_cellVoltageAverage,
    .data.cellVoltageFiltered          = &adi_cellVoltageFiltered,
    .data.cellVoltageRedundant         = &adi_cellVoltageRedundant,
    .data.cellTemperature              = &adi_cellTemperature,
    .data.balancingControl             = &adi_balancingControl,
    .data.errorTable                   = &adi_errorTable,
    .data.allGpioVoltages              = &adi_allGpioVoltage,
    .data.allGpioVoltagesRedundant     = &adi_allGpioVoltageRedundant,
    .data.cellVoltageOpenWireEven      = &adi_cellVoltageOpenWireEven,
    .data.cellVoltageOpenWireOdd       = &adi_cellVoltageOpenWireOdd,
    .data.cellVoltageAverageOpenWire   = &adi_cellVoltageAverageOpenWire,
    .data.cellVoltageRedundantOpenWire = &adi_cellVoltageRedundantOpenWire,
    .data.allGpioVoltageOpenWire       = &adi_allGpioVoltageOpenWire,
    .data.openWire                     = &adi_openWire,
};

/*========== Static Function Prototypes =====================================*/

/**
 * @brief   Read local variables from database and write local variables to
 *          database.
 * @param   pAdiState state of the ADI driver
 */

static void ADI_AccessToDatabase(ADI_STATE_s *pAdiState);

/**
 * @brief   Sets the balancing according to the control values read in the
 *          database.
 * @brief   To set balancing for the cells, the corresponding bits have to be
 *          written in the configuration register.
 *          The ades183x driver only executes the balancing orders written by
 *          the BMS in the database.
 * @param   pAdiState state of the ADI driver
 */
static void ADI_BalanceControl(ADI_STATE_s *pAdiState);

/**
 * @brief   Checks the requests made to the ades183x driver.
 * @param   request request to be made with string addressed
 * @return  STD_OK if request queue was full, STD_NOT_OK otherwise
 */
static STD_RETURN_TYPE_e ADI_GetRequest(AFE_REQUEST_e *request);

/**
 * @brief   Runs the initialization sequence of the driver.
 * @param   pAdiState   state of the ADI driver
 * @param   request     request to be made with string addressed
 * @return  true if measurement has been started, false otherwise
 *
 */
static bool ADI_ProcessMeasurementNotStartedState(ADI_STATE_s *pAdiState, AFE_REQUEST_e *request);

/**
 * @brief   Sets the measurement initialization status.
 * @param   pAdiState state of the ADI driver
 */
static void ADI_SetFirstMeasurementCycleFinished(ADI_STATE_s *pAdiState);

/**
 * @brief Sanity checking for software configurations
 */
static void ADI_SanityConfigurationCheck(ADI_STATE_s *pAdiState);

/*========== Static Function Implementations ================================*/

static void ADI_AccessToDatabase(ADI_STATE_s *pAdiState) {
    FAS_ASSERT(pAdiState != NULL_PTR);

    /* Increment state variable each time new values are written into database */
    (void)DATA_WRITE_DATA(
        pAdiState->data.cellVoltage, pAdiState->data.allGpioVoltages, pAdiState->data.cellTemperature);
    ADI_Wait(2u); /* Block task to leave CPU time for the other tasks */

    (void)DATA_READ_DATA(pAdiState->data.balancingControl);
}

/* RequirementId: D7.1 V1R0 FUN-6.10.01.02 */
static void ADI_BalanceControl(ADI_STATE_s *pAdiState) {
    FAS_ASSERT(pAdiState != NULL_PTR);

    /* Unmute balancing, cell voltage measurements must have been stopped before */
    ADI_CopyCommandBytes(adi_cmdUnmute, adi_command);
    ADI_TransmitCommand(adi_command, pAdiState);

    /* Write the balancing registers of the ades183x */
    ADI_DetermineBalancingRegisterConfiguration(pAdiState);

    /* Wait ADI_BALANCING_TIME_ms milliseconds in order to balance
       Measurements are stopped during this time */
    ADI_Wait(ADI_BALANCING_TIME_ms);

    /* Mute balancing, so that cell voltage measurements can be restarted */
    ADI_CopyCommandBytes(adi_cmdMute, adi_command);
    ADI_TransmitCommand(adi_command, pAdiState);
}

static STD_RETURN_TYPE_e ADI_GetRequest(AFE_REQUEST_e *request) {
    FAS_ASSERT(request != NULL_PTR);
    STD_RETURN_TYPE_e requestReceived = STD_NOT_OK;
    OS_STD_RETURN_e receivedFromQueue =
        OS_ReceiveFromQueue(ftsk_afeRequestQueue, (void *)request, ADI_QUEUE_TIMEOUT_MS);
    if (receivedFromQueue == OS_SUCCESS) {
        /* request queue was full, request retrieved */
        requestReceived = STD_OK;
    }
    return requestReceived;
}

static bool ADI_ProcessMeasurementNotStartedState(ADI_STATE_s *pAdiState, AFE_REQUEST_e *request) {
    FAS_ASSERT(pAdiState != NULL_PTR);
    FAS_ASSERT(request != NULL_PTR);

    bool measurementStarted           = false;
    STD_RETURN_TYPE_e requestReceived = ADI_GetRequest(request);
    if (requestReceived == STD_OK) { /* request queue was not empty */
        if (*request == AFE_START_REQUEST) {
            ADI_SanityConfigurationCheck(pAdiState);
            ADI_InitializeMeasurement(pAdiState);
            measurementStarted = true;
        } else { /* Until requested to start, block task to leave CPU time for the other tasks */
            ADI_Wait(1u);
        }
    } else { /* Until requested to start, block task to leave CPU time for the other tasks */
        ADI_Wait(1u);
    }
    return measurementStarted;
}

static void ADI_RunCurrentStringMeasurement(ADI_STATE_s *pAdiState) {
    FAS_ASSERT(pAdiState != NULL_PTR);

    /* Start auxiliary voltage measurement, all channels */
    ADI_CopyCommandBytes(adi_cmdAdax, adi_command);
    ADI_WriteCommandConfigurationBits(adi_command, ADI_ADAX_OW_POS, ADI_ADAX_OW_LEN, 0u);
    ADI_WriteCommandConfigurationBits(adi_command, ADI_ADAX_PUP_POS, ADI_ADAX_PUP_LEN, 0u);
    ADI_WriteCommandConfigurationBits(adi_command, ADI_ADAX_CH4_POS, ADI_ADAX_CH4_LEN, 0u);
    ADI_WriteCommandConfigurationBits(adi_command, ADI_ADAX_CH03_POS, ADI_ADAX_CH03_LEN, 0u);
    ADI_TransmitCommand(adi_command, pAdiState);
    /* Start redundant auxiliary voltage measurement, one channel */
    ADI_CopyCommandBytes(adi_cmdAdax2, adi_command);
    ADI_WriteCommandConfigurationBits(
        adi_command,
        ADI_ADAX2_CH03_POS,
        ADI_ADAX2_CH03_LEN,
        pAdiState->redundantAuxiliaryChannel[pAdiState->currentString]);
    ADI_TransmitCommand(adi_command, pAdiState);

    ADI_Wait(ADI_WAIT_TIME_1_FOR_ADAX_FULL_CYCLE);

    /* Snapshot to freeze cell voltage measurement result registers */
    ADI_CopyCommandBytes(adi_cmdSnap, adi_command);
    ADI_TransmitCommand(adi_command, pAdiState);
    /* Retrieve filtered cell voltages */
    ADI_GetVoltages(pAdiState, ADI_CELL_VOLTAGE_REGISTER, ADI_CELL_VOLTAGE);
    ADI_GetVoltages(pAdiState, ADI_FILTERED_CELL_VOLTAGE_REGISTER, ADI_FILTERED_CELL_VOLTAGE);
    /* Release snapshot to refresh cell voltage measurement result registers again */
    ADI_CopyCommandBytes(adi_cmdUnsnap, adi_command);
    ADI_TransmitCommand(adi_command, pAdiState);

    /* Wait until auxiliary measurement cycle is finished */
    ADI_Wait(ADI_WAIT_TIME_2_FOR_ADAX_FULL_CYCLE);

    /* Retrieve string and module voltages */
    ADI_GetStringAndModuleVoltage(pAdiState);
    /* Retrieve GPIO voltages (all channels) */
    ADI_GetGpioVoltages(pAdiState, ADI_AUXILIARY_REGISTER, ADI_AUXILIARY_VOLTAGE);
    /* Get temperatures via GPIO voltages */
    ADI_GetTemperatures(pAdiState);

    ADI_AccessToDatabase(pAdiState);

    /* If at least one cell is balanced, measurements are stopped before activating balancing */
    if (pAdiState->data.balancingControl->nrBalancedCells[pAdiState->currentString] > 0u) {
        /* Stop cell voltage measurements before activating balancing */
        ADI_StopContinuousCellVoltageMeasurements(pAdiState);
        /* Activate balancing (if necessary) */
        ADI_BalanceControl(pAdiState);
        /* Wait ADI_BALANCING_TIME_ms milliseconds in order to balance
           Measurements are stopped during this time */
        ADI_Wait(ADI_BALANCING_TIME_ms);
        /* Restart cell voltage measurements before activating balancing */
        ADI_RestartContinuousCellVoltageMeasurements(pAdiState);
    }

    ADI_Diagnostic(pAdiState);

    /* Cycle finished for string, clear values to check that they are not stuck during next reading */
    ADI_CopyCommandBytes(adi_cmdClrcell, adi_command);
    ADI_TransmitCommand(adi_command, pAdiState);
}

static void ADI_SetFirstMeasurementCycleFinished(ADI_STATE_s *pAdiState) {
    FAS_ASSERT(pAdiState != NULL_PTR);
    OS_EnterTaskCritical();
    pAdiState->firstMeasurementMade = true;
    OS_ExitTaskCritical();
}

static void ADI_SanityConfigurationCheck(ADI_STATE_s *pAdiState) {
    FAS_ASSERT(pAdiState != NULL_PTR);

    /* Check configuration for temperature sensors */
    uint8_t configuredTemperatureSensorInputs = 0u;
    for (uint16_t gpioIndex = 0u; gpioIndex < SLV_NR_OF_GPIOS_PER_MODULE; gpioIndex++) {
        if (adi_temperatureInputsUsed[gpioIndex] != 0u) {
            configuredTemperatureSensorInputs++;
        }
    }
    if (configuredTemperatureSensorInputs != BS_NR_OF_TEMP_SENSORS_PER_MODULE) {
        /* According to SW configuration number of configured temperature sensors
         * configured in AFE driver then in overall battery system configuration */
        FAS_ASSERT(FAS_TRAP);
    }
}

/*========== Extern Function Implementations ================================*/

/* START extern functions to adapt if running in other environment (e.g., bare metal) */

extern void ADI_ActivateInterfaceBoard(void) {
    /* Set 3rd PE pins to enable daisy chains */
    PEX_SetPinDirectionOutput(PEX_PORT_EXPANDER3, PEX_PORT_1_PIN_0);
    PEX_SetPinDirectionOutput(PEX_PORT_EXPANDER3, PEX_PORT_1_PIN_1);
    PEX_SetPinDirectionOutput(PEX_PORT_EXPANDER3, PEX_PORT_1_PIN_2);
    PEX_SetPinDirectionOutput(PEX_PORT_EXPANDER3, PEX_PORT_1_PIN_3);
    PEX_SetPinDirectionOutput(PEX_PORT_EXPANDER3, PEX_PORT_1_PIN_4);
    PEX_SetPinDirectionOutput(PEX_PORT_EXPANDER3, PEX_PORT_1_PIN_5);
    PEX_SetPinDirectionOutput(PEX_PORT_EXPANDER3, PEX_PORT_1_PIN_6);
    PEX_SetPinDirectionOutput(PEX_PORT_EXPANDER3, PEX_PORT_1_PIN_7);
    PEX_SetPin(PEX_PORT_EXPANDER3, PEX_PORT_1_PIN_0);
    PEX_SetPin(PEX_PORT_EXPANDER3, PEX_PORT_1_PIN_1);
    PEX_SetPin(PEX_PORT_EXPANDER3, PEX_PORT_1_PIN_2);
    PEX_SetPin(PEX_PORT_EXPANDER3, PEX_PORT_1_PIN_3);
    PEX_SetPin(PEX_PORT_EXPANDER3, PEX_PORT_1_PIN_4);
    PEX_SetPin(PEX_PORT_EXPANDER3, PEX_PORT_1_PIN_5);
    PEX_SetPin(PEX_PORT_EXPANDER3, PEX_PORT_1_PIN_6);
    PEX_SetPin(PEX_PORT_EXPANDER3, PEX_PORT_1_PIN_7);
}

extern STD_RETURN_TYPE_e ADI_MakeRequest(AFE_REQUEST_e request) {
    FAS_ASSERT(request < AFE_REQUEST_E_MAX);

    STD_RETURN_TYPE_e requestSubmitted = STD_NOT_OK;

    if (OS_SUCCESS == OS_SendToBackOfQueue(ftsk_afeRequestQueue, (void *)&request, ADI_QUEUE_TIMEOUT_MS)) {
        /* request queue was empty, request submitted */
        requestSubmitted = STD_OK;
    }
    return requestSubmitted;
}

extern bool ADI_IsFirstMeasurementCycleFinished(ADI_STATE_s *pAdiState) {
    FAS_ASSERT(pAdiState != NULL_PTR);
    return (pAdiState->firstMeasurementMade);
}

/* END extern functions to adapt if running in other environment (e.g., bare metal) */

/* RequirementId: D7.1 V1R0 FUN-1.10.01.03 */
/* RequirementId: D7.1 V1R0 FUN-2.10.01.03 */
/* RequirementId: D7.1 V1R0 FUN-4.10.01.03 */
extern void ADI_MeasurementCycle(ADI_STATE_s *pAdiState) {
    FAS_ASSERT(pAdiState != NULL_PTR);
    STD_RETURN_TYPE_e requestReceived = STD_OK;
    AFE_REQUEST_e request             = AFE_NO_REQUEST;

    /* AXIVION Next Line Style MisraC2012-2.2 FaultDetection-DeadBranches: non-blocking driver requires an infinite
     * loop for the driver implementation */
    while (FOREVER()) {
        if (pAdiState->measurementStarted == false) { /* Wait until requested to start */
            pAdiState->measurementStarted = ADI_ProcessMeasurementNotStartedState(pAdiState, &request);
        } else {
            while (pAdiState->currentString < pAdiState->spiNumberInterfaces) {
                ADI_RunCurrentStringMeasurement(pAdiState);
                ++pAdiState->currentString;
            }
            pAdiState->currentString = 0u;
            if (ADI_IsFirstMeasurementCycleFinished(pAdiState) == false) {
                ADI_SetFirstMeasurementCycleFinished(pAdiState);
            }
            requestReceived = ADI_GetRequest(&request);
            if (requestReceived == STD_OK) {
                /* request queue was not empty */
                if (request == AFE_STOP_REQUEST) {
                    pAdiState->measurementStarted = false;
                }
            }
        }

#ifdef UNITY_UNIT_TEST /* break out of loop in unit test*/
        break;
#endif
    }
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
extern void TEST_ADI_AccessToDatabase(ADI_STATE_s *pAdiState) {
    ADI_AccessToDatabase(pAdiState);
}
extern void TEST_ADI_BalanceControl(ADI_STATE_s *pAdiState) {
    ADI_BalanceControl(pAdiState);
}
extern STD_RETURN_TYPE_e TEST_ADI_GetRequest(AFE_REQUEST_e *request) {
    return ADI_GetRequest(request);
}
extern bool TEST_ADI_ProcessMeasurementNotStartedState(ADI_STATE_s *pAdiState, AFE_REQUEST_e *request) {
    return ADI_ProcessMeasurementNotStartedState(pAdiState, request);
}
extern void TEST_ADI_RunCurrentStringMeasurement(ADI_STATE_s *pAdiState) {
    ADI_RunCurrentStringMeasurement(pAdiState);
}
extern void TEST_ADI_SetFirstMeasurementCycleFinished(ADI_STATE_s *pAdiState) {
    ADI_SetFirstMeasurementCycleFinished(pAdiState);
}
#endif
