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
 * @file    can_cbs_tx_string-state.c
 * @author  foxBMS Team
 * @date    2023-05-31 (date of creation)
 * @updated 2025-03-31 (date of last update)
 * @version v1.9.0
 * @ingroup DRIVERS
 * @prefix  CANTX
 *
 * @brief   CAN driver Tx callback implementation
 * @details CAN Tx callback for state messages
 */

/*========== Includes =======================================================*/
#include "bms.h"
/* AXIVION Next Codeline Generic-LocalInclude: 'can_cbs_tx_cyclic.h' declares
 * the prototype for the callback 'CANTX_StringState' */
#include "can_cbs_tx_cyclic.h"
#include "can_cfg_tx-cyclic-message-definitions.h"
#include "can_helper.h"

#include <math.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/
/**
 * Configuration of the signals
 */
#define CANTX_SIGNAL_MUX_STRING_START_BIT                  (3u)
#define CANTX_SIGNAL_MUX_STRING_LENGTH                     (4u)
#define CANTX_SIGNAL_STRING_DEEP_DISCHARGE_ERROR_START_BIT (7u)
#define CANTX_SIGNAL_STRING_DEEP_DISCHARGE_ERROR_LENGTH    (CAN_BIT)
#define CANTX_SIGNAL_STRING_IS_STRING_FUSE_BLOWN_START_BIT (6u)
#define CANTX_SIGNAL_STRING_IS_STRING_FUSE_BLOWN_LENGTH    (CAN_BIT)
#define CANTX_SIGNAL_STRING_IS_BALANCING_ACTIVE_START_BIT  (5u)
#define CANTX_SIGNAL_STRING_IS_BALANCING_ACTIVE_LENGTH     (CAN_BIT)
#define CANTX_SIGNAL_STRING_IS_STRING_CONNECTED_START_BIT  (4u)
#define CANTX_SIGNAL_STRING_IS_STRING_CONNECTED_LENGTH     (CAN_BIT)

#define CANTX_SIGNAL_STRING_MSL_UNDERVOLTAGE_ERROR_START_BIT               (15u)
#define CANTX_SIGNAL_STRING_MSL_UNDERVOLTAGE_ERROR_LENGTH                  (CAN_BIT)
#define CANTX_SIGNAL_STRING_MSL_OVERVOLTAGE_ERROR_START_BIT                (14u)
#define CANTX_SIGNAL_STRING_MSL_OVERVOLTAGE_ERROR_LENGTH                   (CAN_BIT)
#define CANTX_SIGNAL_STRING_MSL_CELL_OVERCURRENT_DISCHARGE_ERROR_START_BIT (13u)
#define CANTX_SIGNAL_STRING_MSL_CELL_OVERCURRENT_DISCHARGE_ERROR_LENGTH    (CAN_BIT)
#define CANTX_SIGNAL_STRING_MSL_CELL_OVERCURRENT_CHARGE_ERROR_START_BIT    (12u)
#define CANTX_SIGNAL_STRING_MSL_CELL_OVERCURRENT_CHARGE_ERROR_LENGTH       (CAN_BIT)
#define CANTX_SIGNAL_STRING_MSL_UNDERTEMPERATURE_DISCHARGE_ERROR_START_BIT (11u)
#define CANTX_SIGNAL_STRING_MSL_UNDERTEMPERATURE_DISCHARGE_ERROR_LENGTH    (CAN_BIT)
#define CANTX_SIGNAL_STRING_MSL_OVERTEMPERATURE_DISCHARGE_ERROR_START_BIT  (10u)
#define CANTX_SIGNAL_STRING_MSL_OVERTEMPERATURE_DISCHARGE_ERROR_LENGTH     (CAN_BIT)
#define CANTX_SIGNAL_STRING_MSL_UNDERTEMPERATURE_CHARGE_ERROR_START_BIT    (9u)
#define CANTX_SIGNAL_STRING_MSL_UNDERTEMPERATURE_CHARGE_ERROR_LENGTH       (CAN_BIT)
#define CANTX_SIGNAL_STRING_MSL_OVERTEMPERATURE_CHARGE_ERROR_START_BIT     (8u)
#define CANTX_SIGNAL_STRING_MSL_OVERTEMPERATURE_CHARGE_ERROR_LENGTH        (CAN_BIT)

#define CANTX_SIGNAL_STRING_MOL_UNDERVOLTAGE_WARNING_START_BIT               (23u)
#define CANTX_SIGNAL_STRING_MOL_UNDERVOLTAGE_WARNING_LENGTH                  (CAN_BIT)
#define CANTX_SIGNAL_STRING_MOL_OVERVOLTAGE_WARNING_START_BIT                (22u)
#define CANTX_SIGNAL_STRING_MOL_OVERVOLTAGE_WARNING_LENGTH                   (CAN_BIT)
#define CANTX_SIGNAL_STRING_MOL_CELL_OVERCURRENT_DISCHARGE_WARNING_START_BIT (21u)
#define CANTX_SIGNAL_STRING_MOL_CELL_OVERCURRENT_DISCHARGE_WARNING_LENGTH    (CAN_BIT)
#define CANTX_SIGNAL_STRING_MOL_CELL_OVERCURRENT_CHARGE_WARNING_START_BIT    (20u)
#define CANTX_SIGNAL_STRING_MOL_CELL_OVERCURRENT_CHARGE_WARNING_LENGTH       (CAN_BIT)
#define CANTX_SIGNAL_STRING_MOL_UNDERTEMPERATURE_DISCHARGE_WARNING_START_BIT (19u)
#define CANTX_SIGNAL_STRING_MOL_UNDERTEMPERATURE_DISCHARGE_WARNING_LENGTH    (CAN_BIT)
#define CANTX_SIGNAL_STRING_MOL_OVERTEMPERATURE_DISCHARGE_WARNING_START_BIT  (18u)
#define CANTX_SIGNAL_STRING_MOL_OVERTEMPERATURE_DISCHARGE_WARNING_LENGTH     (CAN_BIT)
#define CANTX_SIGNAL_STRING_MOL_UNDERTEMPERATURE_CHARGE_WARNING_START_BIT    (17u)
#define CANTX_SIGNAL_STRING_MOL_UNDERTEMPERATURE_CHARGE_WARNING_LENGTH       (CAN_BIT)
#define CANTX_SIGNAL_STRING_MOL_OVERTEMPERATURE_CHARGE_WARNING_START_BIT     (16u)
#define CANTX_SIGNAL_STRING_MOL_OVERTEMPERATURE_CHARGE_WARNING_LENGTH        (CAN_BIT)

