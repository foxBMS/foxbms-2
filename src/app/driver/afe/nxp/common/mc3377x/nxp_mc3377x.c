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
 * @file    nxp_mc3377x.c
 * @author  foxBMS Team
 * @date    2020-05-08 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup DRIVERS
 * @prefix  N77X
 *
 * @brief   Driver for the MC3377X analog front-end.
 * @details TODO
 */

/*========== Includes =======================================================*/
#include "nxp_mc3377x.h"
/* clang-format off */
#include "nxp_mc3377x-ll.h"
/* clang-format on */
#include "afe.h"
#include "afe_dma.h"
#include "database.h"
#include "fassert.h"
#include "fstd_types.h"
#include "mcu.h"
#include "nxp_mc3377x_balancing.h"
#include "nxp_mc3377x_database.h"
#include "nxp_mc3377x_helpers.h"
#include "nxp_mc3377x_i2c.h"
#include "nxp_mc3377x_measurement.h"
#include "nxp_mc3377x_mux.h"
#include "nxp_mc3377x_reg_def.h"
#include "os.h"
#include "spi.h"

#include <math.h>
#include <stdbool.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/
/** local copies of database tables */
/**@{*/
static DATA_BLOCK_CELL_VOLTAGE_s n77x_cellVoltage           = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE_BASE};
static DATA_BLOCK_CELL_TEMPERATURE_s n77x_cellTemperature   = {.header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE_BASE};
static DATA_BLOCK_MIN_MAX_s n77x_minMax                     = {.header.uniqueId = DATA_BLOCK_ID_MIN_MAX};
static DATA_BLOCK_BALANCING_CONTROL_s n77x_balancingControl = {.header.uniqueId = DATA_BLOCK_ID_BALANCING_CONTROL};
static DATA_BLOCK_ALL_GPIO_VOLTAGES_s n77x_allGpioVoltage   = {.header.uniqueId = DATA_BLOCK_ID_ALL_GPIO_VOLTAGES_BASE};
static DATA_BLOCK_BALANCING_FEEDBACK_s n77x_balancingFeedback = {
    .header.uniqueId = DATA_BLOCK_ID_BALANCING_FEEDBACK_BASE};
static DATA_BLOCK_SLAVE_CONTROL_s n77x_slaveControl = {.header.uniqueId = DATA_BLOCK_ID_SLAVE_CONTROL};
static DATA_BLOCK_OPEN_WIRE_s n77x_openWire         = {.header.uniqueId = DATA_BLOCK_ID_OPEN_WIRE_BASE};
/**@}*/
static N77X_SUPPLY_CURRENT_s n77x_supplyCurrent = {0};
static N77X_ERROR_TABLE_s n77x_errorTable       = {0};

/*========== Extern Constant and Variable Definitions =======================*/

N77X_STATE_s n77x_stateBase = {
    .firstMeasurementMade       = false,
    .currentString              = 0u,
    .pSpiTxSequenceStart        = NULL_PTR,
    .pSpiTxSequence             = NULL_PTR,
    .pSpiRxSequenceStart        = NULL_PTR,
    .pSpiRxSequence             = NULL_PTR,
    .currentMux                 = {0},
    .pMuxSequenceStart          = {0},
    .pMuxSequence               = {0},
    .n77xData.cellVoltage       = &n77x_cellVoltage,
    .n77xData.cellTemperature   = &n77x_cellTemperature,
    .n77xData.allGpioVoltage    = &n77x_allGpioVoltage,
    .n77xData.minMax            = &n77x_minMax,
    .n77xData.balancingFeedback = &n77x_balancingFeedback,
    .n77xData.balancingControl  = &n77x_balancingControl,
    .n77xData.slaveControl      = &n77x_slaveControl,
    .n77xData.openWire          = &n77x_openWire,
    .n77xData.supplyCurrent     = &n77x_supplyCurrent,
    .n77xData.errorTable        = &n77x_errorTable,
};

/*========== Static Function Prototypes =====================================*/
/**
 * @brief   enumerates the N77X slaves.
 * @details This function gives the slaves in the daisy-chain an address.
 * @param   pState state of the NXP MC3377X driver
 */
static STD_RETURN_TYPE_e N77x_Enumerate(N77X_STATE_s *pState);

/**
 * @brief   initializes the N77X driver.
 * @details This function enumerates the slaves and starts the measurement.
 * @param   pState state of the NXP MC3377X driver
 */
static void N77x_Initialize(N77X_STATE_s *pState);

