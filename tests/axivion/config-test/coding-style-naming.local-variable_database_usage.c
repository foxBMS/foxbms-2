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
    // test: TESTCASE.add_expected_stdout("coding-style-naming.local-variable_database_usage.c:21:22: error: Local variables names shall (1) use the lowercase module prefix followed by an underscore if they are static otherwise omit the prefix and then (2) use camelCase, and optional (3) use a phyiscal unit as suffix. [mod_tableSoc2] (Rule CodingStyle-Naming.LocalVariable)")

    /* non-static local variable misses 'table' */
    DATA_BLOCK_SOC_s soc2 = {.header.uniqueId = DATA_BLOCK_ID_SOC}; /* not ok */
    // test: TESTCASE.add_expected_stdout("coding-style-naming.local-variable_database_usage.c:25:22: error: Local variables names shall (1) use the lowercase module prefix followed by an underscore if they are static otherwise omit the prefix and then (2) use camelCase, and optional (3) use a phyiscal unit as suffix. [soc2] (Rule CodingStyle-Naming.LocalVariable)")

    /* static local variable shall use the prefix */
    static DATA_BLOCK_SOC_s tableSoc3 = {.header.uniqueId = DATA_BLOCK_ID_SOC}; /* not ok */
    // test: TESTCASE.add_expected_stdout("coding-style-naming.local-variable_database_usage.c:29:29: error: Local variables names shall (1) use the lowercase module prefix followed by an underscore if they are static otherwise omit the prefix and then (2) use camelCase, and optional (3) use a phyiscal unit as suffix. [tableSoc3] (Rule CodingStyle-Naming.LocalVariable)")

    /* static local variable misses 'table' */
    static DATA_BLOCK_SOC_s mod_soc3 = {.header.uniqueId = DATA_BLOCK_ID_SOC}; /* not ok */
    // test: TESTCASE.add_expected_stdout("coding-style-naming.local-variable_database_usage.c:33:29: error: Local variables names shall (1) use the lowercase module prefix followed by an underscore if they are static otherwise omit the prefix and then (2) use camelCase, and optional (3) use a phyiscal unit as suffix. [mod_soc3] (Rule CodingStyle-Naming.LocalVariable)")

    /* silence compiler warnings */
    (void)tableSoc0;
    (void)mod_tableSoc1;
    (void)mod_tableSoc2;
    (void)soc2;
    (void)tableSoc3;
    (void)mod_soc3;
}
