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
 * @file    test_nxp_mc33775a_mux.c
 * @author  foxBMS Team
 * @date    2025-03-21 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Test of nxp_mc3377x_mux.c
 * @details TODO
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mocknxp_mc3377x-ll.h"
#include "Mocknxp_mc3377x_helpers.h"
#include "Mockos.h"

#include "nxp_mc3377x_mux.h"
#include "nxp_mc3377x_reg_def.h"
#include "spi_cfg-helper.h"

/* clang-format off */
#include "test_assert_helper.h"
/* clang-format on */

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("nxp_mc3377x_mux.c")

TEST_INCLUDE_PATH("../../src/app/driver/afe/api")
TEST_INCLUDE_PATH("../../src/app/driver/afe/nxp/common/mc3377x")
TEST_INCLUDE_PATH("../../src/app/driver/afe/nxp/common/mc3377x/vendor")
TEST_INCLUDE_PATH("../../src/app/driver/afe/nxp/mc33775a")
TEST_INCLUDE_PATH("../../src/app/driver/afe/nxp/mc33775a/config")
TEST_INCLUDE_PATH("../../src/app/driver/afe/nxp/mc33775a/vendor")
TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/spi")

/*========== Definitions and Implementations for Unit Test ==================*/

/** SPI data configuration struct for NXP MC3377X communication, Tx part */
static spiDAT1_t spi_kNxp77xDataConfigTx[BS_NR_OF_STRINGS] = {
    {.CS_HOLD = TRUE,      /* If true, HW chip select kept active */
     .WDEL    = TRUE,      /* Activation of delay between words */
     .DFSEL   = SPI_FMT_0, /* Data word format selection */
     .CSNR    = SPI_HARDWARE_CHIP_SELECT_2_ACTIVE},
};

/** SPI interface configuration for N77X communication Tx part */
SPI_INTERFACE_CONFIG_s spi_nxp77xInterfaceTx[BS_NR_OF_STRINGS] = {
    {
        .pConfig  = &spi_kNxp77xDataConfigTx[0u],
        .pNode    = spiREG1,
        .pGioPort = &(spiREG1->PC3),
        .csPin    = 2u,
        .csType   = SPI_CHIP_SELECT_HARDWARE,
    },
};

/**
 * Default multiplexer measurement sequence
 * Must be adapted to the application
 */
N77X_MUX_CH_CFG_s n77x_muxSequence[N77X_MUX_SEQUENCE_LENGTH] = {
    /*  multiplexer 0 measurement */
    {
        .muxId      = 0,
        .muxChannel = 0,
    },
    {
        .muxId      = 0,
        .muxChannel = 1,
    },
    {
        .muxId      = 0,
        .muxChannel = 2,
    },
    {
        .muxId      = 0,
        .muxChannel = 3,
    },
    {
        .muxId      = 0,
        .muxChannel = 4,
    },
    {
        .muxId      = 0,
        .muxChannel = 5,
    },
    {
        .muxId      = 0,
        .muxChannel = 6,
    },
    {
        .muxId      = 0,
        .muxChannel = 7,
    },
};

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/

void testN77x_IncrementMuxIndex(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(N77x_IncrementMuxIndex(NULL_PTR));

    /* ======= Routine tests =============================================== */
    N77X_STATE_s n77xTestState = {
        .currentString     = 0u,
        .pMuxSequenceStart = n77x_muxSequence,
        .pMuxSequence      = n77x_muxSequence,
        .currentMux        = 0,
    };

    /* ======= RT1/2: Test implementation */
    n77xTestState.currentMux[n77xTestState.currentString] = 0u;
    /* ======= RT1/2: call function under test */
    TEST_ASSERT_PASS_ASSERT(N77x_IncrementMuxIndex(&n77xTestState));
    /* ======= RT1/2: test output verification */
    TEST_ASSERT_EQUAL(1, n77xTestState.currentMux[0]);

    /* ======= RT2/2: Test implementation */
    n77xTestState.currentMux[n77xTestState.currentString] = N77X_MUX_SEQUENCE_LENGTH;
    /* ======= RT2/2: call function under test */
    TEST_ASSERT_PASS_ASSERT(N77x_IncrementMuxIndex(&n77xTestState));
    /* ======= RT2/2: test output verification */
    TEST_ASSERT_EQUAL(0u, n77xTestState.currentMux[n77xTestState.currentString]);
}

