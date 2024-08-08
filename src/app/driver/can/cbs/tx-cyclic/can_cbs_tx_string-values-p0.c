/**
 *
 * @copyright &copy; 2010 - 2024, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * @file    can_cbs_tx_string-values-p0.c
 * @author  foxBMS Team
 * @date    2023-05-31 (date of creation)
 * @updated 2024-08-08 (date of last update)
 * @version v1.7.0
 * @ingroup DRIVERS
 * @prefix  CANTX
 *
 * @brief   CAN driver Tx callback implementation
 * @details CAN Tx callback for pack value and string value messages
 */

/*========== Includes =======================================================*/
#include "can_cbs_tx_cyclic.h"
#include "can_cfg_tx-cyclic-message-definitions.h"
#include "can_helper.h"
#include "foxmath.h"

#include <math.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/
/* configuration of the StringP0 message bits */
#define CANTX_STRING_P0_MUX_START_BIT (7u)
#define CANTX_STRING_P0_MUX_LENGTH    (3u)

/** @{
 * defines of the string voltage signal
*/
#define CANTX_STRING_P0_VOLTAGE_START_BIT         (4u)
#define CANTX_STRING_P0_VOLTAGE_LENGTH            (18u)
#define CANTX_MINIMUM_VALUE_STRING_VOLTAGE_SIGNAL (-1310720.0f)
#define CANTX_MAXIMUM_VALUE_STRING_VOLTAGE_SIGNAL (1310710.0f)
/** @} */

/** @{
 * configuration of the string voltage signal
*/
static const CAN_SIGNAL_TYPE_s cantx_signalStringVoltage = {
    CANTX_STRING_P0_VOLTAGE_START_BIT,
    CANTX_STRING_P0_VOLTAGE_LENGTH,
    UNIT_CONVERSION_FACTOR_10_FLOAT,
    CAN_SIGNAL_OFFSET_0,
    CANTX_MINIMUM_VALUE_STRING_VOLTAGE_SIGNAL,
    CANTX_MAXIMUM_VALUE_STRING_VOLTAGE_SIGNAL};
/** @} */

/** @{
 * defines of the string current signal
*/
#define CANTX_STRING_P0_CURRENT_START_BIT         (18u)
#define CANTX_STRING_P0_CURRENT_LENGTH            (18u)
#define CANTX_MINIMUM_VALUE_STRING_CURRENT_SIGNAL (-1310720.0f)
#define CANTX_MAXIMUM_VALUE_STRING_CURRENT_SIGNAL (1310710.0f)
/** @} */

/** @{
 * configuration of the string current signal
*/
static const CAN_SIGNAL_TYPE_s cantx_signalStringCurrent = {
    CANTX_STRING_P0_CURRENT_START_BIT,
    CANTX_STRING_P0_CURRENT_LENGTH,
    UNIT_CONVERSION_FACTOR_10_FLOAT,
    CAN_SIGNAL_OFFSET_0,
    CANTX_MINIMUM_VALUE_STRING_CURRENT_SIGNAL,
    CANTX_MAXIMUM_VALUE_STRING_CURRENT_SIGNAL};
/** @} */

/** @{
 * defines for the string power signal
*/
#define CANTX_STRING_P0_POWER_START_BIT         (32u)
#define CANTX_STRING_P0_POWER_LENGTH            (18u)
#define CANTX_MINIMUM_VALUE_STRING_POWER_SIGNAL (-1310720.0f)
#define CANTX_MAXIMUM_VALUE_STRING_POWER_SIGNAL (1310710.0f)
/** @} */

/** @{
 * configuration of the string power signal
*/
static const CAN_SIGNAL_TYPE_s cantx_signalStringPower = {
    CANTX_STRING_P0_POWER_START_BIT,
    CANTX_STRING_P0_POWER_LENGTH,
    UNIT_CONVERSION_FACTOR_10_FLOAT,
    CAN_SIGNAL_OFFSET_0,
    CANTX_MINIMUM_VALUE_STRING_POWER_SIGNAL,
    CANTX_MAXIMUM_VALUE_STRING_POWER_SIGNAL};
/** @} */

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/
/**
 * @brief calculates the return value of the voltage
 * @return returns the return value of the voltage
 */
static uint64_t CANTX_CalculateStringVoltage(uint8_t stringNumber, const CAN_SHIM_s *const kpkCanShim);

/**
 * @brief calculates the return value of the current
 * @return returns the return value of the current
 */
static uint64_t CANTX_CalculateStringCurrent(uint8_t stringNumber, const CAN_SHIM_s *const kpkCanShim);

/**
 * @brief calculates the return value of the power
 * @return returns the return value of the power
 */
static uint64_t CANTX_CalculateStringPower(uint8_t stringNumber, const CAN_SHIM_s *const kpkCanShim);

/**
 * @brief sets the power signal of the StringP0 message
 */
static void CANTX_BuildString0Message(uint64_t *pMessageData, uint8_t stringNumber, const CAN_SHIM_s *const kpkCanShim);

