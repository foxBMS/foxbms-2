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
 * @file    bender_ir155_helper.c
 * @author  foxBMS Team
 * @date    2021-09-17 (date of creation)
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
 * @ingroup DRIVERS
 * @prefix  IR155
 *
 * @brief   Driver for the insulation monitoring
 */

/*========== Includes =======================================================*/
#include "bender_ir155_helper.h"

#include "bender_ir155_cfg.h"

#include "fram.h"

#include <math.h>
#include <stdbool.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/
/** Maximum measurable resistance according to formula:
 *    ((90 * 1200kOhm) / (dc - 5) ) - 1200kOhm
 * with dc ]5%,95%[ */
#define IR155_MAXIMUM_INSULATION_RESISTANCE_kOhm (106800u)
#define IR155_MINIMUM_INSULATION_RESISTANCE_kOhm (0u)

/* Normal mode: duty cycle limits */
#define IR155_NORMAL_MODE_LOWER_DUTY_CYCLE_LIMIT_perc (4.0f)
#define IR155_NORMAL_MODE_UPPER_DUTY_CYCLE_LIMIT_perc (96.0f)

/* Speed start mode: duty cycle limits */
#define IR155_SPEED_START_ESTIMATION_GOOD_LOWER_DUTY_CYCLE_LIMIT_perc (4.0f)
#define IR155_SPEED_START_ESTIMATION_GOOD_UPPER_DUTY_CYCLE_LIMIT_perc (11.0f)

#define IR155_SPEED_START_ESTIMATION_BAD_LOWER_DUTY_CYCLE_LIMIT_perc (89.0f)
#define IR155_SPEED_START_ESTIMATION_BAD_UPPER_DUTY_CYCLE_LIMIT_perc (96.0f)

/* Undervoltage mode: duty cycle limits */
#define IR155_UNDERVOLTAGE_MODE_LOWER_DUTY_CYCLE_LIMIT_perc (4.0f)
#define IR155_UNDERVOLTAGE_MODE_UPPER_DUTY_CYCLE_LIMIT_perc (96.0f)

/* Device error mode: duty cycle limits */
#define IR155_DEVICE_ERROR_LOWER_DUTY_CYCLE_LIMIT_perc (46.5f)
#define IR155_DEVICE_ERROR_UPPER_DUTY_CYCLE_LIMIT_perc (53.5f)

/* Device error mode: duty cycle limits */
#define IR155_GROUND_ERROR_LOWER_DUTY_CYCLE_LIMIT_perc (46.5f)
#define IR155_GROUND_ERROR_UPPER_DUTY_CYCLE_LIMIT_perc (53.5f)

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/
extern IR155_STATE_s ir155_state = {
    .ir155Initialized                     = false,
    .measurement.isMeasurementValid       = false,
    .measurement.isUndervoltageDetected   = false,
    .measurement.measurementState         = IR155_UNINITIALIZED,
    .measurement.measurementMode          = IR155_UNKNOWN,
    .measurement.digitalStatusPin         = STD_PIN_LOW,
    .measurement.resistance_kOhm          = 0,
    .measurement.pwmSignal.dutyCycle_perc = 0.0f,
    .measurement.pwmSignal.frequency_Hz   = 0.0f,
    .periodTriggerTime_ms                 = IMD_PERIODIC_CALL_TIME_ms,
};

/*========== Static Function Prototypes =====================================*/
/**
 * @brief   Determines frequency-dependent measurement state.
 * @details Use of intervals because of measuring and signal inaccuracy. This
 *          interval is configurable via define #IR155_MEASUREMENT_INTERVAL_RANGE_Hz
 * @param[in] frequency_Hz   measured signal frequency in Hz
 * @return  #IR155_MEASUREMENT_MODE_e bender measurement mode
 */
static IR155_MEASUREMENT_MODE_e IR155_GetMeasurementMode(float_t frequency_Hz);

/**
 * @brief   Calculate insulation resistance from measured dutycycle.
 * @details Function check, that passed duty-cycle lies within allowed range.
 *          Otherwise, the calculated resistance will be limited to the next
 *          reasonable value.
 * @param[in] dutyCycle_perc   measured signal duty-cycle in percentage
 * @return  measured insulation resistance in kOhm
 */
static uint32_t IR155_CalculateResistance(float_t dutyCycle_perc);

/**
 * @brief   Check if passed duty cycle is within interval limits
 * @param[in] dutyCycle_perc   measured signal duty-cycle in percentage
 * @param[in] lowerLimit_perc  lower interval limit
 * @param[in] upperLimit_perc  upper interval limit
 * @return  measured insulation resistance in kOhm
 */
