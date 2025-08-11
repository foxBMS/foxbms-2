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
 * @file    test_dp83869.c
 * @author  foxBMS Team
 * @date    2025-07-04 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Test of dp83869 phy driver
 * @details TODO:
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "MockHL_mdio.h"
#include "Mockdatabase.h"
#include "Mockio.h"
#include "Mockos.h"

#include "dp83869.h"
#include "test_assert_helper.h"

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("dp83869.c")

TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/io")
TEST_INCLUDE_PATH("../../src/app/driver/phy")

/*========== Definitions and Implementations for Unit Test ==================*/

#define TEST_MDIO_BASE_ADDRESS (0xFCF78900u)
#define TEST_PHY_ADDRESS       (0x1u)
#define TEST_PHY_REGCR         (0x0Du)
#define TEST_PHY_ADDAR         (0x0Eu)
#define TEST_PHY_GEN_CTRL      (0x1Fu)
#define TEST_PHY_BMCR          (0x0u)

static DATA_BLOCK_PHY_s tablePhy = {.header.uniqueId = DATA_BLOCK_ID_PHY};

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Helper Functions ===============================================*/
void helper_ExtendedAddressSpaceRegWrite(uint32_t registerNumber, uint16_t registerValue) {
    MDIOPhyRegWrite_Expect(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, TEST_PHY_REGCR, 0x001F);
    MDIOPhyRegWrite_Expect(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, TEST_PHY_ADDAR, registerNumber);
    MDIOPhyRegWrite_Expect(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, TEST_PHY_REGCR, 0x401F);
    MDIOPhyRegWrite_Expect(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, TEST_PHY_ADDAR, registerValue);
}

void helper_ExtendedAddressSpaceRegRead(uint32_t registerNumber, uint16_t *dataPtr, uint16_t *returnThruPtr) {
    MDIOPhyRegWrite_Expect(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, TEST_PHY_REGCR, 0x001F);
    MDIOPhyRegWrite_Expect(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, TEST_PHY_ADDAR, registerNumber);
    MDIOPhyRegWrite_Expect(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, TEST_PHY_REGCR, 0x401F);
    MDIOPhyRegRead_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, TEST_PHY_ADDAR, dataPtr, true);
    MDIOPhyRegRead_ReturnThruPtr_dataPtr(returnThruPtr);
}

void helper_SwRestart(uint16_t *restartComplete, uint16_t *n_restartComplete) {
    MDIOPhyRegWrite_Expect(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, TEST_PHY_GEN_CTRL, 0x4000u);
    /* Restart in progress */
    MDIOPhyRegRead_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, TEST_PHY_GEN_CTRL, restartComplete, true);
    MDIOPhyRegRead_ReturnThruPtr_dataPtr(n_restartComplete);
    /* Restart complete */
    MDIOPhyRegRead_ExpectAndReturn(
        TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, TEST_PHY_GEN_CTRL, n_restartComplete, true);
    MDIOPhyRegRead_ReturnThruPtr_dataPtr(restartComplete);
}

void helper_SwReset(uint16_t *resetComplete, uint16_t *n_resetComplete) {
    MDIOPhyRegWrite_Expect(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, TEST_PHY_GEN_CTRL, 0x8000u);
    /* Reset in progress */
    MDIOPhyRegRead_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, TEST_PHY_GEN_CTRL, resetComplete, true);
    MDIOPhyRegRead_ReturnThruPtr_dataPtr(n_resetComplete);
    /* Reset complete */
    MDIOPhyRegRead_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, TEST_PHY_GEN_CTRL, n_resetComplete, true);
    MDIOPhyRegRead_ReturnThruPtr_dataPtr(resetComplete);
}

void helper_HardwareReset(void) {
    IO_PinReset_Expect(&((gioPORT_t *)0xFFF7BC34U)->DOUT, 1u);
    OS_DelayTask_Expect(1u);
    IO_PinSet_Expect(&((gioPORT_t *)0xFFF7BC34U)->DOUT, 1u);
    OS_DelayTask_Expect(2u);
}

/*========== Test Cases =====================================================*/

/**
 * @brief   Testing static function #PHY_WriteExtendedAddressSpaceRegister
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/2: Invalid phy address &rarr; assert
 *            - AT2/2: Invalid register number &rarr; assert
 *          - Routine validation:
 *            - RT1/1: Function calls expected subroutine
 */
void testPHY_WriteExtendedAddressSpaceRegister(void) {
    uint32_t testRegisterNumber = 0x00FE;
    uint16_t testRegisterValue  = 0xE720;
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_PHY_WriteExtendedAddressSpaceRegister(TEST_MDIO_BASE_ADDRESS, 33u, 0x0u, 0x1u));
    /* ======= AT2/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(
        TEST_PHY_WriteExtendedAddressSpaceRegister(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, TEST_PHY_REGCR, 0x1u));
    TEST_ASSERT_FAIL_ASSERT(
        TEST_PHY_WriteExtendedAddressSpaceRegister(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, TEST_PHY_ADDAR, 0x1u));
    TEST_ASSERT_FAIL_ASSERT(
        TEST_PHY_WriteExtendedAddressSpaceRegister(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, 0xC1Au, 0x1u));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    helper_ExtendedAddressSpaceRegWrite(testRegisterNumber, testRegisterValue);

    /* ======= RT1/1: Call function under test */
    TEST_PHY_WriteExtendedAddressSpaceRegister(
        TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, testRegisterNumber, testRegisterValue);
}

