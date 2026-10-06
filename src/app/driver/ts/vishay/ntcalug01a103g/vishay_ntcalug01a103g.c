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
 * @file    vishay_ntcalug01a103g.c
 * @author  foxBMS Team
 * @date    2018-10-30 (date of creation)
 * @updated 2026-10-06 (date of last update)
 * @version v1.12.0
 * @ingroup DRIVERS
 * @prefix  TS
 *
 * @brief   Resistive divider used for measuring temperature
 * @details TODO
 *
 */

/*========== Includes =======================================================*/
#include "vishay_ntcalug01a103g.h"

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
static const TS_TEMPERATURE_SENSOR_LUT_s ts_ntcalug01a103gLut[] = {
    {-550, 953773.5f},
    {-500, 664169.3f},
    {-450, 468363.2f},
    {-400, 334274.4f},
    {-350, 241322.8f},
    {-300, 176132.5f},
    {-250, 129899.9f},
    {-200, 96761.1f},
    {-150, 72764.5f},
    {-100, 55218.1f},
    {-50, 42267.8f},
    {0, 32624.2f},
    {50, 25381.3f},
    {100, 19896.9f},
    {150, 15711.2f},
    {200, 12492.7f},
    {250, 10000.0f},
    {300, 8055.9f},
    {350, 6529.7f},
    {400, 5323.8f},
    {450, 4365.2f},
    {500, 3598.7f},
    {550, 2982.2f},
    {600, 2483.8f},
    {650, 2078.6f},
    {700, 1747.6f},
    {750, 1475.9f},
    {800, 1251.8f},
    {850, 1066.1f},
    {900, 911.5f},
    {950, 782.4f},
    {1000, 674.1f},
    {1050, 582.8f},
    {1100, 505.6f},
    {1150, 440.1f},
    {1200, 384.4f},
    {1250, 336.7f},
    {1300, 295.8f},
    {1350, 260.7f},
    {1400, 230.4f},
    {1450, 204.1f},
    {1500, 181.3f}
};
/* clang-format on */

/** size of the #ts_ntcalug01a103gLut LUT */
static const uint16_t ts_ntcalug01a103gLutSize = sizeof(ts_ntcalug01a103gLut) / sizeof(TS_TEMPERATURE_SENSOR_LUT_s);

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
#if defined(TS_VISHAY_NTCALUG01A103G_POSITION_IN_RESISTOR_DIVIDER_IS_R_1) && \
    (TS_VISHAY_NTCALUG01A103G_POSITION_IN_RESISTOR_DIVIDER_IS_R_1 == true)
#define TS_VISHAY_NTCALUG01A103G_ADC_VOLTAGE_V_MAX_V                          \
    (float_t)(                                                                \
        (TS_VISHAY_NTCALUG01A103G_RESISTOR_DIVIDER_SUPPLY_VOLTAGE_V *         \
         ts_ntcalug01a103gLut[ts_ntcalug01a103gLutSize - 1].resistance_Ohm) / \
        (ts_ntcalug01a103gLut[ts_ntcalug01a103gLutSize - 1].resistance_Ohm +  \
         TS_VISHAY_NTCALUG01A103G_RESISTOR_DIVIDER_RESISTANCE_R_1_R_2_Ohm))
#define TS_VISHAY_NTCALUG01A103G_ADC_VOLTAGE_V_MIN_V                                                            \
    (float_t)(                                                                                                  \
        (TS_VISHAY_NTCALUG01A103G_RESISTOR_DIVIDER_SUPPLY_VOLTAGE_V * ts_ntcalug01a103gLut[0].resistance_Ohm) / \
        (ts_ntcalug01a103gLut[0].resistance_Ohm + TS_VISHAY_NTCALUG01A103G_RESISTOR_DIVIDER_RESISTANCE_R_1_R_2_Ohm))
#else /* TS_VISHAY_NTCALUG01A103G_POSITION_IN_RESISTOR_DIVIDER_IS_R_1 == false */
#define TS_VISHAY_NTCALUG01A103G_ADC_VOLTAGE_V_MIN_V                                 \
    ((float_t)((TS_VISHAY_NTCALUG01A103G_RESISTOR_DIVIDER_SUPPLY_VOLTAGE_V *         \
                ts_ntcalug01a103gLut[ts_ntcalug01a103gLutSize - 1].resistance_Ohm) / \
               (ts_ntcalug01a103gLut[ts_ntcalug01a103gLutSize - 1].resistance_Ohm +  \
                TS_VISHAY_NTCALUG01A103G_RESISTOR_DIVIDER_RESISTANCE_R_1_R_2_Ohm)))
