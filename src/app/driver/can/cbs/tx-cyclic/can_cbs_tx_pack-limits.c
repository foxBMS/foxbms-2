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
 * @file    can_cbs_tx_pack-limits.c
 * @author  foxBMS Team
 * @date    2021-07-21 (date of creation)
 * @updated 2024-12-20 (date of last update)
 * @version v1.8.0
 * @ingroup DRIVERS
 * @prefix  CANTX
 *
 * @brief   CAN driver Tx callback implementation
 * @details CAN Tx callback for state messages
 */

/*========== Includes =======================================================*/
#include "battery_cell_cfg.h"

/* AXIVION Next Codeline Generic-LocalInclude: 'can_cbs_tx_cyclic.h' declares
 * the prototype for the callback 'CANTX_PackLimits' */
#include "can_cbs_tx_cyclic.h"
#include "can_cfg_tx-cyclic-message-definitions.h"
#include "can_helper.h"
#include "foxmath.h"

#include <math.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/
/** @{
 * defines of the maximum discharge current signal
*/
#define CANTX_SIGNAL_MAXIMUM_DISCHARGE_CURRENT_START_BIT     (7u)
#define CANTX_SIGNAL_MAXIMUM_DISCHARGE_CURRENT_LENGTH        (12u)
#define CANTX_MINIMUM_VALUE_MAXIMUM_DISCHARGE_CURRENT_SIGNAL (0.0f)
#define CANTX_MAXIMUM_VALUE_MAXIMUM_DISCHARGE_CURRENT_SIGNAL (1023750.0f)
#define CANTX_FACTOR_MAXIMUM_DISCHARGE_CURRENT               (250.0f)
/** @} */

/** @{
 * configuration of the maximum discharge current signal
*/
static const CAN_SIGNAL_TYPE_s cantx_signalMaximumDischargeCurrent = {
    CANTX_SIGNAL_MAXIMUM_DISCHARGE_CURRENT_START_BIT,
    CANTX_SIGNAL_MAXIMUM_DISCHARGE_CURRENT_LENGTH,
    CANTX_FACTOR_MAXIMUM_DISCHARGE_CURRENT,
    CAN_SIGNAL_OFFSET_0,
    CANTX_MINIMUM_VALUE_MAXIMUM_DISCHARGE_CURRENT_SIGNAL,
    CANTX_MAXIMUM_VALUE_MAXIMUM_DISCHARGE_CURRENT_SIGNAL};
/** @} */

/** @{
 * defines of the maximum charge current signal
*/
#define CANTX_SIGNAL_MAXIMUM_CHARGE_CURRENT_START_BIT     (11u)
#define CANTX_SIGNAL_MAXIMUM_CHARGE_CURRENT_LENGTH        (12u)
#define CANTX_MINIMUM_VALUE_MAXIMUM_CHARGE_CURRENT_SIGNAL (0.0f)
#define CANTX_MAXIMUM_VALUE_MAXIMUM_CHARGE_CURRENT_SIGNAL (1023750.0f)
#define CANTX_FACTOR_MAXIMUM_CHARGE_CURRENT               (250.0f)
/** @} */

/** @{
 * configuration of the maximum charge current signal
*/
static const CAN_SIGNAL_TYPE_s cantx_signalMaximumChargeCurrent = {
    CANTX_SIGNAL_MAXIMUM_CHARGE_CURRENT_START_BIT,
    CANTX_SIGNAL_MAXIMUM_CHARGE_CURRENT_LENGTH,
    CANTX_FACTOR_MAXIMUM_CHARGE_CURRENT,
    CAN_SIGNAL_OFFSET_0,
    CANTX_MINIMUM_VALUE_MAXIMUM_CHARGE_CURRENT_SIGNAL,
    CANTX_MAXIMUM_VALUE_MAXIMUM_CHARGE_CURRENT_SIGNAL};
/** @} */

/** @{
 * defines of the maximum discharge power signal
*/
#define CANTX_SIGNAL_MAXIMUM_DISCHARGE_POWER_START_BIT     (31u)
#define CANTX_SIGNAL_MAXIMUM_DISCHARGE_POWER_LENGTH        (12u)
#define CANTX_MINIMUM_VALUE_MAXIMUM_DISCHARGE_POWER_SIGNAL (0.0f)
#define CANTX_MAXIMUM_VALUE_MAXIMUM_DISCHARGE_POWER_SIGNAL (409500.0f)
/** @} */

/** @{
 * configuration of the maximum discharge power signal
*/
static const CAN_SIGNAL_TYPE_s cantx_signalMaximumDischargePower = {
    CANTX_SIGNAL_MAXIMUM_DISCHARGE_POWER_START_BIT,
    CANTX_SIGNAL_MAXIMUM_DISCHARGE_POWER_LENGTH,
    UNIT_CONVERSION_FACTOR_100_FLOAT,
    CAN_SIGNAL_OFFSET_0,
    CANTX_MINIMUM_VALUE_MAXIMUM_DISCHARGE_POWER_SIGNAL,
    CANTX_MAXIMUM_VALUE_MAXIMUM_DISCHARGE_POWER_SIGNAL};
