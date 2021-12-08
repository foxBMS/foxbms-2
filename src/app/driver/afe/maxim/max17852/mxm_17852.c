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
 * @file    mxm_17852.c
 * @author  foxBMS Team
 * @date    2021-11-24 (date of creation)
 * @updated 2021-12-06 (date of last update)
 * @ingroup DRIVERS
 * @prefix  MXM
 *
 * @brief   Operation state machine implementation for the MAX17852
 *
 * @details This file contains that part of the state machine that is executed
 *          during the operation state. It is adapted for MAX17852.
 *
 */

/*========== Includes =======================================================*/
/* clang-format off */
#include "mxm_1785x.h"
/* clang-format on */

#include "database.h"
#include "mxm_1785x_tools.h"
#include "mxm_battery_management.h"
#include "mxm_registry.h"
#include "os.h"

/*========== Macros and Definitions =========================================*/
/** @defgroup MXM_I2C_IMPLEMENTATION symbols and settings pertaining to the I2C implementation in MXM
 * @{
 */
/** @brief address of MUX0 */
#define MXM_I2C_MUX0_ADDRESS (0x4Cu)
/** @brief address of MUX1 */
#define MXM_I2C_MUX1_ADDRESS (0x4Du)
/**@}*/

/** @brief Delay in milliseconds before the balancing status is updated */
#define MXM_DELAY_BALANCING 10000u

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/
extern MXM_MODEL_ID_e MXM_GetModelIdOfDaisyChain(void) {
    return MXM_MODEL_ID_MAX17852;
}

