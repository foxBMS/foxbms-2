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
 * @file    can_cbs_tx.h
 * @author  foxBMS Team
 * @date    2021-04-20 (date of creation)
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
 * @ingroup DRIVER
 * @prefix  CANTX
 *
 * @brief   CAN callbacks header
 *
 */

#ifndef FOXBMS__CAN_CBS_TX_H_
#define FOXBMS__CAN_CBS_TX_H_

/*========== Includes =======================================================*/

#include "can_cfg.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/
/** TX callback functions @{ */
/**
 * @brief can tx callback function for state
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
 * @brief can tx callback function for detail state
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
 * @brief can tx callback function for cell voltages
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
 * @brief can tx callback function for cell temperatures
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
 * @brief can tx callback function for limit values
 * @param[in] message     contains the message ID, DLC and endianness
 * @param[in] pCanData    payload of can frame
 * @param[in] pMuxId      multiplexer for multiplexed CAN messages
 * @param[in] kpkCanShim  shim to the database entries
 */
extern uint32_t CANTX_LimitValues(
    CAN_MESSAGE_PROPERTIES_s message,
    uint8_t *pCanData,
    uint8_t *pMuxId,
    const CAN_SHIM_s *const kpkCanShim);
/**
 * @brief can tx callback function for min/max values
 * @param[in] message     contains the message ID, DLC and endianness
 * @param[in] pCanData    payload of can frame
 * @param[in] pMuxId      multiplexer for multiplexed CAN messages
 * @param[in] kpkCanShim  shim to the database entries
 */
extern uint32_t CANTX_MinimumMaximumValues(
    CAN_MESSAGE_PROPERTIES_s message,
    uint8_t *pCanData,
    uint8_t *pMuxId,
    const CAN_SHIM_s *const kpkCanShim);
/**
 * @brief can tx callback function for state estimation values
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
 * @brief can tx callback function for pack values values
 * @param[in] message     contains the message ID, DLC and endianness
 * @param[in] pCanData    payload of can frame
 * @param[in] pMuxId      multiplexer for multiplexed CAN messages
 * @param[in] kpkCanShim  shim to the database entries
 */
extern uint32_t CANTX_PackValues(
    CAN_MESSAGE_PROPERTIES_s message,
    uint8_t *pCanData,
    uint8_t *pMuxId,
    const CAN_SHIM_s *const kpkCanShim);
/**
 * @brief can tx callback function for string state values
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
 * @brief can tx callback function for string values
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
 * @brief can tx callback function for string minimum and maximum values
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
 * @brief can tx callback function for string state estimation
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
/**
 * @brief can tx callback function for string values 2
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
/** @} */

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
#endif

#endif /* FOXBMS__CAN_CBS_TX_H_ */
