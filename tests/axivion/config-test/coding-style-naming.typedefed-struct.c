// @prefix  TDS

// test: analysis(['CodingStyle-Naming.TypedefedStruct', 'CodingStyle-Naming.StructField'])

/*
 * 'CodingStyle-Naming.StructField': ensure the used struct fields are well styled
 */

typedef struct {
    int value;
} TDS_FOO_s;

typedef struct {
    int value;
} TDS_FOO_S;
// test: TESTCASE.add_expected_stdout("coding-style-naming.typedefed-struct.c:15:3: error: Typedefed struct names shall (1) use the uppercase module prefix followed by an underscore, (2) be all uppercase, (3) separate words by underscores, and (3) end with '_s'. [TDS_FOO_S] (Rule CodingStyle-Naming.TypedefedStruct)")

typedef struct {
    int value;
} TDS_foo_s;
// test: TESTCASE.add_expected_stdout("coding-style-naming.typedefed-struct.c:20:3: error: Typedefed struct names shall (1) use the uppercase module prefix followed by an underscore, (2) be all uppercase, (3) separate words by underscores, and (3) end with '_s'. [TDS_foo_s] (Rule CodingStyle-Naming.TypedefedStruct)")

typedef struct {
    int value;
} Tds_foo_s;
// test: TESTCASE.add_expected_stdout("coding-style-naming.typedefed-struct.c:25:3: error: Typedefed struct names shall (1) use the uppercase module prefix followed by an underscore, (2) be all uppercase, (3) separate words by underscores, and (3) end with '_s'. [Tds_foo_s] (Rule CodingStyle-Naming.TypedefedStruct)")

typedef struct {
    int value;
} WRONG_PREFIX_FOO_s;
// test: TESTCASE.add_expected_stdout("coding-style-naming.typedefed-struct.c:30:3: error: Typedefed struct names shall (1) use the uppercase module prefix followed by an underscore, (2) be all uppercase, (3) separate words by underscores, and (3) end with '_s'. [WRONG_PREFIX_FOO_s] (Rule CodingStyle-Naming.TypedefedStruct)")
