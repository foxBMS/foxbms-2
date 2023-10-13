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
 * @file    test_adi_ades1830.c
 * @author  foxBMS Team
 * @date    2020-08-10 (date of creation)
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
#include "Mockadi_ades1830_defs.h"
#include "Mockadi_ades183x_balancing.h"
#include "Mockadi_ades183x_defs.h"
#include "Mockadi_ades183x_diagnostic.h"
#include "Mockadi_ades183x_gpio_voltages.h"
#include "Mockadi_ades183x_helpers.h"
#include "Mockadi_ades183x_initialization.h"
#include "Mockadi_ades183x_pec.h"
#include "Mockadi_ades183x_temperatures.h"
#include "Mockadi_ades183x_voltages.h"
#include "Mockafe_plausibility.h"
#include "Mockdatabase.h"
#include "Mockdiag.h"
#include "Mockdma.h"
#include "Mockfassert.h"
#include "Mockftask.h"
#include "Mockinfinite-loop-helper.h"
#include "Mockio.h"
#include "Mockos.h"
#include "Mockpex.h"
#include "Mockqueue.h"
#include "Mockspi.h"
#include "Mockspi_cfg.h"
#include "Mocktask.h"
#include "Mocktsi.h"

#include "adi_ades183x_cfg.h"

#include "adi_ades183x.h"
#include "adi_ades183x_buffers.h"  /* use the real command config */
#include "adi_ades183x_commands.h" /* use the real buffer configuration */
#include "spi_cfg-helper.h"
#include "test_assert_helper.h"

#include <stdbool.h>
#include <stdint.h>

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("adi_ades1830_cfg.c")
TEST_SOURCE_FILE("adi_ades183x.c")

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

/** SPI data configuration struct for ADI communication */
static spiDAT1_t spi_kAdiDataConfig[BS_NR_OF_STRINGS] = {
    {                      /* struct is implemented in the TI HAL and uses uppercase true and false */
     .CS_HOLD = TRUE,      /* If true, HW chip select kept active between words */
     .WDEL    = FALSE,     /* Activation of delay between words */
     .DFSEL   = SPI_FMT_0, /* Data word format selection */
     .CSNR    = SPI_HARDWARE_CHIP_SELECT_2_ACTIVE},
};

/**
 * SPI interface configuration for ADI communication
 * This is a list of structs because of multistring
 */
SPI_INTERFACE_CONFIG_s spi_adiInterface[BS_NR_OF_STRINGS] = {
    {
        .pConfig  = &spi_kAdiDataConfig[0u],
        .pNode    = spiREG1,
        .pGioPort = &(spiREG1->PC3),
        .csPin    = 2u,
        .csType   = SPI_CHIP_SELECT_HARDWARE,
    },
};

OS_QUEUE ftsk_afeRequestQueue;

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/

void testADI_IsFirstMeasurementCycleFinished(void) {
    /* Invalid pointer test */
    TEST_ASSERT_FAIL_ASSERT(ADI_IsFirstMeasurementCycleFinished(NULL_PTR));

    adi_stateBase.firstMeasurementMade = true;
    ADI_IsFirstMeasurementCycleFinished(&adi_stateBase);
    TEST_ASSERT_TRUE(ADI_IsFirstMeasurementCycleFinished(&adi_stateBase));

    adi_stateBase.firstMeasurementMade = false;
    ADI_IsFirstMeasurementCycleFinished(&adi_stateBase);
    TEST_ASSERT_FALSE(ADI_IsFirstMeasurementCycleFinished(&adi_stateBase));
}

void testADI_MakeRequest(void) {
    AFE_REQUEST_e testRequest = AFE_START_REQUEST;
    STD_RETURN_TYPE_e queued  = STD_NOT_OK;
    OS_SendToBackOfQueue_ExpectAndReturn(ftsk_afeRequestQueue, (void *)&testRequest, ADI_QUEUE_TIMEOUT_MS, OS_SUCCESS);
    queued = ADI_MakeRequest(testRequest);
    TEST_ASSERT_EQUAL(STD_OK, queued);

    queued = STD_OK;
    OS_SendToBackOfQueue_ExpectAndReturn(ftsk_afeRequestQueue, (void *)&testRequest, ADI_QUEUE_TIMEOUT_MS, OS_FAIL);
    queued = ADI_MakeRequest(testRequest);
    TEST_ASSERT_EQUAL(STD_NOT_OK, queued);
}

