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
 * @file    can_cbs_tx_pack-minimum-maximum-values.c
 * @author  foxBMS Team
 * @date    2021-04-20 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup DRIVERS
 * @prefix  CANTX
 *
 * @brief   CAN driver Tx callback implementation
 * @details CAN Tx callback for min/max values
 */

/*========== Includes =======================================================*/
#include "bms.h"
/* AXIVION Next Codeline Generic-LocalInclude: 'can_cbs_tx_cyclic.h' declares
 * the prototype for the callback 'CANTX_PackMinimumMaximumValues' */
#include "can_cbs_tx_cyclic.h"
#include "can_cfg_tx-cyclic-message-definitions.h"
#include "can_helper.h"
#include "foxmath.h"

#include <math.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/
/**
 * Configuration of the signals
 */
#define CANTX_SIGNAL_MAXIMUM_CELL_VOLTAGE_START_BIT       (7u)
#define CANTX_SIGNAL_MAXIMUM_CELL_VOLTAGE_LENGTH          (14u)
#define CANTX_SIGNAL_MINIMUM_CELL_VOLTAGE_START_BIT       (9u)
#define CANTX_SIGNAL_MINIMUM_CELL_VOLTAGE_LENGTH          (14u)
#define CANTX_SIGNAL_COOLING_INLET_TEMPERATURE_START_BIT  (39u)
#define CANTX_SIGNAL_COOLING_INLET_TEMPERATURE_LENGTH     (8u)
#define CANTX_SIGNAL_COOLING_OUTLET_TEMPERATURE_START_BIT (47u)
#define CANTX_SIGNAL_COOLING_OUTLET_TEMPERATURE_LENGTH    (8u)

/** @{
 * defines of the maximum cell temperature signal
*/
#define CANTX_SIGNAL_MAXIMUM_CELL_TEMPERATURE_START_BIT     (55u)
#define CANTX_SIGNAL_MAXIMUM_CELL_TEMPERATURE_LENGTH        (8u)
#define CANTX_MINIMUM_VALUE_MAXIMUM_CELL_TEMPERATURE_SIGNAL (-1280.0f)
#define CANTX_MAXIMUM_VALUE_MAXIMUM_CELL_TEMPERATURE_SIGNAL (1270.0f)
/** @} */

/** @{
 * configuration of the maximum cell temperature signal
*/
static const CAN_SIGNAL_TYPE_s cantx_signalMaximumCellTemperature = {
    CANTX_SIGNAL_MAXIMUM_CELL_TEMPERATURE_START_BIT,
    CANTX_SIGNAL_MAXIMUM_CELL_TEMPERATURE_LENGTH,
    UNIT_CONVERSION_FACTOR_10_FLOAT,
    CAN_SIGNAL_OFFSET_0,
    CANTX_MINIMUM_VALUE_MAXIMUM_CELL_TEMPERATURE_SIGNAL,
    CANTX_MAXIMUM_VALUE_MAXIMUM_CELL_TEMPERATURE_SIGNAL};
/** @} */

/** @{
 * defines of the minimum cell temperature signal
*/
#define CANTX_SIGNAL_MINIMUM_CELL_TEMPERATURE_START_BIT     (63u)
#define CANTX_SIGNAL_MINIMUM_CELL_TEMPERATURE_LENGTH        (8u)
#define CANTX_MINIMUM_VALUE_MINIMUM_CELL_TEMPERATURE_SIGNAL (-1280.0f)
#define CANTX_MAXIMUM_VALUE_MINIMUM_CELL_TEMPERATURE_SIGNAL (1270.0f)
/** @} */

/** @{
 * configuration of the minimum cell temperature signal
*/
static const CAN_SIGNAL_TYPE_s cantx_signalMinimumCellTemperature = {
    CANTX_SIGNAL_MINIMUM_CELL_TEMPERATURE_START_BIT,
    CANTX_SIGNAL_MINIMUM_CELL_TEMPERATURE_LENGTH,
    UNIT_CONVERSION_FACTOR_10_FLOAT,
    CAN_SIGNAL_OFFSET_0,
    CANTX_MINIMUM_VALUE_MINIMUM_CELL_TEMPERATURE_SIGNAL,
    CANTX_MAXIMUM_VALUE_MINIMUM_CELL_TEMPERATURE_SIGNAL};
