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
 * @file    test_can_helper.c
 * @author  foxBMS Team
 * @date    2021-04-22 (date of creation)
 * @updated 2021-06-09 (date of last update)
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Tests for the CAN helper functions
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockcan.h"
#include "Mockdatabase.h"
#include "Mockdiag.h"
#include "Mockfoxmath.h"
#include "Mockimd.h"
#include "Mockmpu_prototypes.h"
#include "Mockos.h"

#include "database_cfg.h"

#include "can_cbs.h"
#include "can_helper.h"
#include "test_assert_helper.h"

/*========== Definitions and Implementations for Unit Test ==================*/

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/

/** test the interface of CAN_TxSetMessageDataWithSignalData for null pointer and invalid parameters */
void testCAN_TxSetMessageDataWithSignalDataInterfaceNullPointer(void) {
    uint64_t message            = 0;
    uint64_t bitStart           = 0;
    uint8_t bitLength           = 0;
    uint64_t canSignal          = 0;
    CAN_ENDIANNESS_e endianness = CAN_LITTLE_ENDIAN;

    TEST_ASSERT_FAIL_ASSERT(CAN_TxSetMessageDataWithSignalData(&message, bitStart, bitLength, canSignal, endianness));

    TEST_ASSERT_FAIL_ASSERT(CAN_TxSetMessageDataWithSignalData(NULL_PTR, 1u, 1u, canSignal, endianness));

    TEST_ASSERT_FAIL_ASSERT(CAN_TxSetMessageDataWithSignalData(&message, 64u, 0u, canSignal, endianness));
    TEST_ASSERT_FAIL_ASSERT(CAN_TxSetMessageDataWithSignalData(&message, 1u, 65u, canSignal, endianness));
    TEST_ASSERT_FAIL_ASSERT(CAN_TxSetMessageDataWithSignalData(&message, 65u, 64u, canSignal, endianness));
    TEST_ASSERT_FAIL_ASSERT(CAN_TxSetMessageDataWithSignalData(&message, 10u, 64u, canSignal, endianness));

    TEST_ASSERT_PASS_ASSERT(CAN_TxSetMessageDataWithSignalData(&message, 1u, 1u, canSignal, CAN_LITTLE_ENDIAN));
    TEST_ASSERT_PASS_ASSERT(CAN_TxSetMessageDataWithSignalData(&message, 1u, 1u, canSignal, CAN_BIG_ENDIAN));
    TEST_ASSERT_FAIL_ASSERT(CAN_TxSetMessageDataWithSignalData(&message, 1u, 1u, canSignal, INT8_MAX));
}

/** test CAN_TxSetMessageDataWithSignalData with various signals */
void testCAN_TxSetMessageDataWithSignalData64BitMessage(void) {
    uint64_t message            = 0;
    uint64_t bitStart           = 0;
    uint8_t bitLength           = 63;
    uint64_t canSignal          = UINT64_MAX >> 1u;
    CAN_ENDIANNESS_e endianness = CAN_LITTLE_ENDIAN;

    CAN_TxSetMessageDataWithSignalData(&message, bitStart, bitLength, canSignal, endianness);

    TEST_ASSERT_EQUAL_UINT64(canSignal, message);

    message    = 0;
    bitStart   = 0;
    bitLength  = 64;
    canSignal  = UINT64_MAX;
    endianness = CAN_LITTLE_ENDIAN;

    CAN_TxSetMessageDataWithSignalData(&message, bitStart, bitLength, canSignal, endianness);

    TEST_ASSERT_EQUAL_UINT64(canSignal, message);

    message    = 0;
    bitStart   = 0;
    bitLength  = 1;
    canSignal  = 0x1u;
    endianness = CAN_LITTLE_ENDIAN;

    CAN_TxSetMessageDataWithSignalData(&message, bitStart, bitLength, canSignal, endianness);

    TEST_ASSERT_EQUAL_UINT64(canSignal, message);

    message    = 0;
    bitStart   = 0;
    bitLength  = 3;
    canSignal  = 0x7u;
    endianness = CAN_LITTLE_ENDIAN;

    CAN_TxSetMessageDataWithSignalData(&message, bitStart, bitLength, canSignal, endianness);

    TEST_ASSERT_EQUAL_UINT64(canSignal, message);

    message    = 0;
    bitStart   = 0;
    bitLength  = 3;
    canSignal  = UINT64_MAX;
    endianness = CAN_LITTLE_ENDIAN;

    CAN_TxSetMessageDataWithSignalData(&message, bitStart, bitLength, canSignal, endianness);
    /* This behavior is a by-product of implementation and you should not necessarily rely on it */
    TEST_ASSERT_EQUAL_UINT64(0x7u, message);

    message    = 0;
    bitStart   = 5;
    bitLength  = 2;
    canSignal  = 0x3u;
    endianness = CAN_LITTLE_ENDIAN;

    CAN_TxSetMessageDataWithSignalData(&message, bitStart, bitLength, canSignal, endianness);

    TEST_ASSERT_EQUAL_UINT64(0x60u, message);
}

