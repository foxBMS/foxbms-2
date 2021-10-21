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
 * @file    pwm.h
 * @author  foxBMS Team
 * @date    2021-10-07 (date of creation)
 * @updated 2021-10-08 (date of last update)
 * @ingroup DRIVERS
 * @prefix  PWM
 *
 * @brief   PWM driver for the TMS570LC43xx.
 * @details This module uses the PWM hardware of the MCU in order to generate
 *          a PWM signal. Currently, the implementation uses only the ePWM
 *          (not the HET) and supports only setting a duty cycle (in percent).
 *          For the module to work currently, the function etpwmInit() has to
 *          be called before using API of this module.
 */

#ifndef FOXBMS__PWM_H_
#define FOXBMS__PWM_H_

/*========== Includes =======================================================*/
#include "general.h"

/*========== Macros and Definitions =========================================*/

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/
/** @brief  Start the PWM (starts all configured ePWM channels)
 * @details In order for this function to work, it is absolutely necessary that
 *          etpwmInit() is called first.
*/
extern void PWM_StartPwm(void);

/** @brief  Stop the PWM (stops all configured ePWM channels) */
extern void PWM_StopPwm(void);

/** @brief  Set the duty cycle of the PWM (currently only channel 1A)
 * @details Duty cycle values that are out of the range that is supported are
 *          limited to the threshold values.
 * @param[in]   dutyCycle_perm  duty cycle in permill
*/
extern void PWM_SetDutyCycle(uint16_t dutyCycle_perm);

/*========== Getter for static Variables (Unit Test) ========================*/
#ifdef UNITY_UNIT_TEST
extern int16_t TEST_PWM_GetLinearOffset(void);
#endif

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/

#endif /* FOXBMS__PWM_H_ */
