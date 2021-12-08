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
 * @file    adc.c
 * @author  foxBMS Team
 * @date    2019-01-07 (date of creation)
 * @updated 2021-12-08 (date of last update)
 * @ingroup DRIVERS
 * @prefix  ADC
 *
 * @brief   Driver for the ADC module.
 *
 */

/*========== Includes =======================================================*/
#include "adc.h"

#include "database.h"

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/**
 * @brief   describes the current state of the conversion
 * @details This variable is used as a state-variable for switching through the
 *          steps of a conversion.
 */
static ADC_STATE_e adc_conversionState = ADC_START_CONVERSION;

static adcData_t adc1RawVoltages[ADC_ADC1_MAX_NR_CHANNELS] = {0};

static DATA_BLOCK_ADC_VOLTAGE_s adc1Voltages = {.header.uniqueId = DATA_BLOCK_ID_ADC_VOLTAGE};

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/**
 * @brief   converts reading from ADC to a voltage in mV.
 * @param   adcValue_mV     value read from ADC
 * @return  voltage in mV
 */
static float ADC_ConvertVoltage(float adcValue_mV);

/*========== Static Function Implementations ================================*/

static float ADC_ConvertVoltage(float adcValue_mV) {
    /** For details to equation see Equation 28 in Technical Reference Manual SPNU563A - March 2018 page 852 */
    float result_mV = ((ADC_CONV_FACTOR_12BIT * (adcValue_mV - ADC_VREFLOW_mV)) / (ADC_VREFHIGH_mV - ADC_VREFLOW_mV)) -
                      0.5f;

    return result_mV;
}

/*========== Extern Function Implementations ================================*/

extern void ADC_Control(void) {
    bool conversionFinished = true;

    switch (adc_conversionState) {
        case ADC_START_CONVERSION:
            adcStartConversion(adcREG1, adcGROUP1);
            adc_conversionState = ADC_WAIT_CONVERSION_FINISHED;
            break;

        case ADC_WAIT_CONVERSION_FINISHED:
            conversionFinished = true;
            if (ADC_CONVERSION_ENDDBIT != adcIsConversionComplete(adcREG1, adcGROUP1)) {
                conversionFinished = false;
            }
            if (conversionFinished == true) {
                adc_conversionState = ADC_CONVERSION_FINISHED;
            }
            break;

        /* Start initialization procedure, data sheet figure 106 page 79 */
        case ADC_CONVERSION_FINISHED:
            adcGetData(adcREG1, adcGROUP1, &adc1RawVoltages[0]);
            for (uint8_t i = 0U; i < ADC_ADC1_MAX_NR_CHANNELS; i++) {
                adc1Voltages.adc1ConvertedVoltages_mV[i] = ADC_ConvertVoltage((float)(adc1RawVoltages[i].value));
            }
            DATA_WRITE_DATA(&adc1Voltages);
            adc_conversionState = ADC_START_CONVERSION;
            break;

        default:
            /* invalid state */
            FAS_ASSERT(FAS_TRAP);
            break;
    }
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
