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
 * @file    test_nxp_mc33775a.c
 * @author  foxBMS Team
 * @date    2021-10-20 (date of creation)
 * @updated 2024-12-20 (date of last update)
 * @version v1.8.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Test of some module
 * @details TODO
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "MockHL_gio.h"
#include "MockHL_system.h"
#include "Mockafe_dma.h"
#include "Mockdatabase.h"
#include "Mockdiag.h"
#include "Mockftask.h"
#include "Mockio.h"
#include "Mockmcu.h"
#include "Mocknxp_mc33775a-ll.h"
#include "Mocknxp_mc33775a_cfg.h"
#include "Mockos.h"
#include "Mockspi.h"

#include "MC33775A.h"
#include "foxmath.h"
#include "nxp_mc33775a.h"
#include "spi_cfg-helper.h"
#include "uc_msg_t.h"

/* clang-format off */
#include "test_assert_helper.h"
/* clang-format on */

#include <stdbool.h>
#include <stdint.h>

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("nxp_mc33775a.c")

TEST_INCLUDE_PATH("../../src/app/driver/afe/api")
TEST_INCLUDE_PATH("../../src/app/driver/afe/nxp/mc33775a")
TEST_INCLUDE_PATH("../../src/app/driver/afe/nxp/mc33775a/config")
TEST_INCLUDE_PATH("../../src/app/driver/afe/nxp/mc33775a/vendor")
TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/dma")
TEST_INCLUDE_PATH("../../src/app/driver/foxmath")
TEST_INCLUDE_PATH("../../src/app/driver/io")
TEST_INCLUDE_PATH("../../src/app/driver/rtc")
TEST_INCLUDE_PATH("../../src/app/driver/spi")
TEST_INCLUDE_PATH("../../src/app/engine/diag")
TEST_INCLUDE_PATH("../../src/app/task/config")
TEST_INCLUDE_PATH("../../src/app/task/ftask")

/*========== Definitions and Implementations for Unit Test ==================*/

/** SPI data configuration struct for NXP MC33775A communication, Tx part */
static spiDAT1_t spi_kNxp775DataConfigTx[BS_NR_OF_STRINGS] = {
    {.CS_HOLD = TRUE,      /* If true, HW chip select kept active */
     .WDEL    = TRUE,      /* Activation of delay between words */
     .DFSEL   = SPI_FMT_0, /* Data word format selection */
     .CSNR    = SPI_HARDWARE_CHIP_SELECT_2_ACTIVE},
};

/** SPI data configuration struct for NXP MC33775A communication, Rx part */
static spiDAT1_t spi_kNxp775DataConfigRx[BS_NR_OF_STRINGS] = {
    {.CS_HOLD = TRUE,      /* If true, HW chip select kept active */
     .WDEL    = TRUE,      /* Activation of delay between words */
     .DFSEL   = SPI_FMT_0, /* Data word format selection */
     .CSNR    = SPI_HARDWARE_CHIP_SELECT_0_ACTIVE},
};

/** SPI interface configuration for N775 communication Tx part */
SPI_INTERFACE_CONFIG_s spi_nxp775InterfaceTx[BS_NR_OF_STRINGS] = {
    {
        .pConfig  = &spi_kNxp775DataConfigTx[0u],
        .pNode    = spiREG1,
        .pGioPort = &(spiREG1->PC3),
        .csPin    = 2u,
        .csType   = SPI_CHIP_SELECT_HARDWARE,
    },
};

