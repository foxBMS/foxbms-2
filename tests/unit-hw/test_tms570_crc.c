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

/**
 * @file    test_tms570_crc.c
 * @author  foxBMS Team
 * @date    2023-08-01 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup DRIVERS
 * @prefix  CRC
 *
 * @brief   Test the hardware dependent behavior of function in CRC module
 * @details TODO
 */

/*========== Includes =======================================================*/
#include "test_tms570_crc.h"

#include "boot_cfg.h"
#include "can_cfg.h"

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/
uint8_t TEST_CRC_SemiAutoCrcCalculation(void) {
    /* Prepare the test data */
    uint8_t data_8_bytes[8u] = {0x12u, 0x34u, 0x56u, 0x78u, 0x9Au, 0xBCu, 0xDEu, 0xF0u};
    uint8_t *dest            = (uint8_t *)BOOT_SECTOR_BUFFER_START_ADDRESS;
    for (uint64_t i = 0; i < 1024 * 16; i++) {
        memcpy(dest, data_8_bytes, 8u);
        dest += 8u;
    }
    /* Calculate the CRC signature using the onboard resources */
    uint64_t crc_64_signature =
        CRC_SemiAutoCrcCalculation((uint32_t)BOOT_SECTOR_BUFFER_START_ADDRESS, (uint32_t)(1024 * 16), (uint64_t)0u);
    if (crc_64_signature != (uint64_t)0xda2f3a154d659ea5) {
        return 1u;
    }
    return 0u;
}

uint8_t TEST_CRC_CalculateCrc64(void) {
    /* Prepare a vector table */
    extern CAN_DATA_TRANSFER_INFO_s can_infoOfDataTransfer;
    can_infoOfDataTransfer.vectorTable[0] = 0xea00efd7eafffffeu;
    can_infoOfDataTransfer.vectorTable[1] = 0xea000fceeafffffeu;
    can_infoOfDataTransfer.vectorTable[2] = 0xeafffffeea00f1a6u;
    can_infoOfDataTransfer.vectorTable[3] = 0xe51ff1b0e51ff1b0u;

    /* Call software based crc function to calculate the crc signature for vector table */
    uint64_t crc64OfVectorTableOnBoard = CRC_CalculateCrc64(can_infoOfDataTransfer.vectorTable, 4u, 0u);
    if (crc64OfVectorTableOnBoard != 0x8ab8e993caef6489u) {
        return 1u;
    }
    return 0u;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
