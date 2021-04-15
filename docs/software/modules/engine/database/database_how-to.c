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
 * @file    database_how-to.c
 * @author  foxBMS Team
 * @date    2021-04-12 (date of creation)
 * @updated 2021-04-12 (date of last update)
 * @ingroup SOME_GROUP
 * @prefix  DATA
 *
 * @brief   Implementation of some software
 *
 */

/*========== Includes =======================================================*/
#include "general.h"

/*========== Macros and Definitions =========================================*/
/** mock-up of the variadic macro for read access to the database*/
#define DATA_READ_DATA(...)
/** mock-up of the variadic macro for write access to the database */
#define DATA_WRITE_DATA(...)

/** configuration struct of database channel (data block) as
 *  defined in database_cfg.h */
typedef struct DATA_BASE {
    void *pDatabaseEntry; /*!< pointer to the database entry */
    uint16_t datalength;  /*!< length of the entry */
} DATA_BASE_s;

/* example-data-block-id-start */
/** data block identification numbers */
typedef enum DATA_BLOCK_ID {
    DATA_BLOCK_ID_EXAMPLE, /**< some example database entry */
    /* ...
     * ...
     * ...
     */
    DATA_BLOCK_ID_EXAMPLE_0, /**< some more examples */
    DATA_BLOCK_ID_EXAMPLE_1, /**< some more examples */
    DATA_BLOCK_ID_EXAMPLE_2, /**< some more examples */
    DATA_BLOCK_ID_EXAMPLE_3, /**< some more examples */
    DATA_BLOCK_ID_MAX,       /**< DO NOT CHANGE, MUST BE THE LAST ENTRY */
} DATA_BLOCK_ID_e;
/* example-data-block-id-end */

/** typedef for data block header as defined in database_cfg.h */
typedef struct DATA_BLOCKHEADER {
    DATA_BLOCK_ID_e uniqueId;   /*!< uniqueId of database entry */
    uint32_t timestamp;         /*!< timestamp of last database update */
    uint32_t previousTimestamp; /*!< timestamp of previous database update */
} DATA_BLOCK_HEADER_s;

/* example-data-block-typedef-start */
/**
 * data block struct of example
 */
typedef struct DATA_BLOCK_EXAMPLE {
    /* This struct needs to be at the beginning of every database entry. During
    * the initialization of a database struct, uniqueId must be set to the
    * respective database entry representation in enum DATA_BLOCK_ID_e. */
    DATA_BLOCK_HEADER_s header;
    uint16_t dummyValue;
} DATA_BLOCK_EXAMPLE_s;
/* example-data-block-typedef-end */

/**
 * data block struct of example_0
 */
typedef struct DATA_BLOCK_EXAMPLE_0 {
    /* This struct needs to be at the beginning of every database entry. During
    * the initialization of a database struct, uniqueId must be set to the
    * respective database entry representation in enum DATA_BLOCK_ID_e. */
    DATA_BLOCK_HEADER_s header;
    uint16_t dummyValue;
} DATA_BLOCK_EXAMPLE_0_s;

/**
 * data block struct of example_1
 */
typedef struct DATA_BLOCK_EXAMPLE_1 {
    /* This struct needs to be at the beginning of every database entry. During
    * the initialization of a database struct, uniqueId must be set to the
    * respective database entry representation in enum DATA_BLOCK_ID_e. */
    DATA_BLOCK_HEADER_s header;
    uint16_t dummyValue;
} DATA_BLOCK_EXAMPLE_1_s;

/**
 * data block struct of example_2
 */
typedef struct DATA_BLOCK_EXAMPLE_2 {
    /* This struct needs to be at the beginning of every database entry. During
    * the initialization of a database struct, uniqueId must be set to the
    * respective database entry representation in enum DATA_BLOCK_ID_e. */
    DATA_BLOCK_HEADER_s header;
    uint16_t dummyValue;
} DATA_BLOCK_EXAMPLE_2_s;

/**
 * data block struct of example_3
 */
typedef struct DATA_BLOCK_EXAMPLE_3 {
    /* This struct needs to be at the beginning of every database entry. During
    * the initialization of a database struct, uniqueId must be set to the
    * respective database entry representation in enum DATA_BLOCK_ID_e. */
    DATA_BLOCK_HEADER_s header;
    uint16_t dummyValue;
} DATA_BLOCK_EXAMPLE_3_s;