/**
 * @brief   Testing static function #PHY_ReadExtendedAddressSpaceRegister
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/3: Invalid phy address &rarr; assert
 *            - AT2/3: Invalid register number &rarr; assert
 *            - AT2/3: Invalid data pointer &rarr; assert
 *          - Routine validation:
 *            - RT1/1: Function calls expected subroutine and succeeds
 *            - RT1/1: Function calls expected subroutine and fails
 */
void testPHY_ReadExtendedAddressSpaceRegister(void) {
    uint16_t testDataBuffer;
    uint32_t testRegisterNumber = 0x00FE;
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/3 ======= */
    TEST_ASSERT_FAIL_ASSERT(
        TEST_PHY_ReadExtendedAddressSpaceRegister(TEST_MDIO_BASE_ADDRESS, 33u, 0x0u, &testDataBuffer));
    /* ======= AT2/3 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_PHY_ReadExtendedAddressSpaceRegister(
        TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, TEST_PHY_REGCR, &testDataBuffer));
    TEST_ASSERT_FAIL_ASSERT(TEST_PHY_ReadExtendedAddressSpaceRegister(
        TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, TEST_PHY_ADDAR, &testDataBuffer));
    TEST_ASSERT_FAIL_ASSERT(
        TEST_PHY_ReadExtendedAddressSpaceRegister(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, 0xC1Au, &testDataBuffer));
    /* ======= AT3/3 ======= */
    TEST_ASSERT_FAIL_ASSERT(
        TEST_PHY_ReadExtendedAddressSpaceRegister(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, 0x0u, 0x0u));

    /* ======= Routine tests =============================================== */

    /* ======= RT1/2: Test implementation */
    helper_ExtendedAddressSpaceRegRead(testRegisterNumber, &testDataBuffer, &testDataBuffer);
    /* ======= RT1/2: Call function under test */
    const bool succeeded = TEST_PHY_ReadExtendedAddressSpaceRegister(
        TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, testRegisterNumber, &testDataBuffer);
    /* ======= RT1/2: Test output verification */
    TEST_ASSERT_EQUAL(true, succeeded);

    /* ======= RT2/2: Test implementation */
    MDIOPhyRegWrite_Expect(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, TEST_PHY_REGCR, 0x001F);
    MDIOPhyRegWrite_Expect(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, TEST_PHY_ADDAR, testRegisterNumber);
    MDIOPhyRegWrite_Expect(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, TEST_PHY_REGCR, 0x401F);
    MDIOPhyRegRead_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, TEST_PHY_ADDAR, &testDataBuffer, false);
    /* ======= RT2/2: Call function under test */
    const bool failed = TEST_PHY_ReadExtendedAddressSpaceRegister(
        TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, testRegisterNumber, &testDataBuffer);
    /* ======= RT2/2: Test output verification */
    TEST_ASSERT_EQUAL(false, failed);
}

/**
 * @brief   Testing static function #PHY_SelectSpeed
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: Invalid phy address &rarr; assert
 *          - Routine validation:
 *            - RT1/3: Function calls expected subroutine with speed 10 Mbps
 *            - RT2/3: Function calls expected subroutine with speed 100 Mbps
 *            - RT3/3: Function calls expected subroutine with speed 1000 Mbps
 */
void testPHY_SelectSpeed(void) {
    uint16_t testDataBuffer;
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_PHY_SelectSpeed(TEST_MDIO_BASE_ADDRESS, 33u, 0x0u));

    /* ======= Routine tests =============================================== */
    PHY_SPEED_SEL_e speed = 0x0u;
    /* ======= RT1/3: Test implementation */
    MDIOPhyRegRead_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, 0x0u, &testDataBuffer, true);
    MDIOPhyRegWrite_Expect(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, 0x0u, 0x0u);
    /* ======= RT1/3: Call function under test */
    TEST_PHY_SelectSpeed(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, speed);
    /* ======= RT1/3: Test output verification */

    speed = 0x1u;
    /* ======= RT2/3: Test implementation */
    MDIOPhyRegRead_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, 0x0u, &testDataBuffer, true);
    MDIOPhyRegWrite_Expect(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, 0x0u, 0x40u);
    /* ======= RT2/3: Call function under test */
    TEST_PHY_SelectSpeed(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, speed);
    /* ======= RT2/3: Test output verification */

    speed = 0x2u;
    /* ======= RT2/3: Test implementation */
    MDIOPhyRegRead_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, 0x0u, &testDataBuffer, true);
    MDIOPhyRegWrite_Expect(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, 0x0u, 0x2000u);
    /* ======= RT2/3: Call function under test */
    TEST_PHY_SelectSpeed(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, speed);
    /* ======= RT2/3: Test output verification */
}

