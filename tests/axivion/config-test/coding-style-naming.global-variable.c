// @prefix  GLV

// test: analysis(['CodingStyle-Naming.GlobalVariable', 'CodingStyle-Naming.Function', 'CodingStyle-Naming.LocalVariable'])

/*
 * 'CodingStyle-Naming.Function': ensure that helper function is well styled
 * 'CodingStyle-Naming.LocalVariable': ensure the non-global (i.e., the local test) is well styled
 */

int glv_a                                  = 0; /* ok */
int glv_someCamelCase                      = 0; /* ok */
int glv_someCamelCaseWithNumbers2          = 0; /* ok */
int glv_someCamelCaseWithNumbers2InBetween = 0; /* ok */

int no_prefix_a      = 0; /* not ok: no prefix; not camelCase --> correction: 'glv_noPrefixA' */
int a                = 0; /* not ok: no prefix; --> correction: 'glv_a' */
int glv_NotCamelCase = 0; /* not ok: not camelCase --> correction: 'glv_notCamelCase' */
// test: TESTCASE.add_expected_stdout("coding-style-naming.global-variable.c:15:5: error: Global variables names shall (1) use the lowercase module prefix followed by an underscore and then (2) use camelCase, and optional (3) use a phyiscal unit as suffix. [no_prefix_a] (Rule CodingStyle-Naming.GlobalVariable)")
// test: TESTCASE.add_expected_stdout("coding-style-naming.global-variable.c:16:5: error: Global variables names shall (1) use the lowercase module prefix followed by an underscore and then (2) use camelCase, and optional (3) use a phyiscal unit as suffix. [a] (Rule CodingStyle-Naming.GlobalVariable)")
// test: TESTCASE.add_expected_stdout("coding-style-naming.global-variable.c:17:5: error: Global variables names shall (1) use the lowercase module prefix followed by an underscore and then (2) use camelCase, and optional (3) use a phyiscal unit as suffix. [glv_NotCamelCase] (Rule CodingStyle-Naming.GlobalVariable)")

int Abc     = 0; /* not ok: no prefix; not camelCase --> correction: 'glv_abc' */
int DEF     = 0; /* not ok: no prefix; not camelCase --> correction: 'glv_def' */
int glv_DEF = 0; /* not ok: not camelCase --> correction: 'glv_def' */
int GLV_abc = 0; /* not ok: prefix not lowercase --> correction: 'glv_abc' */
// test: TESTCASE.add_expected_stdout("coding-style-naming.global-variable.c:22:5: error: Global variables names shall (1) use the lowercase module prefix followed by an underscore and then (2) use camelCase, and optional (3) use a phyiscal unit as suffix. [Abc] (Rule CodingStyle-Naming.GlobalVariable)")
// test: TESTCASE.add_expected_stdout("coding-style-naming.global-variable.c:23:5: error: Global variables names shall (1) use the lowercase module prefix followed by an underscore and then (2) use camelCase, and optional (3) use a phyiscal unit as suffix. [DEF] (Rule CodingStyle-Naming.GlobalVariable)")
// test: TESTCASE.add_expected_stdout("coding-style-naming.global-variable.c:24:5: error: Global variables names shall (1) use the lowercase module prefix followed by an underscore and then (2) use camelCase, and optional (3) use a phyiscal unit as suffix. [glv_DEF] (Rule CodingStyle-Naming.GlobalVariable)")
// test: TESTCASE.add_expected_stdout("coding-style-naming.global-variable.c:25:5: error: Global variables names shall (1) use the lowercase module prefix followed by an underscore and then (2) use camelCase, and optional (3) use a phyiscal unit as suffix. [GLV_abc] (Rule CodingStyle-Naming.GlobalVariable)")

/* a file-scope static variable is a 'GlobalVariable' for Axivion */
static int glv_abc = 0; /* ok */
static int abc     = 0; /* not ok: no prefix; --> correction: 'glv_abc' */
static int glv_Abc = 0; /* not ok: not camelCase; --> correction: 'glv_abc' */
static int GLV_Blu = 0; /* not ok: uppercase prefix; not camelCase; --> correction: 'glv_blu' */
// test: TESTCASE.add_expected_stdout("coding-style-naming.global-variable.c:33:12: error: Global variables names shall (1) use the lowercase module prefix followed by an underscore and then (2) use camelCase, and optional (3) use a phyiscal unit as suffix. [abc] (Rule CodingStyle-Naming.GlobalVariable)")
// test: TESTCASE.add_expected_stdout("coding-style-naming.global-variable.c:34:12: error: Global variables names shall (1) use the lowercase module prefix followed by an underscore and then (2) use camelCase, and optional (3) use a phyiscal unit as suffix. [glv_Abc] (Rule CodingStyle-Naming.GlobalVariable)")
// test: TESTCASE.add_expected_stdout("coding-style-naming.global-variable.c:35:12: error: Global variables names shall (1) use the lowercase module prefix followed by an underscore and then (2) use camelCase, and optional (3) use a phyiscal unit as suffix. [GLV_Blu] (Rule CodingStyle-Naming.GlobalVariable)")

/* check suffixes */
int glv_foo_kOhm = 0; /* ok */
int glv_foo_As   = 0; /* ok */
int glv_foo_kHz  = 0; /* ok */
int glv_foo_Abc  = 0; /* not ok: no valid suffix */
// test: TESTCASE.add_expected_stdout("coding-style-naming.global-variable.c:44:5: error: Global variables names shall (1) use the lowercase module prefix followed by an underscore and then (2) use camelCase, and optional (3) use a phyiscal unit as suffix. [glv_foo_Abc] (Rule CodingStyle-Naming.GlobalVariable)")

void GLV_DummyFunction(void) {
    /* a function-scope static variable is **NOT** a 'GlobalVariable' for
     * Axivion, therefore this **MUST NOT** trigger an error. */
    static int glv_def = 0;
    int blu;

    /* silence compiler warnings (variables not being used is not important for
     * the naming test) */
    (void)glv_def;
    (void)blu;
}

/* silence compiler warnings (variables not being used is not important for the
 * naming test) */
void GLV_SilenceWarnings(void) {
    (void)glv_abc;
    (void)abc;
    (void)glv_Abc;
    (void)GLV_Blu;

    (void)glv_foo_As;
    (void)glv_foo_kHz;
    (void)glv_foo_Abc;
}