#define CANTX_SIGNAL_STRING_RSL_UNDERVOLTAGE_WARNING_START_BIT               (31u)
#define CANTX_SIGNAL_STRING_RSL_UNDERVOLTAGE_WARNING_LENGTH                  (CAN_BIT)
#define CANTX_SIGNAL_STRING_RSL_OVERVOLTAGE_WARNING_START_BIT                (30u)
#define CANTX_SIGNAL_STRING_RSL_OVERVOLTAGE_WARNING_LENGTH                   (CAN_BIT)
#define CANTX_SIGNAL_STRING_RSL_CELL_OVERCURRENT_DISCHARGE_WARNING_START_BIT (29u)
#define CANTX_SIGNAL_STRING_RSL_CELL_OVERCURRENT_DISCHARGE_WARNING_LENGTH    (CAN_BIT)
#define CANTX_SIGNAL_STRING_RSL_CELL_OVERCURRENT_CHARGE_WARNING_START_BIT    (28u)
#define CANTX_SIGNAL_STRING_RSL_CELL_OVERCURRENT_CHARGE_WARNING_LENGTH       (CAN_BIT)
#define CANTX_SIGNAL_STRING_RSL_UNDERTEMPERATURE_DISCHARGE_WARNING_START_BIT (27u)
#define CANTX_SIGNAL_STRING_RSL_UNDERTEMPERATURE_DISCHARGE_WARNING_LENGTH    (CAN_BIT)
#define CANTX_SIGNAL_STRING_RSL_OVERTEMPERATURE_DISCHARGE_WARNING_START_BIT  (26u)
#define CANTX_SIGNAL_STRING_RSL_OVERTEMPERATURE_DISCHARGE_WARNING_LENGTH     (CAN_BIT)
#define CANTX_SIGNAL_STRING_RSL_UNDERTEMPERATURE_CHARGE_WARNING_START_BIT    (25u)
#define CANTX_SIGNAL_STRING_RSL_UNDERTEMPERATURE_CHARGE_WARNING_LENGTH       (CAN_BIT)
#define CANTX_SIGNAL_STRING_RSL_OVERTEMPERATURE_CHARGE_WARNING_START_BIT     (24u)
#define CANTX_SIGNAL_STRING_RSL_OVERTEMPERATURE_CHARGE_WARNING_LENGTH        (CAN_BIT)