void testADI_ActivateInterfaceBoard(void) {
    /* 8 pins are needed, and they are aligned after PEX_PIN10 so we make the test code short with some loop */
    const uint8_t nrOfPins = 8u;
    for (uint8_t i = 0; i < nrOfPins; i++) {
        PEX_SetPinDirectionOutput_Expect(PEX_PORT_EXPANDER3, PEX_PIN10 + i);
    }
    for (uint8_t i = 0; i < nrOfPins; i++) {
        PEX_SetPin_Expect(PEX_PORT_EXPANDER3, PEX_PIN10 + i);
    }
    ADI_ActivateInterfaceBoard();
}

void testADI_MeasurementCycleAssertValidation(void) {
    /* Invalid pointer test */
    TEST_ASSERT_FAIL_ASSERT(ADI_MeasurementCycle(NULL_PTR));
}
void testADI_MeasurementCycleNoForever(void) {
    FOREVER_ExpectAndReturn(0);
    ADI_MeasurementCycle(&adi_stateBase);
}

void testADI_MeasurementCycleMeasurementNotStartedCase0(void) {
    /* 1.   measurement not started */
    /* 1.1. request could not be retrieved - just wait */
    adi_stateBase.measurementStarted = false;
    FOREVER_ExpectAndReturn(1);
    ADI_Wait_Ignore();
    OS_ReceiveFromQueue_IgnoreAndReturn(OS_FAIL);
    ADI_MeasurementCycle(&adi_stateBase);
    TEST_ASSERT_FALSE(adi_stateBase.measurementStarted); /* start request has not been issued */
}

void testADI_MeasurementCycleMeasurementNotStartedCase1(void) {
    /* 1.   measurement not started */
    /* 1.2. request retrieved, but not the start request - therefore wait */
    adi_stateBase.measurementStarted = false;
    FOREVER_ExpectAndReturn(1);
    ADI_Wait_Ignore();
    OS_ReceiveFromQueue_IgnoreAndReturn(OS_SUCCESS);
    ADI_MeasurementCycle(&adi_stateBase);
    TEST_ASSERT_FALSE(adi_stateBase.measurementStarted); /* start request has not been issued */
}

void testADI_AccessToDatabase(void) {
    /* Invalid pointer test */
    TEST_ASSERT_FAIL_ASSERT(TEST_ADI_AccessToDatabase(NULL_PTR));

    /* Write measured data */
    DATA_Write4DataBlocks_ExpectAndReturn(
        adi_stateBase.data.cellVoltage,
        adi_stateBase.data.cellVoltageFiltered,
        adi_stateBase.data.allGpioVoltages,
        adi_stateBase.data.cellTemperature,
        STD_OK);
    /* Leave some time for other tasks */
    ADI_Wait_Ignore();
    /* Read balancing orders */
    DATA_Read1DataBlock_ExpectAndReturn(adi_stateBase.data.balancingControl, STD_OK);
    TEST_ADI_AccessToDatabase(&adi_stateBase);
}

