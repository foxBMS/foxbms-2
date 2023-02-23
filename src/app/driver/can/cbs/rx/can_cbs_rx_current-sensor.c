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
 * @file    can_cbs_rx_current-sensor.c
 * @author  foxBMS Team
 * @date    2021-04-20 (date of creation)
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
 * @ingroup DRIVER
 * @prefix  CANRX
 *
 * @brief   CAN driver Rx callback implementation
 * @details CAN Rx callback for current sensor measurements
 */

/*========== Includes =======================================================*/
#include "can_cbs_rx.h"
#include "can_cfg_rx-message-definitions.h"
#include "can_helper.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/** Overcurrent flag */
#define CANRX_CURRENT_SENSOR_DIAGNOSIS_OVERCURRENT_BITMASK (0x1u)
/** Actual measurement error flag */
#define CANRX_CURRENT_SENSOR_DIAGNOSIS_ACTUAL_MEASUREMENT_ERROR_BITMASK (0x2u)
/** Any measurement error flag */
#define CANRX_CURRENT_SENSOR_DIAGNOSIS_ANY_MEASUREMENT_ERROR_BITMASK (0x4u)
/** System error flag */
#define CANRX_CURRENT_SENSOR_DIAGNOSIS_SYSTEM_ERROR_BITMASK (0x8u)

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/
extern uint32_t CANRX_CurrentSensor(
    CAN_MESSAGE_PROPERTIES_s message,
    const uint8_t *const kpkCanData,
    const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(((message.id >= CANRX_STRING0_CURRENT_ID) && (message.id <= CANRX_STRING0_ENERGY_COUNTER_ID)));
    FAS_ASSERT(message.idType == CANRX_CURRENT_SENSOR_MESSAGES_ID_TYPE);
    FAS_ASSERT(message.dlc == CANRX_CURRENT_SENSOR_MESSAGES_DLC);
    FAS_ASSERT(kpkCanData != NULL_PTR);
    FAS_ASSERT(kpkCanShim != NULL_PTR);

    /**
     * CAN signals used in this message
     * Parameters:
     * bit start, bit length, factor, offset, minimum value, maximum value
     */
    const CAN_SIGNAL_TYPE_s currentSensorStatus = {7u, 8u, 1.0f, 0.0f, 0.0f, 255.0f};
    const CAN_SIGNAL_TYPE_s currentSensorData   = {23u, 32u, 1.0f, 0.0f, -2147483648.0f, 2147483648.0f};

    uint64_t messageData = 0u;
    uint64_t canSignal   = 0u;

    int32_t sensorSignalValue = 0;
    uint8_t diagInfo          = 0u;
    uint8_t stringNumber      = 0u;

    if (message.id <= CANRX_STRING0_ENERGY_COUNTER_ID) {
        stringNumber = 0u;
    }

    CAN_RxGetMessageDataFromCanData(&messageData, kpkCanData, message.endianness);

    /* Get status*/
    CAN_RxGetSignalDataFromMessageData(
        messageData, currentSensorStatus.bitStart, currentSensorStatus.bitLength, &canSignal, message.endianness);

    /* only high nibble contains diag info */
    diagInfo = canSignal & 0xF0u;
    diagInfo >>= 4u;

    if ((diagInfo & CANRX_CURRENT_SENSOR_DIAGNOSIS_OVERCURRENT_BITMASK) != 0u) {
        /* Overcurrent detected. This feature is currently not supported. */
    }
    if ((diagInfo & CANRX_CURRENT_SENSOR_DIAGNOSIS_ACTUAL_MEASUREMENT_ERROR_BITMASK) != 0u) {
        switch (message.id) {
            case CANRX_STRING0_CURRENT_ID: /* Current status */
                kpkCanShim->pTableCurrentSensor->invalidCurrentMeasurement[stringNumber] = 1;
                break;
            case CANRX_STRING0_VOLTAGE1_ID: /* Voltage status */
                kpkCanShim->pTableCurrentSensor->invalidHighVoltageMeasurement[stringNumber][0] = 1;
                break;
            case CANRX_STRING0_VOLTAGE2_ID:
                kpkCanShim->pTableCurrentSensor->invalidHighVoltageMeasurement[stringNumber][1] = 1;
                break;
            case CANRX_STRING0_VOLTAGE3_ID:
                kpkCanShim->pTableCurrentSensor->invalidHighVoltageMeasurement[stringNumber][2] = 1;
                break;
            case CANRX_STRING0_TEMPERATURE_ID: /* Temperature status */
                kpkCanShim->pTableCurrentSensor->invalidSensorTemperatureMeasurement[stringNumber] = 1;
                break;
            case CANRX_STRING0_POWER_ID: /* Power status */
                kpkCanShim->pTableCurrentSensor->invalidPowerMeasurement[stringNumber] = 1;
                break;
            case CANRX_STRING0_CURRENT_COUNTER_ID: /* CC status */
                kpkCanShim->pTableCurrentSensor->invalidCurrentCountingMeasurement[stringNumber] = 1;
                break;
            case CANRX_STRING0_ENERGY_COUNTER_ID: /* EC status */
                kpkCanShim->pTableCurrentSensor->invalidEnergyCountingMeasurement[stringNumber] = 1;
                break;
            default:
                /* No error detected */
                break;
        }
    } else {
        kpkCanShim->pTableCurrentSensor->invalidCurrentMeasurement[stringNumber]           = 0;
        kpkCanShim->pTableCurrentSensor->invalidHighVoltageMeasurement[stringNumber][0]    = 0;
        kpkCanShim->pTableCurrentSensor->invalidHighVoltageMeasurement[stringNumber][1]    = 0;
        kpkCanShim->pTableCurrentSensor->invalidHighVoltageMeasurement[stringNumber][2]    = 0;
        kpkCanShim->pTableCurrentSensor->invalidSensorTemperatureMeasurement[stringNumber] = 0;
        kpkCanShim->pTableCurrentSensor->invalidPowerMeasurement[stringNumber]             = 0;
        kpkCanShim->pTableCurrentSensor->invalidCurrentCountingMeasurement[stringNumber]   = 0;
        kpkCanShim->pTableCurrentSensor->invalidEnergyCountingMeasurement[stringNumber]    = 0;
    }

    if (((diagInfo & CANRX_CURRENT_SENSOR_DIAGNOSIS_ANY_MEASUREMENT_ERROR_BITMASK) != 0u) ||
        ((diagInfo & CANRX_CURRENT_SENSOR_DIAGNOSIS_SYSTEM_ERROR_BITMASK) != 0u)) {
        kpkCanShim->pTableCurrentSensor->invalidCurrentMeasurement[stringNumber]           = 1;
        kpkCanShim->pTableCurrentSensor->invalidHighVoltageMeasurement[stringNumber][0]    = 1;
        kpkCanShim->pTableCurrentSensor->invalidHighVoltageMeasurement[stringNumber][1]    = 1;
        kpkCanShim->pTableCurrentSensor->invalidHighVoltageMeasurement[stringNumber][2]    = 1;
        kpkCanShim->pTableCurrentSensor->invalidSensorTemperatureMeasurement[stringNumber] = 1;
        kpkCanShim->pTableCurrentSensor->invalidPowerMeasurement[stringNumber]             = 1;
        kpkCanShim->pTableCurrentSensor->invalidCurrentCountingMeasurement[stringNumber]   = 1;
        kpkCanShim->pTableCurrentSensor->invalidEnergyCountingMeasurement[stringNumber]    = 1;
    }

    /* Get measurement */
    CAN_RxGetSignalDataFromMessageData(
        messageData, currentSensorData.bitStart, currentSensorData.bitLength, &canSignal, message.endianness);
    switch (message.id) {
        /* Current measurement */
        case CANRX_STRING0_CURRENT_ID:
            sensorSignalValue                                         = (int32_t)canSignal;
            kpkCanShim->pTableCurrentSensor->current_mA[stringNumber] = sensorSignalValue;
            kpkCanShim->pTableCurrentSensor->newCurrent++;
            kpkCanShim->pTableCurrentSensor->previousTimestampCurrent[stringNumber] =
                kpkCanShim->pTableCurrentSensor->timestampCurrent[stringNumber];
            kpkCanShim->pTableCurrentSensor->timestampCurrent[stringNumber] = OS_GetTickCount();
            break;
        /* Voltage measurement U1 */
        case CANRX_STRING0_VOLTAGE1_ID:
            sensorSignalValue                                                = (int32_t)canSignal;
            kpkCanShim->pTableCurrentSensor->highVoltage_mV[stringNumber][0] = sensorSignalValue;
            kpkCanShim->pTableCurrentSensor->previousTimestampHighVoltage[stringNumber][0] =
                kpkCanShim->pTableCurrentSensor->timestampHighVoltage[stringNumber][0];
            kpkCanShim->pTableCurrentSensor->timestampHighVoltage[stringNumber][0] = OS_GetTickCount();
            break;
        /* Voltage measurement U2 */
        case CANRX_STRING0_VOLTAGE2_ID:
            sensorSignalValue                                                = (int32_t)canSignal;
            kpkCanShim->pTableCurrentSensor->highVoltage_mV[stringNumber][1] = sensorSignalValue;
            kpkCanShim->pTableCurrentSensor->previousTimestampHighVoltage[stringNumber][1] =
                kpkCanShim->pTableCurrentSensor->timestampHighVoltage[stringNumber][1];
            kpkCanShim->pTableCurrentSensor->timestampHighVoltage[stringNumber][1] = OS_GetTickCount();
            break;
        /* Voltage measurement U3 */
        case CANRX_STRING0_VOLTAGE3_ID:
            sensorSignalValue                                                = (int32_t)canSignal;
            kpkCanShim->pTableCurrentSensor->highVoltage_mV[stringNumber][2] = sensorSignalValue;
            kpkCanShim->pTableCurrentSensor->previousTimestampHighVoltage[stringNumber][2] =
                kpkCanShim->pTableCurrentSensor->timestampHighVoltage[stringNumber][2];
            kpkCanShim->pTableCurrentSensor->timestampHighVoltage[stringNumber][2] = OS_GetTickCount();
            break;
        /* Temperature measurement */
        case CANRX_STRING0_TEMPERATURE_ID:
            sensorSignalValue                                                      = (int32_t)canSignal;
            kpkCanShim->pTableCurrentSensor->sensorTemperature_ddegC[stringNumber] = sensorSignalValue;
            break;
        /* Power measurement */
        case CANRX_STRING0_POWER_ID:
            sensorSignalValue                                      = (int32_t)canSignal;
            kpkCanShim->pTableCurrentSensor->power_W[stringNumber] = sensorSignalValue;
            kpkCanShim->pTableCurrentSensor->newPower++;
            kpkCanShim->pTableCurrentSensor->previousTimestampPower[stringNumber] =
                kpkCanShim->pTableCurrentSensor->timestampPower[stringNumber];
            kpkCanShim->pTableCurrentSensor->timestampPower[stringNumber] = OS_GetTickCount();
            break;
        /* CC measurement */
        case CANRX_STRING0_CURRENT_COUNTER_ID:
            sensorSignalValue = (int32_t)canSignal;
            kpkCanShim->pTableCurrentSensor->previousTimestampCurrentCounting[stringNumber] =
                kpkCanShim->pTableCurrentSensor->timestampCurrentCounting[stringNumber];
            kpkCanShim->pTableCurrentSensor->timestampCurrentCounting[stringNumber] = OS_GetTickCount();
            kpkCanShim->pTableCurrentSensor->currentCounter_As[stringNumber]        = sensorSignalValue;
            break;
        /* EC measurement */
        case CANRX_STRING0_ENERGY_COUNTER_ID:
            sensorSignalValue                                               = (int32_t)canSignal;
            kpkCanShim->pTableCurrentSensor->energyCounter_Wh[stringNumber] = sensorSignalValue;
            kpkCanShim->pTableCurrentSensor->previousTimestampEnergyCounting[stringNumber] =
                kpkCanShim->pTableCurrentSensor->timestampEnergyCounting[stringNumber];
            kpkCanShim->pTableCurrentSensor->timestampEnergyCounting[stringNumber] = OS_GetTickCount();
            break;

        default:
            FAS_ASSERT(FAS_TRAP);
            break; /* LCOV_EXCL_LINE */
    }

    DATA_WRITE_DATA(kpkCanShim->pTableCurrentSensor);
    return 0;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
