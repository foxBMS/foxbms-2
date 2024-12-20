// @prefix  DATA

// test: analysis(['CodingStyle-Naming.GlobalVariable', 'CodingStyle-Naming.Function', 'CodingStyle-Naming.Macro', 'CodingStyle-Naming.TypedefedStruct'])

/*
 * 'CodingStyle-Naming.Function': ensure that helper function is well styled
 * 'CodingStyle-Naming.Macro': ensure the included helper header is well styled
 * 'CodingStyle-Naming.TypedefedStruct': ensure the included helper structs are well styled
 */

#include "coding-style-naming.global-variable_database.h"

/* a file-scope static variable is a 'GlobalVariable' for Axivion */
static DATA_BLOCK_SOC_s data_blockSoc = {.header.uniqueId = DATA_BLOCK_ID_SOC}; /* ok */

static DATA_BLOCK_SOC_s data_notBlockSoc = {.header.uniqueId = DATA_BLOCK_ID_SOC}; /* not ok */
// test: TESTCASE.add_expected_stdout("coding-style-naming.global-variable_database.c:16:25: error: Global variables names shall (1) use the lowercase module prefix followed by an underscore and then (2) use camelCase, and optional (3) use a phyiscal unit as suffix. [data_notBlockSoc] (Rule CodingStyle-Naming.GlobalVariable)")

/* silence compiler warnings (variables not being used is not important for the
 * naming test) */
void DATA_SilenceWarnings(void) {
    (void)data_blockSoc;
    (void)data_notBlockSoc;
}
