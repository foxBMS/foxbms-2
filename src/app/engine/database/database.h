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
 * @file    database.h
 * @author  foxBMS Team
 * @date    2015-08-18 (date of creation)
 * @updated 2021-07-23 (date of last update)
 * @ingroup ENGINE
 * @prefix  DATA
 *
 * @brief   Database module header
 *
 * @details Provides interfaces to database module
 *
 */

#ifndef FOXBMS__DATABASE_H_
#define FOXBMS__DATABASE_H_

/*========== Includes =======================================================*/
#include "database_cfg.h"

#include "database_helper.h"
#include "os.h"

/*========== Macros and Definitions =========================================*/

/**
 * Maximum number of database entries that can be read or written during one
 * access call to the database
 */
#define DATA_MAX_ENTRIES_PER_ACCESS (4u)

/** helper macro for the variadic macros for read and write functions */
#define GET_MACRO(_1, _2, _3, _4, NAME, ...) NAME
/** variadic macro for read access to the database */
#define DATA_READ_DATA(...)     \
    GET_MACRO(                  \
        __VA_ARGS__,            \
        DATA_Read_4_DataBlocks, \
        DATA_Read_3_DataBlocks, \
        DATA_Read_2_DataBlocks, \
        DATA_Read_1_DataBlock,  \
        DATA_DummyFunction)     \
    (__VA_ARGS__)
/** variadic macro for write access to the database */
#define DATA_WRITE_DATA(...)     \
    GET_MACRO(                   \
        __VA_ARGS__,             \
        DATA_Write_4_DataBlocks, \
        DATA_Write_3_DataBlocks, \
        DATA_Write_2_DataBlocks, \
        DATA_Write_1_DataBlock,  \
        DATA_DummyFunction)      \
    (__VA_ARGS__)

/**
 * @brief data block access types (read or write)
 */
typedef enum DATA_BLOCK_ACCESS_TYPE {
    DATA_WRITE_ACCESS, /**< write access to data block   */
    DATA_READ_ACCESS,  /**< read access to data block    */
} DATA_BLOCK_ACCESS_TYPE_e;

/** dummy value for the built-in self-test (alternating bit pattern) */
#define DATA_DUMMY_VALUE_UINT8_T_ALTERNATING_BIT_PATTERN ((uint8_t)0xAAu)

/**
 * struct for database queue, contains pointer to data, database entry and access type
 */
typedef struct DATA_QUEUE_MESSAGE {
    DATA_BLOCK_ACCESS_TYPE_e accesstype;               /*!< read or write access type */
    void *pDatabaseEntry[DATA_MAX_ENTRIES_PER_ACCESS]; /*!< reference by general pointer */
} DATA_QUEUE_MESSAGE_s;

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/
/**
 * @brief   Dummy void function of the database module
 * @details This function is needed in the database module in order to
 *          implement the #DATA_READ_DATA() and #DATA_WRITE_DATA() in a ISO C99
 *          standard compatible way. The invocation of a macro that accepts a
 *          variable number of arguments (and this is the case for
 *          #DATA_READ_DATA() and #DATA_WRITE_DATA()) needs more arguments in the
 *          invocation than there are parameters in the macro definition. For
 *          the most simple case, that #DATA_READ_DATA() and #DATA_WRITE_DATA() are
 *          only called with one argument, a violation of the standard would
 *          appear if the #DATA_DummyFunction() would be omitted:
 *          GET_MACRO(databaseVaribale,
 *                    DATA_Read_4_DataBlocks,
 *                    DATA_Read_3_DataBlocks,
 *                    DATA_Read_2_DataBlocks,
 *                    DATA_Read_2_DataBlocks,
 *                    DATA_Read_1_DataBlock)(databaseVaribale)
 *          So the macro invocation has six parameters, but it needs seven and
 *          an ISO C99 violation would appear. Adding the #DATA_DummyFunction()
 *          fixes this violation.
 *          For details see 6.10.3 (paragraph 4) of the ISO C99 standard.
 */
extern void DATA_DummyFunction(void);

/**
 * @brief   Initialization of database manager
 *
 * @return  #STD_OK if initialization successful, otherwise #STD_NOT_OK
 */
extern STD_RETURN_TYPE_e DATA_Init(void);

/**
 * @brief   trigger of database manager
 * @details TODO
 */
extern void DATA_Task(void);

/**
 * @brief   Stores a datablock in database
 * @details This function stores passed data in database and updates timestamp
 *          and previous timestamp in passed struct
 * @warning Do not call this function from inside a critical section, as it is
 *          computationally complex.
 * @param[in,out]  pDataFromSender0 (type: void *)
 * @return  #STD_OK if access was successful, otherwise #STD_NOT_OK
 */
extern STD_RETURN_TYPE_e DATA_Write_1_DataBlock(void *pDataFromSender0);

