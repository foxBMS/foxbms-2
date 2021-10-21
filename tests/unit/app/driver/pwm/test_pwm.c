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
 * @file    test_pwm.c
 * @author  foxBMS Team
 * @date    2021-10-08 (date of creation)
 * @updated 2021-10-08 (date of last update)
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Tests for the PWM driver
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "MockHL_etpwm.h"

#include "pwm.h"
#include "test_assert_helper.h"

/*========== Definitions and Implementations for Unit Test ==================*/
static uint8_t fsysRaisePrivilegeReturnValue = 0u;
long FSYS_RaisePrivilege(void) {
    return fsysRaisePrivilegeReturnValue;
}

/** wraps the duty cycle function for test
 *
 * The output duty cycle is tested against the value computed by the function.
 *
 * @param[in]   timeBasePeriod  This value will be injected as a time base period
 * @param[in]   dutyCycleIn     This value will be passed as requested duty cycle in permill
 * @param[in]   dutyCycleOut    The expected output duty cycle (in counts)
 */
void PWM_SetDutyCycle_Test(uint16_t timeBasePeriod, uint16_t dutyCycleIn, uint16_t dutyCycleOut) {
    etpwm_config_reg_t etPwmConfig = {0};
    etPwmConfig.CONFIG_TBPRD       = timeBasePeriod;

    etpwm1GetConfigValue_Expect(NULL_PTR, CurrentValue);
    etpwm1GetConfigValue_IgnoreArg_config_reg();
    etpwm1GetConfigValue_ReturnThruPtr_config_reg(&etPwmConfig);

    etpwmSetCmpA_Expect(NULL_PTR, dutyCycleOut);
    etpwmSetCmpA_IgnoreArg_etpwm();
    PWM_SetDutyCycle(dutyCycleIn);
}

/** calculates the PWM counter value
 *
 * @param[in]   timeBasePeriod  This time base period will be assumed for the calculation
 * @param[in]   dutyCycle       The requested duty cycle in permill
 */
uint16_t calculateCounterValue(uint16_t timeBasePeriod, uint16_t dutyCycle) {
    /* retrieves the tuning value from the module */
    int16_t linearOffset = TEST_PWM_GetLinearOffset();

    double correctedDutyCycle = ((double)dutyCycle + linearOffset);
    if (correctedDutyCycle < 0) {
        correctedDutyCycle = 0;
    }

    /* bound to upper and lower threshold */
    if (correctedDutyCycle < 1) {
        correctedDutyCycle = 1;
    }
    if (correctedDutyCycle > 999) {
        correctedDutyCycle = 999;
    }

    uint16_t counterSteps = (((double)timeBasePeriod * correctedDutyCycle) / 1000);

    return counterSteps;
}

/*========== Setup and Teardown =============================================*/
void setUp(void) {
    fsysRaisePrivilegeReturnValue = 0;
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/

/** test that the start function calls etPWM API */
void testPWM_StartPwm(void) {
    /* an assertion should happen, when privileges cannot be raised */
    fsysRaisePrivilegeReturnValue = 1;
    TEST_ASSERT_FAIL_ASSERT(PWM_StartPwm();)

    /* otherwise the API should be called */
    fsysRaisePrivilegeReturnValue = 0;
    etpwmStartTBCLK_Expect();
    PWM_StartPwm();
}

/** test that the stop function calls etPWM API */
void testPWM_StopPwm(void) {
    /* an assertion should happen, when privileges cannot be raised */
    fsysRaisePrivilegeReturnValue = 1;
    TEST_ASSERT_FAIL_ASSERT(PWM_StopPwm();)

    /* otherwise the API should be called */
    fsysRaisePrivilegeReturnValue = 0;
    etpwmStopTBCLK_Expect();
    PWM_StopPwm();
}

/** tests the duty cycle function */
void testPWM_SetDutyCycle(void) {
    uint16_t timeBasePeriod = 999u;
    for (uint16_t duty = 1u; duty <= 999; duty = duty + 10) {
        PWM_SetDutyCycle_Test(timeBasePeriod, duty, calculateCounterValue(timeBasePeriod, duty));
    }

    timeBasePeriod = 4999;
    uint16_t duty  = 0u;
    PWM_SetDutyCycle_Test(timeBasePeriod, duty, calculateCounterValue(timeBasePeriod, duty));
    for (uint16_t duty = 1u; duty <= 999; duty = duty + 10) {
        PWM_SetDutyCycle_Test(timeBasePeriod, duty, calculateCounterValue(timeBasePeriod, duty));
    }
}
