/*
 * @prefix  TEST
 */

// test: analysis(['CodingStyle-Naming.Parameter'])
#include <stdint.h>

void TEST_VoidOk00(void) { /* ok */
}
/* suffixes */
void TEST_SuffixOk00(int abc) { /* ok */
}
void TEST_SuffixOk01(int abcPascalCase) { /* ok */
}
void TEST_SuffixOk02(int abc_degC) { /* ok */
}
void TEST_SuffixOk03(int abc_ddegC) { /* ok */
}
void TEST_SuffixOk04(int abc_dK) { /* ok */
}
void TEST_SuffixOk05(int abc_ohm) { /* ok */
}
void TEST_SuffixOk06(int abc_kOhm) { /* ok */
}
void TEST_SuffixOk07(int abc_ms) { /* ok */
}
void TEST_SuffixOk08(int abc_us) { /* ok */
}
void TEST_SuffixOk20(int abc_perc) { /* ok */
}
void TEST_SuffixOk21(int abc_mV) { /* ok */
}
void TEST_SuffixOk22(int abc_V) { /* ok */
}
void TEST_SuffixOk23(int abc_mA) { /* ok */
}
void TEST_SuffixOk24(int abc_A) { /* ok */
}
void TEST_SuffixOk25(int abc_mAs) { /* ok */
}
void TEST_SuffixOk26(int abc_mAh) { /* ok */
}
void TEST_SuffixOk27(int abc_As) { /* ok */
}
void TEST_SuffixOk28(int abc_Wh) { /* ok */
}
void TEST_SuffixOk29(int abc_t) { /* ok */
}
void TEST_SuffixOk30(int abc_Hz) { /* ok */
}
void TEST_SuffixNotOk00(int abc_SuffixNotOk00) { /* not ok: invalid suffix */
    // test: TESTCASE.add_expected_stdout("test_parameter-names.c:51:29: error: The parameter name is not 'camelCase' or does not pre- or suffix the parameter correctly. [abc_SuffixNotOk00] (Rule CodingStyle-Naming.Parameter)")
}

