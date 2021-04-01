/**
 *
 * @copyright &copy; 2010 - 2021, Fraunhofer-Gesellschaft zur Foerderung der
 *  angewandten Forschung e.V. All rights reserved.
 *
 * BSD 3-Clause License
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1.  Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of the copyright holder nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * We kindly request you to use one or more of the following phrases to refer
 * to foxBMS in your hardware, software, documentation or advertising
 * materials:
 *
 * &Prime;This product uses parts of foxBMS&reg;&Prime;
 *
 * &Prime;This product includes parts of foxBMS&reg;&Prime;
 *
 * &Prime;This product is derived from foxBMS&reg;&Prime;
 *
 */

/**
 * @file    test_database.c
 * @author  foxBMS Team
 * @date    2020-04-01 (date of creation)
 * @updated 2020-05-25 (date of last update)
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
#include "Mockmpu_prototypes.h"
#include "Mockos.h"

#include "database_cfg.h"

#include "database.h"

/*========== Definitions and Implementations for Unit Test ==================*/

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

void testDATA_DatabaseEntryUpdatedAtLeastOnce(void) {
    DATA_BLOCK_CELL_VOLTAGE_s databaseEntry = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE};

    /* Database entry has been updated once, after 10ms */
    databaseEntry.header.timestamp         = 10u;
    databaseEntry.header.previousTimestamp = 0u;
    TEST_ASSERT_TRUE(DATA_DatabaseEntryUpdatedAtLeastOnce((void *)&databaseEntry));

    /* Database entry has been updated twice, first after 10ms, then after 50ms */
    databaseEntry.header.timestamp         = 60u;
    databaseEntry.header.previousTimestamp = 10u;
    TEST_ASSERT_TRUE(DATA_DatabaseEntryUpdatedAtLeastOnce((void *)&databaseEntry));

    /* Database entry has been updated three times, first after 10ms, then after 50ms, then after 10ms */
    databaseEntry.header.timestamp         = 70u;
    databaseEntry.header.previousTimestamp = 60u;
    TEST_ASSERT_TRUE(DATA_DatabaseEntryUpdatedAtLeastOnce((void *)&databaseEntry));

    /* Database entry has never been updated */
    databaseEntry.header.timestamp         = 0u;
    databaseEntry.header.previousTimestamp = 0u;
    TEST_ASSERT_FALSE(DATA_DatabaseEntryUpdatedAtLeastOnce((void *)&databaseEntry));
}

void testDATA_DatabaseEntryUpdatedRecently(void) {
    DATA_BLOCK_CELL_VOLTAGE_s databaseEntry = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE};

    /* Always check always if database entry has been updated within the last 100ms */
    uint32_t timeDifference = 100u;

    /* Time difference: 50ms -> true */
    databaseEntry.header.timestamp = 50u;
    OS_GetTickCount_ExpectAndReturn(100u);
    TEST_ASSERT_TRUE(DATA_DatabaseEntryUpdatedRecently((void *)&databaseEntry, timeDifference));

    /* Time difference: 100ms -> true, but never updated */
    databaseEntry.header.timestamp = 0u;
    OS_GetTickCount_ExpectAndReturn(100u);
    TEST_ASSERT_FALSE(DATA_DatabaseEntryUpdatedRecently((void *)&databaseEntry, timeDifference));

    /* Time difference: 101ms -> false */
    databaseEntry.header.timestamp = 0u;
    OS_GetTickCount_ExpectAndReturn(101u);
    TEST_ASSERT_FALSE(DATA_DatabaseEntryUpdatedRecently((void *)&databaseEntry, timeDifference));

    /* Time difference: 63ms -> true */
    databaseEntry.header.timestamp = 4937u;
    OS_GetTickCount_ExpectAndReturn(5000u);
    TEST_ASSERT_TRUE(DATA_DatabaseEntryUpdatedRecently((void *)&databaseEntry, timeDifference));

    /* Time difference: 50ms -> true */
    databaseEntry.header.timestamp = UINT32_MAX;
    OS_GetTickCount_ExpectAndReturn(50u);
    TEST_ASSERT_TRUE(DATA_DatabaseEntryUpdatedRecently((void *)&databaseEntry, timeDifference));

    /* Time difference: 100ms -> true */
    databaseEntry.header.timestamp = UINT32_MAX - 50u;
    OS_GetTickCount_ExpectAndReturn(49u);
    TEST_ASSERT_TRUE(DATA_DatabaseEntryUpdatedRecently((void *)&databaseEntry, timeDifference));

    /* Time difference: 101ms -> false */
    databaseEntry.header.timestamp = UINT32_MAX - 50u;
    OS_GetTickCount_ExpectAndReturn(50u);
    TEST_ASSERT_FALSE(DATA_DatabaseEntryUpdatedRecently((void *)&databaseEntry, timeDifference));

    /* Time difference: UINT32_MAX - 50 -> false */
    databaseEntry.header.timestamp = 50u;
    OS_GetTickCount_ExpectAndReturn(UINT32_MAX);
    TEST_ASSERT_FALSE(DATA_DatabaseEntryUpdatedRecently((void *)&databaseEntry, timeDifference));
}

