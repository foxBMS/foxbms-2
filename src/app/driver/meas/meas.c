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
 * @file    meas.c
 * @author  foxBMS Team
 * @date    2020-02-24 (date of creation)
 * @updated 2021-03-24 (date of last update)
 * @ingroup DRIVERS
 * @prefix  MEAS
 *
 * @brief   Driver for the measurements needed by the BMS (e.g., I,V,T).
 *
 */

/*========== Includes =======================================================*/
#include "meas.h"

#include "battery_system_cfg.h"

#include "afe.h"

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/

void MEAS_Control(void) {
    AFE_TriggerIc();
}

extern STD_RETURN_TYPE_e MEAS_RequestIoWrite(uint8_t string) {
    FAS_ASSERT(string < BS_NR_OF_STRINGS);
    return AFE_RequestIoWrite(string);
}

extern STD_RETURN_TYPE_e MEAS_RequestIoRead(uint8_t string) {
    FAS_ASSERT(string < BS_NR_OF_STRINGS);
    return AFE_RequestIoRead(string);
}

extern STD_RETURN_TYPE_e MEAS_RequestTemperatureRead(uint8_t string) {
    FAS_ASSERT(string < BS_NR_OF_STRINGS);
    return AFE_RequestTemperatureRead(string);
}

extern STD_RETURN_TYPE_e MEAS_RequestBalancingFeedbackRead(uint8_t string) {
    FAS_ASSERT(string < BS_NR_OF_STRINGS);
    return AFE_RequestBalancingFeedbackRead(string);
}

extern STD_RETURN_TYPE_e MEAS_RequestEepromRead(uint8_t string) {
    FAS_ASSERT(string < BS_NR_OF_STRINGS);
    return AFE_RequestEepromRead(string);
}

extern STD_RETURN_TYPE_e MEAS_RequestEepromWrite(uint8_t string) {
    FAS_ASSERT(string < BS_NR_OF_STRINGS);
    return AFE_RequestEepromWrite(string);
}

extern bool MEAS_IsFirstMeasurementCycleFinished(void) {
    return AFE_IsFirstMeasurementCycleFinished();
}

extern STD_RETURN_TYPE_e MEAS_StartMeasurement(void) {
    return AFE_StartMeasurement();
}

extern STD_RETURN_TYPE_e MEAS_Initialize(void) {
    return AFE_Initialize();
}

extern STD_RETURN_TYPE_e MEAS_RequestOpenWireCheck(uint8_t string) {
    FAS_ASSERT(string < BS_NR_OF_STRINGS);
    return AFE_RequestOpenWireCheck(string);
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