void testPHY_RestartSoftware(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_PHY_RestartSoftware(TEST_MDIO_BASE_ADDRESS, 33u));

    /* ======= Routine tests =============================================== */
    uint16_t n_restartComplete = 0x4000u;
    uint16_t restartComplete   = 0x0000u;
    /* ======= RT1/1: Test implementation */
    helper_SwRestart(&restartComplete, &n_restartComplete);
    /* ======= RT1/1: Call function under test */
    TEST_PHY_RestartSoftware(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS);
}

/**
 * @brief   Testing extern function #PHY_Initialize
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - none
 *          - Routine validation:
 *            - RT1/6: Function calls expected subroutine
 *            - RT2/6: PHY ID is not returned
 *            - RT3/6: Phy ID is not correct
 *            - RT4/6: Phy alive not there
 *            - RT5/6: PHY Mii mode not set
 *            - RT6/6: Phy not linked
 */
void testPHY_Initialize(void) {
    uint16_t testDataBuffer  = 0x0000u;
    uint16_t id1             = 0x2000;
    uint16_t id2             = 0xA0F1;
    STD_RETURN_TYPE_e result = STD_OK;
    /* ======= Assertion tests ============================================= */

    /* ======= Routine tests =============================================== */

    /* ======= RT1/6: Test implementation */
    helper_HardwareReset();

    /* Call of PHY_GetId() */
    MDIOPhyRegRead_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, 0x2u, &testDataBuffer, true);
    MDIOPhyRegRead_ReturnThruPtr_dataPtr(&id1);
    MDIOPhyRegRead_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, 0x3u, &id1, true);
    MDIOPhyRegRead_ReturnThruPtr_dataPtr(&id2);
    OS_DelayTask_Expect(10u);

    /* Call of MDIOPhyAliveStatusGet() */
    MDIOPhyAliveStatusGet_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, 0x2u);

    /* Call of PHY_SetMiiMode() */
    uint16_t resetComplete = 0x0000u;
    MDIOPhyRegWrite_Expect(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, TEST_PHY_GEN_CTRL, 0x8000u);
    MDIOPhyRegRead_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, TEST_PHY_GEN_CTRL, &resetComplete, true);
    MDIOPhyRegRead_ReturnThruPtr_dataPtr(&resetComplete);
    uint16_t miiModeNotSet = 0x0u;
    uint16_t miiModeSet    = 0x60u;
    helper_ExtendedAddressSpaceRegRead(0x1DFu, &miiModeNotSet, &miiModeNotSet);
    helper_ExtendedAddressSpaceRegWrite(0x1DFu, miiModeSet);
    uint16_t n_restartComplete = 0x4000u;
    uint16_t restartComplete   = 0x0000u;
    helper_SwRestart(&restartComplete, &n_restartComplete);
    helper_ExtendedAddressSpaceRegRead(0x1DFu, &miiModeNotSet, &miiModeSet);
    /* Call of PHY_GetLinkStatus() */
    uint16_t testLinkStatus  = 0u;
    uint16_t testLinkSuccess = 0x4u;
    MDIOPhyRegRead_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, 0x1u, &testLinkStatus, true);
    MDIOPhyRegRead_ReturnThruPtr_dataPtr(&testLinkSuccess);
    DATA_Write1DataBlock_ExpectAndReturn(&tablePhy, STD_OK);

    DATA_Write1DataBlock_ExpectAndReturn(&tablePhy, STD_OK);

    /* ======= RT1/6: Call function under test */
    result = PHY_Initialize(TEST_MDIO_BASE_ADDRESS);
    /* ======= RT1/6: Test output verification */
    TEST_ASSERT_EQUAL(result, STD_OK);

    /* ======= RT2/6: Test implementation */
    uint32_t phyIdReadCount = 0xFu;
    helper_HardwareReset();

    while (phyIdReadCount > 0u) {
        /* Call of PHY_GetId() */
        MDIOPhyRegRead_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, 0x2u, &testDataBuffer, true);
        MDIOPhyRegRead_ReturnThruPtr_dataPtr(&testDataBuffer);
        MDIOPhyRegRead_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, 0x3u, &testDataBuffer, true);
        MDIOPhyRegRead_ReturnThruPtr_dataPtr(&testDataBuffer);
        OS_DelayTask_Expect(10u);
        phyIdReadCount--;
    }

    DATA_Write1DataBlock_ExpectAndReturn(&tablePhy, STD_OK);

    /* ======= RT2/6: Call function under test */
    result = PHY_Initialize(TEST_MDIO_BASE_ADDRESS);

    /* ======= RT2/6: Test output verification */
    TEST_ASSERT_EQUAL(result, STD_NOT_OK);
    resetTest();

    /* ======= RT3/6: Test implementation */
    helper_HardwareReset();

    /* Call of PHY_GetId() */
    MDIOPhyRegRead_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, 0x2u, &testDataBuffer, true);
    MDIOPhyRegRead_ReturnThruPtr_dataPtr(&id1);
    MDIOPhyRegRead_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, 0x3u, &id1, true);
    MDIOPhyRegRead_ReturnThruPtr_dataPtr(&testDataBuffer);
    OS_DelayTask_Expect(10u);
    DATA_Write1DataBlock_ExpectAndReturn(&tablePhy, STD_OK);

    /* ======= RT3/6: Call function under test */
    result = PHY_Initialize(TEST_MDIO_BASE_ADDRESS);

    /* ======= RT3/6: Test output verification */
    TEST_ASSERT_EQUAL(result, STD_NOT_OK);

    /* ======= RT4/6: Test implementation */
    helper_HardwareReset();

    /* Call of PHY_GetId() */
    MDIOPhyRegRead_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, 0x2u, &testDataBuffer, true);
    MDIOPhyRegRead_ReturnThruPtr_dataPtr(&id1);
    MDIOPhyRegRead_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, 0x3u, &id1, true);
    MDIOPhyRegRead_ReturnThruPtr_dataPtr(&id2);
    OS_DelayTask_Expect(10u);

    /* Call of MDIOPhyAliveStatusGet() */
    MDIOPhyAliveStatusGet_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, 0x1u);
    DATA_Write1DataBlock_ExpectAndReturn(&tablePhy, STD_OK);

    /* ======= RT4/6: Call function under test */
    result = PHY_Initialize(TEST_MDIO_BASE_ADDRESS);

    /* ======= RT4/6: Test output verification */
    TEST_ASSERT_EQUAL(result, STD_NOT_OK);
    resetTest();

    /* ======= RT5/6: Test implementation */
    helper_HardwareReset();

    /* Call of PHY_GetId() */
    MDIOPhyRegRead_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, 0x2u, &testDataBuffer, true);
    MDIOPhyRegRead_ReturnThruPtr_dataPtr(&id1);
    MDIOPhyRegRead_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, 0x3u, &id1, true);
    MDIOPhyRegRead_ReturnThruPtr_dataPtr(&id2);
    OS_DelayTask_Expect(10u);

    /* Call of MDIOPhyAliveStatusGet() */
    MDIOPhyAliveStatusGet_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, 0x2u);

    /* Call of PHY_SetMiiMode() */
    resetComplete = 0x0000u;
    MDIOPhyRegWrite_Expect(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, TEST_PHY_GEN_CTRL, 0x8000u);
    MDIOPhyRegRead_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, TEST_PHY_GEN_CTRL, &resetComplete, true);
    MDIOPhyRegRead_ReturnThruPtr_dataPtr(&resetComplete);
    miiModeNotSet = 0x0u;
    miiModeSet    = 0x60u;
    helper_ExtendedAddressSpaceRegRead(0x1DFu, &miiModeNotSet, &miiModeNotSet);
    helper_ExtendedAddressSpaceRegWrite(0x1DFu, miiModeSet);
    n_restartComplete = 0x4000u;
    restartComplete   = 0x0000u;
    helper_SwRestart(&restartComplete, &n_restartComplete);
    helper_ExtendedAddressSpaceRegRead(0x1DFu, &miiModeNotSet, &miiModeNotSet);
    DATA_Write1DataBlock_ExpectAndReturn(&tablePhy, STD_OK);

    /* ======= RT5/6: Call function under test */
    result = PHY_Initialize(TEST_MDIO_BASE_ADDRESS);

    /* ======= RT5/6: Test output verification */
    TEST_ASSERT_EQUAL(result, STD_NOT_OK);
    resetTest();

    /* ======= RT6/6: Test implementation */
    helper_HardwareReset();

    /* Call of PHY_GetId() */
    MDIOPhyRegRead_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, 0x2u, &testDataBuffer, true);
    MDIOPhyRegRead_ReturnThruPtr_dataPtr(&id1);
    MDIOPhyRegRead_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, 0x3u, &id1, true);
    MDIOPhyRegRead_ReturnThruPtr_dataPtr(&id2);
    OS_DelayTask_Expect(10u);

    /* Call of MDIOPhyAliveStatusGet() */
    MDIOPhyAliveStatusGet_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, 0x2u);

    /* Call of PHY_SetMiiMode() */
    resetComplete = 0x0000u;
    MDIOPhyRegWrite_Expect(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, TEST_PHY_GEN_CTRL, 0x8000u);
    MDIOPhyRegRead_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, TEST_PHY_GEN_CTRL, &resetComplete, true);
    MDIOPhyRegRead_ReturnThruPtr_dataPtr(&resetComplete);
    miiModeNotSet = 0x0u;
    miiModeSet    = 0x60u;
    helper_ExtendedAddressSpaceRegRead(0x1DFu, &miiModeNotSet, &miiModeNotSet);
    helper_ExtendedAddressSpaceRegWrite(0x1DFu, miiModeSet);
    n_restartComplete = 0x4000u;
    restartComplete   = 0x0000u;
    helper_SwRestart(&restartComplete, &n_restartComplete);
    helper_ExtendedAddressSpaceRegRead(0x1DFu, &miiModeNotSet, &miiModeSet);
    /* Call of PHY_GetLinkStatus() */
    testLinkStatus = 0u;

    uint32_t retries = 0xFu;
    while (retries > 0u) {
        MDIOPhyRegRead_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, 0x1u, &testLinkStatus, true);
        MDIOPhyRegRead_ReturnThruPtr_dataPtr(&testLinkStatus);
        OS_DelayTask_Expect(100u);
        retries--;
    }
    DATA_Write1DataBlock_ExpectAndReturn(&tablePhy, STD_OK);

    DATA_Write1DataBlock_ExpectAndReturn(&tablePhy, STD_OK);

    /* ======= RT6/6: Call function under test */
    result = PHY_Initialize(TEST_MDIO_BASE_ADDRESS);
    /* ======= RT6/6: Test output verification */
    TEST_ASSERT_EQUAL(result, STD_NOT_OK);
}

