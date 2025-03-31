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
 * @file    can_cbs_tx_cyclic.h
 * @author  foxBMS Team
 * @date    2021-04-20 (date of creation)
 * @updated 2025-03-31 (date of last update)
 * @version v1.9.0
 * @ingroup DRIVERS
 * @prefix  CANTX
 *
 * @brief   CAN callbacks header
 * @details TODO
 */

#ifndef FOXBMS__CAN_CBS_TX_CYCLIC_H_
#define FOXBMS__CAN_CBS_TX_CYCLIC_H_

/*========== Includes =======================================================*/

#include "can_cfg.h"

#ifdef UNITY_UNIT_TEST
#include "can_helper.h"
#include "sys_mon.h"
#endif

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/
/** TX callback functions @{ */
/**
 * @brief   CAN Tx callback function for state
 * @param[in] message     contains the message ID, DLC and endianness
 * @param[in] pCanData    payload of can frame
 * @param[in] pMuxId      multiplexer for multiplexed CAN messages
 * @param[in] kpkCanShim  shim to the database entries
 */
extern uint32_t CANTX_BmsState(
    CAN_MESSAGE_PROPERTIES_s message,
    uint8_t *pCanData,
    uint8_t *pMuxId,
    const CAN_SHIM_s *const kpkCanShim);
/**
 * @brief   CAN Tx callback function for state, used to send asynchronous
 *          bms state messages, for example when changing the state
 * @return  STD_OK if message has been sent successfully
 */
extern STD_RETURN_TYPE_e CANTX_TransmitBmsState(void);

/**
 * @brief   CAN Tx callback function for detail state
 * @param[in] message     contains the message ID, DLC and endianness
 * @param[in] pCanData    payload of can frame
 * @param[in] pMuxId      multiplexer for multiplexed CAN messages
 * @param[in] kpkCanShim  shim to the database entries
 */
extern uint32_t CANTX_BmsStateDetails(
    CAN_MESSAGE_PROPERTIES_s message,
    uint8_t *pCanData,
    uint8_t *pMuxId,
    const CAN_SHIM_s *const kpkCanShim);
/**
 * @brief   CAN Tx callback function for cell voltages
 * @param[in] message     contains the message ID, DLC and endianness
 * @param[in] pCanData    payload of can frame
 * @param[in] pMuxId      multiplexer for multiplexed CAN messages
 * @param[in] kpkCanShim  shim to the database entries
 */
extern uint32_t CANTX_CellVoltages(
    CAN_MESSAGE_PROPERTIES_s message,
    uint8_t *pCanData,
    uint8_t *pMuxId,
    const CAN_SHIM_s *const kpkCanShim);
/**
 * @brief   CAN Tx callback function for cell temperatures
 * @param[in] message     contains the message ID, DLC and endianness
 * @param[in] pCanData    payload of can frame
 * @param[in] pMuxId      multiplexer for multiplexed CAN messages
 * @param[in] kpkCanShim  shim to the database entries
 */
extern uint32_t CANTX_CellTemperatures(
    CAN_MESSAGE_PROPERTIES_s message,
    uint8_t *pCanData,
    uint8_t *pMuxId,
    const CAN_SHIM_s *const kpkCanShim);
/**
 * @brief   CAN Tx callback function for limit values
 * @param[in] message     contains the message ID, DLC and endianness
 * @param[in] pCanData    payload of can frame
 * @param[in] pMuxId      multiplexer for multiplexed CAN messages
 * @param[in] kpkCanShim  shim to the database entries
 */
extern uint32_t CANTX_PackLimits(
    CAN_MESSAGE_PROPERTIES_s message,
    uint8_t *pCanData,
    uint8_t *pMuxId,
    const CAN_SHIM_s *const kpkCanShim);
/**
 * @brief   CAN Tx callback function for min/max values
 * @param[in] message     contains the message ID, DLC and endianness
 * @param[in] pCanData    payload of can frame
 * @param[in] pMuxId      multiplexer for multiplexed CAN messages
 * @param[in] kpkCanShim  shim to the database entries
 */