void testN77x_ResetMuxIndex(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(N77x_ResetMuxIndex(NULL_PTR));

    /* ======= Routine tests =============================================== */
    N77X_STATE_s n77xTestState = {
        .currentString     = 0u,
        .pMuxSequenceStart = n77x_muxSequence,
        .pMuxSequence      = {0},
        .currentMux        = 1u,
    };
    /* ======= RT1/1 ======= */
    TEST_ASSERT_PASS_ASSERT(N77x_ResetMuxIndex(&n77xTestState));
    TEST_ASSERT_EQUAL(0, n77xTestState.currentMux[0]);
    TEST_ASSERT_EQUAL(n77xTestState.pMuxSequence[0], n77xTestState.pMuxSequenceStart[0]);
}

void testN77x_SetMuxChannel(void) {
    static N77X_ERROR_TABLE_s n77x_errorTable = {0};
    N77X_STATE_s n77xTestState                = {
                       .currentString       = 0u,
                       .pMuxSequenceStart   = n77x_muxSequence,
                       .pMuxSequence        = n77x_muxSequence,
                       .pSpiTxSequence      = spi_nxp77xInterfaceTx,
                       .n77xData.errorTable = &n77x_errorTable,
    };
    uint16_t readValue = 0u;

    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(N77x_SetMuxChannel(NULL_PTR));
    /* ======= AT1/1 ======= */
    n77xTestState.pMuxSequence[0]->muxId = 4;
    TEST_ASSERT_FAIL_ASSERT(N77x_SetMuxChannel(&n77xTestState));
    n77xTestState.pMuxSequence[0]->muxId = 0;

    /* ======= Routine tests =============================================== */

    /* ======= RT1/5 ======= */
    /* Everything ok */
    N77x_CommunicationWrite_Expect(
        N77X_BROADCAST_ADDRESS, MC3377X_I2C_DATA0_OFFSET, 0x0198u, n77xTestState.pSpiTxSequence);
    N77x_CommunicationWrite_Expect(
        N77X_BROADCAST_ADDRESS, MC3377X_I2C_DATA1_OFFSET, 0x0099u, n77xTestState.pSpiTxSequence);
    N77x_CommunicationWrite_Expect(
        N77X_BROADCAST_ADDRESS, MC3377X_I2C_CTRL_OFFSET, 0x0214u, n77xTestState.pSpiTxSequence);
    N77x_CommunicationRead_ExpectAndReturn(
        BS_NR_OF_MODULES_PER_STRING, MC3377X_I2C_STAT_OFFSET, &readValue, &n77xTestState, N77X_COMMUNICATION_OK);
    N77x_Wait_Expect(2u);

    N77x_CommunicationRead_ExpectAndReturn(
        BS_NR_OF_MODULES_PER_STRING, MC3377X_I2C_DATA1_OFFSET, &readValue, &n77xTestState, N77X_COMMUNICATION_OK);

    TEST_ASSERT_PASS_ASSERT(N77x_SetMuxChannel(&n77xTestState));

    /* ======= RT2/5 ======= */
    /* Disable all channels */
    n77xTestState.pMuxSequence[0]->muxChannel = 0xFF;

    N77x_CommunicationWrite_Expect(
        N77X_BROADCAST_ADDRESS, MC3377X_I2C_DATA0_OFFSET, 0x0098u, n77xTestState.pSpiTxSequence);
    N77x_CommunicationWrite_Expect(
        N77X_BROADCAST_ADDRESS, MC3377X_I2C_DATA1_OFFSET, 0x0099u, n77xTestState.pSpiTxSequence);
    N77x_CommunicationWrite_Expect(
        N77X_BROADCAST_ADDRESS, MC3377X_I2C_CTRL_OFFSET, 0x0214u, n77xTestState.pSpiTxSequence);
    N77x_CommunicationRead_ExpectAndReturn(
        BS_NR_OF_MODULES_PER_STRING, MC3377X_I2C_STAT_OFFSET, &readValue, &n77xTestState, N77X_COMMUNICATION_OK);
    N77x_Wait_Expect(2u);

    N77x_CommunicationRead_ExpectAndReturn(
        BS_NR_OF_MODULES_PER_STRING, MC3377X_I2C_DATA1_OFFSET, &readValue, &n77xTestState, N77X_COMMUNICATION_OK);

    TEST_ASSERT_PASS_ASSERT(N77x_SetMuxChannel(&n77xTestState));
    n77xTestState.pMuxSequence[0]->muxChannel = 0u;

    /* ======= RT3/5 ======= */
    /* Communication error */
    N77x_CommunicationWrite_Expect(
        N77X_BROADCAST_ADDRESS, MC3377X_I2C_DATA0_OFFSET, 0x0198u, n77xTestState.pSpiTxSequence);
    N77x_CommunicationWrite_Expect(
        N77X_BROADCAST_ADDRESS, MC3377X_I2C_DATA1_OFFSET, 0x0099u, n77xTestState.pSpiTxSequence);
    N77x_CommunicationWrite_Expect(
        N77X_BROADCAST_ADDRESS, MC3377X_I2C_CTRL_OFFSET, 0x0214u, n77xTestState.pSpiTxSequence);
    N77x_CommunicationRead_ExpectAndReturn(
        BS_NR_OF_MODULES_PER_STRING,
        MC3377X_I2C_STAT_OFFSET,
        &readValue,
        &n77xTestState,
        N77X_COMMUNICATION_ERROR_TIMEOUT);
    N77x_Wait_Expect(2u);

    TEST_ASSERT_PASS_ASSERT(N77x_SetMuxChannel(&n77xTestState));

    /* ======= RT4/5 ======= */
    /* Pending flag set on first iteration */
    uint16_t readValue_pending = 0x1u;

    N77x_CommunicationWrite_Expect(
        N77X_BROADCAST_ADDRESS, MC3377X_I2C_DATA0_OFFSET, 0x0198u, n77xTestState.pSpiTxSequence);
    N77x_CommunicationWrite_Expect(
        N77X_BROADCAST_ADDRESS, MC3377X_I2C_DATA1_OFFSET, 0x0099u, n77xTestState.pSpiTxSequence);
    N77x_CommunicationWrite_Expect(
        N77X_BROADCAST_ADDRESS, MC3377X_I2C_CTRL_OFFSET, 0x0214u, n77xTestState.pSpiTxSequence);
    N77x_CommunicationRead_ExpectAndReturn(
        BS_NR_OF_MODULES_PER_STRING, MC3377X_I2C_STAT_OFFSET, &readValue, &n77xTestState, N77X_COMMUNICATION_OK);
    N77x_CommunicationRead_ReturnThruPtr_pValue(&readValue_pending);
    N77x_Wait_Expect(2u);
    N77x_CommunicationRead_ExpectAndReturn(
        BS_NR_OF_MODULES_PER_STRING,
        MC3377X_I2C_STAT_OFFSET,
        &readValue_pending,
        &n77xTestState,
        N77X_COMMUNICATION_OK);
    N77x_CommunicationRead_ReturnThruPtr_pValue(&readValue);
    N77x_Wait_Expect(2u);

    N77x_CommunicationRead_ExpectAndReturn(
        BS_NR_OF_MODULES_PER_STRING, MC3377X_I2C_DATA1_OFFSET, &readValue, &n77xTestState, N77X_COMMUNICATION_OK);

    TEST_ASSERT_PASS_ASSERT(N77x_SetMuxChannel(&n77xTestState));

    /* ======= RT5/5 ======= */
    /* Second communication error */
    N77x_CommunicationWrite_Expect(
        N77X_BROADCAST_ADDRESS, MC3377X_I2C_DATA0_OFFSET, 0x0198u, n77xTestState.pSpiTxSequence);
    N77x_CommunicationWrite_Expect(
        N77X_BROADCAST_ADDRESS, MC3377X_I2C_DATA1_OFFSET, 0x0099u, n77xTestState.pSpiTxSequence);
    N77x_CommunicationWrite_Expect(
        N77X_BROADCAST_ADDRESS, MC3377X_I2C_CTRL_OFFSET, 0x0214u, n77xTestState.pSpiTxSequence);
    N77x_CommunicationRead_ExpectAndReturn(
        BS_NR_OF_MODULES_PER_STRING, MC3377X_I2C_STAT_OFFSET, &readValue, &n77xTestState, N77X_COMMUNICATION_OK);
    N77x_Wait_Expect(2u);

    N77x_CommunicationRead_ExpectAndReturn(
        BS_NR_OF_MODULES_PER_STRING,
        MC3377X_I2C_DATA1_OFFSET,
        &readValue,
        &n77xTestState,
        N77X_COMMUNICATION_ERROR_TIMEOUT);

    TEST_ASSERT_PASS_ASSERT(N77x_SetMuxChannel(&n77xTestState));
}
