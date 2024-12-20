// @prefix  PAR

// test: analysis(['CodingStyle-Naming.Parameter', 'CodingStyle-Naming.Function'])

/*
 * 'CodingStyle-Naming.Function': ensure that helper functions are well styled
 */

#include <stdint.h>

void PAR_Function0(uint8_t AAmessageData) { /* not ok, uses/starts with uppercase letter */
    // test: TESTCASE.add_expected_stdout("coding-style-naming.parameter_1.c:11:28: error: Paramater names shall (1) use camelCase, and optional (2) use a phyiscal unit as suffix. [AAmessageData] (Rule CodingStyle-Naming.Parameter)")
}

/* this is a collection of valid parameter names, that were actually used in the
 * code base, but due to bugs in the checker implementation, these parameter
 * names were labeled as invalid.
 * The purpose of the this check is to make sure, that these valid names
 * will no longer be flagged as invalid. */
/* clang-format off */
void PAR_Function01(uint8_t messageData)           { /* ok */ }
void PAR_Function02(uint8_t value1)                { /* ok */ }
void PAR_Function03(uint8_t pRegisterAddress)      { /* ok */ }
void PAR_Function04(uint8_t kpReceiveMessage)      { /* ok */ }
void PAR_Function05(uint8_t spi)                   { /* ok */ }
void PAR_Function06(uint8_t pc)                    { /* ok */ }
void PAR_Function08(uint8_t controlOrDiagnostic)   { /* ok */ }
void PAR_Function09(uint8_t outputAllDevices)      { /* ok */ }
void PAR_Function10(uint8_t readData)              { /* ok */ }
void PAR_Function11(uint8_t writeData)             { /* ok */ }
void PAR_Function12(uint8_t data)                  { /* ok */ }
void PAR_Function13(uint8_t cellVoltage_mV)        { /* ok */ }
void PAR_Function14(uint8_t cellTemperature_ddegC) { /* ok */ }
void PAR_Function15(uint8_t x1)                    { /* ok */ }
/* clang-format on */
