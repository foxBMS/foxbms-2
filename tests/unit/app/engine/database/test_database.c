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
 * @file    test_database.c
 * @author  foxBMS Team
 * @date    2020-04-01 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Tests for the database driver
 *
 */

/*========== Includes =======================================================*/

#include "unity.h"
#include "Mockfassert.h"
#include "Mockftask.h"
#include "Mockmpu_prototypes.h"
#include "Mockos.h"

#include "database_cfg.h"

#include "database.h"
#include "fstd_types.h"
#include "test_assert_helper.h"

#include <stdbool.h>

/*========== Unit Testing Framework Directives ==============================*/
TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/rtc")
TEST_INCLUDE_PATH("../../src/app/task/config")
TEST_INCLUDE_PATH("../../src/app/task/ftask")

/*========== Definitions and Implementations for Unit Test ==================*/

/** Maximum queue timeout time in milliseconds */
#define DATA_MAX_QUEUE_TIMEOUT_MS (10u)
#define DATA_QUEUE_TIMEOUT_MS     (DATA_MAX_QUEUE_TIMEOUT_MS / OS_TICK_RATE_MS)

/**
 * struct for message injection in database queue in #testDATA_ExecuteDataBist()
 */
typedef struct {
    DATA_BLOCK_ACCESS_TYPE_e accesstype;                  /*!< read or write access type */
    DATA_BLOCK_DUMMY_FOR_SELF_TEST_s *pDatabaseEntry[1u]; /*!< reference to table */
} DATA_QUEUE_BIST_INJECTED_MESSAGE_s;

OS_QUEUE ftsk_databaseQueue         = NULL_PTR;
OS_QUEUE ftsk_imdCanDataQueue       = NULL_PTR;
OS_QUEUE ftsk_canRxQueue            = NULL_PTR;
volatile bool ftsk_allQueuesCreated = true;

/* prepare send message with attributes of data block */
DATA_QUEUE_MESSAGE_s data_sendMessage = {
    .pDatabaseEntry[DATA_ENTRY_0] = 0u,
    .pDatabaseEntry[DATA_ENTRY_1] = 0u,
    .pDatabaseEntry[DATA_ENTRY_2] = 0u,
    .pDatabaseEntry[DATA_ENTRY_3] = 0u,
    .accessType                   = DATA_WRITE_ACCESS,
};

/** data block struct for the database built-in self-test */
typedef struct {
    /* This struct needs to be at the beginning of every database entry. During
     * the initialization of a database struct, uniqueId must be set to the
     * respective database entry representation in enum DATA_BLOCK_ID_e. */
    DATA_BLOCK_HEADER_s header; /*!< Data block header */
    uint8_t member1;            /*!< first member of self-test struct */
    uint8_t member2;            /*!< second member of self-test struct */
} DATA_BLOCK_TEST_s;

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/**
 * @brief   Testing externalized static function DATA_AccessDatabaseEntries
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/2: invalid access type &rarr; assert
 *            - AT2/2: NULL_PTR for pData0 &rarr; assert
 *          - Routine validation:
 *            - RT1/1: TODO
 */
