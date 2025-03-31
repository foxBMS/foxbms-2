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
 * @file    adi_ades183x_temperatures.c
 * @author  foxBMS Team
 * @date    2019-08-27 (date of creation)
 * @updated 2025-03-31 (date of last update)
 * @version v1.9.0
 * @ingroup DRIVERS
 * @prefix  ADI
 *
 * @brief   Helper functions related to temperature measurement
 * @details This driver implements a helper function to convert the voltages
 *          that are measured on the GPIOs into a temperature.
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
 * @brief   Extracts GPIO index of for requested temperature index
 * @details This function translates the indexes of read GPIO voltages so that
 *          if some inputs are unused, they do not appear in the final cell
 *          temperature table.
 * @param   temperatureSensorIndex   requested sensor index
 * @return  index where the voltage must is stored
 */
static uint16_t ADI_GetMappedGpioIndex(uint16_t temperatureSensorIndex);

/*========== Static Function Implementations ================================*/
static uint16_t ADI_GetMappedGpioIndex(uint16_t temperatureSensorIndex) {
    FAS_ASSERT(temperatureSensorIndex < BS_NR_OF_TEMP_SENSORS_PER_MODULE);

    uint16_t storedGpioIndex = 0u;
    uint16_t mappedIndex     = 0u;
    for (uint8_t gpioIndex = 0u; gpioIndex < SLV_NR_OF_GPIOS_PER_MODULE; gpioIndex++) {
        if (adi_temperatureInputsUsed[gpioIndex] == 1u) {
            if (storedGpioIndex == temperatureSensorIndex) {
                mappedIndex = gpioIndex;
                break;
            } else {
                storedGpioIndex++;
            }
        }
    }

    return mappedIndex;
}

/*========== Extern Function Implementations ================================*/
/* RequirementId: D7.1 V1R0 FUN-2.10.01.01 */
extern void ADI_GetTemperatures(ADI_STATE_s *pAdiState) {
    FAS_ASSERT(pAdiState != NULL_PTR);

    for (uint16_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
        for (uint8_t ts = 0u; ts < BS_NR_OF_TEMP_SENSORS_PER_MODULE; ts++) {
            uint16_t gpioIndex = ADI_GetMappedGpioIndex(ts);
            /* Check GPIO voltage valid flag */
            uint16_t invalidFlag =
                (pAdiState->data.allGpioVoltages->invalidGpioVoltages[pAdiState->currentString][m] & (1u << gpioIndex));
            gpioIndex += (m * SLV_NR_OF_GPIOS_PER_MODULE); /* Add module offset */
            if (invalidFlag == 0u) {
                /* temperature: unit is in deci-degree C */
                int16_t temperature_ddegC = ADI_ConvertGpioVoltageToTemperature(
                    pAdiState->data.allGpioVoltages->gpioVoltages_mV[pAdiState->currentString][gpioIndex]);
                pAdiState->data.cellTemperature->cellTemperature_ddegC[pAdiState->currentString][m][ts] =
                    temperature_ddegC;
                pAdiState->data.cellTemperature->invalidCellTemperature[pAdiState->currentString][m][ts] = false;
            } else {
                /* Invalidate cell temperature */
                pAdiState->data.cellTemperature->cellTemperature_ddegC[pAdiState->currentString][m][ts]  = 0;
                pAdiState->data.cellTemperature->invalidCellTemperature[pAdiState->currentString][m][ts] = true;
            }
        }
    }
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
extern uint16_t TEST_ADI_GetMappedGpioIndex(uint16_t registerGpioIndex) {
    return ADI_GetMappedGpioIndex(registerGpioIndex);
}

#endif
