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
 * @file    can_cbs_rx_aerosol-sensor.c
 * @author  foxBMS Team
 * @date    2023-08-29 (date of creation)
 * @updated 2023-10-12 (date of last update)
 * @version v1.6.0
 * @ingroup DRIVER
 * @prefix  CANRX
 *
 * @brief   CAN driver Rx callback implementation
 * @details CAN Rx callback for aerosol sensor measurements
 */

/*========== Includes =======================================================*/
#include "can_cbs_rx.h"
#include "can_cfg_rx-message-definitions.h"
#include "can_helper.h"
#include "diag.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/
/* configuration of the aerosol sensor message */
#define CANRX_PARTICULAR_MATTER_CONCENTRATION_START_BIT (7u)
#define CANRX_PARTICULAR_MATTER_CONCENTRATION_LENGTH    (16u)
#define CANRX_AEROSOL_SENSOR_STATUS_START_BIT           (34u)
#define CANRX_AEROSOL_SENSOR_STATUS_LENGTH              (3u)
#define CANRX_AEROSOL_SENSOR_FAULTS_START_BIT           (39u)
#define CANRX_AEROSOL_SENSOR_FAULTS_LENGTH              (5u)
#define CANRX_AEROSOL_SENSOR_CRC_CHECK_CODE_START_BIT   (63u)
#define CANRX_AEROSOL_SENSOR_CRC_CHECK_CODE_LENGTH      (8u)

/* sensor status cases */
#define CANRX_AEROSOL_SENSOR_STATUS_NORMAL (0u)
#define CANRX_AEROSOL_SENSOR_STATUS_ALARM  (1u)

/* sensor error cases */
#define CANRX_AEROSOL_SENSOR_NO_ERROR                           (0u)
#define CANRX_AEROSOL_SENSOR_PHOTOELECTRIC_ERROR                (1u)
#define CANRX_AEROSOL_SENSOR_VOLTAGE_SUPPLY_OVER_VOLTAGE_ERROR  (2u)
#define CANRX_AEROSOL_SENSOR_VOLTAGE_SUPPLY_UNDER_VOLTAGE_ERROR (3u)

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/
/**
 * @brief   Sets error flags in database as received from can message
 * @param   kpkCanShim const pointer to can shim
 * @param   signalData data from can message that covers errors
 */
static void CANRX_HandleAerosolSensorErrors(const CAN_SHIM_s *const kpkCanShim, uint16_t signalData);

/**
 * @brief   Sets sensor status in database
 * @param   kpkCanShim const pointer to can shim
 * @param   signalData data from can message that covers status
 */
static void CANRX_HandleAerosolSensorStatus(const CAN_SHIM_s *const kpkCanShim, uint16_t signalData);

/**
 * @brief   Sets current particulate matter measurement value in database
 * @param   kpkCanShim const pointer to can shim
 * @param   signalData data from can message that covers particulate matter measurement
 */
static void CANRX_SetParticulateMatterConcentration(const CAN_SHIM_s *const kpkCanShim, uint16_t signalData);

/**
 * @brief   Sets CRC check code from sensor in database
 * @param   kpkCanShim const pointer to can shim
 * @param   signalData data from can message that covers crc check code
 */
static void CANRX_SetAerosolSensorCrcCheckCode(const CAN_SHIM_s *const kpkCanShim, uint16_t signalData);

/*========== Static Function Implementations ================================*/
static void CANRX_HandleAerosolSensorErrors(const CAN_SHIM_s *const kpkCanShim, uint16_t signalData) {
    FAS_ASSERT(kpkCanShim != NULL_PTR);
    FAS_ASSERT(
        (signalData == CANRX_AEROSOL_SENSOR_NO_ERROR) || (signalData == CANRX_AEROSOL_SENSOR_PHOTOELECTRIC_ERROR) ||
        (signalData == CANRX_AEROSOL_SENSOR_VOLTAGE_SUPPLY_OVER_VOLTAGE_ERROR) ||
        (signalData == CANRX_AEROSOL_SENSOR_VOLTAGE_SUPPLY_UNDER_VOLTAGE_ERROR));
    switch (signalData) {
        case CANRX_AEROSOL_SENSOR_NO_ERROR: /* resets error flags */
            kpkCanShim->pTableAerosolSensor->photoelectricError      = false;
            kpkCanShim->pTableAerosolSensor->supplyOvervoltageError  = false;
            kpkCanShim->pTableAerosolSensor->supplyUndervoltageError = false;
            break;
        case CANRX_AEROSOL_SENSOR_PHOTOELECTRIC_ERROR: /* sets flag for photoelectric device fault */
            kpkCanShim->pTableAerosolSensor->photoelectricError = true;
            break;
        case CANRX_AEROSOL_SENSOR_VOLTAGE_SUPPLY_OVER_VOLTAGE_ERROR: /* sets flag for voltage supply over voltage */
            kpkCanShim->pTableAerosolSensor->supplyOvervoltageError = true;
            break;
        case CANRX_AEROSOL_SENSOR_VOLTAGE_SUPPLY_UNDER_VOLTAGE_ERROR: /* sets flag for voltage supply under voltage */
            kpkCanShim->pTableAerosolSensor->supplyUndervoltageError = true;
            break;
        default:                  /* LCOV_EXCL_LINE */
            /* undefined value */ /* LCOV_EXCL_LINE */
            break;                /* LCOV_EXCL_LINE */
    }
}

