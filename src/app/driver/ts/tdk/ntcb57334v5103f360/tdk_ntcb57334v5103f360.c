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
 * @file    tdk_ntcb57334v5103f360.c
 * @author  foxBMS Team
 * @date    2024-12-03 (date of creation)
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
#include "tdk_ntcb57334v5103f360.h"

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
static const TS_TEMPERATURE_SENSOR_LUT_s ts_ntcb57334v5103f360Lut[] = {
    { -550, 572110.00f },
    { -500, 411880.00f },
    { -450, 300420.00f },
    { -400, 221810.00f },
    { -350, 165660.00f },
    { -300, 125060.00f },
    { -250,  95371.00f },
    { -200,  73432.00f },
    { -150,  57055.00f },
    { -100,  44713.00f },
    {  -50,  35329.00f },
    {    0,  28132.00f },
    {   50,  22568.00f },
    {  100,  18233.00f },
    {  150,  14830.00f },
    {  200,  12140.00f },
    {  250,  10000.00f },
    {  300,   8285.60f },
    {  350,   6904.00f },
    {  400,   5784.00f },
    {  450,   4871.00f },
    {  500,   4122.50f },
    {  550,   3505.70f },
    {  600,   2995.00f },
    {  650,   2570.00f },
    {  700,   2214.60f },
    {  750,   1916.20f },
    {  800,   1664.60f },
    {  850,   1451.40f },
    {  900,   1270.20f },
    {  950,   1115.50f },
    { 1000,   982.96f },
    { 1050,   868.99f },
    { 1100,   770.66f },
    { 1150,   685.54f },
    { 1200,   611.60f },
    { 1250,   547.18f },
    { 1300,   490.89f },
    { 1350,   441.55f },
    { 1400,   398.18f },
    { 1450,   359.96f },
    { 1500,   326.19f },
};
/* clang-format on */

/** size of the #ts_ntcb57334v5103f360Lut LUT */
static uint16_t ts_ntcb57334v5103f360LutSize = sizeof(ts_ntcb57334v5103f360Lut) / sizeof(TS_TEMPERATURE_SENSOR_LUT_s);

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
#if defined(TS_TDK_NTCB57334V5103F360_POSITION_IN_RESISTOR_DIVIDER_IS_R_1) && \
    (TS_TDK_NTCB57334V5103F360_POSITION_IN_RESISTOR_DIVIDER_IS_R_1 == true)
#define TS_TDK_NTCB57334V5103F360_ADC_VOLTAGE_V_MAX_V                           \
    ((float_t)((TS_TDK_NTCB57334V5103F360_RESISTOR_DIVIDER_SUPPLY_VOLTAGE_V *   \
                ts_ntcb57334v5103f360Lut[ts_103jtLutSize - 1].resistance_Ohm) / \
               (ts_ntcb57334v5103f360Lut[ts_103jtLutSize - 1].resistance_Ohm +  \
                TS_TDK_NTCB57334V5103F360_RESISTOR_DIVIDER_RESISTANCE_R_1_R_2_Ohm)))
#define TS_TDK_NTCB57334V5103F360_ADC_VOLTAGE_V_MIN_V                         \
    ((float_t)((TS_TDK_NTCB57334V5103F360_RESISTOR_DIVIDER_SUPPLY_VOLTAGE_V * \
                ts_ntcb57334v5103f360Lut[0].resistance_Ohm) /                 \
               (ts_ntcb57334v5103f360Lut[0].resistance_Ohm +                  \
                TS_TDK_NTCB57334V5103F360_RESISTOR_DIVIDER_RESISTANCE_R_1_R_2_Ohm)))
#else /* TS_TDK_NTCB57334V5103F360_POSITION_IN_RESISTOR_DIVIDER_IS_R_1 == false */
#define TS_TDK_NTCB57334V5103F360_ADC_VOLTAGE_V_MIN_V                                        \
    ((float_t)((TS_TDK_NTCB57334V5103F360_RESISTOR_DIVIDER_SUPPLY_VOLTAGE_V *                \
                ts_ntcb57334v5103f360Lut[ts_ntcb57334v5103f360LutSize - 1].resistance_Ohm) / \
               (ts_ntcb57334v5103f360Lut[ts_ntcb57334v5103f360LutSize - 1].resistance_Ohm +  \
                TS_TDK_NTCB57334V5103F360_RESISTOR_DIVIDER_RESISTANCE_R_1_R_2_Ohm)))
