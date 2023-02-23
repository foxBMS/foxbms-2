/**
 *
 * @copyright &copy; 2010 - 2023, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * @file    database.c
 * @author  foxBMS Team
 * @date    2015-08-18 (date of creation)
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
 * @ingroup ENGINE
 * @prefix  DATA
 *
 * @brief   Database module implementation
 *
 * @details The database read/write functions put references to the database
 *          entries to be read/written in the database queue. The function
 *          DATA_Task() reads the queue to get the database entries to be
 *          read/written. Up to DATA_MAX_ENTRIES_PER_ACCESS entries can be
 *          read/written with the same function call. To avoid code
 *          duplication, the functions to read/write 1 to
 *          DATA_MAX_ENTRIES_PER_ACCESS-1 entries call the function to
 *          read/write DATA_MAX_ENTRIES_PER_ACCESS entries and use NULL_PTR
 *          for the entries that are not to be read/written. DATA_Task()
 *          checks that the first entry is not a NULL_PTR and asserts if it
 *          is not the case. If subsequent entries are found in the database
 *          queue, they are simply ignored if they are NULL_PTR.
 */

/*========== Includes =======================================================*/
#include "general.h"

#include "database.h"

#include "ftask.h"
#include "os.h"

#include <stdint.h>
#include <string.h>

/*========== Macros and Definitions =========================================*/
/** Maximum queue timeout time in milliseconds */
#define DATA_MAX_QUEUE_TIMEOUT_MS (10u)

#define DATA_QUEUE_TIMEOUT_MS (DATA_MAX_QUEUE_TIMEOUT_MS / OS_TICK_RATE_MS)

FAS_STATIC_ASSERT(DATA_QUEUE_TIMEOUT_MS > 0u, "invalid database queue timeout!");

/** configuration struct of database device */
typedef struct {
    uint8_t nrDatabaseEntries; /*!< number of database entries */
    DATA_BASE_s *pDatabase;    /*!< pointer to the array with the database entries */
} DATA_BASE_HEADER_s;

/*========== Static Constant and Variable Definitions =======================*/

/**
 * @brief   device configuration of database
 * @details all attributes of device configuration are listed here (pointer to
 *          channel list, number of channels)
 */
static const DATA_BASE_HEADER_s data_baseHeader = {
    .nrDatabaseEntries = sizeof(data_database) / sizeof(DATA_BASE_s), /**< number of blocks (and block headers) */
    .pDatabase         = &data_database[0],
};

/**
 * @brief   uniqueId to respective database entry selector
 * @details This array is the link between the uniqueId of a database entry and
 *          the actual position of the database entry in data_database[].
 *          The IDs are set to its final value, when Data_Initialize is called.
 */
static uint8_t data_uniqueIdToDatabaseEntry[DATA_BLOCK_ID_MAX] = {0};

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/
static void DATA_IterateOverDatabaseEntries(const DATA_QUEUE_MESSAGE_s *kpReceiveMessage);

static STD_RETURN_TYPE_e DATA_AccessDatabaseEntries(
    DATA_BLOCK_ACCESS_TYPE_e accessType,
    void *pData0,
    void *pData1,
    void *pData2,
    void *pData3);

static void DATA_CopyData(
    DATA_BLOCK_ACCESS_TYPE_e accessType,
    uint32_t dataLength,
    void *pDatabaseStruct,
    void *pPassedDataStruct);

