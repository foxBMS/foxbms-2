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
 * @file    nxp_mc33775a.c
 * @author  foxBMS Team
 * @date    2020-05-08 (date of creation)
 * @updated 2025-03-31 (date of last update)
 * @version v1.9.0
 * @ingroup DRIVERS
 * @prefix  N775
 *
 * @brief   Driver for the MC33775A analog front-end.
 * @details TODO
 */

/*========== Includes =======================================================*/
#include "nxp_mc33775a.h"
/* clang-format off */
#include "nxp_mc33775a-ll.h"
/* clang-format on */
#include "nxp_mc33775a_balancing.h"
#include "nxp_mc33775a_database.h"
#include "nxp_mc33775a_i2c.h"
#include "nxp_mc33775a_mux.h"

#pragma diag_push
#pragma diag_suppress 232
#include "MC33775A.h"
#pragma diag_pop

#include "afe.h"
#include "afe_dma.h"
#include "database.h"
#include "fassert.h"
#include "foxmath.h"
#include "fstd_types.h"
#include "ftask.h"
#include "mcu.h"
#include "os.h"
#include "spi.h"

#include <math.h>
#include <stdbool.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/
/** local copies of database tables */
/**@{*/
static DATA_BLOCK_CELL_VOLTAGE_s n775_cellVoltage           = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE_BASE};
static DATA_BLOCK_CELL_TEMPERATURE_s n775_cellTemperature   = {.header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE_BASE};
static DATA_BLOCK_MIN_MAX_s n775_minMax                     = {.header.uniqueId = DATA_BLOCK_ID_MIN_MAX};
static DATA_BLOCK_BALANCING_CONTROL_s n775_balancingControl = {.header.uniqueId = DATA_BLOCK_ID_BALANCING_CONTROL};
static DATA_BLOCK_ALL_GPIO_VOLTAGES_s n775_allGpioVoltage   = {.header.uniqueId = DATA_BLOCK_ID_ALL_GPIO_VOLTAGES_BASE};
static DATA_BLOCK_BALANCING_FEEDBACK_s n775_balancingFeedback = {
    .header.uniqueId = DATA_BLOCK_ID_BALANCING_FEEDBACK_BASE};
static DATA_BLOCK_SLAVE_CONTROL_s n775_slaveControl = {.header.uniqueId = DATA_BLOCK_ID_SLAVE_CONTROL};
static DATA_BLOCK_OPEN_WIRE_s n775_openWire         = {.header.uniqueId = DATA_BLOCK_ID_OPEN_WIRE_BASE};
/**@}*/
static N775_SUPPLY_CURRENT_s n775_supplyCurrent = {0};
static N775_ERROR_TABLE_s n775_errorTable       = {0};

/*========== Extern Constant and Variable Definitions =======================*/

N775_STATE_s n775_stateBase = {
    .firstMeasurementMade       = false,
    .currentString              = 0u,
    .pSpiTxSequenceStart        = NULL_PTR,
    .pSpiTxSequence             = NULL_PTR,
    .pSpiRxSequenceStart        = NULL_PTR,
    .pSpiRxSequence             = NULL_PTR,
    .currentMux                 = {0},
    .pMuxSequenceStart          = {0},
    .pMuxSequence               = {0},
    .n775Data.cellVoltage       = &n775_cellVoltage,
    .n775Data.cellTemperature   = &n775_cellTemperature,
    .n775Data.allGpioVoltage    = &n775_allGpioVoltage,
    .n775Data.minMax            = &n775_minMax,
    .n775Data.balancingFeedback = &n775_balancingFeedback,
    .n775Data.balancingControl  = &n775_balancingControl,
    .n775Data.slaveControl      = &n775_slaveControl,
    .n775Data.openWire          = &n775_openWire,
    .n775Data.supplyCurrent     = &n775_supplyCurrent,
    .n775Data.errorTable        = &n775_errorTable,
};

/*========== Static Function Prototypes =====================================*/
/**
 * @brief   captures the measurement.
 * @details The MC33775A measures continuously.
 *          This function takes a snapshot on all slaves in the daisy-chain.
 * @param   pState state of the NXP MC33775A driver
 */
