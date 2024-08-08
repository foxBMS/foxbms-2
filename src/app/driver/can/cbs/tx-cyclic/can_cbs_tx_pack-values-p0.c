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
 * @file    can_cbs_tx_pack-values-p0.c
 * @author  foxBMS Team
 * @date    2021-07-21 (date of creation)
 * @updated 2024-08-08 (date of last update)
 * @version v1.7.0
 * @ingroup DRIVERS
 * @prefix  CANTX
 *
 * @brief   CAN driver Tx callback implementation
 * @details CAN Tx callback for pack value and string value messages
 */

/*========== Includes =======================================================*/
/* AXIVION Next Codeline Generic-LocalInclude: 'can_cbs_tx_cyclic.h' declares
 * the prototype for the callback 'CANTX_PackValuesP0' */
#include "can_cbs_tx_cyclic.h"
#include "can_cfg_tx-cyclic-message-definitions.h"
#include "can_helper.h"
#include "foxmath.h"

#include <math.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/
/** @{
 * defines of the battery voltage signal
*/
#define CANTX_PACK_P0_BATTERY_VOLTAGE_START_BIT    (7u)
#define CANTX_PACK_P0_BATTERY_VOLTAGE_LENGTH       (15u)
#define CANTX_MINIMUM_VALUE_BATTERY_VOLTAGE_SIGNAL (-1638400.0f)
#define CANTX_MAXIMUM_VALUE_BATTERY_VOLTAGE_SIGNAL (1638300.0f)
/** @} */

/** @{
 * configuration of the battery voltage signal
*/
static const CAN_SIGNAL_TYPE_s cantx_signalBatteryVoltage = {
    CANTX_PACK_P0_BATTERY_VOLTAGE_START_BIT,
    CANTX_PACK_P0_BATTERY_VOLTAGE_LENGTH,
    UNIT_CONVERSION_FACTOR_100_FLOAT,
    CAN_SIGNAL_OFFSET_0,
    CANTX_MINIMUM_VALUE_BATTERY_VOLTAGE_SIGNAL,
    CANTX_MAXIMUM_VALUE_BATTERY_VOLTAGE_SIGNAL};
/** @} */

/** @{
 * defines of the bus voltage signal
*/
#define CANTX_PACK_P0_BUS_VOLTAGE_START_BIT    (8u)
#define CANTX_PACK_P0_BUS_VOLTAGE_LENGTH       (15u)
#define CANTX_MINIMUM_VALUE_BUS_VOLTAGE_SIGNAL (-1638400.0f)
#define CANTX_MAXIMUM_VALUE_BUS_VOLTAGE_SIGNAL (1638300.0f)
/** @} */

/** @{
 * configuration of the bus voltage signal
*/
static const CAN_SIGNAL_TYPE_s cantx_signalBusVoltage = {
    CANTX_PACK_P0_BUS_VOLTAGE_START_BIT,
    CANTX_PACK_P0_BUS_VOLTAGE_LENGTH,
    UNIT_CONVERSION_FACTOR_100_FLOAT,
    CAN_SIGNAL_OFFSET_0,
    CANTX_MINIMUM_VALUE_BUS_VOLTAGE_SIGNAL,
    CANTX_MAXIMUM_VALUE_BUS_VOLTAGE_SIGNAL};
/** @} */

/** @{
 * defines of the power signal
*/
#define CANTX_PACK_P0_POWER_START_BIT    (25u)
#define CANTX_PACK_P0_POWER_LENGTH       (17u)
#define CANTX_MINIMUM_VALUE_POWER_SIGNAL (-655360.0f)
#define CANTX_MAXIMUM_VALUE_POWER_SIGNAL (655350.0f)
/** @} */