extern void MXM_StateMachineOperation(MXM_MONITORING_INSTANCE_s *pState) {
    FAS_ASSERT(pState != NULL_PTR);

    /**
     * @brief Mapping of voltage registers
     *
     * This array maps registers of the monitoring IC onto cell-numbers. The
     * register values are defined in the #MXM_REG_NAME_e enum.
     * For now the length of this array is #MXM_VOLTAGE_READ_ARRAY_LENGTH
     * as it is enabled for the measurement of all cells, two AUX-voltages and
     * one block voltage. This has to be adapted once this driver is enabled for
     * general operation.
     */
    const MXM_REG_NAME_e mxm_voltageCellAddresses[MXM_VOLTAGE_READ_ARRAY_LENGTH] = {
        MXM_REG_CELL1,
        MXM_REG_CELL2,
        MXM_REG_CELL3,
        MXM_REG_CELL4,
        MXM_REG_CELL5,
        MXM_REG_CELL6,
        MXM_REG_CELL7,
        MXM_REG_CELL8,
        MXM_REG_CELL9,
        MXM_REG_CELL10,
        MXM_REG_CELL11,
        MXM_REG_CELL12,
        MXM_REG_CELL13,
        MXM_REG_CELL14,
        MXM_REG_AUX2,
        MXM_REG_AUX3,
        MXM_REG_BLOCK,
    };

    pState->operationRequested = false;
    /* TODO handle transition to measurement states properly with dedicated state-machine */
    /* TODO parse DATACHECKBYTE where available */

    MXM_MONITORING_STATE_e temp_mon_state = MXM_MONITORING_STATE_PENDING;

    switch (pState->operationSubstate) {
        case MXM_OP_ENTRY_STATE:
            pState->operationSubstate = MXM_OP_DIAGNOSTIC_ENTRY;
            break;
        case MXM_OP_DIAGNOSTIC_ENTRY:
            if (pState->diagnosticCounter >= MXM_THRESHOLD_DIAGNOSTIC_AFTER_CYCLES) {
                pState->diagnosticCounter = 0u;
                pState->operationSubstate = MXM_OP_DIAGNOSTIC_STATUS1;
            } else {
                pState->diagnosticCounter++;
                pState->operationSubstate = MXM_OP_DIAGNOSTIC_EXIT;
            }
            break;
        case MXM_OP_DIAGNOSTIC_STATUS1:
            if (true == MXM_HandleStateReadall(pState, MXM_REG_STATUS1, MXM_OP_DIAGNOSTIC_STATUS2)) {
                const uint8_t temp_len = (uint8_t)(
                    (BATTERY_MANAGEMENT_TX_LENGTH_READALL + (2uL * MXM_5XGetNumberOfSatellites(pState->pInstance5X))) &
                    (uint8_t)UINT8_MAX);
                MXM_MonRegistryParseStatusFmeaIntoDevices(pState, temp_len);
                const bool someDeviceHasBeenReset = MXM_CheckIfADeviceHasBeenReset(pState);
                if (someDeviceHasBeenReset == true) {
                    /* a device has been reset, we should immediately reset the daisy chain by restarting the driver */
                    MXM_ErrorHandlerReset(pState, true);
                }
            }
            break;
        case MXM_OP_DIAGNOSTIC_STATUS2:
            if (true == MXM_HandleStateReadall(pState, MXM_REG_STATUS2, MXM_OP_DIAGNOSTIC_STATUS3)) {
                const uint8_t temp_len = (uint8_t)(
                    (BATTERY_MANAGEMENT_TX_LENGTH_READALL + (2uL * MXM_5XGetNumberOfSatellites(pState->pInstance5X))) &
                    (uint8_t)UINT8_MAX);
                MXM_MonRegistryParseStatusFmeaIntoDevices(pState, temp_len);
            }
            break;
        case MXM_OP_DIAGNOSTIC_STATUS3:
            if (true == MXM_HandleStateReadall(pState, MXM_REG_STATUS3, MXM_OP_DIAGNOSTIC_FMEA1)) {
                const uint8_t temp_len = (uint8_t)(
                    (BATTERY_MANAGEMENT_TX_LENGTH_READALL + (2uL * MXM_5XGetNumberOfSatellites(pState->pInstance5X))) &
                    (uint8_t)UINT8_MAX);
                MXM_MonRegistryParseStatusFmeaIntoDevices(pState, temp_len);
            }
            break;
        case MXM_OP_DIAGNOSTIC_FMEA1:
            if (true == MXM_HandleStateReadall(pState, MXM_REG_FMEA1, MXM_OP_DIAGNOSTIC_FMEA2)) {
                const uint8_t temp_len = (uint8_t)(
                    (BATTERY_MANAGEMENT_TX_LENGTH_READALL + (2uL * MXM_5XGetNumberOfSatellites(pState->pInstance5X))) &
                    (uint8_t)UINT8_MAX);
                MXM_MonRegistryParseStatusFmeaIntoDevices(pState, temp_len);
            }
            break;
        case MXM_OP_DIAGNOSTIC_FMEA2:
            if (true == MXM_HandleStateReadall(pState, MXM_REG_FMEA2, MXM_OP_DIAGNOSTIC_CLEAR_STATUS2)) {
                const uint8_t temp_len = (uint8_t)(
                    (BATTERY_MANAGEMENT_TX_LENGTH_READALL + (2uL * MXM_5XGetNumberOfSatellites(pState->pInstance5X))) &
                    (uint8_t)UINT8_MAX);
                MXM_MonRegistryParseStatusFmeaIntoDevices(pState, temp_len);
            }
            break;
        case MXM_OP_DIAGNOSTIC_CLEAR_STATUS2:
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_STATUS2;
                pState->batteryCmdBuffer.lsb        = 0x00u;
                pState->batteryCmdBuffer.msb        = 0x00u;
            }
            MXM_HandleStateWriteall(pState, MXM_OP_DIAGNOSTIC_CLEAR_FMEA1);
            break;
        case MXM_OP_DIAGNOSTIC_CLEAR_FMEA1:
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_FMEA1;
                pState->batteryCmdBuffer.lsb        = 0x00u;
                pState->batteryCmdBuffer.msb        = 0x00u;
            }
            MXM_HandleStateWriteall(pState, MXM_OP_DIAGNOSTIC_CLEAR_FMEA2);
            break;
        case MXM_OP_DIAGNOSTIC_CLEAR_FMEA2:
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_FMEA2;
                pState->batteryCmdBuffer.lsb        = 0x00u;
                pState->batteryCmdBuffer.msb        = 0x00u;
            }
            MXM_HandleStateWriteall(pState, MXM_OP_DIAGNOSTIC_EXIT);
            break;
        case MXM_OP_DIAGNOSTIC_EXIT:
            pState->operationSubstate = MXM_OP_SELECT_MUX_CHANNEL;
            break;
        case MXM_OP_SELECT_MUX_CHANNEL:
            /* configure I2CPNTR to channel number corresponding to mux counter*/
            pState->batteryCmdBuffer.regAddress = MXM_REG_I2CPNTR;
            pState->batteryCmdBuffer.lsb        = (0x01u << pState->muxCounter);
            pState->batteryCmdBuffer.msb        = 0x00u;
            MXM_HandleStateWriteall(pState, MXM_OP_WRITE_MUX0);
            break;
        case MXM_OP_WRITE_MUX0:
            /* send configuration to MUX0 for channel 0 (PNTR configured to 1u) */
            pState->batteryCmdBuffer.regAddress = MXM_REG_I2CSEND;
            pState->batteryCmdBuffer.lsb        = (MXM_I2C_MUX0_ADDRESS << 1u);
            pState->batteryCmdBuffer.msb        = 0x40u;
            MXM_HandleStateWriteall(pState, MXM_OP_WRITE_MUX1);
            break;
        case MXM_OP_WRITE_MUX1:
            /* send configuration to MUX1 for channel 0 (PNTR configured to 1u) */
            pState->batteryCmdBuffer.regAddress = MXM_REG_I2CSEND;
            pState->batteryCmdBuffer.lsb        = (MXM_I2C_MUX1_ADDRESS << 1u);
            pState->batteryCmdBuffer.msb        = 0x40u;
            MXM_HandleStateWriteall(pState, MXM_OP_SET_SCAN_STROBE);
            break;
        case MXM_OP_SET_SCAN_STROBE:
            pState->batteryCmdBuffer.regAddress = MXM_REG_SCANCTRL;
            /* set SCANSTROBE, enable 4x OVERSAMPL */
            pState->batteryCmdBuffer.lsb = 0x09u;
            /* enable AUTOBALSWDIS */
            pState->batteryCmdBuffer.msb = 0x10u;
            MXM_HandleStateWriteall(pState, MXM_OP_GET_SCAN_STROBE);
            break;
        case MXM_OP_GET_SCAN_STROBE:
            if (true == MXM_HandleStateReadall(pState, MXM_REG_SCANCTRL, MXM_OP_GET_VOLTAGES)) {
                /* no additional handling needed */
            }
            break;
        case MXM_OP_GET_VOLTAGES:
            FAS_ASSERT(pState->mxmVoltageCellCounter < MXM_VOLTAGE_READ_ARRAY_LENGTH);
            temp_mon_state = MXM_MonGetVoltages(pState, mxm_voltageCellAddresses[pState->mxmVoltageCellCounter]);

            if (temp_mon_state == MXM_MONITORING_STATE_PASS) {
                if (pState->mxmVoltageCellCounter < (uint8_t)UINT8_MAX) {
                    pState->mxmVoltageCellCounter++;
                }
                static_assert(
                    MXM_VOLTAGE_READ_ARRAY_LENGTH <= (uint8_t)UINT8_MAX,
                    "invalid define MXM_VOLTAGE_READ_ARRAY_LENGTH");
                /* modified: read one additional aux entry */
                if (pState->mxmVoltageCellCounter >= MXM_VOLTAGE_READ_ARRAY_LENGTH) {
                    pState->mxmVoltageCellCounter = 0;
                    pState->operationSubstate     = MXM_OP_GET_ALRTSUM;
                }
            } else {
                /* MXM_MONITORING_STATE_PENDING, do nothing */
            }
            break;
        case MXM_OP_GET_ALRTSUM:
            if (true == MXM_HandleStateReadall(pState, MXM_REG_ALRTSUM, MXM_OP_PARSE_VOLTAGES_INTO_DB)) {
                /* no additional handling needed */
            }
            break;
        case MXM_OP_PARSE_VOLTAGES_INTO_DB:
            if (MXM_ParseVoltagesIntoDB(pState) == STD_OK) {
                pState->operationSubstate    = MXM_OP_PINOPEN_ENTRY;
                pState->firstMeasurementDone = true;
            } else {
                MXM_ErrorHandlerReset(pState, false);
            }
            if (pState->stopRequested == true) {
                pState->state = MXM_STATEMACHINE_STATES_IDLE;
            } else {
                /* do nothing */
            }
            break;
        case MXM_OP_PINOPEN_ENTRY:
            if (pState->openwireRequested == true) {
                pState->operationSubstate = MXM_OP_PINOPEN_SET_CURRENT_SOURCES;
                pState->openwireRequested = false;
            } else {
                pState->operationSubstate = MXM_OP_PINOPEN_EXIT;
            }
            break;
        case MXM_OP_PINOPEN_EXIT:
            pState->operationSubstate = MXM_OP_BAL_ENTRY;
            break;
        case MXM_OP_PINOPEN_SET_CURRENT_SOURCES:
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_CTSTCFG;
                pState->batteryCmdBuffer.lsb        = 0xFFu;
                pState->batteryCmdBuffer.msb        = 0xFFu; /* execute diagnostic on every cell */
            }
            MXM_HandleStateWriteall(pState, MXM_OP_PINOPEN_COMPSCAN);
            break;
        case MXM_OP_PINOPEN_COMPSCAN:
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_SCANCTRL;
                pState->batteryCmdBuffer.lsb        = 0x81u;
                pState->batteryCmdBuffer.msb        = 0x00u; /* request comp scan */
            }
            MXM_HandleStateWriteall(pState, MXM_OP_PINOPEN_GET_SCAN_STROBE);
            break;
        case MXM_OP_PINOPEN_GET_SCAN_STROBE:
            if (true == MXM_HandleStateReadall(pState, MXM_REG_SCANCTRL, MXM_OP_PINOPEN_GET_ALRT)) {
                /* no additional handling needed */
            }
            break;
        case MXM_OP_PINOPEN_GET_ALRT:
            if (true == MXM_HandleStateReadall(pState, MXM_REG_ALRTCOMPUV, MXM_OP_PINOPEN_PROCESS_OPENWIRE)) {
                /* no additional handling needed */
            }
            break;
        case MXM_OP_PINOPEN_PROCESS_OPENWIRE:
            if (MXM_ProcessOpenWire(pState) == STD_OK) {
                pState->operationSubstate = MXM_OP_PINOPEN_RESTORE_CURRENT_SOURCE_CONF;
            } else {
                MXM_ErrorHandlerReset(pState, false);
            }
            break;
        case MXM_OP_PINOPEN_RESTORE_CURRENT_SOURCE_CONF:
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_CTSTCFG;
                pState->batteryCmdBuffer.lsb        = 0x00u;
                pState->batteryCmdBuffer.msb        = 0x00u;
            }
            MXM_HandleStateWriteall(pState, MXM_OP_PINOPEN_EXIT);
            break;
        case MXM_OP_BAL_ENTRY:
            /* Get the current time */
            pState->pBalancingState->currentTimeStamp = OS_GetTickCount();

            /* Wait 'MXM_DELAY_BALANCING' milliseconds before processing the balancing */
            if (OS_CheckTimeHasPassedWithTimestamp(
                    pState->pBalancingState->previousTimeStamp,
                    pState->pBalancingState->currentTimeStamp,
                    MXM_DELAY_BALANCING)) {
                /* nothing to do, exit balancing state chain */
                pState->operationSubstate = MXM_OP_BAL_EXIT;
            } else {
                /* Balancing needs to be processed */
                pState->pBalancingState->previousTimeStamp = pState->pBalancingState->currentTimeStamp;
                /* First reset the balancing switches, and THEN
                   set the balancing switches according to the database */
                pState->operationSubstate = MXM_OP_BAL_CONTROL_RESET_ALL;

                /* Change the parity of cells to balance */
                if (pState->pBalancingState->evenCellsBalancingProcessed == true) {
                    pState->pBalancingState->evenCellsNeedBalancing = false;
                    pState->pBalancingState->oddCellsNeedBalancing  = true;
                }
                /* Same for odd cells */
                if (pState->pBalancingState->oddCellsBalancingProcessed == true) {
                    pState->pBalancingState->evenCellsNeedBalancing = true;
                    pState->pBalancingState->oddCellsNeedBalancing  = false;
                }
            }
            break;
        case MXM_OP_BAL_CONTROL_RESET_ALL:
            /* Send a WRITEALL command to open all balancing switches */
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_BALSWCTRL;
                /* CBRESTART is not reset to 0, not needed. */
                pState->batteryCmdBuffer.lsb = 0x00U;
                pState->batteryCmdBuffer.msb = 0x00U;
            }
            MXM_HandleStateWriteall(pState, MXM_OP_BAL_CONTROL_SET_ALL);
            break;
        case MXM_OP_BAL_CONTROL_SET_ALL:
            /* Send a WRITEDEVICE command to each module in a daisy-chain
               to close appropriate balancing switches */
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->pBalancingState->evenCellsBalancingProcessed = false;
                pState->pBalancingState->oddCellsBalancingProcessed  = false;

                STD_RETURN_TYPE_e database_retval = DATA_READ_DATA(pState->pBalancingState->pBalancingControl_table);
                if (STD_OK == database_retval) {
                    /* Construct the balancing buffer */
                    const STD_RETURN_TYPE_e retval = MXM_ConstructBalancingBuffer(pState->pBalancingState);

                    if (retval == STD_OK) {
                        if (pState->pBalancingState->cellsToBalance > 0u) {
                            /* Some cells need to be balanced */
                            pState->batteryCmdBuffer.deviceAddress = pState->pBalancingState->moduleBalancingIndex;
                            pState->batteryCmdBuffer.regAddress    = MXM_REG_BALSWCTRL;
                            pState->batteryCmdBuffer.lsb =
                                (uint8_t)(pState->pBalancingState->cellsToBalance & MXM_BM_LSB);
                            pState->batteryCmdBuffer.msb =
                                (uint8_t)(pState->pBalancingState->cellsToBalance >> MXM_CELLS_IN_LSB);

                            const STD_RETURN_TYPE_e setStateRequestReturn = MXM_5XSetStateRequest(
                                pState->pInstance5X,
                                MXM_STATEMACH_5X_WRITE_DEVICE,
                                pState->batteryCmdBuffer,
                                &pState->requestStatus5x);
                            FAS_ASSERT(setStateRequestReturn == STD_OK);
                        } else {
                            /* It is not necessary to re-send 0 to the device, because it has been done previously
                           in the BALANCING_CONTROL_RESET_ALL sub-state */
                            pState->requestStatus5x = MXM_5X_STATE_PROCESSED;
                        }
                    } else {
                        /* this should not happen if the software works as expected */
                        FAS_ASSERT(FAS_TRAP);
                        pState->requestStatus5x = MXM_5X_STATE_PROCESSED;
                    }
                } else {
                    /* database read not successful, retry */
                    pState->requestStatus5x = MXM_5X_STATE_UNSENT;
                }
            } else if (pState->requestStatus5x == MXM_5X_STATE_UNPROCESSED) {
                /* wait for processing */
            } else if (pState->requestStatus5x == MXM_5X_STATE_PROCESSED) {
                if (pState->pBalancingState->moduleBalancingIndex < pState->highest5xDevice) {
                    /* Not all modules have been treated. Repeat this state with the next module */
                    pState->pBalancingState->moduleBalancingIndex++;
                    pState->operationSubstate = MXM_OP_BAL_CONTROL_SET_ALL;
                    /* Re-set the status to UNSENT to repeat the WRITE operation */
                    pState->requestStatus5x = MXM_5X_STATE_UNSENT;
                } else {
                    /* Finished the iteration of all modules in a daisy chain, continue */
                    pState->pBalancingState->moduleBalancingIndex = 0;

                    /* Update flags for the end of balancing */
                    if (pState->pBalancingState->evenCellsNeedBalancing == true) {
                        pState->pBalancingState->evenCellsBalancingProcessed = true;
                    } else if (pState->pBalancingState->oddCellsNeedBalancing == true) {
                        pState->pBalancingState->oddCellsBalancingProcessed = true;
                    } else {
                        /* nothing to do here */
                    }
                    pState->requestStatus5x   = MXM_5X_STATE_UNSENT;
                    pState->operationSubstate = MXM_OP_BAL_EXIT;
                }
            } else if (pState->requestStatus5x == MXM_5X_STATE_ERROR) {
                /* default-behavior: retry */
                pState->requestStatus5x = MXM_5X_STATE_UNSENT;
                MXM_ErrorHandlerReset(pState, false);
            } else {
                FAS_ASSERT(FAS_TRAP);
            }
            break;
        case MXM_OP_BAL_START:
            /* Initiate balancing for all devices in a daisy chain */
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_BALCTRL;
                /*  Manual ON MODE + Balancing halt in case of High temperature
                    + Alert when cell balancing is finished */
                pState->batteryCmdBuffer.lsb = 0x0EU;
                pState->batteryCmdBuffer.msb = 0x18U;
            }
            MXM_HandleStateWriteall(pState, MXM_OP_BAL_EXIT);
            break;
        case MXM_OP_BAL_EXIT:
            pState->operationSubstate = MXM_OP_CYCLE_END_ENTRY;
            break;
        case MXM_OP_CYCLE_END_ENTRY:
            /* actions that should be taken at the end of a measurement cycle */
            pState->operationSubstate = MXM_OP_INCREMENT_MUX_COUNTER;
            break;
        case MXM_OP_INCREMENT_MUX_COUNTER:
            if (pState->muxCounter < (8u - 1u)) {
                pState->muxCounter++;
            } else {
                pState->muxCounter = 0u;
            }
            pState->operationSubstate = MXM_OP_CYCLE_END_EXIT;
            break;
        case MXM_OP_CYCLE_END_EXIT:
            pState->operationSubstate = MXM_OP_ENTRY_STATE;
            break;
        /* "initialization" from here on */
        case MXM_INIT_ENTRY:
            pState->operationSubstate = MXM_INIT_DEVCFG1;
            break;
        case MXM_INIT_DEVCFG1:
            /* switch to single UART with external loopback */
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_DEVCFG1;
                pState->batteryCmdBuffer.lsb        = 0x00u; /* alert interface disabled */
                pState->batteryCmdBuffer.msb        = 0x01u; /* single uart with external loopback*/
            }
            MXM_HandleStateWriteall(pState, MXM_INIT_SET_STATUS2);
            break;
        case MXM_INIT_SET_STATUS2:
            /* clear ALRTDUALUART (as requested for startup routine in data-sheet) */
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_STATUS2;
                pState->batteryCmdBuffer.lsb        = 0x00u; /* clear lsb */
                pState->batteryCmdBuffer.msb        = 0x00u; /* clear msb, containing ALRTDUALUART */
            }
            MXM_HandleStateWriteall(pState, MXM_INIT_STATUS1);
            break;
        case MXM_INIT_STATUS1:
            /* clear ALRTRST */
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_STATUS1;
                pState->batteryCmdBuffer.lsb        = 0x00u;
                pState->batteryCmdBuffer.msb        = 0x00u;
            }
            MXM_HandleStateWriteall(pState, MXM_INIT_GET_VERSION);
            break;
        case MXM_INIT_GET_VERSION:
            /* add version information to registry */
            if (true == MXM_HandleStateReadall(pState, MXM_REG_VERSION, MXM_INIT_GET_ID1)) {
                uint8_t temp_len = (uint8_t)(
                    (BATTERY_MANAGEMENT_TX_LENGTH_READALL + (2uL * MXM_5XGetNumberOfSatellites(pState->pInstance5X))) &
                    (uint8_t)UINT8_MAX);
                MXM_MonRegistryParseVersionIntoDevices(pState, temp_len);
            }
            break;
        case MXM_INIT_GET_ID1:
            /* add ID1 to registry */
            if (true == MXM_HandleStateReadall(pState, MXM_REG_ID1, MXM_INIT_GET_ID2)) {
                uint8_t temp_len = (uint8_t)(
                    (BATTERY_MANAGEMENT_TX_LENGTH_READALL + (2uL * MXM_5XGetNumberOfSatellites(pState->pInstance5X))) &
                    (uint8_t)UINT8_MAX);
                MXM_MonRegistryParseIdIntoDevices(pState, temp_len, MXM_REG_ID1);
            }
            break;
        case MXM_INIT_GET_ID2:
            /* add ID2 to registry */
            if (true == MXM_HandleStateReadall(pState, MXM_REG_ID2, MXM_INIT_MEASUREEN1)) {
                uint8_t temp_len = (uint8_t)(
                    (BATTERY_MANAGEMENT_TX_LENGTH_READALL + (2uL * MXM_5XGetNumberOfSatellites(pState->pInstance5X))) &
                    (uint8_t)UINT8_MAX);
                MXM_MonRegistryParseIdIntoDevices(pState, temp_len, MXM_REG_ID2);
            }
            break;
        case MXM_INIT_MEASUREEN1:
            /* enable all 14 cells */
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_MEASUREEN1;
                pState->batteryCmdBuffer.lsb        = 0xFFu;
                pState->batteryCmdBuffer.msb        = 0x7Fu; /* TODO this also enables block measurement */
            }
            MXM_HandleStateWriteall(pState, MXM_INIT_MEASUREEN2);
            break;
        case MXM_INIT_MEASUREEN2:
            /* enable AUX2 and AUX3 */
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_MEASUREEN2;
                pState->batteryCmdBuffer.lsb        = 0x0Cu; /* AUX2 and AUX3 */
                pState->batteryCmdBuffer.msb        = 0x00u;
            }
            MXM_HandleStateWriteall(pState, MXM_INIT_AUXGPIOCFG);
            break;
        case MXM_INIT_AUXGPIOCFG:
            /* switch GPIO2 and GPIO3 to AUX, enable I2C */
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_AUXGPIOCFG;
                pState->batteryCmdBuffer.lsb        = 0x00u;
                /* conf for MAX17853: 0x3Eu */
                /* conf for MAX17852, I2C enable: 0x8Eu */
                /* I2C enable, AUX2 and AUX3 */
                pState->batteryCmdBuffer.msb = 0x80u;
            }
            MXM_HandleStateWriteall(pState, MXM_INIT_AUXTIMEREG);
            break;
        case MXM_INIT_AUXTIMEREG:
            /* configure settling time that NTC network takes for measurement to 500us */
            /* WARNING: reevaluate this value if thermistor supply is switched
              during sampling */
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_AUXTIME;
                pState->batteryCmdBuffer.lsb        = 0x53u;
                pState->batteryCmdBuffer.msb        = 0x00u;
            }
            MXM_HandleStateWriteall(pState, MXM_INIT_ACQCFG);
            break;
        case MXM_INIT_ACQCFG:
            /* set ACQCFG */
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_ACQCFG;
                /* default values */
                pState->batteryCmdBuffer.lsb = 0x00u;
                /* we have to turn thermistor switch manually on, as charging
                   the network takes to long */
                pState->batteryCmdBuffer.msb = 0x06u;
            }
            MXM_HandleStateWriteall(pState, MXM_INIT_UVTHSETREG);
            break;
        case MXM_INIT_UVTHSETREG:
            /* configure UVTHSETREG */
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_UVTHSET;
                MXM_Unipolar14BitInto16Bit(
                    MXM_VoltageIntoUnipolar14Bit(1700u, MXM_REF_UNIPOLAR_CELL_mV),
                    &pState->batteryCmdBuffer.lsb,
                    &pState->batteryCmdBuffer.msb);
            }
            MXM_HandleStateWriteall(pState, MXM_INIT_OVTHSETREG);
            break;
        case MXM_INIT_OVTHSETREG:
            /* configure OVTHSETREG */
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_OVTHSET;
                MXM_Unipolar14BitInto16Bit(
                    MXM_VoltageIntoUnipolar14Bit(3300u, MXM_REF_UNIPOLAR_CELL_mV),
                    &pState->batteryCmdBuffer.lsb,
                    &pState->batteryCmdBuffer.msb);
            }
            MXM_HandleStateWriteall(pState, MXM_INIT_BALEXP1);
            break;
        case MXM_INIT_BALEXP1:
            /* set BALEXP1 to have 1 minute timeout in manual balancing */
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_BALEXP1;
                pState->batteryCmdBuffer.lsb        = 0x01u;
                pState->batteryCmdBuffer.msb        = 0x00u;
            }
            MXM_HandleStateWriteall(pState, MXM_INIT_BALEXP2);
            break;
        case MXM_INIT_BALEXP2:
            /* set BALEXP2 to have 1 minute timeout in manual balancing */
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_BALEXP2;
                pState->batteryCmdBuffer.lsb        = 0x01u;
                pState->batteryCmdBuffer.msb        = 0x00u;
            }
            MXM_HandleStateWriteall(pState, MXM_INIT_BALEXP3);
            break;
        case MXM_INIT_BALEXP3:
            /* set BALEXP3 to have 1 minute timeout in manual balancing */
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_BALEXP3;
                pState->batteryCmdBuffer.lsb        = 0x01u;
                pState->batteryCmdBuffer.msb        = 0x00u;
            }
            MXM_HandleStateWriteall(pState, MXM_INIT_BALEXP4);
            break;
        case MXM_INIT_BALEXP4:
            /* set BALEXP4 to have 1 minute timeout in manual balancing */
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_BALEXP4;
                pState->batteryCmdBuffer.lsb        = 0x01u;
                pState->batteryCmdBuffer.msb        = 0x00u;
            }
            MXM_HandleStateWriteall(pState, MXM_INIT_BALEXP5);
            break;
        case MXM_INIT_BALEXP5:
            /* set BALEXP5 to have 1 minute timeout in manual balancing */
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_BALEXP5;
                pState->batteryCmdBuffer.lsb        = 0x01u;
                pState->batteryCmdBuffer.msb        = 0x00u;
            }
            MXM_HandleStateWriteall(pState, MXM_INIT_BALEXP6);
            break;
        case MXM_INIT_BALEXP6:
            /* set BALEXP6 to have 1 minute timeout in manual balancing */
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_BALEXP6;
                pState->batteryCmdBuffer.lsb        = 0x01u;
                pState->batteryCmdBuffer.msb        = 0x00u;
            }
            MXM_HandleStateWriteall(pState, MXM_INIT_BALEXP7);
            break;
        case MXM_INIT_BALEXP7:
            /* set BALEXP7 to have 1 minute timeout in manual balancing */
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_BALEXP7;
                pState->batteryCmdBuffer.lsb        = 0x01u;
                pState->batteryCmdBuffer.msb        = 0x00u;
            }
            MXM_HandleStateWriteall(pState, MXM_INIT_BALEXP8);
            break;
        case MXM_INIT_BALEXP8:
            /* set BALEXP8 to have 1 minute timeout in manual balancing */
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_BALEXP8;
                pState->batteryCmdBuffer.lsb        = 0x01u;
                pState->batteryCmdBuffer.msb        = 0x00u;
            }
            MXM_HandleStateWriteall(pState, MXM_INIT_BALEXP9);
            break;
        case MXM_INIT_BALEXP9:
            /* set BALEXP9 to have 1 minute timeout in manual balancing */
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_BALEXP9;
                pState->batteryCmdBuffer.lsb        = 0x01u;
                pState->batteryCmdBuffer.msb        = 0x00u;
            }
            MXM_HandleStateWriteall(pState, MXM_INIT_BALEXP10);
            break;
        case MXM_INIT_BALEXP10:
            /* set BALEXP10 to have 1 minute timeout in manual balancing */
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_BALEXP10;
                pState->batteryCmdBuffer.lsb        = 0x01u;
                pState->batteryCmdBuffer.msb        = 0x00u;
            }
            MXM_HandleStateWriteall(pState, MXM_INIT_BALEXP11);
            break;
        case MXM_INIT_BALEXP11:
            /* set BALEXP11 to have 1 minute timeout in manual balancing */
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_BALEXP11;
                pState->batteryCmdBuffer.lsb        = 0x01u;
                pState->batteryCmdBuffer.msb        = 0x00u;
            }
            MXM_HandleStateWriteall(pState, MXM_INIT_BALEXP12);
            break;
        case MXM_INIT_BALEXP12:
            /* set BALEXP12 to have 1 minute timeout in manual balancing */
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_BALEXP12;
                pState->batteryCmdBuffer.lsb        = 0x01u;
                pState->batteryCmdBuffer.msb        = 0x00u;
            }
            MXM_HandleStateWriteall(pState, MXM_INIT_BALEXP13);
            break;
        case MXM_INIT_BALEXP13:
            /* set BALEXP13 to have 1 minute timeout in manual balancing */
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_BALEXP13;
                pState->batteryCmdBuffer.lsb        = 0x01u;
                pState->batteryCmdBuffer.msb        = 0x00u;
            }
            MXM_HandleStateWriteall(pState, MXM_INIT_BALEXP14);
            break;
        case MXM_INIT_BALEXP14:
            /* set BALEXP14 to have 1 minute timeout in manual balancing */
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_BALEXP14;
                pState->batteryCmdBuffer.lsb        = 0x01u;
                pState->batteryCmdBuffer.msb        = 0x00u;
            }
            MXM_HandleStateWriteall(pState, MXM_INIT_BALSWDLY);
            break;
        case MXM_INIT_BALSWDLY:
            /* set BALSWDLY to 2 ms settling time after balancing */
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_BALSWDLY;
                /* CELLDLY = 1920us (~2ms) --> 20*96us
            CELLDLY corresponds to the time to relax the cell before voltage measurement */
                pState->batteryCmdBuffer.lsb = 0x00u;
                pState->batteryCmdBuffer.msb = 0x14u;
            }
            MXM_HandleStateWriteall(pState, MXM_INIT_ALRTOVEN);
            break;
        case MXM_INIT_ALRTOVEN:
            /* enable ALRTOVEN */
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_ALRTOVEN;
                pState->batteryCmdBuffer.lsb        = 0xFFu;
                pState->batteryCmdBuffer.msb        = 0x3Fu;
            }
            MXM_HandleStateWriteall(pState, MXM_INIT_ALRTUVEN);
            break;
        case MXM_INIT_ALRTUVEN:
            /* enable ALRTUVEN */
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_ALRTUVEN;
                pState->batteryCmdBuffer.lsb        = 0xFFu;
                pState->batteryCmdBuffer.msb        = 0x3Fu;
            }
            MXM_HandleStateWriteall(pState, MXM_INIT_COMPOPNTHREG);
            break;
        case MXM_INIT_COMPOPNTHREG:
            /* configure COMPOPNTHREG */
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_COMPOPNTH;
                /* TODO 0.5V */
                MXM_Unipolar14BitInto16Bit(
                    MXM_VoltageIntoUnipolar14Bit(500u, MXM_REF_UNIPOLAR_CELL_mV),
                    &pState->batteryCmdBuffer.lsb,
                    &pState->batteryCmdBuffer.msb);
            }
            MXM_HandleStateWriteall(pState, MXM_INIT_I2C_GET_STAT1);
            break;
        case MXM_INIT_I2C_GET_STAT1:
            if (true == MXM_HandleStateReadall(pState, MXM_REG_I2CSTAT, MXM_INIT_I2C_CFG)) {
                /* no additional handling needed */
            }
            break;
        case MXM_INIT_I2C_CFG:
            /* configure I2CCFG to
               * 400kHz
               * Alternate write Mode (just a pointer without data)
               * Combined Format
               * 7 Bit addressing
               * one byte pointer length
               * default
               */
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_I2CCFG;
                pState->batteryCmdBuffer.lsb        = 0x00u;
                pState->batteryCmdBuffer.msb        = 0xE0u;
            }
            MXM_HandleStateWriteall(pState, MXM_INIT_I2C_PNTR);
            break;
        case MXM_INIT_I2C_PNTR:
            /* configure I2CPNTR */
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_I2CPNTR;
                pState->batteryCmdBuffer.lsb        = 0x01u;
                pState->batteryCmdBuffer.msb        = 0x00u;
            }
            MXM_HandleStateWriteall(pState, MXM_INIT_I2C_SEND_MUX0);
            break;
        case MXM_INIT_I2C_SEND_MUX0:
            /* send configuration to MUX0 for channel 0 (PNTR configured to 1u) */
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_I2CSEND;
                pState->batteryCmdBuffer.lsb        = (MXM_I2C_MUX0_ADDRESS << 1u);
                pState->batteryCmdBuffer.msb        = 0x40u;
            }
            MXM_HandleStateWriteall(pState, MXM_INIT_I2C_SEND_MUX1);
            break;
        case MXM_INIT_I2C_SEND_MUX1:
            /* send configuration to MUX1 for channel 0 (PNTR configured to 1u) */
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_I2CSEND;
                pState->batteryCmdBuffer.lsb        = (MXM_I2C_MUX1_ADDRESS << 1u);
                pState->batteryCmdBuffer.msb        = 0x40u;
            }
            MXM_HandleStateWriteall(pState, MXM_INIT_GET_I2C_STAT2);
            break;
        case MXM_INIT_GET_I2C_STAT2:
            if (true == MXM_HandleStateReadall(pState, MXM_REG_I2CSTAT, MXM_OP_ENTRY_STATE)) {
                /* no additional handling needed */
            }
            break;
        default:
            /* invalid state */
            FAS_ASSERT(FAS_TRAP);
            break;
    }
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST

#endif
