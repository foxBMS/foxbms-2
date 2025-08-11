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
 * @file    can_cbs_rx_current-sensor.c
 * @author  foxBMS Team
 * @date    2021-04-20 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup DRIVERS
 * @prefix  CANRX
 *
 * @brief   CAN driver Rx callback implementation
 * @details CAN Rx callback for current sensor measurements
 */

/*========== Includes =======================================================*/
/* AXIVION Next Codeline Generic-LocalInclude: 'can_cbs_rx.h' declares the
 * prototype for the callback 'CANRX_CurrentSensor' */
#include "can_cbs_rx.h"
#include "can_cfg_rx-message-definitions.h"
#include "can_helper.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/
#define CANRX_CURRENT_SENSOR_ID_START_BIT                (7u)
#define CANRX_CURRENT_SENSOR_ID_LENGTH                   (8u)
#define CANRX_CURRENT_SENSOR_SYSTEM_ERROR_START_BIT      (15u)
#define CANRX_CURRENT_SENSOR_SYSTEM_ERROR_LENGTH         (CAN_BIT)
#define CANRX_CURRENT_SENSOR_MEASUREMENT_ERROR_START_BIT (14u)
#define CANRX_CURRENT_SENSOR_MEASUREMENT_ERROR_LENGTH    (CAN_BIT)
#define CANRX_CURRENT_SENSOR_CHANNEL_ERROR_START_BIT     (13u)
#define CANRX_CURRENT_SENSOR_CHANNEL_ERROR_LENGTH        (CAN_BIT)
#define CANRX_CURRENT_SENSOR_OCS_START_BIT               (12u)
#define CANRX_CURRENT_SENSOR_OCS_LENGTH                  (CAN_BIT)
#define CANRX_CURRENT_SENSOR_MESSAGE_COUNT_START_BIT     (11u)
#define CANRX_CURRENT_SENSOR_MESSAGE_COUNT_LENGTH        (4u)
#define CANRX_CURRENT_SENSOR_DATA_START_BIT              (23u)
#define CANRX_CURRENT_SENSOR_DATA_LENGTH                 (32u)

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/
/**
 * @brief   Sets the error states in the CAN data shim
 * @param   kpkCanShim   shim to the database entries
 * @param   messageId    message ID to be handled
 * @param   stringNumber addressed string
 */
static void CANRX_HandleChannelError(const CAN_SHIM_s *const kpkCanShim, uint32_t messageId, uint8_t stringNumber);

/**
 * @brief   Resets all error states in the CAN data shim to valid
 * @param   kpkCanShim   shim to the database entries
 * @param   stringNumber addressed string
 */
static void CANRX_ResetError(const CAN_SHIM_s *const kpkCanShim, uint8_t stringNumber);

/**
 * @brief   Sets all error states in the CAN data shim to invalid
 * @param   kpkCanShim   shim to the database entries
 * @param   stringNumber addressed string
 */
static void CANRX_HandleMeasurementError(const CAN_SHIM_s *const kpkCanShim, uint8_t stringNumber);

/**
 * @brief   Sets all error states in the CAN data shim to invalid
 * @param   kpkCanShim   shim to the database entries
 * @param   stringNumber addressed string
 */
static void CANRX_HandleSystemError(const CAN_SHIM_s *const kpkCanShim, uint8_t stringNumber);

/**
 * @brief   Sets current value in the CAN data shim
 * @param   kpkCanShim   shim to the database entries
 * @param   messageId    message ID to be handled
 * @param   stringNumber addressed string
 * @param   signalData   actual signal data
 */
static void CANRX_HandleSensorData(
    const CAN_SHIM_s *const kpkCanShim,
    uint32_t messageId,
    uint8_t stringNumber,
    int32_t signalData);

/**
 * @brief   Sets current value in the CAN data shim
 * @param   kpkCanShim   shim to the database entries
 * @param   stringNumber addressed string
 * @param   signalData   actual signal data
 */
static void CANRX_SetCurrent(const CAN_SHIM_s *const kpkCanShim, uint8_t stringNumber, int32_t signalData);

