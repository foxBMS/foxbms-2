// @prefix  TEST

// test: analysis(['CodingStyle-Naming.Function'])

void TEST_SomeTestHelperFunction(void) { /* ok */
}

void testRunTestXyz(void) { /* ok */
}

void test_RunTestXyz(void) { /* not ok; correction: testRunTestXyz */
}
// test: TESTCASE.add_expected_stdout("test_coding-style-naming.function.c:11:6: error: Function names shall (1) use the uppercase module prefix followed by an underscore and then (2) use PascalCase. [test_RunTestXyz()] (Rule CodingStyle-Naming.Function)")

/* not ok: wrong prefix --> correction: TEST_RunMyTest1 or testRunMyTest1 when it is test function */
void MISC_RunMyTest1(void) {
}
// test: TESTCASE.add_expected_stdout("test_coding-style-naming.function.c:16:6: error: Function names shall (1) use the uppercase module prefix followed by an underscore and then (2) use PascalCase. [MISC_RunMyTest1()] (Rule CodingStyle-Naming.Function)")
