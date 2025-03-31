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
 * @file    test_nxp_mc33775a_balancing.c
 * @author  foxBMS Team
 * @date    2025-03-20 (date of creation)
 * @updated 2025-03-31 (date of last update)
 * @version v1.9.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Test of nxp_mc33775a_balancing.c
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
#include "nxp_mc33775a_database.h"
#include "nxp_mc33775a_i2c.h"
#include "nxp_mc33775a_mux.h"
#include "spi_cfg-helper.h"
#include "uc_msg_t.h"

/* clang-format off */
#include "test_assert_helper.h"
/* clang-format on */

#include <stdbool.h>
#include <stdint.h>

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("nxp_mc33775a_balancing.c")

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