#define CANTX_SIGNAL_STRING_DAISY_CHAIN_BASE_VOLT_MEAS_OOR_ERROR_START_BIT   (39u)
#define CANTX_SIGNAL_STRING_DAISY_CHAIN_BASE_VOLT_MEAS_OOR_ERROR_LENGTH      (CAN_BIT)
#define CANTX_SIGNAL_STRING_DAISY_CHAIN_BASE_CRC_ERROR_START_BIT             (37u)
#define CANTX_SIGNAL_STRING_DAISY_CHAIN_BASE_CRC_ERROR_LENGTH                (CAN_BIT)
#define CANTX_SIGNAL_STRING_DAISY_CHAIN_BASE_COMMUNICATION_ERROR_START_BIT   (35u)
#define CANTX_SIGNAL_STRING_DAISY_CHAIN_BASE_COMMUNICATION_ERROR_LENGTH      (CAN_BIT)
#define CANTX_SIGNAL_STRING_SLAVE_HARDWARE_ERROR_START_BIT                   (34u)
#define CANTX_SIGNAL_STRING_SLAVE_HARDWARE_ERROR_LENGTH                      (CAN_BIT)
#define CANTX_SIGNAL_STRING_NEGATIVE_CONTACTOR_ERROR_START_BIT               (33u)
#define CANTX_SIGNAL_STRING_NEGATIVE_CONTACTOR_ERROR_LENGTH                  (CAN_BIT)
#define CANTX_SIGNAL_STRING_POSITIVE_CONTACTOR_ERROR_START_BIT               (32u)
#define CANTX_SIGNAL_STRING_POSITIVE_CONTACTOR_ERROR_LENGTH                  (CAN_BIT)
#define CANTX_SIGNAL_STRING_FUSED_STRING_VOLTAGE_MEASUREMENT_ERROR_START_BIT (47u)
#define CANTX_SIGNAL_STRING_FUSED_STRING_VOLTAGE_MEASUREMENT_ERROR_LENGTH    (CAN_BIT)
#define CANTX_SIGNAL_STRING_STRING_VOLTAGE_MEASUREMENT_ERROR_START_BIT       (46u)
#define CANTX_SIGNAL_STRING_STRING_VOLTAGE_MEASUREMENT_ERROR_LENGTH          (CAN_BIT)
#define CANTX_SIGNAL_STRING_ENERGY_COUNTING_MEASUREMENT_ERROR_START_BIT      (45u)
#define CANTX_SIGNAL_STRING_ENERGY_COUNTING_MEASUREMENT_ERROR_LENGTH         (CAN_BIT)
#define CANTX_SIGNAL_STRING_COULOMB_COUNTING_MEASUREMENT_ERROR_START_BIT     (44u)
#define CANTX_SIGNAL_STRING_COULOMB_COUNTING_MEASUREMENT_ERROR_LENGTH        (CAN_BIT)
#define CANTX_SIGNAL_STRING_STRING_CURRENT_MEASUREMENT_ERROR_START_BIT       (43u)
#define CANTX_SIGNAL_STRING_STRING_CURRENT_MEASUREMENT_ERROR_LENGTH          (CAN_BIT)
#define CANTX_SIGNAL_STRING_DAISY_CHAIN_BASE_TEMP_MEAS_OOR_ERROR_START_BIT   (41u)
#define CANTX_SIGNAL_STRING_DAISY_CHAIN_BASE_TEMP_MEAS_OOR_ERROR_LENGTH      (CAN_BIT)
#define CANTX_SIGNAL_STRING_VOLTAGE_SPREAD_PLAUSIBILITY_ERROR_START_BIT      (55u)
#define CANTX_SIGNAL_STRING_VOLTAGE_SPREAD_PLAUSIBILITY_ERROR_LENGTH         (CAN_BIT)
#define CANTX_SIGNAL_STRING_TEMPERATURE_SPREAD_PLAUSIBILITY_ERROR_START_BIT  (54u)
#define CANTX_SIGNAL_STRING_TEMPERATURE_SPREAD_PLAUSIBILITY_ERROR_LENGTH     (CAN_BIT)
#define CANTX_SIGNAL_STRING_STRING_VOLTAGE_PLAUSIBILITY_ERROR_START_BIT      (53u)
#define CANTX_SIGNAL_STRING_STRING_VOLTAGE_PLAUSIBILITY_ERROR_LENGTH         (CAN_BIT)
#define CANTX_SIGNAL_STRING_CELL_VOLTAGE_PLAUSIBILITY_ERROR_START_BIT        (52u)
#define CANTX_SIGNAL_STRING_CELL_VOLTAGE_PLAUSIBILITY_ERROR_LENGTH           (CAN_BIT)
#define CANTX_SIGNAL_STRING_CELL_TEMPERATURE_PLAUSIBILITY_ERROR_START_BIT    (51u)
#define CANTX_SIGNAL_STRING_CELL_TEMPERATURE_PLAUSIBILITY_ERROR_LENGTH       (CAN_BIT)
#define CANTX_SIGNAL_STRING_OPEN_WIRE_ERROR_START_BIT                        (49u)
#define CANTX_SIGNAL_STRING_OPEN_WIRE_ERROR_LENGTH                           (CAN_BIT)
#define CANTX_SIGNAL_STRING_PACK_VOLTAGE_MEASUREMENT_ERROR_START_BIT         (48u)
#define CANTX_SIGNAL_STRING_PACK_VOLTAGE_MEASUREMENT_ERROR_LENGTH            (CAN_BIT)
#define CANTX_SIGNAL_STRING_STRING_OVERCURRENT_ERROR_START_BIT               (63u)
#define CANTX_SIGNAL_STRING_STRING_OVERCURRENT_ERROR_LENGTH                  (CAN_BIT)

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/
/**
 * @brief Reads violated msl flags and sets the corresponding bits for the can message
 * @param kpkCanShim Can shim that contains the data to be read
 * @param pMessageData Message data where bits are set
 * @param stringNumber Number of the string whose data is read
 */
static void CANTX_SetMslFlags(const CAN_SHIM_s *const kpkCanShim, uint64_t *pMessageData, uint8_t stringNumber);

/**
 * @brief Reads violated mol flags and sets the corresponding bits for the can message
 * @param kpkCanShim Can shim that contains the data to be read
 * @param pMessageData Message data where bits are set
 * @param stringNumber Number of the string whose data is read
 */
static void CANTX_SetMolFlags(const CAN_SHIM_s *const kpkCanShim, uint64_t *pMessageData, uint8_t stringNumber);

/**
 * @brief Reads violated rsl flags and sets the corresponding bits for the can message
 * @param kpkCanShim Can shim that contains the data to be read
 * @param pMessageData Message data where bits are set
 * @param stringNumber Number of the string whose data is read
 */
static void CANTX_SetRslFlags(const CAN_SHIM_s *const kpkCanShim, uint64_t *pMessageData, uint8_t stringNumber);

/**
 * @brief Reads violated current sensor flags and sets the corresponding bits for the can message
 * @param kpkCanShim Can shim that contains the data to be read
 * @param pMessageData Message data where bits are set
 * @param stringNumber Number of the string whose data is read
 */
static void CANTX_SetCurrentSensorFlags(
    const CAN_SHIM_s *const kpkCanShim,
    uint64_t *pMessageData,
    uint8_t stringNumber);

/**
 * @brief Reads violated plausibility flags and sets the corresponding bits for the can message
 * @param kpkCanShim Can shim that contains the data to be read
 * @param pMessageData Message data where bits are set
 * @param stringNumber Number of the string whose data is read
 */
static void CANTX_SetPlausibilityFlags(
    const CAN_SHIM_s *const kpkCanShim,
    uint64_t *pMessageData,
    uint8_t stringNumber);

/**
 * @brief Reads error flags and sets the corresponding bits for the can message
 * @param kpkCanShim Can shim that contains the data to be read
 * @param pMessageData Message data where bits are set
 * @param stringNumber Number of the string whose data is read
 */
static void CANTX_SetOtherErrorFlags(const CAN_SHIM_s *const kpkCanShim, uint64_t *pMessageData, uint8_t stringNumber);

/**
 * @brief Calls the other helper functions to write the message data
 * @param kpkCanShim Can shim that contains the data to be read
 * @param pMessageData Message data where bits are set
 * @param stringNumber Number of the string whose data is read
 */
static void CANTX_BuildStringStateMessage(
    const CAN_SHIM_s *const kpkCanShim,
    uint64_t *pMessageData,
    uint8_t stringNumber);

