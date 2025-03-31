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

// @prefix  LOV

// test: analysis(['CodingStyle-Naming.LocalVariable', 'CodingStyle-Naming.Function'])

/*
 * 'CodingStyle-Naming.Function': ensure that helper functions are well styled
 */

void LOV_RunTestsWithValidLocalVariableNames0(void) {
    int aaa                = 0; /* ok */
    static int lov_bbb     = 0; /* ok */
    static int lov_ccc1    = 0; /* ok */
    static int lov_ccc1Abc = 0; /* ok */

    (void)aaa;         /* silence compiler warnings (not important for the naming test) */
    (void)lov_bbb;     /* silence compiler warnings (not important for the naming test) */
    (void)lov_ccc1;    /* silence compiler warnings (not important for the naming test) */
    (void)lov_ccc1Abc; /* silence compiler warnings (not important for the naming test) */
}

void LOV_RunTestsWithValidLocalVariableNames1(void) {
    int myVariable            = 0; /* ok */
    static int lov_myVariable = 0; /* ok */

    (void)myVariable;     /* silence compiler warnings (not important for the naming test) */
    (void)lov_myVariable; /* silence compiler warnings (not important for the naming test) */
}

void LOV_RunTestsWithValidLocalVariableNames2(void) {
    int myVariable_kOhm            = 0; /* ok */
    static int lov_myVariable_kOhm = 0; /* ok */

    (void)myVariable_kOhm;     /* silence compiler warnings (not important for the naming test) */
    (void)lov_myVariable_kOhm; /* silence compiler warnings (not important for the naming test) */
}

void LOV_RunTestsWithInValidLocalVariableNames0(void) {
    int M          = 0; /* not ok: not camelCase --> correction: 'm' */
    int MyVariable = 0; /* not ok: not camelCase --> correction: 'myVariable' */
    // test: TESTCASE.add_expected_stdout("coding-style-naming.local-variable.c:79:9: error: Local variables names shall (1) use the lowercase module prefix followed by an underscore if they are static otherwise omit the prefix and then (2) use camelCase, and optional (3) use a phyiscal unit as suffix. [M] (Rule CodingStyle-Naming.LocalVariable)")
    // test: TESTCASE.add_expected_stdout("coding-style-naming.local-variable.c:80:9: error: Local variables names shall (1) use the lowercase module prefix followed by an underscore if they are static otherwise omit the prefix and then (2) use camelCase, and optional (3) use a phyiscal unit as suffix. [MyVariable] (Rule CodingStyle-Naming.LocalVariable)")

    (void)M;          /* silence compiler warnings (not important for the naming test) */
    (void)MyVariable; /* silence compiler warnings (not important for the naming test) */
}

void LOV_RunTestsWithInValidLocalVariableNames1(void) {
    int My_kOhm = 0; /* not ok: not camelCase --> correction: 'my_kOhm' */
    int MyBla   = 0; /* not ok: not camelCase --> correction: 'myBla' */
    // test: TESTCASE.add_expected_stdout("coding-style-naming.local-variable.c:89:9: error: Local variables names shall (1) use the lowercase module prefix followed by an underscore if they are static otherwise omit the prefix and then (2) use camelCase, and optional (3) use a phyiscal unit as suffix. [My_kOhm] (Rule CodingStyle-Naming.LocalVariable)")
    // test: TESTCASE.add_expected_stdout("coding-style-naming.local-variable.c:90:9: error: Local variables names shall (1) use the lowercase module prefix followed by an underscore if they are static otherwise omit the prefix and then (2) use camelCase, and optional (3) use a phyiscal unit as suffix. [MyBla] (Rule CodingStyle-Naming.LocalVariable)")

    (void)My_kOhm; /* silence compiler warnings (not important for the naming test) */
    (void)MyBla;   /* silence compiler warnings (not important for the naming test) */
}

void LOV_RunTestsWithInValidLocalVariableNames2(void) {
    static int myVariable     = 0; /* not prefix for static */
    static int lov_MyVariable = 0; /* not camelCase */
    // test: TESTCASE.add_expected_stdout("coding-style-naming.local-variable.c:99:16: error: Local variables names shall (1) use the lowercase module prefix followed by an underscore if they are static otherwise omit the prefix and then (2) use camelCase, and optional (3) use a phyiscal unit as suffix. [myVariable] (Rule CodingStyle-Naming.LocalVariable)")
    // test: TESTCASE.add_expected_stdout("coding-style-naming.local-variable.c:100:16: error: Local variables names shall (1) use the lowercase module prefix followed by an underscore if they are static otherwise omit the prefix and then (2) use camelCase, and optional (3) use a phyiscal unit as suffix. [lov_MyVariable] (Rule CodingStyle-Naming.LocalVariable)")

    (void)myVariable;     /* silence compiler warnings (not important for the naming test) */
    (void)lov_MyVariable; /* silence compiler warnings (not important for the naming test) */
}

void LOV_RunNotStaticButPrefix(void) {
    int lov_myVariable = 0; /* prefix, but static */
    // test: TESTCASE.add_expected_stdout("coding-style-naming.local-variable.c:109:9: error: Local variables names shall (1) use the lowercase module prefix followed by an underscore if they are static otherwise omit the prefix and then (2) use camelCase, and optional (3) use a phyiscal unit as suffix. [lov_myVariable] (Rule CodingStyle-Naming.LocalVariable)")

    (void)lov_myVariable; /* silence compiler warnings (not important for the naming test) */
}
