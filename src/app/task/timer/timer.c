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
 * @file    timer.c
 * @author  foxBMS Team
 * @date    2024-10-31 (date of creation)
 * @updated 2024-12-20 (date of last update)
 * @version v1.8.0
 * @ingroup TASK
 * @prefix  TIMER
 *
 * @brief   Wrapper for timer implementation
 * @details Our own wrapper for the timer implementation of FreeRTOS
 *
 */

/*========== Includes =======================================================*/
#include "timer.h"

#include "fassert.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/
/* start-include-in-doc */

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/

TimerHandle_t TIMER_Create(
    const char *cpxTimerName,
    uint32_t uxTimerPeriodInMS,
    const UBaseType_t cuxAutoReload,
    void *const cpxTimerID,
    TimerCallbackFunction_t pxCallbackFunction,
    StaticTimer_t *pxTimerBuffer) {
    /* Asserts to check if our parameters exist and make at least some sense*/
    FAS_ASSERT(pxTimerBuffer != NULL);
    FAS_ASSERT(cpxTimerName != NULL);
    FAS_ASSERT(pxCallbackFunction != NULL);

    TimerHandle_t timerHandle = xTimerCreateStatic(
        cpxTimerName, pdMS_TO_TICKS(uxTimerPeriodInMS), cuxAutoReload, cpxTimerID, pxCallbackFunction, pxTimerBuffer);

    return timerHandle;
}

STD_RETURN_TYPE_e TIMER_Delete(TimerHandle_t timerHandle, uint32_t ticks2wait) {
    /* AXIVION Routine Generic-MissingParameterAssert: ticks2wait: parameter accept whole range */
    STD_RETURN_TYPE_e ret = STD_NOT_OK;
    if (timerHandle != NULL) {
        if (xTimerDelete(timerHandle, ticks2wait) == pdPASS) {
            ret = STD_OK;
        }
    }
    return ret;
}

STD_RETURN_TYPE_e TIMER_Start(TimerHandle_t timerHandle, uint32_t ticks2wait) {
    /* AXIVION Routine Generic-MissingParameterAssert: ticks2wait: parameter accept whole range */
    STD_RETURN_TYPE_e ret = STD_NOT_OK;
    if (timerHandle != NULL) {
        if (xTimerStart(timerHandle, ticks2wait) == pdPASS) {
            ret = STD_OK;
        }
    }
    return ret;
}

STD_RETURN_TYPE_e TIMER_Stop(TimerHandle_t timerHandle, uint32_t ticks2wait) {
    /* AXIVION Routine Generic-MissingParameterAssert: ticks2wait: parameter accept whole range */
    STD_RETURN_TYPE_e ret = STD_NOT_OK;
    if (timerHandle != NULL) {
        if (xTimerStop(timerHandle, ticks2wait) == pdPASS) {
            ret = STD_OK;
        }
    }
    return ret;
}

STD_RETURN_TYPE_e TIMER_Reset(TimerHandle_t timerHandle, uint32_t ticks2wait) {
    /* AXIVION Routine Generic-MissingParameterAssert: ticks2wait: parameter accept whole range */
    STD_RETURN_TYPE_e ret = STD_NOT_OK;
    if (timerHandle != NULL) {
        if (xTimerReset(timerHandle, ticks2wait) == pdPASS) {
            ret = STD_OK;
        }
    }
    return ret;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST

#endif
/* stop-include-in-doc */
