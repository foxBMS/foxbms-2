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
 * @file    can_cbs.h
 * @author  foxBMS Team
 * @date    2021-04-20 (date of creation)
 * @updated 2021-07-29 (date of last update)
 * @ingroup DRIVER
 * @prefix  CAN
 *
 * @brief   CAN callbacks header
 *
 */

#ifndef FOXBMS__CAN_CBS_H_
#define FOXBMS__CAN_CBS_H_

/*========== Includes =======================================================*/
#include "general.h"

#include "can_cfg.h"

/*========== Macros and Definitions =========================================*/

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/
/** TX callback functions @{ */
/**
 * @brief can tx callback function for state
 * @param[in] id          CAN ID
 * @param[in] dlc         CAN Data Length Code
 * @param[in] endianness  big or little endian
 * @param[in] pCanData    payload of can frame
 * @param[in] pMuxId      multiplexer for multiplexed CAN messages
 * @param[in] kpkCanShim  shim to the database entries
 */
extern uint32_t CAN_TxState(
    uint32_t id,
    uint8_t dlc,
    CAN_ENDIANNESS_e endianness,
    uint8_t *pCanData,
    uint8_t *pMuxId,
    const CAN_SHIM_s *const kpkCanShim);
/**
 * @brief can tx callback function for cell voltages
 * @param[in] id          CAN ID
 * @param[in] dlc         CAN Data Length Code
 * @param[in] endianness  big or little endian
 * @param[in] pCanData    payload of can frame
 * @param[in] pMuxId      multiplexer for multiplexed CAN messages
 * @param[in] kpkCanShim  shim to the database entries
 */
extern uint32_t CAN_TxVoltage(
    uint32_t id,
    uint8_t dlc,
    CAN_ENDIANNESS_e endianness,
    uint8_t *pCanData,
    uint8_t *pMuxId,
    const CAN_SHIM_s *const kpkCanShim);
/**
 * @brief can tx callback function for cell temperatures
 * @param[in] id          CAN ID
 * @param[in] dlc         CAN Data Length Code
 * @param[in] endianness  big or little endian
 * @param[in] pCanData    payload of can frame
 * @param[in] pMuxId      multiplexer for multiplexed CAN messages
 * @param[in] kpkCanShim  shim to the database entries
 */
extern uint32_t CAN_TxCellTemperature(
    uint32_t id,
    uint8_t dlc,
    CAN_ENDIANNESS_e endianness,
    uint8_t *pCanData,
    uint8_t *pMuxId,
    const CAN_SHIM_s *const kpkCanShim);
/**
 * @brief can tx callback function for limit values
 * @param[in] id          CAN ID
 * @param[in] dlc         CAN Data Length Code
 * @param[in] endianness  big or little endian
 * @param[in] pCanData    payload of can frame
 * @param[in] pMuxId      multiplexer for multiplexed CAN messages
 * @param[in] kpkCanShim  shim to the database entries
 */
extern uint32_t CAN_TxLimitValues(
    uint32_t id,
    uint8_t dlc,
    CAN_ENDIANNESS_e endianness,
    uint8_t *pCanData,
    uint8_t *pMuxId,
    const CAN_SHIM_s *const kpkCanShim);
/**
 * @brief can tx callback function for min/max values
 * @param[in] id          CAN ID
 * @param[in] dlc         CAN Data Length Code
 * @param[in] endianness  big or little endian
 * @param[in] pCanData    payload of can frame
 * @param[in] pMuxId      multiplexer for multiplexed CAN messages
 * @param[in] kpkCanShim  shim to the database entries
 */
extern uint32_t CAN_TxMinimumMaximumValues(
    uint32_t id,
    uint8_t dlc,
    CAN_ENDIANNESS_e endianness,
    uint8_t *pCanData,
    uint8_t *pMuxId,
    const CAN_SHIM_s *const kpkCanShim);
/**
 * @brief can tx callback function for state estimation values
 * @param[in] id          CAN ID
 * @param[in] dlc         CAN Data Length Code
 * @param[in] endianness  big or little endian
 * @param[in] pCanData    payload of can frame
 * @param[in] pMuxId      multiplexer for multiplexed CAN messages
 * @param[in] kpkCanShim  shim to the database entries
 */
extern uint32_t CAN_TxStateEstimation(
    uint32_t id,
    uint8_t dlc,
    CAN_ENDIANNESS_e endianness,
    uint8_t *pCanData,
    uint8_t *pMuxId,
    const CAN_SHIM_s *const kpkCanShim);
