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
 * @file    ltc_afe.c
 * @author  foxBMS Team
 * @date    2020-05-08 (date of creation)
 * @updated 2021-03-24 (date of last update)
 * @ingroup DRIVER
 * @prefix  AFE
 *
 * @brief   AFE driver implementation for LTC ICs
 */

/*========== Includes =======================================================*/
#include "general.h"
/* clang-format off */
#include "afe.h"
/* clang-format on */
#include "ltc.h"

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/

extern STD_RETURN_TYPE_e AFE_TriggerIc(void) {
    LTC_Trigger(&ltc_stateBase);
    return STD_OK;
}

extern STD_RETURN_TYPE_e AFE_Initialize(void) {
    LTC_monitoringPinInit();
    return STD_OK;
}

extern STD_RETURN_TYPE_e AFE_RequestEepromRead(uint8_t string) {
    FAS_ASSERT(string < BS_NR_OF_STRINGS);
    STD_RETURN_TYPE_e retval = STD_OK;
    LTC_REQUEST_s statereq   = {.request = LTC_STATE_EEPROM_READ_REQUEST, .string = string};

    if (LTC_SetStateRequest(&ltc_stateBase, statereq) != LTC_OK) {
        retval = STD_NOT_OK;
    }
    return retval;
}

extern STD_RETURN_TYPE_e AFE_RequestEepromWrite(uint8_t string) {
    FAS_ASSERT(string < BS_NR_OF_STRINGS);
    STD_RETURN_TYPE_e retval = STD_OK;
    LTC_REQUEST_s statereq   = {.request = LTC_STATE_EEPROM_WRITE_REQUEST, .string = string};

    if (LTC_SetStateRequest(&ltc_stateBase, statereq) != LTC_OK) {
        retval = STD_NOT_OK;
    }
    return retval;
}

extern STD_RETURN_TYPE_e AFE_RequestTemperatureRead(uint8_t string) {
    FAS_ASSERT(string < BS_NR_OF_STRINGS);
    STD_RETURN_TYPE_e retval = STD_OK;
    LTC_REQUEST_s statereq   = {.request = LTC_STATE_TEMP_SENS_READ_REQUEST, .string = string};

    if (LTC_SetStateRequest(&ltc_stateBase, statereq) != LTC_OK) {
        retval = STD_NOT_OK;
    }
    return retval;
}

extern STD_RETURN_TYPE_e AFE_RequestBalancingFeedbackRead(uint8_t string) {
    FAS_ASSERT(string < BS_NR_OF_STRINGS);
    STD_RETURN_TYPE_e retval = STD_NOT_OK;
    LTC_REQUEST_s statereq   = {.request = LTC_STATE_BALANCEFEEDBACK_REQUEST, .string = string};

    if (LTC_SetStateRequest(&ltc_stateBase, statereq) == LTC_OK) {
        retval = STD_OK;
    }

    return retval;
}

extern STD_RETURN_TYPE_e AFE_RequestOpenWireCheck(uint8_t string) {
    FAS_ASSERT(string < BS_NR_OF_STRINGS);
    STD_RETURN_TYPE_e retval = STD_OK;
    LTC_REQUEST_s statereq   = {.request = LTC_STATE_OPENWIRE_CHECK_REQUEST, .string = string};

    if (LTC_SetStateRequest(&ltc_stateBase, statereq) != LTC_OK) {
        retval = STD_NOT_OK;
    }
    return retval;
}

extern STD_RETURN_TYPE_e AFE_StartMeasurement(void) {
    STD_RETURN_TYPE_e retval = STD_OK;
    LTC_REQUEST_s statereq   = {.request = LTC_STATE_INIT_REQUEST, .string = 0x0u};

    if (LTC_SetStateRequest(&ltc_stateBase, statereq) != LTC_OK) {
        retval = STD_NOT_OK;
    }
    return retval;
}

extern bool AFE_IsFirstMeasurementCycleFinished(void) {
    return LTC_IsFirstMeasurementCycleFinished(&ltc_stateBase);
}

extern STD_RETURN_TYPE_e AFE_RequestIoRead(uint8_t string) {
    FAS_ASSERT(string < BS_NR_OF_STRINGS);

    STD_RETURN_TYPE_e retval = STD_NOT_OK;

    return (retval);
}

STD_RETURN_TYPE_e AFE_RequestIoWrite(uint8_t string) {
    FAS_ASSERT(string < BS_NR_OF_STRINGS);

    STD_RETURN_TYPE_e retval = STD_NOT_OK;

    return (retval);
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
