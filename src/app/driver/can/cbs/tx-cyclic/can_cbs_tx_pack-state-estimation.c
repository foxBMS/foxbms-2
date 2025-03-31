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
 * @file    can_cbs_tx_pack-state-estimation.c
 * @author  foxBMS Team
 * @date    2021-07-21 (date of creation)
 * @updated 2025-03-31 (date of last update)
 * @version v1.9.0
 * @ingroup DRIVERS
 * @prefix  CANTX
 *
 * @brief   CAN driver Tx callback implementation
 * @details CAN Tx callback for state estimation messages
 */

/*========== Includes =======================================================*/
#include "bms.h"
/* AXIVION Next Codeline Generic-LocalInclude: 'can_cbs_tx_cyclic.h' declares
 * the prototype for the callback 'CANTX_PackStateEstimation' */
#include "can_cbs_tx_cyclic.h"
#include "can_cfg_tx-cyclic-message-definitions.h"
#include "can_helper.h"
#include "foxmath.h"

#include <float.h>
#include <math.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/
#define CANTX_100_PERCENT_FLOAT (100.0f)

#define CANTX_SIGNAL_MINIMUM_SOC_START_BIT (7u)
#define CANTX_SIGNAL_MINIMUM_SOC_LENGTH    (10u)
#define CANTX_SIGNAL_MAXIMUM_SOC_START_BIT (13u)
#define CANTX_SIGNAL_MAXIMUM_SOC_LENGTH    (10u)
#define CANTX_SIGNAL_MINIMUM_SOE_START_BIT (19u)
#define CANTX_SIGNAL_MINIMUM_SOE_LENGTH    (10u)
#define CANTX_SIGNAL_MAXIMUM_SOE_START_BIT (25u)
#define CANTX_SIGNAL_MAXIMUM_SOE_LENGTH    (10u)
#define CANTX_SIGNAL_SOH_START_BIT         (47u)
#define CANTX_SIGNAL_SOH_LENGTH            (8u)
#define CANTX_SIGNAL_ENERGY_START_BIT      (55u)
#define CANTX_SIGNAL_ENERGY_LENGTH         (16u)

#define CANTX_MINIMUM_VALUE_PERCENT_SIGNALS (0.0f)
#define CANTX_MAXIMUM_VALUE_PERCENT_SIGNALS (102.3f)
#define CANTX_MINIMUM_ENERGY_VALUE          (0.0f)
#define CANTX_MAXIMUM_ENERGY_VALUE          (6553500.0f)
#define CANTX_FACTOR_SOH                    (0.5f)
#define CANTX_MAXIMUM_VALUE_SOH_SIGNAL      (127.5f)

/** @{
 * configuration of the minimum soc signal
*/
static const CAN_SIGNAL_TYPE_s cantx_signalMinimumSoc = {
    CANTX_SIGNAL_MINIMUM_SOC_START_BIT,
    CANTX_SIGNAL_MINIMUM_SOC_LENGTH,
    UNIT_CONVERSION_FACTOR_1_10_TH_FLOAT,
    CAN_SIGNAL_OFFSET_0,
    CANTX_MINIMUM_VALUE_PERCENT_SIGNALS,
    CANTX_MAXIMUM_VALUE_PERCENT_SIGNALS};
/** @} */

/** @{
 * configuration of the maximum soc signal
*/
static const CAN_SIGNAL_TYPE_s cantx_signalMaximumSoc = {
    CANTX_SIGNAL_MAXIMUM_SOC_START_BIT,
    CANTX_SIGNAL_MAXIMUM_SOC_LENGTH,
    UNIT_CONVERSION_FACTOR_1_10_TH_FLOAT,
    CAN_SIGNAL_OFFSET_0,
    CANTX_MINIMUM_VALUE_PERCENT_SIGNALS,
    CANTX_MAXIMUM_VALUE_PERCENT_SIGNALS};
/** @} */