static void N775_CaptureMeasurement(N775_STATE_s *pState);

/**
 * @brief   enumerates the N775 slaves.
 * @details This function gives the slaves in the daisy-chain an address.
 * @param   pState state of the NXP MC33775A driver
 */
static STD_RETURN_TYPE_e N775_Enumerate(N775_STATE_s *pState);
/**
 * @brief   handles error when doing measurements.
 * @details This function is used in the measurement function.
 *          It sets the errors flags in the error table according to the value
 *          returned by the communication function.
 * @param   pState      state of the NXP MC33775A driver
 * @param   returnedValue   status of the low-level communication
 * @param   module          number of module addressed
 */
static void N775_ErrorHandling(N775_STATE_s *pState, N775_COMMUNICATION_STATUS_e returnedValue, uint8_t module);

/**
 * @brief   increments index in string sequence.
 * @param   pState state of the NXP MC33775A driver
 */
static void N775_IncrementStringSequence(N775_STATE_s *pState);

/**
 * @brief   initializes the N775 driver.
 * @details This function enumerates the slaves and starts the measurement.
 * @param   pState state of the NXP MC33775A driver
 */
static void N775_Initialize(N775_STATE_s *pState);

/**
 * @brief   reset index in string sequence.
 * @param   pState state of the NXP MC33775A driver
 */
static void N775_ResetStringSequence(N775_STATE_s *pState);

/**
 * @brief   sets the measurement initialization status.
 * @param   pState state of the NXP MC33775A driver
 */
static void N775_SetFirstMeasurementCycleFinished(N775_STATE_s *pState);

/**
 * @brief   starts the measurement.
 * @details The MC33775A measures continuously.
 *          This function starts the measurement.
 * @param  pState  state of the NXP MC33775A driver
 */
static void N775_StartMeasurement(N775_STATE_s *pState);

