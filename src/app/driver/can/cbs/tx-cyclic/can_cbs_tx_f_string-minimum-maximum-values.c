/**
 *
 * @copyright &copy; 2010 - 2026, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * @file    can_cbs_tx_f_string-minimum-maximum-values.c
 * @author  foxBMS Team
 * @date    2021-04-20 (date of creation)
 * @updated 2026-04-20 (date of last update)
 * @version v1.11.0
 * @ingroup DRIVERS
 * @prefix  CANTX
 *
 * @brief   CAN driver Tx callback implementation
 * @details CAN Tx callback for min/max values
 */

/*========== Includes =======================================================*/
#include "bms.h"
#include "can_cbs_tx_cyclic.h"
#include "can_cfg_tx-cyclic-message-definitions.h"
#include "can_helper.h"
#include "foxmath.h"

#include <math.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/
/**
 * Configuration of the signals
 */
#define CANTX_STRING_MUX_START_BIT (3u)
#define CANTX_STRING_MUX_LENGTH    (4u)

/** @{
 * defines of the cell voltage signal
*/
#define CANTX_SIGNAL_STRING_MAXIMUM_CELL_VOLTAGE_START_BIT (31u)
#define CANTX_SIGNAL_STRING_MAXIMUM_CELL_VOLTAGE_LENGTH    (14u)
#define CANTX_SIGNAL_STRING_MINIMUM_CELL_VOLTAGE_START_BIT (33u)
#define CANTX_SIGNAL_STRING_MINIMUM_CELL_VOLTAGE_LENGTH    (14u)
/** @} */

/** @{
 * defines of the maximum cell temperature signal
*/
#define CANTX_SIGNAL_STRING_MAXIMUM_CELL_TEMPERATURE_START_BIT (31u)
#define CANTX_SIGNAL_STRING_MAXIMUM_CELL_TEMPERATURE_LENGTH    (8u)
#define CANTX_MINIMUM_VALUE_MAXIMUM_CELL_TEMPERATURE_SIGNAL    (-1280.0f)
#define CANTX_MAXIMUM_VALUE_MAXIMUM_CELL_TEMPERATURE_SIGNAL    (1275.0f)
#define CANTX_FACTOR_MAXIMUM_CELL_TEMPERATURE                  (5.0f)
/** @} */

/** @{
 * configuration of the maximum cell temperature signal
*/
static const CAN_SIGNAL_TYPE_s cantx_signalMaximumStringCellTemperature = {
    CANTX_SIGNAL_STRING_MAXIMUM_CELL_TEMPERATURE_START_BIT,
    CANTX_SIGNAL_STRING_MAXIMUM_CELL_TEMPERATURE_LENGTH,
    CANTX_FACTOR_MAXIMUM_CELL_TEMPERATURE,
    CAN_SIGNAL_OFFSET_0,
    CANTX_MINIMUM_VALUE_MAXIMUM_CELL_TEMPERATURE_SIGNAL,
    CANTX_MAXIMUM_VALUE_MAXIMUM_CELL_TEMPERATURE_SIGNAL};
/** @} */

/** @{
 * defines of the minimum string cell temperature signal
*/
#define CANTX_SIGNAL_STRING_MINIMUM_CELL_TEMPERATURE_START_BIT (39u)
#define CANTX_SIGNAL_STRING_MINIMUM_CELL_TEMPERATURE_LENGTH    (8u)
#define CANTX_MINIMUM_VALUE_MINIMUM_CELL_TEMPERATURE_SIGNAL    (-1280.0f)
#define CANTX_MAXIMUM_VALUE_MINIMUM_CELL_TEMPERATURE_SIGNAL    (1275.0f)
#define CANTX_FACTOR_MINIMUM_CELL_TEMPERATURE                  (5.0f)
/** @} */

/** @{
 * configuration of the minimum string cell temperature signal
*/
static const CAN_SIGNAL_TYPE_s cantx_signalMinimumStringCellTemperature = {
    CANTX_SIGNAL_STRING_MINIMUM_CELL_TEMPERATURE_START_BIT,
    CANTX_SIGNAL_STRING_MINIMUM_CELL_TEMPERATURE_LENGTH,
    CANTX_FACTOR_MINIMUM_CELL_TEMPERATURE,
    CAN_SIGNAL_OFFSET_0,
    CANTX_MINIMUM_VALUE_MINIMUM_CELL_TEMPERATURE_SIGNAL,
    CANTX_MAXIMUM_VALUE_MINIMUM_CELL_TEMPERATURE_SIGNAL};