/** @{
 * configuration of the minimum soe signal
*/
static const CAN_SIGNAL_TYPE_s cantx_signalMinimumSoe = {
    CANTX_SIGNAL_MINIMUM_SOE_START_BIT,
    CANTX_SIGNAL_MINIMUM_SOC_LENGTH,
    UNIT_CONVERSION_FACTOR_1_10_TH_FLOAT,
    CAN_SIGNAL_OFFSET_0,
    CANTX_MINIMUM_VALUE_PERCENT_SIGNALS,
    CANTX_MAXIMUM_VALUE_PERCENT_SIGNALS};
/** @} */

/** @{
 * configuration of the maximum soe signal
*/
static const CAN_SIGNAL_TYPE_s cantx_signalMaximumSoe = {
    CANTX_SIGNAL_MAXIMUM_SOE_START_BIT,
    CANTX_SIGNAL_MAXIMUM_SOE_LENGTH,
    UNIT_CONVERSION_FACTOR_1_10_TH_FLOAT,
    CAN_SIGNAL_OFFSET_0,
    CANTX_MINIMUM_VALUE_PERCENT_SIGNALS,
    CANTX_MAXIMUM_VALUE_PERCENT_SIGNALS};
/** @} */

/** @{
 * configuration of the soh signal
*/
static const CAN_SIGNAL_TYPE_s cantx_signalSoh = {
    CANTX_SIGNAL_SOH_START_BIT,
    CANTX_SIGNAL_SOH_LENGTH,
    CANTX_FACTOR_SOH,
    CAN_SIGNAL_OFFSET_0,
    CANTX_MINIMUM_VALUE_PERCENT_SIGNALS,
    CANTX_MAXIMUM_VALUE_SOH_SIGNAL};
/** @} */

/** @{
 * configuration of the energy signal
*/
static const CAN_SIGNAL_TYPE_s cantx_signalEnergy = {
    CANTX_SIGNAL_ENERGY_START_BIT,
    CANTX_SIGNAL_ENERGY_LENGTH,
    UNIT_CONVERSION_FACTOR_100_FLOAT,
    CAN_SIGNAL_OFFSET_0,
    CANTX_MINIMUM_ENERGY_VALUE,
    CANTX_MAXIMUM_ENERGY_VALUE};
/** @} */

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/
/**
 * @brief Calculates the return value of the maximum SOC
 * @return Returns the return value of the maximum SOC
 */
static uint64_t CANTX_CalculateMaximumPackSoc(const CAN_SHIM_s *const kpkCanShim);

/**
 * @brief Gets the highest string SOC percentage from all connected Strings
 * @return Returns the highest string SOC percentage from all connected Strings
 */
static float_t CANTX_GetMaximumStringSoc(const CAN_SHIM_s *const kpkCanShim);

/**
 * @brief Calculates the return value of the minimum SOC
 * @return Returns the return value of the minimum SOC
 */
static uint64_t CANTX_CalculateMinimumPackSoc(const CAN_SHIM_s *const kpkCanShim);

/**
 * @brief Gets the lowest string SOC percentage from all connected Strings
 * @return Returns the lowest string SOC percentage from all connected Strings
 */
static float_t CANTX_GetMinimumStringSoc(const CAN_SHIM_s *const kpkCanShim);

/**
 * @brief Calculates the return value of the maximum SOE
 * @return Returns the return value of the maximum SOE
 */
static uint64_t CANTX_CalculateMaximumPackSoe(const CAN_SHIM_s *const kpkCanShim);

/**
 * @brief Gets the highest string SOE percentage from all connected Strings
 * @return Returns the highest string SOE percentage from all connected Strings
 */
static float_t CANTX_GetMaximumStringSoe(const CAN_SHIM_s *const kpkCanShim);

/**
 * @brief Calculates the return value of the minimum SOE
 * @return Returns the return value of the minimum SOE
 */
static uint64_t CANTX_CalculateMinimumPackSoe(const CAN_SHIM_s *const kpkCanShim);

/**
 * @brief Gets the lowest string SOE percentage from all connected Strings
 * @return Returns the lowest string SOE percentage from all connected Strings
 */
static float_t CANTX_GetMinimumStringSoe(const CAN_SHIM_s *const kpkCanShim);

/**
 * @brief Calculates the return value of the SOH
 * @return Returns the return value of the SOH
 */
