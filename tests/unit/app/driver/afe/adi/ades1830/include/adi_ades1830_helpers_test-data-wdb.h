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
 * @file    adi_ades1830_helpers_test-data-wdb.h
 * @author  foxBMS Team
 * @date    2022-12-07 (date of creation)
 * @updated 2025-03-31 (date of last update)
 * @version v1.9.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Test of some module
 * @details TODO
 */

#ifndef FOXBMS__ADI_ADES1830_HELPERS_TEST_DATA_WDB_H_
#define FOXBMS__ADI_ADES1830_HELPERS_TEST_DATA_WDB_H_

/*========== Includes =======================================================*/
#include <stdint.h>

/*========== Unit Testing Framework Directives ==============================*/

/*========== Macros and Definitions =========================================*/
#define TEST_MATRIX_SIZE_WRITE_DATA_BITS (8)

extern const uint8_t testWriteDataBitsDataReceivedData[TEST_MATRIX_SIZE_WRITE_DATA_BITS];
extern const uint8_t testWriteDataBitsDataPositions[TEST_MATRIX_SIZE_WRITE_DATA_BITS];
extern const uint8_t testWriteDataBitsDataMasks[TEST_MATRIX_SIZE_WRITE_DATA_BITS];
extern const uint8_t testWriteDataBitsExpectedDataResults[TEST_MATRIX_SIZE_WRITE_DATA_BITS]
                                                         [TEST_MATRIX_SIZE_WRITE_DATA_BITS]
                                                         [TEST_MATRIX_SIZE_WRITE_DATA_BITS];

#endif /* FOXBMS__ADI_ADES1830_HELPERS_TEST_DATA_WDB_H_ */
