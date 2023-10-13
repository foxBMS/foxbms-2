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
 * @file    adi_ades183x_temperatures.c
 * @author  foxBMS Team
 * @date    2019-08-27 (date of creation)
 * @updated 2023-10-12 (date of last update)
 * @version v1.6.0
 * @ingroup DRIVER
 * @prefix  ADI
 *
 * @brief   Implementation of some software
 *
 */

/*========== Includes =======================================================*/

#include "adi_ades183x_temperatures.h"

#include "adi_ades183x_cfg.h"

#include "fassert.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/
/**
 * @brief   Converts index of read GPIO voltages for temperatures.
 * @details This function translates the indexes of read GPIO voltages so that
 *          if some inputs are unused, they do not appear in the final cell
 *          temperature table.
 * @param   registerGpioIndex   index of the read voltage ion the ades183x
 *                              register
 * @return  index where the voltage must be stored if it is used
 */
static uint16_t ADI_GetStoredTemperatureIndex(uint16_t registerGpioIndex);

/*========== Static Function Implementations ================================*/
static uint16_t ADI_GetStoredTemperatureIndex(uint16_t registerGpioIndex) {
    FAS_ASSERT(registerGpioIndex < BS_NR_OF_GPIOS_PER_MODULE);

    uint16_t storedTemperatureIndex = 0u;
    for (uint8_t c = 0; c < registerGpioIndex; c++) {
        if (adi_temperatureInputsUsed[c] == 1u) {
            storedTemperatureIndex++;
        }
    }

    return storedTemperatureIndex;
}

/*========== Extern Function Implementations ================================*/
/* RequirementId: D7.1 V0R4 FUN-2.10.01.01 */
extern void ADI_GetTemperatures(ADI_STATE_s *adiState) {
    FAS_ASSERT(adiState != NULL_PTR);

    for (uint16_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
        for (uint16_t registerGpioIndex = 0u; registerGpioIndex < BS_NR_OF_GPIOS_PER_MODULE; registerGpioIndex++) {
            if (adi_temperatureInputsUsed[registerGpioIndex] == 1u) {
                uint16_t storedTemperatureIndex = ADI_GetStoredTemperatureIndex(registerGpioIndex);
                int16_t temperature             = ADI_ConvertGpioVoltageToTemperature(
                    adiState->data.allGpioVoltages->gpioVoltages_mV[adiState->currentString]
                                                                   [(m * BS_NR_OF_GPIOS_PER_MODULE) +
                                                                    registerGpioIndex]); /* unit: decidegree C */
                adiState->data.cellTemperature
                    ->cellTemperature_ddegC[adiState->currentString][m][storedTemperatureIndex] = temperature;
            }
        }
    }
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
extern uint16_t TEST_ADI_GetStoredTemperatureIndex(uint16_t registerGpioIndex) {
    return ADI_GetStoredTemperatureIndex(registerGpioIndex);
}

#endif
