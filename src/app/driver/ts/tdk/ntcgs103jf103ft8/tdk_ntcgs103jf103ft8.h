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
 * @file    tdk_ntcgs103jf103ft8.h
 * @author  foxBMS Team
 * @date    2024-12-03 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup DRIVERS
 * @prefix  TS
 *
 * @brief   Resistive divider used for measuring temperature
 * @details Schematics:
 *
 *          V_supply
 *            --.--
 *              |
 *            +-.-+
 *            |   |
 *            |   |  R_1
 *            |   |
 *            +-.-+
 *              |
 *              .--- V_adc
 *              |
 *            +-.-+
 *            |   |
 *            |   |  R_2
 *            |   |
 *            +-.-+
 *              |
 *            --.--
 *             GND
 *
 */

#ifndef FOXBMS__TDK_NTCGS103JF103FT8_H_
#define FOXBMS__TDK_NTCGS103JF103FT8_H_

/*========== Includes =======================================================*/

#include <stdint.h>

/*========== Macros and Definitions =========================================*/
/**
 * Position of the NTC in the voltage resistor
 * true: NTC is positioned above the voltage tap for the ADC voltage.
 * This equals resistor R_1 in the above circuit diagram
 *
 * false: NTC is positioned below the voltage tap for the ADC voltage.
 * This equals resistor R_2 in the above circuit diagram
 */
#define TS_TDK_NTCGS103JF103FT8_POSITION_IN_RESISTOR_DIVIDER_IS_R_1 (false)

/** Resistor divider supply voltage in volt */
#define TS_TDK_NTCGS103JF103FT8_RESISTOR_DIVIDER_SUPPLY_VOLTAGE_V (2.5f)

/**
 * Resistance value of the other resistor (not the NTC) in the resistor
 * divider in Ohm.
 */
#define TS_TDK_NTCGS103JF103FT8_RESISTOR_DIVIDER_RESISTANCE_R_1_R_2_Ohm (10000.0f)

/*========== Extern Constant and Variable Declarations ======================*/
/**
 * @brief   returns temperature based on measured ADC voltage
 * @param   adcVoltage_mV voltage in mV
 * @return  corresponding temperature in deci &deg;C or INT16_MAX/INT16_MIN if
 *          NTC is shorted or got disconnected. The caller of this functions
 *          needs to check for these return values to prevent invalid data.
 */
extern int16_t TS_Tdk00GetTemperatureFromLut(uint16_t adcVoltage_mV);

/**
 * @brief   returns temperature based on measured ADC voltage
 * @param   adcVoltage_mV   voltage in mV
 * @return  corresponding temperature in deci &deg;C
 */
extern int16_t TS_Tdk00GetTemperatureFromPolynomial(uint16_t adcVoltage_mV);

/*========== Extern Function Prototypes =====================================*/

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
#endif

#endif /* FOXBMS__TDK_NTCGS103JF103FT8_H_ */