/**
 * @brief   sets the measurement initialization status.
 * @param   pState state of the NXP MC3377X driver
 */
static void N77x_SetFirstMeasurementCycleFinished(N77X_STATE_s *pState);

/**
 * @brief   increments index in string sequence.
 * @param   pState state of the NXP MC3377X driver
 */
static void N77x_IncrementStringSequence(N77X_STATE_s *pState);

/**
 * @brief   reset index in string sequence.
 * @param   pState state of the NXP MC3377X driver
 */
static void N77x_ResetStringSequence(N77X_STATE_s *pState);

/*========== Static Function Implementations ================================*/

static STD_RETURN_TYPE_e N77x_Enumerate(N77X_STATE_s *pState) {
    FAS_ASSERT(pState != NULL_PTR);
    uint16_t readValue                        = 0u;
    uint16_t uid[3u]                          = {0};
    N77X_COMMUNICATION_STATUS_e returnedValue = N77X_COMMUNICATION_OK;
    STD_RETURN_TYPE_e retVal                  = STD_NOT_OK;

    /** Parse all slaves in the daisy-chain */
    for (uint8_t i = 1; i <= BS_NR_OF_MODULES_PER_STRING; i++) {
        /* First send slave to deep sleep to reset message counter */
        N77x_CommunicationWrite(
            i,
            MC3377X_SYS_MODE_OFFSET,
            (MC3377X_SYS_MODE_TARGETMODE_DEEPSLEEP_ENUM_VAL << MC3377X_SYS_MODE_TARGETMODE_POS),
            pState->pSpiTxSequence);
        N77x_Wait(N77X_T_SW_ACT_DEEP_SLEEP_MS);

        /* Wake up slave */
        returnedValue = N77x_CommunicationRead(i, MC3377X_SYS_COM_CFG_OFFSET, &readValue, pState);
        /* If slave is not enumerated */
        if (returnedValue != N77X_COMMUNICATION_OK) {
            /* Wait until the slave has woken up */
            N77x_Wait(N77X_T_WAKE_COM_MS);

            returnedValue = N77x_CommunicationRead(i, MC3377X_SYS_COM_CFG_OFFSET, &readValue, pState);
            /* If slave is not enumerated */
            if (returnedValue != N77X_COMMUNICATION_OK) {
                /* Enumerate slave */
                N77x_CommunicationWrite(
                    (0u << 6u) + 0u,
                    MC3377X_SYS_COM_CFG_OFFSET,
                    i + (N77X_DEFAULT_CHAIN_ADDRESS << 6) +
                        (MC3377X_SYS_COM_CFG_BUSFW_ENABLED_ENUM_VAL << MC3377X_SYS_COM_CFG_BUSFW_POS),
                    pState->pSpiTxSequence);
            }

            /* Reset the message counter of the driver */
            N77x_ResetMessageCounter((N77X_DEFAULT_CHAIN_ADDRESS << 6) + i, pState->currentString);
            /* Check that the device has been enumerated */
            returnedValue = N77x_CommunicationRead(i, MC3377X_SYS_VERSION_OFFSET, &readValue, pState);
            if (returnedValue == N77X_COMMUNICATION_OK) {
                retVal = STD_OK;
            }
        } else {
            /* Slave already has an address */
            retVal = STD_OK;
        }

        /* Set timeout, enable/disable timeout */
        N77x_CommunicationWrite(
            i,
            MC3377X_SYS_COM_TO_CFG_OFFSET,
            (N77X_TIMEOUT_SWITCH << MC3377X_SYS_COM_TO_CFG_COMTODISABLE_POS) |
                (N77X_TIMEOUT_TO_SLEEP_10MS << MC3377X_SYS_COM_TO_CFG_COMTO_POS),
            pState->pSpiTxSequence);

        /* read uid of each device */
        returnedValue = N77x_CommunicationReadMultiple(i, 3u, 3u, MC3377X_SYS_UID_LOW_OFFSET, uid, pState);
        if (returnedValue == N77X_COMMUNICATION_OK) {
            pState->serialId[pState->currentString][i - 1u] = 0u;
            for (uint8_t j = 0u; j < 3u; j++) {
                pState->serialId[pState->currentString][i - 1u] |= ((uint64_t)uid[j]) << (16u * j);
            }
        }
    }

    return retVal;
}