extern uint32_t CANTX_PackMinimumMaximumValues(
    CAN_MESSAGE_PROPERTIES_s message,
    uint8_t *pCanData,
    uint8_t *pMuxId,
    const CAN_SHIM_s *const kpkCanShim);
/**
 * @brief   CAN Tx callback function for state estimation values
 * @param[in] message     contains the message ID, DLC and endianness
 * @param[in] pCanData    payload of can frame
 * @param[in] pMuxId      multiplexer for multiplexed CAN messages
 * @param[in] kpkCanShim  shim to the database entries
 */
extern uint32_t CANTX_PackStateEstimation(
    CAN_MESSAGE_PROPERTIES_s message,
    uint8_t *pCanData,
    uint8_t *pMuxId,
    const CAN_SHIM_s *const kpkCanShim);
/**
 * @brief   CAN Tx callback function for pack values values
 * @param[in] message     contains the message ID, DLC and endianness
 * @param[in] pCanData    payload of can frame
 * @param[in] pMuxId      multiplexer for multiplexed CAN messages
 * @param[in] kpkCanShim  shim to the database entries
 */
extern uint32_t CANTX_PackValuesP0(
    CAN_MESSAGE_PROPERTIES_s message,
    uint8_t *pCanData,
    uint8_t *pMuxId,
    const CAN_SHIM_s *const kpkCanShim);
/**
 * @brief   CAN Tx callback function for pack values values
 * @param[in] message     contains the message ID, DLC and endianness
 * @param[in] pCanData    payload of can frame
 * @param[in] pMuxId      multiplexer for multiplexed CAN messages
 * @param[in] kpkCanShim  shim to the database entries
 */
extern uint32_t CANTX_PackValuesP1(
    CAN_MESSAGE_PROPERTIES_s message,
    uint8_t *pCanData,
    uint8_t *pMuxId,
    const CAN_SHIM_s *const kpkCanShim);

/**
 * @brief   CAN Tx callback function for string state values
 * @param[in] message     contains the message ID, DLC and endianness
 * @param[in] pCanData    payload of can frame
 * @param[in] pMuxId      multiplexer for multiplexed CAN messages
 * @param[in] kpkCanShim  shim to the database entries
 */
extern uint32_t CANTX_StringState(
    CAN_MESSAGE_PROPERTIES_s message,
    uint8_t *pCanData,
    uint8_t *pMuxId,
    const CAN_SHIM_s *const kpkCanShim);
/**
 * @brief   CAN Tx callback function for string values
 * @param[in] message     contains the message ID, DLC and endianness
 * @param[in] pCanData    payload of can frame
 * @param[in] pMuxId      multiplexer for multiplexed CAN messages
 * @param[in] kpkCanShim  shim to the database entries
 */
extern uint32_t CANTX_StringValuesP0(
    CAN_MESSAGE_PROPERTIES_s message,
    uint8_t *pCanData,
    uint8_t *pMuxId,
    const CAN_SHIM_s *const kpkCanShim);
/**
 * @brief   CAN Tx callback function for string values 2
 * @param[in] message     contains the message ID, DLC and endianness
 * @param[in] pCanData    payload of can frame
 * @param[in] pMuxId      multiplexer for multiplexed CAN messages
 * @param[in] kpkCanShim  shim to the database entries
 */
extern uint32_t CANTX_StringValuesP1(
    CAN_MESSAGE_PROPERTIES_s message,
    uint8_t *pCanData,
    uint8_t *pMuxId,
    const CAN_SHIM_s *const kpkCanShim);
/**
 * @brief   CAN Tx callback function for string minimum and maximum values
 * @param[in] message     contains the message ID, DLC and endianness
 * @param[in] pCanData    payload of can frame
 * @param[in] pMuxId      multiplexer for multiplexed CAN messages
 * @param[in] kpkCanShim  shim to the database entries
 */
extern uint32_t CANTX_StringMinimumMaximumValues(
    CAN_MESSAGE_PROPERTIES_s message,
    uint8_t *pCanData,
    uint8_t *pMuxId,
    const CAN_SHIM_s *const kpkCanShim);