void testDATA_AccessDatabaseEntries(void) {
    /* ======= Assertion tests ============================================= */
    const DATA_BLOCK_ACCESS_TYPE_e invalidAccessType = 300u;
    const DATA_BLOCK_ACCESS_TYPE_e validAccessType   = DATA_WRITE_ACCESS;
    uint8_t dummyValue                               = 0u;
    void *pValidDummy0                               = &dummyValue;
    void *pValidDummy1                               = &dummyValue; /* not used for interface test, see function */
    void *pValidDummy2                               = &dummyValue; /* not used for interface test, see function */
    void *pValidDummy3                               = &dummyValue; /* not used for interface test, see function */

    /* prepare send message with attributes of data block */
    data_sendMessage.pDatabaseEntry[DATA_ENTRY_0] = pValidDummy0;
    data_sendMessage.pDatabaseEntry[DATA_ENTRY_1] = pValidDummy1;
    data_sendMessage.pDatabaseEntry[DATA_ENTRY_2] = pValidDummy2;
    data_sendMessage.pDatabaseEntry[DATA_ENTRY_3] = pValidDummy3;
    data_sendMessage.accessType                   = validAccessType;

    /* ======= AT1/2: Assertion test */
    TEST_ASSERT_FAIL_ASSERT(
        TEST_DATA_AccessDatabaseEntries(invalidAccessType, pValidDummy0, pValidDummy1, pValidDummy2, pValidDummy3));
    /* ======= AT2/2: Assertion test */
    TEST_ASSERT_FAIL_ASSERT(
        TEST_DATA_AccessDatabaseEntries(validAccessType, NULL_PTR, pValidDummy1, pValidDummy2, pValidDummy3));

    /* ======= Routine tests =============================================== */
    const DATA_BLOCK_ACCESS_TYPE_e readAccess  = DATA_WRITE_ACCESS;
    const DATA_BLOCK_ACCESS_TYPE_e writeAccess = DATA_READ_ACCESS;
    /* ======= RT1/2: Test implementation */
    OS_SendToBackOfQueue_ExpectAndReturn(
        ftsk_databaseQueue, (void *)&data_sendMessage, DATA_QUEUE_TIMEOUT_MS, OS_SUCCESS);
    /* ======= RT1/2: call function under test */
    const STD_RETURN_TYPE_e readSuccessfully =
        TEST_DATA_AccessDatabaseEntries(readAccess, pValidDummy0, pValidDummy1, pValidDummy2, pValidDummy3);
    /* ======= RT1/2: test output verification */
    TEST_ASSERT_EQUAL(STD_OK, readSuccessfully);

    data_sendMessage.accessType = DATA_READ_ACCESS;
    /* ======= RT2/2: Test implementation */
    OS_SendToBackOfQueue_ExpectAndReturn(ftsk_databaseQueue, (void *)&data_sendMessage, DATA_QUEUE_TIMEOUT_MS, OS_FAIL);
    /* ======= RT2/2: call function under test */
    const STD_RETURN_TYPE_e writeUnsuccessfully =
        TEST_DATA_AccessDatabaseEntries(writeAccess, pValidDummy0, pValidDummy1, pValidDummy2, pValidDummy3);
    /* ======= RT2/2: test output verification */
    TEST_ASSERT_EQUAL(STD_NOT_OK, writeUnsuccessfully);
}

/**
 * @brief   Testing externalized static function DATA_CopyData
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - TODO
 *          - Routine validation:
 *            - TODO
 */
void testDATA_CopyData(void) {
    /* ======= Assertion tests ============================================= */
    const DATA_BLOCK_ACCESS_TYPE_e invalidAccessType = 300u;
    uint32_t dataLength                              = 20u;
    uint8_t dummyValue                               = 0u;
    void *pValidDummy0                               = &dummyValue;
    void *pValidDummy1                               = &dummyValue; /* not used for interface test, see function */
    /* ======= AT1/3: Assertion test */
    TEST_ASSERT_FAIL_ASSERT(TEST_DATA_CopyData(invalidAccessType, dataLength, pValidDummy0, pValidDummy1));
    /* ======= AT2/3: Assertion test */
    TEST_ASSERT_FAIL_ASSERT(TEST_DATA_CopyData(DATA_WRITE_ACCESS, dataLength, NULL_PTR, pValidDummy1));
    /* ======= AT3/3: Assertion test */
    TEST_ASSERT_FAIL_ASSERT(TEST_DATA_CopyData(DATA_READ_ACCESS, dataLength, pValidDummy0, NULL_PTR));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/3: Test implementation */
    /* Note: uniqueId is not used in this function so we can set it anything,
     * that is not 0, so that we have something to compare against */
    DATA_BLOCK_TEST_s fromEntry = {
        .header.uniqueId          = (DATA_BLOCK_ID_e)1u,
        .header.previousTimestamp = 3u,
        .header.timestamp         = 4u,
        .member1                  = 12u,
        .member2                  = 42u,
    };
    DATA_BLOCK_TEST_s toEntry = {
        .header.uniqueId          = (DATA_BLOCK_ID_e)0u,
        .header.previousTimestamp = 0u,
        .header.timestamp         = 0u,
        .member1                  = 0u,
        .member2                  = 0u,
    };
    dataLength = sizeof(DATA_BLOCK_TEST_s);
    /* ======= RT1/3: call function under test */
    TEST_DATA_CopyData(DATA_READ_ACCESS, dataLength, &toEntry, &fromEntry);
    /* ======= RT1/3: test output verification */
    TEST_ASSERT_EQUAL_MEMORY(&fromEntry, &toEntry, dataLength);

    /* ======= RT2/3: Test implementation */
    /* Note: uniqueId is not used in this function so we can set it anything,
     * that is not 0, so that we have something to compare against */
    fromEntry.header.uniqueId          = (DATA_BLOCK_ID_e)2u;
    fromEntry.header.previousTimestamp = 12u;
    fromEntry.header.timestamp         = 13u;
    fromEntry.member1                  = 0x1Au;
    fromEntry.member2                  = 0xA0u;

    toEntry.header.uniqueId          = (DATA_BLOCK_ID_e)0u;
    toEntry.header.previousTimestamp = 0u;
    toEntry.header.timestamp         = 0u;
    toEntry.member1                  = 0u;
    toEntry.member2                  = 0u;

    const uint32_t newTimestamp     = 15u;
    DATA_BLOCK_TEST_s expectedEntry = {
        .header.uniqueId          = (DATA_BLOCK_ID_e)2u,
        .header.previousTimestamp = 13u,
        .header.timestamp         = newTimestamp,
        .member1                  = 0x1Au,
        .member2                  = 0xA0u,
    };

    OS_GetTickCount_ExpectAndReturn(newTimestamp);
    /* ======= RT2/3: call function under test */
    TEST_DATA_CopyData(DATA_WRITE_ACCESS, dataLength, &toEntry, &fromEntry);
    /* ======= RT2/3: test output verification */
    TEST_ASSERT_EQUAL_MEMORY(&expectedEntry, &toEntry, dataLength);

    /* ======= RT3/3: Test implementation */
    /* invalid access type */
    /* ======= RT2/2: call function under test */
    TEST_DATA_CopyData(2u, dataLength, &toEntry, &fromEntry);
    /* ======= RT2/2: test output verification */
    TEST_ASSERT_EQUAL_MEMORY(&fromEntry, &toEntry, dataLength);
}

