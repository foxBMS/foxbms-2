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
 * @file    murata_ncu15xh103f6sxx.c
 * @author  foxBMS Team
 * @date    2022-10-12 (date of creation)
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
 * @ingroup TEMPERATURE_SENSORS
 * @prefix  TS
 *
 * @brief   Resistive divider used for measuring temperature
 *
 */

/*========== Includes =======================================================*/
#include "murata_ncu15xh103f6sxx.h"

#include "fassert.h"
#include "foxmath.h"
#include "temperature_sensor_defs.h"

#include <math.h>
#include <stdbool.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/** LUT filled from higher resistance to lower resistance - datasheet: RO3E - Aug. 3, 2018 */
static const TS_TEMPERATURE_SENSOR_LUT_s ts_ncu15xh103f6sxxLut[] = {
    {-400, 195652.0f}, {-350, 148171.0f}, {-300, 113347.0f}, {-250, 87559.0f}, {-200, 68237.0f}, {-150, 53650.0f},
    {-100, 42506.0f},  {-50, 33892.0f},   {0, 27219.0f},     {50, 22021.0f},   {100, 17926.0f},  {150, 14674.0f},
    {200, 12081.0f},   {250, 10000.0f},   {300, 8315.0f},    {350, 6948.0f},   {400, 5834.0f},   {450, 4917.0f},
    {500, 4161.0f},    {550, 3535.0f},    {600, 3014.0f},    {650, 2586.0f},   {700, 2228.0f},   {750, 1925.0f},
    {800, 1669.0f},    {850, 1452.0f},    {900, 1268.0f},    {950, 1110.0f},   {1000, 974.0f},   {1050, 858.0f},
    {1100, 758.0f},    {1150, 672.0f},    {1200, 596.0f},    {1250, 531.0f},   {1300, 474.0f},   {1350, 424.0f},
    {1400, 381.0f},    {1450, 342.0f},    {1500, 309.0f},
};

/** size of the #ts_ntcle317e4103sbaLut LUT */
static const uint16_t ts_ncu15xh103f6sxxLutSize = sizeof(ts_ncu15xh103f6sxxLut) / sizeof(TS_TEMPERATURE_SENSOR_LUT_s);

/*========== Extern Constant and Variable Definitions =======================*/
/**
 * Defines for calculating the ADC voltage on the ends of the operating range.
 * The ADC voltage is calculated with the following formula:
 *
 * V_adc = ( ( V_supply * R_ntc ) / ( R + R_ntc ) )
 *
 * Depending on the position of the NTC in the voltage resistor (R_1/R_2),
 * different R_ntc values are used for the calculation.
 * @{
 */
#if defined(TS_MURATA_NCU15XH103F6SXX_POSITION_IN_RESISTOR_DIVIDER_IS_R_1) && \
    (TS_MURATA_NCU15XH103F6SXX_POSITION_IN_RESISTOR_DIVIDER_IS_R_1 == true)
#define TS_MURATA_NCU15XH103F6SXX_ADC_VOLTAGE_V_MAX_V \
    ((float_t)((TS_MURATA_NCU15XH103F6SXX_RESISTOR_DIVIDER_SUPPLY_VOLTAGE_V * ts_ncu15xh103f6sxxLut[ts_ncu15xh103f6sxxLutSize-1u].resistance_Ohm) / (ts_ncu15xh103f6sxxLut[ts_ncu15xh103f6sxxLutSize-1u].resistance_Ohm+TS_MURATA_NCU15XH103F6SXX_RESISTOR_DIVIDER_RESISTANCE_R_1_R_2_Ohm)))
#define TS_MURATA_NCU15XH103F6SXX_ADC_VOLTAGE_V_MIN_V \
    ((float_t)((TS_MURATA_NCU15XH103F6SXX_RESISTOR_DIVIDER_SUPPLY_VOLTAGE_V * ts_ncu15xh103f6sxxLut[0u].resistance_Ohm) / (ts_ncu15xh103f6sxxLut[0u].resistance_Ohm+TS_MURATA_NCU15XH103F6SXX_RESISTOR_DIVIDER_RESISTANCE_R_1_R_2_Ohm)))
#else /* TS_MURATA_NCU15XH103F6SXX_POSITION_IN_RESISTOR_DIVIDER_IS_R_1 is false */
#define TS_MURATA_NCU15XH103F6SXX_ADC_VOLTAGE_V_MIN_V \
    ((float_t)((TS_MURATA_NCU15XH103F6SXX_RESISTOR_DIVIDER_SUPPLY_VOLTAGE_V * ts_ncu15xh103f6sxxLut[ts_ncu15xh103f6sxxLutSize-1u].resistance_Ohm) / (ts_ncu15xh103f6sxxLut[ts_ncu15xh103f6sxxLutSize-1u].resistance_Ohm+TS_MURATA_NCU15XH103F6SXX_RESISTOR_DIVIDER_RESISTANCE_R_1_R_2_Ohm)))
