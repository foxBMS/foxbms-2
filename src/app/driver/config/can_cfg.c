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
 * @file    can_cfg.c
 * @author  foxBMS Team
 * @date    2019-12-04 (date of creation)
 * @updated 2021-10-12 (date of last update)
 * @ingroup DRIVERS_CONFIGURATION
 * @prefix  CAN
 *
 * @brief   Configuration for the CAN module
 *
 * The CAN bus settings and the received messages and their
 * reception handling are to be specified here.
 *
 *
 */

/*========== Includes =======================================================*/
#include "can_cfg.h"

#include "can_cbs.h"
#include "database.h"
#include "ftask.h"

/*========== Macros and Definitions =========================================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Constant and Variable Definitions =======================*/

/** Multiplexer values @{*/
static uint8_t voltageMux               = 0u;
static uint8_t temperatureMux           = 0u;
static uint8_t stringStateMux           = 0u;
static uint8_t stringValuesMux          = 0u;
static uint8_t stringMinMaxMux          = 0u;
static uint8_t stringStateEstimationMux = 0u;
static uint8_t stringValues2Mux         = 0u;
/**@}*/

/*========== Extern Constant and Variable Definitions =======================*/

/* ***************************************
 *  Configure TX messages here
 ****************************************/

/** registry of CAN TX messages */
const CAN_MSG_TX_TYPE_s can_txMessages[] = {
    {CAN1_NODE,
     CAN_ID_TX_STATE,
     CAN_DLC,
     CAN_TX_STATE_PERIOD_MS,
     CAN_TX_STATE_PHASE_MS,
     CAN_BIG_ENDIAN,
     &CAN_TxState,
     NULL_PTR}, /*!< State */
    {CAN1_NODE,
     CAN_ID_TX_VOLTAGES,
     CAN_DLC,
     CAN_TX_VOLTAGES_PERIOD_MS,
     CAN_TX_VOLTAGES_PHASE_MS,
     CAN_BIG_ENDIAN,
     &CAN_TxVoltage,
     &voltageMux}, /*!< Cell voltages */
    {CAN1_NODE,
     CAN_ID_TX_TEMPERATURES,
     CAN_DLC,
     CAN_TX_TEMPERATURES_PERIOD_MS,
     CAN_TX_TEMPERATURES_PHASE_MS,
     CAN_BIG_ENDIAN,
     &CAN_TxCellTemperature,
     &temperatureMux}, /*!< Cell temperatures */
    {CAN1_NODE,
     CAN_ID_TX_LIMIT_VALUES,
     CAN_DLC,
     CAN_TX_LIMIT_VALUES_PERIOD_MS,
     CAN_TX_LIMIT_VALUES_PHASE_MS,
     CAN_BIG_ENDIAN,
     &CAN_TxLimitValues,
     NULL_PTR}, /*!< Limits */
    {CAN1_NODE,
     CAN_ID_TX_MINIMUM_MAXIMUM_VALUES,
     CAN_DLC,
     CAN_TX_MINIMUM_MAXIMUM_VALUES_PERIOD_MS,
     CAN_TX_MINIMUM_MAXIMUM_VALUES_PHASE_MS,
     CAN_BIG_ENDIAN,
     &CAN_TxMinimumMaximumValues,
     NULL_PTR}, /*!< Pack minimum and maximum values */
    {CAN1_NODE,
     CAN_ID_TX_PACK_STATE_ESTIMATION,
     CAN_DLC,
     CAN_TX_PACK_STATE_ESTIMATION_PERIOD_MS,
     CAN_TX_PACK_STATE_ESTIMATION_PHASE_MS,
     CAN_BIG_ENDIAN,
     &CAN_TxStateEstimation,
     NULL_PTR}, /*!< Pack state estimation */
    {CAN1_NODE,
     CAN_ID_TX_PACK_VALUES,
     CAN_DLC,
     CAN_TX_PACK_VALUES_PERIOD_MS,
     CAN_TX_PACK_VALUES_PHASE_MS,
     CAN_BIG_ENDIAN,
     &CAN_TxPackValues,
     NULL_PTR}, /*!< Pack values */
    {CAN1_NODE,
     CAN_ID_TX_STRING_STATE,
     CAN_DLC,
     CAN_TX_STRING_STATE_PERIOD_MS,
     CAN_TX_STRING_STATE_PHASE_MS,
     CAN_BIG_ENDIAN,
     &CAN_TxStringState,
     &stringStateMux}, /*!< String state values */
    {CAN1_NODE,
     CAN_ID_TX_STRING_VALUES,
     CAN_DLC,
     CAN_TX_STRING_VALUES_PERIOD_MS,
     CAN_TX_STRING_VALUES_PHASE_MS,
     CAN_BIG_ENDIAN,
     &CAN_TxStringValues,
     &stringValuesMux}, /*!< String values */
    {CAN1_NODE,
     CAN_ID_TX_STRING_MINIMUM_MAXIMUM,
     CAN_DLC,
     CAN_TX_STRING_MINIMUM_MAXIMUM_PERIOD_MS,
     CAN_TX_STRING_MINIMUM_MAXIMUM_PHASE_MS,
     CAN_BIG_ENDIAN,
     &CAN_TxStringMinimumMaximumValues,
     &stringMinMaxMux}, /*!< String minimum maximum values */
    {CAN1_NODE,
     CAN_ID_TX_STRING_STATE_ESTIMATION,
     CAN_DLC,
     CAN_TX_STRING_STATE_ESTIMATION_PERIOD_MS,
     CAN_TX_STRING_STATE_ESTIMATION_PHASE_MS,
     CAN_BIG_ENDIAN,
     &CAN_TxStringStateEstimation,
     &stringStateEstimationMux}, /*!< String minimum maximum values */
    {CAN1_NODE,
     CAN_ID_TX_STRING_VALUES_2,
     CAN_DLC,
     CAN_TX_STRING_VALUES_2_PERIOD_MS,
     CAN_TX_STRING_VALUES_2_PHASE_MS,
     CAN_BIG_ENDIAN,
     &CAN_TxStringValues2,
     &stringValues2Mux}, /*!< String minimum maximum values */

};

