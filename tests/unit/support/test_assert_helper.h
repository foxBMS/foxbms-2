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
 * @file    test_assert_helper.h
 * @author  foxBMS Team
 * @date    2020-07-22 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Helper for unit tests
 * @details Assert helper macros after http://www.electronvector.com/blog/unit-testing-with-asserts
 *
 */

#ifndef FOXBMS__TEST_ASSERT_HELPER_H_
#define FOXBMS__TEST_ASSERT_HELPER_H_

/*========== Includes =======================================================*/
#include "unity.h"

#include "CException.h"

/*========== Unit Testing Framework Directives ==============================*/

/*========== Macros and Definitions =========================================*/

/**
 * @brief   assert whether assert macro has failed
 * @details This macro allows to check during unit tests whether
 *          our #FAS_ASSERT() macro has failed.
 */
#define TEST_ASSERT_FAIL_ASSERT(_code_under_test)                \
    {                                                            \
        CEXCEPTION_T e;                                          \
        Try {                                                    \
            _code_under_test;                                    \
            TEST_FAIL_MESSAGE("Code under test did not assert"); \
        }                                                        \
        Catch(e) {                                               \
        }                                                        \
    }

/**
 * @brief   assert whether assert macro has passed
 * @details This macro allows to check during unit tests whether
 *          our #FAS_ASSERT() macro has passed.
 *          Using this macro is not mandatory. In the case that an
 *          assertion fails "unknowingly", the unit test will failed
 *          because of an unhandled exception.
 */
#define TEST_ASSERT_PASS_ASSERT(_code_under_test)                     \
    {                                                                 \
        CEXCEPTION_T e;                                               \
        Try {                                                         \
            _code_under_test;                                         \
        }                                                             \
        Catch(e) {                                                    \
            TEST_FAIL_MESSAGE("Code under test failed an assertion"); \
        }                                                             \
    }

#endif /* FOXBMS__TEST_ASSERT_HELPER_H_ */
