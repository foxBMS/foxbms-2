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
 * @file    can.h
 * @author  foxBMS Team
 * @date    2019-12-04 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup DRIVERS
 * @prefix  CAN
 *
 * @brief   Header for the driver for the CAN module
 * @details Provides the interfaces for initialization, receive
 *          and transmit handling
 */

#ifndef FOXBMS__CAN_H_
#define FOXBMS__CAN_H_

/*========== Includes =======================================================*/
#include "can_cfg.h"

#include "fstd_types.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/
/** Half of the 64 message boxes are defined for TX
 *  This is used to determined in the CAN interrupt routine whether the TX or
 *  RX case need to be handled.
 */
#define CAN_NR_OF_TX_MESSAGE_BOX (32u)

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/

/**
 * @brief   Sends over CAN the data passed in parameters.
 * @details This function goes over the message boxes and marks the ones that
 *          should be sent.
 * @param[in]   message message of the type of CAN_BUFFER_ELEMENT_s to be
 *                      sent
 * @return  #STD_OK if a message box was free to send, #STD_NOT_OK otherwise
 */
extern STD_RETURN_TYPE_e CAN_SendMessage(CAN_BUFFER_ELEMENT_s message);

/**
 * @brief   Enables the CAN transceiver..
 * @details This function sets th pins to enable the CAN transceiver.
 *          It must be called before using the CAN interface.
 */
extern void CAN_Initialize(void);

/**
 * @brief   Send the bootloader version info
 * @return  #STD_OK if this message can be sent, #STD_NOT_OK otherwise
 */
extern STD_RETURN_TYPE_e CAN_SendBootloaderVersionInfo(void);

/**
 * @brief   Send the bootloader commit hash
 * @return  #STD_OK if this message can be sent, #STD_NOT_OK otherwise
 */
extern STD_RETURN_TYPE_e CAN_SendBootloaderCommitHash(void);

/**
 * @brief   Send the current data transfer information
 * @return  #STD_OK if this message can be sent, #STD_NOT_OK otherwise
 */
extern STD_RETURN_TYPE_e CAN_SendDataTransferInfo(void);

/**
 * @brief   Send the bootloader info
 * @return  #STD_OK if this message can be sent, #STD_NOT_OK otherwise
 */
extern STD_RETURN_TYPE_e CAN_SendBootloaderInfo(void);

/**
 * @brief   Send the acknowledge message to acknowledge the reception of some
 *          commands
 * @param[in]   acknowledgeFlag     acknowledge flag will be set if the
 *                                  acknowledge is valid
 * @param[in]   acknowledgeMessage  which message is to be acknowledged
 * @param[in]   statusCode          the status of the processing for the
 *                                  command
 * @param[in]   response            'yes/no' response to some question
 * @return  #STD_OK if this message can be sent, #STD_NOT_OK otherwise
 */
extern STD_RETURN_TYPE_e CAN_SendAcknowledgeMessage(
    CAN_ACKNOWLEDGE_FLAG_e acknowledgeFlag,
    CAN_ACKNOWLEDGE_MESSAGE_e acknowledgeMessage,
    CAN_STATUS_CODE_e statusCode,
    CAN_YES_NO_FLAG_e response);

/**
 * @brief   Send a boot message
 * @details Sends in the following order:
 *          - Magic boot start sequence
 *          - Bootloader version information
 *          - Bootloader commit hash (shortend to 14 characters)
 *          - Magic boot end sequence
 */

extern void CAN_SendBootMessage(void);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
#endif

#endif /* FOXBMS__CAN_H_ */