/*========== Static Function Implementations ================================*/
static void CANTX_SetMslFlags(const CAN_SHIM_s *const kpkCanShim, uint64_t *pMessageData, uint8_t stringNumber) {
    FAS_ASSERT(kpkCanShim != NULL_PTR);
    FAS_ASSERT(pMessageData != NULL_PTR);
    FAS_ASSERT(stringNumber <= BS_NR_OF_STRINGS);

    /* Error: Overtemperature charge */
    uint64_t data = kpkCanShim->pTableMsl->overtemperatureCharge[stringNumber];
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_SIGNAL_STRING_MSL_OVERTEMPERATURE_CHARGE_ERROR_START_BIT,
        CANTX_SIGNAL_STRING_MSL_OVERTEMPERATURE_CHARGE_ERROR_LENGTH,
        data,
        CANTX_STRING_STATE_ENDIANNESS);

    /* Error: Undertemperature charge */
    data = kpkCanShim->pTableMsl->undertemperatureCharge[stringNumber];
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_SIGNAL_STRING_MSL_UNDERTEMPERATURE_CHARGE_ERROR_START_BIT,
        CANTX_SIGNAL_STRING_MSL_UNDERTEMPERATURE_CHARGE_ERROR_LENGTH,
        data,
        CANTX_STRING_STATE_ENDIANNESS);

    /* Error: Overtemperature discharge */
    data = kpkCanShim->pTableMsl->overtemperatureDischarge[stringNumber];
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_SIGNAL_STRING_MSL_OVERTEMPERATURE_DISCHARGE_ERROR_START_BIT,
        CANTX_SIGNAL_STRING_MSL_OVERTEMPERATURE_DISCHARGE_ERROR_LENGTH,
        data,
        CANTX_STRING_STATE_ENDIANNESS);

    /* Error: Undertemperature discharge */
    data = kpkCanShim->pTableMsl->undertemperatureDischarge[stringNumber];
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_SIGNAL_STRING_MSL_UNDERTEMPERATURE_DISCHARGE_ERROR_START_BIT,
        CANTX_SIGNAL_STRING_MSL_UNDERTEMPERATURE_DISCHARGE_ERROR_LENGTH,
        data,
        CANTX_STRING_STATE_ENDIANNESS);

    /* Error: cell overcurrent charge */
    if ((kpkCanShim->pTableMsl->cellChargeOvercurrent[stringNumber] == 1u) ||
        (kpkCanShim->pTableMsl->stringChargeOvercurrent[stringNumber] == 1u)) {
        data = 1u;
    } else {
        data = 0u;
    }
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_SIGNAL_STRING_MSL_CELL_OVERCURRENT_CHARGE_ERROR_START_BIT,
        CANTX_SIGNAL_STRING_MSL_CELL_OVERCURRENT_CHARGE_ERROR_LENGTH,
        data,
        CANTX_STRING_STATE_ENDIANNESS);

    /* Error: Overcurrent discharge */
    if ((kpkCanShim->pTableMsl->cellDischargeOvercurrent[stringNumber] == 1u) ||
        (kpkCanShim->pTableMsl->stringDischargeOvercurrent[stringNumber] == 1u)) {
        data = 1u;
    } else {
        data = 0u;
    }
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_SIGNAL_STRING_MSL_CELL_OVERCURRENT_DISCHARGE_ERROR_START_BIT,
        CANTX_SIGNAL_STRING_MSL_CELL_OVERCURRENT_DISCHARGE_ERROR_LENGTH,
        data,
        CANTX_STRING_STATE_ENDIANNESS);

    /* Error: Overvoltage */
    data = kpkCanShim->pTableMsl->overVoltage[stringNumber];
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_SIGNAL_STRING_MSL_OVERVOLTAGE_ERROR_START_BIT,
        CANTX_SIGNAL_STRING_MSL_OVERVOLTAGE_ERROR_LENGTH,
        data,
        CANTX_STRING_STATE_ENDIANNESS);

    /* Error: Undervoltage */
    data = kpkCanShim->pTableMsl->underVoltage[stringNumber];
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_SIGNAL_STRING_MSL_UNDERVOLTAGE_ERROR_START_BIT,
        CANTX_SIGNAL_STRING_MSL_UNDERVOLTAGE_ERROR_LENGTH,
        data,
        CANTX_STRING_STATE_ENDIANNESS);
}

