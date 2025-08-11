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
 * @file    nxp_mc33775a_afe.c
 * @author  foxBMS Team
 * @date    2025-02-17 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup DRIVERS
 * @prefix  N775
 *
 * @brief   API for MC33775A Analog Frontend
 * @details TODO:
 */

/*========== Includes =======================================================*/

/* clang-format off */
#include "nxp_afe.h"
/* clang-format on */
#include "afe_dma.h"
#include "dma.h"
#include "nxp_mc3377x.h"
#include "pex.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/
extern STD_RETURN_TYPE_e NXP_Measure(void) {
    N77x_Measure(&n77x_stateBase);
    return STD_OK;
}

extern STD_RETURN_TYPE_e NXP_Initialize(void) {
    PEX_SetPinDirectionOutput(PEX_PORT_EXPANDER3, PEX_PORT_1_PIN_0);
    PEX_SetPin(PEX_PORT_EXPANDER3, PEX_PORT_1_PIN_0);
    /* Leave time for the PEX to be triggered in the 10ms task */
    uint32_t current_time = OS_GetTickCount();
    OS_DelayTaskUntil(&current_time, 10u);
    return STD_OK;
}

extern STD_RETURN_TYPE_e NXP_StartMeasurement(void) {
    STD_RETURN_TYPE_e retval = STD_NOT_OK;
    return retval;
}

extern bool NXP_IsFirstMeasurementCycleFinished(void) {
    return N77x_IsFirstMeasurementCycleFinished(&n77x_stateBase);
}

extern STD_RETURN_TYPE_e NXP_RequestTemperatureRead(uint8_t string) {
    STD_RETURN_TYPE_e retval = STD_NOT_OK;
    return retval;
}

extern STD_RETURN_TYPE_e NXP_RequestBalancingFeedbackRead(uint8_t string) {
    STD_RETURN_TYPE_e retval = STD_NOT_OK;
    return retval;
}

extern STD_RETURN_TYPE_e NXP_RequestEepromRead(uint8_t string) {
    STD_RETURN_TYPE_e retval = STD_NOT_OK;
    return retval;
}

extern STD_RETURN_TYPE_e NXP_RequestEepromWrite(uint8_t string) {
    STD_RETURN_TYPE_e retVal = STD_NOT_OK;
    return retVal;
}

extern STD_RETURN_TYPE_e NXP_RequestOpenWireCheck(uint8_t string) {
    STD_RETURN_TYPE_e retVal = STD_NOT_OK;
    return retVal;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