/**
 * @brief   Testing externalized static function DATA_IterateOverDatabaseEntries
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - TODO
 *          - Routine validation:
 *            - TODO
 */
void testDATA_IterateOverDatabaseEntries(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1: Assertion test */
    TEST_ASSERT_FAIL_ASSERT(TEST_DATA_IterateOverDatabaseEntries(NULL_PTR));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/2: Test implementation */
    static DATA_BLOCK_CELL_VOLTAGE_s entry_blockCellVoltage = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE};
    DATA_QUEUE_MESSAGE_s messages[4u]                       = {
        {
                                  .accessType     = DATA_READ_ACCESS,
                                  .pDatabaseEntry = (void *)&entry_blockCellVoltage,
        },
        {
                                  .pDatabaseEntry = NULL_PTR,
        },
        {
                                  .pDatabaseEntry = NULL_PTR,
        },
        {
                                  .pDatabaseEntry = NULL_PTR,
        },
    };
    /* ======= RT1/2: call function under test */
    TEST_DATA_IterateOverDatabaseEntries(messages);
    /* ======= RT1/2: test output verification */
    /* TODO comparison */

    /* ======= RT2/2: Test implementation */
    static DATA_BLOCK_CELL_VOLTAGE_s entry_blockInvalidId = {.header.uniqueId = DATA_BLOCK_ID_MAX};
    DATA_QUEUE_MESSAGE_s invalidMessages[4]               = {
        {
                          .accessType     = DATA_READ_ACCESS,
                          .pDatabaseEntry = (void *)&entry_blockInvalidId,
        },
        {
                          .pDatabaseEntry = NULL_PTR,
        },
        {
                          .pDatabaseEntry = NULL_PTR,
        },
        {
                          .pDatabaseEntry = NULL_PTR,
        },
    };
    /* ======= RT2/2: call function under test */
    TEST_ASSERT_FAIL_ASSERT(TEST_DATA_IterateOverDatabaseEntries(invalidMessages));
    /* ======= RT2/2: test output verification */
    /* nothing to do */
}

/**
 * @brief   Testing external function #DATA_Initialize
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - TODO
 *          - Routine validation:
 *            - TODO
 */
void testDATA_Initialize(void) {
    ftsk_allQueuesCreated = 0;
    TEST_ASSERT_FAIL_ASSERT(DATA_Initialize());

    ftsk_allQueuesCreated = 1;
    DATA_Initialize();
}

/**
 * @brief   Testing external function #DATA_Task
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - TODO
 *          - Routine validation:
 *            - TODO
 */
void testDATA_Task(void) {
}

/**
 * @brief   Testing extern function #DATA_Read1DataBlock
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: NULL_PTR for pDataToReceiver0 &rarr; assert
 *          - Routine validation:
 *            - RT1/1: TODO
 */
