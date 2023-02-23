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
 * @file    nxp_mc33775a_afe.c
 * @author  foxBMS Team
 * @date    2020-05-08 (date of creation)
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
 * @ingroup DRIVERS_CONFIGURATION
 * @prefix  N775
 *
 * @brief   Configuration for the MC33775A analog front-end
 *
 */

/*========== Includes =======================================================*/

/* clang-format off */
#include "nxp_afe.h"
/* clang-format on */
#include "afe_dma.h"
#include "dma.h"
#include "nxp_mc33775a.h"
#include "pex.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/
extern STD_RETURN_TYPE_e NXP_Measure(void) {
    N775_Meas(&n775_stateBase);
    return STD_OK;
}

extern STD_RETURN_TYPE_e NXP_Initialize(void) {
    PEX_SetPinDirectionOutput(PEX_PORT_EXPANDER3, PEX_PIN10);
    PEX_SetPin(PEX_PORT_EXPANDER3, PEX_PIN10);
    /* Leave time for the PEX to be triggered in the 10ms task */
    uint32_t current_time = OS_GetTickCount();
    OS_DelayTaskUntil(&current_time, 10u);
    return STD_OK;
}

extern STD_RETURN_TYPE_e NXP_StartMeasurement(void) {
    STD_RETURN_TYPE_e retval = STD_NOT_OK;

    /* if (N775_SetStateRequest(N775_STATE_INITIALIZATION_REQUEST) == N775_OK) {
        retval = STD_OK;
    } */

    return retval;
}

extern bool NXP_IsFirstMeasurementCycleFinished(void) {
    return N775_IsFirstMeasurementCycleFinished(&n775_stateBase);
}

extern STD_RETURN_TYPE_e NXP_RequestIoWrite(uint8_t string) {
    STD_RETURN_TYPE_e retval = STD_NOT_OK;
    return retval;
}

extern STD_RETURN_TYPE_e NXP_RequestIoRead(uint8_t string) {
    STD_RETURN_TYPE_e retval = STD_NOT_OK;
    return retval;
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
