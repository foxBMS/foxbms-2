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
 * @file    nxp_mc33775a.c
 * @author  foxBMS Team
 * @date    2020-05-08 (date of creation)
 * @updated 2023-10-12 (date of last update)
 * @version v1.6.0
 * @ingroup DRIVERS
 * @prefix  N775
 *
 * @brief   Driver for the MC33775A analog front-end.
 *
 */

/*========== Includes =======================================================*/
#include "nxp_mc33775a.h"
/* clang-format off */
#include "nxp_mc33775a-ll.h"
/* clang-format on */

#pragma diag_push
#pragma diag_suppress 232
#include "MC33775A.h"
#pragma diag_pop

#include "afe.h"
#include "afe_dma.h"
#include "database.h"
#include "fassert.h"
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
 * @brief   manages balancing.
 * @details Reads balancing order from database and balances the corresponding
 *          cells.
 * @param   pState state of the NXP MC33775A driver
 */
static void N775_BalanceControl(N775_STATE_s *pState);

/**
 * @brief   setups balancing.
 * @details Sets all balancing timer to max to allow for software balancing
 *          control.
 * @param   pState state of the NXP MC33775A driver
 */
static void N775_BalanceSetup(N775_STATE_s *pState);

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
 * @brief   updates index in mux sequence.
 * @param   pState state of the NXP MC33775A driver
 */
static void N775_IncrementMuxIndex(N775_STATE_s *pState);

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
 * @brief   in the database, initializes the fields related to the N775 driver.
 * @details This function loops through all the N775-related data fields in the
 *          database and sets them to 0.
 *          It shall be called in the initialization or re-initialization
 *          routine of the N775 driver.
 * @param   pState  state of the NXP MC33775A driver
 *
 */
static void N775_InitializeDatabase(N775_STATE_s *pState);

/**
 * @brief   init I2C for the N775 slaves.
 * @details This function makes slaves ready for I2C transactions with on-slave
 *          devices.
 * @param   pState state of the NXP MC33775A driver
 */
static void N775_InitializeI2c(N775_STATE_s *pState);

/**
 * @brief   reset index in string sequence.
 * @param   pState state of the NXP MC33775A driver
 */
static void N775_ResetStringSequence(N775_STATE_s *pState);

/**
 * @brief   resets index in mux sequence.
 * @param   pState state of the NXP MC33775A driver
 */
static void N775_ResetMuxIndex(N775_STATE_s *pState);

/**
 * @brief   sets the measurement initialization status.
 * @param   pState state of the NXP MC33775A driver
 */
static void N775_SetFirstMeasurementCycleFinished(N775_STATE_s *pState);

/**
 * @brief   sets mux channel.
 * @details This function uses I2C to set the mux channel.
 * @param   pState state of the NXP MC33775A driver
 */
static STD_RETURN_TYPE_e N775_SetMuxChannel(N775_STATE_s *pState);

/**
 * @brief   starts the measurement.
 * @details The MC33775A measures continuously.
 *          This function starts the measurement.
 * @param  pState  state of the NXP MC33775A driver
 */
static void N775_StartMeasurement(N775_STATE_s *pState);

/**
 * @brief   transmit over I2C on NXP slave.
 * @param   pState state of the NXP MC33775A driver
 */
static STD_RETURN_TYPE_e N775_TransmitI2c(N775_STATE_s *pState);

/**
 * @brief   waits for a definite amount of time in ms.
 * @details This function uses FreeRTOS. It blocks the tasks for the given
 *          amount of milliseconds.
 * @param   milliseconds time to wait in ms
 */
static void N775_Wait(uint32_t milliseconds);

/*========== Static Function Implementations ================================*/

static void N775_BalanceControl(N775_STATE_s *pState) {
    FAS_ASSERT(pState != NULL_PTR);

    N775_BalanceSetup(pState);

    DATA_READ_DATA(pState->n775Data.balancingControl);

    for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
        uint8_t deviceAddress   = m + 1u;
        uint16_t balancingState = 0u;
        for (uint16_t c = 0u; c < BS_NR_OF_CELL_BLOCKS_PER_MODULE; c++) {
            if (pState->n775Data.balancingControl
                    ->balancingState[pState->currentString][c + (m * BS_NR_OF_CELL_BLOCKS_PER_MODULE)] != 0u) {
                balancingState |= 1u << c;
            }
        }
        /* All channels active --> 14 bits set to 1 --> 0x3FFF */
        FAS_ASSERT(balancingState <= 0x3FFFu);
        /* Enable channels, one written to a channels means balancing active */
        N775_CommunicationWrite(deviceAddress, MC33775_BAL_CH_CFG_OFFSET, balancingState, pState->pSpiTxSequence);
    }
}

