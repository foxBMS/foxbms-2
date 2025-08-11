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
 * @file    bender_ir155_helper.h
 * @author  foxBMS Team
 * @date    2021-09-17 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup DRIVERS
 * @prefix  IR155
 *
 * @brief   Headers for the configuration for the insulation monitoring
 * @details TODO
 */

#ifndef FOXBMS__BENDER_IR155_HELPER_H_
#define FOXBMS__BENDER_IR155_HELPER_H_

/*========== Includes =======================================================*/

/* clang-format off */
#include "imd.h"
/* clang-format on */

#include "io.h"
#include "pwm.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/* Read pin state of digital status pin (OKHS) */
#define IR155_GET_DIGITAL_STATUS_PIN_STATE() \
    IO_PinGet(&IR155_DIGITAL_STATUS_INPUT_PORT->DIN, IR155_DIGITAL_STATUS_INPUT_PIN)

/**
 * symbolic names for the different measurement modes of Bender Isometer.
 * Defined through the frequency of the measurement signal.
 */
typedef enum IR155_MEASUREMENT_MODE {
    IR155_NORMAL_MODE,            /*!< PWM frequency: 10Hz */
    IR155_SPEED_START_MODE,       /*!< PWM frequency: 30Hz */
    IR155_UNDERVOLTAGE_MODE,      /*!< PWM frequency: 20Hz */
    IR155_IMD_ERROR_MODE,         /*!< PWM frequency: 40Hz */
    IR155_GROUND_ERROR_MODE,      /*!< PWM frequency: 50Hz */
    IR155_SHORT_CLAMP,            /*!< PWM frequency:  0Hz */
    IR155_UNDEFINED_FREQUENCY,    /*!< illegal frequency detected*/
    IR155_DUTY_CYCLE_MEASUREMENT, /*!< corrupt signal measurement (e.g., T_on > T_period,)*/
    IR155_NOSIGNAL,               /*!< no signal (e.g. if 100% -> wire break, if 0% -> shortcut to GND */
    IR155_UNKNOWN,
} IR155_MEASUREMENT_MODE_e;

/**
 * symbolic names for the different operating states Bender Isometer.
 * Defined through the duty cycle of the measurement signal.
 */
typedef enum IR155_MEASUREMENT_STATE {
    IR155_RESISTANCE_MEASUREMENT,           /*!< valid normal measurement working */
    IR155_RESISTANCE_MEASUREMENT_UNKNOWN,   /*!< normal measurement with undefined duty cycle */
    IR155_RESISTANCE_ESTIMATION,            /*!< SPEED START estimation working */
    IR155_RESISTANCE_ESTIMATION_UNKNOWN,    /*!< SPEED START estimation with undefined duty cycle */
    IR155_UNDERVOLTAGE_MEASUREMENT,         /*!< SPEED START estimation working */
    IR155_UNDERVOLTAGE_MEASUREMENT_UNKNOWN, /*!< SPEED START estimation with undefined duty cycle */
    IR155_GROUND_ERROR_STATE,               /*!< ground error detected */
    IR155_GROUND_ERROR_STATE_UNKNOWN,       /*!< ground error detected with undefined duty cycle */
    IR155_IMD_ERROR_MEASUREMENT,            /*!< device error detected */
    IR155_IMD_ERROR_MEASUREMENT_UNKNOWN,    /*!< device error detected with unknown duty cycle*/
    IR155_SIGNAL_SHORT,                     /*!< signal short detected */
    IR155_MEASUREMENT_NOT_VALID,            /*!< Invalid measurement detected */
    IR155_UNINITIALIZED,                    /*!< Uninitialized state */
} IR155_MEASUREMENT_STATE_e;

/** type definition for structure of insulation measurement */
typedef struct {
    bool isMeasurementValid;                    /*!< flag if measurement value is valid or not */
    bool isUndervoltageDetected;                /*!< flag if undervoltage has been detected */
    PWM_SIGNAL_s pwmSignal;                     /*!< duty cycle and frequency of measured pwm signal */
    uint32_t resistance_kOhm;                   /*!< measured insulation resistance in kOhm */
    STD_PIN_STATE_e digitalStatusPin;           /*!< pin state of Status output (OK_HS) */
    IR155_MEASUREMENT_MODE_e measurementMode;   /*!< IR155 measurement mode */
    IR155_MEASUREMENT_STATE_e measurementState; /*!< IR155 measurement state */
} IR155_MEASUREMENT_s;

/** IR155 state struct */
typedef struct {
    bool ir155Initialized;                 /*!< flag if IR155 module is initialized */
    IR155_MEASUREMENT_s measurement;       /*!< latest measurement */
    uint32_t timeUntilValidMeasurement_ms; /*!< time until valid measurement result is valid */
    uint16_t periodTriggerTime_ms;         /*!< periodic call time of IR155 module */
} IR155_STATE_s;

/*========== Extern Constant and Variable Declarations ======================*/
extern IR155_STATE_s ir155_state;

/*========== Extern Function Prototypes =====================================*/
/**
 * @brief Software initialization of Timer-module
 * @param triggerTime_ms cyclic call time of IR155_GetMeasurementValues function
 */
extern void IR155_Initialize(uint8_t triggerTime_ms);

/**
 * @brief Software deinitialization of Timer-module
 */
extern void IR155_Uninitialize(void);

/**
 * @brief Interface function which delivers the actual signal measurement (duty cycle) and evaluation.
 *        Use of intervals because of measuring and signal inaccuracy. The evaluated results are
 *        finally written in the database.
 * @return #IR155_MEASUREMENT_s measurement result of IR155 measurement
 */
extern IR155_MEASUREMENT_s IR155_GetMeasurementValues(void);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
#endif

#endif /* FOXBMS__BENDER_IR155_HELPER_H_ */
