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
 * @file    test_database.c
 * @author  foxBMS Team
 * @date    2020-04-01 (date of creation)
 * @updated 2021-12-01 (date of last update)
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Tests for the database driver
 *
 */

/*========== Includes =======================================================*/
#include "general.h"

#include "unity.h"
#include "Mockfassert.h"
#include "Mockftask.h"
#include "Mockmpu_prototypes.h"
#include "Mockos.h"

#include "database_cfg.h"

#include "database.h"
#include "test_assert_helper.h"

/*========== Definitions and Implementations for Unit Test ==================*/
/**
 * struct for message injection in database queue in #testDATA_ExecuteDataBIST()
 */
typedef struct DATA_QUEUE_BIST_INJECTED_MESSAGE {
    DATA_BLOCK_ACCESS_TYPE_e accesstype;                  /*!< read or write access type */
    DATA_BLOCK_DUMMY_FOR_SELF_TEST_s *pDatabaseEntry[1u]; /*!< reference to table */
} DATA_QUEUE_BIST_INJECTED_MESSAGE_s;

QueueHandle_t ftsk_databaseQueue    = NULL_PTR;
QueueHandle_t ftsk_imdCanDataQueue  = NULL_PTR;
QueueHandle_t ftsk_canRxQueue       = NULL_PTR;
volatile bool ftsk_allQueuesCreated = true;

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

void testDummy(void) {
}

/** callback for #testDATA_ExecuteDataBIST(); this not work for other instances */
OS_STD_RETURN_e DATA_mpuInjectValuesForExecuteBISTTestCallback(
    OS_QUEUE xQueue,
    const void *const pvItemToQueue,
    uint32_t xTicksToWait,
    int cmock_num_calls) {
    const DATA_QUEUE_BIST_INJECTED_MESSAGE_s *const injectQueueMessage = pvItemToQueue;
    /* inject the values into the message for a read access */
    if (injectQueueMessage->accesstype == DATA_READ_ACCESS) {
        injectQueueMessage->pDatabaseEntry[0u]->member1 = UINT8_MAX;
        injectQueueMessage->pDatabaseEntry[0u]->member2 = DATA_DUMMY_VALUE_UINT8_T_ALTERNATING_BIT_PATTERN;
    }

    return OS_SUCCESS;
}

void testDATA_ExecuteDataBIST(void) {
    DATA_BLOCK_DUMMY_FOR_SELF_TEST_s dummyTable = {.header.uniqueId = DATA_BLOCK_ID_DUMMY_FOR_SELF_TEST};
    const uint8_t testValue                     = DATA_DUMMY_VALUE_UINT8_T_ALTERNATING_BIT_PATTERN;
    dummyTable.member1                          = UINT8_MAX;
    dummyTable.member2                          = testValue;
    OS_SendToBackOfQueue_Stub(&DATA_mpuInjectValuesForExecuteBISTTestCallback);
    TEST_ASSERT_PASS_ASSERT(DATA_ExecuteDataBIST());
}

/*========== Test Cases =====================================================*/
