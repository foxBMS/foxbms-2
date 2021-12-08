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
 * @file    test_spi.c
 * @author  foxBMS Team
 * @date    2020-04-01 (date of creation)
 * @updated 2021-12-02 (date of last update)
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Tests for the spi module
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "MockHL_spi.h"
#include "MockHL_sys_dma.h"
#include "Mockio.h"
#include "Mockmcu.h"
#include "Mockos.h"

#include "dma_cfg.h"
#include "spi_cfg.h"

#include "spi.h"
#include "test_assert_helper.h"

/*========== Definitions and Implementations for Unit Test ==================*/

long FSYS_RaisePrivilege(void) {
    return 0;
}

/** mock for testing with an SPI handle */
spiBASE_t spiMockHandle = {0};

spi_config_reg_t spiMockConfigRegister = {0};

/*========== Setup and Teardown =============================================*/
void setUp(void) {
    /* make sure PC0 of config register is clean */
    spiMockConfigRegister.CONFIG_PC0 = 0;
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
/** simple API test that function guards against null pointer */
void testSPI_SetFunctionalNullPointer(void) {
    TEST_ASSERT_FAIL_ASSERT(SPI_SetFunctional(NULL_PTR, 0, false));
}

/** test intended function of SPI_SetFunctional() for setting a bit */
void testSPI_SetFunctionalTestIntendedFunctionSet(void) {
    /** fake a config register that is null and inject into function */
    spiMockConfigRegister.CONFIG_PC0 = 0;
    spi1GetConfigValue_Expect(NULL_PTR, CurrentValue);
    spi1GetConfigValue_IgnoreArg_config_reg();
    spi1GetConfigValue_ReturnThruPtr_config_reg(&spiMockConfigRegister);

    /* the function should call spiSetFunctional with a 1 at bit 10 */
    spiSetFunctional_Expect(spiREG1, ((uint32_t)1u << 10u));

    SPI_SetFunctional(spiREG1, 10, true);
}

/** test intended function of SPI_SetFunctional() for clearing a bit */
void testSPI_SetFunctionalTestIntendedFunctionClear(void) {
    /** fake a config register that is UINT32_MAX and inject into function */
    spiMockConfigRegister.CONFIG_PC0 = UINT32_MAX;
    spi1GetConfigValue_Expect(NULL_PTR, CurrentValue);
    spi1GetConfigValue_IgnoreArg_config_reg();
    spi1GetConfigValue_ReturnThruPtr_config_reg(&spiMockConfigRegister);

    /* the function should call spiSetFunctional with a 0 at bit 10 */
    spiSetFunctional_Expect(spiREG1, ~((uint32_t)1u << 10u));

    SPI_SetFunctional(spiREG1, 10, false);
}

/** test usage of right API functions for SPI1 */
void testSPI_SetFunctionalRightApiSpi1(void) {
    /* this test will fail if another function than the intended function is
    called */
    spi1GetConfigValue_Ignore();
    spiSetFunctional_Ignore();
    SPI_SetFunctional(spiREG1, 0, false);
}

/** test usage of right API functions for SPI2 */
void testSPI_SetFunctionalRightApiSpi2(void) {
    /* this test will fail if another function than the intended function is
    called */
    spi2GetConfigValue_Ignore();
    spiSetFunctional_Ignore();
    SPI_SetFunctional(spiREG2, 0, false);
}

/** test usage of right API functions for SPI3 */
void testSPI_SetFunctionalRightApiSpi3(void) {
    /* this test will fail if another function than the intended function is
    called */
    spi3GetConfigValue_Ignore();
    spiSetFunctional_Ignore();
    SPI_SetFunctional(spiREG3, 0, false);
}

/** test usage of right API functions for SPI4 */
void testSPI_SetFunctionalRightApiSpi4(void) {
    /* this test will fail if another function than the intended function is
    called */
    spi4GetConfigValue_Ignore();
    spiSetFunctional_Ignore();
    SPI_SetFunctional(spiREG4, 0, false);
}

/** test usage of right API functions for SPI5 */
void testSPI_SetFunctionalRightApiSpi5(void) {
    /* this test will fail if another function than the intended function is
    called */
    spi5GetConfigValue_Ignore();
    spiSetFunctional_Ignore();
    SPI_SetFunctional(spiREG5, 0, false);
}

/** test invalid input to SPI_CheckInterfaceAvailable */
void testSPI_CheckInterfaceAvailableInvalidInput(void) {
    TEST_ASSERT_FAIL_ASSERT(SPI_CheckInterfaceAvailable(NULL_PTR));
}

/** test all return codes from HAL with SPI_CheckInterfaceAvailable */
void testSPI_CheckInterfaceAvailable(void) {
    SpiTxStatus_ExpectAndReturn(spiREG1, SPI_READY);
    TEST_ASSERT_EQUAL(STD_OK, SPI_CheckInterfaceAvailable(spiREG1));

    SpiTxStatus_ExpectAndReturn(spiREG1, SPI_PENDING);
    TEST_ASSERT_EQUAL(STD_NOT_OK, SPI_CheckInterfaceAvailable(spiREG1));

    SpiTxStatus_ExpectAndReturn(spiREG1, SPI_COMPLETED);
    TEST_ASSERT_EQUAL(STD_OK, SPI_CheckInterfaceAvailable(spiREG1));
}