/*========== Static Function Implementations ================================*/
static STD_RETURN_TYPE_e DATA_AccessDatabaseEntries(
    DATA_BLOCK_ACCESS_TYPE_e accessType,
    void *pData0,
    void *pData1,
    void *pData2,
    void *pData3) {
    FAS_ASSERT((accessType == DATA_WRITE_ACCESS) || (accessType == DATA_READ_ACCESS));
    FAS_ASSERT(pData0 != NULL_PTR);
    /* AXIVION Routine Generic-MissingParameterAssert: pData1: pointer might be NULL_PTR (i.e., if the caller
     * is DATA_Read1DataBlock/DATA_Write1DataBlock). The DATA_Task function checks the pointer being not NULL_PTR prior
     * to usage. */
    /* AXIVION Routine Generic-MissingParameterAssert: pData2: pointer might be NULL_PTR (i.e., if the caller
     * is DATA_Read1DataBlock/DATA_Write1DataBlock or DATA_Read2DataBlocks/DATA_Write2DataBlocks). The DATA_Task
     * function checks the pointer being not NULL_PTR prior to usage. */
    /* AXIVION Routine Generic-MissingParameterAssert: pData3: pointer might be NULL_PTR (i.e., if the caller
     * is DATA_Read1DataBlock/DATA_Write1DataBlock, DATA_Read2DataBlocks/DATA_Write2DataBlocks or
     * DATA_Read3DataBlocks/DATA_Write3DataBlocks). The DATA_Task function checks the pointer being not NULL_PTR prior
     * to usage. */
    STD_RETURN_TYPE_e retval = STD_NOT_OK;
    /* prepare send message with attributes of data block */
    DATA_QUEUE_MESSAGE_s data_sendMessage = {
        .pDatabaseEntry[DATA_ENTRY_0] = pData0,
        .pDatabaseEntry[DATA_ENTRY_1] = pData1,
        .pDatabaseEntry[DATA_ENTRY_2] = pData2,
        .pDatabaseEntry[DATA_ENTRY_3] = pData3,
        .accessType                   = accessType,
    };
    /* Send a pointer to a message object and maximum block time: DATA_QUEUE_TIMEOUT_MS */
    if (OS_SendToBackOfQueue(ftsk_databaseQueue, (void *)&data_sendMessage, DATA_QUEUE_TIMEOUT_MS) == OS_SUCCESS) {
        retval = STD_OK;
    }
    return retval;
}

static void DATA_CopyData(
    DATA_BLOCK_ACCESS_TYPE_e accessType,
    uint32_t dataLength,
    void *pDatabaseStruct,
    void *pPassedDataStruct) {
    FAS_ASSERT((accessType == DATA_WRITE_ACCESS) || (accessType == DATA_READ_ACCESS));
    /* AXIVION Routine Generic-MissingParameterAssert: dataLength: parameter accepts whole range */
    /* Copy data either into database or passed database struct */
    FAS_ASSERT(pDatabaseStruct != NULL_PTR);
    FAS_ASSERT(pPassedDataStruct != NULL_PTR);

    if (accessType == DATA_WRITE_ACCESS) {
        /* Pointer on data block header of passed struct */
        /* AXIVION Next Codeline Style MisraC2012-11.5 this casted is required in order to have a generic interface
         * for all database entries. */
        DATA_BLOCK_HEADER_s *pHeader = (DATA_BLOCK_HEADER_s *)pPassedDataStruct;
        /* Update timestamps in passed database struct and then copy this struct into database */
        pHeader->previousTimestamp = pHeader->timestamp;
        pHeader->timestamp         = OS_GetTickCount();
        /* Copy passed struct in database struct */
        /* memcpy has no return value therefore there is nothing to check: casting to void */
        /* AXIVION Next Codeline Style MisraC2012-21.18
         * */
        (void)memcpy(pDatabaseStruct, pPassedDataStruct, dataLength);
    } else if (accessType == DATA_READ_ACCESS) {
        /* Copy database entry in passed struct */
        /* memcpy has no return value therefore there is nothing to check: casting to void */
        (void)memcpy(pPassedDataStruct, pDatabaseStruct, dataLength);
    } else {
        /* invalid database operation */
        FAS_ASSERT(FAS_TRAP);
    }
}

