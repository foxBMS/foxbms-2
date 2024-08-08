/*
 * @prefix  TEST
 */

// test: analysis(['CodingStyle-Naming.Parameter'])
#include <stdint.h>

void TEST_Function0(uint8_t AAmessageData) { /* not ok, uses/starts with uppercase letter*/
    // test: TESTCASE.add_expected_stdout("test_parameter-names-1.c:8:29: error: Names of parameters shall use the naming convention camelCase (and optional unit suffix preceded by _). [AAmessageData] (Rule CodingStyle-Naming.Parameter)")
    // test: TESTCASE.add_expected_stdout("test_parameter-names-1.c:8:29: error: The parameter name is not 'camelCase' or does not pre- or suffix the parameter correctly. [AAmessageData] (Rule CodingStyle-Naming.Parameter)")
}

/* this is a collection of valid parameter names, that were actually used in the
 * code base, but due to bugs in the checker implementation, these parameter
 * names were labeled as invalid.
 * The purpose of the this check is to make sure, that these valid names
 * will no longer be flagged as invalid. */
/* clang-format off */
void TEST_Function01(uint8_t messageData)           { /* ok */ }
void TEST_Function02(uint8_t value1)                { /* ok */ }
void TEST_Function03(uint8_t pRegisterAddress)      { /* ok */ }
void TEST_Function04(uint8_t kpReceiveMessage)      { /* ok */ }
void TEST_Function05(uint8_t spi)                   { /* ok */ }
void TEST_Function06(uint8_t pc)                    { /* ok */ }
void TEST_Function08(uint8_t controlOrDiagnostic)   { /* ok */ }
void TEST_Function09(uint8_t outputAllDevices)      { /* ok */ }
void TEST_Function10(uint8_t readData)              { /* ok */ }
void TEST_Function11(uint8_t writeData)             { /* ok */ }
void TEST_Function12(uint8_t data)                  { /* ok */ }
void TEST_Function13(uint8_t cellVoltage_mV)        { /* ok */ }
void TEST_Function14(uint8_t cellTemperature_ddegC) { /* ok */ }
void TEST_Function15(uint8_t x1)                    { /* ok */ }
/* clang-format on */