/** @{
 * configuration of the power signal
*/
static const CAN_SIGNAL_TYPE_s cantx_signalPower = {
    CANTX_PACK_P0_POWER_START_BIT,
    CANTX_PACK_P0_POWER_LENGTH,
    UNIT_CONVERSION_FACTOR_10_FLOAT,
    CAN_SIGNAL_OFFSET_0,
    CANTX_MINIMUM_VALUE_POWER_SIGNAL,
    CANTX_MAXIMUM_VALUE_POWER_SIGNAL};
/** @} */

/** @{
 * defines of the current signal
*/
#define CANTX_PACK_P0_CURRENT_START_BIT    (40u)
#define CANTX_PACK_P0_CURRENT_LENGTH       (17u)
#define CANTX_MINIMUM_VALUE_CURRENT_SIGNAL (-655360.0f)
#define CANTX_MAXIMUM_VALUE_CURRENT_SIGNAL (655350.0f)
/** @} */

/** @{
 * configuration of the current signal
*/
static const CAN_SIGNAL_TYPE_s cantx_signalCurrent = {
    CANTX_PACK_P0_CURRENT_START_BIT,
    CANTX_PACK_P0_CURRENT_LENGTH,
    UNIT_CONVERSION_FACTOR_10_FLOAT,
    CAN_SIGNAL_OFFSET_0,
    CANTX_MINIMUM_VALUE_CURRENT_SIGNAL,
    CANTX_MAXIMUM_VALUE_CURRENT_SIGNAL};
/** @} */

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/
/**
 * @brief   calculates the return value of the battery voltage
 * @param   kpkCanShim const pointer to CAN shim
 * @return  returns the return value of the battery voltage
 */
static uint64_t CANTX_CalculateBatteryVoltage(const CAN_SHIM_s *const kpkCanShim);

/**
 * @brief   calculates the return value of the battery voltage
 * @param   kpkCanShim const pointer to CAN shim
 * @return  returns the return value of the battery voltage
 */
static uint64_t CANTX_CalculateBusVoltage(const CAN_SHIM_s *const kpkCanShim);

/**
 * @brief   calculates the return value of the battery voltage
 * @param   kpkCanShim const pointer to CAN shim
 * @return  returns the return value of the battery voltage
 */
static uint64_t CANTX_CalculatePower(const CAN_SHIM_s *const kpkCanShim);

/**
 * @brief   calculates the return value of the battery voltage
 * @param   kpkCanShim const pointer to CAN shim
 * @return  returns the return value of the battery voltage
 */
static uint64_t CANTX_CalculateCurrent(const CAN_SHIM_s *const kpkCanShim);

/**
 * @brief   builds the PackP0 message
 * @param   kpkCanShim const pointer to CAN shim
 * @param   pMessageData message data of the CAN message
 */
static void CANTX_BuildP0Message(const CAN_SHIM_s *const kpkCanShim, uint64_t *pMessageData);

/*========== Static Function Implementations ================================*/
static uint64_t CANTX_CalculateBatteryVoltage(const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(kpkCanShim != NULL_PTR);

    /* Battery voltage */
    float_t signalData = kpkCanShim->pTablePackValues->batteryVoltage_mV;
    CAN_TxPrepareSignalData(&signalData, cantx_signalBatteryVoltage);
    uint64_t data = (uint64_t)signalData;
    return data;
}

static uint64_t CANTX_CalculateBusVoltage(const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(kpkCanShim != NULL_PTR);

    /* Bus voltage */
    float_t signalData = kpkCanShim->pTablePackValues->highVoltageBusVoltage_mV;
    CAN_TxPrepareSignalData(&signalData, cantx_signalBusVoltage);
    uint64_t data = (uint64_t)signalData;
    return data;
}

static uint64_t CANTX_CalculatePower(const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(kpkCanShim != NULL_PTR);

    /* System power */
    float_t signalData = kpkCanShim->pTablePackValues->packPower_W;
    CAN_TxPrepareSignalData(&signalData, cantx_signalPower);
    uint64_t data = (int64_t)signalData;
    return data;
}

