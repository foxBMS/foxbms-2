/**
 *
 * @copyright &copy; 2010 - 2021, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * @file    ftask_how-to.c
 * @author  foxBMS Team
 * @date    2019-08-27 (date of creation)
 * @updated 2020-08-10 (date of last update)
 * @ingroup SOME_GROUP
 * @prefix  FTSK
 *
 * @brief   Implementation of some software
 *
 */

/*========== Includes =======================================================*/
#include "general.h"

#include "HL_gio.h"

/*========== Macros and Definitions =========================================*/

/** mock-up of the variadic macro for read access to the database*/
#define DATA_READ_DATA(...)
/** mock-up of the variadic macro for write access to the database */
#define DATA_WRITE_DATA(...)

/** mock-up of data block identification numbers */
typedef enum DATA_BLOCK_ID {
    DATA_BLOCK_ID_EXAMPLE, /**< some example database entry */
    /* ...
     * ...
     * ...
     */
    DATA_BLOCK_ID_MAX, /**< DO NOT CHANGE, MUST BE THE LAST ENTRY */
} DATA_BLOCK_ID_e;

/** mock-up-typedef for data block header as defined in database_cfg.h */
typedef struct DATA_BLOCKHEADER {
    DATA_BLOCK_ID_e uniqueId;   /*!< uniqueId of database entry */
    uint32_t timestamp;         /*!< timestamp of last database update */
    uint32_t previousTimestamp; /*!< timestamp of previous database update */
} DATA_BLOCK_HEADER_s;

/**
 * mock-up data block struct of example
 */
typedef struct DATA_BLOCK_EXAMPLE {
    /* This struct needs to be at the beginning of every database entry. During
    * the initialization of a database struct, uniqueId must be set to the
    * respective database entry representation in enum DATA_BLOCK_ID_e. */
    DATA_BLOCK_HEADER_s header;
    uint16_t dummyValue;
} DATA_BLOCK_EXAMPLE_s;

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/

/*========== Externalized Static Function Implementations (Unit Test) =======*/

/* ftask-example-cyclic-100ms-start */
void FTSK_RunUserCodeCyclic100ms(void) {
    /* user code */
    static uint32_t ftsk_cyclic_100ms_counter                = 0;
    static DATA_BLOCK_EXAMPLE_s ftsk_tableExampleCyclic100ms = {.header.uniqueId = DATA_BLOCK_ID_EXAMPLE};
    if ((ftsk_cyclic_100ms_counter % 10u) == 0u) {
        gioSetBit(gioPORTB, 6, gioGetBit(gioPORTB, 6) ^ 1);
    }

    DATA_READ_DATA(&ftsk_tableExampleCyclic100ms);
    ftsk_tableExampleCyclic100ms.dummyValue++;
    DATA_WRITE_DATA(&ftsk_tableExampleCyclic100ms);
    ftsk_cyclic_100ms_counter++;
}
/* ftask-example-cyclic-100ms-end */
