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
 * @file    test_nxp_mc33775a.c
 * @author  foxBMS Team
 * @date    2021-10-20 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Test of nxp_mc3377x.c
 * @details TODO
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mocknxp_mc3377x-ll.h"
#include "Mocknxp_mc3377x_balancing.h"
#include "Mocknxp_mc3377x_cfg.h"
#include "Mocknxp_mc3377x_database.h"
#include "Mocknxp_mc3377x_helpers.h"
#include "Mocknxp_mc3377x_i2c.h"
#include "Mocknxp_mc3377x_measurement.h"
#include "Mocknxp_mc3377x_mux.h"
#include "Mockos.h"

#include "nxp_mc3377x.h"
#include "nxp_mc3377x_reg_def.h"
#include "spi_cfg-helper.h"

/* clang-format off */
#include "test_assert_helper.h"
/* clang-format on */

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("nxp_mc3377x.c")

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

/** SPI data configuration struct for NXP MC3377X communication, Rx part */
static spiDAT1_t spi_kNxp77xDataConfigRx[BS_NR_OF_STRINGS] = {
    {.CS_HOLD = TRUE,      /* If true, HW chip select kept active */
     .WDEL    = TRUE,      /* Activation of delay between words */
     .DFSEL   = SPI_FMT_0, /* Data word format selection */
     .CSNR    = SPI_HARDWARE_CHIP_SELECT_0_ACTIVE},
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

/** SPI interface configuration for N775X communication, Rx part */
SPI_INTERFACE_CONFIG_s spi_nxp77xInterfaceRx[BS_NR_OF_STRINGS] = {
    {
        .pConfig  = &spi_kNxp77xDataConfigRx[0u],
        .pNode    = spiREG4,
        .pGioPort = &(spiREG4->PC3),
        .csPin    = 0u,
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

void N77x_Initialize_Expects(N77X_STATE_s *n77xTestState, uint16_t *uid, uint16_t *readValue) {
    N77x_ResetMuxIndex_Expect(n77xTestState);

    /* From N77x_Enumerate */
    for (uint8_t i = 1; i <= BS_NR_OF_MODULES_PER_STRING; i++) {
        N77x_CommunicationWrite_Expect(
            i,
            MC3377X_SYS_MODE_OFFSET,
            (MC3377X_SYS_MODE_TARGETMODE_DEEPSLEEP_ENUM_VAL << MC3377X_SYS_MODE_TARGETMODE_POS),
            n77xTestState->pSpiTxSequence);

        N77x_Wait_Expect(N77X_T_SW_ACT_DEEP_SLEEP_MS);

        N77x_CommunicationRead_ExpectAndReturn(
            i, MC3377X_SYS_COM_CFG_OFFSET, readValue, n77xTestState, N77X_COMMUNICATION_OK);
        N77x_CommunicationWrite_Expect(
            i,
            MC3377X_SYS_COM_TO_CFG_OFFSET,
            (N77X_TIMEOUT_SWITCH << MC3377X_SYS_COM_TO_CFG_COMTODISABLE_POS) |
                (N77X_TIMEOUT_TO_SLEEP_10MS << MC3377X_SYS_COM_TO_CFG_COMTO_POS),
            n77xTestState->pSpiTxSequence);

        N77x_CommunicationReadMultiple_ExpectAndReturn(
            i, 3u, 3u, MC3377X_SYS_UID_LOW_OFFSET, uid, n77xTestState, N77X_COMMUNICATION_OK);
    }

    N77x_StartMeasurement_Expect(n77xTestState);
    N77x_InitializeI2c_Expect(n77xTestState);
    N77x_BalanceSetup_Expect(n77xTestState);
}

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
void testN77x_Enumerate(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_N77x_Enumerate(NULL_PTR));

    /* ======= Routine tests =============================================== */
    N77X_STATE_s n77xTestState = {
        .firstMeasurementMade = true,
        .pSpiTxSequence       = NULL_PTR,
    };
    uint16_t uid[3u]   = {0};
    uint16_t readValue = 0;

    /* ======= RT1/4: Test implementation */
    /* Everything ok */
    for (uint8_t i = 1; i <= BS_NR_OF_MODULES_PER_STRING; i++) {
        N77x_CommunicationWrite_Expect(
            i,
            MC3377X_SYS_MODE_OFFSET,
            (MC3377X_SYS_MODE_TARGETMODE_DEEPSLEEP_ENUM_VAL << MC3377X_SYS_MODE_TARGETMODE_POS),
            n77xTestState.pSpiTxSequence);

        N77x_Wait_Expect(N77X_T_SW_ACT_DEEP_SLEEP_MS);

        N77x_CommunicationRead_ExpectAndReturn(
            i, MC3377X_SYS_COM_CFG_OFFSET, &readValue, &n77xTestState, N77X_COMMUNICATION_OK);
        N77x_CommunicationWrite_Expect(
            i,
            MC3377X_SYS_COM_TO_CFG_OFFSET,
            (N77X_TIMEOUT_SWITCH << MC3377X_SYS_COM_TO_CFG_COMTODISABLE_POS) |
                (N77X_TIMEOUT_TO_SLEEP_10MS << MC3377X_SYS_COM_TO_CFG_COMTO_POS),
            n77xTestState.pSpiTxSequence);

        N77x_CommunicationReadMultiple_ExpectAndReturn(
            i, 3u, 3u, MC3377X_SYS_UID_LOW_OFFSET, uid, &n77xTestState, N77X_COMMUNICATION_OK);
    }

    /* ======= RT1/4: call function under test */
    TEST_ASSERT_EQUAL(STD_OK, TEST_N77x_Enumerate(&n77xTestState));

    /* ======= RT2/4: Test implementation */
    /* First communication read nok, second ok */
    for (uint8_t i = 1; i <= BS_NR_OF_MODULES_PER_STRING; i++) {
        N77x_CommunicationWrite_Expect(
            i,
            MC3377X_SYS_MODE_OFFSET,
            (MC3377X_SYS_MODE_TARGETMODE_DEEPSLEEP_ENUM_VAL << MC3377X_SYS_MODE_TARGETMODE_POS),
            n77xTestState.pSpiTxSequence);

        N77x_Wait_Expect(N77X_T_SW_ACT_DEEP_SLEEP_MS);

        uint16_t readValue = 0;
        N77x_CommunicationRead_ExpectAndReturn(
            i, MC3377X_SYS_COM_CFG_OFFSET, &readValue, &n77xTestState, N77X_COMMUNICATION_ERROR_TIMEOUT);
        N77x_Wait_Expect(N77X_T_WAKE_COM_MS);
        N77x_CommunicationRead_ExpectAndReturn(
            i, MC3377X_SYS_COM_CFG_OFFSET, &readValue, &n77xTestState, N77X_COMMUNICATION_OK);
        N77x_ResetMessageCounter_Expect((N77X_DEFAULT_CHAIN_ADDRESS << 6) + i, n77xTestState.currentString);
        N77x_CommunicationRead_ExpectAndReturn(
            i, MC3377X_SYS_VERSION_OFFSET, &readValue, &n77xTestState, N77X_COMMUNICATION_OK);

        N77x_CommunicationWrite_Expect(
            i,
            MC3377X_SYS_COM_TO_CFG_OFFSET,
            (N77X_TIMEOUT_SWITCH << MC3377X_SYS_COM_TO_CFG_COMTODISABLE_POS) |
                (N77X_TIMEOUT_TO_SLEEP_10MS << MC3377X_SYS_COM_TO_CFG_COMTO_POS),
            n77xTestState.pSpiTxSequence);

        N77x_CommunicationReadMultiple_ExpectAndReturn(
            i, 3u, 3u, MC3377X_SYS_UID_LOW_OFFSET, uid, &n77xTestState, N77X_COMMUNICATION_OK);
    }

    /* ======= RT2/4: call function under test */
    TEST_ASSERT_EQUAL(STD_OK, TEST_N77x_Enumerate(&n77xTestState));

    /* ======= RT3/4: Test implementation */
    /* First and second communication read nok */
    for (uint8_t i = 1; i <= BS_NR_OF_MODULES_PER_STRING; i++) {
        N77x_CommunicationWrite_Expect(
            i,
            MC3377X_SYS_MODE_OFFSET,
            (MC3377X_SYS_MODE_TARGETMODE_DEEPSLEEP_ENUM_VAL << MC3377X_SYS_MODE_TARGETMODE_POS),
            n77xTestState.pSpiTxSequence);

        N77x_Wait_Expect(N77X_T_SW_ACT_DEEP_SLEEP_MS);

        uint16_t readValue = 0;
        N77x_CommunicationRead_ExpectAndReturn(
            i, MC3377X_SYS_COM_CFG_OFFSET, &readValue, &n77xTestState, N77X_COMMUNICATION_ERROR_TIMEOUT);
        N77x_Wait_Expect(N77X_T_WAKE_COM_MS);
        N77x_CommunicationRead_ExpectAndReturn(
            i, MC3377X_SYS_COM_CFG_OFFSET, &readValue, &n77xTestState, N77X_COMMUNICATION_ERROR_TIMEOUT);
        N77x_CommunicationWrite_Expect(
            (0u << 6u) + 0u,
            MC3377X_SYS_COM_CFG_OFFSET,
            i + (N77X_DEFAULT_CHAIN_ADDRESS << 6) +
                (MC3377X_SYS_COM_CFG_BUSFW_ENABLED_ENUM_VAL << MC3377X_SYS_COM_CFG_BUSFW_POS),
            n77xTestState.pSpiTxSequence);

        N77x_ResetMessageCounter_Expect((N77X_DEFAULT_CHAIN_ADDRESS << 6) + i, n77xTestState.currentString);
        N77x_CommunicationRead_ExpectAndReturn(
            i, MC3377X_SYS_VERSION_OFFSET, &readValue, &n77xTestState, N77X_COMMUNICATION_OK);

        N77x_CommunicationWrite_Expect(
            i,
            MC3377X_SYS_COM_TO_CFG_OFFSET,
            (N77X_TIMEOUT_SWITCH << MC3377X_SYS_COM_TO_CFG_COMTODISABLE_POS) |
                (N77X_TIMEOUT_TO_SLEEP_10MS << MC3377X_SYS_COM_TO_CFG_COMTO_POS),
            n77xTestState.pSpiTxSequence);

        N77x_CommunicationReadMultiple_ExpectAndReturn(
            i, 3u, 3u, MC3377X_SYS_UID_LOW_OFFSET, uid, &n77xTestState, N77X_COMMUNICATION_OK);
    }

    /* ======= RT3/4: call function under test */
    TEST_ASSERT_EQUAL(STD_OK, TEST_N77x_Enumerate(&n77xTestState));

    /* ======= RT4/4: Test implementation */
    /* First, third and fourth communication read nok, second ok */
    for (uint8_t i = 1; i <= BS_NR_OF_MODULES_PER_STRING; i++) {
        N77x_CommunicationWrite_Expect(
            i,
            MC3377X_SYS_MODE_OFFSET,
            (MC3377X_SYS_MODE_TARGETMODE_DEEPSLEEP_ENUM_VAL << MC3377X_SYS_MODE_TARGETMODE_POS),
            n77xTestState.pSpiTxSequence);

        N77x_Wait_Expect(N77X_T_SW_ACT_DEEP_SLEEP_MS);

        uint16_t readValue = 0;
        N77x_CommunicationRead_ExpectAndReturn(
            i, MC3377X_SYS_COM_CFG_OFFSET, &readValue, &n77xTestState, N77X_COMMUNICATION_ERROR_TIMEOUT);
        N77x_Wait_Expect(N77X_T_WAKE_COM_MS);
        N77x_CommunicationRead_ExpectAndReturn(
            i, MC3377X_SYS_COM_CFG_OFFSET, &readValue, &n77xTestState, N77X_COMMUNICATION_OK);
        N77x_ResetMessageCounter_Expect((N77X_DEFAULT_CHAIN_ADDRESS << 6) + i, n77xTestState.currentString);
        N77x_CommunicationRead_ExpectAndReturn(
            i, MC3377X_SYS_VERSION_OFFSET, &readValue, &n77xTestState, N77X_COMMUNICATION_ERROR_TIMEOUT);

        N77x_CommunicationWrite_Expect(
            i,
            MC3377X_SYS_COM_TO_CFG_OFFSET,
            (N77X_TIMEOUT_SWITCH << MC3377X_SYS_COM_TO_CFG_COMTODISABLE_POS) |
                (N77X_TIMEOUT_TO_SLEEP_10MS << MC3377X_SYS_COM_TO_CFG_COMTO_POS),
            n77xTestState.pSpiTxSequence);

        N77x_CommunicationReadMultiple_ExpectAndReturn(
            i, 3u, 3u, MC3377X_SYS_UID_LOW_OFFSET, uid, &n77xTestState, N77X_COMMUNICATION_ERROR_TIMEOUT);
    }

    /* ======= RT4/4: call function under test */
    TEST_ASSERT_EQUAL(STD_NOT_OK, TEST_N77x_Enumerate(&n77xTestState));
}

void testN77x_Initialize(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_N77x_Initialize(NULL_PTR));

    /* ======= Routine tests =============================================== */
    N77X_STATE_s n77xTestState = {
        .firstMeasurementMade = false,
    };
    uint16_t uid[3u]   = {0u, 0u, 0u};
    uint16_t readValue = 0;

    /* ======= RT1/1: Test implementation */
    N77x_Initialize_Expects(&n77xTestState, uid, &readValue);

    /* ======= RT1/1: call function under test */
    TEST_N77x_Initialize(&n77xTestState);
}

void testN77x_SetFirstMeasurementCycleFinished(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_N77x_SetFirstMeasurementCycleFinished(NULL_PTR));

    /* ======= Routine tests =============================================== */
    N77X_STATE_s n77xTestState = {
        .firstMeasurementMade = false,
    };

    /* ======= RT1/2: Test implementation */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    /* ======= RT1/2: call function under test */
    TEST_N77x_SetFirstMeasurementCycleFinished(&n77xTestState);
    /* ======= RT1/2: test output verification */
    TEST_ASSERT_TRUE(n77xTestState.firstMeasurementMade);
}

