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
 * @file    can_cbs_tx_string-state-estimation.c
 * @author  foxBMS Team
 * @date    2021-07-21 (date of creation)
 * @updated 2025-03-31 (date of last update)
 * @version v1.9.0
 * @ingroup DRIVERS
 * @prefix  CANTX
 *
 * @brief   CAN driver Tx callback implementation
 * @details CAN Tx callback for state estimation messages
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
#define CANTX_STRING_MUX_START_BIT     (7u)
#define CANTX_SIGNAL_STRING_MUX_LENGTH (4u)

/** @{
 * defines of the minimum soc signal
*/
#define CANTX_SIGNAL_MINIMUM_SOC_START_BIT     (3u)
#define CANTX_SIGNAL_MINIMUM_SOC_LENGTH        (9u)
#define CANTX_MINIMUM_VALUE_MINIMUM_SOC_SIGNAL (0.0f)
#define CANTX_MAXIMUM_VALUE_MINIMUM_SOC_SIGNAL (100.0f)
#define CANTX_FACTOR_MINIMUM_SOC               (0.2f)
/** @} */

/** @{
 * configuration of the minimum soc signal
*/
static const CAN_SIGNAL_TYPE_s cantx_signalMinimumStringSoc = {
    CANTX_SIGNAL_MINIMUM_SOC_START_BIT,
    CANTX_SIGNAL_MINIMUM_SOC_LENGTH,
    CANTX_FACTOR_MINIMUM_SOC,
    CAN_SIGNAL_OFFSET_0,
    CANTX_MINIMUM_VALUE_MINIMUM_SOC_SIGNAL,
    CANTX_MAXIMUM_VALUE_MINIMUM_SOC_SIGNAL};
/** @} */

/** @{
 * defines of the maximum soc signal
*/
#define CANTX_SIGNAL_MAXIMUM_SOC_START_BIT     (10u)
#define CANTX_SIGNAL_MAXIMUM_SOC_LENGTH        (9u)
#define CANTX_MINIMUM_VALUE_MAXIMUM_SOC_SIGNAL (0.0f)
#define CANTX_MAXIMUM_VALUE_MAXIMUM_SOC_SIGNAL (100.0f)
#define CANTX_FACTOR_MAXIMUM_SOC               (0.2f)
/** @} */

/** @{
 * configuration of the maximum soc signal
*/
static const CAN_SIGNAL_TYPE_s cantx_signalMaximumStringSoc = {
    CANTX_SIGNAL_MAXIMUM_SOC_START_BIT,
    CANTX_SIGNAL_MAXIMUM_SOC_LENGTH,
    CANTX_FACTOR_MAXIMUM_SOC,
    CAN_SIGNAL_OFFSET_0,
    CANTX_MINIMUM_VALUE_MAXIMUM_SOC_SIGNAL,
    CANTX_MAXIMUM_VALUE_MAXIMUM_SOC_SIGNAL};
/** @} */

/** @{
 * defines of the minimum soe signal
*/
#define CANTX_SIGNAL_MINIMUM_SOE_START_BIT     (17u)
#define CANTX_SIGNAL_MINIMUM_SOE_LENGTH        (9u)
#define CANTX_MINIMUM_VALUE_MINIMUM_SOE_SIGNAL (0.0f)
#define CANTX_MAXIMUM_VALUE_MINIMUM_SOE_SIGNAL (100.0f)
#define CANTX_FACTOR_MINIMUM_SOE               (0.2f)
/** @} */

/** @{
 * configuration of the minimum soe signal
*/
static const CAN_SIGNAL_TYPE_s cantx_signalMinimumStringSoe = {
    CANTX_SIGNAL_MINIMUM_SOE_START_BIT,
    CANTX_SIGNAL_MINIMUM_SOE_LENGTH,
    CANTX_FACTOR_MINIMUM_SOE,
    CAN_SIGNAL_OFFSET_0,
    CANTX_MINIMUM_VALUE_MINIMUM_SOE_SIGNAL,
    CANTX_MAXIMUM_VALUE_MINIMUM_SOE_SIGNAL};
/** @} */

