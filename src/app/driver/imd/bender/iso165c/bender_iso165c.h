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
 * @file    bender_iso165c.h
 * @author  foxBMS Team
 * @date    2019-04-07 (date of creation)
 * @updated 2021-09-08 (date of last update)
 * @ingroup DRIVERS
 * @prefix  I165C
 *
 * @brief   Headers for the driver for the insulation monitoring
 *
 * Module header for insulation measurements. Sets up the interface to the
 * IO and TIM module. Also sets up Bender Isometer specific settings.
 *
 */

#ifndef FOXBMS__BENDER_ISO165C_H_
#define FOXBMS__BENDER_ISO165C_H_

/*========== Includes =======================================================*/
#include "general.h"

#include "bender_iso165c_cfg.h"

#include "imd.h"

/*========== Macros and Definitions =========================================*/

/**
 * symbolic names for the different operating states Bender Isometer.
 * Defined through the duty cycle of the measurement signal.
 */
typedef enum I165C_STATE {
    I165C_STATE_UNINITIALIZED,
    I165C_STATE_SELFTEST,
    I165C_STATE_SELFTEST_WAIT_ACK,
    I165C_STATE_WAIT_SELFTEST,
    I165C_STATE_INITIALIZATION_FINISHED,
    I165C_STATE_READ_RESISTANCE,
    I165C_STATE_READ_RESISTANCE_WAIT_ACK,
    I165C_STATE_UNLOCK,
    I165C_STATE_UNLOCK_WAIT_ACK,
    I165C_STATE_SET_ERROR_THRESHOLD,
    I165C_STATE_SET_ERROR_THRESHOLD_WAIT_ACK,
    I165C_STATE_SET_WARNING_THRESHOLD,
    I165C_STATE_SET_WARNING_THRESHOLD_WAIT_ACK,
    I165C_STATE_GET_MEASUREMENT,
} I165C_STATE_e;

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
extern void TEST_I165C_ResetCanData(CAN_BUFFERELEMENT_s *canMessage);
extern void TEST_I165C_WriteDataWord(uint8_t dataWord, uint16_t data, CAN_BUFFERELEMENT_s *canMessage);
extern void TEST_I165C_ReadDataWord(uint8_t dataWord, uint16_t *data, CAN_BUFFERELEMENT_s canMessage);
extern void TEST_I165C_ReadDataWordImdInfo(uint8_t dataWord, uint16_t *data, CAN_BUFFERELEMENT_s canMessage);
extern void TEST_I165C_ReadDataByte(uint8_t dataByte, uint8_t *data, CAN_BUFFERELEMENT_s canMessage);
extern void TEST_I165C_WriteCmd(uint8_t id, uint8_t cmd, CAN_BUFFERELEMENT_s *canMessage);
extern bool TEST_I165C_CheckResponse(uint8_t command, CAN_BUFFERELEMENT_s *canMessage);
extern bool TEST_I165C_GetImdInfo(CAN_BUFFERELEMENT_s *canMessage);
extern bool TEST_I165C_IsInitialized(CAN_BUFFERELEMENT_s canMessage);
extern void TEST_I165C_CheckAcknowledgeArrived(
    uint8_t command,
    I165C_STATE_e *currentState,
    I165C_STATE_e nextState,
    uint8_t *tries,
    CAN_BUFFERELEMENT_s *canMessage);
#endif

#endif /* FOXBMS__BENDER_ISO165C_H_ */
