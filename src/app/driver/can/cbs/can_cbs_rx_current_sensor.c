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
 * @file    can_cbs_rx_current_sensor.c
 * @author  foxBMS Team
 * @date    2021-04-20 (date of creation)
 * @updated 2021-06-09 (date of last update)
 * @ingroup DRIVER
 * @prefix  CAN
 *
 * @brief   CAN driver Rx callback implementation
 * @details CAN Rx callback for current sensor measurements
 */

/*========== Includes =======================================================*/
#include "can_cbs.h"
#include "can_helper.h"

/*========== Macros and Definitions =========================================*/

/* Overcurrent flag */
#define CAN_currentSensorDiagOcs (0x1u)
/* Actual measurement error flag */
#define CAN_currentSensorDiagActualMeasurementError (0x2u)
/* Any measurement error flag */
#define CAN_currentSensorDiagAnyMeasurementError (0x4u)
/* System error flag */
#define CAN_currentSensorDiagSystemError (0x8u)

/*========== Static Constant and Variable Definitions =======================*/

/**
 * CAN signals used in this message
 * Parameters:
 * bit start, bit length, factor, offset, minimum value, maximum value
 */
static const CAN_SIGNAL_TYPE_s currentSensorStatus = {7u, 8u, 1.0f, 0.0f, 0.0f, 255.0f};
static const CAN_SIGNAL_TYPE_s currentSensorData   = {23u, 32u, 1.0f, 0.0f, -2147483648.0f, 2147483648.0f};

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/
extern uint32_t CAN_RxCurrentSensor(
    uint32_t id,
    uint8_t dlc,
    CAN_ENDIANNESS_e endianness,
    uint8_t *pCanData,
    uint8_t *pMuxId,
    const CAN_SHIM_s *const kpkCanShim) {
    /* pMuxId is unused in this callback, therefore has to be a NULL_PTR */
    FAS_ASSERT(pMuxId == NULL_PTR);

    FAS_ASSERT(id < CAN_MAX_11BIT_ID); /* Currently standard ID, 11 bit */
    FAS_ASSERT(dlc <= CAN_MAX_DLC);    /* Currently max 8 bytes in a CAN frame */
    FAS_ASSERT(pCanData != NULL_PTR);
    FAS_ASSERT(kpkCanShim != NULL_PTR);
    uint64_t message   = 0u;
    uint64_t canSignal = 0u;

    int32_t sensorSignalValue = 0;
    uint8_t diagInfo          = 0u;
    uint8_t stringNumber      = 0u;

    if (id <= CAN_ID_STRING0_ENERGY_COUNTER) {
        stringNumber = 0u;
    } else if (id <= CAN_ID_STRING1_ENERGY_COUNTER) {
        stringNumber = 1u;
    } else {
        stringNumber = 2u;
    }

    CAN_RxGetMessageDataFromCanData(&message, pCanData, endianness);

    /* Get status*/
    CAN_RxGetSignalDataFromMessageData(
        message, currentSensorStatus.bitStart, currentSensorStatus.bitLength, &canSignal, endianness);

    /* only high nibble contains diag info */
    diagInfo = canSignal & 0xF0u;
    diagInfo >>= 4u;

    if ((diagInfo & CAN_currentSensorDiagOcs) != 0u) {
        /* Overcurrent detected. This feature is currently not supported. */
    }
    if ((diagInfo & CAN_currentSensorDiagActualMeasurementError) != 0u) {
        switch (id) {
            case CAN_ID_STRING0_CURRENT: /* Current status */
            case CAN_ID_STRING1_CURRENT:
            case CAN_ID_STRING2_CURRENT:
                kpkCanShim->pTableCurrentSensor->invalidCurrentMeasurement[stringNumber] = 1;
                break;
            case CAN_ID_STRING0_VOLTAGE1: /* Voltage status */
            case CAN_ID_STRING1_VOLTAGE1:
            case CAN_ID_STRING2_VOLTAGE1:
                kpkCanShim->pTableCurrentSensor->invalidHighVoltageMeasurement[stringNumber][0] = 1;
                break;
            case CAN_ID_STRING0_VOLTAGE2:
            case CAN_ID_STRING1_VOLTAGE2:
            case CAN_ID_STRING2_VOLTAGE2:
                kpkCanShim->pTableCurrentSensor->invalidHighVoltageMeasurement[stringNumber][1] = 1;
                break;
            case CAN_ID_STRING0_VOLTAGE3:
            case CAN_ID_STRING1_VOLTAGE3:
            case CAN_ID_STRING2_VOLTAGE3:
                kpkCanShim->pTableCurrentSensor->invalidHighVoltageMeasurement[stringNumber][2] = 1;
                break;
            case CAN_ID_STRING0_TEMPERATURE: /* Temperature status */
            case CAN_ID_STRING1_TEMPERATURE:
            case CAN_ID_STRING2_TEMPERATURE:
                kpkCanShim->pTableCurrentSensor->invalidSensorTemperatureMeasurement[stringNumber] = 1;
                break;
            case CAN_ID_STRING0_POWER: /* Power status */
            case CAN_ID_STRING1_POWER:
            case CAN_ID_STRING2_POWER:
                kpkCanShim->pTableCurrentSensor->invalidPowerMeasurement[stringNumber] = 1;
                break;
            case CAN_ID_STRING0_CURRENT_COUNTER: /* CC status */
            case CAN_ID_STRING1_CURRENT_COUNTER:
            case CAN_ID_STRING2_CURRENT_COUNTER:
                kpkCanShim->pTableCurrentSensor->invalidCurrentCountingMeasurement[stringNumber] = 1;
                break;
            case CAN_ID_STRING0_ENERGY_COUNTER: /* EC status */
            case CAN_ID_STRING1_ENERGY_COUNTER:
            case CAN_ID_STRING2_ENERGY_COUNTER:
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

    if (((diagInfo & CAN_currentSensorDiagAnyMeasurementError) != 0u) ||
        ((diagInfo & CAN_currentSensorDiagSystemError) != 0u)) {
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
        message, currentSensorData.bitStart, currentSensorData.bitLength, &canSignal, endianness);
    switch (id) {
        /* Current measurement */
        case CAN_ID_STRING0_CURRENT:
        case CAN_ID_STRING1_CURRENT:
        case CAN_ID_STRING2_CURRENT:
            sensorSignalValue                                         = (int32_t)canSignal;
            kpkCanShim->pTableCurrentSensor->current_mA[stringNumber] = sensorSignalValue;
            kpkCanShim->pTableCurrentSensor->newCurrent++;
            kpkCanShim->pTableCurrentSensor->previousTimestampCurrent[stringNumber] =
                kpkCanShim->pTableCurrentSensor->timestampCurrent[stringNumber];
            kpkCanShim->pTableCurrentSensor->timestampCurrent[stringNumber] = OS_GetTickCount();
            break;
        /* Voltage measurement U1 */
        case CAN_ID_STRING0_VOLTAGE1:
        case CAN_ID_STRING1_VOLTAGE1:
        case CAN_ID_STRING2_VOLTAGE1:
            sensorSignalValue                                                = (int32_t)canSignal;
            kpkCanShim->pTableCurrentSensor->highVoltage_mV[stringNumber][0] = sensorSignalValue;
            kpkCanShim->pTableCurrentSensor->previousTimestampHighVoltage[stringNumber][0] =
                kpkCanShim->pTableCurrentSensor->timestampHighVoltage[stringNumber][0];
            kpkCanShim->pTableCurrentSensor->timestampHighVoltage[stringNumber][0] = OS_GetTickCount();
            break;
        /* Voltage measurement U2 */
        case CAN_ID_STRING0_VOLTAGE2:
        case CAN_ID_STRING1_VOLTAGE2:
        case CAN_ID_STRING2_VOLTAGE2:
            sensorSignalValue                                                = (int32_t)canSignal;
            kpkCanShim->pTableCurrentSensor->highVoltage_mV[stringNumber][1] = sensorSignalValue;
            kpkCanShim->pTableCurrentSensor->previousTimestampHighVoltage[stringNumber][1] =
                kpkCanShim->pTableCurrentSensor->timestampHighVoltage[stringNumber][1];
            kpkCanShim->pTableCurrentSensor->timestampHighVoltage[stringNumber][1] = OS_GetTickCount();
            break;
        /* Voltage measurement U3 */
        case CAN_ID_STRING0_VOLTAGE3:
        case CAN_ID_STRING1_VOLTAGE3:
        case CAN_ID_STRING2_VOLTAGE3:
            sensorSignalValue                                                = (int32_t)canSignal;
            kpkCanShim->pTableCurrentSensor->highVoltage_mV[stringNumber][2] = sensorSignalValue;
            kpkCanShim->pTableCurrentSensor->previousTimestampHighVoltage[stringNumber][2] =
                kpkCanShim->pTableCurrentSensor->timestampHighVoltage[stringNumber][2];
            kpkCanShim->pTableCurrentSensor->timestampHighVoltage[stringNumber][2] = OS_GetTickCount();
            break;
        /* Temperature measurement */
        case CAN_ID_STRING0_TEMPERATURE:
        case CAN_ID_STRING1_TEMPERATURE:
        case CAN_ID_STRING2_TEMPERATURE:
            sensorSignalValue                                                      = (int32_t)canSignal;
            kpkCanShim->pTableCurrentSensor->sensorTemperature_ddegC[stringNumber] = sensorSignalValue;
            break;
        /* Power measurement */
        case CAN_ID_STRING0_POWER:
        case CAN_ID_STRING1_POWER:
        case CAN_ID_STRING2_POWER:
            sensorSignalValue                                      = (int32_t)canSignal;
            kpkCanShim->pTableCurrentSensor->power_W[stringNumber] = sensorSignalValue;
            kpkCanShim->pTableCurrentSensor->newPower++;
            kpkCanShim->pTableCurrentSensor->previousTimestampPower[stringNumber] =
                kpkCanShim->pTableCurrentSensor->timestampPower[stringNumber];
            kpkCanShim->pTableCurrentSensor->timestampPower[stringNumber] = OS_GetTickCount();
            break;
        /* CC measurement */
        case CAN_ID_STRING0_CURRENT_COUNTER:
        case CAN_ID_STRING1_CURRENT_COUNTER:
        case CAN_ID_STRING2_CURRENT_COUNTER:
            sensorSignalValue = (int32_t)canSignal;
            kpkCanShim->pTableCurrentSensor->previousTimestampCurrentCounting[stringNumber] =
                kpkCanShim->pTableCurrentSensor->timestampCurrentCounting[stringNumber];
            kpkCanShim->pTableCurrentSensor->timestampCurrentCounting[stringNumber] = OS_GetTickCount();
            kpkCanShim->pTableCurrentSensor->currentCounter_As[stringNumber]        = sensorSignalValue;
            break;
        /* EC measurement */
        case CAN_ID_STRING0_ENERGY_COUNTER:
        case CAN_ID_STRING1_ENERGY_COUNTER:
        case CAN_ID_STRING2_ENERGY_COUNTER:
            sensorSignalValue                                               = (int32_t)canSignal;
            kpkCanShim->pTableCurrentSensor->energyCounter_Wh[stringNumber] = sensorSignalValue;
            kpkCanShim->pTableCurrentSensor->previousTimestampEnergyCounting[stringNumber] =
                kpkCanShim->pTableCurrentSensor->timestampEnergyCounting[stringNumber];
            kpkCanShim->pTableCurrentSensor->timestampEnergyCounting[stringNumber] = OS_GetTickCount();
            break;

        default:
            FAS_ASSERT(FAS_TRAP);
            break;
    }

    DATA_WRITE_DATA(kpkCanShim->pTableCurrentSensor);
    return 0;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST

#endif