#define TS_VISHAY_NTCALUG01A103G_ADC_VOLTAGE_V_MAX_V                                                                   \
    ((float_t)((TS_VISHAY_NTCALUG01A103G_RESISTOR_DIVIDER_SUPPLY_VOLTAGE_V * ts_ntcalug01a103gLut[0].resistance_Ohm) / \
               (ts_ntcalug01a103gLut[0].resistance_Ohm +                                                               \
                TS_VISHAY_NTCALUG01A103G_RESISTOR_DIVIDER_RESISTANCE_R_1_R_2_Ohm)))
#endif
/**@}*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/

extern int16_t TS_Vis00GetTemperatureFromLut(uint16_t adcVoltage_mV, float_t supplyVoltage_V) {
    int16_t temperature_ddegC = 0;
    float_t resistance_Ohm    = 0.0f;
    float_t adcVoltage_V      = adcVoltage_mV / 1000.0f; /* Convert mV to V */

    /* Check for valid ADC measurements to prevent undefined behavior */
    if (adcVoltage_V > TS_VISHAY_NTCALUG01A103G_ADC_VOLTAGE_V_MAX_V) {
        /* Invalid measured ADC voltage -> sensor out of operating range or disconnected/shorted */
        temperature_ddegC = INT16_MIN;
    } else if (adcVoltage_V < TS_VISHAY_NTCALUG01A103G_ADC_VOLTAGE_V_MIN_V) {
        /* Invalid measured ADC voltage -> sensor out of operating range or shorted/disconnected */
        temperature_ddegC = INT16_MAX;
    } else {
        /* Calculate NTC resistance based on measured ADC voltage */
#if defined(TS_VISHAY_NTCALUG01A103G_POSITION_IN_RESISTOR_DIVIDER_IS_R_1) && \
    (TS_VISHAY_NTCALUG01A103G_POSITION_IN_RESISTOR_DIVIDER_IS_R_1 == true)
        /* R_1 = R_2 * ( ( V_supply / V_adc ) - 1 ) */
        resistance_Ohm = TS_VISHAY_NTCALUG01A103G_RESISTOR_DIVIDER_RESISTANCE_R_1_R_2_Ohm *
                         ((supplyVoltage_V / adcVoltage_V) - 1);
#else  /* TS_VISHAY_NTCALUG01A103G_POSITION_IN_RESISTOR_DIVIDER_IS_R_1 == false */
        /* R_2 = R_1 * ( V_2 / ( V_supply - V_adc ) ) */
        resistance_Ohm = TS_VISHAY_NTCALUG01A103G_RESISTOR_DIVIDER_RESISTANCE_R_1_R_2_Ohm *
                         (adcVoltage_V / (supplyVoltage_V - adcVoltage_V));
#endif /* TS_VISHAY_NTCALUG01A103G_POSITION_IN_RESISTOR_DIVIDER_IS_R_1 */

        /* Variables for interpolating LUT value */
        uint16_t between_high = 0;
        uint16_t between_low  = 0;
        for (uint16_t i = 1u; i < ts_ntcalug01a103gLutSize; i++) {
            if (resistance_Ohm < ts_ntcalug01a103gLut[i].resistance_Ohm) {
                between_low  = i + 1u;
                between_high = i;
            }
        }

        /* Interpolate between LUT values, but do not extrapolate LUT! */
        if (!(((between_high == 0u) && (between_low == 0u)) || /* measured resistance > maximum LUT resistance */
              (between_low >= ts_ntcalug01a103gLutSize))) {    /* measured resistance < minimum LUT resistance */
            temperature_ddegC = (int16_t)MATH_LinearInterpolation(
                ts_ntcalug01a103gLut[between_low].resistance_Ohm,
                ts_ntcalug01a103gLut[between_low].temperature_ddegC,
                ts_ntcalug01a103gLut[between_high].resistance_Ohm,
                ts_ntcalug01a103gLut[between_high].temperature_ddegC,
                resistance_Ohm);
        }
    }

    /* Return temperature based on measured NTC resistance */
    return temperature_ddegC;
}

extern int16_t TS_Vis00GetTemperatureFromPolynomial(uint16_t adcVoltage_mV, float_t supplyVoltage_V) {
    (void)adcVoltage_mV;
    (void)supplyVoltage_V;
    FAS_ASSERT(FAS_TRAP);
    int16_t temperature_ddegC = 0;
    /* TODO this is not implemented */
    return temperature_ddegC;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