/** @} */

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/
/**
 * @brief   Gets the value of the maximum pack voltage
 * @param   kpkCanShim const pointer to CAN shim
 * @return  Returns the maximum pack voltage from the database
 */
static int16_t CANTX_GetPackMaximumVoltage(const CAN_SHIM_s *const kpkCanShim);

/**
 * @brief   Gets the value of the minimum pack voltage
 * @param   kpkCanShim const pointer to CAN shim
 * @return  Returns the minimum pack voltage from the database
 */
static int16_t CANTX_GetPackMinimumVoltage(const CAN_SHIM_s *const kpkCanShim);

/**
 * @brief   Gets the value of the maximum pack temperature
 * @param   kpkCanShim const pointer to CAN shim
 * @return  Returns the maximum pack temperature from the database
 */
static int16_t CANTX_GetPackMaximumTemperature(const CAN_SHIM_s *const kpkCanShim);

/**
 * @brief   Gets the value of the minimum pack temperature
 * @param   kpkCanShim const pointer to CAN shim
 * @return  Returns the minimum pack temperature from the database
 */
static int16_t CANTX_GetPackMinimumTemperature(const CAN_SHIM_s *const kpkCanShim);

/**
 * @brief   Calculates the return value of the maximum pack temperature
 * @param   kpkCanShim const pointer to CAN shim
 * @return  Returns the return value of the maximum pack temperature
 */
static uint64_t CANTX_CalculatePackMaximumTemperature(const CAN_SHIM_s *const kpkCanShim);

/**
 * @brief   Calculates the return value of the minimum pack temperature
 * @param   kpkCanShim const pointer to CAN shim
 * @return  Returns the return value of the minimum pack temperature
 */
static uint64_t CANTX_CalculatePackMinimumTemperature(const CAN_SHIM_s *const kpkCanShim);

/**
 * @brief   Adds the data to the message about the pack values
 * @param   kpkCanShim const pointer to CAN shim
 * @param   pMessageData message data of the CAN message
 */
static void CANTX_BuildPackMinimumMaximumValuesMessage(const CAN_SHIM_s *const kpkCanShim, uint64_t *pMessageData);

/*========== Static Function Implementations ================================*/
static int16_t CANTX_GetPackMaximumVoltage(const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(kpkCanShim != NULL_PTR);

    int16_t packMaximumVoltage_mV = INT16_MIN;
    if (0u == BMS_GetNumberOfConnectedStrings()) {
        /* Calculate min/max values of complete pack if all slice switches are open */
        for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
            if (kpkCanShim->pTableMinMax->maximumCellVoltage_mV[s] >= packMaximumVoltage_mV) {
                packMaximumVoltage_mV = kpkCanShim->pTableMinMax->maximumCellVoltage_mV[s];
            }
        }
    } else {
        /* Calculate min/max values of connected slices */
        for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
            if (BMS_IsStringClosed(s) == true) {
                if (kpkCanShim->pTableMinMax->maximumCellVoltage_mV[s] >= packMaximumVoltage_mV) {
                    packMaximumVoltage_mV = kpkCanShim->pTableMinMax->maximumCellVoltage_mV[s];
                }
            }
        }
    }
    return packMaximumVoltage_mV;
}

static int16_t CANTX_GetPackMinimumVoltage(const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(kpkCanShim != NULL_PTR);

    int16_t packMinimumVoltage_mV = INT16_MAX;
    if (0u == BMS_GetNumberOfConnectedStrings()) {
        /* Calculate min/max values of complete pack if all slice switches are open */
        for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
            if (kpkCanShim->pTableMinMax->minimumCellVoltage_mV[s] <= packMinimumVoltage_mV) {
                packMinimumVoltage_mV = kpkCanShim->pTableMinMax->minimumCellVoltage_mV[s];
            }
        }
    } else {
        /* Calculate min/max values of connected slices */
        for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
            if (BMS_IsStringClosed(s) == true) {
                if (kpkCanShim->pTableMinMax->minimumCellVoltage_mV[s] <= packMinimumVoltage_mV) {
                    packMinimumVoltage_mV = kpkCanShim->pTableMinMax->minimumCellVoltage_mV[s];
                }
            }
        }
    }
    return packMinimumVoltage_mV;
}

