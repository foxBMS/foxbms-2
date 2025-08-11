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
 * @file    can_cbs_tx_cell-voltages.c
 * @author  foxBMS Team
 * @date    2021-04-20 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup DRIVERS
 * @prefix  CANTX
 *
 * @brief   CAN driver Tx callback implementation
 * @details CAN Tx callback for cell voltages
 */

/*========== Includes =======================================================*/
/* AXIVION Next Codeline Generic-LocalInclude: 'can_cbs_tx_cyclic.h' declares
 * the prototype for the callback 'CANTX_CellVoltages' */
#include "can_cbs_tx_cyclic.h"
#include "can_cfg_tx-cyclic-message-definitions.h"
#include "can_helper.h"
#include "foxmath.h"

#include <math.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/
/** CAN message parameters for can cell voltage  */
#define CANRX_CAN_CELL_VOLTAGE_MUX_START_BIT           (7u)
#define CANRX_CAN_CELL_VOLTAGE_MUX_LENGTH              (8u)
#define CANRX_CAN_CELL_VOLTAGE0_INVALID_FLAG_START_BIT (12u)
#define CANRX_CAN_CELL_VOLTAGE1_INVALID_FLAG_START_BIT (13u)
#define CANRX_CAN_CELL_VOLTAGE2_INVALID_FLAG_START_BIT (14u)
#define CANRX_CAN_CELL_VOLTAGE3_INVALID_FLAG_START_BIT (15u)
#define CANRX_CAN_CELL_VOLTAGE_INVALID_FLAG_LENGTH     (1u)
#define CANRX_CAN_CELL_VOLTAGE0_START_BIT              (11u)
#define CANRX_CAN_CELL_VOLTAGE1_START_BIT              (30u)
#define CANRX_CAN_CELL_VOLTAGE2_START_BIT              (33u)
#define CANRX_CAN_CELL_VOLTAGE3_START_BIT              (52u)
#define CANRX_CAN_CELL_VOLTAGE_LENGTH                  (13u)

#define CANRX_MINIMUM_VALUE_MUX          (0.0f)
#define CANRX_MAXIMUM_VALUE_MUX          (255.0f)
#define CANRX_MINIMUM_VALUE_INVALID_FLAG (0.0f)
#define CANRX_MAXIMUM_VALUE_INVALID_FLAG (1.0f)
#define CANRX_MINIMUM_VALUE_VOLTAGE      (0.0f)
#define CANRX_MAXIMUM_VALUE_VOLTAGE      (8191.0f)

/*========== Static Constant and Variable Definitions =======================*/
/** the number of voltages per message-frame */
#define CANTX_NUMBER_OF_MUX_VOLTAGES_PER_MESSAGE (4u)

#if ((BS_NR_OF_CELL_BLOCKS % CANTX_NUMBER_OF_MUX_VOLTAGES_PER_MESSAGE) == 0)
#define CANTX_NUMBER_OF_CAN_MESSAGES_FOR_CELL_VOLTAGES \
    ((uint8_t)(BS_NR_OF_CELL_BLOCKS / CANTX_NUMBER_OF_MUX_VOLTAGES_PER_MESSAGE))
#else
#define CANTX_NUMBER_OF_CAN_MESSAGES_FOR_CELL_VOLTAGES \
    ((uint8_t)(BS_NR_OF_CELL_BLOCKS / CANTX_NUMBER_OF_MUX_VOLTAGES_PER_MESSAGE) + 1u)
#endif

/**
 * CAN signals used in this message
 * Parameters:
 * bit start, bit length, factor, offset, minimum value, maximum value
 */
static const CAN_SIGNAL_TYPE_s cantx_cellVoltageMultiplexer = {
    CANRX_CAN_CELL_VOLTAGE_MUX_START_BIT,
    CANRX_CAN_CELL_VOLTAGE_MUX_LENGTH,
    UNIT_CONVERSION_FACTOR_1_FLOAT,
    CAN_SIGNAL_OFFSET_0,
    CANRX_MINIMUM_VALUE_MUX,
    CANRX_MAXIMUM_VALUE_MUX};
static const CAN_SIGNAL_TYPE_s cantx_cellVoltage0InvalidFlag = {
    CANRX_CAN_CELL_VOLTAGE0_INVALID_FLAG_START_BIT,
    CANRX_CAN_CELL_VOLTAGE_INVALID_FLAG_LENGTH,
    UNIT_CONVERSION_FACTOR_1_FLOAT,
    CAN_SIGNAL_OFFSET_0,
    CANRX_MINIMUM_VALUE_INVALID_FLAG,
    CANRX_MAXIMUM_VALUE_INVALID_FLAG};
static const CAN_SIGNAL_TYPE_s cantx_cellVoltage1InvalidFlag = {
    CANRX_CAN_CELL_VOLTAGE1_INVALID_FLAG_START_BIT,
    CANRX_CAN_CELL_VOLTAGE_INVALID_FLAG_LENGTH,
    UNIT_CONVERSION_FACTOR_1_FLOAT,
    CAN_SIGNAL_OFFSET_0,
    CANRX_MINIMUM_VALUE_INVALID_FLAG,
    CANRX_MAXIMUM_VALUE_INVALID_FLAG};