static void N775_BalanceSetup(N775_STATE_s *pState) {
    FAS_ASSERT(pState != NULL_PTR);

    /* Set global timeout counter to max value */
    N775_CommunicationWrite(
        N775_BROADCAST_ADDRESS, MC33775_BAL_GLOB_TO_TMR_OFFSET, N775_GLOBAL_BALANCING_TIMER, pState->pSpiTxSequence);

    /* Disable pre-balancing timer by setting it to zero */
    N775_CommunicationWrite(
        N775_BROADCAST_ADDRESS, MC33775_BAL_PRE_TMR_OFFSET, N775_PRE_BALANCING_TIMER, pState->pSpiTxSequence);

    /* Set PWM value for all channels to 100%, set balancing timer for all channels to maximum value */
    N775_CommunicationWrite(
        N775_BROADCAST_ADDRESS,
        MC33775_BAL_TMR_CH_ALL_OFFSET,
        (MC33775_BAL_TMR_CH_ALL_PWM_PWM100_ENUM_VAL << MC33775_BAL_TMR_CH_ALL_PWM_POS) |
            (N775_ALL_CHANNEL_BALANCING_TIMER << MC33775_BAL_TMR_CH_ALL_BALTIME_POS),
        pState->pSpiTxSequence);

    /* Select timer based balancing and enable balancing */
    N775_CommunicationWrite(
        N775_BROADCAST_ADDRESS,
        MC33775_BAL_GLOB_CFG_OFFSET,
        (MC33775_BAL_GLOB_CFG_BALEN_ENABLED_ENUM_VAL << MC33775_BAL_GLOB_CFG_BALEN_POS) |
            (MC33775_BAL_GLOB_CFG_TMRBALEN_STOP_ENUM_VAL << MC33775_BAL_GLOB_CFG_TMRBALEN_POS),
        pState->pSpiTxSequence);
}

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
                        (((float_t)primaryValues[cb + 1u]) * 154.0e-6f * 1000.0f);
                } else {
                    error++;
                }
            }
            for (uint8_t g = 0u; g < 4u; g++) {
                /* Store GPIO voltages */
                if (N775_INVALID_REGISTER_VALUE != primaryRawValues[g + 16u]) {
                    primaryValues[g + 16u] = (int16_t)primaryRawValues[g + 16u];
                    pState->n775Data.allGpioVoltage
                        ->gpioVoltages_mV[pState->currentString][g + (m * BS_NR_OF_GPIOS_PER_MODULE)] =
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
        }

        N775_ErrorHandling(pState, retValSecondary, m);
        if (retValSecondary == N775_COMMUNICATION_OK) {
            for (uint8_t g = 4u; g < 8u; g++) {
                if (N775_INVALID_REGISTER_VALUE != secondaryRawValues[g - 4u]) {
                    secondaryValues[g - 4u] = (int16_t)secondaryRawValues[g - 4u];
                    pState->n775Data.allGpioVoltage
                        ->gpioVoltages_mV[pState->currentString][g + (m * BS_NR_OF_GPIOS_PER_MODULE)] =
                        (((float_t)secondaryValues[g - 4u]) * 154.0e-6f * 1000.0f);
                } else {
                    gpio47Error = true;
                    error++;
                }
            }
        }

        /* Set temperature values */
        if (N775_USE_MUX_FOR_TEMP == true) {
            /* Mux case */
            if (gpio03Error == false) {
                pState->n775Data.cellTemperature
                    ->cellTemperature_ddegC[pState->currentString][m][pState->currentMux[pState->currentString]] =
                    N775_ConvertVoltagesToTemperatures(
                        pState->n775Data.allGpioVoltage
                            ->gpioVoltages_mV[pState->currentString]
                                             [N775_MUXED_TEMP_GPIO_POSITION + (m * BS_NR_OF_GPIOS_PER_MODULE)]);
            }
        } else if (N775_USE_MUX_FOR_TEMP == false) {
            /* No  mux case */
            if ((gpio03Error == false) && (gpio47Error == false)) {
                for (uint8_t ts = 0u; ts < BS_NR_OF_TEMP_SENSORS_PER_MODULE; ts++) {
                    pState->n775Data.cellTemperature->cellTemperature_ddegC[pState->currentString][m][ts] =
                        N775_ConvertVoltagesToTemperatures(
                            pState->n775Data.allGpioVoltage
                                ->gpioVoltages_mV[pState->currentString][ts + (m * BS_NR_OF_GPIOS_PER_MODULE)]);
                }
            }
        } else {
            /* Invalid value for switch case */
            FAS_ASSERT(FAS_TRAP);
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
        N775_Wait(1u);

        /* Wake up slave */
        returnedValue = N775_CommunicationRead(i, MC33775_SYS_COM_CFG_OFFSET, &readValue, pState);
        /* If slave is not enumerated */
        if (returnedValue != N775_COMMUNICATION_OK) {
            /* Wait until the slave has woken up */
            N775_Wait(N775_WAKEUP_TIME_MS);

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
            for (uint8_t j = 0u; j <= 3u; j++) {
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

static void N775_IncrementMuxIndex(N775_STATE_s *pState) {
    FAS_ASSERT(pState != NULL_PTR);

    pState->currentMux[pState->currentString]++;
    if (pState->currentMux[pState->currentString] >= N775_MUX_SEQUENCE_LENGTH) {
        pState->currentMux[pState->currentString] = 0u;
    }
    pState->pMuxSequence[pState->currentString] = pState->pMuxSequenceStart[pState->currentString] +
                                                  pState->currentMux[pState->currentString];
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

static void N775_InitializeDatabase(N775_STATE_s *pState) {
    FAS_ASSERT(pState != NULL_PTR);
    uint16_t iterator = 0;

    for (uint8_t stringNumber = 0u; stringNumber < BS_NR_OF_STRINGS; stringNumber++) {
        pState->n775Data.cellVoltage->state                               = 0;
        pState->n775Data.minMax->minimumCellVoltage_mV[stringNumber]      = 0;
        pState->n775Data.minMax->maximumCellVoltage_mV[stringNumber]      = 0;
        pState->n775Data.minMax->nrModuleMinimumCellVoltage[stringNumber] = 0;
        pState->n775Data.minMax->nrModuleMaximumCellVoltage[stringNumber] = 0;
        pState->n775Data.minMax->nrCellMinimumCellVoltage[stringNumber]   = 0;
        pState->n775Data.minMax->nrCellMaximumCellVoltage[stringNumber]   = 0;
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t cb = 0u; cb < BS_NR_OF_CELL_BLOCKS_PER_STRING; cb++) {
                pState->n775Data.cellVoltage->cellVoltage_mV[stringNumber][m][cb] = 0;
            }
        }

        pState->n775Data.cellTemperature->state                           = 0;
        pState->n775Data.minMax->minimumTemperature_ddegC[stringNumber]   = 0;
        pState->n775Data.minMax->maximumTemperature_ddegC[stringNumber]   = 0;
        pState->n775Data.minMax->nrModuleMinimumTemperature[stringNumber] = 0;
        pState->n775Data.minMax->nrModuleMaximumTemperature[stringNumber] = 0;
        pState->n775Data.minMax->nrSensorMinimumTemperature[stringNumber] = 0;
        pState->n775Data.minMax->nrSensorMaximumTemperature[stringNumber] = 0;

        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t ts = 0u; ts < BS_NR_OF_TEMP_SENSORS_PER_MODULE; ts++) {
                pState->n775Data.cellTemperature->cellTemperature_ddegC[stringNumber][m][ts] = 0;
            }
        }

        for (iterator = 0u; iterator < BS_NR_OF_CELL_BLOCKS_PER_STRING; iterator++) {
            pState->n775Data.balancingControl->balancingState[stringNumber][iterator] = 0;
        }
        for (iterator = 0u; iterator < BS_NR_OF_MODULES_PER_STRING; iterator++) {
            pState->n775Data.errorTable->communicationOk[stringNumber][iterator]        = false;
            pState->n775Data.errorTable->noCommunicationTimeout[stringNumber][iterator] = false;
            pState->n775Data.errorTable->crcIsValid[stringNumber][iterator]             = false;
            pState->n775Data.errorTable->mux0IsOk[stringNumber][iterator]               = false;
            pState->n775Data.errorTable->mux1IsOK[stringNumber][iterator]               = false;
            pState->n775Data.errorTable->mux2IsOK[stringNumber][iterator]               = false;
            pState->n775Data.errorTable->mux3IsOK[stringNumber][iterator]               = false;
        }
        for (iterator = 0u; iterator < BS_NR_OF_MODULES_PER_STRING; iterator++) {
            pState->n775Data.uid[stringNumber][iterator] = 0;
        }
    }

    DATA_WRITE_DATA(pState->n775Data.cellVoltage);
    DATA_WRITE_DATA(pState->n775Data.cellTemperature);
    DATA_WRITE_DATA(pState->n775Data.minMax);
    DATA_WRITE_DATA(pState->n775Data.balancingControl);
}

static void N775_InitializeI2c(N775_STATE_s *pState) {
    FAS_ASSERT(pState != NULL_PTR);

    /* Enable the I2C module and select 400 kHz */
    N775_CommunicationWrite(
        N775_BROADCAST_ADDRESS,
        MC33775_I2C_CFG_OFFSET,
        (MC33775_I2C_CFG_EN_ENABLED_ENUM_VAL << MC33775_I2C_CFG_EN_POS) +
            (MC33775_I2C_CFG_CLKSEL_F_400K_ENUM_VAL << MC33775_I2C_CFG_CLKSEL_POS),
        pState->pSpiTxSequence);
}

static void N775_ResetStringSequence(N775_STATE_s *pState) {
    FAS_ASSERT(pState != NULL_PTR);

    pState->currentString  = 0u;
    pState->pSpiTxSequence = pState->pSpiTxSequenceStart + pState->currentString;
    pState->pSpiRxSequence = pState->pSpiRxSequenceStart + pState->currentString;
}

static void N775_ResetMuxIndex(N775_STATE_s *pState) {
    FAS_ASSERT(pState != NULL_PTR);

    pState->currentMux[pState->currentString]   = 0u;
    pState->pMuxSequence[pState->currentString] = pState->pMuxSequenceStart[pState->currentString];
}

static void N775_SetFirstMeasurementCycleFinished(N775_STATE_s *pState) {
    FAS_ASSERT(pState != NULL_PTR);

    OS_EnterTaskCritical();
    pState->firstMeasurementMade = true;
    OS_ExitTaskCritical();
}

static STD_RETURN_TYPE_e N775_SetMuxChannel(N775_STATE_s *pState) {
    FAS_ASSERT(pState != NULL_PTR);
    FAS_ASSERT(pState->pMuxSequence[pState->currentString]->muxId < 4u);
    FAS_ASSERT(pState->pMuxSequence[pState->currentString]->muxChannel <= 0xFFu);

    uint16_t readValue                        = 0u;
    uint8_t dataI2c                           = 0u;
    uint8_t addressI2c_write                  = N775_ADG728_ADDRESS_UPPERBITS;
    uint8_t addressI2c_read                   = N775_ADG728_ADDRESS_UPPERBITS;
    uint16_t tries                            = 0u;
    STD_RETURN_TYPE_e retVAL                  = STD_OK;
    N775_COMMUNICATION_STATUS_e returnedValue = N775_COMMUNICATION_OK;

    /* First set channel */

    /* Set bit1 and bit0 with mux address, write to mux */
    addressI2c_write |= ((pState->pMuxSequence[pState->currentString]->muxId) << 1u) | N775_I2C_WRITE;
    /* Set bit1 and bit0 with mux address, read from mux */
    addressI2c_read |= ((pState->pMuxSequence[pState->currentString]->muxId) << 1u) | N775_I2C_READ;

    /**
     * Set data to send, contains channel bit (8 channels)
     * 1 means channel active, 0 means channel inactive
     */
    if (pState->pMuxSequence[pState->currentString]->muxChannel == 0xFF) {
        /* 0xFF in mux sequence means disable all channels */
        dataI2c = 0u;
    } else {
        dataI2c = (uint8_t)(1u << (pState->pMuxSequence[pState->currentString]->muxChannel));
    }

    /* Write data to send on I2C bus in registers */
    N775_CommunicationWrite(
        N775_BROADCAST_ADDRESS,
        MC33775_I2C_DATA0_OFFSET,
        (addressI2c_write << MC33775_I2C_DATA0_BYTE0_POS) | (dataI2c << MC33775_I2C_DATA0_BYTE1_POS),
        pState->pSpiTxSequence);

    /* Read with a repeated start directly after write */
    N775_CommunicationWrite(
        N775_BROADCAST_ADDRESS,
        MC33775_I2C_DATA1_OFFSET,
        (addressI2c_read << MC33775_I2C_DATA1_BYTE2_POS) | (N775_I2C_DUMMY_BYTE << MC33775_I2C_DATA1_BYTE3_POS),
        pState->pSpiTxSequence);

    /* Write into the control register to start transaction */
    N775_CommunicationWrite(
        N775_BROADCAST_ADDRESS,
        MC33775_I2C_CTRL_OFFSET,
        (N775_I2C_NR_BYTES_FOR_MUX_WRITE << MC33775_I2C_CTRL_START_POS) |
            ((MC33775_I2C_CTRL_STPAFTER_STOP_ENUM_VAL << MC33775_I2C_CTRL_STPAFTER_POS) +
             (N775_I2C_NR_BYTES_TO_SWITCH_TO_READ_FOR_UX_READ << MC33775_I2C_CTRL_RDAFTER_POS)),
        pState->pSpiTxSequence);

    /**
     * Wait util transaction ends, test on last device in daisy-chain
     * So device address = number of modules
     */
    tries = N775_FLAG_READY_TRIES;
    do {
        returnedValue =
            N775_CommunicationRead(BS_NR_OF_MODULES_PER_STRING, MC33775_I2C_STAT_OFFSET, &readValue, pState);
        tries--;
        N775_Wait(2u);
    } while ((readValue & MC33775_I2C_STAT_PENDING_MSK) && (returnedValue == N775_COMMUNICATION_OK) && (tries > 0u));

    if ((returnedValue == N775_COMMUNICATION_OK) && (tries > 0u)) {
        /**
         *  Get I2C read data, on last device in daisy-chain
         *  Use result to set error state for all slaves to avoid
         *  reading all slaves in daisy-chain.
         */
        returnedValue =
            N775_CommunicationRead(BS_NR_OF_MODULES_PER_STRING, MC33775_I2C_DATA1_OFFSET, &readValue, pState);
        readValue = readValue >> MC33775_I2C_DATA1_BYTE3_POS;

        if (returnedValue == N775_COMMUNICATION_OK) {
            if (N775_CHECK_MUX_STATE == true) {
                if (readValue == dataI2c) {
                    /* OK */
                    for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
                        pState->n775Data.errorTable->mux0IsOk[pState->currentString][m] = true;
                    }
                } else {
                    /* Not OK */
                    for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
                        pState->n775Data.errorTable->mux0IsOk[pState->currentString][m] = false;
                    }
                }
            } else {
                retVAL = STD_NOT_OK;
            }
        }
    } else {
        retVAL = STD_NOT_OK;
    }

    return retVAL;
}

