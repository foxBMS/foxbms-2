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
 * @file    bender_ir155_cfg.h
 * @author  foxBMS Team
 * @date    2021-09-17 (date of creation)
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
 * @ingroup DRIVERS_CONFIGURATION
 * @prefix  IR155
 *
 * @brief   Headers for the configuration for the insulation monitoring
 *
 *
 */

#ifndef FOXBMS__BENDER_IR155_CFG_H_
#define FOXBMS__BENDER_IR155_CFG_H_

/*========== Includes =======================================================*/

#include "HL_het.h"

/* clang-format off */
#include "imd.h"
/* clang-format on */

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/**
 * During this time, the results of Bender Isometer aren't to be trusted
 * wait time in [ms] <= 65535;
 * IR155_WAIT_TIME_AFTER_GNDERROR
 * \par Type:
 * int
 * \par Range:
 * 0 < x <=65535
 * \par Unit:
 * ms
 * \par Default:
 * 25000
*/
#define IR155_WAIT_TIME_AFTER_GROUND_ERROR_ms (25000u)

/** Time after startup, until the first valid measurement result is available */
#define IR155_STARTUP_TIME_ms (2200u)

/* Symbolic names for the different measurement modes of Bender Isometer. */

/** 0 Hz -> Hi > short-circuit to Ub + (Kl. 15); Low > IMD off or short-circuit
 *  to Kl. 31 */
#define IR155_MINIMUM_FREQUENCY_Hz (5.0f) /* Define a minimum frequency as 0 Hz can not be measured by PWM module */

/** 10 Hz -> Normal condition. Insulation measurement DCP starts two seconds
 *  after power on; First successful insulation measurement at ? 17.5 s.
 * PWM active 5...95 % */
#define IR155_NORMAL_CONDITION_FREQUENCY_Hz (10.0f)

/** 20 Hz -> undervoltage condition. Insulation measurement DCP (continuous
 *  measurement); starts two seconds after power on; PWM active 5...95 %;
 * First successful insulation measurement at ? 17.5 s,  Undervoltage detection
 * 0...500 V (Bender configurable) */
#define IR155_UNDERVOLTAGE_FREQUENCY_Hz (20.0f)

/** 30 Hz -> Speed start measurement. Insulation measurement (only good/bad
 *  evaluation), starts directly after power on ? 2 s; PWM 5...10 % (good) and
 *  90...95 % (bad) */
#define IR155_SPEEDSTART_FREQUENCY_Hz (30.0f)

/** 40 Hz -> Device error */
#define IR155_IMD_DEVICE_ERROR_FREQUENCY_Hz (40.0f)

/** 50 Hz -> Connection fault earth. Fault detected on the earth connection
 *  (Kl. 31), PWM 47.5...52.5 % */
#define IR155_GROUND_ERROR_FREQUENCY_Hz (50.0f)

/** The exact frequency of the pwm signal cannot be measured due to inaccuracies.
 *  Thus, a interval is used to detect any deviations. The interval is for example
 *  selected as 50 Hz +/- IR155_MEASUREMENT_INTERVAL_RANGE_Hz */
#define IR155_MEASUREMENT_INTERVAL_RANGE_Hz (1.0f)

/* Min and max values are defined for tolerance purposes of the measurement. */
#define IR155_NORMAL_CONDITION_UPPER_FREQUENCY_Hz \
    (IR155_NORMAL_CONDITION_FREQUENCY_Hz + IR155_MEASUREMENT_INTERVAL_RANGE_Hz)
#define IR155_NORMAL_CONDITION_LOWER_FREQUENCY_Hz \
    (IR155_NORMAL_CONDITION_FREQUENCY_Hz - IR155_MEASUREMENT_INTERVAL_RANGE_Hz)

#define IR155_UNDERVOLTAGE_UPPER_FREQUENCY_Hz (IR155_UNDERVOLTAGE_FREQUENCY_Hz + IR155_MEASUREMENT_INTERVAL_RANGE_Hz)
#define IR155_UNDERVOLTAGE_LOWER_FREQUENCY_Hz (IR155_UNDERVOLTAGE_FREQUENCY_Hz - IR155_MEASUREMENT_INTERVAL_RANGE_Hz)

#define IR155_SPEEDSTART_UPPER_FREQUENCY_Hz (IR155_SPEEDSTART_FREQUENCY_Hz + IR155_MEASUREMENT_INTERVAL_RANGE_Hz)
#define IR155_SPEEDSTART_LOWER_FREQUENCY_Hz (IR155_SPEEDSTART_FREQUENCY_Hz - IR155_MEASUREMENT_INTERVAL_RANGE_Hz)

#define IR155_IMD_DEVICE_ERROR_UPPER_FREQUENCY_Hz \
    (IR155_IMD_DEVICE_ERROR_FREQUENCY_Hz + IR155_MEASUREMENT_INTERVAL_RANGE_Hz)
#define IR155_IMD_DEVICE_ERROR_LOWER_FREQUENCY_Hz \
    (IR155_IMD_DEVICE_ERROR_FREQUENCY_Hz - IR155_MEASUREMENT_INTERVAL_RANGE_Hz)

#define IR155_GROUND_ERROR_UPPER_FREQUENCY_Hz (IR155_GROUND_ERROR_FREQUENCY_Hz + IR155_MEASUREMENT_INTERVAL_RANGE_Hz)
#define IR155_GROUND_ERROR_LOWER_FREQUENCY_Hz (IR155_GROUND_ERROR_FREQUENCY_Hz - IR155_MEASUREMENT_INTERVAL_RANGE_Hz)

/** Register that the supply enable pin is connected to. */
#define IR155_SUPPLY_ENABLE_PORT (hetREG1)

/** Pin that the supply enable pin is connected to. */
#define IR155_SUPPLY_ENABLE_PIN (25u)

/** Register that the input status pin (OHKS) is connected to. */
#define IR155_DIGITAL_STATUS_INPUT_PORT (hetREG1)

/** Pin that the input status pin (IM_OK) is connected to. */
#define IR155_DIGITAL_STATUS_INPUT_PIN (27u)

/** Register that the input PWM signal (IM_PWM) is connected to. */
#define IR155_PWM_SIGNAL_PORT (hetREG2) /* TODO: this is correct? */

/** Pin that the input PWM signal pin (IM_PWM) is connected to. */
#define IR155_PWM_SIGNAL_PIN (27u) /* TODO: this is correct? */

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
#endif

#endif /* FOXBMS__BENDER_IR155_CFG_H_ */
