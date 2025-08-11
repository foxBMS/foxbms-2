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
 * @file    test_nxp_mc33775a_measurement.c
 * @author  foxBMS Team
 * @date    2025-04-01 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Test of nxp_mc33775a_measurement.c
 * @details TODO
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockdatabase.h"
#include "Mocknxp_mc3377x-ll.h"
#include "Mocknxp_mc3377x_cfg.h"
#include "Mocknxp_mc3377x_helpers.h"

#include "nxp_mc3377x_measurement.h"
#include "nxp_mc3377x_reg_def.h"
#include "spi_cfg-helper.h"

/* clang-format off */
#include "test_assert_helper.h"
/* clang-format on */

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("nxp_mc33775a_measurement.c")

TEST_INCLUDE_PATH("../../src/app/driver/afe/api")
TEST_INCLUDE_PATH("../../src/app/driver/afe/nxp/common/mc3377x")
TEST_INCLUDE_PATH("../../src/app/driver/afe/nxp/common/mc3377x/vendor")
TEST_INCLUDE_PATH("../../src/app/driver/afe/nxp/mc33775a")
TEST_INCLUDE_PATH("../../src/app/driver/afe/nxp/mc33775a/config")
TEST_INCLUDE_PATH("../../src/app/driver/afe/nxp/mc33775a/vendor")
TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/foxmath")
TEST_INCLUDE_PATH("../../src/app/driver/spi")

/*========== Definitions and Implementations for Unit Test ==================*/

static DATA_BLOCK_CELL_VOLTAGE_s n77x_cellVoltage         = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE_BASE};
static DATA_BLOCK_CELL_TEMPERATURE_s n77x_cellTemperature = {.header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE_BASE};
static DATA_BLOCK_ALL_GPIO_VOLTAGES_s n77x_allGpioVoltage = {.header.uniqueId = DATA_BLOCK_ID_ALL_GPIO_VOLTAGES_BASE};

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
void testN77x_CaptureMeasurement(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(N77x_CaptureMeasurement(NULL_PTR));

    /* ======= Routine tests =============================================== */
    N77X_STATE_s n77xTestState = {
        .currentString            = 0u,
        .n77xData.cellTemperature = &n77x_cellTemperature,
        .n77xData.cellVoltage     = &n77x_cellVoltage,
        .n77xData.allGpioVoltage  = &n77x_allGpioVoltage,
    };
    uint16_t deviceAddress         = 0u;
    uint16_t primaryRawValues[20]  = {0u};
    uint16_t secondaryRawValues[4] = {0u};

    /* ======= RT1/1 ======= */
    /* everything ok */

    /* From N77x_InitMeasurement */
    N77x_CommunicationWrite_Expect(
        N77X_BROADCAST_ADDRESS,
        MC3377X_ALLM_APP_CTRL_OFFSET,
        (1u << MC3377X_ALLM_APP_CTRL_PAUSEBAL_POS) | (0x3FFu << MC3377X_PRMM_APP_CTRL_CAPVC_POS) |
            (MC3377X_PRMM_APP_CTRL_VCOLNUM_DISABLED_ENUM_VAL << MC3377X_PRMM_APP_CTRL_VCOLNUM_POS),
        n77xTestState.pSpiTxSequence);
    N77x_Wait_Expect(N77X_MEASUREMENT_CAPTURE_TIME_MS);
    N77x_CommunicationWrite_Expect(
        N77X_BROADCAST_ADDRESS,
        MC3377X_ALLM_APP_CTRL_OFFSET,
        (0u << MC3377X_ALLM_APP_CTRL_PAUSEBAL_POS) | (0x3FFu << MC3377X_PRMM_APP_CTRL_CAPVC_POS) |
            (MC3377X_PRMM_APP_CTRL_VCOLNUM_DISABLED_ENUM_VAL << MC3377X_PRMM_APP_CTRL_VCOLNUM_POS),
        n77xTestState.pSpiTxSequence);
    N77x_Wait_Expect(N77X_MEASUREMENT_READY_TIME_MS);

    for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
        deviceAddress = m + 1u;

        /* From N77x_RetrieveMeasurement */
        N77x_CommunicationReadMultiple_ExpectAndReturn(
            deviceAddress,
            20,
            4u,
            MC3377X_PRMM_APP_VC_CNT_OFFSET,
            primaryRawValues,
            &n77xTestState,
            N77X_COMMUNICATION_OK);
        N77x_CommunicationReadMultiple_ExpectAndReturn(
            deviceAddress,
            4,
            4u,
            MC3377X_SECM_APP_AIN4_OFFSET,
            secondaryRawValues,
            &n77xTestState,
            N77X_COMMUNICATION_OK);
        N77x_ErrorHandling_Expect(&n77xTestState, N77X_COMMUNICATION_OK, m);
        N77x_ErrorHandling_Expect(&n77xTestState, N77X_COMMUNICATION_OK, m);
        N77x_ErrorHandling_Expect(&n77xTestState, N77X_COMMUNICATION_OK, m);
        N77x_ConvertVoltagesToTemperatures_ExpectAndReturn(0, 0);
    }
    DATA_Write3DataBlocks_ExpectAndReturn(
        n77xTestState.n77xData.cellVoltage,
        n77xTestState.n77xData.cellTemperature,
        n77xTestState.n77xData.allGpioVoltage,
        STD_OK);

    TEST_ASSERT_PASS_ASSERT(N77x_CaptureMeasurement(&n77xTestState));
}

