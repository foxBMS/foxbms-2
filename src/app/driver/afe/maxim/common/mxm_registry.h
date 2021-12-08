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
 * @file    mxm_registry.h
 * @author  foxBMS Team
 * @date    2020-07-16 (date of creation)
 * @updated 2021-12-06 (date of last update)
 * @ingroup DRIVERS
 * @prefix  MXM
 *
 * @brief   Functions in order to have a registry of monitoring ICs
 *
 * @details Monitoring registry stores information about the connected ICs.
 *
 */

#ifndef FOXBMS__MXM_REGISTRY_H_
#define FOXBMS__MXM_REGISTRY_H_

/*========== Includes =======================================================*/
#include "mxm_cfg.h"

#include "mxm_1785x_tools.h"
#include "mxm_basic_defines.h"

/*========== Macros and Definitions =========================================*/

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/
/**
 * @brief       Initialize monitoring registry
 * @details     Sets the connected state of every entry of the registry of
 *              monitoring ICs to false.
 * @param[in]   pState  state-pointer
 */
extern void MXM_MonRegistryInit(MXM_MONITORING_INSTANCE_s *pState);

/**
 * @brief           Mark devices as connected in the registry and set the
 *                  address.
 * @details         Takes the number of connected (adjacent!) devices and marks
 *                  them in the registry.
 * @param[in,out]   pState              state-pointer
 * @param[in]       numberOfDevices     number of normal satellites
 * @return          #STD_OK if the number of modules is OK, #STD_NOT_OK
 *                  otherwise
 */
extern STD_RETURN_TYPE_e MXM_MonRegistryConnectDevices(MXM_MONITORING_INSTANCE_s *pState, uint8_t numberOfDevices);

/**
 * @brief       Parse number of highest connected device from monitoring-
 *              register.
 * @param[in]   kpkState  state-pointer
 * @return      #MXM_MONITORING_INSTANCE_s::highest5xDevice
 */
extern uint8_t MXM_MonRegistryGetHighestConnected5XDevice(const MXM_MONITORING_INSTANCE_s *const kpkState);

/**
 * @brief           Parse ID (1 or 2) into the registry
 * @param[in,out]   pState          state-pointer
 * @param[in]       rxBufferLength  length of the rxBuffer
 * @param[in]       type            type of register (has to be #MXM_REG_ID1 or #MXM_REG_ID2)
 */
extern void MXM_MonRegistryParseIdIntoDevices(
    MXM_MONITORING_INSTANCE_s *pState,
    uint8_t rxBufferLength,
    MXM_REG_NAME_e type);

/**
 * @brief           Parse Version into the registry
 * @param[in,out]   pState          state-pointer
 * @param[in]       rxBufferLength  length of the rxBuffer
 */
extern void MXM_MonRegistryParseVersionIntoDevices(MXM_MONITORING_INSTANCE_s *pState, uint8_t rxBufferLength);

/**
 * @brief           Parse STATUS or FMEA into the registry
 * @param[in,out]   pState          state-pointer
 * @param[in]       rxBufferLength  length of the rxBuffer
 */
extern void MXM_MonRegistryParseStatusFmeaIntoDevices(MXM_MONITORING_INSTANCE_s *pState, uint8_t rxBufferLength);

/**
 * @brief       check if one of the devices in the registry has the ALRTRST bit set
 * @details     If a monitoring device is reset, it will set the ALRTRST bit in
 *              #MXM_REG_STATUS1. This should not happen during normal operation.
 *              If this happens, initialization of the devices is lost and has
 *              to be restored. This is done by reseting the state machine.
 * @param[in,out]   kpkState    state-pointer
 * @returns true if a device has been reset, false otherwise
 */
extern bool MXM_CheckIfADeviceHasBeenReset(const MXM_MONITORING_INSTANCE_s *const kpkState);

/**
 * @brief       check if a device is connected
 * @details     Checks the connected state in the registry
 * @param[in]   kpkState    state-pointer
 * @param[in]   device      device number
 * @returns true if a device is connected, false otherwise
 */
extern bool MXM_CheckIfADeviceIsConnected(const MXM_MONITORING_INSTANCE_s *const kpkState, uint8_t device);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/

#endif /* FOXBMS__MXM_REGISTRY_H_ */