static int16_t CANTX_GetPackMaximumTemperature(const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(kpkCanShim != NULL_PTR);

    int16_t packMaximumTemperature_ddegC = INT16_MIN;
    if (0u == BMS_GetNumberOfConnectedStrings()) {
        /* Calculate min/max values of complete pack if all slice switches are open */
        for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
            if (kpkCanShim->pTableMinMax->maximumTemperature_ddegC[s] >= packMaximumTemperature_ddegC) {
                packMaximumTemperature_ddegC = kpkCanShim->pTableMinMax->maximumTemperature_ddegC[s];
            }
        }
    } else {
        /* Calculate min/max values of connected slices */
        for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
            if (BMS_IsStringClosed(s) == true) {
                if (kpkCanShim->pTableMinMax->maximumTemperature_ddegC[s] >= packMaximumTemperature_ddegC) {
                    packMaximumTemperature_ddegC = kpkCanShim->pTableMinMax->maximumTemperature_ddegC[s];
                }
            }
        }
    }
    return packMaximumTemperature_ddegC;
}

static int16_t CANTX_GetPackMinimumTemperature(const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(kpkCanShim != NULL_PTR);

    int16_t packMinimumTemperature_ddegC = INT16_MAX;
    if (0u == BMS_GetNumberOfConnectedStrings()) {
        /* Calculate min/max values of complete pack if all slice switches are open */
        for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
            if (kpkCanShim->pTableMinMax->minimumTemperature_ddegC[s] <= packMinimumTemperature_ddegC) {
                packMinimumTemperature_ddegC = kpkCanShim->pTableMinMax->minimumTemperature_ddegC[s];
            }
        }
    } else {
        /* Calculate min/max values of connected slices */
        for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
            if (BMS_IsStringClosed(s) == true) {
                if (kpkCanShim->pTableMinMax->minimumTemperature_ddegC[s] <= packMinimumTemperature_ddegC) {
                    packMinimumTemperature_ddegC = kpkCanShim->pTableMinMax->minimumTemperature_ddegC[s];
                }
            }
        }
    }
    return packMinimumTemperature_ddegC;
}

static uint64_t CANTX_CalculatePackMaximumTemperature(const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(kpkCanShim != NULL_PTR);

    /* Maximum cell temperature */
    float_t signalData = (float_t)CANTX_GetPackMaximumTemperature(kpkCanShim);
    CAN_TxPrepareSignalData(&signalData, cantx_signalMaximumCellTemperature);
    uint64_t data = (int64_t)signalData;
    return data;
}

static uint64_t CANTX_CalculatePackMinimumTemperature(const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(kpkCanShim != NULL_PTR);

    /* Minimum cell temperature */
    float_t signalData = (float_t)CANTX_GetPackMinimumTemperature(kpkCanShim);
    CAN_TxPrepareSignalData(&signalData, cantx_signalMinimumCellTemperature);
    uint64_t data = (int64_t)signalData;
    return data;
}