void testN77x_StartMeasurement(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(N77x_StartMeasurement(NULL_PTR));

    /* ======= Routine tests =============================================== */
    N77X_STATE_s n77xTestState = {
        .currentString            = 0u,
        .n77xData.cellTemperature = &n77x_cellTemperature,
        .n77xData.cellVoltage     = &n77x_cellVoltage,
        .n77xData.allGpioVoltage  = &n77x_allGpioVoltage,
    };

    /* ======= RT1/1 ======= */
    N77x_CommunicationWrite_Expect(
        N77X_BROADCAST_ADDRESS, MC3377X_ALLM_VCVB_CFG_OFFSET, 0x3FFF, n77xTestState.pSpiTxSequence);
    N77x_Wait_Expect(N77X_T_WAIT_CYC_SOC_MS);
    /* Enable analog inputs 0-3 and module voltage measurement */
    N77x_CommunicationWrite_Expect(
        N77X_BROADCAST_ADDRESS, MC3377X_PRMM_AIN_CFG_OFFSET, 0x1F, n77xTestState.pSpiTxSequence);
    N77x_Wait_Expect(N77X_T_WAIT_CYC_SOC_MS);
    /* Enable analog inputs 4-7 measurement */
    N77x_CommunicationWrite_Expect(
        N77X_BROADCAST_ADDRESS, MC3377X_SECM_AIN_CFG_OFFSET, 0x0F, n77xTestState.pSpiTxSequence);
    N77x_Wait_Expect(N77X_T_WAIT_CYC_SOC_MS);
    /* Set pause of balancing before measurement start, enable the measurement units simultaneously */
    N77x_CommunicationWrite_Expect(
        N77X_BROADCAST_ADDRESS,
        MC3377X_ALLM_CFG_OFFSET,
        (N77X_BALPAUSELEN_10US << MC3377X_ALLM_CFG_BALPAUSELEN_POS) | (1 << MC3377X_ALLM_CFG_MEASEN_POS),
        n77xTestState.pSpiTxSequence);
    N77x_Wait_Expect(N77X_T_WAIT_CYC_SOC_MS);

    TEST_ASSERT_PASS_ASSERT(N77x_StartMeasurement(&n77xTestState));
}

