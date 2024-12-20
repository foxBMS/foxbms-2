// @prefix  MOD

// test: analysis(['CodingStyle-Naming.GlobalVariable', 'CodingStyle-Naming.Function', 'CodingStyle-Naming.Macro', 'CodingStyle-Naming.TypedefedStruct'])

/*
 * 'CodingStyle-Naming.Function': ensure that helper function is well styled
 * 'CodingStyle-Naming.Macro': ensure the included helper header is well styled
 * 'CodingStyle-Naming.TypedefedStruct': ensure the included helper structs are well styled
 */

#include "coding-style-naming.global-variable_database.h"

/* a file-scope static variable is a 'GlobalVariable' for Axivion */
DATA_BLOCK_SOC_s mod_tableSoc0 = {.header.uniqueId = DATA_BLOCK_ID_SOC}; /* ok */

DATA_BLOCK_SOC_s mod_notTableSoc0 = {.header.uniqueId = DATA_BLOCK_ID_SOC}; /* not ok */
// test: TESTCASE.add_expected_stdout("coding-style-naming.global-variable_database_usage.c:16:18: error: Global variables names shall (1) use the lowercase module prefix followed by an underscore and then (2) use camelCase, and optional (3) use a phyiscal unit as suffix. [mod_notTableSoc0] (Rule CodingStyle-Naming.GlobalVariable)")

/* a file-scope static variable is a 'GlobalVariable' for Axivion */
static DATA_BLOCK_SOC_s mod_tableSoc1 = {.header.uniqueId = DATA_BLOCK_ID_SOC}; /* ok */

static DATA_BLOCK_SOC_s mod_notTableSoc1 = {.header.uniqueId = DATA_BLOCK_ID_SOC}; /* not ok */
// test: TESTCASE.add_expected_stdout("coding-style-naming.global-variable_database_usage.c:22:25: error: Global variables names shall (1) use the lowercase module prefix followed by an underscore and then (2) use camelCase, and optional (3) use a phyiscal unit as suffix. [mod_notTableSoc1] (Rule CodingStyle-Naming.GlobalVariable)")

/* silence compiler warnings (variables not being used is not important for the
 * naming test) */
void MOD_SilenceWarnings(void) {
    (void)mod_tableSoc0;
    (void)mod_notTableSoc0;
    (void)mod_tableSoc1;
    (void)mod_notTableSoc1;
}