/** SPI interface configuration for N775 communication, Rx part */
SPI_INTERFACE_CONFIG_s spi_nxp775InterfaceRx[BS_NR_OF_STRINGS] = {
    {
        .pConfig  = &spi_kNxp775DataConfigRx[0u],
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
N775_MUX_CH_CFG_s n775_muxSequence[N775_MUX_SEQUENCE_LENGTH] = {
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

volatile bool ftsk_allQueuesCreated = false;
OS_QUEUE ftsk_afeToI2cQueue;
OS_QUEUE ftsk_afeFromI2cQueue;

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
void testN775_BalanceControl(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_N775_BalanceControl(NULL_PTR));

    /* ======= Routine tests =============================================== */
    DATA_BLOCK_BALANCING_CONTROL_s n775_tableBalancingControl = {.header.uniqueId = DATA_BLOCK_ID_BALANCING_CONTROL};

    N775_STATE_s n775_testState = {
        .currentString             = 0u,
        .pSpiTxSequence            = spi_nxp775InterfaceTx,
        .n775Data.balancingControl = &n775_tableBalancingControl,
    };

    /* ======= RT1/1 ======= */
    /* Only test function N775_BalanceControl */
    N775_CommunicationWrite_Expect(
        N775_BROADCAST_ADDRESS,
        MC33775_BAL_GLOB_TO_TMR_OFFSET,
        N775_GLOBAL_BALANCING_TIMER,
        n775_testState.pSpiTxSequence);

    N775_CommunicationWrite_Expect(
        N775_BROADCAST_ADDRESS, MC33775_BAL_PRE_TMR_OFFSET, N775_PRE_BALANCING_TIMER, n775_testState.pSpiTxSequence);

    N775_CommunicationWrite_Expect(
        N775_BROADCAST_ADDRESS,
        MC33775_BAL_TMR_CH_ALL_OFFSET,
        (MC33775_BAL_TMR_CH_ALL_PWM_PWM100_ENUM_VAL << MC33775_BAL_TMR_CH_ALL_PWM_POS) |
            (N775_ALL_CHANNEL_BALANCING_TIMER << MC33775_BAL_TMR_CH_ALL_BALTIME_POS),
        n775_testState.pSpiTxSequence);

    N775_CommunicationWrite_Expect(
        N775_BROADCAST_ADDRESS,
        MC33775_BAL_GLOB_CFG_OFFSET,
        (MC33775_BAL_GLOB_CFG_BALEN_ENABLED_ENUM_VAL << MC33775_BAL_GLOB_CFG_BALEN_POS) |
            (MC33775_BAL_GLOB_CFG_TMRBALEN_STOP_ENUM_VAL << MC33775_BAL_GLOB_CFG_TMRBALEN_POS),
        n775_testState.pSpiTxSequence);

    DATA_Read1DataBlock_ExpectAndReturn(n775_testState.n775Data.balancingControl, STD_OK);

    /* Activate balancing for all cells */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t cb = 0u; cb < BS_NR_OF_CELL_BLOCKS_PER_MODULE; cb++) {
                n775_testState.n775Data.balancingControl->activateBalancing[s][m][cb] = true;
            }
        }
    }

    for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
        uint8_t deviceAddress   = m + 1u;
        uint16_t balancingState = 0u;
        for (uint16_t cb = 0u; cb < BS_NR_OF_CELL_BLOCKS_PER_MODULE; cb++) {
            if (n775_testState.n775Data.balancingControl->activateBalancing[n775_testState.currentString][m][cb] ==
                true) {
                balancingState |= 1u << cb;
            }
        }
        N775_CommunicationWrite_Expect(
            deviceAddress, MC33775_BAL_CH_CFG_OFFSET, balancingState, n775_testState.pSpiTxSequence);
    }
    TEST_ASSERT_PASS_ASSERT(TEST_N775_BalanceControl(&n775_testState));
}

