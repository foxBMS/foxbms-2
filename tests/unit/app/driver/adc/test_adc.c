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
 * @file    test_adc.c
 * @author  foxBMS Team
 * @date    2020-04-01 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Tests for the ADC driver
 * @details Tests the ADC Control and ConvertVoltage
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "MockHL_adc.h"
#include "Mockdatabase.h"
#include "Mockfassert.h"

#include "HL_reg_adc.h"

#include "adc.h"
#include "test_assert_helper.h"

#include <stdint.h>

/*========== Unit Testing Framework Directives ==============================*/
TEST_INCLUDE_PATH("../../src/app/driver/adc")

/*========== Definitions and Implementations for Unit Test ==================*/

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/

void testADC_ConvertVoltage(void) {
    TEST_ASSERT_EQUAL_FLOAT(15.25879f, TEST_ADC_ConvertVoltage(12u));
    TEST_ASSERT_EQUAL_FLOAT(0.6103516f, TEST_ADC_ConvertVoltage(0u));
    TEST_ASSERT_EQUAL_FLOAT(79999.39f, TEST_ADC_ConvertVoltage(UINT16_MAX));
}

void testADC_Control(void) {
    TEST_ADC_SetAdcConversionState(ADC_START_CONVERSION);
    adcStartConversion_Expect(adcREG1, adcGROUP1);
    ADC_Control();
    TEST_ASSERT_EQUAL(ADC_WAIT_CONVERSION_FINISHED, TEST_ADC_GetAdcConversionState());

    /* stay in ADC_WAIT_CONVERSION_FINISHED 2-times */
    adcIsConversionComplete_ExpectAndReturn(adcREG1, adcGROUP1, 0u);
    ADC_Control();
    TEST_ASSERT_EQUAL(ADC_WAIT_CONVERSION_FINISHED, TEST_ADC_GetAdcConversionState());

    adcIsConversionComplete_ExpectAndReturn(adcREG1, adcGROUP1, 0u);
    ADC_Control();
    TEST_ASSERT_EQUAL(ADC_WAIT_CONVERSION_FINISHED, TEST_ADC_GetAdcConversionState());

    /* reached endbit */
    adcIsConversionComplete_ExpectAndReturn(adcREG1, adcGROUP1, ADC_CONVERSION_ENDDBIT);
    ADC_Control();
    TEST_ASSERT_EQUAL(ADC_CONVERSION_FINISHED, TEST_ADC_GetAdcConversionState());

    /* convert */
    static adcData_t adc_adc1RawVoltages[MCU_ADC1_MAX_NR_CHANNELS] = {0};

    adcGetData_ExpectAndReturn(adcREG1, adcGROUP1, &adc_adc1RawVoltages[0], 0u);
    adcGetData_ReturnArrayThruPtr_data(&adc_adc1RawVoltages[0], MCU_ADC1_MAX_NR_CHANNELS);
    DATA_Write1DataBlock_ExpectAndReturn(TEST_ADC_GetAdc1Voltages(), STD_OK);
    ADC_Control();
    TEST_ASSERT_EQUAL(ADC_START_CONVERSION, TEST_ADC_GetAdcConversionState());
}
