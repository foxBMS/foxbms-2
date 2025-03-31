/**
 *
 * @copyright &copy; 2010 - 2025, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * We kindly request you to use one or more of the following phrases to refer to
 * foxBMS in your hardware, software, documentation or advertising materials:
 *
 * - "This product uses parts of foxBMS&reg;"
 * - "This product includes parts of foxBMS&reg;"
 * - "This product is derived from foxBMS&reg;"
 *
 */

// @prefix  PAR

// test: analysis(['CodingStyle-Naming.Parameter', 'CodingStyle-Naming.Function'])

/*
 * 'CodingStyle-Naming.Function': ensure that helper functions are well styled
 */

#include <stdint.h>

void PAR_Function0(uint8_t AAmessageData) { /* not ok, uses/starts with uppercase letter */
    // test: TESTCASE.add_expected_stdout("coding-style-naming.parameter_1.c:52:28: error: Paramater names shall (1) use camelCase, and optional (2) use a phyiscal unit as suffix. [AAmessageData] (Rule CodingStyle-Naming.Parameter)")
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