void testN77x_InitMeasurement(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_N77x_InitMeasurement(NULL_PTR));

    /* ======= Routine tests =============================================== */
    N77X_STATE_s n77xTestState = {
        .currentString            = 0u,
        .n77xData.cellTemperature = &n77x_cellTemperature,
        .n77xData.cellVoltage     = &n77x_cellVoltage,
        .n77xData.allGpioVoltage  = &n77x_allGpioVoltage,
    };

    /* ======= RT1/1 ======= */
    /* everything ok */

    /* From N77x_InitMeasurement */
    N77x_CommunicationWrite_Expect(
        N77X_BROADCAST_ADDRESS,
        MC3377X_ALLM_APP_CTRL_OFFSET,
        (1u << MC3377X_ALLM_APP_CTRL_PAUSEBAL_POS) | (0x3FFu << MC3377X_PRMM_APP_CTRL_CAPVC_POS) |
            (MC3377X_PRMM_APP_CTRL_VCOLNUM_DISABLED_ENUM_VAL << MC3377X_PRMM_APP_CTRL_VCOLNUM_POS),
        n77xTestState.pSpiTxSequence);
    N77x_Wait_Expect(N77X_MEASUREMENT_CAPTURE_TIME_MS);
    N77x_CommunicationWrite_Expect(
        N77X_BROADCAST_ADDRESS,
        MC3377X_ALLM_APP_CTRL_OFFSET,
        (0u << MC3377X_ALLM_APP_CTRL_PAUSEBAL_POS) | (0x3FFu << MC3377X_PRMM_APP_CTRL_CAPVC_POS) |
            (MC3377X_PRMM_APP_CTRL_VCOLNUM_DISABLED_ENUM_VAL << MC3377X_PRMM_APP_CTRL_VCOLNUM_POS),
        n77xTestState.pSpiTxSequence);
    N77x_Wait_Expect(N77X_MEASUREMENT_READY_TIME_MS);

    TEST_ASSERT_PASS_ASSERT(TEST_N77x_InitMeasurement(&n77xTestState));
}

void testN77x_RetrieveVoltages(void) {
    /* ======= Routine tests =============================================== */
    N77X_STATE_s n77xTestState = {
        .currentString            = 0u,
        .n77xData.cellTemperature = &n77x_cellTemperature,
        .n77xData.cellVoltage     = &n77x_cellVoltage,
        .n77xData.allGpioVoltage  = &n77x_allGpioVoltage,
    };
    uint16_t primaryRawValues[20] = {1u};
    bool gpio03Error              = false;

    /* ======= RT1/4 ======= */
    /* everything ok */
    TEST_ASSERT_PASS_ASSERT(TEST_N77x_RetrieveVoltages(&n77xTestState, 0, primaryRawValues, &gpio03Error));
    TEST_ASSERT_FALSE(n77xTestState.n77xData.cellVoltage->invalidCellVoltage[0][0][0]);

    /* ======= RT2/4 ======= */
    /* invalid cellVoltage */
    primaryRawValues[1] = N77X_INVALID_REGISTER_VALUE;
    TEST_ASSERT_PASS_ASSERT(TEST_N77x_RetrieveVoltages(&n77xTestState, 0, primaryRawValues, &gpio03Error));
    TEST_ASSERT_TRUE(n77xTestState.n77xData.cellVoltage->invalidCellVoltage[0][0][0]);

    /* ======= RT3/4 ======= */
    /* gpio03Error */
    primaryRawValues[16] = N77X_INVALID_REGISTER_VALUE;
    TEST_ASSERT_PASS_ASSERT(TEST_N77x_RetrieveVoltages(&n77xTestState, 0, primaryRawValues, &gpio03Error));
    TEST_ASSERT_TRUE(gpio03Error);

    /* ======= RT4/4 ======= */
    /* invalid moduleVoltage */
    primaryRawValues[15] = N77X_INVALID_REGISTER_VALUE;
    TEST_ASSERT_PASS_ASSERT(TEST_N77x_RetrieveVoltages(&n77xTestState, 0, primaryRawValues, &gpio03Error));
}