static void CANRX_HandleAerosolSensorStatus(const CAN_SHIM_s *const kpkCanShim, uint16_t signalData) {
    FAS_ASSERT(kpkCanShim != NULL_PTR);
    FAS_ASSERT((signalData == CANRX_AEROSOL_SENSOR_STATUS_NORMAL) || (signalData == CANRX_AEROSOL_SENSOR_STATUS_ALARM));

    switch (signalData) {
        case CANRX_AEROSOL_SENSOR_STATUS_NORMAL: /* set status to normal */
            kpkCanShim->pTableAerosolSensor->sensorStatus = 0u;
            break;
        case CANRX_AEROSOL_SENSOR_STATUS_ALARM: /* set status to alarm, update diag */
            kpkCanShim->pTableAerosolSensor->sensorStatus = 1u;
            DIAG_Handler(DIAG_ID_AEROSOL_ALERT, DIAG_EVENT_NOT_OK, DIAG_SYSTEM, 0u);
            break;
        default:                  /* LCOV_EXCL_LINE */
            /* undefined value */ /* LCOV_EXCL_LINE */
            break;                /* LCOV_EXCL_LINE */
    }
}

static void CANRX_SetParticulateMatterConcentration(const CAN_SHIM_s *const kpkCanShim, uint16_t signalData) {
    FAS_ASSERT(kpkCanShim != NULL_PTR);
    /* AXIVION Routine Generic-MissingParameterAssert: signalData: parameter accept whole range */
    /* set particulate matter concentration in database */
    kpkCanShim->pTableAerosolSensor->particulateMatterConcentration = signalData;
}

static void CANRX_SetAerosolSensorCrcCheckCode(const CAN_SHIM_s *const kpkCanShim, uint16_t signalData) {
    FAS_ASSERT(kpkCanShim != NULL_PTR);
    /* AXIVION Routine Generic-MissingParameterAssert: signalData: parameter accept whole range */
    /* set CRC check code in database */
    uint8_t data                                  = (uint8_t)signalData;
    kpkCanShim->pTableAerosolSensor->crcCheckCode = data;
}

/*========== Extern Function Implementations ================================*/
extern uint32_t CANRX_AerosolSensor(
    CAN_MESSAGE_PROPERTIES_s message,
    const uint8_t *const kpkCanData,
    const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(message.id == CANRX_AEROSOL_SENSOR_ID);
    FAS_ASSERT(message.idType == CANRX_AEROSOL_SENSOR_ID_TYPE);
    FAS_ASSERT(message.dlc == CAN_DEFAULT_DLC);
    FAS_ASSERT(kpkCanData != NULL_PTR);
    FAS_ASSERT(kpkCanShim != NULL_PTR);

    uint64_t messageData = 0u;
    uint64_t canSignal   = 0u;

    CAN_RxGetMessageDataFromCanData(&messageData, kpkCanData, message.endianness);

    /* Handle error flags */
    CAN_RxGetSignalDataFromMessageData(
        messageData,
        CANRX_AEROSOL_SENSOR_FAULTS_START_BIT,
        CANRX_AEROSOL_SENSOR_FAULTS_LENGTH,
        &canSignal,
        message.endianness);
    uint16_t signalData = (uint16_t)canSignal;
    CANRX_HandleAerosolSensorErrors(kpkCanShim, signalData);

    /* Set sensor status */
    CAN_RxGetSignalDataFromMessageData(
        messageData,
        CANRX_AEROSOL_SENSOR_STATUS_START_BIT,
        CANRX_AEROSOL_SENSOR_STATUS_LENGTH,
        &canSignal,
        message.endianness);
    signalData = (uint16_t)canSignal;
    CANRX_HandleAerosolSensorStatus(kpkCanShim, signalData);

    /* Set data */
    CAN_RxGetSignalDataFromMessageData(
        messageData,
        CANRX_PARTICULAR_MATTER_CONCENTRATION_START_BIT,
        CANRX_PARTICULAR_MATTER_CONCENTRATION_LENGTH,
        &canSignal,
        message.endianness);
    signalData = (uint16_t)canSignal;
    CANRX_SetParticulateMatterConcentration(kpkCanShim, signalData);

    /* Set CRC check code*/
    CAN_RxGetSignalDataFromMessageData(
        messageData,
        CANRX_AEROSOL_SENSOR_CRC_CHECK_CODE_START_BIT,
        CANRX_AEROSOL_SENSOR_CRC_CHECK_CODE_LENGTH,
        &canSignal,
        message.endianness);
    signalData = (uint16_t)canSignal;
    CANRX_SetAerosolSensorCrcCheckCode(kpkCanShim, signalData);

    DATA_WRITE_DATA(kpkCanShim->pTableAerosolSensor);
    return 0;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
extern void TEST_CANRX_HandleAerosolSensorErrors(const CAN_SHIM_s *const kpkCanShim, uint16_t signalData) {
    CANRX_HandleAerosolSensorErrors(kpkCanShim, signalData);
}
extern void TEST_CANRX_HandleAerosolSensorStatus(const CAN_SHIM_s *const kpkCanShim, uint16_t signalData) {
    CANRX_HandleAerosolSensorStatus(kpkCanShim, signalData);
}
extern void TEST_CANRX_SetParticulateMatterConcentration(const CAN_SHIM_s *const kpkCanShim, uint16_t signalData) {
    CANRX_SetParticulateMatterConcentration(kpkCanShim, signalData);
}
extern void TEST_CANRX_SetAerosolSensorCrcCheckCode(const CAN_SHIM_s *const kpkCanShim, uint16_t signalData) {
    CANRX_SetAerosolSensorCrcCheckCode(kpkCanShim, signalData);
}
#endif
