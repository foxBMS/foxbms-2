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
 * @file    fram.h
 * @author  foxBMS Team
 * @date    2020-03-05 (date of creation)
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
 * @ingroup DRIVERS
 * @prefix  FRAM
 *
 * @brief   Header for the driver for the FRAM module
 *
 * @details TODO
 */

#ifndef FOXBMS__FRAM_H_
#define FOXBMS__FRAM_H_

/*========== Includes =======================================================*/
#include "fram_cfg.h"

#include "fstd_types.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/

/**
 * @brief   Initializes the addresses to be written in the FRAM.
 * @details This function must be called before any use of the FRAM.
 */
extern void FRAM_Initialize(void);

/**
 * @brief   Reinitialize all entries in the FRAM.
 * @details This function must be called if the alignment in the FRAM is
 *          wrong due to a change in the FRAM data structue.
 * @return  #STD_OK if all writes successfull, otherwise #STD_NOT_OK
 */
extern STD_RETURN_TYPE_e FRAM_ReinitializeAllEntries(void);

/* The variable corrresponding to the block_ID is written */
/**
 * @brief   Writes a variable to the FRAM.
 * @details This function stores the variable corresponding to the ID passed as
 *          parameter. Write can fail if SPI interface was locked.
 * @param   blockId ID of variable to write to FRAM
 * @return  one of the values of the FRAM_RETURN_TYPE_e enum
 */
extern FRAM_RETURN_TYPE_e FRAM_WriteData(FRAM_BLOCK_ID_e blockId);

/* The variable corrresponding to the block_ID is written */
/**
 * @brief   Reads a variable from the FRAM.
 * @details This function reads the variable corresponding to the ID passed as
 *          parameter. Read can fail if SPI interface was locked.
 * @param   blockId ID of variable to read from FRAM
 * @return  one of the values of the FRAM_RETURN_TYPE_e enum
 */
extern FRAM_RETURN_TYPE_e FRAM_ReadData(FRAM_BLOCK_ID_e blockId);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
#endif

#endif /* FOXBMS__FRAM_H_ */