/**
 * @brief   Testing extern function #PHY_GetLinkStatus
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: Invalid phy address &rarr; assert
 *          - Routine validation:
 *            - RT1/2: Function calls expected subroutine
 *            - RT2/2: Function calls expected subroutine
 */
void testPHY_LinkStatusGet(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(PHY_GetLinkStatus(TEST_MDIO_BASE_ADDRESS, 33u, 0x1u));

    /* ======= Routine tests =============================================== */
    uint16_t testLinkStatus  = 0u;
    uint16_t testLinkSuccess = 0x4u;
    STD_RETURN_TYPE_e result;

    /* ======= RT1/2: Test implementation */
    MDIOPhyRegRead_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, 0x1u, &testLinkStatus, true);
    MDIOPhyRegRead_ReturnThruPtr_dataPtr(&testLinkSuccess);
    DATA_Write1DataBlock_ExpectAndReturn(&tablePhy, STD_OK);
    /* ======= RT1/2: Call function under test */
    result = PHY_GetLinkStatus(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, 0x1u);
    /* ======= RT1/2: Test output verification */
    TEST_ASSERT_EQUAL(STD_OK, result);

    /* ======= RT2/2: Test implementation */
    uint32_t retries = 0x1u;
    while (retries > 0u) {
        MDIOPhyRegRead_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, 0x1u, &testLinkStatus, false);
        OS_DelayTask_Expect(100u);
        retries--;
    }
    DATA_Write1DataBlock_ExpectAndReturn(&tablePhy, STD_OK);
    /* ======= RT2/2: Call function under test */
    result = PHY_GetLinkStatus(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, 0x1u);
    /* ======= RT2/2: Test output verification */
    TEST_ASSERT_EQUAL(STD_NOT_OK, result);
}

