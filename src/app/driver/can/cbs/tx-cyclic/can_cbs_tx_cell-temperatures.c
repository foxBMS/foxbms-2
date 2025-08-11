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
 * @file    can_cbs_tx_cell-temperatures.c
 * @author  foxBMS Team
 * @date    2021-04-20 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup DRIVERS
 * @prefix  CANTX
 *
 * @brief   CAN driver Tx callback implementation
 * @details CAN Tx callback for cell temperatures
 */

/*========== Includes =======================================================*/
/* AXIVION Next Codeline Generic-LocalInclude: 'can_cbs_tx_cyclic.h' declares
 * the prototype for the callback 'CANTX_CellTemperatures' */
#include "can_cbs_tx_cyclic.h"
#include "can_cfg_tx-cyclic-message-definitions.h"
#include "can_helper.h"
#include "foxmath.h"

#include <math.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/
/** CAN message parameters for can cell temperature  */
#define CANTX_CAN_CELL_TEMPERATURE_MUX_START_BIT           (7u)
#define CANTX_CAN_CELL_TEMPERATURE_MUX_LENGTH              (8u)
#define CANTX_CAN_CELL_TEMPERATURE0_INVALID_FLAG_START_BIT (8u)
#define CANTX_CAN_CELL_TEMPERATURE1_INVALID_FLAG_START_BIT (9u)
#define CANTX_CAN_CELL_TEMPERATURE2_INVALID_FLAG_START_BIT (10u)
#define CANTX_CAN_CELL_TEMPERATURE3_INVALID_FLAG_START_BIT (11u)
#define CANTX_CAN_CELL_TEMPERATURE4_INVALID_FLAG_START_BIT (12u)
#define CANTX_CAN_CELL_TEMPERATURE5_INVALID_FLAG_START_BIT (13u)
#define CANTX_CAN_CELL_TEMPERATURE_INVALID_FLAG_LENGTH     (1u)
#define CANTX_CAN_CELL_TEMPERATURE0_START_BIT              (23u)
#define CANTX_CAN_CELL_TEMPERATURE1_START_BIT              (31u)
#define CANTX_CAN_CELL_TEMPERATURE2_START_BIT              (39u)
#define CANTX_CAN_CELL_TEMPERATURE3_START_BIT              (47u)
#define CANTX_CAN_CELL_TEMPERATURE4_START_BIT              (55u)
#define CANTX_CAN_CELL_TEMPERATURE5_START_BIT              (63u)
#define CANTX_CAN_CELL_TEMPERATURE_LENGTH                  (8u)

#define CANTX_MINIMUM_VALUE_MUX          (0.0f)
#define CANTX_MAXIMUM_VALUE_MUX          (255.0f)
#define CANTX_MINIMUM_VALUE_INVALID_FLAG (0.0f)
#define CANTX_MAXIMUM_VALUE_INVALID_FLAG (1.0f)
#define CANTX_MINIMUM_VALUE_TEMPERATURE  (-1280.0f)
#define CANTX_MAXIMUM_VALUE_TEMPERATURE  (1270.0f)

/*========== Static Constant and Variable Definitions =======================*/
/** the number of temperatures per message-frame */
#define CANTX_NUMBER_OF_MUX_TEMPERATURES_PER_MESSAGE (6u)

#if ((BS_NR_OF_TEMP_SENSORS % CANTX_NUMBER_OF_MUX_TEMPERATURES_PER_MESSAGE) == 0)
#define CANTX_NUMBER_OF_CAN_MESSAGES_FOR_CELL_TEMPERATURES \
    ((uint8_t)(BS_NR_OF_TEMP_SENSORS / CANTX_NUMBER_OF_MUX_TEMPERATURES_PER_MESSAGE))
#else
#define CANTX_NUMBER_OF_CAN_MESSAGES_FOR_CELL_TEMPERATURES \
    ((uint8_t)(BS_NR_OF_TEMP_SENSORS / CANTX_NUMBER_OF_MUX_TEMPERATURES_PER_MESSAGE) + 1u)
#endif

/**
 * CAN signals used in this message
 * Parameters:
 * bit start, bit length, factor, offset, minimum value, maximum value
 */
