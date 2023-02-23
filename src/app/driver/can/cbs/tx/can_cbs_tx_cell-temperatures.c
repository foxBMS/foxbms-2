/**
 *
 * @copyright &copy; 2010 - 2023, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * @file    can_cbs_tx_cell-temperatures.c
 * @author  foxBMS Team
 * @date    2021-04-20 (date of creation)
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
 * @ingroup DRIVER
 * @prefix  CANTX
 *
 * @brief   CAN driver Tx callback implementation
 * @details CAN Tx callback for cell temperatures
 */

/*========== Includes =======================================================*/
#include "can_cbs_tx.h"
#include "can_cfg_tx-message-definitions.h"
#include "can_helper.h"
#include "foxmath.h"

#include <math.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/**
 * CAN signals used in this message
 * Parameters:
 * bit start, bit length, factor, offset, minimum value, maximum value
 */
static const CAN_SIGNAL_TYPE_s cantx_cellTemperatureMultiplexer  = {7u, 8u, 1.0f, 0.0f, 0.0f, 1.0f};
static const CAN_SIGNAL_TYPE_s cantx_cell0TemperatureInvalidFlag = {8u, 1u, 1.0f, 0.0f, 0.0f, 1.0f};
static const CAN_SIGNAL_TYPE_s cantx_cell1TemperatureInvalidFlag = {9u, 1u, 1.0f, 0.0f, 0.0f, 1.0f};
static const CAN_SIGNAL_TYPE_s cantx_cell2TemperatureInvalidFlag = {10u, 1u, 1.0f, 0.0f, 0.0f, 1.0f};
static const CAN_SIGNAL_TYPE_s cantx_cell3TemperatureInvalidFlag = {11u, 1u, 1.0f, 0.0f, 0.0f, 1.0f};
static const CAN_SIGNAL_TYPE_s cantx_cell4TemperatureInvalidFlag = {12u, 1u, 1.0f, 0.0f, 0.0f, 1.0f};
static const CAN_SIGNAL_TYPE_s cantx_cell5TemperatureInvalidFlag = {13u, 1u, 1.0f, 0.0f, 0.0f, 1.0f};
static const CAN_SIGNAL_TYPE_s cantx_cell0Temperature_degC       = {23u, 8u, 1.0f, 0.0f, -128.0f, 127.0f};
static const CAN_SIGNAL_TYPE_s cantx_cell1Temperature_degC       = {31u, 8u, 1.0f, 0.0f, -128.0f, 127.0f};
static const CAN_SIGNAL_TYPE_s cantx_cell2Temperature_degC       = {39u, 8u, 1.0f, 0.0f, -128.0f, 127.0f};
static const CAN_SIGNAL_TYPE_s cantx_cell3Temperature_degC       = {47u, 8u, 1.0f, 0.0f, -128.0f, 127.0f};
static const CAN_SIGNAL_TYPE_s cantx_cell4Temperature_degC       = {55u, 8u, 1.0f, 0.0f, -128.0f, 127.0f};
static const CAN_SIGNAL_TYPE_s cantx_cell5Temperature_degC       = {63u, 8u, 1.0f, 0.0f, -128.0f, 127.0f};

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/**
 * @brief Helper function for CANTX_CellTemperatures()
 *
 * Used in the CANTX_CellTemperatures() callback to set
 * invalid flag data and temperature data
 * in the CAN frame.
 *
 * @param[in] muxId                              multiplexer value
 * @param[in] pMessage                           pointer to CAN frame data
 * @param[in] cellTemperatureSignal              signal characteristics for temperature data
 * @param[in] cellTemperatureInvalidFlagSignal   signal characteristics for invalid flag data
 * @param[in] endianness                         big or little endianness of data
 * @param[in] kpkCanShim                         shim to the database entries
 */
static void CANTX_TemperatureSetData(
    uint8_t muxId,
    uint64_t *pMessage,
    CAN_SIGNAL_TYPE_s cellTemperatureSignal,
    CAN_SIGNAL_TYPE_s cellTemperatureInvalidFlagSignal,
    CAN_ENDIANNESS_e endianness,
    const CAN_SHIM_s *const kpkCanShim);

/*========== Static Function Implementations ================================*/

