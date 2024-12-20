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
 * @file    semitec_103jt.c
 * @author  foxBMS Team
 * @date    2018-10-30 (date of creation)
 * @updated 2024-12-20 (date of last update)
 * @version v1.8.0
 * @ingroup DRIVERS
 * @prefix  TS
 *
 * @brief   Resistive divider used for measuring temperature
 * @details TODO
 *
 */

/*========== Includes =======================================================*/
#include "semitec_103jt.h"

#include "fassert.h"
#include "foxmath.h"
#include "temperature_sensor_defs.h"

#include <math.h>
#include <stdbool.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/* clang-format off */
/** LUT filled from higher resistance to lower resistance */
static const TS_TEMPERATURE_SENSOR_LUT_s ts_103jtLut[] = {
    { -500, 367700.00f },
    { -400, 204700.00f },
    { -300, 118500.00f },
    { -200,  71020.00f },
    { -100,  43670.00f },
    {    0,  27700.00f },
    {  100,  18070.00f },
    {  200,  12110.00f },
    {  250,  10000.00f },
    {  300,   8301.00f },
    {  400,   5811.00f },
    {  500,   4147.00f },
    {  600,   3011.00f },
    {  700,   2224.00f },
    {  800,   1668.00f },
    {  900,   1267.00f }
};
/* clang-format on */

/** size of the #ts_103jtLut LUT */
static uint16_t ts_103jtLutSize = sizeof(ts_103jtLut) / sizeof(TS_TEMPERATURE_SENSOR_LUT_s);

/*========== Extern Constant and Variable Definitions =======================*/
/**
 * @brief   Defines for calculating the ADC voltage on the ends of the operating range.
 * @details The ADC voltage is calculated with the following formula:
 *
 *          V_adc = ((V_supply * R_ntc) / (R + R_ntc))
 *
 *          Depending on the position of the NTC in the voltage resistor (R1/R2),
 *          different R_ntc values are used for the calculation.
 */
/**@{*/
#if defined(TS_SEMITEC_103JT_POSITION_IN_RESISTOR_DIVIDER_IS_R_1) && \
    (TS_SEMITEC_103JT_POSITION_IN_RESISTOR_DIVIDER_IS_R_1 == true)
#define TS_SEMITEC_103JT_ADC_VOLTAGE_V_MAX_V                         \
    ((float_t)((TS_SEMITEC_103JT_RESISTOR_DIVIDER_SUPPLY_VOLTAGE_V * \
                ts_103jtLut[ts_103jtLutSize - 1].resistance_Ohm) /   \
               (ts_103jtLut[ts_103jtLutSize - 1].resistance_Ohm +    \
                TS_SEMITEC_103JT_RESISTOR_DIVIDER_RESISTANCE_R_1_R_2_Ohm)))
#define TS_SEMITEC_103JT_ADC_VOLTAGE_V_MIN_V                                                          \
    ((float_t)((TS_SEMITEC_103JT_RESISTOR_DIVIDER_SUPPLY_VOLTAGE_V * ts_103jtLut[0].resistance_Ohm) / \
               (ts_103jtLut[0].resistance_Ohm + TS_SEMITEC_103JT_RESISTOR_DIVIDER_RESISTANCE_R_1_R_2_Ohm)))
#else /* TS_SEMITEC_103JT_POSITION_IN_RESISTOR_DIVIDER_IS_R_1 == false */
#define TS_SEMITEC_103JT_ADC_VOLTAGE_V_MIN_V                         \
    ((float_t)((TS_SEMITEC_103JT_RESISTOR_DIVIDER_SUPPLY_VOLTAGE_V * \
                ts_103jtLut[ts_103jtLutSize - 1].resistance_Ohm) /   \
               (ts_103jtLut[ts_103jtLutSize - 1].resistance_Ohm +    \
                TS_SEMITEC_103JT_RESISTOR_DIVIDER_RESISTANCE_R_1_R_2_Ohm)))
