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
 * @file    can_cbs_tx_debug-build-configuration.h
 * @author  foxBMS Team
 * @date    2022-11-16 (date of creation)
 * @updated 2024-08-08 (date of last update)
 * @version v1.7.0
 * @ingroup DRIVERS
 * @prefix  CANTX
 *
 * @brief   CAN header
 * @details CAN Tx callback for unsupported requests
 */

#ifndef FOXBMS__CAN_CBS_TX_DEBUG_BUILD_CONFIGURATION_H_
#define FOXBMS__CAN_CBS_TX_DEBUG_BUILD_CONFIGURATION_H_

/*========== Includes =======================================================*/

#include "can_cfg.h"

/*========== Macros and Definitions =========================================*/

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/
/**
 * @brief   Sends the configuration of the software build
 */
extern STD_RETURN_TYPE_e CANTX_DebugBuildConfiguration(void);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST

extern void TEST_CANTX_SetDebugBuildConfigurationMux(uint64_t *pMessageData, uint8_t muxValue);

extern STD_RETURN_TYPE_e TEST_CANTX_SendApplicationConfiguration(void);

extern STD_RETURN_TYPE_e TEST_CANTX_SendSlaveConfiguration(void);

extern STD_RETURN_TYPE_e TEST_CANTX_SendBatteryCellConfiguration(void);

extern void TEST_CANTX_SetBatteryCellMuxMessageData(uint64_t *pMessageData);

extern void TEST_CANTX_SetBatteryCellMuxMaxChargeCurrentMessageData(uint64_t *pMessageData);

extern void TEST_CANTX_SetBatteryCellMuxMaxChargeTemperatureMessageData(uint64_t *pMessageData);

extern void TEST_CANTX_SetBatteryCellMuxMaxDischargeCurrentMessageData(uint64_t *pMessageData);

extern void TEST_CANTX_SetBatteryCellMuxMaxDischargeTemperatureMessageData(uint64_t *pMessageData);

extern void TEST_CANTX_SetBatteryCellMuxMaxVoltageMessageData(uint64_t *pMessageData);

extern void TEST_CANTX_SetBatteryCellMuxMinChargeTemperatureMessageData(uint64_t *pMessageData);

extern void TEST_CANTX_SetBatteryCellMuxMinDischargeTemperatureMessageData(uint64_t *pMessageData);

extern void TEST_CANTX_SetBatteryCellMuxMinVoltageMessageData(uint64_t *pMessageData);

extern STD_RETURN_TYPE_e TEST_CANTX_SendBatterySystemConfiguration(void);

extern void TEST_CANTX_SetBatterySystemMuxGeneral1MessageData(uint64_t *pMessageData);

extern void TEST_CANTX_SetBatterySystemMuxGeneral2MessageData(uint64_t *pMessageData);

extern void TEST_CANTX_SetBatterySystemMuxContactorsMessageData(uint64_t *pMessageData);

extern void TEST_CANTX_SetBatterySystemMuxCurrentSensorMessageData(uint64_t *pMessageData);

extern void TEST_CANTX_SetBatterySystemMuxFuseMessageData(uint64_t *pMessageData);

extern void TEST_CANTX_SetBatterySystemMuxMaxCurrentMessageData(uint64_t *pMessageData);

extern void TEST_CANTX_SetBatterySystemMuxOpenWireCheckMessageData(uint64_t *pMessageData);

extern void TEST_CANTX_SetBatterySystemMuxTotalNumbersMessageData(uint64_t *pMessageData);
#endif

#endif /* FOXBMS__CAN_CBS_TX_DEBUG_BUILD_CONFIGURATION_H_ */