static const CAN_SIGNAL_TYPE_s cantx_cellVoltage2InvalidFlag = {
    CANRX_CAN_CELL_VOLTAGE2_INVALID_FLAG_START_BIT,
    CANRX_CAN_CELL_VOLTAGE_INVALID_FLAG_LENGTH,
    UNIT_CONVERSION_FACTOR_1_FLOAT,
    CAN_SIGNAL_OFFSET_0,
    CANRX_MINIMUM_VALUE_INVALID_FLAG,
    CANRX_MAXIMUM_VALUE_INVALID_FLAG};
static const CAN_SIGNAL_TYPE_s cantx_cellVoltage3InvalidFlag = {
    CANRX_CAN_CELL_VOLTAGE3_INVALID_FLAG_START_BIT,
    CANRX_CAN_CELL_VOLTAGE_INVALID_FLAG_LENGTH,
    UNIT_CONVERSION_FACTOR_1_FLOAT,
    CAN_SIGNAL_OFFSET_0,
    CANRX_MINIMUM_VALUE_INVALID_FLAG,
    CANRX_MAXIMUM_VALUE_INVALID_FLAG};
static const CAN_SIGNAL_TYPE_s cantx_cellVoltage0_mV = {
    CANRX_CAN_CELL_VOLTAGE0_START_BIT,
    CANRX_CAN_CELL_VOLTAGE_LENGTH,
    UNIT_CONVERSION_FACTOR_1_FLOAT,
    CAN_SIGNAL_OFFSET_0,
    CANRX_MINIMUM_VALUE_VOLTAGE,
    CANRX_MAXIMUM_VALUE_VOLTAGE};
static const CAN_SIGNAL_TYPE_s cantx_cellVoltage1_mV = {
    CANRX_CAN_CELL_VOLTAGE1_START_BIT,
    CANRX_CAN_CELL_VOLTAGE_LENGTH,
    UNIT_CONVERSION_FACTOR_1_FLOAT,
    CAN_SIGNAL_OFFSET_0,
    CANRX_MINIMUM_VALUE_VOLTAGE,
    CANRX_MAXIMUM_VALUE_VOLTAGE};
static const CAN_SIGNAL_TYPE_s cantx_cellVoltage2_mV = {
    CANRX_CAN_CELL_VOLTAGE2_START_BIT,
    CANRX_CAN_CELL_VOLTAGE_LENGTH,
    UNIT_CONVERSION_FACTOR_1_FLOAT,
    CAN_SIGNAL_OFFSET_0,
    CANRX_MINIMUM_VALUE_VOLTAGE,
    CANRX_MAXIMUM_VALUE_VOLTAGE};
static const CAN_SIGNAL_TYPE_s cantx_cellVoltage3_mV = {
    CANRX_CAN_CELL_VOLTAGE3_START_BIT,
    CANRX_CAN_CELL_VOLTAGE_LENGTH,
    UNIT_CONVERSION_FACTOR_1_FLOAT,
    CAN_SIGNAL_OFFSET_0,
    CANRX_MINIMUM_VALUE_VOLTAGE,
    CANRX_MAXIMUM_VALUE_VOLTAGE};

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/
/**
 * @brief   Helper function for CANTX_CellVoltages()
 * @details Used in the CANTX_CellVoltages() callback to set invalid flag data
 *          and voltage data in the CAN frame.
 * @param[in] cellId                       ID of th cell
 * @param[in] pMessage                     pointer to CAN frame data
 * @param[in] cellVoltageSignal            signal characteristics for voltage
 *                                         data
 * @param[in] cellVoltageInvalidFlagSignal signal characteristics for invalid
 *                                         flag data
 * @param[in] endianness                   big or little endianness of data
 * @param[in] kpkCanShim                   shim to the database entries
 */
static void CANTX_VoltageSetData(
    uint16_t cellId,
    uint64_t *pMessage,
    CAN_SIGNAL_TYPE_s cellVoltageSignal,
    CAN_SIGNAL_TYPE_s cellVoltageInvalidFlagSignal,
    CAN_ENDIANNESS_e endianness,
    const CAN_SHIM_s *const kpkCanShim);