/** test the interface of CAN_RxGetSignalDataFromMessageData for null pointer and invalid parameters */
void testCAN_RxGetSignalDataFromMessageDataInterfaceNullPointer(void) {
    uint64_t message            = 0;
    uint64_t bitStart           = 0;
    uint8_t bitLength           = 0;
    uint64_t pCanSignal         = 0;
    CAN_ENDIANNESS_e endianness = CAN_LITTLE_ENDIAN;

    TEST_ASSERT_FAIL_ASSERT(CAN_RxGetSignalDataFromMessageData(message, bitStart, bitLength, &pCanSignal, endianness));

    TEST_ASSERT_FAIL_ASSERT(CAN_RxGetSignalDataFromMessageData(message, 1u, 1u, NULL_PTR, endianness));

    TEST_ASSERT_FAIL_ASSERT(CAN_RxGetSignalDataFromMessageData(message, 64u, 0u, &pCanSignal, endianness));
    TEST_ASSERT_FAIL_ASSERT(CAN_RxGetSignalDataFromMessageData(message, 1u, 65u, &pCanSignal, endianness));
    TEST_ASSERT_FAIL_ASSERT(CAN_RxGetSignalDataFromMessageData(message, 65u, 64u, &pCanSignal, endianness));
    TEST_ASSERT_FAIL_ASSERT(CAN_RxGetSignalDataFromMessageData(message, 10u, 64u, &pCanSignal, endianness));

    TEST_ASSERT_PASS_ASSERT(CAN_RxGetSignalDataFromMessageData(message, 1u, 1u, &pCanSignal, CAN_LITTLE_ENDIAN));
    TEST_ASSERT_PASS_ASSERT(CAN_RxGetSignalDataFromMessageData(message, 1u, 1u, &pCanSignal, CAN_BIG_ENDIAN));
    TEST_ASSERT_FAIL_ASSERT(CAN_RxGetSignalDataFromMessageData(message, 1u, 1u, &pCanSignal, INT8_MAX));
}

/** test CAN_RxGetSignalDataFromMessageData with various signals */
void testCAN_RxGetSignalDataFromMessageData64BitMessage(void) {
    uint64_t message            = UINT64_MAX >> 1u;
    uint64_t bitStart           = 0;
    uint8_t bitLength           = 63;
    uint64_t canSignal          = 0;
    CAN_ENDIANNESS_e endianness = CAN_LITTLE_ENDIAN;

    CAN_RxGetSignalDataFromMessageData(message, bitStart, bitLength, &canSignal, endianness);

    TEST_ASSERT_EQUAL_UINT64(message, canSignal);

    message    = UINT64_MAX;
    bitStart   = 0;
    bitLength  = 64;
    canSignal  = 0;
    endianness = CAN_LITTLE_ENDIAN;

    CAN_RxGetSignalDataFromMessageData(message, bitStart, bitLength, &canSignal, endianness);

    TEST_ASSERT_EQUAL_UINT64(message, canSignal);

    message    = 0x1u;
    bitStart   = 0;
    bitLength  = 1;
    canSignal  = 0;
    endianness = CAN_LITTLE_ENDIAN;

    CAN_RxGetSignalDataFromMessageData(message, bitStart, bitLength, &canSignal, endianness);

    TEST_ASSERT_EQUAL_UINT64(message, canSignal);

    message    = 0x7u;
    bitStart   = 0;
    bitLength  = 3;
    canSignal  = 0;
    endianness = CAN_LITTLE_ENDIAN;

    CAN_RxGetSignalDataFromMessageData(message, bitStart, bitLength, &canSignal, endianness);

    TEST_ASSERT_EQUAL_UINT64(message, canSignal);

    message    = UINT64_MAX;
    bitStart   = 0;
    bitLength  = 3;
    canSignal  = 0;
    endianness = CAN_LITTLE_ENDIAN;

    CAN_RxGetSignalDataFromMessageData(message, bitStart, bitLength, &canSignal, endianness);
    /* This behavior is a by-product of implementation and you should not necessarily rely on it */
    TEST_ASSERT_EQUAL_UINT64(0x7u, canSignal);

    message    = 0x60u;
    bitStart   = 5;
    bitLength  = 2;
    canSignal  = 0u;
    endianness = CAN_LITTLE_ENDIAN;

    CAN_RxGetSignalDataFromMessageData(message, bitStart, bitLength, &canSignal, endianness);

    TEST_ASSERT_EQUAL_UINT64(0x3u, canSignal);
}