void testDATA_Read1DataBlock(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1: Assertion test */
    TEST_ASSERT_FAIL_ASSERT(DATA_Read1DataBlock(NULL_PTR));

    /* ======= Routine tests =============================================== */
    uint8_t dummyValue = 0u;
    void *pValidDummy0 = &dummyValue;

    /* prepare send message */
    data_sendMessage.pDatabaseEntry[DATA_ENTRY_0] = pValidDummy0;
    data_sendMessage.pDatabaseEntry[DATA_ENTRY_1] = NULL_PTR;
    data_sendMessage.pDatabaseEntry[DATA_ENTRY_2] = NULL_PTR;
    data_sendMessage.pDatabaseEntry[DATA_ENTRY_3] = NULL_PTR;
    data_sendMessage.accessType                   = DATA_READ_ACCESS;

    /* ======= RT1/2: Test implementation */
    OS_SendToBackOfQueue_ExpectAndReturn(
        ftsk_databaseQueue, (void *)&data_sendMessage, DATA_QUEUE_TIMEOUT_MS, OS_SUCCESS);
    /* ======= RT1/2: call function under test */
    const STD_RETURN_TYPE_e readSuccessfully = DATA_Read1DataBlock(pValidDummy0);
    /* ======= RT1/2: test output verification */
    TEST_ASSERT_EQUAL(STD_OK, readSuccessfully);

    /* ======= RT2/2: Test implementation */
    OS_SendToBackOfQueue_ExpectAndReturn(ftsk_databaseQueue, (void *)&data_sendMessage, DATA_QUEUE_TIMEOUT_MS, OS_FAIL);
    /* ======= RT2/2: call function under test */
    const STD_RETURN_TYPE_e readUnsuccessfully = DATA_Read1DataBlock(pValidDummy0);
    /* ======= RT2/2: test output verification */
    TEST_ASSERT_EQUAL(STD_NOT_OK, readUnsuccessfully);
}

/**
 * @brief   Testing extern function #DATA_Read2DataBlocks
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/2: NULL_PTR for pDataToReceiver0 &rarr; assert
 *            - AT2/2: NULL_PTR for pDataToReceiver1 &rarr; assert
 *          - Routine validation:
 *            - RT1/1: TODO
 */
void testDATA_Read2DataBlocks(void) {
    /* ======= Assertion tests ============================================= */
    uint8_t dummyValue = 0u;
    void *pValidDummy0 = &dummyValue;
    void *pValidDummy1 = &dummyValue;

    /* prepare send message */
    data_sendMessage.pDatabaseEntry[DATA_ENTRY_0] = pValidDummy0;
    data_sendMessage.pDatabaseEntry[DATA_ENTRY_1] = pValidDummy1;
    data_sendMessage.pDatabaseEntry[DATA_ENTRY_2] = NULL_PTR;
    data_sendMessage.pDatabaseEntry[DATA_ENTRY_3] = NULL_PTR;
    data_sendMessage.accessType                   = DATA_READ_ACCESS;

    /* ======= AT1/2: Assertion test */
    TEST_ASSERT_FAIL_ASSERT(DATA_Read2DataBlocks(NULL_PTR, pValidDummy1));
    /* ======= AT2/2: Assertion test */
    TEST_ASSERT_FAIL_ASSERT(DATA_Read2DataBlocks(pValidDummy0, NULL_PTR));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/2: Test implementation */
    OS_SendToBackOfQueue_ExpectAndReturn(
        ftsk_databaseQueue, (void *)&data_sendMessage, DATA_QUEUE_TIMEOUT_MS, OS_SUCCESS);
    /* ======= RT1/2: call function under test */
    const STD_RETURN_TYPE_e readSuccessfully = DATA_Read2DataBlocks(pValidDummy0, pValidDummy1);
    /* ======= RT1/2: test output verification */
    TEST_ASSERT_EQUAL(STD_OK, readSuccessfully);

    /* ======= RT2/2: Test implementation */
    OS_SendToBackOfQueue_ExpectAndReturn(ftsk_databaseQueue, (void *)&data_sendMessage, DATA_QUEUE_TIMEOUT_MS, OS_FAIL);
    /* ======= RT2/2: call function under test */
    const STD_RETURN_TYPE_e readUnsuccessfully = DATA_Read2DataBlocks(pValidDummy0, pValidDummy1);
    /* ======= RT2/2: test output verification */
    TEST_ASSERT_EQUAL(STD_NOT_OK, readUnsuccessfully);
}

