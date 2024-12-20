// @prefix  STF

// test: analysis(['CodingStyle-Naming.StructField', 'CodingStyle-Naming.TypedefedStruct'])

/*
 * 'CodingStyle-Naming.TypedefedStruct': ensure the used helper structs are well styled
 */

typedef struct {
    int value;               /* ok */
    int myOtherValue;        /* ok */
    int someResistance_kOhm; /* ok */
} STF_FOO_s;

typedef struct {
    int some_Resistance_kOhm; /* not ok */
    int NotCamelcase_kOhm;    /* not ok */
    int BLA;                  /* not ok */
} STF_BAR_s;
// test: TESTCASE.add_expected_stdout("coding-style-naming.struct-field.c:16:9: error: Struct field names shall (1) use camelCase, and optional (2) use a phyiscal unit as suffix. [some_Resistance_kOhm] (Rule CodingStyle-Naming.StructField)")
// test: TESTCASE.add_expected_stdout("coding-style-naming.struct-field.c:17:9: error: Struct field names shall (1) use camelCase, and optional (2) use a phyiscal unit as suffix. [NotCamelcase_kOhm] (Rule CodingStyle-Naming.StructField)")
// test: TESTCASE.add_expected_stdout("coding-style-naming.struct-field.c:18:9: error: Struct field names shall (1) use camelCase, and optional (2) use a phyiscal unit as suffix. [BLA] (Rule CodingStyle-Naming.StructField)")

typedef struct {
    int value1;       /* ok */
    int value1Abc;    /* ok */
    int value1_degC;  /* ok */
    int value1Abc_As; /* ok */
} STF_NUMBERS_IN_STRUCT_FIELD_NAME_s;

typedef struct {
    int value_degC;  /* ok: valid suffix */
    int value_ddegC; /* ok: valid suffix */
    int value_dK;    /* ok: valid suffix */
    int value_ohm;   /* ok: valid suffix */
    int value_kOhm;  /* ok: valid suffix */
    int value_kHz;   /* ok: valid suffix */
    int value_ms;    /* ok: valid suffix */
    int value_us;    /* ok: valid suffix */
    int value_perc;  /* ok: valid suffix */
    int value_perm;  /* ok: valid suffix */
    int value_mV;    /* ok: valid suffix */
    int value_V;     /* ok: valid suffix */
    int value_mA;    /* ok: valid suffix */
    int value_A;     /* ok: valid suffix */
    int value_mAs;   /* ok: valid suffix */
    int value_mAh;   /* ok: valid suffix */
    int value_As;    /* ok: valid suffix */
    int value_W;     /* ok: valid suffix */
    int value_Wh;    /* ok: valid suffix */
    int value_t;     /* ok: valid suffix */
    int value_Hz;    /* ok: valid suffix */
} STF_VALID_SUFFIXES_s;

typedef struct {
    int value_degc; /* not ok: invalid suffix */
    int value_asd;  /* not ok: invalid suffix */
} STF_INVALID_SUFFIXES_s;
// test: TESTCASE.add_expected_stdout("coding-style-naming.struct-field.c:56:9: error: Struct field names shall (1) use camelCase, and optional (2) use a phyiscal unit as suffix. [value_degc] (Rule CodingStyle-Naming.StructField)")
// test: TESTCASE.add_expected_stdout("coding-style-naming.struct-field.c:57:9: error: Struct field names shall (1) use camelCase, and optional (2) use a phyiscal unit as suffix. [value_asd] (Rule CodingStyle-Naming.StructField)")
