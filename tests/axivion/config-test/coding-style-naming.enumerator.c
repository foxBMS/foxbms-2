// @prefix  ENUM

// test: analysis(['CodingStyle-Naming.Enumerator', 'CodingStyle-Naming.TypedefedEnum'])

/* 'CodingStyle-Naming.TypedefedEnum': to ensure that these names are as they should be, i.e., to have consistency in the tests. */

typedef enum {
    ENUM_TEST_CASE_A0, /*!< ok */
    ENUM_TEST_CASE_A1, /*!< ok */
} ENUM_MY_ENUM_A_e;

typedef enum {
    ENUM_TEST_CASE_B0, /*!< ok */
    ENUM_Test_CASE_B1, /*!< not ok */
} ENUM_MY_ENUM_B_e;
// test: TESTCASE.add_expected_stdout("coding-style-naming.enumerator.c:14:5: error: Enumerator names shall (1) use the uppercase module prefix followed by an underscore, (2) be all uppercase, (3) separate words by underscores, and optional (4) use a phyiscal unit as suffix. [ENUM_Test_CASE_B1] (Rule CodingStyle-Naming.Enumerator)")

typedef enum {
    ENUM_TEST_CASE_C1_kOhm, /*!< ok */
    ENUM_TEST_CASE_C2_abc,  /*!< not ok */
} ENUM_MY_ENUM_C_e;
// test: TESTCASE.add_expected_stdout("coding-style-naming.enumerator.c:20:5: error: Enumerator names shall (1) use the uppercase module prefix followed by an underscore, (2) be all uppercase, (3) separate words by underscores, and optional (4) use a phyiscal unit as suffix. [ENUM_TEST_CASE_C2_abc] (Rule CodingStyle-Naming.Enumerator)")
