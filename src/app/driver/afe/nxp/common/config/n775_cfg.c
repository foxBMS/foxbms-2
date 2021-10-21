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
 * @file    n775_cfg.c
 * @author  foxBMS Team
 * @date    2020-05-08 (date of creation)
 * @updated 2021-03-22 (date of last update)
 * @ingroup DRIVERS_CONFIGURATION
 * @prefix  N775
 *
 * @brief   Configuration for the MC33775A monitoring chip
 *
 */

/*========== Includes =======================================================*/
#include "n775_cfg.h"

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/
const uint8_t n775_voltage_input_used[BS_MAX_SUPPORTED_CELLS] = {
#if BS_MAX_SUPPORTED_CELLS == 12 || BS_MAX_SUPPORTED_CELLS == 15 || BS_MAX_SUPPORTED_CELLS == 18 || \
    BS_MAX_SUPPORTED_CELLS == 36
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
#endif
#if BS_MAX_SUPPORTED_CELLS == 15 || BS_MAX_SUPPORTED_CELLS == 18 || BS_MAX_SUPPORTED_CELLS == 36
    1, 1, 1,
#endif
#if BS_MAX_SUPPORTED_CELLS == 18 || BS_MAX_SUPPORTED_CELLS == 36
    1, 1, 1,
#endif
#if BS_MAX_SUPPORTED_CELLS == 36
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,

#endif
};

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/
float N775_Convert_MuxVoltages_to_Temperatures(float v_adc_V) {
    float temperature = 0.0;

    /* Example: 5th grade polynomial for EPCOS B57861S0103F045 NTC-Thermistor, 10 kOhm, Series B57861S, Vref = 3V, R in series 10k */
    /* temperature = TS_Epc01GetTemperatureFromPolynomial(v_adc_V*1000); */

    /* Dummy function, must be adapted to the application */
    temperature = 10.0f * v_adc_V;

    return temperature;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
