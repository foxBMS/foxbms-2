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
 * @file    adi_ades1830_helpers_test-data-rdb.c
 * @author  foxBMS Team
 * @date    2022-12-07 (date of creation)
 * @updated 2023-10-12 (date of last update)
 * @version v1.6.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Test of some module
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"

#include "adi_ades1830_helpers_test-data-rdb.h"

#include <stdint.h>

/*========== Unit Testing Framework Directives ==============================*/
TEST_INCLUDE_PATH("../../tests/unit/app/driver/afe/adi/ades1830")

/*========== Definitions and Implementations for Unit Test ==================*/

const uint8_t testReadDataBitsDataReceivedData[TEST_MATRIX_SIZE_READ_DATA_BITS] =
    {0x00u, 0x45u, 0x4Fu, 0xF5u, 0xAAu, 0xBEu, 0x10u, 0xFF};
const uint8_t testReadDataBitsDataPositions[TEST_MATRIX_SIZE_READ_DATA_BITS] = {0u, 1u, 2u, 3u, 4u, 5u, 6u, 7u};
const uint8_t testReadDataBitsDataMasks[TEST_MATRIX_SIZE_READ_DATA_BITS] =
    {0x1Eu, 0x43u, 0xD6u, 0xD0u, 0xD6u, 0x6Fu, 0xBEu, 0xFFu};

const uint8_t testReadDataBitsExpectedDataResults[TEST_MATRIX_SIZE_READ_DATA_BITS][TEST_MATRIX_SIZE_READ_DATA_BITS]
                                                 [TEST_MATRIX_SIZE_READ_DATA_BITS] = {
                                                     {{0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u},
                                                      {0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u},
                                                      {0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u},
                                                      {0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u},
                                                      {0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u},
                                                      {0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u},
                                                      {0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u},
                                                      {0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u}},
                                                     {{4u, 65u, 68u, 64u, 68u, 69u, 4u, 69u},
                                                      {2u, 32u, 34u, 32u, 34u, 34u, 2u, 34u},
                                                      {1u, 16u, 17u, 16u, 17u, 17u, 1u, 17u},
                                                      {0u, 8u, 8u, 8u, 8u, 8u, 0u, 8u},
                                                      {0u, 4u, 4u, 4u, 4u, 4u, 0u, 4u},
                                                      {0u, 2u, 2u, 2u, 2u, 2u, 0u, 2u},
                                                      {0u, 1u, 1u, 1u, 1u, 1u, 0u, 1u},
                                                      {0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u}},
                                                     {{14u, 67u, 70u, 64u, 70u, 79u, 14u, 79u},
                                                      {7u, 33u, 35u, 32u, 35u, 39u, 7u, 39u},
                                                      {3u, 16u, 17u, 16u, 17u, 19u, 3u, 19u},
                                                      {1u, 8u, 8u, 8u, 8u, 9u, 1u, 9u},
                                                      {0u, 4u, 4u, 4u, 4u, 4u, 0u, 4u},
                                                      {0u, 2u, 2u, 2u, 2u, 2u, 0u, 2u},
                                                      {0u, 1u, 1u, 1u, 1u, 1u, 0u, 1u},
                                                      {0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u}},
                                                     {{20u, 65u, 212u, 208u, 212u, 101u, 180u, 245u},
                                                      {10u, 32u, 106u, 104u, 106u, 50u, 90u, 122u},
                                                      {5u, 16u, 53u, 52u, 53u, 25u, 45u, 61u},
                                                      {2u, 8u, 26u, 26u, 26u, 12u, 22u, 30u},
                                                      {1u, 4u, 13u, 13u, 13u, 6u, 11u, 15u},
                                                      {0u, 2u, 6u, 6u, 6u, 3u, 5u, 7u},
                                                      {0u, 1u, 3u, 3u, 3u, 1u, 2u, 3u},
                                                      {0u, 0u, 1u, 1u, 1u, 0u, 1u, 1u}},
                                                     {{10u, 2u, 130u, 128u, 130u, 42u, 170u, 170u},
                                                      {5u, 1u, 65u, 64u, 65u, 21u, 85u, 85u},
                                                      {2u, 0u, 32u, 32u, 32u, 10u, 42u, 42u},
                                                      {1u, 0u, 16u, 16u, 16u, 5u, 21u, 21u},
                                                      {0u, 0u, 8u, 8u, 8u, 2u, 10u, 10u},
                                                      {0u, 0u, 4u, 4u, 4u, 1u, 5u, 5u},
                                                      {0u, 0u, 2u, 2u, 2u, 0u, 2u, 2u},
                                                      {0u, 0u, 1u, 1u, 1u, 0u, 1u, 1u}},
                                                     {{30u, 2u, 150u, 144u, 150u, 46u, 190u, 190u},
                                                      {15u, 1u, 75u, 72u, 75u, 23u, 95u, 95u},
                                                      {7u, 0u, 37u, 36u, 37u, 11u, 47u, 47u},
                                                      {3u, 0u, 18u, 18u, 18u, 5u, 23u, 23u},
                                                      {1u, 0u, 9u, 9u, 9u, 2u, 11u, 11u},
                                                      {0u, 0u, 4u, 4u, 4u, 1u, 5u, 5u},
                                                      {0u, 0u, 2u, 2u, 2u, 0u, 2u, 2u},
                                                      {0u, 0u, 1u, 1u, 1u, 0u, 1u, 1u}},
                                                     {{16u, 0u, 16u, 16u, 16u, 0u, 16u, 16u},
                                                      {8u, 0u, 8u, 8u, 8u, 0u, 8u, 8u},
                                                      {4u, 0u, 4u, 4u, 4u, 0u, 4u, 4u},
                                                      {2u, 0u, 2u, 2u, 2u, 0u, 2u, 2u},
                                                      {1u, 0u, 1u, 1u, 1u, 0u, 1u, 1u},
                                                      {0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u},
                                                      {0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u},
                                                      {0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u}},
                                                     {{30u, 67u, 214u, 208u, 214u, 111u, 190u, 255u},
                                                      {15u, 33u, 107u, 104u, 107u, 55u, 95u, 127u},
                                                      {7u, 16u, 53u, 52u, 53u, 27u, 47u, 63u},
                                                      {3u, 8u, 26u, 26u, 26u, 13u, 23u, 31u},
                                                      {1u, 4u, 13u, 13u, 13u, 6u, 11u, 15u},
                                                      {0u, 2u, 6u, 6u, 6u, 3u, 5u, 7u},
                                                      {0u, 1u, 3u, 3u, 3u, 1u, 2u, 3u},
                                                      {0u, 0u, 1u, 1u, 1u, 0u, 1u, 1u}},
};

/*========== Setup and Teardown =============================================*/

/*========== Test Cases =====================================================*/
/* just a helper file */
