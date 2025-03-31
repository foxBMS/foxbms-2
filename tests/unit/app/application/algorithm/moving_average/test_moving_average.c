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
 * @file    test_moving_average.c
 * @author  foxBMS Team
 * @date    2020-07-01 (date of creation)
 * @updated 2025-03-31 (date of last update)
 * @version v1.9.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Test of the algorithm module
 * @details TODO
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockdatabase.h"
#include "Mockos.h"

#include "moving_average.h"

/*========== Unit Testing Framework Directives ==============================*/
TEST_INCLUDE_PATH("../../src/app/application/algorithm/config")
TEST_INCLUDE_PATH("../../src/app/application/algorithm/moving_average")

TEST_SOURCE_FILE("moving_average.c")

/*========== Definitions and Implementations for Unit Test ==================*/
#define NUM_DATA_READ_SUB_CALLS (1)

typedef struct {
    DATA_BLOCK_CURRENT_SENSOR_s *curPow;
    DATA_BLOCK_MOVING_AVERAGE_s *movingAvg;
} DATA_BLOCKS;

DATA_BLOCK_CURRENT_SENSOR_s curPow_tab        = {.header.uniqueId = DATA_BLOCK_ID_CURRENT_SENSOR};
DATA_BLOCK_MOVING_AVERAGE_s movingAverage_tab = {.header.uniqueId = DATA_BLOCK_ID_MOVING_AVERAGE};

DATA_BLOCKS blocks = {
    .curPow    = &curPow_tab,
    .movingAvg = &movingAverage_tab,
};

/*========== Setup and Teardown =============================================*/
void setUp(void) {
    curPow_tab.newCurrent = 1;
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
/**
 * @brief   Iterate over a callback that supplies various scenarios and check if they work as expected
 * @details This function uses the callback #MockDATA_ReadBlock_Callback() in order to inject
 *          other values into the returned database tables.
 */
STD_RETURN_TYPE_e MockDATA_ReadBlock_Callback(void *pDataToReceiver, int num_calls) {
    uint8_t newCurrent = 0;

    /* determine a value depending on num_calls (has to be synchronized with test) */
    switch (num_calls) {
        case 0:
            /* Set to new current value */
            newCurrent = 1;
            break;
        default:
            TEST_FAIL_MESSAGE("DATA_ReadBlock_Callback was called too often");
    }
    /* ENTER HIGHEST CASE NUMBER IN EXPECT; checks whether all cases are used */
    TEST_ASSERT_EQUAL_MESSAGE(0, (NUM_DATA_READ_SUB_CALLS - 1), "Check code of stub. Something does not fit.");

    if (num_calls >= NUM_DATA_READ_SUB_CALLS) {
        TEST_FAIL_MESSAGE("This stub is fishy");
    }

    /* cast to correct struct */
    ((DATA_BLOCK_CURRENT_SENSOR_s *)pDataToReceiver)->newCurrent = newCurrent;

    return STD_OK;
}

void testALGO_MovingAverage(void) {
    /* tell CMock to use our callback */
    DATA_Read1DataBlock_Stub(MockDATA_ReadBlock_Callback);
    DATA_Read1DataBlock_ExpectAndReturn(&curPow_tab, STD_OK);
    DATA_Read1DataBlock_Stub(NULL);
    DATA_Read1DataBlock_ExpectAndReturn(&movingAverage_tab, STD_OK);

    ALGO_MovingAverage();
}