/**
 * @brief   Testing extern function #PHY_GetId
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: Invalid phy address &rarr; assert
 *          - Routine validation:
 *            - RT1/1: Function calls expected subroutine
 */
void testPHY_IdGet(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(PHY_GetId(TEST_MDIO_BASE_ADDRESS, 33u));
    /* ======= Routine tests =============================================== */
    uint16_t testDataBuffer = 0x0000u;
    uint16_t id1            = 0x2000;
    uint16_t id2            = 0xA0F1;
    /* ======= RT1/1: Test implementation */
    MDIOPhyRegRead_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, 0x2u, &testDataBuffer, true);
    MDIOPhyRegRead_ReturnThruPtr_dataPtr(&id1);
    MDIOPhyRegRead_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, 0x3u, &id1, true);
    MDIOPhyRegRead_ReturnThruPtr_dataPtr(&id2);
    /* ======= RT1/1: Call function under test */
    uint32_t phyId = PHY_GetId(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS);
    /* ======= RT1/1: Test output verification */
    TEST_ASSERT_EQUAL(0x2000A0F1, phyId);
}

/**
 * @brief   Testing extern function #PHY_AutoNegotiate
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: Invalid phy address &rarr; assert
 *          - Routine validation:
 *            - RT1/6: Function calls expected subroutine
 *            - RT2/6: BMCR register read fail
 *            - RT3/6: Fail to enable auto negotiation bit
 *            - RT4/6: Fail to write auto negotiation capabilities
 *            - RT5/6: Second try for complete
 *            - RT4/6: Auto Negotiation time out
 */