static void N775_StartMeasurement(N775_STATE_s *pState) {
    FAS_ASSERT(pState != NULL_PTR);

    /* Enable cell voltage measurements */
    N775_CommunicationWrite(N775_BROADCAST_ADDRESS, MC33775_ALLM_VCVB_CFG_OFFSET, 0x3FFF, pState->pSpiTxSequence);
    /* Enable analog inputs 0-3 and module voltage measurement */
    N775_CommunicationWrite(N775_BROADCAST_ADDRESS, MC33775_PRMM_AIN_CFG_OFFSET, 0x1F, pState->pSpiTxSequence);
    /* Enable analog inputs 4-7 measurement */
    N775_CommunicationWrite(N775_BROADCAST_ADDRESS, MC33775_SECM_AIN_CFG_OFFSET, 0x0F, pState->pSpiTxSequence);
    /* Set pause of balancing before measurement start, enable the measurement units simultaneously */
    N775_CommunicationWrite(
        N775_BROADCAST_ADDRESS,
        MC33775_ALLM_CFG_OFFSET,
        (N775_BALPAUSELEN_10US << MC33775_ALLM_CFG_BALPAUSELEN_POS) | (1 << MC33775_ALLM_CFG_MEASEN_POS),
        pState->pSpiTxSequence);

    N775_Wait(N775_TIME_AFTER_MEASUREMENT_START_MS);
}