static void N77x_Initialize(N77X_STATE_s *pState) {
    FAS_ASSERT(pState != NULL_PTR);

    /* Reset mux sequence */
    N77x_ResetMuxIndex(pState);

    /* Initialize daisy-chain */
    if (STD_OK != N77x_Enumerate(pState)) {
        /* error handling */}
        N77x_StartMeasurement(pState);
        N77x_InitializeI2c(pState);
        N77x_BalanceSetup(pState);
}

static void N77x_SetFirstMeasurementCycleFinished(N77X_STATE_s *pState) {
    FAS_ASSERT(pState != NULL_PTR);

    OS_EnterTaskCritical();
    pState->firstMeasurementMade = true;
    OS_ExitTaskCritical();
}

static void N77x_IncrementStringSequence(N77X_STATE_s *pState) {
    FAS_ASSERT(pState != NULL_PTR);

    pState->currentString++;
    pState->pSpiTxSequence = pState->pSpiTxSequenceStart + pState->currentString;
    pState->pSpiRxSequence = pState->pSpiRxSequenceStart + pState->currentString;
}

static void N77x_ResetStringSequence(N77X_STATE_s *pState) {
    FAS_ASSERT(pState != NULL_PTR);

    pState->currentString  = 0u;
    pState->pSpiTxSequence = pState->pSpiTxSequenceStart + pState->currentString;
    pState->pSpiRxSequence = pState->pSpiRxSequenceStart + pState->currentString;
}

/*========== Extern Function Implementations ================================*/
extern bool N77x_IsFirstMeasurementCycleFinished(N77X_STATE_s *pState) {
    FAS_ASSERT(pState != NULL_PTR);
    bool retval = false;

    OS_EnterTaskCritical();
    retval = pState->firstMeasurementMade;
    OS_ExitTaskCritical();

    return retval;
}

extern void N77x_Measure(N77X_STATE_s *pState) {
    FAS_ASSERT(pState != NULL_PTR);

    N77x_InitializeDatabase(pState);
    /* Initialize SPI sequence pointers */
    pState->pSpiTxSequenceStart = spi_nxp77xInterfaceTx;
    pState->pSpiRxSequenceStart = spi_nxp77xInterfaceRx;

    /* Initialize each string */
    N77x_ResetStringSequence(pState);
    while (pState->currentString < BS_NR_OF_STRINGS) {
        /* Initialize mux sequence pointers */
        pState->pMuxSequenceStart[pState->currentString] = n77x_muxSequence;
        N77x_Initialize(pState);
        N77x_IncrementStringSequence(pState);
    }

    while (1) {
        N77x_ResetStringSequence(pState);

        while (pState->currentString < BS_NR_OF_STRINGS) {
            if (N77X_USE_MUX_FOR_TEMP == true) {
                /* Set mux channel according to mux sequence */
                N77x_SetMuxChannel(pState);
            }
            N77x_CaptureMeasurement(pState);
            if (N77X_USE_MUX_FOR_TEMP == true) {
                /* Update index in mux sequence */
                N77x_IncrementMuxIndex(pState);
            }
            N77x_BalanceControl(pState);

            N77x_TransmitI2c(pState);

            for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
                if (N77x_IsFirstMeasurementCycleFinished(pState) == true) {
                    if (pState->n77xData.errorTable->noCommunicationTimeout[pState->currentString][m] == 0u) {
                        /* Daisy-chain does not answer: initialize it again */
                        pState->pMuxSequenceStart[pState->currentString] = n77x_muxSequence;
                        N77x_Initialize(pState);
                        /* Daisy-chain re-initialized: no need to check further slaves */
                        break;
                    }
                }
            }

            N77x_IncrementStringSequence(pState);
        }

        /* Set flag when all strings have been measured */
        if ((pState->currentString == BS_NR_OF_STRINGS) && (N77x_IsFirstMeasurementCycleFinished(pState) == 0u)) {
            N77x_SetFirstMeasurementCycleFinished(pState);
        }
    }
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
extern STD_RETURN_TYPE_e TEST_N77x_Enumerate(N77X_STATE_s *pState) {
    return N77x_Enumerate(pState);
}
extern void TEST_N77x_Initialize(N77X_STATE_s *pState) {
    N77x_Initialize(pState);
}
extern void TEST_N77x_SetFirstMeasurementCycleFinished(N77X_STATE_s *pState) {
    N77x_SetFirstMeasurementCycleFinished(pState);
}
extern void TEST_N77x_IncrementStringSequence(N77X_STATE_s *pState) {
    N77x_IncrementStringSequence(pState);
}
extern void TEST_N77x_ResetStringSequence(N77X_STATE_s *pState) {
    N77x_ResetStringSequence(pState);
}
#endif
