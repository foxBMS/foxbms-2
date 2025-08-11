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
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Test of nxp_mc33775a_balancing.c
 * @details TODO
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockdatabase.h"
#include "Mocknxp_mc3377x-ll.h"

#include "nxp_mc3377x_balancing.h"
#include "nxp_mc3377x_reg_def.h"
#include "spi_cfg-helper.h"

/* clang-format off */
#include "test_assert_helper.h"
/* clang-format on */

#include <stdbool.h>
#include <stdint.h>

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("nxp_mc33775a_balancing.c")

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

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
void testN77x_BalanceControl(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(N77x_BalanceControl(NULL_PTR));

    /* ======= Routine tests =============================================== */
    DATA_BLOCK_BALANCING_CONTROL_s n77x_tableBalancingControl = {.header.uniqueId = DATA_BLOCK_ID_BALANCING_CONTROL};

    N77X_STATE_s n77x_testState = {
        .currentString             = 0u,
        .pSpiTxSequence            = spi_nxp77xInterfaceTx,
        .n77xData.balancingControl = &n77x_tableBalancingControl,
    };

    /* ======= RT1/2 ======= */
    /* Only test function N77x_BalanceControl */
    N77x_CommunicationWrite_Expect(
        N77X_BROADCAST_ADDRESS,
        MC3377X_BAL_GLOB_TO_TMR_OFFSET,
        N77X_GLOBAL_BALANCING_TIMER,
        n77x_testState.pSpiTxSequence);

    N77x_CommunicationWrite_Expect(
        N77X_BROADCAST_ADDRESS, MC3377X_BAL_PRE_TMR_OFFSET, N77X_PRE_BALANCING_TIMER, n77x_testState.pSpiTxSequence);

    N77x_CommunicationWrite_Expect(
        N77X_BROADCAST_ADDRESS,
        MC3377X_BAL_TMR_CH_ALL_OFFSET,
        (MC3377X_BAL_TMR_CH_ALL_PWM_PWM100_ENUM_VAL << MC3377X_BAL_TMR_CH_ALL_PWM_POS) |
            (N77X_ALL_CHANNEL_BALANCING_TIMER << MC3377X_BAL_TMR_CH_ALL_BALTIME_POS),
        n77x_testState.pSpiTxSequence);

    N77x_CommunicationWrite_Expect(
        N77X_BROADCAST_ADDRESS,
        MC3377X_BAL_GLOB_CFG_OFFSET,
        (MC3377X_BAL_GLOB_CFG_BALEN_ENABLED_ENUM_VAL << MC3377X_BAL_GLOB_CFG_BALEN_POS) |
            (MC3377X_BAL_GLOB_CFG_TMRBALEN_STOP_ENUM_VAL << MC3377X_BAL_GLOB_CFG_TMRBALEN_POS),
        n77x_testState.pSpiTxSequence);

    DATA_Read1DataBlock_ExpectAndReturn(n77x_testState.n77xData.balancingControl, STD_OK);

    /* Activate balancing for all cells */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t cb = 0u; cb < BS_NR_OF_CELL_BLOCKS_PER_MODULE; cb++) {
                n77x_testState.n77xData.balancingControl->activateBalancing[s][m][cb] = true;
            }
        }
    }

    for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
        uint8_t deviceAddress   = m + 1u;
        uint16_t balancingState = 0u;
        for (uint16_t cb = 0u; cb < BS_NR_OF_CELL_BLOCKS_PER_MODULE; cb++) {
            if (n77x_testState.n77xData.balancingControl->activateBalancing[n77x_testState.currentString][m][cb] ==
                true) {
                balancingState |= 1u << cb;
            }
        }
        N77x_CommunicationWrite_Expect(
            deviceAddress, MC3377X_BAL_CH_CFG_OFFSET, balancingState, n77x_testState.pSpiTxSequence);
    }
    TEST_ASSERT_PASS_ASSERT(N77x_BalanceControl(&n77x_testState));

    /* ======= RT2/2 ======= */
    /* Inactive balancing */
    N77x_CommunicationWrite_Expect(
        N77X_BROADCAST_ADDRESS,
        MC3377X_BAL_GLOB_TO_TMR_OFFSET,
        N77X_GLOBAL_BALANCING_TIMER,
        n77x_testState.pSpiTxSequence);

    N77x_CommunicationWrite_Expect(
        N77X_BROADCAST_ADDRESS, MC3377X_BAL_PRE_TMR_OFFSET, N77X_PRE_BALANCING_TIMER, n77x_testState.pSpiTxSequence);

    N77x_CommunicationWrite_Expect(
        N77X_BROADCAST_ADDRESS,
        MC3377X_BAL_TMR_CH_ALL_OFFSET,
        (MC3377X_BAL_TMR_CH_ALL_PWM_PWM100_ENUM_VAL << MC3377X_BAL_TMR_CH_ALL_PWM_POS) |
            (N77X_ALL_CHANNEL_BALANCING_TIMER << MC3377X_BAL_TMR_CH_ALL_BALTIME_POS),
        n77x_testState.pSpiTxSequence);

    N77x_CommunicationWrite_Expect(
        N77X_BROADCAST_ADDRESS,
        MC3377X_BAL_GLOB_CFG_OFFSET,
        (MC3377X_BAL_GLOB_CFG_BALEN_ENABLED_ENUM_VAL << MC3377X_BAL_GLOB_CFG_BALEN_POS) |
            (MC3377X_BAL_GLOB_CFG_TMRBALEN_STOP_ENUM_VAL << MC3377X_BAL_GLOB_CFG_TMRBALEN_POS),
        n77x_testState.pSpiTxSequence);

    DATA_Read1DataBlock_ExpectAndReturn(n77x_testState.n77xData.balancingControl, STD_OK);

    /* Deactivate balancing for all cells */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t cb = 0u; cb < BS_NR_OF_CELL_BLOCKS_PER_MODULE; cb++) {
                n77x_testState.n77xData.balancingControl->activateBalancing[s][m][cb] = false;
            }
        }
    }

    for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
        uint8_t deviceAddress   = m + 1u;
        uint16_t balancingState = 0u;
        for (uint16_t cb = 0u; cb < BS_NR_OF_CELL_BLOCKS_PER_MODULE; cb++) {
            if (n77x_testState.n77xData.balancingControl->activateBalancing[n77x_testState.currentString][m][cb] ==
                true) {
                balancingState |= 1u << cb;
            }
        }
        N77x_CommunicationWrite_Expect(
            deviceAddress, MC3377X_BAL_CH_CFG_OFFSET, balancingState, n77x_testState.pSpiTxSequence);
    }
    TEST_ASSERT_PASS_ASSERT(N77x_BalanceControl(&n77x_testState));
}

void testN77x_BalanceSetup(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(N77x_BalanceSetup(NULL_PTR))
}