#define TS_SEMITEC_103JT_ADC_VOLTAGE_V_MAX_V                                                          \
    ((float_t)((TS_SEMITEC_103JT_RESISTOR_DIVIDER_SUPPLY_VOLTAGE_V * ts_103jtLut[0].resistance_Ohm) / \
               (ts_103jtLut[0].resistance_Ohm + TS_SEMITEC_103JT_RESISTOR_DIVIDER_RESISTANCE_R_1_R_2_Ohm)))
#endif
/**@}*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/

extern int16_t TS_Sem00GetTemperatureFromLut(uint16_t adcVoltage_mV) {
    /* AXIVION Routine Generic-MissingParameterAssert: adcVoltage_mV: parameter accepts whole range */

    int16_t temperature_ddegC = 0;
    float_t resistance_Ohm    = 0.0f;
    float_t adcVoltage_V      = adcVoltage_mV / 1000.0f; /* Convert mV to V */

    /* Check for valid ADC measurements to prevent undefined behavior */
    if (adcVoltage_V > TS_SEMITEC_103JT_ADC_VOLTAGE_V_MAX_V) {
        /* Invalid measured ADC voltage -> sensor out of operating range or disconnected/shorted */
        temperature_ddegC = INT16_MIN;
    } else if (adcVoltage_V < TS_SEMITEC_103JT_ADC_VOLTAGE_V_MIN_V) {
        /* Invalid measured ADC voltage -> sensor out of operating range or shorted/disconnected */
        temperature_ddegC = INT16_MAX;
    } else {
        /* Calculate NTC resistance based on measured ADC voltage */
#if defined(TS_SEMITEC_103JT_POSITION_IN_RESISTOR_DIVIDER_IS_R_1) && \
    (TS_SEMITEC_103JT_POSITION_IN_RESISTOR_DIVIDER_IS_R_1 == true)
        /* R_1 = R_2 * ( ( V_supply / V_adc ) - 1 ) */
        resistance_Ohm = TS_SEMITEC_103JT_RESISTOR_DIVIDER_RESISTANCE_R_1_R_2_Ohm *
                         ((TS_SEMITEC_103JT_RESISTOR_DIVIDER_SUPPLY_VOLTAGE_V / adcVoltage_V) - 1);
#else  /* TS_SEMITEC_103JT_POSITION_IN_RESISTOR_DIVIDER_IS_R_1 == false */
        /* R_2 = R_1 * ( V_2 / ( V_supply - V_adc ) ) */
        resistance_Ohm = TS_SEMITEC_103JT_RESISTOR_DIVIDER_RESISTANCE_R_1_R_2_Ohm *
                         (adcVoltage_V / (TS_SEMITEC_103JT_RESISTOR_DIVIDER_SUPPLY_VOLTAGE_V - adcVoltage_V));
#endif /* TS_SEMITEC_103JT_POSITION_IN_RESISTOR_DIVIDER_IS_R_1 */

        /* Variables for interpolating LUT value */
        uint16_t between_high = 0;
        uint16_t between_low  = 0;
        for (uint16_t i = 1; i < ts_103jtLutSize; i++) {
            if (resistance_Ohm < ts_103jtLut[i].resistance_Ohm) {
                between_low  = i + 1u;
                between_high = i;
            }
        }

        /* Interpolate between LUT values, but do not extrapolate LUT! */
        if (!(((between_high == 0u) && (between_low == 0u)) || /* measured resistance > maximum LUT resistance */
              (between_low > ts_103jtLutSize))) {              /* measured resistance < minimum LUT resistance */
            temperature_ddegC = (int16_t)MATH_LinearInterpolation(
                ts_103jtLut[between_low].resistance_Ohm,
                ts_103jtLut[between_low].temperature_ddegC,
                ts_103jtLut[between_high].resistance_Ohm,
                ts_103jtLut[between_high].temperature_ddegC,
                resistance_Ohm);
        }
    }

    /* Return temperature based on measured NTC resistance */
    return temperature_ddegC;
}

extern int16_t TS_Sem00GetTemperatureFromPolynomial(uint16_t adcVoltage_mV) {
    (void)adcVoltage_mV;
    FAS_ASSERT(FAS_TRAP);
    int16_t temperature_ddegC = 0;
    /* TODO this is not implemented */
    return temperature_ddegC;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