/**
 * @brief can tx callback function for pack values values
 * @param[in] id          CAN ID
 * @param[in] dlc         CAN Data Length Code
 * @param[in] endianness  big or little endian
 * @param[in] pCanData    payload of can frame
 * @param[in] pMuxId      multiplexer for multiplexed CAN messages
 * @param[in] kpkCanShim  shim to the database entries
 */
extern uint32_t CAN_TxPackValues(
    uint32_t id,
    uint8_t dlc,
    CAN_ENDIANNESS_e endianness,
    uint8_t *pCanData,
    uint8_t *pMuxId,
    const CAN_SHIM_s *const kpkCanShim);
/**
 * @brief can tx callback function for string state values
 * @param[in] id          CAN ID
 * @param[in] dlc         CAN Data Length Code
 * @param[in] endianness  big or little endian
 * @param[in] pCanData    payload of can frame
 * @param[in] pMuxId      multiplexer for multiplexed CAN messages
 * @param[in] kpkCanShim  shim to the database entries
 */
extern uint32_t CAN_TxStringState(
    uint32_t id,
    uint8_t dlc,
    CAN_ENDIANNESS_e endianness,
    uint8_t *pCanData,
    uint8_t *pMuxId,
    const CAN_SHIM_s *const kpkCanShim);
/**
 * @brief can tx callback function for string values
 * @param[in] id          CAN ID
 * @param[in] dlc         CAN Data Length Code
 * @param[in] endianness  big or little endian
 * @param[in] pCanData    payload of can frame
 * @param[in] pMuxId      multiplexer for multiplexed CAN messages
 * @param[in] kpkCanShim  shim to the database entries
 */
extern uint32_t CAN_TxStringValues(
    uint32_t id,
    uint8_t dlc,
    CAN_ENDIANNESS_e endianness,
    uint8_t *pCanData,
    uint8_t *pMuxId,
    const CAN_SHIM_s *const kpkCanShim);
/**
 * @brief can tx callback function for string minimum and maximum values
 * @param[in] id          CAN ID
 * @param[in] dlc         CAN Data Length Code
 * @param[in] endianness  big or little endian
 * @param[in] pCanData    payload of can frame
 * @param[in] pMuxId      multiplexer for multiplexed CAN messages
 * @param[in] kpkCanShim  shim to the database entries
 */
extern uint32_t CAN_TxStringMinimumMaximumValues(
    uint32_t id,
    uint8_t dlc,
    CAN_ENDIANNESS_e endianness,
    uint8_t *pCanData,
    uint8_t *pMuxId,
    const CAN_SHIM_s *const kpkCanShim);
/**
 * @brief can tx callback function for string state estimation
 * @param[in] id          CAN ID
 * @param[in] dlc         CAN Data Length Code
 * @param[in] endianness  big or little endian
 * @param[in] pCanData    payload of can frame
 * @param[in] pMuxId      multiplexer for multiplexed CAN messages
 * @param[in] kpkCanShim  shim to the database entries
 */
extern uint32_t CAN_TxStringStateEstimation(
    uint32_t id,
    uint8_t dlc,
    CAN_ENDIANNESS_e endianness,
    uint8_t *pCanData,
    uint8_t *pMuxId,
    const CAN_SHIM_s *const kpkCanShim);
/**
 * @brief can tx callback function for string values 2
 * @param[in] id          CAN ID
 * @param[in] dlc         CAN Data Length Code
 * @param[in] endianness  big or little endian
 * @param[in] pCanData    payload of can frame
 * @param[in] pMuxId      multiplexer for multiplexed CAN messages
 * @param[in] kpkCanShim  shim to the database entries
 */
extern uint32_t CAN_TxStringValues2(
    uint32_t id,
    uint8_t dlc,
    CAN_ENDIANNESS_e endianness,
    uint8_t *pCanData,
    uint8_t *pMuxId,
    const CAN_SHIM_s *const kpkCanShim);
/** @} */

/** RX callback functions @{ */
/**
 * @brief can rx callback function for IMD info messages
 * @param[in] id          CAN ID
 * @param[in] dlc         CAN Data Length Code
 * @param[in] endianness  big or little endian
 * @param[in] pCanData    payload of can frame
 * @param[in] pMuxId      multiplexer for multiplexed CAN messages
 * @param[in] kpkCanShim  shim to the database entries
 */