/**
 * @brief   Testing extern function #DATA_Read3DataBlocks
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/3: NULL_PTR for pDataToReceiver0 &rarr; assert
 *            - AT2/3: NULL_PTR for pDataToReceiver1 &rarr; assert
 *            - AT3/3: NULL_PTR for pDataToReceiver2 &rarr; assert
 *          - Routine validation:
 *            - RT1/1: TODO
 */
void testDATA_Read3DataBlocks(void) {
    /* ======= Assertion tests ============================================= */
    uint8_t dummyValue = 0u;
    void *pValidDummy0 = &dummyValue;
    void *pValidDummy1 = &dummyValue;
    void *pValidDummy2 = &dummyValue;

    /* prepare send message */
    data_sendMessage.pDatabaseEntry[DATA_ENTRY_0] = pValidDummy0;
    data_sendMessage.pDatabaseEntry[DATA_ENTRY_1] = pValidDummy1;
    data_sendMessage.pDatabaseEntry[DATA_ENTRY_2] = pValidDummy2;
    data_sendMessage.pDatabaseEntry[DATA_ENTRY_3] = NULL_PTR;
    data_sendMessage.accessType                   = DATA_READ_ACCESS;

    /* ======= AT1/3: Assertion test */
    TEST_ASSERT_FAIL_ASSERT(DATA_Read3DataBlocks(NULL_PTR, pValidDummy1, pValidDummy2));
    /* ======= AT2/3: Assertion test */
    TEST_ASSERT_FAIL_ASSERT(DATA_Read3DataBlocks(pValidDummy0, NULL_PTR, pValidDummy2));
    /* ======= AT3/3: Assertion test */
    TEST_ASSERT_FAIL_ASSERT(DATA_Read3DataBlocks(pValidDummy0, pValidDummy1, NULL_PTR));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/2: Test implementation */
    OS_SendToBackOfQueue_ExpectAndReturn(
        ftsk_databaseQueue, (void *)&data_sendMessage, DATA_QUEUE_TIMEOUT_MS, OS_SUCCESS);
    /* ======= RT1/2: call function under test */
    const STD_RETURN_TYPE_e readSuccessfully = DATA_Read3DataBlocks(pValidDummy0, pValidDummy1, pValidDummy2);
    /* ======= RT1/2: test output verification */
    TEST_ASSERT_EQUAL(STD_OK, readSuccessfully);

    /* ======= RT2/2: Test implementation */
    OS_SendToBackOfQueue_ExpectAndReturn(ftsk_databaseQueue, (void *)&data_sendMessage, DATA_QUEUE_TIMEOUT_MS, OS_FAIL);
    /* ======= RT2/2: call function under test */
    const STD_RETURN_TYPE_e readUnsuccessfully = DATA_Read3DataBlocks(pValidDummy0, pValidDummy1, pValidDummy2);
    /* ======= RT2/2: test output verification */
    TEST_ASSERT_EQUAL(STD_NOT_OK, readUnsuccessfully);
}

/**
 * @brief   Testing extern function #DATA_Read4DataBlocks
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/4: NULL_PTR for pDataToReceiver0 &rarr; assert
 *            - AT2/4: NULL_PTR for pDataToReceiver1 &rarr; assert
 *            - AT3/4: NULL_PTR for pDataToReceiver2 &rarr; assert
 *            - AT4/4: NULL_PTR for pDataToReceiver3 &rarr; assert
 *          - Routine validation:
 *            - RT1/1: TODO
 */
