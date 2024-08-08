/**
 *
 * @copyright &copy; 2010 - 2024, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * @file    can_cbs_tx_crash-dump.c
 * @author  foxBMS Team
 * @date    2022-11-16 (date of creation)
 * @updated 2024-08-08 (date of last update)
 * @version v1.7.0
 * @ingroup DRIVERS
 * @prefix  CANTX
 *
 * @brief   CAN
 * @details CAN Tx callback for crash dump.
 *          These messages are not guaranteed to be sent, rather it is a best
 *          effort to get information during debugging why a fatal error
 *          occurred.
 */

/*========== Includes =======================================================*/
#include "general.h"

#include "can_cbs_tx_crash-dump.h"

#include "can.h"
#include "can_cfg_tx-async-message-definitions.h"
#include "can_helper.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/
/** @{
 * multiplexer setup for the crash dump message
 */
#define CANTX_CRASH_DUMP_MESSAGE_MUX_START_BIT (7u)
#define CANTX_CRASH_DUMP_MESSAGE_MUX_LENGTH    (8u)
/** @} */

#define CANTX_CRASH_DUMP_MESSAGE_MUX_VALUE_STACK_OVERFLOW (0x00u)

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/
extern void CANTX_CrashDump(CANTX_FATAL_ERRORS_ACTIONS_e action) {
    FAS_ASSERT(action < CANTX_FATAL_ERRORS_ACTIONS_MAX_E);
    uint8_t data[CANTX_CRASH_DUMP_DLC] = {GEN_REPEAT_U(0u, GEN_STRIP(CAN_MAX_DLC))};
    uint64_t messageData               = 0u;

    switch (action) {
        case CANTX_FATAL_ERRORS_ACTIONS_STACK_OVERFLOW:
            CAN_TxSetMessageDataWithSignalData(
                &messageData,
                CANTX_CRASH_DUMP_MESSAGE_MUX_START_BIT,
                CANTX_CRASH_DUMP_MESSAGE_MUX_LENGTH,
                CANTX_CRASH_DUMP_MESSAGE_MUX_VALUE_STACK_OVERFLOW,
                CANTX_CRASH_DUMP_ENDIANNESS);
            break;
        default:                  /* LCOV_EXCL_LINE */
            FAS_ASSERT(FAS_TRAP); /* LCOV_EXCL_LINE */
            break;                /* LCOV_EXCL_LINE */
    }

    /* set can data */
    CAN_TxSetCanDataWithMessageData(messageData, data, CANTX_CRASH_DUMP_ENDIANNESS);
    /* send message */
    (void)CAN_DataSend(CAN_NODE_DEBUG_MESSAGE, CANTX_CRASH_DUMP_ID, CANTX_CRASH_DUMP_ID_TYPE, data);
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
