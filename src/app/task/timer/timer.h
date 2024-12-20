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
 * @file    timer.h
 * @author  foxBMS Team
 * @date    2024-10-31 (date of creation)
 * @updated 2024-12-20 (date of last update)
 * @version v1.8.0
 * @ingroup TASK
 * @prefix  TIMER
 *
 * @brief   Header file for the timer wrapper
 * @details Our wrapper implementation for the software timers from RTOS.
 */

#ifndef FOXBMS__TIMER_H_
#define FOXBMS__TIMER_H_

/*========== Includes =======================================================*/
#include "FreeRTOS.h"
#include "timers.h"

#include "fstd_types.h"
#include "mpu_wrappers.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/
#define DIAG_FatalErrorResendTimerID 1

/*========== Extern Constant and Variable Declarations ======================*/
/* start-include-in-doc */

/*========== Extern Function Prototypes =====================================*/

/**
 * @brief   TIMER_create allows to create a new virtual timer
 * @details This function calls intern xTimerCreateStatic to create a new timer
 *          instance with the given variables.
 *
 * @param   cpxTimerName        the name of the timer
 * @param   uxTimerPeriodInMS   the amount of time it waits before calling the
 *                              callback function pxCallbackFunction
 * @param   cuxAutoReload        whether the timer only fires once or automatically
 *                              restarts after firing
 * @param   cpxTimerID          ID for our timer. Please use the defines in this header to create a new ID
 * @param   pxCallbackFunction  the callback function that gets executed once the timer triggers it
 * @param   pxTimerBuffer       a pointer to an StaticTimer_t type used as memory buffer for the timer
 * @return  a TimeHandle_t to interact with the timer
 */
extern TimerHandle_t TIMER_Create(
    const char *cpxTimerName,
    uint32_t uxTimerPeriodInMS,
    const UBaseType_t cuxAutoReload,
    void *const cpxTimerID,
    TimerCallbackFunction_t pxCallbackFunction,
    StaticTimer_t *pxTimerBuffer);

/**
 * @brief   TIMER_start starts a given virtual timer
 * @details This function calls intern xTimerStart and if the timer command
 *          queue is full waits for x ticks. If the timer has already been
 *          started it acts like TIMER_reset
 *
 * @param   timerHandle the handle of the timer
 * @param   ticks2wait  the amount of ticks to wait if the command queue is full
 * @return  #STD_OK if the timer was started #STD_NOT_OK if something went wrong
 */
extern STD_RETURN_TYPE_e TIMER_Start(TimerHandle_t timerHandle, uint32_t ticks2wait);

/**
 * @brief   TIMER_stops stops a given virtual timer
 * @details This function calls intern xTimerStop and if the timer command
 *          queue is full waits for x ticks.
 *
 * @param   timerHandle the handle of the timer
 * @param   ticks2wait  the amount of ticks to wait if the command queue is full
 * @return  #STD_OK if the timer was stopped #STD_NOT_OK if something went wrong
 */
extern STD_RETURN_TYPE_e TIMER_Stop(TimerHandle_t timerHandle, uint32_t ticks2wait);

/**
 * @brief   TIMER_reset restarts a given virtual timer
 * @details This function calls intern xTimerStart and if the timer command
 *          queue is full waits for x ticks.
 *
 * @param   timerHandle the handle of the timer
 * @param   ticks2wait  the amount of ticks to wait if the command queue is full
 * @return  #STD_OK if the timer was restarted #STD_NOT_OK if something went wrong
 */
extern STD_RETURN_TYPE_e TIMER_Reset(TimerHandle_t timerHandle, uint32_t ticks2wait);

/**
 * @brief   TIMER_delete deletes a given virtual timer
 * @details This function calls intern xTimerDelete and if the timer command
 *          queue is full waits for x ticks.
 *
 * @param   timerHandle the handle of the timer
 * @param   ticks2wait  the amount of ticks to wait if the command queue is full
 * @return  #STD_OK if the timer was deleted #STD_NOT_OK if something went wrong
 */
extern STD_RETURN_TYPE_e TIMER_Delete(TimerHandle_t timerHandle, uint32_t ticks2wait);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST

#endif

/* stop-include-in-doc */
#endif /* FOXBMS__TIMER_H_ */