static const CAN_SIGNAL_TYPE_s cantx_cellTemperatureMultiplexer = {
    CANTX_CAN_CELL_TEMPERATURE_MUX_START_BIT,
    CANTX_CAN_CELL_TEMPERATURE_MUX_LENGTH,
    UNIT_CONVERSION_FACTOR_1_FLOAT,
    CAN_SIGNAL_OFFSET_0,
    CANTX_MINIMUM_VALUE_MUX,
    CANTX_MAXIMUM_VALUE_MUX};
static const CAN_SIGNAL_TYPE_s cantx_cell0TemperatureInvalidFlag = {
    CANTX_CAN_CELL_TEMPERATURE0_INVALID_FLAG_START_BIT,
    CANTX_CAN_CELL_TEMPERATURE_INVALID_FLAG_LENGTH,
    UNIT_CONVERSION_FACTOR_1_FLOAT,
    CAN_SIGNAL_OFFSET_0,
    CANTX_MINIMUM_VALUE_INVALID_FLAG,
    CANTX_MAXIMUM_VALUE_INVALID_FLAG};
static const CAN_SIGNAL_TYPE_s cantx_cell1TemperatureInvalidFlag = {
    CANTX_CAN_CELL_TEMPERATURE1_INVALID_FLAG_START_BIT,
    CANTX_CAN_CELL_TEMPERATURE_INVALID_FLAG_LENGTH,
    UNIT_CONVERSION_FACTOR_1_FLOAT,
    CAN_SIGNAL_OFFSET_0,
    CANTX_MINIMUM_VALUE_INVALID_FLAG,
    CANTX_MAXIMUM_VALUE_INVALID_FLAG};
static const CAN_SIGNAL_TYPE_s cantx_cell2TemperatureInvalidFlag = {
    CANTX_CAN_CELL_TEMPERATURE2_INVALID_FLAG_START_BIT,
    CANTX_CAN_CELL_TEMPERATURE_INVALID_FLAG_LENGTH,
    UNIT_CONVERSION_FACTOR_1_FLOAT,
    CAN_SIGNAL_OFFSET_0,
    CANTX_MINIMUM_VALUE_INVALID_FLAG,
    CANTX_MAXIMUM_VALUE_INVALID_FLAG};
static const CAN_SIGNAL_TYPE_s cantx_cell3TemperatureInvalidFlag = {
    CANTX_CAN_CELL_TEMPERATURE3_INVALID_FLAG_START_BIT,
    CANTX_CAN_CELL_TEMPERATURE_INVALID_FLAG_LENGTH,
    UNIT_CONVERSION_FACTOR_1_FLOAT,
    CAN_SIGNAL_OFFSET_0,
    CANTX_MINIMUM_VALUE_INVALID_FLAG,
    CANTX_MAXIMUM_VALUE_INVALID_FLAG};
static const CAN_SIGNAL_TYPE_s cantx_cell4TemperatureInvalidFlag = {
    CANTX_CAN_CELL_TEMPERATURE4_INVALID_FLAG_START_BIT,
    CANTX_CAN_CELL_TEMPERATURE_INVALID_FLAG_LENGTH,
    UNIT_CONVERSION_FACTOR_1_FLOAT,
    CAN_SIGNAL_OFFSET_0,
    CANTX_MINIMUM_VALUE_INVALID_FLAG,
    CANTX_MAXIMUM_VALUE_INVALID_FLAG};
static const CAN_SIGNAL_TYPE_s cantx_cell5TemperatureInvalidFlag = {
    CANTX_CAN_CELL_TEMPERATURE5_INVALID_FLAG_START_BIT,
    CANTX_CAN_CELL_TEMPERATURE_INVALID_FLAG_LENGTH,
    UNIT_CONVERSION_FACTOR_1_FLOAT,
    CAN_SIGNAL_OFFSET_0,
    CANTX_MINIMUM_VALUE_INVALID_FLAG,
    CANTX_MAXIMUM_VALUE_INVALID_FLAG};
static const CAN_SIGNAL_TYPE_s cantx_cell0Temperature_degC = {
    CANTX_CAN_CELL_TEMPERATURE0_START_BIT,
    CANTX_CAN_CELL_TEMPERATURE_LENGTH,
    UNIT_CONVERSION_FACTOR_10_FLOAT,
    CAN_SIGNAL_OFFSET_0,
    CANTX_MINIMUM_VALUE_TEMPERATURE,
    CANTX_MAXIMUM_VALUE_TEMPERATURE};
