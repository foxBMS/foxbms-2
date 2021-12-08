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
 * @file    vishay_ntcalug01a103g.c
 * @author  foxBMS Team
 * @date    2018-10-30 (date of creation)
 * @updated 2021-11-08 (date of last update)
 * @ingroup TEMPERATURE_SENSORS
 * @prefix  TS
 *
 * @brief   Resistive divider used for measuring temperature
 *
 */

/*========== Includes =======================================================*/
#include "vishay_ntcalug01a103g.h"

#include "foxmath.h"
#include "temperature_sensor_defs.h"

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/* clang-format off */
/** LUT filled from higher resistance to lower resistance */
static const TS_TEMPERATURE_SENSOR_LUT_s ts_ntcalug01a103gLut[] = {
        { -400, 334274.4f},
        { -390, 312904.4f},
        { -380, 293033.6f},
        { -370, 274548.0f},
        { -360, 257343.1f},
        { -350, 241322.9f},
        { -340, 226398.8f},
        { -330, 212489.7f},
        { -320, 199520.6f},
        { -310, 187422.7f},
        { -300, 176132.5f},
        { -290, 165591.5f},
        { -280, 155745.6f},
        { -270, 146545.1f},
        { -260, 137944.1f},
        { -250, 129900.0f},
        { -240, 122373.7f},
        { -230, 115329.0f},
        { -220, 108732.2f},
        { -210, 102552.5f},
        { -200, 96761.1f },
        { -190, 91331.5f },
        { -180, 86239.0f },
        { -170, 81460.9f },
        { -160, 76976.0f },
        { -150, 72764.6f },
        { -140, 68808.6f },
        { -130, 65091.1f },
        { -120, 61596.4f },
        { -110, 58309.9f },
        { -100, 55218.1f },
        {  -90, 52308.4f },
        {  -80, 49569.0f },
        {  -70, 46989.1f },
        {  -60, 44558.56f},
        {  -50, 42267.85f},
        {  -40, 40108.20f},
        {  -30, 38071.41f},
        {  -20, 36149.83f},
        {  -10, 34336.32f},
        {   0, 32624.23f},
        {   10, 31007.34f},
        {   20, 29479.85f},
        {   30, 28036.35f},
        {   40, 26671.76f},
        {   50, 25381.36f},
        {   60, 24160.73f},
        {   70, 23005.71f},
        {   80, 21912.45f},
        {   90, 20877.31f},
        {  100, 19896.90f},
        {  110, 18968.04f},
        {  120, 18087.75f},
        {  130, 17253.25f},
        {  140, 16461.90f},
        {  150, 15711.26f},
        {  160, 14999.01f},
        {  170, 14323.01f},
        {  180, 13681.22f},
        {  190, 13071.73f},
        {  200, 12492.75f},
        {  210, 11942.59f},
        {  220, 11419.69f},
        {  230, 10922.54f},
        {  240, 10449.75f},
        {  250, 10000.00f},
        {  260, 9572.05f },
        {  270, 9164.74f },
        {  280, 8776.97f },
        {  290, 8407.70f },
        {  300, 8055.96f },
        {  310, 7720.82f },
        {  320, 7401.43f },
        {  330, 7096.96f },
        {  340, 6806.64f },
        {  350, 6529.74f },
        {  360, 6265.58f },
        {  370, 6013.51f },
        {  380, 5772.92f },
        {  390, 5543.22f },
        {  400, 5323.88f },
        {  410, 5114.37f },
        {  420, 4914.20f },
        {  430, 4722.92f },
        {  440, 4540.08f },
        {  450, 4365.27f },
        {  460, 4198.11f },
        {  470, 4038.21f },
        {  480, 3885.23f },
        {  490, 3738.84f },
        {  500, 3598.72f },
        {  510, 3464.58f },
        {  520, 3336.12f },
        {  530, 3213.08f },
        {  540, 3095.22f },
        {  550, 2982.27f },
        {  560, 2874.02f },
        {  570, 2770.26f },
        {  580, 2670.76f },
        {  590, 2575.34f },
        {  600, 2483.82f },
        {  610, 2396.00f },
        {  620, 2311.74f },
        {  630, 2230.85f },
        {  640, 2153.21f },
        {  650, 2078.65f },
        {  660, 2007.05f },
        {  670, 1938.27f },
        {  680, 1872.19f },
        {  690, 1808.69f },
        {  700, 1747.65f },
        {  710, 1688.98f },
        {  720, 1632.56f },
        {  730, 1578.31f },
        {  740, 1526.13f },
        {  750, 1475.92f },
        {  760, 1427.62f },
        {  770, 1381.12f },
        {  780, 1336.37f },
        {  790, 1293.29f },
        {  800, 1251.80f },
        {  810, 1211.85f },
        {  820, 1173.36f },
        {  830, 1136.28f },
        {  840, 1100.55f },
        {  850, 1066.11f },
        {  860, 1032.91f },
        {  870, 1000.91f },
        {  880, 970.05f },
        {  890, 940.29f },
        {  900, 911.59f },
        {  910, 883.89f },
        {  920, 857.17f },
        {  930, 831.38f },
        {  940, 806.49f },
        {  950, 782.46f },
        {  960, 759.26f },
        {  970, 736.85f },
        {  980, 715.21f },
        {  990, 694.31f },
        { 1000, 674.11f },
        { 1010, 654.60f },
        { 1020, 635.74f },
        { 1030, 617.51f },
        { 1040, 599.88f },
        { 1050, 582.84f }
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
#if TS_VISHAY_NTCALUG01A103G_POSITION_IN_RESISTOR_DIVIDER_IS_R_1 == true
#define TS_VISHAY_NTCALUG01A103G_ADC_VOLTAGE_V_MAX_V \
    (float)((TS_VISHAY_NTCALUG01A103G_RESISTOR_DIVIDER_SUPPLY_VOLTAGE_V * ts_ntcalug01a103gLut[ts_ntcalug01a103gLutSize-1].resistance_Ohm) / (ts_ntcalug01a103gLut[ts_ntcalug01a103gLutSize-1].resistance_Ohm+TS_VISHAY_NTCALUG01A103G_RESISTOR_DIVIDER_RESISTANCE_R_1_R_2_Ohm))
#define TS_VISHAY_NTCALUG01A103G_ADC_VOLTAGE_V_MIN_V \
    (float)((TS_VISHAY_NTCALUG01A103G_RESISTOR_DIVIDER_SUPPLY_VOLTAGE_V * ts_ntcalug01a103gLut[0].resistance_Ohm) / (ts_ntcalug01a103gLut[0].resistance_Ohm+TS_VISHAY_NTCALUG01A103G_RESISTOR_DIVIDER_RESISTANCE_R_1_R_2_Ohm))
#else /* TS_VISHAY_NTCALUG01A103G_POSITION_IN_RESISTOR_DIVIDER_IS_R_1 == false */
#define TS_VISHAY_NTCALUG01A103G_ADC_VOLTAGE_V_MIN_V \
    (float)((TS_VISHAY_NTCALUG01A103G_RESISTOR_DIVIDER_SUPPLY_VOLTAGE_V * ts_ntcalug01a103gLut[ts_ntcalug01a103gLutSize-1].resistance_Ohm) / (ts_ntcalug01a103gLut[ts_ntcalug01a103gLutSize-1].resistance_Ohm+TS_VISHAY_NTCALUG01A103G_RESISTOR_DIVIDER_RESISTANCE_R_1_R_2_Ohm))
#define TS_VISHAY_NTCALUG01A103G_ADC_VOLTAGE_V_MAX_V \
    (float)((TS_VISHAY_NTCALUG01A103G_RESISTOR_DIVIDER_SUPPLY_VOLTAGE_V * ts_ntcalug01a103gLut[0].resistance_Ohm) / (ts_ntcalug01a103gLut[0].resistance_Ohm+TS_VISHAY_NTCALUG01A103G_RESISTOR_DIVIDER_RESISTANCE_R_1_R_2_Ohm))
#endif
/**@}*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/

extern int16_t TS_Vis00GetTemperatureFromLut(uint16_t adcVoltage_mV) {
    int16_t temperature_ddegC = 0;
    float resistance_Ohm      = 0.0;
    float adcVoltage_V        = adcVoltage_mV / 1000.0f; /* Convert mV to V */

    /* Check for valid ADC measurements to prevent undefined behavior */
    if (adcVoltage_V > TS_VISHAY_NTCALUG01A103G_ADC_VOLTAGE_V_MAX_V) {
        /* Invalid measured ADC voltage -> sensor out of operating range or disconnected/shorted */
        temperature_ddegC = INT16_MIN;
    } else if (adcVoltage_V < TS_VISHAY_NTCALUG01A103G_ADC_VOLTAGE_V_MIN_V) {
        /* Invalid measured ADC voltage -> sensor out of operating range or shorted/disconnected */
        temperature_ddegC = INT16_MAX;
    } else {
        /* Calculate NTC resistance based on measured ADC voltage */
#if TS_VISHAY_NTCALUG01A103G_POSITION_IN_RESISTOR_DIVIDER_IS_R_1 == true
        /* R_1 = R_2 * ( ( V_supply / V_adc ) - 1 ) */
        resistance_Ohm = TS_VISHAY_NTCALUG01A103G_RESISTOR_DIVIDER_RESISTANCE_R_1_R_2_Ohm *
                         ((TS_VISHAY_NTCALUG01A103G_RESISTOR_DIVIDER_SUPPLY_VOLTAGE_V / adcVoltage_V) - 1);
#else  /* NTCALUG01A103G_POSITION_IN_RESISTOR_DIVIDER_IS_R_1 == false */
        /* R_2 = R_1 * ( V_2 / ( V_supply - V_adc ) ) */
        resistance_Ohm = TS_VISHAY_NTCALUG01A103G_RESISTOR_DIVIDER_RESISTANCE_R_1_R_2_Ohm *
                         (adcVoltage_V / (TS_VISHAY_NTCALUG01A103G_RESISTOR_DIVIDER_SUPPLY_VOLTAGE_V - adcVoltage_V));
#endif /* NTCALUG01A103G_POSITION_IN_RESISTOR_DIVIDER_IS_R_1 */

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

extern int16_t TS_Vis00GetTemperatureFromPolynomial(uint16_t adcVoltage_mV) {
    (void)adcVoltage_mV;
    FAS_ASSERT(FAS_TRAP);
    int16_t temperature_ddegC = 0;
    /* TODO this is not implemented */
    return temperature_ddegC;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
