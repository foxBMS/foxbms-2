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
 * @file    test_adi_ades1830_temperatures.c
 * @author  foxBMS Team
 * @date    2022-12-07 (date of creation)
 * @updated 2023-10-12 (date of last update)
 * @version v1.6.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Test of some module
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockadi_ades183x_cfg.h"
#include "Mockadi_ades183x_helpers.h"

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
    .currentString        = 0,
    .data.cellTemperature = &adi_cellTemperature,
    .data.allGpioVoltages = &adi_allGpioVoltage,
};
const uint8_t adi_temperatureInputsUsed[BS_NR_OF_GPIOS_PER_MODULE] = {
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
};

/*========== Setup and Teardown =============================================*/
void setUp(void) {
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        adi_stateBase.currentString = s;
        for (uint16_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint16_t registerGpioIndex = 0u; registerGpioIndex < BS_NR_OF_GPIOS_PER_MODULE; registerGpioIndex++) {
                if (adi_temperatureInputsUsed[registerGpioIndex] == 1u) {
                    adi_stateBase.data.allGpioVoltages
                        ->gpioVoltages_mV[adi_stateBase.currentString]
                                         [(m * BS_NR_OF_GPIOS_PER_MODULE) + registerGpioIndex] = 300.0;
                }
            }
        }
    }
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/

/*========== Externalized Static Function Test Cases ========================*/
void testADI_GetTemperatures(void) {
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        adi_stateBase.currentString = s;
        for (uint16_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint16_t registerGpioIndex = 0u; registerGpioIndex < BS_NR_OF_GPIOS_PER_MODULE; registerGpioIndex++) {
                ADI_ConvertGpioVoltageToTemperature_ExpectAndReturn(300, 250);
            }
        }
        ADI_GetTemperatures(&adi_stateBase);
        for (uint16_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint16_t registerGpioIndex = 0u; registerGpioIndex < BS_NR_OF_GPIOS_PER_MODULE; registerGpioIndex++) {
                for (uint8_t ts = 0; ts < registerGpioIndex; ts++) {
                    TEST_ASSERT_EQUAL_INT16(
                        250,
                        adi_stateBase.data.cellTemperature->cellTemperature_ddegC[adi_stateBase.currentString][m][ts]);
                }
            }
        }
    }
}

/*========== Extern Function Test Cases =====================================*/
void testADI_GetStoredTemperatureIndex(void) {
    /* Invalid index */
    TEST_ASSERT_FAIL_ASSERT(TEST_ADI_GetStoredTemperatureIndex(BS_NR_OF_GPIOS_PER_MODULE));

    /* All inputs used, indices must be equal */
    for (uint8_t c = 0; c < BS_NR_OF_GPIOS_PER_MODULE; c++) {
        TEST_ASSERT_EQUAL_UINT16(c, TEST_ADI_GetStoredTemperatureIndex(c));
    }
}