/** @{
 * defines of the maximum soe signal
*/
#define CANTX_SIGNAL_MAXIMUM_SOE_START_BIT     (24u)
#define CANTX_SIGNAL_MAXIMUM_SOE_LENGTH        (9u)
#define CANTX_MINIMUM_VALUE_MAXIMUM_SOE_SIGNAL (0.0f)
#define CANTX_MAXIMUM_VALUE_MAXIMUM_SOE_SIGNAL (100.0f)
#define CANTX_FACTOR_MAXIMUM_SOE               (0.2f)
/** @} */

/** @{
 * configuration of the maximum soe signal
*/
static const CAN_SIGNAL_TYPE_s cantx_signalMaximumStringSoe = {
    CANTX_SIGNAL_MAXIMUM_SOE_START_BIT,
    CANTX_SIGNAL_MAXIMUM_SOE_LENGTH,
    CANTX_FACTOR_MAXIMUM_SOE,
    CAN_SIGNAL_OFFSET_0,
    CANTX_MINIMUM_VALUE_MAXIMUM_SOE_SIGNAL,
    CANTX_MAXIMUM_VALUE_MAXIMUM_SOE_SIGNAL};
/** @} */

/** @{
 * defines of the soh signal
*/
#define CANTX_SIGNAL_SOH_START_BIT     (47u)
#define CANTX_SIGNAL_SOH_LENGTH        (9u)
#define CANTX_MINIMUM_VALUE_SOH_SIGNAL (0.0f)
#define CANTX_MAXIMUM_VALUE_SOH_SIGNAL (100.0f)
#define CANTX_FACTOR_SOH               (0.2f)
/** @} */

/** @{
 * configuration of the soh signal
*/
static const CAN_SIGNAL_TYPE_s cantx_signalStringSoh = {
    CANTX_SIGNAL_SOH_START_BIT,
    CANTX_SIGNAL_SOH_LENGTH,
    CANTX_FACTOR_SOH,
    CAN_SIGNAL_OFFSET_0,
    CANTX_MINIMUM_VALUE_SOH_SIGNAL,
    CANTX_MAXIMUM_VALUE_SOH_SIGNAL};
/** @} */

/** @{
 * defines of the energy signal
*/
#define CANTX_SIGNAL_ENERGY_START_BIT     (54u)
#define CANTX_SIGNAL_ENERGY_LENGTH        (15u)
#define CANTX_MINIMUM_VALUE_ENERGY_SIGNAL (0.0f)
#define CANTX_MAXIMUM_VALUE_ENERGY_SIGNAL (327670.0f)
/** @} */

/** @{
 * configuration of the energy signal
*/
static const CAN_SIGNAL_TYPE_s cantx_signalStringEnergy = {
    CANTX_SIGNAL_ENERGY_START_BIT,
    CANTX_SIGNAL_ENERGY_LENGTH,
    UNIT_CONVERSION_FACTOR_10_FLOAT,
    CAN_SIGNAL_OFFSET_0,
    CANTX_MINIMUM_VALUE_ENERGY_SIGNAL,
    CANTX_MAXIMUM_VALUE_ENERGY_SIGNAL};
/** @} */

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/
/**
 * @brief Calculates the return value for the strings minimum SOC
 * @return return value for minimum SOC
 */
static uint64_t CANTX_CalculateMinimumStringSoc(uint8_t stringNumber, const CAN_SHIM_s *const kpkCanShim);

/**
 * @brief Calculates the return value for the strings maximum SOC
 * @return return value for maximum SOC
 */
static uint64_t CANTX_CalculateMaximumStringSoc(uint8_t stringNumber, const CAN_SHIM_s *const kpkCanShim);

/**
 * @brief Calculates the return value for the strings minimum SOE
 * @return return value for minimum SOE
 */
static uint64_t CANTX_CalculateMinimumStringSoe(uint8_t stringNumber, const CAN_SHIM_s *const kpkCanShim);

/**
 * @brief Calculates the return value for the strings maximum SOE
 * @return return value for maximum SOE
 */
static uint64_t CANTX_CalculateMaximumStringSoe(uint8_t stringNumber, const CAN_SHIM_s *const kpkCanShim);

/**
 * @brief Calculates the return value for the strings SOH
 * @return return value for SOH
 */
static uint64_t CANTX_CalculateStringSoh(uint8_t stringNumber, const CAN_SHIM_s *const kpkCanShim);