/**
 * @brief   CAN Tx callback function for string state estimation
 * @param[in] message     contains the message ID, DLC and endianness
 * @param[in] pCanData    payload of can frame
 * @param[in] pMuxId      multiplexer for multiplexed CAN messages
 * @param[in] kpkCanShim  shim to the database entries
 */
extern uint32_t CANTX_StringStateEstimation(
    CAN_MESSAGE_PROPERTIES_s message,
    uint8_t *pCanData,
    uint8_t *pMuxId,
    const CAN_SHIM_s *const kpkCanShim);
/** @} */

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
/* externalized functions from src/app/driver/can/cbs/tx-cyclic/can_cbs_tx_bms-state-details.c */
extern void TEST_CANTX_SetTimingViolationEngine(uint64_t *pMessageData, const CAN_SHIM_s *const kpkCanShim);
extern void TEST_CANTX_SetTimingViolation1ms(uint64_t *pMessageData, const CAN_SHIM_s *const kpkCanShim);
extern void TEST_CANTX_SetTimingViolation10Ms(uint64_t *pMessageData, const CAN_SHIM_s *const kpkCanShim);
extern void TEST_CANTX_SetTimingViolation100Ms(uint64_t *pMessageData, const CAN_SHIM_s *const kpkCanShim);
extern void TEST_CANTX_SetTimingViolation100MsAlgo(uint64_t *pMessageData, const CAN_SHIM_s *const kpkCanShim);
extern void TEST_CANTX_SetTimingViolationEngineRec(
    uint64_t *pMessageData,
    SYSM_TIMING_VIOLATION_RESPONSE_s *pRecordedTimingViolations);
extern void TEST_CANTX_SetTimingViolation1MsRec(
    uint64_t *pMessageData,
    SYSM_TIMING_VIOLATION_RESPONSE_s *pRecordedTimingViolations);
extern void TEST_CANTX_SetTimingViolation10MsRec(
    uint64_t *pMessageData,
    SYSM_TIMING_VIOLATION_RESPONSE_s *pRecordedTimingViolations);
extern void TEST_CANTX_SetTimingViolation100MsRec(
    uint64_t *pMessageData,
    SYSM_TIMING_VIOLATION_RESPONSE_s *pRecordedTimingViolations);
extern void TEST_CANTX_SetTimingViolation100MsAlgoRec(
    uint64_t *pMessageData,
    SYSM_TIMING_VIOLATION_RESPONSE_s *pRecordedTimingViolations);

/* externalized functions from src/app/driver/can/cbs/tx-cyclic/can_cbs_tx_bms-state.c */
extern bool TEST_CANTX_AnySysMonTimingIssueDetected(const CAN_SHIM_s *const kpkCanShim);
extern void TEST_CANTX_BuildBmsStateMessage(uint64_t *pMessageData, const CAN_SHIM_s *const kpkCanShim);

/* externalized functions from src/app/driver/can/cbs/tx-cyclic/can_cbs_tx_cell-temperatures.c */
extern void TEST_CANTX_TemperatureSetData(
    uint16_t temperatureSensorId,
    uint64_t *pMessage,
    CAN_SIGNAL_TYPE_s cellTemperatureSignal,
    CAN_SIGNAL_TYPE_s cellTemperatureInvalidFlagSignal,
    CAN_ENDIANNESS_e endianness,
    const CAN_SHIM_s *const kpkCanShim);

/* externalized functions from src/app/driver/can/cbs/tx-cyclic/can_cbs_tx_cell-voltages.c */
extern void TEST_CANTX_VoltageSetData(
    uint16_t cellId,
    uint64_t *pMessage,
    CAN_SIGNAL_TYPE_s cellVoltageSignal,
    CAN_SIGNAL_TYPE_s cellVoltageInvalidFlagSignal,
    CAN_ENDIANNESS_e endianness,
    const CAN_SHIM_s *const kpkCanShim);

