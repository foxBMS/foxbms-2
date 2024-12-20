// @prefix  FUN

// test: analysis(['CodingStyle-Naming.Function'])

void FUN_InitializeSomeDevice(void) { /* ok */
}

void InitializeSomeDevice(void) { /* prefix missing --> not ok */
}
// test: TESTCASE.add_expected_stdout("coding-style-naming.function.c:8:6: error: Function names shall (1) use the uppercase module prefix followed by an underscore and then (2) use PascalCase. [InitializeSomeDevice()] (Rule CodingStyle-Naming.Function)")

void FUN_initializeSomeDevice(void) { /* not PascalCase --> not ok */
}
// test: TESTCASE.add_expected_stdout("coding-style-naming.function.c:12:6: error: Function names shall (1) use the uppercase module prefix followed by an underscore and then (2) use PascalCase. [FUN_initializeSomeDevice()] (Rule CodingStyle-Naming.Function)")

void FUN_InitializeSomeDevice_(void) { /* not alphanumeric due to '_' --> not ok */
}
// test: TESTCASE.add_expected_stdout("coding-style-naming.function.c:16:6: error: Function names shall (1) use the uppercase module prefix followed by an underscore and then (2) use PascalCase. [FUN_InitializeSomeDevice_()] (Rule CodingStyle-Naming.Function)")

void FUN_RunSomeFunctionWithNumberInName2(void) { /* ok */
}

void FUN_RunSomeFunctionWithNumberInName2Foo(void) { /* ok */
}

/* externalize static function for unit testing */
void TEST_FUN_RunSomeFunctionWithNumberInName2Foo(void) { /* ok */
}

/* externalize static function for unit testing uses wrong casing */
void test_FUN_RunSomeFunctionWithNumberInName2Foo(void) { /* not ok */
}
// test: TESTCASE.add_expected_stdout("coding-style-naming.function.c:31:6: error: Function names shall (1) use the uppercase module prefix followed by an underscore and then (2) use PascalCase. [test_FUN_RunSomeFunctionWithNumberInName2Foo()] (Rule CodingStyle-Naming.Function)")

void ABC_FUN_RunSomeFunctionWithNumberInName2Foo(void) { /* not ok */
}
// test: TESTCASE.add_expected_stdout("coding-style-naming.function.c:35:6: error: Function names shall (1) use the uppercase module prefix followed by an underscore and then (2) use PascalCase. [ABC_FUN_RunSomeFunctionWithNumberInName2Foo()] (Rule CodingStyle-Naming.Function)")
