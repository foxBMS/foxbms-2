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

// @prefix  MM

// test: analysis(['CodingStyle-Naming.Macro'])

#include "coding-style-naming.macro.h"

#include "coding-style-naming.macro_1.h"

#define MM_HALLO    /* prefix ok,     casing ok            --> ok */
#define MM_HALLO_ms /* prefix ok,     casing ok, suffix ok --> ok */

#define MACRO_MISSES_PREFIX     /* prefix not ok, casing ok     --> not ok */
#define no_prefixand_bad_casing /* prefix not ok, casing not ok --> not ok */
#define MM_macro_has_bad_casing /* prefix ok,     casing not ok --> not ok */
// test: TESTCASE.add_expected_stdout("coding-style-naming.macro.c:53:9: error: Macro names shall (1) use the uppercase module prefix followed by an underscore, (2) be all uppercase, (3) separate words by underscores, and optional (4) use a phyiscal unit as suffix. [MACRO_MISSES_PREFIX] (Rule CodingStyle-Naming.Macro)")
// test: TESTCASE.add_expected_stdout("coding-style-naming.macro.c:54:9: error: Macro names shall (1) use the uppercase module prefix followed by an underscore, (2) be all uppercase, (3) separate words by underscores, and optional (4) use a phyiscal unit as suffix. [no_prefixand_bad_casing] (Rule CodingStyle-Naming.Macro)")
// test: TESTCASE.add_expected_stdout("coding-style-naming.macro.c:55:9: error: Macro names shall (1) use the uppercase module prefix followed by an underscore, (2) be all uppercase, (3) separate words by underscores, and optional (4) use a phyiscal unit as suffix. [MM_macro_has_bad_casing] (Rule CodingStyle-Naming.Macro)")

/* coding-style-naming.macro_1.h uses an invalid define guard */
// test: TESTCASE.add_expected_stdout("coding-style-naming.macro_1.h:45:9: error: Macro names shall (1) use the uppercase module prefix followed by an underscore, (2) be all uppercase, (3) separate words by underscores, and optional (4) use a phyiscal unit as suffix. [FOXBMS__CODING_STYLE_NAMING_MACRO_INVALID_H_] (Rule CodingStyle-Naming.Macro)")

#define MM_HALLO_Wh /* prefix ok,     casing ok, suffix ok --> ok */
