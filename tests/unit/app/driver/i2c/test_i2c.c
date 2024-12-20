/**
 *
 * @copyright &copy; 2010 - 2024, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * @file    test_i2c.c
 * @author  foxBMS Team
 * @date    2021-07-23 (date of creation)
 * @updated 2024-12-20 (date of last update)
 * @version v1.8.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Tests for the I2C module
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "MockHL_i2c.h"
#include "MockHL_sys_dma.h"
#include "Mockmcu.h"
#include "Mockos.h"
#include "Mocktask.h"

#include "i2c.h"
#include "test_assert_helper.h"

/*========== Unit Testing Framework Directives ==============================*/
TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/dma")
TEST_INCLUDE_PATH("../../src/app/driver/i2c")
TEST_INCLUDE_PATH("../../src/app/engine/diag")

/*========== Definitions and Implementations for Unit Test ==================*/

long FSYS_RaisePrivilege(void) {
    return 0;
}

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/

/** I2C Initialize calls the HAL init function */
void testI2c_Initialize(void) {
    i2cInit_Expect();
    I2C_Initialize();
}

/**
 * @brief   Testing extern function #I2C_WriteReadDma
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/6: NULL_PTR for pSocValues &rarr; assert
 *            - AT2/6: NULL_PTR for writeData &rarr; assert
 *            - AT3/6: invalid nrBytesWrite &rarr; assert
 *            - AT4/6: NULL_PTR for nrBytesWrite &rarr; assert
 *            - AT5/6: invalid nrBytesRead &rarr; assert
 *            - AT6/6: invalid slaveAddress &rarr; assert
 *          - Routine validation:
 *            - RT1/x: TODO
 */
void testI2C_WriteReadDma(void) {
    /* ======= Assertion tests ============================================= */
    i2cBASE_t validI2cInterface;
    uint32_t validSlaveAddress = 0u;
    uint32_t validnrBytesWrite = 1u;
    uint8_t validWriteData     = 1u;
    uint32_t validnrBytesRead  = 2u;
    uint8_t validReadData      = 1u;
    /* ======= AT1/6: Assertion test */
    TEST_ASSERT_FAIL_ASSERT(I2C_WriteReadDma(
        NULL_PTR, validSlaveAddress, validnrBytesWrite, &validWriteData, validnrBytesRead, &validReadData));
    /* ======= AT2/6: Assertion test */
    TEST_ASSERT_FAIL_ASSERT(I2C_WriteReadDma(
        &validI2cInterface, 130u, validnrBytesWrite, &validWriteData, validnrBytesRead, &validReadData));
    /* ======= AT3/6: Assertion test */
    TEST_ASSERT_FAIL_ASSERT(
        I2C_WriteReadDma(&validI2cInterface, validSlaveAddress, 0u, &validWriteData, validnrBytesRead, &validReadData));
    /* ======= AT4/6: Assertion test */
    TEST_ASSERT_FAIL_ASSERT(I2C_WriteReadDma(
        &validI2cInterface, validSlaveAddress, validnrBytesWrite, NULL_PTR, validnrBytesRead, &validReadData));
    /* ======= AT5/6: Assertion test */
    TEST_ASSERT_FAIL_ASSERT(I2C_WriteReadDma(
        &validI2cInterface, validSlaveAddress, validnrBytesWrite, &validWriteData, 1u, &validReadData));
    /* ======= AT6/6: Assertion test */
    TEST_ASSERT_FAIL_ASSERT(I2C_WriteReadDma(
        &validI2cInterface, validSlaveAddress, validnrBytesWrite, &validWriteData, validnrBytesRead, NULL_PTR));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/x: Test implementation */
}

/**
 * @brief   Testing extern function #I2C_Write
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/4: NULL_PTR for pI2cInterface &rarr; assert
 *            - AT2/4: invalid slaveAddress &rarr; assert
 *            - AT3/4: invalid nrBytes &rarr; assert
 *            - AT4/4: NULL_PTR for writeData &rarr; assert
 *          - Routine validation:
 *            - RT1/x: TODO
 */
void testI2C_Write(void) {
    i2cBASE_t validI2cInterface;
    uint32_t validSlaveAddress = 0u;
    uint32_t validnrBytesWrite = 1u;
    uint8_t validWriteData     = 1u;
    /* ======= AT1/4: Assertion test */
    TEST_ASSERT_FAIL_ASSERT(I2C_Write(NULL_PTR, validSlaveAddress, validnrBytesWrite, &validWriteData));
    /* ======= AT2/4: Assertion test */
    TEST_ASSERT_FAIL_ASSERT(I2C_Write(&validI2cInterface, 130u, validnrBytesWrite, &validWriteData));
    /* ======= AT3/4: Assertion test */
    TEST_ASSERT_FAIL_ASSERT(I2C_Write(&validI2cInterface, validSlaveAddress, 0u, &validWriteData));
    /* ======= AT4/4: Assertion test */
    TEST_ASSERT_FAIL_ASSERT(I2C_Write(&validI2cInterface, validSlaveAddress, validnrBytesWrite, NULL_PTR));
}

void testI2C_GetWordTransmitTime(void) {
    TEST_ASSERT_FAIL_ASSERT(TEST_I2C_GetWordTransmitTime(NULL_PTR));
}

void testI2C_WaitStop(void) {
    TEST_ASSERT_FAIL_ASSERT(TEST_I2C_WaitStop(NULL_PTR, 0u));
}