static uint64_t CANTX_CalculatePackSoh(void);

/**
 * @brief Calculates the return value of the stored Energy
 * @return Returns the return value of the stored Energy
 */
static uint64_t CANTX_CalculatePackEnergy(const CAN_SHIM_s *const kpkCanShim);

/**
 * @brief Gets the lowest energy value in Wh from all connected Strings
 * @return Returns the lowest energy value in Wh from all connected Strings
 */
static float_t CANTX_GetStringEnergy(const CAN_SHIM_s *const kpkCanShim);

/**
 * @brief Builds the CAN message form signal data
 */
static void CANTX_BuildPackStateEstimationMessage(const CAN_SHIM_s *const kpkCanShim, uint64_t *pMessageData);

/*========== Static Function Implementations ================================*/
static uint64_t CANTX_CalculateMaximumPackSoc(const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(kpkCanShim != NULL_PTR);

    /* Get maximum SOC percentage of the strings and calculate pack value */
    float_t signalData = 0.0f;
    if (BMS_GetNumberOfConnectedStrings() != 0u) {
        signalData = ((float_t)BMS_GetNumberOfConnectedStrings() * CANTX_GetMaximumStringSoc(kpkCanShim)) /
                     (float_t)BS_NR_OF_STRINGS;
    }

    CAN_TxPrepareSignalData(&signalData, cantx_signalMaximumSoc);
    uint64_t data = (uint64_t)signalData;
    return data;
}

static float_t CANTX_GetMaximumStringSoc(const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(kpkCanShim != NULL_PTR);

    float_t maximumStringSoc_perc = 0.0f;
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        if (BMS_IsStringClosed(s) == true) {
            if (maximumStringSoc_perc < kpkCanShim->pTableSoc->maximumSoc_perc[s]) {
                maximumStringSoc_perc = kpkCanShim->pTableSoc->maximumSoc_perc[s];
            }
        }
    }
    return maximumStringSoc_perc;
}

static uint64_t CANTX_CalculateMinimumPackSoc(const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(kpkCanShim != NULL_PTR);

    /* Get minimum SOC percentage of the strings and calculate pack value */
    float_t signalData = 0.0f;
    if (BMS_GetNumberOfConnectedStrings() != 0u) {
        signalData = (BMS_GetNumberOfConnectedStrings() * CANTX_GetMinimumStringSoc(kpkCanShim)) / BS_NR_OF_STRINGS;
    }

    CAN_TxPrepareSignalData(&signalData, cantx_signalMinimumSoc);
    uint64_t data = (uint64_t)signalData;
    return data;
}

static float_t CANTX_GetMinimumStringSoc(const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(kpkCanShim != NULL_PTR);

    float_t minimumStringSoc_perc = CANTX_100_PERCENT_FLOAT;
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        if (BMS_IsStringClosed(s) == true) {
            if (minimumStringSoc_perc > kpkCanShim->pTableSoc->minimumSoc_perc[s]) {
                minimumStringSoc_perc = kpkCanShim->pTableSoc->minimumSoc_perc[s];
            }
        }
    }
    return minimumStringSoc_perc;
}

static uint64_t CANTX_CalculateMaximumPackSoe(const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(kpkCanShim != NULL_PTR);

    /* Get maximum SOE percentage of the strings and calculate pack value */
    float_t signalData = 0.0f;
    if (BMS_GetNumberOfConnectedStrings() != 0u) {
        signalData = (BMS_GetNumberOfConnectedStrings() * CANTX_GetMaximumStringSoe(kpkCanShim)) / BS_NR_OF_STRINGS;
    }

    CAN_TxPrepareSignalData(&signalData, cantx_signalMaximumSoe);
    uint64_t data = (uint64_t)signalData;
    return data;
}

static float_t CANTX_GetMaximumStringSoe(const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(kpkCanShim != NULL_PTR);

    float_t maximumStringSoe_perc = 0.0f;
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        if (BMS_IsStringClosed(s) == true) {
            if (maximumStringSoe_perc < kpkCanShim->pTableSoe->maximumSoe_perc[s]) {
                maximumStringSoe_perc = kpkCanShim->pTableSoe->maximumSoe_perc[s];
            }
        }
    }
    return maximumStringSoe_perc;
}