/*========== Static Function Implementations ================================*/
static void N775_CaptureMeasurement(N775_STATE_s *pState) {
    FAS_ASSERT(pState != NULL_PTR);

    uint16_t primaryRawValues[20]                   = {0u};
    uint16_t secondaryRawValues[4]                  = {0u};
    uint16_t currentRawValue                        = 0u;
    int16_t primaryValues[20]                       = {0u};
    int16_t secondaryValues[4]                      = {0u};
    int16_t currentValue                            = 0u;
    N775_COMMUNICATION_STATUS_e retValPrimary       = N775_COMMUNICATION_OK;
    N775_COMMUNICATION_STATUS_e retValSecondary     = N775_COMMUNICATION_OK;
    N775_COMMUNICATION_STATUS_e retValSupplyCurrent = N775_COMMUNICATION_OK;
    uint16_t error                                  = 0u;
    bool gpio03Error                                = false;
    bool gpio47Error                                = false;

    /* Send capture command. This ends the last cycle and starts a new one */
    N775_CommunicationWrite(
        N775_BROADCAST_ADDRESS,
        MC33775_ALLM_APP_CTRL_OFFSET,
        (1u << MC33775_ALLM_APP_CTRL_PAUSEBAL_POS) | (0x3FFu << MC33775_PRMM_APP_CTRL_CAPVC_POS) |
            (MC33775_PRMM_APP_CTRL_VCOLNUM_DISABLED_ENUM_VAL << MC33775_PRMM_APP_CTRL_VCOLNUM_POS),
        pState->pSpiTxSequence);
    /* Wait for measurements to take place */
    N775_Wait(N775_MEASUREMENT_CAPTURE_TIME_MS);
    /* Send capture command. This ends the last cycle and starts a new one */
    N775_CommunicationWrite(
        N775_BROADCAST_ADDRESS,
        MC33775_ALLM_APP_CTRL_OFFSET,
        (0u << MC33775_ALLM_APP_CTRL_PAUSEBAL_POS) | (0x3FFu << MC33775_PRMM_APP_CTRL_CAPVC_POS) |
            (MC33775_PRMM_APP_CTRL_VCOLNUM_DISABLED_ENUM_VAL << MC33775_PRMM_APP_CTRL_VCOLNUM_POS),
        pState->pSpiTxSequence);
    /* Wait for measurements to be ready */
    N775_Wait(N775_MEASUREMENT_READY_TIME_MS);

    /* Reset number previous measurement results on string-level */
    pState->n775Data.cellVoltage->stringVoltage_mV[pState->currentString]        = 0u;
    pState->n775Data.cellVoltage->nrValidCellVoltages[pState->currentString]     = 0u;
    pState->n775Data.cellTemperature->nrValidTemperatures[pState->currentString] = 0u;

    for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
        uint8_t deviceAddress = m + 1u;
        retValPrimary         = N775_CommunicationReadMultiple(
            deviceAddress, 20, 4u, MC33775_PRMM_APP_VC_CNT_OFFSET, primaryRawValues, pState);
        retValSecondary = N775_CommunicationReadMultiple(
            deviceAddress, 4, 4u, MC33775_SECM_APP_AIN4_OFFSET, secondaryRawValues, pState);
        if (N775_CHECK_SUPPLY_CURRENT == true) {
            retValSupplyCurrent =
                N775_CommunicationRead(deviceAddress, MC33775_SECM_PER_NPNISENSE_OFFSET, &currentRawValue, pState);
        }

        N775_ErrorHandling(pState, retValPrimary, m);
        if (retValPrimary == N775_COMMUNICATION_OK) {
            for (uint8_t cb = 0u; cb < BS_NR_OF_CELL_BLOCKS_PER_MODULE; cb++) {
                /* Store cell voltages */
                if (N775_INVALID_REGISTER_VALUE != primaryRawValues[cb + 1u]) {
                    primaryValues[cb + 1u] = (int16_t)primaryRawValues[cb + 1u];
                    pState->n775Data.cellVoltage->cellVoltage_mV[pState->currentString][m][cb] =
                        (((float_t)primaryValues[cb + 1u]) * 154.0e-6f * UNIT_CONVERSION_FACTOR_1000_FLOAT);
                    /* String voltage measurement is calculated as sum of individual cell voltage measurement */
                    pState->n775Data.cellVoltage->stringVoltage_mV[pState->currentString] +=
                        pState->n775Data.cellVoltage->cellVoltage_mV[pState->currentString][m][cb];
                    pState->n775Data.cellVoltage->invalidCellVoltage[pState->currentString][m][cb] = false;
                    pState->n775Data.cellVoltage->nrValidCellVoltages[pState->currentString]++;
                } else {
                    error++;
                    pState->n775Data.cellVoltage->cellVoltage_mV[pState->currentString][m][cb] =
                        AFE_DEFAULT_CELL_VOLTAGE_INVALID_VALUE;
                    pState->n775Data.cellVoltage->invalidCellVoltage[pState->currentString][m][cb] = true;
                }
            }
            for (uint8_t g = 0u; g < 4u; g++) {
                /* Store GPIO voltages */
                if (N775_INVALID_REGISTER_VALUE != primaryRawValues[g + 16u]) {
                    primaryValues[g + 16u] = (int16_t)primaryRawValues[g + 16u];
                    pState->n775Data.allGpioVoltage
                        ->gpioVoltages_mV[pState->currentString][g + (m * SLV_NR_OF_GPIOS_PER_MODULE)] =
                        (((float_t)primaryValues[g + 16u]) * 154.0e-6f * 1000.0f);
                } else {
                    gpio03Error = true;
                    error++;
                }
            }
            /* Store module voltage */
            if (N775_INVALID_REGISTER_VALUE != primaryRawValues[15u]) {
                primaryValues[15u] = (int16_t)primaryRawValues[15u];
                pState->n775Data.cellVoltage->moduleVoltage_mV[pState->currentString][m] =
                    (((float_t)primaryValues[15u]) * 2.58e-3f * 1000.0f);
            } else {
                error++;
            }
        } else {
            /* Reset voltage values and the relevant invalid flag if the communication is not ok */
            for (uint8_t cb = 0u; cb < BS_NR_OF_CELL_BLOCKS_PER_MODULE; cb++) {
                pState->n775Data.cellVoltage->cellVoltage_mV[pState->currentString][m][cb]     = 0;
                pState->n775Data.cellVoltage->invalidCellVoltage[pState->currentString][m][cb] = true;
            }
        }

        N775_ErrorHandling(pState, retValSecondary, m);
        if (retValSecondary == N775_COMMUNICATION_OK) {
            for (uint8_t g = 4u; g < 8u; g++) {
                if (N775_INVALID_REGISTER_VALUE != secondaryRawValues[g - 4u]) {
                    secondaryValues[g - 4u] = (int16_t)secondaryRawValues[g - 4u];
                    pState->n775Data.allGpioVoltage
                        ->gpioVoltages_mV[pState->currentString][g + (m * SLV_NR_OF_GPIOS_PER_MODULE)] =
                        (((float_t)secondaryValues[g - 4u]) * 154.0e-6f * 1000.0f);
                } else {
                    gpio47Error = true;
                    error++;
                }
            }
        }

        N775_ErrorHandling(pState, retValPrimary, m);
        if (retValPrimary == N775_COMMUNICATION_OK) {
            /* Set temperature values */
            if (N775_USE_MUX_FOR_TEMP == true) {
                /* Mux case */
                if (gpio03Error == false) {
                    pState->n775Data.cellTemperature
                        ->cellTemperature_ddegC[pState->currentString][m][pState->currentMux[pState->currentString]] =
                        N775_ConvertVoltagesToTemperatures(
                            pState->n775Data.allGpioVoltage->gpioVoltages_mV[pState->currentString]
                                                                            [N775_MULTIPLEXER_TEMP_GPIO_POSITION +
                                                                             (m * SLV_NR_OF_GPIOS_PER_MODULE)]);
                    pState->n775Data.cellTemperature
                        ->invalidCellTemperature[pState->currentString][m][pState->currentMux[pState->currentString]] =
                        false;
                } else {
                    pState->n775Data.cellTemperature
                        ->cellTemperature_ddegC[pState->currentString][m][pState->currentMux[pState->currentString]] =
                        0;
                    pState->n775Data.cellTemperature
                        ->invalidCellTemperature[pState->currentString][m][pState->currentMux[pState->currentString]] =
                        true;
                }
            } else if (N775_USE_MUX_FOR_TEMP == false) {
                /* No  mux case */
                if ((gpio03Error == false) && (gpio47Error == false)) {
                    for (uint8_t ts = 0u; ts < BS_NR_OF_TEMP_SENSORS_PER_MODULE; ts++) {
                        pState->n775Data.cellTemperature->cellTemperature_ddegC[pState->currentString][m][ts] =
                            N775_ConvertVoltagesToTemperatures(
                                pState->n775Data.allGpioVoltage
                                    ->gpioVoltages_mV[pState->currentString][ts + (m * SLV_NR_OF_GPIOS_PER_MODULE)]);
                        pState->n775Data.cellTemperature->invalidCellTemperature[pState->currentString][m][ts] = false;
                    }
                } else {
                    for (uint8_t ts = 0u; ts < BS_NR_OF_TEMP_SENSORS_PER_MODULE; ts++) {
                        pState->n775Data.cellTemperature->cellTemperature_ddegC[pState->currentString][m][ts]  = 0;
                        pState->n775Data.cellTemperature->invalidCellTemperature[pState->currentString][m][ts] = true;
                    }
                }
            } else {
                /* Invalid value for switch case */
                FAS_ASSERT(FAS_TRAP);
            }
        } else {
            /* Reset temperature values the relevant invalid flag if the communication is not ok */
            for (uint8_t ts = 0u; ts < BS_NR_OF_TEMP_SENSORS_PER_MODULE; ts++) {
                pState->n775Data.cellTemperature->cellTemperature_ddegC[pState->currentString][m][ts]  = 0;
                pState->n775Data.cellTemperature->invalidCellTemperature[pState->currentString][m][ts] = true;
            }
        }

        if (N775_CHECK_SUPPLY_CURRENT == true) {
            N775_ErrorHandling(pState, retValSupplyCurrent, m);
            if (retValSupplyCurrent == N775_COMMUNICATION_OK) {
                if (N775_INVALID_REGISTER_VALUE != currentRawValue) {
                    currentValue = (int16_t)currentRawValue;
                    pState->n775Data.supplyCurrent->current[pState->currentString][m] =
                        (((float_t)currentValue) * 7.69e-6f * 1000.0f);
                } else {
                    error++;
                }
            }
        }
    }

    DATA_WRITE_DATA(pState->n775Data.cellVoltage, pState->n775Data.cellTemperature, pState->n775Data.allGpioVoltage);
}

