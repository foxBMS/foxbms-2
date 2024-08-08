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
 * @file    can_cbs_rx_imd-info.h
 * @author  foxBMS Team
 * @date    2023-10-09 (date of creation)
 * @updated 2024-08-08 (date of last update)
 * @version v1.7.0
 * @ingroup DRIVERS
 * @prefix  CANRX
 *
 * @brief   Declarations for handling the extraction of imd info message
 * @details This header declares the interface for reading imd info
 *          messages and everything else that is externally required to be able
 *          to use that interface.
 */

#ifndef FOXBMS__CAN_CBS_RX_IMD_INFO_H_
#define FOXBMS__CAN_CBS_RX_IMD_INFO_H_

/*========== Includes =======================================================*/

#include "can_cfg.h"
#include "database_cfg.h"

#include "fstd_types.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/
/**
 * @brief   Gets data from IMD Info message and stores it in data table
 * @param   kpkCanData Can data from the IMD Info message
 * @param   pTableInsulationMonitoring Pointer to the data table where data will be written
 */
void CANRX_ImdInfoGetDataFromMessage(
    const uint8_t *const kpkCanData,
    DATA_BLOCK_INSULATION_MONITORING_s *pTableInsulationMonitoring);

/**
 * @brief   Checks the current measurement mode
 * @param   kpkCanData Can data from the IMD Info message
 * @param   mode Mode that the actual value is compared to
 * @return  True if mode from can data and mode are equal
 */
bool CANRX_ImdInfoCheckMeasurementMode(const uint8_t *const kpkCanData, uint8_t mode);

/**
 * @brief   Checks if self test has been executed
 * @param   kpkCanData Can data from the IMD Info message
 * @return  true if self test has been executed
 */
bool CANRX_ImdInfoHasSelfTestBeenExecuted(const uint8_t *const kpkCanData);

/**
 * @brief   Checks if self test has finished
 * @param   kpkCanData Can data from the IMD Info message
 * @return  true if self test has finished
 */
bool CANRX_ImdInfoIsSelfTestFinished(const uint8_t *const kpkCanData);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
extern void TEST_CANRX_TransferImdInfoMessageToCanBuffer(
    uint8_t messageDlc,
    const uint8_t *const kpkCanData,
    CAN_BUFFER_ELEMENT_s *canBuffer);
#endif

#endif /* FOXBMS__CAN_CBS_RX_IMD_INFO_H_ */