void testDATA_Read4DataBlocks(void) {
    /* ======= Assertion tests ============================================= */
    uint8_t dummyValue = 0u;
    void *pValidDummy0 = &dummyValue;
    void *pValidDummy1 = &dummyValue;
    void *pValidDummy2 = &dummyValue;
    void *pValidDummy3 = &dummyValue;

    /* prepare send message */
    data_sendMessage.pDatabaseEntry[DATA_ENTRY_0] = pValidDummy0;
    data_sendMessage.pDatabaseEntry[DATA_ENTRY_1] = pValidDummy1;
    data_sendMessage.pDatabaseEntry[DATA_ENTRY_2] = pValidDummy2;
    data_sendMessage.pDatabaseEntry[DATA_ENTRY_3] = pValidDummy3;
    data_sendMessage.accessType                   = DATA_READ_ACCESS;

    /* ======= AT1/4: Assertion test */
    TEST_ASSERT_FAIL_ASSERT(DATA_Read4DataBlocks(NULL_PTR, pValidDummy1, pValidDummy2, pValidDummy3));
    /* ======= AT2/4: Assertion test */
    TEST_ASSERT_FAIL_ASSERT(DATA_Read4DataBlocks(pValidDummy0, NULL_PTR, pValidDummy2, pValidDummy3));
    /* ======= AT3/4: Assertion test */
    TEST_ASSERT_FAIL_ASSERT(DATA_Read4DataBlocks(pValidDummy0, pValidDummy1, NULL_PTR, pValidDummy3));
    /* ======= AT4/4: Assertion test */
    TEST_ASSERT_FAIL_ASSERT(DATA_Read4DataBlocks(pValidDummy0, pValidDummy1, pValidDummy2, NULL_PTR));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/2: Test implementation */
    OS_SendToBackOfQueue_ExpectAndReturn(
        ftsk_databaseQueue, (void *)&data_sendMessage, DATA_QUEUE_TIMEOUT_MS, OS_SUCCESS);
    /* ======= RT1/2: call function under test */
    const STD_RETURN_TYPE_e readSuccessfully =
        DATA_Read4DataBlocks(pValidDummy0, pValidDummy1, pValidDummy2, pValidDummy3);
    /* ======= RT1/2: test output verification */
    TEST_ASSERT_EQUAL(STD_OK, readSuccessfully);

    /* ======= RT2/2: Test implementation */
    OS_SendToBackOfQueue_ExpectAndReturn(ftsk_databaseQueue, (void *)&data_sendMessage, DATA_QUEUE_TIMEOUT_MS, OS_FAIL);
    /* ======= RT2/2: call function under test */
    const STD_RETURN_TYPE_e readUnsuccessfully =
        DATA_Read4DataBlocks(pValidDummy0, pValidDummy1, pValidDummy2, pValidDummy3);
    /* ======= RT2/2: test output verification */
    TEST_ASSERT_EQUAL(STD_NOT_OK, readUnsuccessfully);
}

/**
 * @brief   Testing extern function #DATA_Write1DataBlock
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: NULL_PTR for pDataFromSender0 &rarr; assert
 *          - Routine validation:
 *            - RT1/1: TODO
 */
void testDATA_Write1DataBlock(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1: Assertion test */
    TEST_ASSERT_FAIL_ASSERT(DATA_Write1DataBlock(NULL_PTR));

    /* ======= Routine tests =============================================== */
    uint8_t dummyValue = 0u;
    void *pValidDummy0 = &dummyValue;

    /* prepare send message */
    data_sendMessage.pDatabaseEntry[DATA_ENTRY_0] = pValidDummy0;
    data_sendMessage.pDatabaseEntry[DATA_ENTRY_1] = NULL_PTR;
    data_sendMessage.pDatabaseEntry[DATA_ENTRY_2] = NULL_PTR;
    data_sendMessage.pDatabaseEntry[DATA_ENTRY_3] = NULL_PTR;
    data_sendMessage.accessType                   = DATA_WRITE_ACCESS;

    /* ======= RT1/2: Test implementation */
    OS_SendToBackOfQueue_ExpectAndReturn(
        ftsk_databaseQueue, (void *)&data_sendMessage, DATA_QUEUE_TIMEOUT_MS, OS_SUCCESS);
    /* ======= RT1/2: call function under test */
    const STD_RETURN_TYPE_e writeSuccessfully = DATA_Write1DataBlock(pValidDummy0);
    /* ======= RT1/2: test output verification */
    TEST_ASSERT_EQUAL(STD_OK, writeSuccessfully);

    /* ======= RT2/2: Test implementation */
    OS_SendToBackOfQueue_ExpectAndReturn(ftsk_databaseQueue, (void *)&data_sendMessage, DATA_QUEUE_TIMEOUT_MS, OS_FAIL);
    /* ======= RT2/2: call function under test */
    const STD_RETURN_TYPE_e writeUnsuccessfully = DATA_Write1DataBlock(pValidDummy0);
    /* ======= RT2/2: test output verification */
    TEST_ASSERT_EQUAL(STD_NOT_OK, writeUnsuccessfully);
}

/**
 * @brief   Testing extern function #DATA_Write2DataBlocks
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/2: NULL_PTR for pDataFromSender0 &rarr; assert
 *            - AT2/2: NULL_PTR for pDataFromSender1 &rarr; assert
 *          - Routine validation:
 *            - RT1/1: TODO
 */