/** @} */

/** @{
 * defines of the maximum charge power signal
*/
#define CANTX_SIGNAL_MAXIMUM_CHARGE_POWER_START_BIT     (35u)
#define CANTX_SIGNAL_MAXIMUM_CHARGE_POWER_LENGTH        (12u)
#define CANTX_MINIMUM_VALUE_MAXIMUM_CHARGE_POWER_SIGNAL (0.0f)
#define CANTX_MAXIMUM_VALUE_MAXIMUM_CHARGE_POWER_SIGNAL (409500.0f)
/** @} */

/** @{
 * configuration of the maximum charge power signal
*/
static const CAN_SIGNAL_TYPE_s cantx_signalMaximumChargePower = {
    CANTX_SIGNAL_MAXIMUM_CHARGE_POWER_START_BIT,
    CANTX_SIGNAL_MAXIMUM_CHARGE_POWER_LENGTH,
    UNIT_CONVERSION_FACTOR_100_FLOAT,
    CAN_SIGNAL_OFFSET_0,
    CANTX_MINIMUM_VALUE_MAXIMUM_CHARGE_POWER_SIGNAL,
    CANTX_MAXIMUM_VALUE_MAXIMUM_CHARGE_POWER_SIGNAL};
/** @} */

/** @{
 * defines of the maximum battery voltage signal
*/
#define CANTX_SIGNAL_MAXIMUM_BATTERY_VOLTAGE_START_BIT     (55u)
#define CANTX_SIGNAL_MAXIMUM_BATTERY_VOLTAGE_LENGTH        (8u)
#define CANTX_MINIMUM_VALUE_MAXIMUM_BATTERY_VOLTAGE_SIGNAL (0.0f)
#define CANTX_MAXIMUM_VALUE_MAXIMUM_BATTERY_VOLTAGE_SIGNAL (1020000.0f)
#define CANTX_FACTOR_MAXIMUM_BATTERY_VOLTAGE               (4000.0f)
/** @} */

/** @{
 * configuration of the maximum battery voltage signal
*/
static const CAN_SIGNAL_TYPE_s cantx_signalMaximumBatteryVoltage = {
    CANTX_SIGNAL_MAXIMUM_BATTERY_VOLTAGE_START_BIT,
    CANTX_SIGNAL_MAXIMUM_BATTERY_VOLTAGE_LENGTH,
    CANTX_FACTOR_MAXIMUM_BATTERY_VOLTAGE,
    CAN_SIGNAL_OFFSET_0,
    CANTX_MINIMUM_VALUE_MAXIMUM_BATTERY_VOLTAGE_SIGNAL,
    CANTX_MAXIMUM_VALUE_MAXIMUM_BATTERY_VOLTAGE_SIGNAL};
/** @} */

/** @{
 * defines of the minimum battery voltage signal
*/
#define CANTX_SIGNAL_MINIMUM_BATTERY_VOLTAGE_START_BIT     (63u)
#define CANTX_SIGNAL_MINIMUM_BATTERY_VOLTAGE_LENGTH        (8u)
#define CANTX_MINIMUM_VALUE_MINIMUM_BATTERY_VOLTAGE_SIGNAL (0.0f)
#define CANTX_MAXIMUM_VALUE_MINIMUM_BATTERY_VOLTAGE_SIGNAL (1020000.0f)
#define CANTX_FACTOR_MINIMUM_BATTERY_VOLTAGE               (4000.0f)
/** @} */

/** @{
 * configuration of the minimum battery voltage signal
 */
static const CAN_SIGNAL_TYPE_s cantx_signalMinimumBatteryVoltage = {
    CANTX_SIGNAL_MINIMUM_BATTERY_VOLTAGE_START_BIT,
    CANTX_SIGNAL_MINIMUM_BATTERY_VOLTAGE_LENGTH,
    CANTX_FACTOR_MINIMUM_BATTERY_VOLTAGE,
    CAN_SIGNAL_OFFSET_0,
    CANTX_MINIMUM_VALUE_MINIMUM_BATTERY_VOLTAGE_SIGNAL,
    CANTX_MAXIMUM_VALUE_MINIMUM_BATTERY_VOLTAGE_SIGNAL};
/** @} */

/*========== Static Constant and Variable Definitions =======================*/
/**
 * @brief   Calculates the return value of the maximum discharge current
 * @param   kpkCanShim const pointer to CAN shim
 * @return  Returns the return value of the maximum discharge current
 */