/* ***************************************
 *  Configure RX messages here
 ****************************************/

/** registry of CAN RX messages */
const CAN_MSG_RX_TYPE_s can_rxMessages[] = {
    {CAN1_NODE, CAN_ID_IMD_INFO, CAN_DLC, CAN_LITTLE_ENDIAN, &CAN_RxImdInfo}, /*!< info message from iso165c */
    {CAN1_NODE,
     CAN_ID_IMD_RESPONSE,
     CAN_DLC,
     CAN_LITTLE_ENDIAN,
     &CAN_RxImdResponse}, /*!< response mesage from iso165c */

    {CAN1_NODE, CAN_ID_COMMAND, CAN_DLC, CAN_BIG_ENDIAN, &CAN_RxRequest},           /*!< state request      */
    {CAN1_NODE, CAN_ID_SOFTWARE_RESET, CAN_DLC, CAN_LITTLE_ENDIAN, &CAN_RxSwReset}, /*!< software reset     */

    {CAN1_NODE,
     CAN_ID_STRING0_CURRENT,
     CAN_DLC,
     CAN_BIG_ENDIAN,
     &CAN_RxCurrentSensor}, /*!< current sensor I in cyclic mode   */
    {CAN1_NODE,
     CAN_ID_STRING0_VOLTAGE1,
     CAN_DLC,
     CAN_BIG_ENDIAN,
     &CAN_RxCurrentSensor}, /*!< current sensor U1 in cyclic mode  */
    {CAN1_NODE,
     CAN_ID_STRING0_VOLTAGE2,
     CAN_DLC,
     CAN_BIG_ENDIAN,
     &CAN_RxCurrentSensor}, /*!< current sensor U2 in cyclic mode  */
    {CAN1_NODE,
     CAN_ID_STRING0_VOLTAGE3,
     CAN_DLC,
     CAN_BIG_ENDIAN,
     &CAN_RxCurrentSensor}, /*!< current sensor U3 in cyclic mode  */
    {CAN1_NODE,
     CAN_ID_STRING0_TEMPERATURE,
     CAN_DLC,
     CAN_BIG_ENDIAN,
     &CAN_RxCurrentSensor}, /*!< current sensor T in cyclic mode  */
    {CAN1_NODE,
     CAN_ID_STRING0_POWER,
     CAN_DLC,
     CAN_BIG_ENDIAN,
     &CAN_RxCurrentSensor}, /*!< current sensor Power in cyclic mode  */
    {CAN1_NODE,
     CAN_ID_STRING0_CURRENT_COUNTER,
     CAN_DLC,
     CAN_BIG_ENDIAN,
     &CAN_RxCurrentSensor}, /*!< current sensor C-C in cyclic mode  */
    {CAN1_NODE,
     CAN_ID_STRING0_ENERGY_COUNTER,
     CAN_DLC,
     CAN_BIG_ENDIAN,
     &CAN_RxCurrentSensor}, /*!< current sensor E-C in cyclic mode  */

#if BS_NR_OF_STRINGS > 1u
    {CAN1_NODE,
     CAN_ID_STRING1_CURRENT,
     CAN_DLC,
     CAN_BIG_ENDIAN,
     &CAN_RxCurrentSensor}, /*!< current sensor I in cyclic mode   */
    {CAN1_NODE,
     CAN_ID_STRING1_VOLTAGE1,
     CAN_DLC,
     CAN_BIG_ENDIAN,
     &CAN_RxCurrentSensor}, /*!< current sensor U1 in cyclic mode  */
    {CAN1_NODE,
     CAN_ID_STRING1_VOLTAGE2,
     CAN_DLC,
     CAN_BIG_ENDIAN,
     &CAN_RxCurrentSensor}, /*!< current sensor U2 in cyclic mode  */
    {CAN1_NODE,
     CAN_ID_STRING1_VOLTAGE3,
     CAN_DLC,
     CAN_BIG_ENDIAN,
     &CAN_RxCurrentSensor}, /*!< current sensor U3 in cyclic mode  */
    {CAN1_NODE,
     CAN_ID_STRING1_TEMPERATURE,
     CAN_DLC,
     CAN_BIG_ENDIAN,
     &CAN_RxCurrentSensor}, /*!< current sensor T in cyclic mode  */
    {CAN1_NODE,
     CAN_ID_STRING1_POWER,
     CAN_DLC,
     CAN_BIG_ENDIAN,
     &CAN_RxCurrentSensor}, /*!< current sensor Power in cyclic mode  */
    {CAN1_NODE,
     CAN_ID_STRING1_CURRENT_COUNTER,
     CAN_DLC,
     CAN_BIG_ENDIAN,
     &CAN_RxCurrentSensor}, /*!< current sensor C-C in cyclic mode  */
    {CAN1_NODE,
     CAN_ID_STRING1_ENERGY_COUNTER,
     CAN_DLC,
     CAN_BIG_ENDIAN,
     &CAN_RxCurrentSensor}, /*!< current sensor E-C in cyclic mode  */

#if BS_NR_OF_STRINGS > 2u
    {CAN1_NODE,
     CAN_ID_STRING2_CURRENT,
     CAN_DLC,
     CAN_BIG_ENDIAN,
     &CAN_RxCurrentSensor}, /*!< current sensor I in cyclic mode   */
    {CAN1_NODE,
     CAN_ID_STRING2_VOLTAGE1,
     CAN_DLC,
     CAN_BIG_ENDIAN,
     &CAN_RxCurrentSensor}, /*!< current sensor U1 in cyclic mode  */
    {CAN1_NODE,
     CAN_ID_STRING2_VOLTAGE2,
     CAN_DLC,
     CAN_BIG_ENDIAN,
     &CAN_RxCurrentSensor}, /*!< current sensor U2 in cyclic mode  */
    {CAN1_NODE,
     CAN_ID_STRING2_VOLTAGE3,
     CAN_DLC,
     CAN_BIG_ENDIAN,
     &CAN_RxCurrentSensor}, /*!< current sensor U3 in cyclic mode  */
    {CAN1_NODE,
     CAN_ID_STRING2_TEMPERATURE,
     CAN_DLC,
     CAN_BIG_ENDIAN,
     &CAN_RxCurrentSensor}, /*!< current sensor T in cyclic mode  */
    {CAN1_NODE,
     CAN_ID_STRING2_POWER,
     CAN_DLC,
     CAN_BIG_ENDIAN,
     &CAN_RxCurrentSensor}, /*!< current sensor Power in cyclic mode  */
    {CAN1_NODE,
     CAN_ID_STRING2_CURRENT_COUNTER,
     CAN_DLC,
     CAN_BIG_ENDIAN,
     &CAN_RxCurrentSensor}, /*!< current sensor C-C in cyclic mode  */
    {CAN1_NODE,
     CAN_ID_STRING2_ENERGY_COUNTER,
     CAN_DLC,
     CAN_BIG_ENDIAN,
     &CAN_RxCurrentSensor}, /*!< current sensor E-C in cyclic mode  */

#endif /* BS_NR_OF_STRINGS > 2 */
#endif /* BS_NR_OF_STRINGS > 1 */

    {CAN1_NODE, CAN_ID_DEBUG, CAN_DLC, CAN_LITTLE_ENDIAN, &CAN_RxDebug},          /*!< debug message      */
    {CAN1_NODE, CAN_ID_SW_VERSION, CAN_DLC, CAN_LITTLE_ENDIAN, &CAN_RxSwVersion}, /*!< request SW version */
};