static STD_RETURN_TYPE_e N775_Enumerate(N775_STATE_s *pState) {
    FAS_ASSERT(pState != NULL_PTR);
    uint16_t readValue                        = 0u;
    uint16_t uid[3u]                          = {0};
    N775_COMMUNICATION_STATUS_e returnedValue = N775_COMMUNICATION_OK;
    STD_RETURN_TYPE_e retVal                  = STD_NOT_OK;

    /** Parse all slaves in the daisy-chain */
    for (uint8_t i = 1; i <= BS_NR_OF_MODULES_PER_STRING; i++) {
        /* First send slave to deep sleep to reset message counter */
        N775_CommunicationWrite(
            i,
            MC33775_SYS_MODE_OFFSET,
            (MC33775_SYS_MODE_TARGETMODE_DEEPSLEEP_ENUM_VAL << MC33775_SYS_MODE_TARGETMODE_POS),
            pState->pSpiTxSequence);
        N775_Wait(N775_T_SW_ACT_DEEP_SLEEP_MS);

        /* Wake up slave */
        returnedValue = N775_CommunicationRead(i, MC33775_SYS_COM_CFG_OFFSET, &readValue, pState);
        /* If slave is not enumerated */
        if (returnedValue != N775_COMMUNICATION_OK) {
            /* Wait until the slave has woken up */
            N775_Wait(N775_T_WAKE_COM_MS);

            returnedValue = N775_CommunicationRead(i, MC33775_SYS_COM_CFG_OFFSET, &readValue, pState);
            /* If slave is not enumerated */
            if (returnedValue != N775_COMMUNICATION_OK) {
                /* Enumerate slave */
                N775_CommunicationWrite(
                    (0u << 6u) + 0u,
                    MC33775_SYS_COM_CFG_OFFSET,
                    i + (N775_DEFAULT_CHAIN_ADDRESS << 6) +
                        (MC33775_SYS_COM_CFG_BUSFW_ENABLED_ENUM_VAL << MC33775_SYS_COM_CFG_BUSFW_POS),
                    pState->pSpiTxSequence);
            }

            /* Reset the message counter of the driver */
            N775_ResetMessageCounter((N775_DEFAULT_CHAIN_ADDRESS << 6) + i, pState->currentString);
            /* Check that the device has been enumerated */
            returnedValue = N775_CommunicationRead(i, MC33775_SYS_VERSION_OFFSET, &readValue, pState);
            if (returnedValue == N775_COMMUNICATION_OK) {
                retVal = STD_OK;
            }
        } else {
            /* Slave already has an address */
            retVal = STD_OK;
        }

        /* Set timeout, enable/disable timeout */
        N775_CommunicationWrite(
            i,
            MC33775_SYS_COM_TO_CFG_OFFSET,
            (N775_TIMEOUT_SWITCH << MC33775_SYS_COM_TO_CFG_COMTODISABLE_POS) |
                (N775_TIMEOUT_TO_SLEEP_10MS << MC33775_SYS_COM_TO_CFG_COMTO_POS),
            pState->pSpiTxSequence);

        /* read uid of each device */
        returnedValue = N775_CommunicationReadMultiple(i, 3u, 3u, MC33775_SYS_UID_LOW_OFFSET, uid, pState);
        if (returnedValue == N775_COMMUNICATION_OK) {
            pState->n775Data.uid[pState->currentString][i - 1u] = 0u;
            for (uint8_t j = 0u; j < 3u; j++) {
                pState->n775Data.uid[pState->currentString][i - 1u] |= ((uint64_t)uid[j]) << (16u * j);
            }
        }
    }

    return retVal;
}

