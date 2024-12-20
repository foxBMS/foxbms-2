// @prefix  TDE

// test: analysis(['CodingStyle-Naming.TypedefedEnum', 'CodingStyle-Naming.Enumerator'])

/*
 * 'CodingStyle-Naming.Enumerator': ensure the used enumerators are well styled
 */

typedef enum {
    TDE_DUMMY_VALUE_A0,
} TDE_MY_ENUM_A_e; /*!< ok */

typedef enum {
    TDE_DUMMY_VALUE_B0,
} TDE_MY_ENUM_B_E; /*!< not ok */
// test: TESTCASE.add_expected_stdout("coding-style-naming.typedefed-enum.c:15:3: error: Typedefed enum names shall (1) use the uppercase module prefix followed by an underscore, (2) be all uppercase, (3) separate words by underscores, and (3) end with '_e'. [TDE_MY_ENUM_B_E] (Rule CodingStyle-Naming.TypedefedEnum)")

typedef enum {
    TDE_DUMMY_VALUE_C0,
} TDE_MY_Enum_C_e; /*!< not ok */
// test: TESTCASE.add_expected_stdout("coding-style-naming.typedefed-enum.c:20:3: error: Typedefed enum names shall (1) use the uppercase module prefix followed by an underscore, (2) be all uppercase, (3) separate words by underscores, and (3) end with '_e'. [TDE_MY_Enum_C_e] (Rule CodingStyle-Naming.TypedefedEnum)")

typedef enum {
    TDE_DUMMY_VALUE_D0,
} tde_MY_ENUM_D_E; /*!< not ok */
// test: TESTCASE.add_expected_stdout("coding-style-naming.typedefed-enum.c:25:3: error: Typedefed enum names shall (1) use the uppercase module prefix followed by an underscore, (2) be all uppercase, (3) separate words by underscores, and (3) end with '_e'. [tde_MY_ENUM_D_E] (Rule CodingStyle-Naming.TypedefedEnum)")

typedef enum {
    TDE_DUMMY_VALUE_E0,
} WRONG_PREFIX_MY_ENUM_E_e; /*!< not ok */
// test: TESTCASE.add_expected_stdout("coding-style-naming.typedefed-enum.c:30:3: error: Typedefed enum names shall (1) use the uppercase module prefix followed by an underscore, (2) be all uppercase, (3) separate words by underscores, and (3) end with '_e'. [WRONG_PREFIX_MY_ENUM_E_e] (Rule CodingStyle-Naming.TypedefedEnum)")
