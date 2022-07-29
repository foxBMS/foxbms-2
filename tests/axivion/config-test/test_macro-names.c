/*
 * @prefix  TEST
 */

// test: analysis(['CodingStyle-Naming.Macro'])

#include "test_macro-names.h"
#define TEST_HALLO           /* valid prefix, casing -> good */
#define TEST_HALLO_ms        /* valid prefix, casing and suffix -> good */
#define MACRO_MISSIS_PREFIX  /* must fail */
#define macro_has_bad_casing /* must fail */

// test: TESTCASE.add_expected_stdout("test_macro-names.c:10:9: error: The macro name is not 'ALL_CAPS'. [MACRO_MISSIS_PREFIX] (Rule CodingStyle-Naming.Macro)")
// test: TESTCASE.add_expected_stdout("test_macro-names.c:11:9: error: Macro names shall use the naming convention '(prefix.upper())_ALL_CAPS' (and optional unit suffix preceded by _) or for include guards 'FOXBMS__FILENAME_H_'. [macro_has_bad_casing] (Rule CodingStyle-Naming.Macro)")
// test: TESTCASE.add_expected_stdout("test_macro-names.c:11:9: error: The macro name is not 'ALL_CAPS'. [macro_has_bad_casing] (Rule CodingStyle-Naming.Macro)")