/**
 * @brief Calculates the return value for the strings energy
 * @return return value for energy
 */
static uint64_t CANTX_CalculateStringEnergy(uint8_t stringNumber, const CAN_SHIM_s *const kpkCanShim);

/**
 * @brief Builds the message form the data
 */
static void CANTX_BuildStringStateEstimationMessage(
    uint64_t *pMessageData,
    uint8_t stringNumber,
    const CAN_SHIM_s *const kpkCanShim);

/*========== Static Function Implementations ================================*/
static uint64_t CANTX_CalculateMinimumStringSoc(uint8_t stringNumber, const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);
    FAS_ASSERT(kpkCanShim != NULL_PTR);

    /* Minimum SOC */
    float_t signalData = kpkCanShim->pTableSoc->minimumSoc_perc[stringNumber];
    CAN_TxPrepareSignalData(&signalData, cantx_signalMinimumStringSoc);
    uint64_t data = (uint64_t)signalData;
    return data;
}

static uint64_t CANTX_CalculateMaximumStringSoc(uint8_t stringNumber, const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);
    FAS_ASSERT(kpkCanShim != NULL_PTR);

    /* Maximum SOC */
    float_t signalData = kpkCanShim->pTableSoc->maximumSoc_perc[stringNumber];
    CAN_TxPrepareSignalData(&signalData, cantx_signalMaximumStringSoc);
    uint64_t data = (uint64_t)signalData;
    return data;
}

static uint64_t CANTX_CalculateMinimumStringSoe(uint8_t stringNumber, const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);
    FAS_ASSERT(kpkCanShim != NULL_PTR);

    /* Minimum SOE */
    float_t signalData = kpkCanShim->pTableSoe->minimumSoe_perc[stringNumber];
    CAN_TxPrepareSignalData(&signalData, cantx_signalMinimumStringSoe);
    uint64_t data = (uint64_t)signalData;
    return data;
}

static uint64_t CANTX_CalculateMaximumStringSoe(uint8_t stringNumber, const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);
    FAS_ASSERT(kpkCanShim != NULL_PTR);

    /* Maximum SOE */
    float_t signalData = kpkCanShim->pTableSoe->maximumSoe_perc[stringNumber];
    CAN_TxPrepareSignalData(&signalData, cantx_signalMaximumStringSoe);
    uint64_t data = (uint64_t)signalData;
    return data;
}

static uint64_t CANTX_CalculateStringSoh(uint8_t stringNumber, const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);
    FAS_ASSERT(kpkCanShim != NULL_PTR);

    /* SOH */
    float_t signalData = 100.0f;
    CAN_TxPrepareSignalData(&signalData, cantx_signalStringSoh);
    uint64_t data = (uint64_t)signalData;
    return data;
}

static uint64_t CANTX_CalculateStringEnergy(uint8_t stringNumber, const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);
    FAS_ASSERT(kpkCanShim != NULL_PTR);

    /* String energy */
    float_t signalData = kpkCanShim->pTableSoe->minimumSoe_Wh[stringNumber];
    CAN_TxPrepareSignalData(&signalData, cantx_signalStringEnergy);
    uint64_t data = (uint64_t)signalData;
    return data;
}