/**
 * @brief   Stores a datablock in database
 * @details This function stores passed data in database and updates timestamp
 *          and previous timestamp in passed struct
 * @warning Do not call this function from inside a critical section, as it is
 *          computationally complex.
 * @param[in,out]  pDataFromSender0 (type: void *)
 * @param[in,out]  pDataFromSender1 (type: void *)
 * @return  #STD_OK if access was successful, otherwise #STD_NOT_OK
 */
extern STD_RETURN_TYPE_e DATA_Write_2_DataBlocks(void *pDataFromSender0, void *pDataFromSender1);
/**
 * @brief   Stores a datablock in database
 * @details This function stores passed data in database and updates timestamp
 *          and previous timestamp in passed struct
 * @warning Do not call this function from inside a critical section, as it is
 *          computationally complex.
 * @param[in,out]  pDataFromSender0 (type: void *)
 * @param[in,out]  pDataFromSender1 (type: void *)
 * @param[in,out]  pDataFromSender2 (type: void *)
 * @return  #STD_OK if access was successful, otherwise #STD_NOT_OK
 */
extern STD_RETURN_TYPE_e DATA_Write_3_DataBlocks(
    void *pDataFromSender0,
    void *pDataFromSender1,
    void *pDataFromSender2);
/**
 * @brief   Stores a datablock in database
 * @details This function stores passed data in database and updates timestamp
 *          and previous timestamp in passed struct
 * @warning Do not call this function from inside a critical section, as it is
 *          computationally complex.
 * @param[in,out]  pDataFromSender0 (type: void *)
 * @param[in,out]  pDataFromSender1 (type: void *)
 * @param[in,out]  pDataFromSender2 (type: void *)
 * @param[in,out]  pDataFromSender3 (type: void *)
 * @return  #STD_OK if access was successful, otherwise #STD_NOT_OK
 */
extern STD_RETURN_TYPE_e DATA_Write_4_DataBlocks(
    void *pDataFromSender0,
    void *pDataFromSender1,
    void *pDataFromSender2,
    void *pDataFromSender3);

/**
 * @brief   Reads a datablock in database by value.
 * @details This function reads data from database and copy this content in
 *          passed struct
 * @warning Do not call this function from inside a critical section, as it is
 *          computationally complex.
 * @param[out]  pDataToReceiver0 (type: void *)
 * @return  #STD_OK if access was successful, otherwise #STD_NOT_OK
 */
extern STD_RETURN_TYPE_e DATA_Read_1_DataBlock(void *pDataToReceiver0);
/**
 * @brief   Reads a datablock in database by value.
 * @details This function reads data from database and copy this content in
 *          passed struct
 * @warning Do not call this function from inside a critical section, as it is
 *          computationally complex.
 * @param[out]  pDataToReceiver0 (type: void *)
 * @param[out]  pDataToReceiver1 (type: void *)
 * @return  #STD_OK if access was successful, otherwise #STD_NOT_OK
 */
extern STD_RETURN_TYPE_e DATA_Read_2_DataBlocks(void *pDataToReceiver0, void *pDataToReceiver1);
/**
 * @brief   Reads a datablock in database by value.
 * @details This function reads data from database and copy this content in
 *          passed struct
 * @warning Do not call this function from inside a critical section, as it is
 *          computationally complex.
 * @param[out]  pDataToReceiver0 (type: void *)
 * @param[out]  pDataToReceiver1 (type: void *)
 * @param[out]  pDataToReceiver2 (type: void *)
 * @return  #STD_OK if access was successful, otherwise #STD_NOT_OK
 */
extern STD_RETURN_TYPE_e DATA_Read_3_DataBlocks(void *pDataToReceiver0, void *pDataToReceiver1, void *pDataToReceiver2);
/**
 * @brief   Reads a datablock in database by value.
 * @details This function reads data from database and copy this content in
 *          passed struct
 * @warning Do not call this function from inside a critical section, as it is
 *          computationally complex.
 * @param[out]  pDataToReceiver0 (type: void *)
 * @param[out]  pDataToReceiver1 (type: void *)
 * @param[out]  pDataToReceiver2 (type: void *)
 * @param[out]  pDataToReceiver3 (type: void *)
 * @return  #STD_OK if access was successful, otherwise #STD_NOT_OK
 */
extern STD_RETURN_TYPE_e DATA_Read_4_DataBlocks(
    void *pDataToReceiver0,
    void *pDataToReceiver1,
    void *pDataToReceiver2,
    void *pDataToReceiver3);

/**
 * @brief   Executes a built-in self-test for the database module
 * @details This test writes and reads a database entry in order to check that
 *          the database module is working as expected. If the test fails, it
 *          will fail an assertion.
 */
extern void DATA_ExecuteDataBIST(void);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/

#endif /* FOXBMS__DATABASE_H_ */