/**
 * @brief   Sets V1 voltage measurement value in the CAN data shim
 * @param   kpkCanShim   shim to the database entries
 * @param   stringNumber addressed string
 * @param   signalData   actual signal data
 */
static void CANRX_SetVoltageU1(const CAN_SHIM_s *const kpkCanShim, uint8_t stringNumber, int32_t signalData);

/**
 * @brief   Sets V2 voltage measurement value in the CAN data shim
 * @param   kpkCanShim   shim to the database entries
 * @param   stringNumber addressed string
 * @param   signalData   actual signal data
 */
static void CANRX_SetVoltageU2(const CAN_SHIM_s *const kpkCanShim, uint8_t stringNumber, int32_t signalData);

/**
 * @brief   Sets V3 voltage measurement value in the CAN data shim
 * @param   kpkCanShim   shim to the database entries
 * @param   stringNumber addressed string
 * @param   signalData   actual signal data
 */
static void CANRX_SetVoltageU3(const CAN_SHIM_s *const kpkCanShim, uint8_t stringNumber, int32_t signalData);

/**
 * @brief   Sets temperature measurement value in the CAN data shim
 * @param   kpkCanShim   shim to the database entries
 * @param   stringNumber addressed string
 * @param   signalData   actual signal data
 */
static void CANRX_SetTemperature(const CAN_SHIM_s *const kpkCanShim, uint8_t stringNumber, int32_t signalData);

/**
 * @brief   Sets power measurement value in the CAN data shim
 * @param   kpkCanShim   shim to the database entries
 * @param   stringNumber addressed string
 * @param   signalData   actual signal data
 */
static void CANRX_SetPower(const CAN_SHIM_s *const kpkCanShim, uint8_t stringNumber, int32_t signalData);

/**
 * @brief   Sets Coulomb counting value in the CAN data shim
 * @param   kpkCanShim   shim to the database entries
 * @param   stringNumber addressed string
 * @param   signalData   actual signal data
 */
static void CANRX_SetCoulombCounting(const CAN_SHIM_s *const kpkCanShim, uint8_t stringNumber, int32_t signalData);

/**
 * @brief   Sets energy counting value in the CAN data shim
 * @param   kpkCanShim   shim to the database entries
 * @param   stringNumber addressed string
 * @param   signalData   actual signal data
 */
static void CANRX_SetEnergyCounting(const CAN_SHIM_s *const kpkCanShim, uint8_t stringNumber, int32_t signalData);

/**
 * @brief   Sets the string number based on the associated CAN message ID
 * @details See the documentation of the CAN module FAQ when implementing a
 *          multi-string application
 *          <a
 * href="../../../../software/modules/driver/can/can_how-to.html#multi-string-support-when-using-isabellenhuette-ivt-current-sensors"
 * target="_blank"> Multi-string Support when using Isabellenhuette IVT Current Sensors
 *          </a>.
 * @param   messageId message ID to be handled
 * @return  string number associated with the CAN message ID
 */
static uint8_t CANRX_SetStringNumberBasedOnCanMessageId(uint32_t messageId);

