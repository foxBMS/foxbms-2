/**
 *
 * @copyright &copy; 2010 - 2023, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * - &Prime;This product uses parts of foxBMS&reg;&Prime;
 * - &Prime;This product includes parts of foxBMS&reg;&Prime;
 * - &Prime;This product is derived from foxBMS&reg;&Prime;
 *
 */

/**
 * @file    crc.h
 * @author  foxBMS Team
 * @date    2022-02-22 (date of creation)
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
 * @ingroup DRIVERS
 * @prefix  CRC
 *
 * @brief   crc module header
 *
 * @details Uses the system CRC hardware for data integrity calculation
 */

#ifndef FOXBMS__CRC_H_
#define FOXBMS__CRC_H_

/*========== Includes =======================================================*/

#include "HL_crc.h"

#include "fstd_types.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/** High 32 bit part of the seed used for CRC calculation */
#define CRC_SEED_HIGH (0xBADC0DEDu)
/** Low 32 bit part of the seed used for CRC calculation */
#define CRC_SEED_LOW (0xDEADBEEFu)
/** Mask used to clear bits in config register to set CRC module to Data Capture Mode */
#define CRC_DATA_CAPTURE_MODE_CLEAR_MASK (0xFFFFFFFCu)
/** Mask used to set bits in config register to set CRC module to Full-CPU Mode */
#define CRC_FULL_CPU_MODE_SET_MASK (0x3u)

/* Size in bits of a byte, used for shift operations */
#define CRC_BYTE_SIZE_IN_BITS (8u)
/* Size in bytes of a register, used for shift operations */
#define CRC_REGISTER_SIZE_IN_BYTES (4u)
/* Size in bits of a register, used for shift operations */
#define CRC_REGISTER_SIZE_IN_BITS (32u)
/* Literal used to reverse byte order */
#define CRC_REVERSE_BYTES_ORDER (3u)

/** CRC signature register, low of high part to be used .
 *  The register is 64 bit wide but the data is read in
 *  32 bit chunks. This indicates if the low or the high part
 *  has to be written.
 */
typedef enum {
    CRC_REGISTER_LOW,  /* low part of register to be written */
    CRC_REGISTER_HIGH, /* high part of register to be written */
} CRC_REGISTER_SIDE_e;

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/

/**
 * @brief   Computes CRC of data flow
 * @details This function uses the hardware of the MCU.
 *          Data must be given in  byte chunks.
 * @param[in]   pCrc            uint64_t pointer to the calculated CRC
 * @param[in]   pData           uint8_t pointer to the start of the data flow
 * @param[in]   lengthInBytes   length in bytes of the data flow
 * @returns     #STD_OK if CRC hardware available, #STD_NOT_OK otherwise
 */
extern STD_RETURN_TYPE_e CRC_CalculateCrc(uint64_t *pCrc, uint8_t *pData, uint32_t lengthInBytes);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
#endif

#endif /* FOXBMS__CRC_H_ */
