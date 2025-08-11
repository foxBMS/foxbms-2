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
 * @file    boot_helper.h
 * @author  foxBMS Team
 * @date    2019-12-04 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup DRIVERS
 * @prefix  BOOT
 *
 * @brief   Header for the boot_helper.c that contains the implementation of
 *          the functions that can assist the functions in boot.c and boot.h
 * @details TODO
 */

#ifndef FOXBMS__BOOT_HELPER_H_
#define FOXBMS__BOOT_HELPER_H_

/*========== Includes =======================================================*/

#include "fstd_types.h"

/*========== Macros and Definitions =========================================*/
/** The reset value to write into RESET[1-0] of System Exception Control
 *  Register (SYSECR) according to docref:
 *  SPNU563A-March 2018 p. 197 Table 2-64.
 */
#define BOOT_RESET_VALUE_TO_WRITE (2u)

/** The number of bits to shift in order to set RESET[1-0] of System Exception
 *  Control Register (SYSECR) according to docref:
 *  SPNU563A-March 2018 p. 197 Table 2-64.
 */
#define BOOT_RESET_SHIFT_BITS_NUMBER (14u)

#ifndef UNITY_UNIT_TEST
/** Address of Free Running Counter 0 (FRC0) */
#define BOOT_SYSTEM_REG1_SYSECR (systemREG1->SYSECR)
#else
extern volatile uint32_t BOOT_SYSTEM_REG1_SYSECR;
#endif

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/
/** @brief   Load the flashed program info from flash to a global variable. */
extern void BOOT_LoadProgramInfoFromFlash(void);

/** @brief  Clean up the RAM. */
extern void BOOT_CleanUpRam(void);

/**
 * @brief   Update the program info, current vector table and backup vector
 *          table into flash.
 * @return  #STD_OK when the entire update process succeeds.
 */
extern STD_RETURN_TYPE_e BOOT_UpdateProgramInfoIntoFlash(void);

/**
 * @brief   Reset the flash area that stores the program info and reset the
 *          current vector table to bootloader's vector table.
 * @return  #STD_OK when the entire update process succeeds.
 */
extern STD_RETURN_TYPE_e BOOT_ResetBootInfo(void);

/**
 * @brief   Software reset MCU.
 */
extern void BOOT_SoftwareResetMcu(void);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
#endif
#endif /* FOXBMS__BOOT_HELPER_H_ */
