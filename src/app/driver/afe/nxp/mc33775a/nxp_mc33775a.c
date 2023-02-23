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
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
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

#include "HL_gio.h"
#include "HL_system.h"
#pragma diag_push
#pragma diag_suppress 232
#include "MC33775A.h"
#pragma diag_pop

#include "afe.h"
#include "afe_dma.h"
#include "database.h"
#include "diag.h"
#include "fassert.h"
#include "fstd_types.h"
#include "ftask.h"
#include "io.h"
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
static N775_ERRORTABLE_s n775_errorTable        = {0};

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
static void N775_SetFirstMeasurementCycleFinished(N775_STATE_s *n775_state);
static void N775_InitializeDatabase(N775_STATE_s *n775_state);
static void N775_ResetStringSequence(N775_STATE_s *n775_state);
static void N775_IncrementStringSequence(N775_STATE_s *n775_state);
static void N775_ResetMuxIndex(N775_STATE_s *n775_state);
static void N775_IncrementMuxIndex(N775_STATE_s *n775_state);
static void N775_ErrorHandling(N775_STATE_s *n775_state, N775_COMMUNICATION_STATUS_e returnedValue, uint8_t module);
static void N775_Init(N775_STATE_s *n775_state);
static STD_RETURN_TYPE_e N775_Enumerate(N775_STATE_s *n775_state);
static void N775_I2cInit(N775_STATE_s *n775_state);
static void N775_StartMeasurement(N775_STATE_s *n775_state);
static void N775_CaptureMeasurement(N775_STATE_s *n775_state);
static STD_RETURN_TYPE_e N775_TransmitI2c(N775_STATE_s *n775_state);
static STD_RETURN_TYPE_e N775_SetMuxChannel(N775_STATE_s *n775_state);
static void N775_BalanceSetup(N775_STATE_s *n775_state);
static void N775_BalanceControl(N775_STATE_s *n775_state);
static void N775_waitTime(uint32_t milliseconds);

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/

/**
 * @brief   in the database, initializes the fields related to the N775 driver.
 *
 * This function loops through all the N775-related data fields in the database
 * and sets them to 0. It should be called in the initialization or re-initialization
 * routine of the N775 driver.
 *
 * @param  n775_state  state of the N775A driver
 *
 */
static void N775_InitializeDatabase(N775_STATE_s *n775_state) {
    FAS_ASSERT(n775_state != NULL_PTR);
    uint16_t iterator = 0;

    for (uint8_t stringNumber = 0u; stringNumber < BS_NR_OF_STRINGS; stringNumber++) {
        n775_state->n775Data.cellVoltage->state                               = 0;
        n775_state->n775Data.minMax->minimumCellVoltage_mV[stringNumber]      = 0;
        n775_state->n775Data.minMax->maximumCellVoltage_mV[stringNumber]      = 0;
        n775_state->n775Data.minMax->nrModuleMinimumCellVoltage[stringNumber] = 0;
        n775_state->n775Data.minMax->nrModuleMaximumCellVoltage[stringNumber] = 0;
        n775_state->n775Data.minMax->nrCellMinimumCellVoltage[stringNumber]   = 0;
        n775_state->n775Data.minMax->nrCellMaximumCellVoltage[stringNumber]   = 0;
        for (iterator = 0u; iterator < BS_NR_OF_CELL_BLOCKS_PER_STRING; iterator++) {
            n775_state->n775Data.cellVoltage->cellVoltage_mV[stringNumber][iterator] = 0;
        }

        n775_state->n775Data.cellTemperature->state                           = 0;
        n775_state->n775Data.minMax->minimumTemperature_ddegC[stringNumber]   = 0;
        n775_state->n775Data.minMax->maximumTemperature_ddegC[stringNumber]   = 0;
        n775_state->n775Data.minMax->nrModuleMinimumTemperature[stringNumber] = 0;
        n775_state->n775Data.minMax->nrModuleMaximumTemperature[stringNumber] = 0;
        n775_state->n775Data.minMax->nrSensorMinimumTemperature[stringNumber] = 0;
        n775_state->n775Data.minMax->nrSensorMaximumTemperature[stringNumber] = 0;
        for (iterator = 0u; iterator < BS_NR_OF_TEMP_SENSORS_PER_STRING; iterator++) {
            n775_state->n775Data.cellTemperature->cellTemperature_ddegC[stringNumber][iterator] = 0;
        }

        for (iterator = 0u; iterator < BS_NR_OF_CELL_BLOCKS_PER_STRING; iterator++) {
            n775_state->n775Data.balancingControl->balancingState[stringNumber][iterator] = 0;
        }
        for (iterator = 0u; iterator < BS_NR_OF_MODULES_PER_STRING; iterator++) {
            n775_state->n775Data.errorTable->communicationOk[stringNumber][iterator]        = false;
            n775_state->n775Data.errorTable->noCommunicationTimeout[stringNumber][iterator] = false;
            n775_state->n775Data.errorTable->crcIsValid[stringNumber][iterator]             = false;
            n775_state->n775Data.errorTable->mux0IsOk[stringNumber][iterator]               = false;
            n775_state->n775Data.errorTable->mux1IsOK[stringNumber][iterator]               = false;
            n775_state->n775Data.errorTable->mux2IsOK[stringNumber][iterator]               = false;
            n775_state->n775Data.errorTable->mux3IsOK[stringNumber][iterator]               = false;
        }
        for (iterator = 0u; iterator < BS_NR_OF_MODULES_PER_STRING; iterator++) {
            n775_state->n775Data.uid[stringNumber][iterator] = 0;
        }
    }

    DATA_WRITE_DATA(n775_state->n775Data.cellVoltage);
    DATA_WRITE_DATA(n775_state->n775Data.cellTemperature);
    DATA_WRITE_DATA(n775_state->n775Data.minMax);
    DATA_WRITE_DATA(n775_state->n775Data.balancingControl);
}