static bool IR155_IsDutyCycleWithinInterval(float_t dutyCycle_perc, float_t lowerLimit_perc, float_t upperLimit_perc);

/*========== Static Function Implementations ================================*/
static IR155_MEASUREMENT_MODE_e IR155_GetMeasurementMode(float_t frequency_Hz) {
    FAS_ASSERT(frequency_Hz >= 0.0f);
    IR155_MEASUREMENT_MODE_e retVal = IR155_UNKNOWN;

    if ((frequency_Hz >= IR155_NORMAL_CONDITION_LOWER_FREQUENCY_Hz) &&
        (frequency_Hz < IR155_NORMAL_CONDITION_UPPER_FREQUENCY_Hz)) {
        retVal = IR155_NORMAL_MODE;
    } else if (
        (frequency_Hz >= IR155_UNDERVOLTAGE_LOWER_FREQUENCY_Hz) &&
        (frequency_Hz < IR155_UNDERVOLTAGE_UPPER_FREQUENCY_Hz)) {
        retVal = IR155_UNDERVOLTAGE_MODE; /* should not be detected as default threshold 0V, EOL Bender configurable! */
    } else if (
        (frequency_Hz >= IR155_SPEEDSTART_LOWER_FREQUENCY_Hz) && (frequency_Hz < IR155_SPEEDSTART_UPPER_FREQUENCY_Hz)) {
        retVal = IR155_SPEEDSTART_MODE;
    } else if (
        (frequency_Hz >= IR155_IMD_DEVICE_ERROR_LOWER_FREQUENCY_Hz) &&
        (frequency_Hz < IR155_IMD_DEVICE_ERROR_UPPER_FREQUENCY_Hz)) {
        retVal = IR155_IMD_ERROR_MODE;
    } else if (
        (frequency_Hz >= IR155_GROUND_ERROR_LOWER_FREQUENCY_Hz) &&
        (frequency_Hz < IR155_GROUND_ERROR_UPPER_FREQUENCY_Hz)) {
        retVal = IR155_GROUNDERROR_MODE;
    } else if (frequency_Hz <= IR155_MINIMUM_FREQUENCY_Hz) {
        retVal = IR155_SHORT_CLAMP;
    } else {
        retVal = IR155_UNDEFINED_FREQUENCY;
    }
    return retVal;
}

static uint32_t IR155_CalculateResistance(float_t dutyCycle_perc) {
    FAS_ASSERT(dutyCycle_perc <= 100.0f);
    FAS_ASSERT(dutyCycle_perc > 0.0f);

    float_t insulationResistance_kOhm = 0.0f;
    if (dutyCycle_perc <= 5.0f) {
        insulationResistance_kOhm = (float_t)IR155_MAXIMUM_INSULATION_RESISTANCE_kOhm;
    } else if (dutyCycle_perc > 95.0f) {
        insulationResistance_kOhm = (float_t)IR155_MINIMUM_INSULATION_RESISTANCE_kOhm;
    } else {
        insulationResistance_kOhm = ((90.0f * 1200.0f) / (dutyCycle_perc - 5.0f)) - 1200.0f;
    }
    return (uint32_t)insulationResistance_kOhm;
}

static bool IR155_IsDutyCycleWithinInterval(float_t dutyCycle_perc, float_t lowerLimit_perc, float_t upperLimit_perc) {
    bool retval = false;
    if ((lowerLimit_perc < dutyCycle_perc) && (upperLimit_perc > dutyCycle_perc)) {
        retval = true;
    } else {
        retval = false;
    }
    return retval;
}

/*========== Extern Function Implementations ================================*/

void IR155_Initialize(uint8_t triggerTime_ms) {
    /* Timer peripheral initialization if not already done. */
    if (false == PWM_IsEcapModuleInitialized()) {
        PWM_Initialize(); /* TODO: split PWM_Init function in a dedicated function for ecap and etpwm module */
    }

    /* Read non-volatile FRAM */
    FRAM_ReadData(FRAM_BLOCK_ID_INSULATION_FLAG);

    /* Check grounderror flag */
    if (fram_insulationFlags.groundErrorDetected == true) {
        /* GROUND ERROR occurred before shutting down */
        ir155_state.timeUntilValidMeasurement_ms = IR155_WAIT_TIME_AFTER_GROUND_ERROR_ms;
    } else {
        /* Normal startup delay -> wait 2.2s until first measurement is trustworthy */
        ir155_state.timeUntilValidMeasurement_ms = IR155_STARTUP_TIME_ms;
    }

    ir155_state.periodTriggerTime_ms = triggerTime_ms;
    ir155_state.ir155Initialized     = true;
}