static void N775_ErrorHandling(N775_STATE_s *pState, N775_COMMUNICATION_STATUS_e returnedValue, uint8_t module) {
    FAS_ASSERT(pState != NULL_PTR);

    if (returnedValue == N775_COMMUNICATION_OK) {
        pState->n775Data.errorTable->communicationOk[pState->currentString][module]        = true;
        pState->n775Data.errorTable->noCommunicationTimeout[pState->currentString][module] = true;
        pState->n775Data.errorTable->crcIsValid[pState->currentString][module]             = true;
    } else {
        pState->n775Data.errorTable->communicationOk[pState->currentString][module] = false;
        switch (returnedValue) {
            case N775_COMMUNICATION_ERROR_TIMEOUT:
                pState->n775Data.errorTable->noCommunicationTimeout[pState->currentString][module] = false;
                break;
            case N775_COMMUNICATION_ERROR_WRONG_CRC:
                pState->n775Data.errorTable->crcIsValid[pState->currentString][module] = false;
                break;
            default:
                pState->n775Data.errorTable->communicationOk[pState->currentString][module] = false;
                break;
        }
    }
}

static void N775_IncrementStringSequence(N775_STATE_s *pState) {
    FAS_ASSERT(pState != NULL_PTR);

    pState->currentString++;
    pState->pSpiTxSequence = pState->pSpiTxSequenceStart + pState->currentString;
    pState->pSpiRxSequence = pState->pSpiRxSequenceStart + pState->currentString;
}