static STD_RETURN_TYPE_e N775_TransmitI2c(N775_STATE_s *pState) {
    FAS_ASSERT(pState != NULL_PTR);
    uint8_t i2cAddressByte                    = 0u;
    uint16_t readValue                        = 0u;
    uint16_t tries                            = 0u;
    STD_RETURN_TYPE_e retVal                  = STD_OK;
    N775_COMMUNICATION_STATUS_e returnedValue = N775_COMMUNICATION_OK;
    AFE_I2C_QUEUE_s transactionData;

    if (ftsk_allQueuesCreated == true) {
        if (OS_ReceiveFromQueue(ftsk_afeToI2cQueue, (void *)&transactionData, 0u) == OS_SUCCESS) {
            /* Queue was not empty */
            i2cAddressByte            = (transactionData.deviceAddress << 1u) & 0xFEu; /* I2C address has only 7 bits */
            uint8_t nack              = 0u;
            uint8_t registerIncrement = 0u;
            uint16_t registerStartAddress = 0u;
            uint16_t dataToWrite          = 0u;
            uint8_t byteIndex             = 0u;
            uint8_t bytesWritten          = 0u;
            uint8_t msbIndex              = 1u;
            uint8_t lsbIndex              = 1u;

            switch (transactionData.transferType) {
                case AFE_I2C_TRANSFER_TYPE_WRITE:
                    transactionData.transferType = AFE_I2C_TRANSFER_TYPE_WRITE_FAIL;
                    /* First prepare data to send on I2C bus in registers */
                    i2cAddressByte |= N775_I2C_WRITE;
                    /* Set I2C device address for write access followed by first byte of data */
                    N775_CommunicationWrite(
                        transactionData.module + 1u,
                        MC33775_I2C_DATA0_OFFSET,
                        (i2cAddressByte << MC33775_I2C_DATA0_BYTE0_POS) |
                            (transactionData.writeData[0u] << MC33775_I2C_DATA0_BYTE1_POS),
                        pState->pSpiTxSequence);

                    if (transactionData.writeDataLength > 1u) {
                        /* Now set data to be written to I2C device */
                        registerStartAddress = MC33775_I2C_DATA1_OFFSET;
                        registerIncrement    = 0u;
                        byteIndex            = 1u;
                        dataToWrite          = 0u;
                        while (byteIndex < transactionData.writeDataLength) {
                            /* Each subsequent data register contains two bytes of read data */

                            if ((byteIndex % 2u) != 0u) {
                                dataToWrite |= (uint16_t)(transactionData.writeData[byteIndex]) & 0xFFu;
                                bytesWritten++;
                            } else {
                                dataToWrite |= ((uint16_t)(transactionData.writeData[byteIndex]) << 8u) & 0xFF00u;
                                bytesWritten++;
                            }
                            byteIndex++;
                            if ((bytesWritten == 2u) || (byteIndex == transactionData.writeDataLength)) {
                                N775_CommunicationWrite(
                                    transactionData.module + 1u,
                                    registerStartAddress + registerIncrement,
                                    dataToWrite,
                                    pState->pSpiTxSequence);
                                registerIncrement++;
                                bytesWritten = 0u;
                                dataToWrite  = 0u;
                            }
                        }
                    }
                    /* Data to write ready, now start transmission */
                    /* Write into the control register to start transaction */
                    N775_CommunicationWrite(
                        transactionData.module + 1u,
                        MC33775_I2C_CTRL_OFFSET,
                        /* transactionData.writeDataLength + 1u: data + I2C device address byte */
                        ((transactionData.writeDataLength + 1u) << MC33775_I2C_CTRL_START_POS) |
                            ((MC33775_I2C_CTRL_STPAFTER_STOP_ENUM_VAL << MC33775_I2C_CTRL_STPAFTER_POS) +
                             (0u << MC33775_I2C_CTRL_RDAFTER_POS)),
                        pState->pSpiTxSequence);
                    /* Wait until transaction ends */
                    tries = N775_FLAG_READY_TRIES;
                    do {
                        returnedValue = N775_CommunicationRead(
                            transactionData.module + 1u, MC33775_I2C_STAT_OFFSET, &readValue, pState);
                        tries--;
                        N775_Wait(2u);
                    } while ((readValue & MC33775_I2C_STAT_PENDING_MSK) && (returnedValue == N775_COMMUNICATION_OK) &&
                             (tries > 0u));

                    if ((returnedValue == N775_COMMUNICATION_OK) && (tries > 0u)) {
                        retVal                       = STD_OK;
                        transactionData.transferType = AFE_I2C_TRANSFER_TYPE_WRITE_SUCCESS;
                    }
                    if (OS_SendToBackOfQueue(ftsk_afeFromI2cQueue, (void *)&transactionData, 0u) == OS_SUCCESS) {
                        /* Queue is not full */
                    } else {
                        /* Queue is full */
                        retVal = STD_NOT_OK;
                    }
                    break;

                case AFE_I2C_TRANSFER_TYPE_READ:
                    transactionData.transferType = AFE_I2C_TRANSFER_TYPE_READ_FAIL;
                    i2cAddressByte |= N775_I2C_READ;
                    /* First prepare address to send on I2C bus in registers */
                    N775_CommunicationWrite(
                        transactionData.module + 1u,
                        MC33775_I2C_DATA0_OFFSET,
                        i2cAddressByte << MC33775_I2C_DATA0_BYTE0_POS,
                        pState->pSpiTxSequence);
                    /* Write into the control register to start transaction */
                    /* Stop condition after transfer, no repeated start */
                    N775_CommunicationWrite(
                        transactionData.module + 1u,
                        MC33775_I2C_CTRL_OFFSET,
                        /* transactionData.dataLength + 1u: data + I2C device address byte */
                        ((transactionData.readDataLength + 1u) << MC33775_I2C_CTRL_START_POS) |
                            ((1u << MC33775_I2C_CTRL_STPAFTER_POS) + (0u << MC33775_I2C_CTRL_RDAFTER_POS)),
                        pState->pSpiTxSequence);
                    /* Wait until transaction ends */
                    tries = N775_FLAG_READY_TRIES;
                    do {
                        returnedValue = N775_CommunicationRead(
                            transactionData.module + 1u, MC33775_I2C_STAT_OFFSET, &readValue, pState);
                        tries--;
                        N775_Wait(2u);
                    } while ((readValue & MC33775_I2C_STAT_PENDING_MSK) && (returnedValue == N775_COMMUNICATION_OK) &&
                             (tries > 0u));
                    /* Now retrieve read data */
                    nack = readValue & MC33775_I2C_STAT_NACKRCV_MSK;
                    if ((returnedValue == N775_COMMUNICATION_OK) && (tries > 0u) && (nack == 0u)) {
                        uint16_t readData[13u] = {0u};
                        /* In data registers, bytes0 contains the address, read data begins at byte1 */
                        /* First data register contains byte1, second data register byte2 and byte3, ... */
                        uint16_t nrOfRegisterToRead = (transactionData.readDataLength / 2u) + 1u;
                        returnedValue               = N775_CommunicationReadMultiple(
                            transactionData.module + 1u,
                            nrOfRegisterToRead,
                            4u,
                            MC33775_I2C_DATA0_OFFSET,
                            readData,
                            pState);

                        /* First data register only contains one byte of the read data */
                        transactionData.readData[0u] = (uint8_t)((readData[0u] & 0xFF00) >> 8u);
                        byteIndex                    = 1u;
                        msbIndex                     = 1u;
                        lsbIndex                     = 1u;
                        while (byteIndex < transactionData.readDataLength) {
                            /* Each subsequent data register contains two bytes of read data */
                            if ((byteIndex % 2u) != 0u) {
                                transactionData.readData[byteIndex] = (uint8_t)(readData[lsbIndex] & 0xFFu);
                                lsbIndex++;
                            } else {
                                transactionData.readData[byteIndex] = (uint8_t)((readData[msbIndex] & 0xFF00u) >> 8u);
                                msbIndex++;
                            }
                            byteIndex++;
                        }
                        retVal                       = STD_OK;
                        transactionData.transferType = AFE_I2C_TRANSFER_TYPE_READ_SUCCESS;
                    }
                    if (OS_SendToBackOfQueue(ftsk_afeFromI2cQueue, (void *)&transactionData, 0u) == OS_SUCCESS) {
                        /* Queue is not full */
                    } else {
                        /* Queue is full */
                        retVal = STD_NOT_OK;
                    }
                    break;

                case AFE_I2C_TRANSFER_TYPE_WRITEREAD:
                    transactionData.transferType = AFE_I2C_TRANSFER_TYPE_READ_FAIL;
                    /* First prepare address to send on I2C bus in registers */
                    N775_CommunicationWrite(
                        transactionData.module + 1u,
                        MC33775_I2C_DATA0_OFFSET,
                        ((i2cAddressByte | N775_I2C_WRITE) << MC33775_I2C_DATA0_BYTE0_POS) |
                            (transactionData.writeData[0u] << MC33775_I2C_DATA0_BYTE1_POS),
                        pState->pSpiTxSequence);

                    if (transactionData.writeDataLength > 1u) {
                        /* Now set data to be written to I2C device */
                        registerStartAddress                                       = MC33775_I2C_DATA1_OFFSET;
                        registerIncrement                                          = 0u;
                        byteIndex                                                  = 1u;
                        dataToWrite                                                = 0u;
                        transactionData.writeData[transactionData.writeDataLength] = i2cAddressByte | N775_I2C_READ;
                        while (byteIndex < (transactionData.writeDataLength + 1)) {
                            /* Each subsequent data register contains two bytes of read data */

                            if ((byteIndex % 2u) != 0u) {
                                dataToWrite |= (uint16_t)(transactionData.writeData[byteIndex]) & 0xFFu;
                                bytesWritten++;
                            } else {
                                dataToWrite |= ((uint16_t)(transactionData.writeData[byteIndex]) << 8u) & 0xFF00u;
                                bytesWritten++;
                            }
                            byteIndex++;
                            if ((bytesWritten == 2u) || (byteIndex == (transactionData.writeDataLength + 1))) {
                                N775_CommunicationWrite(
                                    transactionData.module + 1u,
                                    registerStartAddress + registerIncrement,
                                    dataToWrite,
                                    pState->pSpiTxSequence);
                                registerIncrement++;
                                bytesWritten = 0u;
                                dataToWrite  = 0u;
                            }
                        }
                    } else {
                        N775_CommunicationWrite(
                            transactionData.module + 1u,
                            MC33775_I2C_DATA1_OFFSET,
                            ((i2cAddressByte | N775_I2C_READ) << MC33775_I2C_DATA1_BYTE2_POS),
                            pState->pSpiTxSequence);
                    }

                    /* Write into the control register to start transaction */
                    /* Stop condition after transfer, repeated start */
                    N775_CommunicationWrite(
                        transactionData.module + 1u,
                        MC33775_I2C_CTRL_OFFSET,
                        /* transaction length: I2C device address byte for write + data to write
                                   + I2C device address byte for read + data to read */
                        ((transactionData.writeDataLength + transactionData.readDataLength + 2u)
                         << MC33775_I2C_CTRL_START_POS) |
                            ((1u << MC33775_I2C_CTRL_STPAFTER_POS) +
                             ((1u + transactionData.writeDataLength) << MC33775_I2C_CTRL_RDAFTER_POS)),
                        pState->pSpiTxSequence);
                    /* Wait until transaction ends */
                    tries = N775_FLAG_READY_TRIES;
                    do {
                        returnedValue = N775_CommunicationRead(
                            transactionData.module + 1u, MC33775_I2C_STAT_OFFSET, &readValue, pState);
                        tries--;
                        N775_Wait(2u);
                    } while ((readValue & MC33775_I2C_STAT_PENDING_MSK) && (returnedValue == N775_COMMUNICATION_OK) &&
                             (tries > 0u));
                    /* Now retrieve read data */
                    nack = readValue & MC33775_I2C_STAT_NACKRCV_MSK;
                    if ((returnedValue == N775_COMMUNICATION_OK) && (tries > 0u) && (nack == 0u)) {
                        uint16_t readData[13u] = {0u};
                        /* First data to read is at least in this register */
                        uint16_t registerOffset = MC33775_I2C_DATA0_OFFSET;
                        /* Find offset of first register to read */
                        registerOffset += (transactionData.writeDataLength + 2u) / 2u;
                        /* In data registers, byte0 contains the device address, byte1 the first byte written */
                        /* I2C device address byte for read is present before the first read byte */
                        uint16_t nrOfRegisterToRead = 0u;
                        if ((transactionData.writeDataLength % 2u) == 0u) {
                            nrOfRegisterToRead = (transactionData.readDataLength + 1u) / 2u;
                        } else {
                            nrOfRegisterToRead = (transactionData.readDataLength / 2u) + 1u;
                        }

                        returnedValue = N775_CommunicationReadMultiple(
                            transactionData.module + 1u, nrOfRegisterToRead, 4u, registerOffset, readData, pState);

                        /* Second data register only contains one byte of the read data (byte3)
                           Read data starts at second register because:
                           byte0: I2C device address for write access
                           byte1: first byte written
                           I2C device address byte for read is present before the first read byte
                           */
                        if (((transactionData.writeDataLength + 1) % 2u) != 0u) {
                            byteIndex = 0u;
                            msbIndex  = 0u;
                            lsbIndex  = 0u;
                            while (byteIndex < transactionData.readDataLength) {
                                /* Each subsequent data register contains two bytes of read data */
                                if ((byteIndex % 2u) == 0u) {
                                    transactionData.readData[byteIndex] = (uint8_t)(readData[lsbIndex] & 0xFFu);
                                    lsbIndex++;
                                } else {
                                    transactionData.readData[byteIndex] =
                                        (uint8_t)((readData[msbIndex] & 0xFF00u) >> 8u);
                                    msbIndex++;
                                }
                                byteIndex++;
                            }
                        } else {
                            byteIndex = 0u;
                            msbIndex  = 0u;
                            lsbIndex  = 1u;
                            while (byteIndex < transactionData.readDataLength) {
                                /* Each subsequent data register contains two bytes of read data */
                                if ((byteIndex % 2u) != 0u) {
                                    transactionData.readData[byteIndex] = (uint8_t)(readData[lsbIndex] & 0xFFu);
                                    lsbIndex++;
                                } else {
                                    transactionData.readData[byteIndex] =
                                        (uint8_t)((readData[msbIndex] & 0xFF00u) >> 8u);
                                    msbIndex++;
                                }
                                byteIndex++;
                            }
                        }
                        retVal                       = STD_OK;
                        transactionData.transferType = AFE_I2C_TRANSFER_TYPE_READ_SUCCESS;
                    }
                    if (OS_SendToBackOfQueue(ftsk_afeFromI2cQueue, (void *)&transactionData, 0u) == OS_SUCCESS) {
                        /* Queue is not full */
                    } else {
                        /* Queue is full */
                        retVal = STD_NOT_OK;
                    }
                    break;

                default:
                    /* Invalid value transfer type */
                    FAS_ASSERT(FAS_TRAP);
                    break;
            }
        }
    }

    return retVal;
}