static uint64_t CANTX_CalculateMinimumPackSoe(const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(kpkCanShim != NULL_PTR);

    /* Get minimum SOE percentage of the strings and calculate pack value */
    float_t signalData = 0.0f;
    if (BMS_GetNumberOfConnectedStrings() != 0u) {
        signalData = (BMS_GetNumberOfConnectedStrings() * CANTX_GetMinimumStringSoe(kpkCanShim)) / BS_NR_OF_STRINGS;
    }

    CAN_TxPrepareSignalData(&signalData, cantx_signalMinimumSoe);
    uint64_t data = (uint64_t)signalData;
    return data;
}

static float_t CANTX_GetMinimumStringSoe(const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(kpkCanShim != NULL_PTR);

    float_t minimumStringSoe_perc = CANTX_100_PERCENT_FLOAT;
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        if (BMS_IsStringClosed(s) == true) {
            if (minimumStringSoe_perc > kpkCanShim->pTableSoe->minimumSoe_perc[s]) {
                minimumStringSoe_perc = kpkCanShim->pTableSoe->minimumSoe_perc[s];
            }
        }
    }
    return minimumStringSoe_perc;
}

static uint64_t CANTX_CalculatePackSoh(void) {
    float_t signalData = CANTX_100_PERCENT_FLOAT; /* TODO */
    CAN_TxPrepareSignalData(&signalData, cantx_signalSoh);
    uint64_t data = (uint64_t)signalData;
    return data;
}

static uint64_t CANTX_CalculatePackEnergy(const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(kpkCanShim != NULL_PTR);

    /* Get energy in Wh of the strings and calculate pack value */
    float_t signalData = 0.0f;
    if (BMS_GetNumberOfConnectedStrings() != 0u) {
        signalData = BMS_GetNumberOfConnectedStrings() * CANTX_GetStringEnergy(kpkCanShim);
    }

    CAN_TxPrepareSignalData(&signalData, cantx_signalEnergy);
    uint64_t data = (uint64_t)signalData;
    return data;
}

static float_t CANTX_GetStringEnergy(const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(kpkCanShim != NULL_PTR);

    float_t minimumStringEnergy_Wh = kpkCanShim->pTableSoe->minimumSoe_Wh[0u];
    for (uint8_t s = 1u; s < BS_NR_OF_STRINGS; s++) {
        if (minimumStringEnergy_Wh > kpkCanShim->pTableSoe->minimumSoe_Wh[s]) {
            minimumStringEnergy_Wh = kpkCanShim->pTableSoe->minimumSoe_Wh[s];
        }
    }
    return minimumStringEnergy_Wh;
}

static void CANTX_BuildPackStateEstimationMessage(const CAN_SHIM_s *const kpkCanShim, uint64_t *pMessageData) {
    FAS_ASSERT(kpkCanShim != NULL_PTR);
    FAS_ASSERT(pMessageData != NULL_PTR);

    /* minimum SOC */
    uint64_t data = CANTX_CalculateMinimumPackSoc(kpkCanShim);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData, cantx_signalMinimumSoc.bitStart, cantx_signalMinimumSoc.bitLength, data, CAN_BIG_ENDIAN);
    /* maximum SOC */
    data = CANTX_CalculateMaximumPackSoc(kpkCanShim);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData, cantx_signalMaximumSoc.bitStart, cantx_signalMaximumSoc.bitLength, data, CAN_BIG_ENDIAN);
    /* minimum SOE*/
    data = CANTX_CalculateMinimumPackSoe(kpkCanShim);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData, cantx_signalMinimumSoe.bitStart, cantx_signalMinimumSoe.bitLength, data, CAN_BIG_ENDIAN);
    /* maximum SOE */
    data = CANTX_CalculateMaximumPackSoe(kpkCanShim);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData, cantx_signalMaximumSoe.bitStart, cantx_signalMaximumSoe.bitLength, data, CAN_BIG_ENDIAN);
    /* SOH */
    data = CANTX_CalculatePackSoh();
    CAN_TxSetMessageDataWithSignalData(
        pMessageData, cantx_signalSoh.bitStart, cantx_signalSoh.bitLength, data, CAN_BIG_ENDIAN);
    /* Pack energy */
    data = CANTX_CalculatePackEnergy(kpkCanShim);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData, cantx_signalEnergy.bitStart, cantx_signalEnergy.bitLength, data, CAN_BIG_ENDIAN);
}