void N775_Meas(N775_STATE_s *n775_state) {
    FAS_ASSERT(n775_state != NULL_PTR);

    N775_InitializeDatabase(n775_state);
    /* Initialize SPI sequence pointers */
    n775_state->pSpiTxSequenceStart = spi_nxp775InterfaceTx;
    n775_state->pSpiRxSequenceStart = spi_nxp775InterfaceRx;

    /* Initialize each string */
    N775_ResetStringSequence(n775_state);
    while (n775_state->currentString < BS_NR_OF_STRINGS) {
        /* Initialize mux sequence pointers */
        n775_state->pMuxSequenceStart[n775_state->currentString] = n775_muxSequence;
        N775_Init(n775_state);
        N775_IncrementStringSequence(n775_state);
    }

    while (1) {
        N775_ResetStringSequence(n775_state);

        while (n775_state->currentString < BS_NR_OF_STRINGS) {
            if (N775_USE_MUX_FOR_TEMP == true) {
                /* Set mux channel according to mux sequence */
                N775_SetMuxChannel(n775_state);
            }
            N775_CaptureMeasurement(n775_state);
            if (N775_USE_MUX_FOR_TEMP == true) {
                /* Update index in mux sequence */
                N775_IncrementMuxIndex(n775_state);
            }
            N775_BalanceControl(n775_state);

            N775_TransmitI2c(n775_state);

            for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
                if (N775_IsFirstMeasurementCycleFinished(n775_state) == true) {
                    if (n775_state->n775Data.errorTable->noCommunicationTimeout[n775_state->currentString][m] == 0u) {
                        /* Daisy-chain does not answer: initialize it again */
                        n775_state->pMuxSequenceStart[n775_state->currentString] = n775_muxSequence;
                        N775_Init(n775_state);
                        /* Daisy-chain re-initialized: no need to check further slaves */
                        break;
                    }
                }
            }

            N775_IncrementStringSequence(n775_state);
        }

        /* Set flag when all strings have been measured */
        if ((n775_state->currentString == BS_NR_OF_STRINGS) &&
            (N775_IsFirstMeasurementCycleFinished(n775_state) == 0u)) {
            N775_SetFirstMeasurementCycleFinished(n775_state);
        }
    }
}