/** length of CAN message arrays @{*/
const uint8_t can_txLength = sizeof(can_txMessages) / sizeof(can_txMessages[0]);
const uint8_t can_rxLength = sizeof(can_rxMessages) / sizeof(can_rxMessages[0]);
/**@}*/

/** local copies of database tables */
/**@{*/
static DATA_BLOCK_CELL_VOLTAGE_s can_tableCellVoltages        = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE};
static DATA_BLOCK_CELL_TEMPERATURE_s can_tableTemperatures    = {.header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE};
static DATA_BLOCK_MIN_MAX_s can_tableMinimumMaximumValues     = {.header.uniqueId = DATA_BLOCK_ID_MIN_MAX};
static DATA_BLOCK_CURRENT_SENSOR_s can_tableCurrentSensor     = {.header.uniqueId = DATA_BLOCK_ID_CURRENT_SENSOR};
static DATA_BLOCK_OPEN_WIRE_s can_tableOpenWire               = {.header.uniqueId = DATA_BLOCK_ID_OPEN_WIRE_BASE};
static DATA_BLOCK_STATEREQUEST_s can_tableStateRequest        = {.header.uniqueId = DATA_BLOCK_ID_STATEREQUEST};
static DATA_BLOCK_PACK_VALUES_s can_tablePackValues           = {.header.uniqueId = DATA_BLOCK_ID_PACK_VALUES};
static DATA_BLOCK_SOF_s can_tableSof                          = {.header.uniqueId = DATA_BLOCK_ID_SOF};
static DATA_BLOCK_SOX_s can_tableSox                          = {.header.uniqueId = DATA_BLOCK_ID_SOX};
static DATA_BLOCK_ERRORSTATE_s can_tableErrorState            = {.header.uniqueId = DATA_BLOCK_ID_ERRORSTATE};
static DATA_BLOCK_INSULATION_MONITORING_s can_tableInsulation = {
    .header.uniqueId = DATA_BLOCK_ID_INSULATION_MONITORING};
