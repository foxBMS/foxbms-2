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
 * @file    test_main.c
 * @author  foxBMS Team
 * @date    2020-04-01 (date of creation)
 * @updated 2024-08-08 (date of last update)
 * @version v1.7.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Tests for the CAN driver
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "MockHL_adc.h"
#include "MockHL_crc.h"
#include "MockHL_etpwm.h"
#include "MockHL_gio.h"
#include "MockHL_het.h"
#include "MockHL_pinmux.h"
#include "MockHL_sys_core.h"
#include "MockHL_system.h" /* getResetSource */
#include "Mockadc.h"
#include "Mockchecksum.h"
#include "Mockdiag.h"
#include "Mockdiag_cfg.h"
#include "Mockdma.h"
#include "Mockfoxmath.h"
#include "Mocki2c.h"
#include "Mockled.h"
#include "Mockmaster_info.h"
#include "Mockos.h"
#include "Mockpwm.h"
#include "Mockspi.h"

/* clang-format off */
#include "general.h"
/* clang-format on */

#include "main.h"
#include "test_assert_helper.h"

/*========== Unit Testing Framework Directives ==============================*/
TEST_INCLUDE_PATH("../../src/app/driver/adc")
TEST_INCLUDE_PATH("../../src/app/driver/checksum")
TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/dma")
TEST_INCLUDE_PATH("../../src/app/driver/foxmath")
TEST_INCLUDE_PATH("../../src/app/driver/i2c")
TEST_INCLUDE_PATH("../../src/app/driver/io")
TEST_INCLUDE_PATH("../../src/app/driver/led")
TEST_INCLUDE_PATH("../../src/app/driver/pwm")
TEST_INCLUDE_PATH("../../src/app/driver/spi")
TEST_INCLUDE_PATH("../../src/app/engine/diag")
TEST_INCLUDE_PATH("../../src/app/engine/hw_info")

/*========== Definitions and Implementations for Unit Test ==================*/

/* required for mocking of the OS module */
volatile OS_BOOT_STATE_e os_boot = OS_OFF;
uint32_t os_schedulerStartTime   = 0u;
/* required for mocking of the diagnosis module */
#define TEST_DIAG_ID_MAX_FOR_INIT (1u)
#define TEST_DIAG_ID_MAX          (1u)

extern void TEST_DummyCallback(
    DIAG_ID_e diagId,
    DIAG_EVENT_e event,
    const DIAG_DATABASE_SHIM_s *const kpkDiagShim,
    uint32_t data) {
    FAS_ASSERT(diagId < TEST_DIAG_ID_MAX);
    FAS_ASSERT((event == DIAG_EVENT_OK) || (event == DIAG_EVENT_NOT_OK) || (event == DIAG_EVENT_RESET));
    FAS_ASSERT(kpkDiagShim != NULL_PTR);
    /* AXIVION Routine Generic-MissingParameterAssert: data: parameter accept whole range */
    (void)data; /* parameter is unused on purpose */
}

DIAG_ID_CFG_s diag_diagnosisIdConfiguration[] = {
    /* clang-format off */
    {DIAG_ID_SYSTEM_MONITORING, DIAG_ERROR_SENSITIVITY_FIRST_EVENT, DIAG_FATAL_ERROR, DIAG_NO_DELAY, DIAG_RECORDING_ENABLED, DIAG_EVALUATION_ENABLED, TEST_DummyCallback},
    /* clang-format on */
};

DIAG_DEV_s diag_device = {
    .nrOfConfiguredDiagnosisEntries   = sizeof(diag_diagnosisIdConfiguration) / sizeof(DIAG_ID_CFG_s),
    .pConfigurationOfDiagnosisEntries = &diag_diagnosisIdConfiguration[0],
    .numberOfFatalErrors              = 0u,
    .pFatalErrorLinkTable             = {GEN_REPEAT_U(NULL_PTR, GEN_STRIP(TEST_DIAG_ID_MAX_FOR_INIT))},
};

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/