STD_RETURN_TYPE_e N775_I2cWrite(uint8_t module, uint8_t deviceAddress, uint8_t *pData, uint8_t dataLength) {
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
        /* wating for transfer to finish */
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

STD_RETURN_TYPE_e N775_I2cWriteRead(
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
        /* wating for transfer to finish */
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

STD_RETURN_TYPE_e N775_I2cRead(uint8_t module, uint8_t deviceAddress, uint8_t *pData, uint8_t dataLength) {
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
        /* wating for transfer to finish */
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

/**
 * @brief   reset index in string sequence.
 *
 * @param n775_state    state of the N775A driver
 *
 */
static void N775_ResetStringSequence(N775_STATE_s *n775_state) {
    FAS_ASSERT(n775_state != NULL_PTR);

    n775_state->currentString  = 0u;
    n775_state->pSpiTxSequence = n775_state->pSpiTxSequenceStart + n775_state->currentString;
    n775_state->pSpiRxSequence = n775_state->pSpiRxSequenceStart + n775_state->currentString;
}

/**
 * @brief   updates index in string sequence.
 *
 * @param n775_state    state of the N775A driver
 *
 */
static void N775_IncrementStringSequence(N775_STATE_s *n775_state) {
    FAS_ASSERT(n775_state != NULL_PTR);

    n775_state->currentString++;
    n775_state->pSpiTxSequence = n775_state->pSpiTxSequenceStart + n775_state->currentString;
    n775_state->pSpiRxSequence = n775_state->pSpiRxSequenceStart + n775_state->currentString;
}

/**
 * @brief   resets index in mux sequence.
 *
 * @param n775_state    state of the N775A driver
 *
 */
static void N775_ResetMuxIndex(N775_STATE_s *n775_state) {
    FAS_ASSERT(n775_state != NULL_PTR);

    n775_state->currentMux[n775_state->currentString]   = 0u;
    n775_state->pMuxSequence[n775_state->currentString] = n775_state->pMuxSequenceStart[n775_state->currentString];
}

/**
 * @brief   updates index in mux sequence.
 *
 * @param n775_state    state of the N775A driver
 *
 */
static void N775_IncrementMuxIndex(N775_STATE_s *n775_state) {
    FAS_ASSERT(n775_state != NULL_PTR);

    n775_state->currentMux[n775_state->currentString]++;
    if (n775_state->currentMux[n775_state->currentString] >= N775_MUX_SEQUENCE_LENGTH) {
        n775_state->currentMux[n775_state->currentString] = 0u;
    }
    n775_state->pMuxSequence[n775_state->currentString] = n775_state->pMuxSequenceStart[n775_state->currentString] +
                                                          n775_state->currentMux[n775_state->currentString];
}

/**
 * @brief   handles error when doing measurements.
 *
 * This function is used in the measurement function. It sets the errors flags in
 * the error table according to the value returned by the communication function.
 *
 * @param n775_state    state of the N775A driver
 * @param returnedValue status of the low-level communication
 * @param module        number of module addressed
 *
 */
static void N775_ErrorHandling(N775_STATE_s *n775_state, N775_COMMUNICATION_STATUS_e returnedValue, uint8_t module) {
    FAS_ASSERT(n775_state != NULL_PTR);

    if (returnedValue == N775_COMMUNICATION_OK) {
        n775_state->n775Data.errorTable->communicationOk[n775_state->currentString][module]        = true;
        n775_state->n775Data.errorTable->noCommunicationTimeout[n775_state->currentString][module] = true;
        n775_state->n775Data.errorTable->crcIsValid[n775_state->currentString][module]             = true;
    } else {
        n775_state->n775Data.errorTable->communicationOk[n775_state->currentString][module] = false;
        switch (returnedValue) {
            case N775_COMMUNICATION_ERROR_TIMEOUT:
                n775_state->n775Data.errorTable->noCommunicationTimeout[n775_state->currentString][module] = false;
                break;
            case N775_COMMUNICATION_ERROR_WRONG_CRC:
                n775_state->n775Data.errorTable->crcIsValid[n775_state->currentString][module] = false;
                break;
            default:
                n775_state->n775Data.errorTable->communicationOk[n775_state->currentString][module] = false;
                break;
        }
    }
}

/**
 * @brief   initializes the N775 driver.
 *
 * This function enumerates the slaves and starts the measurement.
 *
 * @param  n775_state  state of the N775A driver
 *
 */
static void N775_Init(N775_STATE_s *n775_state) {
    FAS_ASSERT(n775_state != NULL_PTR);

    /* Reset mux sequence */
    N775_ResetMuxIndex(n775_state);

    /* Initialize daisy-chain */
    if (STD_OK != N775_Enumerate(n775_state)) {
        /* error handling */}
        N775_StartMeasurement(n775_state);
        N775_I2cInit(n775_state);
        N775_BalanceSetup(n775_state);
}

/**
 * @brief   enumerates the N775 slaves.
 *
 * This function gives the slaves in the daisy-chain an address.
 *
 * @param  n775_state  state of the N775A driver
 *
 */
static STD_RETURN_TYPE_e N775_Enumerate(N775_STATE_s *n775_state) {
    FAS_ASSERT(n775_state != NULL_PTR);
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
            n775_state->pSpiTxSequence);
        N775_waitTime(1u);

        /* Wake up slave */
        returnedValue = N775_CommunicationRead(i, MC33775_SYS_COM_CFG_OFFSET, &readValue, n775_state);
        /* If slave is not enumerated */
        if (returnedValue != N775_COMMUNICATION_OK) {
            /* Wait until the slave has woken up */
            N775_waitTime(N775_WAKEUP_TIME_MS);

            returnedValue = N775_CommunicationRead(i, MC33775_SYS_COM_CFG_OFFSET, &readValue, n775_state);
            /* If slave is not enumerated */
            if (returnedValue != N775_COMMUNICATION_OK) {
                /* Enumerate slave */
                N775_CommunicationWrite(
                    (0u << 6u) + 0u,
                    MC33775_SYS_COM_CFG_OFFSET,
                    i + (N775_DEFAULT_CHAIN_ADDRESS << 6) +
                        (MC33775_SYS_COM_CFG_BUSFW_ENABLED_ENUM_VAL << MC33775_SYS_COM_CFG_BUSFW_POS),
                    n775_state->pSpiTxSequence);
            }

            /* Reset the message counter of the driver */
            N775_ResetMessageCounter((N775_DEFAULT_CHAIN_ADDRESS << 6) + i, n775_state->currentString);
            /* Check that the device has been enumerated */
            returnedValue = N775_CommunicationRead(i, MC33775_SYS_VERSION_OFFSET, &readValue, n775_state);
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
            n775_state->pSpiTxSequence);

        /* read uid of each device */
        returnedValue = N775_CommunicationReadMultiple(i, 3u, 3u, MC33775_SYS_UID_LOW_OFFSET, uid, n775_state);
        if (returnedValue == N775_COMMUNICATION_OK) {
            n775_state->n775Data.uid[n775_state->currentString][i - 1u] = 0u;
            for (uint8_t j = 0u; j <= 3u; j++) {
                n775_state->n775Data.uid[n775_state->currentString][i - 1u] |= ((uint64_t)uid[j]) << (16u * j);
            }
        }
    }

    return retVal;
}

/**
 * @brief   init I2C for the N775 slaves.
 *
 * This function makes slaves ready for I2C transactions with on-slave devices.
 *
 * @param  n775_state  state of the N775A driver
 *
 */
static void N775_I2cInit(N775_STATE_s *n775_state) {
    FAS_ASSERT(n775_state != NULL_PTR);

    /* Enable the I2C module and select 400 kHz */
    N775_CommunicationWrite(
        N775_BROADCAST_ADDRESS,
        MC33775_I2C_CFG_OFFSET,
        (MC33775_I2C_CFG_EN_ENABLED_ENUM_VAL << MC33775_I2C_CFG_EN_POS) +
            (MC33775_I2C_CFG_CLKSEL_F_400K_ENUM_VAL << MC33775_I2C_CFG_CLKSEL_POS),
        n775_state->pSpiTxSequence);
}

/**
 * @brief   starts the measurement.
 *
 * The MC33775A measures continuously. This function
 * starts the measurement.
 *
 * @param  n775_state  state of the N775A driver
 *
 */
static void N775_StartMeasurement(N775_STATE_s *n775_state) {
    FAS_ASSERT(n775_state != NULL_PTR);

    /* Enable cell voltage measurements */
    N775_CommunicationWrite(N775_BROADCAST_ADDRESS, MC33775_ALLM_VCVB_CFG_OFFSET, 0x3FFF, n775_state->pSpiTxSequence);
    /* Enable analog inputs 0-3 and module voltage measurement */
    N775_CommunicationWrite(N775_BROADCAST_ADDRESS, MC33775_PRMM_AIN_CFG_OFFSET, 0x1F, n775_state->pSpiTxSequence);
    /* Enable analog inputs 4-7 measurement */
    N775_CommunicationWrite(N775_BROADCAST_ADDRESS, MC33775_SECM_AIN_CFG_OFFSET, 0x0F, n775_state->pSpiTxSequence);
    /* Set pause of balancing before measurement start, enable the measurement units simultaneously */
    N775_CommunicationWrite(
        N775_BROADCAST_ADDRESS,
        MC33775_ALLM_CFG_OFFSET,
        (N775_BALPAUSELEN_10US << MC33775_ALLM_CFG_BALPAUSELEN_POS) | (1 << MC33775_ALLM_CFG_MEASEN_POS),
        n775_state->pSpiTxSequence);

    N775_waitTime(N775_TIME_AFTER_MEASUREMENT_START_MS);
}

/**
 * @brief   captures the measurement.
 *
 * The MC33775A measures continuously.
 * This function takes a snapshot on all slaves in the daisy-chain.
 *
 * @param  n775_state  state of the N775A driver
 *
 */
static void N775_CaptureMeasurement(N775_STATE_s *n775_state) {
    FAS_ASSERT(n775_state != NULL_PTR);

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
        n775_state->pSpiTxSequence);
    /* Wait for measurements to take place */
    N775_waitTime(N775_MEASUREMENT_CAPTURE_TIME_MS);
    /* Send capture command. This ends the last cycle and starts a new one */
    N775_CommunicationWrite(
        N775_BROADCAST_ADDRESS,
        MC33775_ALLM_APP_CTRL_OFFSET,
        (0u << MC33775_ALLM_APP_CTRL_PAUSEBAL_POS) | (0x3FFu << MC33775_PRMM_APP_CTRL_CAPVC_POS) |
            (MC33775_PRMM_APP_CTRL_VCOLNUM_DISABLED_ENUM_VAL << MC33775_PRMM_APP_CTRL_VCOLNUM_POS),
        n775_state->pSpiTxSequence);
    /* Wait for measurements to be ready */
    N775_waitTime(N775_MEASUREMENT_READY_TIME_MS);

    for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
        uint8_t deviceAddress = m + 1u;
        retValPrimary         = N775_CommunicationReadMultiple(
            deviceAddress, 20, 4u, MC33775_PRMM_APP_VC_CNT_OFFSET, primaryRawValues, n775_state);
        retValSecondary = N775_CommunicationReadMultiple(
            deviceAddress, 4, 4u, MC33775_SECM_APP_AIN4_OFFSET, secondaryRawValues, n775_state);
        if (N775_CHECK_SUPPLY_CURRENT == true) {
            retValSupplyCurrent =
                N775_CommunicationRead(deviceAddress, MC33775_SECM_PER_NPNISENSE_OFFSET, &currentRawValue, n775_state);
        }

        N775_ErrorHandling(n775_state, retValPrimary, m);
        if (retValPrimary == N775_COMMUNICATION_OK) {
            for (uint8_t c = 0u; c < BS_NR_OF_CELL_BLOCKS_PER_MODULE; c++) {
                /* Store cell voltages */
                if (N775_INVALID_REGISTER_VALUE != primaryRawValues[c + 1u]) {
                    primaryValues[c + 1u] = (int16_t)primaryRawValues[c + 1u];
                    n775_state->n775Data.cellVoltage
                        ->cellVoltage_mV[n775_state->currentString][c + (m * BS_NR_OF_CELL_BLOCKS_PER_MODULE)] =
                        (((float_t)primaryValues[c + 1u]) * 154.0e-6f * 1000.0f);
                } else {
                    error++;
                }
            }
            for (uint8_t g = 0u; g < 4u; g++) {
                /* Store GPIO voltages */
                if (N775_INVALID_REGISTER_VALUE != primaryRawValues[g + 16u]) {
                    primaryValues[g + 16u] = (int16_t)primaryRawValues[g + 16u];
                    n775_state->n775Data.allGpioVoltage
                        ->gpioVoltages_mV[n775_state->currentString][g + (m * BS_NR_OF_GPIOS_PER_MODULE)] =
                        (((float_t)primaryValues[g + 16u]) * 154.0e-6f * 1000.0f);
                } else {
                    gpio03Error = true;
                    error++;
                }
            }
            /* Store module voltage */
            if (N775_INVALID_REGISTER_VALUE != primaryRawValues[15u]) {
                primaryValues[15u] = (int16_t)primaryRawValues[15u];
                n775_state->n775Data.cellVoltage->moduleVoltage_mV[n775_state->currentString][m] =
                    (((float_t)primaryValues[15u]) * 2.58e-3f * 1000.0f);
            } else {
                error++;
            }
        }

        N775_ErrorHandling(n775_state, retValSecondary, m);
        if (retValSecondary == N775_COMMUNICATION_OK) {
            for (uint8_t g = 4u; g < 8u; g++) {
                if (N775_INVALID_REGISTER_VALUE != secondaryRawValues[g - 4u]) {
                    secondaryValues[g - 4u] = (int16_t)secondaryRawValues[g - 4u];
                    n775_state->n775Data.allGpioVoltage
                        ->gpioVoltages_mV[n775_state->currentString][g + (m * BS_NR_OF_GPIOS_PER_MODULE)] =
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
                n775_state->n775Data.cellTemperature
                    ->cellTemperature_ddegC[n775_state->currentString]
                                           [n775_state->currentMux[n775_state->currentString] +
                                            (m * BS_NR_OF_TEMP_SENSORS_PER_MODULE)] =
                    N775_ConvertVoltagesToTemperatures(
                        n775_state->n775Data.allGpioVoltage
                            ->gpioVoltages_mV[n775_state->currentString]
                                             [N775_MUXED_TEMP_GPIO_POSITION + (m * BS_NR_OF_GPIOS_PER_MODULE)]);
            }
        } else if (N775_USE_MUX_FOR_TEMP == false) {
            /* No  mux case */
            if ((gpio03Error == false) && (gpio47Error == false)) {
                for (uint16_t t = 0u; t < BS_NR_OF_TEMP_SENSORS_PER_MODULE; t++) {
                    n775_state->n775Data.cellTemperature
                        ->cellTemperature_ddegC[n775_state->currentString][t + (m * BS_NR_OF_TEMP_SENSORS_PER_MODULE)] =
                        N775_ConvertVoltagesToTemperatures(
                            n775_state->n775Data.allGpioVoltage
                                ->gpioVoltages_mV[n775_state->currentString][t + (m * BS_NR_OF_GPIOS_PER_MODULE)]);
                }
            }
        } else {
            /* Invalid value for switch case */
            FAS_ASSERT(FAS_TRAP);
        }

        if (N775_CHECK_SUPPLY_CURRENT == true) {
            N775_ErrorHandling(n775_state, retValSupplyCurrent, m);
            if (retValSupplyCurrent == N775_COMMUNICATION_OK) {
                if (N775_INVALID_REGISTER_VALUE != currentRawValue) {
                    currentValue = (int16_t)currentRawValue;
                    n775_state->n775Data.supplyCurrent->current[n775_state->currentString][m] =
                        (((float_t)currentValue) * 7.69e-6f * 1000.0f);
                } else {
                    error++;
                }
            }
        }
    }

    DATA_WRITE_DATA(
        n775_state->n775Data.cellVoltage, n775_state->n775Data.cellTemperature, n775_state->n775Data.allGpioVoltage);
}