/*========== Static Function Implementations ================================*/
static void CANRX_HandleChannelError(const CAN_SHIM_s *const kpkCanShim, uint32_t messageId, uint8_t stringNumber) {
    FAS_ASSERT(kpkCanShim != NULL_PTR);
    FAS_ASSERT(((messageId >= CANRX_IVT_STRING0_CURRENT_ID) && (messageId <= CANRX_IVT_STRING0_ENERGY_COUNTER_ID)));
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);
    switch (messageId) {
        case CANRX_IVT_STRING0_CURRENT_ID: /* Current status */
            kpkCanShim->pTableCurrentSensor->invalidCurrentMeasurement[stringNumber] = 1;
            break;
        case CANRX_IVT_STRING0_VOLTAGE1_ID: /* Voltage status */
            kpkCanShim->pTableCurrentSensor->invalidHighVoltageMeasurement[stringNumber][0] = 1;
            break;
        case CANRX_IVT_STRING0_VOLTAGE2_ID:
            kpkCanShim->pTableCurrentSensor->invalidHighVoltageMeasurement[stringNumber][1] = 1;
            break;
        case CANRX_IVT_STRING0_VOLTAGE3_ID:
            kpkCanShim->pTableCurrentSensor->invalidHighVoltageMeasurement[stringNumber][2] = 1;
            break;
        case CANRX_IVT_STRING0_TEMPERATURE_ID: /* Temperature status */
            kpkCanShim->pTableCurrentSensor->invalidSensorTemperatureMeasurement[stringNumber] = 1;
            break;
        case CANRX_IVT_STRING0_POWER_ID: /* Power status */
            kpkCanShim->pTableCurrentSensor->invalidPowerMeasurement[stringNumber] = 1;
            break;
        case CANRX_IVT_STRING0_CURRENT_COUNTER_ID: /* CC status */
            kpkCanShim->pTableCurrentSensor->invalidCurrentCountingMeasurement[stringNumber] = 1;
            break;
        case CANRX_IVT_STRING0_ENERGY_COUNTER_ID: /* EC status */
            kpkCanShim->pTableCurrentSensor->invalidEnergyCountingMeasurement[stringNumber] = 1;
            break;
        default:                    /* LCOV_EXCL_LINE */
            /* No error detected */ /* LCOV_EXCL_LINE */
            break;                  /* LCOV_EXCL_LINE */
    }
}

static void CANRX_ResetError(const CAN_SHIM_s *const kpkCanShim, uint8_t stringNumber) {
    FAS_ASSERT(kpkCanShim != NULL_PTR);
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);
    kpkCanShim->pTableCurrentSensor->invalidCurrentMeasurement[stringNumber]           = 0;
    kpkCanShim->pTableCurrentSensor->invalidHighVoltageMeasurement[stringNumber][0]    = 0;
    kpkCanShim->pTableCurrentSensor->invalidHighVoltageMeasurement[stringNumber][1]    = 0;
    kpkCanShim->pTableCurrentSensor->invalidHighVoltageMeasurement[stringNumber][2]    = 0;
    kpkCanShim->pTableCurrentSensor->invalidSensorTemperatureMeasurement[stringNumber] = 0;
    kpkCanShim->pTableCurrentSensor->invalidPowerMeasurement[stringNumber]             = 0;
    kpkCanShim->pTableCurrentSensor->invalidCurrentCountingMeasurement[stringNumber]   = 0;
    kpkCanShim->pTableCurrentSensor->invalidEnergyCountingMeasurement[stringNumber]    = 0;
}

static void CANRX_HandleMeasurementError(const CAN_SHIM_s *const kpkCanShim, uint8_t stringNumber) {
    FAS_ASSERT(kpkCanShim != NULL_PTR);
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);
    kpkCanShim->pTableCurrentSensor->invalidCurrentMeasurement[stringNumber]           = 1;
    kpkCanShim->pTableCurrentSensor->invalidHighVoltageMeasurement[stringNumber][0]    = 1;
    kpkCanShim->pTableCurrentSensor->invalidHighVoltageMeasurement[stringNumber][1]    = 1;
    kpkCanShim->pTableCurrentSensor->invalidHighVoltageMeasurement[stringNumber][2]    = 1;
    kpkCanShim->pTableCurrentSensor->invalidSensorTemperatureMeasurement[stringNumber] = 1;
    kpkCanShim->pTableCurrentSensor->invalidPowerMeasurement[stringNumber]             = 1;
    kpkCanShim->pTableCurrentSensor->invalidCurrentCountingMeasurement[stringNumber]   = 1;
    kpkCanShim->pTableCurrentSensor->invalidEnergyCountingMeasurement[stringNumber]    = 1;
}

