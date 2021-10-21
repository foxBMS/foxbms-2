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
 * @file    led.h
 * @author  foxBMS Team
 * @date    2021-09-28 (date of creation)
 * @updated 2021-10-05 (date of last update)
 * @ingroup DRIVERS
 * @prefix  LED
 *
 * @brief   Header file of the debug LED driver
 * @details This module controls the debug LED that is directly connected to a
 *          MCU pin. It is used to indicate basic operation states.
 *
 *          * During startup the LED is enabled continuously.
 *          * Once the startup is complete the LED should be called with
 *            #LED_SetToggleTime() in order to indicate whether the BMS has
 *            started normal operation (#LED_NORMAL_OPERATION_ON_OFF_TIME_ms,
 *            slow blinking pattern) or if it is in an error state
 *            (#LED_ERROR_OPERATION_ON_OFF_TIME_ms, fast blinking pattern).
 *          * #LED_SetToggleTime() can be called anytime the indicated pattern
 *            shall change.
 */

#ifndef FOXBMS__LED_H_
#define FOXBMS__LED_H_

/*========== Includes =======================================================*/
#include "general.h"

/*========== Macros and Definitions =========================================*/
/** time span to be used with #LED_SetToggleTime() indicating normal operation */
#define LED_NORMAL_OPERATION_ON_OFF_TIME_ms (500u)
/** time span to be used with #LED_SetToggleTime() indicating an error state */
#define LED_ERROR_OPERATION_ON_OFF_TIME_ms (100u)

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/
/**
 * @brief   Turn debug LED on
 * @details This function is intended to be called once during startup before
 *          #LED_SetToggleTime() is called (after startup).
 */
extern void LED_SetDebugLED(void);

/**
 * @brief   Trigger function to periodically toggle debug LED. Frequency can be
 *          set via function #LED_SetToggleTime()
 */
extern void LED_Trigger(void);

/**
 * @brief   Set debug LED with defined frequency
 * @param   onOffTime_ms requested time for ON/OFF duration in ms
 */
extern void LED_SetToggleTime(uint32_t onOffTime_ms);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/

#endif /* FOXBMS__LED_H_ */
