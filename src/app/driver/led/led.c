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
 * @file    led.c
 * @author  foxBMS Team
 * @date    2021-09-28 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup DRIVERS
 * @prefix  LED
 *
 * @brief   Implementation of the debug LED driver
 * @details TODO
 */

/*========== Includes =======================================================*/
#include "led.h"

#include "ftask_cfg.h"

#include "HL_reg_het.h"

#include "fassert.h"
#include "io.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/
/** Periodic call time of function #LED_Trigger */
#define LED_PERIODIC_CALL_TIME_ms (FTSK_TASK_CYCLIC_100MS_CYCLE_TIME)

/** HET1 GIO register that the Debug LED is connected to. */
#define LED_PORT (hetREG1)
/** Pin of HET1 that the Debug LED is connected to. */
#define LED_PIN (1u)

/*========== Static Constant and Variable Definitions =======================*/
/** duration of a half a LED blink cycle in ms */
static uint32_t led_onOffTime_ms = 0u;

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/
extern void LED_SetDebugLed(void) {
    IO_PinSet(&LED_PORT->DOUT, LED_PIN);
}

extern void LED_Trigger(void) {
    OS_EnterTaskCritical();
    uint32_t led_tmpOnOffTime_ms = led_onOffTime_ms;
    OS_ExitTaskCritical();
    FAS_ASSERT(led_tmpOnOffTime_ms != 0u);

    static uint32_t counter = 0u;
    if (counter == UINT32_MAX) {
        /* wrap counter */
        counter = 0u;
    }
    counter++;

    if (0u == (((uint64_t)counter * LED_PERIODIC_CALL_TIME_ms) % led_tmpOnOffTime_ms)) {
        if (STD_PIN_HIGH == IO_PinGet(&LED_PORT->DIN, LED_PIN)) {
            IO_PinReset(&LED_PORT->DOUT, LED_PIN);
        } else {
            IO_PinSet(&LED_PORT->DOUT, LED_PIN);
        }
        counter = 0u;
    }
}

extern void LED_SetToggleTime(uint32_t onOffTime_ms) {
    FAS_ASSERT(0u == (onOffTime_ms % LED_PERIODIC_CALL_TIME_ms));
    led_onOffTime_ms = onOffTime_ms;
    FAS_ASSERT(led_onOffTime_ms != 0u);
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
extern uint32_t TEST_LED_GetOnOffTime(void) {
    return led_onOffTime_ms;
}
extern void TEST_LED_SetOnOffTime(uint32_t val) {
    led_onOffTime_ms = val;
}
#endif
