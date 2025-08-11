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
 * @file    struct_helper.c
 * @author  foxBMS Team
 * @date    2025-02-28 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Helper for unit tests
 * @details Assert helper macros for custom structs
 *
 */

/*========== Includes =======================================================*/
#include "struct_helper.h"

/*========== Unit Testing Framework Directives ==============================*/

/*========== Definitions and Implementations for Unit Test ==================*/

/*========== Setup and Teardown =============================================*/

/*========== Test Cases =====================================================*/
void AssertEqual_spi_config_reg_t(
    const spi_config_reg_t expected,
    const spi_config_reg_t actual,
    UNITY_LINE_TYPE line,
    const char *message) {
    /* It's common to override the default message with our own */
    (void)message;

    /* Verify the lengths are the same, or they're clearly not matched */
    UNITY_TEST_ASSERT_EQUAL_INT(expected.CONFIG_GCR1, actual.CONFIG_GCR1, line, "mismatch of member CONFIG_GCR1");
    UNITY_TEST_ASSERT_EQUAL_INT(expected.CONFIG_INT0, actual.CONFIG_INT0, line, "mismatch of member CONFIG_INT0");
    UNITY_TEST_ASSERT_EQUAL_INT(expected.CONFIG_LVL, actual.CONFIG_LVL, line, "mismatch of member CONFIG_LVL");
    UNITY_TEST_ASSERT_EQUAL_INT(expected.CONFIG_PC0, actual.CONFIG_PC0, line, "mismatch of member CONFIG_PC0");
    UNITY_TEST_ASSERT_EQUAL_INT(expected.CONFIG_PC1, actual.CONFIG_PC1, line, "mismatch of member CONFIG_PC1");
    UNITY_TEST_ASSERT_EQUAL_INT(expected.CONFIG_PC6, actual.CONFIG_PC6, line, "mismatch of member CONFIG_PC6");
    UNITY_TEST_ASSERT_EQUAL_INT(expected.CONFIG_PC7, actual.CONFIG_PC7, line, "mismatch of member CONFIG_PC7");
    UNITY_TEST_ASSERT_EQUAL_INT(expected.CONFIG_PC8, actual.CONFIG_PC8, line, "mismatch of member CONFIG_PC8");
    UNITY_TEST_ASSERT_EQUAL_INT(expected.CONFIG_DELAY, actual.CONFIG_DELAY, line, "mismatch of member CONFIG_DELAY");
    UNITY_TEST_ASSERT_EQUAL_INT(expected.CONFIG_FMT0, actual.CONFIG_FMT0, line, "mismatch of member CONFIG_FMT0");
    UNITY_TEST_ASSERT_EQUAL_INT(expected.CONFIG_FMT1, actual.CONFIG_FMT1, line, "mismatch of member CONFIG_FMT1");
    UNITY_TEST_ASSERT_EQUAL_INT(expected.CONFIG_FMT2, actual.CONFIG_FMT2, line, "mismatch of member CONFIG_FMT2");
    UNITY_TEST_ASSERT_EQUAL_INT(expected.CONFIG_FMT3, actual.CONFIG_FMT3, line, "mismatch of member CONFIG_FMT3");
}
