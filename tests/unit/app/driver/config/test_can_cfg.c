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
 * @file    test_can_cfg.c
 * @author  foxBMS Team
 * @date    2020-07-28 (date of creation)
 * @updated 2020-07-28 (date of last update)
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Tests for the CAN driver
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockcan.h"
#include "Mockdatabase.h"
#include "Mockdiag.h"
#include "Mockfoxmath.h"
#include "Mockmpu_prototypes.h"
#include "Mockos.h"

#include "can_cfg.h"
#include "database_cfg.h"

#include "imd.h"

/*========== Definitions and Implementations for Unit Test ==================*/

QueueHandle_t imd_canDataQueue = NULL_PTR;

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
void testcan_txVolt(void) {
    DATA_BLOCK_CELL_VOLTAGE_s *pcan_voltTab = TEST_CAN_GetCellvoltageTab();
    for (uint8_t stringNumber = 0u; stringNumber < BS_NR_OF_STRINGS; stringNumber++) {
        pcan_voltTab->cellVoltage_mV[stringNumber][0] = 4200;
        pcan_voltTab->cellVoltage_mV[stringNumber][1] = 0;
        pcan_voltTab->cellVoltage_mV[stringNumber][2] = 2000;
        pcan_voltTab->cellVoltage_mV[stringNumber][3] = 0;
        pcan_voltTab->cellVoltage_mV[stringNumber][4] = 4200;
        pcan_voltTab->cellVoltage_mV[stringNumber][5] = 0;
    }
    DATA_Read_1_DataBlock_ExpectAndReturn(pcan_voltTab, STD_OK);
    DATA_Read_1_DataBlock_ReturnThruPtr_pDataToReceiver0(pcan_voltTab);
    uint8_t data[8] = {0};
    TEST_CAN_TxVoltage(0x110, 8, littleEndian, data, NULL_PTR);

    TEST_ASSERT_EQUAL(60, data[0]);
    TEST_ASSERT_EQUAL(209, data[1]);
    TEST_ASSERT_EQUAL(71, data[2]);
    TEST_ASSERT_EQUAL(56, data[3]);
    TEST_ASSERT_EQUAL(125, data[4]);
    TEST_ASSERT_EQUAL(60, data[5]);
    TEST_ASSERT_EQUAL(209, data[6]);
    TEST_ASSERT_EQUAL(7, data[7]);
}

void testcan_rxDebug(void) {
    /* adapt this test when the callback is implemented */
    uint8_t data[8] = {0};
    TEST_ASSERT_EQUAL(0, TEST_CAN_RxDebug(0, 0, 0, data, NULL_PTR));
}