void testN775_BalanceSetup(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_N775_BalanceSetup(NULL_PTR))
}
void testN775_CaptureMeasurement(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_N775_CaptureMeasurement(NULL_PTR));
}
void testN775_Enumerate(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_N775_Enumerate(NULL_PTR));
}
void testN775_ErrorHandling(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_N775_ErrorHandling(NULL_PTR, N775_COMMUNICATION_OK, 0u));

    /* ======= Routine tests =============================================== */
    N775_ERROR_TABLE_s n775_errorTable = {0};

    N775_STATE_s n775TestState = {
        .currentString       = 0u,
        .n775Data.errorTable = &n775_errorTable,
    };

    /* ======= RT1/4: Test implementation */
    uint8_t currentModule = 0u;

    n775TestState.n775Data.errorTable->communicationOk[n775TestState.currentString][currentModule]        = false;
    n775TestState.n775Data.errorTable->noCommunicationTimeout[n775TestState.currentString][currentModule] = false;
    n775TestState.n775Data.errorTable->crcIsValid[n775TestState.currentString][currentModule]             = false;
    /* ======= RT1/4: call function under test */
    TEST_N775_ErrorHandling(&n775TestState, N775_COMMUNICATION_OK, 0u);
    /* ======= RT1/4: test output verification */
    TEST_ASSERT_TRUE(n775TestState.n775Data.errorTable->communicationOk[n775TestState.currentString][currentModule]);
    TEST_ASSERT_TRUE(
        n775TestState.n775Data.errorTable->noCommunicationTimeout[n775TestState.currentString][currentModule]);
    TEST_ASSERT_TRUE(n775TestState.n775Data.errorTable->crcIsValid[n775TestState.currentString][currentModule]);

    /* ======= RT2/4: Test implementation */
    n775TestState.n775Data.errorTable->communicationOk[n775TestState.currentString][currentModule]        = true;
    n775TestState.n775Data.errorTable->noCommunicationTimeout[n775TestState.currentString][currentModule] = true;
    n775TestState.n775Data.errorTable->crcIsValid[n775TestState.currentString][currentModule]             = true;
    /* ======= RT2/4: call function under test */
    TEST_N775_ErrorHandling(&n775TestState, N775_COMMUNICATION_ERROR_SHORT_MESSAGE, 0u);
    /* ======= RT2/4: test output verification */
    TEST_ASSERT_FALSE(n775TestState.n775Data.errorTable->communicationOk[n775TestState.currentString][currentModule]);
    TEST_ASSERT_TRUE(
        n775TestState.n775Data.errorTable->noCommunicationTimeout[n775TestState.currentString][currentModule]);
    TEST_ASSERT_TRUE(n775TestState.n775Data.errorTable->crcIsValid[n775TestState.currentString][currentModule]);

    /* ======= RT3/4: Test implementation */
    n775TestState.n775Data.errorTable->communicationOk[n775TestState.currentString][currentModule]        = true;
    n775TestState.n775Data.errorTable->noCommunicationTimeout[n775TestState.currentString][currentModule] = true;
    n775TestState.n775Data.errorTable->crcIsValid[n775TestState.currentString][currentModule]             = true;
    /* ======= RT3/4: call function under test */
    TEST_N775_ErrorHandling(&n775TestState, N775_COMMUNICATION_ERROR_TIMEOUT, 0u);
    /* ======= RT3/4: test output verification */
    TEST_ASSERT_FALSE(n775TestState.n775Data.errorTable->communicationOk[n775TestState.currentString][currentModule]);
    TEST_ASSERT_FALSE(
        n775TestState.n775Data.errorTable->noCommunicationTimeout[n775TestState.currentString][currentModule]);
    TEST_ASSERT_TRUE(n775TestState.n775Data.errorTable->crcIsValid[n775TestState.currentString][currentModule]);

    /* ======= RT4/4: Test implementation */
    n775TestState.n775Data.errorTable->communicationOk[n775TestState.currentString][currentModule]        = true;
    n775TestState.n775Data.errorTable->noCommunicationTimeout[n775TestState.currentString][currentModule] = true;
    n775TestState.n775Data.errorTable->crcIsValid[n775TestState.currentString][currentModule]             = true;
    /* ======= RT4/4: call function under test */
    TEST_N775_ErrorHandling(&n775TestState, N775_COMMUNICATION_ERROR_WRONG_CRC, 0u);
    /* ======= RT4/4: test output verification */
    TEST_ASSERT_FALSE(n775TestState.n775Data.errorTable->communicationOk[n775TestState.currentString][currentModule]);
    TEST_ASSERT_TRUE(
        n775TestState.n775Data.errorTable->noCommunicationTimeout[n775TestState.currentString][currentModule]);
    TEST_ASSERT_FALSE(n775TestState.n775Data.errorTable->crcIsValid[n775TestState.currentString][currentModule]);
}

void testN775_IncrementMuxIndex(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_N775_IncrementMuxIndex(NULL_PTR));

    /* ======= Routine tests =============================================== */
    N775_STATE_s n775TestState = {
        .currentString     = 0u,
        .pMuxSequenceStart = {0},
        .pMuxSequence      = {0},
    };

    /* ======= RT1/2: Test implementation */
    n775TestState.currentMux[n775TestState.currentString] = 0u;
    /* ======= RT1/2: call function under test */
    TEST_N775_IncrementMuxIndex(&n775TestState);
    /* ======= RT1/2: test output verification */
    /* TODO: assert missing */

    /* ======= RT2/2: Test implementation */
    n775TestState.currentMux[n775TestState.currentString] = N775_MUX_SEQUENCE_LENGTH;
    /* ======= RT2/2: call function under test */
    TEST_N775_IncrementMuxIndex(&n775TestState);
    /* ======= RT2/2: test output verification */
    TEST_ASSERT_EQUAL(0u, n775TestState.currentMux[n775TestState.currentString]);
}
void testN775_IncrementStringSequence(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_N775_IncrementStringSequence(NULL_PTR));
}
void testN775_Initialize(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_N775_Initialize(NULL_PTR));
}
void testN775_InitializeDatabase(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_N775_InitializeDatabase(NULL_PTR));
}
void testN775_InitializeI2c(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_N775_InitializeI2c(NULL_PTR));
}
void testN775_ResetStringSequence(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_N775_ResetStringSequence(NULL_PTR));
}
void testN775_ResetMuxIndex(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_N775_ResetMuxIndex(NULL_PTR));
}
void testN775_SetFirstMeasurementCycleFinished(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_N775_SetFirstMeasurementCycleFinished(NULL_PTR));

    /* ======= Routine tests =============================================== */
    N775_STATE_s n775TestState = {
        .firstMeasurementMade = false,
    };

    /* ======= RT1/2: Test implementation */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    /* ======= RT1/2: call function under test */
    TEST_N775_SetFirstMeasurementCycleFinished(&n775TestState);
    /* ======= RT1/2: test output verification */
    TEST_ASSERT_TRUE(n775TestState.firstMeasurementMade);
}
void testN775_SetMuxChannel(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_N775_SetMuxChannel(NULL_PTR));
}
void testN775_StartMeasurement(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_N775_StartMeasurement(NULL_PTR));
}
void testN775_TransmitI2c(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_N775_TransmitI2c(NULL_PTR));
}
void testN775_Wait(void) {
    const uint32_t waitTime = 1u;
    uint32_t currentTime    = 2u;
    OS_GetTickCount_ExpectAndReturn(currentTime);
    OS_DelayTaskUntil_Expect(&currentTime, waitTime);
    TEST_N775_Wait(waitTime);
}