/** @} */

/** @{
 * Indices of of the regarding signals
*/
#define CANTX_SIGNAL_SENSOR_MINIMUM_START_BIT (15u)
#define CANTX_SIGNAL_SENSOR_MINIMUM_LENGTH    (4u)
#define CANTX_SIGNAL_SENSOR_MAXIMUM_START_BIT (11u)
#define CANTX_SIGNAL_SENSOR_MAXIMUM_LENGTH    (4u)
#define CANTX_SIGNAL_MODULE_MINIMUM_START_BIT (23u)
#define CANTX_SIGNAL_MODULE_MINIMUM_LENGTH    (4u)
#define CANTX_SIGNAL_MODULE_MAXIMUM_START_BIT (19u)
#define CANTX_SIGNAL_MODULE_MAXIMUM_LENGTH    (4u)
/** @} */

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/
/**
 * @brief   Calculates the return value of the maximum string temperature
 * @return  Returns the return value of the maximum string temperature
 */
static uint64_t CANTX_CalculateStringMaximumTemperature(uint8_t stringNumber, const CAN_SHIM_s *const kpkCanShim);

/**
 * @brief   Calculates the return value of the minimum string temperature
 * @return  Returns the return value of the minimum string temperature
 */
static uint64_t CANTX_CalculateStringMinimumTemperature(uint8_t stringNumber, const CAN_SHIM_s *const kpkCanShim);

/**
 * @brief   Adds the data to the message about the string values
 */
static void CANTX_BuildStringTemperatureMessage(
    uint64_t *pMessageData,
    uint8_t stringNumber,
    const CAN_SHIM_s *const kpkCanShim);

/*========== Static Function Implementations ================================*/
static uint64_t CANTX_CalculateStringMaximumTemperature(uint8_t stringNumber, const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);
    FAS_ASSERT(kpkCanShim != NULL_PTR);

    /* Maximum cell temperature */
    float_t signalData = (float_t)kpkCanShim->pTableMinMax->maximumTemperature_ddegC[stringNumber];
    CAN_TxPrepareSignalData(&signalData, cantx_signalMaximumStringCellTemperature);
    uint64_t data = (int64_t)signalData;
    return data;
}

static uint64_t CANTX_CalculateStringMinimumTemperature(uint8_t stringNumber, const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);
    FAS_ASSERT(kpkCanShim != NULL_PTR);

    /* Minimum cell temperature */
    float_t signalData = (float_t)kpkCanShim->pTableMinMax->minimumTemperature_ddegC[stringNumber];
    CAN_TxPrepareSignalData(&signalData, cantx_signalMinimumStringCellTemperature);
    uint64_t data = (int64_t)signalData;
    return data;
}