static void N775_Initialize(N775_STATE_s *pState) {
    FAS_ASSERT(pState != NULL_PTR);

    /* Reset mux sequence */
    N775_ResetMuxIndex(pState);

    /* Initialize daisy-chain */
    if (STD_OK != N775_Enumerate(pState)) {
        /* error handling */}
        N775_StartMeasurement(pState);
        N775_InitializeI2c(pState);
        N775_BalanceSetup(pState);
}

static void N775_ResetStringSequence(N775_STATE_s *pState) {
    FAS_ASSERT(pState != NULL_PTR);

    pState->currentString  = 0u;
    pState->pSpiTxSequence = pState->pSpiTxSequenceStart + pState->currentString;
    pState->pSpiRxSequence = pState->pSpiRxSequenceStart + pState->currentString;
}

static void N775_SetFirstMeasurementCycleFinished(N775_STATE_s *pState) {
    FAS_ASSERT(pState != NULL_PTR);

    OS_EnterTaskCritical();
    pState->firstMeasurementMade = true;
    OS_ExitTaskCritical();
}

static void N775_StartMeasurement(N775_STATE_s *pState) {
    FAS_ASSERT(pState != NULL_PTR);

    /* Enable cell voltage measurements */
    N775_CommunicationWrite(N775_BROADCAST_ADDRESS, MC33775_ALLM_VCVB_CFG_OFFSET, 0x3FFF, pState->pSpiTxSequence);
    N775_Wait(N775_T_WAIT_CYC_SOC_MS);
    /* Enable analog inputs 0-3 and module voltage measurement */
    N775_CommunicationWrite(N775_BROADCAST_ADDRESS, MC33775_PRMM_AIN_CFG_OFFSET, 0x1F, pState->pSpiTxSequence);
    N775_Wait(N775_T_WAIT_CYC_SOC_MS);
    /* Enable analog inputs 4-7 measurement */
    N775_CommunicationWrite(N775_BROADCAST_ADDRESS, MC33775_SECM_AIN_CFG_OFFSET, 0x0F, pState->pSpiTxSequence);
    N775_Wait(N775_T_WAIT_CYC_SOC_MS);
    /* Set pause of balancing before measurement start, enable the measurement units simultaneously */
    N775_CommunicationWrite(
        N775_BROADCAST_ADDRESS,
        MC33775_ALLM_CFG_OFFSET,
        (N775_BALPAUSELEN_10US << MC33775_ALLM_CFG_BALPAUSELEN_POS) | (1 << MC33775_ALLM_CFG_MEASEN_POS),
        pState->pSpiTxSequence);
    N775_Wait(N775_T_WAIT_CYC_SOC_MS);
}