void testPHY_AutoNegotiate(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(PHY_AutoNegotiate(TEST_MDIO_BASE_ADDRESS, 33u, 0x1E0u));

    /* ======= Routine tests =============================================== */
    uint16_t registerData      = 0u;
    uint16_t autoNegEnable     = 0x1000u;
    uint16_t autoNegComplete   = 0x028u;
    uint16_t autoNegIncomplete = 0x0000u;
    STD_RETURN_TYPE_e result;

    /* ======= RT1/6: Test implementation */
    MDIOPhyRegRead_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, TEST_PHY_BMCR, &registerData, true);

    /* Enable Auto Negotiation */
    MDIOPhyRegWrite_Expect(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, TEST_PHY_BMCR, autoNegEnable);
    MDIOPhyRegRead_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, TEST_PHY_BMCR, &autoNegEnable, true);

    MDIOPhyRegRead_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, 0x4u, &registerData, true);
    MDIOPhyRegWrite_Expect(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, 0x4u, 0x1E0u);

    MDIOPhyRegWrite_Expect(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, TEST_PHY_BMCR, 0x1200u);
    MDIOPhyRegRead_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, 0x1u, &registerData, true);
    MDIOPhyRegRead_ReturnThruPtr_dataPtr(&autoNegComplete);
    /* Wait till auto negotiation is complete */
    /* ======= RT1/6: Call function under test */
    result = PHY_AutoNegotiate(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, 0x1E0u);
    /* ======= RT1/6: Test output verification */
    TEST_ASSERT_EQUAL(STD_OK, result);

    /* ======= RT2/6: Test implementation */
    MDIOPhyRegRead_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, TEST_PHY_BMCR, &registerData, false);
    /* ======= RT2/6: Call function under test */
    result = PHY_AutoNegotiate(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, 0x1E0u);
    /* ======= RT2/6: Test output verification */
    TEST_ASSERT_EQUAL(STD_NOT_OK, result);

    /* ======= RT3/6: Test implementation */
    MDIOPhyRegRead_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, TEST_PHY_BMCR, &registerData, true);
    /* Enable Auto Negotiation */
    MDIOPhyRegWrite_Expect(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, TEST_PHY_BMCR, autoNegEnable);
    MDIOPhyRegRead_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, TEST_PHY_BMCR, &autoNegEnable, false);
    /* ======= RT3/6: Call function under test */
    result = PHY_AutoNegotiate(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, 0x1E0u);
    /* ======= RT3/6: Test output verification */
    TEST_ASSERT_EQUAL(STD_NOT_OK, result);

    /* ======= RT4/6: Test implementation */
    MDIOPhyRegRead_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, TEST_PHY_BMCR, &registerData, true);
    /* Enable Auto Negotiation */
    MDIOPhyRegWrite_Expect(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, TEST_PHY_BMCR, autoNegEnable);
    MDIOPhyRegRead_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, TEST_PHY_BMCR, &autoNegEnable, true);

    MDIOPhyRegRead_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, 0x4u, &registerData, true);
    MDIOPhyRegWrite_Expect(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, 0x4u, 0x1E0u);
    MDIOPhyRegWrite_Expect(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, TEST_PHY_BMCR, 0x1200u);
    MDIOPhyRegRead_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, 0x1u, &registerData, false);

    /* ======= RT4/6: Call function under test */
    result = PHY_AutoNegotiate(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, 0x1E0u);
    /* ======= RT4/6: Test output verification */
    TEST_ASSERT_EQUAL(STD_NOT_OK, result);

    /* ======= RT5/6: Test implementation */
    MDIOPhyRegRead_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, TEST_PHY_BMCR, &registerData, true);

    /* Enable Auto Negotiation */
    MDIOPhyRegWrite_Expect(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, TEST_PHY_BMCR, autoNegEnable);
    MDIOPhyRegRead_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, TEST_PHY_BMCR, &autoNegEnable, true);

    MDIOPhyRegRead_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, 0x4u, &registerData, true);
    MDIOPhyRegWrite_Expect(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, 0x4u, 0x1E0u);

    MDIOPhyRegWrite_Expect(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, TEST_PHY_BMCR, 0x1200u);
    MDIOPhyRegRead_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, 0x1u, &registerData, true);
    MDIOPhyRegRead_ReturnThruPtr_dataPtr(&autoNegIncomplete);

    MDIOPhyRegRead_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, 0x1u, &autoNegIncomplete, true);
    MDIOPhyRegRead_ReturnThruPtr_dataPtr(&autoNegComplete);
    OS_DelayTask_Expect(100u);
    /* ======= RT5/6: Call function under test */
    result = PHY_AutoNegotiate(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, 0x1E0u);
    /* ======= RT5/6: Test output verification */
    TEST_ASSERT_EQUAL(STD_OK, result);

    /* ======= RT6/6: Test implementation */
    MDIOPhyRegRead_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, TEST_PHY_BMCR, &registerData, true);

    /* Enable Auto Negotiation */
    MDIOPhyRegWrite_Expect(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, TEST_PHY_BMCR, autoNegEnable);
    MDIOPhyRegRead_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, TEST_PHY_BMCR, &autoNegEnable, true);

    MDIOPhyRegRead_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, 0x4u, &registerData, true);
    MDIOPhyRegWrite_Expect(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, 0x4u, 0x1E0u);

    MDIOPhyRegWrite_Expect(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, TEST_PHY_BMCR, 0x1200u);
    MDIOPhyRegRead_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, 0x1u, &registerData, true);
    MDIOPhyRegRead_ReturnThruPtr_dataPtr(&autoNegIncomplete);

    uint16_t phyNegTries = 100u;
    while (phyNegTries > 0u) {
        MDIOPhyRegRead_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, 0x1u, &autoNegIncomplete, true);
        MDIOPhyRegRead_ReturnThruPtr_dataPtr(&autoNegIncomplete);
        OS_DelayTask_Expect(100u);
        phyNegTries--;
    }
    /* ======= RT6/6: Call function under test */
    result = PHY_AutoNegotiate(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, 0x1E0u);
    /* ======= RT6/6: Test output verification */
    TEST_ASSERT_EQUAL(STD_NOT_OK, result);
}

/**
 * @brief   Testing extern function #PHY_GetPartnerAbility
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/2: Invalid phy address &rarr; assert
 *            - AT2/2: Invalid ptnerAbltyBuffer &rarr; assert
 *          - Routine validation:
 *            - RT1/1: Function calls expected subroutine
 */