static void CANTX_BuildStringStateEstimationMessage(
    uint64_t *pMessageData,
    uint8_t stringNumber,
    const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(pMessageData != NULL_PTR);
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);
    FAS_ASSERT(kpkCanShim != NULL_PTR);

    /* String MUX */
    uint64_t data = (uint64_t)stringNumber;
    CAN_TxSetMessageDataWithSignalData(
        pMessageData, CANTX_STRING_MUX_START_BIT, CANTX_SIGNAL_STRING_MUX_LENGTH, data, CAN_BIG_ENDIAN);

    /* Minimum SOC */
    data = CANTX_CalculateMinimumStringSoc(stringNumber, kpkCanShim);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        cantx_signalMinimumStringSoc.bitStart,
        cantx_signalMinimumStringSoc.bitLength,
        data,
        CAN_BIG_ENDIAN);

    /* Maximum SOC */
    data = CANTX_CalculateMaximumStringSoc(stringNumber, kpkCanShim);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        cantx_signalMaximumStringSoc.bitStart,
        cantx_signalMaximumStringSoc.bitLength,
        data,
        CAN_BIG_ENDIAN);

    /* Minimum SOE */
    data = CANTX_CalculateMinimumStringSoe(stringNumber, kpkCanShim);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        cantx_signalMinimumStringSoe.bitStart,
        cantx_signalMinimumStringSoe.bitLength,
        data,
        CAN_BIG_ENDIAN);

    /* Maximum SOE */
    data = CANTX_CalculateMaximumStringSoe(stringNumber, kpkCanShim);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        cantx_signalMaximumStringSoe.bitStart,
        cantx_signalMaximumStringSoe.bitLength,
        data,
        CAN_BIG_ENDIAN);

    /* SOH */
    data = CANTX_CalculateStringSoh(stringNumber, kpkCanShim);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData, cantx_signalStringSoh.bitStart, cantx_signalStringSoh.bitLength, data, CAN_BIG_ENDIAN);

    /* ENERGY */
    data = CANTX_CalculateStringEnergy(stringNumber, kpkCanShim);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData, cantx_signalStringEnergy.bitStart, cantx_signalStringEnergy.bitLength, data, CAN_BIG_ENDIAN);
}

/*========== Extern Function Implementations ================================*/

extern uint32_t CANTX_StringStateEstimation(
    CAN_MESSAGE_PROPERTIES_s message,
    uint8_t *pCanData,
    uint8_t *pMuxId,
    const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(message.id == CANTX_STRING_STATE_ESTIMATION_ID);
    FAS_ASSERT(message.idType == CANTX_STRING_STATE_ESTIMATION_ID_TYPE);
    FAS_ASSERT(message.dlc == CAN_FOXBMS_MESSAGES_DEFAULT_DLC);
    FAS_ASSERT(message.endianness == CANTX_STRING_STATE_ESTIMATION_ENDIANNESS);
    FAS_ASSERT(pCanData != NULL_PTR);
    FAS_ASSERT(pMuxId != NULL_PTR);
    FAS_ASSERT(*pMuxId < BS_NR_OF_STRINGS);
    FAS_ASSERT(kpkCanShim != NULL_PTR);
    uint64_t messageData = 0u;

    /** Database entry with state estimation values does not need to be read
     *  within this callback as it is already read by function
     *  #CANTX_PackStateEstimation */
    const uint8_t stringNumber = *pMuxId;

    /* build the message */
    CANTX_BuildStringStateEstimationMessage(&messageData, stringNumber, kpkCanShim);

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
extern uint64_t TEST_CANTX_CalculateMinimumStringSoc(uint8_t stringNumber, const CAN_SHIM_s *const kpkCanShim) {
    return CANTX_CalculateMinimumStringSoc(stringNumber, kpkCanShim);
}
extern uint64_t TEST_CANTX_CalculateMaximumStringSoc(uint8_t stringNumber, const CAN_SHIM_s *const kpkCanShim) {
    return CANTX_CalculateMaximumStringSoc(stringNumber, kpkCanShim);
}
extern uint64_t TEST_CANTX_CalculateMinimumStringSoe(uint8_t stringNumber, const CAN_SHIM_s *const kpkCanShim) {
    return CANTX_CalculateMinimumStringSoe(stringNumber, kpkCanShim);
}
extern uint64_t TEST_CANTX_CalculateMaximumStringSoe(uint8_t stringNumber, const CAN_SHIM_s *const kpkCanShim) {
    return CANTX_CalculateMaximumStringSoe(stringNumber, kpkCanShim);
}
extern uint64_t TEST_CANTX_CalculateStringSoh(uint8_t stringNumber, const CAN_SHIM_s *const kpkCanShim) {
    return CANTX_CalculateStringSoh(stringNumber, kpkCanShim);
}
extern uint64_t TEST_CANTX_CalculateStringEnergy(uint8_t stringNumber, const CAN_SHIM_s *const kpkCanShim) {
    return CANTX_CalculateStringEnergy(stringNumber, kpkCanShim);
}
extern void TEST_CANTX_BuildStringStateEstimationMessage(
    uint64_t *pMessageData,
    uint8_t stringNumber,
    const CAN_SHIM_s *const kpkCanShim) {
    CANTX_BuildStringStateEstimationMessage(pMessageData, stringNumber, kpkCanShim);
}
#endif