static uint64_t CANTX_CalculateCurrent(const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(kpkCanShim != NULL_PTR);

    /* System current */
    float_t signalData = kpkCanShim->pTablePackValues->packCurrent_mA;
    CAN_TxPrepareSignalData(&signalData, cantx_signalCurrent);
    uint64_t data = (int64_t)signalData;
    return data;
}

static void CANTX_BuildP0Message(const CAN_SHIM_s *const kpkCanShim, uint64_t *pMessageData) {
    FAS_ASSERT(kpkCanShim != NULL_PTR);
    FAS_ASSERT(pMessageData != NULL_PTR);

    /* Battery voltage */
    uint64_t data = CANTX_CalculateBatteryVoltage(kpkCanShim);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData, cantx_signalBatteryVoltage.bitStart, cantx_signalBatteryVoltage.bitLength, data, CAN_BIG_ENDIAN);

    /* Bus voltage */
    data = CANTX_CalculateBusVoltage(kpkCanShim);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData, cantx_signalBusVoltage.bitStart, cantx_signalBusVoltage.bitLength, data, CAN_BIG_ENDIAN);

    /* System Power */
    data = CANTX_CalculatePower(kpkCanShim);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData, cantx_signalPower.bitStart, cantx_signalPower.bitLength, data, CAN_BIG_ENDIAN);

    /* System current */
    data = CANTX_CalculateCurrent(kpkCanShim);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData, cantx_signalCurrent.bitStart, cantx_signalCurrent.bitLength, data, CAN_BIG_ENDIAN);
}

/*========== Extern Function Implementations ================================*/
extern uint32_t CANTX_PackValuesP0(
    CAN_MESSAGE_PROPERTIES_s message,
    uint8_t *pCanData,
    uint8_t *pMuxId,
    const CAN_SHIM_s *const kpkCanShim) {
    /* pMuxId is not used here, therefore has to be NULL_PTR */
    FAS_ASSERT(pMuxId == NULL_PTR);

    FAS_ASSERT(message.id == CANTX_PACK_VALUES_P0_ID);
    FAS_ASSERT(message.idType == CANTX_PACK_VALUES_P0_ID_TYPE);
    FAS_ASSERT(message.dlc == CAN_FOXBMS_MESSAGES_DEFAULT_DLC);
    FAS_ASSERT(message.endianness == CAN_BIG_ENDIAN);
    FAS_ASSERT(pCanData != NULL_PTR);
    FAS_ASSERT(kpkCanShim != NULL_PTR);
    uint64_t messageData = 0u;

    /* Read database entry */
    DATA_READ_DATA(kpkCanShim->pTablePackValues);

    /* build message from data */
    CANTX_BuildP0Message(kpkCanShim, &messageData);

    /* now copy data in the buffer that will be used to send data */
    CAN_TxSetCanDataWithMessageData(messageData, pCanData, message.endianness);

    return 0u;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
extern uint64_t TEST_CANTX_CalculateBatteryVoltage(const CAN_SHIM_s *const kpkCanShim) {
    return CANTX_CalculateBatteryVoltage(kpkCanShim);
}
extern uint64_t TEST_CANTX_CalculateBusVoltage(const CAN_SHIM_s *const kpkCanShim) {
    return CANTX_CalculateBusVoltage(kpkCanShim);
}
extern uint64_t TEST_CANTX_CalculatePower(const CAN_SHIM_s *const kpkCanShim) {
    return CANTX_CalculatePower(kpkCanShim);
}
extern uint64_t TEST_CANTX_CalculateCurrent(const CAN_SHIM_s *const kpkCanShim) {
    return CANTX_CalculateCurrent(kpkCanShim);
}
extern void TEST_CANTX_BuildP0Message(const CAN_SHIM_s *const kpkCanShim, uint64_t *pMessageData) {
    CANTX_BuildP0Message(kpkCanShim, pMessageData);
}
#endif