static const CAN_SIGNAL_TYPE_s cantx_cell1Temperature_degC = {
    CANTX_CAN_CELL_TEMPERATURE1_START_BIT,
    CANTX_CAN_CELL_TEMPERATURE_LENGTH,
    UNIT_CONVERSION_FACTOR_10_FLOAT,
    CAN_SIGNAL_OFFSET_0,
    CANTX_MINIMUM_VALUE_TEMPERATURE,
    CANTX_MAXIMUM_VALUE_TEMPERATURE};
static const CAN_SIGNAL_TYPE_s cantx_cell2Temperature_degC = {
    CANTX_CAN_CELL_TEMPERATURE2_START_BIT,
    CANTX_CAN_CELL_TEMPERATURE_LENGTH,
    UNIT_CONVERSION_FACTOR_10_FLOAT,
    CAN_SIGNAL_OFFSET_0,
    CANTX_MINIMUM_VALUE_TEMPERATURE,
    CANTX_MAXIMUM_VALUE_TEMPERATURE};
static const CAN_SIGNAL_TYPE_s cantx_cell3Temperature_degC = {
    CANTX_CAN_CELL_TEMPERATURE3_START_BIT,
    CANTX_CAN_CELL_TEMPERATURE_LENGTH,
    UNIT_CONVERSION_FACTOR_10_FLOAT,
    CAN_SIGNAL_OFFSET_0,
    CANTX_MINIMUM_VALUE_TEMPERATURE,
    CANTX_MAXIMUM_VALUE_TEMPERATURE};
static const CAN_SIGNAL_TYPE_s cantx_cell4Temperature_degC = {
    CANTX_CAN_CELL_TEMPERATURE4_START_BIT,
    CANTX_CAN_CELL_TEMPERATURE_LENGTH,
    UNIT_CONVERSION_FACTOR_10_FLOAT,
    CAN_SIGNAL_OFFSET_0,
    CANTX_MINIMUM_VALUE_TEMPERATURE,
    CANTX_MAXIMUM_VALUE_TEMPERATURE};
static const CAN_SIGNAL_TYPE_s cantx_cell5Temperature_degC = {
    CANTX_CAN_CELL_TEMPERATURE5_START_BIT,
    CANTX_CAN_CELL_TEMPERATURE_LENGTH,
    UNIT_CONVERSION_FACTOR_10_FLOAT,
    CAN_SIGNAL_OFFSET_0,
    CANTX_MINIMUM_VALUE_TEMPERATURE,
    CANTX_MAXIMUM_VALUE_TEMPERATURE};

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/**
 * @brief   Helper function for CANTX_CellTemperatures()
 * @details Used in the CANTX_CellTemperatures() callback to set invalid flag
 *          data and temperature data in the CAN frame.
 * @param[in] temperatureSensorId              ID of the temperature sensor
 * @param[in] pMessage                         pointer to CAN frame data
 * @param[in] cellTemperatureSignal            signal characteristics for
 *                                             temperature data
 * @param[in] cellTemperatureInvalidFlagSignal signal characteristics for
 *                                             invalid flag data
 * @param[in] endianness                       big or little endianness of data
 * @param[in] kpkCanShim                       shim to the database entries
 */
static void CANTX_TemperatureSetData(
    uint16_t temperatureSensorId,
    uint64_t *pMessage,
    CAN_SIGNAL_TYPE_s cellTemperatureSignal,
    CAN_SIGNAL_TYPE_s cellTemperatureInvalidFlagSignal,
    CAN_ENDIANNESS_e endianness,
    const CAN_SHIM_s *const kpkCanShim);

/*========== Static Function Implementations ================================*/

