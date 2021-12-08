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
 * @file    database.c
 * @author  foxBMS Team
 * @date    2015-08-18 (date of creation)
 * @updated 2021-12-01 (date of last update)
 * @ingroup ENGINE
 * @prefix  DATA
 *
 * @brief   Database module implementation
 *
 * @details Implementation of database module
 */

/*========== Includes =======================================================*/
#include "database.h"

#include "ftask.h"
#include "os.h"

#include <string.h>

/*========== Macros and Definitions =========================================*/
/**
 * Maximum queue timeout time in milliseconds
 */
#define DATA_QUEUE_TIMEOUT_MS ((TickType_t)10u)

/**
 * configuration struct of database device
 */
typedef struct DATA_BASE_HEADER {
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
 *          the actual position of the database entry in data_database[]
 */
static uint16_t uniqueIdToDatabaseEntry[DATA_BLOCK_ID_MAX];

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/
STD_RETURN_TYPE_e DATA_Init(void) {
    STD_RETURN_TYPE_e retval = STD_OK;
    /* Check that database queue has been created */
    FAS_ASSERT(true == ftsk_allQueuesCreated);

    static_assert((sizeof(data_database) != 0u), "No database defined");
    /*  make sure that no basic assumptions are broken -- since data_database is
        declared with length DATA_BLOCK_ID_MAX, this assert should never fail */
    static_assert(
        ((sizeof(data_database) / sizeof(DATA_BASE_s)) == (DATA_BLOCK_ID_MAX)), "Database array length error");

    /* Iterate over database and set respective read/write pointer for each database entry */
    for (uint16_t i = 0u; i < data_baseHeader.nrDatabaseEntries; i++) {
        /* Initialize database entry with 0, set write pointer to start of database entry */
        uint8_t *pStartDatabaseEntryWR = (uint8_t *)data_baseHeader.pDatabase[i].pDatabaseEntry;

        /* Start after uniqueId entry. Set j'th byte to zero in database entry */
        for (uint32_t j = 0u; j < (data_baseHeader.pDatabase + i)->datalength; j++) {
            if (j >= sizeof(DATA_BLOCK_ID_e)) {
                *pStartDatabaseEntryWR = 0;
            }
            pStartDatabaseEntryWR++;
        }
    }

    /* Configure link between uniqueId and database entry array position */
    for (uint16_t databaseEntry = 0u; databaseEntry < data_baseHeader.nrDatabaseEntries; databaseEntry++) {
        /* Get pointer to database header entry */
        DATA_BLOCK_HEADER_s *pHeader = (DATA_BLOCK_HEADER_s *)data_baseHeader.pDatabase[databaseEntry].pDatabaseEntry;
        /*  make sure that the database entry is not a null pointer (which would happen if an entry is missing
            despite the ID existing) */
        FAS_ASSERT(pHeader != NULL_PTR);
        /* Get uniqueId */
        DATA_BLOCK_ID_e blockId = pHeader->uniqueId;
        if ((blockId < DATA_BLOCK_ID_MAX) && (databaseEntry < DATA_BLOCK_ID_MAX)) {
            uniqueIdToDatabaseEntry[blockId] =
                databaseEntry; /* e.g. uniqueIdToDatabaseEntry[<some id>] = configured database entry index */
        } else {
            /* Configuration error -> set retval to #STD_NOT_OK */
            retval = STD_NOT_OK;
        }
    }

    if (ftsk_databaseQueue == NULL_PTR) {
        /* Failed to create the queue */
        retval = STD_NOT_OK;
    }
    return retval;
}

void DATA_Task(void) {
    DATA_QUEUE_MESSAGE_s receiveMessage;

    if (ftsk_databaseQueue != NULL_PTR) {
        /* scan queue and wait for a message up to a maximum amount of 1ms (block time) */
        if (OS_ReceiveFromQueue(ftsk_databaseQueue, (&receiveMessage), 1u) == OS_SUCCESS) {
            /* plausibility check, error if first pointer NULL_PTR */
            FAS_ASSERT(receiveMessage.pDatabaseEntry[0] != NULL_PTR);
            /* Iterate over pointer array and handle all access operations if pointer != NULL_PTR */
            for (uint8_t queueEntry = 0u; queueEntry < DATA_MAX_ENTRIES_PER_ACCESS; queueEntry++) {
                if (receiveMessage.pDatabaseEntry[queueEntry] != NULL_PTR) {
                    /* pointer to passed database struct */
                    void *pPassedDataStruct = receiveMessage.pDatabaseEntry[queueEntry];
                    /* Get access type (read or write) of passed data struct */
                    DATA_BLOCK_ACCESS_TYPE_e accesstype = receiveMessage.accesstype;

                    /* Get pointer to database header entry */
                    DATA_BLOCK_HEADER_s *pHeader1 = (DATA_BLOCK_HEADER_s *)receiveMessage.pDatabaseEntry[queueEntry];
                    uint16_t entryIndex           = uniqueIdToDatabaseEntry[(uint16_t)pHeader1->uniqueId];
                    /* Pointer to database struct representation of passed struct */
                    void *pDatabaseStruct = (void *)data_baseHeader.pDatabase[entryIndex].pDatabaseEntry;
                    /* Get datalength of database entry */
                    uint32_t datalength = data_baseHeader.pDatabase[entryIndex].datalength;

                    /* Copy data either into database or passed database struct */
                    if (accesstype == DATA_WRITE_ACCESS) {
                        /* Pointer on datablock header of passed struct */
                        DATA_BLOCK_HEADER_s *pHeader = (DATA_BLOCK_HEADER_s *)pPassedDataStruct;
                        /* Update timestamps in passed database struct and then copy this struct into database */
                        pHeader->previousTimestamp = pHeader->timestamp;
                        pHeader->timestamp         = OS_GetTickCount();
                        /* Copy passed struct in database struct */
                        /* memcpy has no return value therefore there is nothing to check: casting to void */
                        (void)memcpy(pDatabaseStruct, pPassedDataStruct, datalength);
                    } else if (accesstype == DATA_READ_ACCESS) {
                        /* Copy database entry in passed struct */
                        /* memcpy has no return value therefore there is nothing to check: casting to void */
                        (void)memcpy(pPassedDataStruct, pDatabaseStruct, datalength);
                    } else {
                        /* invalid database operation */
                        FAS_ASSERT(FAS_TRAP);
                    }
                }
            }
        }
    }
}

void DATA_DummyFunction(void) {
}

STD_RETURN_TYPE_e DATA_Read_1_DataBlock(void *pDataToReceiver0) {
    /* Call write function with maximum number of database entries to prevent duplicated code */
    return DATA_Read_4_DataBlocks(pDataToReceiver0, NULL_PTR, NULL_PTR, NULL_PTR);
}

STD_RETURN_TYPE_e DATA_Read_2_DataBlocks(void *pDataToReceiver0, void *pDataToReceiver1) {
    /* Call write function with maximum number of database entries to prevent duplicated code */
    return DATA_Read_4_DataBlocks(pDataToReceiver0, pDataToReceiver1, NULL_PTR, NULL_PTR);
}

STD_RETURN_TYPE_e DATA_Read_3_DataBlocks(void *pDataToReceiver0, void *pDataToReceiver1, void *pDataToReceiver2) {
    /* Call write function with maximum number of database entries to prevent duplicated code */
    return DATA_Read_4_DataBlocks(pDataToReceiver0, pDataToReceiver1, pDataToReceiver2, NULL_PTR);
}

STD_RETURN_TYPE_e DATA_Read_4_DataBlocks(
    void *pDataToReceiver0,
    void *pDataToReceiver1,
    void *pDataToReceiver2,
    void *pDataToReceiver3) {
    STD_RETURN_TYPE_e retval = STD_NOT_OK;
    DATA_QUEUE_MESSAGE_s data_send_msg;
    TickType_t queuetimeout;

    queuetimeout = DATA_QUEUE_TIMEOUT_MS / portTICK_RATE_MS;
    if (queuetimeout == 0) {
        queuetimeout = 1;
    }

    /* prepare send message with attributes of data block */
    data_send_msg.pDatabaseEntry[0] = pDataToReceiver0;
    data_send_msg.pDatabaseEntry[1] = pDataToReceiver1;
    data_send_msg.pDatabaseEntry[2] = pDataToReceiver2;
    data_send_msg.pDatabaseEntry[3] = pDataToReceiver3;
    data_send_msg.accesstype        = DATA_READ_ACCESS;

    /* Send a pointer to a message object and */
    /* maximum block time: queuetimeout */
    if (OS_SendToBackOfQueue(ftsk_databaseQueue, (void *)&data_send_msg, queuetimeout) == OS_SUCCESS) {
        retval = STD_OK;
    }
    return retval;
}

STD_RETURN_TYPE_e DATA_Write_1_DataBlock(void *pDataFromSender0) {
    /* Call write function with maximum number of database entries to prevent duplicated code */
    return DATA_Write_4_DataBlocks(pDataFromSender0, NULL_PTR, NULL_PTR, NULL_PTR);
}

STD_RETURN_TYPE_e DATA_Write_2_DataBlocks(void *pDataFromSender0, void *pDataFromSender1) {
    /* Call write function with maximum number of database entries to prevent duplicated code */
    return DATA_Write_4_DataBlocks(pDataFromSender0, pDataFromSender1, NULL_PTR, NULL_PTR);
}

STD_RETURN_TYPE_e DATA_Write_3_DataBlocks(void *pDataFromSender0, void *pDataFromSender1, void *pDataFromSender2) {
    /* Call write function with maximum number of database entries to prevent duplicated code */
    return DATA_Write_4_DataBlocks(pDataFromSender0, pDataFromSender1, pDataFromSender2, NULL_PTR);
}

STD_RETURN_TYPE_e DATA_Write_4_DataBlocks(
    void *pDataFromSender0,
    void *pDataFromSender1,
    void *pDataFromSender2,
    void *pDataFromSender3) {
    STD_RETURN_TYPE_e retval = STD_NOT_OK;
    DATA_QUEUE_MESSAGE_s data_send_msg;
    TickType_t queuetimeout;

    queuetimeout = DATA_QUEUE_TIMEOUT_MS / portTICK_RATE_MS;
    if (queuetimeout == (TickType_t)0) {
        queuetimeout = 1;
    }

    /* prepare send message with attributes of data block */
    data_send_msg.pDatabaseEntry[0] = pDataFromSender0;
    data_send_msg.pDatabaseEntry[1] = pDataFromSender1;
    data_send_msg.pDatabaseEntry[2] = pDataFromSender2;
    data_send_msg.pDatabaseEntry[3] = pDataFromSender3;

    data_send_msg.accesstype = DATA_WRITE_ACCESS;
    /* Send a pointer to a message object and
       maximum block time: queuetimeout */
    if (OS_SendToBackOfQueue(ftsk_databaseQueue, (void *)&data_send_msg, queuetimeout) == OS_SUCCESS) {
        retval = STD_OK;
    }
    return retval;
}

extern void DATA_ExecuteDataBIST(void) {
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