/**
 * @brief   tranmit over I2C on NXP slave.
 *
 * @param  n775_state       state of the N775A driver
 *
 */
static STD_RETURN_TYPE_e N775_TransmitI2c(N775_STATE_s *n775_state) {
    FAS_ASSERT(n775_state != NULL_PTR);
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
                        n775_state->pSpiTxSequence);

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
                                    n775_state->pSpiTxSequence);
                                registerIncrement++;
                                bytesWritten = 0u;
                                dataToWrite  = 0u;
                            }
                        }
                    }
                    /* Data to write ready, now start transmisison */
                    /* Write into the control register to start transaction */
                    N775_CommunicationWrite(
                        transactionData.module + 1u,
                        MC33775_I2C_CTRL_OFFSET,
                        /* transactionData.writeDataLength + 1u: data + I2C device address byte */
                        ((transactionData.writeDataLength + 1u) << MC33775_I2C_CTRL_START_POS) |
                            ((MC33775_I2C_CTRL_STPAFTER_STOP_ENUM_VAL << MC33775_I2C_CTRL_STPAFTER_POS) +
                             (0u << MC33775_I2C_CTRL_RDAFTER_POS)),
                        n775_state->pSpiTxSequence);
                    /* Wait until transaction ends */
                    tries = N775_FLAG_READY_TRIES;
                    do {
                        returnedValue = N775_CommunicationRead(
                            transactionData.module + 1u, MC33775_I2C_STAT_OFFSET, &readValue, n775_state);
                        tries--;
                        N775_waitTime(2u);
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
                        n775_state->pSpiTxSequence);
                    /* Write into the control register to start transaction */
                    /* Stop condition after transfer, no repeated start */
                    N775_CommunicationWrite(
                        transactionData.module + 1u,
                        MC33775_I2C_CTRL_OFFSET,
                        /* transactionData.dataLength + 1u: data + I2C device address byte */
                        ((transactionData.readDataLength + 1u) << MC33775_I2C_CTRL_START_POS) |
                            ((1u << MC33775_I2C_CTRL_STPAFTER_POS) + (0u << MC33775_I2C_CTRL_RDAFTER_POS)),
                        n775_state->pSpiTxSequence);
                    /* Wait until transaction ends */
                    tries = N775_FLAG_READY_TRIES;
                    do {
                        returnedValue = N775_CommunicationRead(
                            transactionData.module + 1u, MC33775_I2C_STAT_OFFSET, &readValue, n775_state);
                        tries--;
                        N775_waitTime(2u);
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
                            n775_state);

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
                        n775_state->pSpiTxSequence);

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
                                    n775_state->pSpiTxSequence);
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
                            n775_state->pSpiTxSequence);
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
                        n775_state->pSpiTxSequence);
                    /* Wait until transaction ends */
                    tries = N775_FLAG_READY_TRIES;
                    do {
                        returnedValue = N775_CommunicationRead(
                            transactionData.module + 1u, MC33775_I2C_STAT_OFFSET, &readValue, n775_state);
                        tries--;
                        N775_waitTime(2u);
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
                            transactionData.module + 1u, nrOfRegisterToRead, 4u, registerOffset, readData, n775_state);

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