static void CANTX_SetMolFlags(const CAN_SHIM_s *const kpkCanShim, uint64_t *pMessageData, uint8_t stringNumber) {
    FAS_ASSERT(kpkCanShim != NULL_PTR);
    FAS_ASSERT(pMessageData != NULL_PTR);
    FAS_ASSERT(stringNumber <= BS_NR_OF_STRINGS);

    /* Info: Overtemperature charge - MOL */
    uint64_t data = kpkCanShim->pTableMol->overtemperatureCharge[stringNumber];
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_SIGNAL_STRING_MOL_OVERTEMPERATURE_CHARGE_WARNING_START_BIT,
        CANTX_SIGNAL_STRING_MOL_OVERTEMPERATURE_CHARGE_WARNING_LENGTH,
        data,
        CANTX_STRING_STATE_ENDIANNESS);

    /* Info: Undertemperature charge - MOL */
    data = kpkCanShim->pTableMol->undertemperatureCharge[stringNumber];
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_SIGNAL_STRING_MOL_UNDERTEMPERATURE_CHARGE_WARNING_START_BIT,
        CANTX_SIGNAL_STRING_MOL_UNDERTEMPERATURE_CHARGE_WARNING_LENGTH,
        data,
        CANTX_STRING_STATE_ENDIANNESS);

    /* Info: Overtemperature discharge - MOL */
    data = kpkCanShim->pTableMol->overtemperatureDischarge[stringNumber];
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_SIGNAL_STRING_MOL_OVERTEMPERATURE_DISCHARGE_WARNING_START_BIT,
        CANTX_SIGNAL_STRING_MOL_OVERTEMPERATURE_DISCHARGE_WARNING_LENGTH,
        data,
        CANTX_STRING_STATE_ENDIANNESS);

    /* Info: Undertemperature discharge - MOL */
    data = kpkCanShim->pTableMol->undertemperatureCharge[stringNumber];
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_SIGNAL_STRING_MOL_UNDERTEMPERATURE_DISCHARGE_WARNING_START_BIT,
        CANTX_SIGNAL_STRING_MOL_UNDERTEMPERATURE_DISCHARGE_WARNING_LENGTH,
        data,
        CANTX_STRING_STATE_ENDIANNESS);

    /* Info: Overcurrent charge - MOL */
    if ((kpkCanShim->pTableMol->cellChargeOvercurrent[stringNumber] == 1u) ||
        (kpkCanShim->pTableMol->stringChargeOvercurrent[stringNumber] == 1u)) {
        data = 1u;
    } else {
        data = 0u;
    }
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_SIGNAL_STRING_MOL_CELL_OVERCURRENT_CHARGE_WARNING_START_BIT,
        CANTX_SIGNAL_STRING_MOL_CELL_OVERCURRENT_CHARGE_WARNING_LENGTH,
        data,
        CANTX_STRING_STATE_ENDIANNESS);

    /* Info: Overcurrent discharge - MOL */
    if ((kpkCanShim->pTableMol->cellDischargeOvercurrent[stringNumber] == 1u) ||
        (kpkCanShim->pTableMol->stringDischargeOvercurrent[stringNumber] == 1u)) {
        data = 1u;
    } else {
        data = 0u;
    }
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_SIGNAL_STRING_MOL_CELL_OVERCURRENT_DISCHARGE_WARNING_START_BIT,
        CANTX_SIGNAL_STRING_MOL_CELL_OVERCURRENT_DISCHARGE_WARNING_LENGTH,
        data,
        CANTX_STRING_STATE_ENDIANNESS);

    /* Info: Overvoltage - MOL */
    data = kpkCanShim->pTableMol->overVoltage[stringNumber];
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_SIGNAL_STRING_MOL_OVERVOLTAGE_WARNING_START_BIT,
        CANTX_SIGNAL_STRING_MOL_OVERVOLTAGE_WARNING_LENGTH,
        data,
        CANTX_STRING_STATE_ENDIANNESS);

    /* Info: Undervoltage - MOL */
    data = kpkCanShim->pTableMol->underVoltage[stringNumber];
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_SIGNAL_STRING_MOL_UNDERVOLTAGE_WARNING_START_BIT,
        CANTX_SIGNAL_STRING_MOL_UNDERVOLTAGE_WARNING_LENGTH,
        data,
        CANTX_STRING_STATE_ENDIANNESS);
}

static void CANTX_SetRslFlags(const CAN_SHIM_s *const kpkCanShim, uint64_t *pMessageData, uint8_t stringNumber) {
    FAS_ASSERT(kpkCanShim != NULL_PTR);
    FAS_ASSERT(pMessageData != NULL_PTR);
    FAS_ASSERT(stringNumber <= BS_NR_OF_STRINGS);

    /* Warning: Overtemperature charge - RSL */
    uint64_t data = kpkCanShim->pTableRsl->overtemperatureCharge[stringNumber];
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_SIGNAL_STRING_RSL_OVERTEMPERATURE_CHARGE_WARNING_START_BIT,
        CANTX_SIGNAL_STRING_RSL_OVERTEMPERATURE_CHARGE_WARNING_LENGTH,
        data,
        CANTX_STRING_STATE_ENDIANNESS);

    /* Warning: Undertemperature charge - RSL */
    data = kpkCanShim->pTableRsl->undertemperatureCharge[stringNumber];
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_SIGNAL_STRING_RSL_UNDERTEMPERATURE_CHARGE_WARNING_START_BIT,
        CANTX_SIGNAL_STRING_RSL_UNDERTEMPERATURE_CHARGE_WARNING_LENGTH,
        data,
        CANTX_STRING_STATE_ENDIANNESS);

    /* Warning: Overtemperature discharge - RSL */
    data = kpkCanShim->pTableRsl->overtemperatureDischarge[stringNumber];
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_SIGNAL_STRING_RSL_OVERTEMPERATURE_DISCHARGE_WARNING_START_BIT,
        CANTX_SIGNAL_STRING_RSL_OVERTEMPERATURE_DISCHARGE_WARNING_LENGTH,
        data,
        CANTX_STRING_STATE_ENDIANNESS);

    /* Warning: Undertemperature discharge - RSL */
    data = kpkCanShim->pTableRsl->undertemperatureDischarge[stringNumber];
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_SIGNAL_STRING_RSL_UNDERTEMPERATURE_DISCHARGE_WARNING_START_BIT,
        CANTX_SIGNAL_STRING_RSL_UNDERTEMPERATURE_DISCHARGE_WARNING_LENGTH,
        data,
        CANTX_STRING_STATE_ENDIANNESS);

    /* Warning: Overcurrent charge - RSL */
    if ((kpkCanShim->pTableRsl->cellChargeOvercurrent[stringNumber] == 1u) ||
        (kpkCanShim->pTableRsl->stringChargeOvercurrent[stringNumber] == 1u)) {
        data = 1u;
    } else {
        data = 0u;
    }
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_SIGNAL_STRING_RSL_CELL_OVERCURRENT_CHARGE_WARNING_START_BIT,
        CANTX_SIGNAL_STRING_RSL_CELL_OVERCURRENT_CHARGE_WARNING_LENGTH,
        data,
        CANTX_STRING_STATE_ENDIANNESS);

    /* Warning: Overcurrent discharge - RSL */
    if ((kpkCanShim->pTableRsl->cellDischargeOvercurrent[stringNumber] == 1u) ||
        (kpkCanShim->pTableRsl->stringDischargeOvercurrent[stringNumber] == 1u)) {
        data = 1u;
    } else {
        data = 0u;
    }
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_SIGNAL_STRING_RSL_CELL_OVERCURRENT_DISCHARGE_WARNING_START_BIT,
        CANTX_SIGNAL_STRING_RSL_CELL_OVERCURRENT_DISCHARGE_WARNING_LENGTH,
        data,
        CANTX_STRING_STATE_ENDIANNESS);

    /* Warning: Overvoltage - RSL */
    data = kpkCanShim->pTableRsl->overVoltage[stringNumber];
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_SIGNAL_STRING_RSL_OVERVOLTAGE_WARNING_START_BIT,
        CANTX_SIGNAL_STRING_RSL_OVERVOLTAGE_WARNING_LENGTH,
        data,
        CANTX_STRING_STATE_ENDIANNESS);

    /* Warning: Undervoltage - RSL */
    data = kpkCanShim->pTableRsl->underVoltage[stringNumber];
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_SIGNAL_STRING_RSL_UNDERVOLTAGE_WARNING_START_BIT,
        CANTX_SIGNAL_STRING_RSL_UNDERVOLTAGE_WARNING_LENGTH,
        data,
        CANTX_STRING_STATE_ENDIANNESS);
}