void testPHY_PartnerAbilityGet(void) {
    /* ======= Assertion tests ============================================= */
    uint16_t ptnerAbltyBuffer = 0u;
    /* ======= AT1/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(PHY_GetPartnerAbility(TEST_MDIO_BASE_ADDRESS, 33u, &ptnerAbltyBuffer));
    /* ======= AT2/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(PHY_GetPartnerAbility(TEST_MDIO_BASE_ADDRESS, 0x1u, NULL_PTR));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    MDIOPhyRegRead_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, 0x5u, &ptnerAbltyBuffer, true);

    /* ======= RT1/1: Call function under test */
    PHY_GetPartnerAbility(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, &ptnerAbltyBuffer);
}

/**
 * @brief   Testing extern function #PHY_SetMiiMode
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: Invalid phy address &rarr; assert
 *          - Routine validation:
 *            - RT1/1: Function calls expected subroutine
 */
void testPHY_MIImmodeSet(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(PHY_SetMiiMode(TEST_MDIO_BASE_ADDRESS, 33u));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    /* Call PHY_ResetSoftware() */
    uint16_t resetComplete   = 0x0000u;
    uint16_t n_resetComplete = 0x8000u;
    helper_SwReset(&resetComplete, &n_resetComplete);

    uint16_t miiModeNotSet = 0x0u;
    uint16_t miiModeSet    = 0x60u;
    helper_ExtendedAddressSpaceRegRead(0x1DFu, &miiModeNotSet, &miiModeNotSet);

    /* Setting MII MOODE. */
    helper_ExtendedAddressSpaceRegWrite(0x1DFu, miiModeSet);

    /* Call PHY_RestartSoftware() */
    uint16_t n_restartComplete = 0x4000u;
    uint16_t restartComplete   = 0x0000u;
    helper_SwRestart(&restartComplete, &n_restartComplete);

    helper_ExtendedAddressSpaceRegRead(0x1DFu, &miiModeNotSet, &miiModeSet);
    /* ======= RT1/1: Call function under test */
    PHY_SetMiiMode(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS);
}

/**
 * @brief   Testing extern function #PHY_ResetMii
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: Invalid phy address &rarr; assert
 *          - Routine validation:
 *            - RT1/1: Function calls expected subroutine
 */
void testPHY_MiiReset(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(PHY_ResetMii(TEST_MDIO_BASE_ADDRESS, 33u));
    /* ======= Routine tests =============================================== */
    uint16_t resetComplete   = 0x0000u;
    uint16_t n_resetComplete = 0x8000u;
    /* ======= RT1/1: Test implementation */
    MDIOPhyRegWrite_Expect(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, TEST_PHY_BMCR, 0x8000u);
    /* Reset in progress */
    MDIOPhyRegRead_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, TEST_PHY_BMCR, &resetComplete, true);
    MDIOPhyRegRead_ReturnThruPtr_dataPtr(&n_resetComplete);
    /* Reset complete */
    MDIOPhyRegRead_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, TEST_PHY_BMCR, &n_resetComplete, true);
    MDIOPhyRegRead_ReturnThruPtr_dataPtr(&resetComplete);
    /* ======= RT1/1: Call function under test */
    PHY_ResetMii(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS);
}

/**
 * @brief   Testing extern function #PHY_EnableLoopback
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: Invalid phy address &rarr; assert
 *          - Routine validation:
 *            - RT1/3: Function calls expected subroutine
 *            - RT2/3: Speed set to 100 Mbps
 *            - RT3/3: Speed set to 100 Mbps and read fail
 */