#define TS_TDK_NTCB57334V5103F360_ADC_VOLTAGE_V_MAX_V                         \
    ((float_t)((TS_TDK_NTCB57334V5103F360_RESISTOR_DIVIDER_SUPPLY_VOLTAGE_V * \
                ts_ntcb57334v5103f360Lut[0].resistance_Ohm) /                 \
               (ts_ntcb57334v5103f360Lut[0].resistance_Ohm +                  \
                TS_TDK_NTCB57334V5103F360_RESISTOR_DIVIDER_RESISTANCE_R_1_R_2_Ohm)))
#endif
/**@}*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/

extern int16_t TS_Tdk02GetTemperatureFromLut(uint16_t adcVoltage_mV, float_t supplyVoltage_V) {
    /* AXIVION Routine Generic-MissingParameterAssert: adcVoltage_mV: parameter accepts whole range */

    int16_t temperature_ddegC = 0;
    float_t resistance_Ohm    = 0.0f;
    float_t adcVoltage_V      = adcVoltage_mV / 1000.0f; /* Convert mV to V */

    /* Check for valid ADC measurements to prevent undefined behavior */
    if (adcVoltage_V > TS_TDK_NTCB57334V5103F360_ADC_VOLTAGE_V_MAX_V) {
        /* Invalid measured ADC voltage -> sensor out of operating range or disconnected/shorted */
        temperature_ddegC = INT16_MIN;
    } else if (adcVoltage_V < TS_TDK_NTCB57334V5103F360_ADC_VOLTAGE_V_MIN_V) {
        /* Invalid measured ADC voltage -> sensor out of operating range or shorted/disconnected */
        temperature_ddegC = INT16_MAX;
    } else {
        /* Calculate NTC resistance based on measured ADC voltage */
#if defined(TS_TDK_NTCB57334V5103F360_POSITION_IN_RESISTOR_DIVIDER_IS_R_1) && \
    (TS_TDK_NTCB57334V5103F360_POSITION_IN_RESISTOR_DIVIDER_IS_R_1 == true)
        /* R_1 = R_2 * ( ( V_supply / V_adc ) - 1 ) */
        resistance_Ohm = TS_TDK_NTCB57334V5103F360_RESISTOR_DIVIDER_RESISTANCE_R_1_R_2_Ohm *
                         ((supplyVoltage_V / adcVoltage_V) - 1.0f);
#else  /* TS_TDK_NTCB57334V5103F360_POSITION_IN_RESISTOR_DIVIDER_IS_R_1 == false */
        /* R_2 = R_1 * ( V_2 / ( V_supply - V_adc ) ) */
        resistance_Ohm = TS_TDK_NTCB57334V5103F360_RESISTOR_DIVIDER_RESISTANCE_R_1_R_2_Ohm *
                         (adcVoltage_V / (supplyVoltage_V - adcVoltage_V));
#endif /* TS_TDK_NTCB57334V5103F360_POSITION_IN_RESISTOR_DIVIDER_IS_R_1 */

        /* Variables for interpolating LUT value */
        uint16_t between_high = 0;
        uint16_t between_low  = 0;
        for (uint16_t i = 1; i < ts_ntcb57334v5103f360LutSize; i++) {
            if (resistance_Ohm < ts_ntcb57334v5103f360Lut[i].resistance_Ohm) {
                between_low  = i + 1u;
                between_high = i;
            }
        }

        /* Interpolate between LUT values, but do not extrapolate LUT! */
        if (!(((between_high == 0u) && (between_low == 0u)) || /* measured resistance > maximum LUT resistance */
              (between_low > ts_ntcb57334v5103f360LutSize))) { /* measured resistance < minimum LUT resistance */
            temperature_ddegC = (int16_t)MATH_LinearInterpolation(
                ts_ntcb57334v5103f360Lut[between_low].resistance_Ohm,
                ts_ntcb57334v5103f360Lut[between_low].temperature_ddegC,
                ts_ntcb57334v5103f360Lut[between_high].resistance_Ohm,
                ts_ntcb57334v5103f360Lut[between_high].temperature_ddegC,
                resistance_Ohm);
        }
    }

    /* Return temperature based on measured NTC resistance */
    return temperature_ddegC;
}

extern int16_t TS_Tdk02GetTemperatureFromPolynomial(uint16_t adcVoltage_mV, float_t supplyVoltage_V) {
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