static void DATA_IterateOverDatabaseEntries(const DATA_QUEUE_MESSAGE_s *kpReceiveMessage) {
    FAS_ASSERT(kpReceiveMessage != NULL_PTR);
    for (uint8_t queueEntry = 0u; queueEntry < DATA_MAX_ENTRIES_PER_ACCESS; queueEntry++) {
        /* Iterate over pointer array and handle all access operations if pointer != NULL_PTR
         * All pointers in the array, expect the first one, might be NULL_PTR, which is valid.
         * Again, to understand explanation see the comments in DATA_Read1DataBlock, DATA_Read2DataBlocks,
         * DATA_Read3DataBlocks and DATA_Read4DataBlocks as well as DATA_Write1DataBlock,
         * DATA_Write2DataBlocks, DATA_Write3DataBlocks and DATA_Write4DataBlocks */
        if (kpReceiveMessage->pDatabaseEntry[queueEntry] != NULL_PTR) {
            /* pointer to passed database struct */
            void *pPassedDataStruct = kpReceiveMessage->pDatabaseEntry[queueEntry];
            /* Get access type (read or write) of passed data struct */
            DATA_BLOCK_ACCESS_TYPE_e accessType = kpReceiveMessage->accessType;

            /* Get pointer to database header entry */
            const DATA_BLOCK_HEADER_s *kpHeader = (DATA_BLOCK_HEADER_s *)kpReceiveMessage->pDatabaseEntry[queueEntry];
            uint8_t uniqueId                    = (uint8_t)(kpHeader->uniqueId);
            FAS_ASSERT(uniqueId < (uint8_t)DATA_BLOCK_ID_MAX);
            uint8_t entryIndex = data_uniqueIdToDatabaseEntry[uniqueId];
            /* Pointer to database struct representation of passed struct */
            void *pDatabaseStruct = (void *)data_baseHeader.pDatabase[entryIndex].pDatabaseEntry;
            /* Get dataLength of database entry */
            uint32_t dataLength = data_baseHeader.pDatabase[entryIndex].dataLength;

            DATA_CopyData(accessType, dataLength, pDatabaseStruct, pPassedDataStruct);
        }
    }
}

/*========== Extern Function Implementations ================================*/
STD_RETURN_TYPE_e DATA_Initialize(void) {
    STD_RETURN_TYPE_e retval = STD_OK;
    /* Check that database queue has been created */
    const bool allQueuesCreatedCopyForAssert = ftsk_allQueuesCreated;
    FAS_ASSERT(allQueuesCreatedCopyForAssert == true);

    FAS_STATIC_ASSERT((sizeof(data_database) != 0u), "No database defined");
    /*  make sure that no basic assumptions are broken -- since data_database is
        declared with length DATA_BLOCK_ID_MAX, this assert should never fail */
    FAS_STATIC_ASSERT(
        ((sizeof(data_database) / sizeof(DATA_BASE_s)) == (uint8_t)(DATA_BLOCK_ID_MAX)), "Database array length error");

    /* Iterate over database and set respective read/write pointer for each database entry */
    for (uint16_t i = 0u; i < data_baseHeader.nrDatabaseEntries; i++) {
        /* Initialize database entry with 0, set write pointer to start of database entry */
        uint8_t *pStartDatabaseEntryWR = (uint8_t *)data_baseHeader.pDatabase[i].pDatabaseEntry;

        /* Start after uniqueId entry. Set j'th byte to zero in database entry */
        for (uint32_t j = 0u; j < (data_baseHeader.pDatabase + i)->dataLength; j++) {
            if (j >= sizeof(DATA_BLOCK_ID_e)) {
                *pStartDatabaseEntryWR = 0;
            }
            pStartDatabaseEntryWR++;
        }
    }

    /* Configure link between uniqueId and database entry array position */
    for (uint8_t databaseEntry = 0u; databaseEntry < data_baseHeader.nrDatabaseEntries; databaseEntry++) {
        /* Get pointer to database header entry */
        DATA_BLOCK_HEADER_s *pHeader = (DATA_BLOCK_HEADER_s *)data_baseHeader.pDatabase[databaseEntry].pDatabaseEntry;
        /*  make sure that the database entry is not a null pointer (which would happen if an entry is missing
            despite the ID existing) */
        FAS_ASSERT(pHeader != NULL_PTR);
        /* Get uniqueId */
        DATA_BLOCK_ID_e blockId = pHeader->uniqueId;
        if ((blockId < DATA_BLOCK_ID_MAX) && (databaseEntry < DATA_BLOCK_ID_MAX)) {
            /* e.g., data_uniqueIdToDatabaseEntry[<some id>] = configured database entry index */
            data_uniqueIdToDatabaseEntry[blockId] = databaseEntry;
        } else {
            /* Configuration error -> set retval to #STD_NOT_OK */
            retval = STD_NOT_OK;
        }
    }

    if (ftsk_databaseQueue == NULL_PTR) {
        retval = STD_NOT_OK; /* Failed to create the queue */
    }
    return retval;
}

