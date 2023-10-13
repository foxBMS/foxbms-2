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
 * @file    adi_ades183x.c
 * @author  foxBMS Team
 * @date    2020-12-09 (date of creation)
 * @updated 2023-10-12 (date of last update)
 * @version v1.6.0
 * @ingroup DRIVERS
 * @prefix  ADI
 *
 * @brief   Driver for the ADI analog front-end.
 *
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
#pragma SET_DATA_SECTION(".sharedRAM")
static uint16_t adi_bufferRxPec[ADI_N_BYTES_FOR_DATA_TRANSMISSION] = {0};
static uint16_t adi_bufferTxPec[ADI_N_BYTES_FOR_DATA_TRANSMISSION] = {0};
#pragma SET_DATA_SECTION()
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
 * @param   adiState state of the ADI driver
 */

static void ADI_AccessToDatabase(ADI_STATE_s *adiState);

/**
 * @brief   Sets the balancing according to the control values read in the
 *          database.
 * @brief   To set balancing for the cells, the corresponding bits have to be
 *          written in the configuration register.
 *          The ades183x driver only executes the balancing orders written by
 *          the BMS in the database.
 * @param   adiState state of the ADI driver
 */
static void ADI_BalanceControl(ADI_STATE_s *adiState);

/**
 * @brief   Checks the requests made to the ades183x driver.
 * @param   request request to be made with string addressed
 * @return  STD_OK if request queue was full, STD_NOT_OK otherwise
 */
static STD_RETURN_TYPE_e ADI_GetRequest(AFE_REQUEST_e *request);

/**
 * @brief   Runs the initialization sequence of the driver.
 * @param   adiState    state of the ADI driver
 * @param   request     request to be made with string addressed
 * @return  true if measurement has been started, false otherwise
 *
 */
static bool ADI_ProcessMeasurementNotStartedState(ADI_STATE_s *adiState, AFE_REQUEST_e *request);

/**
 * @brief   sets the measurement initialization status.
 */
static void ADI_SetFirstMeasurementCycleFinished(ADI_STATE_s *adiState);

/*========== Static Function Implementations ================================*/

static void ADI_AccessToDatabase(ADI_STATE_s *adiState) {
    FAS_ASSERT(adiState != NULL_PTR);

    /* Increment state variable each time new values are written into database */
    (void)DATA_WRITE_DATA(
        adiState->data.cellVoltage,
        adiState->data.cellVoltageFiltered,
        adiState->data.allGpioVoltages,
        adiState->data.cellTemperature);
    ADI_Wait(2u); /* Block task to leave CPU time for the other tasks */

    (void)DATA_READ_DATA(adiState->data.balancingControl);
}

