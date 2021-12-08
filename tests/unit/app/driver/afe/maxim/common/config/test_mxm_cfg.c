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
 * @file    test_mxm_cfg.c
 * @author  foxBMS Team
 * @date    2020-06-24 (date of creation)
 * @updated 2021-12-08 (date of last update)
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Test of the mxm_cfg.c module
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockfassert.h"
#include "Mockio.h"
#include "Mockspi.h"
#include "Mockspi_cfg.h"

#include "mxm_cfg.h"

/*========== Definitions and Implementations for Unit Test ==================*/
/* SPI interface configuration for MXM communication */
SPI_INTERFACE_CONFIG_s spi_MxmInterface = {
    .pConfig  = 0,
    .pNode    = 0,
    .pGioPort = 0,
    .csPin    = 0,
};

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/

void testMXM_InitializeMonitoringPins(void) {
    /* check if the right pinset configs are called */
    IO_SetPinDirectionToOutput_Expect((uint32_t *)&MXM_17841B_GIOPORT->DIR, MXM_17841B_SHTNDL_PIN);
    IO_PinReset_Expect((uint32_t *)&MXM_17841B_GIOPORT->DOUT, MXM_17841B_SHTNDL_PIN);
    SPI_SetFunctional_Expect(NULL_PTR, SPI_PIN_SIMO, true);
    SPI_SetFunctional_IgnoreArg_pNode();
    SPI_SetFunctional_Expect(NULL_PTR, SPI_PIN_SOMI, true);
    SPI_SetFunctional_IgnoreArg_pNode();
    MXM_InitializeMonitoringPins();
}

void testMXM_ShutDownBridgeIc(void) {
    IO_PinReset_Expect((uint32_t *)&MXM_17841B_GIOPORT->DOUT, MXM_17841B_SHTNDL_PIN);
    MXM_ShutDownBridgeIc();
}

void testMXM_EnableBridgeIc(void) {
    IO_PinSet_Expect((uint32_t *)&MXM_17841B_GIOPORT->DOUT, MXM_17841B_SHTNDL_PIN);
    MXM_EnableBridgeIc();
}

void testMXM_GetSpiStateReady(void) {
    SPI_CheckInterfaceAvailable_ExpectAndReturn(spi_MxmInterface.pNode, STD_OK);
    TEST_ASSERT_EQUAL(STD_OK, MXM_GetSPIStateReady());

    SPI_CheckInterfaceAvailable_ExpectAndReturn(spi_MxmInterface.pNode, STD_NOT_OK);
    TEST_ASSERT_EQUAL(STD_NOT_OK, MXM_GetSPIStateReady());
}

void testMXM_SendData(void) {
    /* check if the message is passed properly to the HAL */
    uint16_t buffer       = 0;
    uint16_t bufferLength = 1;

    SPI_TransmitData_ExpectAndReturn(&spi_MxmInterface, &buffer, bufferLength, STD_OK);
    TEST_ASSERT_EQUAL(STD_OK, MXM_SendData(&buffer, bufferLength));
}

void testMXM_ReceiveData(void) {
    /* check if the message is passed properly to the HAL */
    uint16_t buffer       = 0;
    uint16_t bufferLength = 1;

    SPI_TransmitReceiveData_ExpectAndReturn(&spi_MxmInterface, &buffer, &buffer, bufferLength, STD_OK);
    TEST_ASSERT_EQUAL(STD_OK, MXM_ReceiveData(&buffer, &buffer, bufferLength));
}
