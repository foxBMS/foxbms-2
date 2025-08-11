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
 * @file    boot_cfg.h
 * @author  foxBMS Team
 * @date    2019-12-04 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup DRIVERS
 * @prefix  BOOT
 *
 * @brief   Header for the boot_cfg.c that contains the data structs, enums,
 *          global variables, and configuration relevant datatypes and
 *          variables
 * @details TODO
 */

#ifndef FOXBMS__BOOT_CFG_H_
#define FOXBMS__BOOT_CFG_H_

/*========== Includes =======================================================*/
#include <stdint.h>

/*========== Macros and Definitions =========================================*/
/** Maximum size of the flash memory prepared for program */
#define BOOT_PROGRAM_SIZE_MAX (0x003E0000u)

/** Size of the program info sector */
#define BOOT_PROGRAM_INFO_SECTOR_SIZE (0x8000u)

/** Program info sector address where the program info will be saved */
#ifndef UNITY_UNIT_TEST_BOOT_HELPER
#define BOOT_PROGRAM_INFO_SECTOR_ADDRESS (0x00018000u)
#else
#define BOOT_PROGRAM_INFO_SECTOR_ADDRESS (&testFlashProgramInfo[0u])
#endif

/** The address of the magic number that will be written once the program info
 *  has been written.
 */
#ifndef UNITY_UNIT_TEST_BOOT_HELPER
#define BOOT_PROGRAM_INFO_ADDRESS_MAGIC_NUM (BOOT_PROGRAM_INFO_SECTOR_ADDRESS)
#else
#define BOOT_PROGRAM_INFO_ADDRESS_MAGIC_NUM (&testFlashProgramInfo[0u])
#endif

/** The size of the magic numer (4 bytes) */
#define BOOT_PROGRAM_INFO_SIZE_MAGIC_NUM (0x04u)

/** The defined value of the magic number */
#define BOOT_PROGRAM_INFO_MAGIC_NUM (0xAAAAAAAAu)

/** The size of the program relevant information block (which will be loaded
 *  to/from boot_infoOfLastFlashedProgram) inside the program info sector */
#define BOOT_PROGRAM_INFO_SIZE_IN_BYTES (28u)

/** The size to shift from the start of the program info sector and the start
 *  of the program relevant information block (which will be loaded to/from
 *  boot_infoOfLastFlashedProgram) inside the program info sector */
#define BOOT_PROGRAM_INFO_ADDRESS_BASE_SHIFT (BOOT_PROGRAM_INFO_SIZE_MAGIC_NUM)

/** The addresses of the program relevant information block (which will be
 *  loaded to/from boot_infoOfLastFlashedProgram) inside the program info
 *  sector */
/**@{*/
#ifndef UNITY_UNIT_TEST_BOOT_HELPER
#define BOOT_PROGRAM_INFO_ADDRESS_BASE                 (BOOT_PROGRAM_INFO_SECTOR_ADDRESS + BOOT_PROGRAM_INFO_ADDRESS_BASE_SHIFT)
#define BOOT_PROGRAM_INFO_ADDRESS_PROGRAM_LEN          (BOOT_PROGRAM_INFO_ADDRESS_BASE)
#define BOOT_PROGRAM_INFO_ADDRESS_PROGRAM_START        ((BOOT_PROGRAM_INFO_ADDRESS_BASE) + (0x04u))
#define BOOT_PROGRAM_INFO_ADDRESS_CRC_8_BYTES          ((BOOT_PROGRAM_INFO_ADDRESS_BASE) + (0x08u))
#define BOOT_PROGRAM_INFO_ADDRESS_VECTOR_CRC_8_BYTES   ((BOOT_PROGRAM_INFO_ADDRESS_BASE) + (0x10u))
#define BOOT_PROGRAM_INFO_ADDRESS_IS_PROGRAM_AVAILABLE ((BOOT_PROGRAM_INFO_ADDRESS_BASE) + (0x18u))
#else
#define BOOT_PROGRAM_INFO_ADDRESS_BASE                 (&testFlashProgramInfo[BOOT_PROGRAM_INFO_ADDRESS_BASE_SHIFT])
#define BOOT_PROGRAM_INFO_ADDRESS_PROGRAM_LEN          (BOOT_PROGRAM_INFO_ADDRESS_BASE)
#define BOOT_PROGRAM_INFO_ADDRESS_PROGRAM_START        ((BOOT_PROGRAM_INFO_ADDRESS_BASE) + (0x04u))
#define BOOT_PROGRAM_INFO_ADDRESS_CRC_8_BYTES          ((BOOT_PROGRAM_INFO_ADDRESS_BASE) + (0x08u))
#define BOOT_PROGRAM_INFO_ADDRESS_VECTOR_CRC_8_BYTES   ((BOOT_PROGRAM_INFO_ADDRESS_BASE) + (0x10u))
#define BOOT_PROGRAM_INFO_ADDRESS_IS_PROGRAM_AVAILABLE ((BOOT_PROGRAM_INFO_ADDRESS_BASE) + (0x18u))
#endif
/**@}*/

/** Address of the backup vector table */
/**@{*/
#define BOOT_VECTOR_TABLE_BACKUP_ADDRESS_SHIFT (0x60u)
#define BOOT_VECTOR_TABLE_BACKUP_ADDRESS       ((BOOT_PROGRAM_INFO_ADDRESS_BASE) + (BOOT_VECTOR_TABLE_BACKUP_ADDRESS_SHIFT))
/**@}*/

