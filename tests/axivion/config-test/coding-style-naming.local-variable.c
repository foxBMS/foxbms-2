// @prefix  LOV

// test: analysis(['CodingStyle-Naming.LocalVariable', 'CodingStyle-Naming.Function'])

/*
 * 'CodingStyle-Naming.Function': ensure that helper functions are well styled
 */

void LOV_RunTestsWithValidLocalVariableNames0(void) {
    int aaa                = 0; /* ok */
    static int lov_bbb     = 0; /* ok */
    static int lov_ccc1    = 0; /* ok */
    static int lov_ccc1Abc = 0; /* ok */

    (void)aaa;         /* silence compiler warnings (not important for the naming test) */
    (void)lov_bbb;     /* silence compiler warnings (not important for the naming test) */
    (void)lov_ccc1;    /* silence compiler warnings (not important for the naming test) */
    (void)lov_ccc1Abc; /* silence compiler warnings (not important for the naming test) */
}

void LOV_RunTestsWithValidLocalVariableNames1(void) {
    int myVariable            = 0; /* ok */
    static int lov_myVariable = 0; /* ok */

    (void)myVariable;     /* silence compiler warnings (not important for the naming test) */
    (void)lov_myVariable; /* silence compiler warnings (not important for the naming test) */
}

void LOV_RunTestsWithValidLocalVariableNames2(void) {
    int myVariable_kOhm            = 0; /* ok */
    static int lov_myVariable_kOhm = 0; /* ok */

    (void)myVariable_kOhm;     /* silence compiler warnings (not important for the naming test) */
    (void)lov_myVariable_kOhm; /* silence compiler warnings (not important for the naming test) */
}

void LOV_RunTestsWithInValidLocalVariableNames0(void) {
    int M          = 0; /* not ok: not camelCase --> correction: 'm' */
    int MyVariable = 0; /* not ok: not camelCase --> correction: 'myVariable' */
    // test: TESTCASE.add_expected_stdout("coding-style-naming.local-variable.c:38:9: error: Local variables names shall (1) use the lowercase module prefix followed by an underscore if they are static otherwise omit the prefix and then (2) use camelCase, and optional (3) use a phyiscal unit as suffix. [M] (Rule CodingStyle-Naming.LocalVariable)")
    // test: TESTCASE.add_expected_stdout("coding-style-naming.local-variable.c:39:9: error: Local variables names shall (1) use the lowercase module prefix followed by an underscore if they are static otherwise omit the prefix and then (2) use camelCase, and optional (3) use a phyiscal unit as suffix. [MyVariable] (Rule CodingStyle-Naming.LocalVariable)")

    (void)M;          /* silence compiler warnings (not important for the naming test) */
    (void)MyVariable; /* silence compiler warnings (not important for the naming test) */
}

void LOV_RunTestsWithInValidLocalVariableNames1(void) {
    int My_kOhm = 0; /* not ok: not camelCase --> correction: 'my_kOhm' */
    int MyBla   = 0; /* not ok: not camelCase --> correction: 'myBla' */
    // test: TESTCASE.add_expected_stdout("coding-style-naming.local-variable.c:48:9: error: Local variables names shall (1) use the lowercase module prefix followed by an underscore if they are static otherwise omit the prefix and then (2) use camelCase, and optional (3) use a phyiscal unit as suffix. [My_kOhm] (Rule CodingStyle-Naming.LocalVariable)")
    // test: TESTCASE.add_expected_stdout("coding-style-naming.local-variable.c:49:9: error: Local variables names shall (1) use the lowercase module prefix followed by an underscore if they are static otherwise omit the prefix and then (2) use camelCase, and optional (3) use a phyiscal unit as suffix. [MyBla] (Rule CodingStyle-Naming.LocalVariable)")

    (void)My_kOhm; /* silence compiler warnings (not important for the naming test) */
    (void)MyBla;   /* silence compiler warnings (not important for the naming test) */
}

void LOV_RunTestsWithInValidLocalVariableNames2(void) {
    static int myVariable     = 0; /* not prefix for static */
    static int lov_MyVariable = 0; /* not camelCase */
    // test: TESTCASE.add_expected_stdout("coding-style-naming.local-variable.c:58:16: error: Local variables names shall (1) use the lowercase module prefix followed by an underscore if they are static otherwise omit the prefix and then (2) use camelCase, and optional (3) use a phyiscal unit as suffix. [myVariable] (Rule CodingStyle-Naming.LocalVariable)")
    // test: TESTCASE.add_expected_stdout("coding-style-naming.local-variable.c:59:16: error: Local variables names shall (1) use the lowercase module prefix followed by an underscore if they are static otherwise omit the prefix and then (2) use camelCase, and optional (3) use a phyiscal unit as suffix. [lov_MyVariable] (Rule CodingStyle-Naming.LocalVariable)")

    (void)myVariable;     /* silence compiler warnings (not important for the naming test) */
    (void)lov_MyVariable; /* silence compiler warnings (not important for the naming test) */
}

void LOV_RunNotStaticButPrefix(void) {
    int lov_myVariable = 0; /* prefix, but static */
    // test: TESTCASE.add_expected_stdout("coding-style-naming.local-variable.c:68:9: error: Local variables names shall (1) use the lowercase module prefix followed by an underscore if they are static otherwise omit the prefix and then (2) use camelCase, and optional (3) use a phyiscal unit as suffix. [lov_myVariable] (Rule CodingStyle-Naming.LocalVariable)")

    (void)lov_myVariable; /* silence compiler warnings (not important for the naming test) */
}
