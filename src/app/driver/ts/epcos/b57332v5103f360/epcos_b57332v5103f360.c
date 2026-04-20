/**
 *
 * @copyright &copy; 2010 - 2026, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * @file    epcos_b57332v5103f360.c
 * @author  foxBMS Team
 * @date    2018-10-30 (date of creation)
 * @updated 2026-04-20 (date of last update)
 * @version v1.11.0
 * @ingroup DRIVERS
 * @prefix  TS
 *
 * @brief   Resistive divider used for measuring temperature
 * @details TODO
 */

/*========== Includes =======================================================*/
#include "epcos_b57332v5103f360.h"

#include "foxmath.h"
#include "temperature_sensor_defs.h"

#include <math.h>
#include <stdbool.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/* clang-format off */
/** LUT filled from higher resistance to lower resistance */
static const TS_TEMPERATURE_SENSOR_LUT_s ts_B57332V5103F360Lut[] = {
    { -400, 190030.00f },
    { -350, 145360.00f },
    { -300,112060.00f },
    { -250,87041.00f  },
    { -200,68104.00f  },
    { -150,53665.00f  },
    { -100,42576.00f },
    { -50,34001.00f  },
    {  00,27326.00f  },
    {  50,22096.00f  },
    {  100,17973.00f  },
    {  150,14703.00f },
    {  200,12093.00f  },
    {  250,10000.00f  },
    {  300,8311.30f  },
    {  350,6941.80f  },
    {  400,5825.50f  },
    {  450,4911.20f  },
    {  500,4158.70f  },
    {  550,3536.50f },
    {  600,3019.70f },
    {  650,2588.80f  },
    {  700,2227.80f },
    {  750,1924.30f  },
    {  800,1668.10f  },
    {  850,1451.00f },
    {  900,1266.30f},
    {  950,1108.80f  },
    { 1000,973.81f  },
    { 1050,857.88f  },
    { 1100,757.95f  },
    { 1150,671.55f },
    { 1200,596.63f },
    { 1250,531.46f  },
    { 1300,474.63f },
    { 1350,424.93f },
    { 1400,381.34f  },
    { 1450,343.02f },
    { 1500,309.25f  }
};
/* clang-format on */

/** size of the #ts_B57332V5103F360Lut LUT */
static uint16_t B57332V5103F360LutSize = sizeof(ts_B57332V5103F360Lut) / sizeof(TS_TEMPERATURE_SENSOR_LUT_s);

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
#if defined(TS_EPCOS_B57332V5103F360_POSITION_IN_RESISTOR_DIVIDER_IS_R_1) && \
    (TS_EPCOS_B57332V5103F360_POSITION_IN_RESISTOR_DIVIDER_IS_R_1 == true)
#define TS_EPCOS_B57332V5103F360_ADC_VOLTAGE_V_MAX_V                                \
    ((float_t)((TS_EPCOS_B57332V5103F360_RESISTOR_DIVIDER_SUPPLY_VOLTAGE_V *        \
                ts_B57332V5103F360Lut[B57332V5103F360LutSize - 1].resistance_Ohm) / \
               (ts_B57332V5103F360Lut[B57332V5103F360LutSize - 1].resistance_Ohm +  \
                TS_EPCOS_B57332V5103F360_RESISTOR_DIVIDER_RESISTANCE_R_1_R_2_Ohm)))
#define TS_EPCOS_B57332V5103F360_ADC_VOLTAGE_V_MIN_V                         \
    ((float_t)((TS_EPCOS_B57332V5103F360_RESISTOR_DIVIDER_SUPPLY_VOLTAGE_V * \
                ts_B57332V5103F360Lut[0].resistance_Ohm) /                   \
               (ts_B57332V5103F360Lut[0].resistance_Ohm +                    \
                TS_EPCOS_B57332V5103F360_RESISTOR_DIVIDER_RESISTANCE_R_1_R_2_Ohm)))
#else /* TS_EPCOS_B57332V5103F360_POSITION_IN_RESISTOR_DIVIDER_IS_R_1 == false */
#define TS_EPCOS_B57332V5103F360_ADC_VOLTAGE_V_MIN_V                                \
    ((float_t)((TS_EPCOS_B57332V5103F360_RESISTOR_DIVIDER_SUPPLY_VOLTAGE_V *        \
                ts_B57332V5103F360Lut[B57332V5103F360LutSize - 1].resistance_Ohm) / \
               (ts_B57332V5103F360Lut[B57332V5103F360LutSize - 1].resistance_Ohm +  \
                TS_EPCOS_B57332V5103F360_RESISTOR_DIVIDER_RESISTANCE_R_1_R_2_Ohm)))
