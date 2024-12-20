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
 * @file    test_soa.c
 * @author  foxBMS Team
 * @date    2020-04-01 (date of creation)
 * @updated 2024-12-20 (date of last update)
 * @version v1.8.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Tests for SOA module responsible for the current, voltage and
 *          temperature checking of the safe operating area.
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockbms.h"
#include "Mockcontactor.h"
#include "Mockdatabase.h"
#include "Mockdiag.h"
#include "Mockfoxmath.h"
#include "Mocksoa_cfg.h"

#include "soa.h"
#include "test_assert_helper.h"

/*========== Unit Testing Framework Directives ==============================*/
TEST_INCLUDE_PATH("../../src/app/application/bms")
TEST_INCLUDE_PATH("../../src/app/application/soa")
TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/contactor")
TEST_INCLUDE_PATH("../../src/app/driver/foxmath")
TEST_INCLUDE_PATH("../../src/app/driver/sps")
TEST_INCLUDE_PATH("../../src/app/engine/diag")
TEST_INCLUDE_PATH("../../src/app/task/config")

/*========== Definitions and Implementations for Unit Test ==================*/

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
/**
 * @brief   Testing function SOA_CheckVoltages
 * @details The following cases will be tested:
 *          - Argument validation:
 *            - AT1/1: NULL_PTR for pMinimumMaximumCellVoltages &rarr; assert
 *          - Routine validation:
 *            - RT1/x: TODO
 */
void testSOA_CheckVoltages(void) {
    /* ======= Assertion tests ============================================= */
    DATA_BLOCK_MIN_MAX_s pMinimumMaximumCellVoltages = {.header.uniqueId = DATA_BLOCK_ID_MIN_MAX};
    /* ======= AT1/1: Assertion test */
    TEST_ASSERT_FAIL_ASSERT(SOA_CheckVoltages(NULL_PTR));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/x: Test implementation */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        DIAG_Handler_ExpectAndReturn(
            DIAG_ID_CELL_VOLTAGE_OVERVOLTAGE_MSL, DIAG_EVENT_OK, DIAG_STRING, s, DIAG_HANDLER_RETURN_OK);
        DIAG_Handler_ExpectAndReturn(
            DIAG_ID_CELL_VOLTAGE_OVERVOLTAGE_RSL, DIAG_EVENT_OK, DIAG_STRING, s, DIAG_HANDLER_RETURN_OK);
        DIAG_Handler_ExpectAndReturn(
            DIAG_ID_CELL_VOLTAGE_OVERVOLTAGE_MOL, DIAG_EVENT_OK, DIAG_STRING, s, DIAG_HANDLER_RETURN_OK);

        DIAG_Handler_ExpectAndReturn(
            DIAG_ID_CELL_VOLTAGE_UNDERVOLTAGE_MOL, DIAG_EVENT_NOT_OK, DIAG_STRING, s, DIAG_HANDLER_RETURN_OK);
        DIAG_Handler_ExpectAndReturn(
            DIAG_ID_CELL_VOLTAGE_UNDERVOLTAGE_RSL, DIAG_EVENT_NOT_OK, DIAG_STRING, s, DIAG_HANDLER_RETURN_OK);
        DIAG_Handler_ExpectAndReturn(
            DIAG_ID_CELL_VOLTAGE_UNDERVOLTAGE_MSL, DIAG_EVENT_NOT_OK, DIAG_STRING, s, DIAG_HANDLER_RETURN_OK);
    }
    /* ======= RT1/1: call function under test */
    SOA_CheckVoltages(&pMinimumMaximumCellVoltages);

    /* ======= RT1/1: test output verification */
    /* TODO */
}

/**
 * @brief   Testing function SOA_CheckTemperatures
 * @details The following cases will be tested:
 *          - Argument validation:
 *            - AT1/2: NULL_PTR for pMinimumMaximumCellTemperatures &rarr; assert
 *            - AT2/2: NULL_PTR for pCurrent &rarr; assert
 *          - Routine validation:
 *            - RT1/x: TODO
 */
void testSOA_CheckTemperatures(void) {
    /* ======= Assertion tests ============================================= */
    DATA_BLOCK_MIN_MAX_s pMinimumMaximumCellVoltages = {.header.uniqueId = DATA_BLOCK_ID_MIN_MAX};
    DATA_BLOCK_PACK_VALUES_s pCurrent                = {.header.uniqueId = DATA_BLOCK_ID_PACK_VALUES};
    int32_t i_current;

    /* ======= AT1/2: Assertion test */
    TEST_ASSERT_FAIL_ASSERT(SOA_CheckTemperatures(NULL_PTR, &pCurrent));
    /* ======= AT2/2: Assertion test */
    TEST_ASSERT_FAIL_ASSERT(SOA_CheckTemperatures(&pMinimumMaximumCellVoltages, NULL_PTR));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/x: Test implementation */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        i_current = pCurrent.stringCurrent_mA[s];
        BMS_GetCurrentFlowDirection_ExpectAndReturn(i_current, BMS_AT_REST);
        DIAG_Handler_ExpectAndReturn(
            DIAG_ID_TEMP_OVERTEMPERATURE_CHARGE_MSL, DIAG_EVENT_OK, DIAG_STRING, s, DIAG_HANDLER_RETURN_OK);
        DIAG_Handler_ExpectAndReturn(
            DIAG_ID_TEMP_OVERTEMPERATURE_CHARGE_RSL, DIAG_EVENT_OK, DIAG_STRING, s, DIAG_HANDLER_RETURN_OK);
        DIAG_Handler_ExpectAndReturn(
            DIAG_ID_TEMP_OVERTEMPERATURE_CHARGE_MOL, DIAG_EVENT_OK, DIAG_STRING, s, DIAG_HANDLER_RETURN_OK);
        BMS_GetCurrentFlowDirection_ExpectAndReturn(i_current, BMS_AT_REST);
        DIAG_Handler_ExpectAndReturn(
            DIAG_ID_TEMP_UNDERTEMPERATURE_CHARGE_MSL, DIAG_EVENT_OK, DIAG_STRING, s, DIAG_HANDLER_RETURN_OK);
        DIAG_Handler_ExpectAndReturn(
            DIAG_ID_TEMP_UNDERTEMPERATURE_CHARGE_RSL, DIAG_EVENT_OK, DIAG_STRING, s, DIAG_HANDLER_RETURN_OK);
        DIAG_Handler_ExpectAndReturn(
            DIAG_ID_TEMP_UNDERTEMPERATURE_CHARGE_MOL, DIAG_EVENT_OK, DIAG_STRING, s, DIAG_HANDLER_RETURN_OK);
    }
    /* ======= RT1/1: call function under test */
    SOA_CheckTemperatures(&pMinimumMaximumCellVoltages, &pCurrent);

    /* ======= RT1/1: test output verification */
    /* TODO */
}
