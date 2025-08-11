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
 * @file    test_ignore_list.h
 * @author  foxBMS Team
 * @date    2023-07-21 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Warnings that need to be ignored in the unit test build
 * @details These 'unknown-pragmas' needs to ignored as the GCC would otherwise
 *          complain about the TI pragmas, which are however required.
 *          'unused-parameter' needs to ignored as there are generic
 *          interfaces, where then it can appear that some parameters are then
 *          unused.
 */

#ifndef FOXBMS__TEST_IGNORE_LIST_H_
#define FOXBMS__TEST_IGNORE_LIST_H_

/*========== Includes =======================================================*/

/*========== Unit Testing Framework Directives ==============================*/

/*========== Macros and Definitions =========================================*/

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#pragma GCC diagnostic ignored "-Wunused-parameter"
/* GCC bug: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=53119
   TODO: check if we can upgrade to a later, specific GCC version */
#pragma GCC diagnostic ignored "-Wmissing-braces"
/* TODO: Check if this is really required */
#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
/* TODO: fix state machines */
#pragma GCC diagnostic ignored "-Wswitch"
/* TODO: fix once CMock creates correct '*ReturnThruPtr*' mocks */
#pragma GCC diagnostic ignored "-Wpointer-arith"
/* To solve the issue while running unittest test_boot.c */
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"

#endif /* FOXBMS__TEST_IGNORE_LIST_H_ */
