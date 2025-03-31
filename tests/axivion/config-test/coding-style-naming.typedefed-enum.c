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

// @prefix  TDE

// test: analysis(['CodingStyle-Naming.TypedefedEnum', 'CodingStyle-Naming.Enumerator'])

/*
 * 'CodingStyle-Naming.Enumerator': ensure the used enumerators are well styled
 */

typedef enum {
    TDE_DUMMY_VALUE_A0,
} TDE_MY_ENUM_A_e; /*!< ok */

typedef enum {
    TDE_DUMMY_VALUE_B0,
} TDE_MY_ENUM_B_E; /*!< not ok */
// test: TESTCASE.add_expected_stdout("coding-style-naming.typedefed-enum.c:56:3: error: Typedefed enum names shall (1) use the uppercase module prefix followed by an underscore, (2) be all uppercase, (3) separate words by underscores, and (3) end with '_e'. [TDE_MY_ENUM_B_E] (Rule CodingStyle-Naming.TypedefedEnum)")

typedef enum {
    TDE_DUMMY_VALUE_C0,
} TDE_MY_Enum_C_e; /*!< not ok */
// test: TESTCASE.add_expected_stdout("coding-style-naming.typedefed-enum.c:61:3: error: Typedefed enum names shall (1) use the uppercase module prefix followed by an underscore, (2) be all uppercase, (3) separate words by underscores, and (3) end with '_e'. [TDE_MY_Enum_C_e] (Rule CodingStyle-Naming.TypedefedEnum)")

typedef enum {
    TDE_DUMMY_VALUE_D0,
} tde_MY_ENUM_D_E; /*!< not ok */
// test: TESTCASE.add_expected_stdout("coding-style-naming.typedefed-enum.c:66:3: error: Typedefed enum names shall (1) use the uppercase module prefix followed by an underscore, (2) be all uppercase, (3) separate words by underscores, and (3) end with '_e'. [tde_MY_ENUM_D_E] (Rule CodingStyle-Naming.TypedefedEnum)")

typedef enum {
    TDE_DUMMY_VALUE_E0,
} WRONG_PREFIX_MY_ENUM_E_e; /*!< not ok */
// test: TESTCASE.add_expected_stdout("coding-style-naming.typedefed-enum.c:71:3: error: Typedefed enum names shall (1) use the uppercase module prefix followed by an underscore, (2) be all uppercase, (3) separate words by underscores, and (3) end with '_e'. [WRONG_PREFIX_MY_ENUM_E_e] (Rule CodingStyle-Naming.TypedefedEnum)")