static uint64_t CANTX_CalculateMaximumDischargeCurrent(const CAN_SHIM_s *const kpkCanShim);

/**
 * @brief   Calculates the return value of the maximum charge current
 * @param   kpkCanShim const pointer to CAN shim
 * @return  Returns the return value of the maximum charge current
 */
static uint64_t CANTX_CalculateMaximumChargeCurrent(const CAN_SHIM_s *const kpkCanShim);

/**
 * @brief   Calculates the return value of the maximum discharge power
 * @param   kpkCanShim const pointer to CAN shim
 * @return  Returns the return value of the maximum discharge power
 */
static uint64_t CANTX_CalculateMaximumDischargePower(const CAN_SHIM_s *const kpkCanShim);

/**
 * @brief   Calculates the return value of the maximum charge power
 * @param   kpkCanShim const pointer to CAN shim
 * @return  Returns the return value of the maximum charge power
 */
static uint64_t CANTX_CalculateMaximumChargePower(const CAN_SHIM_s *const kpkCanShim);

/**
 * @brief   Calculates the return value of the minimum battery voltage
 * @return  Returns the return value of the minimum battery voltage
 */
static uint64_t CANTX_CalculateMinimumBatteryVoltage(void);

/**
 * @brief   Calculates the return value of the maximum battery voltage
 * @return  Returns the return value of the maximum battery voltage
 */
static uint64_t CANTX_CalculateMaximumBatteryVoltage(void);

/**
 * @brief   Adds the data to the message
 * @param   pMessageData message data of the CAN message
 * @param   kpkCanShim const pointer to CAN shim
 */
static void CANTX_BuildPackLimitsMessage(uint64_t *pMessageData, const CAN_SHIM_s *const kpkCanShim);

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/
static uint64_t CANTX_CalculateMaximumDischargeCurrent(const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(kpkCanShim != NULL_PTR);

    /* maximum discharge current */
    float_t signalData = (float_t)kpkCanShim->pTableSof->recommendedContinuousPackDischargeCurrent_mA;
    CAN_TxPrepareSignalData(&signalData, cantx_signalMaximumDischargeCurrent);
    uint64_t data = (uint64_t)signalData;
    return data;
}

static uint64_t CANTX_CalculateMaximumChargeCurrent(const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(kpkCanShim != NULL_PTR);

    /* maximum charge current */
    float_t signalData = (float_t)kpkCanShim->pTableSof->recommendedContinuousPackChargeCurrent_mA;
    CAN_TxPrepareSignalData(&signalData, cantx_signalMaximumChargeCurrent);
    uint64_t data = (uint64_t)signalData;
    return data;
}

static uint64_t CANTX_CalculateMaximumDischargePower(const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(kpkCanShim != NULL_PTR);

    /* maximum charge power = discharge_current_A * battery_voltage_V */
    float_t signalData =
        ((float_t)kpkCanShim->pTableSof->recommendedContinuousPackDischargeCurrent_mA *
         UNIT_CONVERSION_FACTOR_1_1000_TH_FLOAT) *
        ((float_t)kpkCanShim->pTablePackValues->batteryVoltage_mV * UNIT_CONVERSION_FACTOR_1_1000_TH_FLOAT);
    CAN_TxPrepareSignalData(&signalData, cantx_signalMaximumDischargePower);
    uint64_t data = (uint64_t)signalData;
    return data;
}

static uint64_t CANTX_CalculateMaximumChargePower(const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(kpkCanShim != NULL_PTR);

    /* maximum charge power = charge_current_A * battery_voltage_V */
    float_t signalData =
        ((float_t)kpkCanShim->pTableSof->recommendedContinuousPackChargeCurrent_mA *
         UNIT_CONVERSION_FACTOR_1_1000_TH_FLOAT) *
        ((float_t)kpkCanShim->pTablePackValues->batteryVoltage_mV * UNIT_CONVERSION_FACTOR_1_1000_TH_FLOAT);
    CAN_TxPrepareSignalData(&signalData, cantx_signalMaximumChargePower);
    uint64_t data = (uint64_t)signalData;
    return data;
}

static uint64_t CANTX_CalculateMinimumBatteryVoltage(void) {
    /* minimum battery voltage */
    float_t signalData = (float_t)(BS_NR_OF_CELL_BLOCKS_PER_STRING * BC_VOLTAGE_MIN_MSL_mV);
    CAN_TxPrepareSignalData(&signalData, cantx_signalMinimumBatteryVoltage);
    uint64_t data = (uint64_t)signalData;
    return data;
}

static uint64_t CANTX_CalculateMaximumBatteryVoltage(void) {
    /* maximum battery voltage */
    float_t signalData = (float_t)(BS_NR_OF_CELL_BLOCKS_PER_STRING * BC_VOLTAGE_MAX_MSL_mV);
    CAN_TxPrepareSignalData(&signalData, cantx_signalMaximumBatteryVoltage);
    uint64_t data = (uint64_t)signalData;
    return data;
}

