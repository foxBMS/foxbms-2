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
 * @file    nxp_afe.h
 * @author  foxBMS Team
 * @date    2021-06-04 (date of creation)
 * @updated 2022-07-28 (date of last update)
 * @version v1.4.0
 * @ingroup DRIVER
 * @prefix  NXP
 *
 * @brief   Measurement IC driver wrapper layer for NXP ICs
 */

#ifndef FOXBMS__NXP_AFE_H_
#define FOXBMS__NXP_AFE_H_

/*========== Includes =======================================================*/
#include "general.h"
/* clang-format off */
#include "afe.h"
/* clang-format on */

/*========== Macros and Definitions =========================================*/

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/

extern STD_RETURN_TYPE_e NXP_Measure(void);
extern STD_RETURN_TYPE_e NXP_Initialize(void);
extern STD_RETURN_TYPE_e NXP_RequestEepromRead(uint8_t string);
extern STD_RETURN_TYPE_e NXP_RequestEepromWrite(uint8_t string);
extern STD_RETURN_TYPE_e NXP_RequestTemperatureRead(uint8_t string);
extern STD_RETURN_TYPE_e NXP_RequestBalancingFeedbackRead(uint8_t string);
extern STD_RETURN_TYPE_e NXP_RequestOpenWireCheck(uint8_t string);
extern STD_RETURN_TYPE_e NXP_StartMeasurement(void);
extern bool NXP_IsFirstMeasurementCycleFinished(void);
extern STD_RETURN_TYPE_e NXP_RequestIoRead(uint8_t string);
STD_RETURN_TYPE_e NXP_RequestIoWrite(uint8_t string);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/

#endif /* FOXBMS__NXP_AFE_H_ */
