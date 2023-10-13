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
 * @file    adi_ades183x_buffers.h
 * @author  foxBMS Team
 * @date    2022-12-07 (date of creation)
 * @updated 2023-10-12 (date of last update)
 * @version v1.6.0
 * @ingroup DRIVERS
 * @prefix  ADI
 *
 * @brief   Header for the buffers used by the driver for the ADI analog front-end.
 *
 * @details The buffer are defined in a separate file and used throughout the
 *          driver. Their definition is included with this header.
 *
 */

#ifndef FOXBMS__ADI_ADES183X_BUFFERS_H_
#define FOXBMS__ADI_ADES183X_BUFFERS_H_

/*========== Includes =======================================================*/
#include "adi_ades183x.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Declarations ======================*/

/* RequirementId: D7.1 V0R4 SIF-4.20.01.01 */
/** Driver copy of the configuration stored in the AFE configuration registers */
extern uint8_t adi_configurationRegisterAgroup[BS_NR_OF_STRINGS]
                                              [BS_NR_OF_MODULES_PER_STRING * ADI_MAX_REGISTER_SIZE_IN_BYTES];
extern uint8_t adi_configurationRegisterBgroup[BS_NR_OF_STRINGS]
                                              [BS_NR_OF_MODULES_PER_STRING * ADI_MAX_REGISTER_SIZE_IN_BYTES];

/** Configuration read from the AFE configuration registers, used for comparison with written values */
extern uint8_t adi_readConfigurationRegisterAgroup[BS_NR_OF_STRINGS]
                                                  [BS_NR_OF_MODULES_PER_STRING * ADI_MAX_REGISTER_SIZE_IN_BYTES];
extern uint8_t adi_readConfigurationRegisterBgroup[BS_NR_OF_STRINGS]
                                                  [BS_NR_OF_MODULES_PER_STRING * ADI_MAX_REGISTER_SIZE_IN_BYTES];

/* Variable used to set configuration bits in commands */
extern uint16_t adi_command[ADI_COMMAND_DEFINITION_LENGTH];

/**
 * Buffer used to write data to a register on the AFEs
 * 6 bytes per module, which is the maximum size of a register in the AFE
 * Same buffer used for 4 bytes wide registers
 */
extern uint8_t adi_dataTransmit[BS_NR_OF_MODULES_PER_STRING * ADI_MAX_REGISTER_SIZE_IN_BYTES];

/**
 * Buffer used to read data from a register on the AFEs
 * 6 bytes per module, which is the maximum size of a register in the AFE
 * Same buffer used for 4 bytes wide registers
 */
extern uint8_t adi_dataReceive[BS_NR_OF_MODULES_PER_STRING * ADI_MAX_REGISTER_SIZE_IN_BYTES];

/**
 * Buffer used to write the same data to a register for all the AFEs
 * 6 bytes, which is the maximum size of a register in the AFE
 * Same buffer used for 4 bytes wide registers
 * Used in the function ADI_WriteRegisterGlobal(): there the content of
 * adi_diagnosticWriteGlobal is copied everywhere in adi_dataTransmit
 */
extern uint8_t adi_writeGlobal[ADI_MAX_REGISTER_SIZE_IN_BYTES];

/* Bytes used by CLRFLAG command to indicate which flags have to be cleared */
extern uint8_t adi_clearFlagData[ADI_CLRFLAG_DATA_LENGTH];

/*========== Extern Function Prototypes =====================================*/

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
#endif

#endif /* FOXBMS__ADI_ADES183X_BUFFERS_H_ */