void DATA_Task(void) {
    if (ftsk_databaseQueue != NULL_PTR) {
        DATA_QUEUE_MESSAGE_s receiveMessage = {
            .accessType     = DATA_READ_ACCESS,
            .pDatabaseEntry = {GEN_REPEAT_U(NULL_PTR, GEN_STRIP(DATA_MAX_ENTRIES_PER_ACCESS))}};
        /* scan queue and wait for a message up to a maximum amount of 1ms (block time) */
        if (OS_ReceiveFromQueue(ftsk_databaseQueue, (&receiveMessage), 1u) == OS_SUCCESS) {
            /* plausibility check, error whether the first pointer is a NULL_PTR, as this must not happen.
             * See the comments in DATA_Read1DataBlock, DATA_Read2DataBlocks, DATA_Read3DataBlocks and
             * DATA_Read4DataBlocks as well as DATA_Write1DataBlock, DATA_Write2DataBlocks,
             * DATA_Write3DataBlocks and DATA_Write4DataBlocks */
            FAS_ASSERT(receiveMessage.pDatabaseEntry[0] != NULL_PTR);
            /* ready to start reading/writing database entries */
            DATA_IterateOverDatabaseEntries(&receiveMessage);
        }
    }
}

void DATA_DummyFunction(void) {
}

STD_RETURN_TYPE_e DATA_Read1DataBlock(void *pDataToReceiver0) {
    FAS_ASSERT(pDataToReceiver0 != NULL_PTR);
    return DATA_AccessDatabaseEntries(DATA_READ_ACCESS, pDataToReceiver0, NULL_PTR, NULL_PTR, NULL_PTR);
}

STD_RETURN_TYPE_e DATA_Read2DataBlocks(void *pDataToReceiver0, void *pDataToReceiver1) {
    FAS_ASSERT(pDataToReceiver0 != NULL_PTR);
    FAS_ASSERT(pDataToReceiver1 != NULL_PTR);
    return DATA_AccessDatabaseEntries(DATA_READ_ACCESS, pDataToReceiver0, pDataToReceiver1, NULL_PTR, NULL_PTR);
}

STD_RETURN_TYPE_e DATA_Read3DataBlocks(void *pDataToReceiver0, void *pDataToReceiver1, void *pDataToReceiver2) {
    FAS_ASSERT(pDataToReceiver0 != NULL_PTR);
    FAS_ASSERT(pDataToReceiver1 != NULL_PTR);
    FAS_ASSERT(pDataToReceiver2 != NULL_PTR);
    return DATA_AccessDatabaseEntries(DATA_READ_ACCESS, pDataToReceiver0, pDataToReceiver1, pDataToReceiver2, NULL_PTR);
}

