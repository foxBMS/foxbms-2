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
 * @file    test_master_info.c
 * @author  foxBMS Team
 * @date    2020-07-09 (date of creation)
 * @updated 2024-12-20 (date of last update)
 * @version v1.8.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Tests for the master module
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockdatabase.h"
#include "Mockdiag.h"

#include "master_info.h"

/*========== Unit Testing Framework Directives ==============================*/
TEST_INCLUDE_PATH("../../src/app/engine/config")
TEST_INCLUDE_PATH("../../src/app/engine/database")
TEST_INCLUDE_PATH("../../src/app/engine/diag")
TEST_INCLUDE_PATH("../../src/app/engine/hw_info")

/*========== Definitions and Implementations for Unit Test ==================*/
#define NUM_DATA_READ_SUB_CALLS                        (1)
#define SUPPLY_VOLTAGE_CLAMP_30C_SENSE_INPUT_ADC_INDEX (6u)

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
void testMINFO_SetResetSource(void) {
    /* Set reset reason POWERON_RESET */
    MINFO_SetResetSource(POWERON_RESET);
    /* Check if reset reason is POWERON_RESET */
    TEST_ASSERT_EQUAL(POWERON_RESET, MINFO_GetResetSource());

    /* Set reset reason OSC_FAILURE_RESET */
    MINFO_SetResetSource(OSC_FAILURE_RESET);
    /* Check if reset reason is OSC_FAILURE_RESET */
    TEST_ASSERT_EQUAL(OSC_FAILURE_RESET, MINFO_GetResetSource());

    /* Set reset reason WATCHDOG_RESET */
    MINFO_SetResetSource(WATCHDOG_RESET);
    /* Check if reset reason is WATCHDOG_RESET */
    TEST_ASSERT_EQUAL(WATCHDOG_RESET, MINFO_GetResetSource());

    /* Set reset reason WATCHDOG2_RESET */
    MINFO_SetResetSource(WATCHDOG2_RESET);
    /* Check if reset reason is WATCHDOG2_RESET */
    TEST_ASSERT_EQUAL(WATCHDOG2_RESET, MINFO_GetResetSource());

    /* Set reset reason DEBUG_RESET */
    MINFO_SetResetSource(DEBUG_RESET);
    /* Check if reset reason is DEBUG_RESET */
    TEST_ASSERT_EQUAL(DEBUG_RESET, MINFO_GetResetSource());

    /* Set reset reason INTERCONNECT_RESET */
    MINFO_SetResetSource(INTERCONNECT_RESET);
    /* Check if reset reason is INTERCONNECT_RESET */
    TEST_ASSERT_EQUAL(INTERCONNECT_RESET, MINFO_GetResetSource());

    /* Set reset reason CPU0_RESET */
    MINFO_SetResetSource(CPU0_RESET);
    /* Check if reset reason is CPU0_RESET */
    TEST_ASSERT_EQUAL(CPU0_RESET, MINFO_GetResetSource());

    /* Set reset reason SW_RESET */
    MINFO_SetResetSource(SW_RESET);
    /* Check if reset reason is SW_RESET */
    TEST_ASSERT_EQUAL(SW_RESET, MINFO_GetResetSource());

    /* Set reset reason EXT_RESET */
    MINFO_SetResetSource(EXT_RESET);
    /* Check if reset reason is EXT_RESET */
    TEST_ASSERT_EQUAL(EXT_RESET, MINFO_GetResetSource());

    /* Set reset reason NO_RESET */
    MINFO_SetResetSource(NO_RESET);
    /* Check if reset reason is NO_RESET */
    TEST_ASSERT_EQUAL(NO_RESET, MINFO_GetResetSource());
}

/**
 * @brief   Iterate over a callback that supplies various scenarios and check if they work as expected
 * @details This function uses the callback #MockDATA_ReadBlock_Callback() in order to inject
 *          adc voltages into the returned database tables.
 */
STD_RETURN_TYPE_e MockDATA_ReadBlock_Callback(void *pDataToReceiver, int num_calls) {
    int32_t adcVoltage_mV = 0;

    /* determine a value depending on num_calls (has to be synchronized with test) */
    switch (num_calls) {
        case 0:
            /* Set ADC voltage to 2000mV */
            adcVoltage_mV = 2000;
            break;
        default:
            TEST_FAIL_MESSAGE("DATA_ReadBlock_Callback was called too often");
    }
    /* ENTER HIGHEST CASE NUMBER IN EXPECT; checks whether all cases are used */
    TEST_ASSERT_EQUAL_MESSAGE(0, (NUM_DATA_READ_SUB_CALLS - 1), "Check code of stub. Something does not fit.");

    if (num_calls >= NUM_DATA_READ_SUB_CALLS) {
        TEST_FAIL_MESSAGE("This stub is fishy");
    }

    /* cast to correct struct */
    ((DATA_BLOCK_ADC_VOLTAGE_s *)pDataToReceiver)
        ->adc1ConvertedVoltages_mV[SUPPLY_VOLTAGE_CLAMP_30C_SENSE_INPUT_ADC_INDEX] = adcVoltage_mV;

    return STD_OK;
}

void testMINFO_CheckSupplyVoltageClamp30c(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1: Just see if function can be called successfully */

    /* tell CMock to use our callback */
    DATA_Read1DataBlock_Stub(MockDATA_ReadBlock_Callback);

    DIAG_Handler_ExpectAndReturn(
        DIAG_ID_SUPPLY_VOLTAGE_CLAMP_30C_LOST, DIAG_EVENT_OK, DIAG_SYSTEM, 0u, DIAG_HANDLER_RETURN_OK);

    MINFO_CheckSupplyVoltageClamp30c();

    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: OK event -> database entry must not change */
}
