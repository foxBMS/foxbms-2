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

// @prefix  MOD

// test: analysis(['CodingStyle-Naming.LocalVariable', 'CodingStyle-Naming.Function', 'CodingStyle-Naming.Macro', 'CodingStyle-Naming.TypedefedStruct'])

/*
 * 'CodingStyle-Naming.Function': ensure that helper function is well styled
 * 'CodingStyle-Naming.Macro': ensure the included helper header is well styled
 * 'CodingStyle-Naming.TypedefedStruct': ensure the included helper structs are well styled
 */

/* Use the same database fake API as for 'CodingStyle-Naming.LocalVariable' test in
 * tests/axivion/config-test/coding-style-naming.global-variable_database_usage.c
 */
#include "coding-style-naming.global-variable_database.h"

void MOD_RunSomeFunction(void) {
    DATA_BLOCK_SOC_s tableSoc0            = {.header.uniqueId = DATA_BLOCK_ID_SOC}; /* ok */
    static DATA_BLOCK_SOC_s mod_tableSoc1 = {.header.uniqueId = DATA_BLOCK_ID_SOC}; /* ok */

    /* non-static local variable shall not use a prefix */
    DATA_BLOCK_SOC_s mod_tableSoc2 = {.header.uniqueId = DATA_BLOCK_ID_SOC}; /* not ok */
    // test: TESTCASE.add_expected_stdout("coding-style-naming.local-variable_database_usage.c:62:22: error: Local variables names shall (1) use the lowercase module prefix followed by an underscore if they are static otherwise omit the prefix and then (2) use camelCase, and optional (3) use a phyiscal unit as suffix. [mod_tableSoc2] (Rule CodingStyle-Naming.LocalVariable)")

    /* non-static local variable misses 'table' */
    DATA_BLOCK_SOC_s soc2 = {.header.uniqueId = DATA_BLOCK_ID_SOC}; /* not ok */
    // test: TESTCASE.add_expected_stdout("coding-style-naming.local-variable_database_usage.c:66:22: error: Local variables names shall (1) use the lowercase module prefix followed by an underscore if they are static otherwise omit the prefix and then (2) use camelCase, and optional (3) use a phyiscal unit as suffix. [soc2] (Rule CodingStyle-Naming.LocalVariable)")

    /* static local variable shall use the prefix */
    static DATA_BLOCK_SOC_s tableSoc3 = {.header.uniqueId = DATA_BLOCK_ID_SOC}; /* not ok */
    // test: TESTCASE.add_expected_stdout("coding-style-naming.local-variable_database_usage.c:70:29: error: Local variables names shall (1) use the lowercase module prefix followed by an underscore if they are static otherwise omit the prefix and then (2) use camelCase, and optional (3) use a phyiscal unit as suffix. [tableSoc3] (Rule CodingStyle-Naming.LocalVariable)")

    /* static local variable misses 'table' */
    static DATA_BLOCK_SOC_s mod_soc3 = {.header.uniqueId = DATA_BLOCK_ID_SOC}; /* not ok */
    // test: TESTCASE.add_expected_stdout("coding-style-naming.local-variable_database_usage.c:74:29: error: Local variables names shall (1) use the lowercase module prefix followed by an underscore if they are static otherwise omit the prefix and then (2) use camelCase, and optional (3) use a phyiscal unit as suffix. [mod_soc3] (Rule CodingStyle-Naming.LocalVariable)")

    /* silence compiler warnings */
    (void)tableSoc0;
    (void)mod_tableSoc1;
    (void)mod_tableSoc2;
    (void)soc2;
    (void)tableSoc3;
    (void)mod_soc3;
}