static void CANTX_BuildPackMinimumMaximumValuesMessage(const CAN_SHIM_s *const kpkCanShim, uint64_t *pMessageData) {
    FAS_ASSERT(kpkCanShim != NULL_PTR);
    FAS_ASSERT(pMessageData != NULL_PTR);

    /* maximum cell voltage */
    uint64_t signalData = (int64_t)CANTX_GetPackMaximumVoltage(kpkCanShim);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_SIGNAL_MAXIMUM_CELL_VOLTAGE_START_BIT,
        CANTX_SIGNAL_MAXIMUM_CELL_VOLTAGE_LENGTH,
        signalData,
        CAN_BIG_ENDIAN);
    /* minimum cell voltage */
    signalData = (int64_t)CANTX_GetPackMinimumVoltage(kpkCanShim);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_SIGNAL_MINIMUM_CELL_VOLTAGE_START_BIT,
        CANTX_SIGNAL_MINIMUM_CELL_VOLTAGE_LENGTH,
        signalData,
        CAN_BIG_ENDIAN);

    /* TODO: cooling inlet temperature*/
    /* TODO: cooling outlet temperature */

    /* maximum temperature*/
    signalData = CANTX_CalculatePackMaximumTemperature(kpkCanShim);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        cantx_signalMaximumCellTemperature.bitStart,
        cantx_signalMaximumCellTemperature.bitLength,
        signalData,
        CAN_BIG_ENDIAN);

    /* minimum temperature*/
    signalData = CANTX_CalculatePackMinimumTemperature(kpkCanShim);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        cantx_signalMinimumCellTemperature.bitStart,
        cantx_signalMinimumCellTemperature.bitLength,
        signalData,
        CAN_BIG_ENDIAN);
}

/*========== Extern Function Implementations ================================*/
extern uint32_t CANTX_PackMinimumMaximumValues(
    CAN_MESSAGE_PROPERTIES_s message,
    uint8_t *pCanData,
    uint8_t *pMuxId,
    const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(message.id == CANTX_PACK_MINIMUM_MAXIMUM_VALUES_ID);
    FAS_ASSERT(message.idType == CANTX_PACK_MINIMUM_MAXIMUM_VALUES_ID_TYPE);
    FAS_ASSERT(message.dlc == CAN_FOXBMS_MESSAGES_DEFAULT_DLC);
    FAS_ASSERT(message.endianness == CANTX_PACK_MINIMUM_MAXIMUM_VALUES_ENDIANNESS);
    FAS_ASSERT(pCanData != NULL_PTR);
    FAS_ASSERT(pMuxId == NULL_PTR); /* pMuxId is not used here, therefore has to be NULL_PTR */
    FAS_ASSERT(kpkCanShim != NULL_PTR);
    uint64_t messageData = 0u;

    DATA_READ_DATA(kpkCanShim->pTableMinMax);

    CANTX_BuildPackMinimumMaximumValuesMessage(kpkCanShim, &messageData);

    /* now copy data in the buffer that will be used to send data */
    CAN_TxSetCanDataWithMessageData(messageData, pCanData, message.endianness);

    return 0u;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
extern int16_t TEST_CANTX_GetPackMaximumVoltage(const CAN_SHIM_s *const kpkCanShim) {
    return CANTX_GetPackMaximumVoltage(kpkCanShim);
}
extern int16_t TEST_CANTX_GetPackMinimumVoltage(const CAN_SHIM_s *const kpkCanShim) {
    return CANTX_GetPackMinimumVoltage(kpkCanShim);
}
extern int16_t TEST_CANTX_GetPackMaximumTemperature(const CAN_SHIM_s *const kpkCanShim) {
    return CANTX_GetPackMaximumTemperature(kpkCanShim);
}
extern int16_t TEST_CANTX_GetPackMinimumTemperature(const CAN_SHIM_s *const kpkCanShim) {
    return CANTX_GetPackMinimumTemperature(kpkCanShim);
}
extern uint64_t TEST_CANTX_CalculatePackMaximumTemperature(const CAN_SHIM_s *const kpkCanShim) {
    return CANTX_CalculatePackMaximumTemperature(kpkCanShim);
}
extern uint64_t TEST_CANTX_CalculatePackMinimumTemperature(const CAN_SHIM_s *const kpkCanShim) {
    return CANTX_CalculatePackMinimumTemperature(kpkCanShim);
}
extern void TEST_CANTX_BuildPackMinimumMaximumValuesMessage(
    const CAN_SHIM_s *const kpkCanShim,
    uint64_t *pMessageData) {
    CANTX_BuildPackMinimumMaximumValuesMessage(kpkCanShim, pMessageData);
}
#endif
