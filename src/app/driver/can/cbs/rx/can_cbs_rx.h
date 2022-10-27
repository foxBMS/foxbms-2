/**
 *
 * @copyright &copy; 2010 - 2022, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * @file    can_cbs_rx.h
 * @author  foxBMS Team
 * @date    2021-04-20 (date of creation)
 * @updated 2022-10-27 (date of last update)
 * @version v1.4.1
 * @ingroup DRIVER
 * @prefix  CANRX
 *
 * @brief   CAN callbacks header
 *
 */

#ifndef FOXBMS__CAN_CBS_RX_H_
#define FOXBMS__CAN_CBS_RX_H_

/*========== Includes =======================================================*/
#include "general.h"

#include "can_cfg.h"

/*========== Macros and Definitions =========================================*/

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/

/** RX callback functions @{ */
/**
 * @brief can rx callback function for IMD info messages
 * @param[in] message     contains the message ID, DLC and endianness
 * @param[in] kpkCanData  payload of can frame
 * @param[in] kpkCanShim  shim to the database entries
 */
extern uint32_t CANRX_ImdInfo(
    CAN_MESSAGE_PROPERTIES_s message,
    const uint8_t *const kpkCanData,
    const CAN_SHIM_s *const kpkCanShim);
/**
 * @brief can rx callback function for IMD response messages
 * @param[in] message     contains the message ID, DLC and endianness
 * @param[in] kpkCanData  payload of can frame
 * @param[in] kpkCanShim  shim to the database entries
 */
extern uint32_t CANRX_ImdResponse(
    CAN_MESSAGE_PROPERTIES_s message,
    const uint8_t *const kpkCanData,
    const CAN_SHIM_s *const kpkCanShim);
/**
 * @brief can rx callback function for state requests
 * @param[in] message     contains the message ID, DLC and endianness
 * @param[in] kpkCanData  payload of can frame
 * @param[in] kpkCanShim  shim to the database entries
 */
extern uint32_t CANRX_BmsStateRequest(
    CAN_MESSAGE_PROPERTIES_s message,
    const uint8_t *const kpkCanData,
    const CAN_SHIM_s *const kpkCanShim);
/**
 * @brief can rx callback function for SW reset
 * @param[in] message     contains the message ID, DLC and endianness
 * @param[in] kpkCanData  payload of can frame
 * @param[in] kpkCanShim  shim to the database entries
 */
extern uint32_t CANRX_SoftwareReset(
    CAN_MESSAGE_PROPERTIES_s message,
    const uint8_t *const kpkCanData,
    const CAN_SHIM_s *const kpkCanShim);
/**
 * @brief can rx callback function for current sensor measurements
 * @param[in] message     contains the message ID, DLC and endianness
 * @param[in] kpkCanData  payload of can frame
 * @param[in] kpkCanShim  shim to the database entries
 */
extern uint32_t CANRX_CurrentSensor(
    CAN_MESSAGE_PROPERTIES_s message,
    const uint8_t *const kpkCanData,
    const CAN_SHIM_s *const kpkCanShim);
/**
 * @brief can rx callback function for debug messages
 * @param[in] message     contains the message ID, DLC and endianness
 * @param[in] kpkCanData  payload of can frame
 * @param[in] kpkCanShim  shim to the database entries
 */
extern uint32_t CANRX_Debug(
    CAN_MESSAGE_PROPERTIES_s message,
    const uint8_t *const kpkCanData,
    const CAN_SHIM_s *const kpkCanShim);
/**
 * @brief can rx callback function for SW version
 * @param[in] message     contains the message ID, DLC and endianness
 * @param[in] kpkCanData  payload of can frame
 * @param[in] kpkCanShim  shim to the database entries
 */
extern uint32_t CANRX_SoftwareVersion(
    CAN_MESSAGE_PROPERTIES_s message,
    const uint8_t *const kpkCanData,
    const CAN_SHIM_s *const kpkCanShim);
/** @} */

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST

#endif

#endif /* FOXBMS__CAN_CBS_RX_H_ */