/*========== Extern Function Implementations ================================*/
extern uint32_t CANTX_PackStateEstimation(
    CAN_MESSAGE_PROPERTIES_s message,
    uint8_t *pCanData,
    uint8_t *pMuxId,
    const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(message.id == CANTX_PACK_STATE_ESTIMATION_ID);
    FAS_ASSERT(message.idType == CANTX_PACK_STATE_ESTIMATION_ID_TYPE);
    FAS_ASSERT(message.dlc <= CAN_MAX_DLC);
    FAS_ASSERT(message.endianness == CANTX_PACK_STATE_ESTIMATION_ENDIANNESS);
    FAS_ASSERT(pCanData != NULL_PTR);
    FAS_ASSERT(pMuxId == NULL_PTR); /* pMuxId is not used here, therefore has to be NULL_PTR */
    FAS_ASSERT(kpkCanShim != NULL_PTR);
    uint64_t messageData = 0u;

    DATA_READ_DATA(kpkCanShim->pTableSoc, kpkCanShim->pTableSoe);

    /* build CAN message */
    CANTX_BuildPackStateEstimationMessage(kpkCanShim, &messageData);
    /* now copy data in the buffer that will be used to send data */
    CAN_TxSetCanDataWithMessageData(messageData, pCanData, message.endianness);

    return 0u;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
extern uint64_t TEST_CANTX_CalculateMaximumPackSoc(const CAN_SHIM_s *const kpkCanShim) {
    return CANTX_CalculateMaximumPackSoc(kpkCanShim);
}
extern float_t TEST_CANTX_GetMaximumStringSoc(const CAN_SHIM_s *const kpkCanShim) {
    return CANTX_GetMaximumStringSoc(kpkCanShim);
}
extern uint64_t TEST_CANTX_CalculateMinimumPackSoc(const CAN_SHIM_s *const kpkCanShim) {
    return CANTX_CalculateMinimumPackSoc(kpkCanShim);
}
extern float_t TEST_CANTX_GetMinimumStringSoc(const CAN_SHIM_s *const kpkCanShim) {
    return CANTX_GetMinimumStringSoc(kpkCanShim);
}
extern uint64_t TEST_CANTX_CalculateMaximumPackSoe(const CAN_SHIM_s *const kpkCanShim) {
    return CANTX_CalculateMaximumPackSoe(kpkCanShim);
}
extern float_t TEST_CANTX_GetMaximumStringSoe(const CAN_SHIM_s *const kpkCanShim) {
    return CANTX_GetMaximumStringSoe(kpkCanShim);
}
extern uint64_t TEST_CANTX_CalculateMinimumPackSoe(const CAN_SHIM_s *const kpkCanShim) {
    return CANTX_CalculateMinimumPackSoe(kpkCanShim);
}
extern float_t TEST_CANTX_GetMinimumStringSoe(const CAN_SHIM_s *const kpkCanShim) {
    return CANTX_GetMinimumStringSoe(kpkCanShim);
}
extern uint64_t TEST_CANTX_CalculatePackSoh(void) {
    return CANTX_CalculatePackSoh();
}
extern uint64_t TEST_CANTX_CalculatePackEnergy(const CAN_SHIM_s *const kpkCanShim) {
    return CANTX_CalculatePackEnergy(kpkCanShim);
}
extern float_t TEST_CANTX_GetStringEnergy(const CAN_SHIM_s *const kpkCanShim) {
    return CANTX_GetStringEnergy(kpkCanShim);
}
extern void TEST_CANTX_BuildPackStateEstimationMessage(const CAN_SHIM_s *const kpkCanShim, uint64_t *pMessageData) {
    CANTX_BuildPackStateEstimationMessage(kpkCanShim, pMessageData);
}

#endif