static void CANTX_TemperatureSetData(
    uint16_t temperatureSensorId,
    uint64_t *pMessage,
    CAN_SIGNAL_TYPE_s cellTemperatureSignal,
    CAN_SIGNAL_TYPE_s cellTemperatureInvalidFlagSignal,
    CAN_ENDIANNESS_e endianness,
    const CAN_SHIM_s *const kpkCanShim) {
    /* sensor index must not be greater than the number of sensors */
    FAS_ASSERT(temperatureSensorId < BS_NR_OF_TEMP_SENSORS);
    FAS_ASSERT(pMessage != NULL_PTR);
    FAS_ASSERT(cellTemperatureSignal.bitLength == 8u);
    FAS_ASSERT(
        (cellTemperatureSignal.bitStart == cantx_cell0Temperature_degC.bitStart) ||
        (cellTemperatureSignal.bitStart == cantx_cell1Temperature_degC.bitStart) ||
        (cellTemperatureSignal.bitStart == cantx_cell2Temperature_degC.bitStart) ||
        (cellTemperatureSignal.bitStart == cantx_cell3Temperature_degC.bitStart) ||
        (cellTemperatureSignal.bitStart == cantx_cell4Temperature_degC.bitStart) ||
        (cellTemperatureSignal.bitStart == cantx_cell5Temperature_degC.bitStart));
    FAS_ASSERT(cellTemperatureInvalidFlagSignal.bitLength == 1u);
    FAS_ASSERT(
        (cellTemperatureInvalidFlagSignal.bitStart == cantx_cell0TemperatureInvalidFlag.bitStart) ||
        (cellTemperatureInvalidFlagSignal.bitStart == cantx_cell1TemperatureInvalidFlag.bitStart) ||
        (cellTemperatureInvalidFlagSignal.bitStart == cantx_cell2TemperatureInvalidFlag.bitStart) ||
        (cellTemperatureInvalidFlagSignal.bitStart == cantx_cell3TemperatureInvalidFlag.bitStart) ||
        (cellTemperatureInvalidFlagSignal.bitStart == cantx_cell4TemperatureInvalidFlag.bitStart) ||
        (cellTemperatureInvalidFlagSignal.bitStart == cantx_cell5TemperatureInvalidFlag.bitStart));
    FAS_ASSERT(endianness == CANTX_CELL_TEMPERATURES_ENDIANNESS);
    FAS_ASSERT(kpkCanShim != NULL_PTR);

    /* start_index end_index module
     * 00          17        module1
     * 18          35        module2
     * 20          53        module3
     * 30          71        module4
     */
    /* start_index end_index string
     * 000         071       string0
     */

    /* Get string, module and cell number */
    const uint8_t stringNumber = DATA_GetStringNumberFromTemperatureIndex(temperatureSensorId);
    const uint8_t moduleNumber = DATA_GetModuleNumberFromTemperatureIndex(temperatureSensorId);
    const uint8_t sensorNumber = DATA_GetSensorNumberFromTemperatureIndex(temperatureSensorId);

    uint32_t signalDataIsValid;
    /* Valid bits data */
    if (kpkCanShim->pTableCellTemperature->invalidCellTemperature[stringNumber][moduleNumber][sensorNumber] == false) {
        signalDataIsValid = 1u;
    } else {
        signalDataIsValid = 0u;
    }
    /* Set valid bit data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(
        pMessage,
        cellTemperatureInvalidFlagSignal.bitStart,
        cellTemperatureInvalidFlagSignal.bitLength,
        signalDataIsValid,
        endianness);

    /* Temperature data */
    float_t signalData_degC =
        (float_t)kpkCanShim->pTableCellTemperature->cellTemperature_ddegC[stringNumber][moduleNumber][sensorNumber];
    /* Apply offset and factor, check min/max limits */
    CAN_TxPrepareSignalData(&signalData_degC, cellTemperatureSignal);
    /* Set temperature data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(
        pMessage,
        cellTemperatureSignal.bitStart,
        cellTemperatureSignal.bitLength,
        (int32_t)signalData_degC,
        endianness);
}

/*========== Extern Function Implementations ================================*/
extern uint32_t CANTX_CellTemperatures(
    CAN_MESSAGE_PROPERTIES_s message,
    uint8_t *pCanData,
    uint8_t *pMuxId,
    const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(message.id == CANTX_CELL_TEMPERATURES_ID);
    FAS_ASSERT(message.idType == CANTX_CELL_TEMPERATURES_ID_TYPE);
    FAS_ASSERT(message.dlc == CAN_FOXBMS_MESSAGES_DEFAULT_DLC);
    FAS_ASSERT(message.endianness == CANTX_CELL_TEMPERATURES_ENDIANNESS);
    FAS_ASSERT(pCanData != NULL_PTR);
    FAS_ASSERT(pMuxId != NULL_PTR);
    FAS_ASSERT(kpkCanShim != NULL_PTR);
    uint64_t messageData = 0u;

    /* Reset mux if maximum was reached */
    if (*pMuxId >= CANTX_NUMBER_OF_CAN_MESSAGES_FOR_CELL_TEMPERATURES) {
        *pMuxId = 0u;
        /* first signal to transmit cell temperatures: get database values */
        DATA_READ_DATA(kpkCanShim->pTableCellTemperature);
    }

    /* Set mux signal in CAN frame */
    CAN_TxSetMessageDataWithSignalData(
        &messageData,
        cantx_cellTemperatureMultiplexer.bitStart,
        cantx_cellTemperatureMultiplexer.bitLength,
        (uint64_t)*pMuxId,
        message.endianness);

    /* Set other signals in CAN frame */
    /* Calculate the global temperature sensor ID based on the multiplexer value for the first temperature sensor */
    uint16_t temperatureSensorId = (*pMuxId * CANTX_NUMBER_OF_MUX_TEMPERATURES_PER_MESSAGE);
    CANTX_TemperatureSetData(
        temperatureSensorId,
        &messageData,
        cantx_cell0Temperature_degC,
        cantx_cell0TemperatureInvalidFlag,
        message.endianness,
        kpkCanShim);
    temperatureSensorId++; /* Increment global temperature sensor ID */
    if (temperatureSensorId < BS_NR_OF_TEMP_SENSORS) {
        CANTX_TemperatureSetData(
            temperatureSensorId,
            &messageData,
            cantx_cell1Temperature_degC,
            cantx_cell1TemperatureInvalidFlag,
            message.endianness,
            kpkCanShim);
        temperatureSensorId++; /* Increment global temperature sensor ID */
        if (temperatureSensorId < BS_NR_OF_TEMP_SENSORS) {
            CANTX_TemperatureSetData(
                temperatureSensorId,
                &messageData,
                cantx_cell2Temperature_degC,
                cantx_cell2TemperatureInvalidFlag,
                message.endianness,
                kpkCanShim);

            temperatureSensorId++; /* Increment global temperature sensor ID */
            if (temperatureSensorId < BS_NR_OF_TEMP_SENSORS) {
                CANTX_TemperatureSetData(
                    temperatureSensorId,
                    &messageData,
                    cantx_cell3Temperature_degC,
                    cantx_cell3TemperatureInvalidFlag,
                    message.endianness,
                    kpkCanShim);
                temperatureSensorId++; /* Increment global temperature sensor ID */
                if (temperatureSensorId < BS_NR_OF_TEMP_SENSORS) {
                    CANTX_TemperatureSetData(
                        temperatureSensorId,
                        &messageData,
                        cantx_cell4Temperature_degC,
                        cantx_cell4TemperatureInvalidFlag,
                        message.endianness,
                        kpkCanShim);
                    temperatureSensorId++; /* Increment global temperature sensor ID */
                    if (temperatureSensorId < BS_NR_OF_TEMP_SENSORS) {
                        CANTX_TemperatureSetData(
                            temperatureSensorId,
                            &messageData,
                            cantx_cell5Temperature_degC,
                            cantx_cell5TemperatureInvalidFlag,
                            message.endianness,
                            kpkCanShim);
                    }
                }
            }
        }
    }

    /* Increment multiplexer for next cell */
    (*pMuxId)++;

    /* All signal data copied in CAN frame, now copy data in the buffer that will be use to send the frame */
    CAN_TxSetCanDataWithMessageData(messageData, pCanData, message.endianness);

    return 0u;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
extern void TEST_CANTX_TemperatureSetData(
    uint16_t temperatureSensorId,
    uint64_t *pMessage,
    CAN_SIGNAL_TYPE_s cellTemperatureSignal,
    CAN_SIGNAL_TYPE_s cellTemperatureInvalidFlagSignal,
    CAN_ENDIANNESS_e endianness,
    const CAN_SHIM_s *const kpkCanShim) {
    CANTX_TemperatureSetData(
        temperatureSensorId, pMessage, cellTemperatureSignal, cellTemperatureInvalidFlagSignal, endianness, kpkCanShim);
}
#endif
