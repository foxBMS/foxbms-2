/**
 *
 * @copyright &copy; 2010 - 2024, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * @file    debug_can_afe.c
 * @author  foxBMS Team
 * @date    2020-09-17 (date of creation)
 * @updated 2024-08-08 (date of last update)
 * @version v1.7.0
 * @ingroup DRIVERS
 * @prefix  DECAN
 *
 * @brief   Implementation of the fake AFE driver
 * @details TODO
 */

/*========== Includes =======================================================*/

#include "afe.h"
#include "debug_can.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/
extern STD_RETURN_TYPE_e AFE_TriggerIc(void) {
    return DECAN_TriggerAfe();
}

extern STD_RETURN_TYPE_e AFE_Initialize(void) {
    return DECAN_Initialize();
}

extern STD_RETURN_TYPE_e AFE_StartMeasurement(void) {
    return STD_OK;
}

extern bool AFE_IsFirstMeasurementCycleFinished(void) {
    return true;
}

extern STD_RETURN_TYPE_e AFE_RequestIoWrite(uint8_t string) {
    /* this is a dummy implementation and not using the argument here is fine */
    (void)string;
    return STD_OK;
}

extern STD_RETURN_TYPE_e AFE_RequestIoRead(uint8_t string) {
    /* this is a dummy implementation and not using the argument here is fine */
    (void)string;
    return STD_OK;
}

extern STD_RETURN_TYPE_e AFE_RequestTemperatureRead(uint8_t string) {
    /* this is a dummy implementation and not using the argument here is fine */
    (void)string;
    return STD_OK;
}

extern STD_RETURN_TYPE_e AFE_RequestBalancingFeedbackRead(uint8_t string) {
    /* this is a dummy implementation and not using the argument here is fine */
    (void)string;
    return STD_OK;
}

extern STD_RETURN_TYPE_e AFE_RequestEepromRead(uint8_t string) {
    /* this is a dummy implementation and not using the argument here is fine */
    (void)string;
    return STD_OK;
}

extern STD_RETURN_TYPE_e AFE_RequestEepromWrite(uint8_t string) {
    /* this is a dummy implementation and not using the argument here is fine */
    (void)string;
    return STD_OK;
}

extern STD_RETURN_TYPE_e AFE_RequestOpenWireCheck(uint8_t string) {
    /* this is a dummy implementation and not using the argument here is fine */
    (void)string;
    return STD_OK;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