static void N775_Wait(uint32_t milliseconds) {
    uint32_t current_time = OS_GetTickCount();
    /* Block task without possibility to wake up */
    OS_DelayTaskUntil(&current_time, milliseconds);
}

/*========== Extern Function Implementations ================================*/

extern STD_RETURN_TYPE_e N775_I2cRead(uint8_t module, uint8_t deviceAddress, uint8_t *pData, uint8_t dataLength) {
    FAS_ASSERT(pData != NULL_PTR);
    FAS_ASSERT((dataLength > 0u) && (dataLength <= 13u));
    STD_RETURN_TYPE_e retVal = STD_NOT_OK;
    AFE_I2C_QUEUE_s transactionData;

    transactionData.module         = module;
    transactionData.deviceAddress  = deviceAddress;
    transactionData.readDataLength = dataLength;
    transactionData.transferType   = AFE_I2C_TRANSFER_TYPE_READ;
    if (OS_SendToBackOfQueue(ftsk_afeToI2cQueue, (void *)&transactionData, 0u) == OS_SUCCESS) {
        /* queue is not full */
        /* waiting for transfer to finish */
        if (OS_ReceiveFromQueue(ftsk_afeFromI2cQueue, (void *)&transactionData, N775_I2C_FINISHED_TIMEOUT_ms) ==
            OS_SUCCESS) {
            if (transactionData.transferType == AFE_I2C_TRANSFER_TYPE_READ_SUCCESS) {
                for (uint8_t i = 0u; i < dataLength; i++) {
                    pData[i] = transactionData.readData[i];
                }
                retVal = STD_OK;
            }
        }
    } else {
        /* queue is full */
    }

    return retVal;
}