void testDATA_Write2DataBlocks(void) {
    /* ======= Assertion tests ============================================= */
    uint8_t dummyValue = 0u;
    void *pValidDummy0 = &dummyValue;
    void *pValidDummy1 = &dummyValue;

    /* prepare send message */
    data_sendMessage.pDatabaseEntry[DATA_ENTRY_0] = pValidDummy0;
    data_sendMessage.pDatabaseEntry[DATA_ENTRY_1] = pValidDummy1;
    data_sendMessage.pDatabaseEntry[DATA_ENTRY_2] = NULL_PTR;
    data_sendMessage.pDatabaseEntry[DATA_ENTRY_3] = NULL_PTR;
    data_sendMessage.accessType                   = DATA_WRITE_ACCESS;

    /* ======= AT1/2: Assertion test */
    TEST_ASSERT_FAIL_ASSERT(DATA_Write2DataBlocks(NULL_PTR, pValidDummy1));
    /* ======= AT2/2: Assertion test */
    TEST_ASSERT_FAIL_ASSERT(DATA_Write2DataBlocks(pValidDummy0, NULL_PTR));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/2: Test implementation */
    OS_SendToBackOfQueue_ExpectAndReturn(
        ftsk_databaseQueue, (void *)&data_sendMessage, DATA_QUEUE_TIMEOUT_MS, OS_SUCCESS);
    /* ======= RT1/2: call function under test */
    const STD_RETURN_TYPE_e writeSuccessfully = DATA_Write2DataBlocks(pValidDummy0, pValidDummy1);
    /* ======= RT1/2: test output verification */
    TEST_ASSERT_EQUAL(STD_OK, writeSuccessfully);

    /* ======= RT2/2: Test implementation */
    OS_SendToBackOfQueue_ExpectAndReturn(ftsk_databaseQueue, (void *)&data_sendMessage, DATA_QUEUE_TIMEOUT_MS, OS_FAIL);
    /* ======= RT2/2: call function under test */
    const STD_RETURN_TYPE_e writeUnsuccessfully = DATA_Write2DataBlocks(pValidDummy0, pValidDummy1);
    /* ======= RT2/2: test output verification */
    TEST_ASSERT_EQUAL(STD_NOT_OK, writeUnsuccessfully);
}

/**
 * @brief   Testing extern function #DATA_Write3DataBlocks
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/3: NULL_PTR for pDataFromSender0 &rarr; assert
 *            - AT2/3: NULL_PTR for pDataFromSender1 &rarr; assert
 *            - AT3/3: NULL_PTR for pDataFromSender2 &rarr; assert
 *          - Routine validation:
 *            - RT1/1: TODO
 */
void testDATA_Write3DataBlocks(void) {
    /* ======= Assertion tests ============================================= */
    uint8_t dummyValue = 0u;
    void *pValidDummy0 = &dummyValue;
    void *pValidDummy1 = &dummyValue;
    void *pValidDummy2 = &dummyValue;

    /* prepare send message */
    data_sendMessage.pDatabaseEntry[DATA_ENTRY_0] = pValidDummy0;
    data_sendMessage.pDatabaseEntry[DATA_ENTRY_1] = pValidDummy1;
    data_sendMessage.pDatabaseEntry[DATA_ENTRY_2] = pValidDummy2;
    data_sendMessage.pDatabaseEntry[DATA_ENTRY_3] = NULL_PTR;
    data_sendMessage.accessType                   = DATA_WRITE_ACCESS;

    /* ======= AT1/3: Assertion test */
    TEST_ASSERT_FAIL_ASSERT(DATA_Write3DataBlocks(NULL_PTR, pValidDummy1, pValidDummy2));
    /* ======= AT2/3: Assertion test */
    TEST_ASSERT_FAIL_ASSERT(DATA_Write3DataBlocks(pValidDummy0, NULL_PTR, pValidDummy2));
    /* ======= AT3/3: Assertion test */
    TEST_ASSERT_FAIL_ASSERT(DATA_Write3DataBlocks(pValidDummy0, pValidDummy1, NULL_PTR));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/2: Test implementation */
    OS_SendToBackOfQueue_ExpectAndReturn(
        ftsk_databaseQueue, (void *)&data_sendMessage, DATA_QUEUE_TIMEOUT_MS, OS_SUCCESS);
    /* ======= RT1/2: call function under test */
    const STD_RETURN_TYPE_e writeSuccessfully = DATA_Write3DataBlocks(pValidDummy0, pValidDummy1, pValidDummy2);
    /* ======= RT1/2: test output verification */
    TEST_ASSERT_EQUAL(STD_OK, writeSuccessfully);

    /* ======= RT2/2: Test implementation */
    OS_SendToBackOfQueue_ExpectAndReturn(ftsk_databaseQueue, (void *)&data_sendMessage, DATA_QUEUE_TIMEOUT_MS, OS_FAIL);
    /* ======= RT2/2: call function under test */
    const STD_RETURN_TYPE_e writeUnsuccessfully = DATA_Write3DataBlocks(pValidDummy0, pValidDummy1, pValidDummy2);
    /* ======= RT2/2: test output verification */
    TEST_ASSERT_EQUAL(STD_NOT_OK, writeUnsuccessfully);
}