/**
 * @brief   Testing extern function #N775_I2cRead
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/2: NULL_PTR for pData -> &rarr; assert
 *            - AT2/2: invalid data length -> &rarr; assert
 *          - Routine validation:
 *            - TODO
 */
void testN775_I2cRead(void) {
    /* ======= Assertion tests ============================================= */
    uint8_t data                            = 0u;
    const uint8_t validModuleNumber         = 0u;
    const uint8_t validDeviceAddress        = 0u;
    const uint8_t validDataLength           = 1u;
    const uint8_t invalidDataLengthTooSmall = 0u;
    const uint8_t invalidDataLengthTooLarge = 14u;
    /* ======= AT1/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(N775_I2cRead(validModuleNumber, validDeviceAddress, NULL_PTR, validDataLength));
    /* ======= AT1/2:1 ===== */
    TEST_ASSERT_FAIL_ASSERT(N775_I2cRead(validModuleNumber, validDeviceAddress, &data, invalidDataLengthTooSmall));
    /* ======= AT1/2:2 ===== */
    TEST_ASSERT_FAIL_ASSERT(N775_I2cRead(validModuleNumber, validDeviceAddress, &data, invalidDataLengthTooLarge));
    /* ======= Routine tests =============================================== */
}

void testN775_I2cWrite(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_N775_TransmitI2c(NULL_PTR));
}

/**
 * @brief   Testing extern function #N775_I2cRead
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/5: NULL_PTR for pDataWrite -> &rarr; assert
 *            - AT2/5: NULL_PTR for pDataRead -> &rarr; assert
 *            - AT3/5: invalid write data length -> &rarr; assert
 *            - AT4/5: invalid read data length -> &rarr; assert
 *            - AT5/5: invalid data length combination -> &rarr; assert
 *          - Routine validation:
 *            - TODO
 */
void testN775_I2cWriteRead(void) {
    /* ======= Assertion tests ============================================= */
    uint8_t writeData = 0u;
    uint8_t readData  = 0u;

    const uint8_t validModuleNumber  = 0u;
    const uint8_t validDeviceAddress = 0u;

    /* ======= AT1/5 ======= */
    TEST_ASSERT_FAIL_ASSERT(N775_I2cWriteRead(validModuleNumber, validDeviceAddress, NULL_PTR, 1u, &readData, 1u));
    /* ======= AT2/5 ======= */
    TEST_ASSERT_FAIL_ASSERT(N775_I2cWriteRead(validModuleNumber, validDeviceAddress, &writeData, 1u, NULL_PTR, 1u));
    /* ======= AT3/5 ======= */
    TEST_ASSERT_FAIL_ASSERT(N775_I2cWriteRead(validModuleNumber, validDeviceAddress, &writeData, 0u, &readData, 1u));
    /* ======= AT4/5 ======= */
    TEST_ASSERT_FAIL_ASSERT(N775_I2cWriteRead(validModuleNumber, validDeviceAddress, &writeData, 1u, &readData, 0u));
    /* ======= AT5/5 ======= */
    TEST_ASSERT_FAIL_ASSERT(N775_I2cWriteRead(validModuleNumber, validDeviceAddress, &writeData, 6u, &readData, 7u));

    /* ======= Routine tests =============================================== */
}

void testN775_IsFirstMeasurementCycleFinished(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(N775_IsFirstMeasurementCycleFinished(NULL_PTR));
}

void testN775_Measure(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(N775_Measure(NULL_PTR));
}