/*========== Static Function Implementations ================================*/
static uint64_t CANTX_CalculateStringVoltage(uint8_t stringNumber, const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);
    FAS_ASSERT(kpkCanShim != NULL_PTR);

    /* String voltage */
    float_t signalData = (float_t)kpkCanShim->pTablePackValues->stringVoltage_mV[stringNumber];
    CAN_TxPrepareSignalData(&signalData, cantx_signalStringVoltage);
    uint64_t data = (uint64_t)signalData;
    return data;
}

static uint64_t CANTX_CalculateStringCurrent(uint8_t stringNumber, const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);
    FAS_ASSERT(kpkCanShim != NULL_PTR);

    /* String current */
    float_t signalData = (float_t)kpkCanShim->pTablePackValues->stringCurrent_mA[stringNumber];
    CAN_TxPrepareSignalData(&signalData, cantx_signalStringCurrent);
    uint64_t data = (uint64_t)signalData;
    return data;
}

static uint64_t CANTX_CalculateStringPower(uint8_t stringNumber, const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);
    FAS_ASSERT(kpkCanShim != NULL_PTR);

    /* String power */
    float_t signalData = (float_t)kpkCanShim->pTablePackValues->stringPower_W[stringNumber];
    CAN_TxPrepareSignalData(&signalData, cantx_signalStringPower);
    uint64_t data = (uint64_t)signalData;
    return data;
}

static void CANTX_BuildString0Message(
    uint64_t *pMessageData,
    uint8_t stringNumber,
    const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(pMessageData != NULL_PTR);
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);
    FAS_ASSERT(kpkCanShim != NULL_PTR);

    /* mux value */
    uint64_t signalData = stringNumber;
    CAN_TxSetMessageDataWithSignalData(
        pMessageData, CANTX_STRING_P0_MUX_START_BIT, CANTX_STRING_P0_MUX_LENGTH, signalData, CAN_BIG_ENDIAN);

    /* String voltage */
    signalData = CANTX_CalculateStringVoltage(stringNumber, kpkCanShim);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        cantx_signalStringVoltage.bitStart,
        cantx_signalStringVoltage.bitLength,
        signalData,
        CAN_BIG_ENDIAN);

    /* String Current */
    signalData = CANTX_CalculateStringCurrent(stringNumber, kpkCanShim);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        cantx_signalStringCurrent.bitStart,
        cantx_signalStringCurrent.bitLength,
        signalData,
        CAN_BIG_ENDIAN);

    /* String Power */
    signalData = CANTX_CalculateStringPower(stringNumber, kpkCanShim);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData, cantx_signalStringPower.bitStart, cantx_signalStringPower.bitLength, signalData, CAN_BIG_ENDIAN);
}

/*========== Extern Function Implementations ================================*/
extern uint32_t CANTX_StringValuesP0(
    CAN_MESSAGE_PROPERTIES_s message,
    uint8_t *pCanData,
    uint8_t *pMuxId,
    const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(message.id == CANTX_STRING_VALUES_P0_ID);
    FAS_ASSERT(message.idType == CANTX_STRING_VALUES_P0_ID_TYPE);
    FAS_ASSERT(message.dlc == CAN_FOXBMS_MESSAGES_DEFAULT_DLC);
    FAS_ASSERT(message.endianness == CANTX_STRING_VALUES_P0_ENDIANNESS);
    FAS_ASSERT(pCanData != NULL_PTR);
    FAS_ASSERT(pMuxId != NULL_PTR);
    FAS_ASSERT(*pMuxId < BS_NR_OF_STRINGS);
    FAS_ASSERT(kpkCanShim != NULL_PTR);
    uint64_t messageData = 0u;

    const uint8_t stringNumber = *pMuxId;

    /* First signal to transmit cell voltages: get database values */
    if (stringNumber == 0u) {
        /* Do not read pTableMsl and pTableErrorState as they already are read
         * with a higher frequency from CANTX_BmsState callback */
        DATA_READ_DATA(kpkCanShim->pTablePackValues);
    }

    /* set message data */
    CANTX_BuildString0Message(&messageData, stringNumber, kpkCanShim);

    /* now copy data in the buffer that will be used to send data */
    CAN_TxSetCanDataWithMessageData(messageData, pCanData, message.endianness);

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
extern uint64_t TEST_CANTX_CalculateStringVoltage(uint8_t stringNumber, const CAN_SHIM_s *const kpkCanShim) {
    return CANTX_CalculateStringVoltage(stringNumber, kpkCanShim);
}
extern uint64_t TEST_CANTX_CalculateStringCurrent(uint8_t stringNumber, const CAN_SHIM_s *const kpkCanShim) {
    return CANTX_CalculateStringCurrent(stringNumber, kpkCanShim);
}
extern uint64_t TEST_CANTX_CalculateStringPower(uint8_t stringNumber, const CAN_SHIM_s *const kpkCanShim) {
    return CANTX_CalculateStringPower(stringNumber, kpkCanShim);
}
extern void TEST_CANTX_BuildString0Message(
    uint64_t *pMessageData,
    uint8_t stringNumber,
    const CAN_SHIM_s *const kpkCanShim) {
    CANTX_BuildString0Message(pMessageData, stringNumber, kpkCanShim);
}
#endif