/**
 * @brief   Testing extern function #DATA_Write4DataBlocks
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/4: NULL_PTR for pDataFromSender0 &rarr; assert
 *            - AT2/4: NULL_PTR for pDataFromSender1 &rarr; assert
 *            - AT3/4: NULL_PTR for pDataFromSender2 &rarr; assert
 *            - AT4/4: NULL_PTR for pDataFromSender3 &rarr; assert
 *          - Routine validation:
 *            - RT1/1: TODO
 */
void testDATA_Write4DataBlocks(void) {
    /* ======= Assertion tests ============================================= */
    uint8_t dummyValue = 0u;
    void *pValidDummy0 = &dummyValue;
    void *pValidDummy1 = &dummyValue;
    void *pValidDummy2 = &dummyValue;
    void *pValidDummy3 = &dummyValue;

    /* prepare send message */
    data_sendMessage.pDatabaseEntry[DATA_ENTRY_0] = pValidDummy0;
    data_sendMessage.pDatabaseEntry[DATA_ENTRY_1] = pValidDummy1;
    data_sendMessage.pDatabaseEntry[DATA_ENTRY_2] = pValidDummy2;
    data_sendMessage.pDatabaseEntry[DATA_ENTRY_3] = pValidDummy3;
    data_sendMessage.accessType                   = DATA_WRITE_ACCESS;

    /* ======= AT1/4: Assertion test */
    TEST_ASSERT_FAIL_ASSERT(DATA_Write4DataBlocks(NULL_PTR, pValidDummy1, pValidDummy2, pValidDummy3));
    /* ======= AT2/4: Assertion test */
    TEST_ASSERT_FAIL_ASSERT(DATA_Write4DataBlocks(pValidDummy0, NULL_PTR, pValidDummy2, pValidDummy3));
    /* ======= AT3/4: Assertion test */
    TEST_ASSERT_FAIL_ASSERT(DATA_Write4DataBlocks(pValidDummy0, pValidDummy1, NULL_PTR, pValidDummy3));
    /* ======= AT4/4: Assertion test */
    TEST_ASSERT_FAIL_ASSERT(DATA_Write4DataBlocks(pValidDummy0, pValidDummy1, pValidDummy2, NULL_PTR));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/2: Test implementation */
    OS_SendToBackOfQueue_ExpectAndReturn(
        ftsk_databaseQueue, (void *)&data_sendMessage, DATA_QUEUE_TIMEOUT_MS, OS_SUCCESS);
    /* ======= RT1/2: call function under test */
    const STD_RETURN_TYPE_e writeSuccessfully =
        DATA_Write4DataBlocks(pValidDummy0, pValidDummy1, pValidDummy2, pValidDummy3);
    /* ======= RT1/2: test output verification */
    TEST_ASSERT_EQUAL(STD_OK, writeSuccessfully);

    /* ======= RT2/2: Test implementation */
    OS_SendToBackOfQueue_ExpectAndReturn(ftsk_databaseQueue, (void *)&data_sendMessage, DATA_QUEUE_TIMEOUT_MS, OS_FAIL);
    /* ======= RT2/2: call function under test */
    const STD_RETURN_TYPE_e writeUnsuccessfully =
        DATA_Write4DataBlocks(pValidDummy0, pValidDummy1, pValidDummy2, pValidDummy3);
    /* ======= RT2/2: test output verification */
    TEST_ASSERT_EQUAL(STD_NOT_OK, writeUnsuccessfully);
}

/** callback for #testDATA_ExecuteDataBist(); this not work for other instances */
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

/**
 * @brief   Testing extern function DATA_ExecuteDataBist
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - none
 *          - Routine validation:
 *            - TODO
 */
void testDATA_ExecuteDataBist(void) {
    OS_SendToBackOfQueue_Stub(&DATA_mpuInjectValuesForExecuteBISTTestCallback);
    TEST_ASSERT_PASS_ASSERT(DATA_ExecuteDataBist());
}

/*========== Test Cases =====================================================*/