void testN77x_RetrieveTemperatures(void) {
    /* ======= Routine tests =============================================== */
    N77X_STATE_s n77xTestState = {
        .currentString            = 0u,
        .n77xData.cellTemperature = &n77x_cellTemperature,
        .n77xData.cellVoltage     = &n77x_cellVoltage,
        .n77xData.allGpioVoltage  = &n77x_allGpioVoltage,
    };
    bool gpio03Error = false;
    bool gpio47Error = false;

    /* ======= RT1/2 ======= */
    /* everything ok */
    N77x_ConvertVoltagesToTemperatures_ExpectAndReturn(0, 1);
    TEST_ASSERT_PASS_ASSERT(TEST_N77x_RetrieveTemperatures(&n77xTestState, 0, &gpio03Error, &gpio47Error));
    TEST_ASSERT_EQUAL(1, n77xTestState.n77xData.cellTemperature->cellTemperature_ddegC[0][0][0]);
    TEST_ASSERT_FALSE(n77xTestState.n77xData.cellTemperature->invalidCellTemperature[0][0][0]);

    /* ======= RT2/2 ======= */
    /* gpio03Error */
    gpio03Error = true;
    TEST_ASSERT_PASS_ASSERT(TEST_N77x_RetrieveTemperatures(&n77xTestState, 0, &gpio03Error, &gpio47Error));
    TEST_ASSERT_EQUAL(0, n77xTestState.n77xData.cellTemperature->cellTemperature_ddegC[0][0][0]);
    TEST_ASSERT_TRUE(n77xTestState.n77xData.cellTemperature->invalidCellTemperature[0][0][0]);
}