void testPHY_EnableLoopback(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(PHY_EnableLoopback(TEST_MDIO_BASE_ADDRESS, 33u));

    /* ======= Routine tests =============================================== */
    uint16_t regVal     = 0x0000u;
    uint16_t reg100Mbps = 0x2000u;
    /* ======= RT1/3: Test implementation */
    /* Disabling Auto Negotiate. */
    MDIOPhyRegRead_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, TEST_PHY_BMCR, &regVal, true);
    MDIOPhyRegWrite_Expect(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, TEST_PHY_BMCR, regVal);
    /* Call PHY_SelectSpeed() */
    MDIOPhyRegRead_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, TEST_PHY_BMCR, &regVal, true);
    MDIOPhyRegWrite_Expect(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, TEST_PHY_BMCR, 0x0u);
    MDIOPhyRegWrite_Expect(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, TEST_PHY_BMCR, 0x4000u);
    /* Check for 100BaseTx mode*/
    MDIOPhyRegRead_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, 0x11u, &regVal, true);
    MDIOPhyRegRead_ReturnThruPtr_dataPtr(&regVal);
    helper_ExtendedAddressSpaceRegWrite(0x00FE, 0xE720);
    /* Call PHY_SelectSpeed() */
    MDIOPhyRegRead_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, TEST_PHY_BMCR, &regVal, true);
    MDIOPhyRegWrite_Expect(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, TEST_PHY_BMCR, 0x0u);
    /* ======= RT1/3: Call function under test */
    PHY_EnableLoopback(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS);

    /* ======= RT2/3: Test implementation */
    /* Disabling Auto Negotiate. */
    MDIOPhyRegRead_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, TEST_PHY_BMCR, &regVal, true);
    MDIOPhyRegWrite_Expect(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, TEST_PHY_BMCR, regVal);
    /* Call PHY_SelectSpeed() */
    MDIOPhyRegRead_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, TEST_PHY_BMCR, &regVal, true);
    MDIOPhyRegWrite_Expect(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, TEST_PHY_BMCR, 0x0u);
    MDIOPhyRegWrite_Expect(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, TEST_PHY_BMCR, 0x4000u);
    /* Check for 100BaseTx mode*/
    MDIOPhyRegRead_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, 0x11u, &regVal, true);
    MDIOPhyRegRead_ReturnThruPtr_dataPtr(&reg100Mbps);

    MDIOPhyRegRead_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, 0x16u, &regVal, true);
    MDIOPhyRegWrite_Expect(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, 0x16u, 0x04u);
    MDIOPhyRegRead_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, 0x16u, &regVal, true);

    helper_ExtendedAddressSpaceRegWrite(0x00FE, 0xE720);
    /* Call PHY_SelectSpeed() */
    MDIOPhyRegRead_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, TEST_PHY_BMCR, &regVal, true);
    MDIOPhyRegWrite_Expect(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, TEST_PHY_BMCR, 0x0u);
    /* ======= RT2/3: Call function under test */
    PHY_EnableLoopback(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS);

    /* ======= RT3/3: Test implementation */
    /* Disabling Auto Negotiate. */
    MDIOPhyRegRead_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, TEST_PHY_BMCR, &regVal, true);
    MDIOPhyRegWrite_Expect(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, TEST_PHY_BMCR, regVal);
    /* Call PHY_SelectSpeed() */
    MDIOPhyRegRead_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, TEST_PHY_BMCR, &regVal, true);
    MDIOPhyRegWrite_Expect(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, TEST_PHY_BMCR, 0x0u);
    MDIOPhyRegWrite_Expect(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, TEST_PHY_BMCR, 0x4000u);
    /* Check for 100BaseTx mode*/
    MDIOPhyRegRead_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, 0x11u, &regVal, true);
    MDIOPhyRegRead_ReturnThruPtr_dataPtr(&reg100Mbps);

    MDIOPhyRegRead_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, 0x16u, &regVal, true);
    MDIOPhyRegWrite_Expect(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, 0x16u, 0x04u);
    MDIOPhyRegRead_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, 0x16u, &regVal, false);

    helper_ExtendedAddressSpaceRegWrite(0x00FE, 0xE720);
    /* Call PHY_SelectSpeed() */
    MDIOPhyRegRead_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, TEST_PHY_BMCR, &regVal, true);
    MDIOPhyRegWrite_Expect(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, TEST_PHY_BMCR, 0x0u);
    /* ======= RT3/3: Call function under test */
    PHY_EnableLoopback(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS);
}

void testPHY_SwReset(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(PHY_ResetSoftware(TEST_MDIO_BASE_ADDRESS, 33u));

    /* ======= Routine tests =============================================== */
    uint16_t resetComplete   = 0x0000u;
    uint16_t n_resetComplete = 0x8000u;
    /* ======= RT1/1: Test implementation */
    helper_SwReset(&resetComplete, &n_resetComplete);
    /* ======= RT1/1: Call function under test */
    PHY_ResetSoftware(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS);
}

void testPHY_HardwareReset(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    IO_PinReset_Expect(&((gioPORT_t *)0xFFF7BC34U)->DOUT, 1u);
    OS_DelayTask_Expect(1u);
    IO_PinSet_Expect(&((gioPORT_t *)0xFFF7BC34U)->DOUT, 1u);
    OS_DelayTask_Expect(2u);
    /* ======= RT1/1: Call function under test */
    PHY_ResetHardware();
}

void testPHY_OperationModeGet(void) {
    /* ======= Assertion tests ============================================= */
    PHY_OPERATION_MODE_s opmode;
    /* ======= AT1/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(PHY_GetOperationMode(TEST_MDIO_BASE_ADDRESS, 33u, &opmode));
    /* ======= AT2/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(PHY_GetOperationMode(TEST_MDIO_BASE_ADDRESS, 0x1u, NULL_PTR));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    uint16_t opmodeReg   = 0x0000u;
    uint16_t returnValue = 0x3415u;
    helper_ExtendedAddressSpaceRegRead(0x6Eu, &opmodeReg, &returnValue);

    /* ======= RT1/1: Call function under test */
    PHY_GetOperationMode(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, &opmode);

    /* ======= RT1/1: Test output verification */
    TEST_ASSERT_EQUAL(opmode.rgmii2miiEnable, 1u);
    TEST_ASSERT_EQUAL(opmode.autoNegotiationEnable, 1u);
    TEST_ASSERT_EQUAL(opmode.phyAddressess, 1u);
    TEST_ASSERT_EQUAL(opmode.operationMode, 2u);
    TEST_ASSERT_EQUAL(opmode.mirrorEnable, 1u);
    TEST_ASSERT_EQUAL(opmode.linkLossPassEnable, 1u);
}