static void CANTX_BuildStringTemperatureMessage(
    uint64_t *pMessageData,
    uint8_t stringNumber,
    const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(pMessageData != NULL_PTR);
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);
    FAS_ASSERT(kpkCanShim != NULL_PTR);

    /* set mux data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(
        pMessageData, CANTX_STRING_MUX_START_BIT, CANTX_STRING_MUX_LENGTH, (uint64_t)stringNumber, CAN_BIG_ENDIAN);

    /* maximum temperature */
    uint64_t signalData = CANTX_CalculateStringMaximumTemperature(stringNumber, kpkCanShim);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        cantx_signalMaximumStringCellTemperature.bitStart,
        cantx_signalMaximumStringCellTemperature.bitLength,
        signalData,
        CAN_BIG_ENDIAN);

    /* minimum temperature */
    signalData = CANTX_CalculateStringMinimumTemperature(stringNumber, kpkCanShim);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        cantx_signalMinimumStringCellTemperature.bitStart,
        cantx_signalMinimumStringCellTemperature.bitLength,
        signalData,
        CAN_BIG_ENDIAN);

    /* Nr sensor maximum cell temperature */
    signalData = (uint64_t)kpkCanShim->pTableMinMax->nrSensorMaximumTemperature[stringNumber];
    /* set data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_SIGNAL_SENSOR_MAXIMUM_START_BIT,
        CANTX_SIGNAL_SENSOR_MAXIMUM_LENGTH,
        signalData,
        CAN_BIG_ENDIAN);

    /* Nr module maximum cell temperature */
    signalData = (uint64_t)kpkCanShim->pTableMinMax->nrModuleMaximumTemperature[stringNumber];
    /* set data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_SIGNAL_MODULE_MAXIMUM_START_BIT,
        CANTX_SIGNAL_MODULE_MAXIMUM_LENGTH,
        signalData,
        CAN_BIG_ENDIAN);

    /* Nr sensor minimum cell temperature */
    signalData = (uint64_t)kpkCanShim->pTableMinMax->nrSensorMinimumTemperature[stringNumber];
    /* set data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_SIGNAL_SENSOR_MINIMUM_START_BIT,
        CANTX_SIGNAL_SENSOR_MINIMUM_LENGTH,
        signalData,
        CAN_BIG_ENDIAN);

    /* Nr module minimum cell temperature */
    signalData = (uint64_t)kpkCanShim->pTableMinMax->nrModuleMinimumTemperature[stringNumber];
    /* set data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_SIGNAL_MODULE_MINIMUM_START_BIT,
        CANTX_SIGNAL_MODULE_MINIMUM_LENGTH,
        signalData,
        CAN_BIG_ENDIAN);
}

static void CANTX_BuildStringVoltageMessage(
    uint64_t *pMessageData,
    uint8_t stringNumber,
    const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(pMessageData != NULL_PTR);
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);
    FAS_ASSERT(kpkCanShim != NULL_PTR);

    /* set data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(
        pMessageData, CANTX_STRING_MUX_START_BIT, CANTX_STRING_MUX_LENGTH, (uint64_t)stringNumber, CAN_BIG_ENDIAN);

    /* maximum cell voltage */
    uint64_t signalData = (uint64_t)kpkCanShim->pTableMinMax->maximumCellVoltage_mV[stringNumber];
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_SIGNAL_STRING_MAXIMUM_CELL_VOLTAGE_START_BIT,
        CANTX_SIGNAL_STRING_MAXIMUM_CELL_VOLTAGE_LENGTH,
        signalData,
        CAN_BIG_ENDIAN);

    /* minimum cell voltage */
    signalData = (uint64_t)kpkCanShim->pTableMinMax->minimumCellVoltage_mV[stringNumber];
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_SIGNAL_STRING_MINIMUM_CELL_VOLTAGE_START_BIT,
        CANTX_SIGNAL_STRING_MINIMUM_CELL_VOLTAGE_LENGTH,
        signalData,
        CAN_BIG_ENDIAN);

    /* Nr cell maximum cell Voltage */
    signalData = (uint64_t)kpkCanShim->pTableMinMax->nrCellMaximumCellVoltage[stringNumber];
    /* set data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_SIGNAL_SENSOR_MAXIMUM_START_BIT,
        CANTX_SIGNAL_SENSOR_MAXIMUM_LENGTH,
        signalData,
        CAN_BIG_ENDIAN);

    /* Nr module maximum cell Voltage */
    signalData = (uint64_t)kpkCanShim->pTableMinMax->nrModuleMaximumCellVoltage[stringNumber];
    /* set data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_SIGNAL_MODULE_MAXIMUM_START_BIT,
        CANTX_SIGNAL_MODULE_MAXIMUM_LENGTH,
        signalData,
        CAN_BIG_ENDIAN);

    /* Nr cell minimum cell Voltage */
    signalData = (uint64_t)kpkCanShim->pTableMinMax->nrCellMinimumCellVoltage[stringNumber];
    /* set data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_SIGNAL_SENSOR_MINIMUM_START_BIT,
        CANTX_SIGNAL_SENSOR_MINIMUM_LENGTH,
        signalData,
        CAN_BIG_ENDIAN);

    /* Nr module minimum cell Voltage */
    signalData = (uint64_t)kpkCanShim->pTableMinMax->nrModuleMinimumCellVoltage[stringNumber];
    /* set data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_SIGNAL_MODULE_MINIMUM_START_BIT,
        CANTX_SIGNAL_MODULE_MINIMUM_LENGTH,
        signalData,
        CAN_BIG_ENDIAN);
}

/*========== Extern Function Implementations ================================*/
extern uint32_t CANTX_StringMinimumMaximumVoltage(
    CAN_MESSAGE_PROPERTIES_s message,
    uint8_t *pCanData,
    uint8_t *pMuxId,
    const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(message.id == CANTX_STRING_MIN_MAX_CELL_VOLTAGE_ID);
    FAS_ASSERT(message.idType == CANTX_STRING_MIN_MAX_CELL_VOLTAGE_ID_TYPE);
    FAS_ASSERT(message.dlc == CAN_FOXBMS_MESSAGES_DEFAULT_DLC);
    FAS_ASSERT(message.endianness == CANTX_STRING_MIN_MAX_CELL_VOLTAGE_ENDIANNESS);
    FAS_ASSERT(pCanData != NULL_PTR);
    FAS_ASSERT(pMuxId != NULL_PTR);
    FAS_ASSERT(*pMuxId < BS_NR_OF_STRINGS);
    FAS_ASSERT(kpkCanShim != NULL_PTR);
    uint64_t voltageMessageData = 0u;

    (void)DATA_READ_DATA(kpkCanShim->pTableMinMax);

    /** Database entry with minimum and maximum values does not need to be read
     *  within this callback as it is already read by function
     *  #CANTX_StringMinimumMaximumTemp */
    const uint8_t stringNumber = *pMuxId;

    /* adding data to the messages */
    CANTX_BuildStringVoltageMessage(&voltageMessageData, stringNumber, kpkCanShim);

    /* now copy data in the buffer that will be use to send data */
    CAN_TxSetCanDataWithMessageData(voltageMessageData, pCanData, message.endianness);

    /* Increment multiplexer for next cell */
    (*pMuxId)++;

    /* Check mux value */
    if (*pMuxId >= BS_NR_OF_STRINGS) {
        *pMuxId = 0u;
    }

    return 0u;
}