void testN77x_RetrieveMeasurement(void) {
    /* ======= Routine tests =============================================== */
    N77X_STATE_s n77xTestState = {
        .currentString            = 0u,
        .n77xData.cellTemperature = &n77x_cellTemperature,
        .n77xData.cellVoltage     = &n77x_cellVoltage,
        .n77xData.allGpioVoltage  = &n77x_allGpioVoltage,
    };
    uint16_t deviceAddress                  = 0u;
    uint16_t primaryRawValues[20]           = {0u};
    uint16_t secondaryRawValues[4]          = {0u};
    uint16_t secondaryRawValues_returned[4] = {0x8000u};

    /* ======= RT1/3 ======= */
    /* everything ok */
    for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
        deviceAddress = m + 1u;

        /* From N77x_RetrieveMeasurement */
        N77x_CommunicationReadMultiple_ExpectAndReturn(
            deviceAddress,
            20,
            4u,
            MC3377X_PRMM_APP_VC_CNT_OFFSET,
            primaryRawValues,
            &n77xTestState,
            N77X_COMMUNICATION_OK);
        N77x_CommunicationReadMultiple_ExpectAndReturn(
            deviceAddress,
            4,
            4u,
            MC3377X_SECM_APP_AIN4_OFFSET,
            secondaryRawValues,
            &n77xTestState,
            N77X_COMMUNICATION_OK);
        N77x_ErrorHandling_Expect(&n77xTestState, N77X_COMMUNICATION_OK, m);
        N77x_ErrorHandling_Expect(&n77xTestState, N77X_COMMUNICATION_OK, m);
        N77x_ErrorHandling_Expect(&n77xTestState, N77X_COMMUNICATION_OK, m);
        N77x_ConvertVoltagesToTemperatures_ExpectAndReturn(0, 0);
    }
    DATA_Write3DataBlocks_ExpectAndReturn(
        n77xTestState.n77xData.cellVoltage,
        n77xTestState.n77xData.cellTemperature,
        n77xTestState.n77xData.allGpioVoltage,
        STD_OK);

    TEST_ASSERT_PASS_ASSERT(TEST_N77x_RetrieveMeasurement(&n77xTestState));
    TEST_ASSERT_FALSE(n77xTestState.n77xData.cellVoltage->invalidCellVoltage[0][0][0]);
    TEST_ASSERT_FALSE(n77xTestState.n77xData.cellTemperature->invalidCellTemperature[0][0][0]);

    /* ======= RT2/3 ======= */
    /* Communication error */
    for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
        deviceAddress = m + 1u;

        /* From N77x_RetrieveMeasurement */
        N77x_CommunicationReadMultiple_ExpectAndReturn(
            deviceAddress,
            20,
            4u,
            MC3377X_PRMM_APP_VC_CNT_OFFSET,
            primaryRawValues,
            &n77xTestState,
            N77X_COMMUNICATION_ERROR_NO_RESPONSE);
        N77x_CommunicationReadMultiple_ExpectAndReturn(
            deviceAddress,
            4,
            4u,
            MC3377X_SECM_APP_AIN4_OFFSET,
            secondaryRawValues,
            &n77xTestState,
            N77X_COMMUNICATION_ERROR_NO_RESPONSE);
        N77x_ErrorHandling_Expect(&n77xTestState, N77X_COMMUNICATION_ERROR_NO_RESPONSE, m);
        N77x_ErrorHandling_Expect(&n77xTestState, N77X_COMMUNICATION_ERROR_NO_RESPONSE, m);
        N77x_ErrorHandling_Expect(&n77xTestState, N77X_COMMUNICATION_ERROR_NO_RESPONSE, m);
    }
    DATA_Write3DataBlocks_ExpectAndReturn(
        n77xTestState.n77xData.cellVoltage,
        n77xTestState.n77xData.cellTemperature,
        n77xTestState.n77xData.allGpioVoltage,
        STD_OK);

    TEST_ASSERT_PASS_ASSERT(TEST_N77x_RetrieveMeasurement(&n77xTestState));
    TEST_ASSERT_TRUE(n77xTestState.n77xData.cellVoltage->invalidCellVoltage[0][0][0]);
    TEST_ASSERT_TRUE(n77xTestState.n77xData.cellTemperature->invalidCellTemperature[0][0][0]);

    /* ======= RT3/3 ======= */
    /* gpio47Error */
    for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
        deviceAddress = m + 1u;

        /* From N77x_RetrieveMeasurement */
        N77x_CommunicationReadMultiple_ExpectAndReturn(
            deviceAddress,
            20,
            4u,
            MC3377X_PRMM_APP_VC_CNT_OFFSET,
            primaryRawValues,
            &n77xTestState,
            N77X_COMMUNICATION_OK);
        N77x_CommunicationReadMultiple_ExpectAndReturn(
            deviceAddress,
            4,
            4u,
            MC3377X_SECM_APP_AIN4_OFFSET,
            secondaryRawValues,
            &n77xTestState,
            N77X_COMMUNICATION_OK);
        N77x_CommunicationReadMultiple_ReturnThruPtr_pValues(secondaryRawValues_returned);
        N77x_ErrorHandling_Expect(&n77xTestState, N77X_COMMUNICATION_OK, m);
        N77x_ErrorHandling_Expect(&n77xTestState, N77X_COMMUNICATION_OK, m);
        N77x_ErrorHandling_Expect(&n77xTestState, N77X_COMMUNICATION_OK, m);
        N77x_ConvertVoltagesToTemperatures_ExpectAndReturn(0, 0);
    }
    DATA_Write3DataBlocks_ExpectAndReturn(
        n77xTestState.n77xData.cellVoltage,
        n77xTestState.n77xData.cellTemperature,
        n77xTestState.n77xData.allGpioVoltage,
        STD_OK);

    TEST_ASSERT_PASS_ASSERT(TEST_N77x_RetrieveMeasurement(&n77xTestState));
    TEST_ASSERT_FALSE(n77xTestState.n77xData.cellVoltage->invalidCellVoltage[0][0][0]);
    TEST_ASSERT_FALSE(n77xTestState.n77xData.cellTemperature->invalidCellTemperature[0][0][0]);
}
