/**
 *
 * @copyright &copy; 2010 - 2024, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * @file    test_adi_ades1830_temperatures_2.c
 * @author  foxBMS Team
 * @date    2024-01-30 (date of creation)
 * @updated 2024-12-20 (date of last update)
 * @version v1.8.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Additional tests of the temperature measurement functionality
 * @details This file implements additional parts of the tests of the external
 *          and internal (static) functions of adi_ades183x_temperatures.c.
 *          The interface function #ADI_GetTemperatures is tested directly,
 *          while the static functions are tested via their externalize
 *          wrappers.
 *
 *          The internal functions
 *          - #ADI_GetMappedGpioIndex (external wrapper
 *            #TEST_ADI_GetMappedGpioIndex)
 *
 *          are validated to call the expected routines with the expected
 *          arguments.
 *
 *          The interface function #ADI_GetTemperatures is tested to call the
 *          expected functions and based on their output correctly set the
 *          measured temperature in the data table of the driver.
 */

/*========== Includes =======================================================*/

#include "unity.h"
#include "Mockadi_ades183x_cfg.h"
#include "Mockadi_ades183x_helpers.h"

/* clang-format off */
#include "general.h"
/* clang-format on */

#include "adi_ades183x_buffers.h"  /* use the real command config */
#include "adi_ades183x_commands.h" /* use the real buffer configuration */
#include "adi_ades183x_temperatures.h"
#include "test_assert_helper.h"

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("adi_ades183x_temperatures.c")

TEST_INCLUDE_PATH("../../src/app/application/config")
TEST_INCLUDE_PATH("../../src/app/driver/afe/adi/ades1830")
TEST_INCLUDE_PATH("../../src/app/driver/afe/adi/common/ades183x")
TEST_INCLUDE_PATH("../../src/app/driver/afe/adi/common/ades183x/config")
TEST_INCLUDE_PATH("../../src/app/driver/afe/adi/common/ades183x/pec")
TEST_INCLUDE_PATH("../../src/app/driver/afe/api")
TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/dma")
TEST_INCLUDE_PATH("../../src/app/driver/io")
TEST_INCLUDE_PATH("../../src/app/driver/pex")
TEST_INCLUDE_PATH("../../src/app/driver/rtc")
TEST_INCLUDE_PATH("../../src/app/driver/spi")
TEST_INCLUDE_PATH("../../src/app/driver/ts/api")
TEST_INCLUDE_PATH("../../src/app/engine/database")
TEST_INCLUDE_PATH("../../src/app/engine/diag")
TEST_INCLUDE_PATH("../../src/app/task/config")
TEST_INCLUDE_PATH("../../src/app/task/ftask")

/*========== Definitions and Implementations for Unit Test ==================*/
static DATA_BLOCK_CELL_TEMPERATURE_s adi_cellTemperature = {.header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE_BASE};
static DATA_BLOCK_ALL_GPIO_VOLTAGES_s adi_allGpioVoltage = {.header.uniqueId = DATA_BLOCK_ID_ALL_GPIO_VOLTAGES_BASE};

ADI_STATE_s adi_stateBase = {
    .currentString        = 0u,
    .data.cellTemperature = &adi_cellTemperature,
    .data.allGpioVoltages = &adi_allGpioVoltage,
};

const uint16_t testTemperature_ddegC = 42;
const uint16_t testGpioVoltage_mV    = 0u;

/* this test checks, that the driver works as expected, when the temperature
   inputs are *not* used */
const uint8_t adi_temperatureInputsUsed[BS_NR_OF_GPIOS_PER_MODULE] = {
    GEN_REPEAT_U(0u, GEN_STRIP(BS_NR_OF_GPIOS_PER_MODULE))};

/*========== Setup and Teardown =============================================*/
void setUp(void) {
    /* set all temperatures to a test value, so that we can later check, that
       these were not changed */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        adi_stateBase.currentString = s;
        for (uint16_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint16_t registerGpioIndex = 0u; registerGpioIndex < BS_NR_OF_GPIOS_PER_MODULE; registerGpioIndex++) {
                for (uint8_t ts = 0; ts < registerGpioIndex; ts++) {
                    adi_stateBase.data.cellTemperature->cellTemperature_ddegC[adi_stateBase.currentString][m][ts] =
                        testTemperature_ddegC;
                }
            }
        }
    }
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/