/* externalized functions from src/app/driver/can/cbs/tx-cyclic/can_cbs_tx_pack-limits.c */
extern uint64_t TEST_CANTX_CalculateMaximumDischargeCurrent(const CAN_SHIM_s *const kpkCanShim);
extern uint64_t TEST_CANTX_CalculateMaximumChargeCurrent(const CAN_SHIM_s *const kpkCanShim);
extern uint64_t TEST_CANTX_CalculateMaximumDischargePower(const CAN_SHIM_s *const kpkCanShim);
extern uint64_t TEST_CANTX_CalculateMaximumChargePower(const CAN_SHIM_s *const kpkCanShim);
extern uint64_t TEST_CANTX_CalculateMinimumBatteryVoltage(void);
extern uint64_t TEST_CANTX_CalculateMaximumBatteryVoltage(void);
extern void TEST_CANTX_BuildPackLimitsMessage(uint64_t *pMessageData, const CAN_SHIM_s *const kpkCanShim);

/* externalized functions from src/app/driver/can/cbs/tx-cyclic/can_cbs_tx_pack-minimum-maximum-values.c */
extern int16_t TEST_CANTX_GetPackMaximumVoltage(const CAN_SHIM_s *const kpkCanShim);
extern int16_t TEST_CANTX_GetPackMinimumVoltage(const CAN_SHIM_s *const kpkCanShim);
extern int16_t TEST_CANTX_GetPackMaximumTemperature(const CAN_SHIM_s *const kpkCanShim);
extern int16_t TEST_CANTX_GetPackMinimumTemperature(const CAN_SHIM_s *const kpkCanShim);
extern uint64_t TEST_CANTX_CalculatePackMaximumTemperature(const CAN_SHIM_s *const kpkCanShim);
extern uint64_t TEST_CANTX_CalculatePackMinimumTemperature(const CAN_SHIM_s *const kpkCanShim);
extern void TEST_CANTX_BuildPackMinimumMaximumValuesMessage(const CAN_SHIM_s *const kpkCanShim, uint64_t *pMessageData);

/* externalized functions from src/app/driver/can/cbs/tx-cyclic/can_cbs_tx_pack-state-estimation.c */
extern uint64_t TEST_CANTX_CalculateMaximumPackSoc(const CAN_SHIM_s *const kpkCanShim);
extern float_t TEST_CANTX_GetMaximumStringSoc(const CAN_SHIM_s *const kpkCanShim);
extern uint64_t TEST_CANTX_CalculateMinimumPackSoc(const CAN_SHIM_s *const kpkCanShim);
extern float_t TEST_CANTX_GetMinimumStringSoc(const CAN_SHIM_s *const kpkCanShim);
extern uint64_t TEST_CANTX_CalculateMaximumPackSoe(const CAN_SHIM_s *const kpkCanShim);
extern float_t TEST_CANTX_GetMaximumStringSoe(const CAN_SHIM_s *const kpkCanShim);
extern uint64_t TEST_CANTX_CalculateMinimumPackSoe(const CAN_SHIM_s *const kpkCanShim);
extern float_t TEST_CANTX_GetMinimumStringSoe(const CAN_SHIM_s *const kpkCanShim);
extern uint64_t TEST_CANTX_CalculatePackSoh(void);
extern uint64_t TEST_CANTX_CalculatePackEnergy(const CAN_SHIM_s *const kpkCanShim);
extern float_t TEST_CANTX_GetStringEnergy(const CAN_SHIM_s *const kpkCanShim);
extern void TEST_CANTX_BuildPackStateEstimationMessage(const CAN_SHIM_s *const kpkCanShim, uint64_t *pMessageData);

/* externalized functions from src/app/driver/can/cbs/tx-cyclic/can_cbs_tx_pack-values-p0.c */
extern uint64_t TEST_CANTX_CalculateBatteryVoltage(const CAN_SHIM_s *const kpkCanShim);
extern uint64_t TEST_CANTX_CalculateBusVoltage(const CAN_SHIM_s *const kpkCanShim);
extern uint64_t TEST_CANTX_CalculatePower(const CAN_SHIM_s *const kpkCanShim);
extern uint64_t TEST_CANTX_CalculateCurrent(const CAN_SHIM_s *const kpkCanShim);
extern void TEST_CANTX_BuildP0Message(const CAN_SHIM_s *const kpkCanShim, uint64_t *pMessageData);

