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
 * @file    ti_dummy.h
 * @author  foxBMS Team
 * @date    2023-09-11 (date of creation)
 * @updated 2025-03-31 (date of last update)
 * @version v1.9.0
 * @ingroup DRIVERS_CONFIGURATION
 * @prefix  TIDUM
 *
 * @brief   Configuration for the TI dummy analog front-end
 * @details TODO
 */

#ifndef FOXBMS__TI_DUMMY_H_
#define FOXBMS__TI_DUMMY_H_

/*========== Includes =======================================================*/
#include "fstd_types.h"

/* clang-format off */
#include "ti_afe.h"
/* clang-format on */

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/
extern STD_RETURN_TYPE_e TIDUM_Measure(void);
extern STD_RETURN_TYPE_e TIDUM_Initialize(void);
extern STD_RETURN_TYPE_e TIDUM_RequestEepromRead(uint8_t string);
extern STD_RETURN_TYPE_e TIDUM_RequestEepromWrite(uint8_t string);
extern STD_RETURN_TYPE_e TIDUM_RequestTemperatureRead(uint8_t string);
extern STD_RETURN_TYPE_e TIDUM_RequestBalancingFeedbackRead(uint8_t string);
extern STD_RETURN_TYPE_e TIDUM_RequestOpenWireCheck(uint8_t string);
extern STD_RETURN_TYPE_e TIDUM_StartMeasurement(void);
extern bool TIDUM_IsFirstMeasurementCycleFinished(void);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
#endif

#endif /* FOXBMS__TI_DUMMY_H_ */