void IR155_Deinitialize(void) {
    /* Reset cycle time */
    ir155_state.periodTriggerTime_ms = 0u;

    /* Reset timer duty cycle struct */
    ir155_state.measurement.pwmSignal.dutyCycle_perc = 0.0f;
    ir155_state.measurement.pwmSignal.frequency_Hz   = 0.0f;

    ir155_state.ir155Initialized = false;

    /* Set diagnosis message that measurement is not trustworthy */
    /* TODO: do it really like this? DIAG_Handler(DIAG_CH_ISOMETER_MEAS_INVALID, DIAG_EVENT_NOK, 0); */
}

IR155_MEASUREMENT_s IR155_GetMeasurementValues(void) {
    /* Initialize struct */
    IR155_MEASUREMENT_s measurementResult = {
        .isMeasurementValid       = false,
        .isUndervoltageDetected   = true,
        .pwmSignal.dutyCycle_perc = 0.0f,
        .pwmSignal.frequency_Hz   = 0.0f,
        .resistance_kOhm          = 0u,
        .digitalStatusPin         = STD_PIN_UNDEFINED,
        .measurementMode          = IR155_UNKNOWN,
        .measurementState         = IR155_UNINITIALIZED,
    };

    /* read value of Bender IR155 digital status pin (OHKS) */
    measurementResult.digitalStatusPin = IR155_GET_DIGITAL_STATUS_PIN_STATE();

    /* get duty-cycle and frequency from PWM input measurement */
    measurementResult.pwmSignal = PWM_GetPwmData();

    /* TODO: How-to check valid data? */
    bool isPwmMeasurementValid = true;
    if (isPwmMeasurementValid == true) {
        /* TODO: Throw error? */
        isPwmMeasurementValid = true;
    } else {
        /* Invalid values measurement */

        /* Invalidate data? */
        isPwmMeasurementValid = false;
    }

    /* Get measurement mode */
    if (isPwmMeasurementValid == true) {
        measurementResult.measurementMode = IR155_GetMeasurementMode(measurementResult.pwmSignal.frequency_Hz);
    } else {
        measurementResult.measurementMode = IR155_UNKNOWN;
        /* TODO: Throw what error? */
    }

    switch (measurementResult.measurementMode) {
        case IR155_NORMAL_MODE:
            ir155_state.measurement.isUndervoltageDetected = false;
            if (true == IR155_IsDutyCycleWithinInterval(
                            measurementResult.pwmSignal.dutyCycle_perc,
                            IR155_NORMAL_MODE_LOWER_DUTY_CYCLE_LIMIT_perc,
                            IR155_NORMAL_MODE_UPPER_DUTY_CYCLE_LIMIT_perc)) {
                measurementResult.resistance_kOhm =
                    IR155_CalculateResistance(measurementResult.pwmSignal.dutyCycle_perc);
                measurementResult.isMeasurementValid = true;
                measurementResult.measurementState   = IR155_RESISTANCE_MEASUREMENT;
            } else {
                measurementResult.measurementState   = IR155_RESISTANCE_MEASUREMENT_UNKNOWN;
                measurementResult.isMeasurementValid = false;
                measurementResult.resistance_kOhm    = IR155_MINIMUM_INSULATION_RESISTANCE_kOhm;
            }
            break;

        case IR155_SPEEDSTART_MODE:
            measurementResult.isUndervoltageDetected = false;
            if (true == IR155_IsDutyCycleWithinInterval(
                            measurementResult.pwmSignal.dutyCycle_perc,
                            IR155_SPEED_START_ESTIMATION_GOOD_LOWER_DUTY_CYCLE_LIMIT_perc,
                            IR155_SPEED_START_ESTIMATION_GOOD_UPPER_DUTY_CYCLE_LIMIT_perc)) {
                measurementResult.resistance_kOhm    = IR155_MAXIMUM_INSULATION_RESISTANCE_kOhm;
                measurementResult.isMeasurementValid = true;
                measurementResult.measurementState   = IR155_RESISTANCE_ESTIMATION;
            } else if (
                true == IR155_IsDutyCycleWithinInterval(
                            measurementResult.pwmSignal.dutyCycle_perc,
                            IR155_SPEED_START_ESTIMATION_BAD_LOWER_DUTY_CYCLE_LIMIT_perc,
                            IR155_SPEED_START_ESTIMATION_BAD_UPPER_DUTY_CYCLE_LIMIT_perc)) {
                measurementResult.resistance_kOhm    = IR155_MINIMUM_INSULATION_RESISTANCE_kOhm;
                measurementResult.isMeasurementValid = true;
                measurementResult.measurementState   = IR155_RESISTANCE_ESTIMATION;
            } else {
                measurementResult.resistance_kOhm    = IR155_MINIMUM_INSULATION_RESISTANCE_kOhm;
                measurementResult.isMeasurementValid = false;
                measurementResult.measurementState   = IR155_RESISTANCE_ESTIMATION_UNKNOWN;
            }
            break;

        case IR155_UNDERVOLTAGE_MODE:
            measurementResult.isUndervoltageDetected = true;
            if (true == IR155_IsDutyCycleWithinInterval(
                            measurementResult.pwmSignal.dutyCycle_perc,
                            IR155_UNDERVOLTAGE_MODE_LOWER_DUTY_CYCLE_LIMIT_perc,
                            IR155_UNDERVOLTAGE_MODE_UPPER_DUTY_CYCLE_LIMIT_perc)) {
                measurementResult.resistance_kOhm =
                    IR155_CalculateResistance(measurementResult.pwmSignal.dutyCycle_perc);
                measurementResult.isMeasurementValid = true;
                measurementResult.measurementState   = IR155_UNDERVOLTAGE_MEASUREMENT;
            } else {
                measurementResult.isMeasurementValid = false;
                measurementResult.resistance_kOhm    = IR155_MINIMUM_INSULATION_RESISTANCE_kOhm;
                measurementResult.measurementState   = IR155_UNDERVOLTAGE_MEASUREMENT_UNKNOWN;
            }
            break;

        case IR155_IMD_ERROR_MODE:
            measurementResult.isUndervoltageDetected = false;
            measurementResult.resistance_kOhm        = IR155_MINIMUM_INSULATION_RESISTANCE_kOhm;
            if (true == IR155_IsDutyCycleWithinInterval(
                            measurementResult.pwmSignal.dutyCycle_perc,
                            IR155_DEVICE_ERROR_LOWER_DUTY_CYCLE_LIMIT_perc,
                            IR155_DEVICE_ERROR_UPPER_DUTY_CYCLE_LIMIT_perc)) {
                /* Error detected and verified with duty cycle */
                measurementResult.isMeasurementValid = true;
                measurementResult.measurementState   = IR155_IMD_ERROR_MEASUREMENT;
            } else {
                /* Error detected but invalid duty cycle */
                measurementResult.measurementState   = IR155_IMD_ERROR_MEASUREMENT_UNKNOWN;
                measurementResult.isMeasurementValid = false;
            }
            break;
        case IR155_GROUNDERROR_MODE:
            measurementResult.isUndervoltageDetected = false;
            measurementResult.resistance_kOhm        = IR155_MINIMUM_INSULATION_RESISTANCE_kOhm;
            if (true == IR155_IsDutyCycleWithinInterval(
                            measurementResult.pwmSignal.dutyCycle_perc,
                            IR155_GROUND_ERROR_LOWER_DUTY_CYCLE_LIMIT_perc,
                            IR155_GROUND_ERROR_UPPER_DUTY_CYCLE_LIMIT_perc)) {
                /* Error detected and verified with duty cycle */
                measurementResult.isMeasurementValid = true;
                measurementResult.measurementState   = IR155_GROUND_ERROR_MODE;
            } else {
                /* Error detected but invalid duty cycle */
                measurementResult.measurementState   = IR155_GROUND_ERROR_MODE_UNKNOWN;
                measurementResult.isMeasurementValid = false;
            }
            break;

        case IR155_SHORT_CLAMP:
            measurementResult.isUndervoltageDetected = false;
            measurementResult.resistance_kOhm        = IR155_MINIMUM_INSULATION_RESISTANCE_kOhm;
            measurementResult.measurementState       = IR155_SIGNAL_SHORT;
            measurementResult.isMeasurementValid     = true;
            break;

        default:
            measurementResult.resistance_kOhm    = IR155_MINIMUM_INSULATION_RESISTANCE_kOhm;
            measurementResult.measurementState   = IR155_MEASUREMENT_NOT_VALID;
            measurementResult.isMeasurementValid = false;
            break;
    }

    return measurementResult;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
