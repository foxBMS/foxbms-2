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

// @prefix  PAR

// test: analysis(['CodingStyle-Naming.Parameter', 'CodingStyle-Naming.Function'])

/*
 * 'CodingStyle-Naming.Function': ensure that helper functions are well styled
 */

#include <stdint.h>

void PAR_VoidOk00(void) { /* ok */
}
/* suffixes */
void PAR_SuffixOk00(int abc) { /* ok */
}
void PAR_SuffixOk01(int abcPascalCase) { /* ok */
}
void PAR_SuffixOk02(int abc_degC) { /* ok */
}
void PAR_SuffixOk03(int abc_ddegC) { /* ok */
}
void PAR_SuffixOk04(int abc_dK) { /* ok */
}
void PAR_SuffixOk05(int abc_ohm) { /* ok */
}
void PAR_SuffixOk06(int abc_kOhm) { /* ok */
}
void PAR_SuffixOk07(int abc_ms) { /* ok */
}
void PAR_SuffixOk08(int abc_us) { /* ok */
}
void PAR_SuffixOk20(int abc_perc) { /* ok */
}
void PAR_SuffixOk21(int abc_mV) { /* ok */
}
void PAR_SuffixOk22(int abc_V) { /* ok */
}
void PAR_SuffixOk23(int abc_mA) { /* ok */
}
void PAR_SuffixOk24(int abc_A) { /* ok */
}
void PAR_SuffixOk25(int abc_mAs) { /* ok */
}
void PAR_SuffixOk26(int abc_mAh) { /* ok */
}
void PAR_SuffixOk27(int abc_As) { /* ok */
}
void PAR_SuffixOk28(int abc_Wh) { /* ok */
}
void PAR_SuffixOk29(int abc_t) { /* ok */
}
void PAR_SuffixOk30(int abc_Hz) { /* ok */
}
void PAR_SuffixNotOk00(int abc_SuffixNotOk00) { /* not ok: invalid suffix */
    // test: TESTCASE.add_expected_stdout("coding-style-naming.parameter_0.c:95:28: error: Paramater names shall (1) use camelCase, and optional (2) use a phyiscal unit as suffix. [abc_SuffixNotOk00] (Rule CodingStyle-Naming.Parameter)")
}

