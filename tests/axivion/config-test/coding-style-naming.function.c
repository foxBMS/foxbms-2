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

// @prefix  FUN

// test: analysis(['CodingStyle-Naming.Function'])

void FUN_InitializeSomeDevice(void) { /* ok */
}

void InitializeSomeDevice(void) { /* prefix missing --> not ok */
}
// test: TESTCASE.add_expected_stdout("coding-style-naming.function.c:49:6: error: Function names shall (1) use the uppercase module prefix followed by an underscore and then (2) use PascalCase. [InitializeSomeDevice()] (Rule CodingStyle-Naming.Function)")

void FUN_initializeSomeDevice(void) { /* not PascalCase --> not ok */
}
// test: TESTCASE.add_expected_stdout("coding-style-naming.function.c:53:6: error: Function names shall (1) use the uppercase module prefix followed by an underscore and then (2) use PascalCase. [FUN_initializeSomeDevice()] (Rule CodingStyle-Naming.Function)")

void FUN_InitializeSomeDevice_(void) { /* not alphanumeric due to '_' --> not ok */
}
// test: TESTCASE.add_expected_stdout("coding-style-naming.function.c:57:6: error: Function names shall (1) use the uppercase module prefix followed by an underscore and then (2) use PascalCase. [FUN_InitializeSomeDevice_()] (Rule CodingStyle-Naming.Function)")

void FUN_RunSomeFunctionWithNumberInName2(void) { /* ok */
}

void FUN_RunSomeFunctionWithNumberInName2Foo(void) { /* ok */
}

/* externalize static function for unit testing */
void TEST_FUN_RunSomeFunctionWithNumberInName2Foo(void) { /* ok */
}

/* externalize static function for unit testing uses wrong casing */
void test_FUN_RunSomeFunctionWithNumberInName2Foo(void) { /* not ok */
}
// test: TESTCASE.add_expected_stdout("coding-style-naming.function.c:72:6: error: Function names shall (1) use the uppercase module prefix followed by an underscore and then (2) use PascalCase. [test_FUN_RunSomeFunctionWithNumberInName2Foo()] (Rule CodingStyle-Naming.Function)")

void ABC_FUN_RunSomeFunctionWithNumberInName2Foo(void) { /* not ok */
}
// test: TESTCASE.add_expected_stdout("coding-style-naming.function.c:76:6: error: Function names shall (1) use the uppercase module prefix followed by an underscore and then (2) use PascalCase. [ABC_FUN_RunSomeFunctionWithNumberInName2Foo()] (Rule CodingStyle-Naming.Function)")