static void CANTX_SetCurrentSensorFlags(
    const CAN_SHIM_s *const kpkCanShim,
    uint64_t *pMessageData,
    uint8_t stringNumber) {
    FAS_ASSERT(kpkCanShim != NULL_PTR);
    FAS_ASSERT(pMessageData != NULL_PTR);
    FAS_ASSERT(stringNumber <= BS_NR_OF_STRINGS);

    /* Error: Coulomb counting measurement */
    uint64_t data = CAN_ConvertBooleanToInteger(
        kpkCanShim->pTableErrorState->currentSensorCoulombCounterTimeoutError[stringNumber]);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_SIGNAL_STRING_COULOMB_COUNTING_MEASUREMENT_ERROR_START_BIT,
        CANTX_SIGNAL_STRING_COULOMB_COUNTING_MEASUREMENT_ERROR_LENGTH,
        data,
        CANTX_STRING_STATE_ENDIANNESS);

    /* Error: Energy counting measurement */
    data =
        CAN_ConvertBooleanToInteger(kpkCanShim->pTableErrorState->currentSensorEnergyCounterTimeoutError[stringNumber]);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_SIGNAL_STRING_ENERGY_COUNTING_MEASUREMENT_ERROR_START_BIT,
        CANTX_SIGNAL_STRING_ENERGY_COUNTING_MEASUREMENT_ERROR_LENGTH,
        data,
        CANTX_STRING_STATE_ENDIANNESS);

    /* Error: dedicated string voltage measurement */
    /* CAN:TODO either timeout or measurement invalid */
    data = CAN_ConvertBooleanToInteger(kpkCanShim->pTableErrorState->currentSensorVoltage1TimeoutError[stringNumber]);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_SIGNAL_STRING_STRING_VOLTAGE_MEASUREMENT_ERROR_START_BIT,
        CANTX_SIGNAL_STRING_STRING_VOLTAGE_MEASUREMENT_ERROR_LENGTH,
        data,
        CANTX_STRING_STATE_ENDIANNESS);

    /* Error: dedicated fused string voltage measurement */
    /* CAN:TODO either timeout or measurement invalid */
    data = CAN_ConvertBooleanToInteger(kpkCanShim->pTableErrorState->currentSensorVoltage2TimeoutError[stringNumber]);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_SIGNAL_STRING_FUSED_STRING_VOLTAGE_MEASUREMENT_ERROR_START_BIT,
        CANTX_SIGNAL_STRING_FUSED_STRING_VOLTAGE_MEASUREMENT_ERROR_LENGTH,
        data,
        CANTX_STRING_STATE_ENDIANNESS);

    /* Error: dedicated fused pack voltage measurement */
    /* CAN:TODO either timeout or measurement invalid */
    data = CAN_ConvertBooleanToInteger(kpkCanShim->pTableErrorState->currentSensorVoltage3TimeoutError[stringNumber]);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_SIGNAL_STRING_PACK_VOLTAGE_MEASUREMENT_ERROR_START_BIT,
        CANTX_SIGNAL_STRING_PACK_VOLTAGE_MEASUREMENT_ERROR_LENGTH,
        data,
        CANTX_STRING_STATE_ENDIANNESS);
}

