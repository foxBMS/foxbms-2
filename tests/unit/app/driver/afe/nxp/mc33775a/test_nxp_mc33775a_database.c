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
 * @file    test_nxp_mc33775a_database.c
 * @author  foxBMS Team
 * @date    2025-03-21 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Test of nxp_mc3377x_database.
 * @details TODO
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockdatabase.h"

#include "nxp_mc3377x_database.h"

/* clang-format off */
#include "test_assert_helper.h"
/* clang-format on */

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("nxp_mc3377x_database.c")

TEST_INCLUDE_PATH("../../src/app/driver/afe/nxp/common/mc3377x")
TEST_INCLUDE_PATH("../../src/app/driver/afe/nxp/mc33775a")
TEST_INCLUDE_PATH("../../src/app/driver/afe/nxp/mc33775a/config")
TEST_INCLUDE_PATH("../../src/app/driver/afe/nxp/mc33775a/vendor")
TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/spi")

/*========== Definitions and Implementations for Unit Test ==================*/

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/

void testN77x_InitializeDatabase(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(N77x_InitializeDatabase(NULL_PTR));

    /* ======= Routine tests =============================================== */
    static DATA_BLOCK_CELL_VOLTAGE_s n77x_cellVoltage         = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE_BASE};
    static DATA_BLOCK_CELL_TEMPERATURE_s n77x_cellTemperature = {
        .header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE_BASE};
    static DATA_BLOCK_MIN_MAX_s n77x_minMax                     = {.header.uniqueId = DATA_BLOCK_ID_MIN_MAX};
    static DATA_BLOCK_BALANCING_CONTROL_s n77x_balancingControl = {.header.uniqueId = DATA_BLOCK_ID_BALANCING_CONTROL};
    static N77X_ERROR_TABLE_s n77x_errorTable                   = {0};

    /*========== Extern Constant and Variable Definitions =======================*/

    N77X_STATE_s n77xTestState = {
        .n77xData.cellVoltage      = &n77x_cellVoltage,
        .n77xData.cellTemperature  = &n77x_cellTemperature,
        .n77xData.minMax           = &n77x_minMax,
        .n77xData.balancingControl = &n77x_balancingControl,
        .n77xData.errorTable       = &n77x_errorTable,
    };

    /* ======= RT1/1 ======= */
    DATA_Write4DataBlocks_ExpectAndReturn(
        n77xTestState.n77xData.cellVoltage,
        n77xTestState.n77xData.cellTemperature,
        n77xTestState.n77xData.minMax,
        n77xTestState.n77xData.balancingControl,
        STD_OK);
    TEST_ASSERT_PASS_ASSERT(N77x_InitializeDatabase(&n77xTestState));

    TEST_ASSERT_EQUAL(0, n77xTestState.n77xData.cellVoltage->state);
    TEST_ASSERT_EQUAL(0, n77xTestState.n77xData.minMax->minimumCellVoltage_mV[0]);
    TEST_ASSERT_EQUAL(0, n77xTestState.n77xData.minMax->maximumCellVoltage_mV[0]);
    TEST_ASSERT_EQUAL(0, n77xTestState.n77xData.minMax->nrModuleMaximumCellVoltage[0]);
    TEST_ASSERT_EQUAL(0, n77xTestState.n77xData.minMax->nrModuleMaximumCellVoltage[0]);
    TEST_ASSERT_EQUAL(0, n77xTestState.n77xData.minMax->nrCellMinimumCellVoltage[0]);
    TEST_ASSERT_EQUAL(0, n77xTestState.n77xData.minMax->nrCellMaximumCellVoltage[0]);
    TEST_ASSERT_EQUAL(0, n77xTestState.n77xData.cellVoltage->cellVoltage_mV[0][0][0]);
    TEST_ASSERT_TRUE(n77xTestState.n77xData.cellVoltage->invalidCellVoltage[0][0][0]);

    TEST_ASSERT_EQUAL(0, n77xTestState.n77xData.cellTemperature->state);
    TEST_ASSERT_EQUAL(0, n77xTestState.n77xData.minMax->minimumCellVoltage_mV[0]);
    TEST_ASSERT_EQUAL(0, n77xTestState.n77xData.minMax->maximumCellVoltage_mV[0]);
    TEST_ASSERT_EQUAL(0, n77xTestState.n77xData.minMax->nrModuleMinimumTemperature[0]);
    TEST_ASSERT_EQUAL(0, n77xTestState.n77xData.minMax->nrModuleMaximumCellVoltage[0]);
    TEST_ASSERT_EQUAL(0, n77xTestState.n77xData.minMax->nrSensorMinimumTemperature[0]);
    TEST_ASSERT_EQUAL(0, n77xTestState.n77xData.minMax->nrSensorMaximumTemperature[0]);
    TEST_ASSERT_EQUAL(0, n77xTestState.n77xData.cellTemperature->cellTemperature_ddegC[0][0][0]);
    TEST_ASSERT_TRUE(n77xTestState.n77xData.cellTemperature->invalidCellTemperature[0][0][0]);

    TEST_ASSERT_FALSE(n77xTestState.n77xData.balancingControl->activateBalancing[0][0][0]);

    TEST_ASSERT_FALSE(n77xTestState.n77xData.errorTable->communicationOk[0][0]);
    TEST_ASSERT_FALSE(n77xTestState.n77xData.errorTable->noCommunicationTimeout[0][0]);
    TEST_ASSERT_FALSE(n77xTestState.n77xData.errorTable->crcIsValid[0][0]);
    TEST_ASSERT_FALSE(n77xTestState.n77xData.errorTable->mux0IsOk[0][0]);
    TEST_ASSERT_FALSE(n77xTestState.n77xData.errorTable->mux1IsOK[0][0]);
    TEST_ASSERT_FALSE(n77xTestState.n77xData.errorTable->mux2IsOK[0][0]);
    TEST_ASSERT_FALSE(n77xTestState.n77xData.errorTable->mux3IsOK[0][0]);
    TEST_ASSERT_EQUAL(0, n77xTestState.serialId[0][0]);
}
