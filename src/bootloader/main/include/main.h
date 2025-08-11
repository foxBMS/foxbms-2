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
 * @file    main.h
 * @author  foxBMS Team
 * @date    2024-09-02 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup MAIN
 * @prefix  MAIN
 *
 * @brief   Main function header
 * @details Declares the main function
 *          For the target build this header declares the standard main
 *          function.
 *          For the unit test build this header declares an alternative name
 *          for main so that it is unit testable.
 */

#ifndef FOXBMS__MAIN_H_
#define FOXBMS__MAIN_H_

/*========== Includes =======================================================*/
#include <stdint.h>

/*========== Macros and Definitions =========================================*/
/** Maximum time in microsends that the bootloader waits for an update request before starting the application. */
#define MAIN_TIME_OUT_us (500000u)

/*========== Extern Constant and Variable Declarations ======================*/
/* flash-module = implementation in flash.c */
/** Address in the MCU's flash of the flash-module program.
 *  The value is assigned through the linker command file.
 */
extern uint32_t main_textLoadStartFlashC;

/** Size of the flash-module.
 *  The value is assigned through the linker command file */
extern uint32_t main_textSizeFlashC;

/** Address in the MCU's RAM where the flash-module will be positioned.
 *  The value is assigned through the linker command file.
 */
extern uint32_t main_textRunStartFlashC;

/* flash-config-module = implementation in flash_cfg.c.
 * All variables in the flash-config-module must be constants.
 * see src/bootloader/driver/config/flash_cfg.c for details.
 */
/** Address in the MCU's flash of the flash-config-module (i.e., the
 *  constants).
 *  The value is assigned through the linker command file.
 */
extern uint32_t main_constLoadStartFlashCfgC;

/** Size of the flash-config-module (i.e., its constants).
 *  The value is assigned through the linker command file.
 */
extern uint32_t main_constSizeFlashCfgC;

/** Address in the MCU's RAM where the flash-config-module constants will be
 *  positioned.
 *  The value is assigned through the linker command file.
 */
extern uint32_t main_constRunStartFlashCfgC;

/*========== Extern Function Prototypes =====================================*/
/**
 * @brief   main function of the bootloader
 * @details entry point to the bootloader
 */
#ifndef UNITY_UNIT_TEST
int main(void);
#else
int unit_test_main(void);
#endif

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
#endif

#endif /* FOXBMS__MAIN_H_ */