static void CANRX_HandleSystemError(const CAN_SHIM_s *const kpkCanShim, uint8_t stringNumber) {
    FAS_ASSERT(kpkCanShim != NULL_PTR);
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);
    kpkCanShim->pTableCurrentSensor->invalidCurrentMeasurement[stringNumber]           = 1;
    kpkCanShim->pTableCurrentSensor->invalidHighVoltageMeasurement[stringNumber][0]    = 1;
    kpkCanShim->pTableCurrentSensor->invalidHighVoltageMeasurement[stringNumber][1]    = 1;
    kpkCanShim->pTableCurrentSensor->invalidHighVoltageMeasurement[stringNumber][2]    = 1;
    kpkCanShim->pTableCurrentSensor->invalidSensorTemperatureMeasurement[stringNumber] = 1;
    kpkCanShim->pTableCurrentSensor->invalidPowerMeasurement[stringNumber]             = 1;
    kpkCanShim->pTableCurrentSensor->invalidCurrentCountingMeasurement[stringNumber]   = 1;
    kpkCanShim->pTableCurrentSensor->invalidEnergyCountingMeasurement[stringNumber]    = 1;
}

static void CANRX_HandleSensorData(
    const CAN_SHIM_s *const kpkCanShim,
    uint32_t messageId,
    uint8_t stringNumber,
    int32_t signalData) {
    FAS_ASSERT(kpkCanShim != NULL_PTR);
    FAS_ASSERT(((messageId >= CANRX_IVT_STRING0_CURRENT_ID) && (messageId <= CANRX_IVT_STRING0_ENERGY_COUNTER_ID)));
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);
    /* AXIVION Routine Generic-MissingParameterAssert: signalData: parameter accepts whole range */

    switch (messageId) {
        /* Current measurement */
        case CANRX_IVT_STRING0_CURRENT_ID:
            CANRX_SetCurrent(kpkCanShim, stringNumber, signalData);
            break;
        /* Voltage measurement U1 */
        case CANRX_IVT_STRING0_VOLTAGE1_ID:
            CANRX_SetVoltageU1(kpkCanShim, stringNumber, signalData);
            break;
        /* Voltage measurement U2 */
        case CANRX_IVT_STRING0_VOLTAGE2_ID:
            CANRX_SetVoltageU2(kpkCanShim, stringNumber, signalData);
            break;
        /* Voltage measurement U3 */
        case CANRX_IVT_STRING0_VOLTAGE3_ID:
            CANRX_SetVoltageU3(kpkCanShim, stringNumber, signalData);
            break;
        /* Temperature measurement */
        case CANRX_IVT_STRING0_TEMPERATURE_ID:
            CANRX_SetTemperature(kpkCanShim, stringNumber, signalData);
            break;
        /* Power measurement */
        case CANRX_IVT_STRING0_POWER_ID:
            CANRX_SetPower(kpkCanShim, stringNumber, signalData);
            break;
        /* CC measurement */
        case CANRX_IVT_STRING0_CURRENT_COUNTER_ID:
            CANRX_SetCoulombCounting(kpkCanShim, stringNumber, signalData);
            break;
        /* EC measurement */
        case CANRX_IVT_STRING0_ENERGY_COUNTER_ID:
            CANRX_SetEnergyCounting(kpkCanShim, stringNumber, signalData);
            break;
        default:                  /* LCOV_EXCL_LINE */
            FAS_ASSERT(FAS_TRAP); /* LCOV_EXCL_LINE */
            break;                /* LCOV_EXCL_LINE */
    }
}

static void CANRX_SetCurrent(const CAN_SHIM_s *const kpkCanShim, uint8_t stringNumber, int32_t signalData) {
    FAS_ASSERT(kpkCanShim != NULL_PTR);
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);
    /* AXIVION Routine Generic-MissingParameterAssert: signalData: parameter accepts whole range */
    kpkCanShim->pTableCurrentSensor->current_mA[stringNumber] = signalData;
    kpkCanShim->pTableCurrentSensor->newCurrent++;
    kpkCanShim->pTableCurrentSensor->previousTimestampCurrent[stringNumber] =
        kpkCanShim->pTableCurrentSensor->timestampCurrent[stringNumber];
    kpkCanShim->pTableCurrentSensor->timestampCurrent[stringNumber] = OS_GetTickCount();
}

