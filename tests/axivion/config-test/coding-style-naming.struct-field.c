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
// test: TESTCASE.add_expected_stdout("coding-style-naming.struct-field.c:57:9: error: Struct field names shall (1) use camelCase, and optional (2) use a phyiscal unit as suffix. [some_Resistance_kOhm] (Rule CodingStyle-Naming.StructField)")
// test: TESTCASE.add_expected_stdout("coding-style-naming.struct-field.c:58:9: error: Struct field names shall (1) use camelCase, and optional (2) use a phyiscal unit as suffix. [NotCamelcase_kOhm] (Rule CodingStyle-Naming.StructField)")
// test: TESTCASE.add_expected_stdout("coding-style-naming.struct-field.c:59:9: error: Struct field names shall (1) use camelCase, and optional (2) use a phyiscal unit as suffix. [BLA] (Rule CodingStyle-Naming.StructField)")

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
// test: TESTCASE.add_expected_stdout("coding-style-naming.struct-field.c:97:9: error: Struct field names shall (1) use camelCase, and optional (2) use a phyiscal unit as suffix. [value_degc] (Rule CodingStyle-Naming.StructField)")
// test: TESTCASE.add_expected_stdout("coding-style-naming.struct-field.c:98:9: error: Struct field names shall (1) use camelCase, and optional (2) use a phyiscal unit as suffix. [value_asd] (Rule CodingStyle-Naming.StructField)")