/*========== Static Function Implementations ================================*/
static void CANTX_VoltageSetData(
    uint16_t cellId,
    uint64_t *pMessage,
    CAN_SIGNAL_TYPE_s cellVoltageSignal,
    CAN_SIGNAL_TYPE_s cellVoltageInvalidFlagSignal,
    CAN_ENDIANNESS_e endianness,
    const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(cellId < BS_NR_OF_CELL_BLOCKS);
    FAS_ASSERT(pMessage != NULL_PTR);
    /* AXIVION Routine Generic-MissingParameterAssert: cellVoltageSignal: parameter accepts whole range */
    /* AXIVION Routine Generic-MissingParameterAssert: cellVoltageInvalidFlagSignal: parameter accepts whole range */
    FAS_ASSERT(endianness == CAN_BIG_ENDIAN);
    FAS_ASSERT(kpkCanShim != NULL_PTR);
    /* Get string, module and cell number */
    const uint8_t stringNumber    = DATA_GetStringNumberFromVoltageIndex(cellId);
    const uint8_t moduleNumber    = DATA_GetModuleNumberFromVoltageIndex(cellId);
    const uint8_t cellBlockNumber = DATA_GetCellNumberFromVoltageIndex(cellId);

    uint32_t signalDataIsValid = 0u;
    /* Valid bits data */
    if (kpkCanShim->pTableCellVoltage->invalidCellVoltage[stringNumber][moduleNumber][cellBlockNumber] == false) {
        signalDataIsValid = 1u;
    } else {
        signalDataIsValid = 0u;
    }
    /* Set valid bit data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(
        pMessage,
        cellVoltageInvalidFlagSignal.bitStart,
        cellVoltageInvalidFlagSignal.bitLength,
        signalDataIsValid,
        endianness);

    /*Voltage data */
    float_t signalData_mV =
        (float_t)(kpkCanShim->pTableCellVoltage->cellVoltage_mV[stringNumber][moduleNumber][cellBlockNumber]);
    /* Apply offset and factor */
    CAN_TxPrepareSignalData(&signalData_mV, cellVoltageSignal);
    /* Set voltage data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(
        pMessage, cellVoltageSignal.bitStart, cellVoltageSignal.bitLength, (uint32_t)signalData_mV, endianness);
}

/*========== Extern Function Implementations ================================*/
extern uint32_t CANTX_CellVoltages(
    CAN_MESSAGE_PROPERTIES_s message,
    uint8_t *pCanData,
    uint8_t *pMuxId,
    const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(message.id == CANTX_CELL_VOLTAGES_ID);
    FAS_ASSERT(message.idType == CANTX_CELL_VOLTAGES_ID_TYPE);
    FAS_ASSERT(message.dlc == CAN_FOXBMS_MESSAGES_DEFAULT_DLC);
    FAS_ASSERT(message.endianness == CAN_BIG_ENDIAN);
    FAS_ASSERT(pCanData != NULL_PTR);
    FAS_ASSERT(pMuxId != NULL_PTR);
    FAS_ASSERT(kpkCanShim != NULL_PTR);
    uint64_t messageData = 0u;

    /* Reset mux if maximum was reached */
    if (*pMuxId >= CANTX_NUMBER_OF_CAN_MESSAGES_FOR_CELL_VOLTAGES) {
        *pMuxId = 0u;
        /* First signal to transmit cell voltages: get database values */
        DATA_READ_DATA(kpkCanShim->pTableCellVoltage);
    }

    /* Set mux signal in CAN frame */
    CAN_TxSetMessageDataWithSignalData(
        &messageData,
        cantx_cellVoltageMultiplexer.bitStart,
        cantx_cellVoltageMultiplexer.bitLength,
        (uint64_t)*pMuxId,
        message.endianness);

    /* Set other signals in CAN frame */
    /* Calculate the global cell ID based on the multiplexer value for the first cell */
    uint16_t cellId = (*pMuxId * CANTX_NUMBER_OF_MUX_VOLTAGES_PER_MESSAGE);
    CANTX_VoltageSetData(
        cellId, &messageData, cantx_cellVoltage0_mV, cantx_cellVoltage0InvalidFlag, message.endianness, kpkCanShim);
    cellId++; /* Increment global cell ID */
    if (cellId < BS_NR_OF_CELL_BLOCKS) {
        CANTX_VoltageSetData(
            cellId, &messageData, cantx_cellVoltage1_mV, cantx_cellVoltage1InvalidFlag, message.endianness, kpkCanShim);
        cellId++; /* Increment global cell ID */
        if (cellId < BS_NR_OF_CELL_BLOCKS) {
            CANTX_VoltageSetData(
                cellId,
                &messageData,
                cantx_cellVoltage2_mV,
                cantx_cellVoltage2InvalidFlag,
                message.endianness,
                kpkCanShim);
            cellId++; /* Increment global cell ID */
            if (cellId < BS_NR_OF_CELL_BLOCKS) {
                CANTX_VoltageSetData(
                    cellId,
                    &messageData,
                    cantx_cellVoltage3_mV,
                    cantx_cellVoltage3InvalidFlag,
                    message.endianness,
                    kpkCanShim);
            }
        }
    }

    /* Increment multiplexer for next cell */
    (*pMuxId)++;

    /* All signal data copied in CAN frame, now copy data in the buffer that will be use to send the frame */
    CAN_TxSetCanDataWithMessageData(messageData, pCanData, message.endianness);

    return 0u;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
extern void TEST_CANTX_VoltageSetData(
    uint16_t cellId,
    uint64_t *pMessage,
    CAN_SIGNAL_TYPE_s cellVoltageSignal,
    CAN_SIGNAL_TYPE_s cellVoltageInvalidFlagSignal,
    CAN_ENDIANNESS_e endianness,
    const CAN_SHIM_s *const kpkCanShim) {
    CANTX_VoltageSetData(cellId, pMessage, cellVoltageSignal, cellVoltageInvalidFlagSignal, endianness, kpkCanShim);
}

#endif
