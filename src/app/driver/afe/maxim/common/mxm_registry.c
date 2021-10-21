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
 * @file    mxm_registry.c
 * @author  foxBMS Team
 * @date    2020-07-16 (date of creation)
 * @updated 2021-06-16 (date of last update)
 * @ingroup DRIVERS
 * @prefix  MXM
 *
 * @brief   Functions in order to have a registry of monitoring ICs
 *
 * @details Monitoring registry stores information about the connected ICs.
 *
 */

/*========== Includes =======================================================*/
#include "mxm_registry.h"

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/
extern void MXM_MonRegistryInit(MXM_MONITORING_INSTANCE_s *pState) {
    FAS_ASSERT(pState != NULL_PTR);
    for (uint8_t i = 0; i < MXM_MAXIMUM_NR_OF_MODULES; i++) {
        MXM_REGISTRY_ENTRY_s *entry = &pState->registry[i];
        entry->connected            = false;
        entry->deviceAddress        = 0u;
        entry->deviceID             = 0u;
        entry->model                = MXM_MODEL_ID_NONE;
        entry->siliconVersion       = MXM_siliconVersion_0;
    }
}

extern STD_RETURN_TYPE_e MXM_MonRegistryConnectDevices(MXM_MONITORING_INSTANCE_s *pState, uint8_t numberOfDevices) {
    FAS_ASSERT(pState != NULL_PTR);
    STD_RETURN_TYPE_e retval = STD_OK;
    if (numberOfDevices > MXM_MAXIMUM_NR_OF_MODULES) {
        retval = STD_NOT_OK;
    } else {
        for (uint8_t i = 0; i < numberOfDevices; i++) {
            pState->registry[i].connected     = true;
            pState->registry[i].deviceAddress = HELLOALL_START_SEED + i;
            pState->highest5xDevice           = HELLOALL_START_SEED + i;
        }
    }

    return retval;
}

extern uint8_t MXM_MonRegistryGetHighestConnected5XDevice(const MXM_MONITORING_INSTANCE_s *const kpkState) {
    FAS_ASSERT(kpkState != NULL_PTR);
    /* return highest connected device */
    return kpkState->highest5xDevice;
}

extern void MXM_MonRegistryParseIdIntoDevices(
    MXM_MONITORING_INSTANCE_s *pState,
    uint8_t rxBufferLength,
    MXM_REG_NAME_e type) {
    FAS_ASSERT(pState != NULL_PTR);
    /* only ID1 or ID2 are valid */
    FAS_ASSERT((type == MXM_REG_ID1) || (type == MXM_REG_ID2));

    /* find highest connected device */
    uint8_t highestConnectedDevice = MXM_MonRegistryGetHighestConnected5XDevice(pState);

    const uint8_t startBytes = 2u;

    for (uint8_t i = 0; i <= highestConnectedDevice; i++) {
        uint8_t bufferPosition              = startBytes + (i * 2u);
        MXM_REGISTRY_ENTRY_s *currentDevice = &pState->registry[highestConnectedDevice - i];
        FAS_ASSERT((bufferPosition + 1u) <= rxBufferLength);
        uint16_t id = 0;
        MXM_ExtractValueFromRegister(
            pState->rxBuffer[bufferPosition], pState->rxBuffer[bufferPosition + 1u], MXM_BM_WHOLE_REG, &id);
        if (type == MXM_REG_ID1) {
            currentDevice->deviceID = id;
        } else {
            /* intended condition: (type == MXM_REG_ID2) */
            currentDevice->deviceID = ((uint32_t)id << 16u) | currentDevice->deviceID;
        }
    }
}

extern void MXM_MonRegistryParseVersionIntoDevices(MXM_MONITORING_INSTANCE_s *pState, uint8_t rxBufferLength) {
    FAS_ASSERT(pState != NULL_PTR);

    /* find highest connected device */
    uint8_t highestConnectedDevice = MXM_MonRegistryGetHighestConnected5XDevice(pState);

    for (uint8_t i = 0; i <= highestConnectedDevice; i++) {
        const uint8_t startBytes            = 2u;
        uint8_t bufferPosition              = startBytes + (i * 2u);
        MXM_REGISTRY_ENTRY_s *currentDevice = &pState->registry[highestConnectedDevice - i];
        FAS_ASSERT((bufferPosition + 1u) <= rxBufferLength);
        uint16_t model = 0;
        MXM_ExtractValueFromRegister(
            pState->rxBuffer[bufferPosition], pState->rxBuffer[bufferPosition + 1u], MXM_REG_VERSION_MOD, &model);
        currentDevice->model = (MXM_MODEL_ID_e)model;

        uint16_t version = 0;
        MXM_ExtractValueFromRegister(
            pState->rxBuffer[bufferPosition], pState->rxBuffer[bufferPosition + 1u], MXM_REG_VERSION_VER, &version);
        currentDevice->siliconVersion = (MXM_siliconVersion_e)version;
    }
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