/*========== Externalized Static Function Test Cases ========================*/
/**
 * @brief   Testing static function ADI_GetMappedGpioIndex
 * @details The following additional cases need to be tested:
 *          - Routine validation:
 *            - RT2/2: all inputs are *not* used as inputs, therefore the index
 *                     array shall be fully set.
 *
 *          For all other test cases: see the test description of
 *          ADI_GetTemperatures in
 *          tests/unit/app/driver/afe/adi/ades1830/test_adi_ades1830_temperatures.c
 */
void testADI_GetMappedGpioIndex(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1: Assertion test */
    TEST_ASSERT_FAIL_ASSERT(TEST_ADI_GetMappedGpioIndex(BS_NR_OF_TEMP_SENSORS_PER_MODULE));

    /* ======= Routine tests =============================================== */

    /* ======= RT1/2: Test implementation */
    uint16_t calculatedTemperatureIndex[BS_NR_OF_TEMP_SENSORS_PER_MODULE] = {
        GEN_REPEAT_U(0u, GEN_STRIP(BS_NR_OF_TEMP_SENSORS_PER_MODULE))};
    uint16_t expectedTemperatureIndex[BS_NR_OF_TEMP_SENSORS_PER_MODULE] = {
        GEN_REPEAT_U(0u, GEN_STRIP(BS_NR_OF_TEMP_SENSORS_PER_MODULE))};

    /* ======= RT1/2: call function under test */
    for (uint8_t ts = 0u; ts < BS_NR_OF_TEMP_SENSORS_PER_MODULE; ts++) {
        calculatedTemperatureIndex[ts] = TEST_ADI_GetMappedGpioIndex(ts);
    }

    /* ======= RT1/2: test output verification */
    for (uint8_t ts = 0u; ts < BS_NR_OF_TEMP_SENSORS_PER_MODULE; ts++) {
        TEST_ASSERT_EQUAL_UINT16(expectedTemperatureIndex[ts], calculatedTemperatureIndex[ts]);
    }
}

/*========== Extern Function Test Cases =====================================*/

/**
 * @brief   Testing extern function #ADI_GetTemperatures
 * @details The following additional cases need to be tested:
 *          - Routine validation:
 *            - RT2/2: the input table does not set the temperature
 *                     measurement, i.e., no temperature measurement shall be
 *                     performed.
 *
 *          For all other test cases: see the test description of
 *          ADI_GetTemperatures in
 *          tests/unit/app/driver/afe/adi/ades1830/test_adi_ades1830_temperatures.c
 */
void testADI_GetTemperatures(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1: Assertion test */
    TEST_ASSERT_FAIL_ASSERT(ADI_GetTemperatures(NULL_PTR));

    /* ======= Routine tests =============================================== */
    /* ======= RT2/2: Test implementation */

    int16_t expectedTemperatures[BS_NR_OF_STRINGS][BS_NR_OF_MODULES_PER_STRING][BS_NR_OF_TEMP_SENSORS_PER_MODULE];
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        adi_stateBase.currentString = s;
        for (uint16_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t ts = 0; ts < BS_NR_OF_TEMP_SENSORS_PER_MODULE; ts++) {
                expectedTemperatures[adi_stateBase.currentString][m][ts] = testTemperature_ddegC;
            }
        }
    }

    /* ======= RT2/2: call function under test */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        adi_stateBase.currentString = s;
        for (uint16_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint16_t ts = 0u; ts < BS_NR_OF_TEMP_SENSORS_PER_MODULE; ts++) {
                ADI_ConvertGpioVoltageToTemperature_ExpectAndReturn(testGpioVoltage_mV, testTemperature_ddegC);
            }
        }
        ADI_GetTemperatures(&adi_stateBase);
    }
    /* ======= RT2/2: test output verification */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        adi_stateBase.currentString = s;
        for (uint16_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t ts = 0; ts < BS_NR_OF_TEMP_SENSORS_PER_MODULE; ts++) {
                TEST_ASSERT_EQUAL_INT16(
                    expectedTemperatures[adi_stateBase.currentString][m][ts],
                    adi_stateBase.data.cellTemperature->cellTemperature_ddegC[adi_stateBase.currentString][m][ts]);
            }
        }
    }
}