/**
 * @brief   sets mux channel.
 *
 * This function uses I2C to set the mux channel.
 *
 * @param  n775_state  state of the N775A driver
 *
 */
static STD_RETURN_TYPE_e N775_SetMuxChannel(N775_STATE_s *n775_state) {
    FAS_ASSERT(n775_state != NULL_PTR);
    FAS_ASSERT(n775_state->pMuxSequence[n775_state->currentString]->muxId < 4u);
    FAS_ASSERT(n775_state->pMuxSequence[n775_state->currentString]->muxChannel <= 0xFFu);

    uint16_t readValue                        = 0u;
    uint8_t dataI2c                           = 0u;
    uint8_t addressI2c_write                  = N775_ADG728_ADDRESS_UPPERBITS;
    uint8_t addressI2c_read                   = N775_ADG728_ADDRESS_UPPERBITS;
    uint16_t tries                            = 0u;
    STD_RETURN_TYPE_e retVAL                  = STD_OK;
    N775_COMMUNICATION_STATUS_e returnedValue = N775_COMMUNICATION_OK;

    /* First set channel */

    /* Set bit1 and bit0 with mux address, write to mux */
    addressI2c_write |= ((n775_state->pMuxSequence[n775_state->currentString]->muxId) << 1u) | N775_I2C_WRITE;
    /* Set bit1 and bit0 with mux address, read from mux */
    addressI2c_read |= ((n775_state->pMuxSequence[n775_state->currentString]->muxId) << 1u) | N775_I2C_READ;

    /**
     * Set data to send, contains channel bit (8 channels)
     * 1 means channel active, 0 means channel inactive
     */
    if (n775_state->pMuxSequence[n775_state->currentString]->muxChannel == 0xFF) {
        /* 0xFF in mux sequence means disable all channels */
        dataI2c = 0u;
    } else {
        dataI2c = (uint8_t)(1u << (n775_state->pMuxSequence[n775_state->currentString]->muxChannel));
    }

    /* Write data to send on I2C bus in registers */
    N775_CommunicationWrite(
        N775_BROADCAST_ADDRESS,
        MC33775_I2C_DATA0_OFFSET,
        (addressI2c_write << MC33775_I2C_DATA0_BYTE0_POS) | (dataI2c << MC33775_I2C_DATA0_BYTE1_POS),
        n775_state->pSpiTxSequence);

    /* Read with a repeated start directly after write */
    N775_CommunicationWrite(
        N775_BROADCAST_ADDRESS,
        MC33775_I2C_DATA1_OFFSET,
        (addressI2c_read << MC33775_I2C_DATA1_BYTE2_POS) | (N775_I2C_DUMMY_BYTE << MC33775_I2C_DATA1_BYTE3_POS),
        n775_state->pSpiTxSequence);

    /* Write into the control register to start transaction */
    N775_CommunicationWrite(
        N775_BROADCAST_ADDRESS,
        MC33775_I2C_CTRL_OFFSET,
        (N775_I2C_NR_BYTES_FOR_MUX_WRITE << MC33775_I2C_CTRL_START_POS) |
            ((MC33775_I2C_CTRL_STPAFTER_STOP_ENUM_VAL << MC33775_I2C_CTRL_STPAFTER_POS) +
             (N775_I2C_NR_BYTES_TO_SWITCH_TO_READ_FOR_UX_READ << MC33775_I2C_CTRL_RDAFTER_POS)),
        n775_state->pSpiTxSequence);

    /**
     * Wait util transaction ends, test on last device in daisy-chain
     * So device address = number of modules
     */
    tries = N775_FLAG_READY_TRIES;
    do {
        returnedValue =
            N775_CommunicationRead(BS_NR_OF_MODULES_PER_STRING, MC33775_I2C_STAT_OFFSET, &readValue, n775_state);
        tries--;
        N775_waitTime(2u);
    } while ((readValue & MC33775_I2C_STAT_PENDING_MSK) && (returnedValue == N775_COMMUNICATION_OK) && (tries > 0u));

    if ((returnedValue == N775_COMMUNICATION_OK) && (tries > 0u)) {
        /**
         *  Get I2C read data, on last device in daisy-chain
         *  Use result to set error state for all slaves to avoid
         *  reading all slaves in daisy-chain.
         */
        returnedValue =
            N775_CommunicationRead(BS_NR_OF_MODULES_PER_STRING, MC33775_I2C_DATA1_OFFSET, &readValue, n775_state);
        readValue = readValue >> MC33775_I2C_DATA1_BYTE3_POS;

        if (returnedValue == N775_COMMUNICATION_OK) {
            if (N775_CHECK_MUX_STATE == true) {
                if (readValue == dataI2c) {
                    /* OK */
                    for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
                        n775_state->n775Data.errorTable->mux0IsOk[n775_state->currentString][m] = true;
                    }
                } else {
                    /* Not OK */
                    for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
                        n775_state->n775Data.errorTable->mux0IsOk[n775_state->currentString][m] = false;
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

/**
 * @brief   setups balancing.
 *
 * Sets all balancing timer to max to allow for software balancing control.
 *
 * @param  n775_state  state of the N775A driver
 *
 */
static void N775_BalanceSetup(N775_STATE_s *n775_state) {
    FAS_ASSERT(n775_state != NULL_PTR);

    /* Set global timeout counter to max value */
    N775_CommunicationWrite(
        N775_BROADCAST_ADDRESS,
        MC33775_BAL_GLOB_TO_TMR_OFFSET,
        N775_GLOBAL_BALANCING_TIMER,
        n775_state->pSpiTxSequence);

    /* Disable pre-balancing timer by setting it to zero */
    N775_CommunicationWrite(
        N775_BROADCAST_ADDRESS, MC33775_BAL_PRE_TMR_OFFSET, N775_PRE_BALANCING_TIMER, n775_state->pSpiTxSequence);

    /* Set PWM value for all channels to 100%, set balancing timer for all channels to maximum value */
    N775_CommunicationWrite(
        N775_BROADCAST_ADDRESS,
        MC33775_BAL_TMR_CH_ALL_OFFSET,
        (MC33775_BAL_TMR_CH_ALL_PWM_PWM100_ENUM_VAL << MC33775_BAL_TMR_CH_ALL_PWM_POS) |
            (N775_ALL_CHANNEL_BALANCING_TIMER << MC33775_BAL_TMR_CH_ALL_BALTIME_POS),
        n775_state->pSpiTxSequence);

    /* Select timer based balancing and enable balancing */
    N775_CommunicationWrite(
        N775_BROADCAST_ADDRESS,
        MC33775_BAL_GLOB_CFG_OFFSET,
        (MC33775_BAL_GLOB_CFG_BALEN_ENABLED_ENUM_VAL << MC33775_BAL_GLOB_CFG_BALEN_POS) |
            (MC33775_BAL_GLOB_CFG_TMRBALEN_STOP_ENUM_VAL << MC33775_BAL_GLOB_CFG_TMRBALEN_POS),
        n775_state->pSpiTxSequence);
}

/**
 * @brief   manages balancing.
 *
 * Reads balancing order from database and balances the corresponding cells.
 *
 * @param  n775_state  state of the N775A driver
 *
 */
static void N775_BalanceControl(N775_STATE_s *n775_state) {
    FAS_ASSERT(n775_state != NULL_PTR);

    N775_BalanceSetup(n775_state);

    DATA_READ_DATA(n775_state->n775Data.balancingControl);

    for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
        uint8_t deviceAddress   = m + 1u;
        uint16_t balancingState = 0u;
        for (uint16_t c = 0u; c < BS_NR_OF_CELL_BLOCKS_PER_MODULE; c++) {
            if (n775_state->n775Data.balancingControl
                    ->balancingState[n775_state->currentString][c + (m * BS_NR_OF_CELL_BLOCKS_PER_MODULE)] != 0u) {
                balancingState |= 1u << c;
            }
        }
        /* All channels active --> 14 bits set to 1 --> 0x3FFF */
        FAS_ASSERT(balancingState <= 0x3FFFu);
        /* Enable channels, one written to a channels means balancing active */
        N775_CommunicationWrite(deviceAddress, MC33775_BAL_CH_CFG_OFFSET, balancingState, n775_state->pSpiTxSequence);
    }
}

extern bool N775_IsFirstMeasurementCycleFinished(N775_STATE_s *n775_state) {
    FAS_ASSERT(n775_state != NULL_PTR);
    bool retval = false;

    OS_EnterTaskCritical();
    retval = n775_state->firstMeasurementMade;
    OS_ExitTaskCritical();

    return retval;
}

/**
 * @brief   sets the measurement initialization status.
 *
 * @param  n775_state  state of the N775A driver
 *
 */
static void N775_SetFirstMeasurementCycleFinished(N775_STATE_s *n775_state) {
    FAS_ASSERT(n775_state != NULL_PTR);

    OS_EnterTaskCritical();
    n775_state->firstMeasurementMade = true;
    OS_ExitTaskCritical();
}

extern void TEST_N775_SetFirstMeasurementCycleFinished(N775_STATE_s *n775_state) {
    N775_SetFirstMeasurementCycleFinished(n775_state);
}

/**
 * @brief   waits for a definite amount of time in ms.
 *
 * This function uses FreeRTOS. It blocks the tasks for
 * the given amount of milliseconds.
 *
 * @param  milliseconds  time to wait in ms
 *
 */
void N775_waitTime(uint32_t milliseconds) {
    uint32_t current_time = OS_GetTickCount();
    /* Block task without possibility to wake up */
    OS_DelayTaskUntil(&current_time, milliseconds);
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
