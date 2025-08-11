/*
 * CRC64_calc.c
 *
 * Function to calculate CRC64 in software.
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 *
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
*/

/* This code is based on the file extracted from the downloaded project
 * (http://www.ti.com/lit/zip/spna235), which is linked in
 * https://www.ti.com/lit/an/spna235/spna235.pdf.
 * Inside the zip file file is found in
 * 'spna235/CCS_Projects/Supporting Functions/source'.
 * - In this file a 'calc_crc64_user' function is implemented to call
 *   'crc64_update_word' directly without swapping.
 * - In addition to that the swapping part has also been removed in
 *   'crc64_update_word'.
 * - 'C++-style' comments have been replaced by 'C-style' comments, see
 *   https://en.cppreference.com/w/cpp/comment
 * - Code has been improved to conform with the MISRA-C standard
 * - EOF marker has been removed
 */

#include "crc64_calc.h"

#include "HL_hal_stdtypes.h"

static __inline uint64 crc64_update_word(uint64 crc64, uint64 data) {
    /* AXIVION Routine Generic-MissingParameterAssert: crc64: parameter accepts whole range */
    /* AXIVION Routine Generic-MissingParameterAssert: data: parameter accepts whole range */
    int8_t i       = 0;
    int8_t j       = 0;
    uint64 nextCrc = 0u;

/* for i in 63 to 0 loop */
#pragma MUST_ITERATE(64, 64);
#pragma UNROLL(64);
    for (i = 63; i >= 0; i--) {
        /* NEXT_CRC_VAL(0) := CRC_VAL(63) xor DATA(i); */
        nextCrc = (nextCrc & 0xFFFFFFFFFFFFFFFEuLL) | ((crc64 >> 63) ^ (data >> i));

/* for j in 1 to 63 loop */
#pragma MUST_ITERATE(63, 63);
#pragma UNROLL(63);
        for (j = 1; j < 64; j++) {
            /* case j is */
            /* when 1|3|4 => */
            if ((j == 1) || (j == 3) || (j == 4)) {
                /* NEXT_CRC_VAL(j) := CRC_VAL(j - 1) xor CRC_VAL(63) xor DATA(i); */
                nextCrc = (nextCrc & ~(1uLL << j)) | ((((crc64 >> (j - 1)) ^ (crc64 >> 63) ^ (data >> i)) & 1) << j);
            } else { /* when others => */
                /* NEXT_CRC_VAL(j) := CRC_VAL(j - 1); */
                nextCrc = (nextCrc & ~(1uLL << j)) | (((crc64 >> (j - 1)) & 1) << j);
            }
            /* end case; */
        } /* end loop; */
        crc64 = nextCrc;
    } /* end loop */

    return crc64;
}

/* This function is a user function modified based on calc_crc64 to calculate
 only one crc signature at a time */
uint64 calc_crc64_user(uint64 crc64, uint64 data) {
    /* AXIVION Routine Generic-MissingParameterAssert: crc64: parameter accepts whole range */
    /* AXIVION Routine Generic-MissingParameterAssert: data: parameter accepts whole range */
    return crc64_update_word(crc64, data);
}
