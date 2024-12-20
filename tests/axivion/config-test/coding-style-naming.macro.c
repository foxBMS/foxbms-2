// @prefix  MM

// test: analysis(['CodingStyle-Naming.Macro'])

#include "coding-style-naming.macro.h"

#include "coding-style-naming.macro_1.h"

#define MM_HALLO    /* prefix ok,     casing ok            --> ok */
#define MM_HALLO_ms /* prefix ok,     casing ok, suffix ok --> ok */

#define MACRO_MISSES_PREFIX     /* prefix not ok, casing ok     --> not ok */
#define no_prefixand_bad_casing /* prefix not ok, casing not ok --> not ok */
#define MM_macro_has_bad_casing /* prefix ok,     casing not ok --> not ok */
// test: TESTCASE.add_expected_stdout("coding-style-naming.macro.c:12:9: error: Macro names shall (1) use the uppercase module prefix followed by an underscore, (2) be all uppercase, (3) separate words by underscores, and optional (4) use a phyiscal unit as suffix. [MACRO_MISSES_PREFIX] (Rule CodingStyle-Naming.Macro)")
// test: TESTCASE.add_expected_stdout("coding-style-naming.macro.c:13:9: error: Macro names shall (1) use the uppercase module prefix followed by an underscore, (2) be all uppercase, (3) separate words by underscores, and optional (4) use a phyiscal unit as suffix. [no_prefixand_bad_casing] (Rule CodingStyle-Naming.Macro)")
// test: TESTCASE.add_expected_stdout("coding-style-naming.macro.c:14:9: error: Macro names shall (1) use the uppercase module prefix followed by an underscore, (2) be all uppercase, (3) separate words by underscores, and optional (4) use a phyiscal unit as suffix. [MM_macro_has_bad_casing] (Rule CodingStyle-Naming.Macro)")

/* coding-style-naming.macro_1.h uses an invalid define guard */
// test: TESTCASE.add_expected_stdout("coding-style-naming.macro_1.h:4:9: error: Macro names shall (1) use the uppercase module prefix followed by an underscore, (2) be all uppercase, (3) separate words by underscores, and optional (4) use a phyiscal unit as suffix. [FOXBMS__CODING_STYLE_NAMING_MACRO_INVALID_H_] (Rule CodingStyle-Naming.Macro)")

#define MM_HALLO_Wh /* prefix ok,     casing ok, suffix ok --> ok */
