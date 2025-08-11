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
 * @file    can_cbs_tx_imd-request.h
 * @author  foxBMS Team
 * @date    2022-11-16 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup DRIVERS
 * @prefix  CANTX
 *
 * @brief   Declarations for handling the transmit of imd request messages
 * @details This header declares the interface for transmitting imd request
 *          messages and everything else that is externally required to be able
 *          to use that interface.
 */

#ifndef FOXBMS__CAN_CBS_TX_IMD_REQUEST_H_
#define FOXBMS__CAN_CBS_TX_IMD_REQUEST_H_

/*========== Includes =======================================================*/

#include "fstd_types.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/
/** defines which action is performed when #CANTX_ImdRequest is called */
typedef enum {
    CANTX_IMD_REQUEST_OPEN_POSITIVE_RELAY,
    CANTX_IMD_REQUEST_OPEN_NEGATIVE_RELAY,
    CANTX_IMD_REQUEST_CLOSE_POSITIVE_RELAY,
    CANTX_IMD_REQUEST_CLOSE_NEGATIVE_RELAY,
    CANTX_IMD_REQUEST_POSITIVE_RELAY_STATE,
    CANTX_IMD_REQUEST_NEGATIVE_RELAY_STATE,
    CANTX_IMD_REQUEST_ENABLE_MEASUREMENT,
    CANTX_IMD_REQUEST_DISABLE_MEASUREMENT,
    CANTX_IMD_REQUEST_SET_AVERAGING_FACTOR,
    CANTX_IMD_REQUEST_READ_RESISTANCE,
    CANTX_IMD_REQUEST_INITIALIZATION_UNLOCK,
    CANTX_IMD_REQUEST_INITIALIZATION_SELF_TEST,
    CANTX_IMD_REQUEST_INITIALIZATION_SET_ERROR_THRESHOLD,
    CANTX_IMD_REQUEST_INITIALIZATION_SET_WARNING_THRESHOLD,
    CANTX_IMD_REQUEST_LAST_ACTION, /* always the last action, do not remove */
} CANTX_IMD_REQUEST_ACTIONS_e;

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/

/**
 * @brief   Handles IMD request message
 * @param   action  type of information that should be sent
 * @return  #STD_OK if transmission successful, otherwise #STD_NOT_OK
 */
extern STD_RETURN_TYPE_e CANTX_ImdRequest(CANTX_IMD_REQUEST_ACTIONS_e action);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
extern STD_RETURN_TYPE_e TEST_CANTX_RequestRelayOpen(uint8_t relay);
extern STD_RETURN_TYPE_e TEST_CANTX_RequestRelayClose(uint8_t relay);
extern STD_RETURN_TYPE_e TEST_CANTX_RequestRelayState(uint8_t relay);
#endif

#endif /* FOXBMS__CAN_CBS_TX_IMD_REQUEST_H_ */