static void CANRX_SetVoltageU1(const CAN_SHIM_s *const kpkCanShim, uint8_t stringNumber, int32_t signalData) {
    FAS_ASSERT(kpkCanShim != NULL_PTR);
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);
    /* AXIVION Routine Generic-MissingParameterAssert: signalData: parameter accepts whole range */
    kpkCanShim->pTableCurrentSensor->highVoltage_mV[stringNumber][0] = signalData;
    kpkCanShim->pTableCurrentSensor->previousTimestampHighVoltage[stringNumber][0] =
        kpkCanShim->pTableCurrentSensor->timestampHighVoltage[stringNumber][0];
    kpkCanShim->pTableCurrentSensor->timestampHighVoltage[stringNumber][0] = OS_GetTickCount();
}

static void CANRX_SetVoltageU2(const CAN_SHIM_s *const kpkCanShim, uint8_t stringNumber, int32_t signalData) {
    FAS_ASSERT(kpkCanShim != NULL_PTR);
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);
    /* AXIVION Routine Generic-MissingParameterAssert: signalData: parameter accepts whole range */
    kpkCanShim->pTableCurrentSensor->highVoltage_mV[stringNumber][1] = signalData;
    kpkCanShim->pTableCurrentSensor->previousTimestampHighVoltage[stringNumber][1] =
        kpkCanShim->pTableCurrentSensor->timestampHighVoltage[stringNumber][1];
    kpkCanShim->pTableCurrentSensor->timestampHighVoltage[stringNumber][1] = OS_GetTickCount();
}

static void CANRX_SetVoltageU3(const CAN_SHIM_s *const kpkCanShim, uint8_t stringNumber, int32_t signalData) {
    FAS_ASSERT(kpkCanShim != NULL_PTR);
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);
    /* AXIVION Routine Generic-MissingParameterAssert: signalData: parameter accepts whole range */
    kpkCanShim->pTableCurrentSensor->highVoltage_mV[stringNumber][2] = signalData;
    kpkCanShim->pTableCurrentSensor->previousTimestampHighVoltage[stringNumber][2] =
        kpkCanShim->pTableCurrentSensor->timestampHighVoltage[stringNumber][2];
    kpkCanShim->pTableCurrentSensor->timestampHighVoltage[stringNumber][2] = OS_GetTickCount();
}

static void CANRX_SetTemperature(const CAN_SHIM_s *const kpkCanShim, uint8_t stringNumber, int32_t signalData) {
    FAS_ASSERT(kpkCanShim != NULL_PTR);
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);
    /* AXIVION Routine Generic-MissingParameterAssert: signalData: parameter accepts whole range */
    kpkCanShim->pTableCurrentSensor->sensorTemperature_ddegC[stringNumber] = signalData;
}

static void CANRX_SetPower(const CAN_SHIM_s *const kpkCanShim, uint8_t stringNumber, int32_t signalData) {
    FAS_ASSERT(kpkCanShim != NULL_PTR);
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);
    /* AXIVION Routine Generic-MissingParameterAssert: signalData: parameter accepts whole range */
    kpkCanShim->pTableCurrentSensor->power_W[stringNumber] = signalData;
    kpkCanShim->pTableCurrentSensor->newPower++;
    kpkCanShim->pTableCurrentSensor->previousTimestampPower[stringNumber] =
        kpkCanShim->pTableCurrentSensor->timestampPower[stringNumber];
    kpkCanShim->pTableCurrentSensor->timestampPower[stringNumber] = OS_GetTickCount();
}

static void CANRX_SetCoulombCounting(const CAN_SHIM_s *const kpkCanShim, uint8_t stringNumber, int32_t signalData) {
    FAS_ASSERT(kpkCanShim != NULL_PTR);
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);
    /* AXIVION Routine Generic-MissingParameterAssert: signalData: parameter accepts whole range */
    kpkCanShim->pTableCurrentSensor->previousTimestampCurrentCounting[stringNumber] =
        kpkCanShim->pTableCurrentSensor->timestampCurrentCounting[stringNumber];
    kpkCanShim->pTableCurrentSensor->timestampCurrentCounting[stringNumber] = OS_GetTickCount();
    kpkCanShim->pTableCurrentSensor->currentCounter_As[stringNumber]        = signalData;
}