/* RequirementId: D7.1 V0R4 FUN-6.10.01.01 */
static void ADI_BalanceControl(ADI_STATE_s *adiState) {
    FAS_ASSERT(adiState != NULL_PTR);

    /* Unmute balancing, cell voltage measurements must have been stopped before */
    ADI_CopyCommandBits(adi_cmdUnmute, adi_command);
    ADI_TransmitCommand(adi_command, adiState);

    /* Write the balancing registers of the ades183x */
    ADI_DetermineBalancingRegisterConfiguration(adiState);

    /* Wait ADI_BALANCING_TIME_ms milliseconds in order to balance
       Measurements are stopped during this time */
    ADI_Wait(ADI_BALANCING_TIME_ms);

    /* Mute balancing, so that cell voltage measurements can be restarted */
    ADI_CopyCommandBits(adi_cmdMute, adi_command);
    ADI_TransmitCommand(adi_command, adiState);
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

static bool ADI_ProcessMeasurementNotStartedState(ADI_STATE_s *adiState, AFE_REQUEST_e *request) {
    FAS_ASSERT(adiState != NULL_PTR);
    FAS_ASSERT(request != NULL_PTR);

    bool measurementStarted           = false;
    STD_RETURN_TYPE_e requestReceived = ADI_GetRequest(request);
    if (requestReceived == STD_OK) { /* request queue was not empty */
        if (*request == AFE_START_REQUEST) {
            ADI_InitializeMeasurement(adiState);
            measurementStarted = true;
        } else { /* Until requested to start, block task to leave CPU time for the other tasks */
            ADI_Wait(1u);
        }
    } else { /* Until requested to start, block task to leave CPU time for the other tasks */
        ADI_Wait(1u);
    }
    return measurementStarted;
}

static void ADI_RunCurrentStringMeasurement(ADI_STATE_s *adiState) {
    FAS_ASSERT(adiState != NULL_PTR);

    /* Start auxiliary voltage measurement, all channels */
    ADI_CopyCommandBits(adi_cmdAdax, adi_command);
    ADI_WriteCommandConfigurationBits(adi_command, ADI_ADAX_OW_POS, ADI_ADAX_OW_LEN, 0u);
    ADI_WriteCommandConfigurationBits(adi_command, ADI_ADAX_PUP_POS, ADI_ADAX_PUP_LEN, 0u);
    ADI_WriteCommandConfigurationBits(adi_command, ADI_ADAX_CH4_POS, ADI_ADAX_CH4_LEN, 0u);
    ADI_WriteCommandConfigurationBits(adi_command, ADI_ADAX_CH03_POS, ADI_ADAX_CH03_LEN, 0u);
    ADI_TransmitCommand(adi_command, adiState);
    /* Start redundant auxiliary voltage measurement, one channel */
    ADI_CopyCommandBits(adi_cmdAdax2, adi_command);
    ADI_WriteCommandConfigurationBits(
        adi_command,
        ADI_ADAX2_CH03_POS,
        ADI_ADAX2_CH03_LEN,
        adiState->redundantAuxiliaryChannel[adiState->currentString]);
    ADI_TransmitCommand(adi_command, adiState);

    ADI_Wait(ADI_WAIT_TIME_1_FOR_ADAX_FULL_CYCLE);

    /* Snapshot to freeze cell voltage measurement result registers */
    ADI_CopyCommandBits(adi_cmdSnap, adi_command);
    ADI_TransmitCommand(adi_command, adiState);
    /* Retrieve filtered cell voltages */
    ADI_GetVoltages(adiState, ADI_CELL_VOLTAGE_REGISTER, ADI_CELL_VOLTAGE);
    ADI_GetVoltages(adiState, ADI_FILTERED_CELL_VOLTAGE_REGISTER, ADI_FILTERED_CELL_VOLTAGE);
    /* Release snapshot to refresh cell voltage measurement result registers again */
    ADI_CopyCommandBits(adi_cmdUnsnap, adi_command);
    ADI_TransmitCommand(adi_command, adiState);

    /* Wait until auxiliary measurement cycle is finished */
    ADI_Wait(ADI_WAIT_TIME_2_FOR_ADAX_FULL_CYCLE);

    /* Retrieve GPIO voltages (all channels) */
    ADI_GetGpioVoltages(adiState, ADI_AUXILIARY_REGISTER, ADI_AUXILIARY_VOLTAGE);
    /* Get temperatures via GPIO voltages */
    ADI_GetTemperatures(adiState);

    ADI_AccessToDatabase(adiState);

    /* If at least one cell is balanced, measurements are stopped before activating balancing */
    if (adiState->data.balancingControl->nrBalancedCells[adiState->currentString] > 0u) {
        /* Stop cell voltage measurements before activating balancing */
        ADI_StopContinuousCellVoltageMeasurements(adiState);
        /* Activate balancing (if necessary) */
        ADI_BalanceControl(adiState);
        /* Wait ADI_BALANCING_TIME_ms milliseconds in order to balance
           Measurements are stopped during this time */
        ADI_Wait(ADI_BALANCING_TIME_ms);
        /* Restart cell voltage measurements before activating balancing */
        ADI_RestartContinuousCellVoltageMeasurements(adiState);
    }

    ADI_Diagnostic(adiState);

    /* Cycle finished for string, clear values to check that they are not stuck during next reading */
    ADI_CopyCommandBits(adi_cmdClrcell, adi_command);
    ADI_TransmitCommand(adi_command, adiState);
}

static void ADI_SetFirstMeasurementCycleFinished(ADI_STATE_s *adiState) {
    FAS_ASSERT(adiState != NULL_PTR);
    OS_EnterTaskCritical();
    adiState->firstMeasurementMade = true;
    OS_ExitTaskCritical();
}

/*========== Extern Function Implementations ================================*/

/* START extern functions to adapt if running in other environment (e.g., bare metal) */

extern void ADI_ActivateInterfaceBoard(void) {
    /* Set 3rd PE pins to enable daisy chains */
    PEX_SetPinDirectionOutput(PEX_PORT_EXPANDER3, PEX_PIN10);
    PEX_SetPinDirectionOutput(PEX_PORT_EXPANDER3, PEX_PIN11);
    PEX_SetPinDirectionOutput(PEX_PORT_EXPANDER3, PEX_PIN12);
    PEX_SetPinDirectionOutput(PEX_PORT_EXPANDER3, PEX_PIN13);
    PEX_SetPinDirectionOutput(PEX_PORT_EXPANDER3, PEX_PIN14);
    PEX_SetPinDirectionOutput(PEX_PORT_EXPANDER3, PEX_PIN15);
    PEX_SetPinDirectionOutput(PEX_PORT_EXPANDER3, PEX_PIN16);
    PEX_SetPinDirectionOutput(PEX_PORT_EXPANDER3, PEX_PIN17);
    PEX_SetPin(PEX_PORT_EXPANDER3, PEX_PIN10);
    PEX_SetPin(PEX_PORT_EXPANDER3, PEX_PIN11);
    PEX_SetPin(PEX_PORT_EXPANDER3, PEX_PIN12);
    PEX_SetPin(PEX_PORT_EXPANDER3, PEX_PIN13);
    PEX_SetPin(PEX_PORT_EXPANDER3, PEX_PIN14);
    PEX_SetPin(PEX_PORT_EXPANDER3, PEX_PIN15);
    PEX_SetPin(PEX_PORT_EXPANDER3, PEX_PIN16);
    PEX_SetPin(PEX_PORT_EXPANDER3, PEX_PIN17);
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

extern bool ADI_IsFirstMeasurementCycleFinished(ADI_STATE_s *adiState) {
    FAS_ASSERT(adiState != NULL_PTR);
    return (adiState->firstMeasurementMade);
}

/* END extern functions to adapt if running in other environment (e.g., bare metal) */

/* RequirementId: D7.1 V0R4 FUN-1.10.01.03 */
/* RequirementId: D7.1 V0R4 FUN-2.10.01.03 */
/* RequirementId: D7.1 V0R4 FUN-4.10.01.03 */
extern void ADI_MeasurementCycle(ADI_STATE_s *adiState) {
    FAS_ASSERT(adiState != NULL_PTR);
    STD_RETURN_TYPE_e requestReceived = STD_OK;
    AFE_REQUEST_e request             = AFE_NO_REQUEST;

    /* AXIVION Next Line Style MisraC2012-2.2 FaultDetection-DeadBranches: non-blocking driver requires an infinite
       * loop for the driver implementation */
    while (FOREVER()) {
        if (adiState->measurementStarted == false) { /* Wait until requested to start */
            adiState->measurementStarted = ADI_ProcessMeasurementNotStartedState(adiState, &request);
        } else {
            while (adiState->currentString < adiState->spiNumberInterfaces) {
                ADI_RunCurrentStringMeasurement(adiState);
                ++adiState->currentString;
            }
            adiState->currentString = 0u;
            if (ADI_IsFirstMeasurementCycleFinished(adiState) == false) {
                ADI_SetFirstMeasurementCycleFinished(adiState);
            }
            requestReceived = ADI_GetRequest(&request);
            if (requestReceived == STD_OK) {
                /* request queue was not empty */
                if (request == AFE_STOP_REQUEST) {
                    adiState->measurementStarted = false;
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
extern void TEST_ADI_AccessToDatabase(ADI_STATE_s *adiState) {
    ADI_AccessToDatabase(adiState);
}
extern void TEST_ADI_BalanceControl(ADI_STATE_s *adiState) {
    ADI_BalanceControl(adiState);
}
extern STD_RETURN_TYPE_e TEST_ADI_GetRequest(AFE_REQUEST_e *request) {
    return ADI_GetRequest(request);
}
extern bool TEST_ADI_ProcessMeasurementNotStartedState(ADI_STATE_s *adiState, AFE_REQUEST_e *request) {
    return ADI_ProcessMeasurementNotStartedState(adiState, request);
}
extern void TEST_ADI_RunCurrentStringMeasurement(ADI_STATE_s *adiState) {
    ADI_RunCurrentStringMeasurement(adiState);
}
extern void TEST_ADI_SetFirstMeasurementCycleFinished(ADI_STATE_s *adiState) {
    ADI_SetFirstMeasurementCycleFinished(adiState);
}
#endif
