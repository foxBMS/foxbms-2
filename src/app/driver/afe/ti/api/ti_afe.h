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
 * @file    ti_afe.h
 * @author  foxBMS Team
 * @date    2023-09-12 (date of creation)
 * @updated 2023-10-12 (date of last update)
 * @version v1.6.0
 * @ingroup DRIVER
 * @prefix  TID
 *
 * @brief   AFE driver wrapper layer for TI ICs
 */

#ifndef FOXBMS__TI_AFE_H_
#define FOXBMS__TI_AFE_H_

/*========== Includes =======================================================*/
#include "fstd_types.h"

/* clang-format off */
#include "afe.h"
/* clang-format on */

#include <stdbool.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/

/**
 * @brief   This function is used to wrap the TI AFE measurement API to the
 *          generic AFE measurement API
 * @details This function is used to wrap #AFE_TriggerIc
 */
extern STD_RETURN_TYPE_e TI_Measure(void);

/**
 * @brief   This function is used to wrap the TI AFE measurement API to the
 *          generic AFE measurement API
 * @details This function is used to wrap #AFE_Initialize
 */
extern STD_RETURN_TYPE_e TI_Initialize(void);

/**
 * @brief   This function is used to wrap the TI AFE measurement API to the
 *          generic AFE measurement API
 * @details This function is used to wrap #AFE_RequestEepromRead
 */
extern STD_RETURN_TYPE_e TI_RequestEepromRead(uint8_t string);

/**
 * @brief   This function is used to wrap the TI AFE measurement API to the
 *          generic AFE measurement API
 * @details This function is used to wrap #AFE_RequestEepromWrite
 */
extern STD_RETURN_TYPE_e TI_RequestEepromWrite(uint8_t string);

/**
 * @brief   This function is used to wrap the TI AFE measurement API to the
 *          generic AFE measurement API
 * @details This function is used to wrap #AFE_RequestTemperatureRead
 */
extern STD_RETURN_TYPE_e TI_RequestTemperatureRead(uint8_t string);

/**
 * @brief   This function is used to wrap the TI AFE measurement API to the
 *          generic AFE measurement API
 * @details This function is used to wrap #AFE_RequestBalancingFeedbackRead
 */
extern STD_RETURN_TYPE_e TI_RequestBalancingFeedbackRead(uint8_t string);

/**
 * @brief   This function is used to wrap the TI AFE measurement API to the
 *          generic AFE measurement API
 * @details This function is used to wrap #AFE_RequestOpenWireCheck
 */
extern STD_RETURN_TYPE_e TI_RequestOpenWireCheck(uint8_t string);

/**
 * @brief   This function is used to wrap the TI AFE measurement API to the
 *          generic AFE measurement API
 * @details This function is used to wrap #AFE_StartMeasurement
 */
extern STD_RETURN_TYPE_e TI_StartMeasurement(void);

/**
 * @brief   This function is used to wrap the TI AFE measurement API to the
 *          generic AFE measurement API
 * @details This function is used to wrap #AFE_IsFirstMeasurementCycleFinished
 */
extern bool TI_IsFirstMeasurementCycleFinished(void);

/**
 * @brief   This function is used to wrap the TI AFE measurement API to the
 *          generic AFE measurement API
 * @details This function is used to wrap #AFE_RequestIoRead
 */
extern STD_RETURN_TYPE_e TI_RequestIoRead(uint8_t string);

/**
 * @brief   This function is used to wrap the TI AFE measurement API to the
 *          generic AFE measurement API
 * @details This function is used to wrap #AFE_RequestIoWrite
 */
STD_RETURN_TYPE_e TI_RequestIoWrite(uint8_t string);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
#endif

#endif /* FOXBMS__TI_AFE_H_ */