/* externalized functions from src/app/driver/can/cbs/tx-cyclic/can_cbs_tx_pack-values-p1.c */
extern void TEST_CANTX_SetInsulationResistance(uint64_t *pMessageData, const CAN_SHIM_s *const kpkCanShim);
extern uint64_t TEST_CANTX_GetInsulationResistance(const CAN_SHIM_s *const kpkCanShim);

/* externalized functions from src/app/driver/can/cbs/tx-cyclic/can_cbs_tx_string-minimum-maximum-values.c */
extern uint64_t TEST_CANTX_CalculateStringMaximumTemperature(uint8_t stringNumber, const CAN_SHIM_s *const kpkCanShim);
extern uint64_t TEST_CANTX_CalculateStringMinimumTemperature(uint8_t stringNumber, const CAN_SHIM_s *const kpkCanShim);
extern void TEST_CANTX_BuildStringMessage(
    uint64_t *pMessageData,
    uint8_t stringNumber,
    const CAN_SHIM_s *const kpkCanShim);

/* externalized functions from src/app/driver/can/cbs/tx-cyclic/can_cbs_tx_string-state-estimation.c */
extern uint64_t TEST_CANTX_CalculateMinimumStringSoc(uint8_t stringNumber, const CAN_SHIM_s *const kpkCanShim);
extern uint64_t TEST_CANTX_CalculateMaximumStringSoc(uint8_t stringNumber, const CAN_SHIM_s *const kpkCanShim);
extern uint64_t TEST_CANTX_CalculateMinimumStringSoe(uint8_t stringNumber, const CAN_SHIM_s *const kpkCanShim);
extern uint64_t TEST_CANTX_CalculateMaximumStringSoe(uint8_t stringNumber, const CAN_SHIM_s *const kpkCanShim);
extern uint64_t TEST_CANTX_CalculateStringSoh(uint8_t stringNumber, const CAN_SHIM_s *const kpkCanShim);
extern uint64_t TEST_CANTX_CalculateStringEnergy(uint8_t stringNumber, const CAN_SHIM_s *const kpkCanShim);
extern void TEST_CANTX_BuildStringStateEstimationMessage(
    uint64_t *pMessageData,
    uint8_t stringNumber,
    const CAN_SHIM_s *const kpkCanShim);

/* externalized functions from src/app/driver/can/cbs/tx-cyclic/can_cbs_tx_string-state.c */
/* no static functions for that translation unit */

/* externalized functions from src/app/driver/can/cbs/tx-cyclic/can_cbs_tx_string-values-p0.c */
extern uint64_t TEST_CANTX_CalculateStringVoltage(uint8_t stringNumber, const CAN_SHIM_s *const kpkCanShim);
extern uint64_t TEST_CANTX_CalculateStringCurrent(uint8_t stringNumber, const CAN_SHIM_s *const kpkCanShim);
extern uint64_t TEST_CANTX_CalculateStringPower(uint8_t stringNumber, const CAN_SHIM_s *const kpkCanShim);
extern void TEST_CANTX_BuildString0Message(
    uint64_t *pMessageData,
    uint8_t stringNumber,
    const CAN_SHIM_s *const kpkCanShim);

/* externalized functions from src/app/driver/can/cbs/tx-cyclic/can_cbs_tx_string-values-p1.c */
extern void TEST_CANTX_SetStringP1Mux(uint64_t *pMessageData, uint64_t signalData);
extern void TEST_CANTX_SetStringEnergyCounter(
    uint64_t *pMessageData,
    uint8_t stringNumber,
    const CAN_SHIM_s *const kpkCanShim);

#endif

#endif /* FOXBMS__CAN_CBS_TX_CYCLIC_H_ */