void testN77x_IncrementStringSequence(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_N77x_IncrementStringSequence(NULL_PTR));

    /* ======= Routine tests =============================================== */
    N77X_STATE_s n77xTestState = {
        .currentString       = 1,
        .pSpiTxSequence      = spi_nxp77xInterfaceTx,
        .pSpiRxSequence      = spi_nxp77xInterfaceRx,
        .pSpiTxSequenceStart = spi_nxp77xInterfaceTx,
        .pSpiRxSequenceStart = spi_nxp77xInterfaceRx,
    };

    /* ======= RT1/1: call function under test */
    TEST_N77x_IncrementStringSequence(&n77xTestState);
    /* ======= RT1/1: test output verification */
    TEST_ASSERT_EQUAL(2, n77xTestState.currentString);
    TEST_ASSERT_EQUAL(n77xTestState.pSpiTxSequence, n77xTestState.pSpiTxSequenceStart + n77xTestState.currentString);
    TEST_ASSERT_EQUAL(n77xTestState.pSpiRxSequence, n77xTestState.pSpiRxSequenceStart + n77xTestState.currentString);
}

void testN77x_ResetStringSequence(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_N77x_ResetStringSequence(NULL_PTR));

    /* ======= Routine tests =============================================== */
    /** SPI interface configuration for N77x communication Tx part */
    SPI_INTERFACE_CONFIG_s spi_nxp77xInterfaceTx[BS_NR_OF_STRINGS] = {
        {
            .pConfig  = &spi_kNxp77xDataConfigTx[0u],
            .pNode    = spiREG1,
            .pGioPort = &(spiREG1->PC3),
            .csPin    = SPI_NXP_TX_CHIP_SELECT_PIN,
            .csType   = SPI_CHIP_SELECT_HARDWARE,
        },
    };

    /** SPI interface configuration for N77x communication, Rx part */
    SPI_INTERFACE_CONFIG_s spi_nxp77xInterfaceRx[BS_NR_OF_STRINGS] = {
        {
            .pConfig  = &spi_kNxp77xDataConfigRx[0u],
            .pNode    = spiREG4,
            .pGioPort = &(spiREG4->PC3),
            .csPin    = SPI_NXP_RX_CHIP_SELECT_PIN,
            .csType   = SPI_CHIP_SELECT_HARDWARE,
        },
    };
    N77X_STATE_s n77xTestState = {
        .currentString       = 1,
        .pSpiTxSequence      = spi_nxp77xInterfaceTx,
        .pSpiRxSequence      = spi_nxp77xInterfaceRx,
        .pSpiTxSequenceStart = spi_nxp77xInterfaceTx,
        .pSpiRxSequenceStart = spi_nxp77xInterfaceRx,
    };

    /* ======= RT1/1: call function under test */
    TEST_N77x_ResetStringSequence(&n77xTestState);
    /* ======= RT1/1: test output verification */
    TEST_ASSERT_EQUAL(0, n77xTestState.currentString);
    TEST_ASSERT_EQUAL(n77xTestState.pSpiTxSequence, n77xTestState.pSpiTxSequenceStart + n77xTestState.currentString);
    TEST_ASSERT_EQUAL(n77xTestState.pSpiRxSequence, n77xTestState.pSpiRxSequenceStart + n77xTestState.currentString);
}

void testN77x_IsFirstMeasurementCycleFinished(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(N77x_IsFirstMeasurementCycleFinished(NULL_PTR));

    /* ======= Routine tests =============================================== */
    N77X_STATE_s n77xTestState = {
        .firstMeasurementMade = true,
    };

    /* ======= RT1/1: Test implementation */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    /* ======= RT1/1: call function under test */
    TEST_ASSERT_TRUE(N77x_IsFirstMeasurementCycleFinished(&n77xTestState));
}

void testN77x_Measure(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(N77x_Measure(NULL_PTR));

    /* No routine test since the measurement runs in a logic while(1) loop, only interrupted by the OS */
}