/*========== Static Constant and Variable Definitions =======================*/

/* example-data-block-variable-start */
static DATA_BLOCK_EXAMPLE_s data_blockExample = {.header.uniqueId = DATA_BLOCK_ID_EXAMPLE};
/* example-data-block-variable-end */

/* example-data-database-start */
/**
 * @brief   channel configuration of database (data blocks)
 * @details all data block managed by database are listed here (address, size,
 *          consistency type)
 */
DATA_BASE_s data_database[] = {
    {(void *)(&data_blockExample), sizeof(DATA_BLOCK_EXAMPLE_s)},
    /* ...
     * ...
     * ...
     */
};
/* example-data-database-end */

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

void DATA_ExampleReadData(void) {
    /* example-data-read-data-start */
    /* Create local structure of database entry */
    DATA_BLOCK_EXAMPLE_s tableLocalExampleEntry = {.header.uniqueId = DATA_BLOCK_ID_EXAMPLE};
    /* Read data from database entry and update local variable */
    DATA_READ_DATA(&tableLocalExampleEntry);
    /* example-data-read-data-end */
}

void DATA_ExampleReadDataMultipleEntries(void) {
    /* example-data-read-multiple-data-entries-start */
    DATA_BLOCK_EXAMPLE_0_s tableLocalExampleEntry0 = {.header.uniqueId = DATA_BLOCK_ID_EXAMPLE_0};
    DATA_BLOCK_EXAMPLE_1_s tableLocalExampleEntry1 = {.header.uniqueId = DATA_BLOCK_ID_EXAMPLE_1};
    DATA_BLOCK_EXAMPLE_2_s tableLocalExampleEntry2 = {.header.uniqueId = DATA_BLOCK_ID_EXAMPLE_2};
    DATA_BLOCK_EXAMPLE_3_s tableLocalExampleEntry3 = {.header.uniqueId = DATA_BLOCK_ID_EXAMPLE_3};
    /* Read data from 4 database entries and update local variables */
    DATA_READ_DATA(
        &tableLocalExampleEntry0, &tableLocalExampleEntry1, &tableLocalExampleEntry2, &tableLocalExampleEntry3);
    /* example-data-read-multiple-data-entries-end */
}

void DATA_ExampleWriteData(void) {
    /* example-data-write-data-start */
    /* Create local structure of database entry */
    DATA_BLOCK_EXAMPLE_s tableLocalExampleEntry = {.header.uniqueId = DATA_BLOCK_ID_EXAMPLE};
    /* Set dummy value to 100 */
    tableLocalExampleEntry.dummyValue = 100;
    /* Update database entry with changed dummy value */
    DATA_WRITE_DATA(&tableLocalExampleEntry);
    /* example-data-write-data-end */
}

void DATA_ExampleWriteDataMultipleEntries(void) {
    /* example-data-write-multiple-data-entries-start */
    DATA_BLOCK_EXAMPLE_0_s tableLocalExampleEntry0 = {.header.uniqueId = DATA_BLOCK_ID_EXAMPLE_0};
    DATA_BLOCK_EXAMPLE_1_s tableLocalExampleEntry1 = {.header.uniqueId = DATA_BLOCK_ID_EXAMPLE_1};
    DATA_BLOCK_EXAMPLE_2_s tableLocalExampleEntry2 = {.header.uniqueId = DATA_BLOCK_ID_EXAMPLE_2};
    DATA_BLOCK_EXAMPLE_3_s tableLocalExampleEntry3 = {.header.uniqueId = DATA_BLOCK_ID_EXAMPLE_3};
    /* Set dummy values to 100 */
    tableLocalExampleEntry0.dummyValue = 100;
    tableLocalExampleEntry1.dummyValue = 100;
    tableLocalExampleEntry2.dummyValue = 100;
    tableLocalExampleEntry3.dummyValue = 100;
    /* Update database entries with changed dummy values */
    DATA_WRITE_DATA(
        &tableLocalExampleEntry0, &tableLocalExampleEntry1, &tableLocalExampleEntry2, &tableLocalExampleEntry3);
    /* example-data-write-multiple-data-entries-end */
}

/*========== Extern Function Implementations ================================*/

/*========== Externalized Static Function Implementations (Unit Test) =======*/