extern STD_RETURN_TYPE_e N775_I2cWrite(uint8_t module, uint8_t deviceAddress, uint8_t *pData, uint8_t dataLength) {
    FAS_ASSERT(pData != NULL_PTR);
    FAS_ASSERT((dataLength > 0u) && (dataLength <= 13u));
    STD_RETURN_TYPE_e retVal = STD_NOT_OK;
    AFE_I2C_QUEUE_s transactionData;

    transactionData.module          = module;
    transactionData.deviceAddress   = deviceAddress;
    transactionData.writeDataLength = dataLength;
    transactionData.transferType    = AFE_I2C_TRANSFER_TYPE_WRITE;
    for (uint8_t i = 0u; i < dataLength; i++) {
        transactionData.writeData[i] = pData[i];
    }
    if (OS_SendToBackOfQueue(ftsk_afeToI2cQueue, (void *)&transactionData, 0u) == OS_SUCCESS) {
        /* queue is not full */
        /* waiting for transfer to finish */
        if (OS_ReceiveFromQueue(ftsk_afeFromI2cQueue, (void *)&transactionData, N775_I2C_FINISHED_TIMEOUT_ms) ==
            OS_SUCCESS) {
            if (transactionData.transferType == AFE_I2C_TRANSFER_TYPE_WRITE_SUCCESS) {
                retVal = STD_OK;
            }
        }
    } else {
        /* queue is full */
    }

    return retVal;
}

