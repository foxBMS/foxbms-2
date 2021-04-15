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
 * @file    masterinfo.h
 * @author  foxBMS Team
 * @date    2020-07-08 (date of creation)
 * @updated 2020-07-08 (date of last update)
 * @ingroup GENERAL
 * @prefix  MINFO
 *
 * @brief   General foxBMS-master system information
 *
 * @details Header file for the implementation to gather information on the
 *          foxBMS-Master and its connected peripherals on the master PCB.
 *
 */

#ifndef FOXBMS__MASTERINFO_H_
#define FOXBMS__MASTERINFO_H_

/*========== Includes =======================================================*/
#include "HL_system.h"

/*========== Macros and Definitions =========================================*/
/** values describing the connection state of the debug probe */
typedef enum MINFO_DEBUG_PROBE_CONNECTION_STATE {
    MINFO_DEBUG_PROBE_NOT_CONNECTED, /*!< no debug probe connected */
    MINFO_DEBUG_PROBE_CONNECTED,     /*!< debug probe connected */
} MINFO_DEBUG_PROBE_CONNECTION_STATE_e;

/**
 * @brief   state of the system
 * @details This currently describes the source of the last reset and whether a
 *          debug probe is attached or not.
 */
typedef struct MINFO_MASTER_STATE {
    resetSource_t resetSource;                       /*!< source of the last system reset */
    MINFO_DEBUG_PROBE_CONNECTION_STATE_e debugProbe; /*!< connection state of debug probe */
} MINFO_MASTER_STATE_s;

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/
/**
 * @brief Set reason for last reset
 *
 * @param[in] resetSource  source of last reset
 */
void MINFO_SetResetSource(resetSource_t resetSource);

/**
 * @brief Get reason for last reset
 *
 * @return returns reset reason
 */
resetSource_t MINFO_GetResetSource(void);

/**
 * @brief Check if debug probe is connected
 */
void MINFO_SetDebugProbeConnectionState(MINFO_DEBUG_PROBE_CONNECTION_STATE_e state);

/**
 * @brief Get state if debugger is connected or not
 *
 * @return #MINFO_DEBUG_PROBE_CONNECTION_STATE_e if debugger connected or not
 */
MINFO_DEBUG_PROBE_CONNECTION_STATE_e MINFO_GetDebugProbeConnectionState(void);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/

#endif /* FOXBMS__MASTERINFO_H_ */