/** Vector table address and size  */
#define BOOT_VECTOR_TABLE_START_ADDRESS (0x0001FFE0u)

/** Vector table address and size  */
#define BOOT_VECTOR_TABLE_SIZE (0x20u)

/** Start address of the program (foxBMS) */
#define BOOT_PROGRAM_START_ADDRESS (BOOT_VECTOR_TABLE_START_ADDRESS + BOOT_VECTOR_TABLE_SIZE)

/** Define the availability of the program through this magic number (any other
 * value than this shall mean that the program is not available) */
#define BOOT_PROGRAM_IS_AVAILABLE (0xCCCCCCCCu)

/** RAM start address */
#define BOOT_RAM_START_ADDRESS (0x08010000u)

/** RAM area size */
#define BOOT_RAM_SIZE (0x00060000u)

/** Sector buffer size in the MCU */
#define BOOT_SECTOR_BUFFER_SIZE (0x40000u)

/** Sector buffer address */
/**@{*/
#ifndef UNITY_UNIT_TEST_BOOT_HELPER
#define BOOT_SECTOR_BUFFER_START_ADDRESS (0x08030000u)
#define BOOT_SECTOR_BUFFER_END_ADDRESS   (0x0806FFFFu)
#else
#define BOOT_SECTOR_BUFFER_START_ADDRESS (&testRamSectorBuffer[0u])
#define BOOT_SECTOR_BUFFER_END_ADDRESS   (&testRamSectorBuffer[BOOT_SECTOR_BUFFER_SIZE - 1u])
#endif
/**@}*/

/**  Corresponding sector buffer address of magic number */
#define BOOT_SECTOR_BUFFER_MAGIC_NUMBER_START_ADDRESS (BOOT_SECTOR_BUFFER_START_ADDRESS)

/**  Corresponding sector buffer address of program info */
#define BOOT_SECTOR_BUFFER_PROGRAM_INFO_START_ADDRESS \
    (BOOT_SECTOR_BUFFER_START_ADDRESS + BOOT_PROGRAM_INFO_ADDRESS_BASE_SHIFT)

/** Corresponding sector buffer address of the current vector table*/
#define BOOT_SECTOR_BUFFER_VECTOR_TABLE_START_ADDRESS \
    ((BOOT_SECTOR_BUFFER_START_ADDRESS) + (BOOT_PROGRAM_INFO_SECTOR_SIZE) - (BOOT_VECTOR_TABLE_SIZE))

/** Corresponding sector buffer address of the backup vector table */
#define BOOT_SECTOR_BUFFER_BACKUP_VECTOR_TABLE_START_ADDRESS                         \
    ((BOOT_SECTOR_BUFFER_START_ADDRESS) + (BOOT_VECTOR_TABLE_BACKUP_ADDRESS_SHIFT) + \
     (BOOT_PROGRAM_INFO_ADDRESS_BASE_SHIFT))

/** Number of bytes in 64 bits */
#define BOOT_NUM_OF_BYTES_IN_64_BITS (8u)

/** Number of data bytes contained in a vector table */
#define BOOT_NUM_OF_BYTES_IN_ONE_DATA_TRANSFER_LOOP (8u)

/** Number of vector table parts (8 bytes) in a vector table */
#define BOOT_NUM_OF_VECTOR_TABLE_8_BYTES (4u)

/** Number of data loops (each loop contains 8 bytes of data) in one
 *  subsector */
#define BOOT_NUM_OF_LOOPS_IN_ONE_SUB_SECTOR (1024u)

/** Program relevant information.
 *  This data will be loaded from the flash memory. */
typedef struct {
    uint32_t programLength;
    uint32_t programStartAddress;
    uint64_t programCrc8Bytes;
    uint64_t vectorTableCrc8Bytes;
    uint32_t isProgramAvailable;
} BOOT_PROGRAM_INFO_s;

/** Program's Vector table. */
typedef struct {
    uint64_t vectorTable[BOOT_NUM_OF_VECTOR_TABLE_8_BYTES];
} BOOT_VECTOR_TABLE_s;

/** Boot FSM states. */
typedef enum {
    BOOT_FSM_STATE_WAIT  = 1u,
    BOOT_FSM_STATE_RESET = 2u,
    BOOT_FSM_STATE_RUN   = 3u,
    BOOT_FSM_STATE_LOAD  = 4u,
    BOOT_FSM_STATE_ERROR = 5u,
} BOOT_FSM_STATES_e;

/*========== Extern Constant and Variable Declarations ======================*/
/** Program infomation of the last flashed program */
extern BOOT_PROGRAM_INFO_s boot_infoOfLastFlashedProgram;

/** Backup vector table of the bootloader */
extern BOOT_VECTOR_TABLE_s boot_backupVectorTable;

/** Current vector table of the bootloader */
extern BOOT_VECTOR_TABLE_s boot_currentVectorTable;

/** Current state of the boot FSM state */
extern BOOT_FSM_STATES_e boot_state;

#ifdef UNITY_UNIT_TEST_BOOT_HELPER
/** Reserve a memory space for program info sector in unit tests */
extern uint8_t testFlashProgramInfo[BOOT_PROGRAM_INFO_SECTOR_SIZE];

/** Reserve a memory space for Sector buffer in unit tests */
extern uint8_t testRamSectorBuffer[BOOT_SECTOR_BUFFER_SIZE];
#endif

/*========== Extern Function Prototypes =====================================*/

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
extern void BOOT_NothingToTest(void);
#endif

#endif /* FOXBMS__BOOT_CFG_H_ */
