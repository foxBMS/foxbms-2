/*
 * CRC64_calc.h
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
 * - Added doxygen comment
 * - Use foxBMS-style include guard
 */

#ifndef FOXBMS__CRC64_CALC_H_
#define FOXBMS__CRC64_CALC_H_

#include "HL_hal_stdtypes.h"

/**
 * @brief   This function is a user function to extern the crc64_update_word
 * to calculate the CRC signature of the input uint64_t data.
 * @param   crc64   the root of the 64-bits CRC signature
 * @param   data    the uint64 input data for the calculation of CRC signature
 */
uint64 calc_crc64_user(uint64 crc64, uint64 data);

#endif /* FOXBMS__CRC64_CALC_H_ */