/* const */
void PAR_ConstOk00(const int kConstOk00) { /* ok */
}
void PAR_ConstNotOk00(const int constNotOk00) { /* not ok: 'k'-prefix missing */
    // test: TESTCASE.add_expected_stdout("coding-style-naming.parameter_0.c:102:33: error: Paramater names shall (1) use camelCase, and optional (2) use a phyiscal unit as suffix. [constNotOk00] (Rule CodingStyle-Naming.Parameter)")
}
void PAR_ConstNotOk01(const int kconstNotOk01) { /* not ok: 'k'-prefix, but not 'PascalCase' */
    // test: TESTCASE.add_expected_stdout("coding-style-naming.parameter_0.c:105:33: error: Paramater names shall (1) use camelCase, and optional (2) use a phyiscal unit as suffix. [kconstNotOk01] (Rule CodingStyle-Naming.Parameter)")
}
/* pointers */
void PAR_PointerOk00(int *pPointerOk00) { /* ok */
}
void PAR_PointerNotOk00(int *ointerNotOk00) { /* not ok: 'p'-prefix missing */
    // test: TESTCASE.add_expected_stdout("coding-style-naming.parameter_0.c:111:30: error: Paramater names shall (1) use camelCase, and optional (2) use a phyiscal unit as suffix. [ointerNotOk00] (Rule CodingStyle-Naming.Parameter)")
}
void PAR_PointerNotOk01(int *ppointerNotOk00) { /* not ok: 'p'-prefix, but not 'PascalCase' */
    // test: TESTCASE.add_expected_stdout("coding-style-naming.parameter_0.c:114:30: error: Paramater names shall (1) use camelCase, and optional (2) use a phyiscal unit as suffix. [ppointerNotOk00] (Rule CodingStyle-Naming.Parameter)")
}
/* pointer to const <type> */
void PAR_PointerToConstOk00(const int *pkPointerToConstOk00) { /* ok */
}
void PAR_PointerToConstNotOk00(const int *ointerToConstNotOk00) { /* not ok: 'pk'-prefix missing */
    // test: TESTCASE.add_expected_stdout("coding-style-naming.parameter_0.c:120:43: error: Paramater names shall (1) use camelCase, and optional (2) use a phyiscal unit as suffix. [ointerToConstNotOk00] (Rule CodingStyle-Naming.Parameter)")
}
void PAR_PointerToConstNotOk01(const int *pPointerToConstNotOk01) { /* not ok: 'k'-infix missing */
    // test: TESTCASE.add_expected_stdout("coding-style-naming.parameter_0.c:123:43: error: Paramater names shall (1) use camelCase, and optional (2) use a phyiscal unit as suffix. [pPointerToConstNotOk01] (Rule CodingStyle-Naming.Parameter)")
}
void PAR_PointerToConstNotOk02(const int *kPointerToConstNotOk02) { /* not ok: 'p'-prefix missing */
    // test: TESTCASE.add_expected_stdout("coding-style-naming.parameter_0.c:126:43: error: Paramater names shall (1) use camelCase, and optional (2) use a phyiscal unit as suffix. [kPointerToConstNotOk02] (Rule CodingStyle-Naming.Parameter)")
}
void PAR_PointerToConstNotOk03(const int *pkpointerToConstNotOk03) { /* not ok: not 'PascalCase' */
    // test: TESTCASE.add_expected_stdout("coding-style-naming.parameter_0.c:129:43: error: Paramater names shall (1) use camelCase, and optional (2) use a phyiscal unit as suffix. [pkpointerToConstNotOk03] (Rule CodingStyle-Naming.Parameter)")
}
void PAR_PointerToConstNotOk04(const int *kpPointerToConstNotOk04) { /* not ok: wrong order in prefix */
    // test: TESTCASE.add_expected_stdout("coding-style-naming.parameter_0.c:132:43: error: Paramater names shall (1) use camelCase, and optional (2) use a phyiscal unit as suffix. [kpPointerToConstNotOk04] (Rule CodingStyle-Naming.Parameter)")
}
/* const pointer to <type> */
void PAR_ConstPointerOk00(int *const kpConstPointerOk00) { /* ok */
}
void PAR_ConstPointerNotOk00(int *const constPointerNotOk00) { /* not ok: 'kp'-prefix missing */
    // test: TESTCASE.add_expected_stdout("coding-style-naming.parameter_0.c:138:41: error: Paramater names shall (1) use camelCase, and optional (2) use a phyiscal unit as suffix. [constPointerNotOk00] (Rule CodingStyle-Naming.Parameter)")
}
void PAR_ConstPointerNotOk01(int *const pConstPointerNotOk01) { /* not ok: 'k'-prefix missing */
    // test: TESTCASE.add_expected_stdout("coding-style-naming.parameter_0.c:141:41: error: Paramater names shall (1) use camelCase, and optional (2) use a phyiscal unit as suffix. [pConstPointerNotOk01] (Rule CodingStyle-Naming.Parameter)")
}
void PAR_ConstPointerNotOk02(int *const kConstPointerNotOk02) { /* not ok: 'p'-infix missing */
    // test: TESTCASE.add_expected_stdout("coding-style-naming.parameter_0.c:144:41: error: Paramater names shall (1) use camelCase, and optional (2) use a phyiscal unit as suffix. [kConstPointerNotOk02] (Rule CodingStyle-Naming.Parameter)")
}
void PAR_ConstPointerNotOk03(int *const pkconstPointerNotOk03) { /* not ok: not 'PascalCase' */
    // test: TESTCASE.add_expected_stdout("coding-style-naming.parameter_0.c:147:41: error: Paramater names shall (1) use camelCase, and optional (2) use a phyiscal unit as suffix. [pkconstPointerNotOk03] (Rule CodingStyle-Naming.Parameter)")
}
void PAR_ConstPointerNotOk04(int *const pkConstPointerNotOk04) { /* not ok: wrong order in prefix */
    // test: TESTCASE.add_expected_stdout("coding-style-naming.parameter_0.c:150:41: error: Paramater names shall (1) use camelCase, and optional (2) use a phyiscal unit as suffix. [pkConstPointerNotOk04] (Rule CodingStyle-Naming.Parameter)")
}
/* const pointers to const */
void PAR_ConstPointerToConstOk00(const int *const kpkConstPointerToConstOk00) { /* ok */
}
void PAR_ConstPointerToConstNotOk00(const int *const constPointerToConstNotOk00) { /* not ok: 'kpk'-prefix missing */
    // test: TESTCASE.add_expected_stdout("coding-style-naming.parameter_0.c:156:54: error: Paramater names shall (1) use camelCase, and optional (2) use a phyiscal unit as suffix. [constPointerToConstNotOk00] (Rule CodingStyle-Naming.Parameter)")
}
void PAR_ConstPointerToConstNotOk01(const int *const kpConstPointerToConstNotOk01) { /* not ok: 'k'-infix missing */
    // test: TESTCASE.add_expected_stdout("coding-style-naming.parameter_0.c:159:54: error: Paramater names shall (1) use camelCase, and optional (2) use a phyiscal unit as suffix. [kpConstPointerToConstNotOk01] (Rule CodingStyle-Naming.Parameter)")
}
void PAR_ConstPointerToConstNotOk02(const int *const kkConstPointerToConstNotOk02) { /* not ok: 'p'-infix missing */
    // test: TESTCASE.add_expected_stdout("coding-style-naming.parameter_0.c:162:54: error: Paramater names shall (1) use camelCase, and optional (2) use a phyiscal unit as suffix. [kkConstPointerToConstNotOk02] (Rule CodingStyle-Naming.Parameter)")
}
void PAR_ConstPointerToConstNotOk03(const int *const pkConstPointerToConstNotOk03) { /* not ok: 'k'-prefix missing */
    // test: TESTCASE.add_expected_stdout("coding-style-naming.parameter_0.c:165:54: error: Paramater names shall (1) use camelCase, and optional (2) use a phyiscal unit as suffix. [pkConstPointerToConstNotOk03] (Rule CodingStyle-Naming.Parameter)")
}