/*========== Extern Function Implementations ================================*/

extern void N775_Wait(uint32_t milliseconds) {
    uint32_t current_time = OS_GetTickCount();
    /* Block task without possibility to wake up */
    OS_DelayTaskUntil(&current_time, milliseconds);
}

extern bool N775_IsFirstMeasurementCycleFinished(N775_STATE_s *pState) {
    FAS_ASSERT(pState != NULL_PTR);
    bool retval = false;

    OS_EnterTaskCritical();
    retval = pState->firstMeasurementMade;
    OS_ExitTaskCritical();

    return retval;
}

extern void N775_Measure(N775_STATE_s *pState) {
    FAS_ASSERT(pState != NULL_PTR);

    N775_InitializeDatabase(pState);
    /* Initialize SPI sequence pointers */
    pState->pSpiTxSequenceStart = spi_nxp775InterfaceTx;
    pState->pSpiRxSequenceStart = spi_nxp775InterfaceRx;

    /* Initialize each string */
    N775_ResetStringSequence(pState);
    while (pState->currentString < BS_NR_OF_STRINGS) {
        /* Initialize mux sequence pointers */
        pState->pMuxSequenceStart[pState->currentString] = n775_muxSequence;
        N775_Initialize(pState);
        N775_IncrementStringSequence(pState);
    }

    while (1) {
        N775_ResetStringSequence(pState);

        while (pState->currentString < BS_NR_OF_STRINGS) {
            if (N775_USE_MUX_FOR_TEMP == true) {
                /* Set mux channel according to mux sequence */
                N775_SetMuxChannel(pState);
            }
            N775_CaptureMeasurement(pState);
            if (N775_USE_MUX_FOR_TEMP == true) {
                /* Update index in mux sequence */
                N775_IncrementMuxIndex(pState);
            }
            N775_BalanceControl(pState);

            N775_TransmitI2c(pState);

            for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
                if (N775_IsFirstMeasurementCycleFinished(pState) == true) {
                    if (pState->n775Data.errorTable->noCommunicationTimeout[pState->currentString][m] == 0u) {
                        /* Daisy-chain does not answer: initialize it again */
                        pState->pMuxSequenceStart[pState->currentString] = n775_muxSequence;
                        N775_Initialize(pState);
                        /* Daisy-chain re-initialized: no need to check further slaves */
                        break;
                    }
                }
            }

            N775_IncrementStringSequence(pState);
        }

        /* Set flag when all strings have been measured */
        if ((pState->currentString == BS_NR_OF_STRINGS) && (N775_IsFirstMeasurementCycleFinished(pState) == 0u)) {
            N775_SetFirstMeasurementCycleFinished(pState);
        }
    }
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
extern void TEST_N775_CaptureMeasurement(N775_STATE_s *pState) {
    N775_CaptureMeasurement(pState);
}
extern STD_RETURN_TYPE_e TEST_N775_Enumerate(N775_STATE_s *pState) {
    return N775_Enumerate(pState);
}
extern void TEST_N775_ErrorHandling(N775_STATE_s *pState, N775_COMMUNICATION_STATUS_e returnedValue, uint8_t module) {
    N775_ErrorHandling(pState, returnedValue, module);
}
extern void TEST_N775_IncrementStringSequence(N775_STATE_s *pState) {
    N775_IncrementStringSequence(pState);
}
extern void TEST_N775_Initialize(N775_STATE_s *pState) {
    N775_Initialize(pState);
}
extern void TEST_N775_ResetStringSequence(N775_STATE_s *pState) {
    N775_ResetStringSequence(pState);
}
extern void TEST_N775_SetFirstMeasurementCycleFinished(N775_STATE_s *pState) {
    N775_SetFirstMeasurementCycleFinished(pState);
}
extern void TEST_N775_StartMeasurement(N775_STATE_s *pState) {
    N775_StartMeasurement(pState);
}
extern void TEST_N775_Wait(uint32_t milliseconds) {
    N775_Wait(milliseconds);
}
#endif