static void CANTX_TemperatureSetData(
    uint8_t muxId,
    uint64_t *pMessage,
    CAN_SIGNAL_TYPE_s cellTemperatureSignal,
    CAN_SIGNAL_TYPE_s cellTemperatureInvalidFlagSignal,
    CAN_ENDIANNESS_e endianness,
    const CAN_SHIM_s *const kpkCanShim) {
    /* sensor index must not be greater than the number of sensors */
    if (muxId < BS_NR_OF_TEMP_SENSORS) {
        /* start_index end_index module
         * 00          17        module1
         * 18          35        module2
         * 20          53        module3
         * 30          71        module4
         */
        /* start_index end_index string
         * 000         071       string0
         */

        /* Get string, module and cell number */
        const uint8_t stringNumber = DATA_GetStringNumberFromTemperatureIndex(muxId);
        const uint8_t moduleNumber = DATA_GetModuleNumberFromTemperatureIndex(muxId);
        const uint8_t sensorNumber = DATA_GetSensorNumberFromTemperatureIndex(muxId);

        uint32_t signalData_valid;
        /* Valid bits data */
        if ((kpkCanShim->pTableCellTemperature->invalidCellTemperature[stringNumber][moduleNumber] &
             (1u << sensorNumber)) == 0u) {
            signalData_valid = 0u;
        } else {
            signalData_valid = 1u;
        }
        /* Set valid bit data in CAN frame */
        CAN_TxSetMessageDataWithSignalData(
            pMessage,
            cellTemperatureInvalidFlagSignal.bitStart,
            cellTemperatureInvalidFlagSignal.bitLength,
            signalData_valid,
            endianness);

        /* Temperature data */
        float_t signalData_degC =
            (float_t)kpkCanShim->pTableCellTemperature
                ->cellTemperature_ddegC[stringNumber][(moduleNumber * BS_NR_OF_TEMP_SENSORS_PER_MODULE) + sensorNumber];
        signalData_degC /= UNIT_CONVERSION_FACTOR_10_FLOAT; /* Convert temperature from decidegC to degC */
        /* Apply offset and factor, check min/max limits */
        CAN_TxPrepareSignalData(&signalData_degC, cellTemperatureSignal);
        /* Set temperature data in CAN frame */
        CAN_TxSetMessageDataWithSignalData(
            pMessage,
            cellTemperatureSignal.bitStart,
            cellTemperatureSignal.bitLength,
            (int32_t)signalData_degC,
            endianness);
    }
}

/*========== Extern Function Implementations ================================*/
extern uint32_t CANTX_CellTemperatures(
    CAN_MESSAGE_PROPERTIES_s message,
    uint8_t *pCanData,
    uint8_t *pMuxId,
    const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(message.id == CANTX_CELL_TEMPERATURES_ID);
    FAS_ASSERT(message.idType == CANTX_CELL_TEMPERATURES_ID_TYPE);
    FAS_ASSERT(message.dlc == CAN_FOXBMS_MESSAGES_DEFAULT_DLC);
    FAS_ASSERT(pCanData != NULL_PTR);
    FAS_ASSERT(pMuxId != NULL_PTR);
    FAS_ASSERT(kpkCanShim != NULL_PTR);
    uint64_t messageData = 0u;

    /* Reset mux if maximum was reached */
    if (*pMuxId >= BS_NR_OF_TEMP_SENSORS) {
        *pMuxId = 0u;
    }

    /* first signal to transmit cell voltages: get database values */
    if (*pMuxId == 0u) {
        DATA_READ_DATA(kpkCanShim->pTableCellTemperature);
    }

    /* Set mux signal in CAN frame */
    uint32_t signalData = *pMuxId / 6u; /* 6 temperatures per module */
    CAN_TxSetMessageDataWithSignalData(
        &messageData,
        cantx_cellTemperatureMultiplexer.bitStart,
        cantx_cellTemperatureMultiplexer.bitLength,
        signalData,
        message.endianness);

    /* Set other signals in CAN frame */
    /* Each temperature frame contains 6 temperatures, with a correspond invalid flag*/
    CANTX_TemperatureSetData(
        *pMuxId,
        &messageData,
        cantx_cell0Temperature_degC,
        cantx_cell0TemperatureInvalidFlag,
        message.endianness,
        kpkCanShim);
    /* Increment multiplexer for next cell */
    (*pMuxId)++;
    CANTX_TemperatureSetData(
        *pMuxId,
        &messageData,
        cantx_cell1Temperature_degC,
        cantx_cell1TemperatureInvalidFlag,
        message.endianness,
        kpkCanShim);
    /* Increment multiplexer for next cell */
    (*pMuxId)++;
    CANTX_TemperatureSetData(
        *pMuxId,
        &messageData,
        cantx_cell2Temperature_degC,
        cantx_cell2TemperatureInvalidFlag,
        message.endianness,
        kpkCanShim);
    /* Increment multiplexer for next cell */
    (*pMuxId)++;
    CANTX_TemperatureSetData(
        *pMuxId,
        &messageData,
        cantx_cell3Temperature_degC,
        cantx_cell3TemperatureInvalidFlag,
        message.endianness,
        kpkCanShim);
    /* Increment multiplexer for next cell */
    (*pMuxId)++;
    CANTX_TemperatureSetData(
        *pMuxId,
        &messageData,
        cantx_cell4Temperature_degC,
        cantx_cell4TemperatureInvalidFlag,
        message.endianness,
        kpkCanShim);
    /* Increment multiplexer for next cell */
    (*pMuxId)++;
    CANTX_TemperatureSetData(
        *pMuxId,
        &messageData,
        cantx_cell5Temperature_degC,
        cantx_cell5TemperatureInvalidFlag,
        message.endianness,
        kpkCanShim);
    /* Increment multiplexer for next cell */
    (*pMuxId)++;

    /* All signal data copied in CAN frame, now copy data in the buffer that will be use to send the frame */
    CAN_TxSetCanDataWithMessageData(messageData, pCanData, message.endianness);

    return 0;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
