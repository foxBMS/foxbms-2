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
 * @file    can_helper.h
 * @author  foxBMS Team
 * @date    2021-04-22 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup DRIVERS
 * @prefix  CAN
 *
 * @brief   Declaration of helper functions for the CAN module
 * @details These functions support the main functionalities of the CAN module.
 *
 */

#ifndef FOXBMS__CAN_HELPER_H_
#define FOXBMS__CAN_HELPER_H_

/*========== Includes =======================================================*/
#include "can_cfg.h"

#include <math.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/** position for CAN byte 0 to 7
 * @{
 */
#define CAN_BYTE_0_POSITION (0u)
#define CAN_BYTE_1_POSITION (1u)
#define CAN_BYTE_2_POSITION (2u)
#define CAN_BYTE_3_POSITION (3u)
#define CAN_BYTE_4_POSITION (4u)
#define CAN_BYTE_5_POSITION (5u)
#define CAN_BYTE_6_POSITION (6u)
#define CAN_BYTE_7_POSITION (7u)
/**@}*/

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/
/**
 * @brief   Puts CAN signal data in a 64-bit variable.
 * @details This function is used to compose a 64-bit CAN message.
 *          It takes signal data, signal bit start, signal bit length
 *          and puts the data in the 64-bit variable.
 * @param[out]   pMessage    64-bit variable containing the CAN data
 * @param[in]    bitStart    starting bit were the signal data must be put
 * @param[in]    bitLength   length of the signal data to be put in CAN message
 * @param[in]    canSignal   signal data to be put in message
 * @param[in]    endianness  big or little endianness of data
 */
extern void CAN_TxSetMessageDataWithSignalData(
    uint64_t *pMessage,
    uint64_t bitStart,
    uint8_t bitLength,
    uint64_t canSignal,
    CAN_ENDIANNESS_e endianness);

/**
 * @brief   Copy CAN data from a 64-bit variable to 8 bytes.
 * @details This function is used to copy a 64-bit CAN message to 8 bytes.
 * @param[in]    message    64-bit variable containing the CAN data
 * @param[out]   pCanData   8 bytes where the data is copied
 * @param[in]    endianness big or little endianness of data
 */
extern void CAN_TxSetCanDataWithMessageData(uint64_t message, uint8_t *pCanData, CAN_ENDIANNESS_e endianness);

/**
 * @brief   Gets CAN signal data from a 64-bit variable.
 * @details This function is used to get signal data from a 64-bit
 *          CAN message.
 *          It takes signal bit start, signal bit length and extract
 *          signal data from the the 64-bit variable.
 * @param[in]    message     64-bit variable containing the CAN data
 * @param[in]    bitStart    starting bit were the signal data must be put
 * @param[in]    bitLength   length of the signal data to be put in CAN message
 * @param[out]   pCanSignal  signal data to be retrieved from message
 * @param[in]    endianness  big or little endianness of data
 */
extern void CAN_RxGetSignalDataFromMessageData(
    uint64_t message,
    uint64_t bitStart,
    uint8_t bitLength,
    uint64_t *pCanSignal,
    CAN_ENDIANNESS_e endianness);

/**
 * @brief   Copy CAN data from 8 bytes to a 64-bit variable.
 * @details This function is used to copy data from a 64-bit variable to 8
 *          bytes.
 * @param[in]    pMessage   64-bit where the data is copied
 * @param[out]   kpkCanData 8 bytes containing the data
 * @param[in]    endianness big or little endianness of data
 */
extern void CAN_RxGetMessageDataFromCanData(
    uint64_t *pMessage,
    const uint8_t *const kpkCanData,
    CAN_ENDIANNESS_e endianness);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
extern uint64_t TEST_CAN_ConvertBitStartBigEndian(uint64_t bitStart, uint64_t bitLength);
#endif

#endif /* FOXBMS__CAN_HELPER_H_ */