static void CANRX_SetEnergyCounting(const CAN_SHIM_s *const kpkCanShim, uint8_t stringNumber, int32_t signalData) {
    FAS_ASSERT(kpkCanShim != NULL_PTR);
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);
    /* AXIVION Routine Generic-MissingParameterAssert: signalData: parameter accepts whole range */
    kpkCanShim->pTableCurrentSensor->energyCounter_Wh[stringNumber] = signalData;
    kpkCanShim->pTableCurrentSensor->previousTimestampEnergyCounting[stringNumber] =
        kpkCanShim->pTableCurrentSensor->timestampEnergyCounting[stringNumber];
    kpkCanShim->pTableCurrentSensor->timestampEnergyCounting[stringNumber] = OS_GetTickCount();
}

static uint8_t CANRX_SetStringNumberBasedOnCanMessageId(uint32_t messageId) {
    FAS_ASSERT(((messageId >= CANRX_IVT_STRING0_CURRENT_ID) && (messageId <= CANRX_IVT_STRING0_ENERGY_COUNTER_ID)));
    uint8_t stringNumber = 0u;
    if (messageId <= CANRX_IVT_STRING0_ENERGY_COUNTER_ID) {
        stringNumber = 0u;
    }
    return stringNumber;
}

/*========== Extern Function Implementations ================================*/
extern uint32_t CANRX_CurrentSensor(
    CAN_MESSAGE_PROPERTIES_s message,
    const uint8_t *const kpkCanData,
    const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(((message.id >= CANRX_IVT_STRING0_CURRENT_ID) && (message.id <= CANRX_IVT_STRING0_ENERGY_COUNTER_ID)));
    FAS_ASSERT(message.idType == CANRX_CURRENT_SENSOR_MESSAGES_ID_TYPE);
    FAS_ASSERT(message.dlc == CANRX_CURRENT_SENSOR_MESSAGES_DLC);
    FAS_ASSERT(kpkCanData != NULL_PTR);
    FAS_ASSERT(kpkCanShim != NULL_PTR);

    uint64_t messageData = 0uLL;
    uint64_t canSignal   = 0uLL;

    int32_t signalData   = 0;
    uint8_t stringNumber = CANRX_SetStringNumberBasedOnCanMessageId(message.id);

    CAN_RxGetMessageDataFromCanData(&messageData, kpkCanData, message.endianness);

    /* Handle error flags */
    /* Overcurrent */
    CAN_RxGetSignalDataFromMessageData(
        messageData,
        CANRX_CURRENT_SENSOR_OCS_START_BIT,
        CANRX_CURRENT_SENSOR_OCS_LENGTH,
        &canSignal,
        message.endianness);
    if (canSignal != 0uLL) {
        /* TODO */
    }
    /* Channel error */
    CAN_RxGetSignalDataFromMessageData(
        messageData,
        CANRX_CURRENT_SENSOR_CHANNEL_ERROR_START_BIT,
        CANRX_CURRENT_SENSOR_CHANNEL_ERROR_LENGTH,
        &canSignal,
        message.endianness);
    if (canSignal != 0uLL) {
        CANRX_HandleChannelError(kpkCanShim, message.id, stringNumber);
    } else {
        CANRX_ResetError(kpkCanShim, stringNumber);
    }
    /* Measurement */
    CAN_RxGetSignalDataFromMessageData(
        messageData,
        CANRX_CURRENT_SENSOR_MEASUREMENT_ERROR_START_BIT,
        CANRX_CURRENT_SENSOR_MEASUREMENT_ERROR_LENGTH,
        &canSignal,
        message.endianness);
    if (canSignal != 0uLL) {
        CANRX_HandleMeasurementError(kpkCanShim, stringNumber);
    }
    /* System error */
    CAN_RxGetSignalDataFromMessageData(
        messageData,
        CANRX_CURRENT_SENSOR_SYSTEM_ERROR_START_BIT,
        CANRX_CURRENT_SENSOR_SYSTEM_ERROR_LENGTH,
        &canSignal,
        message.endianness);
    if (canSignal != 0uLL) {
        CANRX_HandleSystemError(kpkCanShim, stringNumber);
    }

    /* Get data from sensor reading */
    CAN_RxGetSignalDataFromMessageData(
        messageData,
        CANRX_CURRENT_SENSOR_DATA_START_BIT,
        CANRX_CURRENT_SENSOR_DATA_LENGTH,
        &canSignal,
        message.endianness);
    signalData = (int32_t)canSignal;
    CANRX_HandleSensorData(kpkCanShim, message.id, stringNumber, signalData);

    DATA_WRITE_DATA(kpkCanShim->pTableCurrentSensor);
    return 0u;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
extern void TEST_CANRX_HandleChannelError(
    const CAN_SHIM_s *const kpkCanShim,
    uint32_t messageId,
    uint8_t stringNumber) {
    CANRX_HandleChannelError(kpkCanShim, messageId, stringNumber);
}
extern void TEST_CANRX_ResetError(const CAN_SHIM_s *const kpkCanShim, uint8_t stringNumber) {
    CANRX_ResetError(kpkCanShim, stringNumber);
}
extern void TEST_CANRX_HandleMeasurementError(const CAN_SHIM_s *const kpkCanShim, uint8_t stringNumber) {
    CANRX_HandleMeasurementError(kpkCanShim, stringNumber);
}
extern void TEST_CANRX_HandleSystemError(const CAN_SHIM_s *const kpkCanShim, uint8_t stringNumber) {
    CANRX_HandleSystemError(kpkCanShim, stringNumber);
}
extern void TEST_CANRX_HandleSensorData(
    const CAN_SHIM_s *const kpkCanShim,
    uint32_t messageId,
    uint8_t stringNumber,
    int32_t signalData) {
    CANRX_HandleSensorData(kpkCanShim, messageId, stringNumber, signalData);
}
extern void TEST_CANRX_SetCurrent(const CAN_SHIM_s *const kpkCanShim, uint8_t stringNumber, int32_t signalData) {
    CANRX_SetCurrent(kpkCanShim, stringNumber, signalData);
}
extern void TEST_CANRX_SetVoltageU1(const CAN_SHIM_s *const kpkCanShim, uint8_t stringNumber, int32_t signalData) {
    CANRX_SetVoltageU1(kpkCanShim, stringNumber, signalData);
}
extern void TEST_CANRX_SetVoltageU2(const CAN_SHIM_s *const kpkCanShim, uint8_t stringNumber, int32_t signalData) {
    CANRX_SetVoltageU2(kpkCanShim, stringNumber, signalData);
}
extern void TEST_CANRX_SetVoltageU3(const CAN_SHIM_s *const kpkCanShim, uint8_t stringNumber, int32_t signalData) {
    CANRX_SetVoltageU3(kpkCanShim, stringNumber, signalData);
}
extern void TEST_CANRX_SetTemperature(const CAN_SHIM_s *const kpkCanShim, uint8_t stringNumber, int32_t signalData) {
    CANRX_SetTemperature(kpkCanShim, stringNumber, signalData);
}
extern void TEST_CANRX_SetPower(const CAN_SHIM_s *const kpkCanShim, uint8_t stringNumber, int32_t signalData) {
    CANRX_SetPower(kpkCanShim, stringNumber, signalData);
}
extern void TEST_CANRX_SetCoulombCounting(
    const CAN_SHIM_s *const kpkCanShim,
    uint8_t stringNumber,
    int32_t signalData) {
    CANRX_SetCoulombCounting(kpkCanShim, stringNumber, signalData);
}
extern void TEST_CANRX_SetEnergyCounting(const CAN_SHIM_s *const kpkCanShim, uint8_t stringNumber, int32_t signalData) {
    CANRX_SetEnergyCounting(kpkCanShim, stringNumber, signalData);
}
extern uint8_t TEST_CANRX_SetStringNumberBasedOnCanMessageId(uint32_t messageId) {
    return CANRX_SetStringNumberBasedOnCanMessageId(messageId);
}
#endif