extern uint32_t CAN_RxImdInfo(
    uint32_t id,
    uint8_t dlc,
    CAN_ENDIANNESS_e endianness,
    uint8_t *pCanData,
    uint8_t *pMuxId,
    const CAN_SHIM_s *const kpkCanShim);
/**
 * @brief can rx callback function for IMD response messages
 * @param[in] id          CAN ID
 * @param[in] dlc         CAN Data Length Code
 * @param[in] endianness  big or little endian
 * @param[in] pCanData    payload of can frame
 * @param[in] pMuxId      multiplexer for multiplexed CAN messages
 * @param[in] kpkCanShim  shim to the database entries
 */
extern uint32_t CAN_RxImdResponse(
    uint32_t id,
    uint8_t dlc,
    CAN_ENDIANNESS_e endianness,
    uint8_t *pCanData,
    uint8_t *pMuxId,
    const CAN_SHIM_s *const kpkCanShim);
/**
 * @brief can rx callback function for state requests
 * @param[in] id          CAN ID
 * @param[in] dlc         CAN Data Length Code
 * @param[in] endianness  big or little endian
 * @param[in] pCanData    payload of can frame
 * @param[in] pMuxId      multiplexer for multiplexed CAN messages
 * @param[in] kpkCanShim  shim to the database entries
 */
extern uint32_t CAN_RxRequest(
    uint32_t id,
    uint8_t dlc,
    CAN_ENDIANNESS_e endianness,
    uint8_t *pCanData,
    uint8_t *pMuxId,
    const CAN_SHIM_s *const kpkCanShim);
/**
 * @brief can rx callback function for SW reset
 * @param[in] id          CAN ID
 * @param[in] dlc         CAN Data Length Code
 * @param[in] endianness  big or little endian
 * @param[in] pCanData    payload of can frame
 * @param[in] pMuxId      multiplexer for multiplexed CAN messages
 * @param[in] kpkCanShim  shim to the database entries
 */
extern uint32_t CAN_RxSwReset(
    uint32_t id,
    uint8_t dlc,
    CAN_ENDIANNESS_e endianness,
    uint8_t *pCanData,
    uint8_t *pMuxId,
    const CAN_SHIM_s *const kpkCanShim);
/**
 * @brief can rx callback function for current sensor measurements
 * @param[in] id          CAN ID
 * @param[in] dlc         CAN Data Length Code
 * @param[in] endianness  big or little endian
 * @param[in] pCanData    payload of can frame
 * @param[in] pMuxId      multiplexer for multiplexed CAN messages
 * @param[in] kpkCanShim  shim to the database entries
 */
extern uint32_t CAN_RxCurrentSensor(
    uint32_t id,
    uint8_t dlc,
    CAN_ENDIANNESS_e endianness,
    uint8_t *pCanData,
    uint8_t *pMuxId,
    const CAN_SHIM_s *const kpkCanShim);
/**
 * @brief can rx callback function for debug messages
 * @param[in] id          CAN ID
 * @param[in] dlc         CAN Data Length Code
 * @param[in] endianness  big or little endian
 * @param[in] pCanData    payload of can frame
 * @param[in] pMuxId      multiplexer for multiplexed CAN messages
 * @param[in] kpkCanShim  shim to the database entries
 */
extern uint32_t CAN_RxDebug(
    uint32_t id,
    uint8_t dlc,
    CAN_ENDIANNESS_e endianness,
    uint8_t *pCanData,
    uint8_t *pMuxId,
    const CAN_SHIM_s *const kpkCanShim);
/**
 * @brief can rx callback function for SW version
 * @param[in] id          CAN ID
 * @param[in] dlc         CAN Data Length Code
 * @param[in] endianness  big or little endian
 * @param[in] pCanData    payload of can frame
 * @param[in] pMuxId      multiplexer for multiplexed CAN messages
 * @param[in] kpkCanShim  shim to the database entries
 */
extern uint32_t CAN_RxSwVersion(
    uint32_t id,
    uint8_t dlc,
    CAN_ENDIANNESS_e endianness,
    uint8_t *pCanData,
    uint8_t *pMuxId,
    const CAN_SHIM_s *const kpkCanShim);
/** @} */

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST

#endif

#endif /* FOXBMS__CAN_CBS_H_ */