static DATA_BLOCK_MSL_FLAG_s can_tableMslFlags = {.header.uniqueId = DATA_BLOCK_ID_MSL_FLAG};
static DATA_BLOCK_RSL_FLAG_s can_tableRslFlags = {.header.uniqueId = DATA_BLOCK_ID_RSL_FLAG};
static DATA_BLOCK_MOL_FLAG_s can_tableMolFlags = {.header.uniqueId = DATA_BLOCK_ID_MOL_FLAG};
/**@}*/

const CAN_SHIM_s can_kShim = {
    .pQueueImd             = &ftsk_imdCanDataQueue,
    .pTableCellVoltage     = &can_tableCellVoltages,
    .pTableCellTemperature = &can_tableTemperatures,
    .pTableMinMax          = &can_tableMinimumMaximumValues,
    .pTableCurrentSensor   = &can_tableCurrentSensor,
    .pTableOpenWire        = &can_tableOpenWire,
    .pTableStateRequest    = &can_tableStateRequest,
    .pTablePackValues      = &can_tablePackValues,
    .pTableSof             = &can_tableSof,
    .pTableSox             = &can_tableSox,
    .pTableErrorState      = &can_tableErrorState,
    .pTableInsulation      = &can_tableInsulation,
    .pTableMsl             = &can_tableMslFlags,
    .pTableRsl             = &can_tableRslFlags,
    .pTableMol             = &can_tableMolFlags,
};

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST

#endif
