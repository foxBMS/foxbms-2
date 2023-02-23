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
 * @file    bender_iso165c.h
 * @author  foxBMS Team
 * @date    2019-04-07 (date of creation)
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
 * @ingroup DRIVERS
 * @prefix  I165C
 *
 * @brief   Headers for the driver for the insulation monitoring
 *
 * @details Module header for insulation measurements. Sets up the interface to
 *          the IO and TIM module.
 *          Also sets up Bender Isometer specific settings.
 */

#ifndef FOXBMS__BENDER_ISO165C_H_
#define FOXBMS__BENDER_ISO165C_H_

/*========== Includes =======================================================*/

#include "bender_iso165c_cfg.h"

#include "imd.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
extern void TEST_I165C_ResetCanData(CAN_BUFFER_ELEMENT_s *canMessage);
extern void TEST_I165C_WriteDataWord(uint8_t dataWord, uint16_t data, CAN_BUFFER_ELEMENT_s *canMessage);
extern void TEST_I165C_ReadDataWord(uint8_t dataWord, uint16_t *data, CAN_BUFFER_ELEMENT_s canMessage);
extern void TEST_I165C_ReadDataWordImdInfo(uint8_t dataWord, uint16_t *data, CAN_BUFFER_ELEMENT_s canMessage);
extern void TEST_I165C_ReadDataByte(uint8_t dataByte, uint8_t *data, CAN_BUFFER_ELEMENT_s canMessage);
extern void TEST_I165C_WriteCmd(uint8_t id, uint8_t command, CAN_BUFFER_ELEMENT_s *canMessage);
extern bool TEST_I165C_CheckResponse(uint8_t command, CAN_BUFFER_ELEMENT_s *canMessage);
extern bool TEST_I165C_GetImdInfo(CAN_BUFFER_ELEMENT_s *canMessage);
extern bool TEST_I165C_IsSelfTestFinished(CAN_BUFFER_ELEMENT_s canMessage);
extern bool TEST_I165C_CheckAcknowledgeArrived(uint8_t command, uint8_t *tries, CAN_BUFFER_ELEMENT_s *canMessage);
#endif

#endif /* FOXBMS__BENDER_ISO165C_H_ */
