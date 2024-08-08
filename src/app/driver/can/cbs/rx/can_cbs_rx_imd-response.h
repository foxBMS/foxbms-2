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
 * @file    can_cbs_rx_imd-response.h
 * @author  foxBMS Team
 * @date    2023-11-02 (date of creation)
 * @updated 2024-08-08 (date of last update)
 * @version v1.7.0
 * @ingroup DRIVERS
 * @prefix  CANRX
 *
 * @brief   Declarations for handling the extraction of imd response message
 * @details This header declares the interface for reading imd response
 *          messages and everything else that is externally required to be able
 *          to use that interface.
 */

#ifndef FOXBMS__CAN_CBS_RX_IMD_RESPONSE_H_
#define FOXBMS__CAN_CBS_RX_IMD_RESPONSE_H_

/*========== Includes =======================================================*/

#include "can_cfg.h"
#include "database_cfg.h"

#include "fstd_types.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/
/**
 * @brief   Gets insulation resistance from IMD response message and stores it in the data table
 * @param   kpkCanData Can data from the IMD Info message
 * @param   pTableInsulationMonitoring Pointer to the data table where data will be written
 */
void CANRX_ImdResponseReadInsulationResistance(
    const uint8_t *const kpkCanData,
    DATA_BLOCK_INSULATION_MONITORING_s *pTableInsulationMonitoring);

/**
 * @brief   Gets tendency of the insulation fault from IMD response message and stores it in the data table
 * @param   kpkCanData Can data from the IMD Info message
 * @param   pTableInsulationMonitoring Pointer to the data table where data will be written
 */
void CANRX_ImdResponseCheckInsulationFaultTendency(
    const uint8_t *const kpkCanData,
    DATA_BLOCK_INSULATION_MONITORING_s *pTableInsulationMonitoring);

/**
 * @brief   Checks the current relay state of one relay
 * @param   kpkCanData Can data from the IMD Info message
 * @param   relay Relay that the actual value is compared to
 * @param   relayState Relay state that the actual value is compared to
 * @return  True if relay state from can data and relay are equal
 */
bool CANRX_ImdResponseCheckRelayState(const uint8_t *const kpkCanData, uint8_t relay, uint8_t relayState);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
extern void TEST_CANRX_TransferImdResponseMessageToCanBuffer(
    uint8_t messageDlc,
    const uint8_t *const kpkCanData,
    CAN_BUFFER_ELEMENT_s *canBuffer);
#endif

#endif /* FOXBMS__CAN_CBS_RX_IMD_RESPONSE_H_ */