void testADI_ProcessMeasurementNotStartedState(void) {
    AFE_REQUEST_e request      = AFE_NO_REQUEST;
    AFE_REQUEST_e requestValue = AFE_START_REQUEST;

    /* Invalid pointer test */
    TEST_ASSERT_FAIL_ASSERT(TEST_ADI_ProcessMeasurementNotStartedState(NULL_PTR, &request));
    TEST_ASSERT_FAIL_ASSERT(TEST_ADI_ProcessMeasurementNotStartedState(&adi_stateBase, NULL_PTR));

    /* No request received */
    OS_ReceiveFromQueue_IgnoreAndReturn(OS_FAIL);
    ADI_Wait_Expect(1u);
    TEST_ADI_ProcessMeasurementNotStartedState(&adi_stateBase, &request);

    /* Request received, but not start */
    requestValue = AFE_STOP_REQUEST;
    OS_ReceiveFromQueue_ExpectAndReturn(ftsk_afeRequestQueue, &request, ADI_QUEUE_TIMEOUT_MS, OS_SUCCESS);
    OS_ReceiveFromQueue_IgnoreArg_pvBuffer();
    OS_ReceiveFromQueue_ReturnThruPtr_pvBuffer(&requestValue);
    ADI_Wait_Expect(1u);
    TEST_ADI_ProcessMeasurementNotStartedState(&adi_stateBase, &request);
    TEST_ASSERT_EQUAL(AFE_STOP_REQUEST, request);

    /* Request received and is start request */
    requestValue = AFE_START_REQUEST;
    OS_ReceiveFromQueue_ExpectAndReturn(ftsk_afeRequestQueue, &request, ADI_QUEUE_TIMEOUT_MS, OS_SUCCESS);
    OS_ReceiveFromQueue_IgnoreArg_pvBuffer();
    OS_ReceiveFromQueue_ReturnThruPtr_pvBuffer(&requestValue);
    ADI_InitializeMeasurement_Expect(&adi_stateBase);
    TEST_ADI_ProcessMeasurementNotStartedState(&adi_stateBase, &request);
    TEST_ASSERT_EQUAL(AFE_START_REQUEST, request);
}

void testADI_BalanceControl(void) {
    /* Invalid pointer test */
    TEST_ASSERT_FAIL_ASSERT(TEST_ADI_BalanceControl(NULL_PTR));

    /* Test for different balancing patterns */
    /* Enum to run through all balancing tests */
    typedef enum {
        TEST_ADI_BALANCING_VALUE0,
        TEST_ADI_BALANCING_VALUE1,
        TEST_ADI_BALANCING_VALUE_E_MAX,
    } TEST_ADI_BALANCING_e;
    /* Test for 0xAA and 0x55 balancing patterns */
    for (TEST_ADI_BALANCING_e i = TEST_ADI_BALANCING_VALUE0; i < TEST_ADI_BALANCING_VALUE_E_MAX; i++) {
        for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
            adi_stateBase.currentString = s;
            /* Mocks for unmute commands */
            ADI_CopyCommandBits_Expect(adi_cmdUnmute, adi_command);
            ADI_TransmitCommand_Expect(adi_command, &adi_stateBase);

            /* actual register configuration for the specific AFE */
            ADI_DetermineBalancingRegisterConfiguration_Expect(&adi_stateBase);

            ADI_Wait_Ignore();

            /* Mocks for mute commands */
            ADI_CopyCommandBits_Expect(adi_cmdMute, adi_command);
            ADI_TransmitCommand_Expect(adi_command, &adi_stateBase);
            TEST_ADI_BalanceControl(&adi_stateBase);
        }
    }
}

void testADI_GetRequest(void) {
    AFE_REQUEST_e request      = AFE_NO_REQUEST;
    AFE_REQUEST_e requestValue = AFE_START_REQUEST;
    /* Invalid pointer test */
    TEST_ASSERT_FAIL_ASSERT(TEST_ADI_GetRequest(NULL_PTR));

    /* Request received from queue: function must return STD_OK */
    OS_ReceiveFromQueue_IgnoreAndReturn(OS_SUCCESS);
    TEST_ASSERT_EQUAL(STD_OK, TEST_ADI_GetRequest(&request));
    /* No request received from queue: function must return STD_NOT_OK */
    OS_ReceiveFromQueue_IgnoreAndReturn(OS_FAIL);
    TEST_ASSERT_EQUAL(STD_NOT_OK, TEST_ADI_GetRequest(&request));

    /* Test setting value of request when a request is present in queue */
    OS_ReceiveFromQueue_ExpectAndReturn(ftsk_afeRequestQueue, &request, ADI_QUEUE_TIMEOUT_MS, OS_SUCCESS);
    OS_ReceiveFromQueue_IgnoreArg_pvBuffer();
    OS_ReceiveFromQueue_ReturnThruPtr_pvBuffer(&requestValue);
    TEST_ASSERT_EQUAL(AFE_NO_REQUEST, request);
    TEST_ASSERT_EQUAL(STD_OK, TEST_ADI_GetRequest(&request));
    TEST_ASSERT_EQUAL(AFE_START_REQUEST, request);
}
