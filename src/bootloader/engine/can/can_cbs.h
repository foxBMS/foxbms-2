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
 * @file    can_cbs.h
 * @author  foxBMS Team
 * @date    2021-04-20 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup DRIVERS
 * @prefix  CAN
 *
 * @brief   CAN callbacks header
 * @details TODO
 */

#ifndef FOXBMS__CAN_CBS_H_
#define FOXBMS__CAN_CBS_H_

/*========== Includes =======================================================*/

#include "can_cfg.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/

/** RX callback functions @{ */
/**
 * @brief   CAN Rx callback function for the 'f_BootloaderActionRequest'
 *          message
 * @param[in] message   contains the message ID, DLC and endianness
 * @param   kpkCanData  Pointer of the received data
 * @return  0 if the function has been executed successfully.
 */
extern uint32_t CAN_RxActionRequest(CAN_MESSAGE_PROPERTIES_s message, const uint8_t *const kpkCanData);

/**
 * @brief   CAN Rx callback function for the 'f_BootloaderTransferProcessInfo'
 *          message
 * @param[in] message   contains the message ID, DLC and endianness
 * @param   kpkCanData  Pointer of the received data
 * @return  0 if the function has been executed successfully.
 */
extern uint32_t CAN_RxTransferProcessInfo(CAN_MESSAGE_PROPERTIES_s message, const uint8_t *const kpkCanData);

/**
 * @brief   CAN Rx callback function for the 'f_BootloaderLoopInfo' message
 * @details CAN Rx callback function that is triggered by an incoming
 *          'f_BootloaderLoopInfo' CAN message to ensure everything is correct
 *          during the transaction of one loop of program data.
 *          This includes detecting the mismatched CAN communication state,
 *          initiating the current loop number if it has not been initialized
 *          yet, updating the state of the CAN communication based on the
 *          current loop number and the current CAN communication state, and
 *          backing up the variable can_infoOfDataTransfer, which stores the
 *          CAN information during the data transfer for the current loop
 *          number if the current loop is the first one in a section.
 * @param[in] message   contains the message ID, DLC and endianness
 * @param   kpkCanData  Pointer of the received data
 * @return  0 if the function has been executed successfully.
 */
extern uint32_t CAN_RxLoopInfo(CAN_MESSAGE_PROPERTIES_s message, const uint8_t *const kpkCanData);

/**
 * @brief   CAN Rx callback function for the 'f_BootloaderData8Bytes' message
 * @param[in] message   contains the message ID, DLC and endianness
 * @param   kpkCanData  Pointer of the received data
 * @return  0 if the function has been executed successfully.
 */
extern uint32_t CAN_RxData8Bytes(CAN_MESSAGE_PROPERTIES_s message, const uint8_t *const kpkCanData);

/**
 * @brief   CAN Rx callback function for the 'f_BootloaderCrc8Bytes' message
 * @details The messages carries an 8-byte CRC signature. This function
 *          validates the last received section of data by comparing its
 *          calculated CRC signature with the received CRC signature.
 * @param[in] message   contains the message ID, DLC and endianness
 * @param   kpkCanData  Pointer of the received data
 * @return  0 if the function has been executed successfully.
 */
extern uint32_t CAN_RxCrc8Bytes(CAN_MESSAGE_PROPERTIES_s message, const uint8_t *const kpkCanData);
/**@}*/

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
#endif
#endif /* FOXBMS__CAN_CBS_H_ */