#define TS_EPCOS_B57332V5103F360_ADC_VOLTAGE_V_MAX_V                         \
    ((float_t)((TS_EPCOS_B57332V5103F360_RESISTOR_DIVIDER_SUPPLY_VOLTAGE_V * \
                ts_B57332V5103F360Lut[0].resistance_Ohm) /                   \
               (ts_B57332V5103F360Lut[0].resistance_Ohm +                    \
                TS_EPCOS_B57332V5103F360_RESISTOR_DIVIDER_RESISTANCE_R_1_R_2_Ohm)))
#endif
/**@}*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/

extern int16_t TS_Epc02GetTemperatureFromLut(uint16_t adcVoltage_mV) {
    /* AXIVION Routine Generic-MissingParameterAssert: adcVoltage_mV: parameter accepts whole range */

    int16_t temperature_ddegC = 0;
    float_t resistance_Ohm    = 0.0f;
    float_t adcVoltage_V      = adcVoltage_mV / 1000.0f; /* Convert mV to V */

    /* Check for valid ADC measurements to prevent undefined behavior */
    if (adcVoltage_V > TS_EPCOS_B57332V5103F360_ADC_VOLTAGE_V_MAX_V) {
        /* Invalid measured ADC voltage -> sensor out of operating range or disconnected/shorted */
        temperature_ddegC = INT16_MIN;
    } else if (adcVoltage_V < TS_EPCOS_B57332V5103F360_ADC_VOLTAGE_V_MIN_V) {
        /* Invalid measured ADC voltage -> sensor out of operating range or shorted/disconnected */
        temperature_ddegC = INT16_MAX;
    } else {
        /* Calculate NTC resistance based on measured ADC voltage */
#if defined(TS_EPCOS_B57332V5103F360_POSITION_IN_RESISTOR_DIVIDER_IS_R_1) && \
    (TS_EPCOS_B57332V5103F360_POSITION_IN_RESISTOR_DIVIDER_IS_R_1 == true)
        /* R_1 = R_2 * ( ( V_supply / V_adc ) - 1 ) */
        resistance_Ohm = TS_EPCOS_B57332V5103F360_RESISTOR_DIVIDER_RESISTANCE_R_1_R_2_Ohm *
                         ((TS_EPCOS_B57332V5103F360_RESISTOR_DIVIDER_SUPPLY_VOLTAGE_V / adcVoltage_V) - 1);
#else  /* TS_EPCOS_B57332V5103F360_POSITION_IN_RESISTOR_DIVIDER_IS_R_1 == false */
        /* R_2 = R_1 * ( V_2 / ( V_supply - V_adc ) ) */
        resistance_Ohm = TS_EPCOS_B57332V5103F360_RESISTOR_DIVIDER_RESISTANCE_R_1_R_2_Ohm *
                         (adcVoltage_V / (TS_EPCOS_B57332V5103F360_RESISTOR_DIVIDER_SUPPLY_VOLTAGE_V - adcVoltage_V));
#endif /* TS_EPCOS_B57332V5103F360_POSITION_IN_RESISTOR_DIVIDER_IS_R_1 */

        /* Variables for interpolating LUT value */
        uint16_t between_high = 0;
        uint16_t between_low  = 0;
        for (uint16_t i = 1; i < B57332V5103F360LutSize; i++) {
            if (resistance_Ohm < ts_B57332V5103F360Lut[i].resistance_Ohm) {
                between_low  = i + 1u;
                between_high = i;
            }
        }

        /* Interpolate between LUT values, but do not extrapolate LUT! */
        if (!(((between_high == 0u) && (between_low == 0u)) || /* measured resistance > maximum LUT resistance */
              (between_low > B57332V5103F360LutSize))) {       /* measured resistance < minimum LUT resistance */
            temperature_ddegC = (int16_t)MATH_LinearInterpolation(
                ts_B57332V5103F360Lut[between_low].resistance_Ohm,
                ts_B57332V5103F360Lut[between_low].temperature_ddegC,
                ts_B57332V5103F360Lut[between_high].resistance_Ohm,
                ts_B57332V5103F360Lut[between_high].temperature_ddegC,
                resistance_Ohm);
        }
    }

    /* Return temperature based on measured NTC resistance */
    return temperature_ddegC;
}

extern int16_t TS_Epc02GetTemperatureFromPolynomial(uint16_t adcVoltage_mV) {
    /* AXIVION Routine Generic-MissingParameterAssert: adcVoltage_mV: parameter accepts whole range */
    /* cspell:ignore vadc */

    return (int16_t)(adcVoltage_mV); /* Convert deg into deci &deg;C */
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