void testDATA_DatabaseEntryUpdatedWithinInterval(void) {
    DATA_BLOCK_CELL_VOLTAGE_s databaseEntry = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE};

    /* Always check always if database entry has been periodically updated within the last 100ms */
    uint32_t timeDifference = 100u;

    /* Time difference timestamp - systick: 40ms -> true
     * Time difference timestamp - previous timestamp:  50ms -> true */
    databaseEntry.header.timestamp         = 60u;
    databaseEntry.header.previousTimestamp = 10u;
    OS_GetTickCount_ExpectAndReturn(100u);
    TEST_ASSERT_TRUE(DATA_DatabaseEntryUpdatedWithinInterval((void *)&databaseEntry, timeDifference));

    /* Within time interval but never updated -> false */
    databaseEntry.header.timestamp         = 0u;
    databaseEntry.header.previousTimestamp = 0u;
    OS_GetTickCount_ExpectAndReturn(50u);
    TEST_ASSERT_FALSE(DATA_DatabaseEntryUpdatedWithinInterval((void *)&databaseEntry, timeDifference));

    /* Time difference timestamp - systick: 100ms -> true
     * Time difference timestamp - previous timestamp:  40ms -> true */
    databaseEntry.header.timestamp         = 80u;
    databaseEntry.header.previousTimestamp = 40u;
    OS_GetTickCount_ExpectAndReturn(180u);
    TEST_ASSERT_TRUE(DATA_DatabaseEntryUpdatedWithinInterval((void *)&databaseEntry, timeDifference));

    /* Time difference timestamp - systick: 101ms -> false
     * Time difference timestamp - previous timestamp:  40ms -> true */
    databaseEntry.header.timestamp         = 80u;
    databaseEntry.header.previousTimestamp = 40u;
    OS_GetTickCount_ExpectAndReturn(181);
    TEST_ASSERT_FALSE(DATA_DatabaseEntryUpdatedWithinInterval((void *)&databaseEntry, timeDifference));

    /* Time difference timestamp - systick: 50ms -> true
     * Time difference timestamp - previous timestamp:  110ms -> false */
    databaseEntry.header.timestamp         = 150u;
    databaseEntry.header.previousTimestamp = 40u;
    OS_GetTickCount_ExpectAndReturn(200u);
    TEST_ASSERT_FALSE(DATA_DatabaseEntryUpdatedWithinInterval((void *)&databaseEntry, timeDifference));

    /* Time difference timestamp - systick: 100ms -> true
     * Time difference timestamp - previous timestamp:  100ms -> true */
    databaseEntry.header.timestamp         = 150u;
    databaseEntry.header.previousTimestamp = 50u;
    OS_GetTickCount_ExpectAndReturn(250u);
    TEST_ASSERT_TRUE(DATA_DatabaseEntryUpdatedWithinInterval((void *)&databaseEntry, timeDifference));

    /* Time difference timestamp - systick: 100ms -> true
     * Time difference timestamp - previous timestamp:  100ms -> true */
    databaseEntry.header.timestamp         = UINT32_MAX - 50u;
    databaseEntry.header.previousTimestamp = UINT32_MAX - 150u;
    OS_GetTickCount_ExpectAndReturn(49u);
    TEST_ASSERT_TRUE(DATA_DatabaseEntryUpdatedWithinInterval((void *)&databaseEntry, timeDifference));

    /* Time difference timestamp - systick: 101ms -> false
     * Time difference timestamp - previous timestamp:  100ms -> true */
    databaseEntry.header.timestamp         = UINT32_MAX - 50u;
    databaseEntry.header.previousTimestamp = UINT32_MAX - 150u;
    OS_GetTickCount_ExpectAndReturn(50u);
    TEST_ASSERT_FALSE(DATA_DatabaseEntryUpdatedWithinInterval((void *)&databaseEntry, timeDifference));

    /* Time difference timestamp - systick: 100ms -> true
     * Time difference timestamp - previous timestamp:  100ms -> true */
    databaseEntry.header.timestamp         = 49u;
    databaseEntry.header.previousTimestamp = UINT32_MAX - 50u;
    OS_GetTickCount_ExpectAndReturn(150u);
    TEST_ASSERT_FALSE(DATA_DatabaseEntryUpdatedWithinInterval((void *)&databaseEntry, timeDifference));

    /* Time difference timestamp - systick: 100ms -> true
     * Time difference timestamp - previous timestamp:  101ms -> false */
    databaseEntry.header.timestamp         = UINT32_MAX - 50u;
    databaseEntry.header.previousTimestamp = UINT32_MAX - 151u;
    OS_GetTickCount_ExpectAndReturn(49u);
    TEST_ASSERT_FALSE(DATA_DatabaseEntryUpdatedWithinInterval((void *)&databaseEntry, timeDifference));

    /* Time difference timestamp - systick: 100ms -> true
     * Time difference timestamp - previous timestamp:  101ms -> false */
    databaseEntry.header.timestamp         = 50u;
    databaseEntry.header.previousTimestamp = UINT32_MAX - 50u;
    OS_GetTickCount_ExpectAndReturn(150u);
    TEST_ASSERT_FALSE(DATA_DatabaseEntryUpdatedWithinInterval((void *)&databaseEntry, timeDifference));

    /* Time difference: UINT32_MAX - 50 -> false */
    databaseEntry.header.timestamp = 50u;
    OS_GetTickCount_ExpectAndReturn(UINT32_MAX);
    TEST_ASSERT_FALSE(DATA_DatabaseEntryUpdatedWithinInterval((void *)&databaseEntry, timeDifference));
}

/*========== Test Cases =====================================================*/