static void CANTX_SetPlausibilityFlags(
    const CAN_SHIM_s *const kpkCanShim,
    uint64_t *pMessageData,
    uint8_t stringNumber) {
    FAS_ASSERT(kpkCanShim != NULL_PTR);
    FAS_ASSERT(pMessageData != NULL_PTR);
    FAS_ASSERT(stringNumber <= BS_NR_OF_STRINGS);

    /* Error: Plausibility: Cell temperature */
    uint64_t data =
        CAN_ConvertBooleanToInteger(kpkCanShim->pTableErrorState->plausibilityCheckCellTemperatureError[stringNumber]);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_SIGNAL_STRING_CELL_TEMPERATURE_PLAUSIBILITY_ERROR_START_BIT,
        CANTX_SIGNAL_STRING_CELL_TEMPERATURE_PLAUSIBILITY_ERROR_LENGTH,
        data,
        CANTX_STRING_STATE_ENDIANNESS);

    /* Error: Plausibility: Cell voltage */
    data = CAN_ConvertBooleanToInteger(kpkCanShim->pTableErrorState->plausibilityCheckCellVoltageError[stringNumber]);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_SIGNAL_STRING_CELL_VOLTAGE_PLAUSIBILITY_ERROR_START_BIT,
        CANTX_SIGNAL_STRING_CELL_VOLTAGE_PLAUSIBILITY_ERROR_LENGTH,
        data,
        CANTX_STRING_STATE_ENDIANNESS);

    /* Error: Plausibility: String voltage */
    data = CAN_ConvertBooleanToInteger(kpkCanShim->pTableErrorState->plausibilityCheckPackVoltageError[stringNumber]);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_SIGNAL_STRING_STRING_VOLTAGE_PLAUSIBILITY_ERROR_START_BIT,
        CANTX_SIGNAL_STRING_STRING_VOLTAGE_PLAUSIBILITY_ERROR_LENGTH,
        data,
        CANTX_STRING_STATE_ENDIANNESS);

    /* Error: Plausibility: Cell temperature spread */
    data = CAN_ConvertBooleanToInteger(
        kpkCanShim->pTableErrorState->plausibilityCheckCellTemperatureSpreadError[stringNumber]);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_SIGNAL_STRING_TEMPERATURE_SPREAD_PLAUSIBILITY_ERROR_START_BIT,
        CANTX_SIGNAL_STRING_TEMPERATURE_SPREAD_PLAUSIBILITY_ERROR_LENGTH,
        data,
        CANTX_STRING_STATE_ENDIANNESS);

    /* Error: Plausibility: Cell voltage spread */
    data = CAN_ConvertBooleanToInteger(
        kpkCanShim->pTableErrorState->plausibilityCheckCellVoltageSpreadError[stringNumber]);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_SIGNAL_STRING_VOLTAGE_SPREAD_PLAUSIBILITY_ERROR_START_BIT,
        CANTX_SIGNAL_STRING_VOLTAGE_SPREAD_PLAUSIBILITY_ERROR_LENGTH,
        data,
        CANTX_STRING_STATE_ENDIANNESS);
}

static void CANTX_SetOtherErrorFlags(const CAN_SHIM_s *const kpkCanShim, uint64_t *pMessageData, uint8_t stringNumber) {
    FAS_ASSERT(kpkCanShim != NULL_PTR);
    FAS_ASSERT(pMessageData != NULL_PTR);
    FAS_ASSERT(stringNumber <= BS_NR_OF_STRINGS);

    /* String fuse blown */
    uint64_t data = CAN_ConvertBooleanToInteger(kpkCanShim->pTableErrorState->stringFuseError[stringNumber]);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_SIGNAL_STRING_IS_STRING_FUSE_BLOWN_START_BIT,
        CANTX_SIGNAL_STRING_IS_STRING_FUSE_BLOWN_LENGTH,
        data,
        CANTX_STRING_STATE_ENDIANNESS);

    /* Balancing active - database entry read for f_BmsState message */
    if (kpkCanShim->pTableBalancingControl->nrBalancedCells[stringNumber] == 0u) {
        data = 0u;
    } else {
        /* At least one cell is currently being balanced */
        data = 1u;
    }
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_SIGNAL_STRING_IS_BALANCING_ACTIVE_START_BIT,
        CANTX_SIGNAL_STRING_IS_BALANCING_ACTIVE_LENGTH,
        data,
        CANTX_STRING_STATE_ENDIANNESS);

    /* Error: Deep-discharge */
    data = CAN_ConvertBooleanToInteger(kpkCanShim->pTableErrorState->deepDischargeDetectedError[stringNumber]);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_SIGNAL_STRING_DEEP_DISCHARGE_ERROR_START_BIT,
        CANTX_SIGNAL_STRING_DEEP_DISCHARGE_ERROR_LENGTH,
        data,
        CANTX_STRING_STATE_ENDIANNESS);

    /* Error: Positive string contactor */
    data = CAN_ConvertBooleanToInteger(
        kpkCanShim->pTableErrorState->contactorInPositivePathOfStringFeedbackError[stringNumber]);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_SIGNAL_STRING_POSITIVE_CONTACTOR_ERROR_START_BIT,
        CANTX_SIGNAL_STRING_POSITIVE_CONTACTOR_ERROR_LENGTH,
        data,
        CANTX_STRING_STATE_ENDIANNESS);

    /* Error: Negative string contactor */
    data = CAN_ConvertBooleanToInteger(
        kpkCanShim->pTableErrorState->contactorInNegativePathOfStringFeedbackError[stringNumber]);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_SIGNAL_STRING_NEGATIVE_CONTACTOR_ERROR_START_BIT,
        CANTX_SIGNAL_STRING_NEGATIVE_CONTACTOR_ERROR_LENGTH,
        data,
        CANTX_STRING_STATE_ENDIANNESS);

    /* Error: Slave hardware: TODO */

    /* Error: Daisy-chain base: communication */
    data = CAN_ConvertBooleanToInteger(kpkCanShim->pTableErrorState->afeCommunicationSpiError[stringNumber]);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_SIGNAL_STRING_DAISY_CHAIN_BASE_COMMUNICATION_ERROR_START_BIT,
        CANTX_SIGNAL_STRING_DAISY_CHAIN_BASE_COMMUNICATION_ERROR_LENGTH,
        data,
        CANTX_STRING_STATE_ENDIANNESS);

    /* Error: Daisy-chain redundancy: communication: TODO */

    /* Error: Daisy-chain base: CRC */
    data = CAN_ConvertBooleanToInteger(kpkCanShim->pTableErrorState->afeCommunicationCrcError[stringNumber]);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_SIGNAL_STRING_DAISY_CHAIN_BASE_CRC_ERROR_START_BIT,
        CANTX_SIGNAL_STRING_DAISY_CHAIN_BASE_CRC_ERROR_LENGTH,
        data,
        CANTX_STRING_STATE_ENDIANNESS);

    /* Error: Daisy-chain redundancy: CRC: TODO */

    /* Error: Daisy-chain base: Voltage out of operating range */
    data = CAN_ConvertBooleanToInteger(kpkCanShim->pTableErrorState->afeCellVoltageInvalidError[stringNumber]);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_SIGNAL_STRING_DAISY_CHAIN_BASE_VOLT_MEAS_OOR_ERROR_START_BIT,
        CANTX_SIGNAL_STRING_DAISY_CHAIN_BASE_VOLT_MEAS_OOR_ERROR_LENGTH,
        data,
        CANTX_STRING_STATE_ENDIANNESS);

    /* Error: Daisy-chain redundancy: Voltage out of operating range: TODO */

    /* Error: Daisy-chain base: Temperature out of operating range */
    data = CAN_ConvertBooleanToInteger(kpkCanShim->pTableErrorState->afeCellTemperatureInvalidError[stringNumber]);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_SIGNAL_STRING_DAISY_CHAIN_BASE_TEMP_MEAS_OOR_ERROR_START_BIT,
        CANTX_SIGNAL_STRING_DAISY_CHAIN_BASE_TEMP_MEAS_OOR_ERROR_LENGTH,
        data,
        CANTX_STRING_STATE_ENDIANNESS);

    /* Error: Daisy-chain redundancy: Temperature out of operating range: TODO */

    /* Error: string current measurement, either communication timeout or invalid measurement */
    if ((kpkCanShim->pTableErrorState->currentMeasurementInvalidError[stringNumber] == true) ||
        (kpkCanShim->pTableErrorState->currentMeasurementTimeoutError[stringNumber] == true)) {
        data = 1u;
    } else {
        data = 0u;
    }
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_SIGNAL_STRING_STRING_CURRENT_MEASUREMENT_ERROR_START_BIT,
        CANTX_SIGNAL_STRING_STRING_CURRENT_MEASUREMENT_ERROR_LENGTH,
        data,
        CANTX_STRING_STATE_ENDIANNESS);

    /* Error: Open wire */
    data = CAN_ConvertBooleanToInteger(kpkCanShim->pTableErrorState->openWireDetectedError[stringNumber]);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_SIGNAL_STRING_OPEN_WIRE_ERROR_START_BIT,
        CANTX_SIGNAL_STRING_OPEN_WIRE_ERROR_LENGTH,
        data,
        CANTX_STRING_STATE_ENDIANNESS);

    /* CAN:TODO OvercurrentError */
}