static void CANTX_BuildPackLimitsMessage(uint64_t *pMessageData, const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(pMessageData != NULL_PTR);
    FAS_ASSERT(kpkCanShim != NULL_PTR);

    /* Maximum discharge current */
    uint64_t data = CANTX_CalculateMaximumDischargeCurrent(kpkCanShim);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        cantx_signalMaximumDischargeCurrent.bitStart,
        cantx_signalMaximumDischargeCurrent.bitLength,
        data,
        CAN_BIG_ENDIAN);
    /* Maximum charge current */
    data = CANTX_CalculateMaximumChargeCurrent(kpkCanShim);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        cantx_signalMaximumChargeCurrent.bitStart,
        cantx_signalMaximumChargeCurrent.bitLength,
        data,
        CAN_BIG_ENDIAN);
    /* Maximum discharge power */
    data = CANTX_CalculateMaximumDischargePower(kpkCanShim);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        cantx_signalMaximumDischargePower.bitStart,
        cantx_signalMaximumDischargePower.bitLength,
        data,
        CAN_BIG_ENDIAN);
    /* Maximum charge power */
    data = CANTX_CalculateMaximumChargePower(kpkCanShim);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        cantx_signalMaximumChargePower.bitStart,
        cantx_signalMaximumChargePower.bitLength,
        data,
        CAN_BIG_ENDIAN);
    /* Minimum battery voltage */
    data = CANTX_CalculateMinimumBatteryVoltage();
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        cantx_signalMinimumBatteryVoltage.bitStart,
        cantx_signalMinimumBatteryVoltage.bitLength,
        data,
        CAN_BIG_ENDIAN);
    /* Maximum battery voltage */
    data = CANTX_CalculateMaximumBatteryVoltage();
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        cantx_signalMaximumBatteryVoltage.bitStart,
        cantx_signalMaximumBatteryVoltage.bitLength,
        data,
        CAN_BIG_ENDIAN);
}

/*========== Extern Function Implementations ================================*/
extern uint32_t CANTX_PackLimits(
    CAN_MESSAGE_PROPERTIES_s message,
    uint8_t *pCanData,
    uint8_t *pMuxId,
    const CAN_SHIM_s *const kpkCanShim) {
    /* pMuxId is not used here, therefore has to be NULL_PTR */
    FAS_ASSERT(pMuxId == NULL_PTR);
    FAS_ASSERT(message.id == CANTX_PACK_LIMITS_ID);
    FAS_ASSERT(message.idType == CANTX_PACK_LIMITS_ID_TYPE);
    FAS_ASSERT(message.dlc == CAN_FOXBMS_MESSAGES_DEFAULT_DLC);
    FAS_ASSERT(message.endianness == CANTX_PACK_LIMITS_ENDIANNESS);
    FAS_ASSERT(pCanData != NULL_PTR);
    FAS_ASSERT(kpkCanShim != NULL_PTR);
    uint64_t messageData = 0u;

    DATA_READ_DATA(kpkCanShim->pTableSof, can_kShim.pTablePackValues);

    CANTX_BuildPackLimitsMessage(&messageData, kpkCanShim);

    /* now copy data in the buffer that will be used to send data */
    CAN_TxSetCanDataWithMessageData(messageData, pCanData, message.endianness);

    return 0u;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
extern uint64_t TEST_CANTX_CalculateMaximumDischargeCurrent(const CAN_SHIM_s *const kpkCanShim) {
    return CANTX_CalculateMaximumDischargeCurrent(kpkCanShim);
}
extern uint64_t TEST_CANTX_CalculateMaximumChargeCurrent(const CAN_SHIM_s *const kpkCanShim) {
    return CANTX_CalculateMaximumChargeCurrent(kpkCanShim);
}
extern uint64_t TEST_CANTX_CalculateMaximumDischargePower(const CAN_SHIM_s *const kpkCanShim) {
    return CANTX_CalculateMaximumDischargePower(kpkCanShim);
}
extern uint64_t TEST_CANTX_CalculateMaximumChargePower(const CAN_SHIM_s *const kpkCanShim) {
    return CANTX_CalculateMaximumChargePower(kpkCanShim);
}
extern uint64_t TEST_CANTX_CalculateMinimumBatteryVoltage(void) {
    return CANTX_CalculateMinimumBatteryVoltage();
}
extern uint64_t TEST_CANTX_CalculateMaximumBatteryVoltage(void) {
    return CANTX_CalculateMaximumBatteryVoltage();
}
extern void TEST_CANTX_BuildPackLimitsMessage(uint64_t *pMessageData, const CAN_SHIM_s *const kpkCanShim) {
    CANTX_BuildPackLimitsMessage(pMessageData, kpkCanShim);
}

#endif
