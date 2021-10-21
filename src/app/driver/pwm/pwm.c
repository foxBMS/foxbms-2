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
 * @file    pwm.c
 * @author  foxBMS Team
 * @date    2021-10-07 (date of creation)
 * @updated 2021-10-08 (date of last update)
 * @ingroup DRIVERS
 * @prefix  PWM
 *
 * @brief   Implementation of the PWM module.
 *
 */

/*========== Includes =======================================================*/
#include "pwm.h"

#include "HL_etpwm.h"

#include "fsystem.h"

/*========== Macros and Definitions =========================================*/
/** lower threshold permill */
#define PWM_LOWER_THRESHOLD_PERM (1u)

/** upper threshold permill */
#define PWM_UPPER_THRESHOLD_PERM (999u)

/** full period in promill */
#define PWM_FULL_PERIOD_PERM (1000u)

/*========== Static Constant and Variable Definitions =======================*/
/** linear offset (through output circuit) */
static const int16_t pwm_kLinearOffset = 0;

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/
/** returns the ePWM time period (currently for ePWM1A)
 * @returns ePWM time period in counter ticks
 */
static uint16_t PWM_GetEtpwmTimePeriod(void);

/** returns a ePWM counter value based on a duty cycle
 * @param[in]   dutyCycle_perm  duty cycle in permill
 * @returns     counter value
 */
static uint16_t PWM_ComputeCounterValueFromDutyCycle(uint16_t dutyCycle_perm);

/*========== Static Function Implementations ================================*/
static uint16_t PWM_GetEtpwmTimePeriod(void) {
    etpwm_config_reg_t etPwmConfig = {0};
    etpwm1GetConfigValue(&etPwmConfig, CurrentValue); /* retrieve the current config */
    return etPwmConfig.CONFIG_TBPRD;
}

static uint16_t PWM_ComputeCounterValueFromDutyCycle(uint16_t dutyCycle_perm) {
    FAS_ASSERT((dutyCycle_perm >= PWM_LOWER_THRESHOLD_PERM) && (dutyCycle_perm <= PWM_UPPER_THRESHOLD_PERM));

    uint16_t basePeriod   = PWM_GetEtpwmTimePeriod();
    uint32_t counterValue = (((uint32_t)basePeriod * (uint32_t)dutyCycle_perm) / PWM_FULL_PERIOD_PERM);

    FAS_ASSERT(counterValue <= (uint16_t)UINT16_MAX);
    return (uint16_t)counterValue;
}

/*========== Extern Function Implementations ================================*/
extern void PWM_StartPwm(void) {
    /* go to privileged mode in order to access control register */
    FAS_ASSERT(FSYS_RaisePrivilege() == 0);
    etpwmStartTBCLK();
    /* done; go back to user mode */
    FSYS_SwitchToUserMode();
}

extern void PWM_StopPwm(void) {
    /* go to privileged mode in order to access control register */
    FAS_ASSERT(FSYS_RaisePrivilege() == 0);
    etpwmStopTBCLK();
    /* done; go back to user mode */
    FSYS_SwitchToUserMode();
}

extern void PWM_SetDutyCycle(uint16_t dutyCycle_perm) {
    FAS_ASSERT(dutyCycle_perm <= (uint16_t)INT16_MAX);
    int16_t intermediateDutyCycle_perm = (int16_t)dutyCycle_perm + pwm_kLinearOffset;
    /* prevent wrap-around */
    if (intermediateDutyCycle_perm < 0) {
        intermediateDutyCycle_perm = 0;
    }
    uint16_t correctedDutyCycle_perm = (uint16_t)intermediateDutyCycle_perm;

    if (correctedDutyCycle_perm < PWM_LOWER_THRESHOLD_PERM) {
        correctedDutyCycle_perm = PWM_LOWER_THRESHOLD_PERM;
    }

    if (correctedDutyCycle_perm > PWM_UPPER_THRESHOLD_PERM) {
        correctedDutyCycle_perm = PWM_UPPER_THRESHOLD_PERM;
    }

    etpwmSetCmpA(etpwmREG1, PWM_ComputeCounterValueFromDutyCycle(correctedDutyCycle_perm));
}

/*========== Getter for static Variables (Unit Test) ========================*/
#ifdef UNITY_UNIT_TEST
extern int16_t TEST_PWM_GetLinearOffset(void) {
    return pwm_kLinearOffset;
}
#endif

/*========== Externalized Static Function Implementations (Unit Test) =======*/