static void CANTX_BuildStringStateMessage(
    const CAN_SHIM_s *const kpkCanShim,
    uint64_t *pMessageData,
    uint8_t stringNumber) {
    FAS_ASSERT(kpkCanShim != NULL_PTR);
    FAS_ASSERT(pMessageData != NULL_PTR);
    FAS_ASSERT(stringNumber <= BS_NR_OF_STRINGS);

    CANTX_SetMslFlags(kpkCanShim, pMessageData, stringNumber);
    CANTX_SetMolFlags(kpkCanShim, pMessageData, stringNumber);
    CANTX_SetRslFlags(kpkCanShim, pMessageData, stringNumber);
    CANTX_SetCurrentSensorFlags(kpkCanShim, pMessageData, stringNumber);
    CANTX_SetPlausibilityFlags(kpkCanShim, pMessageData, stringNumber);
    CANTX_SetOtherErrorFlags(kpkCanShim, pMessageData, stringNumber);
}

/*========== Extern Function Implementations ================================*/
extern uint32_t CANTX_StringState(
    CAN_MESSAGE_PROPERTIES_s message,
    uint8_t *pCanData,
    uint8_t *pMuxId,
    const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(message.id == CANTX_STRING_STATE_ID);
    FAS_ASSERT(message.idType == CANTX_STRING_STATE_ID_TYPE);
    FAS_ASSERT(message.dlc == CAN_FOXBMS_MESSAGES_DEFAULT_DLC);
    FAS_ASSERT(pCanData != NULL_PTR);
    FAS_ASSERT(pMuxId != NULL_PTR);
    FAS_ASSERT(*pMuxId < BS_NR_OF_STRINGS);
    FAS_ASSERT(kpkCanShim != NULL_PTR);
    uint64_t messageData = 0u;

    const uint8_t stringNumber = *pMuxId;

    /* First signal to transmit cell voltages: get database values */
    if (stringNumber == 0u) {
        /* Do not read pTableMsl and pTableErrorState as they already are read
         * with a higher frequency from CAN_TxState callback */
        (void)DATA_READ_DATA(kpkCanShim->pTableRsl, kpkCanShim->pTableMol);
    }

    /* Set mux value */
    CAN_TxSetMessageDataWithSignalData(
        &messageData,
        CANTX_SIGNAL_MUX_STRING_START_BIT,
        CANTX_SIGNAL_MUX_STRING_LENGTH,
        stringNumber,
        CANTX_STRING_STATE_ENDIANNESS);

    /* String connected */
    uint64_t data = CAN_ConvertBooleanToInteger(BMS_IsStringClosed(stringNumber));
    CAN_TxSetMessageDataWithSignalData(
        &messageData,
        CANTX_SIGNAL_STRING_IS_STRING_CONNECTED_START_BIT,
        CANTX_SIGNAL_STRING_IS_STRING_CONNECTED_LENGTH,
        data,
        CANTX_STRING_STATE_ENDIANNESS);

    /* call function to set the message data */
    CANTX_BuildStringStateMessage(kpkCanShim, &messageData, stringNumber);

    /* now copy data in the buffer that will be used to send data */
    CAN_TxSetCanDataWithMessageData(messageData, pCanData, CANTX_STRING_STATE_ENDIANNESS);

    /* Increment multiplexer for next cell */
    (*pMuxId)++;

    /* Check mux value */
    if (*pMuxId >= BS_NR_OF_STRINGS) {
        *pMuxId = 0u;
    }

    return 0u;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