STD_RETURN_TYPE_e DATA_Read4DataBlocks(
    void *pDataToReceiver0,
    void *pDataToReceiver1,
    void *pDataToReceiver2,
    void *pDataToReceiver3) {
    FAS_ASSERT(pDataToReceiver0 != NULL_PTR);
    FAS_ASSERT(pDataToReceiver1 != NULL_PTR);
    FAS_ASSERT(pDataToReceiver2 != NULL_PTR);
    FAS_ASSERT(pDataToReceiver3 != NULL_PTR);
    return DATA_AccessDatabaseEntries(
        DATA_READ_ACCESS, pDataToReceiver0, pDataToReceiver1, pDataToReceiver2, pDataToReceiver3);
}

STD_RETURN_TYPE_e DATA_Write1DataBlock(void *pDataFromSender0) {
    FAS_ASSERT(pDataFromSender0 != NULL_PTR);
    return DATA_AccessDatabaseEntries(DATA_WRITE_ACCESS, pDataFromSender0, NULL_PTR, NULL_PTR, NULL_PTR);
}

STD_RETURN_TYPE_e DATA_Write2DataBlocks(void *pDataFromSender0, void *pDataFromSender1) {
    FAS_ASSERT(pDataFromSender0 != NULL_PTR);
    FAS_ASSERT(pDataFromSender1 != NULL_PTR);
    return DATA_AccessDatabaseEntries(DATA_WRITE_ACCESS, pDataFromSender0, pDataFromSender1, NULL_PTR, NULL_PTR);
}

STD_RETURN_TYPE_e DATA_Write3DataBlocks(void *pDataFromSender0, void *pDataFromSender1, void *pDataFromSender2) {
    FAS_ASSERT(pDataFromSender0 != NULL_PTR);
    FAS_ASSERT(pDataFromSender1 != NULL_PTR);
    FAS_ASSERT(pDataFromSender2 != NULL_PTR);
    return DATA_AccessDatabaseEntries(
        DATA_WRITE_ACCESS, pDataFromSender0, pDataFromSender1, pDataFromSender2, NULL_PTR);
}

STD_RETURN_TYPE_e DATA_Write4DataBlocks(
    void *pDataFromSender0,
    void *pDataFromSender1,
    void *pDataFromSender2,
    void *pDataFromSender3) {
    FAS_ASSERT(pDataFromSender0 != NULL_PTR);
    FAS_ASSERT(pDataFromSender1 != NULL_PTR);
    FAS_ASSERT(pDataFromSender2 != NULL_PTR);
    FAS_ASSERT(pDataFromSender3 != NULL_PTR);
    return DATA_AccessDatabaseEntries(
        DATA_WRITE_ACCESS, pDataFromSender0, pDataFromSender1, pDataFromSender2, pDataFromSender3);
}

extern void DATA_ExecuteDataBist(void) {
    /* compile database entry */
    DATA_BLOCK_DUMMY_FOR_SELF_TEST_s dummyWriteTable = {.header.uniqueId = DATA_BLOCK_ID_DUMMY_FOR_SELF_TEST};
    dummyWriteTable.member1                          = UINT8_MAX;
    dummyWriteTable.member2                          = DATA_DUMMY_VALUE_UINT8_T_ALTERNATING_BIT_PATTERN;

    /* write entry */
    STD_RETURN_TYPE_e writeReturnValue = DATA_WRITE_DATA(&dummyWriteTable);
    FAS_ASSERT(writeReturnValue == STD_OK);

    DATA_BLOCK_DUMMY_FOR_SELF_TEST_s dummyReadTable = {.header.uniqueId = DATA_BLOCK_ID_DUMMY_FOR_SELF_TEST};

    /* read entry into new variable */
    STD_RETURN_TYPE_e readReturnValue = DATA_READ_DATA(&dummyReadTable);
    FAS_ASSERT(readReturnValue == STD_OK);

    FAS_ASSERT(dummyReadTable.member1 == dummyWriteTable.member1);
    FAS_ASSERT(dummyReadTable.member2 == dummyWriteTable.member2);
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