#define TS_MURATA_NCU15XH103F6SXX_ADC_VOLTAGE_V_MAX_V \
    ((float_t)((TS_MURATA_NCU15XH103F6SXX_RESISTOR_DIVIDER_SUPPLY_VOLTAGE_V * ts_ncu15xh103f6sxxLut[0u].resistance_Ohm) / (ts_ncu15xh103f6sxxLut[0u].resistance_Ohm+TS_MURATA_NCU15XH103F6SXX_RESISTOR_DIVIDER_RESISTANCE_R_1_R_2_Ohm)))
#endif
/**@}*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/

extern int16_t TS_Mur00GetTemperatureFromLut(uint16_t adcVoltage_mV) {
    int16_t temperature_ddegC = INT16_MIN;
    float_t adcVoltage_V      = (float_t)adcVoltage_mV / TS_SCALING_FACTOR_1V_IN_MV_FLOAT; /* Convert mV to V */

    /* Check for valid ADC measurements to prevent undefined behavior */
    if (adcVoltage_V > TS_MURATA_NCU15XH103F6SXX_ADC_VOLTAGE_V_MAX_V) {
        /* Invalid measured ADC voltage -> sensor out of operating range or disconnected/shorted */
        temperature_ddegC = INT16_MIN;
    } else if (adcVoltage_V < TS_MURATA_NCU15XH103F6SXX_ADC_VOLTAGE_V_MIN_V) {
        /* Invalid measured ADC voltage -> sensor out of operating range or shorted/disconnected */
        temperature_ddegC = INT16_MAX;
    } else {
        /* Calculate NTC resistance based on measured ADC voltage */
#if defined(TS_MURATA_NCU15XH103F6SXX_POSITION_IN_RESISTOR_DIVIDER_IS_R_1) && \
    (TS_MURATA_NCU15XH103F6SXX_POSITION_IN_RESISTOR_DIVIDER_IS_R_1 == true)
        /* R_1 = R_2 * ( ( V_supply / V_adc ) - 1 ) */
        const float_t resistance_Ohm =
            TS_MURATA_NCU15XH103F6SXX_RESISTOR_DIVIDER_RESISTANCE_R_1_R_2_Ohm *
            ((TS_MURATA_NCU15XH103F6SXX_RESISTOR_DIVIDER_SUPPLY_VOLTAGE_V / adcVoltage_V) - 1);
#else  /* TS_MURATA_NCU15XH103F6SXX_POSITION_IN_RESISTOR_DIVIDER_IS_R_1 is false */
        /* formula: R_2 = R_1 * ( V_2 / ( V_supply - V_adc ) ) */
        const float_t resistance_Ohm =
            TS_MURATA_NCU15XH103F6SXX_RESISTOR_DIVIDER_RESISTANCE_R_1_R_2_Ohm *
            (adcVoltage_V / (TS_MURATA_NCU15XH103F6SXX_RESISTOR_DIVIDER_SUPPLY_VOLTAGE_V - adcVoltage_V));
#endif /* TS_MURATA_NCU15XH103F6SXX_POSITION_IN_RESISTOR_DIVIDER_IS_R_1 */

        /* Variables for interpolating LUT value */
        uint16_t between_high = 0;
        uint16_t between_low  = 0;
        for (uint16_t i = 0u; i < ts_ncu15xh103f6sxxLutSize; i++) {
            if (resistance_Ohm < ts_ncu15xh103f6sxxLut[i].resistance_Ohm) {
                between_low  = i + 1u;
                between_high = i;
            }
        }

        /* Interpolate between LUT values, but do not extrapolate LUT! */
        if (!(((between_high == 0u) && (between_low == 0u)) || /* measured resistance > maximum LUT resistance */
              (between_low >= ts_ncu15xh103f6sxxLutSize))) {   /* measured resistance < minimum LUT resistance */
            temperature_ddegC = (int16_t)MATH_LinearInterpolation(
                ts_ncu15xh103f6sxxLut[between_low].resistance_Ohm,
                (float_t)ts_ncu15xh103f6sxxLut[between_low].temperature_ddegC,
                ts_ncu15xh103f6sxxLut[between_high].resistance_Ohm,
                (float_t)ts_ncu15xh103f6sxxLut[between_high].temperature_ddegC,
                resistance_Ohm);
        }
    }

    /* Return temperature based on measured NTC resistance */
    return temperature_ddegC;
}

extern int16_t TS_Mur00GetTemperatureFromPolynomial(uint16_t adcVoltage_mV) {
    (void)adcVoltage_mV;
    FAS_ASSERT(FAS_TRAP);
    int16_t temperature_ddegC = 0;
    /* TODO this is not implemented */
    return temperature_ddegC;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