extern uint32_t CANTX_StringMinimumMaximumTemp(
    CAN_MESSAGE_PROPERTIES_s message,
    uint8_t *pCanData,
    uint8_t *pMuxId,
    const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(message.id == CANTX_STRING_MIN_MAX_CELL_TEMPERATURE_ID);
    FAS_ASSERT(message.idType == CANTX_STRING_MIN_MAX_CELL_TEMPERATURE_ID_TYPE);
    FAS_ASSERT(message.dlc == CAN_FOXBMS_MESSAGES_DEFAULT_DLC);
    FAS_ASSERT(message.endianness == CANTX_STRING_MIN_MAX_CELL_TEMPERATURE_ENDIANNESS);
    FAS_ASSERT(pCanData != NULL_PTR);
    FAS_ASSERT(pMuxId != NULL_PTR);
    FAS_ASSERT(*pMuxId < BS_NR_OF_STRINGS);
    FAS_ASSERT(kpkCanShim != NULL_PTR);
    uint64_t temperatureMessageData = 0u;

    (void)DATA_READ_DATA(kpkCanShim->pTableMinMax);

    /** Database entry with minimum and maximum values does not need to be read
     *  within this callback as it is already read by function
     *  #CANTX_StringMinimumMaximumVoltage */
    const uint8_t stringNumber = *pMuxId;

    /* adding data to the messages */
    CANTX_BuildStringTemperatureMessage(&temperatureMessageData, stringNumber, kpkCanShim);

    /* now copy data in the buffer that will be use to send data */
    CAN_TxSetCanDataWithMessageData(temperatureMessageData, pCanData, message.endianness);

    /* Increment multiplexer for next cell */
    (*pMuxId)++;

    /* Check mux value */
    if (*pMuxId >= BS_NR_OF_STRINGS) {
        *pMuxId = 0u;
    }

    return 0u;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
extern uint64_t TEST_CANTX_CalculateStringMaximumTemperature(uint8_t stringNumber, const CAN_SHIM_s *const kpkCanShim) {
    return CANTX_CalculateStringMaximumTemperature(stringNumber, kpkCanShim);
}
extern uint64_t TEST_CANTX_CalculateStringMinimumTemperature(uint8_t stringNumber, const CAN_SHIM_s *const kpkCanShim) {
    return CANTX_CalculateStringMinimumTemperature(stringNumber, kpkCanShim);
}
extern void TEST_CANTX_BuildStringTemperatureMessage(
    uint64_t *pMessageData,
    uint8_t stringNumber,
    const CAN_SHIM_s *const kpkCanShim) {
    CANTX_BuildStringTemperatureMessage(pMessageData, stringNumber, kpkCanShim);
}
extern void TEST_CANTX_BuildStringVoltageMessage(
    uint64_t *pMessageData,
    uint8_t stringNumber,
    const CAN_SHIM_s *const kpkCanShim) {
    CANTX_BuildStringVoltageMessage(pMessageData, stringNumber, kpkCanShim);
}

#endif