void testMain(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/3 ======= */
    getResetSource_ExpectAndReturn(POWERON_RESET);
    resetSource_t resetSource = POWERON_RESET;
    MINFO_SetResetSource_Expect(resetSource);
    _enable_IRQ_interrupt__Expect();
    muxInit_Expect();
    gioInit_Expect();
    SPI_Initialize_Expect();
    adcInit_Expect();
    hetInit_Expect();
    etpwmInit_Expect();
    crcInit_Expect();
    LED_SetDebugLed_Expect();
    I2C_Initialize_Expect();
    DMA_Initialize_Expect();
    PWM_Initialize_Expect();
    DIAG_Initialize_ExpectAndReturn(&diag_device, STD_OK);
    MATH_StartupSelfTest_Expect();
    OS_CheckTimeHasPassedSelfTest_ExpectAndReturn(STD_NOT_OK);
    TEST_ASSERT_FAIL_ASSERT(unit_test_main());

    /* ======= AT2/3 ======= */
    getResetSource_ExpectAndReturn(POWERON_RESET);
    resetSource = POWERON_RESET;
    MINFO_SetResetSource_Expect(resetSource);
    _enable_IRQ_interrupt__Expect();
    muxInit_Expect();
    gioInit_Expect();
    SPI_Initialize_Expect();
    adcInit_Expect();
    hetInit_Expect();
    etpwmInit_Expect();
    crcInit_Expect();
    LED_SetDebugLed_Expect();
    I2C_Initialize_Expect();
    DMA_Initialize_Expect();
    PWM_Initialize_Expect();
    DIAG_Initialize_ExpectAndReturn(&diag_device, STD_OK);
    MATH_StartupSelfTest_Expect();
    OS_CheckTimeHasPassedSelfTest_ExpectAndReturn(STD_OK);
    OS_InitializeOperatingSystem_Expect();
    os_boot = OS_OFF; /* initialization of the OS failed */
    TEST_ASSERT_FAIL_ASSERT(unit_test_main());

    /* ======= AT3/3 ======= */
    getResetSource_ExpectAndReturn(POWERON_RESET);
    resetSource = POWERON_RESET;
    MINFO_SetResetSource_Expect(resetSource);
    _enable_IRQ_interrupt__Expect();
    muxInit_Expect();
    gioInit_Expect();
    SPI_Initialize_Expect();
    adcInit_Expect();
    hetInit_Expect();
    etpwmInit_Expect();
    crcInit_Expect();
    LED_SetDebugLed_Expect();
    I2C_Initialize_Expect();
    DMA_Initialize_Expect();
    PWM_Initialize_Expect();
    DIAG_Initialize_ExpectAndReturn(&diag_device, STD_OK);
    MATH_StartupSelfTest_Expect();
    OS_CheckTimeHasPassedSelfTest_ExpectAndReturn(STD_OK);
    OS_InitializeOperatingSystem_Expect();
    os_boot = OS_INIT_PRE_OS;                         /* successful initialization of the OS */
    CHK_ValidateChecksum_ExpectAndReturn(STD_NOT_OK); /* checksum failed */
    DIAG_Handler_ExpectAndReturn(
        DIAG_ID_FLASHCHECKSUM, DIAG_EVENT_NOT_OK, DIAG_SYSTEM, 0u, DIAG_HANDLER_RETURN_ERR_OCCURRED);
    TEST_ASSERT_FAIL_ASSERT(unit_test_main());

    /* ======= Routine tests =============================================== */

    /* ======= RT1/1: Test implementation */
    getResetSource_ExpectAndReturn(POWERON_RESET);
    resetSource = POWERON_RESET;
    MINFO_SetResetSource_Expect(resetSource);
    _enable_IRQ_interrupt__Expect();
    muxInit_Expect();
    gioInit_Expect();
    SPI_Initialize_Expect();
    adcInit_Expect();
    hetInit_Expect();
    etpwmInit_Expect();
    crcInit_Expect();
    LED_SetDebugLed_Expect();
    I2C_Initialize_Expect();
    DMA_Initialize_Expect();
    PWM_Initialize_Expect();
    DIAG_Initialize_ExpectAndReturn(&diag_device, STD_OK);
    MATH_StartupSelfTest_Expect();
    OS_CheckTimeHasPassedSelfTest_ExpectAndReturn(STD_OK);
    OS_InitializeOperatingSystem_Expect();
    os_boot = OS_INIT_PRE_OS;                     /* successful initialization of the OS */
    CHK_ValidateChecksum_ExpectAndReturn(STD_OK); /* checksum check successful */
    OS_GetTickCount_ExpectAndReturn(0u);
    OS_StartScheduler_Expect();
    /* ======= RT1/1: call function under test */
    int ret = unit_test_main();
    /* ======= RT1/1: test output verification */
    TEST_ASSERT_EQUAL(1, ret);
}