extern STD_RETURN_TYPE_e N775_I2cWriteRead(
    uint8_t module,
    uint8_t deviceAddress,
    uint8_t *pDataWrite,
    uint8_t writeDataLength,
    uint8_t *pDataRead,
    uint8_t readDataLength) {
    FAS_ASSERT(pDataWrite != NULL_PTR);
    FAS_ASSERT(pDataRead != NULL_PTR);
    FAS_ASSERT(writeDataLength > 0u);
    FAS_ASSERT(readDataLength > 0u);
    FAS_ASSERT((uint16_t)(writeDataLength + readDataLength) <= 12u);
    STD_RETURN_TYPE_e retVal = STD_NOT_OK;
    AFE_I2C_QUEUE_s transactionData;

    transactionData.module          = module;
    transactionData.deviceAddress   = deviceAddress;
    transactionData.writeDataLength = writeDataLength;
    transactionData.readDataLength  = readDataLength;
    transactionData.transferType    = AFE_I2C_TRANSFER_TYPE_WRITEREAD;
    for (uint8_t i = 0u; i < writeDataLength; i++) {
        transactionData.writeData[i] = pDataWrite[i];
    }
    if (OS_SendToBackOfQueue(ftsk_afeToI2cQueue, (void *)&transactionData, 0u) == OS_SUCCESS) {
        /* queue is not full */
        /* waiting for transfer to finish */
        if (OS_ReceiveFromQueue(ftsk_afeFromI2cQueue, (void *)&transactionData, N775_I2C_FINISHED_TIMEOUT_ms) ==
            OS_SUCCESS) {
            if (transactionData.transferType == AFE_I2C_TRANSFER_TYPE_READ_SUCCESS) {
                for (uint8_t i = 0u; i < readDataLength; i++) {
                    pDataRead[i] = transactionData.readData[i];
                }
                retVal = STD_OK;
            }
        }
    } else {
        /* queue is full */
    }

    return retVal;
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
extern void TEST_N775_BalanceControl(N775_STATE_s *pState) {
    N775_BalanceControl(pState);
}
extern void TEST_N775_BalanceSetup(N775_STATE_s *pState) {
    N775_BalanceSetup(pState);
}
extern void TEST_N775_CaptureMeasurement(N775_STATE_s *pState) {
    N775_CaptureMeasurement(pState);
}
extern STD_RETURN_TYPE_e TEST_N775_Enumerate(N775_STATE_s *pState) {
    return N775_Enumerate(pState);
}
extern void TEST_N775_ErrorHandling(N775_STATE_s *pState, N775_COMMUNICATION_STATUS_e returnedValue, uint8_t module) {
    N775_ErrorHandling(pState, returnedValue, module);
}
extern void TEST_N775_IncrementMuxIndex(N775_STATE_s *pState) {
    N775_IncrementMuxIndex(pState);
}
extern void TEST_N775_IncrementStringSequence(N775_STATE_s *pState) {
    N775_IncrementStringSequence(pState);
}
extern void TEST_N775_Initialize(N775_STATE_s *pState) {
    N775_Initialize(pState);
}
extern void TEST_N775_InitializeDatabase(N775_STATE_s *pState) {
    N775_InitializeDatabase(pState);
}
extern void TEST_N775_InitializeI2c(N775_STATE_s *pState) {
    N775_InitializeI2c(pState);
}
extern void TEST_N775_ResetStringSequence(N775_STATE_s *pState) {
    N775_ResetStringSequence(pState);
}
extern void TEST_N775_ResetMuxIndex(N775_STATE_s *pState) {
    N775_ResetMuxIndex(pState);
}
extern void TEST_N775_SetFirstMeasurementCycleFinished(N775_STATE_s *pState) {
    N775_SetFirstMeasurementCycleFinished(pState);
}
extern STD_RETURN_TYPE_e TEST_N775_SetMuxChannel(N775_STATE_s *pState) {
    return N775_SetMuxChannel(pState);
}
extern void TEST_N775_StartMeasurement(N775_STATE_s *pState) {
    N775_StartMeasurement(pState);
}
extern STD_RETURN_TYPE_e TEST_N775_TransmitI2c(N775_STATE_s *pState) {
    return N775_TransmitI2c(pState);
}
extern void TEST_N775_Wait(uint32_t milliseconds) {
    N775_Wait(milliseconds);
}
#endif