/* const */
void TEST_ConstOk00(const int kConstOk00) { /* ok */
}
void TEST_ConstNotOk00(const int constNotOk00) { /* not ok: 'k'-prefix missing */
    // test: TESTCASE.add_expected_stdout("test_parameter-names.c:58:34: error: The parameter name is not 'camelCase' or does not pre- or suffix the parameter correctly. [constNotOk00] (Rule CodingStyle-Naming.Parameter)")
}
void TEST_ConstNotOk01(const int kconstNotOk01) { /* not ok: 'k'-prefix, but not 'PascalCase' */
    // test: TESTCASE.add_expected_stdout("test_parameter-names.c:61:34: error: The parameter name is not 'camelCase' or does not pre- or suffix the parameter correctly. [kconstNotOk01] (Rule CodingStyle-Naming.Parameter)")
}
/* pointers */
void TEST_PointerOk00(int *pPointerOk00) { /* ok */
}
void TEST_PointerNotOk00(int *ointerNotOk00) { /* not ok: 'p'-prefix missing */
    // test: TESTCASE.add_expected_stdout("test_parameter-names.c:67:31: error: The parameter name is not 'camelCase' or does not pre- or suffix the parameter correctly. [ointerNotOk00] (Rule CodingStyle-Naming.Parameter)")
}
void TEST_PointerNotOk01(int *ppointerNotOk00) { /* not ok: 'p'-prefix, but not 'PascalCase' */
    // test: TESTCASE.add_expected_stdout("test_parameter-names.c:70:31: error: The parameter name is not 'camelCase' or does not pre- or suffix the parameter correctly. [ppointerNotOk00] (Rule CodingStyle-Naming.Parameter)")
}
/* pointer to const <type> */
void TEST_PointerToConstOk00(const int *pkPointerToConstOk00) { /* ok */
}
void TEST_PointerToConstNotOk00(const int *ointerToConstNotOk00) { /* not ok: 'pk'-prefix missing */
    // test: TESTCASE.add_expected_stdout("test_parameter-names.c:76:44: error: The parameter name is not 'camelCase' or does not pre- or suffix the parameter correctly. [ointerToConstNotOk00] (Rule CodingStyle-Naming.Parameter)")
}
void TEST_PointerToConstNotOk01(const int *pPointerToConstNotOk01) { /* not ok: 'k'-infix missing */
    // test: TESTCASE.add_expected_stdout("test_parameter-names.c:79:44: error: The parameter name is not 'camelCase' or does not pre- or suffix the parameter correctly. [pPointerToConstNotOk01] (Rule CodingStyle-Naming.Parameter)")
}
void TEST_PointerToConstNotOk02(const int *kPointerToConstNotOk02) { /* not ok: 'p'-prefix missing */
    // test: TESTCASE.add_expected_stdout("test_parameter-names.c:82:44: error: The parameter name is not 'camelCase' or does not pre- or suffix the parameter correctly. [kPointerToConstNotOk02] (Rule CodingStyle-Naming.Parameter)")
}
void TEST_PointerToConstNotOk03(const int *pkpointerToConstNotOk03) { /* not ok: not 'PascalCase' */
    // test: TESTCASE.add_expected_stdout("test_parameter-names.c:85:44: error: The parameter name is not 'camelCase' or does not pre- or suffix the parameter correctly. [pkpointerToConstNotOk03] (Rule CodingStyle-Naming.Parameter)")
}
void TEST_PointerToConstNotOk04(const int *kpPointerToConstNotOk04) { /* not ok: wrong order in prefix */
    // test: TESTCASE.add_expected_stdout("test_parameter-names.c:88:44: error: The parameter name is not 'camelCase' or does not pre- or suffix the parameter correctly. [kpPointerToConstNotOk04] (Rule CodingStyle-Naming.Parameter)")
}
/* const pointer to <type> */
void TEST_ConstPointerOk00(int *const kpConstPointerOk00) { /* ok */
}
void TEST_ConstPointerNotOk00(int *const constPointerNotOk00) { /* not ok: 'kp'-prefix missing */
    // test: TESTCASE.add_expected_stdout("test_parameter-names.c:94:42: error: The parameter name is not 'camelCase' or does not pre- or suffix the parameter correctly. [constPointerNotOk00] (Rule CodingStyle-Naming.Parameter)")
}
void TEST_ConstPointerNotOk01(int *const pConstPointerNotOk01) { /* not ok: 'k'-prefix missing */
    // test: TESTCASE.add_expected_stdout("test_parameter-names.c:97:42: error: The parameter name is not 'camelCase' or does not pre- or suffix the parameter correctly. [pConstPointerNotOk01] (Rule CodingStyle-Naming.Parameter)")
}
void TEST_ConstPointerNotOk02(int *const kConstPointerNotOk02) { /* not ok: 'p'-infix missing */
    // test: TESTCASE.add_expected_stdout("test_parameter-names.c:100:42: error: The parameter name is not 'camelCase' or does not pre- or suffix the parameter correctly. [kConstPointerNotOk02] (Rule CodingStyle-Naming.Parameter)")
}
void TEST_ConstPointerNotOk03(int *const pkconstPointerNotOk03) { /* not ok: not 'PascalCase' */
    // test: TESTCASE.add_expected_stdout("test_parameter-names.c:103:42: error: The parameter name is not 'camelCase' or does not pre- or suffix the parameter correctly. [pkconstPointerNotOk03] (Rule CodingStyle-Naming.Parameter)")
}
void TEST_ConstPointerNotOk04(int *const pkConstPointerNotOk04) { /* not ok: wrong order in prefix */
    // test: TESTCASE.add_expected_stdout("test_parameter-names.c:106:42: error: The parameter name is not 'camelCase' or does not pre- or suffix the parameter correctly. [pkConstPointerNotOk04] (Rule CodingStyle-Naming.Parameter)")
}
/* const pointers to const */
void TEST_ConstPointerToConstOk00(const int *const kpkConstPointerToConstOk00) { /* ok */
}
void TEST_ConstPointerToConstNotOk00(const int *const constPointerToConstNotOk00) { /* not ok: 'kpk'-prefix missing */
    // test: TESTCASE.add_expected_stdout("test_parameter-names.c:112:55: error: The parameter name is not 'camelCase' or does not pre- or suffix the parameter correctly. [constPointerToConstNotOk00] (Rule CodingStyle-Naming.Parameter)")
}
void TEST_ConstPointerToConstNotOk01(const int *const kpConstPointerToConstNotOk01) { /* not ok: 'k'-infix missing */
    // test: TESTCASE.add_expected_stdout("test_parameter-names.c:115:55: error: The parameter name is not 'camelCase' or does not pre- or suffix the parameter correctly. [kpConstPointerToConstNotOk01] (Rule CodingStyle-Naming.Parameter)")
}
void TEST_ConstPointerToConstNotOk02(const int *const kkConstPointerToConstNotOk02) { /* not ok: 'p'-infix missing */
    // test: TESTCASE.add_expected_stdout("test_parameter-names.c:118:55: error: The parameter name is not 'camelCase' or does not pre- or suffix the parameter correctly. [kkConstPointerToConstNotOk02] (Rule CodingStyle-Naming.Parameter)")
}
void TEST_ConstPointerToConstNotOk03(const int *const pkConstPointerToConstNotOk03) { /* not ok: 'k'-prefix missing */
    // test: TESTCASE.add_expected_stdout("test_parameter-names.c:121:55: error: The parameter name is not 'camelCase' or does not pre- or suffix the parameter correctly. [pkConstPointerToConstNotOk03] (Rule CodingStyle-Naming.Parameter)")
}
