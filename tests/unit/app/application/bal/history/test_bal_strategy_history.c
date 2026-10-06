/**
 *
 * @copyright &copy; 2010 - 2026, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * @file    test_bal_strategy_history.c
 * @author  foxBMS Team
 * @date    2020-06-05 (date of creation)
 * @updated 2026-10-06 (date of last update)
 * @version v1.12.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Test of the history based balancing module
 * @details Tests Balancing init, get state and finished
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockbal_cfg.h"
#include "Mockbattery_system_cfg.h"
#include "Mockbms.h"
#include "Mockdatabase.h"
#include "Mockfassert.h"
#include "Mockfram.h"
#include "Mockio.h"
#include "Mockmcu.h"
#include "Mockos.h"
#include "Mockspi.h"
#include "Mockstate_estimation.h"

#include "database_cfg.h"

#include "bal.h"

#include <math.h>
#include <string.h>

/*========== Unit Testing Framework Directives ==============================*/

/*========== Definitions and Implementations for Unit Test ==================*/

static DATA_BLOCK_BALANCING_CONTROL_s bms_tableControl = {.header.uniqueId = DATA_BLOCK_ID_BALANCING_CONTROL};

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
void testCheckBalancingInitByDisablingBalancing(void) {
    DATA_BLOCK_BALANCING_CONTROL_s *pBalancing = TEST_BAL_GetBalancingControl();
    pBalancing->enableBalancing                = true;
    DATA_Read1DataBlock_ExpectAndReturn(&bms_tableControl, STD_OK);
    DATA_Write1DataBlock_ExpectAndReturn(&bms_tableControl, STD_OK);
    BAL_Init(pBalancing);
    TEST_ASSERT_EQUAL(false, pBalancing->enableBalancing);
}

void testBalancingGetState(void) {
    BAL_STATE_s *balancingState = TEST_BAL_GetBalancingState();
    balancingState->state       = BAL_FSM_UNINITIALIZED;
    TEST_ASSERT_EQUAL(BAL_FSM_UNINITIALIZED, BAL_GetState());
    balancingState->state = BAL_FSM_INITIALIZED;
    TEST_ASSERT_EQUAL(BAL_FSM_INITIALIZED, BAL_GetState());
}

void testBalancingFinished(void) {
    BAL_STATE_s *balancingState            = TEST_BAL_GetBalancingState();
    balancingState->initializationFinished = STD_NOT_OK;
    TEST_ASSERT_EQUAL(STD_NOT_OK, BAL_GetInitializationState());
    balancingState->initializationFinished = STD_OK;
    TEST_ASSERT_EQUAL(STD_OK, BAL_GetInitializationState());
}

/**
 * @brief   Test of history-based balancing imbalance calculation
 * @details Cases:
 *          - Routine validation:
 *            - RT1/1: 3202 mV maps to 10% SOC and 3636 mV maps to 50% SOC;
 *              verify the mV input contract and the resulting charge difference
 */
void testBalancingComputeImbalancesUsesStateEstimationUnits(void) {
    DATA_BLOCK_BALANCING_CONTROL_s *pBalancing = TEST_BAL_GetBalancingControl();
    DATA_BLOCK_CELL_VOLTAGE_s *pCellVoltage    = TEST_BAL_GetCellVoltage();
    const uint8_t stringIndex                  = 0u;
    const uint8_t moduleIndex                  = 0u;
    const uint8_t minimumVoltageCellIndex      = 0u;
    const uint8_t higherVoltageCellIndex       = 1u;
    const int16_t minimumCellVoltage_mV        = 3202;
    const int16_t higherCellVoltage_mV         = 3636;
    const float_t minimumStateOfCharge_perc    = 10.0f;
    const float_t higherStateOfCharge_perc     = 50.0f;
    const int32_t balancingThreshold_mV        = 0;
    /* DOD difference: (3500 mAh * 0.90 * 3600 s/h) - (3500 mAh * 0.50 * 3600 s/h). */
    const uint32_t expectedChargeDifference_mAs = 5040000u;

    (void)memset(pBalancing, 0, sizeof(*pBalancing));
    (void)memset(pCellVoltage, 0, sizeof(*pCellVoltage));
    pBalancing->header.uniqueId   = DATA_BLOCK_ID_BALANCING_CONTROL;
    pCellVoltage->header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE;

    for (uint8_t cb = 0u; cb < BS_NR_OF_CELL_BLOCKS_PER_MODULE; cb++) {
        pCellVoltage->cellVoltage_mV[stringIndex][moduleIndex][cb] = minimumCellVoltage_mV;
    }
    pCellVoltage->cellVoltage_mV[stringIndex][moduleIndex][higherVoltageCellIndex] = higherCellVoltage_mV;

    DATA_Read2DataBlocks_ExpectAndReturn(pBalancing, pCellVoltage, STD_OK);
    SE_GetStateOfChargeFromVoltage_ExpectAndReturn(minimumCellVoltage_mV, minimumStateOfCharge_perc);
    BAL_GetBalancingThreshold_mV_ExpectAndReturn(balancingThreshold_mV);
    SE_GetStateOfChargeFromVoltage_ExpectAndReturn(higherCellVoltage_mV, higherStateOfCharge_perc);
    DATA_Write1DataBlock_ExpectAndReturn(pBalancing, STD_OK);

    TEST_BAL_ComputeImbalances();

    TEST_ASSERT_EQUAL_UINT32(0u, pBalancing->deltaCharge_mAs[stringIndex][moduleIndex][minimumVoltageCellIndex]);
    TEST_ASSERT_EQUAL_UINT32(
        expectedChargeDifference_mAs, pBalancing->deltaCharge_mAs[stringIndex][moduleIndex][higherVoltageCellIndex]);
}
